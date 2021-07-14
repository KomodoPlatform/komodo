/******************************************************************************
 * Copyright © 2014-2021 The SuperNET Developers.                             *
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

#include "CCassets.h"
#include "CCtokens.h"
#include <iomanip> 

vscript_t EncodeAssetOpRetV1(uint8_t assetFuncId, uint256 assetid2, int64_t unit_price, std::vector<uint8_t> origpubkey)
{
    vscript_t vopret; 
	uint8_t evalcode = EVAL_ASSETS;
    uint8_t version = 1;

    switch ( assetFuncId )
    {
        //case 't': this cannot be here
		case 'x': case 'o':
			vopret = E_MARSHAL(ss << evalcode << assetFuncId << version);
            break;
        case 's': case 'b': case 'S': case 'B':
            vopret = E_MARSHAL(ss << evalcode << assetFuncId << version << unit_price << origpubkey);
            break;
        case 'E': case 'e':
            assetid2 = revuint256(assetid2);
            vopret = E_MARSHAL(ss << evalcode << assetFuncId << version << assetid2 << unit_price << origpubkey);
            break;
        default:
            CCLogPrintF(ccassets_log, CCLOG_DEBUG1, "%s illegal funcid.%02x\n", __func__, assetFuncId);
            break;
    }
    return(vopret);
}

uint8_t DecodeAssetTokenOpRetV1(const CScript &scriptPubKey, uint8_t &assetsEvalCode, uint256 &tokenid, uint256 &assetid2, int64_t &unit_price, std::vector<uint8_t> &origpubkey)
{
    vscript_t vopretAssets; //, vopretAssetsStripped;
	uint8_t *script, funcId = 0, assetsFuncId = 0, dummyAssetFuncId, dummyEvalCode, version;
	uint256 dummyTokenid;
	std::vector<CPubKey> voutPubkeysDummy;
    std::vector<vscript_t>  oprets;

	tokenid = zeroid;
	assetid2 = zeroid;
	unit_price = 0;
    assetsEvalCode = 0;
    assetsFuncId = 0;

	// First - decode token opret:
	funcId = DecodeTokenOpRetV1(scriptPubKey, tokenid, voutPubkeysDummy, oprets);
    GetOpReturnCCBlob(oprets, vopretAssets);

    LOGSTREAMFN(ccassets_log, CCLOG_DEBUG2, stream << "from DecodeTokenOpRet returned funcId=" << (int)funcId << std::endl);

	if (funcId == 0 || vopretAssets.size() < 2) {
        LOGSTREAMFN(ccassets_log, CCLOG_DEBUG1, stream << "incorrect opret or no asset's payload" << " funcId=" << (int)funcId << " vopretAssets.size()=" << vopretAssets.size() << std::endl);
		return (uint8_t)0;
	}

    // additional check to prevent crash
    if (vopretAssets.size() >= 2) {

        assetsEvalCode = vopretAssets.begin()[0];
        assetsFuncId = vopretAssets.begin()[1];

        LOGSTREAMFN(ccassets_log, CCLOG_DEBUG2, stream << "assetsEvalCode=" << (int)assetsEvalCode <<  " funcId=" << (char)(funcId ? funcId : ' ') << " assetsFuncId=" << (char)(assetsFuncId ? assetsFuncId : ' ') << std::endl);

        if (assetsEvalCode == EVAL_ASSETS)
        {
            //fprintf(stderr,"DecodeAssetTokenOpRet() decode.[%c] assetFuncId.[%c]\n", funcId, assetFuncId);
            switch (assetsFuncId)
            {
            case 'x': case 'o':
                if (vopretAssets.size() == 3)   // format is 'evalcode funcid version' 
                {
                    return(assetsFuncId);
                }
                break;
            case 's': case 'b': case 'S': case 'B':
                if (E_UNMARSHAL(vopretAssets, ss >> dummyEvalCode; ss >> dummyAssetFuncId; ss >> version; ss >> unit_price; ss >> origpubkey) != 0)
                {
                    //fprintf(stderr,"DecodeAssetTokenOpRet() got price %lld\n",(long long)price);
                    return(assetsFuncId);
                }
                break;
            case 'E': case 'e':
                // not implemented yet
                if (E_UNMARSHAL(vopretAssets, ss >> dummyEvalCode; ss >> dummyAssetFuncId; ss >> version; ss >> assetid2; ss >> unit_price; ss >> origpubkey) != 0)
                {
                    //fprintf(stderr,"DecodeAssetTokenOpRet() got price %lld\n",(long long)price);
                    assetid2 = revuint256(assetid2);
                    return(assetsFuncId);
                }
                break;
            default:
                break;
            }
        }
    }

    LOGSTREAMFN(ccassets_log, CCLOG_DEBUG1, stream << "no asset's payload or incorrect assets funcId or evalcode" << " funcId=" << (int)funcId << " vopretAssets.size()=" << vopretAssets.size() << " assetsEvalCode=" << assetsEvalCode << " assetsFuncId=" << assetsFuncId << std::endl);
    return (uint8_t)0;
}


vscript_t EncodeAssetOpRetV2(uint8_t assetFuncId, uint256 assetid2, int64_t unit_price, std::vector<uint8_t> origpubkey)
{
    vscript_t vopret; 
	uint8_t evalcode = EVAL_ASSETSV2;
    uint8_t version = 1;

    switch ( assetFuncId )
    {
        //case 't': this cannot be here
		case 'x': case 'o':
			vopret = E_MARSHAL(ss << evalcode << assetFuncId << version);
            break;
        case 's': case 'b': case 'S': case 'B':
            vopret = E_MARSHAL(ss << evalcode << assetFuncId << version << unit_price << origpubkey);
            break;
        case 'E': case 'e':
            assetid2 = revuint256(assetid2);
            vopret = E_MARSHAL(ss << evalcode << assetFuncId << version << assetid2 << unit_price << origpubkey);
            break;
        default:
            CCLogPrintF(ccassets_log, CCLOG_DEBUG1, "%s illegal funcid.%02x\n", __func__, assetFuncId);
            break;
    }
    return(vopret);
}

uint8_t DecodeAssetTokenOpRetV2(const CScript &scriptPubKey, uint8_t &assetsEvalCode, uint256 &tokenid, uint256 &assetid2, int64_t &unit_price, std::vector<uint8_t> &origpubkey)
{
    vscript_t vopretAssets; //, vopretAssetsStripped;
	uint8_t *script, funcId = 0, assetsFuncId = 0, dummyAssetFuncId, dummyEvalCode, version;
	uint256 dummyTokenid;
	std::vector<CPubKey> voutPubkeysDummy;
    std::vector<vscript_t>  oprets;

	tokenid = zeroid;
	assetid2 = zeroid;
	unit_price = 0;
    assetsEvalCode = 0;
    assetsFuncId = 0;

	// First - decode token opret:
	funcId = DecodeTokenOpRetV2(scriptPubKey, tokenid, oprets);
    GetOpReturnCCBlob(oprets, vopretAssets);

    LOGSTREAMFN(ccassets_log, CCLOG_DEBUG2, stream << "from DecodeTokenOpRet returned funcId=" << (int)funcId << std::endl);

	if (funcId == 0 || vopretAssets.size() < 2) {
        LOGSTREAMFN(ccassets_log, CCLOG_DEBUG1, stream << "incorrect opret or no asset's payload" << " funcId=" << (int)funcId << " vopretAssets.size()=" << vopretAssets.size() << std::endl);
		return (uint8_t)0;
	}

    // additional check to prevent crash
    if (vopretAssets.size() >= 2) {

        assetsEvalCode = vopretAssets.begin()[0];
        assetsFuncId = vopretAssets.begin()[1];

        LOGSTREAMFN(ccassets_log, CCLOG_DEBUG2, stream << "assetsEvalCode=" << (int)assetsEvalCode <<  " funcId=" << (char)(funcId ? funcId : ' ') << " assetsFuncId=" << (char)(assetsFuncId ? assetsFuncId : ' ') << std::endl);

        if (assetsEvalCode == EVAL_ASSETSV2)
        {
            //fprintf(stderr,"DecodeAssetTokenOpRet() decode.[%c] assetFuncId.[%c]\n", funcId, assetFuncId);
            switch (assetsFuncId)
            {
            case 'x': case 'o':
                if (vopretAssets.size() == 3)   // format is 'evalcode funcid version' 
                {
                    return(assetsFuncId);
                }
                break;
            case 's': case 'b': case 'S': case 'B':
                if (E_UNMARSHAL(vopretAssets, ss >> dummyEvalCode; ss >> dummyAssetFuncId; ss >> version; ss >> unit_price; ss >> origpubkey) != 0)
                {
                    //fprintf(stderr,"DecodeAssetTokenOpRet() got price %lld\n",(long long)price);
                    return(assetsFuncId);
                }
                break;
            case 'E': case 'e':
                // not implemented yet
                if (E_UNMARSHAL(vopretAssets, ss >> dummyEvalCode; ss >> dummyAssetFuncId; ss >> version; ss >> assetid2; ss >> unit_price; ss >> origpubkey) != 0)
                {
                    //fprintf(stderr,"DecodeAssetTokenOpRet() got price %lld\n",(long long)price);
                    assetid2 = revuint256(assetid2);
                    return(assetsFuncId);
                }
                break;
            default:
                break;
            }
        }
    }

    LOGSTREAMFN(ccassets_log, CCLOG_DEBUG1, stream << "no asset's payload or incorrect assets funcId or evalcode" << " funcId=" << (int)funcId << " vopretAssets.size()=" << vopretAssets.size() << " assetsEvalCode=" << assetsEvalCode << " assetsFuncId=" << assetsFuncId << std::endl);
    return (uint8_t)0;
}

// validate:
// unit_price received for a token >= seller's unit_price
// remaining_nValue calculated correctly
bool ValidateBidRemainder(CAmount unit_price, int64_t remaining_nValue, int64_t orig_nValue, int64_t received_nValue, int64_t paid_units)
{
    int64_t received_unit_price;
    // int64_t new_unit_price = 0;
    if (orig_nValue == 0 || received_nValue == 0 || paid_units == 0)
    {
        CCLogPrintF(ccassets_log, CCLOG_ERROR, "%s error any of these values can't be null: orig_nValue == %lld || received_nValue == %lld || paid_units == %lld\n", __func__, (long long)orig_nValue, (long long)received_nValue, (long long)paid_units);
        return(false);
    }
    /* we do not need this check
    else if (orig_units != (remaining_units + paid_units))
    {
        CCLogPrintF(ccassets_log, CCLOG_ERROR, "%s error orig_remaining_units %lld != %lld (remaining_units %lld + %lld paid_units)\n", __func__, (long long)orig_units, (long long)(remaining_units + paid_units), (long long)remaining_units, (long long)paid_units);
        return(false);
    }*/
    else if (orig_nValue != (remaining_nValue + received_nValue))
    {
        CCLogPrintF(ccassets_log, CCLOG_ERROR, "%s error orig_nValue %lld != %lld (remaining_nValue %lld + %lld received_nValue)\n", __func__, (long long)orig_nValue, (long long)(remaining_nValue - received_nValue), (long long)remaining_nValue, (long long)received_nValue);
        return(false);
    }
    else
    {
        //unit_price = AssetsGetUnitPrice(ordertxid);
        received_unit_price = (received_nValue / paid_units);
        if (received_unit_price > unit_price) // can't satisfy bid by sending tokens of higher unit price than requested
        {
            //fprintf(stderr, "%s error can't satisfy bid with higher unit price: received_unit_price %.8f > unit_price %.8f\n", __func__, (double)received_unit_price / (COIN), (double)unit_price / (COIN));
            CCLogPrintF(ccassets_log, CCLOG_ERROR, "%s error can't satisfy bid with higher unit price: received_unit_price %.8f > unit_price %.8f\n", __func__, (double)received_unit_price / (COIN), (double)unit_price / (COIN));
            return(false);
        }
        CCLogPrintF(ccassets_log, CCLOG_DEBUG1, "%s orig_nValue %lld received_nValue %lld, paid_units %lld, received_unit_price %.8f <= unit_price %.8f\n", __func__, (long long)orig_nValue, (long long)received_nValue, (long long)paid_units, (double)received_unit_price / (COIN), (double)unit_price / (COIN));
    }
    return(true);
}

