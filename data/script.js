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

// ================= GPS UUIDS =================

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
// DOM ELEMENTS
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

// ================= GPS DOM =================

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

const mapLatitude =
    document.getElementById("mapLatitude");

const mapLongitude =
    document.getElementById("mapLongitude");


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
// MAP VARIABLES
// ======================================================

let map;
let trackerMarker;


// ======================================================
// INITIALIZE MAP
// ======================================================

map =
    L.map("map").setView(
        [6.5244, 3.3792],
        15
    );

L.tileLayer(
    "https://{s}.tile.openstreetmap.org/{z}/{x}/{y}.png",
    {
        attribution:
            "&copy; OpenStreetMap contributors"
    }
).addTo(map);


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


        // ==================================================
        // MOTION
        // ==================================================

        motionChar =
            await service.getCharacteristic(
                MOTION_UUID
            );


        // ==================================================
        // LAST SEEN
        // ==================================================

        lastSeenChar =
            await service.getCharacteristic(
                LAST_SEEN_UUID
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

                updateTrackerMap();
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

                updateTrackerMap();
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
// DECODE BLE TEXT
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
// UPDATE MAP
// ======================================================

function updateTrackerMap()
{
    const lat =
        Number(latitude.textContent);

    const lng =
        Number(longitude.textContent);


    if (
        !Number.isFinite(lat) ||
        !Number.isFinite(lng)
    )
    {
        return;
    }


    // Update coordinate display

    if (mapLatitude)
    {
        mapLatitude.textContent =
            lat.toFixed(6);
    }

    if (mapLongitude)
    {
        mapLongitude.textContent =
            lng.toFixed(6);
    }


    const position =
        [lat, lng];


    // Create marker first time

    if (!trackerMarker)
    {
        trackerMarker =
            L.marker(position)
                .addTo(map)
                .bindPopup(
                    "Smart Tracker"
                );

        trackerMarker.openPopup();
    }

    // Move existing marker

    else
    {
        trackerMarker.setLatLng(
            position
        );
    }


    // Move map to tracker

    map.setView(
        position,
        17
    );
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
        lat:
            Number(
                latitude.textContent
            ),

        lng:
            Number(
                longitude.textContent
            ),

        battery:
            Number(
                batteryText.textContent
            ),

        time:
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


    gpsStatus.textContent =
        "SEARCHING";

    gpsStatus.className =
        "gps-searching";


    latitude.textContent =
        "--";

    longitude.textContent =
        "--";

    altitude.textContent =
        "--";

    gpsTime.textContent =
        "--";


    if (mapLatitude)
    {
        mapLatitude.textContent =
            "--";
    }

    if (mapLongitude)
    {
        mapLongitude.textContent =
            "--";
    }


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


    device =
        null;

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