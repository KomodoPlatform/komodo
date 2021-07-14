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

#ifndef CC_ASSETS_TX_IMPL_H
#define CC_ASSETS_TX_IMPL_H

#include "CCtokens.h"
#include "CCassets.h"
#include "CCTokelData.h"

template<class T, class A>
UniValue AssetOrders(uint256 refassetid, CPubKey pk, uint8_t evalcodeAdd)
{
	UniValue result(UniValue::VARR);  
    const char *funcname = __func__;

    struct CCcontract_info *cpAssets, assetsC;
    struct CCcontract_info *cpTokens, tokensC;

    cpAssets = CCinit(&assetsC, A::EvalCode());
    cpTokens = CCinit(&tokensC, T::EvalCode());

	auto addOrders = [&](struct CCcontract_info *cp, std::vector<std::pair<CAddressUnspentKey, CAddressUnspentValue> >::const_iterator it)
	{
		uint256 txid, hashBlock, assetid, assetid2;
		int64_t unit_price;
		std::vector<uint8_t> origpubkey;
		CTransaction ordertx;
		uint8_t funcid, evalCode;
		char origaddr[KOMODO_ADDRESS_BUFSIZE], origtokenaddr[KOMODO_ADDRESS_BUFSIZE];

        txid = it->first.txhash;
        LOGSTREAM(ccassets_log, CCLOG_DEBUG2, stream << funcname << " checking txid=" << txid.GetHex() << std::endl);
        if (!myGetTransaction(txid, ordertx, hashBlock)) {
            LOGSTREAM(ccassets_log, CCLOG_DEBUG2, stream << funcname <<" could not load order txid=" << txid.GetHex() << std::endl);
            return;
        }

        if (ordertx.vout.size() > 1 && (funcid = A::DecodeAssetTokenOpRet(ordertx.vout.back().scriptPubKey, evalCode, assetid, assetid2, unit_price, origpubkey)) != 0)
        {
            LOGSTREAM(ccassets_log, CCLOG_DEBUG2, stream << funcname << " checking ordertx.vout.size()=" << ordertx.vout.size() << " funcid=" << (char)(funcid ? funcid : ' ') << " assetid=" << assetid.GetHex() << std::endl);

            if (!pk.IsValid() && (refassetid == zeroid || assetid == refassetid) || // tokenorders
                pk.IsValid() && pk == pubkey2pk(origpubkey))  // mytokenorders
            {
                uint256 spenttxid;
                uint256 init_txid = txid;
                int32_t spentvin;
                int32_t height;
                while(CCgetspenttxid(spenttxid, spentvin, height, init_txid, ASSETS_GLOBALADDR_VOUT) == 0 && IsTxidInActiveChain(spenttxid)) {
                    init_txid = spenttxid;
                }
                if (init_txid != txid) {
                    txid = init_txid;
                    if (!myGetTransaction(txid, ordertx, hashBlock)) {
                        LOGSTREAM(ccassets_log, CCLOG_DEBUG2, stream << funcname << " could not load order txid=" << txid.GetHex() << std::endl);
                        return;
                    }
                    if ((funcid = A::DecodeAssetTokenOpRet(ordertx.vout.back().scriptPubKey, evalCode, assetid, assetid2, unit_price, origpubkey)) == 0) {
                        LOGSTREAM(ccassets_log, CCLOG_DEBUG2, stream << funcname << " could not decode order txid=" << txid.GetHex() << std::endl);
                        return;
                    }
                }

                if (ordertx.vout.size() < 2)  {
                    LOGSTREAM(ccassets_log, CCLOG_DEBUG2, stream << funcname << " txid skipped " << txid.GetHex() << std::endl);
                    return;
                }

                UniValue item(UniValue::VOBJ);

                std::string funcidstr(1, (char)funcid);
                item.push_back(Pair("funcid", funcidstr));
                item.push_back(Pair("txid", txid.GetHex()));
                if (funcid == 'b' || funcid == 'B')
                {
                    item.push_back(Pair("bidamount", ValueFromAmount(ordertx.vout[0].nValue)));
                }
                else if (funcid == 's' || funcid == 'S')
                {
                    item.push_back(Pair("askamount", ordertx.vout[0].nValue));
                }
                if (origpubkey.size() == CPubKey::COMPRESSED_PUBLIC_KEY_SIZE)
                {
                    GetCCaddress(cp, origaddr, pubkey2pk(origpubkey), A::IsMixed());  
                    item.push_back(Pair("origaddress", origaddr));
                    GetTokensCCaddress(cpTokens, origtokenaddr, pubkey2pk(origpubkey), A::IsMixed());
                    item.push_back(Pair("origtokenaddress", origtokenaddr));
                }
                if (assetid != zeroid)
                    item.push_back(Pair("tokenid", assetid.GetHex()));
                if (assetid2 != zeroid)
                    item.push_back(Pair("otherid", assetid2.GetHex()));
                if (unit_price > 0)
                {
                    if (funcid == 's' || funcid == 'S' /*|| funcid == 'e' || funcid == 'E' not supported */)
                    {
                        item.push_back(Pair("totalrequired", ValueFromAmount(unit_price * ordertx.vout[0].nValue)));
                        item.push_back(Pair("price", ValueFromAmount(unit_price)));
                    }
                    else if (funcid == 'b' || funcid == 'B')
                    {
                        item.push_back(Pair("totalrequired", unit_price ? ordertx.vout[0].nValue / unit_price : 0));
                        item.push_back(Pair("price", ValueFromAmount(unit_price)));
                    }
                }
                result.push_back(item);
                LOGSTREAM(ccassets_log, CCLOG_DEBUG1, stream << funcname << " added order funcId=" << (char)(funcid ? funcid : ' ') << " it->first.index=" << it->first.index << " ordertx.vout[it->first.index].nValue=" << ordertx.vout[it->first.index].nValue << " tokenid=" << assetid.GetHex() << std::endl);
            }
        }
	};

    

    if (!pk.IsValid())
    {
        std::vector<std::pair<CAddressUnspentKey, CAddressUnspentValue> > unspentOutputsTokens, unspentOutputsNFTs, unspentOutputsCoins;
        char assetsUnspendableAddr[KOMODO_ADDRESS_BUFSIZE];
        GetCCaddress(cpAssets, assetsUnspendableAddr, GetUnspendable(cpAssets, NULL), A::IsMixed());
        SetCCunspents(unspentOutputsCoins, assetsUnspendableAddr, true);

        char assetsTokensUnspendableAddr[KOMODO_ADDRESS_BUFSIZE];
        TokenDataTuple tokenData;
        vuint8_t vextraData;
        if (refassetid != zeroid) {
            GetTokenData<T>(NULL, refassetid, tokenData, vextraData);
            if (vextraData.size() > 0)
                cpAssets->evalcodeAdd = vextraData[0];
        }
        GetTokensCCaddress(cpAssets, assetsTokensUnspendableAddr, GetUnspendable(cpAssets, NULL), A::IsMixed());
        SetCCunspents(unspentOutputsTokens, assetsTokensUnspendableAddr, true);

        // tokenbids:
        for (std::vector<std::pair<CAddressUnspentKey, CAddressUnspentValue> >::const_iterator itCoins = unspentOutputsCoins.begin();
            itCoins != unspentOutputsCoins.end();
            itCoins++)
            addOrders(cpAssets, itCoins);
        
        // tokenasks:
        for (std::vector<std::pair<CAddressUnspentKey, CAddressUnspentValue> >::const_iterator itTokens = unspentOutputsTokens.begin();
            itTokens != unspentOutputsTokens.end();
            itTokens++)
            addOrders(cpAssets, itTokens);

        if (evalcodeAdd != 0) {  //this would be mytokenorders
            char assetsNFTUnspendableAddr[KOMODO_ADDRESS_BUFSIZE];

            // try also dual eval tokenasks (and we do not need bids (why? bids are on assets global addr anyway.)):
            cpAssets->evalcodeAdd = evalcodeAdd;
            GetTokensCCaddress(cpAssets, assetsNFTUnspendableAddr, GetUnspendable(cpAssets, NULL), A::IsMixed());
            SetCCunspents(unspentOutputsNFTs, assetsNFTUnspendableAddr,true);

            for (std::vector<std::pair<CAddressUnspentKey, CAddressUnspentValue> >::const_iterator itNFTs = unspentOutputsNFTs.begin();
                itNFTs != unspentOutputsNFTs.end();
                itNFTs++)
                addOrders(cpAssets, itNFTs);
        }
    }
    else 
    {
        // use marker on my pk:
        std::vector<std::pair<CAddressUnspentKey, CAddressUnspentValue> > unspentsMyAddr;
        char assetsMyAddr[KOMODO_ADDRESS_BUFSIZE];
        GetCCaddress(cpAssets, assetsMyAddr, pk, A::IsMixed());
        SetCCunspents(unspentsMyAddr, assetsMyAddr, true);
        for (std::vector<std::pair<CAddressUnspentKey, CAddressUnspentValue> >::const_iterator itOrders = unspentsMyAddr.begin();
            itOrders != unspentsMyAddr.end();
            itOrders++)
            addOrders(cpAssets, itOrders);
    }
    return(result);
}

