common_filesdir = @common_filesdir@
simulator_dir = $(top_srcdir)/Simulator

SUBDIRS = io input model prv_utils sched utils

if VENUS_ENABLED
  SUBDIRS += venusclient
endif

INCLUDES = \
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

nodist_Dimemas_SOURCES = \
	$(common_filesdir)/ParaverBinaryTrace.c\
	$(common_filesdir)/Dimemas2Prv.c \
	$(common_filesdir)/EventEncoding.c

Dimemas_CFLAGS=-I$(top_srcdir) -I$(top_srcdir)/include -I$(common_filesdir)
Dimemas_CXXFLAGS=-I$(top_srcdir) -I$(top_srcdir)/include -I$(common_filesdir)

nodist_EXTRA_Dimemas_SOURCES = dummy.cc

Dimemas_LDADD = \
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
