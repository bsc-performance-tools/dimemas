noinst_LTLIBRARIES = libDimemasInput.la

INCLUDES = \
	-I@common_filesdir@ \
	-I$(top_srcdir)/include \
	-I$(top_srcdir)/src/io \
	-I$(top_srcdir)/src/input \
	-I$(top_srcdir)/src/model \
	-I$(top_srcdir)/src/prv_utils \
	-I$(top_srcdir)/src/sched \
	-I$(top_srcdir)/src/utils \
	-I$(top_srcdir)/src/venusclient

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
	file_data_access.c \
	file_data_access.h \
	read.c \
	read.h \
	sddf_records.h \
	ts.h 
