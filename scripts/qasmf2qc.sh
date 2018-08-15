#!/bin/bash
#Shell script that converts .qasmf files to .qc files,
#which are compatible with the QX simulator.
#
# qubits #
# qubit a0 -> map q0, a0
# cbit ma0 -> map b0, ma0
# CNOT a0, a1 -> cnot a0, a1
# Toffoli a0, a1, a2 -> toffoli a0, a1, a2
# MeasZ a0 -> measure a0


exec 10<&0

if [ -z "$1" ]
  then
    echo "[QC Transformer] No arguments supplied, invoke with a qasm file"
    echo -e "\t./qasmf2qc.sh <application_name>.qasmf"
	exit
fi

exec < $1
in=$1

#init
file="${in/qasmf/qc}"
init_file="init_file.qc"
err=0
qubit="qubit *"
cbit="cbit *"
cnot="CNOT *"
toffoli="Tof *"
measZ="MeasZ *"
measX="MeasX *"
prepX="PrepX *"

if [ -f "$file" ]; then echo "Stopped: $file already exists."
else
echo "[QCTransform] Generating $file ..."
let count=0
c_index=0
m_index=0
q_pre='qubit '
c_pre='cbit '
mz_pre='MeasZ '
declare -A labels #associative array requires bash 4.0 or above

while read LINE
do
  case $LINE in
    $qubit)
      # initialize qubits
      echo "${LINE/qubit /map q$count, }" >> $init_file
      labels["${LINE#$q_pre}"]=$count #labels["a0"]=0
      #echo "${LINE/qubit /map b$count, m}" >> $file
      (( count++ ))
    ;;
    $cbit)
      # initialize classical bits (measurement results)
      c_array[$c_index]="${LINE#$c_pre}" #c_array[0]=ma0
      (( c_index++ ))
    ;;
    $cnot)
      # cnot gate
      arrIn=(${LINE/,/ })
      echo "cnot ${arrIn[1]}, ${arrIn[2]}" >> $file
    ;;
    $toffoli)
      # toffoli gate
      arrIn=(${LINE//,/ })
      echo "toffoli ${arrIn[1]}, ${arrIn[2]}, ${arrIn[3]}" >> $file
    ;;
    $measZ)
      # measurement
      echo "${LINE/MeasZ/measure}" >> $file
      c_name=${c_array[m_index]}
      c_bit="b${labels["${LINE#$mz_pre}"]}"
      echo "map $c_bit, $c_name" >> $init_file
      (( m_index++ ))
    ;;
    $measX | $prepX )
      # unsupported gates
      echo "Stopped: Gate not supported (MeasX or PrepX)!"
      err=1
    ;;
    *)
      echo $LINE >> $file
    ;;
  esac
done

if [ "$err" -ne 0 ];
  then
    rm "$file"
    echo "No file generated."
  else
    echo "display" >> $file
    echo "qubits $count" | cat - $init_file > temp && mv temp $init_file
    cat $file >> $init_file && mv $init_file $file
    echo "[QCTransform] Done. Number of qubits: $count"
fi
fi
exec 0<&10 10<&-
