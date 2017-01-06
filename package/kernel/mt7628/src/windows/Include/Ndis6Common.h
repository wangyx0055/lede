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
    Ndis6Common.h

    Abstract:
    Miniport generic portion header file

    Revision History:
    Who         When          What
    --------    ----------    ----------------------------------------------
    */

#ifndef __NDIS6COMMON_H__
#define __NDIS6COMMON_H__

#include "PmfCmm.h"

extern DOT11_AUTH_CIPHER_PAIR BSSUnicastAuthCipherPairs[];
extern USHORT uBSSUnicastAuthCipherPairsSize;
extern DOT11_AUTH_CIPHER_PAIR IBSSUnicastAuthCipherPairs[];
extern USHORT uIBSSUnicastAuthCipherPairsSize;
extern DOT11_AUTH_CIPHER_PAIR BSSMulticastAuthCipherPairs[]; 
extern USHORT uBSSMulticastAuthCipherPairsSize;
extern DOT11_AUTH_CIPHER_PAIR IBSSMulticastAuthCipherPairs[];
extern USHORT uIBSSMulticastAuthCipherPairsSize;

#define MAX_BEACON_PERIOD       1024
#define LIMIT_BEACON_PERIOD(_Period)        \
    if((_Period)>=MAX_BEACON_PERIOD)        \
        (_Period)=MAX_BEACON_PERIOD-1;

NDIS_STATUS 
Ndis6CommonAllocAdapterBlock(
    IN NDIS_HANDLE MiniportAdapterHandle,
    OUT PMP_ADAPTER   *ppAd
    );

VOID
Ndis6CommonFillReceiveContext(
    IN  PMP_ADAPTER  pAd,
    IN  PMP_PORT      pPort,
    IN OUT PDOT11_EXTSTA_RECV_CONTEXT Dot11RecvContext,
    IN ULONGLONG TimeStamp
    );

VOID  
Ndis6CommonPortFreeVariableIE(
    IN PMP_PORT pPort
    );

VOID 
Ndis6CommonPortFreePrivacyExemptionList(
    IN PMP_PORT pPort
    );

VOID    
Ndis6CommonRejectPendingPackets(
    IN  PMP_ADAPTER   pAd
    );

VOID
Ndis6CommonFreePortData(
    IN  PMP_ADAPTER   pAd
    );    

PMT_XMIT_CTRL_UNIT 
Ndis6CommonAllocateNdisNetBufferList(
    IN  PMP_ADAPTER       pAd,
    IN  ULONG                PortNumber,
    IN  PUCHAR              pHeader,
    IN  ULONG               DataLen
    );    

NDIS_STATUS 
Ndis6CommonPortDeferOidRequest(
    IN PMP_ADAPTER pAd,
    IN PMP_PORT pPort,
    IN PNDIS_OID_REQUEST pNdisOidRequest,
    IN NDIS_IO_WORKITEM_FUNCTION DeferredCallback
    );

NDIS_STATUS 
Ndis6CommonPortCreateMac(
    IN PMP_ADAPTER pAd,
    IN PMP_PORT pPort,
    IN PNDIS_OID_REQUEST pNdisOidRequest
    );

VOID 
Ndis6CommonPortCreateMacHandler(
    IN PVOID WorkItemContext,
    IN NDIS_HANDLE NdisIoWorkItemHandle
    );

NDIS_STATUS 
Ndis6CommonPortDeleteMac(
    IN PMP_ADAPTER pAd,
    IN PMP_PORT pPort,
    IN PNDIS_OID_REQUEST pNdisOidRequest
    );

VOID 
Ndis6CommonPortDeleteMacHandler(
    IN PVOID WorkItemContext,
    IN NDIS_HANDLE NdisIoWorkItemHandle
    );

VOID 
Ndis6CommonGetNewMacAddress(
    IN  PMP_ADAPTER   pAd,
    IN  UCHAR           MultiBssidMode,
    IN  UCHAR           PortNumber,
    OUT PUCHAR          pMacAddr
    );

UCHAR 
Ndis6CommonGetBssidIndex(
    IN  PMP_ADAPTER   pAd,
    IN  PMP_PORT      pPort,
    IN  UCHAR           MultiBssidMode
    );

NDIS_STATUS 
Ndis6CommonPortSetNicPowerState(
    IN PMP_ADAPTER pAd,
    IN PMP_PORT pPort,
    IN PNDIS_OID_REQUEST pNdisOidRequest
    );

VOID 
Ndis6CommonPortSetNicPowerStateHandler(
    IN PVOID WorkItemContext,
    IN NDIS_HANDLE NdisIoWorkItemHandle
    );    

PMP_PORT
Ndis6CommonGetStaPort(
    IN PMP_ADAPTER pAd
    );

VOID
Ndis6CommonCheckanyPortConnected(
    IN  PMP_ADAPTER pAd,
    OUT NDIS_PORT_NUMBER  *PortNumber, 
    OUT BOOLEAN *bConnected
    );    

NDIS_STATUS
Ndis6CommonSetDesiredPhyIdList(
    IN PMP_ADAPTER pAd,
    IN PMP_PORT     pPort,
    IN PULONG       PhyIDList,
    IN ULONG        PhyIDCount
    ); 

NDIS_STATUS
Ndis6CommonQueryStatistics(
    IN PMP_ADAPTER pAd,
    IN NDIS_OID         Oid,
    IN OUT PVOID        StatBuffer,
    IN OUT PULONG       BytesWritten
    );

NDIS_STATUS
Ndis6CommonQuerySupportedPHYTypes(
    IN PMP_ADAPTER pAd,
    IN  ULONG uNumMaxEntries,
    OUT PDOT11_SUPPORTED_PHY_TYPES pDot11SupportedPhyTypes
    );    

NDIS_STATUS
Ndis6CommonQueryCurrentSupportedPHYTypes(
    IN PMP_ADAPTER pAd,
    IN PMP_PORT     pPort,
    IN  ULONG uNumMaxEntries,
    OUT PDOT11_SUPPORTED_PHY_TYPES pDot11SupportedPhyTypes
    );    

VOID
Ndis6CommonReportEthFrameToLLC(
    IN  PMP_ADAPTER   pAd,
    IN  PMP_PORT      pPort,
    IN  PUCHAR          p80211DataFrame,
    IN  USHORT          us80211FrameSize,
    IN  UCHAR           UserPriority,
    IN  BOOLEAN         CRCError,
    IN  BOOLEAN         bSingleIndicate
    );

VOID
Ndis6CommonReportEthFrameToLLCAgg(
    IN  PMP_ADAPTER  pAd,
    IN  PMP_PORT  pPort,
    IN  PUCHAR  p80211Header,
    IN  USHORT  Payload1Size,
    IN  USHORT  Payload2Size,
    IN  UCHAR   UserPriority
    );

VOID
Ndis6CommonReportAMSDUEthFrameToLLC(
    IN  PMP_ADAPTER  pAd,
    IN  PMP_PORT      pPort,
    IN  PUCHAR          p80211DataFrame,
    IN  USHORT          us80211FrameSize,
    IN  UCHAR           UserPriority,
    IN  BOOLEAN         bSingleIndicate
    );
    
#endif