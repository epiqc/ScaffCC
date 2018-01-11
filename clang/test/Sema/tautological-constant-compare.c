// RUN: %clang_cc1 -triple x86_64-linux-gnu -fsyntax-only -DTEST -verify %s
// RUN: %clang_cc1 -triple x86_64-linux-gnu -fsyntax-only -Wno-tautological-constant-compare -verify %s
// RUN: %clang_cc1 -triple x86_64-linux-gnu -fsyntax-only -DTEST -verify -x c++ %s
// RUN: %clang_cc1 -triple x86_64-linux-gnu -fsyntax-only -Wno-tautological-constant-compare -verify -x c++ %s

int value(void);

#define macro(val) val

#ifdef __cplusplus
template<typename T>
void TFunc() {
  // Make sure that we do warn for normal variables in template functions !
  unsigned char c = value();
#ifdef TEST
  if (c > 255) // expected-warning {{comparison 'unsigned char' > 255 is always false}}
      return;
#else
  if (c > 255)
      return;
#endif

  if (c > macro(255))
      return;

  T v = value();
  if (v > 255)
      return;
  if (v > 32767)
      return;
}
#endif

int main()
{
#ifdef __cplusplus
  TFunc<unsigned char>();
  TFunc<signed short>();
#endif

  short s = value();

#ifdef TEST
  if (s == 32767)
      return 0;
  if (s != 32767)
      return 0;
  if (s < 32767)
      return 0;
  if (s <= 32767) // expected-warning {{comparison 'short' <= 32767 is always true}}
      return 0;
  if (s > 32767) // expected-warning {{comparison 'short' > 32767 is always false}}
      return 0;
  if (s >= 32767)
      return 0;

  if (32767 == s)
      return 0;
  if (32767 != s)
      return 0;
  if (32767 < s) // expected-warning {{comparison 32767 < 'short' is always false}}
      return 0;
  if (32767 <= s)
      return 0;
  if (32767 > s)
      return 0;
  if (32767 >= s) // expected-warning {{comparison 32767 >= 'short' is always true}}
      return 0;

  // FIXME: assumes two's complement
  if (s == -32768)
      return 0;
  if (s != -32768)
      return 0;
  if (s < -32768) // expected-warning {{comparison 'short' < -32768 is always false}}
      return 0;
  if (s <= -32768)
      return 0;
  if (s > -32768)
      return 0;
  if (s >= -32768) // expected-warning {{comparison 'short' >= -32768 is always true}}
      return 0;

  if (-32768 == s)
      return 0;
  if (-32768 != s)
      return 0;
  if (-32768 < s)
      return 0;
  if (-32768 <= s) // expected-warning {{comparison -32768 <= 'short' is always true}}
      return 0;
  if (-32768 > s) // expected-warning {{comparison -32768 > 'short' is always false}}
      return 0;
  if (-32768 >= s)
      return 0;

  if (s == 32767UL)
      return 0;
  if (s != 32767UL)
      return 0;
  if (s < 32767UL)
      return 0;
  if (s <= 32767UL) // expected-warning {{comparison 'short' <= 32767 is always true}}
      return 0;
  if (s > 32767UL) // expected-warning {{comparison 'short' > 32767 is always false}}
      return 0;
  if (s >= 32767UL)
      return 0;

  if (32767UL == s)
      return 0;
  if (32767UL != s)
      return 0;
  if (32767UL < s) // expected-warning {{comparison 32767 < 'short' is always false}}
      return 0;
  if (32767UL <= s)
      return 0;
  if (32767UL > s)
      return 0;
  if (32767UL >= s) // expected-warning {{comparison 32767 >= 'short' is always true}}
      return 0;

  // FIXME: assumes two's complement
  if (s == -32768L)
      return 0;
  if (s != -32768L)
      return 0;
  if (s < -32768L) // expected-warning {{comparison 'short' < -32768 is always false}}
      return 0;
  if (s <= -32768L)
      return 0;
  if (s > -32768L)
      return 0;
  if (s >= -32768L) // expected-warning {{comparison 'short' >= -32768 is always true}}
      return 0;

  if (-32768L == s)
      return 0;
  if (-32768L != s)
      return 0;
  if (-32768L < s)
      return 0;
  if (-32768L <= s) // expected-warning {{comparison -32768 <= 'short' is always true}}
      return 0;
  if (-32768L > s) // expected-warning {{comparison -32768 > 'short' is always false}}
      return 0;
  if (-32768L >= s)
      return 0;
#else
  // expected-no-diagnostics
  if (s == 32767)
    return 0;
  if (s != 32767)
    return 0;
  if (s < 32767)
    return 0;
  if (s <= 32767)
    return 0;
  if (s > 32767)
    return 0;
  if (s >= 32767)
    return 0;

  if (32767 == s)
    return 0;
  if (32767 != s)
    return 0;
  if (32767 < s)
    return 0;
  if (32767 <= s)
    return 0;
  if (32767 > s)
    return 0;
  if (32767 >= s)
    return 0;

  // FIXME: assumes two's complement
  if (s == -32768)
    return 0;
  if (s != -32768)
    return 0;
  if (s < -32768)
    return 0;
  if (s <= -32768)
    return 0;
  if (s > -32768)
    return 0;
  if (s >= -32768)
    return 0;

  if (-32768 == s)
    return 0;
  if (-32768 != s)
    return 0;
  if (-32768 < s)
    return 0;
  if (-32768 <= s)
    return 0;
  if (-32768 > s)
    return 0;
  if (-32768 >= s)
    return 0;

  if (s == 32767UL)
    return 0;
  if (s != 32767UL)
    return 0;
  if (s < 32767UL)
    return 0;
  if (s <= 32767UL)
    return 0;
  if (s > 32767UL)
    return 0;
  if (s >= 32767UL)
    return 0;

  if (32767UL == s)
    return 0;
  if (32767UL != s)
    return 0;
  if (32767UL < s)
    return 0;
  if (32767UL <= s)
    return 0;
  if (32767UL > s)
    return 0;
  if (32767UL >= s)
    return 0;

  // FIXME: assumes two's complement
  if (s == -32768L)
    return 0;
  if (s != -32768L)
    return 0;
  if (s < -32768L)
    return 0;
  if (s <= -32768L)
    return 0;
  if (s > -32768L)
    return 0;
  if (s >= -32768L)
    return 0;

  if (-32768L == s)
    return 0;
  if (-32768L != s)
    return 0;
  if (-32768L < s)
    return 0;
  if (-32768L <= s)
    return 0;
  if (-32768L > s)
    return 0;
  if (-32768L >= s)
    return 0;
#endif

  if (s == 0)
    return 0;
  if (s != 0)
    return 0;
  if (s < 0)
    return 0;
  if (s <= 0)
    return 0;
  if (s > 0)
    return 0;
  if (s >= 0)
    return 0;

  if (0 == s)
    return 0;
  if (0 != s)
    return 0;
  if (0 < s)
    return 0;
  if (0 <= s)
    return 0;
  if (0 > s)
    return 0;
  if (0 >= s)
    return 0;

  // However the comparison with 0U would warn

  unsigned short us = value();

#ifdef TEST
  if (us == 65535)
      return 0;
  if (us != 65535)
      return 0;
  if (us < 65535)
      return 0;
  if (us <= 65535) // expected-warning {{comparison 'unsigned short' <= 65535 is always true}}
      return 0;
  if (us > 65535) // expected-warning {{comparison 'unsigned short' > 65535 is always false}}
      return 0;
  if (us >= 65535)
      return 0;

  if (65535 == us)
      return 0;
  if (65535 != us)
      return 0;
  if (65535 < us) // expected-warning {{comparison 65535 < 'unsigned short' is always false}}
      return 0;
  if (65535 <= us)
      return 0;
  if (65535 > us)
      return 0;
  if (65535 >= us) // expected-warning {{comparison 65535 >= 'unsigned short' is always true}}
      return 0;

  if (us == 65535UL)
      return 0;
  if (us != 65535UL)
      return 0;
  if (us < 65535UL)
      return 0;
  if (us <= 65535UL) // expected-warning {{comparison 'unsigned short' <= 65535 is always true}}
      return 0;
  if (us > 65535UL) // expected-warning {{comparison 'unsigned short' > 65535 is always false}}
      return 0;
  if (us >= 65535UL)
      return 0;

  if (65535UL == us)
      return 0;
  if (65535UL != us)
      return 0;
  if (65535UL < us) // expected-warning {{comparison 65535 < 'unsigned short' is always false}}
      return 0;
  if (65535UL <= us)
      return 0;
  if (65535UL > us)
      return 0;
  if (65535UL >= us) // expected-warning {{comparison 65535 >= 'unsigned short' is always true}}
      return 0;
#else
  // expected-no-diagnostics
  if (us == 65535)
      return 0;
  if (us != 65535)
      return 0;
  if (us < 65535)
      return 0;
  if (us <= 65535)
      return 0;
  if (us > 65535)
      return 0;
  if (us >= 65535)
      return 0;

  if (65535 == us)
      return 0;
  if (65535 != us)
      return 0;
  if (65535 < us)
      return 0;
  if (65535 <= us)
      return 0;
  if (65535 > us)
      return 0;
  if (65535 >= us)
      return 0;

  if (us == 65535UL)
      return 0;
  if (us != 65535UL)
      return 0;
  if (us < 65535UL)
      return 0;
  if (us <= 65535UL)
      return 0;
  if (us > 65535UL)
      return 0;
  if (us >= 65535UL)
      return 0;

  if (65535UL == us)
      return 0;
  if (65535UL != us)
      return 0;
  if (65535UL < us)
      return 0;
  if (65535UL <= us)
      return 0;
  if (65535UL > us)
      return 0;
  if (65535UL >= us)
      return 0;
#endif

  if (us == 32767)
    return 0;
  if (us != 32767)
    return 0;
  if (us < 32767)
    return 0;
  if (us <= 32767)
    return 0;
  if (us > 32767)
    return 0;
  if (us >= 32767)
    return 0;

  if (32767 == us)
    return 0;
  if (32767 != us)
    return 0;
  if (32767 < us)
    return 0;
  if (32767 <= us)
    return 0;
  if (32767 > us)
    return 0;
  if (32767 >= us)
    return 0;

  if (us == 32767UL)
    return 0;
  if (us != 32767UL)
    return 0;
  if (us < 32767UL)
    return 0;
  if (us <= 32767UL)
    return 0;
  if (us > 32767UL)
    return 0;
  if (us >= 32767UL)
    return 0;

  if (32767UL == us)
    return 0;
  if (32767UL != us)
    return 0;
  if (32767UL < us)
    return 0;
  if (32767UL <= us)
    return 0;
  if (32767UL > us)
    return 0;
  if (32767UL >= us)
    return 0;

#if __SIZEOF_INT128__
  __int128 i128;
  if (i128 == -1) // used to crash
      return 0;
#endif


  enum E {
  yes,
  no,
  maybe
  };
  enum E e;

  if (e == yes)
      return 0;
  if (e != yes)
      return 0;
  if (e < yes)
      return 0;
  if (e <= yes)
      return 0;
  if (e > yes)
      return 0;
  if (e >= yes)
      return 0;

  if (yes == e)
      return 0;
  if (yes != e)
      return 0;
  if (yes < e)
      return 0;
  if (yes <= e)
      return 0;
  if (yes > e)
      return 0;
  if (yes >= e)
      return 0;

  if (e == maybe)
      return 0;
  if (e != maybe)
      return 0;
  if (e < maybe)
      return 0;
  if (e <= maybe)
      return 0;
  if (e > maybe)
      return 0;
  if (e >= maybe)
      return 0;

  if (maybe == e)
      return 0;
  if (maybe != e)
      return 0;
  if (maybe < e)
      return 0;
  if (maybe <= e)
      return 0;
  if (maybe > e)
      return 0;
  if (maybe >= e)
      return 0;

  return 1;
}
