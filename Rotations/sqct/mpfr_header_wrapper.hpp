//
// SHORT DESCRIPTION
// =================
//
// mpfr::real is a C++ interface to the GNU MPFR library
// version 3.0.0 or later.
//
// COPYRIGHT/LICENSE
// =================
//
// Copyright 2010,2011,2012 Christian Schneider <software(at)chschneider(dot)eu>
//
// Version: 0.0.9-alpha
//
// This file is part of mpfr::real.
//
// mpfr::real is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, version 3 of the License, NOT any later
// version.
//
// mpfr::real is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with mpfr::real.  If not, see <http://www.gnu.org/licenses/>.
//

#ifndef MPFR_HEADER_WRAPPER_HPP
#define MPFR_HEADER_WRAPPER_HPP 1

// ////////////////////////////////////////////////////////////////////
// include headers
// ////////////////////////////////////////////////////////////////////

#include <mpfr.h>
#include <iostream>

// ////////////////////////////////////////////////////////////////////
// define default debugging function
// ////////////////////////////////////////////////////////////////////

namespace mpfr {

  template <class _Tp>
  void debug_func(const _Tp& s) {
    std::cerr << s << std::endl;
  }

}  // namespace mpfr

// ////////////////////////////////////////////////////////////////////
// redirect MPFR functions
// ////////////////////////////////////////////////////////////////////

// NOTE: This requires to undefine some preprocessor macros that will be
// used instead of a "true" function, e.g., "mpfr_zero_p" (see below).

#define MPFR_NS  mpfr::

// ////////////////////////////////////////////////////////////////////
// undefine all macros with name of MPFR function
// ////////////////////////////////////////////////////////////////////

#undef mpfr_set_ui
#undef mpfr_get_ui
#undef mpfr_add_ui
#undef mpfr_sub_ui
#undef mpfr_ui_sub
#undef mpfr_mul_ui
#undef mpfr_div_ui
#undef mpfr_ui_div
#undef mpfr_set_si
#undef mpfr_get_si
#undef mpfr_add_si
#undef mpfr_sub_si
#undef mpfr_si_sub
#undef mpfr_mul_si
#undef mpfr_div_si
#undef mpfr_si_div
#undef mpfr_set_d
#undef mpfr_get_d
#undef mpfr_add_d
#undef mpfr_sub_d
#undef mpfr_d_sub
#undef mpfr_mul_d
#undef mpfr_div_d
#undef mpfr_d_div
#undef mpfr_set_flt
#undef mpfr_get_flt
#undef mpfr_set_ld
#undef mpfr_get_ld
#undef mpfr_set_z
#undef mpfr_get_z
#undef mpfr_add_z
#undef mpfr_sub_z
#undef mpfr_mul_z
#undef mpfr_div_z
#undef mpfr_set_q
#undef mpfr_add_q
#undef mpfr_sub_q
#undef mpfr_mul_q
#undef mpfr_div_q
#undef mpfr_set_f
#undef mpfr_get_f
#undef mpfr_set
#undef mpfr_add
#undef mpfr_sub
#undef mpfr_mul
#undef mpfr_div
#undef mpfr_equal_p
#undef mpfr_lessgreater_p
#undef mpfr_less_p
#undef mpfr_lessequal_p
#undef mpfr_greater_p
#undef mpfr_greaterequal_p
#undef mpfr_number_p
#undef mpfr_inf_p
#undef mpfr_nan_p
#undef mpfr_regular_p
#undef mpfr_unordered_p
#undef mpfr_signbit
#undef mpfr_acos
#undef mpfr_acosh
#undef mpfr_asin
#undef mpfr_asinh
#undef mpfr_atan
#undef mpfr_atan2
#undef mpfr_atanh
#undef mpfr_cbrt
#undef mpfr_ceil
#undef mpfr_copysign
#undef mpfr_cos
#undef mpfr_cosh
#undef mpfr_erf
#undef mpfr_erfc
#undef mpfr_exp
#undef mpfr_exp2
#undef mpfr_expm1
#undef mpfr_abs
#undef mpfr_dim
#undef mpfr_floor
#undef mpfr_fma
#undef mpfr_max
#undef mpfr_min
#undef mpfr_fmod
#undef mpfr_hypot
#undef mpfr_log
#undef mpfr_log10
#undef mpfr_log1p
#undef mpfr_log2
#undef mpfr_rint
#undef mpfr_pow
#undef mpfr_remainder
#undef mpfr_round
#undef mpfr_sin
#undef mpfr_sinh
#undef mpfr_sqrt
#undef mpfr_tan
#undef mpfr_tanh
#undef mpfr_gamma
#undef mpfr_trunc
#undef mpfr_j0
#undef mpfr_j1
#undef mpfr_y0
#undef mpfr_y1
#undef mpfr_agm
#undef mpfr_ai
#undef mpfr_cmpabs
#undef mpfr_cot
#undef mpfr_coth
#undef mpfr_csc
#undef mpfr_csch
#undef mpfr_digamma
#undef mpfr_exp10
#undef mpfr_eint
#undef mpfr_fms
#undef mpfr_frac
#undef mpfr_integer_p
#undef mpfr_zero_p
#undef mpfr_li2
#undef mpfr_rec_sqrt
#undef mpfr_sec
#undef mpfr_sech
#undef mpfr_sgn
#undef mpfr_zeta
#undef mpfr_init2
#undef mpfr_clear
#undef mpfr_set_str
#undef mpfr_get_str
#undef mpfr_free_str
#undef mpfr_set_exp
#undef mpfr_get_exp
#undef mpfr_set_nan
#undef mpfr_set_inf
#undef mpfr_set_zero
#undef mpfr_neg
#undef mpfr_const_catalan
#undef mpfr_const_euler
#undef mpfr_const_pi
#undef mpfr_const_log2
#undef mpfr_fac_ui
#undef mpfr_jn
#undef mpfr_yn
#undef mpfr_lgamma
#undef mpfr_modf
#undef mpfr_mul_2si
#undef mpfr_nexttoward
#undef mpfr_remquo
#undef mpfr_root

