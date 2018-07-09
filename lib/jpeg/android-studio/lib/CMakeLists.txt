cmake_minimum_required(VERSION 3.4.1)
set(CMAKE_VERBOSE_MAKEFILE on)

project(jpeg)

include("../../../../../hltypes/android-studio/generic/CMakeLists.txt")

file(
	GLOB_RECURSE CppSrc
	"../../j*.c"
	"../../j*.cpp"
)

add_library(jpeg STATIC ${CppSrc})

target_link_libraries(
	jpeg
)
