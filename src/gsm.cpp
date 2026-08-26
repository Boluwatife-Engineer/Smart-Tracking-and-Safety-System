#include "gsm.h"
#include "sim7600.h"
#include "storage.h"
#include "mpu6050.h"
#include "battery.h"
#include "tracker_state.h"


// ======================================================
// GSM STATE
// ======================================================

bool gsmReady = false;


// ======================================================
// FIREBASE URLS
// ======================================================

const char *historyURL =
    "https://smart-trackerr-default-rtdb.firebaseio.com/Trackers/tracker_001/history.json";

const char *currentURL =
    "https://smart-trackerr-default-rtdb.firebaseio.com/Trackers/tracker_001/current.json";

const char *statusURL =
    "https://smart-trackerr-default-rtdb.firebaseio.com/Trackers/tracker_001/status.json";


// ======================================================
// OFFLINE QUEUE CONTROL
// ======================================================

unsigned long lastOfflineSync = 0;

const unsigned long OFFLINE_SYNC_INTERVAL = 10000;


// ======================================================
// INITIALIZE GSM
// ======================================================

void initGSM()
{
    Serial.println();
    Serial.println("========== GSM INITIALIZATION ==========");


    // ==================================================
    // STORAGE
    // ==================================================

    initStorage();


    // ==================================================
    // SIM STATUS
    // ==================================================

    sendSIM7600Command(
        "AT+CPIN?",
        1000
    );


    // ==================================================
    // SIGNAL
    // ==================================================

    sendSIM7600Command(
        "AT+CSQ",
        1000
    );


    // ==================================================
    // NETWORK REGISTRATION
    // ==================================================

    sendSIM7600Command(
        "AT+CEREG?",
        1000
    );


    // ==================================================
    // PACKET DOMAIN
    // ==================================================

    sendSIM7600Command(
        "AT+CGATT?",
        1000
    );


    // ==================================================
    // APN
    // ==================================================

    sendSIM7600Command(
        "AT+CGDCONT=1,\"IP\",\"web.gprs.mtnnigeria.net\"",
        1000
    );


    // ==================================================
    // ACTIVATE PDP
    // ==================================================

    sendSIM7600Command(
        "AT+CGACT=1,1",
        5000
    );


    // ==================================================
    // GET IP
    // ==================================================

    sendSIM7600Command(
        "AT+CGPADDR=1",
        2000
    );


    // ==================================================
    // SSL
    // ==================================================

    Serial.println();
    Serial.println("========== SSL CONFIGURATION ==========");

    sendSIM7600Command(
        "AT+CSSLCFG=\"sslversion\",0,4",
        2000
    );

    sendSIM7600Command(
        "AT+CSSLCFG=\"ignorelocaltime\",0,1",
        2000
    );

    sendSIM7600Command(
        "AT+CSSLCFG=\"seclevel\",0,0",
        2000
    );

    sendSIM7600Command(
        "AT+CSSLCFG?",
        3000
    );


    // ==================================================
    // GSM READY
    // ==================================================

    gsmReady = true;

    Serial.println();
    Serial.println("GSM initialization complete.");


    // ==================================================
    // OFFLINE QUEUE
    // ==================================================

    Serial.print("Offline records waiting: ");
    Serial.println(getOfflineRecordCount());

    Serial.println();
}


// ======================================================
// GSM STATUS
// ======================================================

bool isGSMReady()
{
    return gsmReady;
}


// ======================================================
// WAIT FOR RESPONSE
// ======================================================

bool waitForResponse(
    const char *expected,
    unsigned long timeout
)
{
    String response = "";

    unsigned long start = millis();

    while (millis() - start < timeout)
    {
        while (sim7600.available())
        {
            char c = (char)sim7600.read();

            response += c;

            Serial.write(c);

            if (response.indexOf(expected) != -1)
            {
                return true;
            }
        }
    }

    return false;
}


// ======================================================
// CHECK NETWORK
// ======================================================

