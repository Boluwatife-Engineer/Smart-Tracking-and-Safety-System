// ======================================================
// BLE
// ======================================================

import {
    connect,
    autoConnect,
    toggleLED,
    isBLEConnected
} from "./ble.js";


// ======================================================
// DOM
// ======================================================

import {

    connectBtn,
    ledBtn,

    statusText,
    bleStatus,

    firebaseGpsStatus,
    locationSource,
    locationState,
    statusTimestamp,

    motionText,
    batteryText,

    ax,
    ay,
    az,

    gx,
    gy,
    gz,

    gpsStatus,
    latitude,
    longitude,
    altitude,
    gpsTime,

    lastLatitude,
    lastLongitude,
    lastAltitude,
    lastLocationTime,

    sosStatus

} from "./dom.js";


// ======================================================
// MAP
// ======================================================

import {
    updateTrackerMap
} from "./map.js";


// ======================================================
// FIREBASE
// ======================================================

const FIREBASE_BASE_URL =
    "https://smart-trackerr-default-rtdb.firebaseio.com";

const TRACKER_PATH =
    "/Trackers/tracker_001";


// ======================================================
// BUTTON EVENTS
// ======================================================

connectBtn.addEventListener(
    "click",
    connect
);

ledBtn.addEventListener(
    "click",
    toggleLED
);


// ======================================================
// FIREBASE GET
// ======================================================

async function getFirebaseData(path)
{
    try
    {
        const response =
            await fetch(
                `${FIREBASE_BASE_URL}${TRACKER_PATH}/${path}.json`
            );


        if (!response.ok)
        {
            throw new Error(
                `Firebase HTTP ${response.status}`
            );
        }


        return await response.json();
    }

    catch(error)
    {
        console.error(
            `Firebase ${path} error:`,
            error
        );

        return null;
    }
}


// ======================================================
// SOS
// ======================================================

function updateSOSStatus(value)
{
    const status =
        String(value || "")
            .trim()
            .toUpperCase();


    if (
        status === "SOS"
    )
    {
        sosStatus.textContent =
            "ACTIVE";

        sosStatus.className =
            "sos-active";

        return;
    }


    sosStatus.textContent =
        "INACTIVE";

    sosStatus.className =
        "sos-inactive";
}


// ======================================================
// UPDATE SOS FROM FIREBASE
// ======================================================

function updateFirebaseSOSFromStatus(
    status
)
{
    if (!status)
    {
        return;
    }


    const trackerMode =
        String(
            status.trackerMode || ""
        )
            .trim()
            .toUpperCase();


    updateSOSStatus(
        trackerMode
    );
}


// ======================================================
// CURRENT SENSOR DATA
// ======================================================

function updateCurrentSensorsFromData(
    current
)
{
    if (!current)
    {
        return;
    }


    // ==================================================
    // MOTION
    // ==================================================

    if (
        current.motion !== undefined &&
        current.motion !== null
    )
    {
        const motion =
            String(
                current.motion
            );


        motionText.textContent =
            motion;


        if (
            motion === "MOVING"
        )
        {
            motionText.className =
                "motion moving";
        }

        else
        {
            motionText.className =
                "motion stationary";
        }
    }


    // ==================================================
    // BATTERY
    // ==================================================

    if (
        current.battery !== undefined &&
        current.battery !== null
    )
    {
        const battery =
            Number(
                current.battery
            );


        if (
            Number.isFinite(
                battery
            )
        )
        {
            batteryText.textContent =
                battery;
        }
    }


    // ==================================================
    // ACCELERATION
    // ==================================================

    if (
        current.acceleration
    )
    {
        const a =
            current.acceleration;


        if (
            a.x !== undefined
        )
        {
            const value =
                Number(a.x);


            if (
                Number.isFinite(value)
            )
            {
                ax.textContent =
                    value.toFixed(2);
            }
        }


        if (
            a.y !== undefined
        )
        {
            const value =
                Number(a.y);


            if (
                Number.isFinite(value)
            )
            {
                ay.textContent =
                    value.toFixed(2);
            }
        }


        if (
            a.z !== undefined
        )
        {
            const value =
                Number(a.z);


            if (
                Number.isFinite(value)
            )
            {
                az.textContent =
                    value.toFixed(2);
            }
        }
    }


    // ==================================================
    // GYROSCOPE
    // ==================================================

    if (
        current.gyroscope
    )
    {
        const g =
            current.gyroscope;


        if (
            g.x !== undefined
        )
        {
            const value =
                Number(g.x);


            if (
                Number.isFinite(value)
            )
            {
                gx.textContent =
                    value.toFixed(2);
            }
        }


        if (
            g.y !== undefined
        )
        {
            const value =
                Number(g.y);


            if (
                Number.isFinite(value)
            )
            {
                gy.textContent =
                    value.toFixed(2);
            }
        }


        if (
            g.z !== undefined
        )
        {
            const value =
                Number(g.z);


            if (
                Number.isFinite(value)
            )
            {
                gz.textContent =
                    value.toFixed(2);
            }
        }
    }
}


