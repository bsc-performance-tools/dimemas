CC          = cc
RM          = rm
TARGET      = libextern_comm_model.so

all: $(TARGET)

INCLUDE_DIRS=-I../../include

libextern_comm_model.so: extern_comm_model.o
	$(CC) -shared extern_comm_model.o -o libextern_comm_model.so
	$(RM) extern_comm_model.o

extern_comm_model.o: extern_comm_model.c 
	$(CC) -fPIC $(INCLUDE_DIRS) -c extern_comm_model.c

clean:
	$(RM) *.so *.o

