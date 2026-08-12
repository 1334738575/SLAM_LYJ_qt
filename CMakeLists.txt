cmake_minimum_required(VERSION 3.20)

# ################################setting##################################
# cmake setting
project(QT_LYJ)
if(TARGET QT_LYJ)
    message(STATUS "目标 QT_LYJ 已存在")
    return()
endif()
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_BUILD_TYPE "Release")

if(WIN32)
    if(MSVC)
        set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} /Zi")

        # set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} /O2 /Ob2")
        set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} /Od /Ob0")

        set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} /Zi")

        # set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} /O2 /Ob2")
        set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} /Od /Ob0")

        # set(CMAKE_EXE_LINKER_FLAGS_RELEASE "${CMAKE_EXE_LINKER_FLAGS_RELEASE} /DEBUG")
        add_link_options(/DEBUG) # 生成调试信息（测试在visual studio中有效）
    endif()
elseif(LINUX)
    set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -g")
    set(CMAKE_C_FLAGS_RELEASE "${CMAKE_C_FLAGS_RELEASE} -g")

    set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -g")
    set(CMAKE_C_FLAGS_DEBUG "${CMAKE_C_FLAGS_DEBUG} -g")
endif()

if(MSVC)
    set_property(GLOBAL PROPERTY USE_FOLDERS ON)
endif()

set(${PROJECT_NAME}_LIBRARY_OUTPUT_PATH ${CMAKE_SOURCE_DIR}/Output)
# set(${PROJECT_NAME}_CMAKE_FILE)
set(CMAKE_INSTALL_PREFIX ${CMAKE_SOURCE_DIR}/install)

# ################################cmake##################################
if(DEFINED ${PROJECT_NAME}_CMAKE_FILE)
    include(${${PROJECT_NAME}_CMAKE_FILE})
elseif(EXISTS ${CMAKE_CURRENT_LIST_DIR}/../cmake/FuncLyj.cmake)
    include(${CMAKE_CURRENT_LIST_DIR}/../cmake/FuncLyj.cmake)
else()
    include(${CMAKE_CURRENT_LIST_DIR}/cmake/FuncLyj.cmake)
endif()

# ################################package##################################
set(${PROJECT_NAME}_INCLUDE_DIRS)
set(${PROJECT_NAME}_LIBRARIES)
set(${PROJECT_NAME}_BINARY_DIRS)
find_package(Qt5 REQUIRED COMPONENTS Widgets Core Gui OpenGL)
option(QT_LYJ_ENABLE_CUDA "Enable the CUDA mesh projector backend" ON)
option(QT_LYJ_ENABLE_VULKAN "Enable the Vulkan mesh projector backend" ON)
# message(FATAL_ERROR ${QT5_DIR})
list(APPEND CMAKE_MODULE_PATH "D:/SLAM_LYJ/dependencies/Glfw")
find_package(Glfw)
include_directories(${GLFW_INCLUDE_DIRS})
list(APPEND CMAKE_MODULE_PATH "D:/SLAM_LYJ/dependencies/Glew")
find_package(Glew)
include_directories(${GLEW_INCLUDE_DIRS})
# find_package(glm)
# include_directories(${GLM_INCLUDE_DIRS})
set(OpenCV_DIR "D:/SLAM_LYJ/dependencies/Opencv/lib")
find_package(OpenCV REQUIRED)
set(Eigen3_DIR "D:/SLAM_LYJ/dependencies/Eigen/share/eigen3/cmake")
find_package(Eigen3)
if(TARGET sqlite3)
    message("sqlite3 has found!")
else()
    list(APPEND CMAKE_MODULE_PATH "D:/SLAM_LYJ/dependencies/sqlite3")
    find_package(sqlite3)
