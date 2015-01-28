noinst_LTLIBRARIES = libDimemasEEEModel.la
simulator_dir = $(top_srcdir)/Simulator

AM_CPPFLAGS = \
	-I$(top_srcdir)/include \
	-I$(simulator_dir)/io \
	-I$(simulator_dir)/input \
	-I$(simulator_dir)/model \
	-I$(simulator_dir)/prv_utils \
	-I$(simulator_dir)/sched \
	-I$(simulator_dir)/utils \
	-I$(simulator_dir)/venusclient \
	-I$(simulator_dir)/eee_model
  

libDimemasEEEModel_la_SOURCES = \
	eee_communic.c \
	eee_communic.h \
	eee_configuration.c \
	eee_configuration.h

