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
