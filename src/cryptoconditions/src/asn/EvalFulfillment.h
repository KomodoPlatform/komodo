/*
 * Generated by asn1c-0.9.28 (https://lionet.info/asn1c)
 * From ASN.1 module "Crypto-Conditions"
 * 	found in "CryptoConditions.asn"
 */

#ifndef	_EvalFulfillment_H_
#define	_EvalFulfillment_H_


#include "asn_application.h"

/* Including external dependencies */
#include "OCTET_STRING.h"
#include "constr_SEQUENCE.h"

#ifdef __cplusplus
extern "C" {
#endif

/* EvalFulfillment */
typedef struct EvalFulfillment {
	OCTET_STRING_t	 code;
	
	/* Context for parsing across buffer boundaries */
	asn_struct_ctx_t _asn_ctx;
} EvalFulfillment_t;

/* Implementation */
extern asn_TYPE_descriptor_t asn_DEF_EvalFulfillment;

#ifdef __cplusplus
}
#endif

#endif	/* _EvalFulfillment_H_ */
#include "asn_internal.h"
