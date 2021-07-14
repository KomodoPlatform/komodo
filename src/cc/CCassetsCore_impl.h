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

#ifndef CC_ASSETS_CORE_IMPL_H
#define CC_ASSETS_CORE_IMPL_H

#include "CCassets.h"
#include "CCtokens.h"


/*
 The SetAssetFillamounts() and ValidateAssetRemainder() work in tandem to calculate the vouts for a fill and to validate the vouts, respectively.
 
 This pair of functions are critical to make sure the trading is correct and is the trickiest part of the assets contract.
 
 //vin.0: normal input
 //vin.1: unspendable.(vout.0 from buyoffer) buyTx.vout[0]
 //vin.2+: valid CC output satisfies buyoffer (*tx.vin[2])->nValue
 //vout.0: remaining amount of bid to unspendable
 //vout.1: vin.1 value to signer of vin.2
 //vout.2: vin.2 assetoshis to original pubkey
 //vout.3: CC output for assetoshis change (if any)
 //vout.4: normal output for change (if any)
 //vout.n-1: opreturn [EVAL_ASSETS] ['B'] [assetid] [remaining asset required] [origpubkey]
    ValidateAssetRemainder(remaining_price,tx.vout[0].nValue,nValue,tx.vout[1].nValue,tx.vout[2].nValue,totalunits);
 
 Yes, this is quite confusing...
 
 In ValidateAssetRemainder the naming convention is nValue is the coin/asset with the offer on the books and "units" is what it is being paid in. The high level check is to make sure we didnt lose any coins or assets, the harder to validate is the actual price paid as the "orderbook" is in terms of the combined nValue for the combined totalunits.
 
 We assume that the effective unit cost in the orderbook is valid and that that amount was paid and also that any remainder will be close enough in effective unit cost to not matter. At the edge cases, this will probably be not true and maybe some orders wont be practically fillable when reduced to fractional state. However, the original pubkey that created the offer can always reclaim it.
*/

// validates opret for asset tx:
template<class A>
bool ValidateAssetOpret(CTransaction tx, int32_t v, uint256 assetid, int64_t &remaining_units_out, std::vector<uint8_t> &origpubkey_out) 
{
	uint256 assetidOpret, assetidOpret2;
	uint8_t funcid, evalCode;

	// this is just for log messages indentation fur debugging recursive calls:
	int32_t n = tx.vout.size();

	if ((funcid = A::DecodeAssetTokenOpRet(tx.vout.back().scriptPubKey, evalCode, assetidOpret, assetidOpret2, remaining_units_out, origpubkey_out)) == 0)
	{
        LOGSTREAMFN(ccassets_log, CCLOG_DEBUG1, stream << "called DecodeAssetTokenOpRet returned funcId=0 for opret from txid=" << tx.GetHash().GetHex() << std::endl);
		return(false);
	}
	else if (funcid != 'E')
	{
		if (assetid != zeroid && assetidOpret == assetid)
		{
			//std::cerr  << "ValidateAssetOpret() returns true for not 'E', funcid=" << (char)funcid << std::endl;
			return(true);
		}
	}
	else if (funcid == 'E')  // NOTE: not implemented yet!
	{
		if (v < 2 && assetid != zeroid && assetidOpret == assetid)
			return(true);
		else if (v == 2 && assetid != zeroid && assetidOpret2 == assetid)
			return(true);
	}
	return false;
}  

// Checks if the vout is a really Asset CC vout
template<class A>
int64_t IsAssetvout(struct CCcontract_info *cp, int64_t &remaining_units_out, std::vector<uint8_t> &origpubkey_out, const CTransaction& tx, int32_t v, uint256 refassetid)
{
    // just check boundaries:
    int32_t n = tx.vout.size();
    if (v >= n - 1) {  
        LOGSTREAMFN(ccassets_log, CCLOG_ERROR, stream << "internal err: (v >= n - 1), returning 0" << std::endl);
        return -1;
    }

	if (tx.vout[v].scriptPubKey.IsPayToCryptoCondition())
	{
		if (!ValidateAssetOpret<A>(tx, v, refassetid, remaining_units_out, origpubkey_out)) {
            LOGSTREAMFN(ccassets_log, CCLOG_ERROR, stream << "invalid assets opreturn" << std::endl);
            return -1;
        }
        return tx.vout[v].nValue;
	}
	return(0);
} 

