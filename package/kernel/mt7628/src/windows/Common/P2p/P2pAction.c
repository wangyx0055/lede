/*
 ***************************************************************************
 * Ralink Tech Inc.
 * 4F, No. 2 Technology 5th Rd.
 * Science-based Industrial Park
 * Hsin-chu, Taiwan, R.O.C.
 *
 * (c) Copyright 2002-2010, Ralink Technology, Inc.
 *
 * All rights reserved. Ralink's source code is an unpublished work and the
 * use of a copyright notice does not imply otherwise. This source code
 * contains confidential trade secret material of Ralink Tech. Any attemp
 * or participation in deciphering, decoding, reverse engineering or in any
 * way altering the source code is stricitly prohibited, unless the prior
 * written consent of Ralink Technology, Inc. is obtained.
 ***************************************************************************

    Module Name:
    p2p.c

    Abstract:
    Peer to peer is also called Wifi Direct. P2P is a Task Group of WFA.

    Revision History:
    Who              When               What
    --------    ----------    ----------------------------------------------
    Jan Lee         2010-05-21    created for Peer-to-Peer Action frame(Wifi Direct)
*/
#include "MtConfig.h"

#define DRIVER_FILE         0x00D00000

/*  
    ==========================================================================
    Description: 
        P2P state machine init. P2P state machine starts to function after P2P group is formed. the main task is support power save
        mechanism.
        
    Parameters: 
        S - pointer to the association state machine
    Note:
        The state machine looks like the following as name implies its function
    ==========================================================================
 */
VOID P2pActionStateMachineInit(
    IN  PMP_ADAPTER   pAd, 
    IN  STATE_MACHINE *S, 
    OUT STATE_MACHINE_FUNC Trans[]) 
{
    StateMachineInit(S, Trans, MAX_P2P_STATE, MAX_P2P_MSG, Drop, P2P_IDLE_STATE, P2P_MACHINE_BASE);
    
    StateMachineSetAction(S, P2P_IDLE_STATE, MT2_MLME_P2P_NOA, P2pActionNoaAction);
    
    StateMachineSetAction(S, P2P_IDLE_STATE, MT2_MLME_P2P_PRESENCE_REQ, P2pActionPresReqAction);
    StateMachineSetAction(S, P2P_IDLE_STATE, MT2_MLME_P2P_PRESENCE_RSP, P2pActionPresRspAction);
    StateMachineSetAction(S, P2P_IDLE_STATE, MT2_MLME_P2P_GO_DIS_REQ, P2pActionGoDiscoverAction);
    StateMachineSetAction(S, P2P_IDLE_STATE, MT2_MLME_P2P_GAS_INT_REQ, P2pActionGasIntialReqAction);
    StateMachineSetAction(S, P2P_IDLE_STATE, MT2_MLME_P2P_GAS_INT_RSP, P2pActionGasIntialRspAction);
    StateMachineSetAction(S, P2P_IDLE_STATE, MT2_MLME_P2P_GAS_CB_REQ, P2pActionGasComebackReqAction);
    StateMachineSetAction(S, P2P_IDLE_STATE, MT2_MLME_P2P_GAS_CB_RSP, P2pActionGasComebackRspAction);

    StateMachineSetAction(S, P2P_IDLE_STATE, MT2_PEER_P2P_NOA, P2pActionPeerNoaAction);
    StateMachineSetAction(S, P2P_IDLE_STATE, MT2_PEER_P2P_PRESENCE_REQ, P2pActionPeerPresReqAction);
    StateMachineSetAction(S, P2P_IDLE_STATE, MT2_PEER_P2P_PRESENCE_RSP, P2pActionPeerPresRspAction);
    StateMachineSetAction(S, P2P_IDLE_STATE, MT2_PEER_P2P_GO_DIS_REQ, P2pActionPeerGoDiscoverAction);
    
}


VOID P2pActionOneTimeNoAOffChannel(
    IN PMP_ADAPTER pAd,
    IN PMP_PORT     pPort,
    IN ULONG        Duration)
{
    pPort->P2PCfg.GONoASchedule.Count = 1;
    if (Duration > 0)
        pPort->P2PCfg.GONoASchedule.Duration = Duration*0x400;    // interval * 1ms
    pPort->P2PCfg.GONoASchedule.Interval = pPort->P2PCfg.GONoASchedule.Duration;    // 100ms
    DBGPRINT(RT_DEBUG_TRACE, ("P2pActionOneTimeNoAOffChannel  Duration %d    \n", pPort->P2PCfg.GONoASchedule.Duration));
    P2pGOStartNoA(pAd, pPort, TRUE);
}

