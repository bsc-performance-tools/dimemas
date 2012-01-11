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
	MPI_EventEncoding.h \
	paraver.c \
	paraver.h \
	paraver_pcf.c \
	paraver_pcf.h 
