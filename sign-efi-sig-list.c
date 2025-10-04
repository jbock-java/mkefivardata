#include <stdint.h>
#include <efi.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <time.h>
#include <unistd.h>

#include <variables.h>
#include <guid.h>
#include <openssl_sign.h>

#define MAX_VAR_LEN 8

void usage()
{
	printf("Usage: sign-efi-sig-list [-g <guid>] [-t <timestamp>] [-c <crt_file>] [-k <key_file>] <var> <x.esl> <x.vardata>\n");
}

void help()
{
	usage();
	printf("Produce an output file with an authentication header for direct\n"
	       "update to a secure variable.\n\n"
	       "Note:"
	       "This version of sign-efi-sig-list does not create the usual \"auth\" format."
	       "Instead, it creates \"vardata\" format for the \"efivarfs\" filesystem, which\n"
	       "includes four leading bytes of \"attributes\" data.\n"
	       "To update the efi variable <var>, simply copy the \"vardata\" file\n"
	       "to the appropriate place in the efivarfs filesystem.\n"
	       "So no \"efi-updatevar\" binary is needed.\n\n"
	       "Options:\n"
	       "\t-t <timestamp>   Use <timestamp> as the timestamp of the timed variable update\n"
	       "\t                 If not present, then the timestamp will be taken from system\n"
	       "\t                 time.  Note you must use this option when doing detached\n"
	       "\t                 signing otherwise the signature will be incorrect because\n"
	       "\t                 of timestamp mismatches.\n"
	       "\t-g <guid>        Use <guid> as the signature owner GUID\n"
	       "\t-c <crt>         <crt> is the file containing the signing certificate in PEM format\n"
	       "\t-k <key>         <key> is the file containing the key for <crt> in PEM format\n"
	       );
}

