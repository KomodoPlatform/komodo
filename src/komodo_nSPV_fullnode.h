
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

#ifndef KOMODO_NSPVFULLNODE_H
#define KOMODO_NSPVFULLNODE_H

// NSPV_get... functions need to return the exact serialized length, which is the size of the structure minus size of pointers, plus size of allocated data

#include "notarisationdb.h"
#include "rpc/server.h"

static std::map<std::string,bool> nspv_remote_commands =  {
    
{"channelsopen", false},{"channelspayment", false},{"channelsclose", false},{"channelsrefund", false},
{"channelslist", false},{"channelsinfo", false},{"oraclescreate", false},{"oraclesfund", false},{"oraclesregister", false},{"oraclessubscribe", false}, 
{"oraclesdata", false},{"oraclesinfo", false},{"oracleslist", false},{"gatewaysbind", false},{"gatewaysdeposit", false},{"gatewayswithdraw", false},
{"gatewayswithdrawsign", false},{"gatewaysmarkdone", false},{"gatewayspendingsignwithdraws", false},{"gatewayssignedwithdraws", false},
{"gatewaysinfo", false},{"gatewayslist", false},{"faucetfund", false},{"faucetget", false},{"pegscreate", false},{"pegsfund", false},{"pegsget", false},{"pegsclose", false},
{"pegsclose", false},{"pegsredeem", false},{"pegsexchange", false},{"pegsliquidate", false},{"pegsaccounthistory", false},{"pegsaccountinfo", false},{"pegsworstaccounts", false},
{"pegsinfo", false},
    // tokens:
    { "tokenask", false }, { "tokenbid", false }, { "tokenfillask", false }, { "tokenfillbid", false }, { "tokencancelask", false }, { "tokencancelbid", false }, 
    { "tokenorders", false }, { "mytokenorders", false }, { "tokentransfer", false },{ "tokencreate", false },
    { "tokenv2ask", true }, { "tokenv2bid", true }, { "tokenv2fillask", true }, { "tokenv2fillbid", true }, { "tokenv2cancelask", true }, { "tokenv2cancelbid", true }, 
    { "tokenv2orders", true }, { "mytokenv2orders", true }, { "tokenv2transfer", true },{ "tokenv2create", true }
};

struct NSPV_ntzargs
{
    uint256 txid,desttxid,blockhash;
    int32_t txidht,ntzheight;
};

int32_t NSPV_notarization_find(struct NSPV_ntzargs *args,int32_t height,int32_t dir)
{
    int32_t ntzheight = 0; uint256 hashBlock; CTransaction tx; Notarisation nota; char *symbol; std::vector<uint8_t> opret;
    symbol = (ASSETCHAINS_SYMBOL[0] == 0) ? (char *)"KMD" : ASSETCHAINS_SYMBOL;
    memset(args,0,sizeof(*args));
    if ( dir > 0 )
        height += 10;
    if ( (args->txidht= ScanNotarisationsDB(height,symbol,1440,nota)) == 0 )
        return(-1);
    args->txid = nota.first;
    if ( !GetTransaction(args->txid,tx,hashBlock,false) || tx.vout.size() < 2 )
        return(-2);
    GetOpReturnData(tx.vout[1].scriptPubKey,opret);
    if ( opret.size() >= 32*2+4 )
        args->desttxid = NSPV_opretextract(&args->ntzheight,&args->blockhash,symbol,opret,args->txid);
    return(args->ntzheight);
}

int32_t NSPV_notarized_bracket(struct NSPV_ntzargs *prev,struct NSPV_ntzargs *next,int32_t height)
{
    uint256 bhash; int32_t txidht,ntzht,nextht,i=0;
    memset(prev,0,sizeof(*prev));
    memset(next,0,sizeof(*next));
    if ( (ntzht= NSPV_notarization_find(prev,height,-1)) < 0 || ntzht > height || ntzht == 0 )
        return(-1);
    txidht = height+1;
    while ( (ntzht=  NSPV_notarization_find(next,txidht,1)) < height )
    {
        nextht = next->txidht + 10*i;
//fprintf(stderr,"found forward ntz, but ntzht.%d vs height.%d, txidht.%d -> nextht.%d\n",next->ntzheight,height,txidht,nextht);
        memset(next,0,sizeof(*next));
        txidht = nextht;
        if ( ntzht <= 0 )
            break;
        if ( i++ > 10 )
            break;
    }
    return(0);
}

int32_t NSPV_ntzextract(struct NSPV_ntz *ptr,uint256 ntztxid,int32_t txidht,uint256 desttxid,int32_t ntzheight)
{
    CBlockIndex *pindex;
    ptr->blockhash = *chainActive[ntzheight]->phashBlock;
    ptr->height = ntzheight;
    ptr->txidheight = txidht;
    ptr->othertxid = desttxid;
    ptr->txid = ntztxid;
    if ( (pindex= komodo_chainactive(ptr->txidheight)) != 0 )
        ptr->timestamp = pindex->nTime;
    return(0);
}

int32_t NSPV_getntzsresp(struct NSPV_ntzsresp *ptr,int32_t origreqheight)
{
    struct NSPV_ntzargs prev,next; int32_t reqheight = origreqheight;
    if ( reqheight < chainActive.LastTip()->GetHeight() )
        reqheight++;
    if ( NSPV_notarized_bracket(&prev,&next,reqheight) == 0 )
    {
        if ( prev.ntzheight != 0 )
        {
            ptr->reqheight = origreqheight;
            if ( NSPV_ntzextract(&ptr->prevntz,prev.txid,prev.txidht,prev.desttxid,prev.ntzheight) < 0 )
                return(-1);
        }
        if ( next.ntzheight != 0 )
        {
            if ( NSPV_ntzextract(&ptr->nextntz,next.txid,next.txidht,next.desttxid,next.ntzheight) < 0 )
                return(-1);
        }
    }
    return(sizeof(*ptr));
}

int32_t NSPV_setequihdr(struct NSPV_equihdr *hdr,int32_t height)
{
    CBlockIndex *pindex;
    if ( (pindex= komodo_chainactive(height)) != 0 )
    {
        hdr->nVersion = pindex->nVersion;
        if ( pindex->pprev == 0 )
            return(-1);
        hdr->hashPrevBlock = pindex->pprev->GetBlockHash();
        hdr->hashMerkleRoot = pindex->hashMerkleRoot;
        hdr->hashFinalSaplingRoot = pindex->hashFinalSaplingRoot;
        hdr->nTime = pindex->nTime;
        hdr->nBits = pindex->nBits;
        hdr->nNonce = pindex->nNonce;
        memcpy(hdr->nSolution,&pindex->nSolution[0],sizeof(hdr->nSolution));
        return(sizeof(*hdr));
    }
    return(-1);
}

int32_t NSPV_getinfo(struct NSPV_inforesp *ptr,int32_t reqheight)
{
    int32_t prevMoMheight,len = 0; CBlockIndex *pindex, *pindex2; struct NSPV_ntzsresp pair;
    if ( (pindex= chainActive.LastTip()) != 0 )
    {
        ptr->height = pindex->GetHeight();
        ptr->blockhash = pindex->GetBlockHash();
        memset(&pair,0,sizeof(pair));
        if ( NSPV_getntzsresp(&pair,ptr->height-1) < 0 )
            return(-1);
        ptr->notarization = pair.prevntz;
        if ( (pindex2= komodo_chainactive(ptr->notarization.txidheight)) != 0 )
            ptr->notarization.timestamp = pindex->nTime;
        //fprintf(stderr, "timestamp.%i\n", ptr->notarization.timestamp );
        if ( reqheight == 0 )
            reqheight = ptr->height;
        ptr->hdrheight = reqheight;
        ptr->version = NSPV_PROTOCOL_VERSION;
        if ( NSPV_setequihdr(&ptr->H,reqheight) < 0 )
            return(-1);
        return(sizeof(*ptr));
    } else return(-1);
}

