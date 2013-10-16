noinst_LTLIBRARIES = libDimemas2ParaverUtils.la

simulator_dir = $(top_srcdir)/Simulator
common_filesdir = @common_filesdir@

INCLUDES = \
	-I$(top_srcdir)/include \
	-I$(common_filesdir) \
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
	paraver_pcf.h \
	pcf_defines.h

nodist_libDimemas2ParaverUtils_la_SOURCES = \
	$(common_filesdir)/MPI_EventEncoding.h
