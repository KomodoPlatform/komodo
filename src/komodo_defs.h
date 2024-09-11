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

#ifndef KOMODO_DEFS_H
#define KOMODO_DEFS_H

#include <map>
#include "arith_uint256.h"
#include "script/script.h"
#include "pubkey.h"
#include "chain.h"
#include "primitives/transaction.h"
#include "threadsafety.h"

#include "komodo_structs.h"
#include "komodo_nk.h"

#define KOMODO_EARLYTXID_HEIGHT 100
//#define ADAPTIVEPOW_CHANGETO_DEFAULTON 1572480000
#define ASSETCHAINS_MINHEIGHT 128
#define ASSETCHAINS_MAX_ERAS 7
#define KOMODO_ELECTION_GAP 2000
#define ROUNDROBIN_DELAY 61
#define KOMODO_ASSETCHAIN_MAXLEN 65
#define KOMODO_LIMITED_NETWORKSIZE 4
#define IGUANA_MAXSCRIPTSIZE 10001
#define KOMODO_MAXMEMPOOLTIME 3600 // affects consensus
#define CRYPTO777_PUBSECPSTR "020e46e79a2a8d12b9b5d12c7a91adb4e454edfae43c0a0cb805427d2ac7613fd9"
#define KOMODO_FIRSTFUNGIBLEID 100
#define KOMODO_SAPLING_ACTIVATION 1544832000 // Dec 15th, 2018
#define KOMODO_SAPLING_DEADLINE 1550188800 // Feb 15th, 2019
#define ASSETCHAINS_STAKED_BLOCK_FUTURE_MAX 57
#define ASSETCHAINS_STAKED_BLOCK_FUTURE_HALF 27
#define ASSETCHAINS_STAKED_MIN_POW_DIFF 536900000 // 537000000 537300000
#define _COINBASE_MATURITY 100

#define _ASSETCHAINS_TIMELOCKOFF 0xffffffffffffffff

#define IS_KMD_CHAIN() (ASSETCHAINS_SYMBOL[0] == '\0')

#define KOMODO_ADDRESS_BUFSIZE 64

// KMD Notary Seasons 
// 1: May 1st 2018 1530921600
// 2: July 15th 2019 1563148800 -> estimated height 1444000
// 3: 3rd season ending isnt known, so use very far times in future.
    // 1751328000 = dummy timestamp, 1 July 2025!
    // 7113400 = 5x current KMD blockheight. 
// to add 4th season, change NUM_KMD_SEASONS to 4, and add timestamp and height of activation to these arrays. 

#define NUM_KMD_SEASONS 9
#define NUM_KMD_NOTARIES 64

extern const uint32_t nStakedDecemberHardforkTimestamp; //December 2019 hardfork
extern const int32_t nDecemberHardforkHeight;   //December 2019 hardfork

extern const uint32_t nS4Timestamp; //dPoW Season 4 2020 hardfork
extern const int32_t nS4HardforkHeight;   //dPoW Season 4 2020 hardfork

extern const uint32_t nS5Timestamp; //dPoW Season 5 June 14th, 2021 hardfork (03:00:00 PM UTC) (defined in komodo_globals.h)
extern const int32_t nS5HardforkHeight;   //dPoW Season 5 June 14th, 2021 hardfork estimated block height (defined in komodo_globals.h)

extern const uint32_t nS6Timestamp; // dPoW Season 6 Fri Jun 24 2022 13:37:33 GMT+0000
extern const int32_t nS6HardforkHeight; // estimated June 24 2022

extern const uint32_t nS7Timestamp;   // dPoW Season 7, Fri Jun 30 2023 13:37:33 GMT+0000
extern const int32_t nS7HardforkHeight;  // dPoW Season 7, Fri Jun 30 2023

extern const uint32_t nS8Timestamp;   // dPoW Season 8,  Oct 4 2024 
extern const int32_t nS8HardforkHeight;  // dPoW Season 8,  Oct 4 2024 

static const uint32_t KMD_SEASON_TIMESTAMPS[NUM_KMD_SEASONS] = {1525132800, 1563148800, nStakedDecemberHardforkTimestamp, nS4Timestamp, nS5Timestamp, nS6Timestamp, nS7Timestamp, nS8Timestamp, 1851328000};
static const int32_t KMD_SEASON_HEIGHTS[NUM_KMD_SEASONS] = {814000, 1444000, nDecemberHardforkHeight, nS4HardforkHeight, nS5HardforkHeight, nS6HardforkHeight, nS7HardforkHeight, nS8HardforkHeight, 7113400};

