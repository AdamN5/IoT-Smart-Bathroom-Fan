#include <WiFi.h>
#include <WebServer.h>
#include "Homepage.h"

// external variables from webserver
extern float temperatureC;
extern float humidity;
extern int mq7Raw;
extern int fanDuty;
extern bool autoMode;
extern bool quietMode;

extern const int pwmChannel;

WebServer server(80);

// wifi name and password
const char* ssid     = "12";
const char* password = "helloapple12";

// data

void handleData() {
  String json = "{";
  json += "\"humidity\":" + String(humidity, 1) + ",";
  json += "\"temperature\":" + String(temperatureC, 1) + ",";
  json += "\"gas\":" + String(mq7Raw) + ",";
  json += "\"fan\":" + String(fanDuty) + ",";
  json += "\"mode\":\"" + String(autoMode ? "AUTO" : "MANUAL") + "\",";
  json += "\"quiet\":\"" + String(quietMode ? "ON" : "OFF") + "\",";
  json += "\"distance\":" + String(distanceCM);
  json += "}";

  server.send(200, "application/json", json);
}

// main webpage 
void handleRoot() {
  String page = homePageHeader;

  page += "<p><strong>Mode:</strong> <span id='mode'></span></p>";
  page += "<p><strong>Quiet Mode:</strong> <span id='quiet'></span></p>";
  page += "<p><strong>Fan Duty:</strong> <span id='fan'></span></p>";

  page += homePageSensors;

  page += "<p><strong>Humidity:</strong> <span id='hum'></span> %</p>";
  page += "<p><strong>Temperature:</strong> <span id='temp'></span> °C</p>";
  page += "<p><strong>Gas:</strong> <span id='gas'></span></p>";
  page += "<p><strong>Distance:</strong> <span id='dist'></span> cm</p>";


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