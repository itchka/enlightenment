MAINTAINERCLEANFILES = Makefile.in

AM_CPPFLAGS = \
-I$(top_srcdir)/src/lib/ecore \
-I$(top_srcdir)/src/lib/ecore_input \
-I$(top_builddir)/src/lib/ecore \
-I$(top_builddir)/src/lib/ecore_input \
-DEFL_ECORE_WINCE_BUILD \
@EVAS_CFLAGS@ \
@EINA_CFLAGS@

lib_LTLIBRARIES = libecore_wince.la

includes_HEADERS = Ecore_WinCE.h
includesdir = $(includedir)/ecore-@VMAJ@

libecore_wince_la_SOURCES = \
ecore_wince.c \
ecore_wince_event.c \
ecore_wince_window.c \
ecore_wince_private.h

libecore_wince_la_LIBADD = \
@WIN32_LIBS@ \
$(top_builddir)/src/lib/ecore_input/libecore_input.la \
$(top_builddir)/src/lib/ecore/libecore.la \
@EVAS_LIBS@ \
@EINA_LIBS@ \
@EVIL_LIBS@

libecore_wince_la_LDFLAGS = -no-undefined -version-info @version_info@ @release_info@
