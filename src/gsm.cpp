#include "gsm.h"

#include "sim7600.h"
#include "storage.h"
#include "mpu6050.h"
#include "battery.h"
#include "tracker_state.h"
#include "sos.h"
#include "gps.h"

bool gsmReady = false;

const char *historyURL =
    "https://smart-trackerr-default-rtdb.firebaseio.com/Trackers/tracker_001/history.json";

const char *currentURL =
    "https://smart-trackerr-default-rtdb.firebaseio.com/Trackers/tracker_001/current.json";

const char *statusURL =
    "https://smart-trackerr-default-rtdb.firebaseio.com/Trackers/tracker_001/status.json";

const char *sosURL =
    "https://smart-trackerr-default-rtdb.firebaseio.com/Trackers/tracker_001/sos.json";

unsigned long lastOfflineSync = 0;
const unsigned long OFFLINE_SYNC_INTERVAL = 10000;

static bool sosEventPending = false;


/* =========================================================
   GSM INITIALIZATION
   ========================================================= */

void initGSM()
{
    Serial.println();
    Serial.println("========== GSM INITIALIZATION ==========");

    /*
       Storage is initialized during startup.

       DO NOT call initStorage() here again.
       Calling it twice causes the Preferences
       initialization warning seen previously.
    */

    sendSIM7600Command("AT+CPIN?", 1000);
    sendSIM7600Command("AT+CSQ", 1000);
    sendSIM7600Command("AT+CEREG?", 1000);
    sendSIM7600Command("AT+CGATT?", 1000);

    /*
       MTN Nigeria APN.
    */

    sendSIM7600Command(
        "AT+CGDCONT=1,\"IP\",\"web.gprs.mtnnigeria.net\"",
        1000
    );

    sendSIM7600Command("AT+CGACT=1,1", 10000);
    sendSIM7600Command("AT+CGPADDR=1", 3000);


    /* =====================================================
       SSL CONFIGURATION
       ===================================================== */

    Serial.println();
    Serial.println("========== SSL CONFIGURATION ==========");

    /*
       SSL context 0.

       sslversion = 4 means "All".

       authmode = 0 means no certificate authentication.

       ignorelocaltime = 1 prevents certificate time checking
       from becoming a problem while the modem RTC is not
       synchronized.

       enableSNI = 1 enables Server Name Indication.
    */

    sendSIM7600Command(
        "AT+CSSLCFG=\"sslversion\",0,4",
        2000
    );

    sendSIM7600Command(
        "AT+CSSLCFG=\"authmode\",0,0",
        2000
    );

    sendSIM7600Command(
        "AT+CSSLCFG=\"ignorelocaltime\",0,1",
        2000
    );

    sendSIM7600Command(
        "AT+CSSLCFG=\"enableSNI\",0,1",
        2000
    );

    sendSIM7600Command(
        "AT+CSSLCFG?",
        3000
    );


    gsmReady = true;

    Serial.println();
    Serial.println("GSM initialization complete.");

    Serial.print("Offline records waiting: ");
    Serial.println(getOfflineRecordCount());

    Serial.println();
}


/* =========================================================
   STATUS
   ========================================================= */

bool isGSMReady()
{
    return gsmReady;
}


/* =========================================================
   WAIT FOR MODEM RESPONSE
   ========================================================= */

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

        delay(10);
    }

    Serial.print("Timeout waiting for: ");
    Serial.println(expected);

    return false;
}


/* =========================================================
   NETWORK CHECK
   ========================================================= */

