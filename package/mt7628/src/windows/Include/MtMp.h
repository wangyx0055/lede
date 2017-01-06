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
#ifndef __MTMP_H__
#define __MTMP_H__

#include "PmfCmm.h"
#include    "NdisOids.h"

typedef struct _P2P_CTRL_T P2P_CTRL_T, *PP2P_CTRL_T;
//#define MT7603_FPGA 1

//
//  NDIS Version definitions
//
#define RTMP_NDIS_MAJOR_VERSION     6
#define RTMP_NDIS_MINOR_VERSION     20

#define RTMP_NDIS620_VERSION    0x60014
#define RTMP_NDIS630_VERSION    0x6001e
#define RTMP_NDIS640_VERSION    0x60028 // Windows Blue

#define NIC_MAJOR_DRIVER_VERSION        0x01
#define NIC_MINOR_DRIVER_VERSION        0x01

extern  char    NIC_VENDOR_DESC[];
extern  int     NIC_VENDOR_DESC_LEN;

extern  char    NIC_WPS_DESC[];
extern  int     NIC_WPS_DESC_LEN;

extern NDIS_OID NICSupportedOids[];
extern ULONG NIC_SUPPORT_OID_SIZE;

extern  unsigned long   NIC_VENDOR_DRIVER_VERSION;
//extern    unsigned short  NIC_PCI_DEVICE_ID;
extern  unsigned short  NIC2860_PCI_DEVICE_ID;
extern  unsigned short  NIC2561_PCI_DEVICE_ID;
extern  unsigned short  NIC2561Turbo_PCI_DEVICE_ID;
extern  unsigned short  NIC_PCI_VENDOR_ID;
extern  unsigned char   SNAP_AIRONET[];

extern  unsigned char   CipherWpaTemplate[];
extern  unsigned char   CipherWpaTemplateLen;
extern  unsigned char   CipherWpa2Template[];
extern  unsigned char   CipherWpa2TemplateLen;

extern  unsigned char   CipherSuiteCiscoCCKMCkipCmic[];
extern  unsigned char   CipherSuiteCiscoCCKM[];
extern  unsigned char   CipherSuiteCiscoCCKMLen;
extern  unsigned char   CipherSuiteCiscoCCKM24[];
extern  unsigned char   CipherSuiteCiscoCCKMWep104[];
extern  unsigned char   CipherSuiteCiscoCCKMCKIP[];
extern  unsigned char   CipherSuiteCiscoCCKMCmic[];
extern  unsigned char   CipherSuiteCiscoCCKMTkip[];
extern  unsigned char   CipherSuiteCiscoCCKMAES[];
extern  unsigned char   CipherSuiteCiscoCCKMTkip24[];
extern  unsigned char   CipherSuiteCiscoCCKMTkip24Len;
extern  unsigned char   CipherSuiteCCXTkip[];
extern  unsigned char   CipherSuiteCCXTkipLen;
extern  unsigned char   CipherSuiteCiscoCCKMWPALen;
extern  unsigned char   CipherSuiteCiscoCCKMWPA2Tkip[];
extern  unsigned char   CipherSuiteCiscoCCKMWPA2AES[];
extern  unsigned char   CipherSuiteCiscoCCKMWPA2Len ;

extern  unsigned char   CISCO_OUI[];
extern  UCHAR   BaSizeArray[4];

#if(COMPILE_WIN8_ABOVE(CURRENT_OS_NDIS_VER))  
extern DOT11_AUTH_CIPHER_PAIR BSSMulticastMgmtAuthCipherPairs[];
extern USHORT  uBSSMulticastMgmtAuthCipherPairsSize;
#endif

extern DOT11_AUTH_CIPHER_PAIR   ExtAPUnicastAuthCipherPairs[];
extern USHORT uExtAPUnicastAuthCipherPairsSize;
extern DOT11_AUTH_CIPHER_PAIR   ExtAPMulticastAuthCipherPairs[];
extern USHORT uExtAPMulticastAuthCipherPairsSize;
    
extern UCHAR BROADCAST_ADDR[MAC_ADDR_LEN];
extern UCHAR ZERO_MAC_ADDR[MAC_ADDR_LEN];
extern ULONG TIM_BITMAP[32];
//extern char* CipherName[];
extern char* NUMB[];
extern UCHAR SNAP_802_1H[6];
extern UCHAR SNAP_AMP_LLC[6];
extern UCHAR SNAP_BRIDGE_TUNNEL[6];
extern UCHAR SNAP_AIRONET[8];
extern UCHAR EAPOL_LLC_SNAP[8];
extern UCHAR EAPOL[2];
extern UCHAR AMP_DATA_FRAME[2];
extern UCHAR AMP_ACTIVITY_REPORT[8];
extern UCHAR AMP_ACTIVITY_REPORT_FRAME[2];
extern UCHAR AMP_SECURITY_FRAME[2];
extern UCHAR    AMP_LSREQ_FRAME[2];
extern UCHAR    AMP_LSREPLY_FRAME[2];
extern  UCHAR   AMP_LSREQ_LLC_SNAP[8];
extern  UCHAR   AMP_LSREPLY_LLC_SNAP[8];
extern UCHAR    AMP_DATA_LLC_SNAP[8];
extern UCHAR IPX[2];
extern UCHAR APPLE_TALK[2];
extern UCHAR TDLS_LLC_SNAP[8];
extern UCHAR TDLS_LLC_SNAP_WITH_CATEGORY[10];
extern UCHAR TDLS_ETHERTYPE[2];
extern UCHAR     OfdmRateToRxwiMCS[];
extern UCHAR MapUserPriorityToAccessCategory[8];

extern UCHAR  RateIdToMbps[];
extern USHORT RateIdTo500Kbps[];

extern UCHAR  CipherSuiteWpaNoneTkip[];
extern UCHAR  CipherSuiteWpaNoneTkipLen;

extern UCHAR  CipherSuiteWpaNoneAes[];
extern UCHAR  CipherSuiteWpaNoneAesLen;

extern UCHAR  CipherSuiteAdHocWpa2Aes[];
extern UCHAR  CipherSuiteAdHocWpa2AesLen;

extern UCHAR  SsidIe;
extern UCHAR  SupRateIe;
extern UCHAR  ExtRateIe;
extern UCHAR  HtCapIe;
extern UCHAR  ExtHtCapIe;
extern UCHAR  BssCoexistIe;
extern UCHAR  AddHtInfoIe;
extern UCHAR  NewExtChanIe;
extern UCHAR  ErpIe;
extern UCHAR  DsIe;
extern UCHAR  TimIe;
extern UCHAR  WpaIe;
extern UCHAR  Wpa2Ie;
extern UCHAR  IbssIe;
extern UCHAR  CcxIe;
 
extern UCHAR  VhtCapIe;
extern UCHAR  VhtOpIe;
extern UCHAR  VhtExtBssLoadIe;
extern UCHAR  VhtWideBwChSwitchIe;
extern UCHAR  VhtTxPowerIe;
extern UCHAR  VhtExtPowerConstraintIe;
extern UCHAR  VhtAidIe;
extern UCHAR  VhtQuietChannel;
extern UCHAR  VhtOperatingModeNotification;

extern UCHAR  WPA_OUI[];
extern UCHAR  WPS_OUI[];
extern UCHAR  WME_INFO_ELEM[];
extern UCHAR  WME_PARM_ELEM[];
extern UCHAR  Ccx2QosInfo[];
extern UCHAR  Ccx2IeInfo[];
extern UCHAR  RALINK_OUI[];
#define MAXSEQ      0xfff
#define   MGMTPIPEIDX           0   // EP6 is highest priority

extern UCHAR  RateSwitchTable[];
extern UCHAR  RateSwitchTable11B[];
extern UCHAR  RateSwitchTable11G[];
extern UCHAR  RateSwitchTable11BGAll[];
extern UCHAR  RateSwitchTable11BG[];
extern UCHAR  RateSwitchTable11BGN1S[];
extern UCHAR  RateSwitchTable11BGN2S[];
extern UCHAR  RateSwitchTable11BGN2SForABand[];
extern UCHAR  RateSwitchTable11N1S[];
extern UCHAR  RateSwitchTable11N2S[];
extern UCHAR  RateSwitchTable11N2SForABand[];

#define MAX_AGG_3SS_BALIMIT     31

extern UCHAR AGS1x1HTRateTable[];
extern UCHAR AGS1x1HTRateTable5G[];
extern UCHAR AGS2x2HTRateTable[];
extern UCHAR AGS3x3HTRateTable[];

//
// The size, in bytes, of an AGS entry in the rate switch table
//
//#define SIZE_OF_AGS_RATE_TABLE_ENTRY  9

extern UCHAR  ZeroSsid[32];

extern ULONG  NIC_SUPPORT_OID_SIZE;

// wsc
extern UCHAR    Wsc_Manufacture_Len;
extern UCHAR    Wsc_Model_Name_Len;
extern UCHAR    Wsc_Model_Number_Len;
extern UCHAR    Wsc_Model_Serial_Len;
extern UCHAR    Wsc_Manufacture[];
extern UCHAR    Wsc_Model_Name[];
extern UCHAR    Wsc_Model_Number[];
extern UCHAR    Wsc_Model_Serial[];
extern UCHAR    Wsc_SMI_Code[];
extern UCHAR    Wsc_Pri_Dev_Type[];
extern UCHAR    Wsc_Uuid[];

// p2p
extern UCHAR    P2POUIBYTE[];
extern UCHAR    WILDP2PSSID[];
extern UCHAR    WILDP2PSSIDLEN;

// ******************************************
// [WAPI]
//
#define ROL(x,y)    ((x)<<(y) |    (x)>>(32-(y)))
#define ROUND            32
extern UCHAR    WAPI_PROTOTYPE[2];
extern UCHAR    WAPI_INIT_PN[2];
extern UCHAR    WAPI_INIT_SEQ[2];
extern  const unsigned long CK[ROUND];
extern UCHAR  CipherSuiteWAPIPsk[];
extern UCHAR  CipherSuiteWAPIPskLen;
extern UCHAR  CipherSuiteWAPICert[];
extern UCHAR  CipherSuiteWAPICertLen;
// ******************************************


#define MAX_WRITE_USB_REQUESTS 2

typedef struct _RTMP_ADAPTER MP_ADAPTER, *PMP_ADAPTER;
typedef struct _MP_PORT MP_PORT, *PMP_PORT;
typedef struct _RTMP_CTXREQ RTMP_CTXREQ,*PRTMP_CTXREQ;

//
//  Queue structure and macros
//
typedef struct  _QUEUE_ENTRY    {
    struct _QUEUE_ENTRY     *Next;
}   QUEUE_ENTRY, *PQUEUE_ENTRY;

// Queue structure
typedef struct  _QUEUE_HEADER   {
    PQUEUE_ENTRY    Head;
    PQUEUE_ENTRY    Tail;
    ULONG           Number;
}   QUEUE_HEADER, *PQUEUE_HEADER;

#define InitializeQueueHeader(QueueHeader)              \
{                                                       \
    (QueueHeader)->Head = (QueueHeader)->Tail = NULL;   \
    (QueueHeader)->Number = 0;                          \
}

#define RemoveHeadQueue(QueueHeader)                \
(QueueHeader)->Head;                                \
{                                                   \
    PQUEUE_ENTRY pNext;                             \
    if ((QueueHeader)->Head != NULL)                \
    {                                               \
        pNext = (QueueHeader)->Head->Next;          \
        (QueueHeader)->Head = pNext;                \
        if (pNext == NULL)                          \
            (QueueHeader)->Tail = NULL;             \
        (QueueHeader)->Number--;                    \
    }                                               \
}
#define GetHeaderNextQueue(QueueHeader)                \
((PQUEUE_HEADER)QueueHeader)->Head->Next 

#define IsQueueEmpty(QueueHeader)                \
    ((((PQUEUE_HEADER)QueueHeader)->Head == NULL) || (((PQUEUE_HEADER)QueueHeader)->Head == (PQUEUE_HEADER)QueueHeader))

#define IsQueueNotEmpty(QueueHeader)                \
    ((QueueHeader)->Head != (QueueHeader)->Tail)

#define InsertHeadQueue(QueueHeader, QueueEntry)            \
{                                                           \
    ((PQUEUE_ENTRY)QueueEntry)->Next = (QueueHeader)->Head; \
    (QueueHeader)->Head = (PQUEUE_ENTRY)(QueueEntry);       \
    if ((QueueHeader)->Tail == NULL)                        \
        (QueueHeader)->Tail = (PQUEUE_ENTRY)(QueueEntry);   \
    (QueueHeader)->Number++;                                \
}

#define InsertTailQueue(QueueHeader, QueueEntry)                \
{                                                               \
    ((PQUEUE_ENTRY)QueueEntry)->Next = NULL;                    \
    if ((QueueHeader)->Tail)                                    \
        (QueueHeader)->Tail->Next = (PQUEUE_ENTRY)(QueueEntry); \
    else                                                        \
        (QueueHeader)->Head = (PQUEUE_ENTRY)(QueueEntry);       \
    (QueueHeader)->Tail = (PQUEUE_ENTRY)(QueueEntry);           \
    (QueueHeader)->Number++;                                    \
}

#define STR_COPY_WITH_ASCII(__S1, __S2, __Len)       \
{                                                                                       \
    LONG    __i;                                                                    \
    for(__i =  0; __i < __Len; __i ++)                               \
    {                                                                                   \
        __S1[__i] = __S2[__i];                                       \
        if(__S2[__i] == '\0')                                                \
        {                                                                               \
            break;                                                                   \
        }                                                                               \
    }                                                                                   \
}

//
//  MACRO for debugging information
//

#if DBG
extern ULONG    RTDebugLevel;

#define DBGPRINT_RAW(Level, Fmt)    \
do{                                   \
    if (Level <= RTDebugLevel)      \
    {                               \
        DbgPrint  Fmt;               \
    }                               \
}while(0)

#if 0
#define DBGPRINT(Level, Fmt)        \
{                                   \
    if (Level <= RTDebugLevel)      \
    {                               \
        DbgPrint(NIC_DBG_STRING);   \
        DbgPrint Fmt;               \
    }                               \
}
#else
#define DBGPRINT(Level, Fmt)    DBGPRINT_RAW(Level, Fmt)
#define MTWF_LOG(Category, SubCategory, Level, Fmt)    DBGPRINT(Level, Fmt)

#define FUNC_ENTER  DbgPrint("===>%s():\n", __FUNCTION__)
#define FUNC_LEAVE   DbgPrint("<===%s():\n", __FUNCTION__)
#define FUNC_LEAVE_STATUS(_S)   DBGPRINT(RT_DEBUG_TRACE, ("<===%s(): Status 0x%x\n", __FUNCTION__, _S))  
#endif

#define DBGPRINT_ERR(Fmt)           \
{                                   \
    DbgPrint("ERROR!!! ");          \
    DbgPrint Fmt;                  \
}

#define DBGPRINT_S(Status, Fmt)                                         \
{                                                                       \
    ULONG   dbglevel;                                                   \
    if (Status == NDIS_STATUS_SUCCESS || Status == NDIS_STATUS_PENDING) \
        dbglevel = RT_DEBUG_TRACE;                                      \
    else                                                                \
        dbglevel = RT_DEBUG_ERROR;                                      \
    DBGPRINT(dbglevel, Fmt);                                            \
}
#else
#define DBGPRINT(Level, Fmt)
#define DBGPRINT_RAW(Level, Fmt)
#define DBGPRINT_S(Status, Fmt)
#define DBGPRINT_ERR(Fmt)
#define FUNC_ENTER
#define FUNC_LEAVE
#define FUNC_LEAVE_STATUS(_S) 
#define MTWF_LOG(Category, SubCategory, Level, Fmt) 
#endif

#ifdef DBG
#define DumpFrame( pStart, DumpLen )    \
{                               \
    do                          \
    {                           \
            ULONG index = 0;            \
            DBGPRINT(RT_DEBUG_TRACE, ("[#RT2870#] "__FUNCTION__"\n"));  \
            for( index = 0; index<DumpLen;index = index+8)                  \
            {           \
            DBGPRINT(RT_DEBUG_TRACE, ("0x%02X-0x%02X-0x%02X-0x%02X-0x%02X-0x%02X-0x%02X-0x%02X \n",*(pStart+index),*(pStart+index+1),*(pStart+index+2),*(pStart+index+3),*(pStart+index+4),*(pStart+index+5),*(pStart+index+6),*(pStart+index+7))); \
            }           \
    }while(0);      \
}

#define DumpFrameMessage( pStart, DumpLen, fmt )    \
{                               \
    DBGPRINT(RT_DEBUG_TRACE, fmt);                  \
    do                          \
    {                           \
            ULONG index = 0;            \
            DBGPRINT(RT_DEBUG_TRACE, ("[#RT2870#] "__FUNCTION__"\n"));  \
            for( index = 0; index<DumpLen;index = index+8)                  \
            {           \
            DBGPRINT(RT_DEBUG_TRACE, ("0x%02X-0x%02X-0x%02X-0x%02X-0x%02X-0x%02X-0x%02X-0x%02X \n",*(pStart+index),*(pStart+index+1),*(pStart+index+2),*(pStart+index+3),*(pStart+index+4),*(pStart+index+5),*(pStart+index+6),*(pStart+index+7))); \
            }           \
    }while(0);      \
}

#define DumpAddress( pStart)    \
{                               \
    do                          \
    {                           \
            ULONG index = 0;            \
            DBGPRINT(RT_DEBUG_TRACE, ("@_@ "__FUNCTION__"\n")); \
                DBGPRINT(RT_DEBUG_TRACE, (" MAC Address: %02X:%02X:%02X:%02X:%02X:%02X \n",*(pStart+index),*(pStart+index+1),*(pStart+index+2),*(pStart+index+3),*(pStart+index+4),*(pStart+index+5))); \
    }while(0);      \
}

#else
#define DumpFrame( pStart, DumpLen )
#define DumpFrameMessage( pStart, DumpLen, fmt )
#define DumpAddress( pStart)
#endif


#define RTMP_DEQUEUE_PACKET(_pAd, _INDEX,_PIPENUM)                          \
{                                                                       \
    if ((!MT_TEST_FLAG(_pAd, fRTMP_ADAPTER_BSS_SCAN_IN_PROGRESS)) &&  \
        (!MT_TEST_FLAG(_pAd, fRTMP_ADAPTER_RADIO_OFF)) &&             \
        (!MT_TEST_FLAG(_pAd, fRTMP_ADAPTER_RESET_IN_PROGRESS)) && \
        (!MT_TEST_FLAG(_pAd, fRTMP_ADAPTER_HALT_IN_PROGRESS)))        \
    {                                                                   \
        for (_INDEX = 0; _INDEX < _PIPENUM; _INDEX++)                   \
        {                                                               \
            if((_pAd)->SendTxWaitQueue[_INDEX].Number > 0)                  \
            {                                                           \
                NdisCommonDeQueuePacket(_pAd, (UCHAR)_INDEX);             \
            }                                                           \
        }                                                               \
    }                                                                   \
}

//
//  Macros for flag and ref count operations
//
#define MT_SET_FLAG(_M, _F)       ((_M)->Flags |= (_F))
#define MT_CLEAR_FLAG(_M, _F)     ((_M)->Flags &= ~(_F))
#define MT_CLEAR_FLAGS(_M)        ((_M)->Flags = 0)
#define MT_TEST_FLAG(_M, _F)      (((_M)->Flags & (_F)) != 0)
#define MT_TEST_FLAGS(_M, _F)     (((_M)->Flags & (_F)) == (_F))

// Flags control for RT2500 USB bulk out frame type
#define MTUSB_SET_BULK_FLAG(_M, _F)             ((_M)->pHifCfg->BulkFlags |= (_F))
#define MTUSB_CLEAR_BULK_FLAG(_M, _F)           ((_M)->pHifCfg->BulkFlags &= ~(_F))
#define MTUSB_TEST_BULK_FLAG(_M, _F)            (((_M)->pHifCfg->BulkFlags & (_F)) != 0)

#define OPSTATUS_SET_FLAG(_pAd, _F)     ((_pAd)->CommonCfg.OpStatusFlags |= (_F))
#define OPSTATUS_CLEAR_FLAG(_pAd, _F)   ((_pAd)->CommonCfg.OpStatusFlags &= ~(_F))
#define OPSTATUS_TEST_FLAG(_pAd, _F)    (((_pAd)->CommonCfg.OpStatusFlags & (_F)) != 0)

#define CLIENT_STATUS_SET_FLAG(_pEntry,_F)      ((_pEntry)->ClientStatusFlags |= (_F))
#define CLIENT_STATUS_CLEAR_FLAG(_pEntry,_F)    ((_pEntry)->ClientStatusFlags &= ~(_F))
#define CLIENT_STATUS_TEST_FLAG(_pEntry,_F)     (((_pEntry)->ClientStatusFlags & (_F)) != 0)

//#define STACFG_SET_FLAG(_pAd, _F)       ((_pAd)->StaCfg.StaCfgFlags |= (_F))
//#define STACFG_CLEAR_FLAG(_pAd, _F)     ((_pAd)->StaCfg.StaCfgFlags &= ~(_F))
//#define STACFG_TEST_FLAG(_pAd, _F)      (((_pAd)->StaCfg.StaCfgFlags & (_F)) != 0)

#define RX_FILTER_SET_FLAG(_pPort, _F)    ((_pPort)->PortCfg.PacketFilter |= (_F))
#define RX_FILTER_CLEAR_FLAG(_pPort, _F)  ((_pPort)->PortCfg.PacketFilter &= ~(_F))
#define RX_FILTER_TEST_FLAG(_pPort, _F)   (((_pPort)->PortCfg.PacketFilter & (_F)) != 0)

#define MT_INC_REF(_A)            NdisInterlockedIncrement (_A)
#define MT_DEC_REF(_A)            NdisInterlockedDecrement(_A)

#define RTMP_OFFSET(field)          ((UINT)FIELD_OFFSET(MP_ADAPTER, field))
#define RTMP_SIZE(field)            sizeof(((PMP_ADAPTER)0)->field)
//#define PORT_OFFSET(field)          ((UINT)FIELD_OFFSET(STA_ADMIN_CONFIG, field))
//#define PORT_SIZE(field)            sizeof(((PSTA_ADMIN_CONFIG)0)->field)

#define REG_CFG_OFFSET(field)    ((UINT)FIELD_OFFSET(MP_ADAPTER, RegistryCfg) + (UINT)FIELD_OFFSET(REGISTRY_CFG, field))
#define REG_CFG_SIZE(field)      sizeof(((PREGISTRY_CFG)0)->field)
#define COMMON_CFG_OFFSET(field)    ((UINT)FIELD_OFFSET(MP_ADAPTER, CommonCfg) + (UINT)FIELD_OFFSET(COMMON_CONFIG, field))
#define STA_CFG_OFFSET(field)       ((UINT)FIELD_OFFSET(MP_ADAPTER, StaCfg) + (UINT)FIELD_OFFSET(STA_ADMIN_CONFIG, field))
#define AP_CFG_OFFSET(field)        ((UINT)FIELD_OFFSET(MP_ADAPTER, SoftAP) + (UINT)FIELD_OFFSET(SOFT_AP, ApCfg) + (UINT)FIELD_OFFSET(AP_ADMIN_CONFIG, field))
#define COMMON_CFG_SIZE(field)      sizeof(((PCOMMON_CONFIG)0)->field)
#define STA_CFG_SIZE(field)         sizeof(((PSTA_ADMIN_CONFIG)0)->field)
#define AP_CFG_SIZE(field)          sizeof(((PAP_ADMIN_CONFIG)0)->field)
//MFP
#define PMF_CFG_OFFSET(field)       ((UINT)FIELD_OFFSET(MP_ADAPTER, StaCfg) + (UINT)FIELD_OFFSET(STA_ADMIN_CONFIG, PmfCfg) + (UINT)FIELD_OFFSET(PMF_CFG, field))
#define PMF_CFG_SIZE(field)         sizeof(((PPMF_CFG)0)->field)

#define TX_BF_CTRL_OFFSET(field)    ((UINT)FIELD_OFFSET(MP_ADAPTER, CommonCfg) + (UINT)FIELD_OFFSET(COMMON_CONFIG, TxBfCtrl) + (UINT)FIELD_OFFSET(TRANSMIT_BEAMFORMING_CONTROL, field))
#define TX_BF_CTRL_SIZE(field)      sizeof(((PTRANSMIT_BEAMFORMING_CONTROL)0)->field)

//LogoTestCfg
#define LOGO_TEST_CFG_OFFSET(field)     ((UINT)FIELD_OFFSET(MP_ADAPTER, LogoTestCfg) + (UINT)FIELD_OFFSET(LOGO_TEST_CONFIG, field))
#define LOGO_TEST_CFG_SIZE(field)       sizeof(((PLOGO_TEST_CONFIG)0)->field)

//HwCfg
#define HW_CFG_OFFSET(field)     ((UINT)FIELD_OFFSET(MP_ADAPTER, HwCfg) + (UINT)FIELD_OFFSET(HW_CONFIG_T, field))
#define HW_CFG_SIZE(field)       sizeof(((PHW_CONFIG_T)0)->field)

//
// New multi-bssid
//
#define IS_NEW_MULTI_BSSID(pAd) (0)

//WSC 2.0 test bed
#define IS_ENABLE_WSC20TB_Version57(_pAd)                   ((((_pAd)->StaCfg.WSC20Testbed) & 0x00000001) == 0x00000001)
#define IS_ENABLE_WSC20TB_2_WSCIEs_IN_ProbReq(_pAd)         ((((_pAd)->StaCfg.WSC20Testbed) & 0x00000002) == 0x00000002)
#define IS_DISABLE_WSC20TB_RSNIE(_pAd)                      ((((_pAd)->StaCfg.WSC20Testbed) & 0x00000004) == 0x00000004)    //00-> no RSN IE, 01->RSN IE, others-> auto
#define IS_ENABLE_WSC20TB_WSCIE(_pAd)                       ((((_pAd)->StaCfg.WSC20Testbed) & 0x00000008) == 0x00000008)
#define IS_ENABLE_SEND_WSC_EAP_FRAGMENTED(_pAd)             ((((_pAd)->StaCfg.WSC20Testbed) & 0x00000010) == 0x00000010)    //Enable sending WSC EAP packet fragmented when doing WPS for test case 4.2.10.

//
// Dual-band NIC (RF/BBP/MAC are in the same chip.)
//
#define IS_RT_NEW_DUAL_BAND_NIC(_pAd) (0)
//
// Is the NIC dual-band NIC?
//
#define IS_DUAL_BAND_NIC(_pAd) (IS_RT_NEW_DUAL_BAND_NIC(_pAd))

//
// Support AGS (Adaptive Group Switching)
//
#define SUPPORT_AGS(_pAd) (0) //(IS_RT6X9X(_pAd))

//
// The NIC is enabled/disabled the explicit TxBf
//
// Unify ToDo
#define isEnableETxBf(_pAd)(0 && (_pAd->HwCfg.TxBfCtrl.ETxBfEn == 1))

//
// Customer Feature setting
//
// bit 0: BT/WiFi Single LED Control
// bit 31~1: not used
// 
#define IS_FEATURE_SETTING_BT_WIFI_SINGLE_LED(_pAd)         ((_pAd)->CommonCfg.FeatureSetting & 0x00000001)    

#ifdef MULTI_CHANNEL_SUPPORT
typedef enum _MULTI_CH_OP_MODE
{
    MULTI_CH_OP_MODE_INFRA = 0, 
    MULTI_CH_OP_MODE_MSCLI = 1,
    MULTI_CH_OP_MODE_MSGO = 2
} MULTI_CH_OP_MODE;

typedef enum _DEFER_REQUEST_OID_TYPE
{
    PENDED_DISCONNECT_OID_REQUEST = 0,
    PENDED_START_GO_OID_REQUEST = 1,
    PENDED_STOP_GO_OID_REQUEST = 2,
    PENDED_RESET_OID_REQUEST = 3,
    PENDED_SCAN_OID_REQUEST = 4
} DEFER_REQUEST_OID_TYPE;
#endif /*MULTI_CHANNEL_SUPPORT*/

#define INC_RING_INDEX(_idx, _RingSize)    \
{                                          \
    (_idx)++;                              \
    if ((_idx) >= (_RingSize)) _idx=0;     \
}

// Increase TxTsc value for next transmission
// TODO: 
// When i==6, means TSC has done one full cycle, do re-keying stuff follow specs
// Should send a special event microsoft defined to request re-key
#define INC_TX_TSC(_tsc)                                \
{                                                       \
    int i=0;                                            \
    while (++_tsc[i] == 0x0)                            \
    {                                                   \
        i++;                                            \
        if (i == 6)                                     \
            break;                                      \
    }                                                   \
}

// StaActive.SupportedHtPhy.MCSSet is copied from AP beacon.  Don't need to update here.
#define COPY_HTSETTINGS_FROM_MLME_AUX_TO_ACTIVE_CFG(_pAd)                                 \
{                                                                                       \
    _pAd->StaActive.SupportedHtPhy.HtChannelWidth = _pAd->MlmeAux.HtCapability.HtCapInfo.ChannelWidth;      \
    _pAd->StaActive.SupportedHtPhy.MimoPs = _pAd->MlmeAux.HtCapability.HtCapInfo.MimoPs;      \
    _pAd->StaActive.SupportedHtPhy.GF = _pAd->MlmeAux.HtCapability.HtCapInfo.GF;      \
    _pAd->StaActive.SupportedHtPhy.ShortGIfor20 = _pAd->MlmeAux.HtCapability.HtCapInfo.ShortGIfor20;      \
    _pAd->StaActive.SupportedHtPhy.ShortGIfor40 = _pAd->MlmeAux.HtCapability.HtCapInfo.ShortGIfor40;      \
    _pAd->StaActive.SupportedHtPhy.TxSTBC = _pAd->MlmeAux.HtCapability.HtCapInfo.TxSTBC;      \
    _pAd->StaActive.SupportedHtPhy.RxSTBC = _pAd->MlmeAux.HtCapability.HtCapInfo.RxSTBC;      \
    _pAd->StaActive.SupportedHtPhy.ExtChanOffset = _pAd->MlmeAux.AddHtInfo.AddHtInfo.ExtChanOffset;      \
    _pAd->StaActive.SupportedHtPhy.RecomWidth = _pAd->MlmeAux.AddHtInfo.AddHtInfo.RecomWidth;      \
    _pAd->StaActive.SupportedHtPhy.OperaionMode = _pAd->MlmeAux.AddHtInfo.AddHtInfo2.OperaionMode;      \
    _pAd->StaActive.SupportedHtPhy.NonGfPresent = _pAd->MlmeAux.AddHtInfo.AddHtInfo2.NonGfPresent;      \
}

#define COPY_AP_HTSETTINGS_FROM_BEACON(_pEntry, _pHtCapability)                                 \
{                                                                                       \
    _pEntry->AMsduSize = (UCHAR)(_pHtCapability->HtCapInfo.AMsduSize);   \
    _pEntry->MmpsMode= (UCHAR)(_pHtCapability->HtCapInfo.MimoPs);    \
    _pEntry->MaxRAmpduFactor = (UCHAR)(_pHtCapability->HtCapParm.MaxRAmpduFactor);   \
}

//
// Macros for mapping Channel ID, KHz, MHz
//
#define     MAP_CHANNEL_ID_TO_KHZ(ch, khz)  {               \
                switch (ch)                                 \
                {                                           \
                    case 1:     khz = 2412000;   break;     \
                    case 2:     khz = 2417000;   break;     \
                    case 3:     khz = 2422000;   break;     \
                    case 4:     khz = 2427000;   break;     \
                    case 5:     khz = 2432000;   break;     \
                    case 6:     khz = 2437000;   break;     \
                    case 7:     khz = 2442000;   break;     \
                    case 8:     khz = 2447000;   break;     \
                    case 9:     khz = 2452000;   break;     \
                    case 10:    khz = 2457000;   break;     \
                    case 11:    khz = 2462000;   break;     \
                    case 12:    khz = 2467000;   break;     \
                    case 13:    khz = 2472000;   break;     \
                    case 14:    khz = 2484000;   break;     \
                    case 36:  /* UNII */  khz = 5180000;   break;     \
                    case 40:  /* UNII */  khz = 5200000;   break;     \
                    case 44:  /* UNII */  khz = 5220000;   break;     \
                    case 48:  /* UNII */  khz = 5240000;   break;     \
                    case 52:  /* UNII */  khz = 5260000;   break;     \
                    case 56:  /* UNII */  khz = 5280000;   break;     \
                    case 60:  /* UNII */  khz = 5300000;   break;     \
                    case 64:  /* UNII */  khz = 5320000;   break;     \
                    case 149: /* UNII */  khz = 5745000;   break;     \
                    case 153: /* UNII */  khz = 5765000;   break;     \
                    case 157: /* UNII */  khz = 5785000;   break;     \
                    case 161: /* UNII */  khz = 5805000;   break;     \
                    case 165: /* UNII */  khz = 5825000;   break;     \
                    case 167: /* UNII */  khz = 5835000;   break;     \
                        case 169: /* UNII */  khz = 5845000;   break;     \
                    case 171: /* UNII */  khz = 5855000;   break;     \
                        case 173: /* UNII */  khz = 5865000;   break;     \
                    case 100: /* HiperLAN2 */  khz = 5500000;   break;     \
                    case 104: /* HiperLAN2 */  khz = 5520000;   break;     \
                    case 108: /* HiperLAN2 */  khz = 5540000;   break;     \
                    case 112: /* HiperLAN2 */  khz = 5560000;   break;     \
                    case 116: /* HiperLAN2 */  khz = 5580000;   break;     \
                    case 120: /* HiperLAN2 */  khz = 5600000;   break;     \
                    case 124: /* HiperLAN2 */  khz = 5620000;   break;     \
                    case 128: /* HiperLAN2 */  khz = 5640000;   break;     \
                    case 132: /* HiperLAN2 */  khz = 5660000;   break;     \
                    case 136: /* HiperLAN2 */  khz = 5680000;   break;     \
                    case 140: /* HiperLAN2 */  khz = 5700000;   break;     \
                    case 34:  /* Japan MMAC */   khz = 5170000;   break;   \
                    case 38:  /* Japan MMAC */   khz = 5190000;   break;   \
                    case 42:  /* Japan MMAC */   khz = 5210000;   break;   \
                    case 46:  /* Japan MMAC */   khz = 5230000;   break;   \
                    default:    khz = 2412000;   break;     \
                }                                           \
            }

#define     MAP_KHZ_TO_CHANNEL_ID(khz, ch)  {               \
                switch (khz)                                \
                {                                           \
                    case 2412000:    ch = 1;     break;     \
                    case 2417000:    ch = 2;     break;     \
                    case 2422000:    ch = 3;     break;     \
                    case 2427000:    ch = 4;     break;     \
                    case 2432000:    ch = 5;     break;     \
                    case 2437000:    ch = 6;     break;     \
                    case 2442000:    ch = 7;     break;     \
                    case 2447000:    ch = 8;     break;     \
                    case 2452000:    ch = 9;     break;     \
                    case 2457000:    ch = 10;    break;     \
                    case 2462000:    ch = 11;    break;     \
                    case 2467000:    ch = 12;    break;     \
                    case 2472000:    ch = 13;    break;     \
                    case 2484000:    ch = 14;    break;     \
                    case 5180000:    ch = 36;  /* UNII */  break;     \
                    case 5200000:    ch = 40;  /* UNII */  break;     \
                    case 5220000:    ch = 44;  /* UNII */  break;     \
                    case 5240000:    ch = 48;  /* UNII */  break;     \
                    case 5260000:    ch = 52;  /* UNII */  break;     \
                    case 5280000:    ch = 56;  /* UNII */  break;     \
                    case 5300000:    ch = 60;  /* UNII */  break;     \
                    case 5320000:    ch = 64;  /* UNII */  break;     \
                    case 5745000:    ch = 149; /* UNII */  break;     \
                    case 5765000:    ch = 153; /* UNII */  break;     \
                    case 5785000:    ch = 157; /* UNII */  break;     \
                    case 5805000:    ch = 161; /* UNII */  break;     \
                    case 5825000:    ch = 165; /* UNII */  break;     \
                    case 5835000:    ch = 167; /* UNII */  break;     \
                    case 5845000:    ch = 169; /* UNII */  break;     \
                    case 5855000:    ch = 171; /* UNII */  break;     \
                    case 5865000:    ch = 173; /* UNII */  break;     \
                    case 5500000:    ch = 100; /* HiperLAN2 */  break;     \
                    case 5520000:    ch = 104; /* HiperLAN2 */  break;     \
                    case 5540000:    ch = 108; /* HiperLAN2 */  break;     \
                    case 5560000:    ch = 112; /* HiperLAN2 */  break;     \
                    case 5580000:    ch = 116; /* HiperLAN2 */  break;     \
                    case 5600000:    ch = 120; /* HiperLAN2 */  break;     \
                    case 5620000:    ch = 124; /* HiperLAN2 */  break;     \
                    case 5640000:    ch = 128; /* HiperLAN2 */  break;     \
                    case 5660000:    ch = 132; /* HiperLAN2 */  break;     \
                    case 5680000:    ch = 136; /* HiperLAN2 */  break;     \
                    case 5700000:    ch = 140; /* HiperLAN2 */  break;     \
                    case 5170000:    ch = 34;  /* Japan MMAC */   break;   \
                    case 5190000:    ch = 38;  /* Japan MMAC */   break;   \
                    case 5210000:    ch = 42;  /* Japan MMAC */   break;   \
                    case 5230000:    ch = 46;  /* Japan MMAC */   break;   \
                    default:         ch = 1;     break;     \
                }                                           \
            }

#define     MAP_MHZ_TO_CHANNEL_ID(khz, ch)  {               \
                switch (khz)                                \
                {                                           \
                    case 2412:    ch = 1;     break;     \
                    case 2417:    ch = 2;     break;     \
                    case 2422:    ch = 3;     break;     \
                    case 2427:    ch = 4;     break;     \
                    case 2432:    ch = 5;     break;     \
                    case 2437:    ch = 6;     break;     \
                    case 2442:    ch = 7;     break;     \
                    case 2447:    ch = 8;     break;     \
                    case 2452:    ch = 9;     break;     \
                    case 2457:    ch = 10;    break;     \
                    case 2462:    ch = 11;    break;     \
                    case 2467:    ch = 12;    break;     \
                    case 2472:    ch = 13;    break;     \
                    case 2484:    ch = 14;    break;     \
                    case 5180:    ch = 36;  /* UNII */  break;     \
                    case 5200:    ch = 40;  /* UNII */  break;     \
                    case 5220:    ch = 44;  /* UNII */  break;     \
                    case 5240:    ch = 48;  /* UNII */  break;     \
                    case 5260:    ch = 52;  /* UNII */  break;     \
                    case 5280:    ch = 56;  /* UNII */  break;     \
                    case 5300:    ch = 60;  /* UNII */  break;     \
                    case 5320:    ch = 64;  /* UNII */  break;     \
                    case 5745:    ch = 149; /* UNII */  break;     \
                    case 5755:    ch = 151; /* UNII */  break;     \
                    case 5765:    ch = 153; /* UNII */  break;     \
                    case 5785:    ch = 157; /* UNII */  break;     \
                    case 5795:    ch = 159; /* UNII */  break;     \
                    case 5805:    ch = 161; /* UNII */  break;     \
                    case 5825:    ch = 165; /* UNII */  break;     \
                    case 5500:    ch = 100; /* HiperLAN2 */  break;     \
                    case 5510:    ch = 102; /* HiperLAN2 */  break;     \
                    case 5520:    ch = 104; /* HiperLAN2 */  break;     \
                    case 5540:    ch = 108; /* HiperLAN2 */  break;     \
                    case 5550:    ch = 110; /* HiperLAN2 */  break;     \
                    case 5560:    ch = 112; /* HiperLAN2 */  break;     \
                    case 5580:    ch = 116; /* HiperLAN2 */  break;     \
                    case 5590:    ch = 118; /* HiperLAN2 */  break;     \
                    case 5600:    ch = 120; /* HiperLAN2 */  break;     \
                    case 5620:    ch = 124; /* HiperLAN2 */  break;     \
                    case 5630:    ch = 126; /* HiperLAN2 */  break;     \
                    case 5640:    ch = 128; /* HiperLAN2 */  break;     \
                    case 5660:    ch = 132; /* HiperLAN2 */  break;     \
                    case 5670:    ch = 134; /* HiperLAN2 */  break;     \
                    case 5680:    ch = 136; /* HiperLAN2 */  break;     \
                    case 5700:    ch = 140; /* HiperLAN2 */  break;     \
                    case 5170:    ch = 34;  /* Japan MMAC */   break;   \
                    case 5190:    ch = 38;  /* UNII */  break;     \
                    case 5210:    ch = 42;  /* Japan MMAC */   break;   \
                    case 5230:    ch = 46;  /* UNII */  break;     \
                    case 5270:    ch = 54;  /* UNII */  break;     \
                    case 5310:    ch = 62;  /* UNII */  break;     \
                    case 4915:    ch = 183; /* Japan */  break;   \
                    case 4920:    ch = 184; /* Japan */  break;   \
                    case 4940:    ch = 188; /* Japan */  break;   \
                    case 4960:    ch = 192; /* Japan */  break;   \
                    case 4980:    ch = 196; /* Japan */  break;   \
                    case 5040:    ch = 208; /* Japan, means J08 */  break;   \
                    case 5060:    ch = 212; /* Japan, means J12 */  break;   \
                    case 5080:    ch = 216; /* Japan, means J16 */  break;   \
                    default:         ch = 1;     break;     \
                }                                           \
            }

//
//  Some utility macros
//
#ifndef min
#define min(_a, _b)     (((_a) < (_b)) ? (_a) : (_b))
#endif

#ifndef max
#define max(_a, _b)     (((_a) > (_b)) ? (_a) : (_b))
#endif

#define GET_LNA_GAIN(_pAd)  ((_pAd->HwCfg.LatchRfRegs.Channel <= 14) ? (_pAd->HwCfg.BLNAGain) : ((_pAd->HwCfg.LatchRfRegs.Channel <= 64) ? (_pAd->HwCfg.ALNAGain0) : ((_pAd->HwCfg.LatchRfRegs.Channel <= 128) ? (_pAd->HwCfg.ALNAGain1) : (_pAd->HwCfg.ALNAGain2))))
#define GET_LNA_GAIN_EXT(_pAd, _Channel) ((_Channel <= 14) ? (_pAd->HwCfg.BLNAGainExt) : ((_Channel <= _pAd->HwCfg.ChIdx5GLbMb) ? (_pAd->HwCfg.ALNAGain0Ext) : ((_Channel <= _pAd->HwCfg.ChIdx5GMbHb) ? (_pAd->HwCfg.ALNAGain1Ext) : (_pAd->HwCfg.ALNAGain2Ext))))

#define INC_COUNTER64(Val)          (Val.QuadPart++)

#define INFRA_ON(_p)                ((_p) && OPSTATUS_TEST_FLAG(_p, fOP_STATUS_INFRA_ON))
#define INFRA_OP(_p)                ((_p) && OPSTATUS_TEST_FLAG(_p, fOP_STATUS_INFRA_OP))
#define ADHOC_ON(_p)                ((_p) && OPSTATUS_TEST_FLAG(_p, fOP_STATUS_ADHOC_ON))
#define IDLE_ON(_p)                 ((_p) && (!(OPSTATUS_TEST_FLAG(_p, fOP_STATUS_INFRA_ON | fOP_STATUS_ADHOC_ON))))
#define P2P_REALLY_IN_IDLE(_pAd, _p)   (((_pAd)->pP2pCtrll->P2PCfg.P2PDiscoProvState < P2P_IDLE) && ((_pAd)->pP2pCtrll->P2PCfg.Dpid == DEV_PASS_ID_NOSPEC) && ((_pAd)->PortList[(_pAd)->pP2pCtrll->P2PCfg.PortNumber]->PortSubtype != PORTSUBTYPE_P2PGO) && IS_P2P_SIGMA_OFF(_pAd))


#define NO_SECURITY_ON(_p)                 (_p->PortCfg.WepStatus == Ralink802_11EncryptionDisabled)
#define WEP_ON(_p)                 ((_p->PortCfg.WepStatus == DOT11_CIPHER_ALGO_WEP40) || (_p->PortCfg.WepStatus == DOT11_CIPHER_ALGO_WEP104) || (_p->PortCfg.WepStatus == DOT11_CIPHER_ALGO_WEP))
#define TKIP_ON(_p)            (_p->PortCfg.WepStatus == DOT11_CIPHER_ALGO_TKIP)
#define AES_ON(_p)            (_p->PortCfg.WepStatus == DOT11_CIPHER_ALGO_CCMP)

// Direct Link
#define DLS_ON(_p)          (((_p)->CommonCfg.bDLSCapable) && ((_p)->CommonCfg.DirectMode == DIRECT_LINK_MODE_DLS)) // 802.11n DLS
#define TDLS_ON(_p)         (((_p)->CommonCfg.bDLSCapable) && ((_p)->CommonCfg.DirectMode == DIRECT_LINK_MODE_TDLS))// 802.11z TDLS

//
//   NDIS6: channge the MiniportReserved from PVOID * 2 to PVOID * 4
//          MiniportReserved[0]: Reserved for TxSWQueue.
//          MiniportReserved[1]: Reserved for Net Buffer List complete indication 
//          MiniportReserved[2]: 
//                                  b0-b3: User Priority
//                                  b4-b7: fragment #
//                                  b8-b13: WCID
//                                  b14-b15: uapsd, indicate more data and eosp
//                                  b16-b23: packet source
//                                  b24-b26: uapsd, indicate U-APSD flag and its physical queue ID
//                                  b27-b31: TX rate index
//          MiniportReserved[3]:
//                                  b0-b3: operation mode
//                                  b4-b7: port number

//#define MT_GET_NETBUFFER_MR(_net)                  (&(_net)->MiniportReserved[0]) 
#define MT_GET_NBL_REF_CNT(_NBL)  ((ULONG_PTR)((_NBL)->MiniportReserved[0]))
#define MT_SET_NBL_REF_CNT(_NBL, _COUNT)  (ULONG_PTR)((_NBL)->MiniportReserved[0]) = _COUNT
#define MT_INCREADE_NBL_REF_CNT(_NBL)  \
            {   \
                ULONG_PTR _Count;   \
                _Count = ((ULONG_PTR)((_NBL)->MiniportReserved[0]));    \
                _Count++;   \
                (ULONG_PTR)((_NBL)->MiniportReserved[0]) = _Count;  \
            }

#define MT_INCREADE_NBL_REF_CNT_1(_NBL)  \
            {   \
                ULONG_PTR _Count, _tmpCnt;   \
                _tmpCnt = ((ULONG_PTR)((_NBL)->MiniportReserved[0]));    \
                _Count = _tmpCnt>>8;                \
                _Count++;   \
                (ULONG_PTR)((_NBL)->MiniportReserved[0]) = ((_Count<<8)|(_tmpCnt &0x00ff) );  \
            }

#define MT_DECREADE_NBL_REF_CNT(_NBL)  \
            {   \
                ULONG_PTR _Count;   \
                _Count = ((ULONG_PTR)((_NBL)->MiniportReserved[0]));    \
                _Count--;   \
                (ULONG_PTR)((_NBL)->MiniportReserved[0]) = _Count;  \
                MT_INCREADE_NBL_REF_CNT_1(_NBL);  \
            }

#define MT_SET_NETBUFFER_XCUIDX(_net, _XCUIDX)         ((ULONG_PTR)((_net)->MiniportReserved[2]) = _XCUIDX)
#define MT_GET_NETBUFFER_XCUIDX(_net)         ((ULONG_PTR)((_net)->MiniportReserved[2]))

#define MT_SET_NETBUFFER_LIST(_net, _List)         ((_net)->MiniportReserved[1] = _List)
#define MT_GET_NETBUFFER_LIST(_net)                ((_net)->MiniportReserved[1])
// b0-b3 as User Priority
#define MT_SET_NETBUFFER_UP(_net, _prio)           ((ULONG)(ULONG_PTR)(_net)->MiniportReserved[2] = ((ULONG)(ULONG_PTR)(_net)->MiniportReserved[2] & 0xfffffff0) | (_prio))
#define MT_GET_NETBUFFER_UP(_net)                  ((ULONG)(ULONG_PTR)(_net)->MiniportReserved[2] & 0x0000000f)

// b4-b7 as fragment #
#define MT_SET_NETBUFFER_FRAGMENTS(_net, _num)     ((ULONG)(ULONG_PTR)(_net)->MiniportReserved[2] = ((ULONG)(ULONG_PTR)(_net)->MiniportReserved[2] & 0xffffff0f) | (_num << 4))
#define MT_GET_NETBUFFER_FRAGMENTS(_net)           (((ULONG)(ULONG_PTR)(_net)->MiniportReserved[2] & 0x000000f0) >> 4)

// b8-b13
#define MT_SET_NETBUFFER_WDS(_net, _wdsidx)        ((ULONG)(ULONG_PTR)(_net)->MiniportReserved[2] = ((ULONG)(ULONG_PTR)(_net)->MiniportReserved[2] & 0xffffc0ff) | (_wdsidx << 8))
#define MT_GET_NETBUFFER_WDS(_net)                (((ULONG)(ULONG_PTR)(_net)->MiniportReserved[2] & 0x00003f00) >> 8)
// Used as STA Adhoc mode. It;s the same meaning as RTMP_S(G)ET_PACKET_WDS. just change naming.
#define MT_SET_NETBUFFER_WCID(_net, _wdsidx)         ((ULONG)(ULONG_PTR)(_net)->MiniportReserved[2] = ((ULONG)(ULONG_PTR)(_net)->MiniportReserved[2] & 0xffffc0ff) | (_wdsidx << 8))
#define MT_GET_NETBUFFER_WCID(_net)                (((ULONG)(ULONG_PTR)(_net)->MiniportReserved[2] & 0x00003f00) >> 8)

// b16-b23
#define MT_SET_NETBUFFER_SOURCE(_net, _pktsrc)     ((ULONG)(ULONG_PTR)(_net)->MiniportReserved[2] = ((ULONG)(ULONG_PTR)(_net)->MiniportReserved[2] & 0xff00ffff) | (_pktsrc << 16))
#define MT_GET_NETBUFFER_SOURCE(_net)              (((ULONG)(ULONG_PTR)(_net)->MiniportReserved[2] & 0x00ff0000) >> 16)

// b24-b26 are reserved by UAPSD

// b27-b31
#define MT_SET_NETBUFFER_TXRATE(_net, _rate)       ((ULONG)(ULONG_PTR)(_net)->MiniportReserved[2] = ((ULONG)(ULONG_PTR)(_net)->MiniportReserved[2] & 0x07ffffff) | (_rate << 27))
#define MT_GET_NETBUFFER_TXRATE(_net)              (((ULONG)(ULONG_PTR)(_net)->MiniportReserved[2] & 0xf8000000) >> 27)

// b0-b3
#define MT_SET_NETBUFFER_OPMODE(_net, _mode)       ((ULONG)(ULONG_PTR)(_net)->MiniportReserved[3] = ((ULONG)(ULONG_PTR)(_net)->MiniportReserved[3] & 0xfffffff0) | (_mode))
#define MT_GET_NETBUFFER_OPMODE(_net)              ((ULONG)(ULONG_PTR)(_net)->MiniportReserved[3] & 0x0000000f)

// b4-b7
#define MT_SET_NETBUFFER_PORT(_net, _Port)       ((ULONG)(ULONG_PTR)(_net)->MiniportReserved[3] = ((ULONG)(ULONG_PTR)(_net)->MiniportReserved[3] & 0xffffff0f) | (_Port << 4))
#define MT_GET_NETBUFFER_PORT(_net)             ( ((ULONG)(ULONG_PTR)(_net)->MiniportReserved[3] & 0x000000f0) >> 4)

#if UAPSD_AP_SUPPORT
////////////////////////////////////////
// b14,b15,b24~b26 are reserved by UAPSD

// b14: indicate more data
#define MT_SET_NETBUFFER_MOREDATA(_net, _flg)         ((ULONG)(ULONG_PTR)(_net)->MiniportReserved[2] = ((ULONG)(ULONG_PTR)(_net)->MiniportReserved[2] & 0xffffbfff) | (_flg << 14))
#define MT_GET_NETBUFFER_MOREDATA(_net)    (((ULONG)(ULONG_PTR)(_net)->MiniportReserved[2] & 0x00004000) >> 14)
// b15: indicate EOSP for UAPSD
#define MT_SET_NETBUFFER_EOSP(_net, _flg)       ((ULONG)(ULONG_PTR)(_net)->MiniportReserved[2] = ((ULONG)(ULONG_PTR)(_net)->MiniportReserved[2] & 0xffff7fff) | (_flg << 15))
#define MT_GET_NETBUFFER_EOSP(_net)          (((ULONG)(ULONG_PTR)(_net)->MiniportReserved[2] & 0x00008000) >> 15)
// b24~b26: indicate U-APSD flag and its physical queue ID 
#define MT_SET_NETBUFFER_UAPSD(_net, _flg_uapsd, _que_id)  ((ULONG)(ULONG_PTR)(_net)->MiniportReserved[2] =  ((ULONG)(ULONG_PTR)(_net)->MiniportReserved[2] & 0xf8ffffff) | ((_flg_uapsd << 26) | _que_id << 24))
#define MT_GET_NETBUFFER_UAPSD_QUEID(_net)         (((ULONG)(ULONG_PTR)(_net)->MiniportReserved[2] & 0x03000000) >> 24)
#define MT_GET_NETBUFFER_UAPSD_FLAG(_net)      (((ULONG)(ULONG_PTR)(_net)->MiniportReserved[2] & 0x04000000) >> 26)
#endif

#define MT_SET_RECEIVE_CONTEXT(_NBL, _RecvContext)  (NET_BUFFER_LIST_INFO(_NBL, MediaSpecificInformation) = _RecvContext)

#define MT_SET_NETBUFFERLIST_IN_NBL(_NBL,_NETL)  (*((PMP_LOCAL_RX_PKTS*)&NET_BUFFER_LIST_MINIPORT_RESERVED(_NBL)) = _NETL)
#define MT_GET_NETBUFFERLIST_FROM_NBL(_NBL)      (*((PMP_LOCAL_RX_PKTS*)&NET_BUFFER_LIST_MINIPORT_RESERVED(_NBL)))

#define PKTSRC_NDIS             0xff        
#define PKTSRC_BAR          0xfe
#define PKTSRC_TDLS         0xfd
#define PKTSRC_INTERNAL     0


#define MP_VERIFY_STATE(_P, _State)       \
    ((_P->State == _State))

#define MP_SET_STATE(_P, _State)       \
    ((_P->State = _State))

//
// Macros for assigning and verifying NDIS_OBJECT_HEADER
//
#define MP_ASSIGN_NDIS_OBJECT_HEADER(_header, _type, _revision, _size) \
    (_header).Type = _type; \
    (_header).Revision = _revision; \
    (_header).Size = _size; 

#define MP_VERIFY_NDIS_OBJECT_HEADER(_header, _type, _revision, _size) \
    (((_header).Type == _type) && \
     ((_header).Revision == _revision) && \
     ((_header).Size == _size))

// 
// With NDIS 6.0 header versioning, the driver should allow higher versions
// of structures to be set. This macro verifies that for sets the version is atleast
// the expected one and size is greater or equal to required
//
#define MP_VERIFY_NDIS_OBJECT_HEADER_DEFAULT(_header, _type, _revision, _size) \
    (((_header).Type == _type) && \
     ((_header).Revision >= _revision) && \
     ((_header).Size >= _size))

#ifndef Add2Ptr
#define Add2Ptr(P,I) ((PVOID)((PUCHAR)(P) + (I)))
#endif

// convert timeout in Milliseconds to relative timeout in 100ns units
//   suitable as parameter 5 to KeWaitForSingleObject(..., TimeOut)
#define PPT_SET_RELATIVE_TIMEOUT_IN_MILLISECONDS(VARIABLE, VALUE) (VARIABLE).QuadPart = -( (LONGLONG) (VALUE)*10*1000 )

//
// Check if it is Japan W53(ch52,56,60,64) channel.
//
#define JapanChannelCheck(channel)  ((channel == 52) || (channel == 56) || (channel == 60) || (channel == 64))

//
//SW will init RTS rate based on below rules.
//
//#define INIT_RTS_RATE_TO_9M(_pAd, _pPort) (INFRA_ON(_pPort) && ((READ_PHY_CFG_MCS(_pAd, &_pPort->MacTab.Content[BSSID_WCID].TxPhyCfg) == MCS_1) ||((READ_PHY_CFG_BW(_pAd, &_pPort->MacTab.Content[BSSID_WCID].TxPhyCfg) == BW_20) && (READ_PHY_CFG_MCS(_pAd, &_pPort->MacTab.Content[BSSID_WCID].TxPhyCfg) == MCS_9))))
//#define INIT_RTS_RATE_TO_18M(_pAd, _pPort) (INFRA_ON(_pPort) && (READ_PHY_CFG_BW(_pAd, &_pPort->MacTab.Content[BSSID_WCID].TxPhyCfg) == BW_20) && ((READ_PHY_CFG_MCS(_pAd, &_pPort->MacTab.Content[BSSID_WCID].TxPhyCfg) == MCS_2) || (READ_PHY_CFG_MCS(_pAd, &_pPort->MacTab.Content[BSSID_WCID].TxPhyCfg) == MCS_3)))


#define MAKE_802_3_HEADER(_p, _pMac1, _pMac2, _pType)                   \
{                                                                       \
    PlatformMoveMemory(_p, _pMac1, MAC_ADDR_LEN);                           \
    PlatformMoveMemory((_p + MAC_ADDR_LEN), _pMac2, MAC_ADDR_LEN);          \
    PlatformMoveMemory((_p + MAC_ADDR_LEN * 2), _pType, LENGTH_802_3_TYPE); \
}

// if pData has no LLC/SNAP (neither RFC1042 nor Bridge tunnel), keep it that way.
// else if the received frame is LLC/SNAP-encaped IPX or APPLETALK, preserve the LLC/SNAP field 
// else remove the LLC/SNAP field from the result Ethernet frame
// Patch for WHQL only, which did not turn on Netbios but use IPX within its payload
// Note:
//     _pData & _DataSize may be altered (remove 8-byte LLC/SNAP) by this MACRO
//     _pRemovedLLCSNAP: pointer to removed LLC/SNAP; NULL is not removed
#define CONVERT_TO_802_3(_p8023hdr, _pDA, _pSA, _pData, _DataSize, _pRemovedLLCSNAP)      \
{                                                                       \
    char LLC_Len[2];                                                    \
                                                                        \
    _pRemovedLLCSNAP = NULL;                                            \
    if (PlatformEqualMemory(SNAP_802_1H, _pData, 6)  ||                     \
        PlatformEqualMemory(SNAP_BRIDGE_TUNNEL, _pData, 6))                 \
    {                                                                   \
        PUCHAR pProto = _pData + 6;                                     \
                                                                        \
        if ((PlatformEqualMemory(IPX, pProto, 2) || PlatformEqualMemory(APPLE_TALK, pProto, 2)) &&  \
            PlatformEqualMemory(SNAP_802_1H, _pData, 6))                    \
        {                                                               \
            LLC_Len[0] = (UCHAR)(_DataSize / 256);                      \
            LLC_Len[1] = (UCHAR)(_DataSize % 256);                      \
            MAKE_802_3_HEADER(_p8023hdr, _pDA, _pSA, LLC_Len);          \
        }                                                               \
        else                                                            \
        {                                                               \
            MAKE_802_3_HEADER(_p8023hdr, _pDA, _pSA, pProto);           \
            _pRemovedLLCSNAP = _pData;                                  \
            _DataSize -= LENGTH_802_1_H;                                \
            _pData += LENGTH_802_1_H;                                   \
        }                                                               \
    }                                                                   \
    else                                                                \
    {                                                                   \
        LLC_Len[0] = (UCHAR)(_DataSize / 256);                          \
        LLC_Len[1] = (UCHAR)(_DataSize % 256);                          \
        MAKE_802_3_HEADER(_p8023hdr, _pDA, _pSA, LLC_Len);              \
    }                                                                   \
}

#define RECORD_LATEST_RX_DATA_RATE(_pAd, _pRxD)  
#define SWITCH_AB(_pEntry, _pAA, _pBB)    \
{                                                                           \
    PVOID pCC;                                                          \
    pCC = _pBB;                                                 \
    _pBB = _pAA;                                                 \
    _pAA = pCC;                                                 \
    (_pEntry->NumOfRxPkt)++;                                                 \
}
#define SWITCH_PhyAB(_pAA, _pBB)    \
{                                                                           \
    ULONG   AABasePaHigh;                           \
    ULONG   AABasePaLow;                           \
    ULONG   BBBasePaHigh;                           \
    ULONG   BBBasePaLow;                           \
    BBBasePaHigh = NdisGetPhysicalAddressHigh(_pBB);                                                 \
    BBBasePaLow = NdisGetPhysicalAddressLow(_pBB);                                                 \
    AABasePaHigh = NdisGetPhysicalAddressHigh(_pAA);                                                 \
    AABasePaLow = NdisGetPhysicalAddressLow(_pAA);                                                 \
    NdisSetPhysicalAddressHigh(_pAA, BBBasePaHigh);                                                 \
    NdisSetPhysicalAddressLow(_pAA, BBBasePaLow);                                                 \
    NdisSetPhysicalAddressHigh(_pBB, AABasePaHigh);                                                 \
    NdisSetPhysicalAddressLow(_pBB, AABasePaLow);                                                 \
}

// Enqueue this frame to MLME engine
// We need to enqueue the whole frame because MLME need to pass data type
// information from 802.11 header
#define REPORT_MGMT_FRAME_TO_MLME(_pAd, _pPort,Wcid, _pFrame, _FrameSize, _Rssi0, _Rssi1, _Rssi2, _AntSel)        \
{                                                                                       \
    MlmeEnqueueForRecv(_pAd,_pPort, Wcid, 0, 0, (UCHAR)_Rssi0, (UCHAR)_Rssi1,(UCHAR)_Rssi2, (UCHAR)_AntSel,_FrameSize, _pFrame);   \
}


// INFRA mode- Address 1 - AP, Address 2 - this STA, Address 3 - DA
// ADHOC mode- Address 1 - DA, Address 2 - this STA, Address 3 - BSSID
// Convert back to Ether header. Ahead the Ether header is one SubAmsdu header.
#define CONVERT_2_AMSDU_HEADER(_pAd, _phdr, _80211hdr, _len)                         \
{                                                                               \
    PlatformZeroMemory(_phdr, 14);                          \
    if (INFRA_ON(_pAd))                                                         \
    {                                                                           \
        PlatformMoveMemory(_phdr, _80211hdr->Addr3, 6);                          \
        PlatformMoveMemory((_phdr + 6), _80211hdr->Addr2, 6);                          \
    }                                                                           \
    else                                                                        \
    {                                                                           \
        PlatformMoveMemory(_phdr, _80211hdr->Addr1, 6);                          \
        PlatformMoveMemory((_phdr + 6), _80211hdr->Addr2, 6);                          \
    }                                                                           \
    *(_phdr + 13) = (UCHAR)(_len&0xff);                          \
    *(_phdr + 12) = (UCHAR)(_len>>8);                          \
}
//Need to collect each ant's rssi concurrently
//rssi1 is report to pair2 Ant and rss2 is reprot to pair1 Ant when 4 Ant
#define COLLECT_RX_ANTENNA_AVERAGE_RSSI(_pAd, _rssi1, _rssi2)                   \
{                                                                               \
    SHORT   AvgRssi;                                                            \
    UCHAR   UsedAnt;                                                            \
    if (_pAd->HwCfg.RxAnt.EvaluatePeriod == 0)                                    \
    {                                                                       \
        UsedAnt = _pAd->HwCfg.RxAnt.Pair1PrimaryRxAnt;                            \
        AvgRssi = _pAd->HwCfg.RxAnt.Pair1AvgRssi[UsedAnt];                        \
        if (AvgRssi < 0)                                                    \
            AvgRssi = AvgRssi - (AvgRssi >> 3) + _rssi1;                    \
        else                                                                \
            AvgRssi = _rssi1 << 3;                                          \
        _pAd->HwCfg.RxAnt.Pair1AvgRssi[UsedAnt] = AvgRssi;                        \
    }                                                                       \
    else                                                                    \
    {                                                                       \
        UsedAnt = _pAd->HwCfg.RxAnt.Pair1SecondaryRxAnt;                          \
        AvgRssi = _pAd->HwCfg.RxAnt.Pair1AvgRssi[UsedAnt];                        \
        if ((AvgRssi < 0) && (_pAd->HwCfg.RxAnt.FirstPktArrivedWhenEvaluate))     \
            AvgRssi = AvgRssi - (AvgRssi >> 3) + _rssi1;                    \
        else                                                                \
        {                                                                   \
            _pAd->HwCfg.RxAnt.FirstPktArrivedWhenEvaluate = TRUE;                 \
            AvgRssi = _rssi1 << 3;                                          \
        }                                                                   \
        _pAd->HwCfg.RxAnt.Pair1AvgRssi[UsedAnt] = AvgRssi;                        \
        _pAd->HwCfg.RxAnt.RcvPktNumWhenEvaluate++;                                \
    }                                                                       \
}

#define EnqueueCmd(cmdq, cmdqelmt)      \
{                                       \
    if (cmdq->size == 0)                \
        cmdq->head = cmdqelmt;          \
    else                                \
        cmdq->tail->next = cmdqelmt;    \
    cmdq->tail = cmdqelmt;              \
    cmdqelmt->next = NULL;              \
    cmdq->size++;                       \
}
// Free Tx ring descriptor MACRO
// This can only called from complete function since it will change the IO counters
#define FREE_TX_RING(_p, _b, _t)            \
{                                       \
    (_t)->InUse      = FALSE;           \
    (_t)->LastOne    = FALSE;           \
    (_t)->IRPPending = FALSE;           \
    (_t)->ReadyToBulkOut = FALSE;      \
    (_t)->BulkOutSize= 0;               \
    (_t)->bFullForBulkOut = FALSE;              \
    (_p)->pHifCfg->NextBulkOutIndex[_b] = (((_p)->pHifCfg->NextBulkOutIndex[_b] + 1) % TX_RING_SIZE); \
    MT_DEC_REF(&(_p)->pHifCfg->TxCount); \
}
        
#define FREE_HTTX_RING(_p, _b, _t)          \
{                                       \
    MT_DEC_REF(&(_p)->pHifCfg->TxCount); \
}

#define LOCAL_TX_RING_EMPTY(_p, _i)     (((_p)->TxContext[_i][(_p)->pHifCfg->NextBulkOutIndex[_i]].InUse) == FALSE)

#define MAC_ADDR_EQUAL(pAddr1,pAddr2)           PlatformEqualMemory((PVOID)(pAddr1), (PVOID)(pAddr2), MAC_ADDR_LEN)
#define COPY_MAC_ADDR(Addr1, Addr2)             ETH_COPY_NETWORK_ADDRESS((Addr1), (Addr2))
#define SSID_EQUAL(ssid1, len1, ssid2, len2)    ((len1==len2) && (PlatformEqualMemory(ssid1, ssid2, len1)))

#define     RTMP_GET_SEND_CONTEXT(_NBL)   ((PDOT11_EXTSTA_SEND_CONTEXT) NET_BUFFER_LIST_INFO(_NBL, MediaSpecificInformation))

#if (COMPILE_VISTA_ABOVE(CURRENT_OS_NDIS_VER))
// the NB might be from the CCX pool, we shall to release them to the right source
#define RELEASE_NDIS_NETBUFFER(_pAd, _pXcu, _Status)                               \
{                                                                                       \
    PNET_BUFFER_LIST    NetBufferList;                                                  \
    PNET_BUFFER    _NetBuffer;                                                               \
    PMP_LOCAL_RX_PKTS pLocalNetBufferList = NULL;                              \
    NetBufferList = ((PMT_XMIT_CTRL_UNIT)_pXcu)->Reserve1;                                \
    _NetBuffer = ((PMT_XMIT_CTRL_UNIT)_pXcu)->Reserve2;                                      \
    if (((PMT_XMIT_CTRL_UNIT)_pXcu)->PktSource == PKTSRC_NDIS)                          \
    {                                                                                   \
        if ((NET_BUFFER_NEXT_NB(_NetBuffer) == NULL) && (NetBufferList != NULL) )                                        \
        {                                                                               \
            NET_BUFFER_LIST_STATUS(NetBufferList) = _Status;                            \
            NdisMSendNetBufferListsComplete(_pAd->AdapterHandle, NetBufferList, 0);     \
            MT_DEC_REF(&_pAd->Counter.MTKCounters.PendingNdisPacketCount);     \
            MT_DEC_REF(&_pAd->pTxCfg->NumQueuePacket);                            \
            _pAd->Counter.MTKCounters.TotalSendNdisPacketCount++;                            \
            MT_SET_NETBUFFER_LIST(_NetBuffer, 0);                    \
            MT_SET_NETBUFFER_XCUIDX(_NetBuffer, 0);                  \
        }                                                                               \
    }                                                                                   \
    else                                                                                \
    {                                                                                   \
        pLocalNetBufferList = MT_GET_NETBUFFERLIST_FROM_NBL(NetBufferList);           \
        NdisAcquireSpinLock(&_pAd->pTxCfg->LocalNetBufferListQueueLock);                        \
        if (pLocalNetBufferList != NULL)                                                \
            InsertTailQueue(&_pAd->pTxCfg->LocalNetBufferListQueue, pLocalNetBufferList);       \
        NdisReleaseSpinLock(&_pAd->pTxCfg->LocalNetBufferListQueueLock);                        \
    }                                                                                   \
    NdisAcquireSpinLock(&_pAd->pTxCfg->XcuIdelQueueLock);        \
    if(_pXcu != NULL)                                                                   \
    {                                                                                           \
        InsertTailQueue(&_pAd->pTxCfg->XcuIdelQueue, _pXcu);      \
    }                                                                                           \
    NdisReleaseSpinLock(&_pAd->pTxCfg->XcuIdelQueueLock);    \
}

#endif

#define MlmeAllocateMemory(_pAd, _pOutBuffer)   MlmeAllocateMemoryEx(_pAd, DRIVER_FILE + __LINE__ ,_pOutBuffer)
#define MLME_MEMORY_2_PTR(P,I) ((PVOID)((PUCHAR)(P) - (I)))

/**
 * This is read write lock state
 */
typedef struct _MP_RW_LOCK_STATE
{
    /** Old Read/Write lock state. Used with NDIS 6.2*/
    LOCK_STATE          OldLockState;

//#ifdef ENABLE_NDIS62_LOCK
    /** New Read/Write lock state */
 //   LOCK_STATE_EX       NewLockState;
//#endif
    /** My variable tracking current lock state */
    BOOLEAN             ReadLock;
    
}MP_RW_LOCK_STATE, *PMP_RW_LOCK_STATE;

typedef struct _CmdQElmt    {
    UINT                command;
    NDIS_PORT_NUMBER    PortNum;
    PVOID               buffer;
    ULONG               bufferlength;
    BOOLEAN             CmdFromNdis;
    BOOLEAN             SetOperation;
    USHORT                 Seq;
    struct _CmdQElmt    *next;
}   CmdQElmt, *PCmdQElmt;

typedef struct  _CmdQ   {
    UINT        size;
    CmdQElmt    *head;
    CmdQElmt    *tail;
}   CmdQ, *PCmdQ;

////////////////////////////////////////////////////////////////////////////
// The TX_BUFFER structure forms the transmitted USB packet to the device
////////////////////////////////////////////////////////////////////////////
typedef struct __TX_BUFFER{
    UCHAR           Aggregation[4];  //Buffer for save Aggregation size.
    union   {
        UCHAR           WirelessPacket[ONE_SEGMENT_UNIT];
        HEADER_802_11   NullFrame;
        PSPOLL_FRAME    PsPollPacket;
        RTS_FRAME       RTSFrame;
    };
} TX_BUFFER, *PTX_BUFFER;

typedef struct __CMD_TX_BUFFER{
    UCHAR           Aggregation[4];  //Buffer for save Aggregation size.
    union   {
        UCHAR           WirelessPacket[MAX_FW_SIZE];
        HEADER_802_11   NullFrame;
        PSPOLL_FRAME    PsPollPacket;
        RTS_FRAME       RTSFrame;
    };
} CMD_TX_BUFFER, *PCMD_TX_BUFFER;

typedef struct __HTTX_BUFFER{
    UCHAR           Aggregation[4];  //Buffer for save Aggregation size.
    union   {
        UCHAR           WirelessPacket[MAX_TXBULK_SIZE];
        HEADER_802_11   NullFrame;
        PSPOLL_FRAME    PsPollPacket;
        RTS_FRAME       RTSFrame;
    };
} HTTX_BUFFER, *PHTTX_BUFFER;

// used to track driver-generated write irps 
typedef struct _TX_CONTEXT
{
    PVOID           pAd;        //Initialized in MiniportInitialize                                 //Initialized in MiniportInitialize
    PTX_BUFFER      TransferBuffer; //Initialized in MiniportInitialize
    ULONG           BulkOutSize;
    UCHAR           BulkOutPipeId;
    BOOLEAN         InUse;
    BOOLEAN         ReadyToBulkOut; // at least one packet is in this TxContext, ready for making IRP anytime.
    BOOLEAN         bFullForBulkOut; // all tx buffer are full , so waiting for tx bulkout.
    BOOLEAN         IRPPending;
    BOOLEAN         LastOne;
    BOOLEAN         bAggregatible;
    ULONG           DataOffset;
    UINT            TxRate;
#if _WIN8_USB_SS_SUPPORTED
#if DBG
    ULONG Index;
#endif
#endif
    UCHAR           Header_802_3[LENGTH_802_3];
    WDFREQUEST          WriteRequestArray[2];
    ULONG           EndPointIndex;
}   TX_CONTEXT, *PTX_CONTEXT, **PPTX_CONTEXT;

// this structure contains all parameters required for recovering packet content from bulkin or bulkout failed.
typedef struct _BULK_FAIL_PARM
{
    BOOLEAN         bHasPending;   // For Bulk IN  failed 
    UCHAR           BulkResetPipeid;    // For Bulk OUT failed 
    UCHAR           BulkInFailRingIdx;   // For Bulk IN  failed 
    UCHAR           IrpIdx[4];  
    ULONG           NextBulkOutPositon[4][MAX_WRITE_USB_REQUESTS];
    ULONG           BulkOutSize[4][MAX_WRITE_USB_REQUESTS]; 
}   BULK_FAIL_PARM, *PBULK_FAIL_PARM;

// used to track driver-generated write irps 
typedef struct _HT_TX_CONTEXT
{
    PVOID           pAd;        //Initialized in MiniportInitialize
    WDFREQUEST      WriteRequestArray[MAX_WRITE_USB_REQUESTS];
    USBD_PIPE_HANDLE  UsbdPipeHandle ;
    WDFMEMORY         UrbMemory;
    WDFUSBPIPE        UsbPipe;
    WDFIOTARGET       IoTarget;
                                    //Initialized in MiniportInitialize
    PHTTX_BUFFER        TransferBuffer; //Initialized in MiniportInitialize
    ULONG           BulkOutSize;
    UCHAR           BulkOutPipeId;
    BOOLEAN         IRPPending[MAX_WRITE_USB_REQUESTS];
    BOOLEAN         LastOne;
    BOOLEAN         bCurWriting;
    BOOLEAN         bCopySavePad;
    ULONG           CurWritePosition;
    ULONG           NextBulkOutPosition;
    ULONG           ENextBulkOutPosition;
    UINT            TxRate;
    UCHAR           AnotherPendingTx;                                       //Initialized in MiniportInitialize
    INT             BulkoutEPCountDown;
    UCHAR           SavedPad[8];
    UCHAR           Header_802_3[LENGTH_802_3];
}   HT_TX_CONTEXT, *PHT_TX_CONTEXT, **PPHT_TX_CONTEXT;

typedef enum _NIC_STATE
{
    NicPaused,
    NicPausing,
    NicRunning
} NIC_STATE;

typedef enum _MP_DOT11_STATE {
    INIT_STATE = 1,
    OP_STATE
} MP_DOT11_STATE, *PMP_DOT11_STATE;

typedef enum _MP_DOT11_RUNNING_MODE {
    RUNNING_MODE_UNKNOWN,
    RUNNING_MODE_STA_INFRASTRUCTURE,
    RUNNING_MODE_STA_ADHOC
} MP_DOT11_RUNNING_MODE, *PMP_DOT11_RUNNING_MODE;

typedef enum _Pendingirp {
    NONEPENDING,
    IRP0PENDING,
    IRP1PENDING
}   PendingIRP;

typedef enum { 
    IRPLOCK_COMPLETED, 
    IRPLOCK_CANCELABLE,
    IRPLOCK_CANCE_START,
    IRPLOCK_CANCE_COMPLETE,
}   IRPLOCK;

//MTK_OID_N5_QUERY_USB_RESET_BULK_OUT will use this to identify which frame causes BULKOUT reset
typedef enum _RTMP_BULKOUT_RESET_FRAME_TYPE
{
    BULKOUT_RESET_TYPE_DATA = 0,
    BULKOUT_RESET_TYPE_MLME,
    BULKOUT_RESET_TYPE_NULLFRAME,
    BULKOUT_RESET_TYPE_RTSFRAME,
    BULKOUT_RESET_TYPE_PSPOLL
}RTMP_BULKOUT_RESET_FRAME_TYPE,*PRTMP_BULKOUT_RESET_FRAME_TYPE;

// ********************************************************************
// WAPI 
// ********************************************************************
//
// Default Length
#define BKID_LEN                    16
#define ADDID_LEN               12
#define NONCE_LEN               32
#define BK_LEN                  16
#define CHALLENGESEED_LEN       32
#define WPTK_LEN                96
#define WGTK_LEN                32
#define WMIC_LEN                20
#define WPIPN_LEN               16
#define IVSEQ_LEN               16
#define ENCRYPT_DATA_LEN        16
#define NMK_LEN                 16
#define PDU_MIC_LEN             16
#define AAD_LEN                 48
#define AUTH_ID_LEN             32
#define MoreByteMICADD          (AAD_LEN-LENGTH_802_11)

// WPTK
#define UEK_LEN                 16
#define UCK_LEN                 16
#define MAK_LEN                 16
#define KEK_LEN                 16

//WGTK
#define MEK_LEN                 16
#define MCK_LEN                 16

typedef enum _APSTA_IBSS_BLOCK_ASUE
{
    WAPI_IBSS_NOBLOCK = 0,
    WAPI_IBSS_BLOCK_ASUE_ACTION,    // If we are in Fake-AE mode, will block ASUE action till finishing AE's action. Then change status to WAPI_IBSS_ACCEPT_ASUE_ACTION
    WAPI_IBSS_ACCEPT_ASUE_ACTION,
    WAPI_IBSS_AE_START_FIRST
 }APSTA_IBSS_BLOCK_ASUE,*PAPSTA_IBSS_BLOCK_ASUE;
 
typedef struct _APSTA_BKSA_INFO{
    ULONG       Wapi_AuthMode;
    UCHAR       Wapi_UserKey[64];
    UCHAR       Wapi_UserKeyLen;    
    UCHAR       Wapi_BKID[BKID_LEN];
    UCHAR       Wapi_ADDID[ADDID_LEN];
    UCHAR       Wapi_BK[BK_LEN];
    UCHAR       Wapi_ChallengeSeed[CHALLENGESEED_LEN];      
    ULONG       Wapi_Lifetime;
    UCHAR       Wapi_AKMCount;
    UCHAR       Wapi_AKM[2];
    UCHAR       Wapi_WAPIIE[256];
    UCHAR       Wapi_WAPIIE_Len;
    BOOLEAN     Wapi_IBSS_IsAE;
    UCHAR       Wapi_IBSS_Block_ASUE; // 0-> noblock 1->block 2->finished Fake-AE

    // WAPI-CERT
    UCHAR       Wapi_AuthID[AUTH_ID_LEN];
}APSTA_BKSA_INFO,*PAPSTA_BKSA_INFO;

typedef struct _APSTA_USKSA_INFO{
    UCHAR   Wapi_USKID;
    UCHAR   Wapi_CacheUSKID; // For rekey   
    UCHAR   Wapi_UnicastKey[WPTK_LEN];
    UCHAR   Wapi_UnicastKey_AdhocTemp[WPTK_LEN];    
    UCHAR   Wapi_PairwiseCipherSuite;
    ULONG   Wapi_Lifetime;
    UCHAR   Wapi_Flag;
    UCHAR   Wapi_ASUE_N2[NONCE_LEN]; //NONCE_LEN
    UCHAR   Wapi_AE_N1[NONCE_LEN];
    UCHAR   Wapi_PN[IVSEQ_LEN]; 
    UCHAR   Wapi_PN_RX[IVSEQ_LEN];      
    UCHAR   Wapi_DisalbeTxRx; // bit 0 ->Rx, bit 1->Tx
}APSTA_USKSA_INFO,*PAPSTA_USKSA_INFO;

typedef struct _APSTA_MSKSA_INFO{
    BOOLEAN     Wapi_TransmitOrRecieve;
    UCHAR       Wapi_MSK_STAKeyID;
    UCHAR       Wapi_GroupCipherSuite;  
    ULONG       Wapi_Lifetime;
    UCHAR       Wapi_MBcastKey[WGTK_LEN];
    UCHAR       Wapi_MBcastKey_AdhocSend[WGTK_LEN]; 
    UCHAR       Wapi_ADDID[ADDID_LEN];
    UCHAR       Wapi_NMK[NMK_LEN*2]; //But we just 16 bytes
    UCHAR       Wapi_NMK_AdhocSend[NMK_LEN*2]; //But we just 16 bytes   
    UCHAR       Wapi_IVSeq[IVSEQ_LEN];  
    UCHAR       Wapi_IVSeq_RX[IVSEQ_LEN];
    UCHAR       Wapi_IVSeq_Cipher[IVSEQ_LEN];       
    UCHAR       Wapi_DisalbeTxRx; // bit 0 ->Rx, bit 1->Tx  
    UCHAR       Wapi_IBSS_5Ways;
}APSTA_MSKSA_INFO,*PAPSTA_MSKSA_INFO;
//
// ********************************************************************
//
// Structure to keep track of receive packets and buffers to indicate
// receive data to the protocol.
//
typedef struct _RX_CONTEXT
{
    PUCHAR              TransferBuffer; 
    ULONG             TransferbufferLen;    
    PVOID               pAd;
    //These 2 Boolean shouldn't both be 1 at the same time. 
    ULONG               ReadPosOffset;  // number of packets waiting for reordering . 
    BOOLEAN             InUse;  // USB Hardware Occupied. Wait for USB HW to put packet. 
    BOOLEAN             Readable;   // Receive Complete back. OK for driver to indicate receiving packet. 
    BOOLEAN             IRPPending;     // TODO: To be removed
    IRPLOCK             IrpLock;
    NDIS_SPIN_LOCK      RxContextLock;
    //for WDF use
    WDFREQUEST          Request;
    WDFMEMORY           RxWdfMemory;
    WDFUSBPIPE          pipe;
    WDFIOTARGET         IoTarget;
    WDFUSBDEVICE        UsbDevice;
    ULONG               Index;
    ULONG               TransferBufferLength;
}   RX_CONTEXT, *PRX_CONTEXT;

//
// Register set pair for initialzation register set definition
//
typedef struct  _RTMP_REG_PAIR
{
    ULONG   Register;
    ULONG   Value;
}   RTMP_REG_PAIR, *PRTMP_REG_PAIR;

typedef struct  _REG_PAIR
{
    UCHAR   Register;
    UCHAR   Value;
}   REG_PAIR, *PREG_PAIR;

//
// Register set pair for initialzation register set definition
//
typedef struct  _RTMP_RF_REGS
{
    UCHAR   Channel;
    ULONG   R1;
    ULONG   R2;
    ULONG   R3;
    ULONG   R4;
}   RTMP_RF_REGS, *PRTMP_RF_REGS;

typedef struct _FREQUENCY_ITEM {
    UCHAR   Channel;
    USHORT  N;
    USHORT  R;
    USHORT  K;
} FREQUENCY_ITEM, *PFREQUENCY_ITEM;

typedef struct _FP {
        BYTE    N_L;
        BYTE    N_H;
        BYTE    K;
        BYTE    R;
} FP, *PFP;

typedef struct SWITCH_CHANNEL_PARAM {
            // 0x00 0~3
                BYTE            Channel;                // 0        //0
            BYTE            BBPCurrentBW;           // 1
            BYTE            TxPath;             // 2
            BYTE            RxPath;             // 3

            // 0x04 4 ~7
            FP          FrequencyPlan_Xtal; 

            // 0x08 8~11
            BYTE            RfR01;          // 8
            BYTE            RfR39;          // 9
            BYTE            RfR45;          // 10
            BYTE            RfR53;          // 11

            //0x0C  12~15
            BYTE            RfR54;          // 12
            BYTE            RfR55;          // 13
            BYTE            TxRxh20M;           // 14
            BYTE            TxRxAgcFc;          // 15
            
            // 0x10 16 ~19
            BYTE            BbpR25;             // 16
            BYTE            BbpR26;             // 17
            BYTE            BbpR62;             // 18
            BYTE            BbpR63;             // 19

            // 0x14 20~23
            BYTE            BbpR64;         // 20
            BYTE            BbpR72;         // 21
            BYTE            BbpR75;         // 22
            BYTE            BbpR77;             // 23

            // 0x18 24~27
            BYTE            BbpR82;             // 24
            BYTE            BbpR83;         // 25
            BYTE            BbpR109;            // 26
            BYTE            BbpR110;            // 27           

            // 0x1C 28~31
            BYTE            BbpR27;             // 28
            BYTE            BbpR66;             // 29
            BYTE            Dummy0;         // 30
            BYTE            Dummy1;         // 31


            BYTE            FW_RDY;             // 32

} SWITCH_CHANNEL_PARAM, *PSWITCH_CHANNEL_PARAM;
#if 0
//
//  Data buffer for DMA operation, the buffer must be contiguous physical memory
//  Both DMA to / from CPU use the same structure.
//
typedef struct  _RTMP_DMABUF
{
    ULONG                   AllocSize;
    PVOID                   AllocVa;            // TxBuf virtual address
    NDIS_PHYSICAL_ADDRESS   AllocPa;            // TxBuf physical address
} RTMP_DMABUF, *PRTMP_DMABUF;
#endif

typedef struct _MP_LOCAL_RX_PKTS
{
    PQUEUE_ENTRY        Next;
//  UCHAR               Index;
#if (COMPILE_VISTA_ABOVE(CURRENT_OS_NDIS_VER))
    PMDL                Mdl;
    PNET_BUFFER         NetBuffer;
    PNET_BUFFER_LIST    NetBufferList;
    DOT11_EXTSTA_RECV_CONTEXT   Dot11RecvContext;
#else
#endif
    PUCHAR              Data;
//  UCHAR               Dummy[1];
} MP_LOCAL_RX_PKTS, *PMP_LOCAL_RX_PKTS;

typedef struct _RTMP_ADHOC_LIST
{
    PQUEUE_ENTRY                Next;

    DOT11_ASSOCIATION_STATE     AssocState;             //Association state of this peer.
    UCHAR                       Bssid[MAC_ADDR_LEN];    
    UCHAR                       PeerMacAddress[MAC_ADDR_LEN];
    USHORT                      CapabilityInfo;
    USHORT                      BeaconInterval;
    ULONGLONG                   ullHostTimeStamp;
    BOOLEAN                     bKeySet;
} RTMP_ADHOC_LIST, *PRTMP_ADHOC_LIST;

typedef struct _PMKID_CACHE {
    // The number of PMKID candicates.
    ULONG Count;

    // The PMKID candidate list.
    DOT11_BSSID_CANDIDATE Candidate[STA_PMKID_MAX_COUNT];
    
} PMKID_CACHE, PMKID_CACHE;

typedef union   _HEADER_802_11_SEQ{
    struct {
    USHORT          Frag:4;
    USHORT          Sequence:12;
    }   field;
    USHORT           value;
}   HEADER_802_11_SEQ, *PHEADER_802_11_SEQ;

//
//  Data buffer for DMA operation, the buffer must be contiguous physical memory
//  Both DMA to / from CPU use the same structure.
//
typedef struct  _RTMP_REORDERBUF
{
    BOOLEAN         IsFull;
    BOOLEAN         IsAMSDU;
    UCHAR           Header802_3[14];
    HEADER_802_11_SEQ           Sequence;   //support compressed bitmap BA, so no consider fragment in BA
//  UCHAR       DataOffset;
    USHORT      Datasize;
    ULONG                   AllocSize;
    PUCHAR              pBuffer; 
}   RTMP_REORDERBUF, *PRTMP_REORDERBUF;

#if 0
//
// Control block (Descriptor) for all ring descriptor DMA operation, buffer must be 
// contiguous physical memory. NDIS_PACKET stored the binding Rx packet descriptor
// which won't be released, driver has to wait until upper layer return the packet
// before giveing up this rx ring descriptor to ASIC. NDIS_BUFFER is assocaited pair
// to describe the packet buffer. For Tx, NDIS_PACKET stored the tx packet descriptor
// which driver should ACK upper layer when the tx is physically done or failed.
//
typedef struct _RTMP_DMACB
{
    ULONG                   AllocSize;          // Control block size
    PVOID                   AllocVa;            // Control block virtual address
    NDIS_PHYSICAL_ADDRESS   AllocPa;            // Control block physical address
    PNDIS_PACKET pNdisPacket;
    PNDIS_PACKET pNextNdisPacket;

    RTMP_DMABUF             DmaBuf;             // Associated DMA buffer structure
} RTMP_DMACB, *PRTMP_DMACB;

typedef struct _RTMP_TX_BUF
{
    PQUEUE_ENTRY    Next;
    UCHAR           Index;
    ULONG                   AllocSize;          // Control block size
    PVOID                   AllocVa;            // Control block virtual address
    NDIS_PHYSICAL_ADDRESS   AllocPa;            // Control block physical address
} RTMP_TXBUF, *PRTMP_TXBUF;
#endif

typedef struct _RTMP_RX_BUF
{
    BOOLEAN           InUse;
    ULONG               ByBaRecIndex;
    RTMP_REORDERBUF MAP_RXBuf[Max_RX_REORDERBUF];
} RTMP_RXBUF, *PRTMP_RXBUF;

//
//  Statistic counter structure
//
typedef struct _COUNTER_802_3
{
    // General Stats
    ULONG       GoodTransmits;
    LARGE_INTEGER       GoodReceives;
    ULONG       TxErrors;
    ULONG       RxErrors;
    ULONG       RxNoBuffer;

    // Ethernet Stats
    ULONG       RcvAlignmentErrors;
    ULONG       OneCollision;
    ULONG       MoreCollisions;

}   COUNTER_802_3, *PCOUNTER_802_3;

typedef struct _COUNTER_802_11 {
    ULONG           Length;
    LARGE_INTEGER   TransmittedFragmentCount;
    LARGE_INTEGER   LastTransmittedFragmentCount;
    LARGE_INTEGER   MulticastTransmittedFrameCount;
    LARGE_INTEGER   FailedCount;
    LARGE_INTEGER   NoRetryCount;
    LARGE_INTEGER   RetryCount;
    LARGE_INTEGER   MultipleRetryCount;
    LARGE_INTEGER   RTSSuccessCount;
    LARGE_INTEGER   RTSFailureCount;
    LARGE_INTEGER   ACKFailureCount;
    LARGE_INTEGER   FrameDuplicateCount;
    LARGE_INTEGER   ReceivedFragmentCount;
    LARGE_INTEGER   MulticastReceivedFrameCount;
    LARGE_INTEGER   FCSErrorCount;

    // Tx aggregation status
    LARGE_INTEGER TxAggRange1Count;
    LARGE_INTEGER TxAggRange2Count;
    LARGE_INTEGER TxAggRange3Count;
    LARGE_INTEGER TxAggRange4Count;

    /* for PER debug */
    LARGE_INTEGER AmpduFailCount;
    LARGE_INTEGER AmpduSuccessCount;
} COUNTER_802_11, *PCOUNTER_802_11;

#define MAX_NUM_PHY_TYPES                    5
typedef struct _NICSTATISTICSINFO {

    ULONGLONG       RxNoBuf;    // RX no buffer error
    
    // Maintains station side statistics
    ULONGLONG               ullUcastWEPExcludedCount;
    ULONGLONG               ullMcastWEPExcludedCount;
    
    // DOT11_STATISTICS
    ULONGLONG  ullFourWayHandshakeFailures;
    ULONGLONG  ullTKIPCounterMeasuresInvoked;
    DOT11_MAC_FRAME_STATISTICS  UcastCounters;
    DOT11_MAC_FRAME_STATISTICS  McastCounters;
    DOT11_PHY_FRAME_STATISTICS  PhyCounters[MAX_NUM_PHY_TYPES];

} NICSTATISTICSINFO, *PNICSTATISTICSINFO;

typedef struct _COUNTER_MTK {
    ULONG           TransmittedByteCount;   // both successful and failure, used to calculate TX throughput
    ULONG           ReceivedByteCount;      // both CRC okay and CRC error, used to calculate RX throughput
    ULONG       LastReceivedByteCount;
    ULONG           BeenDisassociatedCount;
    ULONG           BadCQIAutoRecoveryCount;
    ULONG           PoorCQIRoamingCount;
    ULONG           MgmtRingFullCount;
    ULONG           RxCount;
    ULONG           RxRingErrCount;
    ULONG           KickTxCount;
    ULONG           TxRingErrCount;
    LARGE_INTEGER   RealFcsErrCount;


    ULONG           OneSecOsTxCount[NUM_OF_TX_RING];
    ULONG           OneSecDmaDoneCount[NUM_OF_TX_RING];
    ULONG           OneSecTxDoneCount;
    ULONG           OneSecTxAggregationCount;
    ULONG           OneSecRxAggregationCount;
    ULONG           OneSecReceivedByteCount;

    ULONG           OneSecTransmittedByteCount;   // both successful and failure, used to calculate TX throughput
    ULONG           OneSecTxNoRetryOkCount;
    ULONG           OneSecTxRetryOkCount;
    ULONG           FiveSecTxNoRetryOkCount;
    ULONG           FiveSecTxRetryOkCount;
    ULONG           FiveSecTxFailCount; 
    ULONG           OneSecTxFailCount;
    ULONG           OneSecFalseCCACnt;      // CCA error count, for debug purpose, might move to global counter
    ULONG           OneSecRxOkCnt;          // RX without error
    ULONG           OneSecRxOkDataCnt;      // unicast-to-me DATA frame count
    ULONG           Accu30SecTotalRxCount; // 
    ULONG           Last30SecTotalRxCount; // 
    ULONG           OneSecRxFcsErrCnt;      // CRC error
    ULONG           OneSecBeaconSentCnt;
    ULONG           LastOneSecTotalTxCount; // OneSecTxNoRetryOkCount + OneSecTxRetryOkCount + OneSecTxFailCount
    ULONG           LastOneSecRxOkDataCnt;  // OneSecRxOkDataCnt
    ULONG           OneSecAggCnt;
    ULONG           OneSecNoAggCnt;
    ULONG           OneSecRxBeaconCnt;

    //
    // LastOneSec count only available for USB FastRate Adaption
    //
    ULONG           LastOneSecTxNoRetryOkCount;
    ULONG           LastOneSecTxRetryOkCount;
    ULONG           LastOneSecTxFailCount;

    //
    // counters for watchdog
    //
    ULONG       TotalSendNdisPacketCount;
    ULONG       TotalSendNdisPacketSnapShot;
    ULONG       PendingNdisPacketCount;
    ULONG       NumStalledSendTicks;

    //
    // Save for USB Releated counter
    //
    ULONG           VendorRequestTimeout;
    ULONG           VendorRequestFail;

} COUNTER_MTK, *PCOUNTER_MTK;

typedef struct _PID_COUNTER {
    ULONG           TxAckRequiredCount;      // CRC error
    ULONG           TxAggreCount;
    ULONG           TxSuccessCount; // OneSecTxNoRetryOkCount + OneSecTxRetryOkCount + OneSecTxFailCount
    ULONG       LastSuccessRate;
} PID_COUNTER, *PPID_COUNTER;

typedef struct _COUNTER_DRS {
    // to record the each TX rate's quality. 0 is best, the bigger the worse.
    USHORT          TxQuality[MAX_STEP_OF_TX_RATE_SWITCH];
    UCHAR           PER[MAX_STEP_OF_TX_RATE_SWITCH];
    UCHAR           TxRateUpPenalty;      // extra # of second penalty due to last unstable condition
    ULONG           CurrTxRateStableTime; // # of second in current TX rate
    BOOLEAN         fNoisyEnvironment;
    BOOLEAN         fLastSecAccordingRSSI;
    UCHAR           LastSecTxRateChangeAction; // 0: no change, 1:rate UP, 2:rate down
    UCHAR           LastTimeTxRateChangeAction; //Keep last time value of LastSecTxRateChangeAction     
    ULONG           LastTxOkCount;
} COUNTER_DRS, *PCOUNTER_DRS;

typedef struct _COUNTER_QA {
    LARGE_INTEGER   CRCErrorCount;
    LARGE_INTEGER   RXOverFlowCount;
    LARGE_INTEGER   PHYErrorCount;
    LARGE_INTEGER   FalseCCACount;
    LARGE_INTEGER   U2MDataCount;
    LARGE_INTEGER   OtherDataCount;
    LARGE_INTEGER   BeaconCount;
    LARGE_INTEGER   othersCount;
}   COUNTER_QA, *PCOUNTER_QA;

//
//  Structure for pended OID query request
//
typedef struct  _RTMP_QUERY_REQUEST
{
    IN  NDIS_OID    Oid;
    IN  PVOID       pInformationBuffer;
    IN  ULONG       InformationBufferLength;
    OUT PULONG      pBytesWritten;
    OUT PULONG      pBytesNeeded;
}   RTMP_QUERY_REQUEST, *PRTMP_QUERY_REQUEST;

//
//  Structure for pended OID set request
//
typedef struct  _RTMP_SET_REQUEST
{
    IN  NDIS_OID    Oid;
    IN  PVOID       pInformationBuffer;
    IN  ULONG       InformationBufferLength;
    OUT PULONG      pBytesRead;
    OUT PULONG      pBytesNeeded;
}   RTMP_SET_REQUEST, *PRTMP_SET_REQUEST;

//
//  Arcfour Structure Added by PaulWu
//
typedef struct  _ARCFOUR
{
    UINT            X;
    UINT            Y;
    UCHAR           STATE[256];
}   ARCFOURCONTEXT, *PARCFOURCONTEXT;

// Shared key data structure
typedef struct  _WEP_KEY {
    UCHAR   KeyLen;                     // Key length for each key, 0: entry is invalid
    UCHAR   Key[MAX_LEN_OF_KEY];        // right now we implement 4 keys, 128 bits max
}   WEP_KEY, *PWEP_KEY;

//WinXP
typedef struct _CIPHER_KEY {
    UCHAR   BssId[6];
    UCHAR   CipherAlg;          // 0-none, 1:WEP64, 2:WEP128, 3:TKIP, 4:AES, 5:CKIP64, 6:CKIP128
    UCHAR   KeyLen;             // Key length for each key, 0: entry is invalid
    UCHAR   Key[16];            // right now we implement 4 keys, 128 bits max
    UCHAR   RxMic[8];
    UCHAR   TxMic[8];
    UCHAR   TxTsc[6];           // 48bit TSC value
    UCHAR   RxTsc[6];           // 48bit TSC value
    UCHAR   Type;               // Indicate Pairwise/Group when reporting MIC error
}   CIPHER_KEY, *PCIPHER_KEY;

typedef struct _CMD_802_11_KEY {
    UINT8      ucAddRemove;
    UINT8      ucTxKey;   /* 1 : Tx key */
    UINT8      ucKeyType; /* 0 : group Key, 1 : Pairwise key */
    UINT8      ucIsAuthenticator; /* 1 : authenticator */
    UINT8      aucPeerAddr[6];
    UINT8      ucBssIndex; /* the BSS index */
    UINT8      ucAlgorithmId;
    UINT8      ucKeyId;
    UINT8      ucKeyLen;
    UINT8      ucWlanIndex;
    UINT8      ucReverved;
    UINT8      aucKeyMaterial[32];
    UINT8      aucKeyRsc[16];
} CMD_802_11_KEY, *P_CMD_802_11_KEY;

typedef struct _PER_STA_CIPHER_KEY {
    UCHAR           StaMacAddr[MAC_ADDR_LEN];
    BOOLEAN         Valid;
    CIPHER_KEY      CipherKey[4];
} PER_STA_CIPHER_KEY, *PPER_STA_CIPHER_KEY;

typedef struct _BBP_TUNING_STRUCT {
    BOOLEAN     Enable;
    UCHAR       FalseCcaCountUpperBound;  // 100 per sec
    UCHAR       FalseCcaCountLowerBound;  // 10 per sec
    UCHAR       R17LowerBound;            // specified in E2PROM
    UCHAR       R17UpperBound;            // 0x68 according to David Tung
    UCHAR       CurrentR17Value;
} BBP_TUNING, *PBBP_TUNING;

typedef struct _SOFT_RX_ANT_DIVERSITY_STRUCT {
    UCHAR     EvaluatePeriod;        // 0:not evalute status, 1: evaluate status, 2: switching status
    UCHAR     Pair1PrimaryRxAnt;     // 0:Ant-E1, 1:Ant-E2
    UCHAR     Pair1SecondaryRxAnt;   // 0:Ant-E1, 1:Ant-E2
    UCHAR     Pair2PrimaryRxAnt;     // 0:Ant-E3, 1:Ant-E4
    UCHAR     Pair2SecondaryRxAnt;   // 0:Ant-E3, 1:Ant-E4
    SHORT     Pair1AvgRssi[2];       // AvgRssi[0]:E1, AvgRssi[1]:E2
    SHORT     Pair2AvgRssi[2];       // AvgRssi[0]:E3, AvgRssi[1]:E4
    SHORT     Pair1LastAvgRssi;      // 
    SHORT     Pair2LastAvgRssi;      // 
    ULONG     RcvPktNumWhenEvaluate;
    BOOLEAN   FirstPktArrivedWhenEvaluate;
} SOFT_RX_ANT_DIVERSITY, *PSOFT_RX_ANT_DIVERSITY;

typedef struct _LEAP_AUTH_INFO {
    BOOLEAN         Enabled;        //Ture: Enable LEAP Authentication
    BOOLEAN         CCKM;           //Ture: Use Fast Reauthentication with CCKM
    UCHAR           Reserve[2];
    UCHAR           UserName[256];  //LEAP, User name
    ULONG           UserNameLen;
    UCHAR           Password[256];  //LEAP, User Password
    ULONG           PasswordLen;
} LEAP_AUTH_INFO, *PLEAP_AUTH_INFO;

typedef struct {
    UCHAR        Addr[MAC_ADDR_LEN];
    UCHAR        ErrorCode[2];   //00 01-Invalid authentication type
                                //00 02-Authentication timeout
                                //00 03-Challenge from AP failed
                                //00 04-Challenge to AP failed
    BOOLEAN      Reported;
} ROGUEAP_ENTRY, *PROGUEAP_ENTRY;

typedef struct {
    UCHAR               RogueApNr;
    ROGUEAP_ENTRY       RogueApEntry[MAX_LEN_OF_BSS_TABLE];
} ROGUEAP_TABLE, *PROGUEAP_TABLE;

typedef struct {
    BOOLEAN     Enable;
    UCHAR       Delta;
    BOOLEAN     PlusSign;
} CCK_TX_POWER_CALIBRATE, *PCCK_TX_POWER_CALIBRATE;

//
// Receive Tuple Cache Format
//
typedef struct  _TUPLE_CACHE    {
    BOOLEAN         Valid;
    UCHAR           MacAddress[MAC_ADDR_LEN];
    USHORT          Sequence; 
    USHORT          Frag;
}   TUPLE_CACHE, *PTUPLE_CACHE;

//
// Fragment Frame structure
//
typedef struct  _FRAGMENT_FRAME {
//  UCHAR       Header802_3[LENGTH_802_3];
//  UCHAR       Header_LLC[LENGTH_802_1_H];
    UCHAR       Buffer[MAX_FRAME_SIZE];  // Add header to prevent NETBUEI continuous buffer isssue
    ULONG       RxSize;
    USHORT      Sequence;
    USHORT      LastFrag;
    ULONG       Flags;          // Some extra frame information. bit 0: LLC presented
}   FRAGMENT_FRAME, *PFRAGMENT_FRAME;

//
// Packet information for NdisQueryPacket
//
typedef struct  _PACKET_INFO    {
    UINT            PhysicalBufferCount;    // Physical breaks of buffer descripor chained
    UINT            BufferCount ;           // Number of Buffer descriptor chained
    UINT            TotalPacketLength ;     // Self explained
    PNDIS_BUFFER    pFirstBuffer;           // Pointer to first buffer descriptor
    PUCHAR          pSrcBufVA;      // 1st buffer VA
}   PACKET_INFO, *PPACKET_INFO;

//
// Tkip Key structure which RC4 key & MIC calculation
//
typedef struct  _TKIP_KEY_INFO  {
    UINT        nBytesInM;  // # bytes in M for MICKEY
    ULONG       IV16;
    ULONG       IV32;   
    ULONG       K0;         // for MICKEY Low
    ULONG       K1;         // for MICKEY Hig
    ULONG       L;          // Current state for MICKEY
    ULONG       R;          // Current state for MICKEY
    ULONG       M;          // Message accumulator for MICKEY
    UCHAR       RC4KEY[16];
    UCHAR       MIC[8];
}   TKIP_KEY_INFO, *PTKIP_KEY_INFO;

//
// Private / Misc data, counters for driver internal use
//
typedef struct  __PRIVATE_STRUC {
    ULONG       SystemResetCnt;         // System reset counter
    ULONG       TxRingFullCnt;          // Tx ring full occurrance number
    ULONG       PhyRxErrCnt;            // PHY Rx error count, for debug purpose, might move to global counter
    // Variables for WEP encryption / decryption in rtmp_wep.c
    ULONG           FCSCRC32;
    ARCFOURCONTEXT  WEPCONTEXT;
    // Tkip stuff
    TKIP_KEY_INFO   Tx;
    TKIP_KEY_INFO   Rx;
    NDIS_SPIN_LOCK  CalMicLock; // protect MIC variables    
}   PRIVATE_STRUC, *PPRIVATE_STRUC;

// structure to tune BBP R66 "RX AGC VGC init"
typedef struct _BBP_R66_TUNING {
    BOOLEAN     bEnable;
    USHORT      FalseCcaLowerThreshold;  // default 100
    USHORT      FalseCcaUpperThreshold;  // default 512
    UCHAR       R66Delta;
    UCHAR       R66CurrentValue;
    BOOLEAN     R66LowerUpperSelect; //Before MlmeCntLinkUp, Used LowerBound or UpperBound as R66 value.   
} BBP_R66_TUNING, *PBBP_R66_TUNING;

// structure to store channel TX power
typedef struct _CHANNEL_TX_POWER {
    UCHAR      Channel;
    BOOLEAN       bEffectedChannel; // For BW 40 operating in 2.4GHz , the "effected channel" is the channel that is covered in 40Mhz.
    CHAR       Power;
    CHAR       Power2;
    CHAR        Power3;
    USHORT     RemainingTimeForUse;     //unit: sec
    UCHAR       Tx0FinePowerCtrl; // Tx0 fine power control in 0.1dB step
    UCHAR       Tx1FinePowerCtrl; // Tx1 fine power control in 0.1dB step
    UCHAR       Tx2FinePowerCtrl; // Tx2 fine power control in 0.1dB step
} CHANNEL_TX_POWER, *PCHANNEL_TX_POWER;

typedef enum _ABGBAND_STATE_ {
    UNKNOWN_BAND,
    BG_BAND,
    A_BAND,
} ABGBAND_STATE;

// Power save method control
typedef union   _PS_CONTROL {
    struct  {
        ULONG       EnablePSinIdle:1;           // Enable radio off when not connect to AP. radio on only when sitesurvey,
        ULONG       DisablePS:1;        // Not going to PowerSaving
        ULONG       rsv4:2;         //
        ULONG       rsv3:1;         // 
        ULONG       rsv2:1;         // 
        ULONG       rsv:26;         // 
    }   field;
    ULONG           word;
}   PS_CONTROL, *PPS_CONTROL;

typedef struct _MLME_MEMORY_ELEM {
    LIST_ENTRY              ListEntry;
    ULONG                   Signature;
    ULONG                   Location;
    UCHAR                   AllocVa[MAX_LEN_OF_MLME_BUFFER];
} MLME_MEMORY_ELEM, *PMLME_MEMORY_ELEM;

typedef struct  _MLME_MEMORY_HANDLER {
    LIST_ENTRY              FreeQueue;
    LIST_ENTRY              BusyQueue;

    NDIS_SPIN_LOCK          LockMemoryHandler;
    ULONG                   TotalNumber;
    LONG                    nFreeCount;
    LONG                    nBusyCount;
    BOOLEAN                 bInitalized;
}   MLME_MEMORY_HANDLER, *PMLME_MEMORY_HANDLER;

typedef struct _MLME_STRUCT {
    // STA state machines
    STATE_MACHINE           CntlMachine;
    STATE_MACHINE           AssocMachine;
    STATE_MACHINE           AuthMachine;
    STATE_MACHINE           AuthRspMachine;
    STATE_MACHINE           SyncMachine;
    STATE_MACHINE           WpaPskMachine;
    STATE_MACHINE           LeapMachine;
    STATE_MACHINE           AironetMachine;
    STATE_MACHINE           DlsMachine;
    STATE_MACHINE           WscMachine;
    STATE_MACHINE           AcmMachine; // for CCXv4 CAC / WMM ACM
    STATE_MACHINE           TdlsMachine;
    STATE_MACHINE           P2PMachine; // for P2P
    STATE_MACHINE           PmfMachine; // for 11w SAQuery

    STATE_MACHINE_FUNC      AssocFunc[ASSOC_FUNC_SIZE];
    STATE_MACHINE_FUNC      AuthFunc[AUTH_FUNC_SIZE];
    STATE_MACHINE_FUNC      AuthRspFunc[AUTH_RSP_FUNC_SIZE];
    STATE_MACHINE_FUNC      SyncFunc[SYNC_FUNC_SIZE];
    STATE_MACHINE_FUNC      WpaPskFunc[WPA_PSK_FUNC_SIZE];
    STATE_MACHINE_FUNC      DlsFunc[DLS_FUNC_SIZE];
    STATE_MACHINE_FUNC      ActFunc[ACT_FUNC_SIZE];
    STATE_MACHINE_FUNC      WscFunc[WSC_FUNC_SIZE];
    STATE_MACHINE_FUNC      TdlsFunc[TDLS_FUNC_SIZE]; // 802.11z TDLS
    STATE_MACHINE_FUNC      P2pFunc[P2P_FUNC_SIZE]; // functions for p2p
    STATE_MACHINE_FUNC      PmfFunc[STA_PMF_FUNC_SIZE]; //// for 11w SAQuery

    // Action 
    STATE_MACHINE           ActMachine;

    // AP state machines
    STATE_MACHINE           ApCntlMachine;
    STATE_MACHINE           ApAssocMachine;
    STATE_MACHINE           ApAuthMachine;
    STATE_MACHINE           ApAuthRspMachine;
    STATE_MACHINE           ApSyncMachine;
    STATE_MACHINE           ApWpaMachine;
    STATE_MACHINE_FUNC      ApAssocFunc[AP_ASSOC_FUNC_SIZE];
    STATE_MACHINE_FUNC      ApAuthFunc[AP_AUTH_FUNC_SIZE];
    STATE_MACHINE_FUNC      ApAuthRspFunc[AP_AUTH_RSP_FUNC_SIZE];
    STATE_MACHINE_FUNC      ApSyncFunc[AP_SYNC_FUNC_SIZE];
    STATE_MACHINE_FUNC      ApWpaFunc[AP_WPA_FUNC_SIZE];

    ULONG                   ChannelQuality;  // 0..100, Channel Quality Indication for Roaming
    ULONGLONG               Now64;           // latch the value of NdisGetCurrentSystemTime()
    ULONG                   ChannelStayCount;           // stay at channel counter

    BOOLEAN                 bRunning;
    NDIS_SPIN_LOCK          TaskLock;
    MLME_QUEUE              Queue;

    UINT                    ShiftReg;

#if(COMPILE_WIN8_ABOVE(CURRENT_OS_NDIS_VER))  
    KDPC    MlmeDpc;
    
#endif
    
    MTK_TIMER_STRUCT     PeriodicTimer;
    MTK_TIMER_STRUCT     MlmeCntLinkDownTimer;
    MTK_TIMER_STRUCT     MlmeCntLinkUpTimer;
    MTK_TIMER_STRUCT     RxAntEvalTimer;
    MTK_TIMER_STRUCT     CarrierDetectTimer;
    MTK_TIMER_STRUCT     WakeTimer;
    MTK_TIMER_STRUCT     TBTTTimer;
    MTK_TIMER_STRUCT     SwTbttTimer;
    MTK_TIMER_STRUCT WHCKCPUUtilCheckTimer;
    MTK_TIMER_STRUCT     ChannelRestoreTimer;
    MTK_TIMER_STRUCT    BeaconTimer;
    MTK_TIMER_STRUCT    ScanTimer;
    MTK_TIMER_STRUCT    ChannelTimer;
    MTK_TIMER_STRUCT    ReSendBulkinIRPsTimer;
    MTK_TIMER_STRUCT    ForceSleepTimer;
    MTK_TIMER_STRUCT    AuthTimer;
    MTK_TIMER_STRUCT    AssocTimer;
    MTK_TIMER_STRUCT    ReassocTimer;
    MTK_TIMER_STRUCT    DisassocTimer;
    MTK_TIMER_STRUCT    ConnectTimer;

    
#ifdef MULTI_CHANNEL_SUPPORT
    MTK_TIMER_STRUCT     MultiChannelTimer;
#endif /*MULTI_CHANNEL_SUPPORT*/

#ifdef WFD_NEW_PUBLIC_ACTION
    MTK_TIMER_STRUCT  P2pMsPublicActionFrameTimer;
    MTK_TIMER_STRUCT  P2pMsTriggerReadTxFifoTimer;
#endif /*WFD_NEW_PUBLIC_ACTION*/

    MTK_WORKITEM            ScanTimeoutWorkitem;
    MTK_WORKITEM            AsicSwitchChannelWorkitem;

    BOOLEAN                 bLowThroughput;
    BOOLEAN                 bEnableAutoAntennaCheck;
    UCHAR                   RealRxPath;
    ULONG                   PeriodicRound;
    ULONG                   OneSecPeriodicRound;
    ULONG                   QuickPeriodicRound;

    UCHAR                   CaliBW20RfR24;
    UCHAR                   CaliBW40RfR24;

    UCHAR                   CaliBW20RfR31;
    UCHAR                   CaliBW40RfR31;

    MLME_MEMORY_HANDLER     MemHandler;         //The handler of the nonpaged memory inside MLME

    NDIS_SPIN_LOCK          MlmeInternalCmdBufLock;
    QUEUE_HEADER            MlmeInternalCmdBufHead;
} MLME_STRUCT, *PMLME_STRUCT;

//
// BT30 Mlme
//
typedef struct
{
    STATE_MACHINE           CntlMachine;
    STATE_MACHINE           AssocMachine;
    STATE_MACHINE           AuthMachine;
    STATE_MACHINE           SyncMachine;
    STATE_MACHINE           WpaMachine;

    STATE_MACHINE_FUNC      AssocFunc[BT_ASSOC_FUNC_SIZE];
    STATE_MACHINE_FUNC      AuthFunc[BT_AUTH_FUNC_SIZE];
    STATE_MACHINE_FUNC      SyncFunc[BT_SYNC_FUNC_SIZE];
    STATE_MACHINE_FUNC      WpaFunc[BT_WPA_FUNC_SIZE];

} BT_MLME_STRUCT, *PBT_MLME_STRUCT;

typedef struct
{
    UCHAR               BssType;
    UCHAR               Ssid[MAX_LEN_OF_SSID];
    UCHAR               SsidLen;
    UCHAR               Bssid[MAC_ADDR_LEN];
    UCHAR               AutoReconnectSsid[MAX_LEN_OF_SSID];
    UCHAR               AutoReconnectSsidLen;
    USHORT              Alg;
    UCHAR               ScanType;
    UCHAR               Channel;
    UCHAR               CentralChannel;
    USHORT              Aid;
    USHORT              CapabilityInfo;
    USHORT              BeaconPeriod;
    USHORT              CfpMaxDuration;
    USHORT              CfpPeriod;
    USHORT              AtimWin;

    // Copy supported rate from desired AP's beacon. We are trying to match
    // AP's supported and extended rate settings.
    UCHAR               SupRate[MAX_LEN_OF_SUPPORTED_RATES];
    UCHAR               ExtRate[MAX_LEN_OF_SUPPORTED_RATES];
    UCHAR               SupRateLen;
    UCHAR               ExtRateLen;
    HT_CAPABILITY_IE        HtCapability;
    UCHAR                   HtCapabilityLen;
    ADD_HT_INFO_IE      AddHtInfo;  // AP might use this additional ht info IE 
    EXT_CAP_ELEMT               ExtCapIE;   // this is the extened capibility IE appreed in MGMT frames. Doesn't need to update once set in Init.
    UCHAR           NewExtChannelOffset;
    //RT_HTVHT_CAPABILITY   SupportedHtPhy;

    // new for QOS
    QOS_CAPABILITY_PARM APQosCapability;    // QOS capability of the current associated AP
    EDCA_PARM           APEdcaParm;         // EDCA parameters of the current associated AP
    QBSS_LOAD_PARM      APQbssLoad;         // QBSS load of the current associated AP

    // new to keep Ralink specific feature
    ULONG               APRalinkIe;
    
    BSS_TABLE           SsidBssTab;     // AP list for the same SSID
    BSS_TABLE           RoamTab;        // AP list eligible for roaming
    ULONG               BssIdx;
    ULONG               RoamIdx;

    BOOLEAN             CurrReqIsFromNdis;

    BOOLEAN             bKeepScanningForConnect;  // Used to break Fast Active Scan.
    // If support radio off in idle mode(didn't connec to AP yet), driver doesn't autorescan so much time. 
    // This flag is to indicate there is a need to do rescan later scheduled time.
    BOOLEAN             bKeepScanningForAutoReconnect;  
    ULONG               KeepScanningCnt;

    MTK_TIMER_STRUCT BeaconTimer;
    MTK_TIMER_STRUCT AuthTimer;
    MTK_TIMER_STRUCT AssocTimer;

    UCHAR LastScanChannel;
    BOOLEAN bScanRemainingChannelList;
    BOOLEAN bActiveScanForHiddenAP;
    BOOLEAN bRetainBSSEntriesInScanTable;
    UCHAR LastValidSSID[MAX_LEN_OF_SSID];
    UCHAR LastValidSSIDLength;
    RT_VHT_CAPABILITY  SupportedVhtPhy;
} BT_MLME_AUX, *PBT_MLME_AUX;

//
// Management ring buffer format
//
typedef struct  _MGMT_STRUC {
    PQUEUE_ENTRY    NoAQEntry;
    BOOLEAN     Valid;
    BOOLEAN     bNDPAnnouncement;
    //PHTTRANSMIT_SETTING pHTTxSetting;
    PPHY_CFG    pMgmtPhyCfg;
    PUCHAR      pBuffer;
    ULONG       Length;
}   MGMT_STRUC, *PMGMT_STRUC;

//
// Management ring buffer format
//
typedef struct  _NOAMGMT_STRUC  {
    MGMT_STRUC  NoAQEntry;
}   NOAMGMT_STRUC, *PNOAMGMT_STRUC;

// structure for radar detection and channel switch
typedef struct _RADAR_DETECT_STRUCT {
    UCHAR       CSCount;            //Channel switch counter
    UCHAR       CSPeriod;           //Channel switch period (beacon count)
    UCHAR       RDCount;            //Radar detection counter
    UCHAR       RDMode;             //Radar Detection mode
    UCHAR       BBPR16;
    UCHAR       BBPR17;
    UCHAR       BBPR18;
    UCHAR       BBPR21;
    UCHAR       BBPR22;
    UCHAR       BBPR64;
    ULONG       InServiceMonitorCount; // unit: sec
} RADAR_DETECT_STRUCT, *PRADAR_DETECT_STRUCT;


typedef enum _REC_BLOCKACK_STATUS
{
    Recipient_NONE,
    Recipient_Accept
} REC_BLOCKACK_STATUS, *PREC_BLOCKACK_STATUS;

typedef enum _ORI_BLOCKACK_STATUS
{
    Originator_NONE,
    Originator_WaitRes,
    Originator_Done,
    Originator_SetAutoAdd,  // HT associated, and wait to have traffic.
    Originator_WaitAdd  // HT associated, and wait to be added if traffic.
} ORI_BLOCKACK_STATUS, *PORI_BLOCKACK_STATUS;

typedef struct  PACKED _BA_ORI_ENTRY{
    UCHAR   Wcid;
    UCHAR   TID;
    UCHAR   BAWinSize;
    UCHAR   Token;
// Sequence is to fill every outgoing QoS DATA frame's sequence field in 802.11 header.
    USHORT      TimeOutValue;
    ORI_BLOCKACK_STATUS  ORI_BA_Status;
    MTK_TIMER_STRUCT ORIBATimer;
    PVOID   pAdapter;
} BA_ORI_ENTRY, *PBA_ORI_ENTRY;

typedef struct PACKED _BA_REC_ENTRY {
    UCHAR   Wcid;
    UCHAR   TID;
    UCHAR   BAWinSize;  // # of MPDU, # of BAed frames
    UCHAR   NumOfRxPkt;
    UCHAR    Curindidx; // the head in the RX reordering buffer
    USHORT      LastIndSeq;
    USHORT      LastRcvSeq;
    USHORT      LastIndSeqAtTimer;
    USHORT      TimeOutValue;
    REC_BLOCKACK_STATUS  REC_BA_Status;
    UCHAR   RxBufIdxUsed;
    // corresponding virtual address for RX reordering packet storage.
    //RTMP_REORDERDMABUF MAP_RXBuf[Max_RX_REORDERBUF]; 
    NDIS_SPIN_LOCK          RxReRingLock;                 // Rx Ring spinlock
    struct _BA_REC_ENTRY *pNext;
    PVOID   pAdapter;
} BA_REC_ENTRY, *PBA_REC_ENTRY;


typedef struct {
    ULONG       numAsRecipient;     // I am recipient of numAsRecipient clients. These client are in the BARecEntry[]
    ULONG       numAsOriginator;    // I am originator of   numAsOriginator clients. These clients are in the BAOriEntry[]
    BA_ORI_ENTRY       BAOriEntry[MAX_LEN_OF_BA_ORI_TABLE];
    BA_REC_ENTRY       BARecEntry[MAX_LEN_OF_BA_REC_TABLE];
} BA_TABLE, *PBA_TABLE;


//For QureyBATableOID use;
typedef struct  PACKED _OID_BA_REC_ENTRY{
    UCHAR   MACAddr[MAC_ADDR_LEN];
    UCHAR   BaBitmap;   // if (BaBitmap&(1<<TID)), this session with{MACAddr, TID}exists, so read BufSize[TID] for BufferSize
    UCHAR   rsv;  
    UCHAR   BufSize[8];
    REC_BLOCKACK_STATUS REC_BA_Status[8];
} OID_BA_REC_ENTRY, *POID_BA_REC_ENTRY;

//For QureyBATableOID use;
typedef struct  PACKED _OID_BA_ORI_ENTRY{
    UCHAR   MACAddr[MAC_ADDR_LEN];
    UCHAR   BaBitmap;  // if (BaBitmap&(1<<TID)), this session with{MACAddr, TID}exists, so read BufSize[TID] for BufferSize, read ORI_BA_Status[TID] for status
    UCHAR   rsv; 
    UCHAR   BufSize[8];
    ORI_BLOCKACK_STATUS  ORI_BA_Status[8];
} OID_BA_ORI_ENTRY, *POID_BA_ORI_ENTRY;

typedef struct _QUERYBA_TABLE{
    OID_BA_ORI_ENTRY       BAOriEntry[32];
    OID_BA_REC_ENTRY       BARecEntry[32];
    UCHAR   OriNum;// Number of below BAOriEntry
    UCHAR   RecNum;// Number of below BARecEntry
} QUERYBA_TABLE, *PQUERYBA_TABLE;

typedef union   _BACAP_STRUC    {
    struct  {
        ULONG       RxBAWinLimit:8;
        ULONG       TxBAWinLimit:8;
        ULONG       AutoBA:1;   // automatically BA     
        ULONG       Policy:2;   // 0: DELAY_BA 1:IMMED_BA  (//BA Policy subfiled value in ADDBA frame)   2:BA-not use
        ULONG       MpduDensity:3;  
        ULONG           AmsduEnable:1;  //Enable AMSDU transmisstion
        ULONG           AmsduSize:1;    // 0:3839, 1:7935 bytes. UINT  MSDUSizeToBytes[]    = { 3839, 7935};
        ULONG           MMPSmode:2; // MIMO power save more, 0:static, 1:dynamic, 2:rsv, 3:mimo enable
        ULONG           bHtAdhoc:1;         // adhoc can use ht rate.
        ULONG           b2040CoexistScanSup:1;      //As Sta, support do 2040 coexistence scan for AP. As Ap, support monitor trigger event to check if can use BW 40MHz.
        ULONG           rsv:4;  
    }   field;
    ULONG           word;
} BACAP_STRUC, *PBACAP_STRUC;

//This structure is for all 802.11n card InterOptibilityTest action. Reset all Num every n second.  (Details see MLMEPeriodic)
typedef struct  _IOT_STRUC  {   
    BOOLEAN         bSendBAR;   // Intel
    BOOLEAN         bRTSLongProtOn;
    BOOLEAN         bLastAtheros;
    BOOLEAN         bNextDisableRxBA;
    BOOLEAN         bCurrentAtheros;
    ULONG           LatchProtect[9];    // 2870 use : latch MAC registe 0x1364~0x1378 0x13E0~0x13E8
    UCHAR           AtherosDIR855;  // limit Rx rate to MCS13 and not use shortGI in BW40   
    BOOLEAN         bIntel4965; // Intel 4965 STA
} IOT_STRUC, *PIOT_STRUC;


/*
// This is the registry setting for 802.11n transmit setting.  Used in advanced page.
typedef union _REG_TRANSMIT_SETTING {
    // non-VHT
    struct {
            ULONG  PhyMode:4;                
            ULONG  MCS:7;     // MCS
            ULONG  BW:1;      //channel bandwidth 20MHz or 40 MHz
            ULONG  ShortGI:1;
            ULONG  STBC:1;    //SPACE 
            ULONG  TRANSNO:2; 
            ULONG  HTMODE:2;  //HTMODE_MM(0), HTMODE_GF(1), Udate to 2 byte. Need to Update default setting and discuss RaUI structure.
            ULONG  EXTCHA:2;
            ULONG  rsv:12;
    } HT_field;

    // VHT
    struct {
            ULONG EnableFixedRate:1;  // Fixed Rate, FALSE: Auto Settings.
            ULONG PhyMode:4;          // MODE_CCK(0), MODE_OFDM(1), MODE_HTMIX(2), MODE_HTGREENFIELD(3), MODE_VHT(4)
            ULONG HTMODE:3;           // HTMODE_MM(0), HTMODE_GF(1), HTMODE_11B(2), HTMODE_11G(3), HTMODE_VHT(4)
            ULONG MCS:7;              // MCS
            ULONG BW:3;               //channel bandwidth 20MHz, 40 MHz, 80 MHz, Or 160 MHz
            ULONG ShortGI:1;
            ULONG STBC:1;             //SPACE 
            ULONG NSS:2;              // 1~3
            ULONG CentralChannelSeg1:8;
            ULONG rsv:2;
    } VHT_field;

    ULONG   word;
} REG_TRANSMIT_SETTING, *PREG_TRANSMIT_SETTING;
*/

// This is the registry setting for 802.11n transmit setting.  Used in advanced page.
typedef union _REG_TRANSMIT_SETTING {
    // non-VHT
    struct {
            ULONG PhyMode:4;    // MODE_CCK(0), MODE_OFDM(1), MODE_HTMIX(2), MODE_HTGREENFIELD(3), MODE_VHT(4)
            ULONG MCS:7;        // MCS
            ULONG BW:3;         //channel bandwidth 20MHz, 40 MHz, 80 MHz, Or 160 MHz
            ULONG ShortGI:1;
            ULONG STBC:1;       //SPACE
            ULONG TRANSNO:2;
            ULONG HTMODE:3; // HTMODE_MM(0), HTMODE_GF(1), HTMODE_11B(2), HTMODE_11G(3), HTMODE_VHT(4)
            ULONG EXTCHA:3;     // HT40: 0 below 1 above, VHT80 0~3 Primary 0~3, (TBD VHT160: 0~7 Primary 0~7)
            ULONG NSS:2;        // 1~3
            ULONG rsv:6;
    }field;

    ULONG   word;
} REG_TRANSMIT_SETTING, *PREG_TRANSMIT_SETTING;


#pragma pack(1)

//
//  For OID Query or Set about BA structure
//
typedef struct  _OID_BACAP_STRUC    {
        UCHAR       RxBAWinLimit;
        UCHAR       TxBAWinLimit;
        UCHAR       Policy; // 0: DELAY_BA 1:IMMED_BA  (//BA Policy subfiled value in ADDBA frame)   2:BA-not use. other value invalid
        UCHAR       MpduDensity;    // 0: DELAY_BA 1:IMMED_BA  (//BA Policy subfiled value in ADDBA frame)   2:BA-not use. other value invalid
        UCHAR           AmsduEnable;    //Enable AMSDU transmisstion
        UCHAR           AmsduSize;  // 0:3839, 1:7935 bytes. UINT  MSDUSizeToBytes[]    = { 3839, 7935};
        UCHAR           MMPSmode;   // MIMO power save more, 0:static, 1:dynamic, 2:rsv, 3:mimo enable
        BOOLEAN     AutoBA; // Auto BA will automatically   
} OID_BACAP_STRUC, *POID_BACAP_STRUC;

#pragma pack()

typedef struct {
    BOOLEAN     IsRecipient;
    UCHAR   MACAddr[MAC_ADDR_LEN];
    UCHAR   TID;
    UCHAR   nMSDU;
    USHORT   TimeOut;
    BOOLEAN bAllTid;  // If True, delete all TID for BA sessions with this MACaddr.
} OID_ADD_BA_ENTRY, *POID_ADD_BA_ENTRY;

//
// Pseudo STA configuration when the STA runs in the soft AP mode.
//
typedef struct _AP_PSEUDO_STA_CONFIG {
    ULONG                   AuthMode;   
} AP_PSEUDO_STA_CONFIG, *PAP_PSEUDO_STA_CONFIG;

typedef struct _RT_MESSAGE_EVENT_LOG {
    ULONG       Capable;
    BOOLEAN     Lock;
    BOOLEAN     Reset;   //Clear message
    ULONG       MessageLogLength;
    UCHAR       MessageLog[MAX_MESSAGE_EVENT_LENGTH];
} RT_MESSAGE_EVENT_LOG, *PRT_MESSAGE_EVENT_LOG;

// csr9Enable TRUE, we will use AP mode to sync IBSS Beacon
typedef struct _WIFI_IBSS_SYNC
{
    BCN_TIME_CFG_STRUC      csr9;
    ULONG                   csr9Count;
    BOOLEAN                 csr9Enable;
}WIFI_IBSS_SYNC, *PWIFI_IBSS_SYNC;


typedef union _TGN_CONTROL
{
    struct
    {
        ULONG   IntolerantAssocReq40MHz:1;  //bit0  ->40MHz Intolerant in Association Request 
        ULONG   CoexMgmt40MHzIntolerant1:1; //bit1  ->20/40 BSS coexistence Management frame includes "40MHz intolerant 1" 
        ULONG   CoexMgmt20MHzWidthReq1:1;   //bit2  ->20/40 BSS coexistence Management frame includes "20MHz BSS Width Request 1" 
        ULONG   CoexMgmt40MHzIntolerant0:1; //bit3  ->20/40 BSS coexistence Management frame includes "40MHz intolerant 0" 

        ULONG   CoexMgmt20MHzWidthReq0:1;   //bit4  ->20/40 BSS coexistence Management frame includes "20MHz BSS Width Request 0"       
        ULONG   STBCRXDisable:1;                //bit5  ->[4.2.39 STBC Transmit] On: Force STBC_Rx Disable
        ULONG   CoexIntolerantCh3Report:1;      //bit6  ->Send 20/40 Intolerant Channel Report including channel 3
        ULONG   DisableChangeChBW:1;            //bit7  ->Follow beacon to change channel width

        ULONG   UseWTESupportedMCS:1;       //bit8  ->Use WTESupportedMCS for Supported RX MCS set  
        ULONG   UseWTEMandatoryMCS:1;       //bit9  ->Use WTEMandatoryMCS for Fixed TX Rate
        ULONG   DisableSupportMCS32:1;      //bit10 ->Disable Supported MCS32
        ULONG   TxRxPath11:1;               //bit11 ->Force to 1x1 Device

        ULONG   TxRxPath22:1;               //bit12 ->Force to 2x2 Device   
        ULONG   TxRxPath33:1;               //bit13 ->Force to 3x3 Device
        ULONG   FixedTxSGI:1;                   //bit14 ->Force SGI     
        ULONG   FixedTXGF:1;                    //bit15 ->Force GreenField          
    } field;

    ULONG   word;
}TGN_CONTROL, *PTGN_CONTROL;

//
// Calibration control registry (type: REG_DWORD (32-bit) value)
//
typedef struct _CALIBRATION_CONTROL_REGISTRY
{
    ULONG   EnableInternalAlc:1; // internal ALC (TSSI)
    ULONG   EnableCoefficientAlcRefValue:1; // Coefficient for 2.4 internal ALC reference value (in percentage)
    ULONG   Reserved:30; // reserved
 } CALIBRATION_CONTROL_REGISTRY, *PCALIBRATION_CONTROL_REGISTRY;

//
// The size of the initial calibration parameters in the EEPROM
//
#define SIZE_OF_CALIBRATION_PARAMETERS_IN_EEPROM    5

//
// Transmit beamforming control
//
typedef struct _TRANSMIT_BEAMFORMING_CONTROL
{
    /////////////////////////////////////////////////////////////////////
    // Explicit beamforming parameters
    /////////////////////////////////////////////////////////////////////

    //
    // ETxBfEn=d - Enable/disable the explicit beamforming
    // Values: 0 or 1. Default = 0
    // Description: 0 disables Explicit BF. 1 enables sending of the Sounding packet and Explicit BF of the TX packets.
    //
    ULONG   ETxBfEn;

    //
    // ETxBfTimeout=dddd - Set Explicit BF timeout
    // Values: 0 to 65535. Units are 25 microseconds. Default = 0
    // Description: ETxBF profiles will expire after ETxBFTimeout*25 microseconds. Setting ETxBFTimeout to 0 disables the timeout.
    //
    ULONG   ETxBfTimeout;

    //
    // Explicit non-compressed beamforming feedback capable
    // Values: 0 or 1. Default = 0
    // Description: 0 for compressed beamforming feedback. 1 for non-compressed beamforming feedback.
    //
    ULONG   ETxBfNonCompressedFeedback;

} TRANSMIT_BEAMFORMING_CONTROL, *PTRANSMIT_BEAMFORMING_CONTROL;

//
// GPIO pin-sharing mode
//
typedef enum _GPIO_PIN_SHARING_MODE
{
    GpioPinSharingMode_LowerBound = 0, // Lower bound
    
    //
    // Unconfigurable GPIO mode
    //
    GpioPinSharingMode_DefaultMode = 0,

    //
    // Customer: Arcadyan
    // Chip: 3070
    // Requirements: 
    //  a) GPIO #2: HW WPS PBC
    //  b) GPIO #3: antenna diversity
    // Note: 
    //  a) Not use HW radio on/off
    //  b) Not use any WPS LED mode
    //  c) Not use BT co-existence
    //  d) Not use signal strength control
    //
    GpioPinSharingMode_Mode1 = 1, 

    GpioPinSharingMode_UpperBound = GpioPinSharingMode_Mode1, // upper bound
} GPIO_PIN_SHARING_MODE;

//
// ********************************************************************
// WiFi Direct control

// Suggest Default 0x04308778 for rt2860
// Suggest Default 0x84308778 for rt2870
typedef union       {
    struct  {
        // 1
        ULONG       Enable:1;                   // Enable WiFi Direct.
        ULONG       Managed:1;                  // Support Managed AP
        ULONG       EnablePresistent:1;         // Enable persistent
        ULONG       EnableInvite:1;             // For invitation Test Case.
        // 2
        ULONG       DefaultIntentIndex:4;       // the default intent index GUI used when first editing a profile.
        // 3
        ULONG       ImproveGOScan:1;            // Use P2P power mechanism to improve GO Scan packet loss condition.
        ULONG       ImproveGOScanSTAMulpiPhase:1;           // Use P2P power mechanism to improve GO Scan packet loss condition.
        ULONG       ImproveGOScanConcurrentMulpiPhase:1;            // Use P2P power mechanism to improve GO Scan packet loss condition.
        ULONG       Rsvd3:1;            // Listen Channel
        // 4
        ULONG       DefaultConfigMethod:3;      // Default Config Method that is set in Probe Rsp when P2P On.
        ULONG       EnableIntraBss:1;           //  1: Enable Intra BSS function when I am GO
        // 5
        ULONG       Rsvd2:4;            // default use 2.4GHz channel
        // 6
        ULONG       ClientDiscovery:1;          // Client Discoverbility
        ULONG       ExtendListen:1;             // Extended Listening  
        ULONG       ServiceDiscovery:1;         // Service Discovery
        ULONG       OpPSAlwaysOn:1;             // Service Discovery
        // 7
        ULONG       P2PGroupLimit:1;            // When Limit == 1. GO only support ONE device in my group..
        ULONG       ForceDisableCrossConnect:1;         //
        ULONG       AdvP2PMode:1;       // When ConcurrentMode = 0, disable concurrent mode
        ULONG       Rsvd:4;         // Not Used
        ULONG       SwBasedNoA:1;           // Software Based NoA implementation
    }   field;
    ULONG           word;
}   P2P_CONTROL, *PP2P_CONTROL;

#define P2P_REG_CM_LABEL    1   // Default I broadcast my config method is label. (DefaultConfigMethod:2)
#define P2P_REG_CM_DISPLAY  2   // Default I broadcast my config method is display.
#define P2P_REG_CM_KEYPAD   3   // Default I broadcast my config method is keypad.
#define P2P_REG_CM_PBC  4   // Default I broadcast my config method is pbc. (DefaultConfigMethod:2)
/*
#define CONFIG_METHOD_LABEL         0x0004
#define CONFIG_METHOD_DISPLAY           0x0008
#define CONFIG_METHOD_PUSHBUTTON            0x0080
#define CONFIG_METHOD_KEYPAD            0x0100
*/

// reard/set WPA2PSK AES passphase for GO in OID MTK_OID_N6_SET_P2P_GO_PASSPHRASE
typedef struct _P2P_GO_KEY
{
    CHAR    KeyPassphase[64];     // variable length depending on above field
    ULONG   KeyLength;          // length of key in bytes
} P2P_GO_KEY, *PP2P_GO_KEY;

#if(COMPILE_WIN8_ABOVE(CURRENT_OS_NDIS_VER))  
//
// Win8 NLO Setting
//
#define MAX_NUM_OF_NLO_ENTRY        10
/*
    typedef enum _DOT11_SCAN_METHOD {
        ScanMethodActiveUseSSID     = 0,    // probe packet may use specific SSID
        ScanMethodActiveUseWildcard = 1,    // probe AP should use wildcard
        ScanMethodPassiveListen     = 2     // passively to find APs
    } DOT11_SCAN_METHOD;
*/

/*
    typedef struct DOT11_OFFLOAD_NETWORK {
        DOT11_SSID              Ssid;
        DOT11_CIPHER_ALGORITHM  UnicastCipher;
        DOT11_AUTH_ALGORITHM    AuthAlgo;
        DOT11_CHANNEL_HINT      Dot11ChannelHints[DOT11_MAX_CHANNEL_HINTS];
    } DOT11_OFFLOAD_NETWORK, * PDOT11_OFFLOAD_NETWORK;
*/
typedef struct _NDIS_NLO_ENTRY
{
    BOOLEAN     NLOEnable;      //  NLO Enable or Disable
    ULONG       ulFlags; // DOT11_NLO_FLAG_XXX
    BOOLEAN     IndicateNLO;    //  find candidate, need to indicate NLO event
    ULONG       FastScanPeriod;
    ULONG       FastScanIterations;
    ULONG       SlowScanPeriod;
    ULONG                   uNumOfEntries;
    DOT11_OFFLOAD_NETWORK   OffloadNetwork[MAX_NUM_OF_NLO_ENTRY];       // NLO Entries from OID_DOT11_OFFLOAD_NETWORK_LIST

    CHANNEL_TX_POWER NLOHintedChannels[MAX_NUM_OF_CHANNELS];
    UCHAR NumOfHintedChannels;

    BOOLEAN     bNLOAfterResume; // Perform NLO after resume
    BOOLEAN     bNLOMlmeSyncPeerBeaconFound; // Check MlmeSyncPeerBeacon after resume
    BOOLEAN     bNLOAssocRspToMe; //for USB check Assoc was successed before wake up return
    ULONG       OneSecPeriodicRound;
} NDIS_NLO_ENTRY, *PNDIS_NLO_ENTRY;
#endif

//
// Tx/Rx configuraiton by registry
//
typedef enum TX_RX_CONFIG_BY_REGISTRY
{
    TX_RX_CONFIG_BY_REGISTRY_DISABLE    = 0, 
    TX_RX_CONFIG_BY_REGISTRY_2x3        = 1, // 2x3 NIC
    TX_RX_CONFIG_BY_REGISTRY_MAX        = TX_RX_CONFIG_BY_REGISTRY_2x3, //2 TODO: Change it if add new Tx/Rx configuraiton by registry
} TX_RX_CONFIG_BY_REGISTRY, * PTX_RX_CONFIG_BY_REGISTRY;

typedef struct _RSSI_SAMPLE 
{
    CHAR LastRssi[3]; /* last received RSSI for ant 0~2 */
    CHAR AvgRssi[3];
    SHORT AvgRssiX8[3];
    CHAR LastSnr[3];
    CHAR AvgSnr[3];
    SHORT AvgSnrX8[3];
    //CHAR LastNoiseLevel[3];
} RSSI_SAMPLE;

struct rx_signal_info
{
    CHAR raw_rssi[3];
    UCHAR raw_snr[3];
    CHAR freq_offset;
};

// configuration common to OPMODE_AP as well as OPMODE_STA
typedef struct _COMMON_CONFIG {
    UCHAR       CountryRegion;      // Enum of country region, 0:FCC, 1:IC, 2:ETSI, 3:SPAIN, 4:France, 5:MKK, 6:MKK1, 7:Israel
    UCHAR       CountryRegionForABand;  // Enum of country region for A band
    UCHAR       PhyMode;            // PHY_11XXX
    UCHAR       InitPhyMode;    //use to record the phy mode after reading registry and eeprom.
    UCHAR       LastPhyMode;        // PHY_11XXX
    USHORT      Dsifs;              // in units of usec
    ULONG       PacketFilter;       // Packet filter for receiving  
    USHORT      BeaconPeriod;       
    UCHAR       ChannelToBeSwitched;
    BOOLEAN     bRoamingInProgress;
    USHORT       RoamingCnt;
    
    NDIS_802_11_PRIVACY_FILTER          PrivacyFilter;  // PrivacyFilter enum for 802.1X
                                        //  Otherwise, DOT11_CIPHER_ALGO_IHV_START will be treat as -2147483648

    USHORT                              RsnCapability;
    
    NDIS_802_11_WEP_STATUS              GroupKeyWepStatus;

    UCHAR       SupRate[MAX_LEN_OF_SUPPORTED_RATES];
    UCHAR       SupRateLen;
    UCHAR       ExtRate[MAX_LEN_OF_SUPPORTED_RATES];
    UCHAR       ExtRateLen;
    UCHAR       DesireRate[MAX_LEN_OF_SUPPORTED_RATES];      // OID_802_11_DESIRED_RATES
    UCHAR       MaxDesiredRate;
    UCHAR       ExpectedACKRate[MAX_LEN_OF_SUPPORTED_RATES];

    ULONG       BasicRateBitmap;        // backup basic ratebitmap

    //
    // APSD
    //
    BOOLEAN     bAPSDCapable;
    BOOLEAN     bInServicePeriod;
    BOOLEAN     bAPSDAC_BE;
    BOOLEAN     bAPSDAC_BK;
    BOOLEAN     bAPSDAC_VI;
    BOOLEAN     bAPSDAC_VO;
    UCHAR       MaxSPLength;
    BOOLEAN     bAPSDForcePowerSave;    // Force power save mode, should only use in APSD-STAUT

    //HTTRANSMIT_SETTING    HTPhyMode, MaxHTPhyMode, MinHTPhyMode;
    PHY_CFG TxPhyCfg; // PHY configurations in TXWI
    PHY_CFG MaxPhyCfg;
    PHY_CFG MinPhyCfg;
    
    BW_PRI20_INFO BwPri20Info; // The information of bandwidth and primary 20 location before switching channel
    
    REG_TRANSMIT_SETTING        RegTransmitSetting; //registry transmit setting. this is for reading registry setting only. not useful.
    UCHAR       TxRate;                 // RATE_1, RATE_2, RATE_5_5, RATE_11, ...
    UCHAR       MaxTxRate;              // RATE_1, RATE_2, RATE_5_5, RATE_11
    UCHAR       TxRateIndex;            // Tx rate index in RateSwitchTable
    UCHAR       TxRateTableSize;        // Valid Tx rate table size in RateSwitchTable
    UCHAR       LimitTxRateTableSize;
    BOOLEAN     bAutoTxRateSwitch;
    UCHAR       MinTxRate;              // RATE_1, RATE_2, RATE_5_5, RATE_11
    UCHAR       RtsRate;                // RATE_xxx
    //HTTRANSMIT_SETTING    MlmeTransmit;   // MGMT frame PHY rate setting
    PHY_CFG     MgmtPhyCfg;
    UCHAR       MlmeRate;               // RATE_xxx, used to send MLME frames
    UCHAR       BasicMlmeRate;          // Default Rate for sending MLME frames

    USHORT      RtsThreshold;           // in unit of BYTE
    USHORT      FragmentThreshold;      // in unit of BYTE

    UCHAR       TxPower;                // in unit of mW
    ULONG       TxPowerPercentage;      // 0~100 %
    ULONG       TxPowerDefault;         // keep for TxPowerPercentage
    UCHAR       b11nTxBurstTimeTest;  // For 11n Tx Burst Time Test
    UCHAR       bAdTxPwrForFCCTest;  // For FCC Power Test
    UCHAR           CustomizedPowerMode; // Adjust Tx power mode For Customer , 0: no use, 1: For Asus
    BOOLEAN     bCuzAdTxPwrOn; //Customized Adjust tx power
    UCHAR       CountryTargetTxPowerMode; // Country Target Power Mode: 0: normal, 1: Contry Target Power Table Per rate and including band edge.
    UCHAR       MaxTxPower;
    ULONG       CountryTxPwr;
    ULONG       CountryTxPwr5G;
    USHORT      DefineMaxTxPwr;
    UCHAR       BW2040PwrDelta2G;
    UCHAR       BW2040PwrDelta5G;
    UCHAR       BW80PwrDelta5G;

    BACAP_STRUC        BACapability; //   NO USE = 0XFF  ;  IMMED_BA =1  ;  DELAY_BA=0
    BACAP_STRUC        REGBACapability; //   NO USE = 0XFF  ;  IMMED_BA =1  ;  DELAY_BA=0
    IOT_STRUC       IOTestParm; // 802.11n InterOpbility Test Parameter;
    USHORT      TxPreamble;             // Rt802_11PreambleLong, Rt802_11PreambleShort, Rt802_11PreambleAuto
    BOOLEAN     bUseZeroToDisableFragment;     // Microsoft use 0 as disable 
    ULONG       UseBGProtection;        // 0: auto, 1: always use, 2: always not use
    BOOLEAN     bUseShortSlotTime;      // 0: disable, 1 - use short slot (9us)
    BOOLEAN     bEnableTxBurst;         // 1: enble TX PACKET BURST, 0: disable TX PACKET BURST
    BOOLEAN     bAggregationCapable;      // 1: enable TX aggregation when the peer supports it
    BOOLEAN     bPiggyBackCapable;      // 1: enable TX piggy-back according MAC's version
    BOOLEAN     bIEEE80211H;            // 1: enable IEEE802.11h spec.
    UCHAR       CipherAlg;

    BOOLEAN     bCarrierDetect;         // 1: enable carrier detection
    UCHAR       HitCount;
    BOOLEAN     bCarrierAssert;         //
    BOOLEAN     bCarrierDetectPhase;

    BOOLEAN             bRdg;
    BOOLEAN     bWmmCapable;            // 0:disable WMM, 1:enable WMM, 
                                            // also check NumberOfPipes, should be 5 for WMM supporting.
   // BOOLEAN     bMultipleIRP;           // Multiple Bulk IN flag
    //UCHAR       NumOfBulkInIRP;         // if bMultipleIRP == TRUE, NumOfBulkInIRP will be 4 otherwise be 1

    QOS_CAPABILITY_PARM APQosCapability;    // QOS capability of the current associated AP
    EDCA_PARM           APEdcaParm;         // EDCA parameters of the current associated AP
    QBSS_LOAD_PARM      APQbssLoad;         // QBSS load of the current associated AP
    UCHAR               AckPolicy[4];       // ACK policy of the specified AC. see ACK_xxx
    BOOLEAN             bDLSCapable;        // 0:disable DLS, 1:enable DLS
    ULONG               DirectMode;         // Direct Link Mode, 0: 11n DLS, 1: 11z TDLS (default to DLS)

    // a bitmap of BOOLEAN flags. each bit represent an operation status of a particular 
    // BOOLEAN control, either ON or OFF. These flags should always be accessed via
    // OPSTATUS_TEST_FLAG(), OPSTATUS_SET_FLAG(), OP_STATUS_CLEAR_FLAG() macros.
    // see fOP_STATUS_xxx in RTMP_DEF.C for detail bit definition
    ULONG               OpStatusFlags;

    ABGBAND_STATE       BandState;      // For setting BBP used on B/G or A mode.

    RADAR_DETECT_STRUCT RadarDetect;    
    ULONG                               MaxPktOneTxBulk;
    UCHAR                               TxBulkFactor;
    UCHAR                               RxBulkFactor;
    // >>>======HT ====================================
    RT_HT_CAPABILITY    SupportedHtPhy;
    RT_HT_CAPABILITY    DesiredHtPhy;
    RT_VHT_CAPABILITY   DesiredVhtPhy;

    HT_CAPABILITY_IE        HtCapability;   
    ADD_HT_INFO_IE      AddHTInfo;  // Useful as AP.
    NEW_EXT_CHAN_IE NewExtChanOffset;   //7.3.2.20A,  1 if extension channel is above the control channel, 3 if below, 0 if not present
    SCAN_PARM           ScanParameter;
    ULONG               CountDownCtr;   // CountDown Counter from (Dot11BssWidthTriggerScanInt * Dot11BssWidthChanTranDelayFactor)
    EXT_CAP_ELEMT               ExtCapIE;   // this is the extened capibility IE appreed in MGMT frames. Doesn't need to update once set in Init.
    COEXIST_ELEMT_2040              BSSCoexist2040;
    TRIGGER_EVENT_TAB               TriggerEventTab;
    UCHAR               ChannelListIdx;
    BOOLEAN             bProhibitTKIPonHT;  //the registry either enableTKIP on HT or disable TKIP on HT.

    TGN_CONTROL     TgnControl;
    ULONG               WTESupportedMCS;
    ULONG               WTEMandatoryMCS;    
    ULONG               RxSTBCCount;
    ULONG               TxSTBCCount;    

    // <<<======HT ====================================

    BOOLEAN             PSPXlink;  // 0: Disable. 1: Enable

    UCHAR               BaLimit;    // Block Ack Limitation by HW RAM size

    //
    // Native WiFi
    //
    ULONG                               OperationMode; //DOT11_OPERATION_MODE_EXTENSIBLE_STATION or DOT11_OPERATION_MODE_NETWORK_MONITOR
    
    UINT                    MCAddressCount;
    UCHAR                   MCAddressList[HW_MAX_MCAST_LIST_SIZE][DOT11_ADDRESS_SIZE];

    USHORT                  AtimWindow;
    
    ULONG                   MaxTxLifeTime;
    ULONG                   MaxRxLifeTime;
    

    BOOLEAN                 ExcludeUnencrypted;
    BOOLEAN                 bMediaStreamingEnabled;
    BOOLEAN                 bUnicastUseGroupEnabled;
    BOOLEAN                 bSafeModeEnabled;
    ULONG                   SafeModeQOS_PARAMS;
    UCHAR                   SafeModeRestorePhyMode; 
    BOOLEAN                 bHiddenNetworkEnabled;
    /** 
     * This is generally true and we would be adding the SSID of the network in
     * our probe requests. When we go to sleep, this becomes false, causing us to 
     * stop putting the SSID in our probe requests until we wake up
     */
    BOOLEAN                 bSSIDInProbeRequest;
    BOOLEAN                 bRestoreKey;

    ULONG                   UnreachableDetectionThreshold;
    ULONG                   MPDUMaxLength;


    DOT11_OPERATION_MODE_CAPABILITY     OperationModeCapability;
    DOT11_OPTIONAL_CAPABILITY           OptionalCapability;
    DOT11_CURRENT_OPTIONAL_CAPABILITY   CurrentOptionalCapability;

    // PHY specific MIB
    PDOT11_SUPPORTED_PHY_TYPES          pSupportedPhyTypes;
    ULONG                               SelectedPhyId;          // index of PHY that any PHY specific OID is applied to
    ULONG                               DefaultPhyId;

    UCHAR       SupDo11Rate[MAX_LEN_OF_SUPPORTED_RATES]; //for OID_DOT11_SUPPORTED_DATA_RATES_VALUE

    AP_PSEUDO_STA_CONFIG APPseudoSTAConfig; // Pseudo STA configuration for the soft AP mode.

    PMKID_CACHE PMKIDCache; // PMKID cache.

    BSS_ENTRY LatestBSSEntry; // Latest BSS entry

    //P2P Control Setting
    P2P_CONTROL     P2pControl;
    BOOLEAN         bP2pAckCheck;
    BOOLEAN         P2PVideoBoost;  // 1: Enable VO stream in Ralink P2P Group
    

    // TRUE: The initialization of the hardware and miniport driver is complete.
    // FALSE: The initialization of the hardware and miniport driver is not complete yet. 
    //
    // Note that the Ralink UI can determine when it can make OID requests by verify this variable (MTK_OID_N6_QUERY_INITIALIZATION_STATE).
    BOOLEAN bInitializationState;
    UCHAR               BssidRepositTime;   //the time to determine how long to keep a bssid item   
    ULONG               GainProSpeed;                       // loop count to be used in JunkTimeout to get CPU busy

    // CH14 only supports 11b
    BOOLEAN             Ch14BOnly;
    UCHAR   DelBACount;             // for BRCM IOT
    
    // For BTHS
    UCHAR       PALRegulatoryClass;
    ULONG       PALCapability;  
    UCHAR       PALVersionSpecifier;
    USHORT      PALCompany_ID;
    USHORT      PALSubVersion;
    
    // Japan CH14 regulation
    BOOLEAN             bJapanFilter;       // True: means programed BBP R4 Bit6 Japan for Tx filter coefficients.

    GPIO_PIN_SHARING_MODE GpioPinSharingMode; // GPIO pin-sharing mode

    RT_MESSAGE_EVENT_LOG                    RTMessageEvent;

    //Led config setting in registry
    ULONG LedCfg;
    ULONG       BTLEDStatus;

    //Registry contol to disable FrequencyCalibration
    BOOLEAN             AdaptiveFreq;

    //Registry contol to Enable/Disable StreamMode
    BOOLEAN             StreamMode;
    // Registry control to Enable/Disable function of ChannelSwitchOffloadToFW
    BOOLEAN             bChannelSwitchOffloadToFW;
    //Flag for the status of Station StreamMode
    BOOLEAN             bStaInStreamMode;

    //Registry contol to write specific BBP value
    ULONG               BBPIDV;     


    //Check security when setting WEP to connect AP
    BOOLEAN     WepConnectCheckEncry;

    //
    // Customer feature setting
    //
    // Bit 0: BT/Wi-Fi Single LED Control
    // Bit 31~1: not used;
    //
    ULONG       FeatureSetting;  
    
    //Bit[3:0] is for Rx number of SS
    //Bit[7:4] is for Tx number of SS
    //Bit[31:8] Reserved
    ULONG               NumberOfSS;

    //Patch Vista/Win7
    BOOLEAN     AcceptICMPv6RA;
    ULONG       MPolicy;    //multicast policy: 0-> defalult(enable "bypass RA packet") 1->disable "bypass RA packet" 2->will not clear up accepted multicast poll

    //UCHAR             WakeUpLevel;

    TRANSMIT_BEAMFORMING_CONTROL TxBfCtrl; // Transmit beamforming control

    TX_RX_CONFIG_BY_REGISTRY TxRxConfigByRegistry; // Tx/Rx configuration by registry

    BOOLEAN bPPAD; // Enable PPAD (per-packet antenna diversity)

    CALIBRATION_CONTROL_REGISTRY CalCtrlRegistry; // Calibration control registry

    HW_ARCHITECTURE_VERSION HwArchitectureVersion; // HW architecture version

    USHORT PACfg; // Internal/external PA configurations (PA_CFG_XXX)

    UCHAR AntMode; // Antenna mode (ANT_MODE_XXX)

    TXRX_IQ_CAL_CFG_TABLE TxRxIQCalCfgTable; // Tx/Rx IQ calibration configuration table

    // 1: Enable InBoxMode: Not RadioOff for Power Saving
    BOOLEAN             InBoxMode;

    BOOLEAN BandedgePowerTableEnable; // Bandedge Tx power table

    BOOLEAN SwitchChannelOffloadEnable;

    BOOLEAN FullCalibrationOffloadEnable;

    BOOLEAN ThermalEnable;

    /* Tx & Rx Stream number selection */
    UCHAR TxStream;
    UCHAR RxStream;

    USHORT  lowTrafficThrd;     /* Threshold for reverting to default MCS when traffic is low */
    SHORT   TrainUpRuleRSSI;    /* If TrainUpRule=2 then use Hybrid rule when RSSI < TrainUpRuleRSSI */
    USHORT  TrainUpLowThrd;     /* QuickDRS Hybrid train up low threshold */
    USHORT  TrainUpHighThrd;    /* QuickDRS Hybrid train up high threshold */
    BOOLEAN TrainUpRule;        /* QuickDRS train up criterion: 0=>Throughput, 1=>PER, 2=> Throughput & PER */

    UCHAR       Bssid[MAC_ADDR_LEN];
    CHAR        Ssid[MAX_LEN_OF_SSID];      // NOT NULL-terminated
    UCHAR       SsidLen;                    // the actual ssid length in used
    UCHAR       LastSsidLen;                // the actual ssid length in used
    CHAR        LastSsid[MAX_LEN_OF_SSID];  // NOT NULL-terminated
    UCHAR       LastBssid[MAC_ADDR_LEN];        

    BOOLEAN     bMixCipher;                 // Indicate current Pair & Group use different cipher suites
    

    // TODO:  Unify Jedi
    BOOLEAN bTXRX_RXV_ON;
} COMMON_CONFIG, *PCOMMON_CONFIG;


// STA configuration and status
typedef struct _STA_ADMIN_CONFIG {
    // GROUP 1 -
    //   User configuration loaded from Registry, E2PROM or OID_xxx. These settings describe
    //   the user intended configuration, but not necessary fully equal to the final 
    //   settings in ACTIVE BSS after negotiation/compromize with the BSS holder (either 
    //   AP or IBSS holder).
    //   Once initialized, user configuration can only be changed via OID_xxx
    UCHAR       BssType;              // BSS_INFRA or BSS_ADHOC
    USHORT      AtimWin;          // used when starting a new IBSS

    // GROUP 2 -
    //   User configuration loaded from Registry, E2PROM or OID_xxx. These settings describe
    //   the user intended configuration, and should be always applied to the final 
    //   settings in ACTIVE BSS without compromising with the BSS holder.
    //   Once initialized, user configuration can only be changed via OID_xxx
    UCHAR       RssiTrigger;
    UCHAR       RssiTriggerMode;      // RSSI_TRIGGERED_UPON_BELOW_THRESHOLD or RSSI_TRIGGERED_UPON_EXCCEED_THRESHOLD
    USHORT      DefaultListenCount;   // default listen count;
    ULONG       WindowsPowerMode;           // Power mode for AC power
    ULONG       WindowsBatteryPowerMode;    // Power mode for battery if exists
    BOOLEAN     bWindowsACCAMEnable;        // Enable CAM power mode when AC on
    BOOLEAN     bAutoReconnect;         // Set to TRUE when setting OID_802_11_SSID with no matching BSSID
    BOOLEAN  bAutoPowerSaveEnable;      // Win8 Auto Power Save
#ifdef NDIS51_MINIPORT  
    ULONG       WindowsPowerProfile;    // Windows power profile, for NDIS5.1 PnP
#endif
    // MIB:ieee802dot11.dot11smt(1).dot11StationConfigTable(1)
    USHORT      Psm;                  // power management mode   (PWR_ACTIVE|PWR_SAVE)
    USHORT      DisassocReason;
    UCHAR       DisassocSta[MAC_ADDR_LEN];
    USHORT      DeauthReason;
    UCHAR       DeauthSta[MAC_ADDR_LEN];
    USHORT      AuthFailReason;
    UCHAR       AuthFailSta[MAC_ADDR_LEN];

    CIPHER_KEY  PskKey;                 // WPA PSK mode PMK
    UCHAR       PTK[64];                // WPA PSK mode PTK

    // Wsc state
    WSC_CTRL    WscControl;         // WSC control block, WscState default is SS_NOTUSE and handled by microsoft 802.1x

    // WPA 802.1x port control, WPA_802_1X_PORT_SECURED, WPA_802_1X_PORT_NOT_SECURED
    UCHAR       PortSecured;

    // For WPA countermeasures
    ULONGLONG   LastMicErrorTime;   // record last MIC error time
    ULONG       MicErrCnt;          // Should be 0, 1, 2, then reset to zero (after disassoiciation).
    BOOLEAN     bBlockAssoc;        // Block associate attempt for 60 seconds after counter measure occurred.
    // For WPA-PSK supplicant state
    WPA_STATE   WpaState;           // Default is SS_NOTUSE and handled by microsoft 802.1x
    UCHAR       ReplayCounter[8];
    UCHAR       ANonce[32];         // ANonce for WPA-PSK from aurhenticator
    UCHAR       SNonce[32];         // SNonce for WPA-PSK

    UCHAR       LastSNR0;             // last received BEACON's RSSI
    UCHAR       LastSNR1;            // last received BEACON's RSSI for 2nd  antenna
    UCHAR       LastSNR2;            // last received BEACON's SNR for 3nd  antenna 
    CHAR       LastRssi;             // last received BEACON's RSSI
    CHAR       LastRssi2;            // last received BEACON's RSSI for 2nd  antenna
    CHAR       LastRssi3;            // last received BEACON's RSSI for 3rd  antenna
    CHAR        AvgRssi;              // last 8 BEACON's average RSSI
    SHORT       AvgRssiX8;            // sum of last 8 BEACON's RSSI
    CHAR        AvgRssi2;              // last 8 BEACON's average RSSI
    SHORT       AvgRssi2X8;            // sum of last 8 BEACON's RSSI
    CHAR        AvgRssi3;              // last 8 BEACON's average RSSI
    SHORT       AvgRssi3X8;            // sum of last 8 BEACON's RSSI
    ULONG       NumOfAvgRssiSample;
    // new RSSI
    RSSI_SAMPLE RssiSample;

    ULONGLONG   LastBeaconRxTime;     // OS's timestamp of the last BEACON RX time
    ULONG       BeaconLostTime; // After Beacon Lost Time Indicate dissconnect to OS
    ULONG       DataPacketsFromAP;     // # of data packets received from AP since we receive last Beacon from AP    
    ULONGLONG   LastScanTime;       // Record last scan time for issue BSSID_SCAN_LIST
    BOOLEAN     SmartScan;
    ULONG       OzmoDeviceSuppressScan; //0: Disable suppress scan, 1: Smart suppress scan, 2: Suppress Scan
    ULONGLONG   LastScanTimeFromNdis;       // Record last scan time for issue BSSID_SCAN_LIST from zero config
    ULONG       ScanCnt;            // Scan counts since most recent SSID, BSSID, SCAN OID request
    BOOLEAN     bSwRadioOff;        // For Vista, TRUE means turn off Radio.
    BOOLEAN     bSwRadio;           // Software controlled Radio On/Off, TRUE: On
    ULONG       LastSwRadio;        // Use to keep the last SW Radio status
    BOOLEAN     bHwRadio;           // Hardware controlled Radio On/Off, TRUE: On
    ULONG       LastHwRadio;        // Use to keep the last HW Radio status
    BOOLEAN     bRadio;             // Radio state, And of Sw & Hw radio state
    BOOLEAN     bHardwareRadio;     // Hardware controlled Radio enabled
    BOOLEAN     bHwPBC;             // Hardware controlled WPS PBC On/Off, TRUE: On 
    BOOLEAN     HwPBCState;         // WPS PBC mode for UI query

    BOOLEAN     AdhocCreator;            // TRUE indicates divice is Creator. 
    BOOLEAN     AdhocJoiner;             // TRUE indicates divice is Joiner. It's exclusvie with AdhocCreator.
    
    // New for WPA, windows want us to to keep association information and
    // Fixed IEs from last association response
    NDIS_802_11_ASSOCIATION_INFORMATION     AssocInfo;
    USHORT      ReqVarIELen;                // Length of next VIE include EID & Length
    UCHAR       ReqVarIEs[MAX_VIE_LEN];
    USHORT      ResVarIELen;                // Length of next VIE include EID & Length
    UCHAR       ResVarIEs[MAX_VIE_LEN];

    ULONG       AdhocMode;          // 0:WIFI mode (11b rates only), 1: b/g mixed, 2: 11g only, 3: 11a only, 4: 11abg mixed
    BOOLEAN     bAdhocNMode;                // AdhocN Support
    BOOLEAN     bInitAdhocNMode;        //use to record the value of "AdhocNMode" registry.
    ULONG       AdhocNForceMulticastRate;   // AdhocN Support: legacy(0x00100000)/auto(0x0)/manual(0x0001xxxx) mode.
    BOOLEAN     bAdhoc5GDisable;    // 0: 5G Adhoc is allowed(default), 1: 5G Adhoc is not allowed

    BOOLEAN     bAdhocN40MHzAllowed;        // 0: 40MHz Adhoc is not allowed(default), 1: 40MHz Adhoc is allowed
    BOOLEAN     bAdhocN14Ch40MHzAllowed;    // 0: 40MHz Adhoc in ch-14 is not allowed(default), 1: 40MHz Adhoc in ch-14 is allowed
    UCHAR       LastWcid;                   // For Adhoc mode, record Wcid of LastTxRate and sync to Rx
    
    BOOLEAN         bFastRoamRescan;        // to support Hidden-SSID fast-roaming, fill CommonCfg.SSID in ProbeReq

    // enhancement for Scanning of FastRoaming
    BOOLEAN             bFastRoamingScan;   // TRUE if doing scanning for fast-roaming, or trying to fast-roam
    BOOLEAN             bImprovedScan;
    UCHAR               ScanChannelCnt;     // 0 at the beginning of scan, stop at 11
    UCHAR               LastScanChannel;    

    CCX_CONTROL         CCXControl;                 // Master administration state
    // a threshold to judge whether the candidate AP is worth roaming or not
    CHAR                RssiDelta;

    CHAR                    CCXDbmOffset;   // a positive number, for RF roaming
    CHAR                    FastRoamingSensitivity; // 0: disable, 1~5: enalbe fast-roaming

    BOOLEAN             StaQuickResponeForRateUpTimerRunning;

    RT_802_11_DLS       DLSEntry[MAX_NUM_OF_DLS_ENTRY];

    // IEEE 802.11z % Tunneled Direct Link Setup %
    RT_802_11_TDLS      TDLSEntry[MAX_NUM_OF_TDLS_ENTRY];
    UCHAR               TDLSDialogToken;
    /* TDLS search table for overheard discovery */
    PTDLS_SEARCH_ENTRY  pTdlsSearchEntryPool;
    QUEUE_HEADER        TdlsSearchEntryFreeList;
    QUEUE_HEADER        TdlsSearchTab[TDLS_SEARCH_HASH_TAB_SIZE];
    NDIS_SPIN_LOCK      TdlsSearchTabLock;
    NDIS_SPIN_LOCK      TDLSEntryLock;
    NDIS_SPIN_LOCK      TdlsDiscoveyQueueLock;
    BOOLEAN         bTdlsDiscoveyInUse;

    // MFP
    PMF_CFG             PmfCfg;

    ////////////////////////////////////////////////////////////////////////////////////////
    // This is only for WHQL test.
    //      on test item 2c_wlan_statistics, issue on [88888] FailedCount was not correctly updated
    //      FailedCount:                      155   (Query by WHQL)
    //      PacketsToSen:                     200
    //      FailedCount:                      354   (Query by WHQL) should be 355
    //    
    // Delta(base.FailedCount, current.FailedCount) < packetsToSend
    //
    // Statistic Register seems not report correct number.
    // Used this variable as trick.
    //
    BOOLEAN             WhqlTest;
    ////////////////////////////////////////////////////////////////////////////////////////

    /** List of MAC addresses we should not try to associate with */
    DOT11_MAC_ADDRESS       ExcludedMACAddressList[STA_EXCLUDED_MAC_ADDRESS_MAX_COUNT];
    ULONG                   ExcludedMACAddressCount;
    BOOLEAN                 IgnoreAllMACAddresses;

//  DOT11_SSID              SSID; // move to PortCfg

    /** Desired BSSID we should attempt to associate with */
//  DOT11_MAC_ADDRESS       DesiredBSSIDList[STA_DESIRED_BSSID_MAX_COUNT]; // move to PortCfg
//  ULONG                   DesiredBSSIDCount;
//  BOOLEAN                 AcceptAnyBSSID;

    // TRUE: The set request is from the Ralink UI with the MTK_OID_N6_SET_DESIRED_BSSID_LIST.
    // FALSE: The set request is from the NDIS subsystem with the OID_DOT11_DESIRED_BSSID_LIST.
    BOOLEAN                 bSetDesiredBSSIDListFromRalinkUI;

    /** PHY list */
    ULONG                   DesiredPhyList[5];
    ULONG                   DesiredPhyCount;
    ULONG                   ActivePhyId;
    PDOT11_SUPPORTED_PHY_TYPES          pSupportedPhyTypes;
    ULONG                           SupportedPhyTypesLen;

    /** PMKID list */
    DOT11_PMKID_ENTRY       PMKIDList[STA_PMKID_MAX_COUNT];
    ULONG                   PMKIDCount;

    /** Current IBSS parameters */
    BOOLEAN                 IBSSJoinOnly;

//  DOT11_MAC_ADDRESS           AssocIEBSSID;   // move to PortCfg
//  PVOID                   AdditionalIEData;
//  ULONG                   AdditionalIESize;

    /** Current setting related to acceptance of unencrypted data */
//  BOOLEAN                 ExcludeUnencrypted;
//  PDOT11_PRIVACY_EXEMPTION_LIST   PrivacyExemptionList;   // move to PortCfg

    UCHAR                   BeaconFrame[MAX_LEN_OF_MLME_BUFFER];
    USHORT                  BeaconFrameSize;
    UCHAR                   AssocRequest[MAX_LEN_OF_MLME_BUFFER];
    USHORT                  AssocRequestLength;
    UCHAR                   AssocResponse[MAX_LEN_OF_MLME_BUFFER];
    USHORT                  AssocResponseLength;

    DOT11_ASSOCIATION_COMPLETION_PARAMETERS  AssocBuffer;

    QUEUE_HEADER            FreeAdhocListQueue;
    QUEUE_HEADER            ActiveAdhocListQueue;
    NDIS_SPIN_LOCK          AdhocListLock;
    RTMP_ADHOC_LIST         AdhocList[MAX_LEN_OF_BSS_TABLE];
    //ULONG                 AdhocListCount;

    //for ADHOC if no SSID exit int scan table ,just scan once
    BOOLEAN             bIbssScanOnce;  
    int                 KeepReconnectCnt;

    // For WAPI
    ULONG       WAISeq;
    UCHAR       Wapi_UserKey[64];
    UCHAR       Wapi_UserKeyLen;    
    
    //Power saving control sturcture
    PS_CONTROL          PSControl;
    
    BOOLEAN     bDropPower;
    ULONG       FiveSecTxErrorRatio;    
    //
    UCHAR       FixLinkSpeedStatus;  // 0 : Show Tx Rate, 1 : Show Fix Rx Rate, 2.....show special case rate

    UCHAR    BBPR3;

    MULTI_DOMAIN_SPECTRUM_MANAGEMENT_CTRL MDSMCtrl; // Multi domain, spectrum and transmit power maintance

    //Patch WiFi 11n : IBSS Beacon Generation
    WIFI_IBSS_SYNC              IBSSync;    
    
    //user request show 300M when device idle and rate is 270M  
    BOOLEAN bIdle300M;
    
    // for Bald Eagle mechanism
    BALD_EAGLE          BaldEagle;

    //Link Quality Setting Control
    USHORT  LinkQualitySetting;

    UCHAR Bssid[6];
    
    //bit 0: Enable/Disable: We will switch to HT40 at MlmeSyncMlmeSyncPeerBeaconAtJoinAction
    //bit 1: Toggle Flag: prevent switch to HT40 again in Linkup
    ULONG               Feature11n; 

    //WSC version
    UCHAR   WSCVersion2;
    UCHAR   WSCVersion2Tmp; //record the latest version 1.0 or 2.0 after change version to 5.7.
    UCHAR   WscHwPBCMode;   // 0: delay over 3 sec. 1: delay 1 sec.

    //For match the creteria of WSC 2.0 test bed. 
    // bit 0: use version 5.7 for test item 4.2.8.
    // bit 1: advertise two WPS IEs in the Probe Request for test item 4.2.9.
    //          (the first one including only the first octet of the TLVs and the second including reset of the TLV data).
    // bit 2: 0->No RSN IE 1-> RSN IE for test item 4.2.12.
    // bit 3: 0->No WSC IE 1-> WSC IE for test item 4.2.12.
    // bit 4: fragmented EAP-WSC packets for test item 4.2.10.
    ULONG   WSC20Testbed;

    BOOLEAN bStopScanForNextConnect;
    
#if(COMPILE_WIN8_ABOVE(CURRENT_OS_NDIS_VER))  
    // WIn8 NLO
    NDIS_NLO_ENTRY      NLOEntry;
#endif
} STA_ADMIN_CONFIG, *PSTA_ADMIN_CONFIG;

// This data structure keep the current active BSS/IBSS's configuration that this STA
// had agreed upon joining the network. Which means these parameters are usually decided
// by the BSS/IBSS creator instead of user configuration. Data in this data structurre 
// is valid only when either ADHOC_ON(pAd) or INFRA_ON(pAd) is TRUE.
// Normally, after SCAN or failed roaming attempts, we need to recover back to
// the current active settings.
typedef struct _STA_ACTIVE_CONFIG {
    USHORT      Aid;
    USHORT      AtimWin;                // in kusec; IBSS parameter set element
    USHORT      CapabilityInfo;
    USHORT      CfpMaxDuration;
    USHORT      CfpPeriod;

    // Copy supported rate from desired AP's beacon. We are trying to match
    // AP's supported and extended rate settings.
    UCHAR       SupRate[MAX_LEN_OF_SUPPORTED_RATES];
    UCHAR       ExtRate[MAX_LEN_OF_SUPPORTED_RATES];
    UCHAR       SupRateLen;
    UCHAR       ExtRateLen;
    RT_HT_CAPABILITY    SupportedHtPhy;
    RT_VHT_CAPABILITY  SupportedVhtPhy;
} STA_ACTIVE_CONFIG, *PSTA_ACTIVE_CONFIG;

// ----------- start of AP --------------------------
// AUTH-RSP State Machine Aux data structure
typedef struct _AP_MLME_AUX {
    UCHAR               Addr[MAC_ADDR_LEN];
    USHORT              Alg;
    CHAR                Challenge[CIPHER_TEXT_LEN];
} AP_MLME_AUX, *PAP_MLME_AUX;

// structure to define WPA Group Key Rekey Interval
typedef struct PACKED _RT_802_11_WPA_REKEY {
    ULONG ReKeyMethod;          // mechanism for rekeying: 0:disable, 1: time-based, 2: packet-based
    ULONG ReKeyInterval;        // time-based: seconds, packet-based: kilo-packets
} RT_WPA_REKEY,*PRT_WPA_REKEY, RT_802_11_WPA_REKEY, *PRT_802_11_WPA_REKEY;


typedef struct   RT_SET_ASIC_WCID {
    ULONG WCID;          // mechanism for rekeying: 0:disable, 1: time-based, 2: packet-based
    ULONG SetTid;        // time-based: seconds, packet-based: kilo-packets
    ULONG DeleteTid;        // time-based: seconds, packet-based: kilo-packets
    UCHAR           Addr[MAC_ADDR_LEN];
} RT_SET_ASIC_WCID,*PRT_SET_ASIC_WCID, *PRT_SET_ASIC_WCID;

typedef struct   RT_SET_ASIC_WCID_ATTRI {
    ULONG WCID;          // mechanism for rekeying: 0:disable, 1: time-based, 2: packet-based
    ULONG Cipher;        // ASIC Cipher definition
    NDIS_PORT_NUMBER  PortNum;
} RT_SET_ASIC_WCID_ATTRI,*PRT_SET_ASIC_WCID_ATTRI;

typedef struct   RT_SET_DLS_WCID_KEY {
    ULONG WCID;
    CIPHER_KEY      CipherKey;
} RT_SET_DLS_WCID_KEY,*PRT_SET_DLS_WCID_KEY;

//
// Set WAPI key
//
typedef struct _RT_SET_WAPI_KEY
{
    USHORT WCID;
    UCHAR WPIEK[256];
    UCHAR EKKeyLen;
    UCHAR WPICK[256];
    UCHAR CKKeyLen;
    UCHAR PN[16];
    UCHAR PNLen;
    UCHAR KeyID;
    BOOLEAN bPNIncrement;
    BOOLEAN bUsePairewiseKeyTable;
} RT_SET_WAPI_KEY,*PRT_SET_WAPI_KEY;

typedef enum _NDS_CLIENT_SETTING {
    NDS_CLIENT_OFF,
    NDS_CLIENT_EVL,
    NDS_CLIENT_ON,
} NDS_CLIENT_SETTING;

// WiFi direct ==========================>
#define GROUP_MODE_TEMP         1
#define GROUP_MODE_PERSISTENT   2
#define MAX_P2P_GROUP_SIZE      30 // Max mactab size(32) - 2
#define MAX_P2P_DISCOVERY_SIZE  30
#define MAX_P2P_TABLE_SIZE      8 //  Save Presistent entry
#define MAX_P2P_GO_TABLE_SIZE   8 // max of numbers of clients for GO role port
#define MAX_P2P_DISCOVERY_FILTER_NUM    2 // max num of discovery filters that is supported by WiFi Direct device for device discovery operations
#define MAX_P2P_SAVECREDN_SIZE  2 //  Save credential for PMK use  entry
#define MAX_P2P_FILTER_LIST     4
#define MAX_P2P_SECONDARY_DEVTYPE_LIST      4
#define PROFILE_P2P             1
#define PROFILE_WPSE            2
#define P2P_DEVICE_TYPE_LEN     8
#define P2P_DEVICE_NAME_LEN     32
#define P2P_AUTO_SCAN_CYCLE     4

#define NUM_OF_CONCURRENT_GO_ROLE           1
#define NUM_OF_CONCURRENT_CLIENT_ROLE       2

#define MAX_NUM_OF_MS_WFD_ROLE              (NUM_OF_CONCURRENT_GO_ROLE + NUM_OF_CONCURRENT_CLIENT_ROLE)
#define MAX_NUM_OF_TOTAL_WFD_ROLE           (MAX_NUM_OF_MS_WFD_ROLE + 1) // add one for virtual station port in win8            

// Count
#define OP_CHANNEL_SCAN_TIMES                   20

typedef enum    _P2pDiscoType
{
    P2P_DISCO_TYPE_DEFAULT = 0, // Ralink P2P scan default, it's not the definition of Win8 WFD.
    // Equal to Win8 WFD Discovery Type 
    P2P_DISCO_TYPE_SCAN = 1,
    P2P_DISCO_TYPE_FIND = 2,
    P2P_DISCO_TYPE_AUTO = 3,
    P2P_DISCO_TYEP_SCAN_SOCIAL_CHANNEL = 4,
    P2P_DISCO_TYPE_FORCED = 0x80000000  // Forced bit is 1 for user cmd or some purpose actions. Forced bit is 0 for periodical scan.
}   P2P_DISCOVERY_TYPE;


// Assume P2P_SCAN_TYPE_AUTO = ACTIVE SCAN
typedef enum    _P2pScanType
{
    P2P_SCAN_TYPE_ACTIVE = 1,
    P2P_SCAN_TYPE_PASSIVE = 2,
    P2P_SCAN_TYPE_AUTO = 3
}   P2P_SCAN_TYPE;

// Describe the peer's state when I am performing P2P Operation with the peer.
typedef enum    _P2pClientState
{
    P2PSTATE_NONE,
    P2PSTATE_DISCOVERY, // Not associated. Because need to get SSID from Probe Response. So add this state.
    P2PSTATE_DISCOVERY_GO,  // this device is a GO. has beacon
    P2PSTATE_DISCOVERY_CLIENT,  // this device is a client that associates with a GO. (in a p2p group.)
    P2PSTATE_DISCOVERY_UNKNOWN, // Need to scan to decide this peer's rule is GO or Client or Device.
    P2PSTATE_CLIENT_DISCO_COMMAND,  // Do Client Discovery.
    P2PSTATE_WAIT_GO_DISCO_ACK, // Do Client Discovery.
    P2PSTATE_WAIT_GO_DISCO_ACK_SUCCESS, // Do Client Discovery.
    P2PSTATE_GO_DISCO_COMMAND,  // Need to send to this GO when doing Client Discovery
    P2PSTATE_INVITE_COMMAND,    // Wait to send Invite Req.
    P2PSTATE_CONNECT_COMMAND,   // wait to send Go Nego Req.
    P2PSTATE_PROVISION_COMMAND, // Provision first, then connect.
    P2PSTATE_SERVICE_DISCO_COMMAND, // Do Service Discovery.
    P2PSTATE_SERVICE_COMEBACK_COMMAND, // Do Service Discovery.
    P2PSTATE_SENT_INVITE_REQ,   // 
    P2PSTATE_SENT_PROVISION_REQ,    // 
    P2PSTATE_WAIT_REVOKEINVITE_RSP_ACK, // 15
    P2PSTATE_REVOKEINVITE_RSP_ACK_SUCCESS,      
    P2PSTATE_SENT_GO_NEG_REQ,       
    P2PSTATE_GOT_GO_RSP_INFO_UNAVAI,    // got GO Nego Rsp with Status : information unavailable. Still need 120 sec more to time out.
    P2PSTATE_WAIT_GO_COMFIRM,       
    P2PSTATE_WAIT_GO_COMFIRM_ACK,       
    P2PSTATE_GOT_GO_COMFIRM,        
    P2PSTATE_GO_COMFIRM_ACK_SUCCESS,        
    P2PSTATE_REINVOKEINVITE_TILLCONFIGTIME,     
    P2PSTATE_GO_DONE,       
    P2PSTATE_GO_WPS,    // Internal registra    
    P2PSTATE_GO_AUTH,
    P2PSTATE_GO_ASSOC,      //30    
    P2PSTATE_CLIENT_WPS,        // Enrollee
    P2PSTATE_CLIENT_WPS_DONE,       // Enrollee
    P2PSTATE_CLIENT_AUTH,
    P2PSTATE_CLIENT_ASSOC,  //      
    P2PSTATE_CLIENT_OPERATING,   // 36  
    P2PSTATE_GO_OPERATING,      
// Go can support legacy station not use WiFi Direct Spec to connect to. Assign following 2 states to such client.
    P2PSTATE_NONP2P_PSK,        // legacy client that uses WPA2PSK-AES to connect to GO(me).
    P2PSTATE_NONP2P_WPS,        // Legacy client that uses WPS to connect to GO(me).
}   P2P_CLIENT_STATE;

// Describe the current discovery state and provistioning state if WifI direct is enabled.
typedef enum    _P2pDiscoProvState
{
    P2P_DISABLE,
    P2P_ENABLE_LISTEN_ONLY, // In this state, only reponse to P2P Probe req. that has P2P IE. enable listen mode.
    P2P_IDLE,
    P2P_SEARCH_COMMAND, // 3        
    P2P_SEARCH,     // 4
    P2P_SEARCH_COMPLETE,
    P2P_LISTEN_COMMAND,
    P2P_LISTEN,     // 7
    P2P_LISTEN_COMPLETE,
}   P2P_DISCOPROV_STATE;

// Describe the current state when performing P2P Operation.
typedef enum    _P2pConnectState
{
    P2P_CONNECT_NOUSE,  //      
    P2P_CONNECT_IDLE = 13,  // Set to bigger number, so don't overlap with P2P_DISCOPROV_STATE
    P2P_NEGOTIATION, //14
    P2P_INVITE, //      15
    P2P_ANY_IN_FORMATION_AS_GO, // 16   
    P2P_ANY_IN_FORMATION_AS_CLIENT, //17    
    P2P_DO_GO_NEG_DONE_CLIENT,      // 18
    P2P_DO_GO_SCAN_OP_BEGIN,        // 19
    P2P_DO_GO_SCAN_OP_DONE,     // 20
    P2P_DO_GO_SCAN_BEGIN,        // 21
    P2P_DO_GO_SCAN_DONE,         // 22
    P2P_WPS_REGISTRA,       // 23
    P2P_DO_WPS_ENROLLEE,        // 24
    P2P_DO_WPS_DONE_NEED_SETEVENT,      // 25
    P2P_DO_WPS_ENROLLEE_DONE,       // 26
    P2P_GO_ASSOC_AUTH,      // 27
    P2P_I_AM_CLIENT_ASSOC_AUTH, // 28   
    P2P_I_AM_CLIENT_OP,     // 29 I am operating as client
    P2P_I_AM_GO_OP,         // 30 I am operatin as Go. 
//  P2PGO_DO_WPS_REGISTRA,      // 30
//  P2PGO_I_AM_GO_ASSOC_AUTH,       
}   P2P_CONNECT_STATE;


// 
typedef enum    _P2pChannelState
{
    P2P_DIFF_CHANNEL_NOUSE, //      
    P2P_DIFF_CHANNEL_OFF = 31,  // 
    P2P_USBPRE_NOATHEN_GOTOSCAN_STATE,  // 
    P2P_PRE_NOATHEN_GOTOSCAN_STATE, // 
    P2P_NOATHEN_GOTOSCAN_STATE, // Maybe go to scan...
    P2P_ENTER_GOTOSCAN, // 
}   P2P_CHANNEL_STATE;

typedef struct _SET_EVENT
{
        HANDLE  hEvent;
} SET_EVENT, *PSET_EVENT;

typedef enum    _P2pMs_STATE_PHASE
{
    P2pMs_STATE_NOUSE,
    P2pMs_STATE_IDLE = 41,
    P2pMs_STATE_START_PROGRESS,// start p2p handshake with PD
    P2pMs_STATE_FORMATION,  // go negonition phase(GONEG->...)
    P2pMs_STATE_INVITATION, // invitation phase (INV->PD->...)
    P2pMs_STATE_PROVISIONING,   // wps phase
}   P2pMs_STATE_PHASE, *PP2pMs_STATE_PHASE;

typedef enum    _P2pMs_SCAN_FILTER
{
    P2pMs_SCAN_FILTER_NULL          =   0x00,
    P2pMs_SCAN_FILTER_DEVICE_ID     =   0x01,   
    P2pMs_SCAN_FILTER_DEVICE_TYPE   =   0x02,
    P2pMs_SCAN_FILTER_SSID          =   0x04,
    P2pMs_SCAN_FILTER_FORCE_LEGACY_NETWORK =    0x08,
}   P2pMs_SCAN_FILTER, *PP2pMs_SCAN_FILTER;

typedef enum    _P2pMs_DISCOVER_TRIGGER_TYPE
{
    P2pMs_DISCOVER_BY_OS            =   0x00,
    P2pMs_DISCOVER_BY_DRIVER        =   0x01    
}   P2pMs_DISCOVER_TRIGGER_TYPE;

// Set via OID
#define P2PEVENT_STATUS_IDLE                1
#define P2PEVENT_STATUS_UPDATE_DISCOVERY    2
#define P2PEVENT_STATUS_GO_NEGO_DONE        3
#define P2PEVENT_STATUS_WPS_FAIL            4
// If as GO, will set PMK.  If as Client, will set up a WPA2PSK-AES connection profile for AutoConfig.
#define P2PEVENT_STATUS_WPS_DONE            5
#define P2PEVENT_P2PGROUP_TOPOLOGY_UPDATE   6
// Like Vista Softap mode.. need to generate a fake adhoc connection profile.
#define P2PEVENT_GEN_SEUDO_ADHOC_PROFILE    7

#define P2PEVENT_ENABLE_DHCP_SERVER         8
#define P2PEVENT_DISABLE_DHCP_SERVER        9
// Delete connectino profile that was setup in the AutoConfig Wireless Management 
// Driver set this when he leaves P2P group (disconnected)
#define P2PEVENT_DISCONNECT                 10
//If driver enabled listening, when a P2P peer tries to connect to me, driver need to inform GUI
// to pop up a setting windows, (PIN, PBC..)
#define P2PEVENT_POPUP_SETTING_WINDOWS      11
// Driver use existing persistent credential to connect, but fail. So notify GUI to delete the entry.
#define P2PEVENT_PERSISTENT_CONNT_FAIL      12
//Just disply the PIN.
#define P2PEVENT_DISPLAY_PIN                13
#define P2PEVENT_SWITCH_OPMODE_STAP2P       14
#define P2PEVENT_SWITCH_OPMODE_STA          15
// Disable ICS
#define P2PEVENT_DISABLE_ICS                16
#define P2PEVENT_ENABLE_ICS                 17
#define P2PEVENT_DRIVER_INTERNAL_USE        18
#define P2PEVENT_SIGMA_USE_DISCONNECT       19  // For sigma test, they need us to clear the wireless connection profile.
#define P2PEVENT_SIGMA_IS_ON                20
#define P2PEVENT_ERROR_CODE                 21
#define P2PEVENT_CONNECT_COMPLETE           22
#define P2PEVENT_CONNECT_FAIL           25
#define P2PEVENT_PEER_MANUAL_DISCONNECTION          26

// to pop up a setting windows for autoGO, (PIN, PBC..)
#define P2PEVENT_POPUP_SETTING_WINDOWS_FOR_AUTOGO       23
#define P2PEVENT_CONFIRM_ENTER_PIN_FROM_WPS_M2          24
// Sigma set a switching operation mode to Driver, and Driver pass through to GUI. 
#define P2PEVENT_STATUS_NOTREADY            99

#define ENTRYSTATUS_LEGACYSTATION   0x1
#define ENTRYSTATUS_GOREADY     0x2 // GUI will put a crown on me,.
#define ENTRYSTATUS_PSM         0x4 // psm mode 
#define ENTRYSTATUS_11N         0x8 // support 11n 


typedef struct  _OID_P2PENTRY
{
    UCHAR       EntryRule;  // P2P_IS_GO or P2P_IS_CLIENT
    CHAR        Dbm;    // 
    UCHAR       EntryStatus;    // Legacy station or already sending beacon.
    UCHAR       MacAddr[MAC_ADDR_LEN];  
    UCHAR       ThisPeerIp[4];
    UCHAR       PrimaryDeviceType[8];
    UCHAR       DeviceName[32];
    ULONG       DeviceNameLen;
}   OID_P2PENTRY, *POID_P2PENTRY;

typedef struct  _OID_DISCOVERYENTRY
{
    UCHAR       NoOfThisGroup;  //If this is set >1, then the following (x-1) are all in my group. And need to parse MyRule to know its P2P group topology.
    UCHAR       MyRule;     // .
    UCHAR       P2PFlags;       // .
    USHORT      ConfigMethod;
    UCHAR       MacAddr[MAC_ADDR_LEN];      
    UCHAR       PrimaryDeviceType[8];
    UCHAR       DeviceName[32];
    ULONG       DeviceNameLen;
}   OID_DISCOVERYENTRY, *POID_DISCOVERYENTRY;

// Save for "Persistent" P2P table. Temporary P2P doesn't need to save in the table.
typedef struct _RT_P2P_PERSISTENT_ENTRY
{
    BOOLEAN     bValid;
    UCHAR       MyRule;     // My rule is GO or Client 
    UCHAR       Addr[MAC_ADDR_LEN];     // this addr is to distinguish this persistent entry is for which mac addr  
    WSC_CREDENTIAL  Profile;                //  profile's bssid is always the GO's bssid. 
} RT_P2P_PERSISTENT_ENTRY, *PRT_P2P_PERSISTENT_ENTRY;

typedef struct  _P2PCLIENT_NOA_SCHEDULE {
    BOOLEAN     bValid;
    BOOLEAN     bInAwake;
    BOOLEAN     bNeedResumeNoA; // Set to TRUE if need to restart infinite NoA
    BOOLEAN     bWMMPSInAbsent; // Set to TRUE if enter GO absent period by supported UAPSD GO
    UCHAR       Token;
    ULONG       SwTimerTickCounter; // this Counter is used for sw-based NoA implemetation tick counter 
    ULONG       CurrentTargetTimePoint; // For sw based method NoA usage.
    ULONG       NextTargetTimePoint;
    ULONG       NextTimePointForWMMPSCounting;  // fro counting WMM PS EOSP bit. Not used for NoA implementation.
    UCHAR       Count;
    ULONG       Duration;
    ULONG       Interval;
    ULONG       StartTime;
    ULONG       OngoingAwakeTime;   // this time will keep increasing as time go by.indicate the current awake time point
    ULONG       TsfHighByte;
    ULONG       ThreToWrapAround;
}   P2PCLIENT_NOA_SCHEDULE, *PP2PCLIENT_NOA_SCHEDULE;

typedef struct _P2P_ENTRY_PARM
{
    P2P_CLIENT_STATE        P2pClientState; // From the state, can know peer it registra or enrollee.
    UCHAR       P2pFlag;
    UCHAR       NoAToken;
    UCHAR       GeneralToken;
    UCHAR       DevCapability;  // table 10 
    UCHAR       GroupCapability;    //  
    UCHAR       DevAddr[MAC_ADDR_LEN];  // P2P Device Address
    UCHAR       InterfaceAddr[MAC_ADDR_LEN];
    UCHAR       PrimaryDevType[P2P_DEVICE_TYPE_LEN];
    UCHAR       NumSecondaryType;
    UCHAR   SecondaryDevType[MAX_P2P_SECONDARY_DEVTYPE_LIST][P2P_DEVICE_TYPE_LEN];  // This definition only support save one 2ndary DevType
    UCHAR       DeviceName[32];
    ULONG       DeviceNameLen;
    USHORT      ConfigMethod;
//  USHORT      ExtListenPeriod;    // Period for extended listening
//  USHORT      ExtListenInterval;  // Interval for extended listening
    UCHAR       CTWindow;   // As GO, Store client's Presence request NoA.  As Client, Store GO's NoA In beacon or P2P Action frame
    P2PCLIENT_NOA_SCHEDULE  NoADesc[1]; // As GO, Store client's Presence request NoA.  As Client, Store GO's NoA In beacon or P2P Action frame
}P2P_ENTRY_PARM, *PP2P_ENTRY_PARM;

#define P2P_ERRCODE_BOTHINTENT15        0x1
#define P2P_ERRCODE_CHANNELDISMATCH     0x2
#define P2P_ERRCODE_WPSNACK             0x3
#define P2P_ERRCODE_CANTFIND            0x4
#define P2P_ERRCODE_NOCONFIGMETHOD      0x5
#define P2P_ERRCODE_REACHLIMIT          0x6
#define P2P_ERRCODE_REJECTBYUSER        0x7
#define P2P_ERRCODE_CHANNELCONFLICT     0x8
#define P2P_ERRCODE_ADHOCON             0x9
#define P2P_ERRCODE_PHYMODECONFLICT     0xa
#define P2P_ERRCODE_PEER_WITHOUT_PERSISTENT_TABLE       0xb

// Set via OID
typedef union   _P2P_STATUS_UNION{

// For P2PEVENT_ERROR_CODE
    struct {
    ULONG       ErrorStatusCode;        // Let RaUI to disappear progress bar if P2P group formation failed
    ULONG       PeerChannel;        // Store the OpChannel assigned by Peer. If error code is P2P_ERRCODE_CHANNELDISMATCH
    USHORT      PeerConfigMethod;       // Store the Config Method assigned by Peer. If error code is P2P_ERRCODE_NOCONFIGMETHOD
    }ErrorCode;

    // For P2PEVENT_DISPLAY_PIN
    struct {
    UCHAR       MacAddr[6];     // I use display PIN for this mac addr.
    }DisplayPin;

    // For P2PEVENT_STATUS_UPDATE_DISCOVERY
    struct {
    UCHAR   TotalNumber;        // Number Of Discovered P2P Peer
    OID_DISCOVERYENTRY  DiscoveryEntry[MAX_P2P_GROUP_SIZE];
    }Discovery;
    
    // For P2PEVENT_P2PGROUP_TOPOLOGY_UPDATE
    struct {
    UCHAR   NumberOfPeer;       // Number of peers in this P2P Group, not include me.
    OID_P2PENTRY    P2PEntry[MAX_P2P_GROUP_SIZE];  // Specify MAC address and his rule.
    }P2PGroup;

    // For P2PEVENT_STATUS_WPS_FAIL
    struct {
    ULONG   Reason;     // = Wsc Status
    }WpsFail;
    
    // For P2PEVENT_STATUS_WPS_DONE
    struct {
    BOOLEAN bIsPersistent;      // When set to TRUE, GUI need to save this profile to registry.
    RT_P2P_PERSISTENT_ENTRY WpsDoneContent;
    }WpsDone;

    // For P2PEVENT_STATUS_GO_NEGO_DONE
    struct {
    UCHAR       MyRule;
    ULONG       WscMode;
    UCHAR       PIN[8];
    WSC_CREDENTIAL          WscProfile;
    }GoNegoDone;


    // For P2PEVENT_GEN_SEUDO_ADHOC_PROFILE
    struct {
    UCHAR       Channel;        // this profile's channel
    NDIS_802_11_SSID        uSsid;      // this profile's SsidLen.  Note: must be OpenNone. .
    }SeudoAdhoc;
    
    // For P2PEVENT_POPUP_SETTING_WINDOWS
    struct {
        UCHAR       DeviceName[32];
        ULONG       DeviceNameLen;
    UCHAR       GroupId[6];     // Group ip. most people set to MAC address.
    USHORT      ConfigMethod;       // Group ip. most people set to MAC address.
    }PopUpParm;
    RT_P2P_PERSISTENT_ENTRY  PerstContent;
}   P2P_STATUS_UNION, *PP2P_STATUS_UNION;

typedef struct _RTMP_OID_QUERY_P2P_STATUS_STRUCT 
{
    UCHAR   P2PStatus;       
    P2P_STATUS_UNION        P2PStateContext;
} RT_OID_QUERY_P2P_STATUS_STRUCT, *PRT_OID_QUERY_P2P_STATUS_STRUCT;

// P2P Mac Info on the virtual port 
typedef struct _RT_OID_QUERY_P2P_MAC_INFO_STRUCT
{
    BOOLEAN             bActive;
    ULONG               PortNumber;
    UCHAR               MacAddr[MAC_ADDR_LEN];      
} RT_OID_QUERY_P2P_MAC_INFO_STRUCT, *PRT_OID_QUERY_P2P_MAC_INFO_STRUCT;

// <==WiFi Direct==============================


//
// AGS control
//
typedef struct _AGS_CONTROL
{
    UCHAR MCSGroup; // The MCS group under testing
    UCHAR lastRateIdx;
} AGS_CONTROL,*PAGS_CONTROL;

#pragma pack(1)
typedef struct  _MAC_TABLE_POOL{
    UCHAR                         MacTabPoolListSize;
    QUEUE_HEADER            MacTabPoolList;
    NDIS_RW_LOCK            MacTabPoolLock;    
}MAC_TABLE_POOL, *PMAC_TABLE_POOL;
#pragma pack()


//
// The lowerbound/upperbound of the explicit TxBf
//
#define ETX_BF_TIMEOUT_LOWERBOUND       0
#define ETX_BF_TIMEOUT_UPPERBOUND       65535

//
// The lowerbound/upperbound of the implicit TxBf
//
#define ITX_BF_TIMEOUT_LOWERBOUND       0
#define ITX_BF_TIMEOUT_UPPERBOUND       65535

//
// The explicit non-compressed/compressed beamforming feedback capable
//
#define ETX_BF_DELAYED_FEEDBACK                 1
#define ETX_BF_IMMEDIATE_FEEDBACK               2
#define ETX_BF_DELAYED_AND_IMMEDIATE_FEEDBACK   3
//
// The RSSI threshold of applying the TxBf matrix to the packet
//
#define RSSI_OF_APPLYING_TXBF_MATRIX            (100)

//
// The period of sending the NDP announcement
//
#define PERIOD_OF_SENDING_NDP_ANNOUNCEMENT  5



typedef struct PACKED _MAC_TABLE_ENTRY {
    struct _MAC_TABLE_ENTRY    *Next;
    //Choose 1 from ValidAsWDS and ValidAsCLI  to validize.
    UCHAR           wcid;
    USHORT          WlanIdxRole;        
    BOOLEAN         ValidAsCLI;     // Sta mode, set this TRUE after Linkup,too.
    BOOLEAN         ValidAsWDS; // This is WDS Entry. only for AP mode.
    BOOLEAN         ValidAsP2P; // This is a WiFi Direct client or GO. 
    BOOLEAN         bIAmBadAtheros; // Flag if this is Atheros chip that has IOT problem.  We need to turn on RTS/CTS protection.

    //jan for wpa
    // record which entry revoke MIC Failure , if it leaves the BSS itself, AP won't update aMICFailTime MIB
    UCHAR           CMTimerRunning;
    UCHAR           RSNIE_Len;

    UCHAR           RSN_IE[AP_MAX_LEN_OF_RSNIE];
    UCHAR           ANonce[32];
    UCHAR           R_Counter[8];
    UCHAR           PTK[64];
    UCHAR           ReTryCounter;   

    MTK_TIMER_STRUCT                 RetryTimer;
    ULONG                               AuthMode;   // This should match to whatever microsoft defined, use ULONG Intead of DOT11_AUTH_ALGORITHM.
                                                // Otherwise, DOT11_CIPHER_ALGO_IHV_START will be treat as -2147483648
    ULONG                               WepStatus;  //use ULONG intead of DOT11_CIPHER_ALGORITHM, 
    AP_WPA_STATE    WpaState;
    WSC_STATE       WscState;
    GTK_STATE       GTKState;
    USHORT          PortSecured;
    NDIS_802_11_PRIVACY_FILTER  PrivacyFilter;      // PrivacyFilter enum for 802.1X
    CIPHER_KEY         PairwiseKey;
    PVOID           pAd;

    UCHAR           Addr[MAC_ADDR_LEN];
    UCHAR           PsMode;
    SST             Sst;
    AUTH_STATE      AuthState; // for SHARED KEY authentication state machine used only
    UCHAR          Aid;
    USHORT          CapabilityInfo;
    UCHAR           LastRssi;
    ULONG           NoDataIdleCount;
    ULONG           NoBADataCountDown;  // When >0, keep countdown till zero, when it's zero, resume send AMPDU 
    ULONG           PsQIdleCount;
    QUEUE_HEADER    PsQueue;
//====================================================
//WDS entry needs these
// rt2860 add this. if ValidAsWDS==TRUE, MatchWDSTabIdx is the index in WdsTab.MacTab
    UINT        MatchWDSTabIdx;
    UCHAR           MaxSupportedRate;
    UCHAR           CurrTxRate;
    UCHAR           CurrTxRateIndex;
    UCHAR           LowestTxRateIndex;
    // to record the each TX rate's quality. 0 is best, the bigger the worse.
    USHORT          TxQuality[MAX_STEP_OF_TX_RATE_SWITCH];
    ULONG           LastTxOkCount;
    UCHAR           PER[MAX_STEP_OF_TX_RATE_SWITCH];
    USHORT          OneSecTxOkCount;
    USHORT          OneSecTxRetryOkCount;
    USHORT          OneSecTxFailCount;
    UCHAR           TxRateUpPenalty;      // extra # of second penalty due to last unstable condition
    ULONG           CurrTxRateStableTime; // # of second in current TX rate
    BOOLEAN         fNoisyEnvironment;
    BOOLEAN         fLastSecAccordingRSSI;  
    UCHAR           LastSecTxRateChangeAction; // 0: no change, 1:rate UP, 2:rate down
    UCHAR           LastTimeTxRateChangeAction; //Keep last time value of LastSecTxRateChangeAction 
    UCHAR           LastTxPER;  /* Tx PER in last Rate Adaptation interval */

    NDS_CLIENT_SETTING  NDSClientSetting; // To force use RATE_2 for TX

    ULONGLONG           LastBeaconRxTime;
    ULONG               LastKickTxCount;
    ULONGLONG           LastBeaconRefTime;  // for Adhoc entry update   

    // a bitmap of BOOLEAN flags. each bit represent an operation status of a particular 
    // BOOLEAN control, either ON or OFF. These flags should always be accessed via
    // CLIENT_STATUS_TEST_FLAG(), CLIENT_STATUS_SET_FLAG(), CLIENT_STATUS_CLEAR_FLAG() macros.
    // see fOP_STATUS_xxx in RTMP_DEF.C for detail bit definition
    ULONG           ClientStatusFlags;
    // HT EWC MIMO-N used parameters
    USHORT      RXBAbitmap; // fill to on-chip  RXWI_BA_BITMASK in 8.1.3RX attribute entry format
    USHORT      TXBAbitmap; // This bitmap as originator, only keep in software used to mark AMPDU bit in TXWI 
    USHORT      TXAutoBAbitmap; // This bitmap as originator, only keep in software used to mark AMPDU bit in TXWI 
    UCHAR       BARecWcidArray[NUM_OF_TID]; // The mapping wcid of recipient session. if RXBAbitmap bit is masked
    UCHAR       BAOriWcidArray[NUM_OF_TID]; // The mapping wcid of originator session. if TXBAbitmap bit is masked
    USHORT      BAOriSequence[NUM_OF_TID]; // The mapping wcid of originator session. if TXBAbitmap bit is masked
    //HTTRANSMIT_SETTING    HTPhyMode, MaxHTPhyMode, MinHTPhyMode;// For transmit phy setting in TXWI.

    PHY_CFG TxPhyCfg;
    PHY_CFG MaxPhyCfg;
    PHY_CFG MinPhyCfg;

    UCHAR       MpduDensity;
    UCHAR       MaxRAmpduFactor;
    UCHAR       BaSizeInUse;    // Our rt2860 only support max 16k bytes transmitting AMPDU. 
    UCHAR       AMsduSize;
    UCHAR       MmpsMode;   // MIMO power save more. 
    PID_COUNTER     Stafifoaccu[16];    //fifo stack can be record per {WCID , PID}
    UCHAR       RateLen;
    HT_CAPABILITY_IE        HTCapability;

        // NDIS6.2 ExtAP 
    // 
    BOOLEAN         bWpsEnabled;
    USHORT          usListenInterval;
    UCHAR           SupRate[MAX_LEN_OF_SUPPORTED_RATES];
    BOOLEAN         SupCCKMCS[MAX_LEN_OF_CCK_RATES];
    BOOLEAN         SupOFDMMCS[MAX_LEN_OF_OFDM_RATES];
    UCHAR           SupRateLen;
    DOT11_ASSOCIATION_STATE AssocState; // Current association state of the station
    DOT11_POWER_MODE    PowerMode;
    LARGE_INTEGER   AssocUpTime; // Get timestamp after association decision made by OS

#if(COMPILE_WIN7_ABOVE(CURRENT_OS_NDIS_VER))  
    DOT11_PEER_STATISTICS   Statistics;
#endif

    // [P2P]
    //
    P2P_ENTRY_PARM      P2pInfo;
    //
    // NDIS 6.2
    // Association decision
    //
    BOOLEAN     AcceptAssoc;
    USHORT      Reason;

    // [WAPI]
    //
    APSTA_BKSA_INFO BKSA;
    APSTA_USKSA_INFO    USKSA;  
    APSTA_MSKSA_INFO    MSKSA;

    UCHAR   IdentifyIntelSta; // Used to indentify Intel STA

    // Patch Atheros send out invalid BAR frame.
    USHORT      BARStartSeq;
    ULONG       BadBARCount;
    
    AGS_CONTROL AGSCtrl; // AGS control
    
#if UAPSD_AP_SUPPORT
    BOOLEAN         bAPSDFlagSPStart;   /* 1: SP is started */
    USHORT          UAPSDTxNum;         /* total U-APSD frame numbers to TxSwUapsdQueue*/   
    QUEUE_HEADER    UAPSDQueue[WMM_NUM_OF_AC]; /* queue for each U-APSD */
    UCHAR           MaxSPLength;
    BOOLEAN         bAPSDCapablePerAC[WMM_NUM_OF_AC]; /* for trigger-enabled */
    BOOLEAN         bAPSDDeliverEnabledPerAC[WMM_NUM_OF_AC]; /* for delivery-enabled */
    BOOLEAN         bAPSDAllAC; /* 1: all AC are delivery-enabled U-APSD */
    USHORT          UAPSDQIdleCount;           /* U-APSD queue timeout */
//  ULONGLONG       UAPSDTimeStampLast; /* for accurate mechanism*/
    BOOLEAN         bAPSDFlagSpRoughUse;/* 1: use rough SP */
    UCHAR           UAPSDQAid;
    UINT32          UAPSDTagOffset[WMM_NUM_OF_AC]; /* AC0, AC1, AC2, and AC3*/      
#endif

    VHT_PEER_STA_CTRL VhtPeerStaCtrl; // VHT control of peer STA

    PMP_PORT        pPort;

    // new RSSI
    RSSI_SAMPLE RssiSample;

    UCHAR mcsGroup;/* the mcs group to be tried */

    UCHAR *pTable;  /* Pointer to this entry's Tx Rate Table */
    UCHAR lastRateIdx;
    UCHAR lowTrafficCount;
    BOOLEAN perThrdAdj;
} MAC_TABLE_ENTRY, *PMAC_TABLE_ENTRY;

typedef struct PACKED _MAC_TABLE {
    UCHAR           Size;
    QUEUE_HEADER    McastPsQueue;
    QUEUE_HEADER            MacTabList;
    ULONG           PsQIdleCount;
    BOOLEAN         fAnyStationInPsm;   // TRUE, then outgoing BCAST/MCAST frame should 
    BOOLEAN         fAnyStationIsLegacy;   // TRUE, then outgoing BCAST/MCAST frame should 
    BOOLEAN         fAnyStationNonGF;   // TRUE, then outgoing BCAST/MCAST frame should 
    BOOLEAN         fAnyStation20Only;   // TRUE, 40MHz transmission needs protection 
    BOOLEAN         fAnyStationBadAtheros;   // TRUE, 40MHz transmission needs protection 
    BOOLEAN         fAnyBASession;   // TRUE, 40MHz transmission needs protection 
    BOOLEAN         fAnyStationIsCCK;   // Check if any station uses 11b rate to transmit in IBSS
    BOOLEAN         fAnyStationIsOFDM;   // Check if any station uses 11g rate to transmit in IBSS
} MAC_TABLE, *PMAC_TABLE;

typedef struct PACKED _WDS_ENTRY {
    BOOLEAN         Valid;
    UCHAR           Addr[MAC_ADDR_LEN];
    ULONG           NoDataIdleCount;
    struct _WDS_ENTRY *pNext;
} WDS_ENTRY, *PWDS_ENTRY;

typedef struct PACKED _WDS_TABLE_ENTRY {
    UCHAR           Size;
    UCHAR           WdsAddr[MAC_ADDR_LEN];
    WDS_ENTRY       *Hash[HASH_TABLE_SIZE];
    WDS_ENTRY       Content[MAX_LEN_OF_MAC_TABLE];
    UCHAR           MaxSupportedRate;
    UCHAR           CurrTxRate;
    USHORT          TxQuality[MAX_LEN_OF_SUPPORTED_RATES];
    USHORT          OneSecTxOkCount;
    USHORT          OneSecTxRetryOkCount;
    USHORT          OneSecTxFailCount;
    ULONG           CurrTxRateStableTime; // # of second in current TX rate
    UCHAR           TxRateUpPenalty;      // extra # of second penalty due to last unstable condition
} WDS_TABLE_ENTRY, *PWDS_TABLE_ENTRY;

typedef struct PACKED _WDS_TABLE {
    ULONG               Size;
//  WPA_KEY             Wpa_key;
    CIPHER_KEY          Wpa_key;
    WDS_TABLE_ENTRY     MacTab[MAX_NUM_OF_WDS_LINK];
    UCHAR     MacTabMatchWCID[MAX_NUM_OF_WDS_LINK]; // ASIC
    
} WDS_TABLE, *PWDS_TABLE;

typedef struct PACKED _AP_ADMIN_CONFIG {
    USHORT          CapabilityInfo;

    // for wpa
    UCHAR           GMK[32];
    UCHAR           PMK[32];
    UCHAR           GNonce[32];
    UCHAR           PortSecured;  // WPA 802.1x port control, WPA_802_1X_PORT_SECURED, WPA_802_1X_PORT_NOT_SECURED
    WPA_GTK_STATE   WpaGTKState;
    MTK_TIMER_STRUCT         CounterMeasureTimer;
    ULONG           MICErrorCounter;
    UCHAR           GKeyDoneStations; // number of entry left to update group key

    ULONG           ApPortNum;
    BOOLEAN         bWPSEnable;

    UCHAR           CMTimerRunning;
    UCHAR           BANClass3Data;
    LARGE_INTEGER   aMICFailTime;
    LARGE_INTEGER   PrevaMICFailTime;
    ULONG           MICFailureCounter;
    // for Group Rekey 
    RT_WPA_REKEY    WPAREKEY;
    ULONG           REKEYCOUNTER;
    MTK_TIMER_STRUCT     REKEYTimer;
    UCHAR           REKEYTimerRunning;
   
    UCHAR           LastSNR0;             // last received BEACON's SNR
    UCHAR           LastSNR1;            // last received BEACON's SNR for 2nd  antenna
    UCHAR           LastSNR2;            // last received BEACON's SNR for 3nd  antenna

    // new RSSI
    RSSI_SAMPLE RssiSample;

    ULONG           NumOfAvgRssiSample;

    BOOLEAN         bAutoChannelAtBootup;  // 0: disable, 1: enable
    BOOLEAN         bIsolateInterStaTraffic;
    BOOLEAN         bHideSsid;
    BOOLEAN         bPreAuth;

    // temporary latch for Auto channel selection
    UCHAR           AutoChannel_MaxRssi;  // max RSSI during Auto Channel Selection period
    UCHAR           AutoChannel_Channel;  // channel number during Auto Channel Selection

    UCHAR           RSNIE_Len[2];
    UCHAR           RSN_IE[2][AP_MAX_LEN_OF_RSNIE];
    
    UCHAR           Key_Counter[32];  //jan for wpa

    ULONG           TimIELocationInBeacon;
    UCHAR           CapabilityInfoLocationInBeacon;
    ULONG       NoAStartTimeLocationInBeacon;
    ULONG           TimBitmap;      // bit0 for broadcast, 1 for AID1, 2 for AID2, ...so on
    ULONG           TimBitmap2;     // b0 for AID32, b1 for AID33, ... and so on
    UCHAR           DtimCount;      // 0.. DtimPeriod-1
    UCHAR           DtimPeriod;     // default = 3
    UCHAR           ErpIeContent;
    BOOLEAN         bErpIEChange;
    BOOLEAN         bTIMIEChange;
    ULONGLONG       LastOLBCDetectTime;
    USHORT          AgeoutTime;     // Default 300 second
    // fo CCX 1.0
    BOOLEAN         bCkipCmicOn;
    UCHAR           CkipFlag;
    UCHAR           GIV[3];  //for CCX iv
    UCHAR           RxSEQ[4];
    UCHAR           TxSEQ[4];
    UCHAR           CKIPMIC[4];

    // EDCA parameters to be announced to its local BSS
    EDCA_PARM       BssEdcaParm;

    BOOLEAN         ApQuickResponeForRateUpTimerRunning;    

    UCHAR           AdhocBssid[6];
    ULONG           ApPseudoStaAuthMode;  // Use for indicate to OS when using Ralink SoftAP ;
    
    // rt2860c support unlimited piggyback. should delete this boolean.
    BOOLEAN                 bOneClientUsePiggyBack;
//   DOT11_SSID              SSID; // move to PortCfg
    /** Additonal IEs for beacon */
//  ULONG                   AdditionalBeaconIESize; // move to PortCfg
//  PVOID                   AdditionalBeaconIEData;

    /** Additonal IEs for probe response */
//  ULONG                   AdditionalResponseIESize; // move to PortCfg
//  PVOID                   AdditionalResponseIEData;
//   PDOT11_PRIVACY_EXEMPTION_LIST   PrivacyExemptionList;
    BOOLEAN bSoftAPReady;
    UCHAR       StaChannel ;            
    UCHAR       StaCentralChannel;      // Central Channel when using 40MHz is indicating. not real channel.        
    BOOLEAN    StaHtEnable;

    //Independent setting for softap port
    UCHAR               PhyMode;
    HT_CAPABILITY_IE    HtCapability;
    ADD_HT_INFO_IE      AddHTInfoIe;
    PHY_CFG TxPhyCfg;

    BOOLEAN              bVhtEnable;
    VHT_CAP_IE           VhtCapability;
    VHT_OP_IE            VhtOperation;
    VHT_EXT_BSS_LOAD_IE  VhtExtBssLoad;

    //HTTRANSMIT_SETTING HTPhyMode;

    // For WAPI
    ULONG       WAISeq;
    UCHAR       Wapi_UserKey[64];
    UCHAR       Wapi_UserKeyLen;

    ULONG   SoftAPForceMulticastRate; // Manual: 0x1001xxxx, Legacy/Auto: 0x1010xxxx, Otherwise (0x1000xxxx/0x1100xxxx/...): same as unicast rate
    ULONG   MAX_PSDU_LEN; // Maximum PSDU length    

    PMP_PORT          SoftApPort;
    BOOLEAN             StartSoftApAfterScan;
    
} AP_ADMIN_CONFIG, *PAP_ADMIN_CONFIG;
// ----------- end of AP ----------------------------

//
// PHY specific MIB. The MIB could be different for different vendors.
//
typedef struct _NICPHYMIB       
{
    DOT11_PHY_TYPE                      PhyType;
    ULONG                               PhyID;
    DOT11_RATE_SET                      OperationalRateSet;
    DOT11_RATE_SET                      BasicRateSet;
    DOT11_RATE_SET                      ActiveRateSet;
    UCHAR                               Channel;
    DOT11_SUPPORTED_DATA_RATES_VALUE_V2 SupportedDataRatesValue;
} NICPHYMIB, *PNICPHYMIB;

typedef struct _REPOSIT_BSSID_LIST_EX
{
    UCHAR                   BssNr;
    BSS_ENTRY           *pBssEntry;
    LARGE_INTEGER           ulTime[MAX_LEN_OF_BSS_TABLE];
}REPOSIT_BSSID_LIST_EX, *PREPOSIT_BSSID_LIST_EX;

typedef struct _SIGMA_CFG
{
    BOOLEAN bFixRxStbc;

    //
    // Add the Operating Mode Notification IE in the Association Request
    //
    BOOLEAN bAddOperatingModeNotificationInAssociationRequest;
} SIGMA_CFG, *PSIGMA_CFG;

//
// for RX temperature compensation
//
typedef struct  _RX_TEMP_COMP
{
    char        ADC6B;
    char        TwoStreamCompForGband;
    char        TwoStreamCompForAband;
    char        OneStreamCompForGband;
    char        OneStreamCompForAband;  
} RX_TEMP_COMP, *PRX_TEMP_COMP;

//
// The extension of the transmit power control over MAC
//
typedef struct _TX_POWER_CONTROL_EXT_OVER_MAC
{
    struct
    {
        ULONG   TxPwrCfg0;       // MAC 0x1314
        ULONG   TxPwrCfg0Ext;    // MAC 0x1390
        ULONG   TxPwrCfg1;       // MAC 0x1318
        ULONG   TxPwrCfg1Ext;    // MAC 0x1394
        ULONG   TxPwrCfg2;       // MAC 0x131C
        ULONG   TxPwrCfg2Ext;    // MAC 0x1398
        ULONG   TxPwrCfg3;       // MAC 0x1320
        ULONG   TxPwrCfg3Ext;    // MAC 0x139C
        ULONG   TxPwrCfg4;       // MAC 0x1324
        ULONG   TxPwrCfg4Ext;    // MAC 0x13A0
        ULONG   TxPwrCfg5;       // MAC 0x1384
        ULONG   TxPwrCfg6;       // MAC 0x1388
        ULONG   TxPwrCfg7;       // MAC 0x13D4
        ULONG   TxPwrCfg8;       // MAC 0x13D8
        ULONG   TxPwrCfg9;       // MAC 0x13DC
    } BW20Over2Dot4G;

    struct
    {
        ULONG   TxPwrCfg0;       // MAC 0x1314
        ULONG   TxPwrCfg0Ext;    // MAC 0x1390
        ULONG   TxPwrCfg1;       // MAC 0x1318
        ULONG   TxPwrCfg1Ext;    // MAC 0x1394
        ULONG   TxPwrCfg2;       // MAC 0x131C
        ULONG   TxPwrCfg2Ext;    // MAC 0x1398
        ULONG   TxPwrCfg3;       // MAC 0x1320
        ULONG   TxPwrCfg3Ext;    // MAC 0x139C
        ULONG   TxPwrCfg4;       // MAC 0x1324
        ULONG   TxPwrCfg4Ext;    // MAC 0x13A0
        ULONG   TxPwrCfg5;       // MAC 0x1384
        ULONG   TxPwrCfg6;       // MAC 0x1388
        ULONG   TxPwrCfg7;       // MAC 0x13D4
        ULONG   TxPwrCfg8;       // MAC 0x13D8
        ULONG   TxPwrCfg9;       // MAC 0x13DC
    } BW40Over2Dot4G;

    struct
    {
        ULONG   TxPwrCfg0;       // MAC 0x1314
        ULONG   TxPwrCfg0Ext;    // MAC 0x1390
        ULONG   TxPwrCfg1;       // MAC 0x1318
        ULONG   TxPwrCfg1Ext;    // MAC 0x1394
        ULONG   TxPwrCfg2;       // MAC 0x131C
        ULONG   TxPwrCfg2Ext;    // MAC 0x1398
        ULONG   TxPwrCfg3;       // MAC 0x1320
        ULONG   TxPwrCfg3Ext;    // MAC 0x139C
        ULONG   TxPwrCfg4;       // MAC 0x1324
        ULONG   TxPwrCfg4Ext;    // MAC 0x13A0
        ULONG   TxPwrCfg5;       // MAC 0x1384
        ULONG   TxPwrCfg6;       // MAC 0x1388
        ULONG   TxPwrCfg7;       // MAC 0x13D4
        ULONG   TxPwrCfg8;       // MAC 0x13D8
        ULONG   TxPwrCfg9;       // MAC 0x13DC
    } BW20Over5G;

    struct
    {
        ULONG   TxPwrCfg0;       // MAC 0x1314
        ULONG   TxPwrCfg0Ext;    // MAC 0x1390
        ULONG   TxPwrCfg1;       // MAC 0x1318
        ULONG   TxPwrCfg1Ext;    // MAC 0x1394
        ULONG   TxPwrCfg2;       // MAC 0x131C
        ULONG   TxPwrCfg2Ext;    // MAC 0x1398
        ULONG   TxPwrCfg3;       // MAC 0x1320
        ULONG   TxPwrCfg3Ext;    // MAC 0x139C
        ULONG   TxPwrCfg4;       // MAC 0x1324
        ULONG   TxPwrCfg4Ext;    // MAC 0x13A0
        ULONG   TxPwrCfg5;       // MAC 0x1384
        ULONG   TxPwrCfg6;       // MAC 0x1388
        ULONG   TxPwrCfg7;       // MAC 0x13D4
        ULONG   TxPwrCfg8;       // MAC 0x13D8
        ULONG   TxPwrCfg9;       // MAC 0x13DC
    } BW40Over5G;
} TX_POWER_CONTROL_EXT_OVER_MAC, *PTX_POWER_CONTROL_EXT_OVER_MAC;


typedef struct _TX_POWER_CONTROL_EXT_OVER_MAC_WITH_BANDEDGE
{
    struct
    {
        ULONG   TxPwrCfg0;       // MAC 0x1314 // Tx0, Tx1: CCK 1M/2M, CCK 5.5M/11M, OFDM 6M/9M, OFDM 12M/18M
        ULONG   TxPwrCfg0Ext;       // MAC 0x1390 // Tx2, Tx3: CCK 1M/2M, CCK 5.5M/11M, OFDM 6M/9M, OFDM 12M/18M
        ULONG   TxPwrCfg1;      // MAC 0x1318 // Tx0, Tx1: OFDM 24M/36M, OFDM 48M/54M, HT MCS = 0,1, HT MCS = 2,3
        ULONG   TxPwrCfg1Ext;       // MAC 0x1394 // Tx2, Tx3: OFDM 24M/36M, OFDM 48M/54M, HT MCS = 0,1, HT MCS = 2,3
        ULONG   TxPwrCfg2;      // MAC 0x131C // Tx0, Tx1: HT MCS = 4,5, HT MCS = 6,7, HT MCS = 8,9, HT MCS = 10,11
        ULONG   TxPwrCfg2Ext;       // MAC 0x1398 // Tx2, Tx3: HT MCS = 4,5, HT MCS = 6,7, HT MCS = 8,9, HT MCS = 10,11
        ULONG   TxPwrCfg3;      // MAC 0x1320 // Tx0, Tx1: HT MCS = 12,13 HT MCS = 14,15, STBC MCS = 0,1, STBC MCS = 2,3
        ULONG   TxPwrCfg3Ext;       // MAC 0x139C // Tx2, Tx3: HT MCS = 12,13 HT MCS = 14,15, STBC MCS = 0,1, STBC MCS = 2,3
        ULONG   TxPwrCfg4;       // MAC 0x1324 // Tx0, Tx1: STBC MCS = 4,5, STBC MCS = 6,7 Reserved
        ULONG   TxPwrCfg4Ext;       // MAC 0x13A0 // Tx2, Tx3: STBC MCS = 4,5, STBC MCS = 6,7 Reserved
        ULONG   TxPwrCfg5;       // MAC 0x1384  // Tx0, Tx1,Tx2, Tx3: HT MCS = 16,17, HT MCS = 18,19, 
        ULONG   TxPwrCfg6;       // MAC 0x1388 // Tx0, Tx1,Tx2, Tx3: HT MCS = 20,21, HT MCS = 22, 
        ULONG   TxPwrCfg7;       // MAC 0x13D4 // Tx0, Tx1,Tx2, Tx3: OFDM = 54, HT MCS = 7, 
        ULONG   TxPwrCfg8;       // MAC 0x13D8// Tx0, Tx1,Tx2, Tx3: HT MCS = 15,, HT MCS =23
        ULONG   TxPwrCfg9;       // MAC 0x13DC// Tx0, Tx1,Tx2, Tx3: STBC MCS = 7
    } BW20Over2Dot4G;

    struct
    {
        ULONG   TxPwrCfg0;       // MAC 0x1314 // Tx0, Tx1: CCK 1M/2M, CCK 5.5M/11M, OFDM 6M/9M, OFDM 12M/18M
        ULONG   TxPwrCfg0Ext;       // MAC 0x1390 // Tx2, Tx3: CCK 1M/2M, CCK 5.5M/11M, OFDM 6M/9M, OFDM 12M/18M
        ULONG   TxPwrCfg1;      // MAC 0x1318 // Tx0, Tx1: OFDM 24M/36M, OFDM 48M/54M, HT MCS = 0,1, HT MCS = 2,3
        ULONG   TxPwrCfg1Ext;       // MAC 0x1394 // Tx2, Tx3: OFDM 24M/36M, OFDM 48M/54M, HT MCS = 0,1, HT MCS = 2,3
        ULONG   TxPwrCfg2;      // MAC 0x131C // Tx0, Tx1: HT MCS = 4,5, HT MCS = 6,7, HT MCS = 8,9, HT MCS = 10,11
        ULONG   TxPwrCfg2Ext;       // MAC 0x1398 // Tx2, Tx3: HT MCS = 4,5, HT MCS = 6,7, HT MCS = 8,9, HT MCS = 10,11
        ULONG   TxPwrCfg3;      // MAC 0x1320 // Tx0, Tx1: HT MCS = 12,13 HT MCS = 14,15, STBC MCS = 0,1, STBC MCS = 2,3
        ULONG   TxPwrCfg3Ext;       // MAC 0x139C // Tx2, Tx3: HT MCS = 12,13 HT MCS = 14,15, STBC MCS = 0,1, STBC MCS = 2,3
        ULONG   TxPwrCfg4;       // MAC 0x1324 // Tx0, Tx1: STBC MCS = 4,5, STBC MCS = 6,7 Reserved
        ULONG   TxPwrCfg4Ext;       // MAC 0x13A0 // Tx2, Tx3: STBC MCS = 4,5, STBC MCS = 6,7 Reserved
        ULONG   TxPwrCfg5;       // MAC 0x1384  // Tx0, Tx1,Tx2, Tx3: HT MCS = 16,17, HT MCS = 18,19, 
        ULONG   TxPwrCfg6;       // MAC 0x1388 // Tx0, Tx1,Tx2, Tx3: HT MCS = 20,21, HT MCS = 22, 
        ULONG   TxPwrCfg7;       // MAC 0x13D4 // Tx0, Tx1,Tx2, Tx3: OFDM = 54, HT MCS = 7, 
        ULONG   TxPwrCfg8;       // MAC 0x13D8// Tx0, Tx1,Tx2, Tx3: HT MCS = 15,, HT MCS =23
        ULONG   TxPwrCfg9;       // MAC 0x13DC// Tx0, Tx1,Tx2, Tx3: STBC MCS = 7
    } BW40Over2Dot4G;

    struct
    {
        ULONG   TxPwrCfg0;       // MAC 0x1314 // Tx0, Tx1: CCK 1M/2M, CCK 5.5M/11M, OFDM 6M/9M, OFDM 12M/18M
        ULONG   TxPwrCfg0Ext;       // MAC 0x1390 // Tx2, Tx3: CCK 1M/2M, CCK 5.5M/11M, OFDM 6M/9M, OFDM 12M/18M
        ULONG   TxPwrCfg1;      // MAC 0x1318 // Tx0, Tx1: OFDM 24M/36M, OFDM 48M/54M, HT MCS = 0,1, HT MCS = 2,3
        ULONG   TxPwrCfg1Ext;       // MAC 0x1394 // Tx2, Tx3: OFDM 24M/36M, OFDM 48M/54M, HT MCS = 0,1, HT MCS = 2,3
        ULONG   TxPwrCfg2;      // MAC 0x131C // Tx0, Tx1: HT MCS = 4,5, HT MCS = 6,7, HT MCS = 8,9, HT MCS = 10,11
        ULONG   TxPwrCfg2Ext;       // MAC 0x1398 // Tx2, Tx3: HT MCS = 4,5, HT MCS = 6,7, HT MCS = 8,9, HT MCS = 10,11
        ULONG   TxPwrCfg3;      // MAC 0x1320 // Tx0, Tx1: HT MCS = 12,13 HT MCS = 14,15, STBC MCS = 0,1, STBC MCS = 2,3
        ULONG   TxPwrCfg3Ext;       // MAC 0x139C // Tx2, Tx3: HT MCS = 12,13 HT MCS = 14,15, STBC MCS = 0,1, STBC MCS = 2,3
        ULONG   TxPwrCfg4;       // MAC 0x1324 // Tx0, Tx1: STBC MCS = 4,5, STBC MCS = 6,7 Reserved
        ULONG   TxPwrCfg4Ext;       // MAC 0x13A0 // Tx2, Tx3: STBC MCS = 4,5, STBC MCS = 6,7 Reserved
        ULONG   TxPwrCfg5;       // MAC 0x1384  // Tx0, Tx1,Tx2, Tx3: HT MCS = 16,17, HT MCS = 18,19, 
        ULONG   TxPwrCfg6;       // MAC 0x1388 // Tx0, Tx1,Tx2, Tx3: HT MCS = 20,21, HT MCS = 22, 
        ULONG   TxPwrCfg7;       // MAC 0x13D4 // Tx0, Tx1,Tx2, Tx3: OFDM = 54, HT MCS = 7, 
        ULONG   TxPwrCfg8;       // MAC 0x13D8// Tx0, Tx1,Tx2, Tx3: HT MCS = 15,, HT MCS =23
        ULONG   TxPwrCfg9;       // MAC 0x13DC// Tx0, Tx1,Tx2, Tx3: STBC MCS = 7
    } BW20Over2Dot4GBandEdge;

    struct
    {
        ULONG   TxPwrCfg0;       // MAC 0x1314 // Tx0, Tx1: CCK 1M/2M, CCK 5.5M/11M, OFDM 6M/9M, OFDM 12M/18M
        ULONG   TxPwrCfg0Ext;       // MAC 0x1390 // Tx2, Tx3: CCK 1M/2M, CCK 5.5M/11M, OFDM 6M/9M, OFDM 12M/18M
        ULONG   TxPwrCfg1;      // MAC 0x1318 // Tx0, Tx1: OFDM 24M/36M, OFDM 48M/54M, HT MCS = 0,1, HT MCS = 2,3
        ULONG   TxPwrCfg1Ext;       // MAC 0x1394 // Tx2, Tx3: OFDM 24M/36M, OFDM 48M/54M, HT MCS = 0,1, HT MCS = 2,3
        ULONG   TxPwrCfg2;      // MAC 0x131C // Tx0, Tx1: HT MCS = 4,5, HT MCS = 6,7, HT MCS = 8,9, HT MCS = 10,11
        ULONG   TxPwrCfg2Ext;       // MAC 0x1398 // Tx2, Tx3: HT MCS = 4,5, HT MCS = 6,7, HT MCS = 8,9, HT MCS = 10,11
        ULONG   TxPwrCfg3;      // MAC 0x1320 // Tx0, Tx1: HT MCS = 12,13 HT MCS = 14,15, STBC MCS = 0,1, STBC MCS = 2,3
        ULONG   TxPwrCfg3Ext;       // MAC 0x139C // Tx2, Tx3: HT MCS = 12,13 HT MCS = 14,15, STBC MCS = 0,1, STBC MCS = 2,3
        ULONG   TxPwrCfg4;       // MAC 0x1324 // Tx0, Tx1: STBC MCS = 4,5, STBC MCS = 6,7 Reserved
        ULONG   TxPwrCfg4Ext;       // MAC 0x13A0 // Tx2, Tx3: STBC MCS = 4,5, STBC MCS = 6,7 Reserved
        ULONG   TxPwrCfg5;       // MAC 0x1384  // Tx0, Tx1,Tx2, Tx3: HT MCS = 16,17, HT MCS = 18,19, 
        ULONG   TxPwrCfg6;       // MAC 0x1388 // Tx0, Tx1,Tx2, Tx3: HT MCS = 20,21, HT MCS = 22, 
        ULONG   TxPwrCfg7;       // MAC 0x13D4 // Tx0, Tx1,Tx2, Tx3: OFDM = 54, HT MCS = 7, 
        ULONG   TxPwrCfg8;       // MAC 0x13D8// Tx0, Tx1,Tx2, Tx3: HT MCS = 15,, HT MCS =23
        ULONG   TxPwrCfg9;       // MAC 0x13DC// Tx0, Tx1,Tx2, Tx3: STBC MCS = 7
    } BW40Over2Dot4GBandEdge;
    
    struct
    {
        ULONG   TxPwrCfg0;       // MAC 0x1314 // Tx0, Tx1: CCK 1M/2M, CCK 5.5M/11M, OFDM 6M/9M, OFDM 12M/18M
        ULONG   TxPwrCfg0Ext;       // MAC 0x1390 // Tx2, Tx3: CCK 1M/2M, CCK 5.5M/11M, OFDM 6M/9M, OFDM 12M/18M
        ULONG   TxPwrCfg1;      // MAC 0x1318 // Tx0, Tx1: OFDM 24M/36M, OFDM 48M/54M, HT MCS = 0,1, HT MCS = 2,3
        ULONG   TxPwrCfg1Ext;       // MAC 0x1394 // Tx2, Tx3: OFDM 24M/36M, OFDM 48M/54M, HT MCS = 0,1, HT MCS = 2,3
        ULONG   TxPwrCfg2;      // MAC 0x131C // Tx0, Tx1: HT MCS = 4,5, HT MCS = 6,7, HT MCS = 8,9, HT MCS = 10,11
        ULONG   TxPwrCfg2Ext;       // MAC 0x1398 // Tx2, Tx3: HT MCS = 4,5, HT MCS = 6,7, HT MCS = 8,9, HT MCS = 10,11
        ULONG   TxPwrCfg3;      // MAC 0x1320 // Tx0, Tx1: HT MCS = 12,13 HT MCS = 14,15, STBC MCS = 0,1, STBC MCS = 2,3
        ULONG   TxPwrCfg3Ext;       // MAC 0x139C // Tx2, Tx3: HT MCS = 12,13 HT MCS = 14,15, STBC MCS = 0,1, STBC MCS = 2,3
        ULONG   TxPwrCfg4;       // MAC 0x1324 // Tx0, Tx1: STBC MCS = 4,5, STBC MCS = 6,7 Reserved
        ULONG   TxPwrCfg4Ext;       // MAC 0x13A0 // Tx2, Tx3: STBC MCS = 4,5, STBC MCS = 6,7 Reserved
        ULONG   TxPwrCfg5;       // MAC 0x1384  // Tx0, Tx1,Tx2, Tx3: HT MCS = 16,17, HT MCS = 18,19, 
        ULONG   TxPwrCfg6;       // MAC 0x1388 // Tx0, Tx1,Tx2, Tx3: HT MCS = 20,21, HT MCS = 22, 
        ULONG   TxPwrCfg7;       // MAC 0x13D4 // Tx0, Tx1,Tx2, Tx3: OFDM = 54, HT MCS = 7, 
        ULONG   TxPwrCfg8;       // MAC 0x13D8// Tx0, Tx1,Tx2, Tx3: HT MCS = 15,, HT MCS =23
        ULONG   TxPwrCfg9;       // MAC 0x13DC// Tx0, Tx1,Tx2, Tx3: STBC MCS = 7
    } BW20Over5G;

    struct
    {
        ULONG   TxPwrCfg0;       // MAC 0x1314 // Tx0, Tx1: CCK 1M/2M, CCK 5.5M/11M, OFDM 6M/9M, OFDM 12M/18M
        ULONG   TxPwrCfg0Ext;       // MAC 0x1390 // Tx2, Tx3: CCK 1M/2M, CCK 5.5M/11M, OFDM 6M/9M, OFDM 12M/18M
        ULONG   TxPwrCfg1;      // MAC 0x1318 // Tx0, Tx1: OFDM 24M/36M, OFDM 48M/54M, HT MCS = 0,1, HT MCS = 2,3
        ULONG   TxPwrCfg1Ext;       // MAC 0x1394 // Tx2, Tx3: OFDM 24M/36M, OFDM 48M/54M, HT MCS = 0,1, HT MCS = 2,3
        ULONG   TxPwrCfg2;      // MAC 0x131C // Tx0, Tx1: HT MCS = 4,5, HT MCS = 6,7, HT MCS = 8,9, HT MCS = 10,11
        ULONG   TxPwrCfg2Ext;       // MAC 0x1398 // Tx2, Tx3: HT MCS = 4,5, HT MCS = 6,7, HT MCS = 8,9, HT MCS = 10,11
        ULONG   TxPwrCfg3;      // MAC 0x1320 // Tx0, Tx1: HT MCS = 12,13 HT MCS = 14,15, STBC MCS = 0,1, STBC MCS = 2,3
        ULONG   TxPwrCfg3Ext;       // MAC 0x139C // Tx2, Tx3: HT MCS = 12,13 HT MCS = 14,15, STBC MCS = 0,1, STBC MCS = 2,3
        ULONG   TxPwrCfg4;       // MAC 0x1324 // Tx0, Tx1: STBC MCS = 4,5, STBC MCS = 6,7 Reserved
        ULONG   TxPwrCfg4Ext;       // MAC 0x13A0 // Tx2, Tx3: STBC MCS = 4,5, STBC MCS = 6,7 Reserved
        ULONG   TxPwrCfg5;       // MAC 0x1384  // Tx0, Tx1,Tx2, Tx3: HT MCS = 16,17, HT MCS = 18,19, 
        ULONG   TxPwrCfg6;       // MAC 0x1388 // Tx0, Tx1,Tx2, Tx3: HT MCS = 20,21, HT MCS = 22, 
        ULONG   TxPwrCfg7;       // MAC 0x13D4 // Tx0, Tx1,Tx2, Tx3: OFDM = 54, HT MCS = 7, 
        ULONG   TxPwrCfg8;       // MAC 0x13D8// Tx0, Tx1,Tx2, Tx3: HT MCS = 15,, HT MCS =23
        ULONG   TxPwrCfg9;       // MAC 0x13DC// Tx0, Tx1,Tx2, Tx3: STBC MCS = 7
    } BW40Over5G;

    struct
    {
        ULONG   TxPwrCfg0;       // MAC 0x1314 // Tx0, Tx1: CCK 1M/2M, CCK 5.5M/11M, OFDM 6M/9M, OFDM 12M/18M
        ULONG   TxPwrCfg0Ext;       // MAC 0x1390 // Tx2, Tx3: CCK 1M/2M, CCK 5.5M/11M, OFDM 6M/9M, OFDM 12M/18M
        ULONG   TxPwrCfg1;      // MAC 0x1318 // Tx0, Tx1: OFDM 24M/36M, OFDM 48M/54M, HT MCS = 0,1, HT MCS = 2,3
        ULONG   TxPwrCfg1Ext;       // MAC 0x1394 // Tx2, Tx3: OFDM 24M/36M, OFDM 48M/54M, HT MCS = 0,1, HT MCS = 2,3
        ULONG   TxPwrCfg2;      // MAC 0x131C // Tx0, Tx1: HT MCS = 4,5, HT MCS = 6,7, HT MCS = 8,9, HT MCS = 10,11
        ULONG   TxPwrCfg2Ext;       // MAC 0x1398 // Tx2, Tx3: HT MCS = 4,5, HT MCS = 6,7, HT MCS = 8,9, HT MCS = 10,11
        ULONG   TxPwrCfg3;      // MAC 0x1320 // Tx0, Tx1: HT MCS = 12,13 HT MCS = 14,15, STBC MCS = 0,1, STBC MCS = 2,3
        ULONG   TxPwrCfg3Ext;       // MAC 0x139C // Tx2, Tx3: HT MCS = 12,13 HT MCS = 14,15, STBC MCS = 0,1, STBC MCS = 2,3
        ULONG   TxPwrCfg4;       // MAC 0x1324 // Tx0, Tx1: STBC MCS = 4,5, STBC MCS = 6,7 Reserved
        ULONG   TxPwrCfg4Ext;       // MAC 0x13A0 // Tx2, Tx3: STBC MCS = 4,5, STBC MCS = 6,7 Reserved
        ULONG   TxPwrCfg5;       // MAC 0x1384  // Tx0, Tx1,Tx2, Tx3: HT MCS = 16,17, HT MCS = 18,19, 
        ULONG   TxPwrCfg6;       // MAC 0x1388 // Tx0, Tx1,Tx2, Tx3: HT MCS = 20,21, HT MCS = 22, 
        ULONG   TxPwrCfg7;       // MAC 0x13D4 // Tx0, Tx1,Tx2, Tx3: OFDM = 54, HT MCS = 7, 
        ULONG   TxPwrCfg8;       // MAC 0x13D8// Tx0, Tx1,Tx2, Tx3: HT MCS = 15,, HT MCS =23
        ULONG   TxPwrCfg9;       // MAC 0x13DC// Tx0, Tx1,Tx2, Tx3: STBC MCS = 7
    } BW20Over5GBandEdge;

    struct
    {
        ULONG   TxPwrCfg0;       // MAC 0x1314 // Tx0, Tx1: CCK 1M/2M, CCK 5.5M/11M, OFDM 6M/9M, OFDM 12M/18M
        ULONG   TxPwrCfg0Ext;       // MAC 0x1390 // Tx2, Tx3: CCK 1M/2M, CCK 5.5M/11M, OFDM 6M/9M, OFDM 12M/18M
        ULONG   TxPwrCfg1;      // MAC 0x1318 // Tx0, Tx1: OFDM 24M/36M, OFDM 48M/54M, HT MCS = 0,1, HT MCS = 2,3
        ULONG   TxPwrCfg1Ext;       // MAC 0x1394 // Tx2, Tx3: OFDM 24M/36M, OFDM 48M/54M, HT MCS = 0,1, HT MCS = 2,3
        ULONG   TxPwrCfg2;      // MAC 0x131C // Tx0, Tx1: HT MCS = 4,5, HT MCS = 6,7, HT MCS = 8,9, HT MCS = 10,11
        ULONG   TxPwrCfg2Ext;       // MAC 0x1398 // Tx2, Tx3: HT MCS = 4,5, HT MCS = 6,7, HT MCS = 8,9, HT MCS = 10,11
        ULONG   TxPwrCfg3;      // MAC 0x1320 // Tx0, Tx1: HT MCS = 12,13 HT MCS = 14,15, STBC MCS = 0,1, STBC MCS = 2,3
        ULONG   TxPwrCfg3Ext;       // MAC 0x139C // Tx2, Tx3: HT MCS = 12,13 HT MCS = 14,15, STBC MCS = 0,1, STBC MCS = 2,3
        ULONG   TxPwrCfg4;       // MAC 0x1324 // Tx0, Tx1: STBC MCS = 4,5, STBC MCS = 6,7 Reserved
        ULONG   TxPwrCfg4Ext;       // MAC 0x13A0 // Tx2, Tx3: STBC MCS = 4,5, STBC MCS = 6,7 Reserved
        ULONG   TxPwrCfg5;       // MAC 0x1384  // Tx0, Tx1,Tx2, Tx3: HT MCS = 16,17, HT MCS = 18,19, 
        ULONG   TxPwrCfg6;       // MAC 0x1388 // Tx0, Tx1,Tx2, Tx3: HT MCS = 20,21, HT MCS = 22, 
        ULONG   TxPwrCfg7;       // MAC 0x13D4 // Tx0, Tx1,Tx2, Tx3: OFDM = 54, HT MCS = 7, 
        ULONG   TxPwrCfg8;       // MAC 0x13D8// Tx0, Tx1,Tx2, Tx3: HT MCS = 15,, HT MCS =23
        ULONG   TxPwrCfg9;       // MAC 0x13DC// Tx0, Tx1,Tx2, Tx3: STBC MCS = 7
    } BW40Over5GBandEdge;

} TX_POWER_CONTROL_EXT_OVER_MAC_WITH_BANDEDGE, *PTX_POWER_CONTROL_EXT_OVER_MAC_WITH_BANDEDGE;

// 
// The extension of the transmit power control over VHT MAC
//
typedef struct _TX_POWER_CONTROL_EXT_OVER_MAC_WITH_VHT
{
    // 2.4G - BW20(Ch1/6/11), BW40(Ch3/6/9)
    struct
    {
        ULONG   TxPwrCfg0;      // MAC 0x1314 // CCK 1M/2M, CCK 5.5M/11M, OFDM 6M/9M, OFDM 12M/18M
        ULONG   TxPwrCfg1;      // MAC 0x1318 // OFDM 24M/36M, OFDM 48M, HT(VHT 1SS) MCS 0/1, HT(VHT 1SS) MCS 2/3
        ULONG   TxPwrCfg2;      // MAC 0x131C // HT(VHT 1SS) MCS 4/5, HT(VHT 1SS) MCS 6, -, -
        ULONG   TxPwrCfg3;      // MAC 0x1320 // -, -, STBC MCS 0/1, STBC MCS 2/3
        ULONG   TxPwrCfg4;      // MAC 0x1324 // STBC MCS 4/5, STBC MCS 6, -, -
        ULONG   TxPwrCfg5;      // MAC 0x13D4 // OFDM 54M, -, HT(VHT 1SS) MCS 7, - 
        ULONG   TxPwrCfg6;      // MAC 0x13DC // STBC MCS 7, -, -, - 
    } BW20Over2Dot4GCh1;

    struct
    {
        ULONG   TxPwrCfg0;      // MAC 0x1314 // CCK 1M/2M, CCK 5.5M/11M, OFDM 6M/9M, OFDM 12M/18M
        ULONG   TxPwrCfg1;      // MAC 0x1318 // OFDM 24M/36M, OFDM 48M, HT(VHT 1SS) MCS 0/1, HT(VHT 1SS) MCS 2/3
        ULONG   TxPwrCfg2;      // MAC 0x131C // HT(VHT 1SS) MCS 4/5, HT(VHT 1SS) MCS 6, -, -
        ULONG   TxPwrCfg3;      // MAC 0x1320 // -, -, STBC MCS 0/1, STBC MCS 2/3
        ULONG   TxPwrCfg4;      // MAC 0x1324 // STBC MCS 4/5, STBC MCS 6, -, -
        ULONG   TxPwrCfg5;      // MAC 0x13D4 // OFDM 54M, -, HT(VHT 1SS) MCS 7, - 
        ULONG   TxPwrCfg6;      // MAC 0x13DC // STBC MCS 7, -, -, - 
    } BW20Over2Dot4GCh6;

    struct
    {
        ULONG   TxPwrCfg0;      // MAC 0x1314 // CCK 1M/2M, CCK 5.5M/11M, OFDM 6M/9M, OFDM 12M/18M
        ULONG   TxPwrCfg1;      // MAC 0x1318 // OFDM 24M/36M, OFDM 48M, HT(VHT 1SS) MCS 0/1, HT(VHT 1SS) MCS 2/3
        ULONG   TxPwrCfg2;      // MAC 0x131C // HT(VHT 1SS) MCS 4/5, HT(VHT 1SS) MCS 6, -, -
        ULONG   TxPwrCfg3;      // MAC 0x1320 // -, -, STBC MCS 0/1, STBC MCS 2/3
        ULONG   TxPwrCfg4;      // MAC 0x1324 // STBC MCS 4/5, STBC MCS 6, -, -
        ULONG   TxPwrCfg5;      // MAC 0x13D4 // OFDM 54M, -, HT(VHT 1SS) MCS 7, - 
        ULONG   TxPwrCfg6;      // MAC 0x13DC // STBC MCS 7, -, -, - 
    } BW20Over2Dot4GCh11;

    struct
    {
        ULONG   TxPwrCfg0;      // MAC 0x1314 // CCK 1M/2M, CCK 5.5M/11M, OFDM 6M/9M, OFDM 12M/18M
        ULONG   TxPwrCfg1;      // MAC 0x1318 // OFDM 24M/36M, OFDM 48M, HT(VHT 1SS) MCS 0/1, HT(VHT 1SS) MCS 2/3
        ULONG   TxPwrCfg2;      // MAC 0x131C // HT(VHT 1SS) MCS 4/5, HT(VHT 1SS) MCS 6, -, -
        ULONG   TxPwrCfg3;      // MAC 0x1320 // -, -, STBC MCS 0/1, STBC MCS 2/3
        ULONG   TxPwrCfg4;      // MAC 0x1324 // STBC MCS 4/5, STBC MCS 6, -, -
        ULONG   TxPwrCfg5;      // MAC 0x13D4 // OFDM 54M, -, HT(VHT 1SS) MCS 7, - 
        ULONG   TxPwrCfg6;      // MAC 0x13DC // STBC MCS 7, -, -, - 
    } BW40Over2Dot4GCh3;

    struct
    {
        ULONG   TxPwrCfg0;      // MAC 0x1314 // CCK 1M/2M, CCK 5.5M/11M, OFDM 6M/9M, OFDM 12M/18M
        ULONG   TxPwrCfg1;      // MAC 0x1318 // OFDM 24M/36M, OFDM 48M, HT(VHT 1SS) MCS 0/1, HT(VHT 1SS) MCS 2/3
        ULONG   TxPwrCfg2;      // MAC 0x131C // HT(VHT 1SS) MCS 4/5, HT(VHT 1SS) MCS 6, -, -
        ULONG   TxPwrCfg3;      // MAC 0x1320 // -, -, STBC MCS 0/1, STBC MCS 2/3
        ULONG   TxPwrCfg4;      // MAC 0x1324 // STBC MCS 4/5, STBC MCS 6, -, -
        ULONG   TxPwrCfg5;      // MAC 0x13D4 // OFDM 54M, -, HT(VHT 1SS) MCS 7, - 
        ULONG   TxPwrCfg6;      // MAC 0x13DC // STBC MCS 7, -, -, - 
    } BW40Over2Dot4GCh6;

    struct
    {
        ULONG   TxPwrCfg0;      // MAC 0x1314 // CCK 1M/2M, CCK 5.5M/11M, OFDM 6M/9M, OFDM 12M/18M
        ULONG   TxPwrCfg1;      // MAC 0x1318 // OFDM 24M/36M, OFDM 48M, HT(VHT 1SS) MCS 0/1, HT(VHT 1SS) MCS 2/3
        ULONG   TxPwrCfg2;      // MAC 0x131C // HT(VHT 1SS) MCS 4/5, HT(VHT 1SS) MCS 6, -, -
        ULONG   TxPwrCfg3;      // MAC 0x1320 // -, -, STBC MCS 0/1, STBC MCS 2/3
        ULONG   TxPwrCfg4;      // MAC 0x1324 // STBC MCS 4/5, STBC MCS 6, -, -
        ULONG   TxPwrCfg5;      // MAC 0x13D4 // OFDM 54M, -, HT(VHT 1SS) MCS 7, - 
        ULONG   TxPwrCfg6;      // MAC 0x13DC // STBC MCS 7, -, -, - 
    } BW40Over2Dot4GCh9;
    
    // 5G - BW20(Low Band/Middle Band/High Band/X1 Band), BW40(Low Band/Middle Band/High Band/X1 Band), BW80(Low Band/Middle Band/High Band/X1 Band)
    struct
    {
        ULONG   TxPwrCfg0;      // MAC 0x1314 // CCK 1M/2M, CCK 5.5M/11M, OFDM 6M/9M, OFDM 12M/18M
        ULONG   TxPwrCfg1;      // MAC 0x1318 // OFDM 24M/36M, OFDM 48M, HT(VHT 1SS) MCS 0/1, HT(VHT 1SS) MCS 2/3
        ULONG   TxPwrCfg2;      // MAC 0x131C // HT(VHT 1SS) MCS 4/5, HT(VHT 1SS) MCS 6, -, -
        ULONG   TxPwrCfg3;      // MAC 0x1320 // -, -, STBC MCS 0/1, STBC MCS 2/3
        ULONG   TxPwrCfg4;      // MAC 0x1324 // STBC MCS 4/5, STBC MCS 6, -, -
        ULONG   TxPwrCfg5;      // MAC 0x13D4 // OFDM 54M, -, HT(VHT 1SS) MCS 7, - 
        ULONG   TxPwrCfg6;      // MAC 0x13DC // STBC MCS 7, -, -, - 
        ULONG   TxPwrCfg7;      // MAC 0x13D8 // -, -, VHT 1SS MCS 8, VHT 1SS MCS 9
    } BW20Over5GLow;

    struct
    {
        ULONG   TxPwrCfg0;      // MAC 0x1314 // CCK 1M/2M, CCK 5.5M/11M, OFDM 6M/9M, OFDM 12M/18M
        ULONG   TxPwrCfg1;      // MAC 0x1318 // OFDM 24M/36M, OFDM 48M, HT(VHT 1SS) MCS 0/1, HT(VHT 1SS) MCS 2/3
        ULONG   TxPwrCfg2;      // MAC 0x131C // HT(VHT 1SS) MCS 4/5, HT(VHT 1SS) MCS 6, -, -
        ULONG   TxPwrCfg3;      // MAC 0x1320 // -, -, STBC MCS 0/1, STBC MCS 2/3
        ULONG   TxPwrCfg4;      // MAC 0x1324 // STBC MCS 4/5, STBC MCS 6, -, -
        ULONG   TxPwrCfg5;      // MAC 0x13D4 // OFDM 54M, -, HT(VHT 1SS) MCS 7, - 
        ULONG   TxPwrCfg6;      // MAC 0x13DC // STBC MCS 7, -, -, - 
        ULONG   TxPwrCfg7;      // MAC 0x13D8 // -, -, VHT 1SS MCS 8, VHT 1SS MCS 9
    } BW20Over5GMiddle;

    struct
    {
        ULONG   TxPwrCfg0;      // MAC 0x1314 // CCK 1M/2M, CCK 5.5M/11M, OFDM 6M/9M, OFDM 12M/18M
        ULONG   TxPwrCfg1;      // MAC 0x1318 // OFDM 24M/36M, OFDM 48M, HT(VHT 1SS) MCS 0/1, HT(VHT 1SS) MCS 2/3
        ULONG   TxPwrCfg2;      // MAC 0x131C // HT(VHT 1SS) MCS 4/5, HT(VHT 1SS) MCS 6, -, -
        ULONG   TxPwrCfg3;      // MAC 0x1320 // -, -, STBC MCS 0/1, STBC MCS 2/3
        ULONG   TxPwrCfg4;      // MAC 0x1324 // STBC MCS 4/5, STBC MCS 6, -, -
        ULONG   TxPwrCfg5;      // MAC 0x13D4 // OFDM 54M, -, HT(VHT 1SS) MCS 7, - 
        ULONG   TxPwrCfg6;      // MAC 0x13DC // STBC MCS 7, -, -, - 
        ULONG   TxPwrCfg7;      // MAC 0x13D8 // -, -, VHT 1SS MCS 8, VHT 1SS MCS 9
    } BW20Over5GHigh;

    struct
    {
        ULONG   TxPwrCfg0;      // MAC 0x1314 // CCK 1M/2M, CCK 5.5M/11M, OFDM 6M/9M, OFDM 12M/18M
        ULONG   TxPwrCfg1;      // MAC 0x1318 // OFDM 24M/36M, OFDM 48M, HT(VHT 1SS) MCS 0/1, HT(VHT 1SS) MCS 2/3
        ULONG   TxPwrCfg2;      // MAC 0x131C // HT(VHT 1SS) MCS 4/5, HT(VHT 1SS) MCS 6, -, -
        ULONG   TxPwrCfg3;      // MAC 0x1320 // -, -, STBC MCS 0/1, STBC MCS 2/3
        ULONG   TxPwrCfg4;      // MAC 0x1324 // STBC MCS 4/5, STBC MCS 6, -, -
        ULONG   TxPwrCfg5;      // MAC 0x13D4 // OFDM 54M, -, HT(VHT 1SS) MCS 7, - 
        ULONG   TxPwrCfg6;      // MAC 0x13DC // STBC MCS 7, -, -, - 
        ULONG   TxPwrCfg7;      // MAC 0x13D8 // -, -, VHT 1SS MCS 8, VHT 1SS MCS 9
    } BW20Over5GX1;

    struct
    {
        ULONG   TxPwrCfg0;      // MAC 0x1314 // CCK 1M/2M, CCK 5.5M/11M, OFDM 6M/9M, OFDM 12M/18M
        ULONG   TxPwrCfg1;      // MAC 0x1318 // OFDM 24M/36M, OFDM 48M, HT(VHT 1SS) MCS 0/1, HT(VHT 1SS) MCS 2/3
        ULONG   TxPwrCfg2;      // MAC 0x131C // HT(VHT 1SS) MCS 4/5, HT(VHT 1SS) MCS 6, -, -
        ULONG   TxPwrCfg3;      // MAC 0x1320 // -, -, STBC MCS 0/1, STBC MCS 2/3
        ULONG   TxPwrCfg4;      // MAC 0x1324 // STBC MCS 4/5, STBC MCS 6, -, -
        ULONG   TxPwrCfg5;      // MAC 0x13D4 // OFDM 54M, -, HT(VHT 1SS) MCS 7, - 
        ULONG   TxPwrCfg6;      // MAC 0x13DC // STBC MCS 7, -, -, - 
        ULONG   TxPwrCfg7;      // MAC 0x13D8 // -, -, VHT 1SS MCS 8, VHT 1SS MCS 9
    } BW40Over5GLow;

    struct
    {
        ULONG   TxPwrCfg0;      // MAC 0x1314 // CCK 1M/2M, CCK 5.5M/11M, OFDM 6M/9M, OFDM 12M/18M
        ULONG   TxPwrCfg1;      // MAC 0x1318 // OFDM 24M/36M, OFDM 48M, HT(VHT 1SS) MCS 0/1, HT(VHT 1SS) MCS 2/3
        ULONG   TxPwrCfg2;      // MAC 0x131C // HT(VHT 1SS) MCS 4/5, HT(VHT 1SS) MCS 6, -, -
        ULONG   TxPwrCfg3;      // MAC 0x1320 // -, -, STBC MCS 0/1, STBC MCS 2/3
        ULONG   TxPwrCfg4;      // MAC 0x1324 // STBC MCS 4/5, STBC MCS 6, -, -
        ULONG   TxPwrCfg5;      // MAC 0x13D4 // OFDM 54M, -, HT(VHT 1SS) MCS 7, - 
        ULONG   TxPwrCfg6;      // MAC 0x13DC // STBC MCS 7, -, -, - 
        ULONG   TxPwrCfg7;      // MAC 0x13D8 // -, -, VHT 1SS MCS 8, VHT 1SS MCS 9
    } BW40Over5GMiddle;

    struct
    {
        ULONG   TxPwrCfg0;      // MAC 0x1314 // CCK 1M/2M, CCK 5.5M/11M, OFDM 6M/9M, OFDM 12M/18M
        ULONG   TxPwrCfg1;      // MAC 0x1318 // OFDM 24M/36M, OFDM 48M, HT(VHT 1SS) MCS 0/1, HT(VHT 1SS) MCS 2/3
        ULONG   TxPwrCfg2;      // MAC 0x131C // HT(VHT 1SS) MCS 4/5, HT(VHT 1SS) MCS 6, -, -
        ULONG   TxPwrCfg3;      // MAC 0x1320 // -, -, STBC MCS 0/1, STBC MCS 2/3
        ULONG   TxPwrCfg4;      // MAC 0x1324 // STBC MCS 4/5, STBC MCS 6, -, -
        ULONG   TxPwrCfg5;      // MAC 0x13D4 // OFDM 54M, -, HT(VHT 1SS) MCS 7, - 
        ULONG   TxPwrCfg6;      // MAC 0x13DC // STBC MCS 7, -, -, - 
        ULONG   TxPwrCfg7;      // MAC 0x13D8 // -, -, VHT 1SS MCS 8, VHT 1SS MCS 9
    } BW40Over5GHigh;

    struct
    {
        ULONG   TxPwrCfg0;      // MAC 0x1314 // CCK 1M/2M, CCK 5.5M/11M, OFDM 6M/9M, OFDM 12M/18M
        ULONG   TxPwrCfg1;      // MAC 0x1318 // OFDM 24M/36M, OFDM 48M, HT(VHT 1SS) MCS 0/1, HT(VHT 1SS) MCS 2/3
        ULONG   TxPwrCfg2;      // MAC 0x131C // HT(VHT 1SS) MCS 4/5, HT(VHT 1SS) MCS 6, -, -
        ULONG   TxPwrCfg3;      // MAC 0x1320 // -, -, STBC MCS 0/1, STBC MCS 2/3
        ULONG   TxPwrCfg4;      // MAC 0x1324 // STBC MCS 4/5, STBC MCS 6, -, -
        ULONG   TxPwrCfg5;      // MAC 0x13D4 // OFDM 54M, -, HT(VHT 1SS) MCS 7, - 
        ULONG   TxPwrCfg6;      // MAC 0x13DC // STBC MCS 7, -, -, - 
        ULONG   TxPwrCfg7;      // MAC 0x13D8 // -, -, VHT 1SS MCS 8, VHT 1SS MCS 9
    } BW40Over5GX1;

    struct
    {
        ULONG   TxPwrCfg0;      // MAC 0x1314 // CCK 1M/2M, CCK 5.5M/11M, OFDM 6M/9M, OFDM 12M/18M
        ULONG   TxPwrCfg1;      // MAC 0x1318 // OFDM 24M/36M, OFDM 48M, HT(VHT 1SS) MCS 0/1, HT(VHT 1SS) MCS 2/3
        ULONG   TxPwrCfg2;      // MAC 0x131C // HT(VHT 1SS) MCS 4/5, HT(VHT 1SS) MCS 6, -, -
        ULONG   TxPwrCfg3;      // MAC 0x1320 // -, -, STBC MCS 0/1, STBC MCS 2/3
        ULONG   TxPwrCfg4;      // MAC 0x1324 // STBC MCS 4/5, STBC MCS 6, -, -
        ULONG   TxPwrCfg5;      // MAC 0x13D4 // OFDM 54M, -, HT(VHT 1SS) MCS 7, - 
        ULONG   TxPwrCfg6;      // MAC 0x13DC // STBC MCS 7, -, -, - 
        ULONG   TxPwrCfg7;      // MAC 0x13D8 // -, -, VHT 1SS MCS 8, VHT 1SS MCS 9
    } BW80Over5GLow;

    struct
    {
        ULONG   TxPwrCfg0;      // MAC 0x1314 // CCK 1M/2M, CCK 5.5M/11M, OFDM 6M/9M, OFDM 12M/18M
        ULONG   TxPwrCfg1;      // MAC 0x1318 // OFDM 24M/36M, OFDM 48M, HT(VHT 1SS) MCS 0/1, HT(VHT 1SS) MCS 2/3
        ULONG   TxPwrCfg2;      // MAC 0x131C // HT(VHT 1SS) MCS 4/5, HT(VHT 1SS) MCS 6, -, -
        ULONG   TxPwrCfg3;      // MAC 0x1320 // -, -, STBC MCS 0/1, STBC MCS 2/3
        ULONG   TxPwrCfg4;      // MAC 0x1324 // STBC MCS 4/5, STBC MCS 6, -, -
        ULONG   TxPwrCfg5;      // MAC 0x13D4 // OFDM 54M, -, HT(VHT 1SS) MCS 7, - 
        ULONG   TxPwrCfg6;      // MAC 0x13DC // STBC MCS 7, -, -, - 
        ULONG   TxPwrCfg7;      // MAC 0x13D8 // -, -, VHT 1SS MCS 8, VHT 1SS MCS 9
    } BW80Over5GMiddle;

    struct
    {
        ULONG   TxPwrCfg0;      // MAC 0x1314 // CCK 1M/2M, CCK 5.5M/11M, OFDM 6M/9M, OFDM 12M/18M
        ULONG   TxPwrCfg1;      // MAC 0x1318 // OFDM 24M/36M, OFDM 48M, HT(VHT 1SS) MCS 0/1, HT(VHT 1SS) MCS 2/3
        ULONG   TxPwrCfg2;      // MAC 0x131C // HT(VHT 1SS) MCS 4/5, HT(VHT 1SS) MCS 6, -, -
        ULONG   TxPwrCfg3;      // MAC 0x1320 // -, -, STBC MCS 0/1, STBC MCS 2/3
        ULONG   TxPwrCfg4;      // MAC 0x1324 // STBC MCS 4/5, STBC MCS 6, -, -
        ULONG   TxPwrCfg5;      // MAC 0x13D4 // OFDM 54M, -, HT(VHT 1SS) MCS 7, - 
        ULONG   TxPwrCfg6;      // MAC 0x13DC // STBC MCS 7, -, -, - 
        ULONG   TxPwrCfg7;      // MAC 0x13D8 // -, -, VHT 1SS MCS 8, VHT 1SS MCS 9
    } BW80Over5GHigh;

    struct
    {
        ULONG   TxPwrCfg0;      // MAC 0x1314 // CCK 1M/2M, CCK 5.5M/11M, OFDM 6M/9M, OFDM 12M/18M
        ULONG   TxPwrCfg1;      // MAC 0x1318 // OFDM 24M/36M, OFDM 48M, HT(VHT 1SS) MCS 0/1, HT(VHT 1SS) MCS 2/3
        ULONG   TxPwrCfg2;      // MAC 0x131C // HT(VHT 1SS) MCS 4/5, HT(VHT 1SS) MCS 6, -, -
        ULONG   TxPwrCfg3;      // MAC 0x1320 // -, -, STBC MCS 0/1, STBC MCS 2/3
        ULONG   TxPwrCfg4;      // MAC 0x1324 // STBC MCS 4/5, STBC MCS 6, -, -
        ULONG   TxPwrCfg5;      // MAC 0x13D4 // OFDM 54M, -, HT(VHT 1SS) MCS 7, - 
        ULONG   TxPwrCfg6;      // MAC 0x13DC // STBC MCS 7, -, -, - 
        ULONG   TxPwrCfg7;      // MAC 0x13D8 // -, -, VHT 1SS MCS 8, VHT 1SS MCS 9
    } BW80Over5GX1;

    // To Do: VHT 2SS/3SS
} TX_POWER_CONTROL_EXT_OVER_MAC_WITH_VHT, *PTX_POWER_CONTROL_EXT_OVER_MAC_WITH_VHT;

#if 1
typedef enum _MP_PORT_TYPE
{
    HELPER_PORT,
    EXTSTA_PORT,
    EXTAP_PORT,
    WFD_DEVICE_PORT,    // Wi-Fi Direct Device
    WFD_GO_PORT,        // Wi-Fi Direct GO
    WFD_CLIENT_PORT,    // Wi-Fi Client
} MP_PORT_TYPE, *PMP_PORT_TYPE;
#else
typedef enum _MP_PORT_TYPE
{
    HELPER_PORT,
    EXTSTA_PORT,
    EXTAP_PORT
} MP_PORT_TYPE, *PMP_PORT_TYPE;
#endif

typedef enum _MP_PORT_SUBTYPE
{
    PORTSUBTYPE_STA,        //  
    PORTSUBTYPE_P2PClient,   //  
    PORTSUBTYPE_VwifiAP,          //  
    PORTSUBTYPE_P2PGO,                  //
    PORTSUBTYPE_SoftapAP,                //
    PORTSUBTYPE_P2PDevice
} MP_PORT_SUBTYPE, *PMP_PORT_SUBTYPE;

typedef enum _P2P_MODE
{
    P2P_MODE_WIN7,
    P2P_MODE_WIN8
} P2P_MODE, *PP2P_MODE;


/*
 * Look at RTMPPortHandleOidRequest for information about this function
 */
typedef NDIS_STATUS (*PORT11_OID_REQUEST_FUNC)(
    IN  PMP_PORT                Port,
    IN  PNDIS_OID_REQUEST       OidRequest
    );

//
//  The miniport PORT structure
//

typedef struct _PORT_COMMON_CONFIG
{
    
    ULONG       OpStatusFlags;

    ULONG       OperationMode;              //DOT11_OPERATION_MODE_EXTENSIBLE_STATION or DOT11_OPERATION_MODE_NETWORK_MONITOR
                            // Wi-Fi Direct Modes: DOT11_OPERATION_MODE_WFD_DEVICE , DOT11_OPERATION_MODE_WFD_GROUP_OWNER, DOT11_OPERATION_MODE_WFD_CLIENT
    UCHAR       Bssid[MAC_ADDR_LEN];
    CHAR        Ssid[MAX_LEN_OF_SSID];      // NOT NULL-terminated
    UCHAR       SsidLen;                    // the actual ssid length in used
    UCHAR       LastSsidLen;                // the actual ssid length in used
    CHAR        LastSsid[MAX_LEN_OF_SSID];  // NOT NULL-terminated
    UCHAR       LastBssid[MAC_ADDR_LEN];        

    BOOLEAN     bMixCipher;                 // Indicate current Pair & Group use different cipher suites
    USHORT      RsnCapability;
    
    ULONG       PacketFilter;               // Packet filter for receiving
    
    BOOLEAN     bHiddenNetworkEnabled;
    BOOLEAN     ExcludeUnencrypted;

    
    ULONG       SelectedPhyId;              // index of PHY that any PHY specific OID is applied to
    ULONG       DefaultPhyId;
    NDIS_802_11_PRIVACY_FILTER          PrivacyFilter;  // PrivacyFilter enum for 802.1X
    UINT        MCAddressCount;
    UCHAR       MCAddressList[HW_MAX_MCAST_LIST_SIZE][DOT11_ADDRESS_SIZE];

}PORT_COMMON_CONFIG,*PPORT_COMMON_CONFIG;

//
//  The miniport PORT structure
//

typedef struct _PORT_CONFIG
{
    ULONG   OpStatusFlags;
    ULONG   OperationMode; //DOT11_OPERATION_MODE_EXTENSIBLE_STATION or DOT11_OPERATION_MODE_NETWORK_MONITOR
    
    UCHAR   Bssid[MAC_ADDR_LEN];
    CHAR    Ssid[MAX_LEN_OF_SSID]; // NOT NULL-terminated
    UCHAR   SsidLen;               // the actual ssid length in used
    UCHAR   LastSsidLen;               // the actual ssid length in used
    CHAR    LastSsid[MAX_LEN_OF_SSID]; // NOT NULL-terminated
    UCHAR   LastBssid[MAC_ADDR_LEN];    
    
    ULONG   AuthMode;     // This should match to whatever microsoft defined, use ULONG Intead of DOT11_AUTH_ALGORITHM.         
    ULONG   WepStatus;      //use ULONG intead of DOT11_CIPHER_ALGORITHM, 
    UCHAR   DefaultKeyId;
    
    ULONG   OrigWepStatus;  // Original wep status set from OID
    // Add to support different cipher suite for WPA2/WPA mode
    ULONG   GroupCipher;        // Multicast cipher suite
    ULONG   PairCipher;         // Unicast cipher suite
    UCHAR       CipherAlg;
    NDIS_802_11_WEP_STATUS  GroupKeyWepStatus;
    RT_802_11_CIPHER_SUITE_TYPE         MixedModeGroupCipher; //for WPA+WEP mixed mode, and CCKM WEP
    BOOLEAN                             bMixCipher;         // Indicate current Pair & Group use different cipher suites
    USHORT                              RsnCapability;
    BOOLEAN                             bAKMwSHA256;        // Add for MFP SHA256 case
    /** Currently enabled unicast cipher algorithm */
    DOT11_CIPHER_ALGORITHM  UnicastCipherAlgorithm;
    DOT11_CIPHER_ALGORITHM  MulticastCipherAlgorithmList[6];
    ULONG                   MulticastCipherAlgorithmCount;

    /** Currently enabled multicast cipher algorithm */                 
    DOT11_CIPHER_ALGORITHM  MulticastCipherAlgorithm;
    ULONG       PacketFilter;       // Packet filter for receiving

    UINT                    MCAddressCount;
    UCHAR                   MCAddressList[HW_MAX_MCAST_LIST_SIZE][DOT11_ADDRESS_SIZE];

    BOOLEAN                 bHiddenNetworkEnabled;
    /** Current setting related to acceptance of unencrypted data */
    BOOLEAN                 ExcludeUnencrypted;
    PDOT11_PRIVACY_EXEMPTION_LIST   PrivacyExemptionList;
    ULONG                           PrivacyExemptionListLen;    

    // Additonal IEs for assoc request
    DOT11_MAC_ADDRESS    AssocIEBSSID;
    PUCHAR          AdditionalAssocReqIEData;
    ULONG           AdditionalAssocReqIESize;

    // Additonal IEs for assoc response
    PUCHAR          AdditionalAssocRspIEData;
    ULONG           AdditionalAssocRspIESize;

    // Additonal IEs for beacon
    PUCHAR          AdditionalBeaconIEData;
    ULONG           AdditionalBeaconIESize;
    
    // Additonal IEs for probe response
    PUCHAR          AdditionalResponseIEData;
    ULONG           AdditionalResponseIESize;

    // Additonal IEs for probe request
    PUCHAR          AdditionalRequestIEData;
    ULONG           AdditionalRequestIESize;

    // Additonal IEs for P2P probe request
    PUCHAR          P2PAdditionalRequestIEData;
    ULONG           P2PAdditionalRequestIESize;

    /** Desired BSSID we should attempt to associate with */    
    // For OID_DOT11_DESIRED_BSSID_LIST 
    DOT11_MAC_ADDRESS   DesiredBSSIDList[STA_DESIRED_BSSID_MAX_COUNT];
    ULONG               DesiredBSSIDCount;
    BOOLEAN             AcceptAnyBSSID;

    DOT11_SSID          DesiredSSID;  // Set by OID_DOT11_DESIRED_SSID_LIST 

#if(COMPILE_WIN8_ABOVE(CURRENT_OS_NDIS_VER))  
    // Secondary device type
    PUCHAR          pWFDSecondaryDeviceTypeList;
    ULONG           WFDSecondaryDeviceTypeLen;
    ULONG           WFDNumOfSecondaryDevice;
#endif
    UCHAR           P2pCapability[2];
    UCHAR           P2pConfigTimeout[2]; // wait peer GO/Client  ready with given ConfigTimeout
}PORT_CONFIG,*PPORT_CONFIG;

typedef struct _RTMP_OID_SIGMA_QUERY_P2P_STATUS_STRUCT 
{
    UCHAR       P2PConnectState;
    UCHAR       MyRule;     // 0: Device, 1:GO, 2: Client
    UCHAR       Bssid[6];
    UCHAR       SsidLen;
    UCHAR       Ssid[32];
    USHORT      ConfigMethod;   // label: 0x04, display:0x08, pbc: 0x80, , keypad: 0x100
    BOOLEAN     bInvitationSucced;  // When DEV and CLI success to receive Invitation Request/Response, this flag will turn TRUE.
    UCHAR       InviteConnectingMAC[6]; // I will go to connect the connecting MAC that finished invitation action.
    UCHAR       ErrorCode;
} RT_OID_SIGMA_QUERY_P2P_STATUS_STRUCT , *PRT_OID_SIGMA_QUERY_P2P_STATUS_STRUCT ;

#define GOSCANBEGINCOUNTER_MAX      1000
typedef struct _P2P_COUNTER_STRUCT 
{
    ULONG       ManageAPEnReconCounter; // right after shutdown, 
    ULONG       Counter100ms;   // plus 1 every 100ms 
    ULONG       GoScanBeginCounter100ms;    //   
    ULONG       Wpa2pskCounter100ms;    //   
    ULONG       NextScanRound;  // Unit : 100 TU
    UCHAR       ListenInterval; // Unit : 100 TU
    ULONG       ListenIntervalBias; // for some important action that need more dwell time in listen channel, add this Bias. Unit : 100 TU
    ULONG       ScanIntervalBias;   // for some important action that need more dwell time in scan channel, add this Bias. Unit : 100 TU
    ULONG       ScanIntervalBias2;  // additional scan bias from Win8 Ndis. Unit : 100 TU
    ULONG       CounterAftrScanButton;      // Unit 100ms. Counter for After Pressing "Apply" Profile button.
    ULONG       CounterAftrSetEvent;        // Unit 100ms. Counter for Driver called KeSetEvent.
    ULONG       CounterAftrWpsDone;     // Unit 100ms. Counter for Driver to set WPS_Done event.
    ULONG       CounterAfterConnectCommand;     // Unit 100ms
    ULONG       CounterAfterSetWscMode; // Unit 100ms. Blocking autoreconnection during this period (Virtual Port Only)
    ULONG       TopologyUpdateCounter;      //  Unit 100ms.
    ULONG       CounterAutoReconnectForP2pClient; // Plus 1 every 3 secs
    ULONG       HoldAtNegoDoneClinetTimes;
    ULONG       CounterP2PConnectionFail;
    UCHAR       CounterP2PAcceptReinvoke; // Unit 100ms. accept a peer auto reconne back at most 5 seconds
    ULONG       CounterAfterMSSendCommand;// [Win8] Conuter After MicroSoft Send packet command
    ULONG       SwitchingChannelCounter; // [Win8] Conuter After received the response wtih an error
    ULONG       QuickDiscoveryListUpdateCounter;    // [Win8] Unit 100ms
    ULONG       StartRetryCounter;      // [Win8] Start the counter for retrying action frame  
} P2P_COUNTER_STRUCT, *PP2P_COUNTER_STRUCT;

#define ICS_STATUS_ENABLED      1
#define ICS_STATUS_DISABLED     0

typedef struct _P2P_MANAGED_STRUCT 
{
    UCHAR           ManageAPBSsid[MAC_ADDR_LEN];    // Store the AP's BSSID that is the managed AP and is the latest one that I recently connected to..
    UCHAR           ManageAPSsid[32];   // Store the AP's BSSID that is the managed AP and is the latest one that I recently connected to..
    UCHAR           ManageAPSsidLen;    // Store the AP's BSSID that is the managed AP and is the latest one that I recently connected to..
    UCHAR           CountryContent[6];
    UCHAR           APP2pManageability; // Store the AP's P2P Manageability byte that currently I want to associate.
    UCHAR           APP2pMinorReason;   // Store the AP's minorreason byte that recently being deauthed
    UCHAR           ICSStatus;  // 
    UCHAR           APUsageChannel; //  The suggest channel from AP's IE=0x61
    UCHAR           TotalNumOfP2pAttribute;
} P2P_MANAGED_STRUCT, *PP2P_MANAGED_STRUCT;

#define P2PMANAGED_ENABLE_BIT       0x1
#define P2PMANAGED_ICS_ENABLE_BIT   0x2
#define P2PMANAGED_COEXIST_OPT_BIT  0x4

#define P2P_OPPS_BIT        0x80

// WIN8: P2P Connected Status on role ports
#define P2pMs_CONNECTED_STATUS_OFF                  0x00    // No any p2p connection
#define P2pMs_CONNECTED_STATUS_GO_ASSOC             0x01    // I am GO and client associates to me
#define P2pMs_CONNECTED_STATUS_CLI_ASSOC            0x02    // I am CLI and associated to GO
#define P2pMs_CONNECTED_STATUS_CLI_OP               0x04    // Client operation
#define P2pMs_CONNECTED_STATUS_GO_OP                0x08    // GO operation
#define P2pMs_CONNECTED_STATUS_GO_ON                0x10    // GO is in op/init state control

// WIN8: P2P flags to wait a completed indication
#define P2pMs_WAIT_COMPLETE_NOUSE                   0x00
#define P2pMs_WAIT_COMPLETE_DISCO_REQ               0x01
#define P2pMs_WAIT_COMPLETE_NEGO_REQ                0x02
#define P2pMs_WAIT_COMPLETE_NEGO_RSP                0x04
#define P2pMs_WAIT_COMPLETE_NEGO_COM                0x08
#define P2pMs_WAIT_COMPLETE_INV_REQ                 0x10
#define P2pMs_WAIT_COMPLETE_INV_RSP                 0x20
#define P2pMs_WAIT_COMPLETE_PRO_REQ                 0x40
#define P2pMs_WAIT_COMPLETE_PRO_RSP                 0x80
#define P2pMs_WAIT_COMPLETE_PRO_REQ_BUFFERED        0x100
#define P2pMs_WAIT_COMPLETE_INV_REQ_BUFFERED        0x200
#define P2pMs_WAIT_COMPLETE_NEGO_REQ_BUFFERED       0x400
#define P2pMs_WAIT_COMPLETE_QUICK_DISCO_REQ         0x800
#define P2pMs_WAIT_COMPLETE_NEGO_REQ_BUFFERED_AFTER_SCAN    0x1000

#define P2pMs_CLIENT_1_INUSED                       0x01
#define P2pMs_CLIENT_2_INUSED                       0x02

//
// Mulit-channel flag
//
// End task
#define MULTI_CHANNEL_GO_STOP                                   0x00000001  // GO stop, trigger a defer task to handle the multi-channel state
#define MULTI_CHANNEL_CLI_STOP                              0x00000002  // CLI stop, trigger a defer task to handle the multi-channel state
#define MULTI_CHANNEL_STA_STOP                              0x00000004  // STA stop, trigger a defer task to handle the multi-channel state
#define MULTI_CHANNEL_IN_PROCESS                                0x00000008  // Multi-channel is in process

// Start task after 4-way
#define MULTI_CHANNEL_GO_NEW_CONNECTION_START               0x00000010  // GO start
#define MULTI_CHANNEL_CLI_OUT_NEW_CONNECTION_START      0x00000020  // CLI connects to peer GO, trigger a defer task to handle DHCP
#define MULTI_CHANNEL_CLI_IN_NEW_CONNECTION_START           0x00000040  // GO is connected by peer CLI, trigger a defer task to handle DHCP
#define MULTI_CHANNEL_STA_NEW_CONNECTION_START          0x00000080  // STA connects to peer AP, trigger a defer task to handle DHCP
// Failure case
#define MULTI_CHANNEL_NEW_P2P_CONNECTION_FAIL               0x00000100  // STA connects to peer AP, trigger a defer task to handle DHCP

#define MULTI_CHANNEL_STA_PRE_STOP                          0x00000200
#define MULTI_CHANNEL_CLI_PRE_STOP                          0x00000400
#define MULTI_CHANNEL_GO_PRE_STOP                           0x00000800

#define MULTI_CHANNEL_STA_PRE_START                         0x00001000
#define MULTI_CHANNEL_CLI_PRE_START                         0x00002000
#define MULTI_CHANNEL_GO_PRE_START                          0x00004000
#define MULTI_CHANNEL_CLI_DEBUG                             0x00008000

#define MULTI_CHANNEL_MSDU_TX_SUSPEND                       0x00010000
#define MULTI_CHANNEL_STA_DHCP_DONE                         0x00020000
#define MULTI_CHANNEL_CLI_DHCP_DONE                         0x00040000
#define MULTI_CHANNEL_GO_DHCP_DONE                          0x00080000

#define MULTI_CHANNEL_STA_PRE_RESET_REQ                     0x00100000  // trigger by OS
#define MULTI_CHANNEL_CLI_PRE_RESET_REQ                     0x00200000
#define MULTI_CHANNEL_GO_PRE_RESET_REQ                      0x00400000
#define MULTI_CHANNEL_IN_NEW_SESSION                            0x00800000

#define MULTI_CHANNEL_EXT_SCAN_PRE_START                    0x01000000

//
//  Macros for bit check
//
#define MT_SET_BIT(_M, _F)       ((_M) |= (_F))
#define MT_CLEAR_BIT(_M, _F)     ((_M) &= ~(_F))
#define MT_TEST_BIT(_M, _F)      (((_M) & (_F)) != 0)
#define MT_TEST_BITS(_M, _F)     (((_M) & (_F)) == (_F))

typedef struct _P2P_EVENTQUEUE_STRUCT 
{
    BOOLEAN     bDisableIcs;     // Sometimes can't send Probe Response
    BOOLEAN     bDiscoveryUpdate;    //  Notify GUI need to update discovery list
    BOOLEAN     bP2pTopologyUpdate;  // Sometimes can't send Probe Response
    BOOLEAN     bP2pGoNegoDone;  //  Notify GUI need to update discovery list
    BOOLEAN     bP2pGenSeudoAdhoc;   // Sometimes can't send Probe Response
    BOOLEAN     bEnableDhcp;     // Sometimes can't send Probe Response
    BOOLEAN     bDisableDhcp;    // Sometimes can't send Probe Response
    BOOLEAN     bDeleteWirelessCon;  // Sometimes can't send Probe Response
    BOOLEAN     bWpsNack;
    BOOLEAN     bWpsDone;    // This event should be delayed until receive the disassociate of WPS connection. NOT show in EVentMaintain.
    BOOLEAN     bSwitchToSTA;
    BOOLEAN     bSwitchToConcurrent;    
    BOOLEAN     bSigmaIsOn;  // Sometimes can't send Probe Response
    BOOLEAN     bErrorCode;
    BOOLEAN     bWpaDone;
    BOOLEAN     bPopWindowForGO;
    BOOLEAN     bPopWindow;
    BOOLEAN     bShowPinCode;
    UCHAR       LastSetEvent;   // to store the last event being set, need to setthis event again if GUI miss it.
} P2P_EVENTQUEUE_STRUCT, *PP2P_EVENTQUEUE_STRUCT;

#define P2P_INC_CHA_INDEX(_idx, _RingSize)    \
{                                          \
    (_idx)++;                              \
    if ((_idx) >= (_RingSize)) _idx=0;     \
}
//  channel 1, 6, 11 is listen channel.
#define P2P_LISTEN_CHA_SIZE     3

typedef struct _P2P_RALINK_STRUCT 
{
    UCHAR       ListenChanel[MAX_NUM_OF_CHANNELS + 3];   
    UCHAR       ListenChanelIndex;
    UCHAR       ListenChanelCount;
    UCHAR       ChannelScanRound;
} P2P_RALINK_STRUCT, *PP2P_RALINK_STRUCT;

// often need to change beacon content. So save the offset.
typedef struct _P2P_BEACONOFFSET_STRUCT 
{
//  ULONG               IpBitmapBcnOffset;      //  USed as GO
    ULONG               P2pCapOffset;       //  
} P2P_BEACONOFFSET_STRUCT, *PP2P_BEACONOFFSET_STRUCT;

typedef struct  _P2P_SAVED_PUBLIC_FRAME {
    HEADER_802_11   p80211Header;
    UCHAR          Category;
    UCHAR           Action;
    UCHAR           OUI[3];
    UCHAR       OUIType;
    UCHAR       Subtype;
    UCHAR       Token;
}   P2P_SAVED_PUBLIC_FRAME, *PP2P_SAVED_PUBLIC_FRAME;

typedef struct  P2P_SIGMA_SETTING_STRUCT    {
    BOOLEAN     bWaitGoNegReq;
    USHORT      MyConfigMethod;
    UCHAR       ConfigMode;
    UCHAR       StartNoaWhenGO;
    UCHAR       CTWindow;
}   P2P_SIGMA_SETTING_STRUCT, *PP2P_SIGMA_SETTING_STRUCT;

typedef struct _RT_P2P_CHANNEL_ENTRY_LIST
{
    UCHAR   RegClass;
    UCHAR   ChannelNr;
    //PUCHAR    Channel;
    UCHAR   Channel[MAX_NUM_OF_CHANNELS];
} P2P_CHANNEL_ENTRY_LIST, *PP2P_CHANNEL_ENTRY_LIST;

// Store for persistent P2P group
typedef struct _RT_P2P_CONFIG 
{
    P2P_DISCOPROV_STATE     P2PDiscoProvState;      // P2P State for P2P discovery(listen, scan, search) and P2P provistioning (Service provision, wps provision)
    P2P_CONNECT_STATE       P2PConnectState;        // P2P State for P2P connection.
    P2P_CHANNEL_STATE       P2PChannelState;        // P2P State for P2P current dwelling channel.
    P2P_EVENTQUEUE_STRUCT   P2pEventQueue;
    P2P_COUNTER_STRUCT      P2pCounter;
    P2P_RALINK_STRUCT       P2pProprietary; // Ralink Proprietary to improve usage.
    P2P_BEACONOFFSET_STRUCT P2pBcnOffset;
    P2P_SAVED_PUBLIC_FRAME  LatestP2pPublicFrame;   // Latest received P2P Public frame
    P2P_MANAGED_STRUCT      P2pManagedParm;
    ULONG                   DiscoverType;
    P2P_SCAN_TYPE           ScanType;
    P2pMs_STATE_PHASE       P2pMsSatetPhase;
    ULONG                   P2pMsConnectedStatus;
    ULONG                   DiscoverTriggerType;

    BOOLEAN     bFirstTimeCancelOpps;
    BOOLEAN     bKeepSlient;
    BOOLEAN     bPreKeepSlient;
    BOOLEAN     bAvailability;
    MLME_SCAN_REQ_STRUCT    QueuedScanReq;
    ULONG       GroupFormErrCode;
    ULONG       ScanPeriod;     //scan duration in unit of 100ms
    UCHAR       MyGOwcid;
    UCHAR       LastSentInviteFlag;
    UCHAR       P2pCapability[2];
    UCHAR       WPSVersionsEnabled; // 1 for WPS 1.0, 2 for WPS 2.0

    UCHAR               PortNumber;     // The port number that is P2P. Device port in Win8
    UCHAR               GOPortNumber;       // Role port(GO) number
    UCHAR               ClientPortNumber;   // Role port(Client#1) number
    UCHAR               Client2PortNumber;  // Role port(Client#2) number

    ULONG       MyIp;           //
    ULONG       WscMode;        // predefined WSC mode, 1: PIN, 2: PBC
    BOOLEAN     bConfiguredAP;  // true (1:un-configured), false(2: configured) ; used by GO
    USHORT      Dpid;           // WPS device password ID.
    USHORT      ConfigMethod;   // WPS device password ID.
    UCHAR       PinCode[8];
    UCHAR       PINLen;
    UCHAR       ConfigTimeout[2];
    UCHAR       PhraseKey[64];  // Temporary psk for the current association. Might be used by legacy STA or persistent CLI.
    UCHAR       PhraseKeyLen;
    UCHAR       LegacyPSK[64];  // Store unique psk for legacy STA.
    UCHAR       LegacyPSKLen;
    BOOLEAN     PhraseKeyChanged;
    UCHAR       P2pPhyMode;
    UCHAR       NoAIndex;
    UCHAR       PopUpIndex; // store the p2p entry index when receiving Go Nego Req and need a GUI to pop up a window to set connection.
    UCHAR       ConnectingIndex;    // Point to the ?rd Connecting MAC that I will try to connect with.
    UCHAR       ConnectingMAC[MAX_P2P_GROUP_SIZE][MAC_ADDR_LEN];  // Specify MAC address want to connect. Set to all 0xff or all 0x0 if not specified.
    UCHAR       ConnectingListenChannel;
    UCHAR       ConnectingDeviceName[MAX_P2P_GROUP_SIZE][32];  // Specify the Device Name that want to connect. Set to all 0xff or all 0x0 if not specified.
    UCHAR       SSID[32]; // Ssid in GroupID
    UCHAR       SSIDLen;
    UCHAR       Token;
    UCHAR       p2pidxForServiceCbReq;
    UCHAR       ServiceTransac;
    UCHAR       Bssid[MAC_ADDR_LEN];
    UCHAR       DeviceAddress[MAC_ADDR_LEN]; // Win8: Device address in GroupID
    UCHAR       GroupDevAddr[MAC_ADDR_LEN]; // Device address in GroupID.We use the same Bssid and Device address on our GO, but other vendors might not.
    UCHAR       DeviceName[32];
    ULONG       DeviceNameLen;
    UCHAR       PriDeviceType[8]; // Used in Win8   
    UCHAR       ListenChannel;   // The channel that perform the group formatuib oricedure.
    UCHAR       GroupChannel;    // Group setting channel from GUI. Real OP channel need to be negociated..
    UCHAR       GroupOpChannel;  // Group operating channel.
    UCHAR       CentralChannel;
    UCHAR       GoIntentIdx;    // Value = 0~15. Intent to be a GO in P2P
    HANDLE      P2pEvent;       // this event is created in UI.  informing driver by MTK_OID_N6_SET_P2P_EVENT
    USHORT      ExtListenPeriod;    // Period for extended listening
    USHORT      ExtListenInterval;  // Interval for extended listening
    UCHAR       CTWindows;  // CTWindows and OppPS parameter field
    ULONG       GOBeaconBufWscLen;
    UCHAR       LastConfigMode; // Disable, activate p2p, or activate WPSE, or delete p2p profile.
    BOOLEAN     P2pGOAllPsm; // All of cleints connect in power saving mode
    BOOLEAN     bOppsOn;
    BOOLEAN     bNoAOn;
    BOOLEAN     bPendingNullFrame;
    BOOLEAN     bP2pAckReq;
    UCHAR       DevDisReqCount;
    BOOLEAN     bSigmaFakePkt;
    BOOLEAN     bSetProfile;
#if UAPSD_AP_SUPPORT
    BOOLEAN     LastMoreDataFromGO[WMM_NUM_OF_AC];  // wmm-ps used by p2p Client
#endif
    UCHAR       PersistentGOAddr[MAC_ADDR_LEN];
    ULONG       EapolLastSecquence;
    P2PCLIENT_NOA_SCHEDULE          GONoASchedule;
    RT_OID_QUERY_P2P_STATUS_STRUCT  P2PQueryStatusOid;
    MTK_TIMER_STRUCT             P2pCTWindowTimer; // 
    MTK_TIMER_STRUCT             P2pStopGoTimer; // 
    MTK_TIMER_STRUCT             P2pStopClientTimer; //
    MTK_TIMER_STRUCT             P2pStopWpsTimer; // 
    MTK_TIMER_STRUCT             P2pSwNoATimer; // 
    MTK_TIMER_STRUCT             P2pPreAbsenTimer; // 

    //for Client Port if no SSID exit int scan table ,just scan once
    BOOLEAN             bGroupsScanOnce;
    BOOLEAN             bGOStart;   // Set to true when GO start to send beacons. Support only one GO.

    /* Sigma Configurations*/
    RT_OID_SIGMA_QUERY_P2P_STATUS_STRUCT    SigmaQueryStatus;       
    P2P_SIGMA_SETTING_STRUCT                SigmaSetting;   
    ULONG       LinkState;      // 0: not p2p link up
    BOOLEAN     LockNego;
    
#if(COMPILE_WIN8_ABOVE(CURRENT_OS_NDIS_VER))  
    PUCHAR      pSendOutBufferedFrame;  // bufferring sendout packet
    USHORT      SendOutBufferedSize;
    PUCHAR      pRequesterBufferedFrame; // Requester buffers the first packet and send it out as a delay , like ProvisionRequest and InvitationRequest
    USHORT      RequesterBufferedSize;
    UCHAR       WFDStatus;
    PUCHAR      pPobeReqBuffer;
    ULONG       PobeReqBufferLen;
    BOOLEAN     SetPobeReqTimer;
    MTK_TIMER_STRUCT                     P2pSendProReqTimer; //
#ifdef WFD_NEW_PUBLIC_ACTION
    PUCHAR      pP2PPublicActionFrameData;
    USHORT      P2PPublicActionFrameDataLen;
#endif
    
    // Below parameters are for OID SPEC V1.4
    DOT11_DIALOG_TOKEN      MSDialogToken;
    BOOLEAN                 InGroupFormation;
    BOOLEAN                 WaitForWPSReady;
    BOOLEAN                 ReadyToConnect;
    DOT11_WFD_CHANNEL       GoNOpChannel;           // This op channel is the result after GoN and prvoide by OS
    UCHAR                   GoIOpChannel;           // This op channel is the result after Invitation Request and prvoide by OS
    
    ULONG                   WaitForMSCompletedFlags;
    ULONG                   MSSendTimeout;

    // filter scan and filter lists
    UCHAR                   ScanWithFilter; 
    //UCHAR                 NumberOfFilterDevId;            
    //UCHAR                 FilterDevId[MAX_P2P_FILTER_LIST][MAC_ADDRESS_LENGTH];

    ULONG                   uNumDeviceFilters;
    DOT11_WFD_DISCOVER_DEVICE_FILTER    DeviceFilterList[MAX_P2P_DISCOVERY_FILTER_NUM];

    UCHAR                   NumberOfFilterDevType;
    UCHAR                   FilterDevType[MAX_P2P_FILTER_LIST][P2P_DEVICE_TYPE_LEN];    
    UCHAR                   NumberOfFilterSsid;         
    UCHAR                   FilterSsid[MAX_P2P_FILTER_LIST][MAX_LEN_OF_SSID];
    
    BOOLEAN                 IsValidProprietaryIE; // TRUE: WFDProprietaryIEs as valid WSC IE (patched Win8 issue)   
    USHORT                  WFDProprietaryIEsLen;
    UCHAR                   WFDProprietaryIEs[512]; // WFD proprietary IEs in probe request, only for device port
    BOOLEAN                 RandomListenChTestBitOn;
    BOOLEAN                 SwitchChannelInProgress;
    BOOLEAN                 RestoreChannelInProgress;
    ULONG                   DeviceDiscoverable;
    ULONG                   MaximumGroupLimit;
    UCHAR                   ClientUseBitmap;// bit0: the first client, bit1: the second client
    
   
    // Commsom set channel list
    P2P_CHANNEL_ENTRY_LIST  CommcomSetChannelList;
#endif

#ifdef MULTI_CHANNEL_SUPPORT
    UCHAR                   InfraChannel;           // STA 20M
    UCHAR                   P2PChannel;             // CLI 20M --> CLI's group
    UCHAR                   P2PGOChannel;           // GO 20M --> pPort->P2PCfg.GroupOpChannel --> GO's group (This is replaced by pPort->P2PCfg.GroupOpChannel)
    //ULONG                   MultiChannelStartOrEndProcess;          // Start of end process
    UCHAR                   STAHwQSEL;
    UCHAR                   CLIHwQSEL;
    UCHAR                   GOHwQSEL;
    ULONG                   SwitchChTime;   
    ULONG                   PwrSaveDelayTime;   
    ULONGLONG               STABaseTime;
    ULONGLONG               CLIBaseTime;
    ULONGLONG               STAOffsetime;
    ULONGLONG               CLIOffsetTime;
    UCHAR                   CurrentWcid;
#endif /* MULTI_CHANNEL_SUPPORT */  

    UCHAR                   InfraCentralChannel;        // STA 40M
    UCHAR                   P2PCentralChannel;      // CLI 40M --> pPort->P2PCfg.CentralChannel
    UCHAR                   P2PGOCentralChannel;    // GO 40M
    UCHAR                   InfraExtChanOffset;     // ExtChanOffset for 40M control, default is 0xff
    UCHAR                   P2PExtChanOffset;       // ExtChanOffset for 40M control, default is 0xff
    UCHAR                   P2PGOExtChanOffset;     // ExtChanOffset for 40M control, default is 0xff
    ULONGLONG               LastFlushBssTime;     // OS's timestamp of the LastFlushBssTime time
    ULONGLONG               LastEnumBssTime;     // OS's timestamp of the LastEnumBssTime time

#ifdef WFD_NEW_PUBLIC_ACTION
    UCHAR                   PrevState;
    UCHAR                   CurrState;
    UCHAR                   Signal;
    UCHAR                   SentFrame;
    UCHAR                   PrevSentFrame;
    UCHAR                   LostAckRetryCounter;
    UCHAR                   LostRspRetryCounter;
#endif /*WFD_NEW_PUBLIC_ACTION*/
        
    ULONG                   TbttNextTargetTimePoint;
    UCHAR                   P2PMode;
} RT_P2P_CONFIG, *PRT_P2P_CONFIG;

typedef struct _RT_P2P_CHANNEL_ENTRY
{
    UCHAR   RegClass;
    UCHAR   ChannelNr;
    PUCHAR  Channel;
} RT_P2P_CHANNEL_ENTRY, *PRT_P2P_CHANNEL_ENTRY;



#define P2P_IS_GO           1
#define P2P_IS_CLIENT       2
#define P2P_IS_DEVICE       3

//

#define P2P_SET_FLAG(_M, _F)       ((_M)->P2pFlag |= (_F))
#define P2P_CLEAR_FLAG(_M, _F)     ((_M)->P2pFlag &= ~(_F))
#define P2P_TEST_FLAG(_M, _F)      (((_M)->P2pFlag & (_F)) != 0)
#define P2P_TEST_FLAGS(_M, _F)     (((_M)->P2pFlag & (_F)) == (_F))

#define P2PFLAG_PROVISIONED         0x01
#define P2PFLAG_INVITE_ENABLED      0x02
#define P2PFLAG_DEVICE_DISCOVERABLE 0x04
#define P2PFLAG_PASSED_NEGO_REQ     0x08
#define P2PFLAG_GO_INVITE_ME        0x10

typedef struct  _P2P_NOA_DESC   {
    UCHAR   Count;
    UCHAR   Duration[4];
    UCHAR   Interval[4];
    UCHAR   StartTime[4];
}   P2P_NOA_DESC, *PP2P_NOA_DESC;


typedef struct _RT_P2P_CLIENT_ENTRY
{
    P2P_CLIENT_STATE        P2pClientState; // From the state, can know peer it registra or enrollee.
    UCHAR   MyGOIndex;  // If this device is a client in a P2P group, then this is its GO's table index.
    ULONG   Peerip;
    UCHAR   P2pFlag;
    UCHAR   NoAToken;
    UCHAR   FrameToken;
    UCHAR   GeneralToken;
    UCHAR   StateCount;
    UCHAR   Rule;       // 
    UCHAR   DevCapability;  // table 10 
    UCHAR   GroupCapability;    //  
    USHORT  ConfigMethod;   //  
    UCHAR   PIN[8];
    USHORT  Dpid;
    UCHAR   Ssid[MAX_LEN_OF_SSID];
    UCHAR   SsidLen;
    UCHAR   GoIntent;
    UCHAR   addr[MAC_ADDR_LEN]; // device address
    UCHAR   InterfaceAddr[MAC_ADDR_LEN];
    UCHAR   bssid[MAC_ADDR_LEN];
    UCHAR   PrimaryDevType[P2P_DEVICE_TYPE_LEN];
    UCHAR   NumSecondaryType;
    UCHAR   SecondaryDevType[MAX_P2P_SECONDARY_DEVTYPE_LIST][P2P_DEVICE_TYPE_LEN];
    UCHAR   RegClass;
    UCHAR   ChannelNr;
    UCHAR   OpChannel;
    UCHAR   DeviceName[P2P_DEVICE_NAME_LEN];
    ULONG   DeviceNameLen;
    UCHAR   ConfigTimeOut;
    UCHAR   ListenChannel;
    ULONG   WscMode;
    USHORT  ExtListenPeriod;    // Period for extended listening
    USHORT  ExtListenInterval;  // Interval for extended listening
    UCHAR   CTWindow;   // As GO, Store client's Presence request NoA.  As Client, Store GO's NoA In beacon or P2P Action frame
    UCHAR   GODevDisWaitCount;      // Period of GO waiting for Device Discovery Acknowledgement (unit: 100ms)
//  P2PCLIENT_NOA_SCHEDULE  NoADesc[1]; // As GO, Store client's Presence request NoA.  As Client, Store GO's NoA In beacon or P2P Action frame
} RT_P2P_CLIENT_ENTRY, *PRT_P2P_CLIENT_ENTRY;

typedef struct _RT_P2P_DISCOVERY_ENTRY
{
    BOOLEAN     bValid;
    UCHAR       Bssid[MAC_ADDR_LEN];
    UCHAR       BssType;
    UCHAR       TransmitterAddr[MAC_ADDR_LEN];  
    LONG        Rssi;
    USHORT      BeaconPeriod;
    ULONGLONG   BeaconTimestamp;
    ULONGLONG   ProbeRspTimestamp;
    USHORT      CapabilityInfo;
    UCHAR       BeaconFrame[MAX_LEN_OF_MLME_BUFFER];
    USHORT      BeaconFrameLen;
    UCHAR       ProbeRspFrame[MAX_LEN_OF_MLME_BUFFER];
    USHORT      ProbeRspFrameLen;
    UCHAR       ListenChannel;
    UCHAR       OpChannel;  // GO's operation channel
    LONG        LifeTime;
    BOOLEAN     bLegacyNetworks;
    UCHAR       GroupChannel; // Use in NEGO phase
    UCHAR       DevCapability;
    UCHAR       GroupCapability;    
    
    // filtering
    UCHAR       DeviceId[MAC_ADDR_LEN];
    UCHAR       Ssid[MAX_LEN_OF_SSID];
    UCHAR       SsidLen;
    UCHAR       PrimaryDevType[P2P_DEVICE_TYPE_LEN];
    UCHAR       NumSecondaryType;
    UCHAR       SecondaryDevType[MAX_P2P_SECONDARY_DEVTYPE_LIST][P2P_DEVICE_TYPE_LEN];

}   RT_P2P_DISCOVERY_ENTRY, *PRT_P2P_DISCOVERY_ENTRY;

typedef struct _OID_P2P_PERSISTENT_TABLE
{
    UCHAR  PerstNumber;  // What persistent profile is set ?
    RT_P2P_PERSISTENT_ENTRY  PerstEntry[MAX_P2P_TABLE_SIZE]; // Save persistent profile for auto reconnect
} OID_P2P_PERSISTENT_TABLE, *POID_P2P_PERSISTENT_TABLE;

// Save for "Persistent" P2P table. Temporary P2P doesn't need to save in the table.
typedef struct _RT_GO_CREDENTIAL_ENTRY
{
    BOOLEAN     bValid;
    UCHAR       InterAddr[MAC_ADDR_LEN];        // this addr is to distinguish this persistent entry is for which mac addr  
    WSC_CREDENTIAL  Profile;                //  profile's bssid is always the GO's bssid. 
} RT_GO_CREDENTIAL_ENTRY, *PRT_GO_CREDENTIAL_ENTRY;

// Store for persistent P2P group
typedef struct _RT_P2P_TABLE 
{
    UCHAR       PerstNumber;        // What persistent profile is set ?
    UCHAR       ClientNumber;    // What clients are in my group now?   
    UCHAR       DiscoNumber;    // [WFD] What discovery number are in my group now? 
    RT_P2P_PERSISTENT_ENTRY     PerstEntry[MAX_P2P_TABLE_SIZE]; // Save persistent profile for auto reconnect
    RT_P2P_CLIENT_ENTRY         Client[MAX_P2P_GROUP_SIZE]; // Store for current group member.
    RT_GO_CREDENTIAL_ENTRY      TempCredential[MAX_P2P_SAVECREDN_SIZE];
    RT_P2P_DISCOVERY_ENTRY      DiscoEntry[MAX_P2P_DISCOVERY_SIZE]; // [WFD] Store for current discovery results by each transmitter 
} RT_P2P_TABLE, *PRT_P2P_TABLE;

typedef struct _SOFT_AP
{
    // -----------------------------------------------
    // AP specific configuration & operation status
    // used only when pAd->OpMode == OPMODE_AP
    // -----------------------------------------------
    BOOLEAN                 bAPStart;
    AP_ADMIN_CONFIG         ApCfg;             // user configuration when in AP mode
    AP_MLME_AUX             ApMlmeAux;
    RT_802_11_ACL           AccessControlList;   
} SOFT_AP, *PSOFT_AP;


//
//  The miniport PORT structure
//

typedef struct _MP_PORT
{
    PMP_ADAPTER         pAd;
    NDIS_HANDLE           AdapterHandle;          // Adapetr Context
    MP_PORT_TYPE          PortType;   // Used by Port, ChildPort & MP
    MP_PORT_SUBTYPE       PortSubtype;
    NDIS_PORT_NUMBER      PortNumber; // NDIS allocate port number
    ULONG OpModeMask;
    //port active
    BOOLEAN                 bActive;
    
    ULONG       ApState;
    MP_DOT11_STATE State;
    UCHAR                   CurrentAddress[MAC_ADDR_LEN];      // User changed MAC address

    ULONG                               AutoConfigEnabled;
    
    BA_TABLE            BATable;
    NDIS_SPIN_LOCK          BATabLock;
    NDIS_SPIN_LOCK          BADeleteRECEntry;   
    // resource for software backlog queues
    RTMP_RXBUF              LocalRxReorderBuf[Max_BARECI_SESSION];
    // RX re-assembly buffer for fragmentation
    FRAGMENT_FRAME          FragFrame;                  // Frame storage for fragment frame
    
    UCHAR       Channel;
    UCHAR       CentralChannel;
    UCHAR       BBPCurrentBW;
    UCHAR       ScaningChannel;

    EXT_CMD_CHAN_SWITCH_T   SwitchChannelSturct;
    
    COMMON_CONFIG           CommonCfg;
    //PORT_COMMON_CONFIG CommonCfg;

    PORT_CONFIG           PortCfg;
    
    // encryption/decryption KEY tables 
    CIPHER_KEY              SharedKey[4][4];        // STA always use SharedKey[BSS0][0..3],  4+ Mapping key.
    
    PER_STA_CIPHER_KEY      PerStaKey[PER_STA_KEY_TABLE_SIZE];

    BOOLEAN                 bStartJoin;             // used for IBSS adhoc.

    STA_ADMIN_CONFIG        StaCfg;           // user desired settings
    
    //PP2P_CTRL_T             pP2pCtrll;
    RT_P2P_CONFIG           P2PCfg;

    SOFT_AP     SoftAP;
    MAC_TABLE               MacTab;     // ASIC on-chip WCID entry table.  At TX, ASIC always use key according to this on-chip table.
    UCHAR                   PerStaKeyCount;
    
    /** Currently enabled unicast cipher algorithm */
    DOT11_CIPHER_ALGORITHM  UnicastCipherAlgorithm;
    /** Currently enabled multicast cipher algorithm */                                 
    DOT11_CIPHER_ALGORITHM  MulticastCipherAlgorithm;
    ULONG                   MulticastCipherAlgorithmCount;

    MLME_STRUCT             Mlme;   
    
    /** Desired PHY ID list */
    ULONG                   DesiredPhyList[AP_DESIRED_PHY_MAX_COUNT];
    ULONG                   DesiredPhyCount;

    /** Current setting related to acceptance of unencrypted data */
//  BOOLEAN                 ExcludeUnencrypted; // move to PortCfg
//  PDOT11_PRIVACY_EXEMPTION_LIST   PrivacyExemptionList;

    PDOT11_SUPPORTED_PHY_TYPES          pSupportedPhyTypes;
    
    //ULONG                 LastTxRate;
    //ULONG                 LastRxRate;
    PHY_CFG                 LastTxRatePhyCfg;
    PHY_CFG                 LastRxRatePhyCfg;
    
    ULONG64                   LastIndicateRate;
    UCHAR LastLinkQuality;

    PNDIS_OID_REQUEST DeferredOidRequest;

    //Work item for handling OIDs
    NDIS_HANDLE OidWorkItem;

    BSS_ENTRY CurrentBssEntry;
    BOOLEAN bRoamingAfterResume;

    // pre-build PS-POLL and NULL frame upon link up. for efficiency purpose.
    PSPOLL_FRAME            PsPollFrame;
    HEADER_802_11           NullFrame;

    //
    BOOLEAN bBeaconing;
    ULONG HwBeaconBase;

    ULONG   OwnMACAddressIdx;
}MP_PORT,*PMP_PORT;

//
// 802.11b CCK TSSI information
//
typedef union _CCK_TSSI_INFO
{
    struct
    {
        UCHAR   TxType:2;
        UCHAR   Tx40MSel:2;
        UCHAR   Rate:2;
        UCHAR   ShortPreamble:1;
        UCHAR   Reserved:1;
    } field;

    UCHAR   value;
} CCK_TSSI_INFO, *PCCK_TSSI_INFO;

//
// 802.11a/g OFDM TSSI information
//
typedef union _OFDM_TSSI_INFO
{
    struct
    {
        UCHAR   TxType:2;
        UCHAR   Tx40MSel:2;
        UCHAR   Rate:4;
    } field;

    UCHAR   value;
} OFDM_TSSI_INFO, *POFDM_TSSI_INFO;

//
// 802.11n HT TSSI information
//
typedef union _HT_TSSI_INFO
{
    union
    {
        struct
        {
            UCHAR   TxType:2;
            UCHAR   Tx40MSel:2;
            UCHAR   Aggregation:1;
            UCHAR   STBC:2;
            UCHAR   SGI:1;
        } field;

        UCHAR   value;
    } PartA;

    union
    {
        struct
        {
            UCHAR   MCS:7;
            UCHAR   BW:1;
        } field;

        UCHAR   value;
    } PartB;
} HT_TSSI_INFO, *PHT_TSSI_INFO;

//
// The Tx power control using the internal ALC
//
typedef struct _TX_POWER_CONTROL
{
    BOOLEAN bInternalTxALC; // Internal Tx ALC

    //
    UCHAR LookupTableSize;
    UCHAR LookupTableOffset;

    ULONG CoefficientAlcRefValue; // Coefficient for 2.4 internal ALC reference value (in percentage)

    CHAR CurrentTemperature; // Current temperature
    ULONG CurrentTemperatureState; // Current temperature state (TEMPERATURE_STATE_XXX)
    UCHAR RfB7R73; // RF B7.R73
    UCHAR RfB7R28; // RF B7.R28
    UCHAR RfB0R66; // RF B0.R66
    UCHAR RfB0R67; // RF B0.R67

    PTX_POWER_CONTROL_EXT pTxPowerCtrlExt;
    
} TX_POWER_CONTROL, *PTX_POWER_CONTROL;

//
// The upperbound/lowerbound of the frequency offset
//
#define UPPERBOUND_OF_FREQUENCY_OFFSET      127
#define LOWERBOUND_OF_FREQUENCY_OFFSET  -127

//
// The trigger point of the high/low frequency
//
#define HIGH_FREQUENCY_TRIGGER_POINT_OFDM       20
#define LOW_FREQUENCY_TRIGGER_POINT_OFDM       -20
#define HIGH_FREQUENCY_TRIGGER_POINT_CCK        4
#define LOW_FREQUENCY_TRIGGER_POINT_CCK         -4
//
// The trigger point of decreasng/increasing the frequency offset
//
#define DECREASE_FREQUENCY_OFFSET_OFDM 10
#define INCREASE_FREQUENCY_OFFSET_OFDM -10
#define DECREASE_FREQUENCY_OFFSET_CCK   2
#define INCREASE_FREQUENCY_OFFSET_CCK   -2

#define DECREASE_FREQUENCY_OFFSET           3
#define INCREASE_FREQUENCY_OFFSET           -3

//
// Invalid frequency offset
//
#define INVALID_FREQUENCY_OFFSET            -128

//
// Frequency calibration period
//
#define FREQUENCY_CALIBRATION_PERIOD        100

//
// The frequency calibration control
//
typedef struct _FREQUENCY_CALIBRATION_CONTROL
{
    BOOLEAN bEnableFrequencyCalibration; // Enable the frequency calibration algorithm

    BOOLEAN bSkipFirstFrequencyCalibration; // Avoid calibrating frequency at the time the STA is just link-up

    BOOLEAN bApproachFrequency; // Approach the frequency

    CHAR AdaptiveFreqOffset; // Adaptive frequency offset
    CHAR LatestFreqOffsetOverBeacon; // Latest frequency offset from the beacon

    UCHAR BeaconPhyMode; // Beacon's PHY mode
} FREQUENCY_CALIBRATION_CONTROL, *PFREQUENCY_CALIBRATION_CONTROL;

#if DBG

//
// The maximum size, in bytes, of the external EEPROM image (RT3xUsbEeprom.bin)
//
#define MAX_EXTERNAL_EEPROM_IMAGE_SIZE  1024

#endif // DBG

//
// BT30 config
//
typedef struct
{
    UCHAR BssType;

    USHORT CapabilityInfo;
    
    //11n
    BOOLEAN bHtEnabled;
    HT_CAPABILITY_IE HtCapabilityIe;
    ADD_HT_INFO_IE AddHTInfoIe;

    //Auth/Encryption
    ULONG AuthMode;
    ULONG WepStatus;
    ULONG GroupCipher;
    ULONG PairCipher;
    ULONG DefaultKeyId;

    //BSSID
    UCHAR Bssid[6];

    //SSID
    CHAR Ssid[MAX_LEN_OF_SSID];
    UCHAR SsidLen;

    //Rsn IE
    UCHAR RSNIE_Len;
    UCHAR RSN_IE[AP_MAX_LEN_OF_RSNIE];

    //WPA
    UCHAR PMK[PMK_LEN];
    UCHAR ReplayCounter[8];
    UCHAR ANonce[32];
    UCHAR SNonce[32];
    UCHAR PTK[80];

    //WMM
    BOOLEAN         bWmmCapable;
    EDCA_PARM       BTHSEdcaParm;

    BOOLEAN                 bQueueEAPOL;                // Queue EAPOL frame flag.
    PUCHAR                  pRxEAPOL;                   // Pointer to EAPOL frame, queue here if we didn't indicate connect event to upper-layer
    ULONG                   RxEAPOLLen;                 // Queued EAPOL frame's leng.
    RXWI_STRUC              RxWI;                       // Queued EAPOL frame's RxWI.


} BT_CONFIG, *PBT_CONFIG;

//
//For TDT(Throughput Debugging Tools)
//
#define TDT_CONFIG_VERSION  0 
#define TDT_REG_VERSION  0 

#define TDT_V0_INI_STRUCTURE_LEN    16                  //sizeof(CONFIG_INI)
#define TDT_V0_PERIODIC_INFORMATION_STRUCTURE_LEN 64    //sizeof(PERIODIC_INFORMATION)
#define TDT_MAX_LEN_OF_PACKET_BUFFER 512

#define bFrequency 1
#define bVCO       (1<<1) 

//for selecting TDT Periodic type to save 
typedef enum _TDT_PERIODIC_TYPE 
{
    TDT_CURRENT_TX_PHY_CONFIG,
    TDT_DESIRED_TX_PHY_CONFIG,
    TDT_RATE_TUNING_CRITERIA
} TDT_PHYCONFIG_TYPE;

//for selecting rate tuning name 
typedef enum _TDT_SELECT_RATETABLE 
{
    RATE_SWITCH_TABLE,               //0
    RATE_SWITCH_TABLE_11B,
    RATE_SWITCH_TABLE_11BG,
    RATE_SWITCH_TABLE_11G,  
    RATE_SWITCH_TABLE_11BGAll,
    RATE_SWITCH_TABLE_11N1S,    
    RATE_SWITCH_TABLE_11N1S_FORABAND,
    RATE_SWITCH_TABLE_11N2S,            
    RATE_SWITCH_TABLE_11N2S_FORABAND,       
    RATE_SWITCH_TABLE_11BGN1S,      
    RATE_SWITCH_TABLE_11BGN1S_FORABAND,
    RATE_SWITCH_TABLE_11BGN2S,  
    RATE_SWITCH_TABLE_11BGN2S_FORABAND, 
    AGS_1X1HT_RATE_TABLE,   
    AGS_2X2HT_RATE_TABLE,
    AGS_3X3HT_RATE_TABLE,
    AGS_1X1HT_RATE_TABLE_5G
} TDT_SELECT_RATETABLE;

typedef struct _PHY_CONFIGURATIONS 
{
    UCHAR   PHYConVer;
    UCHAR   Index;       // Tx rate index in RateSwitchTable
    UCHAR   MCS;         
    UCHAR   BW;          //channel bandwidth 20MHz or 40 MHz
    UCHAR   ShortGI;
    UCHAR   STBC;       
    UCHAR   MODE;        // Use definition MODE_xxx.  
} PHY_CONFIGURATIONS, *PPHY_CONFIGURATIONS;

typedef struct _REGISTER_ENTRY 
{
   ULONG Number;     
   ULONG Value;   
} REGISTER_ENTRY, *PREGISTER_ENTRY;

// structure for query TDT periodic hardware register - MAC, BBP, RF,EEPROM register 
typedef struct _PROFILE_REGISTER 
{
    UCHAR HwType;             /*Decide the hardware register type*/
    UCHAR NumReqReg;          /*The number of requirement registers for User Configuration File format (ini) */
    REGISTER_ENTRY   *preg;       
} PROFILE_REGISTER, *PPROFILE_REGISTER;

// structure for INI File
typedef struct _CONFIG_INI 
{
    UCHAR Version;        /*Maintain Driver Hardware Register content*/
    ULONG Length;         /*The dynamical allocate CONFIG_INI structure size; not just sizeof(CONFIG_INI)*/
    UCHAR NumRegType;     /*The number of Hardware registers type for ini*/
    PROFILE_REGISTER *pPR;
    ULONG       pPRLen;
} CONFIG_INI, *PCONFIG_INI;

typedef struct _ENABLED_CALIBRATION 
{
    UCHAR EnCalVer;
    ULONG EnCal;    // bit map [ ... ,bVCO ,bFrequency]
} ENABLED_CALIBRATION, *PENABLED_CALIBRATION;

typedef struct _TDT_PACKET_INFO 
{
    UCHAR   PacketVer;
    ULONG   Length;     // Specifies the length of the TDT_PACKET_INFO structure in bytes.
    UCHAR   BeaconFrame[TDT_MAX_LEN_OF_PACKET_BUFFER];
    USHORT  BeaconFrameSize;
    UCHAR   AssocRequest[TDT_MAX_LEN_OF_PACKET_BUFFER];
    USHORT  AssocRequestLength;
    UCHAR   AssocResponse[TDT_MAX_LEN_OF_PACKET_BUFFER];
    USHORT  AssocResponseLength;
    UCHAR   AuthRequest[TDT_MAX_LEN_OF_PACKET_BUFFER];
    USHORT  AuthRequestLength;
    UCHAR   AuthResponse[TDT_MAX_LEN_OF_PACKET_BUFFER];
    USHORT  AuthResponseLength;
} TDT_PACKET_INFO, *PTDT_PACKET_INFO;

typedef struct _PERIODIC_INFORMATION 
{
    UCHAR   PeriodicVer;
    ULONG   Length; // Specifies the length of the PERIODIC_INFORMATION structure in bytes.  

    /*Rate Tuning Criteria (Statistics info)*/
    CHAR    RSSI;                  //Average for AvgRssi0,AvgRssi1,AvgRssi2
    CHAR    AvgRssi0;              // for 1st  antenna
    CHAR    AvgRssi1;              // for 2nd  antenna
    CHAR    AvgRssi2;              // for 3rd  antenna
    ULONG   TxErrorRatio;
    ULONG   TxTotalCnt;
    ULONG   TxSuccess;
    ULONG   TxRetransmit;
    ULONG   TxFailCount;

    PHY_CONFIGURATIONS CurrentTx;
    PHY_CONFIGURATIONS DesiredTx;
    CONFIG_INI ini;
} PERIODIC_INFORMATION, *PPERIODIC_INFORMATION;

// structure for INI File
typedef struct _CONFIG_INI_UI_SINGLEBUFF 
{
    UCHAR Version;        /*Maintain UI Version: Link to MajorTDTVersion*/
    ULONG Length;         /*The structure size*/
    UCHAR NumRegType;     /*The number of Hardware registers type for ini */
    UCHAR ucBuffer[1];    
} CONFIG_INI_UI_SINGLEBUFF, *PCONFIG_INI_UI_SINGLEBUFF;

typedef struct _PERIODIC_INFORMATION_UI_SINGLEBUFF 
{
    UCHAR   PeriodicVer;
    ULONG   Length; // Specifies the length of the PERIODIC_INFORMATION structure in bytes.  

    /*Rate Tuning Criteria (Statistics info)*/
    CHAR    RSSI;                  //Average for AvgRssi0,AvgRssi1,AvgRssi2
    CHAR    AvgRssi0;              // for 1st  antenna
    CHAR    AvgRssi1;              // for 2nd  antenna
    CHAR    AvgRssi2;              // for 3rd  antenna
    ULONG   TxErrorRatio;
    ULONG   TxTotalCnt;
    ULONG   TxSuccess;
    ULONG   TxRetransmit;
    ULONG   TxFailCount;

    PHY_CONFIGURATIONS CurrentTx;
    PHY_CONFIGURATIONS DesiredTx;
    CONFIG_INI_UI_SINGLEBUFF ini;
} PERIODIC_INFORMATION_UI_SINGLEBUFF, *PPERIODIC_INFORMATION_UI_SINGLEBUFF;


// Store for persistent TDT group
typedef struct _RT_TDT_CONFIG 
{
    UCHAR MajorTDTVersion;      // Specifies the major version of TDT the driver is using. 
    HANDLE  TdtEvent;           //Informing driver by MTK_OID_N6_SET_TDT_EVENT
    
    /*For one-shot*/
    UCHAR  TXSelectRateTable;   //store TX Select Rate Table Name (rate tunning name)
    TDT_PACKET_INFO packet;
    ENABLED_CALIBRATION EnCalibrtion;

    /*For periodic*/
    PERIODIC_INFORMATION PeriodicInfo;
} RT_TDT_CONFIG, *PRT_TDT_CONFIG;


typedef struct _FPGA_TEST_STRUCT
{
    UCHAR TestWCID; //for emulation
    UCHAR DPort;
    ULONG tempCount;
}   FPGA_TEST_STRUCT, *PFPGA_TEST_STRUCT;

#if _WIN8_USB_SS_SUPPORTED
typedef struct _RTMP_SS
{
    PIRP pIrp;
    USB_IDLE_CALLBACK_INFO CallbackInfo;

    NDIS_SPIN_LOCK SsLock;

} RTMP_SS, *PRTMP_SS;
#endif

typedef struct _MP_XMIT_CTRL_UNIT_BUFFER
{
    PUCHAR  VirtualAddr;
    ULONG   Length;
    PUCHAR  CurrentMdlPtr;
}MT_XMIT_CTRL_UNIT_BUFFER, *PMT_XMIT_CTRL_UNIT_BUFFER;

//Transmit control block.
typedef struct _MP_XMIT_CTRL_UNIT
{
    struct _MP_XMIT_CTRL_UNIT    *Next; 
    PMP_PORT         pPort;
    ULONG               PortNumber;
    PMP_ADAPTER    pAd;
    PHEADER_802_11  pHeader80211;
    MT_XMIT_CTRL_UNIT_BUFFER   BufferList[NUM_OF_MP_XMIT_CTRL_UNIT_BUF]; // Point to the buffer of Upper layer; bufferlist is the 802_11_header;
    UCHAR           HeaderBuffer[24];
    PDOT11_EXTSTA_SEND_CONTEXT pDot11SendContext;
    UCHAR              NumOfBuf;
    UCHAR       NumOfMdl;
    ULONG       TotalPacketLength;
    UCHAR       PktSource;
    UCHAR       Aid;
    UCHAR       Wcid;
    UCHAR       NumberOfFrag;
    UCHAR       Rate;
    UCHAR       UserPriority;
    UCHAR       QueIdx;
    UCHAR       OpMode;
    UCHAR       FragmentRequired;
    BOOLEAN     bUAPSD;
    BOOLEAN     bFirstNb;
    BOOLEAN     bEosp;
    BOOLEAN     bMoreData;
    BOOLEAN     bDispatchLevel;
    NDIS_STATUS     Status;
    BOOLEAN     bIndicated;
    ULONG           CurrentXcuidx;
    PVOID          Reserve1;  //Current NBL;
    PVOID          Reserve2;   //Current NB;
}MT_XMIT_CTRL_UNIT, *PMT_XMIT_CTRL_UNIT;


typedef struct _RX_CONFIG 
{
    NDIS_HANDLE             RecvPktsPool;
    NDIS_HANDLE             RecvPktPool;
    QUEUE_HEADER            IndicateRxPktsQueue;
    NDIS_SPIN_LOCK          IndicateRxPktsQueueLock; // for IndicateRxPkts
    MP_LOCAL_RX_PKTS IndicateRxPkts[NUM_OF_LOCAL_RX_PKT_BUFFER];    
    NDIS_SPIN_LOCK          IndicateRxPktsLock[RTMP_MAX_NUMBER_OF_PORT]; // for IndicateRxPkts
    PUCHAR                  DummyForIndicatedMDL;
    RTMP_RXBUF              LocalRxReorderBuf[Max_BARECI_SESSION];
    PUCHAR                  pRxData;                    // Pointer to current RxRing offset / fragment frame offset    
    BOOLEAN                 bQueueEAPOL;                // Queue EAPOL frame flag.
    PUCHAR                  pRxEAPOL;                   // Pointer to EAPOL frame, queue here if we didn't indicate connect event to upper-layer
    ULONG                   RxEAPOLLen;                 // Queued EAPOL frame's leng.
    UCHAR                   UserPriorityOfQueuedEAPOL; // The user priority of the queued EAPOL packet.
    LONG                    nIndicatedRxPkts; // Number of NetBufferList indicated Sync for RTMPPause  
    PVOID        pLastWaitToIndicateRxPkts[RTMP_MAX_NUMBER_OF_PORT];
    PVOID        pWaitToIndicateRxPkts[RTMP_MAX_NUMBER_OF_PORT];
    UCHAR                   nWaitToIndicateRxPktsNum[RTMP_MAX_NUMBER_OF_PORT];
    // RX re-assembly buffer for fragmentation
    FRAGMENT_FRAME          FragFrame;                  // Frame storage for fragment frame    
} RX_CONFIG, *PRX_CONFIG;

typedef struct _TX_CONFIG 
{
    NDIS_SPIN_LOCK          LocalNetBufferListQueueLock;    // for LocalNetBufferList
    QUEUE_HEADER            LocalNetBufferListQueue;
    MP_LOCAL_RX_PKTS LocalNetBufferList[NUM_OF_LOCAL_RX_PKT_BUFFER];
    NDIS_RW_LOCK            PsQueueLock;    
    
     // current TX sequence #
    USHORT                  Sequence;
    USHORT                  TxFragSequence;    
    USHORT                  NonQosDataSeq;

    NDIS_SPIN_LOCK          SendTxWaitQueueLock[4];     // SendTxWaitQueue spinlock
    QUEUE_HEADER            SendTxWaitQueue[4];
    NDIS_SPIN_LOCK          TxContextQueueLock[4];      

    NDIS_SPIN_LOCK          DeQueueLock[4];
    BOOLEAN                 DeQueueRunning[4];          // for ensuring RTUSBDeQueuePacket get call once

    LONG                    NumQueuePacket;

    NDIS_SPIN_LOCK          XcuIdelQueueLock;
    QUEUE_HEADER       XcuIdelQueue;
    MT_XMIT_CTRL_UNIT    XcuArray[NUM_OF_MP_XMIT_CTRL_UNIT_NUMBER];      

    TXWI_STRUC              BeaconTxWI;
    ULONG                   BeaconBufLen;
    CHAR                    BeaconBuf[MAX_LEN_OF_MLME_BUFFER]; // NOTE: BeaconBuf should be 4-byte aligned
    
} TX_CONFIG, *PTX_CONFIG;

typedef struct _WPS_CONFIG 
{
    UCHAR                   WPSDevNameLen;      // the actual WPSDevName length in used
    UCHAR                   WPSDevName[32];     // WPS Device Name, display on UPnP discovery lists of Vista-ER 
} WPS_CONFIG, *PWPS_CONFIG;

typedef struct _COUNTER_T
{
    COUNTER_802_3           Counters8023;               // 802.3 counters
    COUNTER_802_11          WlanCounters;               // 802.11 MIB counters

    COUNTER_MTK             MTKCounters;                // MTK propriety counters
    COUNTER_DRS             DrsCounters;                // counters for Dynamic TX Rate Switching
    NICSTATISTICSINFO       StatisticsInfo;

} COUNTER_T, *PCOUNTER_T;

typedef struct _UI_CONFIG_T
{
    // UI Tool Feature Configuration
    BOOLEAN UtfFixedTxRateEn;   // Indicate "Fixed Tx Rate" is enable or disable
    PHY_CFG UtfFixedTxRateCfg;  // Store the setting of "Fixed Tx rate". Refer to OID: RT_OID_SET_FIXED_RATE    

    ULONG                   ExtraInfo;              // Extra information for displaying status
    ULONG                   SystemErrorBitmap;      // b0: E2PROM version error

    PHY_CFG LastMgmtRxPhyCfg; // Save last management Rx rate for displaying on UI      

    BOOLEAN     bPromiscuous;

} UI_CONFIG_T, *PUI_CONFIG_T;

typedef struct _MULTI_CHANNEL_T
{
    BOOLEAN             MultiChannelEnable;
    MULTI_CH_OP_MODE    MultiChannelCurrentMode;
    UCHAR               SwQSelect;
    PNDIS_OID_REQUEST   PendedDisconnectOidRequest;
    PNDIS_OID_REQUEST   PendedGoOidRequest;
    PNDIS_OID_REQUEST   PendedResetRequest;
    PMP_PORT            pDisconnectPort;
    PMP_PORT            pGoPort;
    PMP_PORT            pResetRequestPort;
    PMP_PORT            pScanRequestPort;
    UCHAR                   TxOp;

    MTK_THREAD_STRUCT   hMultiChannelThread;
    CmdQ                MultiChannelCmdQ;
    NDIS_SPIN_LOCK          MultiChannelCmdQLock;
    
} MULTI_CHANNEL_T, *PMULTI_CHANNEL_T;

typedef struct  _LOGO_TEST_CONFIG
{
    BOOLEAN             WhckCpuUtilTestRunning;
    BOOLEAN             WhckFirst40Secs;
    ULONG               WhckDataCntForPast40Sec;
    ULONG               WhckDataCntForPast60Sec;    
    BOOLEAN             OnTestingWHQL;
    BOOLEAN             OnTestingCPUUtil;
    BOOLEAN             TurnOffBeaconListen;
    UCHAR               CounterForTOBL;
    ULONG               WHQLScanTime;

    SIGMA_CFG           SigmaCfg;

} LOGO_TEST_CONFIG, *PLOGO_TEST_CONFIG;

// for Debug 
typedef struct  _TRACK_INFO_T
{
    ULONG               PacketCountFor7603Debug;
    ULONG               RxPacketCountFor7603Debug;
    BOOLEAN             bTxNULLFrameTest;
    BOOLEAN             bStopCMDEventReceive;
    
    // ---------------------------
    // System event log
    // ---------------------------
    RT_802_11_EVENT_TABLE   EventTab;

    ULONG               SameRxByteCount;    
    ULONG               InfraOnSameRxByteCount;
    ULONG               IdleOnSameRxByteCountUpper;
    ULONG               IdleOnSameRxByteCountLower;
    ULONG               GoOnSameRxByteCount;

    UCHAR               debugpacketcount;

    USHORT              CountDowntoPsm;
    USHORT              CountDownFromS3S4;

#if DBG
    UCHAR               EEPROMImage[MAX_EXTERNAL_EEPROM_IMAGE_SIZE]; // EEPROM image from the external file
    BOOLEAN             bExternalEEPROMImage;
#endif // DBG

} TRACK_INFO_T, *PTRACK_INFO_T;

// Unscheduled Automatic Power Save Delivery (U-APSD)
typedef struct _U_APSD_T
{
    NDIS_SPIN_LOCK      UAPSDEOSPLock;          // Uapsd related parameters spinlock
    NDIS_SPIN_LOCK      TxSwUapsdQueueLock[4];  // TxSwQueue spinlock
    QUEUE_HEADER        TxSwUapsdQueue[NUM_OF_UAPSD_CLI][WMM_NUM_OF_AC];  // 4 AC for WMM-PS per client
    UCHAR               UapsdSWQBitmap; // Record how many uapsd-capable clients and their queue index 
} U_APSD_T, *PU_APSD_T;

typedef struct _USB_CONFIG_T
{
    // Flags for bulk out data priority
    ULONG                   BulkFlags;

    //
    // Used in XP
    //
    UCHAR                       BulkInPipeNum;
    UCHAR                       BulkOutPipeNum;
    USHORT                      BulkOutMaxPacketSize;
    USHORT                      BulkInMaxPacketSize;

    PWCHAR                      AdapterDesc;
    ULONG                       AdapterDescLen;

    PDEVICE_OBJECT              Pdo;
    PDEVICE_OBJECT              Fdo;
    PDEVICE_OBJECT              pNextDeviceObject;  

    //for WDF  use
    //===================================================================   
    WDFDEVICE                   WdfDevice;
    WDFUSBDEVICE                UsbDevice;
    //===================================================================

    UCHAR                       UsedEndpoint; // bit0-> first bulkout enpoint   

    MTK_THREAD_STRUCT                  hControlThread;
    MTK_THREAD_STRUCT                  hMLMEThread;    
    MTK_THREAD_STRUCT                  hBulkOutDataThread[4];
    MTK_THREAD_STRUCT                  hDequeueDataThread;

    CmdQ                    CmdQ;
    NDIS_SPIN_LOCK          CmdQLock;

    NDIS_SPIN_LOCK          ControlLock;
    NDIS_SPIN_LOCK          BBPControlLock;
    NDIS_SPIN_LOCK          PktCmdLock;
    
    TX_CONTEXT                          MLMEContext[PRIO_RING_SIZE];
    TX_CONTEXT                          NullContext;
    TX_CONTEXT                          PsPollContext;
    TX_CONTEXT                          RTSContext;
    TX_CONTEXT                          PktCmdContext;
    TX_CONTEXT                          FwPktContext;
#if 1 // MT7603 FPGA
    TX_CONTEXT                          EPNullContext[6];
#endif

    UCHAR                   NextBulkOutIndex[4];    // only used for 4 EDCA bulkout pipe
    BULK_FAIL_PARM          BulkFailParm;
    BOOLEAN                 BulkOutPending[6];  // used for total 6 bulkout pipe
    BOOLEAN                 bBulkOutMlme;
    LONG                       MlmePendingCnt;
    BOOLEAN                 ControlPending;
    BOOLEAN                 BBPControlPending;
    LONG                    PrioRingTxCnt;
    UCHAR                   PrioRingFirstIndex;

    BOOLEAN                 bBulking;

    BOOLEAN     bForcePrintTX;
    BOOLEAN     bForcePrintRX;
    ULONG       BulkOutComplete;
    ULONG       BulkOutCompleteOther;
    ULONG       BulkOutCompleteCancel;
    ULONG       BulkInReq;
    ULONG       BulkInComplete;
    ULONG       BulkInCompleteFail;
    ULONG       BulkOutReqCount[3];
    ULONG       BulkOutSize;
    ULONG       SegmentEnd;

    // 
    // Watch dog for RX URB no responding case
    //
    ULONG       CurrentBulkInCount;
    ULONG       HWRxPCNCount;

    ULONG                   BulkOutDataOneSecCount;
    ULONG                   BulkInDataOneSecCount;
    ULONG                   BulkLastOneSecCount; // BulkOutDataOneSecCount + BulkInDataOneSecCount

    LONG                    TxCount;        // Number of Bulkout waiting to be send.
    LONG                    PendingTx[6];   // 6x9x has 6 bulk out pipe
    BOOLEAN                 bEndPointUsed[6];

    // Data related context and AC specified, 4 AC supported
    HT_TX_CONTEXT                   TxContext[4];

    PUCHAR                  BulkOutTempBuf[4];  
    BOOLEAN                         bIRPBulkOutData;

    RX_CONTEXT                          RxContext[RX_RING_SIZE ];  // 1 for redundant multiple IRP bulk in.
    LONG                                PendingRx;

    BOOLEAN     bUsbTxBulkAggre;        

    UCHAR                   NextMLMEIndex;              // Next PrioD write pointer
    UCHAR                   PushMgmtIndex;              // Next SW management ring index
    UCHAR                   PopMgmtIndex;               // Next SW management ring index
    ULONG                   MgmtQueueSize;              // Number of Mgmt request stored in MgmtRing
    UCHAR                   NextRxBulkInIndex;
    UCHAR                   NextRxBulkInReadIndex;
    ULONG                   NextRxBulkInPosition;   // Want to contatenate 2 URB buffer while 1st is bulkin failed URB. This Position is 1st URB TransferLength.

    NDIS_SPIN_LOCK          MLMEWaitQueueLock;

    NDIS_SPIN_LOCK          BulkOutMlmeLock;
    NDIS_SPIN_LOCK          BulkInLock;
    NDIS_SPIN_LOCK          BulkInReadLock;
    BOOLEAN                 bBulkInRead;

    NDIS_SPIN_LOCK          MLMEQLock;

    NDIS_SPIN_LOCK          DeMGMTQueueLock;                // for ensuring RTUSBDeQueuePacket get call once
    BOOLEAN                 DeMGMTQueueRunning;

    NDIS_SPIN_LOCK          MT7603EPLock[6];

    MGMT_STRUC              MgmtRing[MGMT_RING_SIZE];   // management ring size

   

} USB_CONFIG_T, *PUSB_CONFIG_T;

typedef struct _REGISTRY_CFG
{
    UCHAR                   OpMode;                     // OPMODE_STA, OPMODE_AP
    UCHAR       CountryRegion;      // Enum of country region, 0:FCC, 1:IC, 2:ETSI, 3:SPAIN, 4:France, 5:MKK, 6:MKK1, 7:Israel
    UCHAR       CountryRegionForABand;  // Enum of country region for A band
    UCHAR       PhyMode;
    UCHAR       Channel;
    USHORT      RtsThreshold;           // in unit of BYTE
    USHORT      FragmentThreshold;      // in unit of BYTE
    BOOLEAN             bRdg;
    UCHAR                               RxBulkFactor;
    UCHAR                               TxBulkFactor;
    ULONG                               MaxPktOneTxBulk;
    ULONG       UseBGProtection;        // 0: auto, 1: always use, 2: always not use
    BOOLEAN     bUseShortSlotTime;      // 0: disable, 1 - use short slot (9us)
    BOOLEAN     bAggregationCapable;      // 1: enable TX aggregation when the peer supports it
    BOOLEAN     bIEEE80211H;            // 1: enable IEEE802.11h spec.
    BOOLEAN     bCarrierDetect;         // 1: enable carrier detection
    BOOLEAN     bEnableTxBurst;         // 1: enble TX PACKET BURST, 0: disable TX PACKET BURST
    BOOLEAN     bWmmCapable;            // 0:disable WMM, 1:enable WMM,
    BOOLEAN     bAPSDCapable;
    BOOLEAN     bAPSDAC_BE;
    BOOLEAN     bAPSDAC_BK;
    BOOLEAN     bAPSDAC_VI;
    BOOLEAN     bAPSDAC_VO;
    BOOLEAN     bMultipleIRP;
    BACAP_STRUC        BACapability; //   NO USE = 0XFF  ;  IMMED_BA =1  ;  DELAY_BA=0
    REG_TRANSMIT_SETTING        RegTransmitSetting; //registry transmit setting. this is for reading registry setting only. not useful.
    BOOLEAN             bDLSCapable;        // 0:disable DLS, 1:enable DLS
    ULONG               DirectMode;         // Direct Link Mode, 0: 11n DLS, 1: 11z TDLS (default to DLS)
    BOOLEAN             PSPXlink;  // 0: Disable. 1: Enable
    P2P_CONTROL     P2pControl;
    BOOLEAN     bSwRadioOff;        // For Vista, TRUE means turn off Radio.
    BOOLEAN     bAutoReconnect;         // Set to TRUE when setting OID_802_11_SSID with no matching BSSID
    UCHAR       BssType;              // BSS_INFRA or BSS_ADHOC
    ULONG       AdhocMode;          // 0:WIFI mode (11b rates only), 1: b/g mixed, 2: 11g only, 3: 11a only, 4: 11abg mixed
    BOOLEAN     bAdhocNMode;                // AdhocN Support
    ULONG       AdhocNForceMulticastRate;   // AdhocN Support: legacy(0x00100000)/auto(0x0)/manual(0x0001xxxx) mode.
    BOOLEAN     bAdhoc5GDisable;    // 0: 5G Adhoc is allowed(default), 1: 5G Adhoc is not allowed
    BOOLEAN     bAdhocN40MHzAllowed;        // 0: 40MHz Adhoc is not allowed(default), 1: 40MHz Adhoc is allowed
    BOOLEAN     bAdhocN14Ch40MHzAllowed;    // 0: 40MHz Adhoc in ch-14 is not allowed(default), 1: 40MHz Adhoc in ch-14 is allowed
    ULONG       WindowsPowerMode;           // Power mode for AC power
    CCX_CONTROL         CCXControl;                 // Master administration state
    CHAR                    CCXDbmOffset;   // a positive number, for RF roaming
    CHAR                    FastRoamingSensitivity; // 0: disable, 1~5: enalbe fast-roaming
    BOOLEAN         bAutoChannelAtBootup;  // 0: disable, 1: enable
    UCHAR           DtimPeriod;      // 0.. DtimPeriod-1
    USHORT          AgeoutTime;     // Default 300 second
    // a threshold to judge whether the candidate AP is worth roaming or not
    CHAR                RssiDelta;
    BOOLEAN     SmartScan;
    PS_CONTROL          PSControl; ////Power saving control sturcture
    int                 KeepReconnectCnt;
    ULONG   SoftAPForceMulticastRate; // Manual: 0x1001xxxx, Legacy/Auto: 0x1010xxxx, Otherwise (0x1000xxxx/0x1100xxxx/...): same as unicast rate
    BOOLEAN             LedControlBySW;  //Default 0:Control by EEPROM
    UCHAR               INFLedCntl;
    USHORT              INFLed1;
    USHORT              INFLed2;
    USHORT              INFLed3;
    ////////////////////////////////////////////////////////////////////////////////////////
    // This is only for WHQL test.
    //      on test item 2c_wlan_statistics, issue on [88888] FailedCount was not correctly updated
    //      FailedCount:                      155   (Query by WHQL)
    //      PacketsToSen:                     200
    //      FailedCount:                      354   (Query by WHQL) should be 355
    //    
    // Delta(base.FailedCount, current.FailedCount) < packetsToSend
    //
    // Statistic Register seems not report correct number.
    // Used this variable as trick.
    //
    BOOLEAN             WhqlTest;
    ULONG       CountryTxPwr;
    ULONG       CountryTxPwr5G;
    UCHAR       FixLinkSpeedStatus;  // 0 : Show Tx Rate, 1 : Show Fix Rx Rate, 2.....show special case rate
    PMF_CONTROL PmfControl;
    UCHAR               BssidRepositTime;   //the time to determine how long to keep a bssid item   
    ULONG       BeaconLostTime; // After Beacon Lost Time Indicate dissconnect to OS
    BOOLEAN             bProhibitTKIPonHT;  //the registry either enableTKIP on HT or disable TKIP on HT.
    //Link Quality Setting Control
    USHORT  LinkQualitySetting;
     // CH14 only supports 11b
    BOOLEAN             Ch14BOnly;

    // For BTHS
    UCHAR       PALRegulatoryClass;
    ULONG       PALCapability;  
    UCHAR       PALVersionSpecifier;
    USHORT      PALCompany_ID;
    USHORT      PALSubVersion;

    GPIO_PIN_SHARING_MODE GpioPinSharingMode; // GPIO pin-sharing mode
    ULONG LedCfg;
    ULONG               Feature11n; 
    CHAR AdaptiveFreqOffset; // Adaptive frequency offset
    ULONG       TxPowerDefault;         // keep for TxPowerPercentage
    UCHAR       b11nTxBurstTimeTest;  // For 11n Tx Burst Time Test
    UCHAR       bAdTxPwrForFCCTest;  // For FCC Power Test
    UCHAR           CustomizedPowerMode; // Adjust Tx power mode For Customer , 0: no use, 1: For Asus
    UCHAR       MaxTxPower;
    UCHAR       CountryTargetTxPowerMode; // Country Target Power Mode: 0: normal, 1: Contry Target Power Table Per rate and including band edge.
     //Registry contol to Enable/Disable StreamMode
    BOOLEAN             StreamMode;
    // Registry control to Enable/Disable function of ChannelSwitchOffloadToFW
    BOOLEAN             bChannelSwitchOffloadToFW;
     //Registry contol to write specific BBP value
    ULONG               BBPIDV;     


    //Check security when setting WEP to connect AP
    BOOLEAN     WepConnectCheckEncry;

     //
    // Customer feature setting
    //
    // Bit 0: BT/Wi-Fi Single LED Control
    // Bit 31~1: not used;
    //
    ULONG       FeatureSetting;  
    //
    // ETxBfEn=d - Enable/disable the explicit beamforming
    // Values: 0 or 1. Default = 0
    // Description: 0 disables Explicit BF. 1 enables sending of the Sounding packet and Explicit BF of the TX packets.
    //
    ULONG   ETxBfEn;

     //
    // ETxBfTimeout=dddd - Set Explicit BF timeout
    // Values: 0 to 65535. Units are 25 microseconds. Default = 0
    // Description: ETxBF profiles will expire after ETxBFTimeout*25 microseconds. Setting ETxBFTimeout to 0 disables the timeout.
    //
    ULONG   ETxBfTimeout;
    //
    // Explicit non-compressed beamforming feedback capable
    // Values: 0 or 1. Default = 0
    // Description: 0 for compressed beamforming feedback. 1 for non-compressed beamforming feedback.
    //
    ULONG   ETxBfNonCompressedFeedback;

    BOOLEAN         bP2pAckCheck;
    BOOLEAN         P2PVideoBoost;  // 1: Enable VO stream in Ralink P2P Group

    //WSC version
    UCHAR   WSCVersion2;
     //For match the creteria of WSC 2.0 test bed. 
    // bit 0: use version 5.7 for test item 4.2.8.
    // bit 1: advertise two WPS IEs in the Probe Request for test item 4.2.9.
    //          (the first one including only the first octet of the TLVs and the second including reset of the TLV data).
    // bit 2: 0->No RSN IE 1-> RSN IE for test item 4.2.12.
    // bit 3: 0->No WSC IE 1-> WSC IE for test item 4.2.12.
    // bit 4: fragmented EAP-WSC packets for test item 4.2.10.
    ULONG   WSC20Testbed;
    UCHAR   WscHwPBCMode;   // 0: delay over 3 sec. 1: delay 1 sec.
    ULONG       MPolicy;    //multicast policy: 0-> defalult(enable "bypass RA packet") 1->disable "bypass RA packet" 2->will not clear up accepted multicast poll
    ULONG       OzmoDeviceSuppressScan; //0: Disable suppress scan, 1: Smart suppress scan, 2: Suppress Scan
    BOOLEAN bPPAD; // Enable PPAD (per-packet antenna diversity)
    CALIBRATION_CONTROL_REGISTRY CalCtrlRegistry; // Calibration control registry
    ULONG CoefficientAlcRefValue; // Coefficient for 2.4 internal ALC reference value (in percentage)
    BOOLEAN             OnTestingWHQL;
    BOOLEAN             OnTestingCPUUtil;
    ULONG               WHQLScanTime;
    UCHAR       ChannelMode;
    // 1: Enable InBoxMode: Not RadioOff for Power Saving
    BOOLEAN             InBoxMode;
    BOOLEAN BandedgePowerTableEnable; // Bandedge Tx power table
    BOOLEAN SwitchChannelOffloadEnable;

    BOOLEAN FullCalibrationOffloadEnable;

    BOOLEAN ThermalEnable;

    ULONG               WAPI;

     //Registry contol to disable FrequencyCalibration
    BOOLEAN             AdaptiveFreq;

    TX_RX_CONFIG_BY_REGISTRY TxRxConfigByRegistry; // Tx/Rx configuration by registry
    
} REGISTRY_CFG,*PREGISTRY_CFG;

typedef struct _HW_CHANNEL_BW_INFO
{
   NDIS_PORT_NUMBER OperatingPortNumber;     
   UCHAR    Channel;
   UCHAR    CentralChannel;
   UCHAR    BW;
   
} HW_CHANNEL_BW_INFO, *PHW_CHANNEL_BW_INFO;

typedef struct _HW_SUPPORT_CAPABILITY
{
    REG_TRANSMIT_SETTING        MaxRegTransmitSetting; //registry transmit setting. this is for reading registry setting only. not useful.
    UCHAR       MaxPhyMode;            // PHY_11XXX
    BACAP_STRUC        MaxBACapability; //   NO USE = 0XFF  ;  IMMED_BA =1  ;  DELAY_BA=0
    BOOLEAN         bWmmCapable;

} HW_SUPPORT_CAPABILITY,*PHW_SUPPORT_CAPABILITY;

typedef struct _HW_CONFIG_T
{
    // configuration: read from Registry & E2PROM
    HW_ARCHITECTURE_VERSION HwArchitectureVersion; // HW architecture version
    BOOLEAN                 bLocalAdminMAC;             // Use user changed MAC
    UCHAR                   PermanentAddress[MAC_ADDR_LEN];    // Factory default MAC address
    UCHAR                   CurrentAddress[MAC_ADDR_LEN];      // User changed MAC address
   
    ULONG                   MACVersion;

    UCHAR       ChannelMode;

    UCHAR BeaconNum;

    HW_SUPPORT_CAPABILITY     HwSupportCapabilities;

    HW_CHANNEL_BW_INFO HWChannelBWInfo;

    CHANNEL_TX_POWER        ChannelList[MAX_NUM_OF_CHANNELS];   // list all supported channels for site survey
    UCHAR                   ChannelListNum;                     // number of channel in ChannelList[]

    NDIS_SPIN_LOCK          RadioStatusChangeLock;
    BOOLEAN                 RadioStatusIsChanging;

    // ---------------------------
    // E2PROM
    // ---------------------------
    ULONG                   EepromVersion;          // byte 0: version, byte 1: revision, byte 2~3: unused
    USHORT                  EEPROMDefaultValue[NUM_EEPROM_BBP_PARMS];
    USHORT                  EEPROMManualChList[MAX_NUM_OF_MANUAL_REGION];
    BOOLEAN                 buseEfuse;
    BOOLEAN                 DisableABandFromEEPROM;
    ULONG                   FirmwareVersion;        // byte 0: Minor version, byte 1: Major version, otherwise unused.

    // ---------------------------
    // BBP Control
    // ---------------------------
    UCHAR                   BbpWriteLatch[MAX_NUM_OF_BBP_LATCH];     // record last BBP register value written via BBP_IO_WRITE/BBP_IO_WRITE_VY_REG_ID
    UCHAR                   BbpRssiToDbmDelta;
    BBP_R66_TUNING          BbpTuning;
    

    // ----------------------------
    // RFIC control
    // ----------------------------
    UCHAR                   RfIcType;       // RFIC_xxx
    ULONG                   RfFreqOffset;   // Frequency offset for channel switching
    RTMP_RF_REGS            LatchRfRegs;    // latch th latest RF programming value since RF IC doesn't support READ

    EEPROM_ANTENNA_STRUC    Antenna;                            // Since ANtenna definition is different for a & g. We need to save it for future reference.
    EEPROM_NIC_CONFIG2_STRUC    NicConfig2;
    EEPROM_NIC_CONFIG3_STRUC    NicConfig3;

    // This soft Rx Antenna Diversity mechanism is used only when user set 
    // RX Antenna = DIVERSITY ON
    SOFT_RX_ANT_DIVERSITY   RxAnt;

    // ----------------------------
    // LED control
    // ----------------------------
    MTK_THREAD_STRUCT                  hLedThread;    
    MCU_LEDCS_STRUC     LedCntl;
    USHORT              Led1;   // read from EEPROM 0x3c
    USHORT              Led2;   // EEPROM 0x3e
    USHORT              Led3;   // EEPROM 0x40
    UCHAR               LedIndicatorStregth;
    BOOLEAN             bLedOnScanning;
    UCHAR               LedStatus;
    UCHAR               BackupLedStatus;
    UCHAR               SetLedStatus;

    BOOLEAN             LedControlBySW;  //Default 0:Control by EEPROM
    UCHAR               INFLedCntl;
    USHORT              INFLed1;
    USHORT              INFLed2;
    USHORT              INFLed3;

    USHORT TXWI_Length;
    USHORT RXWI_Length;

    BOOLEAN                             bLoadingFW;
    ULONG                               LoadingFWCount;
    FW_CTRL FwCtrl;
    NDIS_EVENT                  WaitFWEvent;
    NDIS_EVENT                  LoadFWEvent;

    EFUSE_CONFIGURATION eFuseConfig; // e-Fuse configuration

    BOOLEAN             bRdg;

    //
    // For Tunning
    //
    ULONGLONG               LastDRSTime;           // latch the value of DRSTime()
    ULONGLONG               LastDRSDeltaTime;      // latch the value of DRSDeltaTime()
    ULONGLONG               CurrentDRSDeltaTime;   // latch the value of Current DRSDeltaTime()
    
    BOOLEAN     bAutoTxAgcA;                // Enable driver auto Tx Agc control
    
    BOOLEAN     bAutoTxAgcG;                // Enable driver auto Tx Agc control

#ifdef RTMP_INTERNAL_TX_ALC
    MT7601_TX_ALC_DATA TxALCData;
#endif /* RTMP_INTERNAL_TX_ALC */   

    // TempSensor record MAC1300 for thermal
    ULONG       MAC1300;

    FREQUENCY_CALIBRATION_CONTROL FreqCalibrationCtrl; // The frequency calibration control

    VHT_RX_AGC_VGA_TUNING_CTRL VhtRxAgcVgaTuningCtrl;

    // count down timer for VCO/Freq calibration
    UCHAR SkipFreqCaliTimer;

    TRANSMIT_BEAMFORMING_CONTROL TxBfCtrl; // Transmit beamforming control

    //
    // TODO: Review RSSI various.
    //
    CHAR        BGRssiOffset1;              // Store B/G RSSI#1 Offset value on EEPROM 0x9Ah
    CHAR        BGRssiOffset2;              // Store B/G RSSI#2 Offset value 
    CHAR        BGRssiOffset3;              // Store B/G RSSI#2 Offset value 
    CHAR        ARssiOffset1;               // Store A RSSI#1 Offset value on EEPROM 0x9Ch
    CHAR        ARssiOffset2;               // Store A RSSI#2 Offset value 
    CHAR        ARssiOffset3;               // Store A RSSI#2 Offset value 
    CHAR        BLNAGain;                   // Store B/G external LNA#0 value on EEPROM 0x44h
    CHAR        ALNAGain0;                  // Store A external LNA#0 value for ch36~64
    CHAR        ALNAGain1;                  // Store A external LNA#1 value for ch100~128
    CHAR        ALNAGain2;                  // Store A external LNA#2 value for ch132~173

    UCHAR       BLNAGainExt; // Store B/G external LNA#0 value on EEPROM 0x44h
    UCHAR       ALNAGain0Ext; // Store A external LNA#0 value for ch36~64
    UCHAR       ALNAGain1Ext; // Store A external LNA#1 value for ch100~128
    UCHAR       ALNAGain2Ext; // Store A external LNA#2 value for ch132~173

    UCHAR       ChIdx5GLbMb; // Channel index for 5G LB/MB
    UCHAR       ChIdx5GMbHb; // Channel index for 5G MB/HB

    // for AsicUpdateProtect use
    BOOLEAN     fNoisyEnvironment;
    USHORT      OperaionMode;
    ULONG       SetMask;
    BOOLEAN     bDisableBGProtect;
    BOOLEAN     bNonGFExist;

} HW_CONFIG_T, *PHW_CONFIG_T;

typedef struct _NIC_CONFIG
{
    LONG                    RefCount;           // Keep adapter refer count for checkforhang & halt

    UCHAR                   VendorDescLen;               // the actual ssid length in used
    CHAR                    VendorDesc[MAX_LEN_OF_VENDOR_DESC]; // NOT NULL-terminated

    //Store for different Win OS condition.
    ULONG                   NdisVersion;

    OID_PENDING_ENTRY       PendOidEntry;
    NDIS_SPIN_LOCK          PendingOIDLock;
    
    PNDIS_OID_REQUEST       PendedScanRequest;
    PVOID                   ScanRequestID;

    NDIS_DEVICE_POWER_STATE PendedDevicePowerState;

    BOOLEAN     bIsClearScanTab;
    REPOSIT_BSSID_LIST_EX   RepositBssTable;        // store the previous SCAN result
    REPOSIT_BSSID_LIST_EX   TempRepositBssTable;    //temproal store the ScanTable

    BOOLEAN                 bUpdateBssList;

    PUCHAR              OutBufferForSendProbeReq;
    ULONG               OutBufferForSendProbeReqLen;

    // 802.3 multicast support
    ULONG                   NumberOfMcastAddresses;     // Number of mcast entry exists
    UCHAR                   McastTable[MAX_MCAST_LIST_SIZE][MAC_ADDR_LEN];      // Mcast list

    PRIVATE_STRUC           PrivateInfo;                // Private information & counters   

    NICPHYMIB                           PhyMIB[MAX_NUM_PHY_TYPES];  

    ULONG                               LookAhead;
    ULONG                               AutoConfigEnabled;  

    BOOLEAN                 bConfigChanged;         // Config Change flag for the same SSID setting
    BOOLEAN                 bMultipleIRP;
    UCHAR                       NumOfBulkInIRP;
} NIC_CONFIG, *PNIC_CONFIG;


typedef struct _P2P_CTRL_T
{
    
    
    // For WFA Peer-to-Peer
    
    RT_P2P_TABLE            P2PTable;

    CHAR                    GOBeaconBuf[512]; // NOTE: BeaconBuf should be 4-byte aligned
    ULONG                   GOBeaconBufNoALen;
    ULONG                   GOBeaconBufLen;

    MTK_THREAD_STRUCT       hP2PThread;

    NDIS_SPIN_LOCK          P2PCmdQLock;
    CmdQ                    P2PCmdQ;

    NDIS_SPIN_LOCK          TxSwNoAMgmtQueueLock;
    QUEUE_HEADER            TxSwNoAMgmtQueue;
    PMP_PORT                pInfraPort;
    PMP_PORT                pWfdDevicePort;
    PMP_PORT                pWfdCLTPort;
    PMP_PORT                pWfdCLT2Port;
    PMP_PORT                pWfdGOPort;
    
    ULONG                   P2pMsConnectedStatus;
    ULONG                   MultiChannelStartOrEndProcess;          // Start of end process
    
} P2P_CTRL_T, *PP2P_CTRL_T;

typedef struct _BEACON_PORT
{
    PMP_PORT      pApGoPort;
} BEACON_PORT, *PBEACON_PORT;

//
// The offset of the Tx power tuning entry (zero-based array)
//
#define TX_POWER_TUNING_ENTRY_OFFSET        30

//
// The lower-bound of the Tx power tuning entry
//
#define LOWERBOUND_TX_POWER_TUNING_ENTRY    -30

//
// The upper-bound of the Tx power tuning entry
//
#define UPPERBOUND_TX_POWER_TUNING_ENTRY    45 // zero-based array

typedef struct _WDF_DEVICE_INFO{

    PMP_ADAPTER Adapter;

} WDF_DEVICE_INFO, *PWDF_DEVICE_INFO;

WDF_DECLARE_CONTEXT_TYPE_WITH_NAME(WDF_DEVICE_INFO, GetWdfDeviceInfo)

//
//  Registery definition
//
typedef struct  _RTMP_REG_ENTRY
{
    NDIS_STRING     RegName;            // variable name text
    BOOLEAN         bRequired;          // 1 -> required, 0 -> optional
    ULONG           Type;               // Field Type
    UINT            FieldOffset;        // offset to MP_ADAPTER field
    UINT            FieldSize;          // size (in bytes) of the field
    UINT            Default;            // default value to use
    UINT            Min;                // minimum value allowed
    UINT            Max;                // maximum value allowed
}   RTMP_REG_ENTRY, *PRTMP_REG_ENTRY;

//
// SHA context
//
typedef struct _SHA_CTX
{
    ULONG       H[5];
    ULONG       W[80];
    INT         lenW;
    ULONG       sizeHi, sizeLo;
}   SHA_CTX;

#define SHA256_BLOCK_SIZE   64 /* 512 bits = 64 bytes */
#define SHA256_DIGEST_SIZE  32 /* 256 bits = 32 bytes */
typedef struct _SHA256_CTX_STRUC {
    ULONG HashValue[8];  /* 8 = (SHA256_DIGEST_SIZE / 32) */
    UINT64 MessageLen;    /* total size */
    UCHAR  Block[SHA256_BLOCK_SIZE];
    ULONG  BlockLen;
} SHA256_CTX_STRUC, *PSHA256_CTX_STRUC;

typedef enum RaNdisRequestType
{
    RaNdisRequestQuery,
    RaNdisRequestSet,
    RaNdisRequestMethod,
} RA_NDIS_REQUEST_TYPE;
 

typedef struct RaNdisRequest {
    UCHAR   OUI[4]; 
    RA_NDIS_REQUEST_TYPE   requestType; 
    ULONG                   oid;
    PVOID                   pInfoBuffer;        // Information Buffer must appear directly following this structure
    UINT                    infoBufferLength;
    UINT                    bytesReadOrWritten; // Filled in by driver. Written for query.  Read for Set.
    UINT                    bytesNeeded;        // Filled in by driver
} RA_NDIS_REQUEST,*PRA_NDIS_REQUEST;

////////////////////////////////////////////////////////////////////////////
//
//  inline FUNCTIONS
//
//   IncrementIoCount and DecrementIoCount
//
//  DESCRIPTION
//
//   We keep a pending IO count. This count is initialized to one.
//   Subsequently, the count is incremented for each new IRP received or
//   created and is decremented when each IRP is completed.
//   Transition to 'one' therefore indicates no IO is pending and signals
//   Adapter->RemoveEvent. Transition to 'zero' signals an event
//   (Adapter->RemoveEvent) to enable device removal.
//
//  INPUT
//
//    Adapter                       Ptr to the adapter object
//
//  OUTPUT
//
//    -
//
////////////////////////////////////////////////////////////////////////////

#define DEVICE_VENDOR_REQUEST_OUT       0x40
#define DEVICE_VENDOR_REQUEST_IN        0xc0
#define INTERFACE_VENDOR_REQUEST_OUT    0x41
#define INTERFACE_VENDOR_REQUEST_IN     0xc1

// BBP & RF are using indirect access. Before write any value into it.
// We have to make sure there is no outstanding command pending via checking busy bit.
//
#define MAX_BUSY_COUNT  20         // Number of retry before failing access BBP & RF indirect register



CHAR    MlmeSyncConvertToRssi(
    IN PMP_ADAPTER  pAd,
    IN  UCHAR   Rssi,
    IN UCHAR    RssiNumber,
    IN UCHAR    AntSel,
    IN UCHAR    BW);

VOID MlmeSyncBuildEffectedChannelList(
    IN PMP_ADAPTER pAd);

VOID MlmeSyncDeleteEffectedChannelList(
    IN PMP_ADAPTER pAd);

////
// prototype in action.c
//
VOID ActionStateMachineInit(
    IN  PMP_ADAPTER   pAd, 
    IN  STATE_MACHINE *S, 
    OUT STATE_MACHINE_FUNC Trans[]);

VOID MlmeADDBAAction(
    IN PMP_ADAPTER pAd, 
    IN MLME_QUEUE_ELEM *Elem);

VOID MlmeDELBAAction(
    IN PMP_ADAPTER pAd, 
    IN MLME_QUEUE_ELEM *Elem);

VOID MlmeQOSAction(
    IN PMP_ADAPTER pAd, 
    IN MLME_QUEUE_ELEM *Elem);

VOID MlmeInvalidAction(
    IN PMP_ADAPTER pAd, 
    IN MLME_QUEUE_ELEM *Elem);

#ifdef WFD_NEW_PUBLIC_ACTION
VOID MlmeP2pMsNullState(
    IN PMP_ADAPTER pAd,
    IN PMP_PORT     pPort,
    IN  PUCHAR  pInBuffer,
    IN  ULONG   InBufferLen);
    
VOID MlmeP2pMsSendState(
    IN PMP_ADAPTER pAd,
    IN PMP_PORT     pPort,
    IN  PUCHAR  pInBuffer,
    IN  ULONG   InBufferLen);

VOID MlmeP2pMsWaitAckState(
    IN PMP_ADAPTER pAd,
    IN  PUCHAR  pInBuffer,
    IN  ULONG   InBufferLen);
    
VOID MlmeP2pMsWiatRspState(
    IN PMP_ADAPTER pAd,
    IN  PUCHAR  pInBuffer,
    IN  ULONG   InBufferLen);
    
VOID MlmeP2pMsPublicActionFrameSend(
    IN PMP_ADAPTER    pAd,
    IN PMP_PORT         pPort,
    IN UCHAR            Signal);

VOID MlmeP2pMsPublicActionFrameGotAck(
    IN PMP_ADAPTER    pAd);

VOID MlmeP2pMsPublicActionFrameStateMachine(
    IN PMP_ADAPTER pAd, 
    IN MLME_QUEUE_ELEM *Elem); 

VOID MlmeP2pMsPublicActionFrameStateMachineExec(
    IN PMP_ADAPTER pAd,
    IN PMP_PORT      pPort,
    IN  PUCHAR  pInBuffer,
    IN  ULONG   InBufferLen);

VOID P2pMsPublicActionFrameWaitTimerTimeoutCallback(
    IN PVOID SystemSpecific1, 
    IN PVOID FunctionContext, 
    IN PVOID SystemSpecific2, 
    IN PVOID SystemSpecific3);

VOID P2pMsTriggerReadTXFIFOTimeoutCallback(
    IN PVOID SystemSpecific1, 
    IN PVOID FunctionContext, 
    IN PVOID SystemSpecific2, 
    IN PVOID SystemSpecific3);

VOID P2pMsPublicActionFrameWaitTimerTimeoutExec(
    IN PMP_PORT pPort);

VOID MlmeP2pMsSwitchCh(
    IN PMP_ADAPTER pAd,
    IN UCHAR Channel);

 LPSTR MlmeP2pMsStateDecode (UCHAR P2pState);

 LPSTR MlmeP2pMsSignalDecode (UCHAR P2pSignal);
 
#endif /*WFD_NEW_PUBLIC_ACTION*/

VOID PeerAddBAReqAction(
    IN PMP_ADAPTER pAd, 
    IN MLME_QUEUE_ELEM *Elem);

VOID PeerAddBARspAction(
    IN PMP_ADAPTER pAd, 
    IN MLME_QUEUE_ELEM *Elem);

VOID PeerDelBAAction(
    IN PMP_ADAPTER pAd, 
    IN MLME_QUEUE_ELEM *Elem);

VOID PeerSpectrumAction(
    IN PMP_ADAPTER pAd, 
    IN MLME_QUEUE_ELEM *Elem);

VOID PeerQOSAction(
    IN PMP_ADAPTER pAd, 
    IN MLME_QUEUE_ELEM *Elem);

VOID PeerBAAction(
    IN PMP_ADAPTER pAd, 
    IN MLME_QUEUE_ELEM *Elem);

VOID PeerPublicAction(
    IN PMP_ADAPTER pAd, 
    IN MLME_QUEUE_ELEM *Elem);

VOID StaPeerPublicAction(
    IN PMP_ADAPTER pAd, 
    IN PMP_PORT pPort,
    IN UCHAR Bss2040Coexist);

VOID PeerRMAction(
    IN PMP_ADAPTER pAd, 
    IN MLME_QUEUE_ELEM *Elem);

VOID PeerHTAction(
    IN PMP_ADAPTER pAd, 
    IN MLME_QUEUE_ELEM *Elem);

VOID RECBATimerTimeout(
    IN PMP_ADAPTER    pAd);

VOID ORIBATimerTimeout(
    IN  PMP_ADAPTER   pAd,
    IN  PMP_PORT      pPort);

VOID ActHeaderInit(
    IN  PMP_ADAPTER   pAd, 
    IN PMP_PORT pPort,
    IN OUT PHEADER_802_11 pHdr80211, 
    IN PUCHAR pDA, 
    IN PUCHAR pBssid);

VOID BarHeaderInit(
    IN  PMP_ADAPTER   pAd, 
    IN PMP_PORT pPort,
    IN OUT PFRAME_BAR pCntlBar, 
    IN PUCHAR pDA);

BOOLEAN QosBADataParse(
    IN PMP_ADAPTER    pAd,
     IN PMP_PORT  pPort, 
    IN BOOLEAN bAMSDU,
    IN BOOLEAN bIsBtAcl,
    IN PUCHAR p80211DataFrame,
    IN UCHAR    Wcid,
    IN UCHAR    TID,
    IN USHORT Sequence,
    IN USHORT u80211FrameSize);

BOOLEAN CntlEnqueueForRecv(
    IN  PMP_ADAPTER   pAd, 
    IN  PMP_PORT      pPort, 
    IN ULONG Wcid, 
    IN ULONG MsgLen, 
    IN PFRAME_BA_REQ pMsg);

VOID BaAutoManSwitch(
    IN  PMP_ADAPTER   pAd,
    IN  PMP_PORT      pPort);

VOID SendNotifyChannelWidthAction(
    IN PMP_ADAPTER pAd,
    IN  PMP_PORT      pPort,
    IN UCHAR Wcid,
    IN UCHAR Width);

VOID SendSMPSAction(
    IN  PMP_ADAPTER   pAd,
    IN  PMP_PORT      pPort,
    IN    UCHAR  Wcid,
    IN    UCHAR  Smps);

BOOLEAN ChannelSwitchSanityCheck(
    IN  PMP_ADAPTER   pAd,
    IN    UCHAR  Wcid,
    IN    UCHAR  NewChannel,
    IN    UCHAR  Secondary);

VOID ChannelSwitchAction(
    IN  PMP_ADAPTER   pAd,
    IN    UCHAR  Wcid,
    IN    UCHAR  Channel,
    IN    UCHAR  Secondary);

ULONG BuildIntolerantChannelRep(
    IN  PMP_ADAPTER   pAd,
    IN    PUCHAR  pDest); 

VOID Update2040CoexistFrameAndNotify(
    IN  PMP_ADAPTER   pAd,
    IN  PMP_PORT  pPort,
    IN    UCHAR  Wcid,
    IN  BOOLEAN bAddIntolerantCha);

VOID Send2040CoexistAction(
    IN  PMP_ADAPTER   pAd,
    IN  PMP_PORT  pPort,
    IN    UCHAR  Wcid,
    IN  BOOLEAN bAddIntolerantCha);

VOID UpdateBssScanParm(
    IN  PMP_ADAPTER   pAd,
    IN  OVERLAP_BSS_SCAN_IE APBssScan);

//// 
// function prototype in ap_wpa.c
//

BOOLEAN ApWpaMsgTypeSubst(
    IN UCHAR    EAPType,
    OUT ULONG *MsgType) ;

BOOLEAN ApWpaCheckWPAframe(
    IN PMP_ADAPTER pAd,
    IN PUCHAR pData,
    IN ULONG DataByteCount);

BOOLEAN ApWpaCheckMcast(
    IN PMP_ADAPTER pAd,
    IN PMP_PORT       pPort,
    IN PEID_STRUCT      eid_ptr,
    IN MAC_TABLE_ENTRY  *pEntry);

BOOLEAN ApWpaCheckUcast(
    IN PMP_ADAPTER pAd,
    IN PMP_PORT       pPort,
    IN PEID_STRUCT      eid_ptr,
    IN MAC_TABLE_ENTRY  *pEntry);

BOOLEAN ApWpaCheckAuth(
    IN PMP_ADAPTER pAd,
        IN PMP_PORT       pPort,
    IN PEID_STRUCT      eid_ptr,
    IN MAC_TABLE_ENTRY  *pEntry);

VOID ApWpaStart4WayHS(
    IN PMP_ADAPTER    pAd, 
    IN PMP_PORT       pPort,
    IN MAC_TABLE_ENTRY  *pEntry);

VOID ApWpaEAPPacketAction(
    IN PMP_ADAPTER pAd, 
    IN MLME_QUEUE_ELEM *Elem);

VOID ApWpaEAPOLStartAction(
    IN PMP_ADAPTER pAd, 
    IN MLME_QUEUE_ELEM *Elem);

VOID ApWpaEAPOLLogoffAction(
    IN PMP_ADAPTER pAd, 
    IN MLME_QUEUE_ELEM *Elem);

VOID ApWpaEAPOLKeyAction(
    IN PMP_ADAPTER pAd, 
    IN MLME_QUEUE_ELEM *Elem);

VOID ApWpaEAPOLASFAlertAction(
    IN  PMP_ADAPTER    pAd, 
    IN  MLME_QUEUE_ELEM  *Elem);

VOID ApWpaHandleCounterMeasure(
    IN PMP_ADAPTER pAd, 
    IN MAC_TABLE_ENTRY  *pEntry);

MAC_TABLE_ENTRY *ApWpaPACInquiry(
    IN  PMP_ADAPTER               pAd, 
    IN  PUCHAR                      pAddr, 
    OUT NDIS_802_11_PRIVACY_FILTER  *Privacy,
    OUT AP_WPA_STATE                *WpaState);

VOID ApWpaPeerPairMsg2Action(
    IN PMP_ADAPTER pAd, 
    IN MAC_TABLE_ENTRY  *pEntry,
    IN MLME_QUEUE_ELEM *Elem);

VOID ApWpaCMTimerExecTimerCallback(
    IN PVOID SystemSpecific1, 
    IN PVOID FunctionContext, 
    IN PVOID SystemSpecific2, 
    IN PVOID SystemSpecific3);

VOID ApWpa1PeerPairMsg4Action(
    IN PMP_ADAPTER pAd, 
    IN MAC_TABLE_ENTRY  *pEntry,
    IN MLME_QUEUE_ELEM *Elem);

VOID ApWpa2PeerPairMsg4Action(
    IN  PMP_ADAPTER    pAd, 
    IN  PMAC_TABLE_ENTRY pEntry,
    IN  MLME_QUEUE_ELEM  *Elem);

VOID ApWpaPeerGroupMsg2Action(
    IN  PMP_ADAPTER    pAd, 
    IN  PMAC_TABLE_ENTRY pEntry,
    IN  VOID             *Msg,
    IN  UINT             MsgLen);

VOID    Wpa2PairMsg1Action(
    IN  PMP_ADAPTER   pAd, 
    IN  MLME_QUEUE_ELEM *Elem);

VOID    Wpa2PairMsg3Action(
    IN  PMP_ADAPTER   pAd, 
    IN  MLME_QUEUE_ELEM *Elem); 

NDIS_STATUS ApWpaHardTransmit(
    IN  PMP_ADAPTER    pAd,
    IN  PMAC_TABLE_ENTRY pEntry);

VOID PairApWpaDisAssocAction(
    IN  PMP_ADAPTER    pAd, 
    IN  PMAC_TABLE_ENTRY pEntry,
    IN  USHORT           Reason);

VOID ApWpaDisAssocAction(
    IN  PMP_ADAPTER    pAd, 
    IN  PMP_PORT pPort,
    IN  PMAC_TABLE_ENTRY pEntry,
    IN  USHORT           Reason);

VOID ApWpaGREKEYPeriodicExecTimerCallback(
    IN  PVOID   SystemSpecific1, 
    IN  PVOID   FunctionContext, 
    IN  PVOID   SystemSpecific2, 
    IN  PVOID   SystemSpecific3);

VOID ApWpaCountPTK(
    IN  PMP_ADAPTER pAd,  
    IN  UCHAR   *PMK,
    IN  UCHAR   *ANonce,
    IN  UCHAR   *AA,
    IN  UCHAR   *SNonce,
    IN  UCHAR   *SA,
    OUT UCHAR   *output,
    IN  UINT    len);

VOID ApWpaCountGTK(
    IN  PMP_ADAPTER pAd,  
    IN  UCHAR   *PMK,
    IN  UCHAR   *GNonce,
    IN  UCHAR   *AA,
    OUT UCHAR   *output,
    IN  UINT    len);

VOID    ApWpaGetSmall(
    IN  PVOID   pSrc1,
    IN  PVOID   pSrc2,
    OUT PUCHAR  out,
    IN  ULONG   Length);

VOID    ApWpaGetLarge(
    IN  PVOID   pSrc1,
    IN  PVOID   pSrc2,
    OUT PUCHAR  out,
    IN  ULONG   Length);

VOID ApWpaGenRandom(
    IN PMP_ADAPTER pAd, 
    IN PMP_PORT     pPort,
    OUT UCHAR       *random);

VOID ApWpaAesGtkkeyWrap( 
    IN PMP_ADAPTER pAd,
    IN UCHAR *key,
    IN UCHAR *plaintext,
    IN UCHAR p_len,
    OUT UCHAR *ciphertext);

VOID ApWpaMakeRSNIE(
    IN  PMP_ADAPTER   pAd,
    IN  UINT            AuthMode,
    IN  UINT            WepStatus);

VOID    ApWpaToWirelessSta(
    IN  PMP_ADAPTER   pAd,
    IN  PMP_PORT      pPort,
    IN  USHORT          Aid,
    IN  PUCHAR          pHeader802_3,
    IN  UINT            HdrLen,
    IN  PUCHAR          pData,
    IN  UINT            DataLen);

VOID ApMlmeAsicRxAntEvalTimeout(
    PMP_ADAPTER   pAd);

VOID ApMlmeAsicEvaluateRxAnt(
    IN PMP_ADAPTER    pAd);

////
// prototype in assoc.c
//
VOID MlmeAssocStateMachineInit(
    IN  PMP_ADAPTER       pAd, 
    IN  STATE_MACHINE       *S, 
    OUT STATE_MACHINE_FUNC  Trans[]) ;

VOID MlmeAssocTimeoutTimerCallback(
    IN  PVOID   SystemSpecific1, 
    IN  PVOID   FunctionContext, 
    IN  PVOID   SystemSpecific2, 
    IN  PVOID   SystemSpecific3) ;

VOID MlmeAssocReAssocTimeoutTimerCallback(
    IN  PVOID   SystemSpecific1, 
    IN  PVOID   FunctionContext, 
    IN  PVOID   SystemSpecific2, 
    IN  PVOID   SystemSpecific3) ;

VOID
MlmeAssocConnectionTimeoutTimerCallback(
    IN  PVOID   SystemSpecific1, 
    IN  PVOID   FunctionContext, 
    IN  PVOID   SystemSpecific2, 
    IN  PVOID   SystemSpecific3
    ) ;
    
VOID MlmeAssocDisAssocTimeoutTimerCallback(
    IN  PVOID   SystemSpecific1, 
    IN  PVOID   FunctionContext, 
    IN  PVOID   SystemSpecific2, 
    IN  PVOID   SystemSpecific3) ;

VOID MlmeAssocReqAction(
    IN  PMP_ADAPTER       pAd,    
    IN  MLME_QUEUE_ELEM     *Elem) ;

VOID MlmeReassocReqAction(
    IN PMP_ADAPTER pAd, 
    IN MLME_QUEUE_ELEM *Elem) ;

VOID MlmeDisassocReqAction(
    IN PMP_ADAPTER pAd, 
    IN MLME_QUEUE_ELEM *Elem) ;

VOID MlmeDisassocReqActionforPowerSave(
    IN PMP_ADAPTER pAd, 
    IN MLME_QUEUE_ELEM *Elem) ;

VOID MlmeAssocPeerAssocRspAction(
    IN PMP_ADAPTER pAd, 
    IN MLME_QUEUE_ELEM *Elem) ;

VOID MlmeAssocPeerReassocRspAction(
    IN PMP_ADAPTER pAd, 
    IN MLME_QUEUE_ELEM *Elem) ;

VOID MlmeAssocPostProc(
    IN PMP_ADAPTER pAd, 
    IN PMP_PORT pPort, 
    IN PUCHAR pAddr2, 
    IN USHORT CapabilityInfo, 
    IN USHORT Aid, 
    IN UCHAR SupRate[], 
    IN UCHAR SupRateLen,
    IN UCHAR ExtRate[],
    IN UCHAR ExtRateLen,
    IN PEDCA_PARM pEdcaParm,
    IN HT_CAPABILITY_IE     *pHtCapability,
    IN UCHAR HtCapabilityLen, 
    IN ADD_HT_INFO_IE     *pAddHtInfo,
    IN BOOLEAN            bVhtCapable,
    IN VHT_CAP_IE         *VhtCapability,
    IN VHT_OP_IE          *VhtOperation, 
    IN PEXT_CAP_ELEMT pExtendedCapabilities);

VOID MlmeAssocPeerDisassocAction(
    IN PMP_ADAPTER pAd, 
    IN MLME_QUEUE_ELEM *Elem) ;

VOID MlmeAssocTimeoutTimerCallbackAction(
    IN PMP_ADAPTER pAd, 
    IN MLME_QUEUE_ELEM *Elem) ;

VOID MlmeAssocReAssocTimeoutTimerCallbackAction(
    IN PMP_ADAPTER pAd, 
    IN MLME_QUEUE_ELEM *Elem) ;

VOID MlmeAssocDisAssocTimeoutTimerCallbackAction(
    IN PMP_ADAPTER pAd, 
    IN MLME_QUEUE_ELEM *Elem) ;

VOID MlmeAssocInvalidStateWhenAssoc(
    IN PMP_ADAPTER pAd, 
    IN MLME_QUEUE_ELEM *Elem) ;

VOID MlmeAssocInvalidStateWhenReassoc(
    IN PMP_ADAPTER pAd, 
    IN MLME_QUEUE_ELEM *Elem) ;

VOID MlmeAssocInvalidStateWhenDisassociate(
    IN PMP_ADAPTER pAd, 
    IN MLME_QUEUE_ELEM *Elem) ;

VOID MlmeAssocCls3errAction(
    IN PMP_ADAPTER pAd, 
    IN PUCHAR        pAddr) ;

 

VOID MlmeAssocSwitchBetweenWepAndCkip(
    IN PMP_ADAPTER pAd,
    IN PMP_PORT pPort);
////
// prototype in auth.c
//
void MlmeAuthStateMachineInit(
    IN PMP_ADAPTER pAd, 
    IN STATE_MACHINE *Sm, 
    OUT STATE_MACHINE_FUNC Trans[]) ;

VOID MlmeAuthTimeoutTimerCallback(
    IN PVOID SystemSpecific1, 
    IN PVOID FunctionContext, 
    IN PVOID SystemSpecific2, 
    IN PVOID SystemSpecific3) ;

VOID MlmeAuthReqAction(
    IN PMP_ADAPTER pAd, 
    IN MLME_QUEUE_ELEM *Elem) ;

VOID MlmeAuthPeerAuthRspAtSeq2Action(
    IN PMP_ADAPTER pAd, 
    IN MLME_QUEUE_ELEM *Elem) ;

VOID MlmeAuthPeerAuthRspAtSeq4Action(
    IN PMP_ADAPTER pAd, 
    IN MLME_QUEUE_ELEM *Elem) ;

VOID MlmeAuthDeauthReqAction(
    IN PMP_ADAPTER pAd, 
    IN NDIS_PORT_NUMBER PortNum,
    IN MLME_DEAUTH_REQ_STRUCT *pInfo) ;

VOID MlmeAuthTimeoutAction(
    IN PMP_ADAPTER pAd, 
    IN MLME_QUEUE_ELEM *Elem); 

VOID MlmeAuthInvalidStateWhenAuth(
    IN PMP_ADAPTER pAd, 
    IN MLME_QUEUE_ELEM *Elem) ;

VOID MlmeAuthCls2errAction(
    IN PMP_ADAPTER pAd, 
    IN  PMP_PORT      pPort,
    IN PUCHAR pAddr) ;
////
// prototype in auth_rsp.c
//
VOID MlmeAuthRspStateMachineInit(
    IN PMP_ADAPTER pAd, 
    IN PSTATE_MACHINE Sm, 
    IN STATE_MACHINE_FUNC Trans[]) ;

VOID MlmeAuthRspPeerDeauthAction(
    IN PMP_ADAPTER pAd, 
    IN PMLME_QUEUE_ELEM Elem) ;


////
// prototype in connect.c
//
VOID MlmeCntlInit(
    IN PMP_ADAPTER pAd, 
    IN STATE_MACHINE *S, 
    OUT STATE_MACHINE_FUNC Trans[]) ;

VOID MlmeCntPortCntlInit(
    IN PMP_PORT pPort, 
    IN STATE_MACHINE *S, 
    OUT STATE_MACHINE_FUNC Trans[]) ;

VOID MlmeCntlMachinePerformAction(
    IN PMP_ADAPTER pAd, 
    IN STATE_MACHINE *S, 
    IN MLME_QUEUE_ELEM *Elem) ;

VOID MlmeCntlIdleProc(
    IN PMP_ADAPTER pAd, 
    IN MLME_QUEUE_ELEM *Elem) ;

VOID MlmeCntlOidScanProc(
    IN PMP_ADAPTER pAd,
    IN MLME_QUEUE_ELEM *Elem);

VOID MlmeCntlOidSsidProc(
    IN PMP_ADAPTER pAd, 
    IN MLME_QUEUE_ELEM * Elem) ;

VOID MlmeCntlWscIterate(
    IN PMP_ADAPTER    pAd,
    IN PMP_PORT       pPort);

VOID MlmeCntlOidRTBssidProc(
    IN PMP_ADAPTER pAd, 
    IN MLME_QUEUE_ELEM * Elem) ;

VOID MlmeCntlMlmeRoamingProc(
    IN PMP_ADAPTER pAd, 
    IN MLME_QUEUE_ELEM *Elem);

VOID MlmeCntlOidDLSSetupProc(
    IN PMP_ADAPTER pAd,
    IN MLME_QUEUE_ELEM *Elem);

VOID MlmeCntlOidTDLSRequestProc(
    IN PMP_ADAPTER pAd,
    IN MLME_QUEUE_ELEM *Elem);

VOID MlmeCntlWaitDisassocProc(
    IN PMP_ADAPTER pAd, 
    IN MLME_QUEUE_ELEM *Elem) ;

VOID MlmeCntlWaitJoinProc(
    IN PMP_ADAPTER pAd, 
    IN MLME_QUEUE_ELEM *Elem) ;

VOID MlmeCntlChannelWidth(
    IN PMP_ADAPTER    pAd,
    IN UCHAR            PrimaryChannel,
    IN UCHAR            CentralChannel, 
    IN UCHAR            ChannelWidth,
    IN UCHAR            SecondaryChannelOffset);


VOID MlmeCntlWaitStartProc(
    IN PMP_ADAPTER pAd, 
    IN MLME_QUEUE_ELEM *Elem) ;

VOID MlmeCntlWaitAuthProc(
    IN PMP_ADAPTER pAd, 
    IN MLME_QUEUE_ELEM *Elem) ;

VOID MlmeCntlWaitAuthProc2(
    IN PMP_ADAPTER pAd, 
    IN MLME_QUEUE_ELEM *Elem) ;

VOID MlmeCntlWaitAssocProc(
    IN PMP_ADAPTER pAd, 
    IN MLME_QUEUE_ELEM *Elem) ;

VOID MlmeCntlWaitReassocProc(
    IN PMP_ADAPTER pAd, 
    IN MLME_QUEUE_ELEM *Elem) ;

VOID MlmeCntLinkUp(
    IN PMP_PORT pPort,
    IN UCHAR BssType) ;

VOID MlmeCntLinkDown(
    IN PMP_PORT   pPort,
    IN  BOOLEAN     IsReqFromAP);

VOID MlmeCntlConfirm(
    IN PMP_ADAPTER pAd, 
    IN ULONG MsgType, 
    IN USHORT Msg) ;

VOID MlmeCntIterateOnBssTab(
    IN PMP_PORT pPort) ;

VOID MlmeCntIterateOnBssTab2(
    IN PMP_PORT pPort) ;

VOID MlmeCntJoinParmFill(
    IN PMP_ADAPTER pAd, 
    IN OUT MLME_JOIN_REQ_STRUCT *JoinReq, 
    IN ULONG BssIdx) ;

VOID MlmeCntAssocParmFill(
    IN PMP_ADAPTER pAd, 
    IN OUT MLME_ASSOC_REQ_STRUCT *AssocReq, 
    IN PUCHAR                     pAddr, 
    IN USHORT                     CapabilityInfo, 
    IN ULONG                      Timeout, 
    IN USHORT                     ListenIntv) ;

VOID MlmeCntScanParmFill(
    IN PMP_ADAPTER pAd, 
    IN OUT MLME_SCAN_REQ_STRUCT *ScanReq, 
    IN CHAR Ssid[], 
    IN UCHAR SsidLen, 
    IN UCHAR BssType, 
    IN UCHAR ScanType); 

VOID MlmeCntCheckPMKIDCandidate(
    IN PMP_PORT pPort);

VOID MlmeCntDlsParmFill(
    IN PMP_ADAPTER pAd, 
    IN OUT MLME_DLS_REQ_STRUCT *pDlsReq,
    IN PRT_802_11_DLS pDls,
    IN USHORT reason); 

VOID MlmeCntTdlsParmFill(
    IN PMP_ADAPTER pAd, 
    IN OUT MLME_TDLS_REQ_STRUCT *pTdlsReq,
    IN PRT_802_11_TDLS pTdls,
    IN USHORT Reason);


VOID MlmeCntDisassocParmFill(
    IN PMP_ADAPTER pAd, 
    IN OUT MLME_DISASSOC_REQ_STRUCT *DisassocReq, 
    IN PUCHAR pAddr, 
    IN USHORT Reason); 

VOID MlmeCntStartParmFill(
    IN PMP_PORT pPort, 
    IN OUT MLME_START_REQ_STRUCT *StartReq, 
    IN CHAR Ssid[], 
    IN UCHAR SsidLen); 

VOID MlmeCntAuthParmFill(
    IN PMP_PORT pPort, 
    IN OUT MLME_AUTH_REQ_STRUCT *AuthReq, 
    IN PUCHAR pAddr, 
    IN USHORT Alg) ;

VOID MlmeCntComposePsPoll(
    IN PMP_ADAPTER pAd,
    IN PMP_PORT pPort);

VOID MlmeCntComposeNullFrame(
    IN PMP_ADAPTER pAd,
    IN PMP_PORT   pPort);

////
// prototype in dls.c
//

void DlsStateMachineInit(
    IN PMP_ADAPTER pAd, 
    IN STATE_MACHINE *Sm, 
    OUT STATE_MACHINE_FUNC Trans[]);

VOID MlmeDlsReqAction(
    IN PMP_ADAPTER pAd, 
    IN MLME_QUEUE_ELEM *Elem);

VOID PeerDlsReqAction(
    IN PMP_ADAPTER pAd, 
    IN MLME_QUEUE_ELEM *Elem);

VOID PeerDlsRspAction(
    IN PMP_ADAPTER pAd, 
    IN MLME_QUEUE_ELEM *Elem);

VOID MlmeDlsTearDownAction(
    IN PMP_ADAPTER pAd, 
    IN MLME_QUEUE_ELEM *Elem);

VOID PeerDlsTearDownAction(
    IN PMP_ADAPTER pAd, 
    IN MLME_QUEUE_ELEM *Elem);

VOID RTMPCheckDLSTimeOut(
    IN PMP_ADAPTER    pAd,
    IN PMP_PORT       pPort);

BOOLEAN RTMPRcvFrameDLSCheck(
    IN PMP_ADAPTER    pAd,
    IN PMP_PORT       pPort,
    IN PHEADER_802_11   pHeader,
    IN ULONG            Len,
    IN PRXINFO_STRUC    pRxInfo);

int RTMPCheckDLSFrame(
    IN  PMP_ADAPTER   pAd,
    IN  PMP_PORT  pPort,
    IN  PUCHAR          pDA);

VOID RTMPSendDLSTearDownFrame(
    IN  PMP_ADAPTER   pAd,
    IN  PUCHAR          pDA);

NDIS_STATUS RTMPSendSTAKeyRequest(
    IN  PMP_ADAPTER   pAd,
    IN  PMP_PORT      pPort,
    IN  PUCHAR          pDA);

NDIS_STATUS RTMPSendSTAKeyHandShake(
    IN  PMP_ADAPTER   pAd,
    IN  PMP_PORT      pPort,
    IN  PUCHAR          pDA);

VOID DlsTimeoutTimerCallback(
    IN PVOID SystemSpecific1, 
    IN PVOID FunctionContext, 
    IN PVOID SystemSpecific2, 
    IN PVOID SystemSpecific3);

VOID DlsTearDown(
    IN PMP_ADAPTER  pAd,
    IN PMP_PORT     pPort);

VOID DlsDeleteMacTableEntry(
    IN PMP_ADAPTER pAd,
    IN PMP_PORT    pPort,
    IN PUCHAR pMacAddr);


////
// prototype in MLME.c
//

NDIS_STATUS PortMlmeInit(
    IN PMP_ADAPTER pAd,
    IN PMP_PORT pPort
    ) ;

//
// Initialize the frequency calibration
//
// Parameters
//  pAd: The adapter data structure
//
// Return Value:
//  None
//
VOID InitFrequencyCalibration(
    IN PMP_ADAPTER pAd);

//
// To stop the frequency calibration algorithm
//
// Parameters
//  pAd: The adapter data structure
//
// Return Value:
//  None
//
VOID StopFrequencyCalibration(
    IN PMP_ADAPTER pAd);

//
// The frequency calibration algorithm
//
// Parameters
//  pAd: The adapter data structure
//
// Return Value:
//  None
//
VOID FrequencyCalibration(
    IN PMP_ADAPTER pAd);

//
// Get the frequency offset
//
// Parameters
//  pAd: The adapter data structure
//  pRxWI: Point to the RxWI structure
//
// Return Value:
//  The frequency offset
//
UCHAR GetFrequencyOffset(
    IN PMP_ADAPTER pAd, 
    IN PRXWI_STRUC pRxWI);

//
// Initialize PPAD (per-packet antenna diversity)
//
// Parameters
//  pAd: The adapter data structure
//
// Return Value:
//  None
//
VOID InitPPAD(
    IN PMP_ADAPTER pAd);

VOID
MlmePortWorkItemInit(
    IN PMP_ADAPTER pAd,
    IN PMP_PORT pPort
    );

VOID
MlmePortTimeInit(
    IN PMP_ADAPTER pAd,
    IN PMP_PORT pPort
    );
NDIS_STATUS MlmeInit(
    IN PMP_ADAPTER pAd);

VOID MlmeCancelAllTimers(
    IN PMP_ADAPTER pAd);

//
// Rounding to integer
// e.g., +16.9 ~= 17 and -16.9 ~= -17
//
// Parameters
//  pAd: The adapter data structure
//  Integer: Integer part
//  Fraction: Fraction part
//  DenominatorOfTssiRatio: The denominator of the TSSI ratio
//
// Return Value:
//  Rounding result
//
LONG Rounding(
    IN PMP_ADAPTER pAd, 
    IN LONG Integer, 
    IN LONG Fraction, 
    IN LONG DenominatorOfTssiRatio);

//
// Initialize the 802.11k noise histogram measurement (NHM) control
//
// Parameters
//  pAd: The adapter data structure
//  NHMExternGain: Extern LNA + channel filter loss + antenna switch loss, -31 dB ~ +31 dB
//  NHMPeriod: Noise histogram measurement period (0: 1us, 1: 2us, 2: 3us, 3: 4us)
//  IPIMsrnPeriod: Measurement period of idel power indicator (IPI), in unit of 1.024 ms
//
// Return Value:
//  None
//
VOID RRMInitNHMCtrl(
    IN PMP_ADAPTER pAd, 
    IN CHAR NHMExternGain, 
    IN UCHAR NHMPeriod, 
    IN ULONG IPIMsrnPeriod);

//
// IPI0, IPI1, IPI2, IPI3, IPI4, IPI5, IPI6, IPI7, IPI8, IPI9 and IPI10
//
#define NUM_OF_IPI_VALUE            11

//
// Idle time
//
#define NUM_OF_IDLE_TIME            1

//
// Get the IPI densities after performing the noise histogram measurement (NHM)
//
// Parameters
//  pAd: The adapter data structure
//  pIPIDensityArray: Point to a caller-supplied arrary in which stores the IPI densities (IPI0 density, IPI1 density, ..., IPI10 density)
//
// Return Value:
//  None
//
VOID RRMGetIPIDensities(
    IN PMP_ADAPTER pAd, 
    IN PUCHAR pIPIDensityArray);

VOID MlmeHandler(
    IN PMP_ADAPTER pAd); 

VOID
MlmePortFreeWorkItem(
    IN PMP_ADAPTER pAd,
    IN PMP_PORT pPort
    );

VOID
MlmePortFreeTimer(
    IN PMP_ADAPTER pAd,
    IN PMP_PORT pPort
    );

VOID 
MlmeHalt(
    IN PMP_ADAPTER pAd,
    IN PMP_PORT pPort
    );

VOID  ReSendBulkinIRPsTimerCallback(
    IN PVOID SystemSpecific1, 
    IN PVOID FunctionContext, 
    IN PVOID SystemSpecific2, 
    IN PVOID SystemSpecific3);

VOID  ForceSleepTimeOutTimerCallback(
    IN PVOID SystemSpecific1, 
    IN PVOID FunctionContext, 
    IN PVOID SystemSpecific2, 
    IN PVOID SystemSpecific3);


VOID  MlmePeriodicExecTimerCallback(
    IN PVOID SystemSpecific1, 
    IN PVOID FunctionContext, 
    IN PVOID SystemSpecific2, 
    IN PVOID SystemSpecific3);

VOID MlmePeriodicExecPassivLevel(
    IN PMP_ADAPTER    pAd);

VOID STAMlmePeriodicExec(
    IN PMP_ADAPTER pAd);

VOID MlmeCntLinkDownExecTimerCallback(
    IN PVOID SystemSpecific1, 
    IN PVOID FunctionContext, 
    IN PVOID SystemSpecific2, 
    IN PVOID SystemSpecific3) ;

VOID MlmeCntLinkUpExecTimerCallback(
    IN PVOID SystemSpecific1, 
    IN PVOID FunctionContext, 
    IN PVOID SystemSpecific2, 
    IN PVOID SystemSpecific3) ;

VOID WakeUpExecTimerCallback(
    IN PVOID SystemSpecific1,
    IN PVOID FunctionContext,
    IN PVOID SystemSpecific2,
    IN PVOID SystemSpecific3);

VOID MlmeAutoScan(
    IN PMP_ADAPTER pAd    
    );

VOID MlmeAutoReconnectLastSSID(
    IN PMP_ADAPTER pAd,
    IN PMP_PORT   pPort);

BOOLEAN MlmeValidateSSID(
    IN PUCHAR   pSsid,
    IN UCHAR    SsidLen);

BOOLEAN MlmeCheckForRoaming(
    IN PMP_ADAPTER pAd,   
    IN PMP_PORT pPort,    
    IN ULONGLONG    Now64);

VOID MlmeCheckForFastRoaming(
    IN  PMP_ADAPTER   pAd,
    IN  ULONGLONG           Now);

VOID MlmeCalculateChannelQuality(
    IN PMP_ADAPTER pAd,
    IN PMP_PORT pPort,
    IN ULONGLONG Now64);

VOID MlmeSetTxRate(
    IN PMP_ADAPTER        pAd,
    IN PMAC_TABLE_ENTRY     pEntry,
    IN PRTMP_TX_RATE_SWITCH pTxRate,
    IN PUCHAR               pTable);

VOID MlmeSelectTxRateTable(
    IN PMP_ADAPTER        pAd,
    IN PMAC_TABLE_ENTRY     pEntry,
    IN PUCHAR               *ppTable,
    IN PUCHAR               pTableSize,
    IN PUCHAR               pInitTxRateIdx);

VOID MlmeUpdateLimitTxRateTable(
    IN PMP_ADAPTER    pAd,
    IN PUCHAR       pBasedTable,
    IN OUT PUCHAR       pLimitedTable);

VOID MlmeDynamicTxRateSwitching(
    IN PMP_ADAPTER pAd,
    IN PMP_PORT pPort);

//
// The statistics information for AGS
//
typedef struct _AGS_STATISTICS_INFO
{
    CHAR    RSSI;
    ULONG   TxErrorRatio;
    ULONG   AccuTxTotalCnt;
    ULONG   TxTotalCnt;
    ULONG   TxSuccess;
    ULONG   TxRetransmit;
    ULONG   TxFailCount;
} AGS_STATISTICS_INFO, *PAGS_STATISTICS_INFO;

VOID StaQuickResponeForRateUpExec(
    IN PMP_ADAPTER pAd,
    IN PMP_PORT pPort);

VOID MlmeUpdateTxStatistic(
    IN PMP_ADAPTER pAd,
    OUT TX_STA_CNT0_STRUC *pTxStaCnt0,
    OUT TX_STA_CNT1_STRUC *pStaTx1);

VOID MlmeUpdateDRSTimeAndStatistic(
    IN PMP_ADAPTER pAd,
    OUT TX_STA_CNT0_STRUC *pTxStaCnt0,
    OUT TX_STA_CNT1_STRUC *pStaTx1);

VOID MlmeCheckPsmChange(
    IN PMP_ADAPTER pAd,
    IN PMP_PORT   pPort,
    IN ULONGLONG    Now64);

VOID MlmeSetPsm(
    IN PMP_ADAPTER pAd, 
    IN USHORT psm);

VOID MlmeSetTxPreamble(
    IN PMP_ADAPTER pAd, 
    IN USHORT TxPreamble);

VOID MlmeUpdateTxRates(
    IN PMP_ADAPTER pAd,
    IN PMP_PORT   pPort,
    IN BOOLEAN       bMlmeCntLinkUp);

VOID MlmeUpdateHtVhtTxRates(
    IN PMP_ADAPTER pAd,
    IN PMP_PORT   pPort,
    IN UCHAR         Wcid);

VOID AsicUpdateAutoFallBackTable(
    IN  PMP_ADAPTER   pAd,
    IN  PUCHAR      pRateTable,
    IN  MAC_TABLE_ENTRY *pEntry);

VOID    AsicUpdateProtect(
    IN PMP_ADAPTER pAd,
    IN USHORT OperationMode,
    IN UCHAR SetMask,
    IN BOOLEAN bDisableBGProtect,
    IN BOOLEAN bNonGFExist);

VOID MlmeRadioOff(
    IN PMP_ADAPTER pAd,
    IN PMP_PORT pPort
    );

VOID MlmeRadioOn(
    IN PMP_ADAPTER pAd);

VOID BssTableInit(
    IN BSS_TABLE *Tab) ;

VOID ResetRepositBssTable(
    IN PMP_ADAPTER pAd);

VOID BATableInit(
    IN BA_TABLE *Tab) ;

ULONG BssTableSearch(
    IN PMP_ADAPTER pAd, 
    IN PMP_PORT       pPort,
    IN BSS_TABLE *Tab, 
    IN PUCHAR    pBssid,
    IN UCHAR     Channel) ;

ULONG BssSsidTableSearch(
    IN BSS_TABLE *Tab, 
    IN PUCHAR    pBssid,
    IN PUCHAR    pSsid,
    IN UCHAR     SsidLen,
    IN UCHAR     Channel) ;

ULONG BssTableSearchWithSSID(
    IN BSS_TABLE *Tab, 
    IN PUCHAR    Bssid,
    IN PUCHAR    pSsid,
    IN UCHAR     SsidLen,
    IN UCHAR     Channel);

VOID BssTableDeleteEntry(
    IN OUT  BSS_TABLE *Tab, 
    IN      PUCHAR    pBssid,
    IN      UCHAR     Channel);

VOID BssTableDeleteEntryByChannel(
    IN PMP_ADAPTER pAd, 
    IN OUT PBSS_TABLE pBSSTable, 
    IN UCHAR Channel);

VOID BATableDeleteORIEntry(
    IN OUT  PMP_ADAPTER pAd, 
    IN  PMP_PORT      pPort, 
    IN      BA_ORI_ENTRY    *pBAORIEntry);

VOID BATableDeleteRECEntry(
    IN OUT  PMP_ADAPTER pAd, 
    IN  PMP_PORT      pPort, 
    IN      BA_REC_ENTRY    *pBARECEntry);

VOID BATableTearRECEntry(
    IN OUT  PMP_ADAPTER pAd, 
    IN      PMP_PORT  pPort,
    IN      UCHAR TID, 
    IN      UCHAR Wcid, 
    IN      BOOLEAN ALL) ;

VOID BATableTearORIEntry(
    IN OUT  PMP_ADAPTER pAd, 
    IN      PMP_PORT pPort, 
    IN      UCHAR TID, 
    IN      UCHAR Wcid, 
    IN      BOOLEAN bForceDelete, 
    IN      BOOLEAN ALL) ;

VOID BssEntrySet(
    IN  PMP_ADAPTER   pAd, 
    IN PMP_PORT     pPort,
    OUT BSS_ENTRY *pBss, 
    IN PUCHAR pBssid, 
    IN CHAR Ssid[], 
    IN UCHAR SsidLen, 
    IN UCHAR BssType, 
    IN USHORT BeaconPeriod, 
    IN PCF_PARM pCfParm, 
    IN USHORT AtimWin, 
    IN USHORT CapabilityInfo, 
    IN UCHAR SupRate[], 
    IN UCHAR SupRateLen,
    IN UCHAR ExtRate[], 
    IN UCHAR ExtRateLen,
    IN HT_CAPABILITY_IE *pHtCapability,
    IN VHT_CAP_IE *pVhtCapability,
    IN VHT_OP_IE *pVhtOperation,
    IN ADD_HT_INFO_IE *pAddHtInfo,  // AP might use this additional ht info IE 
    IN UCHAR            HtCapabilityLen,
    IN UCHAR            AddHtInfoLen,
    IN UCHAR            NewExtChanOffset,
    IN UCHAR Channel,
    IN CHAR Rssi,
    IN ULONGLONG TimeStamp,
    IN PEDCA_PARM pEdcaParm,
    IN PQOS_CAPABILITY_PARM pQosCapability,
    IN PQBSS_LOAD_PARM pQbssLoad,
    IN USHORT WSCInfoAtBeaconsLen,
    IN PUCHAR WSCInfoAtBeacons,
    IN USHORT WSCInfoAtProbeRspLen,
    IN PUCHAR WSCInfoAtProbeRsp,
    IN USHORT LengthVIE,    
    IN PNDIS_802_11_VARIABLE_IEs pVIE,
    IN BOOLEAN bUpdateRssi);

ULONG BssTableSetEntry(
    IN  PMP_ADAPTER   pAd, 
    IN  PMP_PORT      pPort,
    OUT BSS_TABLE *Tab, 
    IN PUCHAR pBssid, 
    IN CHAR Ssid[], 
    IN UCHAR SsidLen, 
    IN UCHAR BssType, 
    IN USHORT BeaconPeriod, 
    IN CF_PARM *CfParm, 
    IN USHORT AtimWin, 
    IN USHORT CapabilityInfo, 
    IN UCHAR SupRate[],
    IN UCHAR SupRateLen,
    IN UCHAR ExtRate[],
    IN UCHAR ExtRateLen,
    IN HT_CAPABILITY_IE *pHtCapability,
    IN VHT_CAP_IE *pVhtCapability,
    IN VHT_OP_IE *pVhtOperation,
    IN ADD_HT_INFO_IE *pAddHtInfo,  // AP might use this additional ht info IE 
    IN UCHAR            HtCapabilityLen,
    IN UCHAR            AddHtInfoLen,
    IN UCHAR            NewExtChanOffset,
    IN UCHAR ChannelNo,
    IN CHAR Rssi,
    IN ULONGLONG TimeStamp,
    IN PEDCA_PARM pEdcaParm,
    IN PQOS_CAPABILITY_PARM pQosCapability,
    IN PQBSS_LOAD_PARM pQbssLoad,
    IN USHORT WSCInfoAtBeaconsLen, 
    IN PUCHAR WSCInfoAtBeacons,
    IN USHORT WSCInfoAtProbeRspLen,
    IN PUCHAR WSCInfoAtProbeRsp,
    IN USHORT LengthVIE,    
    IN PNDIS_802_11_VARIABLE_IEs pVIE,
    IN BOOLEAN bUpdateRssi);

VOID BATableInsertEntry(
    IN  PMP_ADAPTER   pAd, 
    IN  PMP_PORT      pPort, 
    IN UCHAR Aid, 
    IN USHORT       TimeOutValue,
    IN USHORT       StartingSeq,
    IN UCHAR TID, 
    IN UCHAR BAWinSize, 
    IN UCHAR OriginatorStatus, 
    IN BOOLEAN Recipient); 

VOID  TriEventInit(
    IN  PMP_ADAPTER   pAd,
    IN PMP_PORT     pPort) ;

VOID TriEventTableSetEntry(
    IN  PMP_ADAPTER   pAd, 
    IN PMP_PORT     pPort,
    OUT TRIGGER_EVENT_TAB *Tab, 
    IN PUCHAR pBssid, 
    IN HT_CAPABILITY_IE *pHtCapability,
    IN UCHAR            HtCapabilityLen,
    IN UCHAR            RegClass,
    IN UCHAR ChannelNo);


VOID BssTableSsidSort(
    IN  PMP_ADAPTER   pAd, 
    IN    PMP_PORT pPort,
    OUT BSS_TABLE *OutTab, 
    IN  CHAR Ssid[], 
    IN  UCHAR SsidLen) ;

VOID BssTableSortByRssi(
    IN PMP_ADAPTER pAd,
    IN OUT BSS_TABLE *OutTab) ;

VOID BssTableWPSAPFirst(
    IN PMP_ADAPTER pAd,
    IN PMP_PORT pPort,
    IN OUT BSS_TABLE *OutTab);

VOID BssCipherParse(
    IN OUT  PBSS_ENTRY  pBss);

VOID MacAddrRandomBssid(
    IN PMP_ADAPTER pAd, 
    OUT PUCHAR pAddr) ;

VOID MgtMacHeaderInit(
    IN  PMP_ADAPTER   pAd, 
    IN  PMP_PORT      pPort, 
    IN OUT PHEADER_802_11 pHdr80211, 
    IN UCHAR SubType, 
    IN UCHAR ToDs, 
    IN PUCHAR pDA, 
    IN PUCHAR pBssid); 


ULONG MakeOutgoingFrame(
    OUT CHAR *Buffer, 
    OUT ULONG *FrameLen, ...); 

NDIS_STATUS MlmeQueueInit(
    IN MLME_QUEUE *Queue); 

BOOLEAN MlmeEnqueue(
    IN  PMP_ADAPTER pAd,
    IN    PMP_PORT pPort,
    IN ULONG Machine, 
    IN ULONG MsgType, 
    IN ULONG MsgLen, 
    IN VOID *Msg);

VOID  BTHS_SendDisconnPhysLink(
    IN PMP_ADAPTER pAd,
    IN  PMP_PORT      pPort,
    IN PUCHAR         pMacAddr,
    IN  UCHAR         PhysHdlIdx);

BOOLEAN BTHSMlmeEnqueue(
    IN PMP_ADAPTER pAd, 
    IN ULONG Machine, 
    IN ULONG MsgType, 
    IN ULONG MsgLen, 
    IN VOID *Msg,
    IN UCHAR PhysHdl);

BOOLEAN MlmeEnqueueForRecv(
    IN  PMP_ADAPTER   pAd, 
    IN  PMP_PORT      pPort, 
    IN ULONG Wcid, 
    IN ULONG TimeStampHigh,
    IN ULONG TimeStampLow, 
    IN UCHAR Rssi0, 
    IN UCHAR Rssi1, 
    IN UCHAR Rssi2, 
    IN UCHAR AntSel,
    IN ULONG MsgLen, 
    IN VOID *Msg) ;

BOOLEAN MlmeDequeue(
    IN MLME_QUEUE *Queue, 
    OUT MLME_QUEUE_ELEM **Elem); 

BOOLEAN MlmeWaitForMlmeHandlerStop(
    IN  PMP_ADAPTER   pAd
    );

VOID    MlmeRestartStateMachine(
    IN  PMP_ADAPTER   pAd,
    IN  PMP_PORT      pPort
    );

BOOLEAN MlmeQueueEmpty(
    IN MLME_QUEUE *Queue) ;

BOOLEAN MlmeQueueFull(
    IN MLME_QUEUE *Queue) ;

VOID MlmeQueueDestroy(
    IN MLME_QUEUE *pQueue) ;

BOOLEAN MsgTypeSubst(
    IN PMP_ADAPTER  pAd,
    IN  PMP_PORT      pPort,
    IN PFRAME_802_11 pFrame, 
    OUT INT *Machine, 
    OUT INT *MsgType); 

VOID StateMachineInit(
    IN STATE_MACHINE *S, 
    IN STATE_MACHINE_FUNC Trans[], 
    IN ULONG StNr, 
    IN ULONG MsgNr, 
    IN STATE_MACHINE_FUNC DefFunc, 
    IN ULONG InitState, 
    IN ULONG Base) ;

VOID StateMachineSetAction(
    IN STATE_MACHINE *S, 
    IN ULONG St, 
    IN ULONG Msg, 
    IN STATE_MACHINE_FUNC Func) ;

VOID StateMachinePerformAction(
    IN  PMP_ADAPTER   pAd, 
    IN STATE_MACHINE *S, 
    IN MLME_QUEUE_ELEM *Elem) ;

VOID Drop(
    IN PMP_ADAPTER pAd, 
    IN MLME_QUEUE_ELEM *Elem) ;

VOID LfsrInit(
    IN PMP_ADAPTER pAd, 
    IN ULONG Seed) ;

UCHAR RandomByte(
    IN PMP_ADAPTER pAd) ;

VOID AsicSwitchChannel(
    IN  PMP_PORT    pPort, 
    IN  UCHAR           Channel,
    IN  UCHAR           CentralChannel,
    IN  UCHAR           BW,
    IN  BOOLEAN         bScan) ;

VOID
AsicSwitchChannelWorkitemCallback(
    IN PVOID Context
    );    

VOID AsicSleepThenAutoWakeup(
    IN PMP_ADAPTER pAd,   
    IN USHORT TbttNumToNextWakeUp) ;

VOID AsicForceWakeup(
    IN PMP_ADAPTER pAd);

VOID MtAsicSetMultiBssidEx(
    IN PMP_ADAPTER pAd,
    IN PUCHAR pBssid,
    IN UCHAR    BssidIndex);

VOID MtAsicSetMultiOwnMac(
    IN PMP_ADAPTER pAd, 
    IN PUCHAR addr,
    IN UCHAR    BssidIndex);

VOID AsicSetBssidWC(
    IN PMP_ADAPTER pAd, 
    IN PMP_PORT pPort, 
    IN PUCHAR pBssid) ;

VOID AsicSetCarrierDetectAction(
    IN PMP_ADAPTER    pAd,
    IN UCHAR            Action,
    IN ULONG            Period) ;

VOID MlmeUpdateBeacon(
    IN PMP_ADAPTER pAd);

VOID AsicReSyncBeaconTime(
    IN      PMP_ADAPTER   pAd,
    IN  PMP_PORT pPort);

VOID AsicEnableRDG(
    IN PMP_ADAPTER pAd) ;

VOID AsicDisableRDG(
    IN PMP_ADAPTER pAd,
    IN PMP_PORT pPort) ;

VOID AsicEnablePiggyB(
    IN PMP_ADAPTER pAd) ;

VOID AsicDisablePiggyB(
    IN PMP_ADAPTER pAd) ;

VOID AsicPauseBssSync(
    IN PMP_ADAPTER pAd);

VOID AsicResumeBssSync(
    IN PMP_ADAPTER pAd);

VOID AsicBbpTuning(
    IN PMP_ADAPTER pAd,
    IN PMP_PORT pPort);

//
// Update the Tx chain address (stream mode)
//
// Parameters
//  pAd: The adapter data structure
//  pMacAddress: The MAC address of the peer STA
//
// Return Value:
//  None
//
VOID AiscUpdateTxChainAddress(
    IN PMP_ADAPTER pAd, 
    IN PUCHAR pMacAddress);

VOID AsicCfgPAPEByStreams(
    IN PMP_ADAPTER pAd,
    IN UCHAR CurrMCS,
    IN UCHAR    NextMCS
    );

VOID AsicUpdateWCIDAttribute(
    IN PMP_ADAPTER pAd,
    IN PMP_PORT   pPort,
    IN USHORT       WCID,
    IN UCHAR        BssIndex,
    IN UCHAR        CipherAlg,
    IN BOOLEAN      bUsePairewiseKeyTable);

VOID AsicUpdateWCIDIVEIV(
    IN PMP_ADAPTER pAd,
    IN USHORT       WCID,
    IN ULONG        uIV,
    IN ULONG        uEIV);

#if 0
VOID AsicUpdateBASession(
    IN PMP_ADAPTER pAd, 
    IN UCHAR wcid, 
    IN UCHAR tid, 
    IN UCHAR basize, 
    IN BOOLEAN isAdd, 
    IN INT ses_type);

VOID AsicSetBssid(
    IN PMP_ADAPTER pAd, 
     IN  PUCHAR pBssid) ;

BOOLEAN AsicSetTxStream(
    IN PMP_ADAPTER pAd, 
    IN UCHAR opmode, 
    IN BOOLEAN up
    );

BOOLEAN AsicSetRxStream(
    IN PMP_ADAPTER pAd, 
    IN ULONG rx_path
    );

BOOLEAN AsicSetBW(
    IN PMP_ADAPTER pAd, 
    IN ULONG bw
    );

#define ASIC_MAC_TX     1
#define ASIC_MAC_RX     2
#define ASIC_MAC_TXRX   3
BOOLEAN AsicSetMacTxRx(
    IN PMP_ADAPTER pAd, 
    IN ULONG txrx, 
    IN BOOLEAN enable);

BOOLEAN AsicSetDevMac(
    IN PMP_ADAPTER pAd, 
    IN PUCHAR addr);

VOID AsicUpdateRxWCIDTable(
    IN PMP_ADAPTER pAd,
    IN USHORT       WCID,
    IN PUCHAR        pAddr);

#endif

BOOLEAN BBPSetBW(
    IN PMP_ADAPTER pAd, 
    IN ULONG bw
    );


void MT7603InitMAC(
    IN PMP_ADAPTER pAd
    );

BOOLEAN WTBLInit(
    IN PMP_ADAPTER pAd
    );

VOID DumpWTBLBaseInfo(
    IN PMP_ADAPTER pAd
    );
#if 0
BOOLEAN WTBLGetEntry234(
    IN MP_ADAPTER *pAd, 
    IN USHORT WCID, 
    IN PWLAN_TABLE_ENTRY pEntry);
#endif

VOID CmdProcAddRemoveKey(
    IN  PMP_ADAPTER     pAd,
    IN  UCHAR           AddRemove,
    IN  UCHAR           BssIdx,
    IN  UCHAR           key_idx,
    IN  UCHAR           Wcid,
    IN  UCHAR           KeyTabFlag,
    IN  PCIPHER_KEY     pCipherKey,
    IN  PUCHAR          PeerAddr);

//UCHAR get_nss_by_mcs(UCHAR phy_mode, UCHAR mcs);
USHORT tx_rate_to_tmi_rate(UCHAR mode, UCHAR mcs, UCHAR nss, BOOLEAN stbc, UCHAR preamble);
//VOID asic_mcs_lut_update(MP_ADAPTER *pAd, MAC_TABLE_ENTRY *pEntry);

//VOID dump_wtbl_entry(MP_ADAPTER *pAd, WLAN_TABLE_ENTRY *ent);

VOID hex_dump(char *str, unsigned char *pSrcBufVA, unsigned int SrcBufLen);
//VOID DumpWTBL1Info(PMP_ADAPTER pAd, PWTBL_1_STRUC tb);
//VOID DumpWTBL2Info(PMP_ADAPTER pAd, PWTBL_2_STRUC tb);
//VOID DumpWTBL3Info(PMP_ADAPTER pAd, PWTBL_3_STRUC tb);
//VOID DumpWTBL4Info(PMP_ADAPTER pAd, PWTBL_4_STRUC tb);
VOID DumpWTBLInfo(PMP_ADAPTER pAd, UINT32 wtbl_idx, PVOID wtblentry);
VOID show_mib_proc(MP_ADAPTER *pAd, PMIB_DATA pMibData);
INT show_mib_proc2(MP_ADAPTER *pAd);
int get_low_mid_hi_index(UINT8 channel);
void mt7603_tx_pwr_gain(MP_ADAPTER *pAd, UINT8 channel);

VOID AsicAddWAPIKey(
    IN PMP_ADAPTER pAd, 
    IN USHORT WCID, 
    IN PUCHAR pWPIEK, 
    IN UCHAR EKKeyLen, 
    IN PUCHAR pWPICK, 
    IN UCHAR CKKeyLen, 
    IN PUCHAR pPN, 
    IN UCHAR PNLen, 
    IN UCHAR KeyID, 
    IN BOOLEAN bPNIncrement, 
    IN BOOLEAN bUsePairewiseKeyTable);
    
VOID AsicAddKeyEntry(
    IN PMP_ADAPTER pAd,
    IN PMP_PORT   pPort,
    IN UCHAR       WCID,
    IN UCHAR        BssIndex,
    IN UCHAR        KeyIdx,
    IN PCIPHER_KEY  pCipherKey, 
    IN BOOLEAN      bUsePairewiseKeyTable,
    IN BOOLEAN      bTxKey);

VOID AsicAddSharedKeyEntry(
    IN PMP_ADAPTER pAd,
    IN PMP_PORT   pPort,
    IN UCHAR         BssIndex,
    IN UCHAR         KeyIdx,
    IN UCHAR         CipherAlg,
    IN PUCHAR        pKey,
    IN PUCHAR        pTxMic,
    IN PUCHAR        pRxMic);

VOID AsicRemoveSharedKeyMode(
    IN PMP_ADAPTER pAd,
    IN UCHAR         BssIndex,
    IN UCHAR         KeyIdx);

VOID AsicRemoveKeyEntry(
    IN PMP_ADAPTER pAd,
    IN USHORT        WCID,
    IN UCHAR        KeyIdx,
    IN BOOLEAN      bUsePairewiseKeyTable    
    );

VOID AsicRemoveSharedKeyEntry(
    IN PMP_ADAPTER pAd,
    IN UCHAR         BssIndex,
    IN UCHAR         KeyIdx);

VOID AsicRemovePairwiseKeyEntry(
    IN PMP_ADAPTER pAd,
    IN UCHAR         KeyIdx);

BOOLEAN AsicSendCommanToMcu(
    IN PMP_ADAPTER pAd,
    IN UCHAR         Command,
    IN UCHAR         Token,
    IN UCHAR         Arg0,
    IN UCHAR         Arg1);

//
// Initiate the VCO calibration
//
// Parameters
//  pAd: The adapter data structure
//
// Return Value:
//  None
//

VOID FullVCOCalibration(
    IN PMP_ADAPTER pAd);

VOID IncrementalVCOCalibration(
    IN PMP_ADAPTER pAd);

VOID AsicVCOCalibration(
    IN PMP_ADAPTER pAd);

VOID    RTMPCheckRates(
    IN      PMP_ADAPTER   pAd,
    IN OUT  UCHAR           SupRate[],
    IN OUT  UCHAR           *SupRateLen);

BOOLEAN RTMPCheckChannel(
    IN PMP_ADAPTER pAd,
    IN UCHAR        CentralChannel,
    IN UCHAR        Channel);

BOOLEAN     RTMPCheckHt(
    IN      PMP_ADAPTER   pAd,
    IN      PMP_PORT      pPort,
    IN      UCHAR   Wcid,
    IN OUT PHT_CAPABILITY_IE pHtCapability, 
    IN OUT PADD_HT_INFO_IE pAddHtInfo, 
    IN BOOLEAN bVhtCapable, 
    IN PVHT_CAP_IE pVhtCapability);

BOOLEAN     RTMPCheckVht(
    IN      PMP_ADAPTER  pAd,
    IN      PMP_PORT     pPort,
    IN      UCHAR          Wcid,
    IN OUT  VHT_CAP_IE     *pVhtCapability,
    IN OUT  VHT_OP_IE      *pVhtOperation);

VOID AsicSetRxAnt(
    IN PMP_ADAPTER    pAd,
    IN UCHAR            Ant);

VOID AsicEvaluateRxAnt(
    IN PMP_ADAPTER    pAd,
    IN PMP_PORT       pPort);

VOID AsicRxAntEvalTimeoutTimerCallback(
    IN PVOID SystemSpecific1, 
    IN PVOID FunctionContext, 
    IN PVOID SystemSpecific2, 
    IN PVOID SystemSpecific3); 

VOID AsicRxAntEvalAction(
    IN PMP_ADAPTER pAd);

VOID RadarDetectionStart(
    IN PMP_ADAPTER    pAd);

BOOLEAN RadarDetectionStop(
    IN PMP_ADAPTER    pAd);

BOOLEAN RadarChannelCheck(
    IN PMP_ADAPTER    pAd,
    IN UCHAR            Ch);

VOID MlmeThread(
    IN PMP_ADAPTER    pAd
    );

// P2pMs Thread functions
KSTART_ROUTINE  P2pMsThread;
VOID P2pMsThread(
    IN PVOID Context);

VOID    MlmeFreeMemory(
    IN PMP_ADAPTER pAd,
    IN PVOID         AllocVa);

NDIS_STATUS MlmeAllocateMemoryEx(
    IN PMP_ADAPTER pAd,
    IN ULONG         Location,    
    OUT PVOID        *AllocVa);

NDIS_STATUS MlmeInitMemoryHandler(
    IN PMP_ADAPTER pAd,
    IN UINT  Number,
    IN UINT  Size);

VOID MlmeFreeMemoryHandler(
    IN PMP_ADAPTER pAd);

VOID RTMPUpdateMlmeRate(
    IN PMP_ADAPTER    pAd,
    UCHAR   Channel);

CHAR RTMPMaxRssi(
    IN PMP_ADAPTER    pAd,
    IN CHAR             Rssi0,
    IN CHAR             Rssi1,
    IN CHAR             Rssi2);

VOID RTMPSetAGCInitValue(
    IN PMP_ADAPTER    pAd,
    IN UCHAR            BandWidth);

VOID AsicSendBA(
    IN PMP_ADAPTER pAd,
    IN PMP_PORT   pPort,
    IN UCHAR        Channel,
    IN ULONG        Wcid);

VOID AsicSendNullFrame(
    IN PMP_ADAPTER pAd,
    IN PMP_PORT   pPort,
    IN UCHAR        Channel,
    IN UCHAR        PwrMgmt); 

VOID CarrierDetectTimeoutTimerCallback(
    IN PVOID SystemSpecific1, 
    IN PVOID FunctionContext, 
    IN PVOID SystemSpecific2, 
    IN PVOID SystemSpecific3) ;

VOID CarrierDetectTimeoutAction(
    IN PMP_ADAPTER pAd) ;

//
// The verification of the Tx power per rate
//
// Parameters
//  pAd: The adapter data structure
//  pTxPwr: Point to a Tx power per rate
//
// Return Value
//  None
//
VOID RTMPVerifyTxPwrPerRateExt(
    IN PMP_ADAPTER pAd, 
    IN OUT PUCHAR pTxPwr);

//
// Read the Tx power (per data rate) based on the extended EEPROM format
//
// Parameters
//  pAd: The adapter data structure
//
// Return Value
//  None
//
VOID    RTMPReadTxPwrPerRate(
    IN  PMP_ADAPTER   pAd);

VOID CpoyRegistryToDriverDefaultSetting (
       IN  PMP_ADAPTER       pAd
       );

VOID UpdatePortDefaultSetting (
       IN  PMP_PORT       pPort
       );

NDIS_STATUS NdisInitNICReadRegParameters(
    IN  PMP_ADAPTER       pAd
    );

NDIS_STATUS RTUSBWriteHWMACAddress(
    IN  PMP_ADAPTER       pAd);

VOID    NICReadEEPROMParameters(
    IN  PMP_ADAPTER   pAd);

//
// calibration
//
VOID NicReadEepromForIqCompensation(
    IN  PMP_ADAPTER   pAd);

//
// Initialize e-Fuse configuration
//
// Parameters
//  pAd: The adapter data structure
//
// Return Value
//  None
//
VOID InitEFuseConfig(
    IN PMP_ADAPTER pAd);

VOID    NICInitAsicFromEEPROM(
    IN  PMP_ADAPTER   pAd);

NDIS_STATUS NICInitializeAsic(
    IN  PMP_ADAPTER   pAd);

//
// Post-process the BBP registers based on the chip model
//
// Parameters
//  pAd: The adapter data structure
//
// Return Value
//  None
//
VOID PostBBPInitialization(
    IN PMP_ADAPTER pAd);

//
// Post-process the RF registers based on the chip model
//
// Parameters
//  pAd: The adapter data structure
//
// Return Value
//  None
//
VOID PostRFInitialization(
    IN PMP_ADAPTER pAd);

VOID NICUpdateRawCounters(
    IN PMP_ADAPTER pAd);

#if DBG

//
// Load the external EEPROM image (RT3xUsbEeprom.bin)
//
// Parameters
//  pAd: The adapter data structure
//
// Return Value:
//  NDIS_STATUS_SUCCESS: The image is valid
//  NDIS_STATUS_FAILURE: Cannot open the external image
//
NDIS_STATUS LoadExternalEEPROMImage(
    IN PMP_ADAPTER pAd);

#endif // DBG

VOID InitAPPseudoSTAConfig(
    IN PMP_ADAPTER pAd);

#ifdef MULTI_CHANNEL_SUPPORT
// Change Beacon parameters
void MlmeSyncPeerBeaconChangeElementValue(
    IN VOID *Msg, 
    IN ULONG MsgLen, 
    IN ULONG ElemId,
    IN UCHAR    Value);
#endif /*MULTI_CHANNEL_SUPPORT*/

UCHAR BtoH(char ch);

void AtoH(char * src, UCHAR * dest, int destlen);

VOID RTMPEnableRxTx(
    IN PMP_ADAPTER    pAd);
    
VOID    RTMPPrepareCTSFrame(
    IN  PMP_ADAPTER   pAd,
    IN  PUCHAR          pDA,
    IN  ULONG           Duration,
    IN  UCHAR           RTSRate,
    IN  UCHAR           CTSIdx,
    IN  UCHAR           FrameGap);

BOOLEAN S345AsicRadioOff6x9x(
    IN PMP_ADAPTER pAd);


VOID SetLEDByStatus(
    IN PMP_ADAPTER    pAd, 
    IN UCHAR            Status);

VOID ParseRxVGroup(
    IN  PMP_ADAPTER pAd, 
    IN  UINT32 Type, 
    IN  struct rx_signal_info *rx_signal, 
    IN  UCHAR *Data);

////
// prototype in rtmp_info.c
//
VOID    N6PlatformIndicateScanStatus(
    IN  PMP_ADAPTER   pAd,
    IN  PMP_PORT      pPort,
    IN  NDIS_STATUS     ScanConfirmStatus,
    IN  BOOLEAN         ClearBssScanFlag,
    IN  BOOLEAN         SetSyncIdle);

NDIS_STATUS
MpScanRequest(
    IN PMP_ADAPTER pAd ,
    IN  PMP_PORT pPort ,
    IN PVOID InformationBuffer,
    IN ULONG InformationBufferLength,
    OUT PULONG BytesRead,
    OUT PULONG BytesNeeded
    );

NDIS_STATUS
RTMPInfoResetRequest(
    IN  PMP_ADAPTER pAd,
    IN  PMP_PORT pPort,
    IN OUT PVOID InformationBuffer,
    IN  ULONG InputBufferLength,
    IN  ULONG OutputBufferLength,
    OUT PULONG BytesRead,
    OUT PULONG BytesWritten,
    OUT PULONG BytesNeeded
    );

NDIS_STATUS
Hw11QueryDot11Statistics(
    IN  PMP_ADAPTER pAd,
    IN  PDOT11_STATISTICS pDot11Stats,
    IN  ULONG InformationBufferLength,
    OUT PULONG BytesWritten,
    OUT PULONG BytesNeeded
    );

NDIS_STATUS
HwQueryStatistics(   IN PMP_ADAPTER pAd,
    IN OUT PVOID InformationBuffer,
    IN ULONG InformationBufferLength,
    IN OUT PULONG BytesWritten,
    IN OUT PULONG BytesNeeded);
    
NDIS_STATUS
DisconnectRequest(
    IN PMP_ADAPTER pAd,
    IN PMP_PORT   pPort,
    IN OUT PULONG BytesRead,
    IN OUT PULONG BytesNeeded, 
    IN BOOLEAN bHwRadioOff
    );

VOID RTMPCLI(
    IN  PMP_ADAPTER pAd,
    IN  PRT_802_11_HARDWARE_REGISTER pHardwareRegister);

////
// prototype in rtmp_power.c
//
NDIS_STATUS N6UsbSetPowerbyPort(
    IN PMP_PORT               pPort ,
    IN NDIS_DEVICE_POWER_STATE  NewDeviceState);

//
// Initialize the transmit beamforming control
//
// Parameters
//  pAd: The adapter data structure
//
// Return Value
//  None
//
VOID InitTxBfCtrl(
    IN PMP_ADAPTER pAd);

//
// Update the TxBf capabilities field
//
// Parameters
//  pAd: The adapter data structure
//
// Return Value
//  None
//
VOID UpdateTxBfCapabilitiesField(
    IN PMP_ADAPTER pAd);

//
// Enable/disable the explicit TxBf
//
// Parameters
//  pAd: The adapter data structure
//  pInfoBuf: Pointer to a caller-supplied variable in which points to the information buffer
//  InfoBufLength: The length of the information buffer
//
// Return Value
//  NDIS_STATUS_INVALID_LENGTH: Incorrect length of the information buffer
//  NDIS_STATUS_INVALID_DATA: Incorrect parameter
//
NDIS_STATUS TxBfOidSetETxEn(
    IN PMP_ADAPTER pAd, 
    IN PVOID pInfoBuf, 
    IN ULONG InfoBufLength);

//
// Set the explicit TxBf timeout value
//
// Parameters
//  pAd: The adapter data structure
//  pInfoBuf: Pointer to a caller-supplied variable in which points to the information buffer
//  InfoBufLength: The length of the information buffer
//
// Return Value
//  NDIS_STATUS_SUCCESS: Success
//  NDIS_STATUS_INVALID_LENGTH: Incorrect length of the information buffer
//  NDIS_STATUS_INVALID_DATA: The explicit TxBf timeout value is out of range.
//
NDIS_STATUS TxBfOidSetETxBfTimeout(
    IN PMP_ADAPTER pAd, 
    IN PVOID pInfoBuf, 
    IN ULONG InfoBufLength);

//
// Select the non-compressed beamforming feedback matrix or the compressed beamforming feedback matrix
//
// Parameters
//  pAd: The adapter data structure
//  pInfoBuf: Pointer to a caller-supplied variable in which points to the information buffer
//  InfoBufLength: The length of the information buffer
//
// Return Value
//  NDIS_STATUS_SUCCESS: Success
//  NDIS_STATUS_INVALID_LENGTH: Incorrect length of the information buffer
//  NDIS_STATUS_INVALID_DATA: The explicit TxBf timeout value is out of range.
//
NDIS_STATUS TxBfOidSetETxBfNonCompressedFeedback(
    IN PMP_ADAPTER pAd, 
    IN PVOID pInfoBuf, 
    IN ULONG InfoBufLength);

//
// Send a +HTC MPDU with the NDP Announcement set to 1
//
// Parameters
//  pAd: The adapter data structure
//  QueIdx: Queue index
//  pPacket: Pointer to a caller-supplied variable in which points to the packet
//  Length: Packet length
//  pNdpPhyCfg: Pointer to a caller-supplied variable in which points to the HTTRANSMIT_SETTING
//
// Return Value
//  None
//
NDIS_STATUS TxBfNDPRequest(
    IN PMP_ADAPTER pAd, 
    IN PVOID pBuffer, 
    IN UINT Length, 
    //IN PHTTRANSMIT_SETTING pHTTxRate
    IN PPHY_CFG pNdpPhyCfg);

//
// Send a +HTC MPDU with the NDP Announcement set to 1
//
// Parameters
//  pAd: The adapter data structure
//  pNdpPhyCfg: Pointer to a caller-supplied variable in which points to the HTTRANSMIT_SETTING
//  bFromAP: Send from an AP
//  pAddress1: Pointer to a caller-supplied variable in which points to the Address 1
//  pAddress2: Pointer to a caller-supplied variable in which points to the Address 2
//  pAddress3: Pointer to a caller-supplied variable in which points to the Address 3
//
// Return Value
//  None
//
VOID    TxBfSendNDPAnnouncement(
    IN PMP_ADAPTER pAd, 
    IN PMP_PORT       pPort,
    //IN PHTTRANSMIT_SETTING pHTTxRate, 
    IN PPHY_CFG pNdpPhyCfg, 
    IN BOOLEAN bFromAP, 
    IN PUCHAR pAddress1, 
    IN PUCHAR pAddress2, 
    IN PUCHAR pAddress3);

//
// The generation of +HTC MPDU(s) with the NDP Announcement set to 1
//
// Parameters
//  pAd: The adapter data structure
//
// Return Value
//  None
//
VOID    TxBfNDPAnnouncementGeneration(
    IN PMP_ADAPTER pAd,
    IN PMP_PORT       pPort);

//
// Determine that the packets should apply the beamforming matrix of the implicit/explicit TxBf, or not
//
// Parameters
//  pAd: The adapter data structure
//
// Return Value
//  None
//
// Note:
//  a) The NIC preferes the explicit TxBf.
//
VOID    TxBfApplyMatrixToPacket(
    IN PMP_ADAPTER pAd);

////
// prototype in rtmp_tkip.c
//
VOID    RTMPTkipAppendByte( 
    IN  PTKIP_KEY_INFO  pTkip,  
    IN  UCHAR           uChar);
VOID    RTMPTkipAppendByte( 
    IN  PTKIP_KEY_INFO  pTkip,  
    IN  UCHAR           uChar);

VOID    RTMPTkipAppend( 
    IN  PTKIP_KEY_INFO  pTkip,  
    IN  PUCHAR          pSrc,
    IN  UINT            nBytes);

VOID    RTMPTkipGetMIC( 
    IN  PTKIP_KEY_INFO  pTkip);

VOID    RTMPInitTkipEngine(
    IN  PMP_ADAPTER   pAd,    
    IN  PUCHAR          pKey,
    IN  UCHAR           KeyId,
    IN  PUCHAR          pTA,
    IN  PUCHAR          pMICKey,
    IN  PUCHAR          pTSC,
    OUT PULONG          pIV16,
    OUT PULONG          pIV32);

VOID    RTMPInitMICEngine(
    IN  PMP_ADAPTER   pAd,    
    IN  PUCHAR          pKey,
    IN  PUCHAR          pDA,
    IN  PUCHAR          pSA,
    IN  UCHAR           UserPriority,
    IN  PUCHAR          pMICKey);

BOOLEAN RTMPTkipCompareMICValue(
    IN  PMP_ADAPTER   pAd,        
    IN  PUCHAR          pSrc,
    IN  PUCHAR          pDA,
    IN  PUCHAR          pSA,
    IN  PUCHAR          pMICKey,
    IN  UINT            Len,
    IN  UCHAR           UP);

VOID    
RTMPCalculateMICValue(
    IN  PMP_ADAPTER   pAd,
    IN  PUCHAR          pDA,
    IN  PUCHAR          pSA,
    IN  PMT_XMIT_CTRL_UNIT pXcu,
    IN  PCIPHER_KEY     pKey,
    IN  UCHAR           UserPriority
    );

BOOLEAN RTMPTkipCompareMICValueWithLLC(
    IN  PMP_ADAPTER   pAd,
    IN  PUCHAR          pLLC,
    IN  PUCHAR          pSrc,
    IN  PUCHAR          pDA,
    IN  PUCHAR          pSA,
    IN  PUCHAR          pMICKey,
    IN  UINT            Len);

VOID RTMPTkipMixKey(
    UCHAR   *key, 
    UCHAR   *ta, 
    ULONG   pnl, /* Least significant 16 bits of PN */
    ULONG   pnh, /* Most significant 32 bits of PN */ 
    UCHAR   *rc4key, 
    UINT    *p1k);

BOOLEAN RTMPSoftEncryptTKIP(    
    IN PMP_ADAPTER pAd,
    IN PMP_PORT pPort,
    IN PUCHAR   pDot11Hdr,
    IN PUCHAR   pData,
    IN ULONG    DataByteCnt, 
    IN UCHAR    UserPriority,
    IN UCHAR    KeyID,
    IN PCIPHER_KEY  pWpaKey,
    IN BOOLEAN  bMgmt);

BOOLEAN RTMPSoftDecryptTKIP(
    IN PMP_ADAPTER    pAd,
    IN PMP_PORT       pPort,
    IN PUCHAR   pDot11Hdr,
    IN PUCHAR           pData,
    IN ULONG            DataByteCnt, 
    IN UCHAR            UserPriority,
    IN PCIPHER_KEY  pWpaKey,
    IN BOOLEAN  bMgmt);

BOOLEAN RTMPSoftEncryptAES(
    IN PMP_ADAPTER pAd,
    IN PMP_PORT pPort,
    IN PUCHAR   pDot11Hdr,
    IN PUCHAR   pData,
    IN ULONG    DataByteCnt,
    IN UCHAR    KeyID,
    IN PCIPHER_KEY  pWpaKey,
    IN BOOLEAN  bMgmt);

BOOLEAN RTMPSoftDecryptAES(
    IN PMP_ADAPTER    pAd,
    IN PUCHAR   pDot11Hdr,
    IN PUCHAR           pData,
    IN ULONG            DataByteCnt, 
    IN PCIPHER_KEY      pWpaKey,
    IN BOOLEAN  bMgmt);

BOOLEAN RTMPSoftEncryptMgmtAES(
    IN PMP_ADAPTER pAd,
    IN PMP_PORT   pPort,
    IN PUCHAR   pDot11Hdr,
    IN PUCHAR   pData,
    IN ULONG    DataByteCnt,
    IN UCHAR    KeyID,
    IN PCIPHER_KEY  pWpaKey);

BOOLEAN RTMPSoftDecryptMgmtAES(
    IN PMP_ADAPTER pAd,
    IN PMP_PORT   pPort,
    IN PUCHAR   pDot11Hdr,
    IN PUCHAR   pData,
    IN ULONG    DataByteCnt, 
    IN PCIPHER_KEY  pWpaKey);

////
// Private routines in rtmp_wep.c
//
VOID    RTMPInitWepEngine(
    IN  PMP_ADAPTER   pAd,    
    IN  PUCHAR          pKey,
    IN  UCHAR           KeyId,
    IN  UCHAR           KeyLen, 
    IN  PUCHAR          pDest);

VOID    RTMPEncryptData(
    IN  PMP_ADAPTER   pAd,    
    IN  PUCHAR          pSrc,
    IN  PUCHAR          pDest,
    IN  UINT            Len);

BOOLEAN
RTMPPortDecryptData(
    IN  PMP_PORT      pPort,
    IN  PUCHAR          pSrc,
    IN  ULONG           Len);

VOID    ARCFOUR_INIT(
    IN  PARCFOURCONTEXT Ctx,
    IN  PUCHAR          pKey,
    IN  UINT            KeyLen);

UCHAR   ARCFOUR_BYTE(
    IN  PARCFOURCONTEXT     Ctx);

VOID    ARCFOUR_DECRYPT(
    IN  PARCFOURCONTEXT Ctx,
    IN  PUCHAR          pDest, 
    IN  PUCHAR          pSrc,
    IN  UINT            Len);

VOID    ARCFOUR_ENCRYPT(
    IN  PARCFOURCONTEXT Ctx,
    IN  PUCHAR          pDest,
    IN  PUCHAR          pSrc,
    IN  UINT            Len);

VOID    WPAARCFOUR_ENCRYPT(
    IN  PARCFOURCONTEXT Ctx,
    IN  PUCHAR          pDest,
    IN  PUCHAR          pSrc,
    IN  UINT            Len);

ULONG   RTMP_CALC_FCS32(
    IN  ULONG   Fcs,
    IN  PUCHAR  Cp,
    IN  INT     Len);

VOID    RTMPSetICV(
    IN  PMP_ADAPTER   pAd,
    IN  PUCHAR          pDest);

////
// prototype in rtusb_bulk.c
//

VOID    N6USBInitTxDesc(
    IN  PMP_ADAPTER           pAd,
    IN  PTX_CONTEXT             pTxContext,
    IN  UCHAR                   BulkOutPipeId,
    IN  PFN_WDF_REQUEST_COMPLETION_ROUTINE  Func);

NTSTATUS    N6USBInitHTTxDesc(
    IN  PMP_ADAPTER           pAd,
    IN  PHT_TX_CONTEXT          pTxContext,
    IN  WDFREQUEST                  writeRequest,
    IN  PVOID                   pWriteContext,
    IN  UCHAR                   BulkOutPipeId,
    IN  ULONG                   NextBulkOutPosition,        
    IN  ULONG                   BulkOutSize,
    IN  UCHAR                   IrpIndex,
    IN   PFN_WDF_REQUEST_COMPLETION_ROUTINE     Func);

ULONG   N6USBComposeUnit(
    IN  PMP_ADAPTER   pAd,
    IN  UCHAR           BulkOutPipeId,
    IN OUT    ULONG     *pAMSDUExist,
    IN OUT    ULONG     *pTmpBulkPos,
    IN OUT    ULONG     *pTotalBulkSize,
    IN OUT    ULONG     *pTmpBulkEndPos);

VOID    N6USBBulkOutDataPacket(
    IN  PMP_ADAPTER   pAd,
    IN  UCHAR           BulkOutPipeId,
    IN  UCHAR           Index);

EVT_WDF_REQUEST_COMPLETION_ROUTINE N6USBBulkOutDataPacketComplete;

VOID    N6USBBulkOutNullFrame(
    IN  PMP_ADAPTER   pAd);

EVT_WDF_REQUEST_COMPLETION_ROUTINE N6USBBulkOutNullFrameComplete;
EVT_WDF_REQUEST_COMPLETION_ROUTINE MT7603BulkOutNullFrameComplete;

#if 0
VOID    N6USBBulkOutMLMEPacket(
    IN  PMP_ADAPTER   pAd,
    IN  UCHAR           Index);
#else
VOID    N6USBBulkOutMLMEPacket(
    IN  PMP_ADAPTER   pAd);
#endif

EVT_WDF_REQUEST_COMPLETION_ROUTINE N6USBBulkOutMLMEPacketComplete;


VOID    N6USBBulkOutPsPoll(
    IN  PMP_ADAPTER   pAd);

EVT_WDF_REQUEST_COMPLETION_ROUTINE N6USBBulkOutPsPollComplete;

VOID N6USBBulkOutPsPollComplete(
        IN WDFREQUEST  Request,
    IN WDFIOTARGET  Target,
    IN PWDF_REQUEST_COMPLETION_PARAMS  CompletionParams,
    IN WDFCONTEXT  Context);

VOID    N6USBBulkOutFwPkt(
    IN  PMP_ADAPTER   pAd);

EVT_WDF_REQUEST_COMPLETION_ROUTINE N6USBBulkOutFwPktComplete;

VOID N6USBBulkOutFwPktComplete(
    IN WDFREQUEST  Request,
    IN WDFIOTARGET  Target,
    IN PWDF_REQUEST_COMPLETION_PARAMS  CompletionParams,
    IN WDFCONTEXT  Context);

VOID    N6USBBulkOutPktCmd(
    IN  PMP_ADAPTER   pAd);

EVT_WDF_REQUEST_COMPLETION_ROUTINE N6USBBulkOutPktCmdComplete;

VOID N6USBBulkOutPktCmdComplete(
    IN WDFREQUEST  Request,
    IN WDFIOTARGET  Target,
    IN PWDF_REQUEST_COMPLETION_PARAMS  CompletionParams,
    IN WDFCONTEXT  Context);

VOID    N6USBBulkReceive(
    IN  PMP_ADAPTER   pAd);

EVT_WDF_REQUEST_COMPLETION_ROUTINE N6USBBulkRxComplete;

VOID    N6USBBulkRxComplete(
    IN WDFREQUEST  Request,
    IN WDFIOTARGET  Target,
    IN PWDF_REQUEST_COMPLETION_PARAMS  CompletionParams,
    IN WDFCONTEXT  Context);

VOID    N6USBBulkReceiveCMD(
    IN  PMP_ADAPTER   pAd);

VOID    N6USBBulkRxCMDComplete(
    IN WDFREQUEST  Request,
    IN WDFIOTARGET  Target,
    IN PWDF_REQUEST_COMPLETION_PARAMS  CompletionParams,
    IN WDFCONTEXT  Context);

VOID    N6USBKickBulkOut(
    IN  PMP_ADAPTER pAd);

VOID
N6USBXmitSendMlmeCmdPkt(
    IN  PMP_ADAPTER       pAd,
    IN  MLME_PKT_TYPE    PktType
    );    

VOID    N6USBCleanUpDataBulkOutQueue(
    IN  PMP_ADAPTER   pAd);

VOID
N6USBWaitPendingRequest(
    IN PMP_ADAPTER pAd
    );

VOID PrintDataType(
    IN  PHEADER_802_11  pHeader,
    IN  PRXINFO_STRUC   pRxInfo);
    
#if 0   
VOID
 NdisCommonReportEthFrameToLLC_LIST(
    IN  PMP_ADAPTER  pAd,
    IN  PMP_PORT      pPort,
    IN  PUCHAR  *p80211DataFrameList,
    IN  USHORT  *us80211FreamSizeList,
    IN  UCHAR   *UserPriorityList,
    IN  UCHAR    MSDUNumber);
 #endif
   
////
// prototype in rtusb_io.c
//
VOID InitEFuseConfig(
    IN PMP_ADAPTER pAd);

NTSTATUS    RTUSBFirmwareOpmode(
    IN  PMP_ADAPTER   pAd,
    OUT PULONG          pValue);

static UINT32 mtusb_physical_addr_map(
    UINT32 addr);

NTSTATUS HW_IO_READ32(
    PMP_ADAPTER pAd, 
    UINT32 Offset, 
    UINT32 *pValue);

NTSTATUS HW_IO_WRITE32(
    PMP_ADAPTER pAd, 
    UINT32 Offset, 
    UINT32 Value);


NTSTATUS EFUSE_IO_READ128(
        PMP_ADAPTER pAd,
        UINT32 Offset,
        UINT8 *pValue);
        
NTSTATUS EFUSE_IO_WRITE128(
        PMP_ADAPTER pAd,
        UINT32 Offset,
        UINT8 *pValue);

NTSTATUS    USBVendorRequest(
    IN  PMP_ADAPTER   pAd,
    IN  ULONG           Direction,
    IN  UCHAR           Request,
    IN  USHORT          Value,
    IN  USHORT          Index,
    IN  PVOID           TransferBuffer,
    IN  ULONG           TransferBufferLength);

NTSTATUS    RTUSBMultiRead(
    IN  PMP_ADAPTER   pAd,
    IN  USHORT          Offset,
    OUT PUCHAR          pData,
    IN  USHORT          length);

NTSTATUS    RTUSBSingleWrite(
    IN  PMP_ADAPTER   pAd,
    IN  USHORT          Offset,
    IN  USHORT          Value);

NTSTATUS    RTUSBMultiWrite(
    IN  PMP_ADAPTER   pAd,
    IN  USHORT          Offset,
    IN  PUCHAR          pData,
    IN  USHORT          length);

NTSTATUS    RTUSBReadMACRegister(
    IN  PMP_ADAPTER   pAd,
    IN  USHORT          Offset,
    OUT PULONG          pValue);

NTSTATUS    RTUSBWriteMACRegister(
    IN  PMP_ADAPTER   pAd,
    IN  USHORT          Offset,
    IN  ULONG           Value);

NTSTATUS    RTUSBReadBBPRegister(
    IN  PMP_ADAPTER   pAd,
    IN  UCHAR           Id,
    IN  PUCHAR          pValue);

NTSTATUS    RTUSBWriteBBPRegister(
    IN  PMP_ADAPTER   pAd,
    IN  UCHAR           Id,
    IN  UCHAR           Value);

NTSTATUS    RTUSBWriteRFRegister(
    IN  PMP_ADAPTER   pAd,
    IN  ULONG           Value);

NTSTATUS    RT30xxWriteRFRegister(
    IN  PMP_ADAPTER   pAd,
    IN  UCHAR           RegID,
    IN  ULONG           Value);

NTSTATUS    RT30xxReadRFRegister(
    IN  PMP_ADAPTER   pAd,
    IN  UCHAR           RegID,
    IN  PUCHAR          pValue);

VOID eFusePhysicalReadRegisters( 
    IN  PMP_ADAPTER   pAd, 
    IN  USHORT Offset, 
    IN  USHORT Length, 
    OUT USHORT* pData);

UCHAR eFuseReadRegisters(
    IN  PMP_ADAPTER   pAd, 
    IN  USHORT Offset, 
    IN  USHORT Length, 
    OUT USHORT* pData);

VOID eFuseReadPhysical( 
    IN  PMP_ADAPTER   pAd, 
    IN  PUSHORT lpInBuffer,
    IN  ULONG nInBufferSize,
    OUT PUSHORT lpOutBuffer,
    IN  ULONG nOutBufferSize);

VOID eFusePhysicalWriteRegisters(
    IN  PMP_ADAPTER   pAd,    
    IN  USHORT Offset, 
    IN  USHORT Length, 
    OUT USHORT* pData);

VOID eFuseWritePhysical( 
    IN  PMP_ADAPTER   pAd,    
    PUSHORT lpInBuffer,
    ULONG nInBufferSize,
    PUCHAR lpOutBuffer, 
    ULONG nOutBufferSize);

NTSTATUS eFuseWriteRegisters(
    IN  PMP_ADAPTER   pAd,
    IN  USHORT Offset, 
    IN  USHORT Length, 
    IN  USHORT* pData);

VOID eFuseGetFreeBlockCount(  
    IN  PMP_ADAPTER   pAd,
    IN  PUCHAR          pFreeBlock);

NTSTATUS eFuseWrite(  
    IN  PMP_ADAPTER   pAd,
    IN  USHORT          Offset,
    IN  PUCHAR          pData,
    IN  USHORT          length);

NTSTATUS    RTUSBReadEEPROM(
    IN  PMP_ADAPTER   pAd,
    IN  USHORT          Offset,
    OUT PUCHAR          pData,
    IN  USHORT          length);

NTSTATUS    RTUSBWriteEEPROM(
    IN  PMP_ADAPTER   pAd,
    IN  USHORT          Offset,
    IN  PUCHAR          pData,
    IN  USHORT          length);

VOID
FreeQueueMemory(
    IN PCmdQElmt   cmdqelmt
    );

VOID    RTUSBDequeueCmd(
    IN  PCmdQ       cmdq,
    OUT PCmdQElmt   *pcmdqelmt);

NTSTATUS    N6UsbIoVendorRequest(
    IN  PMP_ADAPTER   pAd,
    IN  ULONG           TransferFlags,
    IN  UCHAR           ReservedBits,
    IN  UCHAR           Request,
    IN  USHORT          Value,
    IN  USHORT          Index,
    IN  PVOID           TransferBuffer,
    IN  ULONG           TransferBufferLength);

IO_COMPLETION_ROUTINE MakeSynchronousIoctlWithTimeOutCompletion;

NTSTATUS
MakeSynchronousIoctlWithTimeOutCompletion(
    IN PDEVICE_OBJECT   DeviceObject,
    IN PIRP             Irp,
    IN PVOID            Context
    );

UCHAR eFuseReadRegisters(
    IN PMP_ADAPTER pAd, 
    IN USHORT Offset, 
    IN USHORT Length, 
    OUT USHORT* pData);

NTSTATUS eFuseRead(
    IN PMP_ADAPTER    pAd,
    IN USHORT Offset,
    OUT PUCHAR pData,
    IN USHORT Length);

NTSTATUS    RTUSB_SelectConfiguration(
    IN  PMP_ADAPTER                   pAd,
    IN  PUSB_CONFIGURATION_DESCRIPTOR   pConfigDesc,
    IN  PUSBD_INTERFACE_LIST_ENTRY      pInterfaceList,
    OUT PURB                            *pConfigReqUrb);

NTSTATUS    RTUSB_ResetPipe(
    IN  PMP_ADAPTER   pAd, 
    IN  UINT            BulkPipe);

NTSTATUS    RTUSB_ResetDevice(
    IN  PMP_ADAPTER   pAd);

NTSTATUS RTUSB_GetPortStatus(
    IN  PMP_ADAPTER   pAd);

NTSTATUS RTUSB_EndpointUsage(
    IN  PMP_ADAPTER   pAd);

NTSTATUS RTUSB_ReBulkOut(
    IN  PMP_ADAPTER   pAd,
    IN  PHT_TX_CONTEXT  pTxContext,
    IN  UCHAR       BulkOutPipeId,
    IN  UCHAR       BulkFailIrpIdx);

VOID    RTUSBCancelPendingBulkOutRequest(
    IN  PMP_ADAPTER   pAd);

VOID    RTUSBCancelPendingBulkInRequest(
    IN  PMP_ADAPTER   pAd);

NTSTATUS    RTUSB_Replug(
    IN  PMP_ADAPTER   pAd);

NDIS_STATUS QueryHardWareRegister(
    IN  PMP_ADAPTER   pAd,
    IN  PVOID           pBuf);
NDIS_STATUS SetHardWareRegister(
    IN  PMP_ADAPTER   pAd,
    IN  PVOID           pBuf);
    
NDIS_STATUS RTUSBQueryHardWareRegister(
    IN  PMP_ADAPTER   pAd,
    IN  PVOID           pBuf);

NDIS_STATUS RTUSBSetHardWareRegister(
    IN  PMP_ADAPTER   pAd,
    IN  PVOID           pBuf);

////
// prototype in Sanity.c
//
BOOLEAN MlmeStartReqSanity(
    IN PMP_ADAPTER pAd, 
    IN VOID *Msg, 
    IN ULONG MsgLen, 
    OUT CHAR Ssid[], 
    OUT UCHAR *pSsidLen) ;

BOOLEAN MlmeAssocReqSanity(
    IN PMP_ADAPTER pAd, 
    IN VOID *Msg, 
    IN ULONG MsgLen, 
    OUT PUCHAR pApAddr, 
    OUT USHORT *pCapabilityInfo, 
    OUT ULONG *pTimeout, 
    OUT USHORT *pListenIntv) ;

BOOLEAN MlmeAuthReqSanity(
    IN PMP_ADAPTER pAd, 
    IN VOID *Msg, 
    IN ULONG MsgLen, 
    OUT PUCHAR pAddr, 
    OUT ULONG *pTimeout, 
    OUT USHORT *pAlg); 

BOOLEAN PeerAssocRspSanity(
    IN PMP_ADAPTER pAd, 
    IN PMP_PORT     pPort,
    IN VOID *pMsg, 
    IN ULONG MsgLen, 
    OUT PUCHAR pAddr2, 
    OUT USHORT *pCapabilityInfo, 
    OUT USHORT *pStatus, 
    OUT USHORT *pAid, 
    OUT UCHAR SupRate[], 
    OUT UCHAR *pSupRateLen,
    OUT UCHAR ExtRate[], 
    OUT UCHAR *pExtRateLen,
    OUT ULONG *pP2PIeLen,
    OUT HT_CAPABILITY_IE        *pHtCapability,
    OUT VHT_CAP_IE *pVhtCapability,
    OUT VHT_OP_IE  *pVhtOperation,
    OUT ADD_HT_INFO_IE      *pAddHtInfo,    // AP might use this additional ht info IE 
    OUT UCHAR           *pHtCapabilityLen,
    OUT UCHAR           *pAddHtInfoLen,
    OUT UCHAR           *bVhtCapable,
    OUT UCHAR           *pNewExtChannelOffset,
    OUT PEDCA_PARM pEdcaParm,
    OUT PEXT_CAP_ELEMT pExtCapabilities);

BOOLEAN PeerDisassocSanity(
    IN PMP_ADAPTER pAd, 
    IN VOID *Msg, 
    IN ULONG MsgLen, 
    OUT PUCHAR pAddr2, 
    OUT USHORT *pReason); 

BOOLEAN PeerDeauthSanity(
    IN PMP_ADAPTER pAd, 
    IN VOID *Msg, 
    IN ULONG MsgLen, 
    OUT PUCHAR pAddr2, 
    OUT USHORT *pReason); 

BOOLEAN PeerAuthSanity(
    IN PMP_ADAPTER pAd, 
    IN VOID *Msg, 
    IN ULONG MsgLen, 
    OUT PUCHAR pAddr, 
    OUT USHORT *pAlg, 
    OUT USHORT *pSeq, 
    OUT USHORT *pStatus, 
    CHAR *pChlgText);

BOOLEAN PeerProbeReqSanity(
    IN PMP_ADAPTER pAd, 
    IN VOID *Msg, 
    IN ULONG MsgLen, 
    OUT PUCHAR pAddr2,
    OUT CHAR Ssid[], 
    OUT UCHAR *pSsidLen);

BOOLEAN MlmeSyncPeerBeaconAndProbeRspSanity(
    IN PMP_ADAPTER pAd, 
    IN PMP_PORT   pPort,
    IN VOID *Msg, 
    IN ULONG MsgLen, 
    OUT PUCHAR pAddr2, 
    OUT PUCHAR pBssid, 
    OUT CHAR Ssid[], 
    OUT UCHAR *pSsidLen, 
    OUT UCHAR *pBssType, 
    OUT USHORT *pBeaconPeriod, 
    OUT UCHAR *pChannel, 
    OUT UCHAR *pNewChannel,
    OUT ULONGLONG *pTimestamp, 
    OUT CF_PARM *pCfParm, 
    OUT USHORT *pAtimWin, 
    OUT USHORT *pCapabilityInfo, 
    OUT UCHAR *pErp,
    OUT UCHAR *pDtimCount, 
    OUT UCHAR *pDtimPeriod, 
    OUT UCHAR *pBcastFlag, 
    OUT UCHAR *pMessageToMe, 
    OUT UCHAR SupRate[],
    OUT UCHAR *pSupRateLen,
    OUT UCHAR ExtRate[],
    OUT UCHAR *pExtRateLen,
    OUT PEDCA_PARM       pEdcaParm,
    OUT PQBSS_LOAD_PARM  pQbssLoad,
    OUT PQOS_CAPABILITY_PARM pQosCapability,
    OUT ULONG *pRalinkIe,
    OUT UCHAR        *pHtCapabilityLen,
    OUT UCHAR        *pPreNHtCapabilityLen,
    OUT HT_CAPABILITY_IE *pHtCapability,
    OUT BOOLEAN          *bVhtCapable,
    OUT VHT_CAP_IE       *pVhtCapability,
    OUT VHT_OP_IE        *pVhtOperation,
    OUT UCHAR        *AddHtInfoLen,
    OUT ADD_HT_INFO_IE *AddHtInfo,
    OUT EXT_CAP_ELEMT               *pExtCap,   // this is the extened capibility IE  
    OUT UCHAR *NewExtChannelOffset,     // Ht extension channel offset(above or below)
    OUT USHORT *WSCInfoAtBeaconsLen,
    OUT PUCHAR WSCInfoAtBeacons,
    OUT USHORT *WSCInfoAtProbeRspLen,
    OUT PUCHAR WSCInfoAtProbeRsp,
    OUT PSSIDL_IE pSSIDL_VIE,
    OUT PUCHAR pMaxTxPowerLevel, 
    OUT PUCHAR pLocalPowerConstraint, 
    OUT UCHAR *bConnectedToCiscoAp,
    OUT PBOOLEAN bHasOperatingModeField,
    OUT POPERATING_MODE_FIELD pOperatingModeField,
    OUT USHORT *LengthVIE,  
    OUT PNDIS_802_11_VARIABLE_IEs pVIE);

BOOLEAN MlmeSyncPeerBeaconAndProbeRspSanity2(
    IN PMP_ADAPTER pAd, 
    IN VOID *Msg, 
    IN ULONG MsgLen, 
    IN OVERLAP_BSS_SCAN_IE *BssScan,
    OUT UCHAR   *RegClass);

BOOLEAN PeerAddBAReqActionSanity(
    IN PMP_ADAPTER pAd, 
    IN VOID *pMsg, 
    IN ULONG MsgLen,
    OUT PUCHAR pAddr2);


BOOLEAN PeerAddBARspActionSanity(
    IN PMP_ADAPTER pAd, 
    IN VOID *pMsg, 
    IN ULONG MsgLen);

BOOLEAN PeerDelBAActionSanity(
    IN PMP_ADAPTER pAd, 
    IN UCHAR Wcid, 
    IN VOID *pMsg, 
    IN ULONG MsgLen);

BOOLEAN GetTimBit(
    IN  CHAR    *Ptr, 
    IN  USHORT  Aid, 
    OUT UCHAR   *TimLen, 
    OUT UCHAR   *BcastFlag, 
    OUT UCHAR   *DtimCount, 
    OUT UCHAR   *DtimPeriod,
    OUT UCHAR   *MessageToMe);

UCHAR ChannelSanity(
    IN PMP_ADAPTER pAd, 
    IN UCHAR channel);

UCHAR PeerTxTypeInUseSanity(
    IN UCHAR  Channel,
    IN UCHAR  SupRate[],
    IN UCHAR  SupRateLen,
    IN UCHAR  ExtRate[],
    IN UCHAR  ExtRateLen);

BOOLEAN MlmeAddBAReqSanity(
    IN PMP_ADAPTER pAd, 
    IN VOID *Msg, 
    IN ULONG MsgLen,
    OUT PUCHAR pAddr2); 

BOOLEAN MlmeDelBAReqSanity(
    IN PMP_ADAPTER pAd, 
    IN VOID *Msg, 
    IN ULONG MsgLen);

BOOLEAN MlmeDlsReqSanity(
    IN  PMP_ADAPTER   pAd, 
    IN  VOID            *Msg, 
    IN  ULONG           MsgLen,
    OUT PRT_802_11_DLS  *pDLS,
    OUT PUSHORT         pReason);

BOOLEAN PeerDlsReqSanity(
    IN  PMP_ADAPTER   pAd, 
    IN  VOID            *Msg, 
    IN  ULONG           MsgLen,
    OUT PUCHAR          pDA,    
    OUT PUCHAR          pSA,
    OUT USHORT          *pCapabilityInfo, 
    OUT USHORT *pDlsTimeout,
    OUT PHT_CAPABILITY_IE pHtCap,
    OUT PULONG pHtCapLen);

BOOLEAN PeerDlsRspSanity(
    IN  PMP_ADAPTER   pAd, 
    IN  VOID            *Msg, 
    IN  ULONG           MsgLen,
    OUT PUCHAR          pDA,
    OUT PUCHAR          pSA,
    OUT USHORT          *pCapabilityInfo, 
    OUT PHT_CAPABILITY_IE pHtCap,
    OUT PULONG pHtCapLen,
    OUT USHORT          *pStatus);

BOOLEAN PeerDlsTearDownSanity(
    IN PMP_ADAPTER pAd, 
    IN VOID *Msg, 
    IN ULONG MsgLen,
    OUT PUCHAR pDA,
    OUT PUCHAR pSA,
    OUT USHORT *pReason);

BOOLEAN MlmeTdlsReqSanity(
    IN PMP_ADAPTER pAd, 
    IN VOID *Msg, 
    IN ULONG MsgLen,
    OUT PRT_802_11_TDLS *pTDLS,
    OUT PUSHORT pReason);

BOOLEAN PeerTdlsSetupReqSanity(
    IN PMP_ADAPTER    pAd, 
    PMP_PORT      pPort,
    IN VOID     *Msg, 
    IN ULONG    MsgLen,
    OUT UCHAR   *pToken,
    OUT UCHAR   *pSA,
    OUT UCHAR   *pSA2,  
    OUT USHORT  *pCapabilityInfo,
    OUT UCHAR   *pSupRateLen,   
    OUT UCHAR   SupRate[],
    OUT UCHAR   *pExtRateLen,
    OUT UCHAR   ExtRate[],
    OUT BOOLEAN *pbWmmCapable,  
    OUT UCHAR   *pQosCapability,
    OUT UCHAR   *pHtCapLen,
    OUT HT_CAPABILITY_IE    *pHtCap,
    OUT EXT_CAP_ELEMT *pExtCap,
    OUT UCHAR   *pRsnLen,
    OUT UCHAR   RsnIe[],
    OUT UCHAR   *pFTLen,
    OUT UCHAR   FTIe[],
    OUT UCHAR   *pTILen,
    OUT UCHAR   TIIe[]);

BOOLEAN PeerTdlsSetupRspSanity(
    IN PMP_ADAPTER    pAd, 
    PMP_PORT      pPort,
    IN VOID     *Msg, 
    IN ULONG    MsgLen,
    OUT UCHAR   *pToken,
    OUT UCHAR   *pSA,
    OUT UCHAR   *pSA2,  
    OUT USHORT  *pCapabilityInfo,
    OUT UCHAR   *pSupRateLen,   
    OUT UCHAR   SupRate[],
    OUT UCHAR   *pExtRateLen,
    OUT UCHAR   ExtRate[],
    OUT BOOLEAN *pbWmmCapable,  
    OUT UCHAR   *pQosCapability,
    OUT UCHAR   *pHtCapLen,
    OUT HT_CAPABILITY_IE    *pHtCap,
    OUT EXT_CAP_ELEMT   *pExtCap,
    OUT USHORT  *pStatusCode,
    OUT UCHAR   *pRsnLen,
    OUT UCHAR   RsnIe[],
    OUT UCHAR   *pFTLen,
    OUT UCHAR   FTIe[],
    OUT UCHAR   *pTILen,
    OUT UCHAR   TIIe[]);

BOOLEAN PeerTdlsSetupConfSanity(
    IN PMP_ADAPTER    pAd, 
    PMP_PORT      pPort,
    IN VOID     *Msg, 
    IN ULONG    MsgLen,
    OUT UCHAR   *pToken,
    OUT UCHAR   *pSA,
    OUT UCHAR   *pSA2,  
    OUT USHORT  *pCapabilityInfo,
    OUT EDCA_PARM   *pEdcaParm,
    OUT USHORT  *pStatusCode,
    OUT UCHAR   *pRsnLen,
    OUT UCHAR   RsnIe[],
    OUT UCHAR   *pFTLen,
    OUT UCHAR   FTIe[],
    OUT UCHAR   *pTILen,
    OUT UCHAR   TIIe[]);

BOOLEAN PeerTdlsTearDownSanity(
    IN PMP_ADAPTER pAd, 
    PMP_PORT      pPort,
    IN VOID *Msg, 
    IN ULONG MsgLen,
    OUT UCHAR   *pSA,
    OUT UCHAR   *pSA2,  
    OUT BOOLEAN *pIsInitator,
    OUT USHORT *pReasonCode,
    OUT UCHAR   *pFTLen,    
    OUT UCHAR   FTIe[]);

BOOLEAN SAQuerySanity(
    IN PMP_ADAPTER pAd, 
    IN UCHAR Wcid, 
    IN VOID *pMsg, 
    IN ULONG MsgLen);

////
// prototype in Sync.c
//

VOID MlmeSyncStateMachineInit(
    IN  PMP_ADAPTER       pAd, 
    IN  STATE_MACHINE       *Sm, 
    OUT STATE_MACHINE_FUNC  Trans[]) ;

VOID MlmeSyncBeaconTimeoutTimerCallback(
    IN PVOID SystemSpecific1, 
    IN PVOID FunctionContext, 
    IN PVOID SystemSpecific2, 
    IN PVOID SystemSpecific3) ;

VOID MlmeSyncRestoreChannelTimeoutTimerCallback(
    IN PVOID SystemSpecific1, 
    IN PVOID FunctionContext, 
    IN PVOID SystemSpecific2, 
    IN PVOID SystemSpecific3);

VOID MlmeSyncScanTimeoutTimerCallback(
    IN PVOID SystemSpecific1, 
    IN PVOID FunctionContext, 
    IN PVOID SystemSpecific2, 
    IN PVOID SystemSpecific3) ;

VOID MlmeSyncScanReqAction(
    IN PMP_ADAPTER pAd, 
    IN MLME_QUEUE_ELEM *Elem) ;

VOID MlmeSyncJoinReqAction(
    IN PMP_ADAPTER pAd, 
    IN MLME_QUEUE_ELEM *Elem) ;

VOID MlmeSyncStartReqAction(
    IN PMP_ADAPTER pAd, 
    IN MLME_QUEUE_ELEM *Elem) ;

VOID MlmeSyncMlmeSyncPeerBeaconAtScanAction(
    IN PMP_ADAPTER pAd, 
    IN MLME_QUEUE_ELEM *Elem) ;

VOID MlmeSyncMlmeSyncPeerBeaconAtJoinAction(
    IN PMP_ADAPTER pAd, 
    IN MLME_QUEUE_ELEM *Elem) ;

VOID MlmeSyncPeerBeacon(
    IN PMP_ADAPTER pAd, 
    IN MLME_QUEUE_ELEM *Elem) ;

VOID MlmeSyncPeerProbeReqAction(
    IN PMP_ADAPTER pAd, 
    IN MLME_QUEUE_ELEM *Elem) ;

VOID MlmeSyncPeerProbeRspAction(
    IN PMP_ADAPTER pAd, 
    IN MLME_QUEUE_ELEM *Elem);

VOID MlmeSyncBeaconTimeoutAtJoinAction(
    IN PMP_ADAPTER pAd, 
    IN MLME_QUEUE_ELEM *Elem) ;

VOID MlmeSyncPeerReqAtScanAction(
    IN PMP_ADAPTER pAd, 
    IN MLME_QUEUE_ELEM *Elem);

VOID
MlmeSyncMlmeSyncScanTimeoutActionWorkitemCallBack(
    PVOID   Context
    );
    
VOID MlmeSyncScanTimeoutAction(
    IN PMP_ADAPTER pAd, 
    IN MLME_QUEUE_ELEM *Elem) ;

VOID MlmeSyncRestoreChannelTimeoutAction(
    IN PMP_ADAPTER pAd, 
    IN MLME_QUEUE_ELEM *Elem);

BOOLEAN MlmeSyncHasHiddenAPAtThisChannel(
    IN PMP_ADAPTER pAd, 
    IN UCHAR channel,
    IN BOOLEAN bHasCiscoAP);

BOOLEAN MlmeSyncHasWscAPAtThisChannel(
    IN PMP_ADAPTER pAd, 
    IN UCHAR channel);

PMP_PORT MlmeSyncGetApPort(IN PMP_ADAPTER pAd);

PMP_PORT MlmeSyncGetActivePort(
    IN PMP_ADAPTER pAd);

PMP_PORT
MlmeSyncGetP2pClientPort(
    IN PMP_ADAPTER pAd,
    IN UCHAR        CliIdx);

VOID MlmeSyncScanDoneAction(
    IN PMP_ADAPTER pAd,
    IN PMP_PORT   pPort);

VOID MlmeSyncScanMlmeSyncNextChannel(
    PMP_ADAPTER pAd,
    PMP_PORT pPort) ;

VOID MlmeSyncScanReqAction2(
    IN PMP_ADAPTER pAd, 
    IN MLME_QUEUE_ELEM *Elem);

VOID MlmeSyncInvalidStateWhenScan(
    IN PMP_ADAPTER pAd, 
    IN MLME_QUEUE_ELEM *Elem) ;

VOID MlmeSyncInvalidStateWhenJoin(
    IN PMP_ADAPTER pAd, 
    IN MLME_QUEUE_ELEM *Elem) ;

VOID MlmeSyncInvalidStateWhenStart(
    IN PMP_ADAPTER pAd, 
    IN MLME_QUEUE_ELEM *Elem) ;

VOID BuildChannelList(
    IN PMP_ADAPTER pAd,
    IN PMP_PORT     pPort);

UCHAR MlmeSyncFirstChannel(
    IN PMP_ADAPTER pAd);

UCHAR MlmeSyncNextChannel(
    IN PMP_ADAPTER pAd, 
    IN PMP_PORT     pPort,
    IN UCHAR channel);

CHAR    MlmeSyncConvertToRssi(
    IN  PMP_ADAPTER   pAd,
    IN  UCHAR           Rssi,
    IN  UCHAR           RssiNumber,
    IN  UCHAR           AntSel,
    IN  UCHAR           BW);

VOID MlmeSyncBuildEffectedChannelList(
    IN PMP_ADAPTER pAd);

VOID MlmeSyncDeleteEffectedChannelList(
    IN PMP_ADAPTER pAd);

VOID MlmeSyncProbeInactiveStation(
    IN PMP_ADAPTER pAd,
    IN PMP_PORT pPort,
    IN PUCHAR pDA);

BOOLEAN MlmeSyncIsValidChannel(
    IN PMP_ADAPTER pAd,
    IN UCHAR channel);

VOID MlmeSyncCheckBWOffset(
    IN PMP_ADAPTER    pAd,
    IN UCHAR            Channel);

BOOLEAN MlmeSyncStaAdhocUpdateMacTableEntry(
    IN  PMP_ADAPTER       pAd,
    IN  PMP_PORT          pPort,
    IN  PMAC_TABLE_ENTRY    pEntry,
    IN  UCHAR               MaxSupportedRateIn500Kbps,
    IN  UCHAR               MinSupportedRateIn500Kbps,
    IN  HT_CAPABILITY_IE    *pHtCapability,
    IN  UCHAR               HtCapabilityLen,
    IN  ADD_HT_INFO_IE      *pAddHtInfo,
    IN  UCHAR               AddHtInfoLen,
    IN  USHORT              CapabilityInfo);

////
// prototype in Wpa.c
//
void KD_hmac_sha256(
    IN  const unsigned char key[],
    IN  unsigned long key_len,
    IN  const unsigned char data[], 
    IN  unsigned long data_len,
    IN OUT  unsigned char output[], 
    IN  unsigned long output_len);

BOOLEAN WpaMsgTypeSubst(
    IN  UCHAR   EAPType,
    OUT ULONG   *MsgType);  

VOID WpaPskStateMachineInit(
    IN  PMP_ADAPTER   pAd, 
    IN  STATE_MACHINE *S, 
    OUT STATE_MACHINE_FUNC Trans[]);    

VOID WpaEAPOLKeyAction(
    IN  PMP_ADAPTER   pAd, 
    IN  MLME_QUEUE_ELEM *Elem); 

VOID    WpaPairMsg1Action(
    IN  PMP_ADAPTER   pAd, 
    IN  MLME_QUEUE_ELEM *Elem); 

VOID    WpaPairMsg3Action(
    IN  PMP_ADAPTER   pAd, 
    IN  MLME_QUEUE_ELEM *Elem); 

VOID    WpaGroupMsg1Action(
    IN  PMP_ADAPTER   pAd, 
    IN  MLME_QUEUE_ELEM *Elem); 

VOID    WpaMacHeaderInit(
    IN      PMP_ADAPTER   pAd, 
    IN      PMP_PORT      pPort,
    IN OUT  PHEADER_802_11  pHdr80211, 
    IN      UCHAR           wep, 
    IN      PUCHAR          pAddr1);    

VOID    WpaHardTransmit(
    IN  PMP_ADAPTER   pAd,
    IN  PMP_PORT      pPort,
    IN  PUCHAR          pFrame,
    IN  ULONG           FrameLen);

VOID    HMAC_SHA1(
    IN  UCHAR   *text,
    IN  UINT    text_len,
    IN  UCHAR   *key,
    IN  UINT    key_len,
    IN  UCHAR   *digest);

VOID    PRF(
    IN  PMP_ADAPTER pAd,
    IN  UCHAR   *key,
    IN  INT     key_len,
    IN  UCHAR   *prefix,
    IN  INT     prefix_len,
    IN  UCHAR   *data,
    IN  INT     data_len,
    OUT UCHAR   *output,
    IN  INT     len);

VOID    CCKMPRF(
    IN  PMP_ADAPTER pAd,
    IN  UCHAR   *key,
    IN  INT     key_len,
    IN  UCHAR   *data,
    IN  INT     data_len,
    OUT UCHAR   *output,
    IN  INT     len);

VOID WpaApWpaCountPTK(
    IN  PMP_ADAPTER   pAd, 
    IN  PMP_PORT      pPort,
    IN  UCHAR   *PMK,
    IN  UCHAR   *ANonce,
    IN  UCHAR   *AA,
    IN  UCHAR   *SNonce,
    IN  UCHAR   *SA,
    OUT UCHAR   *output,
    IN  UINT    len);

VOID    GenRandom(
    IN  PMP_ADAPTER   pAd, 
    OUT UCHAR           *random);

VOID    AES_GTK_KEY_UNWRAP( 
    IN  UCHAR   *key,
    OUT UCHAR   *plaintext,
    IN  UCHAR   *ciphertext);

VOID HMAC_SHA256 (
    IN  const UCHAR Key[], 
    IN  UINT KeyLen, 
    IN  const UCHAR Message[], 
    IN  UINT MessageLen, 
    OUT UCHAR MAC[],
    IN  UINT MACLen);

VOID    FT_KDF(
    IN  PUCHAR  key,
    IN  INT     key_len,
    IN  PUCHAR  label,
    IN  INT     label_len,
    IN  PUCHAR  data,
    IN  UINT        data_len,
    OUT PUCHAR  output,
    IN  UINT        len);

////
// prototype in wsc.c
//
VOID WscStateMachineInit(
    IN  PMP_ADAPTER       pAd, 
    IN  STATE_MACHINE       *S, 
    OUT STATE_MACHINE_FUNC Trans[]);

VOID WscEAPAction(
    IN  PMP_ADAPTER   pAd, 
    IN  MLME_QUEUE_ELEM *Elem);

VOID    WscEAPEnrolleeAction(
    IN  PMP_ADAPTER       pAd,
    IN  PMP_PORT          pPort,
    IN  UCHAR               MsgType,
    IN  UCHAR               Id,
    IN  MLME_QUEUE_ELEM     *Elem);
    
VOID    WscEAPRegistrarAction(
    IN  PMP_ADAPTER       pAd,
    IN  PMP_PORT          pPort,
    IN  UCHAR               MsgType,
    IN  UCHAR               Id,
    IN  MLME_QUEUE_ELEM     *Elem,
    IN  PWSC_EXTREG_MSG     ExtRegMsg,
    IN  PUCHAR              UserSelEnrMAC);

UCHAR   WscMfNeedBuf(
    IN  PMP_ADAPTER pAd,
    IN  PMP_PORT    pPort,
    OUT PUCHAR              pId,
    IN  PMLME_QUEUE_ELEM    Elem);

UCHAR   WscRxMsgType(
    IN  PMP_ADAPTER   pAd, 
    IN  PMP_PORT      pPort,
    OUT PUCHAR              pId,
    IN  PMLME_QUEUE_ELEM    pElem);

BOOLEAN WscMsgTypeSubst(
    IN  UCHAR   EAPType,
    IN  UCHAR   EAPCode,
    OUT ULONG   *MsgType);

VOID WscConnectTimeoutTimerCallback(
    IN PVOID    SystemSpecific1, 
    IN PVOID    FunctionContext, 
    IN PVOID    SystemSpecific2, 
    IN PVOID    SystemSpecific3);

VOID WscMlmeSyncScanTimeoutTimerCallback(
    IN PVOID    SystemSpecific1, 
    IN PVOID    FunctionContext, 
    IN PVOID    SystemSpecific2, 
    IN PVOID    SystemSpecific3);

VOID WscPBCTimeoutTimerCallback(
    IN PVOID    SystemSpecific1, 
    IN PVOID    FunctionContext, 
    IN PVOID    SystemSpecific2, 
    IN PVOID    SystemSpecific3);

VOID WscPINTimeoutTimerCallback(
    IN PVOID    SystemSpecific1, 
    IN PVOID    FunctionContext, 
    IN PVOID    SystemSpecific2, 
    IN PVOID    SystemSpecific3);

VOID WscEAPRxTimeoutTimerCallback(
    IN PVOID    SystemSpecific1, 
    IN PVOID    FunctionContext, 
    IN PVOID    SystemSpecific2, 
    IN PVOID    SystemSpecific3);

VOID WscRxTimeOuttoScanTimerCallback(
    IN PVOID    SystemSpecific1, 
    IN PVOID    FunctionContext, 
    IN PVOID    SystemSpecific2, 
    IN PVOID    SystemSpecific3);

VOID WscM8WaitEapFailTimeoutTimerCallback(
    IN PVOID    SystemSpecific1, 
    IN PVOID    FunctionContext, 
    IN PVOID    SystemSpecific2, 
    IN PVOID    SystemSpecific3);
    
VOID WscEapM2TimeoutTimerCallback(
    IN PVOID    SystemSpecific1, 
    IN PVOID    FunctionContext, 
    IN PVOID    SystemSpecific2, 
    IN PVOID    SystemSpecific3);

VOID WscLEDTimerTimerCallback(
    IN PVOID    SystemSpecific1, 
    IN PVOID    FunctionContext, 
    IN PVOID    SystemSpecific2, 
    IN PVOID    SystemSpecific3);

VOID WscSkipTurnOffLEDTimerCallback(
    IN PVOID    SystemSpecific1, 
    IN PVOID    FunctionContext, 
    IN PVOID    SystemSpecific2, 
    IN PVOID    SystemSpecific3);


VOID    WscPushHWPBCStartAction(
    IN  PMP_ADAPTER   pAd,
    IN  PMP_PORT  pPort);

VOID    WscPushHWPBCDisconnectAction(
    IN  PMP_ADAPTER   pAd,
    IN  PMP_PORT  pPort);
VOID    WscPushPBCAction(
    IN  PMP_ADAPTER   pAd,
    IN  PMP_PORT      pPort);

VOID    WscScanExec(
    IN  PMP_ADAPTER   pAd,
    IN  PMP_PORT      pPort);

VOID    WscPBCExec(
    IN  PMP_ADAPTER   pAd,
    IN  PMP_PORT      pPort);

VOID    WscPINExec(
    IN  PMP_ADAPTER   pAd);

VOID WscExtregCallPBCExecTimerCallback(
    IN PVOID    SystemSpecific1, 
    IN PVOID    FunctionContext, 
    IN PVOID    SystemSpecific2, 
    IN PVOID    SystemSpecific3);

VOID WscExtregCallScanExecTimerCallback(
    IN PVOID    SystemSpecific1, 
    IN PVOID    FunctionContext, 
    IN PVOID    SystemSpecific2, 
    IN PVOID    SystemSpecific3);

VOID    WscExtregPBCExec(
    IN  PMP_ADAPTER   pAd,
    IN  PMP_PORT      pPort);

VOID    WscExtRegScanExec(
    IN  PMP_PORT      pPort);

VOID WscPBCBssTableSort(
    IN  PMP_ADAPTER   pAd, 
    IN  PMP_PORT      pPort,
    OUT BSS_TABLE       *OutTab);

VOID WscPBCModeAPSearch(
    IN PMP_ADAPTER    pAd, 
    IN PUCHAR pBSSID, 
    IN UCHAR Channel, 
    OUT BSS_TABLE       *OutTab, 
    OUT PULONG          pScanningTableIndex);

VOID WscPINModeAPSearch(
    IN  PMP_ADAPTER   pAd, 
    IN  PMP_PORT      pPort,
    IN  PUCHAR          pBSSID, 
    IN UCHAR Channel, 
    OUT BSS_TABLE       *OutTab, 
    OUT PULONG          pScanningTableIndex);

VOID WscCheckWPSAP(
    IN OUT PBSS_ENTRY pBssEntry);

VOID    WscMacHeaderInit(
    IN      PMP_ADAPTER   pAd, 
    IN      PMP_PORT      pPort,
    IN OUT  PHEADER_802_11  Hdr, 
    IN      PUCHAR          pAddr1);

VOID WscStartAction(
    IN  PMP_ADAPTER   pAd,
    IN      PMP_PORT      pPort);

VOID    WscInitRegistrarPair(
    IN  PMP_ADAPTER       pAd,
    IN      PMP_PORT      pPort);

VOID    WscSetDevInfo(
    IN  PMP_ADAPTER       pAd,
    IN  PMP_PORT      pPort,
    IN  PWSC_DEV_INFO   pDevInfo);

VOID    WscSetDevInfo(
    IN  PMP_ADAPTER       pAd,
    IN  PMP_PORT      pPort,
    IN  PWSC_DEV_INFO   pDevInfo);

VOID    WscMakeEAPFrame(
    IN  PMP_ADAPTER       pAd, 
    IN      PMP_PORT      pPort,
    IN  UCHAR               Id,
    IN  UCHAR               OpCode,
    IN  PUCHAR              pData,
    IN  INT                 Len,
    IN  PMLME_QUEUE_ELEM    pElem,
    IN  BOOLEAN             bFrag,
    IN  BOOLEAN             bFirstFreg);

VOID    WscFragmentedEAP(
    IN  PMP_ADAPTER       pAd, 
    IN  PMP_PORT          pPort,
    IN  UCHAR               Id,
    IN  UCHAR               OpCode,
    IN  PMLME_QUEUE_ELEM    pElem,
    IN  BOOLEAN             bFirstFrag);
    
VOID    WsceSendEapReqWscStart(
    IN  PMP_ADAPTER       pAd, 
    IN  PMP_PORT          pPort,
    IN  UCHAR               Id,
    IN  PMLME_QUEUE_ELEM    pElem);

VOID    WscSendEapReqId(
    IN  PMP_ADAPTER       pAd, 
    IN  PMP_PORT          pPort,
    IN  UCHAR               Id,
    IN  PMLME_QUEUE_ELEM    pElem);

VOID    WscSendEapRspId(
    IN  PMP_ADAPTER       pAd, 
    IN  PMP_PORT          pPort,
    IN  UCHAR               Id,
    IN  PMLME_QUEUE_ELEM    pElem);

VOID    WscSendMessageM1(
    IN  PMP_ADAPTER       pAd, 
    IN  PMP_PORT          pPort,
    IN  UCHAR               Id,
    IN  PMLME_QUEUE_ELEM    pElem);

VOID    WscSendMessageM2(
    IN  PMP_ADAPTER       pAd, 
    IN  PMP_PORT          pPort,
    IN  UCHAR               Id,
    IN  PMLME_QUEUE_ELEM    pElem);

VOID    WscSendMessageM2D(
    IN  PMP_ADAPTER       pAd, 
    IN  PMP_PORT pPort,
    IN  UCHAR               Id,
    IN  PMLME_QUEUE_ELEM    pElem);

VOID    WscSendMessageM3(
    IN  PMP_ADAPTER       pAd, 
    IN  PMP_PORT pPort,
    IN  UCHAR               Id,
    IN  PMLME_QUEUE_ELEM    pElem);

VOID    WscSendMessageM4(
    IN  PMP_ADAPTER       pAd, 
    IN  PMP_PORT pPort,
    IN  UCHAR               Id,
    IN  PMLME_QUEUE_ELEM    pElem);

VOID    WscSendMessageM5(
    IN  PMP_ADAPTER       pAd, 
    IN  PMP_PORT pPort,
    IN  UCHAR               Id,
    IN  PMLME_QUEUE_ELEM    pElem);

VOID    WscSendMessageM6(
    IN  PMP_ADAPTER       pAd, 
    IN  PMP_PORT pPort,
    IN  UCHAR               Id,
    IN  PMLME_QUEUE_ELEM    pElem);

VOID    WscSendMessageM7(
    IN  PMP_ADAPTER       pAd, 
    IN  PMP_PORT pPort,
    IN  UCHAR               Id,
    IN  PMLME_QUEUE_ELEM    pElem);

VOID    WscSendMessageM8(
    IN  PMP_ADAPTER       pAd, 
    IN  PMP_PORT pPort,
    IN  UCHAR               Id,
    IN  PMLME_QUEUE_ELEM    pElem);

VOID    WscSendEapRspAck(
    IN  PMP_ADAPTER       pAd, 
    IN  PMP_PORT pPort,
    IN  UCHAR               Id,
    IN  PMLME_QUEUE_ELEM    pElem);

VOID    WscSendEapFragAck(
    IN  PMP_ADAPTER       pAd, 
    IN  PMP_PORT pPort,
    IN  UCHAR               Id,
    IN  PMLME_QUEUE_ELEM    pElem);

VOID    WscSendEapRspNack(
    IN  PMP_ADAPTER       pAd, 
    IN  PMP_PORT pPort,
    IN  UCHAR               Id,
    IN  PMLME_QUEUE_ELEM    pElem);

VOID    WscSendEapRspDone(
    IN  PMP_ADAPTER       pAd, 
    IN  PMP_PORT pPort,
    IN  UCHAR               Id,
    IN  PMLME_QUEUE_ELEM    pElem);

BOOLEAN WscRecvMessageM1(
    IN  PMP_ADAPTER       pAd, 
    IN  PMP_PORT pPort,
    IN  UCHAR               Id,
    IN  PMLME_QUEUE_ELEM    pElem,
    IN  PWSC_EXTREG_MSG     pExtRegMsg);

BOOLEAN WscRecvMessageM2(
    IN  PMP_ADAPTER       pAd, 
    IN  PMP_PORT pPort,
    IN  UCHAR               Id,
    IN  PMLME_QUEUE_ELEM    pElem);

BOOLEAN WscRecvMessageM2D(
    IN  PMP_ADAPTER       pAd, 
    IN  PMP_PORT pPort,
    IN  UCHAR               Id,
    IN  PMLME_QUEUE_ELEM    pElem);

BOOLEAN WscRecvMessageM3(
    IN  PMP_ADAPTER       pAd, 
    IN  PMP_PORT pPort,
    IN  UCHAR               Id,
    IN  PMLME_QUEUE_ELEM    pElem,
    IN  PWSC_EXTREG_MSG     pExtRegMsg);

BOOLEAN WscRecvMessageM4(
    IN  PMP_ADAPTER       pAd, 
    IN  PMP_PORT pPort,
    IN  UCHAR               Id,
    IN  PMLME_QUEUE_ELEM    pElem);

BOOLEAN WscRecvMessageM5(
    IN  PMP_ADAPTER       pAd, 
    IN  PMP_PORT pPort,
    IN  UCHAR               Id,
    IN  PMLME_QUEUE_ELEM    pElem,
    IN  PWSC_EXTREG_MSG     pExtRegMsg);

BOOLEAN WscRecvMessageM6(
    IN  PMP_ADAPTER       pAd, 
    IN  PMP_PORT pPort,
    IN  UCHAR               Id,
    IN  PMLME_QUEUE_ELEM    pElem);

BOOLEAN WscRecvMessageM7(
    IN  PMP_ADAPTER       pAd, 
    IN  PMP_PORT          pPort,
    IN  UCHAR               Id,
    IN  PMLME_QUEUE_ELEM    pElem,
    IN  PWSC_EXTREG_MSG     pExtRegMsg);

BOOLEAN WscRecvMessageM8(
    IN  PMP_ADAPTER       pAd, 
    IN  PMP_PORT          pPort,
    IN  UCHAR               Id,
    IN  PMLME_QUEUE_ELEM    pElem);

VOID    WscReceiveEapFail(
    IN  PMP_ADAPTER       pAd, 
    IN  UCHAR               Id,
    IN  PMLME_QUEUE_ELEM    pElem);

VOID WscReceiveEapNack(
    IN  PMP_ADAPTER       pAd, 
    IN  PMP_PORT pPort,
    IN  UCHAR               Id,
    IN  PMLME_QUEUE_ELEM    pElem);

VOID    WpseSendEapFail(
    IN  PMP_ADAPTER       pAd, 
    IN  PMP_PORT pPort,
    IN  UCHAR               Id,
    IN  PMLME_QUEUE_ELEM    pElem);

BOOLEAN WscReceiveEapDone(
    IN  PMP_ADAPTER       pAd, 
    IN  PMP_PORT pPort,
    IN  UCHAR               Id,
    IN  PMLME_QUEUE_ELEM    pElem,
    IN  PWSC_EXTREG_MSG     pExtRegMsg);
    
BOOLEAN WscParseEncrSettings(
    IN  PMP_ADAPTER       pAd, 
    IN  PMP_PORT          pPort,
    IN  PUCHAR              pPlainData,
    IN  INT                 PlainLength);

VOID    WscProcessCredential(
    IN  PMP_ADAPTER       pAd, 
    IN  PMP_PORT pPort,
    IN  PUCHAR              pPlainData,
    IN  INT                 PlainLength);

VOID WscMakeProbeReqIE(
    IN  PMP_ADAPTER   pAd,
    IN  PMP_PORT      pPort,
    IN  BOOLEAN         IsP2pScan,
    OUT PUCHAR          pOutBuf,
    OUT PUSHORT         pIeLen);

VOID WscMakeAssociateReqIE(
    IN  PMP_ADAPTER   pAd,
    IN  PMP_PORT      pPort,
    OUT PUCHAR          pOutBuf,
    OUT PUCHAR          pIeLen);

VOID WscPbcEnrTabUpdate(
    IN  PMP_ADAPTER   pAd,
    IN  PMP_PORT      pPort,
    IN  PUCHAR          Macaddr,
    IN  UCHAR           Version,
    IN  PUCHAR          UUID_E,
    IN  USHORT          DevicePassID,
    IN  ULONGLONG       ProbReqLastTime
);

UCHAR WscPbcEnrTabCheck(
    IN  PMP_ADAPTER   pAd,
    IN  PMP_PORT pPort,
    IN  PUCHAR  Mac
);

VOID    
WscStop(
    IN  PMP_ADAPTER   pAd,
    IN  PMP_PORT    pPort
    );

VOID    
WscTimerStop(
    IN  PMP_ADAPTER pAd,
    IN  PMP_PORT pPort
    );

VOID WscAutoGenProfile( 
    IN  PMP_ADAPTER   pAd,
    IN  PMP_PORT      pPort);

BOOLEAN WscSupportWPSLEDMode(
    IN PMP_ADAPTER pAd);

BOOLEAN WscSupportWPSLEDMode10(
    IN PMP_ADAPTER pAd);

BOOLEAN WscAPHasSecuritySetting(
    IN PMP_ADAPTER pAd,
    IN PMP_PORT pPort);

// AdhocN Support
UCHAR AdhocN_XmitMulticastRateNRateToOFDMRate(
    USHORT uMCS,
    BOOLEAN bBW);

// MFP
VOID PmfStateMachineInit(
    IN  PMP_ADAPTER   pAd,
    IN  STATE_MACHINE   *S,
    OUT STATE_MACHINE_FUNC  Trans[]);

//
// extra prototype in md5.c
//
VOID    SHAInit(SHA_CTX *ctx);

static  VOID    SHAHashBlock(SHA_CTX *ctx);

VOID    SHAUpdate(SHA_CTX *ctx, unsigned char *dataIn, int len);

VOID    SHAFinal(SHA_CTX *ctx, unsigned char hashout[20]);

VOID SHA256_Init (
    IN  SHA256_CTX_STRUC *pSHA_CTX);

VOID SHA256_Hash (
    IN  SHA256_CTX_STRUC *pSHA_CTX);

VOID SHA256_Append (
    IN  SHA256_CTX_STRUC *pSHA_CTX, 
    IN  const UCHAR Message[], 
    IN  UINT MessageLen);

VOID SHA256_End (
    IN  SHA256_CTX_STRUC *pSHA_CTX, 
    OUT UCHAR DigestMessage[]);

VOID SHA256 (
    IN  const UCHAR Message[], 
    IN  UINT MessageLen,
    OUT UCHAR DigestMessage[]);

#ifdef MULTI_CHANNEL_SUPPORT
NDIS_STATUS RTMPDeferRequest(
    IN PMP_ADAPTER pAd,
    IN NDIS_IO_WORKITEM_FUNCTION DeferredCallback);
#endif /*MULTI_CHANNEL_SUPPORT*/

BOOLEAN AsicCheckCommandOk(
    IN PMP_ADAPTER pAd,
    IN UCHAR         Command
    );

// 80211z, tdls.c
VOID TdlsStateMachineInit(
    IN PMP_ADAPTER pAd, 
    IN STATE_MACHINE *Sm, 
    OUT STATE_MACHINE_FUNC Trans[]) ;

VOID TdlsSetupReqAction(
    IN PMP_ADAPTER pAd, 
    IN MLME_QUEUE_ELEM *Elem);

VOID TdlsPeerMsgAction(
    IN PMP_ADAPTER pAd, 
    IN MLME_QUEUE_ELEM *Elem);

VOID TdlsPeerSetupReqAction(
    IN PMP_ADAPTER pAd, 
    IN MLME_QUEUE_ELEM *Elem);

VOID TdlsPeerSetupRspAction(
    IN PMP_ADAPTER pAd, 
    IN MLME_QUEUE_ELEM *Elem); 

VOID TdlsPeerSetupConfAction(
    IN PMP_ADAPTER pAd, 
    IN MLME_QUEUE_ELEM *Elem);

VOID TdlsTearDownAction(
    IN PMP_ADAPTER pAd, 
    IN MLME_QUEUE_ELEM *Elem); 

VOID TdlsPeerTearDownAction(
    IN PMP_ADAPTER pAd, 
    IN MLME_QUEUE_ELEM *Elem);

int TdlsCheckTdlsFrame(
    IN  PMP_ADAPTER   pAd,
    IN  PUCHAR          pDA);

VOID TdlsSendTdlsTearDownFrame(
    IN  PMP_ADAPTER   pAd,
    IN  PRT_802_11_TDLS pTDLS);

VOID TdlsSendPacket(
    IN  PMP_ADAPTER   pAd,
    IN  PVOID           pBuffer,
    IN  ULONG           Length);

VOID TdlsTimeoutTimerCallback(
    IN PVOID SystemSpecific1, 
    IN PVOID FunctionContext, 
    IN PVOID SystemSpecific2, 
    IN PVOID SystemSpecific3);

VOID TdlsTearDown(
    IN PMP_ADAPTER pAd,
    IN BOOLEAN      bDeleteTable);

VOID TdlsDeleteMacTableEntry(
    IN PMP_ADAPTER pAd,
    IN  PMP_PORT  pPort,
    IN PUCHAR pMacAddr);

INT TdlsSearchLinkId(
    IN  PMP_ADAPTER   pAd,
    IN  PUCHAR          pAddr);

USHORT  TdlsTPKMsg1Process(
    IN  PMP_ADAPTER       pAd, 
    PMP_PORT              pPort,
    IN  PRT_802_11_TDLS     pTDLS,
    IN  PUCHAR              pRsnIe, 
    IN  UCHAR               RsnLen, 
    IN  PUCHAR              pFTIe, 
    IN  UCHAR               FTLen, 
    IN  PUCHAR              pTIIe, 
    IN  UCHAR               TILen);

USHORT  TdlsTPKMsg2Process(
    IN  PMP_ADAPTER       pAd, 
    PMP_PORT              pPort,
    IN  PRT_802_11_TDLS     pTDLS,
    IN  PUCHAR              pRsnIe, 
    IN  UCHAR               RsnLen, 
    IN  PUCHAR              pFTIe, 
    IN  UCHAR               FTLen, 
    IN  PUCHAR              pTIIe, 
    IN  UCHAR               TILen,
    OUT PUCHAR              pTPK,
    OUT PUCHAR              pTPKName);

USHORT  TdlsTPKMsg3Process(
    IN  PMP_ADAPTER       pAd, 
    PMP_PORT              pPort,
    IN  PRT_802_11_TDLS     pTDLS,
    IN  PUCHAR              pRsnIe, 
    IN  UCHAR               RsnLen, 
    IN  PUCHAR              pFTIe, 
    IN  UCHAR               FTLen, 
    IN  PUCHAR              pTIIe, 
    IN  UCHAR               TILen);

VOID TdlsDeriveTPK(
    IN  PUCHAR  mac_i,
    IN  PUCHAR  mac_r,
    IN  PUCHAR  a_nonce,
    IN  PUCHAR  s_nonce,
    IN  PUCHAR  bssid,
    OUT PUCHAR  tpk,
    OUT PUCHAR  tpk_name);

VOID
TdlsTableInit(
    IN  PMP_ADAPTER   pAd);

VOID
TdlsSearchTabInit(
    IN PMP_ADAPTER pAd);

VOID
TdlsSearchTabDestory(
    IN PMP_ADAPTER pAd);

PTDLS_SEARCH_ENTRY
TdlsSearchEntyAlloc(
    IN PMP_ADAPTER pAd);

VOID
TdlsSearchEntyFree(
    IN PMP_ADAPTER pAd,
    IN PTDLS_SEARCH_ENTRY pTdlsSearchEntry);

BOOLEAN
TdlsSearchEntryLookup(
    IN PMP_ADAPTER pAd,
    IN PUCHAR pMac);

BOOLEAN
TdlsSearchEntryUpdate(
    IN PMP_ADAPTER pAd,
    IN PUCHAR pMac);

BOOLEAN
TdlsSearchEntryDelete(
    IN PMP_ADAPTER pAd,
    IN PUCHAR pMac);

VOID
TdlsSearchTabMaintain(
    IN PMP_ADAPTER pAd);

VOID TdlsLinkMaintenance(
    IN PMP_ADAPTER pAd);

VOID
TdlsDiscoveryPacketLearnAndCheck(
    IN  PMP_ADAPTER   pAd,
    IN  PUCHAR          pAddr);

VOID RTMPEnableWlan(
    IN PMP_ADAPTER    pAd,
    IN BOOLEAN bOn,
    IN BOOLEAN bResetWLAN);

//
// VHT Operating Mode Notification
//
typedef struct _MLME_VHT_OPERATING_MODE_NOTIFICATION_STRUCT
{
    UCHAR DA[MAC_ADDR_LEN]; // Destination address
    UCHAR ChannelWidth:2; // Channel width (CH_WIDTH_BWXXX)
    UCHAR RxNss:3; // Rx Nss (RX_NSS_XXX)
    UCHAR RxNssType:1; // Rx Nss Type (RX_NSS_TYPE_MAX_RX_NSS_XXX)
} MLME_VHT_OPERATING_MODE_NOTIFICATION_STRUCT, *PMLME_VHT_OPERATING_MODE_NOTIFICATION_STRUCT;

//
// Send the VHT Operating Mode Notification Action
//
VOID MlmeVhtOperatingModeNotificationAction(
    IN PMP_ADAPTER pAd, 
    IN MLME_QUEUE_ELEM *pElem);

//
// Peer-STA VHT Action
//
VOID PeerVHTAction(
    IN PMP_ADAPTER pAd, 
    IN MLME_QUEUE_ELEM *pElem);

//
// Peer-STA VHT Operating Mode Notification Action
//
VOID PeerVHTOperatingModeNotificationAction(
    IN PMP_ADAPTER pAd, 
    IN UCHAR Wcid, // WCID
    IN UCHAR ChannelWidth, // Channel width (CH_WIDTH_BWXXX)
    IN UCHAR RxNss, // Rx Nss (RX_NSS_XXX)
    IN UCHAR RxNssType); // Rx Nss Type (RX_NSS_TYPE_MAX_RX_NSS_XXX)

VOID WhckCPUCheckTimerCallback(
    IN PVOID SystemSpecific1, 
    IN PVOID FunctionContext, 
    IN PVOID SystemSpecific2, 
    IN PVOID SystemSpecific3);

#if 0
VOID    RTPCIInitializeCmdQ(
    IN  PCmdQ   cmdq);

NDIS_STATUS RTPCIEnqueueInternalCmd(
    IN PMP_ADAPTER    pAd,
    IN NDIS_OID         Oid,
    IN PVOID            pInformationBuffer,
    IN ULONG            InformationBufferLength);

VOID    RTPCIDequeueCmd(
    IN  PCmdQ       cmdq,
    OUT PCmdQElmt   *pcmdqelmt);
#endif

#ifdef MULTI_CHANNEL_SUPPORT
VOID MlmeSyncForceToTriggerScanComplete(
    IN  PMP_ADAPTER   pAd);

#endif /*MULTI_CHANNEL_SUPPORT*/

#if _WIN8_USB_SS_SUPPORTED

//
// Functions prototype in init.c
//
NDIS_STATUS N6RegisterSelectiveSuspend(
    NDIS_HANDLE NdisMiniportDriverHandle);
//
// Functions prototype in usb_ss.c
//
BOOLEAN UsbSsInit(
    PMP_ADAPTER pAd);
VOID UsbSsHalt(
    PMP_ADAPTER pAd);

VOID UsbSsIdleRequest(
    PMP_ADAPTER pAd);
VOID UsbSsCancelIdleRequest(
    PMP_ADAPTER pAd);

NDIS_STATUS UsbSsSuspend(
    PMP_ADAPTER pAd);
NDIS_STATUS UsbSsResume(
    PMP_ADAPTER pAd);

#endif // _WIN8_USB_SS_SUPPORTED

UCHAR
MlmeSyncGetWlanIdxByPort(
    IN PMP_PORT            pPort,
    IN USHORT  WlanIdxType
    );

USHORT
MlmeSyncGetRoleTypeByWlanIdx(
    IN PMP_PORT pPort,
    IN UCHAR    WlanIdx
    );

VOID
MlmeSyncInitMacTabPool(
    IN PMP_ADAPTER    pAd
    );

PMAC_TABLE_ENTRY
MlmeSyncInsertMacTabByRoleType(
    IN PMP_PORT         pPort,
    IN UCHAR  WlanIdxType
    );

PMAC_TABLE_ENTRY
MlmeSyncInsertMacTabByWlanIdx(
    IN PMP_PORT         pPort,
    IN UCHAR            WlanIdx
    );
    
VOID
MlmeSyncRemoveMacTab(
    IN PMP_PORT         pPort,
    IN PMAC_TABLE_ENTRY  pEntry
    );

VOID    
MlmeSyncInitMacTab(
    IN PMP_PORT         pPort
    );

PMAC_TABLE_ENTRY
MlmeSyncMacTabMatchedRoleType(
    IN PMP_PORT         pPort,
    IN USHORT  WlanIdxType
    );    

PMAC_TABLE_ENTRY
MlmeSyncMacTabMatchedBssid(
    IN PMP_PORT         pPort,
    IN PUCHAR  pBssid
    );

PMAC_TABLE_ENTRY
MlmeSyncMacTabMatchedWlanIdx(
    IN PMP_PORT         pPort,
    IN UCHAR  Wcid
    );
    
VOID
MlmeSyncMacTableDumpInfo(
    IN PMP_PORT         pPort
    );
    
#endif  // __RTMP_H__

