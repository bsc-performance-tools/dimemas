AUTOMAKE_OPTIONS = foreign no-dependencies

SUBDIRS = include src 

if HAVE_JAVA
SUBDIRS += DimemasGUI
endif

EXTRA_DIST = SVNREV

distdir = $(PACKAGE)-$(VERSION).$(SVNREV)
