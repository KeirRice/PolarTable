/*************************************************************
  Calibrate the motors
*************************************************************/
#pragma once

#ifdef DISABLE_CALIBRATION

void calibration_setup() {}
void calibration_loop() {}

#else

#include "Motors.h"

void motors_calibration_enter(){
  motor_wake_and_enable();
  theta_stepper.moveTo(-THETA_MAX_TRAVEL);
  radius_stepper.moveTo(-RADIUS_MAX_TRAVEL);
}

void motors_calibration_active(){
  // Run the motors until we hit our stops
  bool still_going = false;
//  // PIN_C_SWITCH is the refrence point
//  if(PIN_C_SWITCH.digitalRead()){
//    theta_stepper.run();
//    still_going |= true;
//  }
  
  // PIN_A_SWITCH is center, PIN_B_SWITCH is outside
  if(PIN_A_SWITCH.digitalRead() && PIN_B_SWITCH.digitalRead()) {
    radius_stepper.run();
    still_going |= true;
  }

  if(!still_going){
    evtManager.trigger(CALIBRATION_STOP);
    evtManager.trigger(MOTOR_READY);
  }
}

void motors_calibration_exit(){
  theta_stepper.setCurrentPosition(0);
  radius_stepper.setCurrentPosition(0);
}

void calibration_setup() {
  // Use io.pinMode(<pin>, <mode>) to set our relative encoder switches
  PIN_A_SWITCH.pinMode(INPUT_PULLUP);
  PIN_B_SWITCH.pinMode(INPUT_PULLUP);
}

void calibration_loop() {
  // Safety checks
  if(PIN_A_SWITCH.digitalRead()){
    theta_stepper.move(-1);
    theta_stepper.run();
    evtManager.trigger(MOTOR_STOP);
  }
  if(PIN_B_SWITCH.digitalRead()){
    theta_stepper.move(1);
    theta_stepper.run();
    evtManager.trigger(MOTOR_STOP);
  }  
}

#endif // DISABLE_CALIBRATION
