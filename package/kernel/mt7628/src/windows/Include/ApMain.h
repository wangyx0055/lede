/*
 ***************************************************************************
 * Ralink Tech Inc.
 * 4F, No. 2 Technology 5th Rd.
 * Science-based Industrial Park
 * Hsin-chu, Taiwan, R.O.C.
 *
 * (c) Copyright 2002, Ralink Technology, Inc.
 *
 * All rights reserved. Ralink's source code is an unpublished work and the
 * use of a copyright notice does not imply otherwise. This source code
 * contains confidential trade secret material of Ralink Tech. Any attemp
 * or participation in deciphering, decoding, reverse engineering or in any
 * way altering the source code is stricitly prohibited, unless the prior
 * written consent of Ralink Technology, Inc. is obtained.
 ***************************************************************************

    Module Name:
    ap.h

    Abstract:
    Miniport generic portion header file

    Revision History:
    Who         When          What
    --------    ----------    ----------------------------------------------
    Paul Lin    08-01-2002    created
    James Tan   09-06-2002    modified (Revise NTCRegTable)
    John Chang  12-22-2004    modified for RT2561/2661. merge with STA driver
*/
#ifndef __APMAIN_H__
#define __APMAIN_H__

/** Define AP states */
typedef enum _AP_STATE
{
    AP_STATE_STOPPED = 0,                   // AP INIT state
    AP_STATE_STARTING,
    AP_STATE_STARTED,
    AP_STATE_STOPPING,
    AP_STATE_INVALID = 0xFFFFFFFF           // When MP is not in AP mode
} AP_STATE, *PAP_STATE;

AP_STATE
FORCEINLINE
ApGetState(
    __in PMP_PORT pPort)
{
    return (AP_STATE)InterlockedExchangeAdd(
                (LONG *)&pPort->ApState,
                0);
}
/** Set AP state, return the old state */
AP_STATE
FORCEINLINE
ApSetState(
    __in PMP_PORT pPort,
    __in AP_STATE   NewState)
{
    return (AP_STATE)InterlockedExchange(
                (LONG *)&pPort->ApState,
                (LONG)NewState);
}

#define VALIDATE_AP_INIT_STATE(ApPort) \
    { \
        if (ApGetState(ApPort) != AP_STATE_STOPPED) \
        { \
            ndisStatus = NDIS_STATUS_INVALID_STATE; \
            break; \
        } \
    }

#define VALIDATE_AP_OP_STATE(ApPort) \
    { \
        if (ApGetState(ApPort) != AP_STATE_STARTED) \
        { \
            ndisStatus = NDIS_STATUS_INVALID_STATE; \
            break; \
        } \
    }
// =============================================================
//      Function Prototypes
// =============================================================

////
// Ap.c
//

NDIS_STATUS APInitialize(
    IN  PMP_ADAPTER   pAd);

VOID APStartUp(
    IN PMP_ADAPTER pAd,
    IN PMP_PORT   pPort);

VOID APStop(
    IN PMP_ADAPTER pAd,
    IN PMP_PORT pPort) ;

VOID APShutdown(
    IN PMP_ADAPTER pAd);

VOID 
APCleanupPsQueue(
    IN  PMP_ADAPTER   pAd,
    IN  PQUEUE_HEADER   pQueue
    );

VOID ApLogEvent(
    IN PMP_ADAPTER pAd,
    IN PUCHAR   pAddr,
    IN USHORT   Event);

VOID APUpdateOperationMode(
    IN PMP_ADAPTER pAd);

VOID APUpdateCapabilityAndErpIe(
    IN PMP_ADAPTER pAd);

BOOLEAN ApCheckAccessControlList(
    IN PMP_ADAPTER pAd,
    IN PUCHAR        pAddr);

MAC_TABLE_ENTRY *APSsPsInquiry(
    IN  PMP_ADAPTER   pAd, 
    IN  PMP_PORT      pPort,    
    IN  PUCHAR pAddr, 
    OUT SST   *Sst, 
    OUT UCHAR *Aid,
    OUT UCHAR *PsMode,
    OUT UCHAR *Rate);

UCHAR APPsIndicate(
    IN PMP_ADAPTER pAd, 
    IN PMP_PORT   pPort,
    IN PUCHAR pAddr, 
    IN ULONG Wcid, 
    IN UCHAR Rssi,
    IN UCHAR Psm);

