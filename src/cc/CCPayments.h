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
#include <gmp.h>
#include <key_io.h>

#define PAYMENTS_TXFEE 10000
#define PAYMENTS_MERGEOFSET 60 // 1H extra. 
extern std::vector <std::pair<CAmount, CTxDestination>> vAddressSnapshot;
extern int32_t lastSnapShotHeight;

// CCcustom
UniValue PaymentsRelease(struct CCcontract_info *cp,char *jsonstr);
UniValue PaymentsFund(struct CCcontract_info *cp,char *jsonstr);
UniValue PaymentsMerge(struct CCcontract_info *cp,char *jsonstr);
UniValue PaymentsTxidopret(struct CCcontract_info *cp,char *jsonstr);
UniValue PaymentsCreate(struct CCcontract_info *cp,char *jsonstr);
UniValue PaymentsAirdrop(struct CCcontract_info *cp,char *jsonstr);
UniValue PaymentsAirdropTokens(struct CCcontract_info *cp,char *jsonstr);
UniValue PaymentsInfo(struct CCcontract_info *cp,char *jsonstr);
UniValue PaymentsList(struct CCcontract_info *cp,char *jsonstr);

struct CCPaymentsContract_info : public CCcontract_info
{
    CCPaymentsContract_info() : CCcontract_info()
    {
        evalcode = EVAL_PAYMENTS;
        strcpy(unspendableCCaddr, "REpyKi7avsVduqZ3eimncK4uKqSArLTGGK");
        strcpy(normaladdr, "RHRX8RTMAh2STWe9DHqsvJbzS7ty6aZy3d");
        strcpy(CChexstr, "0358f1764f82c63abc7c7455555fd1d3184905e30e819e97667e247e5792b46856");
        uint8_t PaymentsCCpriv[32] = { 0x03, 0xc9, 0x73, 0xc2, 0xb8, 0x30, 0x3d, 0xbd, 0xc8, 
                0xd9, 0xbf, 0x02, 0x49, 0xd9, 0x65, 0x61, 0x45, 0xed, 0x9e, 0x93, 0x51, 0xab, 
                0x8b, 0x2e, 0xe7, 0xc7, 0x40, 0xf1, 0xc4, 0xd2, 0xc0, 0x5b };
        memcpy(CCpriv, PaymentsCCpriv, 32); 
    }
    virtual bool validate(Eval* eval, const CTransaction &tx, uint32_t nIn) override;
};
