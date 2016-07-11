#include <iostream> 
#include <core/circuit.hpp>
#include <core/functions/add_gates.hpp>
#include <boost/lexical_cast.hpp>

using namespace revkit;


void reverseBits32 ( qint a ) {
   qbit ancilla(32);
  assign_value_of_b_to_a (ancilla[31],a[0],1);
  assign_value_of_b_to_a (ancilla[30],a[1],1);
  assign_value_of_b_to_a (ancilla[29],a[2],1);
  assign_value_of_b_to_a (ancilla[28],a[3],1);
  assign_value_of_b_to_a (ancilla[27],a[4],1);
  assign_value_of_b_to_a (ancilla[26],a[5],1);
  assign_value_of_b_to_a (ancilla[25],a[6],1);
  assign_value_of_b_to_a (ancilla[24],a[7],1);
  assign_value_of_b_to_a (ancilla[23],a[8],1);
  assign_value_of_b_to_a (ancilla[22],a[9],1);
  assign_value_of_b_to_a (ancilla[21],a[10],1);
  assign_value_of_b_to_a (ancilla[20],a[11],1);
  assign_value_of_b_to_a (ancilla[19],a[12],1);
  assign_value_of_b_to_a (ancilla[18],a[13],1);
  assign_value_of_b_to_a (ancilla[17],a[14],1);
  assign_value_of_b_to_a (ancilla[16],a[15],1);
  assign_value_of_b_to_a (ancilla[15],a[16],1);
  assign_value_of_b_to_a (ancilla[14],a[17],1);
  assign_value_of_b_to_a (ancilla[13],a[18],1);
  assign_value_of_b_to_a (ancilla[12],a[19],1);
  assign_value_of_b_to_a (ancilla[11],a[20],1);
  assign_value_of_b_to_a (ancilla[10],a[21],1);
  assign_value_of_b_to_a (ancilla[9],a[22],1);
  assign_value_of_b_to_a (ancilla[8],a[23],1);
  assign_value_of_b_to_a (ancilla[7],a[24],1);
  assign_value_of_b_to_a (ancilla[6],a[25],1);
  assign_value_of_b_to_a (ancilla[5],a[26],1);
  assign_value_of_b_to_a (ancilla[4],a[27],1);
  assign_value_of_b_to_a (ancilla[3],a[28],1);
  assign_value_of_b_to_a (ancilla[2],a[29],1);
  assign_value_of_b_to_a (ancilla[1],a[30],1);
  assign_value_of_b_to_a (ancilla[0],a[31],1);
  assign_value_of_b_to_a (a[0],ancilla[0],1);
  assign_value_of_b_to_a (a[1],ancilla[1],1);
  assign_value_of_b_to_a (a[2],ancilla[2],1);
  assign_value_of_b_to_a (a[3],ancilla[3],1);
  assign_value_of_b_to_a (a[4],ancilla[4],1);
  assign_value_of_b_to_a (a[5],ancilla[5],1);
  assign_value_of_b_to_a (a[6],ancilla[6],1);
  assign_value_of_b_to_a (a[7],ancilla[7],1);
  assign_value_of_b_to_a (a[8],ancilla[8],1);
  assign_value_of_b_to_a (a[9],ancilla[9],1);
  assign_value_of_b_to_a (a[10],ancilla[10],1);
  assign_value_of_b_to_a (a[11],ancilla[11],1);
  assign_value_of_b_to_a (a[12],ancilla[12],1);
  assign_value_of_b_to_a (a[13],ancilla[13],1);
  assign_value_of_b_to_a (a[14],ancilla[14],1);
  assign_value_of_b_to_a (a[15],ancilla[15],1);
  assign_value_of_b_to_a (a[16],ancilla[16],1);
  assign_value_of_b_to_a (a[17],ancilla[17],1);
  assign_value_of_b_to_a (a[18],ancilla[18],1);
  assign_value_of_b_to_a (a[19],ancilla[19],1);
  assign_value_of_b_to_a (a[20],ancilla[20],1);
  assign_value_of_b_to_a (a[21],ancilla[21],1);
  assign_value_of_b_to_a (a[22],ancilla[22],1);
  assign_value_of_b_to_a (a[23],ancilla[23],1);
  assign_value_of_b_to_a (a[24],ancilla[24],1);
  assign_value_of_b_to_a (a[25],ancilla[25],1);
  assign_value_of_b_to_a (a[26],ancilla[26],1);
  assign_value_of_b_to_a (a[27],ancilla[27],1);
  assign_value_of_b_to_a (a[28],ancilla[28],1);
  assign_value_of_b_to_a (a[29],ancilla[29],1);
  assign_value_of_b_to_a (a[30],ancilla[30],1);
  assign_value_of_b_to_a (a[31],ancilla[31],1);
}


int main () {
  set_LLVM(true);
  qbit h0(32);
  assign_value_of_int_to_a (h0[0],1,1);
  assign_value_of_int_to_a (h0[1],0,1);
  assign_value_of_int_to_a (h0[2],0,1);
  assign_value_of_int_to_a (h0[3],0,1);
  assign_value_of_int_to_a (h0[4],0,1);
  assign_value_of_int_to_a (h0[5],0,1);
  assign_value_of_int_to_a (h0[6],0,1);
  assign_value_of_int_to_a (h0[7],0,1);
  assign_value_of_int_to_a (h0[8],1,1);
  assign_value_of_int_to_a (h0[9],1,1);
  assign_value_of_int_to_a (h0[10],0,1);
  assign_value_of_int_to_a (h0[11],0,1);
  assign_value_of_int_to_a (h0[12],0,1);
  assign_value_of_int_to_a (h0[13],1,1);
  assign_value_of_int_to_a (h0[14],0,1);
  assign_value_of_int_to_a (h0[15],0,1);
  assign_value_of_int_to_a (h0[16],1,1);
  assign_value_of_int_to_a (h0[17],0,1);
  assign_value_of_int_to_a (h0[18],1,1);
  assign_value_of_int_to_a (h0[19],0,1);
  assign_value_of_int_to_a (h0[20],0,1);
  assign_value_of_int_to_a (h0[21],0,1);
  assign_value_of_int_to_a (h0[22],1,1);
  assign_value_of_int_to_a (h0[23],0,1);
  assign_value_of_int_to_a (h0[24],1,1);
  assign_value_of_int_to_a (h0[25],1,1);
  assign_value_of_int_to_a (h0[26],1,1);
  assign_value_of_int_to_a (h0[27],0,1);
  assign_value_of_int_to_a (h0[28],0,1);
  assign_value_of_int_to_a (h0[29],1,1);
  assign_value_of_int_to_a (h0[30],1,1);
  assign_value_of_int_to_a (h0[31],0,1);
  reverseBits32 (h0);
}

