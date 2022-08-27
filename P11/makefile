CFLAGS = -Wall
OBJSR = pingpong-racecond.o ppos_core.o queue.o
OBJSS = pingpong-semaphore.o ppos_core.o queue.o

all: racecond semaphore

racecond: $(OBJSR)
	gcc -o pingpong-racecond $(CFLAGS) $(OBJSR)

semaphore: $(OBJSS)
	gcc -o pingpong-semaphore $(CFLAGS) $(OBJSS)

pingpong-racecond.o: pingpong-racecond.c
	gcc $(CFLAGS) -c pingpong-racecond.c

pingpong-semaphore.o: pingpong-semaphore.c
	gcc $(CFLAGS) -c pingpong-semaphore.c

queue.o: queue.c
	gcc $(CFLAGS) -c queue.c

ppos_core.o: ppos_core.c
	gcc $(CFLAGS) -c ppos_core.c

pingpong-sleep.o: pingpong-sleep.c
	gcc $(CFLAGS) -c pingpong-sleep.c

clean:
	-rm -f *~ *.o