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

const char *statusURL =
    "https://smart-trackerr-default-rtdb.firebaseio.com/Trackers/tracker_001/status.json";


// ======================================================
// INITIALIZE GSM
// ======================================================

void initGSM()
{
    Serial.println();
    Serial.println("========== GSM INITIALIZATION ==========");

    sendSIM7600Command(
        "AT+CPIN?",
        1000
    );

    sendSIM7600Command(
        "AT+CSQ",
        1000
    );

    sendSIM7600Command(
        "AT+CEREG?",
        1000
    );

    sendSIM7600Command(
        "AT+CGATT?",
        1000
    );

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
// FIREBASE HTTP REQUEST
// ======================================================
//
// method:
// 1 = POST
// 2 = PUT
//
// IMPORTANT:
//
// SIM7600 AT+HTTPACTION supports:
// 0 = GET
// 1 = POST
// 2 = HEAD
// 3 = DELETE
//
// Therefore we CANNOT use:
// AT+HTTPACTION=2
//
// For PUT we use:
// POST + X-HTTP-Method-Override: PUT
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
        Serial.println(
            "GSM is not ready."
        );

        return false;
    }


    // ==================================================
    // DISPLAY REQUEST
    // ==================================================

    Serial.println();
    Serial.println(
        "================================"
    );

    Serial.println(
        "FIREBASE REQUEST"
    );

    Serial.println(
        "================================"
    );

    Serial.print(
        "URL: "
    );

    Serial.println(
        url
    );

    Serial.print(
        "Payload: "
    );

    Serial.println(
        payload
    );


    if (method == 1)
    {
        Serial.println(
            "Method: POST"
        );
    }
    else
    {
        Serial.println(
            "Method: PUT"
        );

        Serial.println(
            "Transport: POST + X-HTTP-Method-Override"
        );
    }


    // ==================================================
    // CLEAN UART
    // ==================================================

    while (
        sim7600.available()
    )
    {
        sim7600.read();
    }


    // ==================================================
    // END PREVIOUS HTTP SESSION
    // ==================================================

    sim7600.println(
        "AT+HTTPTERM"
    );

    delay(500);

    while (
        sim7600.available()
    )
    {
        Serial.write(
            sim7600.read()
        );
    }


    // ==================================================
    // INITIALIZE HTTP
    // ==================================================

    sim7600.println(
        "AT+HTTPINIT"
    );

    if (
        !waitForResponse(
            "OK",
            3000
        )
    )
    {
        Serial.println(
            "ERROR: HTTPINIT failed."
        );

        return false;
    }


    // ==================================================
    // CID
    // ==================================================

    sim7600.println(
        "AT+HTTPPARA=\"CID\",1"
    );

    if (
        !waitForResponse(
            "OK",
            3000
        )
    )
    {
        Serial.println(
            "ERROR: CID failed."
        );

        sim7600.println(
            "AT+HTTPTERM"
        );

        return false;
    }


    // ==================================================
    // URL
    // ==================================================

    String urlCommand =
        "AT+HTTPPARA=\"URL\",\"" +
        String(url) +
        "\"";

    sim7600.println(
        urlCommand
    );

    if (
        !waitForResponse(
            "OK",
            5000
        )
    )
    {
        Serial.println(
            "ERROR: URL setup failed."
        );

        sim7600.println(
            "AT+HTTPTERM"
        );

        return false;
    }


    // ==================================================
    // CONTENT TYPE
    // ==================================================

    sim7600.println(
        "AT+HTTPPARA=\"CONTENT\",\"application/json\""
    );

    if (
        !waitForResponse(
            "OK",
            3000
        )
    )
    {
        Serial.println(
            "ERROR: Content type failed."
        );

        sim7600.println(
            "AT+HTTPTERM"
        );

        return false;
    }


    // ==================================================
    // HTTP METHOD OVERRIDE
    // ==================================================
    //
    // Only use this for logical PUT requests.
    //
    // The actual SIM7600 transport method remains POST.
    //
    // ==================================================

    if (
        method == 2
    )
    {
        Serial.println(
            "Setting HTTP PUT override header..."
        );

        sim7600.println(
            "AT+HTTPPARA=\"USERDATA\",\"X-HTTP-Method-Override: PUT\""
        );

        if (
            !waitForResponse(
                "OK",
                3000
            )
        )
        {
            Serial.println(
                "ERROR: PUT override header failed."
            );

            sim7600.println(
                "AT+HTTPTERM"
            );

            return false;
        }
    }


    // ==================================================
    // CLEAR UART
    // ==================================================

    while (
        sim7600.available()
    )
    {
        sim7600.read();
    }


    // ==================================================
    // HTTP DATA
    // ==================================================

    String httpDataCommand =
        "AT+HTTPDATA=" +
        String(
            payload.length()
        ) +
        ",15000";

    Serial.print(
        ">> "
    );

    Serial.println(
        httpDataCommand
    );

    sim7600.println(
        httpDataCommand
    );


    // ==================================================
    // WAIT FOR DOWNLOAD
    // ==================================================

    if (
        !waitForResponse(
            "DOWNLOAD",
            5000
        )
    )
    {
        Serial.println(
            "ERROR: DOWNLOAD prompt not received."
        );

        sim7600.println(
            "AT+HTTPTERM"
        );

        return false;
    }


    // ==================================================
    // SEND JSON
    // ==================================================

    Serial.println(
        "Sending Firebase JSON..."
    );

    Serial.println(
        payload
    );

    sim7600.print(
        payload
    );


    // ==================================================
    // WAIT FOR PAYLOAD OK
    // ==================================================

    if (
        !waitForResponse(
            "OK",
            15000
        )
    )
    {
        Serial.println(
            "ERROR: Firebase payload was not accepted."
        );

        sim7600.println(
            "AT+HTTPTERM"
        );

        return false;
    }

    Serial.println(
        "Firebase payload accepted."
    );


    // ==================================================
    // CLEAR UART
    // ==================================================

    while (
        sim7600.available()
    )
    {
        sim7600.read();
    }


    // ==================================================
    // HTTP ACTION
    // ==================================================
    //
    // IMPORTANT:
    //
    // Both POST and logical PUT use:
    //
    // AT+HTTPACTION=1
    //
    // The difference is the HTTP header.
    //
    // ==================================================

    if (
        method == 1
    )
    {
        Serial.println(
            "Sending HTTP POST..."
        );
    }
    else
    {
        Serial.println(
            "Sending HTTP PUT via POST override..."
        );
    }

    sim7600.println(
        "AT+HTTPACTION=1"
    );


    // ==================================================
    // WAIT FOR HTTPACTION
    // ==================================================

    String response = "";

    unsigned long start =
        millis();

    while (
        millis() - start < 30000
    )
    {
        while (
            sim7600.available()
        )
        {
            char c =
                (char)sim7600.read();

            response += c;

            Serial.write(
                c
            );
        }

        if (
            response.indexOf(
                "+HTTPACTION:"
            ) != -1
        )
        {
            break;
        }
    }


    // ==================================================
    // DISPLAY HTTP ACTION RESPONSE
    // ==================================================

    Serial.println();

    Serial.println(
        "HTTP ACTION RESPONSE:"
    );

    Serial.println(
        response
    );


    // ==================================================
    // PARSE HTTP RESULT
    // ==================================================

    bool success =
        false;

    int actionIndex =
        response.indexOf(
            "+HTTPACTION:"
        );

    if (
        actionIndex != -1
    )
    {
        String action =
            response.substring(
                actionIndex
            );

        Serial.print(
            "Parsed HTTP ACTION: "
        );

        Serial.println(
            action
        );


        // ==================================================
        // HTTP 200
        // ==================================================

        if (
            action.indexOf(
                ",200,"
            ) != -1
        )
        {
            success = true;
        }


        // ==================================================
        // HTTP 201
        // ==================================================

        if (
            action.indexOf(
                ",201,"
            ) != -1
        )
        {
            success = true;
        }


        // ==================================================
        // HTTP 204
        // ==================================================

        if (
            action.indexOf(
                ",204,"
            ) != -1
        )
        {
            success = true;
        }
    }


    // ==================================================
    // RESULT
    // ==================================================

    if (
        success
    )
    {
        Serial.println();

        Serial.println(
            "FIREBASE REQUEST SUCCESS."
        );


        // ==================================================
        // READ FIREBASE RESPONSE
        // ==================================================

        sim7600.println(
            "AT+HTTPREAD"
        );

        delay(1000);

        while (
            sim7600.available()
        )
        {
            Serial.write(
                sim7600.read()
            );
        }
    }
    else
    {
        Serial.println();

        Serial.println(
            "FIREBASE REQUEST FAILED."
        );
    }


    // ==================================================
    // CLOSE HTTP SESSION
    // ==================================================

    sim7600.println(
        "AT+HTTPTERM"
    );

    delay(500);

    while (
        sim7600.available()
    )
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
        gpsDate.substring(
            0,
            2
        );

    String month =
        gpsDate.substring(
            2,
            4
        );

    String year =
        gpsDate.substring(
            4,
            6
        );

    String hour =
        gpsTime.substring(
            0,
            2
        );

    String minute =
        gpsTime.substring(
            2,
            4
        );

    String second =
        gpsTime.substring(
            4,
            6
        );


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
// SEND LOCATION
// ======================================================

