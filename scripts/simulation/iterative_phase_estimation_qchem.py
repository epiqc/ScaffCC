#!/usr/bin/env python
import sys
import re
from subprocess import check_output

upper_sz = 6
estimate = 0

for iteration in range(0, upper_sz):

    with open(sys.argv[1]) as infile, open(sys.argv[1]+'.ipe', 'w') as outfile:

        for line in infile:
            if "#define _upper_sz" in line:
                line = re.sub('\d+', str(upper_sz), line)
            if "#define _iteration" in line:
                line = re.sub('\d+', str(iteration), line)
            if "#define _estimate" in line:
                line = re.sub('\d+', str(estimate), line)
            outfile.write(line)

    sim_out = check_output(['./scripts/simulation/simulate.sh', './Algorithms/Ground_State_Estimation/ground_state_estimation.h2.scaffold.ipe'])

    state_lines = False
    for line in sim_out.split('\n'):
        if line.strip() == "--------------[quantum state]--------------":
            print("--------------[quantum state]--------------")
            state_lines = True
        elif line.strip() == "-------------------------------------------":
            print("-------------------------------------------")
            state_lines = False
        elif state_lines:
            print(line)
            strings = re.findall(r"[-+]?\d*\.\d+|\d+", line)
            if (float(strings[0]) > 0.5):
                meas_val = int(strings[4][1])
                print(meas_val)
                estimate += meas_val * pow(2,iteration)
                print(estimate)