// ////////////////////////////////////////////////////////////////////
// define MPFR wrapper functions
// ////////////////////////////////////////////////////////////////////

namespace mpfr {

  int mpfr_set_ui(mpfr_ptr rop, const unsigned long int op, mpfr_rnd_t rnd) {
    debug_func("mpfr_set_ui");
    return ::mpfr_set_ui(rop, op, rnd);
  }

  unsigned long int mpfr_get_ui(mpfr_srcptr op, mpfr_rnd_t rnd) {
    debug_func("mpfr_get_ui");
    return ::mpfr_get_ui(op, rnd);
  }

  int mpfr_add_ui(mpfr_ptr rop, mpfr_srcptr op1, const unsigned long int op2, mpfr_rnd_t rnd) {
    debug_func("mpfr_add_ui");
    return ::mpfr_add_ui(rop, op1, op2, rnd);
  }

  int mpfr_sub_ui(mpfr_ptr rop, mpfr_srcptr op1, const unsigned long int op2, mpfr_rnd_t rnd) {
    debug_func("mpfr_sub_ui");
    return ::mpfr_sub_ui(rop, op1, op2, rnd);
  }

  int mpfr_ui_sub(mpfr_ptr rop, const unsigned long int op1, mpfr_srcptr op2, mpfr_rnd_t rnd) {
    debug_func("mpfr_ui_sub");
    return ::mpfr_ui_sub(rop, op1, op2, rnd);
  }

  int mpfr_mul_ui(mpfr_ptr rop, mpfr_srcptr op1, const unsigned long int op2, mpfr_rnd_t rnd) {
    debug_func("mpfr_mul_ui");
    return ::mpfr_mul_ui(rop, op1, op2, rnd);
  }

  int mpfr_div_ui(mpfr_ptr rop, mpfr_srcptr op1, const unsigned long int op2, mpfr_rnd_t rnd) {
    debug_func("mpfr_div_ui");
    return ::mpfr_div_ui(rop, op1, op2, rnd);
  }

  int mpfr_ui_div(mpfr_ptr rop, const unsigned long int op1, mpfr_srcptr op2, mpfr_rnd_t rnd) {
    debug_func("mpfr_ui_div");
    return ::mpfr_ui_div(rop, op1, op2, rnd);
  }

  int mpfr_set_si(mpfr_ptr rop, const long int op, mpfr_rnd_t rnd) {
    debug_func("mpfr_set_si");
    return ::mpfr_set_si(rop, op, rnd);
  }

  long int mpfr_get_si(mpfr_srcptr op, mpfr_rnd_t rnd) {
    debug_func("mpfr_get_si");
    return ::mpfr_get_si(op, rnd);
  }

  int mpfr_add_si(mpfr_ptr rop, mpfr_srcptr op1, const long int op2, mpfr_rnd_t rnd) {
    debug_func("mpfr_add_si");
    return ::mpfr_add_si(rop, op1, op2, rnd);
  }

  int mpfr_sub_si(mpfr_ptr rop, mpfr_srcptr op1, const long int op2, mpfr_rnd_t rnd) {
    debug_func("mpfr_sub_si");
    return ::mpfr_sub_si(rop, op1, op2, rnd);
  }

  int mpfr_si_sub(mpfr_ptr rop, const long int op1, mpfr_srcptr op2, mpfr_rnd_t rnd) {
    debug_func("mpfr_si_sub");
    return ::mpfr_si_sub(rop, op1, op2, rnd);
  }

  int mpfr_mul_si(mpfr_ptr rop, mpfr_srcptr op1, const long int op2, mpfr_rnd_t rnd) {
    debug_func("mpfr_mul_si");
    return ::mpfr_mul_si(rop, op1, op2, rnd);
  }

