/**
 * Copyright (C) 2021, 2022 MAIS Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

/**
 * @file UAIR_BSP.c
 * @brief UAIR BSP Core implementation
 * 
 * @copyright Copyright (C) 2021, 2022 MAIS Project
 *
 * @ingroup UAIR_BSP_CORE
 */

#include "UAIR_BSP.h"
#include "UAIR_tracer.h"
#include "UAIR_rtc.h"
#include "UAIR_lpm.h"
#include "UAIR_BSP_flash.h"
#include "HAL.h"
#include "pvt/UAIR_BSP_internaltemp_p.h"
#include "pvt/UAIR_BSP_externaltemp_p.h"
#include "pvt/UAIR_BSP_powerzone_p.h"
#include "pvt/UAIR_BSP_gpio_p.h"
#include "pvt/UAIR_BSP_i2c_p.h"
#include "pvt/UAIR_BSP_clk_timer_p.h"
#include "pvt/UAIR_BSP_air_quality_p.h"
#include "pvt/UAIR_BSP_microphone_p.h"
#include "pvt/UAIR_BSP_watchdog_p.h"
#include "pvt/UAIR_BSP_commissioning_p.h"


#define ERRCHK_FATAL(x, msg...) \
    err = x ; \
    if (err!=BSP_ERROR_NONE) {\
    BSP_TRACE("Error: " msg); \
    BSP_LED_on(LED_RED); \
    BSP_FATAL();  \
    break; \
    }

#define ERRCHK(x, index, msg...) \
    err = x ; \
    if (err!=BSP_ERROR_NONE) {\
    BSP_TRACE("Error: " msg); \
    BSP_error_set(ERROR_ZONE_BSP, 1, index, err);\
    }

/* Tests */

#undef TEST1
#undef TEST2
#undef TEST3
#undef TEST4
#undef STRESS_I2C

#if defined (TEST1) || defined(TEST2) || defined(TEST3) || defined(TEST4)
#define BSP_IWDG_TIMEOUT_SECONDS (60U)
#else
#define BSP_IWDG_TIMEOUT_SECONDS (5U)
#endif


#ifdef STRESS_I2C

extern BSP_error_t UAIR_BSP_air_quality_sequencer_completed(void);
extern BSP_error_t UAIR_BSP_air_quality_read_adc(void);

static void stress_i2c()
{
#define FRAME_MASK 0x3FF
 //   bool error = false;
    BSP_error_t err;
    uint32_t iter = 0;
    while (1) {
        err = UAIR_BSP_air_quality_read_adc();
        /*
        if (error) {
            BSP_TRACE("Stopping due to previous error, ret %d.", err);
            while (1) {
                UAIR_BSP_watchdog_kick();
                HAL_Delay(500);
            }
        } */
        if (err!=BSP_ERROR_NONE) {
            BSP_TRACE("Air quality read error");
            //error = true;
        }
        iter++;
        if ((iter & FRAME_MASK) == 0x0) {
            BSP_TRACE("Frame check");
            UAIR_BSP_watchdog_kick();
        }

    }
}
#endif

static BSP_board_version_t board_version = UAIR_UNKNOWN;
static bool s_network_enabled = false;

static const BSP_config_t bsp_default_config = {
    .bsp_error = NULL,
    .temp_accuracy = TEMP_ACCURACY_MED,
    .hum_accuracy = HUM_ACCURACY_MED,
    .skip_shield_init = false,
    .high_performance = false,
    .force_uart_on    = false,
    .disable_watchdog = false,
    .disable_network  = false
};

static const HAL_GPIODef_t board_version_gpio = {
    .port = GPIOA,
    .pin = GPIO_PIN_4,
    .clock_control = HAL_clk_GPIOA_clock_control
};

static reset_cause_t s_reset_cause = RESET_CAUSE_UNKNOWN;

static reset_cause_t reset_cause_get(void);

reset_cause_t BSP_get_reset_cause(void)
{
    return s_reset_cause;
}


void BSP_get_default_config(BSP_config_t *dest)
{
    memcpy(dest, &bsp_default_config, sizeof(BSP_config_t));
}
/**
 * @brief Return the current board version
 * @ingroup UAIR_BSP_CORE
 *
 *
 * This function returns the current board which is running the firmware.
 *
 * @return The board version 
 */
