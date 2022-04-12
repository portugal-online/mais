#ifndef OAQ_H__
#define OAQ_H__

#include <cstdint>
#include "ZMOD4510/oaq_2nd_gen.h"

/* Public */
struct OAQInterface
{
    virtual uint16_t getFAST_AQI() = 0;
    virtual uint16_t getEPA_AQI() = 0;
    virtual float getO3ppb() = 0;
};

namespace OAQ
{
    bool setOAQInterface(OAQInterface*);
    void unsetOAQInterface();
}

#endif