endif()
list(APPEND ${PROJECT_NAME}_LIBRARIES sqlite3::sqlite3)
list(APPEND ${PROJECT_NAME}_BINARY_DIRS "${sqlite3_BINARY_DIR}\;")
list(APPEND ${PROJECT_NAME}_INCLUDE_DIRS ${GLFW_INCLUDE_DIRS})
list(APPEND ${PROJECT_NAME}_INCLUDE_DIRS ${EIGEN3_INCLUDE_DIRS})
list(APPEND ${PROJECT_NAME}_INCLUDE_DIRS ${GLEW_INCLUDE_DIRS})
list(APPEND ${PROJECT_NAME}_LIBRARIES
 Qt5::Widgets Qt5::Core Qt5::Gui Qt5::OpenGL
 opengl32 
 glu32 
 ${GLFW_LIBRARIES} 
 ${GLEW_LIBRARIES} 
 ${OpenCV_LIBS}
)
if(QT_LYJ_ENABLE_CUDA)
    enable_language(CUDA)
    find_package(CUDA REQUIRED)
    include_directories(${CUDA_INCLUDE_DIRS})
    set(CMAKE_CUDA_ARCHITECTURES 89)
    list(APPEND ${PROJECT_NAME}_INCLUDE_DIRS ${CUDA_INCLUDE_DIRS})
    list(APPEND ${PROJECT_NAME}_LIBRARIES ${CUDA_LIBRARIES})
endif()

if(TARGET COMMON_LYJ)
    list(APPEND ${PROJECT_NAME}_LIBRARIES COMMON_LYJ)
else()
    set(COMMON_LYJ_DIR "D:/SLAM_LYJ_Packages/SLAM_LYJ_COMMON/install/lib/cmake")
    find_package(COMMON_LYJ)
    list(APPEND ${PROJECT_NAME}_LIBRARIES COMMON_LYJ::COMMON_LYJ)
    list(APPEND ${PROJECT_NAME}_BINARY_DIRS "${COMMON_LYJ_DIR}/../../bin;")
endif()
if(QT_LYJ_ENABLE_CUDA)
    if(TARGET CUDA_LYJ)
        set(QT_LYJ_CUDA_TARGET CUDA_LYJ)
    elseif(TARGET CUDA_LYJ::CUDA_LYJ)
        set(QT_LYJ_CUDA_TARGET CUDA_LYJ::CUDA_LYJ)
    else()
        set(CUDA_LYJ_DIR "D:/SLAM_LYJ_Packages/SLAM_LYJ_CUDA/install/lib/cmake" CACHE PATH
            "Directory containing CUDA_LYJConfig.cmake")
        find_package(CUDA_LYJ CONFIG REQUIRED)
        set(QT_LYJ_CUDA_TARGET CUDA_LYJ::CUDA_LYJ)
        list(APPEND ${PROJECT_NAME}_INCLUDE_DIRS ${CUDA_LYJ_DIR}/../../include)
        list(APPEND ${PROJECT_NAME}_BINARY_DIRS "${CUDA_LYJ_DIR}/../../bin\;")
    endif()
    get_target_property(QT_LYJ_CUDA_IMPORTED ${QT_LYJ_CUDA_TARGET} IMPORTED)
    get_target_property(QT_LYJ_CUDA_SOURCE_DIR ${QT_LYJ_CUDA_TARGET} SOURCE_DIR)
    if(NOT QT_LYJ_CUDA_IMPORTED AND QT_LYJ_CUDA_SOURCE_DIR)
        set(QT_LYJ_CUDA_DEFINES "${QT_LYJ_CUDA_SOURCE_DIR}/CUDADefines.h")
    elseif(CUDA_LYJ_DIR)
        get_filename_component(QT_LYJ_CUDA_DEFINES "${CUDA_LYJ_DIR}/../../include/CUDADefines.h" ABSOLUTE)
    endif()
    if(QT_LYJ_CUDA_DEFINES AND EXISTS "${QT_LYJ_CUDA_DEFINES}")
        file(READ "${QT_LYJ_CUDA_DEFINES}" QT_LYJ_CUDA_DEFINES_CONTENT)
        if(NOT QT_LYJ_CUDA_DEFINES_CONTENT MATCHES "Fisheye[ \t\r\n]*=[ \t\r\n]*1")
            message(FATAL_ERROR "CUDA_LYJ is too old: fisheye CameraModel support is required")
        endif()
    endif()
    list(APPEND ${PROJECT_NAME}_LIBRARIES ${QT_LYJ_CUDA_TARGET})
