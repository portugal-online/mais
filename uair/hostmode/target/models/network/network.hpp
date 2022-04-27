#ifndef NETWORK_NETWORK_H__
#define NETWORK_NETWORK_H__

#include "models/network/payload.hpp"
#include <functional>

struct NetworkInterface;

namespace Network
{
    bool setinterface(NetworkInterface*);
    void unsetinterface(void);
    bool devicejoined();
    void unjoin();

    /* Private */
    void Uplink(UplinkPayload *);
    DownlinkPayload *Downlink(const uint32_t timeout_ms, const float speedup);
    void send_user_downlink(uint8_t fport, const uint8_t *data, size_t len);

    char *sprint_buffer(char *dest, const uint8_t *buffer, size_t size);
};



#endif

