cmake_minimum_required(VERSION 3.16)
project(BelkinCommander LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/personal.cmake")
    include("${CMAKE_CURRENT_SOURCE_DIR}/personal.cmake")
endif()

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
    src/core/FilePluginInterface.cpp
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

target_compile_definitions(BelkinCore PRIVATE BelkinCore_EXPORTS)

# Скрывать все символы по умолчанию (аналог поведения Windows)
set_target_properties(BelkinCore PROPERTIES
    CXX_VISIBILITY_PRESET hidden
    VISIBILITY_INLINES_HIDDEN ON
)

# -----------------------------
# 2. Основное приложение
# -----------------------------
add_executable(BelkinCommander WIN32
    src/app/main.cpp
    src/app/MainWindow.cpp
    src/app/MainWindow.h
    src/app/FilePanel.cpp
    src/app/FilePanel.h
    src/app/FileView.hpp
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

# Настройки для создания пакета
# --- Правила установки (cmake --install) ---

# 1. Библиотеки и исполняемые файлы
if(WIN32)
    install(TARGETS BelkinCommander BelkinCore
        RUNTIME DESTINATION bin
    )
else()
    # На Linux исполняемый файл в bin, библиотеку ядра в lib
    install(TARGETS BelkinCommander RUNTIME DESTINATION bin)
    install(TARGETS BelkinCore LIBRARY DESTINATION lib)
    set_target_properties(BelkinCommander PROPERTIES INSTALL_RPATH "$ORIGIN/../lib")
endif()

# 2. Плагины (устанавливаем из списка PLUGIN_TARGETS)
foreach(plugin_target IN LISTS PLUGIN_TARGETS)
    install(TARGETS ${plugin_target}
        LIBRARY DESTINATION bin/plugins
        RUNTIME DESTINATION bin/plugins
    )
endforeach()

# 3. Настройки CPack
set(CPACK_PACKAGE_NAME "BelkinCommander")
set(CPACK_PACKAGE_VERSION "0.0.${PATCH_VERSION}")
set(CPACK_DEBIAN_PACKAGE_MAINTAINER "Aleksei Belkin")

if(WIN32)
    set(CPACK_GENERATOR "ZIP") # Или NSIS для установщика
else()
    set(CPACK_GENERATOR "DEB")
    set(CPACK_DEBIAN_PACKAGE_SHLIBDEPS ON)
endif()

include(CPack)

# РАЗВЕРТЫВАНИЕ (DEPLOY) ДЛЯ WINDOWS
if(WIN32)
    find_package(Qt6 COMPONENTS Core REQUIRED)
    get_target_property(_qmake_executable Qt6::qmake IMPORTED_LOCATION)
    get_filename_component(_qt_bin_dir "${_qmake_executable}" DIRECTORY)
    find_program(WINDEPLOYQT_EXECUTABLE windeployqt HINTS "${_qt_bin_dir}")

    # Запуск для основного EXE
    add_custom_command(TARGET BelkinCommander POST_BUILD
        COMMAND "${WINDEPLOYQT_EXECUTABLE}"
                --no-compiler-runtime
                --dir "$<TARGET_FILE_DIR:BelkinCommander>"
                "$<TARGET_FILE:BelkinCommander>"
        COMMENT "Running windeployqt for main application..."
    )
endif()


