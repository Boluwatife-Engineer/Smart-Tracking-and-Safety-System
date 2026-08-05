// UUIDS

const SERVICE_UUID = "6f1f9ea6-76b7-4460-918b-5fa33f709630";

const MOTION_UUID = "11111111-1111-1111-1111-111111111111";
const LAST_SEEN_UUID = "44444444-4444-4444-4444-444444444444";

const ACCEL_X_UUID = "22222222-2222-2222-2222-222222222221";
const ACCEL_Y_UUID = "22222222-2222-2222-2222-222222222222";
const ACCEL_Z_UUID = "22222222-2222-2222-2222-222222222223";

const GYRO_X_UUID = "33333333-3333-3333-3333-333333333331";
const GYRO_Y_UUID = "33333333-3333-3333-3333-333333333332";
const GYRO_Z_UUID = "33333333-3333-3333-3333-333333333333";

const LED_UUID = "dabed4fd-f792-443f-b186-3da384f9d673";

// ELEMENTS

const connectBtn = document.getElementById("connectBtn");
const ledBtn = document.getElementById("ledBtn");

const writeLastSeenBtn = document.getElementById("writeLastSeenBtn");
const readLastSeenBtn = document.getElementById("readLastSeenBtn");

const statusText = document.getElementById("status");
const motionText = document.getElementById("motion");

const ax = document.getElementById("ax");
const ay = document.getElementById("ay");
const az = document.getElementById("az");

const gx = document.getElementById("gx");
const gy = document.getElementById("gy");
const gz = document.getElementById("gz");

// BLE

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

let ledChar;

let ledOn = false;

// EVENTS

connectBtn.addEventListener("click", connect);

ledBtn.addEventListener("click", toggleLED);

writeLastSeenBtn.addEventListener("click", writeLastSeen);

readLastSeenBtn.addEventListener("click", readLastSeen);

// CONNECT

async function connect() {

    try {

        device = await navigator.bluetooth.requestDevice({

            filters: [
                {
                    name: "Smart Tracker"
                }
            ],

            optionalServices: [SERVICE_UUID]

        });

        device.addEventListener(
            "gattserverdisconnected",
            onDisconnected
        );

        server = await device.gatt.connect();

        service = await server.getPrimaryService(SERVICE_UUID);

        // Characteristics

        motionChar = await service.getCharacteristic(MOTION_UUID);

        lastSeenChar =
            await service.getCharacteristic(LAST_SEEN_UUID);

        accelXChar =
            await service.getCharacteristic(ACCEL_X_UUID);

        accelYChar =
            await service.getCharacteristic(ACCEL_Y_UUID);

        accelZChar =
            await service.getCharacteristic(ACCEL_Z_UUID);

        gyroXChar =
            await service.getCharacteristic(GYRO_X_UUID);

        gyroYChar =
            await service.getCharacteristic(GYRO_Y_UUID);

        gyroZChar =
            await service.getCharacteristic(GYRO_Z_UUID);

        ledChar =
            await service.getCharacteristic(LED_UUID);

        // Notifications

        await motionChar.startNotifications();

        await accelXChar.startNotifications();
        await accelYChar.startNotifications();
        await accelZChar.startNotifications();

        await gyroXChar.startNotifications();
        await gyroYChar.startNotifications();
        await gyroZChar.startNotifications();

        // Motion

        motionChar.addEventListener(
            "characteristicvaluechanged",
            handleMotion
        );

        // Accelerometer

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

        // Gyroscope

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

        statusText.textContent = "Connected";
        statusText.className = "status connected";

        connectBtn.disabled = true;
        connectBtn.textContent = "Connected";

        ledBtn.disabled = false;

        writeLastSeenBtn.disabled = false;
        readLastSeenBtn.disabled = false;

        console.log("Connected");

    }

    catch (error) {

        console.error(error);

    }

}

// HELPERS

function decode(event) {

    return new TextDecoder().decode(event.target.value);

}

function handleMotion(event) {

    const value = decode(event);

    motionText.textContent = value;

    if (value === "MOVING") {

        motionText.className = "motion moving";

    }

    else {

        motionText.className = "motion stationary";

    }

}

// LED

async function toggleLED() {

    if (!ledChar) return;

    const encoder = new TextEncoder();

    try {

        if (ledOn) {

            await ledChar.writeValue(
                encoder.encode("OFF")
            );

            ledBtn.textContent = "Turn ON LED";

            ledOn = false;

        }

        else {

            await ledChar.writeValue(
                encoder.encode("ON")
            );

            ledBtn.textContent = "Turn OFF LED";

            ledOn = true;

        }

    }

    catch (error) {

        console.error(error);

    }

}

// LAST SEEN

async function writeLastSeen() {

    if (!lastSeenChar) return;

    const lastSeen = {

        lat: 6.5244,

        lng: 3.3792,

        battery: 87,

        time: new Date().toISOString()

    };

    const json = JSON.stringify(lastSeen);

    try {

        await lastSeenChar.writeValue(

            new TextEncoder().encode(json)

        );

        console.log("Last Seen Written");

    }

    catch (error) {

        console.error(error);

    }

}

async function readLastSeen() {

    if (!lastSeenChar) return;

    try {

        const value = await lastSeenChar.readValue();

        const json = new TextDecoder().decode(value);

        console.log("Last Seen:");

        console.log(json);

        alert(json);

    }

    catch (error) {

        console.error(error);

    }

}

// DISCONNECT

function onDisconnected() {

    console.log("Disconnected");

    statusText.textContent = "Disconnected";

    statusText.className = "status";

    motionText.textContent = "--";

    ax.textContent = "0.00";
    ay.textContent = "0.00";
    az.textContent = "0.00";

    gx.textContent = "0.00";
    gy.textContent = "0.00";
    gz.textContent = "0.00";

    connectBtn.disabled = false;
    connectBtn.textContent = "Connect Device";

    ledBtn.disabled = true;
    ledBtn.textContent = "Turn ON LED";

    writeLastSeenBtn.disabled = true;
    readLastSeenBtn.disabled = true;

    ledOn = false;

}