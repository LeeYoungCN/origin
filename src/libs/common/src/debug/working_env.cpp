#include "origin/common/debug/working_env.h"

#include "origin/common/macros/compiler.h"

#if OS_WINDOWS
#include <windows.h>

#include <libloaderapi.h>
#include <minwindef.h>
#include <processthreadsapi.h>
#include <stdlib.h>

#elif OS_LINUX
#include <unistd.h>  // Linux的readlink函数
#elif OS_MACOS
#include <mach-o/dyld.h>  // macOS的_NSGetExecutablePath
#include <unistd.h>
#endif

#include <array>
#include <cstddef>
#include <cstdint>
#include <cstdlib>
#include <filesystem>
#include <iostream>
#include <string>
#include <vector>

#include "working_env_c.h"

namespace {
const char* GetOsName()
{
#ifdef _WIN32
// Windows 32位或64位（_WIN32在64位系统上也会定义）
#ifdef _WIN64
    return "Windows 64-bit";
#else
    return "Windows 32-bit";
#endif
#elif defined(__linux__)
    // Linux 系统
    return "Linux";
#elif defined(__APPLE__) && defined(__MACH__)
    // macOS 系统（基于 Mach 内核）
    return "macOS";
#elif defined(__FreeBSD__)
    // FreeBSD 系统
    return "FreeBSD";
#elif defined(__NetBSD__)
    // NetBSD 系统
    return "NetBSD";
#elif defined(__OpenBSD__)
    // OpenBSD 系统
    return "OpenBSD";
#else
    // 未知操作系统
    return "Unknown OS";
#endif
}

std::vector<const char*> GetCompilers()
{
    static std::vector const compilers = {
#ifdef _MSC_VER
        "_MSC_VER",
#endif
#ifdef __clang__
        "__clang__",
#endif
#ifdef __MINGW32__
        "__MINGW32__",
#endif
#ifdef __MINGW64__
        "__MINGW64__",
#endif
#ifdef __GNUC__
        "__GNUC__",
#endif
    };
    return compilers;
}

std::string GetProcessPath()
{
    constexpr uint32_t BUFFER_LEN = 256;
    char path[BUFFER_LEN] = {'\0'};
#if OS_WINDOWS
    GetModuleFileNameA(nullptr, path, BUFFER_LEN);
#elif OS_LINUX
    auto length = readlink("/proc/self/exe", path, BUFFER_LEN - 1);
    path[length] = '\0';
#elif OS_MACOS
    uint32_t size = sizeof(path);
    _NSGetExecutablePath(path, &size);
#else
#error "Unsupport system."
#endif
    return path;
}

uint64_t GetProcessId()
{
#if OS_WINDOWS
    return GetCurrentProcessId();
#else
    return static_cast<uint64_t>(getpid());
#endif
}

const char* GetArchitecture()
{
// x86 架构 (32位)
#if defined(__i386__) || defined(_M_IX86)
    return "x86 (32-bit)";

// x86_64 架构 (64位)
#elif defined(__x86_64__) || defined(_M_X64)
    return "x86_64 (64-bit)";

// ARM 架构 (32位)
#elif defined(__arm__) || defined(_M_ARM)
    return "ARM (32-bit)";

// ARM64 架构 (64位)
#elif defined(__aarch64__) || defined(_M_ARM64)
    return "ARM64 (64-bit)";

// 其他常见架构
#elif defined(__powerpc__) || defined(__ppc__)
    return "PowerPC";
#elif defined(__mips__)
    return "MIPS";
#elif defined(__riscv)
    return "RISC-V";

// 未知架构
#else
    return "Unknown architecture";
#endif
}

void PrintEnvValue(const char* name)
{
#if COMPILER_MSVC
    char* value = nullptr;
    auto err = 0;
    size_t len = 0;
    err = _dupenv_s(&value, &len, name);
    if (err == 0 && value != nullptr) {
        std::cout << "-------------------- " << name << " --------------------" << std::endl;
        std::cout << name << ": " << value << std::endl;
        free(value);
    }
#else
    if (const auto value = std::getenv(name); value != nullptr) {
        std::cout << "-------------------- " << name << " --------------------" << std::endl;
        std::cout << name << ": " << value << std::endl;
    }
#endif
}

void PrintEnvParams()
{
    constexpr std::array<const char*, 5> ENV_NAMES{
        "PATH", "LD_LIBRARY_PATH", "LIBPATH", "LIB", "INCLUDE"};

    for (const auto& name : ENV_NAMES) {
        PrintEnvValue(name);
    }
}

const char* GetCppStandard()
{
#if __cplusplus >= 202302L
    return "C++23";
#endif
#if __cplusplus >= 202002L
    return "C++20";
#endif
#if __cplusplus >= 201703L
    return "C++17";
#endif
#if __cplusplus >= 201402L
    return "C++14";
#endif
#if __cplusplus >= 201103L
    return "C++11";
#endif
#if __cplusplus >= 199711L
    return "C++98/C++03";
#endif
}

}  // namespace

#ifdef __cplusplus
extern "C" {
#endif

void origin_show_working_env()
{
    std::cout << "OS: " << GetOsName() << std::endl;
    std::cout << "Arch: " << GetArchitecture() << std::endl;
    std::cout << "Compilers: ";
    for (const auto& c : GetCompilers()) {
        std::cout << c << " ";
    }
    std::cout << std::endl;

    std::cout << "C++ Standard: " << GetCppStandard() << std::endl;
    std::cout << "C   Standard: " << GetCStandard() << std::endl;
    std::cout << "Wording directory: " << std::filesystem::current_path() << std::endl;
    std::cout << "Process: " << GetProcessPath() << std::endl;
    std::cout << "Process ID: " << GetProcessId() << std::endl;
    PrintEnvParams();
}

#ifdef __cplusplus
}
#endif
