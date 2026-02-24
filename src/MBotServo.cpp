#include "MBotServo.h"
#include "PCA9685.h"
#include "bsp.h"

MBotServo::MBotServo(PCA9685* pca)
    : _pca(pca) {}

void MBotServo::setAngle(uint8_t index, uint16_t angle) {
    if (angle > SERVO_ANGLE_MAX) angle = SERVO_ANGLE_MAX;
    uint16_t pulseUs = SERVO_PULSE_MIN_US
        + (uint32_t)(SERVO_PULSE_MAX_US - SERVO_PULSE_MIN_US) * angle / SERVO_ANGLE_MAX;
    setPulse(index, pulseUs);
}

void MBotServo::setPulse(uint8_t index, uint16_t pulseUs) {
    uint8_t ch = channelFor(index);
    if (ch == 0xFF) return;
    _pca->setChannel(ch, pulseToTicks(pulseUs));
}

void MBotServo::off(uint8_t index) {
    uint8_t ch = channelFor(index);
    if (ch == 0xFF) return;
    _pca->setChannelOff(ch);
}

void MBotServo::allOff() {
    for (uint8_t i = 1; i <= SERVO_COUNT; i++) {
        off(i);
    }
}

uint8_t MBotServo::channelFor(uint8_t index) {
    if (index < 1 || index > SERVO_COUNT) return 0xFF;
    return SERVO_CHANNELS[index - 1];
}

uint16_t MBotServo::pulseToTicks(uint16_t pulseUs) {
    // ticks = pulseUs * 4096 * (prescale + 1) * 25 / 1 000 000
    // Uses runtime prescale so tick count tracks any frequency changes.
    uint32_t ticks = (uint32_t)pulseUs * 4096UL
                     * ((uint32_t)_pca->prescale() + 1) * 25UL / 1000000UL;
    if (ticks > PCA9685_PWM_MAX) ticks = PCA9685_PWM_MAX;
    return (uint16_t)ticks;
}
