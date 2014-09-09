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

#ifndef NUMBERSSTAT_H
#define NUMBERSSTAT_H

#include "numbersgen.h"
#include <vector>
#include <limits>
#include <iostream>
#include <memory>

using namespace std;

/// \brief Outputs statistics about numbers in the ring \f$ \mathbb{Z}[\frac{1}{\sqrt{2}},i]\f$
/// that have or don't have pairs.
/// Counts number of ways how numbers can be combined into pairs.
/// \note  Class precompiled for n = 2,4,6,8,10,12
template< int n>
class Stats : public numbersGenerator<n>
{
public:
    /// \brief Basic class type
    typedef numbersGenerator<n> base;
    /// \brief All statistical information is computed during construction stage
    Stats();
};

/// \brief Helper function that creates Stats<n> ( which cause statistical
/// information being printed to cout )
/// \note  Function precompiled for n = 2,4,6,8,10,12
template< int n>
void numbersStatistics();

#endif // NUMBERSSTAT_H
