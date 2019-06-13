#!/usr/bin/env python
import sys
import re
from math import sqrt

trace_distance = 0.0

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

        real_diff = real0-real1
        imag_diff = imag0-imag1

        trace_distance += 0.5*sqrt(
            real_diff*real_diff +
            imag_diff*imag_diff
        )

        print (trace_distance)
