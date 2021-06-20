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

#include "komodo_defs.h"
#include "CCinclude.h"

int32_t komodo_priceget(int64_t *buf64,int32_t ind,int32_t height,int32_t numblocks);
extern void GetKomodoEarlytxidScriptPub();
extern CScript KOMODO_EARLYTXID_SCRIPTPUB;

// #define PRICES_DAYWINDOW ((3600*24/ASSETCHAINS_BLOCKTIME) + 1) // defined in komodo_defs.h
#define PRICES_TXFEE 10000
#define PRICES_MAXLEVERAGE 777
#define PRICES_SMOOTHWIDTH 1
#define KOMODO_MAXPRICES 2048 // must be power of 2 and less than 8192
#define KOMODO_PRICEMASK (~(KOMODO_MAXPRICES -  1))     // actually 1111 1000 0000 0000
#define PRICES_WEIGHT (KOMODO_MAXPRICES * 1)            //          0000 1000 0000 0000
#define PRICES_MULT (KOMODO_MAXPRICES * 2)              //          0001 0000 0000 0000
#define PRICES_DIV (KOMODO_MAXPRICES * 3)               //          0001 1000 0000 0000
#define PRICES_INV (KOMODO_MAXPRICES * 4)               //          0010 0000 0000 0000
#define PRICES_MDD (KOMODO_MAXPRICES * 5)               //          0010 1000 0000 0000
#define PRICES_MMD (KOMODO_MAXPRICES * 6)               //          0011 0000 0000 0000
#define PRICES_MMM (KOMODO_MAXPRICES * 7)               //          0011 1000 0000 0000
#define PRICES_DDD (KOMODO_MAXPRICES * 8)               //          0100 0000 0000 0000

//#define PRICES_NORMFACTOR   (int64_t)(SATOSHIDEN)
//#define PRICES_POINTFACTOR   (int64_t)10000

#define PRICES_REVSHAREDUST 10000
#define PRICES_SUBREVSHAREFEE(amount) ((amount) * 199 / 200)    // revshare fee percentage == 0.005
#define PRICES_MINAVAILFUNDFRACTION  0.1                             // leveraged bet limit < fund fraction

// CCcustom
UniValue PricesBet(int64_t txfee,int64_t amount,int16_t leverage,std::vector<std::string> synthetic);
UniValue PricesAddFunding(int64_t txfee,uint256 bettxid,int64_t amount);
UniValue PricesSetcostbasis(int64_t txfee,uint256 bettxid);
UniValue PricesRekt(int64_t txfee,uint256 bettxid,int32_t rektheight);
UniValue PricesCashout(int64_t txfee,uint256 bettxid);
UniValue PricesInfo(uint256 bettxid,int32_t refheight);
UniValue PricesList(uint32_t filter, CPubKey mypk);
UniValue PricesGetOrderbook();
UniValue PricesRefillFund(int64_t amount);

struct CCPricesContract_info : public CCcontract_info
{
    CCPricesContract_info() : CCcontract_info()
    {
        evalcode = EVAL_PRICES;
        strcpy(unspendableCCaddr, "RAL5Vh8NXmFqEKJRKrk1KjKaUckK7mM1iS");
        strcpy(normaladdr, "RBunXCsMHk5NPd6q8SQfmpgre3x133rSwZ");
        strcpy(CChexstr, "039894cb054c0032e99e65e715b03799607aa91212a16648d391b6fa2cc52ed0cf");
        uint8_t PricesCCpriv[32] = { 0x0a, 0x3b, 0xe7, 0x5d, 0xce, 0x06, 0xed, 0xb7, 0xc0, 
                0xb1, 0xbe, 0xe8, 0x7b, 0x5a, 0xd4, 0x99, 0xb8, 0x8d, 0xde, 0xac, 0xb2, 0x7e, 
                0x7a, 0x52, 0x96, 0x15, 0xd2, 0xa0, 0xc6, 0xb9, 0x89, 0x61 };
        memcpy(CCpriv, PricesCCpriv, 32); 
    }
    virtual bool validate(Eval* eval, const CTransaction &tx, uint32_t nIn) override;
};
