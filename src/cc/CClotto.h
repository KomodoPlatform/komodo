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

UniValue LottoInfo(uint256 lottoid);
UniValue LottoList();
std::string LottoTicket(uint64_t txfee,int64_t numtickets);
std::string LottoWinner(uint64_t txfee);

struct CCLottoContract_info : public CCcontract_info
{
    CCLottoContract_info() : CCcontract_info()
    {
        evalcode = EVAL_LOTTO;
        strcpy(unspendableCCaddr, "RNXZxgyWSAE6XS3qGnTaf5dVNCxnYzhPrg");
        strcpy(normaladdr, "RLW6hhRqBZZMBndnyPv29Yg3krh6iBYCyg");
        strcpy(CChexstr, "03f72d2c4db440df1e706502b09ca5fec73ffe954ea1883e4049e98da68690d98f");
        uint8_t LottoCCpriv[32] = { 0xb4, 0xac, 0xc2, 0xd9, 0x67, 0x34, 0xd7, 0x58, 0x80, 0x4e, 
                0x25, 0x55, 0xc0, 0x50, 0x66, 0x84, 0xbb, 0xa2, 0xe7, 0xc0, 0x39, 0x17, 0xb4, 
                0xc5, 0x07, 0xb7, 0x3f, 0xca, 0x07, 0xb0, 0x9a, 0xeb };
        memcpy(CCpriv, LottoCCpriv, 32); 
    }
    virtual bool validate(Eval* eval, const CTransaction &tx, uint32_t nIn) override;
};
