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

#define EVAL_FAUCET 0xe4
#define FAUCETSIZE (COIN / 10)

bool FaucetValidate(struct CCcontract_info *cp,Eval* eval,const CTransaction &tx, uint32_t nIn);

// CCcustom
UniValue FaucetFund(const CPubKey& mypk,uint64_t txfee,int64_t funds);
UniValue FaucetGet(const CPubKey& mypk,uint64_t txfee);
UniValue FaucetInfo();

struct CCFaucetContract_info : public CCcontract_info
{
    CCFaucetContract_info() : CCcontract_info()
    {
        evalcode = EVAL_FAUCET;
        strcpy(unspendableCCaddr,"R9zHrofhRbub7ER77B7NrVch3A63R39GuC");
        strcpy(normaladdr,"RKQV4oYs4rvxAWx1J43VnT73rSTVtUeckk");
        strcpy(CChexstr,"03682b255c40d0cde8faee381a1a50bbb89980ff24539cb8518e294d3a63cefe12");
        uint8_t FaucetCCpriv[32] = { 0xd4, 0x4f, 0xf2, 0x31, 0x71, 0x7d, 0x28, 0x02, 0x4b, 0xc7, 
                0xdd, 0x71, 0xa0, 0x39, 0xc4, 0xbe, 0x1a, 0xfe, 0xeb, 0xc2, 0x46, 0xda, 0x76, 
                0xf8, 0x07, 0x53, 0x3d, 0x96, 0xb4, 0xca, 0xa0, 0xe9 };
        memcpy(CCpriv,FaucetCCpriv,32);
    }
    virtual bool validate(Eval* eval, const CTransaction &tx, uint32_t nIn) override;    
};
