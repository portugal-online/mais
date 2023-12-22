#include "console.h"
#include "debug.h"
#include "ldo.h"
#include "chgdischg.h"
#include "internal_adc.h"
#include "dut.h"
#include "app.h"

extern int external_adc_get_value();

static inline void console_print(const char *fmt, ...)
{
    if (!app_reporting_enabled()) {
        va_list ap;
        va_start(ap, fmt);
        dbgvprintf(fmt, ap);
        va_end(ap);
    }
}

static void console_help(void)
{
    console_print("\r\nConsole commands:\r\n\r\n");
    console_print("\tV/v\tPrint voltages\r\n");
    console_print("\tS/s\tPrint status\r\n");
    console_print("\tL/l\tEnable/disable LDO\r\n");
    console_print("\tC/c\tEnable/disable charge\r\n");
    console_print("\tD/d\tEnable/disable discharge\r\n");
    console_print("\tF/f\tFloat charge\r\n");
    console_print("\tR/r\tEnable/disable reporting" "\r\n");
    console_print("\tU/u\tEnable/disable DUT power" "\r\n");
}

static void console_voltages(void)
{
    console_print("Console voltages: vcc=%dmV! vchg=%dmV vadc=%dmC\r\n",
            internal_adc_get_vref_mv(),
            internal_adc_get_vchg_mv(),
            external_adc_get_value()
           );
}

static const char *console_true_false(bool b)
{
    return b?"TRUE":"FALSE";
}
static void console_status(void)
{
    console_print("\r\n" "Overall status:" "\r\n");
    console_print("\tLDO enabled      : %s\r\n", console_true_false(ldo_enabled())) ;
    console_print("\tCharge enabled   : %s\r\n", console_true_false(chgdischg_charge_enabled())) ;
    console_print("\tDiscarge enabled : %s\r\n", console_true_false(chgdischg_discharge_enabled())) ;
}

void console_char(int ch)
{
    switch (ch) {
    case 'h': /* Fall-through */
    case 'H':
        console_help();
        break;
    case 'v':
    case 'V':
        console_voltages();
        break;
    case 's':
    case 'X':
        console_status();
        break;
    case 'L':
        console_print("\r\n" "Enabling LDO" "\r\n");;
        ldo_enable();
        break;
    case 'l':
        console_print("\r\n" "Disabling LDO" "\r\n");
        ldo_disable();
        break;
    case 'C':
        console_print("\r\n" "Enable charge" "\r\n");
        chgdischg_enable_charge();
        break;
    case 'c':
        console_print("\r\n" "Disabling charge" "\r\n");
        chgdischg_disable_charge();
        break;

    case 'D':
        console_print("\r\n" "Enabling discharge" "\r\n");
        chgdischg_enable_discharge();
        break;
    case 'd':
        console_print("\r\n" "Disabling discharge" "\r\n");
        chgdischg_disable_discharge();
        break;

    case 'U':
        console_print("\r\n" "Enabling DUT" "\r\n");
        dut_enable_power();
        break;
    case 'u':
        console_print("\r\n" "Disabling DUT" "\r\n");
        dut_disable_power();
        break;

    case 'R':
        console_print("\r\n" "Enabling report" "\r\n");
        app_enable_reporting();
        break;
    case 'r':
        console_print("\r\n" "Disabling report" "\r\n");
        app_disable_reporting();
        break;
    case 'F': /* Fall-through */
    case 'f':
        console_print("\r\n" "Disabling charge/discharge" "\r\n");
        chgdischg_disable_discharge();
        chgdischg_disable_charge();
        break;

    };
}
