common_filesdir=@common_filesdir@

nodistdir=`pwd`

INCLUDES = \
 -I$(top_srcdir)/include\
 -I$(common_filesdir)\
 -I$(common_filesdir)/pcfparser

bin_PROGRAMS           = prv2dim

# 'prv2dim' configuration
prv2dim_CFLAGS         = -DNEW_DIMEMAS_TRACE
prv2dim_CXXFLAGS       = -DNEW_DIMEMAS_TRACE
prv2dim_SOURCES        = $(DIST_PROGRAM_SRCS)
nodist_prv2dim_SOURCES = $(COMMON_FILES_SRCS) 


# Common list of sources
DIST_PROGRAM_SRCS = \
	ParaverApplicationDescription.cpp\
	ParaverApplicationDescription.hpp\
	ParaverCommunicator.cpp\
	ParaverCommunicator.hpp\
	ParaverHeader.cpp\
	ParaverHeader.hpp\
	ParaverRecord.cpp\
	ParaverRecord.hpp\
	ParaverTraceParser.cpp\
	ParaverTraceParser.hpp\
	ParaverTraceTranslator.cpp\
	ParaverTraceTranslator.hpp\
	Prv2DimMain.cpp\
	TaskTranslationInfo.cpp\
	TaskTranslationInfo.hpp\
	TranslatorRecord.cpp\
	TranslatorRecord.hpp\
	PCFGeneration.cpp\
	PCFGeneration.hpp\
	PCFdefines.h

# 'common-files' regular dependencies
COMMON_FILES_SRCS = \
	$(common_filesdir)/Dimemas_Generation.c\
    $(common_filesdir)/EventEncoding.c\
    $(common_filesdir)/Dimemas2Prv.c \
	$(common_filesdir)/Error.hpp \
	$(common_filesdir)/Error.cpp 
