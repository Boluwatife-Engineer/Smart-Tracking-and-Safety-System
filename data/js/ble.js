// ======================================================
// BLE IMPORTS
// ======================================================

import {

    SERVICE_UUID,

    MOTION_UUID,

    ACCEL_X_UUID,
    ACCEL_Y_UUID,
    ACCEL_Z_UUID,

    GYRO_X_UUID,
    GYRO_Y_UUID,
    GYRO_Z_UUID,

    BATTERY_UUID,
    BUZZER_UUID,

    GPS_LATITUDE_UUID,
    GPS_LONGITUDE_UUID,
    GPS_ALTITUDE_UUID,
    GPS_TIME_UUID,
    GPS_STATUS_UUID,

    SOS_UUID

} from "./uuids.js";


// ======================================================
// DOM
// ======================================================

import {

    connectBtn,
    ledBtn,

    statusText,
    bleStatus,

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

    sosStatus

} from "./dom.js";


// ======================================================
// MAP
// ======================================================

import {

    updateTrackerMap

} from "./map.js";


// ======================================================
// BLE VARIABLES
// ======================================================

let device = null;
let server = null;
let service = null;

let motionChar = null;

let accelXChar = null;
let accelYChar = null;
let accelZChar = null;

let gyroXChar = null;
let gyroYChar = null;
let gyroZChar = null;

let batteryChar = null;
let ledChar = null;

let gpsLatitudeChar = null;
let gpsLongitudeChar = null;
let gpsAltitudeChar = null;
let gpsTimeChar = null;
let gpsStatusChar = null;

let sosChar = null;

let ledOn = false;


// ======================================================
// BLE CONNECTION STATE
// ======================================================
//
// This is the source of truth for the dashboard.
//
// true  = BLE is connected, BLE owns Device GPS fields
// false = BLE is disconnected, Firebase /current owns
//         Device GPS fields
//
// ======================================================

let bleConnected = false;


// ======================================================
// FIRST-TIME CONNECTION
// ======================================================

async function connect()
{
    try
    {
        console.log(
            "Requesting Smart Tracker..."
        );


        device =
            await navigator.bluetooth.requestDevice(
            {
                filters:
                [
                    {
                        name:
                            "Smart Tracker"
                    }
                ],

                optionalServices:
                [
                    SERVICE_UUID
                ]
            });


        console.log(
            "Device selected:",
            device.name
        );


        setupDisconnectListener();


        await connectToDevice();
    }

    catch(error)
    {
        console.error(
            "BLE connection error:",
            error
        );
    }
}


// ======================================================
// AUTOMATIC CONNECTION
// ======================================================

async function autoConnect()
{
    try
    {
        if (
            !navigator.bluetooth ||
            !navigator.bluetooth.getDevices
        )
        {
            console.log(
                "Automatic BLE reconnect is not supported."
            );

            return;
        }


        const devices =
            await navigator.bluetooth.getDevices();


        const tracker =
            devices.find(
                item =>
                    item.name ===
                    "Smart Tracker"
            );


        if (!tracker)
        {
            console.log(
                "No previously authorized Smart Tracker found."
            );

            return;
        }


        device =
            tracker;


        setupDisconnectListener();


        await connectToDevice();
    }

    catch(error)
    {
        console.error(
            "Automatic BLE connection failed:",
            error
        );
    }
}


// ======================================================
// DISCONNECT LISTENER
// ======================================================

function setupDisconnectListener()
{
    if (!device)
    {
        return;
    }


    device.removeEventListener(
        "gattserverdisconnected",
        onDisconnected
    );


    device.addEventListener(
        "gattserverdisconnected",
        onDisconnected
    );
}


// ======================================================
// CONNECT TO DEVICE
// ======================================================

