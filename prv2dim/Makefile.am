nodistdir=`pwd`

AM_CPPFLAGS = \
	-I$(top_srcdir)/common_utilities\
	-I$(top_srcdir)/include

bin_PROGRAMS           = prv2dim

# 'prv2dim' configuration
prv2dim_CFLAGS         = -DNEW_DIMEMAS_TRACE $(BOOST_CPPFLAGS) 
prv2dim_CXXFLAGS       = -DNEW_DIMEMAS_TRACE $(BOOST_CPPFLAGS) 

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
	PCFGeneration.hpp

prv2dim_LDADD = \
	$(top_srcdir)/common_utilities/libCommonUtilities.la \
	-L$(BOOST_LDPATH) -lboost_program_options
