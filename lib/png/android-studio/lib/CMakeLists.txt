cmake_minimum_required(VERSION 3.4.1)
set(CMAKE_VERBOSE_MAKEFILE on)

project(png)

file(
	GLOB_RECURSE CppSrc
    "../../*.h"
    "../../*.c"
    "../../*.cpp"
)

add_library(
	png
	STATIC
	${CppSrc}
)

include_directories(AFTER "../..")

target_link_libraries(
	png
)
