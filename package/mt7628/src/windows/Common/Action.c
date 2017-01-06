/****************************************************************************
 * Ralink Tech Inc.
 * 4F, No. 2 Technology 5th Rd.
 * Science-based Industrial Park
 * Hsin-chu, Taiwan, R.O.C.
 * (c) Copyright 2007, Ralink Technology, Inc.
 *
 * All rights reserved. Ralink's source code is an unpublished work and the
 * use of a copyright notice does not imply otherwise. This source code
 * contains confidential trade secret material of Ralink Tech. Any attemp
 * or participation in deciphering, decoding, reverse engineering or in any
 * way altering the source code is stricitly prohibited, unless the prior
 * written consent of Ralink Technology, Inc. is obtained.
 ****************************************************************************

    Module Name:
    action.c

    Abstract:
    Handle action frame.

    Revision History:
    Who         When          What
    --------    ----------    ----------------------------------------------
    Jan Lee     2006        created for rt2860 for 802.11n
 */

#include "MtConfig.h"

#define DRIVER_FILE         0x00100000

/*  
    ==========================================================================
    Description: 
        action state machine init. Currently there are 8 category defined in 802.11spec.  We use the category value definition to 
        init corresponding action function in action state machine.
        
    Parameters: 
        S - pointer to the association state machine
    Note:
        The state machine looks like the following as name implies its function
    ==========================================================================
 */
VOID ActionStateMachineInit(
    IN  PMP_ADAPTER   pAd, 
    IN  STATE_MACHINE *S, 
    OUT STATE_MACHINE_FUNC Trans[]) 
{
    StateMachineInit(S, Trans, MAX_ACT_STATE, MAX_ACT_MSG, Drop, ACT_IDLE, ACT_MACHINE_BASE);

    StateMachineSetAction(S, ACT_IDLE, MT2_PEER_SPECTRUM_CATE, PeerSpectrumAction);
    StateMachineSetAction(S, ACT_IDLE, MT2_PEER_QOS_CATE, PeerQOSAction);   
    StateMachineSetAction(S, ACT_IDLE, MT2_PEER_BA_CATE, PeerBAAction);
    StateMachineSetAction(S, ACT_IDLE, MT2_PEER_PUBLIC_CATE, PeerPublicAction);
    StateMachineSetAction(S, ACT_IDLE, MT2_PEER_RM_CATE, PeerRMAction); 
    StateMachineSetAction(S, ACT_IDLE, MT2_PEER_HT_CATE, PeerHTAction);
    StateMachineSetAction(S, ACT_IDLE, MT2_PEER_VHT_CATE, PeerVHTAction);
    
    StateMachineSetAction(S, ACT_IDLE, MT2_MLME_ADD_BA_CATE, MlmeADDBAAction);
    StateMachineSetAction(S, ACT_IDLE, MT2_MLME_ORI_DELBA_CATE, MlmeDELBAAction);
    StateMachineSetAction(S, ACT_IDLE, MT2_MLME_REC_DELBA_CATE, MlmeDELBAAction);
    StateMachineSetAction(S, ACT_IDLE, MT2_MLME_QOS_CATE, MlmeQOSAction);   
    StateMachineSetAction(S, ACT_IDLE, MT2_MLME_VHT_OPERATING_MODE_NOTIFICATION, MlmeVhtOperatingModeNotificationAction);
    StateMachineSetAction(S, ACT_IDLE, MT2_ACT_INVALID, MlmeInvalidAction);
#if(COMPILE_WIN8_ABOVE(CURRENT_OS_NDIS_VER))
#ifdef WFD_NEW_PUBLIC_ACTION    
    StateMachineSetAction(S, ACT_IDLE, MT2_MLME_ACTION_CATE, MlmeP2pMsPublicActionFrameStateMachine);
#endif /*WFD_NEW_PUBLIC_ACTION*/
#endif
    // Regarding to DLS, it has been moved to DLS state machine
}

VOID MlmeADDBAAction(
    IN PMP_ADAPTER pAd, 
    IN MLME_QUEUE_ELEM *Elem) 
{
    MLME_ADDBA_REQ_STRUCT *pInfo;
    UCHAR           Addr[6];
    PUCHAR         pOutBuffer = NULL;
    NDIS_STATUS     NStatus;
    ULONG       Idx;
    FRAME_ADDBA_REQ  Frame;
    ULONG       FrameLen;
    UCHAR           Bssid[MAC_ADDR_LEN];
    PMP_PORT  pPort = pAd->PortList[Elem->PortNum];
    PMAC_TABLE_ENTRY pWcidMacTabEntry = NULL;
    
    pInfo = (MLME_ADDBA_REQ_STRUCT *)Elem->Msg; 
    PlatformZeroMemory(&Frame, sizeof(FRAME_ADDBA_REQ));
    
    pWcidMacTabEntry = MlmeSyncMacTabMatchedWlanIdx(pPort, pInfo->Wcid);
    if(pWcidMacTabEntry == NULL)
    {
        DBGPRINT(RT_DEBUG_TRACE,("%s(), Line:%d, Mac Entry is NULL  Wcid = %d\n", __FUNCTION__, __LINE__,pInfo->Wcid));
        return;
    }
    
    if(MlmeAddBAReqSanity(pAd, Elem->Msg, Elem->MsgLen, Addr)) 
    {
        NStatus = MlmeAllocateMemory(pAd, &pOutBuffer);  //Get an unused nonpaged memory
        if(NStatus != NDIS_STATUS_SUCCESS) 
        {
            DBGPRINT(RT_DEBUG_TRACE,("BA - MlmeADDBAAction() allocate memory failed \n"));
            return;
        }
        // 1. Prepare ADDBA REQ frame. Parameters in ADDBA REQ frame is advisory.
        Idx = pWcidMacTabEntry->BAOriWcidArray[pInfo->TID];

        COPY_MAC_ADDR(Bssid, pPort->PortCfg.Bssid);

        ActHeaderInit(pAd, pPort, &Frame.Hdr, pInfo->pAddr, Bssid);
        
        Frame.Category = CATEGORY_BA;
        Frame.Action = ADDBA_REQ;
        Frame.BaParm.AMSDU = 0;
        Frame.BaParm.BAPolicy = IMMED_BA;
        Frame.BaParm.TID = pInfo->TID;
        Frame.BaParm.BufSize = pInfo->BaBufSize;
        Frame.Token = pInfo->Token;
        Frame.TimeOutValue = pInfo->TimeOutValue;
        Frame.BaStartSeq.field.FragNum = 0;
        Frame.BaStartSeq.field.StartSeq = pWcidMacTabEntry->BAOriSequence[pInfo->TID];
        DBGPRINT(RT_DEBUG_TRACE,("MlmeADDBAAction - ---->pInfo->Wcid = %d, TID = %d,  BAOriSequence = %x, pInfo->BaBufSize = %d\n",pInfo->Wcid, pInfo->TID, pWcidMacTabEntry->BAOriSequence[pInfo->TID], pInfo->BaBufSize));
        
        MakeOutgoingFrame(pOutBuffer,               &FrameLen,
                      sizeof(FRAME_ADDBA_REQ),    &Frame,
                      END_OF_ARGS);
        NdisCommonMiniportMMRequest(pAd, pOutBuffer, FrameLen);
    }
}

/*
    ==========================================================================
    Description:
        send DELBA and delete BaEntry if any
    Parametrs:
        Elem - MLME message MLME_DELBA_REQ_STRUCT
    
    IRQL = DISPATCH_LEVEL

    ==========================================================================
 */
VOID MlmeDELBAAction(
    IN PMP_ADAPTER pAd, 
    IN MLME_QUEUE_ELEM *Elem) 
{
    MLME_DELBA_REQ_STRUCT *pInfo;
    PUCHAR         pOutBuffer = NULL;
    PUCHAR         pOutBuffer2 = NULL;
    PUCHAR         pOutBuffer3 = NULL;
    NDIS_STATUS     NStatus;
    ULONG       Idx;
    FRAME_DELBA_REQ  Frame;
    ULONG       FrameLen;
#if (DISABLE_SEND_BAR_AFTER_DISASSOC == 0 )
    FRAME_BAR   FrameBar;
#endif
    UCHAR               ZeroAddr[6];
    UCHAR               Bssid[MAC_ADDR_LEN];
    PMP_PORT  pPort = pAd->PortList[Elem->PortNum];
    PMAC_TABLE_ENTRY pWcidMacTabEntry = NULL;

    pInfo = (MLME_DELBA_REQ_STRUCT *)Elem->Msg; 
    pWcidMacTabEntry = MlmeSyncMacTabMatchedWlanIdx(pPort, pInfo->Wcid); 
    if(pWcidMacTabEntry == NULL)
    {
        DBGPRINT(RT_DEBUG_TRACE,("%s(), Line:%d, Mac Entry is NULL  Wcid = %d\n", __FUNCTION__, __LINE__,pInfo->Wcid));
        return;
    }
    
    // must send back DELBA 
    PlatformZeroMemory(&Frame, sizeof(FRAME_DELBA_REQ));
    PlatformZeroMemory(ZeroAddr, 6);
    
    if(MlmeDelBAReqSanity(pAd, Elem->Msg, Elem->MsgLen)) 
    {
        if (!PlatformEqualMemory(pInfo->Addr, ZeroAddr, 6))
        {
            NStatus = MlmeAllocateMemory(pAd, &pOutBuffer2);  //Get an unused nonpaged memory
            if(NStatus != NDIS_STATUS_SUCCESS) 
            {
                DBGPRINT(RT_DEBUG_TRACE,("BA - MlmeDELBAAction() allocate memory failed \n"));
                return;
            }

            // Send BAR (Send BAR to refresh peer reordering buffer sequence index.)
            Idx = pWcidMacTabEntry->BAOriWcidArray[pInfo->TID];
#if (DISABLE_SEND_BAR_AFTER_DISASSOC == 0 )
            BarHeaderInit(pAd,pPort,&FrameBar, pInfo->Addr);
            FrameBar.StartingSeq.field.FragNum = 0;  
            FrameBar.StartingSeq.field.StartSeq = pInfo->BAOriSequence;//pPort->MacTab.Content[pInfo->Wcid].BAOriSequence[pInfo->TID];  
            FrameBar.BarControl.TID = pPort->BATable.BAOriEntry[Idx].TID;  
            FrameBar.BarControl.ACKPolicy = IMMED_BA;   // mandatory use Immediate BA Policy
            FrameBar.BarControl.Compressed = 1;  // mandatory use Compressed 
            FrameBar.BarControl.MTID = 0;  
            MakeOutgoingFrame(pOutBuffer2,              &FrameLen,
                          sizeof(FRAME_BAR),      &FrameBar,
                          END_OF_ARGS);
            //Send BAR to refresh peer reordering buffer.
            NdisCommonMiniportMMRequest(pAd, pOutBuffer2, FrameLen);
            DBGPRINT(RT_DEBUG_TRACE,("BA - MlmeDELBAAction() . Send BAR to refresh peer reordering buffer \n"));
#endif
            COPY_MAC_ADDR(Bssid, pPort->PortCfg.Bssid);

            // Send DELBA action frame
            FrameLen = 0;
            NStatus = MlmeAllocateMemory(pAd, &pOutBuffer);  //Get an unused nonpaged memory
            if(NStatus != NDIS_STATUS_SUCCESS) 
            {
                DBGPRINT(RT_DEBUG_TRACE,("BA - MlmeDELBAAction() allocate memory failed \n"));
                return;
            }
            NStatus = MlmeAllocateMemory(pAd, &pOutBuffer3);  //Get an unused nonpaged memory
            if(NStatus != NDIS_STATUS_SUCCESS) 
            {
                DBGPRINT(RT_DEBUG_TRACE,("BA - MlmeDELBAAction() allocate memory failed \n"));
                return;
            }
            ActHeaderInit(pAd, pPort, &Frame.Hdr, pInfo->Addr, Bssid);
            Frame.Category = CATEGORY_BA;
            Frame.Action = DELBA;
            Frame.DelbaParm.Initiator = pInfo->Initiator;
            Frame.DelbaParm.TID = pInfo->TID;
            MakeOutgoingFrame(pOutBuffer,               &FrameLen,
                              sizeof(FRAME_DELBA_REQ),    &Frame,
                              END_OF_ARGS);
            NdisCommonMiniportMMRequest(pAd, pOutBuffer, FrameLen);
        
            MakeOutgoingFrame(pOutBuffer3,               &FrameLen,
                              sizeof(FRAME_DELBA_REQ),    &Frame,
                              END_OF_ARGS);
            NdisCommonMiniportMMRequest(pAd, pOutBuffer3, FrameLen);
        }
    }
}


VOID MlmeQOSAction(
    IN PMP_ADAPTER pAd, 
    IN MLME_QUEUE_ELEM *Elem) 
{
}


VOID MlmeInvalidAction(
    IN PMP_ADAPTER pAd, 
    IN MLME_QUEUE_ELEM *Elem) 
{
    PUCHAR         pOutBuffer = NULL;
    //Return the receiving frame except the MSB of category filed set to 1.  7.3.1.11
}

#if(COMPILE_WIN8_ABOVE(CURRENT_OS_NDIS_VER))
#ifdef WFD_NEW_PUBLIC_ACTION
VOID MlmeP2pMsNullState(
    IN PMP_ADAPTER pAd,
    IN PMP_PORT     pPort,
    IN  PUCHAR  pInBuffer,
    IN  ULONG   InBufferLen)    
{
    PP2P_PUBLIC_FRAME   pFrame = NULL;
    BOOLEAN TimerCancelled = FALSE;
    P2P_PUBLIC_FRAME    Frame;
    DBGPRINT(RT_DEBUG_TRACE, ("%s ===> signal = %d  sendfram = %d\n", __FUNCTION__, pPort->P2PCfg.Signal, pPort->P2PCfg.SentFrame));

    // waiting ACK and really get ACK.
    if((pPort->P2PCfg.CurrState == P2P_WAIT_ACK_STATE) && (pPort->P2PCfg.Signal == P2P_GOT_ACK))
    {
        
       
    // 1.1 Stop ACK timer
        PlatformCancelTimer(&pPort->Mlme.P2pMsPublicActionFrameTimer, &TimerCancelled);
        // ACK done from FIFO register
        PlatformCancelTimer(&pPort->Mlme.P2pMsTriggerReadTxFifoTimer, &TimerCancelled);
    
        pPort->P2PCfg.LostAckRetryCounter = 0;
        
        if ((pPort->P2PCfg.PrevState == P2P_SEND_STATE))
        {
            pPort->P2PCfg.CurrState = P2P_NULL_STATE;
            pPort->P2PCfg.PrevState = P2P_NULL_STATE;
        }
        
        //pPort->P2PCfg.PrevState = TempPrevState;

        DBGPRINT(RT_DEBUG_TRACE, ("%s - GET ACK Set PrevState(%s), CurrState(%s)\n", 
                                    __FUNCTION__, 
                                    MlmeP2pMsStateDecode(pPort->P2PCfg.PrevState), 
                                    MlmeP2pMsStateDecode(pPort->P2PCfg.CurrState)));
        // 1.4 Indicate Req(ok) to OS
        if (pPort->P2PCfg.SentFrame != P2P_ACTION_SEND_COMPLETE)
        {
#ifdef DBG              
            P2pMsIndicateFrameSendComplete(pAd, pPort, 
                                        (PUCHAR) pPort->P2PCfg.pRequesterBufferedFrame, pPort->P2PCfg.RequesterBufferedSize, NDIS_STATUS_SUCCESS, __LINE__);
#else
            P2pMsIndicateFrameSendComplete(pAd, pPort, 
                                        (PUCHAR) pPort->P2PCfg.pRequesterBufferedFrame, pPort->P2PCfg.RequesterBufferedSize, NDIS_STATUS_SUCCESS);
#endif
            // Assign sent frame
            pPort->P2PCfg.PrevSentFrame = pPort->P2PCfg.SentFrame;
            pPort->P2PCfg.SentFrame = P2P_ACTION_SEND_COMPLETE;
        }
        else
        {
            DBGPRINT(RT_DEBUG_TRACE, ("%s - Already sent P2pMsIndicateFrameSendComplete(%d) to OS before, ignore this one\n", __FUNCTION__, pPort->P2PCfg.SentFrame));
        }
        return;
    }
    // retry
    else if((pPort->P2PCfg.CurrState == P2P_WAIT_ACK_STATE) && (pPort->P2PCfg.Signal == pPort->P2PCfg.SentFrame))
    {
        if (pPort->P2PCfg.LostRspRetryCounter < WFD_PUBLIC_ACTION_FRAME_RETRY_COUNT_MAX)
        {
            DBGPRINT(RT_DEBUG_TRACE, ("%s - P2P_NULL_STATE --> Send P2P Action Timeout --> LostACKRetryCounter(%d) last frame\n", __FUNCTION__, pPort->P2PCfg.LostRspRetryCounter));
            pPort->P2PCfg.PrevState = pPort->P2PCfg.CurrState;
            pPort->P2PCfg.CurrState = P2P_SEND_STATE;
            pPort->P2PCfg.Signal = pPort->P2PCfg.SentFrame;
            pPort->P2PCfg.LostRspRetryCounter++;
            pPort->P2PCfg.LostAckRetryCounter = 0;
            Frame.Subtype = pPort->P2PCfg.Signal;
            MlmeEnqueue(pAd, pAd->PortList[pPort->P2PCfg.PortNumber], ACTION_STATE_MACHINE, MT2_MLME_ACTION_CATE, sizeof(P2P_PUBLIC_FRAME), &Frame);
        }
        else
        {
#ifdef DBG              
            P2pMsIndicateFrameSendComplete(pAd, pPort, 
                                        (PUCHAR) pPort->P2PCfg.pRequesterBufferedFrame, pPort->P2PCfg.RequesterBufferedSize, NDIS_STATUS_SUCCESS, __LINE__);
#else
            P2pMsIndicateFrameSendComplete(pAd, pPort, 
                                        (PUCHAR) pPort->P2PCfg.pRequesterBufferedFrame, pPort->P2PCfg.RequesterBufferedSize, NDIS_STATUS_SUCCESS);
#endif
            pPort->P2PCfg.CurrState = P2P_NULL_STATE;
            pPort->P2PCfg.PrevState = P2P_NULL_STATE;
            pPort->P2PCfg.LostRspRetryCounter = 0;
            pPort->P2PCfg.LostAckRetryCounter = 0;
        }
    }
    // waiting ACK but maybe ACK lost in Air.
    // this time I receive Response so indicate "send Request first" and go to following choice to indicate which receive Response
    else if((pPort->P2PCfg.CurrState == P2P_WAIT_ACK_STATE))
    {
        
        PlatformCancelTimer(&pPort->Mlme.P2pMsPublicActionFrameTimer, &TimerCancelled);
        // no ACK from FIFO register
        PlatformCancelTimer(&pPort->Mlme.P2pMsTriggerReadTxFifoTimer, &TimerCancelled); 
        
        pPort->P2PCfg.LostAckRetryCounter = 0;

        if ((pPort->P2PCfg.PrevState == P2P_SEND_STATE))
        {
            pPort->P2PCfg.CurrState = P2P_NULL_STATE;
            pPort->P2PCfg.PrevState = P2P_NULL_STATE;
        }
        
        //pPort->P2PCfg.PrevState = TempPrevState;

        DBGPRINT(RT_DEBUG_TRACE, ("%s - Set PrevState(%s), CurrState(%s)\n", 
                                    __FUNCTION__, 
                                    MlmeP2pMsStateDecode(pPort->P2PCfg.PrevState), 
                                    MlmeP2pMsStateDecode(pPort->P2PCfg.CurrState)));
        // 1.4 Indicate Req(ok) to OS
        if (pPort->P2PCfg.SentFrame != P2P_ACTION_SEND_COMPLETE)
        {
            // Most time, USB can't get  ACK immediately before triggering tx fifo
            // We disable periodic ACK check and reset tx fifo for the next sendframe used
            pPort->P2PCfg.bP2pAckReq = FALSE;
            MTEnqueueInternalCmd(pAd, pPort,  MT_CMD_P2P_READ_TXFIFO, NULL, 0);
        
#ifdef DBG              
            P2pMsIndicateFrameSendComplete(pAd, pPort, 
                                        (PUCHAR) pPort->P2PCfg.pRequesterBufferedFrame, pPort->P2PCfg.RequesterBufferedSize, NDIS_STATUS_SUCCESS, __LINE__);
#else
            P2pMsIndicateFrameSendComplete(pAd, pPort, 
                                        (PUCHAR) pPort->P2PCfg.pRequesterBufferedFrame, pPort->P2PCfg.RequesterBufferedSize, NDIS_STATUS_SUCCESS);
#endif
            // Assign sent frame
            pPort->P2PCfg.PrevSentFrame = pPort->P2PCfg.SentFrame;
            pPort->P2PCfg.SentFrame = P2P_ACTION_SEND_COMPLETE;
        }
        else
        {
            DBGPRINT(RT_DEBUG_TRACE, ("%s - Already sent P2pMsIndicateFrameSendComplete(%d) to OS before, ignore this one\n", __FUNCTION__, pPort->P2PCfg.SentFrame));
        }
    }
    
    if (pInBuffer != NULL)
    {
        DBGPRINT(RT_DEBUG_TRACE, ("%s (Elem != NULL).\n", __FUNCTION__));
        
        pFrame = (PP2P_PUBLIC_FRAME)pInBuffer;

        // Send
        if (MAC_ADDR_EQUAL(pFrame->p80211Header.Addr2, pAd->PortList[pPort->P2PCfg.PortNumber]->CurrentAddress))
        {
            
        }
        // Receive req/resp
        else if (MAC_ADDR_EQUAL(pFrame->p80211Header.Addr1, pAd->PortList[pPort->P2PCfg.PortNumber]->CurrentAddress))
        {           
            pPort->P2PCfg.PrevState = pPort->P2PCfg.CurrState;  
            pPort->P2PCfg.CurrState = P2P_NULL_STATE;

            DBGPRINT(RT_DEBUG_TRACE, ("%s start.\n", __FUNCTION__));
            
            // Indicate Req receive to OS   
            switch (pPort->P2PCfg.Signal)
            {
                case GO_NEGOCIATION_REQ:
                    P2pMsIndicateGONegoReqRecv(pAd, pAd->PortList[pPort->P2PCfg.PortNumber], pInBuffer, InBufferLen);
                    break;
                case GO_NEGOCIATION_CONFIRM:
                    P2pMsIndicateGONegoConfirmRecv(pAd, pAd->PortList[pPort->P2PCfg.PortNumber], pInBuffer, InBufferLen);
                    break;
                case P2P_INVITE_REQ:
                    P2pMsIndicateInviteReqRecv(pAd, pAd->PortList[pPort->P2PCfg.PortNumber], pInBuffer, InBufferLen);
                    break;
                case P2P_PROVISION_REQ:
                    P2pMsIndicateProvDiscoReqRecv(pAd, pAd->PortList[pPort->P2PCfg.PortNumber], pInBuffer, InBufferLen);
                    break;
                case GO_NEGOCIATION_RSP:
                    P2pMsIndicateGONegoRspRecv(pAd, pAd->PortList[pPort->P2PCfg.PortNumber], pInBuffer, InBufferLen);
                    break;
                case P2P_PROVISION_RSP:
                    P2pMsIndicateProvDiscoRspRecv(pAd, pAd->PortList[pPort->P2PCfg.PortNumber], pInBuffer, InBufferLen);
                    break;
                case P2P_INVITE_RSP:
                    P2pMsIndicateInviteRspRecv(pAd, pAd->PortList[pPort->P2PCfg.PortNumber], pInBuffer, InBufferLen);
                    break;
            
                default:
                    DBGPRINT(RT_DEBUG_TRACE, ("%s - No match Signal case(%s)\n", __FUNCTION__, MlmeP2pMsSignalDecode(pPort->P2PCfg.Signal)));
                    break;
            }
            DBGPRINT(RT_DEBUG_TRACE, ("%s - Set PrevState(%s), CurrState(%s)\n", 
                                        __FUNCTION__, 
                                        MlmeP2pMsStateDecode(pPort->P2PCfg.PrevState), 
                                        MlmeP2pMsStateDecode(pPort->P2PCfg.CurrState)));
        }
        else
        {
            DBGPRINT(RT_DEBUG_TRACE, ("%s (1) didn't handle Signal(%s).\n", __FUNCTION__, MlmeP2pMsSignalDecode(pPort->P2PCfg.Signal)));
        }
    }
    else
    {
        DBGPRINT(RT_DEBUG_TRACE, ("%s (2) didn't handle Signal(%s).\n", __FUNCTION__, MlmeP2pMsSignalDecode(pPort->P2PCfg.Signal)));
    }

    DBGPRINT(RT_DEBUG_TRACE, ("%s <===\n", __FUNCTION__));
}

