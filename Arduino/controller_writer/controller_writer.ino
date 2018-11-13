// Wire Controller Writer
// by Nicholas Zambetti <http://www.zambetti.com>

// Demonstrates use of the Wire library
// Writes data to an I2C/TWI slave device
// Refer to the "Wire Slave Receiver" example for use with this

// Created 29 March 2006

// This example code is in the public domain.


#include <Wire.h>

void setup() {
  Serial.begin(19200);      // open the serial port at 9600 bps:    
  Wire.begin(); // join i2c bus (address optional for master)
}

byte x = 1;
int RPI = 0x14;
int REG_LIGHTING_ON = 0x02;

void loop() {
  Wire.beginTransmission(RPI); // transmit to device
  Wire.write(REG_LIGHTING_ON);  // Set Register
  Wire.write(x);  // Set Value
  Wire.endTransmission();    // stop transmitting
  delay(3);
  
  Wire.beginTransmission(RPI); // transmit to device
  Wire.write(REG_LIGHTING_ON);  // Set Register
  Wire.endTransmission(false);  // Send data, restart connection
  byte y = Wire.requestFrom(RPI, 1);    // Request 1 byte

  Serial.print("Sent: ");
  Serial.print(x, BIN);
  Serial.print(" Received: ");
  Serial.println(y, BIN);

  x++;

  delay(1000);
}