bool hasNetworkConnection()
{
    if (!gsmReady)
    {
        Serial.println("GSM is not ready.");
        return false;
    }


    /* -----------------------------------------------------
       CHECK NETWORK REGISTRATION
       ----------------------------------------------------- */

    while (sim7600.available())
        sim7600.read();

    sim7600.println("AT+CEREG?");

    String cereg = "";

    unsigned long start = millis();

    while (millis() - start < 3000)
    {
        while (sim7600.available())
        {
            char c = (char)sim7600.read();

            cereg += c;

            Serial.write(c);
        }

        if (cereg.indexOf("OK") != -1)
            break;

        delay(10);
    }

    Serial.println();
    Serial.print("CEREG RESPONSE: ");
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

    Serial.println("Cellular network registered.");


    /* -----------------------------------------------------
       CHECK PACKET ATTACHMENT
       ----------------------------------------------------- */

    while (sim7600.available())
        sim7600.read();

    sim7600.println("AT+CGATT?");

    String cgatt = "";

    start = millis();

    while (millis() - start < 3000)
    {
        while (sim7600.available())
        {
            char c = (char)sim7600.read();

            cgatt += c;

            Serial.write(c);
        }

        if (cgatt.indexOf("OK") != -1)
            break;

        delay(10);
    }

    Serial.println();
    Serial.print("CGATT RESPONSE: ");
    Serial.println(cgatt);


    if (cgatt.indexOf("+CGATT: 1") == -1)
    {
        Serial.println("Packet domain NOT attached.");
        Serial.println("Attempting CGATT=1");

        while (sim7600.available())
            sim7600.read();

        sim7600.println("AT+CGATT=1");

        if (!waitForResponse("OK", 15000))
        {
            Serial.println("ERROR: CGATT=1 failed.");
            return false;
        }

        Serial.println(
            "Packet domain attachment requested."
        );

        delay(1000);

        while (sim7600.available())
            sim7600.read();

        sim7600.println("AT+CGATT?");

        cgatt = "";

        start = millis();

        while (millis() - start < 3000)
        {
            while (sim7600.available())
            {
                char c = (char)sim7600.read();

                cgatt += c;

                Serial.write(c);
            }

            if (cgatt.indexOf("OK") != -1)
                break;

            delay(10);
        }

        Serial.println();
        Serial.print("CGATT RECHECK: ");
        Serial.println(cgatt);


        if (cgatt.indexOf("+CGATT: 1") == -1)
        {
            Serial.println(
                "ERROR: Packet domain still not attached."
            );

            return false;
        }

        Serial.println("Packet domain attached.");
    }
    else
    {
        Serial.println(
            "Packet domain already attached."
        );
    }


    /* -----------------------------------------------------
       CHECK PDP CONTEXT
       ----------------------------------------------------- */

    while (sim7600.available())
        sim7600.read();

    sim7600.println("AT+CGACT?");

    String cgact = "";

    start = millis();

    while (millis() - start < 3000)
    {
        while (sim7600.available())
        {
            char c = (char)sim7600.read();

            cgact += c;

            Serial.write(c);
        }

        if (cgact.indexOf("OK") != -1)
            break;

        delay(10);
    }

    Serial.println();
    Serial.print("CGACT RESPONSE: ");
    Serial.println(cgact);


    if (cgact.indexOf("+CGACT: 1,1") == -1)
    {
        Serial.println(
            "PDP context 1 is NOT active."
        );

        Serial.println(
            "Attempting to activate PDP context..."
        );

        while (sim7600.available())
            sim7600.read();

        sim7600.println(
            "AT+CGDCONT=1,\"IP\",\"web.gprs.mtnnigeria.net\""
        );

        if (!waitForResponse("OK", 3000))
        {
            Serial.println(
                "WARNING: APN configuration did not return OK."
            );
        }

        while (sim7600.available())
            sim7600.read();

        sim7600.println("AT+CGACT=1,1");

        if (!waitForResponse("OK", 15000))
        {
            Serial.println(
                "ERROR: PDP activation failed."
            );

            return false;
        }

        Serial.println(
            "PDP context activation requested."
        );

        delay(1000);

        while (sim7600.available())
            sim7600.read();

        sim7600.println("AT+CGACT?");

        cgact = "";

        start = millis();

        while (millis() - start < 3000)
        {
            while (sim7600.available())
            {
                char c = (char)sim7600.read();

                cgact += c;

                Serial.write(c);
            }

            if (cgact.indexOf("OK") != -1)
                break;

            delay(10);
        }

        Serial.println();
        Serial.print("CGACT RECHECK: ");
        Serial.println(cgact);


        if (cgact.indexOf("+CGACT: 1,1") == -1)
        {
            Serial.println(
                "ERROR: PDP context is still inactive."
            );

            return false;
        }

        Serial.println(
            "PDP context successfully activated."
        );
    }
    else
    {
        Serial.println(
            "PDP context already active."
        );
    }


    /* -----------------------------------------------------
       GET IP ADDRESS
       ----------------------------------------------------- */

    while (sim7600.available())
        sim7600.read();

    sim7600.println("AT+CGPADDR=1");

    String ipResponse = "";

    start = millis();

    while (millis() - start < 3000)
    {
        while (sim7600.available())
        {
            char c = (char)sim7600.read();

            ipResponse += c;

            Serial.write(c);
        }

        if (ipResponse.indexOf("OK") != -1)
            break;

        delay(10);
    }

    Serial.println();
    Serial.print("IP RESPONSE: ");
    Serial.println(ipResponse);


    if (ipResponse.indexOf("+CGPADDR: 1,") == -1)
    {
        Serial.println(
            "WARNING: No IP address reported."
        );
    }

    Serial.println("NETWORK AVAILABLE.");

    return true;
}