VOID MacTableReset(
    IN  PMP_ADAPTER  pAd,
    IN NDIS_PORT_NUMBER PortNum);

MAC_TABLE_ENTRY *MacTableInsertEntry(
    IN  PMP_ADAPTER   pAd, 
    IN PMP_PORT      pPort,
    IN  PUCHAR pAddr,
    IN BOOLEAN  CleanAll
    ); 

USHORT
GetWlanRoleType(
    IN PMP_PORT        pPort
    );

BOOLEAN 
MacTableDeleteAndResetEntry(
    IN PMP_ADAPTER pAd, 
    IN PMP_PORT   pPort,
    IN UCHAR wcid,
    IN PUCHAR pAddr,
    IN BOOLEAN bForcedDelete
    );

MAC_TABLE_ENTRY *MacTableLookup(
    IN  PMP_ADAPTER pAd, 
    IN  PMP_PORT    pPort,
    IN  PUCHAR  pAddr
    );

VOID MacTableMaintenance(
    IN PMP_ADAPTER pAd,
    IN PMP_PORT pPort);

//
// Are all STAs RDG capable
//
// Parameters
//  pAd: The adapter data structure
//
// Return Value:
//  TRUE: All STAs RDG capable.
//  FALSE: At least one STA is not RDG capable.
//
BOOLEAN AreAllStaRdgCapable(
    IN PMP_ADAPTER pAd);

VOID ApUpdateAccessControlList(
    IN PMP_ADAPTER pAd);

VOID APIndicateAssociationStatus(
    IN PMP_ADAPTER    pAd,
    IN PUCHAR           pAddr);

VOID
APPlatformIndicateDisassociationStatus(
    IN PMP_ADAPTER    pAd,
    IN PMP_PORT       pPort,
    IN PUCHAR           pAddr,
    IN ULONG            Reason);

VOID ApPlatformIndicateDot11Status(
    IN  PMP_ADAPTER   pAd,
    IN  NDIS_PORT_NUMBER          PortNumber, 
    IN  NDIS_STATUS     StatusCode,
    IN  PVOID           RequestID,
    IN  PVOID           pStatusBuffer,
    IN  ULONG           StatusBufferSize);

VOID APPrepareAssocCompletePara(
    IN  PMP_ADAPTER   pAd,
    IN  PMAC_TABLE_ENTRY pEntry,
    IN  PUCHAR pAddr,
    IN  ULONG Status,
    IN  UCHAR ErrorSource,
    IN  BOOLEAN Reassociation,
    OUT PUCHAR RequestFrame,
    IN  USHORT RequestFrameSize,
    OUT PUCHAR ResponseFrame,
    IN  USHORT ResponseFrameSize,
    OUT PDOT11_INCOMING_ASSOC_COMPLETION_PARAMETERS AssocCompletePara,
    OUT PULONG AssocCompleteParaSize);

VOID APPrepareAssocStartPara(
    IN PUCHAR pAddr,
    OUT PDOT11_INCOMING_ASSOC_STARTED_PARAMETERS AssocStartPara);

NDIS_STATUS APAllocAssocCompletePara(
    IN  PMP_ADAPTER   pAd,
    IN  USHORT          AssocReqFrameSize,
    IN  USHORT          AssocRespFrameSize,
    OUT PDOT11_INCOMING_ASSOC_COMPLETION_PARAMETERS *AssocCompletePara,
    OUT PULONG          AssocCompleteParaSize
    );

VOID APPrepareDisassocPara(
    IN  PMP_ADAPTER   pAd,
    IN  PUCHAR  pAddr,
    IN  ULONG   Reason,
    OUT PDOT11_DISASSOCIATION_PARAMETERS    DisassocPara
    );

VOID APSetDefaultCipher(
    IN PMP_PORT   pPort);

////
// Ap_assoc.c
//

VOID ApMlmeAssocStateMachineInit(
    IN  PMP_ADAPTER   pAd, 
    IN  STATE_MACHINE *S, 
    OUT STATE_MACHINE_FUNC Trans[]);

VOID ApAssocPeerAssocReqAction(
    IN PMP_ADAPTER pAd, 
    IN MLME_QUEUE_ELEM *Elem); 

VOID ApAssocPeerReassocReqAction(
    IN PMP_ADAPTER pAd, 
    IN MLME_QUEUE_ELEM *Elem);

VOID ApAssocPeerDisassocReqAction(
    IN PMP_ADAPTER pAd, 
    IN MLME_QUEUE_ELEM *Elem);

