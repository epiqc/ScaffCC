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

#ifndef OUTPUT_H
#define OUTPUT_H

/// \file Contains ostream output operators for basic types used in the project

#include <iostream>
#include <complex>
#include "matrix2x2.h"
#include "vector2.h"
#include "rint.h"
#include "resring.h"
#include "exactdecomposer.h"

/// \brief Outputs x to stream out in Mathematica friendly form
template < int Tmod >
std::ostream& operator<<(std::ostream& out, const resring<Tmod>& x );

/// \brief Outputs x to stream out in Mathematica friendly form
/// \note Precompiled for types used by ring_int
template < class TInt >
std::ostream& operator<<(std::ostream& out, const ring_int<TInt>& x );

/// \brief Outputs x to stream out in Mathematica friendly form
/// \note Precompiled for types used by vector2
template < class TInt >
std::ostream& operator<<(std::ostream& out, const vector2<TInt>& v );

/// \brief Outputs x to stream out in Mathematica friendly form
/// \note Precompiled for types used by matrix2x2
template < class TInt >
std::ostream& operator<<(std::ostream& out, const matrix2x2<TInt>& A );

/// \brief Outputs x to stream out in Mathematica friendly form
/// \note Precompiled for types used by matrix2x2
std::ostream& operator<<(std::ostream& out, const matrix2x2hpr& A );

/// \brief Outputs x to stream out in Mathematica friendly form
std::ostream& operator<<(std::ostream& out, const std::pair< std::complex<double>, std::complex<double> > & A );

/// \brief Reads matrix2x2 from stream, entries first and denominator exponent last
/// \note Precompiled for types used by matrix2x2
template < class TInt >
std::istream& operator>>(std::istream& in, matrix2x2<TInt>& A );

/// \brief Reads ring_int from stream
/// \note Precompiled for types used by ring_int
template < class TInt >
std::istream& operator>>(std::istream& in, ring_int<TInt>& A );

#endif // OUTPUT_H