/* =========================================================
   FIREBASE HTTP REQUEST
   ========================================================= */

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

    Serial.print("Payload length: ");
    Serial.println(payload.length());


    if (method == 1)
    {
        Serial.println("Method: POST");
    }
    else
    {
        Serial.println("Method: PUT");
        Serial.println(
            "Transport: POST + X-HTTP-Method-Override"
        );
    }


    /* -----------------------------------------------------
       TERMINATE PREVIOUS HTTP SESSION
       ----------------------------------------------------- */

    while (sim7600.available())
        sim7600.read();

    sim7600.println("AT+HTTPTERM");

    delay(300);

    while (sim7600.available())
    {
        Serial.write(sim7600.read());
    }


    /* -----------------------------------------------------
       HTTP INIT
       ----------------------------------------------------- */

    Serial.println(
        "Initializing HTTP..."
    );

    sim7600.println("AT+HTTPINIT");

    if (!waitForResponse("OK", 5000))
    {
        Serial.println(
            "ERROR: HTTPINIT failed."
        );

        return false;
    }


    /* -----------------------------------------------------
       CID
       ----------------------------------------------------- */

    Serial.println(
        "Setting CID..."
    );

    sim7600.println(
        "AT+HTTPPARA=\"CID\",1"
    );

    if (!waitForResponse("OK", 3000))
    {
        Serial.println(
            "ERROR: HTTP CID configuration failed."
        );

        sim7600.println("AT+HTTPTERM");

        delay(300);

        return false;
    }


    /* -----------------------------------------------------
       URL
       ----------------------------------------------------- */

    Serial.println(
        "Setting URL..."
    );

    String urlCommand =
        "AT+HTTPPARA=\"URL\",\"";

    urlCommand += url;
    urlCommand += "\"";

    sim7600.println(urlCommand);

    if (!waitForResponse("OK", 5000))
    {
        Serial.println(
            "ERROR: URL setup failed."
        );

        sim7600.println("AT+HTTPTERM");

        delay(300);

        return false;
    }


    /* -----------------------------------------------------
       HTTPS / SSL
       -----------------------------------------------------

       DO NOT SEND:

       AT+HTTPPARA="SSLCFG","0"

       Your SIM7600 firmware rejects that command.

       SSL context 0 is already configured using:

       AT+CSSLCFG="sslversion",0,4
       AT+CSSLCFG="authmode",0,0
       AT+CSSLCFG="ignorelocaltime",0,1
       AT+CSSLCFG="enableSNI",0,1

       HTTPS is therefore used directly through the
       https:// Firebase URL.
    */


    /* -----------------------------------------------------
       CONTENT TYPE
       ----------------------------------------------------- */

    Serial.println(
        "Setting content type..."
    );

    sim7600.println(
        "AT+HTTPPARA=\"CONTENT\",\"application/json\""
    );

    if (!waitForResponse("OK", 3000))
    {
        Serial.println(
            "ERROR: Content type configuration failed."
        );

        sim7600.println("AT+HTTPTERM");

        delay(300);

        return false;
    }


    /* -----------------------------------------------------
       PUT OVERRIDE
       ----------------------------------------------------- */

    if (method == 2)
    {
        Serial.println(
            "Setting PUT method override..."
        );

        sim7600.println(
            "AT+HTTPPARA=\"USERDATA\","
            "\"X-HTTP-Method-Override: PUT\""
        );

        if (!waitForResponse("OK", 3000))
        {
            Serial.println(
                "ERROR: PUT override failed."
            );

            sim7600.println("AT+HTTPTERM");

            delay(300);

            return false;
        }
    }


    /* -----------------------------------------------------
       HTTP DATA
       ----------------------------------------------------- */

    Serial.println(
        "Preparing HTTP data..."
    );

    String httpDataCommand =
        "AT+HTTPDATA=";

    httpDataCommand += payload.length();
    httpDataCommand += ",15000";

    sim7600.println(httpDataCommand);

    if (!waitForResponse("DOWNLOAD", 5000))
    {
        Serial.println(
            "ERROR: DOWNLOAD prompt not received."
        );

        sim7600.println("AT+HTTPTERM");

        delay(300);

        return false;
    }


    /* -----------------------------------------------------
       SEND PAYLOAD
       ----------------------------------------------------- */

    Serial.println(
        "Sending payload..."
    );

    sim7600.print(payload);

    if (!waitForResponse("OK", 15000))
    {
        Serial.println(
            "ERROR: Payload rejected."
        );

        sim7600.println("AT+HTTPTERM");

        delay(300);

        return false;
    }

    Serial.println(
        "Payload accepted by modem."
    );


    /* -----------------------------------------------------
       HTTP ACTION
       ----------------------------------------------------- */

    while (sim7600.available())
        sim7600.read();

    Serial.println();
    Serial.println(
        "Starting HTTPACTION..."
    );

    sim7600.println(
        "AT+HTTPACTION=1"
    );


    String response = "";

    unsigned long start = millis();


    /*
       Give the modem up to 120 seconds.

       HTTPS/TLS negotiation can take considerably
       longer than a normal AT command.
    */

    while (millis() - start < 120000UL)
    {
        while (sim7600.available())
        {
            char c = (char)sim7600.read();

            response += c;

            Serial.write(c);
        }

        if (
            response.indexOf("+HTTPACTION:") != -1
        )
        {
            break;
        }

        delay(10);
    }


    /* -----------------------------------------------------
       HTTP ACTION RESULT
       ----------------------------------------------------- */

    Serial.println();
    Serial.println();

    Serial.println(
        "HTTP ACTION RESPONSE:"
    );

    Serial.println(response);


    int actionIndex =
        response.indexOf("+HTTPACTION:");


    if (actionIndex == -1)
    {
        Serial.println();
        Serial.println(
            "ERROR: No HTTPACTION response received."
        );

        Serial.println(
            "HTTP request timed out or modem did not "
            "return a HTTPACTION URC."
        );

        sim7600.println(
            "AT+HTTPTERM"
        );

        delay(500);

        while (sim7600.available())
        {
            Serial.write(sim7600.read());
        }

        return false;
    }


    /* -----------------------------------------------------
       PARSE HTTPACTION
       ----------------------------------------------------- */

    String action =
        response.substring(actionIndex);


    Serial.print(
        "Parsed HTTPACTION: "
    );

    Serial.println(action);


    /*
       Expected format:

       +HTTPACTION: <method>,<status>,<length>

       Successful Firebase writes can return:

       +HTTPACTION: 1,200,...
       +HTTPACTION: 1,201,...
       +HTTPACTION: 1,204,...
    */

    bool success =
        action.indexOf(",200,") != -1 ||
        action.indexOf(",201,") != -1 ||
        action.indexOf(",204,") != -1;


    /* -----------------------------------------------------
       SUCCESS
       ----------------------------------------------------- */

    if (success)
    {
        Serial.println();
        Serial.println(
            "FIREBASE REQUEST SUCCESS."
        );
    }


    /* -----------------------------------------------------
       FAILURE
       ----------------------------------------------------- */

    else
    {
        Serial.println();
        Serial.println(
            "FIREBASE REQUEST FAILED."
        );

        Serial.println(
            "Attempting HTTPREAD for error details..."
        );


        while (sim7600.available())
            sim7600.read();

        sim7600.println(
            "AT+HTTPREAD"
        );


        unsigned long readStart =
            millis();


        while (
            millis() - readStart < 5000
        )
        {
            while (sim7600.available())
            {
                Serial.write(
                    sim7600.read()
                );
            }

            delay(10);
        }
    }


    /* -----------------------------------------------------
       HTTP TERM
       ----------------------------------------------------- */

    sim7600.println(
        "AT+HTTPTERM"
    );

    delay(500);

    while (sim7600.available())
    {
        Serial.write(
            sim7600.read()
        );
    }


    Serial.println(
        "================================"
    );


    return success;
}


