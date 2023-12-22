#ifndef NETWORK_PAYLOAD_H__
#define NETWORK_PAYLOAD_H__

#include <vector>
#include <inttypes.h>
#include <stdlib.h>
#include <cstdio>

typedef std::vector<uint8_t> payload_data_t;

class Payload
{
public:
    Payload(const payload_data_t &data): m_data(data) {}
    Payload(const uint8_t *data, size_t size): m_data(data, data+size) {}

    payload_data_t::size_type size()  const  { return m_data.size(); }
    void print(char *target, size_t maxsize)
    {
        char *ptr = &target[0];
        for (auto c: m_data) {
            if (ptr!=&target[0])
                *ptr++=' ';
            ptr += sprintf(ptr, "%02X",c);
        }
    };
    const payload_data_t &data() const { return m_data; }
private:
    payload_data_t m_data;
};

class DownlinkPayload: public Payload
{
public:
    DownlinkPayload(const uint8_t *data, size_t size, int16_t rssi, int8_t snr): Payload(data, size), m_rssi(rssi), m_snr(snr) {}
    DownlinkPayload(const payload_data_t &payload, int16_t rssi, int8_t snr): Payload(payload), m_rssi(rssi), m_snr(snr) {}
    DownlinkPayload(const uint8_t *data, size_t size): Payload(data, size), m_rssi(0), m_snr(0) {}
    DownlinkPayload(const payload_data_t &payload, size_t size): Payload(payload), m_rssi(0), m_snr(0) {}

    int16_t rssi() const { return m_rssi; }
    int8_t snr() const { return m_snr; }
private:
    int16_t m_rssi;
    int8_t m_snr;
};


class UplinkPayload: public Payload
{
public:
    UplinkPayload(const uint8_t *data, size_t size, uint32_t packet_airtime): Payload(data,size), m_airtime(packet_airtime) {}
    UplinkPayload(const payload_data_t &payload, uint32_t packet_airtime): Payload(payload), m_airtime(packet_airtime) {}
    uint32_t airtime() const { return m_airtime; }
private:
    uint32_t m_airtime;
};

#endif