async function connectToDevice()
{
    if (!device)
    {
        return;
    }


    try
    {
        console.log(
            "Connecting to GATT server..."
        );


        if (!device.gatt.connected)
        {
            server =
                await device.gatt.connect();
        }

        else
        {
            server =
                device.gatt;
        }


        console.log(
            "GATT connected."
        );


        // ==================================================
        // SERVICE
        // ==================================================

        service =
            await server.getPrimaryService(
                SERVICE_UUID
            );


        // ==================================================
        // MOTION
        // ==================================================

        motionChar =
            await service.getCharacteristic(
                MOTION_UUID
            );


        // ==================================================
        // ACCELEROMETER
        // ==================================================

        accelXChar =
            await service.getCharacteristic(
                ACCEL_X_UUID
            );

        accelYChar =
            await service.getCharacteristic(
                ACCEL_Y_UUID
            );

        accelZChar =
            await service.getCharacteristic(
                ACCEL_Z_UUID
            );


        // ==================================================
        // GYROSCOPE
        // ==================================================

        gyroXChar =
            await service.getCharacteristic(
                GYRO_X_UUID
            );

        gyroYChar =
            await service.getCharacteristic(
                GYRO_Y_UUID
            );

        gyroZChar =
            await service.getCharacteristic(
                GYRO_Z_UUID
            );


        // ==================================================
        // BATTERY
        // ==================================================

        batteryChar =
            await service.getCharacteristic(
                BATTERY_UUID
            );


        // ==================================================
        // BUZZER
        // ==================================================

        ledChar =
            await service.getCharacteristic(
                BUZZER_UUID
            );


        // ==================================================
        // GPS
        // ==================================================

        gpsLatitudeChar =
            await service.getCharacteristic(
                GPS_LATITUDE_UUID
            );

        gpsLongitudeChar =
            await service.getCharacteristic(
                GPS_LONGITUDE_UUID
            );

        gpsAltitudeChar =
            await service.getCharacteristic(
                GPS_ALTITUDE_UUID
            );

        gpsTimeChar =
            await service.getCharacteristic(
                GPS_TIME_UUID
            );

        gpsStatusChar =
            await service.getCharacteristic(
                GPS_STATUS_UUID
            );


        // ==================================================
        // SOS
        // ==================================================

        sosChar =
            await service.getCharacteristic(
                SOS_UUID
            );


        // ==================================================
        // START NOTIFICATIONS
        // ==================================================

        await motionChar.startNotifications();

        await accelXChar.startNotifications();
        await accelYChar.startNotifications();
        await accelZChar.startNotifications();

        await gyroXChar.startNotifications();
        await gyroYChar.startNotifications();
        await gyroZChar.startNotifications();

        await batteryChar.startNotifications();

        await gpsLatitudeChar.startNotifications();
        await gpsLongitudeChar.startNotifications();
        await gpsAltitudeChar.startNotifications();
        await gpsTimeChar.startNotifications();
        await gpsStatusChar.startNotifications();

        await sosChar.startNotifications();


        // ==================================================
        // MOTION
        // ==================================================

        motionChar.addEventListener(
            "characteristicvaluechanged",
            handleMotion
        );


        // ==================================================
        // ACCELEROMETER
        // ==================================================

        accelXChar.addEventListener(
            "characteristicvaluechanged",
            event =>
            {
                ax.textContent =
                    decode(event);
            }
        );


        accelYChar.addEventListener(
            "characteristicvaluechanged",
            event =>
            {
                ay.textContent =
                    decode(event);
            }
        );


        accelZChar.addEventListener(
            "characteristicvaluechanged",
            event =>
            {
                az.textContent =
                    decode(event);
            }
        );


        // ==================================================
        // GYROSCOPE
        // ==================================================

        gyroXChar.addEventListener(
            "characteristicvaluechanged",
            event =>
            {
                gx.textContent =
                    decode(event);
            }
        );


        gyroYChar.addEventListener(
            "characteristicvaluechanged",
            event =>
            {
                gy.textContent =
                    decode(event);
            }
        );


        gyroZChar.addEventListener(
            "characteristicvaluechanged",
            event =>
            {
                gz.textContent =
                    decode(event);
            }
        );


        // ==================================================
        // BATTERY
        // ==================================================

        batteryChar.addEventListener(
            "characteristicvaluechanged",
            event =>
            {
                batteryText.textContent =
                    event.target.value.getUint8(0);
            }
        );


        // ==================================================
        // GPS LATITUDE
        // ==================================================

        gpsLatitudeChar.addEventListener(
            "characteristicvaluechanged",
            event =>
            {
                if (!bleConnected)
                {
                    return;
                }

                const value =
                    decode(event);

                latitude.textContent =
                    value;

                updateMap();
            }
        );


        // ==================================================
        // GPS LONGITUDE
        // ==================================================

        gpsLongitudeChar.addEventListener(
            "characteristicvaluechanged",
            event =>
            {
                if (!bleConnected)
                {
                    return;
                }

                const value =
                    decode(event);

                longitude.textContent =
                    value;

                updateMap();
            }
        );


        // ==================================================
        // GPS ALTITUDE
        // ==================================================

        gpsAltitudeChar.addEventListener(
            "characteristicvaluechanged",
            event =>
            {
                if (!bleConnected)
                {
                    return;
                }

                altitude.textContent =
                    decode(event);
            }
        );


        // ==================================================
        // GPS TIME
        // ==================================================

        gpsTimeChar.addEventListener(
            "characteristicvaluechanged",
            event =>
            {
                if (!bleConnected)
                {
                    return;
                }

                gpsTime.textContent =
                    decode(event);
            }
        );


        // ==================================================
        // GPS STATUS
        // ==================================================

        gpsStatusChar.addEventListener(
            "characteristicvaluechanged",
            event =>
            {
                if (!bleConnected)
                {
                    return;
                }

                updateGPSStatus(
                    decode(event)
                );
            }
        );


        // ==================================================
        // SOS
        // ==================================================

        sosChar.addEventListener(
            "characteristicvaluechanged",
            event =>
            {
                updateSOSStatus(
                    decode(event)
                );
            }
        );


        // ==================================================
        // MARK BLE AS CONNECTED
        // ==================================================
        //
        // Do this before reading the initial GPS status.
        // This ensures BLE is the owner of Device GPS
        // immediately after connection.
        //
        // ==================================================

        bleConnected = true;


        // ==================================================
        // INITIAL BATTERY
        // ==================================================

        const batteryValue =
            await batteryChar.readValue();


        batteryText.textContent =
            batteryValue.getUint8(0);


        // ==================================================
        // INITIAL GPS STATUS
        // ==================================================

        const gpsStatusValue =
            await gpsStatusChar.readValue();


        updateGPSStatus(
            decodeValue(
                gpsStatusValue
            )
        );


        // ==================================================
        // INITIAL SOS STATUS
        // ==================================================

        const sosValue =
            await sosChar.readValue();


        updateSOSStatus(
            decodeValue(
                sosValue
            )
        );


        // ==================================================
        // CONNECTED UI
        // ==================================================

        setConnectedUI();


        console.log(
            "Smart Tracker fully connected."
        );
    }

    catch(error)
    {
        bleConnected = false;

        console.error(
            "Failed to connect to Smart Tracker:",
            error
        );
    }
}


