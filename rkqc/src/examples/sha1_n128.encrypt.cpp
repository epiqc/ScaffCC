#include <iostream> 
#include <core/circuit.hpp> 
#include <core/functions/add_gates.hpp> 
#include <boost/lexical_cast.hpp> 
using namespace revkit; 
void nToffoli(qint target, qint in1, qint in2){
    zero_to_garbage ancilla(32);
    int i;
    for (i=0; i<32; i++){
        toffoli(in1[i], in2[i], ancilla[i]);
    }
    assign_value_of_b_to_a(target, ancilla, 32);
}
void AND(qint target, qint in1, qint in2){
    nToffoli(target, in1, in2);
}
void NAND(qint target, qint in1, qint in2){
    int i;
    AND(target, in1, in2);
    for(i=0; i<32; i++){
        NOT(target[i]);
    }
}
void NOR(qint target, qint in1, qint in2){
    int i;
    zero_to_garbage temp1(32);
    zero_to_garbage temp2(32);
    assign_value_of_b_to_a(temp1, in1, 32);
    assign_value_of_b_to_a(temp2, in2, 32);
    for(i=0; i<32; i++){
        NOT(temp1[i]);
        NOT(temp2[i]);
    }
    AND(target, temp1, temp2);
}
void OR(qint target, qint in1, qint in2){
    int i;
    NOR(target, in1, in2);
    for(i=0; i<32; i++){
        NOT(target[i]);
    }
}
void XOR(qint target, qint in1, qint in2){
    zero_to_garbage temp1(32);
    zero_to_garbage temp2(32);
    zero_to_garbage temp3(32);
    zero_to_garbage temp4(32);
    int i;
    assign_value_of_b_to_a(temp1, in1, 32);
    assign_value_of_b_to_a(temp2, in2, 32);

    for(i=0; i<32; i++){
        NOT(temp2[i]);
    }

    AND(temp3, temp1, temp2);
    
    for(i=0; i<32; i++){
        NOT(temp1[i]);
    }

    AND(temp4, temp1, temp2);
    OR(target, temp3, temp4);
}
void reverseBits(qint a, int n){
    zero_to_garbage ancilla(n);
    int i;
    for (i=0;i<n;i++){
        assign_value_of_b_to_a(ancilla[n-i-1],a[i]);
    }
    assign_value_of_b_to_a(a, ancilla, n);
}
void leftRotate(qint a){
    zero_to_garbage temp;
    int i;
    assign_value_of_b_to_a(temp, a[31]);
    for(i=0;i<31;i++){
        assign_value_of_b_to_a(a[i+1], a[i]);
    }
    assign_value_of_b_to_a(a[0], temp);
}
void leftshift(qint a){
    int i;
    for(i=0;i<31;i++){
        assign_value_of_b_to_a(a[i+1],a[i]);
    }
    assign_value_of_b_to_a(a[0], "0");
}
void pad(qint a, qint input, qint padding){
    assign_value_of_b_to_a(padding, "100000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000010000000", 384);
    reverseBits(padding, 384);
    int i;
    for(i=511;i>383;i--){
        assign_value_of_b_to_a(a[i], input[i-384]);
    }
    for(i=383;i>=0;i--){
       assign_value_of_b_to_a(a[i], padding[i]); 
    }
}
void map(qint a, qint in, int n){
    for(int i=0; i<32; i++)
    {
        assign_value_of_b_to_a(a[i], in[i+(32*(15-n))]);
    }
}
void finalHash(qint a, qint h0, qint h1, qint h2, qint h3, qint h4){
    int i;
    for(i=0; i<32; i++)
    {
        assign_value_of_b_to_a(a[i], h4[i]);
    }

    for(i=32; i<64; i++)
    {
        assign_value_of_b_to_a(a[i], h3[i-32]);
    }

    for(i=64; i<96; i++)
    {
        assign_value_of_b_to_a(a[i], h2[i-64]);
    }

    for(i=96; i<128; i++)
    {
        assign_value_of_b_to_a(a[i], h1[i-96]);
    }

    for(i=128; i<160; i++)
    {
        assign_value_of_b_to_a(a[i], h0[i-128]);
    }

}
// encrypt = main_module
int main (int argc, char** argv){
    qbit input(448);
    qbit hash(160);

    zero_to_garbage padding(384);
    zero_to_garbage paddedInput(512);

    zero_to_garbage h0(32);
    zero_to_garbage h1(32);
    zero_to_garbage h2(32);
    zero_to_garbage h3(32);
    zero_to_garbage h4(32);
    zero_to_garbage A(32);
    zero_to_garbage B(32);
    zero_to_garbage C(32);
    zero_to_garbage D(32);
    zero_to_garbage E(32);
    zero_to_garbage K1(32);
    zero_to_garbage K2(32);
    zero_to_garbage K3(32);
    zero_to_garbage K4(32);
    zero_to_garbage f(32);
    zero_to_garbage a(32);
    zero_to_garbage b(32);

    zero_to_garbage temp(32);
    zero_to_garbage temp1(32);
    zero_to_garbage temp2(32);
    zero_to_garbage temp3(32);
    zero_to_garbage temp4(32);
    zero_to_garbage temp5(32);
    zero_to_garbage target(32);
    zero_to_garbage k(32);

    int i;
    int t;

    assign_value_of_b_to_a(h0, "01100111010001010010001100000001", 32);
    reverseBits(h0, 32);
    assign_value_of_b_to_a(h1, "11101111110011011010101110001001", 32);
    reverseBits(h1, 32);
    assign_value_of_b_to_a(h2, "10011000101110101101110011111110", 32);
    reverseBits(h2, 32);
    assign_value_of_b_to_a(h3, "00010000001100100101010001110110", 32);
    reverseBits(h3, 32);
    assign_value_of_b_to_a(h4, "11000011110100101110000111110000", 32);
    reverseBits(h4, 32);
                 
    assign_value_of_b_to_a(A, "01100111010001010010001100000001", 32);
    reverseBits(A, 32);
    assign_value_of_b_to_a(B, "11101111110011011010101110001001", 32);
    reverseBits(B, 32);
    assign_value_of_b_to_a(C, "10011000101110101101110011111110", 32);
    reverseBits(C, 32);
    assign_value_of_b_to_a(D, "00010000001100100101010001110110", 32);
    reverseBits(D, 32);
    assign_value_of_b_to_a(E, "11000011110100101110000111110000", 32);
    reverseBits(E, 32);
                 
    assign_value_of_b_to_a(K1, "01011010100000100111100110011001", 32);
    reverseBits(K1, 32);
    assign_value_of_b_to_a(K2, "01101110110110011110101110100001", 32);
    reverseBits(K2, 32);
    assign_value_of_b_to_a(K3, "10001111000110111011110011011100", 32);
    reverseBits(K3, 32);
    assign_value_of_b_to_a(K4, "11001010011000101100000111010110", 32);
    reverseBits(K4, 32);
                 
    pad(paddedInput, input, padding);
                 
    zero_to_garbage W(80, 32);  

                 
    for(i=0; i<16; i++)
    {            
        map(W[i], paddedInput, i);
    }            
                 
    for(i=16; i<80; i++)
    {            
        XOR(temp2, W[i-3], W[i-8]);
        XOR(temp3, temp2, W[i-14]);
        XOR(target, W[i-3], W[i-16]);
        leftRotate(target);
        assign_value_of_b_to_a(W[i], target, 32);
    }            
                 
    /*----Rounds of Hashing----*/

    for(t=0; t<80; t++)
    {

        if(t<20)
        {
            assign_value_of_b_to_a(temp3, B, 32);
            AND(temp, B, C);
            for(i=0; i<32; i++)
            {
                NOT(temp3[i]);
            }
            AND(temp1, temp3, D);
            XOR(f, temp, temp1);
            assign_value_of_b_to_a(k, K1, 32);
        }

        else if(t<40 && t>=20)
        {
            XOR(temp, B, C);
            XOR(f, temp, D);
            assign_value_of_b_to_a(k, K2, 32);
        }

        else if(t<60 && t>=40)
        {
            AND(temp1, B, C);
            AND(temp2, B, D);
            XOR(temp3, temp1, temp2);
            AND(temp4, C, D);
            XOR(f, temp3, temp4);
            assign_value_of_b_to_a(k, K3, 32);
        }

        else if(t<80 && t>=60)
        {
            XOR(temp, B, C);
            XOR(f, temp, D);
            assign_value_of_b_to_a(k, K4, 32);
        }
        
        assign_value_of_b_to_a(temp1, A, 32);
        assign_value_of_b_to_a(temp2, B, 32);
        for(i=0; i<5; i++)
        {
            leftRotate(temp1);
        }

        assign_value_of_b_to_a(temp, temp1, 32);
        a_eq_a_plus_b(temp, f, 32);
        a_eq_a_plus_b(temp, E, 32);
        a_eq_a_plus_b(temp, k, 32);
        a_eq_a_plus_b(temp, W[t], 32);

        assign_value_of_b_to_a(E, D, 32);
        assign_value_of_b_to_a(D, C, 32);

        for(i=0; i<30; i++)
        {
            leftRotate(temp2);
        }

        assign_value_of_b_to_a(C, temp2, 32);
        assign_value_of_b_to_a(B, A, 32);
        assign_value_of_b_to_a(A, temp, 32);
    }


    a_eq_a_plus_b(h0, A, 32);
    a_eq_a_plus_b(h1, B, 32);
    a_eq_a_plus_b(h2, C, 32);
    a_eq_a_plus_b(h3, D, 32);
    a_eq_a_plus_b(h4, E, 32);
    
    finalHash( hash, h0, h1, h2, h3, h4);
}   
