/*******************************************************************************
 * Copyright (c) 2020 Renesas Electronics Corporation
 * All Rights Reserved.
 *
 * This code is proprietary to Renesas, and is license pursuant to the terms and
 * conditions that may be accessed at:
 * https://www.renesas.com/eu/en/document/msc/idt-software-license-terms-gas-sensor-software
 *
 ******************************************************************************/

/**
 * @file    zmod4510_config_oaq1.h
 * @brief   This is the configuration for zmod4510 module - oaq 1st gen library
 * @version 3.0.0
 * @author Renesas Electronics Corporation
 */

#ifndef _ZMOD4510_CONFIG_OAQ1_H_
#define _ZMOD4510_CONFIG_OAQ1_H_

#include <stdio.h>
#include "zmod4xxx_types.h"

#define Z_INIT        0
#define Z_MEASURE     1

/* < Define product ID > */
#define ZMOD4510_PID (0x6320)

/* < Define I2C slase address > */
#define ZMOD4510_I2C_ADDR (0x33)

/* < Define product data length > */
#define ZMOD4510_PROD_DATA_LEN 9

/* < Define ADC result data length > */
#define ZMOD4510_ADC_DATA_LEN 30

/* < Define limit of counter > */
#define ZMOD4510_OAQ1_COUNTER_LIMIT 600U

#define ZMOD4510_H_ADDR (0x40)
#define ZMOD4510_D_ADDR (0x50)
#define ZMOD4510_M_ADDR (0x60)
#define ZMOD4510_S_ADDR (0x68)

/* Zmod4510 OAQ 1st Gen Parameters */
#define D_RISING_M1           4.9601944386079566e-05
#define D_FALLING_M1          0.3934693402873666
#define D_CLASS_M1            0.024690087971667385
#define STABILIZATION_SAMPLES 60

extern uint8_t dataset_4510_init[];
extern uint8_t dataset_4510_oaq_1st_gen[];
extern zmod4xxx_conf zmod_oaq_sensor_type[];


#endif //_ZMOD4510_CONFIG_OAQ1_H_
