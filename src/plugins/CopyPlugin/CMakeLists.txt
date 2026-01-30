# этот CMakeLists лежит в plugins/CopyPlugin
cmake_minimum_required(VERSION 3.5)
project(CopyPlugin)

set(CMAKE_CXX_STANDARD 20)
find_package(Qt6 REQUIRED COMPONENTS Core)

set(CMAKE_AUTOMOC ON)
include_directories(${CMAKE_SOURCE_DIR}/../..)  # путь к интерфейсу

add_library(CopyPlugin SHARED
    CopyPlugin.cpp
    CopyPlugin.h
    CopyProgressDialog.hpp
)

target_link_libraries(CopyPlugin
    PRIVATE
        BelkinCore
        Qt6::Core
        Qt6::Widgets
)

# Задаём префикс и суффикс для плагина, как требует Qt
set_target_properties(CopyPlugin PROPERTIES
    PREFIX ""
    OUTPUT_NAME "copyplugin"
   # SUFFIX ".dll"       # Windows
)

add_custom_command(TARGET CopyPlugin POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E make_directory
        "$<TARGET_FILE_DIR:BelkinCommander>/plugins"
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
        "$<TARGET_FILE:CopyPlugin>"
        "$<TARGET_FILE_DIR:BelkinCommander>/plugins/"
)