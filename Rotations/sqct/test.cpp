//     Copyright (c) 2013 Jeff Heckey jheckey(at)ece(dot)ucsb(dot)edu
//     This file uses SQCT, Copyright (c) 2012 Vadym Kliuchnikov, Dmitri Maslov, Michele Mosca;
//     SQCT is distributed under LGPL v3
//

#include "skdecomposer.h"
#include "eapp.h"

#include <stdlib.h>
#include <iostream>

// Scaffold namespace holds a global configuration struct (and sub-structs)
// for configuring Scaffold runtime options. Only the SK levels are included 
// here as an example
namespace Scaffold {

struct SKOpts {
    int levels;
};

struct ScaffoldOpts {
    SKOpts skOpts;
};

ScaffoldOpts opts;

}; // end Scaffold namespace

struct SKOpts {
    int iterations; ///< number of iterations to perform
    int max_sde;    ///< maximal value of the smallest denominator exponent
    double angle;   ///< angle to decompose
};

using namespace std;

int main ( int argc, char** argv ) {
    double angle;
    int iter;
    static const double TwoPi = 2. * hprHelpers::toMachine( hprHelpers::pi() );
    circuit c;

    if ( argc != 2 ) {
        cerr << "Wrong number of arguments (" << argc << "), should be a double for the angle" << endl;
        return 1;
    }
    Scaffold::opts.skOpts.levels = 30;
    angle = atof( argv[1] );
	iter = 1;

	// This stuff goes in the RunOn*Init function
    enetOptions eopts;
    eopts.epsilon_net_layers.clear();
    eopts.epsilon_net_layers.push_back( Scaffold::opts.skOpts.levels );
    enetApplication eapp( eopts );
    eapp.process();
    SKDecompose sk;

	// Perform decomposition - circuit will need to be walked to replace the rotation with the decomposed gates
	// angle = radians to rotate, or fraction of 2*pi to rotate
	// iter = is the number of iterations to perform
	// frac = true if this is a fraction of 2*pi. False by default.
    c = sk.rotZ( angle, iter );

	// Print out
    c.toStream(cout);

    return 0;
}

