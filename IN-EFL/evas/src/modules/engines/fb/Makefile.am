
MAINTAINERCLEANFILES = Makefile.in

AM_CPPFLAGS = \
-I. \
-I$(top_srcdir)/src/lib \
-I$(top_srcdir)/src/lib/include \
-I$(top_srcdir)/src/modules/engines \
@FREETYPE_CFLAGS@ \
@PIXMAN_CFLAGS@ \
@EVAS_GENERAL_CFLAGS@ \
@FRIBIDI_CFLAGS@ \
@evas_engine_fb_cflags@

if BUILD_ENGINE_FB

FB_SOURCES = \
evas_engine.c \
evas_fb_main.c \
evas_outbuf.c

FB_LIBADD = @evas_engine_fb_libs@


includes_HEADERS = Evas_Engine_FB.h
includesdir = $(includedir)/evas-@VMAJ@

if !EVAS_STATIC_BUILD_FB

pkgdir = $(libdir)/evas/modules/engines/fb/$(MODULE_ARCH)
pkg_LTLIBRARIES = module.la
module_la_SOURCES = $(FB_SOURCES)
module_la_LIBADD = @EVAS_GENERAL_LIBS@ $(FB_LIBADD) $(top_builddir)/src/lib/libevas.la
module_la_LDFLAGS = -module -avoid-version
module_la_LIBTOOLFLAGS = --tag=disable-static

else

noinst_LTLIBRARIES = libevas_engine_fb.la

libevas_engine_fb_la_SOURCES = $(FB_SOURCES)
libevas_engine_fb_la_LIBADD = $(FB_LIBADD)

endif
endif

EXTRA_DIST = \
evas_engine.h \
evas_fb.h
