cmake_minimum_required(VERSION 3.16)
project(BelkinCommander LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

find_package(Qt6 REQUIRED COMPONENTS Core Widgets Gui)

# -----------------------------
# 1. Библиотека ядра BelkinCore
# -----------------------------
file(GLOB CORE_ICONS src/core/icons/*.png )

add_library(BelkinCore SHARED
    src/core/CopySignals.cpp
    src/core/FileOperations.cpp
    src/core/ApplicationAPI.h
    src/core/CopySignals.h
    src/core/FileOperations.h
    src/core/FilePluginInterface.h
    src/core/resources.qrc
    src/core/CopyWorkerCore.cpp
    src/core/CopyWorkerCore.h
    ${CORE_ICONS}
)

target_link_libraries(BelkinCore
    PUBLIC
        Qt6::Core
        Qt6::Widgets
)

target_include_directories(BelkinCore
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/src/core
        ${CMAKE_CURRENT_SOURCE_DIR}/src
)

# -----------------------------
# 2. Основное приложение
# -----------------------------
add_executable(BelkinCommander
    src/app/main.cpp
    src/app/MainWindow.cpp
    src/app/MainWindow.h
    src/app/FilePanel.cpp
    src/app/FilePanel.h
)

target_link_libraries(BelkinCommander
    PRIVATE
        BelkinCore
        Qt6::Core
        Qt6::Widgets
)

# -----------------------------
# 3. Плагины
# -----------------------------
add_subdirectory(src/plugins)


set(PLUGINS_OUTPUT_DIR "${CMAKE_BINARY_DIR}/plugins")
file(MAKE_DIRECTORY ${PLUGINS_OUTPUT_DIR})

foreach(plugin_target IN LISTS PLUGIN_TARGETS)
    add_custom_command(TARGET BelkinCommander POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
            $<TARGET_FILE:${plugin_target}>
            ${PLUGINS_OUTPUT_DIR}
    )
endforeach()