BOOLEAN P2pActionMlmeCntlOidScanProc(
    IN PMP_ADAPTER pAd,
    IN PMP_PORT     pPort,
    IN MLME_SCAN_REQ_STRUCT *pScanReq)
{
    //PMP_PORT  pPort = pAd->PortList[pPort->P2PCfg.PortNumber];
    ULONG       Duration = 0;
    UCHAR       channelno = pAd->HwCfg.ChannelListNum;
    
    if (pAd->HwCfg.ChannelListNum > 15)
    {
        channelno = 3;
    }
    else if (pAd->OpMode == OPMODE_STAP2P)
    {
        channelno = 3;
    }
    
    DBGPRINT(RT_DEBUG_TRACE, ("P2pActionMlmeCntlOidScanProc   ==>ChannelListNum = %d.  channelno = %d.  \n", pAd->HwCfg.ChannelListNum, channelno));
    // We need to shorten active scan time in order for WZC connect issue
    // Chnage the channel scan time for CISCO stuff based on its IAPP announcement
    PlatformZeroMemory(&pPort->P2PCfg.QueuedScanReq, sizeof(MLME_SCAN_REQ_STRUCT));
    
    if (IS_P2P_GO_OP(pPort) && IS_P2P_SIGMA_OFF(pPort))
    {
    }
    else
        return FALSE;
        
    if (pPort->CommonCfg.P2pControl.field.ImproveGOScan == 0)
        return FALSE;

    if ((pAd->HwCfg.ChannelListNum > 15) && (pPort->CommonCfg.P2pControl.field.ImproveGOScanConcurrentMulpiPhase == 0) && (pAd->OpMode == OPMODE_STAP2P))
        return FALSE;

    if ((pAd->HwCfg.ChannelListNum > 15) && (pPort->CommonCfg.P2pControl.field.ImproveGOScanSTAMulpiPhase == 0) && (pAd->OpMode == OPMODE_STA))
        return FALSE;

    if (pScanReq != NULL)
    {
        if (pScanReq->ScanType == FAST_SCAN_ACTIVE)
            return FALSE;
        // to shorten channel dwell time for fast roaming scan and ScanTab update
        else if ((pScanReq->ScanType == FAST_SCAN_ACTIVE) || 
                 (pAd->StaCfg.bFastRoamingScan == TRUE))
        {
            Duration = channelno*FAST_ACTIVE_SCAN_TIME; 
        }
        else if (pScanReq->ScanType == SCAN_P2P)
        {
            if (pPort->P2PCfg.P2pProprietary.ListenChanelCount > 11)
                Duration = channelno*SHORT_CHANNEL_TIME(pAd);
            else
                 Duration = channelno*pPort->P2PCfg.P2pCounter.ListenInterval*ONETU;
        }
        else // must be SCAN_PASSIVE or SCAN_ACTIVE
        {
            if ((pPort->CommonCfg.PhyMode == PHY_11ABG_MIXED) || 
                 (pPort->CommonCfg.PhyMode == PHY_11ABGN_MIXED) || 
                 (pPort->CommonCfg.PhyMode == PHY_11AGN_MIXED) || 
                 (pPort->CommonCfg.PhyMode == PHY_11VHT))
            {
                if (pPort->ScaningChannel > 14)
                    Duration = channelno*SHORT_CHANNEL_TIME(pAd);
                else    
                    Duration = channelno*MIN_CHANNEL_TIME(pAd);
            }
            else
                Duration = channelno* MAX_CHANNEL_TIME;
        }
        PlatformMoveMemory(&pPort->P2PCfg.QueuedScanReq, pScanReq, sizeof(MLME_SCAN_REQ_STRUCT));
        // Real absence period should consider time to switch channel. Estimate 1ms for each channel switch.
//      if (pAd->HwCfg.ChannelListNum <= 14)
//          Duration += (pAd->HwCfg.ChannelListNum*1);
//      else 
        // USB spends more time than PCI. So use different number with PCI
        Duration += channelno*39;   // Back to op channel every scan 3 channels.
        Duration += 20;
    }
    P2pActionOneTimeNoAOffChannel(pAd, pPort, Duration);
    pPort->P2PCfg.P2PChannelState = P2P_NOATHEN_GOTOSCAN_STATE;
    GOUpdateBeaconFrame(pAd, pPort);
    return TRUE;
}

/*  
    ==========================================================================
    Description: 
        P2P Action frame differs only in InBuffer. Others are all common to all ACtion Subtype
        
    Parameters: 
        S - pointer to the association state machine
    Note:
        The state machine looks like the following as name implies its function
    ==========================================================================
 */
VOID P2pActionCommonAction(
    IN PMP_ADAPTER pAd, 
    IN UCHAR        OUISubType,
    IN UCHAR        Token,
    IN PUCHAR       pInBuffer,
    IN UCHAR        InBufferLen,
    IN MLME_QUEUE_ELEM *Elem) 
{
    PMLME_P2P_ACTION_STRUCT       pGoReq = (PMLME_P2P_ACTION_STRUCT) Elem->Msg;
    PUCHAR         pOutBuffer = NULL;
    NDIS_STATUS     NStatus;
    PMP_PORT  pPort = pAd->PortList[Elem->PortNum];
    ULONG       FrameLen = 0;
    FRAME_P2P_ACTION        Frame;
    ULONG       TmpLen;
    UCHAR       i;
    PUCHAR      pDest;
        
    NStatus = MlmeAllocateMemory(pAd, &pOutBuffer);  //Get an unused nonpaged memory
    if (NStatus != NDIS_STATUS_SUCCESS)
    {
        return;
    }
    DBGPRINT(RT_DEBUG_TRACE, (" TO= %x %x %x %x %x %x  \n",  pGoReq->Addr[0], pGoReq->Addr[1],pGoReq->Addr[2],pGoReq->Addr[3],pGoReq->Addr[4],pGoReq->Addr[5]));
    DBGPRINT(RT_DEBUG_TRACE, (" Bssid= %x %x %x %x %x %x  \n",  pPort->PortCfg.Bssid[0], pPort->PortCfg.Bssid[1],pPort->PortCfg.Bssid[2],pPort->PortCfg.Bssid[3],pPort->PortCfg.Bssid[4],pPort->PortCfg.Bssid[5]));

    ActHeaderInit(pAd, pPort, &Frame.Hdr, pGoReq->Addr, pPort->PortCfg.Bssid);
    Frame.Category = CATEGORY_VENDOR;
    PlatformMoveMemory(&Frame.OUI[0], P2POUIBYTE, 4);
    Frame.OUISubType = OUISubType;
    Frame.Token = Token;
    // No Element
    MakeOutgoingFrame(pOutBuffer,               &FrameLen,
                        sizeof(FRAME_P2P_ACTION),   &Frame,
                        END_OF_ARGS);
    if ((InBufferLen > 0) && (pInBuffer != NULL))
    {
        MakeOutgoingFrame(pOutBuffer + FrameLen,                &TmpLen,
                            InBufferLen,    pInBuffer,
                            END_OF_ARGS);
        FrameLen += TmpLen;
    }
    
    NdisCommonMiniportMMRequest(pAd, pOutBuffer, FrameLen);
    //MlmeFreeMemory(pAd, pOutBuffer);
    pDest = (PUCHAR)&Frame;
    for (i = 0; i <FrameLen; )
    {
        DBGPRINT(RT_DEBUG_TRACE,(": %x %x %x %x %x %x %x %x %x \n", *(pDest+i), *(pDest+i+1), *(pDest+i+2), 
        *(pDest+i+3), *(pDest+i+4), *(pDest+i+5), *(pDest+i+6), *(pDest+i+7), *(pDest+i+8)));
        i = i + 9;
    }
    
    DBGPRINT(RT_DEBUG_TRACE, ("Common P2P ACT request.   FrameLen = %d.  \n", FrameLen));
}



