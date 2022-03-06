#include "LmHandler.h"
#include <stdlib.h>
#include "stm32wlxx_hal_def.h"

// Host-mode LoRaWan
static LmHandlerCallbacks_t *callbacks;

void LmHandlerProcess(void)
{
    
}

LmHandlerErrorStatus_t LmHandlerSend(LmHandlerAppData_t *appData, LmHandlerMsgTypes_t isTxConfirmed,
                                     TimerTime_t *nextTxIn, bool allowDelayedTx)
{
    HLOG("LoRa transmit port %d size %d",
         appData->Port,
         appData->BufferSize);

    return 0;
}

void LmHandlerJoin(ActivationType_t mode)
{
    // Start join process.
    LmHandlerJoinParams_t JoinParams;

    JoinParams.Mode = ACTIVATION_TYPE_ABP;
    JoinParams.Status = LORAMAC_HANDLER_SUCCESS;

    callbacks->OnJoinRequest(&JoinParams);
}

LmHandlerErrorStatus_t LmHandlerConfigure(LmHandlerParams_t *handlerParams)
{
    return 0;
}

LmHandlerErrorStatus_t LmHandlerInit(LmHandlerCallbacks_t *handlerCallbacks)
{
    callbacks = handlerCallbacks;
    return 0;
}


