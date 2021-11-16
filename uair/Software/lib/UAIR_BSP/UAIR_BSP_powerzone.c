#include "BSP.h"
#include "UAIR_BSP_powerzone.h"
#include "pvt/UAIR_BSP_powerzone_p.h"
#include "HAL_gpio.h"

static uint8_t powerzone_status;

struct powerzone_config {
    Load_Switch_TypeDef loadswitch;
    void (*init)(BSP_powerzone_t);
    void (*discharge)(BSP_powerzone_t, int enable_discharge);
};

static void UAIR_init_indirect_discharge(BSP_powerzone_t);
static void UAIR_BSP_powerzone_discharge_aqs_r1(BSP_powerzone_t, int enable_discharge);


static const struct powerzone_config powerzones_r1[UAIR_POWERZONE_MAX+1] = {
    { LOAD_SWITCH_INTERNAL, NULL, NULL },
    { LOAD_SWITCH_MICROPHONE, NULL, NULL },
    { LOAD_SWITCH_AIRQUALITYSENSOR, &UAIR_init_indirect_discharge, &UAIR_BSP_powerzone_discharge_aqs_r1 }
};

// Discharge for r1
static const HAL_GPIODef_t discharge_r1_gpio = {
    .pin = LOAD_SWITCH2_DISCHG_PIN,
    .port = LOAD_SWITCH2_DISCHG_GPIO_PORT,
    .clock_control = LOAD_SWITCH2_DISCHG_CLK_CONTROL
};

static const struct powerzone_config *powerzone_config;

static void UAIR_init_indirect_discharge(BSP_powerzone_t zone)
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
        HAL_Delay(1);
        HAL_GPIO_set(&discharge_r1_gpio, 0);
        HAL_Delay(1);
        HAL_GPIO_set(&discharge_r1_gpio, 1);
        HAL_Delay(1);
        HAL_GPIO_set(&discharge_r1_gpio, 0);
        HAL_Delay(1);
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
    default:
        powerzone_config = NULL;
        return BSP_ERROR_NO_INIT;
    }

    do {
        // Init lowlevel load switches
        for (i=0; i<=UAIR_POWERZONE_MAX; i++) {
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
        err = BSP_ERROR_NONE;
    }
    return err;
}

BSP_error_t BSP_powerzone_disable(BSP_powerzone_t powerzone)
{
    BSP_error_t err;
    if (NULL==powerzone_config) {
        err = BSP_ERROR_NO_INIT;
    } else {
        const struct powerzone_config *pc = &powerzone_config[powerzone];
        BSP_TRACE("Disabling powerzone %d", powerzone);

        UAIR_BSP_LS_Off(pc->loadswitch);

        if (pc->discharge) {
            pc->discharge(powerzone, 1);
        }
        err = BSP_ERROR_NONE;
    }
    return err;
}