/*  
    ==========================================================================
    Description: 
        Send Publiac action frame. But with ACtion is GAS_INITIAL_REQ (11).
        802.11u. 7.4.7.10
        
    Parameters: 
    Note:

    ==========================================================================
 */
VOID P2pActionGasIntialReqAction(
    IN PMP_ADAPTER pAd, 
    IN MLME_QUEUE_ELEM *Elem) 
{
    UCHAR   Action = Elem->Msg[LENGTH_802_11+1];
    PMP_PORT  pPort = pAd->PortList[Elem->PortNum];
    PMLME_P2P_ACTION_STRUCT       pReq = (PMLME_P2P_ACTION_STRUCT) Elem->Msg;
    PHEADER_802_11  pHeader;
    PUCHAR      pOutBuffer;
    ULONG       FrameLen = 0;
    PUCHAR      pDest, pLength;
    NDIS_STATUS   NStatus;

    DBGPRINT(RT_DEBUG_TRACE, ("P2pActionGasIntialReqAction.Token = %d\n", pPort->P2PCfg.Token));
    // allocate and send out ProbeRsp frame
    NStatus = MlmeAllocateMemory(pAd, &pOutBuffer);  //Get an unused nonpaged memory
    if (NStatus != NDIS_STATUS_SUCCESS)
        return;

    DBGPRINT(RT_DEBUG_TRACE, ("P2pActionGasIntialReqAction.  TO %x %x %x %x %x %x. \n", pReq->Addr[0], pReq->Addr[1], pReq->Addr[2], pReq->Addr[3], pReq->Addr[4], pReq->Addr[5]));
    
    pHeader = (PHEADER_802_11)pOutBuffer;
    ActHeaderInit(pAd, pPort, pHeader,  pReq->Addr, pReq->Addr);
    DBGPRINT(RT_DEBUG_TRACE, (" = %x %x %x %x %x %x  \n",  pPort->CurrentAddress[0], pPort->CurrentAddress[1],pPort->CurrentAddress[2],pPort->CurrentAddress[3],pPort->CurrentAddress[4],pPort->CurrentAddress[5]));
    FrameLen = 24;
    DBGPRINT(RT_DEBUG_TRACE, ("Use Token = %d. \n", pPort->P2PCfg.Token));

    pDest = pOutBuffer + 24;
    *pDest = CATEGORY_PUBLIC;       // ->Category
    *(pDest+1) = GAS_INITIAL_REQ;   // -> Action
    *(pDest+2) = pPort->P2PCfg.Token; // ->Token
    pDest += 3;
    FrameLen += 3;
    // -> Advertisement protocol information element. Exactly ONE.  7.3.2.26
    // Element ID
    *pDest = 0x6c;  // =108
    // Length
    *(pDest+1) = 2; // 
    pLength = pDest + 1;
    // Advertisement tuple 
    *(pDest+2) = 0;     // Query Response Length Limit
    *(pDest + 3) = 0;   // Means the tuple is Vendor specific format
/*  *(pDest+2) = 0x7f;      // Query Response Length Limit
    *(pDest + 3) = 0xdd;    // Means the tuple is Vendor specific format
    *(pDest + 4) = 0x9;
    *(pDest + 5) = 0x1;     // Indicator -1
    *(pDest + 6) = 0x0;     // Indicator -2
        // - >Service TLV
        *(pDest + 7) = 0x2;     // Length -1
        *(pDest + 8) = 0x0;     // Length -2
        *(pDest + 9) = 0x0;     // service Type. 0 : all types
        *(pDest + 10) = 0x1;        // SErvice transaction.*/
    FrameLen += 4;
    pDest += 4;
    
    // -> Query Request length
    *pDest = 14;    // Length -1
    *(pDest+1) = 0; // Length -2
    FrameLen += 2;
    pDest += 2;
        // -> Query Request
    *pDest = 0xdd;  //  EId
    *(pDest+1) = 0xdd;  //  EId 
    *(pDest+2) = 0xa;       // Length -1
    *(pDest + 3) = 0;   // Length -2
    PlatformMoveMemory((pDest + 4), P2POUIBYTE, 4);
    *(pDest + 8) = 0x1;     // Indicator -1
    *(pDest + 9) = 0x0;     // Indicator -2
    *(pDest + 10) = 0x2;        // Length -1
    *(pDest + 11) = 0x0;        // Length -2
    *(pDest + 12) = 0x0;        // Service Type
    pPort->P2PCfg.ServiceTransac++;
    *(pDest + 13) = pPort->P2PCfg.ServiceTransac;     // Transaction ID
    FrameLen += 14;
    pDest += 14;
    
    if (FrameLen > 0)
        NdisCommonMiniportMMRequest(pAd, pOutBuffer, FrameLen);
    else if (pOutBuffer != NULL)
        MlmeFreeMemory(pAd, pOutBuffer);
    
}


/*  
    ==========================================================================
    Description: 
        Send Publiac action frame. But with ACtion is GAS_INITIAL_RSP (12).
        802.11u. 7.4.7.10
        
    Parameters: 
    Note:

    ==========================================================================
 */
