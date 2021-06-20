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

UniValue OracleCreate(const CPubKey& pk, int64_t txfee,std::string name,std::string description,std::string format);
UniValue OracleFund(const CPubKey& pk, int64_t txfee,uint256 oracletxid);
UniValue OracleRegister(const CPubKey& pk, int64_t txfee,uint256 oracletxid,int64_t datafee);
UniValue OracleSubscribe(const CPubKey& pk, int64_t txfee,uint256 oracletxid,CPubKey publisher,int64_t amount);
UniValue OracleData(const CPubKey& pk, int64_t txfee,uint256 oracletxid,std::vector <uint8_t> data);
// CCcustom
UniValue OracleDataSample(uint256 reforacletxid,uint256 txid);
UniValue OracleDataSamples(uint256 reforacletxid,char* batonaddr,int32_t num);
UniValue OracleInfo(uint256 origtxid);
UniValue OraclesList();

struct CCOraclesContract_info : public CCcontract_info
{
    CCOraclesContract_info() : CCcontract_info()
    {
        evalcode = EVAL_ORACLES;
        strcpy(unspendableCCaddr, "REt2C4ZMnX8YYX1DRpffNA4hECZTFm39e3");
        strcpy(normaladdr, "RHkFKzn1csxA3fWzAsxsLWohoCgBbirXb5");
        strcpy(CChexstr, "038c1d42db6a45a57eccb8981b078fb7857b9b496293fe299d2b8d120ac5b5691a");
        uint8_t OraclesCCpriv[32] = { 0xf7, 0x4b, 0x5b, 0xa2, 0x7a, 0x5e, 0x9c, 0xda, 0x89, 
                0xb1, 0xcb, 0xb9, 0xe6, 0x9c, 0x2c, 0x70, 0x85, 0x37, 0xdd, 0x00, 0x7a, 
                0x67, 0xff, 0x7c, 0x62, 0x1b, 0xe2, 0xfb, 0x04, 0x8f, 0x85, 0xbf };
        memcpy(CCpriv, OraclesCCpriv, 32); 
    }
    virtual bool validate(Eval* eval, const CTransaction &tx, uint32_t nIn) override;
};
