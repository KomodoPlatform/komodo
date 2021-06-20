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
#include "CCcontract.h"

#define EVAL_DICE 0xe6

bool DiceValidate(struct CCcontract_info *cp,Eval* eval,const CTransaction &tx, uint32_t nIn);

std::string DiceBet(uint64_t txfee,char *planstr,uint256 fundingtxid,int64_t bet,int32_t odds);
std::string DiceBetFinish(uint8_t &funcid,uint256 &entropyused,int32_t &entropyvout,int32_t *resultp,uint64_t txfee,char *planstr,uint256 fundingtxid,uint256 bettxid,int32_t winlosetimeout,uint256 vin0txid,int32_t vin0vout);
double DiceStatus(uint64_t txfee,char *planstr,uint256 fundingtxid,uint256 bettxid);
std::string DiceCreateFunding(uint64_t txfee,char *planstr,int64_t funds,int64_t minbet,int64_t maxbet,int64_t maxodds,int64_t timeoutblocks);
std::string DiceAddfunding(uint64_t txfee,char *planstr,uint256 fundingtxid,int64_t amount);
UniValue DiceInfo(uint256 diceid);
UniValue DiceList();
int64_t DicePlanFunds(uint64_t &entropyval,uint256 &entropytxid,uint64_t refsbits,struct CCcontract_info *cp,CPubKey dicepk,uint256 reffundingtxid, int32_t &entropytxs,bool random);


struct CCDiceContract_info : public CCcontract_info
{
    CCDiceContract_info() : CCcontract_info()
    {
        evalcode = EVAL_DICE;
        strcpy(unspendableCCaddr, "REabWB7KjFN5C3LFMZ5odExHPenYzHLtVw");
        strcpy(normaladdr, "RLEe8f7Eg3TDuXii9BmNiiiaVGraHUt25c");
        strcpy(CChexstr, "039d966927cfdadab3ee6c56da63c21f17ea753dde4b3dfd41487103e24b27e94e");
        uint8_t DiceCCpriv[32] = { 0x0e, 0xe8, 0xf5, 0xb4, 0x3d, 0x25, 0xcc, 0x35, 0xd1, 0xf1, 0x2f, 0x04, 0x5f, 0x01, 0x26, 0xb8, 0xd1, 0xac, 0x3a, 0x5a, 0xea, 0xe0, 0x25, 0xa2, 0x8f, 0x2a, 0x8e, 0x0e, 0xf9, 0x34, 0xfa, 0x77 };
        memcpy(CCpriv, DiceCCpriv, 32); 
    }
    virtual bool validate(Eval* eval, const CTransaction &tx, uint32_t nIn) override;
};

