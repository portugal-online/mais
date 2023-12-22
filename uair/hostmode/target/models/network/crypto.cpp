#include "models/network/crypto.hpp"
#include "hlog.h"
#include "cmac.h" // From Lorawan
#include <stdlib.h>

namespace Network
{
    void aes_decrypt(uint8_t *buffer, uint16_t size,
                     const uint8_t *key,
                     uint8_t *decBuffer)
    {
        /* Check if the size is divisible by 16 */
        if ((size % 16) != 0)
        {
            fprintf(stderr, "INVALID BUFFER SIZE!!!");
            abort();
        }

        lorawan_aes_context aesContext;

        lorawan_aes_set_key(key, 16, &aesContext);

        uint8_t block = 0;

        while (size != 0)
        {
            lorawan_aes_decrypt(&buffer[block], &decBuffer[block], &aesContext);
            block = block + 16;
            size = size - 16;

        }
    }

    void aes_encrypt(uint8_t *buffer, uint16_t size,
                     const uint8_t *key,
                     uint8_t *decBuffer)
    {
        /* Check if the size is divisible by 16 */
        if ((size % 16) != 0)
        {
            fprintf(stderr, "INVALID BUFFER SIZE!!!");
            abort();
        }

        lorawan_aes_context aesContext;

        lorawan_aes_set_key(key, 16, &aesContext);

        uint8_t block = 0;

        while (size != 0)
        {
            lorawan_aes_encrypt(&buffer[block], &decBuffer[block], &aesContext);
            block = block + 16;
            size = size - 16;

        }
    }

    void derive_session_key_10x(uint8_t baseval, const uint8_t *key,
                                const uint8_t* joinNonce,
                                const uint8_t* netID,
                                const uint8_t* devNonce,
                                uint8_t *out)
    {
        uint8_t buf[16] = { 0 };
        buf[0] = baseval;

        memcpy( &buf[1], joinNonce, 3 );
        memcpy( &buf[4], netID, 3 );
        memcpy( &buf[7], devNonce, 2 );

        aes_encrypt(buf, 16, key, out);
    }

    int compute_cmac(uint8_t *micBxBuffer,
                     uint8_t *buffer,
                     uint16_t size,
                     uint8_t *key,
                     uint32_t *cmac)
    {

        uint8_t Cmac[16];
        AES_CMAC_CTX aesCmacCtx[1];

        AES_CMAC_Init(aesCmacCtx);

        AES_CMAC_SetKey(aesCmacCtx, key);

        if (micBxBuffer != NULL)
        {
            AES_CMAC_Update(aesCmacCtx, micBxBuffer, 16);
        }

        AES_CMAC_Update(aesCmacCtx, buffer, size);

        AES_CMAC_Final(Cmac, aesCmacCtx);

        /* Bring into the required format */
        *cmac = (uint32_t)((uint32_t) Cmac[3] << 24 | (uint32_t) Cmac[2] << 16 | (uint32_t) Cmac[1] << 8 |
                           (uint32_t) Cmac[0]);
        return 0;
    }

    void payload_decrypt( uint8_t* buffer, int16_t size,
                         const uint8_t *key,
                         uint32_t address,
                         uint8_t dir,
                         uint32_t frameCounter )
    {

        uint8_t bufferIndex = 0;
        uint16_t ctr = 1;
        uint8_t sBlock[16] = { 0 };
        uint8_t aBlock[16] = { 0 };

        aBlock[0] = 0x01;

        aBlock[5] = dir;

        aBlock[6] = address & 0xFF;
        aBlock[7] = ( address >> 8 ) & 0xFF;
        aBlock[8] = ( address >> 16 ) & 0xFF;
        aBlock[9] = ( address >> 24 ) & 0xFF;

        aBlock[10] = frameCounter & 0xFF;
        aBlock[11] = ( frameCounter >> 8 ) & 0xFF;
        aBlock[12] = ( frameCounter >> 16 ) & 0xFF;
        aBlock[13] = ( frameCounter >> 24 ) & 0xFF;

        while( size > 0 )
        {
            aBlock[15] = ctr & 0xFF;
            ctr++;
            aes_encrypt(aBlock, 16, key, sBlock);

            for( uint8_t i = 0; i < ( ( size > 16 ) ? 16 : size ); i++ )
            {
                buffer[bufferIndex + i] = buffer[bufferIndex + i] ^ sBlock[i];
            }
            size -= 16;
            bufferIndex += 16;
        }
    }

    void payload_encrypt( uint8_t* buffer, int16_t size,
                         const uint8_t *key,
                         uint32_t address,
                         uint8_t dir,
                         uint32_t frameCounter )
    {

        uint8_t bufferIndex = 0;
        uint16_t ctr = 1;
        uint8_t sBlock[16] = { 0 };
        uint8_t aBlock[16] = { 0 };

        aBlock[0] = 0x01;

        aBlock[5] = dir;

        aBlock[6] = address & 0xFF;
        aBlock[7] = ( address >> 8 ) & 0xFF;
        aBlock[8] = ( address >> 16 ) & 0xFF;
        aBlock[9] = ( address >> 24 ) & 0xFF;

        aBlock[10] = frameCounter & 0xFF;
        aBlock[11] = ( frameCounter >> 8 ) & 0xFF;
        aBlock[12] = ( frameCounter >> 16 ) & 0xFF;
        aBlock[13] = ( frameCounter >> 24 ) & 0xFF;

        while( size > 0 )
        {
            aBlock[15] = ctr & 0xFF;
            ctr++;
            aes_decrypt(aBlock, 16, key, sBlock);

            for( uint8_t i = 0; i < ( ( size > 16 ) ? 16 : size ); i++ )
            {
                buffer[bufferIndex + i] = buffer[bufferIndex + i] ^ sBlock[i];
            }
            size -= 16;
            bufferIndex += 16;
        }
    }

}
