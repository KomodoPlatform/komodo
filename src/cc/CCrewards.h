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

#define EVAL_REWARDS 0xe5
#define REWARDSCC_MAXAPR (COIN * 25)

bool RewardsValidate(struct CCcontract_info *cp,Eval* eval,const CTransaction &tx, uint32_t nIn);
UniValue RewardsInfo(uint256 rewardid);
UniValue RewardsList();

std::string RewardsCreateFunding(uint64_t txfee,char *planstr,int64_t funds,int64_t APR,int64_t minseconds,int64_t maxseconds,int64_t mindeposit);
std::string RewardsAddfunding(uint64_t txfee,char *planstr,uint256 fundingtxid,int64_t amount);
std::string RewardsLock(uint64_t txfee,char *planstr,uint256 fundingtxid,int64_t amount);
std::string RewardsUnlock(uint64_t txfee,char *planstr,uint256 fundingtxid,uint256 locktxid);

struct CCRewardsContract_info : public CCcontract_info
{
    CCRewardsContract_info() : CCcontract_info()
    {
        evalcode = EVAL_REWARDS;
        strcpy(unspendableCCaddr,"RTsRBYL1HSvMoE3qtBJkyiswdVaWkm8YTK");
        strcpy(normaladdr,"RMgye9jeczNjQx9Uzq8no8pTLiCSwuHwkz");
        strcpy(CChexstr,"03da60379d924c2c30ac290d2a86c2ead128cb7bd571f69211cb95356e2dcc5eb9");
        uint8_t RewardsCCpriv[32] = { 0x82, 0xf5, 0xd2, 0xe7, 0xd6, 0x99, 0x33, 0x77, 0xfb, 0x80, 
                0x00, 0x97, 0x23, 0x3d, 0x1e, 0x6f, 0x61, 0xa9, 0xb5, 0x2e, 0x5e, 0xb4, 0x96, 
                0x6f, 0xbc, 0xed, 0x6b, 0xe2, 0xbb, 0x7b, 0x4b, 0xb3 };
        memcpy(CCpriv,RewardsCCpriv,32);
    }
    virtual bool validate(Eval* eval, const CTransaction &tx, uint32_t nIn) override;
};
