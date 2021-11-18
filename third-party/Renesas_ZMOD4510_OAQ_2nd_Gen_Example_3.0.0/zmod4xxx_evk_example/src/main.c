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
 * @file    main.c
 * @brief   This is an example for the ZMOD4510 gas sensor using the OAQ 2nd Gen
 * library.
 * @version 3.0.0
 * @author Renesas Electronics Corporation
 */

#include "zmod4510_config_oaq2.h"
#include "zmod4xxx.h"
#include "zmod4xxx_cleaning.h"
#include "zmod4xxx_hal.h"
#include "oaq_2nd_gen.h"

int main()
{
    /* The ambient compensation needs humidity and temperature measurements! */
    float humidity_pct;
    float temperature_degc;

    int8_t ret;
    zmod4xxx_dev_t dev;

    uint32_t polling_counter = 0;

    /* Sensor target variables */
    uint8_t track_number[6];
    uint8_t zmod4xxx_status;
    uint8_t adc_result[ZMOD4510_ADC_DATA_LEN];
    uint8_t prod_data[ZMOD4510_PROD_DATA_LEN];
    oaq_2nd_gen_handle_t algo_handle;
    oaq_2nd_gen_results_t algo_results;

    /****TARGET SPECIFIC FUNCTION ****/
    /*
     * Users have to write their own init_hardware function in
     * init_hardware (located in dependencies/zmod4xxx_api/HAL directory).
     * Variable of dev has a *read, *write and *delay function pointer. 
     * For those function pointers three functions have to be generated 
     * for corresponding hardware and assigned in init_hardware. 
     * For more information, check the Datasheet section 
     * “I2C Interface and Data Transmission Protocol”.
     */
    ret = init_hardware(&dev);
    if (ret) {
        printf("Error %d during initialize hardware, exiting program!\n", ret);
        goto exit;
    }
    /****TARGET SPECIFIC FUNCTION ****/

    /* Sensor Related Data */
    dev.i2c_addr = ZMOD4510_I2C_ADDR;
    dev.pid = ZMOD4510_PID;
    dev.init_conf = &zmod_oaq_sensor_type[INIT];
    dev.meas_conf = &zmod_oaq_sensor_type[MEASURE];
    dev.prod_data = prod_data;

    /* Read product ID and configuration parameters. */
    ret = zmod4xxx_read_sensor_info(&dev);
    if (ret) {
        printf("Error %d during reading sensor information, exiting program!\n",
               ret);
        goto exit;
    }

    /* Each sensor has a unique tracking number and individual trimming
     * information. Please provide these information when you need support by
     * Renesas. */
    ret = zmod4xxx_read_tracking_number(&dev, track_number);
    if (ret) {
        printf("Error %d during reading tracking number, exiting program!\n",
               ret);
        goto exit;
    }
    printf("sensor tracking number: x0000");
    for (int i = 0; i < sizeof(track_number); i++) {
        printf("%02X", track_number[i]);
    }
    printf("\n");
    printf("sensor trimming data:");
    for (int i = 0; i < sizeof(prod_data); i++) {
        printf(" %i", prod_data[i]);
    }
    printf("\n");

    /* This function starts the cleaning procedure. Please
    * check the Programming Manual on indications for usage.
    * IMPORTANT NOTE: The cleaning procedure can be run only once
    * during the modules lifetime and takes 10 minutes (blocking). */
    //ret = zmod4xxx_cleaning_run(&dev);
    //if (ret) {
    //    printf("Error %d during cleaning procedure, exiting program!\n", ret);
    //    goto exit;
    //}

    /* Calibration parameters are determined and measurement is configured. */
    ret = zmod4xxx_prepare_sensor(&dev);
    if (ret) {
        printf("Error %d during preparation of the sensor, exiting program!\n",
               ret);
        goto exit;
    }

    /* One time initialization of the algorithm. Pass handle to calculation function. */
    ret = init_oaq_2nd_gen(&algo_handle, &dev);
    if (ret) {
        printf("Error %d when initializing algorithm, exiting program!\n", ret);
        goto exit;
    }

    /* Start one measurement. */
    ret = zmod4xxx_start_measurement(&dev);
    if (ret) {
        printf("Error %d during start of measurement, exiting program!\n", ret);
        goto exit;
    }

    /* Main Measurement Loop */
    do {
        /* Instead of polling STATUS REGISTER, interrupts can be used.
         * For more information, please check Programming Manual, section
         * “Interrupt Usage” */
        do {
            ret = zmod4xxx_read_status(&dev, &zmod4xxx_status);
            if (ret) {
                printf(
                    "Error %d during read of sensor status, exiting program!\n",
                    ret);
                goto exit;
            }
            /* Increase polling counter */
            polling_counter++;
            /* Delay for 15 milliseconds */
            dev.delay_ms(15);
        } while ((zmod4xxx_status & STATUS_SEQUENCER_RUNNING_MASK) &&
                 (polling_counter <= ZMOD4510_OAQ2_COUNTER_LIMIT));

        /* Check if timeout is occured */
        if (ZMOD4510_OAQ2_COUNTER_LIMIT <= polling_counter) {
            ret = zmod4xxx_check_error_event(&dev);
            if (ret) {
                printf("Error %d during check event, exiting program!\n", ret);
                goto exit;
            }
            printf("Error %d, exiting program!\n", ERROR_GAS_TIMEOUT);
            goto exit;
        } else {
            polling_counter = 0;
        }

        /* Read sensor ADC output. */
        ret = zmod4xxx_read_adc_result(&dev, adc_result);
        if (ret) {
            printf("Error %d during read of ADC results, exiting program!\n",
                   ret);
            goto exit;
        }

        /* Humidity and temperature measurements are needed for ambient
         * compensation. It is highly recommented to have a real humidity and
         * temperature sensor for these values! */
        humidity_pct = 50.0; // 50% RH
        temperature_degc = 20.0; // 20 degC

        /* calculate algorithm results */
        ret = calc_oaq_2nd_gen(&algo_handle, &dev, adc_result, humidity_pct,
                               temperature_degc, &algo_results);
        if ((ret != OAQ_2ND_GEN_OK) && (ret != OAQ_2ND_GEN_STABILIZATION)) {
            printf("Error %d when calculating algorithm, exiting program!\n",
                   ret);
            goto exit;
            /* OAQ 2nd Gen algorithm skips first samples for sensor stabilization */
        } else {
            printf("*********** Measurements ***********\n");
            for (int i = 0; i < 8; i++) {
                printf(" Rmox[%d] = ", i);
                printf("%.3f kOhm\n", algo_results.rmox[i] / 1e3);
            }
            printf(" O3_conc_ppb = %6.3f\n", algo_results.O3_conc_ppb);
            printf(" Fast AQI = %i\n", algo_results.FAST_AQI);
            printf(" EPA AQI = %i\n", algo_results.EPA_AQI);
            if (ret == OAQ_2ND_GEN_STABILIZATION) {
                printf("Warmup!\n");
            } else {
                printf("Valid!\n");
            }
            printf("************************************\n");
        }

        /* wait 1.98 seconds before starting the next measurement */
        dev.delay_ms(1980);

        /* start a new measurement before result calculation */
        ret = zmod4xxx_start_measurement(&dev);
        if (ret) {
            printf("Error %d when starting measurement, exiting program!\n",
                   ret);
            goto exit;
        }

    } while (!is_key_pressed());

exit:
    ret = deinit_hardware();
    if (ret) {
        printf("Error %d during deinitialize hardware, exiting program\n", ret);
        return ret;
    }
    return 0;
}
