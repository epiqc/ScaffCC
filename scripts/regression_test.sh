#!/bin/bash

DIR=$(dirname $0)
ROOT=$DIR/..
OPT=$ROOT/build/Release+Asserts/bin/opt
SCAF=$ROOT/build/Release+Asserts/lib/Scaffold.so

echo -e "          Regression Test          "
echo -e "==================================="
for test_case in $ROOT/test_cases/*; do
    if [[ -d $test_case ]]; then
        $ROOT/scaffold.sh -R $test_case/*.scaffold > tmp.txt
        python get_total_gates_line.py *.resources  > total_gates.txt
        diff total_gates.txt $test_case/total_gates.txt
        if cmp total_gates.txt $test_case/total_gates.txt
        then
            echo -e "[$test_case] Generating Resource Estimate \e[32mSucceeded\e[39m"
        else
            echo -e "[$test_case] Generating Resource Estimate \e[31mFailed\e[39m"
        fi
        $ROOT/scaffold.sh -q -R $test_case/*.scaffold
        if cmp *.qasmh $test_case/*.qasmh
        then
            echo -e "[$test_case] Generating QASM \e[32mSucceeded\e[39m"
        else
            echo -e "[$test_case] Generating QASM \e[31mFailed\e[39m"
        fi
        $ROOT/scaffold.sh -f -R $test_case/*.scaffold 
        if cmp *.qasmf $test_case/*.qasmf
        then
            echo -e "[$test_case] Generating Flattened QASM \e[32mSucceeded\e[39m"
        else
            echo -e "[$test_case] Generating Flattened QASM \e[31mFailed\e[39m"
        fi
        $ROOT/scaffold.sh -c $test_case/*.scaffold
        rm -rf tmp.txt
        rm -rf total_gates.txt
    fi
done
echo -e "==================================="
