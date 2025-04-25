#include "TimeReader.h"

ThreeWire myWire(12,13,2);
RtcDS1302<ThreeWire> Rtc(myWire);

#define countof(a) (sizeof(a) / sizeof(a[0]))

void TimeReader::printDateTime(const RtcDateTime& dt) {
   char datestring[20];
   
   snprintf_P(datestring, 
    countof(datestring),
    PSTR("%02u/%02u/%04u %02u:%02u:%02u"),
    dt.Month(),
    dt.Day(),
    dt.Year(),
    dt.Hour(),
    dt.Minute(),
    dt.Second() );
    Serial.print(datestring);
}

void TimeReader::initialize() {
    Rtc.Begin();

    RtcDateTime compiled = RtcDateTime(__DATE__, __TIME__);
    printDateTime(compiled);
    Serial.println();

    if (!Rtc.IsDateTimeValid()) 
    {
        // Common Causes:
        //    1) first time you ran and the device wasn't running yet
        //    2) the battery on the device is low or even missing

        Serial.println("RTC lost confidence in the DateTime!");
        Rtc.SetDateTime(compiled);
    }

    if (Rtc.GetIsWriteProtected())
    {
        Serial.println("RTC was write protected, enabling writing now");
        Rtc.SetIsWriteProtected(false);
    }

    if (!Rtc.GetIsRunning())
    {
        Serial.println("RTC was not actively running, starting now");
        Rtc.SetIsRunning(true);
    }

    RtcDateTime now = Rtc.GetDateTime();
    if (now < compiled) 
    {
        Serial.println("RTC is older than compile time!  (Updating DateTime)");
        Rtc.SetDateTime(compiled);
    }
    else if (now > compiled) 
    {
        Serial.println("RTC is newer than compile time. (this is expected)");
    }
    else if (now == compiled) 
    {
        Serial.println("RTC is the same as compile time! (not expected but all is fine)");
    }
}

void TimeReader::doLoop() {
    RtcDateTime now = Rtc.GetDateTime();

    printDateTime(now);
    Serial.println();

    if (!now.IsValid())
    {
        // Common Causes:
        //    1) the battery on the device is low or even missing and the power line was disconnected
        Serial.println("RTC lost confidence in the DateTime!");
    }

    delay(50000); // five seconds
}

void TimeReader::parseDate(int dateIndex, String incomingData) {
    int spaceIndex = incomingData.indexOf(' ', dateIndex);
    this -> currentDay = incomingData.substring(dateIndex, spaceIndex);
    String time = incomingData.substring(spaceIndex + 1, spaceIndex + 5);
    this -> currentHour = time.substring(0, 2).toInt();
    this -> currentMinute = time.substring(2, 4).toInt();
}