/* =========================================================
   TIMESTAMP
   ========================================================= */

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


/* =========================================================
   SAVE HISTORY OR QUEUE
   ========================================================= */

bool saveHistoryOrQueue(
    const String &payload
)
{
    /*
       First check whether cellular data is available.
    */

    if (!hasNetworkConnection())
    {
        Serial.println();
        Serial.println(
            "NO INTERNET / NETWORK."
        );

        Serial.println(
            "Saving history record locally."
        );


        bool queued =
            saveOfflineRecord(payload);


        if (queued)
        {
            Serial.println(
                "History record QUEUED locally."
            );
        }
        else
        {
            Serial.println(
                "ERROR: Failed to queue history record."
            );
        }


        return queued;
    }


    /*
       Network is available, so try Firebase first.
    */

    bool success =
        sendFirebaseRequest(
            historyURL,
            payload,
            1
        );


    if (success)
    {
        Serial.println(
            "History uploaded to Firebase successfully."
        );

        return true;
    }


    /*
       Firebase failed.

       Keep the record locally so the offline sync
       mechanism can retry it later.
    */

    Serial.println();
    Serial.println(
        "History upload failed."
    );

    Serial.println(
        "Saving record to offline queue."
    );


    bool queued =
        saveOfflineRecord(payload);


    if (queued)
    {
        Serial.println(
            "History record QUEUED locally."
        );
    }
    else
    {
        Serial.println(
            "ERROR: Failed to queue history record."
        );
    }


    return queued;
}


/* =========================================================
   SOS EVENT
   ========================================================= */

