// ======================================================
// DOM ELEMENTS
// ======================================================


// ======================================================
// CONNECT
// ======================================================

const connectBtn =
    document.getElementById("connectBtn");


// ======================================================
// BUZZER
// ======================================================

const ledBtn =
    document.getElementById("ledBtn");


// ======================================================
// LAST SEEN
// ======================================================

const writeLastSeenBtn =
    document.getElementById(
        "writeLastSeenBtn"
    );

const readLastSeenBtn =
    document.getElementById(
        "readLastSeenBtn"
    );


// ======================================================
// STATUS
// ======================================================

const statusText =
    document.getElementById("status");


// ======================================================
// MOTION
// ======================================================

const motionText =
    document.getElementById("motion");


// ======================================================
// BATTERY
// ======================================================

const batteryText =
    document.getElementById("battery");


// ======================================================
// ACCELEROMETER
// ======================================================

const ax =
    document.getElementById("ax");

const ay =
    document.getElementById("ay");

const az =
    document.getElementById("az");


// ======================================================
// GYROSCOPE
// ======================================================

const gx =
    document.getElementById("gx");

const gy =
    document.getElementById("gy");

const gz =
    document.getElementById("gz");


// ======================================================
// GPS
// ======================================================

const gpsStatus =
    document.getElementById("gpsStatus");

const latitude =
    document.getElementById("latitude");

const longitude =
    document.getElementById("longitude");

const altitude =
    document.getElementById("altitude");

const gpsTime =
    document.getElementById("gpsTime");


// ======================================================
// MAP COORDINATES
// ======================================================

const mapLatitude =
    document.getElementById("mapLatitude");

const mapLongitude =
    document.getElementById("mapLongitude");


// ======================================================
// EXPORT
// ======================================================

export {
    connectBtn,

    ledBtn,

    writeLastSeenBtn,
    readLastSeenBtn,

    statusText,

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

    mapLatitude,
    mapLongitude
};