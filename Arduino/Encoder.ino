/*************************************************************
Manage the encoders.
*************************************************************/

#include <Wire.h> // Include the I2C library

extern SX1509 io; // Create an SX1509 object to be used throughout

/*************************************************************
 Pin assignmets
*************************************************************/

// SX1509 Pins
static const char SX1509_CHANNEL_1_PIN = 4;
static const char SX1509_CHANNEL_2_PIN = 5;
static const char SX1509_CHANNEL_3_PIN = 6;
static const char SX1509_CHANNEL_4_PIN = 7;

static const char SX1509_CHANNEL_A_PIN = 8;
static const char SX1509_CHANNEL_B_PIN = 9;

// Masks
static const int ABSOLUTE_PIN_MASK = (1<<SX1509_CHANNEL_1_PIN) | (1<<SX1509_CHANNEL_2_PIN) | (1<<SX1509_CHANNEL_3_PIN) | (1<<SX1509_CHANNEL_4_PIN);
static const int RELATIVE_PIN_MASK = (1<<SX1509_CHANNEL_B_PIN) | (1<<SX1509_CHANNEL_A_PIN);

// Arduino pin
static const char ARDUINO_INT_PIN = 2;

/*************************************************************
State variables.
*************************************************************/

// Global variables:
volatile bool SX1509Interrupt = false; // Track Interrupts in ISR

// Encoder state packed into a byte so we can use it as an index into the direction array.
// bit 0 == current CHANNEL_A
// bit 1 == current CHANNEL_B
// bit 3 == prev CHANNEL_A
// bit 4 == prev CHANNEL_B
char relativeEncoderState;
int steps = 0;
char direction = 1;

// Encoder state packed into a byte so we can use it as an index into the direction array.
// bits 0-3 == current values
// bits 4-7 == previous values
char absoluteEncoderState;
int position;


/*************************************************************
Encoder lookup tables.
*************************************************************/

char absoluteDirectionLookup[16][16] = {2};