bool sendSOSEvent()
{
    if (!gsmReady)
    {
        Serial.println(
            "SOS EVENT: GSM is not ready."
        );

        return false;
    }


    bool sosActive =
        isSOSActive();

    bool gpsAvailable =
        hasGPSFix();


    float latitude = 0;
    float longitude = 0;
    float altitude = 0;

    String gpsTime = "";
    String gpsDate = "";


    if (gpsAvailable)
    {
        latitude =
            getLatitude();

        longitude =
            getLongitude();

        altitude =
            getAltitude();

        gpsTime =
            getGPSTime();

        gpsDate =
            getGPSDate();
    }


    String timestamp;


    if (
        gpsTime.length() >= 6 &&
        gpsDate.length() >= 6
    )
    {
        timestamp =
            buildTimestamp(
                gpsTime,
                gpsDate
            );
    }
    else
    {
        timestamp =
            "unknown";
    }


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


    const char *event =
        sosActive
            ? "SOS_ACTIVATED"
            : "SOS_DEACTIVATED";


    const char *sosStatus =
        sosActive
            ? "ACTIVE"
            : "INACTIVE";


    String payload = "{";


    payload +=
        "\"event\":\"" +
        String(event) +
        "\",";


    payload +=
        "\"sos\":\"" +
        String(sosStatus) +
        "\",";


    payload +=
        "\"timestamp\":\"" +
        timestamp +
        "\",";


    payload +=
        "\"source\":\"device_sos\",";


    payload +=
        "\"gpsAvailable\":" +
        String(
            gpsAvailable
                ? "true"
                : "false"
        ) +
        ",";


    if (gpsAvailable)
    {
        payload +=
            "\"latitude\":" +
            String(latitude, 6) +
            ",";


        payload +=
            "\"longitude\":" +
            String(longitude, 6) +
            ",";


        payload +=
            "\"altitude\":" +
            String(altitude, 1) +
            ",";


        payload +=
            "\"gpsTime\":\"" +
            gpsTime +
            "\",";


        payload +=
            "\"gpsDate\":\"" +
            gpsDate +
            "\",";
    }


    payload +=
        "\"motion\":\"" +
        String(motion) +
        "\",";


    payload +=
        "\"trackerMode\":\"SOS\",";


    payload +=
        "\"battery\":" +
        String(battery) +
        ",";


    payload +=
        "\"acceleration\":{";


    payload +=
        "\"x\":" +
        String(accelX, 3) +
        ",";


    payload +=
        "\"y\":" +
        String(accelY, 3) +
        ",";


    payload +=
        "\"z\":" +
        String(accelZ, 3);


    payload += "},";


    payload +=
        "\"gyroscope\":{";


    payload +=
        "\"x\":" +
        String(gyroX, 3) +
        ",";


    payload +=
        "\"y\":" +
        String(gyroY, 3) +
        ",";


    payload +=
        "\"z\":" +
        String(gyroZ, 3);


    payload += "}";


    payload += "}";


    Serial.println();
    Serial.println(
        "================================"
    );

    Serial.println(
        "SOS EVENT"
    );

    Serial.println(
        "================================"
    );

    Serial.println(payload);


    /*
       Save the SOS event to history.

       If Firebase fails, saveHistoryOrQueue() puts it
       into the offline history queue.
    */

    bool historySuccess =
        saveHistoryOrQueue(
            payload
        );


    /*
       SOS endpoint is NOT part of the offline history queue.
       It is attempted only when the network is available.
    */

    bool sosSuccess = false;


    if (hasNetworkConnection())
    {
        sosSuccess =
            sendFirebaseRequest(
                sosURL,
                payload,
                2
            );
    }


    /* -----------------------------------------------------
       CURRENT SOS STATE
       ----------------------------------------------------- */

    bool currentSuccess = false;


    if (hasNetworkConnection())
    {
        String currentPayload = "{";


        currentPayload +=
            "\"sos\":\"" +
            String(sosStatus) +
            "\",";


        currentPayload +=
            "\"timestamp\":\"" +
            timestamp +
            "\",";


        currentPayload +=
            "\"source\":\"device_sos\",";


        if (gpsAvailable)
        {
            currentPayload +=
                "\"latitude\":" +
                String(latitude, 6) +
                ",";


            currentPayload +=
                "\"longitude\":" +
                String(longitude, 6) +
                ",";


            currentPayload +=
                "\"altitude\":" +
                String(altitude, 1) +
                ",";
        }


        currentPayload +=
            "\"battery\":" +
            String(battery);


        currentPayload += "}";


        currentSuccess =
            sendFirebaseRequest(
                currentURL,
                currentPayload,
                2
            );
    }


    /* -----------------------------------------------------
       STATUS
       ----------------------------------------------------- */

    bool statusSuccess = false;


    if (hasNetworkConnection())
    {
        String statusPayload = "{";


        statusPayload +=
            "\"status\":\"" +
            String(
                sosActive
                    ? "SOS_ACTIVE"
                    : "SOS_INACTIVE"
            ) +
            "\",";


        statusPayload +=
            "\"source\":\"device_sos\",";


        statusPayload +=
            "\"timestamp\":\"" +
            timestamp +
            "\",";


        statusPayload +=
            "\"battery\":" +
            String(battery);


        statusPayload += "}";


        statusSuccess =
            sendFirebaseRequest(
                statusURL,
                statusPayload,
                2
            );
    }


    /* -----------------------------------------------------
       RESULTS
       ----------------------------------------------------- */

    Serial.println();
    Serial.println(
        "SOS FIREBASE RESULTS"
    );


    Serial.print(
        "History: "
    );

    Serial.println(
        historySuccess
            ? "UPLOADED / QUEUED"
            : "FAILED"
    );


    Serial.print(
        "SOS: "
    );

    Serial.println(
        sosSuccess
            ? "SUCCESS"
            : "FAILED"
    );


    Serial.print(
        "Current: "
    );

    Serial.println(
        currentSuccess
            ? "SUCCESS"
            : "FAILED"
    );


    Serial.print(
        "Status: "
    );

    Serial.println(
        statusSuccess
            ? "SUCCESS"
            : "FAILED"
    );


    return historySuccess;
}


