/*************************************************************
  Manage the encoders.
*************************************************************/

extern SX1509 io; // Create an SX1509 object to be used throughout

/*************************************************************
  Pin assignmets
*************************************************************/

// Masks
static const int ABSOLUTE_PIN_MASK = (1 << PIN_G_IR) | (1 << PIN_H_IR) | (1 << PIN_I_IR) | (1 << PIN_J_IR);
static const int RELATIVE_PIN_MASK = (1 << PIN_F_SWITCH) | (1 << PIN_E_SWITCH);

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
static byte relativeEncoderState;
static long long int steps = 0;
char direction = 1;

// Encoder state packed into a byte so we can use it as an index into the direction array.
// bits 0-3 == current values
// bits 4-7 == previous values
static byte absoluteEncoderState;
static long long int position;


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
const char absolutePositionLookup[16] = {0, 15, 7, 8, 3, 12, 4, 11, 1, 14, 6, 9, 2, 13, 5, 10};

const unsigned int absoluteDirectionLookupRowMask = 15 << 4;
const unsigned int absoluteDirectionLookupColumnMask = 15;

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
void ResetRelativePosition() {
  steps = 0;
}

void UpdateAbsolutePosition(unsigned int intStatus)
{
  if (intStatus & ABSOLUTE_PIN_MASK)
  {
    absoluteEncoderState = (absoluteEncoderState << 4) |
                           (io.digitalRead(PIN_G_IR) |
                            (io.digitalRead(PIN_H_IR) << 1) |
                            (io.digitalRead(PIN_I_IR) << 2) |
                            (io.digitalRead(PIN_J_IR) << 3));

    byte row = absoluteEncoderState & absoluteDirectionLookupRowMask;
    byte column = absoluteEncoderState & absoluteDirectionLookupColumnMask;
    byte lookup = absoluteDirectionLookup[row][column];
    position = absolutePositionLookup[column];
    if (lookup == 2) {
      //error
      Serial.println("We jumped positions");
    }
    // else if (lookup == 0){
    //   // no change
    // }
    else {
      // Serial.println(p("We moved to position %d", position));
      direction = lookup;
      steps += lookup;
    }
  }
}


// Check if we have an interrupt from the relative encoders
void UpdateRelativePosition(const unsigned int intStatus)
{
  if (intStatus & RELATIVE_PIN_MASK)
  {
    relativeEncoderState = (relativeEncoderState << 2) | (io.digitalRead(PIN_E_SWITCH) | (io.digitalRead(PIN_F_SWITCH) << 1));
    char lookup = relativeDirectionLookup[(int) relativeEncoderState];
    if (lookup == 2) {
      //error
      Serial.println("Relative encode lost position");
    }
    else if (lookup == 0) {
      // no change
    }
    else {
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
  io.pinMode(PIN_E_SWITCH, INPUT_PULLUP);
  io.pinMode(PIN_F_SWITCH, INPUT_PULLUP);

  // Use io.pinMode(<pin>, <mode>) to set our absolute encoder switches
  io.pinMode(PIN_G_IR, INPUT_PULLUP);
  io.pinMode(PIN_H_IR, INPUT_PULLUP);
  io.pinMode(PIN_I_IR, INPUT_PULLUP);
  io.pinMode(PIN_J_IR, INPUT_PULLUP);

  // Use io.enableInterrupt(<pin>, <signal>) to enable an
  // interrupt on a pin. The <signal> variable can be either
  // FALLING, RISING, or CHANGE. Set it to falling, which will
  // mean the button was pressed:
  io.enableInterrupt(PIN_E_SWITCH, CHANGE);
  io.enableInterrupt(PIN_F_SWITCH, CHANGE);

  io.enableInterrupt(PIN_G_IR, CHANGE);
  io.enableInterrupt(PIN_H_IR, CHANGE);
  io.enableInterrupt(PIN_I_IR, CHANGE);
  io.enableInterrupt(PIN_J_IR, CHANGE);

  // Read the encoder state from the SX1509 and pack it into a byte
  // On init current and previous can be the same.
  relativeEncoderState = io.digitalRead(PIN_E_SWITCH) | (io.digitalRead(PIN_F_SWITCH) << 1);
  relativeEncoderState = (relativeEncoderState << 2) | relativeEncoderState;

  absoluteEncoderState =
    (io.digitalRead(PIN_G_IR) |
     (io.digitalRead(PIN_H_IR) << 1) |
     (io.digitalRead(PIN_I_IR) << 2) |
     (io.digitalRead(PIN_J_IR) << 3));
  absoluteEncoderState = (absoluteEncoderState << 4) | absoluteEncoderState;

  // Don't forget to configure your Arduino pins! Set the
  // Arduino's interrupt input to INPUT_PULLUP. The SX1509's
  // interrupt output is active-low.
  pinMode(PIN_SX1509_INT, INPUT_PULLUP);

  // Attach an Arduino interrupt to the interrupt pin. Call
  // the encodeInterupt function, whenever the pin goes from HIGH to
  // LOW.
  attachInterrupt(digitalPinToInterrupt(PIN_SX1509_INT), encoderISR, FALLING);
}


void encode_loop() {
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
