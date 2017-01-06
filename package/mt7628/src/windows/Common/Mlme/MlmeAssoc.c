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
    assoc.c

    Abstract:

    Revision History:
    Who         When            What
    --------    ----------      ----------------------------------------------
    John        2004-9-3        porting from RT2500
*/
#include "MtConfig.h"

#define DRIVER_FILE         0x00900000

UCHAR   CipherWpaTemplate[] = {
        0xdd,                   // WPA IE
        0x16,                   // Length
        0x00, 0x50, 0xf2, 0x01, // oui
        0x01, 0x00,             // Version
        0x00, 0x50, 0xf2, 0x02, // Multicast
        0x01, 0x00,             // Number of unicast
        0x00, 0x50, 0xf2, 0x02, // unicast
        0x01, 0x00,             // number of authentication method
        0x00, 0x50, 0xf2, 0x01  // authentication
        };

UCHAR   CipherWpaTemplateLen = sizeof(CipherWpaTemplate)/sizeof(UCHAR);

UCHAR   CipherWpa2Template[] = {
        0x30,                   // RSN IE
        0x14,                   // Length   
        0x01, 0x00,             // Version
        0x00, 0x0f, 0xac, 0x02, // group cipher, TKIP
        0x01, 0x00,             // number of pairwise
        0x00, 0x0f, 0xac, 0x02, // unicast
        0x01, 0x00,             // number of authentication method
        0x00, 0x0f, 0xac, 0x02, // authentication
        0x00, 0x00,             // RSN capability
        };

UCHAR   CipherWpa2TemplateLen = sizeof(CipherWpa2Template)/sizeof(UCHAR);

// suppport CCXv5
UCHAR   CcxVerIE[] = { 0x00, 0x40, 0x96, 0x03, 0x05};
/*  
    ==========================================================================
    Description: 
        association state machine init, including state transition and timer init
    Parameters: 
        S - pointer to the association state machine

    IRQL = PASSIVE_LEVEL
    
    ==========================================================================
 */
VOID MlmeAssocStateMachineInit(
    IN  PMP_ADAPTER       pAd, 
    IN  STATE_MACHINE       *S, 
    OUT STATE_MACHINE_FUNC  Trans[]) 
{
    StateMachineInit(S, Trans, MAX_ASSOC_STATE, MAX_ASSOC_MSG, Drop, ASSOC_IDLE, ASSOC_MACHINE_BASE);

    // first column
    StateMachineSetAction(S, ASSOC_IDLE, MT2_MLME_ASSOC_REQ, MlmeAssocReqAction);
    // For Vista, now we use MlmeAssocReqAction for both MT2_MLME_ASSOC_REQ and MT2_MLME_REASSOC_REQ
    StateMachineSetAction(S, ASSOC_IDLE, MT2_MLME_REASSOC_REQ, MlmeAssocReqAction);
    StateMachineSetAction(S, ASSOC_IDLE, MT2_MLME_DISASSOC_REQ, MlmeDisassocReqAction);
    StateMachineSetAction(S, ASSOC_IDLE, MT2_PEER_DISASSOC_REQ, MlmeAssocPeerDisassocAction);

    // second column
    StateMachineSetAction(S, ASSOC_WAIT_RSP, MT2_MLME_ASSOC_REQ, MlmeAssocInvalidStateWhenAssoc);
    StateMachineSetAction(S, ASSOC_WAIT_RSP, MT2_MLME_REASSOC_REQ, MlmeAssocInvalidStateWhenReassoc);
    StateMachineSetAction(S, ASSOC_WAIT_RSP, MT2_MLME_DISASSOC_REQ, MlmeAssocInvalidStateWhenDisassociate);
    StateMachineSetAction(S, ASSOC_WAIT_RSP, MT2_PEER_DISASSOC_REQ, MlmeAssocPeerDisassocAction);
    StateMachineSetAction(S, ASSOC_WAIT_RSP, MT2_PEER_ASSOC_RSP, MlmeAssocPeerAssocRspAction);
    //
    // Patch 3Com AP MOde:3CRWE454G72
    // We send Assoc request frame to this AP, it always send Reassoc Rsp not Associate Rsp.
    //
    StateMachineSetAction(S, ASSOC_WAIT_RSP, MT2_PEER_REASSOC_RSP, MlmeAssocPeerAssocRspAction);
    StateMachineSetAction(S, ASSOC_WAIT_RSP, MT2_ASSOC_TIMEOUT, MlmeAssocTimeoutTimerCallbackAction);

    // third column
    StateMachineSetAction(S, REASSOC_WAIT_RSP, MT2_MLME_ASSOC_REQ, MlmeAssocInvalidStateWhenAssoc);
    StateMachineSetAction(S, REASSOC_WAIT_RSP, MT2_MLME_REASSOC_REQ, MlmeAssocInvalidStateWhenReassoc);
    StateMachineSetAction(S, REASSOC_WAIT_RSP, MT2_MLME_DISASSOC_REQ, MlmeAssocInvalidStateWhenDisassociate);
    StateMachineSetAction(S, REASSOC_WAIT_RSP, MT2_PEER_DISASSOC_REQ, MlmeAssocPeerDisassocAction);
    StateMachineSetAction(S, REASSOC_WAIT_RSP, MT2_PEER_REASSOC_RSP, MlmeAssocPeerReassocRspAction);
    //
    // Patch, AP doesn't send Reassociate Rsp frame to Station.
    //
    StateMachineSetAction(S, REASSOC_WAIT_RSP, MT2_PEER_ASSOC_RSP, MlmeAssocPeerReassocRspAction);   
    StateMachineSetAction(S, REASSOC_WAIT_RSP, MT2_REASSOC_TIMEOUT, MlmeAssocReAssocTimeoutTimerCallbackAction);

    // fourth column
    StateMachineSetAction(S, DISASSOC_WAIT_RSP, MT2_MLME_ASSOC_REQ, MlmeAssocInvalidStateWhenAssoc);
    StateMachineSetAction(S, DISASSOC_WAIT_RSP, MT2_MLME_REASSOC_REQ, MlmeAssocInvalidStateWhenReassoc);
    StateMachineSetAction(S, DISASSOC_WAIT_RSP, MT2_MLME_DISASSOC_REQ, MlmeAssocInvalidStateWhenDisassociate);
    StateMachineSetAction(S, DISASSOC_WAIT_RSP, MT2_PEER_DISASSOC_REQ, MlmeAssocPeerDisassocAction);
    StateMachineSetAction(S, DISASSOC_WAIT_RSP, MT2_DISASSOC_TIMEOUT, MlmeAssocDisAssocTimeoutTimerCallbackAction);
}

/*
    ==========================================================================
    Description:
        Association timeout procedure. After association timeout, this function 
        will be called and it will put a message into the MLME queue
    Parameters:
        Standard timer parameters

    IRQL = DISPATCH_LEVEL

    ==========================================================================
 */
VOID MlmeAssocTimeoutTimerCallback(
    IN  PVOID   SystemSpecific1, 
    IN  PVOID   FunctionContext, 
    IN  PVOID   SystemSpecific2, 
    IN  PVOID   SystemSpecific3) 
{
    PMP_PORT pPort = (PMP_PORT )FunctionContext;
    PMP_ADAPTER  pAd = pPort->pAd;
    MLME_ASSOC_REQ_STRUCT   AssocReq;



    // Do nothing if the driver is starting halt state.
    // This might happen when timer already been fired before cancel timer with mlmehalt
    if (MT_TEST_FLAG(pAd, fRTMP_ADAPTER_HALT_IN_PROGRESS))
        return;

    if (pPort == NULL)
        return;

    if (pAd->MlmeAux.AssocRetryCount < 2)
    {
        pAd->MlmeAux.AssocState = dot11_assoc_state_auth_unassoc;
        pAd->MlmeAux.AssocRetryCount ++;
        pPort->Mlme.AssocMachine.CurrState = ASSOC_IDLE;
        MlmeCntAssocParmFill(pAd, &AssocReq, pAd->MlmeAux.Bssid, pAd->MlmeAux.CapabilityInfo, 
                  ASSOC_TIMEOUT, pAd->StaCfg.DefaultListenCount);

        MlmeEnqueue(pAd,pPort, ASSOC_STATE_MACHINE, MT2_MLME_ASSOC_REQ, 
                    sizeof(MLME_ASSOC_REQ_STRUCT), &AssocReq);

        pPort->Mlme.CntlMachine.CurrState = CNTL_WAIT_ASSOC;
        DBGPRINT(RT_DEBUG_TRACE,("ASSOC - retry %d time \n", pAd->MlmeAux.AssocRetryCount + 1));
    }
    else
    {
        DBGPRINT(RT_DEBUG_TRACE,("ASSOC - enqueue MT2_ASSOC_TIMEOUT (PortNum=%d) \n", pPort->PortNumber));
        
        MlmeEnqueue(pAd,pPort, ASSOC_STATE_MACHINE, MT2_ASSOC_TIMEOUT, 0, NULL);
    }
}

/*
    ==========================================================================
    Description:
        Reassociation timeout procedure. After reassociation timeout, this 
        function will be called and put a message into the MLME queue
    Parameters:
        Standard timer parameters

    IRQL = DISPATCH_LEVEL

    ==========================================================================
 */
