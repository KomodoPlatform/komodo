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

#define EVAL_AUCTION 0xe8

std::string AuctionPost(uint64_t txfee,uint256 itemhash,int64_t minbid,char *title,char *description);
std::string AuctionBid(uint64_t txfee,uint256 itemhash,int64_t amount);
std::string AuctionDeliver(uint64_t txfee,uint256 itemhash,uint256 bidtxid);

struct CCAuctionContract_info : public CCcontract_info
{
    CCAuctionContract_info() : CCcontract_info()
    {
        evalcode = EVAL_AUCTION;
        strcpy(unspendableCCaddr,"RL4YPX7JYG3FnvoPqWF2pn3nQknH5NWEwx");
        strcpy(normaladdr,"RFtVDNmdTZBTNZdmFRbfBgJ6LitgTghikL");
        strcpy(CChexstr,"037eefe050c14cb60ae65d5b2f69eaa1c9006826d729bc0957bdc3024e3ca1dbe6");
        uint8_t AuctionCCpriv[32] = { 0x8c, 0x1b, 0xb7, 0x8c, 0x02, 0xa3, 0x9d, 0x21, 0x28, 
                0x59, 0xf5, 0xea, 0xda, 0xec, 0x0d, 0x11, 0xcd, 0x38, 0x47, 0xac, 0x0b, 0x6f, 
                0x19, 0xc0, 0x24, 0x36, 0xbf, 0x1c, 0x0a, 0x06, 0x31, 0xfb };
        memcpy(CCpriv,AuctionCCpriv,32);
    }
    virtual bool validate(Eval* eval, const CTransaction &tx, uint32_t nIn) override;
};
