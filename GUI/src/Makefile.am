JAVAROOT = $(top_builddir)/GUI/src

if HAVE_JAVA
all: $(jarfile) runscript
else
all: 
endif

basejarfile = $(PACKAGE)-gui-$(VERSION).jar
jarfile     = $(top_builddir)/GUI/$(basejarfile)
jardir      = $(libdir)/GUI
jar_DATA    = $(jarfile)

JAVA_HOME=@JAVA_HOME@
JAVAC=@JAVAC@
JAVA=@JAVA@
JAR=@JAR@

include DIST_NOINST_DATA.include
include DIST_NOINST_JAVA.include

# List all the dependencies required by the GUI
DEPENDENCIES=$(top_builddir)/GUI/lib/commons-io-2.4.jar

$(jarfile): $(dist_noinst_JAVA) $(dist_noinst_DATA)
	$(JAR) cfm $@ $(MANIFEST) $(JARFLAGS) $(dist_noinst_DATA) `find . -name "*.class"`
	
$(dist_noinst_JAVA): updateAndCheckJavaFilesList manifest

updateAndCheckJavaFilesList:
	rm $(JAVAROOT)/new_DIST_NOINST_JAVA.include; \
	line="dist_noinst_JAVA = "; \
	for next in `find $(JAVAROOT) -name "*.java" && echo "THE_END"` ; do \
	  if test "$$next" != "THE_END"; then \
	    echo $$line" \\" >> $(JAVAROOT)/new_DIST_NOINST_JAVA.include; \
	    line=`echo "$$next" | sed "s/^.*src\/classes\/\(.*\)/\1 /"` ; \
	  else \
	    echo $$line >> $(JAVAROOT)/new_DIST_NOINST_JAVA.include; \
	  fi; \
	done; \
	diff $(JAVAROOT)/DIST_NOINST_JAVA.include $(JAVAROOT)/new_DIST_NOINST_JAVA.include; \
	if test false = true; then \
	  echo "DIST_NOINST_JAVA.include is too old, replace it with new_DIST_NOINST_JAVA.include and try again."; \
	fi

dist-local: 

clean-local: rmclassfiles

rmclassfiles:
	find $(JAVAROOT) -name "*.class" -exec rm {} \;

MANIFEST=$(JAVAROOT)/MANIFEST.MF

RUNSCRIPT=$(top_builddir)/GUI/DimemasGUI
runscript: DimemasGUI.in
	sed s/@@JARFILE@@/$(basejarfile)/ < DimemasGUI.in > $(RUNSCRIPT)

install-exec-am: $(runscript)

manifest:
	echo "Manifest-Version: 1.0" > $(MANIFEST); \
	echo "Main-Class: es.bsc.cepbatools.dimemas.gui.Main" >> $(MANIFEST); \
	echo "Class-path: commons-io-2.4.jar" >> $(MANIFEST)

JAVACFLAGS = -Xlint -encoding utf8 -cp $(DEPENDENCIES)

CLEANFILES = $(jarfile) $(MANIFEST) $(RUNSCRIPT) new_DIST_NOINST_JAVA.include

install-exec-am: $(jarfile) $(runscript) $(DEPENDENCIES)
	mkdir -p $(libdir)/GUI
	cp $(jarfile) $(libdir)/GUI
	cp $(DEPENDENCIES) $(libdir)/GUI
	chmod a+x $(RUNSCRIPT)
	cp $(RUNSCRIPT) $(bindir)

