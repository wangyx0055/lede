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
    NdisIndicate.c

    Abstract:
    Miniport main initialization routines

    Revision History:
    Who         When          What
    --------    ----------    ----------------------------------------------
*/
#include    "MtConfig.h"

ULONG   CurrentIndicateState[RTMP_MAX_NUMBER_OF_PORT];
ULONG   NextIndicateState;

VOID 
PlatformIndicatePktToNdis(
    IN PMP_ADAPTER pAd,
    IN PVOID Pkts,
    IN ULONG PortNumber,
    IN ULONG NumOfPkt
    )
{
    //DBGPRINT(RT_DEBUG_TRACE,("PlatformIndicateNBLToNdis\n");
#if (COMPILE_VISTA_ABOVE(CURRENT_OS_NDIS_VER))
    N6PlatformIndicatePktToNdis(pAd, Pkts, PortNumber, NumOfPkt);
#endif    
}

VOID PlatformIndicateStatusToNdis(
    IN PMP_ADAPTER pAd,
    IN PNDIS_STATUS_INDICATION pStatusIndication
    )
{
#if (COMPILE_VISTA_ABOVE(CURRENT_OS_NDIS_VER))
    N6PlatformIndicateStatusToNdis(pAd, pStatusIndication);
#endif     
}

VOID 
PlatformIndicateDot11Status(
    IN  PMP_ADAPTER   pAd,    
    IN  PMP_PORT      pPort,
    IN  NDIS_STATUS     StatusCode,
    IN  PVOID           RequestID,
    IN  PVOID           pStatusBuffer,
    IN  ULONG           StatusBufferSize)
{
#if (COMPILE_VISTA_ABOVE(CURRENT_OS_NDIS_VER))
    N6PlatformIndicateDot11Status(pAd, pPort, StatusCode, RequestID, pStatusBuffer, StatusBufferSize);
#endif    
    
}

VOID 
PlatformIndicateAssociationStart(
    IN  PMP_ADAPTER   pAd,
    IN PMP_PORT pPort,
    IN  PBSS_ENTRY  pBss)
{
    FUNC_ENTER;
    DBGPRINT(RT_DEBUG_TRACE, ("CurrentIndicateState [%d] = 0x%x,\n",pPort->PortNumber, CurrentIndicateState[pPort->PortNumber])); 
#if (COMPILE_VISTA_ABOVE(CURRENT_OS_NDIS_VER))
    CurrentIndicateState[pPort->PortNumber] = ASSOCIATION_START;
    
    if(CurrentIndicateState[pPort->PortNumber] & NextIndicateState)
    {
    N6PlatformIndicateAssociationStart(pAd, pPort, pBss);

        NextIndicateState =  ASSOCIATION_START| ASSOCIATION_COMPLETE | SCAN_COMPLETE;
    }
#endif    
    FUNC_LEAVE;
}

NDIS_STATUS 
PlatformIndicateAssociationCompletion(
    IN  PMP_ADAPTER       pAd,
    IN  PMP_PORT          pPort,
    IN  DOT11_ASSOC_STATUS  Status)
{
    NDIS_STATUS ntStatus = NDIS_STATUS_SUCCESS;
    
    FUNC_ENTER;
    DBGPRINT(RT_DEBUG_TRACE, ("CurrentIndicateState [%d] = 0x%x,\n",pPort->PortNumber, CurrentIndicateState[pPort->PortNumber])); 
#if (COMPILE_VISTA_ABOVE(CURRENT_OS_NDIS_VER))

    CurrentIndicateState[pPort->PortNumber]  = ASSOCIATION_COMPLETE;
    
    if(CurrentIndicateState[pPort->PortNumber]  & NextIndicateState)
    {
        ntStatus = N6PlatformIndicateAssociationCompletion(pAd, pPort, Status);
        NextIndicateState =  ASSOCIATION_START| ASSOCIATION_COMPLETE | SCAN_COMPLETE | CONNECTION_COMPLETE|ROAMING_COMPLETE;
    }
#endif    
    FUNC_LEAVE;
    
    return ntStatus;
}

