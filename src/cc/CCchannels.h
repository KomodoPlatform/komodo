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
#define CHANNELS_MAXPAYMENTS 1000

UniValue ChannelOpen(const CPubKey& pk,uint64_t txfee,CPubKey destpub,int32_t numpayments,int64_t payment,uint256 tokenid);
UniValue ChannelPayment(const CPubKey& pk,uint64_t txfee,uint256 opentxid,int64_t amount, uint256 secret);
UniValue ChannelClose(const CPubKey& pk,uint64_t txfee,uint256 opentxid);
UniValue ChannelRefund(const CPubKey& pk,uint64_t txfee,uint256 opentxid,uint256 closetxid);
UniValue ChannelsList(const CPubKey& pk);
// CCcustom
UniValue ChannelsInfo(const CPubKey& pk,uint256 opentxid);

struct CCChannelsContract_info : public CCcontract_info
{
    CCChannelsContract_info() : CCcontract_info()
    {

        evalcode = EVAL_CHANNELS;
        strcpy(unspendableCCaddr, "RQy3rwX8sP9oDm3c39vGKA6H315cgtPLfr");
        strcpy(normaladdr, "RQUuT8zmkvDfXqECH4m3VD3SsHZAfnoh1v");
        strcpy(CChexstr, "035debdb19b1c98c615259339500511d6216a3ffbeb28ff5655a7ef5790a12ab0b");
        uint8_t ChannelsCCpriv[32] = { 0xec, 0x91, 0x36, 0x15, 0x2d, 0xd4, 0x48, 0x73, 0x22, 
                0x36, 0x4f, 0x6a, 0x34, 0x5c, 0x61, 0x0f, 0x01, 0xb4, 0x79, 0xe8, 0x1c, 0x2f, 
                0xa1, 0x1d, 0x4a, 0x0a, 0x21, 0x16, 0xea, 0x82, 0x84, 0x60 };
        memcpy(CCpriv, ChannelsCCpriv,32);
    }
    virtual bool validate(Eval* eval, const CTransaction &tx, uint32_t nIn) override;
};
