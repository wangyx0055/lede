/*
 ***************************************************************************
 * MediaTek Inc.
 * 5F, No.5, Tai-Yuan 1st St., 
 * Chupei City, Hsinchu County 30265, Taiwan, R.O.C.
 *
 * (c) Copyright 2014-2016, MediaTek, Inc.
 *
 * All rights reserved. MediaTek's source code is an unpublished work and the
 * use of a copyright notice does not imply otherwise. This source code
 * contains confidential trade secret material of MediaTek. Any attemp
 * or participation in deciphering, decoding, reverse engineering or in any
 * way altering the source code is stricitly prohibited, unless the prior
 * written consent of MediaTek, Inc. is obtained.
 ***************************************************************************

    Module Name:
    rtmpCommon.h

    Abstract:
    Miniport generic portion header file

    Revision History:
    Who         When          What
    --------    ----------    ----------------------------------------------
    */

#ifndef __COMMON_H__
#define __COMMON_H__

//#include "MtConfig.h"
typedef struct  _CmdQ CmdQ, *PCmdQ;
typedef struct _CmdQElmt  CmdQElmt, *PCmdQElmt;


#define STRING_OFFSET(field)            ((UINT) FIELD_OFFSET(LOCAL_STRINGS, field))
#define STRING_SIZE(field)              sizeof(((PLOCAL_STRINGS)0)->field)
#define INIT_NDIS_STR(str, buf, len)    {(str).Length = 0;(str).MaximumLength = len;(str).Buffer = buf;}

#ifdef RTMP_INTERNAL_TX_ALC
//#define DEFAULT_BO              4
#define LIN2DB_ERROR_CODE       (-10000)
#define TEMPERATURE_DPD_UNKONW  0xFFFF
#define MAX_TSSI_WAITING_COUNT  40

#if 0
#define RF_PA_MODE0_DECODE      0
#define RF_PA_MODE1_DECODE      20480   // 2.5 * 8192
#define RF_PA_MODE2_DECODE      4096    // 0.5 * 8192
#define RF_PA_MODE3_DECODE      4096    // 0.5 * 8192
#else
#define RF_PA_MODE0_DECODE      0
#define RF_PA_MODE1_DECODE      8847    // 1.08 * 8192
#define RF_PA_MODE2_DECODE      -5734   // -0.7 * 8192
#define RF_PA_MODE3_DECODE      -5734   // -0.7 * 8192
#endif
#define BW20_MCS_POWER_CCK_1M           ((pAd->Tx20MPwrCfgGBand[0] & 0xFF) < 0x20)?(pAd->Tx20MPwrCfgGBand[0] & 0xFF):(CHAR)((pAd->Tx20MPwrCfgGBand[0] & 0xFF) - 0x40)
#define BW20_MCS_POWER_CCK_2M           ((pAd->Tx20MPwrCfgGBand[0] & 0xFF) < 0x20)?(pAd->Tx20MPwrCfgGBand[0] & 0xFF):(CHAR)((pAd->Tx20MPwrCfgGBand[0] & 0xFF) - 0x40)
#define BW20_MCS_POWER_CCK_5M           (((pAd->Tx20MPwrCfgGBand[0] & 0xFF00) >> 8) < 0x20)?((pAd->Tx20MPwrCfgGBand[0] & 0xFF00) >> 8):(CHAR)(((pAd->Tx20MPwrCfgGBand[0] & 0xFF00) >> 8)-0x40)
#define BW20_MCS_POWER_CCK_11M          (((pAd->Tx20MPwrCfgGBand[0] & 0xFF00) >> 8) < 0x20)?((pAd->Tx20MPwrCfgGBand[0] & 0xFF00) >> 8):(CHAR)(((pAd->Tx20MPwrCfgGBand[0] & 0xFF00) >> 8)-0x40)
#define BW40_MCS_POWER_CCK_1M           ((pAd->Tx40MPwrCfgGBand[0] & 0xFF) < 0x20)?(pAd->Tx40MPwrCfgGBand[0] & 0xFF) :(CHAR)(((pAd->Tx40MPwrCfgGBand[0] & 0xFF) < 0x20)-0x40)
#define BW40_MCS_POWER_CCK_2M           ((pAd->Tx40MPwrCfgGBand[0] & 0xFF) < 0x20)?(pAd->Tx40MPwrCfgGBand[0] & 0xFF) :(CHAR)(((pAd->Tx40MPwrCfgGBand[0] & 0xFF) < 0x20)-0x40)
#define BW40_MCS_POWER_CCK_5M           (((pAd->Tx40MPwrCfgGBand[0] & 0xFF00) >> 8) < 0x20)?((pAd->Tx40MPwrCfgGBand[0] & 0xFF00) >> 8):(CHAR)(((pAd->Tx40MPwrCfgGBand[0] & 0xFF00) >> 8)-0x40)
#define BW40_MCS_POWER_CCK_11M          (((pAd->Tx40MPwrCfgGBand[0] & 0xFF00) >> 8) < 0x20)?((pAd->Tx40MPwrCfgGBand[0] & 0xFF00) >> 8):(CHAR)(((pAd->Tx40MPwrCfgGBand[0] & 0xFF00) >> 8)-0x40)