// extract sell/buy owner's pubkey from the opret
template<class A>
uint8_t GetOrderParams(std::vector<uint8_t> &origpubkey_out, CAmount &unit_price, uint256 &assetid, const CTransaction &tx)
{
    uint256 assetid2;
    uint8_t evalCode, funcid;

    if (tx.vout.size() > 0 && (funcid = A::DecodeAssetTokenOpRet(tx.vout.back().scriptPubKey, evalCode, assetid, assetid2, unit_price, origpubkey_out)) != 0)
        return funcid;
    else
        return 0;
}

// Calculate seller/buyer's dest cc address from ask/bid tx funcid
template<class A>
bool GetAssetorigaddrs(struct CCcontract_info *cp, char *origCCaddr, char *origNormalAddr, const CTransaction& vintx)
{
    uint256 assetid, assetid2; 
    int64_t price,nValue=0; 
    int32_t n; 
    uint8_t vintxFuncId; 
	std::vector<uint8_t> origpubkey; 
	CScript script;
	uint8_t evalCode;

    n = vintx.vout.size();
    if( n == 0 || (vintxFuncId = A::DecodeAssetTokenOpRet(vintx.vout[n-1].scriptPubKey, evalCode, assetid, assetid2, price, origpubkey)) == 0 ) {
        LOGSTREAMFN(ccassets_log, CCLOG_INFO, stream << "could not get vintx opreturn" << std::endl);
        return(false);
    }

    struct CCcontract_info *cpTokens, tokensC;
    cpTokens = CCinit(&tokensC, A::TokensEvalCode());

	if (vintxFuncId == 's' || vintxFuncId == 'S' || vintxFuncId == 'b' || vintxFuncId == 'B') {
        cpTokens->evalcodeAdd = cp->evalcodeAdd;  // add non-fungible evalcode if present
        if (!GetTokensCCaddress(cpTokens, origCCaddr, pubkey2pk(origpubkey), A::IsMixed()))  // tokens to single-eval token or token+nonfungible
            return false;
	}
	else  {
		LOGSTREAMFN(ccassets_log, CCLOG_INFO, stream << "incorrect vintx funcid=" << (char)(vintxFuncId?vintxFuncId:' ') << std::endl);
		return false;
	}
    if (Getscriptaddress(origNormalAddr, CScript() << origpubkey << OP_CHECKSIG))
        return(true);
    else 
		return(false);
}

