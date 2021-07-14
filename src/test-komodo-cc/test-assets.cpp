
#include <cryptoconditions.h>
#include <gtest/gtest.h>

#include "cc/CCinclude.h"

#include "cc/CCtokens.h"
#include "cc/CCtokens_impl.h"

#include "cc/CCassets.h"
#include "cc/CCassetstx_impl.h"

#include "cc/CCTokelData.h"


#include "cc/eval.h"
#include "importcoin.h"
#include "base58.h"
#include "core_io.h"
#include "key.h"
#include "main.h"
#include "primitives/transaction.h"
#include "script/cc.h"
#include "script/interpreter.h"
#include "script/serverchecker.h"
#include "txmempool.h"

extern Eval* EVAL_TEST;

namespace CCAssetsTests {

static uint8_t testNum = 0;

uint256 getRandomHash()
{
    CSHA256 sha;
    uint256 res; 
    uint32_t rnd = rand();
    sha.Write((uint8_t*)&rnd, sizeof(rnd));
    sha.Finalize((uint8_t*)&res);
    return res;
}


class EvalMock : public Eval
{
private:
    int currentHeight;
    std::map<uint256, CTransaction> txs;
    std::map<uint256, CBlockIndex> blocks;
    //sstd::map<uint256, std::vector<CTransaction>> spends;
public:
    void AddTx(const CTransaction &tx) {
        if (!tx.IsNull()) {
            txs[tx.GetHash()] = tx;
        }
    }
    const std::map<uint256, CTransaction> & getTxs() { return txs; }
    bool GetTxUnconfirmed(const uint256 &hash, CTransaction &txOut, uint256 &hashBlock) const
    {
        //std::cerr <<__func__ << " hash=" << hash.GetHex() << std::endl;
        auto r = txs.find(hash);
        if (r != txs.end()) {
            //std::cerr <<__func__ << " hash=" << hash.GetHex() << " found" << std::endl;
            txOut = r->second;
            if (blocks.count(hash) > 0)
                hashBlock = hash;
            return true;
        }
        return false;
    }
};

static EvalMock eval;

static CTransaction txnormal1, txnormal2, txnormal3, txnormal4, txnormalg, txask1, txask2, txbid1, txbid2, txtokencreate1, txtokencreate2, txtokencreate3, txtokencreateUnused;
static uint256 tokenid1, tokenid2, tokenid3, tokenidUnused;

//  RJXkCF7mn2DRpUZ77XBNTKCe55M2rJbTcu
static CPubKey pk1 = CPubKey(ParseHex("035d3b0f2e98cf0fba19f80880ec7c08d770c6cf04aa5639bc57130d5ac54874db"));
static uint8_t privkey1[] = { 0x0d, 0xf0, 0x44, 0xc4, 0xbe, 0xd3, 0x3b, 0x74, 0xaf, 0x69, 0x6b, 0x05, 0x1d, 0xbf, 0x70, 0x14, 0x2f, 0xc3, 0xa7, 0x8d, 0xa3, 0x47, 0x38, 0xc0, 0x33, 0x6f, 0x50, 0x15, 0xe3, 0xd2, 0x85, 0xee };
//  RR2nTYFBPTJafxQ6en2dhUgaJcMDk4RWef
static CPubKey pk2 = CPubKey(ParseHex("034777b18effce6f7a849b72de8e6810bf7a7e050274b3782e1b5a13d0263a44dc"));
static uint8_t privkey2[] = { 0x9e, 0x69, 0x33, 0x27, 0x1c, 0x1c, 0x60, 0x5e, 0x57, 0xaf, 0xb6, 0xb7, 0xfd, 0xeb, 0xac, 0xa3, 0x11, 0x41, 0xd1, 0x3a, 0xe2, 0x36, 0x47, 0xfc, 0xe4, 0xe0, 0x79, 0x44, 0xae, 0xee, 0x43, 0xde };
//  RTbiYv9u1mrp7TmJspxduJCe3oarCqv9K4
static CPubKey pk3 = CPubKey(ParseHex("025f97b6c42409e8e69eb2fdab281219aafe15169deec801ee621c63cc1ba0bb8c"));
static uint8_t privkey3[] = { 0x0d, 0xf8, 0xcd, 0xd4, 0x42, 0xbd, 0x77, 0xd2, 0xdd, 0x44, 0x89, 0x4b, 0x21, 0x78, 0xbf, 0x8d, 0x8a, 0xc3, 0x30, 0x0c, 0x5a, 0x70, 0x3d, 0xbe, 0xc9, 0x21, 0x75, 0x33, 0x23, 0x77, 0xd3, 0xde };

std::map<CPubKey, uint8_t*> testKeys = {
    { pk1, privkey1 },
    { pk2, privkey2 },
    { pk3, privkey3 },
};

//char Tokensv2CChexstr[] = { "032fd27f72591b02f13a7f9701246eb0296b2be7cfdad32c520e594844ec3d4801" };
//uint8_t Tokensv2CCpriv[] = { 0xb5, 0xba, 0x92, 0x7f, 0x53, 0x45, 0x4f, 0xf8, 0xa4, 0xad, 0x0d, 0x38, 0x30, 0x4f, 0xd0, 0x97, 0xd1, 0xb7, 0x94, 0x1b, 0x1f, 0x52, 0xbd, 0xae, 0xa2, 0xe7, 0x49, 0x06, 0x2e, 0xd2, 0x2d, 0xa5 };

static CPubKey pkunused = CPubKey(ParseHex("034b082c5819b5bf8798a387630ad236a8e800dbce4c4e24a46f36dfddab3cbff5"));

CAmount TestAddNormalInputs(CMutableTransaction &mtx, CPubKey mypk, CAmount amount)
{
    CAmount totalInputs = 0LL;
    char mypkaddr[KOMODO_ADDRESS_BUFSIZE];
    Getscriptaddress(mypkaddr,  CScript() << ParseHex(HexStr(mypk)) << OP_CHECKSIG);
    for (auto const &t : eval.getTxs()) {
        for (int32_t v = 0; v < t.second.vout.size(); v ++) {
            char utxoaddr[KOMODO_ADDRESS_BUFSIZE];
            Getscriptaddress(utxoaddr, t.second.vout[v].scriptPubKey);
            //std::cerr << __func__ << " utxoaddr=" << utxoaddr << " mypkaddr=" << mypkaddr << " tx=" << t.second.GetHash().GetHex() << " v=" << v << std::endl;
            if (strcmp(utxoaddr, mypkaddr) == 0)    {
                mtx.vin.push_back(CTxIn(t.second.GetHash(), v));
                totalInputs += t.second.vout[v].nValue;
                if (totalInputs >= amount)
                    return totalInputs;
            }
        }
    }
    return 0LL;
}
CAmount TestAddTokenInputs(CMutableTransaction &mtx, CPubKey mypk, uint256 tokenid, CAmount amount)
{
    CAmount totalInputs = 0LL;
    struct CCcontract_info *cp, C;
    cp = CCinit(&C, TokensV2::EvalCode());
    char mypkaddr[KOMODO_ADDRESS_BUFSIZE];
    Getscriptaddress(mypkaddr,  TokensV2::MakeCC1vout(EVAL_TOKENSV2, 0, mypk).scriptPubKey);
    for (auto const &t : eval.getTxs()) {
        for (int32_t v = 0; v < t.second.vout.size(); v ++) {
            char utxoaddr[KOMODO_ADDRESS_BUFSIZE];
            Getscriptaddress(utxoaddr, t.second.vout[v].scriptPubKey);
            CTransaction tx;
            //std::cerr << __func__ << " utxoaddr=" << utxoaddr << " mypkaddr=" << mypkaddr << " tx=" << t.second.GetHash().GetHex() << " v=" << v << std::endl;
            if (strcmp(utxoaddr, mypkaddr) == 0 &&
                IsTokensvout<TokensV2>(cp, &eval, t.second, v, tokenid))    {
                mtx.vin.push_back(CTxIn(t.second.GetHash(), v));
                totalInputs += t.second.vout[v].nValue;
                if (totalInputs >= amount)
                    return totalInputs;
            }
        }
    }
    return 0LL;
}


bool TestSignTx(const CKeyStore& keystore, CMutableTransaction& mtx, int32_t vini, CAmount utxovalue, const CScript scriptPubKey)
{
    CTransaction txNewConst(mtx);
    SignatureData sigdata;
    auto consensusBranchId = CurrentEpochBranchId(chainActive.Height() + 1, Params().GetConsensus());
    if (ProduceSignature(TransactionSignatureCreator(&keystore, &txNewConst, vini, utxovalue, SIGHASH_ALL), scriptPubKey, sigdata, consensusBranchId) != 0) {
        UpdateTransaction(mtx, vini, sigdata);
        return true;
    } else {
        std::cerr << __func__ << " signing error for vini=" << vini << " amount=" << utxovalue << std::endl;
        return false;
    }
}

// sign normal and cc inputs like FinalizeCCV2Tx does
// signatures are not checked really in these tests
// however normal and cc vins should have valid script or condition as this is checked in the assets cc validation code 
// in functions like cp->ismyvin() or TotalPubkeyNormalAmount()
bool TestFinalizeTx(CMutableTransaction& mtx, struct CCcontract_info *cp, uint8_t *myprivkey, CAmount txfee, CScript opret)
{
    auto consensusBranchId = CurrentEpochBranchId(chainActive.Height() + 1, Params().GetConsensus());
    CAmount totaloutputs = 0LL;
    for (int i = 0; i < mtx.vout.size(); i ++) 
        totaloutputs += mtx.vout[i].nValue;

    if (!cp || !cp->evalcode)  {
        std::cerr << __func__ << " invalid cp param" << std::endl;
        return false;
    }

    CAmount totalinputs = 0LL;
    for (int i = 0; i < mtx.vin.size(); i ++) 
    {
        CTransaction vintx;
        uint256 hashBlock;
        if (i == 0 && mtx.vin[i].prevout.n == 10e8)
            continue; // skip pegs vin
        
        if (GetTxUnconfirmedOpt(&eval, mtx.vin[i].prevout.hash, vintx, hashBlock)) {
            totalinputs += vintx.vout[mtx.vin[i].prevout.n].nValue;
        } else {
            fprintf(stderr, "%s couldnt find vintx %s\n", __func__, mtx.vin[i].prevout.hash.ToString().c_str());
            return false;
        }
    }

    if (!myprivkey)  {
        std::cerr << __func__ << "myprivkey not set" << std::endl;
        return false;
    }

    CKey mykey;
    mykey.SetKey32(myprivkey);
    CPubKey mypk = mykey.GetPubKey();
    if (totalinputs >= totaloutputs + txfee) {
        CAmount change = totalinputs - (totaloutputs + txfee);
        if (change > 0)
            mtx.vout.push_back(CTxOut(change, CScript() << ParseHex(HexStr(mypk)) << OP_CHECKSIG));
    }

    if (opret.size() > 0)
        mtx.vout.push_back(CTxOut(0, opret));

    CPubKey globalpk;
    char myccaddr[KOMODO_ADDRESS_BUFSIZE], 
         globaladdr[KOMODO_ADDRESS_BUFSIZE],
         mytokenaddr[KOMODO_ADDRESS_BUFSIZE] = { '\0' };
    globalpk = GetUnspendable(cp, NULL);
    _GetCCaddress(myccaddr, cp->evalcode, mypk, true);
    _GetCCaddress(globaladdr, cp->evalcode, globalpk, true);
    GetTokensCCaddress(cp, mytokenaddr, mypk, true); // get token or nft probe

    PrecomputedTransactionData txdata(mtx);
    for (int i = 0; i < mtx.vin.size(); i++) 
    {
        CTransaction vintx;
        uint256 hashBlock;
        bool mgret;

        if (i == 0 && mtx.vin[i].prevout.n == 10e8) // skip PEGS vin
            continue;
        if ((mgret = GetTxUnconfirmedOpt(&eval, mtx.vin[i].prevout.hash, vintx, hashBlock)) != false) {
            CCwrapper cond;
            uint8_t *privkey = NULL;

            if (!vintx.vout[mtx.vin[i].prevout.n].scriptPubKey.IsPayToCryptoCondition()) 
            {
                /*char coinaddr[KOMODO_ADDRESS_BUFSIZE];
                Getscriptaddress(coinaddr, CScript() << ParseHex(HexStr(mypk)) << OP_CHECKSIG);
                if (strcmp(destaddr, coinaddr) != 0)    {
                    fprintf(stderr, "%s signing error for normal vini.%d myprivkey does not match\n", __func__, i);
                    return false;
                }*/

                CBasicKeyStore tempKeystore;
                CKey key;
                key.Set(myprivkey, myprivkey+32, true);
                tempKeystore.AddKey(key);
                if (!TestSignTx(tempKeystore, mtx, i, vintx.vout[mtx.vin[i].prevout.n].nValue, vintx.vout[mtx.vin[i].prevout.n].scriptPubKey))  {
                    fprintf(stderr, "%s signing error for normal vini.%d\n", __func__, i);
                    return false;
                }

            } else {
                char destaddr[KOMODO_ADDRESS_BUFSIZE];
                if (!Getscriptaddress(destaddr, vintx.vout[mtx.vin[i].prevout.n].scriptPubKey))  {
                    std::cerr << __func__ << " could not Getscriptaddress for scriptPubKey=" << vintx.vout[mtx.vin[i].prevout.n].scriptPubKey.ToString() << std::endl;
                    return false;
                }

                if (strcmp(destaddr, globaladdr) == 0) {
                    std::cerr << __func__ << " found globaladdress=" << globaladdr << " destaddr=" << destaddr << " strlen=" << strlen(globaladdr) << " evalcode=" << (int)cp->evalcode << std::endl;

                    privkey = cp->CCpriv;
                    cond.reset(MakeCCcond1(cp->evalcode, globalpk));
                } else if (strcmp(destaddr, myccaddr) == 0) {
                    privkey = myprivkey;
                    cond.reset(MakeCCcond1(cp->evalcode, mypk));
                    std::cerr << __func__ << " found myccaddr=" << myccaddr << std::endl;

                } else if (strcmp(destaddr, mytokenaddr) == 0) {
                    privkey = myprivkey;
                    cond.reset(MakeTokensv2CCcond1(cp->evalcode, cp->evalcodeAdd, mypk));
                    std::cerr << __func__ << " found mytokenaddr=" << mytokenaddr << " evalcode=" << (int)cp->evalcode << " evalcodeAdd=" << (int)cp->evalcodeAdd << std::endl;
                } else {
                    const uint8_t nullpriv[32] = {'\0'};
                    // use vector of dest addresses and conds to probe vintxconds
                    for (auto& t : cp->CCvintxprobes) {
                        char coinaddr[KOMODO_ADDRESS_BUFSIZE];
                        if (t.CCwrapped.get() != NULL) {
                            CCwrapper anonCond = t.CCwrapped;
                            CCtoAnon(anonCond.get());
                            Getscriptaddress(coinaddr, CCPubKey(anonCond.get(), true));
                            if (strcmp(destaddr, coinaddr) == 0) {
                                std::cerr << __func__ << " found vintxprobe=" << coinaddr  << " privkey=" << (memcmp(t.CCpriv, nullpriv, sizeof(t.CCpriv) / sizeof(t.CCpriv[0])) != 0) << std::endl;
                                if (memcmp(t.CCpriv, nullpriv, sizeof(t.CCpriv) / sizeof(t.CCpriv[0])) != 0)
                                    privkey = t.CCpriv;
                                else
                                    privkey = myprivkey;
                                cond = t.CCwrapped;
                                break;
                            }
                        }
                    }
                }
                if (cond.get() == NULL) {
                    fprintf(stderr, "%s vini.%d has CC signing error: could not find matching cond, address.(%s) %s\n", __func__, i, destaddr, EncodeHexTx(mtx).c_str());
                    return false;
                }

                uint256 sighash = SignatureHash(CCPubKey(cond.get()), mtx, i, SIGHASH_ALL, vintx.vout[mtx.vin[i].prevout.n].nValue, consensusBranchId, &txdata);
                if (cc_signTreeSecp256k1Msg32(cond.get(), privkey, sighash.begin()) == 0) {
                    fprintf(stderr, "%s vini.%d has CC signing error: cc_signTreeSecp256k1Msg32 returned error, address.(%s) %s\n", __func__, i, destaddr, EncodeHexTx(mtx).c_str());
                    return false;
                }
                mtx.vin[i].scriptSig = CCSig(cond.get());
            }
        } else {
            fprintf(stderr, "%s could not find tx %s myGetTransaction returned %d\n", __func__, mtx.vin[i].prevout.hash.ToString().c_str(), mgret);
            return false;
        }
    }
    return true;
}



class TestAssetsCC : public ::testing::Test {
public:
    //uint32_t GetAssetchainsCC() const { return testCcid; }
    //std::string GetAssetchainsSymbol() const { return testSymbol; }

protected:
    static void SetUpTestCase() {  
        // setup eval for tests
        ASSETCHAINS_CC = 2;

        CreateMockTransactions();
    }
    virtual void SetUp() {
        // enable print
        fDebug = true;
        fPrintToConsole = true;
        mapMultiArgs["-debug"] = { "cctokens", "ccassets" };
    }

