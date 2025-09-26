BINARY = mkefivardata
BINDIR = $(DESTDIR)/usr/bin

CFLAGS = 
CFLAGS += -I/usr/include/efi
CFLAGS += -Iinclude
CFLAGS += -Wall
CFLAGS += -pedantic-errors

mkefivardata: mkefivardata.o
	$(CC) $(CFLAGS) -o mkefivardata $^

cert-to-efi-sig-list: cert-to-efi-sig-list.o lib/guid.o
	$(CC) $(CFLAGS) -o cert-to-efi-sig-list -lcrypto $^ 

%.o: %.c
	$(CC) $(CFLAGS) -o $@ -c $<

clean:
	rm -f mkefivardata cert-to-efi-sig-list
	rm -f  *.o
	rm -f lib/*.o

install: all
	install -m 755 -d $(BINDIR)
	install -m 755 mkefivardata $(BINDIR)
