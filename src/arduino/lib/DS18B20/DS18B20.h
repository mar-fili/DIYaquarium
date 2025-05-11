#ifndef DS18B20_H
#define DS18B20_H

#include <OneWire.h>
#include <DallasTemperature.h>

class DS18B20 {
public:
    explicit DS18B20(uint8_t pin);

    void begin();
    float readTemperatureC();

private:
    uint8_t _pin;
    OneWire _oneWire;
    DallasTemperature _sensor;
};

#endif // DS18B20_H