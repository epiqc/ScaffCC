#include "fn.h"

// st_product() implement the "star" product from alg A.2 
// Essentially, this product finds the reduced ideal "just to the right"
// of I \cdot J
//
//
rkqc st_product(ideal_t *res, ideal_t *i, ideal_t *j,
      int delta, double sdelta)
{
  // skipping line 1 of Alg A.2 beccause a and k are not defined
  // (I have email pending to Brian Matt)
  ideal_t tmp;
  product(&tmp, i, j, delta, sdelta);

  if (res->d <= tmp.d)
  {
    while (res->d <= tmp.d)
      rho(&tmp, &tmp, delta, sdelta);
    *res = tmp;
  }
  else
  {
    while (tmp->d >= tmp.d)
      rho_inv(&tmp, &tmp, delta, sdelta);
    rho(res, &tmp, delta, sdelta);
  }
}
