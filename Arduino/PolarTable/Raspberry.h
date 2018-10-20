/*************************************************************
  Communicate with the Rasberry Pi
*************************************************************/
#pragma once

#ifdef DISABLE_RASPBERRY_COMS

void raspberry_setup() {}
void raspberry_loop() {}

#else

#include "Wire.h"
#include "Helpers.h"
#include <util/atomic.h> // this library includes the ATOMIC_BLOCK macro.

/*************************************************************
  States
*************************************************************/

// The high bit is used to flag if we need to send the data
// or if we are recieving the data.
static const char RASP_SEND_DATA = 1 << 7;

// The rest are values to specify the data type.
static const char RASP_NULL = 0;

static const char RASP_LED_ON = 1;
static const char RASP_LED_OFF = 2;
static const char RASP_LED_COLOR = 3;

static const char RASP_THETA = 5;
static const char RASP_RADIUS = 6;

// const unsigned int RASP_GCODE = 10;

static const char RASP_STAYALIVE = 20;
static const char RASP_SLEEP = 21;
static const char RASP_WAKE = 22;

char RASP_REQ = RASP_NULL;

// Buffers
byte send_buffer[32];
unsigned char send_buffer_size = 0;

volatile byte recieve_buffer[32];
volatile unsigned char recieve_buffer_size = 0;


/*************************************************************
  Interrupts
*************************************************************/

// The controller calls us (the peripheral) and tells use what data it wants.
// We save that into the recieve buffer and use it for generating data for the
// next request to sendData

// Data selection by controller
void receiveData(int byteCount) { // Wire supports max 32bytes
  UNUSED(byteCount);
  for (recieve_buffer_size = 0; Wire.available(); ++recieve_buffer_size)
  {
    recieve_buffer[recieve_buffer_size] = Wire.read();
  }
}

// Data rquest by controller
void sendData() {
  if (send_buffer_size > 0) {
    Wire.write(send_buffer, send_buffer_size);
    send_buffer_size = 0;
  }
}


void process_recieve_data(char request, byte *recieve_data, byte recieve_data_size){
  // We have new data to push out to the device.
  switch (request) {
    case RASP_STAYALIVE :
      break;
      
    case RASP_LED_ON :
      evtManager.trigger(LIGHTING_STATE, LED_ON);
      break;
      
    case RASP_LED_OFF :
      evtManager.trigger(LIGHTING_STATE, LED_OFF);
      break;

    case RASP_LED_COLOR :
      {
        DEBUG_PRINT("ledColorChanged ");
        char *leds[3];
        memcpy(&leds[0], &recieve_data[1], recieve_data_size - 1);
        evtManager.trigger(LIGHTING_COLOR, leds);
        break;
      }

    case RASP_THETA :
      {
        MotorPosition theta;
        memcpy(&theta.uBytes[0], &recieve_data[1], recieve_data_size - 1);
        evtManager.trigger(MOTOR_TARGET_THETA, &theta);
        break;
      }

    case RASP_RADIUS :
      {
        MotorPosition radius;
        memcpy(&radius.uBytes[0], &recieve_data[1], recieve_data_size - 1);
        evtManager.trigger(MOTOR_TARGET_RADIUS, &radius);
        break;
      }

    case RASP_SLEEP :
      evtManager.trigger(SYSTEM_EVENT, SYSTEM_SLEEP);
      break;

    default :
      break;
  }
}

void process_send_data(char request){
  static byte send_data[32];
  static byte send_data_size = 0;
  
  // If we have a request for data load up the send buffer.
  switch (request) {
    case RASP_NULL:
      break;
      
    case RASP_LED_ON :
      send_data_size = 1;
      send_data[0] = 1;  // true
      break;

    case RASP_LED_OFF :
      send_data_size = 1;
      send_data[0] = 1;  // true
      break;

    case RASP_LED_COLOR :
    {
      send_data_size = 3;
      memcpy(&send_data[0], &get_color()[0], send_data_size);
      break;
    }

    case RASP_THETA :
    {
      wire_packed<long> theta_out = wire_pack(321657498);
      memcpy(&send_data[0], &theta_out.uBytes[0], theta_out.size);
      break;
   }

    case RASP_RADIUS :
    {
      wire_packed<long> radius_out = wire_pack(2132498);
      memcpy(&send_data[0], &radius_out.uBytes[0], radius_out.size);
      break;
    }

    case RASP_STAYALIVE :
    {
      // TODO: This is our chance to tell the Pi to shutdown.
      send_data_size = 1;
      send_data[0] = 1;  // true
      break;
    }
    default :
      break;
  }
  // Reset the request?
  request = RASP_NULL;

  if (send_data_size) {
    // Don't overwrite the last send_buffer message until it's sent
    if (send_buffer_size == 0) {
      ATOMIC_BLOCK(ATOMIC_RESTORESTATE) {
        memcpy(&send_buffer[0], &send_data[0], send_data_size);
        send_buffer_size = send_data_size;
      }
    }
  }
}

/*************************************************************
  Setup and loop
*************************************************************/

void raspberry_setup() {
  // Initialize i2c as peripheral
  Wire.begin(ARDUINO_I2C_ADDRESS);

  // Define callbacks for i2c communication
  Wire.onReceive(receiveData);
  Wire.onRequest(sendData);
}

void raspberry_loop() {  
  // Check if there is any data waiting for us in the recieve buffer
  // If we have some new data run it quickly through the state machine, set values and trigger events.
  if (recieve_buffer_size) {
    raspberry_heartbeat();
    
    byte recieve_data_size = 0;
    byte recieve_data[recieve_buffer_size];
    ATOMIC_BLOCK(ATOMIC_RESTORESTATE) {
      memcpy(&recieve_data[0], (byte*) &recieve_buffer[0], recieve_buffer_size);
      recieve_data_size = recieve_buffer_size;
      recieve_buffer_size = 0;
    }

    // Unpack the bits and find out what data we aew working with.
    bool is_send = recieve_data[0] & RASP_SEND_DATA;
    RASP_REQ = recieve_data[0] & (~RASP_SEND_DATA);
  
    if (!is_send){
      process_recieve_data(RASP_REQ, recieve_data, recieve_data_size);
    }
  }
  process_send_data(RASP_REQ);
}

#endif // DISABLE_RASPBERRY_COMS