bool hasNetworkConnection()
{
    if (!gsmReady)
    {
        return false;
    }


    // ==================================================
    // CLEAR UART
    // ==================================================

    while (sim7600.available())
    {
        sim7600.read();
    }


    // ==================================================
    // CEREG
    // ==================================================

    sim7600.println("AT+CEREG?");

    String cereg = "";

    unsigned long start = millis();

    while (millis() - start < 2000)
    {
        while (sim7600.available())
        {
            char c = (char)sim7600.read();

            cereg += c;
        }

        if (cereg.indexOf("OK") != -1)
        {
            break;
        }
    }


    Serial.print("CEREG: ");
    Serial.println(cereg);


    bool registered =
        cereg.indexOf("+CEREG: 0,1") != -1 ||
        cereg.indexOf("+CEREG: 0,5") != -1 ||
        cereg.indexOf("+CEREG: 1,1") != -1 ||
        cereg.indexOf("+CEREG: 1,5") != -1 ||
        cereg.indexOf("+CEREG: 2,1") != -1 ||
        cereg.indexOf("+CEREG: 2,5") != -1;


    if (!registered)
    {
        Serial.println("Network NOT registered.");

        return false;
    }


    // ==================================================
    // CGATT
    // ==================================================

    while (sim7600.available())
    {
        sim7600.read();
    }

    sim7600.println("AT+CGATT?");

    String cgatt = "";

    start = millis();

    while (millis() - start < 2000)
    {
        while (sim7600.available())
        {
            char c = (char)sim7600.read();

            cgatt += c;
        }

        if (cgatt.indexOf("OK") != -1)
        {
            break;
        }
    }


    Serial.print("CGATT: ");
    Serial.println(cgatt);


    if (cgatt.indexOf("+CGATT: 1") == -1)
    {
        Serial.println("Packet domain NOT attached.");

        return false;
    }


    // ==================================================
    // CGACT
    // ==================================================

    while (sim7600.available())
    {
        sim7600.read();
    }

    sim7600.println("AT+CGACT?");

    String cgact = "";

    start = millis();

    while (millis() - start < 2000)
    {
        while (sim7600.available())
        {
            char c = (char)sim7600.read();

            cgact += c;
        }

        if (cgact.indexOf("OK") != -1)
        {
            break;
        }
    }


    Serial.print("CGACT: ");
    Serial.println(cgact);


    if (cgact.indexOf("+CGACT: 1,1") == -1)
    {
        Serial.println("PDP context is NOT active.");

        return false;
    }


    Serial.println("NETWORK AVAILABLE.");

    return true;
}


// ======================================================
// FIREBASE HTTP REQUEST
// ======================================================
//
// method:
//
// 1 = POST
// 2 = PUT
//
// ======================================================

