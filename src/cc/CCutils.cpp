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

/*
 CCutils has low level functions that are universally useful for all contracts.
 */

#include "CCinclude.h"
#include "CCtokens.h"
#include "komodo_structs.h"
#include "komodo_defs.h"
#include "key_io.h"


thread_local CCERROR CCerror = "";

#ifdef TESTMODE           
    #define MIN_NON_NOTARIZED_CONFIRMS 2
#else
    #define MIN_NON_NOTARIZED_CONFIRMS 101
#endif // TESTMODE
int32_t komodo_dpowconfs(int32_t height,int32_t numconfs);
struct komodo_state *komodo_stateptr(char *symbol,char *dest);
extern uint32_t KOMODO_DPOWCONFS;

void endiancpy(uint8_t *dest,uint8_t *src,int32_t len)
{
    int32_t i,j=0;
#if defined(WORDS_BIGENDIAN)
    for (i=31; i>=0; i--)
        dest[j++] = src[i];
#else
    memcpy(dest,src,len);
#endif
}

CC *MakeCCcond1of2(uint8_t evalcode,CPubKey pk1,CPubKey pk2)
{
    std::vector<CC*> pks;
    pks.push_back(CCNewSecp256k1(pk1));
    pks.push_back(CCNewSecp256k1(pk2));
    CC *condCC = CCNewEval(E_MARSHAL(ss << evalcode));
    CC *Sig = CCNewThreshold(1, pks);
    return CCNewThreshold(2, {condCC, Sig});
}

CC *MakeCCcond1(uint8_t evalcode,CPubKey pk)
{
    std::vector<CC*> pks;
    pks.push_back(CCNewSecp256k1(pk));
    CC *condCC = CCNewEval(E_MARSHAL(ss << evalcode));
    CC *Sig = CCNewThreshold(1, pks);
    return CCNewThreshold(2, {condCC, Sig});
}

int32_t has_opret(const CTransaction &tx, uint8_t evalcode)
{
    int i = 0;
    for ( auto vout : tx.vout )
    {
        //fprintf(stderr, "[txid.%s] 1.%i 2.%i 3.%i 4.%i\n",tx.GetHash().GetHex().c_str(),  vout.scriptPubKey[0], vout.scriptPubKey[1], vout.scriptPubKey[2], vout.scriptPubKey[3]);
        if ( vout.scriptPubKey.size() > 3 && vout.scriptPubKey[0] == OP_RETURN && vout.scriptPubKey[2] == evalcode )
            return i;
        i++;
    }
    return 0;
}

bool getCCopret(const CScript &scriptPubKey, CScript &opret)
{
    std::vector<std::vector<unsigned char>> vParams = std::vector<std::vector<unsigned char>>();
    CScript dummy; bool ret = false;
    if ( scriptPubKey.IsPayToCryptoCondition(&dummy, vParams) != 0 )
    {
        ret = true;
        if ( vParams.size() == 1)
        {
            opret = CScript(vParams[0].begin()+6, vParams[0].end());
            //fprintf(stderr, "vparams.%s\n", HexStr(vParams[0].begin(), vParams[0].end()).c_str());
        }
    }
    return ret;
}

bool makeCCopret(CScript &opret, std::vector<std::vector<unsigned char>> &vData)
{
    if ( opret.empty() )
        return false;
    vData.push_back(std::vector<unsigned char>(opret.begin(), opret.end()));
    return true;
}

CTxOut MakeCC1vout(uint8_t evalcode, CAmount nValue, CPubKey pk, std::vector<std::vector<unsigned char>>* vData)
{
    CTxOut vout;
    CCwrapper payoutCond(MakeCCcond1(evalcode, pk));
    vout = CTxOut(nValue, CCPubKey(payoutCond.get()));
    if (vData)
    {
        //std::vector<std::vector<unsigned char>> vtmpData = std::vector<std::vector<unsigned char>>(vData->begin(), vData->end());
        std::vector<CPubKey> vPubKeys = std::vector<CPubKey>();
        //vPubKeys.push_back(pk);   // Warning: if add a pubkey here, the Solver function will add it to vSolutions and ExtractDestination might use it to get the spk address (such result might not be expected)
        COptCCParams ccp = COptCCParams(COptCCParams::VERSION_1, evalcode, 1, 1, vPubKeys, (*vData));
        vout.scriptPubKey << ccp.AsVector() << OP_DROP;
    }
    return(vout);
}

CTxOut MakeCC1of2vout(uint8_t evalcode, CAmount nValue, CPubKey pk1, CPubKey pk2, std::vector<std::vector<unsigned char>>* vData)
{
    CTxOut vout;
    CCwrapper payoutCond(MakeCCcond1of2(evalcode, pk1, pk2));
    vout = CTxOut(nValue, CCPubKey(payoutCond.get()));
    if (vData)
    {
        //std::vector<std::vector<unsigned char>> vtmpData = std::vector<std::vector<unsigned char>>(vData->begin(), vData->end());
        std::vector<CPubKey> vPubKeys = std::vector<CPubKey>();
        // skip pubkeys. These need to maybe be optional and we need some way to get them out that is easy!
        // this is for multisig
        //vPubKeys.push_back(pk1);  // Warning: if add a pubkey here, the Solver function will add it to vSolutions and ExtractDestination might use it to get the spk address (such result might not be expected)
        //vPubKeys.push_back(pk2);
        COptCCParams ccp = COptCCParams(COptCCParams::VERSION_1, evalcode, 1, 2, vPubKeys, (*vData));
        vout.scriptPubKey << ccp.AsVector() << OP_DROP;
    }
    return(vout);
}
/*
bool CCtoAnon(const CC *cond)
{
    for (int i=0; i<cond->size;i++)
            if (cc_typeId(cond->subconditions[i])==CC_Threshold)
            {
                cond->subconditions[i]=cc_anon(cond->subconditions[i]);
                return (true);
            }
    return (false);
}
*/
CTxOut MakeCC1voutMixed(uint8_t evalcode,CAmount nValue, CPubKey pk, std::vector<unsigned char> *vData)
{
    CTxOut vout;
    CCwrapper payoutCond(MakeCCcond1(evalcode,pk));
    if (!CCtoAnon(payoutCond.get())) return (vout);
    vout = CTxOut(nValue,CCPubKey(payoutCond.get(),true));
    if ( vData )
    {
        vout.scriptPubKey << *vData << OP_DROP;
    }
    return(vout);
}

CTxOut MakeCC1of2voutMixed(uint8_t evalcode,CAmount nValue,CPubKey pk1,CPubKey pk2, std::vector<unsigned char> *vData)
{
    CTxOut vout;
    CCwrapper payoutCond(MakeCCcond1of2(evalcode,pk1,pk2));
    if (!CCtoAnon(payoutCond.get())) return (vout);
    vout = CTxOut(nValue,CCPubKey(payoutCond.get(),true));
    if ( vData )
    {
        vout.scriptPubKey << *vData << OP_DROP;
    }
    return(vout);
}

CC* GetCryptoCondition(CScript const& scriptSig)
{
    auto pc = scriptSig.begin();
    opcodetype opcode;
    std::vector<unsigned char> ffbin;
    if (scriptSig.GetOp(pc, opcode, ffbin))
        if (ffbin.data() != NULL)  // could return NULL if called for coinbase
            return cc_readFulfillmentBinary((uint8_t*)ffbin.data(), ffbin.size()-1);
    
    return(NULL);
}

bool IsCCInput(CScript const& scriptSig)
{
    CCwrapper cond(GetCryptoCondition(scriptSig));
    if ( cond.get() == 0 )
        return false;
    return true;
}

bool CheckTxFee(const CTransaction &tx, uint64_t txfee, uint32_t height, uint64_t blocktime, int64_t &actualtxfee)
{
    LOCK(mempool.cs);
    CCoinsView dummy;
    CCoinsViewCache view(&dummy);
    int64_t interest; uint64_t valuein;
    CCoinsViewMemPool viewMemPool(pcoinsTip, mempool);
    view.SetBackend(viewMemPool);
    valuein = view.GetValueIn(height,&interest,tx,blocktime);
    actualtxfee = valuein-tx.GetValueOut();
    if ( actualtxfee > txfee )
    {
        //fprintf(stderr, "actualtxfee.%li vs txfee.%li\n", actualtxfee, txfee);
        return false;
    }
    return true;
}

uint32_t GetLatestTimestamp(int32_t height)
{
    if ( KOMODO_NSPV_SUPERLITE ) return ((uint32_t)NSPV_blocktime(height));
    return(komodo_heightstamp(height));
} // :P

void CCaddr2set(struct CCcontract_info *cp,uint8_t evalcode,CPubKey pk,uint8_t *priv,char *coinaddr)
{
    cp->unspendableEvalcode2 = evalcode;
    cp->unspendablepk2 = pk;
    memcpy(cp->unspendablepriv2,priv,32);
    strcpy(cp->unspendableaddr2,coinaddr);
}

