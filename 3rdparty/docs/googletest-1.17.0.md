# googletest-1.17.0 可精简文件清单

## 当前使用方式

项目根 `CMakeLists.txt` 当前使用的是：

- `find_package(GTest REQUIRED)`

也就是说，当前构建并没有直接 `add_subdirectory(3rdparty/googletest-1.17.0)`，而是依赖系统/工具链中可被 CMake 找到的 GTest 包。`README.md` 中记录过另一种方案：把源码放在 `3rdparty/googletest-1.17.0` 后用 `add_subdirectory(...)` 引入。

因此，精简需要分两种情况看：

- 如果项目继续使用 `find_package(GTest REQUIRED)`，理论上整个 `3rdparty/googletest-1.17.0/` 都不参与当前构建，可以整体移除；但这样会删除本地备用源码，之后无法直接切换到 vendored GTest。
- 如果未来要改成 `add_subdirectory(3rdparty/googletest-1.17.0)`，则需要保留 CMake 构建所需的核心源码、头文件和部分 CMake 辅助文件。

以下清单按“保守精简”口径编写：假设仍希望保留 vendored googletest 源码，以便未来可通过 `add_subdirectory(...)` 构建。

## 可直接精简

这些文件不参与 CMake 默认构建 `gtest`、`gtest_main`、`gmock`、`gmock_main`，删除后通常不会影响作为第三方库嵌入项目。

| 路径 | 理由 |
| --- | --- |
| `3rdparty/googletest-1.17.0/.github/` | 上游 GitHub Issue 模板，只服务上游协作流程，本项目构建不使用。 |
| `3rdparty/googletest-1.17.0/ci/` | 上游 Linux/macOS/Windows presubmit 脚本，本项目 CMake 构建不调用。 |
| `3rdparty/googletest-1.17.0/docs/` | 上游文档站点内容，不参与库构建。 |
| `3rdparty/googletest-1.17.0/.clang-format` | 上游代码格式化配置，不参与编译；如果不打算格式化第三方源码可删除。 |
| `3rdparty/googletest-1.17.0/.gitignore` | 上游仓库忽略规则，作为 vendored 第三方源码时不影响构建。 |
| `3rdparty/googletest-1.17.0/CONTRIBUTING.md` | 上游贡献指南，不参与构建。 |
| `3rdparty/googletest-1.17.0/README.md` | 上游说明文档，不参与构建；如果需要保留第三方库说明或升级参考，可保留。 |
| `3rdparty/googletest-1.17.0/googletest/README.md` | googletest 子项目说明文档，不参与构建。 |
| `3rdparty/googletest-1.17.0/googlemock/README.md` | googlemock 子项目说明文档，不参与构建。 |
| `3rdparty/googletest-1.17.0/googletest/docs/` | googletest 子项目文档，不参与构建。 |
| `3rdparty/googletest-1.17.0/googlemock/docs/` | googlemock 子项目文档，不参与构建。 |
| `3rdparty/googletest-1.17.0/BUILD.bazel` | Bazel 构建入口；本项目使用 CMake 时不需要。 |
| `3rdparty/googletest-1.17.0/MODULE.bazel` | Bazel bzlmod 配置；本项目使用 CMake 时不需要。 |
| `3rdparty/googletest-1.17.0/WORKSPACE` | Bazel workspace 配置；本项目使用 CMake 时不需要。 |
| `3rdparty/googletest-1.17.0/WORKSPACE.bzlmod` | Bazel workspace/bzlmod 配置；本项目使用 CMake 时不需要。 |
| `3rdparty/googletest-1.17.0/googletest_deps.bzl` | Bazel 依赖声明；本项目使用 CMake 时不需要。 |
| `3rdparty/googletest-1.17.0/fake_fuchsia_sdk.bzl` | Bazel/Fuchsia 相关辅助文件；本项目使用 CMake 时不需要。 |
| `3rdparty/googletest-1.17.0/googletest/test/` | GoogleTest 自测源码，默认 `gtest_build_tests=OFF`，不参与业务测试构建。 |
| `3rdparty/googletest-1.17.0/googlemock/test/` | GoogleMock 自测源码，默认 `gmock_build_tests=OFF`，不参与业务测试构建。 |
| `3rdparty/googletest-1.17.0/googletest/samples/` | GoogleTest 示例源码，默认 `gtest_build_samples=OFF`，不参与构建。 |

## 可按需精简

这些文件是否可以删除，取决于项目后续是否需要对应能力。

