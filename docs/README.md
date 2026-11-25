# Origin 文档

这里是 Origin 的文档入口。构建、安装和版本管理请先看根目录的 [README.md](../README.md)。

## 入门

- [README.md](../README.md)：构建、preset、安装、版本管理
- [集成指南](integration.md)：下游项目的 `find_package`、`FetchContent`、`add_subdirectory` 和 vcpkg 用法

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