  int mpfr_div_si(mpfr_ptr rop, mpfr_srcptr op1, const long int op2, mpfr_rnd_t rnd) {
    debug_func("mpfr_div_si");
    return ::mpfr_div_si(rop, op1, op2, rnd);
  }

  int mpfr_si_div(mpfr_ptr rop, const long int op1, mpfr_srcptr op2, mpfr_rnd_t rnd) {
    debug_func("mpfr_si_div");
    return ::mpfr_si_div(rop, op1, op2, rnd);
  }

  int mpfr_set_d(mpfr_ptr rop, const double op, mpfr_rnd_t rnd) {
    debug_func("mpfr_set_d");
    return ::mpfr_set_d(rop, op, rnd);
  }

  double mpfr_get_d(mpfr_srcptr op, mpfr_rnd_t rnd) {
    debug_func("mpfr_get_d");
    return ::mpfr_get_d(op, rnd);
  }

  int mpfr_add_d(mpfr_ptr rop, mpfr_srcptr op1, const double op2, mpfr_rnd_t rnd) {
    debug_func("mpfr_add_d");
    return ::mpfr_add_d(rop, op1, op2, rnd);
  }

  int mpfr_sub_d(mpfr_ptr rop, mpfr_srcptr op1, const double op2, mpfr_rnd_t rnd) {
    debug_func("mpfr_sub_d");
    return ::mpfr_sub_d(rop, op1, op2, rnd);
  }

  int mpfr_d_sub(mpfr_ptr rop, const double op1, mpfr_srcptr op2, mpfr_rnd_t rnd) {
    debug_func("mpfr_d_sub");
    return ::mpfr_d_sub(rop, op1, op2, rnd);
  }

  int mpfr_mul_d(mpfr_ptr rop, mpfr_srcptr op1, const double op2, mpfr_rnd_t rnd) {
    debug_func("mpfr_mul_d");
    return ::mpfr_mul_d(rop, op1, op2, rnd);
  }

  int mpfr_div_d(mpfr_ptr rop, mpfr_srcptr op1, const double op2, mpfr_rnd_t rnd) {
    debug_func("mpfr_div_d");
    return ::mpfr_div_d(rop, op1, op2, rnd);
  }

  int mpfr_d_div(mpfr_ptr rop, const double op1, mpfr_srcptr op2, mpfr_rnd_t rnd) {
    debug_func("mpfr_d_div");
    return ::mpfr_d_div(rop, op1, op2, rnd);
  }

  int mpfr_set_flt(mpfr_ptr rop, const float op, mpfr_rnd_t rnd) {
    debug_func("mpfr_set_flt");
    return ::mpfr_set_flt(rop, op, rnd);
  }

  float mpfr_get_flt(mpfr_srcptr op, mpfr_rnd_t rnd) {
    debug_func("mpfr_get_flt");
    return ::mpfr_get_flt(op, rnd);
  }

  int mpfr_set_ld(mpfr_ptr rop, const long double op, mpfr_rnd_t rnd) {
    debug_func("mpfr_set_ld");
    return ::mpfr_set_ld(rop, op, rnd);
  }

  long double mpfr_get_ld(mpfr_srcptr op, mpfr_rnd_t rnd) {
    debug_func("mpfr_get_ld");
    return ::mpfr_get_ld(op, rnd);
  }

  int mpfr_set_z(mpfr_ptr rop, mpz_srcptr op, mpfr_rnd_t rnd) {
    debug_func("mpfr_set_z");
    return ::mpfr_set_z(rop, op, rnd);
  }

  int mpfr_get_z(mpz_t rop, mpfr_srcptr op, mpfr_rnd_t rnd) {
    debug_func("mpfr_get_z");
    return ::mpfr_get_z(rop, op, rnd);
  }

  int mpfr_add_z(mpfr_ptr rop, mpfr_srcptr op1, mpz_srcptr op2, mpfr_rnd_t rnd) {
    debug_func("mpfr_add_z");
    return ::mpfr_add_z(rop, op1, op2, rnd);
  }

  int mpfr_sub_z(mpfr_ptr rop, mpfr_srcptr op1, mpz_srcptr op2, mpfr_rnd_t rnd) {
    debug_func("mpfr_sub_z");
    return ::mpfr_sub_z(rop, op1, op2, rnd);
  }

  int mpfr_mul_z(mpfr_ptr rop, mpfr_srcptr op1, mpz_srcptr op2, mpfr_rnd_t rnd) {
    debug_func("mpfr_mul_z");
    return ::mpfr_mul_z(rop, op1, op2, rnd);
  }

  int mpfr_div_z(mpfr_ptr rop, mpfr_srcptr op1, mpz_srcptr op2, mpfr_rnd_t rnd) {
    debug_func("mpfr_div_z");
    return ::mpfr_div_z(rop, op1, op2, rnd);
  }

  int mpfr_set_q(mpfr_ptr rop, mpq_srcptr op, mpfr_rnd_t rnd) {
    debug_func("mpfr_set_q");
    return ::mpfr_set_q(rop, op, rnd);
  }

