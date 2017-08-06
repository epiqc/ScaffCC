#!/usr/bin/python
import sys

the_file = open(sys.argv[1],"r")
for line in the_file.readlines():
    if line.startswith( 'total_gates' ):
        print (line)
the_file.close()
