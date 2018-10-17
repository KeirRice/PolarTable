/*************************************************************
  Manage the encoders.
*************************************************************/

#ifdef ENABLE_ENCODER

/*************************************************************
  State variables.
*************************************************************/

// bit 0 == current CHANNEL_A
// bit 1 == current CHANNEL_B
// bit 3 == prev CHANNEL_A
// bit 4 == prev CHANNEL_B
static byte relativeEncoderState;
static long relative_steps = 0;
static char relative_direction = 1;

/*************************************************************
  Encoder lookup tables.
*************************************************************/

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

// Check if we have an interrupt from the relative encoders
void UpdateRelativePosition(const unsigned int intStatus)
{
  // if (intStatus & RELATIVE_PIN_MASK)
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
      relative_direction = lookup;
      relative_steps += lookup;
    }
  }
}

void relativeISR(){
  // TODO: Get the pin register here...
}


/*************************************************************
  Setup and main loop.
*************************************************************/

void encoder_relative_setup()
{
  // Use io.pinMode(<pin>, <mode>) to set our relative encoder switches
  pinMode(PIN_E_SWITCH, INPUT_PULLUP);
  pinMode(PIN_F_SWITCH, INPUT_PULLUP);

  attachInterrupt(digitalPinToInterrupt(PIN_INTERUPT), relativeISR, CHANGE);

  // Read the encoder state from the SX1509 and pack it into a byte
  // On init current and previous can be the same.
  relativeEncoderState = io.digitalRead(PIN_E_SWITCH) | (io.digitalRead(PIN_F_SWITCH) << 1);
  relativeEncoderState = (relativeEncoderState << 2) | relativeEncoderState;

  // Attach an Arduino interrupt to the interrupt pin. Call
  // the encodeInterupt function, whenever the pin goes from HIGH to
  // LOW.
  
}

void encoder_relative_loop() {
  // UpdateRelativePosition(intStatus);
}

#else
void encoder_relative_setup() {}
void encoder_relative_loop() {}
#endif // ENABLE_ENCODER
