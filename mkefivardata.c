/*
 * Copyright 2013 <James.Bottomley@HansenPartnership.com>
 *
 * see COPYING file
 */

#include <stdio.h>
#include <string.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>

#include <openssl/err.h>

#include <efi.h>

#define RED "\x1b[31m"
#define GREEN "\x1b[32m"
#define NC "\x1b[0m"

EFI_GUID GV_GUID = EFI_GLOBAL_VARIABLE;
EFI_GUID SIG_DB = { 0xd719b2cb, 0x3d3a, 0x4596, {0xa3, 0xbc, 0xda, 0xd0,  0xe, 0x67, 0x65, 0x6f }};

void usage() {
  printf("Usage: mkefivardata /path/to/in.auth /path/to/out.vardata PK|KEK|db\n");
}

void help() {
  usage();
  printf("Creates a file that can be copied to an efivarfs variable.\n");
}

EFI_GUID* get_owner(char* var) {
  if (strcmp(var, "PK") == 0 || strcmp(var, "KEK") == 0) {
    return &GV_GUID;
  }
  if (strcmp(var, "db") == 0 || strcmp(var, "dbx") == 0) {
    return &SIG_DB;
  }
  return NULL;
}

int write_vardata(
    const char* vardata_file,
    uint32_t attributes,
    uint32_t size,
    void* buf) {

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

int main(int argc, char *argv[]) {
  uint32_t attributes = EFI_VARIABLE_NON_VOLATILE
    | EFI_VARIABLE_RUNTIME_ACCESS
    | EFI_VARIABLE_BOOTSERVICE_ACCESS
    | EFI_VARIABLE_TIME_BASED_AUTHENTICATED_WRITE_ACCESS;
  if (strcmp("--help", argv[1]) == 0) {
    help();
    return 0;
  }
  if (argc != 4) {
    usage();
    return 1;
  }
  char* infile = argv[1];
  char* vardata_file = argv[2];
  char* var = argv[3];

  EFI_GUID *owner = get_owner(var);
  if (!owner) {
    printf(RED "[ERROR]" NC " Variable must be one of: PK, KEK, db, dbx\n");
    return 1;
  }

  int fd = open(infile, O_RDONLY);
  if (fd < 0) {
    printf(RED "[ERROR]" NC " Failed to read file %s\n", infile);
    return 1;
  }
  struct stat st;
  if (fstat(fd, &st) < 0) {
    printf(RED "[ERROR]" NC " stat failed\n");
    return 1;
  }
  char* buf = malloc(st.st_size);
  read(fd, buf, st.st_size);
  int ret = write_vardata(vardata_file, attributes, st.st_size, buf);
  close(fd);
  free(buf);

  if (ret != 0) {
    printf(RED "[ERROR]" NC " Failed to write to %s\n", vardata_file);
    return 1;
  }
  printf(GREEN "[OK]" NC " Copy %s to /sys/firmware/efi/efivars/%s-%08x-%04hx-%04hx-%02hhx%02hhx-%02hhx%02hhx%02hhx%02hhx%02hhx%02hhx to update %s.\n",
    vardata_file, var, owner->Data1, owner->Data2, owner->Data3,
		owner->Data4[0], owner->Data4[1], owner->Data4[2],
		owner->Data4[3], owner->Data4[4], owner->Data4[5],
		owner->Data4[6], owner->Data4[7], var);
  return 0;
}
