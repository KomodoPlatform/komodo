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

#include "CCtokens.h"
#include "CCassets.h"
#include "CCtokens_impl.h"
#include "CCassetsCore_impl.h"
#include "CCTokelData.h"

const int32_t CCVOUT = 1;
const int32_t NORMALVOUT = 0;


/*
 TODO: update:

 Assets can be created or transferred.
 
 native coins are also locked in the EVAL_ASSETS address, so we need a strict rule on when utxo in the special address are native coins and when they are assets. The specific rule that must not be violated is that vout0 for 'b'/'B' funcid are native coins. All other utxo locked in the special address are assets.
 
 To create an asset use CC EVAL_ASSETS to create a transaction where vout[0] funds the assets. Externally each satoshi can be interpreted to represent 1 asset, or 100 million satoshis for one asset with 8 decimals, and the other decimals in between. The interpretation of the number of decimals is left to the higher level usages.
 
 Once created, the assetid is the txid of the create transaction and using the assetid/0 it can spend the assets to however many outputs it creates. The restriction is that the last output must be an opreturn with the assetid. The sum of all but the first output needs to add up to the total assetoshis input. The first output is ignored and used for change from txfee.
 
 What this means is that vout 0 of the creation txid and vouts 1 ... n-2 for transfer vouts are assetoshi outputs.
 
 There is a special type of transfer to an unspendable address, that locks the asset and creates an offer for all. The details specified in the opreturn. In order to be compatible with the signing restrictions, the "unspendable" address is actually an address whose privkey is known to all. Funds sent to this address can only be spent if the swap parameters are fulfilled, or if the original pubkey cancels the offer by spending it.
 
 Types of transactions:
 create name:description -> txid for assetid
 transfer <pubkey> <assetid> -> [{address:amount}, ... ] // like withdraw api
 selloffer <pubkey> <txid/vout> <amount> -> cancel or fillsell -> mempool txid or rejected, might not confirm
 buyoffer <amount> <assetid> <required> -> cancelbuy or fillbuy -> mempool txid or rejected, might not confirm
 exchange <pubkey> <txid/vout> <required> <required assetid> -> cancel or fillexchange -> mempool txid or rejected, might not confirm
 
 assetsaddress <pubkey> // all assets end up in a special address for each pubkey
 assetbalance <pubkey> <assetid>
 assetutxos <pubkey> <assetid>
 assetsbalances <pubkey>
 asks <assetid>
 bids <assetid>
 swaps <assetid>
 
 valid CC output: create or transfer or buyoffer or selloffer or exchange or cancel or fill
 
 
 buyoffer:
 vins.*: normal inputs (bid + change)
 vout.0: amount of bid to unspendable
 vout.1: CC output for marker
 vout.2: normal output for change (if any)
 vout.n-1: opreturn [EVAL_ASSETS] ['b'] [assetid] [amount of asset required] [origpubkey]

 cancelbuy:
 vin.0: normal input
 vin.1: unspendable.(vout.0 from buyoffer) buyTx.vout[0]
 vin.2: CC marker from buyoffer for txfee
 vout.0: vin.1 value to original pubkey buyTx.vout[0].nValue -> [origpubkey]
 vout.1: vin.2 back to users pubkey
 vout.2: normal output for change (if any)
 vout.n-1: opreturn [EVAL_ASSETS] ['o'] [assetid] 0 0 [origpubkey]
 
 fillbuy:
 vin.0: normal input
 vin.1: unspendable.(vout.0 from buyoffer) buyTx.vout[0]
 vin.2+: valid CC output satisfies buyoffer (*tx.vin[2])->nValue
 vout.0: remaining amount of bid to unspendable
 vout.1: vin.1 value to signer of vin.2
 vout.2: vin.2 assetoshis to original pubkey
 vout.3: CC output for assetoshis change (if any)
 vout.4: normal output for change (if any)
 vout.n-1: opreturn [EVAL_ASSETS] ['B'] [assetid] [remaining asset required] [origpubkey]

 selloffer:
 vin.0: normal input
 vin.1+: valid CC output for sale
 vout.0: vin.1 assetoshis output to CC to unspendable
 vout.1: CC output for marker
 vout.2: CC output for change (if any)
 vout.3: normal output for change (if any)
 vout.n-1: opreturn [EVAL_ASSETS] ['s'] [assetid] [amount of native coin required] [origpubkey]
 
 exchange:
 vin.0: normal input
 vin.1+: valid CC output
 vout.0: vin.1 assetoshis output to CC to unspendable
 vout.1: CC output for change (if any)
 vout.2: normal output for change (if any)
 vout.n-1: opreturn [EVAL_ASSETS] ['e'] [assetid] [assetid2] [amount of asset2 required] [origpubkey]
 
 cancel:
 vin.0: normal input
 vin.1: unspendable.(vout.0 from exchange or selloffer) sellTx/exchangeTx.vout[0] inputTx
 vin.2: CC marker from selloffer for txfee
 vout.0: vin.1 assetoshis to original pubkey CC sellTx/exchangeTx.vout[0].nValue -> [origpubkey]
 vout.1: vin.2 back to users pubkey
 vout.2: normal output for change (if any)
 vout.n-1: opreturn [EVAL_ASSETS] ['x'] [assetid]
 
 fillsell:
 vin.0: normal input
 vin.1: unspendable.(vout.0 assetoshis from selloffer) sellTx.vout[0]
 vin.2+: normal output that satisfies selloffer (*tx.vin[2])->nValue
 vout.0: remaining assetoshis -> unspendable
 vout.1: vin.1 assetoshis to signer of vin.2 sellTx.vout[0].nValue -> any
 vout.2: vin.2 value to original pubkey [origpubkey]
 vout.3: CC asset for change (if any)
 vout.4: CC asset2 for change (if any) 'E' only
 vout.5: normal output for change (if any)
 vout.n-1: opreturn [EVAL_ASSETS] ['S'] [assetid] [amount of coin still required] [origpubkey]
 
 fillexchange:
 vin.0: normal input
 vin.1: unspendable.(vout.0 assetoshis from exchange) exchangeTx.vout[0]
 vin.2+: valid CC assetid2 output that satisfies exchange (*tx.vin[2])->nValue
 vout.0: remaining assetoshis -> unspendable
 vout.1: vin.1 assetoshis to signer of vin.2 exchangeTx.vout[0].nValue -> any
 vout.2: vin.2 assetoshis2 to original pubkey [origpubkey]
 vout.3: normal output for change (if any)
 vout.n-1: opreturn [EVAL_ASSETS] ['E'] [assetid vin0+1] [assetid vin2] [remaining asset2 required] [origpubkey]
*/