    // run over CC V2 validation 
    bool TestRunCCEval(const CMutableTransaction &mtx)
    {
        CTransaction tx(mtx);
        PrecomputedTransactionData txdata(tx);
        ServerTransactionSignatureChecker checker(&tx, 0, 0, false, NULL, txdata);
        CValidationState verifystate;
        VerifyEval verifyEval = [] (CC *cond, void *checker) {
            //fprintf(stderr,"checker.%p\n",(TransactionSignatureChecker*)checker);
            return ((TransactionSignatureChecker*)checker)->CheckEvalCondition(cond);
        };

        // set some vars used in validation:
        KOMODO_CONNECTING = 1;
        KOMODO_CCACTIVATE = 1;
        eval.state = CValidationState(); // clear validation state
        EVAL_TEST = &eval;

        for(const auto &vin : tx.vin)  {
            if (IsCCInput(vin.scriptSig))  {
                CC *cond = GetCryptoCondition(vin.scriptSig);
                if (cond == NULL) {
                    std::cerr << __func__ << " GetCryptoCondition could not decode vin.scriptSig" << std::endl;
                    return false;
                }

                int r = cc_verifyEval(cond, verifyEval, &checker);
                if (r == 0) {
                    std::cerr << __func__ << " cc_verify error D" << std::endl;
                    return false;
                }
                std::cerr << __func__ << " cc_verify okay for vin.hash=" << vin.prevout.hash.GetHex() << std::endl;
                break;
            }
        }
        for(const auto &vout : tx.vout)  {
            if (vout.scriptPubKey.IsPayToCCV2())  {

                ScriptError error;

                bool bCheck = checker.CheckCryptoCondition(vout.scriptPubKey.GetCCV2SPK(), &error);
                if (!bCheck) {
                    std::cerr << __func__ << " CheckCryptoCondition error=" << ScriptErrorString(error) << " eval=" << eval.state.GetRejectReason() << std::endl;
                    return false;
                }
                std::cerr << __func__ << " cc_verify okay for vout.nValue=" << vout.nValue << std::endl;
            }
        }
        return true;
    }

