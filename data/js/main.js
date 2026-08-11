import {
    connect,
    autoConnect,
    toggleLED,
    writeLastSeen,
    readLastSeen
} from "./ble.js";


import {
    connectBtn,
    ledBtn,
    writeLastSeenBtn,
    readLastSeenBtn
} from "./dom.js";


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
// AUTOMATIC RECONNECT
// ======================================================

autoConnect();