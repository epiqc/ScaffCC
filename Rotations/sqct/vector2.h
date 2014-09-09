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

#ifndef VECTOR2_H
#define VECTOR2_H

#include "rint.h"

/// \brief Vector with entries in the ring \f$ \mathbb{Z}[\frac{1}{\sqrt{2}},i] \f$
/// \see http://arxiv.org/abs/1206.5236 for more details.
/// \note Precompiled for Tint in {int, long int, mpz_class, resring <8>}
template < class TInt = long int >
struct vector2
{
    /// \brief Integer parts of all entries, all entries have denominator sqrt(2)^de
    ring_int<TInt> d[2];
    /// \brief Denominator exponent of a base sqrt(2) of entries, non-negative
    int de;

    vector2( const ring_int<TInt>& z, const ring_int<TInt>& w, int denom_exp = 0 );
    vector2();
    /// \brief Sets integer parts of vector to z,w and power of denominator to denom_exp
    void set( const ring_int<TInt>& z, const ring_int<TInt>& w, int denom_exp );

    /// \brief Divides by \f$ \sqrt{2}^a \f$
    void div_eq_sqrt2_exp(int a);
    /// \brief Minimal gde of integer parts of entries
    int min_gde();
    /// \brief Assumes de >= 0, reduces de if possible, ensures de is non negative
    /// \returns de
    int reduce();

    /// \brief Provides access to vector entries
    const ring_int<TInt>& operator[]( int a ) const;
    /// \brief Provides access to vector entries
    ring_int<TInt>& operator[]( int a );

    /// \brief Lexicographical order, assumes A,B
    /// in canonical form ( with minimal possible power of \f$ \sqrt{2}^a \f$ in the denominator )
    bool operator <( const vector2<TInt>& B ) const;
};

#endif // VECTOR2_H
