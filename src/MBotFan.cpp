#include "MBotFan.h"
#include "PCA9685.h"
#include "bsp.h"

MBotFan::MBotFan(PCA9685* pca)
    : _pca(pca) {}

void MBotFan::forward(uint16_t speed) {
    if (speed > FAN_SPEED_MAX) speed = FAN_SPEED_MAX;
    _pca->setChannel(FAN_FWD_CH, speed);
    _pca->setChannelOff(FAN_REV_CH);
}

void MBotFan::reverse(uint16_t speed) {
    if (speed > FAN_SPEED_MAX) speed = FAN_SPEED_MAX;
    _pca->setChannelOff(FAN_FWD_CH);
    _pca->setChannel(FAN_REV_CH, speed);
}

void MBotFan::set(int16_t speed) {
    if (speed > 0) {
        forward(speed);
    } else if (speed < 0) {
        reverse(-speed);
    } else {
        stop();
    }
}

void MBotFan::stop() {
    _pca->setChannelOff(FAN_FWD_CH);
    _pca->setChannelOff(FAN_REV_CH);
}
