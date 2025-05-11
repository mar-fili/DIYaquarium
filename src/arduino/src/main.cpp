#include <TranzistorControl.h>
#include <EspCommunication.h>
#include <Schedule.h>
#include <TimeReader.h>
#include <DS18B20.h>

enum DataType {
  DATE,
  SCHED,
  TEMP,
  NONE,
};

DS18B20 sensor(7);
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
  sensor.begin();
  Serial.println("Czujnik temperatury uruchomiony");

  Serial.println("Zmieniam timery");
  TCCR0B = TCCR0B & B11111000 | B00000001;
  TCCR1B = TCCR1B & B11111000 | B00000001;
  TCCR2B = TCCR2B & B11111000 | B00000001;

  timeReader.initialize();

}

DataType getDataType(String incomingData) {
  if (incomingData.indexOf("Date") != -1) {
    return DATE;
  } else if (incomingData.indexOf("SCHED") != -1) {
    return SCHED;
  } else if (incomingData.indexOf("Temp") != -1) {
    return TEMP;
  } else {
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

    schedule.updatePWM(timeReader.currentHour, timeReader.currentMinute, headTab);
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
          if (esp.incomingData.indexOf("&r=") != -1) {
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
          Serial.println("Otrzymano dane: " + esp.incomingData);
          esp.sendHTTPResponse("OK");
          esp.closeConnection();
          esp.incomingData = "";
          dataType = NONE;
          break;
        case NONE:
          Serial.println("No valid data received.");
          esp.incomingData = "";
          break;
        case TEMP:
          Serial.println("TEMP data received.");
          float tempC = sensor.readTemperatureC();
          esp.sendHTTPResponse(String(tempC));
          esp.closeConnection();
          esp.incomingData = "";
          dataType = NONE;
          break;
      }
      
    }
  }
}