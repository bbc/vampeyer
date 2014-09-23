CC=g++
SOURCES=$(filter-out Template.cpp, $(wildcard *.cpp))
OBJECTS=$(SOURCES:.cpp=.so)
CFLAGS=-Wall -shared -fPIC -I../
LDFLAGS=-lcairo

all: $(OBJECTS)

%.so: %.cpp
	$(CC) $(CFLAGS) $< -o $@ $(LDFLAGS)

clean:
	rm -f $(OBJECTS)
