#include "fn.h"

// builds the ideal I, the product of the generators classically produced
// for phase II
//
// out: the output ideal I
// g[]: the input generators
// p[]: corresponding powers of g[]
// k:   number of ideals in g[]
//
rkqc ghat(ideal_t *out, ideal_t g[], int p[], int k,
    int delta, double sdelta)
{
  ideal_t   a, t;
  int       i, j;

  a->a = 1;
  a->b = 1;
  a->k = 1;
  a->delta = 1.0;

  for (i=0; i < k; i++)
  {
    // compute powers first
    for (j=0; j < p[i]; j++)
      product(&a, &a, &g[i], delta, sdelta);
  }

  // now ensure a is reduced
  //
  while (a->k != 1) 
    rho(&a, &a, delta, sdelta)

  *out = a;
}
