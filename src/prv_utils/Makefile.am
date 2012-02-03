noinst_LTLIBRARIES = libDimemas2ParaverUtils.la

common_filesdir = @common_filesdir@

INCLUDES = \
	-I$(top_srcdir)/include \
	-I$(common_filesdir) \
	-I$(top_srcdir)/src/io \
	-I$(top_srcdir)/src/input \
	-I$(top_srcdir)/src/model \
	-I$(top_srcdir)/src/prv_utils \
	-I$(top_srcdir)/src/sched \
	-I$(top_srcdir)/src/utils \
	-I$(top_srcdir)/src/venusclient

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