VOID P2pActionGasIntialRspAction(
    IN PMP_ADAPTER pAd, 
    IN MLME_QUEUE_ELEM *Elem) 
{
    UCHAR   Action = Elem->Msg[LENGTH_802_11+1];
    PMP_PORT  pPort = pAd->PortList[Elem->PortNum];
    PMLME_P2P_ACTION_STRUCT       pReq = (PMLME_P2P_ACTION_STRUCT) Elem->Msg;
    PHEADER_802_11  pHeader;
    PUCHAR      pOutBuffer;
    ULONG       FrameLen = 0;
    PUCHAR      pDest, pLength;
    NDIS_STATUS   NStatus;

    DBGPRINT(RT_DEBUG_TRACE, ("P2pActionGasIntialRspAction. \n"));
    // allocate and send out ProbeRsp frame
    NStatus = MlmeAllocateMemory(pAd, &pOutBuffer);  //Get an unused nonpaged memory
    if (NStatus != NDIS_STATUS_SUCCESS)
        return;
    DBGPRINT(RT_DEBUG_TRACE, ("P2pActionGasIntialRspAction.  TO %x %x %x %x %x %x. \n", pReq->Addr[0], pReq->Addr[1], pReq->Addr[2], pReq->Addr[3], pReq->Addr[4], pReq->Addr[5]));

    pHeader = (PHEADER_802_11)pOutBuffer;
    ActHeaderInit(pAd, pPort, pHeader,  pReq->Addr, pPort->CurrentAddress);
    DBGPRINT(RT_DEBUG_TRACE, ("pPort = %x %x %x %x %x %x  \n",  pPort->CurrentAddress[0], pPort->CurrentAddress[1],pPort->CurrentAddress[2],pPort->CurrentAddress[3],pPort->CurrentAddress[4],pPort->CurrentAddress[5]));
    FrameLen = 24;

    pDest = pOutBuffer + 24;
    *pDest = CATEGORY_PUBLIC;   // ->Category
    *(pDest+1) = GAS_INITIAL_RSP;   // ->Action
    *(pDest+2) = pPort->P2PCfg.Token;     // ->Token is copied from Req frame.
    *(pDest+3) = 0;     // ->Status
    *(pDest+4) = 0;     // ->Status
    *(pDest+5) = 0;     // ->Comeback Delay
    *(pDest+6) = 0;     // ->Comeback Delay
    pDest += 7;
    FrameLen += 7;

    // 
    // -> Advertisement protocol information element
    // Element ID
    *pDest = 0x6c;  // =108
    // Length
    *(pDest+1) = 2; // 
    pLength = pDest + 1;
    // Advertisement tuple 
    *(pDest+2) = 0;     // Query Response Length Limit
    *(pDest + 3) = 0;   // Means the tuple is Vendor specific format
    FrameLen += 4;
    pDest += 4;
    
    // -> Query Request length
    *pDest = 14;    // Length -1
    *(pDest+1) = 0; // Length -2
    FrameLen += 2;
    pDest += 2;
        // -> Query Request
    *pDest = 0xdd;  //  EId  56797 = 0xdddd
    *(pDest+1) = 0xdd;  //  EId 
    *(pDest+2) = 0xa;       // Length -1
    *(pDest + 3) = 0;   // Length -2
    PlatformMoveMemory((pDest + 4), P2POUIBYTE, 4);
    *(pDest + 8) = 0x1;     // Indicator -1
    *(pDest + 9) = 0x0;     // Indicator -2
    *(pDest + 10) = 0x2;        // Length -1
    *(pDest + 11) = 0x0;        // Length -2
    *(pDest + 12) = 0x0;        // Service Type
    *(pDest + 13) = pPort->P2PCfg.ServiceTransac;     // Transaction ID
    FrameLen += 14;
    pDest += 14;
    if (FrameLen > 0)
        NdisCommonMiniportMMRequest(pAd, pOutBuffer, FrameLen);
    else if (pOutBuffer != NULL)
        MlmeFreeMemory(pAd, pOutBuffer);
    
}


/*  
    ==========================================================================
    Description: 
        Send Publiac action frame. But with ACtion is GAS_INITIAL_REQ (11).
        802.11u. 7.4.7.10
        
    Parameters: 
    Note:

    ==========================================================================
 */
VOID P2pActionGasComebackReqAction(
    IN PMP_ADAPTER pAd, 
    IN MLME_QUEUE_ELEM *Elem) 
{
    UCHAR   Action = Elem->Msg[LENGTH_802_11+1];
    PMP_PORT  pPort = pAd->PortList[Elem->PortNum];
    PMLME_P2P_ACTION_STRUCT       pReq = (PMLME_P2P_ACTION_STRUCT) Elem->Msg;
    PHEADER_802_11  pHeader;
    PUCHAR      pOutBuffer;
    ULONG       FrameLen = 0;
    PUCHAR      pDest;
    NDIS_STATUS   NStatus;

    DBGPRINT(RT_DEBUG_TRACE, ("P2pActionGasComebackReqAction. \n"));
    // allocate and send out ProbeRsp frame
    NStatus = MlmeAllocateMemory(pAd, &pOutBuffer);  //Get an unused nonpaged memory
    if (NStatus != NDIS_STATUS_SUCCESS)
        return;

    DBGPRINT(RT_DEBUG_TRACE, ("P2pActionGasComebackReqAction.  TO %x %x %x %x %x %x. \n", pReq->Addr[0], pReq->Addr[1], pReq->Addr[2], pReq->Addr[3], pReq->Addr[4], pReq->Addr[5]));
    
    pHeader = (PHEADER_802_11)pOutBuffer;
    ActHeaderInit(pAd, pPort, pHeader,  pReq->Addr, pPort->CurrentAddress);
    DBGPRINT(RT_DEBUG_TRACE, ("port = %x %x %x %x %x %x  \n",  pPort->CurrentAddress[0], pPort->CurrentAddress[1],pPort->CurrentAddress[2],pPort->CurrentAddress[3],pPort->CurrentAddress[4],pPort->CurrentAddress[5]));
    FrameLen = 24;

    pDest = pOutBuffer + 24;
    *pDest = CATEGORY_PUBLIC;   // ->Category
    *(pDest+1) = GAS_COMEBACK_REQ;  // -> Action
    *(pDest+2) = pPort->P2PCfg.Token;     // ->Token
    pDest += 3;
    FrameLen += 3;
    DBGPRINT(RT_DEBUG_TRACE, ("Use Token = %d. \n", pPort->P2PCfg.Token));

    if (FrameLen > 0)
        NdisCommonMiniportMMRequest(pAd, pOutBuffer, FrameLen);
    else if (pOutBuffer != NULL)
        MlmeFreeMemory(pAd, pOutBuffer);
    
}

