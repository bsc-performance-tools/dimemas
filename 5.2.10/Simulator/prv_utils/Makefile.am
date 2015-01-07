noinst_LTLIBRARIES = libDimemas2ParaverUtils.la

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

libDimemas2ParaverUtils_la_SOURCES = \
	paraver_records.cc \
	paraver_records.h \
	external_sort.cc \
	external_sort.h \
	paraver.cc \
	paraver.h \
	paraver_pcf.c \
	paraver_pcf.h 

libDimemas2ParaverUtils_la_LIBADD = \
	$(top_srcdir)/common_utilities/libCommonUtilities.la
