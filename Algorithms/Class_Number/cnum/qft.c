
// QFT code borrowed from Oana
// 
module qft(qreg reg[n]) //n*(n-1)/2 *(3Rz + 2 CNOT)
{
  int i,j;
  for (i = 0; i < n; i++) {
    H(reg[i]);

    for (j = 1; j < (n-i); j++) {
      ControlledRotationPi(reg[i+j], reg[i], j);
    }
  }

  for (i=0;i<(n/2);i++) {
    Swap(reg[i],reg[n-1-i]);
  }
}

// Swaps 2 qubit states
// Takes as input the 2 qubits
module Swap(qreg q1,qreg q2)
{
  CNOT(q2,q1);
  CNOT(q1,q2);
  CNOT(q2,q1);
}


// Implements a controlled rotation
// inputs are the target qubit, control qubit and the rotation angle
module ControlledRotation(qreg target, qreg control, double theta)
{
  Rz(target, theta / 2.0);
  CNOT(target, control);
  Rz(target, -1.0 * theta / 2.0);
  CNOT(target, control);
}

// Controlled version of gate [1 0; 0 e^(i*PI/2^k)] - the R gate appearing
// in the quantum Fourier transform
// parameters are the target qubit, control qubit and the integer k from 2^k

module ControlledRotationPi(qreg target, qreg control,int j)
{
  ControlledRotation(target, control, PI/pow(2,j));
}


