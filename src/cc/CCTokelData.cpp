/******************************************************************************
* Copyright  2014-2021 The SuperNET Developers.                             *
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

#include <stdexcept>
#include "CCtokens.h"
#include "CCTokelData.h"


static UniValue tklReadString(CDataStream &ss)
{
    std::string sval;
    ::Unserialize(ss, sval);
    UniValue ret(sval);
    return ret;
}

static UniValue tklReadInt64(CDataStream &ss)
{
    uint64_t ui64val;
    //::Unserialize(ss, i64val);
    ss >> COMPACTSIZE(ui64val);
    UniValue ret((int64_t)ui64val);
    return ret;
}

static UniValue tklReadVuint8(CDataStream &ss)
{
    vuint8_t vuint8val;
    ::Unserialize(ss, vuint8val);
    UniValue ret(HexStr(vuint8val));
    return ret;
}

static void tklWriteString(CDataStream &ss, const UniValue &val)
{
    std::string sval = val.getValStr();
    ::Serialize(ss, sval);
}

static void tklWriteInt64(CDataStream &ss, const UniValue &val)
{
    int64_t i64val = 0;
    ParseInt64(val.getValStr(), &i64val);
    //::Serialize(ss, i64val);
    ss << COMPACTSIZE((uint64_t)i64val);
}

static void tklWriteVuint8(CDataStream &ss, const UniValue &val)
{
    vuint8_t vuint8val = ParseHex(val.getValStr());
    ::Serialize(ss, vuint8val);
}

typedef std::map<tklPropId, tklPropDesc_t> tklPropDesc_map;
static const tklPropDesc_map tklPropDesc = {
    { TKLPROP_ID, std::make_tuple(TKLTYP_INT64, std::string("id"), &tklReadInt64, &tklWriteInt64) },
    { TKLPROP_URL, std::make_tuple(TKLTYP_VUINT8, std::string("url"), &tklReadString, &tklWriteString) },
    { TKLPROP_ROYALTY, std::make_tuple(TKLTYP_INT64, std::string("royalty"), &tklReadInt64, &tklWriteInt64) },
    { TKLPROP_ARBITRARY, std::make_tuple(TKLTYP_VUINT8, std::string("arbitrary"), &tklReadVuint8, &tklWriteVuint8) }
};

/*typedef struct {
    tklPropType type;
    struct
    {
        uint64_t uint64;
        vuint8_t vuint8;
    } v;
} tklPropValue;
*/

static tklPropId FindTokelDataIdByName(const std::string &name)
{
    auto found = std::find_if(tklPropDesc.begin(), tklPropDesc.end(), [&](const std::pair<tklPropId, tklPropDesc_t> &e){ return std::get<1>(e.second) == name; });
    if (found != tklPropDesc.end())
        return found->first;
    else
        return (tklPropId)0;
}

static tklPropDesc_t GetTokelDataDesc(tklPropId id)
{
    tklPropDesc_t empty = std::make_tuple( TKLTYP_INVALID, std::string(), nullptr, nullptr );
    auto found = tklPropDesc.find(id);
    if (found != tklPropDesc.end())
        return found->second;
    else
        return empty;
}

vuint8_t ParseTokelJson(const UniValue &jsonParams)
{
    uint8_t evalcode = EVAL_TOKELDATA;
    CDataStream ss(SER_NETWORK, PROTOCOL_VERSION);
    ss << evalcode << (uint8_t)TKLDATA_VERSION;
    for(int i = 0; i < jsonParams.getKeys().size(); i ++)
    {
        std::string key = jsonParams.getKeys()[i];
        tklPropId id;
        //CAmount valAmount;
        //vuint8_t valVuint8;
        if ((id = FindTokelDataIdByName(key)) == (tklPropId)0)
            throw std::runtime_error("invalid token data id=" + key);   
        tklPropDesc_t entry = GetTokelDataDesc(id);
        ss << (uint8_t)id;
        (*std::get<3>(entry))(ss, jsonParams[key]);
    }

    return vuint8_t(ss.begin(), ss.end());
}

