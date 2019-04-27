CC=$(CORSS_COMPILE)gcc
AR=$(CORSS_COMPILE)ar
LD=$(CORSS_COMPILE)ld

SRCS=test-loop.c
OBJS=$(patsubst %.c,%.o,$(SRCS))

LIB_SRCS=mainloop.c queue.c util.c
LIB_OBJS=$(patsubst %.c,%.o,$(LIB_SRCS))

PROG=threads
LIB=libmainloop.a

LDFLAGS=-lmainloop
LDFLAGS+=-pthread -L.

.PHONY:all clean

all: $(PROG) $(LIB)

$(PROG):$(OBJS) $(LIB)
	$(CC) -o $@ $< $(LDFLAGS)

$(LIB):$(LIB_OBJS)
	$(AR) -rcs $(LIB) $(LIB_OBJS)
	
clean:
	$(RM) $(PROG) $(OBJS) $(LIB_OBJS)
