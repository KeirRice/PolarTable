/*************************************************************
  Manage the encoders.
*************************************************************/
#pragma once

// Debug stubs
#ifdef DISABLE_ENCODER_ABSOLUTE

void encoder_absolute_setup() {}
void encoder_absolute_loop() {}

#else

#include "SparkFunSX1509.h"
#define REG_DATA_B 0x10 // From sx1509_register.h

extern SX1509 io; // Create an SX1509 object to be used throughout

/*************************************************************
  State variables.
*************************************************************/

// Global variables:
volatile bool SX1509Interrupt = false; // Track Interrupts in ISR

static char absolute_position;

// The encoder is using 4 bit grey codes. This means we need to map from the 4bit number to 
// the actual positions in sequence around the wheel.
static const char absolute_position_table[16] = {0, 15, 7, 8, 3, 12, 4, 11, 1, 14, 6, 9, 2, 13, 5, 10};

// Masks
static const byte absolute_port_read_mask = 0b00001111; // Only keep the four lowest bits
static const byte absolute_lower_nibble_mask = 0b00001111; // Only keep the four lowest bits


/*************************************************************
  Interface
*************************************************************/

long GetAbsolutePosition() {
  return absolute_position;
}


/*************************************************************
  Encoder lookup tables.
*************************************************************/


char absoluteDirectionLookup(char previous_segment, char current_segment) {
  /* Turns out a giant set of if statements is more memory effient than a 2D lookup table.
  It's also compiled down to the same size as doing less comparisions by using the
  raw absolute_encoder_state. 
  
  I guess sometimes super dumb and simple code works best.
  */
  
  // No change
  if (previous_segment == current_segment) {
    return 0;
  }

  // Foward
  if (previous_segment == 0 && current_segment == 1)
  {
    return 1;
  }
  if (previous_segment == 1 && current_segment == 3)
  {
    return 1;
  }
  if (previous_segment == 2 && current_segment == 6)
  {
    return 1;
  }
  if (previous_segment == 3 && current_segment == 2)
  {
    return 1;
  }
  if (previous_segment == 4 && current_segment == 12)
  {
    return 1;
  }
  if (previous_segment == 5 && current_segment == 4)
  {
    return 1;
  }
  if (previous_segment == 6 && current_segment == 7)
  {
    return 1;
  }
  if (previous_segment == 7 && current_segment == 5)
  {
    return 1;
  }
  if (previous_segment == 8 && current_segment == 0)
  {
    return 1;
  }
  if (previous_segment == 9 && current_segment == 8)
  {
    return 1;
  }
  if (previous_segment == 10 && current_segment == 11)
  {
    return 1;
  }
  if (previous_segment == 11 && current_segment == 9)
  {
    return 1;
  }
  if (previous_segment == 12 && current_segment == 13)
  {
    return 1;
  }
  if (previous_segment == 13 && current_segment == 15)
  {
    return 1;
  }
  if (previous_segment == 14 && current_segment == 10)
  {
    return 1;
  }
  if (previous_segment == 15 && current_segment == 14)
  {
    return 1;
  }

  // Back
  if (previous_segment == 0 && current_segment == 8)
  {
    return -1;
  }
  if (previous_segment == 1 && current_segment == 0)
  {
    return -1;
  }
  if (previous_segment == 2 && current_segment == 3)
  {
    return -1;
  }
  if (previous_segment == 3 && current_segment == 1)
  {
    return -1;
  }
  if (previous_segment == 4 && current_segment == 5)
  {
    return -1;
  }
  if (previous_segment == 5 && current_segment == 7)
  {
    return -1;
  }
  if (previous_segment == 6 && current_segment == 2)
  {
    return -1;
  }
  if (previous_segment == 7 && current_segment == 6)
  {
    return -1;
  }
  if (previous_segment == 8 && current_segment == 9)
  {
    return -1;
  }
  if (previous_segment == 9 && current_segment == 11)
  {
    return -1;
  }
  if (previous_segment == 10 && current_segment == 14)
  {
    return -1;
  }
  if (previous_segment == 11 && current_segment == 10)
  {
    return -1;
  }
  if (previous_segment == 12 && current_segment == 4)
  {
    return -1;
  }
  if (previous_segment == 13 && current_segment == 12)
  {
    return -1;
  }
  if (previous_segment == 14 && current_segment == 15)
  {
    return -1;
  }
  if (previous_segment == 15 && current_segment == 13)
  {
    return -1;
  }
  return 2; // Error
}


