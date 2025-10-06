# sign-efi-siglist

```
This tool is derived from efitools' "sign-efi-sig-list".
The name was changed to avoid confusion, because the output format is different:
"sign-efi-sig-list" creates output in "auth" format,
which is suitable for UEFI's standard "SetVariable" call.
"sign-efi-siglist" instead outputs the native format of the Linux kernel's "efivarfs" filesystem,
which is called "vardata" here.
This can be more convenient, because a "vardata" file can be copied directly
to the efivarfs filesystem.
There is no need for an additional tool like "efi-updatevar".
```

[efitools upstream](https://git.kernel.org/pub/scm/linux/kernel/git/jejb/efitools.git)

[docs: UEFI Services - Runtime Services](https://uefi.org/specs/UEFI/2.11/08_Services_Runtime_Services.html)

[docs: UEFI Secure Boot and Driver Signing](https://uefi.org/specs/UEFI/2.11/32_Secure_Boot_and_Driver_Signing.html)

[docs: kernel efivarfs](https://www.kernel.org/doc/html/latest/filesystems/efivarfs.html)

### Install dependencies

```sh
# Ubuntu / Debian
sudo apt-get install gnu-efi
# Fedora
sudo dnf install gnu-efi-devel
```

### Installation

```sh
sudo make install
```

### Create and enroll your keys (FIXME)

Copy the `.vardata` files to the efivars filesystem (requires administrator privilege):

```sh
chattr -i /sys/firmware/efi/efivars/*
cp db.vardata /sys/firmware/efi/efivars/db-d719b2cb-3d3a-4596-a3bc-dad00e67656f
cp KEK.vardata /sys/firmware/efi/efivars/KEK-8be4df61-93ca-11d2-aa0d-00e098032b8c
cp PK.vardata /sys/firmware/efi/efivars/PK-8be4df61-93ca-11d2-aa0d-00e098032b8c
```