VOID MlmeAssocReAssocTimeoutTimerCallback(
    IN  PVOID   SystemSpecific1, 
    IN  PVOID   FunctionContext, 
    IN  PVOID   SystemSpecific2, 
    IN  PVOID   SystemSpecific3) 
{
    PMP_PORT pPort = (PMP_PORT )FunctionContext;
    PMP_ADAPTER  pAd = pPort->pAd;
    DBGPRINT(RT_DEBUG_INFO,("ASSOC - enqueue MT2_REASSOC_TIMEOUT \n"));
    
    // Do nothing if the driver is starting halt state.
    // This might happen when timer already been fired before cancel timer with mlmehalt
    if (MT_TEST_FLAG(pAd, fRTMP_ADAPTER_HALT_IN_PROGRESS))
        return;
    
    MlmeEnqueue(pAd,pPort, ASSOC_STATE_MACHINE, MT2_REASSOC_TIMEOUT, 0, NULL);
}

VOID
MlmeAssocConnectionTimeoutTimerCallback(
    IN  PVOID   SystemSpecific1, 
    IN  PVOID   FunctionContext, 
    IN  PVOID   SystemSpecific2, 
    IN  PVOID   SystemSpecific3
    ) 
{
    PMP_PORT pPort = (PMP_PORT )FunctionContext;
    PMP_ADAPTER  pAd = pPort->pAd;

    FUNC_ENTER;
    if(pAd->bConnectionInProgress == TRUE)
    {
        PlatformIndicateConnectionCompletion(pAd, pPort, DOT11_CONNECTION_STATUS_CANDIDATE_LIST_EXHAUSTED);
    }
    FUNC_LEAVE;
}


/*
    ==========================================================================
    Description:
        Disassociation timeout procedure. After disassociation timeout, this 
        function will be called and put a message into the MLME queue
    Parameters:
        Standard timer parameters

    IRQL = DISPATCH_LEVEL

    ==========================================================================
 */
VOID MlmeAssocDisAssocTimeoutTimerCallback(
    IN  PVOID   SystemSpecific1, 
    IN  PVOID   FunctionContext, 
    IN  PVOID   SystemSpecific2, 
    IN  PVOID   SystemSpecific3) 
{
    PMP_PORT pPort = (PMP_PORT )FunctionContext;
    PMP_ADAPTER  pAd = pPort->pAd;
    DBGPRINT(RT_DEBUG_INFO,("ASSOC - enqueue MT2_DISASSOC_TIMEOUT \n"));

    // for p2p enrollee wps retry after receive m2d
    if ((pAd->PortList[pPort->P2PCfg.PortNumber] != NULL) &&
        (IS_P2P_ENROLLEE(pPort)) && 
        (IS_P2P_CON_CLI(pAd, pAd->PortList[pPort->P2PCfg.PortNumber])) &&
        (INFRA_ON(pAd->PortList[pPort->P2PCfg.PortNumber])))
    {
        pPort = (PMP_PORT)pAd->PortList[pPort->P2PCfg.PortNumber];
        DBGPRINT(RT_DEBUG_TRACE, ("change to virtual#2 (P2PCfg.PortNumber = %d) \n", pPort->P2PCfg.PortNumber));
        DBGPRINT(RT_DEBUG_TRACE, ("pPort %d %d ", pPort->PortNumber, pPort->Mlme.AssocMachine.CurrState));
    }
    // Do nothing if the driver is starting halt state.
    // This might happen when timer already been fired before cancel timer with mlmehalt
    if (MT_TEST_FLAG(pAd, fRTMP_ADAPTER_HALT_IN_PROGRESS))
        return;

    MlmeEnqueue(pAd,pPort, ASSOC_STATE_MACHINE, MT2_DISASSOC_TIMEOUT, 0, NULL);
}

/*
    ==========================================================================
    Description:
        mlme assoc req handling procedure
    Parameters:
        Adapter - Adapter pointer
        Elem - MLME Queue Element
    Pre:
        the station has been authenticated and the following information is stored in the config
            -# SSID
            -# supported rates and their length
            -# listen interval (Adapter->StaCfg.default_listen_count)
            -# Transmit power  (Adapter->StaCfg.tx_power)
    Post  :
        -# An association request frame is generated and sent to the air
        -# Association timer starts
        -# Association state -> ASSOC_WAIT_RSP

    IRQL = DISPATCH_LEVEL

    ==========================================================================
 */
VOID MlmeAssocReqAction(
    IN  PMP_ADAPTER       pAd,    
    IN  MLME_QUEUE_ELEM     *Elem) 
{
    UCHAR           ApAddr[6];
    USHORT          ListenIntv;
    ULONG           Timeout;
    USHORT          CapabilityInfo;
    BOOLEAN         TimerCancelled = FALSE;
    PUCHAR          pOutBuffer = NULL;
    NDIS_STATUS     NStatus;
    ULONG           FrameLen = 0;
       
    USHORT          VarIesOffset = 0;   
    

    USHORT          Status;

    PMP_PORT      pPort = pAd->PortList[Elem->PortNum];
    

    // check sanity first
    if (MlmeAssocReqSanity(pAd, Elem->Msg, Elem->MsgLen, ApAddr, &CapabilityInfo, &Timeout, &ListenIntv)) 
    {
        // Block all authentication request durning WPA block period
        if( (pAd->StaCfg.bBlockAssoc == TRUE) && MAC_ADDR_EQUAL(ApAddr,pPort->PortCfg.LastBssid))               
        {
            DBGPRINT(RT_DEBUG_TRACE, ("ASSOC - Block Assoc request durning WPA block period!\n"));
            pPort->Mlme.AssocMachine.CurrState = ASSOC_IDLE;
            Status = MLME_STATE_MACHINE_REJECT;
            MlmeEnqueue(pAd,pPort, MLME_CNTL_STATE_MACHINE, MT2_ASSOC_CONF, 2, &Status);
            return;
        }   
    
        DBGPRINT(RT_DEBUG_TRACE, ("MFP: AP RSNCAP 0x%x\n", pPort->PortCfg.RsnCapability));
        if((((pPort->PortCfg.RsnCapability & 0xc0) == 0x40) && (pAd->StaCfg.PmfCfg.MFPC == 1)) || //invailed capabilies of AP
           (((pPort->PortCfg.RsnCapability & 0xc0) == 0x00) && (pAd->StaCfg.PmfCfg.RSNCapability == 0xC0))) // STA demand MFP required
        {
            MLME_DISASSOC_REQ_STRUCT    DisassocReq;
            DBGPRINT(RT_DEBUG_ERROR, ("MFP: ASSOC - Robust Management frame policy violation!\n"));
            pPort->Mlme.AssocMachine.CurrState = ASSOC_IDLE;
            MlmeCntDisassocParmFill(pAd, &DisassocReq, ApAddr, REASON_RMF_POLICY_VIOLATION);
            MlmeEnqueue(pAd, pPort, ASSOC_STATE_MACHINE, MT2_MLME_DISASSOC_REQ, 
                sizeof(MLME_DISASSOC_REQ_STRUCT), &DisassocReq);

            Status = MLME_RMF_POLICY_VIOLATION;
            MlmeEnqueue(pAd, pPort, MLME_CNTL_STATE_MACHINE, MT2_DISASSOC_CONF, 2, &Status);
            return;
        }
        //patch Windows Auto Config for temp
        else if(((pPort->PortCfg.RsnCapability & 0xc0) == 0x00) && (pAd->StaCfg.PmfCfg.MFPC == 1))
        {
            pAd->StaCfg.PmfCfg.MFPC = 0;
            pAd->StaCfg.PmfCfg.MFPR = 0;
        }
    
        PlatformCancelTimer(&pPort->Mlme.AssocTimer, &TimerCancelled);
        COPY_MAC_ADDR(pAd->MlmeAux.Bssid, ApAddr);

        NStatus = MlmeAllocateMemory(pAd, &pOutBuffer);  //Get an unused nonpaged memory
        if (NStatus != NDIS_STATUS_SUCCESS) 
        {
            DBGPRINT(RT_DEBUG_TRACE,("ASSOC - MlmeAssocReqAction() allocate memory failed \n"));
            pPort->Mlme.AssocMachine.CurrState = ASSOC_IDLE;
            Status = MLME_FAIL_NO_RESOURCE;
            MlmeEnqueue(pAd,pPort, MLME_CNTL_STATE_MACHINE, MT2_ASSOC_CONF, 2, &Status);
            return;
        }

        // Add by James 03/06/27
        pAd->StaCfg.AssocInfo.Length = sizeof(NDIS_802_11_ASSOCIATION_INFORMATION); 
        // Association don't need to report MAC address
        pAd->StaCfg.AssocInfo.AvailableRequestFixedIEs =
            NDIS_802_11_AI_REQFI_CAPABILITIES | NDIS_802_11_AI_REQFI_LISTENINTERVAL;

        pAd->StaCfg.AssocInfo.RequestFixedIEs.Capabilities = CapabilityInfo;
        pAd->StaCfg.AssocInfo.RequestFixedIEs.ListenInterval = ListenIntv;      
        // Only reassociate need this
        //COPY_MAC_ADDR(pAd->StaCfg.AssocInfo.RequestFixedIEs.CurrentAPAddress, ApAddr);
        pAd->StaCfg.AssocInfo.OffsetRequestIEs = sizeof(NDIS_802_11_ASSOCIATION_INFORMATION);
        
        // First add SSID
        PlatformMoveMemory(pAd->StaCfg.ReqVarIEs + VarIesOffset, &SsidIe, 1);
        VarIesOffset += 1;
        PlatformMoveMemory(pAd->StaCfg.ReqVarIEs + VarIesOffset, &pAd->MlmeAux.SsidLen, 1);
        VarIesOffset += 1;
        PlatformMoveMemory(pAd->StaCfg.ReqVarIEs + VarIesOffset, pAd->MlmeAux.Ssid, pAd->MlmeAux.SsidLen);
        VarIesOffset += pAd->MlmeAux.SsidLen;

        // Second add Supported rates
        PlatformMoveMemory(pAd->StaCfg.ReqVarIEs + VarIesOffset, &SupRateIe, 1);
        VarIesOffset += 1;
        PlatformMoveMemory(pAd->StaCfg.ReqVarIEs + VarIesOffset, &pAd->MlmeAux.SupRateLen, 1);
        VarIesOffset += 1;
        PlatformMoveMemory(pAd->StaCfg.ReqVarIEs + VarIesOffset, pAd->MlmeAux.SupRate, pAd->MlmeAux.SupRateLen);
        VarIesOffset += pAd->MlmeAux.SupRateLen;
        // End Add by James

        DBGPRINT(RT_DEBUG_TRACE, ("\n ASSOC - Send ASSOC request..., PortNum = %d \n\n", Elem->PortNum));
        // CCKM - Reassociation
        PlatformCancelTimer(&pPort->Mlme.AssocTimer, &TimerCancelled);

        MgntPktConstructAsscReqFrame(pAd, 
                                            pPort, 
                                            FALSE, 
                                            CapabilityInfo, 
                                            ApAddr, 
                                            ListenIntv, 
                                            VarIesOffset,
                                            pOutBuffer, 
                                            &FrameLen);

 
        NdisCommonMiniportMMRequest(pAd,pOutBuffer, FrameLen);
        PlatformSetTimer(pPort, &pPort->Mlme.AssocTimer, Timeout);
        pPort->Mlme.AssocMachine.CurrState = ASSOC_WAIT_RSP;
        DBGPRINT(RT_DEBUG_TRACE, ("MlmeAssocReqAction, FrameLen=%d, pAd->StaCfg.ReqVarIELen=%d , pAd->StaCfg.AssocInfo.RequestIELength =%d !!!!! \n", FrameLen, pAd->StaCfg.ReqVarIELen, pAd->StaCfg.AssocInfo.RequestIELength ));

        //This indication is not for IBSS...
        if ((pAd->StaCfg.BssType == BSS_INFRA) && (pAd->MlmeAux.AssocRetryCount == 0))
        {
            DBGPRINT(RT_DEBUG_TRACE, ("ASSOC - pAd->MlmeAux.BssIdx = %d\n", pAd->MlmeAux.BssIdx));
            // Indicate Assoc Info with the desired Beacon
            COPY_MAC_ADDR(pAd->MlmeAux.Bssid, (pAd->MlmeAux.SsidBssTab.BssEntry[pAd->MlmeAux.BssIdx].Bssid));
            PlatformIndicateAssociationStart(pAd,pPort, &(pAd->MlmeAux.SsidBssTab.BssEntry[pAd->MlmeAux.BssIdx]));
        }
    } 
    else 
    {
        DBGPRINT(RT_DEBUG_TRACE,("ASSOC - MlmeAssocReqAction() sanity check failed. BUG!!!!!! \n"));
        pPort->Mlme.AssocMachine.CurrState = ASSOC_IDLE;
        Status = MLME_INVALID_FORMAT;
        MlmeEnqueue(pAd,pPort, MLME_CNTL_STATE_MACHINE, MT2_ASSOC_CONF, 2, &Status);
    }
}

