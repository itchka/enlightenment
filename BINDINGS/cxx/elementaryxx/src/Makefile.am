## Process this file with automake to produce Makefile.in

## Created by Anjuta

AM_CPPFLAGS = \
	-DPACKAGE_LOCALE_DIR=\""$(prefix)/$(DATADIRNAME)/locale"\" \
	-DPACKAGE_SRC_DIR=\""$(srcdir)"\" \
	-DPACKAGE_DATA_DIR=\""$(datadir)"\" \
	$(EFLXX_CFLAGS) \
	$(EFL_CFLAGS) \
	-I../include

AM_CFLAGS =\
	 -Wall\
	 -g

lib_LTLIBRARIES = \
	libelementaryxx.la

libelementaryxx_la_SOURCES = \
	Application.cpp  \
	Window.cpp  \
	Button.cpp  \
	Clock.cpp  \
	Scroller.cpp  \
	Entry.cpp  \
	Check.cpp  \
	Icon.cpp  \
	Image.cpp  \
	Radio.cpp  \
	Slider.cpp  \
	Bubble.cpp  \
	Label.cpp  \
	Background.cpp  \
	Box.cpp  \
	Frame.cpp  \
	List.cpp  \
	ListItem.cpp  \
	Table.cpp  \
	Object.cpp  \
	Layout.cpp  \
	Hover.cpp  \
	Spinner.cpp  \
	Progressbar.cpp  \
	Panel.cpp  \
	Separator.cpp  \
	GenList.cpp  \
	GenDataModel.cpp  \
	GenListColumnConstructor.cpp \
	GenListColumnSelector.cpp \
	localUtil.cpp \
	localUtil.h \
	Gen.cpp \
	GenItem.cpp \
	GenListItem.cpp  \
	Theme.cpp

libelementaryxx_la_LIBADD = \
	$(EFLXX_LIBS)        \
	$(EFL_LIBS)

