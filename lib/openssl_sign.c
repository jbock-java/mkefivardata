#include <openssl/pem.h>
#include <openssl/err.h>
#include <openssl/sha.h>

#include <openssl_sign.h>

EVP_PKEY * read_private_key(char *keyfile)
{
	BIO *key = BIO_new_file(keyfile, "r");
	EVP_PKEY *pkey;

	if (!key) {
		ERR_print_errors_fp(stdout);
		fprintf(stderr, "error reading private key file %s\n", keyfile);
		return NULL;
	}
	pkey = PEM_read_bio_PrivateKey(key, NULL, NULL, NULL);
	BIO_free(key);

	if (!pkey) {
		ERR_print_errors_fp(stdout);
		fprintf(stderr, "error processing private key file %s\n", keyfile);
		return NULL;
	}
	return pkey;
}

int sign_efi_var_ssl(
	char *payload,
	int payload_size,
	EVP_PKEY *pkey,
	X509 *cert,
	unsigned char **sig,
	int *sigsize)
{
	BIO *bio_data = BIO_new_mem_buf(payload, payload_size);
	PKCS7 *p7;

	p7 = PKCS7_sign(NULL, NULL, NULL, bio_data, PKCS7_BINARY|PKCS7_PARTIAL|PKCS7_DETACHED|PKCS7_NOATTR);
	const EVP_MD *md = EVP_get_digestbyname("SHA256");
	PKCS7_sign_add_signer(p7, cert, pkey, md, PKCS7_BINARY|PKCS7_DETACHED|PKCS7_NOATTR);
	PKCS7_final(p7, bio_data, PKCS7_BINARY|PKCS7_DETACHED|PKCS7_NOATTR);

	*sig = NULL;
	*sigsize = i2d_PKCS7_SIGNED(p7->d.sign, sig);
	PKCS7_free(p7);
	ERR_print_errors_fp(stdout);

	return 0;
}

int sign_efi_var(
	char *payload,
	int payload_size,
	char *keyfile,
	char *certfile,
	unsigned char **sig,
	int *sigsize)
{
	ERR_load_crypto_strings();
	OpenSSL_add_all_digests();
	OpenSSL_add_all_ciphers();
	/* here we may get highly unlikely failures or we'll get a
	 * complaint about FIPS signatures (usually becuase the FIPS
	 * module isn't present).  In either case ignore the errors
	 * (malloc will cause other failures out lower down */
	ERR_clear_error();

	BIO *cert_bio = BIO_new_file(certfile, "r");
	if (!cert_bio) {
		ERR_print_errors_fp(stdout);
		fprintf(stderr, "error reading certificate %s\n", certfile);
		return 1;
	}
	X509 *cert = PEM_read_bio_X509(cert_bio, NULL, NULL, NULL);
	BIO_free(cert_bio);
	if (!cert) {
		ERR_print_errors_fp(stdout);
		fprintf(stderr, "error reading certificate %s\n", certfile);
		return 1;
	}

	EVP_PKEY *pkey = read_private_key(keyfile);
	if (!pkey) {
		ERR_print_errors_fp(stdout);
		fprintf(stderr, "error reading private key %s\n", keyfile);
		return 1;
	}
	int ret = sign_efi_var_ssl(payload, payload_size, pkey, cert,
			       sig, sigsize);
	EVP_PKEY_free(pkey);
	X509_free(cert);

	return ret;
}

