#pragma once
#include <Arduino.h>
const char INTERACTIONS_SCRIPT[] PROGMEM = R"JS('use strict';
let trendData=[],autoCapture=null,autoChecking=false;
function drawTrend(){
  const canvas=$('trendChart');if(!canvas)return;
  const width=Math.max(280,canvas.clientWidth),height=240,dpr=window.devicePixelRatio||1;
  canvas.width=Math.round(width*dpr);canvas.height=Math.round(height*dpr);
  const ctx=canvas.getContext('2d');ctx.scale(dpr,dpr);
  const css=getComputedStyle(document.body),ink=css.getPropertyValue('--muted'),accent=css.getPropertyValue('--accent'),grid=css.getPropertyValue('--line');
  const i=Number($('chartChannel').value),rows=trendData,vals=rows.map(p=>p.values[i]).filter(v=>v!==null&&Number.isFinite(v));
  ctx.clearRect(0,0,width,height);ctx.font='12px system-ui';ctx.fillStyle=ink;
  if(!vals.length){ctx.fillText('Aguardando leituras válidas deste canal.',18,95);$('chartSummary').textContent=names[i]+' · sem dados disponíveis';return}
  const low=Math.min(...vals),high=Math.max(...vals),pad=high===low?Math.max(1,Math.abs(high)*.05):(high-low)*.12,lo=low-pad,hi=high+pad;
  const left=62,right=width-15,top=14,bottom=190,old=rows[0].age_ms,last=rows[rows.length-1].age_ms;
  const x=p=>old===last?(left+right)/2:left+(old-p.age_ms)/(old-last)*(right-left),y=v=>bottom-(v-lo)/(hi-lo)*(bottom-top);
  for(let k=0;k<5;k++){const py=top+k*(bottom-top)/4;ctx.strokeStyle=grid;ctx.beginPath();ctx.moveTo(left,py);ctx.lineTo(right,py);ctx.stroke();ctx.fillText((hi-k*(hi-lo)/4).toFixed(i===3?0:1),2,py+4)}
  ctx.strokeStyle=accent;ctx.lineWidth=2;ctx.fillStyle=accent;let prev=null;
  for(const row of rows){const v=row.values[i];if(v===null){prev=null;continue}const pt=[x(row),y(v)];if(prev&&prev.age-row.age_ms<=15000){ctx.beginPath();ctx.moveTo(prev.x,prev.y);ctx.lineTo(pt[0],pt[1]);ctx.stroke()}else{ctx.beginPath();ctx.arc(pt[0],pt[1],2,0,Math.PI*2);ctx.fill()}prev={x:pt[0],y:pt[1],age:row.age_ms}}
  ctx.fillStyle=ink;ctx.fillText('há '+age(old),left,215);const end='há '+age(last);ctx.fillText(end,right-ctx.measureText(end).width,215);
  const sim=rows.some(p=>(p.simulated_mask&(1<<i))&&p.values[i]!==null);
  $('chartSummary').textContent=names[i]+' · '+(sim?'SIMULADO':'SENSOR REAL')+' · mín. '+low.toFixed(2)+' / máx. '+high.toFixed(2)+' '+units[i]+' · '+vals.length+' pontos'+(!connected?' · DADOS ANTIGOS':'');
}
function drawOledGraph(ctx,i){
  const vals=trendData.map(p=>p.values[i]).filter(v=>v!==null);
  if(!vals.length){ctx.fillText('Aguardando leituras',0,32);return}
  const lo=Math.min(...vals),hi=Math.max(...vals);ctx.fillText(hi.toFixed(Math.abs(hi)>=100?0:1),0,25);ctx.fillText(lo.toFixed(Math.abs(lo)>=100?0:1),0,44);
  ctx.fillRect(34,24,1,29);ctx.fillRect(34,52,94,1);ctx.strokeStyle=ctx.fillStyle;ctx.lineWidth=1;
  const old=trendData[0].age_ms,last=trendData[trendData.length-1].age_ms;let prev=null;
  for(const p of trendData){const v=p.values[i];if(v===null){prev=null;continue}const x=old>last?35+(old-p.age_ms)*91/(old-last):80,y=hi>lo?50-(v-lo)*24/(hi-lo):38;if(prev&&prev.age-p.age_ms<=15000){ctx.beginPath();ctx.moveTo(prev.x,prev.y);ctx.lineTo(x,y);ctx.stroke()}else ctx.fillRect(x,y,1,1);prev={x,y,age:p.age_ms}}
}
function cancelCapture(message='Captura automática cancelada.'){
  if(!autoCapture)return;autoCapture=null;$('autoStatus').textContent=message;$('cancelAuto').hidden=true;calState();
}
async function startAuto(kind){
  if(autoCapture||writing){toast('Conclua ou cancele a captura em andamento.');return}
  await action(async()=>{
    const channel=confirmed();if(!$('key').value)throw Error('Informe a senha da placa.');
    const reference=num(kind==='temp'?'tempRef':kind==='verify'?'verifyRef':'ref'+kind);
    const tolerance=kind==='verify'?num('tolerance'):null;
    const index={temp:0,ph:1,turb:2,ec:3}[channel],max={temp:85,ph:14,turb:100,ec:100000}[channel];
    if(reference<(channel==='temp'?-10:0)||reference>max)throw Error('Referência fora da faixa do canal.');
    if(tolerance!==null&&tolerance<=0)throw Error('Informe uma tolerância positiva.');
    if(index>0&&kind===2&&!saved.calibrations[index-1].first_captured)throw Error('Capture primeiro o ponto 1.');
    if(kind==='verify'&&!saved.calibrations[index-1].configured)throw Error('Capture os dois pontos antes de verificar.');
    await command('preparar '+channel);
    autoCapture={channel,index,kind,reference,tolerance,started:performance.now(),uptime:latest.uptime_ms};
    $('cancelAuto').hidden=false;$('autoStatus').textContent='Aguardando 20 s mínimos e quatro leituras estáveis. Não troque a referência até a conclusão.';
  });
}
async function checkAuto(){
  const task=autoCapture;if(!task||autoChecking)return;
  if(!connected||!latest||latest.uptime_ms<task.uptime){cancelCapture('Conexão interrompida ou placa reiniciada. Confira o estado e inicie novamente.');return}
  const elapsed=performance.now()-task.started;if(elapsed>180000){cancelCapture('Sem estabilidade em 3 minutos. Confira sonda, referência e montagem.');return}
  if($('calChannel').value!==task.channel||!$('referenceConfirmed').checked||latest.readings[task.index].simulated){cancelCapture('Preparação ou modo alterado. Inicie novamente quando estiver pronto.');return}
  const r=latest.readings[task.index],remaining=Math.max(0,Math.ceil((20000-elapsed)/1000));
  $('autoStatus').textContent=remaining?'Captura automática: aguarde pelo menos '+remaining+' s. Não troque a referência.':'Captura automática: aguardando estabilidade e condições do sensor.';
  const t=latest.readings[0];
  if(elapsed<20000||!r.stable||(task.index>0&&(r.mv<150||r.mv>2300))||(task.index===0&&(r.value===null||r.state!=='atual'))||(task.index===3&&(t.value===null||t.state!=='atual'||Math.abs(t.value-25)>.5)))return;
  if(writing)return;autoChecking=true;autoCapture=null;$('cancelAuto').hidden=true;
  const cmd=task.kind==='temp'?'tempcal '+task.reference:task.kind==='verify'?'verificar '+task.channel+' '+task.reference+' '+task.tolerance:'capturar '+task.channel+' '+task.kind+' '+task.reference;
  await action(async()=>{try{const result=await command(cmd);$('autoStatus').textContent=result+(task.kind===1?' Próximo passo: enxágue, coloque a segunda referência e inicie o ponto 2.':task.kind===2?' Próximo passo: use uma terceira referência independente.':'');toast(result)}catch(e){$('autoStatus').textContent='Captura não confirmada: '+e.message+' Confira o estado antes de repetir.';throw e}});
  autoChecking=false;calState();
}
function initInteractions(){
  $('chartChannel').onchange=drawTrend;window.addEventListener('resize',drawTrend);
  $('cancelAuto').onclick=()=>cancelCapture();
  $('capture1').onclick=()=>startAuto(1);$('capture2').onclick=()=>startAuto(2);$('verify').onclick=()=>startAuto('verify');$('tempCal').onclick=()=>startAuto('temp');
  ['ref1','ref2','verifyRef','tolerance','tempRef'].forEach(id=>$(id).addEventListener('input',()=>cancelCapture('Referência alterada; inicie a captura novamente.')));
  $('calChannel').addEventListener('change',()=>cancelCapture());$('referenceConfirmed').addEventListener('change',()=>{if(!$('referenceConfirmed').checked)cancelCapture()});
  document.addEventListener('visibilitychange',()=>{if(document.hidden)cancelCapture('Captura cancelada porque a página ficou em segundo plano.');});
}
)JS";
