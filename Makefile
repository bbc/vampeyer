CC=g++

PROG=main
SOURCES=main.cpp
CFLAGS=-c -Wall
LDFLAGS=-ldl
OBJECTS=$(SOURCES:.cpp=.o)

PLUGINS_DIR=plugins
PLUGINS=$(PLUGINS_DIR)/TestPlugin.so
PLUGINS_CFLAGS=-Wall -shared -fPIC


all: $(OBJECTS) $(PROG) $(PLUGINS)

$(PROG): $(OBJECTS)
	$(CC) -o $@ $(OBJECTS) $(LDFLAGS)

%.o: %.cpp
	$(CC) $(CFLAGS) $< -o $@

$(PLUGINS_DIR)/%.so: $(PLUGINS_DIR)/%.cpp
	$(CC) $(PLUGINS_CFLAGS) $< -o $@

clean:
	rm -f $(OBJECTS) $(PROG) $(PLUGINS)
