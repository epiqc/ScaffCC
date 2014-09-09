#include <bbrl________________________.h>
#include <bbrl_lib_fixed_pt_arithmetic.h>



void
a_less_than_b__as_unsigned_n_bit_integers(MANDATORY_ARGS
/*args:*/ , uint64_t n)
{
  BEGIN_DEFINITION_OF_MODULE

  uint64_t i;

  /* Signals:
        const a[n-1 ... 0]
        const b[n-1 ... 0]
              x
  */

  /* Make sure n != 0. */
  if (n == 0) {
    fprintf(stderr, "Error.\n");
    s_file_and_line_fprint(stderr, file, line);
    fprintf(stderr,
            " `module a__eq__a_minus_b__n_bit_integers(...)' : "
            "The number of bits must be >= 1.\n");
    call_stack_fprint(stderr);
    clean_exit(-1);
  }

  INSTANTIATE_MODULE
    for (i = 0; i < n; i++) {
      MAP_SIGNAL("a", i,   "a", i)
      MAP_SIGNAL("b", i,   "b", i)
    }
    MAP_SIGNAL("unsigned_underflow", ni,
               "x", ni);
  a__eq__a_minus_b__w_unsigned_underflow_bit(LOCATION_INFO
    , i);

  INSTANTIATE_MODULE
    for (i = 0; i < n; i++) {
      MAP_SIGNAL("a", i,   "a", i)
      MAP_SIGNAL("b", i,   "b", i)
    }
  a__eq__a_plus_b(LOCATION_INFO
    , i);

  END_DEFINITION_OF_MODULE
}



void
fxp_invert_sign(MANDATORY_ARGS
/*args:*/ , uint64_t m, uint64_t k)
{
  BEGIN_DEFINITION_OF_MODULE

  uint64_t m_plus_k = m + k;
  uint64_t i;

  /* Signals:
              a[m+k-1 ... 0]
  */

  /* Make sure m_plus_k != 0. */
  if (m_plus_k == 0) {
    fprintf(stderr, "Error.\n");
    s_file_and_line_fprint(stderr, file, line);
    fprintf(stderr,
            " `fxp_invert_sign(...)' : "
            "The number of bits must be >= 1.\n");
    call_stack_fprint(stderr);
    clean_exit(-1);
  }

  for (i = 0; i < m_plus_k; i++) {
    YqNOT("a", i)
  }

  END_DEFINITION_OF_MODULE
}



void
fxp_abs(MANDATORY_ARGS
/*args:*/ , uint64_t m, uint64_t k)
{
  BEGIN_DEFINITION_OF_MODULE

  uint64_t i;
  uint64_t m_plus_k = m + k;

  /* Signals:
              a[m+k-1 ... 0]
  */

  ZERO_TO_GARBAGE_ANCILLA("s", 0)

  /* Make sure m_plus_k != 0. */
  if (m_plus_k == 0) {
    fprintf(stderr, "Error.\n");
    s_file_and_line_fprint(stderr, file, line);
    fprintf(stderr,
            " `module a__eq__a_plus_b__mk_bit_fx_pt(...)' : "
            "The number of bits must be >= 1.\n");
    call_stack_fprint(stderr);
    clean_exit(-1);
  }

  for (i = 0; i < m_plus_k - 1; i++) {
    YqCNOT("a", m_plus_k - 1,  "a", i)
  }

  /* Swap "a", m_plus_k - 1  and  "s", 0. */
  YqCNOT("s", 0,                   "a", m_plus_k - 1)
  YqCNOT("a", m_plus_k - 1,  "s", 0)
  YqCNOT("s", 0,                   "a", m_plus_k - 1)

  END_DEFINITION_OF_MODULE
}



void
fxp_floor(MANDATORY_ARGS
/*args:*/ , uint64_t m, uint64_t k)
{
  BEGIN_DEFINITION_OF_MODULE

  uint64_t i;

  /* Signals:
              a[m+k-1 ... 0]
  */

  for (i = 0; i < k; i++) {
    ZERO_TO_GARBAGE_ANCILLA("ztg", i)
  }

  for (i = 0; i < k; i++) {
    YqCNOT("ztg", i,         "a", i)
    YqCNOT("a", i,  "ztg", i)
    YqCNOT("ztg", i,         "a", i)
  }

  END_DEFINITION_OF_MODULE
}



void
fxpt_primitive_unsigned_mult(MANDATORY_ARGS
/*args:*/ , uint64_t m, uint64_t n)
{
  BEGIN_DEFINITION_OF_MODULE

  uint64_t i;
  uint64_t j;

  /* Signals:
              r_of_pmult[m+n-1 ... 0]
        const b_of_pmult[m+n-1 ... 0]
        const c_of_pmult[m+n-1 ... 0]
  */

  for (i = n; i > 0; i--) {
    PUSH_CONTROL_SIGNAL("c_of_pmult", n - i)
    INSTANTIATE_MODULE
      for (j = 0; j < m + n - i; j++) {
        MAP_SIGNAL("b", j,
                   "b_of_pmult", j + i)
      }
      for (/* j = m + n - i */; j < m + n; j++) {
        MAP_SIGNAL("b", j,
                   "b_of_pmult", j - (m + n - i))
      }
      for (j = 0; j < m + n; j++) {
        MAP_SIGNAL("a", j,
                   "r_of_pmult", j)
      }
    a__eq__a_plus_b(LOCATION_INFO
      , m + n);
    POP_AND_DELETE_CONTROL_SIGNAL

    PUSH_CONTROL_SIGNAL("c_of_pmult", n - i)
    INSTANTIATE_MODULE
      for (j = 0; j < i; j++) {
        MAP_SIGNAL("b", j,
                    "b_of_pmult", j)
        MAP_SIGNAL("a", j,
                   "r_of_pmult", m + n - i + j)
      }
    a__eq__a_minus_b(LOCATION_INFO
      , i);
    POP_AND_DELETE_CONTROL_SIGNAL

  }

  for (i = 0; i < m; i++) {
    PUSH_CONTROL_SIGNAL("c_of_pmult", n + i)
    INSTANTIATE_MODULE
      for (j = 0; j < m + n - i; j++) {
        MAP_SIGNAL("b", j,
                   "b_of_pmult", j)
        MAP_SIGNAL("a", j,
                   "r_of_pmult", j + i)
      }
    a__eq__a_plus_b(LOCATION_INFO
      , m + n - i);
    POP_AND_DELETE_CONTROL_SIGNAL

  }

  END_DEFINITION_OF_MODULE
}



