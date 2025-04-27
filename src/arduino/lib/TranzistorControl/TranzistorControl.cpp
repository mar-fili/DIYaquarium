#include "Arduino.h"
#include "TranzistorControl.h"

void TranzistorControl::turnOnLED(int pwm[4]) {
    for (int i = 0; i < 4; i++) {
        if (pwm[i] == 100) {
          digitalWrite(this -> transistorPins[i], HIGH);
        } else if (pwm[i] == 0) {
          digitalWrite(this -> transistorPins[i], LOW);
        } else {
          int pwmValue = 255 * pwm[i] / 100;
          analogWrite(this -> transistorPins[i], pwmValue);
        }
      }
}

void TranzistorControl::turnOffLed(int pwm[4]) {
    for (int i = 0; i < 4; i++) {
        if (pwm[i] > 0) {
            digitalWrite(this -> transistorPins[i], LOW);
        }
    }
}