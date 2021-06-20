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
#pragma once

#include "CCinclude.h"
#include "CCtokens.h"

class CoinHelper;
class TokenHelper;

UniValue HeirFundCoinCaller(int64_t txfee, int64_t coins, std::string heirName, CPubKey heirPubkey, int64_t inactivityTimeSec, std::string memo);
UniValue HeirFundTokenCaller(int64_t txfee, int64_t satoshis, std::string heirName, CPubKey heirPubkey, int64_t inactivityTimeSec, std::string memo, uint256 tokenid);
UniValue HeirClaimCaller(uint256 fundingtxid, int64_t txfee, std::string amount);
UniValue HeirAddCaller(uint256 fundingtxid, int64_t txfee, std::string amount);
UniValue HeirInfo(uint256 fundingtxid);
UniValue HeirList();

struct CCHeirContract_info : public CCcontract_info
{
    CCHeirContract_info() : CCcontract_info()
    {
        evalcode = EVAL_HEIR;
        strcpy(unspendableCCaddr, "RDVHcSekmXgeYBqRupNTmqo3Rn8QRXNduy");
        strcpy(normaladdr, "RTPwUjKYECcGn6Y4KYChLhgaht1RSU4jwf");
        strcpy(CChexstr, "03c91bef3d7cc59c3a89286833a3446b29e52a5e773f738a1ad2b09785e5f4179e");
        uint8_t HeirCCpriv[32] = { 0x9d, 0xa1, 0xf8, 0xf7, 0xba, 0x0a, 0x91, 0x36, 0x89, 0x9a, 
                0x86, 0x30, 0x63, 0x20, 0xd7, 0xdf, 0xaa, 0x35, 0xe3, 0x99, 0x32, 0x2b, 0x63, 
                0xc0, 0x66, 0x9c, 0x93, 0xc4, 0x5e, 0x9d, 0xb9, 0xce };
        memcpy(CCpriv, HeirCCpriv, 32); 
    }
    virtual bool validate(Eval* eval, const CTransaction &tx, uint32_t nIn) override;
};
