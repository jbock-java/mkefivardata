# efi-updatevar

* [efitools was removed from Fedora 41](https://discussion.fedoraproject.org/t/f41-secure-boot-with-only-your-own-keys/138120)
* [efitools upstream](https://web.git.kernel.org/pub/scm/linux/kernel/git/jejb/efitools.git/) is unmaintained
* sbctl can generate keys and sign, but [efi-updatevar is still needed](https://github.com/Foxboron/sbctl/issues/434)

The upstream efi-updatevar was modified so that it doesn't write to the efivars filesystem directly, but converts the "auth" files to intermediate "vardata" files instead. These vardata files can simply be copied onto the target machine's efivarfs.

### dependencies

```sh
sudo dnf group install c-development
sudo dnf install efivar-devel gnu-efi-devel openssl openssl-devel openssl-devel-engine help2man
```

### Building efi-updatevar

```sh
make clean
make
```

### create ctags

```sh
ctags -R --exclude .git
```

### Testing

Install sbctl:

```sh
dnf copr enable chenxiaolong/sbctl
dnf install sbctl
```

Generate auth files:

```sh
sbctl create-keys
sbctl enroll-keys --microsoft --export auth
```

Allow writing to efivarfs:

```sh
chattr -i /sys/firmware/efi/efivars/*
```

Create the vardata files:

```sh
./efi-updatevar db.auth /tmp/db.vardata db
./efi-updatevar KEK.auth /tmp/KEK.vardata KEK
./efi-updatevar PK.auth /tmp/PK.vardata PK
```

To update the efi variables, simply copy the vardata files to their correct destination in the efivars fs.
The destination filenames in the efivars fs look random, but they are always the same:

```sh
cp /tmp/db.vardata /sys/firmware/efi/efivars/db-d719b2cb-3d3a-4596-a3bc-dad00e67656f
cp /tmp/KEK.vardata /sys/firmware/efi/efivars/KEK-8be4df61-93ca-11d2-aa0d-00e098032b8c
cp /tmp/PK.vardata /sys/firmware/efi/efivars/PK-8be4df61-93ca-11d2-aa0d-00e098032b8c
```
