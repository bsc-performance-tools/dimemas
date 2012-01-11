noinst_LTLIBRARIES = libDimemasScheduling.la

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

libDimemasScheduling_la_SOURCES = \
	events.c \
	events.h \
	SCH_boost.c \
	SCH_boost.h \
	schedule.c \
	schedule.h \
	sched_vars.c \
	sched_vars.h \
	SCH_fifo.c \
	SCH_fifo.h \
	SCH_prio_fifo.c \
	SCH_prio_fifo.h \
	SCH_rr.c \
	SCH_rr.h \
	SCH_svr4.c \
	SCH_svr4.h \
	SCH_ss_mpi_cp.c \
	SCH_ss_mpi_cp.h
