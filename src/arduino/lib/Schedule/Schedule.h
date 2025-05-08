// Schedule.h
#ifndef SCHEDULE_H
#define SCHEDULE_H

#include <Arduino.h>

class Schedule {
public:
    struct ScheduleNode {
        int hour;
        int minute;
        int pwm;
        int updatedPWM;
        float deltaPwmPM;
        float PWMCorrectionFactor;
        int pinNumber;
        ScheduleNode* next;
    };

    ScheduleNode* parseSchedule(const String& incomingData, String color);
    void freeScheduleList();
    void updatePWM(int currentHour, int currentMinute, ScheduleNode* headTab[4]);
    void checkForSchedule(int currentHour, int currentMinute, ScheduleNode* headTab[4]);
    ScheduleNode* getScheduleHead();
};
#endif