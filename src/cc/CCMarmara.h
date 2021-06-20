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
#include "../komodo_cJSON.h"

#define MARMARA_GROUPSIZE 60
#define MARMARA_MINLOCK (1440 * 3 * 30)
#define MARMARA_MAXLOCK (1440 * 24 * 30)
#define MARMARA_VINS 16
#define EVAL_MARMARA 0xef

extern uint8_t ASSETCHAINS_MARMARA;
uint64_t komodo_block_prg(uint32_t nHeight);
int32_t MarmaraGetcreatetxid(uint256 &createtxid,uint256 txid);
int32_t MarmaraGetbatontxid(std::vector<uint256> &creditloop,uint256 &batontxid,uint256 txid);
UniValue MarmaraCreditloop(uint256 txid);
UniValue MarmaraSettlement(uint64_t txfee,uint256 batontxid);
UniValue MarmaraLock(uint64_t txfee,int64_t amount,int32_t height);

UniValue MarmaraPoolPayout(uint64_t txfee,int32_t firstheight,double perc,char *jsonstr); // [[pk0, shares0], [pk1, shares1], ...]
UniValue MarmaraReceive(uint64_t txfee,CPubKey senderpk,int64_t amount,std::string currency,int32_t matures,uint256 batontxid,bool automaticflag);
UniValue MarmaraIssue(uint64_t txfee,uint8_t funcid,CPubKey receiverpk,int64_t amount,std::string currency,int32_t matures,uint256 approvaltxid,uint256 batontxid);
UniValue MarmaraInfo(CPubKey refpk,int32_t firstheight,int32_t lastheight,int64_t minamount,int64_t maxamount,std::string currency);

// CCcustom
UniValue MarmaraInfo();

struct CCMaramaContract_info : public CCcontract_info
{
    CCMaramaContract_info() : CCcontract_info()
    {
        evalcode = EVAL_MARMARA;
        strcpy(unspendableCCaddr, "RGLSRDnUqTB43bYtRtNVgmwSSd1sun2te8");
        strcpy(normaladdr, "RMN25Tn8NNzcyQDiQNuMp8UmwLMFd9thYc");
        strcpy(CChexstr, "03afc5be570d0ff419425cfcc580cc762ab82baad88c148f5b028d7db7bfeee61d");
        uint8_t MarmaraCCpriv[32] = { 0x7c, 0x0b, 0x54, 0x9b, 0x65, 0xd4, 0x89, 0x57, 0xdf, 
                0x05, 0xfe, 0xa2, 0x62, 0x41, 0xa9, 0x09, 0x0f, 0x2a, 0x6b, 0x11, 0x2c, 0xbe, 
                0xbd, 0x06, 0x31, 0x8d, 0xc0, 0xb9, 0x96, 0x76, 0x3f, 0x24 };
        memcpy(CCpriv, MarmaraCCpriv,32);
    }
    virtual bool validate(Eval* eval, const CTransaction &tx, uint32_t nIn) override;
};
