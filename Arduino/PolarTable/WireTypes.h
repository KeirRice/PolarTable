#ifndef _WIRE_TYPES_H    // Put these two lines at the top of your file.
#define _WIRE_TYPES_H    // (Use a suitable name, usually based on the file name.)

// Place your main header code here.
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
        long s;
        long steps;
      };
    };
    char uBytes[sizeof(long)];
  };
};
typedef motor motor;


// struct wire_color
// {
//   union {
//     struct              // component-wise representation
//     {
//       union {
//         unsigned char r;
//         unsigned char red;
//       };
//       union {
//         unsigned char b;
//         unsigned char blue;
//       };
//       union {
//         unsigned char g;
//         unsigned char green;
//       };
//     };
//     char uBytes[sizeof(int)];
//     unsigned int c;          // packed representation
//   };
// };


#endif // _WIRE_TYPES_H