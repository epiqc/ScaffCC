#!/bin/bash

DIR=$(dirname $0)
ROOT=$DIR/..
OPT=$ROOT/build/bin/opt
SCAF=$ROOT/build/lib/LLVMScaffold.dylib

echo -e "          Regression Test          "
echo -e "==================================="
successful_tests=0
total_tests=0
for test_case in $ROOT/test_cases/*; do
    if [[ -d $test_case ]]; then
        $ROOT/scaffold.sh -T $test_case/*.scaffold > tmp.txt

        python get_total_gates_line.py *.resources  > total_gates.txt
        diff total_gates.txt $test_case/total_gates.txt
        if cmp total_gates.txt $test_case/total_gates.txt
        then
            echo -e "[$test_case] Generating Resource Estimate \033[32mSucceeded\033[39m"
			let "successful_tests=successful_tests+1"
        else
            echo -e "[$test_case] Generating Resource Estimate \033[31mFailed\033[39m"
        fi
		let "total_tests=total_tests+1"

        $ROOT/scaffold.sh -q -T $test_case/*.scaffold
        if cmp *.qasmh $test_case/*.qasmh;
        then
            echo -e "[$test_case] Generating Hierarchical QASM \033[32mSucceeded\033[39m"
			let "successful_tests=successful_tests+1"
        else
            echo -e "[$test_case] Generating Hierarchical QASM \033[31mFailed\033[39m"
        fi

		let "total_tests=total_tests+1"

        $ROOT/scaffold.sh -f -T $test_case/*.scaffold
        if cmp *.qasmf $test_case/*.qasmf;
        then
            echo -e "[$test_case] Generating Flattened QASM \033[32mSucceeded\033[39m"
			let "successful_tests=successful_tests+1"
        else
            echo -e "[$test_case] Generating Flattened QASM \033[31mFailed\033[39m"
        fi

		let "total_tests=total_tests+1"

        $ROOT/scaffold.sh -b -T $test_case/*.scaffold
        if cmp *.qasm $test_case/*.qasm;
        then
            echo -e "[$test_case] Generating OpenQASM \033[32mSucceeded\033[39m"
			let "successful_tests=successful_tests+1"
        else
            echo -e "[$test_case] Generating OpenQASM \033[31mFailed\033[39m"
        fi

		let "total_tests=total_tests+1"

        $ROOT/scaffold.sh -c $test_case/*.scaffold
        rm -rf tmp.txt
        rm -rf total_gates.txt
    fi
done
if [ ${successful_tests} -eq ${total_tests} ]; then 
	echo -e "\033[32mAll Tests Successful\033[39m"
else 
	echo -e "\033[31m${successful_tests}/${total_tests} tests successful\033[39m"
fi
echo -e "==================================="