  int mpfr_add_q(mpfr_ptr rop, mpfr_srcptr op1, mpq_srcptr op2, mpfr_rnd_t rnd) {
    debug_func("mpfr_add_q");
    return ::mpfr_add_q(rop, op1, op2, rnd);
  }

  int mpfr_sub_q(mpfr_ptr rop, mpfr_srcptr op1, mpq_srcptr op2, mpfr_rnd_t rnd) {
    debug_func("mpfr_sub_q");
    return ::mpfr_sub_q(rop, op1, op2, rnd);
  }

  int mpfr_mul_q(mpfr_ptr rop, mpfr_srcptr op1, mpq_srcptr op2, mpfr_rnd_t rnd) {
    debug_func("mpfr_mul_q");
    return ::mpfr_mul_q(rop, op1, op2, rnd);
  }

  int mpfr_div_q(mpfr_ptr rop, mpfr_srcptr op1, mpq_srcptr op2, mpfr_rnd_t rnd) {
    debug_func("mpfr_div_q");
    return ::mpfr_div_q(rop, op1, op2, rnd);
  }

  int mpfr_set_f(mpfr_ptr rop, mpf_srcptr op, mpfr_rnd_t rnd) {
    debug_func("mpfr_set_f");
    return ::mpfr_set_f(rop, op, rnd);
  }

  int mpfr_get_f(mpf_t rop, mpfr_srcptr op, mpfr_rnd_t rnd) {
    debug_func("mpfr_get_f");
    return ::mpfr_get_f(rop, op, rnd);
  }

  int mpfr_set(mpfr_ptr rop, mpfr_srcptr op, mpfr_rnd_t rnd) {
    debug_func("mpfr_set");
    return ::mpfr_set(rop, op, rnd);
  }

  int mpfr_add(mpfr_ptr rop, mpfr_srcptr op1, mpfr_srcptr op2, mpfr_rnd_t rnd) {
    debug_func("mpfr_add");
    return ::mpfr_add(rop, op1, op2, rnd);
  }

  int mpfr_sub(mpfr_ptr rop, mpfr_srcptr op1, mpfr_srcptr op2, mpfr_rnd_t rnd) {
    debug_func("mpfr_sub");
    return ::mpfr_sub(rop, op1, op2, rnd);
  }

  int mpfr_mul(mpfr_ptr rop, mpfr_srcptr op1, mpfr_srcptr op2, mpfr_rnd_t rnd) {
    debug_func("mpfr_mul");
    return ::mpfr_mul(rop, op1, op2, rnd);
  }

  int mpfr_div(mpfr_ptr rop, mpfr_srcptr op1, mpfr_srcptr op2, mpfr_rnd_t rnd) {
    debug_func("mpfr_div");
    return ::mpfr_div(rop, op1, op2, rnd);
  }

  int mpfr_equal_p(mpfr_srcptr op1, mpfr_srcptr op2) {
    debug_func("mpfr_equal_p");
    return ::mpfr_equal_p(op1, op2);
  }

  int mpfr_lessgreater_p(mpfr_srcptr op1, mpfr_srcptr op2) {
    debug_func("mpfr_lessgreater_p");
    return ::mpfr_lessgreater_p(op1, op2);
  }

  int mpfr_less_p(mpfr_srcptr op1, mpfr_srcptr op2) {
    debug_func("mpfr_less_p");
    return ::mpfr_less_p(op1, op2);
  }

  int mpfr_lessequal_p(mpfr_srcptr op1, mpfr_srcptr op2) {
    debug_func("mpfr_lessequal_p");
    return ::mpfr_lessequal_p(op1, op2);
  }

  int mpfr_greater_p(mpfr_srcptr op1, mpfr_srcptr op2) {
    debug_func("mpfr_greater_p");
    return ::mpfr_greater_p(op1, op2);
  }

  int mpfr_greaterequal_p(mpfr_srcptr op1, mpfr_srcptr op2) {
    debug_func("mpfr_greaterequal_p");
    return ::mpfr_greaterequal_p(op1, op2);
  }

  int mpfr_number_p(mpfr_srcptr op) {
    debug_func("mpfr_number_p");
    return ::mpfr_number_p(op);
  }

  int mpfr_inf_p(mpfr_srcptr op) {
    debug_func("mpfr_inf_p");
    return ::mpfr_inf_p(op);
  }

  int mpfr_nan_p(mpfr_srcptr op) {
    debug_func("mpfr_nan_p");
    return ::mpfr_nan_p(op);
  }

  int mpfr_regular_p(mpfr_srcptr op) {
    debug_func("mpfr_regular_p");
    return ::mpfr_regular_p(op);
  }

  int mpfr_unordered_p(mpfr_srcptr op1, mpfr_srcptr op2) {
    debug_func("mpfr_unordered_p");
    return ::mpfr_unordered_p(op1, op2);
  }

