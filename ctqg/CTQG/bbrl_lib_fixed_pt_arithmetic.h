#ifndef BBRL_LIB_FIXED_PT_ARITHMETIC_H
#define BBRL_LIB_FIXED_PT_ARITHMETIC_H



#include <bbrl________________________.h>
#include <stdint.h>



/* Comparator: Inverses r_of_comparator if a < b as     */
/* unsigned. Does not change a and b.                   */
/*                                                      */
/* Zero-to-garbage bits : 0                             */
/*                                                      */
/* TOFFOLI : 4n - 3                                     */
/* CNOT    : 12n - 5                                    */
/* NOT     : 0                                          */
/*                                                      */
/* Signals:                                             */
/*          a_of_comparator[n-1 ... 0]                  */
/*          b_of_comparator[n-1 ... 0]                  */
/*          r_of_comparator                             */
void
a_less_than_b__as_unsigned_n_bit_integers(MANDATORY_ARGS
/*args:*/ , uint64_t n);



/* Change the sign to the opposite for an m.k bit fx. point number: a = -a . */
/*                                                                           */
/* Zero-to-garbage bits : 0                                                  */
/*                                                                           */
/* TOFFOLI : 0                                                               */
/* CNOT    : 0                                                               */
/* NOT     : m+k                                                             */
/*                                                                           */
/* Signals:                                                                  */
/*          a[m+k-1 ... 0]                                                   */
void
fxp_invert_sign(MANDATORY_ARGS
/*args:*/ , uint64_t m, uint64_t k);



/* Absolute value of a fx. point number: a = |a|.          */
/*                                                         */
/* Zero-to-garbage bits : 1                                */
/*                                                         */
/* TOFFOLI : 0                                             */
/* CNOT    : m+k+2                                         */
/* NOT     : 0                                             */
/*                                                         */
/* Signals:                                                */
/*          a[m+k-1 ... 0]                                 */
void
fxp_abs(MANDATORY_ARGS
/*args:*/ , uint64_t m, uint64_t k);



/* Floor of an m.k - bit fixed point number : a = floor(a). */
/*                                                          */
/* Zero-to-garbage bits : k                                 */
/*                                                          */
/* TOFFOLI : 0                                              */
/* CNOT    : 3k                                             */
/* NOT     : 0                                              */
/*                                                          */
/* Signals:                                                 */
/*          a[m+k-1 ... 0]                                  */
void
fxp_floor(MANDATORY_ARGS
/*args:*/ , uint64_t m, uint64_t k);



/* Multiplier for  m.k - bit fixed point numbers : a = bc.  */
/*                                                          */
/* Zero-to-garbage bits : m + k + 3*ceiling(log_2(m+k))     */
/*                                                          */
/* TOFFOLI : 7*(m+k+ceiling(log_2(m+k)))                    */
/* CNOT    : 15*(m+k+ceiling(log_2(m+k)))^2                 */
/* NOT     : 0                                              */
/*                                                          */
/* Signals:                                                 */
/*          a[m+k-1 ... 0]                                  */
/*          b[m+k-1 ... 0]                                  */
/*          c[m+k-1 ... 0]                                  */
void
fxp_mult(MANDATORY_ARGS
/*args:*/ , uint64_t m, uint64_t k);



/* Multiplicative inverse of  m.k - bit fixed point numbers : a = 1/b.  */
/*                                                                      */
/* Zero-to-garbage bits :                                               */
/*                                                                      */
/* TOFFOLI :                                                            */
/* CNOT    :                                                            */
/* NOT     :                                                            */
/*                                                                      */
/* Signals:                                                             */
/*          a[m+k-1 ... 0]                                              */
/*          b[m+k-1 ... 0]                                              */
void
fxp_inverse(MANDATORY_ARGS
/*args:*/ , uint64_t m, uint64_t k);



/* Exponent of  m.k - bit fixed point numbers : a = e^b.    */
/*                                                          */
/* Zero-to-garbage bits : 6 * (m+k)^2                       */
/*                                                          */
/* TOFFOLI : 102 * (m+k)^3                                  */
/* CNOT    :                                                */
/* NOT     :                                                */
/*                                                          */
/* Signals:                                                 */
/*          a[m+k-1 ... 0]                                  */
/*          b[m+k-1 ... 0]                                  */
void
fxp_exp(MANDATORY_ARGS
/*args:*/ , uint64_t m, uint64_t k);



/* Logarithm of  m.k - bit fixed point numbers : r = ln(a). */
/*                                                          */
/* Zero-to-garbage bits :                                   */
/*                                                          */
/* TOFFOLI :                                                */
/* CNOT    :                                                */
/* NOT     :                                                */
/*                                                          */
/* Signals:                                                 */
/*          a[m+k-1 ... 0]                                  */
/*          b[m+k-1 ... 0]                                  */
void
fxp_ln(MANDATORY_ARGS
/*args:*/ , uint64_t m, uint64_t k);



/* Sine of  m.k - bit fixed point numbers : a = sin(b).     */
/*                                                          */
/* Zero-to-garbage bits :                                   */
/*                                                          */
/* TOFFOLI :                                                */
/* CNOT    :                                                */
/* NOT     :                                                */
/*                                                          */
/* Signals:                                                 */
/*          a[m+k-1 ... 0]                                  */
/*          b[m+k-1 ... 0]                                  */
void
fxp_sin(MANDATORY_ARGS
/*args:*/ , uint64_t m, uint64_t k);



/* Cosine of  m.k - bit fixed point numbers : a = cos(b).   */
/*                                                          */
/* Zero-to-garbage bits :                                   */
/*                                                          */
/* TOFFOLI :                                                */
/* CNOT    :                                                */
/* NOT     :                                                */
/*                                                          */
/* Signals:                                                 */
/*          a[m+k-1 ... 0]                                  */
/*          b[m+k-1 ... 0]                                  */
void
fxp_cos(MANDATORY_ARGS
/*args:*/ , uint64_t m, uint64_t k);



#endif
