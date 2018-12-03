void setup() {
  // put your setup code here, to run once:

  Serial.begin(115200);
  Serial3.begin(115200);
}

void loop() {
  // put your main code  here, to run repeatedly:
  while(Serial3.available()){
    Serial.write(Serial3.read());
  }
  while(Serial.available()){
    Serial3.write(Serial.read());
  }
}
