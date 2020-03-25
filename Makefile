TARGET   = nte
CC       = gcc
CCFLAGS  = -std=c99 -pedantic -Wall -Werror
LDFLAGS  = -lncurses
SOURCES  = nte.c
INCLUDES = $(wildcard *.h)
OBJECTS  = $(SOURCES:.c=.o)

all:$(TARGET)

$(TARGET):$(OBJECTS)
	$(CC) -o $(TARGET) $(OBJECTS) $(LDFLAGS)

$(OBJECTS):$(SOURCES) $(INCLUDES)
	$(CC) -c $(CCFLAGS) $(SOURCES)

clean:
	rm -f $(TARGET) $(OBJECTS)
