#ifndef TIMEREADER_H
#define TIMEREADER_H
#include <ThreeWire.h>  
#include <RtcDS1302.h>

class TimeReader {
public:
    int currentHour = 0;
    int currentMinute = 0;
    String currentDay = "";

    void printDateTime(const RtcDateTime& dt);
    void initialize();
    void doLoop();
    void parseDate(int dateIndex, String incomingData);
};

#endif // TIMEREADER_H