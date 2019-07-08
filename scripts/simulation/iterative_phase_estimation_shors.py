#!/usr/bin/env python
import sys
import re
from subprocess import check_output

def egcd(a, b):
    if a == 0:
        return (b, 0, 1)
    else:
        g, y, x = egcd(b % a, a)
        return (g, x - (b // a) * y, y)

def modinv(a, m):
    g, x, y = egcd(a, m)
    if g != 1:
        raise Exception('modular inverse does not exist')
    else:
        return x % m

a_val = [1,1,4,7]
a_inv = [1,1,4,13]
upper_sz = 4
estimate = 0

for iteration in range(0, upper_sz):

    with open(sys.argv[1]) as infile, open(sys.argv[1]+'.ipe', 'w') as outfile:

        for line in infile:
            if "#define _a_val" in line:
                line = re.sub('\d+', str(a_val[iteration]), line)
            if "#define _a_inv" in line:
                line = re.sub('\d+', str(a_inv[iteration]), line)
            if "#define _upper_sz" in line:
                line = re.sub('\d+', str(1), line)
            if "#define _iteration" in line:
                line = re.sub('\d+', str(iteration), line)
            if "#define _estimate" in line:
                line = re.sub('\d+', str(estimate), line)
            outfile.write(line)

    sim_out = check_output(['./scripts/simulation/simulate.sh', './Algorithms/Shors/Shors_15/shors.n3.scaffold.ipe'])

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
                meas_val = int(strings[4][-1])
                print(meas_val)
                estimate += meas_val * pow(2,iteration)
                print(estimate)

    # a_val = (a_val*a_val) % 15
    # a_inv = modinv(a_val, 15)
