JAVAROOT = .

jarfile = $(PACKAGE)-gui-$(VERSION).jar
runscript = DimemasGUI
jardir  = $(libdir)/GUI
jhdrout = ..
gui_srcdir = $(top_srcdir)/GUI

jar_DATA = $(jarfile)

# List all the dependencies required by the GUI
DEPENDENCIES=$(srcdir)/lib/commons-io-2.4.jar

JAVACFLAGS = -Xlint -encoding utf8 -cp $(DEPENDENCIES)

if HAVE_JAVA
all: $(jarfile) $(runscript)
else
all: 
endif

$(runscript): DimemasGUI.in
	sed s/@@JARFILE@@/$(jarfile)/ < DimemasGUI.in > DimemasGUI

CLEANFILES = $(jarfile) $(runscript) 

dist_noinst_JAVA = Main.java 
include data/Makefile.sources
include gui/Makefile.sources
include tools/Makefile.sources

# Not needed, as the 'records_definition.txt' and 'icon.png' are now included
# in the JAR file
#include defaults/Makefile.sources


dist_noinst_DATA = manifest.mf DimemasGUI.in resources/bsc_logo.jpg resources/dimemas_icon.png resources/records_definition.txt lib/commons-io-2.4.jar

# The sleep command is to guarantee the compilation before the packaging

$(jarfile): $(dist_noinst_JAVA) manifest.mf
	sleep 5 && $(JAR) cvfm $(jarfile) manifest.mf $(srcdir)/data/*.class $(srcdir)/gui/*.class $(srcdir)/tools/*.class $(srcdir)/resources/dimemas_icon.png $(srcdir)/resources/bsc_logo.jpg $(srcdir)/resources/records_definition.txt *.class 

install-exec-am: $(jarfile) $(runscript)
	chmod a+x $(runscript)
	cp $(runscript) $(bindir)
	mkdir -p $(libdir)/GUI
	cp $(jarfile) $(libdir)/GUI
	cp $(DEPENDENCIES) $(libdir)/GUI

