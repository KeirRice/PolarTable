/*************************************************************
  Motor control
*************************************************************/
#pragma once

#ifdef DISABLE_MOTORS

void motor_setup() {}
void motor_loop() {}

#else

#include <AccelStepper.h>
#include <MultiStepper.h>
#include <Fsm.h>
#include "Event.h"
#include "ProjectEvents.h"

extern EventManager evtManager;

// Define some steppers and the pins the will use
AccelStepper theta_stepper(AccelStepper::DRIVER, PIN_MOTOR_YSTEP, PIN_MOTOR_YDIR);
AccelStepper radius_stepper(AccelStepper::DRIVER, PIN_MOTOR_XSTEP, PIN_MOTOR_XDIR);
MultiStepper stepper = MultiStepper();

AccelStepper *steppers[2] = {
  &theta_stepper,
  &radius_stepper
  };

/*************************************************************
  Variables
*************************************************************/

static const int THETA = 0;
static const int RADIUS = 1;

// static const char sleepXMask = 1 << PIN_MOTOR_SLEEPX;
// static const char enXMask = 1 << PIN_MOTOR_ENABLEX;
// static const char sleepYMask = 1 << PIN_MOTOR_SLEEPY;
// static const char enYMask = 1 << PIN_MOTOR_ENABLEY;
// static const char yMSMask = B11 << PIN_MOTOR_YMS1; // 3 == b11
// static const char xMSMask = B11 << PIN_MOTOR_XMS1; // 3 == b11

static const byte FULL_STEP = B00;
static const byte HALF_STEP = B10;
static const byte QUARTER_STEP = B01;
static const byte EIGHTH_STEP = B11;

unsigned char motor_settings = 0;

/*************************************************************
  Steps
*************************************************************/

struct Steps {
  // absolute
  MotorPosition (*steps_target)[2];
  // long steps_previous[2] = {0, 0};
  boolean theta_ready = true;
  boolean radius_ready = true;

  boolean ready_to_read();
  boolean set_steps(long int (*target)[2]);
  boolean set_theta(long target);
  boolean set_radius(long target);
  MotorPosition (*get_steps())[2];
};

boolean Steps::ready_to_read(){
  return theta_ready && radius_ready;
}

MotorPosition (*Steps::get_steps())[2]{
  theta_ready = false;
  radius_ready = false;
  // memcpy(&steps_previous, &steps_target, 2 * sizeof(long));
  return steps_target;
}

boolean Steps::set_steps(long int (*target)[2]){
  if(!ready_to_read()){
    memcpy(&steps_target, target, 2 * sizeof(long));
    theta_ready = true;
    radius_ready = true;
    return true;
  }
  return false;    
}
boolean Steps::set_theta(long target){
  if(!theta_ready){
    steps_target[THETA]->steps = target;
    theta_ready = true;
    return true;
  }
  return false;    
}
boolean Steps::set_radius(long target){
  if(!radius_ready){
    steps_target[RADIUS]->steps = target;
    radius_ready = true;
    return true;
  }
  return false;    
}

Steps motor_steps;

boolean motor_set_target_theta(long theta){
  return motor_steps.set_theta(theta);
}
boolean motor_set_target_radius(long radius){
  return motor_steps.set_radius(radius);
}
boolean motor_set_target(long theta, long radius)
{
  long blah[2] = {theta, radius};
  return motor_steps.set_steps(&blah);
}

void motor_theta_event(void *data){
  if(!motor_set_target_theta((long) data)){
    DEBUG_PRINTLN("Motor wasn't ready for data.");
    evtManager.trigger(ERROR_EVENT, ERROR_MOTOR);
  }
}
void motor_radius_event(void *data){
  if(!motor_set_target_radius((long) data)){
    DEBUG_PRINTLN("Motor wasn't ready for data.");
    evtManager.trigger(ERROR_EVENT, ERROR_MOTOR);
  }
}

/*************************************************************
  Functions
*************************************************************/