VOID MlmeP2pMsSendState(
    IN PMP_ADAPTER pAd,
    IN PMP_PORT     pPort,
    IN  PUCHAR  pInBuffer,
    IN  ULONG   InBufferLen)    
{
    
    PUCHAR      SendOutAction = NULL, SendOutAction2 = NULL;
    NDIS_STATUS     NStatus, NStatus2;
    UCHAR           TempPid;
    static UCHAR PrevSignal = P2P_NULL_STATE;
    //PMP_PORT    pPort = pAd->PortList[FXXK_PORT_0];
    
    
    DBGPRINT(RT_DEBUG_TRACE, ("%s ===>\n", __FUNCTION__));
    
    switch (pPort->P2PCfg.Signal)
    {
        case GO_NEGOCIATION_REQ:
        case GO_NEGOCIATION_RSP:
        case GO_NEGOCIATION_CONFIRM:
        case P2P_INVITE_REQ:
        case P2P_INVITE_RSP:
        case P2P_PROVISION_REQ:
        case P2P_PROVISION_RSP:
            // 1. Reset retry times variable if necessary
            /* Use ACK from tx fifo instead */
            /*if((pPort->P2PCfg.Signal == GO_NEGOCIATION_RSP) ||
                    (pPort->P2PCfg.Signal == GO_NEGOCIATION_CONFIRM) ||
                    (pPort->P2PCfg.Signal == P2P_INVITE_RSP) ||
                    (pPort->P2PCfg.Signal == P2P_PROVISION_RSP))
            {
                pPort->P2PCfg.LostAckRetryCounter = WFD_PUBLIC_ACTION_FRAME_RETRY_COUNT_MAX;
                pPort->P2PCfg.LostRspRetryCounter = WFD_PUBLIC_ACTION_FRAME_RETRY_COUNT_MAX;

            }
            else*/      
#if 1
            // 1. Reset retry times variable if necessary
            if (pPort->P2PCfg.Signal != PrevSignal)
            {
                pPort->P2PCfg.LostAckRetryCounter = 0;
                pPort->P2PCfg.LostRspRetryCounter = 0;
                PrevSignal = pPort->P2PCfg.Signal;
                DBGPRINT(RT_DEBUG_TRACE, ("%s - Reset Retry counter\n", __FUNCTION__));     
            }
#else
            {
                pPort->P2PCfg.LostAckRetryCounter = 0;
                pPort->P2PCfg.LostRspRetryCounter = 0;
            }
#endif
            DBGPRINT(RT_DEBUG_TRACE, (" signal(%d) LostAckRetryCounter = %d", pPort->P2PCfg.Signal, pPort->P2PCfg.LostAckRetryCounter));
            
            if((pPort->P2PCfg.pRequesterBufferedFrame != NULL) && (pPort->P2PCfg.RequesterBufferedSize > 0))
            {
                DBGPRINT(RT_DEBUG_TRACE, ("%s - Send out Public Action frame(len:%d)\n", __FUNCTION__, pPort->P2PCfg.RequesterBufferedSize));

                NStatus = MlmeAllocateMemory(pAd, &SendOutAction);

                if (NStatus == NDIS_STATUS_SUCCESS)
                {
                    PlatformMoveMemory(SendOutAction, pPort->P2PCfg.pRequesterBufferedFrame, pPort->P2PCfg.RequesterBufferedSize);
                    // 2. Send out bufer frame
                    NdisCommonMiniportMMRequest(pAd, SendOutAction, pPort->P2PCfg.RequesterBufferedSize);
                    
                    // 3. Go to Wait ACK State
                    pPort->P2PCfg.PrevState = pPort->P2PCfg.CurrState;
                    pPort->P2PCfg.CurrState = P2P_WAIT_ACK_STATE;
                    
                    //Set timer for reading TXFIFO (after 1~10ms)
                    PlatformSetTimer(pPort, &pPort->Mlme.P2pMsTriggerReadTxFifoTimer, 1);
                    
                    // 4. Set Wait ACK timeout timer
                    //PlatformSetTimer(pPort, &pPort->Mlme.P2pMsPublicActionFrameTimer, WFD_WAIT_ACK_TIMEOUT);
                    DBGPRINT(RT_DEBUG_TRACE, ("%s - Set PrevState(%s), CurrState(%s)\n", 
                                                __FUNCTION__, 
                                                MlmeP2pMsStateDecode(pPort->P2PCfg.PrevState), 
                                                MlmeP2pMsStateDecode(pPort->P2PCfg.CurrState)));
                    pPort->P2PCfg.SentFrame = pPort->P2PCfg.Signal;
                }
                else
                {
                    DBGPRINT(RT_DEBUG_TRACE, ("%s: 1. Alloc Mem fail. Try again \n", __FUNCTION__));

                    NStatus2 = MlmeAllocateMemory(pAd, &SendOutAction2);

                    if (NStatus2 == NDIS_STATUS_SUCCESS)
                    {
                        PlatformMoveMemory(SendOutAction2, pPort->P2PCfg.pRequesterBufferedFrame, pPort->P2PCfg.RequesterBufferedSize);
                        // 2. Send out bufer frame
                        NdisCommonMiniportMMRequest(pAd, SendOutAction2, pPort->P2PCfg.RequesterBufferedSize);
                        
                        // 3. Go to Wait ACK State
                        pPort->P2PCfg.PrevState = pPort->P2PCfg.CurrState;
                        pPort->P2PCfg.CurrState = P2P_WAIT_ACK_STATE;
                        
                        // 4. Set Wait ACK timeout timer
                        PlatformSetTimer(pPort, &pPort->Mlme.P2pMsPublicActionFrameTimer, WFD_WAIT_ACK_TIMEOUT);

                        //Set timer for reading TXFIFO (after 1~10ms)
                        PlatformSetTimer(pPort, &pPort->Mlme.P2pMsTriggerReadTxFifoTimer, 1);

                        DBGPRINT(RT_DEBUG_TRACE, ("%s - Set PrevState(%s), CurrState(%s)\n", 
                                                    __FUNCTION__, 
                                                    MlmeP2pMsStateDecode(pPort->P2PCfg.PrevState), 
                                                    MlmeP2pMsStateDecode(pPort->P2PCfg.CurrState)));
                        pPort->P2PCfg.SentFrame = pPort->P2PCfg.Signal;
                    }
                    else
                    {
                        DBGPRINT(RT_DEBUG_TRACE, ("%s: 2. Alloc Mem fail, indicate error to OS.\n", __FUNCTION__));
                        // Set waiting complete flag and then report the failure to OS
                        P2pMsAckRequiredCheck(pAd, pPort, (PP2P_PUBLIC_FRAME)pPort->P2PCfg.pRequesterBufferedFrame, &TempPid);
#ifdef DBG              
                        P2pMsIndicateFrameSendComplete(pAd, pPort, 
                                                    (PUCHAR) pPort->P2PCfg.pRequesterBufferedFrame, pPort->P2PCfg.RequesterBufferedSize, NDIS_STATUS_FAILURE, __LINE__);
#else
                        P2pMsIndicateFrameSendComplete(pAd, pPort, 
                                                    (PUCHAR) pPort->P2PCfg.pRequesterBufferedFrame, pPort->P2PCfg.RequesterBufferedSize, NDIS_STATUS_FAILURE);
#endif

                    }
                }   
            }
            else
            {
                pPort->P2PCfg.PrevState = P2P_NULL_STATE; 
                pPort->P2PCfg.CurrState = P2P_NULL_STATE;
                DBGPRINT(RT_DEBUG_TRACE, ("%s - Set PrevState(%s), CurrState(%s), Fail!\n", 
                                            __FUNCTION__, 
                                            MlmeP2pMsStateDecode(pPort->P2PCfg.PrevState), 
                                            MlmeP2pMsStateDecode(pPort->P2PCfg.CurrState)));
            }           
            break;
        default:
            DBGPRINT(RT_DEBUG_TRACE, ("%s - No match Signal case(%s)\n", __FUNCTION__, MlmeP2pMsSignalDecode(pPort->P2PCfg.Signal)));
            break;
    }
    DBGPRINT(RT_DEBUG_TRACE, ("%s <===\n", __FUNCTION__));
}

VOID MlmeP2pMsWaitAckState(
    IN PMP_ADAPTER pAd,
    IN  PUCHAR  pInBuffer,
    IN  ULONG   InBufferLen)
{
    BOOLEAN TimerCancelled = FALSE;
    UCHAR   TempPrevState = P2P_NULL_STATE;
    PMP_PORT pPort = pAd->PortList[FXXK_PORT_0];
    DBGPRINT(RT_DEBUG_TRACE, ("%s ===>\n", __FUNCTION__));
    
    switch (pPort->P2PCfg.Signal)
    {
        // 1. Get ACK
        case P2P_GOT_ACK:
            // 1.1 Stop ACK timer
            PlatformCancelTimer(&pPort->Mlme.P2pMsPublicActionFrameTimer, &TimerCancelled);
            // ACK done from FIFO register
            PlatformCancelTimer(&pPort->Mlme.P2pMsTriggerReadTxFifoTimer, &TimerCancelled);
            
            pPort->P2PCfg.LostAckRetryCounter = 0;
            
            // 1.2 Assign state
            TempPrevState = pPort->P2PCfg.CurrState;

            if ((pPort->P2PCfg.PrevState == P2P_SEND_STATE) && 
                ((pPort->P2PCfg.SentFrame == GO_NEGOCIATION_RSP) ||
                (pPort->P2PCfg.SentFrame == P2P_INVITE_RSP) ||
                (pPort->P2PCfg.SentFrame == P2P_PROVISION_RSP) ||
                (pPort->P2PCfg.SentFrame == GO_NEGOCIATION_CONFIRM)))
            {
                pPort->P2PCfg.CurrState = P2P_NULL_STATE;
            }
            else
            {
                // 1.3 Set Wait Rsp timeout timer
                pPort->P2PCfg.CurrState = P2P_WAIT_RSP_STATE;
                PlatformSetTimer(pPort, &pPort->Mlme.P2pMsPublicActionFrameTimer, WFD_WAIT_RSP_TIMEOUT);
            }
            
            pPort->P2PCfg.PrevState = TempPrevState;

            DBGPRINT(RT_DEBUG_TRACE, ("%s - Set PrevState(%s), CurrState(%s)\n", 
                                        __FUNCTION__, 
                                        MlmeP2pMsStateDecode(pPort->P2PCfg.PrevState), 
                                        MlmeP2pMsStateDecode(pPort->P2PCfg.CurrState)));
            // 1.4 Indicate Req(ok) to OS
            if (pPort->P2PCfg.SentFrame != pPort->P2PCfg.PrevSentFrame)
            {
#ifdef DBG              
                P2pMsIndicateFrameSendComplete(pAd, pPort, 
                                            (PUCHAR) pPort->P2PCfg.pRequesterBufferedFrame, pPort->P2PCfg.RequesterBufferedSize, NDIS_STATUS_SUCCESS, __LINE__);
#else
                P2pMsIndicateFrameSendComplete(pAd, pPort, 
                                            (PUCHAR) pPort->P2PCfg.pRequesterBufferedFrame, pPort->P2PCfg.RequesterBufferedSize, NDIS_STATUS_SUCCESS);
#endif
                // Assign sent frame
                pPort->P2PCfg.PrevSentFrame = pPort->P2PCfg.SentFrame;
            }
            else
            {
                DBGPRINT(RT_DEBUG_TRACE, ("%s - Already sent P2pMsIndicateFrameSendComplete(%d) to OS before, ignore this one\n", __FUNCTION__, pPort->P2PCfg.SentFrame));
            }
            // It might fail to get RSP, and we still need pRequesterBufferedFrame to do retry and won't free pRequesterBufferedFrame here
            break;
            
        // 2. Get RSP or Cfm
        case GO_NEGOCIATION_RSP:
        case P2P_PROVISION_RSP:
        case P2P_INVITE_RSP:
        case GO_NEGOCIATION_CONFIRM:
            MlmeP2pMsWiatRspState(pAd, pInBuffer, InBufferLen);
            break;

        case GO_NEGOCIATION_REQ:
        case P2P_PROVISION_REQ:
            // 1.1 Stop ACK timer
            PlatformCancelTimer(&pPort->Mlme.P2pMsPublicActionFrameTimer, &TimerCancelled);
            PlatformCancelTimer(&pPort->Mlme.P2pMsTriggerReadTxFifoTimer, &TimerCancelled);
            
            pPort->P2PCfg.LostAckRetryCounter = 0;

            if ((pPort->P2PCfg.CurrState == P2P_WAIT_ACK_STATE) && (pPort->P2PCfg.SentFrame != pPort->P2PCfg.PrevSentFrame))
            {
                // Indicate Req(ok) to OS
#ifdef DBG              
                P2pMsIndicateFrameSendComplete(pAd, pPort, 
                                            (PUCHAR) pPort->P2PCfg.pRequesterBufferedFrame, pPort->P2PCfg.RequesterBufferedSize, NDIS_STATUS_SUCCESS, __LINE__);
#else
                P2pMsIndicateFrameSendComplete(pAd, pPort, 
                                            (PUCHAR) pPort->P2PCfg.pRequesterBufferedFrame, pPort->P2PCfg.RequesterBufferedSize, NDIS_STATUS_SUCCESS);
#endif
                // Assign sent frame
                pPort->P2PCfg.PrevSentFrame = pPort->P2PCfg.SentFrame;

                // Free Old Buffered Data
                P2pMsFreeIeMemory(&pPort->P2PCfg.pRequesterBufferedFrame, (PLONG)&pPort->P2PCfg.RequesterBufferedSize);     
            }
            
            switch (pPort->P2PCfg.Signal)
            {
                case GO_NEGOCIATION_REQ:
                    P2pMsIndicateGONegoReqRecv(pAd, pAd->PortList[pPort->P2PCfg.PortNumber], pInBuffer, InBufferLen);
                    break;
                case GO_NEGOCIATION_CONFIRM:
                    P2pMsIndicateGONegoConfirmRecv(pAd, pAd->PortList[pPort->P2PCfg.PortNumber], pInBuffer, InBufferLen);
                    break;
                case P2P_INVITE_REQ:
                    P2pMsIndicateInviteReqRecv(pAd, pAd->PortList[pPort->P2PCfg.PortNumber], pInBuffer, InBufferLen);
                    break;
                case P2P_PROVISION_REQ:
                    P2pMsIndicateProvDiscoReqRecv(pAd, pAd->PortList[pPort->P2PCfg.PortNumber], pInBuffer, InBufferLen);
                    break;
                default:
                    DBGPRINT(RT_DEBUG_TRACE, ("%s - No match Signal case(%s)\n", __FUNCTION__, MlmeP2pMsSignalDecode(pPort->P2PCfg.Signal)));
                    break;
            }   
            pPort->P2PCfg.PrevState = pPort->P2PCfg.CurrState;
            pPort->P2PCfg.CurrState = P2P_NULL_STATE;
            DBGPRINT(RT_DEBUG_TRACE, ("%s - Set PrevState(%s), CurrState(%s)\n", 
                                        __FUNCTION__, 
                                        MlmeP2pMsStateDecode(pPort->P2PCfg.PrevState), 
                                        MlmeP2pMsStateDecode(pPort->P2PCfg.CurrState)));          
        break;
        
        default:
            DBGPRINT(RT_DEBUG_TRACE, ("%s - No match Signal case(%s)\n", __FUNCTION__, MlmeP2pMsSignalDecode(pPort->P2PCfg.Signal)));
            break;
    }   
    
    DBGPRINT(RT_DEBUG_TRACE, ("%s <===\n", __FUNCTION__));
}

VOID MlmeP2pMsWiatRspState(
    IN PMP_ADAPTER pAd,
    IN  PUCHAR  pInBuffer,
    IN  ULONG   InBufferLen)
{
    BOOLEAN TimerCancelled = FALSE;
    PMP_PORT pPort = pAd->PortList[FXXK_PORT_0];
    DBGPRINT(RT_DEBUG_TRACE, ("%s ===>\n", __FUNCTION__));

    PlatformCancelTimer(&pPort->Mlme.P2pMsPublicActionFrameTimer, &TimerCancelled);
    pPort->P2PCfg.LostRspRetryCounter = 0;

    // Cover lost ACK task
    if ((pPort->P2PCfg.CurrState == P2P_WAIT_ACK_STATE) && (pPort->P2PCfg.SentFrame != pPort->P2PCfg.PrevSentFrame))
    {
        // Indicate Req(ok) to OS
#ifdef DBG              
        P2pMsIndicateFrameSendComplete(pAd, pPort, 
                                    (PUCHAR) pPort->P2PCfg.pRequesterBufferedFrame, pPort->P2PCfg.RequesterBufferedSize, NDIS_STATUS_SUCCESS, __LINE__);
#else
        P2pMsIndicateFrameSendComplete(pAd, pPort, 
                                    (PUCHAR) pPort->P2PCfg.pRequesterBufferedFrame, pPort->P2PCfg.RequesterBufferedSize, NDIS_STATUS_SUCCESS);
#endif
        // Assign sent frame
        pPort->P2PCfg.PrevSentFrame = pPort->P2PCfg.SentFrame;

        // Free Old Buffered Data
        P2pMsFreeIeMemory(&pPort->P2PCfg.pRequesterBufferedFrame, (PLONG)&pPort->P2PCfg.RequesterBufferedSize);     
    }

    // Indicate Rsp/Confrim receive to OS
    switch (pPort->P2PCfg.Signal)
    {
        case GO_NEGOCIATION_RSP:
            P2pMsIndicateGONegoRspRecv(pAd, pAd->PortList[pPort->P2PCfg.PortNumber], pInBuffer, InBufferLen);
            break;
        case P2P_PROVISION_RSP:
            P2pMsIndicateProvDiscoRspRecv(pAd, pAd->PortList[pPort->P2PCfg.PortNumber], pInBuffer, InBufferLen);
            break;
        case P2P_INVITE_RSP:
            P2pMsIndicateInviteRspRecv(pAd, pAd->PortList[pPort->P2PCfg.PortNumber], pInBuffer, InBufferLen);
            break;
        case GO_NEGOCIATION_CONFIRM:
            P2pMsIndicateGONegoConfirmRecv(pAd, pAd->PortList[pPort->P2PCfg.PortNumber], pInBuffer, InBufferLen);
            break;
        default:
            DBGPRINT(RT_DEBUG_TRACE, ("%s - No match Signal case(%s)\n", __FUNCTION__, MlmeP2pMsSignalDecode(pPort->P2PCfg.Signal)));
            break;
    }

    if ((pPort->P2PCfg.Signal == GO_NEGOCIATION_RSP) ||
        (pPort->P2PCfg.Signal == P2P_PROVISION_RSP) ||
        (pPort->P2PCfg.Signal == P2P_INVITE_RSP))
    {
        pPort->P2PCfg.PrevState = pPort->P2PCfg.CurrState;      
        pPort->P2PCfg.CurrState = P2P_NULL_STATE;
    }

    DBGPRINT(RT_DEBUG_TRACE, ("%s - Set PrevState(%s), CurrState(%s)\n", 
                                __FUNCTION__, 
                                MlmeP2pMsStateDecode(pPort->P2PCfg.PrevState), 
                                MlmeP2pMsStateDecode(pPort->P2PCfg.CurrState)));

    DBGPRINT(RT_DEBUG_TRACE, ("%s <===\n", __FUNCTION__));
}

