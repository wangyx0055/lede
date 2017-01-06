/*
 ***************************************************************************
 * Ralink Tech Inc.
 * 4F, No. 2 Technology 5th Rd.
 * Science-based Industrial Park
 * Hsin-chu, Taiwan, R.O.C.
 *
 * (c) Copyright 2002-2004, Ralink Technology, Inc.
 *
 * All rights reserved. Ralink's source code is an unpublished work and the
 * use of a copyright notice does not imply otherwise. This source code
 * contains confidential trade secret material of Ralink Tech. Any attemp
 * or participation in deciphering, decoding, reverse engineering or in any
 * way altering the source code is stricitly prohibited, unless the prior
 * written consent of Ralink Technology, Inc. is obtained.
 ***************************************************************************

    Module Name:
    rtmp.h

    Abstract:
    Miniport generic portion header file

    Revision History:
    Who         When          What
    --------    ----------    ----------------------------------------------

*/
#ifndef __ADAPTER_H__
#define __ADAPTER_H__
#include    "MTmp.h"
#include    "NdisOids.h"
#include    "mac\mac_mt\smac\mt_mac.h"
#include    "..\backup\MT7603_backup.h"


#if IS_BUS_TYPE_USB(DEV_BUS_TYPE)
typedef struct _USB_CONFIG_T HIF_CONFIG, *PHIF_CONFIG;
#elif IS_BUS_TYPE_PCI(DEV_BUS_TYPE)
// Add PCI here
#elif IS_BUS_TYPE_SDIO(DEV_BUS_TYPE)
// Add SDIO here.
#endif

#define HW_MAX_SUPPORT_MBSSID 4
#define DEFAULT_BSSID_INDEX     0xff
#define BSSID_INDEX_0               0
#define BSSID_INDEX_1               1
#define BSSID_INDEX_2               2
#define BSSID_INDEX_3               3

// Own MAC & BSSID MAC option
#define UPDATE_MAC                      0x0
#define DELETE_MAC                      0x1
#define DO_NOTHING                      0xff

