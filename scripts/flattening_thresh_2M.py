#!/usr/bin/env python

import math
import argparse

def genFlattenModules(benchName):

    fn = benchName+'.out'
    f = open(fn,'r')
    r = f.read().split('\n')
    f.close()

    l = filter(lambda x: (len(x)>1), r)
    #print l

    m = map(lambda x: x.replace(':',''), l)
    m = map(lambda x: x.split(), m)
    #print m
    
    vals = map(lambda x: int(x[1]), m)
    #print vals 

    numVals = len(vals)
    print '[flattening_thresh_2M.py] Total Num of Functions = ',numVals

    # Change this value to change flattening thresholds. Currently 2M=2000000 ops.
    names = ['2M']
    buckets = [(0,2000000)]

    numBuckets = len(buckets)
    
    histVals = []

    for i in range(numBuckets):
        n = filter(lambda x: (int(x[1])>=buckets[i][0]) and (int(x[1])<buckets[i][1]), m)
        histVals.append(len(n))

    sumFunc = 0
    for i in range(numBuckets):
        print buckets[i][0],'-',buckets[i][1],' : ',histVals[i]
        sumFunc = sumFunc+histVals[i]

    print '>',buckets[-1][1] ,': ', numVals - sumFunc

    for i in range(numBuckets):
        can1k = filter(lambda x: (int(x[1])>=0) and (int(x[1])<buckets[i][1]), m)
        n1k = map(lambda x: x[0], can1k)
        fn = benchName+'_flat'+names[i]+'.txt'
    
        fout = open(fn,'w')
    
        for e in n1k:
            fout.write(e)
            fout.write('\n')

        fout.close()

    

parser = argparse.ArgumentParser(description='Generate flattened module list for this benchmark')

parser.add_argument("input")

args = parser.parse_args()

genFlattenModules(args.input)

