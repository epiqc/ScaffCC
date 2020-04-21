#!/bin/bash

function show_help {
    echo "Usage: $0 [-b <build_dir>] [-G <build_system>]"
    echo "    -b   target build directory"
    echo "    -a   build all of LLVM and Scaffold"
    echo "    -m   build Scaffold"
    echo "    -G   Build system (Ninja or Make)"
}

curr_dir=$(pwd)
build_dir=""
build=0
source_dir=$(echo $0 | sed -E -e "s|^(.*)/?build.sh$|\1|g")
build_string="llvm-headers clang opt LLVMScaffold"

OS=$(uname -s)
generator="Ninja"

while getopts "h?mab:G:" opt; do
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
    G)
        generator="${OPTARG}"
        ;;
    esac
done

cmake_flags=""
if [ "${OS}" != "Darwin" ]; then
    cmake_flags="-DLLVM_USE_LINKER=gold"
fi

if [ "${build_dir}" == "" ]; then
    build_dir=$(cd "$(dirname '$0')" && pwd -P)"/build"
fi

if [ "${generator}" != "Ninja" ]; then
    generator="Unix Makefiles"
fi

llvm_src=$(cd "$(dirname '$0')" && pwd -P)"/llvm"

mkdir "${build_dir}"
cd ${build_dir}
cmake -G "${generator}" "${llvm_src}" ${cmake_flags} -DLLVM_ENABLE_PROJECTS="clang"
cd ${curr_dir}
if [ build = 1 ]; then
  ninja -C "${build_dir}" ${build_string}
fi
