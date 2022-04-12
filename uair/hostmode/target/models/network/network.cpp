#include "models/network/network.hpp"
#include "models/network/crypto.hpp"
#include "hlog.h"
#include "cqueue.hpp"
#include "lorawan.hpp"

DECLARE_LOG_TAG(LORA_NETWORK)
#define TAG "LORA_NETWORK"


namespace Network
{
    static const uint8_t appkey[] = { 0x9B, 0x45, 0x27, 0xBA, 0x42, 0x28, 0xF4, 0x3C, 0xB9, 0x30, 0x0F, 0xCF, 0xD5, 0xDE, 0x5C, 0xA6 };
    static uint32_t netid = 0xfade3e;
    static uint32_t devaddr = 0xe3ab7c23;
    static uint8_t mcroot[16];
    static uint8_t mcke[16];
    static uint8_t nwk_s[16];
    static uint8_t app_s[16];
    bool joined = false;

    static CQueue<DownlinkPayload*> downlink_queue;
    static NetworkInterface *interface = nullptr;

    bool setinterface(NetworkInterface*i)
    {
        if (interface != nullptr)
        {
            return false;
        }
        interface = i;
        return true;
    }

    void unsetinterface(void)
    {
        interface = nullptr;
    }

    bool devicejoined()
    {
        return joined;
    }

    void unjoin()
    {
        joined = false;
    }

    static int16_t get_rssi()
    {
        return -100;
    }

    static int8_t get_snr()
    {
        return  30;
    }


    char *sprint_buffer(char *dest, const uint8_t *buffer, size_t size)
    {
        char *ptr = dest;

        for (size_t i=0;i<size;i++) {
            if (i!=0)
                *ptr++=' ';

            ptr += sprintf(ptr, "%02X",buffer[i]);
        }
        return ptr;
    }


    uint32_t get_net_id()
    {
        return netid;
    }

    uint32_t get_dev_addr()
    {
        return devaddr;
    }

    uint8_t get_dl_setting()
    {
        return 0;
    }

    static void prepare_accept_request(const uint8_t *dev_nonce)
    {
        uint8_t acceptreq[1+12+4];
        uint8_t encrypted[1+12+4];
        uint8_t decrypted[1+12+4];

        uint32_t lmic;

        uint32_t nonce = random();
        uint32_t netid = get_net_id();
        uint32_t devaddr = get_dev_addr();
        uint8_t dlsettings = get_dl_setting();
        uint8_t rxdelay = 0;

        int i = 0;
        acceptreq[i++] = (0x01)<<5; // Join-Accept
        acceptreq[i++] = nonce;
        acceptreq[i++] = nonce >> 8;
        acceptreq[i++] = nonce >> 24;
        acceptreq[i++] = netid;
        acceptreq[i++] = netid >> 8;
        acceptreq[i++] = netid >> 16;
        acceptreq[i++] = devaddr;
        acceptreq[i++] = devaddr >> 8;
        acceptreq[i++] = devaddr >> 16;
        acceptreq[i++] = devaddr >> 24;
        acceptreq[i++] = dlsettings;
        acceptreq[i++] = rxdelay;

        // Encrypt


        compute_cmac(NULL,
                     (uint8_t*)&acceptreq[0],
                     i,
                     (uint8_t*)appkey,
                     &lmic);

        acceptreq[i++] = lmic;
        acceptreq[i++] = lmic>>8;
        acceptreq[i++] = lmic>>16;
        acceptreq[i++] = lmic>>24;

        aes_decrypt(&acceptreq[1], 16, appkey, &encrypted[1]);
        encrypted[0] = acceptreq[0];

        aes_encrypt(&encrypted[1], 16, appkey, &decrypted[1]);
        decrypted[0] = acceptreq[0];


        HWARN(TAG,"Using MIC=%08x", lmic);
        char temp[128];
        sprint_buffer(temp, acceptreq, i);
        HWARN(TAG, "Original : [%s]", temp);
        sprint_buffer(temp, encrypted, i);
        HWARN(TAG, "Encrypt  : [%s]", temp);
        sprint_buffer(temp, decrypted, i);
        HWARN(TAG, "Decrypt  : [%s]", temp);
        sprint_buffer(temp, appkey, 16);
        HWARN(TAG, "Key      : [%s]", temp);


        DownlinkPayload *payload = new DownlinkPayload(encrypted, i, get_rssi(), get_snr());

        downlink_queue.enqueue(payload);

        uint8_t inputdata[16] = { 0 };

        aes_encrypt(inputdata, 16, appkey, mcroot);

        //char temp[128];

        sprint_buffer(temp, mcroot, 16);
        HWARN(TAG, "Storing MC ROOT key as [%s]", temp);
        aes_encrypt(inputdata, 16, mcroot, mcke);

        sprint_buffer(temp, mcke, 16);
        HWARN(TAG, "Storing MC KE key as [%s]", temp);

        derive_session_key_10x(0x02, appkey, &acceptreq[1], &acceptreq[4], dev_nonce, app_s);
        sprint_buffer(temp, app_s, 16);
        HWARN(TAG, "Storing APP_S key as [%s]", temp);

        derive_session_key_10x(0x01, appkey,  &acceptreq[1], &acceptreq[4], dev_nonce, nwk_s);
        sprint_buffer(temp, nwk_s, 16);
        HWARN(TAG, "Storing NWK_S key as [%s]", temp);

        joined = true;
    }


