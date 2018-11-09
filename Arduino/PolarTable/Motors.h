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

static long incomming_steps[2];
volatile bool new_position_ready = true;

/*************************************************************
  State
*************************************************************/

void motors_calibration_enter();
void motors_calibration_active();
void motors_calibration_exit();
void motors_wake_enter();
void motors_active_enter();
void motors_active_state();
void motors_stop_enter();
void motors_stop_state();
void motors_idle_enter();
void motors_sleep_enter();
void motors_sleep_exit();

State state_motors_calibration(&motors_calibration_enter, &motors_calibration_active, &motors_calibration_exit);
State state_motors_wake(&motors_wake_enter, NULL, NULL);

State state_motors_active(&motors_active_enter, &motors_active_state, NULL);

State state_motors_stop(&motors_stop_enter, &motors_stop_state, NULL);
State state_motors_idle(&motors_idle_enter, NULL, NULL);
State state_motors_sleep(&motors_sleep_enter, NULL, &motors_sleep_exit);
// State state_motors_error(NULL, NULL, NULL);

Fsm fsm_motors(&state_motors_wake);

void build_transitions() {
  fsm_motors.add_transition(&state_motors_calibration, &state_motors_stop, CALIBRATION_STOP, NULL);

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

  // Shutdown from anywhere
  fsm_motors.add_transition(&state_motors_calibration, &state_motors_sleep, MOTOR_SHUTDOWN, NULL);
  fsm_motors.add_transition(&state_motors_wake, &state_motors_sleep, MOTOR_SHUTDOWN, NULL);
  fsm_motors.add_transition(&state_motors_active, &state_motors_sleep, MOTOR_SHUTDOWN, NULL);
  fsm_motors.add_transition(&state_motors_stop, &state_motors_sleep, MOTOR_SHUTDOWN, NULL);
  fsm_motors.add_transition(&state_motors_idle, &state_motors_sleep, MOTOR_SHUTDOWN, NULL);
}

void motors_wake_enter() {
  motor_wake_and_enable();
}

void motors_active_enter() {
  // If we have a new position lets move to it.
  stepper.moveTo((long*) incomming_steps);
  new_position_ready = false;
  evtManager.trigger(MOTOR_READY);
  stepper.run();
}

void motors_active_state() {
  // Pump the motor library, the stepper will move if it needs to
  stepper.run();

  // Check if we have arrived
  if (theta_stepper.distanceToGo() == 0 && radius_stepper.distanceToGo() == 0)
  {
    if (new_position_ready) {
      fsm_motors.trigger(MOTOR_MOVE);
    }
    else {
      // We are where we need to be, we can stop now.
      fsm_motors.trigger(MOTOR_STOP);
    }
  }
}

void motors_stop_enter() {
  theta_stepper.stop();
  radius_stepper.stop();
  stepper.run();
}

void motors_stop_state() {
  // TODO: Do we need to pump the steppers to stop?
  stepper.run();
}

void motors_idle_enter() {
  motor_enable(false);
}

void motors_sleep_enter() {
  motor_enable(false);
  motor_sleep(true);
}
void motors_sleep_exit() {
  motor_sleep(false);
}

struct MotorEventDriver : public FsmEventDriver
{
  Fsm *fsm;
  MotorEventDriver();
  MotorEventDriver(Fsm *statemachine) : fsm(statemachine) {}

  using EventTask::execute;

  void execute(Event *evt)
  {
    if (evt->label == MOTOR_SET) {
      if (new_position_ready) {
        DEBUG_PRINT("We haven't clears the last position yet.");
        return;
      }

      uint8_t *data = (uint8_t*)evt->extra;
      incomming_steps[0] = (data[0] << 8) | data[1];
      incomming_steps[1] = (data[2] << 8) | data[3];
      new_position_ready = true;

      fsm->trigger(MOTOR_MOVE);
    }
  }
};



/*************************************************************
  Setup and Main loop
*************************************************************/

void motor_setup() {
  struct MotorEventDriver motor_event_driver = MotorEventDriver(&fsm_motors);
  evtManager.subscribe(Subscriber(MOVEMENT, &motor_event_driver));

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