template<class A>
CAmount AssetValidateCCvin(struct CCcontract_info *cpAssets, Eval* eval, char *origCCaddr_out, char *origaddr_out, const CTransaction &tx, int32_t vini, CTransaction &vinTx)
{
	uint256 hashBlock;
    uint256 assetid, assetid2;
    uint256 vinAssetId, vinAssetId2;
	CAmount tmpprice, vinPrice;
    std::vector<uint8_t> tmporigpubkey;
    std::vector<uint8_t> vinOrigpubkey;
    uint8_t evalCode;
    uint8_t vinEvalCode;
	char destaddr[KOMODO_ADDRESS_BUFSIZE], unspendableAddr[KOMODO_ADDRESS_BUFSIZE];

    origaddr_out[0] = destaddr[0] = origCCaddr_out[0] = 0;

    uint8_t funcid = 0;
    uint8_t vinFuncId = 0;
	if (tx.vout.size() > 0) 
		funcid = A::DecodeAssetTokenOpRet(tx.vout.back().scriptPubKey, evalCode, assetid, assetid2, tmpprice, tmporigpubkey);
    else
        return eval->Invalid("no vouts in tx"), 0LL;

    if (tx.vin.size() < 2)
        return eval->Invalid("not enough vins"), 0LL;
    else if(tx.vin[vini].prevout.n != ASSETS_GLOBALADDR_VOUT)   // check global addr vout number == 0
        return eval->Invalid("asset cc vin must refer bid vintx.vout[0]"), 0LL;
    else if(eval->GetTxUnconfirmed(tx.vin[vini].prevout.hash, vinTx, hashBlock) == false)
    {
		LOGSTREAMFN(ccassets_log, CCLOG_ERROR, stream << "cannot load vintx for vin=" << vini << " vintx id=" << tx.vin[vini].prevout.hash.GetHex() << std::endl);
        return eval->Invalid("could not load previous tx or it has too few vouts"), 0LL;
    }
    else if (vinTx.vout.size() < 1 || (vinFuncId = A::DecodeAssetTokenOpRet(vinTx.vout.back().scriptPubKey, vinEvalCode, vinAssetId, vinAssetId2, vinPrice, vinOrigpubkey)) == 0) {
        return eval->Invalid("could not find assets opreturn in previous tx"), 0LL;
    }
	// if fillSell or cancelSell --> should spend tokens from dual-eval token-assets global addr:
    else if((funcid == 'S' || funcid == 'x') && 
       (tx.vin[vini].prevout.n >= vinTx.vout.size() ||  // check bounds
		Getscriptaddress(destaddr, vinTx.vout[tx.vin[vini].prevout.n].scriptPubKey) == false || 
		!GetTokensCCaddress(cpAssets, unspendableAddr, GetUnspendable(cpAssets, NULL), A::IsMixed()) ||                  
		strcmp(destaddr, unspendableAddr) != 0))
    {
        CCLogPrintF(ccassets_log, CCLOG_ERROR, "%s cc addr %s is not dual token-evalcode=0x%02x asset unspendable addr %s\n", __func__, destaddr, (int)cpAssets->evalcode, unspendableAddr);
        return eval->Invalid("invalid vin assets CCaddr"), 0LL;
    }
	// if fillBuy or cancelBuy --> should spend coins from asset cc global addr
	else if ((funcid == 'B' || funcid == 'o') && 
	   (tx.vin[vini].prevout.n >= vinTx.vout.size() ||  // check bounds
        Getscriptaddress(destaddr, vinTx.vout[tx.vin[vini].prevout.n].scriptPubKey) == false ||
		!GetCCaddress(cpAssets, unspendableAddr, GetUnspendable(cpAssets, NULL), A::IsMixed()) ||
		strcmp(destaddr, unspendableAddr) != 0))
	{
        CCLogPrintF(ccassets_log, CCLOG_ERROR, "%s cc addr %s is not evalcode=0x%02x asset unspendable addr %s\n", __func__, destaddr, (int)cpAssets->evalcode, unspendableAddr);
		return eval->Invalid("invalid vin assets CCaddr"), 0LL;
	}
    // end of check source unspendable cc address
    //else if ( vinTx.vout[0].nValue < 10000 )
    //    return eval->Invalid("invalid dust for buyvin");
    // get user dest cc and normal addresses:
    else if(GetAssetorigaddrs<A>(cpAssets, origCCaddr_out, origaddr_out, vinTx) == false)  
        return eval->Invalid("couldnt get origaddr for vin tx"), 0LL;

    //fprintf(stderr,"AssetValidateCCvin() got %.8f to origaddr.(%s)\n", (double)vinTx.vout[tx.vin[vini].prevout.n].nValue/COIN,origaddr);

    // check that vinTx B or S has assets cc vins:
    if (vinFuncId == 'B' || vinFuncId == 'S')
    {
        if (std::find_if(vinTx.vin.begin(), vinTx.vin.end(), [&](const CTxIn &vin) { return cpAssets->ismyvin(vin.scriptSig); }) == vinTx.vin.end())
            return eval->Invalid("no assets cc vins in previous fillbuy or fillsell tx"), 0LL;
    }

    // check no more other vins spending from global addr:
    if (AssetsGetCCInputs(eval, cpAssets, unspendableAddr, tx) != vinTx.vout[ASSETS_GLOBALADDR_VOUT].nValue)
        return eval->Invalid("invalid assets cc vins found"), 0LL;

    if (vinTx.vout[ASSETS_GLOBALADDR_VOUT].nValue == 0)
        return eval->Invalid("null value in previous tx CC vout0"), 0LL;

    return vinTx.vout[ASSETS_GLOBALADDR_VOUT].nValue;
}

