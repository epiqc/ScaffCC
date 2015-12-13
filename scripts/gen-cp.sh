THRESHOLDS=(005k 010k 2M)

for f in $(cat file_list)
do
    echo $f
    echo -e "\tComputing module gate counts..."
    ~/ScaffCC/build/Release+Asserts/bin/opt -S -load ~/ScaffCC/build/Release+Asserts/lib/Scaffold.so -ResourceCount2 ${f}.ll > /dev/null 2> ${f}.out

    for th in ${THRESHOLDS[@]}
    do
        echo -e "\tFlattening modules smaller than Th..."
        python gen-flattening-files.py ${f}
        mv ${f}_inline${th}.txt inline_info.txt
        ~/ScaffCC/build/Release+Asserts/bin/opt -S -load ~/ScaffCC/build/Release+Asserts/lib/Scaffold.so -InlineModule -dce -internalize -globaldce ${f}.ll -o ${f}_inline${th}.ll
        echo -e "\tCritical Path Calculation..."
        time ~/ScaffCC/build/Release+Asserts/bin/opt -load ~/ScaffCC/build/Release+Asserts/lib/Scaffold.so -GetCriticalPath ${f}_inline${th}.ll >/dev/null 2> ${f}_inline${th}.log
    done
    rm ${f}.out *txt
done
