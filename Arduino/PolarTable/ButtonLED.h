/*************************************************************
  Manage the wake/sleep button.
*************************************************************/

extern SX1509 io;


/*************************************************************
  Event
*************************************************************/

class EventManager {
  public:
  EventManager();

  void subscribe(Subscriber sub);
  void trigger(Event evt);
}


class Event {
  public:
  Event();
  
  void trigger(void *data);
  void handler(void *data=0);
  
  private:
  void *extraData = 0;
}
Event::trigger(void *data=0){
  extraData = data;
  fsm_button_led.trigger((int) lstn->extraData);
}

class LEDEvent : public Event {
  public:
  LEDEvent();
}

LEDEvent::handler(void *data){
  fsm_button_led.trigger((int) data);
}


class LEDListener : public EvtListener {
  public:
  LEDListener(EvtAction trigger);

  void setupListener();
  bool isEventTriggered();
  void trigger(void *data);
  private:
  static bool fired;
};
LEDListener::LEDListener(){}
LEDListener::LEDListener(EvtAction t)
{
  this->triggerAction = t;
}
bool LEDListener::fired = false;
void LEDListener::setupListener() {
  // Nothing to setup
}

bool LEDListener::isEventTriggered() {
  bool shouldFire = fired;
  fired = false;
  return shouldFire;
}
void LEDListener::trigger(void *data=0) {
  extraData = data;
  fired = true;
}

void led_event_handler(EvtListener *lstn) {
  fsm_button_led.trigger((int) lstn->extraData);
}


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

Fsm fsm_button_led(&state_button_led_off);

void on_button_led_on_enter()
{
  io.analogWrite(PIN_WAKE_LED, 255);
}
void off_button_led_enter(){
  io.analogWrite(PIN_WAKE_LED, 0);
}
void off_button_led_state(){
}

void pulse_button_led_enter(){
  int low_ms = 1000;
  int high_ms = 1000;
  int rise_ms = 500;
  int fall_ms = 250;
  io.breathe(PIN_WAKE_LED, low_ms, high_ms, rise_ms, fall_ms);
}

/*************************************************************
  Setup and main loop.
*************************************************************/

void button_led_setup()
{
  evtManager.addListener(new LEDListener((EvtAction)led_event_handler));
  
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
