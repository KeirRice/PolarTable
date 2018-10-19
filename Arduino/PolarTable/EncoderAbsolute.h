/*************************************************************
  Manage the encoders.
*************************************************************/
#pragma once

// Debug stubs
#ifdef DISABLE_ENCODER_ABSOLUTE

void encoder_setup() {}
void encode_loop() {}

#else

#include "SX1509.h"
#define REG_DATA_B 0x10 // From sx1509_register.h

extern SX1509 io; // Create an SX1509 object to be used throughout

/*************************************************************
  State variables.
*************************************************************/

// Global variables:
volatile bool SX1509Interrupt = false; // Track Interrupts in ISR

// Masks
static const byte absolute_port_read_mask = B00001111; // Only keep the four lowest bits
static const byte absolute_lower_nibble_mask = B00001111; // Only keep the four lowest bits

const unsigned char absolute_position_mask = B00001111;
const unsigned char previous_absolute_position_mask = B11110000;

const char absolute_position_table[16] = {0, 15, 7, 8, 3, 12, 4, 11, 1, 14, 6, 9, 2, 13, 5, 10};

// Encoder state packed into a byte so we can use it as an index into the direction array.
// bits 7-4 == previous values
// bits 3-0 == current values

static byte absolute_encoder_state;
static char absolute_position;


/*************************************************************
  Interface
*************************************************************/

long GetAbsolutePosition() {
  return absolute_position;
}


/*************************************************************
  Encoder lookup tables.
*************************************************************/

char absoluteDirectionLookup[16][16] = {2};

void setupAbsoluteDirectionLookup() {
  // TODO: Make this [15][2] and derive the index from row < column (beware wrapping)

  absoluteDirectionLookup[0][0] = 0;
  absoluteDirectionLookup[0][1] = 1;
  absoluteDirectionLookup[0][8] = -1;

  absoluteDirectionLookup[1][0] = -1;
  absoluteDirectionLookup[1][1] = 0;
  absoluteDirectionLookup[1][3] = 1;

  absoluteDirectionLookup[2][2] = 0;
  absoluteDirectionLookup[2][3] = -1;
  absoluteDirectionLookup[2][6] = 1;

  absoluteDirectionLookup[3][1] = -1;
  absoluteDirectionLookup[3][2] = 1;
  absoluteDirectionLookup[3][3] = 0;

  absoluteDirectionLookup[4][4] = 0;
  absoluteDirectionLookup[4][5] = -1;
  absoluteDirectionLookup[4][12] = 1;

  absoluteDirectionLookup[5][4] = 1;
  absoluteDirectionLookup[5][5] = 0;
  absoluteDirectionLookup[5][7] = -1;

  absoluteDirectionLookup[6][2] = -1;
  absoluteDirectionLookup[6][6] = 0;
  absoluteDirectionLookup[6][7] = 1;

  absoluteDirectionLookup[7][5] = 1;
  absoluteDirectionLookup[7][6] = -1;
  absoluteDirectionLookup[7][7] = 0;

  absoluteDirectionLookup[8][0] = 1;
  absoluteDirectionLookup[8][8] = 0;
  absoluteDirectionLookup[8][9] = -1;

  absoluteDirectionLookup[9][8] = 1;
  absoluteDirectionLookup[9][9] = 0;
  absoluteDirectionLookup[9][11] = -1;

  absoluteDirectionLookup[10][10] = 0;
  absoluteDirectionLookup[10][11] = 1;
  absoluteDirectionLookup[10][14] = -1;

  absoluteDirectionLookup[11][9] = 1;
  absoluteDirectionLookup[11][10] = -1;
  absoluteDirectionLookup[11][11] = 0;

  absoluteDirectionLookup[12][4] = -1;
  absoluteDirectionLookup[12][12] = 0;
  absoluteDirectionLookup[12][13] = 1;

  absoluteDirectionLookup[13][12] = -1;
  absoluteDirectionLookup[13][13] = 0;
  absoluteDirectionLookup[13][15] = 1;

  absoluteDirectionLookup[14][10] = 1;
  absoluteDirectionLookup[14][14] = 0;
  absoluteDirectionLookup[14][15] = -1;

  absoluteDirectionLookup[15][13] = -1;
  absoluteDirectionLookup[15][14] = 1;
  absoluteDirectionLookup[15][15] = 0;
}


/*************************************************************
  Functions
*************************************************************/

void UpdateAbsolutePosition()
{
  // byte previous_absolute_position_index = absolute_encoder_state & absolute_lower_nibble_mask;
  
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

//  byte absolute_direction = absoluteDirectionLookup[previous_absolute_position_index][absolute_position_index];
//  switch (absolute_direction){
//    case 0:
//      // No change
//      break;
//    case 2:
//      // Error
//      DEBUG_PRINTLN("Absolute position error.");
//      break;
//    default:
//      DEBUG_PRINT_VAR("We moved to ", absolute_position));
//      break;
//  }
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
  
  // Initalize the lookup tables.
  // setupAbsoluteDirectionLookup();

  // The SX1509 has built-in debounce.
  char debounce_time = 4; // <time_ms> can be either 0, 1, 2, 4, 8, 16, 32, or 64 ms.
  io.debounceTime(debounce_time);

  // Use io.pinMode(<pin>, <mode>) to set our absolute encoder switches
  io.pinMode(PIN_G_IR, INPUT_PULLUP);
  io.pinMode(PIN_H_IR, INPUT_PULLUP);
  io.pinMode(PIN_I_IR, INPUT_PULLUP);
  io.pinMode(PIN_J_IR, INPUT_PULLUP);

  // Prime our values
  UpdateAbsolutePosition(); 

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