void send_state() {
  // turn off the output so the pins don't light up
  // while you're shifting bits:
  digitalWrite(PIN_SHIFT_LATCH, LOW);

  // TODO: If we need to go faster we could use the SPI hardware...
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

/*************************************************************
  State
*************************************************************/

void motors_wake_enter();
void motors_active_enter();
void motors_active_state();
void motors_stop_enter();
void motors_stop_state();
void motors_idle_enter();
void motors_sleep_enter();
void motors_sleep_exit();

State state_motors_wake(&motors_wake_enter, NULL, NULL);
State state_motors_active(&motors_active_enter, &motors_active_state, NULL);
State state_motors_stop(&motors_stop_enter, &motors_stop_state, NULL);
State state_motors_idle(&motors_idle_enter, NULL, NULL);
State state_motors_sleep(&motors_sleep_enter, NULL, &motors_sleep_exit);
// State state_motors_error(NULL, NULL, NULL);

Fsm fsm_motors(&state_motors_wake);

void build_transitions(){
  // Short timmed event to enable the wake command to propergate
  fsm_motors.add_timed_transition(&state_motors_wake, &state_motors_active, 5, NULL);

  // Active stays active if there are new positions.
  fsm_motors.add_transition(&state_motors_active, &state_motors_active, MOTOR_NEW_POSITION, NULL);
  
  // Stop the motor
  fsm_motors.add_transition(&state_motors_active, &state_motors_stop, MOTOR_STOP, NULL);
  fsm_motors.add_timed_transition(&state_motors_stop, &state_motors_idle, 10, NULL);
  fsm_motors.add_timed_transition(&state_motors_idle, &state_motors_sleep, 10000, NULL);
  
  // New position lets go
  fsm_motors.add_transition(&state_motors_stop, &state_motors_wake, MOTOR_NEW_POSITION, NULL);
  fsm_motors.add_transition(&state_motors_idle, &state_motors_wake, MOTOR_NEW_POSITION, NULL);
  fsm_motors.add_transition(&state_motors_sleep, &state_motors_wake, MOTOR_NEW_POSITION, NULL);
}

void motors_wake_enter(){
  motor_sleep(false);
  motor_enable(true);
}

void motors_active_enter(){
  if(motor_steps.ready_to_read())
  {
    // If we have a new position lets move to it.
    stepper.moveTo((long*) motor_steps.get_steps());
    
    evtManager.trigger(MOTOR_READY_FOR_DATA, (void*) NULL);
    
    stepper.run();
  }
}

void motors_active_state(){
  // Pump the motor library, the stepper will move if it needs to
  stepper.run();

  // Check if we have arrived
  if(steppers[THETA]->distanceToGo() == 0 && steppers[RADIUS]->distanceToGo() == 0)
  {    
    if(motor_steps.ready_to_read()){
      // A new position has been set while we were moving.
      fsm_motors.trigger(MOTOR_NEW_POSITION);        
    }
    else {
      // We are where we need to be, we can stop now.
      fsm_motors.trigger(MOTOR_STOP);
    }
  }
}

void motors_stop_enter(){
  steppers[THETA]->stop();
  steppers[RADIUS]->stop();
  stepper.run();
}

void motors_stop_state(){
  // TODO: Do we need to pump the steppers to stop?
  stepper.run();
}

void motors_idle_enter(){
  motor_enable(false);
}

void motors_sleep_enter(){
  motor_sleep(true);
}
void motors_sleep_exit(){
  motor_sleep(false);
}

/*************************************************************
  Setup and Main loop
*************************************************************/

void motor_setup() {
  evtManager.subscribe(Subscriber(MOTOR_TARGET_THETA, motor_theta_event));
  evtManager.subscribe(Subscriber(MOTOR_TARGET_RADIUS, motor_radius_event));
  
  build_transitions();
  
  theta_stepper.setMaxSpeed(200.0);
  theta_stepper.setMinPulseWidth(20); // Driver supports 1us
  // theta_stepper.setAcceleration(200.0);

  radius_stepper.setMaxSpeed(100.0);
  radius_stepper.setMinPulseWidth(20); // Driver supports 1us
  // radius_stepper.setAcceleration(100.0);

  stepper.addStepper(theta_stepper);
  stepper.addStepper(radius_stepper);

  /* We use a shift register for the motor settings. */
  // Set pins to output because they are addressed in the main loop
  pinMode(PIN_SHIFT_LATCH, OUTPUT);
  pinMode(PIN_SHIFT_DATA, OUTPUT);
  pinMode(PIN_SHIFT_CLOCK, OUTPUT);

  // Full steps
  bitsWrite(motor_settings, PIN_MOTOR_XMS1, FULL_STEP, 2);
  bitsWrite(motor_settings, PIN_MOTOR_YMS1, FULL_STEP, 2);

  // Enable (Sleep is active low)
  bitWrite(motor_settings, PIN_MOTOR_ENABLEX, 1);
  bitWrite(motor_settings, PIN_MOTOR_ENABLEY, 1);

  // Turn off sleep (Sleep is active low)
  bitWrite(motor_settings, PIN_MOTOR_SLEEPX, 1);
  bitWrite(motor_settings, PIN_MOTOR_SLEEPY, 1);
}

void motor_loop()
{
  fsm_motors.run_machine();
}

#endif // DISABLE_MOTORS