/* =========================================================
   SOS PROCESSING
   ========================================================= */

void processSOS()
{
    if (!hasSOSStateChanged())
        return;


    Serial.println();
    Serial.println(
        "SOS STATE CHANGE DETECTED."
    );


    sosEventPending = true;


    if (sendSOSEvent())
    {
        sosEventPending = false;

        clearSOSStateChanged();

        Serial.println(
            "SOS EVENT PROCESSED."
        );
    }
    else
    {
        Serial.println(
            "SOS EVENT NOT SENT."
        );

        Serial.println(
            "Will retry."
        );
    }
}


/* =========================================================
   OFFLINE HISTORY SYNC
   ========================================================= */

void syncOfflineHistory()
{
    int count =
        getOfflineRecordCount();


    if (count <= 0)
    {
        return;
    }


    Serial.println();
    Serial.println(
        "================================"
    );

    Serial.println(
        "OFFLINE HISTORY SYNC"
    );

    Serial.println(
        "================================"
    );

    Serial.print(
        "Records waiting: "
    );

    Serial.println(count);


    /* -----------------------------------------------------
       CHECK NETWORK
       ----------------------------------------------------- */

    if (!hasNetworkConnection())
    {
        Serial.println(
            "Network unavailable. "
            "Sync postponed."
        );

        return;
    }


    /*
       Limit the amount of work performed in one cycle.

       This prevents hundreds of queued records from
       blocking the rest of the tracker indefinitely.

       The next gsmLoop() cycle will continue the queue.
    */

    const int MAX_UPLOADS_PER_CYCLE = 10;

    int uploaded = 0;


    /* -----------------------------------------------------
       UPLOAD QUEUED HISTORY
       ----------------------------------------------------- */

    while (
        uploaded < MAX_UPLOADS_PER_CYCLE &&
        getOfflineRecordCount() > 0
    )
    {
        /*
           Re-check network before every upload.

           If the network disappears, stop immediately.
        */

        if (!hasNetworkConnection())
        {
            Serial.println();
            Serial.println(
                "Network lost during offline sync."
            );

            Serial.println(
                "Stopping sync."
            );

            Serial.println(
                "Remaining records are safely queued."
            );

            break;
        }


        /*
           Always retrieve record 0.

           storage.cpp finds the numerically newest
           record ID.
        */

        String payload =
            getOfflineRecord(0);


        if (payload.length() == 0)
        {
            Serial.println();
            Serial.println(
                "ERROR: Could not read oldest "
                "offline record."
            );

            Serial.println(
                "Stopping sync to preserve queue."
            );

            break;
        }


        Serial.println();
        Serial.println(
            "--------------------------------"
        );

        Serial.println(
            "Uploading offline record"
        );

        Serial.println(
            "--------------------------------"
        );


        Serial.print(
            "Payload length: "
        );

        Serial.println(
            payload.length()
        );


        /*
           IMPORTANT:

           Offline records are uploaded ONLY to:

               historyURL

           They are NEVER replayed to:

               currentURL
               statusURL
               sosURL
        */

        bool success =
            sendFirebaseRequest(
                historyURL,
                payload,
                1
            );


        /* -------------------------------------------------
           UPLOAD FAILED
           ------------------------------------------------- */

        if (!success)
        {
            Serial.println();
            Serial.println(
                "OFFLINE UPLOAD FAILED."
            );

            Serial.println(
                "Record has NOT been deleted."
            );

            Serial.println(
                "Sync stopped."
            );

            Serial.println(
                "Will retry on the next sync cycle."
            );

            break;
        }


        /* -------------------------------------------------
           UPLOAD SUCCESSFUL
           ------------------------------------------------- */

        Serial.println();
        Serial.println(
            "Firebase accepted offline record."
        );


        /*
           CRITICAL:

           Delete the local record ONLY after the
           Firebase request returned HTTP 200, 201,
           or 204.

           sendFirebaseRequest() returns true only for
           those successful HTTP status codes.
        */

        if (
            !deleteOfflineRecord(0)
        )
        {
            Serial.println();
            Serial.println(
                "ERROR: Firebase upload succeeded,"
            );

            Serial.println(
                "but local deletion failed."
            );

            Serial.println(
                "Stopping sync."
            );

            break;
        }


        uploaded++;


        Serial.println(
            "Offline record uploaded and deleted."
        );


        Serial.print(
            "Uploaded this cycle: "
        );

        Serial.println(
            uploaded
        );


        Serial.print(
            "Records remaining: "
        );

        Serial.println(
            getOfflineRecordCount()
        );


        /*
           Small delay between uploads.

           This gives the modem a little breathing room
           and prevents the queue sync from hammering
           the network continuously.
        */

        delay(500);
    }


    /* -----------------------------------------------------
       SYNC SUMMARY
       ----------------------------------------------------- */

    Serial.println();
    Serial.println(
        "================================"
    );

    Serial.print(
        "Offline records uploaded this cycle: "
    );

    Serial.println(
        uploaded
    );


    Serial.print(
        "Offline records remaining: "
    );

    Serial.println(
        getOfflineRecordCount()
    );


    if (
        getOfflineRecordCount() == 0
    )
    {
        Serial.println(
            "OFFLINE QUEUE COMPLETELY CLEARED."
        );
    }
    else
    {
        Serial.println(
            "More records remain."
        );

        Serial.println(
            "They will be uploaded on the next sync cycle."
        );
    }


    Serial.println(
        "================================"
    );
}


