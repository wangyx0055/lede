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
    N6Indicate.h

    Abstract:
    Miniport generic portion header file

    Revision History:
    Who         When          What
    --------    ----------    ----------------------------------------------
    */

#ifndef __N6INDICATE_H__
#define __N6INDICATE_H__

#include    "MtConfig.h"

VOID N6PlatformIndicatePktToNdis(
    IN PMP_ADAPTER pAd,
    IN PNET_BUFFER_LIST pNetBufferList,
    IN NDIS_PORT_NUMBER PortNumber,
    IN ULONG NumberOfNetBufferLists
    );

VOID N6PlatformIndicateStatusToNdis(
    IN PMP_ADAPTER pAd,
    IN PNDIS_STATUS_INDICATION pStatusIndication
    );

VOID 
N6PlatformIndicateDot11Status(
    IN  PMP_ADAPTER   pAd,    
    IN  PMP_PORT      pPort,
    IN  NDIS_STATUS     StatusCode,
    IN  PVOID           RequestID,
    IN  PVOID           pStatusBuffer,
    IN  ULONG           StatusBufferSize
    );

VOID 
N6PlatformIndicateAssociationStart(
    IN  PMP_ADAPTER   pAd,
    IN PMP_PORT pPort,
    IN  PBSS_ENTRY  pBss
    );

NDIS_STATUS 
N6PlatformIndicateAssociationCompletion(
    IN  PMP_ADAPTER       pAd,
    IN  PMP_PORT          pPort,
    IN  DOT11_ASSOC_STATUS  Status
    );

VOID
N6PlatformIndicateConnectionStart(
    IN PMP_ADAPTER    pAd,
    IN PMP_PORT       pPort,
    IN DOT11_BSS_TYPE   BSSType,
    IN PUCHAR           AdhocBSSID,
    IN PUCHAR           AdhocSsid,
    IN UCHAR            AdhocSsidLen);

NDIS_STATUS
N6PlatformIndicateConnectionCompletion(
    IN  PMP_ADAPTER   pAd,
    IN  PMP_PORT pPort,
    IN  ULONG CompletionStatus
    );

VOID
N6PlatformIndicateAdhocRoamingStart(
    IN  PMP_ADAPTER   pAd,
    IN  PMP_PORT  pPort,
    IN PUCHAR       Bssid,
    IN PUCHAR       Ssid,
    IN UCHAR        SsidLen
    );

VOID
N6PlatformIndicateRoamingStart(
    IN  PMP_ADAPTER   pAd,
    IN  PMP_PORT  pPort,
    IN  ULONG RoamingReason
    );

VOID
N6PlatformIndicateRoamingCompletion(
    IN  PMP_ADAPTER   pAd,
    IN  PMP_PORT  pPort,
    IN  ULONG CompletionStatus
    );

VOID
N6PlatformIndicateLinkQuality(
    IN PMP_ADAPTER    pAd,
    IN NDIS_PORT_NUMBER PortNumber
    );

VOID
N6PlatformIndicateNewLinkSpeed(
    IN PMP_ADAPTER    pAd,
    IN NDIS_PORT_NUMBER PortNumber,
    IN UCHAR            OpMode
    );

VOID
N6PlatformIndicateCurrentPhyPowerState(
    IN PMP_ADAPTER    pAd,
    IN NDIS_PORT_NUMBER PortNumber,
    IN ULONG    PhyId
    );

VOID
N6PlatformIndicateDisassociation(
    IN PMP_ADAPTER    pAd,
    IN  PMP_PORT  pPort,
    IN PUCHAR   MacAddress,
    IN ULONG Reason
    );

VOID
N6PlatformIndicateAdhocAssociation(
    IN PMP_ADAPTER    pAd,
    IN  PMP_PORT      pPort,
    IN PUCHAR           MacAddress
    );

VOID 
N6PlatformIndicateNLOStatus(
    IN  PMP_ADAPTER   pAd,
    IN  PMP_PORT      pPort
    );

VOID 
N6PlatformIndicateLinkSpeedForNetshCmd(
    IN PMP_ADAPTER    pAd
    ); 

VOID
N6PlatformIndicateMICFailure(
    IN PMP_ADAPTER    pAd,
    IN PUCHAR   MacAddress,
    IN ULONG    KeyId,
    IN BOOLEAN  IsDefaultKey
    );    
#endif