// Era array of pubkeys. Add extra seasons to bottom as requried, after adding appropriate info above. 
static const char *notaries_elected[NUM_KMD_SEASONS][NUM_KMD_NOTARIES][2] =
{
    {
        { "0_jl777_testA", "03b7621b44118017a16043f19b30cc8a4cfe068ac4e42417bae16ba460c80f3828" },
        { "0_jl777_testB", "02ebfc784a4ba768aad88d44d1045d240d47b26e248cafaf1c5169a42d7a61d344" },
        { "0_kolo_testA", "0287aa4b73988ba26cf6565d815786caf0d2c4af704d7883d163ee89cd9977edec" },
        { "artik_AR", "029acf1dcd9f5ff9c455f8bb717d4ae0c703e089d16cf8424619c491dff5994c90" },
        { "artik_EU", "03f54b2c24f82632e3cdebe4568ba0acf487a80f8a89779173cdb78f74514847ce" },
        { "artik_NA", "0224e31f93eff0cc30eaf0b2389fbc591085c0e122c4d11862c1729d090106c842" },
        { "artik_SH", "02bdd8840a34486f38305f311c0e2ae73e84046f6e9c3dd3571e32e58339d20937" },
        { "badass_EU", "0209d48554768dd8dada988b98aca23405057ac4b5b46838a9378b95c3e79b9b9e" },
        { "badass_NA", "02afa1a9f948e1634a29dc718d218e9d150c531cfa852843a1643a02184a63c1a7" },
        { "badass_SH", "026b49dd3923b78a592c1b475f208e23698d3f085c4c3b4906a59faf659fd9530b" },
        { "crackers_EU", "03bc819982d3c6feb801ec3b720425b017d9b6ee9a40746b84422cbbf929dc73c3" }, // 10
        { "crackers_NA", "03205049103113d48c7c7af811b4c8f194dafc43a50d5313e61a22900fc1805b45" },
        { "crackers_SH", "02be28310e6312d1dd44651fd96f6a44ccc269a321f907502aae81d246fabdb03e" },
        { "durerus_EU", "02bcbd287670bdca2c31e5d50130adb5dea1b53198f18abeec7211825f47485d57" },
        { "etszombi_AR", "031c79168d15edabf17d9ec99531ea9baa20039d0cdc14d9525863b83341b210e9" },
        { "etszombi_EU", "0281b1ad28d238a2b217e0af123ce020b79e91b9b10ad65a7917216eda6fe64bf7" }, // 15
        { "etszombi_SH", "025d7a193c0757f7437fad3431f027e7b5ed6c925b77daba52a8755d24bf682dde" },
        { "farl4web_EU", "0300ecf9121cccf14cf9423e2adb5d98ce0c4e251721fa345dec2e03abeffbab3f" },
        { "farl4web_SH", "0396bb5ed3c57aa1221d7775ae0ff751e4c7dc9be220d0917fa8bbdf670586c030" },
        { "fullmoon_AR", "0254b1d64840ce9ff6bec9dd10e33beb92af5f7cee628f999cb6bc0fea833347cc" },
        { "fullmoon_NA", "031fb362323b06e165231c887836a8faadb96eda88a79ca434e28b3520b47d235b" }, // 20
        { "fullmoon_SH", "030e12b42ec33a80e12e570b6c8274ce664565b5c3da106859e96a7208b93afd0d" },
        { "grewal_NA", "03adc0834c203d172bce814df7c7a5e13dc603105e6b0adabc942d0421aefd2132" },
        { "grewal_SH", "03212a73f5d38a675ee3cdc6e82542a96c38c3d1c79d25a1ed2e42fcf6a8be4e68" },
        { "indenodes_AR", "02ec0fa5a40f47fd4a38ea5c89e375ad0b6ddf4807c99733c9c3dc15fb978ee147" },
        { "indenodes_EU", "0221387ff95c44cb52b86552e3ec118a3c311ca65b75bf807c6c07eaeb1be8303c" },
        { "indenodes_NA", "02698c6f1c9e43b66e82dbb163e8df0e5a2f62f3a7a882ca387d82f86e0b3fa988" },
        { "indenodes_SH", "0334e6e1ec8285c4b85bd6dae67e17d67d1f20e7328efad17ce6fd24ae97cdd65e" },
        { "jeezy_EU", "023cb3e593fb85c5659688528e9a4f1c4c7f19206edc7e517d20f794ba686fd6d6" },
        { "jsgalt_NA", "027b3fb6fede798cd17c30dbfb7baf9332b3f8b1c7c513f443070874c410232446" },
        { "karasugoi_NA", "02a348b03b9c1a8eac1b56f85c402b041c9bce918833f2ea16d13452309052a982" }, // 30
        { "kashifali_EU", "033777c52a0190f261c6f66bd0e2bb299d30f012dcb8bfff384103211edb8bb207" },
        { "kolo_AR", "03016d19344c45341e023b72f9fb6e6152fdcfe105f3b4f50b82a4790ff54e9dc6" },
        { "kolo_SH", "02aa24064500756d9b0959b44d5325f2391d8e95c6127e109184937152c384e185" },
        { "metaphilibert_AR", "02adad675fae12b25fdd0f57250b0caf7f795c43f346153a31fe3e72e7db1d6ac6" },
        { "movecrypto_AR", "022783d94518e4dc77cbdf1a97915b29f427d7bc15ea867900a76665d3112be6f3" },
        { "movecrypto_EU", "021ab53bc6cf2c46b8a5456759f9d608966eff87384c2b52c0ac4cc8dd51e9cc42" },
        { "movecrypto_NA", "02efb12f4d78f44b0542d1c60146738e4d5506d27ec98a469142c5c84b29de0a80" },
        { "movecrypto_SH", "031f9739a3ebd6037a967ce1582cde66e79ea9a0551c54731c59c6b80f635bc859" },
        { "muros_AR", "022d77402fd7179335da39479c829be73428b0ef33fb360a4de6890f37c2aa005e" },
        { "noashh_AR", "029d93ef78197dc93892d2a30e5a54865f41e0ca3ab7eb8e3dcbc59c8756b6e355" }, // 40
        { "noashh_EU", "02061c6278b91fd4ac5cab4401100ffa3b2d5a277e8f71db23401cc071b3665546" },
        { "noashh_NA", "033c073366152b6b01535e15dd966a3a8039169584d06e27d92a69889b720d44e1" },
        { "nxtswe_EU", "032fb104e5eaa704a38a52c126af8f67e870d70f82977e5b2f093d5c1c21ae5899" },
        { "polycryptoblog_NA", "02708dcda7c45fb54b78469673c2587bfdd126e381654819c4c23df0e00b679622" },
        { "pondsea_AR", "032e1c213787312099158f2d74a89e8240a991d162d4ce8017d8504d1d7004f735" },
        { "pondsea_EU", "0225aa6f6f19e543180b31153d9e6d55d41bc7ec2ba191fd29f19a2f973544e29d" },
        { "pondsea_NA", "031bcfdbb62268e2ff8dfffeb9ddff7fe95fca46778c77eebff9c3829dfa1bb411" },
        { "pondsea_SH", "02209073bc0943451498de57f802650311b1f12aa6deffcd893da198a544c04f36" },
        { "popcornbag_AR", "02761f106fb34fbfc5ddcc0c0aa831ed98e462a908550b280a1f7bd32c060c6fa3" },
        { "popcornbag_NA", "03c6085c7fdfff70988fda9b197371f1caf8397f1729a844790e421ee07b3a93e8" }, // 50
        { "ptytrader_NA", "0328c61467148b207400b23875234f8a825cce65b9c4c9b664f47410b8b8e3c222" },
        { "ptytrader_SH", "0250c93c492d8d5a6b565b90c22bee07c2d8701d6118c6267e99a4efd3c7748fa4" },
        { "rnr_AR", "029bdb08f931c0e98c2c4ba4ef45c8e33a34168cb2e6bf953cef335c359d77bfcd" },
        { "rnr_EU", "03f5c08dadffa0ffcafb8dd7ffc38c22887bd02702a6c9ac3440deddcf2837692b" },
        { "rnr_NA", "02e17c5f8c3c80f584ed343b8dcfa6d710dfef0889ec1e7728ce45ce559347c58c" },
        { "rnr_SH", "037536fb9bdfed10251f71543fb42679e7c52308bcd12146b2568b9a818d8b8377" },
        { "titomane_AR", "03cda6ca5c2d02db201488a54a548dbfc10533bdc275d5ea11928e8d6ab33c2185" },
        { "titomane_EU", "02e41feded94f0cc59f55f82f3c2c005d41da024e9a805b41105207ef89aa4bfbd" },
        { "titomane_SH", "035f49d7a308dd9a209e894321f010d21b7793461b0c89d6d9231a3fe5f68d9960" },
        { "vanbreuk_EU", "024f3cad7601d2399c131fd070e797d9cd8533868685ddbe515daa53c2e26004c3" }, // 60
        { "xrobesx_NA", "03f0cc6d142d14a40937f12dbd99dbd9021328f45759e26f1877f2a838876709e1" },
        { "xxspot1_XX", "02ef445a392fcaf3ad4176a5da7f43580e8056594e003eba6559a713711a27f955" },
        { "xxspot2_XX", "03d85b221ea72ebcd25373e7961f4983d12add66a92f899deaf07bab1d8b6f5573" }
    },
    {
        {"0dev1_jl777", "03b7621b44118017a16043f19b30cc8a4cfe068ac4e42417bae16ba460c80f3828" },
        {"0dev2_kolo", "030f34af4b908fb8eb2099accb56b8d157d49f6cfb691baa80fdd34f385efed961" },
        {"0dev3_kolo", "025af9d2b2a05338478159e9ac84543968fd18c45fd9307866b56f33898653b014" },
        {"0dev4_decker", "028eea44a09674dda00d88ffd199a09c9b75ba9782382cc8f1e97c0fd565fe5707" },
        {"a-team_SH", "03b59ad322b17cb94080dc8e6dc10a0a865de6d47c16fb5b1a0b5f77f9507f3cce" },
        {"artik_AR", "029acf1dcd9f5ff9c455f8bb717d4ae0c703e089d16cf8424619c491dff5994c90" },
        {"artik_EU", "03f54b2c24f82632e3cdebe4568ba0acf487a80f8a89779173cdb78f74514847ce" },
        {"artik_NA", "0224e31f93eff0cc30eaf0b2389fbc591085c0e122c4d11862c1729d090106c842" },
        {"artik_SH", "02bdd8840a34486f38305f311c0e2ae73e84046f6e9c3dd3571e32e58339d20937" },
        {"badass_EU", "0209d48554768dd8dada988b98aca23405057ac4b5b46838a9378b95c3e79b9b9e" },
        {"badass_NA", "02afa1a9f948e1634a29dc718d218e9d150c531cfa852843a1643a02184a63c1a7" }, // 10
        {"batman_AR", "033ecb640ec5852f42be24c3bf33ca123fb32ced134bed6aa2ba249cf31b0f2563" },
        {"batman_SH", "02ca5898931181d0b8aafc75ef56fce9c43656c0b6c9f64306e7c8542f6207018c" },
        {"ca333_EU", "03fc87b8c804f12a6bd18efd43b0ba2828e4e38834f6b44c0bfee19f966a12ba99" },
        {"chainmakers_EU", "02f3b08938a7f8d2609d567aebc4989eeded6e2e880c058fdf092c5da82c3bc5ee" },
        {"chainmakers_NA", "0276c6d1c65abc64c8559710b8aff4b9e33787072d3dda4ec9a47b30da0725f57a" },
        {"chainstrike_SH", "0370bcf10575d8fb0291afad7bf3a76929734f888228bc49e35c5c49b336002153" },
        {"cipi_AR", "02c4f89a5b382750836cb787880d30e23502265054e1c327a5bfce67116d757ce8" },
        {"cipi_NA", "02858904a2a1a0b44df4c937b65ee1f5b66186ab87a751858cf270dee1d5031f18" },
        {"crackers_EU", "03bc819982d3c6feb801ec3b720425b017d9b6ee9a40746b84422cbbf929dc73c3" },
        {"crackers_NA", "03205049103113d48c7c7af811b4c8f194dafc43a50d5313e61a22900fc1805b45" }, // 20
        {"dwy_EU", "0259c646288580221fdf0e92dbeecaee214504fdc8bbdf4a3019d6ec18b7540424" },
        {"emmanux_SH", "033f316114d950497fc1d9348f03770cd420f14f662ab2db6172df44c389a2667a" },
        {"etszombi_EU", "0281b1ad28d238a2b217e0af123ce020b79e91b9b10ad65a7917216eda6fe64bf7" },
        {"fullmoon_AR", "03380314c4f42fa854df8c471618751879f9e8f0ff5dbabda2bd77d0f96cb35676" },
        {"fullmoon_NA", "030216211d8e2a48bae9e5d7eb3a42ca2b7aae8770979a791f883869aea2fa6eef" },
        {"fullmoon_SH", "03f34282fa57ecc7aba8afaf66c30099b5601e98dcbfd0d8a58c86c20d8b692c64" },
        {"goldenman_EU", "02d6f13a8f745921cdb811e32237bb98950af1a5952be7b3d429abd9152f8e388d" },
        {"indenodes_AR", "02ec0fa5a40f47fd4a38ea5c89e375ad0b6ddf4807c99733c9c3dc15fb978ee147" },
        {"indenodes_EU", "0221387ff95c44cb52b86552e3ec118a3c311ca65b75bf807c6c07eaeb1be8303c" },
        {"indenodes_NA", "02698c6f1c9e43b66e82dbb163e8df0e5a2f62f3a7a882ca387d82f86e0b3fa988" }, // 30
        {"indenodes_SH", "0334e6e1ec8285c4b85bd6dae67e17d67d1f20e7328efad17ce6fd24ae97cdd65e" },
        {"jackson_AR", "038ff7cfe34cb13b524e0941d5cf710beca2ffb7e05ddf15ced7d4f14fbb0a6f69" },
        {"jeezy_EU", "023cb3e593fb85c5659688528e9a4f1c4c7f19206edc7e517d20f794ba686fd6d6" },
        {"karasugoi_NA", "02a348b03b9c1a8eac1b56f85c402b041c9bce918833f2ea16d13452309052a982" },
        {"komodoninja_EU", "038e567b99806b200b267b27bbca2abf6a3e8576406df5f872e3b38d30843cd5ba" },
        {"komodoninja_SH", "033178586896915e8456ebf407b1915351a617f46984001790f0cce3d6f3ada5c2" },
        {"komodopioneers_SH", "033ace50aedf8df70035b962a805431363a61cc4e69d99d90726a2d48fb195f68c" },
        {"libscott_SH", "03301a8248d41bc5dc926088a8cf31b65e2daf49eed7eb26af4fb03aae19682b95" },
        {"lukechilds_AR", "031aa66313ee024bbee8c17915cf7d105656d0ace5b4a43a3ab5eae1e14ec02696" },
        {"madmax_AR", "03891555b4a4393d655bf76f0ad0fb74e5159a615b6925907678edc2aac5e06a75" }, // 40
        {"meshbits_AR", "02957fd48ae6cb361b8a28cdb1b8ccf5067ff68eb1f90cba7df5f7934ed8eb4b2c" },
        {"meshbits_SH", "025c6e94877515dfd7b05682b9cc2fe4a49e076efe291e54fcec3add78183c1edb" },
        {"metaphilibert_AR", "02adad675fae12b25fdd0f57250b0caf7f795c43f346153a31fe3e72e7db1d6ac6" },
        {"metaphilibert_SH", "0284af1a5ef01503e6316a2ca4abf8423a794e9fc17ac6846f042b6f4adedc3309" },
        {"patchkez_SH", "0296270f394140640f8fa15684fc11255371abb6b9f253416ea2734e34607799c4" },
        {"pbca26_NA", "0276aca53a058556c485bbb60bdc54b600efe402a8b97f0341a7c04803ce204cb5" },
        {"peer2cloud_AR", "034e5563cb885999ae1530bd66fab728e580016629e8377579493b386bf6cebb15" },
        {"peer2cloud_SH", "03396ac453b3f23e20f30d4793c5b8ab6ded6993242df4f09fd91eb9a4f8aede84" },
        {"polycryptoblog_NA", "02708dcda7c45fb54b78469673c2587bfdd126e381654819c4c23df0e00b679622" },
        {"hyper_AR", "020f2f984d522051bd5247b61b080b4374a7ab389d959408313e8062acad3266b4" }, // 50
        {"hyper_EU", "03d00cf9ceace209c59fb013e112a786ad583d7de5ca45b1e0df3b4023bb14bf51" },
        {"hyper_SH", "0383d0b37f59f4ee5e3e98a47e461c861d49d0d90c80e9e16f7e63686a2dc071f3" },
        {"hyper_NA", "03d91c43230336c0d4b769c9c940145a8c53168bf62e34d1bccd7f6cfc7e5592de" },
        {"popcornbag_AR", "02761f106fb34fbfc5ddcc0c0aa831ed98e462a908550b280a1f7bd32c060c6fa3" },
        {"popcornbag_NA", "03c6085c7fdfff70988fda9b197371f1caf8397f1729a844790e421ee07b3a93e8" },
        {"alien_AR", "0348d9b1fc6acf81290405580f525ee49b4749ed4637b51a28b18caa26543b20f0" },
        {"alien_EU", "020aab8308d4df375a846a9e3b1c7e99597b90497efa021d50bcf1bbba23246527" },
        {"thegaltmines_NA", "031bea28bec98b6380958a493a703ddc3353d7b05eb452109a773eefd15a32e421" },
        {"titomane_AR", "029d19215440d8cb9cc6c6b7a4744ae7fb9fb18d986e371b06aeb34b64845f9325" },
        {"titomane_EU", "0360b4805d885ff596f94312eed3e4e17cb56aa8077c6dd78d905f8de89da9499f" }, // 60
        {"titomane_SH", "03573713c5b20c1e682a2e8c0f8437625b3530f278e705af9b6614de29277a435b" },
        {"webworker01_NA", "03bb7d005e052779b1586f071834c5facbb83470094cff5112f0072b64989f97d7" },
        {"xrobesx_NA", "03f0cc6d142d14a40937f12dbd99dbd9021328f45759e26f1877f2a838876709e1" },
    },
    {
        {"madmax_NA", "0237e0d3268cebfa235958808db1efc20cc43b31100813b1f3e15cc5aa647ad2c3" }, // 0
        {"alright_AR", "020566fe2fb3874258b2d3cf1809a5d650e0edc7ba746fa5eec72750c5188c9cc9" },
        {"strob_NA", "0206f7a2e972d9dfef1c424c731503a0a27de1ba7a15a91a362dc7ec0d0fb47685" },
        {"dwy_EU", "021c7cf1f10c4dc39d13451123707ab780a741feedab6ac449766affe37515a29e" },
        {"phm87_SH", "021773a38db1bc3ede7f28142f901a161c7b7737875edbb40082a201c55dcf0add" },
        {"chainmakers_NA", "02285d813c30c0bf7eefdab1ff0a8ad08a07a0d26d8b95b3943ce814ac8e24d885" },
        {"indenodes_EU", "0221387ff95c44cb52b86552e3ec118a3c311ca65b75bf807c6c07eaeb1be8303c" },
        {"blackjok3r_SH", "021eac26dbad256cbb6f74d41b10763183ee07fb609dbd03480dd50634170547cc" },
        {"chainmakers_EU", "03fdf5a3fce8db7dee89724e706059c32e5aa3f233a6b6cc256fea337f05e3dbf7" },
        {"titomane_AR", "023e3aa9834c46971ff3e7cb86a200ec9c8074a9566a3ea85d400d5739662ee989" },
        {"fullmoon_SH", "023b7252968ea8a955cd63b9e57dee45a74f2d7ba23b4e0595572138ad1fb42d21" }, // 10
        {"indenodes_NA", "02698c6f1c9e43b66e82dbb163e8df0e5a2f62f3a7a882ca387d82f86e0b3fa988" },
        {"chmex_EU", "0281304ebbcc39e4f09fda85f4232dd8dacd668e20e5fc11fba6b985186c90086e" },
        {"metaphilibert_SH", "0284af1a5ef01503e6316a2ca4abf8423a794e9fc17ac6846f042b6f4adedc3309" },
        {"ca333_DEV", "02856843af2d9457b5b1c907068bef6077ea0904cc8bd4df1ced013f64bf267958" },
        {"cipi_NA", "02858904a2a1a0b44df4c937b65ee1f5b66186ab87a751858cf270dee1d5031f18" },
        {"pungocloud_SH", "024dfc76fa1f19b892be9d06e985d0c411e60dbbeb36bd100af9892a39555018f6" },
        {"voskcoin_EU", "034190b1c062a04124ad15b0fa56dfdf34aa06c164c7163b6aec0d654e5f118afb" },
        {"decker_DEV", "028eea44a09674dda00d88ffd199a09c9b75ba9782382cc8f1e97c0fd565fe5707" },
        {"cryptoeconomy_EU", "0290ab4937e85246e048552df3e9a66cba2c1602db76e03763e16c671e750145d1" },
        {"etszombi_EU", "0293ea48d8841af7a419a24d9da11c34b39127ef041f847651bae6ab14dcd1f6b4" },  // 20
        {"karasugoi_NA", "02a348b03b9c1a8eac1b56f85c402b041c9bce918833f2ea16d13452309052a982" },
        {"pirate_AR", "03e29c90354815a750db8ea9cb3c1b9550911bb205f83d0355a061ac47c4cf2fde" },
        {"metaphilibert_AR", "02adad675fae12b25fdd0f57250b0caf7f795c43f346153a31fe3e72e7db1d6ac6" },
        {"zatjum_SH", "02d6b0c89cacd58a0af038139a9a90c9e02cd1e33803a1f15fceabea1f7e9c263a" },
        {"madmax_AR", "03c5941fe49d673c094bc8e9bb1a95766b4670c88be76d576e915daf2c30a454d3" },
        {"lukechilds_NA", "03f1051e62c2d280212481c62fe52aab0a5b23c95de5b8e9ad5f80d8af4277a64b" },
        {"cipi_AR", "02c4f89a5b382750836cb787880d30e23502265054e1c327a5bfce67116d757ce8" },
        {"tonyl_AR", "02cc8bc862f2b65ad4f99d5f68d3011c138bf517acdc8d4261166b0be8f64189e1" },
        {"infotech_DEV", "0345ad4ab5254782479f6322c369cec77a7535d2f2162d103d666917d5e4f30c4c" },
        {"fullmoon_NA", "032c716701fe3a6a3f90a97b9d874a9d6eedb066419209eed7060b0cc6b710c60b" },  // 30
        {"etszombi_AR", "02e55e104aa94f70cde68165d7df3e162d4410c76afd4643b161dea044aa6d06ce" },
        {"node-9_EU", "0372e5b51e86e2392bb15039bac0c8f975b852b45028a5e43b324c294e9f12e411" },
        {"phba2061_EU", "03f6bd15dba7e986f0c976ea19d8a9093cb7c989d499f1708a0386c5c5659e6c4e" },
        {"indenodes_AR", "02ec0fa5a40f47fd4a38ea5c89e375ad0b6ddf4807c99733c9c3dc15fb978ee147" },
        {"and1-89_EU", "02736cbf8d7b50835afd50a319f162dd4beffe65f2b1dc6b90e64b32c8e7849ddd" },
        {"komodopioneers_SH", "032a238a5747777da7e819cfa3c859f3677a2daf14e4dce50916fc65d00ad9c52a" },
        {"komodopioneers_EU", "036d02425916444fff8cc7203fcbfc155c956dda5ceb647505836bef59885b6866" },
        {"d0ct0r_NA", "0303725d8525b6f969122faf04152653eb4bf34e10de92182263321769c334bf58" },
        {"kolo_DEV", "02849e12199dcc27ba09c3902686d2ad0adcbfcee9d67520e9abbdda045ba83227" },
        {"peer2cloud_AR", "02acc001fe1fe8fd68685ba26c0bc245924cb592e10cec71e9917df98b0e9d7c37" }, // 40
        {"webworker01_SH", "031e50ba6de3c16f99d414bb89866e578d963a54bde7916c810608966fb5700776" },
        {"webworker01_NA", "032735e9cad1bb00eaababfa6d27864fa4c1db0300c85e01e52176be2ca6a243ce" },
        {"pbca26_NA", "03a97606153d52338bcffd1bf19bb69ef8ce5a7cbdc2dbc3ff4f89d91ea6bbb4dc" },
        {"indenodes_SH", "0334e6e1ec8285c4b85bd6dae67e17d67d1f20e7328efad17ce6fd24ae97cdd65e" },
        {"pirate_NA", "0255e32d8a56671dee8aa7f717debb00efa7f0086ee802de0692f2d67ee3ee06ee" },
        {"lukechilds_AR", "025c6a73ff6d750b9ddf6755b390948cffdd00f344a639472d398dd5c6b4735d23" },
        {"dragonhound_NA", "0224a9d951d3a06d8e941cc7362b788bb1237bb0d56cc313e797eb027f37c2d375" },
        {"fullmoon_AR", "03da64dd7cd0db4c123c2f79d548a96095a5a103e5b9d956e9832865818ffa7872" },
        {"chainzilla_SH", "0360804b8817fd25ded6e9c0b50e3b0782ac666545b5416644198e18bc3903d9f9" },
        {"titomane_EU", "03772ac0aad6b0e9feec5e591bff5de6775d6132e888633e73d3ba896bdd8e0afb" }, // 50
        {"jeezy_EU", "037f182facbad35684a6e960699f5da4ba89e99f0d0d62a87e8400dd086c8e5dd7" },
        {"titomane_SH", "03850fdddf2413b51790daf51dd30823addb37313c8854b508ea6228205047ef9b" },
        {"alien_AR", "03911a60395801082194b6834244fa78a3c30ff3e888667498e157b4aa80b0a65f" },
        {"pirate_EU", "03fff24efd5648870a23badf46e26510e96d9e79ce281b27cfe963993039dd1351" },
        {"thegaltmines_NA", "02db1a16c7043f45d6033ccfbd0a51c2d789b32db428902f98b9e155cf0d7910ed" },
        {"computergenie_NA", "03a78ae070a5e9e935112cf7ea8293f18950f1011694ea0260799e8762c8a6f0a4" },
        {"nutellalicka_SH", "02f7d90d0510c598ce45915e6372a9cd0ba72664cb65ce231f25d526fc3c5479fc" },
        {"chainstrike_SH", "03b806be3bf7a1f2f6290ec5c1ea7d3ea57774dcfcf2129a82b2569e585100e1cb" },
        {"dwy_SH", "036536d2d52d85f630b68b050f29ea1d7f90f3b42c10f8c5cdf3dbe1359af80aff" },
        {"alien_EU", "03bb749e337b9074465fa28e757b5aa92cb1f0fea1a39589bca91a602834d443cd" }, // 60
        {"gt_AR", "0348430538a4944d3162bb4749d8c5ed51299c2434f3ee69c11a1f7815b3f46135" },
        {"patchkez_SH", "03f45e9beb5c4cd46525db8195eb05c1db84ae7ef3603566b3d775770eba3b96ee" },
        {"decker_AR", "03ffdf1a116300a78729608d9930742cd349f11a9d64fcc336b8f18592dd9c91bc" }, // 63
    },
    {
        // Season 3.5
        {"madmax_NA", "0237e0d3268cebfa235958808db1efc20cc43b31100813b1f3e15cc5aa647ad2c3" }, // 0
        {"alright_AR", "020566fe2fb3874258b2d3cf1809a5d650e0edc7ba746fa5eec72750c5188c9cc9" },
        {"strob_NA", "0206f7a2e972d9dfef1c424c731503a0a27de1ba7a15a91a362dc7ec0d0fb47685" },
        {"hunter_EU", "0378224b4e9d8a0083ce36f2963ec0a4e231ec06b0c780de108e37f41181a89f6a" },
        {"phm87_SH", "021773a38db1bc3ede7f28142f901a161c7b7737875edbb40082a201c55dcf0add" },
        {"chainmakers_NA", "02285d813c30c0bf7eefdab1ff0a8ad08a07a0d26d8b95b3943ce814ac8e24d885" },
        {"indenodes_EU", "0221387ff95c44cb52b86552e3ec118a3c311ca65b75bf807c6c07eaeb1be8303c" },
        {"blackjok3r_SH", "021eac26dbad256cbb6f74d41b10763183ee07fb609dbd03480dd50634170547cc" },
        {"chainmakers_EU", "03fdf5a3fce8db7dee89724e706059c32e5aa3f233a6b6cc256fea337f05e3dbf7" },
        {"titomane_AR", "023e3aa9834c46971ff3e7cb86a200ec9c8074a9566a3ea85d400d5739662ee989" },
        {"fullmoon_SH", "023b7252968ea8a955cd63b9e57dee45a74f2d7ba23b4e0595572138ad1fb42d21" }, // 10
        {"indenodes_NA", "02698c6f1c9e43b66e82dbb163e8df0e5a2f62f3a7a882ca387d82f86e0b3fa988" },
        {"chmex_EU", "0281304ebbcc39e4f09fda85f4232dd8dacd668e20e5fc11fba6b985186c90086e" },
        {"metaphilibert_SH", "0284af1a5ef01503e6316a2ca4abf8423a794e9fc17ac6846f042b6f4adedc3309" },
        {"ca333_DEV", "02856843af2d9457b5b1c907068bef6077ea0904cc8bd4df1ced013f64bf267958" },
        {"cipi_NA", "02858904a2a1a0b44df4c937b65ee1f5b66186ab87a751858cf270dee1d5031f18" },
        {"pungocloud_SH", "024dfc76fa1f19b892be9d06e985d0c411e60dbbeb36bd100af9892a39555018f6" },
        {"voskcoin_EU", "034190b1c062a04124ad15b0fa56dfdf34aa06c164c7163b6aec0d654e5f118afb" },
        {"decker_DEV", "028eea44a09674dda00d88ffd199a09c9b75ba9782382cc8f1e97c0fd565fe5707" },
        {"cryptoeconomy_EU", "0290ab4937e85246e048552df3e9a66cba2c1602db76e03763e16c671e750145d1" },
        {"etszombi_EU", "0293ea48d8841af7a419a24d9da11c34b39127ef041f847651bae6ab14dcd1f6b4" },  // 20
        {"karasugoi_NA", "02a348b03b9c1a8eac1b56f85c402b041c9bce918833f2ea16d13452309052a982" },
        {"pirate_AR", "03e29c90354815a750db8ea9cb3c1b9550911bb205f83d0355a061ac47c4cf2fde" },
        {"metaphilibert_AR", "02adad675fae12b25fdd0f57250b0caf7f795c43f346153a31fe3e72e7db1d6ac6" },
        {"zatjum_SH", "02d6b0c89cacd58a0af038139a9a90c9e02cd1e33803a1f15fceabea1f7e9c263a" },
        {"madmax_AR", "03c5941fe49d673c094bc8e9bb1a95766b4670c88be76d576e915daf2c30a454d3" },
        {"lukechilds_NA", "03f1051e62c2d280212481c62fe52aab0a5b23c95de5b8e9ad5f80d8af4277a64b" },
        {"cipi_AR", "02c4f89a5b382750836cb787880d30e23502265054e1c327a5bfce67116d757ce8" },
        {"tonyl_AR", "02cc8bc862f2b65ad4f99d5f68d3011c138bf517acdc8d4261166b0be8f64189e1" },
        {"infotech_DEV", "0345ad4ab5254782479f6322c369cec77a7535d2f2162d103d666917d5e4f30c4c" },
        {"fullmoon_NA", "032c716701fe3a6a3f90a97b9d874a9d6eedb066419209eed7060b0cc6b710c60b" },  // 30
        {"etszombi_AR", "02e55e104aa94f70cde68165d7df3e162d4410c76afd4643b161dea044aa6d06ce" },
        {"node-9_EU", "0372e5b51e86e2392bb15039bac0c8f975b852b45028a5e43b324c294e9f12e411" },
        {"phba2061_EU", "03f6bd15dba7e986f0c976ea19d8a9093cb7c989d499f1708a0386c5c5659e6c4e" },
        {"indenodes_AR", "02ec0fa5a40f47fd4a38ea5c89e375ad0b6ddf4807c99733c9c3dc15fb978ee147" },
        {"and1-89_EU", "02736cbf8d7b50835afd50a319f162dd4beffe65f2b1dc6b90e64b32c8e7849ddd" },
        {"komodopioneers_SH", "032a238a5747777da7e819cfa3c859f3677a2daf14e4dce50916fc65d00ad9c52a" },
        {"komodopioneers_EU", "036d02425916444fff8cc7203fcbfc155c956dda5ceb647505836bef59885b6866" },
        {"d0ct0r_NA", "0303725d8525b6f969122faf04152653eb4bf34e10de92182263321769c334bf58" },
        {"kolo_DEV", "02849e12199dcc27ba09c3902686d2ad0adcbfcee9d67520e9abbdda045ba83227" },
        {"peer2cloud_AR", "02acc001fe1fe8fd68685ba26c0bc245924cb592e10cec71e9917df98b0e9d7c37" }, // 40
        {"webworker01_SH", "031e50ba6de3c16f99d414bb89866e578d963a54bde7916c810608966fb5700776" },
        {"webworker01_NA", "032735e9cad1bb00eaababfa6d27864fa4c1db0300c85e01e52176be2ca6a243ce" },
        {"pbca26_NA", "03a97606153d52338bcffd1bf19bb69ef8ce5a7cbdc2dbc3ff4f89d91ea6bbb4dc" },
        {"indenodes_SH", "0334e6e1ec8285c4b85bd6dae67e17d67d1f20e7328efad17ce6fd24ae97cdd65e" },
        {"pirate_NA", "0255e32d8a56671dee8aa7f717debb00efa7f0086ee802de0692f2d67ee3ee06ee" },
        {"lukechilds_AR", "025c6a73ff6d750b9ddf6755b390948cffdd00f344a639472d398dd5c6b4735d23" },
        {"dragonhound_NA", "0224a9d951d3a06d8e941cc7362b788bb1237bb0d56cc313e797eb027f37c2d375" },
        {"fullmoon_AR", "03da64dd7cd0db4c123c2f79d548a96095a5a103e5b9d956e9832865818ffa7872" },
        {"chainzilla_SH", "0360804b8817fd25ded6e9c0b50e3b0782ac666545b5416644198e18bc3903d9f9" },
        {"titomane_EU", "03772ac0aad6b0e9feec5e591bff5de6775d6132e888633e73d3ba896bdd8e0afb" }, // 50
        {"jeezy_EU", "037f182facbad35684a6e960699f5da4ba89e99f0d0d62a87e8400dd086c8e5dd7" },
        {"titomane_SH", "03850fdddf2413b51790daf51dd30823addb37313c8854b508ea6228205047ef9b" },
        {"alien_AR", "03911a60395801082194b6834244fa78a3c30ff3e888667498e157b4aa80b0a65f" },
        {"pirate_EU", "03fff24efd5648870a23badf46e26510e96d9e79ce281b27cfe963993039dd1351" },
        {"thegaltmines_NA", "02db1a16c7043f45d6033ccfbd0a51c2d789b32db428902f98b9e155cf0d7910ed" },
        {"computergenie_NA", "03a78ae070a5e9e935112cf7ea8293f18950f1011694ea0260799e8762c8a6f0a4" },
        {"nutellalicka_SH", "02f7d90d0510c598ce45915e6372a9cd0ba72664cb65ce231f25d526fc3c5479fc" },
        {"chainstrike_SH", "03b806be3bf7a1f2f6290ec5c1ea7d3ea57774dcfcf2129a82b2569e585100e1cb" },
        {"hunter_SH", "02407db70ad30ce4dfaee8b4ae35fae88390cad2b0ba0373fdd6231967537ccfdf" },
        {"alien_EU", "03bb749e337b9074465fa28e757b5aa92cb1f0fea1a39589bca91a602834d443cd" }, // 60
        {"gt_AR", "0348430538a4944d3162bb4749d8c5ed51299c2434f3ee69c11a1f7815b3f46135" },
        {"patchkez_SH", "03f45e9beb5c4cd46525db8195eb05c1db84ae7ef3603566b3d775770eba3b96ee" },
        {"decker_AR", "03ffdf1a116300a78729608d9930742cd349f11a9d64fcc336b8f18592dd9c91bc" }, // 63
    },
    {
        // Season 4
        { "alien_AR", "03911a60395801082194b6834244fa78a3c30ff3e888667498e157b4aa80b0a65f" },
        { "alien_EU", "03bb749e337b9074465fa28e757b5aa92cb1f0fea1a39589bca91a602834d443cd" },
        { "strob_NA", "02a1c0bd40b294f06d3e44a52d1b2746c260c475c725e9351f1312e49e01c9a405" },
        { "titomane_SH", "020014ad4eedf6b1aeb0ad3b101a58d0a2fc570719e46530fd98d4e585f63eb4ae" },
        { "fullmoon_AR", "03b251095e747f759505ec745a4bbff9a768b8dce1f65137300b7c21efec01a07a" },
        { "phba2061_EU", "03a9492d2a1601d0d98cfe94d8adf9689d1bb0e600088127a4f6ca937761fb1c66" },
        { "fullmoon_NA", "03931c1d654a99658998ce0ddae108d825943a821d1cddd85e948ac1d483f68fb6" },
        { "fullmoon_SH", "03c2a1ed9ddb7bb8344328946017b9d8d1357b898957dd6aaa8c190ae26740b9ff" },
        { "madmax_AR", "022be5a2829fa0291f9a51ff7aeceef702eef581f2611887c195e29da49092e6de" },
        { "titomane_EU", "0285cf1fdba761daf6f1f611c32d319cd58214972ef822793008b69dde239443dd" },
        { "cipi_NA", "022c6825a24792cc3b010b1531521eba9b5e2662d640ed700fd96167df37e75239" },
        { "indenodes_SH", "0334e6e1ec8285c4b85bd6dae67e17d67d1f20e7328efad17ce6fd24ae97cdd65e" },
        { "decker_AR", "03ffdf1a116300a78729608d9930742cd349f11a9d64fcc336b8f18592dd9c91bc" },
        { "indenodes_EU", "0221387ff95c44cb52b86552e3ec118a3c311ca65b75bf807c6c07eaeb1be8303c" },
        { "madmax_NA", "02997b7ab21b86bbea558ae79acc35d62c9cedf441578f78112f986d72e8eece08" },
        { "chainzilla_SH", "02288ba6dc57936b59d60345e397d62f5d7e7d975f34ed5c2f2e23288325661563" },
        { "peer2cloud_AR", "0250e7e43a3535731b051d1bcc7dc88fbb5163c3fe41c5dee72bd973bcc4dca9f2" },
        { "pirate_EU", "0231c0f50a06655c3d2edf8d7e722d290195d49c78d50de7786b9d196e8820c848" },
        { "webworker01_NA", "02dfd5f3cef1142879a7250752feb91ddd722c497fb98c7377c0fcc5ccc201bd55" },
        { "zatjum_SH", "036066fd638b10e555597623e97e032b28b4d1fa5a13c2b0c80c420dbddad236c2" },
        { "titomane_AR", "0268203a4c80047edcd66385c22e764ea5fb8bc42edae389a438156e7dca9a8251" },
        { "chmex_EU", "025b7209ba37df8d9695a23ea706ea2594863ab09055ca6bf485855937f3321d1d" },
        { "indenodes_NA", "02698c6f1c9e43b66e82dbb163e8df0e5a2f62f3a7a882ca387d82f86e0b3fa988" },
        { "patchkez_SH", "02cabd6c5fc0b5476c7a01e9d7b907e9f0a051d7f4f731959955d3f6b18ee9a242" },
        { "metaphilibert_AR", "02adad675fae12b25fdd0f57250b0caf7f795c43f346153a31fe3e72e7db1d6ac6" },
        { "etszombi_EU", "0341adbf238f33a33cc895633db996c3ad01275313ac6641e046a3db0b27f1c880" },
        { "pirate_NA", "02207f27a13625a0b8caef6a7bb9de613ff16e4a5f232da8d7c235c7c5bad72ffe" },
        { "metaphilibert_SH", "0284af1a5ef01503e6316a2ca4abf8423a794e9fc17ac6846f042b6f4adedc3309" },
        { "indenodes_AR", "02ec0fa5a40f47fd4a38ea5c89e375ad0b6ddf4807c99733c9c3dc15fb978ee147" },
        { "chainmakers_NA", "029415a1609c33dfe4a1016877ba35f9265d25d737649f307048efe96e76512877" },
        { "mihailo_EU", "037f9563f30c609b19fd435a19b8bde7d6db703012ba1aba72e9f42a87366d1941" },
        { "tonyl_AR", "0299684d7291abf90975fa493bf53212cf1456c374aa36f83cc94daece89350ae9" },
        { "alien_NA", "03bea1ac333b95c8669ec091907ea8713cae26f74b9e886e13593400e21c4d30a8" },
        { "pungocloud_SH", "025b97d8c23effaca6fa7efacce20bf54df73081b63004a0fe22f3f98fece5669f" },
        { "node9_EU", "029ffa793b5c3248f8ea3da47fa3cf1810dada5af032ecd0e37bab5b92dd63b34e" },
        { "smdmitry_AR", "022a2a45979a6631a25e4c96469423de720a2f4c849548957c35a35c91041ee7ac" },
        { "nodeone_NA", "03f9dd0484e81174fd50775cb9099691c7d140ff00c0f088847e38dc87da67eb9b" },
        { "gcharang_SH", "02ec4172eab854a0d8cd32bc691c83e93975a3df5a4a453a866736c56e025dc359" },
        { "cipi_EU", "02f2b6defff1c544202f66e47cfd6909c54d67c7c39b9c2a99f137dbaf6d0bd8fa" },
        { "etszombi_AR", "0329944b0ac65b6760787ede042a2fde0be9fca1d80dd756bc0ee0b98d389b7682" },
        { "pbca26_NA", "0387e0fb6f2ca951154c87e16c6cbf93a69862bb165c1a96bcd8722b3af24fe533" },
        { "mylo_SH", "03b58f57822e90fe105e6efb63fd8666033ea503d6cc165b1e479bbd8c2ba033e8" },
        { "swisscertifiers_EU", "03ebcc71b42d88994b8b2134bcde6cb269bd7e71a9dd7616371d9294ec1c1902c5" },
        { "marmarachain_AR", "035bbd81a098172592fe97f50a0ce13cbbf80e55cc7862eccdbd7310fab8a90c4c" },
        { "karasugoi_NA", "0262cf2559703464151153c12e00c4b67a969e39b330301fdcaa6667d7eb02c57d" },
        { "phm87_SH", "021773a38db1bc3ede7f28142f901a161c7b7737875edbb40082a201c55dcf0add" },
        { "oszy_EU", "03d1ffd680491b98a3ec5541715681d1a45293c8efb1722c32392a1d792622596a" },
        { "chmex_AR", "036c856ea778ea105b93c0be187004d4e51161eda32888aa307b8f72d490884005" },
        { "dragonhound_NA", "0227e5cad3731e381df157de189527aac8eb50d82a13ce2bd81153984ebc749515" },
        { "strob_SH", "025ceac4256cef83ca4b110f837a71d70a5a977ecfdf807335e00bc78b560d451a" },
        { "madmax_EU", "02ea0cf4d6d151d0528b07efa79cc7403d77cb9195e2e6c8374f5074b9a787e287" },
        { "dudezmobi_AR", "027ecd974ff2a27a37ee69956cd2e6bb31a608116206f3e31ef186823420182450" },
        { "daemonfox_NA", "022d6f4885f53cbd668ad7d03d4f8e830c233f74e3a918da1ed247edfc71820b3d" },
        { "nutellalicka_SH", "02f4b1e71bc865a79c05fe333952b97cb040d8925d13e83925e170188b3011269b" },
        { "starfleet_EU", "025c7275bd750936862b47793f1f0bb3cbed60fb75a48e7da016e557925fe375eb" },
        { "mrlynch_AR", "031987dc82b087cd53e23df5480e265a5928e9243e0e11849fa12359739d8b18a4" },
        { "greer_NA", "03e0995615d7d3cf1107effa6bdb1133e0876cf1768e923aa533a4e2ee675ec383" },
        { "mcrypt_SH", "025faab3cc2e83bf7dad6a9463cbff86c08800e937942126f258cf219bc2320043" },
        { "decker_EU", "03777777caebce56e17ca3aae4e16374335b156f1dd62ee3c7f8799c6b885f5560" },
        { "dappvader_SH", "02962e2e5af746632016bc7b24d444f7c90141a5f42ce54e361b302cf455d90e6a" },
        { "alright_DEV", "02b73a589d61691efa2ada15c006d27bc18493fea867ce6c14db3d3d28751f8ce3" },
        { "artemii235_DEV", "03bb616b12430bdd0483653de18733597a4fd416623c7065c0e21fe9d96460add1" },
        { "tonyl_DEV", "02d5f7fd6e25d34ab2f3318d60cdb89ff3a812ec5d0212c4c113bb12d12616cfdc" },
        { "decker_DEV", "028eea44a09674dda00d88ffd199a09c9b75ba9782382cc8f1e97c0fd565fe5707" }
    },
    {
        // Season 5
        {"alrighttt_DEV", "02a876c6c35060041f6beadb201f4dfc567e80eedd3a4206ff10d99878087bd440" }, // 0
        {"alien_AR", "024f20c096b085308e21893383f44b4faf1cdedea9ad53cc7d7e7fbfa0c30c1e71" },
        {"artempikulin_AR", "03a45c4ad7f279cbc50acb48d81fc0eb63c4c5f556e3a4393fb3d6414df09c6e4c" },
        {"chmex_AR", "030cd487e10fbf142e0e8d582e702ecb775f378569c3cb5acd0ff97b6b12803588" },
        {"cipi_AR", "02336758998f474659020e6887ece61ac7b8567f9b2d38724ebf77ae800c1fb2b7" },
        {"shadowbit_AR", "03949b06c2773b4573aeb0b52e70ccc2d98dc5794a47e24eeb902c9d28e0e8d28b" },
        {"goldenman_AR", "03d745bc6921104b73734e6d9615671bc70b9e11e26c9b0c9abf0d2f9babd01a4d" },
        {"kolo_AR", "027579d0722b2f75b3d11a73829449e4251b4471716b6cb743c7667379750c8fb0" },
        {"madmax_AR", "02ddb23f18e61ea792ae0f28be5a52859e7963bf7f1d2c4f19eec18ac6497cfa2a" },
        {"mcrypt_AR", "02845d016c68c3e5ce924b164abc271511f3092ae359677a515e8f81a9533472f4" },
        {"mrlynch_AR", "03e67440141f53a08684c329ebc852b018e41f905da88e52aa4a6dc5aa4b12447a" }, // 10
        {"ocean_AR", "02d216e72d37a38449d661413cbc6e1f008b21cffdb06865f7be636e2cbc1e5346" },
        {"smdmitry_AR", "0397b7584cb29717b721c0c587d4462477efc1f36a56921f133c9d17b0cd7f278a" },
        {"tokel_AR", "02e4e07060fcd3640a3fd6d54cc15924f2bf63f8172b96a9f1d538ca7a0e490dc5" },
        {"tonyl_AR", "02e2d9ecdc9f462a4767f7dfe8ed243c98fcccc1511989a60e3f859dc6fda42d16" },
        {"tonyl_DEV", "0399c4f8c5b604cda64c1ccb8fdbd7a89730131519f87491a79b0811e619102d8f" },
        {"artem_DEV", "025ee88d1c12f546c1c8942d7a3e0678f10bc27cc566e27bf4a2d2178e018d18c6" },
        {"alien_EU", "022b85908191788f409506ebcf96a892f3274f352864c3ed566c5a16de63953236" },
        {"alienx_EU", "025de0911bab55616c307f02ea8a5915a2e0c8e479aa97968e7f00d1025cbe6c6d" },
        {"ca333_EU", "03a582cfae3760bb1cb38311d686cfeede8f8c4ce263aa1c082fc836c367859122" },
        {"chmex_EU", "030bf7bd7ad0515c33b5d5d9a91e0729baf801b9002f80495ae535ea1cebb352cb" }, // 20
        {"cipi_EU", "033a812d6cccdc4208378728f3a0e15db5b12074def9ab686ddc3752715ff1a194" },
        {"cipi2_EU", "0302ca28a041ed00544de737651bdec9bafe3b7f1c0bf2c6092f2368d59fec75c2" },
        {"shadowbit_EU", "025f8de3a6181270ceb5c31654e6a6e95d0339bc14b46b5e3050e8a69861c91baa" },
        {"komodopioneers_EU", "02fb31b130babe79ac780a6118702555a8c66875835f35c2232a6cb8b1438fe71d" },
        {"madmax_EU", "02e7e5306f159df252ecfded9bab6297050d12640b908b456ea553f90872f8a160" },
        {"marmarachain_EU", "027029380f49b0c3cc1b814976f1a83f0c25d84020ad0a27454e55ebdb2ccc83d7" },
        {"node-9_EU", "029401e427cffa29bb2bd7664110e160d525fac6f1518ac7b59343b16de301e0ac" },
        {"slyris_EU", "02a0705ec221a94a6a5b3ea2e763ba0350f8213c73e8dad49a708fb1e87acdc5f8" },
        {"smdmitry_EU", "0338f30ca34d0aca0d79b69abde447036aaaa75f482b6c75801fd382e984337d01" },
        {"van_EU", "0370305b9e91d46331da202ae733d6050d01038ef6eceb2036ada394a48fae84b9" }, // 30
        {"shadowbit_DEV", "03e2de3418c88be0cfe2fa0dcfdaea001b5a36ad86e6833ad284d79021ae7e2b94" },
        {"gcharang_DEV", "0321868e0eb39271330fa2c3a9f4e542275d9719f8b87773c5432448ab10d6943d" },
        {"alien_NA", "022f62b56ddfd07c9860921c701285ac39bb3ac8f6f083d1b59c8f4943be3de162" },
        {"alienx_NA", "025d5e11725233ab161f4f63d697c5f9f0c6b9d3aa2b9c68299638f8cc63faa9c2" },
        {"cipi_NA", "0335352862da521bd90b99d394db1ee3ecde379db9cf7ba2f28b16fa76153e289f" },
        {"computergenie_NA", "02f945d87b7cd6e9f2173a110399d36b369edb1f10bdf5a4ba6fd4923e2986e137" },
        {"dragonhound_NA", "0366a87a476a09e05560c5aae0e44d2ab9ba56e69701cee24307871ddd37c86258" },
        {"hyper_NA", "0303503ea8f5ec8bcab474962dfadd3561b44732b6ad308acd8d04276dd2f1baf3" },
        {"madmax_NA", "0378e47061572e4a406bbad1522c03c3331d0a6c820fde1248ccf2cbc72fec47c2" },
        {"node-9_NA", "03fac1468a949244dd4c563062459d46e966479fe23748382fc2e3e8d05218023e" }, // 40
        {"nodeone_NA", "0310a249c6c2dcc29f2135715138a9ddb8e01c0eab701cbd0b96d9cec660dbdc58" },
        {"pbca26_NA", "03e8485883eba6d4f2902338ab6aac87654a4b98d3bc01f89638aaf9c37db66ccf" },
        {"ptyx_NA", "028267c92db3c48a99dfb8d88e9cdab60d8a1525913ab3978b1b629667b12b1ee2" },
        {"strob_NA", "02285bf2f9e96068ecac14bc6f770e394927b4da9f5ba833eaa9468b5d47f203a3" },
        {"karasugoi_NA", "02f803e6f159824a181cc5d709f3d1e7ff65f19e1899920724aeb4e3d2d869f911" },
        {"webworker01_NA", "03d6c76aabe24fde7ce7cc37cff0899d50a20d4147ac0b2db812e2a1edcf0d5232" },
        {"yurii_DEV", "0243977da0533c7c1a37f0f6e30175225c9012d9f3f426180ff6e5710f5a50e32b" },
        {"ca333_DEV", "035f3413d71856ac0859f564ced42fe1ce5c5058df888f4592b8a11d34a5ba3a45" },
        {"chmex_SH", "03e09c8ee6ae20cde64857d116c4bb5d50db6de2887ac39ea3ccf6434b1abf8698" },
        {"collider_SH", "033a1b62de10c3802f359da7767b033eac3837b58530722f3ddd2f359a2cd0a8f9" }, // 50
        {"dappvader_SH", "02684e2e7425ffa36d331f7a2f9c4542b61e88370dc6b4313a5025643f82ee17fa" },
        {"drkush_SH", "0210320b03f00f10f16313eb6e8929b5be7e66a034a4e9b7d11f2d87aa92708c6c" },
        {"majora31_SH", "03bc75c112ac7c6a99d6eb3fe5582feef4fd1b43f11c08ad887e21c4c3bc4e9104" },
        {"mcrypt_SH", "027a4ca7b11d3456ff558c08bb04483a89c7f383448461fd0b6b3b07424aabe9a4" },
        {"metaphilibert_SH", "03b21ff042bf1730b28bde43f44c064578b41996117ac7634b567c3773089e3be3" },
        {"mylo_SH", "026a52dba25ca4deb225a5ef7fca117d59e20ef2319b00e1bb6750a5d61e5ed601" },
        {"nutellaLicka_SH", "03ca46ea9a32de632823419948188088069f5820023920d810da6076624adb9901" },
        {"pbca26_SH", "021b39173b2b966ab277799a1f148a1d9e6cf26020f5f7eb9708f020ee0461e9c0" },
        {"phit_SH", "021b893b7978284e3d73701a623f23104fcce27e70fb49427c215f9a7481f652da" },
        {"sheeba_SH", "030dd2c3c02cbc5b3c25e3c54ed02c1541951a6f5ecf8adbd353e8d9052d08b8fc" }, // 60
        {"strob_SH", "0213751a1c59d3489ca85b3d62a3d606dcef7f0428aa021c1978ea16fb38a2fad6" },
        {"strobnidan_SH", "033e33ef18effb979437cd202bb87dc32385e16ebd52d6f762d8a3b308d6f89c52" },
        {"dragonhound_DEV", "02b3c168ed4acd96594288cee3114c77de51b6afe1ab6a866887a13a96ee80f33c" }
   },
   {
        // Season 6
        {"blackice_DEV", "03e2de3418c88be0cfe2fa0dcfdaea001b5a36ad86e6833ad284d79021ae7e2b94"},
        {"blackice_AR", "03949b06c2773b4573aeb0b52e70ccc2d98dc5794a47e24eeb902c9d28e0e8d28b"},
        {"alien_EU", "022b85908191788f409506ebcf96a892f3274f352864c3ed566c5a16de63953236"},
        {"alien_NA", "022f62b56ddfd07c9860921c701285ac39bb3ac8f6f083d1b59c8f4943be3de162"},
        {"alien_SH", "024f20c096b085308e21893383f44b4faf1cdedea9ad53cc7d7e7fbfa0c30c1e71"},
        {"alienx_EU", "025de0911bab55616c307f02ea8a5915a2e0c8e479aa97968e7f00d1025cbe6c6d"},
        {"alienx_NA", "025d5e11725233ab161f4f63d697c5f9f0c6b9d3aa2b9c68299638f8cc63faa9c2"},
        {"artem.pikulin_AR", "03a45c4ad7f279cbc50acb48d81fc0eb63c4c5f556e3a4393fb3d6414df09c6e4c"},
        {"artem.pikulin_DEV", "025ee88d1c12f546c1c8942d7a3e0678f10bc27cc566e27bf4a2d2178e018d18c6"},
        {"blackice_EU", "025f8de3a6181270ceb5c31654e6a6e95d0339bc14b46b5e3050e8a69861c91baa"},
        {"chmex_AR", "030cd487e10fbf142e0e8d582e702ecb775f378569c3cb5acd0ff97b6b12803588"},
        {"chmex_EU", "030bf7bd7ad0515c33b5d5d9a91e0729baf801b9002f80495ae535ea1cebb352cb"},
        {"chmex_NA", "024e88a36d729352a391e07d1821dbfda1fca6409320cf9c2869b6fb99f05fbddd"},
        {"chmex_SH", "03e09c8ee6ae20cde64857d116c4bb5d50db6de2887ac39ea3ccf6434b1abf8698"},
        {"chmex1_SH", "02d59db293de6c7da6673beeb373ebce62fd6d3522f715ea1356b5a2624fbd11a2"},
        {"cipi_1_EU", "033a812d6cccdc4208378728f3a0e15db5b12074def9ab686ddc3752715ff1a194"},
        {"cipi_2_EU", "0302ca28a041ed00544de737651bdec9bafe3b7f1c0bf2c6092f2368d59fec75c2"},
        {"cipi_AR", "02336758998f474659020e6887ece61ac7b8567f9b2d38724ebf77ae800c1fb2b7"},
        {"cipi_NA", "0335352862da521bd90b99d394db1ee3ecde379db9cf7ba2f28b16fa76153e289f"},
        {"computergenie_EU", "033a2474a762700b452b96a49730280040a9872070bc51461e3727f6f118ff5358"},
        {"computergenie_NA", "02f945d87b7cd6e9f2173a110399d36b369edb1f10bdf5a4ba6fd4923e2986e137"},
        {"dimxy_AR", "0337e443df52f278f313f90628aaaa7a8db777f17bc7ce519069eb72717c1c2755"},
        {"dimxy_DEV", "03a7edd6d0ba188960e39eced4d6b4ca6946bd98323ab40cbc13d6e52696de7dc4"},
        {"dragonhound_NA", "0366a87a476a09e05560c5aae0e44d2ab9ba56e69701cee24307871ddd37c86258"},
        {"fediakash_AR", "035be6a54242a53e3ca55bd63430ac9b960fbfaad336d8c1464b5802f03ab184be"},
        {"gcharang_DEV", "03a3878af1152f648e6084fd3fbe697a26b1c2e92d407dd96c375f45f7d3ca13bf"},
        {"gcharang_SH", "0321868e0eb39271330fa2c3a9f4e542275d9719f8b87773c5432448ab10d6943d"},
        {"goldenman_AR", "03e027927dd1e379c2f45624ed9b057b187fe094595fee55c53f36ed665ed566ab"},
        {"kolo_AR", "032c2a6aeaf8176f9630abe2e1bbea5e481da23ab1f9a5b10f220a9b2bc9607bd6"},
        {"kolo_EU", "03b21717e84c0a6cf22b557b198daba4f78980048211b9139b4517fa08f9f17359"},
        {"kolox_AR", "038010e24e6d53f26871d31287f446f407fa87596d946bcd1f7b7b8458e34ad73f"},
        {"komodopioneers_EU", "02fb31b130babe79ac780a6118702555a8c66875835f35c2232a6cb8b1438fe71d"},
        {"madmax_DEV", "02d1ace4a25794de5a5287edeb9df9619dc97020114801c5cb287b1efdd49ddbb3"},
        {"marmarachain_EU", "02ca3e0618bc7c75afa6359ae476ee639682adfaa6fc463bbe7016c4f00da23ccf"},
        {"mcrypt_AR", "02845d016c68c3e5ce924b164abc271511f3092ae359677a515e8f81a9533472f4"},
        {"mcrypt_SH", "027a4ca7b11d3456ff558c08bb04483a89c7f383448461fd0b6b3b07424aabe9a4"},
        {"metaphilibert_SH", "03b21ff042bf1730b28bde43f44c064578b41996117ac7634b567c3773089e3be3"},
        {"mylo_NA", "02493412e6014d2bb985b1026e31204e7634a211a16454fc696f4dc4dfe409e998"},
        {"mylo_SH", "026a52dba25ca4deb225a5ef7fca117d59e20ef2319b00e1bb6750a5d61e5ed601"},
        {"nodeone_NA", "0310a249c6c2dcc29f2135715138a9ddb8e01c0eab701cbd0b96d9cec660dbdc58"},
        {"nutellalicka_AR", "03a5502c61839a34edc8443aee5c16c67d4a958874e13d9b6b69d29075354739ad"},
        {"nutellalicka_SH", "03c7822f0c3434037fa17bfa4d3b7f9d3e0eb9abe49e8cc1188d92c6b3c360a675"},
        {"ocean_AR", "02d216e72d37a38449d661413cbc6e1f008b21cffdb06865f7be636e2cbc1e5346"},
        {"pbca26_NA", "030b6515e80aaa489215875e2aa6f2a15fa36cb3342580e885275f8c636252cbc8"},
        {"pbca26_SH", "02d17840724692a9d5e72d61d001c31eb5ddc4d2f0f104b760e6854bf144e63fb8"},
        {"phit_SH", "021b893b7978284e3d73701a623f23104fcce27e70fb49427c215f9a7481f652da"},
        {"ptyx_NA", "025d72ef0f1c5103de306d536360bcf3c5b6129c2046334b21cec8202e9bfc4baf"},
        {"ptyx2_NA", "02913b3af6f67453eaff2318f3eed0fa0ea60eb2b37c8da64c575fec972f178d9b"},
        {"sheeba_SH", "030dd2c3c02cbc5b3c25e3c54ed02c1541951a6f5ecf8adbd353e8d9052d08b8fc"},
        {"smdmitry_AR", "0397b7584cb29717b721c0c587d4462477efc1f36a56921f133c9d17b0cd7f278a"},
        {"smdmitry_EU", "0338f30ca34d0aca0d79b69abde447036aaaa75f482b6c75801fd382e984337d01"},
        {"smdmitry_SH", "03f7d5ac650baaccedab959adf7c4f416584f4c05a37bf079f710227560c456978"},
        {"strob_SH", "0213751a1c59d3489ca85b3d62a3d606dcef7f0428aa021c1978ea16fb38a2fad6"},
        {"strobnidan_SH", "033e33ef18effb979437cd202bb87dc32385e16ebd52d6f762d8a3b308d6f89c52"},
        {"tokel_NA", "0315d866c09e15709e2036a05faec6aaf28e0ecf67329b2adb922b746f22e694bc"},
        {"tonyl_AR", "02e2d9ecdc9f462a4767f7dfe8ed243c98fcccc1511989a60e3f859dc6fda42d16"},
        {"tonyl_DEV", "0399c4f8c5b604cda64c1ccb8fdbd7a89730131519f87491a79b0811e619102d8f"},
        {"van_EU", "0370305b9e91d46331da202ae733d6050d01038ef6eceb2036ada394a48fae84b9"},
        {"webworker01_EU", "0390ba250f20b5849b9d37ad2bcb58d3d9a3a0385937e652c26511ba9f445f5db1"},
        {"webworker01_NA", "03bde0df98de0ff9697b7d5ecea225fd7267f55c061caa2c43a47b313e35c9b6c6"},
        {"who-biz_NA", "0268d30efafc6ac84b1c8210e99fd4936e178794581d348b87f64fcbbfa8d5e73b"},
        {"yurii-khi_DEV", "0243977da0533c7c1a37f0f6e30175225c9012d9f3f426180ff6e5710f5a50e32b"},
        {"ca333_EU", "03c34a62c8c89889e4529962578e0b75a010a6e1d9bcbe8f4bb9cc680d82c7261e"},
        {"dragonhound_DEV", "02b3c168ed4acd96594288cee3114c77de51b6afe1ab6a866887a13a96ee80f33c"},
   }, 
   {
        // Season 7
		{"blackice_DEV", "035509136135ba8e3f5d4733f7a9c160c2e1fefd8dc4658c3d95a5407e8da14749"},
		{"blackice_AR", "032674b15524dab1c7a5824aa9d3d38f231a8a04095e11920677ee99d8197d9c60"},
		{"blackice_EU", "0271663454ffe07b7a13f25c93482bb554bab646627ee78941f6e59473a423e9c5"},
		{"blackice_NA", "035e356c96d4bc8ddd11109f679b44034fdd22003b87a8deeaae6ba2bb938f7e05"},
		{"alien_NA", "022f62b56ddfd07c9860921c701285ac39bb3ac8f6f083d1b59c8f4943be3de162"},
		{"alien_EU", "022b85908191788f409506ebcf96a892f3274f352864c3ed566c5a16de63953236"},
		{"alien_SH", "024f20c096b085308e21893383f44b4faf1cdedea9ad53cc7d7e7fbfa0c30c1e71"},
		{"alienx_NA", "025d5e11725233ab161f4f63d697c5f9f0c6b9d3aa2b9c68299638f8cc63faa9c2"},
		{"alright_EU", "036d2b943e386bd855780b2e81a9f358b684884f396e653eb93b83d2f2ce06b4f7"},
		{"alright_DEV", "02b698fee0945be7c2a489fc2ef93f956e491d5352228e5b4612219bba36590716"},
		{"artem.pikulin_AR", "03a45c4ad7f279cbc50acb48d81fc0eb63c4c5f556e3a4393fb3d6414df09c6e4c"},
		{"batman_AR", "0201b3f4d90f9c49e8c54adaee880c05c6b7d83ec5ce2e371c509308acd9c7bfdc"},
		{"blackice2_AR", "022b10b7ec56b5c9f12bc9db1055edb3c2c0e0530453b7cc05239e4a3c442e80c4"},
		{"ca333_EU", "030d0ae0016a160ffedb40ffbc4e7421d5908a8489e84e7e66d9fca7783e655384"},
		{"caglarkaya_EU", "03b872d6920bbfd59c4b20331929a8503af6f37c7445a7dea816c6c8862e9a2f02"},
		{"chmex_AR", "030cd487e10fbf142e0e8d582e702ecb775f378569c3cb5acd0ff97b6b12803588"},
		{"chmex_EU", "030bf7bd7ad0515c33b5d5d9a91e0729baf801b9002f80495ae535ea1cebb352cb"},
		{"chmex_NA", "024e88a36d729352a391e07d1821dbfda1fca6409320cf9c2869b6fb99f05fbddd"},
		{"chmex_SH", "03e09c8ee6ae20cde64857d116c4bb5d50db6de2887ac39ea3ccf6434b1abf8698"},
		{"chmex2_SH", "02d59db293de6c7da6673beeb373ebce62fd6d3522f715ea1356b5a2624fbd11a2"},
		{"cipi_AR", "02336758998f474659020e6887ece61ac7b8567f9b2d38724ebf77ae800c1fb2b7"},
		{"cipi_EU", "033a812d6cccdc4208378728f3a0e15db5b12074def9ab686ddc3752715ff1a194"},
		{"cipi_NA", "0335352862da521bd90b99d394db1ee3ecde379db9cf7ba2f28b16fa76153e289f"},
		{"colmapol_EU", "02754ae5585df0eba995f8e0aa1768b1424eaa04a3f1269a4cf37a8d4446f48a6d"},
		{"computergenie_EU", "033a2474a762700b452b96a49730280040a9872070bc51461e3727f6f118ff5358"},
		{"computergenie_NA", "02f945d87b7cd6e9f2173a110399d36b369edb1f10bdf5a4ba6fd4923e2986e137"},
		{"computergenie2_NA", "037bef188375444d4706be966e90468ffa325d6f12b2e40021c12b4bf481cf0294"},
		{"dimxy_AR", "0337e443df52f278f313f90628aaaa7a8db777f17bc7ce519069eb72717c1c2755"},
		{"dimxy_DEV", "03a7edd6d0ba188960e39eced4d6b4ca6946bd98323ab40cbc13d6e52696de7dc4"},
		{"emmaccen_DEV", "0361b883704b1fe34380eac11e3f9fdcb533534de472b9a9db961c80a96ecaffbc"},
		{"fediakash_AR", "035be6a54242a53e3ca55bd63430ac9b960fbfaad336d8c1464b5802f03ab184be"},
		{"gcharang_AR", "039e01651c0afa1fc80b620301ff1981dd1db0f6c6b637b8f2f0fd986e9f5ece59"},
		{"gcharang_SH", "0321868e0eb39271330fa2c3a9f4e542275d9719f8b87773c5432448ab10d6943d"},
		{"gcharang_DEV", "03a3878af1152f648e6084fd3fbe697a26b1c2e92d407dd96c375f45f7d3ca13bf"},
		{"kmdude_SH", "0253649e80366bb3a84c447ae2632d0ab272640be61ceb34291068c58531dae528"},
		{"marmara_AR", "0359809ec0774cf6a2679257c4db240b9061b08f72d9888e3323326f7428ddf93a"},
		{"marmara_EU", "02ca3e0618bc7c75afa6359ae476ee639682adfaa6fc463bbe7016c4f00da23ccf"},
		{"mcrypt_SH", "027a4ca7b11d3456ff558c08bb04483a89c7f383448461fd0b6b3b07424aabe9a4"},
		{"nodeone_NA", "0310a249c6c2dcc29f2135715138a9ddb8e01c0eab701cbd0b96d9cec660dbdc58"},
		{"nodeone2_NA", "026b8ae180e5e927fbe0cd89606d73df739288501d36d8ed4435d68f66dfcecc08"},
		{"ozkanonur_NA", "03259f6aa4d3c451a52b7327bff4af9f649be3477459997b52711b54f4c8fb6394"},
		{"pbca26_NA", "0357a82231480c343c043fe71a775e233e3de5e01b00bbec5f8061997a2d027650"},
		{"pbca26_SH", "023404b4ca207c68acc5c8b08dafb0b43bfd39697bd70c9136093712984a994be5"},
		{"phit_SH", "021b893b7978284e3d73701a623f23104fcce27e70fb49427c215f9a7481f652da"},
		{"ptyx_SH", "03a9e2a517d2e55650dca99035601c2738406010b8d4b7f042d3e8b3b3680927b0"},
		{"shamardy_SH", "03589e28ad013c6ed3df0ed9947ecc2b6873a211be8aa3a745d3fea459b12a6435"},
		{"sheeba_SH", "030dd2c3c02cbc5b3c25e3c54ed02c1541951a6f5ecf8adbd353e8d9052d08b8fc"},
		{"sheeba2_SH", "028bf664c7e9ae6c3571c0796dc4197ce75d4a161ba16e3b5e3dac1c4825730c91"},
		{"smdmitry_AR", "0397b7584cb29717b721c0c587d4462477efc1f36a56921f133c9d17b0cd7f278a"},
		{"smdmitry_EU", "0338f30ca34d0aca0d79b69abde447036aaaa75f482b6c75801fd382e984337d01"},
		{"smdmitry_SH", "03f7d5ac650baaccedab959adf7c4f416584f4c05a37bf079f710227560c456978"},
		{"smdmitry2_AR", "03c1efa0a64392e68cf50a13e4611b272b914cfba1e07f49f94389db3bac4497de"},
		{"strob_SH", "0213751a1c59d3489ca85b3d62a3d606dcef7f0428aa021c1978ea16fb38a2fad6"},
		{"tonyl_AR", "0380edd4bc37635cbed92c5510ebdf35653d0b8f5fac41cfc29cead1de993b6b27"},
		{"tonyl_DEV", "0301023f890de7c514d3acfc1710acf9a14c5b9edd06f97696c011b678a199bbbc"},
		{"van_EU", "0370305b9e91d46331da202ae733d6050d01038ef6eceb2036ada394a48fae84b9"},
		{"webworker01_EU", "03de3f5bfd58b947e790d6c726afbef240e08d04020bb447d0d31fd40e50caeceb"},
		{"webworker01_NA", "031d46df12ac739d33748d9191a9ea7433f13109463331739cf3accc801294ac7f"},
		{"who-biz_NA", "0268d30efafc6ac84b1c8210e99fd4936e178794581d348b87f64fcbbfa8d5e73b"},
		{"yurri-khi_DEV", "0243977da0533c7c1a37f0f6e30175225c9012d9f3f426180ff6e5710f5a50e32b"},
		{"dragonhound_AR", "039bb16266b0216264e7d3ccae12633105e1c14bd5d0e144e8b9c2b6d298a6c545"},
		{"dragonhound_EU", "0382d7e027bf5cda31264867f4e389a1a72f6671c7bd8818beb35b874d906e22d5"},
		{"dragonhound_NA", "02b4bf90b57f7c8363043c7ea8b02c8c776301ef3c1abb2df1caa2e6947be1422b"},
		{"dragonhound_DEV", "02a473e980bf0d198ece8ed11f1ecbe437edb688de6c83b82efa6f7de3a5d43c19"}
   },
{
        // Season 8
		{"alright_DEV", "02b698fee0945be7c2a489fc2ef93f956e491d5352228e5b4612219bba36590716"},
		{"alien_AR", "03ae0b483a41bafd5ac9820ef15ffe60091f795726fd0994dea4043dfa9863fb46"},
		{"alien_EU", "022b85908191788f409506ebcf96a892f3274f352864c3ed566c5a16de63953236"},
		{"alien_NA", "022f62b56ddfd07c9860921c701285ac39bb3ac8f6f083d1b59c8f4943be3de162"},
		{"alien_SH", "024f20c096b085308e21893383f44b4faf1cdedea9ad53cc7d7e7fbfa0c30c1e71"},
		{"alienx_EU", "025de0911bab55616c307f02ea8a5915a2e0c8e479aa97968e7f00d1025cbe6c6d"},
		{"alienx_NA", "025d5e11725233ab161f4f63d697c5f9f0c6b9d3aa2b9c68299638f8cc63faa9c2"},
		{"artist_SH", "03f0a59c15984d7627a38a7ee75fb4f4c41c6c6db58ff245efb429a1e7820c51dc"},
		{"batman_EU", "0373c4bf6cadb907597ef11d37d040cdb5dc73486beb0b8902837448f4087e0b7a"},
		{"blackice_AR", "032674b15524dab1c7a5824aa9d3d38f231a8a04095e11920677ee99d8197d9c60"},
		{"blackice_EU", "0271663454ffe07b7a13f25c93482bb554bab646627ee78941f6e59473a423e9c5"},
		{"blackice_NA", "035e356c96d4bc8ddd11109f679b44034fdd22003b87a8deeaae6ba2bb938f7e05"},
		{"blackice_SH", "0320b53f460e2867aacc578432d7ad5deb8f6f0da8e9c5ae65711011d692838bd4"},
		{"blackice2_AR", "022b10b7ec56b5c9f12bc9db1055edb3c2c0e0530453b7cc05239e4a3c442e80c4"},
		{"ca333_EU", "02da199f5da76cee5f09b4065e12ed454876308195cf45103e55f9837b4a1422d5"},
		{"chmex_AR", "030cd487e10fbf142e0e8d582e702ecb775f378569c3cb5acd0ff97b6b12803588"},
		{"chmex_EU", "030bf7bd7ad0515c33b5d5d9a91e0729baf801b9002f80495ae535ea1cebb352cb"},
		{"chmex_NA", "024e88a36d729352a391e07d1821dbfda1fca6409320cf9c2869b6fb99f05fbddd"},
		{"chmex_SH", "03e09c8ee6ae20cde64857d116c4bb5d50db6de2887ac39ea3ccf6434b1abf8698"},
		{"chmexvet_SH", "02d59db293de6c7da6673beeb373ebce62fd6d3522f715ea1356b5a2624fbd11a2"},
		{"cipi_AR", "02336758998f474659020e6887ece61ac7b8567f9b2d38724ebf77ae800c1fb2b7"},
		{"cipi_EU", "033a812d6cccdc4208378728f3a0e15db5b12074def9ab686ddc3752715ff1a194"},
		{"cipi_NA", "0335352862da521bd90b99d394db1ee3ecde379db9cf7ba2f28b16fa76153e289f"},
		{"cipi2_NA", "03b267592b0cb91d115524e7412e9ea79b30fbc8e29b8dcbbf850110f2e1d8c1ab"},
		{"colmapol_EU", "02754ae5585df0eba995f8e0aa1768b1424eaa04a3f1269a4cf37a8d4446f48a6d"},
		{"dimxy_AR", "0337e443df52f278f313f90628aaaa7a8db777f17bc7ce519069eb72717c1c2755"},
		{"dragonhound_AR", "039bb16266b0216264e7d3ccae12633105e1c14bd5d0e144e8b9c2b6d298a6c545"},
		{"dragonhound_EU", "0382d7e027bf5cda31264867f4e389a1a72f6671c7bd8818beb35b874d906e22d5"},
		{"dragonhound_NA", "02b4bf90b57f7c8363043c7ea8b02c8c776301ef3c1abb2df1caa2e6947be1422b"},
		{"dragonhound_SH", "02125e737f34c426474c9d29ec280bf49dc5ac99bffb6296b71a5031f58485ec86"},
		{"fediakash_AR", "035be6a54242a53e3ca55bd63430ac9b960fbfaad336d8c1464b5802f03ab184be"},
		{"fediakash_NA", "032485e92b4f06613c43123a178de96386e130311030958a736805d7b278df2f11"},
		{"francois_SH", "0285e1699d28cc6d134e476d85ee37badabc795f136df39e414fe6ca2b204e3a2f"},
		{"gcharang_AR", "039e01651c0afa1fc80b620301ff1981dd1db0f6c6b637b8f2f0fd986e9f5ece59"},
		{"gcharang_SH", "0321868e0eb39271330fa2c3a9f4e542275d9719f8b87773c5432448ab10d6943d"},
		{"gcharang2_SH", "0261b0337647329677b9885b050069a4737f139266c55ec711148af757a0276bc5"},
		{"kmdude_SH", "0253649e80366bb3a84c447ae2632d0ab272640be61ceb34291068c58531dae528"},
		{"marmarachain_AR", "0359809ec0774cf6a2679257c4db240b9061b08f72d9888e3323326f7428ddf93a"},
		{"marmarachain_EU", "02ca3e0618bc7c75afa6359ae476ee639682adfaa6fc463bbe7016c4f00da23ccf"},
		{"mcrypt_NA", "038e6f54c15eda9844e956206138494312648da158280a837341894fc642d8066a"},
		{"naezith_EU", "0310f58935bc72ed76f90301f8bcd8caea576603275493b9c664aa9ed25aa492c6"},
		{"naezith_NA", "031e48218c2e7fea3c852a3674848786c05d06fd451da4e14a6193363cf89febce"},
		{"onur-ozkan_EU", "02387f4093ba78ea6aa9023bd2b3e1d57a4d9d5e745a9cc6cb1bca2565c22f8c89"},
		{"onur-ozkan_NA", "03259f6aa4d3c451a52b7327bff4af9f649be3477459997b52711b54f4c8fb6394"},
		{"pbca26_NA", "025349e2ef906bb8dff17275d5d1034edf261d7b3b0fef02643c4e7227e161cd54"},
		{"ptyx_SH", "02eeaea84667462ae3e0c9262037666af81200b7982682ea06b690b1ac32a48c83"},
		{"shamardy_SH", "03589e28ad013c6ed3df0ed9947ecc2b6873a211be8aa3a745d3fea459b12a6435"},
		{"smdmitry_AR", "0397b7584cb29717b721c0c587d4462477efc1f36a56921f133c9d17b0cd7f278a"},
		{"smdmitry2_AR", "03c1efa0a64392e68cf50a13e4611b272b914cfba1e07f49f94389db3bac4497de"},
		{"smdmitry_EU", "0338f30ca34d0aca0d79b69abde447036aaaa75f482b6c75801fd382e984337d01"},
		{"smdmitry_NA", "038429f7abe33af2d6841029710b835223a05830fc8eee29521ec051e2f2d2e517"},
		{"smdmitry_SH", "03f7d5ac650baaccedab959adf7c4f416584f4c05a37bf079f710227560c456978"},
		{"strob_SH", "0213751a1c59d3489ca85b3d62a3d606dcef7f0428aa021c1978ea16fb38a2fad6"},
		{"gcharang_DEV", "03a3878af1152f648e6084fd3fbe697a26b1c2e92d407dd96c375f45f7d3ca13bf"},
		{"dimxy_DEV", "03a7edd6d0ba188960e39eced4d6b4ca6946bd98323ab40cbc13d6e52696de7dc4"},
		{"ca333_DEV", "0387b2a4c99127e871efeed28b73d6cf8296bb4aa8630e2cfb16490fdc47e0a561"},
		{"qa_DEV", "02ba8283b22c6572d83fd852e04928621dd0e2dd53150b5636ba64404ea73423bb"},
		{"core_DEV", "035509136135ba8e3f5d4733f7a9c160c2e1fefd8dc4658c3d95a5407e8da14749"},
		{"security_DEV", "037f666a67135f748ca3ffec29394662c71e917709bf61fee15a363655a8dff57e"},
		{"tonyl_AR", "03643e81eb0ac245c2caefe013b4357979f69ea585d6803182b230790dbcfb9ecf"},
		{"van_EU", "0370305b9e91d46331da202ae733d6050d01038ef6eceb2036ada394a48fae84b9"},
		{"who-biz_AR", "03a185b059983a5479e3adba44eb5dde8d899468632ba30480301a9f1bbbcbc4a8"},
		{"who-biz_NA", "026d5cecdee3021a80416c3d8ac68e59fd3d77a954bbe88a8eee794ade6c58f411"},
		{"dragonhound_DEV", "02a473e980bf0d198ece8ed11f1ecbe437edb688de6c83b82efa6f7de3a5d43c19"}
   }
};

