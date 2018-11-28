/*************************************************************
  I2C Communication
*************************************************************/
#pragma once

#ifdef DISABLE_I2C_COMS

void i2c_controller_setup() {}
void i2c_controller_loop() {}

#else

#include "Wire.h"
#include "Helpers.h"
#include <util/atomic.h> // this library includes the ATOMIC_BLOCK macro.


typedef struct Register {

  public:
    Register(const EventID &_event, unsigned char _register_adderess, const byte _mask_width=1) : set_event(_event), packed_register_mask(_register_adderess | (_mask_width << 6)) {}
    
    const EventID &set_event;  // Callback for when we have a value

    byte get_address(){
      return packed_register_mask & 0b00111111;
    }
    byte get_bytes(){
      return packed_register_mask >> 6;
    }
    long get_mask(){
      return 1; // return offset_bitmask(this->get_bytes(), this->get_adderess());
    }
    
  private:
    const byte packed_register_mask;
    
} Register;


/*************************************************************
  Controller mode (master)
*************************************************************/

void send_raspberry_shutdown() {
  /*
     0: Successful send.
     1: Send buffer too large for the twi buffer. This should not happen, as the TWI buffer length set in twi.h is equivalent to the send buffer length set in Wire.h.
     2: Address was sent and a NACK received. This is an issue, and the master should send a STOP condition.
     3: Data was sent and a NACK received. This means the slave has no more to send. The master can send a STOP condition, or a repeated START.
     4: Another twi error took place (eg, the master lost bus arbitration).
  */
  Wire.beginTransmission(RASPBERRY_I2C_ADDRESS);
  byte payload = 0b10101010;
  Wire.write(payload);
  byte ack = Wire.endTransmission();
  if (ack != 0) {
    DEBUG_PRINT("I2C send failed: ");
    DEBUG_PRINT_VAR(RASPBERRY_I2C_ADDRESS, payload, ack);
    // evtManager.trigger(ERROR_RASPBERRY_SEND);
  }
  else {
    DEBUG_PRINT("I2C send success: ");
    DEBUG_PRINT_VAR(RASPBERRY_I2C_ADDRESS, payload, ack);    
  }
}

void send_motor_ready() {
  /*
     0: Successful send.
     1: Send buffer too large for the twi buffer. This should not happen, as the TWI buffer length set in twi.h is equivalent to the send buffer length set in Wire.h.
     2: Address was sent and a NACK received. This is an issue, and the master should send a STOP condition.
     3: Data was sent and a NACK received. This means the slave has no more to send. The master can send a STOP condition, or a repeated START.
     4: Another twi error took place (eg, the master lost bus arbitration).
  */
  Wire.beginTransmission(RASPBERRY_I2C_ADDRESS);
  Wire.write(2);
  byte ack = Wire.endTransmission();
  if (ack != 0) {
    DEBUG_PRINT("I2C send failed: ");
    DEBUG_PRINT_VAR(ack);
    evtManager.trigger(ERROR_RASPBERRY_SEND);
  }
}


#define ACK_POLL_TIMEOUT 1
bool ack_poll() {
  byte ACK = 0;
  for (byte time = 0; time < ACK_POLL_TIMEOUT; time++) {
    Wire.beginTransmission(RASPBERRY_I2C_ADDRESS);
    ACK = Wire.endTransmission();
    if (ACK == 0) {
      return true;
    }
    delay(1);
  }
  return false;
}

byte read_byte(byte peripheral_address, byte register_address){
  byte register_length = 1;
  
  Wire.beginTransmission(peripheral_address);
  Wire.write(register_address);
  Wire.endTransmission(false);
  Wire.requestFrom(peripheral_address, register_length);
  
  byte data = 0;
  if(Wire.available()) {
    data = Wire.read();
  }
  Wire.endTransmission(true);
  return data;
}

void read_bytes(byte peripheral_address, byte register_address, byte data[], byte byte_count){
  Wire.beginTransmission(peripheral_address);
  Wire.write(register_address);
  Wire.endTransmission(false);
  Wire.requestFrom(peripheral_address, byte_count);
  
  byte p = 0;
  while(Wire.available() and p <= byte_count) {
    data[p++] = Wire.read();
  }
  Wire.endTransmission(true);
}

int read_word(byte peripheral_address, byte register_address){
  byte data[2];
  read_bytes(peripheral_address, register_address, data, 2);
  int data_word = data[0];
  data_word = (data_word << 8) | data[1];
  return data_word;
}

void write_byte(byte peripheral_address, byte register_address, byte value){
  Wire.beginTransmission(peripheral_address);
  Wire.write(register_address);
  Wire.write(value);
  Wire.endTransmission(true);
}

void write_bytes(byte peripheral_address, byte register_address, byte data[], byte byte_count){
  Wire.beginTransmission(peripheral_address);
  Wire.write(register_address);
  byte p = 0;
  while(p <= byte_count) {
    Wire.write(data[p++]);
  }
  Wire.endTransmission(true);
}

void write_word(byte peripheral_address, byte register_address, long value){
  byte data[2];
  data[0] = value >> 8;
  data[1] = value & 255;
  write_bytes(peripheral_address, register_address, data, 2);
}