void setupAbsoluteDirectionLookup(){
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
const char absolutePositionLookup[16] = {0, 15, 7, 8, 3, 12, 4, 11, 1, 14, 6, 9, 2, 13, 5, 10};

const uint absoluteDirectionLookupRowMask = 15 << 4;
const uint absoluteDirectionLookupColumnMask = 15;


const char relativeDirectionLookup[16] = {
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
void ResetRelativePosition(){
  steps = 0;
}

void UpdateAbsolutePosition(unsigned int intStatus)
{
  if (intStatus & ABSOLUTE_PIN_MASK)
  {
    absoluteEncoderState = (absoluteEncoderState<<4) | 
      (io.digitalRead(SX1509_CHANNEL_1_PIN) | 
      (io.digitalRead(SX1509_CHANNEL_2_PIN)<<1) |
      (io.digitalRead(SX1509_CHANNEL_3_PIN)<<2) |
      (io.digitalRead(SX1509_CHANNEL_4_PIN)<<3));
    
    char row = (char) absoluteEncoderState & absoluteDirectionLookupRowMask;
    char column = (char) absoluteEncoderState & absoluteDirectionLookupColumnMask;
    char lookup = absoluteDirectionLookup[row][column];
    position = absolutePositionLookup[column];
    if(lookup == 2){
      //error
      Serial.println("We jumped positions");
    }
    // else if (lookup == 0){
    //   // no change
    // }
    else{
      // Serial.println(p("We moved to position %d", position));
      direction = lookup;
      steps += lookup;
    }
  }
}


// Check if we have an interrupt from the relative encoders
void UpdateRelativePosition(unsigned int intStatus)
{
    if (intStatus & RELATIVE_PIN_MASK)
    {
      relativeEncoderState = (relativeEncoderState<<2) | (io.digitalRead(SX1509_CHANNEL_A_PIN) | (io.digitalRead(SX1509_CHANNEL_B_PIN)<<1));
      char lookup = relativeDirectionLookup[(int) relativeEncoderState];
      if(lookup == 2){
        //error
        Serial.println("Relative encode lost position");
      }
      else if (lookup == 0){
        // no change
      }
      else{
        direction = lookup;
        steps += lookup;
      }
    }
}


void encoderISR()
{
  SX1509Interrupt = true; // Set the SX1509Interrupts flag
  // We can't do I2C communication in an Arduino ISR. The best
  // we can do is set a flag, to tell the loop() to check next 
  // time through.
}


/*************************************************************
Setup and main loop.
*************************************************************/

void encoder_setup()
{
  // Initalize the lookup tables.
  setupAbsoluteDirectionLookup();
  
  // Call io.begin(<address>) to initialize the SX1509. If
  // it successfully communicates, it'll return 1.
  if (!io.begin(SX1509_ADDRESS))
  {
    Serial.println("Failed to communicate.");
    while (1) ;
  }

  // The SX1509 has built-in debounce.
  char debounce_time = 4; // <time_ms> can be either 0, 1, 2, 4, 8, 16, 32, or 64 ms.
  io.debounceTime(debounce_time);
  
  // Use io.pinMode(<pin>, <mode>) to set our relative encoder switches
  io.pinMode(SX1509_CHANNEL_A_PIN, INPUT_PULLUP);
  io.pinMode(SX1509_CHANNEL_B_PIN, INPUT_PULLUP);

  // Use io.pinMode(<pin>, <mode>) to set our absolute encoder switches
  io.pinMode(SX1509_CHANNEL_1_PIN, INPUT_PULLUP);
  io.pinMode(SX1509_CHANNEL_2_PIN, INPUT_PULLUP);
  io.pinMode(SX1509_CHANNEL_3_PIN, INPUT_PULLUP);
  io.pinMode(SX1509_CHANNEL_4_PIN, INPUT_PULLUP);
  
  // Use io.enableInterrupt(<pin>, <signal>) to enable an
  // interrupt on a pin. The <signal> variable can be either 
  // FALLING, RISING, or CHANGE. Set it to falling, which will
  // mean the button was pressed:
  io.enableInterrupt(SX1509_CHANNEL_A_PIN, CHANGE);
  io.enableInterrupt(SX1509_CHANNEL_B_PIN, CHANGE);
  
  io.enableInterrupt(SX1509_CHANNEL_1_PIN, CHANGE);
  io.enableInterrupt(SX1509_CHANNEL_2_PIN, CHANGE);
  io.enableInterrupt(SX1509_CHANNEL_3_PIN, CHANGE);
  io.enableInterrupt(SX1509_CHANNEL_4_PIN, CHANGE);
  
  // Read the encoder state from the SX1509 and pack it into a byte
  // On init current and previous can be the same.
  relativeEncoderState = io.digitalRead(SX1509_CHANNEL_A_PIN) | (io.digitalRead(SX1509_CHANNEL_B_PIN) << 1);
  relativeEncoderState = (relativeEncoderState << 2) | relativeEncoderState;
  
  absoluteEncoderState = 
    (io.digitalRead(SX1509_CHANNEL_1_PIN) | 
    (io.digitalRead(SX1509_CHANNEL_2_PIN)<<1) |
    (io.digitalRead(SX1509_CHANNEL_3_PIN)<<2) |
    (io.digitalRead(SX1509_CHANNEL_4_PIN)<<3));
  absoluteEncoderState = (absoluteEncoderState<<4) | absoluteEncoderState;
  
  
  // Don't forget to configure your Arduino pins! Set the
  // Arduino's interrupt input to INPUT_PULLUP. The SX1509's
  // interrupt output is active-low.
  pinMode(ARDUINO_INT_PIN, INPUT_PULLUP);
  
  // Attach an Arduino interrupt to the interrupt pin. Call 
  // the encodeInterupt function, whenever the pin goes from HIGH to 
  // LOW.
  attachInterrupt(digitalPinToInterrupt(ARDUINO_INT_PIN), encoderISR, FALLING);
}


void encode_loop(){
  if (SX1509Interrupt) // If the encoderISR was executed called
  {
    // read io.interruptSource() find out which pin generated
	  // an interrupt and clear the SX1509's interrupt output.
    unsigned int intStatus = io.interruptSource();
    UpdateRelativePosition(intStatus);
    UpdateAbsolutePosition(intStatus);
    SX1509Interrupt = false; // Clear the interupt flag
  }
}