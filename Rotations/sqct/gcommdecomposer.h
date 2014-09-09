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

#ifndef GCOMMDECOMPOSER_H
#define GCOMMDECOMPOSER_H

#include "rint.h"
#include "matrix2x2.h"
#include "vector3hpr.h"

/// \brief Represents rotation by angle \f$ \frac{num\cdot\pi}{den} \f$
/// around axis (nx,ny,nz)
struct Rotation
{
    double num; ///< Multiple of \f$\pi\f$ in the numerator of rotation angle
    double den; ///< Denominator of rotation angle
    double nx;  ///< x coordinate of rotation axis
    double ny;  ///< y coordinate of rotation axis
    double nz;  ///< z coordinate of rotation axis

    /// \brief High precision unitary matrix corresponding to rotation:
    /// \f$ I\cos\left(\frac{num\cdot\pi}{den}\right)-i\sin\left(\frac{num\cdot\pi}{den}\right)\left(n_{x}X+n_{y}Y+n_{z}Z\right) \f$
    matrix2x2hpr matrix();

    /// \brief Symbolic form of the matrix representing rotation
    std::string symbolic() const;
    /// \brief Rotation representation for CSV file
    std::string CSV() const;
    /// \brief Mathematica friendly representation of rotation
    std::string Mathematica() const;
    /// \brief File name prefix for special rotations
    std::string name() const;
    /// \brief Returns label X,Y,Z if rotation axis either x,y or z and angle is a rational multiplier of Pi,
    /// returns 'N' otherwise.
    char isSpecial() const;
};

/// \brief Finds balanced group commutator decomposition of a unitary.
/// Provides helper functions to compute high precision single qubit rotations
class GC
{
public:
    /// \brief Type for high precision real numbers
    typedef ring_int<int>::mpclass mpclass;
    /// \brief Type for high precision matrices
    typedef matrix2x2hpr M;
    /// \brief Type for high precision matrices
    typedef Vector3hpr V;

    /// \brief Performes balanced group commutator decomposition of U
    /// \see Section 4.1 of http://arxiv.org/abs/quant-ph/0505030
    static void decompose( const M& U, M& Vr, M& W);

};

#endif // GCOMMDECOMPOSER_H
