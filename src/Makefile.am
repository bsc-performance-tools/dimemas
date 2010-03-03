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

noinst_HEADERS = 
noinst_SRCS =
nodist_HEADERS =

noinstdir = $(srcdir)/.tmp
nodistdir = $(srcdir)/.tmp

include $(srcdir)/input/Makefile.sources
include $(srcdir)/model/Makefile.sources
include $(srcdir)/prv_utils/Makefile.sources
include $(srcdir)/sched/Makefile.sources
include $(srcdir)/utils/Makefile.sources

Dimemas3_OBJS = \
  $(input_OBJS) \
  $(model_OBJS) \
  $(prv_utils_OBJS) \
  $(sched_OBJS) \
  $(utils_OBJS) \
  main.o

Dimemas3_INCLUDES = \
  -I$(top_srcdir)/include \
  -I$(top_srcdir)/../common-files \
  $(binfmt_INCLUDES) \
  $(input_INCLUDES) \
  $(model_INCLUDES) \
  $(prv_utils_INCLUDES) \
  $(sched_INCLUDES) \
  $(utils_INCLUDES) 

Dimemas3_SOURCES = \
	$(input_SRCS) \
    $(nodist_input_SRCS) \
	$(model_SRCS) \
	$(prv_utils_SRCS) \
	$(sched_SRCS) \
	$(utils_SRCS) \
	main.c

#noinst_HEADERS = \
#	$(input_noinst_HEADERS) \
#	$(model_noinst_HEADERS) \
#	$(prv_utils_noinst_HEADERS) \
#	$(sched_noinst_HEADERS) \
#	$(utils_noinst_HEADERS) \
#	$(venusclient_noinst_HEADERS)

#nodist_HEADERS = \
#	$(input_nodist_HEADERS))

if VENUS_ENABLED
  include $(srcdir)/venusclient/Makefile.sources
  Dimemas3_OBJS += $(venusclient_OBJS)
  Dimemas3_INCLUDES += $(venusclient_INCLUDES) 
  Dimemas3_SOURCES += $(venusclient_CPPSRCS)
endif

DEFS = @arch_defines@ @DEFS@ 

$(top_srcdir)/input/yacc.o: $(top_srcdir)/input/yacc.c
	$(CC) $(CFLAGS) $(Dimemas3_INCLUDES) $(DEFS) -o $@ -c $<

%.o:%.c 
	$(CC) $(CFLAGS) $(Dimemas3_INCLUDES) $(DEFS) -o $@ -c $<

%.o:%.cc
	$(CXX) $(CXXFLAGS) $(Dimemas3_INCLUDES) $(DEFS) -o $@ -c $<

LIBS = -L. $(LEXLIB) -lm $(Z_LIBS)

Dimemas3$(EXEEXT): $(Dimemas3_OBJS) 
	$(CXX) $(LDFLAGS) $(Dimemas3_INCLUDES) -o Dimemas3$(EXEEXT) $(Dimemas3_OBJS) $(LIBS) 

clean: 
	rm -rf Dimemas3$(EXEEXT) $(Dimemas3_OBJS) $(input_TRASH)

#main.o: main.c
#	$(CC) $(CFLAGS) -o main.o -c main.c $(Dimemas_INCLUDES) $(DEFS)
	
