cmake_minimum_required(VERSION 3.16)
project(BelkinCommander)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

find_package(Qt6 REQUIRED COMPONENTS Widgets)

add_executable(BelkinCommander
    main.cpp
    mainwindow.cpp
    mainwindow.h
    FilePluginInterface.h
    FilePanel.h
    FilePanel.cpp
	ApplicationAPI.h
    resources.qrc
)

target_include_directories(BelkinCommander PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})
target_link_libraries(BelkinCommander Qt6::Widgets)


add_subdirectory(plugins)

set(PLUGINS_OUTPUT_DIR "${CMAKE_BINARY_DIR}/plugins")
file(MAKE_DIRECTORY ${PLUGINS_OUTPUT_DIR})

foreach(plugin_target IN LISTS PLUGIN_TARGETS)
    add_custom_command(TARGET BelkinCommander POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
            $<TARGET_FILE:${plugin_target}>
            ${PLUGINS_OUTPUT_DIR}
    )
endforeach()
