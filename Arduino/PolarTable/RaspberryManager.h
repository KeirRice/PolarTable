/*************************************************************
  Communicate with the Rasberry Pi
*************************************************************/
#pragma once

#include <Fsm.h>

/*************************************************************
  Raspberry State machine
*************************************************************/

void raspberry_heartbeat();
void raspberry_on_state();
void raspberry_shutdown_enter();
void raspberry_shutdown_exit();
void raspberry_startup_enter();

State state_raspberry_on(&raspberry_heartbeat, &raspberry_on_state, NULL);
State state_raspberry_shutdown(&raspberry_shutdown_enter, NULL, &raspberry_shutdown_exit);
State state_raspberry_off(NULL, NULL, NULL);
State state_raspberry_startup(&raspberry_startup_enter, NULL, NULL);
State state_raspberry_restart(&raspberry_shutdown_enter, NULL, &raspberry_shutdown_exit);

Fsm fsm_raspberry(&state_raspberry_on);

void send_raspberry_shutdown(){
  // TODO: Send shutdown signal to the PI
}

void set_raspberry_power(bool power_on){
  // Pull low to disconnect the power.
  digitalWrite(PIN_PI_POWER, power_on ? HIGH : LOW);  
}

void raspberry_listener(void* data){
  fsm_raspberry.trigger((int) data);
}

void raspberry_startup_enter(){
  set_raspberry_power(true);
}

void raspberry_shutdown_enter(){
  send_raspberry_shutdown();
}

void raspberry_shutdown_exit(){
  set_raspberry_power(false);
}

/*************************************************************
  Heartbeat
*************************************************************/

static unsigned long heartbeat_next_check;
void raspberry_heartbeat(){
  // TODO: Can we spare a hardware timer for this?
  heartbeat_next_check = millis() + 10000;
}

void raspberry_on_state(){
  if(millis() > heartbeat_next_check){
    evtManager.trigger(RASPBERRY_EVENT, RASPBERRY_RESTART);
    
    DEBUG_PRINTLN("Raspberry needed a restart.");
    evtManager.trigger(ERROR_EVENT, ERROR_RASPBERRY);
  }
}

/*************************************************************
  Setup and loop
*************************************************************/

void raspberry_manager_setup() {
  
  pinMode(PIN_PI_POWER, OUTPUT);
  digitalWrite(PIN_PI_POWER, HIGH);
  
  fsm_raspberry.add_transition(&state_raspberry_on, &state_raspberry_shutdown, RASPBERRY_SHUTDOWN, NULL);
  fsm_raspberry.add_timed_transition(&state_raspberry_shutdown, &state_raspberry_off, 3000, NULL);
  fsm_raspberry.add_transition(&state_raspberry_off, &state_raspberry_startup, RASPBERRY_STARTUP, NULL);
  fsm_raspberry.add_timed_transition(&state_raspberry_startup, &state_raspberry_on, 3000, NULL);
  
  // Restart state
  fsm_raspberry.add_timed_transition(&state_raspberry_restart, &state_raspberry_startup, 3000, NULL);
  
  // On & Off & Shutdown => Restart
  fsm_raspberry.add_transition(&state_raspberry_on, &state_raspberry_restart, RASPBERRY_RESTART, NULL);
  fsm_raspberry.add_transition(&state_raspberry_shutdown, &state_raspberry_restart, RASPBERRY_RESTART, NULL);
  fsm_raspberry.add_transition(&state_raspberry_off, &state_raspberry_restart, RASPBERRY_RESTART, NULL);
  
  evtManager.subscribe(Subscriber(RASPBERRY_EVENT, raspberry_listener));
  evtManager.subscribe(Subscriber(RASPBERRY_HEARTBEAT, raspberry_heartbeat));
}

void raspberry_manager_loop() {
  fsm_raspberry.run_machine();
}
