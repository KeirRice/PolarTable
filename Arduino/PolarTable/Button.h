/*************************************************************
  Manage the wake/sleep button.
*************************************************************/
#pragma once

#ifdef DISABLE_BUTTON

void button_setup() {}
void button_loop() {}

#else

#include <Fsm.h>
#include <Bounce2.h>
#include "Event.h"
#include "ProjectEvents.h"

extern EventManager evtManager;

// Instantiate a Bounce object
Bounce bounce = Bounce();

bool just_woke = false;

/*************************************************************
  State
*************************************************************/

void system_on_enter();
void system_on_state();
void system_shutdown_enter();
void system_shutdown_state();
void system_off_enter();
void system_off_state();
void system_waking_enter();

void on_trans_system_on_system_shutdown();

State state_system_on(&system_on_enter, &system_on_state, NULL);
State state_system_shutdown(&system_shutdown_enter, &system_shutdown_state, NULL);
State state_system_off(&system_off_enter, &system_off_state, NULL);
State state_system_wake(&system_waking_enter, NULL, NULL);

Fsm fsm_system(&state_system_on);


/*************************************************************
  Interface
*************************************************************/

void request_sleep() {
  fsm_system.trigger(SYSTEM_SLEEP);
}

/*************************************************************
  Functions
*************************************************************/

/* WAKING */
void system_waking_enter(){
  just_woke = true;
}

/* ON */
void system_on_enter(){
  evtManager.trigger(BUTTON_LED, BUTTON_PULSE_ON);
}

void system_on_state(){
  bounce.update();
  if (bounce.rose()) { // OnRelease
    // We need to ignore the first release after we wake as it is part of
    // the same press-release pair that woke us up.
    if(just_woke){
      just_woke = false;
    }
    else {
      fsm_system.trigger(SYSTEM_SLEEP);
    }
  }
}

/* SHUTDOWN */
void system_shutdown_enter(){
  evtManager.trigger(BUTTON_LED, BUTTON_PULSE);
  // TODO: Send shutdown command to PI
  delay(3000);
}
void system_shutdown_state(){
  // TODO: Check if Pi is down?
}

/* OFF */
void system_off_enter(){
  evtManager.trigger(BUTTON_LED, BUTTON_OFF);
  delay(5);
}

void system_off_state(){
  DEBUG_PRINTLN("Sleeping.");
  sleepNow();
  DEBUG_PRINTLN("Waking.");
  fsm_system.trigger(SYSTEM_WAKE);
}

/*************************************************************
  Setup and main loop.
*************************************************************/

void button_setup()
{
  pinMode(PIN_INTERUPT, INPUT);
  pinMode(PIN_WAKE_SWITCH, INPUT_PULLUP);
    
  char debounce_time = 5; // Milliseconds
  bounce.interval(debounce_time);
  bounce.attach(PIN_WAKE_SWITCH);

  fsm_system.add_transition(&state_system_on, &state_system_shutdown, SYSTEM_SLEEP, NULL);  
  fsm_system.add_timed_transition(&state_system_shutdown, &state_system_off, 3000, NULL);
  
  fsm_system.add_transition(&state_system_off, &state_system_wake, SYSTEM_WAKE, NULL);
  fsm_system.add_timed_transition(&state_system_wake, &state_system_on, 100, NULL);

  // Create the bridge from the event system to the system fsm.
  struct FsmEventDriver system_event_listner = FsmEventDriver(&fsm_system);
  evtManager.subscribe(Subscriber(SYSTEM_EVENT, &system_event_listner));
}

void button_loop() {
  fsm_system.run_machine();
}

#endif // DISABLE_BUTTON
