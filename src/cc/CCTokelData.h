#ifndef CC_TKLDATA_H
#define CC_TKLDATA_H

#include "CCinclude.h"

const uint8_t TKLDATA_NULL_EVAL = 0;
const uint8_t TKLDATA_VERSION = 1;
const uint64_t TKLROYALTY_DIVISOR = 1000;

// tkl Prop id:
enum tklPropId : uint8_t {
    TKLPROP_NONE = 0x0, 
    TKLPROP_ID = 0x1, 
    TKLPROP_URL = 0x2,
    TKLPROP_ROYALTY = 0x3,  
    TKLPROP_ARBITRARY = 0x4  
};

// tkl property type
enum tklPropType : uint8_t {
    TKLTYP_INVALID = 0x0, 
    TKLTYP_INT64 = 0x1, 
    TKLTYP_VUINT8 = 0x2 
};

typedef void tklWriteSS(CDataStream &ss, const UniValue &val);
typedef UniValue tklReadSS(CDataStream &ss);



typedef std::tuple<tklPropType, std::string, tklReadSS*, tklWriteSS*> tklPropDesc_t;

bool GetTokelDataAsInt64(const vuint8_t &vdata, tklPropId propId, int64_t &val);
bool GetTokelDataAsVuint8(const vuint8_t &vdata, tklPropId propId, vuint8_t &val);
vuint8_t ParseTokelJson(const UniValue &jsonParams);
UniValue ParseTokelVData(const vuint8_t &vdata);


bool TokelDataValidate(struct CCcontract_info *cp, Eval* eval, const CTransaction &tx, uint32_t nIn);

#endif