int32_t NSPV_getaddressutxos(struct NSPV_utxosresp *ptr,char *coinaddr,bool isCC,int32_t skipcount,uint32_t filter)
{
    int64_t total = 0,interest=0; uint32_t locktime; int32_t ind=0,tipheight,maxlen,txheight,n = 0,len = 0;
    std::vector<std::pair<CAddressUnspentKey, CAddressUnspentValue> > unspentOutputs;
    SetCCunspents(unspentOutputs,coinaddr,isCC);
    maxlen = MAX_BLOCK_SIZE(tipheight) - 512;
    maxlen /= sizeof(*ptr->utxos);
    strncpy(ptr->coinaddr,coinaddr,sizeof(ptr->coinaddr)-1);
    ptr->CCflag = isCC;
    ptr->filter = filter;
    if ( skipcount < 0 )
        skipcount = 0;
    if ( (ptr->numutxos= (int32_t)unspentOutputs.size()) >= 0 && ptr->numutxos < maxlen )
    {
        tipheight = chainActive.LastTip()->GetHeight();
        ptr->nodeheight = tipheight;
        if ( skipcount >= ptr->numutxos )
            skipcount = ptr->numutxos-1;
        ptr->skipcount = skipcount;
        if ( ptr->numutxos-skipcount > 0 )
        {
            ptr->utxos = (struct NSPV_utxoresp *)calloc(ptr->numutxos-skipcount,sizeof(*ptr->utxos));
            for (std::vector<std::pair<CAddressUnspentKey, CAddressUnspentValue> >::const_iterator it=unspentOutputs.begin(); it!=unspentOutputs.end(); it++)
            {
                // if gettxout is != null to handle mempool
                {
                    if ( n >= skipcount && myIsutxo_spentinmempool(ignoretxid,ignorevin,it->first.txhash,(int32_t)it->first.index) == 0  )
                    {
                        ptr->utxos[ind].txid = it->first.txhash;
                        ptr->utxos[ind].vout = (int32_t)it->first.index;
                        ptr->utxos[ind].satoshis = it->second.satoshis;
                        ptr->utxos[ind].height = it->second.blockHeight;
                        if ( ASSETCHAINS_SYMBOL[0] == 0 && it->second.satoshis >= 10*COIN )
                        {
                            ptr->utxos[n].extradata = komodo_accrued_interest(&txheight,&locktime,ptr->utxos[ind].txid,ptr->utxos[ind].vout,ptr->utxos[ind].height,ptr->utxos[ind].satoshis,tipheight);
                            interest += ptr->utxos[ind].extradata;
                        }
                        ind++;
                        total += it->second.satoshis;
                    }
                    n++;
                }
            }
        }
        ptr->numutxos = ind;
        if ( len < maxlen )
        {
            len = (int32_t)(sizeof(*ptr) + sizeof(*ptr->utxos)*ptr->numutxos - sizeof(ptr->utxos));
            //fprintf(stderr,"getaddressutxos for %s -> n.%d:%d total %.8f interest %.8f len.%d\n",coinaddr,n,ptr->numutxos,dstr(total),dstr(interest),len);
            ptr->total = total;
            ptr->interest = interest;
            return(len);
        }
    }
    if ( ptr->utxos != 0 )
        free(ptr->utxos);
    memset(ptr,0,sizeof(*ptr));
    return(0);
}

class BaseCCChecker {
public:
    /// base check function
    /// @param vouts vouts where checked vout and opret are
    /// @param nvout vout index to check
    /// @param evalcode which must be in the opret
    /// @param funcids allowed funcids in string
    /// @param filtertxid txid that should be in the opret (after funcid)
    virtual bool checkCC(uint256 txid, const std::vector<CTxOut> &vouts, int32_t nvout, uint8_t evalcode, std::string funcids, uint256 filtertxid) = 0;
};

/// default cc vout checker for use in NSPV_getccmoduleutxos
/// checks if a vout is cc, has required evalcode, allowed funcids and txid
/// check both cc opret and last vout opret
/// maybe customized in via inheritance
class DefaultCCChecker : public BaseCCChecker {

private:

public:
    DefaultCCChecker() { }
    virtual bool checkCC(uint256 txid, const std::vector<CTxOut> &vouts, int32_t nvout, uint8_t evalcode, std::string funcids, uint256 filtertxid)
    {
        CScript opret, dummy;
        std::vector< vscript_t > vParams;
        vscript_t vopret;

        if (nvout < vouts.size())
        {
            // first check if it is cc vout
            if (vouts[nvout].scriptPubKey.IsPayToCryptoCondition(&dummy, vParams))
            {
                // try to find cc opret
                if (vParams.size() > 0)
                {
                    COptCCParams p(vParams[0]); // parse vout data
                    if (p.vData.size() > 0)
                    {
                        vopret = p.vData[0]; // get opret data
                    }
                }
                // if no cc opret check last vout opret
                if (vopret.size() == 0)
                {
                    GetOpReturnData(vouts.back().scriptPubKey, vopret);
                }
                if (vopret.size() > 2)
                {
                    uint8_t opretEvalcode, opretFuncid;
                    uint256 opretTxid;
                    bool isEof = true;
                    bool isCreateTx = false;

                    // parse opret first 3 fields:
                    bool parseOk = E_UNMARSHAL(vopret, 
                        ss >> opretEvalcode; 
                        ss >> opretFuncid; 
                        if (funcids.size() > 0 && opretFuncid == funcids[0]) // this means that we check txid only for second+ funcid in array (considering that the first funcid is the creation txid itself like tokens) 
                        {
                            isCreateTx = true;
                        }
                        else
                        {
                            ss >> opretTxid;
                            isCreateTx = false;
                        }
                        isEof = ss.eof(); );

                    opretTxid = revuint256(opretTxid);
                    std::cerr << __func__ << " " << "opretEvalcode=" << opretEvalcode << " opretFuncid=" << (char)opretFuncid << " isCreateTx=" << isCreateTx << " opretTxid=" << opretTxid.GetHex() << std::endl;
                    if( parseOk /*parseOk=true if eof reached*/|| !isEof /*more data means okay*/)
                    {
                        if (evalcode == opretEvalcode && std::find(funcids.begin(), funcids.end(), (char)opretFuncid) != funcids.end() && 
                            (isCreateTx && filtertxid == txid || !isCreateTx && filtertxid == opretTxid))
                        {
                            return true;
                        }
                    }
                }
            }
        }
        return false;
    }
};

static class DefaultCCChecker defaultCCChecker;

// table of pluggable cc vout checkers for usage in NSPV_getccmoduleutxos
// if the checker is not in the table for a evalcode then defaultCCChecker is used 
static std::map<uint8_t, class BaseCCChecker*> ccCheckerTable = 
{
};