/*
    ==========================================================================
    Description:
        Upper layer issues disassoc request
    Parameters:
        Elem -

    IRQL = PASSIVE_LEVEL

    ==========================================================================
 */
VOID MlmeDisassocReqAction(
    IN PMP_ADAPTER pAd, 
    IN MLME_QUEUE_ELEM *Elem) 
{
    PMLME_DISASSOC_REQ_STRUCT   pDisassocReq;
    HEADER_802_11               DisassocHdr;
    PCHAR                       pOutBuffer = NULL;
    PCHAR                       pOutBuffer2 = NULL;
    ULONG                       FrameLen = 0;
    NDIS_STATUS                 NStatus;
    BOOLEAN                     TimerCancelled;
    ULONG                       Timeout = 0;
    USHORT                      Status;
    PMP_PORT                  pPort = pAd->PortList[Elem->PortNum];
    // send DLS-TEAR_DOWN message, 
    if (DLS_ON(pAd))
    {
        DlsTearDown(pAd, pPort);
    }

    if (TDLS_ON(pAd))
    {
        TdlsTearDown(pAd, FALSE);
    }

    // skip sanity check
    pDisassocReq = (PMLME_DISASSOC_REQ_STRUCT)(Elem->Msg);

    NStatus = MlmeAllocateMemory(pAd, &pOutBuffer);  //Get an unused nonpaged memory
    if (NStatus != NDIS_STATUS_SUCCESS) 
    {
        DBGPRINT(RT_DEBUG_TRACE, ("ASSOC - MlmeDisassocReqAction() allocate memory failed\n"));
        pPort->Mlme.AssocMachine.CurrState = ASSOC_IDLE;
        Status = MLME_FAIL_NO_RESOURCE;
        MlmeEnqueue(pAd,pPort, MLME_CNTL_STATE_MACHINE, MT2_DISASSOC_CONF, 2, &Status);
        return;
    }

    NStatus = MlmeAllocateMemory(pAd, &pOutBuffer2);  //Get an unused nonpaged memory
    if (NStatus != NDIS_STATUS_SUCCESS) 
    {
        DBGPRINT(RT_DEBUG_TRACE, ("ASSOC - MlmeDisassocReqAction() allocate memory failed\n"));
        pPort->Mlme.AssocMachine.CurrState = ASSOC_IDLE;
        Status = MLME_FAIL_NO_RESOURCE;
        MlmeFreeMemory(pAd, pOutBuffer);
        MlmeEnqueue(pAd,pPort, MLME_CNTL_STATE_MACHINE, MT2_DISASSOC_CONF, 2, &Status);
        return;
    }
    PlatformCancelTimer(&pPort->Mlme.DisassocTimer, &TimerCancelled);

    DBGPRINT(RT_DEBUG_TRACE, ("ASSOC - Send DISASSOC request[BSSID::%02x:%02x:%02x:%02x:%02x:%02x] (Reason=%d) (PortNum=%d)\n", 
                pDisassocReq->Addr[0], pDisassocReq->Addr[1], pDisassocReq->Addr[2],
                pDisassocReq->Addr[3], pDisassocReq->Addr[4], pDisassocReq->Addr[5], pDisassocReq->Reason, Elem->PortNum));
    MgtMacHeaderInit(pAd,pPort, &DisassocHdr, SUBTYPE_DISASSOC, 0, pDisassocReq->Addr, pDisassocReq->Addr);
    MakeOutgoingFrame(pOutBuffer,           &FrameLen, 
                    sizeof(HEADER_802_11),&DisassocHdr, 
                    2,                    &pDisassocReq->Reason, 
                    END_OF_ARGS);
    NdisCommonMiniportMMRequest(pAd, pOutBuffer, FrameLen);

    // To patch Instance and Buffalo(N) AP
    // Driver has to send deauth to Instance AP, but Buffalo(N) needs to send disassoc to reset Authenticator's state machine
    // Therefore, we send both of them.
    DisassocHdr.FC.SubType = SUBTYPE_DEAUTH;
    MakeOutgoingFrame(pOutBuffer2,           &FrameLen, 
                    sizeof(HEADER_802_11),&DisassocHdr, 
                    2,                    &pDisassocReq->Reason, 
                    END_OF_ARGS);
    NdisCommonMiniportMMRequest(pAd, pOutBuffer2, FrameLen);
    
    // Set the control aux SSID to prevent it reconnect to old SSID
    // Since calling this indicate user don't want to connect to that SSID anymore.
    // 2004-11-10 can't reset this info, cause it may be the new SSID that user requests for
    // pAd->MlmeAux.SsidLen = MAX_LEN_OF_SSID;
    // PlatformZeroMemory(pAd->MlmeAux.Ssid, MAX_LEN_OF_SSID);
    // PlatformZeroMemory(pAd->MlmeAux.Bssid, MAC_ADDR_LEN);

    pAd->StaCfg.DisassocReason = REASON_DEAUTH_STA_LEAVING;
    COPY_MAC_ADDR(pAd->StaCfg.DisassocSta, pDisassocReq->Addr);

    pPort->Mlme.AssocMachine.CurrState = DISASSOC_WAIT_RSP;
    PlatformSetTimer(pPort, &pPort->Mlme.DisassocTimer, Timeout); /* in mSec */
}

/*
    ==========================================================================
    Description:
        Upper layer issues disassoc request
    Parameters:
        Elem -

    IRQL = PASSIVE_LEVEL

    ==========================================================================
 */
