#ifndef SCHEDULE_H
#define SCHEDULE_H

class Schedule {
public:
    int pwm[4]; // PWM values for each transistor
    int startHour;
    int startMinute;
    int endHour;
    int endMinute;
    bool isActive = false; // Indicates if the schedule is currently active

    void getPWM(String incomingData);
    void getScheduleTime(String incomingData);
    void checkForSchedule(int currentHour, int currentMinute);
};

#endif // SCHEDULE_H