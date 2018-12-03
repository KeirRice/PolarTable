/*************************************************************
  I2C Communication
*************************************************************/
#pragma once

#ifdef DISABLE_COMS_REGISTERS

void registers_setup() {}
void registers_loop() {}

#else

/*
   I2C Registers

   Register map:
   0x01 - System State (Sleep/Wake)
   0x02 - Lighting On/Off
   0x03 - Lighting Red
   0x04 - Lighting Green
   0x05 - Lighting Blue
   0x06 - Lighting Blend Time
   0x07 - Motor Theta Steps MSB
   0x08 - Motor Theta Steps LSB
   0x09 - Motor Radius Steps MSB
   0x0A - Motor Radius Steps LSB

   Total size: 10
*/


static byte registers[10];

void registers_setup() {}
void registers_loop() {}


#endif // DISABLE_COMS_REGISTERS
