# Necessary cmakes
cmake_minimum_required(VERSION 3.5)

set(EXTRA_COMPONENT_DIRS /components)
add_compile_options(-fdiagnostics-color=always)
include($ENV{IDF_PATH}/tools/cmake/project.cmake)
project(stream)