/*
REG_STATUS = Register('REG_STATUS')
REG_SYS_SLEEP = Register('REG_SYS_SLEEP')  # System State (Sleep/Wake)
REG_LIGHTING_ON = Register('REG_LIGHTING_ON')  # Lighting On/Off
REG_LIGHTING_R = Register('REG_LIGHTING_R')   # Lighting Red
REG_LIGHTING_G = Register('REG_LIGHTING_G')  # Lighting Green
REG_LIGHTING_B = Register('REG_LIGHTING_B')  # Lighting Blue
REG_LIGHTING_TIME = Register('REG_LIGHTING_TIME')  # Lighting Blend Time
REG_MOTOR_T1 = Register('REG_MOTOR_T1')  # Motor Theta Steps MSB
REG_MOTOR_T2 = Register('REG_MOTOR_T2')  # Motor Theta Steps LSB
REG_MOTOR_R1 = Register('REG_MOTOR_R1')  # Motor Radius Steps MSB
REG_MOTOR_R2 = Register('REG_MOTOR_R2')  # Motor Radius Steps LSB

REG_INTERUPTS_0 = Register('REG_INTERUPTS_0')  # Interupts Bank0
REG_INTERUPTS_1 = Register('REG_INTERUPTS_1')  # Interupts Bank1
 */

const char raspberry_register_count = 4;

//const Register raspberry_registers[raspberry_register_count] = {
//  // REG_STATUS = Register(0);
//  Register(LIGHTING_SET_STATE, 2, 1), // REG_LIGHTING_ON
//  Register(LIGHTING_SET_COLOUR, 3, 3),  // REG_LIGHTING_COLOUR 
//  Register(LIGHTING_SET_BLEND_TIME, 6, 1),  // REG_LIGHTING_BLEND_TIME 
//  Register(MOTOR_SET, 7, 4) // REG_MOTOR 
// };

void push_settings() {
  byte peripheral_address = RASPBERRY_I2C_ADDRESS;
  byte register_address = 0;  
  
  Wire.beginTransmission(peripheral_address);
  Wire.write(register_address);

  // Status
  Wire.write(0);

  // Sleep/Wake
  Wire.write(1);

  // Lighting
  // Wire.write(get_lighting_state());
  Wire.write(1);
  
  // char *colors = get_lighting_color();
  unsigned char colors[3] = {0x22, 0xEE, 0x22};
  Wire.write(colors[0]);
  Wire.write(colors[1]);
  Wire.write(colors[2]);
  
  Wire.write(10); // Blend time

  // long t_steps = get_theta_motor_steps();
  byte motor_steps_lsb, motor_steps_msb;
  long t_steps = 500;
  motor_steps_lsb = t_steps & 255;
  motor_steps_msb = t_steps >> 8;
  Wire.write(motor_steps_msb);
  Wire.write(motor_steps_lsb);
  
  // long r_steps = get_radius_motor_steps();
  long r_steps = 300;
  motor_steps_lsb = r_steps & 255;
  motor_steps_msb = r_steps >> 8;
  Wire.write(motor_steps_msb);
  Wire.write(motor_steps_lsb);

  Wire.endTransmission(true);  
}

long poll_interupt() {
  byte peripheral_address = RASPBERRY_I2C_ADDRESS;
  byte register_address = 10;  
  return read_word(peripheral_address, register_address);
}

void clear_interupts() {
  byte peripheral_address = RASPBERRY_I2C_ADDRESS;
  byte register_address = 10;  
  write_word(peripheral_address, register_address, 0);
}

void i2c_controller_setup(){
  push_settings();
}

void i2c_controller_loop(){

  static unsigned int timer = 0;
  if(timer < millis()){
    send_raspberry_shutdown();
    timer = millis() + 5000;
  }
  
//  long interupt_state = poll_interupt();
//
//  // TODO: Can we spread out the updates over multiple frames.
//  
//  // Look the events we know about.
//  // Check if their bits have changed, if they have fire the events.
//  if (interupt_state == 0) {
//    return;
//  }
//  for (int i = 0; i < raspberry_register_count; ++i) {
//    // If we are all 0 we are done
//    if (interupt_state == 0) {
//      break;
//    }
//  
//    // Test this bit
//    Register reg_event = raspberry_registers[i];
//    long mask = reg_event.get_mask();
//    if (interupt_state & mask) {
//
//      byte byte_count = reg_event.get_bytes();
//      byte reg_buffer[byte_count];
//      if(byte_count > 1){
//        read_bytes(RASPBERRY_I2C_ADDRESS, reg_event.get_address(), reg_buffer, reg_event.get_bytes());
//      }
//      else {
//        reg_buffer[0] = read_byte(RASPBERRY_I2C_ADDRESS, reg_event.get_address());
//      }
//      
//      // Reset the bits
//      interupt_state &= ~mask;
//      
//      // Fire the event
//      evtManager.trigger(*reg_event.set_event, &reg_buffer);
//    }
//  }
//  
//  clear_interupts();
  
}

#endif // DISABLE_I2C_COMS
