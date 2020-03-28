#!/bin/bash

function show_help {
    echo "Usage: $0 [-b <build_dir>]"
    echo "    -b   target build directory"
    echo "    -m   build Scaffold"
}

build_dir=""
build=0
source_dir=$(echo $0 | sed -E -e "s|^(.*)/?build.sh$|\1|g")


echo $0
echo \"$source_dir\"

while getopts "h?mb:" opt; do
    case "$opt" in
    h|\?)
        show_help
        exit 0
        ;;
    b)
        build_dir="${OPTARG}"
        ;;
    m)
        build=1
        ;;
    esac
done

echo $source_dir

if [ "${build_dir}" == "" ]; then
  build_dir="${source_dir}/build"
fi

llvm_src="${source_dir}/llvm"

mkdir "${build_dir}"
cmake -S "${llvm_src}" -B "${build_dir}"
if [ build = 1 ]; then
  make -c "${build_dir}"
fi