// unit_price is unit price set by the bidder (maximum)
// received_nValue is coins amount received by token seller from the bidder
// orig_nValue is bid amount
// paid_units is tokens paid to the bidder
// orig_units it the tokens amount the bidder wants to buy
// paid_unit_price is unit_price that token seller sells his tokens for
bool SetBidFillamounts(CAmount unit_price, int64_t &received_nValue, int64_t orig_nValue, int64_t &paid_units, int64_t orig_units, CAmount paid_unit_price)
{
    // int64_t remaining_nValue;

    //if (unit_price <= 0LL)
    //    unit_price = AssetsGetUnitPrice(ordertxid);
    if (orig_units == 0)
    {
        received_nValue = paid_units = 0;
        return(false);
    }
    if (paid_units > orig_units)   // not 
    {
        paid_units = 0;
        // received_nValue = orig_nValue;
        received_nValue = (paid_units * paid_unit_price);  // as paid unit_price might be less than original unit_price
        //  remaining_units = 0;
        fprintf(stderr, "%s not enough units!\n", __func__);
        return(false);
    }
    //remaining_units = (orig_units - paid_units);
    //unitprice = (orig_nValue * COIN) / totalunits;
    //received_nValue = (paidunits * unitprice) / COIN;
    //unit_price = (orig_nValue / orig_remaining_units);
    
    received_nValue = (paid_units * paid_unit_price);
    fprintf(stderr, "%s orig_units.%lld - paid_units.%lld, (orig_value.%lld - received_value.%lld)\n", __func__, 
            (long long)orig_units, (long long)paid_units, (long long)orig_nValue, (long long)received_nValue);
    if (unit_price > 0 && received_nValue > 0 && received_nValue <= orig_nValue)
    {
        CAmount remaining_nValue = (orig_nValue - received_nValue);
        return (ValidateBidRemainder(unit_price, remaining_nValue, orig_nValue, received_nValue, paid_units));
    }
    else 
    {
        fprintf(stderr, "%s incorrect values: unit_price %lld > 0, orig_value.%lld >= received_value.%lld\n", __func__, 
            (long long)unit_price, (long long)orig_nValue, (long long)received_nValue);
        return(false);
    }
}

