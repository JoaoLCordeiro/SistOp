CFLAGS = -Wall
OBJS = pingpong-contab.o ppos_core.o queue.o
OBJS2 = pingpong-contab-prio.o ppos_core.o queue.o

all: pingpong

prio: pingpong-prio

pingpong: $(OBJS)
	gcc -o pingpong-contab $(CFLAGS) $(OBJS)

pingpong-prio: $(OBJS2)
	gcc -o pingpong-prio $(CFLAGS) $(OBJS2)

pingpong-contab-prio.o: pingpong-contab-prio.c
	gcc $(CFLAGS) -c pingpong-contab-prio.c

queue.o: queue.c
	gcc $(CFLAGS) -c queue.c

ppos_core.o: ppos_core.c
	gcc $(CFLAGS) -c ppos_core.c

pingpong-contab.o: pingpong-contab.c
	gcc $(CFLAGS) -c pingpong-contab.c

clean:
	-rm -f *~ *.o