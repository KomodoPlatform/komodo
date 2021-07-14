/******************************************************************************
 * Copyright © 2014-2019 The SuperNET Developers.                             *
 *                                                                            *
 * See the AUTHORS, DEVELOPER-AGREEMENT and LICENSE files at                  *
 * the top-level directory of this distribution for the individual copyright  *
 * holder information and the developer policies on copyright and licensing.  *
 *                                                                            *
 * Unless otherwise agreed in a custom licensing agreement, no part of the    *
 * SuperNET software, including this file may be copied, modified, propagated *
 * or distributed except according to the terms contained in the LICENSE file *
 *                                                                            *
 * Removal or modification of this copyright notice is prohibited.            *
 *                                                                            *
 ******************************************************************************/


/*
 CCassetstx has the functions that create the EVAL_ASSETS transactions. It is expected that rpc calls would call these functions. For EVAL_ASSETS, the rpc functions are in rpcwallet.cpp
 
 CCassetsCore has functions that are used in two contexts, both during rpc transaction create time and also during the blockchain validation. Using the identical functions is a good way to prevent them from being mismatched. The must match or the transaction will get rejected.
 */

#ifndef CC_ASSETS_H
#define CC_ASSETS_H

#include "CCinclude.h"

#include "old/CCassets_v0.h"

#define ASSETS_GLOBALADDR_VIN  1
#define ASSETS_GLOBALADDR_VOUT 0
#define ASSETS_MARKER_AMOUNT 10000

// CCcustom
bool AssetsValidate(struct CCcontract_info *cp, Eval* eval, const CTransaction &tx, uint32_t nIn);
bool Assetsv2Validate(struct CCcontract_info *cp, Eval* eval, const CTransaction &tx, uint32_t nIn);

// CCassetsCore
vscript_t EncodeAssetOpRetV1(uint8_t assetFuncId, uint256 assetid2, int64_t unit_price, std::vector<uint8_t> origpubkey);
uint8_t DecodeAssetTokenOpRetV1(const CScript &scriptPubKey, uint8_t &assetsEvalCode, uint256 &tokenid, uint256 &assetid2, int64_t &unit_price, std::vector<uint8_t> &origpubkey);
vscript_t EncodeAssetOpRetV2(uint8_t assetFuncId, uint256 assetid2, int64_t unit_price, std::vector<uint8_t> origpubkey);
uint8_t DecodeAssetTokenOpRetV2(const CScript &scriptPubKey, uint8_t &assetsEvalCode, uint256 &tokenid, uint256 &assetid2, int64_t &unit_price, std::vector<uint8_t> &origpubkey);

bool ValidateBidRemainder(CAmount unit_price, int64_t remaining_nValue, int64_t orig_nValue, int64_t received_nValue, int64_t paid_units);
bool ValidateAskRemainder(CAmount unit_price, int64_t remaining_assetoshis, int64_t orig_assetoshis, int64_t received_assetoshis, int64_t paid_nValue);
bool ValidateSwapRemainder(int64_t remaining_units, int64_t remaining_nValue, int64_t orig_nValue, int64_t received_nValue, int64_t paidprice, int64_t totalprice);
bool SetBidFillamounts(CAmount unit_price, int64_t &received_nValue, int64_t orig_nValue, int64_t &paid_units, int64_t orig_units, CAmount paid_unit_price);
bool SetAskFillamounts(CAmount unit_price, int64_t fill_assetoshis, int64_t orig_assetoshis, int64_t paid_nValue);
bool SetSwapFillamounts(CAmount unit_price, int64_t &paid, int64_t orig_nValue, int64_t &received, int64_t totalprice); // not implemented
CAmount AssetsGetCCInputs(Eval *eval, struct CCcontract_info *cp, const char *addr, const CTransaction &tx);

const char ccassets_log[] = "ccassets";


class AssetsV1 {
public:
    static uint8_t EvalCode() { return EVAL_ASSETS; }
    static uint8_t TokensEvalCode() { return EVAL_TOKENS; }
    static bool IsMixed() { return false; }

    static vscript_t EncodeAssetOpRet(uint8_t assetFuncId, uint256 assetid2, int64_t unit_price, std::vector<uint8_t> origpubkey)
    {
        return ::EncodeAssetOpRetV1(assetFuncId, assetid2, unit_price, origpubkey);
    }

    static uint8_t DecodeAssetTokenOpRet(const CScript &scriptPubKey, uint8_t &assetsEvalCode, uint256 &tokenid, uint256 &assetid2, int64_t &unit_price, std::vector<uint8_t> &origpubkey)
    {
        return ::DecodeAssetTokenOpRetV1(scriptPubKey, assetsEvalCode, tokenid, assetid2, unit_price, origpubkey);
    }

    static bool ConstrainVout(CTxOut vout, int32_t CCflag, char *cmpaddr, int64_t nValue, uint8_t evalCode)
    {
        return ::ConstrainVout(vout, CCflag, cmpaddr, nValue);
    }

};

class AssetsV2 {
public:
    static uint8_t EvalCode() { return EVAL_ASSETSV2; }
    static uint8_t TokensEvalCode() { return EVAL_TOKENSV2; }

    static bool IsMixed() { return true; }

    static vscript_t EncodeAssetOpRet(uint8_t assetFuncId, uint256 assetid2, int64_t unit_price, std::vector<uint8_t> origpubkey)
    {
        return ::EncodeAssetOpRetV2(assetFuncId, assetid2, unit_price, origpubkey);
    }

    static uint8_t DecodeAssetTokenOpRet(const CScript &scriptPubKey, uint8_t &assetsEvalCode, uint256 &tokenid, uint256 &assetid2, int64_t &unit_price, std::vector<uint8_t> &origpubkey)
    {
        return ::DecodeAssetTokenOpRetV2(scriptPubKey, assetsEvalCode, tokenid, assetid2, unit_price, origpubkey);
    }

    static bool ConstrainVout(CTxOut vout, int32_t CCflag, char *cmpaddr, int64_t nValue, uint8_t evalCode)
    {
        return ::ConstrainVoutV2(vout, CCflag, cmpaddr, nValue, evalCode);
    }
};

#endif
