cmake_minimum_required(VERSION 3.4.1)
set(CMAKE_VERBOSE_MAKEFILE on)

project(jpeg)

file(
	GLOB_RECURSE CppSrc
    "../../*.h"
    "../../*.c"
    "../../*.cpp"
)

add_library(
	jpeg
	STATIC
	${CppSrc}
)

include_directories(AFTER "../..")

target_link_libraries(
	jpeg
)
