/*************************************************************
  Error LED
*************************************************************/

void error_listener(void *data){
  digitalWrite(PIN_ERROR_LED, HIGH);

  // TODO: Pulse the switch LED as well (If we can)
}

void error_setup(){
  pinMode(PIN_ERROR_LED, OUTPUT);
  digitalWrite(PIN_ERROR_LED, LOW);  
  evtManager.subscribe(Subscriber(ERROR_EVENT, error_listener));  
}
