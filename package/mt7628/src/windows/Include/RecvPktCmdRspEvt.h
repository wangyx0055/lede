/*
    ***************************************************************************
    * Ralink Technology, Corp.
    * 5F., No. 5, Tai-Yuan 1st St., Jhubei City, Hsinchu County 30265, Taiwan
    *
    * (c) Copyright 2002-2011, Ralink Technology, Corp.
    *
    * All rights reserved. Ralink's source code is an unpublished work and the use of a copyright notice 
    * does not imply otherwise. This source code contains confidential trade secret material of 
    * Ralink Technology, Corp. Any attempt or participation in deciphering, decoding, reverse engineering 
    * or in any way altering the source code is stricitly prohibited, unless the prior written consent of 
    * Ralink Technology, Corp. is obtained.
    ***************************************************************************

    Module Name: 
    PktCmdRspEvt.h

    Abstract: 
    Andes packet commands and response events

    Revision History:
    Who                         When            What
    -------------------     ----------      ----------------------------------------------
    Ian Tang                        2011/11/7       Start implementation
*/

#ifndef __RECVPKTCMDANDRSPEVT_H__
#define __RECVPKTCMDANDRSPEVT_H__

#pragma pack(push, struct_pack1)
#pragma pack(1)

extern PMP_ADAPTER    G_pAdapter;
typedef struct _EXT_CMD_CHAN_SWITCH_T  EXT_CMD_CHAN_SWITCH_T, *PEXT_CMD_CHAN_SWITCH_T;
//
// Queue ID for packet command
//
#define QUEUE_IDX_PKT_CMD                       (QID_AC_BE)

//
// Valid queue ID
//
//2 TODO: Add new queue index
//
#define VALID_QUEUE_IDX(QueueIdx) \
    ((QueueIdx == QID_AC_BE) || (QueueIdx == QID_AC_BK) || \
    (QueueIdx == QID_AC_VI) || (QueueIdx == QID_AC_VO))

//
// Information type (INFO_TYPE_XXX)
//
#define INFO_TYPE_DOT11_PKT     (0)
#define INFO_TYPE_PKT_CMD       (1)
#define INFO_TYPE_RSP_EVT       (1)

//
// Response event type (RSP_EVT_TYPE_XXX)
//
#define RSP_EVT_TYPE_DONE       (0)
#define RSP_EVT_TYPE_RETRY      (1)
#define RSP_EVT_TYPE_ERROR      (2)

//
// Command types (PCK_CMD_TYPE_XXX)
//
#define PKT_CMD_TYPE_BURST_WRITE        (8)
#define PKT_CMD_TYPE_READ_MODIFY_WRITE  (9)
#define PKT_CMD_TYPE_RANDOM_READ        (10)
#define PKT_CMD_TYPE_BURST_READ         (11)
//legacy calibration 12 ~13
//new rf calibration 14 ~15
#define PKT_CMD_TYPE_LED_MODE           (16)
//reserved 17~19
#define PKT_CMD_TYPE_PWR_SAVE           (20)
#define PKT_CMD_TYPE_WOW_ENTRY          (21)
#define PKT_CMD_TYPE_NLO                (22)
#define PKT_CMD_TYPE_SCRIPT             (23)
#define PKT_CMD_TYPE_FEATURE            (24)
#define PKT_CMD_TYPE_QUERY              (25)
#define PKT_CMD_TYPE_D0_OFFLOAD         (26)
//reserved 27
#define PKT_CMD_TYPE_CARRIER_DETECT     (28)
#define PKT_CMD_TYPE_RADAR_DETECT       (29)
#define PKT_CMD_TYPE_SWITCH_CHANNEL     (30)
#define PKT_CMD_TYPE_CALIBRATION        (31)
//reserved 32
#define PKT_CMD_TYPE_BEACON_HANDLING    (33)
#define PKT_CMD_TYPE_ANTENNA_HANDLING   (34)