  int mpfr_signbit(mpfr_srcptr op) {
    debug_func("mpfr_signbit");
    return ::mpfr_signbit(op);
  }

  int mpfr_acos(mpfr_ptr rop, mpfr_srcptr op, mpfr_rnd_t rnd) {
    debug_func("mpfr_acos");
    return ::mpfr_acos(rop, op, rnd);
  }

  int mpfr_acosh(mpfr_ptr rop, mpfr_srcptr op, mpfr_rnd_t rnd) {
    debug_func("mpfr_acosh");
    return ::mpfr_acosh(rop, op, rnd);
  }

  int mpfr_asin(mpfr_ptr rop, mpfr_srcptr op, mpfr_rnd_t rnd) {
    debug_func("mpfr_asin");
    return ::mpfr_asin(rop, op, rnd);
  }

  int mpfr_asinh(mpfr_ptr rop, mpfr_srcptr op, mpfr_rnd_t rnd) {
    debug_func("mpfr_asinh");
    return ::mpfr_asinh(rop, op, rnd);
  }

  int mpfr_atan(mpfr_ptr rop, mpfr_srcptr op, mpfr_rnd_t rnd) {
    debug_func("mpfr_atan");
    return ::mpfr_atan(rop, op, rnd);
  }

  int mpfr_atan2(mpfr_ptr rop, mpfr_srcptr op1, mpfr_srcptr op2, mpfr_rnd_t rnd) {
    debug_func("mpfr_atan2");
    return ::mpfr_atan2(rop, op1, op2, rnd);
  }

  int mpfr_atanh(mpfr_ptr rop, mpfr_srcptr op, mpfr_rnd_t rnd) {
    debug_func("mpfr_atanh");
    return ::mpfr_atanh(rop, op, rnd);
  }

  int mpfr_cbrt(mpfr_ptr rop, mpfr_srcptr op, mpfr_rnd_t rnd) {
    debug_func("mpfr_cbrt");
    return ::mpfr_cbrt(rop, op, rnd);
  }

  int mpfr_ceil(mpfr_ptr rop, mpfr_srcptr op) {
    debug_func("mpfr_ceil");
    return ::mpfr_ceil(rop, op);
  }

  int mpfr_copysign(mpfr_ptr rop, mpfr_srcptr op1, mpfr_srcptr op2, mpfr_rnd_t rnd) {
    debug_func("mpfr_copysign");
    return ::mpfr_copysign(rop, op1, op2, rnd);
  }

  int mpfr_cos(mpfr_ptr rop, mpfr_srcptr op, mpfr_rnd_t rnd) {
    debug_func("mpfr_cos");
    return ::mpfr_cos(rop, op, rnd);
  }

  int mpfr_cosh(mpfr_ptr rop, mpfr_srcptr op, mpfr_rnd_t rnd) {
    debug_func("mpfr_cosh");
    return ::mpfr_cosh(rop, op, rnd);
  }

  int mpfr_erf(mpfr_ptr rop, mpfr_srcptr op, mpfr_rnd_t rnd) {
    debug_func("mpfr_erf");
    return ::mpfr_erf(rop, op, rnd);
  }

  int mpfr_erfc(mpfr_ptr rop, mpfr_srcptr op, mpfr_rnd_t rnd) {
    debug_func("mpfr_erfc");
    return ::mpfr_erfc(rop, op, rnd);
  }

  int mpfr_exp(mpfr_ptr rop, mpfr_srcptr op, mpfr_rnd_t rnd) {
    debug_func("mpfr_exp");
    return ::mpfr_exp(rop, op, rnd);
  }

  int mpfr_exp2(mpfr_ptr rop, mpfr_srcptr op, mpfr_rnd_t rnd) {
    debug_func("mpfr_exp2");
    return ::mpfr_exp2(rop, op, rnd);
  }

  int mpfr_expm1(mpfr_ptr rop, mpfr_srcptr op, mpfr_rnd_t rnd) {
    debug_func("mpfr_expm1");
    return ::mpfr_expm1(rop, op, rnd);
  }

  int mpfr_abs(mpfr_ptr rop, mpfr_srcptr op, mpfr_rnd_t rnd) {
    debug_func("mpfr_abs");
    return ::mpfr_abs(rop, op, rnd);
  }

  int mpfr_dim(mpfr_ptr rop, mpfr_srcptr op1, mpfr_srcptr op2, mpfr_rnd_t rnd) {
    debug_func("mpfr_dim");
    return ::mpfr_dim(rop, op1, op2, rnd);
  }

  int mpfr_floor(mpfr_ptr rop, mpfr_srcptr op) {
    debug_func("mpfr_floor");
    return ::mpfr_floor(rop, op);
  }

  int mpfr_fma(mpfr_ptr rop, mpfr_srcptr op1, mpfr_srcptr op2, mpfr_srcptr op3, mpfr_rnd_t rnd) {
    debug_func("mpfr_fma");
    return ::mpfr_fma(rop, op1, op2, op3, rnd);
  }

