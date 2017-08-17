simulator_dir = $(top_srcdir)/Simulator

SUBDIRS = io input model eee_model prv_utils sched utils

EXTRA_DIST = lib_extern_model

if VENUS_ENABLED
  SUBDIRS += venusclient
endif

AM_CPPFLAGS = \
	-I$(top_srcdir)/common_utilities \
	-I$(top_srcdir)/include \
	-I$(simulator_dir)/io \
	-I$(simulator_dir)/input \
	-I$(simulator_dir)/model \
	-I$(simulator_dir)/prv_utils \
	-I$(simulator_dir)/sched \
	-I$(simulator_dir)/utils \
	-I$(simulator_dir)/venusclient \
	-I$(simulator_dir)/eee_model \
	$(BOOST_CPPFLAGS)


bin_PROGRAMS = Dimemas

Dimemas_SOURCES = main.cpp

Dimemas_CFLAGS= \
	-I$(top_srcdir) \
	-I$(top_srcdir)/include

Dimemas_CXXFLAGS= \
	-I$(top_srcdir) \
	-I$(top_srcdir)/include

Dimemas_LDFLAGS= \
	$(BOOST_LDFLAGS)

Dimemas_LDADD = \
	$(simulator_dir)/model/libDimemasModel.la \
	$(top_srcdir)/common_utilities/libCommonUtilities.la \
	$(simulator_dir)/input/libDimemasInput.la \
	$(simulator_dir)/io/libDimemasIO.la \
	$(simulator_dir)/prv_utils/libDimemas2ParaverUtils.la \
	$(simulator_dir)/sched/libDimemasScheduling.la \
	$(simulator_dir)/utils/libDimemasUtils.la \
	$(simulator_dir)/eee_model/libDimemasEEEModel.la \
	$(BOOST_PROGRAM_OPTIONS_LIB)

if VENUS_ENABLED
Dimemas_LDADD += $(simulator_dir)/venusclient/libDimemasVenusClient.la
endif

LIB_EXTERN_MODEL_SRCS=lib_extern_model/Makefile \
	lib_extern_model/extern_comm_model.c \
	lib_extern_model/README

install-data-hook:
	$(mkdir_p) $(includedir)/
	$(mkdir_p) $(datadir)/lib_extern_model_example
	cp -f $(LIB_EXTERN_MODEL_SRCS) $(datadir)/lib_extern_model_example
	cp -f lib_extern_model/extern_comm_model.h $(includedir)/

