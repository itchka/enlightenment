## Process this file with automake to produce Makefile.in

AM_CPPFLAGS = -I$(top_srcdir) \
              -DPACKAGE_BIN_DIR=\"$(bindir)\" \
              -DPACKAGE_DATA_DIR=\"$(pkgdatadir)\" \
              -DPACKAGE_LIBEXEC_DIR=\"$(libexecdir)\" \
              -Wall -Werror

bin_PROGRAMS = empower

empower_SOURCES = main.c gui.c pka.c marshal.c ckit.c logind.c

empower_CFLAGS = @EMPOWER_CFLAGS@

empower_LDADD =  $(LTLIBINTL) \
                 @EMPOWER_LIBS@
                 
if HAVE_LIBSYSTEMD_LOGIN
  empower_CFLAGS += @LIBSYSTEMD_LOGIN_CFLAGS@
  empower_LDADD += @LIBSYSTEMD_LOGIN_LIBS@
endif

EXTRA_DIST = Empower.h gui.h pka.h marshal.h ckit.h logind.h

MAINTAINERCLEANFILES = Makefile.in config.h.in