  int mpfr_max(mpfr_ptr rop, mpfr_srcptr op1, mpfr_srcptr op2, mpfr_rnd_t rnd) {
    debug_func("mpfr_max");
    return ::mpfr_max(rop, op1, op2, rnd);
  }

  int mpfr_min(mpfr_ptr rop, mpfr_srcptr op1, mpfr_srcptr op2, mpfr_rnd_t rnd) {
    debug_func("mpfr_min");
    return ::mpfr_min(rop, op1, op2, rnd);
  }

  int mpfr_fmod(mpfr_ptr rop, mpfr_srcptr op1, mpfr_srcptr op2, mpfr_rnd_t rnd) {
    debug_func("mpfr_fmod");
    return ::mpfr_fmod(rop, op1, op2, rnd);
  }

  int mpfr_hypot(mpfr_ptr rop, mpfr_srcptr op1, mpfr_srcptr op2, mpfr_rnd_t rnd) {
    debug_func("mpfr_hypot");
    return ::mpfr_hypot(rop, op1, op2, rnd);
  }

  int mpfr_log(mpfr_ptr rop, mpfr_srcptr op, mpfr_rnd_t rnd) {
    debug_func("mpfr_log");
    return ::mpfr_log(rop, op, rnd);
  }

  int mpfr_log10(mpfr_ptr rop, mpfr_srcptr op, mpfr_rnd_t rnd) {
    debug_func("mpfr_log10");
    return ::mpfr_log10(rop, op, rnd);
  }

  int mpfr_log1p(mpfr_ptr rop, mpfr_srcptr op, mpfr_rnd_t rnd) {
    debug_func("mpfr_log1p");
    return ::mpfr_log1p(rop, op, rnd);
  }

  int mpfr_log2(mpfr_ptr rop, mpfr_srcptr op, mpfr_rnd_t rnd) {
    debug_func("mpfr_log2");
    return ::mpfr_log2(rop, op, rnd);
  }

  int mpfr_rint(mpfr_ptr rop, mpfr_srcptr op, mpfr_rnd_t rnd) {
    debug_func("mpfr_rint");
    return ::mpfr_rint(rop, op, rnd);
  }

  int mpfr_pow(mpfr_ptr rop, mpfr_srcptr op1, mpfr_srcptr op2, mpfr_rnd_t rnd) {
    debug_func("mpfr_pow");
    return ::mpfr_pow(rop, op1, op2, rnd);
  }

  int mpfr_remainder(mpfr_ptr rop, mpfr_srcptr op1, mpfr_srcptr op2, mpfr_rnd_t rnd) {
    debug_func("mpfr_remainder");
    return ::mpfr_remainder(rop, op1, op2, rnd);
  }

  int mpfr_round(mpfr_ptr rop, mpfr_srcptr op) {
    debug_func("mpfr_round");
    return ::mpfr_round(rop, op);
  }

  int mpfr_sin(mpfr_ptr rop, mpfr_srcptr op, mpfr_rnd_t rnd) {
    debug_func("mpfr_sin");
    return ::mpfr_sin(rop, op, rnd);
  }

  int mpfr_sinh(mpfr_ptr rop, mpfr_srcptr op, mpfr_rnd_t rnd) {
    debug_func("mpfr_sinh");
    return ::mpfr_sinh(rop, op, rnd);
  }

  int mpfr_sqrt(mpfr_ptr rop, mpfr_srcptr op, mpfr_rnd_t rnd) {
    debug_func("mpfr_sqrt");
    return ::mpfr_sqrt(rop, op, rnd);
  }

  int mpfr_tan(mpfr_ptr rop, mpfr_srcptr op, mpfr_rnd_t rnd) {
    debug_func("mpfr_tan");
    return ::mpfr_tan(rop, op, rnd);
  }

  int mpfr_tanh(mpfr_ptr rop, mpfr_srcptr op, mpfr_rnd_t rnd) {
    debug_func("mpfr_tanh");
    return ::mpfr_tanh(rop, op, rnd);
  }

  int mpfr_gamma(mpfr_ptr rop, mpfr_srcptr op, mpfr_rnd_t rnd) {
    debug_func("mpfr_gamma");
    return ::mpfr_gamma(rop, op, rnd);
  }

  int mpfr_trunc(mpfr_ptr rop, mpfr_srcptr op) {
    debug_func("mpfr_trunc");
    return ::mpfr_trunc(rop, op);
  }

  int mpfr_j0(mpfr_ptr rop, mpfr_srcptr op, mpfr_rnd_t rnd) {
    debug_func("mpfr_j0");
    return ::mpfr_j0(rop, op, rnd);
  }

  int mpfr_j1(mpfr_ptr rop, mpfr_srcptr op, mpfr_rnd_t rnd) {
    debug_func("mpfr_j1");
    return ::mpfr_j1(rop, op, rnd);
  }

