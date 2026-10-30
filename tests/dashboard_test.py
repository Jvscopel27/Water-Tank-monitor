"""Browser checks for the offline dashboard and its generated PDF (mock data)."""
import argparse
import base64
import json
import math
import re
from pathlib import Path
from urllib.parse import parse_qs

from playwright.sync_api import sync_playwright
from pypdf import PdfReader
import pymupdf as fitz

ROOT = Path(__file__).resolve().parents[1]
parser = argparse.ArgumentParser()
parser.add_argument('--output', required=True)
args = parser.parse_args()
out = Path(args.output)
out.mkdir(parents=True, exist_ok=True)
html = (ROOT / 'firmware/WaterTankMonitor/Panel.h').read_text(encoding='utf-8').split('R"HTML(', 1)[1].split(')HTML";', 1)[0]
report = (ROOT / 'firmware/WaterTankMonitor/Report.h').read_text(encoding='utf-8').split('R"JS(', 1)[1].split(')JS";', 1)[0]
interactions = (ROOT / 'firmware/WaterTankMonitor/Interactions.h').read_text(encoding='utf-8').split('R"JS(', 1)[1].split(')JS";', 1)[0]
settings = dict(dark=0, layout=0, focus=0, rotation=0, inverse=0, contrast=160, demo_mask=7,
                temperature_offset=0, oled_ready=True, calibrations=[dict(configured=False, verified=False, first_captured=False) for _ in range(3)])
snapshot = dict(uptime_ms=3600000, session_remaining_s=0, assessment='DEMONSTRACAO: dados de teste, nao sao medicoes reais.',
                readings=[dict(value=v, mv=m, state='simulado' if i else 'sensor_indisponivel', simulated=i>0, stable=True, stability_samples=4, stability_span=2 if i else 0, age_ms=2000 if i else None)
                          for i, (v,m) in enumerate([(None,0),(7.1,1700),(46.2,1600),(1050,1750)])])
history = [dict(age_ms=(60-i)*60000, mean=[None,7+math.sin(i/7),45+i/5,1000+math.cos(i/8)*100], count=[0,12,12,12], simulated_mask=14) for i in range(60)]
trend = [dict(age_ms=(48-i)*5000,values=[None,7+math.sin(i/6),50+i/4,1000+math.cos(i/7)*100],simulated_mask=14) for i in range(48)]
commands=[]
offline = False
def route(r):
    path=r.request.url.split('http://test.local',1)[-1]
    if offline and path.startswith('/api/'):
        r.abort('internetdisconnected');return
    if path=='/': r.fulfill(body=html,content_type='text/html; charset=utf-8')
    elif path=='/report.js':r.fulfill(body=report,content_type='application/javascript; charset=utf-8')
    elif path=='/interactions.js':r.fulfill(body=interactions,content_type='application/javascript; charset=utf-8')
    elif path=='/api/settings':r.fulfill(json=settings)
    elif path=='/api/readings':r.fulfill(json=snapshot)
    elif path=='/api/history':r.fulfill(json=history)
    elif path=='/api/trend':r.fulfill(json=trend)
    elif path=='/api/command':
        fields=parse_qs(r.request.post_data or '')
        if fields.get('senha')!=['test-password']:r.fulfill(status=403,body='Senha invalida');return
        c=fields['comando'][0];commands.append(c)
        if c.startswith('ui '):
            for k,v in zip(['dark','layout','focus','rotation','inverse','contrast'],map(int,c.split()[1:])):settings[k]=v
        if c.startswith('preparar ph'):
            snapshot['readings'][1]['stable']=False;snapshot['readings'][1]['stability_samples']=0
        if c.startswith('capturar ph 1 '):settings['calibrations'][0]['first_captured']=True
        r.fulfill(body='Configuracao salva.')
    else:r.fulfill(status=404,body='Not found')

