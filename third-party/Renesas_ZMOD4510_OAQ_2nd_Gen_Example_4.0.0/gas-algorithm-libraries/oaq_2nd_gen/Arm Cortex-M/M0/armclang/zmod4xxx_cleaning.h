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
 * @file    zmod4xxx_cleaning.h
 * @brief   This file contains the cleaning function definition for ZMOD4xxx.
 * @version 4.0.0
 * @author Renesas Electronics Corporation
 * @details The library contains the function that starts the cleaning procedure.
 *          **The procedure takes 10 minutes.** After successful cleaning,
 *          the function returns 0. **The procedure can be run only once.**
 */

#ifndef _ZMOD4XXX_CLEANING_H_
#define _ZMOD4XXX_CLEANING_H_

#include "zmod4xxx.h"

/**
 * @brief Start a cleaning procedure
 * @param [in] dev pointer to the device
 * @return Error code
 * @retval 0 Success
 * @retval "!= 0" Error
 */
int8_t zmod4xxx_cleaning_run(zmod4xxx_dev_t *dev);

#endif // _ZMOD4XXX_CLEANING_H_
