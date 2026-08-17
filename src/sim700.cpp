#include "sim7600.h"

HardwareSerial sim7600(1);

void initSIM7600()
{
    sim7600.begin(
        SIM7600_BAUD,
        SERIAL_8N1,
        SIM7600_RX,
        SIM7600_TX
    );

    delay(2000);

    Serial.println();
    Serial.println("========== SIM7600 INITIALIZATION ==========");

    sim7600.println("AT");

    delay(500);

    while (sim7600.available())
    {
        Serial.write(sim7600.read());
    }

    Serial.println();
    Serial.println("SIM7600 UART ready.");
}

void sendSIM7600Command(
    const char *command,
    unsigned long waitTime
)
{
    Serial.print("\n>> ");
    Serial.println(command);

    sim7600.println(command);

    unsigned long start = millis();

    while (millis() - start < waitTime)
    {
        while (sim7600.available())
        {
            Serial.write(sim7600.read());
        }
    }
}

String readSIM7600Response(
    unsigned long waitTime
)
{
    String response = "";

    unsigned long start = millis();

    while (millis() - start < waitTime)
    {
        while (sim7600.available())
        {
            response += (char)sim7600.read();
        }
    }

    return response;
}