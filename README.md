# Smart Bathroom Fan

This is my 2nd year IoT project for my Bachelor of Software & Electronic Engineering at Atlantic Technical University Galway.

The idea is a smart bathroom fan that automatically controls itself based on sensors rather than being always on or manually switched. It runs on an ESP32 and has a web interface you can access from your phone or laptop.

## What it does

- Detects when someone is in the bathroom using an ultrasonic sensor and turns the fan on
- Monitors humidity and CO levels using a BME280 and MQ-7 sensor
- Keeps the fan running for 5 minutes after you leave (run-on timer)
- Sends you an SMS alert if CO or humidity gets too high (using Twilio)
- Has a web page hosted on the ESP32 where you can see live sensor readings and control the fan
- Logs sensor data to ThingSpeak and shows live graphs on the web page
- Has different modes - Auto, Manual, Eco, Quiet

## Hardware used

- ESP32 WROOM-32E
- BME280 (temperature and humidity)
- MQ-7 (carbon monoxide)
- HC-SR04 (ultrasonic distance sensor for occupancy)
- IRLZ44N MOSFET (to control the 12V fan)
- 1N4007 Flyback Diode
- 12V DC Extractor Fan
- 1k and 2k resistors (voltage divider on ECHO pin)

## How to set it up

1. Clone the repo
2. Rename secrets_h.example to secrets.h
3. Add your own WiFi name, password and Twilio details to secrets.h
4. Open in Arduino IDE and upload to your ESP32
5. Check the Serial Monitor for the IP address
6. Type the IP address into your browser and the web page should load

## Files

- main.ino - main loop and sensor reading
- WebServer.ino - handles the web server and all commands
- ThingSpeak.ino - sends data to ThingSpeak every 20 seconds
- Alerts.ino - checks sensor values and sends SMS if needed
- Homepage.h - all the HTML, CSS and JavaScript for the web page
- secrets.h - your credentials (not uploaded to GitHub for security)

## Built with

- Arduino IDE
- C/C++
- Adafruit BME280 library
- Twilio API
- ThingSpeak

## Author

Adam Nafea - ATU Galway 2025/2026