    static void CreateMockTransactions()
    {
        txnormal1 = MakeNormalTx(pk1, 20000);
        eval.AddTx(txnormal1);

        txnormal2 = MakeNormalTx(pk1, 20000);
        eval.AddTx(txnormal2);

        txnormal3 = MakeNormalTx(pk2, 20000);
        eval.AddTx(txnormal3);

        txnormal4 = MakeNormalTx(pk2, 20000);
        eval.AddTx(txnormal4);

        struct CCcontract_info *cpTokens, tokensC;  
        cpTokens = CCinit(&tokensC, TokensV2::EvalCode());
        struct CCcontract_info *cpAssets, C; 
        cpAssets = CCinit(&C, AssetsV2::EvalCode());   // NOTE: assets here!
    
        txnormalg = MakeNormalTx(GetUnspendable(cpTokens, NULL), 20000);
        eval.AddTx(txnormalg);

        txtokencreate1 = MakeTokenV2CreateTx(pk1, 10);
        eval.AddTx(txtokencreate1);
        tokenid1 = txtokencreate1.GetHash();

        txtokencreate2 = MakeTokenV2CreateTx(pk2, 10);
        eval.AddTx(txtokencreate2);
        tokenid2 = txtokencreate2.GetHash();

        txtokencreate3 = MakeTokenV2CreateTx(pk2, 10);
        eval.AddTx(txtokencreate3);
        tokenid3 = txtokencreate3.GetHash();

        txtokencreateUnused = MakeTokenV2CreateTx(pk1, 10);
        eval.AddTx(txtokencreateUnused);
        tokenidUnused = txtokencreateUnused.GetHash();

        txask1 = MakeTokenV2AskTx(cpTokens, pk1, tokenid1, 2, ASSETS_NORMAL_DUST+1);
        eval.AddTx(txask1);

        txask2 = MakeTokenV2AskTx(cpTokens, pk1, tokenid1, 2, ASSETS_NORMAL_DUST+1);
        eval.AddTx(txask2);

        txbid1 = MakeTokenV2BidTx(cpAssets, pk2, tokenid2, 2, ASSETS_NORMAL_DUST+1);
        eval.AddTx(txbid1);


        //txbid2 = MakeTokenV2BidTx(pk2, 1000+1, 2, 1000/2);  // test dust

    }

    static CTransaction MakeNormalTx(CPubKey pk, CAmount val)
    {
        CMutableTransaction mtx = CreateNewContextualCMutableTransaction(Params().GetConsensus(), komodo_nextheight());
        mtx.vin.push_back(CTxIn(getRandomHash(), 0));
        mtx.vout.push_back(CTxOut(val, GetScriptForDestination(pk)));
        //eval.txs[mtx.GetHash()] = mtx;
        return CTransaction(mtx);
    }

    static CMutableTransaction MakeTokenV2CreateTx(CPubKey pk, CAmount amount, const UniValue &utokeldata = NullUniValue)
    {
        struct CCcontract_info *cp, C;
	    cp = CCinit(&C, TokensV2::EvalCode());
        std::string name = "Test" + std::to_string(rand());
        std::string description = "desc";
        CAmount txfee = 10000;

        CMutableTransaction mtx = CreateNewContextualCMutableTransaction(Params().GetConsensus(), komodo_nextheight());
        if (TestAddNormalInputs(mtx, pk, txfee) == 0LL)  {
            std::cerr << __func__ << " could not add normal inputs" << std::endl;
            return CTransaction();
        }
        mtx.vout.push_back(TokensV2::MakeCC1vout(TokensV2::EvalCode(), TOKENS_MARKER_VALUE, GetUnspendable(cp, NULL)));           
		mtx.vout.push_back(TokensV2::MakeTokensCC1vout(0, amount, pk));

        std::vector<vuint8_t> vextras;
        vuint8_t vtokeldata;
        if (!utokeldata.isNull())
            vtokeldata = ParseTokelJson(utokeldata);
        if (!vtokeldata.empty())
            vextras.push_back(vtokeldata);

        if (!TestFinalizeTx(mtx, cp, testKeys[pk], txfee,
            TokensV2::EncodeTokenCreateOpRet(vscript_t(pk.begin(), pk.end()), name, description, vextras)))  {
            std::cerr << __func__ << " could finalize tx" << std::endl;
            return CTransaction(); 
        }
        //eval.txs[mtx.GetHash()] = mtx;
        return mtx;
    }


    static CMutableTransaction MakeTokenV2AskTx(struct CCcontract_info *cpTokens, CPubKey pk, uint256 tokenid, CAmount numtokens, CAmount unit_price)
    {
        CMutableTransaction mtx = CreateNewContextualCMutableTransaction(Params().GetConsensus(), komodo_nextheight());

        struct CCcontract_info *cpAssets, C; 
        cpAssets = CCinit(&C, AssetsV2::EvalCode());   

        CAmount txfee = 10000;
        CAmount askamount = numtokens * unit_price;

        if (TestAddNormalInputs(mtx, pk, txfee) == 0LL) {
            std::cerr << __func__ << " cant add normal inputs" << std::endl;
            return CTransaction();
        }

        TokenDataTuple tokenData;
        vuint8_t vextraData;
        uint8_t evalcodeAdd = 0;
        int64_t royaltyFract = 0;  // royaltyFract is N in N/1000 fraction
        if (!GetTokenData<TokensV2>(&eval, tokenid, tokenData, vextraData)) {
            std::cerr << __func__ << " cant get tokendata" << std::endl;
            return CTransaction(); 
        }
        if (vextraData.size() > 0)  {
            evalcodeAdd = vextraData[0];
        }

        CAmount inputs = TestAddTokenInputs(mtx, pk, tokenid, numtokens);
        if (inputs == 0)    {
            std::cerr << __func__ << " cant add token inputs" << std::endl;
            return CTransaction();
        }

        CPubKey unspendableAssetsPubkey = GetUnspendable(cpAssets, NULL);
        mtx.vout.push_back(TokensV2::MakeTokensCC1vout(AssetsV2::EvalCode(), evalcodeAdd, numtokens, unspendableAssetsPubkey));
        mtx.vout.push_back(TokensV2::MakeCC1vout(AssetsV2::EvalCode(), ASSETS_MARKER_AMOUNT, pk));  
        CAmount CCchange = inputs - numtokens;
        if (CCchange != 0LL) {
            // change to single-eval or non-fungible token vout (although for non-fungible token change currently is not possible)
            mtx.vout.push_back(TokensV2::MakeTokensCC1vout(evalcodeAdd ? evalcodeAdd : TokensV2::EvalCode(), CCchange, pk));	
        }

        // cond to spend NFT from mypk 
        CCwrapper wrCond(TokensV2::MakeTokensCCcond1(evalcodeAdd, pk));
        CCAddVintxCond(cpTokens, wrCond, NULL); //NULL indicates to use myprivkey

        // sign vins:
        if(!TestFinalizeTx(mtx, cpTokens, testKeys[pk], txfee,
            TokensV2::EncodeTokenOpRet(tokenid, { unspendableAssetsPubkey },     
                { AssetsV2::EncodeAssetOpRet('s', zeroid, unit_price, vuint8_t(pk.begin(), pk.end())) }))) {
            std::cerr << __func__ << " cant finalise tx" << std::endl;
            return CTransaction();
        }
        return mtx;
    }

    static CMutableTransaction MakeTokenV2BidTx(struct CCcontract_info *cpAssets, CPubKey pk, uint256 tokenid, CAmount numtokens, CAmount unit_price)
    {
        CMutableTransaction mtx = CreateNewContextualCMutableTransaction(Params().GetConsensus(), komodo_nextheight());
        //struct CCcontract_info *cpAssets, C; 

        CAmount bidamount = numtokens * unit_price;
        //CAmount numtokens = 2;
        CAmount txfee = 10000;

        //cpAssets = CCinit(&C, AssetsV2::EvalCode());   // NOTE: assets here!
        //CAmount unit_price = bidamount / numtokens;

        if (TestAddNormalInputs(mtx, pk, txfee) == 0LL) {
            std::cerr << __func__ << " cant add normal inputs" << std::endl;
            return CTransaction();
        }
        CPubKey unspendableAssetsPubkey = GetUnspendable(cpAssets, 0);
        mtx.vout.push_back(TokensV2::MakeCC1vout(AssetsV2::EvalCode(), bidamount, unspendableAssetsPubkey));
        mtx.vout.push_back(TokensV2::MakeCC1vout(AssetsV2::EvalCode(), ASSETS_MARKER_AMOUNT, pk));  // marker for my orders

        // sign vins:
        if (!TestFinalizeTx(mtx, cpAssets, testKeys[pk], txfee,
            TokensV2::EncodeTokenOpRet(tokenid, {},     
                { AssetsV2::EncodeAssetOpRet('b', zeroid, unit_price, vuint8_t(pk.begin(), pk.end())) }))) {
            std::cerr << __func__ << " cant finalise tx" << std::endl;
            return CTransaction(); 
        }  
        return mtx;
    }

