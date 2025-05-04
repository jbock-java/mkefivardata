BINARIES = efi-updatevar

export TOPDIR	:= $(shell pwd)/

include Make.rules

efiupdate: efi-updatevar

all: $(BINARIES) $(MANPAGES)

install: all
	$(INSTALL) -m 755 -d $(MANDIR)
	$(INSTALL) -m 644 $(MANPAGES) $(MANDIR)
	$(INSTALL) -m 755 -d $(BINDIR)
	$(INSTALL) -m 755 $(BINARIES) $(BINDIR)
	$(INSTALL) -m 755 -d $(DOCDIR)
	$(INSTALL) -m 644 README COPYING $(DOCDIR)

lib/lib.a:
	$(MAKE) -C lib $(notdir $@)

efi-updatevar: efi-updatevar.o lib/lib.a
	$(CC) $(ARCH3264) -o $@ $< lib/lib.a

clean:
	rm -f $(BINARIES) *.o
	rm -f doc/*.1
	$(MAKE) -C lib clean
