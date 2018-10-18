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
#include "ProjectEvents.h"
#include "SX1509.h"

extern SX1509 io;
extern EventManager evtManager;


/*************************************************************
  State
*************************************************************/

void on_button_led_on_enter();
void off_button_led_enter();
void pulse_button_led_enter();
void pulse_button_led_on_state();

void pulse_to_on_button_led_enter();
void pulse_to_off_button_led_enter();

State state_button_led_on(&on_button_led_on_enter, NULL, NULL);
State state_button_led_off(&off_button_led_enter, NULL, NULL);
State state_button_led_pulse(&pulse_button_led_enter, NULL, NULL);

// New states so we can add timed transitions, but reuse use the pulse functions.
State state_button_led_pulse_on(&pulse_button_led_enter, NULL, NULL);
State state_button_led_pulse_off(&pulse_button_led_enter, NULL, NULL);

State state_button_led_error(NULL, NULL, NULL);
Fsm fsm_button_led(&state_button_led_off);

void on_button_led_on_enter()
{
  io.analogWrite(PIN_WAKE_LED, 255);
}
void off_button_led_enter(){
  io.analogWrite(PIN_WAKE_LED, 0);
}
void pulse_button_led_enter(){
  int low_ms = 1000;
  int high_ms = 1000;
  int rise_ms = 500;
  int fall_ms = 250;
  io.breathe(PIN_WAKE_LED, low_ms, high_ms, rise_ms, fall_ms);
}


/*************************************************************
  Event Dispatch
*************************************************************/

void button_LED_listener(void* data){
  fsm_button_led.trigger((int) data);
}

void error_LED_listener(void *data){
  byte code = (byte)data;
  while (true) 
  {
    for(unsigned int i = 0; i < (sizeof(code) * 8); ++i)
    {
      DEBUG_PRINT_VAR(i, code);
      digitalWrite(ARDUINO_D13, HIGH);
      delay((code & (1 << i)) ? 1000 : 500);
      digitalWrite(ARDUINO_D13, LOW);
      delay(500);
    }
    delay(3000);
  }
}
/*************************************************************
  Setup and main loop.
*************************************************************/

void button_led_setup()
{
  evtManager.subscribe(Subscriber(BUTTON_LED, button_LED_listener));
  evtManager.subscribe(Subscriber(ERROR_LED_SIGNGAL, error_LED_listener));
  
  // Off => On, On => OFF
  fsm_button_led.add_transition(&state_button_led_off, &state_button_led_on, BUTTON_ON, NULL);  
  fsm_button_led.add_transition(&state_button_led_on, &state_button_led_off, BUTTON_OFF, NULL);

  // Off/On => Blink, Blink => Off/On
  fsm_button_led.add_transition(&state_button_led_off, &state_button_led_pulse, BUTTON_PULSE, NULL);  
  fsm_button_led.add_transition(&state_button_led_on, &state_button_led_pulse, BUTTON_PULSE, NULL);  
  fsm_button_led.add_transition(&state_button_led_pulse, &state_button_led_off, BUTTON_OFF, NULL);  
  fsm_button_led.add_transition(&state_button_led_pulse, &state_button_led_on, BUTTON_ON, NULL);  

  fsm_button_led.add_transition(&state_button_led_off, &state_button_led_pulse_on, BUTTON_PULSE_ON, NULL);
  fsm_button_led.add_timed_transition(&state_button_led_pulse_on, &state_button_led_on, 3000, NULL);
  
  fsm_button_led.add_transition(&state_button_led_on, &state_button_led_pulse_off, BUTTON_PULSE_OFF, NULL);  
  fsm_button_led.add_timed_transition(&state_button_led_pulse_off, &state_button_led_off, 3000, NULL);
    
  // Use the internal 2MHz oscillator.
  // Set LED clock to 500kHz (2MHz / (2^(3-1)):
  io.clock(INTERNAL_CLOCK_2MHZ, 3);
  io.pinMode(PIN_WAKE_LED, ANALOG_OUTPUT); // LED needs PWM
  io.ledDriverInit(PIN_WAKE_LED);
}

void button_led_loop() {
  fsm_button_led.run_machine();
}

#endif // DISABLE_BUTTON_LED
