#define WIRE_ENABLED
#define SLAVE_ADDRESS 0x04

#include <util/atomic.h> // this library includes the ATOMIC_BLOCK macro.

#ifdef WIRE_ENABLED
  #include <Wire.h>
#else
class SerialWire {
  public:
  
  SerialWire();
  void begin(char address)
  void onReceive(void (*func)(int))
  void onRequest(void (*func)())
  
  bool available()
  int read()
  int write()
  
  static void (*user_onRequest)(void);
  static void (*user_onReceive)(int);
  static void onRequestService();
  static void onReceiveService();
}
void SerialWire::SerialWire() {}
void SerialWire::begin(char address){}
void SerialWire::onReceive(void (*func)(int)){
  int bytesInBuffer = 0;
  (*func)(bytesInBuffer);
}
void SerialWire::onRequest(void (*func)()){
  (*func)();
}
void SerialWire::available(){
  return Serial.available();
}
int SerialWire::read(){
  return Serial.read()
}
<template T>
int SerialWire::write(T value){
  return Serial.write(value);
}

void SerialWire::onReceiveService()
{
  int numBytes = 1;
  user_onReceive(numBytes);
}
SerialWire Wire;


void serialEvent() {
  Wire.onReceiveService();
}
#endif // WIRE_ENABLED

volatile boolean ledOn;
volatile boolean ledOnChanged = false;

volatile boolean shouldSleep = false;
volatile boolean shouldSleepChanged = false;

volatile CHSV ledColor;
volatile boolean ledColorChanged = false;

volatile motor theta;
volatile boolean thetaChanged = false;
volatile motor radius;
volatile boolean radiusChanged = false;

volatile boolean heartbeatChanged = false;

volatile boolean receiveFlag = false;


/* STATE MACHINE */

static const char RASP_SEND_DATA = 1 << 7;
static const char RASP_REQUEST_DATA = 1 << 6;

static const char RASP_NULL = 0;

static const char RASP_LED_ON = 1;
static const char RASP_LED_COLOR = 2;

static const char RASP_THETA = 5;
static const char RASP_RADIUS = 6;

// const unsigned int RASP_GCODE = 10;

static const char RASP_STAYALIVE = 20;
static const char RASP_SLEEP = 21;

char RASP_REQ = RASP_NULL;

unsigned long heartbeatTime = 0;
CHSV color_buffered;
motor theta_buffered;
motor radius_buffered;


void raspberry_setup() {
  // initialize i2c as slave
  Wire.begin(SLAVE_ADDRESS);
  
  // define callbacks for i2c communication
  Wire.onReceive(receiveData);
  Wire.onRequest(sendData);
}
  
void raspberry_loop() {
  if (receiveFlag) {
    if(ledOnChanged)
    {
      // Set led on with ledOn
      ledOnChanged = false;
    }
    if(shouldSleepChanged) {
      if(shouldSleep) {
        // Emit sleep signal
        if(pi_request_sleep_event()){
          // Keep trying until we sleep?
          shouldSleep = false;
        }
      }
    }
    if(ledColorChanged){
      ATOMIC_BLOCK(ATOMIC_RESTORESTATE) {
        color_buffered.h = ledColor.h;
        color_buffered.s = ledColor.s;
        color_buffered.v = ledColor.v;
      }
      set_color(color_buffered);
      // Set the color with color_buffered
      ledColorChanged = false;
    }
    if(thetaChanged){
      ATOMIC_BLOCK(ATOMIC_RESTORESTATE) {
        theta_buffered.steps = theta.steps;
      }
      // Set the theta with theta_buffered
      set_theta_motor_position(theta_buffered);
      thetaChanged = false;
    }
    if(radiusChanged){
      ATOMIC_BLOCK(ATOMIC_RESTORESTATE) {
        radius_buffered.steps = radius.steps;
      }
      // Set the radius with radius_buffered
      set_theta_motor_position(radius_buffered);
      radiusChanged = false;
    }
    if(heartbeatChanged){
      // TODO: Make the heat beat monitor
      heartbeatTime = currentMillis + 10000;
      heartbeatChanged = false;
    }
  }
}


// callback for received data
void receiveData(int byteCount){ // Wire supports max 32bytes
  DEBUG_PRINTLN("receiveData");

  if(Wire.available())
  { 
    byte header = Wire.read();
    DEBUG_PRINTLN(header);
    
    if(header & RASP_REQUEST_DATA){
      // Drop the MS two bits and setup our state for the next request.
      RASP_REQ = header & 0x00111111;
    }
    else if(header & RASP_SEND_DATA){
      // Drop the MS two bits and read in the expected values.
      header = header & 0x00111111;
      DEBUG_PRINTLN(header);
      switch(header){
        
        case RASP_LED_ON:
          DEBUG_PRINTLN("LED ON");
          ledOn = Wire.read();
          ledOnChanged = true;
          break;
          
        case RASP_LED_COLOR :
          DEBUG_PRINT("ledColorChanged ");
          for(char i=0; Wire.available(); ++i)
          {
            char c = Wire.read();
            ledColor.raw[i] = c;
            DEBUG_PRINT(c, HEX);
            DEBUG_PRINT(', ');
          }
          DEBUG_PRINTLN(' ');
          
          ledColorChanged = true;
          break;
          
        case RASP_THETA :
          for(char i=0; Wire.available(); ++i)
          {
            char c = Wire.read();
            theta.uBytes[i] = c;
          }
          thetaChanged = true;
          break;
          
        case RASP_RADIUS :
          for(char i=0; Wire.available(); ++i)
          {
            char c = Wire.read();
            radius.uBytes[i] = c;
          }
          radiusChanged = true;
          break;
        
        case RASP_SLEEP :
          shouldSleep = true;
          shouldSleepChanged = true;
        
        case RASP_STAYALIVE :
          heartbeatChanged = true;
          
        default :
          break;
      }
    }
    else
    {
      // Didn't get a header?
    }
  }
  receiveFlag = true;
}


// callback for sending data
void sendData(){
  switch(RASP_REQ){
    case RASP_LED_ON :
      Wire.write(true);
      break;
      
    case RASP_LED_COLOR :
      Wire.write(get_color().raw, 3);
      break;
      
    case RASP_THETA :
      wire_long theta_out;
      theta_out.u = 21321657498;
      Wire.write(theta_out.uBytes, 4);
      break;
      
    case RASP_RADIUS :
      wire_long radius_out;
      radius_out.u = 21321657498;
      Wire.write(radius_out.uBytes, 4);
      break;
      
    case RASP_STAYALIVE :
      // TODO: This is our chance to tell the Pi to shutdown.
      bool stayalive;
      stayalive = true;
      Wire.write(stayalive);
      break;
      
    default :
      break;
  }
  RASP_REQ = RASP_NULL;
}