// implements SPV server's part, gets cc module utxos, filtered by evalcode, funcid and txid on opret, for the specified amount
// if the amount param is 0 returns total available filtere utxo amount and returns no utxos 
// first char funcid in the string param is considered as the creation tx funcid so filtertxid is compared to the creation txid itself
// for other funcids filtertxid is compared to the txid in opreturn
int32_t NSPV_getccmoduleutxos(struct NSPV_utxosresp *ptr, char *coinaddr, int64_t amount, uint8_t evalcode,  std::string funcids, uint256 filtertxid)
{
    int64_t total = 0, totaladded = 0; 
    uint32_t locktime; 
    int32_t tipheight=0, len, maxlen;
    int32_t maxinputs = CC_MAXVINS;

    std::vector<struct CC_utxo> utxoSelected;
    utxoSelected.reserve(CC_MAXVINS);

    std::vector<std::pair<CAddressUnspentKey, CAddressUnspentValue> > unspentOutputs;
    SetCCunspents(unspentOutputs, coinaddr, true);

    maxlen = MAX_BLOCK_SIZE(tipheight) - 512;
    //maxlen /= sizeof(*ptr->utxos);  // TODO why was this? we need maxlen in bytes, don't we? 
    
    //ptr->numutxos = (uint16_t)unspentOutputs.size();
    //if (ptr->numutxos >= 0 && ptr->numutxos < maxlen)
    //{
    ptr->utxos = NULL;
    ptr->numutxos = 0;
    strncpy(ptr->coinaddr, coinaddr, sizeof(ptr->coinaddr) - 1);
    ptr->CCflag = 1;
    tipheight = chainActive.LastTip()->GetHeight();  
    ptr->nodeheight = tipheight; // will be checked in libnspv
    //}
   
    // select all appropriate utxos:
    std::cerr << __func__ << " " << "searching addr=" << coinaddr << std::endl;
    for (std::vector<std::pair<CAddressUnspentKey, CAddressUnspentValue> >::const_iterator it = unspentOutputs.begin(); it != unspentOutputs.end(); it++)
    {
        if (myIsutxo_spentinmempool(ignoretxid, ignorevin, it->first.txhash, (int32_t)it->first.index) == 0)
        {
            //const CCoins *pcoins = pcoinsTip->AccessCoins(it->first.txhash); <-- no opret in coins
            CTransaction tx;
            uint256 hashBlock;
            int32_t nvout = it->first.index;
            if (myGetTransaction(it->first.txhash, tx, hashBlock))
            {
                class BaseCCChecker *baseChecker = ccCheckerTable[evalcode];

                // if a checker is set for evalcode use it otherwise use the default checker:
                if (baseChecker && baseChecker->checkCC(it->first.txhash, tx.vout, nvout, evalcode, funcids, filtertxid) || defaultCCChecker.checkCC(it->first.txhash, tx.vout, nvout, evalcode, funcids, filtertxid))
                {
                    std::cerr << __func__ << " " << "filtered utxo with amount=" << tx.vout[nvout].nValue << std::endl;

                    struct CC_utxo utxo;
                    utxo.txid = it->first.txhash;
                    utxo.vout = (int32_t)it->first.index;
                    utxo.nValue = it->second.satoshis;
                    //utxo.height = it->second.blockHeight;
                    utxoSelected.push_back(utxo);
                    total += it->second.satoshis;
                }
            }
            else
                std::cerr << __func__ << " " << "ERROR: cant load tx for txid, please reindex" << std::endl;
        }
    }


    if (amount == 0) {
        // just return total value
        ptr->total = total;
        len = (int32_t)(sizeof(*ptr) - sizeof(ptr->utxos)/*subtract not serialized part of NSPV_utxoresp*/);
        return len;
    }

    // pick optimal utxos for the requested amount
    CAmount remains = amount;
    std::vector<struct CC_utxo> utxoAdded;

    while (utxoSelected.size() > 0)
    {
        int64_t below = 0, above = 0;
        int32_t abovei = -1, belowi = -1, ind = -1;

        if (CC_vinselect(&abovei, &above, &belowi, &below, utxoSelected.data(), utxoSelected.size(), remains) < 0)
        {
            std::cerr << "error CC_vinselect" << " remains=" << remains << " amount=" << amount << " abovei=" << abovei << " belowi=" << belowi << " ind=" << " utxoSelected.size()=" << utxoSelected.size() << ind << std::endl;
            return 0;
        }
        if (abovei >= 0) // best is 'above'
            ind = abovei;
        else if (belowi >= 0)  // second try is 'below'
            ind = belowi;
        else
        {
            std::cerr << "error finding unspent" << " remains=" << remains << " amount=" << amount << " abovei=" << abovei << " belowi=" << belowi << " ind=" << " utxoSelected.size()=" << utxoSelected.size() << ind << std::endl;
            return 0;
        }

        utxoAdded.push_back(utxoSelected[ind]);
        total += utxoSelected[ind].nValue;
        remains -= utxoSelected[ind].nValue;

        // remove used utxo[ind]:
        utxoSelected[ind] = utxoSelected.back();
        utxoSelected.pop_back();

        if (total >= amount) // found the requested amount
            break;
        if (utxoAdded.size() >= maxinputs)  // reached maxinputs
            break;
    }
    ptr->numutxos = (uint16_t)utxoAdded.size();
    ptr->total = total;
    ptr->utxos = (NSPV_utxoresp*)calloc(ptr->numutxos, sizeof(ptr->utxos[0]));

    for (uint16_t i = 0; i < ptr->numutxos; i++)
    {
        ptr->utxos[i].satoshis = utxoAdded[i].nValue;
        ptr->utxos[i].txid = utxoAdded[i].txid;
        ptr->utxos[i].vout = utxoAdded[i].vout;
    }
   
    len = (int32_t)(sizeof(*ptr) - sizeof(ptr->utxos)/*subtract not serialized part of NSPV_utxoresp*/ + sizeof(*ptr->utxos)*ptr->numutxos);
    if (len < maxlen) 
        return len;  // good length
    else
    {
        NSPV_utxosresp_purge(ptr);
        return 0;
    }
}

int32_t NSPV_getaddresstxids(struct NSPV_txidsresp *ptr,char *coinaddr,bool isCC,int32_t skipcount,uint32_t filter)
{
    int32_t maxlen,txheight,ind=0,n = 0,len = 0; CTransaction tx; uint256 hashBlock;
    std::vector<std::pair<CAddressIndexKey, CAmount> > txids;
    SetAddressIndexOutputs(txids,coinaddr,isCC);
    ptr->nodeheight = chainActive.LastTip()->GetHeight();
    maxlen = MAX_BLOCK_SIZE(ptr->nodeheight) - 512;
    maxlen /= sizeof(*ptr->txids);
    strncpy(ptr->coinaddr,coinaddr,sizeof(ptr->coinaddr)-1);
    ptr->CCflag = isCC;
    ptr->filter = filter;
    if ( skipcount < 0 )
        skipcount = 0;
    if ( (ptr->numtxids= (int32_t)txids.size()) >= 0 && ptr->numtxids < maxlen )
    {
        if ( skipcount >= ptr->numtxids )
            skipcount = ptr->numtxids-1;
        ptr->skipcount = skipcount;
        if ( ptr->numtxids-skipcount > 0 )
        {
            ptr->txids = (struct NSPV_txidresp *)calloc(ptr->numtxids-skipcount,sizeof(*ptr->txids));
            for (std::vector<std::pair<CAddressIndexKey, CAmount> >::const_iterator it=txids.begin(); it!=txids.end(); it++)
            {
                if ( n >= skipcount )
                {
                    ptr->txids[ind].txid = it->first.txhash;
                    ptr->txids[ind].vout = (int32_t)it->first.index;
                    ptr->txids[ind].satoshis = (int64_t)it->second;
                    ptr->txids[ind].height = (int64_t)it->first.blockHeight;
                    ind++;
                }
                n++;
            }
        }
        ptr->numtxids = ind;
        len = (int32_t)(sizeof(*ptr) + sizeof(*ptr->txids)*ptr->numtxids - sizeof(ptr->txids));
        return(len);
    }
    if ( ptr->txids != 0 )
        free(ptr->txids);
    memset(ptr,0,sizeof(*ptr));
    return(0);
}

