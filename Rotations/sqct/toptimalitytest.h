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

#ifndef TOPTIMALITYTEST_H
#define TOPTIMALITYTEST_H

#include "optsequencegenerator.h"
#include "gatelibrary.h"

/// \brief Performes check of the conjecture of the T optimality of exact
/// decomposition of the algorithm
class toptimalitytest
{
public:
    /// \brief Initialized by call of init()
    toptimalitytest();
    /// \brief Start exhaustive search of T-optimal circuits
    /// then applies exact synthesis algorithm to each of them
    /// and checks if the result of algorithm contains optimal
    /// number of T gates. Outputs to console number of non-optimal
    /// decompositions
    void init();
private:
    /// \brief Optimal sequence generator used to get short circuits for Clifford group unitaries
    optSequenceGenerator ogc;
    /// \brief Optimal sequence generator used to get T-optimal circuits
    optSequenceGeneratorSdeLim og;
    /// \brief Circuits for Clifford group unitaries
    std::vector< circuit > m_clifford;
};

#endif // TOPTIMALITYTEST_H
