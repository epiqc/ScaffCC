#include <iostream> 
#include <core/circuit.hpp> 
#include <core/functions/add_gates.hpp> 
#include <boost/lexical_cast.hpp> 
using namespace revkit; 
void example (qint a, qint b){
    qint c;
    zero_to_garbage d;
    cnot(a,b);
    NOT(a);
    toffoli(a,b,b);
    a_swap_b(a,b,1);
//    a_eq_a_plus_b(a,b,1);
//    a_eq_a_minus_b(a,b,1);
//    a_eq_a_plus_b_times_c(a,b,c,1);
    assign_value_of_b_to_a(a,b,1);
}
// test2 = main_module
int main (int argc, char** argv){
    cnot(a,b);
    cnot(b,c);
    toffoli(a,b,c);
    example(a, b);
}
