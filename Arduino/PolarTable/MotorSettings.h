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
  digitalWrite(PIN_MOTOR_SLEEPX, do_sleep ? 0 : 1);
  digitalWrite(PIN_MOTOR_SLEEPY, do_sleep ? 0 : 1);
}

void motor_enable(boolean enable)
{
  digitalWrite(PIN_MOTOR_ENABLEX, enable ? 0 : 1);
  digitalWrite(PIN_MOTOR_ENABLEY, enable ? 0 : 1);
}

void motor_wake_and_enable() { 
//    motor_sleep(false);
//    motor_enable(true);
  
  // Optimised version
  PORTA = (PORTA & 0b11110000) | 0b00000101;
}

/*************************************************************
  Setup and Main loop
*************************************************************/

void motor_settings_setup() {
  /* We use a shift register for the motor settings. */
  // Set pins to output because they are addressed in the main loop
  pinMode(PIN_MOTOR_SLEEPX, OUTPUT);
  pinMode(PIN_MOTOR_ENABLEX, OUTPUT);
  pinMode(PIN_MOTOR_SLEEPY, OUTPUT);
  pinMode(PIN_MOTOR_ENABLEY, OUTPUT);
  pinMode(PIN_MOTOR_XMS1, OUTPUT);
  pinMode(PIN_MOTOR_XMS2, OUTPUT);
  pinMode(PIN_MOTOR_YMS1, OUTPUT);
  pinMode(PIN_MOTOR_YMS2, OUTPUT); 

//  // Full steps
//  PIN_MOTOR_XMS1.digitalWrite(FULL_STEP & 0x01);
//  PIN_MOTOR_XMS2.digitalWrite(FULL_STEP >> 1);
//  PIN_MOTOR_YMS1.digitalWrite(FULL_STEP & 0x01);
//  PIN_MOTOR_YMS2.digitalWrite(FULL_STEP >> 1);
//  motor_wake_and_enable();

  // Optimised version
  // Wake, enable, and set full steps
  PORTA = 0b00000101; // ;
}

#endif // DISABLE_MOTORS
