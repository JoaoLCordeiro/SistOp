CFLAGS = -Wall
OBJS = pingpong-sleep.o ppos_core.o queue.o

all: pingpong

pingpong: $(OBJS)
	gcc -o pingpong-sleep $(CFLAGS) $(OBJS)

queue.o: queue.c
	gcc $(CFLAGS) -c queue.c

ppos_core.o: ppos_core.c
	gcc $(CFLAGS) -c ppos_core.c

pingpong-sleep.o: pingpong-sleep.c
	gcc $(CFLAGS) -c pingpong-sleep.c

clean:
	-rm -f *~ *.o