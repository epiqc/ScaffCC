#!/usr/bin/env python

import sys
import csv
import numpy as np

import warnings
warnings.filterwarnings("ignore", message="numpy.dtype size changed")
from scipy import stats

reg_vals = {}
with open(sys.argv[1]) as csvfile:
    reader = csv.DictReader(csvfile)
    for field in reader.fieldnames:
        if field not in ['polar_r', 'polar_phi', 'rect_real', 'rect_imag']:
            reg_vals[field] = []
    for row in reader:
        for reg in reg_vals.keys():
            reg_val = int(row[reg])
            if reg_val not in reg_vals[reg]:
                reg_vals[reg].append(reg_val)

dimensions = []
for values in reg_vals.values():
    dimensions.append(len(values))
cross_tab = np.ndarray(dimensions)

with open(sys.argv[1]) as csvfile:
    reader = csv.DictReader(csvfile)
    for row in reader:
        index = []
        for reg in reg_vals.keys():
            reg_val = int(row[reg])
            index.append(reg_vals[reg].index(reg_val))
        print index
        cross_tab.itemset(tuple(index), row['polar_r'])

print cross_tab

print stats.chi2_contingency(cross_tab)
