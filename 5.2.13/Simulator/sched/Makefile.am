noinst_LTLIBRARIES = libDimemasScheduling.la

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
