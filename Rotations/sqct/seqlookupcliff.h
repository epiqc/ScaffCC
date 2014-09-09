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

#ifndef SEQLOOKUPCLIFF_H
#define SEQLOOKUPCLIFF_H

#include "optsequencegenerator.h"
#include "gatelibrary.h"

/// \brief Generates and stores T-optimal sequences using Clifford + T library
/// using columnsCounter<4>. Exhaustively finds optimal circuits for all unitaries with
/// \f$ sde(|\cdot|^2) \f$ of entries in the range \f$[0,4]\f$
class seqLookupCliff
{
public:
    /// \brief Constructs the class, if full_init is true performes search of optimal sequences
    seqLookupCliff( bool full_init = true );
    /// \brief Finds a circuit for unitary m and writes it into res
    void find( const matrix2x2<int>& m, circuit& res );
    /// \brief Performs search of optimal sequences
    void init();
private:
    /// \brief Optimal sequence generator used to get short circuits for Clifford group unitaries
    optSequenceGenerator ogc;
    /// \brief Optimal sequence generator used to get T-optimal circuits
    optSequenceGeneratorSdeLim og;
    /// \brief Circuits for Clifford group unitaries
    std::vector< circuit > m_clifford;
};

#endif // SEQLOOKUPCLIFF_H
