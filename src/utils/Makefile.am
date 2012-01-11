noinst_LTLIBRARIES = libDimemasUtils.la

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

libDimemasUtils_la_SOURCES = \
	aleatorias.c \
	aleatorias.h \
	cp.c \
	cp.h \
	list.c \
	list.h \
	mallocame.c \
	mallocame.h \
	random.c \
	random.h \
	subr.c \
	subr.h \
	listE.h \
	listE.cc \
	modules_map.h \
	modules_map.cc 

#if USE_EQUEUE
#libDimemasUtils_la_SOURCES += \
#	listE.h \
#	listE.cc
#endif
