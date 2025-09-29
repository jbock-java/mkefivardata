#pragma once
#include <wincert.h>
//***********************************************************************
// Signature Database
//***********************************************************************
///
/// The format of a signature database. 
///

typedef UINT8  EFI_SHA256_HASH[32];
typedef UINT8  EFI_SHA384_HASH[48];
typedef UINT8  EFI_SHA512_HASH[64];

// https://stackoverflow.com/questions/484357/trying-to-copy-struct-members-to-byte-array-in-c
void USARTWrite(const void *object, size_t size)
{
	const unsigned char *byte;
	for ( byte = object; size--; ++byte ) {
		printf("%02X", *byte);
	}
	putchar('\n');
}

typedef struct {
	///
	/// An identifier which identifies the agent which added the signature to the list.
	///
	EFI_GUID          SignatureOwner;
	///
	/// The format of the signature is defined by the SignatureType.
	///
	UINT8             SignatureData[1];
} EFI_SIGNATURE_DATA;

typedef struct {
	///
	/// The SHA384 hash of an X.509 certificate's To-Be-Signed contents.
	///
	EFI_SHA384_HASH     ToBeSignedHash;
	///
	/// The time that the certificate shall be considered to be revoked.
	///
	EFI_TIME            TimeOfRevocation;
} EFI_CERT_X509_SHA384;

typedef struct {
	///
	/// The SHA512 hash of an X.509 certificate's To-Be-Signed contents.
	///
	EFI_SHA512_HASH     ToBeSignedHash;
	///
	/// The time that the certificate shall be considered to be revoked.
	///
	EFI_TIME            TimeOfRevocation;
} EFI_CERT_X509_SHA512;

//
// _WIN_CERTIFICATE.wCertificateType
// 
#define WIN_CERT_TYPE_PKCS_SIGNED_DATA 0x0002
#define WIN_CERT_TYPE_EFI_PKCS115      0x0EF0
#define WIN_CERT_TYPE_EFI_GUID         0x0EF1

#define EFI_CERT_X509_GUID \
  (EFI_GUID){								\
    0xa5c059a1, 0x94e4, 0x4aa7, {0x87, 0xb5, 0xab, 0x15, 0x5c, 0x2b, 0xf0, 0x72} \
  }

#define EFI_CERT_RSA2048_GUID \
  (EFI_GUID){								\
    0x3c5766e8, 0x269c, 0x4e34, {0xaa, 0x14, 0xed, 0x77, 0x6e, 0x85, 0xb3, 0xb6} \
  }


#define EFI_CERT_TYPE_PKCS7_GUID \
  (EFI_GUID){								\
    0x4aafd29d, 0x68df, 0x49ee, {0x8a, 0xa9, 0x34, 0x7d, 0x37, 0x56, 0x65, 0xa7} \
  }

#define EFI_CERT_X509_SHA256_GUID \
	(EFI_GUID) { 0x3bd2a492, 0x96c0, 0x4079,		\
			{ 0xb4, 0x20, 0xfc, 0xf9, 0x8e, 0xf1, 0x03, 0xed } }

#define EFI_CERT_X509_SHA384_GUID \
	(EFI_GUID) { 0x7076876e, 0x80c2, 0x4ee6,		\
			{ 0xaa, 0xd2, 0x28, 0xb3, 0x49, 0xa6, 0x86, 0x5b } }

#define EFI_CERT_X509_SHA512_GUID \
	(EFI_GUID) { 0x446dbf63, 0x2502, 0x4cda,		\
			{ 0xbc, 0xfa, 0x24, 0x65, 0xd2, 0xb0, 0xfe, 0x9d } }

#define OFFSET_OF(TYPE, Field) ((UINTN) &(((TYPE *)0)->Field))

///
/// Attributes of Authenticated Variable
///
#define EFI_VARIABLE_AUTHENTICATED_WRITE_ACCESS              0x00000010
#define EFI_VARIABLE_TIME_BASED_AUTHENTICATED_WRITE_ACCESS   0x00000020
#define EFI_VARIABLE_APPEND_WRITE                            0x00000040


///
/// Size of AuthInfo prior to the data payload.
///
#define AUTHINFO_SIZE ((OFFSET_OF (EFI_VARIABLE_AUTHENTICATION, AuthInfo)) + \
                       (OFFSET_OF (WIN_CERTIFICATE_UEFI_GUID, CertData)) + \
                       sizeof (EFI_CERT_BLOCK_RSA_2048_SHA256))

#define AUTHINFO2_SIZE(VarAuth2) ((OFFSET_OF (EFI_VARIABLE_AUTHENTICATION_2, AuthInfo)) + \
                                  (UINTN) ((EFI_VARIABLE_AUTHENTICATION_2 *) (VarAuth2))->AuthInfo.Hdr.dwLength)

#define OFFSET_OF_AUTHINFO2_CERT_DATA ((OFFSET_OF (EFI_VARIABLE_AUTHENTICATION_2, AuthInfo)) + \
                                       (OFFSET_OF (WIN_CERTIFICATE_UEFI_GUID, CertData)))

