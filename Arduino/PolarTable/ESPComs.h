/*************************************************************
  Talk with the WIFI
*************************************************************/
#pragma once

static const char reg_size = 10;
volatile uint8_t i2c_regs[reg_size];
volatile byte reg_position;
volatile long int i2c_reg_changed;

/*
void serialEvent3(){
  byte incoming_byte;
  if(Serial3.available()){
    incoming_byte = Serial3.read();
    byte is_read = incoming_byte >> 8;  // 1 MSB
    byte expected_length = (incoming_byte >> 4) & 15; // Next 3 bits
    reg_position = (incoming_byte & 31); // LSB 4 bits

    if(is_read){
      while(expected_length){
        Serial3.write(i2c_regs[reg_position++]);
        --expected_length;
      }
    }
    else{
      do
      {
        while(Serial3.available()){
          i2c_regs[reg_position] = Serial3.read();
          i2c_reg_changed |= 1 << reg_position++;
          --expected_length;
        }
        if(expected_length){
          delay(100);
          if(not Serial3.available())
          {
            // Timeout
            return;
          }
        }        
      } while(expected_length);
    }
  }
}

void sendToESP(byte value){
  Serial3.write(value);
}

void esp_setup(){
  Serial3.begin(115200);
}
*/
void setLed(byte brightness) {
  analogWrite(13, brightness);
}

int inc(int a) {
  return a + 1;
}

void esp_loop(){
  interface(
    inc, "inc: Increment a value. @a: Value. @return: a + 1.",
    setLed, "set_led: Set LED brightness. @brightness: Brightness.");
}
