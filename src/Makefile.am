common_filesdir = @common_filesdir@

SUBDIRS = io input model prv_utils sched utils

if VENUS_ENABLED
  SUBDIRS += venusclient
endif

INCLUDES = \
	-I$(top_srcdir)/include \
	-I$(top_srcdir)/src/io \
	-I$(top_srcdir)/src/input \
	-I$(top_srcdir)/src/model \
	-I$(top_srcdir)/src/prv_utils \
	-I$(top_srcdir)/src/sched \
	-I$(top_srcdir)/src/utils \
	-I$(top_srcdir)/src/venusclient

bin_PROGRAMS = Dimemas3 DimemasUpdateCFG

Dimemas3_SOURCES = \
	main.c 

nodist_Dimemas3_SOURCES = \
	$(common_filesdir)/ParaverBinaryTrace.c\
	$(common_filesdir)/Dimemas2Prv.c \
	$(common_filesdir)/EventEncoding.c

Dimemas3_CFLAGS=-I$(top_srcdir) -I$(top_srcdir)/include -I$(common_filesdir)
Dimemas3_CXXFLAGS=-I$(top_srcdir) -I$(top_srcdir)/include -I$(common_filesdir)

nodist_EXTRA_Dimemas3_SOURCES = dummy.cc

Dimemas3_LDADD = \
	$(top_srcdir)/src/input/libDimemasInput.la \
	$(top_srcdir)/src/io/libDimemasIO.la \
	$(top_srcdir)/src/model/libDimemasModel.la \
	$(top_srcdir)/src/prv_utils/libDimemas2ParaverUtils.la \
	$(top_srcdir)/src/sched/libDimemasScheduling.la \
	$(top_srcdir)/src/utils/libDimemasUtils.la 

if VENUS_ENABLED
Dimemas3_LDADD += $(top_srcdir)/src/venusclient/libDimemasVenusClient.la
endif

DimemasUpdateCFG_SOURCES = Dimemas_updateCFGs.cpp
