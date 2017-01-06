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
    auth.c

    Abstract:

    Revision History:
    Who         When            What
    --------    ----------      ----------------------------------------------
    John        2004-9-3        porting from RT2500
*/
#include "MtConfig.h"

#define DRIVER_FILE         0x00A00000

/*
    ==========================================================================
    Description:
        authenticate state machine init, including state transition and timer init
    Parameters:
        Sm - pointer to the auth state machine
    Note:
        The state machine looks like this

    AUTH_REQ_IDLE           AUTH_WAIT_SEQ2                   AUTH_WAIT_SEQ4
    MT2_MLME_AUTH_REQ   mlme_auth_req_action    invalid_state_when_auth          invalid_state_when_auth
    MT2_PEER_AUTH_EVEN  drop                    peer_auth_even_at_seq2_action    peer_auth_even_at_seq4_action
    MT2_AUTH_TIMEOUT    Drop                    auth_timeout_action              auth_timeout_action

    IRQL = PASSIVE_LEVEL

    ==========================================================================
 */

void MlmeAuthStateMachineInit(
    IN PMP_ADAPTER pAd, 
    IN STATE_MACHINE *Sm, 
    OUT STATE_MACHINE_FUNC Trans[]) 
{
    StateMachineInit(Sm, Trans, MAX_AUTH_STATE, MAX_AUTH_MSG, Drop, AUTH_REQ_IDLE, AUTH_MACHINE_BASE);
 
    // the first column
    StateMachineSetAction(Sm, AUTH_REQ_IDLE, MT2_MLME_AUTH_REQ, MlmeAuthReqAction);

    // the second column
    StateMachineSetAction(Sm, AUTH_WAIT_SEQ2, MT2_MLME_AUTH_REQ, MlmeAuthInvalidStateWhenAuth);
    StateMachineSetAction(Sm, AUTH_WAIT_SEQ2, MT2_PEER_AUTH_EVEN, MlmeAuthPeerAuthRspAtSeq2Action);
    StateMachineSetAction(Sm, AUTH_WAIT_SEQ2, MT2_AUTH_TIMEOUT, MlmeAuthTimeoutAction);

    // the third column
    StateMachineSetAction(Sm, AUTH_WAIT_SEQ4, MT2_MLME_AUTH_REQ, MlmeAuthInvalidStateWhenAuth);
    StateMachineSetAction(Sm, AUTH_WAIT_SEQ4, MT2_PEER_AUTH_EVEN, MlmeAuthPeerAuthRspAtSeq4Action);
    StateMachineSetAction(Sm, AUTH_WAIT_SEQ4, MT2_AUTH_TIMEOUT, MlmeAuthTimeoutAction);
}

/*
    ==========================================================================
    Description:
        function to be executed at timer thread when auth timer expires
    
    IRQL = DISPATCH_LEVEL

    ==========================================================================
 */
VOID MlmeAuthTimeoutTimerCallback(
    IN PVOID SystemSpecific1, 
    IN PVOID FunctionContext, 
    IN PVOID SystemSpecific2, 
    IN PVOID SystemSpecific3) 
{
    PMP_PORT pPort = (PMP_PORT )FunctionContext;
    PMP_ADAPTER  pAd = pPort->pAd;


    // Do nothing if the driver is starting halt state.
    // This might happen when timer already been fired before cancel timer with mlmehalt
    if (MT_TEST_FLAG(pAd, fRTMP_ADAPTER_HALT_IN_PROGRESS))
        return;

    if (pPort == NULL)
        return;
    
    DBGPRINT(RT_DEBUG_TRACE,("AUTH - MlmeAuthTimeoutTimerCallback (PortNum=%d)\n", pPort->PortNumber));
   
    // send a de-auth to reset AP's state machine (Patch AP-Dir635)
    if (pPort->Mlme.AuthMachine.CurrState == AUTH_WAIT_SEQ2)
        MlmeAuthCls2errAction(pAd, pPort, pAd->MlmeAux.Bssid);
    
    MlmeEnqueue(pAd,pPort, AUTH_STATE_MACHINE, MT2_AUTH_TIMEOUT, 0, NULL);
}


/*
    ==========================================================================
    Description:
    
    IRQL = DISPATCH_LEVEL

    ==========================================================================
 */
