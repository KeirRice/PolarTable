/*************************************************************
  Motor control
*************************************************************/
#pragma once

#ifdef DISABLE_MOTORS

void motor_settings_setup() {}

#else

/*************************************************************
  Variables
*************************************************************/

static const byte FULL_STEP = 0b00;
static const byte HALF_STEP = 0b10;
static const byte QUARTER_STEP = 0b01;
static const byte EIGHTH_STEP = 0b11;

static byte motor_settings = 0;

/*************************************************************
  Functions
*************************************************************/

void send_state() {
  // turn off the output so the pins don't light up
  // while you're shifting bits:
  digitalWrite(PIN_SHIFT_LATCH, LOW);

  // shift the bits out:
  shiftOut(PIN_SHIFT_DATA, PIN_SHIFT_CLOCK, MSBFIRST, motor_settings);

  // turn on the output so the LEDs can light up:
  digitalWrite(PIN_SHIFT_LATCH, HIGH);
}

void motor_sleep(boolean do_sleep)
{
  // SLEEP active on low
  bitWrite(motor_settings, PIN_MOTOR_SLEEPX, do_sleep ? 0 : 1);
  bitWrite(motor_settings, PIN_MOTOR_SLEEPY, do_sleep ? 0 : 1);

  send_state();
}

void motor_enable(boolean enable)
{
  // ENABLE active on low
  bitWrite(motor_settings, PIN_MOTOR_ENABLEX, enable ? 0 : 1);
  bitWrite(motor_settings, PIN_MOTOR_ENABLEY, enable ? 0 : 1);

  send_state();
}

void motor_wake_and_enable() {
  /* Optimised version below
 
    // SLEEP active on low
    bitWrite(motor_settings, PIN_MOTOR_SLEEPX, 1);
    bitWrite(motor_settings, PIN_MOTOR_SLEEPY, 1);
    // ENABLE active on low
    bitWrite(motor_settings, PIN_MOTOR_ENABLEX, 0);
    bitWrite(motor_settings, PIN_MOTOR_ENABLEY, 0);
  */
  motor_settings = (motor_settings & 0b00001111) | 0b10100000;
  send_state();
}

/*************************************************************
  Setup and Main loop
*************************************************************/

void motor_settings_setup() {
  /* We use a shift register for the motor settings. */
  // Set pins to output because they are addressed in the main loop
  pinMode(PIN_SHIFT_LATCH, OUTPUT);
  pinMode(PIN_SHIFT_DATA, OUTPUT);
  pinMode(PIN_SHIFT_CLOCK, OUTPUT);

  // Full steps
  bitsWrite(motor_settings, PIN_MOTOR_XMS1, FULL_STEP, 2);
  bitsWrite(motor_settings, PIN_MOTOR_YMS1, FULL_STEP, 2);

  // Enable holding power (Enable is active low)
  bitWrite(motor_settings, PIN_MOTOR_ENABLEX, 1);
  bitWrite(motor_settings, PIN_MOTOR_ENABLEY, 1);

  // Turn off sleep (Sleep is active low)
  bitWrite(motor_settings, PIN_MOTOR_SLEEPX, 1);
  bitWrite(motor_settings, PIN_MOTOR_SLEEPY, 1);
}

#endif // DISABLE_MOTORS
