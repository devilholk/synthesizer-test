CC = gcc
CFLAGS = -Wall -O2 -march=native -std=gnu99 -fPIC -g
LDFLAGS =  -lpython2.7 -lm -shared
INCLUDES = -I/usr/include/python2.7 -I../inc
SOURCES = synthesizer.c
OBJECTS = $(SOURCES:.c=.o)
EXECUTABLE = test.so

all: $(SOURCES) $(EXECUTABLE)

$(EXECUTABLE): $(OBJECTS) 
	$(CC) $(OBJECTS) -o $@ $(LDFLAGS)

.c.o:
	$(CC) $(INCLUDES) $(CFLAGS) -c $< -o $@

clean:
	rm $(OBJECTS)
	rm $(EXECUTABLE)
