/*************************************************************
  Talk with the WIFI

  Ubuntu/RPi/Phone <=> Over TELNET <=> ESP <=> Serial3 <=> Arduino
*************************************************************/
#pragma once

#include <util/atomic.h> // this library includes the ATOMIC_BLOCK macro.
#include <PacketSerial.h>
#include <simpleRPC.h>

extern EventManager evtManager;
PacketSerial myPacketSerial;
SimpleRPC rpc;

void send_raspberry_shutdown() {
  /*
     0: Successful send.
     1: Send buffer too large for the twi buffer. This should not happen, as the TWI buffer length set in twi.h is equivalent to the send buffer length set in Wire.h.
     2: Address was sent and a NACK received. This is an issue, and the master should send a STOP condition.
     3: Data was sent and a NACK received. This means the slave has no more to send. The master can send a STOP condition, or a repeated START.
     4: Another twi error took place (eg, the master lost bus arbitration).
  */
//  Wire.beginTransmission(RASPBERRY_I2C_ADDRESS);
//  byte payload = 0b10101010;
//  Wire.write(payload);
//  byte ack = Wire.endTransmission();
//  if (ack != 0) {
//    DEBUG_PRINT("I2C send failed: ");
//    DEBUG_PRINT_VAR(RASPBERRY_I2C_ADDRESS, payload, ack);
//    // evtManager.trigger(ERROR_RASPBERRY_SEND);
//  }
//  else {
//    DEBUG_PRINT("I2C send success: ");
//    DEBUG_PRINT_VAR(RASPBERRY_I2C_ADDRESS, payload, ack);    
//  }
}

void send_motor_ready() {
  /*
     0: Successful send.
     1: Send buffer too large for the twi buffer. This should not happen, as the TWI buffer length set in twi.h is equivalent to the send buffer length set in Wire.h.
     2: Address was sent and a NACK received. This is an issue, and the master should send a STOP condition.
     3: Data was sent and a NACK received. This means the slave has no more to send. The master can send a STOP condition, or a repeated START.
     4: Another twi error took place (eg, the master lost bus arbitration).
  */
//  Wire.beginTransmission(RASPBERRY_I2C_ADDRESS);
//  Wire.write(2);
//  byte ack = Wire.endTransmission();
//  if (ack != 0) {
//    DEBUG_PRINT("I2C send failed: ");
//    DEBUG_PRINT_VAR(ack);
//    evtManager.trigger(ERROR_RASPBERRY_SEND);
//  }
}



static const char reg_size = 10;
volatile uint8_t i2c_regs[reg_size];
volatile byte reg_position;
volatile long int i2c_reg_changed;

void SetRegisterEvent(const uint8_t* buffer, size_t howMany)
{
  if (howMany < 1)
  {
    return; // Sanity-check
  }

  int read_head = 0;
  
  reg_position = buffer[read_head++];
  --howMany;
  if (!howMany)
  {
    return; // This write was only to set the buffer for next read
  }

  while (howMany--)
  {
    //Store the recieved data in the currently selected register
    i2c_regs[reg_position] = buffer[read_head++];
    ++reg_position;
    i2c_reg_changed |= 1 << reg_position;
    reg_position = (reg_position >= reg_size) ? reg_position : 0;
  }
} //End SetRegisterEvent()


void GetRegisterEvent()
{
  // send_size is from register position to the end of the buffer
  size_t send_size = reg_size - reg_position;

  // Take a copy of the volitile register so they don't change under us.
  byte send_buffer[send_size];
  ATOMIC_BLOCK(ATOMIC_RESTORESTATE) {
    memcpy((void*) &send_buffer, (void*) &i2c_regs[reg_position], send_size);
  }

  // Ready to send
  myPacketSerial.send(send_buffer, send_size);
  reg_position = 0;
} //End GetRegisterEvent


void FireEvent(const uint8_t* buffer, size_t howMany){
  assert(howMany <= 3);
  long event_id = 0;
  for(unsigned int i = 0; i < howMany; ++i){
    // LSB mode?
    event_id = (event_id << 8) | buffer[i];
  }
  evtManager.trigger(EventID(event_id));
}

static const byte REGISTER_MODE = 0x20;
static const byte EVENT_MODE = 0x21;
static const byte ECHO_MODE = 0x22;
void onPacketReceived(const void* sender, const uint8_t* buffer, size_t size)
{
   DEBUG_PRINTLN("onPacketReceived");
    if (sender == &myPacketSerial)
    {
        // Do something with the packet from myPacketSerial.
        byte address = buffer[0] >> 1;
        boolean do_read = buffer[0] & 1;
        boolean do_write = !do_read;

        // "Pop" the address element.
        buffer = &buffer[1];
        --size;

        if(address == REGISTER_MODE){
          if(do_read){
            GetRegisterEvent();
          }
          if(do_write){
            SetRegisterEvent(buffer, size);
          }
        }
        else if(address == EVENT_MODE){
          FireEvent(buffer, size);
        }
        else {
          for (uint i = 0; i < size; ++i){
            DEBUG_PRINT_VAR(i, buffer[i]);
          }
        }
    }
//    else if (sender == &myOtherPacketSerial)
//    {
//        // Do something with the packet from myOtherPacketSerial.
//    }
}


void esp_setup(){
  Serial3.begin(115200);


  // rpc.begin(Serial3);
}


void esp_loop(){
  // put your main code  here, to run repeatedly:
  static boolean setup_already = true;
  
  char *search = {"OFF uart log"};
  int index = 0;
  
  while(Serial3.available()){
    char current_character = Serial3.read();
    if(search[index++] != current_character){
      index = 0;
    }
    if(index == 11){
      setup_already = false;
    }
    Serial.write(current_character);
  }
  while(Serial.available()){
    Serial3.write(Serial.read());
  }
  
  
  if (not setup_already){
    myPacketSerial.setStream(&Serial3);
    myPacketSerial.setPacketHandler(&onPacketReceived);
    setup_already = true;
  }

  
//  rpc.interface(
//    send_raspberry_shutdown, "send_raspberry_shutdown: Shutdown the raspberry pi.",
//    send_motor_ready, "send_motor_ready: Signal the motor is ready for the next command.");
}
