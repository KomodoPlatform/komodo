#include "eval.h"
#include "CCutils.h" // GetCryptoCondition()
#include "CCinclude.h" // pubkey2pk
#include "utilstrencodings.h" // ParseHex

/****
 * Constructor that sets the most common values
 * @param evalcode the type of contract
 * @param unspendable the unspendable CC address
 * @param normalAddr the normal address
 * @param hex the hex string
 * @param priv the binary private key
 */
CCcontract_info::CCcontract_info(const uint8_t evalcode, const char *unspendable, const char *normalAddr, const char *hex, const uint8_t *priv)
        : CCcontract_info()
{
    this->evalcode = evalcode;
    strcpy(unspendableCCaddr, unspendable);
    strcpy(normaladdr, normalAddr);
    strcpy(CChexstr, hex);
    memcpy(CCpriv, priv,32);    
}

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
bool CCcontract_info::ismyvin(CScript const& scriptSig)
{
    CC *cond;
    if (!(cond = GetCryptoCondition(scriptSig)))
        return false;
    // Recurse the CC tree to find asset condition
    auto findEval = [] (CC *cond, struct CCVisitor visitor) {
        CCcontract_info* obj = (CCcontract_info*)visitor.context;
        bool r = cc_typeId(cond) == CC_Eval && cond->codeLength == 1 && cond->code[0] == obj->evalcode;
        // false for a match, true for continue
        return r ? 0 : 1;
    };
    CCVisitor visitor = {findEval, (uint8_t*)"", 0, this};
    bool out =! cc_visit(cond, visitor);
    cc_free(cond);
    return out;    
}

/// cc contract transaction validation callback that enforces the contract consensus rules
/// @param eval object of Eval type, used to report validation error like eval->Invalid("some error");
/// @param tx transaction object to validate
/// @param nIn not used at this time
bool CCcontract_info::validate(Eval* eval, const CTransaction &tx, uint32_t nIn)
{
    throw std::logic_error("validation not supported for eval code");
    return false; // just to keep the compiler quiet
}

/***
 * Get an unspendable public key
 * @param unspendablepriv filled with the private key
 * @param returns the public key
 */
CPubKey CCcontract_info::GetUnspendable(uint8_t *unspendablepriv)
{
    if ( unspendablepriv != 0 )
        memcpy(unspendablepriv,CCpriv,32);
    return(pubkey2pk(ParseHex(CChexstr)));
}