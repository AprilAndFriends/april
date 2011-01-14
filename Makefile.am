ACLOCAL_AMFLAGS = -I m4
SUBDIRS = src include/april rendersystems/OpenGL windowsystems/SDL
EXTRA_DIST = autogen.sh

lib_LTLIBRARIES = libapril.la
libapril_la_SOURCES = main_linux.cpp
libapril_la_LIBADD =  $(LIBOBJS) \
	src/libapril_src.la \
	rendersystems/OpenGL/libapril_rendersystems_opengl.la \
	windowsystems/SDL/libapril_windowsystems_sdl.la
libapril_la_LDFLAGS = -version-info @LT_VERSION@

