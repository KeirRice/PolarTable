/*************************************************************
  Manage the wake/sleep button.
*************************************************************/
#pragma once

#ifdef DISABLE_BUTTON_LED

void button_led_setup() {}
void button_led_loop() {}

#else

#include <Fsm.h>
#include "Event.h"
#include "Error.h"
#include "ProjectEvents.h"

extern EventManager evtManager;


/*************************************************************
  State
*************************************************************/

void on_led_enter();
void off_led_enter();
void pulse_led_enter();
void pulse_led_state();
void pulse_led_exit();
void pulse_on_led_enter();
void pulse_off_led_enter();

State state_led_on(&on_led_enter, NULL, NULL);
State state_led_off(&off_led_enter, NULL, NULL);
State state_led_pulse(&pulse_led_enter, NULL, &pulse_led_exit);
State state_button_led_error(NULL, NULL, NULL);

// New states so we can add timed transitions, but reuse use the pulse functions.
State state_led_pulse_on(&pulse_on_led_enter, NULL, NULL);
State state_led_pulse_off(&pulse_off_led_enter, NULL, NULL);

Fsm fsm_button_led(&state_led_on);

void on_led_enter()
{
  DEBUG_WHERE();
  PIN_WAKE_LED.digitalWrite(1);
  // io.blink(PIN_WAKE_LED, 1000, 500);
}

void off_led_enter(){
  DEBUG_WHERE();
  PIN_WAKE_LED.digitalWrite(0);
}

void pulse_led_enter(){
  DEBUG_WHERE();
  PIN_WAKE_LED.blink(1000, 500);
//  int low_ms = 1000;
//  int high_ms = 1000;
//  int rise_ms = 500;
//  int fall_ms = 250;
//  io.breathe(PIN_WAKE_LED, low_ms, high_ms, rise_ms, fall_ms);
}

void pulse_led_exit(){
  PIN_WAKE_LED.setupBlink(PIN_WAKE_LED, 0, 0, 255);
}

void pulse_led_state(){}
void pulse_on_led_enter(){}
void pulse_off_led_enter(){}

/*************************************************************
  Helpers
*************************************************************/

void fade_in()
{
  PIN_WAKE_LED.breathe(
    0,  // tOn , time on ms, needs to be 0 for static mode
    1,  // tOff, time off ms, need to be != 0 for static mode
    8000, // fade in, raise time in ms
    0 // fade out fall time in ms
    );
  PIN_WAKE_LED.digitalWrite(PIN_WAKE_LED, 0); // turn led on, it will slowly fade in
}

void fade_out()
{
  PIN_WAKE_LED.breathe(
    0,  // tOn , time on ms, needs to be 0 for static mode
    1,  // tOff, time off ms, need to be != 0 for static mode
    8000, // fade out, fall time / speed ms
    0 // fade out fall time in ms
  );
  PIN_WAKE_LED.digitalWrite(1); // turn led off, it will slowly fade out
}


/*************************************************************
  Setup and main loop.
*************************************************************/

void button_led_setup()
{  
  // Use the internal 2MHz oscillator.
  // Set LED clock to 500kHz (2MHz / (2^(3-1)):
  io.clock(INTERNAL_CLOCK_2MHZ, 4);
  PIN_WAKE_LED.pinMode(OUTPUT);
  // io.ledDriverInit(PIN_WAKE_LED);
  
  // Off => On, On => OFF
  fsm_button_led.add_transition(&state_led_off, &state_led_on, BUTTON_ON, NULL); // , &fade_in
  fsm_button_led.add_transition(&state_led_on, &state_led_off, BUTTON_OFF, NULL); // , &fade_out

  // Off/On => Blink, Blink => Off/On
  fsm_button_led.add_transition(&state_led_off, &state_led_pulse, BUTTON_PULSE, NULL);  
  fsm_button_led.add_transition(&state_led_on, &state_led_pulse, BUTTON_PULSE, NULL);  
  fsm_button_led.add_transition(&state_led_pulse, &state_led_off, BUTTON_OFF, NULL);  
  fsm_button_led.add_transition(&state_led_pulse, &state_led_on, BUTTON_ON, NULL);  

  fsm_button_led.add_transition(&state_led_off, &state_led_pulse_on, BUTTON_PULSE_ON, NULL);
  fsm_button_led.add_timed_transition(&state_led_pulse_on, &state_led_on, 3000, NULL);
  
  fsm_button_led.add_transition(&state_led_on, &state_led_pulse_off, BUTTON_PULSE_OFF, NULL);  
  fsm_button_led.add_timed_transition(&state_led_pulse_off, &state_led_off, 3000, NULL);

  // Create the bridge from the event system to the button LED fsm
  struct FsmEventDriver button_LED_listener = FsmEventDriver(&fsm_button_led);
  evtManager.subscribe(Subscriber(BUTTON, &button_LED_listener));

  struct ErrorEventListener error_event_listener = ErrorEventListener();
  evtManager.subscribe(Subscriber(ERROR_LED_SIGNAL, &error_event_listener));
}

void button_led_loop() {
  fsm_button_led.run_machine();
}

#endif // DISABLE_BUTTON_LED