VOID MlmeP2pMsPublicActionFrameSend(
    IN PMP_ADAPTER    pAd,
    IN PMP_PORT         pPort,
    IN UCHAR            Signal)
{
    P2P_PUBLIC_FRAME    Frame;
    //PMP_PORT    pPort = pAd->PortList[FXXK_PORT_0];
    
    DBGPRINT(RT_DEBUG_TRACE, ("%s ===>\n", __FUNCTION__));

    Frame.Subtype = Signal;
    
    MTEnqueueInternalCmd(pAd, pPort,  MT_CMD_P2pMs_PUBLIC_ACTION_FRAME_SEND_EXEC, &Frame, sizeof(P2P_PUBLIC_FRAME));

    DBGPRINT(RT_DEBUG_TRACE, ("%s <===\n", __FUNCTION__));
}

VOID MlmeP2pMsPublicActionFrameGotAck(
    IN PMP_ADAPTER    pAd)
{
    P2P_PUBLIC_FRAME    Frame;
    PMP_PORT    pPort = pAd->PortList[FXXK_PORT_0];
    
    DBGPRINT(RT_DEBUG_TRACE, ("%s ===>\n", __FUNCTION__));
    
    Frame.Subtype = P2P_GOT_ACK;
    
    MTEnqueueInternalCmd(pAd, pPort,  MT_CMD_P2pMs_PUBLIC_ACTION_FRAME_GOT_ACK_EXEC, &Frame, sizeof(P2P_PUBLIC_FRAME));
    
    DBGPRINT(RT_DEBUG_TRACE, ("%s <===\n", __FUNCTION__));
}

VOID MlmeP2pMsPublicActionFrameStateMachine(
    IN PMP_ADAPTER pAd, 
    IN MLME_QUEUE_ELEM *Elem) 
{
    PMP_PORT    pPort = pAd->PortList[Elem->PortNum];
    MTEnqueueInternalCmd(pAd, pPort,  MT_CMD_P2pMs_PUBLIC_ACTION_FRAME_EXEC, &Elem->Msg[0], Elem->MsgLen);
}

VOID MlmeP2pMsPublicActionFrameStateMachineExec(
    IN PMP_ADAPTER pAd,
    IN PMP_PORT      pPort,
    IN  PUCHAR  pInBuffer,
    IN  ULONG   InBufferLen)
{
    PP2P_PUBLIC_FRAME   pFrame = NULL;

    if (pInBuffer == NULL)
    {
        DBGPRINT(RT_DEBUG_TRACE, ("%s, RETURN\n", __FUNCTION__));
        return;
    }

    pFrame = (PP2P_PUBLIC_FRAME)pInBuffer;
    pPort->P2PCfg.Signal = pFrame->Subtype;
    
    DBGPRINT(RT_DEBUG_TRACE, ("%s ===> CurrState(%s), Signal(%d:%s)\n", 
                                __FUNCTION__, 
                                MlmeP2pMsStateDecode(pPort->P2PCfg.CurrState), 
                                pPort->P2PCfg.Signal,
                                MlmeP2pMsSignalDecode(pPort->P2PCfg.Signal)));
    
    switch (pPort->P2PCfg.CurrState)
    {
        // 0. Null state
        case P2P_NULL_STATE:
            case P2P_WAIT_ACK_STATE:
            MlmeP2pMsNullState(pAd, pPort, pInBuffer, InBufferLen);
            break;
        // 1. Send out Req/Cfm/Rsp frame
        case P2P_SEND_STATE:
            MlmeP2pMsSendState(pAd, pPort, pInBuffer, InBufferLen);
            break;
        // 2. Wait for ACK
        //case P2P_WAIT_ACK_STATE:
            //MlmeP2pMsWaitAckState(pAd, pInBuffer, InBufferLen);
            //break;
        // 3. Wait for Rsp
        //case P2P_WAIT_RSP_STATE:
            //MlmeP2pMsWiatRspState(pAd, pInBuffer, InBufferLen);
            //break;
        default:
            DBGPRINT(RT_DEBUG_TRACE, ("%s - No match CurrState case(%s)\n", __FUNCTION__, MlmeP2pMsStateDecode(pPort->P2PCfg.CurrState)));
            break;
    }   
    DBGPRINT(RT_DEBUG_TRACE, ("%s <===\n", __FUNCTION__));      
}

VOID P2pMsPublicActionFrameWaitTimerTimeout(
    IN PVOID SystemSpecific1, 
    IN PVOID FunctionContext, 
    IN PVOID SystemSpecific2, 
    IN PVOID SystemSpecific3)
{
    PMP_PORT pPort = (PMP_PORT )FunctionContext;
    PMP_ADAPTER  pAd = pPort->pAd;
    MTEnqueueInternalCmd(pAd, pPort,  MT_CMD_P2pMs_PUBLIC_ACTION_FRAME_TIMEOUT_EXEC, NULL, 0);
}

VOID P2pMsTriggerReadTXFIFOTimeout(
    IN PVOID SystemSpecific1, 
    IN PVOID FunctionContext, 
    IN PVOID SystemSpecific2, 
    IN PVOID SystemSpecific3)
{
    PMP_PORT pPort = (PMP_PORT )FunctionContext;
    PMP_ADAPTER  pAd = pPort->pAd;
    DBGPRINT(RT_DEBUG_TRACE, ("%s ====> \n", __FUNCTION__));
    MTEnqueueInternalCmd(pAd, pPort,  MT_CMD_P2P_READ_TXFIFO, NULL, 0);
}

VOID P2pMsPublicActionFrameWaitTimerTimeoutCallback(
    IN PVOID SystemSpecific1, 
    IN PVOID FunctionContext, 
    IN PVOID SystemSpecific2, 
    IN PVOID SystemSpecific3)
{
    PMP_PORT pPort = (PMP_PORT )FunctionContext;
    PMP_ADAPTER  pAd = pPort->pAd;
    
    if (MT_TEST_FLAG(pAd, fRTMP_ADAPTER_HALT_IN_PROGRESS) ||(MT_TEST_FLAG(pAd, fRTMP_ADAPTER_SURPRISE_REMOVED)))
      return;

    MTEnqueueInternalCmd(pAd, pPort,  MT_CMD_P2pMs_PUBLIC_ACTION_FRAME_TIMEOUT_EXEC, NULL, 0);
}

VOID P2pMsTriggerReadTXFIFOTimeoutCallback(
    IN PVOID SystemSpecific1, 
    IN PVOID FunctionContext, 
    IN PVOID SystemSpecific2, 
    IN PVOID SystemSpecific3
    )
{
    PMP_PORT pPort = (PMP_PORT )FunctionContext;
    PMP_ADAPTER  pAd = pPort->pAd;
    
    DBGPRINT(RT_DEBUG_TRACE, ("%s ====> \n", __FUNCTION__));
    if (MT_TEST_FLAG(pAd, fRTMP_ADAPTER_HALT_IN_PROGRESS) ||(MT_TEST_FLAG(pAd, fRTMP_ADAPTER_SURPRISE_REMOVED)))
      return;

    MTEnqueueInternalCmd(pAd, pPort,  MT_CMD_P2P_READ_TXFIFO, NULL, 0);
}

VOID P2pMsPublicActionFrameWaitTimerTimeoutExec(
    IN PMP_PORT pPort)
{
    P2P_PUBLIC_FRAME    Frame;
    PMP_ADAPTER pAd = pPort->pAd;
    
    switch (pPort->P2PCfg.CurrState)
    {
        case P2P_WAIT_ACK_STATE:
            // 1. retry
            if (pPort->P2PCfg.LostAckRetryCounter < WFD_PUBLIC_ACTION_FRAME_RETRY_COUNT_MAX)
            {
                DBGPRINT(RT_DEBUG_TRACE, ("%s - P2P_WAIT_ACK_STATE --> Ack Timeout --> LostAckRetryCounter(%d) last frame\n", __FUNCTION__, pPort->P2PCfg.LostAckRetryCounter));
                pPort->P2PCfg.PrevState = pPort->P2PCfg.CurrState;
                pPort->P2PCfg.CurrState = P2P_SEND_STATE;
                pPort->P2PCfg.Signal = pPort->P2PCfg.SentFrame;
                pPort->P2PCfg.LostAckRetryCounter++;
                Frame.Subtype = pPort->P2PCfg.Signal;
                //MlmeEnqueue(pAd, pAd->PortList[pPort->P2PCfg.PortNumber], ACTION_STATE_MACHINE, MT2_MLME_ACTION_CATE, sizeof(P2P_PUBLIC_FRAME), &Frame);
                //MlmeHandler(pAd); 
                MlmeP2pMsPublicActionFrameStateMachineExec(pAd, pPort, (PUCHAR)&Frame, sizeof(P2P_PUBLIC_FRAME));
            }
            // 2. Indicate error to OS
            else
            {
                DBGPRINT(RT_DEBUG_TRACE, ("%s - P2P_WAIT_ACK_STATE Failed to retry(%d) --> Go to P2P_NULL_STATE indicate fail\n", __FUNCTION__, WFD_PUBLIC_ACTION_FRAME_RETRY_COUNT_MAX));      
                pPort->P2PCfg.PrevState = P2P_NULL_STATE;
                pPort->P2PCfg.CurrState = P2P_NULL_STATE;
                pPort->P2PCfg.Signal = P2P_NULL_STATE;
                pPort->P2PCfg.LostAckRetryCounter = 0;
#ifdef DBG              
                P2pMsIndicateFrameSendComplete(pAd, pPort, 
                                            (PUCHAR) pPort->P2PCfg.pRequesterBufferedFrame, pPort->P2PCfg.RequesterBufferedSize, NDIS_STATUS_SUCCESS, __LINE__);
#else
                P2pMsIndicateFrameSendComplete(pAd, pPort, 
                                            (PUCHAR) pPort->P2PCfg.pRequesterBufferedFrame, pPort->P2PCfg.RequesterBufferedSize, NDIS_STATUS_SUCCESS);
#endif
            }
            break;
        case P2P_WAIT_RSP_STATE:
            // 1. retry
            if (pPort->P2PCfg.LostRspRetryCounter < WFD_PUBLIC_ACTION_FRAME_RETRY_COUNT_MAX)
            {
                DBGPRINT(RT_DEBUG_TRACE, ("%s - P2P_WAIT_RSP_STATE --> Rsp Timeout --> LostRspRetryCounter(%d) last frame\n", __FUNCTION__, pPort->P2PCfg.LostRspRetryCounter));
                pPort->P2PCfg.PrevState = pPort->P2PCfg.CurrState;
                pPort->P2PCfg.CurrState = P2P_SEND_STATE;
                pPort->P2PCfg.Signal = pPort->P2PCfg.SentFrame;
                pPort->P2PCfg.LostRspRetryCounter++;
                pPort->P2PCfg.LostAckRetryCounter = 0;
                Frame.Subtype = pPort->P2PCfg.Signal;
                //MlmeEnqueue(pAd, pAd->PortList[pPort->P2PCfg.PortNumber], ACTION_STATE_MACHINE, MT2_MLME_ACTION_CATE, sizeof(P2P_PUBLIC_FRAME), &Frame);
                //MlmeHandler(pAd); 
                MlmeP2pMsPublicActionFrameStateMachineExec(pAd, pPort, (PUCHAR)&Frame, sizeof(P2P_PUBLIC_FRAME));
            }
            // 2. Indicate error to OS
            else
            {
                DBGPRINT(RT_DEBUG_TRACE, ("%s - P2P_WAIT_RSP_STATE Failed to retry(%d) --> Go to P2P_NULL_STATE\n", __FUNCTION__, WFD_PUBLIC_ACTION_FRAME_RETRY_COUNT_MAX));        
                pPort->P2PCfg.PrevState = P2P_NULL_STATE;
                pPort->P2PCfg.CurrState = P2P_NULL_STATE;
                pPort->P2PCfg.Signal = P2P_NULL_STATE;
                pPort->P2PCfg.LostRspRetryCounter = 0;
                pPort->P2PCfg.LostAckRetryCounter = 0;
            }
            break;
        default:
            DBGPRINT(RT_DEBUG_TRACE, ("%s - No match state\n", __FUNCTION__));
            break;
    }
}

// When connect STA(40M) firsrtly then conenct CLI, CLI will trigger scan then restore to 40M channel, 
// we need this swicth function to correctly swicth to 20M channel for p2p public action frame exchange
VOID MlmeP2pMsSwitchCh(
    IN PMP_ADAPTER pAd,
    IN UCHAR Channel)
{
    //UCHAR     Value;
    //ULONG     Data;
    UCHAR       BBPValue = 0;

    RTUSBReadBBPRegister(pAd, BBP_R4, &BBPValue);
    BBPValue &= (~0x18);
    RTUSBWriteBBPRegister(pAd, BBP_R4, BBPValue);

    //AsicSwitchChannel(pAd, Channel, FALSE);
    
    DBGPRINT(RT_DEBUG_TRACE, ("SYNC - BBP R4 to 20MHz. End\n"));    
}

 LPSTR MlmeP2pMsStateDecode (UCHAR P2pState)
{
    LPSTR retval = "                                                                       ";
    switch (P2pState)
    {
        case P2P_NULL_STATE:                retval = "P2P_NULL_STATE";              break;
        case P2P_SEND_STATE:            retval = "P2P_SEND_STATE";          break;
        case P2P_WAIT_ACK_STATE:        retval = "P2P_WAIT_ACK_STATE";      break;
        case P2P_WAIT_RSP_STATE:        retval = "P2P_WAIT_RSP_STATE";      break;
        default:                            retval = "P2P_STATE_UNKNOWN";       break;
        }
    return(retval);
}

 LPSTR MlmeP2pMsSignalDecode (UCHAR P2pSignal)
{
    LPSTR retval = "                                                                       ";
    switch (P2pSignal)
    {
        case GO_NEGOCIATION_REQ:        retval = "GO_NEGOCIATION_REQ";      break;
        case GO_NEGOCIATION_RSP:        retval = "GO_NEGOCIATION_RSP";      break;
        case GO_NEGOCIATION_CONFIRM:    retval = "GO_NEGOCIATION_CONFIRM";  break;
        case P2P_INVITE_REQ:                retval = "P2P_INVITE_REQ";              break;
        case P2P_INVITE_RSP:                retval = "P2P_INVITE_RSP";              break;
        case P2P_PROVISION_REQ:         retval = "P2P_PROVISION_REQ";           break;
        case P2P_PROVISION_RSP:         retval = "P2P_PROVISION_RSP";           break;
        case P2P_GOT_ACK:               retval = "P2P_GOT_ACK";             break;
        default:                            retval = "P2P_SIGNAL_UNKNOWN";      break;
        }
    return(retval);
} 
#endif /*WFD_NEW_PUBLIC_ACTION*/
#endif
VOID PeerAddBAReqAction(
    IN PMP_ADAPTER pAd, 
    IN MLME_QUEUE_ELEM *Elem) 
{
    //  7.4.4.1
    ULONG   Idx;
    UCHAR   Status = 1;
    UCHAR   pAddr[6];
    FRAME_ADDBA_RSP ADDframe;
    PUCHAR         pOutBuffer = NULL;
    NDIS_STATUS     NStatus;
    PFRAME_ADDBA_REQ  pAddreqFrame = NULL;
    UCHAR               BufSize = 0;
    ULONG       FrameLen;
    USHORT      StartSeq;
    PFRAME_802_11       pFrame; 
    UCHAR               Bssid[MAC_ADDR_LEN];
    MP_RW_LOCK_STATE PsQLockState;
    PMP_PORT  pPort = pAd->PortList[Elem->PortNum];

    PMAC_TABLE_ENTRY pWcidMacTabEntry = NULL;
    pWcidMacTabEntry = MlmeSyncMacTabMatchedWlanIdx(pPort, Elem->Wcid);
    if(pWcidMacTabEntry == NULL)
    {
        DBGPRINT(RT_DEBUG_TRACE,("%s(), Line:%d, Mac Entry is NULL  Wcid = %d\n", __FUNCTION__, __LINE__, Elem->Wcid));
        return;
    }
    
    if (pPort == NULL)
    {
        DBGPRINT(RT_DEBUG_ERROR, ("%s - pPort is  NULL, Error, and return this function", __FUNCTION__));
        return;
    }

    //ADDBA Request from unknown peer, ignore this.
    if (Elem->Wcid >= MAX_LEN_OF_MAC_TABLE)
    {
        DBGPRINT(RT_DEBUG_ERROR, ("%s - Elem->Wcid = %d (%d)", __FUNCTION__, Elem->Wcid, MAX_LEN_OF_MAC_TABLE));
        return;
    }

    if ((pPort->CommonCfg.BACapability.field.Policy == BA_NOTUSE) || (pPort->CommonCfg.IOTestParm.bNextDisableRxBA == TRUE))
    {
        Status = REASON_DECLINED;
        pFrame = (PFRAME_802_11)Elem->Msg;
        COPY_MAC_ADDR(pAddr, pFrame->Hdr.Addr2);
        DBGPRINT(RT_DEBUG_TRACE,("ACTION - %s Status = %d\n", __FUNCTION__, Status));
    }
    //
    // Fix IOT, Buffalo AP Mode:WZR-AMPG300NH (Marvell Solution) 
    // If we decline ADDBA request, AP will block the data.
    // So we allow ADDBA request from AP even if it enables TKIP.
    //
#if 0       
    else if (TKIP_ON(pPort) 
        && (!CLIENT_STATUS_TEST_FLAG(&pPort->MacTab.Content[Elem->Wcid], fCLIENT_STATUS_RALINK_CHIPSET)))
    {
        Status = REASON_DECLINED;
        pFrame = (PFRAME_802_11)Elem->Msg;
        COPY_MAC_ADDR(pAddr, pFrame->Hdr.Addr2);
        DBGPRINT(RT_DEBUG_TRACE,("ACTION - PeerBAAction() Status = %d,   \n", Status));
    }
#endif
    else if (PeerAddBAReqActionSanity(pAd, Elem->Msg, Elem->MsgLen, pAddr)) 
    {
        MAC_TABLE_ENTRY *pEntry = NULL;
        pAddreqFrame = (PFRAME_ADDBA_REQ)(&Elem->Msg[0]);

        // AdhocN Support, DLS/TDLS Support
        // We don't know if the peer STA is dear or not. If one associated BA peer try to create BA. We will delete BA session first.
        if (((ADHOC_ON(pPort) && pAd->StaCfg.bAdhocNMode) ||(INFRA_ON(pPort) &&(Elem->Wcid != RESERVED_WCID))) && 
            !((pWcidMacTabEntry->RXBAbitmap &(1<<pAddreqFrame->BaParm.TID)) == 0))
        {
            if ((pWcidMacTabEntry->ValidAsCLI == TRUE))
            {
                UCHAR           TIDStart, TIDEnd, CurTID;
                BOOLEAN         bCanceld;
                BA_REC_ENTRY    *pBARECEntry;   
                
                DBGPRINT(RT_DEBUG_TRACE,("ACTION - PeerAddBAReqAction(): Delete previous peer's BA session\n"));

                pEntry = pWcidMacTabEntry;

                // ORIEntry: Send DelBA and delete parameter about it.
                BATableTearORIEntry(pAd,pPort, 0, (UCHAR)pEntry->Aid, FALSE, TRUE);

                // RECEntry: not Send DelBA, but delete parameter about it.
                TIDStart    = 0;
                TIDEnd  = Max_BARECI_SESSION;
                for (CurTID = TIDEnd; CurTID < TIDEnd; CurTID++)
                {
                    pBARECEntry = NULL;
                    Idx = pEntry->BARecWcidArray[CurTID];
                    pBARECEntry = &pAd->BATable.BARecEntry[Idx];

                    if ((pBARECEntry!=NULL) && (pBARECEntry->TID == CurTID))
                        BATableDeleteRECEntry(pAd, pPort,pBARECEntry);
                }
                PlatformAcquireOldRWLockForWrite(&pAd->pTxCfg->PsQueueLock, &PsQLockState);
                APCleanupPsQueue(pAd, &pEntry->PsQueue); // return all NDIS packet in PSQ
                PlatformReleaseOldRWLock(&pAd->pTxCfg->PsQueueLock, &PsQLockState);
                PlatformCancelTimer(&pEntry->RetryTimer, &bCanceld);
            }
        }

        // 1. 
        Idx = pWcidMacTabEntry->BARecWcidArray[pAddreqFrame->BaParm.TID];
        if ((pWcidMacTabEntry->RXBAbitmap &(1<<pAddreqFrame->BaParm.TID)) == 0)
        {
            //Newly add
            BufSize = (pAddreqFrame->BaParm.BufSize > pPort->CommonCfg.BACapability.field.RxBAWinLimit)? (UCHAR)pPort->CommonCfg.BACapability.field.RxBAWinLimit:pAddreqFrame->BaParm.BufSize;

            // Intel Station send ADDBA with bufsize=0, we need to return bufsize=64.
            if ((BufSize == 0) && (pAddreqFrame->TimeOutValue == 5000) && (pAddreqFrame->BaParm.BAPolicy == IMMED_BA))
            {           
                DBGPRINT(RT_DEBUG_TRACE,("STA is Intel 4965 wcid=%d\n", Elem->Wcid));
                pWcidMacTabEntry->IdentifyIntelSta = IOT_STRUC_INTEL_4965;                
                pPort->CommonCfg.IOTestParm.bIntel4965 = TRUE;
            }
            // Intel Station send ADDBA with bufsize=0, we need to return bufsize=64.           
            if (BufSize < 2)
                BufSize = 64;

            BufSize = 64;
            StartSeq = (pAddreqFrame->BaStartSeq.field.StartSeq == 0)?0xfff:(pAddreqFrame->BaStartSeq.field.StartSeq - 1);
            DBGPRINT(RT_DEBUG_TRACE,("Peer Add BA req StartSeq = %d\n", StartSeq));
            BATableInsertEntry(pAd, pPort,Elem->Wcid,  pAddreqFrame->TimeOutValue, StartSeq, (UCHAR)pAddreqFrame->BaParm.TID, BufSize, 0, TRUE);
        }
        else
        {
            // BA  agreement already setup. So return with existing NMSDU , Status Code = SUCCESS
            // Check !!!  change buffer size? rt2860
            BufSize = pPort->BATable.BARecEntry[Idx].BAWinSize;

            BufSize = 64;
        }
        Status = 0;
        DBGPRINT(RT_DEBUG_TRACE,("ACTION - PeerBAAction() BufSize = %d \n", BufSize));
    }
    
    pAddreqFrame = (PFRAME_ADDBA_REQ)(&Elem->Msg[0]);
    
    // 2. Always send back ADDBA Response 
    NStatus = MlmeAllocateMemory(pAd, &pOutBuffer);  //Get an unused nonpaged memory
    if (NStatus != NDIS_STATUS_SUCCESS) 
    {
        DBGPRINT(RT_DEBUG_ERROR,("ACTION - PeerBAAction() allocate memory failed \n"));
        return;
    }
    
    COPY_MAC_ADDR(Bssid, pPort->PortCfg.Bssid);

    // 2-1. Prepare ADDBA Response frame.
    ActHeaderInit(pAd, pPort, &ADDframe.Hdr, pAddr, Bssid);
    ADDframe.Category = CATEGORY_BA;
    ADDframe.Action = ADDBA_RESP;
    ADDframe.Token = pAddreqFrame->Token;
    // What is the Status code??  need to check.
    ADDframe.StatusCode = Status;
    ADDframe.BaParm.AMSDU = 0;
    ADDframe.BaParm.BAPolicy = IMMED_BA;
    ADDframe.BaParm.TID = pAddreqFrame->BaParm.TID;
    ADDframe.BaParm.BufSize = BufSize;
    if (pWcidMacTabEntry->IdentifyIntelSta == IOT_STRUC_INTEL_4965)
        ADDframe.TimeOutValue = 0;
    else        
        ADDframe.TimeOutValue = pAddreqFrame->TimeOutValue;
    
    MakeOutgoingFrame(pOutBuffer,               &FrameLen,
              sizeof(FRAME_ADDBA_RSP),  &ADDframe,
              END_OF_ARGS);

    DBGPRINT(RT_DEBUG_TRACE,("ACTION - Send  BA response\n"));
    NdisCommonMiniportMMRequest(pAd, pOutBuffer, FrameLen);
    // 
    if ((pPort->CommonCfg.BACapability.field.AutoBA) && (pWcidMacTabEntry->TXAutoBAbitmap == 0))
    {
        BATableInsertEntry(pAd, pPort,pWcidMacTabEntry->Aid,  0, 0x1, 0, pWcidMacTabEntry->BaSizeInUse,  Originator_SetAutoAdd, FALSE);
    }

    ORIBATimerTimeout(pAd, pPort);
}

