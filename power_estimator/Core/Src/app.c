#include "internal_adc.h"
#include "debug.h"
#include "stm32l1xx_hal.h"
#include <stdbool.h>
#include "main.h"
#include "serial.h"
#include "console.h"
#include "ltc2451.h"
#include "cap.h"

typedef enum {
    LD3,
    LD4,
    LED_GREEN=LD3,
    LED_BLUE=LD4
} led_t;

static int external_adc = -1;
static bool report_enabled = false;
static unsigned report_sample = 0;

bool app_reporting_enabled(void)
{
    return report_enabled;
}

void app_enable_reporting()
{
    report_enabled=true;
}

void app_disable_reporting()
{
    report_enabled=false;
}

int external_adc_get_value()
{
    return external_adc;
}

void external_adc_complete_callback(int value)
{
    double old_charge, new_charge;
    if (value<0) {
        dbgprintf("ERR: cannot read ADC" "\r\n");
        external_adc = -1;
    } else {
        // Vref = 4.096V
        // Bits: 16
        external_adc = (value + 0x7) >>4;
        old_charge = cap_get_charge_coulombs();
        cap_update_voltage(external_adc);
        new_charge = cap_get_charge_coulombs();
        if (report_enabled){
            // Use uA, 50Hz
            double current = (new_charge-old_charge)*(1000000.0/0.02);
            int current_ua=current;

            dbgprintf_full("R:%d:%d:%d" "\r\n",
                           report_sample++,
                           external_adc,
                           current_ua);
        }
    }
}

void internal_adc_completed_callback()
{
    ltc2415_read();
}


void app_set_led(led_t led, bool on)
{
    switch(led) {
    case LD3:
        HAL_GPIO_WritePin(LD3_GPIO_Port, LD3_Pin, on?1:0);
        break;
    case LD4:
        HAL_GPIO_WritePin(LD4_GPIO_Port, LD4_Pin, on?1:0);
        break;
    default:
        break;
    }
}

void app_init()
{
    ltc2415_set_callback(external_adc_complete_callback);

    internal_adc_start();
    serial_init();



    app_set_led(LED_GREEN, true);
}

static void app_idle()
{
    if (serial_available()) {
        int r = serial_read();
        console_char(r);
    }
}

void app_delay(unsigned ms)
{
    unsigned expire = HAL_GetTick()+ms;
    do {
        app_idle();
    } while (HAL_GetTick()<expire);
}

void app_run()
{
    while (1) {
        /*
        dbgprintf("Hello world %d vcc=%dmV! vchg=%dmV\r\n",
                internal_adc_get_completed(),
                internal_adc_get_vref_mv(),
                internal_adc_get_vchg_mv()
                );
                */
        app_set_led(LED_BLUE, true);
        app_delay(10);
        app_set_led(LED_BLUE, false);
        app_delay(490);
    }
}
