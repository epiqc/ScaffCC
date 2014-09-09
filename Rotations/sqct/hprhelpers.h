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

#ifndef HPRHELPERS_H
#define HPRHELPERS_H

#include "real.hpp"
#include <complex>

/// \brief Helper functions and data for high precision arithmetic
class hprHelpers
{
public:
    /// \brief High precision real type used in the project
    typedef mpfr::real<4096> hpr_real;
    /// \brief High precision complex type used in the project
    typedef std::complex<hpr_real> hpr_complex;
    /// \brief Transforms high precision complex number into machine complex
    static void convert( const hpr_complex& from, std::complex<double>& to );
    /// \brief Transforms high precision real number into double
    static void convert( const hpr_real& from, double& to );
    /// \brief Transforms high precision real number into double
    static double toMachine( const hpr_real& from );
    /// \brief Transforms high precision complex number into machine complex
    static std::complex<double> toMachine( const hpr_complex& from );

    /// \brief High precision \f$ \pi \f$
    static const hpr_real& pi();
    /// \brief High precision one
    static const hpr_real& one();
    /// \brief High precision one
    static const hpr_real& two();
    /// \brief High precision 1/2
    static const hpr_real& half();
    /// \brief High precision -1/2
    static const hpr_real& mhalf();
    /// \brief High precision \f$ \frac{\sqrt{2}}{2} \f$
    static const hpr_real& sqrt2ov2();
};

#endif // HPRHELPERS_H
