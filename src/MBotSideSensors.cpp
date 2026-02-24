#include "MBotSideSensors.h"
#include "bsp.h"

MBotSideSensors::MBotSideSensors()
    : _initialized(false) {}

void MBotSideSensors::ensureInit() {
    if (_initialized) return;
    _initialized = true;
    pinMode(SIDE_LEFT_PIN, INPUT);
    pinMode(SIDE_RIGHT_PIN, INPUT);
}

bool MBotSideSensors::leftRaw() {
    ensureInit();
    return digitalRead(SIDE_LEFT_PIN) == HIGH;
}

bool MBotSideSensors::rightRaw() {
    ensureInit();
    return digitalRead(SIDE_RIGHT_PIN) == HIGH;
}

bool MBotSideSensors::leftFlame() {
    ensureInit();
    return digitalRead(SIDE_LEFT_PIN) == LOW;
}

bool MBotSideSensors::rightFlame() {
    ensureInit();
    return digitalRead(SIDE_RIGHT_PIN) == LOW;
}