endif()

if(QT_LYJ_ENABLE_VULKAN)
    find_package(Vulkan REQUIRED)
    if(TARGET Vulkan_LYJ)
        set(QT_LYJ_VULKAN_TARGET Vulkan_LYJ)
    elseif(TARGET Vulkan_LYJ::Vulkan_LYJ)
        set(QT_LYJ_VULKAN_TARGET Vulkan_LYJ::Vulkan_LYJ)
    else()
        find_package(Vulkan_LYJ CONFIG QUIET)
        if(TARGET Vulkan_LYJ::Vulkan_LYJ)
            set(QT_LYJ_VULKAN_TARGET Vulkan_LYJ::Vulkan_LYJ)
        endif()
    endif()
    if(QT_LYJ_VULKAN_TARGET)
        get_target_property(QT_LYJ_VULKAN_IMPORTED ${QT_LYJ_VULKAN_TARGET} IMPORTED)
        get_target_property(QT_LYJ_VULKAN_SOURCE_DIR ${QT_LYJ_VULKAN_TARGET} SOURCE_DIR)
        if(NOT QT_LYJ_VULKAN_IMPORTED AND QT_LYJ_VULKAN_SOURCE_DIR)
            set(QT_LYJ_VULKAN_DEFINES "${QT_LYJ_VULKAN_SOURCE_DIR}/VulkanDefines.h")
        elseif(Vulkan_LYJ_DIR)
            get_filename_component(QT_LYJ_VULKAN_DEFINES "${Vulkan_LYJ_DIR}/../../include/VulkanDefines.h" ABSOLUTE)
        endif()
        if(QT_LYJ_VULKAN_DEFINES AND EXISTS "${QT_LYJ_VULKAN_DEFINES}")
            file(READ "${QT_LYJ_VULKAN_DEFINES}" QT_LYJ_VULKAN_DEFINES_CONTENT)
            if(NOT QT_LYJ_VULKAN_DEFINES_CONTENT MATCHES "Fisheye[ \t\r\n]*=[ \t\r\n]*1")
                message(FATAL_ERROR "Vulkan_LYJ is too old: fisheye CameraModel support is required")
            endif()
        endif()
        list(APPEND ${PROJECT_NAME}_LIBRARIES ${QT_LYJ_VULKAN_TARGET})
        list(APPEND ${PROJECT_NAME}_LIBRARIES Vulkan::Vulkan)
        if(TARGET glm::glm)
            list(APPEND ${PROJECT_NAME}_LIBRARIES glm::glm)
        endif()
        if(TARGET glfw)
            list(APPEND ${PROJECT_NAME}_LIBRARIES glfw)
        endif()
        if(NOT QT_LYJ_VULKAN_IMPORTED AND QT_LYJ_VULKAN_SOURCE_DIR)
            get_target_property(QT_LYJ_VULKAN_BINARY_DIR ${QT_LYJ_VULKAN_TARGET} BINARY_DIR)
            if(EXISTS "${QT_LYJ_VULKAN_BINARY_DIR}/_deps/glfw-src/include/GLFW/glfw3.h")
                list(APPEND ${PROJECT_NAME}_INCLUDE_DIRS "${QT_LYJ_VULKAN_BINARY_DIR}/_deps/glfw-src/include")
            endif()
            if(EXISTS "${QT_LYJ_VULKAN_BINARY_DIR}/_deps/glm-src/glm/glm.hpp")
                list(APPEND ${PROJECT_NAME}_INCLUDE_DIRS "${QT_LYJ_VULKAN_BINARY_DIR}/_deps/glm-src")
            endif()
        endif()
        list(APPEND ${PROJECT_NAME}_BINARY_DIRS "$<TARGET_FILE_DIR:${QT_LYJ_VULKAN_TARGET}>\;")
    else()
        message(WARNING "QT_LYJ_ENABLE_VULKAN is ON, but Vulkan_LYJ was not found; Vulkan support is disabled")
    endif()