// rpc tokenbid implementation, locks 'bidamount' coins for the 'pricetotal' of tokens
template<class T, class A>
UniValue CreateBuyOffer(const CPubKey &mypk, int64_t txfee, int64_t bidamount, uint256 assetid, int64_t numtokens)
{
    CMutableTransaction mtx = CreateNewContextualCMutableTransaction(Params().GetConsensus(), komodo_nextheight());
	struct CCcontract_info *cpAssets, C; 
	uint256 hashBlock; 
	CTransaction vintx; 
	std::vector<uint8_t> origpubkey; 
	std::string name,description;
	int64_t inputs;
    std::vector <vscript_t> oprets;

    if (bidamount <= 0 || numtokens <= 0)    {
        CCerror = "invalid bidamount or numtokens";
        return("");
    }

    // check if valid token
    if (myGetTransaction(assetid, vintx, hashBlock) == 0)     {
        CCerror = "could not find assetid\n";
        return("");
    }
    if (vintx.vout.size() == 0 || T::DecodeTokenCreateOpRet(vintx.vout.back().scriptPubKey, origpubkey, name, description, oprets) == 0)    {
        CCerror = "assetid isn't token creation txid\n";
        return("");
    }

    cpAssets = CCinit(&C, A::EvalCode());   // NOTE: assets here!
    if (txfee == 0)
        txfee = 10000;

    // use AddNormalinputsRemote to sign only with mypk
    if ((inputs = AddNormalinputsRemote(mtx, mypk, bidamount+(txfee+ASSETS_MARKER_AMOUNT), 0x10000)) > 0)   
    {
		if (inputs < bidamount+txfee) {
			CCerror = strprintf("insufficient coins to make buy offer");
			return ("");
		}

        CAmount unit_price = bidamount / numtokens;
        if (unit_price <= 0)  {
            CCerror = "invalid bid params";
            return ("");
        }
		CPubKey unspendableAssetsPubkey = GetUnspendable(cpAssets, 0);
        mtx.vout.push_back(T::MakeCC1vout(A::EvalCode(), bidamount, unspendableAssetsPubkey));
        mtx.vout.push_back(T::MakeCC1vout(A::EvalCode(), ASSETS_MARKER_AMOUNT, mypk));  // marker for my orders

        UniValue sigData = T::FinalizeCCTx(IsRemoteRPCCall(), FINALIZECCTX_NO_CHANGE_WHEN_DUST, cpAssets, mtx, mypk, txfee, 
			T::EncodeTokenOpRet(assetid, {},     // TODO: actually this tx is not 'tokens', maybe it is better not to have token opret here but only asset opret.
				{ A::EncodeAssetOpRet('b', zeroid, unit_price, vuint8_t(mypk.begin(), mypk.end())) } ));   // But still such token opret should not make problems because no token eval in these vouts
        if (!ResultHasTx(sigData))
            return MakeResultError("Could not finalize tx");
        return sigData;
        
    }
	CCerror = "no coins found to make buy offer";
    return("");
}

