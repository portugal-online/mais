#include "uair_payloads.h"
#include "network/lorawan.hpp"

class uAirUplinkMessage
{
public:
    uAirUplinkMessage(const LoRaUplinkMessage &);


    union
    {
        struct generic_payload generic;
        struct payload_type0 type0;
    } m_payload;
};
