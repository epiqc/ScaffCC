#!/bin/bash

DIR=$(dirname $0)
ROOT=$DIR/..
OPT=$ROOT/build/bin/opt
SCAF=$ROOT/build/lib/LLVMScaffold.so

for f in $*; do
    b=$(basename $f)
    k=$(perl -e '($n,$size,$th,$simd,$k,$d,$x) = split /\./, $ARGV[0]; print $k' ${b})
    d=$(perl -e '($n,$size,$th,$simd,$k,$d,$x) = split /\./, $ARGV[0]; print $d' ${b})
    echo "[full_sched_regress.sh] $f: Running sched.pl ... k: $k d: $d"
    #if [ ! -e ${b}.ss ]; then
    #    /usr/bin/time -f "\t%E real,\t%U user,\t%S sys:\t%C" ${DIR}/sched.pl -n ss -k $k -d $d $f -s > ${b}.ss
    #fi
    #if [ "$k" -eq "1" ]; then
    #    if [ ! -e ${b}.l0.lpfs ]; then
    #        /usr/bin/time -f "\t%E real,\t%U user,\t%S sys:\t%C" ${DIR}/sched.pl -n lpfs -k $k -l 0 -d $d $f -s > ${b}.l0.lpfs
    #    fi
    #fi
    #for (( l=1; l < k; l++ )); do
    for (( l=1; l < 2; l++ )); do
    #    if [ ! -e ${b}.l${l}.lpfs ]; then
    #        /usr/bin/time -f "\t%E real,\t%U user,\t%S sys:\t%C" ${DIR}/sched.pl -n lpfs -k $k -l $l -d $d $f -s > ${b}.l${l}.lpfs
    #    fi
    #    if [ ! -e ${b}.r.l${l}.lpfs ]; then        
    #      ${DIR}/sched.pl -n lpfs -k $k -l $l -d $d $f -s -refill > ${b}.r.l${l}.lpfs
    #    fi
    #    if [ ! -e ${b}.s.l${l}.lpfs ]; then
    #        /usr/bin/time -f "\t%E real,\t%U user,\t%S sys:\t%C" ${DIR}/sched.pl -n lpfs -k $k -l $l -d $d $f -s -opp > ${b}.s.l${l}.lpfs
    #    fi
        if [ ! -e ${b}.rs.l${l}.lpfs ]; then
            /usr/bin/time -f "\t%E real,\t%U user,\t%S sys:\t%C" ${DIR}/sched.pl -n lpfs -k $k -l $l -d $d $f -a -opp -refill > ${b}.rs.l${l}.lpfs
        fi
    done
    #if [ ! -e ${b}.o1.d1.s1.rcp ]; then
    #    /usr/bin/time -f "\t%E real,\t%U user,\t%S sys:\t%C" ${DIR}/sched.pl -n rcp -k $k -d $d --op 1 --dist 1 --slack 1 -s $f > ${b}.o1.d1.s1.rcp
    #fi
    #if [ ! -e ${b}.o10.d1.s1.rcp ]; then
    #    /usr/bin/time -f "\t%E real,\t%U user,\t%S sys:\t%C" ${DIR}/sched.pl -n rcp -k $k -d $d --op 10 --dist 1 --slack 1 -s $f > ${b}.o10.d1.s1.rcp
    #fi
    #if [ ! -e ${b}.o1.d10.s1.rcp ]; then
    #    /usr/bin/time -f "\t%E real,\t%U user,\t%S sys:\t%C" ${DIR}/sched.pl -n rcp -k $k -d $d --op 1 --dist 10 --slack 1 -s $f > ${b}.o1.d10.s1.rcp
    #fi
    #if [ ! -e ${b}.o1.d1.s10.rcp ]; then
    #    /usr/bin/time -f "\t%E real,\t%U user,\t%S sys:\t%C" ${DIR}/sched.pl -n rcp -k $k -d $d --op 1 --dist 1 --slack 10 -s $f > ${b}.o1.d1.s10.rcp
    #fi
done
