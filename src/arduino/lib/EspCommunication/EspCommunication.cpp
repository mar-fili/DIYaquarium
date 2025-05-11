#include "EspCommunication.h"
#include "secrets.h"

SoftwareSerial EspCommunication::espSerial(10, 11); // RX, TX

void EspCommunication::initialize() {
    this->espSerial.begin(9600);
    sendCommand("AT", 1000);
    sendCommand("AT+CWMODE=1", 2000);
    sendCommand("AT+CWJAP=\"" SSID "\",\"" PASSWORD "\"", 5000);
    sendCommand("AT+CIPMUX=1", 1000);
    sendCommand("AT+CIPSERVER=1,80", 1000);
    sendCommand("AT+CIFSR", 1000);
}

void EspCommunication::sendCommand(String command, int timeout) {
    espSerial.println(command);

    long int time = millis();
    while ((millis() - time) < timeout) {
        while (espSerial.available()) {
            char c = espSerial.read();
            Serial.write(c);
        }
    }
    Serial.println();
}

int EspCommunication::getClientId() {
    int ipdIndex = this->incomingData.indexOf("+IPD,");
    if (ipdIndex != -1) {
        ipdIndex += 4;
        int secondComma = this->incomingData.indexOf(',', ipdIndex + 1);
        return this->incomingData.substring(ipdIndex, secondComma).toInt();
    }
}

void EspCommunication::sendHTTPResponse(String response) {
    this -> clientId = getClientId();
    String body = response;
    sprintf(this->buffer, "AT+CIPSEND=%d,%d", this->clientId, body.length());
    sendCommand(this->buffer, 2000);
    sendCommand(body, 2000);
    sprintf(this->buffer, "AT+CIPCLOSE=%d", clientId);
    sendCommand(this->buffer, 1000);
}

void EspCommunication::closeConnection() {
    sprintf(this->buffer, "AT+CIPCLOSE=%d", this->clientId);
    sendCommand(this->buffer, 1000);
    this->incomingData = "";
}