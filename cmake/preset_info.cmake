if (CMAKE_SIZEOF_VOID_P EQUAL 8)
    set(ARCHITECTURE "x64")
else()
    set(ARCHITECTURE "x86")
endif()

# 只有在没有显式给 CMAKE_PRESET 时才推断（用 cmake --preset 时，preset 名由
# CMakePresets.json 直接注入）。推断结果必须和 CMakePresets.json 里的名字一致。
if (NOT CMAKE_PRESET)
    # Clang 的 target 可能来自 CMAKE_<LANG>_COMPILER_TARGET，也可能被塞进
    # CMAKE_C_FLAGS（现有 toolchain 就是这种写法），两种都兼容。
    set(_origin_clang_target "${CMAKE_C_COMPILER_TARGET}")
    if (NOT _origin_clang_target AND CMAKE_C_FLAGS MATCHES "--target=([^ ]+)")
        set(_origin_clang_target "${CMAKE_MATCH_1}")
    endif()

    if (CMAKE_SYSTEM_NAME STREQUAL "Windows")
        if (MSVC AND CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
            # clang-cl：MSVC 变量为真，但编译器是 Clang，必须先判掉，
            # 否则会被当成 MSVC 而和 msvc_* 的安装目录撞车。
            set(RAW_CMAKE_PRESET "Win_Clang_Cl_${CMAKE_BUILD_TYPE}")
        elseif (MSVC)
            set(RAW_CMAKE_PRESET "MSVC_${ARCHITECTURE}_${CMAKE_BUILD_TYPE}")
        elseif (CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
            if (_origin_clang_target MATCHES "mingw")
                set(RAW_CMAKE_PRESET "Win_Clang_MinGW_${CMAKE_BUILD_TYPE}")
            elseif (_origin_clang_target MATCHES "windows-msvc")
                set(RAW_CMAKE_PRESET "Win_Clang_MSVC_${CMAKE_BUILD_TYPE}")
            else()
                message(FATAL_ERROR
                    "无法推断 CMAKE_PRESET：Clang 的 target 未知（'${_origin_clang_target}'）。"
                    "请显式指定 -DCMAKE_PRESET=<preset>")
            endif()
        elseif (MINGW)
            set(RAW_CMAKE_PRESET "Win_MinGW_${CMAKE_BUILD_TYPE}")
        else()
            message(FATAL_ERROR
                "无法推断 CMAKE_PRESET：未知编译器 '${CMAKE_CXX_COMPILER_ID}'。"
                "请显式指定 -DCMAKE_PRESET=<preset>")
        endif()
    elseif (CMAKE_SYSTEM_NAME STREQUAL "Linux")
        if (CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
            set(RAW_CMAKE_PRESET "Linux_GNU_${CMAKE_BUILD_TYPE}")
        elseif (CMAKE_CXX_COMPILER_ID MATCHES "Clang")
            set(RAW_CMAKE_PRESET "Linux_Clang_${CMAKE_BUILD_TYPE}")
        else()
            message(FATAL_ERROR
                "无法推断 CMAKE_PRESET：未知编译器 '${CMAKE_CXX_COMPILER_ID}'。"
                "请显式指定 -DCMAKE_PRESET=<preset>")
        endif()
    elseif (CMAKE_SYSTEM_NAME STREQUAL "Darwin")
        if (CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
            set(RAW_CMAKE_PRESET "Darwin_GNU_${CMAKE_BUILD_TYPE}")
        elseif (CMAKE_CXX_COMPILER_ID MATCHES "(Apple)?Clang")
            set(RAW_CMAKE_PRESET "Darwin_Clang_${CMAKE_BUILD_TYPE}")
        else()
            message(FATAL_ERROR
                "无法推断 CMAKE_PRESET：未知编译器 '${CMAKE_CXX_COMPILER_ID}'。"
                "请显式指定 -DCMAKE_PRESET=<preset>")
        endif()
    else()
        message(FATAL_ERROR
            "无法推断 CMAKE_PRESET：未知系统 '${CMAKE_SYSTEM_NAME}'。"
            "请显式指定 -DCMAKE_PRESET=<preset>")
    endif()

    string(TOLOWER "${RAW_CMAKE_PRESET}" RAW_CMAKE_PRESET)
    set(CMAKE_PRESET "${RAW_CMAKE_PRESET}")
    message(STATUS "推断 CMAKE_PRESET: ${CMAKE_PRESET}")
endif()