#pragma once
#include <Arduino.h>
// PDF vetorial gerado no navegador, sem dependencias externas ou acesso a internet.
const char REPORT_SCRIPT[] PROGMEM = R"JS('use strict';
function buildReport(snapshot,history,generated){
  const pages=[],W=595,H=842,ink=[.07,.19,.26],teal=[.02,.48,.50],muted=[.32,.42,.48],pale=[.92,.96,.96];
  const labels=['Temperatura','pH','Turbidez relativa','Condutividade'],units=['°C','','% rel.','µS/cm'];
  let ops=[];
  function esc(t){return String(t).replace(/[–—−]/g,'-').replace(/…/g,'...').replace(/[^\x20-\xff]/g,'?').replace(/([\\()])/g,'\\$1')}
  function rect(x,y,w,h,c){ops.push(c.join(' ')+' rg '+x+' '+(H-y-h)+' '+w+' '+h+' re f')}
  function text(t,x,y,size=10,color=ink,bold=false){ops.push('BT /'+(bold?'F2':'F1')+' '+size+' Tf '+color.join(' ')+' rg 1 0 0 1 '+x+' '+(H-y)+' Tm ('+esc(t)+') Tj ET')}
  function line(x1,y1,x2,y2,color=teal){ops.push(color.join(' ')+' RG 1 w '+x1+' '+(H-y1)+' m '+x2+' '+(H-y2)+' l S')}
  function newPage(){if(ops.length)pages.push(ops.join('\n'));ops=[];rect(0,0,W,7,teal);text('WATER TANK / BANCADA',36,35,10,teal,true);line(36,51,559,51,[.82,.89,.90])}
  const fmt=(v,i)=>v===null||!Number.isFinite(v)?'--':v.toFixed(i===1?2:1);
  const when=generated.toLocaleString('pt-BR');
  newPage();text('Relatório de monitoramento',36,92,25,ink,true);text('Uma visão documentada da sua sessão de bancada.',36,115,11,muted);
  text('Gerado em '+when+' (relógio do navegador)',36,142,9,muted);text('ESP32 ligado há '+Math.floor(snapshot.uptime_ms/60000)+' min | '+history.length+' médias de um minuto',36,158,9,muted);
  rect(36,176,523,48,[1,.95,.83]);text('PROTÓTIPO EXPERIMENTAL - POTABILIDADE NÃO CONFIRMADA',48,195,10,ink,true);text('SIM indica potenciômetro. Valores simulados não são medições da água.',48,212,9,ink);
  text('01 / LEITURAS NA EXPORTAÇÃO',36,253,11,teal,true);
  snapshot.readings.forEach((r,i)=>{const x=36+i%2*268,y=270+Math.floor(i/2)*106;rect(x,y,255,94,pale);text(labels[i]+(r.simulated?' / SIM':' / REAL'),x+12,y+20,10,teal,true);text(fmt(r.value,i)+' '+units[i],x+12,y+52,23,ink,true);text(r.state.replaceAll('_',' '),x+12,y+72,9,muted);text(r.age_ms===null?'Sem amostra válida':'Idade da amostra: '+Math.floor(r.age_ms/1000)+' s',x+12,y+85,8,muted)});
  text('02 / VISÃO DO HISTÓRICO',36,510,11,teal,true);
  for(let i=0;i<4;i++){
    const x=36+i%2*268,y=526+Math.floor(i/2)*100;
    const valid=history.filter(r=>r.mean[i]!==null&&Number.isFinite(r.mean[i]));const vals=valid.map(r=>r.mean[i]);
    text(labels[i],x,y,10,ink,true);
    if(!vals.length){text('Sem médias disponíveis neste canal.',x,y+25,9,muted);continue}
    const lo=Math.min(...vals),hi=Math.max(...vals),sim=valid.some(r=>r.simulated_mask&(1<<i));
    text('Mín. '+fmt(lo,i)+' / Máx. '+fmt(hi,i)+' '+units[i]+(sim?' / SIM':''),x,y+16,8,muted);
    const top=y+29,bottom=y+65;line(x,bottom,x+245,bottom,[.82,.89,.90]);
    let prev=null;
    history.forEach((r,j)=>{if(r.mean[i]===null){prev=null;return}const pt=[x+(history.length>1?j/(history.length-1):.5)*245,bottom-(hi>lo?(r.mean[i]-lo)/(hi-lo):.5)*32];if(prev)line(prev[0],prev[1],pt[0],pt[1]);else rect(pt[0],pt[1],2,2,teal);prev=pt});
  }
  text('Traços: médias em ordem temporal, escala própria em cada canal.',36,727,8,muted);
  text('Sem data de calendário na placa: o histórico usa idade relativa à consulta.',36,745,8,muted);
  text('Histórico em RAM. Reinício e mudanças de modo/calibração apagam registros.',36,760,8,muted);
  for(let start=0;start<history.length;start+=27){
    newPage();text('Registro detalhado',36,87,23,ink,true);text('Médias de um minuto | SIM = simulado | -- = sem leitura válida',36,109,9,muted);
    rect(36,125,523,28,teal);['Há (min)','Temp. °C','pH','Turb. %','EC µS/cm'].forEach((t,i)=>text(t,46+i*104,143,9,[1,1,1],true));
    history.slice(start,start+27).forEach((r,j)=>{let y=154+j*22;if(j%2===0)rect(36,y,523,22,pale);const cells=[(r.age_ms/60000).toFixed(1),...r.mean.map((v,i)=>fmt(v,i)+(v!==null&&(r.simulated_mask&(1<<i))?' SIM':''))];cells.forEach((t,i)=>text(t,46+i*104,y+14,9,ink))});
    text('Células sem média não foram substituídas por zero. Intervalos sem dados podem existir.',36,775,8,muted);
  }
  pages.push(ops.join('\n'));
  const objects=[null,'<< /Type /Catalog /Pages 2 0 R >>','', '<< /Type /Font /Subtype /Type1 /BaseFont /Helvetica /Encoding /WinAnsiEncoding >>','<< /Type /Font /Subtype /Type1 /BaseFont /Helvetica-Bold /Encoding /WinAnsiEncoding >>'];
  const kids=[];
  pages.forEach((stream,i)=>{const footer='\nBT /F1 8 Tf .32 .42 .48 rg 1 0 0 1 36 24 Tm (Water Tank Monitor - Relatorio local) Tj ET\nBT /F1 8 Tf .32 .42 .48 rg 1 0 0 1 500 24 Tm ('+(i+1)+' / '+pages.length+') Tj ET';const content=stream+footer;const p=objects.length,c=p+1;kids.push(p+' 0 R');objects.push('<< /Type /Page /Parent 2 0 R /MediaBox [0 0 595 842] /Resources << /Font << /F1 3 0 R /F2 4 0 R >> >> /Contents '+c+' 0 R >>');objects.push('<< /Length '+content.length+' >>\nstream\n'+content+'\nendstream')});
  objects[2]='<< /Type /Pages /Count '+pages.length+' /Kids ['+kids.join(' ')+'] >>';let pdf='%PDF-1.4\n',offsets=[0];for(let i=1;i<objects.length;i++){offsets[i]=pdf.length;pdf+=i+' 0 obj\n'+objects[i]+'\nendobj\n'}const xref=pdf.length;pdf+='xref\n0 '+objects.length+'\n0000000000 65535 f \n';for(let i=1;i<objects.length;i++)pdf+=String(offsets[i]).padStart(10,'0')+' 00000 n \n';pdf+='trailer\n<< /Size '+objects.length+' /Root 1 0 R >>\nstartxref\n'+xref+'\n%%EOF';return new Blob([Uint8Array.from(pdf,c=>c.charCodeAt(0))],{type:'application/pdf'});
}
)JS";
