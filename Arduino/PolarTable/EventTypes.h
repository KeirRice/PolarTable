/*************************************************************
  Base types for events
*************************************************************/
#pragma once

#include <Flash.h>

#define ENABLE_DEBUG_STRINGS 0

// EventID number - 0 to 6
// ActionID - 7
// StateID - 8
// SystemID - 9 to 24

static long event_id_mask =   0b00000000000000000000000001111111;
static long action_id_mask =  0b00000000000000000000000010000000;
static long state_id_mask =   0b00000000000000000000000100000000;
static long system_id_mask =  0b11111111111111111111111000000000;

int event_id_counter = 0;
int system_id_offset = 6;

// EventID
//  * Group
//  * Action
//  * State

typedef struct EventID {

  protected:
    unsigned char id[3];

#if ENABLE_DEBUG_STRINGS == 1
    _FLASH_STRING *debug_name = NULL;
#endif

  public:
    EventID();
    EventID(const long _event_id);
    EventID(const int _event_id);

#if ENABLE_DEBUG_STRINGS == 1
    EventID(const long _event_id, _FLASH_STRING &debug_name);
    EventID(const int _event_id, _FLASH_STRING &debug_name);  
#endif  
  
    bool operator> (const EventID  &f) const {
      return (long) * this >  (long)f;
    };
    bool operator>=(const EventID  &f) const {
      return (long) * this >=  (long)f;
    };
    bool operator==(const EventID  &f) const {
      return (long) * this ==  (long)f;
    };
    bool operator<=(const EventID  &f) const {
      return (long) * this <=  (long)f;
    };
    bool operator< (const EventID  &f) const {
      return (long) * this <  (long)f;
    };
    bool operator| (const EventID  &f) const {
      return (long) * this |  (long)f;
    };
    bool operator& (const EventID  &f) const {
      return (long) * this &  (long)f;
    };

    bool operator> (const long   &i) const {
      return (long) * this >  i;
    };
    bool operator>=(const long   &i) const {
      return (long) * this >= i;
    };
    bool operator==(const long   &i) const {
      return (long) * this == i;
    };
    bool operator<=(const long   &i) const {
      return (long) * this <= i;
    };
    bool operator< (const long   &i) const {
      return (long) * this <  i;
    };
    bool operator| (const long   &i) const {
      return (long) * this |  i;
    };
    bool operator& (const long   &i) const {
      return (long) * this &  i;
    };

    operator long () const {
      return ((long)id[2] << 16) | ((long)id[1] << 8) | ((long)id[0] << 0);
    };
    //    operator int () const {
    //      assert(id[2] == 0); // If we are going to loose data be noisy.
    //      return (id[1] << 8) | (id[0] << 0);
    //    };

} EventID;

#if ENABLE_DEBUG_STRINGS == 1
EventID::EventID(const long _event_id) : debug_name(NULL){
  char* pChar;
  pChar = (char*)&_event_id;

  id[0] = pChar[0];
  id[1] = pChar[1];
  id[2] = pChar[2];
};
EventID::EventID(const int _event_id) : debug_name(NULL){
  char* pChar;
  pChar = (char*)&_event_id;

  id[0] = pChar[0];
  id[1] = pChar[1];
  id[2] = 0;
};

EventID::EventID(const long _event_id, _FLASH_STRING &_debug_name) : debug_name(&_debug_name) {
  char* pChar;
  pChar = (char*)&_event_id;

  id[0] = pChar[0];
  id[1] = pChar[1];
  id[2] = 0;
};
EventID::EventID(const int _event_id, _FLASH_STRING &_debug_name) : debug_name(&_debug_name) {
  char* pChar;
  pChar = (char*)&_event_id;

  id[0] = pChar[0];
  id[1] = pChar[1];
  id[2] = 0;
};
#else
EventID::EventID(const long _event_id){
  char* pChar;
  pChar = (char*)&_event_id;

  id[0] = pChar[0];
  id[1] = pChar[1];
  id[2] = pChar[2];
};
EventID::EventID(const int _event_id){
  char* pChar;
  pChar = (char*)&_event_id;

  id[0] = pChar[0];
  id[1] = pChar[1];
  id[2] = 0;
};
#endif // ENABLE_DEBUG_STRINGS


bool operator> (const long &i, const EventID &f) {
  return i >  (long)f;
};
bool operator>=(const long &i, const EventID &f) {
  return i >= (long)f;
};
bool operator==(const long &i, const EventID &f) {
  return i == (long)f;
};
bool operator<=(const long &i, const EventID &f) {
  return i <= (long)f;
};
bool operator< (const long &i, const EventID &f) {
  return i <  (long)f;
};
bool operator| (const long &i, const EventID &f) {
  return i |  (long)f;
};
bool operator& (const long &i, const EventID &f) {
  return i &  (long)f;
};


typedef struct SystemID : public EventID {
  SystemID(const int _bit_offset): EventID(1 << _bit_offset) {}
#if ENABLE_DEBUG_STRINGS == 1
  SystemID(const int _bit_offset, _FLASH_STRING &_debug_name): EventID(1 << _bit_offset, _debug_name) {}
#endif
} SystemID;


/*
Macros for setting up all of the global event variables.
 t is the variable type you want (StateID, ActionID, etc)
 a is the name of the variable that will get created as well as the debug string (if enabled);

The debug strings are all stored in PROGMEM.
*/
#if DEBUG == 1 && ENABLE_DEBUG_STRINGS == 1
#define BuildEvent(t, a) \
  FLASH_STRING(prog_##a, #a); \
  t a(event_id_counter++, prog_##a); 
#define BuildSystem(t, a) \
  FLASH_STRING(prog_##a, #a); \
  t a(system_id_offset++, prog_##a); 
#else
#define BuildEvent(t, a) \
  t a(event_id_counter++)
#define BuildSystem(t, a) \
  t a(system_id_offset++)
#endif // DEBUG == 1

// Two bits to seperate the actions from the states if we need to.
typedef SystemID EventType;
BuildSystem(EventType, ACTION);
BuildSystem(EventType, STATE);

typedef struct ActionID : public EventID {
  ActionID(const long _event_id): EventID(_event_id | ACTION) {}
#if ENABLE_DEBUG_STRINGS == 1
  ActionID(const long _event_id, _FLASH_STRING &_debug_name): EventID(_event_id | ACTION, _debug_name) {}  
#endif
} ActionID;

typedef struct StateID : public EventID {
  StateID(const long _event_id): EventID(_event_id | STATE) {}
#if ENABLE_DEBUG_STRINGS == 1
  StateID(const long _event_id, _FLASH_STRING &_debug_name): EventID(_event_id | ACTION, _debug_name) {}  
#endif
} StateID;
