#!/bin/bash 

ROOT=$(dirname $0)

# Get RKQC path
export RKQC_PATH=${ROOT}/rkqc
if [ $(echo $PATH | grep ${RKQC_PATH} | wc -l) -eq 0 ]; then
	export PATH=$PATH:$RKQC_PATH
fi

function show_help {
    echo "Usage: $0 [-hv] [-rqfRFcpds] [-L #] <filename>.scaffold"
    echo "    -r   Generate resource estimate (default)"
    echo "    -q   Generate QASM"
    echo "    -f   Generate flattened QASM"
    echo "    -R   Disable rotation decomposition"
    echo "    -T   Disable Toffoli decomposition"    
	echo "    -l   Levels of recursion to run (default=1)"
    echo "    -F   Force running all steps"
    echo "    -c   Clean all files (no other actions)"
    echo "    -p   Purge all intermediate files (preserves specified output,"
    echo "         but requires recompilation for any new output)"
    echo "    -d   Dry-run; show all commands to be run, but do not execute"
    echo "    -s   Generate QX Simulator input file"  
    echo "    -v   Show current Scaffold version information" 
}

function show_version {
    echo "Scaffold - Release 2.0 (July 10, 2016) Beta"
}

# Parse opts
OPTIND=1         # Reset in case getopts has been used previously in the shell.
rkqc=0
clean=0
dryrun=""
force=0
purge=0
res=0
rot=1
toff=1
flat=0
qc=0
targets=""
while getopts "h?vcdfsFpqrRTl:" opt; do
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
    p) purge=1
        ;;
    q) targets="${targets} qasm"
        ;;
    r) res=1
        ;;
    R) rot=0
        ;;
    T) toff=0
        ;;        
	s) qc=1
		;;
    l) targets="${targets} SQCT_LEVELS=${OPTARG}"
        ;;
    esac
done
shift $((OPTIND-1))
[ "$1" = "--" ] && shift

if [ ${flat} -eq 1 ]; then
	targets="${targets} flat"
fi

if [ ${qc} -eq 1 ]; then
	targets="${targets} qc"
fi

# Put resources at the end so it is easy to read
if [ ${res} -eq 1 ]; then
    targets="${targets} resources"
fi
# Don't purge until done
if [ ${purge} -eq 1 ]; then
    targets="${targets} purge"
fi
# Force first
if [ ${force} -eq 1 ]; then
    targets="clean ${targets}"
fi
# Default to resource estimate
if [ -z "${targets}" ]; then
    targets="resources"
fi

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
make -f $ROOT/scaffold/Scaffold.makefile ${dryrun} ROOT=$ROOT DIRNAME=${dir} FILENAME=${filename} FILE=${file} CFILE=${cfile} TOFF=${toff} RKQC=${rkqc} ROTATIONS=${rot} ${targets}

exit 0