template<class A>
CAmount AssetValidateBuyvin(struct CCcontract_info *cpAssets, Eval* eval, CAmount &unit_price, std::vector<uint8_t> &origpubkey_out, char *origCCaddr_out, char *origaddr_out, const CTransaction &tx, uint256 refassetid)
{
    CTransaction vinTx; CAmount nValue; uint256 assetid, assetid2; uint8_t funcid, evalCode;

    origCCaddr_out[0] = origaddr_out[0] = 0;

    // get first ccvin:
    int32_t ivincc;
    for (ivincc = 0; ivincc < tx.vin.size(); ivincc ++)
        if (cpAssets->ismyvin(tx.vin[ivincc].scriptSig))
            break;
    if (ivincc == tx.vin.size())
        return eval->Invalid("cc vin not found in tx"), 0LL;

    // validate locked coins on Assets vin[1]
    if ((nValue = AssetValidateCCvin<A>(cpAssets, eval, origCCaddr_out, origaddr_out, tx, /*ASSETS_GLOBALADDR_VIN*/ivincc, vinTx)) == 0)
        return 0LL; // eval is set already in AssetValidateCCvin
    else if (vinTx.vout.size() < 2)
        return eval->Invalid("invalid previous tx, too few vouts"), 0LL;
    else if (vinTx.vout[0].scriptPubKey.IsPayToCryptoCondition() == false)
        return eval->Invalid("invalid not cc vout0 for buyvin"), 0LL;
    else if ((funcid = A::DecodeAssetTokenOpRet(vinTx.vout[vinTx.vout.size() - 1].scriptPubKey, evalCode, assetid, assetid2, unit_price, origpubkey_out)) == 'b' &&
        vinTx.vout[1].scriptPubKey.IsPayToCryptoCondition() == false)  // marker is only in 'b'?
        return eval->Invalid("invalid not cc vout1 for buyvin"), 0LL;
    else
    {
        if (funcid != 'b' && funcid != 'B')
            return eval->Invalid("invalid opreturn funcid for buyvin"), 0LL;
        else if (refassetid != assetid)
            return eval->Invalid("invalid assetid for buyvin"), 0LL;
    }
    return nValue;
}

template<class A>
CAmount AssetValidateSellvin(struct CCcontract_info *cpAssets, Eval* eval, CAmount &unit_price, std::vector<uint8_t> &origpubkey_out, char *origCCaddr_out, char *origaddr_out, const CTransaction &tx, uint256 assetid)
{
    CTransaction vinTx; CAmount nValue, assetoshis;

    // get first ccvin:
    int32_t ivincc;
    for (ivincc = 0; ivincc < tx.vin.size(); ivincc ++)
        if (cpAssets->ismyvin(tx.vin[ivincc].scriptSig))
            break;
    if (ivincc == tx.vin.size())
        return eval->Invalid("cc vin not found"), 0LL;
    if ((nValue = AssetValidateCCvin<A>(cpAssets, eval, origCCaddr_out, origaddr_out, tx, /*ASSETS_GLOBALADDR_VIN*/ivincc, vinTx)) == 0)
        return 0LL; // eval is set already in AssetValidateCCvin
    
    if ((assetoshis = IsAssetvout<A>(cpAssets, unit_price, origpubkey_out, vinTx, ASSETS_GLOBALADDR_VOUT, assetid)) == 0)
        return eval->Invalid("invalid missing CC vout0 for sellvin"), 0LL;
    else
        return assetoshis;
}



#endif // #ifndef CC_ASSETS_CORE_IMPL_H
