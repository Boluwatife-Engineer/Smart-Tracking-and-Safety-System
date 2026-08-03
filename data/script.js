const SERVICE_UUID = "6f1f9ea6-76b7-4460-918b-5fa33f709630";

const TEMP_UUID = "bc35d307-d854-44ca-96fd-f5e5e08fd3c4";
const HUM_UUID  = "b6042e4d-e374-4666-8159-aecd1e097b5c";
const LED_UUID  = "dabed4fd-f792-443f-b186-3da384f9d673";

//========================================

const connectBtn = document.getElementById("connectBtn");

const tempText = document.getElementById("temp");
const humText = document.getElementById("hum");

const ledBtn = document.getElementById("ledBtn");

//========================================

let device;
let server;

let tempCharacteristic;
let humCharacteristic;
let ledCharacteristic;

let ledOn = false;

//========================================

connectBtn.addEventListener("click", connect);

ledBtn.addEventListener("click", toggleLED);

//========================================

async function connect() {

    try {

        device = await navigator.bluetooth.requestDevice({

            filters: [
                {
                    name: "My Tracker"
                }
            ],

            optionalServices: [SERVICE_UUID]

        });

        device.addEventListener(
            "gattserverdisconnected",
            onDisconnected
        );

        server = await device.gatt.connect();

        const service =
            await server.getPrimaryService(SERVICE_UUID);

        tempCharacteristic =
            await service.getCharacteristic(TEMP_UUID);

        humCharacteristic =
            await service.getCharacteristic(HUM_UUID);

        ledCharacteristic =
            await service.getCharacteristic(LED_UUID);

        //----------------------------------

        const ledValue =
            await ledCharacteristic.readValue();

        const state =
            new TextDecoder().decode(ledValue);

        ledOn = (state === "ON");

        ledBtn.textContent =
            ledOn ? "Turn OFF" : "Turn ON";

        //----------------------------------

        await tempCharacteristic.startNotifications();

        await humCharacteristic.startNotifications();

        tempCharacteristic.addEventListener(
            "characteristicvaluechanged",
            handleTemperature
        );

        humCharacteristic.addEventListener(
            "characteristicvaluechanged",
            handleHumidity
        );

        connectBtn.textContent = "Connected";

        connectBtn.disabled = true;

        ledBtn.disabled = false;

        console.log("Connected");

    }

    catch (error) {

        console.error(error);

        connectBtn.textContent = "Connect";

        connectBtn.disabled = false;

    }

}

//========================================

function handleTemperature(event) {

    const value = new TextDecoder().decode(
        event.target.value
    );

    tempText.textContent = value + " °C";

}

//========================================

function handleHumidity(event) {

    const value = new TextDecoder().decode(
        event.target.value
    );

    humText.textContent = value + " %";

}

//========================================

async function toggleLED() {

    if (!ledCharacteristic) return;

    const encoder = new TextEncoder();

    try {

        if (ledOn) {

            await ledCharacteristic.writeValue(
                encoder.encode("OFF")
            );

            ledOn = false;

            ledBtn.textContent = "Turn ON";

            console.log("LED OFF");

        }

        else {

            await ledCharacteristic.writeValue(
                encoder.encode("ON")
            );

            ledOn = true;

            ledBtn.textContent = "Turn OFF";

            console.log("LED ON");

        }

    }

    catch (error) {

        console.error(error);

    }

}

//========================================

function onDisconnected() {

    console.log("Disconnected");

    connectBtn.textContent = "Connect";

    connectBtn.disabled = false;

    ledBtn.disabled = true;

    ledBtn.textContent = "Turn ON";

    tempText.textContent = "-C";

    humText.textContent = "-%";

    ledOn = false;

    tempCharacteristic = null;
    humCharacteristic = null;
    ledCharacteristic = null;

}