bool sendLocation(
    float latitude,
    float longitude,
    float altitude,
    String gpsTime,
    String gpsDate
)
{
    if (
        !gsmReady
    )
    {
        Serial.println(
            "GSM is not ready."
        );

        return false;
    }


    // ==================================================
    // TIMESTAMP
    // ==================================================

    String timestamp =
        buildTimestamp(
            gpsTime,
            gpsDate
        );


    // ==================================================
    // LOCATION PAYLOAD
    // ==================================================

    String locationPayload =
        "{"
        "\"latitude\":" +
        String(
            latitude,
            6
        ) +
        ","
        "\"longitude\":" +
        String(
            longitude,
            6
        ) +
        ","
        "\"altitude\":" +
        String(
            altitude,
            1
        ) +
        ","
        "\"gpsTime\":\"" +
        gpsTime +
        "\","
        "\"timestamp\":\"" +
        timestamp +
        "\","
        "\"source\":\"device_gps\","
        "\"status\":\"LOCATION_AVAILABLE\""
        "}";


    // ==================================================
    // STATUS PAYLOAD
    // ==================================================

    String statusPayload =
        "{"
        "\"status\":\"LOCATION_AVAILABLE\","
        "\"source\":\"device_gps\","
        "\"timestamp\":\"" +
        timestamp +
        "\""
        "}";


    Serial.println();

    Serial.println(
        "================================"
    );

    Serial.println(
        "GPS LOCATION AVAILABLE"
    );

    Serial.println(
        locationPayload
    );

    Serial.println(
        "================================"
    );


    // ==================================================
    // 1. HISTORY
    // ==================================================

    Serial.println(
        "Saving location to HISTORY..."
    );

    bool historySuccess =
        sendFirebaseRequest(
            historyURL,
            locationPayload,
            1
        );

    delay(
        1000
    );


    // ==================================================
    // 2. CURRENT
    // ==================================================

    Serial.println(
        "Updating CURRENT location..."
    );

    bool currentSuccess =
        sendFirebaseRequest(
            currentURL,
            locationPayload,
            2
        );

    delay(
        1000
    );


    // ==================================================
    // 3. STATUS
    // ==================================================

    Serial.println(
        "Updating STATUS..."
    );

    bool statusSuccess =
        sendFirebaseRequest(
            statusURL,
            statusPayload,
            2
        );


    // ==================================================
    // RESULTS
    // ==================================================

    Serial.println();

    Serial.println(
        "Firebase GPS upload results:"
    );


    Serial.print(
        "History: "
    );

    Serial.println(
        historySuccess
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


    if (
        historySuccess &&
        currentSuccess &&
        statusSuccess
    )
    {
        Serial.println();

        Serial.println(
            "GPS LOCATION UPDATE SUCCESSFUL."
        );
    }
    else
    {
        Serial.println();

        Serial.println(
            "GPS LOCATION UPDATE FAILED."
        );
    }


    return
        historySuccess &&
        currentSuccess &&
        statusSuccess;
}


// ======================================================
// LOG GPS NO FIX
// ======================================================

bool logGPSNoFix(
    String gpsTime,
    String gpsDate
)
{
    if (
        !gsmReady
    )
    {
        Serial.println(
            "GSM is not ready."
        );

        return false;
    }


    // ==================================================
    // TIMESTAMP
    // ==================================================

    String timestamp =
        buildTimestamp(
            gpsTime,
            gpsDate
        );


    // ==================================================
    // HISTORY PAYLOAD
    // ==================================================

    String historyPayload =
        "{"
        "\"status\":\"NO_FIX\","
        "\"source\":\"device_gps\","
        "\"reason\":\"no_satellite_fix\","
        "\"timestamp\":\"" +
        timestamp +
        "\""
        "}";


    // ==================================================
    // STATUS PAYLOAD
    // ==================================================

    String statusPayload =
        "{"
        "\"status\":\"NO_FIX\","
        "\"source\":\"device_gps\","
        "\"reason\":\"no_satellite_fix\","
        "\"timestamp\":\"" +
        timestamp +
        "\""
        "}";


    Serial.println();

    Serial.println(
        "================================"
    );

    Serial.println(
        "GPS NO FIX"
    );

    Serial.println(
        "Logging GPS failure..."
    );

    Serial.println(
        historyPayload
    );

    Serial.println(
        "================================"
    );


    // ==================================================
    // HISTORY
    // ==================================================

    bool historySuccess =
        sendFirebaseRequest(
            historyURL,
            historyPayload,
            1
        );

    delay(
        1000
    );


    // ==================================================
    // STATUS
    // ==================================================

    bool statusSuccess =
        sendFirebaseRequest(
            statusURL,
            statusPayload,
            2
        );


    // ==================================================
    // CURRENT IS NOT TOUCHED
    // ==================================================
    //
    // When GPS has no fix, /current keeps the
    // last known valid GPS position.
    //
    // ==================================================


    Serial.println();

    Serial.println(
        "GPS NO FIX upload results:"
    );


    Serial.print(
        "History: "
    );

    Serial.println(
        historySuccess
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


    return
        historySuccess &&
        statusSuccess;
}