/* =========================================================
   GSM LOOP
   ========================================================= */

void gsmLoop()
{
    /*
       Process SOS first.

       SOS behaviour remains independent from the
       offline history queue.
    */

    processSOS();


    /*
       Offline history sync runs every 10 seconds.
    */

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


/* =========================================================
   SEND LOCATION
   ========================================================= */

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
        Serial.println(
            "GSM is not ready."
        );

        return false;
    }


    String timestamp =
        buildTimestamp(
            gpsTime,
            gpsDate
        );


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


    const char *status =
        gpsAvailable
            ? "LOCATION_AVAILABLE"
            : "SENSOR_ONLY";


    const char *source =
        gpsAvailable
            ? "device_gps"
            : "device_sensor";


    /* -----------------------------------------------------
       HISTORY / CURRENT PAYLOAD
       ----------------------------------------------------- */

    String currentPayload = "{";


    if (gpsAvailable)
    {
        currentPayload +=
            "\"latitude\":" +
            String(latitude, 6) +
            ",";


        currentPayload +=
            "\"longitude\":" +
            String(longitude, 6) +
            ",";


        currentPayload +=
            "\"altitude\":" +
            String(altitude, 1) +
            ",";


        currentPayload +=
            "\"gpsTime\":\"" +
            gpsTime +
            "\",";


        currentPayload +=
            "\"gpsDate\":\"" +
            gpsDate +
            "\",";
    }


    currentPayload +=
        "\"timestamp\":\"" +
        timestamp +
        "\",";


    currentPayload +=
        "\"source\":\"" +
        String(source) +
        "\",";


    currentPayload +=
        "\"status\":\"" +
        String(status) +
        "\",";


    currentPayload +=
        "\"motion\":\"" +
        String(motion) +
        "\",";


    currentPayload +=
        "\"trackerMode\":\"" +
        String(trackerMode) +
        "\",";


    currentPayload +=
        "\"battery\":" +
        String(battery) +
        ",";


    currentPayload +=
        "\"acceleration\":{";


    currentPayload +=
        "\"x\":" +
        String(accelX, 3) +
        ",";


    currentPayload +=
        "\"y\":" +
        String(accelY, 3) +
        ",";


    currentPayload +=
        "\"z\":" +
        String(accelZ, 3);


    currentPayload += "},";


    currentPayload +=
        "\"gyroscope\":{";


    currentPayload +=
        "\"x\":" +
        String(gyroX, 3) +
        ",";


    currentPayload +=
        "\"y\":" +
        String(gyroY, 3) +
        ",";


    currentPayload +=
        "\"z\":" +
        String(gyroZ, 3);


    currentPayload += "}";


    currentPayload += "}";


    /* -----------------------------------------------------
       STATUS PAYLOAD
       ----------------------------------------------------- */

    String statusPayload = "{";


    statusPayload +=
        "\"status\":\"" +
        String(status) +
        "\",";


    statusPayload +=
        "\"source\":\"" +
        String(source) +
        "\",";


    statusPayload +=
        "\"motion\":\"" +
        String(motion) +
        "\",";


    statusPayload +=
        "\"trackerMode\":\"" +
        String(trackerMode) +
        "\",";


    statusPayload +=
        "\"battery\":" +
        String(battery) +
        ",";


    statusPayload +=
        "\"timestamp\":\"" +
        timestamp +
        "\",";


    statusPayload +=
        "\"acceleration\":{";


    statusPayload +=
        "\"x\":" +
        String(accelX, 3) +
        ",";


    statusPayload +=
        "\"y\":" +
        String(accelY, 3) +
        ",";


    statusPayload +=
        "\"z\":" +
        String(accelZ, 3);


    statusPayload += "},";


    statusPayload +=
        "\"gyroscope\":{";


    statusPayload +=
        "\"x\":" +
        String(gyroX, 3) +
        ",";


    statusPayload +=
        "\"y\":" +
        String(gyroY, 3) +
        ",";


    statusPayload +=
        "\"z\":" +
        String(gyroZ, 3);


    statusPayload += "}";


    statusPayload += "}";


    /* -----------------------------------------------------
       HISTORY
       ----------------------------------------------------- */

    bool historySuccess =
        saveHistoryOrQueue(
            currentPayload
        );


    bool currentSuccess = false;
    bool statusSuccess = false;


    /* -----------------------------------------------------
       CURRENT + STATUS

       These are intentionally NOT queued offline.

       Only history is queued.
       ----------------------------------------------------- */

    if (hasNetworkConnection())
    {
        currentSuccess =
            sendFirebaseRequest(
                currentURL,
                currentPayload,
                2
            );


        delay(500);


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
        Serial.println(
            "Network unavailable."
        );

        Serial.println(
            "Current/status NOT updated."
        );
    }


    /* -----------------------------------------------------
       RESULTS
       ----------------------------------------------------- */

    Serial.println();

    Serial.println(
        "Firebase tracker upload results:"
    );


    Serial.print(
        "History: "
    );

    Serial.println(
        historySuccess
            ? "UPLOADED / QUEUED"
            : "FAILED"
    );


    Serial.print(
        "Current: "
    );

    Serial.println(
        currentSuccess
            ? "SUCCESS"
            : "NOT UPDATED"
    );


    Serial.print(
        "Status: "
    );

    Serial.println(
        statusSuccess
            ? "SUCCESS"
            : "NOT UPDATED"
    );


    return historySuccess;
}


