# этот CMakeLists лежит в plugins/ExamplePlugin
cmake_minimum_required(VERSION 3.5)
project(ExamplePlugin)

set(CMAKE_CXX_STANDARD 20)
find_package(Qt6 REQUIRED COMPONENTS Core)

set(CMAKE_AUTOMOC ON)
include_directories(${CMAKE_SOURCE_DIR}/../..)  # путь к интерфейсу

add_library(ExamplePlugin SHARED
    examplePlugin.cpp
    examplePlugin.h
)

target_link_libraries(ExamplePlugin Qt6::Core Qt6::Widgets)

# Задаём префикс и суффикс для плагина, как требует Qt
set_target_properties(ExamplePlugin PROPERTIES
    PREFIX ""
    OUTPUT_NAME "exampleplugin"
   # SUFFIX ".dll"       # Windows
)
