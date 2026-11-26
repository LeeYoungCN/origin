# 用预定义宏区分编译器（MSVC/MinGW等）与操作系统（Linux/macOS等）

在跨平台C/C++开发中，常需根据不同编译器或操作系统编写适配代码。编译器会自动定义特定宏，可通过条件编译（`#ifdef`/`#elif`）区分环境。本文整理常用宏、使用方法及实战示例，包括获取线程ID和进程路径等跨平台场景。


## 一、核心宏定义说明

### 1. 区分操作系统的宏
不同操作系统由编译器预定义专属宏，可直接用于判断：

| 操作系统       | 对应宏                | 说明                                  |
|----------------|-----------------------|---------------------------------------|
| Windows（32/64位） | `_WIN32`              | 32位和64位Windows均会定义此宏         |
| Linux          | `__linux__`           | 所有Linux发行版（如Ubuntu、CentOS等） |
| macOS          | `__APPLE__`           | 基于苹果系统的环境（包括macOS、iOS等，需结合其他宏进一步区分） |
| FreeBSD        | `__FreeBSD__`         | FreeBSD类Unix系统                     |


### 2. 区分编译器的宏
不同编译器（如MSVC、MinGW、GCC）也有专属宏：

| 编译器         | 对应宏                | 说明                                  |
|----------------|-----------------------|---------------------------------------|
| MSVC（Visual Studio） | `_MSC_VER`       | Microsoft Visual C++编译器，`_MSC_VER`值表示版本（如1930对应VS2022） |
| MinGW          | `__MINGW32__` 或 `__MINGW64__` | Windows上的GCC衍生编译器，32位对应`__MINGW32__`，64位对应`__MINGW64__` |
| GCC            | `__GNUC__`            | GNU编译器集合（Linux/macOS上的GCC）    |
| Clang          | `__clang__`           | LLVM Clang编译器（常见于macOS默认编译器） |


## 二、基础使用示例

### 1. 区分操作系统
```cpp
#ifdef _WIN32
    // Windows系统代码（如调用Win32 API）
    #include <windows.h>
#elif defined(__linux__)
    // Linux系统代码（如调用POSIX接口）
    #include <unistd.h>
#elif defined(__APPLE__)
    // macOS系统代码（如调用Cocoa框架相关接口）
    #include <CoreFoundation/CoreFoundation.h>
#else
    // 未知系统处理（如提示不支持）
    #error "不支持的操作系统"
#endif
```


### 2. 区分编译器
```cpp
#ifdef _MSC_VER
    // MSVC编译器专属代码（如使用MSVC扩展语法）
    #pragma warning(disable: 4996) // 关闭MSVC特定警告
#elif defined(__MINGW32__) || defined(__MINGW64__)
    // MinGW编译器代码（Windows上的GCC，需适配Windows路径）
    #define PATH_SEPARATOR "\\"
#elif defined(__GNUC__)
    // Linux/macOS上的GCC代码（如使用GCC属性）
    __attribute__((unused)) int temp; // 标记未使用变量，避免GCC警告
#elif defined(__clang__)
    // Clang编译器代码（如Clang专属优化）
    #pragma clang diagnostic ignored "-Wunused-variable"
#else
    #error "不支持的编译器"
#endif
```


### 3. 综合区分（操作系统+编译器）
实际开发中常需同时判断系统和编译器（如“Windows+MSVC”与“Windows+MinGW”的差异）：

```cpp
// 先定义统一标识宏，方便后续使用
#ifdef _WIN32
    #define OS_WINDOWS 1
    // 进一步区分Windows下的编译器
    #ifdef _MSC_VER
        #define ENV_WIN_MSVC 1
    #elif defined(__MINGW32__) || defined(__MINGW64__)
        #define ENV_WIN_MINGW 1
    #endif
#elif defined(__linux__)
    #define OS_LINUX 1
    #define ENV_LINUX_GCC 1 // Linux通常用GCC/Clang
#elif defined(__APPLE__)
    #define OS_MACOS 1
    #define ENV_MAC_CLANG 1 // macOS默认用Clang
#endif

// 使用示例：不同环境下的文件路径处理
#ifdef ENV_WIN_MSVC
    // Windows+MSVC：路径分隔符为反斜杠，且需处理长路径
    #define FILE_PATH "C:\\project\\file.txt"
#elif defined(ENV_WIN_MINGW)
    // Windows+MinGW：支持正斜杠（MinGW兼容POSIX路径）
    #define FILE_PATH "C:/project/file.txt"
#elif defined(OS_LINUX) || defined(OS_MACOS)
    // Linux/macOS：路径分隔符为正斜杠
    #define FILE_PATH "/home/user/project/file.txt"
#endif
```


