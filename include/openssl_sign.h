#include <openssl/pem.h>

int
sign_efi_var(char *payload, int payload_size, char *keyfile, char *certfile,
	     unsigned char **sig, int *sigsize);
EVP_PKEY *
read_private_key(char *keyfile);