void
fxp_mult(MANDATORY_ARGS
/*args:*/ , uint64_t m, uint64_t k)
{
  BEGIN_DEFINITION_OF_MODULE

  uint64_t i;
  uint64_t j;
  uint64_t mpk = m + k;
  uint64_t ceil_log2_mpk = 0; while ((((uint64_t)1) << ++ceil_log2_mpk) < mpk);

  /*
  printf("m = %d, k = %d, m+k = %d, ceil_log2_mpk = %d.\n",
          (int32_t) m, (int32_t) k, (int32_t) (m+k), (int32_t) ceil_log2_mpk);
  */

  /* Signals:
              a[m+k-1 ... 0]
              b[m+k-1 ... 0]
              c[m+k-1 ... 0]
  */

  for (i = 0; i < ceil_log2_mpk; i++) {
    ZERO_TO_GARBAGE_ANCILLA("pad_b", i)
    ZERO_TO_GARBAGE_ANCILLA("pad_c", i)
  }
  for (i = 0; i < ceil_log2_mpk + mpk; i++) {
    ZERO_TO_GARBAGE_ANCILLA("res", i)
  }

  PUSH_CONTROL_SIGNAL("b", mpk - 1)
  INSTANTIATE_MODULE
    j = 0;
    for (i = 0; i < ceil_log2_mpk; i++, j++) {
      MAP_SIGNAL("a", j,   "pad_b", i)
    }
    for (i = 0; i < mpk - 1; i++, j++) {
      MAP_SIGNAL("a", j,   "b", i)
    }
  fxp_invert_sign(LOCATION_INFO
    , ceil_log2_mpk + mpk - 1, 0);
  POP_AND_DELETE_CONTROL_SIGNAL


  PUSH_CONTROL_SIGNAL("c", mpk - 1)
  INSTANTIATE_MODULE
    j = 0;
    for (i = 0; i < ceil_log2_mpk; i++, j++) {
      MAP_SIGNAL("a", j,   "pad_c", i)
    }
    for (i = 0; i < mpk - 1; i++, j++) {
      MAP_SIGNAL("a", j,   "c", i)
    }
  fxp_invert_sign(LOCATION_INFO
    , ceil_log2_mpk + mpk - 1, 0);
  POP_AND_DELETE_CONTROL_SIGNAL


  INSTANTIATE_MODULE
    j = 0;
    for (i = 0; i < ceil_log2_mpk; i++, j++) {
      MAP_SIGNAL("b_of_pmult", j,   "pad_b", i)
      MAP_SIGNAL("c_of_pmult", j,   "pad_c", i)
      MAP_SIGNAL("r_of_pmult", j,   "res", j)
    }
    for (i = 0; i < mpk - 1; i++, j++) {
      MAP_SIGNAL("b_of_pmult", j,   "b", i)
      MAP_SIGNAL("c_of_pmult", j,   "c", i)
      MAP_SIGNAL("r_of_pmult", j,   "res", j)
    }
  fxpt_primitive_unsigned_mult(LOCATION_INFO
    , m - 1, k + ceil_log2_mpk);

  PUSH_CONTROL_SIGNAL("b", mpk - 1)
  INSTANTIATE_MODULE
    for (j = 0; j < ceil_log2_mpk + mpk; j++) {
      MAP_SIGNAL("a", j,   "res", j)
    }
  fxp_invert_sign(LOCATION_INFO
    , ceil_log2_mpk + mpk, 0);
  INSTANTIATE_MODULE
    for (i = 0; i < mpk - 1; i++) {
      MAP_SIGNAL("a", i,   "b", i)
    }
  fxp_invert_sign(LOCATION_INFO
    , mpk - 1, 0);
  POP_AND_DELETE_CONTROL_SIGNAL


  PUSH_CONTROL_SIGNAL("c", mpk - 1)
  INSTANTIATE_MODULE
    for (j = 0; j < ceil_log2_mpk + mpk; j++) {
      MAP_SIGNAL("a", j,   "res", j)
    }
  fxp_invert_sign(LOCATION_INFO
    , ceil_log2_mpk + mpk, 0);
  INSTANTIATE_MODULE
    for (i = 0; i < mpk - 1; i++) {
      MAP_SIGNAL("a", i,   "c", i)
    }
  fxp_invert_sign(LOCATION_INFO
    , mpk - 1, 0);
  POP_AND_DELETE_CONTROL_SIGNAL

  for (i = 0; i < mpk; i++) {
    YqCNOT("a", i,            "res", i + ceil_log2_mpk)
    YqCNOT("res", i + ceil_log2_mpk,  "a", i)
    YqCNOT("a", i,            "res", i + ceil_log2_mpk)
  }

  END_DEFINITION_OF_MODULE
}



void
left_cyclic_shift(MANDATORY_ARGS
/*args:*/ , uint64_t n)
{
  BEGIN_DEFINITION_OF_MODULE

  uint64_t i;

  /* Signals:
              a_of_cyclic_sh[n-1 ... 0]
  */

  for (i = n - 1; i > 0; i--) {
    YqCNOT("a_of_cyclic_sh", i,     "a_of_cyclic_sh", i - 1)
    YqCNOT("a_of_cyclic_sh", i - 1, "a_of_cyclic_sh", i)
    YqCNOT("a_of_cyclic_sh", i,     "a_of_cyclic_sh", i - 1)
  }

  END_DEFINITION_OF_MODULE
}



