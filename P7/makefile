CFLAGS = -Wall
OBJS = pingpong-maintask.o ppos_core.o queue.o

all: pingpong

pingpong: $(OBJS)
	gcc -o pingpong-maintask $(CFLAGS) $(OBJS)

queue.o: queue.c
	gcc $(CFLAGS) -c queue.c

ppos_core.o: ppos_core.c
	gcc $(CFLAGS) -c ppos_core.c

pingpong-maintask.o: pingpong-maintask.c
	gcc $(CFLAGS) -c pingpong-maintask.c

clean:
	-rm -f *~ *.o