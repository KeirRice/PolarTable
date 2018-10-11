/*************************************************************
  Manage the wake/sleep button.
*************************************************************/

#include <Bounce2.h>

// Instantiate a Bounce object
Bounce bounce = Bounce();

/*************************************************************
  State
*************************************************************/

static const State POWER_STATE_OFF = 1;
static const State POWER_STATE_SHUTDOWN = 2;
static const State POWER_STATE_ON = 3;

static const State POWER_STATE_TO_OFF = 11;
static const State POWER_STATE_TO_ON = 12;

Event button_current_event = 0;


/*************************************************************
  Interface
*************************************************************/

void request_sleep() {
  button_current_event = POWER_STATE_TO_OFF;
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

  // Shutdown the Pi
  delay(300);

  sleepNow();     // sleep function called here

  wake(); // Called after we are woken
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
}


void button_loop() {
  DEBUG_PRINTLN("button_loop");

  static State power_state = POWER_STATE_ON;
  static unsigned long piOffTimer = 0;

  bounce.update();

  DEBUG_PRINTLN("switching power state");
  switch (power_state) {
    case POWER_STATE_TO_OFF :
      // Blink the light for 3 seconds, then leave off
      request_led_blink(LED_STATE_OFF, 3000);

      // Shut down the pi
      piOffTimer = millis() + 3000;
      power_state = POWER_STATE_SHUTDOWN;
      break;

    case POWER_STATE_SHUTDOWN :
      // Check if we have a timer active and action the state change.
      if (piOffTimer > 0 and piOffTimer > millis()) {
        piOffTimer = 0;
        power_state = POWER_STATE_OFF;
      }
      break;

    case POWER_STATE_OFF :
      sleep();
      power_state = POWER_STATE_TO_ON;
    /* FALL THROUGH */

    case POWER_STATE_TO_ON :
      // Blink the light for 3 seconds, then leave on
      request_led_blink(LED_STATE_ON, 3000);

      power_state = POWER_STATE_ON;
    /* FALLTHROUGH */
    case POWER_STATE_ON :
      if (bounce.rose() or button_current_event == SLEEP_REQUEST) { // Relase
        power_state = LED_STATE_TO_OFF;
        button_current_event = 0;
      }
      break;

    default :
      break;
  }
}