BSP_board_version_t BSP_get_board_version()
{
    return board_version;
}

static BSP_error_t BSP_init_check_board_version(void)
{
    board_version_gpio.clock_control(1);
    int pin_val;

    HAL_GPIO_configure_input_pu(&board_version_gpio);
    // At this point we do not have timer, but we need to delay a bit
    volatile uint16_t dly = 0xffff;
    while (dly--) {
        __NOP();
    }
    pin_val = HAL_GPIO_read(&board_version_gpio);
    HAL_GPIO_configure_input_analog(&board_version_gpio);
    board_version_gpio.clock_control(0);

    if (pin_val==0) {
        board_version = UAIR_NUCLEO_REV2;
    } else {
        board_version = UAIR_NUCLEO_REV1;
    }
    return BSP_ERROR_NONE;
}


const char *BSP_get_board_name(void)
{
    const char *boardname = "UNKNOWN";
    switch (board_version) {
    case UAIR_NUCLEO_REV1:
        boardname =  "uAir NUCLEO rev. 1";
        break;
    case UAIR_NUCLEO_REV2:
        boardname =  "uAir NUCLEO rev. 2";
        break;
    default:
        break;
    }
    return boardname;
}


#ifdef HOSTMODE
extern void bsp_preinit();
extern void bsp_postinit(HAL_StatusTypeDef result);
extern void bsp_deinit();
#endif

void UAIR_BSP_internal_powerzone_changed(void *userdata, const powerstate_t state)
{
    UAIR_BSP_internal_temp_hum_powerzone_changed(userdata, state);
}

void UAIR_BSP_microphone_powerzone_changed(void *userdata, const powerstate_t state)
{
}

void UAIR_BSP_ambientsens_powerzone_changed(void *userdata, const powerstate_t state)
{
    UAIR_BSP_air_quality_powerzone_changed(
                                           UAIR_BSP_air_quality_get_zmod(),
                                           state
                                          );

}

BSP_error_t UAIR_BSP_link_powerzones()
{
    BSP_error_t err;
    BSP_TRACE("Linking powerzones");
    err = BSP_powerzone_attach_callback(UAIR_POWERZONE_INTERNALI2C, &UAIR_BSP_internal_powerzone_changed, NULL);
    if (err == BSP_ERROR_NONE)
    {
        err = BSP_powerzone_attach_callback(UAIR_POWERZONE_MICROPHONE,  &UAIR_BSP_microphone_powerzone_changed, NULL);
        if (err == BSP_ERROR_NONE)
        {
            err = BSP_powerzone_attach_callback(UAIR_POWERZONE_AMBIENTSENS, &UAIR_BSP_ambientsens_powerzone_changed, NULL);
        }
    }
    return err;
}


static void disable_uart_pins()
{
    GPIO_InitTypeDef gpio_init_structure = {0};

    gpio_init_structure.Mode = GPIO_MODE_ANALOG;
    gpio_init_structure.Speed = GPIO_SPEED_FREQ_LOW;

    gpio_init_structure.Pin = DEBUG_USART_TX_PIN | DEBUG_USART_RX_PIN;
    gpio_init_structure.Pull = GPIO_NOPULL;

    //STATIC_ASSERT(DEBUG_USART_TX_GPIO_PORT == DEBUG_USART_RX_GPIO_PORT);

    HAL_GPIO_Init(DEBUG_USART_TX_GPIO_PORT, &gpio_init_structure);

    //STATIC_ASSERT(DEBUG_ALT_USART_TX_GPIO_PORT == DEBUG_ALT_USART_RX_GPIO_PORT);
    gpio_init_structure.Pin = DEBUG_ALT_USART_TX_PIN | DEBUG_ALT_USART_RX_PIN;
    HAL_GPIO_Init(DEBUG_ALT_USART_TX_GPIO_PORT, &gpio_init_structure);
}

