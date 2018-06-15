// Computes the \tau() function defined in Jozsa (and probably elsewhere)
// defined as implicitly taking \Delta (the fundamental discriminant of our
// real quadratic number field) and computing tau(b, a) = b mod 2a where
// the value is either in (\sqrt{\Delta}-2a, \sqrt{\Delta}]
// if a < \sqrt{\Delta} or in (-a, a] if a is >= \sqrt{Delta}  (see GFI pg 2)
//
// This function is used to normalize ideals to their unique
// canonical representation 
//
// Note: we assume that Scaffold's % operator preserves the sign of the
//   dividend (like C99 and Java, but unlike Python and VHDL)
//
// a, b : inputs (note that b can and often will be negative)
// sdelta: sqrt{\Delta} (Delta is the overall input to the class number alg)
//
rkqc tau(int b, int a, int sdelta)
{
  int   ret;

  ret = b % (2*a);

  if (a > sdelta)
  {
    while (ret < sdelta - 2*a)
      ret += 2*a;
    while (ret > sdelta)
      ret -= 2*a;
  }
  else
  {
    while (ret <= -a)
      ret += 2*a;
    while (ret > a)
      ret -= 2*a;
  }
  
  return ret;
}