#define SETBIT(bits,bitoffset) (((uint8_t *)bits)[(bitoffset) >> 3] |= (1 << ((bitoffset) & 7)))
#define GETBIT(bits,bitoffset) (((uint8_t *)bits)[(bitoffset) >> 3] & (1 << ((bitoffset) & 7)))
#define CLEARBIT(bits,bitoffset) (((uint8_t *)bits)[(bitoffset) >> 3] &= ~(1 << ((bitoffset) & 7)))

#define KOMODO_MAXNVALUE (((uint64_t)1 << 63) - 1)
#define KOMODO_BIT63SET(x) ((x) & ((uint64_t)1 << 63))
#define KOMODO_VALUETOOBIG(x) ((x) > (uint64_t)10000000001*COIN)

#ifdef TESTMODE_PRICES
    #define PRICES_DAYWINDOW (7)
#else
    #define PRICES_DAYWINDOW ((3600*24/ASSETCHAINS_BLOCKTIME) + 1)
#endif

extern uint8_t ASSETCHAINS_TXPOW;
extern int8_t ASSETCHAINS_ADAPTIVEPOW;
int32_t MAX_BLOCK_SIZE(int32_t height);
extern char ASSETCHAINS_SYMBOL[KOMODO_ASSETCHAIN_MAXLEN];
extern uint16_t ASSETCHAINS_P2PPORT,ASSETCHAINS_RPCPORT;
extern uint32_t ASSETCHAIN_INIT, ASSETCHAINS_MAGIC;
extern int32_t VERUS_BLOCK_POSUNITS, ASSETCHAINS_LWMAPOS, ASSETCHAINS_SAPLING, ASSETCHAINS_OVERWINTER,ASSETCHAINS_BLOCKTIME;
extern uint64_t ASSETCHAINS_SUPPLY, ASSETCHAINS_FOUNDERS_REWARD;

