// Schedule.cpp
#include "Schedule.h"
#include <Arduino.h>

Schedule::ScheduleNode* Schedule::parseSchedule(const String& incomingData, String color) {
    int pinNumber;
    if (color == "b") {
        pinNumber = 6;
    } else if (color == "r") {
        pinNumber = 9;
    } else if (color == "wCold") {
        pinNumber = 5;
    } else if (color == "wWarm") {
        pinNumber = 3;
    } else {
        return nullptr;
    }
    int startIndex = incomingData.indexOf(color + "=");
    int endIndex = incomingData.indexOf("&", startIndex);
    
    if (startIndex == -1 || endIndex == -1) return nullptr;

    String values;
    if (color == "b" || color == "r") {
        values = incomingData.substring(startIndex + 2, endIndex);
    } else {
        values = incomingData.substring(startIndex + 6, endIndex);
    }
    

    ScheduleNode* head = nullptr;
    ScheduleNode* tail = nullptr;

    int lastPos = 0;
    while (true) {
        int semicolonPos = values.indexOf(';', lastPos);
        String entry = (semicolonPos == -1) ? values.substring(lastPos) : values.substring(lastPos, semicolonPos);
        if (entry.length() == 0) break;

        int commaPos = entry.indexOf(',');
        if (commaPos == -1) break;

        String time = entry.substring(0, commaPos);
        int pwm = entry.substring(commaPos + 1).toInt();
        pwm = round((float)pwm * 2.55);
        pwm = constrain(pwm, 0, 255);

        int colonPos = time.indexOf(':');
        if (colonPos == -1) break;

        int hour = time.substring(0, colonPos).toInt();
        int minute = time.substring(colonPos + 1).toInt();

        if (!head) {
            ScheduleNode* newNode = new ScheduleNode{hour, minute, pwm,0, 0, 0, pinNumber,nullptr};
            head = newNode;
            tail = newNode;
        } else {
            float minutes = (hour * 60 + minute) - (tail->hour * 60 + tail->minute); 
            float deltaPwmPM = 0.0f;

            if (minutes != 0) {
                deltaPwmPM = (float)(pwm - tail->pwm) / minutes;
            }
            ScheduleNode* newNode = new ScheduleNode{hour, minute, pwm,0, deltaPwmPM, 0, pinNumber, nullptr};
            tail->next = newNode;
            tail = newNode;
        }

        if (semicolonPos == -1) break;
        lastPos = semicolonPos + 1;
    }

    return head;
}

void Schedule::updatePWM(int currentHour, int currentMinute, ScheduleNode* headTab[4]) {
    for (int i = 0; i < 4; i++) {
        ScheduleNode* current = headTab[i];
        if (current == nullptr) continue;

        int currentTimeInMinutes = currentHour * 60 + currentMinute;
        while (current->next != nullptr) {
            int nodeStart = current->hour * 60 + current->minute;
            int nodeEnd = current->next->hour * 60 + current->next->minute;

            if (currentTimeInMinutes >= nodeStart && currentTimeInMinutes <= nodeEnd) {
                int elapsedMinutes = currentTimeInMinutes - nodeStart;
                int computedPWM = (int)(current->pwm + current->next->deltaPwmPM * elapsedMinutes);
                computedPWM = constrain(computedPWM, 0, 255);
                if (computedPWM != current->updatedPWM) {
                    current->updatedPWM = computedPWM;
                    Serial.print("Aktualizacja PWM: ");
                    Serial.print(current->updatedPWM);
                    Serial.print(" dla koloru: ");
                    Serial.println(i == 0 ? "b" : i == 1 ? "r" : i == 2 ? "wCold" : "wWarm");
                    Serial.print("Pin: ");
                    Serial.println(current->pinNumber);
                    Serial.print("Godzina: ");
                    Serial.print(current->hour);
                    Serial.print(":");
                    Serial.print(current->minute);
                    if (current -> updatedPWM == 0) {
                        digitalWrite(current->pinNumber, LOW);
                    } else {
                        analogWrite(current->pinNumber, current->updatedPWM);
                    }
                }
            }

            current = current->next;
        }
    }
}

void Schedule::checkForSchedule(int currentHour, int currentMinute, ScheduleNode* headTab[4]) {
    for (int i = 0; i < 4; i++) {
        ScheduleNode* current = headTab[i];
        if (current == nullptr) continue;

        int currentTimeInMinutes = currentHour * 60 + currentMinute;
        while (current->next != nullptr) {
            int start = current->hour * 60 + current->minute;
            int end = current->next->hour * 60 + current->next->minute;

            if (currentTimeInMinutes >= start && currentTimeInMinutes < end) {
                Serial.print("Aktywny harmonogram od ");
                Serial.print(current->hour);
                Serial.print(":");
                Serial.print(current->minute);
                Serial.print(" do ");
                Serial.print(current->next->hour);
                Serial.print(":");
                Serial.println(current->next->minute);
                Serial.println("PWM(analog-write-value): ");
                Serial.print("PWM to analog write: " + String(current->updatedPWM));
                Serial.println("Basic PWM: " + String(current->pwm));
                break;
            }

            current = current->next;
        }
    }
}