#define BW20_MCS_POWER_OFDM_6M          (((pAd->Tx20MPwrCfgGBand[0] & 0xFF0000) >> 16) < 0x20)?((pAd->Tx20MPwrCfgGBand[0] & 0xFF0000) >> 16):(CHAR)(((pAd->Tx20MPwrCfgGBand[0] & 0xFF0000) >> 16)-0x40)
#define BW20_MCS_POWER_OFDM_9M          (((pAd->Tx20MPwrCfgGBand[0] & 0xFF0000) >> 16) < 0x20)?((pAd->Tx20MPwrCfgGBand[0] & 0xFF0000) >> 16):(CHAR)(((pAd->Tx20MPwrCfgGBand[0] & 0xFF0000) >> 16)-0x40)
#define BW20_MCS_POWER_OFDM_12M     (((pAd->Tx20MPwrCfgGBand[0] & 0xFF000000) >> 24) < 0x20)?((pAd->Tx20MPwrCfgGBand[0] & 0xFF000000) >> 24):(CHAR)(((pAd->Tx20MPwrCfgGBand[0] & 0xFF000000) >> 24)-0x40)
#define BW20_MCS_POWER_OFDM_18M     (((pAd->Tx20MPwrCfgGBand[0] & 0xFF000000) >> 24) < 0x20)?((pAd->Tx20MPwrCfgGBand[0] & 0xFF000000) >> 24):(CHAR)(((pAd->Tx20MPwrCfgGBand[0] & 0xFF000000) >> 24)-0x40)
#define BW20_MCS_POWER_OFDM_24M     ((pAd->Tx20MPwrCfgGBand[1] & 0xFF) < 0x20)?(pAd->Tx20MPwrCfgGBand[1] & 0xFF):(CHAR)((pAd->Tx20MPwrCfgGBand[1] & 0xFF)-0x40)
#define BW20_MCS_POWER_OFDM_36M     ((pAd->Tx20MPwrCfgGBand[1] & 0xFF) < 0x20)?(pAd->Tx20MPwrCfgGBand[1] & 0xFF):(CHAR)((pAd->Tx20MPwrCfgGBand[1] & 0xFF)-0x40)
#define BW20_MCS_POWER_OFDM_48M     (((pAd->Tx20MPwrCfgGBand[1] & 0xFF00) >> 8) < 0x20)?((pAd->Tx20MPwrCfgGBand[1] & 0xFF00) >> 8):(CHAR)(((pAd->Tx20MPwrCfgGBand[1] & 0xFF00) >> 8)-0x40)
#define BW20_MCS_POWER_OFDM_54M     (((pAd->Tx20MPwrCfgGBand[1] & 0xFF00) >> 8) < 0x20)?((pAd->Tx20MPwrCfgGBand[1] & 0xFF00) >> 8):(CHAR)(((pAd->Tx20MPwrCfgGBand[1] & 0xFF00) >> 8)-0x40)
#define BW40_MCS_POWER_OFDM_6M          (((pAd->Tx40MPwrCfgGBand[0] & 0xFF0000) >> 16) < 0x20)?((pAd->Tx40MPwrCfgGBand[0] & 0xFF0000) >> 16):(CHAR)(((pAd->Tx40MPwrCfgGBand[0] & 0xFF0000) >> 16)-0x40)
#define BW40_MCS_POWER_OFDM_9M          (((pAd->Tx40MPwrCfgGBand[0] & 0xFF0000) >> 16) < 0x20)?((pAd->Tx40MPwrCfgGBand[0] & 0xFF0000) >> 16):(CHAR)(((pAd->Tx40MPwrCfgGBand[0] & 0xFF0000) >> 16)-0x40)
#define BW40_MCS_POWER_OFDM_12M     (((pAd->Tx40MPwrCfgGBand[0] & 0xFF000000) >> 24) < 0x20)?((pAd->Tx40MPwrCfgGBand[0] & 0xFF000000) >> 24):(CHAR)(((pAd->Tx40MPwrCfgGBand[0] & 0xFF000000) >> 24)-0x40)
#define BW40_MCS_POWER_OFDM_18M     (((pAd->Tx40MPwrCfgGBand[0] & 0xFF000000) >> 24) < 0x20)?((pAd->Tx40MPwrCfgGBand[0] & 0xFF000000) >> 24):(CHAR)(((pAd->Tx40MPwrCfgGBand[0] & 0xFF000000) >> 24)-0x40)
#define BW40_MCS_POWER_OFDM_24M     ((pAd->Tx40MPwrCfgGBand[1] & 0xFF) < 0x20)?(pAd->Tx40MPwrCfgGBand[1] & 0xFF):(CHAR)((pAd->Tx40MPwrCfgGBand[1] & 0xFF)-0x40)
#define BW40_MCS_POWER_OFDM_36M     ((pAd->Tx40MPwrCfgGBand[1] & 0xFF) < 0x20)?(pAd->Tx40MPwrCfgGBand[1] & 0xFF):(CHAR)((pAd->Tx40MPwrCfgGBand[1] & 0xFF)-0x40)
#define BW40_MCS_POWER_OFDM_48M     (((pAd->Tx40MPwrCfgGBand[1] & 0xFF00) >> 8) < 0x20)?((pAd->Tx40MPwrCfgGBand[1] & 0xFF00) >> 8):(CHAR)(((pAd->Tx40MPwrCfgGBand[1] & 0xFF00) >> 8)-0x40)
#define BW40_MCS_POWER_OFDM_54M     (((pAd->Tx40MPwrCfgGBand[1] & 0xFF00) >> 8) < 0x20)?((pAd->Tx40MPwrCfgGBand[1] & 0xFF00) >> 8):(CHAR)(((pAd->Tx40MPwrCfgGBand[1] & 0xFF00) >> 8)-0x40)