void CCaddr3set(struct CCcontract_info *cp,uint8_t evalcode,CPubKey pk,uint8_t *priv,char *coinaddr)
{
    cp->unspendableEvalcode3 = evalcode;
    cp->unspendablepk3 = pk;
    memcpy(cp->unspendablepriv3,priv,32);
    strcpy(cp->unspendableaddr3,coinaddr);
}

void CCaddr1of2set(struct CCcontract_info *cp, CPubKey pk1, CPubKey pk2, uint8_t *priv, char *coinaddr)
{
	cp->coins1of2pk[0] = pk1;
	cp->coins1of2pk[1] = pk2;
    memcpy(cp->coins1of2priv,priv,32);
    strcpy(cp->coins1of2addr,coinaddr);
}

void CCaddrTokens1of2set(struct CCcontract_info *cp, CPubKey pk1, CPubKey pk2, uint8_t *priv, char *tokenaddr)
{
	cp->tokens1of2pk[0] = pk1;
	cp->tokens1of2pk[1] = pk2;
    memcpy(cp->tokens1of2priv,priv,32);
	strcpy(cp->tokens1of2addr, tokenaddr);
}

bool Getscriptaddress(char *destaddr,const CScript &scriptPubKey)
{
    CTxDestination address; txnouttype whichType;
    destaddr[0] = 0;
    if ( scriptPubKey.begin() != 0 )
    {
        if ( ExtractDestination(scriptPubKey,address) != 0 )
        {
            strcpy(destaddr,(char *)CBitcoinAddress(address).ToString().c_str());
            return(true);
        }
    }
    //fprintf(stderr,"ExtractDestination failed\n");
    return(false);
}

bool GetCustomscriptaddress(char *destaddr,const CScript &scriptPubKey,uint8_t taddr,uint8_t prefix, uint8_t prefix2)
{
    CTxDestination address; txnouttype whichType;
    if ( ExtractDestination(scriptPubKey,address) != 0 )
    {
        strcpy(destaddr,(char *)CCustomBitcoinAddress(address,taddr,prefix,prefix2).ToString().c_str());
        return(true);
    }
    //fprintf(stderr,"ExtractDestination failed\n");
    return(false);
}

bool GetCCParams(Eval* eval, const CTransaction &tx, uint32_t nIn,
                 CTransaction &txOut, std::vector<std::vector<unsigned char>> &preConditions, std::vector<std::vector<unsigned char>> &params)
{
    uint256 blockHash;

    if (eval->GetTxUnconfirmed(tx.vin[nIn].prevout.hash, txOut, blockHash) && txOut.vout.size() > tx.vin[nIn].prevout.n)
    {
        CBlockIndex index;
        if (eval->GetBlock(blockHash, index))
        {
            // read preconditions
            CScript subScript = CScript();
            preConditions.clear();
            if (txOut.vout[tx.vin[nIn].prevout.n].scriptPubKey.IsPayToCryptoCondition(&subScript, preConditions))
            {
                // read any available parameters in the output transaction
                params.clear();
                if (tx.vout.size() > 0 && tx.vout[tx.vout.size() - 1].scriptPubKey.IsOpReturn())
                {
                    if (tx.vout[tx.vout.size() - 1].scriptPubKey.GetOpretData(params) && params.size() == 1)
                    {
                        CScript scr = CScript(params[0].begin(), params[0].end());

                        // printf("Script decoding inner:\n%s\nouter:\n%s\n", scr.ToString().c_str(), tx.vout[tx.vout.size() - 1].scriptPubKey.ToString().c_str());

                        if (!scr.GetPushedData(scr.begin(), params))
                        {
                            return false;
                        }
                        else return true;
                    }
                    else return false;
                }
                else return true;
            }
        }
    }
    return false;
    //fprintf(stderr,"ExtractDestination failed\n");
    return(false);
}

bool pubkey2addr(char *destaddr,uint8_t *pubkey33)
{
    std::vector<uint8_t>pk; int32_t i;
    for (i=0; i<33; i++)
        pk.push_back(pubkey33[i]);
    return(Getscriptaddress(destaddr,CScript() << pk << OP_CHECKSIG));
}

CPubKey CCtxidaddr(char *txidaddr,uint256 txid)
{
    uint8_t buf33[33]; CPubKey pk;
    buf33[0] = 0x02;
    endiancpy(&buf33[1],(uint8_t *)&txid,32);
    pk = buf2pk(buf33);
    if (txidaddr != NULL)
        Getscriptaddress(txidaddr,CScript() << ParseHex(HexStr(pk)) << OP_CHECKSIG);
    return(pk);
}

// CCtxidaddr version that makes valid pubkey by tweaking it
CPubKey CCtxidaddr_tweak(char *txidaddr, uint256 txid)
{
    uint8_t buf33[33]; 
    CPubKey pk;
    
    buf33[0] = 0x02;
    endiancpy(&buf33[1], (uint8_t *)&txid, 32);

    // tweak last byte
    // NOTE: this algorithm should not be changed, it should remain compatible with existing in chains txid-pubkeys
    int maxtweaks = 256;
    while (maxtweaks--) {
        pk = buf2pk(buf33);
        if (pk.IsFullyValid())
            break;
        buf33[sizeof(buf33)-1]++;
    }

    if (pk.IsFullyValid()) {
        if (txidaddr != NULL)
            Getscriptaddress(txidaddr, CScript() << ParseHex(HexStr(pk)) << OP_CHECKSIG);
        return(pk);
    }
    else    {
        if (txidaddr != NULL)
            strcpy(txidaddr, "");
        return CPubKey();
    }
}

CPubKey CCCustomtxidaddr(char *txidaddr,uint256 txid,uint8_t taddr,uint8_t prefix,uint8_t prefix2)
{
    uint8_t buf33[33]; CPubKey pk;
    buf33[0] = 0x02;
    endiancpy(&buf33[1],(uint8_t *)&txid,32);
    int maxtweaks = 256;
    while (maxtweaks--) {
        pk = buf2pk(buf33);
        if (pk.IsFullyValid())
            break;
        buf33[sizeof(buf33)-1]++;
    }

    if (pk.IsFullyValid()) {
        if (txidaddr != NULL)
            GetCustomscriptaddress(txidaddr, CScript() << ParseHex(HexStr(pk)) << OP_CHECKSIG,taddr,prefix,prefix2);
        return(pk);
    }
    else    {
        if (txidaddr != NULL)
            strcpy(txidaddr, "");
        return CPubKey();
    }
    return(pk);
}

bool _GetCCaddress(char *destaddr,uint8_t evalcode,CPubKey pk,bool mixed)
{
    CCwrapper payoutCond(MakeCCcond1(evalcode,pk));
    destaddr[0] = 0;
    if (payoutCond.get() != 0 )
    {
        if (mixed) CCtoAnon(payoutCond.get());
        Getscriptaddress(destaddr,CCPubKey(payoutCond.get(),mixed));
    }
    return(destaddr[0] != 0);
}

bool GetCCaddress(struct CCcontract_info *cp,char *destaddr,CPubKey pk,bool mixed)
{
    destaddr[0] = 0;
    if ( pk.size() == 0 )
        pk = GetUnspendable(cp,0);
    return(_GetCCaddress(destaddr,cp->evalcode,pk,mixed));
}

static bool _GetTokensCCaddress(char *destaddr, uint8_t evalcode1, uint8_t evalcode2, CPubKey pk, bool mixed)
{
	CCwrapper payoutCond;
    if (!mixed)
        payoutCond.reset(MakeTokensCCcond1(evalcode1, evalcode2, pk));
    else
        payoutCond.reset(MakeTokensv2CCcond1(evalcode1, evalcode2, pk));

	destaddr[0] = 0;
	if (payoutCond != nullptr)
	{
        if (mixed) 
            CCtoAnon(payoutCond.get());
		Getscriptaddress(destaddr, CCPubKey(payoutCond.get(), mixed));
	}
	return(destaddr[0] != 0);
}

// get scriptPubKey adddress for three/dual eval token cc vout
bool GetTokensCCaddress(struct CCcontract_info *cp, char *destaddr, CPubKey pk, bool mixed)
{
	destaddr[0] = 0;
	if (pk.size() == 0)
		pk = GetUnspendable(cp, 0);
	return(_GetTokensCCaddress(destaddr, cp->evalcode, cp->evalcodeAdd, pk, mixed));
}

bool GetCCaddress1of2(struct CCcontract_info *cp,char *destaddr,CPubKey pk,CPubKey pk2, bool mixed)
{
    CCwrapper payoutCond(MakeCCcond1of2(cp->evalcode,pk,pk2));
    destaddr[0] = 0;
    if ( payoutCond.get() != 0 )
    {
        if (mixed) CCtoAnon(payoutCond.get());
        Getscriptaddress(destaddr,CCPubKey(payoutCond.get(),mixed));
    }
    return(destaddr[0] != 0);
}

