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

    void activateSchedule();
    void deactivateSchedule();
    void checkSchedule(int currentHour, int currentMinute);
    void parseSchedule(String incomingData);
    void getPWM(String incomingData);
    void getScheduleTime(String incomingData);
};

#endif // SCHEDULE_H