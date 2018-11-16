#!/usr/bin/env python

import sys, re, copy, os

check_indx = 0
checkpoints = [[]]

with open(sys.argv[1]) as source:
    for line in source:

        if "assert_integer" in line:
            # start the next checkpoint
            checkpoints.append(copy.copy(checkpoints[check_indx]))

            param_string = re.findall("assert_integer\s*\((.*?)\)\s*;", line.strip())
            params = [param.strip() for param in param_string[0].split(',')]

            # close out this checkpoint
            checkpoints[check_indx].append('for ( int i=0 ; i<{:s} ; i++ )\n'.format(params[1]))
            checkpoints[check_indx].append('    MeasZ ( {:s}[i] );\n'.format(params[0]))
            checkpoints[check_indx].append('}\n')

            check_indx+=1

        if "assert_uniform" in line:
            # start the next checkpoint
            checkpoints.append(copy.copy(checkpoints[check_indx]))

            param_string = re.findall("assert_uniform\s*\((.*?)\)\s*;", line.strip())
            params = [param.strip() for param in param_string[0].split(',')]

            # close out this checkpoint
            checkpoints[check_indx].append('for ( int i=0 ; i<{:s} ; i++ )\n'.format(params[1]))
            checkpoints[check_indx].append('    MeasZ ( {:s}[i] );\n'.format(params[0]))
            checkpoints[check_indx].append('}\n')

            check_indx+=1

        else:
            checkpoints[check_indx].append(line)

# generate checkpoints
for check_indx in range(len(checkpoints)):
    out_name = os.path.basename(sys.argv[1]) + str(check_indx) + '.scaffold'
    with open(out_name, 'w') as outfile:
        for line in checkpoints[check_indx]:
            outfile.write(line)