void
fxp_inverse(MANDATORY_ARGS
/*args:*/ , uint64_t m, uint64_t k)
{
  BEGIN_DEFINITION_OF_MODULE

  uint64_t i;
  uint64_t j;
  uint64_t mpk = m + k;
  uint64_t ceil_log2_mpk = 0; while ((((uint64_t)1) << ++ceil_log2_mpk) < mpk);

  /* Signals:
              a[m+k-1 ... 0]
              b[m+k-1 ... 0]
  */

  for (i = 0; i < mpk; i++) {
    ZERO_TO_GARBAGE_ANCILLA("copy_of_b", i);
    YqCNOT("b", i,   "copy_of_b", i)
  }

  for (i = 0; i < 2 * mpk + 2; i++) {
    ZERO_TO_GARBAGE_ANCILLA("unsigned_shifted_padded_b", i)
  }
  for (i = 0; i < mpk + 2; i++) {
    ZERO_TO_GARBAGE_ANCILLA("shifted_const_1", i)
    for (j = 0; j <= ceil_log2_mpk; j++) {
      ZERO_TO_GARBAGE_ANCILLA("1mx_pow_exp2j", j * (mpk + 2) + i)
      ZERO_TO_GARBAGE_ANCILLA("ztg2",          j * (mpk + 2) + i)
      ZERO_TO_GARBAGE_ANCILLA("ztg3",          j * (mpk + 2) + i)
    }
  }
  for (i = 0; i < mpk; i++) {
    ZERO_TO_GARBAGE_ANCILLA("clear_r", i)
  }
  for (i = 0; i < mpk - 1; i++) {
    ZERO_TO_GARBAGE_ANCILLA("shift_counter", i)
    ZERO_TO_GARBAGE_ANCILLA("ztg1", i);
  }

  YqNOT("shifted_const_1", mpk - 1)
  /*
     "shifted_const_1":
     format: `3.(mpk - 1)'

                        mpk + 1 : 0
                        mpk     : 0
                        mpk - 1 : 1
                        mpk - 2 : 0
                        mpk - 3 : 0
                        ... ... ...
  */

  PUSH_CONTROL_SIGNAL("copy_of_b", mpk - 1)
  INSTANTIATE_MODULE
    for (j = 0; j < mpk - 1; j++) {
      MAP_SIGNAL("a", j,   "copy_of_b", j)
    }
  fxp_invert_sign(LOCATION_INFO
    , mpk - 1, 0);
  POP_AND_DELETE_CONTROL_SIGNAL


  /* Copy (swap) unsigned "copy_of_b" without the highest bit to       */
  /* `unsigned_shifted_padded_b'. The target is padded with mpk+3 `0'  */
  /* highest bits.                                                     */
  for (i = 0; i < mpk - 1; i++) {
    YqCNOT("copy_of_b", i,                       "unsigned_shifted_padded_b", i)
    YqCNOT("unsigned_shifted_padded_b", i,      "copy_of_b", i)
    YqCNOT("copy_of_b", i,                       "unsigned_shifted_padded_b", i)
  }
  /*
     "unsigned_shifted_padded_b"                 "copy_of_b"

       2*mpk+1 : 0
       ... ... ...
       mpk + 1 : 0
       mpk     : 0
       mpk - 1 : 0                       mpk - 1 : sign bit of original input
       mpk - 2 : abs(b)[mpk - 2]         mpk - 2 : 0
       mpk - 3 : abs(b)[mpk - 3]         mpk - 3 : 0
       ... ... ...                       ... ... ...
  */



  /* Shift "unsigned_shifted_padded_b" to the left leaving the three */
  /* highest bits '0'.                                               */
  for (i = 0; i < mpk - 1; i++) {
    YqNOT("ztg1", i)
    YqCNOT("unsigned_shifted_padded_b", m + k - 2,  "ztg1", i)
    PUSH_CONTROL_SIGNAL("ztg1", i)
    INSTANTIATE_MODULE
      for (j = 0; j < mpk - 1; j++) {
        MAP_SIGNAL("a_of_cyclic_sh", j, "unsigned_shifted_padded_b", j)
      }
    left_cyclic_shift(LOCATION_INFO
      , mpk - 1);
    POP_AND_DELETE_CONTROL_SIGNAL

    YqCNOT("ztg1", i,  "shift_counter", i)
  }

  /*
     "unsigned_shifted_padded_b"                          "shift_counter"
                                                        may conatain between
       2*mpk+1 : 0                                      0 and (mpk - 2) ones
       ... ... ...
       mpk + 1 : 0
       mpk     : 0
       mpk - 1 : 0
       mpk - 2 : 1                                         mpk - 2 : 0
       mpk - 3 : first  bit after leading `1' of abs(b)    mpk - 3 : 0
       mpk - 4 : second bit after leading `1' of abs(b)    mpk - 4 : 0
       ... ... ...                                         ... ... ...
                                                                   : 0
                                                                   : 1
                                                                   : 1
                                                           ... ... ...
                                                                   : 1
  */

  /* Now "unsigned_shifted_padded_b" is ready. Do the factorized Taylor */
  /* series for 1/x.                                                    */
  for (i = 0; i < mpk + 2; i++) {
    YqCNOT("unsigned_shifted_padded_b", i,  "1mx_pow_exp2j", i);
  }
  INSTANTIATE_MODULE
    for (i = 0; i < mpk + 2; i++) {
      MAP_SIGNAL("a", i,   "1mx_pow_exp2j", i);
    }
  fxp_invert_sign(LOCATION_INFO
    , mpk + 2, 0);
  INSTANTIATE_MODULE
    for (i = 0; i < mpk + 2; i++) {
      MAP_SIGNAL("a", i,   "1mx_pow_exp2j", i);
      MAP_SIGNAL("b", i,   "shifted_const_1", i);
    }
  a__eq__a_plus_b(LOCATION_INFO
    , mpk + 2);

  /* "1mx_pow_exp2j" has (ceil_log2_mpk + 1) blocks of length (mpk + 2) */
  /* Now 0-th block contains 1 - unsigned_shifted_padded_b              */
  /* in `3.(mpk - 1)' fixed point format.                               */

  for (j = 1; j <= ceil_log2_mpk; j++) {
    for (i = 0; i < mpk + 2; i++) {
      YqCNOT("1mx_pow_exp2j", (j - 1) * (mpk + 2) + i,
           "ztg2" , (j - 1) * (mpk + 2) + i)
    }
    INSTANTIATE_MODULE
    for (i = 0; i < mpk + 2; i++) {
      MAP_SIGNAL("a", i,   "1mx_pow_exp2j", j * (mpk + 2) + i)
      MAP_SIGNAL("b", i,   "1mx_pow_exp2j", (j - 1) * (mpk + 2) + i)
      MAP_SIGNAL("c", i,   "ztg2", (j - 1) * (mpk + 2) + i)
    }
    fxp_mult(LOCATION_INFO
     , 3, mpk - 1);
  }
  for (j = 0; j <= ceil_log2_mpk; j++) {
    INSTANTIATE_MODULE
      for (i = 0; i < mpk + 2; i++) {
        MAP_SIGNAL("a", i,   "1mx_pow_exp2j", j * (mpk + 2) + i);
        MAP_SIGNAL("b", i,   "shifted_const_1", i);
      }
    a__eq__a_plus_b(LOCATION_INFO
      , mpk + 2);
  }
  for (i = 0; i < mpk + 2; i++) {
    YqCNOT("1mx_pow_exp2j", i,  "ztg3", i)
  }
  for (j = 1; j <= ceil_log2_mpk; j++) {
    INSTANTIATE_MODULE
    for (i = 0; i < mpk + 2; i++) {
      MAP_SIGNAL("a", i,   "ztg3", j * (mpk + 2) + i)
      MAP_SIGNAL("b", i,   "ztg3", (j - 1) * (mpk + 2) + i)
      MAP_SIGNAL("c", i,   "1mx_pow_exp2j", j * (mpk + 2) + i)
    }
    fxp_mult(LOCATION_INFO
     , 3, mpk - 1);
  }
  for (i = 0; i < mpk + 2; i++) {
    YqCNOT("unsigned_shifted_padded_b", i,
         "ztg3", ceil_log2_mpk * (mpk + 2) + i)
    YqCNOT("ztg3", ceil_log2_mpk * (mpk + 2) + i,
         "unsigned_shifted_padded_b", i)
    YqCNOT("unsigned_shifted_padded_b", i,
         "ztg3", ceil_log2_mpk * (mpk + 2) + i)
  }

  /* Shift "unsigned_shifted_padded_b" back. */
  for (i = 0; i < mpk - 1; i++) {
    PUSH_CONTROL_SIGNAL("shift_counter", i)
    INSTANTIATE_MODULE
      for (j = 0; j < 2 * mpk + 2; j++) {
        MAP_SIGNAL("a_of_cyclic_sh", j, "unsigned_shifted_padded_b", j)
      }
    left_cyclic_shift(LOCATION_INFO
      , 2 * mpk + 2);
    POP_AND_DELETE_CONTROL_SIGNAL

  }

  /* Copy "unsigned_shifted_padded_b" to the _empty_ (`0') */
  /* lower m+k-1 bits of "copy_of_b".                       */
  for (i = 0; (i < mpk - 1) && (2 * m + i - 2 < 2 * mpk + 2); i++) {
    YqCNOT("unsigned_shifted_padded_b", 2 * m + i - 2, "copy_of_b", i)
  }

  PUSH_CONTROL_SIGNAL("copy_of_b", mpk - 1)
  INSTANTIATE_MODULE
    for (j = 0; j < mpk - 1; j++) {
      MAP_SIGNAL("a", j,   "copy_of_b", j)
    }
  fxp_invert_sign(LOCATION_INFO
    , mpk - 1, 0);
  POP_AND_DELETE_CONTROL_SIGNAL


  /* Clear "a".*/
  for (i = 0; i < mpk; i++) {
    YqCNOT("a", i,       "clear_r", i)
    YqCNOT("clear_r", i,        "a", i)
    YqCNOT("a", i,       "clear_r", i)
  }
  /* Copy "copy_of_b" to "a". */
  for (i = 0; i < mpk; i++) {
    YqCNOT("copy_of_b", i, "a", i)
  }

  END_DEFINITION_OF_MODULE
}



