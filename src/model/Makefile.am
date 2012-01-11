noinst_LTLIBRARIES = libDimemasModel.la

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

libDimemasModel_la_SOURCES = \
	communic.c \
	communic.h \
	cpu.c \
	cpu.h \
	fs.c \
	fs.h \
	links.c \
	links.h \
	memory.c \
	memory.h \
	ports.c \
	ports.h \
	semaphore.c \
	semaphore.h \
	task.c \
	task.h \
	thread_states.c \
	thread_states.h

