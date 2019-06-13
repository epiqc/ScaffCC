#!/usr/bin/env python

import sys
import re
from math import sqrt
from math import atan2
from math import cos
from math import sin

fidelity_real = 0.0
fidelity_imag = 0.0

with open(sys.argv[1]) as infile0, open(sys.argv[2]) as infile1:

    # Parse QX Simulator output format
    for line0,line1 in zip(infile0,infile1):

        strings0 = re.findall(r"[-+]?\d*\.\d+|\d+", line0)
        strings1 = re.findall(r"[-+]?\d*\.\d+|\d+", line1)
        assert strings0[4]==strings1[4]

        # grab real and imaginary parts of basis state amplitude
        real0 = float(strings0[2])
        imag0 = float(strings0[3])
        real1 = float(strings1[2])
        imag1 = float(strings1[3])

        mag0 = sqrt( real0*real0 + imag0*imag0 )
        phase0 = atan2(imag0,real0)

        mag1 = sqrt( real1*real1 + imag1*imag1 )
        phase1 = atan2(imag1,real1)

        sqrt_mag = sqrt(mag0*mag1)
        sqrt_phase = 0.5*(phase0+phase1)

        fidelity_real += sqrt_mag*cos(sqrt_phase)
        fidelity_imag += sqrt_mag*sin(sqrt_phase)

        out_string = '({:f},{:f})'.format(
            fidelity_real,
            fidelity_imag
        )
        print (out_string)
