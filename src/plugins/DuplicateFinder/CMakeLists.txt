# этот CMakeLists лежит в plugins/DuplicateFinder
cmake_minimum_required(VERSION 3.5)
project(DuplicateFinder)

set(CMAKE_CXX_STANDARD 20)
find_package(Qt6 REQUIRED COMPONENTS Core)

set(CMAKE_AUTOMOC ON)
include_directories(${CMAKE_SOURCE_DIR}/../..)  # путь к интерфейсу

add_library(DuplicateFinder SHARED
    core/duplicate_finder.hpp
    core/duplicate_finder.cpp
    core/utils.cpp
    core/utils.hpp
    UI/DuplicateFinderDialog.hpp
    UI/DuplicateFinderDialog.cpp
    UI/DuplicateResultModel.hpp
    UI/DuplicateResultModel.cpp
    DuplicateFinderPlugin.hpp
    DuplicateFinderPlugin.cpp
)

target_link_libraries(DuplicateFinder
    PRIVATE
        BelkinCore
        Qt6::Core
        Qt6::Widgets
)

find_package(Boost REQUIRED COMPONENTS filesystem system)
target_link_libraries(DuplicateFinder PRIVATE Boost::filesystem Boost::system)


# Задаём префикс и суффикс для плагина, как требует Qt
set_target_properties(DuplicateFinder PROPERTIES
    PREFIX ""
    OUTPUT_NAME "duplicateFinder"
   # SUFFIX ".dll"       # Windows
)

target_include_directories(DuplicateFinder
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/core
        ${CMAKE_CURRENT_SOURCE_DIR}/UI
)

add_custom_command(TARGET DuplicateFinder POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E make_directory
        "$<TARGET_FILE_DIR:BelkinCommander>/plugins"
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
        "$<TARGET_FILE:DuplicateFinder>"
        "$<TARGET_FILE_DIR:BelkinCommander>/plugins/"
)