bool GetTokensCCaddress1of2(struct CCcontract_info *cp, char *destaddr, CPubKey pk1, CPubKey pk2, bool mixed)
{
	CCwrapper payoutCond;
    if (!mixed)
        payoutCond.reset(MakeTokensCCcond1of2(cp->evalcode, cp->evalcodeAdd, pk1, pk2));
    else
        payoutCond.reset(MakeTokensv2CCcond1of2(cp->evalcode, cp->evalcodeAdd, pk1, pk2));

	destaddr[0] = 0;
	if (payoutCond != nullptr)  //  if evalcodeAdd not set then it is dual-eval cc else three-eval cc
	{
        if (mixed) 
            CCtoAnon(payoutCond.get());
		Getscriptaddress(destaddr, CCPubKey(payoutCond.get(), mixed));
	}
	return(destaddr[0] != 0);
}

// validate cc or normal vout address and value
bool ConstrainVout(CTxOut vout, int32_t CCflag, char *cmpaddr, int64_t nValue)
{
    char destaddr[64];
    if ( vout.scriptPubKey.IsPayToCryptoCondition() != CCflag )
    {
        //fprintf(stderr,"constrain vout error isCC %d vs %d CCflag\n", vout.scriptPubKey.IsPayToCryptoCondition(), CCflag);
        return(false);
    }
    else if ( cmpaddr != 0 && (Getscriptaddress(destaddr, vout.scriptPubKey) == 0 || strcmp(destaddr, cmpaddr) != 0) )
    {
        //fprintf(stderr,"constrain vout error: check addr %s vs script addr %s\n", cmpaddr!=0?cmpaddr:"", destaddr!=0?destaddr:"");
        return(false);
    }
    else if ( nValue != 0 && nValue != vout.nValue ) //(nValue == 0 && vout.nValue < 10000) || (
    {
        //fprintf(stderr,"constrain vout error nValue %.8f vs %.8f\n",(double)nValue/COIN,(double)vout.nValue/COIN);
        return(false);
    }
    else return(true);
}

// validate cc or normal vout address and value
bool ConstrainVoutV2(CTxOut vout, int32_t CCflag, char *cmpaddr, int64_t nValue, uint8_t evalCode)
{
    if (ConstrainVout(vout, CCflag, cmpaddr, nValue))
        if (CCflag && evalCode != 0)
            return vout.scriptPubKey.SpkHasEvalcodeCCV2(evalCode);
        else
            return true;
    else
        return false;
}

bool PreventCC(Eval* eval,const CTransaction &tx,int32_t preventCCvins,int32_t numvins,int32_t preventCCvouts,int32_t numvouts)
{
    int32_t i;
    if ( preventCCvins >= 0 )
    {
        for (i=preventCCvins; i<numvins; i++)
        {
            if ( IsCCInput(tx.vin[i].scriptSig) != 0 )
                return eval->Invalid("invalid CC vin");
        }
    }
    if ( preventCCvouts >= 0 )
    {
        for (i=preventCCvouts; i<numvouts; i++)
        {
            if ( tx.vout[i].scriptPubKey.IsPayToCryptoCondition() != 0 )
            {
                fprintf(stderr,"vout.%d is CC\n",i);
                return eval->Invalid("invalid CC vout");
            }
        }
    }
    return(true);
}

bool priv2addr(char *coinaddr,uint8_t *buf33,uint8_t priv32[32])
{
    CKey priv; CPubKey pk; int32_t i; uint8_t *src;
    priv.SetKey32(priv32);
    pk = priv.GetPubKey();
    if ( buf33 != 0 )
    {
        src = (uint8_t *)pk.begin();
        for (i=0; i<33; i++)
            buf33[i] = src[i];
    }
    return(Getscriptaddress(coinaddr, CScript() << ParseHex(HexStr(pk)) << OP_CHECKSIG));
}

std::vector<uint8_t> Mypubkey()
{
    extern uint8_t NOTARY_PUBKEY33[33];
    std::vector<uint8_t> pubkey; int32_t i; uint8_t *dest,*pubkey33;
    pubkey33 = NOTARY_PUBKEY33;
    pubkey.resize(33);
    dest = pubkey.data();
    for (i=0; i<33; i++)
        dest[i] = pubkey33[i];
    return(pubkey);
}

extern char NSPV_wifstr[],NSPV_pubkeystr[];
extern uint32_t NSPV_logintime;
#define NSPV_AUTOLOGOUT 777

bool Myprivkey(uint8_t myprivkey[])
{
    char coinaddr[64],checkaddr[64]; std::string strAddress; char *dest; int32_t i,n; CBitcoinAddress address; CKeyID keyID; CKey vchSecret; uint8_t buf33[33];
    if ( KOMODO_NSPV_SUPERLITE )
    {
        if ( NSPV_logintime != 0 && time(NULL) <= NSPV_logintime+NSPV_AUTOLOGOUT )
        {
            vchSecret = DecodeSecret(NSPV_wifstr);
            memcpy(myprivkey,vchSecret.begin(),32);
            //for (i=0; i<32; i++)
            //    fprintf(stderr,"%02x",myprivkey[i]);
            //fprintf(stderr," myprivkey %s\n",NSPV_wifstr);
            memset((uint8_t *)vchSecret.begin(),0,32);
            return true;
        }
        else if ( KOMODO_DEX_P2P == 0 )
        {
            fprintf(stderr,"need to be logged in to get myprivkey\n");
            return false;
        }
    }
    if ( pwalletMain != 0 && Getscriptaddress(coinaddr,CScript() << Mypubkey() << OP_CHECKSIG) != 0 )
    {
        n = (int32_t)strlen(coinaddr);
        strAddress.resize(n+1);
        dest = (char *)strAddress.data();
        for (i=0; i<n; i++)
            dest[i] = coinaddr[i];
        dest[i] = 0;
        if ( address.SetString(strAddress) != 0 && address.GetKeyID(keyID) != 0 )
        {
#ifdef ENABLE_WALLET
            if ( pwalletMain->GetKey(keyID,vchSecret) != 0 )
            {
                memcpy(myprivkey,vchSecret.begin(),32);
                memset((uint8_t *)vchSecret.begin(),0,32);
                if ( 0 )
                {
                    for (i=0; i<32; i++)
                        fprintf(stderr,"0x%02x, ",myprivkey[i]);
                    fprintf(stderr," found privkey for %s!\n",dest);
                }
                if ( priv2addr(checkaddr,buf33,myprivkey) != 0 )
                {
                    if ( buf2pk(buf33) == Mypubkey() && strcmp(checkaddr,coinaddr) == 0 )
                        return(true);
                    else printf("mismatched privkey -> addr %s vs %s\n",checkaddr,coinaddr);
                }
                return(false);
            } else fprintf(stderr,"(%p) cant find (%s) privkey\n",pwalletMain,coinaddr);
#endif
        } else fprintf(stderr,"cant find (%s) in wallet\n",coinaddr);
    }
    if ( KOMODO_DEX_P2P != 0 )
    {
        static int32_t onetimeflag; static uint8_t sessionpriv[32];
        if ( onetimeflag == 0 )
        {
            OS_randombytes(sessionpriv,32);
            fprintf(stderr,"privkey for pubkey not found -> generate session specific privkey\n");
            onetimeflag = 1;
        }
        memcpy(myprivkey,sessionpriv,32);
        return(true);
    }
    fprintf(stderr,"privkey for the -pubkey= address is not in the wallet, importprivkey!\n");
    return(false);
}

CPubKey GetUnspendable(struct CCcontract_info *cp,uint8_t *unspendablepriv)
{
    if ( unspendablepriv != 0 )
        memcpy(unspendablepriv,cp->CCpriv,32);
    return(pubkey2pk(ParseHex(cp->CChexstr)));
}

void CCclearvars(struct CCcontract_info *cp)
{
    cp->unspendableEvalcode2 = cp->unspendableEvalcode3 = 0;
    cp->unspendableaddr2[0] = cp->unspendableaddr3[0] = 0;
}

int64_t CCduration(int32_t &numblocks,uint256 txid)
{
    CTransaction tx; uint256 hashBlock; uint32_t txheight,txtime=0; char str[65]; CBlockIndex *pindex; int64_t duration = 0;
    numblocks = 0;
    if ( myGetTransaction(txid,tx,hashBlock) == 0 )
    {
        //fprintf(stderr,"CCduration cant find duration txid %s\n",uint256_str(str,txid));
        return(0);
    }
    else if ( hashBlock == zeroid )
    {
        //fprintf(stderr,"CCduration no hashBlock for txid %s\n",uint256_str(str,txid));
        return(0);
    }
    else if ( (pindex= komodo_getblockindex(hashBlock)) == 0 || (txtime= pindex->nTime) == 0 || (txheight= pindex->GetHeight()) <= 0 )
    {
        fprintf(stderr,"CCduration no txtime %u or txheight.%d %p for txid %s\n",txtime,txheight,pindex,uint256_str(str,txid));
        return(0);
    }
    else if ( (pindex= chainActive.LastTip()) == 0 || pindex->nTime < txtime || pindex->GetHeight() <= txheight )
    {
        if ( pindex->nTime < txtime )
            fprintf(stderr,"CCduration backwards timestamps %u %u for txid %s hts.(%d %d)\n",(uint32_t)pindex->nTime,txtime,uint256_str(str,txid),txheight,(int32_t)pindex->GetHeight());
        return(0);
    }
    numblocks = (pindex->GetHeight() - txheight);
    duration = (pindex->nTime - txtime);
    //fprintf(stderr,"duration %d (%u - %u) numblocks %d (%d - %d)\n",(int32_t)duration,(uint32_t)pindex->nTime,txtime,numblocks,pindex->GetHeight(),txheight);
    return(duration);
}

