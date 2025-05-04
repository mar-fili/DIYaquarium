// Schedule.cpp
#include "Schedule.h"
#include <Arduino.h>

Schedule::Schedule() : head(nullptr) {}

Schedule::~Schedule() {
    freeScheduleList();  // Zwolnienie pamięci przy usuwaniu obiektu
}

Schedule::ScheduleNode* Schedule::parseBlueSchedule(const String& incomingData) {
    int startIndex = incomingData.indexOf("b=");
    int endIndex = incomingData.indexOf("&", startIndex);
    if (startIndex == -1 || endIndex == -1) return nullptr;

    String values = incomingData.substring(startIndex + 2, endIndex);  // "b=" to 2 znaki

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

        int colonPos = time.indexOf(':');
        if (colonPos == -1) break;

        int hour = time.substring(0, colonPos).toInt();
        int minute = time.substring(colonPos + 1).toInt();

        if (!head) {
            ScheduleNode* newNode = new ScheduleNode{hour, minute, pwm,0, 0, 0, 3,nullptr};
            head = newNode;
            tail = newNode;
        } else {
            float minutes = (hour * 60 + minute) - (tail -> hour * 60 + tail -> minute); 
            float deltaPwmPM = ((float)(pwm - tail->pwm) / minutes) * 255.0f / 100.0f;
            ScheduleNode* newNode = new ScheduleNode{hour, minute, pwm,0, deltaPwmPM, 0, 3, nullptr};
            tail->next = newNode;
            tail = newNode;
        }

        if (semicolonPos == -1) break;
        lastPos = semicolonPos + 1;
    }

    return head;
}

void Schedule::freeScheduleList() {
    while (head != nullptr) {
        ScheduleNode* tmp = head;
        head = head->next;
        delete tmp;
    }
}

Schedule::ScheduleNode* Schedule::getScheduleHead() {
    return head;
}

void Schedule::updatePWM(int currentHour, int currentMinute, ScheduleNode* head) {
    if (head == nullptr || head->next == nullptr) return;

    ScheduleNode* current = head;
    int currentTimeInMinutes = currentHour * 60 + currentMinute;
    
    while (current->next != nullptr) {
        int nodeStart = current->hour * 60 + current->minute;
        int nodeEnd = current->next->hour * 60 + current->next->minute;

        if (currentTimeInMinutes >= nodeStart && currentTimeInMinutes < nodeEnd) {
            int elapsedMinutes = currentTimeInMinutes - nodeStart;

            current->updatedPWM = (int)(current->pwm + current->next->deltaPwmPM * elapsedMinutes);
            current->updatedPWM = constrain(current->updatedPWM, 0, 255);
        }

        current = current->next;
    }

    if (current->next == nullptr) {
        int nodeStart = current->hour * 60 + current->minute;
        if (currentTimeInMinutes > nodeStart) {
            int elapsedMinutes = currentTimeInMinutes - nodeStart;
            current->updatedPWM = (int)(current->pwm + current->deltaPwmPM * elapsedMinutes);
            current->updatedPWM = constrain(current->updatedPWM, 0, 255);
        }
    }
}

void Schedule::checkForSchedule(int currentHour, int currentMinute, ScheduleNode* head) {
    if (head == nullptr || head->next == nullptr) return;

    int currentTimeInMinutes = currentHour * 60 + currentMinute;
    ScheduleNode* current = head;

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
            Serial.println("Basic PWM: " + String(current->pwm ));

            break;
        }

        current = current->next;
    }
}
