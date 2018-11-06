/*************************************************************
  Base types for events
*************************************************************/

#pragma once

// EventID number - 0 to 6
// ActionID - 7
// StateID - 8
// SystemID - 9 to 15

long event_id_mask =   0b00000000000000000000000001111111;
long action_id_mask =  0b00000000000000000000000010000000;
long state_id_mask =   0b00000000000000000000000100000000;
long system_id_mask =  0b00000000000000011111111000000000;

// EventID
//  * Group
//  * Action
//  * State

typedef struct EventID {

  protected:
    unsigned char id[3];

#if DEBUG == 1
    const char* debug_name;
#endif // DEBUG

  public:
    EventID();
    EventID(const long _event_id);
    EventID(const int _event_id);
    EventID(const long _event_id, char* _debug_name);
    EventID(const int _event_id, char* _debug_name);

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

#if DEBUG == 1
    operator const char* () {
      return debug_name;
    };
#endif // DEBUG

} EventID;

EventID::EventID(const long _event_id) {
  char* pChar;
  pChar = (char*)&_event_id;

  id[0] = pChar[0];
  id[1] = pChar[1];
  id[2] = pChar[2];
};
EventID::EventID(const int _event_id) {
  char* pChar;
  pChar = (char*)&_event_id;

  id[0] = pChar[0];
  id[1] = pChar[1];
  id[2] = 0;
};

#if DEBUG == 1
EventID::EventID(const long _event_id, char* _debug_name) : debug_name(_debug_name) {
  char* pChar;
  pChar = (char*)&_event_id;

  id[0] = pChar[0];
  id[1] = pChar[1];
  id[2] = pChar[2];
};
EventID::EventID(const int _event_id, char* _debug_name) : debug_name(_debug_name) {
  char* pChar;
  pChar = (char*)&_event_id;

  id[0] = pChar[0];
  id[1] = pChar[1];
  id[2] = 0;
};
#endif // DEBUG


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



typedef EventID EventType;
// bit 8 state vs action
EventType ACTION(1 << 7);
EventType STATE(2 << 7);

typedef struct ActionID : public EventID {
  ActionID(const long _event_id): EventID(_event_id | ACTION) {}
  ActionID(const long _event_id, char* debug_name): EventID(_event_id | ACTION, debug_name) {}
} ActionID;

typedef struct StateID : public EventID {
  StateID(const long _event_id): EventID(_event_id | STATE) {}
  StateID(const long _event_id, char* debug_name): EventID(_event_id | STATE, debug_name) {}
} StateID;

typedef struct SystemID : public EventID {
  SystemID(const int _bit_offset): EventID(1 << _bit_offset) {}
  SystemID(const int _bit_offset, char* debug_name): EventID(1 << _bit_offset, debug_name) {}
} SystemID;

// high bits for flags
int system_id_offset = 8;
SystemID GLOBAL = SystemID(system_id_offset++);
SystemID SYSTEM = SystemID(system_id_offset++);
SystemID MOVEMENT = SystemID(system_id_offset++);
SystemID REL_MOVEMENT = SystemID(system_id_offset++);
SystemID ABS_MOVEMENT = SystemID(system_id_offset++);
SystemID MOTOR = SystemID(system_id_offset++);
SystemID MOTOR_THETA = SystemID(system_id_offset++);
SystemID MOTOR_RADIUS = SystemID(system_id_offset++);
SystemID LIGHTING = SystemID(system_id_offset++);
SystemID BUTTON = SystemID(system_id_offset++);
SystemID COMMUNICATION = SystemID(system_id_offset++);
SystemID ERROR = SystemID(system_id_offset++);
SystemID RASPBERRY = SystemID(system_id_offset++);
