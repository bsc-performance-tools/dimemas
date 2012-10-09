nodistdir=`pwd`

INCLUDES = \
	-I$(top_srcdir)/common_utilities\
	-I$(top_srcdir)/include

bin_PROGRAMS           = prv2dim

# 'prv2dim' configuration
prv2dim_CFLAGS         = -DNEW_DIMEMAS_TRACE
prv2dim_CXXFLAGS       = -DNEW_DIMEMAS_TRACE

# Common list of sources
prv2dim_SOURCES = \
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

prv2dim_LDADD = \
	$(top_srcdir)/common_utilities/libCommonUtilities.la

# 'common-files' regular dependencies
# COMMON_FILES_SRCS = \
#	$(common_filesdir)/Dimemas_Generation.c\
#    $(common_filesdir)/EventEncoding.c\
#    $(common_filesdir)/Dimemas2Prv.c \
#	$(common_filesdir)/Error.hpp \
#	$(common_filesdir)/Error.cpp 
