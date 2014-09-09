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

#ifndef NETGENERATOR_H
#define NETGENERATOR_H

#include "epsilonnet.h"
#include <string>

/// \brief Performes generation of epsilon nets. First you need to
/// generate initial levels using generateInitial and then build
/// all further layers starting from them.
class netGenerator
{
public:
    /// \brief Type of the ring used
    typedef ring_int<int> ri;

    /// \brief Generates initial levels of epsilon net and writes down them to file
    static void generateInitial();
    /// \brief Filename used for epsilon nets with \f$ sde(|\cdot|^2) \f$
    static std::string fileName( int sde );
    /// \brief Creates the next layer of epsilon net based on enet
    static epsilonnet* generate( const epsilonnet& enet );
};

#endif // NETGENERATOR_H
