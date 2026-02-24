#include "MBotMotors.h"
#include "PCA9685.h"
#include "bsp.h"

MBotMotors::MBotMotors(PCA9685* pca)
    : _pca(pca) {}

void MBotMotors::forward(uint8_t speed) {
    set(speed, speed);
}

void MBotMotors::backward(uint8_t speed) {
    set(-speed, -speed);
}

void MBotMotors::left(uint8_t speed) {
    set(-speed, speed);
}

void MBotMotors::right(uint8_t speed) {
    set(speed, -speed);
}

void MBotMotors::stop() {
    set(0, 0);
}

void MBotMotors::setLeft(int16_t speed) {
    setMotor(MOTOR_LEFT_FWD_CH, MOTOR_LEFT_REV_CH, speed);
}

void MBotMotors::setRight(int16_t speed) {
    setMotor(MOTOR_RIGHT_FWD_CH, MOTOR_RIGHT_REV_CH, speed);
}

void MBotMotors::set(int16_t left, int16_t right) {
    setLeft(left);
    setRight(right);
}

void MBotMotors::setMotor(uint8_t fwdCh, uint8_t revCh, int16_t speed) {
    speed = constrain(speed, -MOTOR_SPEED_MAX, MOTOR_SPEED_MAX);
    if (speed >= 0) {
        _pca->setChannel(fwdCh, speedToPWM(speed));
        _pca->setChannelOff(revCh);
    } else {
        _pca->setChannelOff(fwdCh);
        _pca->setChannel(revCh, speedToPWM(-speed));
    }
}

uint16_t MBotMotors::speedToPWM(uint8_t speed) {
    // Map 0-255 to 0-4095
    return (uint16_t)speed * PCA9685_PWM_MAX / MOTOR_SPEED_MAX;
}
