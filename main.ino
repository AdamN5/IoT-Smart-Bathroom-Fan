#include <Arduino.h>
#include <Adafruit_BME280.h>
#include <Wire.h>

#define PIN_TRIG      5
#define PIN_ECHO      18
#define PIN_MQ7_ADC   34
#define PIN_RELAY     26

Adafruit_BME280 bme;

const bool relayActiveLow = false;

void fanSet(bool on) {
  digitalWrite(PIN_RELAY, (relayActiveLow ? !on : on));
}
bool fanGet() {
  int s = digitalRead(PIN_RELAY);
  return relayActiveLow ? (s == LOW) : (s == HIGH);
}

enum Mode { AUTO_HUMID = 1, OCCUPANCY = 2, GAS_SAFETY = 3, MANUAL = 4 };
Mode mode = AUTO_HUMID;

float     rhSetpoint      = 65.0;
float     rhHyst          = 5.0;
uint16_t  occDistanceCm   = 120;
uint32_t  occBoostMs      = 3UL * 60UL * 1000UL;
uint16_t  mq7ThresholdRaw = 1800;

uint32_t  lastOccTriggerMs = 0;

long readDistanceCm() {
  digitalWrite(PIN_TRIG, LOW); delayMicroseconds(3);
  digitalWrite(PIN_TRIG, HIGH); delayMicroseconds(10);
  digitalWrite(PIN_TRIG, LOW);
  unsigned long us = pulseIn(PIN_ECHO, HIGH, 30000UL);
  if (!us) return 9999;
  return us / 58;
}

void printStatus(float h, float t, long dist, int mqRaw) {
  Serial.printf("[Mode %d] RH=%.1f%%  T=%.1f°C  Dist=%ldcm  MQ7=%d  Fan:%s\n",
                mode, h, t, dist, mqRaw, fanGet() ? "ON" : "OFF");
}

void setup() {
  Serial.begin(115200);

  pinMode(PIN_TRIG, OUTPUT);
  pinMode(PIN_ECHO, INPUT);
  pinMode(PIN_RELAY, OUTPUT);
  fanSet(false);

  Wire.begin();
  if (!bme.begin(0x76)) {
    Serial.println("BME280 not found!");
    while (1) delay(100);
  }

  Serial.println("ESP32 Smart Fan");
  Serial.println("Commands: mode 1|2|3|4, on, off, set rh <val>, set mq <val>");
}

void loop() {

  float h = bme.readHumidity();
  float t = bme.readTemperature();

  long dist = readDistanceCm();
  int  mqRaw = analogRead(PIN_MQ7_ADC);

  bool wantFan = fanGet();

  switch (mode) {
    case AUTO_HUMID:
      if (!isnan(h)) {
        if (h >= rhSetpoint)            wantFan = true;
        if (h <= rhSetpoint - rhHyst)   wantFan = false;
      }
      break;

    case OCCUPANCY:
      if (dist < occDistanceCm) {
        lastOccTriggerMs = millis();
        wantFan = true;
      }
      if (millis() - lastOccTriggerMs > occBoostMs) wantFan = false;
      break;

    case GAS_SAFETY:
      wantFan = (mqRaw >= mq7ThresholdRaw);
      break;

    case MANUAL:
      break;
  }

  if (mqRaw > mq7ThresholdRaw + 600) wantFan = true;

  fanSet(wantFan);

  static uint32_t lastPrint = 0;
  if (millis() - lastPrint > 1000) {
    printStatus(h, t, dist, mqRaw);
    lastPrint = millis();
  }

  if (Serial.available()) {
    String s = Serial.readStringUntil('\n');
    s.trim();

    if (s.startsWith("mode")) {
      int m = s.substring(5).toInt();
      if (m >= 1 && m <= 4) { mode = (Mode)m; Serial.printf("Mode set to %d\n", m); }

    } else if (s == "on")  { mode = MANUAL; fanSet(true);  Serial.println("Manual ON"); }
    else if (s == "off")   { mode = MANUAL; fanSet(false); Serial.println("Manual OFF"); }
    else if (s.startsWith("set rh ")) { rhSetpoint = s.substring(7).toFloat(); Serial.printf("RH setpoint=%.1f\n", rhSetpoint); }
    else if (s.startsWith("set mq ")) { mq7ThresholdRaw = s.substring(7).toInt(); Serial.printf("MQ7 threshold=%d\n", mq7ThresholdRaw); }
  }

  delay(20);
}
