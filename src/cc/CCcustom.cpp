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
#include <memory>
#include "key_io.h"
#include "CCinclude.h"
#include "CCassets.h"
#include "CCfaucet.h"
#include "CCrewards.h"
#include "CCdice.h"
#include "CCauction.h"
#include "CClotto.h"
#include "CCfsm.h"
#include "CCHeir.h"
#include "CCchannels.h"
#include "CCOracles.h"
#include "CCPrices.h"
#include "CCPegs.h"
#include "CCMarmara.h"
#include "CCPayments.h"
#include "CCGateways.h"
#include "CCtokens.h"
#include "CCImportGateway.h"
#include "CClib.h"

/*
 CCcustom has most of the functions that need to be extended to create a new CC contract.
 
 A CC scriptPubKey can only be spent if it is properly signed and validated. By constraining the vins and vouts, it is possible to implement a variety of functionality. CC vouts have an otherwise non-standard form, but it is properly supported by the enhanced bitcoin protocol code as a "cryptoconditions" output and the same pubkey will create a different address.
 
 This allows creation of a special address(es) for each contract type, which has the privkey public. That allows anybody to properly sign and spend it, but with the constraints on what is allowed in the validation code, the contract functionality can be implemented.
 
 what needs to be done to add a new contract:
 1. add EVAL_CODE to eval.h
 2. initialize the variables in the CCinit function below
 3. write a Validate function to reject any unsanctioned usage of vin/vout
 4. make helper functions to create rawtx for RPC functions
 5. add rpc calls to rpcserver.cpp and rpcserver.h and in one of the rpc.cpp files
 6. add the new .cpp files to src/Makefile.am
 
 IMPORTANT: make sure that all CC inputs and CC outputs are properly accounted for and reconcile to the satoshi. The built in utxo management will enforce overall vin/vout constraints but it wont know anything about the CC constraints. That is what your Validate function needs to do.
 
 Generally speaking, there will be normal coins that change into CC outputs, CC outputs that go back to being normal coins, CC outputs that are spent to new CC outputs.
 
 Make sure both the CC coins and normal coins are preserved and follow the rules that make sense. It is a good idea to define specific roles for specific vins and vouts to reduce the complexity of validation.
 */

// to create a new CCaddr, add to rpcwallet the CCaddress and start with -pubkey= with the pubkey of the new address, with its wif already imported. set normaladdr and CChexstr. run CCaddress and it will print the privkey along with autocorrect the CCaddress. which should then update the CCaddr here


std::shared_ptr<CCcontract_info> CCinit(uint8_t evalcode)
{
    switch ( evalcode )
    {
        case EVAL_ASSETS:
            return std::make_shared<CCAssetContract_info>();
        case EVAL_FAUCET:
            return std::make_shared<CCFaucetContract_info>();
        case EVAL_REWARDS:
            return std::make_shared<CCRewardsContract_info>();
        case EVAL_DICE:
            return std::make_shared<CCDiceContract_info>();
        case EVAL_LOTTO:
            return std::make_shared<CCLottoContract_info>();
        case EVAL_FSM:
            return std::make_shared<CCFSMContract_info>();
        case EVAL_AUCTION:
            return std::make_shared<CCAuctionContract_info>();
        case EVAL_HEIR:
            return std::make_shared<CCHeirContract_info>();
        case EVAL_CHANNELS:
            return std::make_shared<CCChannelsContract_info>();
        case EVAL_ORACLES:
            return std::make_shared<CCOraclesContract_info>();
        case EVAL_PRICES:
            return std::make_shared<CCPricesContract_info>();
        case EVAL_PEGS:
            return std::make_shared<CCPegsContract_info>();
        case EVAL_MARMARA:
            return std::make_shared<CCMaramaContract_info>();
        case EVAL_PAYMENTS:
            return std::make_shared<CCPaymentsContract_info>();
        case EVAL_GATEWAYS:
            return std::make_shared<CCGatewaysContract_info>();
		case EVAL_TOKENS:
            return std::make_shared<CCTokensContract_info>();
        case EVAL_IMPORTGATEWAY:
            return std::make_shared<CCImportGatewayContract_info>();
        default:
            try
            {
                return std::make_shared<CClibContract_info>(evalcode);
            }
            catch(const std::logic_error le)
            {
                return nullptr;
            }
    }
    return nullptr;
}

