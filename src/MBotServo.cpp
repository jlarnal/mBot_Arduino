#include "MBotServo.h"
#include "PCA9685.h"
#include "bsp.h"

MBotServo::MBotServo(PCA9685 *pca)
    : _pca(pca), _freqSet(false) {}

void MBotServo::ensureServoFreq()
{
    /*
        if (_freqSet)
            return;
        _freqSet = true;

        // Turn off RGB channels to avoid flash during frequency change
        _pca->setChannelOff(RGB_CH_R);
        _pca->setChannelOff(RGB_CH_G);
        _pca->setChannelOff(RGB_CH_B);

        // Switch to 50Hz (manual spec for servo operation)
        _pca->setFreqHz(50);
        */
}

void MBotServo::setPulse_us(uint16_t pulseUs)
{
    ensureServoFreq();
    _pca->setChannel(SERVO_CH_S1, usToTicks(pulseUs));
}

void MBotServo::off()
{
    _pca->setChannelOff(SERVO_CH_S1);
}

uint16_t MBotServo::getPulseTicks()
{
    return _pca->getChannelOff(SERVO_CH_S1);
}

uint16_t MBotServo::getPulse_us()
{
    // tickDuration = (prescale + 1) / 25 MHz  →  us = ticks * (prescale + 1) / 25
    uint32_t ticks = getPulseTicks();
    return (uint16_t)(ticks * ((uint32_t)_pca->prescale() + 1) / 25UL);
}

void MBotServo::setAngle(int8_t angle)
{
    ensureServoFreq();
    // Cap the angle to avoid mechanical interference of the sonar sensor.  The mBot's default servo horn provides about ±45° of safe travel.
    if (angle < -45)
        angle = -45;
    else if (angle > 45)
        angle = 45;
    _pca->setChannel(SERVO_CH_S1,
                     usToTicks(map(angle,
                                   -45 - SERVO_TRIM_DEG, 45 - SERVO_TRIM_DEG,
                                   1000, 2000)));
}

int8_t MBotServo::getAngle()
{
    int result = map(getPulse_us(), 1000, 2000,
                     -45 - SERVO_TRIM_DEG, 45 - SERVO_TRIM_DEG);
    if (result < INT8_MIN)
        return INT8_MIN;
    if (result > INT8_MAX)
        return INT8_MAX;
    return (int8_t)result;
}

uint16_t MBotServo::usToTicks(uint16_t pulseUs)
{
    // ticks = pulseUs / tickDuration  where tickDuration = (prescale+1) / 25 MHz
    //       = pulseUs * 25 / (prescale + 1)
    uint32_t ticks = (uint32_t)pulseUs * 25UL / ((uint32_t)_pca->prescale() + 1);
    if (ticks > PCA9685_PWM_MAX)
        ticks = PCA9685_PWM_MAX;
    return (uint16_t)ticks;
}