// checks cc vins (only mine) and cc vouts (all) count. If param is -1 then it does not check it
static bool CountCCVinVouts(struct CCcontract_info *cpAssets, const CTransaction & tx, int32_t _ccvins, int32_t _ccvouts)
{
    int32_t ccvins = 0;
    int32_t ccvouts = 0;

    if (_ccvins >= 0)   {
        for (auto const &vin : tx.vin)
            if (cpAssets->ismyvin(vin.scriptSig))
                ccvins ++;
    }
    if (_ccvouts >= 0)  {
        for (auto const &vout : tx.vout)
            if (vout.scriptPubKey.IsPayToCryptoCondition())
                ccvouts ++;
    }

    if (_ccvins >= 0 && _ccvins != ccvins || _ccvouts >= 0 && _ccvouts != ccvouts) {
        LOGSTREAMFN(ccassets_log, CCLOG_ERROR, stream << " invalid vins or vout count: ccvins=" << ccvins << " check=" << _ccvins << " ccvouts=" << ccvouts << " check=" << _ccvouts << std::endl);
        return false;
    }
    return true;
}


// tx validation
template<class T, class A>
static bool AssetsValidateInternal(struct CCcontract_info *cp, Eval* eval,const CTransaction &tx, uint32_t nIn)
{
    static uint256 zero;
    CTxDestination address; 
    CTransaction vinTx, createTx; 
    uint256 hashBlock, assetid, assetid2; 
    int32_t ccvins = -1, ccvouts = -1;
	int64_t unit_price, vin_unit_price; 
    vuint8_t vorigpubkey, vin_origpubkey, vextraData;
    TokenDataTuple tokenData;
	uint8_t funcid, evalCodeInOpret; 
	char destaddr[KOMODO_ADDRESS_BUFSIZE], origNormalAddr[KOMODO_ADDRESS_BUFSIZE], ownerNormalAddr[KOMODO_ADDRESS_BUFSIZE]; 
    char origTokensCCaddr[KOMODO_ADDRESS_BUFSIZE], origCCaddrDummy[KOMODO_ADDRESS_BUFSIZE]; 
    char tokensDualEvalUnspendableCCaddr[KOMODO_ADDRESS_BUFSIZE], origAssetsCCaddr[KOMODO_ADDRESS_BUFSIZE], globalAssetsCCaddr[KOMODO_ADDRESS_BUFSIZE];

    CAmount vin_tokens = 0;
    CAmount vin_coins = 0;

	if (tx.vin.size() == 0)
		return eval->Invalid("AssetValidate: no vins"); 
	if (tx.vout.size() == 0)
		return eval->Invalid("AssetValidate: no vouts");

    if((funcid = A::DecodeAssetTokenOpRet(tx.vout.back().scriptPubKey, evalCodeInOpret, assetid, assetid2, unit_price, vorigpubkey)) == 0 )
        return eval->Invalid("AssetValidate: invalid opreturn payload");

	// reinit cpAssets as we could set evalcodeAdd in it
	struct CCcontract_info *cpAssets, assetsC;
	cpAssets = CCinit(&assetsC, A::EvalCode());

	// we need this for validating single-eval tokens' vins/vous:
	struct CCcontract_info *cpTokens, tokensC;
	cpTokens = CCinit(&tokensC, T::EvalCode());

    // non-fungible tokens support:
    GetTokenData<T>(eval, assetid, tokenData, vextraData);
    int64_t royaltyFract = 0;  // royalty is N in N/1000 fraction
    if (vextraData.size() > 0)   {
        cpAssets->evalcodeAdd = vextraData.begin()[0];
        GetTokelDataAsInt64(vextraData, TKLPROP_ROYALTY, royaltyFract);
        if (royaltyFract > TKLROYALTY_DIVISOR-1)
            royaltyFract = TKLROYALTY_DIVISOR-1; // royalty upper limit
    }

    vuint8_t ownerpubkey = std::get<0>(tokenData);
    Getscriptaddress(ownerNormalAddr, CScript() << ownerpubkey << OP_CHECKSIG);

	// find dual-eval tokens global addr where tokens are locked:
	GetTokensCCaddress(cpAssets, tokensDualEvalUnspendableCCaddr, GetUnspendable(cpAssets, NULL), A::IsMixed());

    // originator cc address, this is for marker validation:
    GetCCaddress(cpAssets, origAssetsCCaddr, vorigpubkey, A::IsMixed()); 

    // global cc address:
    GetCCaddress(cpAssets, globalAssetsCCaddr, GetUnspendable(cpAssets, NULL), A::IsMixed()); 

    if( IsCCInput(tx.vin[0].scriptSig) != false )   // vin0 should be normal vin
        return eval->Invalid("illegal asset cc vin0");
    else if(tx.vout.size() < 2)
        return eval->Invalid("too few vouts");  // it was if(numvouts < 1) but it refers at least to vout[1] below
    
    if (assetid == zeroid)
        return eval->Invalid("illegal opreturn assetid");
    
    switch( funcid )
    {
        case 'c': 
            // see token cc
			return eval->Invalid("invalid asset funcid \'c\'");
            
        case 't': 
            // see token cc
			return eval->Invalid("invalid asset funcid \'t\'");
                        
        case 'b': // bid offer
            //vins.*: normal inputs (bid + change)
            //vout.0: amount of bid to cc assets global address
            //vout.1: CC output for marker
            //vout.2: normal output for change (if any)
            //vout.n-1: opreturn [EVAL_ASSETS] ['b'] [assetid] [amount of asset required] [origpubkey]

            // as we don't use tokenconvert 'b' does not have cc inputs and we should not be here:
            if (cpAssets->evalcode == EVAL_ASSETS)
                return eval->Invalid("invalid asset funcid (b)");
            else {
                // for EVAL_ASSETSV2 check the creation tx 
                char origTokenAddr[KOMODO_ADDRESS_BUFSIZE];
                
                //preventCCvouts = 2;
                if (tx.vout.size() < 3)
                    return eval->Invalid("too few vouts");
                else if(A::ConstrainVout(tx.vout[0], CCVOUT, globalAssetsCCaddr, 0LL, A::EvalCode()) == false)
                    return eval->Invalid("invalid funding for bid");
                else if(A::ConstrainVout(tx.vout[1], CCVOUT, origAssetsCCaddr, ASSETS_MARKER_AMOUNT, A::EvalCode()) == false)       // marker to originator asset cc addr
                    return eval->Invalid("invalid vout1 marker for original pubkey");
                else if(TotalPubkeyNormalInputs(eval, tx, pubkey2pk(vorigpubkey)) == 0) // check tx is signed by originator pubkey
                    return eval->Invalid("not the originator pubkey signed for bid");
                else if (unit_price <= ASSETS_NORMAL_DUST)
                    return eval->Invalid("invalid or too low unit price");
                else if (tx.vout[0].nValue < unit_price)
                    return eval->Invalid("invalid bid amount too low");

                // check it should not be assets cc vins for tokenv2bid at all:
                for (auto const &vin : tx.vin)   {
                    if (cpAssets->ismyvin(vin.scriptSig))
                        return eval->Invalid("could not have cc assets vin for creation tx");
                }
                ccvouts = 2;
            }            
            break;

        case 'o': // cancelbid
            //vin.0: normal input
            //vin.1: unspendable.(vout.0 from buyoffer) buyTx.vout[0]
            //vin.2: CC marker from buyoffer for txfee
            //vout.0: vin.1 value to original pubkey buyTx.vout[0].nValue -> [origpubkey]
            //vout.1: vin.2 back to users pubkey
            //vout.2: normal output for change (if any)
            //vout.n-1: opreturn [EVAL_ASSETS] ['o']
            ccvins = 2;  // order and marker
            ccvouts = 0;
            vin_coins = AssetValidateBuyvin<A>(cpAssets, eval, vin_unit_price, vin_origpubkey, origCCaddrDummy, origNormalAddr, tx, assetid);
            if (vin_coins == 0)
                return false;  // eval already set
            
            if (tx.vout[0].nValue > ASSETS_NORMAL_DUST)  {
                if (!A::ConstrainVout(tx.vout[0], NORMALVOUT, origNormalAddr, vin_coins, 0))
                    return eval->Invalid("invalid refund for cancelbid");
            } else {
                // dust must go to global cc address
                if (!A::ConstrainVout(tx.vout[0], CCVOUT, globalAssetsCCaddr, vin_coins, 0))
                    return eval->Invalid("invalid dust refund for cancelbid");
                ccvouts ++;
            }
            if (TotalPubkeyNormalInputs(eval, tx, pubkey2pk(vin_origpubkey)) == 0) // check tx is signed by originator pubkey
                return eval->Invalid("not the originator pubkey signed for cancelbid");
            break;
            
        case 'B': // fillbid:
            //vin.0: normal input
            //vin.1: cc assets unspendable (vout.0 from buyoffer) buyTx.vout[0]
            //vin.2+: valid CC output satisfies buyoffer (*tx.vin[2])->nValue
            //vout.0: remaining amount of bid to cc assets global address (if sufficient to buy a token or dust)
            //vout.1: vin.1 value to signer of vin.2
            //vout.2: vin.2 satoshi to original pubkey (except royalty if any)
            //vout.3: royalty to token owner (optional)
            //vout.4: CC output for satoshi change (if any)
            //vout.5: normal output for change (if any)
            //vout.n-1: opreturn [EVAL_ASSETS] ['B'] [assetid] [remaining asset required] [origpubkey]
			ccvins = 1;
            ccvouts = 0;  // at least tokens output
            vin_coins = AssetValidateBuyvin<A>(cpAssets, eval, vin_unit_price, vin_origpubkey, origTokensCCaddr, origNormalAddr, tx, assetid);
            if (vin_coins == 0)
                return false;  // eval already set
            else if(tx.vout.size() < 3)
                return eval->Invalid("not enough vouts for fillbid");
            else if(vin_origpubkey != vorigpubkey)    // originator pk does not change in the new opret
                return eval->Invalid("mismatched opreturn originator pubkeys for fillbid");
            else if (unit_price != vin_unit_price)
                return eval->Invalid("mismatched unit price for fillbid");
            else if (unit_price <= 0)
                return eval->Invalid("invalid unit price");
            else
            {
                if (royaltyFract < 0LL || royaltyFract >= TKLROYALTY_DIVISOR)
                    return eval->Invalid("invalid royalty value");

                CAmount royaltyValue = royaltyFract > 0 ? vin_coins / TKLROYALTY_DIVISOR * royaltyFract : 0;
                if (royaltyValue <= ASSETS_NORMAL_DUST)
                    royaltyValue = 0LL; // reset if dust
                int32_t r = royaltyValue > 0LL ? 1 : 0;
                /*int32_t r = 0;
                if (royaltyFract > 0 && tx.vout[1].nValue > ASSETS_NORMAL_DUST / royaltyFract * TKLROYALTY_DIVISOR - ASSETS_NORMAL_DUST)  // if vout1 > min  
                    r = 1;
                if (royaltyFract > 0)
                    std::cerr << __func__ << " r=" << r << " tx.vout[1].nValue=" << tx.vout[1].nValue << " min-vout1=" << (ASSETS_NORMAL_DUST / royaltyFract * TKLROYALTY_DIVISOR - ASSETS_NORMAL_DUST) << std::endl;*/
                //std::cerr << __func__ << " royaltyFract=" << royaltyFract << " royaltyValue=" << royaltyValue << std::endl;

                CAmount assetoshis = tx.vout[0].nValue + tx.vout[1].nValue + (r ? tx.vout[2].nValue : 0LL);
                if( vin_coins != assetoshis )             // coins -> global cc address (remainder) + normal self address
                    return eval->Invalid("input cc value does not equal to vout0+1" + std::string(r ? "+2" : "") + " for fillbid");

                // coins remainder
                if (tx.vout[0].nValue / unit_price > 0 || tx.vout[0].nValue <= ASSETS_NORMAL_DUST) // there are remaining tokens to buy or dust must return to global cc address
                {
                    if (!A::ConstrainVout(tx.vout[0], CCVOUT, globalAssetsCCaddr, 0LL, A::EvalCode()))  // if remainder sufficient to buy tokens -> coins to asset global cc addr
                        return eval->Invalid("mismatched vout0 global assets CC addr for fillbid");
                    ccvouts ++;
                }
                else 
                {   // if remaining_units == 0 (empty bid) then the remainder should go to the originator normal address, if it is not dust
                    if (!A::ConstrainVout(tx.vout[0], NORMALVOUT, origNormalAddr, 0LL, 0))  // remainder less than token price should return to originator normal addr
                        return eval->Invalid("vout0 should be originator normal address with remainder for fillbid");
                }

                vin_tokens = AssetsGetCCInputs(eval, cpTokens, NULL, tx);
                int32_t myNormalVout = 1;
                int32_t myTokenVout = 2+r;
                int32_t tokenRemainderVout; 
                if (tx.vout[0].nValue / unit_price > 0)
                    tokenRemainderVout = 4+r;  // marker present before this vout 
                else
                    tokenRemainderVout = 3+r;  // no marker
                
                if (tx.vout.size() > tokenRemainderVout && tx.vout[tokenRemainderVout].scriptPubKey.IsPayToCryptoCondition())    // if tokens remainder exists 
                {
                    if (!A::ConstrainVout(tx.vout[myTokenVout], CCVOUT, origTokensCCaddr, 0LL, T::EvalCode()))	// tokens to originator cc addr (tokens+nonfungible evalcode)
                        return eval->Invalid("vout" + std::to_string(myTokenVout) + " tokens value should go to originator pubkey for fillbid");
                    else if (vin_tokens != tx.vout[myTokenVout].nValue + tx.vout[tokenRemainderVout].nValue)    // tokens from cc global address -> token global addr (remainder) + originator cc address
                        return eval->Invalid("tokens inputs doesnt match tokens vout" + std::to_string(myTokenVout) + "+" + "vout"+std::to_string(tokenRemainderVout) + " for fillbid");
                    ccvouts += 2;
                }
                else {
                    if (tx.vout.size() <= myTokenVout || !A::ConstrainVout(tx.vout[myTokenVout], CCVOUT, origTokensCCaddr, vin_tokens, T::EvalCode()))   // all tokens to originator cc addr, no cc change present
                        return eval->Invalid("vout" + std::to_string(myTokenVout) + " should have tokens to originator cc addr for fillbid");
                    ccvouts ++;
                }

                if (!A::ConstrainVout(tx.vout[myNormalVout], NORMALVOUT, NULL, 0LL, 0))                            // amount paid for tokens goes to normal addr (we can't check 'self' address)
                    return eval->Invalid("vout " + std::to_string(myNormalVout) + " should be normal for fillbid");

                if (tx.vout[0].nValue / unit_price > 0)  { // bid coins remainder sufficient for more tokens - marker should exist
                    int32_t markerVout = 3 + r;
                    if (tx.vout.size() <= markerVout || !A::ConstrainVout(tx.vout[markerVout], CCVOUT, origAssetsCCaddr, ASSETS_MARKER_AMOUNT, A::EvalCode()))       // marker to originator asset cc addr
                        return eval->Invalid("invalid vout" + std::to_string(markerVout) + " marker for originator pubkey for fillbid");
                    ccvouts ++;
                }
                
                CAmount received_value = r ? tx.vout[1].nValue + tx.vout[2].nValue : tx.vout[1].nValue; // vout1 paid value to seller, vout2 royalty to owner
                CAmount paid_units = tx.vout[2+r].nValue;
                if (!ValidateBidRemainder(unit_price, tx.vout[0].nValue, vin_coins, received_value, paid_units)) // check real price and coins spending from global addr
                    return eval->Invalid("vout" + std::to_string(2+r) + " mismatched remainder for fillbid");
                if (r) {
                    if ((tx.vout[1].nValue + tx.vout[2].nValue) / TKLROYALTY_DIVISOR * royaltyFract != tx.vout[2].nValue)  // validate royalty value
                        return eval->Invalid("vout2 invalid royalty amount for fillask");
                    if (!A::ConstrainVout(tx.vout[2], NORMALVOUT, ownerNormalAddr, 0LL, 0))        // validate owner royalty dest
                        return eval->Invalid("vout2 invalid royalty detination for fillask");
                }
            }
            break;

        //case 'e': // sell swap offer
        //    break; // disable swaps

        case 's': // ask offer
            //vin.0: normal input
            //vin.1+: valid CC output for sale
            //vout.0: vin.1 assetoshis output to CC assets global address
            //vout.1: CC output to CC assets global address for marker
            //vout.2: CC output for change (if any)
            //vout.3: normal output for change (if any)
            //'s'.vout.n-1: opreturn [EVAL_ASSETS] ['s'] [assetid] [amount of native coin required] [origpubkey]
            //'e'.vout.n-1: opreturn [EVAL_ASSETS] ['e'] [assetid] [assetid2] [amount of asset2 required] [origpubkey]

            // as we don't use tokenconvert 's' does not have cc inputs and we should not be here:
            if (cpAssets->evalcode == EVAL_ASSETS)
                return eval->Invalid("invalid asset funcid (s)");
            else {
                // for EVAL_ASSETSV2 check the creation tx 
                char origTokenAddr[KOMODO_ADDRESS_BUFSIZE];
                CPubKey origpk = pubkey2pk(vorigpubkey);
                ccvouts = 2;
                if (tx.vout.size() < 3)
                    return eval->Invalid("too few vouts");
                else if (A::ConstrainVout(tx.vout[0], CCVOUT, tokensDualEvalUnspendableCCaddr, 0LL, T::EvalCode()) == false)      // tokens sent to global addr
                    return eval->Invalid("invalid vout0 global two eval address for sell");
                else if( A::ConstrainVout(tx.vout[1], CCVOUT, origAssetsCCaddr, ASSETS_MARKER_AMOUNT, A::EvalCode()) == false )  // marker to originator asset cc addr
                    return eval->Invalid("invalid vout1 marker for originator pubkey");
                else if (TotalPubkeyNormalInputs(eval, tx, origpk) == 0)  // check tx is signed by originator pubkey
                    return eval->Invalid("not the originator pubkey signed for ask");
                else if (unit_price <= ASSETS_NORMAL_DUST)
                    return eval->Invalid("invalid or too low unit price");

                if (tx.vout[2].scriptPubKey.IsPayToCryptoCondition())
                    if (!tx.vout[2].scriptPubKey.IsPayToCCV2() || tx.vout[2].scriptPubKey.SpkHasEvalcodeCCV2(T::EvalCode()))  // have token change
                        ccvouts ++;

                // check should not be assets cc vins for tokenv2ask at all:
                for (auto const &vin : tx.vin)   {
                    if (cpAssets->ismyvin(vin.scriptSig))
                        return eval->Invalid("could not have cc assets vin for creation tx");
                }
            }
            break;
            
        case 'x': // cancel ask            
            //vin.0: normal input
            //vin.1: cc assets global address (vout.0 from exchange or selloffer) sellTx/exchangeTx.vout[0] inputTx
            //vin.2: CC marker from selloffer for txfee
            //vout.0: vin.1 assetoshis to original pubkey CC sellTx/exchangeTx.vout[0].nValue -> [origpubkey]
            //vout.1: vin.2 back to users pubkey
            //vout.2: normal output for change (if any)
            //vout.n-1: opreturn [EVAL_ASSETS] ['x'] [assetid]
            vin_tokens = AssetValidateSellvin<A>(cpAssets, eval, vin_unit_price, vin_origpubkey, origTokensCCaddr, origNormalAddr, tx, assetid);
            if (vin_tokens == 0)  
                return false;  // eval already set
            else if (A::ConstrainVout(tx.vout[0], CCVOUT, origTokensCCaddr, vin_tokens, T::EvalCode()) == false)      // tokens returning to originator cc addr
                return eval->Invalid("invalid vout0 for cancelask");
            else if (TotalPubkeyNormalInputs(eval, tx, pubkey2pk(vin_origpubkey)) == 0)  // check tx is signed by originator pubkey
                return eval->Invalid("not the originator pubkey signed for cancelask");
            ccvins = 2;  // order and marker vins
            ccvouts = 1;  // token back to the originator
            break;
            
        case 'S': // fill ask
            //vin.0: normal input
            //vin.1: cc assets global address (vout.0 assetoshis from selloffer) sellTx.vout[0]
            //'S'.vin.2+: normal output that satisfies selloffer (*tx.vin[2])->nValue
            //vout.0: remaining assetoshis -> cc assets global address
            //vout.1: vin.1 assetoshis to signer of vin.2 sellTx.vout[0].nValue -> any
            //'S'.vout.2: vin.2 value to original pubkey [origpubkey]
            //vout.3: normal output for change (if any)
            //'S'.vout.n-1: opreturn [EVAL_ASSETS] ['S'] [assetid] [amount of coin still required] [origpubkey]
			ccvins = 1;
            ccvouts = 2;
            vin_tokens = AssetValidateSellvin<A>(cpAssets, eval, vin_unit_price, vin_origpubkey, origTokensCCaddr, origNormalAddr, tx, assetid);
            if (vin_tokens == 0)
                return false;  // eval is already set
            else if (tx.vout.size() < 3)
                return eval->Invalid("not enough vouts for fillask");
            else if (vin_origpubkey != vorigpubkey)
                return eval->Invalid("mismatched origpubkeys for fillask");
            else if (unit_price != vin_unit_price)
                return eval->Invalid("mismatched unit price for fillask");
            else
            {
                if (vin_tokens != tx.vout[0].nValue + tx.vout[1].nValue)
                    return eval->Invalid("locked value doesnt match vout0+1 fillask");
                if (royaltyFract < 0LL || royaltyFract >= TKLROYALTY_DIVISOR)
                    return eval->Invalid("invalid royalty value");

                int32_t r = royaltyFract > 0 ? 1 : 0;
                // check if royalty not dust
                if (royaltyFract > 0 && tx.vout[2].nValue <= ASSETS_NORMAL_DUST / royaltyFract * TKLROYALTY_DIVISOR - ASSETS_NORMAL_DUST)  // if value paid to seller less than when the royalty is minimum
                    r = 0;
                //if (royaltyValue <= ASSETS_NORMAL_DUST)
                //    royaltyValue = 0LL; // reset if dust
                CAmount paid_value = r > 0 ? tx.vout[2].nValue + tx.vout[3].nValue : tx.vout[2].nValue; // vout2 paid value to seller, vout3 royalty to owner
                if (!ValidateAskRemainder(unit_price, tx.vout[0].nValue, vin_tokens, tx.vout[1].nValue, paid_value)) 
                    return eval->Invalid("mismatched vout0 remainder for fillask");
                else if (!A::ConstrainVout(tx.vout[1], CCVOUT, NULL, 0LL, T::EvalCode()))      // do not check tokens buyer's 'self' cc addr
                    return eval->Invalid("vout1 should be cc for fillask");
                else if (!A::ConstrainVout(tx.vout[2], NORMALVOUT, origNormalAddr, 0LL, 0))        // coins to originator normal addr
                    return eval->Invalid("vout2 should be cc for fillask");
                if (r > 0)  {
                    if ((tx.vout[2].nValue + tx.vout[3].nValue) / TKLROYALTY_DIVISOR * royaltyFract != tx.vout[3].nValue)  // validate royalty value
                        return eval->Invalid("vout3 invalid royalty amount for fillask");
                    if (!A::ConstrainVout(tx.vout[3], NORMALVOUT, ownerNormalAddr, 0LL, 0))        // validate owner royalty dest
                        return eval->Invalid("vout3 invalid royalty destination for fillask");
                }
                if (!A::ConstrainVout(tx.vout[0], CCVOUT, tokensDualEvalUnspendableCCaddr, 0LL, A::EvalCode()))   // tokens remainder on global addr
                    return eval->Invalid("invalid vout0 should pay to tokens/assets global address for fillask");
                if (tx.vout[0].nValue > 0)  {
                    int32_t markerVout = r > 0 ? 4 : 3;
                    // marker should exist if remainder not empty
                    if (tx.vout.size() <= markerVout || A::ConstrainVout(tx.vout[markerVout], CCVOUT, origAssetsCCaddr, ASSETS_MARKER_AMOUNT, A::EvalCode()) == false)  // marker to originator asset cc addr
                        return eval->Invalid("invalid marker vout for original pubkey");
                    ccvouts ++;
                }
            }
            break;

        case 'E': // fillexchange	
			////////// not implemented yet ////////////
            return eval->Invalid("unexpected assets fillexchange funcid");
            break; // disable asset swaps
            //vin.0: normal input
            //vin.1: unspendable.(vout.0 assetoshis from selloffer) sellTx.vout[0]
            //vin.2+: valid CC assetid2 output that satisfies exchange (*tx.vin[2])->nValue
            //vout.0: remaining assetoshis -> unspendable
            //vout.1: vin.1 assetoshis to signer of vin.2 sellTx.vout[0].nValue -> any
            //vout.2: vin.2+ assetoshis2 to original pubkey [origpubkey]
            //vout.3: CC output for asset2 change (if any)
            //vout.3/4: normal output for change (if any)
            //vout.n-1: opreturn [EVAL_ASSETS] ['E'] [assetid vin0+1] [assetid vin2] [remaining asset2 required] [origpubkey]
            
			//if ( AssetExactAmounts(false, cp,inputs,outputs,eval,tx,assetid2) == false )    
            //    eval->Invalid("asset2 inputs != outputs");

			////////// not implemented yet ////////////
            /*if( (vin_tokens = AssetValidateSellvin<A>(cpTokens, eval, vin_unit_price, vin_origpubkey, origTokensCCaddr, origNormalAddr, tx, assetid)) == 0 )
                return(false);
            else if(tx.vout.size() < 3)
                return eval->Invalid("not enough vouts for fillex");
            else if(vin_origpubkey != vorigpubkey)
                return eval->Invalid("mismatched origpubkeys for fillex");
            else if (unit_price != vin_unit_price)
                return eval->Invalid("mismatched unit price for fillex");
            else
            {
                if( tokens != tx.vout[0].nValue + tx.vout[1].nValue )
                    return eval->Invalid("locked value doesnt match vout0+1 fillex");
                else if( tx.vout[3].scriptPubKey.IsPayToCryptoCondition() != false )
				////////// not implemented yet ////////////
                {
                    if( ConstrainVout(tx.vout[2], CCVOUT, origTokensCCaddr, 0) == false )
                        return eval->Invalid("vout2 doesnt go to origpubkey fillex");
                    else if( tokens != tx.vout[2].nValue + tx.vout[3].nValue )
                    {
                        fprintf(stderr,"inputs %.8f != %.8f + %.8f\n",(double)tokens/COIN,(double)tx.vout[2].nValue/COIN,(double)tx.vout[3].nValue/COIN);
                        return eval->Invalid("asset inputs doesnt match vout2+3 fillex");
                    }
                }
				////////// not implemented yet ////////////
                else if( ConstrainVout(tx.vout[2], CCVOUT, origTokensCCaddr, tokens) == false )  
                    return eval->Invalid("vout2 doesnt match inputs fillex");
                else if( ConstrainVout(tx.vout[1], NORMALVOUT, 0, 0) == false )
                    return eval->Invalid("vout1 is CC for fillex");
                fprintf(stderr,"assets vout0 %lld, tokens %lld, vout2 %lld -> orig, vout1 %lld, total %lld\n", (long long)tx.vout[0].nValue, (long long)tokens,(long long)tx.vout[2].nValue,(long long)tx.vout[1].nValue,(long long)orig_units);
                if( ValidateSwapRemainder(remaining_units, tx.vout[0].nValue, tokens, tx.vout[1].nValue, tx.vout[2].nValue, orig_units) == false )
                //    return eval->Invalid("mismatched remainder for fillex");
                else if( ConstrainVout(tx.vout[1], CCVOUT, 0, 0) == false )
				////////// not implemented yet ////////////
                    return eval->Invalid("normal vout1 for fillex");
                else if( remaining_units != 0 ) 
                {
                    if( ConstrainVout(tx.vout[0], CCVOUT, (char *)cpAssets->unspendableCCaddr, 0) == false )  // TODO: unsure about this, but this is not impl yet anyway
                        return eval->Invalid("mismatched vout0 AssetsCCaddr for fillex");
                }
            }*/
			////////// not implemented yet ////////////
            break;

        default:
            fprintf(stderr,"illegal assets funcid.(%c)\n",funcid);
            return eval->Invalid("unexpected assets funcid");
    }

    // replaced PreventCC with min/max cc vin/vout calc to allow vin position flexibility:
    if (!CountCCVinVouts(cpAssets, tx, ccvins, ccvouts))
        return eval->Invalid("invalid cc vin or vout count");
    return true;
}

