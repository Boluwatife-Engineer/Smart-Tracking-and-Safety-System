
#ifndef TRACKER_STATE_H
#define TRACKER_STATE_H

enum TrackerMode
{
    MODE_BLE_CONNECTED,
    MODE_AUTONOMOUS,
    MODE_STATIONARY,
    MODE_MOVING,
    MODE_SOS
};

TrackerMode getTrackerMode();

void updateTrackerMode();

const char* getTrackerModeName();

#endif