| 路径 | 可删条件 | 理由 |
| --- | --- | --- |
| `3rdparty/googletest-1.17.0/CONTRIBUTORS` | 不需要保留上游贡献者信息时。 | 不参与构建；但出于第三方源码溯源，建议和 `LICENSE` 一起保留。 |
| `3rdparty/googletest-1.17.0/googletest/cmake/Config.cmake.in` | `INSTALL_GTEST=OFF`，且不需要安装/导出 `GTestConfig.cmake` 时。 | 仅用于生成安装包的 CMake package 配置。 |
| `3rdparty/googletest-1.17.0/googletest/cmake/gtest.pc.in` | `INSTALL_GTEST=OFF`，且不需要 pkg-config 文件时。 | 仅用于安装导出 `gtest.pc`。 |
| `3rdparty/googletest-1.17.0/googletest/cmake/gtest_main.pc.in` | `INSTALL_GTEST=OFF`，且不需要 pkg-config 文件时。 | 仅用于安装导出 `gtest_main.pc`。 |
| `3rdparty/googletest-1.17.0/googletest/cmake/libgtest.la.in` | 不使用 libtool `.la` 安装产物时。 | 仅用于安装相关元数据。 |
| `3rdparty/googletest-1.17.0/googlemock/cmake/gmock.pc.in` | `BUILD_GMOCK=ON` 但 `INSTALL_GTEST=OFF`，且不需要 pkg-config 文件时。 | 仅用于安装导出 `gmock.pc`。 |
| `3rdparty/googletest-1.17.0/googlemock/cmake/gmock_main.pc.in` | `BUILD_GMOCK=ON` 但 `INSTALL_GTEST=OFF`，且不需要 pkg-config 文件时。 | 仅用于安装导出 `gmock_main.pc`。 |
| `3rdparty/googletest-1.17.0/googlemock/` | 项目只使用 `GTest::gtest` / `GTest::gtest_main`，不使用 `gmock` / `gmock_main`，并在引入源码时设置 `BUILD_GMOCK=OFF`。 | 顶层 CMake 默认 `BUILD_GMOCK=ON` 会进入 `googlemock/`；若不关闭该选项直接删除会导致配置失败。 |

## 需谨慎精简源码文件

GoogleTest/GoogleMock 的 CMake 默认通过聚合源文件构建：

- `googletest/src/gtest-all.cc`
- `googletest/src/gtest_main.cc`
- `googlemock/src/gmock-all.cc`
- `googlemock/src/gmock_main.cc`

这些聚合源文件内部会 `#include` 同目录下的多个 `.cc` 和内部头文件。因此不能只看 CMakeLists 中列出的源文件就删除其它 `.cc`。

| 路径 | 说明 |
| --- | --- |
| `3rdparty/googletest-1.17.0/googletest/src/gtest-all.cc` | 构建 `gtest` 的聚合源文件，必须保留。 |
| `3rdparty/googletest-1.17.0/googletest/src/gtest_main.cc` | 构建 `gtest_main` 的 main 入口，使用 `GTest::gtest_main` 时必须保留。 |
| `3rdparty/googletest-1.17.0/googletest/src/*.cc` | 被 `gtest-all.cc` 间接包含，通常都应保留。 |
| `3rdparty/googletest-1.17.0/googletest/src/gtest-internal-inl.h` | gtest 内部实现头，被源码包含，必须保留。 |
| `3rdparty/googletest-1.17.0/googlemock/src/gmock-all.cc` | 构建 `gmock` 的聚合源文件；使用 GoogleMock 时必须保留。 |
| `3rdparty/googletest-1.17.0/googlemock/src/gmock_main.cc` | 构建 `gmock_main` 的 main 入口；使用 `GTest::gmock_main` 时必须保留。 |
| `3rdparty/googletest-1.17.0/googlemock/src/*.cc` | 被 `gmock-all.cc` 间接包含，使用 GoogleMock 时通常都应保留。 |

## 不建议精简

以下文件是作为 vendored CMake 源码构建时的核心文件，或出于许可证/溯源建议保留。

| 路径 | 理由 |
| --- | --- |
| `3rdparty/googletest-1.17.0/LICENSE` | 第三方库许可证，必须保留。 |
| `3rdparty/googletest-1.17.0/CMakeLists.txt` | `add_subdirectory(3rdparty/googletest-1.17.0)` 的入口。 |
| `3rdparty/googletest-1.17.0/googletest/CMakeLists.txt` | 定义 `gtest`、`gtest_main` 目标。 |
| `3rdparty/googletest-1.17.0/googletest/include/` | 对外头文件，业务测试通过 `<gtest/gtest.h>` 等包含。 |
| `3rdparty/googletest-1.17.0/googletest/src/` | GoogleTest 核心实现源码。 |
| `3rdparty/googletest-1.17.0/googletest/cmake/internal_utils.cmake` | GoogleTest CMake 目标创建、编译选项和安装函数依赖的内部脚本。 |
| `3rdparty/googletest-1.17.0/googlemock/CMakeLists.txt` | 默认 `BUILD_GMOCK=ON` 时需要；如果保留 GoogleMock 就必须保留。 |
| `3rdparty/googletest-1.17.0/googlemock/include/` | GoogleMock 对外头文件，使用 `<gmock/gmock.h>` 时需要。 |
| `3rdparty/googletest-1.17.0/googlemock/src/` | GoogleMock 核心实现源码。 |

## 推荐精简策略

1. 如果确认项目长期使用系统 `find_package(GTest REQUIRED)`，可以删除整个 `3rdparty/googletest-1.17.0/`；但这会失去本地备用源码。
2. 如果想保留 vendored 源码，第一阶段只删除“可直接精简”项：上游文档、CI、Bazel 配置、自测和示例。
3. 如果项目只需要 GoogleTest，不需要 GoogleMock，第二阶段可在引入源码前设置 `BUILD_GMOCK=OFF`，再删除 `googlemock/`。
4. 如果不需要安装导出包，建议在引入源码前设置 `INSTALL_GTEST=OFF`，再考虑删除安装导出模板文件。
5. 每次精简后都运行一次 CMake 配置和至少构建 `gtest`、`gtest_main`；如果保留 GoogleMock，也构建 `gmock`、`gmock_main`。
