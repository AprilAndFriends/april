cmake_minimum_required(VERSION 3.4.1)
set(CMAKE_VERBOSE_MAKEFILE on)

project(demo_tictactoe)

include("../../../../hltypes/android-studio/generic/CMakeLists.txt")

file(
	GLOB_RECURSE CppSrc
	"../../../demos/demo_tictactoe/*.cpp"
)

add_library(demo_tictactoe SHARED ${CppSrc})

include_directories(AFTER "../../../../hltypes/include")
include_directories(AFTER "../../../../gtypes/include")

add_subdirectory("../../lib" april)
add_subdirectory("../../../../hltypes/android-studio/lib" hltypes)
add_subdirectory("../../../../gtypes/android-studio/lib" gtypes)
add_subdirectory("../../../lib/jpeg/android-studio/lib" jpeg)
add_subdirectory("../../../lib/png/android-studio/lib" png)

target_link_libraries(
	demo_tictactoe
	april
	hltypes
	gtypes
	jpeg
	png
	${SYSTEM_LIBS}
)
