#include <TranzistorControl.h>
#include <EspCommunication.h>
#include <Schedule.h>
#include <TimeReader.h>

enum DataType {
  DATE,
  SCHEDULE,
  RAW_LED_DATA,
  NONE,
};

TimeReader timeReader;
EspCommunication esp;
TranzistorControl tranzistorControl;
Schedule schedule;

Schedule allSchedules[10];
int scheduleCount = 0;
unsigned long lastCheck = 0;
const unsigned long interval = 60000;
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
  } else if (incomingData.indexOf("Schedule") != -1) {
    return SCHEDULE;
  } else if (incomingData.indexOf("Set") != -1) {
    return RAW_LED_DATA;
  }
}

void loop() {
  unsigned long now = correctedMillis();

  if (now - lastCheck >= interval) {
    lastCheck = now;

    timeReader.updateTime();

    for (int i = 0; i < scheduleCount; i++) {
      allSchedules[i].checkForSchedule(timeReader.currentHour, timeReader.currentMinute);
      //turn on harmonogram?? = bool Schedule::checkIfShouldTurnOn()
    }
  }

  while (esp.espSerial.available()) {
    char c = esp.espSerial.read();
    esp.incomingData += c;
    
    if (esp.incomingData.endsWith("OK")) {

      DataType dataType = getDataType(esp.incomingData);

      if (dataType == RAW_LED_DATA || dataType == SCHEDULE) {
        schedule.getPWM(esp.incomingData);
      }
      
      switch (dataType) {
        case DATE:
          timeReader.parseDate(esp.incomingData.indexOf("Date") + 5, esp.incomingData);
          break;
        case RAW_LED_DATA:
          Serial.println("Raw LED data received.");
          //tranzistorControl.turnOnLED(schedule.pwm);
          break;
        case SCHEDULE:
          schedule.getScheduleTime(esp.incomingData);
          Schedule newSchedule = schedule;
          allSchedules[scheduleCount] = newSchedule;
          scheduleCount++;

          Serial.print("Dodano harmonogram: ");
          Serial.print(schedule.startHour);
          Serial.print(":");
          Serial.print(schedule.startMinute);
          Serial.print(" - ");
          Serial.print(schedule.endHour);
          Serial.print(":");
          Serial.println(schedule.endMinute);
          for (int i = 0; i<4;i++){
            Serial.print("PWM: ");
            Serial.print(schedule.pwm[i]);
            Serial.print(" ");
          }
          break;
      }
      esp.sendHTTPResponse();
      esp.closeConnection();
      esp.incomingData = "";
      dataType = NONE;
    }
  }
}