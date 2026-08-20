// ======================================================
// MAP VARIABLES
// ======================================================

import {
    mapLatitude,
    mapLongitude
} from "./dom.js";


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
            "© OpenStreetMap contributors"
    }
).addTo(map);


// ======================================================
// UPDATE MAP FROM FIREBASE
// ======================================================

function updateTrackerMap(
    lat,
    lng
)
{
    lat =
        Number(lat);

    lng =
        Number(lng);


    if (
        !Number.isFinite(lat) ||
        !Number.isFinite(lng)
    )
    {
        return;
    }


    // ==============================================
    // DISPLAY COORDINATES
    // ==============================================

    mapLatitude.textContent =
        lat.toFixed(6);

    mapLongitude.textContent =
        lng.toFixed(6);


    const position =
        [
            lat,
            lng
        ];


    // ==============================================
    // CREATE MARKER
    // ==============================================

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


    // ==============================================
    // MOVE MARKER
    // ==============================================

    else
    {
        trackerMarker.setLatLng(
            position
        );
    }


    // ==============================================
    // CENTER MAP
    // ==============================================

    map.setView(
        position,
        17
    );
}


export {
    updateTrackerMap
};