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

    latitude,
    longitude,
    altitude,
    gpsTime,

    lastLatitude,
    lastLongitude,
    lastAltitude,
    lastLocationTime

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
// UPDATE CURRENT GPS
// ======================================================
//
// Firebase /current is the authoritative source
// for the latest valid GPS location.
//
// This works even when BLE is disconnected.
// ======================================================

async function updateCurrentGPS()
{
    const current =
        await getFirebaseData("current");


    if (!current)
    {
        return;
    }


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


        if (Number.isFinite(lat))
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


        if (Number.isFinite(lng))
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


        if (Number.isFinite(alt))
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
    // UPDATE MAP
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
// Firebase /status is independent of BLE.
//
// LOCATION_AVAILABLE:
//     GPS has a valid fix.
//
// NO_FIX:
//     Device currently has no GPS fix.
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


        // ----------------------------------------------
        // DEVICE GPS STATUS
        // ----------------------------------------------

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


        // ----------------------------------------------
        // DEVICE GPS STATUS
        // ----------------------------------------------

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
//
// /history contains both:
//
// LOCATION_AVAILABLE
// NO_FIX
//
// We only use LOCATION_AVAILABLE entries when
// determining the last known valid position.
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


    if (entries.length === 0)
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


    if (validLocations.length === 0)
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


    if (Number.isFinite(lat))
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


    if (Number.isFinite(lng))
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


        if (Number.isFinite(alt))
        {
            lastAltitude.textContent =
                alt.toFixed(1) + " m";
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

async function updateFirebaseDashboard()
{
    await updateFirebaseStatus();

    await updateCurrentGPS();

    await updateHistory();
}


// ======================================================
// BLE STATUS
// ======================================================
//
// BLE status is completely separate from GPS status.
//
// BLE disconnected does NOT mean GPS disconnected.
//
// Firebase GPS can continue working while BLE is OFF.
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
// Refresh Firebase every 3 seconds.
//
// This means:
//
// BLE ON  -> Firebase GPS works
// BLE OFF -> Firebase GPS still works
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