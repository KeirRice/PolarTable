/*************************************************************
Debug functions that can be compiled out on release
*************************************************************/

#pragma once

#define DEBUG_PRINT(...) \
            do { \
              if (DEBUG){ \
                Serial.print(F("DEBUG: ")); \
                Serial.print(__VA_ARGS__); \
                Serial.flush(); \
              }\
            } while (0)

#define DEBUG_PRINT_VAR1(a) \
            do { \
              if (DEBUG){ \
                Serial.print(#a ":\t"); Serial.println(a); \
                Serial.flush(); \
              }\
            } while (0)
#define DEBUG_PRINT_VAR2(a, b) \
            do { \
              if (DEBUG){ \
                Serial.print(#a ":\t"); Serial.print(a); Serial.print('\t'); \
                Serial.print(#a ":\t"); Serial.println(b); \
                Serial.flush(); \
              }\
            } while (0)
#define DEBUG_PRINT_VAR3(a, b, c) \
            do { \
              if (DEBUG){ \
                Serial.print(#a ":\t"); Serial.print(a); Serial.print('\t'); \
                Serial.print(#a ":\t"); Serial.print(b); Serial.print('\t'); \
                Serial.print(#a ":\t"); Serial.println(c); \
                Serial.flush(); \
              }\
            } while (0)
              
#define GET_MACRO(_1, _2, _3, NAME, ...) NAME
#define FOO(...) GET_MACRO(__VA_ARGS__, DEBUG_PRINT_VAR3, DEBUG_PRINT_VAR2, DEBUG_PRINT_VAR1)(__VA_ARGS__)

#define DEBUG_PRINTLN(...) \
            do { \
              if (DEBUG){ \
                Serial.print(F("DEBUG: ")); \
                Serial.println(__VA_ARGS__); \
                Serial.flush(); \
              }\
            } while (0)

#define DEBUG_WHERE() \
            do { \
              if (DEBUG){ \
                Serial.print(F("At "));  Serial.print(__FILE__); Serial.print(F(": ")); Serial.println(__LINE__); \
                Serial.flush(); \
              }\
            } while (0)