VOID ApAssocMlmeDisassocReqAction(
    IN PMP_ADAPTER pAd, 
    IN MLME_QUEUE_ELEM *Elem);

VOID ApAssocMlmeAssocCls3errAction(
    IN PMP_ADAPTER pAd, 
    IN PUCHAR        pAddr); 

VOID ApAssocGoBuildAssociation(
    IN PMP_ADAPTER pAd,
    IN PMP_PORT   pPort,
    IN MAC_TABLE_ENTRY *pEntry,
    IN UCHAR        *RSN,
    IN UCHAR        *pRSNLen,
    IN UCHAR        *pP2pElement,
    IN UCHAR        P2pElementLen,
    IN UCHAR        *pWpsElement,
    IN UCHAR        WpsElementLen,
    IN HT_CAPABILITY_IE     *pHtCapability,
    IN UCHAR        HtCapabilityLen);

USHORT APBuildAssociation(
    IN PMP_ADAPTER pAd,
    IN NDIS_PORT_NUMBER PortNumber,
    IN MAC_TABLE_ENTRY *pEntry,
    IN USHORT        CapabilityInfo,
    IN UCHAR         MaxSupportedRateIn500Kbps,
    IN UCHAR         *RSN,
    IN UCHAR         *pRSNLen,
    IN BOOLEAN       bWmmCapable,
    IN ULONG         ClientRalinkIe,
    IN UCHAR        *pP2pElement,
    IN UCHAR         P2pElementLen,
    IN UCHAR        *pWpsElement,
    IN UCHAR         WpsElementLen,
    IN HT_CAPABILITY_IE     *pHtCapability,
    IN UCHAR         HtCapabilityLen,
    IN VHT_CAP_IE    *pVhtCapability,
    IN UCHAR         VhytCapabilityLen,
    OUT USHORT       *pAid);


////
// Ap_auth.c
//

void ApMlmeAuthStateMachineInit(
    IN PMP_ADAPTER pAd, 
    IN STATE_MACHINE *Sm, 
    OUT STATE_MACHINE_FUNC Trans[]); 

VOID ApAuthDeauthReqAction(
    IN PMP_ADAPTER pAd, 
    IN MLME_QUEUE_ELEM *Elem); 

VOID ApAuthGoDeauthReqAction(
    IN PMP_ADAPTER pAd, 
    IN NDIS_PORT_NUMBER PortNum,
    IN MLME_DEAUTH_REQ_STRUCT *pInfo);

VOID ApAuthMlmeAuthCls2errAction(
    IN PMP_ADAPTER pAd, 
    IN PMP_PORT   pPort,
    IN PUCHAR pAddr); 

////
// Ap_authrsp.c
//

VOID ApMlmeAuthRspStateMachineInit(
    IN PMP_ADAPTER pAd, 
    IN PSTATE_MACHINE Sm, 
    IN STATE_MACHINE_FUNC Trans[]); 

VOID ApAuthPeerAuthAtAuthRspIdleAction(
    IN PMP_ADAPTER pAd, 
    IN PMLME_QUEUE_ELEM Elem); 

VOID ApAuthPeerDeauthReqAction(
    IN PMP_ADAPTER pAd, 
    IN PMLME_QUEUE_ELEM Elem); 

VOID ApAuthPeerAuthSimpleRspGenAndSend(
    IN PMP_ADAPTER pAd, 
    IN PMP_PORT pPort, 
    IN PHEADER_802_11 pHdr, 
    IN USHORT Alg, 
    IN USHORT Seq, 
    IN USHORT StatusCode); 

////
// Ap_connect.c
//
VOID ApConnectUpdateBeaconFrame(
    IN PMP_ADAPTER pAd,
    IN PMP_PORT   pPort); 

////
// Ap_data.c
//

NDIS_STATUS 
ApDataSendPacket(
    IN  PMP_ADAPTER       pAd,
    IN  NDIS_PORT_NUMBER    PortNumber,
    IN  PMT_XMIT_CTRL_UNIT             pXcu
    );

NDIS_STATUS 
ApDataHardTransmit(
    IN  PMP_ADAPTER   pAd,
    IN  PMT_XMIT_CTRL_UNIT     pXcu
    );

NTSTATUS    ApDataN6RxPacket(
    IN  PMP_ADAPTER   pAd,
    IN    BOOLEAN           bBulkReceive,
    IN  ULONG           nIndex);

