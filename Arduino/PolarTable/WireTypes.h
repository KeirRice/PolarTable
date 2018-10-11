#pragma once

union wire_long
{
  long u;
  byte uBytes[sizeof(long)];
};

union wire_int
{
  int u;
  byte uBytes[sizeof(int)];
};

struct motor
{
  union {
    struct              // component-wise representation
    {
      union {
        long long s;
        long long steps;
      };
    };
    char uBytes[sizeof(long long)];
  };
};
typedef motor Motor;
