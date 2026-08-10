// ======================================================
// UUIDS
// ======================================================

const SERVICE_UUID =
    "6f1f9ea6-76b7-4460-918b-5fa33f709630";

const MOTION_UUID =
    "11111111-1111-1111-1111-111111111111";

const LAST_SEEN_UUID =
    "44444444-4444-4444-4444-444444444444";

const ACCEL_X_UUID =
    "22222222-2222-2222-2222-222222222221";

const ACCEL_Y_UUID =
    "22222222-2222-2222-2222-222222222222";

const ACCEL_Z_UUID =
    "22222222-2222-2222-2222-222222222223";

const GYRO_X_UUID =
    "33333333-3333-3333-3333-333333333331";

const GYRO_Y_UUID =
    "33333333-3333-3333-3333-333333333332";

const GYRO_Z_UUID =
    "33333333-3333-3333-3333-333333333333";

const BATTERY_UUID =
    "55555555-5555-5555-5555-555555555555";

const BUZZER_UUID =
    "dabed4fd-f792-443f-b186-3da384f9d673";

// GPS

const GPS_LATITUDE_UUID =
    "66666666-6666-6666-6666-666666666661";

const GPS_LONGITUDE_UUID =
    "66666666-6666-6666-6666-666666666662";

const GPS_ALTITUDE_UUID =
    "66666666-6666-6666-6666-666666666663";

const GPS_TIME_UUID =
    "66666666-6666-6666-6666-666666666664";

const GPS_STATUS_UUID =
    "66666666-6666-6666-6666-666666666665";


// ======================================================
// DOM
// ======================================================

const connectBtn =
    document.getElementById("connectBtn");

const ledBtn =
    document.getElementById("ledBtn");

const writeLastSeenBtn =
    document.getElementById("writeLastSeenBtn");

const readLastSeenBtn =
    document.getElementById("readLastSeenBtn");

const statusText =
    document.getElementById("status");

const motionText =
    document.getElementById("motion");

const batteryText =
    document.getElementById("battery");

const ax =
    document.getElementById("ax");

const ay =
    document.getElementById("ay");

const az =
    document.getElementById("az");

const gx =
    document.getElementById("gx");

const gy =
    document.getElementById("gy");

const gz =
    document.getElementById("gz");

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
// BLE VARIABLES
// ======================================================

let device;
let server;
let service;

let motionChar;
let lastSeenChar;

let accelXChar;
let accelYChar;
let accelZChar;

let gyroXChar;
let gyroYChar;
let gyroZChar;

let batteryChar;
let ledChar;

let gpsLatitudeChar;
let gpsLongitudeChar;
let gpsAltitudeChar;
let gpsTimeChar;
let gpsStatusChar;

let ledOn = false;


// ======================================================
// EVENTS
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
// CONNECT
// ======================================================

