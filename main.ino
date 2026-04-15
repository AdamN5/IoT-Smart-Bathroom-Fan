#include <WebServer.h>
#include <Wire.h>
#include <Adafruit_BME280.h>

void webserver_setup();
void checkAlerts();
void sendToThingSpeak();
extern WebServer server;

// pin setup
const int pwmPin       = 25;
const int pwmChannel   = 0;
const int pwmFreq      = 25000;
const int pwmResBits   = 8;

const int mq7Pin       = 34;

// const int ModeLEDPin   = 27; // Not used

// sensors
Adafruit_BME280 bme;

// HC SR04
const int trigPin = 5;
const int echoPin = 18;
long distanceCM = 0;

// MQ-7 PPM conversion
// Adjust R0 to calibrate, measure in clean air and tweak until you get ~5-10 ppm
const float MQ7_RL  = 10.0;   // load resistor in kΩ (check your circuit)
const float MQ7_R0  = 10.0;   // sensor resistance in clean air (kΩ) — calibrate this
const float MQ7_VCC = 3.3;    // ESP32 ADC reference voltage

float calculatePPM(int raw) {
  if (raw <= 0) return 0;
  float voltage = (raw / 4095.0) * MQ7_VCC;
  if (voltage <= 0) return 0;
  float rs  = ((MQ7_VCC * MQ7_RL) / voltage) - MQ7_RL;
  float ratio = rs / MQ7_R0;
  return 99.042 * pow(ratio, -1.518);  // MQ-7 CO curve
}

// data
float temperatureC = 0.0;
float humidity     = 0.0;
int   mq7Raw       = 0;
float coPPM        = 0.0;
int   fanDuty      = 0;

bool autoMode         = true;
bool quietMode        = false;
bool ecoMode          = false;
bool runOnMode        = false;
bool runOnEnabled     = true;
bool occupied         = false;
bool occupancyEnabled = true;

unsigned long runOnStartTime  = 0;
const unsigned long RUN_ON_MS = 5UL * 60UL * 1000UL;  // 5 minutes

bool prevOccupied = false;

// SETUP
void setup() {
  Serial.begin(115200);

  // PWM
  ledcSetup(pwmChannel, pwmFreq, pwmResBits);
  ledcAttachPin(pwmPin, pwmChannel);
  ledcWrite(pwmChannel, 0);

  // BME280
  Wire.begin(21, 22);
  if (!bme.begin(0x76)) {
    bme.begin(0x77);
  }

  // HC SR04
  pinMode(trigPin, OUTPUT);
  pinMode(echoPin, INPUT);

  // setup website and server
  webserver_setup();
}

// Sensor reads
long lastValidDistance = -1;

long readDistance() {
  long readings[3];
  int  validCount = 0;

  for (int i = 0; i < 3; i++) {
    digitalWrite(trigPin, LOW);
    delayMicroseconds(2);
    digitalWrite(trigPin, HIGH);
    delayMicroseconds(10);
    digitalWrite(trigPin, LOW);

    long duration = pulseIn(echoPin, HIGH, 30000);
    long dist = duration * 0.0343 / 2;

    if (dist > 0 && dist <= 400) {
      readings[validCount++] = dist;
    }
    delay(10);
  }

  if (validCount == 0) return lastValidDistance;  // hold last good reading

  long sum = 0;
  for (int i = 0; i < validCount; i++) sum += readings[i];
  lastValidDistance = sum / validCount;
  return lastValidDistance;
}

void readSensors() {
  mq7Raw       = analogRead(mq7Pin);
  coPPM        = calculatePPM(mq7Raw);
  temperatureC = bme.readTemperature();
  humidity     = bme.readHumidity();
  distanceCM   = readDistance();

  // AUTO MODE
  if (autoMode) {
    occupied = occupancyEnabled && (distanceCM > 0 && distanceCM <= 10);

    if (occupied) {
      runOnMode  = false;
      fanDuty    = quietMode ? 40 : (ecoMode ? 100 : 255);  // quiet wins if both on
    } else if (prevOccupied && !runOnMode && runOnEnabled) {
      // start 5 min run on
      runOnMode      = true;
      runOnStartTime = millis();
      fanDuty        = ecoMode ? 100 : 255;  // quiet doesn't apply to run-on
    } else if (runOnMode) {
      if (millis() - runOnStartTime >= RUN_ON_MS) {
        // run on expired
        runOnMode = false;
        fanDuty   = 0;
      } else {
        fanDuty   = ecoMode ? 100 : 255;  // quiet doesn't apply to run-on
      }
    } else {
      fanDuty   = 0;
    }

    prevOccupied = occupied;

    // Humidity override — eco caps at 100, otherwise full speed
    if (humidity > 70) {
      fanDuty = ecoMode ? 100 : 255;
    }

    ledcWrite(pwmChannel, fanDuty);
  }
}

// Main Loop
void loop() {
  readSensors();
  checkAlerts();
  sendToThingSpeak();
  server.handleClient();
}