// unit_price is the mininum token price set by the token seller
// fill_assetoshis is tokens purchased
// orig_assetoshis is available tokens to sell
// paid_nValue is the paid coins calculated as fill_assetoshis * paid_unit_price 
bool SetAskFillamounts(CAmount unit_price, int64_t fill_assetoshis, int64_t orig_assetoshis, int64_t paid_nValue)
{
    //int64_t remaining_assetoshis; 
    //double dunit_price;

    /*if (orig_nValue == 0)
    {
        fill_assetoshis = remaining_nValue = paid_nValue = 0;
        return(false);
    }
    if (paid_nValue >= orig_nValue) // TODO maybe if paid_nValue > orig_nValue just return error but not 
    {
        // paid_nValue = orig_nValue; <- do not reset it
        fill_assetoshis = orig_assetoshis;
        remaining_nValue = 0;
        fprintf(stderr, "%s ask order totally filled!\n", __func__);
        return(true);
    }*/
    if (orig_assetoshis == 0)   {
        fprintf(stderr, "%s ask order empty!\n", __func__);
        return false;
    }
    if (fill_assetoshis == 0)   {
        fprintf(stderr, "%s ask fill tokens is null!\n", __func__);
        return false;
    }
    CAmount paid_unit_price = paid_nValue / fill_assetoshis;
    //remaining_nValue = (orig_nValue - paid_nValue);
    //CAmount unit_price = AssetsGetUnitPrice(ordertxid);
    //remaining_nValue = orig_nValue - unit_price * fill_assetoshis;
    // dunit_price = ((double)orig_nValue / orig_assetoshis);
    // fill_assetoshis = (paid_nValue / dunit_price);  // back conversion -> could be loss of value
    fprintf(stderr, "%s paid_unit_price %lld fill_assetoshis %lld orig_assetoshis %lld unit_price %lld fill_assetoshis %lld\n", __func__, 
            (long long)paid_unit_price, (long long)fill_assetoshis, (long long)orig_assetoshis, (long long)unit_price, (long long)fill_assetoshis);
    if (paid_unit_price > 0 && fill_assetoshis <= orig_assetoshis)
    {
        CAmount remaining_assetoshis = (orig_assetoshis - fill_assetoshis);
        if (remaining_assetoshis == 0)
            fprintf(stderr, "%s ask order totally filled!\n", __func__);
        return ValidateAskRemainder(unit_price, remaining_assetoshis, orig_assetoshis, fill_assetoshis, paid_nValue);
    }
    else 
    {
        fprintf(stderr, "%s incorrect values paid_unit_price %lld > 0, fill_assetoshis %lld > 0 and <= orig_assetoshis %lld\n", __func__, 
            (long long)paid_unit_price, (long long)fill_assetoshis, (long long)orig_assetoshis);
        return(false);
    }
}

