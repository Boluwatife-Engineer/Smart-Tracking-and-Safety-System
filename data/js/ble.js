// ======================================================
// BLE IMPORTS
// ======================================================

import {
    SERVICE_UUID,

    MOTION_UUID,
    LAST_SEEN_UUID,

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
    GPS_STATUS_UUID
} from "./uuids.js";


import {
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
    gpsTime
} from "./dom.js";


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
let lastSeenChar = null;

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

let ledOn = false;


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
                        name: "Smart Tracker"
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
                "Bluetooth automatic reconnect is not supported."
            );

            return;
        }

        const devices =
            await navigator.bluetooth.getDevices();

        const tracker =
            devices.find(
                device =>
                    device.name === "Smart Tracker"
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
// DISCONNECT EVENT
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
    // CHARACTERISTICS
    // ==================================================

    motionChar =
        await service.getCharacteristic(
            MOTION_UUID
        );

    lastSeenChar =
        await service.getCharacteristic(
            LAST_SEEN_UUID
        );


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


    batteryChar =
        await service.getCharacteristic(
            BATTERY_UUID
        );


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
            const value =
                decode(event);

            latitude.textContent =
                value;

            updateTrackerMap(
                latitude.textContent,
                longitude.textContent
            );
        }
    );


    // ==================================================
    // GPS LONGITUDE
    // ==================================================

    gpsLongitudeChar.addEventListener(
        "characteristicvaluechanged",
        event =>
        {
            const value =
                decode(event);

            longitude.textContent =
                value;

            updateTrackerMap(
                latitude.textContent,
                longitude.textContent
            );
        }
    );


    // ==================================================
    // GPS ALTITUDE
    // ==================================================

    gpsAltitudeChar.addEventListener(
        "characteristicvaluechanged",
        event =>
        {
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
            const value =
                decode(event);

            gpsStatus.textContent =
                value;

            if (value === "FIXED")
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
    );


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

    gpsStatus.textContent =
        decodeValue(
            gpsStatusValue
        );


    // ==================================================
    // UI
    // ==================================================

    setConnectedUI();

    console.log(
        "Smart Tracker fully connected."
    );
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

    connectBtn.disabled =
        true;

    connectBtn.textContent =
        "Connected";

    ledBtn.disabled =
        false;

    writeLastSeenBtn.disabled =
        false;

    readLastSeenBtn.disabled =
        false;
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

    if (value === "MOVING")
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
// WRITE LAST SEEN
// ======================================================

async function writeLastSeen()
{
    if (!lastSeenChar)
    {
        return;
    }

    const lastSeen =
    {
        latitude:
            Number(
                latitude.textContent
            ),

        longitude:
            Number(
                longitude.textContent
            ),

        altitude:
            Number(
                altitude.textContent
            ),

        battery:
            Number(
                batteryText.textContent
            ),

        timestamp:
            new Date().toISOString()
    };

    const json =
        JSON.stringify(
            lastSeen
        );

    try
    {
        await lastSeenChar.writeValue(
            new TextEncoder().encode(
                json
            )
        );

        console.log(
            "Last Seen Written:",
            json
        );
    }

    catch(error)
    {
        console.error(
            "Last Seen write error:",
            error
        );
    }
}


// ======================================================
// READ LAST SEEN
// ======================================================

async function readLastSeen()
{
    if (!lastSeenChar)
    {
        return;
    }

    try
    {
        const value =
            await lastSeenChar.readValue();

        const json =
            new TextDecoder().decode(
                value
            );

        console.log(
            "Last Seen:",
            json
        );

        alert(json);
    }

    catch(error)
    {
        console.error(
            "Last Seen read error:",
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
    // BLE STATUS
    // ==================================================

    statusText.textContent =
        "Disconnected";

    statusText.className =
        "status";


    // ==================================================
    // IMPORTANT
    //
    // DO NOT CLEAR GPS.
    //
    // Firebase will continue supplying GPS.
    // ==================================================


    // ==================================================
    // SENSOR VALUES
    // ==================================================

    motionText.textContent =
        "--";

    motionText.className =
        "motion";


    batteryText.textContent =
        "--";


    ax.textContent =
        "0.00";

    ay.textContent =
        "0.00";

    az.textContent =
        "0.00";


    gx.textContent =
        "0.00";

    gy.textContent =
        "0.00";

    gz.textContent =
        "0.00";


    // ==================================================
    // DO NOT CLEAR THESE:
    //
    // latitude
    // longitude
    // altitude
    // gpsTime
    //
    // Firebase owns current GPS.
    // ==================================================


    // ==================================================
    // BUTTONS
    // ==================================================

    connectBtn.disabled =
        false;

    connectBtn.textContent =
        "Connect Device";

    ledBtn.disabled =
        true;

    ledBtn.textContent =
        "Turn ON Sound";

    writeLastSeenBtn.disabled =
        true;

    readLastSeenBtn.disabled =
        true;


    ledOn =
        false;


    // ==================================================
    // CLEAR BLE OBJECTS
    // ==================================================

    server =
        null;

    service =
        null;

    motionChar =
        null;

    lastSeenChar =
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
}


// ======================================================
// EXPORT
// ======================================================

export {
    connect,
    autoConnect,
    toggleLED,
    writeLastSeen,
    readLastSeen
};