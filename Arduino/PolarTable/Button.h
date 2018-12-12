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
#include "kEvent.h"
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
void system_off_enter();
void system_off_state();
void system_waking_enter();

void on_trans_system_on_system_shutdown();

State state_system_on(&system_on_enter, &system_on_state, NULL);
State state_system_shutdown(&system_shutdown_enter, NULL, NULL);
State state_system_off(&system_off_enter, &system_off_state, NULL);
State state_system_wake(&system_waking_enter, NULL, NULL);

Fsm fsm_system(&state_system_on);

/*************************************************************
  Functions
*************************************************************/

/* WAKING */
void system_waking_enter(){
  just_woke = true;
}

/* ON */
void system_on_enter(){
  evtManager.trigger(BUTTON_PULSE_ON);
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
      fsm_system.trigger(SYSTEM_SLEEP_ACTION);
    }
  }
}

/* SHUTDOWN */
void system_shutdown_enter(){
  evtManager.trigger(BUTTON_PULSE);
  evtManager.trigger(RASPBERRY_SHUTDOWN);
  evtManager.trigger(MOTOR_SHUTDOWN);
  evtManager.trigger(LIGHTING_TURN_OFF);

  DEBUG_PRINTLN("Shutting down.");
  DEBUG_WHERE();
}

/* OFF */
void system_off_enter(){
  evtManager.trigger(BUTTON_OFF);
  delay(5);
}

void system_off_state(){
  DEBUG_PRINTLN("Sleeping.");
  sleepNow();
  DEBUG_PRINTLN("Waking.");
  DEBUG_WHERE();
  fsm_system.trigger(SYSTEM_WAKE_ACTION);
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

  fsm_system.add_transition(&state_system_on, &state_system_shutdown, SYSTEM_SLEEP_ACTION, NULL);  
  fsm_system.add_timed_transition(&state_system_shutdown, &state_system_off, 4000, NULL);
  
  fsm_system.add_transition(&state_system_off, &state_system_wake, SYSTEM_WAKE_ACTION, NULL);
  fsm_system.add_timed_transition(&state_system_wake, &state_system_on, 100, NULL);

  // Create the bridge from the event system to the system fsm.
  FsmEventDriver system_event_listner = FsmEventDriver(&fsm_system);
  evtManager.subscribe(Subscriber(SYSTEM, &system_event_listner));
}

void button_loop() {
  fsm_system.run_machine();
}

#endif // DISABLE_BUTTON

/*************************************************************
  Manage the wake/sleep button.
*************************************************************/
#pragma once

#ifdef DISABLE_BUTTON_LED

void button_led_setup() {}
void button_led_loop() {}

#else

#include <Fsm.h>
#include "kEvent.h"
#include "ProjectEvents.h"
#include <jled.h>

extern EventManager evtManager;

JLed wake_led(PIN_WAKE_LED);

#define FADE_TIME 1000

/*************************************************************
  State
*************************************************************/

void on_led_enter();
void off_led_enter();
void pulse_led_enter();
void led_update();

State state_led_on(&on_led_enter, NULL, NULL);
State state_led_off(&off_led_enter, NULL, NULL);
State state_led_pulse(&pulse_led_enter, &led_update, NULL);
State state_button_led_error(NULL, NULL, NULL);

// New states so we can add timed transitions, but reuse use the pulse functions.
State state_led_pulse_on(&pulse_led_enter, &led_update, NULL);
State state_led_pulse_off(&pulse_led_enter, &led_update, NULL);

Fsm fsm_button_led(&state_led_on);

void on_led_enter()
{
  DEBUG_WHERE();
  wake_led.On();
  wake_led.Update();
}

void off_led_enter(){
  DEBUG_WHERE();
  wake_led.Off();
  wake_led.Update();
}

void pulse_led_enter(){
  DEBUG_WHERE();
  wake_led.Breathe(2000).DelayAfter(500).Forever();
}

void led_update(){
  wake_led.Update();
}


/*************************************************************
  Helpers
*************************************************************/

void fade_in()
{
  wake_led = wake_led.FadeOn(FADE_TIME);
}

void fade_out()
{
  wake_led = wake_led.FadeOff(FADE_TIME);
}


/*************************************************************
  Setup and main loop.
*************************************************************/



void button_led_setup()
{    
  // Off => On, On => OFF
  fsm_button_led.add_transition(&state_led_off, &state_led_on, BUTTON_ON, NULL);
  fsm_button_led.add_transition(&state_led_on, &state_led_off, BUTTON_OFF, NULL);

  // Off => Pulse => On
  fsm_button_led.add_transition(&state_led_off, &state_led_pulse_on, BUTTON_PULSE_ON, NULL);
  fsm_button_led.add_timed_transition(&state_led_pulse_on, &state_led_on, FADE_TIME, NULL);
  
  // On => Pulse => Off
  fsm_button_led.add_transition(&state_led_on, &state_led_pulse_off, BUTTON_PULSE_OFF, NULL);  
  fsm_button_led.add_timed_transition(&state_led_pulse_off, &state_led_off, FADE_TIME, NULL);

  // Create the bridge from the event system to the button LED fsm
  FsmEventDriver button_LED_listener = FsmEventDriver(&fsm_button_led);
  evtManager.subscribe(Subscriber(BUTTON, &button_LED_listener));
}

void button_led_loop() {
  fsm_button_led.run_machine();
}

#endif // DISABLE_BUTTON_LED