/* =========================================================
   GPS NO FIX LOG
   ========================================================= */

bool logGPSNoFix(
    String gpsTime,
    String gpsDate
)
{
    if (!gsmReady)
    {
        Serial.println(
            "GSM is not ready."
        );

        return false;
    }


    String timestamp =
        buildTimestamp(
            gpsTime,
            gpsDate
        );


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


    String payload = "{";


    payload +=
        "\"status\":\"NO_FIX\",";


    payload +=
        "\"source\":\"device_sensor\",";


    payload +=
        "\"gpsTime\":\"" +
        gpsTime +
        "\",";


    payload +=
        "\"gpsDate\":\"" +
        gpsDate +
        "\",";


    payload +=
        "\"timestamp\":\"" +
        timestamp +
        "\",";


    payload +=
        "\"motion\":\"" +
        String(motion) +
        "\",";


    payload +=
        "\"trackerMode\":\"" +
        String(trackerMode) +
        "\",";


    payload +=
        "\"battery\":" +
        String(battery) +
        ",";


    payload +=
        "\"acceleration\":{";


    payload +=
        "\"x\":" +
        String(accelX, 3) +
        ",";


    payload +=
        "\"y\":" +
        String(accelY, 3) +
        ",";


    payload +=
        "\"z\":" +
        String(accelZ, 3);


    payload += "},";


    payload +=
        "\"gyroscope\":{";


    payload +=
        "\"x\":" +
        String(gyroX, 3) +
        ",";


    payload +=
        "\"y\":" +
        String(gyroY, 3) +
        ",";


    payload +=
        "\"z\":" +
        String(gyroZ, 3);


    payload += "}";


    payload += "}";


    Serial.println();
    Serial.println(
        "NO_FIX PAYLOAD:"
    );

    Serial.println(payload);


    bool success =
        saveHistoryOrQueue(
            payload
        );


    if (success)
    {
        Serial.println(
            "GPS NO_FIX HISTORY "
            "SAVED/QUEUED."
        );
    }
    else
    {
        Serial.println(
            "FAILED TO SAVE GPS "
            "NO_FIX RECORD."
        );
    }


    return success;
}