static bool UnmarshalTokelVData(const vuint8_t &vdata, std::map<tklPropId, UniValue> &propMapOut, std::string &sError)
{
    if (vdata.size() >= 2 && vdata[0] == EVAL_TOKELDATA) 
    {
        uint8_t evalCode, version;
        bool invalidPropType = false;
        bool hasDuplicates = false;
        const char *funcname = __func__;
        //int mandatoryCount = 0;
        std::map<tklPropId, UniValue> propMap;

        if (vdata[1] != TKLDATA_VERSION)
        {
            LOGSTREAMFN(cctokens_log, CCLOG_DEBUG1, stream << "invalid tkl data version" << std::endl); 
            sError = "invalid tkl data version";
            return false;
        }

        try {
            CDataStream ss(vdata, SER_NETWORK, PROTOCOL_VERSION);
            ::Unserialize(ss, evalCode);
            ::Unserialize(ss, version);
            while(!ss.eof())  {
                uint8_t id;
                ::Unserialize(ss, id);
                tklPropDesc_t entry = GetTokelDataDesc((tklPropId)id);
                if (std::get<0>(entry) == TKLTYP_INVALID)  {
                    sError = "invalid tokel data property type";
                    return false;
                }
                if (propMap.count((tklPropId)id) != 0) {
                    sError = "duplicate tokel data property type";
                    return false;            
                }
                UniValue val = (*std::get<2>(entry))(ss);  // read ss
                propMap.insert(std::make_pair((tklPropId)id, val));
            }
            propMapOut = propMap;
            return true;
        } catch(...) {
            sError = "could not parse tokel token data";
            return false;
        }
    }
    sError = "invalid tokel data";
    return false;
}

UniValue ParseTokelVData(const vuint8_t &vdata)
{
    std::map<tklPropId, UniValue> propMap;
    std::string sError;
    UniValue result(UniValue::VOBJ);

    UnmarshalTokelVData(vdata, propMap, sError);
    int i = 0;
    for (auto const &p : propMap) {
        tklPropDesc_t entry = GetTokelDataDesc(p.first);
        if (std::get<0>(entry) != TKLTYP_INVALID)  
            result.pushKV(std::get<1>(entry), p.second);
        else 
            result.pushKV(std::string("unknown")+std::to_string(++i), p.second);
    }
    return result;
}

bool GetTokelDataAsInt64(const vuint8_t &vdata, tklPropId propId, int64_t &val)
{
    std::map<tklPropId, UniValue> propMap;
    std::string sError;
    UnmarshalTokelVData(vdata, propMap, sError);
    if (propMap.count(propId) > 0 && propMap[propId].isNum()) {
        ParseInt64(propMap[propId].getValStr(), &val);
        return true;
    }
    return false;
}

bool GetTokelDataAsVuint8(const vuint8_t &vdata, tklPropId propId, vuint8_t &val)
{
    std::map<tklPropId, UniValue> propMap;
    std::string sError;

    UnmarshalTokelVData(vdata, propMap, sError);
    if (propMap.count(propId) > 0) {
        val = ParseHex(propMap[propId].getValStr());
        return true;
    }
    return false;
}

static bool CheckTokelData(const vuint8_t &vdata, std::string &sError)
{
    std::map<tklPropId, UniValue> propMap;
    if( UnmarshalTokelVData(vdata, propMap, sError) )  {
        // check props if present
        if (propMap.count(TKLPROP_ROYALTY) > 0)  {
            int64_t val;
            if (!ParseInt64(propMap[TKLPROP_ROYALTY].getValStr(), &val)) {
                sError = "could not parse tokel royalty";
                return false;
            }
            if (val < 0 || val >= TKLROYALTY_DIVISOR) {
                sError = "invalid tokel royalty value (must be in 0...999)";
                return false;
            }
        }
        return true;
    }
    return false;    
}