    static CMutableTransaction MakeTokenV2FillAskTx(struct CCcontract_info *cpAssets, CPubKey mypk, uint256 tokenid, uint256 asktxid, CAmount fill_units, CAmount paid_unit_price, UniValue &data)
    {
        CMutableTransaction mtx = CreateNewContextualCMutableTransaction(Params().GetConsensus(), komodo_nextheight());
        struct CCcontract_info *cpTokens, tokensC;

        cpTokens = CCinit(&tokensC, TokensV2::EvalCode());  

        CTransaction asktx;
        uint256 hashBlock;
        if (!GetTxUnconfirmedOpt(&eval, asktxid, asktx, hashBlock)) {
            std::cerr << __func__ << " could not load asktx" << std::endl;
            return CTransaction();
        }

        CAmount unit_price = 0;
        // CAmount fillunits = ask;
        CAmount txfee = 10000;
        const int32_t askvout = ASSETS_GLOBALADDR_VOUT; 
        uint256 assetidOpret;
        //CAmount paid_unit_price = -1;  // not set
        CAmount orig_assetoshis = asktx.vout[askvout].nValue;

        TokenDataTuple tokenData;
        vuint8_t vextraData;
        uint8_t evalcodeAdd = 0;
        int64_t royaltyFract = 0;  // royaltyFract is N in N/1000 fraction
        if (!GetTokenData<TokensV2>(&eval, tokenid, tokenData, vextraData)) {
            std::cerr << __func__ << " cant get tokendata" << std::endl;
            return CTransaction(); 
        }
        if (vextraData.size() > 0)  {
            evalcodeAdd = vextraData[0];
            GetTokelDataAsInt64(vextraData, TKLPROP_ROYALTY, royaltyFract);
            if (royaltyFract > TKLROYALTY_DIVISOR-1)
                royaltyFract = TKLROYALTY_DIVISOR-1; // royalty upper limit
        }
        vuint8_t ownerpubkey = std::get<0>(tokenData);

        vuint8_t origpubkey;
        uint8_t funcid = GetOrderParams<AssetsV2>(origpubkey, unit_price, assetidOpret, asktx); // get orig pk, orig value

        if (paid_unit_price <= 0LL)
            paid_unit_price = unit_price;

        CAmount paid_nValue = paid_unit_price * fill_units;
        CAmount royaltyValue = royaltyFract > 0 ? paid_nValue / TKLROYALTY_DIVISOR * royaltyFract : 0;
        if (royaltyFract > 0 && paid_nValue - royaltyValue <= ASSETS_NORMAL_DUST / royaltyFract * TKLROYALTY_DIVISOR - ASSETS_NORMAL_DUST)  // if value paid to seller less than when the royalty is minimum
            royaltyValue = 0LL;

        if (TestAddNormalInputs(mtx, mypk, txfee) == 0LL)  {
            std::cerr << __func__ << " cant add normal inputs" << std::endl;
            return CTransaction();             
        }
        mtx.vin.push_back(CTxIn(asktx.GetHash(), askvout, CScript()));  // spend order tx

        // vout.0 tokens remainder to unspendable cc addr:
        mtx.vout.push_back(TokensV2::MakeTokensCC1vout(AssetsV2::EvalCode(), evalcodeAdd, orig_assetoshis - fill_units, GetUnspendable(cpAssets, NULL)));  // token remainder on cc global addr

        //vout.1 purchased tokens to self token single-eval or dual-eval token+nonfungible cc addr:
        mtx.vout.push_back(TokensV2::MakeTokensCC1vout(evalcodeAdd ? evalcodeAdd : TokensV2::EvalCode(), fill_units, mypk));					
        mtx.vout.push_back(CTxOut(paid_nValue - royaltyValue, CScript() << origpubkey << OP_CHECKSIG));		//vout.2 coins to ask originator's normal addr
        if (royaltyValue > 0)       // note it makes the vout even if roaltyValue is 0
            mtx.vout.push_back(CTxOut(royaltyValue, CScript() << ownerpubkey << OP_CHECKSIG));	// vout.3 royalty to token owner

        if (orig_assetoshis - fill_units > 0) // we dont need the marker if order is filled
            mtx.vout.push_back(TokensV2::MakeCC1vout(AssetsV2::EvalCode(), ASSETS_MARKER_AMOUNT, origpubkey));    //vout.3(4 if royalty) marker to origpubkey (for my tokenorders?)

        uint8_t unspendableAssetsPrivkey[32];
        CPubKey unspendableAssetsPk = GetUnspendable(cpAssets, unspendableAssetsPrivkey);

        CCwrapper wrCond(TokensV2::MakeTokensCCcond1(AssetsV2::EvalCode(), evalcodeAdd, unspendableAssetsPk));
        CCAddVintxCond(cpAssets, wrCond, unspendableAssetsPrivkey);

        if (!TestFinalizeTx(mtx, cpAssets, testKeys[mypk], txfee,
            TokensV2::EncodeTokenOpRet(tokenid, { mypk }, 
                { AssetsV2::EncodeAssetOpRet('S', zeroid, unit_price, origpubkey) } ))) {
            std::cerr << __func__ << " cant finalise tx" << std::endl;
            return CTransaction(); 
        }
        data.pushKV("ownerpubkey", HexStr(ownerpubkey));      
        data.pushKV("origpubkey", HexStr(origpubkey));      
        data.pushKV("unit_price", unit_price);
          
        return mtx;
    }


    static CMutableTransaction MakeTokenV2FillBidTx(struct CCcontract_info *cpTokens, CPubKey mypk, uint256 tokenid, uint256 bidtxid, CAmount fill_units, CAmount unit_price, UniValue &data)
    {
        CMutableTransaction mtx = CreateNewContextualCMutableTransaction(Params().GetConsensus(), komodo_nextheight());
        struct CCcontract_info *cpAssets, C; 
        //struct CCcontract_info *cpTokens, tokensC;

        const int32_t bidvout = ASSETS_GLOBALADDR_VOUT; 
        CAmount paid_amount;
        //CAmount bid_amount;
        CAmount orig_units;
        //CAmount unit_price = 0;
        //CAmount fill_units = 2;
        CAmount txfee = 10000;
        uint256 assetidOpret;
        CAmount paid_unit_price = 0;  // not set

        cpAssets = CCinit(&C, AssetsV2::EvalCode());   
        //cpTokens = CCinit(&tokensC, TokensV2::EvalCode());  

        CTransaction bidtx;
        uint256 hashBlock;
        std::cerr << __func__ << " call to GetTxUnconfirmedOpt " << bidtxid.GetHex() << std::endl;
        if (!GetTxUnconfirmedOpt(&eval, bidtxid, bidtx, hashBlock)) {
            std::cerr << __func__ << " could not load bidtx" << std::endl;
            return CTransaction();
        }

        TokenDataTuple tokenData;
        vuint8_t vextraData;
        uint8_t evalcodeAdd = 0;
        int64_t royaltyFract = 0;  // royaltyFract is N in N/1000 fraction
        if (!GetTokenData<TokensV2>(&eval, tokenid, tokenData, vextraData)) {
            std::cerr << __func__ << " cant get token data" << std::endl;
            return CTransaction();
        }
        if (vextraData.size() > 0)  {
            evalcodeAdd = vextraData.begin()[0];
            GetTokelDataAsInt64(vextraData, TKLPROP_ROYALTY, royaltyFract);
            if (royaltyFract > TKLROYALTY_DIVISOR-1)
                royaltyFract = TKLROYALTY_DIVISOR-1; // royalty upper limit
        }
        vuint8_t ownerpubkey = std::get<0>(tokenData);

        vuint8_t origpubkey;
        std::cerr << __func__ << " bidtx=" << bidtx.GetHash().GetHex() << " " << HexStr(E_MARSHAL(ss << bidtx)) << " vouts=" << bidtx.vout.size() << std::endl;

        CAmount bid_amount = bidtx.vout[bidvout].nValue;
        if (GetOrderParams<AssetsV2>(origpubkey, unit_price, assetidOpret, bidtx) == 0) { // get orig pk, orig value
            std::cerr << __func__ << " cant get order data" << std::endl;
            return CTransaction();
        }
        if (unit_price == 0)  {
            std::cerr << __func__ << " zero unit_price" << std::endl;
            return CTransaction();            
        }

        orig_units = bid_amount / unit_price;

        if (paid_unit_price <= 0LL)
            paid_unit_price = unit_price;

        if (TestAddNormalInputs(mtx, mypk, txfee) == 0LL) {
            std::cerr << __func__ << " cant add normal inputs" << std::endl;
            return CTransaction();                  
        }
        CAmount tokenInputs = TestAddTokenInputs(mtx, mypk, tokenid, fill_units);
        if (tokenInputs == 0LL) {
            std::cerr << __func__ << " cant add token inputs" << std::endl;
            return CTransaction();                  
        }  
        //mtx.vin.push_back(CTxIn(tokenid, 1, CScript()));  // spend token tx
        mtx.vin.push_back(CTxIn(bidtx.GetHash(), bidvout, CScript()));  // spend order tx

        if (!SetBidFillamounts(unit_price, paid_amount, bid_amount, fill_units, orig_units, paid_unit_price)) {
            std::cerr << __func__ << " SetBidFillamounts return false" << std::endl;
            return CTransaction();    
        }

        CAmount royaltyValue = royaltyFract > 0 ? paid_amount / TKLROYALTY_DIVISOR * royaltyFract : 0;
        if (royaltyValue <= ASSETS_NORMAL_DUST)
            royaltyValue = 0LL;
        std::cerr << __func__ << " royaltyFract=" << royaltyFract << " royaltyValue=" << royaltyValue << std::endl;
        CAmount tokensChange = tokenInputs - fill_units;

        uint8_t unspendableAssetsPrivkey[32];
        CPubKey unspendableAssetsPk = GetUnspendable(cpAssets, unspendableAssetsPrivkey);

        if (orig_units - fill_units > 0 || bid_amount - paid_amount <= ASSETS_NORMAL_DUST) { // bidder has coins for more tokens or only dust is sent back to global address
            mtx.vout.push_back(TokensV2::MakeCC1vout(AssetsV2::EvalCode(), bid_amount - paid_amount, unspendableAssetsPk));     // vout0 coins remainder or the dust is sent back to cc global addr
            if (orig_units - fill_units == 0)
                std::cerr << __func__ << " dust detected (bid_amount - paid_amount)=" << (bid_amount - paid_amount) << std::endl;
        }
        else
            mtx.vout.push_back(CTxOut(bid_amount - paid_amount, CScript() << ParseHex(HexStr(origpubkey)) << OP_CHECKSIG));     // vout0 if no more tokens to buy, send the remainder to originator
        mtx.vout.push_back(CTxOut(paid_amount - royaltyValue, CScript() << ParseHex(HexStr(mypk)) << OP_CHECKSIG));	// vout1 coins to mypk normal 
        if (royaltyValue > 0)   // note it makes vout even if roaltyValue is 0
            mtx.vout.push_back(CTxOut(royaltyValue, CScript() << ParseHex(HexStr(ownerpubkey)) << OP_CHECKSIG));  // vout2 trade royalty to token owner
        mtx.vout.push_back(TokensV2::MakeTokensCC1vout(evalcodeAdd ? evalcodeAdd : TokensV2::EvalCode(), fill_units, pubkey2pk(origpubkey)));	  // vout2(3) single-eval tokens sent to the originator
        if (orig_units - fill_units > 0)  // order is not finished yet
            mtx.vout.push_back(TokensV2::MakeCC1vout(AssetsV2::EvalCode(), ASSETS_MARKER_AMOUNT, origpubkey));                    // vout3(4 if royalty) marker to origpubkey

        if (tokensChange != 0LL)
            mtx.vout.push_back(TokensV2::MakeTokensCC1vout(evalcodeAdd ? evalcodeAdd : TokensV2::EvalCode(), tokensChange, mypk));  // change in single-eval tokens

        CMutableTransaction mtx2(mtx);  // copy

        CCwrapper wrCond1(MakeCCcond1(AssetsV2::EvalCode(), unspendableAssetsPk));  // spend coins
        CCAddVintxCond(cpTokens, wrCond1, unspendableAssetsPrivkey);
        
        CCwrapper wrCond2(TokensV2::MakeTokensCCcond1(evalcodeAdd, mypk));  // spend my tokens to fill buy
        CCAddVintxCond(cpTokens, wrCond2, NULL); //NULL indicates to use myprivkey

        if (!TestFinalizeTx(mtx, cpTokens, testKeys[mypk], txfee,
            TokensV2::EncodeTokenOpRet(tokenid, { pubkey2pk(origpubkey) },
                { AssetsV2::EncodeAssetOpRet('B', zeroid, unit_price, origpubkey) }))) {
            std::cerr << __func__ << " could not finalize tx" << std::endl;
            return CTransaction();
        }
        data.pushKV("ownerpubkey", HexStr(ownerpubkey));      
        data.pushKV("origpubkey", HexStr(origpubkey));
        data.pushKV("unit_price", unit_price);

        return mtx;
    }


