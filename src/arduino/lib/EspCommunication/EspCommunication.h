#ifndef ESPCOMMUNICATION_H
#define ESPCOMMUNICATION_H

#include <Arduino.h>
#include <SoftwareSerial.h>
#include "TimeReader.h"
#include "Schedule.h"
#include "TranzistorControl.h"

class EspCommunication {
    public:
        static SoftwareSerial espSerial;
        int clientId;
        char buffer[50];
        String incomingData;

        void initialize();
        void sendCommand(String command, int timeout);
        int getClientId();
        void sendHTTPResponse(String response);
        void closeConnection();
    };
    
#endif // ESPCOMMUNICATION_H