// validate: 
// paid unit_price for a token <= the bidder's unit_price
// remaining coins calculated correctly
bool ValidateAskRemainder(CAmount unit_price, int64_t remaining_assetoshis, int64_t orig_assetoshis, int64_t received_assetoshis, int64_t paid_nValue)
{
    int64_t paid_unit_price;
    //CAmount unit_price = AssetsGetUnitPrice(ordertxid);
    //int64_t new_unit_price = 0;

    if (orig_assetoshis == 0 || received_assetoshis == 0 || paid_nValue == 0)
    {
        CCLogPrintF(ccassets_log, CCLOG_ERROR, "%s error any of these values can't be null: orig_assetoshis == %lld || received_assetoshis == %lld || paid_nValue == %lld\n", __func__, (long long)orig_assetoshis, (long long)received_assetoshis, (long long)paid_nValue);
        return(false);
    }
    // this check does not affect anything
    //else if (orig_nValue != (remaining_nValue + paid_nValue))
    /*else if (orig_nValue != (remaining_nValue + received_assetoshis * unit_price))  // remaining_nValue is calced from the source unit_price, not paid unit_price
    {
        CCLogPrintF(ccassets_log, CCLOG_ERROR,  "%s error orig_nValue %lld != %lld + (received_assetoshis %lld * unit_price %lld)\n", __func__, (long long)orig_nValue, (long long)remaining_nValue, (long long)received_assetoshis, (long long)unit_price);
        return(false);
    }*/
    else if (orig_assetoshis != (remaining_assetoshis + received_assetoshis))
    {
        CCLogPrintF(ccassets_log, CCLOG_ERROR,  "%s error orig_assetoshis %lld != %lld (remaining_nValue %lld + received_nValue %lld)\n", __func__, (long long)orig_assetoshis, (long long)(remaining_assetoshis - received_assetoshis), (long long)remaining_assetoshis, (long long)received_assetoshis);
        return(false);
    }
    else
    {
        // unit_price = (orig_nValue / orig_assetoshis);
        paid_unit_price = (paid_nValue / received_assetoshis);
        //if (remaining_nValue != 0)
        //    new_unit_price = (remaining_nValue / remaining_assetoshis);  // for debug printing
        if (paid_unit_price < unit_price)  // can't pay for ask with lower unit price than requested
        {
            //fprintf(stderr, "%s error can't pay for ask with lower price: paid_unit_price %.8f < %.8f unit_price\n", __func__, (double)paid_unit_price / COIN, (double)unit_price / COIN);
            CCLogPrintF(ccassets_log, CCLOG_ERROR, "%s error can't pay for ask with lower price: paid_unit_price %.8f < unit_price %.8f\n", __func__, (double)paid_unit_price / COIN, (double)unit_price / COIN);

            return(false);
        }
        CCLogPrintF(ccassets_log, CCLOG_DEBUG1, "%s got paid_unit_price %.8f >= unit_price %.8f\n", __func__, (double)paid_unit_price / COIN, (double)unit_price / COIN);
    }
    return(true);
}

