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

set(LIB_PATH ".externalNativeBuild/cmake/${CMAKE_BUILD_TYPE}/${ANDROID_ABI}")
#add_library(april STATIC IMPORTED)
#add_library(hltypes STATIC IMPORTED)
#add_library(gtypes STATIC IMPORTED)
#add_library(jpeg STATIC IMPORTED)
#add_library(png STATIC IMPORTED)
set(ROOT_hltypes "../../../../hltypes/android-studio/lib")
set(ROOT_gtypes "../../../../gtypes/android-studio/lib")
set(ROOT_jpeg "../../../lib/jpeg/android-studio/lib")
set(ROOT_png "../../../lib/png/android-studio/lib")
set(ROOT_april "../../lib")
add_subdirectory(${ROOT_hltypes} hltypes)
add_subdirectory(${ROOT_gtypes} gtypes)
add_subdirectory(${ROOT_jpeg} jpeg)
add_subdirectory(${ROOT_png} png)
add_subdirectory(${ROOT_april} april)
#set_target_properties(april PROPERTIES IMPORTED_LOCATION "${CMAKE_CURRENT_SOURCE_DIR}/${ROOT_april}/${LIB_PATH}/libapril.a")
#set_target_properties(hltypes PROPERTIES IMPORTED_LOCATION "${CMAKE_CURRENT_SOURCE_DIR}/${ROOT_hltypes}/${LIB_PATH}/libhltypes.a")
#set_target_properties(gtypes PROPERTIES IMPORTED_LOCATION "${CMAKE_CURRENT_SOURCE_DIR}/${ROOT_gtypes}/${LIB_PATH}/libgtypes.a")
#set_target_properties(jpeg PROPERTIES IMPORTED_LOCATION "${CMAKE_CURRENT_SOURCE_DIR}/${ROOT_jpeg}/${LIB_PATH}/libjpeg.a")
#set_target_properties(png PROPERTIES IMPORTED_LOCATION "${CMAKE_CURRENT_SOURCE_DIR}/${ROOT_png}/${LIB_PATH}/libpng.a")

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