bool ValidatePrevTxTokelOpretV1(struct CCcontract_info *cp, Eval* eval, const CTransaction &tx, const CScript &opret)
{
    uint256 tokenid;
    std::vector<CPubKey> vpks;
    std::vector<vuint8_t> voprets;
    if (DecodeTokenOpRetV1(opret, tokenid, vpks, voprets) == 0)
        return eval->Error("could not decode tkl data");
    
    for(auto const &vin : tx.vin)
    {
        if (vin.prevout.hash == tokenid)    // for token v1 check directly spent token create tx
        {
            CTransaction vintx;
            uint256 hashBlock;
            vuint8_t vorigpk;
            std::string name, desc;
            std::vector<vuint8_t> vcroprets;
            std::string sError;

            if (!myGetTransaction(vin.prevout.hash, vintx, hashBlock))
                return eval->Error("could load vintx");
            if (DecodeTokenCreateOpRetV1(vintx.vout.back().scriptPubKey, vorigpk, name, desc, vcroprets) == 0)
                return eval->Error("could not decode token create tx opreturn");
            if (vcroprets.size() == 0)
                return eval->Error("no tkl data in opreturn");
            if (!CheckTokelData(vcroprets[0], sError))
                return eval->Error("tkl data invalid: " + sError);
        }
    }
    return true;
}

// check token v2 create tx
bool ValidateTokelOpretV2(struct CCcontract_info *cp, Eval* eval, const CTransaction &tx, const CScript &opret)
{
    vuint8_t vorigpk;
    std::string name, desc;
    std::vector<vuint8_t> vcroprets;

    if (IsTokenCreateFuncid(DecodeTokenCreateOpRetV2(opret, vorigpk, name, desc, vcroprets)))
    {
        std::string sError;

        if (vcroprets.size() == 0)
            return eval->Error("no tkl data in opreturn");
        if (!CheckTokelData(vcroprets[0], sError))
            return eval->Error("tkl data invalid: " + sError);
    }
    return true;
}

bool TokelDataValidate(struct CCcontract_info *cp, Eval* eval, const CTransaction &tx, uint32_t nIn)
{
    //if (strcmp(ASSETCHAINS_SYMBOL, "TKLTEST") == 0 && chainActive.Height() <= 33711)
    //    return true;
    //if (strcmp(ASSETCHAINS_SYMBOL, "DIMXY20") == 0 && chainActive.Height() <= 1704)
    //    return true;

    if (tx.vout.size() < 1)
        return eval->Error("no vouts");

    vuint8_t vopret;
    uint8_t evalTokens;
    int goodTokenData = 0;

    // check if this is a token v2 create tx
    if (GetOpReturnData(tx.vout.back().scriptPubKey, vopret) &&
        vopret.size() > 2 &&
        vopret[0] == EVAL_TOKENSV2)
    {
        if (IsTokenCreateFuncid(vopret[1]))
            return ValidateTokelOpretV2(cp, eval, tx, tx.vout.back().scriptPubKey);
        else
            return true; // do not check further tokens txns
    }

    // for token v1 we do not have create tx in validation
    // let's find TKL vout and check if previous tx 
    for (int i = 0; i < tx.vout.size(); i ++)   {
        //uint256 tokenid;
        //std::string errstr;
        if (tx.vout[i].scriptPubKey.IsPayToCryptoCondition()) 
        {
            CScript ccdata;
            vuint8_t vopdrop;
            if (!(ccdata = GetCCDropAsOpret(tx.vout[i].scriptPubKey)).empty() &&
                GetOpReturnData(ccdata, vopdrop) &&
                vopdrop.size() > 2 &&
                vopdrop[0] == EVAL_TOKENS)
            {
                if( !ValidatePrevTxTokelOpretV1(cp, eval, tx, ccdata) )
                    return false;  // eval is set
            }
        }
    }

    // if last vout v1 opreturn exists
    if (vopret.size() > 2 &&
        vopret[0] == EVAL_TOKENS)
    {
        if (!ValidatePrevTxTokelOpretV1(cp, eval, tx, tx.vout.back().scriptPubKey))
            return false;
    }
    return true; // no prev tx is token create tx
}