VOID
PlatformIndicateConnectionStart(
    IN PMP_ADAPTER    pAd,
    IN PMP_PORT       pPort,
    IN DOT11_BSS_TYPE   BSSType,
    IN PUCHAR           AdhocBSSID,
    IN PUCHAR           AdhocSsid,
    IN UCHAR            AdhocSsidLen)
{
    FUNC_ENTER;
    DBGPRINT(RT_DEBUG_TRACE, ("CurrentIndicateState [%d] = 0x%x,\n",pPort->PortNumber, CurrentIndicateState[pPort->PortNumber])); 
#if (COMPILE_VISTA_ABOVE(CURRENT_OS_NDIS_VER))
    CurrentIndicateState[pPort->PortNumber]  = CONNECTION_START;
    
    if(CurrentIndicateState[pPort->PortNumber]  & NextIndicateState)
    {
        pAd->bConnectionInProgress = TRUE;
        PlatformSetTimer(pPort, &pPort->Mlme.ConnectTimer, 15000); //15 Seconds
    N6PlatformIndicateConnectionStart(pAd, pPort, BSSType, AdhocBSSID, AdhocSsid, AdhocSsidLen);
        NextIndicateState =  ASSOCIATION_START | SCAN_COMPLETE | CONNECTION_COMPLETE;
    }
#endif    

    FUNC_LEAVE;
}

NDIS_STATUS
PlatformIndicateConnectionCompletion(
    IN  PMP_ADAPTER   pAd,
    IN  PMP_PORT pPort,
    IN  ULONG CompletionStatus)
{
    NDIS_STATUS ntStatus = NDIS_STATUS_SUCCESS;
    
    FUNC_ENTER;
    DBGPRINT(RT_DEBUG_TRACE, ("CurrentIndicateState [%d] = 0x%x,\n",pPort->PortNumber, CurrentIndicateState[pPort->PortNumber])); 
#if (COMPILE_VISTA_ABOVE(CURRENT_OS_NDIS_VER))

    CurrentIndicateState[pPort->PortNumber]  = CONNECTION_COMPLETE;

    if(CurrentIndicateState[pPort->PortNumber]  & NextIndicateState)
    {   
        BOOLEAN Cancelled = FALSE;
        PlatformCancelTimer(&pPort->Mlme.ConnectTimer, &Cancelled);
        
        ntStatus =N6PlatformIndicateConnectionCompletion(pAd, pPort, CompletionStatus);
        pAd->bConnectionInProgress = FALSE;
        NextIndicateState =  SCAN_COMPLETE | CONNECTION_START | ROAMING_START;
    }
#endif    
    FUNC_LEAVE;

    return ntStatus;
}

VOID
PlatformIndicateAdhocRoamingStart(
    IN  PMP_ADAPTER   pAd,
    IN  PMP_PORT  pPort,
    IN PUCHAR       Bssid,
    IN PUCHAR       Ssid,
    IN UCHAR        SsidLen)
{
    FUNC_ENTER;
#if (COMPILE_VISTA_ABOVE(CURRENT_OS_NDIS_VER))

    CurrentIndicateState[pPort->PortNumber]  = ROAMING_START;

    if(CurrentIndicateState[pPort->PortNumber]  & NextIndicateState)
    {
    N6PlatformIndicateAdhocRoamingStart(pAd, pPort, Bssid, Ssid, SsidLen);
        
        NextIndicateState =  SCAN_COMPLETE |ROAMING_COMPLETE;
    }
#endif    
    FUNC_LEAVE;
}

VOID
PlatformIndicateRoamingStart(
    IN  PMP_ADAPTER   pAd,
    IN  PMP_PORT  pPort,
    IN  ULONG RoamingReason)
{
    FUNC_ENTER;
    DBGPRINT(RT_DEBUG_TRACE, ("CurrentIndicateState [%d] = 0x%x,\n",pPort->PortNumber, CurrentIndicateState[pPort->PortNumber])); 
#if (COMPILE_VISTA_ABOVE(CURRENT_OS_NDIS_VER))

    CurrentIndicateState[pPort->PortNumber]  = ROAMING_START;

    if(CurrentIndicateState[pPort->PortNumber]  & NextIndicateState)
    {
    N6PlatformIndicateRoamingStart(pAd, pPort, RoamingReason);
        
        NextIndicateState =  SCAN_COMPLETE |ROAMING_COMPLETE|ASSOCIATION_START;
        pPort->CommonCfg.bRoamingInProgress =TRUE;
    }
#endif    
    FUNC_LEAVE;
}

