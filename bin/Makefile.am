if OS_WIN32
no_undefined = -no-undefined
endif

SUBDIRS=lua

AM_CPPFLAGS = \
	-I$(top_srcdir) \
	-I$(top_srcdir)/bin \
	-I$(top_builddir)/bin \
	-I$(top_builddir)/gegl \
	-I$(top_srcdir)/gegl \
	-I$(top_builddir)/gegl/buffer \
	-I$(top_srcdir)/gegl/buffer \
	-I$(top_builddir)/gegl/graph \
	-I$(top_srcdir)/gegl/graph \
	-I$(top_builddir)/gegl/module \
	-I$(top_srcdir)/gegl/module \
	-I$(top_builddir)/gegl/operation \
	-I$(top_srcdir)/gegl/operation \
	-I$(top_builddir)/gegl/opencl \
	-I$(top_srcdir)/gegl/opencl \
	-I$(top_builddir)/gegl/process \
	-I$(top_srcdir)/gegl/process \
	-I$(top_builddir)/gegl/property-types \
	-I$(top_srcdir)/gegl/property-types

AM_CFLAGS = \
	$(DEP_CFLAGS) $(BABL_CFLAGS) $(PNG_CFLAGS) \
        $(MRG_CFLAGS) $(GEXIV2_CFLAGS)

AM_LDFLAGS =  \
	$(no_undefined) ../gegl/libgegl-$(GEGL_API_VERSION).la \
	$(DEP_LIBS) $(BABL_LIBS) $(PNG_LIBS) $(LIBSPIRO) $(MATH_LIB) \
        $(MRG_LIBS) $(GEXIV2_LIBS)

if HAVE_LUA
AM_LDFLAGS += $(LUA_LIBS)
AM_CFLAGS  += $(LUA_CFLAGS)
endif


bin_PROGRAMS = gegl

BUILT_SOURCES=argvs-commands.inc

CLEANFILES=argvs-commands.inc

gegl_SOURCES =			\
	argvs.c			\
	argvs.h			\
	gegl.c			\
	gegl-options.c		\
	gegl-options.h		\
	gegl-path-smooth.c	\
	gegl-path-smooth.h

argvs-commands.inc: gegl.c ui-core.c ui-viewer.c ui-collection.c argvs.c Makefile
	@echo " GEN" $@
	@$(top_srcdir)/tools/argvs_extract.sh $(top_srcdir)/bin/*.c > $(top_builddir)/bin/argvs-commands.inc

if HAVE_MRG
if HAVE_GEXIV2
gegl_SOURCES += ui-core.c ui.h ui-viewer.c ui-collection.c mrg-gegl.c mrg-gegl.h
AM_CFLAGS += $(SDL_CFLAGS)
AM_LDFLAGS += $(SDL_LIBS)
endif
endif

if HAVE_SPIRO
gegl_SOURCES += gegl-path-spiro.h gegl-path-spiro.c
endif