VOID
ApDataCommonRxPacket(
    IN  PMP_ADAPTER   pAd,
    IN  PMP_PORT      pPort,
    IN  PRXDSCR_BASE_STRUC  pRxD,
    IN  PHEADER_802_11  pHeader,
    IN  PUCHAR          pData,
    IN  USHORT              PacketSize,
    IN  UCHAR               QosPadding,
    IN  PUCHAR             pQoSField,
    IN  struct rx_signal_info *rx_signal
    );

NDIS_STATUS ApDataCheckRxError(
    IN  PMP_ADAPTER   pAd,
    IN  PRXDSCR_BASE_STRUC  pRxD,
    IN  PHEADER_802_11  pHeader);

BOOLEAN ApDataCheckClass2Class3Error(
    IN  PMP_ADAPTER   pAd,
    IN  PMP_PORT      pPort,
    IN  ULONG           Wcid, 
    IN  PHEADER_802_11  pHeader);

VOID ApDataHandleRxPsPoll(
    IN  PMP_ADAPTER   pAd,
    IN  PUCHAR          pAddr,
    IN  USHORT          Aid,
#if (UAPSD_AP_SUPPORT == 0) 
    IN  BOOLEAN     IsPsPoll);
#else
    IN  BOOLEAN     IsPsPoll,
    IN  PMP_PORT      pPort,
    IN  BOOLEAN     isActive);
#endif  

BOOLEAN ApDataBridgeToWdsAndWirelessSta(
    IN  PMP_ADAPTER   pAd,
    IN  PMP_PORT  pPort,
    IN  PHEADER_802_11  pHeader80211,
    IN  UINT            us80211FreamSize,
    IN  ULONG           fromwdsidx,
    IN  UCHAR           UserPriority);

VOID ApDataCheckNDSClient(
    IN  PMP_ADAPTER       pAd, 
    IN  PMAC_TABLE_ENTRY    pEntry,
    IN  PRXWI_STRUC         pRxWI);

VOID ApDataTbttTimeoutTimerCallback(
    IN PVOID SystemSpecific1, 
    IN PVOID FunctionContext, 
    IN PVOID SystemSpecific2, 
    IN PVOID SystemSpecific3);

////
// Ap_info.c
//

NDIS_STATUS
ApSetDesiredSsidList(
    IN  PMP_ADAPTER           pAd,
    IN  PMP_PORT              pPort,  
    IN  PDOT11_SSID_LIST        SsidList);

NDIS_STATUS
ApOidSetDesiredPhyList(
    IN  PMP_ADAPTER           pAd,
    IN    NDIS_PORT_NUMBER          PortNumber,    
    IN    PVOID                     InformationBuffer,
    IN    ULONG                     InformationBufferLength,
    OUT   PULONG                    BytesRead,
    OUT   PULONG                    BytesNeeded);

BOOLEAN
ApValidateUnicastAuthCipherPair(
    IN DOT11_AUTH_ALGORITHM AuthAlgo,
    IN DOT11_CIPHER_ALGORITHM   CipherAlgo);

BOOLEAN
ApValidateMulticastAuthCipherPair(
    IN DOT11_AUTH_ALGORITHM AuthAlgo,
    IN DOT11_CIPHER_ALGORITHM CipherAlgo);

VOID
APCleanupPrivacyExemptionList(
    IN PMP_ADAPTER pAd);
    
NDIS_STATUS RTMPApAddKey(
    IN  PMP_ADAPTER   pAd,
    IN  PMP_PORT      pPort,
    IN  PUCHAR          MacAddr,
    IN  UCHAR           KeyIdx,
    IN  DOT11_CIPHER_ALGORITHM AlgorithmId,
    IN  ULONG           KeyLen,
    IN  PUCHAR          KeyValue,
    IN  BOOLEAN         bDelete,
    IN  BOOLEAN         bPairwise,
    IN  BOOLEAN         bTxKey);

NDIS_STATUS ApOidDisassociatePeerRequest(
    IN  PMP_ADAPTER       pAd,
    IN    NDIS_PORT_NUMBER  PortNumber,    
    IN    PVOID             InformationBuffer,
    IN    ULONG             InformationBufferLength,
    OUT   PULONG            BytesRead,
    OUT   PULONG            BytesNeeded);

