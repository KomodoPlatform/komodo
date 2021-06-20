#pragma once
#include "CCinclude.h"

struct CClibContract_info : public CCcontract_info
{
    CClibContract_info(uint8_t evalcode) : CCcontract_info()
    {
        const char* CClibNormaladdr = "RVVeUg43rNcq3mZFnvZ8yqagyzqFgUnq4u";
        if (evalcode < EVAL_FIRSTUSER || evalcode > EVAL_LASTUSER)
            throw std::logic_error("Invalid evalcode for CClib");

        this->evalcode = evalcode;
        uint8_t CClibCCpriv[32] = { 0x57, 0xcf, 0x49, 0x71, 0x7d, 0xb4, 0x15, 0x1b, 0x4f, 0x98, 
                0xc5, 0x45, 0x8d, 0x26, 0x52, 0x4b, 0x7b, 0xe9, 0xbd, 0x55, 0xd8, 0x20, 0xd6, 
                0xc4, 0x82, 0x0f, 0xf5, 0xec, 0x6c, 0x1c, 0xa0, 0xc0 };
        memcpy(CCpriv, CClibCCpriv,32);

        if ( evalcode == EVAL_FIRSTUSER ) // eventually make a hashchain for each evalcode
        {
            strcpy(CChexstr,"032447d97655da079729dc024c61088ea415b22f4c15d4810ddaf2069ac6468d2f");
            uint8_t pub33[33];
            decode_hex(pub33,33,CChexstr);
            CPubKey pk = buf2pk(pub33);
            Getscriptaddress(normaladdr,CScript() << ParseHex(HexStr(pk)) << OP_CHECKSIG);
            if ( strcmp(normaladdr,CClibNormaladdr) != 0 )
                fprintf(stderr,"CClib_initcp addr mismatch %s vs %s\n",normaladdr,CClibNormaladdr);
            GetCCaddress(this,unspendableCCaddr,pk);
            char checkaddr[64];
            uint8_t check33[33];
            if ( priv2addr(checkaddr,check33,CCpriv) != 0 )
            {
                if ( buf2pk(check33) == pk && strcmp(checkaddr,normaladdr) == 0 )
                {
                    return;
                } 
                else
                {
                    std::string msg = std::string("CClib ctor mismatched privkey -> addr ") + checkaddr + " vs " + normaladdr;
                    throw std::logic_error(msg);
                }
            }
        }
        else
        {
            for (int i = EVAL_FIRSTUSER; i<evalcode; i++)
            {
                uint8_t hash[32];
                vcalc_sha256(0,hash,CCpriv,32);
                memcpy(CCpriv,hash,32);
            }
            uint8_t pub33[33];
            if ( priv2addr(normaladdr,pub33,CCpriv) != 0 )
            {
                CPubKey pk = buf2pk(pub33);
                int i;
                for (i=0; i<33; i++)
                    sprintf(&CChexstr[i*2],"%02x",pub33[i]);
                CChexstr[i*2] = 0;
                GetCCaddress(this,unspendableCCaddr,pk);
                return;
            }
            else
            {
                throw std::logic_error("CClib ctor unable to convert private key to address");
            }
        }
    }
};