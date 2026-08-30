#include "sos.h"

#include <Arduino.h>

#include "pins.h"


// ======================================================
// SOS STATE
// ======================================================

static bool sosActive = false;


// ======================================================
// BUTTON STATE
// ======================================================

static bool lastButtonReading = HIGH;

static bool stableButtonState = HIGH;

static unsigned long lastDebounceTime = 0;


// ======================================================
// HOLD STATE
// ======================================================

static bool buttonHolding = false;

static unsigned long buttonPressStartTime = 0;

static bool holdTriggered = false;


// ======================================================
// STATE CHANGE FLAG
// ======================================================

static bool sosStateChanged = false;


// ======================================================
// DEBUG STATE
// ======================================================

static bool lastReportedGPIOState = HIGH;


// ======================================================
// INITIALIZE SOS
// ======================================================

void initSOS()
{
    pinMode(
        SOS_BUTTON_PIN,
        INPUT_PULLUP
    );


    // --------------------------------------------------
    // Read initial state
    // --------------------------------------------------

    lastButtonReading =
        digitalRead(SOS_BUTTON_PIN);

    stableButtonState =
        lastButtonReading;

    lastReportedGPIOState =
        lastButtonReading;


    // --------------------------------------------------
    // Reset state
    // --------------------------------------------------

    sosActive = false;

    buttonHolding = false;

    buttonPressStartTime = 0;

    holdTriggered = false;

    sosStateChanged = false;


    // --------------------------------------------------
    // Serial diagnostics
    // --------------------------------------------------

    Serial.println();

    Serial.println(
        "================================"
    );

    Serial.println(
        "SOS SYSTEM INITIALIZED"
    );

    Serial.println(
        "================================"
    );

    Serial.print(
        "SOS GPIO: "
    );

    Serial.println(
        SOS_BUTTON_PIN
    );

    Serial.println(
        "Button wiring:"
    );

    Serial.println(
        "GPIO 3 -> BUTTON -> GND"
    );

    Serial.println(
        "Released = HIGH"
    );

    Serial.println(
        "Pressed  = LOW"
    );

    Serial.print(
        "SOS hold time: "
    );

    Serial.print(
        SOS_HOLD_MS
    );

    Serial.println(
        " ms"
    );

    Serial.println(
        "Short presses are ignored."
    );

    Serial.println(
        "3-second hold toggles SOS."
    );

    Serial.println(
        "================================"
    );

    Serial.println();
}


// ======================================================
// UPDATE SOS
// ======================================================

void updateSOS()
{
    // --------------------------------------------------
    // Read GPIO
    // --------------------------------------------------

    bool reading =
        digitalRead(
            SOS_BUTTON_PIN
        );


    // ==================================================
    // RAW GPIO DEBUG
    // ==================================================

    if (
        reading !=
        lastReportedGPIOState
    )
    {
        lastReportedGPIOState =
            reading;

        Serial.print(
            "SOS GPIO changed: "
        );

        Serial.println(
            reading == HIGH
                ? "HIGH"
                : "LOW"
        );
    }


    // ==================================================
    // BUTTON READING CHANGED
    // ==================================================

    if (
        reading !=
        lastButtonReading
    )
    {
        lastDebounceTime =
            millis();

        lastButtonReading =
            reading;
    }


    // ==================================================
    // DEBOUNCE
    // ==================================================

    if (
        millis() -
        lastDebounceTime >=
        SOS_DEBOUNCE_MS
    )
    {
        // ------------------------------------------------
        // Stable button state changed
        // ------------------------------------------------

        if (
            reading !=
            stableButtonState
        )
        {
            stableButtonState =
                reading;


            // ==========================================
            // BUTTON PRESSED
            // ==========================================

            if (
                stableButtonState == LOW
            )
            {
                buttonHolding = true;

                buttonPressStartTime =
                    millis();

                holdTriggered = false;


                Serial.println();

                Serial.println(
                    "SOS BUTTON PRESSED"
                );

                Serial.println(
                    "Hold for 3 seconds..."
                );
            }


            // ==========================================
            // BUTTON RELEASED
            // ==========================================

            else
            {
                buttonHolding = false;

                buttonPressStartTime = 0;


                // --------------------------------------
                // If released before 3 seconds
                // --------------------------------------

                if (!holdTriggered)
                {
                    Serial.println(
                        "SOS button released "
                        "before 3 seconds."
                    );

                    Serial.println(
                        "SOS action cancelled."
                    );
                }
                else
                {
                    Serial.println(
                        "SOS button released "
                        "after successful hold."
                    );
                }


                holdTriggered = false;

                Serial.println();
            }
        }
    }


    // ==================================================
    // CHECK 3-SECOND HOLD
    // ==================================================

    if (
        buttonHolding &&
        !holdTriggered
    )
    {
        unsigned long heldTime =
            millis() -
            buttonPressStartTime;


        // ----------------------------------------------
        // 3 seconds reached
        // ----------------------------------------------

        if (
            heldTime >=
            SOS_HOLD_MS
        )
        {
            // ------------------------------------------
            // Prevent repeated triggering
            // while button remains held
            // ------------------------------------------

            holdTriggered = true;


            // ------------------------------------------
            // TOGGLE SOS
            // ------------------------------------------

            sosActive =
                !sosActive;


            // ------------------------------------------
            // Mark state changed
            // ------------------------------------------

            sosStateChanged = true;


            // ------------------------------------------
            // Serial
            // ------------------------------------------

            Serial.println();

            Serial.println(
                "!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!"
            );

            if (sosActive)
            {
                Serial.println(
                    "       SOS ACTIVATED"
                );
            }
            else
            {
                Serial.println(
                    "       SOS DEACTIVATED"
                );
            }

            Serial.println(
                "       3 SECOND HOLD"
            );

            Serial.println(
                "!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!"
            );

            Serial.println();
        }
    }
}


// ======================================================
// GET SOS STATE
// ======================================================

bool isSOSActive()
{
    return sosActive;
}


// ======================================================
// CLEAR SOS
// ======================================================

void clearSOS()
{
    if (!sosActive)
    {
        return;
    }


    sosActive = false;

    sosStateChanged = true;


    Serial.println();

    Serial.println(
        "SOS CLEARED."
    );

    Serial.println();
}


// ======================================================
// CHECK STATE CHANGE
// ======================================================

bool hasSOSStateChanged()
{
    return sosStateChanged;
}


// ======================================================
// CLEAR STATE CHANGE FLAG
// ======================================================

void clearSOSStateChanged()
{
    sosStateChanged = false;
}