  int mpfr_y0(mpfr_ptr rop, mpfr_srcptr op, mpfr_rnd_t rnd) {
    debug_func("mpfr_y0");
    return ::mpfr_y0(rop, op, rnd);
  }

  int mpfr_y1(mpfr_ptr rop, mpfr_srcptr op, mpfr_rnd_t rnd) {
    debug_func("mpfr_y1");
    return ::mpfr_y1(rop, op, rnd);
  }

  int mpfr_agm(mpfr_ptr rop, mpfr_srcptr op1, mpfr_srcptr op2, mpfr_rnd_t rnd) {
    debug_func("mpfr_agm");
    return ::mpfr_agm(rop, op1, op2, rnd);
  }

  int mpfr_ai(mpfr_ptr rop, mpfr_srcptr op, mpfr_rnd_t rnd) {
    debug_func("mpfr_ai");
    return ::mpfr_ai(rop, op, rnd);
  }

  int mpfr_cmpabs(mpfr_srcptr op1, mpfr_srcptr op2) {
    debug_func("mpfr_cmpabs");
    return ::mpfr_cmpabs(op1, op2);
  }

  int mpfr_cot(mpfr_ptr rop, mpfr_srcptr op, mpfr_rnd_t rnd) {
    debug_func("mpfr_cot");
    return ::mpfr_cot(rop, op, rnd);
  }

  int mpfr_coth(mpfr_ptr rop, mpfr_srcptr op, mpfr_rnd_t rnd) {
    debug_func("mpfr_coth");
    return ::mpfr_coth(rop, op, rnd);
  }

  int mpfr_csc(mpfr_ptr rop, mpfr_srcptr op, mpfr_rnd_t rnd) {
    debug_func("mpfr_csc");
    return ::mpfr_csc(rop, op, rnd);
  }

  int mpfr_csch(mpfr_ptr rop, mpfr_srcptr op, mpfr_rnd_t rnd) {
    debug_func("mpfr_csch");
    return ::mpfr_csch(rop, op, rnd);
  }

  int mpfr_digamma(mpfr_ptr rop, mpfr_srcptr op, mpfr_rnd_t rnd) {
    debug_func("mpfr_digamma");
    return ::mpfr_digamma(rop, op, rnd);
  }

  int mpfr_exp10(mpfr_ptr rop, mpfr_srcptr op, mpfr_rnd_t rnd) {
    debug_func("mpfr_exp10");
    return ::mpfr_exp10(rop, op, rnd);
  }

  int mpfr_eint(mpfr_ptr rop, mpfr_srcptr op, mpfr_rnd_t rnd) {
    debug_func("mpfr_eint");
    return ::mpfr_eint(rop, op, rnd);
  }

  int mpfr_fms(mpfr_ptr rop, mpfr_srcptr op1, mpfr_srcptr op2, mpfr_srcptr op3, mpfr_rnd_t rnd) {
    debug_func("mpfr_fms");
    return ::mpfr_fms(rop, op1, op2, op3, rnd);
  }

  int mpfr_frac(mpfr_ptr rop, mpfr_srcptr op, mpfr_rnd_t rnd) {
    debug_func("mpfr_frac");
    return ::mpfr_frac(rop, op, rnd);
  }

  int mpfr_integer_p(mpfr_srcptr op) {
    debug_func("mpfr_integer_p");
    return ::mpfr_integer_p(op);
  }

  int mpfr_zero_p(mpfr_srcptr op) {
    debug_func("mpfr_zero_p");
    return ::mpfr_zero_p(op);
  }

  int mpfr_li2(mpfr_ptr rop, mpfr_srcptr op, mpfr_rnd_t rnd) {
    debug_func("mpfr_li2");
    return ::mpfr_li2(rop, op, rnd);
  }

  int mpfr_rec_sqrt(mpfr_ptr rop, mpfr_srcptr op, mpfr_rnd_t rnd) {
    debug_func("mpfr_rec_sqrt");
    return ::mpfr_rec_sqrt(rop, op, rnd);
  }

  int mpfr_sec(mpfr_ptr rop, mpfr_srcptr op, mpfr_rnd_t rnd) {
    debug_func("mpfr_sec");
    return ::mpfr_sec(rop, op, rnd);
  }

  int mpfr_sech(mpfr_ptr rop, mpfr_srcptr op, mpfr_rnd_t rnd) {
    debug_func("mpfr_sech");
    return ::mpfr_sech(rop, op, rnd);
  }

  int mpfr_sgn(mpfr_srcptr op) {
    debug_func("mpfr_sgn");
    return ::mpfr_sgn(op);
  }

  int mpfr_zeta(mpfr_ptr rop, mpfr_srcptr op, mpfr_rnd_t rnd) {
    debug_func("mpfr_zeta");
    return ::mpfr_zeta(rop, op, rnd);
  }

  void mpfr_init2(mpfr_ptr op, mpfr_prec_t prec) {
    debug_func("mpfr_init2");
    return ::mpfr_init2(op, prec);
  }

