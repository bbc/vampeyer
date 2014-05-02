CC=g++
VAMP_SDK=~/builds/vamp-plugin-sdk-2.5

PROG=main
SOURCES=VampHost.cpp VisHost.cpp PNGWriter.cpp GUI.cpp main.cpp
CFLAGS=-c -g -Wall -I$(VAMP_SDK)
LDFLAGS=-L$(VAMP_SDK) -ldl -lpng -lsndfile -lvamp-hostsdk -lfltk
OBJECTS=$(SOURCES:.cpp=.o)

PLUGINS_DIR=plugins
PLUGINS_SRC=$(wildcard $(PLUGINS_DIR)/*.cpp)
PLUGINS_OBJ=$(PLUGINS_SRC:.cpp=.so)
PLUGINS_CFLAGS=-Wall -shared -fPIC
PLUGINS_LDFLAGS=-lcairo

all: $(PROG) $(PLUGINS_OBJ)

$(PROG): $(OBJECTS)
	$(CC) -o $@ $(OBJECTS) $(LDFLAGS)

%.o: %.cpp
	$(CC) $(CFLAGS) $< -o $@

$(PLUGINS_DIR)/%.so: $(PLUGINS_DIR)/%.cpp
	$(CC) $(PLUGINS_CFLAGS) $< -o $@ $(PLUGINS_LDFLAGS)

clean:
	rm -f $(OBJECTS) $(PROG) $(PLUGINS_OBJ)
