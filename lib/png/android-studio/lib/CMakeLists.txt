cmake_minimum_required(VERSION 3.4.1)
set(CMAKE_VERBOSE_MAKEFILE on)

project(png)

# defining manually so source files don't need to be changed
file(
	GLOB CppSrc
	"../../png.c"
	"../../pngerror.c"
	"../../pngget.c"
	"../../pngpmem.c"
	"../../pngpread.c"
	"../../pngread.c"
	"../../pngrio.c"
	"../../pngrtran.c"
	"../../pngrutil.c"
	"../../pngset.c"
	"../../pngtrans.c"
	"../../pngwio.c"
	"../../pngwrite.c"
	"../../pngwtran.c"
	"../../pngwutil.c"
)

add_library(png STATIC ${CppSrc})

include_directories(AFTER "../..")

target_link_libraries(
	png
)
