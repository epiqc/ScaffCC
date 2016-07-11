#include <iostream> 
#include <core/circuit.hpp>
#include <core/functions/add_gates.hpp>
#include <boost/lexical_cast.hpp>

using namespace revkit;


int main () {
  set_LLVM(true);
  qbit a(1);
  qbit b(1);
  qbit c(1);
  NOT (a[0]);
  cnot (a[0],b[0]);
  toffoli (a[0],b[0],c[0]);
}