## 三、实战示例：获取线程ID

不同系统获取线程ID的API不同，需通过宏区分实现：

```cpp
#include <iostream>
#ifdef _WIN32
#include <windows.h>  // Windows API头文件
#else
#include <pthread.h>  // POSIX线程头文件
#endif

// 获取当前线程ID
uint64_t get_thread_id() {
#ifdef _WIN32
    // Windows系统：使用GetCurrentThreadId()
    // MSVC和MinGW均支持Windows API
    return GetCurrentThreadId();
#elif defined(__linux__) || defined(__APPLE__)
    // Linux/macOS：使用pthread_self()
    // 注意：pthread_t可能不是数值类型，需转换
    pthread_t tid = pthread_self();
    return static_cast<uint64_t>(tid);
#else
    #error "不支持的系统：无法获取线程ID"
#endif
}

int main() {
    std::cout << "当前线程ID：" << get_thread_id() << std::endl;
    return 0;
}
```

**说明**：
- Windows通过`GetCurrentThreadId()`（需包含`<windows.h>`）。
- Linux/macOS通过`pthread_self()`（需包含`<pthread.h>`）。
- 跨平台项目可封装为统一函数（如上例的`get_thread_id()`）。


## 四、实战示例：获取进程路径

获取当前进程的可执行文件路径，不同系统实现差异较大：

```cpp
#include <iostream>
#include <string>
#include <cstring>
#ifdef _WIN32
#include <windows.h>       // Windows API
#elif defined(__linux__)
#include <unistd.h>        // Linux的readlink函数
#elif defined(__APPLE__)
#include <mach-o/dyld.h>   // macOS的_NSGetExecutablePath
#endif

// 获取当前进程的可执行文件路径
std::string get_process_path() {
#ifdef _WIN32
    // Windows系统：使用GetModuleFileNameA()
    char path[MAX_PATH] = {0};
    // MSVC和MinGW均支持此API
    GetModuleFileNameA(NULL, path, MAX_PATH);
    return std::string(path);

#elif defined(__linux__)
    // Linux系统：读取/proc/self/exe符号链接
    char path[1024] = {0};
    ssize_t len = readlink("/proc/self/exe", path, sizeof(path)-1);
    if (len != -1) {
        path[len] = '\0';
        return std::string(path);
    }
    return "获取Linux进程路径失败";

#elif defined(__APPLE__)
    // macOS系统：使用_NSGetExecutablePath()
    char path[1024] = {0};
    uint32_t size = sizeof(path);
    if (_NSGetExecutablePath(path, &size) == 0) {
        return std::string(path);
    }
    return "获取macOS进程路径失败";

#else
    #error "不支持的系统：无法获取进程路径"
#endif
}

int main() {
    std::cout << "当前进程路径：" << get_process_path() << std::endl;
    return 0;
}
```

**编译说明**：
- Windows（MSVC/MinGW）：无需额外链接库。
- Linux：需包含头文件`<unistd.h>`（`readlink`函数）。
- macOS：`_NSGetExecutablePath`属于系统API，无需额外库。


## 五、注意事项

1. **宏的兼容性**：
   - `_WIN32`在64位Windows中仍会定义（无需用`_WIN64`），兼容性更好。
   - `__APPLE__`不仅适用于macOS，还包括iOS、watchOS等，若需严格区分macOS，可结合`__MACH__`（`#if defined(__APPLE__) && defined(__MACH__)`）。


