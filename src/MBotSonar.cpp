#include "MBotSonar.h"
#include "bsp.h"

MBotSonar::MBotSonar()
    : _initialized(false) {}

void MBotSonar::ensureInit() {
    if (_initialized) return;
    _initialized = true;
    pinMode(SONAR_TRIG_PIN, OUTPUT);
    digitalWrite(SONAR_TRIG_PIN, LOW);
    pinMode(SONAR_ECHO_PIN, INPUT);
}

float MBotSonar::readRaw() {
    ensureInit();
    uint32_t us = ping();
    if (us == 0) return -1.0f;
    return usToMetres(us);
}

float MBotSonar::read(uint8_t samples) {
    ensureInit();

    if (samples <= 1) return readRaw();
    if (samples > SONAR_MAX_SAMPLES) samples = SONAR_MAX_SAMPLES;

    float buf[SONAR_MAX_SAMPLES];
    uint8_t valid = 0;

    for (uint8_t i = 0; i < samples; i++) {
        uint32_t us = ping();
        if (us > 0) {
            buf[valid++] = usToMetres(us);
        }
        // Brief pause between pings to let echoes die out
        if (i < samples - 1) delayMicroseconds(2000);
    }

    if (valid == 0) return -1.0f;

    // Median filter: sort and pick the middle element
    sortFloats(buf, valid);
    return buf[valid / 2];
}

uint32_t MBotSonar::ping() {
    // Ensure clean LOW before trigger
    digitalWrite(SONAR_TRIG_PIN, LOW);
    delayMicroseconds(2);

    // 10 µs trigger pulse
    digitalWrite(SONAR_TRIG_PIN, HIGH);
    delayMicroseconds(10);
    digitalWrite(SONAR_TRIG_PIN, LOW);

    // Measure echo pulse width (returns 0 on timeout)
    return pulseIn(SONAR_ECHO_PIN, HIGH, SONAR_TIMEOUT_US);
}

float MBotSonar::usToMetres(uint32_t echoUs) {
    // distance_cm = echoUs / SONAR_US_PER_CM
    // distance_m  = distance_cm / 100
    return (float)echoUs / ((float)SONAR_US_PER_CM * 100.0f);
}

void MBotSonar::sortFloats(float* arr, uint8_t len) {
    // Insertion sort — optimal for small arrays (max 15 elements)
    for (uint8_t i = 1; i < len; i++) {
        float key = arr[i];
        uint8_t j = i;
        while (j > 0 && arr[j - 1] > key) {
            arr[j] = arr[j - 1];
            j--;
        }
        arr[j] = key;
    }
}
