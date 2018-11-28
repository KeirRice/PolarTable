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

/*************************************************************
  Functions
*************************************************************/

void motor_sleep(boolean do_sleep)
{
  // SLEEP active on low
  PIN_MOTOR_SLEEPX.digitalWrite(do_sleep ? 0 : 1);
  PIN_MOTOR_SLEEPY.digitalWrite(do_sleep ? 0 : 1);
}

void motor_enable(boolean enable)
{
  // ENABLE active on low
  PIN_MOTOR_ENABLEX.digitalWrite(enable ? 0 : 1);
  PIN_MOTOR_ENABLEY.digitalWrite(enable ? 0 : 1);
}

void motor_wake_and_enable() {
  motor_sleep(false);
  motor_enable(true);
}

/*************************************************************
  Setup and Main loop
*************************************************************/

void motor_settings_setup() {
  // Full steps
  PIN_MOTOR_XMS1.digitalWrite(FULL_STEP & 0x01);
  PIN_MOTOR_XMS2.digitalWrite(FULL_STEP >> 1);
  PIN_MOTOR_YMS1.digitalWrite(FULL_STEP & 0x01);
  PIN_MOTOR_YMS2.digitalWrite(FULL_STEP >> 1);

  motor_wake_and_enable();
}

#endif // DISABLE_MOTORS
