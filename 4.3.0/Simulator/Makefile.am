simulator_dir = $(top_srcdir)/Simulator

SUBDIRS = io input model prv_utils sched utils

if VENUS_ENABLED
  SUBDIRS += venusclient
endif

INCLUDES = \
	-I$(top_srcdir)/common_utilities \
	-I$(top_srcdir)/include \
	-I$(simulator_dir)/io \
	-I$(simulator_dir)/input \
	-I$(simulator_dir)/model \
	-I$(simulator_dir)/prv_utils \
	-I$(simulator_dir)/sched \
	-I$(simulator_dir)/utils \
	-I$(simulator_dir)/venusclient

bin_PROGRAMS = Dimemas DimemasUpdateCFG

Dimemas_SOURCES = \
	main.c 

Dimemas_CFLAGS=-I$(top_srcdir) -I$(top_srcdir)/include
Dimemas_CXXFLAGS=-I$(top_srcdir) -I$(top_srcdir)/include

nodist_EXTRA_Dimemas_SOURCES = dummy.cc

Dimemas_LDADD = \
	$(top_srcdir)/common_utilities/libCommonUtilities.la \
	$(simulator_dir)/input/libDimemasInput.la \
	$(simulator_dir)/io/libDimemasIO.la \
	$(simulator_dir)/model/libDimemasModel.la \
	$(simulator_dir)/prv_utils/libDimemas2ParaverUtils.la \
	$(simulator_dir)/sched/libDimemasScheduling.la \
	$(simulator_dir)/utils/libDimemasUtils.la 

if VENUS_ENABLED
Dimemas_LDADD += $(simulator_dir)/venusclient/libDimemasVenusClient.la
endif

DimemasUpdateCFG_SOURCES = Dimemas_updateCFGs.cpp
