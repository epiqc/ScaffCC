#!/usr/bin/env python

import sys, csv, warnings
import numpy as np
warnings.filterwarnings("ignore", message="numpy.dtype size changed")
from scipy import stats

# find the possible values of each register
reg_vals = {}
with open(sys.argv[1]) as csvfile:
    reader = csv.DictReader(csvfile)
    # establish the registers
    for field in reader.fieldnames:
        if field not in ['probability', 'polar_r', 'polar_phi', 'rect_real', 'rect_imag']:
            reg_vals[field] = []
    # add the value if not already seen
    for row in reader:
        for reg in reg_vals.keys():
            reg_val = int(row[reg])
            if reg_val not in reg_vals[reg]:
                reg_vals[reg].append(reg_val)

# print ("reg_vals=")
# print (reg_vals)

# create a cross table with the right dimensions
dimensions = []
for values in reg_vals.values():
    dimensions.append(len(values))
cross_tab = np.zeros(dimensions)

# sum up probabilities
with open(sys.argv[1]) as csvfile:
    reader = csv.DictReader(csvfile)
    for row in reader:
        index = []
        for reg in reg_vals.keys():
            reg_val = int(row[reg])
            index.append(reg_vals[reg].index(reg_val))
        # print (index)
        sum = cross_tab.item( tuple(index) ) + float( row['probability'] )
        # print ("cross_tab.item( tuple(index) ) = ")
        # print (cross_tab.item( tuple(index) ))
        # print ("float( row['probability'] ) = ")
        # print (float( row['probability'] ))
        # print ("sum = ")
        # print (sum)
        cross_tab.itemset ( tuple(index), sum )

# print ("cross_tab=")
# print (cross_tab)

chi2_stat, p_val, dof, ex = stats.chi2_contingency(cross_tab)

print("===Chi2 Stat===")
print(chi2_stat)
print("\n")

print("===Degrees of Freedom===")
print(dof)
print("\n")

print("===P-Value===")
print(p_val)
print("\n")

print("===Contingency Table===")
print(ex)
