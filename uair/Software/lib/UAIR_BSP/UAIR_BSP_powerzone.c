#include "BSP.h"
#include "UAIR_BSP_powerzone.h"
#include "UAIR_BSP_i2c.h"
#include "pvt/UAIR_BSP_i2c_p.h"
#include "pvt/UAIR_BSP_powerzone_p.h"
#include "HAL_gpio.h"

static uint8_t powerzone_status;

struct powerzone_config {
    Load_Switch_TypeDef loadswitch;
    void (*init)(BSP_powerzone_t);
    void (*discharge)(BSP_powerzone_t, int enable_discharge);
    BSP_error_t (*get_power)(BSP_powerzone_t,powerstate_t *power);
};

static void UAIR_BSP_powerzone_init_indirect_discharge(BSP_powerzone_t);
static void UAIR_BSP_powerzone_discharge_aqs_r1(BSP_powerzone_t, int enable_discharge);
static void UAIR_BSP_powerzone_discharge_i2c(BSP_powerzone_t, int enable_discharge);
static BSP_error_t UAIR_BSP_powerzone_check_i2c_power(BSP_powerzone_t zone,powerstate_t*);

static const struct powerzone_config powerzones_r1[UAIR_POWERZONE_MAX+1] = {
    {
        .loadswitch = LOAD_SWITCH_INTERNAL,
        .init = NULL,
        .discharge = NULL,
        .get_power = NULL
    },
    {
        .loadswitch = LOAD_SWITCH_MICROPHONE,
        .init = NULL,
        .discharge = NULL,
        .get_power = NULL
    },
    {
        .loadswitch = LOAD_SWITCH_AIRQUALITYSENSOR,
        .init = &UAIR_BSP_powerzone_init_indirect_discharge,
        .discharge  = &UAIR_BSP_powerzone_discharge_aqs_r1,
        .get_power = NULL
    }
};

static const struct powerzone_config powerzones_r2[UAIR_POWERZONE_MAX+1] = {
    {
        .loadswitch = LOAD_SWITCH_INTERNAL,
        .init = NULL,
        .discharge = &UAIR_BSP_powerzone_discharge_i2c,
        .get_power = &UAIR_BSP_powerzone_check_i2c_power
    },
    {
        .loadswitch = LOAD_SWITCH_MICROPHONE,
        .init = NULL,
        .discharge = &UAIR_BSP_powerzone_discharge_i2c,
        .get_power = &UAIR_BSP_powerzone_check_i2c_power
    },
    {
        .loadswitch = LOAD_SWITCH_AIRQUALITYSENSOR,
        .init = NULL,
        .discharge = &UAIR_BSP_powerzone_discharge_i2c,
        .get_power = &UAIR_BSP_powerzone_check_i2c_power
    },
};

// Discharge for r1
static const HAL_GPIODef_t discharge_r1_gpio = {
    .pin = LOAD_SWITCH2_DISCHG_PIN,
    .port = LOAD_SWITCH2_DISCHG_GPIO_PORT,
    .clock_control = LOAD_SWITCH2_DISCHG_CLK_CONTROL
};

static const struct powerzone_config *powerzone_config = NULL;

typedef struct {
    powerzone_notify_callback_t callback;
    void *userdata;
} powerzone_callback_t;

static powerzone_callback_t powerzones_notify_callbacks[UAIR_POWERZONE_MAX+1] = {0};

static void UAIR_BSP_powerzone_init_indirect_discharge(BSP_powerzone_t zone)
{
    /*
    GPIO_InitTypeDef gpio_init_structure = {0};
    gpio_init_structure.Pin = LOAD_SWITCH2_DISCHG_PIN;
    gpio_init_structure.Mode = GPIO_MODE_OUTPUT_PP; // Save power.
    gpio_init_structure.Pull = GPIO_NOPULL;
    gpio_init_structure.Speed = GPIO_SPEED_FREQ_LOW;
    HAL_GPIO_Init(LOAD_SWITCH2_DISCHG_GPIO_PORT, &gpio_init_structure);
    */
    BSP_TRACE("AQS: initializing discharge");
    HAL_GPIO_configure_output_pp(&discharge_r1_gpio);
    HAL_GPIO_set(&discharge_r1_gpio, 0);
}

