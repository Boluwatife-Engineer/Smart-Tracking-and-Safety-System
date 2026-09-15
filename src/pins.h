#ifndef PINS_H
#define PINS_H

// ======================================================
// BUZZER
// ======================================================

#define BUZZER_PIN 4


// ======================================================
// SOS BUTTON
// ======================================================

#define SOS_BUTTON_PIN 3

// Button must remain pressed for 3 seconds
// before SOS changes state.

#define SOS_HOLD_MS 3000

// Debounce time

#define SOS_DEBOUNCE_MS 50

#endif