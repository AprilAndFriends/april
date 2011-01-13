#ACLOCAL_AMFLAGS=-I m4
SUBDIRS = src include/april rendersystems/OpenGL windowsystems/SDL
EXTRA_DIST = autogen.sh

lib_LIBRARIES = libapril.a
libapril_a_SOURCES = 
libapril_a_LIBADD =  $(LIBOBJS) \
	src/libapril_src.a \
	rendersystems/OpenGL/libapril_rendersystems_opengl.a \
	windowsystems/SDL/libapril_windowsystems_sdl.a