// rpc tokenask implementation, locks 'numtokens' tokens for the 'askamount' 
template<class T, class A>
UniValue CreateSell(const CPubKey &mypk, int64_t txfee, int64_t numtokens, uint256 assetid, int64_t askamount)
{
    CMutableTransaction mtx = CreateNewContextualCMutableTransaction(Params().GetConsensus(), komodo_nextheight());
	uint64_t mask = 0LL; 
	struct CCcontract_info *cpAssets, assetsC;
	struct CCcontract_info *cpTokens, tokensC;

    if (numtokens <= 0 || askamount <= 0)    {
        CCerror = "invalid askamount or numtokens";
        return("");
    }

    cpAssets = CCinit(&assetsC, A::EvalCode());  // NOTE: for signing
   
    if (txfee == 0)
        txfee = 10000;

    if (AddNormalinputsRemote(mtx, mypk, txfee+ASSETS_MARKER_AMOUNT, 0x10000) > 0)   // use AddNormalinputsRemote to sign with mypk
    {
        CAmount inputs;
		// add single-eval tokens (or non-fungible tokens):
        cpTokens = CCinit(&tokensC, T::EvalCode());  // NOTE: adding inputs only from EVAL_TOKENS cc
        if ((inputs = AddTokenCCInputs<T>(cpTokens, mtx, mypk, assetid, numtokens, 0x1000, false)) > 0LL)
        {
			if (inputs < numtokens) {
				CCerror = "insufficient tokens for ask";
				return ("");
			}

            CAmount unit_price = askamount / numtokens;
            if (unit_price <= 0)  {
				CCerror = "invalid ask params";
				return ("");
			}

            uint8_t evalcodeAdd = cpTokens->evalcodeAdd ? cpTokens->evalcodeAdd : 0;

			CPubKey unspendableAssetsPubkey = GetUnspendable(cpAssets, NULL);
            mtx.vout.push_back(T::MakeTokensCC1vout(A::EvalCode(), evalcodeAdd, numtokens, unspendableAssetsPubkey));
            mtx.vout.push_back(T::MakeCC1vout(A::EvalCode(), ASSETS_MARKER_AMOUNT, mypk));  //marker (seems, it is not for my tokenorders, not used yet)
            CAmount CCchange = inputs - numtokens;
            if (CCchange != 0LL) {
                // change to single-eval or non-fungible token vout (although for non-fungible token change currently is not possible)
                mtx.vout.push_back(T::MakeTokensCC1vout(evalcodeAdd ? evalcodeAdd : T::EvalCode(), CCchange, mypk));	
            }

            // cond to spend NFT from mypk 
            CCwrapper wrCond(T::MakeTokensCCcond1(evalcodeAdd, mypk));
            CCAddVintxCond(cpTokens, wrCond, NULL); //NULL indicates to use myprivkey

            UniValue sigData = T::FinalizeCCTx(IsRemoteRPCCall(), FINALIZECCTX_NO_CHANGE_WHEN_DUST, cpTokens, mtx, mypk, txfee, 
                T::EncodeTokenOpRet(assetid, { unspendableAssetsPubkey }, 
                    { A::EncodeAssetOpRet('s', zeroid, unit_price, vuint8_t(mypk.begin(), mypk.end()) ) } ));
            if (!ResultHasTx(sigData))
                return MakeResultError("Could not finalize tx");
            return sigData;
		}
		else {
            CCerror = "need some tokens to place ask";
		}
    }
	else {  
        CCerror = "need some native coins to place ask";
	}
    return("");
}

