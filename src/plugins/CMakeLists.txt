# Очищаем список в текущей области видимости
set(LOCAL_PLUGINS "")
include_directories("${CMAKE_SOURCE_DIR}")
add_subdirectory(ExamplePlugin)
list(APPEND LOCAL_PLUGINS ExamplePlugin)

add_subdirectory(FileOperationsBtn)
list(APPEND LOCAL_PLUGINS FileOperationsBtn)

add_subdirectory(CopyPlugin)
list(APPEND LOCAL_PLUGINS CopyPlugin)

find_package(Boost CONFIG COMPONENTS filesystem system)

if(Boost_FOUND)
    message(STATUS "Boost found — enabling DuplicateFinder plugin")
    add_subdirectory(DuplicateFinder)
    list(APPEND LOCAL_PLUGINS DuplicateFinder)
else()
    message(STATUS "Boost NOT found — DuplicateFinder plugin disabled")
endif()

# Передаем итоговый список наверх в основной CMakeLists.txt
set(PLUGIN_TARGETS ${LOCAL_PLUGINS} PARENT_SCOPE)