VOID PeerAddBARspAction(
    IN PMP_ADAPTER pAd, 
    IN MLME_QUEUE_ELEM *Elem) 
{
    UCHAR       Idx;
    PFRAME_ADDBA_RSP    pFrame = NULL;
    PBA_ORI_ENTRY       pBss;
    PUCHAR         pOutBuffer1 = NULL;
    PUCHAR         pOutBuffer2 = NULL;
    NDIS_STATUS     NStatus;
    ULONG       FrameLen;
    FRAME_BAR   FrameBar;
    PMP_PORT  pPort = pAd->PortList[Elem->PortNum];
    
    PMAC_TABLE_ENTRY pWcidMacTabEntry = NULL;
    pWcidMacTabEntry = MlmeSyncMacTabMatchedWlanIdx(pPort, Elem->Wcid);   
    if(pWcidMacTabEntry == NULL)
    {
        DBGPRINT(RT_DEBUG_TRACE,("%s(), Line:%d, Mac Entry is NULL  Wcid = %d\n", __FUNCTION__, __LINE__, Elem->Wcid));
        return;
    }    
    //ADDBA Response from unknown peer, ignore this.
    if (Elem->Wcid >= MAX_LEN_OF_MAC_TABLE)
        return;
    
    if (PeerAddBARspActionSanity(pAd, Elem->Msg, Elem->MsgLen))
    {
        pFrame = (PFRAME_ADDBA_RSP)(&Elem->Msg[0]);
        
        Idx = pWcidMacTabEntry->BAOriWcidArray[pFrame->BaParm.TID];
        // return on dismatched TID on ADDBA response 
        if ((pPort->BATable.BAOriEntry[Idx].TID != pFrame->BaParm.TID))
            return;
        
        DBGPRINT(RT_DEBUG_TRACE,("PeerAddBARsp---->StatusCode = %d, Wcid = %d, BAid = %d, TID=%d\n", pFrame->StatusCode, Elem->Wcid, Idx, pFrame->BaParm.TID));
        switch (pFrame->StatusCode)
        {
            case 0:
                // I am the originatorwant of this BA session.
                if ((pPort->BATable.BAOriEntry[Idx].ORI_BA_Status != Originator_Done))
                {
                    NdisAcquireSpinLock(&pPort->BATabLock);
                    // Change parameter.
                    pPort->BATable.BAOriEntry[Idx].BAWinSize = (UCHAR)pFrame->BaParm.BufSize;
                    // update parameter from receiver. Ultimate parameter is based on ADDBA responsed frame.
                    pBss = &pPort->BATable.BAOriEntry[Idx];
                    pBss->ORI_BA_Status = Originator_Done; 
                    pBss->TID = (UCHAR)pFrame->BaParm.TID;
                    pPort->BATable.numAsOriginator++;
                    pBss->BAWinSize = (UCHAR)pFrame->BaParm.BufSize;  //NMSDU will not surpass 8 bits
                    if (pBss->BAWinSize == 0)
                        pBss->BAWinSize = 1;  //NMSDU will not surpass 8 bits
                        
                    pBss->TimeOutValue= pFrame->TimeOutValue;
                    pWcidMacTabEntry->TXBAbitmap |= (1<<(pFrame->BaParm.TID));
                    pWcidMacTabEntry->TXAutoBAbitmap &= (~(1<<(pFrame->BaParm.TID)));
                    NdisReleaseSpinLock(&pPort->BATabLock);
                    // SEND BAR ;
                    NStatus = MlmeAllocateMemory(pAd, &pOutBuffer1);  //Get an unused nonpaged memory
                    if(NStatus != NDIS_STATUS_SUCCESS) 
                    {
                        DBGPRINT(RT_DEBUG_TRACE,("BA - MlmeDELBAAction() allocate memory failed \n"));
                        return;
                    }
                    NStatus = MlmeAllocateMemory(pAd, &pOutBuffer2);  //Get an unused nonpaged memory
                    if(NStatus != NDIS_STATUS_SUCCESS) 
                    {
                        DBGPRINT(RT_DEBUG_TRACE,("BA - MlmeDELBAAction() allocate memory failed \n"));
                        return;
                    }

                    BarHeaderInit(pAd,pPort,&FrameBar, pWcidMacTabEntry->Addr);
                    FrameBar.StartingSeq.field.FragNum = 0; // make sure sequence not clear in DEL function.
                    FrameBar.StartingSeq.field.StartSeq = pWcidMacTabEntry->BAOriSequence[pBss->TID]; // make sure sequence not clear in DEL funciton.
                    FrameBar.BarControl.TID = pBss->TID; // make sure sequence not clear in DEL funciton.
                    FrameBar.BarControl.ACKPolicy = 0; 
                    FrameBar.BarControl.Compressed = 1; // make sure sequence not clear in DEL funciton.
                    FrameBar.BarControl.MTID = 0; // make sure sequence not clear in DEL funciton.
                    DBGPRINT(RT_DEBUG_TRACE,("PeerAddBARsp ----->Wcid = %d, TID = %d,  BAOriSequence = %x, FrameBar.StartingSeq.field.StartSeq = %d\n",Elem->Wcid, pBss->TID, pWcidMacTabEntry->BAOriSequence[pBss->TID], FrameBar.StartingSeq.field.StartSeq));
                    MakeOutgoingFrame(pOutBuffer1,              &FrameLen,
                                  sizeof(FRAME_BAR),      &FrameBar,
                                  END_OF_ARGS);
                    NdisCommonMiniportMMRequest(pAd, pOutBuffer1, FrameLen);
                    MakeOutgoingFrame(pOutBuffer2,              &FrameLen,
                                  sizeof(FRAME_BAR),      &FrameBar,
                                  END_OF_ARGS);
                    NdisCommonMiniportMMRequest(pAd, pOutBuffer2, FrameLen);

                    MtAsicUpdateBASession(pAd, Elem->Wcid, pBss->TID, pWcidMacTabEntry->BAOriSequence[pBss->TID], pBss->BAWinSize, 1, ORIGINATOR);
                }

                break;
            case REASON_DECLINED:
                if (pPort->BATable.BAOriEntry[Idx].ORI_BA_Status == Originator_WaitRes)
                {
                    BATableTearORIEntry(pAd,pPort, (UCHAR)pFrame->BaParm.TID, Elem->Wcid, TRUE, FALSE);
                }
                break;
            default:
                if (pPort->BATable.BAOriEntry[Idx].ORI_BA_Status == Originator_WaitRes)
                {
                    // After retry 10 times, delete.
                    if (++pPort->BATable.BAOriEntry[Idx].Token > 20)
                    {
                        BATableTearORIEntry(pAd,pPort, (UCHAR)pFrame->BaParm.TID, Elem->Wcid, FALSE, FALSE);
                    }
                    else
                    {
                        // Decrease Buf zise and retry again.
                        MLME_ADDBA_REQ_STRUCT   AddbaReq;
                        if (pPort->BATable.BAOriEntry[Idx].BAWinSize > 3)
                            pPort->BATable.BAOriEntry[Idx].BAWinSize =(pPort->BATable.BAOriEntry[Idx].BAWinSize)*7/8;
                        
                        PlatformZeroMemory(&AddbaReq, sizeof(AddbaReq));
                        COPY_MAC_ADDR(AddbaReq.pAddr, pWcidMacTabEntry->Addr);
                        AddbaReq.Wcid = (UCHAR)(Elem->Wcid);
                        AddbaReq.TID = (UCHAR)pFrame->BaParm.TID;
                        AddbaReq.Token = pPort->BATable.BAOriEntry[Idx].Token;
                        AddbaReq.BaBufSize = pPort->BATable.BAOriEntry[Idx].BAWinSize;
                        AddbaReq.TimeOutValue = pPort->BATable.BAOriEntry[Idx].TimeOutValue;
                        MlmeEnqueue(pAd,pPort, ACTION_STATE_MACHINE, MT2_MLME_ADD_BA_CATE, sizeof(MLME_ADDBA_REQ_STRUCT), (PVOID)&AddbaReq);
                    }
                        
                }
                break;

        }
        DBGPRINT(RT_DEBUG_TRACE,("PeerAddBARspAction - ----> numAsOriginator = %d, \n",pPort->BATable.numAsOriginator));
    }
}

VOID PeerDelBAAction(
    IN PMP_ADAPTER pAd, 
    IN MLME_QUEUE_ELEM *Elem) 
{
    UCHAR       Idx;
    PFRAME_DELBA_REQ  pDelFrame = NULL;
    PMP_PORT pPort = pAd->PortList[Elem->PortNum];
    
    PMAC_TABLE_ENTRY pWcidMacTabEntry = NULL;
    pWcidMacTabEntry = MlmeSyncMacTabMatchedWlanIdx(pPort, Elem->Wcid);     
    if(pWcidMacTabEntry == NULL)
    {
        DBGPRINT(RT_DEBUG_TRACE,("%s(), Line:%d, Mac Entry is NULL  Wcid = %d\n", __FUNCTION__, __LINE__, Elem->Wcid));
        return;
    }
    
    //DELBA Request from unknown peer, ignore this.
    if (PeerDelBAActionSanity(pAd, Elem->Wcid, Elem->Msg, Elem->MsgLen))
    {
        pDelFrame = (PFRAME_DELBA_REQ)(&Elem->Msg[0]);
        if (pDelFrame->DelbaParm.Initiator == ORIGINATOR)
        {
            pPort->CommonCfg.DelBACount++;
            DBGPRINT(RT_DEBUG_TRACE,("BA - PeerDelBAAction----> ORIGINATOR\n"));
            Idx = pWcidMacTabEntry->BARecWcidArray[pDelFrame->DelbaParm.TID];
            if (pDelFrame->DelbaParm.TID == pPort->BATable.BARecEntry[Idx].TID)
            {
                // lock BATableDeleteRECEntry and QosBADataParse
                NdisAcquireSpinLock(&pPort->BADeleteRECEntry);
                BATableDeleteRECEntry(pAd, pPort,&(pPort->BATable.BARecEntry[Idx]));
                NdisReleaseSpinLock(&pPort->BADeleteRECEntry);
            }
        }
        else 
        {
            DBGPRINT(RT_DEBUG_TRACE,("BA - PeerDelBAAction----> RECIPIENT\n"));
            Idx = pWcidMacTabEntry->BAOriWcidArray[pDelFrame->DelbaParm.TID];
            if (pDelFrame->DelbaParm.TID == pPort->BATable.BAOriEntry[Idx].TID)
            BATableDeleteORIEntry(pAd,pPort, &(pPort->BATable.BAOriEntry[Idx]));
            if (pPort->CommonCfg.BACapability.field.AutoBA == TRUE)
            {
                if (INFRA_ON(pPort) && (MlmeSyncMacTabMatchedRoleType(pPort, ROLE_WLANIDX_BSSID)) && (pAd->StaActive.SupportedHtPhy.bHtEnable == TRUE))
                {
                    // If add myself as one Recipient of peer (pPort->PortCfg.Bssid), the sequence 0xffff indicates the BA
                    // session not started yet. 
                    BATableInsertEntry(pAd,pPort, Elem->Wcid,  0, 0x1, (UCHAR)pDelFrame->DelbaParm.TID, pWcidMacTabEntry->BaSizeInUse,  Originator_SetAutoAdd, FALSE);
                    pWcidMacTabEntry->TXAutoBAbitmap |= (1<<pDelFrame->DelbaParm.TID);
                }
                else if (INFRA_ON(pPort) && (IS_P2P_CON_CLI(pAd, pPort) || IS_P2P_MS_CLI(pAd, pPort) || IS_P2P_MS_CLI2(pAd, pPort)) && (IS_P2P_MS_CLI_WCID(pPort, Elem->Wcid)) && (pPort->P2PCfg.P2pPhyMode != P2P_PHYMODE_LEGACY_ONLY))
                {
                    BATableInsertEntry(pAd,pPort, Elem->Wcid,  0, 0x1, (UCHAR)pDelFrame->DelbaParm.TID, pWcidMacTabEntry->BaSizeInUse,  Originator_SetAutoAdd, FALSE);
                    pWcidMacTabEntry->TXAutoBAbitmap |= (1<<pDelFrame->DelbaParm.TID);
                }
                else if ((pWcidMacTabEntry->Sst == SST_ASSOC) && 
                        (READ_PHY_CFG_MODE(pAd, &pWcidMacTabEntry->MaxPhyCfg) >= MODE_HTMIX))
                {
                    BATableInsertEntry(pAd,pPort, Elem->Wcid,  0, 0x1, (UCHAR)pDelFrame->DelbaParm.TID, pWcidMacTabEntry->BaSizeInUse,  Originator_SetAutoAdd, FALSE);
                    pWcidMacTabEntry->TXAutoBAbitmap |= (1<<pDelFrame->DelbaParm.TID);
                }
            }
        } 
    }
}

VOID PeerSpectrumAction(
    IN PMP_ADAPTER pAd, 
    IN MLME_QUEUE_ELEM *Elem) 
{

    UCHAR   Action = Elem->Msg[LENGTH_802_11+1];    
    switch(Action)
    {
        case SPEC_MRQ:  //7.3.2.21
            break;
            
        case SPEC_MRP:
            break;
        case SPEC_CHANNEL_SWITCH:
        {
            SEC_CHA_OFFSET_IE   Secondary;
            CHA_SWITCH_ANNOUNCE_IE  ChannelSwitch;
            PlatformZeroMemory(&Secondary, sizeof (SEC_CHA_OFFSET_IE));
            // This Action frame includes 2 IEs.
            PlatformMoveMemory(&ChannelSwitch, &Elem->Msg[LENGTH_802_11+4], sizeof (CHA_SWITCH_ANNOUNCE_IE));
            if (Elem->MsgLen > 9)
            {
                PlatformMoveMemory(&Secondary, &Elem->Msg[LENGTH_802_11+9], sizeof (SEC_CHA_OFFSET_IE));
            }
            
            DBGPRINT(RT_DEBUG_TRACE,("Spectrum - Action = ChannelSwitchMode = %d, ChannelSwitchCount = %d, bHtEnable = %d\n",ChannelSwitch.SwitchMode,ChannelSwitch.SwitchCount,pAd->StaActive.SupportedHtPhy.bHtEnable));          
            // 802.11h only has Channel Switch Announcement IE. 802.11n D3.03 adds secondary channel offset element in the end.
            if ((Elem->Msg[LENGTH_802_11+2] == IE_CHANNEL_SWITCH_ANNOUNCEMENT) && (Elem->Msg[LENGTH_802_11+3] == 3) && ((ChannelSwitch.SwitchCount == 0)||(pAd->StaActive.SupportedHtPhy.bHtEnable == FALSE)))
            {
                if (Elem->Msg[LENGTH_802_11+7] == IE_NEW_EXT_CHA_OFFSET && (Elem->Msg[LENGTH_802_11+8] == 1))
                {
                    ChannelSwitchAction(pAd, Elem->Wcid, ChannelSwitch.NewChannel, Secondary.SecondartChannelOffset);
                }
                else
                    ChannelSwitchAction(pAd, Elem->Wcid, ChannelSwitch.NewChannel, 0);
            }
            break;
        }       
    }
// unknown category field value received.
// return to source with most significant bit set to 1 if received packet's MSB = 0
}

VOID PeerQOSAction(
    IN PMP_ADAPTER pAd, 
    IN MLME_QUEUE_ELEM *Elem) 
{
}

VOID PeerBAAction(
    IN PMP_ADAPTER pAd, 
    IN MLME_QUEUE_ELEM *Elem) 
{
    UCHAR   Action = Elem->Msg[LENGTH_802_11+1];
    
    switch(Action)
    {
        case ADDBA_REQ:
            PeerAddBAReqAction(pAd,Elem);
            DBGPRINT(RT_DEBUG_TRACE,("BA - PeerBAAction() Action = ADDBA_REQ \n"));
            break;
        case ADDBA_RESP:
            PeerAddBARspAction(pAd,Elem);
            DBGPRINT(RT_DEBUG_TRACE,("BA - PeerBAAction() Action = ADDBA_RESP \n"));
            break;
        case DELBA:
            PeerDelBAAction(pAd,Elem);
            DBGPRINT(RT_DEBUG_TRACE,("BA - PeerBAAction() Action = PeerDelBAAction( \n"));
            break;
    }
}

