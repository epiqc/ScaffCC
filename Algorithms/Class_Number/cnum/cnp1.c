
// Class Number Algorithm -- Phase I (estimate the regulator)
//
//
// As specified in the GFI, inputs here are S and \Delta (the discriminant)
// We assume we have jp, q, t, lgdelta and bound (all classically generated)
//
// (bound = i/N + jp/L, where i iterates classically as described in 4.1.1
// of the GFI)
//
cnp1(delta, jp, q, t, bound)
{
  qreg in[q];
  qreg out[t];
  bit res[t];
  bit period[q];
  qreg dist[lgdelta];

  gfor (i=0; i < q; i++)
  {
    zprepare(in[i], 0);               // init qubits to |0>
    H(in[i]);                         // put into uniform superposition
  }

  gfor (i=0; i < t; i++)
    zprepare(out[i], 0);              // init output qubits to |0>

  fn(bound, delta, j, out[0..t-1], dist);    // oracle call

  // now measure the output of our oracle
  for (i=0; i < t; i++)
    measX(out[i], res[i]);
  
  qft(in);

  // and measure to get period
  for (i=0; i < q; i++)
    measX(int[i], period[i]);
}

