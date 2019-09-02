# To test the build of the assignment, just type "make".
# But you can also make testparsemessage, testpeer, readexample, trickyclient.

CC = gcc
CFLAGS = -Wall -g

yak: yak.o parsemessage.o peer.o util.o
	gcc -g -o yak yak.o parsemessage.o peer.o util.o

yak.o: yak.c peer.h parsemessage.h util.h
parsemessage.o: parsemessage.c parsemessage.h
peer.o: peer.c peer.h
util.o: util.c util.h

testparsemessage: testparsemessage.c parsemessage.o parsemessage.h
	gcc -o testparsemessage testparsemessage.c parsemessage.o

testpeer: testpeer.c peer.o peer.h
	gcc -o testpeer testpeer.c peer.o

readexample: readexample.c
	gcc -Wall -o readexample readexample.c

trickyclient: trickyclient.c
	gcc -Wall -o trickyclient trickyclient.c

clean:
	rm -f yak yak.o parsemessage.o peer.o util.o testparsemessage testpeer readexample
