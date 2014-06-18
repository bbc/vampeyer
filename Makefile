CC=g++

PROG=vampeyer
PREFIX=/usr/local
SOURCES=VampHost.cpp VisHost.cpp PNGWriter.cpp GUI.cpp Vampeyer.cpp
CFLAGS=-c -g -Wall
LDFLAGS=-ldl -lpng -lsndfile -lvamp-hostsdk -lfltk
OBJECTS=$(SOURCES:.cpp=.o)

all: $(PROG)

$(PROG): $(OBJECTS)
	$(CC) -o $@ $(OBJECTS) $(LDFLAGS)

%.o: %.cpp
	$(CC) $(CFLAGS) $< -o $@

clean:
	rm -f $(OBJECTS) $(PROG)

install: all
	install -m 0755 $(PROG) $(PREFIX)/bin
