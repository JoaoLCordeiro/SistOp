CFLAGS = -Wall
OBJS1 = pingpong-disco1.o ppos_core.o queue.o disk.o ppos_disk.o
OBJS2 = pingpong-disco2.o ppos_core.o queue.o disk.o ppos_disk.o

all: disco1 disco2

disco1: $(OBJS1)
	gcc -o pingpong-disco1 $(CFLAGS) $(OBJS1) -lrt

disco2: $(OBJS2)
	gcc -o pingpong-disco2 $(CFLAGS) $(OBJS2) -lrt

pingpong-disco1.o: pingpong-disco1.c
	gcc $(CFLAGS) -c pingpong-disco1.c

pingpong-disco2.o: pingpong-disco2.c
	gcc $(CFLAGS) -c pingpong-disco2.c

pingpong-mqueue.o: pingpong-mqueue.c
	gcc $(CFLAGS) -c pingpong-mqueue.c

queue.o: queue.c
	gcc $(CFLAGS) -c queue.c

ppos_core.o: ppos_core.c
	gcc $(CFLAGS) -c ppos_core.c

disk.o: disk.c
	gcc $(CFLAGS) -c disk.c -lrt

ppos_disk.o: ppos_disk.c
	gcc $(CFLAGS) -c ppos_disk.c

clean:
	-rm -f *~ *.o