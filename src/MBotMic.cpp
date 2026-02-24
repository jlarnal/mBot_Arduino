#include "MBotMic.h"
#include "bsp.h"

MBotMic::MBotMic()
    : _initialized(false) {}

void MBotMic::ensureInit() {
    if (_initialized) return;
    _initialized = true;
    pinMode(MIC_EN_PIN, OUTPUT);
    digitalWrite(MIC_EN_PIN, HIGH);  // enable microphone
}

uint16_t MBotMic::level() {
    ensureInit();
    return analogRead(MIC_PIN);
}

bool MBotMic::isLoud(uint16_t threshold) {
    return level() > threshold;
}
