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
 * @file    main.c
 * @brief   This is an example for the ZMOD4510 gas sensor using the oaq_2nd_gen library.
 * @version 4.0.0
 * @author Renesas Electronics Corporation
 **/

#include "zmod4510_config_oaq2.h"
#include "zmod4xxx.h"
#include "zmod4xxx_cleaning.h"
#include "zmod4xxx_hal.h"
#include "oaq_2nd_gen.h"

int main()
{
    int8_t ret;
    zmod4xxx_dev_t dev;
    

    /* Sensor specific variables */
    uint8_t zmod4xxx_status;
    uint8_t track_number[ZMOD4XXX_LEN_TRACKING];
    uint8_t adc_result[ZMOD4510_ADC_DATA_LEN];
    uint8_t prod_data[ZMOD4510_PROD_DATA_LEN];
    oaq_2nd_gen_handle_t algo_handle;
    oaq_2nd_gen_results_t algo_results;
    oaq_2nd_gen_inputs_t algo_input;

    /****TARGET SPECIFIC FUNCTION ****/
    /*
	* To allow the example running on customer-specific hardware, the init_hardware
	* function must be adapted accordingly. The mandatory funtion pointers *read,
	* *write and *delay require to be passed to "dev" (reference files located
	* in "dependencies/zmod4xxx_api/HAL" directory). For more information, read
	* the Datasheet, section "I2C Interface and Data Transmission Protocol".
    */
    ret = init_hardware(&dev);
    if (ret) {
        printf("Error %d during initialize hardware, exiting program!\n", ret);
        goto exit;
    }
    /****TARGET SPECIFIC FUNCTION ****/

    /* Sensor related data */
    dev.i2c_addr = ZMOD4510_I2C_ADDR;
    dev.pid = ZMOD4510_PID;
    dev.init_conf = &zmod_oaq2_sensor_cfg[INIT];
    dev.meas_conf = &zmod_oaq2_sensor_cfg[MEASUREMENT];
    dev.prod_data = prod_data;

    /* Read product ID and configuration parameters. */
    ret = zmod4xxx_read_sensor_info(&dev);
    if (ret) {
        printf("Error %d during reading sensor information, exiting program!\n",
               ret);
        goto exit;
    }

    /*
	* Retrieve sensors unique tracking number and individual trimming information.
	* Provide this information when requesting support from Renesas.
    */

    ret = zmod4xxx_read_tracking_number(&dev, track_number);
    if (ret) {
        printf("Error %d during reading tracking number, exiting program!\n",
               ret);
        goto exit;
    }
    printf("Sensor tracking number: x0000");
    for (int i = 0; i < sizeof(track_number); i++) {
        printf("%02X", track_number[i]);
    }
    printf("\n");
    printf("Sensor trimming data: ");
    for (int i = 0; i < sizeof(prod_data); i++) {
        printf(" %i", prod_data[i]);
    }
    printf("\n");

	/*
    * Start the cleaning procedure. Check the Programming Manual on indications
	* of usage. IMPORTANT NOTE: The cleaning procedure can be run only once
	* during the modules lifetime and takes 10 minutes (blocking).
    */

    //ret = zmod4xxx_cleaning_run(&dev);
    //if (ret) {
    //    printf("Error %d during cleaning procedure, exiting program!\n", ret);
    //    goto exit;
    //}

    /* Determine calibration parameters and configure measurement. */
    ret = zmod4xxx_prepare_sensor(&dev);
    if (ret) {
        printf("Error %d during preparation of the sensor, exiting program!\n",
               ret);
        goto exit;
    }

    /*
    * One-time initialization of the algorithm. Handle passed to calculation
    * function.
    */
    ret = init_oaq_2nd_gen(&algo_handle);
    if (ret) {
        printf("Error %d during initializing algorithm, exiting program!\n",
                ret);
        goto exit;
    }

    printf("Evaluate measurements in a loop. Press any key to quit.\n\n");
    do {
        /* Start a measurement. */
        ret = zmod4xxx_start_measurement(&dev);
        if (ret) {
            printf("Error %d during starting measurement, exiting program!\n",
                   ret);
            goto exit;
        }
        /* Perform delay. Required to keep proper measurement timing. */
        dev.delay_ms(ZMOD4510_OAQ2_SAMPLE_TIME);

        /* Verify completion of measurement sequence. */
        ret = zmod4xxx_read_status(&dev, &zmod4xxx_status);
        if (ret) {
            printf("Error %d during reading sensor status, exiting program!\n",
                   ret);
            goto exit;
        }
        /* Check if measurement is running. */
        if (zmod4xxx_status & STATUS_SEQUENCER_RUNNING_MASK) {
            /*
            * Check if reset during measurement occured. For more information,
            * read the Programming Manual, section "Error Codes".
            */
            ret = zmod4xxx_check_error_event(&dev);
            switch (ret) {
            case ERROR_POR_EVENT:
                printf(
                    "Measurement completion fault. Unexpected sensor reset.\n");
                break;
            case ZMOD4XXX_OK:
                printf(
                    "Measurement completion fault. Wrong sensor setup.\n");
                break;
            default:
                printf("Error during reading status register (%d)\n", ret);
                break;
            }
            goto exit;
        }
        /* Read sensor ADC output. */
        ret = zmod4xxx_read_adc_result(&dev, adc_result);
        if (ret) {
            printf("Error %d during reading of ADC results, exiting program!\n",
                   ret);
            goto exit;
        }

        /*
        * Check validity of the ADC results. For more information, read the
		* Programming Manual, section "Error Codes".
        */
        ret = zmod4xxx_check_error_event(&dev);
        if (ret) {
            printf("Error during reading status register (%d)\n", ret);
            goto exit;
        }


        algo_input.adc_result = adc_result;
        /*
		* The ambient compensation needs humidity [RH] and temperature [DegC]
        * measurements! Input them here.
		*/
        algo_input.humidity_pct = 50.0;
        algo_input.temperature_degc = 20.0;

        /* Calculate algorithm results */
        ret = calc_oaq_2nd_gen(&algo_handle, &dev, &algo_input, &algo_results);
        /* Skip 900 stabilization samples for oaq_2nd_gen algorithm. */
        if ((ret != OAQ_2ND_GEN_OK) && (ret != OAQ_2ND_GEN_STABILIZATION)) {
            printf("Error %d during calculating algorithm, exiting program!\n",
                   ret);
            goto exit;

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
                printf("Warm-Up!\n");
            } else {
                printf("Valid!\n");
            }
            printf("************************************\n");

        }
        

    } while (!is_key_pressed());

exit:
    ret = deinit_hardware();
    if (ret) {
        printf("Error %d during deinitializing hardware, exiting program!\n",
               ret);
        return ret;
    }
    return 0;
}