VOID MlmeDisassocReqActionforPowerSave(
    IN PMP_ADAPTER pAd, 
    IN MLME_QUEUE_ELEM *Elem) 
{
    PMLME_DISASSOC_REQ_STRUCT   pDisassocReq;
    HEADER_802_11               DisassocHdr;
    PCHAR                       pOutBuffer = NULL;
    PCHAR                       pOutBuffer2 = NULL;
    ULONG                       FrameLen = 0;
    NDIS_STATUS                 NStatus;
    BOOLEAN                     TimerCancelled = FALSE;
    USHORT                      Status;
    PMP_PORT                  pPort = pAd->PortList[Elem->PortNum];
        
    // send DLS-TEAR_DOWN message, 
    if (DLS_ON(pAd))
    {
        DlsTearDown(pAd, pPort);
    }

    if (TDLS_ON(pAd))
    {
        TdlsTearDown(pAd, FALSE);
    }

    // skip sanity check
    pDisassocReq = (PMLME_DISASSOC_REQ_STRUCT)(Elem->Msg);

    NStatus = MlmeAllocateMemory(pAd, &pOutBuffer);  //Get an unused nonpaged memory
    if (NStatus != NDIS_STATUS_SUCCESS) 
    {
        DBGPRINT(RT_DEBUG_TRACE, ("ASSOC - MlmeDisassocReqActionforPowerSave() allocate memory failed\n"));
        pPort->Mlme.AssocMachine.CurrState = ASSOC_IDLE;
        Status = MLME_FAIL_NO_RESOURCE;
        MlmeEnqueue(pAd,pPort, MLME_CNTL_STATE_MACHINE, MT2_DISASSOC_CONF, 2, &Status);
        return;
    }

    NStatus = MlmeAllocateMemory(pAd, &pOutBuffer2);  //Get an unused nonpaged memory
    if (NStatus != NDIS_STATUS_SUCCESS) 
    {
        DBGPRINT(RT_DEBUG_TRACE, ("ASSOC - MlmeDisassocReqActionforPowerSave() allocate memory failed\n"));
        pPort->Mlme.AssocMachine.CurrState = ASSOC_IDLE;
        Status = MLME_FAIL_NO_RESOURCE;
        MlmeFreeMemory(pAd, pOutBuffer);
        MlmeEnqueue(pAd,pPort, MLME_CNTL_STATE_MACHINE, MT2_DISASSOC_CONF, 2, &Status);
        return;
    }
    PlatformCancelTimer(&pPort->Mlme.DisassocTimer, &TimerCancelled);

    DBGPRINT(RT_DEBUG_TRACE, ("ASSOC - Send DISASSOC request[BSSID::%02x:%02x:%02x:%02x:%02x:%02x\n", 
                pDisassocReq->Addr[0], pDisassocReq->Addr[1], pDisassocReq->Addr[2],
                pDisassocReq->Addr[3], pDisassocReq->Addr[4], pDisassocReq->Addr[5]));
    MgtMacHeaderInit(pAd,pPort, &DisassocHdr, SUBTYPE_DISASSOC, 0, pDisassocReq->Addr, pDisassocReq->Addr);
    MakeOutgoingFrame(pOutBuffer,           &FrameLen, 
                    sizeof(HEADER_802_11),&DisassocHdr, 
                    2,                    &pDisassocReq->Reason, 
                    END_OF_ARGS);
    NdisCommonMiniportMMRequest(pAd, pOutBuffer, FrameLen);

    // To patch Instance and Buffalo(N) AP
    // Driver has to send deauth to Instance AP, but Buffalo(N) needs to send disassoc to reset Authenticator's state machine
    // Therefore, we send both of them.
    DisassocHdr.FC.SubType = SUBTYPE_DEAUTH;
    MakeOutgoingFrame(pOutBuffer2,           &FrameLen, 
                    sizeof(HEADER_802_11),&DisassocHdr, 
                    2,                    &pDisassocReq->Reason, 
                    END_OF_ARGS);
    NdisCommonMiniportMMRequest(pAd, pOutBuffer2, FrameLen);
    
    // Set the control aux SSID to prevent it reconnect to old SSID
    // Since calling this indicate user don't want to connect to that SSID anymore.
    // 2004-11-10 can't reset this info, cause it may be the new SSID that user requests for
    // pAd->MlmeAux.SsidLen = MAX_LEN_OF_SSID;
    // PlatformZeroMemory(pAd->MlmeAux.Ssid, MAX_LEN_OF_SSID);
    // PlatformZeroMemory(pAd->MlmeAux.Bssid, MAC_ADDR_LEN);

    pAd->StaCfg.DisassocReason = REASON_DEAUTH_STA_LEAVING;
    COPY_MAC_ADDR(pAd->StaCfg.DisassocSta, pDisassocReq->Addr);
}

/*
    ==========================================================================
    Description:
        peer sends assoc rsp back
    Parameters:
        Elme - MLME message containing the received frame

    IRQL = DISPATCH_LEVEL

    ==========================================================================
 */