uint256 CCOraclesReverseScan(char const *logcategory,uint256 &txid,int32_t height,uint256 reforacletxid,uint256 batontxid)
{
    CTransaction tx; uint256 hash,mhash,bhash,hashBlock,oracletxid; int32_t len,len2,numvouts;
    int64_t val,merkleht; CPubKey pk; std::vector<uint8_t>data; char str[65],str2[65];
    
    txid = zeroid;
    LogPrint(logcategory,"start reverse scan %s\n",uint256_str(str,batontxid));
    while ( myGetTransaction(batontxid,tx,hashBlock) != 0 && (numvouts= tx.vout.size()) > 0 )
    {
        LogPrint(logcategory,"check %s\n",uint256_str(str,batontxid));
        if ( DecodeOraclesData(tx.vout[numvouts-1].scriptPubKey,oracletxid,bhash,pk,data) == 'D' && oracletxid == reforacletxid )
        {
            LogPrint(logcategory,"decoded %s\n",uint256_str(str,batontxid));
            if ( oracle_format(&hash,&merkleht,0,'I',(uint8_t *)data.data(),0,(int32_t)data.size()) == sizeof(int32_t) && merkleht == height )
            {
                len = oracle_format(&hash,&val,0,'h',(uint8_t *)data.data(),sizeof(int32_t),(int32_t)data.size());
                len2 = oracle_format(&mhash,&val,0,'h',(uint8_t *)data.data(),(int32_t)(sizeof(int32_t)+sizeof(uint256)),(int32_t)data.size());

                LogPrint(logcategory,"found merkleht.%d len.%d len2.%d %s %s\n",(int32_t)merkleht,len,len2,uint256_str(str,hash),uint256_str(str2,mhash));
                if ( len == sizeof(hash)+sizeof(int32_t) && len2 == 2*sizeof(mhash)+sizeof(int32_t) && mhash != zeroid )
                {
                    txid = batontxid;
                    LogPrint(logcategory,"set txid\n");
                    return(mhash);
                }
                else
                {
                    LogPrint(logcategory,"missing hash\n");
                    return(zeroid);
                }
            }
            else LogPrint(logcategory,"height.%d vs search ht.%d\n",(int32_t)merkleht,(int32_t)height);
            batontxid = bhash;
            LogPrint(logcategory,"new hash %s\n",uint256_str(str,batontxid));
        } else break;
    }
    LogPrint(logcategory,"end of loop\n");
    return(zeroid);
}

int64_t CCOraclesGetDepositBalance(char const *logcategory,uint256 reforacletxid,uint256 batontxid)
{
    CTransaction tx; uint256 hash,prevbatontxid,hashBlock,oracletxid; int32_t len,len2,numvouts;
    int64_t val,balance=0; CPubKey pk; std::vector<uint8_t>data;

    if ( myGetTransaction(batontxid,tx,hashBlock) != 0 && (numvouts= tx.vout.size()) > 0 )
    {
        if ( DecodeOraclesData(tx.vout[numvouts-1].scriptPubKey,oracletxid,prevbatontxid,pk,data) == 'D' && oracletxid == reforacletxid )
        {
            if ( oracle_format(&hash,&balance,0,'L',(uint8_t *)data.data(),(int32_t)(sizeof(int32_t)+sizeof(uint256)*2),(int32_t)data.size()) == (int32_t)(sizeof(int32_t)+sizeof(uint256)*2+sizeof(int64_t)))
            {
                return (balance);
            }
        }
    }
    return (0);
}


int32_t NSPV_coinaddr_inmempool(char const *logcategory,char *coinaddr,uint8_t CCflag);

int32_t myIs_coinaddr_inmempoolvout(char const *logcategory,uint256 txid,char *coinaddr)
{
    int32_t i,n; char destaddr[64];
    if ( KOMODO_NSPV_SUPERLITE )
        return(NSPV_coinaddr_inmempool(logcategory,coinaddr,0));
    BOOST_FOREACH(const CTxMemPoolEntry &e,mempool.mapTx)
    {
        const CTransaction &tx = e.GetTx();
        if ( (n= tx.vout.size()) > 0 )
        {
            if (txid == tx.GetHash()) continue;
            for (i=0; i<n; i++)
            {
                Getscriptaddress(destaddr,tx.vout[i].scriptPubKey);
                if ( strcmp(destaddr,coinaddr) == 0 )
                {
                    LogPrint(logcategory,"found (%s) vout in mempool\n",coinaddr);
                    return(1);
                }
            }
        }
    }
    return(0);
}

extern struct NSPV_mempoolresp NSPV_mempoolresult;
extern bool NSPV_evalcode_inmempool(uint8_t evalcode,uint8_t funcid);

int32_t myGet_mempool_txs(std::vector<CTransaction> &txs,uint8_t evalcode,uint8_t funcid)
{
    int i=0;

    if ( KOMODO_NSPV_SUPERLITE )
    {
        CTransaction tx; uint256 hashBlock;

        NSPV_evalcode_inmempool(evalcode,funcid);
        for (int i=0;i<NSPV_mempoolresult.numtxids;i++)
        {
            if (myGetTransaction(NSPV_mempoolresult.txids[i],tx,hashBlock)!=0) txs.push_back(tx);
        }
        return (NSPV_mempoolresult.numtxids);
    }
    LOCK(mempool.cs);
    BOOST_FOREACH(const CTxMemPoolEntry &e, mempool.mapTx)
    {
        txs.push_back(e.GetTx());
        i++;
    }
    return(i);
}

int32_t CCCointxidExists(char const* logcategory, uint256 txid, uint256 cointxid)
{
    char txidaddr[64];
    std::string coin;
    int32_t numvouts;
    uint256 hashBlock;

    std::vector<std::pair<CAddressIndexKey, CAmount>> addressIndex;
    CCtxidaddr_tweak(txidaddr, cointxid);
    SetAddressIndexOutputs(addressIndex, txidaddr, false);
    for (std::vector<std::pair<CAddressIndexKey, CAmount>>::const_iterator it = addressIndex.begin(); it != addressIndex.end(); it++) {
        return (-1);
    }
    return (myIs_coinaddr_inmempoolvout(logcategory, txid, txidaddr));
}

bool CompareHexVouts(std::string hex1, std::string hex2)
{
    CTransaction tx1,tx2;

    if (!DecodeHexTx(tx1,hex1)) return (false);
    if (!DecodeHexTx(tx2,hex2)) return (false);
    if (tx1.vout.size()!=tx2.vout.size()) return (false);
    for (int i=0;i<(int32_t)tx1.vout.size();i++) if (tx1.vout[i]!=tx2.vout[i]) return (false);
    return (true);
}

CPubKey CheckVinPk(struct CCcontract_info *cp,const CTransaction &tx, int32_t n, std::vector<CPubKey> &pubkeys)
{
    CTransaction vintx; uint256 blockHash; char destaddr[64],pkaddr[64];

    if(myGetTransaction(tx.vin[n].prevout.hash, vintx, blockHash)==0) return (CPubKey());
    if( tx.vin[n].prevout.n < vintx.vout.size() && Getscriptaddress(destaddr, vintx.vout[tx.vin[n].prevout.n].scriptPubKey) != 0 )
    {
        for(int i=0;i<(int32_t)pubkeys.size();i++)
        {
            pubkey2addr(pkaddr,(uint8_t *)pubkeys[i].begin());
            if (strcmp(pkaddr, destaddr) == 0) {
                return (pubkeys[i]);
            }
        }
    }    
    return (CPubKey());
}

/* Get the block merkle root for a proof
 * IN: proofData
 * OUT: merkle root
 * OUT: transaction IDS
 */
uint256 BitcoinGetProofMerkleRoot(const std::vector<uint8_t> &proofData, std::vector<uint256> &txids)
{
    CMerkleBlock merkleBlock;
    if (!E_UNMARSHAL(proofData, ss >> merkleBlock))
        return uint256();
    return merkleBlock.txn.ExtractMatches(txids);
}

int64_t komodo_get_blocktime(uint256 hashBlock)
{
    BlockMap::iterator mi = mapBlockIndex.find(hashBlock);
    if (mi != mapBlockIndex.end() && (*mi).second)
    {
        CBlockIndex* pindex = (*mi).second;
        if (chainActive.Contains(pindex))
            return pindex->GetBlockTime();
    }
    return 0;
}

