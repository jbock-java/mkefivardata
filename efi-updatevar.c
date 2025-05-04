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
#include <openssl_sign.h>
#include <guid.h>
#include <version.h>

static void usage(char *progname) {
  printf("Usage: %s: <file> <var>\n", progname);
}

static void help(char *progname) {
  usage(progname);
  printf("Write the key file <file> (.esl or .auth) to the <var>.\n");
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

  if (strcmp("--version", argv[1]) == 0) {
    version(progname);
    return 0;
  }
  if (strcmp("--help", argv[1]) == 0) {
    help(progname);
    return 0;
  }
  if (argc != 3) {
    usage(progname);
    return 1;
  }
  char* file = argv[1];
  char* var = argv[2];

  EFI_GUID *owner = get_owner(var);
  if (!owner) {
    fprintf(stderr, "Variable must be one of: PK, KEK, db, dbx\n", var);
    return 1;
  }

  kernel_variable_init();
  ERR_load_crypto_strings();
  OpenSSL_add_all_digests();
  OpenSSL_add_all_ciphers();

  int fd = open(file, O_RDONLY);
  if (fd < 0) {
    fprintf(stderr, "Failed to read file %s: ", file);
    perror("");
    return 1;
  }
  struct stat st;
  if (fstat(fd, &st) < 0) {
    perror("stat failed");
    return 1;
  }
  char* buf = malloc(st.st_size);
  read(fd, buf, st.st_size);
  int ret = set_variable(var, owner, attributes, st.st_size, buf);
  close(fd);
  free(buf);

  if (ret == EACCES) {
    fprintf(stderr, "Cannot write to %s, wrong filesystem permissions\n", var);
    return 1;
  }
  if (ret != 0) {
    fprintf(stderr, "Failed to update %s: ", var);
    perror("");
    return 1;
  }
  return 0;
}
