AUTOMAKE_OPTIONS=foreign

bin_PROGRAMS = e16keyedit

e16keyedit_SOURCES = ipc.c ipc.h menus.c menus.h viewer.c viewer.h

CPPFLAGS = -D DATADIR=\"$(datadir)\"

LDADD = $(X_LIBS) -lX11