#define BW20_MCS_POWER_HT_MCS0          (((pAd->Tx20MPwrCfgGBand[1] & 0xFF0000) >> 16) < 0x20)?((pAd->Tx20MPwrCfgGBand[1] & 0xFF0000) >> 16):(CHAR)(((pAd->Tx20MPwrCfgGBand[1] & 0xFF0000) >> 16)-0x40)
#define BW20_MCS_POWER_HT_MCS1          (((pAd->Tx20MPwrCfgGBand[1] & 0xFF0000) >> 16) < 0x20)?((pAd->Tx20MPwrCfgGBand[1] & 0xFF0000) >> 16):(CHAR)(((pAd->Tx20MPwrCfgGBand[1] & 0xFF0000) >> 16)-0x40)
#define BW20_MCS_POWER_HT_MCS2          (((pAd->Tx20MPwrCfgGBand[1] & 0xFF000000) >> 24) < 0x20)?((pAd->Tx20MPwrCfgGBand[1] & 0xFF000000) >> 24):(CHAR)(((pAd->Tx20MPwrCfgGBand[1] & 0xFF000000) >> 24)-0x40)
#define BW20_MCS_POWER_HT_MCS3          (((pAd->Tx20MPwrCfgGBand[1] & 0xFF000000) >> 24) < 0x20)?((pAd->Tx20MPwrCfgGBand[1] & 0xFF000000) >> 24):(CHAR)(((pAd->Tx20MPwrCfgGBand[1] & 0xFF000000) >> 24)-0x40)
#define BW20_MCS_POWER_HT_MCS4          ((pAd->Tx20MPwrCfgGBand[2] & 0xFF) < 0x20)?(pAd->Tx20MPwrCfgGBand[2] & 0xFF):(CHAR)((pAd->Tx20MPwrCfgGBand[2] & 0xFF)-0x40)
#define BW20_MCS_POWER_HT_MCS5          ((pAd->Tx20MPwrCfgGBand[2] & 0xFF) < 0x20)?(pAd->Tx20MPwrCfgGBand[2] & 0xFF):(CHAR)((pAd->Tx20MPwrCfgGBand[2] & 0xFF)-0x40)
#define BW20_MCS_POWER_HT_MCS6          (((pAd->Tx20MPwrCfgGBand[2] & 0xFF00 ) >> 8) < 0x20)?((pAd->Tx20MPwrCfgGBand[2] & 0xFF00 ) >> 8):(CHAR)(((pAd->Tx20MPwrCfgGBand[2] & 0xFF00 ) >> 8)-0x40)
#define BW20_MCS_POWER_HT_MCS7          (((pAd->Tx20MPwrCfgGBand[2] & 0xFF00 ) >> 8) < 0x20)?((pAd->Tx20MPwrCfgGBand[2] & 0xFF00 ) >> 8):(CHAR)(((pAd->Tx20MPwrCfgGBand[2] & 0xFF00 ) >> 8)-0x40)
#define BW40_MCS_POWER_HT_MCS0          (((pAd->Tx40MPwrCfgGBand[1] & 0xFF0000) >> 16) < 0x20)?((pAd->Tx40MPwrCfgGBand[1] & 0xFF0000) >> 16):(CHAR)(((pAd->Tx40MPwrCfgGBand[1] & 0xFF0000) >> 16)-0x40)
#define BW40_MCS_POWER_HT_MCS1          (((pAd->Tx40MPwrCfgGBand[1] & 0xFF0000) >> 16) < 0x20)?((pAd->Tx40MPwrCfgGBand[1] & 0xFF0000) >> 16):(CHAR)(((pAd->Tx40MPwrCfgGBand[1] & 0xFF0000) >> 16)-0x40)
#define BW40_MCS_POWER_HT_MCS2          (((pAd->Tx40MPwrCfgGBand[1] & 0xFF000000) >> 24) < 0x20)?((pAd->Tx40MPwrCfgGBand[1] & 0xFF000000) >> 24):(CHAR)(((pAd->Tx40MPwrCfgGBand[1] & 0xFF000000) >> 24)-0x40)
#define BW40_MCS_POWER_HT_MCS3          (((pAd->Tx40MPwrCfgGBand[1] & 0xFF000000) >> 24) < 0x20)?((pAd->Tx40MPwrCfgGBand[1] & 0xFF000000) >> 24):(CHAR)(((pAd->Tx40MPwrCfgGBand[1] & 0xFF000000) >> 24)-0x40)
#define BW40_MCS_POWER_HT_MCS4          ((pAd->Tx40MPwrCfgGBand[2] & 0xFF) < 0x20)?(pAd->Tx40MPwrCfgGBand[2] & 0xFF):(CHAR)((pAd->Tx40MPwrCfgGBand[2] & 0xFF)-0x40)
#define BW40_MCS_POWER_HT_MCS5          ((pAd->Tx40MPwrCfgGBand[2] & 0xFF) < 0x20)?(pAd->Tx40MPwrCfgGBand[2] & 0xFF):(CHAR)((pAd->Tx40MPwrCfgGBand[2] & 0xFF)-0x40)
#define BW40_MCS_POWER_HT_MCS6          (((pAd->Tx40MPwrCfgGBand[2] & 0xFF00) >> 8) < 0x20)?(pAd->Tx40MPwrCfgGBand[2] & 0xFF00):(CHAR)((pAd->Tx40MPwrCfgGBand[2] & 0xFF00)-0x40)
#define BW40_MCS_POWER_HT_MCS7          (((pAd->Tx40MPwrCfgGBand[2] & 0xFF00) >> 8) < 0x20)?(pAd->Tx40MPwrCfgGBand[2] & 0xFF00):(CHAR)((pAd->Tx40MPwrCfgGBand[2] & 0xFF00)-0x40)

