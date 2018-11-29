#!/usr/bin/env python

import sys, csv, warnings
warnings.filterwarnings("ignore", message="numpy.dtype size changed")
from scipy import stats

qubit_count = int(sys.argv[3])
tallies_dut = {}
for val in range(1<<qubit_count):
    tallies_dut[val] = 0.0

# tabulate observed counts
with open(sys.argv[1]) as csvfile:
    reader = csv.DictReader(csvfile)
    for row in reader:
        key = int(row[sys.argv[2]])
        tallies_dut[key] += float(row['probability'])
print tallies_dut

print stats.chisquare(tallies_dut.values())
