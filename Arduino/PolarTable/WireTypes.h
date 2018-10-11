#pragma once

template<typename T>
struct wire_packed
{
  union
  {
    T u;
    byte uBytes[sizeof(T)];
  };
  int size = sizeof(T);
};

template<typename T>
wire_packed<T> wire_pack(T t) {
  wire_packed<T> r;
  r.u = t;
  return r;
}

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
