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

#ifndef RESRING_H
#define RESRING_H

#include <gmpxx.h>

/// \brief Ring of residues modulo TMod
/// \note The only required version of the class is resring<8> is precompiled in
/// resring.cpp
template< int TMod>
class resring
{
public:
    /// \brief Underlying integer type
    typedef int int_type;
    /// \brief Initalizes to 0
    resring();
    /// \brief Initializes to val mod TMod
    resring( int_type val );
    /// \brief Intitializes to value represented by three lower bits of val
    explicit resring( const mpz_class& val );

    /// \brief Usual order on integers
    bool operator <( const resring& y ) const;
    /// \brief Usual order on integers
    bool operator >( const resring& y ) const;
    /// \brief Equality testing
    bool operator ==( const resring& y ) const;
    /// \brief Non-equality testing
    bool operator !=( const resring& y ) const;

    /// \brief Addition modulo TMod
    resring& operator +=(const resring& y);
    /// \brief Substruction modulo TMod
    resring& operator -=(const resring& y);
    /// \brief Addition modulo TMod
    resring operator +(const resring& y) const;
    /// \brief Substruction modulo TMod
    resring operator -(const resring& y) const;
    /// \brief Additive inverse modulo TMod
    resring operator -() const;
    /// \brief Multiplication modulo TMod
    resring operator *(const resring& y) const;
    /// \brief Integer division
    resring operator /( const resring& e) const;
    /// \brief Integer division
    resring& operator /=( const resring& e);
    /// \brief Multiplication by \f$ 2^e \f$
    resring& operator <<=( int e);
    /// \brief Division by \f$ 2^e \f$
    resring operator >> (int e ) const;
    /// \brief Division by \f$ 2^e \f$
    resring& operator >>=( int e);
    /// \brief Residual modulo e
    int_type operator %( int e);
    /// \brief Conversion to basic integer type
    explicit operator int_type () const;

    /// \brief Initializes to val mod TMod
    void set( int_type val );
    /// \brief Computes val modulo TMod
    static int_type mod( int_type val );
protected:
    /// \brief Stored value
    int_type v;
};

/// \brief Residues modulo 8 used in Algorithm 2 in http://arxiv.org/abs/1206.5236
typedef resring<8> rring8;

/// \brief Absolute value ( added for compatibility with ring_int )
template< int TMod>
int abs( const resring<TMod>& val )
{
    return abs( (typename resring<TMod>::int_type) val );
}

#endif // RESRING_H