/**
 * @brief Initialize the BSP layer.
 * @ingroup UAIR_BSP_CORE
 *
 *
 * This function initializes the BSP layer with the specified configuration.
 * This function should be called within main() prior to doing any operation on the hardware.
 *
 * The default configuration can be obtained with BSP_get_default_config() and used to initialize the BSP.
 *
 * This function will:
 * - Verify that the board is recognised
 * - Initialize all system clocks
 * - Initialize the on-board LEDs, if applicable for the board
 * - Initialize the DMA subsystem
 * - Enable the serial monitoring system if not running on battery \note If the supply voltage is below 3.1V it is assumed
 that the system is running on battery (production) and hence the serial monitoring is not available.
 * - Initialize the debug pins, if applicable for the board
 * - Initialize the internal timers (including real-time clock) and the scheduler.
 * - Initialize the independent watchdog.
 *
 * If the config field skip_shield_init is not set, then this function will:
 * - Initialize all powerzones (see \ref UAIR_BSP_POWERZONE)
 * - Power up and initialize the external temperature sensor.
 * - Power up and initialize the internal temperature sensor.
 * - Power up and initialize the air quality sensor.
 * - Power up and initialize the microphone
 *
 * @param config Pointer to a config structure with the required configuration. This parameter cannot be NULL.
 *
 * @returns \ref BSP_ERROR_NONE if successful, a \ref BSP_error_t error if any error occurred during initalization. More error information
 *          can be obtained using \ref BSP_error_get_last_error()
 */
