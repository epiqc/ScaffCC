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
    print '\t[flattening_thresh.py] Total Num of Functions = ',numVals

    names = ['000k','001k','010k','125k','300k','1M','2M','25M']
    buckets = [(0,0),(0,1000),(1000,10000),(10000,125000),(125000,300000),(300000,1000000),(1000000,2000000),(2000000,25000000)]

    numBuckets = len(buckets)
    
    histVals = []

    for i in range(numBuckets):
        n = filter(lambda x: (int(x[1])>=buckets[i][0]) and (int(x[1])<buckets[i][1]), m)
        histVals.append(len(n))

    sumFunc = 0
    for i in range(numBuckets):
        print '\t',buckets[i][0],'-',buckets[i][1],' : ',histVals[i]
        sumFunc = sumFunc+histVals[i]

    print '\t>',buckets[-1][1] ,': ', numVals - sumFunc

    for i in range(numBuckets):
        can1k = filter(lambda x: (int(x[1])>=0) and (int(x[1])<buckets[i][1]), m)
        n1k = map(lambda x: x[0], can1k)
        fn = benchName+'.flat'+names[i]+'.txt'
    
        fout = open(fn,'w')
    
        for e in n1k:
            fout.write(e)
            fout.write('\n')

        fout.close()

    

parser = argparse.ArgumentParser(description='Generate flattened module list for this benchmark')

parser.add_argument("input")

args = parser.parse_args()

genFlattenModules(args.input)