extern struct NSPV_inforesp NSPV_inforesult;
int32_t komodo_get_current_height()
{
    if ( KOMODO_NSPV_SUPERLITE )
    {
        return (NSPV_inforesult.height);
    }
    else return chainActive.LastTip()->GetHeight();
}

bool komodo_txnotarizedconfirmed(uint256 txid, int32_t minconfirms)
{
    char str[65];
    int32_t confirms,minimumconfirms,notarized=0,txheight=0,currentheight=0;;
    CTransaction tx;
    uint256 hashBlock;
    CBlockIndex *pindex;    
    char symbol[KOMODO_ASSETCHAIN_MAXLEN],dest[KOMODO_ASSETCHAIN_MAXLEN]; struct komodo_state *sp;

    if (minconfirms==0) return (true);
    if ( KOMODO_NSPV_SUPERLITE )
    {
        if ( NSPV_myGetTransaction(txid,tx,hashBlock,txheight,currentheight) == 0 )
        {
            fprintf(stderr,"komodo_txnotarizedconfirmed cant find txid %s\n",txid.ToString().c_str());
            return(0);
        }
        else if (txheight<=0)
        {
            fprintf(stderr,"komodo_txnotarizedconfirmed no txheight.%d for txid %s\n",txheight,txid.ToString().c_str());
            return(0);
        }
        else if (txheight>currentheight)
        {
            fprintf(stderr,"komodo_txnotarizedconfirmed backwards heights for txid %s hts.(%d %d)\n",txid.ToString().c_str(),txheight,currentheight);
            return(0);
        }
        confirms=1 + currentheight - txheight;
    }
    else
    {
        if ( myGetTransaction(txid,tx,hashBlock) == 0 )
        {
            fprintf(stderr,"komodo_txnotarizedconfirmed cant find txid %s\n",txid.ToString().c_str());
            return(0);
        }
        else if ( hashBlock == zeroid )
        {
            fprintf(stderr,"komodo_txnotarizedconfirmed no hashBlock for txid %s\n",txid.ToString().c_str());
            return(0);
        }
        else if ( (pindex= komodo_blockindex(hashBlock)) == 0 || (txheight= pindex->GetHeight()) <= 0 )
        {
            fprintf(stderr,"komodo_txnotarizedconfirmed no txheight.%d %p for txid %s\n",txheight,pindex,txid.ToString().c_str());
            return(0);
        }
        else if ( (pindex= chainActive.LastTip()) == 0 || pindex->GetHeight() < txheight )
        {
            fprintf(stderr,"komodo_txnotarizedconfirmed backwards heights for txid %s hts.(%d %d)\n",txid.ToString().c_str(),txheight,(int32_t)pindex->GetHeight());
            return(0);
        }    
        confirms=1 + pindex->GetHeight() - txheight;
    }
    if (minconfirms>1) minimumconfirms=minconfirms;
    else minimumconfirms=MIN_NON_NOTARIZED_CONFIRMS;
    if ((sp= komodo_stateptr(symbol,dest)) != 0 && (notarized=sp->NOTARIZED_HEIGHT) > 0 && txheight > sp->NOTARIZED_HEIGHT)  notarized=0;            
#ifdef TESTMODE           
    notarized=0;
#endif //TESTMODE
    if (notarized>0 && confirms > 1)
        return (true);
    else if (notarized==0 && confirms >= minimumconfirms)
        return (true);
    return (false);
}

CPubKey check_signing_pubkey(CScript scriptSig)
{
	bool found = false;
	CPubKey pubkey;
	
    auto findEval = [](CC *cond, struct CCVisitor _) {
        bool r = false;

        if (cc_typeId(cond) == CC_Secp256k1) {
            *(CPubKey*)_.context=buf2pk(cond->publicKey);
            r = true;
        }
        // false for a match, true for continue
        return r ? 0 : 1;
    };

    CCwrapper cond(GetCryptoCondition(scriptSig));

    if (cond.get()) {
        CCVisitor visitor = { findEval, (uint8_t*)"", 0, &pubkey };
        bool out = !cc_visit(cond.get(), visitor);

        if (pubkey.IsValid()) {
            return pubkey;
        }
    }
	return CPubKey();
}


// returns total of normal inputs signed with this pubkey
CAmount TotalPubkeyNormalInputs(Eval *eval, const CTransaction &tx, const CPubKey &pubkey)
{
    CAmount total = 0;

    for (const auto &vin : tx.vin) {
        CTransaction vintx;
        uint256 hashBlock;
        if (!IsCCInput(vin.scriptSig) && GetTxUnconfirmedOpt(eval, vin.prevout.hash, vintx, hashBlock)) {
            typedef std::vector<unsigned char> valtype;
            std::vector<valtype> vSolutions;
            txnouttype whichType;

            if (Solver(vintx.vout[vin.prevout.n].scriptPubKey, whichType, vSolutions)) {
                switch (whichType) {
                case TX_PUBKEY:
                    if (pubkey == CPubKey(vSolutions[0]))   // is my input?
                        total += vintx.vout[vin.prevout.n].nValue;
                    break;
                case TX_PUBKEYHASH:
                    if (pubkey.GetID() == CKeyID(uint160(vSolutions[0])))    // is my input?
                        total += vintx.vout[vin.prevout.n].nValue;
                    break;
                }
            }
        }
    }
    return total;
}

// returns total of CC inputs signed with this pubkey
CAmount TotalPubkeyCCInputs(Eval *eval, const CTransaction &tx, const CPubKey &pubkey)
{
    CAmount total = 0;
    for (const auto &vin : tx.vin) {
        if (IsCCInput(vin.scriptSig)) {
            CPubKey vinPubkey = check_signing_pubkey(vin.scriptSig);
            if (vinPubkey.IsValid()) {
                if (vinPubkey == pubkey) {
                    CTransaction vintx;
                    uint256 hashBlock;
                    if (GetTxUnconfirmedOpt(eval, vin.prevout.hash, vintx, hashBlock)) {
                        total += vintx.vout[vin.prevout.n].nValue;
                    }
                }
            }
        }
    }
    return total;
}

bool ProcessCC(struct CCcontract_info* cp, Eval* eval, std::vector<uint8_t> paramsNull, const CTransaction& ctx, unsigned int nIn, std::shared_ptr<CCheckCCEvalCodes> evalcodeChecker)
{
    CTransaction createTx;
    uint256 assetid, assetid2, hashBlock;
    uint8_t funcid;
    int32_t height, i, n, from_mempool = 0;
    int64_t amount;
    std::vector<uint8_t> origpubkey;
    height = KOMODO_CONNECTING;
    if (KOMODO_CONNECTING < 0) // always comes back with > 0 for final confirmation
        return (true);
    if (ASSETCHAINS_CC == 0 || (height & ~(1 << 30)) < KOMODO_CCACTIVATE)
        return eval->Invalid("CC are disabled or not active yet");
    if ((KOMODO_CONNECTING & (1 << 30)) != 0) {
        from_mempool = 1;
        height &= ((1 << 30) - 1);
    }
    if (cp->validate == NULL)
        return eval->Invalid("validation not supported for eval code");

    //fprintf(stderr,"KOMODO_CONNECTING.%d mempool.%d vs CCactive.%d\n",height,from_mempool,KOMODO_CCACTIVATE);
    // there is a chance CC tx is valid in mempool, but invalid when in block, so we cant filter duplicate requests. if any of the vins are spent, for example
    //txid = ctx.GetHash();
    //if ( txid == cp->prevtxid )
    //    return(true);
    //fprintf(stderr,"process CC %02x\n",cp->evalcode);
    CCclearvars(cp);
    if (paramsNull.size() != 0) // Don't expect params
        return eval->Invalid("Cannot have params");
    //else if ( ctx.vout.size() == 0 )      // spend can go to z-addresses
    //    return eval->Invalid("no-vouts");
    else if ((*cp->validate)(cp, eval, ctx, nIn) != 0) {
        //fprintf(stderr,"done CC %02x\n",cp->evalcode);
        //cp->prevtxid = txid;
        if (evalcodeChecker.get() != NULL)
            evalcodeChecker->MarkEvalCode(ctx.GetHash(), cp->evalcode);
        return (true);
    }
    //fprintf(stderr,"invalid CC %02x\n",cp->evalcode);
    return (false);
}

