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

    Schedule();
    ~Schedule();

    ScheduleNode* parseBlueSchedule(const String& incomingData);
    void freeScheduleList();
    void updatePWM(int currentHour, int currentMinute, ScheduleNode* head);
    void checkForSchedule(int currentHour, int currentMinute, ScheduleNode* head);
    ScheduleNode* getScheduleHead();

private:
    ScheduleNode* head;
};

#endif // SCHEDULE_H