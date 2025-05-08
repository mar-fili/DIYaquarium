#include <TranzistorControl.h>
#include <EspCommunication.h>
#include <Schedule.h>
#include <TimeReader.h>

enum DataType {
  DATE,
  SCHEDULE,
  SCHED,
  RAW_LED_DATA,
  NONE,
};

TimeReader timeReader;
EspCommunication esp;
TranzistorControl tranzistorControl;
Schedule schedule;
Schedule::ScheduleNode* headTab [4] = {nullptr, nullptr, nullptr, nullptr};

Schedule allSchedules[10];
int scheduleCount = 0;
unsigned long lastCheck = 0;
const unsigned long interval = 20000;
const float millisCorrectionFactor = 1.0 / 64.0;
unsigned long correctedMillis() {
  return millis() * millisCorrectionFactor;
}

void setup() {
  for (int i = 0; i < sizeof(tranzistorControl.transistorPins) / sizeof(tranzistorControl.transistorPins[0]); i++) {
    pinMode(tranzistorControl.transistorPins[i], OUTPUT);
    digitalWrite(tranzistorControl.transistorPins[i], LOW);
  }

  Serial.begin(9600);
  delay(2000);
  esp.initialize();

  Serial.println("Zmieniam timery");
  TCCR0B = TCCR0B & B11111000 | B00000001;
  TCCR1B = TCCR1B & B11111000 | B00000001;
  TCCR2B = TCCR2B & B11111000 | B00000001;

  timeReader.initialize();

}

//---------------------------------------------- Set PWM frequency for D5 & D6 -------------------------------

//TCCR0B = TCCR0B & B11111000 | B00000001;    // set timer 0 divisor to     1 for PWM frequency of 62500.00 Hz
//TCCR0B = TCCR0B & B11111000 | B00000010;    // set timer 0 divisor to     8 for PWM frequency of  7812.50 Hz
//TCCR0B = TCCR0B & B11111000 | B00000011;    // set timer 0 divisor to    64 for PWM frequency of   976.56 Hz (The DEFAULT)
//TCCR0B = TCCR0B & B11111000 | B00000100;    // set timer 0 divisor to   256 for PWM frequency of   244.14 Hz
//TCCR0B = TCCR0B & B11111000 | B00000101;    // set timer 0 divisor to  1024 for PWM frequency of    61.04 Hz


//---------------------------------------------- Set PWM frequency for D9 & D10 ------------------------------

//TCCR1B = TCCR1B & B11111000 | B00000001;    // set timer 1 divisor to     1 for PWM frequency of 31372.55 Hz
//TCCR1B = TCCR1B & B11111000 | B00000010;    // set timer 1 divisor to     8 for PWM frequency of  3921.16 Hz
//TCCR1B = TCCR1B & B11111000 | B00000011;    // set timer 1 divisor to    64 for PWM frequency of   490.20 Hz (The DEFAULT)
//TCCR1B = TCCR1B & B11111000 | B00000100;    // set timer 1 divisor to   256 for PWM frequency of   122.55 Hz
//TCCR1B = TCCR1B & B11111000 | B00000101;    // set timer 1 divisor to  1024 for PWM frequency of    30.64 Hz

//---------------------------------------------- Set PWM frequency for D3 & D11 ------------------------------

//TCCR2B = TCCR2B & B11111000 | B00000001;    // set timer 2 divisor to     1 for PWM frequency of 31372.55 Hz
//TCCR2B = TCCR2B & B11111000 | B00000010;    // set timer 2 divisor to     8 for PWM frequency of  3921.16 Hz
//TCCR2B = TCCR2B & B11111000 | B00000011;    // set timer 2 divisor to    32 for PWM frequency of   980.39 Hz
//TCCR2B = TCCR2B & B11111000 | B00000100;    // set timer 2 divisor to    64 for PWM frequency of   490.20 Hz (The DEFAULT)
//TCCR2B = TCCR2B & B11111000 | B00000101;    // set timer 2 divisor to   128 for PWM frequency of   245.10 Hz
//TCCR2B = TCCR2B & B11111000 | B00000110;    // set timer 2 divisor to   256 for PWM frequency of   122.55 Hz
//TCCR2B = TCCR2B & B11111000 | B00000111;    // set timer 2 divisor to  1024 for PWM frequency of    30.64 Hz

DataType getDataType(String incomingData) {
  if (incomingData.indexOf("Date") != -1) {
    return DATE;
  } else if (incomingData.indexOf("SCHED") != -1) {
    return SCHED;
  } else if (incomingData.indexOf("Schedule") != -1) {
    return SCHEDULE;
  } else if (incomingData.indexOf("Set") != -1) {
    return RAW_LED_DATA;
  }
  else {
    return NONE;
  }
}

void loop() {
  unsigned long now = correctedMillis();

  if (now - lastCheck >= interval) {
    lastCheck = now;

    timeReader.updateTime();
    Serial.print("Aktualny czas: ");
    Serial.print(timeReader.currentHour);
    Serial.print(":");
    Serial.println(timeReader.currentMinute);
    // for (int i = 0; i < scheduleCount; i++) {
    //   allSchedules[i].checkForSchedule(timeReader.currentHour, timeReader.currentMinute, );
    // }
    
    schedule.updatePWM(timeReader.currentHour, timeReader.currentMinute, headTab);
    //schedule.checkForSchedule(timeReader.currentHour, timeReader.currentMinute, headTab);
    
  }
  while (esp.espSerial.available()) {
    char c = esp.espSerial.read();
    esp.incomingData += c;

    if (esp.incomingData.endsWith("OK")) {
      DataType dataType = getDataType(esp.incomingData);
      Serial.println("Data type: " + String(dataType));
      switch (dataType) {
        case SCHED:
          Serial.println("SCHED data received.");
          if (esp.incomingData.indexOf("&b=") != -1) {
            headTab[0] = schedule.parseSchedule(esp.incomingData, "b");
          }
          if (esp.incomingData.indexOf("?r=") != -1) {
            headTab[1] = schedule.parseSchedule(esp.incomingData, "r");
          }
          if (esp.incomingData.indexOf("&wCold=") != -1) {
            headTab[2] = schedule.parseSchedule(esp.incomingData, "wCold");
          }
          if (esp.incomingData.indexOf("&wWarm=") != -1) {
            headTab[3] = schedule.parseSchedule(esp.incomingData, "wWarm");
          }
          
          for (int i = 0; i < 4; i++) {
            if (headTab[i] != nullptr) {
              Serial.print("Harmonogram dla koloru: ");
              Serial.println(i == 0 ? "b" : i == 1 ? "r" : i == 2 ? "wCold" : "wWarm");
              Schedule::ScheduleNode* current = headTab[i];
              while (current != nullptr) {
                current->pwm *= 255 / 100;
                current->pwm = constrain(current->pwm, 0, 255);
                Serial.print("Godzina: ");
                Serial.print(current->hour);
                Serial.print(":");
                Serial.print(current->minute);
                Serial.print(" → PWM: ");
                Serial.println(current->pwm);
                current = current->next;
              }
            } else {
              Serial.println("Brak harmonogramu dla koloru.");
            }
          }
      }

      Serial.println("Otrzymano dane: " + esp.incomingData);
      esp.sendHTTPResponse();
      esp.closeConnection();
      esp.incomingData = "";
      dataType = NONE;
    }
  }
}