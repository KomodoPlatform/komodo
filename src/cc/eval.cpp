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

#include <assert.h>
#include <cryptoconditions.h>

#include "primitives/block.h"
#include "primitives/transaction.h"
#include "script/cc.h"
#include "cc/eval.h"
#include "cc/utils.h"
#include "cc/CCinclude.h"
#include "main.h"
#include "chain.h"
#include "core_io.h"
#include <memory>

bool CClib_Dispatch(const CC *cond,Eval *eval,std::vector<uint8_t> paramsNull,const CTransaction &txTo,unsigned int nIn);
char *CClib_name();

struct CCcontract_info CCinfos[0x100];
extern pthread_mutex_t KOMODO_CC_mutex;

bool RunCCEval(const CC *cond, const CTransaction &tx, unsigned int nIn, Eval* eval)
{
    pthread_mutex_lock(&KOMODO_CC_mutex);
    bool out = eval->Dispatch(cond, tx, nIn);
    pthread_mutex_unlock(&KOMODO_CC_mutex);
    if ( eval->state.IsValid() != out)
        fprintf(stderr,"out %d vs %d isValid\n",(int32_t)out,(int32_t)eval->state.IsValid());
    //assert(eval->state.IsValid() == out);

    if (eval->state.IsValid()) return true;

    std::string lvl = eval->state.IsInvalid() ? "Invalid" : "Error!";
    fprintf(stderr, "CC Eval %s %s: %s spending tx %s\n",
            EvalToStr(cond->code[0]).data(),
            lvl.data(),
            eval->state.GetRejectReason().data(),
            tx.vin[nIn].prevout.hash.GetHex().data());
    if (eval->state.IsError()) fprintf(stderr, "Culprit: %s\n", EncodeHexTx(tx).data());
    CTransaction tmp; 
    if (eval->pool.lookup(tx.GetHash(), tmp))
    {
        // This is to remove a payments airdrop if it gets stuck in the mempool. 
        // Miner will mine 1 invalid block, but doesnt stop them mining until a restart.
        // This would almost never happen in normal use.
        std::list<CTransaction> dummy;
        eval->pool.remove(tx,dummy,true);
    }
    return false;
}


/*
 * Test the validity of an Eval node
 */
bool Eval::Dispatch(const CC *cond, const CTransaction &txTo, unsigned int nIn)
{
    struct CCcontract_info *cp;
    if (cond->codeLength == 0)
        return Invalid("empty-eval");

    uint8_t ecode = cond->code[0];
    if ( ASSETCHAINS_CCDISABLES[ecode] != 0 )
    {
        // check if a height activation has been set. 
        if ( mapHeightEvalActivate[ecode] == 0 || this->GetCurrentHeight() == 0 || mapHeightEvalActivate[ecode] > this->GetCurrentHeight() )
        {
            fprintf(stderr,"%s evalcode.%d %02x\n",txTo.GetHash().GetHex().c_str(),ecode,ecode);
            fprintf(stderr, "ac_ccactivateht: evalcode.%i activates at height.%i vs current height.%i\n", ecode, mapHeightEvalActivate[ecode], this->GetCurrentHeight());
            return Invalid("disabled-code, -ac_ccenables didnt include this ecode");
        }
    }
    std::vector<uint8_t> vparams(cond->code+1, cond->code+cond->codeLength);
    if ( ecode >= EVAL_FIRSTUSER && ecode <= EVAL_LASTUSER )
    {
        if ( ASSETCHAINS_CCLIB.size() > 0 && ASSETCHAINS_CCLIB == CClib_name() )
            return CClib_Dispatch(cond,this,vparams,txTo,nIn);
        else return Invalid("mismatched -ac_cclib vs CClib_name");
    }
    cp = &CCinfos[(int32_t)ecode];
    if ( cp->didinit == 0 )
    {
        CCinit(cp,ecode);
        cp->didinit = 1;
    }

    switch ( ecode )
    {
        case EVAL_IMPORTPAYOUT:
            return ImportPayout(vparams, txTo, nIn);
            break;

        case EVAL_IMPORTCOIN:
            return ImportCoin(vparams, txTo, nIn);
            break;

        default:
            return(ProcessCC(cp,this, vparams, txTo, nIn));
            break;
    }
    return Invalid("invalid-code, dont forget to add EVAL_NEWCC to Eval::Dispatch");
}


