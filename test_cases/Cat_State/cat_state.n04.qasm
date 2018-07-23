OPENQASM 2.0;
include "qelib1.inc";
qreg bits[4];
h bits[0];
cx bits[1],bits[0];
cx bits[2],bits[1];
cx bits[3],bits[2];