extern uint64_t ASSETCHAINS_TIMELOCKGTE;
extern uint32_t ASSETCHAINS_ALGO, ASSETCHAINS_VERUSHASH,ASSETCHAINS_EQUIHASH,KOMODO_INITDONE;

extern int32_t KOMODO_MININGTHREADS,KOMODO_LONGESTCHAIN,ASSETCHAINS_SEED,IS_KOMODO_NOTARY,USE_EXTERNAL_PUBKEY,KOMODO_CHOSEN_ONE,KOMODO_ON_DEMAND,KOMODO_PASSPORT_INITDONE,ASSETCHAINS_STAKED,KOMODO_NSPV;
extern uint64_t ASSETCHAINS_COMMISSION, ASSETCHAINS_LASTERA,ASSETCHAINS_CBOPRET;
extern bool VERUS_MINTBLOCKS;
extern uint64_t ASSETCHAINS_REWARD[ASSETCHAINS_MAX_ERAS+1], ASSETCHAINS_NOTARY_PAY[ASSETCHAINS_MAX_ERAS+1], ASSETCHAINS_TIMELOCKGTE, ASSETCHAINS_NONCEMASK[],ASSETCHAINS_NK[2];
extern const char *ASSETCHAINS_ALGORITHMS[];
extern int32_t VERUS_MIN_STAKEAGE;
extern uint32_t ASSETCHAINS_VERUSHASH, ASSETCHAINS_VERUSHASHV1_1, ASSETCHAINS_NONCESHIFT[], ASSETCHAINS_HASHESPERROUND[];
extern std::string NOTARY_PUBKEY,ASSETCHAINS_OVERRIDE_PUBKEY,ASSETCHAINS_SCRIPTPUB;
extern uint8_t NOTARY_PUBKEY33[33],ASSETCHAINS_OVERRIDE_PUBKEY33[33];
extern std::vector<std::string> ASSETCHAINS_PRICES, ASSETCHAINS_STOCKS;
extern std::vector<uint8_t> Mineropret; // opreturn data set by the data gathering code
extern uint64_t ASSETCHAINS_PEGSCCPARAMS[3];
extern uint8_t ASSETCHAINS_OVERRIDE_PUBKEYHASH[];

