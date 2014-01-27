## Process this file with automake to produce Makefile.in

AM_CPPFLAGS = \
	-I$(top_srcdir)/common_utilities \
	-I$(top_srcdir)/include 

bin_PROGRAMS = trf2dim

trf2dim_CFLAGS   = -DNEW_DIMEMAS_TRACE
trf2dim_CXXFLAGS = -DNEW_DIMEMAS_TRACE

trf2dim_SOURCES = \
	ApplicationDescription.cpp\
	ApplicationDescription.hpp\
	Communicator.hpp\
	DimemasRecord.cpp\
	DimemasRecord.hpp\
	DimemasSDDFTraceParser.cpp\
	DimemasSDDFTraceParser.hpp\
	DimemasSDDFTranslator.cpp\
	DimemasSDDFTranslator.hpp\
	SDDFCommunicator.cpp\
	PCFGenerator.hpp\
	PCFGenerator.cpp\
	Trf2DimMain.cpp

trf2dim_LDADD = \
	$(top_srcdir)/common_utilities/libCommonUtilities.la

