//     Copyright (c) 2013 Jeff Heckey jheckey(at)ece(dot)ucsb(dot)edu
//     This file uses SQCT, Copyright (c) 2012 Vadym Kliuchnikov, Dmitri Maslov, Michele Mosca;
//     SQCT is distributed under LGPL v3
//

#include "sk.h"
#include "exactdecomposer.h"
#include "gcommdecomposer.h"
#include "theoremverification.h"
#include "toptimalitytest.h"
#include "hoptimalitytest.h"
#include "netgenerator.h"

class SKDecompose {
private:
    circuit c;                          ///< Circuit
    Rotation r;                         ///< Rotation
    sk skd;                             ///< SK Decomposer
    exactDecomposer ed;                 ///< Exact Decomposer

    int hc;     ///< Hadamard counts
    int tc;     ///< T and inverse of T gates counts
    int pc;     ///< Phase and inverse of Phase gate counts
    int plc;    ///< Pauli matrices counts
    int total;  ///< Total cost using cost function defined by vector gateLibrary::cost
    double dst; ///< Trace distance to approximation
    double tappr; ///< Time required to do an approximation
    double tdecomp; ///< Time required to do a decomposition
    int nr; ///< Number of iterations performed
    int denom_reduction; ///< Difference between \f$ sde(|\cdot|^2) \f$
                         /// before and after conversion to canonical form
    int denom ; ///< \f$ sde(|\cdot|^2) \f$ of resulting unitary
    matrix2x2<mpz_class> exact_uni; /// exact unitary corresponding to the circuit c

public:
    SKDecompose();

    /// \brief Summarize gate statistics for Clifford + T library
    void updateForCliffordT();

    /// \brief Finds circuit and all supplementary information for unitary
    void calculate( const matrix2x2hpr& matrix, int recursion_level );

    void rotation( double angle, int iter, bool frac );

    /// \brief generate a X rotation
    circuit rotX( double angle, int iter, bool frac = false );

    /// \brief generate a Y rotation
    circuit rotY( double angle, int iter, bool frac = false );

    /// \brief generate a Z rotation
    circuit rotZ( double angle, int iter, bool frac = false );
};