////////////////////////// NOT IMPLEMENTED YET/////////////////////////////////
template<class T, class A>
std::string CreateSwap(int64_t txfee,int64_t askamount,uint256 assetid,uint256 assetid2,int64_t pricetotal)
{
    CMutableTransaction mtx = CreateNewContextualCMutableTransaction(Params().GetConsensus(), komodo_nextheight());
    CPubKey mypk; uint64_t mask; int64_t inputs,CCchange; CScript opret; struct CCcontract_info *cp,C;

	////////////////////////// NOT IMPLEMENTED YET/////////////////////////////////
    fprintf(stderr,"asset swaps disabled\n");
    return("");
	////////////////////////// NOT IMPLEMENTED YET/////////////////////////////////

    if ( askamount < 0 || pricetotal < 0 )
    {
        fprintf(stderr,"negative askamount %lld, askamount %lld\n",(long long)pricetotal,(long long)askamount);
        return("");
    }
    cp = CCinit(&C, EVAL_ASSETS);

    if ( txfee == 0 )
        txfee = 10000;
	////////////////////////// NOT IMPLEMENTED YET/////////////////////////////////
    mypk = pubkey2pk(Mypubkey());

    if (AddNormalinputs(mtx, mypk, txfee, 0x10000) > 0)
    {
        mask = ~((1LL << mtx.vin.size()) - 1);
        /*if ((inputs = AddAssetInputs(cp, mtx, mypk, assetid, askamount, 60)) > 0)
        {
			////////////////////////// NOT IMPLEMENTED YET/////////////////////////////////
			if (inputs < askamount) {
				//was: askamount = inputs;
				std::cerr << "CreateSwap(): insufficient tokens for ask" << std::endl;
				CCerror = strprintf("insufficient tokens for ask");
				return ("");
			}
			////////////////////////// NOT IMPLEMENTED YET/////////////////////////////////
			CPubKey unspendablePubkey = GetUnspendable(cp, 0);
            mtx.vout.push_back(MakeCC1vout(EVAL_ASSETS, askamount, unspendablePubkey));

            if (inputs > askamount)
                CCchange = (inputs - askamount);
            if (CCchange != 0)
                mtx.vout.push_back(MakeCC1vout(EVAL_ASSETS, CCchange, mypk));

			////////////////////////// NOT IMPLEMENTED YET/////////////////////////////////
			std::vector<CPubKey> voutTokenPubkeys;  // should be empty - no token vouts

			if (assetid2 == zeroid) {
				opret = EncodeTokenOpRet(assetid, voutTokenPubkeys,
							EncodeAssetOpRet('s', zeroid, pricetotal, Mypubkey()));
			}
            else    {
                opret = EncodeTokenOpRet(assetid, voutTokenPubkeys,
							EncodeAssetOpRet('e', assetid2, pricetotal, Mypubkey()));
            } 
			////////////////////////// NOT IMPLEMENTED YET/////////////////////////////////
            return(FinalizeCCTx(FINALIZECCTX_NO_CHANGE_WHEN_DUST,cp,mtx,mypk,txfee,opret));
        } 
		else {
			fprintf(stderr, "need some assets to place ask\n");
		} */
    }
	else { // dimxy added 'else', because it was misleading message before
		fprintf(stderr,"need some native coins to place ask\n");
	}
    
    return("");
}  ////////////////////////// NOT IMPLEMENTED YET/////////////////////////////////

// unlocks coins, ends bid order
template<class T, class A>
UniValue CancelBuyOffer(const CPubKey &mypk, int64_t txfee,uint256 assetid,uint256 bidtxid)
{
    CMutableTransaction mtx = CreateNewContextualCMutableTransaction(Params().GetConsensus(), komodo_nextheight());
    CTransaction vintx;
	uint256 hashBlock; 
	struct CCcontract_info *cpAssets, C;

    cpAssets = CCinit(&C, A::EvalCode());

    if (txfee == 0)
        txfee = 10000;

    // add normal inputs only from my mypk (not from any pk in the wallet) to validate the ownership of the canceller
    if (AddNormalinputsRemote(mtx, mypk, txfee /*+ ASSETS_MARKER_AMOUNT*/, 0x10000) > 0)
    {
        uint256 spendingtxid;
        int32_t spendingvin, h;

        uint64_t mask = 0LL;
        LOCK(cs_main);
        if ((CCgetspenttxid(spendingtxid, spendingvin, h, bidtxid, ASSETS_GLOBALADDR_VOUT) != 0 || !IsTxidInActiveChain(spendingtxid)) && 
            myGetTransaction(bidtxid, vintx, hashBlock) && vintx.vout.size() > ASSETS_GLOBALADDR_VOUT)
        {
            uint8_t dummyEvalCode; uint256 dummyAssetid, dummyAssetid2; CAmount dummyPrice; std::vector<uint8_t> dummyOrigpubkey;

            CAmount bidamount = vintx.vout[ASSETS_GLOBALADDR_VOUT].nValue;
            if (bidamount == 0) {
                CCerror = "bid is empty";
                return "";
            }
            mtx.vin.push_back(CTxIn(bidtxid, ASSETS_GLOBALADDR_VOUT, CScript()));		// coins in Assets

            uint8_t funcid = A::DecodeAssetTokenOpRet(vintx.vout.back().scriptPubKey, dummyEvalCode, dummyAssetid, dummyAssetid2, dummyPrice, dummyOrigpubkey);
            if (funcid == 'b' && vintx.vout.size() > 1)
                mtx.vin.push_back(CTxIn(bidtxid, 1, CScript()));		// spend marker if funcid='b'
            else if (funcid == 'B' && vintx.vout.size() > 3)
                mtx.vin.push_back(CTxIn(bidtxid, 3, CScript()));		// spend marker if funcid='B'
            else {
                CCerror = "invalid bidtx or not enough vouts";
                return "";
            }

            if (bidamount > ASSETS_NORMAL_DUST)  
                mtx.vout.push_back(CTxOut(bidamount, CScript() << ParseHex(HexStr(mypk)) << OP_CHECKSIG));
            else {
                // send dust back to global addr
                mtx.vout.push_back(T::MakeCC1vout(A::EvalCode(), bidamount, GetUnspendable(cpAssets, NULL)));
                std::cerr << __func__ << " dust detected bidamount=" << bidamount << std::endl;
            }

            // mtx.vout.push_back(CTxOut(ASSETS_MARKER_AMOUNT, CScript() << ParseHex(HexStr(mypk)) << OP_CHECKSIG));  // we dont need a marker for cancelled orders

            UniValue sigData = T::FinalizeCCTx(IsRemoteRPCCall(), FINALIZECCTX_NO_CHANGE_WHEN_DUST, cpAssets, mtx, mypk, txfee,
                T::EncodeTokenOpRet(assetid, {},
                    { A::EncodeAssetOpRet('o', zeroid, 0, vuint8_t(mypk.begin(), mypk.end())) }));
            if (!ResultHasTx(sigData))
                return MakeResultError("Could not finalize tx");
            return sigData;
        }
        else
            CCerror = "could not load bid tx";
    }
    else
        CCerror = "could not get normal coins for txfee";
    return("");
}

