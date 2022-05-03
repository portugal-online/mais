#include "uair_payloads.h"
#include "models/network/lorawan.hpp"
#include <ostream>

class uAirUplinkMessageType0;

class uAirUplinkMessage
{
public:
    uint8_t type() const;
public:
    static uAirUplinkMessage *create(const LoRaUplinkMessage &);

    static constexpr unsigned MAX_PAYLOAD_SIZE = 64;
    virtual void dump(std::ostream&) = 0;
protected:
    union
    {
        struct generic_payload generic;
        struct payload_type0 type0;
        uint8_t data[MAX_PAYLOAD_SIZE];
    } m_payload;
private:
    friend class uAirUplinkMessageType0;
    uAirUplinkMessage(const std::vector<uint8_t> &);
    uAirUplinkMessage &operator=(const uAirUplinkMessage &);
};


class uAirUplinkMessageType0: public uAirUplinkMessage
{
    // Accessors
public:
    uint16_t maxOAQ() const;
    uint16_t averageOAQ() const;
    bool OAQValid() const;
    bool microphoneValid() const;
    bool externalTHValid() const;
    bool internalTHValid() const;
    float averageExternalTemperature() const;
    uint8_t averageExternalHumidity() const;
    uint8_t maximumSoundLevel() const;
    uint8_t averageSoundLevel() const;
    float maximumInternalTemperature() const;
    uint8_t maximumInternalHumidity() const;

    virtual void dump(std::ostream &);

protected:
    friend class uAirUplinkMessage;
    uAirUplinkMessageType0(const std::vector<uint8_t>& data): uAirUplinkMessage(data)
    {}

};
