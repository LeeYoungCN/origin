# Origin 文档

这里是 Origin 的文档入口。构建、安装和版本管理请先看根目录的 [README.md](../README.md)。

## 新会话阅读顺序

1. [架构与系统模型](architecture.md)：模块、错误模型、日志对象图、所有权、构建产物；
2. [状态快照](status.md)：当前版本/HEAD、审计基线、仍 open 的 P0、文档偏差；
3. [测试与验证](testing.md)：ctest 实际跑什么、测试目标、怎么补用例；
4. 任务相关的组件指南（见下）；
5. 涉及错误码、日志、异常边界或 `utils` / `logging` 改动时，读 [审计记录](audit/README.md)。

## 入门

- [README.md](../README.md)：构建、preset、安装、版本管理
- [集成指南](integration.md)：下游项目的 `find_package`、`FetchContent`、`add_subdirectory` 和 vcpkg 用法

## 项目概览

- [架构与系统模型](architecture.md)：组件分层、错误码布局、日志生命周期、C ABI 约定
- [状态快照](status.md)：当前健康度、open P0、已知文档偏差
- [测试与验证](testing.md)：测试目标矩阵、ctest 注册现状、补测要求

## 组件与工具

- [Logging](guide/logging.md)：logger、formatter、sink 和 C API
- [日期时间](guide/date_time.md)：`origin::date_time`
- [文件系统](guide/filesystem.md)：`origin::filesystem`
- [错误码](guide/system_error_code.md)：`ErrorCode` 与 `std::error_category`
- [平台与编译器宏](guide/platform_macros.md)：Windows、Linux、macOS 和编译器判断
- [Windows 沙箱完整性标签](guide/windows_sandbox_label.md)：工作区写权限问题

## 项目记录

- [审计记录](audit/README.md)：问题清单、证据和修复状态
- [经验记录](lessons.md)：可复用的排查和处理经验
- [CHANGELOG](../CHANGELOG.md)：版本变更记录