bool CClib_Dispatch(const CC* cond, Eval* eval, std::vector<uint8_t> paramsNull, const CTransaction& txTo, unsigned int nIn, std::shared_ptr<CCheckCCEvalCodes> evalcodeChecker)
{
    uint8_t evalcode;
    int32_t height, from_mempool;
    struct CCcontract_info* cp;
    if (ASSETCHAINS_CCLIB != MYCCLIBNAME) {
        fprintf(stderr, "-ac_cclib=%s vs myname %s\n", ASSETCHAINS_CCLIB.c_str(), MYCCLIBNAME.c_str());
        return eval->Invalid("-ac_cclib name mismatches myname");
    }
    height = KOMODO_CONNECTING;
    if (KOMODO_CONNECTING < 0) // always comes back with > 0 for final confirmation
        return (true);
    if (ASSETCHAINS_CC == 0 || (height & ~(1 << 30)) < KOMODO_CCACTIVATE)
        return eval->Invalid("CC are disabled or not active yet");
    if ((KOMODO_CONNECTING & (1 << 30)) != 0) {
        from_mempool = 1;
        height &= ((1 << 30) - 1);
    }
    evalcode = cond->code[0];
    if (evalcodeChecker.get() != NULL && evalcodeChecker->CheckEvalCode(txTo.GetHash(), evalcode) != 0)
        return true;
    if (evalcode >= EVAL_FIRSTUSER && evalcode <= EVAL_LASTUSER) {
        cp = &CCinfos[(int32_t)evalcode];
        if (cp->didinit == 0) {
            if (CClib_initcp(cp, evalcode) == 0)
                cp->didinit = 1;
            else
                return eval->Invalid("unsupported CClib evalcode");
        }
        CCclearvars(cp);
        if (paramsNull.size() != 0) // Don't expect params
            return eval->Invalid("Cannot have params");
        else if (CClib_validate(cp, height, eval, txTo, nIn) != 0) {
            if (evalcodeChecker.get() != NULL)
                evalcodeChecker->MarkEvalCode(txTo.GetHash(), evalcode);
            return (true);
        }
        return (false); //eval->Invalid("error in CClib_validate");
    }
    return eval->Invalid("cclib CC must have evalcode between 16 and 127");
}

//void OS_randombytes(unsigned char *x,long xlen);
//extern bits256 curve25519_basepoint9();

int32_t _SuperNET_cipher(uint8_t nonce[crypto_box_NONCEBYTES],uint8_t *cipher,uint8_t *message,int32_t len,bits256 destpub,bits256 srcpriv,uint8_t *buf)
{
    memset(cipher,0,len+crypto_box_ZEROBYTES);
    memset(buf,0,crypto_box_ZEROBYTES);
    memcpy(buf+crypto_box_ZEROBYTES,message,len);
    if ( crypto_box(cipher,buf,len+crypto_box_ZEROBYTES,nonce,destpub.bytes,srcpriv.bytes) != 0 )
        return(-1);
    return(len + crypto_box_ZEROBYTES);
}

uint8_t *_SuperNET_decipher(uint8_t nonce[crypto_box_NONCEBYTES],uint8_t *cipher,uint8_t *message,int32_t len,bits256 srcpub,bits256 mypriv)
{
    int32_t err;
    if ( (0) )
    {
        int32_t z;
        for (z=0; z<crypto_box_NONCEBYTES; z++)
            fprintf(stderr,"%02x",nonce[z]);
        fprintf(stderr," nonce, ");
        for (z=0; z<32; z++)
            fprintf(stderr,"%02x",mypriv.bytes[z]);
        fprintf(stderr," priv\n");
        for (z=0; z<32; z++)
            fprintf(stderr,"%02x",srcpub.bytes[z]);
        fprintf(stderr," srcpub ");
        for (z=0; z<len; z++)
            fprintf(stderr,"%02x",cipher[z]);
        fprintf(stderr," cipher[%d]\n",len);
    }
    if ( (err= crypto_box_open(message,cipher,len,nonce,srcpub.bytes,mypriv.bytes)) == 0 )
    {
        message += crypto_box_ZEROBYTES;
        len -= crypto_box_ZEROBYTES;
        return(message);
    }
    return(0);
}

uint8_t *SuperNET_deciphercalc(uint8_t *senderpub,uint8_t **ptrp,int32_t *msglenp,bits256 privkey,uint8_t *cipher,int32_t cipherlen)
{
    bits256 srcpubkey; uint8_t *origptr,*nonce,*message; uint8_t *retptr = 0;
    message = (uint8_t *)calloc(1,cipherlen);
    *ptrp = message;
    origptr = cipher;
    memcpy(srcpubkey.bytes,cipher,sizeof(srcpubkey));
    memcpy(senderpub,srcpubkey.bytes,sizeof(srcpubkey));
    cipher += sizeof(srcpubkey);
    cipherlen -= sizeof(srcpubkey);
    nonce = cipher;
    cipher += crypto_box_NONCEBYTES, cipherlen -= crypto_box_NONCEBYTES;
    *msglenp = cipherlen - crypto_box_ZEROBYTES;
    if ( *msglenp <= 0 || (retptr= _SuperNET_decipher(nonce,cipher,message,cipherlen,srcpubkey,privkey)) == 0 )
    {
        *msglenp = -1;
        free(*ptrp);
        *ptrp = 0;
    }
    return(retptr);
}

uint8_t *SuperNET_ciphercalc(uint8_t **ptrp,int32_t *cipherlenp,bits256 privkey,bits256 destpubkey,uint8_t *data,int32_t datalen)
{
    bits256 mypubkey; uint8_t *buf,*nonce,*cipher,*origptr,space[1024]; int32_t allocsize;
    *ptrp = 0;
    allocsize = (datalen + crypto_box_NONCEBYTES + crypto_box_ZEROBYTES + sizeof(mypubkey));
    if ( allocsize > sizeof(space) )
        buf = (uint8_t *)calloc(1,allocsize);
    else
    {
        memset(space,0,sizeof(space));
        buf = space;
    }
    cipher = (uint8_t *)calloc(1,allocsize);
    *ptrp = cipher;
    origptr = nonce = cipher;
    mypubkey = curve25519(privkey,curve25519_basepoint9());
    memcpy(cipher,mypubkey.bytes,sizeof(mypubkey));
    nonce = &cipher[sizeof(mypubkey)];
    OS_randombytes(nonce,crypto_box_NONCEBYTES);
    cipher = &nonce[crypto_box_NONCEBYTES];
    _SuperNET_cipher(nonce,cipher,data,datalen,destpubkey,privkey,buf);
    if ( (0) )
    {
        int32_t z;
        uint8_t message[8192];
        for (z=0; z<32; z++)
            fprintf(stderr,"%02x",mypubkey.bytes[z]);
        fprintf(stderr," mypub\n");
        if ( _SuperNET_decipher(nonce,cipher,message,datalen+crypto_box_ZEROBYTES,destpubkey,privkey) != 0 )
        {
            for (z=0; z<datalen; z++)
                fprintf(stderr,"%02x",message[z+crypto_box_ZEROBYTES]);
            fprintf(stderr," deciphered.%d\n",z);
        } else fprintf(stderr,"decipher error\n");
    }
    if ( buf != space )
        free(buf);
    *cipherlenp = allocsize;
    return(origptr);
}

uint8_t *komodo_DEX_encrypt(uint8_t **allocatedp,uint8_t *data,int32_t *datalenp,bits256 destpubkey,bits256 privkey)
{
     int32_t cipherlen; uint8_t *cipher;
    cipher = SuperNET_ciphercalc(allocatedp,&cipherlen,privkey,destpubkey,data,*datalenp);
    *datalenp = cipherlen;
    return(cipher);
}

uint8_t *komodo_DEX_decrypt(uint8_t *senderpub,uint8_t **allocatedp,uint8_t *data,int32_t *datalenp,bits256 privkey)
{
    int32_t msglen;
    *allocatedp = 0;
    if ( (msglen= *datalenp) <= crypto_box_NONCEBYTES + crypto_box_ZEROBYTES + sizeof(bits256) )
    {
        *datalenp = 0;
        return(0);
    }
    if ( (data= SuperNET_deciphercalc(senderpub,allocatedp,&msglen,privkey,data,*datalenp)) == 0 )
    {
        //printf("komodo_DEX_decrypt decrytion error\n");
        *datalenp = 0;
        return(0);
    } else *datalenp = msglen;
    return(data);
}

void komodo_DEX_privkey(bits256 &privkey)
{
    bits256 priv,hash;
    Myprivkey(priv.bytes);
    vcalc_sha256(0,hash.bytes,priv.bytes,32);
    vcalc_sha256(0,privkey.bytes,hash.bytes,32);
    memset(priv.bytes,0,sizeof(priv));
    memset(hash.bytes,0,sizeof(hash));
}

void komodo_DEX_pubkey(bits256 &pubkey)
{
    bits256 privkey;
    komodo_DEX_privkey(privkey);
    /*{
        char *bits256_str(char hexstr[65],bits256 x);
        char str[65];
        fprintf(stderr,"new DEX_privkey %s\n",bits256_str(str,privkey));
    }*/
    pubkey = curve25519(privkey,curve25519_basepoint9());
    memset(privkey.bytes,0,sizeof(privkey));
}

// add probe vintx conditions for making CCSig in FinalizeCCTx
void CCAddVintxCond(struct CCcontract_info *cp, const CCwrapper &condWrapped, const uint8_t *priv)
{
    if (cp == NULL) return;
    if (condWrapped.get() == NULL) return;

    struct CCVintxProbe ccprobe(condWrapped, priv);
    cp->CCvintxprobes.push_back(ccprobe);
}

