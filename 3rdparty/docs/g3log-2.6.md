# g3log-2.6 可精简文件清单

## 当前使用方式

项目根 `CMakeLists.txt` 通过 `add_subdirectory(3rdparty/g3log-2.6)` 直接构建 g3log，并在 `cmake/set_g3log.cmake` 中关闭了单元测试、示例和安装：

- `ADD_G3LOG_UNIT_TEST=OFF`
- `ADD_FATAL_EXAMPLE=OFF`
- `INSTALL_G3LOG=OFF`
- `G3_SHARED_LIB=OFF`

因此，当前业务编译主要依赖 `src/` 下的源码和头文件，以及 g3log 顶层 CMake 构建脚本。以下清单按“可直接精简”和“需配套修改 CMake 后可精简”区分。

## 可直接精简

这些文件不参与当前项目构建，删除后通常不会影响 `add_subdirectory(3rdparty/g3log-2.6)` 构建 g3log 静态库。

| 路径 | 理由 |
| --- | --- |
| `3rdparty/g3log-2.6/.github/` | 上游 GitHub Actions、Issue 模板，只服务上游仓库 CI 和协作流程，本项目构建不使用。 |
| `3rdparty/g3log-2.6/.devcontainer/` | 上游 Codespaces/Dev Container 开发环境配置，本项目本地构建不依赖。 |
| `3rdparty/g3log-2.6/.vscode/` | 上游 VS Code 工作区配置，只影响编辑器体验，不参与编译。 |
| `3rdparty/g3log-2.6/.clang-format` | 上游代码格式化配置，不参与编译；如果不打算格式化第三方源码可删除。 |
| `3rdparty/g3log-2.6/.gitignore` | 上游仓库忽略规则；作为 vendored 第三方源码时不影响构建。 |
| `3rdparty/g3log-2.6/.hgrc_copy` | 上游 Mercurial/Bitbucket 迁移相关配置，本项目不用。 |
| `3rdparty/g3log-2.6/.hgtags` | 上游 Mercurial tag 信息，本项目不用。 |
| `3rdparty/g3log-2.6/CODE_OF_CONDUCT.md` | 上游社区治理文档，不参与构建。 |
| `3rdparty/g3log-2.6/CONTRIBUTING.md` | 上游贡献指南，不参与构建。 |
| `3rdparty/g3log-2.6/PULL_REQUEST_TEMPLATE.md` | 上游 PR 模板，不参与构建。 |
| `3rdparty/g3log-2.6/README.md` | 上游说明文档，不参与构建；如果需要保留第三方库说明或升级参考，可保留。 |
| `3rdparty/g3log-2.6/docs/` | 上游文档站点内容和图片，不参与构建；其中 `.DS_Store`、`.ciignore` 尤其可删除。 |
| `3rdparty/g3log-2.6/mkdocs.yml` | 上游文档站点配置，仅用于生成文档站点，不参与当前编译。 |
| `3rdparty/g3log-2.6/scripts/` | 上游 Travis/测试脚本，本项目 CMake 构建不调用。 |
| `3rdparty/g3log-2.6/CMakeLists.txt.in` | 仅在 `ADD_G3LOG_UNIT_TEST=ON` 时用于下载 googletest；当前已关闭单元测试。 |
| `3rdparty/g3log-2.6/cmake/g3logConfig.cmake` | 仅在 `INSTALL_G3LOG=ON` 安装/导出 CMake package 时使用；当前已关闭安装。 |
| `3rdparty/g3log-2.6/iOS.cmake` | iOS 工具链配置文件；当前项目未引用该文件。 |

## 需配套修改 CMake 后可精简

这些目录中的源码目标已被当前配置关闭，但对应 `.cmake` 文件仍被 `3rdparty/g3log-2.6/CMakeLists.txt` 无条件 `include`。如果直接删除整个目录，会导致 CMake 配置阶段找不到被 include 的文件。若要删除，需要先修改 g3log 的 `CMakeLists.txt`，移除或条件化对应 `include(...)`。

