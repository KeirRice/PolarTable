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

/*
   I2C Registers

   Register map:
   0x00 - Status
   0x01 - System State (Sleep/Wake)
   0x02 - Lighting On/Off
   0x03 - Lighting Red
   0x04 - Lighting Green
   0x05 - Lighting Blue
   0x06 - Lighting Blend Time
   0x07 - Motor Theta Direction
   0x08 - Motor Theta Steps
   0x09 - Motor Radius Direction
   0x0A - Motor Radius Steps

   Total size: 11
*/
const byte reg_size = 11;
volatile uint8_t i2c_regs[reg_size];
volatile byte reg_position;

/*
   I2C Handelers
*/
void i2cReceiveEvent(int howMany)
{
  if (howMany < 1 || !Wire.available())
  {
    return;// Sanity-check
  }

  reg_position = Wire.read();
  howMany--;
  if (!howMany)
  {
    return; // This write was only to set the buffer for next read
  }

  while (howMany-- && Wire.available())
  {
    //Store the recieved data in the currently selected register
    i2c_regs[reg_position] = Wire.read();

    //Proceed to the next register
    reg_position++;
    if (reg_position >= reg_size)
    {
      reg_position = 0;
    }
  }
  // led_needs_update = true;

} //End i2cReceiveEvent()


void i2cRequestEvent()
{
  // send_size is from register position to the end of the buffer
  size_t send_size = reg_size - reg_position;

  // Take a copy of the volitile register so they don't change under us.
  uint8_t send_buffer[send_size];
  ATOMIC_BLOCK(ATOMIC_RESTORESTATE) {
    memcpy((void*) &send_buffer, (void*) &i2c_regs[reg_position], send_size);
  }

  // Ready to send
  Wire.write(send_buffer, send_size);
  reg_position = 0;
} //End i2cRequestEvent


/*
   Initialize instances/classes
*/

void i2c_setup()
{
  // Initialize i2c and make sure we are addressable
  Wire.begin(ARDUINO_I2C_ADDRESS);

  // Define callbacks for i2c peripheral mode communication
  Wire.onReceive(i2cReceiveEvent);
  Wire.onRequest(i2cRequestEvent);
}

void i2c_loop()
{
}

#endif // DISABLE_I2C_COMS
