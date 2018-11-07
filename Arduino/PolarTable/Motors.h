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
#include "MotorsSettings.h"

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

/*************************************************************
  Steps
*************************************************************/

struct Steps {
  // Absolute
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

struct MotorEventDriver : public EventTask
{
  EventID motor;
  MotorEventDriver();
  MotorEventDriver(EventID motor) : motor(motor) {}

  using EventTask::execute;
 
  void execute(Event *evt);
};

struct MotorThetaEventDriver : public MotorEventDriver
{
  MotorThetaEventDriver();
  MotorThetaEventDriver(EventID motor) : MotorEventDriver(motor) {}

  using MotorEventDriver::execute;
  
  void execute(Event *evt)
  {
    if(!motor_set_target_theta(*(long*)evt->extra)){
      DEBUG_PRINTLN("Motor wasn't ready for data.");
      evtManager.trigger(ERROR_MOTOR);
    }
  }
};

struct MotorRadiusEventDriver : public MotorEventDriver
{
  MotorRadiusEventDriver();
  MotorRadiusEventDriver(EventID motor) : MotorEventDriver(motor) {}

  using MotorEventDriver::execute;
  
  void execute(Event *evt)
  {
    if(!motor_set_target_radius(*(long*)evt->extra)){
      DEBUG_PRINTLN("Motor wasn't ready for data.");
      evtManager.trigger(ERROR_MOTOR);
    }
  }
};
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
  // Short timmed event to enable the wake command to propagate
  fsm_motors.add_timed_transition(&state_motors_wake, &state_motors_active, 5, NULL);

  // Active stays active if there are new positions.
  fsm_motors.add_transition(&state_motors_active, &state_motors_active, MOTOR_MOVE, NULL);
  
  // Stop the motor
  fsm_motors.add_transition(&state_motors_active, &state_motors_stop, MOTOR_STOP, NULL);
  fsm_motors.add_timed_transition(&state_motors_stop, &state_motors_idle, 10, NULL);
  fsm_motors.add_timed_transition(&state_motors_idle, &state_motors_sleep, 10000, NULL);
  
  // New position lets go
  fsm_motors.add_transition(&state_motors_stop, &state_motors_wake, MOTOR_MOVE, NULL);
  fsm_motors.add_transition(&state_motors_idle, &state_motors_wake, MOTOR_MOVE, NULL);
  fsm_motors.add_transition(&state_motors_sleep, &state_motors_wake, MOTOR_MOVE, NULL);
}

void motors_wake_enter(){
  motor_wake_and_enable();
}

void motors_active_enter(){
  if(motor_steps.ready_to_read())
  {
    // If we have a new position lets move to it.
    stepper.moveTo((long*) motor_steps.get_steps());
    
    evtManager.trigger(MOTOR_READY, (void*) NULL);
    
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
      fsm_motors.trigger(MOTOR_MOVE);        
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
  struct MotorEventDriver theta_event_listner = MotorThetaEventDriver(MOTOR_THETA_POSITION);
  evtManager.subscribe(Subscriber(MOTOR_THETA_POSITION, &theta_event_listner));
  struct MotorEventDriver radius_event_listner = MotorRadiusEventDriver(MOTOR_RADIUS_POSITION);
  evtManager.subscribe(Subscriber(MOTOR_RADIUS_POSITION, &radius_event_listner));
  
  build_transitions();
  
  theta_stepper.setMaxSpeed(200.0);
  theta_stepper.setMinPulseWidth(20); // Driver supports 1us
  // theta_stepper.setAcceleration(200.0);

  radius_stepper.setMaxSpeed(100.0);
  radius_stepper.setMinPulseWidth(20); // Driver supports 1us
  // radius_stepper.setAcceleration(100.0);

  stepper.addStepper(theta_stepper);
  stepper.addStepper(radius_stepper);

  motor_settings_setup();
}

void motor_loop()
{
  fsm_motors.run_machine();
}

#endif // DISABLE_MOTORS
