#include "gsm.h"
#include "sim7600.h"

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

// ======================================================
// INITIALIZE GSM
// ======================================================

void initGSM()
{
    Serial.println();
    Serial.println("========== GSM INITIALIZATION ==========");

    sendSIM7600Command("AT+CPIN?", 1000);

    sendSIM7600Command("AT+CSQ", 1000);

    sendSIM7600Command("AT+CEREG?", 1000);

    sendSIM7600Command("AT+CGATT?", 1000);

    sendSIM7600Command(
        "AT+CGDCONT=1,\"IP\",\"web.gprs.mtnnigeria.net\"",
        1000
    );

    sendSIM7600Command(
        "AT+CGACT=1,1",
        5000
    );

    sendSIM7600Command(
        "AT+CGPADDR=1",
        2000
    );

    gsmReady = true;

    Serial.println();
    Serial.println("GSM initialization complete.");
}

// ======================================================
// GSM STATUS
// ======================================================

bool isGSMReady()
{
    return gsmReady;
}

// ======================================================
// SEND FIREBASE HTTP REQUEST
// ======================================================

bool sendFirebaseRequest(
    const char *url,
    const String &payload,
    int method
)
{
    Serial.println();
    Serial.println("--------------------------------");
    Serial.println("Firebase HTTP Request");
    Serial.println("--------------------------------");

    Serial.print("URL: ");
    Serial.println(url);

    Serial.print("Payload: ");
    Serial.println(payload);

    if (method == 1)
    {
        Serial.println("Method: POST");
    }
    else if (method == 2)
    {
        Serial.println("Method: PUT");
    }
    else
    {
        Serial.println("Invalid HTTP method.");
        return false;
    }

    // Close previous HTTP session
    sendSIM7600Command(
        "AT+HTTPTERM",
        1000
    );

    // Initialize HTTP
    sendSIM7600Command(
        "AT+HTTPINIT",
        2000
    );

    // PDP context
    sendSIM7600Command(
        "AT+HTTPPARA=\"CID\",1",
        1000
    );

    // URL
    String urlCommand =
        "AT+HTTPPARA=\"URL\",\"" +
        String(url) +
        "\"";

    sendSIM7600Command(
        urlCommand.c_str(),
        2000
    );

    // Content type
    sendSIM7600Command(
        "AT+HTTPPARA=\"CONTENT\",\"application/json\"",
        1000
    );

    // Payload size
    String httpDataCommand =
        "AT+HTTPDATA=" +
        String(payload.length()) +
        ",10000";

    sendSIM7600Command(
        httpDataCommand.c_str(),
        2000
    );

    // Send JSON
    Serial.println("Sending JSON...");

    sim7600.print(payload);

    delay(1000);

    // HTTP action
    if (method == 1)
    {
        Serial.println("HTTP POST...");

        sendSIM7600Command(
            "AT+HTTPACTION=1",
            15000
        );
    }
    else
    {
        Serial.println("HTTP PUT...");

        sendSIM7600Command(
            "AT+HTTPACTION=2",
            15000
        );
    }

    // Read response
    sendSIM7600Command(
        "AT+HTTPREAD",
        3000
    );

    // Close HTTP
    sendSIM7600Command(
        "AT+HTTPTERM",
        1000
    );

    Serial.println("--------------------------------");

    return true;
}

// ======================================================
// BUILD TIMESTAMP
// ======================================================

String buildTimestamp(
    String gpsTime,
    String gpsDate
)
{
    if (gpsDate.length() < 6)
    {
        return "unknown";
    }

    if (gpsTime.length() < 6)
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

    return
        "20" +
        year +
        "-" +
        month +
        "-" +
        day +
        "T" +
        hour +
        ":" +
        minute +
        ":" +
        second +
        "Z";
}

// ======================================================
// SEND VALID GPS LOCATION
// ======================================================

bool sendLocation(
    float latitude,
    float longitude,
    float altitude,
    String gpsTime,
    String gpsDate
)
{
    if (!gsmReady)
    {
        Serial.println("GSM is not ready.");
        return false;
    }

    String timestamp =
        buildTimestamp(
            gpsTime,
            gpsDate
        );

    String payload =
        "{"
        "\"latitude\":" +
        String(latitude, 6) +
        "," 
        "\"longitude\":" +
        String(longitude, 6) +
        ","
        "\"altitude\":" +
        String(altitude, 1) +
        ","
        "\"timestamp\":\"" +
        timestamp +
        "\","
        "\"source\":\"device_gps\","
        "\"status\":\"LOCATION_AVAILABLE\""
        "}";

    Serial.println();
    Serial.println("DEVICE GPS LOCATION AVAILABLE");
    Serial.println(payload);

    // -----------------------------------------------
    // HISTORY
    // -----------------------------------------------

    bool historySuccess =
        sendFirebaseRequest(
            historyURL,
            payload,
            1
        );

    // -----------------------------------------------
    // CURRENT
    // -----------------------------------------------

    bool currentSuccess =
        sendFirebaseRequest(
            currentURL,
            payload,
            2
        );

    return
        historySuccess &&
        currentSuccess;
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
        Serial.println(
            "GSM is not ready. Cannot log GPS NO_FIX."
        );

        return false;
    }

    String timestamp =
        buildTimestamp(
            gpsTime,
            gpsDate
        );

    String payload =
        "{"
        "\"status\":\"NO_FIX\","
        "\"source\":\"device_gps\","
        "\"reason\":\"no_satellite_fix\","
        "\"timestamp\":\"" +
        timestamp +
        "\""
        "}";

    Serial.println();
    Serial.println("DEVICE GPS HAS NO FIX");
    Serial.println("Logging GPS failure...");
    Serial.println(payload);

    // -----------------------------------------------
    // HISTORY
    // -----------------------------------------------

    bool historySuccess =
        sendFirebaseRequest(
            historyURL,
            payload,
            1
        );

    // -----------------------------------------------
    // CURRENT
    // -----------------------------------------------

    bool currentSuccess =
        sendFirebaseRequest(
            currentURL,
            payload,
            2
        );

    return
        historySuccess &&
        currentSuccess;
}