BSP_error_t BSP_init(const BSP_config_t *config)
{
#ifdef HOSTMODE
    bsp_preinit();
#endif

    BSP_error_t err;
    err = BSP_init_check_board_version();

    if (err!=BSP_ERROR_NONE)
        return err;

    if (NULL==config)
        config = &bsp_default_config;

    s_network_enabled  = !(config->disable_network);

    s_reset_cause = reset_cause_get();

    /* Ensure that MSI is wake-up system clock */
    __HAL_RCC_WAKEUPSTOP_CLK_CONFIG(RCC_STOP_WAKEUPCLOCK_MSI);

    UAIR_HAL_SysClk_Init( !(config->high_performance) );

    UAIR_BSP_LED_Init(LED_BLUE);
    UAIR_BSP_LED_Init(LED_RED);
    UAIR_BSP_LED_Init(LED_GREEN);

    UAIR_BSP_LPM_init();

    HAL_PWREx_EnableBORPVD_ULP(); // Enable ultra low-power.

    UAIR_BSP_FR_Init();

    UAIR_BSP_UART_DMA_Init();

    UAIR_BSP_BM_Init();

#ifdef TEST0
    while (1) {
        UAIR_LPM_EnterLowPower();
        // Test result 1.9V @17C: ~5uA.
    }
#endif

#undef SKIP_BAT_MEASUREMENT

#ifndef SKIP_BAT_MEASUREMENT

    UAIR_BSP_BM_PrepareAcquisition();

    battery_measurements_t batt;

    UAIR_BSP_BM_MeasureBlocking(&batt);

    if ( (batt.supply_voltage_mv >= 3100) || config->force_uart_on) {
        TRACER_INIT();
    } else {
        // TBD - disable UART pins
        disable_uart_pins();
    }
    BSP_TRACE("Running on supply voltage: %dmV", batt.supply_voltage_mv);

    BSP_TRACE("Network is %s", s_network_enabled?"enabled":"DISABLED");


//    const uint16_t *p = UAIR_BSP_BM_GetRawValues();

//    BSP_TRACE("%08x %08x %08x",p[0], p[1], p[2]);

    UAIR_BSP_BM_EndAcquisition();
    UAIR_BSP_BM_DeInit();
#endif

    UAIR_BSP_DP_Init(DEBUG_PIN1);
    UAIR_BSP_DP_Init(DEBUG_PIN2);


    UAIR_BSP_LPTIM_Init();

    /*Initialises timer and RTC*/
    UTIL_TIMER_Init();

    /* Initialize the Low Power Manager and Debugger */
#if defined(RELEASE) && (RELEASE==1)
    UAIR_LPM_Init(UAIR_LPM_SLEEP_STOP_MODE);
#else
# if defined(DEBUGGER_ON) && (DEBUGGER_ON == 1)
    UAIR_LPM_Init(UAIR_LPM_SLEEP_STOP_DEBUG_MODE);
# elif defined(DEBUGGER_ON) && (DEBUGGER_ON == 0)
    UAIR_LPM_Init(UAIR_LPM_SLEEP_STOP_MODE);
# endif
#endif
    /* Initialize watchdog */
    if (! config->disable_watchdog)
    {
        if (UAIR_BSP_watchdog_init(BSP_IWDG_TIMEOUT_SECONDS) != BSP_ERROR_NONE)
        {
            BSP_TRACE("Cannot initialize watchdog");
            BSP_FATAL();
        }
    }

    if (UAIR_BSP_commissioning_init()!=BSP_ERROR_NONE)
    {
        BSP_TRACE("Cannot initialise commissioning");
    }

    BSP_TRACE("Starting BSP on board %s", BSP_get_board_name());

    {
        uint8_t deveui[8];
        (void)BSP_commissioning_get_device_eui(deveui);
        BSP_TRACE("Device EUI: [%02X%02X%02X%02X%02X%02X%02X%02X]",
                  deveui[0], deveui[1], deveui[2], deveui[3],
                  deveui[4], deveui[5], deveui[6], deveui[7]);
    }


    if (config->skip_shield_init==false) {

        err =UAIR_BSP_powerzone_init();
        if (err!=BSP_ERROR_NONE) {
            BSP_LED_on(LED_RED);
            BSP_TRACE("Cannot init powerzones!");
            return err;
        }
        UAIR_BSP_link_powerzones();

        //#define TEST4

        // TEST1: calculate power consumption with all zones off.
#ifdef TEST1
        while (1) {
            UAIR_LPM_EnterLowPower();
            // Test result 1.9V @17C: ~5uA.
        }
#endif

        // TEST2: calculate power consumption with microphone ON and idle.
#ifdef TEST2
        UAIR_BSP_microphone_init();
        BSP_STOP_FOR_POWER_CALCULATION();

        // Test result 1.9V @17C: ~20uA. A bit higher than expected (15uA).
        // Test result 1.9V @17C, mic not connected: ~5uA.
#endif


        // run POST
        err = UAIR_BSP_powerzone_BIT();

        if (err==BSP_ERROR_NONE)
        {
            BSP_TRACE("%s: PASS", "Powerzone BIT");
        }
        else
        {
            BSP_TRACE("%s: FAIL (error %d)", "Powerzone BIT", err);
        }

#ifdef TEST3
        BSP_STOP_FOR_POWER_CALCULATION();
        // Test result 1.9V @17C: ~5uA.
#endif

        // Power-on subsystems


        BSP_TRACE("Configuring devices");

        UAIR_BSP_external_temp_set_defaults(config->temp_accuracy, config->hum_accuracy);

        do {
            /* Microphone needs to be ON otherwise it will sometimes
             bring SDA/SCL down. This is only required for r1. */
            if (BSP_get_board_version()==UAIR_NUCLEO_REV1) {
                ERRCHK_FATAL( BSP_powerzone_ref(UAIR_POWERZONE_MICROPHONE), "Cannot enable MICROPHONE powerzone");
                ERRCHK_FATAL( BSP_powerzone_ref(UAIR_POWERZONE_INTERNALI2C), "Cannot enable INTERNAL powerzone");
            }
            //ERRCHK( BSP_powerzone_enable(UAIR_POWERZONE_INTERNALI2C), "Cannot enable internal I2C powerzone" );



            //ERRCHK( BSP_powerzone_enable(UAIR_POWERZONE_AMBIENTSENS), "Cannot enable AMBIENTSENS powerzone");

            ERRCHK( UAIR_BSP_external_temp_hum_init(), 1, "Error initialising external temperature/humidity sensor" );
            ERRCHK( UAIR_BSP_internal_temp_hum_init(), 2, "Error initialising internal temperature/humidity sensor" );
            ERRCHK( UAIR_BSP_air_quality_init(), 3, "Error initializing AIR quality" );
            ERRCHK( UAIR_BSP_microphone_init(), 4, "Error initializing microphone" );

            if (BSP_get_board_version()==UAIR_NUCLEO_REV1) {
                // Unref zones. Power will still be applied if the driver succeeded
                BSP_powerzone_unref(UAIR_POWERZONE_MICROPHONE);
                BSP_powerzone_unref(UAIR_POWERZONE_INTERNALI2C);
            }
        } while (0);

#ifdef TEST4
        BSP_STOP_FOR_POWER_CALCULATION();
        // Test result 1.9V @17C: Mic: 21uA
        // Test result 1.9V @17C: Mic+SHTC3(sleep): 21uA
        // Test result 1.9V @17C: Mic+SHTC3+HS300X: 21uA
        // Test result 1.9V @17C: Mic+SHTC3+HS300X+ZMOD4510: 21uA
#endif

#ifdef STRESS_I2C
        stress_i2c();
#endif

    }
    if (err != BSP_ERROR_NONE)
    {
        // Notify upper layers. TBD
        BSP_FATAL();
    }
#ifdef HOSTMODE
    bsp_postinit(err);
#endif
    return err;
}