| 路径 | 当前状态 | 精简方式 |
| --- | --- | --- |
| `3rdparty/g3log-2.6/example/main_contract.cpp` | 示例目标由 `ADD_FATAL_EXAMPLE=OFF` 关闭，不参与构建。 | 可删除示例源码；但需保留 `example/Example.cmake`，或从 `CMakeLists.txt` 移除 `include(${g3log_SOURCE_DIR}/example/Example.cmake)` 后删除整个 `example/`。 |
| `3rdparty/g3log-2.6/example/main_fatal_choice.cpp` | 同上。 | 同上。 |
| `3rdparty/g3log-2.6/example/main_sigsegv.cpp` | 同上。 | 同上。 |
| `3rdparty/g3log-2.6/test_unit/*.cpp` | 单元测试由 `ADD_G3LOG_UNIT_TEST=OFF` 关闭，不参与构建。 | 可删除测试源码；但需保留 `test_unit/Test.cmake`，或从 `CMakeLists.txt` 移除 `include(${g3log_SOURCE_DIR}/test_unit/Test.cmake)` 后删除整个 `test_unit/`。 |
| `3rdparty/g3log-2.6/test_unit/*.h` | 单元测试辅助头文件，不参与构建。 | 同上。 |
| `3rdparty/g3log-2.6/test_unit/Test.cmake` | 当前仍被顶层 CMake 无条件 include。 | 只有在修改 `CMakeLists.txt` 移除对应 include 后才能删除。 |
| `3rdparty/g3log-2.6/test_main/` | 只给 g3log 单元测试提供 `test_main.cpp`，当前测试关闭。 | 删除前需同时处理 `test_unit/Test.cmake` 或顶层 include。 |
| `3rdparty/g3log-2.6/test_performance/*.cpp` | 性能测试默认关闭，不参与构建。 | 可删除性能测试源码；但需保留 `test_performance/Performance.cmake`，或从 `CMakeLists.txt` 移除 `include(${g3log_SOURCE_DIR}/test_performance/Performance.cmake)` 后删除整个 `test_performance/`。 |
| `3rdparty/g3log-2.6/test_performance/performance.h` | 性能测试辅助头文件，不参与构建。 | 同上。 |
| `3rdparty/g3log-2.6/test_performance/Performance.cmake` | 当前仍被顶层 CMake 无条件 include。 | 只有在修改 `CMakeLists.txt` 移除对应 include 后才能删除。 |
| `3rdparty/g3log-2.6/CPackLists.txt` | `INSTALL_G3LOG=OFF` 时不会安装，但仍被顶层 CMake 无条件 include，并会配置 CPack。 | 若不需要安装包/导出包，可从 `CMakeLists.txt` 移除 `include(${g3log_SOURCE_DIR}/CPackLists.txt)` 后删除。 |
| `3rdparty/g3log-2.6/iOSBuild.cmake` | 当前普通 Windows/Linux 构建不走 `G3_IOS_LIB`，但仍被顶层 CMake 无条件 include。 | 若确认不支持 iOS 构建，可从 `CMakeLists.txt` 移除 `include(${g3log_SOURCE_DIR}/iOSBuild.cmake)` 后删除。 |
| `3rdparty/g3log-2.6/CleanAll.cmake` | 只用于非 MSVC 下的 `clean-cmake` 自定义目标。 | 若不需要该目标，可删除 `CMakeLists.txt` 中 `add_custom_target(clean-cmake ...)` 相关代码后删除。 |

## 不建议精简

以下文件是当前直接构建 g3log 所需，或出于合规/溯源建议保留。

| 路径 | 理由 |
| --- | --- |
| `3rdparty/g3log-2.6/src/` | g3log 核心源码和对外头文件，当前业务代码通过 `<g3log/...>` 使用。 |
| `3rdparty/g3log-2.6/CMakeLists.txt` | `add_subdirectory(3rdparty/g3log-2.6)` 入口。 |
| `3rdparty/g3log-2.6/Options.cmake` | 当前 `cmake/set_g3log.cmake` 设置的多个选项在这里定义并参与生成配置头。 |
| `3rdparty/g3log-2.6/GenerateMacroDefinitionsFile.cmake` | 生成 `g3log/generated_definitions.hpp`，被核心源码包含。 |
| `3rdparty/g3log-2.6/Build.cmake` | 定义 `g3log` 静态库目标、源码收集、include 路径和平台链接库。 |
| `3rdparty/g3log-2.6/LICENSE` | 第三方库许可证/版权声明，建议保留。 |

## 推荐精简策略

1. 第一阶段只删除“可直接精简”项，风险最低，不需要改第三方库 CMake。
2. 第二阶段如果需要进一步瘦身，再修改 `3rdparty/g3log-2.6/CMakeLists.txt`，移除示例、测试、性能测试、CPack、iOS、clean-cmake 的 include/target 后删除对应目录和脚本。
3. 每次精简后都重新运行项目 CMake 配置和构建，确认 `g3log` 目标仍可生成，业务目标仍能链接 `g3log`。