// not implemented
bool SetSwapFillamounts(CAmount unit_price, int64_t &received_assetoshis, int64_t orig_assetoshis, int64_t &paid_assetoshis2, int64_t total_assetoshis2)
{
	int64_t remaining_assetoshis; double dunitprice;
	if ( total_assetoshis2 == 0 )
	{
		fprintf(stderr,"%s total_assetoshis2.0 orig_assetoshis.%lld paid_assetoshis2.%lld\n", __func__, (long long)orig_assetoshis,(long long)paid_assetoshis2);
		received_assetoshis = paid_assetoshis2 = 0;
		return(false);
	}
	if ( paid_assetoshis2 >= total_assetoshis2 )
	{
		paid_assetoshis2 = total_assetoshis2;
		received_assetoshis = orig_assetoshis;
		// remaining_assetoshis2 = 0;
		fprintf(stderr,"%s swap order totally filled!\n", __func__);
		return(true);
	}
	// remaining_assetoshis2 = (total_assetoshis2 - paid_assetoshis2);
	dunitprice = ((double)total_assetoshis2 / orig_assetoshis);
	received_assetoshis = (paid_assetoshis2 / dunitprice);
	fprintf(stderr,"%s (%lld - %lld)\n", __func__, (long long)total_assetoshis2/COIN, (long long)paid_assetoshis2/COIN);
	fprintf(stderr,"%s unitprice %.8f received_assetoshis %lld orig %lld\n", __func__,dunitprice/COIN,(long long)received_assetoshis,(long long)orig_assetoshis);
	if ( fabs(dunitprice) > SMALLVAL && received_assetoshis > 0 && received_assetoshis <= orig_assetoshis )
	{
		remaining_assetoshis = (orig_assetoshis - received_assetoshis);
		return ValidateAskRemainder(unit_price, remaining_assetoshis,orig_assetoshis,received_assetoshis,paid_assetoshis2);
	} 
    else 
        return(false);
}

