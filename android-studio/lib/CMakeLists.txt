cmake_minimum_required(VERSION 3.4.1)
set(CMAKE_VERBOSE_MAKEFILE on)

project(april)

include("../../../hltypes/android-studio/generic/cmake-preprocessors.txt")

file(
	GLOB_RECURSE CppSrc
	"../../src/*.c"
	"../../src/*.cpp"
)

add_library(april STATIC ${CppSrc})

include_directories(AFTER "../../include/april")
include_directories(AFTER "../../src/util")
include_directories(AFTER "../../src/async")
include_directories(AFTER "../../src/windowsystems/Win32")
include_directories(AFTER "../../src/windowsystems/SDL")
include_directories(AFTER "../../src/windowsystems/AndroidJNI")
include_directories(AFTER "../../src/rendersystems/DirectX")
include_directories(AFTER "../../src/rendersystems/DirectX/9")
include_directories(AFTER "../../src/rendersystems/OpenGL")
include_directories(AFTER "../../src/rendersystems/OpenGL/GL")
include_directories(AFTER "../../src/rendersystems/OpenGL/GL/1")
include_directories(AFTER "../../src/rendersystems/OpenGL/GL/ES1")
include_directories(AFTER "../../src/rendersystems/OpenGL/GLES")
include_directories(AFTER "../../src/rendersystems/OpenGL/GLES/2")
include_directories(AFTER "../../../hltypes/include")
include_directories(AFTER "../../../gtypes/include")
include_directories(AFTER "../../../hltypes/lib/zlib")
include_directories(AFTER "../../lib/jpeg")
include_directories(AFTER "../../lib/png")

add_definitions(
	-DAPRIL_EXPORTS
	-DBUILDING_APRIL
	-D_OPENGL
	-D_OPENGLES
	-D_OPENGLES2
	-D_ANDROIDJNI_WINDOW
)