/*  
    ==========================================================================
    Description: 
        Send Publiac action frame. But with ACtion is GAS_INITIAL_REQ (11).
        802.11u. 7.4.7.10
        
    Parameters: 
    Note:

    ==========================================================================
 */
VOID P2pActionGasComebackRspAction(
    IN PMP_ADAPTER pAd, 
    IN MLME_QUEUE_ELEM *Elem) 
{
    UCHAR   Action = Elem->Msg[LENGTH_802_11+1];
    PMP_PORT  pPort = pAd->PortList[Elem->PortNum];
    PMLME_P2P_ACTION_STRUCT       pReq = (PMLME_P2P_ACTION_STRUCT) Elem->Msg;
    PHEADER_802_11  pHeader;
    PUCHAR      pOutBuffer;
    ULONG       FrameLen = 0;
    PUCHAR      pDest;
    NDIS_STATUS   NStatus;

    DBGPRINT(RT_DEBUG_TRACE, ("P2pActionGasComebackRspAction. \n"));
    // allocate and send out ProbeRsp frame
    NStatus = MlmeAllocateMemory(pAd, &pOutBuffer);  //Get an unused nonpaged memory
    if (NStatus != NDIS_STATUS_SUCCESS)
        return;

    DBGPRINT(RT_DEBUG_TRACE, ("P2pActionGasComebackRspAction.  TO %x %x %x %x %x %x. \n", pReq->Addr[0], pReq->Addr[1], pReq->Addr[2], pReq->Addr[3], pReq->Addr[4], pReq->Addr[5]));
    
    pHeader = (PHEADER_802_11)pOutBuffer;
    ActHeaderInit(pAd, pPort, pHeader,  pReq->Addr, pPort->CurrentAddress);
    FrameLen = 24;

    pDest = pOutBuffer + 24;
    *pDest = CATEGORY_PUBLIC;   // ->Category
    *(pDest+1) = GAS_COMEBACK_RSP;  // -> Action
    *(pDest+2) = pPort->P2PCfg.Token;     // ->Token
    pDest += 3;
    FrameLen += 3;
        
    if (FrameLen > 0)
        NdisCommonMiniportMMRequest(pAd, pOutBuffer, FrameLen);
    else if (pOutBuffer != NULL)
        MlmeFreeMemory(pAd, pOutBuffer);
    
}

VOID P2pActionPresReqAction(
    IN PMP_ADAPTER pAd, 
    IN MLME_QUEUE_ELEM *Elem) 
{
    UCHAR       NoAAttribute[32];
    PMLME_P2P_ACTION_STRUCT       pGoReq = (PMLME_P2P_ACTION_STRUCT) Elem->Msg;
    UCHAR           P2pIEFixed[6] = {0xdd, 0x04, 0x00, 0x50, 0xf2, 0x09};
    UCHAR           Index = 0;
    PMP_PORT    pPort = pAd->PortList[FXXK_PORT_0];
    PMAC_TABLE_ENTRY pWcidMacTabEntry = MlmeSyncMacTabMatchedRoleType(pPort, ROLE_WLANIDX_P2P_CLIENT);
    UCHAR           Wcid = (UCHAR)MlmeSyncGetWlanIdxByPort(pPort, ROLE_WLANIDX_BSSID);
    if(pWcidMacTabEntry == NULL)
    {
        DBGPRINT(RT_DEBUG_TRACE,("%s(), Line:%d, Mac Entry is NULL  Wcid = %d\n", __FUNCTION__, __LINE__, Wcid));
        return;
    }
    
    // Use the owner of P2P wcid in concurrent Client if this entry has inserted
    if ((pAd->OpMode == OPMODE_STAP2P) && (pWcidMacTabEntry->ValidAsCLI))
    {
        Wcid = (UCHAR)MlmeSyncGetWlanIdxByPort(pPort, ROLE_WLANIDX_P2P_CLIENT);
    }

    pWcidMacTabEntry = MlmeSyncMacTabMatchedWlanIdx(pPort, Wcid);

    if(pWcidMacTabEntry == NULL)
    {
        DBGPRINT(RT_DEBUG_TRACE,("%s(), Line:%d, Mac Entry is NULL  Wcid = %d\n", __FUNCTION__, __LINE__, Wcid));
        return;
    }
    
    DBGPRINT(RT_DEBUG_TRACE, ("P2pActionPresReqAction. \n"));
    pWcidMacTabEntry->P2pInfo.GeneralToken++;
    if (pWcidMacTabEntry->P2pInfo.GeneralToken == 0)
        pWcidMacTabEntry->P2pInfo.GeneralToken++;
    PlatformMoveMemory(&P2pIEFixed[2], P2POUIBYTE, 4);
    {
        // Add the header of P2P IE
        PlatformMoveMemory(&NoAAttribute[0], &P2pIEFixed, 6);
        Index += 6;
        pWcidMacTabEntry->P2pInfo.GeneralToken++;
        NoAAttribute[Index] = SUBID_P2P_NOA;
        NoAAttribute[Index+1] = 15;
        NoAAttribute[Index+2] = 0;
        NoAAttribute[Index+3] = pWcidMacTabEntry->P2pInfo.GeneralToken; // index
        NoAAttribute[Index+4] = 0; // CTWindows force to zero
        //Set NoA parameters followed test case 6.1.9
        pPort->P2PCfg.GONoASchedule.Count = 1;    // peferred
        pPort->P2PCfg.GONoASchedule.StartTime = 0;
        pPort->P2PCfg.GONoASchedule.Duration = 0xc800;
        pPort->P2PCfg.GONoASchedule.Interval = 0x19000;
        
        // Count
        NoAAttribute[Index+5] = pPort->P2PCfg.GONoASchedule.Count;    
        // Duration
        PlatformMoveMemory(&NoAAttribute[Index+6], &pPort->P2PCfg.GONoASchedule.Duration, 4);
        // Interval
        PlatformMoveMemory(&NoAAttribute[Index+10], &pPort->P2PCfg.GONoASchedule.Interval, 4);
        PlatformMoveMemory(&NoAAttribute[Index+14], &pPort->P2PCfg.GONoASchedule.StartTime, 4);
        // Update IE length
        NoAAttribute[1] += 18;
        P2pActionCommonAction(pAd, P2PACT_PRESENCE_REQ, pWcidMacTabEntry->P2pInfo.GeneralToken, &NoAAttribute[0], (18+6), Elem);
        DBGPRINT(RT_DEBUG_TRACE, ("P2P- Presence request sent. \n"));
    }
}

