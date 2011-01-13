AC_PREREQ(2.61)

AC_INIT(libapril, 1.2, ivan@vucica.net)
AC_CONFIG_SRCDIR(src/Window.cpp)

PACKAGE=libapril
VERSION=1.2.0



APRIL_RELEASE=1:2:0
APRIL_LIBRARY_VERSION=0:0:0
#                     | | |
#              +------+ | +---+
#              |        |     |
#            current:revision:age
#              |        |     |
#              |        |     +- increment if interfaces have been added
#              |        |        set to zero if interfaces have been removed
#              |        |        or changed
#              |        +- increment if source code has changed
#              |           set to zero if current is incremented
#              +- increment if interfaces have been added, removed or changed



#AM_INIT_AUTOMAKE($PACKAGE, $VERSION)
AM_INIT_AUTOMAKE([foreign])


AC_PROG_CC
AC_PROG_CXX
AC_PROG_RANLIB
AC_LANG(C++)

AC_HEADER_STDC

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




AC_OUTPUT(Makefile src/Makefile include/april/Makefile rendersystems/OpenGL/Makefile windowsystems/SDL/Makefile)