//Power Operation
#define PWR_PmOn                                0x1
#define PWR_PmOff                                   0x2
#define PWR_RadioOn                             0x1
#define PWR_RadioOff                                    0x2
#define PWR_RadioOnThenSwitchChannel            
#define PWR_RadioOffAutoWakeup                  0x32
#define PWR_RadioOffAdvance                     0x33
#define PWR_RadioOnAdvance                      0x34

//Power Level
#define AndesNoPowerSaving          0
#define AndesPowerSavingLevel1      1
#define AndesPowerSavingLevel2      2
#define AndesPowerSavingLevel3      3
#define AndesPowerSavingLevel4      4
#define AndesPowerSavingLevel5      5
//
// Valid packet command types
//
//2 TODO: Add new packet command type
//
#define VALID_PKT_CMD_TYPE(PktCmdType) \
    ((PktCmdType == PKT_CMD_TYPE_BURST_WRITE) || \
    (PktCmdType == PKT_CMD_TYPE_READ_MODIFY_WRITE) || \
    (PktCmdType == PKT_CMD_TYPE_RANDOM_READ) || \
    (PktCmdType == PKT_CMD_TYPE_BURST_READ) || \
    (PktCmdType == PKT_CMD_TYPE_LED_MODE) || \
    (PktCmdType == PKT_CMD_TYPE_PWR_SAVE) || \
    (PktCmdType == PKT_CMD_TYPE_WOW_ENTRY) || \
    (PktCmdType == PKT_CMD_TYPE_QUERY) || \
    (PktCmdType == PKT_CMD_TYPE_CARRIER_DETECT) || \
    (PktCmdType == PKT_CMD_TYPE_RADAR_DETECT) || \
    (PktCmdType == PKT_CMD_TYPE_SWITCH_CHANNEL) || \
    (PktCmdType == PKT_CMD_TYPE_CALIBRATION) || \
    (PktCmdType == PKT_CMD_TYPE_D0_OFFLOAD) || \
    (PktCmdType == PKT_CMD_TYPE_NLO) || \
    (PktCmdType == PKT_CMD_TYPE_BEACON_HANDLING) || \
    (PktCmdType == PKT_CMD_TYPE_ANTENNA_HANDLING))

//
// No need the response event of the packet command
//
#define NO_PKT_CMD_RSP_EVENT            (0)

//
// Since response event did not have CMD ID, switch channel use command sequence 1 to recongenize the response packet.
//
#define PKT_CMD_RSP_SWITCH_CH           (1)

//
// Minimum/maximum packet commnad sequence
//
#define MIN_PKT_CMD_SEQ                 (1)
#define MAX_PKT_CMD_SEQ                 (15)

//
// Valid packet command sequence
//
#define VALID_PKT_CMD_SEQ(PktCmdSeq) \
    ((PktCmdSeq == NO_PKT_CMD_RSP_EVENT) || \
    ( (PktCmdSeq >= MIN_PKT_CMD_SEQ) && (PktCmdSeq <= MAX_PKT_CMD_SEQ)))

//
// Event types (RSP_EVT_TYPE_XXX)
//
#define RSP_EVT_TYPE_CMD_DONE                   (0)
#define RSP_EVT_TYPE_CMD_ERROR                  (1)
#define RSP_EVT_TYPE_CMD_RETRY                  (2)
#define RSP_EVT_TYPE_PWR_RSP_EVENT              (4)
#define RSP_EVT_TYPE_WOW_RSP_EVENT              (5)
#define RSP_EVT_TYPE_CARRIER_DETECT_RSP_EVENT   (6)
#define RSP_EVT_TYPE_DFS_DETECT_RSP_EVENT       (7)
#define RSP_EVT_TYPE_HW_MAC_STATISTIC_RETURN    (15)

