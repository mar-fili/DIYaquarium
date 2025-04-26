#ifndef TIMEREADER_H
#define TIMEREADER_H
#include <ThreeWire.h>  
#include <RtcDS1302.h>

class TimeReader {
public:
    int currentHour = 0;
    int currentMinute = 0;
    String currentDay = "";
    RtcDateTime now;

    void printDateTime(const RtcDateTime& dt);
    void initialize();
    void parseDate(int dateIndex, String incomingData);
    void showTime();
    void updateTime();
};

#endif // TIMEREADER_H