2. **版本判断（可选）**：
   部分宏可判断版本（如针对特定编译器版本适配）：
   - MSVC：`_MSC_VER`（如`#if _MSC_VER >= 1930`表示VS2022及以上）。
   - GCC：`__GNUC__`（主版本）和`__GNUC_MINOR__`（次版本，如`#if __GNUC__ >= 11`）。


3. **链接库与头文件**：
   - 涉及线程函数时，Linux/macOS编译需加`-lpthread`参数（如`g++ main.cpp -lpthread`）。
   - 系统API需包含对应头文件（如Windows的`<windows.h>`、Linux的`<unistd.h>`）。


4. **避免过度依赖**：
   宏区分仅用于必要的平台差异处理（如系统API调用），尽量通过跨平台库（如Qt、Boost）减少条件编译代码，降低维护成本。


通过上述宏和示例，可高效实现跨平台代码的条件编译，确保在不同环境下的兼容性，特别是系统相关的API调用（如线程ID、进程路径）。实际开发中，建议将这些逻辑封装为工具类，减少重复代码。

## 六、Origin 内置的平台/编译器宏

上面的判断在 `origin` 里已经封装好，不需要再手写一遍。

### 6.1 平台与编译器（`origin/common/macros/compiler.h`）

三个平台宏和四个编译器宏都是 **0/1 值**，可以直接用在 `#if` 里：

| 宏 | 说明 |
|---|---|
| `OS_WINDOWS` | Windows（32/64 位均为 1） |
| `OS_LINUX` | Linux |
| `OS_MACOS` | macOS |
| `COMPILER_MSVC` | MSVC（cl.exe） |
| `COMPILER_MINGW` | MinGW（Windows 上的 GCC） |
| `COMPILER_GCC` | GCC；MinGW 下也为 1 |
| `COMPILER_CLANG` | Clang（含 clang-cl） |

```cpp
#include "origin/common/macros/compiler.h"

#if OS_WINDOWS
    // Windows 专属实现
#elif OS_LINUX
    // Linux 专属实现
#endif

#if COMPILER_MSVC
    // MSVC 专属代码
#endif
```

### 6.2 编译器属性（`origin/common/macros/attributes.h`）

常用属性被封装成 `ATTR_*` 宏，屏蔽 `__declspec` 与 `__attribute__` 的差异：

| 宏 | 作用 |
|---|---|
| `ATTR_FORCE_INLINE` | 强制内联 |
| `ATTR_NOINLINE` | 禁止内联 |
| `ATTR_DEPRECATED(msg)` | 标记废弃接口 |
| `ATTR_ALIGNED(n)` | 指定类型/变量的内存对齐 |

> `ATTR_USED` / `ATTR_PURE` / `ATTR_CONST` / `ATTR_WEAK` 的 MSVC 替代并不等价，已删除；
> 需要这些语义时按编译器直接写扩展属性并做平台验证。

### 6.3 特性探测（`origin/common/macros/feature.h`）

`SUPPORTS_*` 系列是 0/1 值，用于按编译器能力分支：

| 宏 | 说明 |
|---|---|
| `SUPPORTS_THREAD_LOCAL` | `thread_local` |
| `SUPPORTS_CONSTEXPR` | `constexpr` |
| `SUPPORTS_STRUCTURED_BINDINGS` | 结构化绑定 |
| `SUPPORTS_FOLD_EXPRESSIONS` | 折叠表达式 |

> 项目要求 C++20，这些特性在目标编译器上都可用，`feature.h` 主要用于兼容旧代码或第三方环境。
> 不再把 `thread_local` 重定义为 `__thread`；不支持该关键字的环境应直接编译失败。

### 6.4 局部警告控制（`origin/common/macros/warnings.h`）

`warnings.h` 不再在头文件顶层修改全局警告状态，只提供成对宏：

```cpp
#include "origin/common/macros/warnings.h"

ORIGIN_WARNINGS_PUSH
// 可能触发警告的声明或实现
ORIGIN_WARNINGS_POP
```

警告级别本身由目标的编译选项决定（MSVC `/W4`，GCC/Clang `-Wall -Wextra`）。

