# Очищаем список в текущей области видимости
set(LOCAL_PLUGINS "")
include_directories("${CMAKE_SOURCE_DIR}")
add_subdirectory(ExamplePlugin)
list(APPEND LOCAL_PLUGINS ExamplePlugin)

add_subdirectory(FileOperationsBtn)
list(APPEND LOCAL_PLUGINS FileOperationsBtn)

add_subdirectory(CopyPlugin)
list(APPEND LOCAL_PLUGINS CopyPlugin)

add_subdirectory(DuplicateFinder)
list(APPEND LOCAL_PLUGINS DuplicateFinder)

# Передаем итоговый список наверх в основной CMakeLists.txt
set(PLUGIN_TARGETS ${LOCAL_PLUGINS} PARENT_SCOPE)