/*std::ostream& operator<<(std::ostream& os, const CCwrapper& cc)
{
    os << cc_conditionToJSONString(cc.get());
    return os;
}*/

int32_t oracle_format(uint256 *hashp,int64_t *valp,char *str,uint8_t fmt,uint8_t *data,int32_t offset,int32_t datalen)
{
    char _str[65]; int32_t sflag = 0,i,val32,len = 0,slen = 0,dlen = 0; uint32_t uval32; uint16_t uval16; int16_t val16; int64_t val = 0; uint64_t uval = 0;
    *valp = 0;
    *hashp = zeroid;
    if ( str != 0 )
        str[0] = 0;
    switch ( fmt )
    {
        case 's': slen = data[offset++]; break;
        case 'S': slen = data[offset++]; slen |= ((int32_t)data[offset++] << 8); break;
        case 'd': dlen = data[offset++]; break;
        case 'D': dlen = data[offset++]; dlen |= ((int32_t)data[offset++] << 8); break;
        case 'c': len = 1; sflag = 1; break;
        case 'C': len = 1; break;
        case 't': len = 2; sflag = 1; break;
        case 'T': len = 2; break;
        case 'i': len = 4; sflag = 1; break;
        case 'I': len = 4; break;
        case 'l': len = 8; sflag = 1; break;
        case 'L': len = 8; break;
        case 'h': len = 32; break;
        default: return(-1); break;
    }
    if ( slen != 0 )
    {
        if ( str != 0 )
        {
            if ( slen < IGUANA_MAXSCRIPTSIZE && offset+slen <= datalen )
            {
                for (i=0; i<slen; i++)
                    str[i] = data[offset++];
                str[i] = 0;
            } else return(-1);
        }
    }
    else if ( dlen != 0 )
    {
        if ( str != 0 )
        {
            if ( dlen < IGUANA_MAXSCRIPTSIZE && offset+dlen <= datalen )
            {
                for (i=0; i<dlen; i++)
                    sprintf(&str[i<<1],"%02x",data[offset++]);
                str[i<<1] = 0;
            } else return(-1);
        }
    }
    else if ( len != 0 && len+offset <= datalen )
    {
        if ( len == 32 )
        {
            iguana_rwbignum(0,&data[offset],len,(uint8_t *)hashp);
            if ( str != 0 )
                sprintf(str,"%s",uint256_str(_str,*hashp));
        }
        else
        {
            if ( sflag != 0 )
            {
                switch ( len )
                {
                    case 1: val = (int8_t)data[offset]; break;
                    case 2: iguana_rwnum(0,&data[offset],len,(void *)&val16); val = val16; break;
                    case 4: iguana_rwnum(0,&data[offset],len,(void *)&val32); val = val32; break;
                    case 8: iguana_rwnum(0,&data[offset],len,(void *)&val); break;
                }
                if ( str != 0 )
                    sprintf(str,"%lld",(long long)val);
                *valp = val;
            }
            else
            {
                switch ( len )
                {
                    case 1: uval = data[offset]; break;
                    case 2: iguana_rwnum(0,&data[offset],len,(void *)&uval16); uval = uval16; break;
                    case 4: iguana_rwnum(0,&data[offset],len,(void *)&uval32); uval = uval32; break;
                    case 8: iguana_rwnum(0,&data[offset],len,(void *)&uval); break;
                }
                if ( str != 0 )
                    sprintf(str,"%llu",(long long)uval);
                *valp = (int64_t)uval;
            }
        }
        offset += len;
    } else return(-1);
    return(offset);
}

int32_t oracle_parse_data_format(std::vector<uint8_t> data,std::string format)
{
    int64_t offset=0,len=0; char fmt;

    for (int i=0; i<format.size();i++)
    {
        fmt=format[i];
        switch (fmt)
        {
            case 's': len = data[offset++]; break;
            case 'S': len = data[offset++]; len |= ((int32_t)data[offset++] << 8); break;
            case 'd': len = data[offset++]; break;
            case 'D': len = data[offset++]; len |= ((int32_t)data[offset++] << 8); break;
            case 'c': len = 1; break;
            case 'C': len = 1; break;
            case 't': len = 2; break;
            case 'T': len = 2; break;
            case 'i': len = 4; break;
            case 'I': len = 4; break;
            case 'l': len = 8; break;
            case 'L': len = 8; break;
            case 'h': len = 32; break;
            default: return(0); break;
        }
        if (len>data.size()-offset) return (0);
        if (fmt=='S' || fmt=='s')
        {
            for (int j=offset;j<offset+len;j++)
                if (data[j]<32 || data[j]>127) return (0);
        }
        offset+=len;
    }
    if (offset!=data.size()) return (0);
    else return (offset);
}

int64_t _correlate_price(int64_t *prices,int32_t n,int64_t price)
{
    int32_t i,count = 0; int64_t diff,threshold = (price >> 8);
    for (i=0; i<n; i++)
    {
        if ( (diff= (price - prices[i])) < 0 )
            diff = -diff;
        if ( diff <= threshold )
            count++;
    }
    if ( count < (n >> 1) )
        return(0);
    else return(price);
}

int64_t correlate_price(int32_t height,int64_t *prices,int32_t n)
{
    int32_t i,j; int64_t price = 0;
    if ( n == 1 )
        return(prices[0]);
    for (i=0; i<n; i++)
    {
        j = (height + i) % n;
        if ( prices[j] != 0 && (price= _correlate_price(prices,n,prices[j])) != 0 )
            break;
    }
    for (i=0; i<n; i++)
        fprintf(stderr,"%llu ",(long long)prices[i]);
    fprintf(stderr,"-> %llu ht.%d\n",(long long)price,height);
    return(price);
}

int64_t OracleCorrelatedPrice(int32_t height,std::vector <int64_t> origprices)
{
    std::vector <int64_t> sorted; int32_t i,n; int64_t *prices,price;
    if ( (n= origprices.size()) == 1 )
        return(origprices[0]);
    std::sort(origprices.begin(), origprices.end());
    prices = (int64_t *)calloc(n,sizeof(*prices));
    i = 0;
    for (std::vector<int64_t>::const_iterator it=sorted.begin(); it!=sorted.end(); it++)
        prices[i++] = *it;
    price = correlate_price(height,prices,i);
    free(prices);
    return(price);
}

int32_t oracleprice_add(std::vector<struct oracleprice_info> &publishers,CPubKey pk,int32_t height,std::vector <uint8_t> data,int32_t maxheight)
{
    struct oracleprice_info item; int32_t flag = 0;
    for (std::vector<struct oracleprice_info>::iterator it=publishers.begin(); it!=publishers.end(); it++)
    {
        if ( pk == it->pk )
        {
            flag = 1;
            if ( height > it->height )
            {
                it->height = height;
                it->data = data;
                return(height);
            }
        }
    }
    if ( flag == 0 )
    {
        item.pk = pk;
        item.data = data;
        item.height = height;
        publishers.push_back(item);
        return(height);
    } else return(0);
}

UniValue OracleFormat(uint8_t *data,int32_t datalen,char *format,int32_t formatlen)
{
    UniValue obj(UniValue::VARR); uint256 hash; int32_t i,j=0; int64_t val; char str[IGUANA_MAXSCRIPTSIZE*2+1];
    for (i=0; i<formatlen && j<datalen; i++)
    {
        str[0] = 0;
        j = oracle_format(&hash,&val,str,format[i],data,j,datalen);
        obj.push_back(str);
        if ( j < 0 )
            break;
        if ( j >= datalen )
            break;
    }
    return(obj);
}


// get OP_DROP data:
CScript GetCCDropAsOpret(const CScript &scriptPubKey)
{
    std::vector<std::vector<unsigned char>> vParams;
    CScript dummy; 
    CScript opret;

    if (scriptPubKey.IsPayToCryptoCondition(&dummy, vParams))
    {

        if (vParams.size() > 0)  {
            COptCCParams parsed(vParams[0]);

            LOGSTREAMFN("ccutils", CCLOG_DEBUG1, stream << " evalcode=" << (int)parsed.evalCode << " vKeys.size()=" << (int)parsed.vKeys.size() << " vData.size()=" << (int)parsed.vData.size() << std::endl);
            if (parsed.vData.size() > 0)      {
                opret << OP_RETURN << parsed.vData[0];  // return vData[0] as cc opret
                return opret;
            }
        }

        /* parse OP_DROP without verus header (such opdrops are not supported anymore):
        if (vParams.size() >= 1)  // allow more data after cc opret
        {
            //uint8_t version;
            //uint8_t evalCode;
            //uint8_t m, n;
            std::vector< vscript_t > vData;

            // parse vParams[0] as script
            // try read verus header <evalcode version M N>, do not allow pubkeys after the header
            CScript inScript(vParams[0].begin(), vParams[0].end());
            CScript::const_iterator pc = inScript.begin();
            inScript.GetPushedData(pc, vData);

            if (vData.size() > 1 && vData[0].size() == 4) // first vector is 4-byte verus header
            {
                // support Verus-style vData
                opret << OP_RETURN << vData[1];  // return vData[1] as cc opret
                return true;
            }
            else if (vParams.size() == 1)
            {
                // support token-v2-style vData:
                opret << OP_RETURN << vParams[0];  // no verus header, treat vParams[0] as cc data and return as opret
                return true;
            }
        } */
    }
    return CScript();
}