extern int32_t VERUS_BLOCK_POSUNITS, VERUS_CONSECUTIVE_POS_THRESHOLD, VERUS_NOPOS_THRESHHOLD;
extern uint256 KOMODO_EARLYTXID;

extern int32_t KOMODO_CONNECTING,KOMODO_CCACTIVATE,KOMODO_DEALERNODE,KOMODO_DEX_P2P;
extern uint32_t ASSETCHAINS_CC;
extern std::string ASSETCHAINS_CCLIB;
extern uint8_t ASSETCHAINS_CCDISABLES[256],ASSETCHAINS_CCZEROTXFEE[256];

extern int32_t USE_EXTERNAL_PUBKEY;
extern std::string NOTARY_PUBKEY,NOTARY_ADDRESS;
extern int32_t KOMODO_EXCHANGEWALLET;
extern int32_t VERUS_MIN_STAKEAGE;
extern std::string DONATION_PUBKEY;
extern uint8_t ASSETCHAINS_PRIVATE;
extern int32_t USE_EXTERNAL_PUBKEY;
extern char NOTARYADDRS[64][64];
extern char NOTARY_ADDRESSES[NUM_KMD_SEASONS][64][64];
extern int32_t KOMODO_TESTNODE, KOMODO_SNAPSHOT_INTERVAL,IS_STAKED_NOTARY,STAKED_ERA;
extern int32_t ASSETCHAINS_EARLYTXIDCONTRACT;
extern int32_t ASSETCHAINS_STAKED_SPLIT_PERCENTAGE;
int tx_height( const uint256 &hash );
extern std::vector<std::string> vWhiteListAddress;
extern std::map <std::int8_t, int32_t> mapHeightEvalActivate;
void komodo_netevent(std::vector<uint8_t> payload);
int32_t getacseason(uint32_t timestamp);
int32_t getkmdseason(int32_t height);

