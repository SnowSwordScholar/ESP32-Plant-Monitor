#include "SoilMoistureSensor.h"

namespace {
    constexpr float kAdcReferenceVoltage = 3.3F;
    constexpr uint16_t kAdcResolution = 4095U;
}

SoilMoistureSensor::SoilMoistureSensor(uint8_t adcPin, float wetVoltage, float dryVoltage)
    : _pin(adcPin), _wetVoltage(wetVoltage), _dryVoltage(dryVoltage) {
    if (_wetVoltage > _dryVoltage) {
        const float tmp = _wetVoltage;
        _wetVoltage = _dryVoltage;
        _dryVoltage = tmp;
    }
}

void SoilMoistureSensor::begin() const {
    pinMode(_pin, INPUT);
#ifdef ARDUINO_ARCH_ESP32
    analogReadResolution(12);
    analogSetPinAttenuation(_pin, ADC_11db);
#endif
}

SoilMoistureReading SoilMoistureSensor::sample() const {
    const uint16_t raw = analogRead(_pin);
    const float voltage = rawToVoltage(raw);
    const float percent = voltageToPercent(voltage);
    return {raw, voltage, percent};
}

void SoilMoistureSensor::setCalibration(float wetVoltage, float dryVoltage) {
    _wetVoltage = min(wetVoltage, dryVoltage);
    _dryVoltage = max(wetVoltage, dryVoltage);
}

float SoilMoistureSensor::rawToVoltage(uint16_t raw) const {
    return (static_cast<float>(raw) / static_cast<float>(kAdcResolution)) * kAdcReferenceVoltage;
}

float SoilMoistureSensor::voltageToPercent(float voltage) const {
    const float constrainedWet = min(_wetVoltage, _dryVoltage);
    const float constrainedDry = max(_wetVoltage, _dryVoltage);
    const float clamped = constrain(voltage, constrainedWet, constrainedDry);
    const float span = constrainedDry - constrainedWet;
    if (span <= 0.0F) {
        return 0.0F;
    }
    const float dryness = (clamped - constrainedWet) / span;
    const float moisture = 1.0F - dryness;
    return moisture * 100.0F;
}
