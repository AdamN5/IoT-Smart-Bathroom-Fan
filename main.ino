#include <Arduino.h>
#include <DFRobot_DHT11.h>

//                      PIN SETUP 
#define PIN_DHT      17
#define PIN_TRIG     5
#define PIN_ECHO     18
#define PIN_MQ7_ADC  34
#define PIN_RELAY    26   // SIG1

DFRobot_DHT11 dht;

// Relay is ACTIVE-HIGH 
const bool relayActiveLow = false;

void fanSet(bool on) {
  digitalWrite(PIN_RELAY, (relayActiveLow ? !on : on));
}
bool fanGet() {
  int s = digitalRead(PIN_RELAY);
  return relayActiveLow ? (s == LOW) : (s == HIGH);
}

  //                Modes
enum Mode { AUTO_HUMID = 1, OCCUPANCY = 2, GAS_SAFETY = 3, MANUAL = 4 };
Mode mode = AUTO_HUMID;

  //                Settings
float     rhSetpoint      = 65.0;            // %Relative Humidity
float     rhHyst          = 5.0;             // %Relative Humidity hysteresis
uint16_t  occDistanceCm   = 120;             // Distance in cm for occupancy
uint32_t  occBoostMs      = 3UL * 60UL * 1000UL; // 3 minutes boost after occupancy
uint16_t  mq7ThresholdRaw = 1800;            // MQ7 ADC threshold

  //                State
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
  float espVolts = (mqRaw * 3.3f) / 4095.0f;
  float aoVolts  = espVolts / 0.4f; // 10k/15k divider -> 0.4 ratio to ESP
  Serial.printf("[Mode %d] RH=%.1f%%  T=%.1f°C  Dist=%ldcm  MQ7=%d (ESP=%.2fV AO≈%.2fV)  Fan:%s\n",
                mode, h, t, dist, mqRaw, espVolts, aoVolts, fanGet() ? "ON" : "OFF");
}

//                  Setup
void setup() {
  Serial.begin(115200);

  pinMode(PIN_TRIG, OUTPUT);
  pinMode(PIN_ECHO, INPUT);
  pinMode(PIN_RELAY, OUTPUT);

  // Make sure fan is OFF at start
  fanSet(false);

  Serial.println("ESP32 Smart Fan");
  Serial.println("Commands: mode 1|2|3|4, on, off, set rh <val>, set mq <val>");
}

//                  Main Loop
void loop() {
  dht.read(PIN_DHT);
  float h = dht.humidity;
  float t = dht.temperature;

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

  // safety incase gas is too high
  if (mqRaw > mq7ThresholdRaw + 600) wantFan = true;

  fanSet(wantFan);

  static uint32_t lastPrint = 0;
  if (millis() - lastPrint > 1000) {
    printStatus(h, t, dist, mqRaw);
    lastPrint = millis();
  }

  if (Serial.available()) {
    String s = Serial.readStringUntil('\n'); s.trim();
    if (s.startsWith("mode")) {
      int m = s.substring(5).toInt();
      if (m >= 1 && m <= 4) { mode = (Mode)m; Serial.printf("Mode set to %d\n", m); }
    } else if (s == "on")  { mode = MANUAL; fanSet(true);  Serial.println("Manual ON"); }
      else if (s == "off") { mode = MANUAL; fanSet(false); Serial.println("Manual OFF"); }
      else if (s.startsWith("set rh ")) { rhSetpoint = s.substring(7).toFloat(); Serial.printf("RH setpoint=%.1f\n", rhSetpoint); }
      else if (s.startsWith("set mq ")) { mq7ThresholdRaw = s.substring(7).toInt(); Serial.printf("MQ7 threshold=%d\n", mq7ThresholdRaw); }
  }

  delay(20);
}