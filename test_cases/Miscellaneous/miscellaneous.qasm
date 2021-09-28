OPENQASM 2.0;
include "qelib1.inc";
qreg qb[4];
creg cb[2];
reset qb[0];
reset qb[1];
x qb[1];
reset qb[2];
h qb[2];
reset qb[3];
x qb[3];
h qb[3];
cx qb[2], qb[1];
h qb[2]
cx qb[1],  qb[2]
tdg qb[2]
cx qb[0],  qb[2]
t qb[2]
cx qb[1],  qb[2]
tdg qb[2]
cx qb[0],  qb[2]
t qb[1]
t qb[2]
h qb[2]
cx qb[0],  qb[1]
t qb[0]
tdg qb[1]
cx qb[2], qb[1];
h qb[2];
measure qb[2] -> cb[0];
measure qb[3] -> cb[1];
