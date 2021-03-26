include makefile.inc

PROCESS_SOURCES=master.c
PROCESS_OBJECTS=$(PROCESS_SOURCES:.c=.o)

SLAVE_SOURCES=slave.c
SLAVE_OBJECTS=$(SLAVE_SOURCES:.c=.o)

COMMUNICATION_SOURCES=communication/pipe.c
COMMUNICATION_OBJECTS=$(COMMUNICATION_SOURCES:.c=.o)

MASTER_OUT = master
SLAVE_OUT = slave

all: $(MASTER_OUT) $(SLAVE_OUT) bin_move

$(MASTER_OUT): $(COMMUNICATION_OBJECTS) $(PROCESS_OBJECTS)

$(SLAVE_OUT): $(SLAVE_OBJECTS)

%.o: %.c
	$(CC) $(CFLAGS) -I./include -c $< -o $@
	mkdir -p bin

bin_move:
	mv $(MASTER_OUT) bin/
	mv $(SLAVE_OUT) bin/
	mv *.o bin/

clean:
	rm -rf *.o bin/
	rm -rf communication/*.o
.PHONY: all clean