  void mpfr_clear(mpfr_ptr op) {
    debug_func("mpfr_clear");
    return ::mpfr_clear(op);
  }

  int mpfr_set_str(mpfr_ptr rop, const char* c, int i, mpfr_rnd_t rnd) {
    debug_func("mpfr_set_str");
    return ::mpfr_set_str(rop, c, i, rnd);
  }

  char* mpfr_get_str(char* c, mpfr_exp_t* exp, int i, size_t s, mpfr_srcptr op, mpfr_rnd_t rnd) {
    debug_func("mpfr_get_str");
    return ::mpfr_get_str(c, exp, i, s, op, rnd);
  }

  void mpfr_free_str(char* c) {
    debug_func("mpfr_free_str");
    return ::mpfr_free_str(c);
  }

  int mpfr_set_exp(mpfr_ptr rop, mpfr_exp_t e) {
    debug_func("mpfr_set_exp");
    return ::mpfr_set_exp(rop, e);
  }

  mpfr_exp_t mpfr_get_exp(mpfr_srcptr op) {
    debug_func("mpfr_get_exp");
    return ::mpfr_get_exp(op);
  }

  void mpfr_set_nan(mpfr_ptr op) {
    debug_func("mpfr_set_nan");
    return ::mpfr_set_nan(op);
  }

  void mpfr_set_inf(mpfr_ptr op, int i) {
    debug_func("mpfr_set_inf");
    return ::mpfr_set_inf(op, i);
  }

  void mpfr_set_zero(mpfr_ptr op, int i) {
    debug_func("mpfr_set_zero");
    return ::mpfr_set_zero(op, i);
  }

  int mpfr_neg(mpfr_ptr op, mpfr_srcptr rop, mpfr_rnd_t rnd) {
    debug_func("mpfr_neg");
    return ::mpfr_neg(op, rop, rnd);
  }

  int mpfr_const_catalan(mpfr_ptr op, mpfr_rnd_t rnd) {
    debug_func("mpfr_const_catalan");
    return ::mpfr_const_catalan(op, rnd);
  }

  int mpfr_const_euler(mpfr_ptr op, mpfr_rnd_t rnd) {
    debug_func("mpfr_const_euler");
    return ::mpfr_const_euler(op, rnd);
  }

  int mpfr_const_pi(mpfr_ptr op, mpfr_rnd_t rnd) {
    debug_func("mpfr_const_pi");
    return ::mpfr_const_pi(op, rnd);
  }

  int mpfr_const_log2(mpfr_ptr op, mpfr_rnd_t rnd) {
    debug_func("mpfr_const_log2");
    return ::mpfr_const_log2(op, rnd);
  }

  int mpfr_fac_ui(mpfr_ptr op, unsigned long int n, mpfr_rnd_t rnd) {
    debug_func("mpfr_fac_ui");
    return ::mpfr_fac_ui(op, n, rnd);
  }

  int mpfr_jn(mpfr_ptr rop, long int n, mpfr_srcptr op, mpfr_rnd_t rnd) {
    debug_func("mpfr_jn");
    return ::mpfr_jn(rop, n, op, rnd);
  }

  int mpfr_yn(mpfr_ptr rop, long int n, mpfr_srcptr op, mpfr_rnd_t rnd) {
    debug_func("mpfr_yn");
    return ::mpfr_yn(rop, n, op, rnd);
  }

  int mpfr_lgamma(mpfr_ptr rop, int* n, mpfr_srcptr op, mpfr_rnd_t rnd) {
    debug_func("mpfr_lgamma");
    return ::mpfr_lgamma(rop, n, op, rnd);
  }

  int mpfr_modf(mpfr_ptr rop1, mpfr_ptr rop2, mpfr_srcptr op, mpfr_rnd_t rnd) {
    debug_func("mpfr_modf");
    return ::mpfr_modf(rop1, rop2, op, rnd);
  }

  int mpfr_mul_2si(mpfr_ptr rop, mpfr_srcptr op, long l, mpfr_rnd_t rnd) {
    debug_func("mpfr_mul_2si");
    return ::mpfr_mul_2si(rop, op, l, rnd);
  }

  void mpfr_nexttoward(mpfr_ptr rop, mpfr_srcptr op) {
    debug_func("mpfr_nexttoward");
    return ::mpfr_nexttoward(rop, op);
  }

  int mpfr_remquo(mpfr_ptr rop, long* n, mpfr_srcptr op1, mpfr_srcptr op2, mpfr_rnd_t rnd) {
    debug_func("mpfr_remquo");
    return ::mpfr_remquo(rop, n, op1, op2, rnd);
  }

  int mpfr_root(mpfr_ptr rop, mpfr_srcptr op, unsigned long int n, mpfr_rnd_t rnd) {
    debug_func("mpfr_root");
    return ::mpfr_root(rop, op, n, rnd);
  }

}  // namespace mpfr

#endif  // MPFR_HEADER_WRAPPER_HPP
