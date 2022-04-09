#ifndef LM_HANDLER_H__
#define LM_HANDLER_H__

#include "LoRaMacTypes.h"
#include "LoRaMac.h"

// Host-mode

typedef struct LmHandlerAppData_s
{
  uint8_t Port;
  uint8_t BufferSize;
  uint8_t *Buffer;
} LmHandlerAppData_t;

typedef struct {
} LmHandlerJoinParams_t;

typedef struct {
} LmHandlerRxParams_t;

typedef struct {
} LmHandlerTxParams_t;

typedef int ActivationType_t;

typedef struct LmHandlerParams_s
{
  LoRaMacRegion_t ActiveRegion;
  DeviceClass_t DefaultClass;
  bool AdrEnable;
  int8_t TxDatarate;
  bool DutyCycleEnabled;
  uint8_t PingPeriodicity;
} LmHandlerParams_t;


typedef struct LmHandlerCallbacks_s
{
  uint8_t (*GetBatteryLevel)(void);
  uint16_t (*GetTemperature)(void);
  void (*OnMacProcess)(void);
  void (*OnJoinRequest)(LmHandlerJoinParams_t *params);
  void (*OnTxData)(LmHandlerTxParams_t *params);
  void (*OnRxData)(LmHandlerAppData_t *appData, LmHandlerRxParams_t *params);
  void (*OnClassChange)(DeviceClass_t deviceClass);
  void (*OnSysTimeUpdate)(void);
} LmHandlerCallbacks_t;



#define ACTIVATION_TYPE_OTAA (1)

int LmHandlerJoin(ActivationType_t activation_type);

#endif
