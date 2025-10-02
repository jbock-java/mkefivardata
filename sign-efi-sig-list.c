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

static void
usage(const char *progname)
{
	printf("Usage: %s [-g <guid>] [-t <timestamp>] [-c <crt_file>] [-k <key_file>] <var> <efi_sig_list_file> <output_file>\n", progname);
}

static void
help(const char *progname)
{
	usage(progname);
	printf("Produce an output file with an authentication header for direct\n"
	       "update to a secure variable.\n\n"
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

int
main(int argc, char *argv[])
{
	char *certfile = NULL, *efifile, *keyfile = NULL, *outfile,
		*str, *timestampstr = NULL;
	const char *progname = argv[0];
	unsigned char *sigbuf;
	int varlen, sigsize;
	EFI_GUID vendor_guid;
	struct stat st;
	short unsigned int var[256];
	UINT32 attributes = EFI_VARIABLE_NON_VOLATILE
		| EFI_VARIABLE_RUNTIME_ACCESS
		| EFI_VARIABLE_BOOTSERVICE_ACCESS
		| EFI_VARIABLE_TIME_BASED_AUTHENTICATED_WRITE_ACCESS;
	EFI_TIME timestamp = { 0 };

	while (argc > 1) {
		if (strcmp("--help", argv[1]) == 0) {
			help(progname);
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
		usage(progname);
		exit(1);
	}

	str = argv[1];
	efifile = argv[2];
	outfile = argv[3];

	/* Specific GUIDs for special variables */
	if (strcmp(str, "PK") == 0 || strcmp(str, "KEK") == 0) {
		vendor_guid = (EFI_GUID)EFI_GLOBAL_VARIABLE;
	} else if (strcmp(str, "db") == 0 || strcmp(str, "dbx") == 0) {
		vendor_guid = (EFI_GUID){ 0xd719b2cb, 0x3d3a, 0x4596, {0xa3, 0xbc, 0xda, 0xd0,  0xe, 0x67, 0x65, 0x6f }};
	}

	memset(&timestamp, 0, sizeof(timestamp));
	time_t t;
	struct tm *tm, tms;

	memset(&tms, 0, sizeof(tms));

	if (timestampstr) {
		strptime(timestampstr, "%Y-%m-%d %H:%M:%S", &tms);
		tm = &tms;
		/* timestamp.Year is from 0 not 1900 as tm year is */
		tm->tm_year += 1900;
		tm->tm_mon += 1; /* tm_mon is 0-11 not 1-12 */
	} else if (attributes & EFI_VARIABLE_APPEND_WRITE) {
		/* for append update timestamp should be zero */
		memset(&tms, 0, sizeof(tms));
		tm = &tms;
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
	do {
		var[i] = str[i];
	} while (str[i++] != '\0');

	varlen = (i - 1)*sizeof(short unsigned int);

	int fdefifile = open(efifile, O_RDONLY);
	if (fdefifile == -1) {
		fprintf(stderr, "failed to open file %s: ", efifile);
		perror("");
		exit(1);
	}
	fstat(fdefifile, &st);

	/* signature is over variable name (no null), the vendor GUID, the
	 * attributes, the timestamp and the contents */
	int signbuflen = varlen + sizeof(EFI_GUID) + sizeof(UINT32) + sizeof(EFI_TIME) + st.st_size;
	char *signbuf = malloc(signbuflen);
	char *ptr = signbuf;
	memcpy(ptr, var, varlen);
	ptr += varlen;
	memcpy(ptr, &vendor_guid, sizeof(vendor_guid));
	ptr += sizeof(vendor_guid);
	memcpy(ptr, &attributes, sizeof(attributes));
	ptr += sizeof(attributes);
	memcpy(ptr, &timestamp, sizeof(timestamp));
	ptr += sizeof(timestamp);
	read(fdefifile, ptr, st.st_size);

	printf("Authentication Payload size %d\n", signbuflen);

	if (!keyfile || !certfile) {
		fprintf(stderr, "Doing signing, need certificate and key\n");
		exit(1);
	}
	if (sign_efi_var(signbuf, signbuflen, keyfile, certfile,
			 &sigbuf, &sigsize))
		exit(1);
	printf("Signature of size %d\n", sigsize);

	int outlen = OFFSET_OF(EFI_VARIABLE_AUTHENTICATION_2, AuthInfo.CertData) + sigsize;
	EFI_VARIABLE_AUTHENTICATION_2 *var_auth = malloc(outlen);

	var_auth->TimeStamp = timestamp;
	var_auth->AuthInfo.CertType = EFI_CERT_TYPE_PKCS7_GUID;
	var_auth->AuthInfo.Hdr.dwLength = sigsize + OFFSET_OF(WIN_CERTIFICATE_UEFI_GUID, CertData);
	var_auth->AuthInfo.Hdr.wRevision = 0x0200;
	var_auth->AuthInfo.Hdr.wCertificateType = WIN_CERT_TYPE_EFI_GUID;

	memcpy(var_auth->AuthInfo.CertData, sigbuf, sigsize);
	sigbuf = var_auth->AuthInfo.CertData;
	printf("Signature at: %ld\n", sigbuf - (unsigned char *)var_auth);

	int fdoutfile = open(outfile, O_CREAT|O_WRONLY|O_TRUNC, S_IWUSR|S_IRUSR);
	if (fdoutfile == -1) {
		fprintf(stderr, "failed to open %s: ", outfile);
		perror("");
		exit(1);
	}
	/* first we write the authentication header */
	write(fdoutfile, var_auth, outlen);
	/* Then we write the payload */
	write(fdoutfile, ptr, st.st_size);
	/* so now the file is complete and can be fed straight into
	 * SetVariable() as an authenticated variable update */
	close(fdoutfile);

	return 0;
}
