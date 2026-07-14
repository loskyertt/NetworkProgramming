#!/usr/bin/env bash

set -e

# 获取脚本文件所在的真实绝对路径（处理软链接）
SCRIPT_DIR="$(cd "$(dirname "$0")" && pwd -P)"

# 获取当前执行时所在的工作目录
CURRENT_DIR="$(pwd -P)"

if [ "$SCRIPT_DIR" != "$CURRENT_DIR" ]; then
    echo "错误：请在脚本所在目录（$SCRIPT_DIR）下执行此脚本" >&2
    exit 1
fi

cd ..

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
        -G Ninja \
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
-b | --build)
    build
    ;;
-c | --compile)
    compile
    ;;
*)
    usage
    exit 1
    ;;
esac
