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

#ifndef EXACTDECOMPOSER_H
#define EXACTDECOMPOSER_H

#include "matrix2x2.h"
#include "gatelibrary.h"
#include "seqlookupcliff.h"

/// \brief Uses Algorithm 1 from http://arxiv.org/abs/1206.5236 to get circuit for exact unitary
class exactDecomposer
{
public:
    /// \brief Initializes generators
    exactDecomposer();
    /// \brief Decomposes matr into circuit c
    void decompose( const matrix2x2<mpz_class> &matr, circuit& c );
private:
    /// \brief Matrix over the ring type used during decomposition
    typedef matrix2x2<mpz_class> M;
    /// \brief Ring type used during decomposition
    typedef ring_int<mpz_class> ri;
    /// \brief Number of generators used to perform decomposition
    static const int gen_count = 4;
    /// \brief Generators used for decomposition
    M generators[gen_count];
    /// \brief Inverses of generators used for decomposition
    M generators_ctr[gen_count];
    /// \brief Object of class to lookup T-optimal sequences for unitaries with \f$ sde(|\cdot|^2) < 4 \f$
    seqLookupCliff slC;
};

#endif // EXACTDECOMPOSER_H
