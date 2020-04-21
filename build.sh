#!/bin/bash

function show_help {
    echo "Usage: $0 [-b <build_dir>]"
    echo "    -b   target build directory"
    echo "    -a   build all of LLVM and Scaffold"
    echo "    -m   build Scaffold"
}

curr_dir=$(pwd)
build_dir=""
build=0
source_dir=$(echo $0 | sed -E -e "s|^(.*)/?build.sh$|\1|g")
build_string="llvm-headers clang opt LLVMScaffold"

OS=$(uname -s)

while getopts "h?mab:" opt; do
    case "$opt" in
    h|\?)
        show_help
        exit 0
        ;;
    b)
        build_dir="${OPTARG}"
        ;;
    a)
        build_string=""
        ;;
    m)
        build=1
        ;;
    esac
done

cmake_flags=""
if [ "${OS}" == "Darwin" ]; then
    cmake_flags="-DLLVM_USE_LINKER=gold"
fi

if [ "${build_dir}" == "" ]; then
    build_dir=$(realpath "${source_dir}/build")
fi

llvm_src=$(realpath "${source_dir}/llvm")

mkdir "${build_dir}"
cd ${build_dir}
cmake "${llvm_src}" ${cmake_flags} -DLLVM_ENABLE_PROJECTS="clang"
cd ${curr_dir}
if [ build = 1 ]; then
  make -c "${build_dir}" ${build_string}
fi
