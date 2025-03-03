cmake_minimum_required(VERSION 3.20)
#project
project(QT_LYJ)
#cmake
if(EXISTS ${CMAKE_CURRENT_LIST_DIR}/../cmake/FuncLyj.cmake)
    include(${CMAKE_CURRENT_LIST_DIR}/../cmake/FuncLyj.cmake)
else()
    include(${CMAKE_CURRENT_LIST_DIR}/cmake/FuncLyj.cmake)
endif()
#package
find_package(Qt5 REQUIRED COMPONENTS Widgets Core Gui OpenGL)
set(CMAKE_AUTOMOC ON)  # 自动处理 moc 文件
#project sources
set(QT_LYJ_SRC_FILES)
GroupFiles(QT_LYJ_SRC_FILES ${CMAKE_CURRENT_LIST_DIR} ".")
add_library(${PROJECT_NAME} SHARED ${QT_LYJ_SRC_FILES})
#dependencies headers and libraries
target_include_directories(${PROJECT_NAME} PRIVATE ${CMAKE_CURRENT_LIST_DIR})
target_link_libraries(${PROJECT_NAME} Qt5::Widgets Qt5::Core Qt5::Gui Qt5::OpenGL opengl32 glu32)

