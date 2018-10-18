/*************************************************************
  Manage the relative encoder.

  This circuit expects the two sensors to have a digital pin each on the same port.
  Their outputs are also XORed and sent to an interupt pin.

  The data is collected on interupt and batched togeather until
  the main loop comes around and syncs the data.

  The idea is that the interupts should spin really fast and just grab all
  data including bounces. The loop will be slower and the data should have
  setteled a bit.  
    
*************************************************************/
#pragma once

#ifdef DISABLE_ENCODER_RELATIVE

void encoder_relative_setup() {}
void encoder_relative_loop() {}

#else

#include "Helpers.h"
#include <util/atomic.h> // this library includes the ATOMIC_BLOCK macro.

/*************************************************************
  State variables.
*************************************************************/

static const byte port_read_mask = B00000011; // ARDUINO_D9, ARDUINO_D8
static const byte lower_nibble_mask = B00001111; // Only keep the four lowest bits

static volatile byte relative_encoder_state;
static volatile signed char relative_direction_buffer = 1;
static volatile int relative_steps_buffer = 0;
static volatile boolean relative_error_flag = false;

static signed char relative_direction = 1;
static long relative_steps = 0;

/*************************************************************
  Encoder lookup tables.
*************************************************************/

static const signed char relativeDirectionLookup[16] = {
  0,  // 00 00 // no change
  -1, // 00 01
  1,  // 00 10
  2,  // 00 11 // error

  1,  // 01 00
  0,  // 01 01 // no change
  2,  // 01 10 // error
  -1, // 01 11

  -1, // 10 00
  2,  // 10 01 // error
  0,  // 10 10 // no change
  1,  // 10 11

  2,  // 11 00 // error
  1,  // 11 01
  -1,  // 11 10
  0  // 11 11 // no change
};


/*************************************************************
  Functions
*************************************************************/

void SetHomePosition() {
  // Reset current steps to 0
  ATOMIC_BLOCK(ATOMIC_RESTORESTATE) {
    relative_steps_buffer = 0;
      relative_steps = 0;
  }  
}

long GetRelativePosition() {
  // Reset current steps to 0
  return relative_steps;
}


/*************************************************************
  Interupt
*************************************************************/

void relativeISR(){
  /*
  PORTB maps to Arduino digital pins 8 to 13 The two high bits (6 & 7) map to the crystal pins and are not usable
  DDRB - The Port B Data Direction Register - read/write
  PORTB - The Port B Data Register - read/write
  PINB - The Port B Input Pins Register - read only

  PIN_E_SWITCH == ARDUINO_D8;
  PIN_F_SWITCH == ARDUINO_D9;
  */ 

  // Read directly from the PINB port
  // Mask off just pins D9 (bit 2) and D10 (bit 3)
  // Shift the read data to fill bits 1 & 2
  // Clear the last state high bits, keeping only bits 1 & 2
  // Shift the last state up to bits 3 & 4
  // OR togeather the old and new data into their correct bit locations
  relative_encoder_state = (relative_encoder_state << 2) | (PINB & port_read_mask);
  byte new_direction = relativeDirectionLookup[relative_encoder_state & lower_nibble_mask];
  if (new_direction == 2){
    // Don't do anything for errors
    relative_error_flag = true;
  }
  else {
    relative_steps += new_direction;
  }
}

/*************************************************************
  Internal calls
*************************************************************/

void sync_interupt_counts(){
  // Sync the values across the gap
  ATOMIC_BLOCK(ATOMIC_RESTORESTATE) {
    relative_steps += relative_steps_buffer;
    relative_direction = sign(relative_steps);
    relative_steps_buffer = 0;
  }
}

boolean encoder_error(boolean reset_flag=true){
  // Sync the values across the gap
  if(relative_error_flag){
    relative_error_flag = !reset_flag;
    return true;
  }
  return false;
}

/*************************************************************
  Setup and main loop.
*************************************************************/

void encoder_relative_setup()
{
  // Check the pins as relativeISR is hardcoded to them.
  assert(PIN_E_SWITCH == ARDUINO_D8);
  assert(PIN_F_SWITCH == ARDUINO_D9);
  
  // Use io.pinMode(<pin>, <mode>) to set our relative encoder switches
  pinMode(PIN_E_SWITCH, INPUT_PULLUP);
  pinMode(PIN_F_SWITCH, INPUT_PULLUP);

  // Read the encoder state to get us started. On init current and previous can be the same.
  relative_encoder_state = (io.digitalRead(PIN_F_SWITCH) << 1) | io.digitalRead(PIN_E_SWITCH);
  relative_encoder_state = (relative_encoder_state << 2) | relative_encoder_state;
  attachInterrupt(digitalPinToInterrupt(PIN_INTERUPT), relativeISR, CHANGE);
}

void encoder_relative_loop() {
  sync_interupt_counts();
  if(encoder_error()){
    evtManager.trigger(ERROR_EVENT, ERROR_RELATIVE_ENCODER);
  }
}

#endif // DISABLE_ENCODER_RELATIVE
