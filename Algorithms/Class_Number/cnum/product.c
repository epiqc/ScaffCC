#include "fn.h"

// implements the product of two ideals (which may not be reduced)
//
// out:    the product of i1 and i2
// i1, i2: multiplier, multiplicand
// delta:  \Delta
// sdelta: sqrt of \Delta
//
rkqc product(ideal_t *out, ideal_t *i1, ideal_t *i2, int delta, double sdelta)
{
  // ignoring line 1 since I_1 and I_2 are not used subsequently
  //
  int kh, uh, vh, k, x, w, t;

  kh = gcd(i1->a, i2->a, &uh, &vh);
  k = gcd(kh, (i1->b + i2->b)/2, &x, &w);
  out->a = (i1->a * i2->a / k) / k;

  t = (   x * uh * i1->a * i2->b 
	+ x * vh * i2->a * i1->b 
	+ (w * i1->b * i2->b + delta)/2
      );

  out->b = t % (2 * out->a);
  if (out->a > sdelta) 
    out->b -= out->a - 1;
  else
    out->b += sdelta  - 2*out->a + 1;

  // unsure about how this is used (from g-hat and st_product)
  out->k = k;
}
