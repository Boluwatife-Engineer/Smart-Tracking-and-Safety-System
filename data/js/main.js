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
        await getFirebaseData(
            "current"
        );


    // ==================================================
    // IF CURRENT NODE EXISTS
    // ==================================================

    if (current)
    {
        if (
            current.latitude !== undefined &&
            current.latitude !== null
        )
        {
            latitude.textContent =
                Number(
                    current.latitude
                ).toFixed(6);
        }


        if (
            current.longitude !== undefined &&
            current.longitude !== null
        )
        {
            longitude.textContent =
                Number(
                    current.longitude
                ).toFixed(6);
        }


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


        if (
            current.gpsTime !== undefined &&
            current.gpsTime !== null
        )
        {
            gpsTime.textContent =
                current.gpsTime;
        }


        if (
            current.latitude !== undefined &&
            current.longitude !== undefined
        )
        {
            updateTrackerMap(
                Number(current.latitude),
                Number(current.longitude)
            );
        }
    }
}


// ======================================================
// UPDATE FIREBASE STATUS
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

        locationSource.textContent =
            "-";

        locationState.textContent =
            "-";

        statusTimestamp.textContent =
            "-";

        return;
    }


    // ==================================================
    // STATUS
    // ==================================================

    firebaseGpsStatus.textContent =
        status.status || "UNKNOWN";


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
        status.status ===
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
        status.status ===
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
            status.status ||
            "UNKNOWN";
    }
}


// ======================================================
// UPDATE LAST SEEN
// ======================================================

async function updateLastSeen()
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
        lastSeen.latitude !== undefined
    )
    {
        lastLatitude.textContent =
            Number(
                lastSeen.latitude
            ).toFixed(6);
    }


    // ==================================================
    // LONGITUDE
    // ==================================================

    if (
        lastSeen.longitude !== undefined
    )
    {
        lastLongitude.textContent =
            Number(
                lastSeen.longitude
            ).toFixed(6);
    }


    // ==================================================
    // ALTITUDE
    // ==================================================

    if (
        lastSeen.altitude !== undefined
    )
    {
        lastAltitude.textContent =
            Number(
                lastSeen.altitude
            ).toFixed(1) + " m";
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
// FALLBACK: GET LAST VALID GPS FROM HISTORY
// ======================================================
//
// This is only used until the ESP32 starts creating
// /current and /lastSeen.
//

async function updateHistoryFallback()
{
    const history =
        await getFirebaseData(
            "history"
        );


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
    // SORT BY TIMESTAMP
    // ==================================================

    entries.sort(
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

            return timeA - timeB;
        }
    );


    // ==================================================
    // FIND MOST RECENT VALID GPS
    // ==================================================

    const validLocations =
        entries.filter(
            entry =>
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


    const latestValid =
        validLocations[
            validLocations.length - 1
        ];


    // ==================================================
    // LAST KNOWN LOCATION
    // ==================================================

    lastLatitude.textContent =
        Number(
            latestValid.latitude
        ).toFixed(6);


    lastLongitude.textContent =
        Number(
            latestValid.longitude
        ).toFixed(6);


    if (
        latestValid.altitude !== undefined
    )
    {
        lastAltitude.textContent =
            Number(
                latestValid.altitude
            ).toFixed(1) + " m";
    }


    lastLocationTime.textContent =
        latestValid.timestamp ||
        "-";
}


// ======================================================
// UPDATE DASHBOARD
// ======================================================

async function updateFirebaseDashboard()
{
    await updateFirebaseStatus();

    await updateCurrentGPS();

    await updateLastSeen();

    await updateHistoryFallback();
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
// START FIREBASE
// ======================================================

updateFirebaseDashboard();


setInterval(
    updateFirebaseDashboard,
    3000
);


// ======================================================
// BLE STATUS
// ======================================================

updateBLEStatus();


setInterval(
    updateBLEStatus,
    1000
);


// ======================================================
// AUTO CONNECT
// ======================================================

autoConnect();