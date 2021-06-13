#include "komodo_algorithms.h"
#include "chainparams.h"
#include "arith_uint256.h"

hash_algorithm hash_algorithm::get_algorithm(const std::string& in)
{
    if (in == "equihash")
        return get_algorithm(HASH_ALGO_EQUIHASH);
    if (in == "verushash")
        return get_algorithm(HASH_ALGO_VERUSHASH);
    if (in == "verushash11")
        return get_algorithm(HASH_ALGO_VERUSHASHV1_1);
    return hash_algorithm();
}

hash_algorithm hash_algorithm::get_algorithm(hash_algo in)
{
    if (in == HASH_ALGO_EQUIHASH)
        return equihash();
    if (in == HASH_ALGO_VERUSHASH)
        return verushash();
    if (in == HASH_ALGO_VERUSHASHV1_1)
        return verushash11();
    return hash_algorithm();
}

arith_uint256 hash_algorithm::GetPoWLimit(const Consensus::Params &in)
{
    return UintToArith256(in.powLimit);
}

void hash_algorithm::SetConsensusValues(CChainParams *in)
{
}

int32_t hash_algorithm::CalculatePercentPoS(int32_t inPct, int32_t m, int32_t n, int32_t goalPct)
{
    return ((inPct * n) + (goalPct * (100-n))) / 100;       
}

void verushash::SetConsensusValues(CChainParams *in)
{
    // this is only good for 60 second blocks with an averaging window of 45. for other parameters, use:
    // nLwmaAjustedWeight = (N+1)/2 * (0.9989^(500/nPowAveragingWindow)) * nPowTargetSpacing
    in->consensus.nLwmaAjustedWeight = 1350;
    in->consensus.nPowAveragingWindow = 45;
    in->consensus.powAlternate = uint256S("00000f0f0f0f0f0f0f0f0f0f0f0f0f0f0f0f0f0f0f0f0f0f0f0f0f0f0f0f0f0f");    
}

arith_uint256 verushash::GetPoWLimit(const Consensus::Params &in)
{
    return UintToArith256(in.powAlternate);
}

int32_t verushash::CalculatePercentPoS(int32_t inPct, int32_t m, int32_t n, int32_t goalPct)
{
    return (inPct * 100) / (m + n);     
}

void verushash11::SetConsensusValues(CChainParams *in)
{
    // this is only good for 60 second blocks with an averaging window of 45. for other parameters, use:
    // nLwmaAjustedWeight = (N+1)/2 * (0.9989^(500/nPowAveragingWindow)) * nPowTargetSpacing
    in->consensus.nLwmaAjustedWeight = 1350;
    in->consensus.nPowAveragingWindow = 45;
    in->consensus.powAlternate = uint256S("0000000f0f0f0f0f0f0f0f0f0f0f0f0f0f0f0f0f0f0f0f0f0f0f0f0f0f0f0f0f");    
}