//unlocks tokens, ends ask order
template<class T, class A>
UniValue CancelSell(const CPubKey &mypk, int64_t txfee, uint256 assetid, uint256 asktxid)
{
    CMutableTransaction mtx = CreateNewContextualCMutableTransaction(Params().GetConsensus(), komodo_nextheight());
    CTransaction vintx; uint64_t mask; 
	uint256 hashBlock; 	int64_t askamount; 
    struct CCcontract_info *cpTokens, *cpAssets, tokensC, assetsC;

    cpAssets = CCinit(&assetsC, A::EvalCode());

    if (txfee == 0)
        txfee = 10000;

    // add normal inputs only from my mypk (not from any pk in the wallet) to validate the ownership
    if (AddNormalinputsRemote(mtx, mypk, txfee /* + ASSETS_MARKER_AMOUNT*/, 0x10000) > 0)
    {
        uint256 spendingtxid;
        int32_t spendingvin, h;

        mask = ~((1LL << mtx.vin.size()) - 1);
        LOCK(cs_main);
        if ((CCgetspenttxid(spendingtxid, spendingvin, h, asktxid, ASSETS_GLOBALADDR_VOUT) != 0 || !IsTxidInActiveChain(spendingtxid)) && myGetTransaction(asktxid, vintx, hashBlock) != 0 && vintx.vout.size() > 0)
        {
            uint8_t dummyEvalCode; 
            uint256 dummyAssetid, dummyAssetid2; 
            int64_t dummyPrice; 
            std::vector<uint8_t> dummyOrigpubkey;

            askamount = vintx.vout[ASSETS_GLOBALADDR_VOUT].nValue;
            if (askamount == 0) {
                CCerror = "ask is empty";
                return "";
            }
            mtx.vin.push_back(CTxIn(asktxid, ASSETS_GLOBALADDR_VOUT, CScript()));
            
            uint8_t funcid = A::DecodeAssetTokenOpRet(vintx.vout.back().scriptPubKey, dummyEvalCode, dummyAssetid, dummyAssetid2, dummyPrice, dummyOrigpubkey);
            if (funcid == 's' && vintx.vout.size() > 1)
                mtx.vin.push_back(CTxIn(asktxid, 1, CScript()));		// spend marker if funcid='s'
            else if (funcid == 'S' && vintx.vout.size() > 3)
                mtx.vin.push_back(CTxIn(asktxid, 3, CScript()));		// spend marker if funcid='S'
            else {
                CCerror = "invalid ask tx or not enough vouts";
                return "";
            }

            TokenDataTuple tokenData;
            vuint8_t vextraData;
            GetTokenData<T>(NULL, assetid, tokenData, vextraData);
            if (vextraData.size() > 0)
                cpAssets->evalcodeAdd = vextraData.begin()[0];

            mtx.vout.push_back(T::MakeTokensCC1vout(cpAssets->evalcodeAdd ? cpAssets->evalcodeAdd : T::EvalCode(), askamount, mypk));	// one-eval token vout
            // mtx.vout.push_back(CTxOut(ASSETS_MARKER_AMOUNT, CScript() << ParseHex(HexStr(mypk)) << OP_CHECKSIG));  // we dont need marker for cancelled orders

            // init assets 'unspendable' privkey and pubkey
            uint8_t unspendableAssetsPrivkey[32];
            CPubKey unspendableAssetsPk = GetUnspendable(cpAssets, unspendableAssetsPrivkey);

            // add additional eval-tokens unspendable assets privkey:
            CCwrapper wrCond(T::MakeTokensCCcond1(A::EvalCode(), cpAssets->evalcodeAdd, unspendableAssetsPk));
            CCAddVintxCond(cpAssets, wrCond, unspendableAssetsPrivkey);

            UniValue sigData = T::FinalizeCCTx(IsRemoteRPCCall(), FINALIZECCTX_NO_CHANGE_WHEN_DUST, cpAssets, mtx, mypk, txfee,
                T::EncodeTokenOpRet(assetid, { mypk },
                    { A::EncodeAssetOpRet('x', zeroid, 0, vuint8_t(mypk.begin(), mypk.end())) } ));
            if (!ResultHasTx(sigData))
                return MakeResultError("Could not finalize tx");
            return sigData;
        }
        else
            CCerror = "could not get ask tx";
    }
    else
        CCerror = "could not get normal coins for txfee";
    return("");
}

