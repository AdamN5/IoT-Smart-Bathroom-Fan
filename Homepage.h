String homePageHeader = R"====(
<!DOCTYPE html>
<html>
<head>
<title>Smart Bathroom Fan</title>
<meta charset="UTF-8">
<meta name="viewport" content="width=device-width, initial-scale=1">
<style>
  body {
    font-family: Arial, sans-serif;
    background: #f0f0f0;
    margin: 0;
  }

  h1 {
    background-color: #3366cc;
    color: white;
    margin: 0;
    padding: 12px 16px;
    font-size: 20px;
  }

  .grid {
    display: flex;
    flex-wrap: wrap;
    gap: 12px;
    padding: 12px;
  }

  .box {
    background: white;
    border: 1px solid #aaa;
    padding: 12px;
    flex: 1 1 280px;
  }

  .box h2 {
    font-size: 15px;
    color: #3366cc;
    border-bottom: 1px solid #3366cc;
    padding-bottom: 4px;
    margin-bottom: 10px;
  }

  p { font-size: 14px; margin: 5px 0; }

  .big  { font-size: 26px; font-weight: bold; margin: 2px 0; }
  .unit { font-size: 12px; color: #666; margin-bottom: 8px; }

  button {
    background: #3366cc;
    color: white;
    border: none;
    padding: 7px 11px;
    margin: 3px 2px;
    font-size: 13px;
    cursor: pointer;
  }
  button.green  { background: #2e7d32; }
  button.grey   { background: #777; }
  button.purple { background: #6a1b9a; }

  input[type=tel], input[type=text] {
    padding: 6px;
    border: 1px solid #aaa;
    font-size: 13px;
    width: 160px;
    max-width: 100%;
  }

  input[type=range] { width: 100%; margin-top: 5px; }

  .section { margin-top: 10px; padding-top: 8px; border-top: 1px solid #ddd; }
  .lbl { font-size: 12px; color: #666; }
</style>

<script>
let draggingSlider = false;

function updateData() {
  fetch('/data')
    .then(r => r.json())
    .then(d => {
      document.getElementById("hum").innerText  = d.humidity;
      document.getElementById("temp").innerText = d.temperature;
      document.getElementById("gas").innerText  = d.gas;
      document.getElementById("dist").innerText = d.distance;

      document.getElementById("mode").innerText             = d.mode;
      document.getElementById("quiet").innerText            = d.quiet;
      document.getElementById("occupied").innerText         = d.occupied;
      document.getElementById("occupancyEnabled").innerText = d.occupancyEnabled;
      document.getElementById("runon").innerText            = d.runon;
      document.getElementById("runonEnabled").innerText     = d.runonEnabled;
      document.getElementById("fan").innerText              = d.fan;
      document.getElementById("smsEnabled").innerText       = d.smsEnabled;
      document.getElementById("alertNumber").innerText      = d.alertNumber;

      const modeBtn = document.getElementById("modeToggle");
      if (modeBtn) modeBtn.innerText = d.mode === "AUTO" ? "Switch to Manual" : "Switch to Auto";

      const smsBtn = document.getElementById("smsToggle");
      if (smsBtn) {
        smsBtn.innerText = d.smsEnabled ? "SMS ON" : "SMS OFF";
        smsBtn.className = d.smsEnabled ? "green" : "grey";
      }

      const autoCtrl = document.getElementById("autoControls");
      if (autoCtrl) autoCtrl.style.display = d.mode === "AUTO" ? "block" : "none";

      const wrap = document.getElementById("sliderWrap");
      if (wrap) wrap.style.display = d.mode === "MANUAL" ? "block" : "none";

      const s = document.getElementById("fanSlider");
      const v = document.getElementById("fanSliderVal");
      if (s && v && !draggingSlider) { s.value = d.fan; v.innerText = d.fan; }
    });
}

setInterval(updateData, 1000);
window.onload = updateData;

function sendCmd(c) {
  fetch('/cmd?cmd=' + c).then(() => updateData());
}
function sendSpeed(val) {
  document.getElementById("fanSliderVal").innerText = val;
  fetch('/cmd?cmd=speed' + val);
}
function toggleMode() {
  const cur = document.getElementById("mode").innerText;
  sendCmd(cur === "AUTO" ? "mode manual" : "mode auto");
}
function toggleSMS() {
  const on = document.getElementById("smsEnabled").innerText === "true";
  sendCmd(on ? "sms_disable" : "sms_enable");
}
function setAlertNumber() {
  const num = document.getElementById("alertInput").value.trim();
  if (num) sendCmd('setnum:' + num);
}
</script>
</head>
<body>

<h1>Smart Bathroom Fan</h1>

<div class="grid">

<div class="box">
<h2>Status</h2>
)====";

String homePageSensors = R"====(
</div>

<div class="box">
<h2>Sensors</h2>
)====";

String homePageControls = R"====(
</div>

<div class="box">
<h2>Controls</h2>

<button id="modeToggle" onclick="toggleMode()">Switch to Manual</button>
<button id="smsToggle" class="green" onclick="toggleSMS()">SMS ON</button>

<div id="autoControls" style="display:none" class="section">
  <button class="purple" onclick="sendCmd('occupancy_enable')">Occupancy ON</button>
  <button class="grey"   onclick="sendCmd('occupancy_disable')">Occupancy OFF</button>
  <br>
  <button class="purple" onclick="sendCmd('runon_enable')">Run-On ON</button>
  <button class="grey"   onclick="sendCmd('runon_disable')">Run-On OFF</button>
</div>

<div class="section">
  <p class="lbl">Alert Number: <span id="alertNumber"></span></p>
  <input id="alertInput" type="tel" placeholder="+353876937035">
  <button onclick="setAlertNumber()">Set</button>
</div>

<div id="sliderWrap" style="display:none" class="section">
  <p class="lbl">Fan Speed: <strong><span id="fanSliderVal">40</span></strong></p>
  <input id="fanSlider" type="range" min="40" max="255" value="40"
    onmousedown="draggingSlider=true" onmouseup="draggingSlider=false"
    ontouchstart="draggingSlider=true" ontouchend="draggingSlider=false"
    oninput="sendSpeed(this.value)">
</div>

)====";

String homePageFooter = R"====(
</div>

<div class="box"><h2>Humidity</h2><iframe width="100%" height="260" style="border:none;" src="https://thingspeak.com/channels/3341317/charts/1?bgcolor=%23ffffff&color=%233366cc&dynamic=true&results=60&type=line&title=Humidity+%25"></iframe></div>
<div class="box"><h2>Temperature</h2><iframe width="100%" height="260" style="border:none;" src="https://thingspeak.com/channels/3341317/charts/2?bgcolor=%23ffffff&color=%23cc3333&dynamic=true&results=60&type=line&title=Temperature+%C2%B0C"></iframe></div>
<div class="box"><h2>CO Level</h2><iframe width="100%" height="260" style="border:none;" src="https://thingspeak.com/channels/3341317/charts/3?bgcolor=%23ffffff&color=%23cc7700&dynamic=true&results=60&type=line&title=Gas"></iframe></div>

</div>
</body>
</html>
)====";