VOID MlmeAssocPeerAssocRspAction(
    IN PMP_ADAPTER pAd, 
    IN MLME_QUEUE_ELEM *Elem) 
{
    USHORT        CapabilityInfo, Status, Aid;
    UCHAR         SupRate[MAX_LEN_OF_SUPPORTED_RATES], SupRateLen;
    UCHAR         ExtRate[MAX_LEN_OF_SUPPORTED_RATES], ExtRateLen;
    UCHAR         Addr2[MAC_ADDR_LEN];
    BOOLEAN       TimerCancelled;
    EDCA_PARM     EdcaParm;
    HT_CAPABILITY_IE        HtCapability;
    ADD_HT_INFO_IE      AddHtInfo;  // AP might use this additional ht info IE 
    UCHAR           HtCapabilityLen;
    UCHAR           AddHtInfoLen;
    UCHAR           NewExtChannelOffset = 0xff;
    //MAC_DW0_STRUC     StaMacReg0;
    //MAC_DW1_STRUC     StaMacReg1;
    PMP_PORT      pPort = pAd->PortList[Elem->PortNum];
    ULONG           P2PSubelementLen = 0;

    // for VHT
    BOOLEAN            bVhtCapable=FALSE;
    VHT_CAP_IE         VhtCapability;
    VHT_OP_IE          VhtOperation;
    EXT_CAP_ELEMT      ExtendedCapabilities = {0};

    if (PeerAssocRspSanity(pAd,
                            pPort,
                            Elem->Msg,
                            Elem->MsgLen,
                            Addr2,
                            &CapabilityInfo,
                            &Status,
                            &Aid,
                            SupRate,
                            &SupRateLen,
                            ExtRate,
                            &ExtRateLen, 
                            &P2PSubelementLen,
                            &HtCapability,
                            &VhtCapability, 
                            &VhtOperation, 
                            &AddHtInfo,
                            &HtCapabilityLen,
                            &AddHtInfoLen,
                            &bVhtCapable,
                            &NewExtChannelOffset,
                            &EdcaParm,
                            &ExtendedCapabilities))
    {
        DBGPRINT(RT_DEBUG_ERROR, ("Receieve - ASSOC response from %02x:%02x:%02x:%02x:%02x:%02x\n", Addr2[0],Addr2[1],Addr2[2],Addr2[3],Addr2[4],Addr2[5]));
        // The frame is for me ?
        if(MAC_ADDR_EQUAL(Addr2, pAd->MlmeAux.Bssid)) 
        {
            DBGPRINT(RT_DEBUG_TRACE, ("ASSOC - receive ASSOC_RSP to me (status=%d)(AID=%d)(PortNum=%d)\n", Status, Aid, Elem->PortNum));
            //DBGPRINT(RT_DEBUG_TRACE, ("MacTable [%d].AMsduSize = %d. ClientStatusFlags = 0x%x \n",Elem->Wcid, pPort->MacTab.Content[Elem->Wcid].AMsduSize, pPort->MacTab.Content[Elem->Wcid].ClientStatusFlags));
            PlatformCancelTimer(&pPort->Mlme.AssocTimer, &TimerCancelled);
            if(Status == MLME_SUCCESS) 
            {
#if(COMPILE_WIN8_ABOVE(CURRENT_OS_NDIS_VER))
                pAd->StaCfg.NLOEntry.bNLOAssocRspToMe = TRUE; // for instant connect, make sure to receive assoc rsp before wake up 
#endif
                DBGPRINT(RT_DEBUG_TRACE, ("ASSOC - P2PSubelementLen = %d  )\n", P2PSubelementLen));
                if ((P2PSubelementLen > 0) && (P2P_ON(pPort)) && (IS_P2P_CONNECT_IDLE(pPort)))
                {
                    // Sometimes we connect to GO via Wireless network connect profile reconnect after a link down.
                    // in this case, we still need to remember we are p2p client. so update state to P2P_I_AM_CLIENT_ASSOC_AUTH
                    // In this case, doesn't need to reset Wpa2pskCounter100ms. 
                    if (PlatformEqualMemory(pAd->MlmeAux.Ssid, WILDP2PSSID, WILDP2PSSIDLEN))
                        pPort->P2PCfg.P2PConnectState = P2P_I_AM_CLIENT_ASSOC_AUTH;

                }
                //
                // There may some packets will be drop, if we haven't set the BSS type!
                // For example: EAPOL packet and the case of WHQL lost Packets.
                // Since this may some delays to set those variables at MlmeCntLinkUp(..) on this (Mlme) thread. 
                // 
                // This is a trick, set the BSS type here.
                //
                if (pAd->StaCfg.BssType == BSS_INFRA)
                {
                    OPSTATUS_SET_FLAG(pPort, fOP_STATUS_MEDIA_STATE_CONNECTED);
                    //
                    // Patch Linksys AP: BEFW11S4 Ver.4. We need to queue EAPOL frame if necessary 
                    //
                    pAd->pRxCfg->bQueueEAPOL = TRUE;
                    //to compatible with multiple bssid mode
                    
                    if (pAd->HwCfg.BeaconNum == 0)
                    {
                        MtAsicEnableBssSync(pPort->pAd, pPort->CommonCfg.BeaconPeriod);
                    }
                    //
                    // when associating with incoming AP, the packets from previous one should be ignored
                    // update the incoming BSSID as early as possible to get rid of the unexpected packets from previous AP
                    //
                    COPY_MAC_ADDR(pPort->PortCfg.Bssid, pAd->MlmeAux.Bssid);
                }

                // go to procedure listed on page 376
                MlmeAssocPostProc(pAd,
                            pPort,
                            Addr2,
                            CapabilityInfo,
                            Aid,
                            SupRate,
                            SupRateLen,
                            ExtRate,
                            ExtRateLen, 
                            &EdcaParm,
                            &HtCapability,
                            HtCapabilityLen,
                            &AddHtInfo,
                            bVhtCapable,
                            &VhtCapability,
                            &VhtOperation,
                            &ExtendedCapabilities);

            } 
            else // AssocRsp responses failed.
            {  
                // Faile on Association, we need to check the status code
                if (Status == MLME_ASSOC_REJ_TRY_LATER)
                {

                    PFRAME_802_11 pFrame = (PFRAME_802_11)Elem->Msg;
                    PEID_STRUCT   pEid;
                    ULONG         Length = 0;
                    ULONG         TimeOutIntvl = 0;
    
 
                    Length += LENGTH_802_11 + 6; //LENGTH_802_11 + CapabilityInfo, Status, Aid
                    SupRateLen = pFrame->Octet[7];
                    Length = Length + 2 + SupRateLen; // + Support rate len

                    pEid = (PEID_STRUCT) &pFrame->Octet[8 + (SupRateLen)];
            
                    // get variable fields from payload and advance the pointer
                    while ((Length + 2 + pEid->Len) <= Elem->MsgLen)
                    {
                        switch (pEid->Eid)
                        {
                            case IE_TIMEOUT_INTERVAL:
                                DBGPRINT(RT_DEBUG_TRACE, ("MFP:Len %d, Type %d\n", pEid->Len, pEid->Octet[0]));
                                if(pEid->Len+2 == 7 && pEid->Octet[0] == 3)
                                {
                                    PlatformMoveMemory(&TimeOutIntvl, pEid->Octet + 1, sizeof(ULONG));
                                    DBGPRINT(RT_DEBUG_TRACE, ("MFP:Assoc Come Back Time:%d\n", TimeOutIntvl));
                                }
                                break;

                            default:
                                break;

                        }

                        Length = Length + 2 + pEid->Len;
                        pEid = (PEID_STRUCT)((UCHAR*)pEid + 2 + pEid->Len);
                    }

                    DBGPRINT(RT_DEBUG_TRACE, ("MFP: Assoc Rejected, Try Later after %d TUs\n",TimeOutIntvl));
                    if( TimeOutIntvl != 0)
                    {
                        PlatformSetTimer(pPort, &pPort->Mlme.AssocTimer, TimeOutIntvl); // 1024 us ~= 1 ms
                        //pAd->StaCfg.PmfCfg.ComeBackTime = TimeOutIntvl; //for future Assoc Indication
                        pPort->Mlme.AssocMachine.CurrState = ASSOC_WAIT_RSP;
                        return;
                    }
                }

            }

            //
            // Save this Association Response frame for NDIS_STATUS_DOT11_ASSOCIATION_COMPLETION
            //
            PlatformMoveMemory(pAd->StaCfg.AssocResponse, Elem->Msg, Elem->MsgLen);
            pAd->StaCfg.AssocResponseLength = (USHORT)Elem->MsgLen;

            // Set active PhyId
            pAd->StaCfg.ActivePhyId = MlmeInfoGetPhyIdByChannel(pAd, pPort->Channel);
            
            pPort->Mlme.AssocMachine.CurrState = ASSOC_IDLE;
            MlmeEnqueue(pAd,pPort, MLME_CNTL_STATE_MACHINE, MT2_ASSOC_CONF, 2, &Status);
        } 
        else
            DBGPRINT(RT_DEBUG_TRACE,("The ASSOC response is NOT for me!\n"));
    }
    else
    {
        DBGPRINT(RT_DEBUG_TRACE, ("ASSOC - %s() sanity check fail\n", __FUNCTION__));
    }
}

/*
    ==========================================================================
    Description:
        peer sends reassoc rsp
    Parametrs:
        Elem - MLME message cntaining the received frame

    IRQL = DISPATCH_LEVEL

    ==========================================================================
 */
VOID MlmeAssocPeerReassocRspAction(
    IN PMP_ADAPTER pAd, 
    IN MLME_QUEUE_ELEM *Elem) 
{
    USHORT      CapabilityInfo;
    USHORT      Status;
    USHORT      Aid;
    UCHAR       SupRate[MAX_LEN_OF_SUPPORTED_RATES], SupRateLen;
    UCHAR       ExtRate[MAX_LEN_OF_SUPPORTED_RATES], ExtRateLen;
    UCHAR       Addr2[MAC_ADDR_LEN];
    BOOLEAN     TimerCancelled;
    EDCA_PARM   EdcaParm;
    HT_CAPABILITY_IE        HtCapability;
    ADD_HT_INFO_IE      AddHtInfo;  // AP might use this additional ht info IE 
    UCHAR           HtCapabilityLen=0;
    UCHAR           AddHtInfoLen=0;
    UCHAR           NewExtChannelOffset = 0xff;
    PMP_PORT      pPort = pAd->PortList[Elem->PortNum];
    ULONG       P2PSubelementLen;
    EXT_CAP_ELEMT ExtendedCapabilities = {0};

    // for VHT
    BOOLEAN       bVhtCapable=FALSE;
    VHT_CAP_IE    VhtCapability;
    VHT_OP_IE     VhtOperation;

    if(PeerAssocRspSanity(pAd,
                        pPort,
                        Elem->Msg,
                        Elem->MsgLen,
                        Addr2,
                        &CapabilityInfo,
                        &Status,
                        &Aid,
                        SupRate,
                        &SupRateLen,
                        ExtRate,
                        &ExtRateLen,
                        &P2PSubelementLen,
                        &HtCapability,
                        &VhtCapability,
                        &VhtOperation,
                        &AddHtInfo,
                        &bVhtCapable,
                        &HtCapabilityLen,
                        &AddHtInfoLen,
                        &NewExtChannelOffset,
                        &EdcaParm,
                        &ExtendedCapabilities))
    {
        if(MAC_ADDR_EQUAL(Addr2, pAd->MlmeAux.Bssid)) // The frame is for me ?
        {
            DBGPRINT(RT_DEBUG_TRACE, ("ASSOC - receive REASSOC_RSP to me (status=%d)(PortNum=%d)\n", Status, Elem->PortNum));
            PlatformCancelTimer(&pPort->Mlme.ReassocTimer, &TimerCancelled);

            if(Status == MLME_SUCCESS) 
            {
                DBGPRINT(RT_DEBUG_TRACE, ("ASSOC - P2PSubelementLen = %d  )\n", P2PSubelementLen));
                if ((P2PSubelementLen > 0) && (P2P_ON(pPort)) && (IS_P2P_CONNECT_IDLE(pPort)))
                {
                    // Sometimes we connect to GO via Wireless network connect profile reconnect after a link down.
                    // in this case, we still need to remember we are p2p client. so update state to P2P_I_AM_CLIENT_ASSOC_AUTH
                    // In this case, doesn't need to reset Wpa2pskCounter100ms. 
                    if (PlatformEqualMemory(pAd->MlmeAux.Ssid, WILDP2PSSID, WILDP2PSSIDLEN))
                        pPort->P2PCfg.P2PConnectState = P2P_I_AM_CLIENT_ASSOC_AUTH;

                }
                
                if (pAd->StaCfg.BssType == BSS_INFRA)
                {
                    OPSTATUS_SET_FLAG(pPort, fOP_STATUS_MEDIA_STATE_CONNECTED);
                    //
                    // Patch Linksys AP: BEFW11S4 Ver.4. We need to queue EAPOL frame if necessary 
                    //
                    pAd->pRxCfg->bQueueEAPOL = TRUE;

                    if (pAd->HwCfg.BeaconNum == 0)
                    {
                        MtAsicEnableBssSync(pPort->pAd, pPort->CommonCfg.BeaconPeriod);
                    }

                    //
                    // when associating with incoming AP, the packets from previous one should be ignored
                    // update the incoming BSSID as early as possible to get rid of the unexpected packets from previous AP
                    //
                    COPY_MAC_ADDR(pPort->PortCfg.Bssid, pAd->MlmeAux.Bssid);
                }

                // go to procedure listed on page 376
                MlmeAssocPostProc(pAd,
                            pPort,
                            Addr2,
                            CapabilityInfo,
                            Aid,
                            SupRate,
                            SupRateLen,
                            ExtRate,
                            ExtRateLen,
                            &EdcaParm,
                            &HtCapability,
                            HtCapabilityLen,
                            &AddHtInfo,
                            bVhtCapable,
                            &VhtCapability,
                            &VhtOperation,
                            &ExtendedCapabilities);
            } 
            else if (Status == MLME_ASSOC_REJ_TRY_LATER)
            {

                PFRAME_802_11 pFrame = (PFRAME_802_11)Elem->Msg;
                PEID_STRUCT   pEid;
                ULONG         Length = 0;
                BOOLEAN       bCisco_OUI =FALSE;
                ULONG         TimeOutIntvl = 0;


                Length += LENGTH_802_11 + 6; //LENGTH_802_11 + CapabilityInfo, Status, Aid
                SupRateLen = pFrame->Octet[7];
                Length = Length + 2 + SupRateLen; // + Support rate len

                pEid = (PEID_STRUCT) &pFrame->Octet[8 + (SupRateLen)];
        
                // get variable fields from payload and advance the pointer
                while ((Length + 2 + pEid->Len) <= Elem->MsgLen)
                {
                    switch (pEid->Eid)
                    {
                        case IE_TIMEOUT_INTERVAL:
                            DBGPRINT(RT_DEBUG_TRACE, ("MFP:Len %d, Type %d\n", pEid->Len, pEid->Octet[0]));
                            if(pEid->Len+2 == 7 && pEid->Octet[0] == 3)
                            {
                                PlatformMoveMemory(&TimeOutIntvl, pEid->Octet + 1, sizeof(ULONG));
                                DBGPRINT(RT_DEBUG_TRACE, ("MFP:Assoc Come Back Time:%d\n", TimeOutIntvl));
                            }
                            break;

                        default:
                            break;

                    }

                    Length = Length + 2 + pEid->Len;
                    pEid = (PEID_STRUCT)((UCHAR*)pEid + 2 + pEid->Len);
                }

                DBGPRINT(RT_DEBUG_TRACE, ("MFP: Assoc Rejected, Try Later after %d TUs\n",TimeOutIntvl));
                if( TimeOutIntvl != 0)
                {
                    PlatformSetTimer(pPort, &pPort->Mlme.AssocTimer, TimeOutIntvl); // 1024 us ~= 1 ms
                    //pAd->StaCfg.PmfCfg.ComeBackTime = TimeOutIntvl; //for future Assoc Indication
                    pPort->Mlme.AssocMachine.CurrState = REASSOC_WAIT_RSP;
                    return;
                }
            }

            pPort->Mlme.AssocMachine.CurrState = ASSOC_IDLE;
            MlmeEnqueue(pAd,pPort, MLME_CNTL_STATE_MACHINE, MT2_REASSOC_CONF, 2, &Status);
        }
    } 
    else
    {
        DBGPRINT(RT_DEBUG_TRACE, ("ASSOC - MlmeAssocPeerReassocRspAction() sanity check fail\n"));
    }

}

