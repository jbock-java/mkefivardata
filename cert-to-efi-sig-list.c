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

static void
usage(const char *progname)
{
	printf("Usage: %s [-g <guid>] <crt file> <efi sig list file>\n", progname);
}

static void
help(const char * progname)
{
	usage(progname);
	printf("Take an input X509 certificate (in PEM format) and convert it to an EFI\n"
	       "signature list file containing only that single certificate\n\n"
	       "Options:\n"
	       "\t-g <guid>        Use <guid> as the owner of the signature. If this is not\n"
	       "\t                 supplied, an all zero guid will be used\n"

	       );
	
}

int
main(int argc, char *argv[])
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

        ERR_load_crypto_strings();
        OpenSSL_add_all_digests();
        OpenSSL_add_all_ciphers();
	/* here we may get highly unlikely failures or we'll get a
	 * complaint about FIPS signatures (usually becuase the FIPS
	 * module isn't present).  In either case ignore the errors
	 * (malloc will cause other failures out lower down */
	ERR_clear_error();

        BIO *cert_bio = BIO_new_file(certfile, "r");
        X509 *cert = PEM_read_bio_X509(cert_bio, NULL, NULL, NULL);
	int PkCertLen = i2d_X509(cert, NULL);

	int sizeof_efi_signature_list = 28;
	PkCertLen += sizeof_efi_signature_list + OFFSET_OF(EFI_SIGNATURE_DATA, SignatureData);
	void          *PkCert = malloc (PkCertLen);
	if (!PkCert) {
		fprintf(stderr, "failed to malloc cert\n");
		exit(1);
	}
	unsigned char *tmp = (unsigned char *)PkCert + sizeof_efi_signature_list + OFFSET_OF(EFI_SIGNATURE_DATA, SignatureData);
	i2d_X509(cert, &tmp);

	EFI_SIGNATURE_DATA *PkCertData = (void *)((unsigned char *)PkCert + sizeof_efi_signature_list);

	PkCertData->SignatureOwner = owner; 

	UINT32 signature_size = (UINT32) (PkCertLen - sizeof_efi_signature_list);
	unsigned char result[PkCertLen];
	memset(result, 0, PkCertLen);
	memcpy(result, &EFI_CERT_X509_GUID, sizeof(EFI_GUID));
	memcpy(result + sizeof(EFI_CERT_X509_GUID), &PkCertLen, sizeof(PkCertLen));
	memcpy(result + sizeof(EFI_CERT_X509_GUID) + sizeof(PkCertLen) + sizeof(UINT32), &signature_size, sizeof(UINT32));
	memcpy(result + sizeof_efi_signature_list, PkCertData, PkCertLen - sizeof_efi_signature_list);

	FILE *f = fopen(efifile, "w");
	if (!f) {
		fprintf(stderr, "failed to open efi file %s: ", efifile);
		perror("");
		exit(1);
	}
	if (fwrite(result, 1, PkCertLen, f) != PkCertLen) {
		perror("Did not write enough bytes to efi file");
		exit(1);
	}


	return 0;
}

/*
typedef struct {          
    UINT32  Data1;
    UINT16  Data2;
    UINT16  Data3;
    UINT8   Data4[8]; 
} EFI_GUID;
*/
