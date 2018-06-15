#include "fn.h"

// h computes the h_I oracle
//
// bound: b/N + jhatp/L (see alg A.6 in GFI)
// delta: \Delta (the input to the class number alg)
//
//
rkqc h(int a, int b, int bound, int delta, double sdelta,
	    ideal_t *i, ideal_t *out, int *dist)
{
  ideal_t j, k;
  int li, dst;

  j = *i;
  k.a = 1;  k.b = 1;  k.k = 1;  k.d = 0.0;
  li = a;

  while (k.k != 1)
  {
    if (i % 2 == 1)
      st_product(&k, &k, &j, delta, sdelta);
    st_product(&j, &j, &j, delta, sdelta);
  }
  fn(bound, delta, &j, &dst);
  st_product(&k, &k, &j, delta, sdelta);

  *out = *k;
  *dist = floor(N * k.d);
}
