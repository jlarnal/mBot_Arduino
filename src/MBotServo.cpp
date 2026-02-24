#include "MBotServo.h"
#include "PCA9685.h"
#include "bsp.h"

MBotServo::MBotServo(PCA9685* pca)
    : _pca(pca) {}

void MBotServo::setAngle(uint16_t angle) {
    if (angle > SERVO_ANGLE_MAX) angle = SERVO_ANGLE_MAX;
    uint16_t pulseUs = SERVO_PULSE_MIN_US
        + (uint32_t)(SERVO_PULSE_MAX_US - SERVO_PULSE_MIN_US) * angle / SERVO_ANGLE_MAX;
    setPulse(pulseUs);
}

void MBotServo::setPulse(uint16_t pulseUs) {
    _pca->setChannel(SERVO_CH_S1, pulseToTicks(pulseUs));
}

void MBotServo::off() {
    _pca->setChannelOff(SERVO_CH_S1);
}

uint16_t MBotServo::pulseToTicks(uint16_t pulseUs) {
    // ticks = pulseUs * 4096 * (prescale + 1) * 25 / 1 000 000
    // Uses runtime prescale so tick count tracks any frequency changes.
    uint32_t ticks = (uint32_t)pulseUs * 4096UL
                     * ((uint32_t)_pca->prescale() + 1) * 25UL / 1000000UL;
    if (ticks > PCA9685_PWM_MAX) ticks = PCA9685_PWM_MAX;
    return (uint16_t)ticks;
}
