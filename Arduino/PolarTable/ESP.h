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

static const byte REGISTER_MODE = 0x20;
static const byte EVENT_MODE = 0x21;
static const byte ECHO_MODE = 0x22;

static const char reg_size = 10;
volatile uint8_t i2c_regs[reg_size];
volatile byte reg_position;
volatile long int i2c_reg_changed;



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
} // End SetRegisterEvent()


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
}

bool search_char(const char *search_string, int string_length, char current_character, int &index){
  if(search_string[index++] != current_character){
    index = 0;
  }
  else if(index == string_length){
    return true;
  }
  return false;
}

const char *search_string = "OFF uart log";
void esp_loop(){
  static boolean packets_enabled = false;
  static boolean esp_logging_complete = false;
  int index = 0;
  
  // Read from Serial3 (ESP) and map to Serial1 (USB)
  while(Serial3.available()){
    char current_character = Serial3.read();
    Serial.write(current_character);

    // Check for the end of the ESP logging
    if(!esp_logging_complete){
      // TODO: Move strings into PROGMEM and just fetch the byte we need to check next.
      
      int string_length = 11;
      if(search_char(search_string, string_length, current_character, index)){
        esp_logging_complete = true;
      }
    }
  }
  
  // Read from Serial1 (USB) and map to Serial3 (ESP)
  while(Serial.available()){
    Serial3.write(Serial.read());
  }

  // Enable the packet interface
  if (not packets_enabled && esp_logging_complete){
    myPacketSerial.setStream(&Serial3);
    myPacketSerial.setPacketHandler(&onPacketReceived);
    packets_enabled = true;
  }
}
