#include "MBotLineTracker.h"
#include "bsp.h"

MBotLineTracker::MBotLineTracker()
    : _initialized(false) {}

void MBotLineTracker::ensureInit() {
    if (_initialized) return;
    _initialized = true;
    pinMode(LINE_LEFT_PIN, INPUT);
    pinMode(LINE_RIGHT_PIN, INPUT);
}

bool MBotLineTracker::left() {
    ensureInit();
    return digitalRead(LINE_LEFT_PIN) == HIGH;
}

bool MBotLineTracker::right() {
    ensureInit();
    return digitalRead(LINE_RIGHT_PIN) == HIGH;
}