VOID MlmeAuthReqAction(
    IN PMP_ADAPTER pAd, 
    IN MLME_QUEUE_ELEM *Elem) 
{
    UCHAR           Addr[6];
    USHORT          Alg, Seq, Status;
    ULONG           Timeout;
    HEADER_802_11   AuthHdr;
    BOOLEAN         TimerCancelled = FALSE;
    NDIS_STATUS     NStatus;
    PUCHAR          pOutBuffer = NULL;
    ULONG           FrameLen = 0;
    PMP_PORT      pPort = pAd->PortList[Elem->PortNum];

    if(MlmeAuthReqSanity(pAd, Elem->Msg, Elem->MsgLen, Addr, &Timeout, &Alg)) 
    {
        UCHAR   Atheros_DIR855A[]   = { 0x00, 0x03, 0x7F};
        UCHAR   Atheros_DIR855A2[]  = { 0x00, 0x00, 0x11};      
        UCHAR   Atheros_DIR855G[]   = { 0x00, 0x40, 0xF4};
        UCHAR   Atheros_DIR855[]    = { 0x00, 0x1E, 0x58};      
        UCHAR   Atheros_DIR655[]    = { 0x00, 0x1c, 0xF0};
        
        // Block all authentication request durning WPA MIC-ERROR block period
        if((pAd->StaCfg.bBlockAssoc == TRUE) && MAC_ADDR_EQUAL(pAd->MlmeAux.Bssid,pPort->PortCfg.LastBssid))            
        {
            DBGPRINT(RT_DEBUG_TRACE, ("AUTH - Block Auth request durning WPA block period!\n"));
            pPort->Mlme.AuthMachine.CurrState = AUTH_REQ_IDLE;
            Status = MLME_STATE_MACHINE_REJECT;
            MlmeEnqueue(pAd,pPort, MLME_CNTL_STATE_MACHINE, MT2_AUTH_CONF, 2, &Status);
            return;
        }   

        // reset timer
        PlatformCancelTimer(&pPort->Mlme.AuthTimer, &TimerCancelled);
        COPY_MAC_ADDR(pAd->MlmeAux.Bssid, Addr);
        pAd->MlmeAux.Alg  = Alg;
        Seq = 1;
        Status = MLME_SUCCESS;

        //
        // Limit maximum Rx's MCS rate to 13 and using longGI when connencting to DIR-855 or DIR-655
        //
        if ((PlatformEqualMemory(pAd->MlmeAux.Bssid, Atheros_DIR855, 3))||(PlatformEqualMemory(pAd->MlmeAux.Bssid, Atheros_DIR855A2, 3))||(PlatformEqualMemory(pAd->MlmeAux.Bssid, Atheros_DIR855A, 3))||(PlatformEqualMemory(pAd->MlmeAux.Bssid, Atheros_DIR855G, 3))||(PlatformEqualMemory(pAd->MlmeAux.Bssid, Atheros_DIR655, 3)))
        {
            DBGPRINT(RT_DEBUG_TRACE, ("MlmeAuthReqAction===>  Is Atheros AP\n"));
            pPort->CommonCfg.IOTestParm.AtherosDIR855 = TRUE;
        }
        else
        {
            pPort->CommonCfg.IOTestParm.AtherosDIR855 = FALSE;
        }

        NStatus = MlmeAllocateMemory(pAd, &pOutBuffer);  //Get an unused nonpaged memory
        if(NStatus != NDIS_STATUS_SUCCESS) 
        {
            DBGPRINT(RT_DEBUG_TRACE, ("AUTH - MlmeAuthReqAction() allocate memory failed\n"));
            pPort->Mlme.AuthMachine.CurrState = AUTH_REQ_IDLE;
            Status = MLME_FAIL_NO_RESOURCE;
            MlmeEnqueue(pAd,pPort, MLME_CNTL_STATE_MACHINE, MT2_AUTH_CONF, 2, &Status);
            return;
        }

        DBGPRINT(RT_DEBUG_TRACE, ("AUTH - Send AUTH request seq#1 (Alg=%d)(PortNum=%d)...\n", Alg, Elem->PortNum));
        MgtMacHeaderInit(pAd,pPort, &AuthHdr, SUBTYPE_AUTH, 0, Addr, pAd->MlmeAux.Bssid);
        MakeOutgoingFrame(pOutBuffer,           &FrameLen, 
                          sizeof(HEADER_802_11),&AuthHdr, 
                          2,                    &Alg, 
                          2,                    &Seq, 
                          2,                    &Status, 
                          END_OF_ARGS);
        
        NdisCommonMiniportMMRequest(pAd, pOutBuffer, FrameLen);

        PlatformSetTimer(pPort, &pPort->Mlme.AuthTimer, Timeout);
        pPort->Mlme.AuthMachine.CurrState = AUTH_WAIT_SEQ2;
    } 
    else 
    {
        DBGPRINT_ERR(("AUTH - MlmeAuthReqAction() sanity check failed\n"));
        pPort->Mlme.AuthMachine.CurrState = AUTH_REQ_IDLE;
        Status = MLME_INVALID_FORMAT;
        MlmeEnqueue(pAd,pPort, MLME_CNTL_STATE_MACHINE, MT2_AUTH_CONF, 2, &Status);
    }
}

