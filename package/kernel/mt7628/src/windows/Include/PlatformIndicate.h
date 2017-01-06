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
    NdisIndicate.h

    Abstract:
    Miniport generic portion header file

    Revision History:
    Who         When          What
    --------    ----------    ----------------------------------------------
    */

#ifndef __PLATFORMINDICATE_H__
#define __PLATFORMINDICATE_H__

#include    "MtConfig.h"

typedef struct _RTMP_ADAPTER MP_ADAPTER, *PMP_ADAPTER;
typedef struct _MP_PORT MP_PORT, *PMP_PORT;
typedef struct _BSS_ENTRY BSS_ENTRY, *PBSS_ENTRY;

typedef enum _INDICATE_STATE_MACHINE {
    CONNECTION_START =0x0001,
    CONNECTION_COMPLETE = 0x0002,
    ASSOCIATION_START = 0x0004,
    ASSOCIATION_COMPLETE = 0x0008,
    ROAMING_START =0x0010,
    ROAMING_COMPLETE =0x0020,
    SCAN_COMPLETE   = 0x0040,
    STATE_UNKNOW = 0x8000,
}INDICATE_STATE_MACHINE, *PINDICATE_STATE_MACHINE;

VOID 
PlatformIndicatePktToNdis(
    IN PMP_ADAPTER pAd,
    IN PVOID Pkts,
    IN ULONG PortNumber,
    IN ULONG NumOfPkt
    );

VOID PlatformIndicateStatusToNdis(
    IN PMP_ADAPTER pAd,
    IN PNDIS_STATUS_INDICATION pStatusIndication
    );

VOID 
PlatformIndicateDot11Status(
    IN  PMP_ADAPTER   pAd,    
    IN  PMP_PORT      pPort,
    IN  NDIS_STATUS     StatusCode,
    IN  PVOID           RequestID,
    IN  PVOID           pStatusBuffer,
    IN  ULONG           StatusBufferSize
    );

VOID 
PlatformIndicateAssociationStart(
    IN  PMP_ADAPTER   pAd,
    IN PMP_PORT pPort,
    IN  PBSS_ENTRY  pBss
    );

NDIS_STATUS 
PlatformIndicateAssociationCompletion(
    IN  PMP_ADAPTER       pAd,
    IN  PMP_PORT          pPort,
    IN  DOT11_ASSOC_STATUS  Status
    );

VOID
PlatformIndicateConnectionStart(
    IN PMP_ADAPTER    pAd,
    IN PMP_PORT       pPort,
    IN DOT11_BSS_TYPE   BSSType,
    IN PUCHAR           AdhocBSSID,
    IN PUCHAR           AdhocSsid,
    IN UCHAR            AdhocSsidLen
    );

NDIS_STATUS
PlatformIndicateConnectionCompletion(
    IN  PMP_ADAPTER   pAd,
    IN  PMP_PORT pPort,
    IN  ULONG CompletionStatus
    );

VOID
PlatformIndicateAdhocRoamingStart(
    IN  PMP_ADAPTER   pAd,
    IN  PMP_PORT  pPort,
    IN PUCHAR       Bssid,
    IN PUCHAR       Ssid,
    IN UCHAR        SsidLen
    );

VOID
PlatformIndicateRoamingStart(
    IN  PMP_ADAPTER   pAd,
    IN  PMP_PORT  pPort,
    IN  ULONG RoamingReason
    );

VOID
PlatformIndicateRoamingCompletion(
    IN  PMP_ADAPTER   pAd,
    IN  PMP_PORT  pPort,
    IN  ULONG CompletionStatus
    );

VOID
PlatformIndicateLinkQuality(
    IN PMP_ADAPTER    pAd,
    IN NDIS_PORT_NUMBER PortNumber
    );

VOID
PlatformIndicateNewLinkSpeed(
    IN PMP_ADAPTER    pAd,
    IN NDIS_PORT_NUMBER PortNumber,
    IN UCHAR            OpMode
    );

VOID
PlatformIndicateCurrentPhyPowerState(
    IN PMP_ADAPTER    pAd,
    IN NDIS_PORT_NUMBER PortNumber,
    IN ULONG    PhyId
    );

VOID
PlatformIndicateDisassociation(
    IN PMP_ADAPTER    pAd,
    IN  PMP_PORT  pPort,
    IN PUCHAR   MacAddress,
    IN ULONG Reason
    );

VOID
PlatformIndicateAdhocAssociation(
    IN PMP_ADAPTER    pAd,
    IN  PMP_PORT      pPort,
    IN PUCHAR           MacAddress
    );

VOID   
PlatformIndicateScanStatus(
    IN  PMP_ADAPTER   pAd,
    IN  PMP_PORT      pPort,
    IN  NDIS_STATUS     ScanConfirmStatus,
    IN  BOOLEAN         ClearBssScanFlag,
    IN  BOOLEAN         SetSyncIdle
    );


VOID   
PlatformIndicateResetAllStatus(
    IN  PMP_ADAPTER   pAd,
    IN  PMP_PORT      pPort
    );
#endif
