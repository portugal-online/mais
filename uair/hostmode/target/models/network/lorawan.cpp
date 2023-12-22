#include "models/network/lorawan.hpp"
#include "models/network/network.hpp"

const char *lora_names[] = {
    "JOIN_REQUEST",
    "JOIN_ACCEPT",
    "UNCONFIRMED_UPLINK",
    "UNCONFIRMED_DOWNLINK",
    "CONFIRMED_UPLINK",
    "CONFIRMED_DOWNLINK",
    "RFU",
    "PROPRIETARY"
};

namespace LoRaWAN
{
    bool setNetworkInterface(NetworkInterface*interface)
    {
        return Network::setinterface(interface);
    }

    void unsetNetworkInterface()
    {
        Network::unsetinterface();
    }

    void sendDownlinkMessage(uint16_t fport, const uint8_t *data, size_t len)
    {
        Network::send_user_downlink(fport, data, len);
    }

    void sendDownlinkMessage(uint16_t fport, const std::vector<uint8_t> &message)
    {
        Network::send_user_downlink(fport, message.data(), message.size());
    }

    bool hasDeviceJoined(void)
    {
        return Network::devicejoined();
    }

    void unjoinDevice(void)
    {
        Network::unjoin();
    }

}

LoRaUplinkMessage::LoRaUplinkMessage(const std::vector<uint8_t> &data): m_data(data)
{
}

LoRaUplinkMessage::LoRaUplinkMessage(const uint8_t *data, size_t len): m_data(data, data+len)
{
}
lora_message_type_t LoRaUplinkMessage::type() const
{
    return (lora_message_type_t)(m_data[0]>>5);
}
const char *LoRaUplinkMessage::typeString() const
{
    return lora_names[ (m_data[0]>>5) ];
}

std::vector<uint8_t> LoRaUplinkMessage::data() const
{
    return std::vector<uint8_t>(m_data.begin()+9, m_data.end());
}

std::ostream &operator<<(std::ostream &s, const LoRaUplinkMessage &m)
{
    char tmp[4];
    s<<"{ type="<< m.typeString()<<", payload=[";
    for (auto i: m.data())
    {
        sprintf(tmp," %02x", i);
        s<<tmp;
    }
    s<<" ] }";
    return s;
}
