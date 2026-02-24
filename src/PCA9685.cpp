#include "PCA9685.h"
#include "bsp.h"

// PCA9685 registers
static constexpr uint8_t REG_MODE1     = 0x00;
static constexpr uint8_t REG_MODE2     = 0x01;
static constexpr uint8_t REG_LED0_ON_L = 0x06;
static constexpr uint8_t REG_PRESCALE  = 0xFE;

// MODE1 bits
static constexpr uint8_t MODE1_SLEEP   = 0x10;
static constexpr uint8_t MODE1_AI      = 0x20;  // auto-increment
static constexpr uint8_t MODE1_RESTART = 0x80;

// MODE2 bits
static constexpr uint8_t MODE2_OUTDRV  = 0x04;  // totem-pole outputs

// Internal oscillator frequency (Hz)
static constexpr uint32_t OSC_CLOCK = 25000000UL;

PCA9685::PCA9685(uint8_t addr)
    : _addr(addr), _prescale(PCA9685_PRESCALE), _initialized(false) {}

void PCA9685::ensureInit() {
    if (_initialized) return;
    _initialized = true;

    Wire.begin();

    // Put to sleep so we can set prescaler
    writeReg(REG_MODE1, MODE1_SLEEP);

    // prescale = round(25 MHz / (4096 * freq)) - 1
    writeReg(REG_PRESCALE, _prescale);

    // Wake up with auto-increment enabled
    writeReg(REG_MODE1, MODE1_AI);
    delayMicroseconds(PCA9685_OSC_STARTUP_US);  // oscillator startup

    // Restart + auto-increment
    writeReg(REG_MODE1, MODE1_AI | MODE1_RESTART);

    // Totem-pole outputs
    writeReg(REG_MODE2, MODE2_OUTDRV);
}

void PCA9685::setFreqHz(uint16_t hz) {
    ensureInit();

    // prescale = round(25 MHz / (4096 * hz)) - 1
    uint8_t ps = (uint8_t)((OSC_CLOCK + (uint32_t)hz * 2048UL)
                            / ((uint32_t)hz * 4096UL) - 1);

    // Prescaler can only be written while the chip is asleep
    uint8_t mode1 = readReg(REG_MODE1);
    writeReg(REG_MODE1, (mode1 & ~MODE1_RESTART) | MODE1_SLEEP);
    writeReg(REG_PRESCALE, ps);
    writeReg(REG_MODE1, mode1 & ~MODE1_SLEEP);
    delayMicroseconds(PCA9685_OSC_STARTUP_US);
    writeReg(REG_MODE1, mode1 | MODE1_RESTART);

    _prescale = ps;
}

uint16_t PCA9685::freqHz() const {
    // freq = 25 MHz / (4096 * (prescale + 1))
    return (uint16_t)(OSC_CLOCK / (4096UL * ((uint32_t)_prescale + 1)));
}

void PCA9685::setPWM(uint8_t channel, uint16_t on, uint16_t off) {
    ensureInit();
    uint8_t base = REG_LED0_ON_L + 4 * channel;
    Wire.beginTransmission(_addr);
    Wire.write(base);
    Wire.write(on & 0xFF);
    Wire.write(on >> 8);
    Wire.write(off & 0xFF);
    Wire.write(off >> 8);
    Wire.endTransmission();
}

void PCA9685::setChannel(uint8_t channel, uint16_t value) {
    if (value >= PCA9685_PWM_MAX) {
        // Full on
        setPWM(channel, 0x1000, 0);
    } else if (value == 0) {
        setChannelOff(channel);
    } else {
        setPWM(channel, 0, value);
    }
}

void PCA9685::setChannelOff(uint8_t channel) {
    setPWM(channel, 0, 0x1000);
}

void PCA9685::writeReg(uint8_t reg, uint8_t value) {
    Wire.beginTransmission(_addr);
    Wire.write(reg);
    Wire.write(value);
    Wire.endTransmission();
}

uint8_t PCA9685::readReg(uint8_t reg) {
    Wire.beginTransmission(_addr);
    Wire.write(reg);
    Wire.endTransmission();
    Wire.requestFrom(_addr, (uint8_t)1);
    return Wire.read();
}