    static void process_join_request(const uint8_t *data, unsigned datalen)
    {
        uint32_t lmic;
        uint32_t mic;

        if (datalen!=23)
            return;
#if 0
        const uint8_t *appeui = &data[1];
        const uint8_t *deveui = &data[1+8];
#endif

        const uint8_t *nonce = &data[1+8+8];
        const uint8_t *micptr = &data[1+8+8+2];

        compute_cmac(NULL,
                     (uint8_t*)&data[0],
                     datalen - 4 ,
                     (uint8_t*)appkey,
                     &lmic);

        mic = micptr[0] | ((uint32_t)micptr[1]<<8) | ((uint32_t)micptr[2]<<16) | (uint32_t)micptr[3]<<24;
        if (mic != lmic)
        {
            HERROR(TAG,"Invalid message MIC, dropping");
            return;
        }

        HWARN(TAG, "Join request from device");
        if (nullptr==interface || interface->handleJoin())
        {
            prepare_accept_request(nonce);
        }
    }



    static uint32_t downlink_counter = 0;

    uint32_t get_downlink_frame_counter()
    {
        return downlink_counter++;
    }


    void send_user_downlink(uint8_t fport, const uint8_t *data, size_t len)
    {
        uint8_t packet[128];

        if (!joined) {
            HERROR(TAG,"Cannot queue downlink if device has not joined!");
            return;
        }

        packet[0] = (0x5)<<5;
        packet[1] = devaddr;
        packet[2] = devaddr>>8;
        packet[3] = devaddr>>16;
        packet[4] = devaddr>>24;
        packet[5] = 0x80; // Fcntrl

        uint32_t counter = get_downlink_frame_counter();

        packet[6] = counter;
        packet[7] = counter>>8;
        packet[8] = fport;

        memcpy(&packet[9], data, len);

        // Encrypt payload
        payload_decrypt(&packet[9], len, app_s, devaddr, 0x01, counter);

        char temp[256];
        sprint_buffer(temp, app_s, 16);

        HWARN(TAG,"Encrypted with key [%s]",temp);

        int16_t datalen = 9 + len + 4;

        // Compute MIC
        uint8_t bblk[16] = {0};

        bblk[0] = 0x49;
        bblk[5] = 0x01; // Downlink
        bblk[6] = packet[1]; // devaddr[0]
        bblk[7] = packet[2]; // devaddr[1]
        bblk[8] = packet[3]; // devaddr[2]
        bblk[9] = packet[4]; // devaddr[3]

        bblk[10] = packet[6]; // Fcnt[0]
        bblk[11] = packet[7]; // Fcnt[1]
        bblk[12] = 0; // Fcnt
        bblk[13] = 0; // Fcnt

        bblk[15] = datalen -4;



        sprint_buffer(temp, bblk, 16);

        HWARN(TAG,"Using bblk [%s]",temp);
        uint32_t lmic;

        compute_cmac(bblk,
                     (uint8_t*)&packet[0],
                     datalen - 4 ,
                     (uint8_t*)nwk_s,
                     &lmic);

        uint8_t *mic = &packet[datalen-4];
        mic[0] = lmic;
        mic[1] = lmic>>8;
        mic[2] = lmic>>16;
        mic[3] = lmic>>24;


        DownlinkPayload *payload = new DownlinkPayload(packet, datalen, get_rssi(), get_snr());

        downlink_queue.enqueue(payload);

    }