endif()

list(APPEND ${PROJECT_NAME}_BINARY_DIRS "D:/SLAM_LYJ/dependencies/Glfw/Dll;")
list(APPEND ${PROJECT_NAME}_BINARY_DIRS "D:/SLAM_LYJ/dependencies/Glew/bin/Release/x64;")
list(APPEND ${PROJECT_NAME}_BINARY_DIRS "E:/anaconda3/pkgs/qt-main-5.15.2-h19c9488_10/Library/bin;")
list(APPEND ${PROJECT_NAME}_BINARY_DIRS "${OpenCV_INSTALL_PATH}/bin;")
list(APPEND ${PROJECT_NAME}_BINARY_DIRS "D:/SLAM_LYJ/dependencies/libpng/bin;")
list(APPEND ${PROJECT_NAME}_BINARY_DIRS "D:/SLAM_LYJ/dependencies/zlib/bin;")
list(APPEND ${PROJECT_NAME}_BINARY_DIRS "E:/anaconda3/pkgs/icu-73.1-h6c2663c_0/Library/bin;")
list(APPEND ${PROJECT_NAME}_BINARY_DIRS "E:/anaconda3/pkgs/zstd-1.5.5-hd43e919_2/Library/bin;")
list(APPEND ${PROJECT_NAME}_BINARY_DIRS "D:/SLAM_LYJ/dependencies/flann/bin\;")
list(APPEND ${PROJECT_NAME}_BINARY_DIRS "D:/SLAM_LYJ/dependencies/lz4/bin\;")
list(APPEND ${PROJECT_NAME}_BINARY_DIRS "D:/SLAM_LYJ/dependencies/libjpeg/bin\;")
list(APPEND ${PROJECT_NAME}_BINARY_DIRS "D:/SLAM_LYJ/dependencies/sqlite3/bin\;")
message(STATUS ${${PROJECT_NAME}_BINARY_DIRS})

# list(APPEND ${PROJECT_NAME}_BINARY_DIRS )
include_directories(${${PROJECT_NAME}_INCLUDE_DIRS})

# ################################code##################################
set(${PROJECT_NAME}_SRC_FILES)
GroupFiles(${PROJECT_NAME}_SRC_FILES ${CMAKE_CURRENT_LIST_DIR} ".")
list(FILTER ${PROJECT_NAME}_SRC_FILES EXCLUDE REGEX "[/\\]tests[/\\]")

# ################################library##################################
add_library(${PROJECT_NAME} SHARED ${${PROJECT_NAME}_SRC_FILES})
target_include_directories(${PROJECT_NAME} PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
    $<INSTALL_INTERFACE:include>
)
target_include_directories(${PROJECT_NAME} PRIVATE
    $<BUILD_INTERFACE:${${PROJECT_NAME}_INCLUDE_DIRS}>
)
target_link_libraries(${PROJECT_NAME} PRIVATE
    ${${PROJECT_NAME}_LIBRARIES}
)
target_compile_definitions(${PROJECT_NAME} PRIVATE ${PROJECT_NAME}_API_EXPORTS) # TODO
if(QT_LYJ_CUDA_TARGET)
    target_compile_definitions(${PROJECT_NAME} PRIVATE QT_LYJ_WITH_CUDA=1)
endif()
if(QT_LYJ_VULKAN_TARGET)
    target_compile_definitions(${PROJECT_NAME} PRIVATE QT_LYJ_WITH_VULKAN=1)
