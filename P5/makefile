CFLAGS = -Wall
OBJS = pingpong-preempcao.o ppos_core.o queue.o

all: pingpong

pingpong: $(OBJS)
	gcc -o pingpong-preempcao $(CFLAGS) $(OBJS)

queue.o: queue.c
	gcc $(CFLAGS) -c queue.c

ppos_core.o: ppos_core.c
	gcc $(CFLAGS) -c ppos_core.c

pingpong-preempcao.o: pingpong-preempcao.c
	gcc $(CFLAGS) -c pingpong-preempcao.c

clean:
	-rm -f *~ *.o