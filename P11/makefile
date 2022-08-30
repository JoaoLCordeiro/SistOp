CFLAGS = -Wall
OBJS = pingpong-prodcons.o ppos_core.o queue.o

all: prodcons

prodcons: $(OBJS)
	gcc -o pingpong-prodcons $(CFLAGS) $(OBJS)

pingpong-prodcons.o: pingpong-prodcons.c
	gcc $(CFLAGS) -c pingpong-prodcons.c

queue.o: queue.c
	gcc $(CFLAGS) -c queue.c

ppos_core.o: ppos_core.c
	gcc $(CFLAGS) -c ppos_core.c

pingpong-sleep.o: pingpong-sleep.c
	gcc $(CFLAGS) -c pingpong-sleep.c

clean:
	-rm -f *~ *.o