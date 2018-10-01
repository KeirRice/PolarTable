/*************************************************************
Manage the wake/sleep button.
*************************************************************/

#include <Bounce2.h>

extern SX1509 io;

// Instantiate a Bounce object
Bounce bounce = Bounce(); 


/*************************************************************
Pins
*************************************************************/

static const char ARDUINO_WAKE_SWITCH_PIN = 3;

// SX1509 Pins:
static const char SX1509_LED_PIN = 3;


/*************************************************************
State
*************************************************************/

static const char LED_STATE_OFF = 0;
static const char LED_STATE_ON = 1;
static const char LED_STATE_BLINK = 2;

static const char LED_STATE_TO_OFF = 10;
static const char LED_STATE_TO_ON = 11;
static const char LED_STATE_TO_BLINK = 12;


static const int POWER_STATE_OFF = 0;
static const int POWER_STATE_ON = 1;

static const char POWER_STATE_TO_OFF = 10;
static const char POWER_STATE_TO_ON = 11;

char POWER_STATE = POWER_STATE_ON;
char LED_STATE = LED_STATE_OFF;

/*************************************************************
Functions
*************************************************************/

void sleep(){
    DEBUG_PRINTLN("going to sleep");
    delay(300);
    // Breathe an LED: 500ms LOW, 500ms HIGH,
    // 251ms to rise from low to high
    // 250ms to fall from high to low
    io.breathe(SX1509_LED_PIN, 500, 500, 251, 250);
  
    // Shutdown the Pi
    
    sleepNow();     // sleep function called here
    
    wake(); // Called after we are woken
}

void wake(){
    // Restart out millis counter as it wasn't running while alseep
    resetMills();
}


bool pi_request_sleep_event(){
  if (POWER_STATE == POWER_STATE_ON || POWER_STATE == POWER_STATE_TO_OFF){
    POWER_STATE == POWER_STATE_TO_OFF;
    return true;
  }
  return false;
}


/*************************************************************
Setup and main loop.
*************************************************************/

void button_setup()
{
  // Use the internal 2MHz oscillator.
  // Set LED clock to 500kHz (2MHz / (2^(3-1)):
  io.clock(INTERNAL_CLOCK_2MHZ, 3);
  io.pinMode(SX1509_LED_PIN, ANALOG_OUTPUT); // LED needs PWM
  io.ledDriverInit(SX1509_LED_PIN);

  pinMode(ARDUINO_WAKE_SWITCH_PIN, INPUT_PULLUP);
  char debounce_time = 5; // Milliseconds
  bounce.interval(debounce_time);
  bounce.attach(ARDUINO_WAKE_SWITCH_PIN);
}

void button_loop(){
  
  DEBUG_PRINTLN("button_loop");
  static unsigned long ledOffTimer = 0;
  static unsigned long piOffTimer = 0;
  
  bounce.update();
  DEBUG_PRINTLN("bounce updated");
  
  
  DEBUG_PRINTLN("switching power state");
  switch(POWER_STATE) {
    case POWER_STATE_TO_OFF :
      // Shut down the pi
      piOffTimer = millis() + 3000;
      
      // Blink the light for 3 seconds
      ledOffTimer = millis() + 3000;
      LED_STATE = LED_STATE_TO_BLINK;
      
      // Check if we have a timer active and action the state change.
      if (piOffTimer > 0 and piOffTimer > millis()){
        piOffTimer = 0;
        POWER_STATE = POWER_STATE_OFF;
      }
      break;
      
    case POWER_STATE_OFF :
      sleep();
      POWER_STATE = POWER_STATE_TO_ON;
      /* FALL THROUGH */

    case POWER_STATE_TO_ON :
      // Blink the light for 3 seconds
      ledOffTimer = millis() + 3000;
      LED_STATE = LED_STATE_TO_BLINK;
      
      POWER_STATE = POWER_STATE_ON;
      /* FALLTHROUGH */
    case POWER_STATE_ON :
      if (bounce.rose()){ // Relase
        POWER_STATE = LED_STATE_TO_OFF;
      }
      break;
      
    default :
      break;
  }
  
  // Check if we have a timer active and action the state change.
  if (ledOffTimer > 0 and ledOffTimer > millis()){
    ledOffTimer = 0;
    LED_STATE = LED_STATE_TO_OFF;
  }
  
  DEBUG_PRINTLN("switching led state");
  switch(LED_STATE){
    
    case LED_STATE_TO_OFF :
      io.digitalWrite(SX1509_LED_PIN, LOW);
      LED_STATE = LED_STATE_OFF;
      /* FALL THROUGH */
    case LED_STATE_OFF :
      break;
      
    case LED_STATE_TO_ON :
      io.breathe(SX1509_LED_PIN, 0, 1000, 1000, 250);
      io.digitalWrite(SX1509_LED_PIN, HIGH);
      LED_STATE = LED_STATE_ON;
      /* FALL THROUGH */
    case LED_STATE_ON :
      break;
      
    case LED_STATE_TO_BLINK :
      io.breathe(SX1509_LED_PIN, 500, 500, 250, 250);
      LED_STATE = LED_STATE_BLINK;
      /* FALL THROUGH */
    case LED_STATE_BLINK :
      break;
    
    default :
      break;
  }
}