with sync_playwright() as p:
    browser=p.chromium.launch(executable_path=r'C:\Program Files\Google\Chrome\Application\chrome.exe', headless=True)
    page=browser.new_page(viewport=dict(width=1320,height=1000),device_scale_factor=1)
    errors=[];page.on('pageerror',lambda e:errors.append(str(e)))
    page.route('http://test.local/**',route);page.goto('http://test.local/')
    page.wait_for_selector('.card');assert page.locator('.card').count()==4
    page.screenshot(path=str(out/'dashboard-light.png'),full_page=False)
    page.locator('#theme').click();assert 'dark' in page.locator('body').get_attribute('class')
    page.locator('#key').fill('test-password')
    # An unrelated action must not discard the unsaved OLED preview.
    page.locator('#layout').select_option('2')
    page.locator('#startSession').click();page.wait_for_function('!writing')
    assert page.locator('#layout').input_value()=='2', 'Session action discarded unsaved display preference'
    for layout in ['1','2','3','0']:
        page.locator('#layout').select_option(layout)
        page.locator('#screenForm button.primary').click();page.wait_for_function('!writing')
        assert settings['layout']==int(layout)
        if layout=='3':
            page.locator('#focus').select_option('2')
            page.locator('.oled').screenshot(path=str(out/'oled-graph-preview.png'))
    page.reload();page.wait_for_selector('.card');assert 'dark' in page.locator('body').get_attribute('class')
    page.evaluate('scrollTo(0,0)');page.screenshot(path=str(out/'dashboard-dark.png'))
    page.locator('#screen').screenshot(path=str(out/'oled-controls.png'))
    page.locator('#referenceConfirmed').check();assert page.locator('#capture1').is_disabled()
    page.locator('#calChannel').select_option('temp');assert page.locator('#temperatureWizard').is_visible()
    page.locator('#calChannel').select_option('ph')
    # Auto capture waits for stability and sends the final command only once.
    page.locator('#key').fill('test-password')
    snapshot['readings'][1].update(simulated=False,state='calibracao_pendente')
    page.evaluate('update()');page.wait_for_function('!busy')
    page.locator('#referenceConfirmed').check();page.locator('#ref1').fill('7')
    page.locator('#capture1').click();page.wait_for_function('!writing && autoCapture !== null')
    page.evaluate('autoCapture.started -= 21000; checkAuto()')
    assert not any(c.startswith('capturar ph') for c in commands)
    snapshot['readings'][1].update(stable=True,stability_samples=4)
    page.evaluate('update()');page.wait_for_function('!writing && !autoChecking && autoCapture === null')
    assert commands.count('capturar ph 1 7')==1
    page.evaluate('update()');page.wait_for_function('!busy');assert commands.count('capturar ph 1 7')==1
    page.locator('#ref2').fill('4');page.locator('#capture2').click();page.wait_for_function('!writing && autoCapture !== null')
    page.locator('#ref2').fill('4.01');assert page.evaluate('autoCapture===null')
    # Losing connectivity cancels capture and retains unsaved preferences.
    page.locator('#capture2').click();page.wait_for_function('!writing && autoCapture !== null')
    page.locator('#layout').select_option('3');offline=True
    page.evaluate('update()');page.wait_for_function('!busy && !connected')
    assert page.evaluate('autoCapture===null') and page.locator('#layout').input_value()=='3'
    offline=False;page.evaluate('update()');page.wait_for_function('!busy && connected')
    assert page.locator('#layout').input_value()=='3'
    page.locator('#restoreScreen').click();page.wait_for_function('!writing');assert page.locator('#layout').input_value()==str(settings['layout'])
    snapshot['readings'][1].update(simulated=True,state='simulado',stable=True,stability_samples=4)
    page.locator('#charts').screenshot(path=str(out/'live-chart.png'))
    page.locator('#history').click();page.wait_for_selector('#rows tr');assert page.locator('#rows tr').count()==60
    with page.expect_download() as download:page.locator('#pdf').click()
    download.value.save_as(out/'report-sample.pdf')
    reader=PdfReader(out/'report-sample.pdf');assert len(reader.pages)==4
    text='\n'.join(x.extract_text() for x in reader.pages)
    assert 'SIM' in text and 'POTABILIDADE' in text and 'Condutividade' in text
    pdf=fitz.open(out/'report-sample.pdf')
    for i in [0,1,3]:pdf[i].get_pixmap(matrix=fitz.Matrix(1.4,1.4)).save(str(out/f'report-page-{i+1}.png'))
    # Empty and maximum-size histories must create valid, complete PDF files.
    for count in [0,1440]:
        payload=page.evaluate('''async ({snapshot,rows,count})=>{const b=new Uint8Array(await buildReport(snapshot,Array.from({length:count},(_,i)=>({...rows[i%rows.length],age_ms:(count-i)*60000})),new Date()).arrayBuffer());let s='';for(let i=0;i<b.length;i+=8192)s+=String.fromCharCode(...b.subarray(i,i+8192));return btoa(s)}''',dict(snapshot=snapshot,rows=history,count=count))
        path=out/f'report-{count}.pdf';path.write_bytes(base64.b64decode(payload))
        assert len(PdfReader(path).pages)==1+math.ceil(count/27)
    page.set_viewport_size(dict(width=390,height=844));page.goto('http://test.local/');page.wait_for_selector('.card')
    assert page.evaluate('document.documentElement.scrollWidth <= innerWidth')
    page.screenshot(path=str(out/'dashboard-mobile.png'))
    assert not errors,errors
    browser.close()
print('PASS: four OLED formats, graphs, unsaved choices, auto capture exactly once, cancellation, offline recovery, mobile width and PDF pagination.')
