#include "fn.h"

// \rho in the Class Number algorithm steps between ideals and has various
// guarantees about how much and how little it steps, which equivalence 
// class it resides in, how the delta() function reacts to it, etc
// This is all documented in the GFI
//
// \rho is defined on pg 3 of the GFI as \rho(I) = I/\gamma(I), but we
// take a simpler approach here based on the fact that 4a divides \Delta-b^2
// (GFI pg 2) and some algebra.  Here's the idea: let c = (\Delta-b^2)/4a,
// then a' = c and b' = \tau(b, a') in \rho(I).
//
// This function also updates the ideal's delta() value
// The formula is
//   \delta(\rho(I)) = ln abs(\overline(\gamma(I))/\gamma(I)) + \delta(I)
// from app A.1 in GFI;  note that this simplifies (see code)
//
// in:      the input ideal
// out:     \rho(in) is stored here, with its delta() value
// delta:   \Delta  (input to class number algorithm)
// sdelta:  \sqrt{\Delta}
//
rkqc rho(ideal_t *out, ideal_t *in, int delta, double sdelta)
{
  out->a = (abs(delta - in->b*in->b))/(4*in->a);
  out->b = tau(-in->b, out->a);
  out->d = log(fabs((in->b - sdelta)/(in->b + sdelta))) + in->d;
}

// rho^{-1} is sigma(rho(sigma(I))) where sigma is the algebraic conjugate
//
// The GFI does not specify how to compute rho^{-1}, so I did the algebra
// by hand.  It should probably be double-checked by someone (note: I just
// found parts of Jozsa's paper that back up part of my implementation)
//
rkqc rho_inv(ideal_t *out, ideal_t *in, int delta, double sdelta)
{
  int bs = tau(-in->b, in->a);

  out->a = (delta - bs*bs)/(4*in->a);
  out->b = tau(bs, out->a);
  out->d = -log(fabs((out->b - sdelta)/(out->b + sdelta))) + out->d;
}
