CC = gcc
CFLAGS = -Wall -fPIC
LDFLAGS = -shared

all: libhtml6.so

libhtml6.so: functions.o
	$(CC) $(LDFLAGS) -o libhtml6.so functions.o

functions.o: functions.c functions.h
	$(CC) $(CFLAGS) -c functions.c

main: main.c libhtml6.so
	$(CC) -o main main.c -L. -lhtml6

clean:
	rm -f *.o *.so main
