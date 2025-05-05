BINARY = mkefivardata

CFLAGS = 
CFLAGS += -Iinclude
CFLAGS += -I/usr/include/efi
CFLAGS += -Wall
CFLAGS += -pedantic-errors

all: $(BINARY).o lib/guid.o lib/kernel_efivars.o
	$(CC) $(CFLAGS) -o $(BINARY) $^

%.o: %.c
	$(CC) $(CFLAGS) -o $@ -c $<

clean:
	rm -f $(BINARY)
	rm -f  *.o
	rm -f lib/*.o
