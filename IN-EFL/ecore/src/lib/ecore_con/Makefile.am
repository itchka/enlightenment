MAINTAINERCLEANFILES = Makefile.in

AM_CPPFLAGS = \
-I$(top_builddir)/src/lib/ecore \
-I$(top_builddir)/src/lib/ecore_con \
-I$(top_srcdir)/src/lib/ecore \
-I$(top_srcdir)/src/lib/ecore_con \
-DEFL_ECORE_CON_BUILD \
@SSL_CFLAGS@ \
@CURL_CFLAGS@ \
@EVIL_CFLAGS@ \
@EINA_CFLAGS@ \
@TLS_CFLAGS@ \
@CARES_CFLAGS@ \
@EET_CFLAGS@

lib_LTLIBRARIES = libecore_con.la
includes_HEADERS = Ecore_Con.h
includesdir = $(includedir)/ecore-@VMAJ@

libecore_con_la_SOURCES = \
ecore_con.c \
ecore_con_socks.c \
ecore_con_ssl.c \
ecore_con_url.c \
ecore_con_alloc.c \
ecore_con_private.h

if ECORE_HAVE_WIN32
libecore_con_la_SOURCES += ecore_con_local_win32.c
else
libecore_con_la_SOURCES += ecore_con_local.c
endif

if HAVE_CARES
libecore_con_la_SOURCES += ecore_con_ares.c
else
if HAVE_IPV6
libecore_con_la_CFLAGS = @ECORE_CON_CFLAGS@
libecore_con_la_SOURCES += ecore_con_dns.c dns.c dns.h
else
libecore_con_la_SOURCES += ecore_con_info.c
endif
endif

if ECORE_HAVE_EET
libecore_con_la_SOURCES += ecore_con_eet.c
includes_HEADERS += Ecore_Con_Eet.h
endif

libecore_con_la_LIBADD = \
$(top_builddir)/src/lib/ecore/libecore.la \
@SSL_LIBS@ @CURL_LIBS@ @EINA_LIBS@ @EVIL_LIBS@ @TLS_LIBS@ @CARES_LIBS@ @WIN32_LIBS@ @EET_LIBS@

libecore_con_la_LDFLAGS = -no-undefined -version-info @version_info@ @release_info@
