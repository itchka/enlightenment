MAINTAINERCLEANFILES = Makefile.in

AM_CPPFLAGS = \
-I $(top_srcdir)/src/lib/dbus \
@EDBUS_CFLAGS@

lib_LTLIBRARIES = libehal.la
includes_HEADERS = E_Hal.h
includesdir = $(includedir)/e_dbus-@VMAJ@

libehal_la_SOURCES = \
e_hal_device.c  \
e_hal_manager.c \
e_hal_util.c \
e_hal_main.c \
e_hal_private.h

libehal_la_LIBADD = \
@EDBUS_LIBS@ $(top_builddir)/src/lib/dbus/libedbus.la

libehal_la_LDFLAGS = -no-undefined -version-info @version_info@ @release_info@
