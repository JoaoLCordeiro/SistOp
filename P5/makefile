CFLAGS = -Wall
OBJS = pingpong-preempcao.o ppos_core.o queue.o
OBJSS = pingpong-preempcao-stress.o ppos_core.o queue.o

all: pingpong

stress: pingpongstress

pingpongstress: $(OBJSS)
	gcc -o pingpongstress $(CFLAGS) $(OBJSS)

pingpong: $(OBJS)
	gcc -o pingpong-preempcao $(CFLAGS) $(OBJS)

queue.o: queue.c
	gcc $(CFLAGS) -c queue.c

ppos_core.o: ppos_core.c
	gcc $(CFLAGS) -c ppos_core.c

pingpong-preempcao-stress.o: pingpong-preempcao-stress.c
	gcc $(CFLAGS) -c pingpong-preempcao-stress.c

pingpong-preempcao.o: pingpong-preempcao.c
	gcc $(CFLAGS) -c pingpong-preempcao.c

clean:
	-rm -f *~ *.o