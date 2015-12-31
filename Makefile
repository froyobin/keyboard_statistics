all:Keyinput

CC=g++
CPPFLAGS=-Wall -std=c++11 -ggdb
LDFLAGS=-pthread

Keyinput:Keyinput.o
	    $(CC) $(LDFLAGS) -o $@ $^

Keyinput.o:Keyinput.cpp
	    $(CC) $(CPPFLAGS) -o $@ -c $^


.PHONY:
	    clean

clean:
	    rm Keyinput.o Keyinput