#define RF_PA_MODE_CCK_1M               (pAd->HwCfg.TxALCData.PAModeCCK[0])
#define RF_PA_MODE_CCK_2M               (pAd->HwCfg.TxALCData.PAModeCCK[1]) 
#define RF_PA_MODE_CCK_5M               (pAd->HwCfg.TxALCData.PAModeCCK[2]) 
#define RF_PA_MODE_CCK_11M              (pAd->HwCfg.TxALCData.PAModeCCK[3]) 

#define RF_PA_MODE_OFDM_6M              (pAd->HwCfg.TxALCData.PAModeOFDM[0])
#define RF_PA_MODE_OFDM_9M              (pAd->HwCfg.TxALCData.PAModeOFDM[1])
#define RF_PA_MODE_OFDM_12M             (pAd->HwCfg.TxALCData.PAModeOFDM[2]) 
#define RF_PA_MODE_OFDM_18M             (pAd->HwCfg.TxALCData.PAModeOFDM[3]) 
#define RF_PA_MODE_OFDM_24M             (pAd->HwCfg.TxALCData.PAModeOFDM[4])
#define RF_PA_MODE_OFDM_36M             (pAd->HwCfg.TxALCData.PAModeOFDM[5])
#define RF_PA_MODE_OFDM_48M             (pAd->HwCfg.TxALCData.PAModeOFDM[6])
#define RF_PA_MODE_OFDM_54M             (pAd->HwCfg.TxALCData.PAModeOFDM[7])

