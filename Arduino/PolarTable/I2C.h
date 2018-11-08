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
static const byte reg_size = 10;
volatile uint8_t i2c_regs[reg_size];
volatile byte reg_position;

typedef struct RegMask {
  RegMask(const EventID &_event, int _start_offset, int _size) : event(&_event), start_offset(_start_offset), mask_size(_size){};

  /* Maybe cache this if we can spare the memory */
  long mask() const{
    return offset_bitmask(mask_size, start_offset);
  }
  
  const EventID *event;
  const byte start_offset;
  const byte mask_size;
} RegMask;


const byte i2c_reg_change_events_size = 4;
const RegMask i2c_reg_change_events[i2c_reg_change_events_size] = {
  // RegMask(SYSTEM_STATE, 0, 1),
  RegMask(LIGHTING_SET_STATE, 1, 1),
  RegMask(LIGHTING_SET_COLOUR, 2, 3),
  RegMask(LIGHTING_SET_BLEND_TIME, 5, 1),
  RegMask(MOTOR_SET, 6, 4),
  };
  
volatile long int i2c_reg_changed;


/*
   I2C Handelers
*/
void i2cReceiveEvent(int howMany)
{
  if (howMany < 1 || !Wire.available())
  {
    return; // Sanity-check
  }

  reg_position = Wire.read();
  --howMany;
  if (!howMany)
  {
    return; // This write was only to set the buffer for next read
  }

  while (howMany-- && Wire.available())
  {
    //Store the recieved data in the currently selected register
    i2c_regs[reg_position] = Wire.read();
    ++reg_position;
    i2c_reg_changed |= 1 << reg_position;
    reg_position = (reg_position >= reg_size) ? reg_position : 0;
  }
} //End i2cReceiveEvent()


void i2cRequestEvent()
{
  // send_size is from register position to the end of the buffer
  size_t send_size = reg_size - reg_position;

  // Take a copy of the volitile register so they don't change under us.
  byte send_buffer[send_size];
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
  // Look the events we know about.
  // Check if their bits have changed, if they have fire the events.
  if(i2c_reg_changed == 0){
      return;
  }
  for(int i=0; i < i2c_reg_change_events_size; ++i) {
    // If we are all 0 we are done
    if(i2c_reg_changed == 0){
      break;
    }

    // Test this bit
    RegMask reg_event = i2c_reg_change_events[i];
    long mask = reg_event.mask();
    if(i2c_reg_changed & mask){
      // Fire the event
      byte reg_buffer[reg_event.mask_size];
      ATOMIC_BLOCK(ATOMIC_RESTORESTATE) {
        // Buffer the data
        memcpy(&reg_buffer[0], (byte*) &reg_buffer[0], reg_event.mask_size);
        // Reset the bits
        i2c_reg_changed = i2c_reg_changed & ~mask;
      }
      evtManager.trigger(*reg_event.event, &reg_buffer);
    }
  }
}

#endif // DISABLE_I2C_COMS
