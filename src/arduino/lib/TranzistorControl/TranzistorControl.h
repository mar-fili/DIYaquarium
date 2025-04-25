#ifndef TRANZISTOR_CONTROL_H
#define TRANZISTOR_CONTROL_H

class TranzistorControl {
public:
    int transistorPins[4] = {3,5,6,9}; // Array of transistor control pins
    void turnOnLED(int pwm[4]);
};
#endif // TRANZISTOR_CONTROL_H