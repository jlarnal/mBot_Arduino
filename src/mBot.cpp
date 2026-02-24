#include "mBot.h"
#include "bsp.h"

MBot::MBot()
    : _pca(PCA9685_I2C_ADDR),
      motors(&_pca),
      rgb(&_pca),
      servo(&_pca) {
    cli.attach(this);
}