VOID PeerPublicAction(
    IN PMP_ADAPTER pAd, 
    IN MLME_QUEUE_ELEM *Elem) 
{
    UCHAR   Action = Elem->Msg[LENGTH_802_11+1];
    UCHAR       BssCoexist;
    EXT_CHA_SWITCH_IE   ExtChaSwitchIE;
    PMP_PORT  pPort = pAd->PortList[Elem->PortNum];
        
    // P2P allow unknown peer that send public with action value = VENDOR_PUBLIC_ACT.
    // so return criteriam use AND.
    if ((Elem->Wcid >= MAX_LEN_OF_MAC_TABLE) && (Action != P2P_ACTION) 
        && (Action != GAS_INITIAL_REQ) && (Action != GAS_INITIAL_RSP)
         && (Action != GAS_COMEBACK_REQ) && (Action != GAS_COMEBACK_RSP))
    {
        DBGPRINT(RT_DEBUG_TRACE, (" Port[%d] Ignore Public Action Frame - Action %x. \n", Elem->PortNum, Action));
        return;
    }

    switch(Action)
    {
        case COEXIST_2040_ACTION:
            // Format as in IEEE 7.4.7.2
            BssCoexist = Elem->Msg[LENGTH_802_11+4];
            if (INFRA_ON(pPort))
            {
                StaPeerPublicAction(pAd,pPort,BssCoexist);
            }
            else if (pAd->OpMode == OPMODE_AP)
            {
            }
            break;

        // Extened channel switch's content should be the same as Channel Switch.
        case EXTENTED_CHANNEL_SWITCH:   // 11y-7.3.2.53
            PlatformMoveMemory(&ExtChaSwitchIE, &Elem->Msg[LENGTH_802_11+2], 4);
            DBGPRINT(RT_DEBUG_TRACE,("Spectrum. New Channel = %d, NewRegClass = %d \n", ExtChaSwitchIE.NewChannel, ExtChaSwitchIE.NewRegClass));
            break;

        case GAS_INITIAL_REQ:
            PeerGASIntialReqAction(pAd, Elem);
            break;

        case GAS_INITIAL_RSP:
            DBGPRINT(RT_DEBUG_TRACE,("Public.  Action  GAS_INITIAL_RSP\n" ));
            PeerGASIntialRspAction(pAd, Elem);
            break;
            
        case GAS_COMEBACK_REQ:
            DBGPRINT(RT_DEBUG_TRACE,("Public.  Action  GAS_COMEBACK_REQ\n" ));
            PeerGASComebackReqAction(pAd, Elem);
            break;
            
        case GAS_COMEBACK_RSP:
            DBGPRINT(RT_DEBUG_TRACE,("Public.  Action  GAS_COMEBACK_RSP\n" ));
            PeerGASComebackRspAction(pAd, Elem);
            break;

        case P2P_ACTION:
            DBGPRINT(RT_DEBUG_TRACE,("Port[%d]:: P2P Public Action. Vendor OUI+type = %x \n", Elem->PortNum, *(PULONG)&Elem->Msg[LENGTH_802_11+2] ));    
            if ((*(PULONG)&Elem->Msg[LENGTH_802_11+2] == P2P_OUI) && (P2P_ON(pPort)))
            {
#if(COMPILE_WIN8_ABOVE(CURRENT_OS_NDIS_VER))
#ifdef WFD_NEW_PUBLIC_ACTION
                if (IS_P2P_MS_DEV(pAd, pAd->PortList[Elem->PortNum]))
                {
                    MlmeP2pMsPublicActionFrameStateMachine(pAd, Elem);
                }
                else
#endif /*WFD_NEW_PUBLIC_ACTION*/
#endif
                {
                    P2PPublicAction(pAd, Elem);
                }
            }
            break;
    }

}

VOID StaPeerPublicAction(
    IN PMP_ADAPTER pAd, 
    IN PMP_PORT pPort,
    IN UCHAR Bss2040Coexist) 
{
    COEXIST_ELEMT_2040      BssCoexist;
    MLME_SCAN_REQ_STRUCT            ScanReq;

    DBGPRINT(RT_DEBUG_TRACE,("ACTION - StaPeerPublicAction  Bss2040Coexist = %x\n", Bss2040Coexist));
    BssCoexist.word = Bss2040Coexist;
    // AP asks Station to return a 20/40 BSS Coexistence mgmt frame.  So we first starts a scan, then send back 20/40 BSS Coexistence mgmt frame 
    // Temp remove 20/40 scan in BT mode
    if ((BssCoexist.field.InfoReq == 1) && (OPSTATUS_TEST_FLAG(pAd, fOP_STATUS_SCAN_2040)) && (OPSTATUS_TEST_FLAG(pPort, fOP_STATUS_SCAN_2040)))
    {
        // Clear record first.  After scan , will update those bit and send back to transmiter(AP).
        pPort->CommonCfg.BSSCoexist2040.field.InfoReq = 1;
        pPort->CommonCfg.BSSCoexist2040.field.Intolerant40 = 0;
        pPort->CommonCfg.BSSCoexist2040.field.BSS20WidthReq = 0;
        // Clear Trigger event table
        TriEventInit(pAd, pPort);
        // Fill out stuff for scan request and kick to scan
        MlmeCntScanParmFill(pAd, &ScanReq, ZeroSsid, 0, BSS_ANY, SCAN_2040_BSS_COEXIST);
        MlmeEnqueue(pAd, pPort,SYNC_STATE_MACHINE, MT2_MLME_SCAN_REQ, sizeof(MLME_SCAN_REQ_STRUCT), &ScanReq);
        pPort->Mlme.CntlMachine.CurrState = CNTL_WAIT_OID_LIST_SCAN;
    }
}

VOID PeerRMAction(
    IN PMP_ADAPTER pAd, 
    IN MLME_QUEUE_ELEM *Elem) 
{
}

VOID PeerHTAction(
    IN PMP_ADAPTER pAd, 
    IN MLME_QUEUE_ELEM *Elem) 
{
    UCHAR   Action = Elem->Msg[LENGTH_802_11+1];
    UCHAR   Information;
    MLME_DISASSOC_REQ_STRUCT DisReq = {0};
    MLME_QUEUE_ELEM MsgElem = {0};
    PMP_PORT pPort = pAd->PortList[Elem->PortNum];
    PMAC_TABLE_ENTRY pWcidMacTabEntry = NULL;
    pWcidMacTabEntry = MlmeSyncMacTabMatchedWlanIdx(pPort, Elem->Wcid);    
    if(pWcidMacTabEntry == NULL)
    {
        DBGPRINT(RT_DEBUG_TRACE,("%s(), Line:%d, Mac Entry is NULL  Wcid = %d\n", __FUNCTION__, __LINE__, Elem->Wcid));
        return;
    }
    
    if (Elem->Wcid >= MAX_LEN_OF_MAC_TABLE)
        return;
    
    switch(Action)
    {
        case Notify_BW_ACTION:
            if (pPort->PortType == EXTAP_PORT)
            {
                break;
            }
            if (pAd->StaActive.SupportedHtPhy.bHtEnable == FALSE)
            {
                // Note, this is to patch DIR-1353 AP. When the AP set to Wep, it will use legacy mode. But AP still keeps 
                // sending BW_Notify Action frame, and cause us to linkup and linkdown. 
                // In legacy mode, don't need to parse HT action frame.
                DBGPRINT(RT_DEBUG_TRACE,("ACTION -Ignore HT Notify Channel BW when link as legacy mode. Wcid=%d, BW = %d---> \n", Elem->Wcid, Elem->Msg[LENGTH_802_11+2] ));
                break;
            }
            DBGPRINT(RT_DEBUG_TRACE,("ACTION - HT Notify Channel bandwidth action----> \n"));
            if (Elem->Msg[LENGTH_802_11+2] == 0)
            {
                // 7.4.8.2. if value is 1, keep the same as supported channel bandwidth. 
                WRITE_PHY_CFG_BW(pAd, &pWcidMacTabEntry->TxPhyCfg, BW_20);                
                DBGPRINT(RT_DEBUG_TRACE,("ACTION - Set wicd = %d to use BW = 0---> \n", Elem->Wcid));
            }
            else
            {
                // Link UP as 40MHz, can set BW as 1 immediately.
                if (pAd->StaActive.SupportedHtPhy.HtChannelWidth == 1)
                {                   
                    WRITE_PHY_CFG_BW(pAd, &pWcidMacTabEntry->TxPhyCfg, BW_40);
                    DBGPRINT(RT_DEBUG_TRACE,("ACTION - HT Notify Channel bandwidth - Link up as 40MHz, can set BW as 1 immediately. \n"));
                }
                // Link up as 20MHz, but HT capability is 40MHz. 
                else if (pPort->CommonCfg.HtCapability.HtCapInfo.ChannelWidth == 1)
                {
                    // Some station might send Notify BW action frame too. So this MlmeCntLinkDown()
                    // is limited to only as station mode.  In AP mode, ignore this packet.
                    if (pPort->PortType == EXTSTA_PORT)
                    {
                        // 2009 PF: We will listen Beacon to change channel bandwidth, so will not link down here !!
                        DBGPRINT(RT_DEBUG_TRACE,("ACTION - Link Downd and Reassociate as 40MHz - because my HT capability is 40MHz.---> \n"));
                    }
                }
                else
                {
                    WRITE_PHY_CFG_BW(pAd, &pWcidMacTabEntry->TxPhyCfg, BW_20);
                    DBGPRINT(RT_DEBUG_TRACE,("ACTION - HT Notify Channel bandwidth - HT capability is 20MHZ. Keep as 20Mhz \n"));
                }
                    
            }
            
            break;
        case SMPS_ACTION:
            // 7.3.1.25
            DBGPRINT(RT_DEBUG_TRACE,("ACTION - SMPS action----> \n"));
            if (((Elem->Msg[LENGTH_802_11+2]&0x1) == 0))
            {
                pWcidMacTabEntry->MmpsMode = MMPS_ENABLE;
            }
            else if (((Elem->Msg[LENGTH_802_11+2]&0x2) == 0))
            {
                pWcidMacTabEntry->MmpsMode = MMPS_STATIC;
            }
            else
            {
                pWcidMacTabEntry->MmpsMode = MMPS_DYNAMIC;
            }
            break;
 
        case SETPCO_ACTION:
            break;
        case MIMO_CHA_MEASURE_ACTION:
            break;
        case HT_INFO_EXCHANGE:
            Information = Elem->Msg[LENGTH_802_11+2];
            DBGPRINT(RT_DEBUG_TRACE,("ACTION - HT Information Exchange= %x----> \n", Information));
            break;
            
        case NON_COPMRESSED_BEAMFORMING: // Non-compressed beamforming report
        {
            if (isEnableETxBf(pAd))
            {
                DBGPRINT(RT_DEBUG_TRACE, ("TxBf: %s: Receive an non-compressed beamforming feedback matrix\n", __FUNCTION__));
            }
        }
        break;

        case COMPRESSED_BEAMFORMING:  // Compressed beamforming report
        {
            if (isEnableETxBf(pAd))
            {
                DBGPRINT(RT_DEBUG_TRACE, ("TxBf: %s: Receive a compressed beamforming feedback matrix\n", __FUNCTION__));
            }
        }
        break;
    }
}

VOID RECBATimerTimeout(
    IN PMP_ADAPTER    pAd)
{
    //MP_ADAPTER  *pAd = (MP_ADAPTER *)FunctionContext;
    PBA_REC_ENTRY   pBAEntry;
    ULONG    RecTabSize;
    PRTMP_REORDERBUF pBuffer;
    UCHAR   count, num, PortNum;
    UCHAR   UserPriority;
    PMP_PORT pPort;

    for(PortNum = 0; PortNum< RTMP_MAX_NUMBER_OF_PORT/*pAd->NumberOfPorts*/;PortNum++)
    {
        if ((pAd->PortList[PortNum] == NULL) || (pAd->PortList[PortNum]->bActive == FALSE))
        {
            continue;
        }

        //reset ReceivingNo here to prevent two port add at this variable the same time.
        pPort = pAd->PortList[PortNum];

        // If now station is in sleep, don't maintain reordering buffer.
        if (INFRA_ON(pPort) && (OPSTATUS_TEST_FLAG(pAd, fOP_STATUS_DOZE)))
            return; 
        
        RecTabSize = pPort->BATable.numAsRecipient;
        for (num = 0; num < MAX_LEN_OF_BA_REC_TABLE;num++)
        {
            if (RecTabSize == 0)
                break;
            pBAEntry = &pPort->BATable.BARecEntry[num];
            NdisAcquireSpinLock(&pBAEntry->RxReRingLock);
            UserPriority = (pBAEntry->TID & 0x07);          
            if (pBAEntry->REC_BA_Status == Recipient_Accept) 
            {
                RecTabSize--;
                if ((pBAEntry->LastRcvSeq == pBAEntry->LastIndSeqAtTimer) && (pBAEntry->NumOfRxPkt > (0)))
                {
                    DBGPRINT(RT_DEBUG_TRACE, ("6-%d.  (Port%d) Timeout Refresh. LastIndSeq = 0x%x.  NumOfRxPkt = 0x%x.\n",num, PortNum, pBAEntry->LastIndSeq, pBAEntry->NumOfRxPkt));
                    // Always shift one packet. The 1st packet must be 
                    pBuffer = &pAd->pRxCfg->LocalRxReorderBuf[pBAEntry->RxBufIdxUsed].MAP_RXBuf[pBAEntry->Curindidx];
                    if (pBuffer->IsFull == TRUE)
                    {
                        DBGPRINT(RT_DEBUG_TRACE, ("6-%d. (Port%d) ERR Timeout Refresh. LastRcvSeq = 0x%x. NumOfRxPkt = 0x%x.\n",num, PortNum, pBAEntry->LastRcvSeq, pBAEntry->NumOfRxPkt));
                    }
                    else
                    {
                        pBAEntry->LastIndSeq++; 
                        pBAEntry->LastIndSeq &= MAXSEQ;
                        INC_RING_INDEX(pBAEntry->Curindidx, Max_RX_REORDERBUF);
                    }
                    // Then go through other packet, 
                    if (pBAEntry->NumOfRxPkt > ((7*pBAEntry->BAWinSize)/8))
                    {                   
                        count = pBAEntry->BAWinSize/2;
                    }
                    else
                    {                   
                        count = 1;
                    }
                
                    do
                    {
                        pBuffer = &pAd->pRxCfg->LocalRxReorderBuf[pBAEntry->RxBufIdxUsed].MAP_RXBuf[pBAEntry->Curindidx];
                        if (pBuffer->IsFull == TRUE)
                        {
                            pBuffer->IsFull = FALSE;

                            if (pBuffer->IsAMSDU)
                            {
                                NdisCommonReportAMSDUEthFrameToLLC(pAd,pPort, pBuffer->pBuffer, pBuffer->Datasize, UserPriority, TRUE);
                            }
                            else
                            {
                                NdisCommonReportEthFrameToLLC(pAd, pPort, pBuffer->pBuffer, pBuffer->Datasize, UserPriority, FALSE, TRUE);                           
                            }
                            
                            pBAEntry->NumOfRxPkt--;
                            DBGPRINT(RT_DEBUG_TRACE, ("66-%d. (Port%d) INDI. %dth buffer. pBuffer->Sequence.value = 0x%x.\n" ,num, PortNum, pBAEntry->Curindidx, pBuffer->Sequence.value));
                            if (pBAEntry->NumOfRxPkt == 0)
                            {
                                INC_RING_INDEX(pBAEntry->Curindidx, Max_RX_REORDERBUF);
                                pBAEntry->LastIndSeq = pBuffer->Sequence.value&MAXSEQ;  
                                    break;
                            }
                            // Since this packet is full, we would like to go through next packet.. So count++ to prolong the while loop
                            count++;
                        }
                        else
                        {
                            // Stop immediately when no packet
                            break;
                        }
                        pBAEntry->LastIndSeq++; 
                        pBAEntry->LastIndSeq &= MAXSEQ;
                        INC_RING_INDEX(pBAEntry->Curindidx, Max_RX_REORDERBUF);
                        count--;
                    }while (count > 0);     

                    DBGPRINT(RT_DEBUG_INFO, ("666. (Port%d) LastIndSeq = 0x%x. NumOfRxPkt = %d.\n" , PortNum, pBAEntry->LastIndSeq, pBAEntry->NumOfRxPkt));
                }
                // Update sequence at timer.
                pBAEntry->LastIndSeqAtTimer = pBAEntry->LastRcvSeq;             
            }           
            NdisReleaseSpinLock(&pBAEntry->RxReRingLock);
        }
    }
}

/*
    ==========================================================================
    Description:
        Retry sending ADDBA Reqest.
        
    IRQL = DISPATCH_LEVEL
    
    Parametrs:
    p8023Header: if this is already 802.3 format, p8023Header is NULL
    
    Return  : TRUE if put into rx reordering buffer, shouldn't indicaterxhere.
                FALSE , then continue indicaterx at this moment.
    ==========================================================================
 */
