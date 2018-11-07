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
   Wire.write("shutdown");
   byte ack = Wire.endTransmission();
   if(ack != 0){
    DEBUG_PRINT("I2C send failed: ");
    DEBUG_PRINT_VAR(ack);
    evtManager.trigger(ERROR_RASPBERRY_SEND);
   }
}

void send_motor_ready(){
   /*
      0: Successful send.
      1: Send buffer too large for the twi buffer. This should not happen, as the TWI buffer length set in twi.h is equivalent to the send buffer length set in Wire.h.
      2: Address was sent and a NACK received. This is an issue, and the master should send a STOP condition.
      3: Data was sent and a NACK received. This means the slave has no more to send. The master can send a STOP condition, or a repeated START.
      4: Another twi error took place (eg, the master lost bus arbitration).
    */
   Wire.beginTransmission(RASPBERRY_I2C_ADDRESS);
   Wire.write("motor_ready");
   byte ack = Wire.endTransmission();
   if(ack != 0){
    DEBUG_PRINT("I2C send failed: ");
    DEBUG_PRINT_VAR(ack);
    evtManager.trigger(ERROR_RASPBERRY_SEND);
   }
}

#define ACK_POLL_TIMEOUT 1
bool ack_poll() {
    byte ACK = 0;
    for (byte time = 0;time < ACK_POLL_TIMEOUT;time++) {
        Wire.beginTransmission(RASPBERRY_I2C_ADDRESS);
        ACK = Wire.endTransmission();
        if (ACK == 0) {
          return true;
        }
        delay(1);
    }
    return false;
}

struct RaspberryEventDriver : public EventTask
{
  RaspberryEventDriver() {};

  using EventTask::execute;
  
  void execute(Event *evt)
  {
    send_motor_ready();
  }
};

/*************************************************************
  Setup and loop
*************************************************************/

void raspberry_setup() {
  struct RaspberryEventDriver raspberry_event_listner = RaspberryEventDriver();
  evtManager.subscribe(Subscriber(MOTOR_READY, &raspberry_event_listner));
}
  
void raspberry_loop() {}


#endif // DISABLE_RASPBERRY_COMS
