/****************************************************************************
 * Ralink Tech Inc.
 * 4F, No. 2 Technology 5th Rd.
 * Science-based Industrial Park
 * Hsin-chu, Taiwan, R.O.C.
 * (c) Copyright 2002, Ralink Technology, Inc.
 *
 * All rights reserved. Ralink's source code is an unpublished work and the
 * use of a copyright notice does not imply otherwise. This source code
 * contains confidential trade secret material of Ralink Tech. Any attemp
 * or participation in deciphering, decoding, reverse engineering or in any
 * way altering the source code is stricitly prohibited, unless the prior
 * written consent of Ralink Technology, Inc. is obtained.
 ****************************************************************************

    Module Name:
    auth.c

    Abstract:
    Handle de-auth request from local MLME

    Revision History:
    Who         When          What
    --------    ----------    ----------------------------------------------
    John Chang  08-04-2003    created for 11g soft-AP
 */

#include "MtConfig.h"

#define DRIVER_FILE         0x00300000

/*
    ==========================================================================
    Description:
        authenticate state machine init, including state transition and timer init
    Parameters:
        Sm - pointer to the auth state machine
    Note:
        The state machine looks like this
    
                                    AP_AUTH_REQ_IDLE           
        APMT2_MLME_DEAUTH_REQ     mlme_deauth_req_action  
    ==========================================================================
 */
void ApMlmeAuthStateMachineInit(
    IN PMP_ADAPTER pAd, 
    IN STATE_MACHINE *Sm, 
    OUT STATE_MACHINE_FUNC Trans[]) 
{
    StateMachineInit(Sm, Trans, AP_MAX_AUTH_STATE, AP_MAX_AUTH_MSG, Drop, AP_AUTH_REQ_IDLE, AP_AUTH_MACHINE_BASE);
 
    // the first column
    StateMachineSetAction(Sm, AP_AUTH_REQ_IDLE, APMT2_MLME_DEAUTH_REQ, ApAuthDeauthReqAction);
}

/*
    ==========================================================================
    Description:
        Upper Layer request to kick out a STA
    ==========================================================================
 */
VOID ApAuthDeauthReqAction(
    IN PMP_ADAPTER pAd, 
    IN MLME_QUEUE_ELEM *Elem) 
{
    MLME_DEAUTH_REQ_STRUCT *pInfo;
    HEADER_802_11 Hdr;
    PUCHAR        pOutBuffer = NULL;
    NDIS_STATUS   NStatus;
    ULONG         FrameLen = 0;
    PMP_PORT  pPort = pAd->PortList[Elem->PortNum];
    
    pInfo = (MLME_DEAUTH_REQ_STRUCT *)Elem->Msg;

    if (Elem->Wcid < MAX_LEN_OF_MAC_TABLE)
    {
        // 1. remove this STA from MAC table
        ApLogEvent(pAd, pInfo->Addr, EVENT_DISASSOCIATED);
        MacTableDeleteAndResetEntry(pAd, pPort, Elem->Wcid, pInfo->Addr, TRUE);

        // 2. send out DE-AUTH request frame
        NStatus = MlmeAllocateMemory(pAd, &pOutBuffer);
        if (NStatus != NDIS_STATUS_SUCCESS) 
            return;

        DBGPRINT(RT_DEBUG_TRACE, ("AUTH - Send DE-AUTH req to %02x:%02x:%02x:%02x:%02x:%02x\n",
            pInfo->Addr[0],pInfo->Addr[1],pInfo->Addr[2],pInfo->Addr[3],pInfo->Addr[4],pInfo->Addr[5]));
        MgtMacHeaderInit(pAd,pPort, &Hdr, SUBTYPE_DEAUTH, 0, pInfo->Addr, pPort->PortCfg.Bssid);
        MakeOutgoingFrame(pOutBuffer,            &FrameLen, 
                          sizeof(HEADER_802_11), &Hdr, 
                          2,                     &pInfo->Reason, 
                          END_OF_ARGS);
        NdisCommonMiniportMMRequest(pAd, pOutBuffer, FrameLen);
    }
}


/*
    ==========================================================================
    Description:
        GO deauth P2P Clients
    ==========================================================================
 */
VOID ApAuthGoDeauthReqAction(
    IN PMP_ADAPTER pAd, 
    IN NDIS_PORT_NUMBER PortNum,
    IN MLME_DEAUTH_REQ_STRUCT *pInfo) 
{
    HEADER_802_11 Hdr;
    PUCHAR        pOutBuffer = NULL;
    NDIS_STATUS   NStatus;
    ULONG         FrameLen = 0;
    PMP_PORT pPort= pAd->PortList[PortNum];

    // Send out DE-AUTH request frame
    NStatus = MlmeAllocateMemory(pAd, &pOutBuffer);
    if (NStatus != NDIS_STATUS_SUCCESS) 
        return;

    DBGPRINT(RT_DEBUG_TRACE, ("AUTH - GO Send DE-AUTH req to %02x:%02x:%02x:%02x:%02x:%02x\n",
        pInfo->Addr[0],pInfo->Addr[1],pInfo->Addr[2],pInfo->Addr[3],pInfo->Addr[4],pInfo->Addr[5]));
    MgtMacHeaderInit(pAd, pPort, &Hdr, SUBTYPE_DEAUTH, 0, pInfo->Addr, pPort->PortCfg.Bssid);
    MakeOutgoingFrame(pOutBuffer,            &FrameLen, 
                      sizeof(HEADER_802_11), &Hdr, 
                      2,                     &pInfo->Reason, 
                      END_OF_ARGS);
    NdisCommonMiniportMMRequest(pAd, pOutBuffer, FrameLen);
//  MlmeFreeMemory(pAd, pOutBuffer);
}



/*
    ==========================================================================
    Description:
        Some STA/AP
    Note:
        This action should never trigger AUTH state transition, therefore we
        separate it from AUTH state machine, and make it as a standalone service
    ==========================================================================
 */
VOID ApAuthMlmeAuthCls2errAction(
    IN PMP_ADAPTER pAd, 
    IN PMP_PORT   pPort,
    IN PUCHAR pAddr) 
{
    HEADER_802_11 Hdr;
    PUCHAR        pOutBuffer = NULL;
    NDIS_STATUS   NStatus;
    ULONG         FrameLen = 0;
    USHORT        Reason = REASON_CLS2ERR;
    MAC_TABLE_ENTRY *pEntry; 

    pEntry = MacTableLookup(pAd,  pPort, pAddr);
    if (pEntry)
    {
        MacTableDeleteAndResetEntry(pAd, pPort,pEntry->Aid, pAddr, TRUE);
    }

    // send out DEAUTH request frame 
    NStatus = MlmeAllocateMemory(pAd, &pOutBuffer);
    if (NStatus != NDIS_STATUS_SUCCESS) 
        return;

    DBGPRINT(RT_DEBUG_TRACE, ("AUTH - Class 2 error, Send DEAUTH frame to %02x:%02x:%02x:%02x:%02x:%02x\n", pAddr[0],pAddr[1],pAddr[2],pAddr[3],pAddr[4],pAddr[5]));
    MgtMacHeaderInit(pAd, pPort,&Hdr, SUBTYPE_DEAUTH, 0, pAddr, pPort->PortCfg.Bssid);
    MakeOutgoingFrame(pOutBuffer,            &FrameLen, 
                      sizeof(HEADER_802_11), &Hdr, 
                      2,                     &Reason, 
                      END_OF_ARGS);
    NdisCommonMiniportMMRequest(pAd, pOutBuffer, FrameLen);
}

