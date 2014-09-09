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

#ifndef MATRIX2X2_H
#define MATRIX2X2_H

#include "rint.h"
#include "vector2.h"

/// \brief Matrix with entries in the ring \f$ \mathbb{Z}[\frac{1}{\sqrt{2}},i]\f$
/// \see http://arxiv.org/abs/1206.5236 for more details.
/// \note We use notation \f$ \omega = e^{i \frac{\pi}{4}}\f$
/// \note Precompiled for Tint in {int, long int, mpz_class, resring <8>}
template < class TInt = long int >
struct matrix2x2
{
    /// \brief Integer parts of all entries, they have denominator sqrt(2)^de
    ring_int<TInt> d[2][2];
    /// \brief  Denominator exponent of a base sqrt(2) of entries, must be non-negative
    int de;

    /// Matrix multiplication, denominator exponents add up
    matrix2x2 operator*( const matrix2x2& b ) const;
    /// Element-wise multiplication by constant b
    matrix2x2 operator*( const ring_int<TInt>& b ) const;
    /// Element-wise multiplication by constant b
    void mul_eq_w( int k = 1 );
    /// \brief Divides by \f$ \sqrt{2}^a \f$
    matrix2x2& div_eq_sqrt2_exp(int a);

    /// \brief Sets matrix value to
    /// \f$ \frac{1}{\sqrt{2}^{dp}}\left(\begin{array}{cc} z & u\\w & v\end{array}\right) \f$
    void set( const ring_int<TInt>& z, const ring_int<TInt>& u, const ring_int<TInt>& w, const ring_int<TInt>& v, int dp );
    /// \brief Initiualizes matrix value to
    /// \f$ \frac{1}{\sqrt{2}^{dp}}\left(\begin{array}{cc} z & u\\w & v\end{array}\right) \f$
    matrix2x2( const ring_int<TInt>& z, const ring_int<TInt>& u, const ring_int<TInt>& w, const ring_int<TInt>& v, int dp = 0 );
    /// \brief Creates zero matrix
    matrix2x2();

    /// \brief Computes minimal gde of integer parts of  entries
    int min_gde() const;
    /// \brief Computes minimal gde of absolute values squared of integer parts of  entries
    int min_gde_abs2() const;
    /// \brief Assumes denominator exponent (de) >= 0, reduces it if possible and ensures its non negativity
    /// \returns Denominator exponent
    int reduce();
    /// \brief Reduces denominator exponent by number
    int reduce(int number);
    /// \brief Matrix complexity measure
    int max_sde_abs2() const;
    /// \brief Multiplies by \f$ T^k H \f$
    void mul_TkH( int k, matrix2x2<TInt>& out );

    /// \brief Returns T^power, where T is a unitary correspoding to T gate
    static matrix2x2<TInt> T( int power = 1 );
    /// \brief Returns a unitary correspoding to Hadamard gate
    static matrix2x2<TInt> H();
    /// \brief Returns a unitary correspoding to Phase gate
    static matrix2x2<TInt> P();
    /// \brief Returns a unitary correspoding to Pauli Z gate
    static matrix2x2<TInt> Z();
    /// \brief Returns a unitary correspoding to Pauli X gate
    static matrix2x2<TInt> X();
    /// \brief Returns a unitary correspoding to Pauli Y gate
    static matrix2x2<TInt> Y();
    /// \brief Returns an identity multiplied by \f$ \omega^k \f$
    static matrix2x2<TInt> Id( int k = 0 );

    /// \brief Allows to convert between matrices using different underlying rings.
    /// Conversion is entry wise
    template < class T >
    matrix2x2( const matrix2x2<T>& val );
    /// \brief Returns conjugate transpose of the matrix
    matrix2x2<TInt> conjugateTranspose();

    /// \brief Returns true if matrix is unitary
    bool is_unitary() const;

    /// \brief Assignment operator
    matrix2x2<TInt>& operator= ( const vector2<TInt>& v );

