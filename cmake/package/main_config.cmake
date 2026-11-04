include(GNUInstallDirs)
include(CMakePackageConfigHelpers)

# 版本文件
write_basic_package_version_file(
    "${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}ConfigVersion.cmake"
    VERSION ${PROJECT_VERSION}
    COMPATIBILITY SameMajorVersion
)

# 主Config模板
configure_package_config_file(
    "${CMAKE_CURRENT_LIST_DIR}/project_config.cmake.in"
    "${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}Config.cmake"
    INSTALL_DESTINATION lib/cmake/${PROJECT_NAME}
)

# 安装主配置文件
install(FILES
    "${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}Config.cmake"
    "${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}ConfigVersion.cmake"
    DESTINATION lib/cmake/${PROJECT_NAME}
)

install(TARGETS ${LIB_LIST}
    EXPORT ${PROJECT_NAME}_EXPORT
    LIBRARY DESTINATION lib
    ARCHIVE DESTINATION lib
    RUNTIME DESTINATION bin
    INCLUDES DESTINATION include
)

# 生成的 version.h 随组件头文件一起安装，保持 include 下的目录结构
install(FILES "${CMAKE_BINARY_DIR}/generated/origin/version.h"
    DESTINATION include/origin)

install(EXPORT ${PROJECT_NAME}_EXPORT
    FILE ${PROJECT_NAME}Target.cmake
    NAMESPACE ${PROJECT_NAME}::
    DESTINATION lib/cmake/${PROJECT_NAME}
)