//send tokens, receive coins:
template<class T, class A>
UniValue FillBuyOffer(const CPubKey &mypk, int64_t txfee, uint256 assetid, uint256 bidtxid, int64_t fill_units, CAmount paid_unit_price)
{
    CMutableTransaction mtx = CreateNewContextualCMutableTransaction(Params().GetConsensus(), komodo_nextheight());
    CTransaction vintx; 
	uint256 hashBlock; 
	std::vector<uint8_t> origpubkey; 
	const int32_t bidvout = ASSETS_GLOBALADDR_VOUT; 
	uint64_t mask = 0LL; 
	CAmount orig_units, unit_price, bid_amount, paid_amount, remaining_units, inputs;
	struct CCcontract_info *cpTokens, tokensC;
	struct CCcontract_info *cpAssets, assetsC;

    if (fill_units < 0)    {
        CCerror = "negative fill units";
        return("");
    }
    cpTokens = CCinit(&tokensC, T::EvalCode());
    
    TokenDataTuple tokenData;
    vuint8_t vextraData;
    uint8_t evalcodeAdd = 0;
    int64_t royaltyFract = 0;  // royaltyFract is N in N/1000 fraction
    GetTokenData<T>(NULL, assetid, tokenData, vextraData);
    if (vextraData.size() > 0)  {
        evalcodeAdd = vextraData.begin()[0];
        GetTokelDataAsInt64(vextraData, TKLPROP_ROYALTY, royaltyFract);
        if (royaltyFract > TKLROYALTY_DIVISOR-1)
            royaltyFract = TKLROYALTY_DIVISOR-1; // royalty upper limit
    }
    vuint8_t ownerpubkey = std::get<0>(tokenData);

	if (txfee == 0)
        txfee = 10000;

    if (AddNormalinputs(mtx, mypk, txfee+ASSETS_MARKER_AMOUNT, 0x10000, IsRemoteRPCCall()) > 0)
    {
        uint256 spendingtxid;
        int32_t spendingvin, h;

        LOCK(cs_main);
        if ((CCgetspenttxid(spendingtxid, spendingvin, h, bidtxid, bidvout) != 0 || !IsTxidInActiveChain(spendingtxid)) && myGetTransaction(bidtxid, vintx, hashBlock) != 0 && vintx.vout.size() > bidvout)
        {
            uint256 assetidOpret;

            bid_amount = vintx.vout[bidvout].nValue;
            uint8_t funcid = GetOrderParams<A>(origpubkey, unit_price, assetidOpret, vintx);  // get orig pk, orig units
            if (funcid != 'b' && funcid != 'B')  {
                CCerror = "not an bid order";
                return "";
            }
            if (assetid != assetidOpret)  {
                CCerror = "invalid tokenid";
                return "";
            }
            orig_units = bid_amount / unit_price;
            if (paid_unit_price == 0)
                paid_unit_price = unit_price;
            mtx.vin.push_back(CTxIn(bidtxid, bidvout, CScript()));					// Coins on Assets unspendable

            if ((inputs = AddTokenCCInputs<T>(cpTokens, mtx, mypk, assetid, fill_units, 0x1000, false)) > 0)
            {
                if (inputs < fill_units) {
                    CCerror = strprintf("insufficient tokens to fill buy offer");
                    return ("");
                }

                if (!SetBidFillamounts(unit_price, paid_amount, bid_amount, fill_units, orig_units, paid_unit_price)) {
                    CCerror = "incorrect units or price";
                    return ("");
                }
                CAmount royaltyValue = royaltyFract > 0 ? paid_amount / TKLROYALTY_DIVISOR * royaltyFract : 0;
                // check for dust:
                if (royaltyValue <= ASSETS_NORMAL_DUST)
                    royaltyValue = 0;
                CAmount tokensChange = inputs - fill_units;

                uint8_t unspendableAssetsPrivkey[32];
                cpAssets = CCinit(&assetsC, A::EvalCode());
                CPubKey unspendableAssetsPk = GetUnspendable(cpAssets, unspendableAssetsPrivkey);

                if (orig_units - fill_units > 0 || bid_amount - paid_amount <= ASSETS_NORMAL_DUST) { // bidder has coins for more tokens or only dust is sent back to global address
                    mtx.vout.push_back(T::MakeCC1vout(A::EvalCode(), bid_amount - paid_amount, unspendableAssetsPk));     // vout0 coins remainder or the dust is sent back to cc global addr
                    if (bid_amount - paid_amount <= ASSETS_NORMAL_DUST)
                        std::cerr << __func__ << " dust detected (bid_amount - paid_amount)=" << (bid_amount - paid_amount) << std::endl;
                }
                else
                    mtx.vout.push_back(CTxOut(bid_amount - paid_amount, CScript() << ParseHex(HexStr(origpubkey)) << OP_CHECKSIG));     // vout0 if no more tokens to buy, send the remainder to originator
                mtx.vout.push_back(CTxOut(paid_amount - royaltyValue, CScript() << ParseHex(HexStr(mypk)) << OP_CHECKSIG));	// vout1 coins to mypk normal 
                if (royaltyValue > 0)   { // note it makes vout even if roaltyValue is 0
                    mtx.vout.push_back(CTxOut(royaltyValue, CScript() << ParseHex(HexStr(ownerpubkey)) << OP_CHECKSIG));  // vout2 trade royalty to token owner
                    std::cerr << __func__ << " royaltyFract=" << royaltyFract << " royaltyValue=" << royaltyValue << " paid_amount - royaltyValue=" << paid_amount - royaltyValue << std::endl;
                }
                mtx.vout.push_back(T::MakeTokensCC1vout(evalcodeAdd ? evalcodeAdd : T::EvalCode(), fill_units, pubkey2pk(origpubkey)));	  // vout2(3) single-eval tokens sent to the originator
                if (orig_units - fill_units > 0)  // order is not finished yet
                    mtx.vout.push_back(T::MakeCC1vout(A::EvalCode(), ASSETS_MARKER_AMOUNT, origpubkey));                    // vout3(4 if royalty) marker to origpubkey

                if (tokensChange != 0LL)
                    mtx.vout.push_back(T::MakeTokensCC1vout(evalcodeAdd ? evalcodeAdd : T::EvalCode(), tokensChange, mypk));  // change in single-eval tokens
                
                CCwrapper wrCond1(MakeCCcond1(A::EvalCode(), unspendableAssetsPk));  // spend coins
                CCAddVintxCond(cpTokens, wrCond1, unspendableAssetsPrivkey);
                
                CCwrapper wrCond2(T::MakeTokensCCcond1(evalcodeAdd, mypk));  // spend my tokens to fill buy
                CCAddVintxCond(cpTokens, wrCond2, NULL); //NULL indicates to use myprivkey

                UniValue sigData = T::FinalizeCCTx(IsRemoteRPCCall(), FINALIZECCTX_NO_CHANGE_WHEN_DUST, cpTokens, mtx, mypk, txfee,
                    T::EncodeTokenOpRet(assetid, { pubkey2pk(origpubkey) },
                        { A::EncodeAssetOpRet('B', zeroid, unit_price, origpubkey) }));
                if (!ResultHasTx(sigData))
                    return MakeResultError("Could not finalize tx");
                return sigData;
            }
            else {
                CCerror = "dont have any assets to fill bid";
                return "";
            }
        }
        else {
            CCerror = "can't load or bad bidtx";
            return "";
        }
    }
    CCerror = "no normal coins left";
    return "";
}

