#include "uAirUplinkMessage.hpp"
#include <cstring>
#include <ostream>

uAirUplinkMessage *uAirUplinkMessage::create(const LoRaUplinkMessage &m)
{
    auto payload = m.data();

    uint8_t type = (payload[0]) >> 6;
    switch (type) {
    case 0:
        return new uAirUplinkMessageType0(payload);
        break;
    default:
        abort();
    }
}

uint8_t uAirUplinkMessage::type() const
{
    return m_payload.generic.payload_type;
}

uAirUplinkMessage::uAirUplinkMessage(const std::vector<uint8_t> &d)
{
    memcpy(&m_payload.data[0], d.data(), d.size());
}

void uAirUplinkMessageType0::dump(std::ostream &s)
{
    if (OAQValid())
    {
        s<<"Max OAQ             : "<<(unsigned)maxOAQ()<<std::endl;
        s<<"Average (EPA) OAQ   : "<<(unsigned)averageOAQ()<<std::endl;
    }
    else
    {
        s<<"Max OAQ             : "<< (maxOAQ() == 511 ? "STABILIZING": "Unavailable") <<std::endl;
        s<<"Average (EPA) OAQ   : "<< (averageOAQ() == 511 ? "STABILIZING": "Unavailable") <<std::endl;
    }

    if (microphoneValid())
    {
        s<<"Max sound level     : "<<(unsigned)maximumSoundLevel()<<std::endl;
        s<<"Average sound level : "<<(unsigned)averageSoundLevel()<<std::endl;
    }
    else
    {
        s<<"Max sound level     : "<<"Unavailable"<<std::endl;
        s<<"Average sound level : "<<"Unavailable"<<std::endl;
    }

    if (externalTHValid())
    {
        s<<"Average ext. temp   : "<< averageExternalTemperature()<<"C"<<std::endl;
        s<<"Average ext. hum    : "<<(unsigned)averageExternalHumidity()<<"%"<<std::endl;
    }
    else
    {
        s<<"Average ext. temp   : "<< "Unavailable"<<std::endl;
        s<<"Average ext. hum    : "<< "Unavailable"<<std::endl;
    }

    if (internalTHValid())
    {
        s<<"Max. internal temp  : "<< maximumInternalTemperature()<<"C"<<std::endl;
        s<<"Max. internal hum   : "<<(unsigned)maximumInternalHumidity()<<"%"<<std::endl;
    }
    else
    {
        s<<"Max. internal temp  : "<< "Unavailable"<<std::endl; 
        s<<"Max. internal hum   : "<< "Unavailable"<<std::endl;
    }

    /*
    bool OAQValid() const;
    bool microphoneValid() const;
    bool externalTHValid() const;
    bool internalTHValid() const;
    float averageExternalTemperature() const;
    uint8_t averageExternalHumidity() const;
    uint8_t maximumSoundLevel() const;
    uint8_t averageSoundLevel() const;
    float maximumInternalTemperature() const;
    uint8_t maxiumumInternalHumidity() const;

      */
}

unsigned uAirUplinkMessageType0::maxOAQ() const
{
    uint16_t v = m_payload.type0.max_oaq_msb;
    v <<=8 ;
    v += m_payload.type0.max_oaq_lsb;
    return v;
}

unsigned uAirUplinkMessageType0::averageOAQ() const
{
    uint16_t v = m_payload.type0.epa_oaq_msb;
    v <<=8 ;
    v += m_payload.type0.epa_oaq_lsb;
    return v;
}

bool uAirUplinkMessageType0::OAQValid() const
{
    return (m_payload.type0.health_oaq==1) ? true: false;
}
bool uAirUplinkMessageType0::microphoneValid() const
{
    return (m_payload.type0.health_microphone==1) ? true: false;
}

bool uAirUplinkMessageType0::externalTHValid() const
{
    return (m_payload.type0.health_ext_temp_hum==1) ? true: false;
}

bool uAirUplinkMessageType0::internalTHValid() const
{
    return (m_payload.type0.health_int_temp_hum==1) ? true: false;
}

float uAirUplinkMessageType0::averageExternalTemperature() const
{
    return (m_payload.type0.avg_ext_temp - 47) / 4.0;
}

unsigned uAirUplinkMessageType0::averageExternalHumidity() const
{
    return m_payload.type0.avg_ext_hum;
}

unsigned uAirUplinkMessageType0::maximumSoundLevel() const
{
    uint8_t v = m_payload.type0.max_sound_level_msb;
    v<<=4;
    v += m_payload.type0.max_sound_level_lsb;
    return v;
}

unsigned uAirUplinkMessageType0::averageSoundLevel() const
{
    uint8_t v = m_payload.type0.avg_sound_level_msb;
    v<<=4;
    v += m_payload.type0.avg_sound_level_lsb;
    return v;

}

float uAirUplinkMessageType0::maximumInternalTemperature() const
{
    return (m_payload.type0.max_int_temp - 47) / 4.0;
}

unsigned uAirUplinkMessageType0::maximumInternalHumidity() const
{
    return m_payload.type0.max_int_hum;
}
