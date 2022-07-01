/*******************************************************************************
 * Copyright (c) 2022 Renesas Electronics Corporation
 * All Rights Reserved.
 *
 * This code is proprietary to Renesas, and is license pursuant to the terms and
 * conditions that may be accessed at:
 * https://www.renesas.com/eu/en/document/msc/renesas-software-license-terms-gas-sensor-software
 *
 ******************************************************************************/

/**
 * @file    hal_hicom.h
 * @brief   Hardware abstraction layer for windows
 * @version 2.5.1
 * @author Renesas Electronics Corporation
 */

#ifndef _HAL_HICOM_H
#define _HAL_HICOM_H

#include "hicom.h"
#include "hicom_i2c.h"
#include "zmod4xxx_types.h"

#include <conio.h> /** < Windows Target > */

/**
 * @brief   Initialize the target hardware
 * @param   [in] dev pointer to the device
 * @return  error code
 * @retval  0 success
 * @retval  "!= 0" error
 */
int8_t init_hardware(zmod4xxx_dev_t *dev);

/**
 * @brief   Check if any key is pressed
 * @retval  1 pressed
 * @retval  0 not pressed
 */
int8_t is_key_pressed();

/**
 * @brief   deinitialize target hardware
 * @return  error code
 * @retval  0 success
 * @retval  "!= 0" error
 */
int8_t deinit_hardware();

#endif // _HAL_HICOM_H
