cmake_minimum_required(VERSION 3.20)
#project
project(QT_LYJ)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_BUILD_TYPE "Release")
if(WIN32)
    if(MSVC)
        set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} /Zi")
        # set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} /O2 /Ob2")
        set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} /Od /Ob0")
        set(CMAKE_EXE_LINKER_FLAGS_RELEASE "${CMAKE_EXE_LINKER_FLAGS_RELEASE} /DEBUG")
        add_link_options(/DEBUG) #生成调试信息（测试在visual studio中有效）
    endif()
elseif(LINUX)
    set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -g")
    set(CMAKE_C_FLAGS_RELEASE "${CMAKE_C_FLAGS_RELEASE} -g")
endif()

#cmake
if(EXISTS ${CMAKE_CURRENT_LIST_DIR}/../cmake/FuncLyj.cmake)
    include(${CMAKE_CURRENT_LIST_DIR}/../cmake/FuncLyj.cmake)
else()
    include(${CMAKE_CURRENT_LIST_DIR}/cmake/FuncLyj.cmake)
endif()
#package
find_package(Qt5 REQUIRED COMPONENTS Widgets Core Gui OpenGL)
find_package(Glfw)
include_directories(${GLFW_INCLUDE_DIRS})
find_package(Glew)
include_directories(${GLEW_INCLUDE_DIRS})
find_package(glm)
include_directories(${GLM_INCLUDE_DIRS})
#project sources
set(QT_LYJ_SRC_FILES)
GroupFiles(QT_LYJ_SRC_FILES ${CMAKE_CURRENT_LIST_DIR} ".")
add_library(${PROJECT_NAME} SHARED ${QT_LYJ_SRC_FILES})
#dependencies headers and libraries
include_directories(${GLFW_INCLUDE_DIRS})
target_include_directories(${PROJECT_NAME} PRIVATE ${CMAKE_CURRENT_LIST_DIR} ${GLEW_INCLUDE_DIRS} )
target_link_libraries(${PROJECT_NAME} Qt5::Widgets Qt5::Core Qt5::Gui Qt5::OpenGL opengl32 glu32 ${GLFW_LIBRARIES} ${GLEW_LIBRARIES} ${OpenCV_LIBS})
set_target_properties(${PROJECT_NAME} PROPERTIES
    AUTOMOC ON
    AUTOUIC ON
    AUTORCC ON
)

if(DEFINED LIBRARY_LYJ_OUTPUT_PATH)
    set_target_properties(${PROJECT_NAME} PROPERTIES
    LIBRARY_OUTPUT_DIRECTORY ${LIBRARY_LYJ_OUTPUT_PATH}
    ARCHIVE_OUTPUT_DIRECTORY ${LIBRARY_LYJ_OUTPUT_PATH}
    RUNTIME_OUTPUT_DIRECTORY ${LIBRARY_LYJ_OUTPUT_PATH}
    )
endif()

