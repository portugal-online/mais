#include "UAIR_BSP.h"
//#include "UAIR_bm.h"
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

static BSP_board_version_t board_version = UAIR_UNKNOWN;

static const BSP_config_t bsp_default_config = {
    .bsp_error = NULL,
    .temp_accuracy = TEMP_ACCURACY_MED,
    .hum_accuracy = HUM_ACCURACY_MED,
    .skip_shield_init = false,
    .high_performance = false
};

static const HAL_GPIODef_t board_version_gpio = {
    .port = GPIOA,
    .pin = GPIO_PIN_4,
    .clock_control = HAL_clk_GPIOA_clock_control
};

typedef enum reset_cause_e
{
    RESET_CAUSE_UNKNOWN = 0,
    RESET_CAUSE_LOW_POWER_RESET,
    RESET_CAUSE_WINDOW_WATCHDOG_RESET,
    RESET_CAUSE_INDEPENDENT_WATCHDOG_RESET,
    RESET_CAUSE_SOFTWARE_RESET,
    RESET_CAUSE_POWER_ON_POWER_DOWN_RESET,
    RESET_CAUSE_EXTERNAL_RESET_PIN_RESET,
    RESET_CAUSE_BROWNOUT_RESET,
} reset_cause_t;

static const char * reset_cause_get_name(reset_cause_t reset_cause);
static reset_cause_t reset_cause_get(void);

void BSP_get_default_config(BSP_config_t *dest)
{
    memcpy(dest, &bsp_default_config, sizeof(bsp_default_config));
}

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

static const char *BSP_get_board_name(void)
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
#endif

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


    /* Ensure that MSI is wake-up system clock */
    __HAL_RCC_WAKEUPSTOP_CLK_CONFIG(RCC_STOP_WAKEUPCLOCK_MSI);

    UAIR_HAL_SysClk_Init( !(config->high_performance) );

    UAIR_BSP_LED_Init(LED_BLUE);
    UAIR_BSP_LED_Init(LED_RED);
    UAIR_BSP_LED_Init(LED_GREEN);

    UAIR_BSP_UART_DMA_Init();



    HAL_BM_Init();
    (void)HAL_BM_GetBatteryVoltage();
    HAL_BM_DeInit();

    if (!HAL_BM_OnBattery()) {
        //DEBUG_USART_CLK_ENABLE();
        TRACER_INIT();
    }

    UAIR_BSP_DP_Init(DEBUG_PIN1);
    UAIR_BSP_DP_Init(DEBUG_PIN2);
    UAIR_BSP_DP_Init(DEBUG_PIN3);

    UAIR_BSP_LPTIM_Init();

    /*Initialises timer and RTC*/
    UTIL_TIMER_Init();

    /* Initialize the Low Power Manager and Debugger */
#if defined(DEBUGGER_ON) && (DEBUGGER_ON == 1)
    UAIR_LPM_Init(UAIR_LPM_SLEEP_STOP_DEBUG_MODE);
#elif defined(DEBUGGER_ON) && (DEBUGGER_ON == 0)
    UAIR_LPM_Init(UAIR_LPM_SLEEP_STOP_MODE);
#endif

    BSP_TRACE("Starting BSP on board %s", BSP_get_board_name());
    BSP_TRACE("Reset: %s", reset_cause_get_name(reset_cause_get()));

    if (config->skip_shield_init==false) {

        err =UAIR_BSP_powerzone_init();
        if (err!=BSP_ERROR_NONE) {
            BSP_LED_on(LED_RED);
            BSP_TRACE("Cannot init powerzones!");
            return err;
        }

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
        if (err==BSP_ERROR_NONE) {
            BSP_TRACE("%s: PASS", "Powerzone BIT");
        } else {
            BSP_TRACE("%s: FAIL (error %d)", "Powerzone BIT", err);
        }

        if (err!=BSP_ERROR_NONE) {
            BSP_LED_on(LED_RED);
            return err;
        }

#ifdef TEST3
        BSP_STOP_FOR_POWER_CALCULATION();
        // Test result 1.9V @17C: ~5uA.
#endif

        // Power-on subsystems

#define ERRCHK_FATAL(x, msg...) \
    err = x ; \
    if (err!=BSP_ERROR_NONE) {\
    BSP_TRACE("Error: " msg); \
    break; \
    }
#define ERRCHK(x, msg...) \
    err = x ; \
    if (err!=BSP_ERROR_NONE) {\
    BSP_TRACE("Error: " msg); \
    BSP_LED_on(LED_RED); \
    }

        BSP_TRACE("Configuring devices");

        do {
            /* Microphone needs to be ON otherwise it will sometimes
             bring SDA/SCL down. This is only required for r1. */
            if (BSP_get_board_version()==UAIR_NUCLEO_REV1) {
                ERRCHK_FATAL( BSP_powerzone_enable(UAIR_POWERZONE_MICROPHONE), "Cannot enable MICROPHONE powerzone");
                ERRCHK_FATAL( BSP_powerzone_enable(UAIR_POWERZONE_INTERNALI2C), "Cannot enable INTERNAL powerzone");
            }
            //ERRCHK( BSP_powerzone_enable(UAIR_POWERZONE_INTERNALI2C), "Cannot enable internal I2C powerzone" );



            //ERRCHK( BSP_powerzone_enable(UAIR_POWERZONE_AMBIENTSENS), "Cannot enable AMBIENTSENS powerzone");

            ERRCHK( UAIR_BSP_external_temp_hum_init(config->temp_accuracy, config->hum_accuracy), "Error initialising external temperature/humidity sensor" );
            ERRCHK( UAIR_BSP_internal_temp_hum_init(), "Error initialising internal temperature/humidity sensor" );
            ERRCHK( UAIR_BSP_air_quality_init(), "Error initializing AIR quality" );
            ERRCHK( UAIR_BSP_microphone_init(), "Error initializing microphone" );

        } while (0);

#ifdef TEST4
        BSP_STOP_FOR_POWER_CALCULATION();
        // Test result 1.9V @17C: Mic: 21uA
        // Test result 1.9V @17C: Mic+SHTC3(sleep): 21uA
        // Test result 1.9V @17C: Mic+SHTC3+HS300X: 21uA
        // Test result 1.9V @17C: Mic+SHTC3+HS300X+ZMOD4510: 21uA
#endif

    }

    err = BSP_ERROR_NONE;
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
static const char * reset_cause_get_name(reset_cause_t reset_cause)
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

void  __attribute__((noreturn)) BSP_FATAL(void)
{
    while (1) {
        __WFI();
    }
}
