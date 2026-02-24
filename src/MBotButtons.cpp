#include "MBotButtons.h"

// PIN_BUTTON_A (5) and PIN_BUTTON_B (11) are defined by the board variant

MBotButtons::MBotButtons()
    : _initialized(false) {}

void MBotButtons::ensureInit() {
    if (_initialized) return;
    _initialized = true;
    pinMode(PIN_BUTTON_A, INPUT);
    pinMode(PIN_BUTTON_B, INPUT);
}

bool MBotButtons::a() {
    ensureInit();
    return digitalRead(PIN_BUTTON_A) == LOW;  // active-low
}

bool MBotButtons::b() {
    ensureInit();
    return digitalRead(PIN_BUTTON_B) == LOW;  // active-low
}