// send coins, receive tokens 
template<class T, class A>
UniValue FillSell(const CPubKey &mypk, int64_t txfee, uint256 assetid, uint256 assetid2, uint256 asktxid, int64_t fillunits, CAmount paid_unit_price)
{
    CMutableTransaction mtx = CreateNewContextualCMutableTransaction(Params().GetConsensus(), komodo_nextheight());
    CTransaction vintx; 
	uint256 hashBlock; 
	std::vector<uint8_t> origpubkey; 
	uint64_t mask = 0; 
	const int32_t askvout = ASSETS_GLOBALADDR_VOUT; 
	CAmount unit_price, orig_assetoshis, paid_nValue; 
	struct CCcontract_info *cpAssets, assetsC;

    if (fillunits < 0)
    {
        CCerror = strprintf("negative fillunits %lld\n",(long long)fillunits);
        return("");
    }
    if (assetid2 != zeroid)
    {
        CCerror = "asset swaps disabled";
        return("");
    }

    TokenDataTuple tokenData;
    vuint8_t vextraData;
    uint8_t evalcodeAdd = 0;
    int64_t royaltyFract = 0;  // royaltyFract is N in N/1000 fraction
    GetTokenData<T>(NULL, assetid, tokenData, vextraData);
    if (vextraData.size() > 0)  {
        evalcodeAdd = vextraData.begin()[0];
        GetTokelDataAsInt64(vextraData, TKLPROP_ROYALTY, royaltyFract);
        if (royaltyFract > TKLROYALTY_DIVISOR-1)
            royaltyFract = TKLROYALTY_DIVISOR-1; // royalty upper limit
    }
    vuint8_t ownerpubkey = std::get<0>(tokenData);

    cpAssets = CCinit(&assetsC, A::EvalCode());

    if (txfee == 0)
        txfee = 10000;
    uint256 spendingtxid;
    int32_t spendingvin, h;

    LOCK(cs_main);
    if ((CCgetspenttxid(spendingtxid, spendingvin, h, asktxid, askvout) != 0 || !IsTxidInActiveChain(spendingtxid)) && myGetTransaction(asktxid, vintx, hashBlock) && vintx.vout.size() > askvout)
    {
        uint256 assetidOpret;
        orig_assetoshis = vintx.vout[askvout].nValue;
        uint8_t funcid = GetOrderParams<A>(origpubkey, unit_price, assetidOpret, vintx); // get orig pk, orig value
        if (funcid != 's' && funcid != 'S')  {
            CCerror = "not an ask order";
            return "";
        }
        if (assetid != assetidOpret)  {
            CCerror = "invalid tokenid";
            return "";
        }
        if (paid_unit_price <= 0LL)
            paid_unit_price = unit_price;
        if (paid_unit_price <= 0LL)    {
            CCerror = "could not get unit price";
            return "";
        }
        paid_nValue = paid_unit_price * fillunits;

        CAmount royaltyValue = royaltyFract > 0 ? paid_nValue / TKLROYALTY_DIVISOR * royaltyFract : 0;
        // check for dust:
        //if (royaltyValue <= ASSETS_NORMAL_DUST)
        //    royaltyValue = 0;
        // more accurate check matching to AssetValidate's check
        if (royaltyFract > 0 && paid_nValue - royaltyValue <= ASSETS_NORMAL_DUST / royaltyFract * TKLROYALTY_DIVISOR - ASSETS_NORMAL_DUST)  // if value paid to seller less than when the royalty is minimum
            royaltyValue = 0LL;

        if (assetid2 != zeroid) {
            // inputs = AddAssetInputs(cpAssets, mtx, mypk, assetid2, paid_nValue, 60);  // not implemented yet
            CCerror = "swaps not implemented";
            return "";            
        }

        // Use only one AddNormalinputs() in each rpc call to allow payment if user has only single utxo with normal funds
        CAmount inputs = AddNormalinputs(mtx, mypk, txfee + ASSETS_MARKER_AMOUNT + paid_nValue, 0x10000, IsRemoteRPCCall());  
        if (inputs > 0)
        {
			if (inputs < paid_nValue) {
				CCerror = strprintf("insufficient coins to fill sell");
				return ("");
			}

            // cc vin should be after normal vin
            mtx.vin.push_back(CTxIn(asktxid, askvout, CScript()));
            
            if (!SetAskFillamounts(unit_price, fillunits, orig_assetoshis, paid_nValue)) {
                CCerror = "incorrect units or price";
                return "";
            }
    
            if (paid_nValue == 0) {
                CCerror = "ask totally filled";
                return "";
            }

            // vout.0 tokens remainder to unspendable cc addr:
            mtx.vout.push_back(T::MakeTokensCC1vout(A::EvalCode(), evalcodeAdd, orig_assetoshis - fillunits, GetUnspendable(cpAssets, NULL)));  // token remainder on cc global addr

            //vout.1 purchased tokens to self token single-eval or dual-eval token+nonfungible cc addr:
            mtx.vout.push_back(T::MakeTokensCC1vout(evalcodeAdd ? evalcodeAdd : T::EvalCode(), fillunits, mypk));					
            mtx.vout.push_back(CTxOut(paid_nValue - royaltyValue, CScript() << origpubkey << OP_CHECKSIG));		//vout.2 coins to ask originator's normal addr
            if (royaltyValue > 0)    {   // note it makes the vout even if roaltyValue is 0
                mtx.vout.push_back(CTxOut(royaltyValue, CScript() << ownerpubkey << OP_CHECKSIG));	// vout.3 royalty to token owner
                std::cerr << __func__ << " royaltyFract=" << royaltyFract << " royaltyValue=" << royaltyValue << " paid_nValue - royaltyValue=" << paid_nValue - royaltyValue << std::endl;
            }
        
            if (orig_assetoshis - fillunits > 0) // we dont need the marker if order is filled
                mtx.vout.push_back(T::MakeCC1vout(A::EvalCode(), ASSETS_MARKER_AMOUNT, origpubkey));    //vout.3(4 if royalty) marker to origpubkey (for my tokenorders?)

			// init assets 'unspendable' privkey and pubkey
			uint8_t unspendableAssetsPrivkey[32];
			CPubKey unspendableAssetsPk = GetUnspendable(cpAssets, unspendableAssetsPrivkey);

            CCwrapper wrCond(T::MakeTokensCCcond1(A::EvalCode(), evalcodeAdd, unspendableAssetsPk));
            CCAddVintxCond(cpAssets, wrCond, unspendableAssetsPrivkey);

            //cpAssets->evalcodeAdd = evalcodeAdd;  // set token eval for signing

            UniValue sigData = T::FinalizeCCTx(IsRemoteRPCCall(), FINALIZECCTX_NO_CHANGE_WHEN_DUST, cpAssets, mtx, mypk, txfee,
				T::EncodeTokenOpRet(assetid, { mypk }, 
                    { A::EncodeAssetOpRet('S', assetid2, unit_price, origpubkey) } ));
            if (!ResultHasTx(sigData))
                return MakeResultError("Could not finalize tx");
            return sigData;
        } else {
            CCerror = "filltx not enough normal utxos";
            return "";
        }
    }
    CCerror = "can't get ask tx";
    return("");
}

#endif // #ifndef CC_ASSETS_TX_IMPL_H