VOID
PlatformIndicateRoamingCompletion(
    IN  PMP_ADAPTER   pAd,
    IN  PMP_PORT  pPort,
    IN  ULONG CompletionStatus)
{
    FUNC_ENTER;
    DBGPRINT(RT_DEBUG_TRACE, ("CurrentIndicateState [%d] = 0x%x,\n",pPort->PortNumber, CurrentIndicateState[pPort->PortNumber])); 
#if (COMPILE_VISTA_ABOVE(CURRENT_OS_NDIS_VER))

    CurrentIndicateState[pPort->PortNumber]  = ROAMING_COMPLETE;

    if(CurrentIndicateState[pPort->PortNumber]  & NextIndicateState)
    {
    N6PlatformIndicateRoamingCompletion(pAd, pPort, CompletionStatus);

        NextIndicateState =  SCAN_COMPLETE |ASSOCIATION_COMPLETE|ROAMING_START;
        pPort->CommonCfg.bRoamingInProgress =FALSE;
    }
#endif    
    FUNC_LEAVE;
}

VOID
PlatformIndicateLinkQuality(
    IN PMP_ADAPTER    pAd,
    IN NDIS_PORT_NUMBER PortNumber
    )
{
    //DBGPRINT(RT_DEBUG_TRACE,("PlatformIndicateLinkQuality\n"));
#if (COMPILE_VISTA_ABOVE(CURRENT_OS_NDIS_VER))
    N6PlatformIndicateLinkQuality(pAd, PortNumber);
#endif    
}

VOID
PlatformIndicateNewLinkSpeed(
    IN PMP_ADAPTER    pAd,
    IN NDIS_PORT_NUMBER PortNumber,
    IN UCHAR            OpMode)
{
#if (COMPILE_VISTA_ABOVE(CURRENT_OS_NDIS_VER))
    N6PlatformIndicateNewLinkSpeed(pAd, PortNumber, OpMode);
#endif    
}

VOID
PlatformIndicateCurrentPhyPowerState(
    IN PMP_ADAPTER    pAd,
    IN NDIS_PORT_NUMBER PortNumber,
    IN ULONG    PhyId)
{
    FUNC_ENTER;
#if (COMPILE_VISTA_ABOVE(CURRENT_OS_NDIS_VER))
    N6PlatformIndicateCurrentPhyPowerState(pAd, PortNumber, PhyId);
#endif    
    FUNC_LEAVE;
}


/*
    ========================================================================
    
    Routine Description:
        A miniport driver must make a NDIS_STATUS_DOT11_DISASSOCIATION indication 
        if the 802.11 station performs a disassociation operation with either an 
        access point (AP) or peer station.

    Arguments:
        pAd           Pointer to our adapter
        MacAddress    MAC Address of the AP or peer station that the 802.11 station has disassociated from.
                      0xFFFFFFFFFFFF means the 802.11 station has disassociated from the AP or all peer stations.
        Reason        the disassociation reason code.

    Return Value:
        NONE
        
    Note:
    
    ========================================================================
*/
VOID
PlatformIndicateDisassociation(
    IN PMP_ADAPTER    pAd,
    IN  PMP_PORT  pPort,
    IN PUCHAR   MacAddress,
    IN ULONG Reason)
{
    FUNC_ENTER;
    DBGPRINT(RT_DEBUG_TRACE, ("CurrentIndicateState [%d] = 0x%x,\n",pPort->PortNumber, CurrentIndicateState[pPort->PortNumber])); 
#if (COMPILE_VISTA_ABOVE(CURRENT_OS_NDIS_VER))
    N6PlatformIndicateDisassociation(pAd, pPort, MacAddress, Reason);
#endif    
    FUNC_LEAVE;
}

