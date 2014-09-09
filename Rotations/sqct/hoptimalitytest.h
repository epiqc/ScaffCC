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

#ifndef HOPTIMALITYTEST_H
#define HOPTIMALITYTEST_H

#include "optsequencegenerator.h"
#include "gatelibrary.h"

/// \brief Performs brute force check required for proof of
/// Appendix 2, http://arxiv.org/abs/1206.5236
/// Shows connection between optimal number of H gates and sde
/// when using (X,Y,Z,T,P) + H library
class hoptimalitytest
{
public:
    /// \brief Initialized by call of init()
    hoptimalitytest();
    /// \brief Prints minimal and maximal number of H gates in optimal circuit per sde
    void init();
private:
    /// \brief Optimal sequence generator used to get short circuits for Clifford group unitaries
    optSequenceGenerator ogc;
    /// \brief Optimal sequence generator used to get T-optimal circuits
    optSequenceGeneratorSdeLim og;
    /// \brief Circuits for Clifford group unitaries
    std::vector< circuit > m_clifford;
};

#endif // HOPTIMALITYTEST_H