VOID P2pActionPresRspAction(
    IN PMP_ADAPTER pAd, 
    IN MLME_QUEUE_ELEM *Elem) 
{
    UCHAR       NoAAttribute[36];
    UCHAR       p2pindex;
    PMLME_P2P_ACTION_STRUCT       pGoReq = (PMLME_P2P_ACTION_STRUCT) Elem->Msg;
    UCHAR           P2pIEFixed[6] = {0xdd, 0x04, 0x00, 0x50, 0xf2, 0x09};   
    UCHAR           Index = 0;
    PMP_PORT pPort = pAd->PortList[Elem->PortNum];
    PMAC_TABLE_ENTRY pWcidMacTabEntry = NULL;
    

    PlatformMoveMemory(&P2pIEFixed[2], P2POUIBYTE, 4);
    p2pindex = pGoReq->TabIndex;

    pWcidMacTabEntry = MlmeSyncMacTabMatchedWlanIdx(pPort, p2pindex);   
    if(pWcidMacTabEntry == NULL)
    {
        DBGPRINT(RT_DEBUG_TRACE,("%s(), Line:%d, Mac Entry is NULL  Wcid = %d\n", __FUNCTION__, __LINE__, p2pindex));
        return;
    }
    
    // If AP's index exists, it also means I am connected.  Similar to sanity check.
    if (p2pindex < MAX_LEN_OF_MAC_TABLE)
    {
        // Add the header of P2P IE
        PlatformMoveMemory(&NoAAttribute[0], &P2pIEFixed, 6);
        Index += 6;

        NoAAttribute[Index] = SUBID_P2P_STATUS;
        NoAAttribute[Index+1] = 1;
        NoAAttribute[Index+2] = 0;

        // Count Field is also called Type. type = 1 means preferred. type = 2 means max limit.
        if ((pWcidMacTabEntry->P2pInfo.NoADesc[0].Count > 2) || (pWcidMacTabEntry->P2pInfo.NoADesc[0].Count == 0))
        {
            NoAAttribute[Index+3] = P2PSTATUS_INVALID_PARM; // index
            P2pActionCommonAction(pAd, P2PACT_PRESENCE_RSP, pWcidMacTabEntry->P2pInfo.NoAToken, &NoAAttribute[0], 3+6, Elem);
            DBGPRINT(RT_DEBUG_TRACE, ("P2P- Presence Response sent with error. \n"));
        }
        else
        {
            //TODO: ADD NoA Descriptor in PRESENCE_RSP and BEACON
            //To Pass Sigma 5.1.9, we don't need to include NoA descriptor
            NoAAttribute[Index+3] = P2PSTATUS_SUCCESS;  // index
            NoAAttribute[Index+4] = SUBID_P2P_NOA;
            NoAAttribute[Index+5] = 2;
            NoAAttribute[Index+6] = 0;
            NoAAttribute[Index+7] = pWcidMacTabEntry->P2pInfo.NoAToken; // NoAToken should save the token from client's presence request.
            NoAAttribute[Index+8] = pPort->P2PCfg.CTWindows;  // CTWindows
            NoAAttribute[1] += 9; 
            P2pActionCommonAction(pAd, P2PACT_PRESENCE_RSP, pWcidMacTabEntry->P2pInfo.NoAToken, &NoAAttribute[0], 9+6, Elem);
            DBGPRINT(RT_DEBUG_TRACE, ("P2P- Presence Response sent. \n"));

            // Trigger to update GO's beacon
            pPort->P2PCfg.GONoASchedule.bValid = TRUE;
            DBGPRINT(RT_DEBUG_TRACE, ("P2pActionPresRspAction: Update NoA Schedual on GO!\n"));

        }

    }
}



VOID P2pActionGoDiscoverAction(
    IN PMP_ADAPTER pAd, 
    IN MLME_QUEUE_ELEM *Elem) 
{
    PMLME_P2P_ACTION_STRUCT       pGoReq = (PMLME_P2P_ACTION_STRUCT) Elem->Msg;
    DBGPRINT(RT_DEBUG_TRACE, ("P2P- GO DISCOVERY request. \n"));
    if (pGoReq->TabIndex < MAX_P2P_GROUP_SIZE)
    {
        // when used in P2pActionGoDiscoverAction, WcidIndex
        pAd->pP2pCtrll->P2PTable.Client[pGoReq->TabIndex].P2pClientState = P2PSTATE_WAIT_GO_DISCO_ACK;
        pAd->pP2pCtrll->P2PTable.Client[pGoReq->TabIndex].GODevDisWaitCount = 0;
        DBGPRINT(RT_DEBUG_TRACE, ("P2P- Client[%d] State %s  \n", pGoReq->TabIndex,decodeP2PClientState(pAd->pP2pCtrll->P2PTable.Client[pGoReq->TabIndex].P2pClientState)));
    }
    P2pActionCommonAction(pAd, P2PACT_GO_DISCOVER_REQ, 0, NULL, 0, Elem); 
}

VOID P2pActionNoaAction(
    IN PMP_ADAPTER pAd, 
    IN MLME_QUEUE_ELEM *Elem) 
{
}

/*  
    ==========================================================================
    Description: 
        Send Publiac action frame. But with ACtion is GAS_INITIAL_RSP (12).
        802.11u. 7.4.7.10
        
    Parameters: 
    Note:

    ==========================================================================
 */
VOID P2pActionSendGasCbReq(
    IN PMP_ADAPTER pAd, 
    IN MLME_QUEUE_ELEM *Elem) 
{
    
}


/*  
    ==========================================================================
    Description: 
        Support WiFi Direct Certification test for P2P Client to send Presence Request Test Case..
        
    Parameters: 
        S - pointer to the association state machine
    Note:
        The state machine looks like the following as name implies its function
    ==========================================================================
 */
