noinst_LTLIBRARIES = libDimemasModel.la
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
	-I$(simulator_dir)/venusclient \
	-I$(simulator_dir)/eee_model

libDimemasModel_la_SOURCES = \
	communic.c \
	communic.h \
	cpu.c \
	cpu.h \
	dim_omp.cc \
	dim_omp.h \
	fs.c \
	fs.h \
	links.c \
	links.h \
	machine.c \
	machine.h \
	memory.c \
	memory.h \
	node.c \
	node.h \
	ports.c \
	ports.h \
	semaphore.c \
	semaphore.h \
	simulator.c \
	simulator.h \
	task.c \
	task.h 

