#!/usr/bin/env python

import sys
import csv

import warnings
warnings.filterwarnings("ignore", message="numpy.dtype size changed")
from scipy import stats

tallies = {}
qubit_count = int(sys.argv[2])
for val in range(1<<qubit_count):
    tallies[val] = 0

with open(sys.argv[1]) as csvfile:
    reader = csv.DictReader(csvfile)
    for row in reader:
        key = int(row['ket'])
        tallies[key] = tallies[key]+1

print tallies
print stats.chisquare(tallies.values())

# chisquare([16, 18, 16, 14, 12, 12], f_exp=[16, 16, 16, 16, 16, 8])
