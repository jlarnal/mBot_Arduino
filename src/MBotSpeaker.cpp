#include "MBotSpeaker.h"
#include "bsp.h"

MBotSpeaker::MBotSpeaker()
    : _initialized(false), _playing(false) {}

void MBotSpeaker::ensureInit() {
    if (_initialized) return;
    _initialized = true;
    pinMode(SPEAKER_PIN, OUTPUT);
    digitalWrite(SPEAKER_PIN, LOW);
}

void MBotSpeaker::tone(uint16_t freq, uint32_t durationMs) {
    ensureInit();
    if (freq == 0) { noTone(); return; }

    // Software square-wave via toggle
    unsigned long halfPeriodUs = 500000UL / freq;
    if (durationMs > 0) {
        unsigned long end = millis() + durationMs;
        while (millis() < end) {
            digitalWrite(SPEAKER_PIN, HIGH);
            delayMicroseconds(halfPeriodUs);
            digitalWrite(SPEAKER_PIN, LOW);
            delayMicroseconds(halfPeriodUs);
        }
        _playing = false;
    } else {
        // For continuous tone, just play a short burst
        // (true continuous would need timer interrupt — keep it simple for V1)
        unsigned long end = millis() + TONE_BURST_MS;
        while (millis() < end) {
            digitalWrite(SPEAKER_PIN, HIGH);
            delayMicroseconds(halfPeriodUs);
            digitalWrite(SPEAKER_PIN, LOW);
            delayMicroseconds(halfPeriodUs);
        }
        _playing = true;
    }
}

void MBotSpeaker::noTone() {
    ensureInit();
    digitalWrite(SPEAKER_PIN, LOW);
    _playing = false;
}

void MBotSpeaker::beep() {
    tone(BEEP_FREQ_HZ, BEEP_DURATION_MS);
}
