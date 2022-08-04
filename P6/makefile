CFLAGS = -Wall
OBJS = pingpong-contab.o ppos_core.o queue.o

all: pingpong

pingpong: $(OBJS)
	gcc -o pingpong-contab $(CFLAGS) $(OBJS)

queue.o: queue.c
	gcc $(CFLAGS) -c queue.c

ppos_core.o: ppos_core.c
	gcc $(CFLAGS) -c ppos_core.c

pingpong-contab.o: pingpong-contab.c
	gcc $(CFLAGS) -c pingpong-contab.c

clean:
	-rm -f *~ *.o