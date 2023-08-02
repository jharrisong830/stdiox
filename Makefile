#
#  NAME:    John Graham
#  PLEDGE:  I pledge my honor that I have abided by the Stevens Honor System.
#  
#  FILE: Makefile
#


CC = gcc
CFLAGS = -c -g


all: stdiox clean

stdiox: stdiox.o
	$(CC) stdiox.o -o stdiox

stdiox.o: stdiox.c
	$(CC) $(CFLAGS) stdiox.c

clean:
	rm *.o