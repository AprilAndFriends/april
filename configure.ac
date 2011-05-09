
# initialize autoconf
AC_INIT(libapril, 1.2, ivan@vucica.net)

# autoconf configuration
AC_PREREQ(2.61)
AC_CONFIG_SRCDIR(src/Window.cpp)

# libtool configuration
#  LT_VERSION has nothing to do with VERSION.
#  It is used in versioning of .so (as part of .so's filename), but
#  it is unrelated to VERSION.
#
#   +1 : ? : +1  == new interface that does not break old one
#   +1 : ? : 0   == new interface that breaks old one
#    ? : ? : 0   == no new interfaces, but breaks apps
#    ? :+1 : ?   == just some internal changes, nothing breaks but might work
#                   better
#   CURRENT : REVISION : AGE
LT_VERSION=0:0:0
AC_SUBST(LT_VERSION)

# set up languages that will be used
AC_PROG_CC
AC_PROG_CXX
AC_LANG(C++)

# initialize automake
AM_INIT_AUTOMAKE([foreign])

# more libtool setup
AC_PROG_LIBTOOL # pre-2.2 libtool
#LT_PREREQ([2.2]) # post-2.2 libtool
#LT_INIT #([disable-shared])
AC_SUBST([LIBTOOL_DEPS])

# begin checks
# check standard c headers
AC_HEADER_STDC

# check for opengl
AC_CHECK_HEADERS(GL/gl.h, ,[AC_MSG_ERROR("*** OpenGL headers not found.")])
AC_CHECK_LIB(GL, glLoadIdentity, 	
	[CFLAGS="$CFLAGS -D_OPENGL"] [CXXFLAGS="$CXXFLAGS -D_OPENGL"], 
	[AC_CHECK_LIB(opengl32, glLoadIdentity, 
			[CFLAGS="$CFLAGS -D_OPENGL"] [CXXFLAGS="$CXXFLAGS -D_OPENGL"], 
			[AC_MSG_ERROR("*** OpenGL library not found.")])
])

#check SDL
SDL_VERSION=1.2.0
AM_PATH_SDL($SDL_VERSION, :, AC_MSG_ERROR([*** SDL version $SDL_VERSION not found!]))
CFLAGS="$CFLAGS $SDL_CFLAGS -DHAVE_SDL"
CXXFLAGS="$CXXFLAGS $SDL_CFLAGS -DHAVE_SDL"
LIBS="$LIBS $SDL_LIBS"
AC_SUBST(SDL_LIBS)

#check devil
AC_CHECK_HEADERS([IL/il.h], ,[AC_MSG_ERROR("*** devil headers not found.")])
LIBS="$LIBS -lIL"
CFLAGS="$CFLAGS -DHAVE_IL=1 -DUSE_IL=1"
CXXFLAGS="$CXXFLAGS -DHAVE_IL=1 -DUSE_IL=1"

# check for hltypes
AC_CHECK_HEADERS(hltypes/exception.h, ,[AC_MSG_ERROR("*** hltypes headers not found.")])
#SAVED_LDFLAGS=$LDFLAGS
#LDFLAGS="$LDFLAGS -lhltypes"
#AC_LINK_IFELSE(
#	[AC_LANG_PROGRAM(
#		[#include <hltypes/hstring.h>],
#		[hltypes::string dummy]
#	)],
#	,
#	[AC_MSG_ERROR("*** hltypes library not found.")]
#)
#LDFLAGS=$SAVED_LDFLAGS


# check for gtypes
AC_CHECK_HEADERS(gtypes/Rectangle.h, ,[AC_MSG_ERROR("*** gtypes headers not found.")])


# end checks; output files
AC_OUTPUT(Makefile src/Makefile include/april/Makefile rendersystems/OpenGL/Makefile windowsystems/SDL/Makefile)

