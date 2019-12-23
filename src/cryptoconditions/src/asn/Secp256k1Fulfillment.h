/*
 * Generated by asn1c-0.9.28 (https://lionet.info/asn1c)
 * From ASN.1 module "Crypto-Conditions"
 * 	found in "CryptoConditions.asn"
 */

#ifndef	_Secp256k1Fulfillment_H_
#define	_Secp256k1Fulfillment_H_


#include "asn_application.h"

/* Including external dependencies */
#include "OCTET_STRING.h"
#include "constr_SEQUENCE.h"

#ifdef __cplusplus
extern "C" {
#endif

/* Secp256k1Fulfillment */
typedef struct Secp256k1Fulfillment {
	OCTET_STRING_t	 publicKey;
	OCTET_STRING_t	 signature;
	
	/* Context for parsing across buffer boundaries */
	asn_struct_ctx_t _asn_ctx;
} Secp256k1Fulfillment_t;

/* Implementation */
extern asn_TYPE_descriptor_t asn_DEF_Secp256k1Fulfillment;

#ifdef __cplusplus
}
#endif

#endif	/* _Secp256k1Fulfillment_H_ */
#include "asn_internal.h"