#define RF_PA_MODE_HT_MCS0              (pAd->HwCfg.TxALCData.PAModeHT[0])
#define RF_PA_MODE_HT_MCS1              (pAd->HwCfg.TxALCData.PAModeHT[1]) 
#define RF_PA_MODE_HT_MCS2              (pAd->HwCfg.TxALCData.PAModeHT[2]) 
#define RF_PA_MODE_HT_MCS3              (pAd->HwCfg.TxALCData.PAModeHT[3]) 
#define RF_PA_MODE_HT_MCS4              (pAd->HwCfg.TxALCData.PAModeHT[4])
#define RF_PA_MODE_HT_MCS5              (pAd->HwCfg.TxALCData.PAModeHT[5])
#define RF_PA_MODE_HT_MCS6              (pAd->HwCfg.TxALCData.PAModeHT[6])
#define RF_PA_MODE_HT_MCS7              (pAd->HwCfg.TxALCData.PAModeHT[7])
#define RF_PA_MODE_HT_MCS8              (pAd->HwCfg.TxALCData.PAModeHT[8])
#define RF_PA_MODE_HT_MCS9              (pAd->HwCfg.TxALCData.PAModeHT[9]) 
#define RF_PA_MODE_HT_MCS10             (pAd->HwCfg.TxALCData.PAModeHT[10]) 
#define RF_PA_MODE_HT_MCS11             (pAd->HwCfg.TxALCData.PAModeHT[11]) 
#define RF_PA_MODE_HT_MCS12             (pAd->HwCfg.TxALCData.PAModeHT[12])
#define RF_PA_MODE_HT_MCS13             (pAd->HwCfg.TxALCData.PAModeHT[13])
#define RF_PA_MODE_HT_MCS14             (pAd->HwCfg.TxALCData.PAModeHT[14])
#define RF_PA_MODE_HT_MCS15             (pAd->HwCfg.TxALCData.PAModeHT[15])

