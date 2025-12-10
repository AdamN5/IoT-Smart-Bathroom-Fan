String homePageHeader = R"====(
<!DOCTYPE html>
<html>
<head>
<title>Smart Bathroom Fan</title>
<meta charset="UTF-8">
<style>
  body { font-family: Arial; background:#f3f3f3; padding:20px; }
  .box { background:white; padding:15px; border-radius:6px; margin-bottom:15px; }
  button { padding:10px; margin:5px; border:none; border-radius:4px; background:#2196f3; color:white; }
  button:hover { background:#0b7dda; }
</style>

<script>
function updateData() {
  fetch('/data')
    .then(r => r.json())
    .then(d => {
      document.getElementById("hum").innerText = d.humidity;
      document.getElementById("temp").innerText = d.temperature;
      document.getElementById("gas").innerText = d.gas;
      document.getElementById("fan").innerText = d.fan;
      document.getElementById("mode").innerText = d.mode;
      document.getElementById("quiet").innerText = d.quiet;
    });
}

setInterval(updateData, 1000);
window.onload = updateData;

function sendCmd(c) {
  fetch('/cmd?cmd=' + c)
    .then(r => r.text())
    .then(t => { updateData(); });
}
</script>

</head>
<body>

<h1>Smart Bathroom Fan</h1>

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

<button onclick="sendCmd('mode auto')">Auto Mode</button>
<button onclick="sendCmd('mode manual')">Manual Mode</button>
<br>

<button onclick="sendCmd('mode quiet_on')">Quiet Mode ON</button>
<button onclick="sendCmd('mode quiet_off')">Quiet Mode OFF</button>
<br><br>

<button onclick="sendCmd('speed255')">Fan Full</button>
<button onclick="sendCmd('speed128')">Fan Medium</button>
<button onclick="sendCmd('speed0')">Fan Off</button>

)====";

String homePageFooter = R"====(
</div>

</body>
</html>
)====";