VOID
PlatformIndicateAdhocAssociation(
    IN PMP_ADAPTER    pAd,
    IN  PMP_PORT      pPort,
    IN PUCHAR           MacAddress)
{
    FUNC_ENTER;
    DBGPRINT(RT_DEBUG_TRACE, ("CurrentIndicateState [%d] = 0x%x,\n",pPort->PortNumber, CurrentIndicateState[pPort->PortNumber])); 
#if (COMPILE_VISTA_ABOVE(CURRENT_OS_NDIS_VER))
    N6PlatformIndicateAdhocAssociation(pAd, pPort, MacAddress);
#endif    
    FUNC_LEAVE;
}

VOID   
PlatformIndicateScanStatus(
    IN  PMP_ADAPTER   pAd,
    IN  PMP_PORT      pPort,
    IN  NDIS_STATUS     ScanConfirmStatus,
    IN  BOOLEAN         ClearBssScanFlag,
    IN  BOOLEAN         SetSyncIdle
    )
{
    FUNC_ENTER;

    if (pPort->PortType != EXTSTA_PORT)
    {
        return;
    }
    DBGPRINT(RT_DEBUG_TRACE, ("CurrentIndicateState [%d] = 0x%x,\n",pPort->PortNumber, CurrentIndicateState[pPort->PortNumber])); 
#if (COMPILE_VISTA_ABOVE(CURRENT_OS_NDIS_VER))
    //CurrentIndicateState[pPort->PortNumber]  = SCAN_COMPLETE;
    N6PlatformIndicateScanStatus(pAd, pPort, ScanConfirmStatus, ClearBssScanFlag, SetSyncIdle);

    NextIndicateState = SCAN_COMPLETE | CONNECTION_START | CONNECTION_COMPLETE | ROAMING_COMPLETE |ASSOCIATION_COMPLETE|ASSOCIATION_START |ROAMING_START;
#endif      
    FUNC_LEAVE;
}

VOID   
PlatformIndicateResetAllStatus(
    IN  PMP_ADAPTER   pAd,
    IN  PMP_PORT      pPort
    )
{
    UCHAR i = 0;
    FUNC_ENTER;
    DBGPRINT(RT_DEBUG_TRACE, ("CurrentIndicateState [%d] = 0x%x,\n",pPort->PortNumber, CurrentIndicateState[pPort->PortNumber])); 
    
    if ( CurrentIndicateState[pPort->PortNumber] == CONNECTION_START )
    {
        PlatformIndicateConnectionCompletion(pAd, pPort, DOT11_CONNECTION_STATUS_CANDIDATE_LIST_EXHAUSTED);
    }
    else if ( CurrentIndicateState[pPort->PortNumber] == ASSOCIATION_START )
    {
        PlatformIndicateAssociationCompletion(pAd, pPort, DOT11_ASSOC_STATUS_UNREACHABLE);
        PlatformIndicateConnectionCompletion(pAd, pPort, DOT11_CONNECTION_STATUS_CANDIDATE_LIST_EXHAUSTED);
    }
    else if ( CurrentIndicateState[pPort->PortNumber] == ASSOCIATION_COMPLETE)
    {
        PlatformIndicateConnectionCompletion(pAd, pPort, DOT11_CONNECTION_STATUS_CANDIDATE_LIST_EXHAUSTED);
    }    
    else if ( CurrentIndicateState[pPort->PortNumber] == ROAMING_START )
    {
        PlatformIndicateRoamingCompletion(pAd, pPort, DOT11_CONNECTION_STATUS_CANDIDATE_LIST_EXHAUSTED);
    }
    else if ( CurrentIndicateState[pPort->PortNumber] == ROAMING_COMPLETE )
    {
        PlatformIndicateConnectionCompletion(pAd, pPort, DOT11_CONNECTION_STATUS_CANDIDATE_LIST_EXHAUSTED); 
    }

    for(i = 0; i <  RTMP_MAX_NUMBER_OF_PORT; i ++)
    {
        CurrentIndicateState[i] = STATE_UNKNOW;
    }
    NextIndicateState = SCAN_COMPLETE | CONNECTION_START | CONNECTION_COMPLETE | ROAMING_COMPLETE |ASSOCIATION_COMPLETE|ASSOCIATION_START |ROAMING_START;

    pAd->bConnectionInProgress = FALSE;
    FUNC_LEAVE;
}


