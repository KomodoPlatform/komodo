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


/*
 CCassetstx has the functions that create the EVAL_ASSETS transactions. It is expected that rpc calls would call these functions. For EVAL_ASSETS, the rpc functions are in rpcwallet.cpp
 
 CCassetsCore has functions that are used in two contexts, both during rpc transaction create time and also during the blockchain validation. Using the identical functions is a good way to prevent them from being mismatched. The must match or the transaction will get rejected.
 */

#include "CCinclude.h"

#include "CCtokens.h"

// CCcustom
bool AssetsValidate(struct CCcontract_info *cp,Eval* eval,const CTransaction &tx, uint32_t nIn);

// CCassetsCore
vscript_t EncodeAssetOpRet(uint8_t assetFuncId, uint256 assetid2, int64_t price, std::vector<uint8_t> origpubkey);
uint8_t DecodeAssetTokenOpRet(const CScript &scriptPubKey, uint8_t &assetsEvalCode, uint256 &tokenid, uint256 &assetid2, int64_t &price, std::vector<uint8_t> &origpubkey);
bool SetAssetOrigpubkey(std::vector<uint8_t> &origpubkey,int64_t &price,const CTransaction &tx);
int64_t IsAssetvout(struct CCcontract_info *cp, int64_t &price, std::vector<uint8_t> &origpubkey, const CTransaction& tx, int32_t v, uint256 refassetid);
bool ValidateBidRemainder(int64_t remaining_price,int64_t remaining_nValue,int64_t orig_nValue,int64_t received_nValue,int64_t paidprice,int64_t totalprice);
bool ValidateAskRemainder(int64_t remaining_price,int64_t remaining_nValue,int64_t orig_nValue,int64_t received_nValue,int64_t paidprice,int64_t totalprice);
bool ValidateSwapRemainder(int64_t remaining_price,int64_t remaining_nValue,int64_t orig_nValue,int64_t received_nValue,int64_t paidprice,int64_t totalprice);
bool SetBidFillamounts(int64_t &paid,int64_t &remaining_price,int64_t orig_nValue,int64_t &received,int64_t totalprice);
bool SetAskFillamounts(int64_t &paid,int64_t &remaining_price,int64_t orig_nValue,int64_t &received,int64_t totalprice);
bool SetSwapFillamounts(int64_t &paid,int64_t &remaining_price,int64_t orig_nValue,int64_t &received,int64_t totalprice);
int64_t AssetValidateBuyvin(struct CCcontract_info *cp,Eval* eval,int64_t &tmpprice,std::vector<uint8_t> &tmporigpubkey,char *CCaddr,char *origaddr,const CTransaction &tx,uint256 refassetid);
int64_t AssetValidateSellvin(struct CCcontract_info *cp,Eval* eval,int64_t &tmpprice,std::vector<uint8_t> &tmporigpubkey,char *CCaddr,char *origaddr,const CTransaction &tx,uint256 assetid);
bool AssetCalcAmounts(struct CCcontract_info *cpAssets, int64_t &inputs, int64_t &outputs, Eval* eval, const CTransaction &tx, uint256 assetid);

// CCassetstx
int64_t AddAssetInputs(struct CCcontract_info *cp, CMutableTransaction &mtx, CPubKey pk, uint256 assetid, int64_t total, int32_t maxinputs);

UniValue AssetOrders(uint256 tokenid, CPubKey pubkey, uint8_t additionalEvalCode);

std::string CreateBuyOffer(int64_t txfee,int64_t bidamount,uint256 assetid,int64_t pricetotal);
std::string CancelBuyOffer(int64_t txfee,uint256 assetid,uint256 bidtxid);
std::string FillBuyOffer(int64_t txfee,uint256 assetid,uint256 bidtxid,int64_t fillamount);
std::string CreateSell(int64_t txfee,int64_t askamount,uint256 assetid,int64_t pricetotal);
std::string CreateSwap(int64_t txfee,int64_t askamount,uint256 assetid,uint256 assetid2,int64_t pricetotal);
std::string CancelSell(int64_t txfee,uint256 assetid,uint256 asktxid);
std::string FillSell(int64_t txfee,uint256 assetid,uint256 assetid2,uint256 asktxid,int64_t fillamount);

struct CCAssetContract_info : public CCcontract_info
{
    CCAssetContract_info() : CCcontract_info()
    {
        evalcode = EVAL_ASSETS;
        strcpy(unspendableCCaddr, "RGKRjeTBw4LYFotSDLT6RWzMHbhXri6BG6");
        strcpy(normaladdr, "RFYE2yL3KknWdHK6uNhvWacYsCUtwzjY3u");
        strcpy(CChexstr, "02adf84e0e075cf90868bd4e3d34a03420e034719649c41f371fc70d8e33aa2702");
        uint8_t AssetsCCpriv[32] = { 0x9b, 0x17, 0x66, 0xe5, 0x82, 0x66, 0xac, 0xb6, 0xba, 0x43, 
                0x83, 0x74, 0xf7, 0x63, 0x11, 0x3b, 0xf0, 0xf3, 0x50, 0x6f, 0xd9, 0x6b, 0x67, 
                0x85, 0xf9, 0x7a, 0xf0, 0x54, 0x4d, 0xb1, 0x30, 0x77 };
        memcpy(CCpriv, AssetsCCpriv,32);
    }
    virtual bool validate(Eval* eval, const CTransaction &tx, uint32_t nIn) override;
};

