# efi-updatevar

* [efitools was removed from Fedora 41](https://discussion.fedoraproject.org/t/f41-secure-boot-with-only-your-own-keys/138120)
* [efitools upstream](https://web.git.kernel.org/pub/scm/linux/kernel/git/jejb/efitools.git/) is unmaintained
* sbctl can generate keys and sign, but [efi-updatevar is still needed](https://github.com/Foxboron/sbctl/issues/434)

The upstream efi-updatevar was modified so that it doesn't write to the efivars filesystem directly, but converts the "auth" files to intermediate "vardata" files instead.

It is safe to copy the vardata files onto an untrusted machine as they do not contain the private key. To enroll the secureboot keys they contain, simply copy the vardata files to the appropriate file in the efivars filesystem.

### dependencies

```sh
sudo dnf group install c-development
sudo dnf install gnu-efi-devel openssl-devel
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
sudo dnf copr enable chenxiaolong/sbctl
sudo dnf install sbctl
```

Generate auth files:

```sh
sudo sbctl create-keys
sudo sbctl enroll-keys --microsoft --export auth
```

Convert auth to vardata.

```sh
./efi-updatevar db.auth /tmp/db.vardata db
./efi-updatevar KEK.auth /tmp/KEK.vardata KEK
./efi-updatevar PK.auth /tmp/PK.vardata PK
```

Next, we update the efivars filesystem.
This may only work in setup mode.

To verify that the system is in setup mode, run `mokutil --sb-state` or `sbctl status`.

Now copy each vardata file to its correct destination in the efivars fs:

```sh
sudo chattr -i /sys/firmware/efi/efivars/*
sudo cp /tmp/db.vardata /sys/firmware/efi/efivars/db-d719b2cb-3d3a-4596-a3bc-dad00e67656f
sudo cp /tmp/KEK.vardata /sys/firmware/efi/efivars/KEK-8be4df61-93ca-11d2-aa0d-00e098032b8c
sudo cp /tmp/PK.vardata /sys/firmware/efi/efivars/PK-8be4df61-93ca-11d2-aa0d-00e098032b8c
```

Notes:

* The destination filenames in the efivars fs look random, but they are always the same.
* After copying `PK.vardata`, the system should not be in setup mode anymore.