//
// WoW packet detection mask (WOW_PKT_DETECT_MASK_XXX) - packet type
//
#define WOW_PKT_DETECT_MASK_MAGIC_PKT                   (0x00000001)
#define WOW_PKT_DETECT_MASK_BITMAP_PATTERN              (0x00000002)
#define WOW_PKT_DETECT_MASK_IPV4_TCP_SYN                (0x00000004)
#define WOW_PKT_DETECT_MASK_IPV6_TCP_SYN                (0x00000008)
#define WOW_PKT_DETECT_MASK_EAPOL_IDENTIFY_RQST_PKT     (0x00000010)
#define WOW_PKT_DETECT_MASK_WILDCARD_IPV4_DEST_ADDR_PKT (0x00000020)
#define WOW_PKT_DETECT_MASK_WILDCARD_IPV6_DEST_ADDR_PKT (0x00000040)
#define WOW_PKT_DETECT_MASK_WFD_INVITATION_RST          (0x00000080)

//
// Valid WoW packet detection mask
//
//2 TODO: Add new WoW packet detection mask
//
#define VALID_WOW_PKT_DETECT_MASK(WoWPktDetectMask) \
    ((WoWPktDetectMask != 0) && \
    ((WoWPktDetectMask == WOW_PKT_DETECT_MASK_MAGIC_PKT) || \
    (WoWPktDetectMask == WOW_PKT_DETECT_MASK_BITMAP_PATTERN) || \
    (WoWPktDetectMask == WOW_PKT_DETECT_MASK_IPV4_TCP_SYN) || \
    (WoWPktDetectMask == WOW_PKT_DETECT_MASK_IPV6_TCP_SYN) || \
    (WoWPktDetectMask == WOW_PKT_DETECT_MASK_EAPOL_IDENTIFY_RQST_PKT) || \
    (WoWPktDetectMask == WOW_PKT_DETECT_MASK_WILDCARD_IPV4_DEST_ADDR_PKT) || \
    (WoWPktDetectMask == WOW_PKT_DETECT_MASK_WILDCARD_IPV6_DEST_ADDR_PKT) || \
    (WoWPktDetectMask == WOW_PKT_DETECT_MASK_WFD_INVITATION_RST)))

//
// WoW packet event mask (WOW_PKT_EVT_MASK_XXX) - media status and protocol status
//
#define WOW_PKT_EVT_MASK_WAKE_ON_MEDIA_CONNECT          (0x00000001)
#define WOW_PKT_EVT_MASK_WAKE_ON_MEDIA_DISCONNECT       (0x00000002)
#define WOW_PKT_EVT_MASK_WAKE_ON_NLO_DISCOVERY          (0x00000004)
#define WOW_PKT_EVT_MASK_WAKE_ON_AP_ASSOCIATION_LOST    (0x00000008)
#define WOW_PKT_EVT_MASK_WAKE_ON_GTK_HANDSHAKE_ERROR    (0x00000010)
#define WOW_PKT_EVT_MASK_WAKE_ON_4WAY_HANDSHAKE_RQST    (0x00000020)

//
// Calibration ID (CMD_TYPE=31)
//
#define CALIBRATION_ID_R_CALIBRATION    (1)
#define CALIBRATION_ID_RX_DCOC_CALIBRATION  (2)
#define CALIBRATION_ID_LC_CALIBRATION   (3)
#define CALIBRATION_ID_LOFT_CALIBRATION (4)
#define CALIBRATION_ID_TX_IQ_CALIBRATION    (5)
#define CALIBRATION_ID_BW_CALIBRATION   (6)
#define CALIBRATION_ID_DPD_CALIBRATION  (7)
#define CALIBRATION_ID_RX_IQ_CALIBRATION    (8)
#define CALIBRATION_ID_TX_DCOC_CALIBRATION  (9)
#define CALIBRATION_ID_RX_GROUP_DELAY       (10)
#define CALIBRATION_ID_TX_GROUP_DELAY       (11)
#define CAL_ID_FULL_CAL                     (0xFF)


//
// Full calibration band (FULL_CAL_BAND_XXX)
//
#define FULL_CAL_BAND_2G (0) // 2G
#define FULL_CAL_BAND_5G_EXTERNAL_PA (1) // 5G + external PA
#define FULL_CAL_BAND_5G_INTERNAL_PA (2) // 5G +  internal PA

