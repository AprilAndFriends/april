cmake_minimum_required(VERSION 3.4.1)
set(CMAKE_VERBOSE_MAKEFILE on)

project(demo_helloworld)

include("../../../../hltypes/android-studio/generic/cmake-preprocessors.txt")

file(
	GLOB_RECURSE CppSrc
	"../../../demos/demo_helloworld/*.cpp"
)

add_library(demo_helloworld SHARED ${CppSrc})

include_directories(AFTER "../../../include")
include_directories(AFTER "../../../../hltypes/include")
include_directories(AFTER "../../../../gtypes/include")

add_subdirectory("../../../../hltypes/android-studio/lib" hltypes)
add_subdirectory("../../../../gtypes/android-studio/lib" gtypes)
add_subdirectory("../../../lib/jpeg/android-studio/lib" jpeg)
add_subdirectory("../../../lib/png/android-studio/lib" png)
add_subdirectory("../../lib" april)

find_library(liblog log)
find_library(libz z)
find_library(libc c)
find_library(libdl dl)
find_library(libandroid android)
find_library(libGLESv2 GLESv2)

target_link_libraries(
	demo_helloworld
	april
	hltypes
	gtypes
	jpeg
	png
	${liblog}
	${libz}
	${libc}
	${libdl}
	${libandroid}
	${libGLESv2}
)