endif()
set_target_properties(${PROJECT_NAME} PROPERTIES
    AUTOMOC ON
    AUTOUIC ON
    AUTORCC ON
)

# ################################output##################################
if(DEFINED ${PROJECT_NAME}_LIBRARY_OUTPUT_PATH)
    set_target_properties(${PROJECT_NAME} PROPERTIES
        LIBRARY_OUTPUT_DIRECTORY ${${PROJECT_NAME}_LIBRARY_OUTPUT_PATH}
        ARCHIVE_OUTPUT_DIRECTORY ${${PROJECT_NAME}_LIBRARY_OUTPUT_PATH}
        RUNTIME_OUTPUT_DIRECTORY ${${PROJECT_NAME}_LIBRARY_OUTPUT_PATH}
    )
    target_compile_definitions(${PROJECT_NAME} PRIVATE SHADERPATH="${CMAKE_CURRENT_SOURCE_DIR}/shaders")
endif()

# ################################example##################################
add_executable(${PROJECT_NAME}Bin example/main.cpp)
target_include_directories(${PROJECT_NAME}Bin PRIVATE ${CMAKE_CURRENT_SOURCE_DIR} ${${PROJECT_NAME}_INCLUDE_DIRS})
target_link_libraries(${PROJECT_NAME}Bin PRIVATE ${PROJECT_NAME} ${${PROJECT_NAME}_LIBRARIES})

if(DEFINED ${PROJECT_NAME}_LIBRARY_OUTPUT_PATH)
    set_target_properties(${PROJECT_NAME}Bin PROPERTIES
        LIBRARY_OUTPUT_DIRECTORY ${${PROJECT_NAME}_LIBRARY_OUTPUT_PATH}
        ARCHIVE_OUTPUT_DIRECTORY ${${PROJECT_NAME}_LIBRARY_OUTPUT_PATH}
        RUNTIME_OUTPUT_DIRECTORY ${${PROJECT_NAME}_LIBRARY_OUTPUT_PATH}
    )
endif()

include(CTest)
if(BUILD_TESTING)
    add_executable(${PROJECT_NAME}ProjectorSmoke tests/ProjectorSmoke.cpp)
    target_include_directories(${PROJECT_NAME}ProjectorSmoke PRIVATE ${CMAKE_CURRENT_SOURCE_DIR} ${${PROJECT_NAME}_INCLUDE_DIRS})
    target_link_libraries(${PROJECT_NAME}ProjectorSmoke PRIVATE ${PROJECT_NAME} ${${PROJECT_NAME}_LIBRARIES})
    if(QT_LYJ_CUDA_TARGET)
        add_test(NAME ${PROJECT_NAME}ProjectorSmokeCUDA COMMAND ${PROJECT_NAME}ProjectorSmoke cuda)
    endif()
    if(QT_LYJ_VULKAN_TARGET)
        add_test(NAME ${PROJECT_NAME}ProjectorSmokeVulkan COMMAND ${PROJECT_NAME}ProjectorSmoke vulkan)
    endif()
    if(DEFINED ${PROJECT_NAME}_LIBRARY_OUTPUT_PATH)
        set_target_properties(${PROJECT_NAME}ProjectorSmoke PROPERTIES
            RUNTIME_OUTPUT_DIRECTORY ${${PROJECT_NAME}_LIBRARY_OUTPUT_PATH}
        )
    endif()
endif()

if(MSVC)
    set_target_properties(${PROJECT_NAME}Bin PROPERTIES VS_DEBUGGER_ENVIRONMENT "PATH=${${PROJECT_NAME}_BINARY_DIRS}")
endif()

# ################################install##################################
file(WRITE ${CMAKE_CURRENT_SOURCE_DIR}/Config.cmake.in
    "@PACKAGE_INIT@\ninclude(\"\${CMAKE_CURRENT_LIST_DIR}/${PROJECT_NAME}Targets.cmake\" )"
)