bool sendFirebaseRequest(
    const char *url,
    const String &payload,
    int method
)
{
    if (!gsmReady)
    {
        Serial.println("GSM is not ready.");

        return false;
    }


    Serial.println();
    Serial.println("================================");
    Serial.println("FIREBASE REQUEST");
    Serial.println("================================");


    Serial.print("URL: ");
    Serial.println(url);


    Serial.print("Payload: ");
    Serial.println(payload);


    if (method == 1)
    {
        Serial.println("Method: POST");
    }
    else
    {
        Serial.println("Method: PUT");
        Serial.println("Transport: POST + X-HTTP-Method-Override");
    }


    // ==================================================
    // CLEAR UART
    // ==================================================

    while (sim7600.available())
    {
        sim7600.read();
    }


    // ==================================================
    // TERMINATE PREVIOUS SESSION
    // ==================================================

    sim7600.println("AT+HTTPTERM");

    delay(500);


    while (sim7600.available())
    {
        Serial.write(sim7600.read());
    }


    // ==================================================
    // HTTP INIT
    // ==================================================

    sim7600.println("AT+HTTPINIT");

    if (!waitForResponse("OK", 3000))
    {
        Serial.println("ERROR: HTTPINIT failed.");

        return false;
    }


    // ==================================================
    // CID
    // ==================================================

    sim7600.println("AT+HTTPPARA=\"CID\",1");

    if (!waitForResponse("OK", 3000))
    {
        Serial.println("ERROR: CID failed.");

        sim7600.println("AT+HTTPTERM");

        return false;
    }


    // ==================================================
    // URL
    // ==================================================

    String urlCommand = "AT+HTTPPARA=\"URL\",\"";

    urlCommand += url;
    urlCommand += "\"";

    sim7600.println(urlCommand);

    if (!waitForResponse("OK", 5000))
    {
        Serial.println("ERROR: URL setup failed.");

        sim7600.println("AT+HTTPTERM");

        return false;
    }


    // ==================================================
    // CONTENT TYPE
    // ==================================================

    sim7600.println(
        "AT+HTTPPARA=\"CONTENT\",\"application/json\""
    );

    if (!waitForResponse("OK", 3000))
    {
        Serial.println("ERROR: Content type failed.");

        sim7600.println("AT+HTTPTERM");

        return false;
    }


    // ==================================================
    // PUT OVERRIDE
    // ==================================================

    if (method == 2)
    {
        sim7600.println(
            "AT+HTTPPARA=\"USERDATA\",\"X-HTTP-Method-Override: PUT\""
        );

        if (!waitForResponse("OK", 3000))
        {
            Serial.println("ERROR: PUT override failed.");

            sim7600.println("AT+HTTPTERM");

            return false;
        }
    }


    // ==================================================
    // HTTP DATA
    // ==================================================

    String httpDataCommand = "AT+HTTPDATA=";

    httpDataCommand += payload.length();
    httpDataCommand += ",15000";

    sim7600.println(httpDataCommand);

    if (!waitForResponse("DOWNLOAD", 5000))
    {
        Serial.println("ERROR: DOWNLOAD prompt not received.");

        sim7600.println("AT+HTTPTERM");

        return false;
    }


    // ==================================================
    // SEND JSON
    // ==================================================

    sim7600.print(payload);

    if (!waitForResponse("OK", 15000))
    {
        Serial.println("ERROR: Payload rejected.");

        sim7600.println("AT+HTTPTERM");

        return false;
    }


    // ==================================================
    // HTTP ACTION
    // ==================================================

    sim7600.println("AT+HTTPACTION=1");

    String response = "";

    unsigned long start = millis();

    while (millis() - start < 30000)
    {
        while (sim7600.available())
        {
            char c = (char)sim7600.read();

            response += c;

            Serial.write(c);
        }

        if (response.indexOf("+HTTPACTION:") != -1)
        {
            break;
        }
    }


    Serial.println();
    Serial.println("HTTP ACTION RESPONSE:");
    Serial.println(response);


    // ==================================================
    // PARSE HTTP CODE
    // ==================================================

    bool success = false;

    int actionIndex =
        response.indexOf("+HTTPACTION:");


    if (actionIndex != -1)
    {
        String action =
            response.substring(actionIndex);


        if (action.indexOf(",200,") != -1)
        {
            success = true;
        }

        if (action.indexOf(",201,") != -1)
        {
            success = true;
        }

        if (action.indexOf(",204,") != -1)
        {
            success = true;
        }
    }


    // ==================================================
    // READ RESPONSE
    // ==================================================

    if (success)
    {
        Serial.println("FIREBASE REQUEST SUCCESS.");

        sim7600.println("AT+HTTPREAD");

        delay(1000);

        while (sim7600.available())
        {
            Serial.write(sim7600.read());
        }
    }
    else
    {
        Serial.println("FIREBASE REQUEST FAILED.");
    }


    // ==================================================
    // CLOSE HTTP
    // ==================================================

    sim7600.println("AT+HTTPTERM");

    delay(500);

    while (sim7600.available())
    {
        Serial.write(sim7600.read());
    }


    Serial.println("================================");

    return success;
}


// ======================================================
// BUILD TIMESTAMP
// ======================================================

String buildTimestamp(
    String gpsTime,
    String gpsDate
)
{
    if (
        gpsDate.length() < 6 ||
        gpsTime.length() < 6
    )
    {
        return "unknown";
    }


    String day =
        gpsDate.substring(0, 2);

    String month =
        gpsDate.substring(2, 4);

    String year =
        gpsDate.substring(4, 6);


    String hour =
        gpsTime.substring(0, 2);

    String minute =
        gpsTime.substring(2, 4);

    String second =
        gpsTime.substring(4, 6);


    String timestamp = "20";

    timestamp += year;
    timestamp += "-";
    timestamp += month;
    timestamp += "-";
    timestamp += day;
    timestamp += "T";
    timestamp += hour;
    timestamp += ":";
    timestamp += minute;
    timestamp += ":";
    timestamp += second;
    timestamp += "Z";


    return timestamp;
}


// ======================================================
// SAVE HISTORY OR QUEUE OFFLINE
// ======================================================

bool saveHistoryOrQueue(
    const String &payload
)
{
    // ==================================================
    // NETWORK CHECK
    // ==================================================

    if (!hasNetworkConnection())
    {
        Serial.println();
        Serial.println("NO INTERNET / NETWORK.");
        Serial.println("Saving history record locally.");

        return saveOfflineRecord(payload);
    }


    // ==================================================
    // TRY LIVE UPLOAD
    // ==================================================

    bool success =
        sendFirebaseRequest(
            historyURL,
            payload,
            1
        );


    if (success)
    {
        Serial.println("History uploaded successfully.");

        return true;
    }


    // ==================================================
    // UPLOAD FAILED
    // ==================================================

    Serial.println();
    Serial.println("History upload failed.");
    Serial.println("Saving record to offline queue.");


    return saveOfflineRecord(payload);
}


