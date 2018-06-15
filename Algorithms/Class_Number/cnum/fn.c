#include "fn.h"

// fn is the main entry point for the oracle function that implements f_N,
// the oracle function which enables approximation of the regulator
//
// bound: i/N + jp/L (see alg A.3 in GFI)
// delta: \Delta (the input to the class number alg)
//
//
rkqc fn(int bound, int delta, ideal_t *j, int *dist)
{
  // used a lot, so compute once
  double sdelta = sqrt(delta);

  // a reduced ideal is represented by a pair of integers a, b where
  // I = Z + (b+sqrt(Delta))/2a Z
  // 
  // We start with O (the ring of integers) which is an ideal of O and has
  // delta(O) = 0; O is represented by a=1, b=1

  // j holds all ideals as we search; j[0] is O
  //
  ideal_t j[JMAX];
  ideal_t jstar, jtemp;

  j[0].a = 1;  j[0].b = 1;  j[0].k = 1;  j[0].d = 0.0;

  // run rho() on it twice, always tracking its delta
  rho(&j[1], &j[0], delta, sdelta);
  rho(&j[2], &j[1], delta, sdelta);

  int k = 1;
  while (j[k].d <= bound)
  {
    st_product(&jtemp, &j[k], &jstar, delta, sdelta);
    k++;
  }

  jstar = j[k-1];
  k -= 2;

  while (k > 0)
  {
    st_product(&jtemp, &j[k], &jstar, delta, sdelta);
    if (jtemp.d <= bound)
      jstar = jtemp;
    k--;
  }

  while (jstar.d < bound)
  {
    rho(&jstar, &jstar, delta, sdelta);
    jtemp = jstar;
    rho(&jstar, &jstar, delta, sdelta);
  }

  if (jtemp.d >= bound)
    *j = jtemp;
  else
    *j = jstar;

  *dist = floor(N * (bound - j->d));
}
