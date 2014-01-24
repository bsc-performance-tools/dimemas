JAVAROOT = .

jarfile = $(PACKAGE)-gui-$(VERSION).jar
runscript = DimemasGUI
jardir  = $(bindir)
jhdrout = ..
gui_srcdir = $(top_srcdir)/GUI

jar_DATA = $(jarfile)

JAVACFLAGS = -Xlint -encoding utf8

if HAVE_JAVA
all: $(jarfile) $(runscript)
else
all: 
endif

$(runscript): DimemasGUI.template
	sed s/@@JARFILE@@/$(jarfile)/ < DimemasGUI.template >> DimemasGUI

CLEANFILES = $(jarfile) $(runscript) 

dist_noinst_JAVA = Main.java
include data/Makefile.sources
include gui/Makefile.sources
include tools/Makefile.sources
# Not needed, as the 'records_definition.txt' and 'icon.png' are now included
# in the JAR file
#include defaults/Makefile.sources


dist_noinst_DATA = manifest.mf DimemasGUI.template

# The sleep command is to guarantee the compilation before the packaging

$(jarfile): $(dist_noinst_JAVA) manifest.mf
	sleep 5 && $(JAR) cvfm $(jarfile) manifest.mf $(srcdir)/data/*.class $(srcdir)/gui/*.class $(srcdir)/tools/*.class $(srcdir)/resources/icon.jpg $(srcdir)/resources/records_definition.txt *.class 

install-data-hook: $(jarfile) $(runscript)
	chmod a+x $(runscript)
	cp $(runscript) $(bindir)
	cp $(jarfile) $(bindir)


