const SERVICE_UUID = "6f1f9ea6-76b7-4460-918b-5fa33f709630";

const TEMP_UUID = "bc35d307-d854-44ca-96fd-f5e5e08fd3c4";
const HUM_UUID = "b6042e4d-e374-4666-8159-aecd1e097b5c";

let tempCharacteristic;
let humCharacteristic;

document
    .getElementById("connectBtn")
    .addEventListener("click", connect);

async function connect() {

    try {

        const device = await navigator.bluetooth.requestDevice({

            filters: [
                {
                    services: [SERVICE_UUID]
                }
            ]

        });

        console.log(device.name);

        const server = await device.gatt.connect();

        console.log("Connected");

        const service = await server.getPrimaryService(SERVICE_UUID);

        tempCharacteristic = await service.getCharacteristic(TEMP_UUID);

        humCharacteristic = await service.getCharacteristic(HUM_UUID);

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

        console.log("Notifications Enabled");

    } catch (err) {

        console.log(err);

    }
}

function handleTemperature(event) {

    const value = new TextDecoder().decode(event.target.value);

    document.getElementById("temp").innerHTML = value + " °C";

    console.log("Temperature:", value);

}

function handleHumidity(event) {

    const value = new TextDecoder().decode(event.target.value);

    document.getElementById("hum").innerHTML = value + " %";

    console.log("Humidity:", value);

}