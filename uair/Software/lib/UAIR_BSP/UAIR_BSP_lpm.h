#ifndef UAIR_BSP_LPM_H__
#define UAIR_BSP_LPM_H__

#ifdef __cplusplus
extern "C" {
#endif

/**
 * Enter Low power mode. This mode is either STOP2 or STOP1, depending on
 */
void BSP_LPM_enter_low_power_mode(void);

#ifdef __cplusplus
}
#endif

#endif