// get OP_DROP data for mixed cc vouts
// the function returns OP_DROP data as OP_RETURN script. This looks strange but all cc modules have DecodeOpReturn-like functions 
// that accept OP_RETURN scripts 
/*bool GetCCVDataAsOpret(const CScript &scriptPubKey, CScript &opret)
{
    std::vector<vscript_t> vParams;
    CScript dummy; 

    if (scriptPubKey.IsPayToCryptoCondition(&dummy, vParams) != 0)
    {
        if (vParams.size() >= 1)  // allow more data after cc opret
        {
            opret << OP_RETURN << vParams[0];  // return vData[1] as cc opret
            return true;
        }
    }
    return false;
}*/

// get cc address for pubkey or mypk
UniValue CCaddress(struct CCcontract_info *cp, const char *name, const std::vector<unsigned char> &pubkey, bool mixed)
{
    UniValue result(UniValue::VOBJ); 
    char destaddr[64],str[64]; 
    CPubKey mypk,globalpk;
    
    globalpk = GetUnspendable(cp, NULL);
    GetCCaddress(cp, destaddr, globalpk, mixed);
    if (strcmp(destaddr,cp->unspendableCCaddr) != 0)
    {
        uint8_t priv[32];
        Myprivkey(priv); // it is assumed the CC's normal address'es -pubkey was used
        fprintf(stderr,"fix mismatched CCaddr %s -> %s\n",cp->unspendableCCaddr,destaddr);
        strcpy(cp->unspendableCCaddr,destaddr);
        memset(priv,0,32);
    }
    result.push_back(Pair("result", "success"));
    sprintf(str,"GlobalPk %s CC Address",name);
    result.push_back(Pair(str,cp->unspendableCCaddr));
    sprintf(str,"GlobalPk %s CC Balance",name);
    result.push_back(Pair(str,ValueFromAmount(CCaddress_balance(cp->unspendableCCaddr,1))));
    sprintf(str,"GlobalPk %s Normal Address",name);
    result.push_back(Pair(str,cp->normaladdr));
    sprintf(str,"GlobalPk %s Normal Balance",name);
    result.push_back(Pair(str,ValueFromAmount(CCaddress_balance(cp->normaladdr,0))));
    if (strcmp(name,"Gateways")==0) result.push_back(Pair("GatewaysPubkey","03ea9c062b9652d8eff34879b504eda0717895d27597aaeb60347d65eed96ccb40"));
    
    else if (strcmp(name,"Tokens")!=0 && strcmp(name,"Tokensv2")!=0)
    {
        if (GetTokensCCaddress(cp, destaddr, globalpk, mixed)>0)
        {
            sprintf(str,"GlobalPk %s/Tokens CC Address",name);
            result.push_back(Pair(str,destaddr));
        }
    }
    if (pubkey.size() == 33)
    {
        if (GetCCaddress(cp,destaddr,pubkey2pk(pubkey), mixed) != 0)
        {
            sprintf(str,"pubkey %s CC Address",name);
            result.push_back(Pair(str,destaddr));
            sprintf(str,"pubkey %s CC Balance",name);
            result.push_back(Pair(str,ValueFromAmount(CCaddress_balance(destaddr,0))));
        }
    }
    if ((strcmp(name,"Channels")==0 || strcmp(name,"Heir")==0) && pubkey.size() == 33)
    {
        sprintf(str,"mypk/pubkey 1of2 %s CC Address",name);
        mypk = pubkey2pk(Mypubkey());
        GetCCaddress1of2(cp, destaddr, mypk, pubkey2pk(pubkey), mixed);
        result.push_back(Pair(str,destaddr));
        if (GetTokensCCaddress1of2(cp,destaddr,mypk,pubkey2pk(pubkey), mixed) > 0)
        {
            sprintf(str,"mypk/pubkey 1of2 %s/Tokens CC Address",name);
            result.push_back(Pair(str,destaddr));
        }
    }
    if (GetCCaddress(cp, destaddr, pubkey2pk(Mypubkey()), mixed) != 0)
    {
        sprintf(str,"mypk %s CC Address",name);
        result.push_back(Pair(str,destaddr));
        sprintf(str,"mypk %s CC Balance",name);
        result.push_back(Pair(str,ValueFromAmount(CCaddress_balance(destaddr,1))));
    }
    if ( Getscriptaddress(destaddr,(CScript() << Mypubkey() << OP_CHECKSIG)) != 0 )
    {
        result.push_back(Pair("mypk Normal Address",destaddr));
        result.push_back(Pair("mypk Normal Balance",ValueFromAmount(CCaddress_balance(destaddr,0))));
    }
    return(result);
}

// decode cc transaction:
// try to find cc data in vout's opdrop or in the last vout opreturn
// return funcid, version and creationid
bool CCDecodeTxVout(const CTransaction &tx, int32_t n, uint8_t &evalcode, uint8_t &funcid, uint8_t &version, uint256 &creationId)
{
    CScript opdrop;
    vscript_t vccdata;

    if (tx.vout.size() > 0)     
    {
        // note: assumes that this is a cc vout (does not check this)

        // first try if OP_DROP data exists
        bool usedOpreturn;
        CScript opdrop;
        if (!(opdrop = GetCCDropAsOpret(tx.vout[n].scriptPubKey)).empty()) {
            GetOpReturnData(opdrop, vccdata);
            usedOpreturn = false;
        }
        else   {
            GetOpReturnData(tx.vout.back().scriptPubKey, vccdata); 
            usedOpreturn = true;  // use OP_RETURN in the last vout if no OP_DROP data
        }

        // use following algotithm to determine creationId
        // get the evalcode from ccdata
        // if no cc vins found with this evalcode this is the creation tx and creationId = tx.GetHash()
        // else the creationId is after the version field: 'evalcode funcid version creationId'
        if (vccdata.size() >= 3)  {
            struct CCcontract_info *cp, C; 
            cp = CCinit(&C, vccdata[0]);
            int32_t i = 0;
            for (; i < tx.vin.size(); i ++)
                if (cp->ismyvin(tx.vin[i].scriptSig))
                    break;
            if (i == tx.vin.size()) 
            {
                creationId = tx.GetHash(); // tx is the creation tx
                evalcode = vccdata[0];
                funcid = vccdata[1];
                version = vccdata[2];
                LOGSTREAMFN("ccutils", CCLOG_DEBUG1, stream << " evalcode=" << (int)evalcode << " funcid=" << (char)funcid << "(" << (int)funcid << "), version=" << (int)version << std::endl); 
            }
            else
            {
                uint256 encodedCrid;
                if (vccdata.size() >= 3 + sizeof(uint256))   {  // get creationId from the ccdata
                    bool isEof = true;
                    if (!E_UNMARSHAL(vccdata, ss >> evalcode; ss >> funcid; ss >> version; ss >> encodedCrid; isEof = ss.eof()) && isEof) {  // E_UNMARSHAL might parse okay but return false if not EoF yet. So EoF==true means bad parse
                        LOGSTREAMFN("ccutils", CCLOG_DEBUG1, stream << "failed to decode ccdata, isEof=" << isEof << " usedOpreturn=" << usedOpreturn << " tx=" << HexStr(E_MARSHAL(ss << tx)) << std::endl);
                        return false;
                    }
                }
                creationId = revuint256(encodedCrid);
                LOGSTREAMFN("ccutils", CCLOG_DEBUG1, stream << " evalcode=" << (int)evalcode << " funcid=" << (char)funcid << "(" << (int)funcid << "), version=" << (int)version << " in opret found creationid=" << creationId.GetHex() << std::endl);
            }
        }
        return true;
    }
    return false;
}

bool IsBlockHashInActiveChain(uint256 hashBlock)
{
    AssertLockHeld(cs_main);
    BlockMap::iterator mi = mapBlockIndex.find(hashBlock);
    if (mi != mapBlockIndex.end() && (*mi).second) {
        CBlockIndex* pindex = (*mi).second;
        if (chainActive.Contains(pindex)) 
            return true;
    }
    return false;
}

/**
 * IsTxidInActiveChain load a tx and checks if it is in active chain (not in orphaned blocks)
 * cs_main section should be locked
 */
bool IsTxidInActiveChain(uint256 txid)
{
    CTransaction tx;
    uint256 hashBlock;

    if (myGetTransaction(txid, tx, hashBlock))
    {
        return IsBlockHashInActiveChain(hashBlock);
    }
    return false;
}
