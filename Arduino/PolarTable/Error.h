/*************************************************************
  Error LED
*************************************************************/
#pragma once

void error_listener(void *data){
  UNUSED(data);
  // digitalWrite(PIN_ERROR_LED, HIGH);

  // TODO: Pulse the switch LED as well (If we can)
}


void error_LED(void *data); // Foward declear, the body is in ButtonLED

struct ErrorEventListener : public EventTask
{
  ErrorEventListener();
  
  using EventTask::execute;
  
  void execute(Event *evt)
  {
    error_LED(evt->extra);
  }
};
ErrorEventListener::ErrorEventListener(){}

void error_setup(){
  // pinMode(PIN_ERROR_LED, OUTPUT);
  // digitalWrite(PIN_ERROR_LED, LOW);  
  // evtManager.subscribe(Subscriber(ERROR_EVENT, error_listener));  
}