void
fxp_exp(MANDATORY_ARGS
/*args:*/ , uint64_t m, uint64_t k)
{
  BEGIN_DEFINITION_OF_MODULE

  uint64_t i;
  uint64_t j;
  uint64_t mpk = m + k;
  uint64_t ab = 3;

  /* Signals:
              a[m+k-1 ... 0]
              b[m+k-1 ... 0]
  */

  for (i = 0; i < (2 * mpk + 2 * ab) * (mpk + ab + 1); i++) {
    ZERO_TO_GARBAGE_ANCILLA("s", i)
  }
  for (i = 0; i < 2 * mpk + 2 * ab; i++) {
    ZERO_TO_GARBAGE_ANCILLA("const_1", i)
    ZERO_TO_GARBAGE_ANCILLA("sq", i)
  }

  YqNOT("const_1", k + mpk + 2 * ab)

  for (i = 0; i < mpk; i++) {
    YqCNOT("b", i,  "s", i + ab)
  }
  for (i = ab + mpk; i < 2 * (ab + mpk); i++) {
    YqCNOT("b", mpk - 1,  "s", i)
  }

  INSTANTIATE_MODULE
    for (i = 0; i < 2 * mpk + 2 * ab; i++) {
      MAP_SIGNAL("a", i,   "s", i);
      MAP_SIGNAL("b", i,   "const_1", i);
    }
  a__eq__a_plus_b(LOCATION_INFO
    , 2 * mpk + 2 * ab);

  for (j = 1; j <= mpk + ab; j++) {
    for (i = 0; i < 2 * mpk + 2 * ab; i++) {
      YqCNOT("s", (j - 1) * (2 * mpk + 2 * ab) + i,
           "s",    j    * (2 * mpk + 2 * ab) + i)
    }
    INSTANTIATE_MODULE
    for (i = 0; i < 2 * mpk + 2 * ab; i++) {
      MAP_SIGNAL("a", i,  "sq", i)
      MAP_SIGNAL("b", i, "s", (j - 1) * (2 * mpk + 2 * ab) + i)
      MAP_SIGNAL("c", i, "s",    j    * (2 * mpk + 2 * ab) + i)
    }
    fxp_mult(LOCATION_INFO
      , m, k + mpk + 2 * ab);
    for (i = 0; i < 2 * mpk + 2 * ab; i++) {
      YqCNOT("sq", i,  "s",    j    * (2 * mpk + 2 * ab) + i)
      YqCNOT("s",    j    * (2 * mpk + 2 * ab) + i,  "sq", i)
      YqCNOT("sq", i,  "s",    j    * (2 * mpk + 2 * ab) + i)
    }
  }

  for (i = 0; i < mpk; i++) {
    YqCNOT("a", i,
         "s", (mpk + ab) * (2 * mpk + 2 * ab) + mpk + 2 * ab + i)
    YqCNOT("s", (mpk + ab) * (2 * mpk + 2 * ab) + mpk + 2 * ab + i,
         "a", i)
    YqCNOT("a", i,
         "s",  (mpk + ab) * (2 * mpk + 2 * ab) + mpk + 2 * ab + i)
  }

  END_DEFINITION_OF_MODULE
}



void
fxp_ln(MANDATORY_ARGS
/*args:*/ , uint64_t m, uint64_t k)
{
  BEGIN_DEFINITION_OF_MODULE

  uint64_t i;
  uint64_t j;
  uint64_t a;
  uint64_t mpk = m + k;
  uint64_t ceil_log2_mpk = 0; while ((((uint64_t)1) << ++ceil_log2_mpk) < mpk);

  /* Signals:
              a[m+k-1 ... 0]
              b[m+k-1 ... 0]
  */

  for (i = 0; i < mpk + ceil_log2_mpk; i++) {
    ZERO_TO_GARBAGE_ANCILLA("shifted_padded_a", i)
    ZERO_TO_GARBAGE_ANCILLA("clear_shpa", i)
    ZERO_TO_GARBAGE_ANCILLA("shifted_const_1", i)
    ZERO_TO_GARBAGE_ANCILLA("shifted_const_ln2", i)
  }
  for (i = 0; i < mpk; i++) {
    ZERO_TO_GARBAGE_ANCILLA("clear_r", i);
  }
  for (i = 0; i < mpk - 1; i++) {
    ZERO_TO_GARBAGE_ANCILLA("ztg1", i)
    ZERO_TO_GARBAGE_ANCILLA("shift_counter", i)
  }
  for (j = 0; j < k + 3; j++) {
    for (i = 0; i < mpk + ceil_log2_mpk; i++) {
      ZERO_TO_GARBAGE_ANCILLA("powers_of_1mx", j * (mpk + ceil_log2_mpk) + i)
      ZERO_TO_GARBAGE_ANCILLA("coef", j * (mpk + ceil_log2_mpk) + i)
    }
  }

  YqNOT("shifted_const_1", mpk - 1)
  YqNOT("powers_of_1mx", mpk - 1)
  YqNOT("coef", (mpk + ceil_log2_mpk) + mpk - 1)

  /* Copy "b" without the highest bit to                        */
  /* `unsigned_shifted_padded_a'. The target is padded with three `0' */
  /* highest bits.                                                    */
  for (i = 0; i < mpk - 1; i++) {
    YqCNOT("b", i,                 "shifted_padded_a", i)
  }

  /* Shift "shifted_padded_a" to the left leaving the          */
  /* ceil_log2_mpk + 1 highest bits '0'.                       */
  for (i = 0; i < mpk - 1; i++) {
    YqNOT("ztg1", i)
    YqCNOT("shifted_padded_a", m + k - 2,  "ztg1", i)
    PUSH_CONTROL_SIGNAL("ztg1", i)
    INSTANTIATE_MODULE
      for (j = 0; j < mpk - 1; j++) {
        MAP_SIGNAL("a_of_cyclic_sh", j, "shifted_padded_a", j)
      }
    left_cyclic_shift(LOCATION_INFO
      , mpk - 1);
    POP_AND_DELETE_CONTROL_SIGNAL

    YqCNOT("ztg1", i,  "shift_counter", i)
  }
  INSTANTIATE_MODULE
    for (i = 0; i < mpk + ceil_log2_mpk; i++) {
      MAP_SIGNAL("a", i, "shifted_padded_a", i)
    }
  fxp_invert_sign(LOCATION_INFO
    , mpk + ceil_log2_mpk, 0);
  INSTANTIATE_MODULE
    for (i = 0; i < mpk + ceil_log2_mpk; i++) {
      MAP_SIGNAL("a", i, "shifted_padded_a", i)
      MAP_SIGNAL("b", i, "shifted_const_1", i)
    }
  a__eq__a_plus_b(LOCATION_INFO
    , mpk + ceil_log2_mpk);

  for (j = 1; j < k + 3; j++) {
    INSTANTIATE_MODULE
      for (i = 0; i < mpk + ceil_log2_mpk; i++) {
        MAP_SIGNAL("a", i,
                   "powers_of_1mx", j * (mpk + ceil_log2_mpk) + i)
        MAP_SIGNAL("b", i,
                   "powers_of_1mx", (j - 1) * (mpk + ceil_log2_mpk) + i)
        MAP_SIGNAL("c", i,
                   "shifted_padded_a", i)
      }
    fxp_mult(LOCATION_INFO
      , ceil_log2_mpk + 1, mpk - 1);
  }

  for (j = 2; j < k + 3; j++) {
    a = 2;
    for (i = 1; i <= mpk - 1; i++) {
      if (a < j) {
        a *= 2;
        /* printf("0"); */
      } else {
        a -= j;
        a *= 2;
        YqNOT("coef", j * (mpk + ceil_log2_mpk) + mpk - 1 - i)
        /* printf("1"); */
      }
    }
    /* printf("     is 1 / %d\n", (int) j); */
  }

  for (j = 1; j < k + 3; j++) {
    INSTANTIATE_MODULE
      for (i = 0; i < mpk + ceil_log2_mpk; i++) {
        MAP_SIGNAL("a", i,
                   "shifted_padded_a", i)
        MAP_SIGNAL("b", i,
                   "powers_of_1mx", j  * (mpk + ceil_log2_mpk) + i)
        MAP_SIGNAL("c", i,
                   "coef", j * (mpk + ceil_log2_mpk) + i)
      }
    fxp_mult(LOCATION_INFO
      , ceil_log2_mpk + 1, mpk - 1);
    for (i = 0; i < mpk + ceil_log2_mpk; i++) {
      YqCNOT("shifted_padded_a", i,
           "powers_of_1mx", j  * (mpk + ceil_log2_mpk) + i)
      YqCNOT("powers_of_1mx", j  * (mpk + ceil_log2_mpk) + i,
           "shifted_padded_a", i)
      YqCNOT("shifted_padded_a", i,
           "powers_of_1mx", j  * (mpk + ceil_log2_mpk) + i)
    }
  }

  for (i = 0; i < mpk + ceil_log2_mpk; i++) {
    YqCNOT("clear_shpa", i, "shifted_padded_a", i)
    YqCNOT("shifted_padded_a", i,  "clear_shpa", i)
    YqCNOT("clear_shpa", i, "shifted_padded_a", i)
  }

  for (j = 1; j < k + 3; j++) {
    INSTANTIATE_MODULE
      for (i = 0; i < mpk + ceil_log2_mpk; i++) {
        MAP_SIGNAL("a", i,
                   "shifted_padded_a", i)
        MAP_SIGNAL("b", i,
                   "powers_of_1mx", j  * (mpk + ceil_log2_mpk) + i)
      }
    a__eq__a_minus_b(LOCATION_INFO
      , mpk + ceil_log2_mpk);
  }

  for (j = 1; j < k + 3; j++) {
    INSTANTIATE_MODULE
      for (i = 0; i < mpk + ceil_log2_mpk; i++) {
        MAP_SIGNAL("a", i, "shifted_const_ln2", i)
        MAP_SIGNAL("b", i,
                   "coef", (i + j < mpk + ceil_log2_mpk) ?
                             j * (mpk + ceil_log2_mpk) + i + j :
                             i )
      }
    a__eq__a_plus_b(LOCATION_INFO
      , mpk + ceil_log2_mpk);
  }

  for (j = 0; j < m - 1; j++) {
    INSTANTIATE_MODULE
      for (i = 0; i < mpk + ceil_log2_mpk; i++) {
        MAP_SIGNAL("a", i, "shifted_padded_a", i)
        MAP_SIGNAL("b", i, "shifted_const_ln2", i)
      }
    a__eq__a_plus_b(LOCATION_INFO
      , mpk + ceil_log2_mpk);
  }
  for (j = 0; j < mpk - 1; j++) {
    PUSH_CONTROL_SIGNAL("shift_counter", j)
    INSTANTIATE_MODULE
      for (i = 0; i < mpk + ceil_log2_mpk; i++) {
        MAP_SIGNAL("a", i, "shifted_padded_a", i)
        MAP_SIGNAL("b", i, "shifted_const_ln2", i)
      }
    a__eq__a_minus_b(LOCATION_INFO
      , mpk + ceil_log2_mpk);
    POP_AND_DELETE_CONTROL_SIGNAL

  }

  /* Clear "a".*/
  for (i = 0; i < mpk; i++) {
    YqCNOT("a", i,      "clear_r", i)
    YqCNOT("clear_r", i,      "a", i)
    YqCNOT("a", i,      "clear_r", i)
  }
  /* Copy "shifted_padded_a" to "a". */
  for (i = 0; i < ceil_log2_mpk + k + 1; i++) {
    YqCNOT("shifted_padded_a", i + m - 1, "a", i)
  }
  for (     ; i < mpk; i++) {
    YqCNOT("shifted_padded_a", mpk + ceil_log2_mpk - 1, "a", i)
  }

  END_DEFINITION_OF_MODULE
}



