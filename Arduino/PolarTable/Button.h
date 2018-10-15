/*************************************************************
  Manage the wake/sleep button.
*************************************************************/

#include <Bounce2.h>

// Instantiate a Bounce object
Bounce bounce = Bounce();

/*************************************************************
  State
*************************************************************/

void on_system_on_enter();
void on_system_on_state();
void on_system_shutdown_enter();
void off_system_shutdown_state();
void on_system_off_enter();
void off_system_off_state();

void on_trans_system_on_system_shutdown();

State state_system_on(&on_system_on_enter, &on_system_on_state, NULL);
State state_system_shutdown(&on_system_shutdown_enter, &off_system_shutdown_state, NULL);
State state_system_off(&on_system_off_enter, &off_system_off_state, NULL);

Fsm fsm_system(&state_system_on);


/*************************************************************
  Interface
*************************************************************/

void request_sleep() {
  fsm_system.trigger(SLEEP_REQUEST);
}

/*************************************************************
  Functions
*************************************************************/

void wake() {
  // Restart out millis counter as it wasn't running while alseep
  resetMillis();
}

void sleep() {
  DEBUG_PRINTLN("going to sleep");
  // TODO: Shutdown the SX1509 and the motor drivers and the LEDs
  sleepNow();     // sleep function called here
  wake(); // Called after we are woken
}


void on_system_on_enter(){
  // evtManager.trigger(Event("button.led", &BUTTON_PULSE_ON));
  LEDListener().trigger((void*) &BUTTON_PULSE_ON);
}
void on_system_on_state(){
  if (bounce.rose()) { // OnRelease
    fsm_system.trigger(SLEEP_REQUEST);
  }
}
void on_system_shutdown_enter(){
  // evtManager.trigger(Event("button.led", &BUTTON_OFF));
  
  // TODO: Send shutdown command to PI
}
void off_system_shutdown_state(){
  // TODO: Check if Pi is down?
}
void on_system_off_enter(){
  // evtManager.trigger(Event("button.led", &BUTTON_OFF));
}
void off_system_off_state(){
  sleep();
  fsm_system.trigger(WAKE_REQUEST);
}

/*************************************************************
  Setup and main loop.
*************************************************************/

void button_setup()
{
  pinMode(PIN_WAKE_SWITCH, INPUT_PULLUP);
  char debounce_time = 5; // Milliseconds
  bounce.interval(debounce_time);
  bounce.attach(PIN_WAKE_SWITCH);

  fsm_system.add_transition(&state_system_on, &state_system_shutdown, SLEEP_REQUEST, NULL);  
  fsm_system.add_timed_transition(&state_system_shutdown, &state_system_off, 3000, NULL);
  fsm_system.add_transition(&state_system_off, &state_system_on, WAKE_REQUEST, NULL);
}

void button_loop() {
  fsm_system.run_machine();
}
