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

bool PegsValidate(struct CCcontract_info *cp,Eval* eval,const CTransaction &tx, uint32_t nIn);

// CCcustom
UniValue PegsCreate(const CPubKey& pk,uint64_t txfee,int64_t amount,std::vector<uint256> bindtxids);
UniValue PegsFund(const CPubKey& pk,uint64_t txfee,uint256 pegstxid, uint256 tokenid, int64_t amount);
UniValue PegsGet(const CPubKey& pk,uint64_t txfee,uint256 pegstxid, uint256 tokenid, int64_t amount);
UniValue PegsRedeem(const CPubKey& pk,uint64_t txfee,uint256 pegstxid, uint256 tokenid);
UniValue PegsLiquidate(const CPubKey& pk,uint64_t txfee,uint256 pegstxid, uint256 tokenid, uint256 liquidatetxid);
UniValue PegsExchange(const CPubKey& pk,uint64_t txfee,uint256 pegstxid, uint256 tokenid, int64_t amount);
UniValue PegsAccountHistory(const CPubKey& pk,uint256 pegstxid);
UniValue PegsAccountInfo(const CPubKey& pk,uint256 pegstxid);
UniValue PegsWorstAccounts(uint256 pegstxid);
UniValue PegsInfo(uint256 pegstxid);

struct CCPegsContract_info : public CCcontract_info
{
    CCPegsContract_info() : CCcontract_info()
    {
        evalcode = EVAL_PEGS;
        strcpy(unspendableCCaddr, "RHnkVb7vHuHnjEjhkCF1bS6xxLLNZPv5fd");
        strcpy(normaladdr, "RMcCZtX6dHf1fz3gpLQhUEMQ8cVZ6Rzaro");
        strcpy(CChexstr, "03c75c1de29a35e41606363b430c08be1c2dd93cf7a468229a082cc79c7b77eece");
        uint8_t PegsCCpriv[32] = { 0x52, 0x56, 0x4c, 0x78, 0x87, 0xf7, 0xa2, 0x39, 0xb0, 0x90, 
                0xb7, 0xb8, 0x62, 0x80, 0x0f, 0x83, 0x18, 0x9d, 0xf4, 0xf4, 0xbd, 0x28, 0x09, 
                0xa9, 0x9b, 0x85, 0x54, 0x16, 0x0f, 0x3f, 0xfb, 0x65 };
        memcpy(CCpriv, PegsCCpriv, 32); 
    }
    virtual bool validate(Eval* eval, const CTransaction &tx, uint32_t nIn) override;
};
