// ======================================================
// BLE
// ======================================================

import {
    connect,
    autoConnect,
    toggleLED,
    writeLastSeen,
    readLastSeen
} from "./ble.js";


// ======================================================
// DOM
// ======================================================

import {
    connectBtn,
    ledBtn,
    writeLastSeenBtn,
    readLastSeenBtn,

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

writeLastSeenBtn.addEventListener(
    "click",
    writeLastSeen
);

readLastSeenBtn.addEventListener(
    "click",
    readLastSeen
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
            Number(current.latitude);

        latitude.textContent =
            lat.toFixed(6);
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
            Number(current.longitude);

        longitude.textContent =
            lng.toFixed(6);
    }


    // ==================================================
    // ALTITUDE
    // ==================================================

    if (
        current.altitude !== undefined &&
        current.altitude !== null
    )
    {
        altitude.textContent =
            Number(
                current.altitude
            ).toFixed(1);
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
        current.longitude !== undefined &&
        current.latitude !== null &&
        current.longitude !== null
    )
    {
        updateTrackerMap(
            Number(current.latitude),
            Number(current.longitude)
        );
    }


    // ==================================================
    // CURRENT LOCATION IS ALSO LAST KNOWN LOCATION
    // ==================================================

    if (
        current.latitude !== undefined &&
        current.longitude !== undefined
    )
    {
        lastLatitude.textContent =
            Number(
                current.latitude
            ).toFixed(6);

        lastLongitude.textContent =
            Number(
                current.longitude
            ).toFixed(6);
    }


    if (
        current.altitude !== undefined &&
        current.altitude !== null
    )
    {
        lastAltitude.textContent =
            Number(
                current.altitude
            ).toFixed(1) + " m";
    }


    if (current.timestamp)
    {
        lastLocationTime.textContent =
            current.timestamp;
    }
}


// ======================================================
// UPDATE FIREBASE STATUS
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
    // STATUS
    // ==================================================

    const gpsState =
        status.status || "UNKNOWN";

    firebaseGpsStatus.textContent =
        gpsState;


    // ==================================================
    // SOURCE
    // ==================================================

    locationSource.textContent =
        status.source || "-";


    // ==================================================
    // TIMESTAMP
    // ==================================================

    statusTimestamp.textContent =
        status.timestamp || "-";


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
// UPDATE HISTORY
// ======================================================
//
// Gets the most recent valid GPS location.
//
// This is useful because /current always contains
// the last valid GPS location.
//

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
                entry.longitude !== undefined
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
                    a.timestamp || 0
                ).getTime();

            const timeB =
                new Date(
                    b.timestamp || 0
                ).getTime();

            return timeB - timeA;
        }
    );


    const latest =
        validLocations[0];


    // ==================================================
    // LAST KNOWN LATITUDE
    // ==================================================

    lastLatitude.textContent =
        Number(
            latest.latitude
        ).toFixed(6);


    // ==================================================
    // LAST KNOWN LONGITUDE
    // ==================================================

    lastLongitude.textContent =
        Number(
            latest.longitude
        ).toFixed(6);


    // ==================================================
    // LAST KNOWN ALTITUDE
    // ==================================================

    if (
        latest.altitude !== undefined &&
        latest.altitude !== null
    )
    {
        lastAltitude.textContent =
            Number(
                latest.altitude
            ).toFixed(1) + " m";
    }


    // ==================================================
    // LAST LOCATION TIME
    // ==================================================

    lastLocationTime.textContent =
        latest.timestamp || "-";
}


// ======================================================
// UPDATE DASHBOARD
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

function updateBLEStatus()
{
    const connected =
        document
            .getElementById("status")
            ?.textContent ===
        "Connected";


    if (connected)
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
// REFRESH FIREBASE EVERY 3 SECONDS
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
// REFRESH BLE STATUS
// ======================================================

setInterval(
    updateBLEStatus,
    1000
);


// ======================================================
// AUTOMATIC BLE CONNECTION
// ======================================================

autoConnect();