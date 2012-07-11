## Process this file with automake to produce Makefile.in
common_filesdir=@common_filesdir@

INCLUDES = \
	-I$(top_srcdir)/include \
	-I$(common_filesdir)

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

nodist_trf2dim_SOURCES = \
	$(common_filesdir)/Dimemas_Generation.c\
    $(common_filesdir)/EventEncoding.c\
    $(common_filesdir)/Dimemas2Prv.c \
    $(common_filesdir)/Error.hpp \
    $(common_filesdir)/Error.cpp