/*
    ==========================================================================
    Description:
    
    IRQL = DISPATCH_LEVEL

    ==========================================================================
 */
VOID MlmeAuthPeerAuthRspAtSeq2Action(
    IN PMP_ADAPTER pAd, 
    IN MLME_QUEUE_ELEM *Elem) 
{
    UCHAR         Addr2[MAC_ADDR_LEN];
    USHORT        Seq, Status, RemoteStatus, Alg;
    UCHAR         ChlgText[CIPHER_TEXT_LEN];
    UCHAR         CyperChlgText[CIPHER_TEXT_LEN + 8 + 8];
    UCHAR         Element[2];
    HEADER_802_11 AuthHdr;
    BOOLEAN       TimerCancelled;
    PUCHAR        pOutBuffer = NULL;
    NDIS_STATUS   NStatus;
    ULONG         FrameLen = 0;
    USHORT        Status2;
    PMP_PORT      pPort = pAd->PortList[Elem->PortNum];

    if (PeerAuthSanity(pAd, Elem->Msg, Elem->MsgLen, Addr2, &Alg, &Seq, &Status, ChlgText)) 
    {
        if (MAC_ADDR_EQUAL(pAd->MlmeAux.Bssid, Addr2) && Seq == 2) 
        {
            DBGPRINT(RT_DEBUG_TRACE, ("AUTH - Receive AUTH_RSP seq#2 to me (Alg=%d, Status=%d) (PortNum=%d)\n", Alg, Status, Elem->PortNum));
            PlatformCancelTimer(&pPort->Mlme.AuthTimer, &TimerCancelled);

            if (Status == MLME_SUCCESS) 
            {    
                // Authentication Mode "Network-EAP" is allowed
                if (pAd->MlmeAux.Alg == AUTH_OPEN_SYSTEM)
                {
                    pPort->Mlme.AuthMachine.CurrState = AUTH_REQ_IDLE;                  
                    MlmeEnqueue(pAd,pPort, MLME_CNTL_STATE_MACHINE, MT2_AUTH_CONF, 2, &Status);
                } 
                else 
                {
                    // 2. shared key, need to be challenged
                    Seq++;
                    RemoteStatus = MLME_SUCCESS;
                    NStatus = MlmeAllocateMemory(pAd, &pOutBuffer);  //Get an unused nonpaged memory
                    if(NStatus != NDIS_STATUS_SUCCESS) 
                    {
                        DBGPRINT(RT_DEBUG_TRACE, ("AUTH - MlmeAuthPeerAuthRspAtSeq2Action() allocate memory fail\n"));
                        pPort->Mlme.AuthMachine.CurrState = AUTH_REQ_IDLE;
                        Status2 = MLME_FAIL_NO_RESOURCE;
                        MlmeEnqueue(pAd,pPort, MLME_CNTL_STATE_MACHINE, MT2_AUTH_CONF, 2, &Status2);
                        return;
                    }

                    DBGPRINT(RT_DEBUG_TRACE, ("AUTH - Send AUTH request seq#3...\n"));
                    MgtMacHeaderInit(pAd,pPort, &AuthHdr, SUBTYPE_AUTH, 0, Addr2, pAd->MlmeAux.Bssid);
                    AuthHdr.FC.Wep = 1;
                    // Encrypt challenge text & auth information
                    RTMPInitWepEngine(
                                    pAd,
                                    pPort->SharedKey[BSS0][pPort->PortCfg.DefaultKeyId].Key,
                                    pPort->PortCfg.DefaultKeyId,
                                    pPort->SharedKey[BSS0][pPort->PortCfg.DefaultKeyId].KeyLen,
                                    CyperChlgText);
                    RTMPEncryptData(pAd, (PUCHAR) &Alg, CyperChlgText + 4, 2);
                    RTMPEncryptData(pAd, (PUCHAR) &Seq, CyperChlgText + 6, 2);
                    RTMPEncryptData(pAd, (PUCHAR) &RemoteStatus, CyperChlgText + 8, 2);
                    Element[0] = 16;
                    Element[1] = 128;
                    RTMPEncryptData(pAd, Element, CyperChlgText + 10, 2);
                    RTMPEncryptData(pAd, ChlgText, CyperChlgText + 12, 128);
                    RTMPSetICV(pAd, CyperChlgText + 140);
                    MakeOutgoingFrame(pOutBuffer,               &FrameLen, 
                                      sizeof(HEADER_802_11),    &AuthHdr,  
                                      CIPHER_TEXT_LEN + 16,     CyperChlgText, 
                                      END_OF_ARGS);
                    NdisCommonMiniportMMRequest(pAd, pOutBuffer, FrameLen);

                    PlatformSetTimer(pPort, &pPort->Mlme.AuthTimer, AUTH_TIMEOUT); //AUTH_KEY_TIMEOUT);
                    pPort->Mlme.AuthMachine.CurrState = AUTH_WAIT_SEQ4;
                }
            } 
            else 
            {
                pAd->StaCfg.AuthFailReason = Status;
                COPY_MAC_ADDR(pAd->StaCfg.AuthFailSta, Addr2);
                pPort->Mlme.AuthMachine.CurrState = AUTH_REQ_IDLE;
                MlmeEnqueue(pAd,pPort, MLME_CNTL_STATE_MACHINE, MT2_AUTH_CONF, 2, &Status);
            }
        }
    }
    else
    {
        DBGPRINT(RT_DEBUG_TRACE, ("AUTH - PeerAuthSanity() sanity check fail\n"));
    }
}

