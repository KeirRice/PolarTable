#include <EnableInterrupt.h>
#include <kEncoder.h>
using namespace kEncoder;

AbsoluteEncoder abs_encoder((PinCollectionInterface) PinBank<A8, A9, A10, A11>());
RelativeEncoder rel_encoder((PinCollectionInterface) PinGroup<A12, A13>());

void abs_encoder_isr(){
  abs_encoder.interputHandler();
}

void rel_encoder_isr(){
  rel_encoder.interputHandler();
}

void setup() {
  // put your setup code here, to run once:
  abs_encoder.setup(&abs_encoder_isr);
  rel_encoder.setup(&rel_encoder_isr);
}

void loop() {
  // put your main code here, to run repeatedly:
  Serial.print("Abs Position "); Serial.print(abs_encoder.position);
  Serial.print(" & Abs Direction "); Serial.print(abs_encoder.direction);
  Serial.println("");
  Serial.print("Steps "); Serial.print(rel_encoder.steps);
  Serial.print(" & Rel Direction "); Serial.print(rel_encoder.direction);
  Serial.println("");
}
