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
 * @file    zmod4510_config_oaq2.h
 * @brief   This is the configuration for ZMOD4510 module - oaq_2nd_gen library
 * @version 3.0.0
 * @author Renesas Electronics Corporation
 */

#ifndef ZMOD4510_CONFIG_OAQ_2ND_GEN_H__
#define ZMOD4510_CONFIG_OAQ_2ND_GEN_H__

#include <stdio.h>
#include "zmod4xxx_types.h"

#define Z_INIT    0
#define Z_MEASURE 1

/**********************************/
/* < Define product ID > */
#define ZMOD4510_PID 0x6320

/**********************************/
/* < Define I2C slase address > */
#define ZMOD4510_I2C_ADDR 0x33

/**********************************/
/* < Define product data length > */
#define ZMOD4510_PROD_DATA_LEN 10

/*************************************/
/* < Define ADC result data length > */
#define ZMOD4510_ADC_DATA_LEN 18

/* < Define limit of counter > */
#define ZMOD4510_OAQ2_COUNTER_LIMIT 10U

#define ZMOD4XXX_H_ADDR 0x40
#define ZMOD4XXX_D_ADDR 0x50
#define ZMOD4XXX_M_ADDR 0x60
#define ZMOD4XXX_S_ADDR 0x68

extern uint8_t data_set_4510_init[];
extern uint8_t data_set_4510_oaq_2nd_gen[];
extern zmod4xxx_conf zmod_oaq_sensor_type[];

#endif //_ZMOD4510_CONFIG_OAQ_2ND_GEN_H_
