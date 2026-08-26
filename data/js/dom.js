// ======================================================
// DOM ELEMENTS
// ======================================================


// ======================================================
// CONNECT
// ======================================================

const connectBtn =
    document.getElementById(
        "connectBtn"
    );


// ======================================================
// BUZZER
// ======================================================

const ledBtn =
    document.getElementById(
        "ledBtn"
    );


// ======================================================
// BLE CONNECTION STATUS
// ======================================================

const statusText =
    document.getElementById(
        "status"
    );


const bleStatus =
    document.getElementById(
        "bleStatus"
    );


// ======================================================
// FIREBASE STATUS
// ======================================================

const firebaseGpsStatus =
    document.getElementById(
        "firebaseGpsStatus"
    );


const locationSource =
    document.getElementById(
        "locationSource"
    );


const locationState =
    document.getElementById(
        "locationState"
    );


const statusTimestamp =
    document.getElementById(
        "statusTimestamp"
    );


// ======================================================
// MOTION
// ======================================================

const motionText =
    document.getElementById(
        "motion"
    );


// ======================================================
// BATTERY
// ======================================================

const batteryText =
    document.getElementById(
        "battery"
    );


// ======================================================
// ACCELEROMETER
// ======================================================

const ax =
    document.getElementById(
        "ax"
    );


const ay =
    document.getElementById(
        "ay"
    );


const az =
    document.getElementById(
        "az"
    );


// ======================================================
// GYROSCOPE
// ======================================================

const gx =
    document.getElementById(
        "gx"
    );


const gy =
    document.getElementById(
        "gy"
    );


const gz =
    document.getElementById(
        "gz"
    );


// ======================================================
// DEVICE GPS
// ======================================================

const gpsStatus =
    document.getElementById(
        "gpsStatus"
    );


const latitude =
    document.getElementById(
        "latitude"
    );


const longitude =
    document.getElementById(
        "longitude"
    );


const altitude =
    document.getElementById(
        "altitude"
    );


const gpsTime =
    document.getElementById(
        "gpsTime"
    );


// ======================================================
// LAST KNOWN FIREBASE LOCATION
// ======================================================

const lastLatitude =
    document.getElementById(
        "lastLatitude"
    );


const lastLongitude =
    document.getElementById(
        "lastLongitude"
    );


const lastAltitude =
    document.getElementById(
        "lastAltitude"
    );


const lastLocationTime =
    document.getElementById(
        "lastLocationTime"
    );


// ======================================================
// MAP
// ======================================================

const mapLatitude =
    document.getElementById(
        "mapLatitude"
    );


const mapLongitude =
    document.getElementById(
        "mapLongitude"
    );


// ======================================================
// EXPORT
// ======================================================

export {

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

    mapLatitude,
    mapLongitude

};