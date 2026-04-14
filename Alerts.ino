#include <HTTPClient.h>
#include <WiFiClientSecure.h>
#include "secrets.h"

// Twilio credentials (values loaded from secrets.h)
const char* TWILIO_SID   = TWILIO_ACCOUNT_SID;
const char* TWILIO_TOKEN = TWILIO_AUTH_TOKEN;
const char* TWILIO_FROM  = TWILIO_FROM_NUMBER;
String alertNumber = TWILIO_TO_NUMBER;
bool   smsEnabled  = true;

// Alert thresholds
const int   GAS_ALERT_THRESHOLD      = 2000;
const float HUMIDITY_ALERT_THRESHOLD = 70.0;

// 10 min cooldown per alert type
const unsigned long ALERT_COOLDOWN_MS = 10UL * 60UL * 1000UL;

// initialised so the cooldown is already expired at boot
unsigned long lastGasAlert      = 0 - ALERT_COOLDOWN_MS - 1UL;
unsigned long lastHumidityAlert = 0 - ALERT_COOLDOWN_MS - 1UL;

// external sensor values from main.ino
extern int   mq7Raw;
extern float humidity;

void sendSMS(String message) {
  WiFiClientSecure client;
  client.setInsecure();  // skip SSL cert check

  HTTPClient http;

  String url = "https://api.twilio.com/2010-04-01/Accounts/";
  url += TWILIO_SID;
  url += "/Messages.json";

  http.begin(client, url);
  http.setAuthorization(TWILIO_SID, TWILIO_TOKEN);
  http.addHeader("Content-Type", "application/x-www-form-urlencoded");

  String body = "From=" + String(TWILIO_FROM)
              + "&To="   + alertNumber
              + "&Body=" + message;

  int code = http.POST(body);
  Serial.println("SMS HTTP code: " + String(code));
  Serial.println("SMS response: " + http.getString());
  http.end();
}

void checkAlerts() {
  if (!smsEnabled) return;

  unsigned long now = millis();

  if (mq7Raw > GAS_ALERT_THRESHOLD && now - lastGasAlert > ALERT_COOLDOWN_MS) {
    lastGasAlert = now;
    sendSMS("ALERT: Gas level high! Reading: " + String(mq7Raw));
  }

  if (humidity > HUMIDITY_ALERT_THRESHOLD && now - lastHumidityAlert > ALERT_COOLDOWN_MS) {
    lastHumidityAlert = now;
    sendSMS("ALERT: Humidity too high! Reading: " + String(humidity, 1) + "%");
  }
}
