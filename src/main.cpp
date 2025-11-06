#include <Arduino.h>
#include <WiFi.h>
#include <ArduinoHA.h>
#include <cstdio>
#include "SoilMoistureSensor.h"

// Config: credentials and broker settings
#if defined(__has_include)
#  if __has_include("config.h")
#    include "config.h"
#  endif
#endif

// Fallback defaults (used when src/config.h is not present)
#ifndef WIFI_SSID
#define WIFI_SSID       "YOUR_WIFI_SSID"
#endif
#ifndef WIFI_PASSWORD
#define WIFI_PASSWORD   "YOUR_WIFI_PASSWORD"
#endif

#ifndef BROKER_ADDR
#define BROKER_ADDR     IPAddress(192,168,1,100)
#endif
#ifndef BROKER_PORT
#define BROKER_PORT     1883
#endif
#ifndef BROKER_USER
#define BROKER_USER     "mqtt_user"
#endif
#ifndef BROKER_PASS
#define BROKER_PASS     "mqtt_password"
#endif

// Soil moisture sensor configuration
constexpr uint8_t SOIL_ADC_PIN = 34;

// LED Configuration (Optional, for status indication)
#define LED_PIN 2
// Water pump control pin (GPIO32)
#define PUMP_PIN 32

SoilMoistureSensor soilSensor(SOIL_ADC_PIN);
WiFiClient client;
HADevice device;
HAMqtt mqtt(client, device, BROKER_PORT);

// Home Assistant Devices
HASensor soilMoisture("plant_soil_moisture");
HASwitch led("plant_led");
HASwitch pump("plant_pump");

// Timer Variables
unsigned long lastSensorRead = 0;
const unsigned long sensorInterval = 1000; // Read sensor every 1 second

void onSwitchCommand(bool state, HASwitch* sender) {
    digitalWrite(LED_PIN, state ? HIGH : LOW);
    sender->setState(state);
    Serial.println(state ? "LED ON" : "LED OFF");
}

void onPumpCommand(bool state, HASwitch* sender) {
    // HIGH = pump on, LOW = pump off
    digitalWrite(PUMP_PIN, state ? LOW : HIGH);
    sender->setState(state);
    Serial.println(state ? "Pump ON" : "Pump OFF");
}

void setup() {
    Serial.begin(115200);
    Serial.println("Starting Plant Monitor System...");

    // Initialize LED pin
    pinMode(LED_PIN, OUTPUT);
    digitalWrite(LED_PIN, LOW);

    // Initialize pump control pin (GPIO32)
    pinMode(PUMP_PIN, OUTPUT);
    digitalWrite(PUMP_PIN, HIGH); // ensure pump is off at boot

    // Initialize soil moisture sensor
    soilSensor.begin();

    // Connect to WiFi
    WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
    Serial.print("Connecting to WiFi");
    while (WiFi.status() != WL_CONNECTED) {
        delay(500);
        Serial.print(".");
    }
    Serial.println();
    Serial.println("WiFi connected successfully!");
    Serial.print("IP Address: ");
    Serial.println(WiFi.localIP());

    // Set device unique ID
    byte mac[6];
    WiFi.macAddress(mac);
    device.setUniqueId(mac, sizeof(mac));

    // Set device information
    device.setName("Group1 Plant Monitor");
    device.setSoftwareVersion("1.0.1");
    device.setManufacturer("Group1");
    device.setModel("ESP32-Plant-Monitor");

    // Configure soil moisture sensor entity
    soilMoisture.setDeviceClass("moisture");
    soilMoisture.setName("Plant Soil Moisture");
    soilMoisture.setUnitOfMeasurement("%");
    soilMoisture.setIcon("mdi:water");

    // Configure LED switch
    led.setName("Status LED");
    led.setIcon("mdi:led-on");
    led.onCommand(onSwitchCommand);

    // Configure pump switch (exposed to Home Assistant for manual control)
    pump.setName("Water Pump");
    pump.setIcon("mdi:water-pump");
    pump.onCommand(onPumpCommand);

    // Connect to MQTT (with authentication)
    mqtt.begin(BROKER_ADDR, BROKER_USER, BROKER_PASS);
    Serial.println("MQTT connection initialized");
}

void loop() {
    mqtt.loop();

    // Read sensor data periodically
    if (millis() - lastSensorRead >= sensorInterval) {
        lastSensorRead = millis();
        
    const SoilMoistureReading reading = soilSensor.sample();
    char moisturePayload[8];
    snprintf(moisturePayload, sizeof(moisturePayload), "%.1f", reading.percent);
    soilMoisture.setValue(moisturePayload);

    Serial.printf("Soil moisture: %.2f%% (raw %u, %.3f V)\n", reading.percent, reading.raw, reading.voltage);
    }

    delay(100);
}
