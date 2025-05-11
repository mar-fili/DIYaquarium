#include "DS18B20.h"

DS18B20::DS18B20(uint8_t pin)
    : _pin(pin), _oneWire(pin), _sensor(&_oneWire) {}

void DS18B20::begin() {
    _sensor.begin();
}

float DS18B20::readTemperatureC() {
    _sensor.requestTemperatures();

    return _sensor.getTempCByIndex(0);
}