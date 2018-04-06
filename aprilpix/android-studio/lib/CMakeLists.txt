cmake_minimum_required(VERSION 3.4.1)
set(CMAKE_VERBOSE_MAKEFILE on)

project(aprilpix)

include("../../../../hltypes/android-studio/generic/cmake-preprocessors.txt")

file(
	GLOB_RECURSE CppSrc
	"../../src/*.c"
	"../../src/*.cpp"
)

add_library(aprilpix STATIC ${CppSrc})

include_directories(AFTER "../../../../hltypes/include")
include_directories(AFTER "../../../../gtypes/include")
include_directories(AFTER "../../../include")
include_directories(AFTER "../../include")
include_directories(AFTER "../../src")

add_definitions(
	-DAPRILPIX_EXPORTS
)

target_link_libraries(
	aprilpix
)
