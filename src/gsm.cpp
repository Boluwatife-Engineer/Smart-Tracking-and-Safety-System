#include "gsm.h"
#include "sim7600.h"

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
// INITIALIZE GSM
// ======================================================

void initGSM()
{
    Serial.println();
    Serial.println("========== GSM INITIALIZATION ==========");


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
    // PACKET DOMAIN ATTACHMENT
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
    // ACTIVATE PDP CONTEXT
    // ==================================================

    sendSIM7600Command(
        "AT+CGACT=1,1",
        5000
    );


    // ==================================================
    // GET IP ADDRESS
    // ==================================================

    sendSIM7600Command(
        "AT+CGPADDR=1",
        2000
    );


    // ==================================================
    // SSL CONFIGURATION
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


    // ==================================================
    // CHECK SSL
    // ==================================================

    sendSIM7600Command(
        "AT+CSSLCFG?",
        3000
    );


    // ==================================================
    // GSM READY
    // ==================================================

    gsmReady = true;


    Serial.println();
    Serial.println(
        "GSM initialization complete."
    );

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


    while (
        millis() - start < timeout
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


            if (
                response.indexOf(
                    expected
                ) != -1
            )
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
//
// 1 = POST
// 2 = PUT
//
// SIM7600 HTTPACTION:
//
// 0 = GET
// 1 = POST
// 2 = HEAD
// 3 = DELETE
//
// Therefore logical PUT is implemented using:
//
// POST
//
// with:
//
// X-HTTP-Method-Override: PUT
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


    if (
        method == 1
    )
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
    // CLEAR UART
    // ==================================================

    while (
        sim7600.available()
    )
    {
        sim7600.read();
    }


    // ==================================================
    // TERMINATE PREVIOUS HTTP SESSION
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
    // HTTP INIT
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
        "AT+HTTPPARA=\"URL\",\"";


    urlCommand +=
        String(url);


    urlCommand +=
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
    // PUT OVERRIDE
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
        "AT+HTTPDATA=";


    httpDataCommand +=
        String(
            payload.length()
        );


    httpDataCommand +=
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
    // WAIT DOWNLOAD
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
    // WAIT PAYLOAD ACCEPTANCE
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
    // WAIT HTTP ACTION
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
    // DISPLAY RESPONSE
    // ==================================================

    Serial.println();

    Serial.println(
        "HTTP ACTION RESPONSE:"
    );

    Serial.println(
        response
    );


    // ==================================================
    // PARSE RESULT
    // ==================================================

    bool success = false;


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


        // HTTP 200

        if (
            action.indexOf(
                ",200,"
            ) != -1
        )
        {
            success = true;
        }


        // HTTP 201

        if (
            action.indexOf(
                ",201,"
            ) != -1
        )
        {
            success = true;
        }


        // HTTP 204

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
    // CLOSE HTTP
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


    String timestamp =
        "20";


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
// SEND LOCATION + TRACKER STATE
// ======================================================
//
// gpsAvailable:
//
// true
//  -> GPS + sensors
//
// false
//  -> sensors only
//
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
    // DISPLAY
    // ==================================================

    Serial.println();
    Serial.println(
        "================================"
    );

    Serial.println(
        "TRACKER STATE"
    );

    Serial.println(
        "================================"
    );


    Serial.print(
        "GPS: "
    );

    Serial.println(
        gpsAvailable
            ? "AVAILABLE"
            : "NOT AVAILABLE"
    );


    Serial.print(
        "Motion: "
    );

    Serial.println(
        motion
    );


    Serial.print(
        "Tracker Mode: "
    );

    Serial.println(
        trackerMode
    );


    Serial.print(
        "Battery: "
    );

    Serial.print(
        battery
    );

    Serial.println(
        "%"
    );


    Serial.print(
        "Accel X: "
    );

    Serial.println(
        accelX,
        2
    );


    Serial.print(
        "Accel Y: "
    );

    Serial.println(
        accelY,
        2
    );


    Serial.print(
        "Accel Z: "
    );

    Serial.println(
        accelZ,
        2
    );


    Serial.print(
        "Gyro X: "
    );

    Serial.println(
        gyroX,
        2
    );


    Serial.print(
        "Gyro Y: "
    );

    Serial.println(
        gyroY,
        2
    );


    Serial.print(
        "Gyro Z: "
    );

    Serial.println(
        gyroZ,
        2
    );


    // ==================================================
    // BUILD CURRENT PAYLOAD
    // ==================================================

    String currentPayload = "{";


    // ==================================================
    // GPS DATA
    // ==================================================

    if (
        gpsAvailable
    )
    {
        currentPayload +=
            "\"latitude\":";


        currentPayload +=
            String(
                latitude,
                6
            );


        currentPayload +=
            ",";


        currentPayload +=
            "\"longitude\":";


        currentPayload +=
            String(
                longitude,
                6
            );


        currentPayload +=
            ",";


        currentPayload +=
            "\"altitude\":";


        currentPayload +=
            String(
                altitude,
                1
            );


        currentPayload +=
            ",";


        currentPayload +=
            "\"gpsTime\":\"";


        currentPayload +=
            gpsTime;


        currentPayload +=
            "\",";


        currentPayload +=
            "\"gpsDate\":\"";


        currentPayload +=
            gpsDate;


        currentPayload +=
            "\",";
    }


    // ==================================================
    // COMMON DATA
    // ==================================================

    currentPayload +=
        "\"timestamp\":\"";


    currentPayload +=
        timestamp;


    currentPayload +=
        "\",";


    currentPayload +=
        "\"source\":\"";


    currentPayload +=
        source;


    currentPayload +=
        "\",";


    currentPayload +=
        "\"status\":\"";


    currentPayload +=
        status;


    currentPayload +=
        "\",";


    currentPayload +=
        "\"motion\":\"";


    currentPayload +=
        motion;


    currentPayload +=
        "\",";


    currentPayload +=
        "\"trackerMode\":\"";


    currentPayload +=
        trackerMode;


    currentPayload +=
        "\",";


    currentPayload +=
        "\"battery\":";


    currentPayload +=
        String(
            battery
        );


    currentPayload +=
        ",";


    // ==================================================
    // ACCELEROMETER
    // ==================================================

    currentPayload +=
        "\"acceleration\":{";


    currentPayload +=
        "\"x\":";


    currentPayload +=
        String(
            accelX,
            3
        );


    currentPayload +=
        ",";


    currentPayload +=
        "\"y\":";


    currentPayload +=
        String(
            accelY,
            3
        );


    currentPayload +=
        ",";


    currentPayload +=
        "\"z\":";


    currentPayload +=
        String(
            accelZ,
            3
        );


    currentPayload +=
        "},";


    // ==================================================
    // GYROSCOPE
    // ==================================================

    currentPayload +=
        "\"gyroscope\":{";


    currentPayload +=
        "\"x\":";


    currentPayload +=
        String(
            gyroX,
            3
        );


    currentPayload +=
        ",";


    currentPayload +=
        "\"y\":";


    currentPayload +=
        String(
            gyroY,
            3
        );


    currentPayload +=
        ",";


    currentPayload +=
        "\"z\":";


    currentPayload +=
        String(
            gyroZ,
            3
        );


    currentPayload +=
        "}";


    // ==================================================
    // CLOSE JSON
    // ==================================================

    currentPayload +=
        "}";


    // ==================================================
    // STATUS PAYLOAD
    // ==================================================

    String statusPayload = "{";


    statusPayload +=
        "\"status\":\"";


    statusPayload +=
        status;


    statusPayload +=
        "\",";


    statusPayload +=
        "\"source\":\"";


    statusPayload +=
        source;


    statusPayload +=
        "\",";


    statusPayload +=
        "\"motion\":\"";


    statusPayload +=
        motion;


    statusPayload +=
        "\",";


    statusPayload +=
        "\"trackerMode\":\"";


    statusPayload +=
        trackerMode;


    statusPayload +=
        "\",";


    statusPayload +=
        "\"battery\":";


    statusPayload +=
        String(
            battery
        );


    statusPayload +=
        ",";


    statusPayload +=
        "\"timestamp\":\"";


    statusPayload +=
        timestamp;


    statusPayload +=
        "\",";


    // ==================================================
    // STATUS ACCELERATION
    // ==================================================

    statusPayload +=
        "\"acceleration\":{";


    statusPayload +=
        "\"x\":";


    statusPayload +=
        String(
            accelX,
            3
        );


    statusPayload +=
        ",";


    statusPayload +=
        "\"y\":";


    statusPayload +=
        String(
            accelY,
            3
        );


    statusPayload +=
        ",";


    statusPayload +=
        "\"z\":";


    statusPayload +=
        String(
            accelZ,
            3
        );


    statusPayload +=
        "},";


    // ==================================================
    // STATUS GYROSCOPE
    // ==================================================

    statusPayload +=
        "\"gyroscope\":{";


    statusPayload +=
        "\"x\":";


    statusPayload +=
        String(
            gyroX,
            3
        );


    statusPayload +=
        ",";


    statusPayload +=
        "\"y\":";


    statusPayload +=
        String(
            gyroY,
            3
        );


    statusPayload +=
        ",";


    statusPayload +=
        "\"z\":";


    statusPayload +=
        String(
            gyroZ,
            3
        );


    statusPayload +=
        "}";


    // ==================================================
    // CLOSE STATUS JSON
    // ==================================================

    statusPayload +=
        "}";


    // ==================================================
    // DISPLAY PAYLOAD
    // ==================================================

    Serial.println();

    Serial.println(
        "================================"
    );

    Serial.println(
        "FIREBASE TRACKER UPDATE"
    );

    Serial.println(
        "================================"
    );

    Serial.println(
        currentPayload
    );

    Serial.println(
        "================================"
    );


    // ==================================================
    // HISTORY
    // ==================================================

    Serial.println(
        "Saving tracker state to HISTORY..."
    );


    bool historySuccess =
        sendFirebaseRequest(
            historyURL,
            currentPayload,
            1
        );


    delay(1000);


    // ==================================================
    // CURRENT
    // ==================================================

    Serial.println(
        "Updating CURRENT tracker state..."
    );


    bool currentSuccess =
        sendFirebaseRequest(
            currentURL,
            currentPayload,
            2
        );


    delay(1000);


    // ==================================================
    // STATUS
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
        "Firebase tracker upload results:"
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
            "COMPLETE TRACKER UPDATE SUCCESSFUL."
        );
    }
    else
    {
        Serial.println();

        Serial.println(
            "COMPLETE TRACKER UPDATE FAILED."
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
//
// Saves sensor information even when GPS has no fix.
//
// Saves:
//
// - GPS status
// - GPS date
// - GPS time
// - timestamp
// - motion
// - tracker mode
// - battery
// - accelerometer
// - gyroscope
//
// ======================================================

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
    // DISPLAY
    // ==================================================

    Serial.println();

    Serial.println(
        "================================"
    );

    Serial.println(
        "GPS NO-FIX TRACKER STATE"
    );

    Serial.println(
        "================================"
    );


    Serial.print(
        "GPS Date: "
    );

    Serial.println(
        gpsDate
    );


    Serial.print(
        "GPS Time: "
    );

    Serial.println(
        gpsTime
    );


    Serial.print(
        "Motion: "
    );

    Serial.println(
        motion
    );


    Serial.print(
        "Tracker Mode: "
    );

    Serial.println(
        trackerMode
    );


    Serial.print(
        "Battery: "
    );

    Serial.print(
        battery
    );

    Serial.println(
        "%"
    );


    Serial.print(
        "Accel X: "
    );

    Serial.println(
        accelX,
        2
    );


    Serial.print(
        "Accel Y: "
    );

    Serial.println(
        accelY,
        2
    );


    Serial.print(
        "Accel Z: "
    );

    Serial.println(
        accelZ,
        2
    );


    Serial.print(
        "Gyro X: "
    );

    Serial.println(
        gyroX,
        2
    );


    Serial.print(
        "Gyro Y: "
    );

    Serial.println(
        gyroY,
        2
    );


    Serial.print(
        "Gyro Z: "
    );

    Serial.println(
        gyroZ,
        2
    );


    // ==================================================
    // BUILD PAYLOAD
    // ==================================================

    String payload = "{";


    // ==================================================
    // GPS STATUS
    // ==================================================

    payload +=
        "\"status\":\"NO_FIX\",";


    payload +=
        "\"source\":\"device_sensor\",";


    payload +=
        "\"gpsTime\":\"";


    payload +=
        gpsTime;


    payload +=
        "\",";


    payload +=
        "\"gpsDate\":\"";


    payload +=
        gpsDate;


    payload +=
        "\",";


    payload +=
        "\"timestamp\":\"";


    payload +=
        timestamp;


    payload +=
        "\",";


    // ==================================================
    // MOTION
    // ==================================================

    payload +=
        "\"motion\":\"";


    payload +=
        motion;


    payload +=
        "\",";


    // ==================================================
    // TRACKER MODE
    // ==================================================

    payload +=
        "\"trackerMode\":\"";


    payload +=
        trackerMode;


    payload +=
        "\",";


    // ==================================================
    // BATTERY
    // ==================================================

    payload +=
        "\"battery\":";


    payload +=
        String(
            battery
        );


    payload +=
        ",";


    // ==================================================
    // ACCELEROMETER
    // ==================================================

    payload +=
        "\"acceleration\":{";


    payload +=
        "\"x\":";


    payload +=
        String(
            accelX,
            3
        );


    payload +=
        ",";


    payload +=
        "\"y\":";


    payload +=
        String(
            accelY,
            3
        );


    payload +=
        ",";


    payload +=
        "\"z\":";


    payload +=
        String(
            accelZ,
            3
        );


    payload +=
        "},";


    // ==================================================
    // GYROSCOPE
    // ==================================================

    payload +=
        "\"gyroscope\":{";


    payload +=
        "\"x\":";


    payload +=
        String(
            gyroX,
            3
        );


    payload +=
        ",";


    payload +=
        "\"y\":";


    payload +=
        String(
            gyroY,
            3
        );


    payload +=
        ",";


    payload +=
        "\"z\":";


    payload +=
        String(
            gyroZ,
            3
        );


    payload +=
        "}";


    // ==================================================
    // CLOSE JSON
    // ==================================================

    payload +=
        "}";


    // ==================================================
    // DISPLAY PAYLOAD
    // ==================================================

    Serial.println();

    Serial.println(
        "GPS NO-FIX PAYLOAD:"
    );


    Serial.println(
        payload
    );


    // ==================================================
    // SAVE TO HISTORY
    // ==================================================

    Serial.println();

    Serial.println(
        "Saving GPS NO_FIX state to HISTORY..."
    );


    bool success =
        sendFirebaseRequest(
            historyURL,
            payload,
            1
        );


    // ==================================================
    // RESULT
    // ==================================================

    if (
        success
    )
    {
        Serial.println();

        Serial.println(
            "GPS NO_FIX STATE SAVED SUCCESSFULLY."
        );
    }
    else
    {
        Serial.println();

        Serial.println(
            "FAILED TO SAVE GPS NO_FIX STATE."
        );
    }


    return success;
}