int32_t NSPV_mempoolfuncs(bits256 *satoshisp,int32_t *vindexp,std::vector<uint256> &txids,char *coinaddr,bool isCC,uint8_t funcid,uint256 txid,int32_t vout)
{
    int32_t num = 0,vini = 0,vouti = 0; uint8_t evalcode=0,func=0;  std::vector<uint8_t> vopret; char destaddr[64];
    *vindexp = -1;
    memset(satoshisp,0,sizeof(*satoshisp));
    if ( funcid == NSPV_CC_TXIDS)
    {
        std::vector<std::pair<CAddressIndexKey, CAmount> > tmp_txids; uint256 tmp_txid,hashBlock;
        int32_t n=0,skipcount=vout>>16; uint8_t eval=(vout>>8)&0xFF, func=vout&0xFF;

        CTransaction tx;
        SetAddressIndexOutputs(tmp_txids,coinaddr,isCC);
        if ( skipcount < 0 ) skipcount = 0;
        if ( skipcount >= tmp_txids.size() )
            skipcount = tmp_txids.size()-1;
        if ( tmp_txids.size()-skipcount > 0 )
        {
            for (std::vector<std::pair<CAddressIndexKey, CAmount> >::const_iterator it=tmp_txids.begin(); it!=tmp_txids.end(); it++)
            {
                if (txid!=zeroid || func!=0)
                {
                    myGetTransaction(it->first.txhash,tx,hashBlock);
                    std::vector<vscript_t>  oprets; uint256 tokenid,txid;
                    std::vector<uint8_t> vopret,vOpretExtra; uint8_t *script,e,f;
                    std::vector<CPubKey> pubkeys;

                    if (DecodeTokenOpRetV1(tx.vout[tx.vout.size()-1].scriptPubKey,tokenid,pubkeys,oprets)!=0 && GetOpReturnCCBlob(oprets, vOpretExtra) && vOpretExtra.size()>0)
                    {
                        vopret=vOpretExtra;
                    }
                    else GetOpReturnData(tx.vout[tx.vout.size()-1].scriptPubKey, vopret);
                    script = (uint8_t *)vopret.data();
                    if ( vopret.size() > 2 && script[0]==eval )
                    {
                        switch (eval)
                        {
                            case EVAL_CHANNELS:EVAL_PEGS:EVAL_ORACLES:EVAL_GAMES:EVAL_IMPORTGATEWAY:EVAL_ROGUE:
                                E_UNMARSHAL(vopret,ss >> e; ss >> f; ss >> tmp_txid;);
                                if (e!=eval || (txid!=zeroid && txid!=tmp_txid) || (func!=0 && f!=func)) continue;
                                break;
                            case EVAL_TOKENS:EVAL_DICE:EVAL_DILITHIUM:EVAL_FAUCET:EVAL_LOTO:EVAL_PAYMENTS:EVAL_REWARDS:
                                E_UNMARSHAL(vopret,ss >> e; ss >> f;);
                                if (e!=eval || (func!=0 && f!=func)) continue;
                                break;
                            default:
                                break;
                        }
                    }                        
                }
                if ( n >= skipcount ) txids.push_back(it->first.txhash);
                n++;
            }
            return (n-skipcount);
        }
        return (0);
    }
    if ( mempool.size() == 0 )
        return(0);
    if ( funcid == NSPV_MEMPOOL_CCEVALCODE )
    {
        isCC = true;
        evalcode = vout & 0xff;
        func = (vout >> 8) & 0xff;
    }
    LOCK(mempool.cs);
    BOOST_FOREACH(const CTxMemPoolEntry &e,mempool.mapTx)
    {
        const CTransaction &tx = e.GetTx();
        const uint256 &hash = tx.GetHash();
        if ( funcid == NSPV_MEMPOOL_ALL )
        {
            txids.push_back(hash);
            num++;
            continue;
        }
        else if ( funcid == NSPV_MEMPOOL_INMEMPOOL )
        {
            if ( hash == txid )
            {
                txids.push_back(hash);
                return(++num);
            }
            continue;
        }
        else if ( funcid == NSPV_MEMPOOL_CCEVALCODE )
        {
            if ( tx.vout.size() > 1 )
            {
                CScript scriptPubKey = tx.vout[tx.vout.size()-1].scriptPubKey;
                if ( GetOpReturnData(scriptPubKey,vopret) != 0 )
                {
                    if ( vopret[0] != evalcode || (func!=0 && vopret[1] != func) ) continue;
                    txids.push_back(hash);
                    num++;
                }
            }
            continue;
        }
        if ( funcid == NSPV_MEMPOOL_ISSPENT )
        {
            BOOST_FOREACH(const CTxIn &txin,tx.vin)
            {
                //fprintf(stderr,"%s/v%d ",uint256_str(str,txin.prevout.hash),txin.prevout.n);
                if ( txin.prevout.n == vout && txin.prevout.hash == txid )
                {
                    txids.push_back(hash);
                    *vindexp = vini;
                    return(++num);
                }
                vini++;
            }
        }
        else if ( funcid == NSPV_MEMPOOL_ADDRESS )
        {
            BOOST_FOREACH(const CTxOut &txout,tx.vout)
            {
                if ( txout.scriptPubKey.IsPayToCryptoCondition() == isCC )
                {
                    Getscriptaddress(destaddr,txout.scriptPubKey);
                    if ( strcmp(destaddr,coinaddr) == 0 )
                    {
                        txids.push_back(hash);
                        *vindexp = vouti;
                        if ( num < 4 )
                            satoshisp->ulongs[num] = txout.nValue;
                        num++;
                    }
                }
                vouti++;
            }
        }
        //fprintf(stderr,"are vins for %s\n",uint256_str(str,hash));
    }
    return(num);
}

int32_t NSPV_mempooltxids(struct NSPV_mempoolresp *ptr,char *coinaddr,uint8_t isCC,uint8_t funcid,uint256 txid,int32_t vout)
{
    std::vector<uint256> txids; bits256 satoshis; uint256 tmp,tmpdest; int32_t i,len = 0;
    ptr->nodeheight = chainActive.LastTip()->GetHeight();
    strncpy(ptr->coinaddr,coinaddr,sizeof(ptr->coinaddr)-1);
    ptr->CCflag = isCC;
    ptr->txid = txid;
    ptr->vout = vout;
    ptr->funcid = funcid;
    if ( NSPV_mempoolfuncs(&satoshis,&ptr->vindex,txids,coinaddr,isCC,funcid,txid,vout) >= 0 )
    {
        if ( (ptr->numtxids= (int32_t)txids.size()) >= 0 )
        {
            if ( ptr->numtxids > 0 )
            {
                ptr->txids = (uint256 *)calloc(ptr->numtxids,sizeof(*ptr->txids));
                for (i=0; i<ptr->numtxids; i++)
                {
                    tmp = txids[i];
                    iguana_rwbignum(IGUANA_READ,(uint8_t *)&tmp,sizeof(*ptr->txids),(uint8_t *)&ptr->txids[i]);
                }
            }
            if ( funcid == NSPV_MEMPOOL_ADDRESS )
            {
                memcpy(&tmp,&satoshis,sizeof(tmp));
                iguana_rwbignum(IGUANA_READ,(uint8_t *)&tmp,sizeof(ptr->txid),(uint8_t *)&ptr->txid);
            }
            len = (int32_t)(sizeof(*ptr) + sizeof(*ptr->txids)*ptr->numtxids - sizeof(ptr->txids));
            return(len);
        }
    }
    if ( ptr->txids != 0 )
        free(ptr->txids);
    memset(ptr,0,sizeof(*ptr));
    return(0);
}

int32_t NSPV_remoterpc(struct NSPV_remoterpcresp *ptr,char *json,int n)
{
    std::vector<uint256> txids; int32_t i,len = 0; UniValue result; std::string response;
    UniValue request(UniValue::VOBJ),rpc_result(UniValue::VOBJ); JSONRequest jreq; CPubKey mypk;

    try
    {
        request.read(json,n);
        jreq.parse(request);
        strcpy(ptr->method,jreq.strMethod.c_str());
        len+=sizeof(ptr->method);
        std::map<std::string, bool>::iterator it = nspv_remote_commands.find(jreq.strMethod);
        if (it==nspv_remote_commands.end())
            throw JSONRPCError(RPC_METHOD_NOT_FOUND, "Method not supported!");
        const CRPCCommand *cmd=tableRPC[jreq.strMethod];
        if (!cmd)
            throw JSONRPCError(RPC_METHOD_NOT_FOUND, "Method not found");
        if (it->second)
        {
            if (!request.exists("mypk"))
                throw JSONRPCError(RPC_PARSE_ERROR, "No pubkey supplied in remote rpc request, necessary for this type of rpc");
            std::string str=request["mypk"].get_str();
            mypk=pubkey2pk(ParseHex(str));
            if (!mypk.IsValid())
                throw JSONRPCError(RPC_PARSE_ERROR, "Not valid pubkey passed in remote rpc call");
        }
        if ((result = cmd->actor(jreq.params,false,mypk)).isObject() || result.isArray())
        {
            rpc_result = JSONRPCReplyObj(result, NullUniValue, jreq.id);
            response=rpc_result.write();
            ptr->json = (char*)malloc(response.size()+1);
            strcpy(ptr->json, response.c_str());
            len+=response.size();
            return (len);
        }
        else throw JSONRPCError(RPC_MISC_ERROR, "Error in executing RPC on remote node");        
    }
    catch (const UniValue& objError)
    {
        rpc_result = JSONRPCReplyObj(NullUniValue, objError, jreq.id);
        response=rpc_result.write();
    }
    catch (const runtime_error& e)
    {
        rpc_result = JSONRPCReplyObj(NullUniValue,JSONRPCError(RPC_PARSE_ERROR, e.what()), jreq.id);
        response=rpc_result.write();
    }
    catch (const std::exception& e)
    {
        rpc_result = JSONRPCReplyObj(NullUniValue,JSONRPCError(RPC_PARSE_ERROR, e.what()), jreq.id);
        response=rpc_result.write();
    }
    ptr->json = (char*)malloc(response.size()+1);
    strcpy(ptr->json,response.c_str());
    len+=response.size();
    return (len);
}

