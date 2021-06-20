/******************************************************************************
 * Copyright © 2014-2018 The SuperNET Developers.                             *
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

// CCcustom
bool ImportGatewayExactAmounts(bool goDeeper, struct CCcontract_info *cpTokens, int64_t &inputs, int64_t &outputs, Eval* eval, const CTransaction &tx, uint256 tokenid);
std::string ImportGatewayBind(uint64_t txfee,std::string coin,uint256 oracletxid,uint8_t M,uint8_t N,std::vector<CPubKey> pubkeys,uint8_t p1,uint8_t p2,uint8_t p3,uint8_t p4);
std::string ImportGatewayDeposit(uint64_t txfee,uint256 bindtxid,int32_t height,std::string refcoin,uint256 burntxid,int32_t burnvout,std::string rawburntx,std::vector<uint8_t>proof,CPubKey destpub,int64_t amount);
std::string ImportGatewayWithdraw(uint64_t txfee,uint256 bindtxid,std::string refcoin,CPubKey withdrawpub,int64_t amount);
std::string ImportGatewayPartialSign(uint64_t txfee,uint256 lasttxid,std::string refcoin, std::string hex);
std::string ImportGatewayCompleteSigning(uint64_t txfee,uint256 lasttxid,std::string refcoin,std::string hex);
std::string ImportGatewayMarkDone(uint64_t txfee,uint256 completetxid,std::string refcoin);
UniValue ImportGatewayPendingWithdraws(uint256 bindtxid,std::string refcoin);
UniValue ImportGatewayProcessedWithdraws(uint256 bindtxid,std::string refcoin);
UniValue ImportGatewayExternalAddress(uint256 bindtxid,CPubKey pubkey);
UniValue ImportGatewayDumpPrivKey(uint256 bindtxid,CKey key);
UniValue ImportGatewayList();
UniValue ImportGatewayInfo(uint256 bindtxid);

struct CCImportGatewayContract_info : public CCcontract_info
{
    CCImportGatewayContract_info() : CCcontract_info()
    {
        evalcode = EVAL_IMPORTGATEWAY;
        strcpy(unspendableCCaddr, "RXJT6CRAXHFuQ2UjqdxMj7EfrayF6UJpzZ");
        strcpy(normaladdr, "RNFRho63Ddz1Rh2eGPETykrU4fA8r67S4Y");
        strcpy(CChexstr, "0397231cfe04ea32d5fafb2206773ec9fba6e15c5a4e86064468bca195f7542714");
        uint8_t ImportGatewayCCpriv[32] = { 0x65, 0xef, 0x27, 0xeb, 0x3d, 0xb0, 0xb4, 0xae, 0x0f, 0xbc, 0x77, 0xdb, 0xf8, 0x40, 0x48, 0x90, 0x52, 0x20, 0x9e, 0x45, 0x3b, 0x49, 0xd8, 0x97, 0x60, 0x8c, 0x27, 0x4c, 0x59, 0x46, 0xe1, 0xdf };
        memcpy(CCpriv, ImportGatewayCCpriv, 32); 
    }
    virtual bool validate(Eval* eval, const CTransaction &tx, uint32_t nIn) override;
};