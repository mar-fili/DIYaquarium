#ifndef TRANZISTOR_CONTROL_H
#define TRANZISTOR_CONTROL_H

class TranzistorControl {
public:
    int transistorPins[4] = {3,5,6,9}; // Array of transistor control pins
    void turnOnLED(int pwm[4]); // Method to turn on the LED with PWM values
    void turnOffLed(int pwm[4]); // Method to turn off the LED
};
#endif // TRANZISTOR_CONTROL_H