cmake_minimum_required(VERSION 3.5)
project(FileManagerQt)

set(CMAKE_CXX_STANDARD 17)
find_package(Qt5 REQUIRED COMPONENTS Widgets)

# Включаем automoc, указываем, где лежит интерфейс
set(CMAKE_AUTOMOC ON)
include_directories(${CMAKE_CURRENT_SOURCE_DIR})

add_executable(FileManagerQt
    main.cpp
    mainwindow.cpp
    mainwindow.h
    FilePluginInterface.h
)

target_link_libraries(FileManagerQt Qt5::Widgets)

# Добавляем подпроект с плагином
add_subdirectory(plugins/ExamplePlugin)

# Создадим папку plugins в выходном билде, если её нет
set(PLUGINS_OUTPUT_DIR "${CMAKE_BINARY_DIR}/plugins")
file(MAKE_DIRECTORY ${PLUGINS_OUTPUT_DIR})

# После сборки основного exe — копируем плагины
add_custom_command(TARGET FileManagerQt POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
        $<TARGET_FILE:ExamplePlugin>
        ${PLUGINS_OUTPUT_DIR}
)