void ADV_TRACER_PreSendHook(void)
{
    UAIR_LPM_SetStopMode((1 << UAIR_LPM_UART_TRACER), UAIR_LPM_DISABLE);
}

/* Re-enable StopMode when traces have been printed */
void ADV_TRACER_PostSendHook(void)
{
    UAIR_LPM_SetStopMode((1 << UAIR_LPM_UART_TRACER), UAIR_LPM_ENABLE);
}



/**
 * @brief This function configures the source of the time base.
 * @brief  don't enable systick
 * @param TickPriority: Tick interrupt priority.
 * @return HAL status
 */
HAL_StatusTypeDef HAL_InitTick(uint32_t TickPriority)
{
    /*Initialize the RTC services */
    return HAL_OK;
}

/**
 * @brief Provide a tick value in millisecond measured using RTC
 * @note This function overwrites the __weak one from HAL
 * @return tick value
 */
uint32_t HAL_GetTick(void)
{
    return UAIR_RTC_GetTimerValue();
}

/**
 * @brief This function provides delay (in ms)
 * @param Delay: specifies the delay time length, in milliseconds.
 * @return None
 */
void HAL_Delay(__IO uint32_t Delay)
{
    UAIR_RTC_DelayMs(Delay); /* based on RTC */
}


/// @brief      Obtain the STM32 system reset cause
/// @param      None
/// @return     The system reset cause
static reset_cause_t reset_cause_get(void)
{
    reset_cause_t reset_cause;

    if (__HAL_RCC_GET_FLAG(RCC_FLAG_LPWRRST))
    {
        reset_cause = RESET_CAUSE_LOW_POWER_RESET;
    }
    else if (__HAL_RCC_GET_FLAG(RCC_FLAG_WWDGRST))
    {
        reset_cause = RESET_CAUSE_WINDOW_WATCHDOG_RESET;
    }
    else if (__HAL_RCC_GET_FLAG(RCC_FLAG_IWDGRST))
    {
        reset_cause = RESET_CAUSE_INDEPENDENT_WATCHDOG_RESET;
    }
    else if (__HAL_RCC_GET_FLAG(RCC_FLAG_SFTRST))
    {
        reset_cause = RESET_CAUSE_SOFTWARE_RESET; // This reset is induced by calling the ARM CMSIS `NVIC_SystemReset()` function!
    }
    //else if (__HAL_RCC_GET_FLAG(RCC_FLAG_PORRST))
   // {
    //    reset_cause = RESET_CAUSE_POWER_ON_POWER_DOWN_RESET;
    //}
    else if (__HAL_RCC_GET_FLAG(RCC_FLAG_PINRST))
    {
        reset_cause = RESET_CAUSE_EXTERNAL_RESET_PIN_RESET;
    }
    // Needs to come *after* checking the `RCC_FLAG_PORRST` flag in order to ensure first that the reset cause is 
    // NOT a POR/PDR reset. See note below. 
    else if (__HAL_RCC_GET_FLAG(RCC_FLAG_BORRST))
    {
        reset_cause = RESET_CAUSE_BROWNOUT_RESET;
    }
    else
    {
        reset_cause = RESET_CAUSE_UNKNOWN;
    }

    // Clear all the reset flags or else they will remain set during future resets until system power is fully removed.
    __HAL_RCC_CLEAR_RESET_FLAGS();

    return reset_cause; 
}

