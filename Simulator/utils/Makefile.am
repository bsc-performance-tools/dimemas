noinst_LTLIBRARIES = libDimemasUtils.la

simulator_dir = $(top_srcdir)/Simulator

AM_CPPFLAGS = \
	-I$(top_srcdir)/common_utilities \
	-I$(top_srcdir)/include \
	-I$(simulator_dir)/io \
	-I$(simulator_dir)/input \
	-I$(simulator_dir)/model \
	-I$(simulator_dir)/prv_utils \
	-I$(simulator_dir)/sched \
	-I$(simulator_dir)/utils \
	-I$(simulator_dir)/venusclient

libDimemasUtils_la_SOURCES = \
	aleatorias.c \
	aleatorias.h \
	cp.c \
	cp.h \
	list.c \
	list.h \
	random.c \
	random.h \
	subr.c \
	subr.h \
	listE.h \
	listE.cc \
	modules_map.h \
	modules_map.cc \
	deadlock_analysis.c \
	deadlock_analysis.h \
	graph.c \
	graph.h

#if USE_EQUEUE
#libDimemasUtils_la_SOURCES += \
#	listE.h \
#	listE.cc
#endif
