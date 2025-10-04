# mkefivardata

[Upstream](https://git.kernel.org/pub/scm/linux/kernel/git/jejb/efitools.git)

[Services - Runtime Services](https://uefi.org/specs/UEFI/2.11/08_Services_Runtime_Services.html)

[Secure Boot and Driver Signing](https://uefi.org/specs/UEFI/2.11/32_Secure_Boot_and_Driver_Signing.html)

[kernel efivarfs](https://www.kernel.org/doc/html/latest/filesystems/efivarfs.html)

To enroll signed secureboot keys, aka `.auth` files, [efitools is needed](https://github.com/Foxboron/sbctl/issues/434). The purpose of `mkefivardata` is to convert  `.auth` to a format which can be enrolled on a system where `efitools` is not available. This facilitates rollout of secureboot keys on "untrusted" machines.

Just like the `.auth` files, the `.vardata` files do not contain private signing keys. It is safe to copy them onto an untrusted machine. Note that `sbctl` can also do the enrolling, but it needs access to the private keys.

### Install dependencies

```sh
# Fedora
sudo dnf group install c-development
sudo dnf install gnu-efi-devel
```

### Build the binary

```sh
make
```

### Installation

```sh
#make DESTDIR=build install
sudo make install
```

### Create and enroll your keys

Signed secureboot keys can be generated with [efi-mkkeys](https://github.com/jirutka/efi-mkkeys).

Convert the signed secureboot keys (`*.auth`) to `*.vardata` files:

```sh
mkefivardata db.auth db.vardata
mkefivardata KEK.auth KEK.vardata
mkefivardata PK.auth PK.vardata
```

The remaining steps will only work in [setup mode](https://wiki.archlinux.org/title/Unified_Extensible_Firmware_Interface/Secure_Boot).

Copy the `.vardata` files to the efivars filesystem (requires administrator privilege):

```sh
chattr -i /sys/firmware/efi/efivars/*
cp db.vardata /sys/firmware/efi/efivars/db-d719b2cb-3d3a-4596-a3bc-dad00e67656f
cp KEK.vardata /sys/firmware/efi/efivars/KEK-8be4df61-93ca-11d2-aa0d-00e098032b8c
cp PK.vardata /sys/firmware/efi/efivars/PK-8be4df61-93ca-11d2-aa0d-00e098032b8c
```

Congratulations, the secureboot keys are now enrolled.

Notes:

* `cp <var>.vardata /sys/...` is equivalent to `efi-updatevar -f <var>.auth <var>`.
* The destination filenames in the efivars filesystem may look random, but they are always the same.
* The order of the `cp` commands matters. Writing to `/sys/firmware/efi/efivars/PK-8be4df61-93ca-11d2-aa0d-00e098032b8c` ends the setup mode.

### cert-to-efi-sig-list

```sh
make cert-to-efi-sig-list
./cert-to-efi-sig-list -g a25e0ad4-9c64-11f0-8e7e-0800279b31a2 fedora.crt fedora.esl
md5sum fedora.esl | sed -E 's/^e868d249.*/OK/'
```
