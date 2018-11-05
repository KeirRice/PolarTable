/*************************************************************
  Base types for events
*************************************************************/

#pragma once

// EventID number - 0 to 6
// ActionID - 7
// StateID - 8
// SystemID - 9 to 15

int event_id_mask =   0b0000000001111111;
int action_id_mask =  0b0000000010000000;
int state_id_mask =   0b0000000100000000;
int system_id_mask =  0b1111111000000000;

// EventID
//  * Group
//  * Action
//  * State

typedef struct EventID{

  public:
    EventID();
    EventID(const int _event_id);
  
  int id;

  bool operator> (const EventID  &f) const {return id >  f.id;};
  bool operator>=(const EventID  &f) const {return id == f.id;};
  bool operator==(const EventID  &f) const {return id == f.id;};
  bool operator<=(const EventID  &f) const {return id <= f.id;};
  bool operator< (const EventID  &f) const {return id <  f.id;};
  bool operator| (const EventID  &f) const {return id |  f.id;};
  bool operator& (const EventID  &f) const {return id &  f.id;};

  bool operator> (const int   &i) const {return id >  i;};
  bool operator>=(const int   &i) const {return id >= i;};
  bool operator==(const int   &i) const {return id == i;};
  bool operator<=(const int   &i) const {return id <= i;};
  bool operator< (const int   &i) const {return id <  i;};
  bool operator| (const int   &i) const {return id |  i;};
  bool operator& (const int   &i) const {return id &  i;};
  operator int () const {return id;};
  
} EventID;
EventID::EventID(const int _event_id) : id(_event_id){};


bool operator> (const int &i, const EventID &f) {return i >  f.id;};
bool operator>=(const int &i, const EventID &f) {return i >= f.id;};
bool operator==(const int &i, const EventID &f) {return i == f.id;};
bool operator<=(const int &i, const EventID &f) {return i <= f.id;};
bool operator< (const int &i, const EventID &f) {return i <  f.id;};
bool operator| (const int &i, const EventID &f) {return i |  f.id;};
bool operator& (const int &i, const EventID &f) {return i &  f.id;};

typedef EventID EventType;
// bit 8 state vs action
EventType ACTION(1 << 7);
EventType STATE(2 << 7);

typedef struct ActionID : public EventID{
  ActionID(const int _event_id): EventID(_event_id | ACTION){}
} ActionID;

typedef struct StateID : public EventID{
  StateID(const int _event_id): EventID(_event_id | STATE){}
} StateID;

typedef EventID SystemID;
// 7 high bits for system
int system_id_base = 1 << 8;
SystemID GLOBAL = SystemID(system_id_base << 1);
SystemID SYSTEM = SystemID(system_id_base << 2);
SystemID MOVEMENT = SystemID(system_id_base << 3);
SystemID LIGHTING = SystemID(system_id_base << 4);
SystemID BUTTON = SystemID(system_id_base << 5);
SystemID COMMUNICATION = SystemID(system_id_base << 6);
SystemID ERROR = SystemID(system_id_base << 7);
SystemID RASPBERRY = SystemID(system_id_base << 8);
