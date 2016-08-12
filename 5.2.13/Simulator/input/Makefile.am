noinst_LTLIBRARIES = libDimemasInput.la
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

# Special definition of the yacc header

AM_YFLAGS=-d
BUILT_SOURCES = yacc.h  

libDimemasInput_la_SOURCES = \
	yacc.y \
	lex.l \
	check.c \
	check.h \
	comm_configuration.c \
	comm_configuration.h \
	configuration.c \
	configuration.h \
	new_configuration.c \
	new_configuration.h \
	file_data_access.c \
	file_data_access.h \
	read.c \
	read.h \
	sddf_records.h \
	ts.h 
