#pragma once
#include <Arduino.h>

// Single-page web UI: live dashboard + settings/calibration.
// It polls /api/state twice a second.
static const char WEB_UI_HTML[] PROGMEM = R"HTML(<!doctype html>
<html lang="en">
<head>
<meta charset="utf-8">
<meta name="viewport" content="width=device-width,initial-scale=1,viewport-fit=cover">
<meta name="theme-color" content="#0f1115">
<title>Domyos Ride</title>
<style>
:root{--bg:#f4f5f7;--card:#fff;--fg:#14161a;--muted:#6b7280;--line:#e3e5e9;--accent:#0a7cff;--ok:#16a34a;--warn:#d97706}
@media (prefers-color-scheme:dark){:root{--bg:#0f1115;--card:#181b21;--fg:#eef0f3;--muted:#8b93a1;--line:#262a32;--accent:#4c9dff;--ok:#22c55e;--warn:#f59e0b}}
*{box-sizing:border-box}
body{margin:0;background:var(--bg);color:var(--fg);font:16px/1.4 system-ui,-apple-system,Segoe UI,Roboto,sans-serif;-webkit-tap-highlight-color:transparent}
main{max-width:720px;margin:0 auto;padding:16px 16px calc(24px + env(safe-area-inset-bottom))}
header{display:flex;align-items:center;justify-content:space-between;gap:8px;margin-bottom:12px}
h1{font-size:18px;margin:0;font-weight:650}
.pills{display:flex;gap:6px;flex-wrap:wrap}
.pill{font-size:12px;padding:3px 9px;border-radius:99px;border:1px solid var(--line);color:var(--muted)}
.pill.on{color:var(--ok);border-color:currentColor}
.pill.pause{color:var(--warn);border-color:currentColor}
.card{background:var(--card);border:1px solid var(--line);border-radius:16px;padding:16px}
.hero{text-align:center;padding:20px 16px 12px}
.label{font-size:13px;color:var(--muted);text-transform:uppercase;letter-spacing:.06em}
.big{font-size:clamp(64px,22vw,120px);font-weight:700;line-height:1;font-variant-numeric:tabular-nums;letter-spacing:-.02em}
.unit{font-size:16px;color:var(--muted);margin-left:4px;font-weight:500;letter-spacing:0}
canvas{width:100%;height:70px;display:block;margin-top:10px}
.grid{display:grid;grid-template-columns:repeat(2,1fr);gap:10px;margin-top:10px}
@media (min-width:560px){.grid{grid-template-columns:repeat(3,1fr)}}
.val{font-size:32px;font-weight:650;font-variant-numeric:tabular-nums;margin-top:2px}
.val small{font-size:14px;color:var(--muted);font-weight:500;margin-left:3px}
.est{font-size:11px;color:var(--muted)}
.row{display:flex;gap:10px;margin-top:12px}
button{font:inherit;font-weight:600;border:0;border-radius:12px;padding:12px 16px;background:var(--accent);color:#fff;cursor:pointer;flex:1}
button.ghost{background:transparent;color:var(--fg);border:1px solid var(--line)}
button:active{transform:scale(.98)}
.hidden{display:none}
h2{font-size:15px;margin:0 0 10px}
.section{margin-top:12px}
label.f{display:grid;grid-template-columns:1fr 130px;align-items:center;gap:10px;padding:8px 0;border-top:1px solid var(--line);font-size:14px}
label.f:first-of-type{border-top:0}
label.f span small{display:block;color:var(--muted);font-size:12px}
input{font:inherit;width:100%;padding:8px 10px;border-radius:10px;border:1px solid var(--line);background:var(--bg);color:var(--fg);text-align:right}
input[type=text],input[type=password]{text-align:left}
.help{font-size:13px;color:var(--muted);margin:6px 0 0}
.calib{display:flex;align-items:baseline;justify-content:space-between;gap:10px}
.msg{font-size:13px;color:var(--ok);min-height:18px;margin-top:6px}
.lv{display:grid;grid-template-columns:repeat(8,1fr);gap:6px;margin-top:8px}
.lv button{padding:12px 0;background:transparent;color:var(--fg);border:1px solid var(--line);font-variant-numeric:tabular-nums}
.lv button.on{background:var(--accent);border-color:var(--accent);color:#fff}
.mults{display:grid;grid-template-columns:repeat(4,1fr);gap:8px;margin-top:8px}
.mults label{font-size:12px;color:var(--muted)}
select{font:inherit;width:100%;padding:8px 10px;border-radius:10px;border:1px solid var(--line);background:var(--bg);color:var(--fg)}
.res{width:100%;border-collapse:collapse;font-size:14px;font-variant-numeric:tabular-nums;margin-top:4px}
.res th,.res td{text-align:right;padding:6px 4px;border-top:1px solid var(--line)}
.res th:first-child,.res td:first-child{text-align:left}
.res th{color:var(--muted);font-weight:500;border-top:0}
#wLive{padding:12px 0 0}
</style>
</head>
<body>
<main>
<header>
  <h1>Domyos Ride</h1>
  <div class="pills">
    <span id="pMove" class="pill">idle</span>
    <span id="pBle" class="pill">BLE</span>
    <span id="pConn" class="pill">offline</span>
  </div>
</header>

<section id="dash">
  <div class="card hero">
    <div class="label">Speed</div>
    <div><span id="speed" class="big">0.0</span><span class="unit">km/h</span></div>
    <canvas id="chart"></canvas>
  </div>
  <div class="card section">
    <div class="label">Knob level</div>
    <div id="lv" class="lv"></div>
  </div>
  <div class="grid">
    <div class="card"><div class="label">Time</div><div id="time" class="val">0:00</div></div>
    <div class="card"><div class="label">Distance</div><div class="val"><span id="dist">0.00</span><small>km</small></div></div>
    <div class="card"><div class="label">Calories</div><div class="val"><span id="kcal">0</span><small>kcal</small></div><div class="est">estimate</div></div>
    <div class="card"><div class="label">Cadence</div><div class="val"><span id="rpm">0</span><small>rpm</small></div></div>
    <div class="card"><div class="label">Power</div><div class="val"><span id="watt">0</span><small>W</small></div><div class="est">estimate</div></div>
    <div class="card"><div class="label">Avg speed</div><div class="val"><span id="avg">0.0</span><small>km/h</small></div></div>
  </div>
  <div class="row">
    <button id="bReset" class="ghost">New ride</button>
    <button id="bSettings" class="ghost">Settings</button>
  </div>
</section>

<section id="setup" class="hidden">
  <div class="card">
    <h2>Calibration</h2>
    <div class="calib">
      <div><div class="label">Pulses counted</div><div class="val" id="cPulses">0</div></div>
      <button id="bZero" class="ghost" style="flex:0 0 auto">Zero</button>
    </div>
    <p class="help">Press <b>Zero</b>, pedal exactly 20 turns, and check the count matches (× pulses per rev). To match the old display's distance: ride on the old display, note the km and the number of pedal turns, then set <i>metres per revolution</i> = km × 1000 ÷ turns.</p>
  </div>

  <div class="card section">
    <h2>Bike &amp; rider</h2>
    <form id="fSet">
      <label class="f"><span>Metres per revolution<small>virtual wheel size</small></span><input name="mpr" type="number" step="0.01" inputmode="decimal"></label>
      <label class="f"><span>Weight (kg)</span><input name="weight" type="number" step="0.5" inputmode="decimal"></label>
      <label class="f"><span>Power factor<small>W = factor × rpm² at the reference knob level</small></span><input name="powerK" type="number" step="0.001" inputmode="decimal"></label>
      <label class="f"><span>Calorie multiplier<small>scale to match the old display</small></span><input name="calF" type="number" step="0.01" inputmode="decimal"></label>
      <label class="f"><span>Pulses per revolution</span><input name="ppr" type="number" step="1" inputmode="numeric"></label>
      <label class="f"><span>Debounce (ms)<small>raise if you see extra pulses</small></span><input name="debounce" type="number" step="1" inputmode="numeric"></label>
      <div class="row"><button type="submit">Save</button></div>
      <div id="mSet" class="msg"></div>
    </form>
  </div>

  <div class="card section">
    <h2>Knob levels</h2>
    <form id="fLvl">
      <p class="help">Power multiplier for each level, relative to the reference level (1.0). Power = power factor × multiplier × rpm².</p>
      <div id="mults" class="mults"></div>
      <div class="row"><button type="button" id="bCal" class="ghost">Calibrate levels</button><button type="submit">Save</button></div>
      <div id="mLvl" class="msg"></div>
    </form>
  </div>

  <div class="card section">
    <h2>Wi-Fi</h2>
    <form id="fWifi">
      <label class="f"><span>Network</span><input name="ssid" type="text" autocomplete="off"></label>
      <label class="f"><span>Password</span><input name="pass" type="password" autocomplete="off"></label>
      <div class="row"><button type="submit">Save &amp; restart</button></div>
      <p class="help" id="ipInfo"></p>
      <div id="mWifi" class="msg"></div>
    </form>
  </div>

  <div class="row"><button id="bBack" class="ghost">Back to ride</button></div>
</section>

<section id="wiz" class="hidden">
  <div class="card">
    <div class="label" id="wStep"></div>
    <h2 id="wTitle" style="margin-top:4px"></h2>
    <p class="help" id="wText"></p>
    <div id="wLive" class="hero">
      <div><span id="wRpm" class="big">0</span><span class="unit">rpm</span></div>
      <p class="help" id="wSub"></p>
    </div>
    <div id="wBody"></div>
    <div class="row" id="wBtns"></div>
  </div>
</section>
</main>

<script>
const $=id=>document.getElementById(id);
const hist=[];const HIST_MAX=600; // 5 min at 2 Hz
let zeroAt=0,lastTotal=0,okAt=0,cfg=null,lvBusy=0;
const LEVELS=8;
for(let i=1;i<=LEVELS;i++){
  const b=document.createElement('button');b.textContent=i;b.onclick=()=>setLevel(i);$('lv').appendChild(b);
  $('mults').insertAdjacentHTML('beforeend','<label>Level '+i+'<input name="m'+i+'" type="number" step="0.01" inputmode="decimal"></label>');
}
function markLevel(n){[...$('lv').children].forEach((b,i)=>b.classList.toggle('on',i+1===n));}
async function setLevel(n){markLevel(n);lvBusy++;
  try{await fetch('/api/level',{method:'POST',body:new URLSearchParams({level:n})});}finally{lvBusy--;}}

function fmtTime(s){const h=Math.floor(s/3600),m=Math.floor(s/60)%60,x=s%60;
  return (h?h+':'+String(m).padStart(2,'0'):m)+':'+String(x).padStart(2,'0');}

function draw(){
  const c=$('chart'),d=devicePixelRatio||1,w=c.clientWidth,h=c.clientHeight;
  if(c.width!==w*d){c.width=w*d;c.height=h*d;}
  const g=c.getContext('2d');g.setTransform(d,0,0,d,0,0);g.clearRect(0,0,w,h);
  if(hist.length<2)return;
  const max=Math.max(20,...hist)*1.1,step=w/(HIST_MAX-1),x0=w-(hist.length-1)*step;
  const col=getComputedStyle(document.documentElement).getPropertyValue('--accent').trim();
  g.beginPath();hist.forEach((v,i)=>{const x=x0+i*step,y=h-v/max*h;i?g.lineTo(x,y):g.moveTo(x,y);});
  g.strokeStyle=col;g.lineWidth=2;g.lineJoin='round';g.stroke();
  g.lineTo(w,h);g.lineTo(x0,h);g.closePath();g.globalAlpha=.15;g.fillStyle=col;g.fill();g.globalAlpha=1;
}

async function poll(){
  try{
    const r=await fetch('/api/state',{cache:'no-store'});const s=await r.json();okAt=Date.now();
    $('speed').textContent=s.speed.toFixed(1);
    $('time').textContent=fmtTime(s.time);
    $('dist').textContent=s.distance.toFixed(2);
    $('kcal').textContent=Math.round(s.kcal);
    $('rpm').textContent=Math.round(s.cadence);
    $('watt').textContent=Math.round(s.power);
    $('avg').textContent=(s.time?s.distance/(s.time/3600):0).toFixed(1);
    lastTotal=s.totalPulses;$('cPulses').textContent=s.totalPulses-zeroAt;
    const pm=$('pMove');pm.textContent=s.moving?'riding':(s.time?'paused':'idle');
    pm.className='pill '+(s.moving?'on':(s.time?'pause':''));
    const pb=$('pBle');pb.className='pill '+(s.ble?'on':'');pb.textContent=s.ble?'BLE app':'BLE';
    if(!lvBusy)markLevel(s.level);
    wizTick(s);
    hist.push(s.speed);if(hist.length>HIST_MAX)hist.shift();draw();
  }catch(e){}
  const on=Date.now()-okAt<3000;$('pConn').textContent=on?'live':'offline';$('pConn').className='pill '+(on?'on':'');
  setTimeout(poll,500);
}

async function loadSettings(){
  const s=await (await fetch('/api/settings')).json();const f=$('fSet');cfg=s;
  for(const k of ['mpr','weight','powerK','calF','ppr','debounce'])f[k].value=s[k];
  s.mult.forEach((m,i)=>$('fLvl')['m'+(i+1)].value=m);
  $('fWifi').ssid.value=s.ssid;$('ipInfo').textContent='Current IP: '+s.ip+' · also http://domyos.local';
}
function show(v){for(const id of ['dash','setup','wiz'])$(id).classList.toggle('hidden',id!==v);
  if(v==='setup')loadSettings();scrollTo(0,0);}

// Level calibration by effort matching, hands-free: after Start the page runs
// levels 1..8 on a timer and cues each knob change (beeps, vibration, voice).
// The rider keeps the same effort throughout; equal effort = equal power, so
// mult = (level 1 rpm / level rpm)², then scaled so the reference level is 1.0.
const TURN_S=10,SETTLE1_S=90,SETTLE_S=45,RETRY_S=20,MEASURE_S=60;
const W={on:false,run:false,ref:4,order:[],i:0,rpm:{},check:0,phase:'',until:0,p0:0,t0:0,lastP:0,beeped:0};
let actx=null;
function wBtns(list){const r=$('wBtns');r.innerHTML='';
  for(const [t,fn,ghost] of list){const b=document.createElement('button');b.textContent=t;b.type='button';
    if(ghost)b.className='ghost';b.onclick=fn;r.appendChild(b);}}
function wScreen(step,title,text,live){$('wStep').textContent=step;$('wTitle').textContent=title;
  $('wText').innerHTML=text;$('wLive').classList.toggle('hidden',!live);$('wBody').innerHTML='';$('wSub').textContent='';}
function wQuit(){W.on=W.run=false;W.phase='';try{speechSynthesis.cancel();}catch(e){}show('setup');}
function beep(n,ms,hz){if(!actx)return;for(let k=0;k<n;k++){const o=actx.createOscillator(),g=actx.createGain(),t=actx.currentTime+k*(ms+120)/1000;
  o.frequency.value=hz||880;g.gain.setValueAtTime(.25,t);g.gain.setValueAtTime(0,t+ms/1000);o.connect(g);g.connect(actx.destination);o.start(t);o.stop(t+ms/1000);}}
function say(t){try{speechSynthesis.cancel();speechSynthesis.speak(new SpeechSynthesisUtterance(t));}catch(e){}}
function cue(n,text){beep(n,180);try{navigator.vibrate(n>1?[200,100,200]:200);}catch(e){}if(text)setTimeout(()=>say(text),n*300);}

function wizIntro(){
  W.on=true;W.run=false;W.phase='';show('wiz');
  wScreen('Level calibration','Before you start',
    'This sets the power multiplier of each knob level by <b>effort matching</b>. You ride all 8 levels at the <b>same effort</b>, '+
    'pedalling faster on light levels and slower on heavy ones; the ESP32 measures your cadence at each. '+
    'After <b>Start</b> it runs by itself (about 18 minutes): a beep and a voice tell you when to turn the knob up.<br><br>'+
    '<b>Warm up for 5 minutes first</b>, turn the sound up and keep the screen on. '+
    'Choose a moderate effort you could also hold on level 8 by pedalling slowly; about 80–90 rpm on level 1 usually works. '+
    'A heart-rate watch helps a lot: keep the same heart rate the whole time. Without one, keep the same breathing.',false);
  let o='';for(let i=1;i<=LEVELS;i++)o+='<option'+(i===(cfg?cfg.level:4)?' selected':'')+'>'+i+'</option>';
  $('wBody').innerHTML='<label class="f"><span>Reference level<small>the level you usually ride; it keeps multiplier 1.0 and the current power factor</small></span><select id="wRef">'+o+'</select></label>';
  wBtns([['Cancel',wQuit,1],['Start on level 1',()=>{
    // Audio must be unlocked by this tap.
    try{actx=actx||new (window.AudioContext||window.webkitAudioContext)();actx.resume();}catch(e){}
    try{navigator.wakeLock&&navigator.wakeLock.request('screen').catch(()=>{});}catch(e){}
    W.ref=+$('wRef').value;W.rpm={};W.check=0;W.i=0;
    W.order=[1,2,3,4,5,6,7,8,'check'];W.run=true;
    wLevel(true);}]]);
}

function wLevel(first){
  const cur=W.order[W.i],L=cur==='check'?1:cur,r1=W.rpm[1];
  setLevel(L);
  const step='Step '+(W.i+1)+' of '+W.order.length;
  if(cur===1)wScreen(step,'Level 1: find your effort',
    'Pedal at a steady, moderate effort (about 80–90 rpm). Settle into it: this effort is the target for every level.',true);
  else if(cur==='check')wScreen(step,'Back to level 1: fatigue check',
    'Turn the knob back to <b>1</b> and keep the same effort. This shows whether you got tired along the way'+
    (r1?' (it was '+Math.round(r1)+' rpm at the start).':'.'),true);
  else wScreen(step,'Level '+L,
    'Turn the knob to <b>'+L+'</b> and keep the <b>same effort</b>: it is heavier, so pedal slower. Match the effort, not the cadence'+
    (r1?' (level 1 was '+Math.round(r1)+' rpm).':'.'),true);
  if(first){wPhase('settle',SETTLE1_S);cue(1,'Level 1. Find a steady moderate effort.');}
  else{wPhase('turn',TURN_S);cue(2,cur==='check'?'Turn back to level 1':'Turn to level '+L);}
  wRunBtns();
}

function wRunBtns(){
  const b=[['Stop',wQuit,1]];
  if(W.order[W.i]!==1)b.push(['Skip level',()=>{W.i++;wAdvance();},1]);
  b.push(W.run?['Pause',()=>{W.run=false;W.phase='paused';$('wSub').textContent='Paused. Resume restarts this level.';try{speechSynthesis.cancel();}catch(e){}wRunBtns();}]
              :['Resume',()=>{W.run=true;wPhase('settle',W.order[W.i]===1?SETTLE1_S:SETTLE_S);wRunBtns();}]);
  wBtns(b);
}

function wPhase(p,s){W.phase=p;W.until=Date.now()+s*1000;W.beeped=0;}
function wAdvance(){if(W.i<W.order.length)wLevel(false);else wResult();}

function wizTick(s){
  if(!W.on)return;
  $('wRpm').textContent=Math.round(s.cadence);
  if(!W.run){W.lastP=s.totalPulses;return;}
  const ppr=cfg?cfg.ppr:1,now=Date.now(),left=Math.max(0,Math.ceil((W.until-now)/1000)),pulse=s.totalPulses!==W.lastP;
  W.lastP=s.totalPulses;
  if(W.phase==='turn'){$('wSub').textContent='Turn the knob now… '+left+' s';if(left<=0)wPhase('settle',SETTLE_S);}
  else if(W.phase==='settle'){$('wSub').textContent='Settle into the effort… measuring in '+left+' s';
    if(left<=0)W.phase='wait';}
  else if(W.phase==='wait'){$('wSub').textContent=s.moving?'Starting measurement…':'Keep pedalling…';
    // Start on a pedal turn, so whole turns are counted.
    if(pulse&&s.moving){W.phase='meas';W.p0=s.totalPulses;W.t0=now;W.beeped=0;beep(1,120,660);}}
  else if(W.phase==='meas'){
    if(!s.moving){wPhase('settle',RETRY_S);cue(1,'Keep pedalling. Measuring this level again.');return;}
    const el=(now-W.t0)/1000,avg=(s.totalPulses-W.p0)/ppr/el*60,rem=Math.ceil(MEASURE_S-el);
    if(rem<=3&&rem>=1&&W.beeped!==rem){W.beeped=rem;beep(1,80,660);}
    if(el>=MEASURE_S&&pulse){const cur=W.order[W.i];if(cur==='check')W.check=avg;else W.rpm[cur]=avg;W.i++;wAdvance();return;}
    $('wSub').textContent='Measuring, hold it steady… '+Math.max(0,rem)+' s'+(el>5?' · average '+avg.toFixed(1)+' rpm':'');
  }
}

function wResult(){
  W.phase='';W.run=false;cue(3,'Calibration done. You can stop.');
  const r0=W.rpm[1],m={},est={};
  for(let L=1;L<=LEVELS;L++)if(W.rpm[L])m[L]=(r0/W.rpm[L])**2;
  // Fill skipped levels: log-linear between measured neighbours, or
  // extrapolated from the two nearest on one side; else keep the old value.
  const known=Object.keys(m).map(Number);
  const lerp=(a,b,L)=>Math.exp(Math.log(m[a])+(Math.log(m[b])-Math.log(m[a]))*(L-a)/(b-a));
  for(let L=1;L<=LEVELS;L++){if(m[L])continue;est[L]=1;
    const lo=known.filter(k=>k<L),hi=known.filter(k=>k>L);
    if(lo.length&&hi.length)m[L]=lerp(lo[lo.length-1],hi[0],L);
    else if(lo.length>1)m[L]=lerp(lo[lo.length-2],lo[lo.length-1],L);
    else if(hi.length>1)m[L]=lerp(hi[0],hi[1],L);
    else m[L]=cfg.mult[L-1]/cfg.mult[0];}
  const k=m[W.ref];for(let L=1;L<=LEVELS;L++)m[L]/=k;
  let rows='';
  for(let L=1;L<=LEVELS;L++)rows+='<tr><td>'+L+(L===W.ref?' (ref)':'')+'</td><td>'+(W.rpm[L]?W.rpm[L].toFixed(1):'–')+
    '</td><td>'+cfg.mult[L-1].toFixed(2)+'</td><td><b>'+m[L].toFixed(2)+'</b>'+(est[L]?' <small>est.</small>':'')+'</td></tr>';
  const notes=[];
  if(W.check){const d=W.check/r0-1;
    notes.push(Math.abs(d)>0.08
      ?'⚠ Fatigue check: level 1 was '+W.check.toFixed(1)+' rpm at the end vs '+r0.toFixed(1)+' at the start ('+(d*100).toFixed(0)+' %). '+
       'Your effort drifted, so the later levels are off. Consider resting and redoing the calibration.'
      :'✓ Fatigue check: level 1 was '+W.check.toFixed(1)+' rpm at the end vs '+r0.toFixed(1)+' at the start, consistent.');}
  for(let L=2;L<=LEVELS;L++)if(m[L]<m[L-1]){notes.push('⚠ Level '+L+' came out lighter than level '+(L-1)+'. Consider redoing those two.');break;}
  if(Object.keys(est).length)notes.push('Levels marked est. were skipped and estimated from their neighbours.');
  wScreen('Level calibration','Results',
    'Multiplier = (level 1 rpm ÷ level rpm)², scaled so level '+W.ref+' is 1.0. The power factor ('+cfg.powerK+') applies to level '+W.ref+'.',false);
  $('wBody').innerHTML='<table class="res"><tr><th>Level</th><th>rpm</th><th>Old</th><th>New</th></tr>'+rows+'</table>'+
    notes.map(n=>'<p class="help">'+n+'</p>').join('');
  wBtns([['Discard',wQuit,1],['Save',async()=>{
    const b=new URLSearchParams();for(let L=1;L<=LEVELS;L++)b.set('m'+L,m[L].toFixed(3));
    await fetch('/api/settings',{method:'POST',body:b});wQuit();
    $('mLvl').textContent='Level calibration saved.';setTimeout(()=>$('mLvl').textContent='',4000);}]]);
}

$('bCal').onclick=wizIntro;
$('bSettings').onclick=()=>show('setup');
$('bBack').onclick=()=>show('dash');
$('bReset').onclick=async()=>{if(!confirm('Start a new ride? Current values will be cleared.'))return;
  await fetch('/api/reset',{method:'POST'});hist.length=0;draw();};
$('bZero').onclick=()=>{zeroAt=lastTotal;$('cPulses').textContent=0;};
$('fSet').onsubmit=async e=>{e.preventDefault();
  await fetch('/api/settings',{method:'POST',body:new URLSearchParams(new FormData(e.target))});
  $('mSet').textContent='Saved.';setTimeout(()=>$('mSet').textContent='',2000);loadSettings();};
$('fLvl').onsubmit=async e=>{e.preventDefault();
  await fetch('/api/settings',{method:'POST',body:new URLSearchParams(new FormData(e.target))});
  $('mLvl').textContent='Saved.';setTimeout(()=>$('mLvl').textContent='',2000);loadSettings();};
$('fWifi').onsubmit=async e=>{e.preventDefault();
  await fetch('/api/wifi',{method:'POST',body:new URLSearchParams(new FormData(e.target))});
  $('mWifi').textContent='Saved. Restarting — reconnect to the new network.';};
addEventListener('resize',draw);
poll();
</script>
</body>
</html>)HTML";
