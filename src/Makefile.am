#SUBDIRS = \
#	binfmt \
#	input \
#	license \
#	model \
#	prv_utils \
#	sched \
#	utils \
#	vampir 

bin_PROGRAMS = Dimemas3

# De estos subdirectorios tan solo necesitamos los objetos (no se genera una
# libreria o un binario)

if VENUS_ENABLED
include $(srcdir)/binfmt/Makefile.sources
include $(srcdir)/input/Makefile.sources
include $(srcdir)/license/Makefile.sources
include $(srcdir)/model/Makefile.sources
include $(srcdir)/prv_utils/Makefile.sources
include $(srcdir)/sched/Makefile.sources
include $(srcdir)/utils/Makefile.sources
include $(srcdir)/vampir/Makefile.sources
include $(srcdir)/venusclient/Makefile.sources
else
include $(srcdir)/binfmt/Makefile.sources
include $(srcdir)/input/Makefile.sources
include $(srcdir)/license/Makefile.sources
include $(srcdir)/model/Makefile.sources
include $(srcdir)/prv_utils/Makefile.sources
include $(srcdir)/sched/Makefile.sources
include $(srcdir)/utils/Makefile.sources
include $(srcdir)/vampir/Makefile.sources
endif


if VENUS_ENABLED
Dimemas3_OBJS = \
  $(binfmt_OBJS) \
  $(input_OBJS) \
  $(license_OBJS) \
  $(model_OBJS) \
  $(prv_utils_OBJS) \
  $(sched_OBJS) \
  $(utils_OBJS) \
  $(venusclient_OBJS) \
  $(vampir_OBJS) \
  main.o
else
Dimemas3_OBJS = \
  $(binfmt_OBJS) \
  $(input_OBJS) \
  $(license_OBJS) \
  $(model_OBJS) \
  $(prv_utils_OBJS) \
  $(sched_OBJS) \
  $(utils_OBJS) \
  $(vampir_OBJS) \
  main.o
endif

if VENUS_ENABLED
Dimemas3_INCLUDES = \
  -I$(top_srcdir)/include \
  -I$(top_srcdir)/../../common-files \
  $(binfmt_INCLUDES) \
  $(input_INCLUDES) \
  $(license_INCLUDES) \
  $(model_INCLUDES) \
  $(prv_utils_INCLUDES) \
  $(sched_INCLUDES) \
  $(utils_INCLUDES) \
  $(venusclient_INCLUDES) \
  $(vampir_INCLUDES)
else
Dimemas3_INCLUDES = \
  -I$(top_srcdir)/include \
  -I$(top_srcdir)/../../common-files \
  $(binfmt_INCLUDES) \
  $(input_INCLUDES) \
  $(license_INCLUDES) \
  $(model_INCLUDES) \
  $(prv_utils_INCLUDES) \
  $(sched_INCLUDES) \
  $(utils_INCLUDES) \
  $(vampir_INCLUDES)
endif
DEFS = @arch_defines@ @DEFS@ 

$(top_srcdir)/input/yacc.o: $(top_srcdir)/input/yacc.c
	$(CC) $(CFLAGS) $(Dimemas3_INCLUDES) $(DEFS) -o $@ -c $<

%.o: %.c 
	$(CC) $(CFLAGS) $(Dimemas3_INCLUDES) $(DEFS) -o $@ -c $<

%.o: %.cc
	$(CXX) $(CXXFLAGS) $(Dimemas3_INCLUDES) $(DEFS) -o $@ -c $<

#INCLUDES = -I$(top_srcdir)/include $(dimemas_inc)


LIBS = -L. $(LEXLIB) -lm $(Z_LIBS)

Dimemas3_SOURCES = \
	input/check.c\
	main.c 

Dimemas3$(EXEEXT): $(Dimemas3_OBJS) 
	$(CXX) $(LDFLAGS) $(Dimemas3_INCLUDES) -o Dimemas3$(EXEEXT) $(Dimemas3_OBJS) $(LIBS) 

clean: 
	rm -rf Dimemas3$(EXEEXT) $(Dimemas3_OBJS) $(input_TRASH)

#main.o: main.c
#	$(CC) $(CFLAGS) -o main.o -c main.c $(Dimemas_INCLUDES) $(DEFS)
	