uint8_t *NSPV_getrawtx(CTransaction &tx,uint256 &hashBlock,int32_t *txlenp,uint256 txid)
{
    uint8_t *rawtx = 0;
    *txlenp = 0;
    {
        if (!GetTransaction(txid, tx, hashBlock, false))
        {
            //fprintf(stderr,"error getting transaction %s\n",txid.GetHex().c_str());
            return(0);
        }
        string strHex = EncodeHexTx(tx);
        *txlenp = (int32_t)strHex.size() >> 1;
        if ( *txlenp > 0 )
        {
            rawtx = (uint8_t *)calloc(1,*txlenp);
            decode_hex(rawtx,*txlenp,(char *)strHex.c_str());
        }
    }
    return(rawtx);
}

int32_t NSPV_sendrawtransaction(struct NSPV_broadcastresp *ptr,uint8_t *data,int32_t n)
{
    CTransaction tx;
    ptr->retcode = 0;
    if ( NSPV_txextract(tx,data,n) == 0 )
    {
        //LOCK(cs_main);
        ptr->txid = tx.GetHash();
        //fprintf(stderr,"try to addmempool transaction %s\n",ptr->txid.GetHex().c_str());
        if ( myAddtomempool(tx) != 0 )
        {
            ptr->retcode = 1;
            //int32_t i;
            //for (i=0; i<n; i++)
            //    fprintf(stderr,"%02x",data[i]);
            fprintf(stderr," relay transaction %s retcode.%d\n",ptr->txid.GetHex().c_str(),ptr->retcode);
            RelayTransaction(tx);
        } else ptr->retcode = -3;

    } else ptr->retcode = -1;
    return(sizeof(*ptr));
}

// get txproof for txid, 
// to get the proof object the height should be passed
// otherwise only block hash alone will be returned
int32_t NSPV_gettxproof(struct NSPV_txproof* ptr, int32_t vout, uint256 txid, int32_t height)
{
    int32_t flag = 0, len = 0;
    CTransaction _tx;
    uint256 hashBlock;
    CBlock block;
    CBlockIndex* pindex;
    ptr->height = -1;

    if ((ptr->tx = NSPV_getrawtx(_tx, hashBlock, &ptr->txlen, txid)) != 0) {
        ptr->txid = txid;
        ptr->vout = vout;
        ptr->hashblock = hashBlock;
        if (height == 0)
            ptr->height = komodo_blockheight(hashBlock);
        else {
            ptr->height = height;
            if ((pindex = komodo_chainactive(height)) != 0 && komodo_blockload(block, pindex) == 0) {
                BOOST_FOREACH (const CTransaction& tx, block.vtx) {
                    if (tx.GetHash() == txid) {
                        flag = 1;
                        break;
                    }
                }
                if (flag != 0) {
                    set<uint256> setTxids;
                    CDataStream ssMB(SER_NETWORK, PROTOCOL_VERSION);
                    setTxids.insert(txid);
                    CMerkleBlock mb(block, setTxids);
                    ssMB << mb;
                    std::vector<uint8_t> proof(ssMB.begin(), ssMB.end());
                    ptr->txprooflen = (int32_t)proof.size();
                    LogPrint("nspv-details", "%s txid.%s found txproof.(%s) height.%d\n", __func__, txid.GetHex().c_str(), HexStr(proof).c_str(), ptr->height);
                    if (ptr->txprooflen > 0) {
                        ptr->txproof = (uint8_t*)calloc(1, ptr->txprooflen);
                        memcpy(ptr->txproof, &proof[0], ptr->txprooflen);
                    }
                    //fprintf(stderr,"gettxproof slen.%d\n",(int32_t)(sizeof(*ptr) - sizeof(ptr->tx) - sizeof(ptr->txproof) + ptr->txlen + ptr->txprooflen));
                }
            }
        }
        ptr->unspentvalue = CCgettxout(txid, vout, 1, 1);
    }
    return (sizeof(*ptr) - sizeof(ptr->tx) - sizeof(ptr->txproof) + ptr->txlen + ptr->txprooflen);
}

int32_t NSPV_getntzsproofresp(struct NSPV_ntzsproofresp *ptr,uint256 prevntztxid,uint256 nextntztxid)
{
    int32_t i; uint256 hashBlock,bhash0,bhash1,desttxid0,desttxid1; CTransaction tx;
    ptr->prevtxid = prevntztxid;
    ptr->prevntz = NSPV_getrawtx(tx,hashBlock,&ptr->prevtxlen,ptr->prevtxid);
    ptr->prevtxidht = komodo_blockheight(hashBlock);
    if ( NSPV_notarizationextract(0,&ptr->common.prevht,&bhash0,&desttxid0,tx) < 0 )
        return(-2);
    else if ( komodo_blockheight(bhash0) != ptr->common.prevht )
        return(-3);
    
    ptr->nexttxid = nextntztxid;
    ptr->nextntz = NSPV_getrawtx(tx,hashBlock,&ptr->nexttxlen,ptr->nexttxid);
    ptr->nexttxidht = komodo_blockheight(hashBlock);
    if ( NSPV_notarizationextract(0,&ptr->common.nextht,&bhash1,&desttxid1,tx) < 0 )
        return(-5);
    else if ( komodo_blockheight(bhash1) != ptr->common.nextht )
        return(-6);

    else if ( ptr->common.prevht > ptr->common.nextht || (ptr->common.nextht - ptr->common.prevht) > 1440 )
    {
        fprintf(stderr,"illegal prevht.%d nextht.%d\n",ptr->common.prevht,ptr->common.nextht);
        return(-7);
    }
    //fprintf(stderr,"%s -> prevht.%d, %s -> nexht.%d\n",ptr->prevtxid.GetHex().c_str(),ptr->common.prevht,ptr->nexttxid.GetHex().c_str(),ptr->common.nextht);
    ptr->common.numhdrs = (ptr->common.nextht - ptr->common.prevht + 1);
    ptr->common.hdrs = (struct NSPV_equihdr *)calloc(ptr->common.numhdrs,sizeof(*ptr->common.hdrs));
    //fprintf(stderr,"prev.%d next.%d allocate numhdrs.%d\n",prevht,nextht,ptr->common.numhdrs);
    for (i=0; i<ptr->common.numhdrs; i++)
    {
        //hashBlock = NSPV_hdrhash(&ptr->common.hdrs[i]);
        //fprintf(stderr,"hdr[%d] %s\n",prevht+i,hashBlock.GetHex().c_str());
        if ( NSPV_setequihdr(&ptr->common.hdrs[i],ptr->common.prevht+i) < 0 )
        {
            fprintf(stderr,"error setting hdr.%d\n",ptr->common.prevht+i);
            free(ptr->common.hdrs);
            ptr->common.hdrs = 0;
            return(-1);
        }
    }
    //fprintf(stderr,"sizeof ptr %ld, common.%ld lens.%d %d\n",sizeof(*ptr),sizeof(ptr->common),ptr->prevtxlen,ptr->nexttxlen);
    return(sizeof(*ptr) + sizeof(*ptr->common.hdrs)*ptr->common.numhdrs - sizeof(ptr->common.hdrs) - sizeof(ptr->prevntz) - sizeof(ptr->nextntz) + ptr->prevtxlen + ptr->nexttxlen);
}

int32_t NSPV_getspentinfo(struct NSPV_spentinfo *ptr,uint256 txid,int32_t vout)
{
    int32_t len = 0;
    ptr->txid = txid;
    ptr->vout = vout;
    ptr->spentvini = -1;
    len = (int32_t)(sizeof(*ptr) - sizeof(ptr->spent.tx) - sizeof(ptr->spent.txproof));
    if ( CCgetspenttxid(ptr->spent.txid,ptr->spentvini,ptr->spent.height,txid,vout) == 0 )
    {
        if ( NSPV_gettxproof(&ptr->spent,0,ptr->spent.txid,ptr->spent.height) > 0 )
            len += ptr->spent.txlen + ptr->spent.txprooflen;
        else
        {
            NSPV_txproof_purge(&ptr->spent);
            return(-1);
        }
    }
    return(len);
}

