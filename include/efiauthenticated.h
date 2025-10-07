#pragma once

///
/// The WIN_CERTIFICATE structure is part of the PE/COFF specification.
///
typedef struct {
	///
	/// The length of the entire certificate,
	/// including the length of the header, in bytes.
	///
	uint32_t  dwLength;
	///
	/// The revision level of the WIN_CERTIFICATE
	/// structure. The current revision level is 0x0200.
	///
	uint16_t  wRevision;
	///
	/// The certificate type. See WIN_CERT_TYPE_xxx for the UEFI
	/// certificate types. The UEFI specification reserves the range of
	/// certificate type values from 0x0EF0 to 0x0EFF.
	///
	uint16_t  wCertificateType;
	///
	/// The following is the actual certificate. The format of
	/// the certificate depends on wCertificateType.
	///
	/// UINT8 bCertificate[ANYSIZE_ARRAY];
	///
} WIN_CERTIFICATE;

//***********************************************************************
// Signature Database
//***********************************************************************
///
/// The format of a signature database. 
///

typedef struct {
	///
	/// An identifier which identifies the agent which added the signature to the list.
	///
	EFI_GUID          SignatureOwner;
	///
	/// The format of the signature is defined by the SignatureType.
	///
	uint8_t             SignatureData[1];
} EFI_SIGNATURE_DATA;

typedef struct {
	///
	/// Type of the signature. GUID signature types are defined in below.
	///
	EFI_GUID            SignatureType;
	///
	/// Total size of the signature list, including this header.
	///
	uint32_t              SignatureListSize;
	///
	/// Size of the signature header which precedes the array of signatures.
	///
	uint32_t              SignatureHeaderSize;
	///
	/// Size of each signature.
	///
	uint32_t              SignatureSize;
	///
	/// Header before the array of signatures. The format of this header is specified
	/// by the SignatureType.
	/// UINT8           SignatureHeader[SignatureHeaderSize];
	///
	/// An array of signatures. Each signature is SignatureSize bytes in length.
	/// EFI_SIGNATURE_DATA Signatures[][SignatureSize];
	///
} EFI_SIGNATURE_LIST;

//
// _WIN_CERTIFICATE.wCertificateType
// 
#define WIN_CERT_TYPE_EFI_GUID 0x0ef1

#define EFI_CERT_X509_GUID \
	(EFI_GUID) { \
		0xa5c059a1, 0x94e4, 0x4aa7, { 0x87, 0xb5, 0xab, 0x15, 0x5c, 0x2b, 0xf0, 0x72 } \
	}

#define EFI_CERT_TYPE_PKCS7_GUID \
	(EFI_GUID) { \
		0x4aafd29d, 0x68df, 0x49ee, { 0x8a, 0xa9, 0x34, 0x7d, 0x37, 0x56, 0x65, 0xa7 } \
	}
