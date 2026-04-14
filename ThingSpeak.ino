#include <HTTPClient.h>

const char* TS_WRITE_KEY = "TGUWTT89ZDIGN86I";
const unsigned long TS_INTERVAL_MS = 20000UL;  // 20 sec — free tier minimum is 15s

unsigned long lastTSUpdate = 0 - TS_INTERVAL_MS - 1UL;  // fire immediately on boot

extern float temperatureC;
extern float humidity;
extern int   mq7Raw;

void sendToThingSpeak() {
  unsigned long now = millis();
  if (now - lastTSUpdate < TS_INTERVAL_MS) return;
  lastTSUpdate = now;

  HTTPClient http;
  String url = "http://api.thingspeak.com/update?api_key=";
  url += TS_WRITE_KEY;
  url += "&field1=" + String(humidity, 1);
  url += "&field2=" + String(temperatureC, 1);
  url += "&field3=" + String(mq7Raw);

  http.begin(url);
  int code = http.GET();
  Serial.println("ThingSpeak update: " + String(code));
  http.end();
}
