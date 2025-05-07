# mkefivardata

* [efitools was removed from Fedora 41](https://discussion.fedoraproject.org/t/f41-secure-boot-with-only-your-own-keys/138120)
* [efitools upstream](https://web.git.kernel.org/pub/scm/linux/kernel/git/jejb/efitools.git/) is unmaintained
* sbctl can generate keys and sign, but [efi-updatevar is still needed](https://github.com/Foxboron/sbctl/issues/434)

The upstream `efi-updatevar` was modified so that it converts the `*.auth` files to intermediate `*.vardata` files (by writing to a user-specified file, rather than directly to the efivars filesystem). To avoid confusion, it was also renamed to `mkefivardata`.

The `*.vardata` files do not contain the private key used for signing. Hence it is safe to copy them onto an untrusted machine. To enroll the keys, simply copy the vardata files to the appropriate place in the efivars filesystem.

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

### Enroll keys

Install sbctl:

```sh
sudo dnf copr enable chenxiaolong/sbctl
sudo dnf install sbctl
```

Generate keys and auth files:

```sh
sudo sbctl create-keys
sudo sbctl enroll-keys --microsoft --export auth
```

Convert auth files to vardata files:

```sh
mkefivardata db.auth db.vardata db
mkefivardata KEK.auth KEK.vardata KEK
mkefivardata PK.auth PK.vardata PK
```

The remaining steps may only work in setup mode.

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
* After copying `PK.vardata`, the system should not be in setup mode anymore.
* Make a backup of `/var/lib/sbctl`.
