MAINTAINERCLEANFILES = Makefile.in

EDJE_CC = $(top_builddir)/src/bin/@EDJE_CC_PRG@
EDJE_CC_FLAGS_VERBOSE_0 =
EDJE_CC_FLAGS_VERBOSE_1 = -v
EDJE_CC_FLAGS = $(EDJE_CC_FLAGS_$(V)) -id $(srcdir) -fd $(srcdir)

EDJE_CODEGEN = $(top_builddir)/src/bin/@EDJE_CODEGEN_PRG@
EDJE_CODEGEN_FLAGS = --prefix=codegen_example

examplesdir = $(pkgdatadir)/examples

if ENABLE_MULTISENSE
SND_DIR = -sd $(srcdir)
endif

#put here all EDCs one needs to the examples
EDCS = \
animations2.edc \
animations.edc \
basic2.edc \
basic.edc \
box.edc \
codegen.edc \
color-class.edc \
drag.edc \
edje_example.edc \
edje-nested.edc \
embryo_custom_state.edc \
embryo_pong.edc \
embryo_run_program.edc \
embryo_set_state.edc \
embryo_set_text.edc \
embryo_timer.edc \
external_elm_anchorblock.edc \
external_elm_button.edc \
external_elm_check.edc \
external_elm_panes.edc \
external_emotion_elm.edc \
lua_script.edc \
messages_echo.edc \
perspective.edc \
signalsBubble.edc \
signals-messages.edc \
sigtest.edc \
swallow.edc \
table.edc \
text.edc \
toggle_using_filter.edc

if ENABLE_MULTISENSE
EDCS += multisense.edc
endif

if ENABLE_EPHYSICS
EDCS += \
physics_3d.edc \
physics_actions.edc \
physics_backcull.edc \
physics_basic.edc \
physics_complex.edc \
physics_soft_bodies.edc
endif

.edc.edj:
	$(EDJE_CC) $(EDJE_CC_FLAGS) $(SND_DIR) $< $(builddir)/$(@F)


EDJS = $(EDCS:%.edc=%.edj)

filesdir = $(pkgdatadir)/examples
files_DATA = \
$(EDCS) \
bubble.png \
bubble-blue.png \
red.png \
test.png \
Vera.ttf \
duck.wav \
edje-codegen-example.c \
edje_example.c \
edje-basic.c \
edje-swallow.c \
edje-text.c \
edje-table.c \
edje-box.c \
edje-box2.c \
edje-drag.c \
edje-signals-messages.c \
edje-color-class.c \
edje-perspective.c \
edje-animations.c \
sigtest.c \
animations2.c \
edje-basic2.c \
signals2.c \
edje-swallow2.c

if ENABLE_MULTISENSE
files_DATA += edje-multisense.c
endif

EXTRA_DIST = $(files_DATA)

if BUILD_EXAMPLES

AM_CPPFLAGS = \
-I. \
-I$(top_srcdir)/src/lib \
-I$(top_srcdir)/src/lib/include \
-DPACKAGE_BIN_DIR=\"$(bindir)\" \
-DPACKAGE_LIB_DIR=\"$(libdir)\" \
-DPACKAGE_DATA_DIR=\"$(pkgdatadir)\" \
@EDJE_CFLAGS@

codegen_example_generated.c codegen_example_generated.h: codegen.edj
	$(EDJE_CODEGEN) $(EDJE_CODEGEN_FLAGS) codegen.edj example_group $(builddir)/codegen_example_generated.c $(builddir)/codegen_example_generated.h

BUILT_SOURCES = codegen_example_generated.c codegen_example_generated.h

edje_codegen_example_SOURCES = \
edje-codegen-example.c
nodist_edje_codegen_example_SOURCES = \
codegen_example_generated.c \
codegen_example_generated.h

files_DATA += $(EDJS)

examples_PROGRAMS = \
edje-animations \
edje-basic \
edje-box \
edje-box2 \
edje-codegen-example \
edje-color-class \
edje-drag\
edje-perspective \
edje-signals-messages \
edje-swallow \
edje-table \
edje-text \
sigtest \
animations2 \
edje-basic2 \
signals2 \
edje-swallow2

if ENABLE_MULTISENSE
examples_PROGRAMS += edje-multisense
endif

LDADD = $(top_builddir)/src/lib/libedje.la @EDJE_LIBS@

endif

clean-local:
	rm -f *.edj