int main(int argc, char *argv[])
{
	char *certfile = NULL, *efifile, *keyfile = NULL, *outfile,
		*var_str, *timestampstr = NULL;
	unsigned char *sigbuf;
	int varlen, sigsize;
	EFI_GUID vendor_guid;
	struct stat st;
	uint32_t attributes = EFI_VARIABLE_NON_VOLATILE
		| EFI_VARIABLE_RUNTIME_ACCESS
		| EFI_VARIABLE_BOOTSERVICE_ACCESS
		| EFI_VARIABLE_TIME_BASED_AUTHENTICATED_WRITE_ACCESS;
	EFI_TIME timestamp;
	memset(&timestamp, 0, sizeof(timestamp));

	while (argc) {
		if (strcmp("--help", argv[1]) == 0) {
			help();
			exit(0);
		} else if (strcmp("-g", argv[1]) == 0) {
			str_to_guid(argv[2], &vendor_guid);
			argv += 2;
			argc -= 2;
		} else if (strcmp("-t", argv[1]) == 0) {
			timestampstr = argv[2];
			argv += 2;
			argc -= 2;
		} else if (strcmp("-k", argv[1]) == 0) {
			keyfile = argv[2];
			argv += 2;
			argc -= 2;
		} else if (strcmp("-c", argv[1]) == 0) {
			certfile = argv[2];
			argv += 2;
			argc -= 2;
		} else  {
			break;
		}
	}

	if (argc != 4) {
		usage();
		exit(1);
	}

	var_str = argv[1];
	efifile = argv[2];
	outfile = argv[3];

	/* Specific GUIDs for special variables */
	if (strcmp(var_str, "PK") == 0 || strcmp(var_str, "KEK") == 0) {
		vendor_guid = (EFI_GUID) EFI_GLOBAL_VARIABLE;
	} else if (strcmp(var_str, "db") == 0 || strcmp(var_str, "dbx") == 0) {
		vendor_guid = (EFI_GUID) { 0xd719b2cb, 0x3d3a, 0x4596, {0xa3, 0xbc, 0xda, 0xd0, 0xe, 0x67, 0x65, 0x6f }};
	}

	time_t t;
	struct tm *tm, tms;

	memset(&tms, 0, sizeof(tms));

	if (timestampstr) {
		strptime(timestampstr, "%Y-%m-%d %H:%M:%S", &tms);
		tm = &tms;
		/* timestamp.Year is from 0 not 1900 as tm year is */
		tm->tm_year += 1900;
		tm->tm_mon += 1; /* tm_mon is 0-11 not 1-12 */
	} else {
		time(&t);
		tm = localtime(&t);
		/* timestamp.Year is from 0 not 1900 as tm year is */
		tm->tm_year += 1900;
		tm->tm_mon += 1; /* tm_mon is 0-11 not 1-12 */
	}

	timestamp.Year = tm->tm_year;
	timestamp.Month = tm->tm_mon;
	timestamp.Day = tm->tm_mday;
	timestamp.Hour = tm->tm_hour;
	timestamp.Minute = tm->tm_min;
	timestamp.Second = tm->tm_sec;

	printf("Timestamp is %d-%d-%d %02d:%02d:%02d\n", timestamp.Year,
	       timestamp.Month, timestamp.Day, timestamp.Hour, timestamp.Minute,
	       timestamp.Second);

	int i = 0;
	unsigned short int var[MAX_VAR_LEN];
	while (i < MAX_VAR_LEN && var_str[i] != '\0') {
		var[i] = var_str[i];
		i++;
	}
	varlen = i * sizeof(unsigned short int);

	int fdefifile = open(efifile, O_RDONLY);
	if (fdefifile == -1) {
		fprintf(stderr, "failed to open file %s: ", efifile);
		exit(1);
	}
	fstat(fdefifile, &st);

	/* signature is over variable name (no null), the vendor GUID, the
	 * attributes, the timestamp and the contents */
	int signbuf_header_len = varlen + sizeof(EFI_GUID) + sizeof(uint32_t) + sizeof(EFI_TIME);
	int signbuflen = signbuf_header_len + st.st_size;
	char *signbuf = malloc(signbuflen);
	memcpy(signbuf, var, varlen);
	memcpy(signbuf + varlen, &vendor_guid, sizeof(EFI_GUID));
	memcpy(signbuf + varlen + sizeof(EFI_GUID), &attributes, sizeof(uint32_t));
	memcpy(signbuf + varlen + sizeof(EFI_GUID) + sizeof(uint32_t), &timestamp, sizeof(EFI_TIME));
	read(fdefifile, signbuf + signbuf_header_len, st.st_size);

	printf("Authentication Payload size %d\n", signbuflen);

	if (!keyfile || !certfile) {
		fprintf(stderr, "Doing signing, need certificate and key\n");
		exit(1);
	}
	if (sign_efi_var(signbuf, signbuflen, keyfile, certfile,
			 &sigbuf, &sigsize))
		exit(1);
	printf("Signature of size %d\n", sigsize);


	int outlen = sizeof(uint32_t) + sizeof(EFI_TIME) + sizeof(WIN_CERTIFICATE) + sizeof(EFI_GUID) + sigsize + st.st_size;
	unsigned char *var_auth = malloc(outlen);

	// The length of the entire certificate, including the length of the header, in bytes.
	// -- Is the timestamp not part of the header?
	uint32_t dwLength = sizeof(WIN_CERTIFICATE) + sizeof(EFI_GUID) + sigsize;

	// The revision level of the WIN_CERTIFICATE structure. The current revision level is 0x0200.
	uint16_t wRevision = 0x0200;

	// The certificate type. See WIN_CERT_TYPE_xxx for the UEFI certificate types.
	// The UEFI specification reserves the range of certificate type values from 0x0EF0 to 0x0EFF.
	uint16_t wCertificateType = WIN_CERT_TYPE_EFI_GUID;

	// This is the unique id which determines the format of the CertData.
	EFI_GUID certType = EFI_CERT_TYPE_PKCS7_GUID;

	// efivarfs vardata format: 4_bytes_of_attributes + efivar_data
	memcpy(var_auth, &attributes, sizeof(uint32_t));

	// EFI_TIME TimeStamp
	memcpy(var_auth + sizeof(uint32_t), &timestamp, sizeof(EFI_TIME));

	// UINT32 AuthInfo.Hdr.dwLength
	memcpy(var_auth + sizeof(uint32_t) + sizeof(EFI_TIME), &dwLength, sizeof(uint32_t));

	// UINT16 AuthInfo.Hdr.wRevision
	memcpy(var_auth + sizeof(uint32_t) + sizeof(EFI_TIME) + sizeof(uint32_t), &wRevision, sizeof(uint16_t));

	// UINT16 AuthInfo.Hdr.wCertificateType
	memcpy(var_auth + sizeof(uint32_t) + sizeof(EFI_TIME) + sizeof(uint32_t) + sizeof(uint16_t),
		&wCertificateType, sizeof(uint16_t));

	// EFI_GUID AuthInfo.CertType
	memcpy(var_auth + sizeof(uint32_t) + sizeof(EFI_TIME) + sizeof(WIN_CERTIFICATE),
		&certType, sizeof(EFI_GUID));

	// AuthInfo.CertData
	memcpy(var_auth + sizeof(uint32_t) + sizeof(EFI_TIME) + sizeof(WIN_CERTIFICATE) + sizeof(EFI_GUID),
		sigbuf, sigsize);

	// Authentication header complete, now write the payload (the original esl)
	memcpy(var_auth + sizeof(uint32_t) + sizeof(EFI_TIME) + sizeof(WIN_CERTIFICATE) + sizeof(EFI_GUID) + sigsize,
		signbuf + signbuf_header_len, st.st_size);

	int fdoutfile = open(outfile, O_CREAT|O_WRONLY|O_TRUNC, S_IWUSR|S_IRUSR);
	if (fdoutfile == -1) {
		fprintf(stderr, "failed to open %s: ", outfile);
		exit(1);
	}

	write(fdoutfile, var_auth, outlen);

	/* so now the file is complete and can be fed straight into
	 * SetVariable() as an authenticated variable update */
	close(fdoutfile);

	return 0;
}
