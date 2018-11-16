#!/usr/bin/env python

import sys
import csv

import warnings
warnings.filterwarnings("ignore", message="numpy.dtype size changed")
from scipy import stats

tallies_model = {}
tallies_dut = {}

qubit_count = int(sys.argv[2])
integer_val = int(sys.argv[3])

for val in range(1<<qubit_count):
    tallies_model[val] = sys.maxsize if val==integer_val else 1
    tallies_dut[val] = 0

with open(sys.argv[1]) as csvfile:
    reader = csv.DictReader(csvfile)
    for row in reader:
        key = int(row['ket'])
        tallies_dut[key] = tallies_dut[key]+1

print tallies_model
print tallies_dut
print stats.chisquare(tallies_dut.values(), f_exp=tallies_model.values())
