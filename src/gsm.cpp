#include "gsm.h"

#include "sim7600.h"
#include "storage.h"
#include "mpu6050.h"
#include "battery.h"
#include "tracker_state.h"
#include "sos.h"
#include "gps.h"


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

const char *sosURL =
    "https://smart-trackerr-default-rtdb.firebaseio.com/Trackers/tracker_001/sos.json";


// ======================================================
// OFFLINE QUEUE CONTROL
// ======================================================

unsigned long lastOfflineSync = 0;

const unsigned long OFFLINE_SYNC_INTERVAL = 10000;


// ======================================================
// SOS UPLOAD CONTROL
// ======================================================

static bool sosEventPending = false;


// ======================================================
// INITIALIZE GSM
// ======================================================

void initGSM()
{
    Serial.println();
    Serial.println(
        "========== GSM INITIALIZATION =========="
    );


    // ==================================================
    // STORAGE
    // ==================================================

    initStorage();


    // ==================================================
    // SIM
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
    // NETWORK
    // ==================================================

    sendSIM7600Command(
        "AT+CEREG?",
        1000
    );


    // ==================================================
    // ATTACH
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
    Serial.println(
        "========== SSL CONFIGURATION =========="
    );


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
    // READY
    // ==================================================

    gsmReady = true;


    Serial.println();
    Serial.println(
        "GSM initialization complete."
    );


    Serial.print(
        "Offline records waiting: "
    );

    Serial.println(
        getOfflineRecordCount()
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

    unsigned long start =
        millis();


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

            Serial.write(c);


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

    while (
        sim7600.available()
    )
    {
        sim7600.read();
    }


    // ==================================================
    // CEREG
    // ==================================================

    sim7600.println(
        "AT+CEREG?"
    );


    String cereg = "";

    unsigned long start =
        millis();


    while (
        millis() - start < 2000
    )
    {
        while (
            sim7600.available()
        )
        {
            char c =
                (char)sim7600.read();

            cereg += c;
        }


        if (
            cereg.indexOf("OK") != -1
        )
        {
            break;
        }
    }


    Serial.print(
        "CEREG: "
    );

    Serial.println(
        cereg
    );


    bool registered =
        cereg.indexOf("+CEREG: 0,1") != -1 ||
        cereg.indexOf("+CEREG: 0,5") != -1 ||
        cereg.indexOf("+CEREG: 1,1") != -1 ||
        cereg.indexOf("+CEREG: 1,5") != -1 ||
        cereg.indexOf("+CEREG: 2,1") != -1 ||
        cereg.indexOf("+CEREG: 2,5") != -1;


    if (!registered)
    {
        Serial.println(
            "Network NOT registered."
        );

        return false;
    }


    // ==================================================
    // CGATT
    // ==================================================

    while (
        sim7600.available()
    )
    {
        sim7600.read();
    }


    sim7600.println(
        "AT+CGATT?"
    );


    String cgatt = "";

    start =
        millis();


    while (
        millis() - start < 2000
    )
    {
        while (
            sim7600.available()
        )
        {
            char c =
                (char)sim7600.read();

            cgatt += c;
        }


        if (
            cgatt.indexOf("OK") != -1
        )
        {
            break;
        }
    }


    Serial.print(
        "CGATT: "
    );

    Serial.println(
        cgatt
    );


    if (
        cgatt.indexOf("+CGATT: 1") == -1
    )
    {
        Serial.println(
            "Packet domain NOT attached."
        );

        return false;
    }


    // ==================================================
    // CGACT
    // ==================================================

    while (
        sim7600.available()
    )
    {
        sim7600.read();
    }


    sim7600.println(
        "AT+CGACT?"
    );


    String cgact = "";

    start =
        millis();


    while (
        millis() - start < 2000
    )
    {
        while (
            sim7600.available()
        )
        {
            char c =
                (char)sim7600.read();

            cgact += c;
        }


        if (
            cgact.indexOf("OK") != -1
        )
        {
            break;
        }
    }


    Serial.print(
        "CGACT: "
    );

    Serial.println(
        cgact
    );


    if (
        cgact.indexOf("+CGACT: 1,1") == -1
    )
    {
        Serial.println(
            "PDP context is NOT active."
        );

        return false;
    }


    Serial.println(
        "NETWORK AVAILABLE."
    );


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
        Serial.println(
            "GSM is not ready."
        );

        return false;
    }


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
    // CLEAR UART
    // ==================================================

    while (
        sim7600.available()
    )
    {
        sim7600.read();
    }


    // ==================================================
    // TERMINATE PREVIOUS SESSION
    // ==================================================

    sim7600.println(
        "AT+HTTPTERM"
    );

    delay(300);


    while (
        sim7600.available()
    )
    {
        Serial.write(
            sim7600.read()
        );
    }


    // ==================================================
    // INIT HTTP
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


    urlCommand += url;

    urlCommand += "\"";


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

    if (method == 2)
    {
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
                "ERROR: PUT override failed."
            );

            sim7600.println(
                "AT+HTTPTERM"
            );

            return false;
        }
    }


    // ==================================================
    // HTTP DATA
    // ==================================================

    String httpDataCommand =
        "AT+HTTPDATA=";


    httpDataCommand +=
        payload.length();


    httpDataCommand +=
        ",15000";


    sim7600.println(
        httpDataCommand
    );


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
    // SEND PAYLOAD
    // ==================================================

    sim7600.print(
        payload
    );


    if (
        !waitForResponse(
            "OK",
            15000
        )
    )
    {
        Serial.println(
            "ERROR: Payload rejected."
        );

        sim7600.println(
            "AT+HTTPTERM"
        );

        return false;
    }


    // ==================================================
    // HTTP ACTION
    // ==================================================

    sim7600.println(
        "AT+HTTPACTION=1"
    );


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

            Serial.write(c);
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


    Serial.println();

    Serial.println(
        "HTTP ACTION RESPONSE:"
    );

    Serial.println(
        response
    );


    // ==================================================
    // PARSE HTTP STATUS
    // ==================================================

    int actionIndex =
        response.indexOf(
            "+HTTPACTION:"
        );


    if (actionIndex == -1)
    {
        Serial.println(
            "ERROR: No HTTPACTION response."
        );

        sim7600.println(
            "AT+HTTPTERM"
        );

        delay(300);


        while (
            sim7600.available()
        )
        {
            Serial.write(
                sim7600.read()
            );
        }


        return false;
    }


    String action =
        response.substring(
            actionIndex
        );


    bool success =
        action.indexOf(",200,") != -1 ||
        action.indexOf(",201,") != -1 ||
        action.indexOf(",204,") != -1;


    if (success)
    {
        Serial.println(
            "FIREBASE REQUEST SUCCESS."
        );
    }
    else
    {
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


    delay(300);


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
// SAVE HISTORY OR QUEUE OFFLINE
// ======================================================

bool saveHistoryOrQueue(
    const String &payload
)
{
    if (!hasNetworkConnection())
    {
        Serial.println();
        Serial.println(
            "NO INTERNET / NETWORK."
        );

        Serial.println(
            "Saving history record locally."
        );


        return saveOfflineRecord(
            payload
        );
    }


    bool success =
        sendFirebaseRequest(
            historyURL,
            payload,
            1
        );


    if (success)
    {
        Serial.println(
            "History uploaded successfully."
        );

        return true;
    }


    Serial.println();
    Serial.println(
        "History upload failed."
    );

    Serial.println(
        "Saving record to offline queue."
    );


    return saveOfflineRecord(
        payload
    );
}


// ======================================================
// SOS EVENT
// ======================================================

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


    // ==================================================
    // GPS DATA
    // ==================================================

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


    // ==================================================
    // TIMESTAMP
    // ==================================================

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


    // ==================================================
    // SENSOR DATA
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


    const char *event =
        sosActive
            ? "SOS_ACTIVATED"
            : "SOS_DEACTIVATED";


    const char *sosStatus =
        sosActive
            ? "ACTIVE"
            : "INACTIVE";


    // ==================================================
    // BUILD SOS PAYLOAD
    // ==================================================

    String payload =
        "{";


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


    payload +=
        "},";


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


    payload +=
        "}";


    payload +=
        "}";


    // ==================================================
    // DEBUG
    // ==================================================

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

    Serial.println(
        payload
    );


    // ==================================================
    // SAVE TO HISTORY
    // ==================================================

    bool historySuccess =
        saveHistoryOrQueue(
            payload
        );


    // ==================================================
    // UPDATE /SOS
    // ==================================================

    bool sosSuccess =
        false;


    if (
        hasNetworkConnection()
    )
    {
        sosSuccess =
            sendFirebaseRequest(
                sosURL,
                payload,
                2
            );
    }


    // ==================================================
    // UPDATE CURRENT
    // ==================================================

    bool currentSuccess =
        false;


    if (
        hasNetworkConnection()
    )
    {
        String currentPayload =
            "{";


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


        currentPayload +=
            "}";


        currentSuccess =
            sendFirebaseRequest(
                currentURL,
                currentPayload,
                2
            );
    }


    // ==================================================
    // UPDATE STATUS
    // ==================================================

    bool statusSuccess =
        false;


    if (
        hasNetworkConnection()
    )
    {
        String statusPayload =
            "{";


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


        statusPayload +=
            "}";


        statusSuccess =
            sendFirebaseRequest(
                statusURL,
                statusPayload,
                2
            );
    }


    // ==================================================
    // RESULT
    // ==================================================

    Serial.println();
    Serial.println(
        "SOS FIREBASE RESULTS"
    );


    Serial.print(
        "History: "
    );

    Serial.println(
        historySuccess
            ? "SUCCESS"
            : "QUEUED/FAILED"
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


    // ==================================================
    // EVENT CONSIDERED PROCESSED
    // ==================================================

    return historySuccess;
}


// ======================================================
// PROCESS SOS STATE CHANGE
// ======================================================
//
// IMPORTANT:
// This does NOT depend on BLE.
//
// updateSOS() detects the physical button.
// gsmLoop() detects the SOS state change.
// Therefore the SOS system continues working even
// when BLE is disconnected.
//
// ======================================================

void processSOS()
{
    if (!hasSOSStateChanged())
    {
        return;
    }


    Serial.println();
    Serial.println(
        "SOS STATE CHANGE DETECTED."
    );


    sosEventPending = true;


    // ==================================================
    // TRY TO SEND EVENT
    // ==================================================

    if (
        sendSOSEvent()
    )
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

    Serial.println(
        count
    );


    if (!hasNetworkConnection())
    {
        Serial.println(
            "Network still unavailable."
        );

        return;
    }


    int uploaded = 0;


    while (
        getOfflineRecordCount() > 0
    )
    {
        String payload =
            getOfflineRecord(
                0
            );


        if (
            payload.length() == 0
        )
        {
            Serial.println(
                "ERROR: Could not read offline record."
            );

            break;
        }


        Serial.println();
        Serial.println(
            "Uploading offline record..."
        );


        bool success =
            sendFirebaseRequest(
                historyURL,
                payload,
                1
            );


        if (!success)
        {
            Serial.println(
                "Offline upload failed."
            );

            Serial.println(
                "Stopping sync."
            );

            break;
        }


        if (
            deleteOfflineRecord(0)
        )
        {
            uploaded++;


            Serial.print(
                "Offline record uploaded. Remaining: "
            );

            Serial.println(
                getOfflineRecordCount()
            );
        }
        else
        {
            Serial.println(
                "ERROR: Upload succeeded but deletion failed."
            );

            break;
        }


        delay(500);
    }


    Serial.println();

    Serial.print(
        "Offline records uploaded: "
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


    Serial.println(
        "================================"
    );
}


// ======================================================
// GSM LOOP
// ======================================================

void gsmLoop()
{
    // ==================================================
    // SOS MUST BE CHECKED FIRST
    // ==================================================
    //
    // Do NOT put SOS behind:
    //
    // if (!BLE connected)
    //
    // or any BLE condition.
    //
    // SOS is completely independent of BLE.
    //

    processSOS();


    // ==================================================
    // OFFLINE HISTORY SYNC TIMER
    // ==================================================

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
    // CURRENT PAYLOAD
    // ==================================================

    String currentPayload =
        "{";


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


    currentPayload +=
        "},";


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


    currentPayload +=
        "}"
    ;


    currentPayload +=
        "}";


    // ==================================================
    // STATUS PAYLOAD
    // ==================================================

    String statusPayload =
        "{";


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


    statusPayload +=
        "},";


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


    statusPayload +=
        "}";


    statusPayload +=
        "}";


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

    bool currentSuccess =
        false;

    bool statusSuccess =
        false;


    if (
        hasNetworkConnection()
    )
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
            : "QUEUED"
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


    String payload =
        "{";


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


    payload +=
        "},";


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


    payload +=
        "}";


    payload +=
        "}";


    Serial.println();
    Serial.println(
        "NO_FIX PAYLOAD:"
    );

    Serial.println(
        payload
    );


    bool success =
        saveHistoryOrQueue(
            payload
        );


    if (success)
    {
        Serial.println(
            "GPS NO_FIX HISTORY SAVED/QUEUED."
        );
    }
    else
    {
        Serial.println(
            "FAILED TO SAVE GPS NO_FIX RECORD."
        );
    }


    return success;
}