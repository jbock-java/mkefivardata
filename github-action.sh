#!/bin/bash

sudo apt-get install -y gnu-efi || exit 1
make esltest || exit 1
make sigtest || exit 1
make efitest