    static CMutableTransaction MakeTokenV2CancelAskTx(struct CCcontract_info *cpAssets, uint256 tokenid, uint256 asktxid, UniValue &data)
    {
        CMutableTransaction mtx = CreateNewContextualCMutableTransaction(Params().GetConsensus(), komodo_nextheight());

        CTransaction asktx;
        uint256 hashBlock;
        if (!GetTxUnconfirmedOpt(&eval, asktxid, asktx, hashBlock)) {
            std::cerr << __func__ << " could not load asktx" << std::endl;
            return CTransaction();
        }
        CAmount txfee = 10000;
        CAmount askamount = asktx.vout[ASSETS_GLOBALADDR_VOUT].nValue;

        TokenDataTuple tokenData;
        vuint8_t vextraData;
        if (!GetTokenData<TokensV2>(&eval, tokenid, tokenData, vextraData)) {
            std::cerr << __func__ << " could not load token data" << std::endl;
            return CTransaction();
        }
        uint8_t evalcodeAdd = 0;
        if (vextraData.size() > 0)
            evalcodeAdd = vextraData[0];
        vuint8_t ownerpubkey = std::get<0>(tokenData);

        if (TestAddNormalInputs(mtx, ownerpubkey, txfee) == 0LL)  {
            std::cerr << __func__ << " could not add normal inputs" << std::endl;
            return CTransaction();
        }
        mtx.vin.push_back(CTxIn(asktxid, ASSETS_GLOBALADDR_VOUT, CScript()));
        
        uint8_t dummyEvalCode; 
        uint256 dummyAssetid, dummyAssetid2; 
        int64_t dummyPrice; 
        std::vector<uint8_t> dummyOrigpubkey;
        uint8_t funcid = AssetsV2::DecodeAssetTokenOpRet(asktx.vout.back().scriptPubKey, dummyEvalCode, dummyAssetid, dummyAssetid2, dummyPrice, dummyOrigpubkey);
        if (funcid == 's' && asktx.vout.size() > 1)
            mtx.vin.push_back(CTxIn(asktxid, 1, CScript()));		// spend marker if funcid='s'
        else if (funcid == 'S' && asktx.vout.size() > 3)
            mtx.vin.push_back(CTxIn(asktxid, 3, CScript()));		// spend marker if funcid='S'
        else {
            std::cerr << __func__ << "invalid ask tx" << std::endl;
            return CTransaction();
        }

        mtx.vout.push_back(TokensV2::MakeTokensCC1vout(cpAssets->evalcodeAdd ? cpAssets->evalcodeAdd : TokensV2::EvalCode(), askamount, ownerpubkey));	// one-eval token vout

        // init assets 'unspendable' privkey and pubkey
        uint8_t unspendableAssetsPrivkey[32];
        CPubKey unspendableAssetsPk = GetUnspendable(cpAssets, unspendableAssetsPrivkey);

        CCwrapper wrCond(TokensV2::MakeTokensCCcond1(AssetsV2::EvalCode(), cpAssets->evalcodeAdd, unspendableAssetsPk));
        CCAddVintxCond(cpAssets, wrCond, unspendableAssetsPrivkey);

        if (!TestFinalizeTx(mtx, cpAssets, testKeys[ownerpubkey], txfee,
            TokensV2::EncodeTokenOpRet(tokenid, { ownerpubkey },
                { AssetsV2::EncodeAssetOpRet('x', zeroid, 0, vuint8_t(ownerpubkey.begin(), ownerpubkey.end())) } ))) {
            std::cerr << __func__ << " could not finalize tx" << std::endl;
            return CTransaction();
        }
        
        data.pushKV("ownerpubkey", HexStr(ownerpubkey));
        data.pushKV("askamount", askamount);

        return mtx;
    }

