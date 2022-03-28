#include "stm32wlxx_hal.h"
#include <stdlib.h>


#define WIDTH    ((8U * sizeof(uint32_t)))
#define TOPBIT   (((uint32_t)1U << (WIDTH - 1U)))

static void CRC32_Init( CRC_TypeDef *crc )
{
    uint32_t dividend = 0U;

    printf("CRC INIT: poly %08x\n", crc->poly);
    for( dividend = 0U; dividend < 256U; ++ dividend )
    {
        uint32_t remainder = 0U;
        uint8_t bit = 0U;
        remainder = dividend << ( WIDTH - 8U );
        for( bit = 8U; bit > 0U; -- bit )
        {
            if( ( remainder & TOPBIT ) != 0U )
            {
                remainder = ( remainder << 1U ) ^ crc->poly;
            }
            else
            {
                remainder = ( remainder << 1U );
            }
        }

        crc->crcTable[dividend] = remainder;
    }
}

HAL_StatusTypeDef HAL_CRC_Init(CRC_HandleTypeDef *hcrc)
{

    if (hcrc->Init.DefaultPolynomialUse == DEFAULT_POLYNOMIAL_DISABLE)
    {
        hcrc->Instance->poly = hcrc->Init.GeneratingPolynomial;
    }
    else
    {
        hcrc->Instance->poly = DEFAULT_CRC32_POLY;
    }
    hcrc->Instance->initial = DEFAULT_CRC_INITVALUE;
    hcrc->Instance->finalxor = 0xFFFFFFFFU;

    // Init CRC table
    CRC32_Init(hcrc->Instance);
    return HAL_OK;
}



HAL_StatusTypeDef HAL_CRC_DeInit(CRC_HandleTypeDef *hcrc)
{
    return HAL_OK;
}

uint32_t HAL_CRC_Calculate(CRC_HandleTypeDef *hcrc, uint32_t pBuffer[], uint32_t BufferLength)
{
    uint32_t remainder = hcrc->Instance->initial;
    uint32_t byte = 0U;
    const uint8_t *message = (uint8_t*)pBuffer;

    for( byte = 0U; byte < BufferLength<<2; ++ byte )
    {
        uint8_t data = message[byte] ^ (uint8_t)( remainder >> ( WIDTH - 8U ) );

        remainder = hcrc->Instance->crcTable[data] ^ ( remainder << 8U );
    }

    remainder ^= hcrc->Instance->finalxor;

    return remainder;
}
