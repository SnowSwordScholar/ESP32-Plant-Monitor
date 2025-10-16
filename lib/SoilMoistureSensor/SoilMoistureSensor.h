#pragma once

#include <Arduino.h>

struct SoilMoistureReading {
    uint16_t raw;
    float voltage;
    float percent;
};

class SoilMoistureSensor {
public:
    SoilMoistureSensor(uint8_t adcPin, float wetVoltage = 0.0F, float dryVoltage = 3.3F);
    void begin() const;
    SoilMoistureReading sample() const;
    void setCalibration(float wetVoltage, float dryVoltage);
    float rawToVoltage(uint16_t raw) const;
    float voltageToPercent(float voltage) const;
    uint8_t pin() const { return _pin; }

private:
    uint8_t _pin;
    float _wetVoltage;
    float _dryVoltage;
};