// ======================================================
// SYNC OFFLINE HISTORY
// ======================================================

void syncOfflineHistory()
{
    int count =
        getOfflineRecordCount();


    if (count <= 0)
    {
        return;
    }


    Serial.println();
    Serial.println("================================");
    Serial.println("OFFLINE HISTORY SYNC");
    Serial.println("================================");


    Serial.print("Records waiting: ");
    Serial.println(count);


    // ==================================================
    // CHECK NETWORK
    // ==================================================

    if (!hasNetworkConnection())
    {
        Serial.println("Network still unavailable.");

        return;
    }


    // ==================================================
    // UPLOAD FROM OLDEST TO NEWEST
    // ==================================================

    int uploaded = 0;


    while (getOfflineRecordCount() > 0)
    {
        String payload =
            getOfflineRecord(0);


        if (payload.length() == 0)
        {
            Serial.println("Invalid offline record.");

            deleteOfflineRecord(0);

            continue;
        }


        Serial.println();
        Serial.println("Uploading offline record...");


        bool success =
            sendFirebaseRequest(
                historyURL,
                payload,
                1
            );


        if (!success)
        {
            Serial.println("Offline upload failed.");
            Serial.println("Stopping sync. Record retained.");

            break;
        }


        // ==================================================
        // DELETE ONLY AFTER SUCCESS
        // ==================================================

        deleteOfflineRecord(0);

        uploaded++;


        Serial.print(
            "Offline record uploaded. Remaining: "
        );

        Serial.println(
            getOfflineRecordCount()
        );


        delay(500);
    }


    Serial.println();

    Serial.print(
        "Offline records uploaded: "
    );

    Serial.println(uploaded);


    Serial.print(
        "Offline records remaining: "
    );

    Serial.println(
        getOfflineRecordCount()
    );


    Serial.println("================================");
}


// ======================================================
// GSM LOOP
// ======================================================

void gsmLoop()
{
    if (
        millis() - lastOfflineSync <
        OFFLINE_SYNC_INTERVAL
    )
    {
        return;
    }


    lastOfflineSync =
        millis();


    if (
        getOfflineRecordCount() > 0
    )
    {
        syncOfflineHistory();
    }
}


// ======================================================
// SEND LOCATION
// ======================================================

