/*
 * Copyright 2013 <James.Bottomley@HansenPartnership.com>
 *
 * see COPYING file
 */
#include <stdlib.h>
#include <errno.h>
#include <stdio.h>
#include <string.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>

#include <efi.h>

#include <kernel_efivars.h>
#include <guid.h>

char* kernel_efi_path = NULL;

void kernel_variable_init() {
  char fname[] = "/tmp/efi.XXXXXX";
  char cmdline[256];
  int fd, ret;
  struct stat st;
  char *buf;

  if (kernel_efi_path)
    return;
  mkstemp(fname);
  snprintf(cmdline, sizeof(cmdline), "mount -l > %s", fname);
  ret = system(cmdline);
  if (WEXITSTATUS(ret) != 0)
    /* hopefully stderr said what was wrong */
    exit(1);
  fd = open(fname, O_RDONLY);
  unlink(fname);
  if (fd < 0) {
    printf(RED "[ERROR]" NC " Failed to open output of %s\n", cmdline);
    exit(1);
  }
  if (fstat(fd, &st) < 0) {
    printf(RED "[ERROR]" NC " stat failed\n");
    exit(1);
  }
  if (st.st_size == 0) {
    printf(RED "[ERROR]" NC " No efivarfs filesystem is mounted\n");
    exit(1);
  }
  buf = malloc(st.st_size);
  read(fd, buf, st.st_size);
  close(fd);

  char *ptr = buf;
  char path[512], type[512];
  while (ptr < buf + st.st_size) {
    int count;
    sscanf(ptr, "%*s on %s type %s %*[^\n]\n%n", path, type, &count);
    ptr += count;
    if (strcmp(type, "efivarfs") == 0)
      break;
  }
  if (strcmp(type, "efivarfs") != 0) {
    printf(RED "[ERROR]" NC " No efivarfs filesystem is mounted\n");
    exit(1);
  }
  kernel_efi_path = malloc(strlen(path) + 1);
  strcpy(kernel_efi_path, path);
}

int write_vardata(
    const char* vardata_file,
    uint32_t attributes,
    uint32_t size,
    void* buf) {
  if (!kernel_efi_path)
    return -EINVAL;

  char* newbuf = malloc(size + sizeof(attributes));
  int fd = open(vardata_file, O_RDWR|O_CREAT|O_TRUNC, 0644);
  if (fd < 0)
    return errno;
  memcpy(newbuf, &attributes, sizeof(attributes));
  memcpy(newbuf + sizeof(attributes), buf, size);
  ssize_t result = write(fd, newbuf, size + sizeof(attributes));
  close(fd);
  if (result != size + sizeof(attributes))
    return errno;
  return 0;
}