//
// Full calibration mode (FULL_CAL_MODE_XXX)
//
#define FULL_CAL_MODE_FULL_CAL (0) // Full calibration (invalid)
#define FULL_CAL_MODE_PARTIAL_CAL (1) // Partial calibration (invalid)
#define FULL_CAL_MODE_G_BAND_FULL_CAL_SAVE (2) // G band full calibration and save the results
#define FULL_CAL_MODE_A_BAND_LOW_FULL_CAL_SAVE (3) // A band (low) full calibration and save the results
#define FULL_CAL_MODE_A_BAND_MID_FULL_CAL_SAVE (4) // A band (mid) full calibration and save the results
#define FULL_CAL_MODE_A_BAND_HIGH_FULL_CAL_SAVE (5) // A band (high) full calibration and save the results
#define FULL_CAL_MODE_G_BAND_RESTORE_CAL (6) // Restore the G band calibration results
#define FULL_CAL_MODE_A_BAND_LOW_RESTORE_CAL (7) // Restore the A band (low) calibration results
#define FULL_CAL_MODE_A_BAND_MID_RESTORE_CAL (8) // Restore the A band (mid) calibration results
#define FULL_CAL_MODE_A_BAND_HIGH_RESTORE_CAL (9) // Restore the A band (high) calibration results

//
// Full calibration
//
typedef struct _FULL_CALIBRATION_CMD_PARAMETERS
{
    ULONG CalibrationID; // Calibration ID
    ULONG CalibrationBand:8; // Calibration band (FULL_CAL_BAND_XXX)
    ULONG CalibrationMode:8; // Calibration mode (FULL_CAL_MODE_XXX)
    ULONG Chanel:8; // Channel (2G only)
    ULONG BW:8; // Bandwidth (BW_XXX, 2G only)
} FULL_CALIBRATION_CMD_PARAMETERS, *PFULL_CALIBRATION_CMD_PARAMETERS;


//
// Valid WoW packet event mask
//
//2 TODO: Add new WoW packet event mask
//
#define VALID_WOW_PKT_EVT_MASK(WoWPktEvtMask) \
    ((WoWPktEvtMask != 0) && \
    ((WoWPktEvtMask == WOW_PKT_EVT_MASK_WAKE_ON_MEDIA_CONNECT) || \
    (WoWPktEvtMask == WOW_PKT_EVT_MASK_WAKE_ON_MEDIA_DISCONNECT) || \
    (WoWPktEvtMask == WOW_PKT_EVT_MASK_WAKE_ON_NLO_DISCOVERY) || \
    (WoWPktEvtMask == WOW_PKT_EVT_MASK_WAKE_ON_AP_ASSOCIATION_LOST) || \
    (WoWPktEvtMask == WOW_PKT_EVT_MASK_WAKE_ON_GTK_HANDSHAKE_ERROR) || \
    (WoWPktEvtMask == WOW_PKT_EVT_MASK_WAKE_ON_4WAY_HANDSHAKE_RQST)))

//
// Number of supported bitmap patterns
// Size, in bytes, of a bitmap patterns
//
#define NUM_OF_BITMAP_PATTERNS  (16)
#define SIZE_OF_A_BITMAP_PATTER (128)

//
// WoW packet command format
//
typedef struct _WOW_PKT_CMD
{
    ULONG FunctionEnabled; // WOW_FUNCTION_XXX
    ULONG PktDetectMask; // WOW_PKT_DETECT_MASK_XXX
    ULONG PktEvtMask; // WOW_PKT_EVT_MASK_XXX
    ULONG Reserved;
    ULONG PwrLevel;
    ULONG InitTbtt;
    ULONG PreTbtt;
    ULONG TimInfo;
    UCHAR BitmapPatterns[NUM_OF_BITMAP_PATTERNS][SIZE_OF_A_BITMAP_PATTER]; // Valid if WOW_PKT_DETECT_MASK_BITMAP_PATTERN is enabled
} WOW_PKT_CMD, *PWOW_PKT_CMD;

//
// Size, in bytes, of the wakeup packet
//
#define SIZE_OF_WAKEUP_PKT  (3 * 512)