void komodo_nSPVreq(CNode *pfrom,std::vector<uint8_t> request) // received a request
{
    int32_t slen, reqheight; 
    std::vector<uint8_t> response; 
    uint32_t timestamp = (uint32_t)time(NULL);

    if (request.size() == 0) {
        LogPrint("nspv", "empty request from peer %d\n", pfrom->id);
        return;
    }
    
    int32_t len = request.size();

    // rate limit no more 1 request/sec of same type from same node:
    int32_t ind = request[0] >> 1;
    if (ind >= sizeof(pfrom->prevtimes)/sizeof(*pfrom->prevtimes) )
        ind = (int32_t)(sizeof(pfrom->prevtimes)/sizeof(*pfrom->prevtimes)) - 1;
    if (pfrom->prevtimes[ind] > timestamp)
        pfrom->prevtimes[ind] = 0;
    else if (timestamp == pfrom->prevtimes[ind])  {
        LogPrint("nspv", "rate limit from peer %d\n", pfrom->id);
        return;
    }

    switch(request[0])    
    {
    case NSPV_INFO: // info
        {
            struct NSPV_inforesp I;
            //fprintf(stderr,"check info %u vs %u, ind.%d\n",timestamp,pfrom->prevtimes[ind],ind);    
            if (len == 1 + sizeof(reqheight))
                iguana_rwnum(IGUANA_READ, &request[1], sizeof(reqheight), &reqheight);
            else
                reqheight = 0;
            //fprintf(stderr,"request height.%d\n",reqheight);
            memset(&I, 0, sizeof(I));
            if ((slen = NSPV_getinfo(&I, reqheight)) > 0) {
                response.resize(1 + slen);
                response[0] = NSPV_INFORESP;
                //fprintf(stderr,"slen.%d version.%d\n",slen,I.version);
                if (NSPV_rwinforesp(IGUANA_WRITE, &response[1], &I) == slen)
                {
                    //fprintf(stderr,"send info resp to id %d\n",(int32_t)pfrom->id);
                    pfrom->PushMessage("nSPV",response);
                    pfrom->prevtimes[ind] = timestamp;
                    LogPrint("nspv-details", "NSPV_INFO response: version %d to node=%d\n", I.version, pfrom->id);
                }
                else 
                    LogPrint("nspv", "NSPV_rwinforesp incorrect response len.%d\n", slen); 
                NSPV_inforesp_purge(&I);
            } else
                LogPrint("nspv", "NSPV_getinfo error.%d\n", slen);
        }
        break;

    case NSPV_UTXOS:
        {
            struct NSPV_utxosresp U;
            char coinaddr[KOMODO_ADDRESS_BUFSIZE];                 
            int32_t skipcount = 0;     
            uint32_t filter = 0; 
            uint8_t isCC = 0;
            //fprintf(stderr,"utxos: %u > %u, ind.%d, len.%d\n",timestamp,pfrom->prevtimes[ind],ind,len);
        
            if (len < 2)  {
                LogPrint("nspv", "NSPV_UTXOS bad request too short len.%d node %d\n", len, pfrom->id);
                return;
            }
            
            int32_t offset = sizeof(request[0]) + sizeof(request[1]);
            int32_t addrlen = request[1];
            if (offset + addrlen > len || addrlen > sizeof(coinaddr)-1)  // out of bounds
            {
                LogPrint("nspv", "NSPV_UTXOS bad request len.%d too short or addrlen.%d out of bounds, node=%d\n", len, addrlen, pfrom->id);
                return;
            }

            memcpy(coinaddr, &request[offset], addrlen);
            coinaddr[addrlen] = 0;
            offset += addrlen;
            if (offset + sizeof(isCC) <= len)  // TODO: a bit different from others format - allows omitted params, maybe better have fixed
            {
                isCC = (request[offset] != 0);
                offset += sizeof(isCC);
                if (offset + sizeof(skipcount) <= len)
                {
                    iguana_rwnum(IGUANA_READ, &request[offset], sizeof(skipcount), &skipcount);
                    offset += sizeof(skipcount);
                    if (offset + sizeof(filter) <= len)  
                    {
                        iguana_rwnum(IGUANA_READ, &request[offset], sizeof(filter), &filter);
                        offset += sizeof(filter);
                    }
                }
            }
            if (offset != len) {
                LogPrint("nspv", "NSPV_UTXOS bad request parameters format: len.%d, offset.%d, addrlen.%d, node=%d\n", len, offset, addrlen, pfrom->id);
                return;
            }

            LogPrint("nspv-details", "NSPV_UTXOS address=%s isCC.%d skipcount.%d filter.%x\n", coinaddr, isCC, skipcount, filter);
            memset(&U, 0, sizeof(U));
            if ((slen = NSPV_getaddressutxos(&U, coinaddr, isCC, skipcount, filter)) > 0)
            {
                response.resize(1 + slen);
                response[0] = NSPV_UTXOSRESP;
                if ( NSPV_rwutxosresp(IGUANA_WRITE, &response[1], &U) == slen )
                {
                    pfrom->PushMessage("nSPV",response);
                    pfrom->prevtimes[ind] = timestamp;
                    LogPrint("nspv-details", "NSPV_UTXOS response: numutxos=%d to node=%d\n", U.numutxos, pfrom->id);
                }
                else 
                    LogPrint("nspv", "NSPV_rwutxosresp incorrect response len.%d\n", slen); 
                NSPV_utxosresp_purge(&U);
            }
            else
                LogPrint("nspv", "NSPV_getaddressutxos error.%d\n", slen);
        }
        break;

    case NSPV_TXIDS:
        {
            struct NSPV_txidsresp T;
            char coinaddr[KOMODO_ADDRESS_BUFSIZE];                 
            int32_t skipcount = 0;     
            uint32_t filter = 0; 
            uint8_t isCC = 0;
            //fprintf(stderr,"utxos: %u > %u, ind.%d, len.%d\n",timestamp,pfrom->prevtimes[ind],ind,len);
        
            if (len < 2)  {
                LogPrint("nspv", "NSPV_TXIDS bad request too short len.%d, node %d\n", len, pfrom->id);
                return;
            }
            
            int32_t offset = sizeof(request[0]) + sizeof(request[1]);
            int32_t addrlen = request[1];
            if (offset + addrlen > len || addrlen > sizeof(coinaddr)-1)  // out of bounds
            {
                LogPrint("nspv", "NSPV_TXIDS bad request len.%d too short or addrlen.%d out of bounds, node=%d\n", len, addrlen, pfrom->id);
                return;
            }

            memcpy(coinaddr, &request[offset], addrlen);
            coinaddr[addrlen] = 0;
            offset += addrlen;
            if (offset + sizeof(isCC) <= len)  // TODO: a bit different from others format - allows omitted params, maybe better have fixed
            {
                isCC = (request[offset] != 0);
                offset += sizeof(isCC);
                if (offset + sizeof(skipcount) <= len)
                {
                    iguana_rwnum(IGUANA_READ, &request[offset], sizeof(skipcount), &skipcount);
                    offset += sizeof(skipcount);
                    if (offset + sizeof(filter) <= len)  
                    {
                        iguana_rwnum(IGUANA_READ, &request[offset], sizeof(filter), &filter);
                        offset += sizeof(filter);
                    }
                }
            }
            if (offset != len) {
                LogPrint("nspv", "NSPV_TXIDS bad request parameters format: len.%d, offset.%d, addrlen.%d, node=%d\n", len, offset, addrlen, pfrom->id);
                return;
            }

            LogPrint("nspv-details", "NSPV_TXIDS address=%s isCC.%d skipcount.%d filter.%x\n",coinaddr, isCC, skipcount, filter);

            memset(&T, 0, sizeof(T));
            if ((slen = NSPV_getaddresstxids(&T, coinaddr, isCC, skipcount, filter)) > 0)
            {
                //fprintf(stderr,"slen.%d\n",slen);
                response.resize(1 + slen);
                response[0] = NSPV_TXIDSRESP;
                if (NSPV_rwtxidsresp(IGUANA_WRITE, &response[1], &T) == slen)
                {
                    pfrom->PushMessage("nSPV",response);
                    pfrom->prevtimes[ind] = timestamp;
                    LogPrint("nspv-details", "NSPV_TXIDS response: numtxids=%d to node=%d\n", (int)T.numtxids, pfrom->id);
                }
                else 
                    LogPrint("nspv", "NSPV_rwtxidsresp incorrect response len.%d\n", slen);       
                NSPV_txidsresp_purge(&T);
            }
            else
                LogPrint("nspv", "NSPV_getaddresstxids error.%d\n", slen);
        }
        break;

    case NSPV_MEMPOOL: 
        {
            struct NSPV_mempoolresp M;
            char coinaddr[KOMODO_ADDRESS_BUFSIZE];
            int32_t vout;
            uint256 txid;
            uint8_t funcid, isCC = 0;
            int8_t addrlen;
            if (len > 1 + sizeof(isCC) + sizeof(funcid) + sizeof(vout) + sizeof(txid) + sizeof(addrlen)) 
            {
                uint32_t offset = 1;
                offset += iguana_rwnum(IGUANA_READ, &request[offset], sizeof(isCC), &isCC);
                offset += iguana_rwnum(IGUANA_READ, &request[offset], sizeof(funcid), &funcid);
                offset += iguana_rwnum(IGUANA_READ, &request[offset], sizeof(vout), &vout);
                offset += iguana_rwbignum(IGUANA_READ, &request[offset], sizeof(txid), (uint8_t*)&txid);
                addrlen = request[offset++];
                if (addrlen < sizeof(coinaddr) && offset + addrlen == len) 
                {
                    memcpy(coinaddr, &request[offset], addrlen);
                    coinaddr[addrlen] = 0;
                    offset += addrlen;
                    LogPrint("nspv-details", "address (%s) isCC.%d funcid.%d %s/v%d len.%d slen.%d\n", coinaddr, isCC, funcid, txid.GetHex().c_str(), vout, len, addrlen);
                    memset(&M, 0, sizeof(M));
                    if ((slen = NSPV_mempooltxids(&M, coinaddr, isCC, funcid, txid, vout)) > 0) {
                        //fprintf(stderr,"NSPV_mempooltxids slen.%d\n",slen);
                        response.resize(1 + slen);
                        response[0] = NSPV_MEMPOOLRESP;
                        if (NSPV_rwmempoolresp(IGUANA_WRITE, &response[1], &M) == slen) {
                            pfrom->PushMessage("nSPV", response);
                            pfrom->prevtimes[ind] = timestamp;
                            LogPrint("nspv-details", "NSPV_MEMPOOL response: numtxids=%d to node=%d\n", M.numtxids, pfrom->id);
                        } else
                            LogPrint("nspv", "NSPV_rwmempoolresp incorrect response len.%d\n", slen);
                        NSPV_mempoolresp_purge(&M);
                    } 
                    else
                        LogPrint("nspv", "NSPV_mempooltxids err.%d\n", slen);
                } 
                else
                    LogPrint("nspv", "NSPV_MEMPOOL incorrect addrlen.%d offset.%d len.%d\n", addrlen, offset, len);
            } 
            else
                LogPrint("nspv", "NSPV_MEMPOOL incorrect len.%d too short, node %d\n", len,  pfrom->id);
        } 
        break;

    case NSPV_NTZS:
        {
            struct NSPV_ntzsresp N; 
            int32_t height;
            if (len == 1 + sizeof(height)) 
            {
                iguana_rwnum(IGUANA_READ, &request[1], sizeof(height), &height);
                memset(&N, 0, sizeof(N));
                if ((slen = NSPV_getntzsresp(&N, height)) > 0)
                {
                    response.resize(1 + slen);
                    response[0] = NSPV_NTZSRESP;
                    if (NSPV_rwntzsresp(IGUANA_WRITE, &response[1], &N) == slen)
                    {
                        pfrom->PushMessage("nSPV",response);
                        pfrom->prevtimes[ind] = timestamp;
                        LogPrint("nspv-details", "NSPV_NTZS response: prevntz.txid=%s nextntx.txid=%s node=%d\n", N.prevntz.txid.GetHex().c_str(), N.nextntz.txid.GetHex().c_str(), pfrom->id);
                    }
                    else 
                        LogPrint("nspv", "NSPV_rwntzsresp incorrect response len.%d\n", slen);
                    NSPV_ntzsresp_purge(&N);
                }
                else 
                    LogPrint("nspv", "NSPV_rwntzsresp err.%d\n", slen);
            } else
                LogPrint("nspv","NSPV_NTZS bad request len.%d node %d\n", len, pfrom->id);
        }
        break;

    case NSPV_NTZSPROOF:
        {
            struct NSPV_ntzsproofresp P; 
            uint256 prevntz, nextntz;
            if (len == 1 + sizeof(prevntz) + sizeof(nextntz))
            {
                iguana_rwbignum(IGUANA_READ, &request[1], sizeof(prevntz), (uint8_t*)&prevntz);
                iguana_rwbignum(IGUANA_READ, &request[1 + sizeof(prevntz)], sizeof(nextntz), (uint8_t*)&nextntz);
                memset(&P, 0, sizeof(P));
                if ((slen = NSPV_getntzsproofresp(&P, prevntz, nextntz)) > 0) 
                {
                    // fprintf(stderr,"slen.%d msg prev.%s next.%s\n",slen,prevntz.GetHex().c_str(),nextntz.GetHex().c_str());
                    response.resize(1 + slen);
                    response[0] = NSPV_NTZSPROOFRESP;
                    if (NSPV_rwntzsproofresp(IGUANA_WRITE, &response[1], &P) == slen)
                    {
                        pfrom->PushMessage("nSPV", response);
                        pfrom->prevtimes[ind] = timestamp;
                        LogPrint("nspv-details", "NSPV_NTZSPROOF response: prevtxidht=%d nexttxidht=%d node=%d\n", P.prevtxidht, P.nexttxidht, pfrom->id);
                    }
                    else 
                        LogPrint("nspv", "NSPV_rwntzsproofresp incorrect response len.%d\n", slen);
                    NSPV_ntzsproofresp_purge(&P);
                } 
                else
                    LogPrint("nspv", "NSPV_NTZSPROOF err.%d\n", slen);
            }
            else 
                LogPrint("nspv","NSPV_NTZSPROOF bad request len.%d node %d\n", len, pfrom->id);
        }
        break;

    case NSPV_TXPROOF: 
        {
            struct NSPV_txproof P; 
            uint256 txid; 
            int32_t height, vout;
            if (len == 1 + sizeof(txid) + sizeof(height) + sizeof(vout))
            {
                iguana_rwnum(IGUANA_READ, &request[1], sizeof(height), &height);
                iguana_rwnum(IGUANA_READ, &request[1 + sizeof(height)], sizeof(vout), &vout);
                iguana_rwbignum(IGUANA_READ, &request[1 + sizeof(height) + sizeof(vout)], sizeof(txid), (uint8_t*)&txid);
                //fprintf(stderr,"got txid %s/v%d ht.%d\n",txid.GetHex().c_str(),vout,height);
                memset(&P, 0, sizeof(P));
                if ((slen = NSPV_gettxproof(&P,vout,txid,height)) > 0)
                {
                    //fprintf(stderr,"slen.%d\n",slen);
                    response.resize(1 + slen);
                    response[0] = NSPV_TXPROOFRESP;
                    if (NSPV_rwtxproof(IGUANA_WRITE, &response[1], &P) == slen)
                    {
                        //fprintf(stderr,"send response\n");
                        pfrom->PushMessage("nSPV",response);
                        pfrom->prevtimes[ind] = timestamp;
                        LogPrint("nspv-details", "NSPV_TXPROOF response: txlen=%d txprooflen=%d node=%d\n", P.txlen, P.txprooflen, pfrom->id);
                    }
                    else
                        LogPrint("nspv", "NSPV_rwtxproof incorrect response len.%d\n", slen);
                    NSPV_txproof_purge(&P);
                } 
                else 
                    LogPrint("nspv", "gettxproof error.%d\n", slen);
            } 
            else 
                LogPrint("nspv","txproof bad request len.%d node %d\n", len, pfrom->id);
        }
        break;

    case NSPV_SPENTINFO:
        {
            struct NSPV_spentinfo S; 
            int32_t vout; 
            uint256 txid;

            if (len == 1 + sizeof(txid) + sizeof(vout))
            {
                iguana_rwnum(IGUANA_READ, &request[1], sizeof(vout), &vout);
                iguana_rwbignum(IGUANA_READ, &request[1 + sizeof(vout)], sizeof(txid), (uint8_t*)&txid);
                memset(&S, 0, sizeof(S));
                if ((slen = NSPV_getspentinfo(&S, txid, vout)) > 0)
                {
                    response.resize(1 + slen);
                    response[0] = NSPV_SPENTINFORESP;
                    if (NSPV_rwspentinfo(IGUANA_WRITE, &response[1], &S) == slen)
                    {
                        pfrom->PushMessage("nSPV",response);
                        pfrom->prevtimes[ind] = timestamp;
                        LogPrint("nspv-details", "NSPV_SPENTINFO response: spent txid=%s vout=%d node=%d\n", S.txid.GetHex().c_str(), S.vout, pfrom->id);
                    }
                    else
                        LogPrint("nspv", "NSPV_rwspentinfo incorrect response len.%d\n", slen);
                    NSPV_spentinfo_purge(&S);
                }
                else
                    LogPrint("nspv", "NSPV_getspentinfo error.%d node=%d\n", slen, pfrom->id);
            }
            else
                LogPrint("nspv", "NSPV_SPENTINFO bad request len.%d node=%d\n", len, pfrom->id);
        }
        break;

    case NSPV_BROADCAST:
        {
            struct NSPV_broadcastresp B; 
            uint256 txid;
            int32_t txlen;
            if (len > 1 + sizeof(txid) + sizeof(txlen))
            {
                int32_t offset = 1;

                offset += iguana_rwbignum(IGUANA_READ, &request[offset], sizeof(txid), (uint8_t*)&txid);
                offset += iguana_rwnum(IGUANA_READ, &request[offset], sizeof(txlen), &txlen);
                memset(&B, 0, sizeof(B));
                if (txlen < MAX_TX_SIZE_AFTER_SAPLING && request.size() == offset + txlen && (slen = NSPV_sendrawtransaction(&B, &request[offset], txlen)) > 0) 
                {
                    response.resize(1 + slen);
                    response[0] = NSPV_BROADCASTRESP;
                    if (NSPV_rwbroadcastresp(IGUANA_WRITE, &response[1], &B) == slen)
                    {
                        pfrom->PushMessage("nSPV",response);
                        pfrom->prevtimes[ind] = timestamp;
                        LogPrint("nspv-details", "NSPV_BROADCAST response: txid=%s vout=%d to node=%d\n", B.txid.GetHex().c_str(), pfrom->id);
                    }
                    else
                        LogPrint("nspv", "NSPV_rwbroadcastresp incorrect response len.%d\n", slen);
                    NSPV_broadcast_purge(&B);
                } 
                else
                    LogPrint("nspv", "NSPV_BROADCAST either wrong tx len.%d or NSPV_sendrawtransaction error.%d, node=%d\n", txlen, slen, pfrom->id);
            }
            else
                LogPrint("nspv", "NSPV_BROADCAST bad request len.%d node %d\n", len, pfrom->id);

        }
        break;

    case NSPV_REMOTERPC:   
        {
            struct NSPV_remoterpcresp R; 
            int32_t offset = 1;
            offset += iguana_rwnum(IGUANA_READ, &request[offset], sizeof(slen), &slen);
            memset(&R, 0, sizeof(R));
            if (len == (offset + slen) && (slen = NSPV_remoterpc(&R,(char *)&request[offset],slen))>0 )
            {
                response.resize(1 + slen);
                response[0] = NSPV_REMOTERPCRESP;
                NSPV_rwremoterpcresp(IGUANA_WRITE, &response[1], &R, slen);
                pfrom->PushMessage("nSPV", response);
                pfrom->prevtimes[ind] = timestamp;
                LogPrint("nspv-details", "NSPV_REMOTERPCRESP response: method=%s json=%s to node=%d\n", R.method, R.json, pfrom->id);
                NSPV_remoterpc_purge(&R);
            }
            else
                LogPrint("nspv", "NSPV_REMOTERPC bad request len.%d node %d\n", len, pfrom->id);
        }                
        break;
        
    case NSPV_CCMODULEUTXOS:  // get cc module utxos from coinaddr for the requested amount, evalcode, funcid list and txid
        {
            struct NSPV_utxosresp U;
            char coinaddr[64];
            int64_t amount;
            uint8_t evalcode;
            uint8_t funcidslen;
            char funcids[27];
            uint256 filtertxid;
            bool errorFormat = false;
            //fprintf(stderr,"utxos: %u > %u, ind.%d, len.%d\n",timestamp,pfrom->prevtimes[ind],ind,len);
            
            if (len < 3)  {
                LogPrint("nspv", "NSPV_CCMODULEUTXOS bad request len.%d too short, node=%d\n", len, pfrom->id);
                return;
            }
            
            int32_t offset = 1;
            int32_t addrlen = request[offset++];
            if (addrlen >= sizeof(coinaddr) || offset + addrlen > len)
            {
                LogPrint("nspv", "NSPV_CCMODULEUTXOS bad request len.%d too short or addrlen %d too long, node=%d\n", len, addrlen, pfrom->id);
                return;
            }

            memcpy(coinaddr, &request[offset], addrlen);
            coinaddr[addrlen] = 0;
            offset += addrlen;

            if (offset + sizeof(amount) + sizeof(evalcode) + sizeof(funcidslen) > len)  {
                LogPrint("nspv", "NSPV_CCMODULEUTXOS bad request len.%d too short, node=%d\n", len, pfrom->id);
                return;
            }
            offset += iguana_rwnum(IGUANA_READ, &request[offset], sizeof(amount), &amount);
            offset += iguana_rwnum(IGUANA_READ, &request[offset], sizeof(evalcode), &evalcode);
            funcidslen = request[offset++];

            if (funcidslen >= sizeof(funcids) || offset + funcidslen > len)
            {
                LogPrint("nspv", "NSPV_CCMODULEUTXOS bad request len.%d no room for funcids or too many funcids %d, node=%d\n", len, funcidslen, pfrom->id);
                return;
            }

            memcpy(funcids, &request[offset], funcidslen);
            funcids[funcidslen] = 0;
            offset += funcidslen;

            if (offset + sizeof(filtertxid) != len)  {
                LogPrint("nspv", "NSPV_CCMODULEUTXOS bad request len.%d incorrect room for filtertxid param, node=%d\n", len, funcidslen, pfrom->id);
                return;
            }
            iguana_rwbignum(IGUANA_READ, &request[offset], sizeof(filtertxid), (uint8_t *)&filtertxid);
            memset(&U, 0, sizeof(U));
            if ((slen = NSPV_getccmoduleutxos(&U, coinaddr, amount, evalcode, funcids, filtertxid)) > 0)
            {
                response.resize(1 + slen);
                response[0] = NSPV_CCMODULEUTXOSRESP;
                if (NSPV_rwutxosresp(IGUANA_WRITE, &response[1], &U) == slen)
                {
                    pfrom->PushMessage("nSPV", response);
                    pfrom->prevtimes[ind] = timestamp;
                    LogPrint("nspv-details", "NSPV_CCMODULEUTXOS returned %d utxos to node=%d\n", (int)U.numutxos, pfrom->id);
                }
                else
                    LogPrint("nspv", "NSPV_rwutxosresp incorrect response len.%d\n", slen);
                NSPV_utxosresp_purge(&U);
            }
            else
                LogPrint("nspv", "NSPV_getccmoduleutxos error.%d, node %d\n", slen, pfrom->id);
        }
        break;
    }
}

#endif // KOMODO_NSPVFULLNODE_H
