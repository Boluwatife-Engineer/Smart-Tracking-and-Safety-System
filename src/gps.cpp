#include "gps.h"
#include <Arduino.h>

// ================= UART PINS =================

#define GPS_RX 20   // ESP32 RX <- SIM7600 TX
#define GPS_TX 21   // ESP32 TX -> SIM7600 RX

#define GPS_BAUD 115200

HardwareSerial sim7600(1);

// ================= GPS DATA =================

float latitude = 0.0;
float longitude = 0.0;
float altitude = 0.0;

String gpsTime = "";
String gpsDate = "";

bool gpsFix = false;

// ================= SEND AT COMMAND =================

void sendGPSCommand(const char *command)
{
    sim7600.println(command);

    delay(500);

    while (sim7600.available())
    {
        sim7600.read();
    }
}

// ================= PARSE GPS DATA =================

void parseGPSInfo(String response)
{
    int start = response.indexOf("+CGPSINFO:");

    if (start == -1)
    {
        return;
    }

    String data = response.substring(
        start + strlen("+CGPSINFO:")
    );

    data.trim();

    // No GPS fix
    if (data.startsWith(","))
    {
        gpsFix = false;
        return;
    }

    // Split comma-separated fields

    String fields[8];

    int fieldIndex = 0;
    int previousIndex = 0;

    for (int i = 0; i <= data.length(); i++)
    {
        if (data[i] == ',' || i == data.length())
        {
            if (fieldIndex < 8)
            {
                fields[fieldIndex] =
                    data.substring(previousIndex, i);
            }

            fieldIndex++;
            previousIndex = i + 1;
        }
    }

    // Need at least latitude/longitude
    if (fieldIndex < 4)
    {
        gpsFix = false;
        return;
    }

    // ================= LATITUDE =================

    if (fields[0].length() == 0 ||
        fields[1].length() == 0)
    {
        gpsFix = false;
        return;
    }

    // ================= LONGITUDE =================

    if (fields[2].length() == 0 ||
        fields[3].length() == 0)
    {
        gpsFix = false;
        return;
    }

    float rawLat = fields[0].toFloat();
    float rawLon = fields[2].toFloat();

    // Convert DDMM.MMMMM → decimal degrees

    int latDegrees = (int)(rawLat / 100);
    float latMinutes = rawLat -
                       (latDegrees * 100);

    latitude =
        latDegrees +
        (latMinutes / 60.0);

    int lonDegrees = (int)(rawLon / 100);
    float lonMinutes = rawLon -
                       (lonDegrees * 100);

    longitude =
        lonDegrees +
        (lonMinutes / 60.0);

    // South / West

    if (fields[1] == "S")
    {
        latitude = -latitude;
    }

    if (fields[3] == "W")
    {
        longitude = -longitude;
    }

    // ================= DATE / TIME =================

    if (fields[4].length() > 0)
    {
        gpsDate = fields[4];
    }

    if (fields[5].length() > 0)
    {
        gpsTime = fields[5];
    }

    // ================= ALTITUDE =================

    if (fields[6].length() > 0)
    {
        altitude = fields[6].toFloat();
    }

    gpsFix = true;
}

// ================= INITIALIZE GPS =================

void initGPS()
{
    sim7600.begin(
        GPS_BAUD,
        SERIAL_8N1,
        GPS_RX,
        GPS_TX
    );

    delay(2000);

    Serial.println();
    Serial.println("========== GPS INITIALIZATION ==========");

    // Check SIM7600 communication

    sim7600.println("AT");

    delay(500);

    while (sim7600.available())
    {
        Serial.write(sim7600.read());
    }

    // Make sure GPS starts cleanly

    sendGPSCommand("AT+CGPS=0");

    delay(500);

    sendGPSCommand("AT+CGPS=1");

    Serial.println("GPS engine enabled.");
    Serial.println("Waiting for satellite fix...");
}

// ================= UPDATE GPS =================

void updateGPS()
{
    // Request GPS information

    sim7600.println("AT+CGPSINFO");

    String response = "";

    unsigned long start = millis();

    while (millis() - start < 1000)
    {
        while (sim7600.available())
        {
            response += (char)sim7600.read();
        }
    }

    parseGPSInfo(response);

    // ================= DISPLAY =================

    if (gpsFix)
    {
        Serial.println();
        Serial.println("========== GPS DATA ==========");

        Serial.print("Latitude:  ");
        Serial.println(latitude, 6);

        Serial.print("Longitude: ");
        Serial.println(longitude, 6);

        Serial.print("Altitude:  ");
        Serial.print(altitude, 1);
        Serial.println(" m");

        Serial.print("Date:      ");
        Serial.println(gpsDate);

        Serial.print("Time:      ");
        Serial.println(gpsTime);

        Serial.println("GPS STATUS: FIXED");
        Serial.println("==============================");
    }
    else
    {
        Serial.println("GPS STATUS: SEARCHING...");
    }
}

// ================= GETTERS =================

bool hasGPSFix()
{
    return gpsFix;
}

float getLatitude()
{
    return latitude;
}

float getLongitude()
{
    return longitude;
}

float getAltitude()
{
    return altitude;
}

String getGPSTime()
{
    return gpsTime;
}

String getGPSDate()
{
    return gpsDate;
}