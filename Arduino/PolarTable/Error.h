/*************************************************************
  Error LED
*************************************************************/
#pragma once


void error_LED(int code){
  while (true) 
  {
    for(unsigned int i = 0; i < (sizeof(code) * 8); ++i)
    {
      DEBUG_PRINT_VAR(i, code);
      digitalWrite(PIN_ERROR_LED, HIGH);
      delay((code & (1 << i)) ? 1000 : 500);
      digitalWrite(PIN_ERROR_LED, LOW);
      delay(500);
    }
    delay(3000);
  }
}

struct ErrorEventListener : public EventTask
{
  ErrorEventListener();
  
  using EventTask::execute;
  
  void execute(Event *evt)
  {
    error_LED(*(int*)evt->extra);
  }
};
ErrorEventListener::ErrorEventListener(){}

void error_setup(){
  ErrorEventListener error_event_listner = ErrorEventListener();
  evtManager.subscribe(Subscriber(ERROR_EVENT, &error_event_listner));
}
