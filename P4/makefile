CFLAGS = -Wall
OBJS = pingpong-scheduler.o ppos_core.o queue.o

all: pingpong

pingpong: $(OBJS)
	gcc -o pingpong-scheduler $(CFLAGS) $(OBJS)

queue.o: queue.c
	gcc $(CFLAGS) -c queue.c

ppos_core.o: ppos_core.c
	gcc $(CFLAGS) -c ppos_core.c

pingpong-scheduler.o: pingpong-scheduler.c
	gcc $(CFLAGS) -c pingpong-scheduler.c

clean:
	-rm -f *~ *.o