#define KOMODO_KVDURATION 1440
#define KOMODO_KVBINARY 2
#define PRICES_SMOOTHWIDTH 1
#define PRICES_MAXDATAPOINTS 8
uint64_t komodo_paxprice(uint64_t *seedp,int32_t height,char *base,char *rel,uint64_t basevolume);
int32_t komodo_paxprices(int32_t *heights,uint64_t *prices,int32_t max,char *base,char *rel);
int32_t komodo_notaries(uint8_t pubkeys[64][33],int32_t height,uint32_t timestamp);
char *bitcoin_address(char *coinaddr,uint8_t addrtype,uint8_t *pubkey_or_rmd160,int32_t len);
int32_t komodo_minerids(uint8_t *minerids,int32_t height,int32_t width);

uint32_t komodo_blocktime(uint256 hash);
int32_t komodo_dpowconfs(int32_t height,int32_t numconfs);
int8_t komodo_segid(int32_t nocache,int32_t height);
int32_t komodo_heightpricebits(uint64_t *seedp,uint32_t *heightbits,int32_t nHeight);
char *komodo_pricename(char *name,int32_t ind);
int32_t komodo_priceind(const char *symbol);
int32_t komodo_pricesinit();
int64_t komodo_priceave(int64_t *tmpbuf,int64_t *correlated,int32_t cskip);
int64_t komodo_pricecorrelated(uint64_t seed,int32_t ind,uint32_t *rawprices,int32_t rawskip,uint32_t *nonzprices,int32_t smoothwidth);
uint32_t komodo_heightstamp(int32_t height);
int64_t komodo_pricemult_to10e8(int32_t ind);
int32_t komodo_priceget(int64_t *buf64,int32_t ind,int32_t height,int32_t numblocks);
uint64_t komodo_accrued_interest(int32_t *txheightp,uint32_t *locktimep,uint256 hash,int32_t n,int32_t checkheight,uint64_t checkvalue,int32_t tipheight);
int32_t komodo_currentheight();
int32_t komodo_notarized_bracket(struct notarized_checkpoint *nps[2],int32_t height);
arith_uint256 komodo_adaptivepow_target(int32_t height,arith_uint256 bnTarget,uint32_t nTime);
bool komodo_hardfork_active(uint32_t time);
int32_t komodo_newStakerActive(int32_t height, uint32_t timestamp);