/*
    ==========================================================================
    Description:
    
    IRQL = DISPATCH_LEVEL

    ==========================================================================
 */
VOID MlmeAuthPeerAuthRspAtSeq4Action(
    IN PMP_ADAPTER pAd, 
    IN MLME_QUEUE_ELEM *Elem) 
{
    UCHAR         Addr2[MAC_ADDR_LEN];
    USHORT        Alg, Seq, Status;
    CHAR          ChlgText[CIPHER_TEXT_LEN];
    BOOLEAN       TimerCancelled;
    PMP_PORT  pPort = pAd->PortList[Elem->PortNum];
    
    if(PeerAuthSanity(pAd, Elem->Msg, Elem->MsgLen, Addr2, &Alg, &Seq, &Status, ChlgText)) 
    {
        if(MAC_ADDR_EQUAL(pAd->MlmeAux.Bssid, Addr2) && Seq == 4) 
        {
            DBGPRINT(RT_DEBUG_TRACE, ("AUTH - Receive AUTH_RSP seq#4 to me (PortNum=%d)\n", Elem->PortNum));
            PlatformCancelTimer(&pPort->Mlme.AuthTimer, &TimerCancelled);

            if (Status != MLME_SUCCESS) 
            {
                pAd->StaCfg.AuthFailReason = Status;
                COPY_MAC_ADDR(pAd->StaCfg.AuthFailSta, Addr2);
            }                

            pPort->Mlme.AuthMachine.CurrState = AUTH_REQ_IDLE;
            MlmeEnqueue(pAd,pPort, MLME_CNTL_STATE_MACHINE, MT2_AUTH_CONF, 2, &Status);
        }
    }
    else
    {
        DBGPRINT(RT_DEBUG_TRACE, ("AUTH - MlmeAuthPeerAuthRspAtSeq4Action() sanity check fail\n"));
    }
}

/*
    ==========================================================================
    Description:
    
    IRQL = DISPATCH_LEVEL

    ==========================================================================
 */
VOID MlmeAuthDeauthReqAction(
    IN PMP_ADAPTER pAd, 
    IN NDIS_PORT_NUMBER PortNum,
    IN MLME_DEAUTH_REQ_STRUCT *pInfo) 
{
    HEADER_802_11 DeauthHdr;
    PUCHAR        pOutBuffer = NULL;
    NDIS_STATUS   NStatus;
    ULONG         FrameLen = 0;
    USHORT        Status;
    PMP_PORT  pPort = pAd->PortList[PortNum];

    NStatus = MlmeAllocateMemory(pAd, &pOutBuffer);  //Get an unused nonpaged memory
    if (NStatus != NDIS_STATUS_SUCCESS) 
    {
        DBGPRINT(RT_DEBUG_TRACE, ("AUTH - MlmeAuthDeauthReqAction() allocate memory fail\n"));
        pPort->Mlme.AuthMachine.CurrState = AUTH_REQ_IDLE;
        Status = MLME_FAIL_NO_RESOURCE;
        MlmeEnqueue(pAd,pPort, MLME_CNTL_STATE_MACHINE, MT2_DEAUTH_CONF, 2, &Status);
        return;
    }

    DBGPRINT(RT_DEBUG_TRACE, ("AUTH - Send DE-AUTH request (Reason=%d)(PortNum=%d)...\n", pInfo->Reason, PortNum));
    MgtMacHeaderInit(pAd,pPort, &DeauthHdr, SUBTYPE_DEAUTH, 0, pInfo->Addr, pAd->MlmeAux.Bssid);
    MakeOutgoingFrame(pOutBuffer,           &FrameLen, 
                      sizeof(HEADER_802_11),&DeauthHdr, 
                      2,                    &pInfo->Reason, 
                      END_OF_ARGS);
    NdisCommonMiniportMMRequest(pAd, pOutBuffer, FrameLen);

    pAd->StaCfg.DeauthReason = pInfo->Reason;
    COPY_MAC_ADDR(pAd->StaCfg.DeauthSta, pInfo->Addr);
    pPort->Mlme.AuthMachine.CurrState = AUTH_REQ_IDLE;
    Status = MLME_SUCCESS;
    MlmeEnqueue(pAd,pPort, MLME_CNTL_STATE_MACHINE, MT2_DEAUTH_CONF, 2, &Status);
}

