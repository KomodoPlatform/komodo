#pragma once

#include <cstdint>
#include "pubkey.h"
#include "script/script.h"

/// CC contract (Antara module) info structure that contains data used for signing and validation of cc contract transactions
struct CCcontract_info
{
    /***
     * Default ctor, sets everything to 0
     */
    CCcontract_info() { memset(this, 0, sizeof(CCcontract_info)); }
    /****
     * Constructor that sets the most common values
     * @param evalcode the type of contract
     * @param unspendable the unspendable CC address
     * @param normalAddr the normal address
     * @param hex the hex string
     * @param priv the 32 byte binary private key
     */
    CCcontract_info(const uint8_t evalcode, const char *unspendable, const char *normalAddr, const char *hex, const uint8_t *priv);

    /***
     * Get an unspendable public key
     * @param unspendablepriv filled with the private key
     * @param returns the public key
     */
    CPubKey GetUnspendable(uint8_t *unspendablepriv = nullptr);

    virtual ~CCcontract_info() {}
    uint8_t evalcode;  //!< cc contract eval code, set by ctor
    uint8_t additionalTokensEvalcode2 = 0;  //!< additional eval code for spending from three-eval-code vouts with EVAL_TOKENS, evalcode, additionalTokensEvalcode2 
                                        //!< or vouts with two evalcodes: EVAL_TOKENS, additionalTokensEvalcode2. 
                                        //!< Set by AddTokenCCInputs function

    char unspendableCCaddr[64]; //!< global contract cryptocondition address, set by ctor
    char CChexstr[72];          //!< global contract pubkey in hex, set by ctor
    char normaladdr[64];        //!< global contract normal address, set by ctor
    uint8_t CCpriv[32];         //!< global contract private key, set by ctor

    /// vars for spending from 1of2 cc.
    /// NOTE: the value of 'evalcode' member variable will be used for constructing 1of2 cryptocondition
    char coins1of2addr[64];     //!< address of 1of2 cryptocondition, set by CCaddr1of2set function
    CPubKey coins1of2pk[2];     //!< two pubkeys of 1of2 cryptocondition, set by CCaddr1of2set function
    uint8_t coins1of2priv[32];  //!< privkey for the one of two pubkeys of 1of2 cryptocondition, set by CCaddr1of2set function

    /// vars for spending from 1of2 token cc.
    /// NOTE: the value of 'evalcode' member variable will be used for constructing 1of2 token cryptocondition
    char tokens1of2addr[64];    //!< address of 1of2 token cryptocondition set by CCaddrTokens1of2set function
    CPubKey tokens1of2pk[2];    //!< two pubkeys of 1of2 token cryptocondition set by CCaddrTokens1of2set function
    uint8_t tokens1of2priv[32]; //!< privkey for the one of two pubkeys of 1of2 token cryptocondition set by CCaddrTokens1of2set function

    /// vars for spending from two additional global CC addresses of other contracts with their own eval codes
    uint8_t unspendableEvalcode2;   //!< other contract eval code, set by CCaddr2set function
    uint8_t unspendableEvalcode3;   //!< yet another other contract eval code, set by CCaddr3set function
    char    unspendableaddr2[64];   //!< other contract global cc address, set by CCaddr2set function
    char    unspendableaddr3[64];   //!< yet another contract global cc address, set by CCaddr3set function
    uint8_t unspendablepriv2[32];   //!< other contract private key for the global cc address, set by CCaddr2set function
    uint8_t unspendablepriv3[32];   //!< yet another contract private key for the global cc address, set by CCaddr3set function
    CPubKey unspendablepk2;         //!< other contract global public key, set by CCaddr2set function
    CPubKey unspendablepk3;         //!< yet another contract global public key, set by CCaddr3set function

    /// cc contract transaction validation callback that enforces the contract consensus rules
    /// @param eval object of Eval type, used to report validation error like eval->Invalid("some error");
    /// @param tx transaction object to validate
    /// @param nIn not used at this time
    virtual bool validate(Eval* eval, const CTransaction &tx, uint32_t nIn);

    /// checks if the value of evalcode in cp object is present in the scriptSig parameter, 
    /// that is, the vin for this scriptSig will be validated by the cc contract (Antara module) defined by the eval code in this CCcontract_info object
    /// @param scriptSig scriptSig to check\n
    /// Example:
    /// \code
    /// bool ccFound = false;
    /// for(int i = 0; i < tx.vin.size(); i ++)
    ///     if (cp->ismyvin(tx.vin[i].scriptSig)) {
    ///         ccFound = true;
    ///         break;
    ///     }
    /// \endcode
    virtual bool ismyvin(CScript const& scriptSig);

    /// @private
    //uint8_t didinit;
};