void
fxp_sin(MANDATORY_ARGS
/*args:*/ , uint64_t m, uint64_t k)
{
  BEGIN_DEFINITION_OF_MODULE

  uint64_t i;
  uint64_t j;
  uint64_t mpk = m + k;
  uint64_t ab = 4;
  uint64_t higest_power_of_x;

  /* Binary constant 2 * pi. 512 bits before the point and 512 after t.p. */
  char * two_pi_int_part =
    "0000000000000000000000000000000000000000000000000000000000000000"
    "0000000000000000000000000000000000000000000000000000000000000000"
    "0000000000000000000000000000000000000000000000000000000000000000"
    "0000000000000000000000000000000000000000000000000000000000000000"
    "0000000000000000000000000000000000000000000000000000000000000000"
    "0000000000000000000000000000000000000000000000000000000000000000"
    "0000000000000000000000000000000000000000000000000000000000000000"
    "0000000000000000000000000000000000000000000000000000000000000110";
  char * two_pi_frac_part =
    "0100100001111110110101010001000100001011010001100001000110100110"
    "0010011000110011000101000101110000000110111000001110011010001001"
    "0100100000010010011100000100010001010011001111100110001110100000"
    "0001000001011101111101010011000111011000100111001101100100010010"
    "1000101001010000010000111100110001110001101000000010011011101111"
    "0111110010101000110011011001111001101001110100100001100011011001"
    "1000000101011000010100110110111110010010111110001010000110111010"
    "0111111100001001101010110110101101101010100011100001001000101111";

  /* Binary constant 1 / (2 * pi). 512 bits before the p. and 512 after t.p. */
  char * inv_two_pi_int_part =
    "0000000000000000000000000000000000000000000000000000000000000000"
    "0000000000000000000000000000000000000000000000000000000000000000"
    "0000000000000000000000000000000000000000000000000000000000000000"
    "0000000000000000000000000000000000000000000000000000000000000000"
    "0000000000000000000000000000000000000000000000000000000000000000"
    "0000000000000000000000000000000000000000000000000000000000000000"
    "0000000000000000000000000000000000000000000000000000000000000000"
    "0000000000000000000000000000000000000000000000000000000000000000";
  char * inv_two_pi_frac_part =
    "0010100010111110011000001101101110010011100100010000010101001010"
    "0111111100001001110101011111010001111101010011010011011101110000"
    "0011011011011000101001010110011001001111000100001110010000010000"
    "0111111110010100010110001110101011110111101011101111000101011000"
    "0110110111001001000110111000111010010000100100110111010010111000"
    "0000000110010010010010111011101010000010011101000110010010000111"
    "0011111110000111011110101100011100101100010010100110100111001111"
    "1011101000100000100011010111110101001011101011101101000100100001";

  /* Signals:
              a[m+k-1 ... 0]
              b[m+k-1 ... 0]
  */

  /* 'b' with additional bits: b[m+k-1 ... 0] ab_of_b[ab-1 ... 0] */
  /* it is a fixed point number [m.k+ab] .                        */
  for (i = 0; i < ab; i++) {
    ZERO_TO_ZERO_ANCILLA("ab_of_b", i);
  }

  /* Number 2 * pi as   [m-1 ... 0] . [k+ab-1 ... 0]  */
  if ( (m > 512) || (k + ab > 512) ) {
    fprintf(stderr, "Error.\n");
    s_file_and_line_fprint(stderr, file, line);
    fprintf(stderr,
            " `module fxp_cos(...)' : "
            "The number of bits before or after the point "
            "is too large (greater than 512).\n");
    call_stack_fprint(stderr);
    clean_exit(-1);
  }
  for (i = 0; i < k + ab; i++) {
    if (two_pi_frac_part[k+ab-1-i] == '0') {
      ZERO_TO_ZERO_ANCILLA("two_pi", i)
    } else {
      ONE_TO_ONE_ANCILLA("two_pi", i)
    }
  }
  for (/* i = k + ab */ j = 511;
       i < mpk + ab;
       i++, j--) {
    if (two_pi_int_part[j] == '0') {
      ZERO_TO_ZERO_ANCILLA("two_pi", i)
    } else {
      ONE_TO_ONE_ANCILLA("two_pi", i)
    }
  }

  /* Number 1 / (2 * pi) as   [m-1 ... 0] . [k+ab-1 ... 0]  */
  if ( (m > 512) || (k + ab > 512) ) {
    fprintf(stderr, "Error.\n");
    s_file_and_line_fprint(stderr, file, line);
    fprintf(stderr,
            " `module fxp_cos(...)' : "
            "The number of bits before or after the point "
            "is too large (greater than 512).\n");
    call_stack_fprint(stderr);
    clean_exit(-1);
  }
  for (i = 0; i < k + ab; i++) {
    if (inv_two_pi_frac_part[k+ab-1-i] == '0') {
      ZERO_TO_ZERO_ANCILLA("inv_two_pi", i)
    } else {
      ONE_TO_ONE_ANCILLA("inv_two_pi", i)
    }
  }
  for (/* i = k + ab */ j = 511;
       i < mpk + ab;
       i++, j--) {
    if (inv_two_pi_int_part[j] == '0') {
      ZERO_TO_ZERO_ANCILLA("inv_two_pi", i)
    } else {
      ONE_TO_ONE_ANCILLA("inv_two_pi", i)
    }
  }

  /* Compute "b_div_2pi", format: [m-1 ... 0] . [k+ab-1 ... 0] */
  for (i = 0; i < mpk + ab; i++) {
    ZERO_TO_GARBAGE_ANCILLA("b_div_2pi", i)
  }
  INSTANTIATE_MODULE
  for (i = 0; i < mpk+ab; i++) {
    MAP_SIGNAL("a"/* of_mult*/, i,   "b_div_2pi", i);
    MAP_SIGNAL("b"/* of_mult*/, i,   "inv_two_pi", i);
    if (i < ab) {
      MAP_SIGNAL("c"/* of_mult*/, i,   "ab_of_b", i);
    } else {
      MAP_SIGNAL("c"/* of_mult*/, i,   "b", i - ab);
    }
  }
  fxp_mult(LOCATION_INFO
    , m, k+ab);

  /* Compute "const_one_quarter", format: [m-1 ... 0] . [k+ab-1 ... 0] */
  for (i = 0; i < mpk + ab; i++) {
    ZERO_TO_ZERO_ANCILLA("const_one_quarter", i)
  }
  YqNOT("const_one_quarter", k+ab-2)

  /* Compute "frac_b_div_2pi__minus_quarter",  */
  /* format: [m-1 ... 0] . [k+ab-1 ... 0]      */
  for (i = 0; i < mpk + ab; i++) {
    ZERO_TO_GARBAGE_ANCILLA("frac_b_div_2pi__minus_quarter", i)
  }
  for (i = 0; i < k+ab; i++) {
    YqCNOT("b_div_2pi", i,   "frac_b_div_2pi__minus_quarter", i)
  }
  INSTANTIATE_MODULE
    for (i = 0; i < mpk+ab; i++) {
      MAP_SIGNAL("a", i,   "frac_b_div_2pi__minus_quarter", i);
      MAP_SIGNAL("b", i,   "const_one_quarter", i);
    }
  a__eq__a_minus_b(LOCATION_INFO
    , mpk+ab);

  ZERO_TO_GARBAGE_ANCILLA("sign", 0)
  INSTANTIATE_MODULE
    for (i = 0; i < mpk+ab; i++) {
      MAP_SIGNAL("a", i,   "frac_b_div_2pi__minus_quarter", i);
      MAP_SIGNAL("b", i,   "const_one_quarter", i);
    }
    MAP_SIGNAL("x", 0,   "sign", 0);
  a_greater_than_or_eq_to_b__as_signed(LOCATION_INFO
    , mpk + ab);
  PUSH_CONTROL_SIGNAL("sign", 0)
  INSTANTIATE_MODULE
    for (i = 0; i < mpk+ab; i++) {
      MAP_SIGNAL("a", i,   "frac_b_div_2pi__minus_quarter", i);
      MAP_SIGNAL("b", i,   "const_one_quarter", i);
    }
  a__eq__a_minus_b(LOCATION_INFO
    , mpk+ab);
  INSTANTIATE_MODULE
    for (i = 0; i < mpk+ab; i++) {
      MAP_SIGNAL("a", i,   "frac_b_div_2pi__minus_quarter", i);
      MAP_SIGNAL("b", i,   "const_one_quarter", i);
    }
  a__eq__a_minus_b(LOCATION_INFO
    , mpk+ab);
  POP_AND_DELETE_CONTROL_SIGNAL

  /* Compute "arg_to_series", format: [m-1 ... 0] . [k+ab-1 ... 0] */
  for (i = 0; i < mpk + ab; i++) {
    ZERO_TO_GARBAGE_ANCILLA("arg_to_series", i)
  }
  INSTANTIATE_MODULE
  for (i = 0; i < mpk+ab; i++) {
    MAP_SIGNAL("a"/* of_mult*/, i,   "arg_to_series", i);
    MAP_SIGNAL("b"/* of_mult*/, i,   "two_pi", i);
    MAP_SIGNAL("c"/* of_mult*/, i,   "frac_b_div_2pi__minus_quarter", i);
  }
  fxp_mult(LOCATION_INFO
    , m, k+ab);

  higest_power_of_x = k; /* Comp. series for 1 + x + ... x^higest_power_of_x */

  for (i = 0; i < mpk + ab; i++) {
    ZERO_TO_ZERO_ANCILLA("const_1", i)
  }
  YqNOT("const_1", k+ab)

  for (i = 0; i < mpk + ab; i++) {
    ZERO_TO_GARBAGE_ANCILLA("counter", i)
  }
  YqNOT("counter", k+ab)

  for (i = 0; i < (higest_power_of_x + 1) * (mpk + ab); i++) {
    /* 1, 1/2, 1/3, ..., 1/(higest_power_of_x + 1) */
    ZERO_TO_GARBAGE_ANCILLA("inv", i)

    /* x, x^2, x^3 / 2, ..., x^(higest_power_of_x + 1) / higest_power_of_x! */
    ZERO_TO_GARBAGE_ANCILLA("aux_s", i)

    /* 1, x, x^2 / 2, ..., x^higest_power_of_x / higest_power_of_x!         */
    ZERO_TO_GARBAGE_ANCILLA("s", i)
  }

  YqNOT("inv", k+ab)
  YqNOT("s", k+ab)
  for (i = 0; i < mpk+ab; i++) {
    YqCNOT("arg_to_series", i, "aux_s", i)
  }

  for (j = 1; j <= higest_power_of_x; j++) {
    INSTANTIATE_MODULE
    for (i = 0; i < mpk+ab; i++) {
      MAP_SIGNAL("a", i,   "counter", i);
      MAP_SIGNAL("b", i,   "const_1", i);
    }
    a__eq__a_plus_b(LOCATION_INFO
      , mpk+ab);

    INSTANTIATE_MODULE
    for (i = 0; i < mpk+ab; i++) {
      MAP_SIGNAL("a", i,   "inv", j*(mpk+ab) + i);
      MAP_SIGNAL("b", i,   "counter", i);
    }
    fxp_inverse(LOCATION_INFO
      , m, k+ab);

    INSTANTIATE_MODULE
    for (i = 0; i < mpk+ab; i++) {
      MAP_SIGNAL("a"/* of_mult*/, i,   "s",     j*(mpk+ab) + i);
      MAP_SIGNAL("b"/* of_mult*/, i,   "aux_s", (j-1)*(mpk+ab) + i);
      MAP_SIGNAL("c"/* of_mult*/, i,   "inv",   (j-1)*(mpk+ab) + i);
    }
    fxp_mult(LOCATION_INFO
      , m, k+ab);

    INSTANTIATE_MODULE
    for (i = 0; i < mpk+ab; i++) {
      MAP_SIGNAL("a"/* of_mult*/, i,   "aux_s",   j*(mpk+ab) + i);
      MAP_SIGNAL("b"/* of_mult*/, i,   "s",       j*(mpk+ab) + i);
      MAP_SIGNAL("c"/* of_mult*/, i,   "arg_to_series", i);
    }
    fxp_mult(LOCATION_INFO
      , m, k+ab);

  }

  for (i = 0; i < mpk + ab; i++) {
    ZERO_TO_GARBAGE_ANCILLA("result", i)
  }

  for (j = 0; j <= higest_power_of_x; j++) {
    if (j % 4 == 0) {
      INSTANTIATE_MODULE
      for (i = 0; i < mpk+ab; i++) {
        MAP_SIGNAL("a", i,   "result", i);
        MAP_SIGNAL("b", i,   "s", j*(mpk+ab) + i);
      }
      a__eq__a_plus_b(LOCATION_INFO
        , mpk+ab);
    }
    if (j % 4 == 2) {
      INSTANTIATE_MODULE
      for (i = 0; i < mpk+ab; i++) {
        MAP_SIGNAL("a", i,   "result", i);
        MAP_SIGNAL("b", i,   "s", j*(mpk+ab) + i);
      }
      a__eq__a_minus_b(LOCATION_INFO
        , mpk+ab);
    }
  }

  PUSH_CONTROL_SIGNAL("sign", 0)
  INSTANTIATE_MODULE
    for (i = 0; i < mpk+ab; i++) {
      MAP_SIGNAL("a", i,   "result", i);
    }
    fxp_invert_sign(LOCATION_INFO
    , m, k+ab);
  POP_AND_DELETE_CONTROL_SIGNAL

  for (i = 0; i < mpk; i++) {
    YqCNOT("a", i,    "result", i + ab)
    YqCNOT("result", i + ab,    "a", i)
    YqCNOT("a", i,    "result", i + ab)
  }

  YqNOT("const_one_quarter", k+ab-2)
  YqNOT("const_1", k+ab)

  END_DEFINITION_OF_MODULE
}