VOID ORIBATimerTimeout(
    IN  PMP_ADAPTER   pAd,
    IN  PMP_PORT      pPort) 
{
    MAC_TABLE_ENTRY *pEntry;
    ULONG   i,  total;
    FRAME_BAR           FrameBar;   
    PUCHAR          pOutBuffer = NULL;
    ULONG       FrameLen;
    USHORT      Sequence;
    PMAC_TABLE_ENTRY pWcidMacTabEntry = NULL, tmpEntry =NULL;
    NDIS_STATUS     NStatus;
    

    if (pPort->CommonCfg.BACapability.field.Policy == BA_NOTUSE)
    {
        DBGPRINT(RT_DEBUG_TRACE,("BA Policy not use.\n"));
        return;
    }

    DBGPRINT(RT_DEBUG_INFO,("ORIBATimerTimeout--> pPort->MacTab.MacTabList.Number = %d\n", pPort->MacTab.MacTabList.Number));
    
    total = pPort->MacTab.MacTabList.Number * NUM_OF_TID;    
    for (i = 1; ((i <MAX_LEN_OF_BA_ORI_TABLE) && (total > 0)) ; i++)
    {
        DBGPRINT(RT_DEBUG_INFO,("ORIBATimerTimeout-->pPort->BATable.BAOriEntry[%d].ORI_BA_Status = %d, pPort->BATable.BAOriEntry[%d].Token = %d\n", i, pPort->BATable.BAOriEntry[i].ORI_BA_Status, i, pPort->BATable.BAOriEntry[i].Token));
        
        pWcidMacTabEntry = MlmeSyncMacTabMatchedWlanIdx(pPort, pPort->BATable.BAOriEntry[i].Wcid);   
        if(pWcidMacTabEntry == NULL)
        {
            DBGPRINT(RT_DEBUG_TRACE,("%s(), Line:%d, Mac Entry is NULL  Wcid = %d\n", __FUNCTION__, __LINE__, pPort->BATable.BAOriEntry[i].Wcid));
            return;
        }
        
        tmpEntry = MlmeSyncMacTabMatchedWlanIdx(pPort, (USHORT)i);  
        if(pWcidMacTabEntry == NULL)
        {
            DBGPRINT(RT_DEBUG_TRACE,("%s(), Line:%d, Mac Entry is NULL  Wcid = %d\n", __FUNCTION__, __LINE__, i));
            return;
        }
        
        if ((pPort->BATable.BAOriEntry[i].ORI_BA_Status == Originator_WaitRes) && (pPort->BATable.BAOriEntry[i].Token++ < 25))
        {
            MLME_ADDBA_REQ_STRUCT    AddbaReq;  
            pEntry = pWcidMacTabEntry;
            PlatformZeroMemory(&AddbaReq, sizeof(AddbaReq));
            COPY_MAC_ADDR(AddbaReq.pAddr, pEntry->Addr);
            AddbaReq.Wcid = (UCHAR)(pEntry->Aid);
            AddbaReq.TID = pPort->BATable.BAOriEntry[i].TID;
            AddbaReq.BaBufSize = pEntry->BaSizeInUse;
            AddbaReq.TimeOutValue = 0;
            AddbaReq.Token = (++pPort->BATable.BAOriEntry[i].Token);
            MlmeEnqueue(pAd, pPort,ACTION_STATE_MACHINE, MT2_MLME_ADD_BA_CATE, sizeof(MLME_ADDBA_REQ_STRUCT), (PVOID)&AddbaReq);
            DBGPRINT(RT_DEBUG_INFO,("BA Policy = %d. - ADDBATimer. Send to wcid = %d, TID= %d\n", pPort->CommonCfg.BACapability.field.Policy , AddbaReq.Wcid, AddbaReq.TID));
        }
        else if ((pPort->BATable.BAOriEntry[i].ORI_BA_Status == Originator_WaitAdd) &&
                ((NO_SECURITY_ON(pPort) || AES_ON(pPort))))
        {
            // No security || AES case . needn't special behavior 
            MLME_ADDBA_REQ_STRUCT    AddbaReq;  
            pPort->BATable.BAOriEntry[i].ORI_BA_Status = Originator_WaitRes;
            pEntry = pWcidMacTabEntry;
            PlatformZeroMemory(&AddbaReq, sizeof(AddbaReq));
            COPY_MAC_ADDR(AddbaReq.pAddr, pEntry->Addr);
            AddbaReq.Wcid = (UCHAR)(pEntry->Aid);
            AddbaReq.TID = pPort->BATable.BAOriEntry[i].TID;
            AddbaReq.BaBufSize = pEntry->BaSizeInUse;
            AddbaReq.TimeOutValue = 0;
            AddbaReq.Token = 1;
            MlmeEnqueue(pAd, pPort,ACTION_STATE_MACHINE, MT2_MLME_ADD_BA_CATE, sizeof(MLME_ADDBA_REQ_STRUCT), (PVOID)&AddbaReq);
            DBGPRINT(RT_DEBUG_INFO,("BA - ADDBATimer. Send to wcid = %d, TID= %d\n", AddbaReq.Wcid, AddbaReq.TID));
        }
        else if ((pPort->BATable.BAOriEntry[i].ORI_BA_Status == Originator_WaitAdd))
        {
            // WEP || TKIP case . need special behavior 
            pEntry = pWcidMacTabEntry;
            // Ralink chip.  All use AMPDU.
            if (CLIENT_STATUS_TEST_FLAG(pEntry, fCLIENT_STATUS_RALINK_CHIPSET))
            {
                // Ralink can turn on AMPDU in all security case.
                MLME_ADDBA_REQ_STRUCT    AddbaReq;  
                pPort->BATable.BAOriEntry[i].ORI_BA_Status = Originator_WaitRes;
                PlatformZeroMemory(&AddbaReq, sizeof(AddbaReq));
                COPY_MAC_ADDR(AddbaReq.pAddr, pEntry->Addr);
                AddbaReq.Wcid = (UCHAR)(pEntry->Aid);
                AddbaReq.TID = pPort->BATable.BAOriEntry[i].TID;
                AddbaReq.BaBufSize = pEntry->BaSizeInUse;
                AddbaReq.TimeOutValue = 0;
                AddbaReq.Token = (++pPort->BATable.BAOriEntry[i].Token);
                MlmeEnqueue(pAd,pPort, ACTION_STATE_MACHINE, MT2_MLME_ADD_BA_CATE, sizeof(MLME_ADDBA_REQ_STRUCT), (PVOID)&AddbaReq);
                DBGPRINT(RT_DEBUG_INFO,("BA - 0ADDBATimer. Send to wcid = %d, TID= %d\n", AddbaReq.Wcid, AddbaReq.TID));
            }
            // Atheros & WEP on.
            else if (WEP_ON(pPort) && ((pEntry->bIAmBadAtheros == TRUE) || (tmpEntry->RXBAbitmap != 0)))
            {
                // Use AMSDU with atheros in WEP.  Broadmcom WEP&TKIP won't set as itself BA originator.  
                // so We check REC_BA_Status in the above to know it's atheros. Sigh~~~~~~~ 
                CLIENT_STATUS_SET_FLAG(pEntry, fCLIENT_STATUS_AMSDU_INUSED);
                pPort->BATable.BAOriEntry[i].ORI_BA_Status = Originator_NONE;
                DBGPRINT(RT_DEBUG_INFO,("BA - 4ADDBATimer. fCLIENT_STATUS_AMSDU_INUSED\n"));
            }
            // Non-Ralink chip & TKIP.
            else if (TKIP_ON(pPort))
            {
                pPort->BATable.BAOriEntry[i].ORI_BA_Status = Originator_NONE;
                DBGPRINT(RT_DEBUG_ERROR,("BA - 2ADDBATimer. fCLIENT_STATUS_AMSDU_INUSED\n"));
            }
            else
                DBGPRINT(RT_DEBUG_INFO,("BA - 3ADDBATimer.  BARecEntry[0].NumOfRxPkt  = %d,OneSecTxRetryOkCount  = %d \n", pPort->BATable.BARecEntry[0].NumOfRxPkt, pAd->Counter.MTKCounters.OneSecTxRetryOkCount ));
        }
        else if ((pPort->BATable.BAOriEntry[i].ORI_BA_Status == Originator_WaitRes))
        {
            DBGPRINT(RT_DEBUG_INFO,("BA - BA_Status == Originator_WaitRes\n"));
            if (pPort->CommonCfg.BACapability.field.AutoBA == 1)
            {
                pPort->BATable.BAOriEntry[i].ORI_BA_Status = Originator_SetAutoAdd;
                pEntry = pWcidMacTabEntry;
                pEntry->TXAutoBAbitmap |= (1<<(pPort->BATable.BAOriEntry[i].TID));
            }
            else
            pPort->BATable.BAOriEntry[i].ORI_BA_Status = Originator_NONE;
            
            pPort->BATable.BAOriEntry[i].Token = 1;
        }
        else if ((pPort->BATable.BAOriEntry[i].ORI_BA_Status == Originator_Done) && (pPort->CommonCfg.IOTestParm.bSendBAR == TRUE))
        {
            //pPort->CommonCfg.IOTestParm.bSendBAR = FALSE;       
            pEntry = pWcidMacTabEntry;
            Sequence = pEntry->BAOriSequence[pPort->BATable.BAOriEntry[i].TID];
            if (Sequence == 0)
                Sequence = 0xfff;
            else
                Sequence--;
            BarHeaderInit(pAd,pPort, &FrameBar, pWcidMacTabEntry->Addr);

            FrameBar.StartingSeq.field.FragNum = 0; // make sure sequence not clear in DEL function.
            FrameBar.StartingSeq.field.StartSeq = Sequence; // make sure sequence not clear in DEL funciton.
            FrameBar.BarControl.TID = pPort->BATable.BAOriEntry[i].TID; // make sure sequence not clear in DEL funciton.
            DBGPRINT(RT_DEBUG_ERROR,("BA - bSendBAR = %d Seq = 0x%x TID = %d Wcid = %d\n", pPort->CommonCfg.IOTestParm.bSendBAR, Sequence,pPort->BATable.BAOriEntry[i].TID,pPort->BATable.BAOriEntry[i].Wcid));

            // Use StartSeq for Wcid. N6USBComposeUnit will update later
            FrameBar.StartingSeq.field.StartSeq = pPort->BATable.BAOriEntry[i].Wcid;            
            // We found INTEL station is very sensitive to lost packet. So if there is retry failed, 
            if (((pPort->PortSubtype == PORTSUBTYPE_P2PGO) || (pPort->PortSubtype == PORTSUBTYPE_STA) || 
                (pPort->PortSubtype == PORTSUBTYPE_SoftapAP) || (pPort->PortSubtype == PORTSUBTYPE_VwifiAP)) 
                && (!(CLIENT_STATUS_TEST_FLAG(pEntry, fCLIENT_STATUS_RALINK_CHIPSET))))
            {               
                
                NStatus = MlmeAllocateMemory(pAd, &pOutBuffer);  //Get an unused nonpaged memory
                if(NStatus != NDIS_STATUS_SUCCESS) 
                {
                    DBGPRINT(RT_DEBUG_TRACE,("BA - MlmeDELBAAction() allocate memory failed \n"));
                    return;
                }
               
                MakeOutgoingFrame(pOutBuffer,              &FrameLen,
                              sizeof(FRAME_BAR),      &FrameBar,
                              END_OF_ARGS);
                //Send BAR to refresh peer reordering buffer.
                NdisCommonMiniportMMRequest(pAd, pOutBuffer, FrameLen);
                DBGPRINT(RT_DEBUG_TRACE,("BA - MlmeDELBAAction() . Send BAR to refresh peer reordering buffer \n"));
            
            }           
        }
        else if ((pPort->BATable.BAOriEntry[i].ORI_BA_Status == Originator_Done) && (pPort->CommonCfg.IOTestParm.bSendBAR == FALSE))
        {
            pEntry = pWcidMacTabEntry;
            pEntry->NoBADataCountDown = 0;
        }
        total --;
    }

    // I don't have Tx counter per AC.
    if (pPort->CommonCfg.IOTestParm.bSendBAR == TRUE)
        pPort->CommonCfg.IOTestParm.bSendBAR = FALSE; 

    DBGPRINT(RT_DEBUG_INFO,("ORIBATimerTimeout<--\n"));
    
}

VOID ActHeaderInit(
    IN  PMP_ADAPTER   pAd, 
    IN PMP_PORT pPort,
    IN OUT PHEADER_802_11 pHdr80211, 
    IN PUCHAR pDA, 
    IN PUCHAR pBssid) 
{
    PlatformZeroMemory(pHdr80211, sizeof(HEADER_802_11));
    pHdr80211->FC.Type = BTYPE_MGMT;
    pHdr80211->FC.SubType = SUBTYPE_ACTION;
    COPY_MAC_ADDR(pHdr80211->Addr1, pDA);
    COPY_MAC_ADDR(pHdr80211->Addr2, pPort->CurrentAddress);
    COPY_MAC_ADDR(pHdr80211->Addr3, pBssid);
}

VOID BarHeaderInit(
    IN  PMP_ADAPTER   pAd, 
    IN  PMP_PORT      pPort, 
    IN OUT PFRAME_BAR pCntlBar, 
    IN PUCHAR pDA) 
{
    
    PlatformZeroMemory(pCntlBar, sizeof(FRAME_BAR));
    pCntlBar->FC.Type = BTYPE_CNTL;
    pCntlBar->FC.SubType = SUBTYPE_BLOCK_ACK_REQ;
    pCntlBar->BarControl.MTID = 0;
    pCntlBar->BarControl.Compressed = 1;
    pCntlBar->BarControl.ACKPolicy = 0;

    pCntlBar->Duration = 16 + XmitCalcDuration(pAd, RATE_1, sizeof(FRAME_BAR));

    COPY_MAC_ADDR(pCntlBar->Addr1, pDA);
    COPY_MAC_ADDR(pCntlBar->Addr2, pPort->CurrentAddress);
}

/*
    ==========================================================================
    Description:
        Check Every QoSData frame's sequence in BA session to decide whether 1.put to reordering buffer or 
        2. indicate receive because it's in sequence.
        
    IRQL = DISPATCH_LEVEL
    
    Parametrs:
    p8023Header: if this is already 802.3 format, p8023Header is NULL
    
    Return  : TRUE if put into rx reordering buffer, shouldn't indicaterxhere.
                FALSE , then continue indicaterx at this moment.
    ==========================================================================
 */
BOOLEAN QosBADataParse(
    IN PMP_ADAPTER    pAd, 
    IN PMP_PORT   pPort, 
    IN BOOLEAN bAMSDU,
    IN BOOLEAN bIsBtAcl,
    IN PUCHAR p80211DataFrame,
    IN UCHAR    Wcid,
    IN UCHAR    TID,
    IN USHORT Sequence,
    IN USHORT u80211FrameSize)
{
    ULONG   Idx = 3;
    UCHAR       brc = NORMAL_RECEIVE;
    PBA_REC_ENTRY   pBAEntry = NULL;
    PRTMP_REORDERBUF    pReBuffer;
    PHEADER_802_11  pHeader;
    INT i;
    UCHAR UserPriority = TID & 0x07;
    PMAC_TABLE_ENTRY pWcidMacTabEntry = MlmeSyncMacTabMatchedWlanIdx(pPort, Wcid);   
    pHeader = (PHEADER_802_11) p80211DataFrame;

    if(pWcidMacTabEntry == NULL)
    {
        DBGPRINT(RT_DEBUG_TRACE,("%s(), Line:%d, Mac Entry is NULL  Wcid = %d\n", __FUNCTION__, __LINE__, Wcid));
        return FALSE;
    }

    if (Wcid < MAX_LEN_OF_MAC_TABLE)
    {
        // if this receiving packet is from SA that is in our OriEntry. Since WCID <9 has direct mapping. no need search.
        Idx = pWcidMacTabEntry->BARecWcidArray[TID];
        pBAEntry = &pPort->BATable.BARecEntry[Idx];
    }
    else
    {
        return FALSE;
    }

    ASSERT( pHeader->Sequence == Sequence );

    if (pBAEntry->REC_BA_Status == Recipient_Accept)
    {
        // find corresponding BaEntry and IMME BACK policy in use, so put into reordering buffer
        //  *****************************************************************************
        //  start insert , NO Wait For BAR
        //  *****************************************************************************   
        // 0-0. update last sequence 
        // 1. check if in order.
        pBAEntry->LastRcvSeq = pHeader->Sequence;       
        if (SEQ_STEPONE (pHeader->Sequence, pBAEntry->LastIndSeq) )
        {           
            NdisAcquireSpinLock(&pBAEntry->RxReRingLock);
            pBAEntry->LastIndSeq = pHeader->Sequence;
            INC_RING_INDEX(pBAEntry->Curindidx, Max_RX_REORDERBUF); 


            NdisReleaseSpinLock(&pBAEntry->RxReRingLock);
            brc = TRUE;
            if (bAMSDU)
            {
                NdisCommonReportAMSDUEthFrameToLLC(pAd,pPort, p80211DataFrame, u80211FrameSize, UserPriority, FALSE);
            }
            else
            {
                NdisCommonReportEthFrameToLLC(pAd, pPort, p80211DataFrame, u80211FrameSize, UserPriority, FALSE, FALSE);
            }
            
            NdisAcquireSpinLock(&pBAEntry->RxReRingLock);
            if ((pBAEntry->NumOfRxPkt > 0))
            {
                
                for ( i=1; i < pBAEntry->BAWinSize ; i++)
                {
                    pReBuffer = &pAd->pRxCfg->LocalRxReorderBuf[pBAEntry->RxBufIdxUsed].MAP_RXBuf[pBAEntry->Curindidx];
                    //pBuffer = &pBAEntry->MAP_RXBuf[pBAEntry->Curindidx];
                    if (pReBuffer->IsFull == TRUE)
                    {
                        pReBuffer->IsFull = FALSE;
                        if (pReBuffer->IsAMSDU)
                        {
                            NdisCommonReportAMSDUEthFrameToLLC(pAd, pPort,pReBuffer->pBuffer, pReBuffer->Datasize, UserPriority, FALSE);
                        }
                        else
                        {
                            NdisCommonReportEthFrameToLLC(pAd,pPort, pReBuffer->pBuffer, pReBuffer->Datasize, UserPriority, FALSE, FALSE);
                        }
                        pBAEntry->LastIndSeq = pReBuffer->Sequence.value&MAXSEQ;                                
                        pBAEntry->NumOfRxPkt-- ;
                        DBGPRINT(RT_DEBUG_INFO, ("22-%ds. INDI. %dth buffer. pBuffer->Sequence = 0x%x, NumOfRxPkt = %d\n" , Idx,pBAEntry->Curindidx, pReBuffer->Sequence.value, pBAEntry->NumOfRxPkt));
                        INC_RING_INDEX(pBAEntry->Curindidx, Max_RX_REORDERBUF);
                    }
                    else
                    {
                        break;
                    }
                }

                
            }
            NdisReleaseSpinLock(&pBAEntry->RxReRingLock);
            return brc;

        }
        //  rule 2.  Duplicate check.
        else if (pHeader->Sequence == pBAEntry->LastIndSeq)
        {
            DBGPRINT(RT_DEBUG_INFO, ("33-%d. DUPLICATE receive pHeader->Sequence = 0x%x ,LastIndSeq = 0x%x. drop \n" , Idx, pHeader->Sequence, pBAEntry->LastIndSeq));
            pAd->Counter.WlanCounters.FrameDuplicateCount.LowPart ++;   
            //drop this
            brc = TRUE;
        }
        // ====================================================
        // check rule 2-2.   Duplicate check.
        else if(SEQ_SMALLER(pHeader->Sequence, pBAEntry->LastIndSeq))
        {
            DBGPRINT(RT_DEBUG_INFO, ("3-%d. DUPLICATE receive pHeader->Sequence = 0x%x ,LastIndSeq = 0x%x. drop \n" , Idx, pHeader->Sequence, pBAEntry->LastIndSeq));
            pAd->Counter.WlanCounters.FrameDuplicateCount.LowPart ++;       
            //drop this
            brc = TRUE;
        }
        // 4 Receive seq within Window Size
        else if (SEQ_WITHIN_WIN(pHeader->Sequence, pBAEntry->LastIndSeq, pBAEntry->BAWinSize))
        {
            // Put to CurIndex + (RecSeq - LastSeq) -1
            i = (pHeader->Sequence > pBAEntry->LastIndSeq)?(pHeader->Sequence - pBAEntry->LastIndSeq -1):(pHeader->Sequence + MAXSEQ - pBAEntry->LastIndSeq);
            if ( i > pBAEntry->BAWinSize)
            {
                DBGPRINT(RT_DEBUG_TRACE, ("!!pHeader->Sequence = 0x%x,pBAEntry->LastIndSeq = 0x%x, Header->Frag = 0x%x \n", pHeader->Sequence, pBAEntry->LastIndSeq, pHeader->Frag));
                brc = TRUE;
            }
            else
            {
                NdisAcquireSpinLock(&pBAEntry->RxReRingLock);
                pReBuffer = &pAd->pRxCfg->LocalRxReorderBuf[pBAEntry->RxBufIdxUsed].MAP_RXBuf[((pBAEntry->Curindidx+i)%Max_RX_REORDERBUF)];
                if (pReBuffer->IsFull == FALSE)
                {
                    // Write RxD buffer address & allocated buffer length
                    
                    pReBuffer->Sequence.value = (USHORT)pHeader->Sequence & MAXSEQ;
                    pReBuffer->Datasize = u80211FrameSize;
                    //Copy packet to reorder buffer
                    PlatformMoveMemory(pReBuffer->pBuffer, p80211DataFrame, u80211FrameSize);
                    pBAEntry->NumOfRxPkt++;  
                    pReBuffer->IsFull = TRUE;
                    pReBuffer->IsAMSDU = bAMSDU;
  
                    DBGPRINT(RT_DEBUG_INFO, ("4-%d. Rx out of order. Insert[%dth] to [%dth]   pBAEntry->LastIndSeq =0x%x  pHeader->Seq = 0x%x, Datasize = 0x%x\n",
                             Idx,i, pBAEntry->Curindidx ,pBAEntry->LastIndSeq , pHeader->Sequence, pReBuffer->Datasize));
                    // don't indicate 
                    brc = TRUE;
                }
                else
                {
                    DBGPRINT(RT_DEBUG_INFO,  ("44-%d. Duplicate  Reorder Buffer pHeader->Sequence = 0x%x,\n", Idx, pHeader->Sequence));
                    // don't indicate 
                    brc = TRUE;
                }
                NdisReleaseSpinLock(&pBAEntry->RxReRingLock);
            }
        }
        // 5.1 Receive seq surpasses Win(lastseq + nMSDU). So refresh all reorder buffer
        else
        {
            DBGPRINT(RT_DEBUG_TRACE, ("5-%d. Refresh. pHeader->Sequence=0x%x, #RxPkt = %d. Curindidx = 0x%x  LastIndSeq  = 0x%x.\n", Idx,pHeader->Sequence,pBAEntry->NumOfRxPkt, pBAEntry->Curindidx, pBAEntry->LastIndSeq  ));
            // Then refresh all reorder buffer. and update LastSeq = ReceiveSeq
            NdisAcquireSpinLock(&pBAEntry->RxReRingLock);
            DBGPRINT(RT_DEBUG_INFO, ("55-%d. INDI Sequences are : \n", Idx));
            for ( i=0; i < pBAEntry->BAWinSize; i++)
            {
                pReBuffer = &pAd->pRxCfg->LocalRxReorderBuf[pBAEntry->RxBufIdxUsed].MAP_RXBuf[pBAEntry->Curindidx];
                //pBuffer = &pBAEntry->MAP_RXBuf[pBAEntry->Curindidx];
                if (pReBuffer->IsFull == TRUE)
                {
                    pReBuffer->IsFull = FALSE;
                    
                    if (pReBuffer->IsAMSDU)
                    {
                        NdisCommonReportAMSDUEthFrameToLLC(pAd, pPort,pReBuffer->pBuffer, pReBuffer->Datasize, UserPriority, FALSE);
                    }
                    else
                    {
                        NdisCommonReportEthFrameToLLC(pAd,pPort, pReBuffer->pBuffer, pReBuffer->Datasize, UserPriority, FALSE, FALSE);
                    }

                    pBAEntry->LastIndSeq = pReBuffer->Sequence.value&MAXSEQ;    
                    if (pBAEntry->NumOfRxPkt == 0)
                    {   
                        DBGPRINT_ERR( ("55 Error Counting Reorder Buffer\n"));
                    }
                    else
                        pBAEntry->NumOfRxPkt--;
                    DBGPRINT(RT_DEBUG_INFO, ("0x%x, ", pReBuffer->Sequence.value));
                }
                INC_RING_INDEX(pBAEntry->Curindidx, Max_RX_REORDERBUF);
            }
            DBGPRINT(RT_DEBUG_INFO, ("\n"));            

            if (pBAEntry->LastIndSeq == pHeader->Sequence)
            {
                DBGPRINT_ERR( ("555-%d refresh . pHeader->Sequence = %x\n", Idx, pHeader->Sequence));
                //not receive here
                brc = TRUE;
            }
            else
            {
                pBAEntry->LastIndSeq = pHeader->Sequence;
                DBGPRINT(RT_DEBUG_TRACE, ("55-%d  INDI.  pHeader->Sequence  0x%x\n" , Idx,pBAEntry->LastIndSeq));
                // receive here
                brc = FALSE;
            }
            pBAEntry->NumOfRxPkt  = 0;  
            NdisReleaseSpinLock(&pBAEntry->RxReRingLock);
        }
        return brc ;
    }
    else
    {
        return FALSE;
    }
}

