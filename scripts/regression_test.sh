#!/bin/bash

DIR=$(dirname $0)
ROOT=$DIR/..
OPT=$ROOT/build/Release+Asserts/bin/opt
UNAME_S=$(uname -s)
if [ $UNAME_S = "Darwin" ]; then
    SCAF=$ROOT/build/Release+Asserts/lib/Scaffold.dylib
else
    SCAF=$ROOT/build/Release+Asserts/lib/Scaffold.so
fi

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
            echo -e "[$test_case] Generating Resource Estimate \e[32mSucceeded\e[39m"
			let "successful_tests=successful_tests+1"
        else
            echo -e "[$test_case] Generating Resource Estimate \e[31mFailed\e[39m"
        fi
		let "total_tests=total_tests+1"

        $ROOT/scaffold.sh -q -T $test_case/*.scaffold
        if cmp *.qasmh $test_case/*.qasmh;
        then
            echo -e "[$test_case] Generating Hierarchical QASM \e[32mSucceeded\e[39m"
			let "successful_tests=successful_tests+1"
        else
            echo -e "[$test_case] Generating Hierarchical QASM \e[31mFailed\e[39m"
        fi

		let "total_tests=total_tests+1"

        $ROOT/scaffold.sh -f -T $test_case/*.scaffold
        if cmp *.qasmf $test_case/*.qasmf;
        then
            echo -e "[$test_case] Generating Flattened QASM \e[32mSucceeded\e[39m"
			let "successful_tests=successful_tests+1"
        else
            echo -e "[$test_case] Generating Flattened QASM \e[31mFailed\e[39m"
        fi

		let "total_tests=total_tests+1"

        $ROOT/scaffold.sh -b -T $test_case/*.scaffold
        if cmp *.qasm $test_case/*.qasm;
        then
            echo -e "[$test_case] Generating OpenQASM \e[32mSucceeded\e[39m"
			let "successful_tests=successful_tests+1"
        else
            echo -e "[$test_case] Generating OpenQASM \e[31mFailed\e[39m"
        fi

		let "total_tests=total_tests+1"

        $ROOT/scaffold.sh -c $test_case/*.scaffold
        rm -rf tmp.txt
        rm -rf total_gates.txt
    fi
done
if [ ${successful_tests} -eq ${total_tests} ]; then 
	echo -e "\e[32mAll Tests Successful\e[39m"
else 
	echo -e "\e[31m${successful_tests}/${total_tests} tests successful\e[39m"
fi
echo -e "==================================="