typedef struct _RTMP_ADAPTER
{
    // Handle given by NDIS when the Adapter registered itself.
    // Various NDIS handle function
    //
    NDIS_HANDLE             AdapterHandle;          // Adapetr Context

    // flags, see fRTMP_ADAPTER_xxx flags
    ULONG                   Flags;                      // Represent current device status

    NIC_STATE               AdapterState;
    MP_DOT11_STATE          State;

    // OP mode: either AP or STA
    UCHAR                   OpMode;                     // OPMODE_STA, OPMODE_AP

    BOOLEAN                 TerminateThreads;   

    NDIS_SPIN_LOCK          AdapterStateLock;  // for AdapterState resource.

    NDIS_DEVICE_POWER_STATE             DevicePowerState;  
    NDIS_DEVICE_POWER_STATE             NextDevicePowerState;
    BOOLEAN   MBssidTable[HW_MAX_SUPPORT_MBSSID];

    REGISTRY_CFG    RegistryCfg;

    //
    // HAL control
    //
    HAL_CTRL_T              HalCtrl;

    PRX_CONFIG              pRxCfg;

    PTX_CONFIG              pTxCfg;


    WPS_CONFIG              WpsCfg;

    COUNTER_T               Counter;

    UI_CONFIG_T             UiCfg;

#ifdef MULTI_CHANNEL_SUPPORT
    MULTI_CHANNEL_T         MccCfg;
#endif /* MULTI_CHANNEL_SUPPORT */

    LOGO_TEST_CONFIG        LogoTestCfg;

    TRACK_INFO_T            TrackInfo;

    PHIF_CONFIG             pHifCfg;

    HW_CONFIG_T             HwCfg;

    PNIC_CONFIG             pNicCfg;

    PP2P_CTRL_T             pP2pCtrll;

    BEACON_PORT             BeaconPort;
    
    /**
     * List to hold the ports created on this adapter
     */
    PMP_PORT                PortList[RTMP_MAX_NUMBER_OF_PORT];    // Public
    ULONG                   NumberOfPorts;                      // Public
    //Mac address list for WFD roles and one virtual staion port(the last mac list)
    DOT11_MAC_ADDRESS     MacAddressList[MAX_NUM_OF_TOTAL_WFD_ROLE];
    BOOLEAN                         MacAddressUsed[MAX_NUM_OF_TOTAL_WFD_ROLE];
    
    NDIS_PORT_NUMBER        ucActivePortNum;
    NDIS_PORT_NUMBER        ucScanPortNum;  

    USHORT                  TimerAllociateCnt;    
    
    // outgoing BEACON frame buffer of AP or Adhoc and corresponding TXD 
    // ------------------------------------------------------
    // common configuration to both OPMODE_STA and OPMODE_AP
    // ------------------------------------------------------
    COMMON_CONFIG           CommonCfg;
    MLME_STRUCT             Mlme;
 
    // WMM-PS: Curently, only support UAPSD feature for P2PGO
#if UAPSD_AP_SUPPORT
    U_APSD_T                UAPSD;
#endif

    // -----------------------------------------------
    // STA specific configuration & operation status
    // used only when pAd->OpMode == OPMODE_STA
    // -----------------------------------------------
    STA_ADMIN_CONFIG        StaCfg;           // user desired settings
    STA_ACTIVE_CONFIG       StaActive;         // valid only when ADHOC_ON(pAd) || INFRA_ON(pAd)
    MLME_AUX                MlmeAux;           // temporary settings used during MLME state machine
    BSS_TABLE               ScanTab;           // store the latest SCAN result

    // -----------------------------------------------
    // AP specific configuration & operation status
    // used only when pAd->OpMode == OPMODE_AP
    // -----------------------------------------------
    
    MAC_TABLE_POOL      MacTablePool;
    MAC_TABLE_ENTRY      MacTabPoolArray[MAX_LEN_OF_MAC_TABLE];
    
    WDS_TABLE               WdsTab;            // WDS table when working as an AP
    BA_TABLE                BATable;
    NDIS_SPIN_LOCK          BATabLock;

    NDIS_SPIN_LOCK          MemLock;

    BOOLEAN                 bInShutdown;
    BOOLEAN                 bWaitShutdownCMD;
    ULONG                   Shutdowncnt;

    BOOLEAN                 bWakeupFromS3S4;

    //for debug used
    ULONG                   BBPBusycnt;
    UINT                    CountDowntoRadioOff;  // if CommandBusyCount != 0, it means that a command is set within a period(default 5 secs)
    USHORT                  ConsecutivelyBeaconLostCount; // a counter to record consecutive forcesleep, ie. a counter for beacon lost consecutively
    USHORT                  SleepTBTT;

    BOOLEAN                 bConnectionInProgress;
    PHY_CFG                 UtfFixedTxRateCfg;  // Store the setting of "Fixed Tx rate". Refer to OID: MTK_OID_N6_SET_FIXED_RATE

    

#if _WIN8_USB_SS_SUPPORTED
    //
    // Selective Suspend
    //
    RTMP_SS                 Ss;
    ULONG                   PendingStatusIndication;
    ULONG                   PendingNBLIndication;
#endif

    //
    // TX Power
    //
    CHANNEL_TX_POWER        TxPower[MAX_NUM_OF_CHANNELS];       // Store Tx power value for all channels.
    ULONG                   Tx20MPwrCfgABand[5];
    ULONG                   Tx20MPwrCfgGBand[5];
    ULONG                   Tx40MPwrCfgABand[5];
    ULONG                   Tx40MPwrCfgGBand[5];

//  for debug only
//  byte3   RSSI threshold1 
//  byte2   VGA offset 1
//  byte1   RSSI threshold0
//  byte0   VGA offset 0
//  threshold1 should bigger than threshold0

////////////////////////////
////////////////////////////
//// MT7603
////////////////////////////
////////////////////////////
//  WTBL_CONTROL    WTBLControl;

#ifdef MT_MAC
    struct rtmp_mac_ctrl mac_ctrl;
#endif /* MT_MAC */

    /*
        Frequency setting for rate adaptation
            @ra_interval:       for baseline time interval
            @ra_fast_interval:  for quick response time interval
    */
    UINT32 ra_interval;
    UINT32 ra_fast_interval;

    ULONG  LastTxRate;


    // TODO: Need review. For build pass purpose.
    UINT32 ChipID;
    UINT32 HWVersion;
    UINT32 FWVersion;
    UCHAR AntMode;
    
    RTMP_CHIP_OP chipOps;
    RTMP_CHIP_CAP chipCap;

    BOOLEAN bDisableRtsProtect;
    UINT8 FlgCtsEnabled;
    ULONG TbttTickCount;	/* beacon timestamp work-around */
} MP_ADAPTER, *PMP_ADAPTER, RTMP_ADAPTER, *PRTMP_ADAPTER;

#endif
