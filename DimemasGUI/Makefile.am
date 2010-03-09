JAVAROOT = .

jarfile = $(PACKAGE)-$(VERSION)-gui.jar
runscript = dimemas-gui.sh
jardir  = $(bindir)
jhdrout = ..

#$(jarfile): $(dist_noinst_JAVA) manifest.mf
#	$(JAR) $(JARFLAGS) $@ $(JAVAROOT)/*.class

jar_DATA = $(jarfile)

JFLAGS = -Xlint -encoding utf8

#java-headers.stamp: $(dist_noinst_JAVA)
#	@list="`echo $(dist_noinst_JAVA) |\
#	sed -e 's|\.java||g' -e 's|/|.|g'`"; \
#	for class in $$list; do \
#		echo "$(JAVAH) -jni -d $(jhdrout)\
#		$(JAVAHFLAGS) $$class"; \
#		$(JAVAH) -jni -d $(jhdrout)\
#		$(JAVAHFLAGS) $$class; \
#	done
#	@echo "JNI headers generated"\
#	> java-headers.stamp

if HAVE_JAVA
all: $(jarfile) $(runscript)
else
all: 
endif

DIMEMAS_HOME_TEXT="DIMEMAS_HOME"

$(runscript):
	echo "#!/bin/sh"                                           > $(runscript)
	echo "export DIMEMAS_HOME=$(prefix)"                      >> $(runscript)
	echo ""                                                   >> $(runscript)
	echo "$(JAVA) -jar -DDIMEMAS_HOME=$(prefix) $$$(DIMEMAS_HOME_TEXT)/bin/$(jarfile)" >> $(runscript)

CLEANFILES = $(jarfile) $(runscript) 

dist_noinst_JAVA = Main.java
include data/Makefile.sources
include gui/Makefile.sources
include tools/Makefile.sources
include defaults/Makefile.sources

dist_noinst_DATA = manifest.mf

$(jarfile): $(dist_noinst_JAVA) manifest.mf
	$(JAR) cvfm $(jarfile) manifest.mf $(srcdir)/data/*.class $(srcdir)/gui/*.class $(srcdir)/tools/*.class *.class

install-data-hook: $(jarfile) $(runscript)
	chmod u+x $(runscript)
	cp $(runscript) $(bindir)
	cp $(jarfile) $(bindir)


