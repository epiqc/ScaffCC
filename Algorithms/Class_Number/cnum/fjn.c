#include "fn.h"

// fjn is the main entry point for the oracle function that implements f_J,N,
// the oracle function which enables approximation of the regulator
//
// bound: i/N + jp/L (see alg A.3 in GFI)
// delta: \Delta (the input to the class number alg)
// sdelta: floor(sqrt(delta))
//
//
rkqc fjn(int bound, int delta, int sdelta,
	      ideal_t *j, ideal_t *out, int *dist)
{
  ideal_t i, k, ktemp;

  i = *j;
  fn(bound, delta, i, dist);
  product(&k, &i, j, delta, sdelta);

  while (k.k != 1)
  {
    rho(&k, &k, delta, sdelta);
  }

  if (k.d <= bound)
  {
    while (k.d < bound)
      rho(&k, &k, delta, sdelta);
  }
  else
  {
    while (k.d >= bound)
      rhoinv(&k, &k, delta, sdelta);

    rho(&k, &k, delta, sdelta);
  }

  *out = k;
  *dist = floor(N * (bound - j->d));
}
