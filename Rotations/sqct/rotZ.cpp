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
  char axis;
  int iter;
  circuit c;

  if ( argc < 3 ) {
    cerr << "Usage: " << argv[0] << " <angle> <X|Y|Y> [levels]\n";
    return 1;
  }
  Scaffold::opts.skOpts.levels = 30;
  angle = atof( argv[1] );
  axis = argv[2][0];
  if ( argc == 3) 
    iter = 1;
  else 
    iter = atoi( argv[3] );

  if (axis != 'X' && axis != 'Y' && axis != 'Z') {
    cerr << "Axis must be one of X, Y, or Z\n";
    return 1;
  }

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
  switch (axis) {
    case 'X': c = sk.rotX( angle, iter ); break;
    case 'Y': c = sk.rotY( angle, iter ); break;
    case 'Z': c = sk.rotZ( angle, iter ); break;
  }

  // Print out
  cout << c.toString() << endl;

  return 0;
}

