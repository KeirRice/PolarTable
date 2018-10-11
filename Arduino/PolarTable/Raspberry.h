
#include <util/atomic.h> // this library includes the ATOMIC_BLOCK macro.
#include "Wire.h"

/* STATE MACHINE */

static const char RASP_SEND_DATA = 1 << 7;
static const char RASP_REQUEST_DATA = 1 << 6;

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

/* Buffers */
byte send_buffer[32];
unsigned char send_buffer_size = 0;

volatile byte recieve_buffer[32];
volatile unsigned char recieve_buffer_size = 0;

// callback for received data
void receiveData(int byteCount){ // Wire supports max 32bytes
  UNUSED(byteCount);
  for(recieve_buffer_size=0; Wire.available(); ++recieve_buffer_size)
  {
    recieve_buffer[recieve_buffer_size] = Wire.read();
  }
}

// callback for sending data
void sendData(){
  if(send_buffer_size > 0){
    Wire.write(send_buffer, send_buffer_size);
    send_buffer_size = 0;
  }
}

/* Setup and loop */
void raspberry_setup() {
  // initialize i2c as slave
  Wire.begin(SLAVE_ADDRESS);
  
  // define callbacks for i2c communication
  Wire.onReceive(receiveData);
  Wire.onRequest(sendData);
}
  
void raspberry_loop() {
  // Check if there is any data waiting for us in the recieve buffer
  byte recieve_data[recieve_buffer_size];
  byte recieve_data_size = 0;
  if(recieve_buffer_size){
    ATOMIC_BLOCK(ATOMIC_RESTORESTATE) {
      memcpy(&recieve_data[0], (byte*) &recieve_buffer[0], recieve_buffer_size);
      recieve_data_size = recieve_buffer_size;
      recieve_buffer_size = 0;
    }
  }

  // If we have some new data run it quickly through the state machine, set values and trigger events.
  if(recieve_data_size){
    byte header = recieve_data[0];
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
        case RASP_LED_ON :
          lighting_event(LED_ON_REQUEST);
          break;
        case RASP_LED_OFF :
          lighting_event(LED_OFF_REQUEST);
          break;
          
        case RASP_LED_COLOR :
          {
            DEBUG_PRINT("ledColorChanged ");
            CHSV ledColor;
            memcpy(&ledColor.raw[0], &recieve_data[1], recieve_buffer_size - 1);
            set_color(ledColor);
            break;
          }
          
        case RASP_THETA :
          {
            motor theta;
            memcpy(&theta.uBytes[0], &recieve_data[1], recieve_buffer_size - 1);
            set_theta_motor_position(theta);
            break;
          }
          
        case RASP_RADIUS :
          {
            motor radius;
            memcpy(&radius.uBytes[0], &recieve_data[1], recieve_buffer_size - 1);
            set_radius_motor_position(radius);
            break;
          }
        
        case RASP_SLEEP :
          request_sleep();
          break;
          
        case RASP_STAYALIVE :
          // TODO
          break;
          
        default :
          break;
      }
    }
  }

  static byte send_data[32];
  static byte send_data_size = 0;
  if(RASP_REQ != RASP_NULL){
    // If we have a request for data load up the send buffer.
    switch(RASP_REQ){
      case RASP_LED_ON :
        send_data_size = 1;
        send_data[0] = 1;  // true
        break;
        
      case RASP_LED_OFF :
        send_data_size = 1;
        send_data[0] = 1;  // true
        break;
        
      case RASP_LED_COLOR :
        send_data_size = 3;
        memcpy(&send_data[0], &get_color().raw[0], send_data_size);
        break;
        
      case RASP_THETA :
        wire_long theta_out;
        theta_out.u = 321657498;
        
        send_data_size = 4;
        memcpy(&send_data[0], &theta_out.uBytes[0], send_data_size);
        break;
        
      case RASP_RADIUS :
        wire_long radius_out;
        radius_out.u = 2132498;
        
        send_data_size = 4;
        memcpy(&send_data[0], &radius_out.uBytes[0], send_data_size);
        break;
        
      case RASP_STAYALIVE :
        // TODO: This is our chance to tell the Pi to shutdown.
        send_data_size = 1;
        send_data[0] = 1;  // true
        break;
        
      default :
        break;
    }
    RASP_REQ = RASP_NULL;
  }
  
  if(send_data_size){
    // Don't overwrite the last send_buffer message until it's sent
    if(send_buffer_size == 0){
      ATOMIC_BLOCK(ATOMIC_RESTORESTATE) {
        memcpy(&send_buffer[0], &send_data[0], send_data_size);
        send_buffer_size = send_data_size;
      }
    }
  }
}