bool sendLocation(
    float latitude,
    float longitude,
    float altitude,
    String gpsTime,
    String gpsDate,
    bool gpsAvailable
)
{
    if (!gsmReady)
    {
        Serial.println("GSM is not ready.");

        return false;
    }


    // ==================================================
    // FIRST: SYNC OLD DATA
    // ==================================================

    syncOfflineHistory();


    // ==================================================
    // TIMESTAMP
    // ==================================================

    String timestamp =
        buildTimestamp(
            gpsTime,
            gpsDate
        );


    // ==================================================
    // SENSOR VALUES
    // ==================================================

    float accelX =
        getAccelX();

    float accelY =
        getAccelY();

    float accelZ =
        getAccelZ();


    float gyroX =
        getGyroX();

    float gyroY =
        getGyroY();

    float gyroZ =
        getGyroZ();


    uint8_t battery =
        getBatteryLevel();


    bool moving =
        isMoving();


    const char *motion =
        moving
            ? "MOVING"
            : "STATIONARY";


    const char *trackerMode =
        getTrackerModeName();


    // ==================================================
    // STATUS
    // ==================================================

    const char *status =
        gpsAvailable
            ? "LOCATION_AVAILABLE"
            : "SENSOR_ONLY";


    const char *source =
        gpsAvailable
            ? "device_gps"
            : "device_sensor";


    // ==================================================
    // CURRENT PAYLOAD
    // ==================================================

    String currentPayload = "{";


    // ==================================================
    // GPS
    // ==================================================

    if (gpsAvailable)
    {
        currentPayload += "\"latitude\":";
        currentPayload += String(latitude, 6);
        currentPayload += ",";


        currentPayload += "\"longitude\":";
        currentPayload += String(longitude, 6);
        currentPayload += ",";


        currentPayload += "\"altitude\":";
        currentPayload += String(altitude, 1);
        currentPayload += ",";


        currentPayload += "\"gpsTime\":\"";
        currentPayload += gpsTime;
        currentPayload += "\",";


        currentPayload += "\"gpsDate\":\"";
        currentPayload += gpsDate;
        currentPayload += "\",";
    }


    // ==================================================
    // COMMON
    // ==================================================

    currentPayload += "\"timestamp\":\"";
    currentPayload += timestamp;
    currentPayload += "\",";


    currentPayload += "\"source\":\"";
    currentPayload += source;
    currentPayload += "\",";


    currentPayload += "\"status\":\"";
    currentPayload += status;
    currentPayload += "\",";


    currentPayload += "\"motion\":\"";
    currentPayload += motion;
    currentPayload += "\",";


    currentPayload += "\"trackerMode\":\"";
    currentPayload += trackerMode;
    currentPayload += "\",";


    currentPayload += "\"battery\":";
    currentPayload += String(battery);
    currentPayload += ",";


    // ==================================================
    // ACCELERATION
    // ==================================================

    currentPayload += "\"acceleration\":{";


    currentPayload += "\"x\":";
    currentPayload += String(accelX, 3);
    currentPayload += ",";


    currentPayload += "\"y\":";
    currentPayload += String(accelY, 3);
    currentPayload += ",";


    currentPayload += "\"z\":";
    currentPayload += String(accelZ, 3);


    currentPayload += "},";


    // ==================================================
    // GYROSCOPE
    // ==================================================

    currentPayload += "\"gyroscope\":{";


    currentPayload += "\"x\":";
    currentPayload += String(gyroX, 3);
    currentPayload += ",";


    currentPayload += "\"y\":";
    currentPayload += String(gyroY, 3);
    currentPayload += ",";


    currentPayload += "\"z\":";
    currentPayload += String(gyroZ, 3);


    currentPayload += "}";


    currentPayload += "}";


    // ==================================================
    // STATUS PAYLOAD
    // ==================================================

    String statusPayload = "{";


    statusPayload += "\"status\":\"";
    statusPayload += status;
    statusPayload += "\",";


    statusPayload += "\"source\":\"";
    statusPayload += source;
    statusPayload += "\",";


    statusPayload += "\"motion\":\"";
    statusPayload += motion;
    statusPayload += "\",";


    statusPayload += "\"trackerMode\":\"";
    statusPayload += trackerMode;
    statusPayload += "\",";


    statusPayload += "\"battery\":";
    statusPayload += String(battery);
    statusPayload += ",";


    statusPayload += "\"timestamp\":\"";
    statusPayload += timestamp;
    statusPayload += "\",";


    // ==================================================
    // STATUS ACCELERATION
    // ==================================================

    statusPayload += "\"acceleration\":{";


    statusPayload += "\"x\":";
    statusPayload += String(accelX, 3);
    statusPayload += ",";


    statusPayload += "\"y\":";
    statusPayload += String(accelY, 3);
    statusPayload += ",";


    statusPayload += "\"z\":";
    statusPayload += String(accelZ, 3);


    statusPayload += "},";


    // ==================================================
    // STATUS GYROSCOPE
    // ==================================================

    statusPayload += "\"gyroscope\":{";


    statusPayload += "\"x\":";
    statusPayload += String(gyroX, 3);
    statusPayload += ",";


    statusPayload += "\"y\":";
    statusPayload += String(gyroY, 3);
    statusPayload += ",";


    statusPayload += "\"z\":";
    statusPayload += String(gyroZ, 3);


    statusPayload += "}";


    statusPayload += "}";


    // ==================================================
    // DEBUG PAYLOADS
    // ==================================================

    Serial.println();
    Serial.println("CURRENT PAYLOAD:");
    Serial.println(currentPayload);

    Serial.println();
    Serial.println("STATUS PAYLOAD:");
    Serial.println(statusPayload);


    // ==================================================
    // HISTORY
    // ==================================================

    bool historySuccess =
        saveHistoryOrQueue(
            currentPayload
        );


    // ==================================================
    // CURRENT + STATUS
    // ==================================================

    bool currentSuccess = false;

    bool statusSuccess = false;


    if (hasNetworkConnection())
    {
        // ==================================================
        // CURRENT
        // ==================================================

        currentSuccess =
            sendFirebaseRequest(
                currentURL,
                currentPayload,
                2
            );


        delay(500);


        // ==================================================
        // STATUS
        // ==================================================

        statusSuccess =
            sendFirebaseRequest(
                statusURL,
                statusPayload,
                2
            );
    }
    else
    {
        Serial.println();
        Serial.println("Network unavailable.");
        Serial.println("Current/status NOT updated.");
    }


    // ==================================================
    // RESULT
    // ==================================================

    Serial.println();
    Serial.println("Firebase tracker upload results:");


    Serial.print("History: ");

    Serial.println(
        historySuccess
            ? "SUCCESS"
            : "QUEUED"
    );


    Serial.print("Current: ");

    Serial.println(
        currentSuccess
            ? "SUCCESS"
            : "NOT UPDATED"
    );


    Serial.print("Status: ");

    Serial.println(
        statusSuccess
            ? "SUCCESS"
            : "NOT UPDATED"
    );


    return historySuccess;
}