# 安装库和头文件
install(TARGETS ${PROJECT_NAME}
    EXPORT ${PROJECT_NAME}Targets
    ARCHIVE DESTINATION lib
    LIBRARY DESTINATION lib # DLL 在 Windows 下会被安装到 bin/
    RUNTIME DESTINATION bin # 适用于 DLL 的运行时组件
    INCLUDES DESTINATION include
)

# #file
# install(FILES test.h
# DESTINATION include
# PERMISSIONS OWNER_READ GROUP_READ WORLD_READ)
# #directory
# install(DIRECTORY ./          # 注意末尾的斜杠
# DESTINATION include
# FILES_MATCHING PATTERN "*.h" # 仅匹配 .h 文件
# PERMISSIONS OWNER_READ GROUP_READ WORLD_READ
# )
# install(DIRECTORY ./
# DESTINATION include
# FILES_MATCHING
# PATTERN "*.h"            # 包含所有 .h 文件
# PATTERN "build/*"             # 匹配子目录名
# EXCLUDE                  # 排除该目录及其内容
# PATTERN "cmake/*"             # 匹配子目录名
# EXCLUDE                  # 排除该目录及其内容
# PATTERN "install/*"             # 匹配子目录名
# EXCLUDE                  # 排除该目录及其内容
# PATTERN "Lib/*"             # 匹配子目录名
# EXCLUDE                  # 排除该目录及其内容
# PERMISSIONS
# OWNER_READ GROUP_READ WORLD_READ
# )
install(DIRECTORY ./
    DESTINATION include
    FILES_MATCHING
    PATTERN "*.h" # 包含所有 .h 文件

    # 排除指定目录（递归匹配）
    PATTERN "build" EXCLUDE # 排除所有名为 build 的目录及内容
    PATTERN "cmake" EXCLUDE # 排除所有名为 cmake 的目录及内容
    PATTERN "install" EXCLUDE # 排除所有名为 install 的目录及内容
    PATTERN "Lib" EXCLUDE # 排除所有名为 Lib 的目录及内容

    # 或使用正则表达式精确匹配路径（更严格）
    # PATTERN "/(build|cmake|install|Lib)(/.*)?$" EXCLUDE
    # 设置权限
    PERMISSIONS
    OWNER_READ GROUP_READ WORLD_READ
)

# 安装后删除空目录（可选）
install(SCRIPT "${CMAKE_CURRENT_BINARY_DIR}/cleanup.cmake")
file(WRITE ${CMAKE_CURRENT_BINARY_DIR}/cleanup.cmake [[
file(REMOVE_RECURSE "${CMAKE_INSTALL_PREFIX}/include/build")
file(REMOVE_RECURSE "${CMAKE_INSTALL_PREFIX}/include/cmake")
file(REMOVE_RECURSE "${CMAKE_INSTALL_PREFIX}/include/install")
file(REMOVE_RECURSE "${CMAKE_INSTALL_PREFIX}/include/Lib")
]])

# set(TARGETNAME ${PROJECT_NAME}Targets)
# 导出目标配置
install(EXPORT ${PROJECT_NAME}Targets
    FILE ${PROJECT_NAME}Targets.cmake
    NAMESPACE ${PROJECT_NAME}::
    DESTINATION lib/cmake
)

# 生成包配置文件
include(CMakePackageConfigHelpers)
configure_package_config_file(
    ${CMAKE_CURRENT_SOURCE_DIR}/Config.cmake.in
    ${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}Config.cmake
    INSTALL_DESTINATION lib/cmake
)
write_basic_package_version_file(
    ${PROJECT_NAME}ConfigVersion.cmake
    VERSION 1.0.0
    COMPATIBILITY SameMajorVersion
)
install(FILES
    ${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}Config.cmake
    ${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}ConfigVersion.cmake
    DESTINATION lib/cmake
)