static void UAIR_BSP_powerzone_discharge_aqs_r1(BSP_powerzone_t zone, int enable_discharge)
{
    BSP_TRACE("AQS: %s powerzone", enable_discharge?"draining":"releasing drain on");

    if (enable_discharge) {
        HAL_GPIO_set(&discharge_r1_gpio, 1);
        HAL_Delay(20);
    } else {
        HAL_GPIO_set(&discharge_r1_gpio, 0);
    }
}


BSP_error_t UAIR_BSP_powerzone_init(void)
{
    BSP_error_t err;
    powerzone_status = 0;
    uint8_t i;


    BSP_board_version_t board = BSP_get_board_version();
    switch (board) {
    case UAIR_NUCLEO_REV1:
        powerzone_config = &powerzones_r1[0];
        break;
    case UAIR_NUCLEO_REV2:
        powerzone_config = &powerzones_r2[0];
        break;
    default:
        powerzone_config = NULL;
        return BSP_ERROR_NO_INIT;
    }

    do {
        // Init lowlevel load switches
        for (i=0; i<=UAIR_POWERZONE_MAX; i++) {
            // Clear eventual callback
            powerzones_notify_callbacks[i].callback = NULL;

            BSP_TRACE("Init powerzone %d", i);
            if (powerzone_config[i].init) {
                BSP_TRACE("> Low-level powerzone %d init", i);
                powerzone_config[i].init((BSP_powerzone_t)i);
            }

            err = UAIR_BSP_LS_Init(powerzone_config[i].loadswitch);
            if (err!=BSP_ERROR_NONE)
                break;
        }

        if (err!=BSP_ERROR_NONE)
            break;

        // Switch off all zones.

        for (i=0; i<=UAIR_POWERZONE_MAX;i++) {
            err = BSP_powerzone_disable((BSP_powerzone_t)i);
            if (err!=BSP_ERROR_NONE)
                break;
        }

    } while (0);

    return err;
}

BSP_error_t BSP_powerzone_enable(BSP_powerzone_t powerzone)
{
    BSP_error_t err;
    if (NULL==powerzone_config) {
        err = BSP_ERROR_NO_INIT;
    } else {
        const struct powerzone_config *pc = &powerzone_config[powerzone];
        BSP_TRACE("Enabling powerzone %d", powerzone);
        if (pc->discharge) {
            pc->discharge(powerzone, 0);
        }
        UAIR_BSP_LS_On(pc->loadswitch);

        if (powerzones_notify_callbacks[powerzone].callback != NULL) {
            powerzones_notify_callbacks[powerzone].callback( powerzones_notify_callbacks[powerzone].userdata, POWER_ON );
        }

        err = BSP_ERROR_NONE;
    }
    return err;
}

BSP_error_t BSP_powerzone_attach_callback(BSP_powerzone_t powerzone, powerzone_notify_callback_t callback, void *userdata)
{
    if (powerzones_notify_callbacks[powerzone].callback != NULL) {
        return BSP_ERROR_BUSY;
    }
    powerzones_notify_callbacks[powerzone].userdata = userdata;
    powerzones_notify_callbacks[powerzone].callback = callback;

    return BSP_ERROR_NONE;
}

BSP_error_t BSP_powerzone_disable(BSP_powerzone_t powerzone)
{
    BSP_error_t err;
    if (NULL==powerzone_config) {
        err = BSP_ERROR_NO_INIT;
    } else {
        const struct powerzone_config *pc = &powerzone_config[powerzone];
        BSP_TRACE("Disabling powerzone %d", powerzone);

        if (powerzones_notify_callbacks[powerzone].callback != NULL) {
            powerzones_notify_callbacks[powerzone].callback( powerzones_notify_callbacks[powerzone].userdata, POWER_OFF );
        }

        UAIR_BSP_LS_Off(pc->loadswitch);

        if (pc->discharge) {
            pc->discharge(powerzone, 1);
        }
        err = BSP_ERROR_NONE;
    }
    return err;
}

static BSP_I2C_busnumber_t UAIR_BSP_powerzone_get_i2c_bus(BSP_powerzone_t zone)
{
    BSP_I2C_busnumber_t busno = BSP_I2C_BUS_NONE;

    switch (zone) {
    case UAIR_POWERZONE_INTERNALI2C:
        busno = BSP_I2C_BUS0;
        break;
    case UAIR_POWERZONE_MICROPHONE:
        busno = BSP_I2C_BUS1;
        break;
    case UAIR_POWERZONE_AMBIENTSENS:
        busno = BSP_I2C_BUS2;
        break;
    default:
         break;
    }
    return busno;
}

