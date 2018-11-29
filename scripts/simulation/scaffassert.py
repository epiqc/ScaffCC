#!/usr/bin/env python

import sys, re, copy, os

break_indx = 0
breakpoints = [[]]

with open(sys.argv[1]) as source:
    for line in source:

        if re.findall("^assert_", line.strip()):

            if "assert_classical" in line:
                # start the next breakpoint
                breakpoints.append(copy.copy(breakpoints[break_indx]))

                param_string = re.findall("^assert_classical\s*\((.*?)\)\s*;", line.strip())
                params = [param.strip() for param in param_string[0].split(',')]

                # close out this breakpoint
                breakpoints[break_indx].append('for ( int i=0 ; i<{:s} ; i++ )\n'.format(params[1]))
                breakpoints[break_indx].append('    MeasZ ( {:s}[i] );\n'.format(params[0]))
                breakpoints[break_indx].append('}\n')

                # generate script for checking the breakpoint
                breakpoint_name = os.path.splitext(os.path.basename(sys.argv[1]))[0] + '.breakpoint_' + str(break_indx+1)
                with open(breakpoint_name + '.bash', 'w') as outfile:
                    outfile.write('$SCAFFCC_PATH/scripts/simulation/assert_classical.py {:s}.csv {:s} {:s} {:s}'.format(
                    breakpoint_name,
                    params[0],
                    params[1],
                    params[2]))

                break_indx+=1

            elif "assert_superposition" in line:
                # start the next breakpoint
                breakpoints.append(copy.copy(breakpoints[break_indx]))

                param_string = re.findall("^assert_superposition\s*\((.*?)\)\s*;", line.strip())
                params = [param.strip() for param in param_string[0].split(',')]

                # close out this breakpoint
                breakpoints[break_indx].append('for ( int i=0 ; i<{:s} ; i++ )\n'.format(params[1]))
                breakpoints[break_indx].append('    MeasZ ( {:s}[i] );\n'.format(params[0]))
                breakpoints[break_indx].append('}\n')

                # generate script for checking the breakpoint
                breakpoint_name = os.path.splitext(os.path.basename(sys.argv[1]))[0] + '.breakpoint_' + str(break_indx+1)
                with open(breakpoint_name + '.bash', 'w') as outfile:
                    outfile.write('$SCAFFCC_PATH/scripts/simulation/assert_superposition.py {:s}.csv {:s} {:s}'.format(
                    breakpoint_name,
                    params[0],
                    params[1]))

                break_indx+=1

            elif "assert_product" in line:
                # start the next breakpoint
                breakpoints.append(copy.copy(breakpoints[break_indx]))

                param_string = re.findall("^assert_product\s*\((.*?)\)\s*;", line.strip())
                params = [param.strip() for param in param_string[0].split(',')]

                # close out this breakpoint
                breakpoints[break_indx].append('for ( int i=0 ; i<{:s} ; i++ )\n'.format(params[1]))
                breakpoints[break_indx].append('    MeasZ ( {:s}[i] );\n'.format(params[0]))
                breakpoints[break_indx].append('for ( int i=0 ; i<{:s} ; i++ )\n'.format(params[3]))
                breakpoints[break_indx].append('    MeasZ ( {:s}[i] );\n'.format(params[2]))
                breakpoints[break_indx].append('}\n')

                # generate script for checking the breakpoint
                breakpoint_name = os.path.splitext(os.path.basename(sys.argv[1]))[0] + '.breakpoint_' + str(break_indx+1)
                with open(breakpoint_name + '.bash', 'w') as outfile:
                    outfile.write('$SCAFFCC_PATH/scripts/simulation/assert_product.py {:s}.csv {:s} {:s} {:s} {:s}'.format(
                    breakpoint_name,
                    params[0],
                    params[1],
                    params[2],
                    params[3]))

                break_indx+=1

        else:
            breakpoints[break_indx].append(line)

# generate breakpoints
for break_indx in range(len(breakpoints)):
    out_name = os.path.splitext(os.path.basename(sys.argv[1]))[0] + '.breakpoint_' + str(break_indx+1) + '.scaffold'
    with open(out_name, 'w') as outfile:
        for line in breakpoints[break_indx]:
            outfile.write(line)

print (break_indx+1)