VOID RTMPAPMakeRSNIE(
    IN  PMP_ADAPTER   pAd,
    IN  PMP_PORT      pPort,
    IN  UINT            AuthMode,
    IN  UINT            WepStatus);

VOID  APRemoveAdditionalIE(
    PMP_PORT pPort);

VOID  ApInfoResetHandler(
    PMP_PORT pPort);

NDIS_STATUS
ApInfoResetRequest(
    IN  PMP_ADAPTER       pAd,
    IN  NDIS_PORT_NUMBER    PortNumber,
    IN  OUT PVOID           InformationBuffer,
    IN  ULONG               InputBufferLength,
    IN  ULONG               OutputBufferLength,
    OUT PULONG              BytesRead,
    OUT PULONG              BytesWritten,
    OUT PULONG              BytesNeeded);

NDIS_STATUS
ApSetAdditionalIe(
    IN  PMP_PORT pPort,
    IN  PDOT11_ADDITIONAL_IE AdditionalIe);

NDIS_STATUS
ValiateAdditionalIeSize(
    IN  PDOT11_ADDITIONAL_IE    IeData,
    IN  ULONG                   IeSize,
    OUT PULONG                  SizeNeeded);

VOID 
RTMPApIndicateFrequencyAdoped(
    IN PMP_PORT ApPort
    );

////
// Ap_mlme.c
//

VOID ApMlmePeriodicExec(
    IN PMP_ADAPTER pAd,
    IN PMP_PORT pPort);

VOID ApCheckMcastpsmData(   
    IN PMP_ADAPTER pAd
);

VOID ApMlmeSelectTxRateTable(
    IN PMP_ADAPTER        pAd,
    IN PMP_PORT           pPort,
    IN PMAC_TABLE_ENTRY     pEntry,
    IN PUCHAR               *ppTable,
    IN PUCHAR               pTableSize,
    IN PUCHAR               pInitTxRateIdx);

VOID ApMlmeSetTxRate(
    IN PMP_ADAPTER        pAd,
    IN PMAC_TABLE_ENTRY     pEntry,
    IN PRTMP_TX_RATE_SWITCH pTxRate,
    IN PUCHAR               pTable);

VOID ApMlmeDynamicTxRateSwitching(
    IN PMP_ADAPTER pAd,
    IN PMP_PORT   pPort, 
    IN TX_STA_CNT0_STRUC TxStaCnt0,
    IN TX_STA_CNT1_STRUC StaTx1);

//
// The dynamic Tx rate switching for AGS (Adaptive Group Switching)
//
// Parameters
//  pAd: The adapter data structure
//  pPort: The port data structure
//  pEntry: Pointer to a caller-supplied variable in which points to a MAC table entry
//  pTable: Pointer to a caller-supplied variable in wich points to a Tx rate switching table
//  TableSize: The size, in bytes, of the specified Tx rate switching table
//  pAGSStatisticsInfo: Pointer to a caller-supplied variable in which points to the statistics information
//  InitTxRateIdx: The initial Tx rate index (3x3: 0x18, 2x2: 0x0F and 1x1: 0x07)
//
// Return Value:
//  None
//
VOID ApMlmeDynamicTxRateSwitchingAGS(
    IN PMP_ADAPTER pAd, 
    IN PMP_PORT     pPort,
    IN PMAC_TABLE_ENTRY pEntry, 
    IN PUCHAR pTable, 
    IN UCHAR TableSize, 
    IN PAGS_STATISTICS_INFO pAGSStatisticsInfo, 
    IN UCHAR InitTxRateIdx);

//
// The dynamic Tx rate switching for AGS in VHT(Adaptive Group Switching)
//
// Parameters
//  pAd: The adapter data structure
//  pEntry: Pointer to a caller-supplied variable in which points to a MAC table entry
//  pTable: Pointer to a caller-supplied variable in wich points to a Tx rate switching table
//  TableSize: The size, in bytes, of the specified Tx rate switching table
//  pAGSStatisticsInfo: Pointer to a caller-supplied variable in which points to the statistics information
//
// Return Value:
//  None
//
VOID ApMlmeDynamicTxRateSwitchingAGSv2(
    IN PMP_ADAPTER pAd, 
    IN PMP_PORT     pPort,
    IN PMAC_TABLE_ENTRY pEntry, 
    IN PUCHAR pTable, 
    IN UCHAR TableSize, 
    IN PAGS_STATISTICS_INFO pAGSStatisticsInfo,
    IN UCHAR InitTxRateIdx);

