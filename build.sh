#!/usr/bin/env bash

set -e

BUILD_DIR="cmake-build-remote"

usage() {
    cat <<EOF
用法：
    bash build.sh -b
    bash build.sh --build

        生成 CMake 构建目录。

    bash build.sh -c
    bash build.sh --compile

        编译项目。

说明：
    -b, --build      执行 CMake 配置
    -c, --compile    编译工程

EOF
}

build() {
    cmake -B "${BUILD_DIR}" \
        -G "Unix Makefiles" \
        -DCMAKE_BUILD_TYPE=Debug \
        -DCMAKE_EXPORT_COMPILE_COMMANDS=ON \
        -DCMAKE_C_COMPILER=gcc \
        -DCMAKE_CXX_COMPILER=g++
}

compile() {
    local jobs

    if command -v nproc >/dev/null 2>&1; then
        jobs=$(nproc)
    else
        jobs=4
    fi

    cmake --build "${BUILD_DIR}" -j"${jobs}"
}

case "$1" in
    -b|--build)
        build
        ;;
    -c|--compile)
        compile
        ;;
    *)
        usage
        exit 1
        ;;
esac