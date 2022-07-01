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
 * @file    hicom.c
 * @brief   Hardware abstraction layer for windows
 * @version 2.5.1
 * @author Renesas Electronics Corporation
 ********************************************************************/

#include "hal_hicom.h"

hicom_handle_t hicom_handle;
hicom_status_t hicom_status;

int8_t init_hardware(zmod4xxx_dev_t *dev)
{
    int8_t ret;

    hicom_status = hicom_open(&hicom_handle);
    if (FTC_SUCCESS != hicom_status) {
        return hicom_status;
    }

    hicom_status = hicom_power_on(hicom_handle);
    if (FTC_SUCCESS != hicom_status) {
        return hicom_status;
    }

    set_hicom_handle(&hicom_handle);
    dev->read = hicom_i2c_read;
    dev->write = hicom_i2c_write;
    dev->delay_ms = hicom_sleep;

    return ZMOD4XXX_OK;
}

int8_t is_key_pressed()
{
    int8_t ch;

    if (kbhit()) {
        return 1;
    }
    return 0;
}

int8_t deinit_hardware()
{
    int8_t ret;
    hicom_status = hicom_power_off(hicom_handle);
    if (FTC_SUCCESS != hicom_status) {
        return hicom_status;
    }

    hicom_status = hicom_close(hicom_handle);
    if (FTC_SUCCESS != hicom_status) {
        return hicom_status;
    }
    return ZMOD4XXX_OK;
}