// ======================================================
// CURRENT GPS
// ======================================================
//
// Firebase /current is ALWAYS the current tracker
// location.
//
// Map:
//     ALWAYS uses /current.
//
// Device GPS:
//     BLE connected    -> BLE GPS notifications
//     BLE disconnected -> Firebase /current
//
// Last Known Location:
//     NEVER comes from here.
//     It comes only from /lastSeen.
//
// ======================================================

function updateCurrentGPSFromData(
    current
)
{
    if (!current)
    {
        return;
    }


    if (
        current.latitude === undefined ||
        current.longitude === undefined
    )
    {
        return;
    }


    const lat =
        Number(
            current.latitude
        );


    const lng =
        Number(
            current.longitude
        );


    if (
        !Number.isFinite(lat) ||
        !Number.isFinite(lng)
    )
    {
        return;
    }


    // ==================================================
    // DEVICE GPS WHEN BLE IS DISCONNECTED
    // ==================================================
    //
    // BLE owns these fields only while BLE is connected.
    //
    // Once BLE disconnects, Firebase /current becomes
    // the source for Device GPS.
    //
    // ==================================================

    if (
        !isBLEConnected()
    )
    {
        latitude.textContent =
            lat.toFixed(6);


        longitude.textContent =
            lng.toFixed(6);


        // ==============================================
        // ALTITUDE
        // ==============================================

        if (
            current.altitude !== undefined &&
            current.altitude !== null
        )
        {
            const alt =
                Number(
                    current.altitude
                );


            if (
                Number.isFinite(alt)
            )
            {
                altitude.textContent =
                    alt.toFixed(1) +
                    " m";
            }
        }


        // ==============================================
        // GPS TIME
        // ==============================================

        if (
            current.gpsTime !== undefined &&
            current.gpsTime !== null
        )
        {
            gpsTime.textContent =
                current.gpsTime;
        }


        // ==============================================
        // DEVICE GPS STATUS
        // ==============================================
        //
        // The detailed Firebase GPS status is handled
        // separately by /status.
        //
        // This only prevents the BLE disconnect from
        // leaving the Device GPS showing "DISCONNECTED".
        //
        // ==============================================

        gpsStatus.textContent =
            "AVAILABLE";

        gpsStatus.className =
            "gps-fixed";
    }


    // ==================================================
    // LIVE LOCATION MAP
    // ==================================================
    //
    // The map ALWAYS follows /current.
    //
    // BLE connection has no effect on the map.
    //
    // ==================================================

    updateTrackerMap(
        lat,
        lng
    );
}


// ======================================================
// FIREBASE STATUS
// ======================================================
//
// /status is the authoritative tracker GPS/system status.
//
// It controls:
//
// - GPS status
// - Location Source
// - Location State
// - Last Status Update
// - SOS mode
//
// It does NOT control BLE connection status.
//
// ======================================================

async function updateFirebaseStatus()
{
    const status =
        await getFirebaseData(
            "status"
        );


    if (!status)
    {
        firebaseGpsStatus.textContent =
            "UNKNOWN";


        firebaseGpsStatus.className =
            "gps-searching";


        locationSource.textContent =
            "-";


        locationState.textContent =
            "UNKNOWN";


        locationState.className =
            "gps-searching";


        statusTimestamp.textContent =
            "-";


        return;
    }


    // ==================================================
    // SOS
    // ==================================================

    updateFirebaseSOSFromStatus(
        status
    );


    // ==================================================
    // GPS STATUS
    // ==================================================

    const gpsState =
        status.status ||
        "UNKNOWN";


    firebaseGpsStatus.textContent =
        gpsState;


    // ==================================================
    // LOCATION SOURCE
    // ==================================================

    locationSource.textContent =
        status.source ||
        "-";


    // ==================================================
    // STATUS TIMESTAMP
    // ==================================================

    statusTimestamp.textContent =
        status.timestamp ||
        "-";


    // ==================================================
    // LOCATION STATE
    // ==================================================

    if (
        gpsState ===
        "LOCATION_AVAILABLE"
    )
    {
        firebaseGpsStatus.className =
            "gps-fixed";


        locationState.textContent =
            "AVAILABLE";


        locationState.className =
            "gps-fixed";
    }

    else if (
        gpsState ===
        "NO_FIX"
    )
    {
        firebaseGpsStatus.className =
            "gps-searching";


        locationState.textContent =
            "GPS NO FIX";


        locationState.className =
            "gps-searching";
    }

    else
    {
        firebaseGpsStatus.className =
            "gps-searching";


        locationState.textContent =
            gpsState;


        locationState.className =
            "gps-searching";
    }
}