void UAIR_BSP_powerzone_discharge_i2c(BSP_powerzone_t zone, int enable_discharge)
{
    BSP_I2C_busnumber_t busno = UAIR_BSP_powerzone_get_i2c_bus(zone);

    if (busno!=BSP_I2C_BUS_NONE) {
        BSP_error_t err = UAIR_BSP_I2C_set_discharge(busno, enable_discharge);
        if (err != BSP_ERROR_NONE) {
            BSP_TRACE("Cannot set discharge on I2C bus: %d", err);
        }
    }
}

static BSP_error_t UAIR_BSP_powerzone_check_i2c_power(BSP_powerzone_t zone, powerstate_t *powerstate)
{
    BSP_error_t ret;
    int sda, scl;

    BSP_I2C_busnumber_t busno = UAIR_BSP_powerzone_get_i2c_bus(zone);

    if (busno!=BSP_I2C_BUS_NONE) {

        ret = UAIR_BSP_I2C_read_sda_scl(busno, &sda, &scl);
        if (ret==BSP_ERROR_NONE) {
            if (sda!=scl) {
                *powerstate = POWER_INCONSISTENT;
            } else if (sda) {
                *powerstate = POWER_ON;
            } else {
                *powerstate = POWER_OFF;
            }
        }
    } else {
        ret = BSP_ERROR_UNKNOWN_COMPONENT;
    }

    return ret;
}


static BSP_error_t UAIR_BSP_powerzone_BIT_zone(BSP_powerzone_t zone)
{
    BSP_error_t err;
    powerstate_t state;

    const struct powerzone_config *pc = &powerzone_config[zone];
    do {
        if (pc->get_power==NULL) {
            err = BSP_ERROR_NONE; /* Cannot check */
            break;
        }

        // Turn off powerzone
        UAIR_BSP_LS_Off(pc->loadswitch);
        // Discharge powerzone
        if (pc->discharge) {
            pc->discharge(zone, 1);
        }
        // Let power drain.
        HAL_delay_us(5000);

        // Remove discharge
        if (pc->discharge) {
            pc->discharge(zone, 0);
        }
        // Let power settle.
        HAL_delay_us(5000);

        err = pc->get_power(zone, &state);

        if (err==BSP_ERROR_NONE) {
            if (state!=POWER_OFF) {
                BSP_TRACE("Zone %d error: powered off, but still powered!", zone);
                BSP_error_set(ERROR_ZONE_POWERZONE, BSP_ERROR_TYPE_POWERZONE_ZONE_STILL_POWERED, (uint8_t)zone, state);
                err = BSP_ERROR_COMPONENT_FAILURE;
                break;
            }
        } else {
            BSP_TRACE("Cannot get power for zone %d", zone);
            break;
        }

        // Now, power on zone
        UAIR_BSP_LS_On(pc->loadswitch);

        HAL_delay_us(5000);

        err = pc->get_power(zone, &state);

        if (err==BSP_ERROR_NONE) {
            if (state!=POWER_ON) {
                BSP_TRACE("Zone %d error: powered on, but still with no power!", zone);
                BSP_error_set(ERROR_ZONE_POWERZONE, BSP_ERROR_TYPE_POWERZONE_ZONE_NO_POWER, (uint8_t)zone, state);
                err = BSP_ERROR_COMPONENT_FAILURE;
            }
        } else {
            BSP_TRACE("Cannot get power for zone %d", zone);
            // Do not return here. We want to disable power below.
        }

        UAIR_BSP_LS_Off(pc->loadswitch);

        // Discharge powerzone
        if (pc->discharge) {
            pc->discharge(zone, 1);
        }
        // Let power drain.
        HAL_delay_us(5000);

        // Remove discharge
        if (pc->discharge) {
            pc->discharge(zone, 0);
        }
    } while (0);

    return err;
}


BSP_error_t UAIR_BSP_powerzone_BIT(void)
{
    int i;
    BSP_error_t ret = BSP_ERROR_NONE;
    if (NULL==powerzone_config)
        return BSP_ERROR_NO_INIT;

    for (i=0;i<=UAIR_POWERZONE_MAX;i++) {
        BSP_TRACE("Performing self-test on powerzone %d", i);
        BSP_error_t err = UAIR_BSP_powerzone_BIT_zone(i);
        if (err!=BSP_ERROR_NONE)
            ret = err;
    }

    return ret;
}


