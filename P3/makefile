CFLAGS = -Wall
OBJS = pingpong-dispatcher.o ppos_core.o queue.o

all: pingpong

pingpong: $(OBJS)
	gcc -o pingpong-dispatcher $(CFLAGS) $(OBJS)

queue.o: queue.c
	gcc $(CFLAGS) -c queue.c

ppos_core.o: ppos_core.c
	gcc $(CFLAGS) -c ppos_core.c

pingpong-dispatcher.o: pingpong-dispatcher.c
	gcc $(CFLAGS) -c pingpong-dispatcher.c

clean:
	-rm -f *~ *.o