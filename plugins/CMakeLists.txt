# plugins/CMakeLists.txt
set(PLUGIN_TARGETS "")

add_subdirectory(ExamplePlugin)
list(APPEND PLUGIN_TARGETS ExamplePlugin)

add_subdirectory(FileOperationsBtn)
list(APPEND PLUGIN_TARGETS FileOperationsBtn)

# Передаем список в родительский CMake
set(PLUGIN_TARGETS ${PLUGIN_TARGETS} PARENT_SCOPE)
