# autoconf configuration
AC_PREREQ(2.61)
AC_CONFIG_SRCDIR(src/Window.cpp)
#AC_CONFIG_MACRO_DIR([m4]

# initialize autoconf
AC_INIT(libapril, 1.2, ivan@vucica.net)

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
LT_PREREQ([2.2])
LT_INIT #([disable-shared])
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