VOID P2pActionSendComebackReq(
    IN PMP_ADAPTER pAd, 
    IN PMP_PORT     pPort,
    IN UCHAR        p2pindex,
    IN PUCHAR   Addr)
{
    MLME_P2P_ACTION_STRUCT  P2PActReq;  
    PRT_P2P_CLIENT_ENTRY pP2pEntry = NULL;
    
    DBGPRINT(RT_DEBUG_TRACE, ("P2pActionSendComebackReq = %x %x %x %x %x %x  \n",  Addr[0], Addr[1],Addr[2],Addr[3],Addr[4],Addr[5]));
    if (p2pindex < MAX_P2P_GROUP_SIZE)
    {
        pP2pEntry = &pAd->pP2pCtrll->P2PTable.Client[p2pindex];
        PlatformZeroMemory(&P2PActReq, sizeof(P2PActReq));
        COPY_MAC_ADDR(P2PActReq.Addr, pP2pEntry->addr);
        DBGPRINT(RT_DEBUG_TRACE, (" = %x %x %x %x %x %x  \n",  P2PActReq.Addr[0], P2PActReq.Addr[1],P2PActReq.Addr[2],P2PActReq.Addr[3],P2PActReq.Addr[4],P2PActReq.Addr[5]));
        
        P2PActReq.TabIndex = p2pindex;
        MlmeEnqueue(pAd, pAd->PortList[pPort->P2PCfg.PortNumber], P2P_STATE_MACHINE, MT2_MLME_P2P_GAS_CB_REQ, sizeof(MLME_P2P_ACTION_STRUCT), (PVOID)&P2PActReq);
    }
}



/*  
    ==========================================================================
    Description: 
        Support WiFi Direct Certification test for P2P Client to send Presence Request Test Case..
        
    Parameters: 
        S - pointer to the association state machine
    Note:
        The state machine looks like the following as name implies its function
    ==========================================================================
 */
VOID P2pActionSendServiceReqCmd(
    IN PMP_ADAPTER pAd, 
    IN PMP_PORT     pPort,
    IN UCHAR    p2pindex,
    IN PUCHAR   Addr)
{
    MLME_P2P_ACTION_STRUCT  P2PActReq;  
    PRT_P2P_CLIENT_ENTRY pP2pEntry = NULL;
    
    DBGPRINT(RT_DEBUG_TRACE, ("P2pActionSendServiceReqCmd = %x %x %x %x %x %x  \n",  Addr[0], Addr[1],Addr[2],Addr[3],Addr[4],Addr[5]));
    if (p2pindex < MAX_P2P_GROUP_SIZE)
    {
        pP2pEntry = &pAd->pP2pCtrll->P2PTable.Client[p2pindex];
        PlatformZeroMemory(&P2PActReq, sizeof(P2PActReq));
        COPY_MAC_ADDR(P2PActReq.Addr, pP2pEntry->addr);
        DBGPRINT(RT_DEBUG_TRACE, (" = %x %x %x %x %x %x  \n",  P2PActReq.Addr[0], P2PActReq.Addr[1],P2PActReq.Addr[2],P2PActReq.Addr[3],P2PActReq.Addr[4],P2PActReq.Addr[5]));
        
        P2PActReq.TabIndex = p2pindex;
        MlmeEnqueue(pAd, pAd->PortList[pPort->P2PCfg.PortNumber], P2P_STATE_MACHINE, MT2_MLME_P2P_GAS_INT_REQ, sizeof(MLME_P2P_ACTION_STRUCT), (PVOID)&P2PActReq);
    }
}

/*  
    ==========================================================================
    Description: 
        Support WiFi Direct Certification test for P2P Client to send Presence Request Test Case..
        
    Parameters: 
        S - pointer to the association state machine
    Note:
        The state machine looks like the following as name implies its function
    ==========================================================================
 */
VOID P2pActionSendPresenceReqCmd(
    IN PMP_ADAPTER pAd, 
    IN PMP_PORT     pPort,
    IN UCHAR     macindex) 
{
    MLME_P2P_ACTION_STRUCT  P2PActReq;  
    
    DBGPRINT(RT_DEBUG_TRACE,("P2pActionSendPresenceReqCmd. Send to Macindex = %d, \n", macindex));
    PlatformZeroMemory(&P2PActReq, sizeof(P2PActReq));
    COPY_MAC_ADDR(P2PActReq.Addr, pPort->P2PCfg.Bssid);
    P2PActReq.TabIndex = macindex;
    MlmeEnqueue(pAd, pAd->PortList[pPort->P2PCfg.PortNumber], P2P_STATE_MACHINE, MT2_MLME_P2P_PRESENCE_REQ, sizeof(MLME_P2P_ACTION_STRUCT), (PVOID)&P2PActReq);
}


