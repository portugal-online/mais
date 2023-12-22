#ifndef NETWORK_LORAWAN_H__
#define NETWORK_LORAWAN_H__

#include <vector>
#include <inttypes.h>
#include <stdlib.h>
#include <ostream>

typedef enum {
    JOIN_REQUEST,
    JOIN_ACCEPT,
    UNCONFIRMED_UPLINK,
    UNCONFIRMED_DOWNLINK,
    CONFIRMED_UPLINK,
    CONFIRMED_DOWNLINK,
    RFU,
    PROPRIETARY
} lora_message_type_t;



class LoRaUplinkMessage
{
public:
    LoRaUplinkMessage(const std::vector<uint8_t> &data);
    LoRaUplinkMessage(const uint8_t *data, size_t len);

public:
    lora_message_type_t type() const;
    const char *typeString() const;
    std::vector<uint8_t> data() const;
private:
    std::vector<uint8_t> m_data;
};

std::ostream &operator<<(std::ostream &s, const LoRaUplinkMessage &m);

/* Public */
struct NetworkInterface
{
    virtual bool handleUserUplinkMessage(const LoRaUplinkMessage &) = 0;
    /* Handle join. Return true if join accepted, false otherwise */
    virtual bool handleJoin() { return true; }
};

/* Public interface */

namespace LoRaWAN
{
    bool setNetworkInterface(NetworkInterface*);
    void unsetNetworkInterface();

    void sendDownlinkMessage(uint16_t fport, const uint8_t *data, size_t len);
    void sendDownlinkMessage(uint16_t fport, const std::vector<uint8_t> &message);

    bool hasDeviceJoined(void);
    void unjoinDevice(void);
};

#endif