void
fxp_cos(MANDATORY_ARGS
/*args:*/ , uint64_t m, uint64_t k)
{
  BEGIN_DEFINITION_OF_MODULE

  uint64_t i;
  uint64_t j;
  uint64_t mpk = m + k;
  uint64_t ab = 4;
  uint64_t higest_power_of_x;

  /* Binary constant 2 * pi. 512 bits before the point and 512 after t.p. */
  char * two_pi_int_part =
    "0000000000000000000000000000000000000000000000000000000000000000"
    "0000000000000000000000000000000000000000000000000000000000000000"
    "0000000000000000000000000000000000000000000000000000000000000000"
    "0000000000000000000000000000000000000000000000000000000000000000"
    "0000000000000000000000000000000000000000000000000000000000000000"
    "0000000000000000000000000000000000000000000000000000000000000000"
    "0000000000000000000000000000000000000000000000000000000000000000"
    "0000000000000000000000000000000000000000000000000000000000000110";
  char * two_pi_frac_part =
    "0100100001111110110101010001000100001011010001100001000110100110"
    "0010011000110011000101000101110000000110111000001110011010001001"
    "0100100000010010011100000100010001010011001111100110001110100000"
    "0001000001011101111101010011000111011000100111001101100100010010"
    "1000101001010000010000111100110001110001101000000010011011101111"
    "0111110010101000110011011001111001101001110100100001100011011001"
    "1000000101011000010100110110111110010010111110001010000110111010"
    "0111111100001001101010110110101101101010100011100001001000101111";

  /* Binary constant 1 / (2 * pi). 512 bits before the p. and 512 after t.p. */
  char * inv_two_pi_int_part =
    "0000000000000000000000000000000000000000000000000000000000000000"
    "0000000000000000000000000000000000000000000000000000000000000000"
    "0000000000000000000000000000000000000000000000000000000000000000"
    "0000000000000000000000000000000000000000000000000000000000000000"
    "0000000000000000000000000000000000000000000000000000000000000000"
    "0000000000000000000000000000000000000000000000000000000000000000"
    "0000000000000000000000000000000000000000000000000000000000000000"
    "0000000000000000000000000000000000000000000000000000000000000000";
  char * inv_two_pi_frac_part =
    "0010100010111110011000001101101110010011100100010000010101001010"
    "0111111100001001110101011111010001111101010011010011011101110000"
    "0011011011011000101001010110011001001111000100001110010000010000"
    "0111111110010100010110001110101011110111101011101111000101011000"
    "0110110111001001000110111000111010010000100100110111010010111000"
    "0000000110010010010010111011101010000010011101000110010010000111"
    "0011111110000111011110101100011100101100010010100110100111001111"
    "1011101000100000100011010111110101001011101011101101000100100001";

  /* Signals:
              a[m+k-1 ... 0]
              b[m+k-1 ... 0]
  */

  /* 'b' with additional bits: b[m+k-1 ... 0] ab_of_b[ab-1 ... 0] */
  /* it is a fixed point number [m.k+ab] .                        */
  for (i = 0; i < ab; i++) {
    ZERO_TO_ZERO_ANCILLA("ab_of_b", i);
  }

  /* Number 2 * pi as   [m-1 ... 0] . [k+ab-1 ... 0]  */
  if ( (m > 512) || (k + ab > 512) ) {
    fprintf(stderr, "Error.\n");
    s_file_and_line_fprint(stderr, file, line);
    fprintf(stderr,
            " `module fxp_cos(...)' : "
            "The number of bits before or after the point "
            "is too large (greater than 512).\n");
    call_stack_fprint(stderr);
    clean_exit(-1);
  }
  for (i = 0; i < k + ab; i++) {
    if (two_pi_frac_part[k+ab-1-i] == '0') {
      ZERO_TO_ZERO_ANCILLA("two_pi", i)
    } else {
      ONE_TO_ONE_ANCILLA("two_pi", i)
    }
  }
  for (/* i = k + ab */ j = 511;
       i < mpk + ab;
       i++, j--) {
    if (two_pi_int_part[j] == '0') {
      ZERO_TO_ZERO_ANCILLA("two_pi", i)
    } else {
      ONE_TO_ONE_ANCILLA("two_pi", i)
    }
  }

  /* Number 1 / (2 * pi) as   [m-1 ... 0] . [k+ab-1 ... 0]  */
  if ( (m > 512) || (k + ab > 512) ) {
    fprintf(stderr, "Error.\n");
    s_file_and_line_fprint(stderr, file, line);
    fprintf(stderr,
            " `module fxp_cos(...)' : "
            "The number of bits before or after the point "
            "is too large (greater than 512).\n");
    call_stack_fprint(stderr);
    clean_exit(-1);
  }
  for (i = 0; i < k + ab; i++) {
    if (inv_two_pi_frac_part[k+ab-1-i] == '0') {
      ZERO_TO_ZERO_ANCILLA("inv_two_pi", i)
    } else {
      ONE_TO_ONE_ANCILLA("inv_two_pi", i)
    }
  }
  for (/* i = k + ab */ j = 511;
       i < mpk + ab;
       i++, j--) {
    if (inv_two_pi_int_part[j] == '0') {
      ZERO_TO_ZERO_ANCILLA("inv_two_pi", i)
    } else {
      ONE_TO_ONE_ANCILLA("inv_two_pi", i)
    }
  }

  /* Compute "b_div_2pi", format: [m-1 ... 0] . [k+ab-1 ... 0] */
  for (i = 0; i < mpk + ab; i++) {
    ZERO_TO_GARBAGE_ANCILLA("b_div_2pi", i)
  }
  INSTANTIATE_MODULE
  for (i = 0; i < mpk+ab; i++) {
    MAP_SIGNAL("a"/* of_mult*/, i,   "b_div_2pi", i);
    MAP_SIGNAL("b"/* of_mult*/, i,   "inv_two_pi", i);
    if (i < ab) {
      MAP_SIGNAL("c"/* of_mult*/, i,   "ab_of_b", i);
    } else {
      MAP_SIGNAL("c"/* of_mult*/, i,   "b", i - ab);
    }
  }
  fxp_mult(LOCATION_INFO
    , m, k+ab);

  /* Compute "const_one_quarter", format: [m-1 ... 0] . [k+ab-1 ... 0] */
  for (i = 0; i < mpk + ab; i++) {
    ZERO_TO_ZERO_ANCILLA("const_one_quarter", i)
  }
  YqNOT("const_one_quarter", k+ab-2)

  /* This is cosine, add 1/4. */
  INSTANTIATE_MODULE
    for (i = 0; i < mpk+ab; i++) {
      MAP_SIGNAL("a", i,   "b_div_2pi", i);
      MAP_SIGNAL("b", i,   "const_one_quarter", i);
    }
  a__eq__a_plus_b(LOCATION_INFO
    , mpk+ab);

  /* Compute "frac_b_div_2pi__minus_quarter",  */
  /* format: [m-1 ... 0] . [k+ab-1 ... 0]      */
  for (i = 0; i < mpk + ab; i++) {
    ZERO_TO_GARBAGE_ANCILLA("frac_b_div_2pi__minus_quarter", i)
  }
  for (i = 0; i < k+ab; i++) {
    YqCNOT("b_div_2pi", i,   "frac_b_div_2pi__minus_quarter", i)
  }
  INSTANTIATE_MODULE
    for (i = 0; i < mpk+ab; i++) {
      MAP_SIGNAL("a", i,   "frac_b_div_2pi__minus_quarter", i);
      MAP_SIGNAL("b", i,   "const_one_quarter", i);
    }
  a__eq__a_minus_b(LOCATION_INFO
    , mpk+ab);

  ZERO_TO_GARBAGE_ANCILLA("sign", 0)
  INSTANTIATE_MODULE
    for (i = 0; i < mpk+ab; i++) {
      MAP_SIGNAL("a", i,   "frac_b_div_2pi__minus_quarter", i);
      MAP_SIGNAL("b", i,   "const_one_quarter", i);
    }
    MAP_SIGNAL("x", 0,   "sign", 0);
  a_greater_than_or_eq_to_b__as_signed(LOCATION_INFO
    , mpk + ab);
  PUSH_CONTROL_SIGNAL("sign", 0)
  INSTANTIATE_MODULE
    for (i = 0; i < mpk+ab; i++) {
      MAP_SIGNAL("a", i,   "frac_b_div_2pi__minus_quarter", i);
      MAP_SIGNAL("b", i,   "const_one_quarter", i);
    }
  a__eq__a_minus_b(LOCATION_INFO
    , mpk+ab);
  INSTANTIATE_MODULE
    for (i = 0; i < mpk+ab; i++) {
      MAP_SIGNAL("a", i,   "frac_b_div_2pi__minus_quarter", i);
      MAP_SIGNAL("b", i,   "const_one_quarter", i);
    }
  a__eq__a_minus_b(LOCATION_INFO
    , mpk+ab);
  POP_AND_DELETE_CONTROL_SIGNAL

  /* Compute "arg_to_series", format: [m-1 ... 0] . [k+ab-1 ... 0] */
  for (i = 0; i < mpk + ab; i++) {
    ZERO_TO_GARBAGE_ANCILLA("arg_to_series", i)
  }
  INSTANTIATE_MODULE
  for (i = 0; i < mpk+ab; i++) {
    MAP_SIGNAL("a"/* of_mult*/, i,   "arg_to_series", i);
    MAP_SIGNAL("b"/* of_mult*/, i,   "two_pi", i);
    MAP_SIGNAL("c"/* of_mult*/, i,   "frac_b_div_2pi__minus_quarter", i);
  }
  fxp_mult(LOCATION_INFO
    , m, k+ab);

  higest_power_of_x = k; /* Comp. series for 1 + x + ... x^higest_power_of_x */

  for (i = 0; i < mpk + ab; i++) {
    ZERO_TO_ZERO_ANCILLA("const_1", i)
  }
  YqNOT("const_1", k+ab)

  for (i = 0; i < mpk + ab; i++) {
    ZERO_TO_GARBAGE_ANCILLA("counter", i)
  }
  YqNOT("counter", k+ab)

  for (i = 0; i < (higest_power_of_x + 1) * (mpk + ab); i++) {
    /* 1, 1/2, 1/3, ..., 1/(higest_power_of_x + 1) */
    ZERO_TO_GARBAGE_ANCILLA("inv", i)

    /* x, x^2, x^3 / 2, ..., x^(higest_power_of_x + 1) / higest_power_of_x! */
    ZERO_TO_GARBAGE_ANCILLA("aux_s", i)

    /* 1, x, x^2 / 2, ..., x^higest_power_of_x / higest_power_of_x!         */
    ZERO_TO_GARBAGE_ANCILLA("s", i)
  }

  YqNOT("inv", k+ab)
  YqNOT("s", k+ab)
  for (i = 0; i < mpk+ab; i++) {
    YqCNOT("arg_to_series", i, "aux_s", i)
  }

  for (j = 1; j <= higest_power_of_x; j++) {
    INSTANTIATE_MODULE
    for (i = 0; i < mpk+ab; i++) {
      MAP_SIGNAL("a", i,   "counter", i);
      MAP_SIGNAL("b", i,   "const_1", i);
    }
    a__eq__a_plus_b(LOCATION_INFO
      , mpk+ab);

    INSTANTIATE_MODULE
    for (i = 0; i < mpk+ab; i++) {
      MAP_SIGNAL("a", i,   "inv", j*(mpk+ab) + i);
      MAP_SIGNAL("b", i,   "counter", i);
    }
    fxp_inverse(LOCATION_INFO
      , m, k+ab);

    INSTANTIATE_MODULE
    for (i = 0; i < mpk+ab; i++) {
      MAP_SIGNAL("a"/* of_mult*/, i,   "s",     j*(mpk+ab) + i);
      MAP_SIGNAL("b"/* of_mult*/, i,   "aux_s", (j-1)*(mpk+ab) + i);
      MAP_SIGNAL("c"/* of_mult*/, i,   "inv",   (j-1)*(mpk+ab) + i);
    }
    fxp_mult(LOCATION_INFO
      , m, k+ab);

    INSTANTIATE_MODULE
    for (i = 0; i < mpk+ab; i++) {
      MAP_SIGNAL("a"/* of_mult*/, i,   "aux_s",   j*(mpk+ab) + i);
      MAP_SIGNAL("b"/* of_mult*/, i,   "s",       j*(mpk+ab) + i);
      MAP_SIGNAL("c"/* of_mult*/, i,   "arg_to_series", i);
    }
    fxp_mult(LOCATION_INFO
      , m, k+ab);

  }

  for (i = 0; i < mpk + ab; i++) {
    ZERO_TO_GARBAGE_ANCILLA("result", i)
  }

  for (j = 0; j <= higest_power_of_x; j++) {
    if (j % 4 == 0) {
      INSTANTIATE_MODULE
      for (i = 0; i < mpk+ab; i++) {
        MAP_SIGNAL("a", i,   "result", i);
        MAP_SIGNAL("b", i,   "s", j*(mpk+ab) + i);
      }
      a__eq__a_plus_b(LOCATION_INFO
        , mpk+ab);
    }
    if (j % 4 == 2) {
      INSTANTIATE_MODULE
      for (i = 0; i < mpk+ab; i++) {
        MAP_SIGNAL("a", i,   "result", i);
        MAP_SIGNAL("b", i,   "s", j*(mpk+ab) + i);
      }
      a__eq__a_minus_b(LOCATION_INFO
        , mpk+ab);
    }
  }

  PUSH_CONTROL_SIGNAL("sign", 0)
  INSTANTIATE_MODULE
    for (i = 0; i < mpk+ab; i++) {
      MAP_SIGNAL("a", i,   "result", i);
    }
    fxp_invert_sign(LOCATION_INFO
    , m, k+ab);
  POP_AND_DELETE_CONTROL_SIGNAL

  for (i = 0; i < mpk; i++) {
    YqCNOT("a", i,    "result", i + ab)
    YqCNOT("result", i + ab,    "a", i)
    YqCNOT("a", i,    "result", i + ab)
  }

  YqNOT("const_one_quarter", k+ab-2)
  YqNOT("const_1", k+ab)

  END_DEFINITION_OF_MODULE
}