// ======================================================
// LOG GPS NO FIX
// ======================================================

bool logGPSNoFix(
    String gpsTime,
    String gpsDate
)
{
    if (!gsmReady)
    {
        Serial.println("GSM is not ready.");

        return false;
    }


    // ==================================================
    // SYNC OLD DATA FIRST
    // ==================================================

    syncOfflineHistory();


    // ==================================================
    // TIMESTAMP
    // ==================================================

    String timestamp =
        buildTimestamp(
            gpsTime,
            gpsDate
        );


    // ==================================================
    // SENSOR VALUES
    // ==================================================

    float accelX =
        getAccelX();

    float accelY =
        getAccelY();

    float accelZ =
        getAccelZ();


    float gyroX =
        getGyroX();

    float gyroY =
        getGyroY();

    float gyroZ =
        getGyroZ();


    uint8_t battery =
        getBatteryLevel();


    bool moving =
        isMoving();


    const char *motion =
        moving
            ? "MOVING"
            : "STATIONARY";


    const char *trackerMode =
        getTrackerModeName();


    // ==================================================
    // BUILD PAYLOAD
    // ==================================================

    String payload = "{";


    // ==================================================
    // STATUS
    // ==================================================

    payload += "\"status\":\"NO_FIX\",";


    // ==================================================
    // SOURCE
    // ==================================================

    payload += "\"source\":\"device_sensor\",";


    // ==================================================
    // GPS TIME
    // ==================================================

    payload += "\"gpsTime\":\"";
    payload += gpsTime;
    payload += "\",";


    // ==================================================
    // GPS DATE
    // ==================================================

    payload += "\"gpsDate\":\"";
    payload += gpsDate;
    payload += "\",";


    // ==================================================
    // TIMESTAMP
    // ==================================================

    payload += "\"timestamp\":\"";
    payload += timestamp;
    payload += "\",";


    // ==================================================
    // MOTION
    // ==================================================

    payload += "\"motion\":\"";
    payload += motion;
    payload += "\",";


    // ==================================================
    // TRACKER MODE
    // ==================================================

    payload += "\"trackerMode\":\"";
    payload += trackerMode;
    payload += "\",";


    // ==================================================
    // BATTERY
    // ==================================================

    payload += "\"battery\":";
    payload += String(battery);
    payload += ",";


    // ==================================================
    // ACCELERATION
    // ==================================================

    payload += "\"acceleration\":{";


    payload += "\"x\":";
    payload += String(accelX, 3);
    payload += ",";


    payload += "\"y\":";
    payload += String(accelY, 3);
    payload += ",";


    payload += "\"z\":";
    payload += String(accelZ, 3);


    payload += "},";


    // ==================================================
    // GYROSCOPE
    // ==================================================

    payload += "\"gyroscope\":{";


    payload += "\"x\":";
    payload += String(gyroX, 3);
    payload += ",";


    payload += "\"y\":";
    payload += String(gyroY, 3);
    payload += ",";


    payload += "\"z\":";
    payload += String(gyroZ, 3);


    payload += "}";


    payload += "}";


    // ==================================================
    // DEBUG
    // ==================================================

    Serial.println();
    Serial.println("NO_FIX PAYLOAD:");
    Serial.println(payload);


    // ==================================================
    // SAVE / UPLOAD HISTORY
    // ==================================================

    bool success =
        saveHistoryOrQueue(
            payload
        );


    if (success)
    {
        Serial.println();
        Serial.println(
            "GPS NO_FIX HISTORY SAVED/QUEUED."
        );
    }
    else
    {
        Serial.println();
        Serial.println(
            "FAILED TO SAVE GPS NO_FIX RECORD."
        );
    }


    // ==================================================
    // IMPORTANT
    // ==================================================
    //
    // NO_FIX records NEVER update /current.
    //
    // NO_FIX records NEVER update /status.
    //
    // They are history-only records.
    //
    // ==================================================

    return success;
}