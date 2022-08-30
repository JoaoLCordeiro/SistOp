CFLAGS = -Wall
OBJS = pingpong-mqueue.o ppos_core.o queue.o

all: mqueue

mqueue: $(OBJS)
	gcc -o pingpong-mqueue $(CFLAGS) $(OBJS)

pingpong-mqueue.o: pingpong-mqueue.c
	gcc $(CFLAGS) -c pingpong-mqueue.c -lm

queue.o: queue.c
	gcc $(CFLAGS) -c queue.c

ppos_core.o: ppos_core.c
	gcc $(CFLAGS) -c ppos_core.c

pingpong-sleep.o: pingpong-sleep.c
	gcc $(CFLAGS) -c pingpong-sleep.c

clean:
	-rm -f *~ *.o