    static CMutableTransaction MakeTokenV2CancelBidTx(struct CCcontract_info *cpAssets, uint256 tokenid, uint256 bidtxid, UniValue &data)
    {
        CMutableTransaction mtx = CreateNewContextualCMutableTransaction(Params().GetConsensus(), komodo_nextheight());

        CTransaction bidtx;
        uint256 hashBlock;
        if (!GetTxUnconfirmedOpt(&eval, bidtxid, bidtx, hashBlock)) {
            std::cerr << __func__ << " could not load bidtx" << std::endl;
            return CTransaction();
        }
        CAmount txfee = 10000;
        CAmount bidamount = bidtx.vout[ASSETS_GLOBALADDR_VOUT].nValue;

        TokenDataTuple tokenData;
        vuint8_t vextraData;
        if (!GetTokenData<TokensV2>(&eval, tokenid, tokenData, vextraData)) {
            std::cerr << __func__ << " could not load token data" << std::endl;
            return CTransaction();
        }


        uint8_t evalcodeAdd = 0;
        if (vextraData.size() > 0)
            evalcodeAdd = vextraData[0];
        vuint8_t ownerpubkey = std::get<0>(tokenData);

        if (TestAddNormalInputs(mtx, ownerpubkey, txfee) == 0LL) {
            std::cerr << __func__ << " could not add normal inputs" << std::endl;
            return CTransaction();
        }
        mtx.vin.push_back(CTxIn(bidtxid, ASSETS_GLOBALADDR_VOUT, CScript()));
        
        uint8_t dummyEvalCode; 
        uint256 dummyAssetid, dummyAssetid2; 
        int64_t dummyPrice; 
        std::vector<uint8_t> dummyOrigpubkey;
        uint8_t funcid = AssetsV2::DecodeAssetTokenOpRet(bidtx.vout.back().scriptPubKey, dummyEvalCode, dummyAssetid, dummyAssetid2, dummyPrice, dummyOrigpubkey);
        if (funcid == 'b' && bidtx.vout.size() > 1)
            mtx.vin.push_back(CTxIn(bidtxid, 1, CScript()));		// spend marker if funcid='b'
        else if (funcid == 'B' && bidtx.vout.size() > 3)
            mtx.vin.push_back(CTxIn(bidtxid, 3, CScript()));		// spend marker if funcid='B'
        else {
            std::cerr << __func__ << "invalid bid tx" << std::endl;
            return CTransaction();        }

        if (bidamount > ASSETS_NORMAL_DUST)  
            mtx.vout.push_back(CTxOut(bidamount, CScript() << ParseHex(HexStr(ownerpubkey)) << OP_CHECKSIG));
        else {
            // send dust back to global addr
            mtx.vout.push_back(TokensV2::MakeCC1vout(AssetsV2::EvalCode(), bidamount, GetUnspendable(cpAssets, NULL)));
            std::cerr << __func__ << " dust detected bidamount=" << bidamount << std::endl;
        }

        // init assets 'unspendable' privkey and pubkey
        uint8_t unspendableAssetsPrivkey[32];
        CPubKey unspendableAssetsPk = GetUnspendable(cpAssets, unspendableAssetsPrivkey);

        // sign, add change, add opreturn
        if (!TestFinalizeTx(mtx, cpAssets, testKeys[ownerpubkey], txfee,
                            TokensV2::EncodeTokenOpRet(tokenid, { ownerpubkey },
                                { AssetsV2::EncodeAssetOpRet('o', zeroid, 0, vuint8_t(ownerpubkey.begin(), ownerpubkey.end())) } ))) {
            std::cerr << __func__ << " could not finalise tx" << std::endl;
            return CTransaction();
        }

        data.pushKV("ownerpubkey", HexStr(ownerpubkey));
        data.pushKV("bidamount", bidamount);
        return mtx;
    }


};


// --------------------------------------------------
// assets cc tests:

TEST_F(TestAssetsCC, tokenv2ask)
{
	struct CCcontract_info *cpTokens, tokensC;
    cpTokens = CCinit(&tokensC, TokensV2::EvalCode());  
    CAmount numtokens = 2LL;
    CPubKey mypk = pk1;
    uint256 mytokenid = tokenid1;
    uint8_t evalcodeAdd = 0;

    struct CCcontract_info *cpAssets, C; 
    cpAssets = CCinit(&C, AssetsV2::EvalCode());   // NOTE: assets here!
    CPubKey unspendableAssetsPubkey = GetUnspendable(cpAssets, 0);    

    CMutableTransaction mtx = MakeTokenV2AskTx(cpTokens, mypk, mytokenid, numtokens, 501); // price more than dust
    ASSERT_FALSE(CTransaction(mtx).IsNull());

    vuint8_t origpubkey;
    CAmount unit_price;
    uint256 assetidOpret;
    uint8_t funcid = GetOrderParams<AssetsV2>(origpubkey, unit_price, assetidOpret, mtx);

    // test: valid tokenv2ask
    EXPECT_TRUE(TestRunCCEval(mtx));

    {
        // test: invalid unit_price == 0
        CMutableTransaction mtx1(mtx);
        mtx1.vout.pop_back();
        ASSERT_TRUE(TestFinalizeTx(mtx1, cpTokens, testKeys[mypk], 10000,
            TokensV2::EncodeTokenOpRet(assetidOpret, { unspendableAssetsPubkey },     
                { AssetsV2::EncodeAssetOpRet('s', zeroid, 0, vuint8_t(origpubkey.begin(), origpubkey.end())) })));
        EXPECT_FALSE(TestRunCCEval(mtx1));
    }
    {
        // test: invalid token dest pubkey (not global)
        CMutableTransaction mtx1(mtx);
        mtx1.vout.pop_back();
        ASSERT_TRUE(TestFinalizeTx(mtx1, cpTokens, testKeys[mypk], 10000,
            TokensV2::EncodeTokenOpRet(assetidOpret, { pkunused }, // bad pk instead of global    
                { AssetsV2::EncodeAssetOpRet('s', zeroid, unit_price, vuint8_t(origpubkey.begin(), origpubkey.end())) })));
        EXPECT_TRUE(TestRunCCEval(mtx1));  // == true as pk in token opreturn not used in tokens v2
    }
    {
        // test: bad origpk in opreturn (not the tx signer)
        CMutableTransaction mtx1(mtx);
        mtx1.vout.pop_back();
        ASSERT_TRUE(TestFinalizeTx(mtx1, cpTokens, testKeys[mypk], 10000,
            TokensV2::EncodeTokenOpRet(assetidOpret, { unspendableAssetsPubkey },     
                { AssetsV2::EncodeAssetOpRet('s', zeroid, unit_price, vuint8_t(pkunused.begin(), pkunused.end())) })));  // not matched origpk (should be pk1)
        EXPECT_FALSE(TestRunCCEval(mtx1));
    }
    {
        // test: only one opreturn supported
        CMutableTransaction mtx1(mtx);
        // mtx1.vout.pop_back();
        ASSERT_TRUE(TestFinalizeTx(mtx1, cpTokens, testKeys[mypk], 10000,
            TokensV2::EncodeTokenOpRet(assetidOpret, { unspendableAssetsPubkey },     
                { AssetsV2::EncodeAssetOpRet('s', zeroid, unit_price, vuint8_t(origpubkey.begin(), origpubkey.end())) })));
        EXPECT_FALSE(TestRunCCEval(mtx1));
    }
    {
        // test: send to non-global assets destination
        CMutableTransaction mtx1(mtx);
        mtx1.vout[0] = TokensV2::MakeTokensCC1vout(AssetsV2::EvalCode(), evalcodeAdd, numtokens, pkunused);
        mtx1.vout.pop_back();
        ASSERT_TRUE(TestFinalizeTx(mtx1, cpTokens, testKeys[mypk], 10000,
            TokensV2::EncodeTokenOpRet(assetidOpret, { unspendableAssetsPubkey },   
                { AssetsV2::EncodeAssetOpRet('s', zeroid, unit_price, vuint8_t(origpubkey.begin(), origpubkey.end())) })));
        EXPECT_FALSE(TestRunCCEval(mtx1));  // == true as pk in token opreturn not used in tokens v2
    }
    {
        // test: add extra global addr vout
        CMutableTransaction mtx1(mtx);
        // add two vouts with numtokens/2 (to have token balance correct). Assets cc should fail however:
        mtx1.vout[0] = TokensV2::MakeTokensCC1vout(AssetsV2::EvalCode(), evalcodeAdd, numtokens/2, unspendableAssetsPubkey);
        mtx1.vout.insert(mtx1.vout.begin(), TokensV2::MakeTokensCC1vout(AssetsV2::EvalCode(), evalcodeAdd, numtokens/2, unspendableAssetsPubkey));
        mtx1.vout.pop_back(); // remove old opreturn
        ASSERT_TRUE(TestFinalizeTx(mtx1, cpTokens, testKeys[mypk], 10000,
            TokensV2::EncodeTokenOpRet(assetidOpret, { unspendableAssetsPubkey },   
                { AssetsV2::EncodeAssetOpRet('s', zeroid, unit_price, vuint8_t(origpubkey.begin(), origpubkey.end())) })));
        EXPECT_FALSE(TestRunCCEval(mtx1));  // == true as pk in token opreturn not used in tokens v2
    }
    {
        // test: add extra global addr null vout
        CMutableTransaction mtx1(mtx);
        // add two vouts with numtokens/2 (to have token balance correct). Assets cc should fail however:
        mtx1.vout[0] = TokensV2::MakeTokensCC1vout(AssetsV2::EvalCode(), evalcodeAdd, numtokens, unspendableAssetsPubkey);
        mtx1.vout.insert(mtx1.vout.begin(), TokensV2::MakeTokensCC1vout(AssetsV2::EvalCode(), evalcodeAdd, 0, unspendableAssetsPubkey));
        mtx1.vout.pop_back(); // remove old opreturn
        ASSERT_TRUE(TestFinalizeTx(mtx1, cpTokens, testKeys[mypk], 10000,
            TokensV2::EncodeTokenOpRet(assetidOpret, { unspendableAssetsPubkey },   
                { AssetsV2::EncodeAssetOpRet('s', zeroid, unit_price, vuint8_t(origpubkey.begin(), origpubkey.end())) })));
        EXPECT_FALSE(TestRunCCEval(mtx1));  // == true as pk in token opreturn not used in tokens v2
    }
    {
        // test: add extra marker
        CMutableTransaction mtx1(mtx);
        ASSERT_TRUE(mtx.vout.size() > 2);
        mtx1.vout.insert(mtx1.vout.begin()+1, mtx1.vout[1]);  // copy marker
        mtx1.vout.pop_back(); // remove old opreturn
        ASSERT_TRUE(TestFinalizeTx(mtx1, cpTokens, testKeys[mypk], 10000,
            TokensV2::EncodeTokenOpRet(assetidOpret, { unspendableAssetsPubkey },   
                { AssetsV2::EncodeAssetOpRet('s', zeroid, unit_price, vuint8_t(origpubkey.begin(), origpubkey.end())) })));
        EXPECT_FALSE(TestRunCCEval(mtx1));  // == true as pk in token opreturn not used in tokens v2
    }
}

TEST_F(TestAssetsCC, tokenv2bid)
{
    CAmount txfee = 10000;
    struct CCcontract_info *cpAssets, C; 
    cpAssets = CCinit(&C, AssetsV2::EvalCode());   // NOTE: assets here!
    CPubKey unspendableAssetsPubkey = GetUnspendable(cpAssets, 0);    

    CMutableTransaction mtx = MakeTokenV2BidTx(cpAssets, pk2, tokenid1, 2, 501); // price more than dust
    ASSERT_FALSE(CTransaction(mtx).IsNull());

    // test: valid tokenv2bid
    EXPECT_TRUE(TestRunCCEval(mtx));

    {
        CMutableTransaction mtx1(mtx);

        // test: too low bid amount < unit_price
        mtx1.vout[0] = TokensV2::MakeCC1vout(AssetsV2::EvalCode(), 9999, unspendableAssetsPubkey);
        mtx1.vout.pop_back();

        ASSERT_TRUE(TestFinalizeTx(mtx1, cpAssets, testKeys[pk2], txfee,
            TokensV2::EncodeTokenOpRet(tokenid1, {},     
                { AssetsV2::EncodeAssetOpRet('b', zeroid, 10000, vuint8_t(pk2.begin(), pk2.end())) })));   
        EXPECT_FALSE(TestRunCCEval(mtx1));    // must fail
    }
}


TEST_F(TestAssetsCC, tokenv2fillask)
{
    UniValue data(UniValue::VOBJ);
    struct CCcontract_info *cpAssets, C; 
    cpAssets = CCinit(&C, AssetsV2::EvalCode());
    CMutableTransaction mtx = MakeTokenV2FillAskTx(cpAssets, pk2, tokenid1, txask1.GetHash(), 2, 0, data);
    ASSERT_FALSE(CTransaction(mtx).IsNull());

    // test: valid tokenv2fillask
    EXPECT_TRUE(TestRunCCEval(mtx));

    //vuint8_t ownerpubkey = ParseHex(data["ownerpubkey"].getValStr());

    // test: spend invalid tokenid
    {
        CMutableTransaction mtx1(mtx);
        mtx1.vin.back() = CTxIn(txask1.GetHash(), ASSETS_GLOBALADDR_VOUT, CScript());  // spend order tx
        vuint8_t origpubkey;
        CAmount unit_price;
        uint256 assetidOpret;
        uint8_t funcid = GetOrderParams<AssetsV2>(origpubkey, unit_price, assetidOpret, txask1);
        ASSERT_TRUE(funcid != 0);
        mtx1.vout.pop_back();
        ASSERT_TRUE(TestFinalizeTx(mtx1, cpAssets, testKeys[pk2], 10000,
            TokensV2::EncodeTokenOpRet(tokenidUnused, { pk2 }, 
                { AssetsV2::EncodeAssetOpRet('S', zeroid, unit_price, origpubkey) } )));

        EXPECT_FALSE(TestRunCCEval(mtx1));    // must fail
    }
    {
        // test: use opposite funcid 
        CMutableTransaction mtx1(mtx);

        CAmount txfee = 10000;
        vuint8_t origpubkey;
        CAmount unit_price;
        uint256 assetidOpret;
        uint8_t funcid = GetOrderParams<AssetsV2>(origpubkey, unit_price, assetidOpret, txbid1); 

        mtx1.vout.pop_back(); // remove opret to replace it in TestFinalizeTx
        ASSERT_TRUE(TestFinalizeTx(mtx1, cpAssets, testKeys[pk2], txfee,
            TokensV2::EncodeTokenOpRet(assetidOpret, { pubkey2pk(origpubkey) },
                { AssetsV2::EncodeAssetOpRet('B', zeroid, unit_price, origpubkey) }))); // 'S' -> 'B'

        EXPECT_FALSE(TestRunCCEval(mtx1));  // must fail: incorrect funcid
    }
    // test: spend yet another order 
    {
        CMutableTransaction mtx2(mtx);
        mtx2.vin.push_back( CTxIn(txask2.GetHash(), ASSETS_GLOBALADDR_VOUT, CScript()) );  // spend yet another order tx
        vuint8_t origpubkey;
        CAmount unit_price;
        uint256 assetidOpret;
        uint8_t funcid = GetOrderParams<AssetsV2>(origpubkey, unit_price, assetidOpret, txask2); 
        ASSERT_TRUE(funcid != 0);
        mtx2.vout.pop_back();
        ASSERT_TRUE(TestFinalizeTx(mtx2, cpAssets, testKeys[pk2], 10000,
            TokensV2::EncodeTokenOpRet(assetidOpret, { pk2 }, 
                { AssetsV2::EncodeAssetOpRet('S', zeroid, unit_price, origpubkey) } )));

        EXPECT_FALSE(TestRunCCEval(mtx2));    // must fail
    }
    {
        // test: change unit_price in opret
        CMutableTransaction mtx2(mtx);
        vuint8_t origpubkey;
        CAmount unit_price;
        uint256 assetidOpret;
        uint8_t funcid = GetOrderParams<AssetsV2>(origpubkey, unit_price, assetidOpret, txask1); // get orig pk, orig value
        ASSERT_TRUE(funcid != 0);
        mtx2.vout.pop_back();
        ASSERT_TRUE(TestFinalizeTx(mtx2, cpAssets, testKeys[pk2], 10000,
            TokensV2::EncodeTokenOpRet(assetidOpret, { pk2 }, 
                { AssetsV2::EncodeAssetOpRet('S', zeroid, unit_price+1, origpubkey) } )));

        EXPECT_FALSE(TestRunCCEval(mtx2));    // must fail
    }
    {
        // test: changed origpk in assets opreturn
        CMutableTransaction mtx1(mtx);
        vuint8_t origpubkey;
        CAmount unit_price;
        uint256 assetidOpret;
        uint8_t funcid = GetOrderParams<AssetsV2>(origpubkey, unit_price, assetidOpret, txask1); // get orig pk, orig value
        mtx1.vout.pop_back();
        ASSERT_TRUE(TestFinalizeTx(mtx1, cpAssets, testKeys[pk2], 10000,
            TokensV2::EncodeTokenOpRet(tokenid1, { pk2 },     
                { AssetsV2::EncodeAssetOpRet('S', zeroid, unit_price, vuint8_t(pkunused.begin(), pkunused.end())) })));  // not matched origpk (should be pk1)
        EXPECT_FALSE(TestRunCCEval(mtx1));
    }
}


TEST_F(TestAssetsCC, tokenv2fillbid)
{
    UniValue data(UniValue::VOBJ);
    struct CCcontract_info *cpTokens, C; 
    cpTokens = CCinit(&C, TokensV2::EvalCode());  

    CMutableTransaction mtx = MakeTokenV2FillBidTx(cpTokens, pk2, tokenid2, txbid1.GetHash(), 2, 0, data);  
    ASSERT_FALSE(CTransaction(mtx).IsNull());

    // test: valid tokenv2fillbid
    EXPECT_TRUE(TestRunCCEval(mtx));
    {
        // test: fill with another tokenid 
        CMutableTransaction mtx1(mtx);

        CAmount txfee = 10000;
        vuint8_t ownerpubkey = ParseHex(data["ownerpubkey"].getValStr());
        vuint8_t origpubkey = ParseHex(data["origpubkey"].getValStr());
        CAmount unit_price = data["unit_price"].get_int64();

        mtx1.vin[1] = CTxIn(txtokencreate3.GetHash(), 1, CScript());  // spend other tokenid3
        mtx1.vout.pop_back(); // remove opret to replace it in TestFinalizeTx
        ASSERT_TRUE(TestFinalizeTx(mtx1, cpTokens, testKeys[pk2], txfee,
            TokensV2::EncodeTokenOpRet(tokenid3, { pubkey2pk(origpubkey) },
                { AssetsV2::EncodeAssetOpRet('B', zeroid, unit_price, origpubkey) }))); 

        EXPECT_FALSE(TestRunCCEval(mtx1));  // must fail: can't fill with another tokenid3
    }
    {
        // test: use opposite funcid 
        CMutableTransaction mtx1(mtx);

        CAmount txfee = 10000;
        vuint8_t origpubkey;
        CAmount unit_price;
        uint256 assetidOpret;
        uint8_t funcid = GetOrderParams<AssetsV2>(origpubkey, unit_price, assetidOpret, txbid1); // get orig pk, orig value

        mtx1.vout.pop_back(); // remove opret to replace it in TestFinalizeTx
        ASSERT_TRUE(TestFinalizeTx(mtx1, cpTokens, testKeys[pk2], txfee,
            TokensV2::EncodeTokenOpRet(assetidOpret, { pubkey2pk(origpubkey) },
                { AssetsV2::EncodeAssetOpRet('S', zeroid, unit_price, origpubkey) }))); // 'B' -> 'S'

        EXPECT_FALSE(TestRunCCEval(mtx1));  // must fail: incorrect funcid
    }
    {
        // test: changed origpk in opreturn
        CMutableTransaction mtx1(mtx);
        vuint8_t origpubkey;
        CAmount unit_price;
        uint256 assetidOpret;
        uint8_t funcid = GetOrderParams<AssetsV2>(origpubkey, unit_price, assetidOpret, txbid1); // get orig pk, orig value
        mtx1.vout.pop_back();
        ASSERT_TRUE(TestFinalizeTx(mtx1, cpTokens, testKeys[pk2], 10000,
            TokensV2::EncodeTokenOpRet(assetidOpret, { pk2 },     
                { AssetsV2::EncodeAssetOpRet('B', zeroid, unit_price, vuint8_t(pkunused.begin(), pkunused.end())) })));  // not matched origpk (should be pk1)
        EXPECT_FALSE(TestRunCCEval(mtx1));
    }
}

TEST_F(TestAssetsCC, tokenv2fillbid_royalty)
{
    for(int r = 0; r < 1000; r += 100)
    {
        UniValue data(UniValue::VOBJ); // some data returned from MakeTokenV2FillBidTx
        UniValue tokeldata(UniValue::VOBJ);
        struct CCcontract_info *cpTokens, tokensC; 
        cpTokens = CCinit(&tokensC, TokensV2::EvalCode());  
        struct CCcontract_info *cpAssets, assetsC; 
        cpAssets = CCinit(&assetsC, AssetsV2::EvalCode());  

        tokeldata.pushKV("royalty", r);
        CTransaction mytxtokencreate = MakeTokenV2CreateTx(pk1, 1, tokeldata);
        uint256 mytokenid = mytxtokencreate.GetHash();
        eval.AddTx(mytxtokencreate);

        CTransaction mytxbid = MakeTokenV2BidTx(cpAssets, pk2, mytokenid, 1, ASSETS_NORMAL_DUST*2+1);
        eval.AddTx(mytxbid);

        CMutableTransaction mytxfill = MakeTokenV2FillBidTx(cpTokens, pk1, mytokenid, mytxbid.GetHash(), 1, 0, data);  
        ASSERT_FALSE(CTransaction(mytxfill).IsNull());

        // test: valid tokenv2fillbid
        EXPECT_TRUE(TestRunCCEval(mytxfill));
    }
}

TEST_F(TestAssetsCC, tokenv2fillask_royalty)
{
    for(int r = 0; r < 1000; r += 100)
    {
        UniValue data(UniValue::VOBJ); // some data returned from MakeTokenV2FillBidTx
        UniValue tokeldata(UniValue::VOBJ);
        struct CCcontract_info *cpTokens, tokensC; 
        cpTokens = CCinit(&tokensC, TokensV2::EvalCode());  
        struct CCcontract_info *cpAssets, assetsC; 
        cpAssets = CCinit(&assetsC, AssetsV2::EvalCode());  

        tokeldata.pushKV("royalty", r);
        CTransaction mytxtokencreate = MakeTokenV2CreateTx(pk1, 1, tokeldata);
        uint256 mytokenid = mytxtokencreate.GetHash();
        eval.AddTx(mytxtokencreate);

        CTransaction mytxask = MakeTokenV2AskTx(cpTokens, pk1, mytokenid, 1, ASSETS_NORMAL_DUST*2+1);
        eval.AddTx(mytxask);

        CMutableTransaction mytxfill = MakeTokenV2FillAskTx(cpAssets, pk2, mytokenid, mytxask.GetHash(), 1, 0, data);  
        ASSERT_FALSE(CTransaction(mytxfill).IsNull());

        // test: valid tokenv2fillbid
        EXPECT_TRUE(TestRunCCEval(mytxfill));
    }
}


TEST_F(TestAssetsCC, tokenv2cancelask)
{
	struct CCcontract_info *cpAssets, C; 
	struct CCcontract_info *cpTokens, tokensC;

    CAmount txfee = 10000;

    cpAssets = CCinit(&C, AssetsV2::EvalCode());   
    cpTokens = CCinit(&tokensC, TokensV2::EvalCode());  

    for (CTransaction vintx : std::vector<CTransaction>{ txask1 })  // TODO add more txasks
    {
        UniValue data(UniValue::VOBJ);
        uint256 asktxid = vintx.GetHash();

        struct CCcontract_info *cpAssets, C; 
        cpAssets = CCinit(&C, AssetsV2::EvalCode());
        CMutableTransaction mtx = MakeTokenV2CancelAskTx(cpAssets, tokenid1, asktxid, data);
        ASSERT_FALSE(CTransaction(mtx).IsNull());

        vuint8_t ownerpubkey = ParseHex(data["ownerpubkey"].getValStr());
        CAmount askamount = data["askamount"].get_int64();

        // test: valid tokenv2cancelask
        EXPECT_TRUE(TestRunCCEval(mtx));
        {
            // test: invalid pk in assets opreturn
            CMutableTransaction mtx2(mtx);
            mtx2.vout.pop_back();
            mtx2.vout.push_back(CTxOut(0, TokensV2::EncodeTokenOpRet(tokenid1, { ownerpubkey },
                                    { AssetsV2::EncodeAssetOpRet('x', zeroid, 0, vuint8_t(pk2.begin(), pk2.end())) } )));
            EXPECT_TRUE(TestRunCCEval(mtx2)); // pk in opret not checked
        }
        {
            // test: another pk in token opret
            CMutableTransaction mtx3(mtx);
            mtx3.vout.pop_back();
            mtx3.vout.push_back(CTxOut(0, TokensV2::EncodeTokenOpRet(tokenid1, { pkunused },
                                    { AssetsV2::EncodeAssetOpRet('x', zeroid, 0, vuint8_t(ownerpubkey.begin(), ownerpubkey.end())) } )));
            EXPECT_TRUE(TestRunCCEval(mtx3)); // pk in opret is not checked
        }
        {
            // test: invalid pk where to funds sent
            CMutableTransaction mtx4(mtx);
            mtx4.vout[0] = TokensV2::MakeTokensCC1vout(cpAssets->evalcodeAdd ? cpAssets->evalcodeAdd : TokensV2::EvalCode(), askamount, pk2);
            EXPECT_FALSE(TestRunCCEval(mtx4)); // must fail
        }
        {
            // test: invalid tokenid in token opret
            CMutableTransaction mtx5(mtx);
            mtx5.vout.pop_back();
            mtx5.vout.push_back(CTxOut(0, TokensV2::EncodeTokenOpRet(tokenidUnused, { ownerpubkey },
                                    { AssetsV2::EncodeAssetOpRet('x', zeroid, 0, vuint8_t(ownerpubkey.begin(), ownerpubkey.end())) } )));
            EXPECT_FALSE(TestRunCCEval(mtx5)); // must fail: cant send to another tokenid
        }
        {
            // test: invalid funcid in token opret
            CMutableTransaction mtx6(mtx);
            mtx6.vout.pop_back();
            mtx6.vout.push_back(CTxOut(0, TokensV2::EncodeTokenOpRet(tokenid1, { ownerpubkey },
                                    { AssetsV2::EncodeAssetOpRet('o', zeroid, 0, vuint8_t(ownerpubkey.begin(), ownerpubkey.end())) } )));
            EXPECT_FALSE(TestRunCCEval(mtx6)); // must fail
        }
    }
}

TEST_F(TestAssetsCC, tokenv2cancelbid)
{
	struct CCcontract_info *cpAssets, C; 
	struct CCcontract_info *cpTokens, tokensC;
    CAmount txfee = 10000;

    cpAssets = CCinit(&C, AssetsV2::EvalCode());   
    cpTokens = CCinit(&tokensC, TokensV2::EvalCode());  

    for (CTransaction vintx : std::vector<CTransaction>{ txbid1 })  // TODO add more txbid
    {
        UniValue data(UniValue::VOBJ);
        uint256 bidtxid = vintx.GetHash();

        struct CCcontract_info *cpAssets, C; 
        cpAssets = CCinit(&C, AssetsV2::EvalCode());
        CMutableTransaction mtx = MakeTokenV2CancelBidTx(cpAssets, tokenid2, bidtxid, data);
        ASSERT_FALSE(CTransaction(mtx).IsNull());

        vuint8_t ownerpubkey = ParseHex(data["ownerpubkey"].getValStr());
        CAmount bidamount = data["bidamount"].get_int64();

        // test: valid tokenv2cancelbid
        EXPECT_TRUE(TestRunCCEval(mtx));
        {
            // test: invalid pk in assets opreturn
            CMutableTransaction mtx2(mtx);
            mtx2.vout.pop_back();
            mtx2.vout.push_back(CTxOut(0, TokensV2::EncodeTokenOpRet(tokenid2, { ownerpubkey },
                                    { AssetsV2::EncodeAssetOpRet('o', zeroid, 0, vuint8_t(pk1.begin(), pk1.end())) } )));
            EXPECT_TRUE(TestRunCCEval(mtx2)); // pk in opret is not checked
        }
        {
            // test: invalid pk in token opret
            CMutableTransaction mtx3(mtx);
            mtx3.vout.pop_back();
            mtx3.vout.push_back(CTxOut(0, TokensV2::EncodeTokenOpRet(tokenid2, { pkunused },
                                    { AssetsV2::EncodeAssetOpRet('o', zeroid, 0, vuint8_t(ownerpubkey.begin(), ownerpubkey.end())) } )));
            EXPECT_TRUE(TestRunCCEval(mtx3)); // pk in opret is not checked
        }
        {
            // test: invalid pk where to funds sent
            CMutableTransaction mtx4(mtx);
            mtx4.vout[0] = CTxOut(bidamount, CScript() << ParseHex(HexStr(pkunused)) << OP_CHECKSIG);
            EXPECT_FALSE(TestRunCCEval(mtx4)); // must fail as can't send the remainder to another pk
        }
        {
            // test: invalid tokenid in token opret
            CMutableTransaction mtx5(mtx);
            mtx5.vout.pop_back();
            mtx5.vout.push_back(CTxOut(0, TokensV2::EncodeTokenOpRet(tokenidUnused, { ownerpubkey },
                                    { AssetsV2::EncodeAssetOpRet('o', zeroid, 0, vuint8_t(ownerpubkey.begin(), ownerpubkey.end())) } )));
            EXPECT_FALSE(TestRunCCEval(mtx5)); 
        }
        {
            // test: invalid funcid in token opret
            CMutableTransaction mtx6(mtx);
            mtx6.vout.pop_back();
            mtx6.vout.push_back(CTxOut(0, TokensV2::EncodeTokenOpRet(tokenid2, { pk2 },
                                    { AssetsV2::EncodeAssetOpRet('x', zeroid, 0, vuint8_t(ownerpubkey.begin(), ownerpubkey.end())) } )));
            EXPECT_FALSE(TestRunCCEval(mtx6)); 
        }
        {
            // test: send dust to normals - bad test params here
            //CMutableTransaction mtx7(mtx);
            //mtx7.vout[0] = CTxOut(bidamount, CScript() << ParseHex(HexStr(ownerpubkey)) << OP_CHECKSIG);
            //EXPECT_FALSE(TestRunCCEval(mtx7)); // must fail: dust should stay on cc global output
        }
    }
}


/* ---------------------------------------------------------------------------------------------------------------------------------- */
// tokens tests:

TEST_F(TestAssetsCC, tokenv2create)
{
    struct CCcontract_info *cpTokens, C; 
    cpTokens = CCinit(&C, TokensV2::EvalCode()); 
    CMutableTransaction mtx = MakeTokenV2CreateTx(pk1, 10);
    ASSERT_FALSE(CTransaction(mtx).IsNull());

    EXPECT_TRUE(TestRunCCEval(mtx));

    CAmount txfee = 0;
    std::string name = "T2";
    std::string description = "desc2";
    {
        // test: token sent to another pk
        CMutableTransaction mtx1(mtx);
        mtx1.vout[1] = TokensV2::MakeTokensCC1vout(0, 10, pk2); 
        mtx1.vout.pop_back();
        ASSERT_TRUE(TestFinalizeTx(mtx1, cpTokens, testKeys[pk1], txfee,
                            TokensV2::EncodeTokenCreateOpRet(vscript_t(pk1.begin(), pk1.end()), name, description, {  })));
        EXPECT_TRUE(TestRunCCEval(mtx1));  // allow sending created tokens to any pk!!
    }
    {
        // test: invalid pk in opreturn:
        CMutableTransaction mtx2(mtx);
        mtx2.vout.pop_back();
        ASSERT_TRUE(TestFinalizeTx(mtx2, cpTokens, testKeys[pk1], txfee,
                            TokensV2::EncodeTokenCreateOpRet(vscript_t(pk2.begin(), pk2.end()), name, description, {  })));
        EXPECT_FALSE(TestRunCCEval(mtx2)); // must fail
    }
    {
        // test: no token vouts,  sent to normal
        CMutableTransaction mtx3(mtx);
        mtx3.vout[1] = CTxOut(10, CScript() << ParseHex(HexStr(pk1)) << OP_CHECKSIG); 
        mtx3.vout.pop_back();
        ASSERT_TRUE(TestFinalizeTx(mtx3, cpTokens, testKeys[pk1], txfee,
                            TokensV2::EncodeTokenCreateOpRet(vscript_t(pk1.begin(), pk1.end()), name, description, {  })));
        EXPECT_FALSE(TestRunCCEval(mtx3));  // fail for no token cc vouts
    }
    {
        // test: no token vouts,  sent to cc v1
        CMutableTransaction mtx4(mtx);
        mtx4.vout[1] = TokensV1::MakeCC1vout(EVAL_ASSETS, 10, pk2); // sent to cc v1 vout
        mtx4.vout.pop_back();
        ASSERT_TRUE(TestFinalizeTx(mtx4, cpTokens, testKeys[pk1], txfee,
                            TokensV2::EncodeTokenCreateOpRet(vscript_t(pk1.begin(), pk1.end()), name, description, {  })));
        EXPECT_FALSE(TestRunCCEval(mtx4));  // fail for no token cc vouts
    }
    {
        // test: invalid token created with global pk:
        CMutableTransaction mtx5 = CreateNewContextualCMutableTransaction(Params().GetConsensus(), komodo_nextheight());
        
        uint8_t privkeyg[32];
        CPubKey pkg = GetUnspendable(cpTokens, privkeyg);

        mtx5.vin.push_back(CTxIn(txnormalg.GetHash(), 0));
        mtx5.vout.push_back(TokensV2::MakeCC1vout(TokensV2::EvalCode(), TOKENS_MARKER_VALUE, GetUnspendable(cpTokens, NULL)));           
        mtx5.vout.push_back(TokensV2::MakeTokensCC1vout(0, 10, pk1));

        ASSERT_TRUE(TestFinalizeTx(mtx5, cpTokens, privkeyg, txfee,
                            TokensV2::EncodeTokenCreateOpRet(vscript_t(pkg.begin(), pkg.end()), name, description, {  })));
        EXPECT_FALSE(TestRunCCEval(mtx5));  // must fail
    }
}

} /* namespace CCAssetsTests */