//
// WoW response event format
//
typedef struct _WOW_RSP_EVT
{
    ULONG PktDetectMask; // WOW_FUNCTION_XXX
    ULONG PktEvtMask; // WOW_PKT_EVT_MASK_XXX
    UCHAR WakeupPkt[SIZE_OF_WAKEUP_PKT]; // Wakeup packet, including header and data payload
} WOW_RSP_EVT, *PWOW_RSP_EVT;

//
// Burst write packet command parameters
//
typedef struct _BURST_WRITE_PKT_CMD_PARAMETERS
{
    ULONG StartAddress:24; // Start address
    ULONG Reserved:8; // Reserved
    ULONG RegValue; // The register value
} BURST_WRITE_PKT_CMD_PARAMETERS, *PBURST_WRITE_PKT_CMD_PARAMETERS;

//
// Length, in bytes, of a register value
//
#define LENGTH_OF_A_REGISTER_VALUE  (4)

//
// Burst read packet command parameters
//
typedef struct _BURST_READ_PKT_CMD_PARAMETERS
{
    ULONG StartAddress:24; // Start address
    ULONG Reserved:8; // Reserved
    ULONG NumOfRegValues; // Number of registers to be read (Data_Cnt)
} BURST_READ_PKT_CMD_PARAMETERS, *PBURST_READ_PKT_CMD_PARAMETERS;

//
// Andes Power Saving packet command format
//
typedef struct _ANDES_POWER_SAVING_PACKET
{
    ULONG PowerOperation; // Pwr_operation : It defines the power saving operation of this comman
    ULONG PowerLevel; // Pwr_level : It defines the power saving level
    ULONG InitialTBTT; // A specific value for TBTT timer 
    ULONG PreTBTT;//Eefines the lead time of TBTT interrupt
} ANDES_POWER_SAVING_PACKET, *PANDES_POWER_SAVING_PACKET;

typedef struct _SWITCH_CHANNEL_CMD_PARAMETERS
{

    union
    {
        struct
        {
            ULONG Channel:8; // Central Channel
            ULONG bScan:8; // bScan
            ULONG PrimaryChannel:8; // Primary Channel
            ULONG BBPCurrentBW:8; // Current Band Width
        };
        ULONG Word;
    }CSW1;

    union
    {
        struct
        {
            ULONG Power:8; // Tx Power of scanning channel
            ULONG PAType:8; // PA type of Chip
            ULONG PhyMode:8; // Phy Mode
            ULONG CurrentTemperatureState:8; // Current Temperature State
        };
        ULONG Word;
    }CSW2;

    union
    {
        struct
        {
            ULONG LNAGain:8; // Current channel/band's LNA gain
            ULONG ALNAGain0:8; // A external LNA#0 value for ch 36~64 (Linux only)
            ULONG ALNAGain1:8; // A external LNA#1 value for ch 100~128 (Linux only)
            ULONG ALNAGain2:8; // A external LNA#2 value for ch 132~165 (Linux only)
        };
        ULONG Word;
    }CSW3;

    union
    {
        struct
        {
            ULONG FreqOffsetSource:8; // AdativeFreqOffset or RfFreqOffset
            ULONG bJapanFilter:1; // Japan Filter Flag
            ULONG bEnableVhtRxAgcVgaTuning:1; //AGC VGA Tuning Flag
            ULONG bDonePowerOnFullCalibration:1; // Full Power ON Calibration Flag
            ULONG bInternalTxALC:1; // Internal Tx ALC Flag
            ULONG Reserved:20;
        };
        ULONG Word;
    }CSW4;

}SWITCH_CHANNEL_CMD_PARAMETERS, *PSWITCH_CHANNEL_CMD_PARAMETERS;

#pragma pack(pop, struct_pack1)

//
// Load Andes firmware
//
NDIS_STATUS LoadAndesFirmware(
    IN PMP_ADAPTER pAd);

