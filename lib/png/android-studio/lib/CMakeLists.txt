# For more information about using CMake with Android Studio, read the
# documentation: https://d.android.com/studio/projects/add-native-code.html

# Sets the minimum version of CMake required to build the native library.

file(GLOB CppSrc
    "../../*.c"
    "../../*.cpp"
)

#set(CMAKE_CXX_STANDARD 11)  # enable C++11 standard
#SET( CMAKE_CXX_FLAGS  "${CMAKE_CXX_FLAGS} ${GCC_COVERAGE_COMPILE_FLAGS}" )
#add_definition(-D__ANDROIDX__)
#set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++1y -Wall")

include_directories(AFTER ../..)
#include_directories( ${MY_SOURCE_DIR}/src )

add_library(
	png
	STATIC
	${CppSrc}
)

find_library(
	# Sets the name of the path variable.
	log-lib
	# Specifies the name of the NDK library that
	# you want CMake to locate.
	log
)

# Specifies libraries CMake should link to your target library. You
# can link multiple libraries, such as libraries you define in this
# build script, prebuilt third-party libraries, or system libraries.

target_link_libraries(
	# Specifies the target library.
	png
	# Links the target library to the log library
	# included in the NDK.
	${log-lib}
)
