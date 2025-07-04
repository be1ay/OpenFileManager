cmake_minimum_required(VERSION 3.5)
project(FileManagerQt)

set(CMAKE_CXX_STANDARD 11)

find_package(Qt5 REQUIRED COMPONENTS Widgets)

set(CMAKE_AUTOMOC ON)

add_executable(FileManagerQt
    main.cpp
    mainwindow.cpp
    mainwindow.h
)

target_link_libraries(FileManagerQt Qt5::Widgets)
