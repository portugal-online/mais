#ifndef NETWORK_CRYPTO_H__
#define NETWORK_CRYPTO_H__

#include <inttypes.h>

namespace Network
{
    void aes_decrypt(uint8_t *buffer, uint16_t size,
                     const uint8_t *key,
                     uint8_t *decBuffer);

    void aes_encrypt(uint8_t *buffer, uint16_t size,
                     const uint8_t *key,
                     uint8_t *decBuffer);

    void derive_session_key_10x(uint8_t baseval, const uint8_t *key,
                                const uint8_t* joinNonce,
                                const uint8_t* netID,
                                const uint8_t* devNonce,
                                uint8_t *out);

    int compute_cmac(uint8_t *micBxBuffer,
                     uint8_t *buffer,
                     uint16_t size,
                     uint8_t *key,
                     uint32_t *cmac);

    void payload_decrypt( uint8_t* buffer, int16_t size,
                         const uint8_t *key,
                         uint32_t address,
                         uint8_t dir,
                         uint32_t frameCounter );

    void payload_encrypt( uint8_t* buffer, int16_t size,
                         const uint8_t *key,
                         uint32_t address,
                         uint8_t dir,
                         uint32_t frameCounter );

};

#endif
