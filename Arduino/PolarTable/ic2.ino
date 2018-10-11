//#include "Wire.h"
//
//#define SLAVE_ADDRESS 0x08
//
//byte b[4] = {99, 223, 244};
//// data buffer
//int data[9];
//
//void receiveData(int byteCount){
//  int counter = 0;
//  while(Wire.available()) {
//      data[counter] = Wire.read();
//      Serial.print("Got data: ");
//      Serial.println(data[counter]);
//      counter ++;
//  }
//}
//
//void sendData(){
//    Wire.write(b, 4);
//    Serial.println("data sent");
//}
//
//void setup(){
//  Serial.begin(9600); // start serial for output
//  Wire.begin(SLAVE_ADDRESS);
//  Wire.onReceive(receiveData);
//  Wire.onRequest(sendData);
//  Serial.println("I2C Ready!");
//}
//
//void loop(){
//  delay(1000);
//}
//
//
//
///////
//
//
//void Send()
//{
//  Wire.beginTransmission( (uint8_t) MMA8451Q_Address );
//  Wire.write( (uint8_t) CTRL_REG1 );
//  Wire.endTransmission( false );
//  
//  Wire.requestFrom( (uint8_t) MMA8451Q_Address, (uint8_t) 1 , (uint8_t) true );
//  
//  if ( Wire.available() >= 1 )
//  {
//    int result = Wire.read() ;
//    Serial.println( result, HEX );
//  }
//}
//
//
///////
//
//
//#include <Wire.h>
//
//#define SLAVE_ADDRESS 0x68
//byte x = 0x00;
//byte simulated_registers[120];
//byte register_address;
//void setup()
//{
//  simulated_registers[0x75] = 0x68;   // fill the who_am_i
//  Wire.begin(SLAVE_ADDRESS);
//  Wire.onReceive(receiveEvent);
//  Wire.onRequest(requestEvent);  
//}
//
//void loop()
//{
//}
//
//void requestEvent() 
//{
//  Wire.write( (byte *) &simulated_registers[register_address], 32);
//}
//
//void receiveEvent(int bytes)
//{
//  if( bytes > 0)
//  {
//    register_address = Wire.read();
//  }
//}
