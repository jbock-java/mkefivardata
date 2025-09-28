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

test: cert-to-efi-sig-list
	./cert-to-efi-sig-list -g a25e0ad4-9c64-11f0-8e7e-0800279b31a2 fedora.crt fedora.esl
	md5sum fedora.esl | grep -q ^e868d249 && printf "\033[1;32m[OK]\033[0m\n" || printf "\033[1;31m[FAIL]\033[0m\n"

%.o: %.c
	$(CC) $(CFLAGS) -o $@ -c $<

clean:
	rm -f mkefivardata cert-to-efi-sig-list
	rm -f  *.o
	rm -f lib/*.o

install: all
	install -m 755 -d $(BINDIR)
	install -m 755 mkefivardata $(BINDIR)
