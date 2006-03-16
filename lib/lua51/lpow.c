/* lpow.c
 *
 * Copyright Adam Goode, 2006
 * Licensed under the same terms as Lua 5.1
 */

#include "lpow.h"

/* does O(log n) pow on integers, using Russian Peasant-like algorithm */
long lpow(long a, long b) {
  long result = 1;
  
  if (b < 0) {
    return 0;
  }

  while (b > 0) {
    if ((b & 1) == 0) {   /* even */
      /* a^(2b) == a * a^(b) */
      a *= a;
      b >>= 1;
    } else {              /* odd */
      /* take the remainder, and put it towards the result */
      result *= a;
      b--;
    }
  }

  return result;
}
