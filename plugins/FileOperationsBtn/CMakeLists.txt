# этот CMakeLists лежит в plugins/FileOperationsBtn
cmake_minimum_required(VERSION 3.5)
project(FileOperationsBtn)

set(CMAKE_CXX_STANDARD 20)
find_package(Qt6 REQUIRED COMPONENTS Core)

set(CMAKE_AUTOMOC ON)
include_directories(${CMAKE_SOURCE_DIR}/../..)  # путь к интерфейсу

add_library(FileOperationsBtn SHARED
    FileOperationsBtn.cpp
    FileOperationsBtn.h
    FileOperations.h
    FileOperations.cpp
)

target_link_libraries(FileOperationsBtn Qt6::Core Qt6::Widgets)

# Задаём префикс и суффикс для плагина, как требует Qt
set_target_properties(FileOperationsBtn PROPERTIES
    PREFIX ""
    OUTPUT_NAME "FileOperationsBtn"
   # SUFFIX ".dll"       # Windows
)