/*************************************************************
  Functions
*************************************************************/

void UpdateAbsolutePosition()
{
  // Encoder state packed into a byte so we can use it as an index into the direction array.
  // bits 7-4 == previous values
  // bits 3-0 == current values
  static byte absolute_encoder_state;

  byte previous_absolute_position_index = absolute_encoder_state & absolute_lower_nibble_mask;

  // Read the input data state of Bank B
  // Mask out the data keeping only pins 11-8
  // Shift the last data we got left four bits
  // OR the values togeather
  absolute_encoder_state = (absolute_encoder_state << 4) | (io.readByte(REG_DATA_B) & absolute_port_read_mask);

  // Clear the iterrupt flag.
  // TODO: Add a function to the SX1509 library to clear without reading.
  io.interruptSource();

  byte absolute_position_index = absolute_encoder_state & absolute_lower_nibble_mask;
  absolute_position = absolute_position_table[absolute_position_index];

  //absoluteDirectionLookup
  byte absolute_direction = absoluteDirectionLookup(previous_absolute_position_index, absolute_position_index);
  switch (absolute_direction) {
    case 0:
      // No change
      break;
    case 2:
      // Error
      DEBUG_PRINTLN("Absolute position error.");
      break;
    default:
      DEBUG_PRINT_VAR("We moved to ", absolute_position);
      break;
  }
}

/*************************************************************
  Interupt
*************************************************************/

void encoderISR()
{
  // We can't do I2C communication in an Arduino ISR. The best
  // we can do is set a flag, to tell the loop() to check next
  // time through.
  SX1509Interrupt = true; // Set the SX1509Interrupts flag
}


/*************************************************************
  Setup and main loop.
*************************************************************/

void encoder_absolute_setup()
{
  // Check the pins as we are hardcoded to them in the port_read_mask and UpdateAbsolutePosition.
  assert(PIN_G_IR == SX1509_B8);
  assert(PIN_H_IR == SX1509_B9);
  assert(PIN_I_IR == SX1509_B10);
  assert(PIN_J_IR == SX1509_B11);

  // Use io.pinMode(<pin>, <mode>) to set our absolute encoder switches
  PIN_G_IR.pinMode(INPUT_PULLUP);
  PIN_H_IR.pinMode(INPUT_PULLUP);
  PIN_I_IR.pinMode(INPUT_PULLUP);
  PIN_J_IR.pinMode(INPUT_PULLUP);

  // Prime our values
  UpdateAbsolutePosition();

  // TODO: How to abstract away the interupts?

  // Interupts on
  io.enableInterrupt(PIN_G_IR, CHANGE);
  io.enableInterrupt(PIN_H_IR, CHANGE);
  io.enableInterrupt(PIN_I_IR, CHANGE);
  io.enableInterrupt(PIN_J_IR, CHANGE);

  // Don't forget to configure your Arduino pins! Set the
  // Arduino's interrupt input to INPUT_PULLUP. The SX1509's
  // interrupt output is active-low.
  pinMode(PIN_SX1509_INT, INPUT_PULLUP);

  // Attach an Arduino interrupt to the interrupt pin. Call
  // the encodeInterupt function, whenever the pin goes from HIGH to LOW.
  attachInterrupt(digitalPinToInterrupt(PIN_SX1509_INT), encoderISR, FALLING);
}

void encoder_absolute_loop() {
  if (SX1509Interrupt) // If the encoderISR was executed called
  {
    UpdateAbsolutePosition();
    SX1509Interrupt = false;
  }
}

#endif // DISABLE_ENCODER_ABSOLUTE
