#include <Arduino.h>
#include <WiFi.h>
#include <ArduinoHA.h>
#include "DHT.h"

// WiFi Configuration
#define WIFI_SSID       "550W"
#define WIFI_PASSWORD   "12345678"

// MQTT Broker Configuration (Home Assistant IP Address)
#define BROKER_ADDR     IPAddress(10,210,235,170)  // Change to your HA server IP
#define BROKER_PORT     1883

// DHT Sensor Configuration
#define DHTPIN 23
#define DHTTYPE DHT11

// LED Configuration (Optional, for status indication)
#define LED_PIN 2

DHT dht(DHTPIN, DHTTYPE);
WiFiClient client;
HADevice device;
HAMqtt mqtt(client, device, BROKER_PORT);

// Home Assistant Devices
HASensor temperature("plant_temp");
HASensor humidity("plant_humidity");
HASwitch led("plant_led");

// Timer Variables
unsigned long lastSensorRead = 0;
const unsigned long sensorInterval = 30000; // Read sensor every 30 seconds

void onSwitchCommand(bool state, HASwitch* sender) {
    digitalWrite(LED_PIN, state ? HIGH : LOW);
    sender->setState(state);
    Serial.println(state ? "LED ON" : "LED OFF");
}

void setup() {
    Serial.begin(115200);
    Serial.println("Starting Plant Monitor System...");

    // Initialize LED pin
    pinMode(LED_PIN, OUTPUT);
    digitalWrite(LED_PIN, LOW);

    // Initialize DHT sensor
    dht.begin();

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
    device.setName("Plant Monitor");
    device.setSoftwareVersion("1.0.0");
    device.setManufacturer("DIY");
    device.setModel("ESP32-Plant-Monitor");

    // Configure temperature sensor
    temperature.setDeviceClass("temperature");
    temperature.setName("Plant Temperature");
    temperature.setUnitOfMeasurement("°C");
    temperature.setIcon("mdi:thermometer");

    // Configure humidity sensor
    humidity.setDeviceClass("humidity");
    humidity.setName("Plant Humidity");
    humidity.setUnitOfMeasurement("%");
    humidity.setIcon("mdi:water-percent");

    // Configure LED switch
    led.setName("Status LED");
    led.setIcon("mdi:led-on");
    led.onCommand(onSwitchCommand);

    // Connect to MQTT (with authentication)
    mqtt.begin(BROKER_ADDR, "mqtt-user", "your-mqtt-password");
    Serial.println("MQTT connection initialized");
}

void loop() {
    mqtt.loop();

    // Read sensor data periodically
    if (millis() - lastSensorRead >= sensorInterval) {
        lastSensorRead = millis();
        
        float h = dht.readHumidity();
        float t = dht.readTemperature();

        if (!isnan(h) && !isnan(t)) {
            // Send data to Home Assistant
            temperature.setValue(String(t, 2).c_str());
            humidity.setValue(String(h, 2).c_str());
            
            Serial.printf("Temperature: %.2f°C, Humidity: %.2f%%\n", t, h);
            

        } else {
            Serial.println("Failed to read from DHT sensor!");
        }
    }

    delay(100);
}
