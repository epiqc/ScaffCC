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

#ifndef SK_H
#define SK_H

#include "matrix2x2.h"
#include "unitaryapproximator.h"

/// \brief Implements the Solovay Kitaev algorithm
/// to approximated high precision floating point unitaries
/// by unitaries over the ring \f$ \mathbb{Z}[\frac{1}{\sqrt{2}},i]\f$.
/// \see See section 3 of http://arxiv.org/pdf/quant-ph/0505030v2.pdf

class sk
{
public:
    /// \brief High precision matrix type
    typedef matrix2x2hpr Ma;
    /// \brief Matrix over the ring \f$ \mathbb{Z}[\frac{1}{\sqrt{2}},i]\f$ type
    typedef matrix2x2<mpz_class> Me;
    /// \brief Loads epsilon nets up to max_layer non inclusively
    /// \see indexedUnitaryApproximator constructor
    sk( int max_layer = 31 );
    /// \brief Runs n iteration of the Solovay Kitaev algorithm and writes
    /// result into out
    void decompose( const Ma& U, Me& out, int n );
private:
    /// \brief Class used for approximation
    indexedUnitaryApproximator uapp;
};

#endif // SK_H
