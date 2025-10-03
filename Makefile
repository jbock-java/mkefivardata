BINARY = mkefivardata
BINDIR = $(DESTDIR)/usr/bin

CFLAGS += -I/usr/include/efi
CFLAGS += -Iinclude
CFLAGS += -Wall
CFLAGS += -pedantic-errors
CFLAGS += -Werror=vla
CFLAGS += -D_XOPEN_SOURCE=700

TEST_OWNER = a25e0ad4-9c64-11f0-8e7e-0800279b31a2

mkefivardata: mkefivardata.c
	$(CC) $(CFLAGS) -o mkefivardata $^

cert-to-efi-sig-list: cert-to-efi-sig-list.c
	$(CC) $(CFLAGS) -o cert-to-efi-sig-list $^ -lcrypto

sign-efi-sig-list: sign-efi-sig-list.c lib/openssl_sign.c
	$(CC) $(CFLAGS) -o sign-efi-sig-list $^ -lcrypto

esltest: cert-to-efi-sig-list
	rm -f fedora.esl
	./cert-to-efi-sig-list -g $(TEST_OWNER) fedora.crt fedora.esl
	md5sum fedora.esl | grep -q ^e868d249 && printf "\033[1;32m[OK]\033[0m\n"

sigtest: sign-efi-sig-list
	./sign-efi-sig-list -g $(TEST_OWNER) -t "2025-03-24 14:26:01" -k KEK.key -c KEK.crt db fedora.esl fedora.auth
	md5sum fedora.auth | grep -q ^960a4d050 && printf "\033[1;32m[OK]\033[0m\n"

efitest: mkefivardata
	./mkefivardata -q fedora.auth fedora.vardata
	md5sum fedora.vardata | grep -q ^c0c07e6b0 && printf "\033[1;32m[OK]\033[0m\n"

expand:
	$(CC) -E $(CFLAGS) sign-efi-sig-list.c

clean:
	rm -f mkefivardata cert-to-efi-sig-list sign-efi-sig-list

install: all
	install -m 755 -d $(BINDIR)
	install -m 755 mkefivardata $(BINDIR)
