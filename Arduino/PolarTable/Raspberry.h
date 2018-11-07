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

static const char RASP_LIGHTING_ON = 1;
static const char RASP_LIGHTING_OFF = 2;
static const char RASP_LIGHTING_COLOR = 3;

static const char RASP_THETA = 5;
static const char RASP_RADIUS = 6;

// const unsigned int RASP_GCODE = 10;

static const char RASP_SLEEP = 21;
static const char RASP_WAKE = 22;

char RASP_REQ = RASP_NULL;

/*************************************************************
  Processing for peripheral mode (slave)
*************************************************************/

// Data has been sent to us from a controller
void process_recieve_data(char request, byte *recieve_data, byte recieve_data_size) {
  // We have new data to push out to the device.
  switch (request) {
    case RASP_LIGHTING_ON :
      evtManager.trigger(LIGHTING_TURN_ON);
      break;

    case RASP_LIGHTING_OFF :
      evtManager.trigger(LIGHTING_TURN_OFF);
      break;

    case RASP_LIGHTING_COLOR :
      {
        DEBUG_PRINT("ledColorChanged ");
        char *leds[3];
        memcpy(&leds[0], &recieve_data[1], recieve_data_size - 1);
        evtManager.trigger(LIGHTING_BLEND, leds);
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
      evtManager.trigger(SYSTEM_SLEEP_ACTION);
      break;

    default :
      break;
  }
}

// Data has been requested from us by a controller. We need to prep it for the next request.
void process_send_data(char request) {
  static byte send_data[32];
  static byte send_data_size = 0;

  // If we have a request for data load up the send buffer.
  switch (request) {
    case RASP_NULL:
      break;

    case RASP_LIGHTING_ON :
      send_data_size = 1;
      send_data[0] = get_lighting_state();
      break;

    case RASP_LIGHTING_OFF :
      send_data_size = 1;
      send_data[0] = !get_lighting_state();
      break;

    case RASP_LIGHTING_COLOR :
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
  Controller mode (master)
*************************************************************/

void send_raspberry_shutdown(){
   /*
      0: Successful send.
      1: Send buffer too large for the twi buffer. This should not happen, as the TWI buffer length set in twi.h is equivalent to the send buffer length set in Wire.h.
      2: Address was sent and a NACK received. This is an issue, and the master should send a STOP condition.
      3: Data was sent and a NACK received. This means the slave has no more to send. The master can send a STOP condition, or a repeated START.
      4: Another twi error took place (eg, the master lost bus arbitration).
    */
   Wire.beginTransmission(RASPBERRY_I2C_ADDRESS);
   Wire.send("shutdown");
   byte ack = Wire.endTransmission();
   if(ack != 0){
    DEBUG_PRINT("I2C send failed: ");
    DEBUG_PRINT_LN(ack);
    evtManager.trigger(ERROR_RASPBERRY_SEND);
   }
}

#define ACK_POLL_TIMEOUT 1
bool ack_poll() {
    byte ACK = 0;
    for (byte time = 0;time < ACK_POLL_TIMEOUT;time++) {
        Wire.beginTransmission(this->deviceAddress);
        ACK = Wire.endTransmission();
        if (ACK == 0) {
          return true;
        }
        delay(1);
    }
    return false;
}

/*************************************************************
  Setup and loop
*************************************************************/


/*************************************************************
  State
*************************************************************/

void state_coms_sync_recieve_data();

State state_coms_idle(NULL, NULL, NULL);
State state_coms_peripheral(NULL, NULL, NULL);
State state_coms_on_receive(NULL, NULL, NULL);
State state_coms_on_request(NULL, NULL, NULL);

State state_coms_controller(NULL, NULL, NULL);
State state_coms_send(NULL, NULL, NULL);
State state_coms_recieve(NULL, NULL, NULL);

Fsm fsm_i2c_coms(&state_coms_idle);


void state_coms_sync_recieve_data(){
  if (recieve_buffer_size) {
    byte recieve_data_size = 0;
    byte recieve_data[recieve_buffer_size];
    ATOMIC_BLOCK(ATOMIC_RESTORESTATE) {
      memcpy(&recieve_data[0], (byte*) &recieve_buffer[0], recieve_buffer_size);
      recieve_data_size = recieve_buffer_size;
      recieve_buffer_size = 0;
    }
  }
}

void state_coms_sync_process_recieve_data(){
  // Unpack the bits and find out what data we are working with.
  bool is_send = recieve_data[0] & RASP_SEND_DATA;
  RASP_REQ = recieve_data[0] & (~RASP_SEND_DATA);

  if (!is_send) {
    process_recieve_data(RASP_REQ, recieve_data, recieve_data_size);
  }
  else {
    process_send_data(RASP_REQ);
  }
}


void raspberry_setup() {
  // Initialize i2c and make sure we are addressable
  Wire.begin(ARDUINO_I2C_ADDRESS);

  // Define callbacks for i2c peripheral mode communication
  Wire.onReceive(receiveData);
  Wire.onRequest(sendData);
}

void raspberry_loop() {
  // Check if there is any data waiting for us in the recieve buffer
  // If we have some new data run it quickly through the state machine, set values and trigger events.
  if (recieve_buffer_size) {
    evtManager.trigger(RASPBERRY_HEARTBEAT);

    byte recieve_data_size = 0;
    byte recieve_data[recieve_buffer_size];
    ATOMIC_BLOCK(ATOMIC_RESTORESTATE) {
      memcpy(&recieve_data[0], (byte*) &recieve_buffer[0], recieve_buffer_size);
      recieve_data_size = recieve_buffer_size;
      recieve_buffer_size = 0;
    }

    // Unpack the bits and find out what data we are working with.
    bool is_send = recieve_data[0] & RASP_SEND_DATA;
    RASP_REQ = recieve_data[0] & (~RASP_SEND_DATA);

    if (!is_send) {
      process_recieve_data(RASP_REQ, recieve_data, recieve_data_size);
    }
  }
  process_send_data(RASP_REQ);
}

#endif // DISABLE_RASPBERRY_COMS
