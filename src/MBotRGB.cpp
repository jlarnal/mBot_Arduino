#include "MBotRGB.h"
#include "PCA9685.h"
#include "bsp.h"

MBotRGB::MBotRGB(PCA9685* pca)
    : _pca(pca) {}

void MBotRGB::setColor(uint8_t r, uint8_t g, uint8_t b) {
    _pca->setChannel(RGB_CH_R, colorToPWM(r));
    _pca->setChannel(RGB_CH_G, colorToPWM(g));
    _pca->setChannel(RGB_CH_B, colorToPWM(b));
}

void MBotRGB::setColor(uint8_t index, uint8_t r, uint8_t g, uint8_t b) {
    (void)index;  // only one LED set on this board
    setColor(r, g, b);
}

void MBotRGB::off() {
    setColor(0, 0, 0);
}

uint16_t MBotRGB::colorToPWM(uint8_t value) {
    return (uint16_t)value * PCA9685_PWM_MAX / RGB_VALUE_MAX;
}