VOID P2pActionPeerNoaAction(
    IN PMP_ADAPTER pAd, 
    IN MLME_QUEUE_ELEM *Elem) 
{

    PP2P_ACTION_FRAME   pFrame = (PP2P_ACTION_FRAME)Elem->Msg;
    PMAC_TABLE_ENTRY        pClient;
    PMP_PORT pPort = pAd->PortList[Elem->PortNum];
    
    pClient =  MlmeSyncMacTabMatchedWlanIdx(pPort, Elem->Wcid);  
    if(pClient == NULL)
    {
        DBGPRINT(RT_DEBUG_TRACE,("%s(), Line:%d, Mac Entry is NULL  Wcid = %d\n", __FUNCTION__, __LINE__, Elem->Wcid));
        return;
    }
    
    DBGPRINT(RT_DEBUG_TRACE,("P2pActionPeerNoaAction  %s. \n", decodeP2PState(pPort->P2PCfg.P2PConnectState)));
    DBGPRINT(RT_DEBUG_TRACE,("Category = %d. Subtype = %d. Token = %d.\n", pFrame->Category, pFrame->Subtype, pFrame->Token));

    if (!IS_P2P_CLIENT_OP(pPort))
    {
        DBGPRINT(RT_DEBUG_TRACE,("P2pActionPeerNoaAction return %s. \n", decodeP2PState(pPort->P2PCfg.P2PConnectState)));
        return;
    }
    if (Elem->Wcid >= MAX_LEN_OF_MAC_TABLE)
    {
        DBGPRINT(RT_DEBUG_TRACE,("P2pActionPeerNoaAction Elem->Wcid %d.  return.\n",  Elem->Wcid));
        return;
    }

    pPort->P2PCfg.NoAIndex = Elem->Wcid;
    DBGPRINT(RT_DEBUG_TRACE,("P2pActionPeerNoaAction Current  NoAToken = %d. \n",  pClient->P2pInfo.NoAToken));
    if (pFrame->Token != pClient->P2pInfo.NoAToken)
    {
        if ((PlatformEqualMemory(&pFrame->Octet[0], P2POUIBYTE, 4)) && (pFrame->Octet[4] == SUBID_P2P_NOA))
        {
            DBGPRINT(RT_DEBUG_TRACE,("P2pActionPeerNoaAction  CTWindow = %d. \n", pFrame->Octet[8]));
            pClient->P2pInfo.CTWindow = pFrame->Octet[8]; 
            P2pHandleNoAAttri(pAd, pPort, pClient, &pFrame->Octet[0]);
        }
    }
}


VOID P2pActionPeerPresReqAction(
    IN PMP_ADAPTER pAd, 
    IN MLME_QUEUE_ELEM *Elem) 
{
    MLME_P2P_ACTION_STRUCT  P2PActReq;  
    MAC_TABLE_ENTRY     *pEntry;
    PFRAME_P2P_ACTION       pFrame;
    PP2P_NOA_DESC   pNoADesc;   
    
    PMP_PORT pPort = pAd->PortList[Elem->PortNum];
    
    pFrame = (PFRAME_P2P_ACTION)Elem->Msg;

    if (Elem->Wcid >= MAX_LEN_OF_MAC_TABLE)
    {
        DBGPRINT(RT_DEBUG_TRACE,("P2pActionPeerPresReqAction. unknown Elem->Wcid  = %d \n", Elem->Wcid ));
    }
        
    DBGPRINT(RT_DEBUG_TRACE,("P2pActionPeerPresReqAction. Send back to Elem->Wcid  = %d \n", Elem->Wcid ));
        
    pEntry =  MlmeSyncMacTabMatchedWlanIdx(pPort, Elem->Wcid); 

    if(pEntry == NULL)
    {
        DBGPRINT(RT_DEBUG_TRACE,("%s(), Line:%d, Mac Entry is NULL  Wcid = %d\n", __FUNCTION__, __LINE__, Elem->Wcid));
        return;
    }
    

    pNoADesc = (PP2P_NOA_DESC)(&Elem->Msg[11 + sizeof(FRAME_P2P_ACTION)]);
    pEntry->P2pInfo.NoADesc[0].Count = pNoADesc->Count;
    pEntry->P2pInfo.NoADesc[0].Duration = *(PULONG)&pNoADesc->Duration[0];
    pEntry->P2pInfo.NoADesc[0].Interval = *(PULONG)&pNoADesc->Interval[0];
    pEntry->P2pInfo.NoADesc[0].StartTime = *(PULONG)&pNoADesc->StartTime[0];
    DBGPRINT(RT_DEBUG_TRACE,(" pP2pEntry->NoADesc[0].Count = %d, \n", pEntry->P2pInfo.NoADesc[0].Count));
    DBGPRINT(RT_DEBUG_TRACE,(" pP2pEntry->NoADesc[0].Duration = %d, \n", pEntry->P2pInfo.NoADesc[0].Duration));
    DBGPRINT(RT_DEBUG_TRACE,(" pP2pEntry->NoADesc[0].Interval = %d, \n", pEntry->P2pInfo.NoADesc[0].Interval));
    DBGPRINT(RT_DEBUG_TRACE,(" pP2pEntry->NoADesc[0].StartTime = %d, \n", pEntry->P2pInfo.NoADesc[0].StartTime));
    DBGPRINT(RT_DEBUG_TRACE,(" pFrame->Token  = %d \n", pFrame->Token));

    pEntry->P2pInfo.NoAToken = pFrame->Token;
//  pP2pEntry->NoADesc[0].Duration = Elem->Msg;
    PlatformZeroMemory(&P2PActReq, sizeof(P2PActReq));
    COPY_MAC_ADDR(P2PActReq.Addr, pEntry->Addr);
    P2PActReq.TabIndex = Elem->Wcid;
    
    MlmeEnqueue(pAd, pAd->PortList[pPort->P2PCfg.PortNumber], P2P_STATE_MACHINE, MT2_MLME_P2P_PRESENCE_RSP, sizeof(MLME_P2P_ACTION_STRUCT), (PVOID)&P2PActReq);
}


VOID P2pActionPeerPresRspAction(
    IN PMP_ADAPTER pAd, 
    IN MLME_QUEUE_ELEM *Elem) 
{
    DBGPRINT(RT_DEBUG_TRACE,("P2pActionPeerPresRspAction.\n"));
}


VOID P2pActionPeerGoDiscoverAction(
    IN PMP_ADAPTER pAd, 
    IN MLME_QUEUE_ELEM *Elem) 
{
    UCHAR       i;
    PUCHAR      pDest;

    DBGPRINT(RT_DEBUG_TRACE,("P2pActionPeerGoDiscoverAction.\n"));
    //DBGPRINT(RT_DEBUG_TRACE,("bKeepSlient = %d.\n", pPort->P2PCfg.bKeepSlient));
    pDest = &Elem->Msg[0];
    for (i = 0; i <Elem->MsgLen; )
    {
        DBGPRINT(RT_DEBUG_TRACE,(": %x %x %x %x %x %x %x %x %x \n", *(pDest+i), *(pDest+i+1), *(pDest+i+2), 
        *(pDest+i+3), *(pDest+i+4), *(pDest+i+5), *(pDest+i+6), *(pDest+i+7), *(pDest+i+8)));
        i = i + 9;
    }

}