CBlockIndex *komodo_getblockindex(uint256 hash);
uint256 Parseuint256(const char *hexstr);
void komodo_sendmessage(int32_t minpeers, int32_t maxpeers, const char *message, std::vector<uint8_t> payload);
CBlockIndex *komodo_getblockindex(uint256 hash);
int32_t komodo_nextheight();
CBlockIndex *komodo_blockindex(uint256 hash);
CBlockIndex *komodo_chainactive(int32_t height);
int32_t komodo_blockheight(uint256 hash);
int64_t komodo_get_blocktime(uint256 hash);
bool komodo_txnotarizedconfirmed(uint256 txid,int32_t minconfirms=1);
int32_t komodo_blockload(CBlock& block, CBlockIndex *pindex);
uint32_t komodo_chainactive_timestamp();
uint32_t GetLatestTimestamp(int32_t height);
int32_t komodo_get_current_height();
struct komodo_state *komodo_stateptrget(char *base);

void komodo_prefetch(FILE *fp);
void komodo_stateupdate(int32_t height,uint8_t notarypubs[][33],uint8_t numnotaries,uint8_t notaryid,uint256 txhash,uint64_t voutmask,uint8_t numvouts,uint32_t *pvals,uint8_t numpvals,int32_t kheight,uint32_t ktime,uint64_t opretvalue,uint8_t *opretbuf,uint16_t opretlen,uint16_t vout,uint256 MoM,int32_t MoMdepth);
void komodo_init(int32_t height);
int32_t komodo_MoMdata(int32_t *notarized_htp,uint256 *MoMp,uint256 *kmdtxidp,int32_t nHeight,uint256 *MoMoMp,int32_t *MoMoMoffsetp,int32_t *MoMoMdepthp,int32_t *kmdstartip,int32_t *kmdendip);
int32_t komodo_notarizeddata(int32_t nHeight,uint256 *notarized_hashp,uint256 *notarized_desttxidp);
char *komodo_issuemethod(char *userpass,char *method,char *params,uint16_t port);
int32_t komodo_chosennotary(int32_t *notaryidp,int32_t height,uint8_t *pubkey33,uint32_t timestamp);
int32_t komodo_isrealtime(int32_t *kmdheightp);
uint64_t komodo_paxtotal();
int32_t komodo_longestchain();
uint64_t komodo_maxallowed(int32_t baseid);
int32_t komodo_bannedset(int32_t *indallvoutsp,uint256 *array,int32_t max);
int32_t komodo_checkvout(int32_t vout,int32_t k,int32_t indallvouts);
int32_t komodo_notarized_height(int32_t *prevMoMheightp,uint256 *hashp,uint256 *txidp);
int64_t komodo_block_unlocktime(uint32_t nHeight);
int32_t komodo_notaryvin(CMutableTransaction &txNew,uint8_t *notarypub33, void *pTr);
uint64_t komodo_notarypayamount(int32_t nHeight, int64_t notarycount);
CScript komodo_mineropret(int32_t nHeight);
bool komodo_appendACscriptpub();
uint64_t komodo_commission(const CBlock *pblock,int32_t height);
int32_t komodo_staked(CMutableTransaction &txNew,uint32_t nBits,uint32_t *blocktimep,uint32_t *txtimep,uint256 *utxotxidp,int32_t *utxovoutp,uint64_t *utxovaluep,uint8_t *utxosig, uint256 merkleroot);
uint256 komodo_calcmerkleroot(CBlock *pblock, uint256 prevBlockHash, int32_t nHeight, bool fNew, CScript scriptPubKey);
int32_t verus_staked(CBlock *pBlock, CMutableTransaction &txNew, uint32_t &nBits, arith_uint256 &hashResult, uint8_t *utxosig, CPubKey &pk);
int32_t komodo_getnotarizedheight(uint32_t timestamp,int32_t height, uint8_t *script, int32_t len);
int32_t komodo_is_notarytx(const CTransaction& tx);
int32_t komodo_validate_interest(const CTransaction &tx,int32_t txheight,uint32_t cmptime,int32_t dispflag);
uint64_t komodo_notarypay(CMutableTransaction &txNew, std::vector<int8_t> &NotarisationNotaries, uint32_t timestamp, int32_t height, uint8_t *script, int32_t len);
CScript komodo_makeopret(CBlock *pblock, bool fNew);
int32_t notarizedtxid_height(char *dest,char *txidstr,int32_t *kmdnotarized_heightp);
int32_t komodo_whoami(char *pubkeystr,int32_t height,uint32_t timestamp);
int64_t komodo_coinsupply(int64_t *zfundsp,int64_t *sproutfundsp,int32_t height);
int32_t komodo_isnotaryvout(char *coinaddr,uint32_t tiptime);
void komodo_passport_iteration();
void komodo_cbopretupdate(int32_t forceflag);
uint64_t komodo_interestsum();
uint64_t komodo_interest(int32_t txheight, uint64_t nValue, uint32_t nLockTime, uint32_t tiptime);
bool komodo_dailysnapshot(int32_t height);
void komodo_setactivation(int32_t height);
void komodo_pricesupdate(int32_t height,CBlock *pblock);
void komodo_broadcast(CBlock *pblock,int32_t limit);
int32_t komodo_block2pubkey33(uint8_t *pubkey33,CBlock *block);
void komodo_event_rewind(struct komodo_state *sp,char *symbol,int32_t height);
int32_t komodo_connectblock(bool fJustCheck, CBlockIndex *pindex,CBlock& block) EXCLUSIVE_LOCKS_REQUIRED(cs_main) ;
arith_uint256 komodo_PoWtarget(int32_t *percPoSp,arith_uint256 target,int32_t height,int32_t goalperc,int32_t newStakerActive);
int32_t komodo_baseid(char *origbase);
int32_t komodo_eligiblenotary(uint8_t pubkeys[66][33],int32_t *mids,uint32_t *blocktimes,int32_t *nonzpkeysp,int32_t height);
uint64_t komodo_current_supply(uint32_t nHeight);

