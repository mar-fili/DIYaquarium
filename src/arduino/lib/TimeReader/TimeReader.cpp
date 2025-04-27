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

    if (!Rtc.IsDateTimeValid()) {
        Serial.println("RTC lost confidence in the DateTime!");
    }

    if (Rtc.GetIsWriteProtected()) {
        Serial.print("RTC is write protected");
    }

    if (!Rtc.GetIsRunning()) {
        Serial.print("RTC is NOT running");
    }

    updateTime();
}

void TimeReader::showTime() {
    this -> now = Rtc.GetDateTime();
    Serial.print("DS1302 RTC DateTime: ");
    printDateTime(now);
    Serial.println();
}

void TimeReader::parseDate(int dateIndex, String incomingData) {
    int spaceIndex = incomingData.indexOf(' ', dateIndex);
    this -> currentDay = incomingData.substring(dateIndex, spaceIndex);
    String time = incomingData.substring(spaceIndex + 1, spaceIndex + 5);
    this -> currentHour = time.substring(0, 2).toInt();
    this -> currentMinute = time.substring(2, 4).toInt();
}

void TimeReader::updateTime() {
    this -> now = Rtc.GetDateTime();
    this -> currentHour = now.Hour();
    this -> currentMinute = now.Minute();
    // int month = now.Day();
    // int day = now.Month();
    // int year = now.Year();

    // char Date[11];
    // sprintf(Date, "%02u/%02u/%04u", month, day, year);
    // this -> currentDay = String(Date);

    // Serial.println("Zaktualizowany czas: ");
    // Serial.print(this -> currentDay);
    // Serial.print(" ");
//     Serial.print(this -> currentHour);
//     Serial.print(":");
//     Serial.print(this -> currentMinute);
//     Serial.println();
// }
}