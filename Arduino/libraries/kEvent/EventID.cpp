/*************************************************************
  EventID
*************************************************************/

#include "EventID.h"

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



bool EventID::operator> (const EventID  &f) const {
  return (long) * this >  (long)f;
};
bool EventID::operator>=(const EventID  &f) const {
  return (long) * this >=  (long)f;
};
bool EventID::operator==(const EventID  &f) const {
  return (long) * this ==  (long)f;
};
bool EventID::operator<=(const EventID  &f) const {
  return (long) * this <=  (long)f;
};
bool EventID::operator< (const EventID  &f) const {
  return (long) * this <  (long)f;
};
EventID EventID::operator| (const EventID  &f) const {
  return EventID((long) * this |  (long)f);
};
EventID EventID::operator& (const EventID  &f) const {
  return EventID((long) * this &  (long)f);
};

bool EventID::operator> (const long   &i) const {
  return (long) * this >  i;
};
bool EventID::operator>=(const long   &i) const {
  return (long) * this >= i;
};
bool EventID::operator==(const long   &i) const {
  return (long) * this == i;
};
bool EventID::operator<=(const long   &i) const {
  return (long) * this <= i;
};
bool EventID::operator< (const long   &i) const {
  return (long) * this <  i;
};
long EventID::operator| (const long   &i) const {
  return (long) *this |  i;
};
long EventID::operator& (const long   &i) const {
  return (long) *this &  i;
};

EventID::operator long () const {
  return ((long)id[2] << 16) | ((long)id[1] << 8) | ((long)id[0] << 0);
};
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