//
// Transmit the packet command
//
VOID TxPktCmd(
    IN PMP_ADAPTER pAd, 
    IN UCHAR PktCmdType, // Packet command type (PKT_CMD_TYPE_XXX)
    IN UCHAR PktCmdSeq, // Packet command sequence (NO_PKT_CMD_RSP_EVENT or 1~15 command sequence)
    IN PVOID pPktCmdParameters, // Packet command parameters
    IN USHORT PktCmdParametersLength, // Length of packet command
    IN UINT MicrosecondsToStall); // The number of microseconds to delay. A driver should specify no more than 50 microseconds.

//
// Request the Tx packet command
//
VOID TxPktCmdRst(
    IN PMP_ADAPTER pAd, 
    IN UCHAR QueueIdx, // Queue index
    IN PUCHAR pPktCmd, // Point to a packet command
    IN UCHAR PktCmdType, // Packet command type (PKT_CMD_TYPE_XXX)
    IN UCHAR PktCmdSeq, // Packet command sequence (NO_PKT_CMD_RSP_EVENT or 1~15 command sequence)
    IN USHORT PktCmdParametersLength); // Length of packet command

//
// Burst write (single or multiple registers)
//
VOID BurstWrite(
    IN PMP_ADAPTER pAd, 
    IN ULONG StartAddress, // Start address
    IN ULONG RegValue, // The register value
    IN ULONG NumOfRegValues); // Number of the register values to be written

//
// Burst read (single or multiple registers)
//
VOID BurstRead(
    IN PMP_ADAPTER pAd, 
    IN ULONG StartAddress, // Start address
    IN ULONG NumOfRegValues); // Number of the register values to be read

VOID SendEFuseBufferModeCMD(
    IN PMP_ADAPTER    pAd,
    IN UINT8                EepromType);

//VOID calib_table_get(unsigned int calibId, calibItem* CalibItem);
VOID calib_result_show(MP_ADAPTER *pAd,unsigned int calibId);
VOID calib_robust_routine(MP_ADAPTER *pAd,unsigned int calibId);

VOID SendRFRegAccessCMD(
    IN PMP_ADAPTER    pAd,
    IN UINT32           WifiStream,
    IN UINT32           Address);

VOID SendRFTestCMD(
    IN PMP_ADAPTER    pAd,
    IN UINT32               Action,
    IN UINT32               Mode,
    IN UINT32               CalibId);

VOID SendPowerManagementCMD(
    IN PMP_ADAPTER    pAd, 
    IN UINT8                PmNumber,
    IN UINT8                PmState,
    IN UINT8                *Bssid,
    IN UINT8                DtimPeriod,
    IN UINT8                BeaconInterval);

VOID SendRadioOnOffCMD(
    IN PMP_ADAPTER    pAd, 
    IN UINT8                bRadio);

VOID SendDisableRxCMD(
    IN PMP_ADAPTER    pAd);

VOID SendSwitchChannelCMD(
    IN PMP_ADAPTER pAd, 
    IN EXT_CMD_CHAN_SWITCH_T    CmdChSwitch);

VOID SendLEDCmd(
    IN PMP_ADAPTER pAd,
    IN PMP_PORT    pPort,
    IN ULONG    LEDNumber,
    IN ULONG    LEDBehavior);

//
//AndesPowerOperation
//
VOID SendPowerOperationCMDToAndes(
    IN PMP_ADAPTER pAd, 
    IN ULONG PowerOperation,
    IN ULONG PowerLevel) ;

VOID SendSwitchChannelOperationCMDToAndes(
    IN PMP_ADAPTER pAd, 
    IN SWITCH_CHANNEL_CMD_PARAMETERS SwitchChannelPacket) ;

//
// Rx response event handle
//
VOID RxRspEvtHandle(
    IN PMP_ADAPTER pAd, 
    IN PRX_FCE_INFO_STRUC pRxFceInfo, // Response event
    IN ULONG RxRingIdx); // Rx ring index

//
// Register the callback function of Rx response event
//
VOID RegisterRxRspEvtCallback(
    IN PMP_ADAPTER pAd);

#endif // __PKT_CMD_AND_RSP_EVT_H__

//////////////////////////////////// End of File ////////////////////////////////////
