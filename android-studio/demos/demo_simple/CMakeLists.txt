cmake_minimum_required(VERSION 3.4.1)
set(CMAKE_VERBOSE_MAKEFILE on)

project(demo_simple)

file(
	GLOB_RECURSE CppSrc
	"../../../demos/demo_simple/*.cpp"
)

add_library(demo_simple SHARED ${CppSrc})

include_directories(AFTER "../../../include")
include_directories(AFTER "../../../../hltypes/include")
include_directories(AFTER "../../../../gtypes/include")

add_library(april STATIC IMPORTED)
add_library(hltypes STATIC IMPORTED)
add_library(gtypes STATIC IMPORTED)
add_library(jpeg STATIC IMPORTED)
add_library(png STATIC IMPORTED)
set(root_april "../lib")
set(root_hltypes "../../../hltypes/android-studio/lib")
set(root_gtypes "../../../gtypes/android-studio/lib")
set(root_jpeg "../../lib/jpeg/android-studio/lib")
set(root_png "../../lib/png/android-studio/lib")
set_target_properties(april PROPERTIES IMPORTED_LOCATION ${root_april}/build/intermediates/cmake/debug/obj/${ANDROID_ABI}/libapril.a)
set_target_properties(hltypes PROPERTIES IMPORTED_LOCATION ${root_hltypes}/build/intermediates/cmake/debug/obj/${ANDROID_ABI}/libhltypes.a)
set_target_properties(gtypes PROPERTIES IMPORTED_LOCATION ${root_gtypes}/build/intermediates/cmake/debug/obj/${ANDROID_ABI}/libgtypes.a)
set_target_properties(jpeg PROPERTIES IMPORTED_LOCATION ${root_jpeg}/build/intermediates/cmake/debug/obj/${ANDROID_ABI}/libjpeg.a)
set_target_properties(png PROPERTIES IMPORTED_LOCATION ${root_png}/build/intermediates/cmake/debug/obj/${ANDROID_ABI}/libpng.a)

find_library(liblog log)
find_library(libz z)
find_library(libc c)
find_library(libdl dl)
find_library(libandroid android)
find_library(libGLESv2 GLESv2)

target_link_libraries(
	demo_simple
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
