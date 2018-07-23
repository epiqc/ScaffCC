
// Class Number Algorithm -- Phase II (structure of the class group)
//
// Sect 6 of the GFI indicates the inputs to Phase II are delta, the
// approximation of the regulator R (from Phase I), and q, k, N, and M.
//
// We further assume that the classical step of producing k generators
// g[0], ..., g[k-1], was done (see GFI section 4.1.3)
//
// Finally, we assume the classical precomputation of logdelta = 
// ceiling(log_2(delta)), lognr = ceiling(log_2(N*R), logm = ceiling(log_2(M))
// sdelta = sqrt(Delta), and the jitter jp, and bound (see cnp1.c)
//
// res is the output array for the k q-qubit results
//
// See 4.1.4 of the GFI for details
//
cnp2(delta, q, k, g, R, N, M, logdelta, lognr, logm, sdelta, jp, bound, res)
{
  qreg in[q*k];                      // input registers
  qreg I[logdelta];
  qreg i[lognr];
  qreg j[lognr];
  qreg fout[logdelta+lognr];
  qreg x[logm + lognr];
  qreg y[lognr];
  qreg size[logdelta+lognr];
  qreg dist[lgdelta];
  qreg found[1];
  qreg c1[logm + lognr];
  qreg c2[logm + lognr];
  qreg d1[lognr];
  qreg d2[lognr];
  qreg a[logm + lognr];
  qreg b[lognr];

  gfor (i=0; i < q*k; i++)
  {
    zprepare(in[i], 0);               // init qubits to |0>
    H(in[i]);                         // put into uniform superposition
  }
  zprepare(I);                        // prepare all 6 output registers
  zprepare(i);
  zprepare(fout);
  zprepare(x);
  zprepare(y);
  zprepare(size);

  
  ghat(I, g, in, k, delta, sdelta);    // oracle that produces I

  H(i);                                // put i into uniform superposition

  fjn(bound, delta, sdelta, I, in[q..q+t-1], dist); 

  do {
    do {
      H(x);
      H(y);
      h(x, y, bound, delta, sdelta, i, size, dist);

      // measure the output of our oracle
      for (i=0; i < logdelta+lognr; i++)
	measX(size[i], result[i]);
      
      qft(x);
      qft(y);
     
      for (i=0; i < logm+lognr; i++)
	measX(x[i], c1[i]);
      
      for (i=0; i < lognr; i++)
	measX(y[i], d1[i]);

      H(x);
      H(y);
      h(x, y, bound, delta, sdelta, i, size, dist);

      // measure the output of our oracle
      for (i=0; i < logdelta+lognr; i++)
	measX(size[i], result[i]);
      
      qft(x);
      qft(y);
     
      for (i=0; i < logm+lognr; i++)
	measX(x[i], c2[i]);
      
      for (i=0; i < lognr; i++)
	measX(y[i], d2[i]);
    }
    while (gcd(d1, d2, a, b) != 1);
    
  } while (xd(c1, c2, a, b, N, M, R) != dist);

  fjn(bound, delta, sdelta, I, in[q..q+t-1], dist); 
  h(x, y, bound, delta, sdelta, i, size, dist);
  ghat(I, g, in, k, delta, sdelta); 

  for (i=0; i < logdelta+lognr; i++)
	measX(fout[i], result[i]);

  qft(result);
  
  for (j=0; j < k; j++)
    for (i=0; i < q; i++)
      measX(in[j][i], res[j][i]);
}
