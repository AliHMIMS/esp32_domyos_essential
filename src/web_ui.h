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
      <label class="f"><span>Power factor<small>W = factor × rpm²; raise for more tension</small></span><input name="powerK" type="number" step="0.001" inputmode="decimal"></label>
      <label class="f"><span>Calorie multiplier<small>scale to match the old display</small></span><input name="calF" type="number" step="0.01" inputmode="decimal"></label>
      <label class="f"><span>Pulses per revolution</span><input name="ppr" type="number" step="1" inputmode="numeric"></label>
      <label class="f"><span>Debounce (ms)<small>raise if you see extra pulses</small></span><input name="debounce" type="number" step="1" inputmode="numeric"></label>
      <div class="row"><button type="submit">Save</button></div>
      <div id="mSet" class="msg"></div>
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
</main>

<script>
const $=id=>document.getElementById(id);
const hist=[];const HIST_MAX=600; // 5 min at 2 Hz
let zeroAt=0,lastTotal=0,okAt=0;

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
    hist.push(s.speed);if(hist.length>HIST_MAX)hist.shift();draw();
  }catch(e){}
  const on=Date.now()-okAt<3000;$('pConn').textContent=on?'live':'offline';$('pConn').className='pill '+(on?'on':'');
  setTimeout(poll,500);
}

async function loadSettings(){
  const s=await (await fetch('/api/settings')).json();const f=$('fSet');
  for(const k of ['mpr','weight','powerK','calF','ppr','debounce'])f[k].value=s[k];
  $('fWifi').ssid.value=s.ssid;$('ipInfo').textContent='Current IP: '+s.ip+' · also http://domyos.local';
}
function show(setup){$('dash').classList.toggle('hidden',setup);$('setup').classList.toggle('hidden',!setup);
  if(setup)loadSettings();scrollTo(0,0);}

$('bSettings').onclick=()=>show(true);
$('bBack').onclick=()=>show(false);
$('bReset').onclick=async()=>{if(!confirm('Start a new ride? Current values will be cleared.'))return;
  await fetch('/api/reset',{method:'POST'});hist.length=0;draw();};
$('bZero').onclick=()=>{zeroAt=lastTotal;$('cPulses').textContent=0;};
$('fSet').onsubmit=async e=>{e.preventDefault();
  await fetch('/api/settings',{method:'POST',body:new URLSearchParams(new FormData(e.target))});
  $('mSet').textContent='Saved.';setTimeout(()=>$('mSet').textContent='',2000);loadSettings();};
$('fWifi').onsubmit=async e=>{e.preventDefault();
  await fetch('/api/wifi',{method:'POST',body:new URLSearchParams(new FormData(e.target))});
  $('mWifi').textContent='Saved. Restarting — reconnect to the new network.';};
addEventListener('resize',draw);
poll();
</script>
</body>
</html>)HTML";