// ======================================================
// CONNECTED UI
// ======================================================

function setConnectedUI()
{
    statusText.textContent =
        "Connected";


    statusText.className =
        "status connected";


    if (bleStatus)
    {
        bleStatus.textContent =
            "Connected";

        bleStatus.className =
            "gps-fixed";
    }


    connectBtn.disabled =
        true;


    connectBtn.textContent =
        "Connected";


    if (ledBtn)
    {
        ledBtn.disabled =
            false;
    }
}


// ======================================================
// SOS STATUS
// ======================================================

function updateSOSStatus(value)
{
    const status =
        String(value).trim();


    sosStatus.textContent =
        status;


    if (
        status ===
        "ACTIVE"
    )
    {
        sosStatus.className =
            "sos-active";
    }

    else
    {
        sosStatus.className =
            "sos-inactive";
    }
}


// ======================================================
// GPS STATUS
// ======================================================

function updateGPSStatus(value)
{
    gpsStatus.textContent =
        value;


    if (
        value ===
        "FIXED"
    )
    {
        gpsStatus.className =
            "gps-fixed";
    }

    else
    {
        gpsStatus.className =
            "gps-searching";
    }
}


// ======================================================
// MAP
// ======================================================

function updateMap()
{
    const lat =
        Number(
            latitude.textContent
        );


    const lng =
        Number(
            longitude.textContent
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


// ======================================================
// DECODE
// ======================================================

function decode(event)
{
    return new TextDecoder().decode(
        event.target.value
    );
}


function decodeValue(value)
{
    return new TextDecoder().decode(
        value
    );
}


// ======================================================
// MOTION
// ======================================================

function handleMotion(event)
{
    const value =
        decode(event);


    motionText.textContent =
        value;


    if (
        value ===
        "MOVING"
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


// ======================================================
// BUZZER
// ======================================================

async function toggleLED()
{
    if (!ledChar)
    {
        return;
    }


    const encoder =
        new TextEncoder();


    try
    {
        if (ledOn)
        {
            await ledChar.writeValue(
                encoder.encode("OFF")
            );


            ledBtn.textContent =
                "Turn ON Sound";


            ledOn =
                false;
        }

        else
        {
            await ledChar.writeValue(
                encoder.encode("ON")
            );


            ledBtn.textContent =
                "Turn OFF Sound";


            ledOn =
                true;
        }
    }

    catch(error)
    {
        console.error(
            "Buzzer error:",
            error
        );
    }
}


// ======================================================
// DISCONNECTED
// ======================================================

function onDisconnected()
{
    console.log(
        "Smart Tracker disconnected."
    );


    // ==================================================
    // BLE CONNECTION STATE
    // ==================================================

    bleConnected =
        false;


    // ==================================================
    // BLE CONNECTION STATUS
    // ==================================================

    statusText.textContent =
        "Disconnected";

    statusText.className =
        "status";


    if (bleStatus)
    {
        bleStatus.textContent =
            "Disconnected";

        bleStatus.className =
            "gps-searching";
    }


    connectBtn.disabled =
        false;

    connectBtn.textContent =
        "Connect Device";


    // ==================================================
    // DEVICE GPS
    // ==================================================
    //
    // IMPORTANT:
    //
    // DO NOT clear latitude/longitude/altitude/gpsTime.
    //
    // After BLE disconnects, dashboard.js will read
    // Firebase /current and use the tracker GSM/GPS
    // location to keep these fields updated.
    //
    // Last Known Location remains separate and continues
    // to come only from Firebase /lastSeen.
    //
    // ==================================================

    gpsStatus.textContent =
        "GSM/GPS";

    gpsStatus.className =
        "gps-fixed";


    // ==================================================
    // SOS
    // ==================================================
    //
    // DO NOTHING HERE.
    //
    // SOS is independent of BLE.
    // Keep the last known SOS state on screen.
    //
    // ==================================================


    // ==================================================
    // BUZZER
    // ==================================================

    if (ledBtn)
    {
        ledBtn.disabled =
            true;

        ledBtn.textContent =
            "Turn ON Sound";
    }


    ledOn =
        false;


    // ==================================================
    // CLEAR BLE REFERENCES
    // ==================================================

    server =
        null;

    service =
        null;

    motionChar =
        null;

    accelXChar =
        null;

    accelYChar =
        null;

    accelZChar =
        null;

    gyroXChar =
        null;

    gyroYChar =
        null;

    gyroZChar =
        null;

    batteryChar =
        null;

    ledChar =
        null;

    gpsLatitudeChar =
        null;

    gpsLongitudeChar =
        null;

    gpsAltitudeChar =
        null;

    gpsTimeChar =
        null;

    gpsStatusChar =
        null;

    sosChar =
        null;
}


// ======================================================
// BLE CONNECTION STATE
// ======================================================
//
// Dashboard uses this to decide which source owns
// the Device GPS fields.
//
// ======================================================

function isBLEConnected()
{
    return bleConnected;
}


// ======================================================
// EXPORTS
// ======================================================

export {

    connect,
    autoConnect,
    toggleLED,
    isBLEConnected

};