bool Eval::GetSpendsConfirmed(uint256 hash, std::vector<CTransaction> &spends) const
{
    // NOT IMPLEMENTED
    return false;
}


bool Eval::GetTxUnconfirmed(const uint256 &hash, CTransaction &txOut, uint256 &hashBlock) const
{
    return myGetTransaction(hash, txOut,hashBlock, pool);
}


bool Eval::GetTxConfirmed(const uint256 &hash, CTransaction &txOut, CBlockIndex &block) const
{
    uint256 hashBlock;
    if (!GetTxUnconfirmed(hash, txOut, hashBlock))
        return false;
    if (hashBlock.IsNull() || !GetBlock(hashBlock, block))
        return false;
    return true;
}

unsigned int Eval::GetCurrentHeight() const
{
    return chainActive.Height();
}

bool Eval::GetBlock(uint256 hash, CBlockIndex& blockIdx) const
{
    auto r = mapBlockIndex.find(hash);
    if (r != mapBlockIndex.end()) {
        blockIdx = *r->second;
        return true;
    }
    fprintf(stderr, "CC Eval Error: Can't get block from index\n");
    return false;
}

extern int32_t komodo_notaries(uint8_t pubkeys[64][33],int32_t height,uint32_t timestamp);


int32_t Eval::GetNotaries(uint8_t pubkeys[64][33], int32_t height, uint32_t timestamp) const
{
    return komodo_notaries(pubkeys, height, timestamp);
}

bool Eval::CheckTxAuthority(const CTransaction &tx, CrosschainAuthority auth) const
{
    if (tx.vin.size() < auth.requiredSigs) return false;

    uint8_t seen[64] = {0};

    BOOST_FOREACH(const CTxIn &txIn, tx.vin)
    {
        // Get notary pubkey
        CTransaction tx;
        uint256 hashBlock;
        if (!this->GetTxUnconfirmed(txIn.prevout.hash, tx, hashBlock)) return false;
        if (tx.vout.size() < txIn.prevout.n) return false;
        CScript spk = tx.vout[txIn.prevout.n].scriptPubKey;
        if (spk.size() != 35) return false;
        const unsigned char *pk = &spk[0];
        if (pk++[0] != 33) return false;
        if (pk[33] != OP_CHECKSIG) return false;

        // Check it's a notary
        for (int i=0; i<auth.size; i++) {
            if (!seen[i]) {
                if (memcmp(pk, auth.notaries[i], 33) == 0) {
                    seen[i] = 1;
                    goto found;
                } else {
                    //printf("notary.%i is not valid!\n",i);
                }
            }
        }

        return false;
        found:;
    }

    return true;
}

bool Eval::CheckNotaryInputs(const CTransaction &tx, uint32_t height, uint32_t timestamp) const
{
    if (tx.vin.size() < 11) return false;

    CrosschainAuthority auth;
    auth.requiredSigs = 11;
    auth.size = GetNotaries(auth.notaries, height, timestamp);

    return CheckTxAuthority(tx, auth);
}

/*
 * Get MoM from a notarisation tx hash (on KMD)
 */
bool Eval::GetNotarisationData(const uint256 notaryHash, NotarisationData &data) const
{
    CTransaction notarisationTx;
    CBlockIndex block;
    if (!GetTxConfirmed(notaryHash, notarisationTx, block)) return false;
    if (!CheckNotaryInputs(notarisationTx, block.GetHeight(), block.nTime)) return false;
    if (!ParseNotarisationOpReturn(notarisationTx, data)) return false;
    return true;
}


uint32_t Eval::GetAssetchainsCC() const
{
    return ASSETCHAINS_CC;
}


std::string Eval::GetAssetchainsSymbol() const
{
    return std::string(ASSETCHAINS_SYMBOL);
}


/*
 * Notarisation data, ie, OP_RETURN payload in notarisation transactions
 */
bool ParseNotarisationOpReturn(const CTransaction &tx, NotarisationData &data)
{
    if (tx.vout.size() < 2) return false;
    std::vector<unsigned char> vdata;
    if (!GetOpReturnData(tx.vout[1].scriptPubKey, vdata)) return false;
    bool out = E_UNMARSHAL(vdata, ss >> data);
    return out;
}


/*
 * Misc
 */
std::string EvalToStr(EvalCode c)
{
    FOREACH_EVAL(EVAL_GENERATE_STRING);
    char s[10];
    sprintf(s, "0x%x", c);
    return std::string(s);

}
