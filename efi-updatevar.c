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

#include <kernel_efivars.h>
#include <guid.h>
#include <version.h>

static void usage(char *progname) {
  printf("Usage: %s: <infile> <vardata_file> <var>\n", progname);
}

static void help(char *progname) {
  usage(progname);
  printf("Create a file that can be written to an efivarfs variable.\n");
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

int main(int argc, char *argv[]) {
  uint32_t attributes = EFI_VARIABLE_NON_VOLATILE
    | EFI_VARIABLE_RUNTIME_ACCESS
    | EFI_VARIABLE_BOOTSERVICE_ACCESS
    | EFI_VARIABLE_TIME_BASED_AUTHENTICATED_WRITE_ACCESS;
  char *progname = argv[0];

  if (argc == 1) {
    usage(progname);
    return 1;
  }

  if (strcmp("--version", argv[1]) == 0) {
    version(progname);
    return 0;
  }
  if (strcmp("--help", argv[1]) == 0) {
    help(progname);
    return 0;
  }
  if (argc != 4) {
    usage(progname);
    return 1;
  }
  char* infile = argv[1];
  char* vardata_file = argv[2];
  char* var = argv[3];

  EFI_GUID *owner = get_owner(var);
  if (!owner) {
    printf(RED "[ERROR]" NC " Variable must be one of: PK, KEK, db, dbx\n", var);
    return 1;
  }

  kernel_variable_init();

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

  if (ret == EACCES) {
    printf(RED "[ERROR]" NC " Cannot write to %s, wrong filesystem permissions\n", vardata_file);
    return 1;
  }
  if (ret != 0) {
    printf(RED "[ERROR]" NC " Failed to write to %s\n", vardata_file);
    return 1;
  }
  printf(GREEN "[OK]" NC " Copy %s to %s/%s-%s to update %s.\n",
    vardata_file, kernel_efi_path, var, guid_to_str(owner), var);
  return 0;
}
