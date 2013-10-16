noinst_LTLIBRARIES = libDimemasModel.la
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

libDimemasModel_la_SOURCES = \
	communic.c \
	communic.h \
	cpu.c \
	cpu.h \
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
	task.h \
	thread_states.c \
	thread_states.h 

