cmake_minimum_required(VERSION 3.4.1)
set(CMAKE_VERBOSE_MAKEFILE on)

project(demo_pix)

include("../../../../hltypes/android-studio/generic/CMakeLists.txt")

file(
	GLOB_RECURSE CppSrc
	"../../../demos/demo_pix/*.cpp"
)

add_library(demo_pix SHARED ${CppSrc})

include_directories(AFTER "../../../aprilpix/include")
include_directories(AFTER "../../../include")
include_directories(AFTER "../../../../hltypes/include")
include_directories(AFTER "../../../../gtypes/include")

add_subdirectory("../../../aprilpix/android-studio/lib" aprilpix)
add_subdirectory("../../lib" april)
add_subdirectory("../../../../hltypes/android-studio/lib" hltypes)
add_subdirectory("../../../../gtypes/android-studio/lib" gtypes)
add_subdirectory("../../../lib/jpeg/android-studio/lib" jpeg)
add_subdirectory("../../../lib/png/android-studio/lib" png)
add_subdirectory("../../../lib/webp/android-studio/lib" webp)

target_link_libraries(
	demo_pix
	aprilpix
	april
	hltypes
	gtypes
	jpeg
	png
	webp
	${liblog}
	${libz}
	${libc}
	${libdl}
	${libandroid}
	${libGLESv2}
)