//
// Auto Tx rate faster train up/down for AGS (Adaptive Group Switching)
//
// Parameters
//  pAd: The adapter data structure
//  pPort: The port data structure
//  pEntry: Pointer to a caller-supplied variable in which points to a MAC table entry
//  pTable: Pointer to a caller-supplied variable in wich points to a Tx rate switching table
//  TableSize: The size, in bytes, of the specified Tx rate switching table
//  pAGSStatisticsInfo: Pointer to a caller-supplied variable in which points to the statistics information
//  InitTxRateIdx: The initial Tx rate index (3x3: 0x18, 2x2: 0x0F and 1x1: 0x07)
//
// Return Value:
//  None
//
VOID ApMlmeQuickResponeForRateUpExecAGS(
    IN PMP_ADAPTER pAd, 
    IN PMAC_TABLE_ENTRY pEntry, 
    IN PUCHAR pTable, 
    IN UCHAR TableSize, 
    IN PAGS_STATISTICS_INFO pAGSStatisticsInfo, 
    IN UCHAR InitTxRateIdx);

VOID ApMlmeQuickResponeForRateUpExecAGSv2(
    IN PMP_ADAPTER pAd, 
    IN PMAC_TABLE_ENTRY pEntry, 
    IN PUCHAR pTable, 
    IN UCHAR TableSize, 
    IN PAGS_STATISTICS_INFO pAGSStatisticsInfo,
    IN UCHAR InitTxRateIdx);

VOID ApMlmeQuickResponeForRateUpExec(
    IN PMP_ADAPTER pAd,
    IN PMP_PORT   pPort);

BOOLEAN ApMlmeMsgTypeSubst(
    IN PMP_ADAPTER pAd,
    IN  PMP_PORT    pPort,
    IN PFRAME_802_11 pFrame, 
    OUT INT *Machine, 
    OUT INT *MsgType);

VOID ApMlmeSetPiggyBack(
    IN PMP_ADAPTER    pAd,
    IN BOOLEAN          bPiggyBack);

VOID ApMlmeAsicEvaluateRxAnt(
    IN PMP_ADAPTER    pAd);
    
VOID ApMlmeAsicRxAntEvalTimeout(
    PMP_ADAPTER   pAd); 

VOID ApMlmeSetSupportedLegacyMCS(
    IN PMAC_TABLE_ENTRY     pEntry);

////
// Ap_sanity.c
//

BOOLEAN ApSanityPeerAssocReqSanity(
    IN PMP_ADAPTER pAd, 
    IN NDIS_PORT_NUMBER PortNumber,
    IN VOID *Msg, 
    IN ULONG MsgLen, 
    OUT PUCHAR pAddr2,
    OUT USHORT *pCapabilityInfo, 
    OUT USHORT *pListenInterval, 
    OUT UCHAR *pSsidLen,
    OUT char *Ssid,
    OUT UCHAR *pRatesLen,
    OUT UCHAR Rates[],
    OUT UCHAR *RSN,
    OUT UCHAR *pRSNLen,
    OUT BOOLEAN *pbWmmCapable,
    OUT ULONG  *pRalinkIe,
    OUT ULONG *WpsLen, 
    OUT PUCHAR pWps, 
    OUT ULONG *P2PSubelementLen, 
    OUT PUCHAR pP2pSubelement, 
    OUT UCHAR        *pHtCapabilityLen,
    OUT HT_CAPABILITY_IE *pHtCapability,
    OUT UCHAR            *pVhtCapabilityLen,
    OUT VHT_CAP_IE       *pVhtCapability);

BOOLEAN ApSanityPeerReassocReqSanity(
    IN  PMP_ADAPTER   pAd, 
    IN NDIS_PORT_NUMBER PortNumber,
    IN  VOID *Msg, 
    IN  ULONG MsgLen, 
    OUT PUCHAR pAddr2, 
    OUT USHORT *pCapabilityInfo, 
    OUT USHORT *pListenInterval, 
    OUT PUCHAR pApAddr,
    OUT UCHAR *pSsidLen,
    OUT char *Ssid,
    OUT UCHAR *pRatesLen,
    OUT UCHAR Rates[],
    OUT UCHAR *RSN,
    OUT UCHAR *pRSNLen,
    OUT BOOLEAN *pbWmmCapable,
    OUT ULONG *pRalinkIe,
    OUT ULONG *WpsLen, 
    OUT PUCHAR pWps, 
    OUT ULONG *P2PSubelementLen, 
    OUT PUCHAR pP2pSubelement, 
    OUT UCHAR        *pHtCapabilityLen,
    OUT HT_CAPABILITY_IE *pHtCapability,
    OUT UCHAR        *pVhtCapabilityLen,
    OUT VHT_CAP_IE *pVhtCapability);