enum TEMPERATURE_MODE {
    TEMPERATURE_MODE_NORMAL,
    TEMPERATURE_MODE_LOW,
    TEMPERATURE_MODE_HIGH,
};

typedef struct _MT7601_TX_ALC_DATA {
    INT32   PowerDiffPre;
    INT32   TemperatureDPD;
    INT32   MT7601_TSSI_T0_Delta_Offset;
    INT32   CurrentTemperature;
    INT16   TSSI_DBOFFSET_HVGA;
    INT16   TSSI0_DB;
    UCHAR   TssiSlope;
    CHAR    TssiDC0;
    CHAR    TssiDC0_HVGA;
    CHAR    TemperatureRef25C;
    UCHAR   InitTxAlcCfg1;
    UCHAR   TemperatureMode;
    BOOLEAN TSSI_USE_HVGA;
    BOOLEAN TssiTriggered;
    CHAR    MT7601_TSSI_OFFSET[3];
    INT16   PAModeCCK[4];
    INT16   PAModeOFDM[8];
    INT16   PAModeHT[16];
} MT7601_TX_ALC_DATA, *PMT7601_TX_ALC_DATA;
#endif /* RTMP_INTERNAL_TX_ALC */


//#pragma pack(pop, struct_pack1)

//
// Initialize HW versoin
//
VOID InitHwVersion(
    IN PMP_ADAPTER pAd
    );

//
// Initialize RT6X9X MAC
//
VOID InitMacRT6X9X(
    IN PMP_ADAPTER pAd
    );

//
// Select MAC version address base from PID
//
ULONG GET_ASIC_VERSION_ID(
    IN  PMP_ADAPTER   pAd
    );

//
// Initialize firmware control
//
VOID InitFwCtrl(
    IN PMP_ADAPTER pAd
    );

//
// Initialize VHT control
//
VOID VhtInitCtrl(
    IN PMP_ADAPTER pAd
    );

//
// Switch channel bandwidth for 6x9x
//
VOID SwitchBandwidth6x9x(
    IN PMP_ADAPTER pAd, 
    IN BOOLEAN bSwitchChannel, 
    IN UCHAR Channel, 
    IN UCHAR Bw,                        // BW_XXX
    IN UCHAR ExtensionChannelOffset// EXTCHA_XXX
    );   

//
// Set Rx antenna based on the number of Rx paths
//
VOID BbSetRxAntByRxPath(
    IN PMP_ADAPTER pAd
    );

//
// Set RF channel frequency parameters
//
VOID SetRfChFreqParameters7601(
    IN PMP_ADAPTER pAd, 
    IN UCHAR Channel// Channel number
    );  

//
// Get the index of the VHT rate table
//
//
// Get the index of the VHT rate table
//
ULONG GetVhtRateTableIdx(
    IN PMP_ADAPTER pAd, 
    IN ULONG Bw, // BW_XXX
    IN ULONG Nss, // NSS_XXX
    IN ULONG ShortGI, // GI_XXX
    IN ULONG MCS// MCS_XXX
    ); 

