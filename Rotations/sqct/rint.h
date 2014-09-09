//     Copyright (c) 2012 Vadym Kliuchnikov sqct(dot)software(at)gmail(dot)com, Dmitri Maslov, Michele Mosca
//
//     This file is part of SQCT.
// 
//     SQCT is free software: you can redistribute it and/or modify
//     it under the terms of the GNU Lesser General Public License as published by
//     the Free Software Foundation, either version 3 of the License, or
//     (at your option) any later version.
// 
//     SQCT is distributed in the hope that it will be useful,
//     but WITHOUT ANY WARRANTY; without even the implied warranty of
//     MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//     GNU Lesser General Public License for more details.
// 
//     You should have received a copy of the GNU Lesser General Public License
//     along with SQCT.  If not, see <http://www.gnu.org/licenses/>.
// 

#ifndef RINT_OPT_PROOF_H
#define RINT_OPT_PROOF_H

#include <gmpxx.h>
#include <complex>
#include "hprhelpers.h"

/// \brief Finds sde given denominator exponent and gde, see formula (1) http://arxiv.org/abs/1206.5236
int sde( int denom_exponent, int gde );
/// \brief Returns canonical form of complex number with complex argument in \f$ [0,\pi/8] \f$
std::complex<double> canonical( const std::complex<double>& val, int& w_power, bool& conj );

/// \brief Specifies type of product of two elements of type T
template< class T>
struct product_type
{
    /// \brief Product type
    typedef T pr_type;
};

/// \brief Specifies that procut of two int number must be of type long
template<>
struct product_type<int>
{
    /// \brief Product type
    typedef long int pr_type;
};

template < class TInt = long int >
struct ring_int_real;

/// \brief Integer in the ring of the form  \f$ a + \omega b + \omega ^ 2 c + \omega ^3 d,\f$
/// for\f$ \omega = e^{ i \frac{\pi}{4} k }\f$ and
/// integers \f$ a,b,c,d \f$.
///
/// \tparam TInt Defines the underlying ring to use ( int, long int, arbitrary precision integers -- mpz_class )
/// \note Precompiled for Tint in {int, long int, mpz_class, resring <8>}
template < class TInt = long int >
struct ring_int
{
    /// \brief Type used for high precision floating point data
    typedef hprHelpers::hpr_real mpclass;
    /// \brief Type of underlying ring
    typedef TInt int_type;
    /// \brief Type of product two elements of int_type
    typedef typename product_type< int_type >::pr_type pr_type;

    /// \brief Convert between different underlying rings
    template<class T>
    explicit ring_int( const ring_int<T>& val );
    /// \brief Value of new object is \f$ 0 + \omega 0 + \omega ^ 2 0 + \omega ^3 0,\f$
    ring_int();
    /// \brief Value of new object is \f$ a + \omega b + \omega ^ 2 c + \omega ^3 d,\f$
    ring_int ( int_type a, int_type b, int_type c, int_type d);
    /// \brief Value of new object is \f$ a + \omega 0 + \omega ^ 2 0 + \omega ^3 0,\f$
    ring_int ( int_type a );
    /// \brief Sets value to \f$ a + \omega b + \omega ^ 2 c + \omega ^3 d,\f$
    void set( int_type a, int_type b, int_type c, int_type d);
    ring_int& operator =(const ring_int& b);
    ring_int (const ring_int& b);

    /// \brief Returns canonical form of the number \f$x\f$, such that \f$Arg(x)\f$ belongs to \f$[0, \pi/8 )\f$
    ring_int canonical() const;

    /// \brief Ring addition
    ring_int& operator +=(const ring_int& y);
    /// \brief Ring substruction
    ring_int& operator -=(const ring_int& y);
    /// \brief Ring addition
    ring_int operator +(const ring_int& y) const;
    /// \brief Ring substruction
    ring_int operator -(const ring_int& y) const;
    /// \brief Ring additive inverse
    ring_int operator -() const;
    /// \brief Ring multiplication
    ring_int operator *(const ring_int& y) const;
    /// \brief Integer division of all integer coefficients by e
    ring_int operator /( int_type e) const;
    /// \brief Integer division of all integer coefficients by e
    ring_int& operator /=( int_type e);
    /// \brief Multiplies by 2^e
    ring_int& operator <<=( long int e);
    /// \brief Divides by 2^e
    ring_int& operator >>=( long int e);
    /// \brief Coefficient near \f$ \omega ^ k, 0 \le k \le 3 \f$
    const int_type& operator[] ( int k ) const;
    /// \brief Coefficient near \f$ \omega ^ k, 0 \le k \le 3 \f$
    int_type& operator[] ( int k );

