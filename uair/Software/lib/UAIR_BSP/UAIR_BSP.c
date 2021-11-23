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
#include "pvt/UAIR_BSP_clk_timer_p.h"
#include "pvt/UAIR_BSP_air_quality_p.h"

static BSP_board_version_t board_version = UAIR_UNKNOWN;

static const BSP_config_t bsp_default_config = {
    .bsp_error = NULL,
    .temp_accuracy = TEMP_ACCURACY_MED,
    .hum_accuracy = HUM_ACCURACY_MED
};

static const HAL_GPIODef_t board_version_gpio = {
    .port = GPIOA,
    .pin = GPIO_PIN_4,
    .clock_control = HAL_clk_GPIOA_clock_control
};

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


BSP_error_t BSP_init(const BSP_config_t *config)
{
    BSP_error_t err;
    err = BSP_init_check_board_version();

    if (err!=BSP_ERROR_NONE)
        return err;

    if (NULL==config)
        config = &bsp_default_config;


    /* Ensure that MSI is wake-up system clock */
    __HAL_RCC_WAKEUPSTOP_CLK_CONFIG(RCC_STOP_WAKEUPCLOCK_MSI);

    UAIR_HAL_SysClk_Init(false);

    UAIR_BSP_LED_Init(LED_BLUE);
    UAIR_BSP_LED_Init(LED_RED);
    UAIR_BSP_LED_Init(LED_GREEN);


    //UAIR_BM_Init();
   // (void)UAIR_BM_GetBatteryVoltage();
   // UAIR_BM_DeInit();

    //if (!UAIR_BM_OnBattery()) {
        //DEBUG_USART_CLK_ENABLE();
        //UAIR_BSP_USART_Init();
        TRACER_INIT();
//    }

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


    // Power-on subsystems

#define ERRCHK(x, msg...) \
    err = x ; \
    if (err!=BSP_ERROR_NONE) {\
    BSP_TRACE("Error: " msg); \
    break; }

    BSP_TRACE("Configuring devices");

    do {
        ERRCHK( UAIR_BSP_powerzone_init(), "Cannot init powerzones!" );
        /* Microphone needs to be ON otherwise it will sometimes
         bring SDA/SCL down */
        ERRCHK( BSP_powerzone_enable(UAIR_POWERZONE_MICROPHONE), "Cannot enable MICROPHONE powerzone");
        ERRCHK( BSP_powerzone_enable(UAIR_POWERZONE_INTERNALI2C), "Cannot enable INTERNAL powerzone");

        //ERRCHK( BSP_powerzone_enable(UAIR_POWERZONE_INTERNALI2C), "Cannot enable internal I2C powerzone" );



        //ERRCHK( BSP_powerzone_enable(UAIR_POWERZONE_AMBIENTSENS), "Cannot enable AMBIENTSENS powerzone");
        ERRCHK( UAIR_BSP_external_temp_hum_init(config->temp_accuracy, config->hum_accuracy), "Error initialising external temperature/humidity sensor" );

        ERRCHK( UAIR_BSP_internal_temp_hum_init(), "Error initialising internal temperature/humidity sensor" );

        ERRCHK( UAIR_BSP_air_quality_init(), "Error initializing AIR quality" );


    } while (0);

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

