for f in $(cat file_list)
do
    echo $f
    ../scaffold.sh ${f}.scaffold
    rm -f ${f}_merged.scaffold ${f}_noctqg.scaffold ${f}.ll ${f}1.ll ${f}1a.ll ${f}1b.ll ${f}2.ll ${f}3.ll ${f}3a.ll ${f}4.ll ${f}5.ll ${f}5a.ll ${f}6.ll ${f}7.ll ${f}8.ll ${f}9.ll ${f}10.ll ${f}tmp.ll ${f}_qasm ${f}_qasm.scaffold fdecl.out ${f}.tmp ${f}.qhf ${f}_res_count.log
    mv ${f}11.ll ${f}.ll
done
