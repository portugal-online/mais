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
 * @file    zmod4xxx_hal.h
 * @brief   zmod4xxx hardware abstraction layer (HAL)
 * @version 2.5.1
 * @author Renesas Electronics Corporation
 */

#ifndef _ZMOD4XXX_HAL_H_
#define _ZMOD4XXX_HAL_H_

#ifdef HICOM
#include "hal_hicom.h"
#elif defined RL78
#include "hal_rl78.h"
#elif defined ARDUINO
#include "hal_arduino.h"
#elif defined RASPI
#include "hal_raspi.h"
#endif

#endif // _ZMOD4XXX_HAL_H_