/*
    ==========================================================================
    Description:
        procedures on IEEE 802.11/1999 p.376 
    Parametrs:

    IRQL = DISPATCH_LEVEL

    ==========================================================================
 */
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
    IN VHT_CAP_IE         *pVhtCapability,
    IN VHT_OP_IE          *pVhtOperation,   // AP might use this additional ht info IE 
    IN PEXT_CAP_ELEMT pExtendedCapabilities)
{
    ULONG Idx = 0;
    USHORT VarIesOffset = 0;
    UCHAR   Wcid = (UCHAR)MlmeSyncGetWlanIdxByPort(pPort, ROLE_WLANIDX_BSSID);
    PMAC_TABLE_ENTRY pWcidMacTabEntry = NULL;
    
    BOOLEAN bP2pLegacy = FALSE;

    if (pPort == NULL)
    {
        DBGPRINT(RT_DEBUG_ERROR, ("%s - pPort is  NULL, Error, and return this function", __FUNCTION__));
        return;
    }

    pAd->MlmeAux.BssType = BSS_INFRA;
    COPY_MAC_ADDR(pAd->MlmeAux.Bssid, pAddr2);
    pAd->MlmeAux.Aid = Aid;

    pAd->MlmeAux.CapabilityInfo = CapabilityInfo & SUPPORTED_CAPABILITY_INFO;

    // Some HT AP might lost WMM IE. We add WMM ourselves. beacuase HT requires QoS on.
    if ((HtCapabilityLen > 0) && (pEdcaParm->bValid == FALSE))
    {
        pEdcaParm->bValid = TRUE;
        pEdcaParm->Aifsn[0] = 3;
        pEdcaParm->Aifsn[1] = 7;
        pEdcaParm->Aifsn[2] = 2;
        pEdcaParm->Aifsn[3] = 2;

        pEdcaParm->Cwmin[0] = 4;
        pEdcaParm->Cwmin[1] = 4;
        pEdcaParm->Cwmin[2] = 3;
        pEdcaParm->Cwmin[3] = 2;

        pEdcaParm->Cwmax[0] = 10;
        pEdcaParm->Cwmax[1] = 10;
        pEdcaParm->Cwmax[2] = 4;
        pEdcaParm->Cwmax[3] = 3;

        pEdcaParm->Txop[0]  = 0;
        pEdcaParm->Txop[1]  = 0;
        pEdcaParm->Txop[2]  = AC2_DEF_TXOP;
        pEdcaParm->Txop[3]  = AC3_DEF_TXOP;

    }

    PlatformMoveMemory(&pAd->MlmeAux.APEdcaParm, pEdcaParm, sizeof(EDCA_PARM));

    // filter out un-supported rates
    pAd->MlmeAux.SupRateLen = SupRateLen;
    PlatformMoveMemory(pAd->MlmeAux.SupRate, SupRate, SupRateLen);
    RTMPCheckRates(pAd, pAd->MlmeAux.SupRate, &pAd->MlmeAux.SupRateLen);

    // filter out un-supported rates
    pAd->MlmeAux.ExtRateLen = ExtRateLen;
    PlatformMoveMemory(pAd->MlmeAux.ExtRate, ExtRate, ExtRateLen);
    RTMPCheckRates(pAd, pAd->MlmeAux.ExtRate, &pAd->MlmeAux.ExtRateLen);

    if (IS_P2P_CON_CLI(pAd, pPort) || IS_P2P_MS_CLI(pAd, pPort) || IS_P2P_MS_CLI2(pAd, pPort))
    {
        Wcid = P2pGetClientWcid(pAd, pPort);
        
        // Legacy mode doesn't care about HT capabilities
        if (pPort->P2PCfg.P2pPhyMode == P2P_PHYMODE_LEGACY_ONLY)
            bP2pLegacy = TRUE;
    }
    
    if ((HtCapabilityLen > 0) && (bP2pLegacy == FALSE))
    {
        RTMPCheckHt(pAd, pPort, Wcid, pHtCapability, pAddHtInfo, bVhtCapable, pVhtCapability);
    }

    if ((pPort->CommonCfg.PhyMode==PHY_11VHT) // 1. we do support VHT
        && (bVhtCapable==TRUE))             // 2. the other side support VHT
    {
        RTMPCheckVht(pAd, pPort, Wcid, pVhtCapability, pVhtOperation);
    }

    pWcidMacTabEntry = MlmeSyncMacTabMatchedWlanIdx(pPort, Wcid);  

    if(pWcidMacTabEntry != NULL)
    {
        DBGPRINT(RT_DEBUG_TRACE, ("%s===>PortNum = %d. Wcid = %d. \n",__FUNCTION__, pPort->PortNumber, Wcid));
        DBGPRINT(RT_DEBUG_TRACE, ("%s===>AP.AMsduSize = %d. ClientStatusFlags = 0x%x \n",__FUNCTION__, pWcidMacTabEntry->AMsduSize, pWcidMacTabEntry->ClientStatusFlags));
        DBGPRINT(RT_DEBUG_TRACE, ("%s===>    (Mmps=%d, AmsduSize=%d, )\n",__FUNCTION__, pWcidMacTabEntry->MmpsMode, pWcidMacTabEntry->AMsduSize));
    
    }
    

#ifdef MULTI_CHANNEL_SUPPORT
    pPort->P2PCfg.CurrentWcid = Wcid;
#endif /*MULTI_CHANNEL_SUPPORT*/    

    // Set New WPA information
    Idx = BssTableSearch(pAd, pPort, &pAd->ScanTab, pAddr2, pPort->ScaningChannel);
    if (Idx == BSS_NOT_FOUND) 
    {
        DBGPRINT_ERR(("ASSOC - Can't find BSS after receiving Assoc response\n"));
    }
    else
    {
        // Mod by James to fix OID_802_11_ASSOCIATION_INFORMATION
        pAd->StaCfg.AssocInfo.Length = sizeof(NDIS_802_11_ASSOCIATION_INFORMATION); //+ sizeof(NDIS_802_11_FIXED_IEs);  // Filled in assoc request
        pAd->StaCfg.AssocInfo.AvailableResponseFixedIEs =
            NDIS_802_11_AI_RESFI_CAPABILITIES | NDIS_802_11_AI_RESFI_STATUSCODE | NDIS_802_11_AI_RESFI_ASSOCIATIONID;
        pAd->StaCfg.AssocInfo.ResponseFixedIEs.Capabilities  = CapabilityInfo;
        pAd->StaCfg.AssocInfo.ResponseFixedIEs.StatusCode    = MLME_SUCCESS;        // Should be success, add failed later
        pAd->StaCfg.AssocInfo.ResponseFixedIEs.AssociationId = Aid;

        // Copy BSS VarIEs to StaCfg associnfo structure.

        // Second add RSN
        PlatformMoveMemory(pAd->StaCfg.ResVarIEs + VarIesOffset, pAd->ScanTab.BssEntry[Idx].VarIEs, pAd->ScanTab.BssEntry[Idx].VarIELen);
        VarIesOffset += pAd->ScanTab.BssEntry[Idx].VarIELen;
        
        // Set Variable IEs Length
        pAd->StaCfg.ResVarIELen = VarIesOffset;
        pAd->StaCfg.AssocInfo.ResponseIELength = VarIesOffset;
    }

    pAd->MlmeAux.AssocState = dot11_assoc_state_auth_assoc;
    NdisGetCurrentSystemTime(&pAd->MlmeAux.AssociationUpTime);

    if (VHT_CAPABLE(pAd))
    {
        if(pWcidMacTabEntry != NULL)
        {
            if (pExtendedCapabilities->DoubleWord & EXTENDED_CAPABILITIES_BIT_MASK)
            {
                pWcidMacTabEntry->VhtPeerStaCtrl.bOperatingModeNotificationCapable = TRUE;
            }
            else
            {
                pWcidMacTabEntry->VhtPeerStaCtrl.bOperatingModeNotificationCapable = FALSE;
            }
            
            DBGPRINT(RT_DEBUG_TRACE, ("%s: bOperatingModeNotificationCapable (%d)\n", 
                __FUNCTION__, 
                pWcidMacTabEntry->VhtPeerStaCtrl.bOperatingModeNotificationCapable));
        }
    }
}

