#include <stdint.h>
#include "vc_property.h"

// required buffer size in bytes (read buffer size or write buffer size)
// -1 means buffer is variable length
// -2 for error
int vc_property_length(unsigned int property) {
    int result;
    switch(property) {
        case 0x00048001:
            result = 0;
            break;
        case 0x00000001:
        case 0x00010001:
        case 0x00010002:
        case 0x00060001:
        case 0x0003000d:
        case 0x0003000e:
        case 0x0003000f:
        case 0x00040002:
        case 0x00040005:
        case 0x00044005:
        case 0x00048005:
        case 0x00040006:
        case 0x00044006:
        case 0x00048006:
        case 0x00040007:
        case 0x00044007:
        case 0x00048007:
        case 0x00040008:
            result = 4;
            break;
        case 0x00010003:
            result = 6;
            break;
        case 0x00010004:
        case 0x00010005:
        case 0x00010006:
        case 0x00020001:
        case 0x00020002:
        case 0x00028001:
        case 0x00030001:
        case 0x00038001:
        case 0x00030002:
        case 0x00030004:
        case 0x00030007:
        case 0x00030009:
        case 0x00038009:
        case 0x00030003:
        case 0x00038003:
        case 0x00030005:
        case 0x00030008:
        case 0x00030006:
        case 0x0003000a:
        case 0x00030014:
        case 0x00040001:
        case 0x00040003:
        case 0x00044003:
        case 0x00048003:
        case 0x00040004:
        case 0x00044004:
        case 0x00048004:
        case 0x00040009:
        case 0x00044009:
        case 0x00048009:
            result = 8;
            break;
        case 0x00038002:
        case 0x0003000c:
            result = 12;
            break;
        case 0x0004000a:
        case 0x0004400a:
        case 0x0004800a:
        case 0x00008011:
            result = 16;
            break;
        case 0x00008010:
            result = 24;
            break;
        case 0x00030010:
            result = 28;
            break;
        case 0x00030020:
            result = 136;
            break;
        case 0x0004000b:
            result = 1024;
            break;
        case 0x00010007:
        case 0x00050001:
        case 0x0004400b:
        case 0x0004800b:
            result = -1;
            break;
        default :
            result = -2;
    }
    return result;
}
