/******************************************************************************
 * Copyright © 2014-2018 The SuperNET Developers.                             *
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
#pragma once

#include "CCinclude.h"

bool TokensExactAmounts(bool goDeeper, struct CCcontract_info *cpTokens, int64_t &inputs, int64_t &outputs, Eval* eval, const CTransaction &tx, uint256 tokenid);
std::string CreateToken(int64_t txfee, int64_t assetsupply, std::string name, std::string description, std::vector<uint8_t> nonfungibleData);
std::string TokenTransfer(int64_t txfee, uint256 assetid, std::vector<uint8_t> destpubkey, int64_t total);
int64_t HasBurnedTokensvouts(struct CCcontract_info *cp, Eval* eval, const CTransaction& tx, uint256 reftokenid);
CPubKey GetTokenOriginatorPubKey(CScript scriptPubKey);
bool IsTokenMarkerVout(CTxOut vout);

int64_t GetTokenBalance(CPubKey pk, uint256 tokenid);
UniValue TokenInfo(uint256 tokenid);
UniValue TokenList();

struct CCTokensContract_info : public CCcontract_info
{
    CCTokensContract_info() : CCcontract_info(EVAL_TOKENS, 
            "RAMvUfoyURBRxAdVeTMHxn3giJZCFWeha2", 
            "RCNgAngYAdrfzujYyPgfbjCGNVQZzCgTad", 
            "03e6191c70c9c9a28f9fd87089b9488d0e6c02fb629df64979c9cdb6b2b4a68d95",
            (const uint8_t[]){ 0x1d, 0x0d, 0x0d, 0xce, 0x2d, 0xd2, 0xe1, 0x9d, 0xf5, 
                0xb6, 0x26, 0xd5, 0xad, 0xa0, 0xf0, 0x0a, 0xdd, 0x7a, 0x72, 0x7d, 0x17, 0x35, 
                0xb5, 0xe3, 0x2c, 0x6c, 0xa9, 0xa2, 0x03, 0x16, 0x4b, 0xcf })
    { }
    /*****
     * Validate a transaction
     * @param eval holds results
     * @param tx the transaction to validate
     * @param nIn not used
     * @returns true on success
     */    
    virtual bool validate(Eval* eval, const CTransaction &tx, uint32_t nIn) override;
};