/*
    ==========================================================================
    Description:
        left part of IEEE 802.11/1999 p.374 
    Parameters:
        Elem - MLME message containing the received frame

    IRQL = DISPATCH_LEVEL

    ==========================================================================
 */
VOID MlmeAssocPeerDisassocAction(
    IN PMP_ADAPTER pAd, 
    IN MLME_QUEUE_ELEM *Elem) 
{
    UCHAR       Addr2[MAC_ADDR_LEN];
    USHORT      Reason;
    PMP_PORT  pPort = pAd->PortList[Elem->PortNum];
    
    DBGPRINT(RT_DEBUG_TRACE, ("ASSOC - MlmeAssocPeerDisassocAction()\n"));
    if(PeerDisassocSanity(pAd, Elem->Msg, Elem->MsgLen, Addr2, &Reason)) 
    {
    
        DBGPRINT(RT_DEBUG_TRACE, ("ASSOC - MlmeAssocPeerDisassocAction (Reason=%d) (PortNum=%d)\n", Reason, Elem->PortNum));

        if(P2P_ON(pPort) && (pPort->CommonCfg.P2pControl.field.EnablePresistent == 1) && (pAd->PortList[pPort->P2PCfg.PortNumber]->PortSubtype == PORTSUBTYPE_P2PClient))
            pPort->P2PCfg.P2pCounter.CounterAutoReconnectForP2pClient = 10;
        if (INFRA_ON(pPort) && MAC_ADDR_EQUAL(pPort->PortCfg.Bssid, Addr2)) 
        {
            if((Reason == REASON_P2P_PEER_MANUAL_DEAUTH) && IS_P2P_SIGMA_OFF(pPort) && P2P_ON(pPort) && (pPort->CommonCfg.P2pControl.field.EnablePresistent == TRUE))
            {
                
                pPort->P2PCfg.P2PQueryStatusOid.P2PStatus = P2PEVENT_PEER_MANUAL_DISCONNECTION;
                if (pPort->P2PCfg.P2pEvent != NULL)
                {   
                    PlatformMoveMemory(pPort->P2PCfg.P2PQueryStatusOid.P2PStateContext.DisplayPin.MacAddr, Addr2, 6);
                    KeSetEvent((PKEVENT)pPort->P2PCfg.P2pEvent, 0, FALSE);
                    pPort->P2PCfg.P2pEventQueue.LastSetEvent = P2PEVENT_PEER_MANUAL_DISCONNECTION;
                    pPort->P2PCfg.P2pCounter.CounterAftrSetEvent = 0;
                    DBGPRINT(RT_DEBUG_TRACE,("P2pKeSetEvent %d", P2PEVENT_PEER_MANUAL_DISCONNECTION));
                    
                }
                
            }

            pAd->MlmeAux.AssocState = dot11_assoc_state_unauth_unassoc;
            PlatformIndicateDisassociation(pAd,pPort, pPort->PortCfg.Bssid, DOT11_ASSOC_STATUS_PEER_DISASSOCIATED_START | Reason);
            DBGPRINT(RT_DEBUG_TRACE, ("TONDIS:  MlmeAssocPeerDisassocAction, PlatformIndicateDisassociation Reason=0x%08x, [%02x:%02x:%02x:%02x:%02x:%02x]\n",
                        Reason | DOT11_ASSOC_STATUS_PEER_DISASSOCIATED_START, pPort->PortCfg.Bssid[0], pPort->PortCfg.Bssid[1], 
                        pPort->PortCfg.Bssid[2], pPort->PortCfg.Bssid[3], pPort->PortCfg.Bssid[4], pPort->PortCfg.Bssid[5]));                   

            if((pPort->StaCfg.WscControl.WscState == WSC_STATE_OFF) &&
                (pPort->StaCfg.WscControl.bSkipWPSTurnOffLEDAfterSuccess == FALSE))
            {
                LedCtrlSetBlinkMode(pAd, LED_WPS_TURN_LED_OFF);
                pPort->StaCfg.WscControl.PeerDisconnect = TRUE;
            }

            if(((pAd->HwCfg.LedCntl.field.LedMode & LED_MODE) == WPS_LED_MODE_6) &&
                (pPort->StaCfg.WscControl.WscState != WSC_STATE_OFF))
            {
                LedCtrlSetBlinkMode(pAd, LED_LINK_DOWN);
                pPort->StaCfg.WscControl.PeerDisconnect = TRUE;
            }

            MlmeCntLinkDown(pPort, TRUE);
            pPort->Mlme.AssocMachine.CurrState = ASSOC_IDLE;
            
            NdisGetCurrentSystemTime((PLARGE_INTEGER)&pAd->Mlme.Now64);         
#if 0
            // 2004-09-11 john: can't remember why AP will DISASSOCIATE us.
            //   But since it says for 2430 only, we temporaily remove the patch.
            // 2002/11/21 -
            //   patch RT2430/RT2420 hangup issue. We suspect this AP DIS-ASSOCIATE frame
            //   is caused by PHY hangup, so we reset PHY, then auto recover the connection.
            //   if this attempt fails, then remains in MlmeCntLinkDown and leaves the problem
            //   to MlmePeriodicExecTimerCallback()
            // NICPatchRT2430Bug(pAd);
            pAd->Counter.MTKCounters.BeenDisassociatedCount ++;
            // Remove auto recover effort when disassociate by AP, re-enable for patch 2430 only
            DBGPRINT(RT_DEBUG_TRACE, ("ASSOC - Disassociated by AP, Auto Recovery attempt #%d\n", pAd->Counter.MTKCounters.BeenDisassociatedCount));
            MlmeAutoReconnectLastSSID(pAd);
#endif
        }
    }
    else
    {
        DBGPRINT(RT_DEBUG_TRACE, ("ASSOC - MlmeAssocPeerDisassocAction() sanity check fail\n"));
    }

}

/*
    ==========================================================================
    Description:
        what the state machine will do after assoc timeout
    Parameters:
        Elme -

    IRQL = DISPATCH_LEVEL

    ==========================================================================
 */
VOID MlmeAssocTimeoutTimerCallbackAction(
    IN PMP_ADAPTER pAd, 
    IN MLME_QUEUE_ELEM *Elem) 
{
    PMP_PORT  pPort = pAd->PortList[Elem->PortNum];
    USHORT      Status;
    DBGPRINT(RT_DEBUG_TRACE, ("ASSOC - MlmeAssocTimeoutTimerCallbackAction\n"));
    pPort->Mlme.AssocMachine.CurrState = ASSOC_IDLE;
    Status = MLME_REJ_TIMEOUT;
    MlmeEnqueue(pAd,pPort, MLME_CNTL_STATE_MACHINE, MT2_ASSOC_CONF, 2, &Status);
}

/*
    ==========================================================================
    Description:
        what the state machine will do after reassoc timeout

    IRQL = DISPATCH_LEVEL

    ==========================================================================
 */
VOID MlmeAssocReAssocTimeoutTimerCallbackAction(
    IN PMP_ADAPTER pAd, 
    IN MLME_QUEUE_ELEM *Elem) 
{
    PMP_PORT  pPort = pAd->PortList[Elem->PortNum];
    USHORT      Status;
    DBGPRINT(RT_DEBUG_TRACE, ("ASSOC - MlmeAssocReAssocTimeoutTimerCallbackAction\n"));
    pPort->Mlme.AssocMachine.CurrState = ASSOC_IDLE;
    Status = MLME_REJ_TIMEOUT;
    MlmeEnqueue(pAd,pPort, MLME_CNTL_STATE_MACHINE, MT2_REASSOC_CONF, 2, &Status);
}