/*
    ==========================================================================
    Description:

    IRQL = DISPATCH_LEVEL

    ==========================================================================
 */
VOID MlmeAuthTimeoutAction(
    IN PMP_ADAPTER pAd, 
    IN MLME_QUEUE_ELEM *Elem) 
{
    USHORT Status;
    PMP_PORT  pPort = pAd->PortList[Elem->PortNum];
    
    DBGPRINT(RT_DEBUG_TRACE, ("AUTH - MlmeAuthTimeoutAction\n"));
    pPort->Mlme.AuthMachine.CurrState = AUTH_REQ_IDLE;
    Status = MLME_REJ_TIMEOUT;
    MlmeEnqueue(pAd,pPort, MLME_CNTL_STATE_MACHINE, MT2_AUTH_CONF, 2, &Status);
}

/*
    ==========================================================================
    Description:

    IRQL = DISPATCH_LEVEL

    ==========================================================================
 */
VOID MlmeAuthInvalidStateWhenAuth(
    IN PMP_ADAPTER pAd, 
    IN MLME_QUEUE_ELEM *Elem) 
{
    USHORT      Status;
    PMP_PORT  pPort = pAd->PortList[Elem->PortNum];
    
    DBGPRINT(RT_DEBUG_TRACE, ("AUTH - MlmeAuthInvalidStateWhenAuth (state=%d), reset AUTH state machine\n", pPort->Mlme.AuthMachine.CurrState));
    pPort->Mlme.AuthMachine.CurrState = AUTH_REQ_IDLE;
    Status = MLME_STATE_MACHINE_REJECT;
    MlmeEnqueue(pAd,pPort, MLME_CNTL_STATE_MACHINE, MT2_AUTH_CONF, 2, &Status);
}

/*
    ==========================================================================
    Description:
        Some STA/AP
    Note:
        This action should never trigger AUTH state transition, therefore we
        separate it from AUTH state machine, and make it as a standalone service
    
    IRQL = DISPATCH_LEVEL

    ==========================================================================
 */
VOID MlmeAuthCls2errAction(
    IN PMP_ADAPTER pAd, 
    IN PMP_PORT   pPort,
    IN PUCHAR pAddr) 
{
    HEADER_802_11 DeauthHdr;
    PUCHAR        pOutBuffer = NULL;
    NDIS_STATUS   NStatus;
    ULONG         FrameLen = 0;
    USHORT        Reason = REASON_CLS2ERR;
//  PMP_PORT  pPort = pAd->PortList[PORT_0];
    NStatus = MlmeAllocateMemory(pAd, &pOutBuffer);  //Get an unused nonpaged memory
    if (NStatus != NDIS_STATUS_SUCCESS) 
        return;

    DBGPRINT(RT_DEBUG_TRACE, ("AUTH - Class 2 error, Send DEAUTH frame...\n"));
    MgtMacHeaderInit(pAd,pPort, &DeauthHdr, SUBTYPE_DEAUTH, 0, pAddr, pAd->MlmeAux.Bssid);
    MakeOutgoingFrame(pOutBuffer,           &FrameLen, 
                      sizeof(HEADER_802_11),&DeauthHdr, 
                      2,                    &Reason, 
                      END_OF_ARGS);
    NdisCommonMiniportMMRequest(pAd, pOutBuffer, FrameLen);

    pAd->StaCfg.DeauthReason = Reason;
    COPY_MAC_ADDR(pAd->StaCfg.DeauthSta, pAddr);
}


