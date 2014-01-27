noinst_LTLIBRARIES = libDimemasVenusClient.la

simulator_dir = $(top_srcdir)/Simulator

AM_CPPFLAGS = \
	-I$(top_srcdir)/include \
	-I$(simulator_dir)/io \
	-I$(simulator_dir)/input \
	-I$(simulator_dir)/model \
	-I$(simulator_dir)/prv_utils \
	-I$(simulator_dir)/sched \
	-I$(simulator_dir)/utils \
	-I$(simulator_dir)/venusclient

if VENUS_ENABLED
libDimemasVenusClient_la_SOURCES = \
	ClientSocket.cc \
	ClientSocket.h \
	ServerSocket.cc \
	ServerSocket.h \
	Socket.cc \
	SocketException.h \
	Socket.h \
	venusclient.cc \
	venusclient.h
endif
