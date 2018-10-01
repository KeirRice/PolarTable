#ifndef _TEMPLATE_HELPERS_H    // Put these two lines at the top of your file.
#define _TEMPLATE_HELPERS_H    // (Use a suitable name, usually based on the file name.)

template<typename T, typename T2, typename T3>
void bitsWrite(T &x, const T2 &n, const T3 b, const int w)
/* Extention to bitWrite that supports more than one bit getting set at a time.
x: the numeric variable to which to write
n: which bit of the number to write, starting at 0 for the least-significant (rightmost) bit
b: the value to write to the bits
w: the width of the in bits of b
*/
{
  int mask = ((1 << (w + 1)) - 1) << b; // Build a mask w wide, move it into position b
  x = (x & ~mask) | ((n << b) & mask); // Clear the bits under mask and apply the value
}

#endif //_TEMPLATE_HELPERS_H
