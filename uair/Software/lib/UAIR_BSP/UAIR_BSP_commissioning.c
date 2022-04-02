/*
 * Copyright (C) 2021, 2022 MAIS Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

/**
 * @file UAIR_BSP_commissioning.c
 * 
 * @copyright Copyright (C) 2021, 2022 MAIS Project
 *
 * @ingroup UAIR_BSP_COMMISSIONING
 *
 * uAir commissioning
 *
 */


#include "pvt/UAIR_BSP_commissioning_p.h"
#include "UAIR_BSP_flash.h" // For page size
#include "UAIR_BSP.h"
#include "stm32wlxx_hal_crc.h"
#include <stdbool.h>
#include <string.h>

#if defined (UAIR_HOST_MODE)

extern uint8_t commissioning_data[];

#else // UAIR_HOST_MODE

uint8_t COMMISSIONING_STORAGE_SECTION commissioning_data[BSP_FLASH_PAGE_SIZE];

#endif // UAIR_HOST_MODE

#define COMMISSIONING_FLAG_DEVEUI_SET (1<<0)

struct commissioning_info
{
    uint32_t flags;
    uint8_t deveui[8];
    uint32_t crc32;
};

static bool commissioning_valid = false;

static bool UAIR_BSP_commissioning_flag_active(const struct commissioning_info *info, uint32_t flag)
{
    // Flags are set to zero.
    return ( (~(info->flags)) & flag ) != 0;
}

BSP_error_t UAIR_BSP_commissioning_init(void)
{
    CRC_HandleTypeDef hcrc;
    size_t crc_datalen;

    const struct commissioning_info *info =  (const struct commissioning_info*)commissioning_data;

    crc_datalen = sizeof(struct commissioning_info) - sizeof(uint32_t);

    if ((crc_datalen & 3) !=0)
        return BSP_ERROR_WRONG_PARAM;

    hcrc.Instance = CRC;
    hcrc.Init.DefaultPolynomialUse = DEFAULT_POLYNOMIAL_ENABLE;
    hcrc.Init.DefaultInitValueUse = DEFAULT_INIT_VALUE_ENABLE;
    hcrc.Init.InputDataInversionMode = CRC_INPUTDATA_INVERSION_NONE;
    hcrc.Init.OutputDataInversionMode = CRC_OUTPUTDATA_INVERSION_DISABLE;
    hcrc.InputDataFormat = CRC_INPUTDATA_FORMAT_WORDS;

    commissioning_valid = false;

    if (HAL_CRC_Init(&hcrc)!=HAL_OK) {
        BSP_TRACE("Cannot initialize HW CRC!");
        return BSP_ERROR_COMPONENT_FAILURE;
    }

    uint32_t calc_crc = HAL_CRC_Calculate(&hcrc,
                                          (uint32_t*)(&info->flags),
                                          crc_datalen>>2 );
    // Final inversion
    calc_crc = ~calc_crc;

    HAL_CRC_DeInit(&hcrc);

    if (calc_crc == info->crc32)
    {
        commissioning_valid = true;
    }
    else
    {
        BSP_TRACE("Invalid CRC: calculated %08x expected %08x! %08x", calc_crc, info->crc32,
                 info);
        return BSP_ERROR_COMPONENT_FAILURE;
    }

    return BSP_ERROR_NONE;
}

BSP_error_t UAIR_BSP_commissioning_get_device_eui(uint8_t target[8])
{
    BSP_error_t r;

    struct commissioning_info *info =  (struct commissioning_info*)&commissioning_data[0];

    if (commissioning_valid)
    {

        if ( UAIR_BSP_commissioning_flag_active( info, COMMISSIONING_FLAG_DEVEUI_SET ) )
        {
            memcpy(target, info->deveui, 8);

            r = BSP_ERROR_NONE;
        }
        else
        {
            r= BSP_ERROR_NO_INIT;
        }
    }
    else
    {
        r = BSP_ERROR_COMPONENT_FAILURE;
    }

    return r;
}


BSP_error_t BSP_commissioning_get_device_eui(uint8_t *target)
{
    BSP_error_t err = UAIR_BSP_commissioning_get_device_eui(target);

    if (err != BSP_ERROR_NONE)
    {
        unsigned index = 0;

        uint32_t udn = LL_FLASH_GetUDN();
        uint32_t devid = LL_FLASH_GetDeviceID();
        uint32_t manid = LL_FLASH_GetSTCompanyID();

        target[index++] = (manid>>16U) & 0xFFU;
        target[index++] = (manid>>8U) & 0xFFU;
        target[index++] = (manid) & 0xFFU;
        target[index++] = devid & 0xFFU;
        target[index++] = (udn>>24U) & 0xFFU;
        target[index++] = (udn>>16U) & 0xFFU;
        target[index++] = (udn>>8U) & 0xFFU;
        target[index++] = (udn) & 0xFFU;

        err = BSP_ERROR_NONE;
    }

    return err;
}