    static void process_unconfirmed_uplink(const uint8_t *data, unsigned datalen)
    {
        uint32_t lmic;
        uint32_t mic;
        uint8_t bblk[16] = {0};
        uint8_t decrypted[128];
        const uint8_t *addrptr = &data[1];

        /*
         Join response:
         [20
         B0 03 29
         3E DE FA
         23 7C AB E3
         00 00 69 5E 04 43]
         */
        // parse frame header
        /*
         [40
         23 7C AB E3 // DEVADDR
         80  // Fctrl
         01 00  // Fcnt
         02 // port
         98 B2 93 CA 8A C4 BD 1F
         AF E5 B3 1A]
         */
        bblk[0] = 0x49;
        bblk[6] = data[1]; // devaddr[0]
        bblk[7] = data[2]; // devaddr[1]
        bblk[8] = data[3]; // devaddr[2]
        bblk[9] = data[4]; // devaddr[3]

        bblk[10] = data[6]; // Fcnt[0]
        bblk[11] = data[7]; // Fcnt[1]
        bblk[12] = 0; // Fcnt
        bblk[13] = 0; // Fcnt

        bblk[15] = datalen-4;//



        char temp[256];
        sprint_buffer(temp, bblk, 16);

        HWARN(TAG,"Using bblk [%s]",temp);


        const uint8_t *micptr = &data[datalen-4];

        compute_cmac(bblk,
                     (uint8_t*)&data[0],
                     datalen - 4 ,
                     (uint8_t*)nwk_s,
                     &lmic);

        mic = micptr[0] | ((uint32_t)micptr[1]<<8) | ((uint32_t)micptr[2]<<16) | (uint32_t)micptr[3]<<24;

        if (mic != lmic)
        {
            HERROR(TAG,"Invalid message MIC, dropping");
            return;
        }


        // Decrypt message
        uint32_t framecounter = ((uint32_t)data[7]<<8) | data[6];
        int16_t size = datalen - 8 - 5;

        uint32_t addr = addrptr[0] | ((uint32_t)addrptr[1]<<8) | ((uint32_t)addrptr[2]<<16) | (uint32_t)addrptr[3]<<24;


        sprint_buffer(temp, app_s, 16);
        HWARN(TAG,"Using address=0x%08x framecounter=0x%08x key=[%s]", addr, framecounter, temp );


        memcpy( decrypted, data, size+9);

        sprint_buffer(temp, &decrypted[9], size);

        HWARN(TAG," Data: [%s]", temp);



        payload_decrypt( &decrypted[9],
                        size,
                        app_s,
                        addr,
                        0,
                        framecounter);

        sprint_buffer(temp, &decrypted[9], size);

        HWARN(TAG,"Decrypted: [%s]",temp);


        if (nullptr!=interface)
        {
            // Copy back the decrypted payload
            LoRaUplinkMessage s(decrypted, size+9);

            HWARN(TAG,"Passing message to lower layer");
            interface->handleUserUplinkMessage(s);
        } else {
            HWARN(TAG,"No network layer installed, ignoring message");

        }

    }

    static void process_radio_data(const uint8_t *data, size_t datalen)
    {
        uint8_t mhdr;

        if (datalen<5)
            return;

        mhdr = data[0];

        switch (mhdr>>5) {
        case 0: // Join
            HLOG(TAG, "Processing join request");
            process_join_request(data, datalen);
            break;
        case 2: // Unconfirmed uplink
            HLOG(TAG,"Unconfirmed uplink");
            process_unconfirmed_uplink(data, datalen);
            break;
        case 4: // Confirmed uplink
            HLOG(TAG,"Confirmed uplink");
            break;
        default:
            HERROR(TAG, "Cannot handle MHDR=%02x\n", mhdr);
            abort();
        }
    }

    void Uplink(UplinkPayload *payload)
    {
        process_radio_data( payload->data().data(), payload->size() );
    }

    DownlinkPayload *Downlink(const uint32_t timeout_ms, const float speedup)
    {
        DownlinkPayload *p = nullptr;
        uint32_t realtimeout =  timeout_ms/speedup;

        if (downlink_queue.timed_dequeue(realtimeout, p))
            return p;

        HWARN(TAG ,"No downlink available");
        return nullptr;
    }


};
