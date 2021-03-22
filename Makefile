include Makefile.inc

SOURCES=$(wildcard *.c)
OBJECTS=$(SOURCES:.c=.o)
OUT = solve

all: $(OUT)

$(OUT): $(OBJECTS)

%.o: %.c
	$(CC) $(CFLAGS) -I./include -c $< -o $@

clean:
	rm -rf *.o solve
.PHONY: all clean