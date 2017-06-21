CC = gcc
CFLAGS = -g -Wall -Wextra -I -pthread.
PROG = s-talk
OBJS = main.o list.o

run: $(OBJS)
	$(CC) $(CFLAGS) -o $(PROG) $(OBJS)

.c.o:
	$(CC) $(CFLAGS) -c $*.c

clean:
	rm *.o s-talk