/*
Description : Handle subtype = BlockAckRequest control frame (BAR).
BAR frame is used to sync frame sequence in BA session. 
*/
BOOLEAN CntlEnqueueForRecv(
    IN  PMP_ADAPTER   pAd, 
    IN  PMP_PORT      pPort, 
    IN ULONG Wcid, 
    IN ULONG MsgLen, 
    IN PFRAME_BA_REQ pMsg) 
{
    PFRAME_BA_REQ   pFrame = pMsg;
    PRTMP_REORDERBUF    pBuffer;
    BA_REC_ENTRY *pBAEntry;
    ULONG   Idx;
    UCHAR   TID, i, UserPriority = 0;
    PMAC_TABLE_ENTRY pWcidMacTabEntry = NULL;

    if (Wcid == 0xff)  // workaround......need to check with HW designer 2014.05.28
    {
        pWcidMacTabEntry = MlmeSyncMacTabMatchedBssid(pPort, pFrame->Addr2);     

        if(pWcidMacTabEntry == NULL)
        {
            DBGPRINT(RT_DEBUG_TRACE,("%s(), Line:%d, Mac Entry is NULL  Wcid = %d\n", __FUNCTION__, __LINE__, Wcid));
            return FALSE;
        }
    }
    else
    {
    pWcidMacTabEntry = MlmeSyncMacTabMatchedWlanIdx(pPort, (USHORT)Wcid);     

    if(pWcidMacTabEntry == NULL)
    {
        DBGPRINT(RT_DEBUG_TRACE,("%s(), Line:%d, Mac Entry is NULL  Wcid = %d\n", __FUNCTION__, __LINE__, Wcid));
        return FALSE;
    }
    }

    if(pWcidMacTabEntry == NULL)
    {
        DBGPRINT(RT_DEBUG_TRACE,("%s(), Line:%d, Mac Entry is NULL  Wcid = %d\n", __FUNCTION__, __LINE__, Wcid));
        return FALSE;
    }
    
    TID = (UCHAR)pFrame->BARControl.TID;
    UserPriority = TID & 0x07;

    // Do nothing if the driver is starting halt state.
    // This might happen when timer already been fired before cancel timer with mlmehalt
    if (MT_TEST_FLAG(pAd, fRTMP_ADAPTER_HALT_IN_PROGRESS))
        return FALSE;

    // First check the size, it MUST not exceed the mlme queue size
    if (MsgLen > MGMT_DMA_BUFFER_SIZE)
    {
        DBGPRINT_ERR(("%s: frame too large, size = %d \n",__FUNCTION__,MsgLen));
        return FALSE;
    }
    else if (MsgLen != sizeof(FRAME_BA_REQ))
    {
        DBGPRINT_ERR(("%s: BlockAck Request frame length size = %d incorrect\n",__FUNCTION__,MsgLen));
        return FALSE;
    }
        
    if ((Wcid < MAX_LEN_OF_MAC_TABLE) && (TID < 8))
    {
        // if this receiving packet is from SA that is in our OriEntry. Since WCID <9 has direct mapping. no need search.
        Idx = pWcidMacTabEntry->BARecWcidArray[TID];
        pBAEntry = &pPort->BATable.BARecEntry[Idx];
    }
    else
    {
        return FALSE;
    }
    
    //
    // To patch Atheros send out invalid BAR frame.
    // 
    if (pWcidMacTabEntry->BARStartSeq != pFrame->BAStartingSeq.field.StartSeq)
    {
        pWcidMacTabEntry->BARStartSeq = pFrame->BAStartingSeq.field.StartSeq;
    }
    else
    {
        if ((pFrame->BARControl.Rsv1 != 0) && (pFrame->BARControl.Compressed == 0))
        {
            pWcidMacTabEntry->BadBARCount++;
        }
    }
    
    // Then refresh all reorder buffer. and update LastSeq = ReceiveSeq
    if (SEQ_SMALLER(pBAEntry->LastIndSeq, pFrame->BAStartingSeq.field.StartSeq))
    {
        DBGPRINT(RT_DEBUG_TRACE, ("Receive BAR : LastIndSeq  = 0x%x, Starting Seq = 0x%x, [wcid = %d, TID = %d].  Curindidx = %d \n", pBAEntry->LastIndSeq, pFrame->BAStartingSeq.field.StartSeq, Wcid, TID,pBAEntry->Curindidx));
        NdisAcquireSpinLock(&pBAEntry->RxReRingLock);
        for ( i=0; i < pBAEntry->BAWinSize; i++)
        {
            if (SEQ_STEPONE (pFrame->BAStartingSeq.field.StartSeq, pBAEntry->LastIndSeq) )
            {
                if (pBAEntry->NumOfRxPkt > 0)
                {
                    for ( i=1; i < pBAEntry->BAWinSize ; i++)
                    {
                        pBuffer = &pAd->pRxCfg->LocalRxReorderBuf[pBAEntry->RxBufIdxUsed].MAP_RXBuf[pBAEntry->Curindidx];
                        //pBuffer = &pBAEntry->MAP_RXBuf[pBAEntry->Curindidx];
                        if (pBuffer->IsFull == TRUE)
                        {
                            pBuffer->IsFull = FALSE;
                            if (pBuffer->IsAMSDU)
                            {
                                NdisCommonReportAMSDUEthFrameToLLC(pAd,pPort, pBuffer->pBuffer, pBuffer->Datasize, UserPriority, FALSE);
                            }
                            else
                                NdisCommonReportEthFrameToLLC(pAd,pPort, pBuffer->pBuffer, pBuffer->Datasize, UserPriority, FALSE, FALSE);

                            pBAEntry->LastIndSeq = pBuffer->Sequence.value&MAXSEQ;      
                            if (pBAEntry->NumOfRxPkt > 0)
                                pBAEntry->NumOfRxPkt-- ;
                            DBGPRINT(RT_DEBUG_INFO, ("BAR : 22-%d. INDI. %dth buffer. pBuffer->Sequence = 0x%x, NumOfRxPkt = %d\n" , Idx,pBAEntry->Curindidx, pBuffer->Sequence.value, pBAEntry->NumOfRxPkt));
                            INC_RING_INDEX(pBAEntry->Curindidx, Max_RX_REORDERBUF);
                        }
                        else
                            break;
                    }
                }
                // After we flush the reordering queue, break BAR ;
                break;
            }
            pBuffer = &pAd->pRxCfg->LocalRxReorderBuf[pBAEntry->RxBufIdxUsed].MAP_RXBuf[pBAEntry->Curindidx];
            if (pBuffer->IsFull == TRUE)
            {
                pBuffer->IsFull = FALSE;
                if (pBuffer->IsAMSDU)
                {
                    NdisCommonReportAMSDUEthFrameToLLC(pAd, pPort,pBuffer->pBuffer, pBuffer->Datasize, UserPriority, FALSE);
                }
                else
                    NdisCommonReportEthFrameToLLC(pAd,pPort, pBuffer->pBuffer, pBuffer->Datasize, UserPriority, FALSE, FALSE);

                if (pBAEntry->NumOfRxPkt == 0)
                {   DBGPRINT_ERR( ("55 Error Counting Reorder Buffer\n"));}
                else
                    pBAEntry->NumOfRxPkt--;
                DBGPRINT(RT_DEBUG_TRACE, ("0x%x, ", pBuffer->Sequence.value));
            }
            pBAEntry->LastIndSeq++; 
            pBAEntry->LastIndSeq &= MAXSEQ;
            INC_RING_INDEX(pBAEntry->Curindidx, Max_RX_REORDERBUF);
        }
        DBGPRINT(RT_DEBUG_INFO, ("\n"));

        NdisReleaseSpinLock(&pBAEntry->RxReRingLock);
    }
            
        return TRUE;
}

/*
Description : When there is a Ba policy switch.  (Auto <-> Manual)

Definition :
Auto : After link up, when there is a unicast frame, BA session will be automatically setup.
Manual : BA session won't be setup unless use "ADD button" on RAUI 11n setting page. 
*/
VOID BaAutoManSwitch(
    IN  PMP_ADAPTER   pAd,
    IN  PMP_PORT      pPort)
{
    UCHAR   j;
    PMAC_TABLE_ENTRY pEntry = NULL;
    PMAC_TABLE_ENTRY pNextEntry = NULL;
    PQUEUE_HEADER pHeader; 
    
    if (pPort->MacTab.Size == 0)
        return;
    
    DBGPRINT(RT_DEBUG_TRACE, ("BaAutoManSwitch====>"));
    // Now switch from Manual to Auto
    if (pPort->CommonCfg.BACapability.field.AutoBA)
    {
        DBGPRINT_RAW(RT_DEBUG_TRACE, (" : Auto\n"));

        pHeader = &pPort->MacTab.MacTabList;
        pNextEntry = (PMAC_TABLE_ENTRY)(pHeader->Head);
        while (pNextEntry != NULL)
        {
            pEntry = (PMAC_TABLE_ENTRY)pNextEntry;
            
            if(pEntry->WlanIdxRole == ROLE_WLANIDX_BSSID)
            {
                pNextEntry = pNextEntry->Next;    
                pEntry = NULL;         
                continue;
            }
            
            if (pEntry->ValidAsCLI)
            {
                for (j = 0; j <NUM_OF_TID; j++)
                {
                    // BaEntry starts from 1, so  0 means no entry is assigned to this {peer,Tid} yet.
                    if (pEntry->BAOriWcidArray[j] == 0)
                    {
                        BATableInsertEntry(pAd, pPort, pEntry->Aid,  0, 0x1, j, pEntry->BaSizeInUse,  Originator_SetAutoAdd, FALSE);
                        pEntry->TXAutoBAbitmap |= (1<<j);
                    }
                }
            }
            pNextEntry = pNextEntry->Next;    
            pEntry = NULL;
        }
    }
    // Now switch from Auto to Manual.
    else
    {
        USHORT i=0;
        DBGPRINT_RAW(RT_DEBUG_TRACE, (" : Manual\n"));
        for (i = 0; i <MAX_LEN_OF_BA_ORI_TABLE ; i++)
        {
            if (pPort->BATable.BAOriEntry[i].ORI_BA_Status != Originator_Done)
            {
                pPort->BATable.BAOriEntry[i].ORI_BA_Status = Originator_NONE;
            }
        }
    }

}

/*
Description : Send SMPS Action frame If SMPS mode switches.
*/
//2 1x1 chip does not support MIMO power save mode.
VOID SendSMPSAction(
    IN  PMP_ADAPTER   pAd,
    IN  PMP_PORT      pPort,
    IN    UCHAR  Wcid,
    IN    UCHAR  Smps) 
{
    PUCHAR          pOutBuffer = NULL;
    NDIS_STATUS     NStatus;
    FRAME_SMPS_ACTION   Frame;
    ULONG           FrameLen, macdata = 0;
    UCHAR           bbpdata = 0;
    PMAC_TABLE_ENTRY pWcidMacTabEntry = NULL;
    pWcidMacTabEntry = MlmeSyncMacTabMatchedWlanIdx(pPort, Wcid);   
    NStatus = MlmeAllocateMemory(pAd, &pOutBuffer);  //Get an unused nonpaged memory
    if(NStatus != NDIS_STATUS_SUCCESS) 
    {
        DBGPRINT(RT_DEBUG_ERROR,("BA - %s() allocate memory failed \n",__FUNCTION__));
        return;
    }

    if(pWcidMacTabEntry == NULL)
    {
        DBGPRINT(RT_DEBUG_TRACE,("%s(), Line:%d, Mac Entry is NULL  Wcid = %d\n", __FUNCTION__, __LINE__, Wcid));
        return;
    }
    
    ActHeaderInit(pAd, pPort, &Frame.Hdr, pWcidMacTabEntry->Addr, pPort->PortCfg.Bssid);
    Frame.Category = CATEGORY_HT;
    Frame.Action = SMPS_ACTION;

    // If in power save state, force wake up to configure register
    if (OPSTATUS_TEST_FLAG(pAd, fOP_STATUS_DOZE) 
        || MT_TEST_FLAG(pAd, fRTMP_ADAPTER_IDLE_RADIO_OFF))
        AsicForceWakeup(pAd);

    switch (Smps)
    {
        case MMPS_ENABLE:
            DBGPRINT(RT_DEBUG_ERROR,("HT - MMPS_ENABLE( %d )  \n", Frame.Smps));

            Frame.Smps = 0;
            if (IS_P2P_GO_OP(pPort))
            {
                pPort->SoftAP.ApCfg.HtCapability.HtCapInfo.MimoPs = 0;
            }
            break;

        case MMPS_DYNAMIC:
            DBGPRINT(RT_DEBUG_ERROR,("HT - MMPS_DYNAMIC( %d )  \n", Frame.Smps));

            Frame.Smps = 3;
            if (IS_P2P_GO_OP(pPort))
            {
                pPort->SoftAP.ApCfg.HtCapability.HtCapInfo.MimoPs = 3;
            }
            break;

        case MMPS_STATIC:
            DBGPRINT(RT_DEBUG_ERROR,("HT - MMPS_STATIC( %d )  \n", Frame.Smps));

            Frame.Smps = 1;
            if (IS_P2P_GO_OP(pPort))
            {
                pPort->SoftAP.ApCfg.HtCapability.HtCapInfo.MimoPs = 1;
            }
            break;
    }
    MakeOutgoingFrame(pOutBuffer,               &FrameLen,
                  sizeof(FRAME_PSMP_ACTION),      &Frame,
                  END_OF_ARGS);
    NdisCommonMiniportMMRequest(pAd, pOutBuffer, FrameLen);
    DBGPRINT(RT_DEBUG_ERROR,("HT - SendSMPSAction( %d )  \n", Frame.Smps));
}

BOOLEAN ChannelSwitchSanityCheck(
    IN  PMP_ADAPTER   pAd,
    IN    UCHAR  Wcid,
    IN    UCHAR  NewChannel,
    IN    UCHAR  Secondary) 
{
    UCHAR       Idx;
    
    DBGPRINT(RT_DEBUG_TRACE, ("!!!ChannelSwitchSanityCheck !!! NewChannel = %d. \n", NewChannel ));
    // From unknow SSID.
    if (Wcid >= MAX_LEN_OF_MAC_TABLE)
        return FALSE;

    // lower or uppwer extension channel sanity check
    if ((NewChannel > 7) && (Secondary == EXTCHA_ABOVE))
        return FALSE;

    if ((NewChannel < 5) && (Secondary == EXTCHA_BELOW))
        return FALSE;

    // Check if new (control) channel is in our channellist which we currently agree to operate in.
    for (Idx = 0;Idx < pAd->HwCfg.ChannelListNum;Idx++)
    {
        if (pAd->HwCfg.ChannelList[Idx].Channel == NewChannel)
        {
            break;
        }
    }

    if (Idx == pAd->HwCfg.ChannelListNum)
        return FALSE;
    
    return TRUE;
}


VOID ChannelSwitchAction(
    IN  PMP_ADAPTER   pAd,
    IN    UCHAR  Wcid,
    IN    UCHAR  NewChannel,
    IN    UCHAR  Secondary) 
{
    UCHAR       BBPValue = 0;
    ULONG       MACValue = 0;
    PMP_PORT    pPort = pAd->PortList[FXXK_PORT_0];
    PMAC_TABLE_ENTRY pWcidMacTabEntry = NULL;
    pWcidMacTabEntry = MlmeSyncMacTabMatchedWlanIdx(pPort, Wcid);     
    DBGPRINT(RT_DEBUG_TRACE,("SPECTRUM - ChannelSwitchAction(NewChannel = %d , Secondary = %d)  \n", NewChannel, Secondary));

    if(pWcidMacTabEntry == NULL)
    {
        DBGPRINT(RT_DEBUG_TRACE,("%s(), Line:%d, Mac Entry is NULL  Wcid = %d\n", __FUNCTION__, __LINE__, Wcid));
        return;
    }

    if (ChannelSwitchSanityCheck(pAd, Wcid, NewChannel, Secondary) == FALSE)
        return;
    
    // 1.  Switches to BW = 20.
    if (Secondary == 0)
    {
        pPort->BBPCurrentBW = BW_20;
        pPort->Channel = NewChannel;
        pPort->CentralChannel = pPort->Channel;
        SwitchBandwidth(pAd, TRUE, pPort->Channel, BW_20, EXTCHA_NONE);

        DBGPRINT(RT_DEBUG_TRACE, ("!!!20MHz   !!! \n" ));
    }
    // 1.  Switches to BW = 40 And Station supports BW = 40.
    else if (((Secondary == 1) || (Secondary == 3)) && (pPort->CommonCfg.HtCapability.HtCapInfo.ChannelWidth == BW_40))
    {
        pPort->Channel = NewChannel;
        pPort->BBPCurrentBW = BW_40;

        if (Secondary == 1)
        {
            pPort->CentralChannel = pPort->Channel + 2;
            SwitchBandwidth(pAd, TRUE, pPort->CentralChannel, BW_40, EXTCHA_ABOVE);

            DBGPRINT(RT_DEBUG_TRACE, ("!!!40MHz Lower LINK UP !!! Control Channel at Below. Central = %d \n", pPort->CentralChannel ));
        }
        else
        {
            pPort->CentralChannel = pPort->Channel - 2;

            SwitchBandwidth(pAd, TRUE, pPort->CentralChannel, BW_40, EXTCHA_BELOW);

            DBGPRINT(RT_DEBUG_TRACE, ("!!!40MHz Upper LINK UP !!! Control Channel at UpperCentral = %d \n", pPort->CentralChannel ));
        }
    }

    WRITE_PHY_CFG_BW(pAd, &pWcidMacTabEntry->TxPhyCfg, pPort->BBPCurrentBW);

}

/*
Description : Build Intolerant Channel Rerpot from Trigger event table.
return : how many bytes copied. 
*/
ULONG BuildIntolerantChannelRep(
    IN  PMP_ADAPTER   pAd,
    IN    PUCHAR  pDest) 
{
    ULONG           FrameLen = 0;
    ULONG           ReadOffset = 0;
    UCHAR           i, j, k, idx = 0;
    UCHAR           ChannelList[MAX_TRIGGER_EVENT];
    UCHAR           TmpRegClass;
    UCHAR           RegClassArray[7] = {0, 11,12, 32, 33, 54,55}; // Those regulatory class has channel in 2.4GHz. See Annex J.
    PMP_PORT    pPort = pAd->PortList[FXXK_PORT_0];

    PlatformZeroMemory(ChannelList, MAX_TRIGGER_EVENT);
    if (pPort->CommonCfg.TgnControl.field.CoexIntolerantCh3Report == TRUE)        
    {
        idx = 1;
        TmpRegClass = 0;
        ChannelList[0] = 3;
        if (idx > 0)
        {
            // For each regaulatory IE report, contains all channels that has the same regulatory class.
            *(pDest + ReadOffset) = IE_2040_BSS_INTOLERANT_REPORT;  // IE
            *(pDest + ReadOffset + 1) = 1+ idx; // Len = RegClass byte + channel byte.
            *(pDest + ReadOffset + 2) = TmpRegClass;    // Len = RegClass byte + channel byte.
            PlatformMoveMemory(pDest + ReadOffset + 3, ChannelList, idx);

            FrameLen += (3 + idx);
            ReadOffset += (3 + idx);
        }   
    }
    else
    {
        // Find every regulatory class
        for ( k = 0;k < 7;k++)
        {
            TmpRegClass = RegClassArray[k];
            
            idx = 0;
            // Find Channel report with the same regulatory class in 2.4GHz.
            for ( i = 0;i < pPort->CommonCfg.TriggerEventTab.EventANo;i++)
            {
                if (pPort->CommonCfg.TriggerEventTab.EventA[i].bValid == TRUE)
                {
                    if (pPort->CommonCfg.TriggerEventTab.EventA[i].RegClass == TmpRegClass)
                    {               
                        for (j = 0;j < idx;j++)
                        {
                            if (ChannelList[j] == (UCHAR)pPort->CommonCfg.TriggerEventTab.EventA[i].Channel)
                                break;
                        }
                        if ((j == idx))
                        {
                            ChannelList[idx] = (UCHAR)pPort->CommonCfg.TriggerEventTab.EventA[i].Channel;
                            idx++;
                        } 
                        pPort->CommonCfg.TriggerEventTab.EventA[i].bValid = FALSE;
                    }
                    DBGPRINT(RT_DEBUG_ERROR,("ACT - BuildIntolerantChannelRep , Total Channel number = %d \n", idx));
                }
            }

            // idx > 0 means this regulatory class has some channel report and need to copy to the pDest.
            if (idx > 0)
            {
                // For each regaulatory IE report, contains all channels that has the same regulatory class.
                *(pDest + ReadOffset) = IE_2040_BSS_INTOLERANT_REPORT;  // IE
                *(pDest + ReadOffset + 1) = 1+ idx; // Len = RegClass byte + channel byte.
                *(pDest + ReadOffset + 2) = TmpRegClass;    // Len = RegClass byte + channel byte.
                PlatformMoveMemory(pDest + ReadOffset + 3, ChannelList, idx);

                FrameLen += (3 + idx);
                ReadOffset += (3 + idx);
            }
            
        }
    }
    return FrameLen;
}

