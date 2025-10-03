#include <stdint.h>
#include <efi.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <openssl/pem.h>
#include <openssl/err.h>

#include <guid.h>
#include <efiauthenticated.h>
#include <version.h>

static void usage(const char *progname)
{
	printf("Usage: %s [-g <guid>] <crt file> <efi sig list file>\n", progname);
}

static void help(const char * progname)
{
	usage(progname);
	printf("Take an input X509 certificate (in PEM format) and convert it to an EFI\n"
	       "signature list file containing only that single certificate\n\n"
	       "Options:\n"
	       "\t-g <guid>        Use <guid> as the owner of the signature. If this is not\n"
	       "\t                 supplied, an all zero guid will be used\n"

	       );
	
}

int main(int argc, char *argv[])
{
	char *certfile, *efifile;
	const char *progname = argv[0];
	EFI_GUID owner = { 0 };

	while (argc > 1) {
		if (strcmp("--version", argv[1]) == 0) {
			version(progname);
			exit(0);
		} else if (strcmp("--help", argv[1]) == 0) {
			help(progname);
			exit(0);
		} else if (strcmp("-g", argv[1]) == 0) {
			str_to_guid(argv[2], &owner);
			argv += 2;
			argc -= 2;
		} else {
			break;
		}
	}
	  

	if (argc != 3) {
		exit(1);
	}

	certfile = argv[1];
	efifile = argv[2];

	BIO *cert_bio = BIO_new_file(certfile, "r");
	X509 *cert = PEM_read_bio_X509(cert_bio, NULL, NULL, NULL);
	unsigned char *PkCert = NULL;
	UINT32 PkCertLen = i2d_X509(cert, &PkCert);

	UINT32 signature_size = PkCertLen + sizeof(EFI_GUID);
	UINT32 result_size = PkCertLen + 2 * sizeof(EFI_GUID) + 3 * sizeof(UINT32);
	unsigned char *result = malloc(result_size);

	// https://uefi.org/specs/UEFI/2.11/32_Secure_Boot_and_Driver_Signing.html
	memcpy(result + 0 * sizeof(EFI_GUID) + 0 * sizeof(UINT32), &EFI_CERT_X509_GUID, sizeof(EFI_GUID)); // SignatureType
	memcpy(result + 1 * sizeof(EFI_GUID) + 0 * sizeof(UINT32), &result_size, sizeof(UINT32));          // SignatureListSize
	memset(result + 1 * sizeof(EFI_GUID) + 1 * sizeof(UINT32), 0, sizeof(UINT32));                 // SignatureHeaderSize
	memcpy(result + 1 * sizeof(EFI_GUID) + 2 * sizeof(UINT32), &signature_size, sizeof(UINT32));       // SignatureSize
	memcpy(result + 1 * sizeof(EFI_GUID) + 3 * sizeof(UINT32), &owner, sizeof(EFI_GUID));              // Signatures[0]->SignatureOwner
	memcpy(result + 2 * sizeof(EFI_GUID) + 3 * sizeof(UINT32), PkCert, PkCertLen);                     // Signatures[0]->SignatureData

	FILE *f = fopen(efifile, "w");
	if (!f) {
		fprintf(stderr, "failed to open efi file %s: ", efifile);
		exit(1);
	}
	if (fwrite(result, 1, result_size, f) != result_size) {
		fprintf(stderr, "Did not write enough bytes to efi file");
		exit(1);
	}

	OPENSSL_free(PkCert);
	free(result);

	return 0;
}