    /// \brief Lexicographical order, assumes A,B
    /// in canonical form ( with smallest possible power of \f$ \sqrt{2} \f$ denominator )
    bool operator <( const matrix2x2<TInt>& B ) const;
    /// \brief Compares integer parts of two matrixes and returns true if they are equal
    bool operator ==( const matrix2x2<TInt>& B ) const;
};

/// \brief Type for machine precision 2x2 matrices
typedef std::complex<double> matrix2x2cd[2][2];

/// \brief Represents 2x2 matrix with high precision entries
struct matrix2x2hpr
{
    /// \brief High precision real type
    typedef ring_int<int>::mpclass mpclass;
    /// \brief High precision complex type
    typedef std::complex<mpclass> scalar;
    /// \brief Machine precision complex
    typedef std::complex<double> cd;
    /// \brief Machine precision real
    typedef double db;
    /// \brief Matrix entries
    scalar d[2][2];

    /// \brief Conversion from exact unitaries
    template < class TInt >
    matrix2x2hpr( const matrix2x2<TInt>& val );
    /// \brief Matrix multiplication
    matrix2x2hpr operator*( const matrix2x2hpr& b ) const;
    /// \brief Matrix addition
    matrix2x2hpr operator+( const matrix2x2hpr& b ) const;
    /// \brief Matrix sustruction
    matrix2x2hpr operator-( const matrix2x2hpr& b ) const;
    /// \brief Multiplication by real scalar
    matrix2x2hpr operator*( const mpclass& val ) const;
    /// \brief Multiplication by complex scalar
    matrix2x2hpr operator*( const scalar& val ) const;
    /// \brief Assignement operator
    matrix2x2hpr& operator= ( const matrix2x2hpr& v );
    /// \brief Conjugate transpose
    matrix2x2hpr adjoint() const;

    /// \brief Default constructor that sets all entries to 0
    matrix2x2hpr();
    /// \brief Copy constructor
    matrix2x2hpr( const matrix2x2hpr& val );
    /// \brief Sets matrix value to
    /// \f$ \left(\begin{array}{cc} a & b\\ c & d\end{array}\right) \f$
    matrix2x2hpr( scalar a, scalar b, scalar c, scalar _d);
    /// \brief Sets matrix value to
    /// \f$ \left(\begin{array}{cc} a & b\\ c & d\end{array}\right) \f$
    matrix2x2hpr( cd a, cd b, cd c, cd _d);
    /// \brief Copies values from machine precision matrix
    matrix2x2hpr( const matrix2x2cd& val );
    /// \brief Returns trace of the matrix
    scalar trace() const;
    /// \brief Provide access to matrix elements
    const scalar& operator() ( int a, int b ) const
    { return d[a][b]; }
    /// \brief Frobenius distance between matrices
    double dist(  const matrix2x2hpr& matr ) const;

    /// \brief Returns reference to matrix2x2hpr initialized to Identity matrix
    static const matrix2x2hpr& Id();
    /// \brief Returns reference to matrix2x2hpr initialized to Pauli X matrix
    static const matrix2x2hpr& X();
    /// \brief Returns reference to matrix2x2hpr initialized to Pauli Y matrix
    static const matrix2x2hpr& Y();
    /// \brief Returns reference to matrix2x2hpr initialized to Pauli Z matrix
    static const matrix2x2hpr& Z();
};

/// \brief Multiplication by high precision real number from the left side
matrix2x2hpr operator*( const matrix2x2hpr::mpclass& val, const matrix2x2hpr& rhs );
/// \brief Multiplication by high precision complex number from the left side
matrix2x2hpr operator*( const matrix2x2hpr::scalar& val, const matrix2x2hpr& rhs );

/// \brief Trace distance between two matrices, \see Formula (1) in http://arxiv.org/abs/quant-ph/0411206
double trace_dist( const matrix2x2hpr& a, const matrix2x2hpr& b );

/// \brief Converts high precision complex matrix to machine precision complex matrix
void convert(const matrix2x2hpr& in, matrix2x2cd &out);


#endif // MATRIX2X2_H
