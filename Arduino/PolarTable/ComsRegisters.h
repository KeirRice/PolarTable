/*************************************************************
  I2C Communication
*************************************************************/
#pragma once

#ifdef DISABLE_COMS_REGISTERS

void i2c_peripheral_setup() {}
void i2c_peripheral_loop() {}

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


typedef struct RegMask {

  public:
    RegMask(const EventID &_event, unsigned char _offset, unsigned char _size) : event(&_event), packed_size_offset((_size << 4) | _offset) {};
    RegMask(const EventID &_event, unsigned char _packed_size_offset) : event(&_event), packed_size_offset(_packed_size_offset) {};

    long mask() const {
      return offset_bitmask(mask_size(), start_offset());
    }

    unsigned char start_offset() const {
      return packed_size_offset >> 4;
    }
    unsigned char mask_size() const {
      return packed_size_offset & 0b1111;
    }
    const EventID *event;

  private:
    // Pack the size into the high bits and offset in the low
    const unsigned char packed_size_offset;
} RegMask;


const char i2c_reg_change_events_size = 4;
const RegMask i2c_reg_change_events[i2c_reg_change_events_size] = {
  // RegMask(SYSTEM_STATE, 1),
  RegMask(LIGHTING_SET_STATE, 1, 1),
  RegMask(LIGHTING_SET_COLOUR, 2, 3),
  RegMask(LIGHTING_SET_BLEND_TIME, 5, 1),
  RegMask(MOTOR_SET, 6, 4),
};




void i2c_peripheral_loop()
{
  // Look the events we know about.
  // Check if their bits have changed, if they have fire the events.
  if (i2c_reg_changed == 0) {
    return;
  }
  for (int i = 0; i < i2c_reg_change_events_size; ++i) {
    // If we are all 0 we are done
    if (i2c_reg_changed == 0) {
      break;
    }

    // Test this bit
    RegMask reg_event = i2c_reg_change_events[i];
    long mask = reg_event.mask();
    if (i2c_reg_changed & mask) {
      // Buffer the data
      byte reg_buffer[reg_event.mask_size()];
      ATOMIC_BLOCK(ATOMIC_RESTORESTATE) {
        memcpy(&reg_buffer[0], (byte*) &reg_buffer[0], reg_event.mask_size());
        // Reset the bits
        i2c_reg_changed = i2c_reg_changed & ~mask;
      }
      // Fire the event
      evtManager.trigger(*reg_event.event, &reg_buffer);
    }
  }
}

#endif // DISABLE_COMS_REGISTERS