// ======================================================
// LAST KNOWN LOCATION
// ======================================================
//
// ONLY /lastSeen.
//
// Never /current.
// Never /history.
//
// This value remains frozen until the ESP32 writes a new
// /lastSeen record.
//
// ======================================================

async function updateLastKnownLocation()
{
    const lastSeen =
        await getFirebaseData(
            "lastSeen"
        );


    if (!lastSeen)
    {
        return;
    }


    // ==================================================
    // LATITUDE
    // ==================================================

    if (
        lastSeen.latitude !== undefined &&
        lastSeen.latitude !== null
    )
    {
        const lat =
            Number(
                lastSeen.latitude
            );


        if (
            Number.isFinite(lat)
        )
        {
            lastLatitude.textContent =
                lat.toFixed(6);
        }
    }


    // ==================================================
    // LONGITUDE
    // ==================================================

    if (
        lastSeen.longitude !== undefined &&
        lastSeen.longitude !== null
    )
    {
        const lng =
            Number(
                lastSeen.longitude
            );


        if (
            Number.isFinite(lng)
        )
        {
            lastLongitude.textContent =
                lng.toFixed(6);
        }
    }


    // ==================================================
    // ALTITUDE
    // ==================================================

    if (
        lastSeen.altitude !== undefined &&
        lastSeen.altitude !== null
    )
    {
        const alt =
            Number(
                lastSeen.altitude
            );


        if (
            Number.isFinite(alt)
        )
        {
            lastAltitude.textContent =
                alt.toFixed(1) +
                " m";
        }
    }


    // ==================================================
    // TIME
    // ==================================================

    if (
        lastSeen.timestamp
    )
    {
        lastLocationTime.textContent =
            lastSeen.timestamp;
    }
}


// ======================================================
// FIREBASE DASHBOARD UPDATE
// ======================================================

async function updateFirebaseDashboard()
{
    // ==================================================
    // CURRENT
    // ==================================================

    const current =
        await getFirebaseData(
            "current"
        );


    if (current)
    {
        updateCurrentSensorsFromData(
            current
        );


        updateCurrentGPSFromData(
            current
        );
    }


    // ==================================================
    // STATUS
    // ==================================================

    await updateFirebaseStatus();


    // ==================================================
    // LAST SEEN
    // ==================================================

    await updateLastKnownLocation();
}


// ======================================================
// BLE STATUS UI
// ======================================================
//
// This controls ONLY the BLE connection indicators.
//
// It does NOT control:
//
// - Device GPS
// - Firebase GPS status
// - SOS
// - Last Known Location
// - Map
//
// ======================================================

function updateBLEStatus()
{
    if (
        isBLEConnected()
    )
    {
        statusText.textContent =
            "Connected";


        statusText.className =
            "status connected";


        bleStatus.textContent =
            "Connected";


        bleStatus.className =
            "gps-fixed";
    }

    else
    {
        statusText.textContent =
            "Disconnected";


        statusText.className =
            "status";


        bleStatus.textContent =
            "Disconnected";


        bleStatus.className =
            "gps-searching";
    }
}


// ======================================================
// INITIAL FIREBASE LOAD
// ======================================================

updateFirebaseDashboard();


// ======================================================
// FIREBASE REFRESH
// ======================================================

setInterval(
    updateFirebaseDashboard,
    3000
);


// ======================================================
// INITIAL BLE STATUS
// ======================================================

updateBLEStatus();


// ======================================================
// BLE STATUS REFRESH
// ======================================================

setInterval(
    updateBLEStatus,
    500
);


// ======================================================
// AUTOMATIC BLE CONNECTION
// ======================================================

autoConnect();