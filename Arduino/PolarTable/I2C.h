/*************************************************************
  I2C Communication
*************************************************************/
#pragma once

#ifdef DISABLE_I2C_COMS

void i2c_setup() {}
void i2c_loop() {}

#else

#include "Wire.h"
#include "Helpers.h"
#include <util/atomic.h> // this library includes the ATOMIC_BLOCK macro.

#include "I2CController.h"
// #include "I2CPeripheral.h"

void i2c_setup()
{
  // Initialize i2c and make sure we are addressable
  Wire.begin(); // ARDUINO_I2C_ADDRESS
  Wire.setClock(10000);

  i2c_controller_setup();
  // i2c_peripheral_setup();
}

void i2c_loop()
{

  i2c_controller_loop();
  // i2c_peripheral_loop();
}

#endif // DISABLE_I2C_COMS
