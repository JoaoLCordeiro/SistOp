CFLAGS = -Wall
OBJS = pingpong-join.o ppos_core.o queue.o

all: pingpong

pingpong: $(OBJS)
	gcc -o pingpong-join $(CFLAGS) $(OBJS)

queue.o: queue.c
	gcc $(CFLAGS) -c queue.c

ppos_core.o: ppos_core.c
	gcc $(CFLAGS) -c ppos_core.c

pingpong-join.o: pingpong-join.c
	gcc $(CFLAGS) -c pingpong-join.c

clean:
	-rm -f *~ *.o