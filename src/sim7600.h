#ifndef SIM7600_H
#define SIM7600_H

#include <Arduino.h>

#define SIM7600_RX 20
#define SIM7600_TX 21
#define SIM7600_BAUD 115200

extern HardwareSerial sim7600;

void initSIM7600();

void sendSIM7600Command(
    const char *command,
    unsigned long waitTime = 2000
);

String readSIM7600Response(
    unsigned long waitTime = 1000
);

#endif