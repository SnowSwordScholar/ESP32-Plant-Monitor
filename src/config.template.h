// Copy this file to `src/config.h` and fill in your credentials.
// The real `src/config.h` should be gitignored.

#pragma once

// WiFi
#define WIFI_SSID       "YOUR_WIFI_SSID"
#define WIFI_PASSWORD   "YOUR_WIFI_PASSWORD"

// MQTT Broker - if using an IP address, use the IPAddress macro as in the example below
#define BROKER_ADDR     IPAddress(192,168,1,100)
#define BROKER_PORT     1883
#define BROKER_USER     "mqtt_user"
#define BROKER_PASS     "mqtt_password"

// Optional: device identification
//#define DEVICE_NAME "My Plant Monitor"
