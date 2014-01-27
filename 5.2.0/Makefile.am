AUTOMAKE_OPTIONS = foreign no-dependencies

SUBDIRS = common_utilities include Simulator prv2dim 

if HAVE_JAVA
SUBDIRS += GUI
endif

if HAVE_BOOST
SUBDIRS += trf2dim
endif

EXTRA_DIST = 

distdir = $(PACKAGE)-$(VERSION)