// Note: any of the STM32 Hardware Abstraction Layer (HAL) Reset and Clock Controller (RCC) header
// files, such as "STM32Cube_FW_F7_V1.12.0/Drivers/STM32F7xx_HAL_Driver/Inc/stm32f7xx_hal_rcc.h",
// "STM32Cube_FW_F2_V1.7.0/Drivers/STM32F2xx_HAL_Driver/Inc/stm32f2xx_hal_rcc.h", etc., indicate that the 
// brownout flag, `RCC_FLAG_BORRST`, will be set in the event of a "POR/PDR or BOR reset". This means that a 
// Power-On Reset (POR), Power-Down Reset (PDR), OR Brownout Reset (BOR) will trip this flag. See the 
// doxygen just above their definition for the `__HAL_RCC_GET_FLAG()` macro to see this:
// "@arg RCC_FLAG_BORRST: POR/PDR or BOR reset." <== indicates the Brownout Reset flag will *also* be set in 
// the event of a POR/PDR. 
// Therefore, you must check the Brownout Reset flag, `RCC_FLAG_BORRST`, *after* first checking the 
// `RCC_FLAG_PORRST` flag in order to ensure first that the reset cause is NOT a POR/PDR reset.


/// @brief      Obtain the system reset cause as an ASCII-printable name string from a reset cause type
/// @param[in]  reset_cause     The previously-obtained system reset cause
/// @return     A null-terminated ASCII name string describing the system reset cause
const char * BSP_reset_cause_get_name(reset_cause_t reset_cause)
{
    const char * reset_cause_name = "TBD";

    switch (reset_cause)
    {
        case RESET_CAUSE_UNKNOWN:
            reset_cause_name = "UNKNOWN";
            break;
        case RESET_CAUSE_LOW_POWER_RESET:
            reset_cause_name = "LOW_POWER_RESET";
            break;
        case RESET_CAUSE_WINDOW_WATCHDOG_RESET:
            reset_cause_name = "WINDOW_WATCHDOG_RESET";
            break;
        case RESET_CAUSE_INDEPENDENT_WATCHDOG_RESET:
            reset_cause_name = "INDEPENDENT_WATCHDOG_RESET";
            break;
        case RESET_CAUSE_SOFTWARE_RESET:
            reset_cause_name = "SOFTWARE_RESET";
            break;
        case RESET_CAUSE_POWER_ON_POWER_DOWN_RESET:
            reset_cause_name = "POWER-ON_RESET (POR) / POWER-DOWN_RESET (PDR)";
            break;
        case RESET_CAUSE_EXTERNAL_RESET_PIN_RESET:
            reset_cause_name = "EXTERNAL_RESET_PIN_RESET";
            break;
        case RESET_CAUSE_BROWNOUT_RESET:
            reset_cause_name = "BROWNOUT_RESET (BOR)";
            break;
    }

    return reset_cause_name;
}

void BSP_deinit()
{
#ifdef HOSTMODE
    bsp_deinit();
#endif
}

/**
 * @brief Trigger a fatal BSP error
 */
extern int _estack;

void  __attribute__((noreturn)) BSP_FATAL(void)
{
 
#ifdef HOSTMODE
    exit(-1);
#else

#if (defined RELEASE) && (RELEASE==1)
    /* No prints for release */
#else
    TRACER_INIT();

    uint32_t *fp = __builtin_frame_address(0);
    uint32_t *es = (uint32_t*)&_estack;

    BSP_error_detail_t err = BSP_error_get_last_error();
    BSP_TRACE("FATAL ERROR: %d %d %d %d", err.zone, err.type, err.index, err.value);
    BSP_TRACE("Stack trace");
    while (fp<es) {
        BSP_TRACE(" > %08x %08x", fp, *fp);
        fp++;
    }
#endif
    __disable_irq();
    while (1) {
        __WFI();
    }
#endif
}
#ifndef HOSTMODE
void abort()
{
    BSP_FATAL();
}
#endif

void BSP_SystemResetRequest()
{
    BSP_FATAL();
}

bool BSP_network_enabled(void)
{
    return s_network_enabled;
}
