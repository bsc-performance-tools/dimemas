#SUBDIRS = \
#	binfmt \
#	input \
#	license \
#	model \
#	prv_utils \
#	sched \
#	utils \
#	vampir 

bin_PROGRAMS = Dimemas3

common_filesdir = @common_filesdir@

Dimemas3_CFLAGS=-I$(top_srcdir) -I$(top_srcdir)/include -I$(common_filesdir)
Dimemas3_CXXFLAGS=-I$(top_srcdir) -I$(top_srcdir)/include -I$(common_filesdir)

Dimemas3_SOURCES = main.c

# BUILT_SOURCES = yacc.c yacc.h lex.c

nodist_Dimemas3_SOURCES = \
  $(common_filesdir)/ParaverBinaryTrace.c\
  $(common_filesdir)/Dimemas2Prv.c \
  $(common_filesdir)/EventEncoding.c
  
include $(srcdir)/input/Makefile.sources
include $(srcdir)/model/Makefile.sources
include $(srcdir)/prv_utils/Makefile.sources
include $(srcdir)/sched/Makefile.sources
include $(srcdir)/utils/Makefile.sources
include $(srcdir)/venusclient/Makefile.sources