/*  
    ==========================================================================
    Description: 
    After scan, Update 20/40 BSS Coexistence IE and send out.
    According to 802.11n D3.03 11.14.10
        
    Parameters: 
    ==========================================================================
 */
VOID Update2040CoexistFrameAndNotify(
    IN  PMP_ADAPTER   pAd,
    IN  PMP_PORT  pPort,
    IN    UCHAR  Wcid,
    IN  BOOLEAN bAddIntolerantCha) 
{
    COEXIST_ELEMT_2040  OldValue;

    DBGPRINT(RT_DEBUG_ERROR,("ACT - Update2040CoexistFrameAndNotify. BSSCoexist2040 = %x. EventANo = %d. \n", pPort->CommonCfg.BSSCoexist2040.word, pPort->CommonCfg.TriggerEventTab.EventANo));
    OldValue.word = pPort->CommonCfg.BSSCoexist2040.word;
    // Reset value.
    pPort->CommonCfg.BSSCoexist2040.word = 0;
    
    //
    // 2009 PF#1: 20/40 Coexistence in 2.4G Band: Manual Setting
    //
    if (pPort->CommonCfg.TgnControl.field.CoexMgmt40MHzIntolerant1 == TRUE)       
    {
        // 20/40 BSS coexistence Management frame includes "40MHz intolerant" 
        pPort->CommonCfg.BSSCoexist2040.field.Intolerant40 = 1;
    }
    else if (pPort->CommonCfg.TgnControl.field.CoexMgmt20MHzWidthReq1 == TRUE)    
    {
        // 20/40 BSS coexistence Management frame includes "20MHz BSS Width Request" 
        pPort->CommonCfg.BSSCoexist2040.field.BSS20WidthReq = 1;
    }
    else if (pPort->CommonCfg.TgnControl.field.CoexMgmt40MHzIntolerant0 == TRUE)
    {
        // 20/40 BSS coexistence Management frame includes "40MHz intolerant" 
        pPort->CommonCfg.BSSCoexist2040.field.Intolerant40 = 0;
    }
    else if (pPort->CommonCfg.TgnControl.field.CoexMgmt20MHzWidthReq0 == TRUE)    
    {
        // 20/40 BSS coexistence Management frame includes "20MHz BSS Width Request" 
        pPort->CommonCfg.BSSCoexist2040.field.BSS20WidthReq = 0;
    }   
    else
    {
        if (pPort->CommonCfg.TriggerEventTab.EventBCountDown > 0)
        {
            pPort->CommonCfg.BSSCoexist2040.field.BSS20WidthReq = 1;
        }
    }

    // Need to check !!!!
    // How STA will set Intolerant40 if implementation dependent. Now we don't set this bit first!!!!!
    // So Only check BSS20WidthReq change.
    //if (OldValue.field.BSS20WidthReq != pPort->CommonCfg.BSSCoexist2040.field.BSS20WidthReq)
    {
        Send2040CoexistAction(pAd,pPort, Wcid, bAddIntolerantCha);
    }
}

/*
Description : Send 20/40 BSS Coexistence Action frame If one trigger event is triggered.
*/
VOID Send2040CoexistAction(
    IN  PMP_ADAPTER   pAd,
    IN  PMP_PORT  pPort,
    IN    UCHAR  Wcid,
    IN  BOOLEAN bAddIntolerantCha) 
{
    PUCHAR          pOutBuffer = NULL;
    NDIS_STATUS     NStatus;
    FRAME_ACTION_HDR    Frame;
    ULONG           FrameLen;
    ULONG           IntolerantChaRepLen = 0;
    UCHAR           HtLen = 1;
    PMAC_TABLE_ENTRY pWcidMacTabEntry = NULL;
    pWcidMacTabEntry = MlmeSyncMacTabMatchedWlanIdx(pPort, Wcid);  

    NStatus = MlmeAllocateMemory(pAd, &pOutBuffer);  //Get an unused nonpaged memory
    if(NStatus != NDIS_STATUS_SUCCESS) 
    {
        DBGPRINT(RT_DEBUG_ERROR,("ACT - Send2040CoexistAction() allocate memory failed \n"));
        return;
    }

    if(pWcidMacTabEntry == NULL)
    {
        DBGPRINT(RT_DEBUG_TRACE,("%s(), Line:%d, Mac Entry is NULL  Wcid = %d\n", __FUNCTION__, __LINE__, Wcid));
        return;
    }
    
    ActHeaderInit(pAd, pPort,&Frame.Hdr, pWcidMacTabEntry->Addr, pPort->PortCfg.Bssid);
    Frame.Category = CATEGORY_PUBLIC;
    Frame.Action = COEXIST_2040_ACTION;
    
    MakeOutgoingFrame(pOutBuffer,               &FrameLen,
                  sizeof(FRAME_ACTION_HDR),   &Frame,
                  1,                                &BssCoexistIe,
                  1,                                &HtLen,
                  1,                                &pPort->CommonCfg.BSSCoexist2040.word,
                  END_OF_ARGS);
    
    if (bAddIntolerantCha == TRUE)
        IntolerantChaRepLen = BuildIntolerantChannelRep(pAd, pOutBuffer + FrameLen);

    //2009 PF#3: IOT issue with Motorola AP. It will not check the field of BSSCoexist2040.
    //11.14.12 Switching between 40 MHz and 20 MHz
    if (!((IntolerantChaRepLen == 0) && (pPort->CommonCfg.BSSCoexist2040.word == 0))) 
        NdisCommonMiniportMMRequest(pAd, pOutBuffer, FrameLen + IntolerantChaRepLen);
    else if (pOutBuffer != NULL)
        MlmeFreeMemory(pAd, pOutBuffer);
    DBGPRINT(RT_DEBUG_TRACE,("ACT - Send2040CoexistAction( BSSCoexist2040 = 0x%x )  \n", pPort->CommonCfg.BSSCoexist2040.word));
        
}

VOID UpdateBssScanParm(
    IN  PMP_ADAPTER   pAd,
    IN  OVERLAP_BSS_SCAN_IE APBssScan) 
{
    PMP_PORT    pPort = pAd->PortList[FXXK_PORT_0];
    
    pPort->CommonCfg.ScanParameter.Dot11BssWidthChanTranDelayFactor = APBssScan.DelayFactor[1] * 256 + APBssScan.DelayFactor[0];
    // out of range defined in MIB... So fall back to default value.
    if ((pPort->CommonCfg.ScanParameter.Dot11BssWidthChanTranDelayFactor <5) || (pPort->CommonCfg.ScanParameter.Dot11BssWidthChanTranDelayFactor > 100))
    {
        DBGPRINT(RT_DEBUG_ERROR,("ACT - UpdateBssScanParm( Dot11BssWidthChanTranDelayFactor out of range !!!!)  \n"));
        pPort->CommonCfg.ScanParameter.Dot11BssWidthChanTranDelayFactor = 5;
    }

    pPort->CommonCfg.ScanParameter.Dot11BssWidthTriggerScanInt = APBssScan.TriggerScanInt[1] * 256 + APBssScan.TriggerScanInt[0];
    // out of range defined in MIB... So fall back to default value.
    if ((pPort->CommonCfg.ScanParameter.Dot11BssWidthTriggerScanInt < 10) ||(pPort->CommonCfg.ScanParameter.Dot11BssWidthTriggerScanInt >900))
    {
        DBGPRINT(RT_DEBUG_ERROR,("ACT - UpdateBssScanParm( Dot11BssWidthTriggerScanInt out of range !!!!)  \n"));
        pPort->CommonCfg.ScanParameter.Dot11BssWidthTriggerScanInt = 900;
    }
        
    pPort->CommonCfg.ScanParameter.Dot11OBssScanPassiveDwell = APBssScan.ScanPassiveDwell[1] * 256 + APBssScan.ScanPassiveDwell[0];
    // out of range defined in MIB... So fall back to default value.
    if ((pPort->CommonCfg.ScanParameter.Dot11OBssScanPassiveDwell < 5) ||(pPort->CommonCfg.ScanParameter.Dot11OBssScanPassiveDwell > 1000))
    {
        DBGPRINT(RT_DEBUG_ERROR,("ACT - UpdateBssScanParm( Dot11OBssScanPassiveDwell out of range !!!!)  \n"));
        pPort->CommonCfg.ScanParameter.Dot11OBssScanPassiveDwell = 20;
    }
    
    pPort->CommonCfg.ScanParameter.Dot11OBssScanActiveDwell = APBssScan.ScanActiveDwell[1] * 256 + APBssScan.ScanActiveDwell[0];
    // out of range defined in MIB... So fall back to default value.
    if ((pPort->CommonCfg.ScanParameter.Dot11OBssScanActiveDwell < 10) ||(pPort->CommonCfg.ScanParameter.Dot11OBssScanActiveDwell > 1000))
    {
        DBGPRINT(RT_DEBUG_ERROR,("ACT - UpdateBssScanParm( Dot11OBssScanActiveDwell out of range !!!!)  \n"));
        pPort->CommonCfg.ScanParameter.Dot11OBssScanActiveDwell = 10;
    }
    
    pPort->CommonCfg.ScanParameter.Dot11OBssScanPassiveTotalPerChannel = APBssScan.PassiveTalPerChannel[1] * 256 + APBssScan.PassiveTalPerChannel[0];
    // out of range defined in MIB... So fall back to default value.
    if ((pPort->CommonCfg.ScanParameter.Dot11OBssScanPassiveTotalPerChannel < 200) ||(pPort->CommonCfg.ScanParameter.Dot11OBssScanPassiveTotalPerChannel > 10000))
    {
        DBGPRINT(RT_DEBUG_ERROR,("ACT - UpdateBssScanParm( Dot11OBssScanPassiveTotalPerChannel out of range !!!!)  \n"));
        pPort->CommonCfg.ScanParameter.Dot11OBssScanPassiveTotalPerChannel = 200;
    }
    
    pPort->CommonCfg.ScanParameter.Dot11OBssScanActiveTotalPerChannel = APBssScan.ActiveTalPerChannel[1] * 256 + APBssScan.ActiveTalPerChannel[0];
    // out of range defined in MIB... So fall back to default value.
    if ((pPort->CommonCfg.ScanParameter.Dot11OBssScanActiveTotalPerChannel < 20) ||(pPort->CommonCfg.ScanParameter.Dot11OBssScanActiveTotalPerChannel > 10000))
    {
        DBGPRINT(RT_DEBUG_ERROR,("ACT - UpdateBssScanParm( Dot11OBssScanActiveTotalPerChannel out of range !!!!)  \n"));
        pPort->CommonCfg.ScanParameter.Dot11OBssScanActiveTotalPerChannel = 20;
    }
    
    pPort->CommonCfg.ScanParameter.Dot11OBssScanActivityThre = APBssScan.ScanActThre[1] * 256 + APBssScan.ScanActThre[0];
    // out of range defined in MIB... So fall back to default value.
    if (pPort->CommonCfg.ScanParameter.Dot11OBssScanActivityThre > 100)
    {
        DBGPRINT(RT_DEBUG_ERROR,("ACT - UpdateBssScanParm( Dot11OBssScanActivityThre out of range !!!!)  \n"));
        pPort->CommonCfg.ScanParameter.Dot11OBssScanActivityThre = 25;
    }

    pPort->CommonCfg.ScanParameter.Dot11BssWidthChanTranDelay = (pPort->CommonCfg.ScanParameter.Dot11BssWidthTriggerScanInt * pPort->CommonCfg.ScanParameter.Dot11BssWidthChanTranDelayFactor);
    DBGPRINT(RT_DEBUG_LOUD,("ACT - UpdateBssScanParm( Dot11BssWidthTriggerScanInt = %d )  \n", pPort->CommonCfg.ScanParameter.Dot11BssWidthTriggerScanInt));

}

//
// Send the VHT Operating Mode Notification Action
//
VOID MlmeVhtOperatingModeNotificationAction(
    IN PMP_ADAPTER pAd, 
    IN MLME_QUEUE_ELEM *pElem)
{
    MLME_VHT_OPERATING_MODE_NOTIFICATION_STRUCT *pOperatingModeNotification = NULL;
    PUCHAR pOutBuffer = NULL;
    NDIS_STATUS NdisStatus = NDIS_STATUS_SUCCESS;
    ULONG FrameLength = 0;
    UCHAR Bssid[MAC_ADDR_LEN] = {0};
    PMP_PORT pPort = pAd->PortList[pElem->PortNum];
    ULONG FrameSize = sizeof (HEADER_802_11) + sizeof (OPERATING_MODE_NOTIFICATION_ACTION);
    HEADER_802_11 Dot11Header = {0};
    OPERATING_MODE_NOTIFICATION_ACTION OperatingModeNotificationAction = {0};

    pOperatingModeNotification = (PMLME_VHT_OPERATING_MODE_NOTIFICATION_STRUCT)(pElem->Msg);
    
    NdisStatus = MlmeAllocateMemory(pAd, &pOutBuffer);  //Get an unused nonpaged memory
    if(NdisStatus != NDIS_STATUS_SUCCESS) 
    {
        DBGPRINT(RT_DEBUG_ERROR,("%s: Allocate memory failure\n",__FUNCTION__));
        return;
    }

    DBGPRINT(RT_DEBUG_TRACE, ("%s: -->\n", __FUNCTION__));
    
    COPY_MAC_ADDR(Bssid, pPort->PortCfg.Bssid);
    ActHeaderInit(pAd, pPort, &Dot11Header, pOperatingModeNotification->DA, Bssid);

    OperatingModeNotificationAction.Category = CATEGORY_VHT;
    OperatingModeNotificationAction.Action = VHT_ACTION_OPERATING_MODE_NOTIFICATION;
    OperatingModeNotificationAction.OperatingMode.ChannelWidth = pOperatingModeNotification->ChannelWidth;
    OperatingModeNotificationAction.OperatingMode.RxNss = pOperatingModeNotification->RxNss;
    OperatingModeNotificationAction.OperatingMode.RxNssType= pOperatingModeNotification->RxNssType;

    DBGPRINT(RT_DEBUG_TRACE, ("%s: DA = %02X:%02X:%02X:%02X:%02X:%02X:, Channel Width = %d, Rx Nss = %d, Rx Nss Type = %d\n", 
        __FUNCTION__, 
        pOperatingModeNotification->DA[0], pOperatingModeNotification->DA[1], pOperatingModeNotification->DA[2], 
        pOperatingModeNotification->DA[3], pOperatingModeNotification->DA[4], pOperatingModeNotification->DA[5], 
        pOperatingModeNotification->ChannelWidth, 
        pOperatingModeNotification->RxNss, 
        pOperatingModeNotification->RxNssType));

    MakeOutgoingFrame(pOutBuffer, &FrameLength, 
        sizeof(HEADER_802_11), &Dot11Header, 
        sizeof(OPERATING_MODE_NOTIFICATION_ACTION), &OperatingModeNotificationAction, 
        END_OF_ARGS);
    
    NdisCommonMiniportMMRequest(pAd, pOutBuffer, FrameLength);
    //MlmeFreeMemory(pAd, pOutBuffer);
    
    DBGPRINT(RT_DEBUG_TRACE, ("%s: -->\n", __FUNCTION__));
}

//
// Peer-STA VHT Action
//
VOID PeerVHTAction(
    IN PMP_ADAPTER pAd, 
    IN MLME_QUEUE_ELEM *pElem)
{
    POPERATING_MODE_NOTIFICATION_ACTION pOperatingModeNotification = (POPERATING_MODE_NOTIFICATION_ACTION)(&pElem->Msg[LENGTH_802_11]);
    PMP_PORT pPort = pAd->PortList[pElem->PortNum];

    if ((pElem->Wcid >= MAX_LEN_OF_MAC_TABLE) || 
         (pElem->MsgLen != (LENGTH_802_11 + sizeof(OPERATING_MODE_NOTIFICATION_ACTION))))
    {
        DBGPRINT(RT_DEBUG_ERROR, ("%s: Invalid WCID (%d) or invalid packet length (%d)\n", 
            __FUNCTION__, 
            pElem->Wcid, 
            pElem->MsgLen));
        
        return;
    }

    DBGPRINT(RT_DEBUG_TRACE, ("%s: -->\n", __FUNCTION__));
    
    switch(pOperatingModeNotification->Action)
    {
        case VHT_ACTION_VHT_COMPRESSED_BEAMFORMING: 
        {
            DBGPRINT(RT_DEBUG_TRACE, ("%s: Receive VHT Compressed Beamforming Action\n", __FUNCTION__));
        }
        break;

        case VHT_ACTION_GROUP_ID_MANAGEMENT: 
        {
            DBGPRINT(RT_DEBUG_TRACE, ("%s: Receive VHT Group ID Management Action\n", __FUNCTION__));
        }
        break;

        case VHT_ACTION_OPERATING_MODE_NOTIFICATION: 
        {
            DBGPRINT(RT_DEBUG_TRACE, ("%s: Receive VHT Operating Mode Notification Action\n", __FUNCTION__));

            PeerVHTOperatingModeNotificationAction(pAd, pElem->Wcid, 
                pOperatingModeNotification->OperatingMode.ChannelWidth, 
                pOperatingModeNotification->OperatingMode.RxNss, 
                pOperatingModeNotification->OperatingMode.RxNssType);
        }
        break;

        default: 
        {
            DBGPRINT(RT_DEBUG_ERROR, ("%s: Invalid VHT Action (%d)\n", 
                __FUNCTION__, 
                pOperatingModeNotification->Action));
        }
        break;
    }

    DBGPRINT(RT_DEBUG_TRACE, ("%s: -->\n", __FUNCTION__));
}

//
// Peer-STA VHT Operating Mode Notification Action
//
VOID PeerVHTOperatingModeNotificationAction(
    IN PMP_ADAPTER pAd, 
    IN UCHAR Wcid, // WCID
    IN UCHAR ChannelWidth, // Channel width (CH_WIDTH_BWXXX)
    IN UCHAR RxNss, // Rx Nss (RX_NSS_XXX)
    IN UCHAR RxNssType) // Rx Nss Type (RX_NSS_TYPE_MAX_RX_NSS_XXX)
{
    EEPROM_ANTENNA_STRUC AntCfgNic = {0};
    PMP_PORT    pPort = pAd->PortList[FXXK_PORT_0];
    PMAC_TABLE_ENTRY pWcidMacTabEntry = NULL;
    pWcidMacTabEntry = MlmeSyncMacTabMatchedWlanIdx(pPort, Wcid);  

    AntCfgNic.word = pAd->HwCfg.EEPROMDefaultValue[EEPROM_NIC_CFG1_OFFSET];

    if(pWcidMacTabEntry == NULL)
    {
        DBGPRINT(RT_DEBUG_TRACE,("%s(), Line:%d, Mac Entry is NULL  Wcid = %d\n", __FUNCTION__, __LINE__, Wcid));
        return;
    }
    
    if (RxNssType == RX_NSS_TYPE_MAX_RX_NSS_FOR_TXBF)
    {
        DBGPRINT(RT_DEBUG_ERROR, ("%s: Not support RX_NSS_TYPE_MAX_RX_NSS_FOR_TXBF\n", __FUNCTION__));

        return;
    }

    if (ChannelWidth == CH_WIDTH_BW160_OR_BW80_80)
    {
        DBGPRINT(RT_DEBUG_ERROR, ("%s: Not support CH_WIDTH_BW160_OR_BW80_80\n", __FUNCTION__));

        return;
    }   

    if (((AntCfgNic.field.TxPath == 1) && (RxNss > RX_NSS_1)) || 
        ((AntCfgNic.field.TxPath == 2) && (RxNss > RX_NSS_2)))
    {
        DBGPRINT(RT_DEBUG_ERROR, ("%s: Not support RxNss (TxPath = %d, RxNss = %d)\n", 
            __FUNCTION__, 
            pAd->HwCfg.Antenna.field.TxPath, 
            RxNss));

        return;
    }
    
    DBGPRINT(RT_DEBUG_TRACE, ("%s: -->\n", __FUNCTION__));

    //
    // Rate table selection
    //
    pAd->HwCfg.Antenna.field.TxPath = (RxNss + 1); // Zero-based index of RxNss
    pAd->HwCfg.NicConfig3.field.TxStream = (RxNss + 1); // Zero-based index of RxNss

    //
    // Per-packet BW/Nss configurations
    //
    WRITE_PHY_CFG_BW(pAd, &pWcidMacTabEntry->TxPhyCfg, ChannelWidth);
    WRITE_PHY_CFG_NSS(pAd, &pWcidMacTabEntry->TxPhyCfg, RxNss);
    pWcidMacTabEntry->VhtPeerStaCtrl.MaxBW = ChannelWidth;

    DBGPRINT(RT_DEBUG_TRACE, ("%s: Update desired channel width (%d) and Rx Nss (%d) on Wcid (%d); AntCfgNic.field.TxPath = %d, TxStream = %d\n", 
        __FUNCTION__, 
        ChannelWidth, 
        RxNss, 
        Wcid, 
        AntCfgNic.field.TxPath, 
        pAd->HwCfg.NicConfig3.field.TxStream));

    DBGPRINT(RT_DEBUG_TRACE, ("%s: -->\n", __FUNCTION__));
}
