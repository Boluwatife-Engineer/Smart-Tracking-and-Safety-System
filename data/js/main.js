
// ======================================================
// BLE
// ======================================================

import {
    connect,
    autoConnect,
    toggleLED
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
// GET FIREBASE NODE
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
// UPDATE SOS STATUS
// ======================================================
//
// SOS DOES NOT DEPEND ON BLE.
//
// Firebase:
// /Trackers/tracker_001/status/trackerMode
//
// Examples:
//
// trackerMode = "SOS"
//     → ACTIVE
//
// trackerMode = "MOVING"
//     → INACTIVE
//
// trackerMode = "STATIONARY"
//     → INACTIVE
//
// trackerMode = "BLE_CONNECTED"
//     → INACTIVE
//
// ======================================================

function updateSOSStatus(value)
{
    const status =
        String(value || "")
            .trim()
            .toUpperCase();


    // ==================================================
    // SOS ACTIVE
    // ==================================================

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


    // ==================================================
    // SOS INACTIVE
    // ==================================================

    sosStatus.textContent =
        "INACTIVE";

    sosStatus.className =
        "sos-inactive";
}


// ======================================================
// UPDATE SOS FROM FIREBASE STATUS
// ======================================================
//
// IMPORTANT:
//
// We read trackerMode from /status.
//
// BLE can be connected or disconnected.
// It has NO effect on this value.
//
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
// UPDATE CURRENT SENSOR DATA
// ======================================================

async function updateCurrentSensors()
{
    const current =
        await getFirebaseData("current");


    if (!current)
    {
        return;
    }


    updateCurrentSensorsFromData(
        current
    );
}


// ======================================================
// CURRENT SENSOR DATA FROM FIREBASE
// ======================================================

function updateCurrentSensorsFromData(
    current
)
{
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
// UPDATE CURRENT GPS
// ======================================================

async function updateCurrentGPS()
{
    const current =
        await getFirebaseData("current");


    if (!current)
    {
        return;
    }


    updateCurrentGPSFromData(
        current
    );
}


// ======================================================
// CURRENT GPS FROM FIREBASE
// ======================================================

function updateCurrentGPSFromData(
    current
)
{
    // ==================================================
    // LATITUDE
    // ==================================================

    if (
        current.latitude !== undefined &&
        current.latitude !== null
    )
    {
        const lat =
            Number(
                current.latitude
            );


        if (
            Number.isFinite(lat)
        )
        {
            latitude.textContent =
                lat.toFixed(6);
        }
    }


    // ==================================================
    // LONGITUDE
    // ==================================================

    if (
        current.longitude !== undefined &&
        current.longitude !== null
    )
    {
        const lng =
            Number(
                current.longitude
            );


        if (
            Number.isFinite(lng)
        )
        {
            longitude.textContent =
                lng.toFixed(6);
        }
    }


    // ==================================================
    // ALTITUDE
    // ==================================================

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
                alt.toFixed(1);
        }
    }


    // ==================================================
    // GPS TIME
    // ==================================================

    if (
        current.gpsTime !== undefined &&
        current.gpsTime !== null
    )
    {
        gpsTime.textContent =
            current.gpsTime;
    }


    // ==================================================
    // MAP
    // ==================================================

    if (
        current.latitude !== undefined &&
        current.longitude !== undefined
    )
    {
        const lat =
            Number(
                current.latitude
            );


        const lng =
            Number(
                current.longitude
            );


        if (
            Number.isFinite(lat) &&
            Number.isFinite(lng)
        )
        {
            updateTrackerMap(
                lat,
                lng
            );
        }
    }
}


// ======================================================
// UPDATE FIREBASE STATUS
// ======================================================
//
// Firebase /status contains:
//
// status
// source
// timestamp
// trackerMode
//
// trackerMode is ALSO used for SOS.
//
// ======================================================

async function updateFirebaseStatus()
{
    const status =
        await getFirebaseData("status");


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
    //
    // IMPORTANT:
    //
    // Do this BEFORE any BLE logic.
    //
    // Firebase remains the source of truth.
    //
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


        gpsStatus.textContent =
            "AVAILABLE";
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


        gpsStatus.textContent =
            "NO FIX";
    }


    else
    {
        firebaseGpsStatus.className =
            "gps-searching";


        locationState.textContent =
            gpsState;


        locationState.className =
            "gps-searching";


        gpsStatus.textContent =
            gpsState;
    }
}


// ======================================================
// UPDATE HISTORY
// ======================================================

async function updateHistory()
{
    const history =
        await getFirebaseData("history");


    if (!history)
    {
        return;
    }


    const entries =
        Object.values(history);


    if (
        entries.length === 0
    )
    {
        return;
    }


    // ==================================================
    // ONLY VALID GPS LOCATIONS
    // ==================================================

    const validLocations =
        entries.filter(
            entry =>
                entry &&
                entry.status ===
                    "LOCATION_AVAILABLE" &&
                entry.latitude !== undefined &&
                entry.longitude !== undefined &&
                entry.timestamp
        );


    if (
        validLocations.length === 0
    )
    {
        return;
    }


    // ==================================================
    // SORT NEWEST FIRST
    // ==================================================

    validLocations.sort(
        (a, b) =>
        {
            const timeA =
                new Date(
                    a.timestamp
                ).getTime();


            const timeB =
                new Date(
                    b.timestamp
                ).getTime();


            return timeB - timeA;
        }
    );


    // ==================================================
    // NEWEST VALID LOCATION
    // ==================================================

    const latest =
        validLocations[0];


    // ==================================================
    // LAST KNOWN LATITUDE
    // ==================================================

    const lat =
        Number(
            latest.latitude
        );


    if (
        Number.isFinite(lat)
    )
    {
        lastLatitude.textContent =
            lat.toFixed(6);
    }


    // ==================================================
    // LAST KNOWN LONGITUDE
    // ==================================================

    const lng =
        Number(
            latest.longitude
        );


    if (
        Number.isFinite(lng)
    )
    {
        lastLongitude.textContent =
            lng.toFixed(6);
    }


    // ==================================================
    // LAST KNOWN ALTITUDE
    // ==================================================

    if (
        latest.altitude !== undefined &&
        latest.altitude !== null
    )
    {
        const alt =
            Number(
                latest.altitude
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
    // LAST LOCATION TIME
    // ==================================================

    lastLocationTime.textContent =
        latest.timestamp;
}


// ======================================================
// UPDATE FIREBASE DASHBOARD
// ======================================================
//
// Firebase provides:
//
// current
//     → sensors + GPS
//
// status
//     → GPS status + SOS trackerMode
//
// history
//     → last known location
//
// BLE status is completely separate.
//
// ======================================================

async function updateFirebaseDashboard()
{
    // ==================================================
    // CURRENT
    // ==================================================

    const current =
        await getFirebaseData("current");


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
    // HISTORY
    // ==================================================

    await updateHistory();
}


// ======================================================
// BLE STATUS
// ======================================================
//
// BLE status ONLY controls:
//
// - BLE connection indicator
// - Connect button
//
// It MUST NOT modify SOS.
//
// ======================================================

function updateBLEStatus()
{
    if (
        statusText &&
        statusText.textContent ===
            "Connected"
    )
    {
        bleStatus.textContent =
            "Connected";


        bleStatus.className =
            "gps-fixed";
    }

    else
    {
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
//
// Refresh every 3 seconds.
//
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