/*
    ==========================================================================
    Description:
        what the state machine will do after disassoc timeout

    IRQL = DISPATCH_LEVEL

    ==========================================================================
    */
VOID MlmeAssocDisAssocTimeoutTimerCallbackAction(
    IN PMP_ADAPTER pAd, 
    IN MLME_QUEUE_ELEM *Elem) 
{
    USHORT      Status;
    PMP_PORT  pPort = pAd->PortList[Elem->PortNum];
    DBGPRINT(RT_DEBUG_TRACE, ("ASSOC - MlmeAssocDisAssocTimeoutTimerCallbackAction\n"));
    pPort->Mlme.AssocMachine.CurrState = ASSOC_IDLE;
    Status = MLME_SUCCESS;

    MlmeEnqueue(pAd,pPort, MLME_CNTL_STATE_MACHINE, MT2_DISASSOC_CONF, 2, &Status);
}

VOID MlmeAssocInvalidStateWhenAssoc(
    IN PMP_ADAPTER pAd, 
    IN MLME_QUEUE_ELEM *Elem) 
{
    USHORT      Status;
    PMP_PORT  pPort = pAd->PortList[Elem->PortNum];
    DBGPRINT(RT_DEBUG_TRACE, ("ASSOC - MlmeAssocInvalidStateWhenAssoc(state=%d), reset ASSOC state machine\n", 
            pPort->Mlme.AssocMachine.CurrState));
    pPort->Mlme.AssocMachine.CurrState = ASSOC_IDLE;
    Status = MLME_STATE_MACHINE_REJECT;
    MlmeEnqueue(pAd,pPort, MLME_CNTL_STATE_MACHINE, MT2_ASSOC_CONF, 2, &Status);
}

VOID MlmeAssocInvalidStateWhenReassoc(
    IN PMP_ADAPTER pAd, 
    IN MLME_QUEUE_ELEM *Elem) 
{
    USHORT      Status;
    PMP_PORT  pPort = pAd->PortList[Elem->PortNum];
    DBGPRINT(RT_DEBUG_TRACE, ("ASSOC - MlmeAssocInvalidStateWhenReassoc(state=%d), reset ASSOC state machine\n", 
            pPort->Mlme.AssocMachine.CurrState));
    pPort->Mlme.AssocMachine.CurrState = ASSOC_IDLE;
    Status = MLME_STATE_MACHINE_REJECT;
    MlmeEnqueue(pAd,pPort, MLME_CNTL_STATE_MACHINE, MT2_REASSOC_CONF, 2, &Status);
}

VOID MlmeAssocInvalidStateWhenDisassociate(
    IN PMP_ADAPTER pAd, 
    IN MLME_QUEUE_ELEM *Elem) 
{
    USHORT      Status;
    PMP_PORT  pPort = pAd->PortList[Elem->PortNum];
    DBGPRINT(RT_DEBUG_TRACE, ("ASSOC - InvalidStateWhenDisassoc(state=%d), reset ASSOC state machine\n", 
            pPort->Mlme.AssocMachine.CurrState));
    pPort->Mlme.AssocMachine.CurrState = ASSOC_IDLE;
    Status = MLME_STATE_MACHINE_REJECT;
    MlmeEnqueue(pAd,pPort, MLME_CNTL_STATE_MACHINE, MT2_DISASSOC_CONF, 2, &Status);
}

/*
    ==========================================================================
    Description:
        right part of IEEE 802.11/1999 page 374
    Note: 
        This event should never cause ASSOC state machine perform state
        transition, and has no relationship with CNTL machine. So we separate
        this routine as a service outside of ASSOC state transition table.

    IRQL = DISPATCH_LEVEL

    ==========================================================================
 */
VOID MlmeAssocCls3errAction(
    IN PMP_ADAPTER pAd, 
    IN PUCHAR        pAddr) 
{
    HEADER_802_11   DisassocHdr;
    PCHAR           pOutBuffer = NULL;
    PCHAR           pOutBuffer2 = NULL;
    ULONG           FrameLen = 0;
    NDIS_STATUS     NStatus;
    USHORT          Reason = REASON_CLS3ERR;
    PMP_PORT      pPort = pAd->PortList[PORT_0];
    
    NStatus = MlmeAllocateMemory(pAd, &pOutBuffer);  //Get an unused nonpaged memory
    if (NStatus != NDIS_STATUS_SUCCESS) 
        return;

    NStatus = MlmeAllocateMemory(pAd, &pOutBuffer2);  //Get an unused nonpaged memory
    if (NStatus != NDIS_STATUS_SUCCESS) 
    {
        MlmeFreeMemory(pAd, pOutBuffer);
        return;
    }
    DBGPRINT(RT_DEBUG_TRACE, ("ASSOC - Class 3 Error, Send DISASSOC frame (PortNum=%d)\n", pPort->PortNumber));
    MgtMacHeaderInit(pAd,pPort, &DisassocHdr, SUBTYPE_DISASSOC, 0, pAddr, pPort->PortCfg.Bssid);
    MakeOutgoingFrame(pOutBuffer,           &FrameLen, 
                    sizeof(HEADER_802_11),&DisassocHdr, 
                    2,                    &Reason, 
                    END_OF_ARGS);
    NdisCommonMiniportMMRequest(pAd, pOutBuffer, FrameLen);

    // To patch Instance and Buffalo(N) AP
    // Driver has to send deauth to Instance AP, but Buffalo(N) needs to send disassoc to reset Authenticator's state machine
    // Therefore, we send both of them.
    DisassocHdr.FC.SubType = SUBTYPE_DEAUTH;
    MakeOutgoingFrame(pOutBuffer2,           &FrameLen, 
                        sizeof(HEADER_802_11),&DisassocHdr, 
                        2,                    &Reason, 
                        END_OF_ARGS);
    NdisCommonMiniportMMRequest(pAd, pOutBuffer2, FrameLen);
    pAd->StaCfg.DisassocReason = REASON_CLS3ERR;
    COPY_MAC_ADDR(pAd->StaCfg.DisassocSta, pAddr);
}
 
 /*
    ==========================================================================
    Description:
        Switch between WEP and CKIP upon new association up.
    Parameters:

    IRQL = DISPATCH_LEVEL
 
    ==========================================================================
  */
VOID MlmeAssocSwitchBetweenWepAndCkip(
    IN PMP_ADAPTER pAd,
    IN PMP_PORT pPort)
{
#if 0
    INT i;
    SHAREDKEY_MODE_STRUC  csr1;

    // modify hardware key table so that MAC use correct algorithm to decrypt RX
    RTUSBReadMACRegister(pAd, SHARED_KEY_MODE_BASE, &csr1.word);
    if (csr1.field.Bss0Key0CipherAlg == CIPHER_CKIP64)
        csr1.field.Bss0Key0CipherAlg = CIPHER_WEP64;
    else if (csr1.field.Bss0Key0CipherAlg == CIPHER_CKIP104)
        csr1.field.Bss0Key0CipherAlg = CIPHER_WEP128;

    if (csr1.field.Bss0Key1CipherAlg == CIPHER_CKIP64)
        csr1.field.Bss0Key1CipherAlg = CIPHER_WEP64;
    else if (csr1.field.Bss0Key1CipherAlg == CIPHER_CKIP104)
        csr1.field.Bss0Key1CipherAlg = CIPHER_WEP128;

    if (csr1.field.Bss0Key2CipherAlg == CIPHER_CKIP64)
        csr1.field.Bss0Key2CipherAlg = CIPHER_WEP64;
    else if (csr1.field.Bss0Key2CipherAlg == CIPHER_CKIP104)
        csr1.field.Bss0Key2CipherAlg = CIPHER_WEP128;

    if (csr1.field.Bss0Key3CipherAlg == CIPHER_CKIP64)
        csr1.field.Bss0Key3CipherAlg = CIPHER_WEP64;
    else if (csr1.field.Bss0Key3CipherAlg == CIPHER_CKIP104)
        csr1.field.Bss0Key3CipherAlg = CIPHER_WEP128;
    else if (csr1.field.Bss0Key3CipherAlg == CIPHER_CKIP128)
        csr1.field.Bss0Key3CipherAlg = CIPHER_WEP128;
    RTUSBWriteMACRegister(pAd, SHARED_KEY_MODE_BASE, csr1.word);
    DBGPRINT(RT_DEBUG_TRACE, ("MlmeAssocSwitchBetweenWepAndCkip: modify BSS0 cipher to %s\n", DecodeCipherName(csr1.field.Bss0Key0CipherAlg)));

    // modify software key table so that driver can specify correct algorithm in TXD upon TX
    for (i = 0; i < SHARE_KEY_NUM; i++)
    {
        if (pPort->SharedKey[BSS0][i].CipherAlg == CIPHER_CKIP64)
            pPort->SharedKey[BSS0][i].CipherAlg = CIPHER_WEP64;
        else if (pPort->SharedKey[BSS0][i].CipherAlg == CIPHER_CKIP104)
            pPort->SharedKey[BSS0][i].CipherAlg = CIPHER_WEP128;
        else if (pPort->SharedKey[BSS0][i].CipherAlg == CIPHER_CKIP128)
            pPort->SharedKey[BSS0][i].CipherAlg = CIPHER_WEP128;
    }
#endif  
}

