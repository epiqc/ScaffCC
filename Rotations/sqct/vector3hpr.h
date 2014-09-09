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

#ifndef VECTOR3HPR_H
#define VECTOR3HPR_H

#include "rint.h"

/// \brief High precision three dimensioal vector
struct Vector3hpr
{
    /// \brief High precision real numbers class
    typedef ring_int<int>::mpclass mpclass;
    /// \brief Vector entries
    mpclass v[3];

    /// \brief Constructs vector with zero entries
    Vector3hpr();
    /// \brief Set's vector tp (x,y,z)
    Vector3hpr( const mpclass &x, const mpclass& y, const mpclass& z );
    /// \brief Cross product
    Vector3hpr cross( const Vector3hpr& a );
    /// \brief Assignment operator
    Vector3hpr& operator=( const Vector3hpr& val );
    /// \brief Element-wise division
    Vector3hpr operator /( const mpclass& val );

    /// \brief Dot product
    mpclass dot( const Vector3hpr& val );
    /// \brief Euclidean norm squared
    mpclass squaredNorm();
    /// \brief Euclidean norm
    mpclass norm();
};

#endif // VECTOR3HPR_H
