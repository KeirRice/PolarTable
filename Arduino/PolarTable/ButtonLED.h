/*************************************************************
  Manage the wake/sleep button.
*************************************************************/

extern SX1509 io;

/*************************************************************
  State
*************************************************************/

const State LED_STATE_OFF = 1;
const State LED_STATE_ON = 2;
const State LED_STATE_BLINK = 3;

static const State LED_STATE_TO_OFF = 11;
static const State LED_STATE_TO_ON = 12;
static const State LED_STATE_TO_BLINK = 13;

static State current_led_state = LED_STATE_OFF;
static State post_transition_state = 0;

static unsigned long delay_timer = 0;
static unsigned long transition_time = 0;


/*************************************************************
  Interface
*************************************************************/

void request_led_on(State transition = 0, int transition_time = 0) {
  current_led_state = LED_STATE_TO_ON;
  post_transition_state = transition;
  transition_time = millis() + transition_time;
}

void request_led_off(State transition = 0, int transition_time = 0) {
  current_led_state = LED_STATE_TO_OFF;
  post_transition_state = transition;
  transition_time = millis() + transition_time;
}

void request_led_blink(State transition = 0, int transition_time = 0) {
  current_led_state = LED_STATE_TO_BLINK;
  post_transition_state = transition;
  transition_time = millis() + transition_time;
}

/*************************************************************
  Setup and main loop.
*************************************************************/

void button_led_setup()
{
  // Use the internal 2MHz oscillator.
  // Set LED clock to 500kHz (2MHz / (2^(3-1)):
  io.clock(INTERNAL_CLOCK_2MHZ, 3);
  io.pinMode(PIN_WAKE_LED, ANALOG_OUTPUT); // LED needs PWM
  io.ledDriverInit(PIN_WAKE_LED);
}

void button_led_loop() {
  DEBUG_PRINTLN("switching led state");

  if (post_transition_state) {
    if (transition_time > millis()) {
      current_led_state = post_transition_state;
      post_transition_state = 0;
    }
  }

  switch (current_led_state) {

    case LED_STATE_TO_OFF :
      if (delay_timer == 0 or delay_timer > millis())
      {
        io.breathe(PIN_WAKE_LED, 1000, 0, 1000, 250);
        io.digitalWrite(PIN_WAKE_LED, LOW);
        current_led_state = LED_STATE_OFF;
      }
      break;

    case LED_STATE_OFF :
      break;

    case LED_STATE_TO_ON :
      if (delay_timer == 0 or delay_timer > millis())
      {
        io.breathe(PIN_WAKE_LED, 0, 1000, 1000, 250);
        io.digitalWrite(PIN_WAKE_LED, HIGH);
        current_led_state = LED_STATE_ON;
      }
      break;

    case LED_STATE_ON :
      break;

    case LED_STATE_TO_BLINK :
      if (delay_timer == 0 or delay_timer > millis())
      {
        io.breathe(PIN_WAKE_LED, 500, 500, 250, 250);
        current_led_state = LED_STATE_BLINK;
      }
      break;

    case LED_STATE_BLINK :
      break;

    default :
      break;
  }
}
