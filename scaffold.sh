#!/bin/bash 

ROOT=$(dirname $0)

# Get RKQC path
export RKQC_PATH=${ROOT}/rkqc
if [ $(echo $PATH | grep ${RKQC_PATH} | wc -l) -eq 0 ]; then
	export PATH=$PATH:$RKQC_PATH
fi

function show_help {
    echo "Usage: $0 [-hv] [-rqfRTFckdso] [-l #] [-P #] <filename>.scaffold"
    echo "    -r   Generate resource estimate (default)"
    echo "    -q   Generate QASM"
    echo "    -f   Generate flattened QASM"
    echo "    -b   Generate OpenQASM"
    echo "    -R   Enable rotation decomposition"
    echo "    -T   Enable Toffoli decomposition"
    echo "    -l   Levels of recursion to run (default=1)"
    echo "    -P   Set precision of rotation decomposition in decimal digits (default=10)"
    echo "    -F   Force running all steps"
    echo "    -c   Clean all files (no other actions)"
    echo "    -k   Keep all intermediate files (default only keeps specified output,"
    echo "         but requires recompilation for any new output)"
    echo "    -d   Dry-run; show all commands to be run, but do not execute"
    echo "    -s   Generate QX Simulator input file"
    echo "    -o   Generate optimized QASM"
    echo "    -v   Show current Scaffold version information"
}

function show_version {
    echo "Scaffold - Release 4.1 (June 28, 2018)"
}

# Parse opts
OPTIND=1         # Reset in case getopts has been used previously in the shell.
rkqc=0
clean=0
dryrun=""
force=0
purge=1
res=0
rot=0
toff=0
flat=0
openqasm=0
qc=0
precision=4
targets=""
optimize=0
while getopts "h?vcdfbsFkqroTRl:P:" opt; do
    case "$opt" in
    h|\?)
        show_help
        exit 0
        ;;
    v)
        show_version
        exit 0
        ;;
    c) clean=1
        ;;
    d) dryrun="--dry-run"
        ;;
    F) force=1
        ;;
    f) flat=1
        ;;
	b) openqasm=1
		;;
    k) purge=0
        ;;
    q) targets="${targets} qasm"
        ;;
    r) res=1
        ;;
    R) rot=1
        ;;
    T) toff=1
        ;;        
    s) qc=1
        ;;
    o) optimize=1
        ;;
    l) targets="${targets} SQCT_LEVELS=${OPTARG}"
        ;;
    P) precision=${OPTARG}
    esac
done
shift $((OPTIND-1))
[ "$1" = "--" ] && shift

if [ ${flat} -eq 1 ]; then
	targets="${targets} flat"
fi

if [ ${openqasm} -eq 1 ]; then
	targets="${targets} openqasm"
fi

if [ ${qc} -eq 1 ]; then
	targets="${targets} qc"
fi

if [ ${optimize} -eq 1 ]; then
	targets="${targets} optimize"
fi

# Put resources at the end so it is easy to read
if [ ${res} -eq 1 ]; then
    targets="${targets} resources"
fi
# Force first
if [ ${force} -eq 1 ]; then
    targets="clean ${targets}"
fi
# Default to resource estimate
if [ -z "${targets}" ]; then
    targets="resources"
fi
# Don't purge until done
if [ ${purge} -eq 1 ]; then
    targets="${targets} purge"
fi

echo ${1}
if [ $# -lt 1 ]; then 
    echo "Error: Missing filename argument" 
    show_help 
    exit 1 
fi 

filename=${1}
if [ ! -e ${filename} ]; then
    echo "${filename}: file not found"
    show_help
    exit 1
fi
dir="$(dirname ${filename})/"
file=$(basename ${filename} .scaffold)
cfile="${file}.*"

if [ $(egrep '^rkqc.*{\s*' ${filename} | wc -l) -gt 0 ]; then
	rkqc=1
	toff=1
fi

if [ ${clean} -eq 1 ]; then
	make -f $ROOT/scaffold/Scaffold.makefile ${dryrun} ROOT=$ROOT DIRNAME=${dir} FILENAME=${filename} FILE=${file} CFILE=${cfile} clean
    exit
fi
make -f $ROOT/scaffold/Scaffold.makefile ${dryrun} ROOT=$ROOT DIRNAME=${dir} FILENAME=${filename} FILE=${file} CFILE=${cfile} TOFF=${toff} RKQC=${rkqc} ROTATIONS=${rot} PRECISION=${precision} OPTIMIZE=${optimize} ${targets}

exit 0
