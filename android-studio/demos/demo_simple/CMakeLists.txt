cmake_minimum_required(VERSION 3.4.1)
set(CMAKE_VERBOSE_MAKEFILE on)

project(demo_simple)

file(
	GLOB_RECURSE CppSrc
    "../../../demos/demo_simple/*.cpp"
)

add_library(
	demo_simple
	SHARED
	${CppSrc}
)

include_directories(AFTER "../../../include")
include_directories(AFTER "../../../../hltypes/include")
include_directories(AFTER "../../../../gtypes/include")

find_library(llog log)
find_library(lz z)
find_library(lc c)
find_library(ldl dl)
find_library(landroid android)
find_library(lGLESv2 GLESv2)

target_link_libraries(
	demo_simple
	april
	hltypes
	gtypes
	jpeg
	png
	${llog}
	${lz}
	${lc}
	${ldl}
	${landroid}
	${lGLESv2}
)
