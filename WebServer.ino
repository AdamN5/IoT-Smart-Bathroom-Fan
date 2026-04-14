#include <WiFi.h>
#include <WebServer.h>
#include "Homepage.h"
#include "secrets.h"

// external variables from webserver
extern float temperatureC;
extern float humidity;
extern int mq7Raw;
extern int fanDuty;
extern bool autoMode;
extern bool quietMode;
extern bool runOnMode;
extern bool runOnEnabled;
extern bool occupied;
extern bool occupancyEnabled;
extern long distanceCM;

extern const int pwmChannel;

void sendSMS(String message);
extern String alertNumber;
extern bool   smsEnabled;

WebServer server(80);

// wifi credentials loaded from secrets.h
const char* ssid     = WIFI_SSID;
const char* password = WIFI_PASSWORD;

// data

void handleData() {
  String json = "{";
  json += "\"humidity\":" + String(humidity, 1) + ",";
  json += "\"temperature\":" + String(temperatureC, 1) + ",";
  json += "\"gas\":" + String(mq7Raw) + ",";
  json += "\"fan\":" + String(fanDuty) + ",";
  json += "\"mode\":\"" + String(autoMode ? "AUTO" : "MANUAL") + "\",";
  json += "\"quiet\":\"" + String(quietMode ? "ON" : "OFF") + "\",";
  json += "\"runon\":" + String(runOnMode ? "true" : "false") + ",";
  json += "\"runonEnabled\":" + String(runOnEnabled ? "true" : "false") + ",";
  json += "\"occupied\":" + String(occupied ? "true" : "false") + ",";
  json += "\"occupancyEnabled\":" + String(occupancyEnabled ? "true" : "false") + ",";
  json += "\"distance\":" + String(distanceCM) + ",";
  json += "\"alertNumber\":\"" + alertNumber + "\",";
  json += "\"smsEnabled\":" + String(smsEnabled ? "true" : "false");
  json += "}";

  server.send(200, "application/json", json);
}

// main webpage
void handleRoot() {
  String page = homePageHeader;

  page += "<p><b>Mode:</b> <span id='mode'></span></p>";
  page += "<p><b>Quiet Mode:</b> <span id='quiet'></span></p>";
  page += "<p><b>Occupied:</b> <span id='occupied'></span> (enabled: <span id='occupancyEnabled'></span>)</p>";
  page += "<p><b>Run-On:</b> <span id='runon'></span> (enabled: <span id='runonEnabled'></span>)</p>";
  page += "<p><b>Fan Duty:</b> <span id='fan'></span></p>";
  page += "<p><b>SMS Alerts:</b> <span id='smsEnabled'></span></p>";

  page += homePageSensors;

  page += "<div class='big' id='hum'>--</div><div class='unit'>Humidity %</div>";
  page += "<div class='big' id='temp'>--</div><div class='unit'>Temperature °C</div>";
  page += "<div class='big' id='gas'>--</div><div class='unit'>CO Gas</div>";
  page += "<div class='big' id='dist'>--</div><div class='unit'>Distance cm</div>";


  page += homePageControls;
  page += homePageFooter;

  server.send(200, "text/html", page);
}

// cmd handler
// This function handles commands sent from the web interface
void handleCommand() {

  if (!server.hasArg("cmd")) {
    server.send(400, "text/plain", "Missing cmd");
    return;
  }

  String cmd = server.arg("cmd");

  // auto and manual modes
  if (cmd == "mode auto") {
    autoMode = true;
    server.send(200, "text/plain", "AUTO MODE");
    return;
  }
  if (cmd == "mode manual") {
    autoMode = false;
    server.send(200, "text/plain", "MANUAL MODE");
    return;
  }

  // quiet mode
  if (cmd == "mode quiet_on") {
    quietMode = true;
    server.send(200, "text/plain", "QUIET MODE ON");
    return;
  }
  if (cmd == "mode quiet_off") {
    quietMode = false;
    server.send(200, "text/plain", "QUIET MODE OFF");
    return;
  }

  if (cmd == "occupancy_enable") {
    occupancyEnabled = true;
    server.send(200, "text/plain", "OCCUPANCY ENABLED");
    return;
  }
  if (cmd == "occupancy_disable") {
    occupancyEnabled = false;
    occupied         = false;  // clear current occupied state
    server.send(200, "text/plain", "OCCUPANCY DISABLED");
    return;
  }

  if (cmd == "runon_enable") {
    runOnEnabled = true;
    server.send(200, "text/plain", "RUN-ON ENABLED");
    return;
  }
  if (cmd == "runon_disable") {
    runOnEnabled = false;
    runOnMode    = false;  // cancel any active run on
    server.send(200, "text/plain", "RUN-ON DISABLED");
    return;
  }

  if (cmd.startsWith("setnum:")) {
    alertNumber = cmd.substring(7);
    server.send(200, "text/plain", "NUMBER SET: " + alertNumber);
    return;
  }

  if (cmd == "sms_enable") {
    smsEnabled = true;
    server.send(200, "text/plain", "SMS ENABLED");
    return;
  }
  if (cmd == "sms_disable") {
    smsEnabled = false;
    server.send(200, "text/plain", "SMS DISABLED");
    return;
  }

  // manual fan speed control
  // This sets the fan speed directly, ignoring auto mode
  if (cmd.startsWith("speed")) {
    if (autoMode) {
      server.send(200, "text/plain", "IGNORED — AUTO MODE");
      return;
    }

    int value = cmd.substring(5).toInt();
    fanDuty = constrain(value, 0, 255);
    ledcWrite(pwmChannel, fanDuty);

    server.send(200, "text/plain", "SPEED SET");
    return;
  }

  server.send(200, "text/plain", "OK");
}

// 404 handler
// This function handles requests for non-existent pages
void handleNotFound() {
  server.send(404, "text/plain", "Not Found");
}

// wifi and server setup
// This function initializes the WiFi connection and starts the web server
void webserver_setup() {

  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(300);
    Serial.print(".");
  }

  Serial.print("\nConnected: ");
  Serial.println(WiFi.localIP());

  server.on("/", handleRoot);
  server.on("/cmd", handleCommand);
  server.on("/data", handleData);
  server.onNotFound(handleNotFound);

  server.begin();
  Serial.println("HTTP server started");
}