CC=g++

PROG=main
SOURCES=VampHost.cpp PNGWriter.cpp GUI.cpp main.cpp
CFLAGS=-c -g -Wall -I/home/chrisbau/builds/vamp-plugin-sdk-2.5
LDFLAGS=-L/home/chrisbau/builds/vamp-plugin-sdk-2.5 -ldl -lpng -lsndfile \
				-lvamp-hostsdk -lfltk
OBJECTS=$(SOURCES:.cpp=.o)

PLUGINS_DIR=plugins
PLUGINS=$(PLUGINS_DIR)/Amplitude.so \
				$(PLUGINS_DIR)/MFCC.so \
				$(PLUGINS_DIR)/AmpMFCC.so \
				$(PLUGINS_DIR)/FreeSound.so \
				$(PLUGINS_DIR)/SMD.so
PLUGINS_CFLAGS=-Wall -shared -fPIC
PLUGINS_LDFLAGS=-lcairo


all: $(OBJECTS) $(PROG) $(PLUGINS)

$(PROG): $(OBJECTS)
	$(CC) -o $@ $(OBJECTS) $(LDFLAGS)

%.o: %.cpp
	$(CC) $(CFLAGS) $< -o $@

$(PLUGINS_DIR)/%.so: $(PLUGINS_DIR)/%.cpp
	$(CC) $(PLUGINS_CFLAGS) $< -o $@ $(PLUGINS_LDFLAGS)

clean:
	rm -f $(OBJECTS) $(PROG) $(PLUGINS)
