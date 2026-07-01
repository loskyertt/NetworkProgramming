#!/usr/bin/env bash

set -e

cd ..

BUILD_DIR="cmake-build-remote"

usage() {
    cat <<EOF
用法：
    bash clean.sh -b
    bash clean.sh --build

        清理 CMake 构建目录（删除 ${BUILD_DIR}）。

    bash clean.sh -c
    bash clean.sh --compile

        清理编译产物（保留 CMake 配置）。

    bash clean.sh -a
    bash clean.sh --all

        清理所有生成文件（构建目录、日志等）。

参数说明：
    -b, --build      清理构建目录
    -c, --compile    清理编译产物
    -a, --all        清理全部

EOF
}

clean_build() {
    if [[ -d "${BUILD_DIR}" ]]; then
        echo ">>> 删除构建目录：${BUILD_DIR}"
        rm -rf "${BUILD_DIR}"
    else
        echo ">>> 构建目录不存在，无需清理。"
    fi
}

clean_compile() {
    if [[ -d "${BUILD_DIR}" ]]; then
        echo ">>> 清理编译产物..."
        cmake --build "${BUILD_DIR}" --target clean
    else
        echo ">>> 构建目录不存在，请先执行 build.sh -b"
    fi
}

clean_all() {
    clean_build

    echo ">>> 清理日志文件..."

    rm -rf log/*
    rm -rf logs/*

    echo ">>> 全部清理完成。"
}

case "$1" in
-b | --build)
    clean_build
    ;;
-c | --compile)
    clean_compile
    ;;
-a | --all)
    clean_all
    ;;
*)
    usage
    exit 1
    ;;
esac
