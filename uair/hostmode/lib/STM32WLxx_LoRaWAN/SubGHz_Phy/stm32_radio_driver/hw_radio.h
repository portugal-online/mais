#ifndef HW_RADIO_H__
#define HW_RADIO_H__

#include "radio.h"

#ifdef __cplusplus
extern "C" {
#endif


#include <stdint.h>
#include <stdbool.h>

/* Exported constants --------------------------------------------------------*/
#define RFO_LP                                      1
#define RFO_HP                                      2

/*!
 * Generic SUBGRF_ error code
 */
#define SUBGRF_OK 0
#define SUBGRF_ERROR -1

/*!
 * \brief Radio complete Wake-up Time with margin for temperature compensation
 */
#define RADIO_WAKEUP_TIME                           3 // [ms]

/*!
 * \brief Compensation delay for SetAutoTx/Rx functions in 15.625 microseconds
 */
#define AUTO_RX_TX_OFFSET                           2

/*!
 * \brief LFSR initial value to compute IBM type CRC
 */
#define CRC_IBM_SEED                                0xFFFF

/*!
 * \brief LFSR initial value to compute CCIT type CRC
 */
#define CRC_CCITT_SEED                              0x1D0F

/*!
 * \brief Polynomial used to compute IBM CRC
 */
#define CRC_POLYNOMIAL_IBM                          0x8005

/*!
 * \brief Polynomial used to compute CCIT CRC
 */
#define CRC_POLYNOMIAL_CCITT                        0x1021

/*!
 * \brief The address of the register holding the first byte defining the CRC seed
 *
 */
#define REG_LR_CRCSEEDBASEADDR                      0x06BC

/*!
 * \brief The address of the register holding the first byte defining the CRC polynomial
 */
#define REG_LR_CRCPOLYBASEADDR                      0x06BE

/*!
 * \brief The address of the register holding the first byte defining the whitening seed
 */
#define REG_LR_WHITSEEDBASEADDR_MSB                 0x06B8
#define REG_LR_WHITSEEDBASEADDR_LSB                 0x06B9

/*!
 * \brief The address of the register holding the packet configuration
 */
#define REG_LR_PACKETPARAMS                         0x0704

/*!
 * \brief The address of the register holding the payload size
 */
#define REG_LR_PAYLOADLENGTH                        0x0702

/*!
 * \brief The address of the register holding the re-calculated number of symbols
 */
#define REG_LR_SYNCH_TIMEOUT                        0x0706

/*!
 * \brief The addresses of the registers holding SyncWords values
 */
#define REG_LR_SYNCWORDBASEADDRESS                  0x06C0

/*!
 * \brief The addresses of the register holding LoRa Modem SyncWord value
 */
#define REG_LR_SYNCWORD                             0x0740

/*!
 * \brief Syncword for Private LoRa networks
 */
#define LORA_MAC_PRIVATE_SYNCWORD                   0x1424

/*!
 * \brief Syncword for Public LoRa networks
 */
#define LORA_MAC_PUBLIC_SYNCWORD                    0x3444

/*!
 * \brief The address of the register giving a 32-bit random number
 */
#define RANDOM_NUMBER_GENERATORBASEADDR             0x0819

/*!
 * \brief The address of the register used to disable the LNA
 */
#define REG_ANA_LNA                                 0x08E2

/*!
 * The address of the register used to disable the mixer
 */
#define REG_ANA_MIXER                               0x08E5

/*!
 * \brief The address of the register holding RX Gain value (0x94: power saving, 0x96: rx boosted)
 */
#define REG_RX_GAIN                                 0x08AC

/*!
 * \brief The address of the register holding Bit Sync configuration
 */
#define REG_BIT_SYNC                                0x06AC

/*!
 * \brief Change the value on the device internal trimming capacitor
 */
#define REG_XTA_TRIM                                0x0911

/*!
 * \brief Change the value on the device internal trimming capacitor
 */
#define REG_XTB_TRIM                                0x0912

/*!
 * \brief Set the current max value in the over current protection
 */
#define REG_OCP                                     0x08E7

/*!
 * \brief PA Clamping threshold
 */
#define REG_TX_CLAMP                                0x08D8

/**
  * @brief  Sub-GHz radio register (re) definition
  * @note   The sub-GHz radio peripheral registers can be accessed by sub-GHz radio command
  *         SUBGRF_WriteRegisters() and SUBGRF_ReadRegisters() "
  */
/*Sub-GHz radio generic bit synchronization register*/
#define SUBGHZ_GBSYNCR                              REG_BIT_SYNC
/*Sub-GHz radio generic packet control 1A register*/
#define SUBGHZ_GPKTCTL1AR                           REG_LR_WHITSEEDBASEADDR_MSB
/*Sub-GHz radio generic whitening LSB register*/
#define SUBGHZ_GWHITEINIRL                          REG_LR_WHITSEEDBASEADDR_LSB  
/*Sub-GHz radio generic CRC initial MSB register*/
#define SUBGHZ_GCRCINIRH                            REG_LR_CRCSEEDBASEADDR
/*Sub-GHz radio generic CRC initial LSB register*/
#define SUBGHZ_GCRCINIRL                            0x06BD
/*Sub-GHz radio generic CRC polynomial MSB register*/
#define SUBGHZ_GCRCPOLRH                            REG_LR_CRCPOLYBASEADDR
/*Sub-GHz radio generic CRC polynomial LSB register*/
#define SUBGHZ_GCRCPOLRL                            0x06BF
/*Sub-GHz radio generic synchronization word control register 7*/
#define SUBGHZ_GSYNCR7                              REG_LR_SYNCWORDBASEADDRESS
/*Sub-GHz radio generic synchronization word control register 6*/
#define SUBGHZ_GSYNCR6                              0x06C1
/*Sub-GHz radio generic synchronization word control register 5*/
#define SUBGHZ_GSYNCR5                              0x06C2 
/*Sub-GHz radio generic synchronization word control register 4*/
#define SUBGHZ_GSYNCR4                              0x06C3
/*Sub-GHz radio generic synchronization word control register 3*/
#define SUBGHZ_GSYNCR3                              0x06C4
/*Sub-GHz radio generic synchronization word control register 2*/
#define SUBGHZ_GSYNCR2                              0x06C5
/*Sub-GHz radio generic synchronization word control register 1*/
#define SUBGHZ_GSYNCR1                              0x06C6 
/*Sub-GHz radio generic synchronization word control register 0*/
#define SUBGHZ_GSYNCR0                              0x06C7
/*Sub-GHz radio LoRa synchronization word MSB register*/
#define SUBGHZ_LSYNCRH                              REG_LR_SYNCWORD
/*Sub-GHz radio LoRa synchronization word LSB register*/
#define SUBGHZ_LSYNCRL                              0x0741
/*Sub-GHz radio random number register 3*/
#define SUBGHZ_RNGR3                                RANDOM_NUMBER_GENERATORBASEADDR
/*Sub-GHz radio  random number register 2*/
#define SUBGHZ_RNGR2                                0x081A
/*Sub-GHz radio  random number register 1*/
#define SUBGHZ_RNGR1                                0x081B
/*Sub-GHz radio  random number register 0*/
#define SUBGHZ_RNGR0                                0x081C
/*Sub-GHz radio receiver gain control register*/
#define SUBGHZ_RXGAINCR                             REG_RX_GAIN
/*Sub-GHz radio PA over current protection register*/
#define SUBGHZ_PAOCPR                               REG_OCP
/*Sub-GHz radio HSE32 OSC_IN capacitor trim register*/
#define SUBGHZ_HSEINTRIMR                           REG_XTA_TRIM 
/*Sub-GHz radio HSE32 OSC_OUT capacitor trim register*/
#define SUBGHZ_HSEOUTTRIMR                          REG_XTB_TRIM 
/*Sub-GHz radio SMPS control 0 register */
#define SUBGHZ_SMPSC0R                              0x0916
/*Sub-GHz radio power control register*/
#define SUBGHZ_PCR                                  0x091A
/*Sub-GHz radio SMPS control 2 register */
#define SUBGHZ_SMPSC2R                              0x0923

#define SMPS_CLK_DET_ENABLE ((uint8_t) (1<<6))

#define SMPS_DRV_20  ((uint8_t) ((0x0)<<1))
#define SMPS_DRV_40  ((uint8_t) ((0x1)<<1))
#define SMPS_DRV_60  ((uint8_t) ((0x2)<<1))
#define SMPS_DRV_100 ((uint8_t) ((0x3)<<1))
#define SMPS_DRV_MASK ((uint8_t) ((0x3)<<1))


/* Exported types ------------------------------------------------------------*/
/*!
 * \brief Structure describing the radio status
 */
typedef union RadioStatus_u
{
    uint8_t Value;
    struct
    {   //bit order is lsb -> msb
        uint8_t Reserved  : 1;  //!< Reserved
        uint8_t CmdStatus : 3;  //!< Command status
        uint8_t ChipMode  : 3;  //!< Chip mode
        uint8_t CpuBusy   : 1;  //!< Flag for CPU radio busy
    }Fields;
}RadioStatus_t;

/*!
 * \brief Structure describing the error codes for callback functions
 */
typedef enum
{
    IRQ_HEADER_ERROR_CODE                   = 0x01,
    IRQ_SYNCWORD_ERROR_CODE                 = 0x02,
    IRQ_CRC_ERROR_CODE                      = 0x04,
}IrqErrorCode_t;

enum IrqPblSyncHeaderCode_t
{
    IRQ_PBL_DETECT_CODE                     = 0x01,
    IRQ_SYNCWORD_VALID_CODE                 = 0x02,
    IRQ_HEADER_VALID_CODE                   = 0x04,
};



/*!
 * \brief Declares the oscillator in use while in standby mode
 *
 * Using the STDBY_RC standby mode allow to reduce the energy consumption
 * STDBY_XOSC should be used for time critical applications
 */
typedef enum
{
    STDBY_RC                                = 0x00,
    STDBY_XOSC                              = 0x01,
}RadioStandbyModes_t;

/*!
 * \brief Declares the power regulation used to power the device
 *
 * This command allows the user to specify if DC-DC or LDO is used for power regulation.
 * Using only LDO implies that the Rx or Tx current is doubled
 */
typedef enum
{
    USE_LDO                                 = 0x00, // default
    USE_DCDC                                = 0x01,
}RadioRegulatorMode_t;

/*!
 * \brief Represents the possible packet type (i.e. modem) used
 */
typedef enum
{
    PACKET_TYPE_GFSK                        = 0x00,
    PACKET_TYPE_LORA                        = 0x01,
    PACKET_TYPE_BPSK                        = 0x02,
    PACKET_TYPE_GMSK                        = 0x03,
    PACKET_TYPE_NONE                        = 0x0F,
}RadioPacketTypes_t;

/*!
 * \brief Represents the ramping time for power amplifier
 */
typedef enum
{
    RADIO_RAMP_10_US                        = 0x00,
    RADIO_RAMP_20_US                        = 0x01,
    RADIO_RAMP_40_US                        = 0x02,
    RADIO_RAMP_80_US                        = 0x03,
    RADIO_RAMP_200_US                       = 0x04,
    RADIO_RAMP_800_US                       = 0x05,
    RADIO_RAMP_1700_US                      = 0x06,
    RADIO_RAMP_3400_US                      = 0x07,
}RadioRampTimes_t;

/*!
 * \brief Represents the number of symbols to be used for channel activity detection operation
 */
typedef enum
{
    LORA_CAD_01_SYMBOL                      = 0x00,
    LORA_CAD_02_SYMBOL                      = 0x01,
    LORA_CAD_04_SYMBOL                      = 0x02,
    LORA_CAD_08_SYMBOL                      = 0x03,
    LORA_CAD_16_SYMBOL                      = 0x04,
}RadioLoRaCadSymbols_t;

/*!
 * \brief Represents the Channel Activity Detection actions after the CAD operation is finished
 */
typedef enum
{
    LORA_CAD_ONLY                           = 0x00,
    LORA_CAD_RX                             = 0x01,
    LORA_CAD_LBT                            = 0x10,
}RadioCadExitModes_t;

/*!
 * \brief Represents the modulation shaping parameter
 */
typedef enum
{
    MOD_SHAPING_OFF                         = 0x00,
    MOD_SHAPING_G_BT_03                     = 0x08,
    MOD_SHAPING_G_BT_05                     = 0x09,
    MOD_SHAPING_G_BT_07                     = 0x0A,
    MOD_SHAPING_G_BT_1                      = 0x0B,
    MOD_SHAPING_DBPSK                       = 0x16,
}RadioModShapings_t;

/*!
 * \brief Represents the modulation shaping parameter
 */
typedef enum
{
    RX_BW_4800                              = 0x1F,
    RX_BW_5800                              = 0x17,
    RX_BW_7300                              = 0x0F,
    RX_BW_9700                              = 0x1E,
    RX_BW_11700                             = 0x16,
    RX_BW_14600                             = 0x0E,
    RX_BW_19500                             = 0x1D,
    RX_BW_23400                             = 0x15,
    RX_BW_29300                             = 0x0D,
    RX_BW_39000                             = 0x1C,
    RX_BW_46900                             = 0x14,
    RX_BW_58600                             = 0x0C,
    RX_BW_78200                             = 0x1B,
    RX_BW_93800                             = 0x13,
    RX_BW_117300                            = 0x0B,
    RX_BW_156200                            = 0x1A,
    RX_BW_187200                            = 0x12,
    RX_BW_234300                            = 0x0A,
    RX_BW_312000                            = 0x19,
    RX_BW_373600                            = 0x11,
    RX_BW_467000                            = 0x09,
}RadioRxBandwidth_t;

/*!
 * \brief Represents the possible spreading factor values in LoRa packet types
 */
typedef enum
{
    LORA_SF5                                = 0x05,
    LORA_SF6                                = 0x06,
    LORA_SF7                                = 0x07,
    LORA_SF8                                = 0x08,
    LORA_SF9                                = 0x09,
    LORA_SF10                               = 0x0A,
    LORA_SF11                               = 0x0B,
    LORA_SF12                               = 0x0C,
}RadioLoRaSpreadingFactors_t;

/*!
 * \brief Represents the bandwidth values for LoRa packet type
 */
typedef enum
{
    LORA_BW_500                             = 6,
    LORA_BW_250                             = 5,
    LORA_BW_125                             = 4,
    LORA_BW_062                             = 3,
    LORA_BW_041                             = 10,
    LORA_BW_031                             = 2,
    LORA_BW_020                             = 9,
    LORA_BW_015                             = 1,
    LORA_BW_010                             = 8,
    LORA_BW_007                             = 0,
}RadioLoRaBandwidths_t;

/*!
 * \brief Represents the coding rate values for LoRa packet type
 */
typedef enum
{
    LORA_CR_4_5                             = 0x01,
    LORA_CR_4_6                             = 0x02,
    LORA_CR_4_7                             = 0x03,
    LORA_CR_4_8                             = 0x04,
}RadioLoRaCodingRates_t;

/*!
 * \brief Represents the preamble length used to detect the packet on Rx side
 */
typedef enum
{
    RADIO_PREAMBLE_DETECTOR_OFF             = 0x00,         //!< Preamble detection length off
    RADIO_PREAMBLE_DETECTOR_08_BITS         = 0x04,         //!< Preamble detection length 8 bits
    RADIO_PREAMBLE_DETECTOR_16_BITS         = 0x05,         //!< Preamble detection length 16 bits
    RADIO_PREAMBLE_DETECTOR_24_BITS         = 0x06,         //!< Preamble detection length 24 bits
    RADIO_PREAMBLE_DETECTOR_32_BITS         = 0x07,         //!< Preamble detection length 32 bit
}RadioPreambleDetection_t;

/*!
 * \brief Represents the possible combinations of SyncWord correlators activated
 */
typedef enum
{
    RADIO_ADDRESSCOMP_FILT_OFF              = 0x00,         //!< No correlator turned on, i.e. do not search for SyncWord
    RADIO_ADDRESSCOMP_FILT_NODE             = 0x01,
    RADIO_ADDRESSCOMP_FILT_NODE_BROAD       = 0x02,
}RadioAddressComp_t;

/*!
 *  \brief Radio GFSK packet length mode
 */
typedef enum
{
    RADIO_PACKET_FIXED_LENGTH               = 0x00,         //!< The packet is known on both sides, no header included in the packet
    RADIO_PACKET_VARIABLE_LENGTH            = 0x01,         //!< The packet is on variable size, header included
}RadioPacketLengthModes_t;

/*!
 * \brief Represents the CRC length
 */
typedef enum
{
    RADIO_CRC_OFF                           = 0x01,         //!< No CRC in use
    RADIO_CRC_1_BYTES                       = 0x00,
    RADIO_CRC_2_BYTES                       = 0x02,
    RADIO_CRC_1_BYTES_INV                   = 0x04,
    RADIO_CRC_2_BYTES_INV                   = 0x06,
    RADIO_CRC_2_BYTES_IBM                   = 0xF1,
    RADIO_CRC_2_BYTES_CCIT                  = 0xF2,
}RadioCrcTypes_t;

/*!
 * \brief Radio whitening mode activated or deactivated
 */
typedef enum
{
    RADIO_DC_FREE_OFF                       = 0x00,
    RADIO_DC_FREEWHITENING                  = 0x01,
}RadioDcFree_t;

/*!
 * \brief Holds the Radio lengths mode for the LoRa packet type
 */
typedef enum
{
    LORA_PACKET_VARIABLE_LENGTH             = 0x00,         //!< The packet is on variable size, header included
    LORA_PACKET_FIXED_LENGTH                = 0x01,         //!< The packet is known on both sides, no header included in the packet
    LORA_PACKET_EXPLICIT                    = LORA_PACKET_VARIABLE_LENGTH,
    LORA_PACKET_IMPLICIT                    = LORA_PACKET_FIXED_LENGTH,
}RadioLoRaPacketLengthsMode_t;

/*!
 * \brief Represents the CRC mode for LoRa packet type
 */
typedef enum
{
    LORA_CRC_ON                             = 0x01,         //!< CRC activated
    LORA_CRC_OFF                            = 0x00,         //!< CRC not used
}RadioLoRaCrcModes_t;

/*!
 * \brief Represents the IQ mode for LoRa packet type
 */
typedef enum
{
    LORA_IQ_NORMAL                          = 0x00,
    LORA_IQ_INVERTED                        = 0x01,
}RadioLoRaIQModes_t;

/*!
 * \brief Represents the voltage used to control the TCXO on/off VDD_TCXO
 */
typedef enum
{
    TCXO_CTRL_1_6V                          = 0x00,
    TCXO_CTRL_1_7V                          = 0x01,
    TCXO_CTRL_1_8V                          = 0x02,
    TCXO_CTRL_2_2V                          = 0x03,
    TCXO_CTRL_2_4V                          = 0x04,
    TCXO_CTRL_2_7V                          = 0x05,
    TCXO_CTRL_3_0V                          = 0x06,
    TCXO_CTRL_3_3V                          = 0x07,
}RadioTcxoCtrlVoltage_t;

/*!
 * \brief Represents the interruption masks available for the radio
 *
 * \remark Note that not all these interruptions are available for all packet types
 */
typedef enum
{
    IRQ_RADIO_NONE                          = 0x0000,
    IRQ_TX_DONE                             = 0x0001,
    IRQ_RX_DONE                             = 0x0002,
    IRQ_PREAMBLE_DETECTED                   = 0x0004,
    IRQ_SYNCWORD_VALID                      = 0x0008,
    IRQ_HEADER_VALID                        = 0x0010,
    IRQ_HEADER_ERROR                        = 0x0020,
    IRQ_CRC_ERROR                           = 0x0040,
    IRQ_CAD_CLEAR                           = 0x0080,
    IRQ_CAD_DETECTED                        = 0x0100,
    IRQ_RX_TX_TIMEOUT                       = 0x0200,
    IRQ_RADIO_ALL                           = 0xFFFF,
}RadioIrqMasks_t;



/*!
 * \brief The type describing the modulation parameters for every packet types
 */
typedef struct
{
    RadioPacketTypes_t                   PacketType;        //!< Packet to which the modulation parameters are referring to.
    struct
    {
        struct
        {
            uint32_t                     BitRate;
            uint32_t                     Fdev;
            RadioModShapings_t           ModulationShaping;
            uint8_t                      Bandwidth;
        }Gfsk;
        struct
        {
            uint32_t                     BitRate;
            RadioModShapings_t           ModulationShaping;
        }Bpsk;
        struct
        {
            RadioLoRaSpreadingFactors_t  SpreadingFactor;   //!< Spreading Factor for the LoRa modulation
            RadioLoRaBandwidths_t        Bandwidth;         //!< Bandwidth for the LoRa modulation
            RadioLoRaCodingRates_t       CodingRate;        //!< Coding rate for the LoRa modulation
            uint8_t                      LowDatarateOptimize; //!< Indicates if the modem uses the low datarate optimization
        }LoRa;
    }Params;                                                //!< Holds the modulation parameters structure
}ModulationParams_t;

/*!
 * \brief The type describing the packet parameters for every packet types
 */
typedef struct
{
    RadioPacketTypes_t                    PacketType;        //!< Packet to which the packet parameters are referring to.
    struct
    {
        /*!
         * \brief Holds the GFSK packet parameters
         */
        struct
        {
            uint16_t                     PreambleLength;    //!< The preamble Tx length for GFSK packet type in bit
            RadioPreambleDetection_t     PreambleMinDetect; //!< The preamble Rx length minimal for GFSK packet type
            uint8_t                      SyncWordLength;    //!< The synchronization word length for GFSK packet type
            RadioAddressComp_t           AddrComp;          //!< Activated SyncWord correlators
            RadioPacketLengthModes_t     HeaderType;        //!< If the header is explicit, it will be transmitted in the GFSK packet. If the header is implicit, it will not be transmitted
            uint8_t                      PayloadLength;     //!< Size of the payload in the GFSK packet
            RadioCrcTypes_t              CrcLength;         //!< Size of the CRC block in the GFSK packet
            RadioDcFree_t                DcFree;
        }Gfsk;
        /*!
         * \brief Holds the BPSK packet parameters
         */
        struct
        {
            uint8_t                      PayloadLength;     //!< Size of the payload in the BPSK packet
        }Bpsk;
        /*!
         * \brief Holds the LoRa packet parameters
         */
        struct
        {
            uint16_t                     PreambleLength;    //!< The preamble length is the number of LoRa symbols in the preamble
            RadioLoRaPacketLengthsMode_t HeaderType;        //!< If the header is explicit, it will be transmitted in the LoRa packet. If the header is implicit, it will not be transmitted
            uint8_t                      PayloadLength;     //!< Size of the payload in the LoRa packet
            RadioLoRaCrcModes_t          CrcMode;           //!< Size of CRC block in LoRa packet
            RadioLoRaIQModes_t           InvertIQ;          //!< Allows to swap IQ for LoRa packet
        }LoRa;
    }Params;                                                //!< Holds the packet parameters structure
}PacketParams_t;

/*!
 * \brief Represents the packet status for every packet type
 */
typedef struct
{
    RadioPacketTypes_t                    packetType;      //!< Packet to which the packet status are referring to.
    struct
    {
        struct
        {
            uint8_t RxStatus;
            int8_t RssiAvg;                                //!< The averaged RSSI
            int8_t RssiSync;                               //!< The RSSI measured on last packet
            uint32_t FreqError;
        }Gfsk;
        struct
        {
            int8_t RssiPkt;                                //!< The RSSI of the last packet
            int8_t SnrPkt;                                 //!< The SNR of the last packet
            int8_t SignalRssiPkt;
            uint32_t FreqError;
        }LoRa;
    }Params;
}PacketStatus_t;

/*!
 * \brief Represents the Rx internal counters values when GFSK or LoRa packet type is used
 */
typedef struct
{
    RadioPacketTypes_t                    packetType;       //!< Packet to which the packet status are referring to.
    uint16_t PacketReceived;
    uint16_t CrcOk;
    uint16_t LengthError;
}RxCounter_t;

/*!
 * \brief Represents a calibration configuration
 */
typedef union
{
    struct
    {
        uint8_t RC64KEnable    : 1;                             //!< Calibrate RC64K clock
        uint8_t RC13MEnable    : 1;                             //!< Calibrate RC13M clock
        uint8_t PLLEnable      : 1;                             //!< Calibrate PLL
        uint8_t ADCPulseEnable : 1;                             //!< Calibrate ADC Pulse
        uint8_t ADCBulkNEnable : 1;                             //!< Calibrate ADC bulkN
        uint8_t ADCBulkPEnable : 1;                             //!< Calibrate ADC bulkP
        uint8_t ImgEnable      : 1;
        uint8_t                : 1;
    }Fields;
    uint8_t Value;
}CalibrationParams_t;

/*!
 * \brief Represents a sleep mode configuration
 */
typedef union
{
    struct
    {
        uint8_t WakeUpRTC               : 1;                    //!< Get out of sleep mode if wakeup signal received from RTC
        uint8_t Reset                   : 1;
        uint8_t WarmStart               : 1;
        uint8_t Reserved                : 5;
    }Fields;
    uint8_t Value;
}SleepParams_t;

/*!
 * \brief Represents the possible radio system error states
 */
typedef union
{
    struct
    {
        uint8_t Rc64kCalib              : 1;                    //!< RC 64kHz oscillator calibration failed
        uint8_t Rc13mCalib              : 1;                    //!< RC 13MHz oscillator calibration failed
        uint8_t PllCalib                : 1;                    //!< PLL calibration failed
        uint8_t AdcCalib                : 1;                    //!< ADC calibration failed
        uint8_t ImgCalib                : 1;                    //!< Image calibration failed
        uint8_t XoscStart               : 1;                    //!< XOSC oscillator failed to start
        uint8_t PllLock                 : 1;                    //!< PLL lock failed
        uint8_t BuckStart               : 1;                    //!< Buck converter failed to start
        uint8_t PaRamp                  : 1;                    //!< PA ramp failed
        uint8_t                         : 7;                    //!< Reserved
    }Fields;
    uint16_t Value;
}RadioError_t;

/*!
 * \brief Represents the operating mode the radio is actually running
 */
typedef enum
{
    MODE_SLEEP                              = 0x00,         //! The radio is in sleep mode
    MODE_STDBY_RC,                                          //! The radio is in standby mode with RC oscillator
    MODE_STDBY_XOSC,                                        //! The radio is in standby mode with XOSC oscillator
    MODE_FS,                                                //! The radio is in frequency synthesis mode
    MODE_TX,                                                //! The radio is in transmit mode
    MODE_RX,                                                //! The radio is in receive mode
    MODE_RX_DC,                                             //! The radio is in receive duty cycle mode
    MODE_CAD                                                //! The radio is in channel activity detection mode
}RadioOperatingModes_t;

void hw_radio_init( RadioEvents_t *events );
void hw_radio_deinit();
RadioState_t hw_radio_get_status( void );
void hw_radio_set_modem( RadioModems_t modem );
void hw_radio_set_channel( uint32_t freq );
bool hw_radio_is_channel_free( uint32_t freq, uint32_t rxBandwidth, int16_t rssiThresh, uint32_t maxCarrierSenseTime );
uint32_t hw_radio_random( void );
void  hw_radio_set_rx_config( RadioModems_t modem, uint32_t bandwidth,
                             uint32_t datarate, uint8_t coderate,
                             uint32_t bandwidthAfc, uint16_t preambleLen,
                             uint16_t symbTimeout, bool fixLen,
                             uint8_t payloadLen,
                             bool crcOn, bool freqHopOn, uint8_t hopPeriod,
                             bool iqInverted, bool rxContinuous );

void hw_radio_set_tx_config( RadioModems_t modem, int8_t power, uint32_t fdev,
                            uint32_t bandwidth, uint32_t datarate,
                            uint8_t coderate, uint16_t preambleLen,
                            bool fixLen, bool crcOn, bool freqHopOn,
                            uint8_t hopPeriod, bool iqInverted, uint32_t timeout );

bool hw_radio_check_rf_frequency( uint32_t frequency );

uint32_t hw_radio_time_on_air( RadioModems_t modem, uint32_t bandwidth,
                              uint32_t datarate, uint8_t coderate,
                              uint16_t preambleLen, bool fixLen, uint8_t payloadLen,
                              bool crcOn );
void hw_radio_send ( uint8_t *buffer, uint8_t size );
void hw_radio_sleep ( void );
void hw_radio_standby ( void );
void hw_radio_rx ( uint32_t timeout );
void hw_radio_start_cad ( void );
void hw_radio_set_tx_continuous_wave ( uint32_t freq, int8_t power, uint16_t time );
int16_t hw_radio_rssi ( RadioModems_t modem );
void hw_radio_write ( uint16_t addr, uint8_t data );
uint8_t hw_radio_read ( uint16_t addr );
void hw_radio_write_registers ( uint16_t addr, uint8_t *buffer, uint8_t size );
void hw_radio_read_registers ( uint16_t addr, uint8_t *buffer, uint8_t size );
void hw_radio_set_max_payload_length ( RadioModems_t modem, uint8_t max );
void hw_radio_set_public_network ( bool enable );
uint32_t  hw_radio_get_wakeup_time ( void );
void hw_radio_irq_process ( void );
void hw_radio_set_event_notify ( void ( * notify ) ( void ) );
void hw_radio_rx_boosted ( uint32_t timeout );
void hw_radio_set_rx_duty_cycle( uint32_t rxTime, uint32_t sleepTime );
void hw_radio_tx_prbs( void );
void hw_radio_tx_cw( int8_t power );
int32_t hw_radio_radio_set_rx_generic_config( GenericModems_t modem, RxConfigGeneric_t* config, uint32_t rxContinuous, uint32_t symbTimeout);
int32_t hw_radio_radio_set_tx_generic_config( GenericModems_t modem, TxConfigGeneric_t* config, int8_t power, uint32_t timeout );

#ifdef __cplusplus
}
#endif

#endif