// redirect to AssetsValidateInternal and log error
bool AssetsValidate(struct CCcontract_info *cpAssets, Eval* eval,const CTransaction &tx, uint32_t nIn)
{
    // add specific chains exceptions for old token support:
    if (strcmp(ASSETCHAINS_SYMBOL, "SEC") == 0 && chainActive.Height() <= 144073)
        return true;
    
    if (strcmp(ASSETCHAINS_SYMBOL, "MGNX") == 0 && chainActive.Height() <= 210190)
        return true;

    if (!TokensIsVer1Active(NULL))   {	 
        bool valid = tokensv0::AssetsValidate(cpAssets, eval, tx, nIn); // call assets validation version 0
        if (!valid) 
            LOGSTREAMFN(ccassets_log, CCLOG_ERROR, stream << "v0 validation error: " << eval->state.GetRejectReason() << ", code: " << eval->state.GetRejectCode() << ", tx: " << HexStr(E_MARSHAL(ss << tx)) << std::endl);
        return valid;
    }

    if (!AssetsValidateInternal<TokensV1, AssetsV1>(cpAssets, eval, tx, nIn))    {
        LOGSTREAMFN(ccassets_log, CCLOG_ERROR, stream << "validation error: " << eval->state.GetRejectReason() << ", code: " << eval->state.GetRejectCode() << ", tx: " << HexStr(E_MARSHAL(ss << tx)) << std::endl);
        return false;
    }
    return true;
}

// redirect to AssetsValidateInternal and log error
bool Assetsv2Validate(struct CCcontract_info *cp, Eval* eval,const CTransaction &tx, uint32_t nIn)
{
    if (!AssetsValidateInternal<TokensV2, AssetsV2>(cp, eval, tx, nIn))    {
        LOGSTREAMFN(ccassets_log, CCLOG_ERROR, stream << "validation error: " << eval->state.GetRejectReason() << ", code: " << eval->state.GetRejectCode() << ", tx: " << HexStr(E_MARSHAL(ss << tx)) << std::endl);
        return false;
    }
    return true;
}