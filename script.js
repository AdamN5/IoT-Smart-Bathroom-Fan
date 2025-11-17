// Mode names
const modes = {
    1: "Auto Humidity",
    2: "Occupancy",
    3: "Gas Safety",
    4: "Manual"
};

function updateData(data) {
    document.getElementById('humidity').textContent = data.humidity.toFixed(1);
    document.getElementById('temperature').textContent = data.temperature.toFixed(1);
    document.getElementById('distance').textContent = data.distance;
    document.getElementById('gas').textContent = data.gas;
    document.getElementById('mode').textContent = modes[data.mode] || data.mode;
    document.getElementById('fan-status').textContent = data.fan ? 'ON' : 'OFF';
}

function sendCommand(cmd) {
    console.log('Command:', cmd);
}

function setMode(m) {
    sendCommand('mode ' + m);
}

function fanOn() {
    sendCommand('on');
}

function fanOff() {
    sendCommand('off');
}

// to do later get it to send commands to esp32