bool ValidateSwapRemainder(int64_t remaining_price, int64_t remaining_nValue, int64_t orig_nValue, int64_t received_nValue, int64_t paidunits, int64_t totalunits)
{
    int64_t unitprice, recvunitprice, newunitprice = 0;
    if (orig_nValue == 0 || received_nValue == 0 || paidunits == 0 || totalunits == 0)
    {
        CCLogPrintF(ccassets_log, CCLOG_ERROR, "%s orig_nValue == %lld || received_nValue == %lld || paidunits == %lld || totalunits == %lld\n", __func__, (long long)orig_nValue, (long long)received_nValue, (long long)paidunits, (long long)totalunits);
        return(false);
    }
    else if (totalunits != (remaining_price + paidunits))
    {
        CCLogPrintF(ccassets_log, CCLOG_ERROR, "%s totalunits %lld != %lld (remaining_price %lld + %lld paidunits)\n", __func__, (long long)totalunits, (long long)(remaining_price + paidunits), (long long)remaining_price, (long long)paidunits);
        return(false);
    }
    else if (orig_nValue != (remaining_nValue + received_nValue))
    {
        CCLogPrintF(ccassets_log, CCLOG_ERROR, "%s orig_nValue %lld != %lld (remaining_nValue %lld + %lld received_nValue)\n", __func__, (long long)orig_nValue, (long long)(remaining_nValue - received_nValue), (long long)remaining_nValue, (long long)received_nValue);
        return(false);
    }
    else
    {
        unitprice = (orig_nValue * COIN) / totalunits;
        recvunitprice = (received_nValue * COIN) / paidunits;
        if (remaining_price != 0)
            newunitprice = (remaining_nValue * COIN) / remaining_price;
        if (recvunitprice < unitprice)
        {
            CCLogPrintF(ccassets_log, CCLOG_ERROR, "%s error recvunitprice %.8f < %.8f unitprice, new unitprice %.8f\n", __func__, (double)recvunitprice / (COIN*COIN), (double)unitprice / (COIN*COIN), (double)newunitprice / (COIN*COIN));
            return(false);
        }
        fprintf(stderr, "%s recvunitprice %.8f >= %.8f unitprice, new unitprice %.8f\n", __func__, (double)recvunitprice / (COIN*COIN), (double)unitprice / (COIN*COIN), (double)newunitprice / (COIN*COIN));
    }
    return(true);
}

// get tx's vin inputs for cp->evalcode and addr. If addr is null then all inputs are added
CAmount AssetsGetCCInputs(Eval *eval, struct CCcontract_info *cp, const char *addr, const CTransaction &tx)
{
	CTransaction vinTx; 
    uint256 hashBlock; 
	CAmount inputs = 0LL;

	//struct CCcontract_info *cpTokens, C;
	//cpTokens = CCinit(&C, EVAL_TOKENS);

	for (int32_t i = 0; i < tx.vin.size(); i++)
	{												    
		if (cp->ismyvin(tx.vin[i].scriptSig))
		{
			if (eval->GetTxUnconfirmed(tx.vin[i].prevout.hash, vinTx, hashBlock))
			{
                char scriptaddr[KOMODO_ADDRESS_BUFSIZE];
                if (addr == NULL || Getscriptaddress(scriptaddr, vinTx.vout[tx.vin[i].prevout.n].scriptPubKey) && strcmp(scriptaddr, addr) == 0)  {
                    //std::cerr << __func__ << " adding amount=" << vinTx.vout[tx.vin[i].prevout.n].nValue << " for vin i=" << i << " eval=" << std::hex << (int)cp->evalcode << std::resetiosflags(std::ios::hex) << std::endl;
                    inputs += vinTx.vout[tx.vin[i].prevout.n].nValue;
                }
			}
		}
	}
	return inputs;
}