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
include defaults/Makefile.sources

dist_noinst_DATA = manifest.mf

# The sleep command is to guarantee the compilation before the packaging

$(jarfile): $(dist_noinst_JAVA) manifest.mf
	sleep 5 && $(JAR) cvfm $(jarfile) manifest.mf $(srcdir)/data/*.class $(srcdir)/gui/*.class $(srcdir)/tools/*.class *.class

install-data-hook: $(jarfile) $(runscript)
	chmod u+x $(runscript)
	cp $(runscript) $(bindir)
	cp $(jarfile) $(bindir)


