cmake_minimum_required(VERSION 3.4.1)
set(CMAKE_VERBOSE_MAKEFILE on)

project(webp)

include("../../../../../hltypes/android-studio/generic/cmake-preprocessors.txt")

# defining manually so source files don't need to be changed
file(
	GLOB CppSrc
	"../../src/cpu-features.c"
	"../../src/dec/*.c"
	"../../src/demux/*.c"
	"../../src/dsp/*.c"
	"../../src/util/*.c"
)

add_library(webp STATIC ${CppSrc})

add_definitions(
	-DWEBP_USE_THREAD
)

target_link_libraries(
	webp
)
