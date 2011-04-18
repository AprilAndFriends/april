SUBDIRS = src include/aprilutil
EXTRA_DIST = autogen.sh

lib_LTLIBRARIES = libaprilutil.la
libaprilutil_la_SOURCES = 
libaprilutil_la_LIBADD =  $(LIBOBJS) \
	src/libaprilutil_src.la
libaprilutil_la_LDFLAGS = -version-info @LT_VERSION@