//
// Initialize the Rx AGC VGA tuning table
//
VOID BbInitRxAgcVgaTuningTable(
    IN PMP_ADAPTER pAd
    );

//
// Initialize 6X9X RF registers
//
VOID Init6x9xRfReg(
    IN PMP_ADAPTER pAd
    );

VOID MT7601_TemperatureCompensation(
    IN PMP_ADAPTER pAd,
    IN CHAR Temperature
    );

VOID BbpInit7601(
    IN PMP_ADAPTER pAd
    );

////
//  Private routines in rtmp_init.c
//
LPSTR 
DecodeCipherName (
    ULONG algorithm
    );

VOID AsicRadioOn(
    IN PMP_ADAPTER pAd
    );

VOID 
RTMPSetDefaultCipher(
    IN PMP_ADAPTER pAd,
    IN PMP_PORT pPort
    );

VOID 
RTMPSetDefaultAuthAlgo(
    IN PMP_ADAPTER pAd,
    IN PMP_PORT pPort
    );  
    
VOID    
NICResetFromError(
    IN  PMP_ADAPTER   pAd
    );

VOID    ReleaseAdapterInternalBlock(
    IN  PMP_ADAPTER   pAd);

VOID    
ReleaseAdapter(
    IN  PMP_ADAPTER   pAd
    );   


NDIS_STATUS 
NICLoadFirmware(
    IN PMP_ADAPTER pAd
    );    

VOID 
RTMPInitPhyType(IN PMP_ADAPTER pAd
    );   

VOID    
UserCfgInit(
    IN  PMP_ADAPTER pAd
    );  

NDIS_STATUS
RTMPSetRegistrySwRadioOff(
    IN PMP_ADAPTER    pAd,
    IN BOOLEAN          SwRadioOff
    );    

VOID 
NICRestoreBBPValue(
    IN PMP_ADAPTER pAd
    );    

VOID 
AsicRadioOff(
    IN PMP_ADAPTER pAd
    );  

VOID 
AsicRFOn(
    IN PMP_ADAPTER pAd
    );   

VOID 
RTMPSetExtRegMessageEvent(
    IN PMP_ADAPTER    pAd,
    IN ULONG            EventId,
    IN PUCHAR           Msg,
    IN ULONG            Length
    );

VOID    
MTInitializeCmdQ(
    IN  PCmdQ   cmdq
    );   

VOID
MtInitInternalCmdBuffer(
    IN PMP_ADAPTER    pAd
    );
    
VOID
MtFreeInternalCmdBuffer(
    IN PMP_ADAPTER    pAd
    );

PCmdQElmt
MtGetInternalCmdBuffer(
    IN PMP_ADAPTER    pAd
    );

VOID
MtReturnInternalCmdBuffer(
    IN PMP_ADAPTER    pAd,
    IN PCmdQElmt   cmdqelmt
    );
    
NDIS_STATUS 
MTEnqueueInternalCmd(
    IN PMP_ADAPTER    pAd,
    IN PMP_PORT      pPort,
    IN LONG             Cmd,
    IN PVOID            Buffer,
    IN ULONG            BufferLen
    );    

NDIS_STATUS 
MTDeQueueNoAMgmtPacket(
    IN  PMP_ADAPTER   pAd
    );

VOID    
MTSuspendMsduTransmission(
    IN  PMP_ADAPTER   pAd
    ); 

UCHAR 
MTDecideSegmentEnd(
    IN    ULONG             CurWritePosition
    );    

NDIS_STATUS
InitializeHWCtrl (
    IN  PMP_ADAPTER pAd,
    OUT PADAPTER_INITIAL_STAGE  pInitialStage
    );    

NDIS_STATUS
InitializeProtocolCtrl (
    IN  PMP_ADAPTER pAd
    );    
#endif