int32_t gettxout_scriptPubKey(uint8_t *scriptPubkey,int32_t maxsize,uint256 txid,int32_t n);
bool Getscriptaddress(char *destaddr,const CScript &scriptPubKey);

char *nonportable_path(char *str);
char *portable_path(char *str);
void *loadfile(char *fname,uint8_t **bufp,long *lenp,long *allocsizep);
void *filestr(long *allocsizep,char *_fname);

extern uint64_t KOMODO_INTERESTSUM, KOMODO_WALLETBALANCE;
extern int32_t KOMODO_INSYNC, KOMODO_LASTMINED, prevKOMODO_LASTMINED;
extern uint64_t ASSETCHAINS_ENDSUBSIDY[ASSETCHAINS_MAX_ERAS + 1], ASSETCHAINS_HALVING[ASSETCHAINS_MAX_ERAS + 1];
extern uint64_t ASSETCHAINS_DECAY[ASSETCHAINS_MAX_ERAS + 1];
extern uint64_t ASSETCHAINS_LINEAR, ASSETCHAINS_SUPPLY;
extern uint8_t ASSETCHAINS_PUBLIC, ASSETCHAINS_PRIVATE;
extern int32_t KOMODO_LOADINGBLOCKS;
extern int32_t ASSETCHAINS_FOUNDERS;
extern char ASSETCHAINS_USERPASS[];
extern int32_t KOMODO_REWIND;
extern uint32_t ASSETCHAINS_NUMALGOS;
extern uint32_t STAKING_MIN_DIFF;
extern uint32_t ASSETCHAINS_MINDIFF[];
extern uint64_t ASSETCHAINS_TIMEUNLOCKFROM;
extern uint64_t ASSETCHAINS_TIMEUNLOCKTO;
extern uint32_t KOMODO_DPOWCONFS;
extern uint16_t KMD_PORT, BITCOIND_RPCPORT, DEST_PORT;
extern char KMDUSERPASS[], BTCUSERPASS[];
extern uint32_t KOMODO_STOPAT;
extern int32_t ASSETCHAINS_CBMATURITY;
extern struct komodo_state KOMODO_STATES[];
extern int32_t KOMODO_EXTRASATOSHI;


#ifndef KOMODO_NSPV_FULLNODE
#define KOMODO_NSPV_FULLNODE (KOMODO_NSPV <= 0)
#endif // !KOMODO_NSPV_FULLNODE
#ifndef KOMODO_NSPV_SUPERLITE
#define KOMODO_NSPV_SUPERLITE (KOMODO_NSPV > 0)
#endif // !KOMODO_NSPV_SUPERLITE

struct komodo_staking
{
    char address[64];
    uint256 txid;
    arith_uint256 hashval;
    uint64_t nValue;
    uint32_t segid32, txtime;
    int32_t vout;
    CScript scriptPubKey;
};
struct komodo_staking *komodo_addutxo(struct komodo_staking *array, int32_t *numkp, int32_t *maxkp, uint32_t txtime, uint64_t nValue, uint256 txid, int32_t vout, char *address, uint8_t *hashbuf, CScript pk);
void komodo_createminerstransactions();
uint32_t komodo_segid32(char *coinaddr);

int32_t komodo_voutupdate(bool fJustCheck,int32_t *isratificationp,int32_t notaryid,uint8_t *scriptbuf,int32_t scriptlen,int32_t height,uint256 txhash,int32_t i,int32_t j,uint64_t *voutmaskp,int32_t *specialtxp,int32_t *notarizedheightp,uint64_t value,int32_t notarized,uint64_t signedmask,uint32_t timestamp);


// #ifndef _WIN32
void OS_randombytes(unsigned char *x, long xlen);  // this func impl exists for win too
// #endif

// curve25519 and sha256
#ifndef _BITS256
#define _BITS256
    union _bits256 { uint8_t bytes[32]; uint16_t ushorts[16]; uint32_t uints[8]; uint64_t ulongs[4]; uint64_t txid; };
    typedef union _bits256 bits256;
#endif  

bits256 curve25519_shared(bits256 privkey,bits256 otherpub);
bits256 curve25519_basepoint9();
bits256 curve25519(bits256 mysecret,bits256 basepoint);
void vcalc_sha256(char deprecated[(256 >> 3) * 2 + 1],uint8_t hash[256 >> 3],uint8_t *src,int32_t len);
bits256 bits256_doublesha256(char *deprecated,uint8_t *data,int32_t datalen);

// supernet cipher
int32_t _SuperNET_cipher(uint8_t nonce[crypto_box_NONCEBYTES],uint8_t *cipher,uint8_t *message,int32_t len,bits256 destpub,bits256 srcpriv,uint8_t *buf);
uint8_t *_SuperNET_decipher(uint8_t nonce[crypto_box_NONCEBYTES],uint8_t *cipher,uint8_t *message,int32_t len,bits256 srcpub,bits256 mypriv);
uint8_t *SuperNET_deciphercalc(uint8_t *senderpub,uint8_t **ptrp,int32_t *msglenp,bits256 privkey,uint8_t *cipher,int32_t cipherlen);
uint8_t *SuperNET_ciphercalc(uint8_t **ptrp,int32_t *cipherlenp,bits256 privkey,bits256 destpubkey,uint8_t *data,int32_t datalen);

#ifdef TESTMODE           
    #define MIN_NON_NOTARIZED_CONFIRMS 2
#else
    #define MIN_NON_NOTARIZED_CONFIRMS 101
#endif // TESTMODE


extern int32_t JUMBLR_PAUSE;
int32_t Jumblr_depositaddradd(char *depositaddr);
int32_t Jumblr_secretaddradd(char *secretaddr);


#define KOMODO_KVPROTECTED 1
#define KOMODO_KVBINARY 2
#define KOMODO_KVDURATION 1440

uint64_t PAX_fiatdest(uint64_t *seedp,int32_t tokomodo,char *destaddr,uint8_t pubkey37[37],char *coinaddr,int32_t height,char *base,int64_t fiatoshis);
int32_t komodo_opreturnscript(uint8_t *script,uint8_t type,uint8_t *opret,int32_t opretlen);
#define CRYPTO777_KMDADDR "RXL3YXG2ceaB6C5hfJcN4fvmLH2C34knhA"
extern int32_t KOMODO_PAX;
int32_t komodo_is_issuer();

// script tools analogue
#define IGUANA_READ 0
#define IGUANA_WRITE 1
int32_t iguana_rwnum(int32_t rwflag,uint8_t *serialized,int32_t len,void *endianedp);
int32_t iguana_rwbignum(int32_t rwflag,uint8_t *serialized,int32_t len,uint8_t *endianedp);
int32_t iguana_rwvarint(int32_t rwflag, uint8_t* serialized, uint64_t* varint64p);
int32_t iguana_rwbuf(int32_t rwflag,uint8_t *serialized,int32_t len,uint8_t *buf);

int32_t pax_fiatstatus(uint64_t *available,uint64_t *deposited,uint64_t *issued,uint64_t *withdrawn,uint64_t *approved,uint64_t *redeemed,char *base);

int32_t komodo_pending_withdraws(char *opretstr);
void komodo_kvupdate(uint8_t *opretbuf,int32_t opretlen,uint64_t value);
int32_t komodo_kvsearch(uint256 *refpubkeyp,int32_t current_height,uint32_t *flagsp,int32_t *heightp,uint8_t value[IGUANA_MAXSCRIPTSIZE],uint8_t *key,int32_t keylen);
int32_t komodo_kvcmp(uint8_t *refvalue,uint16_t refvaluesize,uint8_t *value,uint16_t valuesize);
uint64_t komodo_kvfee(uint32_t flags,int32_t opretlen,int32_t keylen);
uint256 komodo_kvsig(uint8_t *buf,int32_t len,uint256 privkey);
int32_t komodo_kvduration(uint32_t flags);
uint256 komodo_kvprivkey(uint256 *pubkeyp,char *passphrase);
int32_t komodo_kvsigverify(uint8_t *buf,int32_t len,uint256 _pubkey,uint256 sig);

uint64_t komodo_interestnew(int32_t txheight,uint64_t nValue,uint32_t nLockTime,uint32_t tiptime);
uint64_t _komodo_interestnew(int32_t txheight,uint64_t nValue,uint32_t nLockTime,uint32_t tiptime);

uint32_t komodo_next_tx_locktime();

#endif
