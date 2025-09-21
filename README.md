# mkefivardata

To enroll signed secureboot keys, [efitools is needed](https://github.com/Foxboron/sbctl/issues/434). mkefivardata converts the signed secureboot keys to a format that can be enrolled on a system where `efitools` is not available. This allows you to rollout your secureboot keys on "untrusted" machines, while keeping your private keys safe.

The `*.vardata` files do not contain the private keys used for signing. It is safe to copy them onto an untrusted machine. Note that sbctl can also enroll, but it needs access to the private keys for this.

### Install dependencies

```sh
sudo dnf group install c-development
sudo dnf install gnu-efi-devel openssl-devel
```

### Build the binary

```sh
make clean
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

The remaining steps will only work in setup mode.

To verify that the system is in setup mode, run `mokutil --sb-state` or `sbctl status`.

Copy each vardata file to its correct destination in the efivars filesystem:

```sh
sudo chattr -i /sys/firmware/efi/efivars/*
sudo cp db.vardata /sys/firmware/efi/efivars/db-d719b2cb-3d3a-4596-a3bc-dad00e67656f
sudo cp KEK.vardata /sys/firmware/efi/efivars/KEK-8be4df61-93ca-11d2-aa0d-00e098032b8c
sudo cp PK.vardata /sys/firmware/efi/efivars/PK-8be4df61-93ca-11d2-aa0d-00e098032b8c
```

Congratulations, the keys are now enrolled.

Notes:

* `cp <var>.vardata /sys/...` is equivalent to `efi-updatevar -f <var>.auth <var>`.
* The destination filenames in the efivars filesystem may look random, but they are always the same.
* Writing to `/sys/firmware/efi/efivars/PK-8be4df61-93ca-11d2-aa0d-00e098032b8c` ends the setup mode.