BOOLEAN ApSanityPeerDisassocReqSanity(
    IN PMP_ADAPTER pAd, 
    IN VOID *Msg, 
    IN ULONG MsgLen, 
    OUT PUCHAR pAddr2, 
    OUT USHORT *Reason); 

BOOLEAN ApSanityPeerDeauthReqSanity(
    IN PMP_ADAPTER pAd, 
    IN VOID *Msg, 
    IN ULONG MsgLen, 
    OUT PUCHAR pAddr2, 
    OUT USHORT *Reason);

BOOLEAN ApSanityPeerAuthSanity(
    IN PMP_ADAPTER pAd, 
    IN VOID *Msg, 
    IN ULONG MsgLen, 
    OUT PUCHAR pAddr, 
    OUT USHORT *Alg, 
    OUT USHORT *Seq, 
    OUT USHORT *Status, 
    CHAR *ChlgText); 

BOOLEAN ApSanityPeerProbeReqSanity(
    IN PMP_ADAPTER pAd, 
    IN VOID *Msg, 
    IN ULONG MsgLen, 
    OUT PUCHAR pAddr2,
    OUT CHAR Ssid[], 
    OUT UCHAR *SsidLen); 

BOOLEAN ApSanityMlmeSyncPeerBeaconAndProbeRspSanity(
    IN PMP_ADAPTER pAd, 
    IN PMP_PORT    pPort,
    IN VOID *Msg, 
    IN ULONG MsgLen, 
    OUT PUCHAR pAddr2, 
    OUT PUCHAR pBssid, 
    OUT CHAR Ssid[], 
    OUT UCHAR *SsidLen, 
    OUT UCHAR *BssType, 
    OUT USHORT *BeaconPeriod, 
    OUT UCHAR *Channel, 
    OUT LARGE_INTEGER *Timestamp, 
    OUT USHORT *CapabilityInfo, 
    OUT UCHAR Rate[], 
    OUT UCHAR *RateLen,
    OUT UCHAR    *pHtCapabilityLen,
    OUT UCHAR   *pVHtCapabilityLen,
    OUT BOOLEAN *ExtendedRateIeExist,
    OUT UCHAR *Erp);

////
// Ap_sync.c
//

VOID ApMlmeSyncStateMachineInit(
    IN PMP_ADAPTER pAd, 
    IN STATE_MACHINE *Sm, 
    OUT STATE_MACHINE_FUNC Trans[]); 

VOID ApSyncMlmeSyncPeerProbeRspAction(
    IN PMP_ADAPTER pAd, 
    IN MLME_QUEUE_ELEM *Elem);

VOID ApSyncMlmeSyncPeerProbeReqAction(
    IN PMP_ADAPTER pAd, 
    IN MLME_QUEUE_ELEM *Elem); 

VOID ApSyncMlmeSyncPeerBeaconAction(
    IN PMP_ADAPTER pAd, 
    IN MLME_QUEUE_ELEM *Elem); 

VOID APABandGet40MChannel( 
    IN UCHAR inChannel, 
    IN UCHAR *ExtOffset);

////
// ap_wpa.c
//

VOID ApWpaStateMachineInit(
    IN  PMP_ADAPTER   pAd, 
    IN  STATE_MACHINE *Sm, 
    OUT STATE_MACHINE_FUNC Trans[]);

VOID ApWpaRetryExecTimerCallback(
    IN PVOID SystemSpecific1, 
    IN PVOID FunctionContext, 
    IN PVOID SystemSpecific2, 
    IN PVOID SystemSpecific3);

VOID 
ApRestartAPHandler(
    IN PMP_ADAPTER pAd, 
    IN PMP_PORT   pPort
    );

VOID 
ApStartApRequestHandler(
    IN  PMP_ADAPTER pAd,
    IN  PMP_PORT pPort
    );

//
// TODO: Remove this API once Unify code base supports multiple BSSID.
//
VOID ApAsicEnableBssSync(MP_ADAPTER *pAd);
    
#endif  // __AP_H__

