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
 * @file    zmod4510_config_oaq2.c
 * @brief   This is the configuration for ZMOD4510 module - oaq_2nd_gen library
 * @version 3.0.0
 * @author Renesas Electronics Corporation
 */

#include "zmod4510_config_oaq1.h"

uint8_t dataset_4510_init[] = {
                                 0x00, 0x50,
                                 0x00, 0x28,
                                 0xC3, 0xE3,
                                 0x00, 0x00, 0x80, 0x40,
};

uint8_t dataset_4510_oaq_1st_gen[] = {
                                 0xFE, 0x48, 0xFE, 0X16,
                                 0xFD, 0xE4, 0XFD, 0xB2,
                                 0xFD, 0x80,
                                 0x20, 0x05, 0xA0, 0x18,
                                 0xC0, 0x1C,
                                 0x03,
                                 0x00, 0x00, 0x00, 0x08,
                                 0x00, 0x10, 0x00, 0x01,
                                 0x00, 0x09, 0x00, 0x11,
                                 0x00, 0x02, 0x00, 0x0A,
                                 0x00, 0x12, 0x00, 0x03,
                                 0x00, 0x0B, 0x00, 0x13,
                                 0x00, 0x04, 0x00, 0x0C,
                                 0x80, 0x14,
};

zmod4xxx_conf zmod_oaq_sensor_type[] = {
    [Z_INIT] = {
        .start = 0x80,
        .h = { .addr = ZMOD4510_H_ADDR, .len = 2, .data_buf = &dataset_4510_init[0]},
        .d = { .addr = ZMOD4510_D_ADDR, .len = 2, .data_buf = &dataset_4510_init[2]},
        .m = { .addr = ZMOD4510_M_ADDR, .len = 2, .data_buf = &dataset_4510_init[4]},
        .s = { .addr = ZMOD4510_S_ADDR, .len = 4, .data_buf = &dataset_4510_init[6]},
        .r = { .addr = 0x97, .len = 4},
    },
    [Z_MEASURE] ={
        .start = 0x80,
        .h = { .addr = ZMOD4510_H_ADDR, .len = 10, .data_buf = &dataset_4510_oaq_1st_gen[0]},
        .d = { .addr = ZMOD4510_D_ADDR, .len = 6, .data_buf = &dataset_4510_oaq_1st_gen[10]},
        .m = { .addr = ZMOD4510_M_ADDR, .len = 1, .data_buf = &dataset_4510_oaq_1st_gen[16]},
        .s = { .addr = ZMOD4510_S_ADDR, .len = 30, .data_buf = &dataset_4510_oaq_1st_gen[17]},
        .r = { .addr = 0x97, .len = 30},
        .prod_data_len = ZMOD4510_PROD_DATA_LEN,
    },
};
