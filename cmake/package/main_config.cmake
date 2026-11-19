include(GNUInstallDirs)
include(CMakePackageConfigHelpers)

# 版本文件：SameMinorVersion 表示只有同一 major.minor 才视为兼容。
# 破坏性变更（如头文件路径调整）放在新 major（2.0 与 1.x 不兼容），
# 同一 major.minor 的补丁号之间保持兼容；新增接口的 minor 也要求下游显式升版本。
write_basic_package_version_file(
    "${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}ConfigVersion.cmake"
    VERSION ${PROJECT_VERSION}
    COMPATIBILITY SameMinorVersion
)

# 主Config模板
configure_package_config_file(
    "${CMAKE_CURRENT_LIST_DIR}/project_config.cmake.in"
    "${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}Config.cmake"
    INSTALL_DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/${PROJECT_NAME}
)

# 安装主配置文件
install(FILES
    "${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}Config.cmake"
    "${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}ConfigVersion.cmake"
    DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/${PROJECT_NAME}
    COMPONENT development
)

install(TARGETS ${LIB_LIST}
    EXPORT ${PROJECT_NAME}_EXPORT
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR} COMPONENT runtime
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR} COMPONENT runtime
            NAMELINK_COMPONENT development
    ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR} COMPONENT development
    INCLUDES DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
)

# 生成的 version.h 随组件头文件一起安装，保持 include 下的目录结构
install(FILES "${CMAKE_BINARY_DIR}/generated/origin/version.h"
    DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/origin
    COMPONENT development)

install(EXPORT ${PROJECT_NAME}_EXPORT
    FILE ${PROJECT_NAME}Target.cmake
    NAMESPACE ${PROJECT_NAME}::
    DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/${PROJECT_NAME}
    COMPONENT development
)
