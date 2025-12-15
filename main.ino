#include <WebServer.h>
#include <Wire.h>
#include <Adafruit_BME280.h>

void webserver_setup();
extern WebServer server;

// pin setup
const int pwmPin       = 25;
const int pwmChannel   = 0;
const int pwmFreq      = 25000;
const int pwmResBits   = 8;

const int mq7Pin       = 34;


// sensors
Adafruit_BME280 bme;

// HC SR04
const int trigPin = 5;
const int echoPin = 18;
long distanceCM = 0;

// data
float temperatureC = 0.0;
float humidity     = 0.0;
int   mq7Raw       = 0;
int   fanDuty      = 0;

bool autoMode  = false;
bool quietMode = false;

// SETUP
void setup() {
  Serial.begin(115200);

  // PWM
  ledcSetup(pwmChannel, pwmFreq, pwmResBits);
  ledcAttachPin(pwmPin, pwmChannel);
  ledcWrite(pwmChannel, 0);

  // BME280 using i2c to communicate with esp
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

long readDistance() {
  digitalWrite(trigPin, LOW);
  delayMicroseconds(2);

  digitalWrite(trigPin, HIGH);
  delayMicroseconds(10);
  digitalWrite(trigPin, LOW);

  long duration = pulseIn(echoPin, HIGH, 30000);
  long dist = duration * 0.0343 / 2;

  if (dist == 0 || dist > 400) return -1;
  return dist;
}

void readSensors() {
  mq7Raw       = analogRead(mq7Pin);
  temperatureC = bme.readTemperature();
  humidity     = bme.readHumidity();
  distanceCM   = readDistance();

  // AUTO MODE
  if (autoMode) {
    if (humidity > 75) {
      fanDuty = (quietMode ? 128 : 255);
    }
    else if (humidity < 60) {
      fanDuty = 0;
    }

    // OCCUPANCY QUIET MODE
if (distanceCM > 0 && distanceCM < 130) {
  quietMode = true;
} 
else {
  quietMode = false;
}

    ledcWrite(pwmChannel, fanDuty);
  }
}

// Main Loop
void loop() {
  readSensors();
  server.handleClient();
}