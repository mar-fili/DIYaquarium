#include "Arduino.h"
#include "Schedule.h"
#include "TranzistorControl.h"

void Schedule::parseSchedule(String incomingData) {
    int getIndex = incomingData.indexOf("GET");
    getIndex += 5;

    for (int i = 0; i < 4; i++) {
      this -> pwm[i] = incomingData.substring(getIndex, getIndex + 4).toInt();
      getIndex += 4;
    }
    Serial.print("PWM: ");
    for (int i = 0; i < 4; i++) {
      Serial.print(this -> pwm[i]); Serial.print(" ");
    }
    Serial.println();
    String scheduleTime = incomingData.substring(getIndex, getIndex + 10);
    Serial.print("Schedule time len: ");
    Serial.println(scheduleTime.length());

    if (scheduleTime.length() < 10) {
        TranzistorControl tc;
        tc.turnOnLED(this -> pwm);
    }
}

void Schedule::getPWM(String incomingData) {
    int slashIndex = incomingData.indexOf('/');
    String dataPart = incomingData.substring(slashIndex);

    int v1, v2, v3, v4;
    if (sscanf(dataPart.c_str(), "%*s %d %d %d %d", &v1, &v2, &v3, &v4) == 4) {
        this -> pwm[0] = v1;
        this -> pwm[1] = v2;
        this -> pwm[2] = v3;
        this -> pwm[3] = v4;
    } else {
        Serial.println("Error parsing PWM values.");
    }
}

void Schedule::getScheduleTime(String incomingData) {
    int dashIndex = incomingData.indexOf('-');
    if (dashIndex != -1) {
        int startIndex = dashIndex - 1;
        while (startIndex > 0 && isDigit(incomingData.charAt(startIndex - 1))) {
            startIndex--;
        }

        String startStr = incomingData.substring(startIndex, dashIndex);
        String endStr = incomingData.substring(dashIndex + 1);

        if (startStr.length() > 0 && endStr.length() > 0 && isDigit(startStr.charAt(0)) && isDigit(endStr.charAt(0))) {
            int startInt = startStr.toInt();
            int endInt = endStr.toInt();

            this->startHour = startInt / 100;
            this->startMinute = startInt % 100;
            this->endHour = endInt / 100;
            this->endMinute = endInt % 100;
        }
    }
}