async function connect()
{
    try
    {
        device =
            await navigator.bluetooth.requestDevice({

                filters: [
                    {
                        name: "Smart Tracker"
                    }
                ],

                optionalServices: [
                    SERVICE_UUID
                ]

            });

        device.addEventListener(
            "gattserverdisconnected",
            onDisconnected
        );

        server =
            await device.gatt.connect();

        service =
            await server.getPrimaryService(
                SERVICE_UUID
            );


        // ================= MOTION =================

        motionChar =
            await service.getCharacteristic(
                MOTION_UUID
            );


        // ================= LAST SEEN =================

        lastSeenChar =
            await service.getCharacteristic(
                LAST_SEEN_UUID
            );


        // ================= ACCEL =================

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


        // ================= GYRO =================

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


        // ================= BATTERY =================

        batteryChar =
            await service.getCharacteristic(
                BATTERY_UUID
            );


        // ================= BUZZER =================

        ledChar =
            await service.getCharacteristic(
                BUZZER_UUID
            );


        // ================= GPS =================

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
        // NOTIFICATIONS
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
        // ACCEL
        // ==================================================

        accelXChar.addEventListener(
            "characteristicvaluechanged",
            e => ax.textContent = decode(e)
        );

        accelYChar.addEventListener(
            "characteristicvaluechanged",
            e => ay.textContent = decode(e)
        );

        accelZChar.addEventListener(
            "characteristicvaluechanged",
            e => az.textContent = decode(e)
        );


        // ==================================================
        // GYRO
        // ==================================================

        gyroXChar.addEventListener(
            "characteristicvaluechanged",
            e => gx.textContent = decode(e)
        );

        gyroYChar.addEventListener(
            "characteristicvaluechanged",
            e => gy.textContent = decode(e)
        );

        gyroZChar.addEventListener(
            "characteristicvaluechanged",
            e => gz.textContent = decode(e)
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
        // GPS
        // ==================================================

        gpsLatitudeChar.addEventListener(
            "characteristicvaluechanged",
            event =>
            {
                latitude.textContent =
                    decode(event);
            }
        );


        gpsLongitudeChar.addEventListener(
            "characteristicvaluechanged",
            event =>
            {
                longitude.textContent =
                    decode(event);
            }
        );


        gpsAltitudeChar.addEventListener(
            "characteristicvaluechanged",
            event =>
            {
                altitude.textContent =
                    decode(event);
            }
        );


        gpsTimeChar.addEventListener(
            "characteristicvaluechanged",
            event =>
            {
                gpsTime.textContent =
                    decode(event);
            }
        );


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
        // INITIAL BATTERY READ
        // ==================================================

        const batteryValue =
            await batteryChar.readValue();

        batteryText.textContent =
            batteryValue.getUint8(0);


        // ==================================================
        // INITIAL GPS READ
        // ==================================================

        const gpsStatusValue =
            await gpsStatusChar.readValue();

        gpsStatus.textContent =
            decodeValue(gpsStatusValue);


        // ==================================================
        // UI
        // ==================================================

        statusText.textContent =
            "Connected";

        statusText.className =
            "status connected";

        connectBtn.disabled = true;

        connectBtn.textContent =
            "Connected";

        ledBtn.disabled = false;

        writeLastSeenBtn.disabled = false;

        readLastSeenBtn.disabled = false;

        console.log(
            "Smart Tracker connected"
        );
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
        return;

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

            ledOn = false;
        }
        else
        {
            await ledChar.writeValue(
                encoder.encode("ON")
            );

            ledBtn.textContent =
                "Turn OFF Sound";

            ledOn = true;
        }
    }
    catch(error)
    {
        console.error(error);
    }
}


// ======================================================
// LAST SEEN
// ======================================================

async function writeLastSeen()
{
    if (!lastSeenChar)
        return;

    const lastSeen =
    {
        lat: Number(latitude.textContent),
        lng: Number(longitude.textContent),

        battery:
            Number(
                batteryText.textContent
            ),

        time:
            new Date().toISOString()
    };

    const json =
        JSON.stringify(lastSeen);

    try
    {
        await lastSeenChar.writeValue(
            new TextEncoder().encode(json)
        );

        console.log(
            "Last Seen Written:",
            json
        );
    }
    catch(error)
    {
        console.error(error);
    }
}


// ======================================================
// READ LAST SEEN
// ======================================================

async function readLastSeen()
{
    if (!lastSeenChar)
        return;

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
        console.error(error);
    }
}


// ======================================================
// DISCONNECT
// ======================================================

function onDisconnected()
{
    console.log(
        "Smart Tracker disconnected"
    );

    statusText.textContent =
        "Disconnected";

    statusText.className =
        "status";

    motionText.textContent =
        "--";

    motionText.className =
        "motion";

    batteryText.textContent =
        "--";

    ax.textContent = "0.00";
    ay.textContent = "0.00";
    az.textContent = "0.00";

    gx.textContent = "0.00";
    gy.textContent = "0.00";
    gz.textContent = "0.00";

    gpsStatus.textContent =
        "Searching...";

    gpsStatus.className =
        "gps-searching";

    latitude.textContent = "--";
    longitude.textContent = "--";
    altitude.textContent = "--";
    gpsTime.textContent = "--";

    connectBtn.disabled = false;

    connectBtn.textContent =
        "Connect Device";

    ledBtn.disabled = true;

    ledBtn.textContent =
        "Turn ON Sound";

    writeLastSeenBtn.disabled = true;

    readLastSeenBtn.disabled = true;

    ledOn = false;

    device = null;
    server = null;
    service = null;

    motionChar = null;
    lastSeenChar = null;

    accelXChar = null;
    accelYChar = null;
    accelZChar = null;

    gyroXChar = null;
    gyroYChar = null;
    gyroZChar = null;

    batteryChar = null;
    ledChar = null;

    gpsLatitudeChar = null;
    gpsLongitudeChar = null;
    gpsAltitudeChar = null;
    gpsTimeChar = null;
    gpsStatusChar = null;
}