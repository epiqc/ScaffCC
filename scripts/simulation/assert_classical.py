#!/usr/bin/env python

import sys, csv, warnings
import numpy as np
warnings.filterwarnings("ignore", message="numpy.dtype size changed")
from scipy import stats

qubit_count = int(sys.argv[3])
integer_val = int(sys.argv[4])

# generate golden model
tallies_model = {}
tallies_dut = {}
for val in range(1<<qubit_count):
    tallies_model[val] = 65536.0*65536.0 if val==integer_val else 1.0
    tallies_dut[val] = 0.0
print (tallies_model)
f_exp = tallies_model.values() / np.sum(tallies_model.values())
print (f_exp)

# tabulate observed counts
with open(sys.argv[1]) as csvfile:
    reader = csv.DictReader(csvfile)
    for row in reader:
        key = int(row[sys.argv[2]])
        tallies_dut[key] += float(row['probability'])
print (tallies_dut)
f_obs = tallies_dut.values() / np.sum(tallies_dut.values())
print (f_obs)

print (stats.chisquare(f_obs,f_exp,ddof=1))