    /// \brief Divides by \f$ \sqrt{2}^k \f$
    void div_eq_sqrt2( int k );
    /// \brief Divides by \f$ \sqrt{2} \f$
    void div_eq_sqrt2();
    /// \brief Divides by \f$ 2 \f$
    void div_eq_2();
    /// \brief Multiplies by \f$ \omega^k \f$
    void mul_eq_w( int k );
    /// \brief Multiplies by \f$ \omega^k \f$
    void mul_w( int k, ring_int<TInt>& out ) const;
    /// \brief Multiplies by \f$ \omega \f$
    void mul_eq_w();
    /// \brief Multiplies by \f$ \sqrt{2} \f$
    void mul_eq_sqrt2();
    /// \brief Computes gde of a base \f$ \sqrt{2} \f$
    /// \see Section 4 of http://arxiv.org/abs/1206.5236 for defintion and discussion
    int gde() const;
    /// \brief Returns true if all entries divisible by 2, false otherwise
    bool is_div_2() const;
    /// \brief Computes \f$ P(x) \f$
    /// \see Section 5 of http://arxiv.org/abs/1206.5236 for defintion and discussion
    pr_type ipxx() const;
    /// \brief Computes \f$ Q(x) \f$
    /// \see Section 5 of http://arxiv.org/abs/1206.5236 for defintion and discussion
    pr_type ipQxx() const;
    /// \brief Computes absolute value squared
    ring_int_real< pr_type > abs2() const;
     /// \brief Computes Chebyshev distance between integer coordinates
    int_type max_dist(const ring_int& y) const;

    /// \brief Returns \f$ e^{ i \frac{\pi}{4} k } \f$
    static ring_int omega( int k);
    /// \brief Returns \f$ \sqrt{2} = \omega - \omega^3 \f$
    static ring_int sqrt2();
    /// \brief Returns Complex conjugate
    ring_int conjugate() const;
    /// \brief Sets value to its complex conjugate
    void conjugate_eq();

    /// \brief Returns complex double precision number approximately equal to
    /// \f$ \frac{1}{\sqrt{2}^d }( a + \omega b + \omega ^ 2 c + \omega ^3 d)\f$
    /// \param d Power of \f$ \sqrt{2} \f$ in the denominator
    std::complex<double> toComplex( int d ) const;

    /// \brief Finds complex double precision number approximately equal to
    /// \f$ z = \frac{1}{\sqrt{2}^d }( a + \omega b + \omega ^ 2 c + \omega ^3 d)\f$
    /// \param d Power of \f$ \sqrt{2} \f$ in the denominator
    /// \param re Contains \f$ Re(z)\f$ after fuinction call
    /// \param im Contains \f$ Im(z)\f$ after fuinction call
    void toComplex( int d, double& re, double& im ) const;

    /// \brief Returns complex high precision number approximately equal to
    /// \f$ \frac{1}{\sqrt{2}^d }( a + \omega b + \omega ^ 2 c + \omega ^3 d)\f$
    /// \param d Power of \f$ \sqrt{2} \f$ in the denominator
    std::complex<mpclass> toHprComplex( int d ) const;

    /// \brief Lexicographical order. Returns 1 if \f$x < y\f$, 0 if \f$x = y\f$, -1 if \f$x > y.\f$
    int le( const ring_int& y ) const;
    /// \brief Lexicographical order
    bool operator <( const ring_int& y ) const;
    /// \brief Returns true if two numbers are equal
    bool operator ==( const ring_int& y ) const;
    /// \brief Returns true if two numbers are not equal
    bool operator !=( const ring_int& y ) const;

    /// \brief Returns true if 2 divides x
    static bool is_div_2( pr_type e );
    /// \brief Computes gde of base 2 of integer
    static int gde2 ( pr_type e );
    /// \brief Returns true if gde of a base \f$ \sqrt{2} \f$ of \f$ a + \sqrt{2} b \f$ is 1
    static int isGde1 ( pr_type a, pr_type b );
    /// \brief Returns true if gde of a base \f$ \sqrt{2} \f$ of \f$ a + \sqrt{2} b \f$ is 0
    static int isGde0 ( pr_type a, pr_type b );
    /// \brief Returns true if gde of a base \f$ \sqrt{2} \f$ of \f$ a + \sqrt{2} b \f$ is 2
    static int isGde2 ( pr_type a, pr_type b );
    /// \brief Returns true if two integers can form a column
    bool is_compl( const ring_int& v) const;
    /// \brief Returns true if number is real
    bool is_im_eq0() const;
protected:
    /// \brief Stores integer coefficients of the ring integer
    int_type v[4];
};

/// \brief Represents real integers in the ring.
template < class TInt >
struct ring_int_real : public ring_int<TInt>
{
    /// \brief Basic type
    typedef ring_int<TInt> base;
    /// \brief Type of underlying ring
    typedef TInt int_type;
    /// \brief Sets value to \f$0\f$
    ring_int_real() = default;
    /// \brief Sets value to \f$a+\sqrt{2}b\f$
    ring_int_real ( int_type a, int_type b );
    /// \brief Fast algorithm for computation of gde. See discussion in Section 5 of http://arxiv.org/abs/1206.5236
    int gde() const;
};

#endif // RINT_OPT_PROOF_H
