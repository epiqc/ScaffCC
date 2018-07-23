The UCCSD_N.scaffold benchmarks implement the Unitary Coupled Cluster Single-Double ansatz for
N qubits. See [Barkoutsos et al 2018](https://arxiv.org/pdf/1805.04340.pdf) for more details on the underlying physics.
The ansatz is truncated to only include the T_2 term but the implemented matrix exponential
is otherwise exact.

The matrix exponential form of the ansatz is specified in Appendix A of the Barkoutsos paper.
The quantum circuits for U_ij (single excitation operator) and U_ikjl (double excitation operator)
are given in Table A1 of [Whitfield et al 2010](https://arxiv.org/pdf/1001.3855.pdf). Note that the roles of i, j, k, and l
in the Barkoutsos paper are interchanged with p, q, r, and s in the Whitfield paperas well as
in this Scaffold code.

To generate a benchmark for different N, simply modify the definition of N and update the
Theta_p_q and Theta_p_q_r_s arrays to contain N^2 and N^4 random numbers [0, 2pi) respectively.
