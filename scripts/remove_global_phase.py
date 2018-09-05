import sys
import re
from math import sqrt
from math import atan2
from math import cos
from math import sin

with open(sys.argv[1]) as infile, open(sys.argv[2], 'w') as outfile:

    state_lines = False
    first_basis = True
    phase_global = 0.0

    # Parse QX Simulator output format
    for line in infile:
        if line.strip() == "--------------[quantum state]--------------":
            print("--------------[quantum state]--------------")
            state_lines = True
        elif line.strip() == "-------------------------------------------":
            print("-------------------------------------------")
            state_lines = False
        elif state_lines:
            strings = re.findall(r"[-+]?\d*\.\d+|\d+", line)

            # grab real and imaginary parts of basis state amplitude
            real = float(strings[0])
            imag = float(strings[1])
            magnitude = sqrt(real*real + imag*imag)
            phase_local = atan2(imag,real)

            # if this is the first basis state then this is the global phase to factor out
            if first_basis:
                phase_global = phase_local
                first_basis = False

            real_no_global = magnitude*cos(phase_local-phase_global)
            imag_no_global = magnitude*sin(phase_local-phase_global)
            out_string = 'mag {:f} ang {:f} ({:f},{:f}) |{:s}> +'.format(
                magnitude,
                phase_local-phase_global,
                real_no_global,
                imag_no_global,
                strings[2]
            )

            print(out_string)
            outfile.write(out_string)
