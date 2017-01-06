/*
 ***************************************************************************
 * Ralink Tech Inc.
 * 4F, No. 2 Technology 5th Rd.
 * Science-based Industrial Park
 * Hsin-chu, Taiwan, R.O.C.
 *
 * (c) Copyright 2002-2006, Ralink Technology, Inc.
 *
 * All rights reserved. Ralink's source code is an unpublished work and the
 * use of a copyright notice does not imply otherwise. This source code
 * contains confidential trade secret material of Ralink Tech. Any attemp
 * or participation in deciphering, decoding, reverse engineering or in any
 * way altering the source code is stricitly prohibited, unless the prior
 * written consent of Ralink Technology, Inc. is obtained.
 ***************************************************************************/

/****************************************************************************
    Abstract:

***************************************************************************/

//#include <stdio.h>

#include "MtConfig.h"

#define DRIVER_FILE         0x01100000

/* The definition in IEEE 802.11w/D10.0 - Table 7-32 Cipher suite selectors */
UCHAR       OUI_PMF_BIP_CIPHER[4]   = {0x00, 0x0F, 0xAC, 0x06};

/* The definition in IEEE 802.11w/D10.0 - Table 7-34 AKM suite selectors */
UCHAR       OUI_PMF_8021X_AKM[4]    = {0x00, 0x0F, 0xAC, 0x05};
UCHAR       OUI_PMF_PSK_AKM[4]      = {0x00, 0x0F, 0xAC, 0x06};

UCHAR       PMF_MMIE_BUFFER[18]     = {0x4C, 0x10,
                                       0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
                                       0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};

/*
========================================================================
Routine Description:

Arguments:

Return Value:

Note:

========================================================================
*/
VOID PmfStateMachineInit(
    IN  PMP_ADAPTER   pAd,
    IN  STATE_MACHINE   *S,
    OUT STATE_MACHINE_FUNC  Trans[])
{
    StateMachineInit(S, Trans, PMF_MAX_STATE, PMF_MAX_MSG, Drop, PMF_IDLE, PMF_MACHINE_BASE);

    //StateMachineSetAction(S, PMF_IDLE, PMF_MLME_SAQUERY_RSP, PmfConfirmAction);
    StateMachineSetAction(S, PMF_IDLE, PMF_MLME_SAQUERY_REQ, PmfSAQueryReqAction);
    StateMachineSetAction(S, PMF_IDLE, PMF_MLME_SAQUERY_RSP, PmfSAQueryRspAction);
    StateMachineSetAction(S, PMF_IDLE, PMF_PEER_SAQUERY_RSP, PmfPeerSAQueryRspAction);

    StateMachineSetAction(S, PMF_CONFIRM, PMF_MLME_SAQUERY_CONFIRM, PmfSAQueryConfirmAction);
    StateMachineSetAction(S, PMF_CONFIRM, PMF_PEER_SAQUERY_RSP, PmfPeerSAQueryRspAction);
    // initialize the timer (Move to MlmeInit)
    // init the SAQuery Request periodic timer

}

/*
    ========================================================================

    Routine Description:
        SAQuery Request timeout action, could be periodic

    Arguments:
        pAd Pointer to our adapter

    Return Value:
        None

    Note:

    ========================================================================
*/
VOID    PmfReqTimeoutActionTimerCallback(
    IN  PVOID   SystemSpecific1,
    IN  PVOID   FunctionContext,
    IN  PVOID   SystemSpecific2,
    IN  PVOID   SystemSpecific3)
{
    MP_ADAPTER *pAd = (MP_ADAPTER *)FunctionContext;
    PMP_PORT pPort = pAd->PortList[0];

    DBGPRINT(RT_DEBUG_TRACE, ("MFP: PmfReqTimeoutActionTimerCallback\n"));
    // sned SA Query Request frame
    PmfSendSAQuery(pAd, pPort, PMF_ACTION_SAQUERY_REQ, &pAd->StaCfg.PmfCfg.TransactionID);
}

/*
    ========================================================================

    Routine Description:
        SAQuery Request timeout action, could be periodic

    Arguments:
        pAd Pointer to our adapter

    Return Value:
        None

    Note:

    ========================================================================
*/
VOID    PmfConfirmTimeoutActionTimerCallback(
    IN  PVOID   SystemSpecific1,
    IN  PVOID   FunctionContext,
    IN  PVOID   SystemSpecific2,
    IN  PVOID   SystemSpecific3)
{
    PCHAR                       pOutBuffer = NULL;
    ULONG                       FrameLen = 0;
    BOOLEAN             TimerCancelled;
    ULONG                       Timeout = 0;
    PMP_PORT pPort = (PMP_PORT )FunctionContext;
    PMP_ADAPTER  pAd = pPort->pAd;

    DBGPRINT(RT_DEBUG_TRACE, ("MFP: PmfConfirmTimeoutActionTimerCallback ---> \n"));
    // Cancle SAQuery Timers

    PlatformCancelTimer(&pPort->StaCfg.PmfCfg.SAQueryTimer, &TimerCancelled);
    PlatformCancelTimer(&pPort->StaCfg.PmfCfg.SAQueryConfirmTimer, &TimerCancelled);

    //reset state to idle
    pPort->Mlme.PmfMachine.CurrState = PMF_IDLE;

    //SAQuery Request Time Out, disconnect with AP and reset state

    // Reset the desired BSSID list that set by the Ralink Vista UI with MTK_OID_N6_SET_DESIRED_BSSID_LIST.
    // In this way, other connection request is allowed again.
    pAd->PortList[PORT_0]->PortCfg.DesiredBSSIDList[0][0] = 0xFF;
    pAd->PortList[PORT_0]->PortCfg.DesiredBSSIDList[0][1] = 0xFF;
    pAd->PortList[PORT_0]->PortCfg.DesiredBSSIDList[0][2] = 0xFF;
    pAd->PortList[PORT_0]->PortCfg.DesiredBSSIDList[0][3] = 0xFF;
    pAd->PortList[PORT_0]->PortCfg.DesiredBSSIDList[0][4] = 0xFF;
    pAd->PortList[PORT_0]->PortCfg.DesiredBSSIDList[0][5] = 0xFF;
    pAd->PortList[PORT_0]->PortCfg.DesiredBSSIDCount = 1;
    pAd->PortList[PORT_0]->PortCfg.AcceptAnyBSSID = TRUE;

    // Reset to FALSE.
    pAd->StaCfg.bSetDesiredBSSIDListFromRalinkUI = FALSE;

    DBGPRINT(RT_DEBUG_TRACE, ("%s: Clear the desired BSSID list and reset the bSetDesiredBSSIDListFromRalinkUI.\n",
    __FUNCTION__));

    DBGPRINT(RT_DEBUG_TRACE, ("%s: MTK_OID_N6_SET_DISCONNECT_REQUEST\n", __FUNCTION__));

    //SAQuery Request Time Out, disconnect with AP
    //MP_SET_STATE(pPort, OP_STATE);
    MTEnqueueInternalCmd(pAd, pPort, MT_CMD_DISCONNECT_REQUEST, NULL, 0);

    DBGPRINT(RT_DEBUG_TRACE, ("MFP: <--- PmfConfirmTimeoutActionTimerCallback \n"));
}

/*
    ========================================================================

    Routine Description:
        Handle (Re)Associate request by SA Query Service

    Arguments:

    Return Value:
        None

    Note:

    ========================================================================
*/

VOID PmfSendSAQuery(
    IN PMP_ADAPTER pAd,
    IN PMP_PORT pPort,
    IN UCHAR Action,
    IN PUSHORT pTranscationID)
{
    HEADER_802_11   ActionHdr;
    PMF_SA_QUERY_ACTION SAQueryFrame;
    ULONG           FrameLen;
    PUCHAR          pOutBuffer = NULL;
    NDIS_STATUS     NStatus = NDIS_STATUS_FAILURE;


    DBGPRINT(RT_DEBUG_TRACE, ("MFP: PmfSendSAQuery ---> \n"));

    //Get an unused nonpaged memory
    NStatus = MlmeAllocateMemory(pAd, &pOutBuffer);  //Get an unused nonpaged memory
    if (NStatus != NDIS_STATUS_SUCCESS)
        return;

    DBGPRINT(RT_DEBUG_TRACE, ("MFP: Send %s \n", (Action == PMF_ACTION_SAQUERY_REQ)? "SA Query Request":"SA Query Response"));

    if(Action == PMF_ACTION_SAQUERY_REQ)
        *pTranscationID += 1; // Transcation plus 1 when SA Query Request

    // make the frame header
    MgtMacHeaderInit(pAd, pPort, &ActionHdr, SUBTYPE_ACTION, 0, pPort->PortCfg.Bssid, pPort->PortCfg.Bssid);

    // set to TYPE_MGMT
    ActionHdr.FC.Type = BTYPE_MGMT;

    //Make SA Query Request body
    SAQueryFrame.Category = CATEGORY_SAQUERY;
    SAQueryFrame.Action = Action;
    PlatformMoveMemory(&SAQueryFrame.TransactionID, pTranscationID, sizeof(USHORT));

    DBGPRINT(RT_DEBUG_ERROR, ("MFP: TransactionID %x \n", SAQueryFrame.TransactionID[0]));

    //pSAQueryFrame->Category and pSAQueryFrame->TransactionID are the same as the recv frame.

    MakeOutgoingFrame(pOutBuffer,                       &FrameLen,
                          LENGTH_802_11,                &ActionHdr,
                          sizeof(PMF_SA_QUERY_ACTION),  &SAQueryFrame,
                          END_OF_ARGS);

    NdisCommonMiniportMMRequest(pAd, pOutBuffer, FrameLen);

    DBGPRINT(RT_DEBUG_TRACE, ("MFP: <--- PmfSendSAQuery\n"));
}

/*
    ========================================================================

    Routine Description:
        Handle (Re)Associate request by SA Query Service

    Arguments:

    Return Value:
        None

    Note:

    ========================================================================
*/

VOID PmfSAQueryConfirmAction(
    IN PMP_ADAPTER pAd,
    IN MLME_QUEUE_ELEM *Elem)
{
    PMP_PORT pPort = pAd->PortList[FXXK_PORT_0];
    
    PlatformSetTimer(pPort, &pPort->StaCfg.PmfCfg.SAQueryConfirmTimer, SAQueryMaximumTimeout);  // set timer (3 s)
}

/*
    ========================================================================

    Routine Description:
        Handle (Re)Associate request by SA Query Service

    Arguments:

    Return Value:
        None

    Note:

    ========================================================================
*/

VOID PmfSAQueryReqAction(
    IN PMP_ADAPTER pAd,
    IN MLME_QUEUE_ELEM *Elem)
{
    PMP_PORT  pPort = pAd->PortList[Elem->PortNum];
    DBGPRINT(RT_DEBUG_ERROR, ("MFP: SAQueryReqAction ---> \n"));

    // SAQuery Check
    //if (!SAQuerySanity(pAd, Elem->Wcid, Elem->Msg, Elem->MsgLen))
        //return;

    PmfSendSAQuery( pAd, pPort, PMF_ACTION_SAQUERY_REQ, &pAd->StaCfg.PmfCfg.TransactionID);

    pPort->Mlme.PmfMachine.CurrState = PMF_CONFIRM; // set for periodicly reporting PLM

    PlatformSetTimer(pPort, &pPort->StaCfg.PmfCfg.SAQueryTimer, SAQueryRetryTimeout);

    MlmeEnqueue(pAd, pPort, STA_PMF_STATE_MACHINE, PMF_MLME_SAQUERY_CONFIRM, Elem->MsgLen, Elem->Msg);

    DBGPRINT(RT_DEBUG_TRACE, ("MFP: <--- SAQueryReqAction\n"));

}

/*
    ========================================================================

    Routine Description:
        Handle (Re)Associate request by SA Query Service

    Arguments:

    Return Value:
        None

    Note:

    ========================================================================
*/

VOID PmfSAQueryRspAction(
    IN PMP_ADAPTER pAd,
    IN MLME_QUEUE_ELEM *Elem)
{
    //HEADER_802_11   ActionHdr;
    PPMF_SA_QUERY_ACTION pSAQueryFrame;
    //ULONG         FrameLen;
    //PUCHAR            pOutBuffer = NULL;
    //NDIS_STATUS   NStatus = NDIS_STATUS_FAILURE;
    //USHORT          TransactionID;
    PMP_PORT  pPort = pAd->PortList[Elem->PortNum];


    pSAQueryFrame =  (PPMF_SA_QUERY_ACTION)&(Elem->Msg[LENGTH_802_11]);
    DBGPRINT(RT_DEBUG_ERROR, ("MFP: SAQueryRspAction ---> \n"));

    // SAQuery Check
    if (!SAQuerySanity(pAd, Elem->Wcid, Elem->Msg, Elem->MsgLen))
        return;

    //send SAQuery response only when no pending Associate request.
    if(pPort->Mlme.AssocMachine.CurrState != ASSOC_WAIT_RSP && pPort->Mlme.AssocMachine.CurrState != REASSOC_WAIT_RSP)
        PmfSendSAQuery( pAd, pPort, PMF_ACTION_SAQUERY_RSP, (PUSHORT)&pSAQueryFrame->TransactionID);
    else
        DBGPRINT(RT_DEBUG_ERROR, ("MFP: Current state is (RE)ASSOC_WAIT_RSP\n"));

    DBGPRINT(RT_DEBUG_TRACE, ("MFP: <--- SAQueryRspAction\n"));

}

/*
    ========================================================================

    Routine Description:
        Handle (Re)Associate request by SA Query Service

    Arguments:

    Return Value:
        None

    Note:

    ========================================================================
*/

VOID PmfPeerSAQueryRspAction(
    IN PMP_ADAPTER pAd,
    IN MLME_QUEUE_ELEM *Elem)
{
    BOOLEAN         TimerCancelled;
    PFRAME_802_11 pFrame = (PFRAME_802_11)Elem->Msg;
    PMP_PORT  pPort = pAd->PortList[Elem->PortNum];


    DBGPRINT(RT_DEBUG_ERROR, ("MFP: SAQuery Rsp Confirm ---> \n"));

    if (pPort->Mlme.PmfMachine.CurrState != PMF_CONFIRM)
    {
        DBGPRINT(RT_DEBUG_ERROR, ("MFP: SAQ Rsp TransactionID %x \n", pFrame->Octet[2]));
        DBGPRINT(RT_DEBUG_ERROR, ("MFP: Not in Confirm State, drop SAQuery Rsp\n"));
        return;
    }

    if(!PlatformEqualMemory(&pFrame->Octet[2], &pAd->StaCfg.PmfCfg.TransactionID, 2))
    {
        DBGPRINT(RT_DEBUG_ERROR, ("MFP: TranscationID not match, drop SAQuery Rsp %d\n", pAd->StaCfg.PmfCfg.TransactionID));
        return;
    }

    DBGPRINT(RT_DEBUG_ERROR, ("MFP: TransactionID %x \n", pFrame->Octet[2]));

    // SAQuery Check
    if (!SAQuerySanity(pAd, Elem->Wcid, Elem->Msg, Elem->MsgLen))
        return;


    // Reset PMF state
    PlatformCancelTimer(&pPort->StaCfg.PmfCfg.SAQueryTimer, &TimerCancelled);
    PlatformCancelTimer(&pPort->StaCfg.PmfCfg.SAQueryConfirmTimer, &TimerCancelled);

    pPort->Mlme.PmfMachine.CurrState = PMF_IDLE;

    DBGPRINT(RT_DEBUG_ERROR, ("MFP: <--- SAQuery Rsp Confirm \n"));

}

VOID PmfInjectPckets(
    IN PMP_ADAPTER pAd,
    IN PMP_PORT pPort)
{
    MLME_DISASSOC_REQ_STRUCT DisassocReq = {0};
    HEADER_802_11         FrameHdr;
    PCHAR                 pOutBuffer = NULL;
    ULONG                 FrameLen = 0;
    NDIS_STATUS           NStatus;
    //BOOLEAN               TimerCancelled;
    ULONG                 Timeout = 0;

    pAd->StaCfg.PmfCfg.PmfControl.InjectPacket = 0; // reset Inject packet flag

    if(!MFP_ON(pAd, pPort))
    {
        DBGPRINT(RT_DEBUG_TRACE, ("MFP: MFP not negotiated!!\n"));
        return;
    }

    NStatus = MlmeAllocateMemory(pAd, (PVOID*)&pOutBuffer);  //Get an unused nonpaged memory
    if (NStatus != NDIS_STATUS_SUCCESS)
    {
        DBGPRINT(RT_DEBUG_TRACE, ("MFP: PmfInjectPckets() allocate memory failed\n"));
        return;
    }

    if(pAd->StaCfg.PmfCfg.PmfControl.Disassoc || pAd->StaCfg.PmfCfg.PmfControl.Deauth)
    {

        MlmeCntDisassocParmFill(pAd, &DisassocReq, pPort->PortCfg.Bssid, REASON_DEAUTH_STA_LEAVING);
        MgtMacHeaderInit(pAd, pPort, &FrameHdr,
        ((pAd->StaCfg.PmfCfg.PmfControl.Disassoc == TRUE) ? SUBTYPE_DISASSOC : SUBTYPE_DEAUTH)
        , 0, DisassocReq.Addr, DisassocReq.Addr);   // patch peap ttls switching issue



        DBGPRINT(RT_DEBUG_TRACE, ("MFP - Send DISASSOC request[BSSID::%02x:%02x:%02x:%02x:%02x:%02x] (Reason=%d)\n",
                    DisassocReq.Addr[0], DisassocReq.Addr[1], DisassocReq.Addr[2],
                    DisassocReq.Addr[3], DisassocReq.Addr[4], DisassocReq.Addr[5], DisassocReq.Reason));

        DBGPRINT(RT_DEBUG_INFO, ("MFP - Send DISASSOC MFP_ON(pAd) = %d\n",MFP_ON(pAd, pPort)));

        MakeOutgoingFrame(pOutBuffer,           &FrameLen,
                          sizeof(HEADER_802_11),&FrameHdr,
                          2,                    &DisassocReq.Reason,
                          END_OF_ARGS);
        NdisCommonMiniportMMRequest(pAd, pOutBuffer, FrameLen);
    }
    else if(pAd->StaCfg.PmfCfg.PmfControl.Assoc )
    {
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
        UCHAR           WmeIe[9] = {IE_VENDOR_SPECIFIC, 0x07, 0x00, 0x50, 0xf2, 0x02, 0x00, 0x01, 0x00};
        UCHAR           CipherTmp[64];
        UCHAR           CipherTmpLen;
        //USHORT            ListenIntv;
        //USHORT            CapabilityInfo;
        ULONG           tmp, idx = 0;
        BOOLEAN         FoundPMK = FALSE;
        USHORT          VarIesOffset = 0;

        //USHORT            Status;

        MLME_ASSOC_REQ_STRUCT        AssocReq;

        if(pAd->StaCfg.PmfCfg.PmfControl.Assoc)
        {

            USHORT Alg,Seq, Status;

            Alg = pAd->MlmeAux.Alg;
            Seq = 1;
            Status = MLME_SUCCESS;

            FrameLen = 0;

            DBGPRINT(RT_DEBUG_TRACE, ("MFP - Send AUTH request seq#1 (Alg=%d)...\n", Alg));
            MgtMacHeaderInit(pAd, pPort, &FrameHdr, SUBTYPE_AUTH, 0, pPort->PortCfg.Bssid, pPort->PortCfg.Bssid);
            MakeOutgoingFrame(pOutBuffer,           &FrameLen,
                          sizeof(HEADER_802_11),&FrameHdr,
                          2,                    &Alg,
                          2,                    &Seq,
                          2,                    &Status,
                          END_OF_ARGS);

            NdisCommonMiniportMMRequest(pAd, pOutBuffer, FrameLen);
        }

        FrameLen = 0;

        MlmeCntAssocParmFill(pAd, &AssocReq, pAd->MlmeAux.Bssid, pAd->MlmeAux.CapabilityInfo,
                          ASSOC_TIMEOUT, pAd->StaCfg.DefaultListenCount);

        DBGPRINT(RT_DEBUG_TRACE, ("MFP - Send ASSOC request...\n"));
        MgtMacHeaderInit(pAd, pPort, &FrameHdr, SUBTYPE_ASSOC_REQ, 0, (PUCHAR)&AssocReq.Addr, (PUCHAR)&AssocReq.Addr);

        COPY_MAC_ADDR(pAd->MlmeAux.Bssid, AssocReq.Addr);

        // make frame, use bssid as the AP address??
        DBGPRINT(RT_DEBUG_TRACE, ("MFP - Send ASSOC request [BSSID::%02x:%02x:%02x:%02x:%02x:%02x]\n",
                    AssocReq.Addr[0], AssocReq.Addr[1], AssocReq.Addr[2],
                    AssocReq.Addr[3], AssocReq.Addr[4], AssocReq.Addr[5]));

        // Build basic frame first
        MakeOutgoingFrame(pOutBuffer,               &FrameLen,
                          sizeof(HEADER_802_11),    &FrameHdr,
                          2,                        &AssocReq.CapabilityInfo,
                          2,                        &AssocReq.ListenIntv,
                          1,                        &SsidIe,
                          1,                        &pAd->MlmeAux.SsidLen,
                          pAd->MlmeAux.SsidLen,     pAd->MlmeAux.Ssid,
                          1,                        &SupRateIe,
                          1,                        &pAd->MlmeAux.SupRateLen,
                          pAd->MlmeAux.SupRateLen,  pAd->MlmeAux.SupRate,
                          END_OF_ARGS);

        if (pAd->MlmeAux.ExtRateLen != 0)
        {
            MakeOutgoingFrame(pOutBuffer + FrameLen,    &tmp,
                              1,                        &ExtRateIe,
                              1,                        &pAd->MlmeAux.ExtRateLen,
                              pAd->MlmeAux.ExtRateLen,  pAd->MlmeAux.ExtRate,
                              END_OF_ARGS);
            FrameLen += tmp;
        }

        //
        // not including HT and RalinkIE for CCX Diag
        // CH14 only supports 11b and will not also include HT info
        //
        if ((!(pPort->CommonCfg.Ch14BOnly && (pPort->ScaningChannel == CHANNEL_14))))
        {
            // HT
            if ((pAd->MlmeAux.HtCapabilityLen > 0) && (pPort->CommonCfg.PhyMode >= PHY_11ABGN_MIXED))
            {
                ULONG TmpLen;
                UCHAR HtLen;
                UCHAR BROADCOM[4] = {0x0, 0x90, 0x4c, 0x33};
                if (pAd->StaActive.SupportedHtPhy.bPreNHt == TRUE)
                {
                    HtLen = SIZE_HT_CAP_IE + 4;
                    MakeOutgoingFrame(pOutBuffer + FrameLen,            &TmpLen,
                                  1,                                &WpaIe,
                                  1,                                &HtLen,
                                  4,                                &BROADCOM[0],
                                 pAd->MlmeAux.HtCapabilityLen,          &pAd->MlmeAux.HtCapability,
                                  END_OF_ARGS);
                }
                else
                {
                    MakeOutgoingFrame(pOutBuffer + FrameLen,            &TmpLen,
                                  1,                                &HtCapIe,
                                  1,                                &pAd->MlmeAux.HtCapabilityLen,
                                 pAd->MlmeAux.HtCapabilityLen,          &pAd->MlmeAux.HtCapability,
                                  END_OF_ARGS);
                }
                FrameLen += TmpLen;
                if ((pPort->ScaningChannel <= 14) && (pPort->CommonCfg.BACapability.field.b2040CoexistScanSup == 1))
                {
                    HtLen = sizeof(EXT_CAP_ELEMT);
                    MakeOutgoingFrame(pOutBuffer + FrameLen,            &tmp,
                                      1,                                &ExtHtCapIe,
                                      1,                                &HtLen,
                                      HtLen,                            &pPort->CommonCfg.ExtCapIE.DoubleWord, 
                                      END_OF_ARGS);

                    FrameLen += tmp;
                }
            }

            // add Ralink proprietary IE to inform AP this STA is going to use AGGREGATION or PIGGY-BACK+AGGREGATION
            // Case I: (Aggregation + Piggy-Back)
            // 1. user enable aggregation, AND
            // 2. Mac support piggy-back
            // 3. AP annouces it's PIGGY-BACK+AGGREGATION-capable in BEACON
            // Case II: (Aggregation)
            // 1. user enable aggregation, AND
            // 2. AP annouces it's AGGREGATION-capable in BEACON
            if (pPort->CommonCfg.bAggregationCapable)
            {
                if ((pPort->CommonCfg.bPiggyBackCapable) && ((pAd->MlmeAux.APRalinkIe & 0x00000003) == 3))
                {
                    ULONG TmpLen;
                    UCHAR RalinkIe[9] = {IE_VENDOR_SPECIFIC, 7, 0x00, 0x0c, 0x43, 0x03, 0x00, 0x00, 0x00};
                    MakeOutgoingFrame(pOutBuffer+FrameLen,           &TmpLen,
                                      9,                             RalinkIe,
                                      END_OF_ARGS);
                    FrameLen += TmpLen;
                }
                else if (pAd->MlmeAux.APRalinkIe & 0x00000001)
                {
                    ULONG TmpLen;
                    UCHAR RalinkIe[9] = {IE_VENDOR_SPECIFIC, 7, 0x00, 0x0c, 0x43, 0x01, 0x00, 0x00, 0x00};
                    MakeOutgoingFrame(pOutBuffer+FrameLen,           &TmpLen,
                                      9,                             RalinkIe,
                                      END_OF_ARGS);
                    FrameLen += TmpLen;
                }
                else
                {
                    ULONG TmpLen;
                    UCHAR RalinkIe[9] = {IE_VENDOR_SPECIFIC, 7, 0x00, 0x0c, 0x43, 0x00, 0x00, 0x00, 0x00};
                    MakeOutgoingFrame(pOutBuffer+FrameLen,           &TmpLen,
                                      9,                             RalinkIe,
                                      END_OF_ARGS);
                    FrameLen += TmpLen;
                }
            }
            else
            {
                ULONG TmpLen;
                UCHAR RalinkIe[9] = {IE_VENDOR_SPECIFIC, 7, 0x00, 0x0c, 0x43, 0x06, 0x00, 0x00, 0x00};
                MakeOutgoingFrame(pOutBuffer+FrameLen,       &TmpLen,
                                  9,                         RalinkIe,
                                  END_OF_ARGS);
                FrameLen += TmpLen;
            }
        }

        if (pAd->MlmeAux.APEdcaParm.bValid)
        {
            if (pPort->CommonCfg.bAPSDCapable && pAd->MlmeAux.APEdcaParm.bAPSDCapable)
            {
                QBSS_STA_INFO_PARM QosInfo;

                PlatformZeroMemory(&QosInfo, sizeof(QBSS_STA_INFO_PARM));
                QosInfo.UAPSD_AC_BE = pPort->CommonCfg.bAPSDAC_BE;
                QosInfo.UAPSD_AC_BK = pPort->CommonCfg.bAPSDAC_BK;
                QosInfo.UAPSD_AC_VI = pPort->CommonCfg.bAPSDAC_VI;
                QosInfo.UAPSD_AC_VO = pPort->CommonCfg.bAPSDAC_VO;
                QosInfo.MaxSPLength = pPort->CommonCfg.MaxSPLength;
                WmeIe[8] |= *(PUCHAR)&QosInfo;
            }
            else
            {
                // The Parameter Set Count is set to ¡§0¡¨ in the association request frames
                // WmeIe[8] |= (pAd->MlmeAux.APEdcaParm.EdcaUpdateCount & 0x0f);
            }

            MakeOutgoingFrame(pOutBuffer + FrameLen,    &tmp,
                              9,                        &WmeIe[0],
                              END_OF_ARGS);
            FrameLen += tmp;
        }
        //
        // The power capability IE
        //
        if (pAd->StaCfg.MDSMCtrl.dot11SpectrumManagementRequired == TRUE)
        {
            POWER_CAPABILITY_IE PowerCapabilityIE = {0};

            MDSMCreatePowerCapabilityIE(pAd, &PowerCapabilityIE);

            MakeOutgoingFrame(pOutBuffer + FrameLen, &tmp,
                                                sizeof(POWER_CAPABILITY_IE), &PowerCapabilityIE,
                                                END_OF_ARGS);

            FrameLen += tmp;
        }

        // init the CipherTmp
        CipherTmpLen = 0;
        PlatformZeroMemory(CipherTmp, sizeof(CipherTmp));

        // For WPA2 / WPA2-PSK
        if (((pPort->PortCfg.AuthMode == Ralink802_11AuthModeWPA2) ||
                 (pPort->PortCfg.AuthMode == Ralink802_11AuthModeWPA2PSK)) &&
                 (pPort->StaCfg.WscControl.WscState == WSC_STATE_OFF)) // WPS ignore RSNIE
        {
            // Copy WPA2 template to buffer
            CipherTmpLen = sizeof(CipherWpa2Template);
            PlatformMoveMemory(CipherTmp, CipherWpa2Template, CipherTmpLen);

            // Modify Group cipher, WPA2+WEP mixed mode is possible
            switch (pPort->PortCfg.GroupCipher)
            {
                case Ralink802_11Encryption1Enabled:  // WEP-40, WEP-104
                    CipherTmp[7] = ((pPort->PortCfg.MixedModeGroupCipher == Cipher_Type_WEP40) ? 0x1 : 0x5);
                    break;
                case Ralink802_11Encryption2Enabled:  // TKIP
                    CipherTmp[7] = 0x2;
                    break;
                default: // otherwise AES
                    CipherTmp[7] = 0x4;
                    break;
            }

            // Modify Pairwise cipher
            CipherTmp[13] = ((pPort->PortCfg.PairCipher == Ralink802_11Encryption2Enabled) ? 0x2 : 0x4);
            // Modify AKM
            CipherTmp[19] = ((pPort->PortCfg.AuthMode == Ralink802_11AuthModeWPA2) ? 0x1 : 0x2);

            // Check for WPA PMK cache list
            if (pPort->PortCfg.AuthMode == Ralink802_11AuthModeWPA2)
            {
                // Search chched PMKID, append it if existed
                for (idx = 0; idx < pAd->StaCfg.PMKIDCount; idx++)
                {
                    if (PlatformEqualMemory(pAd->MlmeAux.Ssid, &pAd->StaCfg.PMKIDList[idx].BSSID, 6))
                    {
                        FoundPMK = TRUE;
                        break;
                    }
                }
            }

                if (FoundPMK)
                {
                    // Update length within RSN IE
                    CipherTmp[1] += 18;
                    // Set PMK number
                    *(PUSHORT) &CipherTmp[CipherTmpLen] = 1;
                    PlatformMoveMemory(&CipherTmp[CipherTmpLen + 2], &pAd->StaCfg.PMKIDList[idx].PMKID, 16);
                    CipherTmpLen += 18;
                }
                else
                {
                    //add PMKID list as 0 for fit IE formate
                    CipherTmp[1] += 2;
                    *(UNALIGNED PUSHORT) &CipherTmp[CipherTmpLen] = 0;
                    CipherTmpLen += 2;
                }


                    //add BIP cipher suite OUI
                    CipherTmp[1] += 4;
                    PlatformMoveMemory(&CipherTmp[CipherTmpLen], OUI_PMF_BIP_CIPHER, 4);
                    CipherTmpLen += 4;


                    DBGPRINT(RT_DEBUG_INFO, ("MFP: Outgoing RSNCapability: 0x%x\n",pAd->StaCfg.PmfCfg.RSNCapability));
                    // Modify AKM when SHA256 was used
                    if(pAd->StaCfg.PmfCfg.PmfControl.AKMwSHA256 || pPort->PortCfg.bAKMwSHA256)
                        {
                            CipherTmp[19] = ((pPort->PortCfg.AuthMode == Ralink802_11AuthModeWPA2) ? 0x5 : 0x6);
                            DBGPRINT(RT_DEBUG_TRACE, ("MFP: AP SHA256 = 0x%x, STA SHA256 = 0x%x\n",pPort->PortCfg.bAKMwSHA256, pAd->StaCfg.PmfCfg.PmfControl.AKMwSHA256));
                        }
                    CipherTmp[20] = (UCHAR)pAd->StaCfg.PmfCfg.RSNCapability;
                    //CipherTmp[21] = (UCHAR)pAd->StaCfg.PmfCfg.RSNCapability >> 8;

            // Make outgoing frame
            MakeOutgoingFrame(pOutBuffer + FrameLen,    &tmp,
                              CipherTmpLen,         &CipherTmp[0],
                              END_OF_ARGS);

            FrameLen += tmp;
        }

        NdisCommonMiniportMMRequest(pAd, pOutBuffer, FrameLen);
    }
    else if(pAd->StaCfg.PmfCfg.PmfControl.Reassoc)
    {
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

        UCHAR           WmeIe[9] = {IE_VENDOR_SPECIFIC, 0x07, 0x00, 0x50, 0xf2, 0x02, 0x00, 0x01, 0x00};
        UCHAR           CipherTmp[64];
        UCHAR           CipherTmpLen;
        //USHORT            ListenIntv;
        //USHORT            CapabilityInfo;
        ULONG           tmp, idx = 0;
        BOOLEAN         FoundPMK = FALSE;

        MLME_ASSOC_REQ_STRUCT        AssocReq;

        FrameLen = 0;

        MlmeCntAssocParmFill(pAd, &AssocReq, pAd->MlmeAux.Bssid, pAd->MlmeAux.CapabilityInfo,
                          ASSOC_TIMEOUT, pAd->StaCfg.DefaultListenCount);

        // make frame, use bssid as the AP address??
        DBGPRINT(RT_DEBUG_TRACE, ("MFP - Send RE-ASSOC request...\n"));

        MgtMacHeaderInit(pAd, pPort, &FrameHdr, SUBTYPE_REASSOC_REQ, 0, (PUCHAR)&AssocReq.Addr, (PUCHAR)&AssocReq.Addr);
        MakeOutgoingFrame(pOutBuffer,               &FrameLen,
                          sizeof(HEADER_802_11),    &FrameHdr,
                          2,                        &AssocReq.CapabilityInfo,
                          2,                        &AssocReq.ListenIntv,
                          MAC_ADDR_LEN,             pAd->MlmeAux.Bssid,
                          1,                        &SsidIe,
                          1,                        &pAd->MlmeAux.SsidLen,
                          pAd->MlmeAux.SsidLen,     pAd->MlmeAux.Ssid,
                          1,                        &SupRateIe,
                          1,                        &pAd->MlmeAux.SupRateLen,
                          pAd->MlmeAux.SupRateLen,  pAd->MlmeAux.SupRate,
                          END_OF_ARGS);

        if (pAd->MlmeAux.ExtRateLen != 0)
        {
            MakeOutgoingFrame(pOutBuffer + FrameLen,        &tmp,
                              1,                            &ExtRateIe,
                              1,                            &pAd->MlmeAux.ExtRateLen,
                              pAd->MlmeAux.ExtRateLen,      pAd->MlmeAux.ExtRate,
                              END_OF_ARGS);
            FrameLen += tmp;
        }

        if (pAd->MlmeAux.APEdcaParm.bValid)
        {
            if (pPort->CommonCfg.bAPSDCapable && pAd->MlmeAux.APEdcaParm.bAPSDCapable)
            {
                QBSS_STA_INFO_PARM QosInfo;

                PlatformZeroMemory(&QosInfo, sizeof(QBSS_STA_INFO_PARM));
                QosInfo.UAPSD_AC_BE = pPort->CommonCfg.bAPSDAC_BE;
                QosInfo.UAPSD_AC_BK = pPort->CommonCfg.bAPSDAC_BK;
                QosInfo.UAPSD_AC_VI = pPort->CommonCfg.bAPSDAC_VI;
                QosInfo.UAPSD_AC_VO = pPort->CommonCfg.bAPSDAC_VO;
                QosInfo.MaxSPLength = pPort->CommonCfg.MaxSPLength;
                WmeIe[8] |= *(PUCHAR)&QosInfo;
            }
            else
            {
                // The Parameter Set Count is set to ¡§0¡¨ in the association request frames
                // WmeIe[8] |= (pAd->MlmeAux.APEdcaParm.EdcaUpdateCount & 0x0f);
            }

            MakeOutgoingFrame(pOutBuffer + FrameLen,    &tmp,
                              9,                        &WmeIe[0],
                              END_OF_ARGS);
            FrameLen += tmp;
        }

        if (pPort->CommonCfg.Ch14BOnly && (pPort->ScaningChannel == CHANNEL_14))
        {
            //CH14 only supports 11b and will not carry APSD info
            ;
        }
        else
        {
            // HT
            if ((pAd->MlmeAux.HtCapabilityLen > 0) && (pPort->CommonCfg.PhyMode >= PHY_11ABGN_MIXED))
            {
                ULONG TmpLen;
                UCHAR HtLen;
                UCHAR BROADCOM[4] = {0x0, 0x90, 0x4c, 0x33};
                if (pAd->StaActive.SupportedHtPhy.bPreNHt == TRUE)
                {
                    HtLen = SIZE_HT_CAP_IE + 4;
                    MakeOutgoingFrame(pOutBuffer + FrameLen,            &TmpLen,
                                  1,                                &WpaIe,
                                  1,                                &HtLen,
                                  4,                                &BROADCOM[0],
                                 pAd->MlmeAux.HtCapabilityLen,          &pAd->MlmeAux.HtCapability,
                                  END_OF_ARGS);
                }
                else
                {
                    MakeOutgoingFrame(pOutBuffer + FrameLen,            &TmpLen,
                                  1,                                &HtCapIe,
                                  1,                                &pAd->MlmeAux.HtCapabilityLen,
                                 pAd->MlmeAux.HtCapabilityLen,          &pAd->MlmeAux.HtCapability,
                                  END_OF_ARGS);
                }
                FrameLen += TmpLen;

                if ((pPort->ScaningChannel <= 14) && (pPort->CommonCfg.BACapability.field.b2040CoexistScanSup == 1))
                {
                    HtLen = sizeof(EXT_CAP_ELEMT);
                    MakeOutgoingFrame(pOutBuffer + FrameLen,            &tmp,
                                      1,                                &ExtHtCapIe,
                                      1,                                &HtLen,
                                      HtLen,                            &pPort->CommonCfg.ExtCapIE.DoubleWord, 
                                      END_OF_ARGS);

                    FrameLen += tmp;
                }
            }
        }
        // add Ralink proprietary IE to inform AP this STA is going to use AGGREGATION or PIGGY-BACK+AGGREGATION
        // Case I: (Aggregation + Piggy-Back)
        // 1. user enable aggregation, AND
        // 2. Mac support piggy-back
        // 3. AP annouces it's PIGGY-BACK+AGGREGATION-capable in BEACON
        // Case II: (Aggregation)
        // 1. user enable aggregation, AND
        // 2. AP annouces it's AGGREGATION-capable in BEACON
        if (pPort->CommonCfg.bAggregationCapable)
        {
            if ((pPort->CommonCfg.bPiggyBackCapable) && ((pAd->MlmeAux.APRalinkIe & 0x00000003) == 3))
            {
                ULONG TmpLen;
                UCHAR RalinkIe[9] = {IE_VENDOR_SPECIFIC, 7, 0x00, 0x0c, 0x43, 0x03, 0x00, 0x00, 0x00};
                MakeOutgoingFrame(pOutBuffer+FrameLen,           &TmpLen,
                                  9,                             RalinkIe,
                                  END_OF_ARGS);
                FrameLen += TmpLen;
            }
            else if (pAd->MlmeAux.APRalinkIe & 0x00000001)
            {
                ULONG TmpLen;
                UCHAR RalinkIe[9] = {IE_VENDOR_SPECIFIC, 7, 0x00, 0x0c, 0x43, 0x01, 0x00, 0x00, 0x00};
                MakeOutgoingFrame(pOutBuffer+FrameLen,           &TmpLen,
                                  9,                             RalinkIe,
                                  END_OF_ARGS);
                FrameLen += TmpLen;
            }
            else
            {
                ULONG TmpLen;
                UCHAR RalinkIe[9] = {IE_VENDOR_SPECIFIC, 7, 0x00, 0x0c, 0x43, 0x00, 0x00, 0x00, 0x00};
                MakeOutgoingFrame(pOutBuffer+FrameLen,           &TmpLen,
                                  9,                             RalinkIe,
                                  END_OF_ARGS);
                FrameLen += TmpLen;
            }
        }
        else
        {
            ULONG TmpLen;
            UCHAR RalinkIe[9] = {IE_VENDOR_SPECIFIC, 7, 0x00, 0x0c, 0x43, 0x04, 0x00, 0x00, 0x00};
            MakeOutgoingFrame(pOutBuffer+FrameLen,       &TmpLen,
                              9,                         RalinkIe,
                              END_OF_ARGS);
            FrameLen += TmpLen;
        }

        //
        // The power capability IE
        //
        if (pAd->StaCfg.MDSMCtrl.dot11SpectrumManagementRequired == TRUE)
        {
            POWER_CAPABILITY_IE PowerCapabilityIE = {0};

            MDSMCreatePowerCapabilityIE(pAd, &PowerCapabilityIE);

            MakeOutgoingFrame(pOutBuffer + FrameLen, &tmp,
                                                sizeof(POWER_CAPABILITY_IE), &PowerCapabilityIE,
                                                END_OF_ARGS);

            FrameLen += tmp;
        }

        // init the CipherTmp
        CipherTmpLen = 0;
        PlatformZeroMemory(CipherTmp, sizeof(CipherTmp));

        // For WPA2 / WPA2-PSK
        if (((pPort->PortCfg.AuthMode == Ralink802_11AuthModeWPA2) ||
                 (pPort->PortCfg.AuthMode == Ralink802_11AuthModeWPA2PSK)) &&
                 (pPort->StaCfg.WscControl.WscState == WSC_STATE_OFF)) // WPS ignore RSNIE
        {
            //ULONG TmpLen;
            // Copy WPA2 template to buffer
            CipherTmpLen = sizeof(CipherWpa2Template);
            PlatformMoveMemory(CipherTmp, CipherWpa2Template, CipherTmpLen);

            // Modify Group cipher, WPA2+WEP mixed mode is possible
            switch (pPort->PortCfg.GroupCipher)
            {
                case Ralink802_11Encryption1Enabled:  // WEP-40, WEP-104
                    CipherTmp[7] = ((pPort->PortCfg.MixedModeGroupCipher == Cipher_Type_WEP40) ? 0x1 : 0x5);
                    break;
                case Ralink802_11Encryption2Enabled:  // TKIP
                    CipherTmp[7] = 0x2;
                    break;
                default: // otherwise AES
                    CipherTmp[7] = 0x4;
                    break;
            }

            // Modify Pairwise cipher
            CipherTmp[13] = ((pPort->PortCfg.PairCipher == Ralink802_11Encryption2Enabled) ? 0x2 : 0x4);
            // Modify AKM
            CipherTmp[19] = ((pPort->PortCfg.AuthMode == Ralink802_11AuthModeWPA2) ? 0x1 : 0x2);

            // Check for WPA PMK cache list
            if (pPort->PortCfg.AuthMode == Ralink802_11AuthModeWPA2)
            {
                // Search chched PMKID, append it if existed
                for (idx = 0; idx < pAd->StaCfg.PMKIDCount; idx++)
                {
                    if (PlatformEqualMemory(pAd->MlmeAux.Ssid, &pAd->StaCfg.PMKIDList[idx].BSSID, 6))
                    {
                        FoundPMK = TRUE;
                        break;
                    }
                }
            }

                if (FoundPMK)
                {
                    // Update length within RSN IE
                    CipherTmp[1] += 18;
                    // Set PMK number
                    *(PUSHORT) &CipherTmp[CipherTmpLen] = 1;
                    PlatformMoveMemory(&CipherTmp[CipherTmpLen + 2], &pAd->StaCfg.PMKIDList[idx].PMKID, 16);
                    CipherTmpLen += 18;
                }
                else
                {
                    //add PMKID list as 0 for fit IE formate
                    CipherTmp[1] += 2;
                    *(UNALIGNED PUSHORT) &CipherTmp[CipherTmpLen] = 0;
                    CipherTmpLen += 2;
                }


                    //add BIP cipher suite OUI
                    CipherTmp[1] += 4;
                    PlatformMoveMemory(&CipherTmp[CipherTmpLen], OUI_PMF_BIP_CIPHER, 4);
                    CipherTmpLen += 4;


                    DBGPRINT(RT_DEBUG_INFO, ("MFP: Outgoing RSNCapability: 0x%x\n",pAd->StaCfg.PmfCfg.RSNCapability));
                    // Modify AKM when SHA256 was used
                    if(pAd->StaCfg.PmfCfg.PmfControl.AKMwSHA256 || pPort->PortCfg.bAKMwSHA256)
                        {
                            CipherTmp[19] = ((pPort->PortCfg.AuthMode == Ralink802_11AuthModeWPA2) ? 0x5 : 0x6);
                            DBGPRINT(RT_DEBUG_TRACE, ("MFP: AP SHA256 = 0x%x, STA SHA256 = 0x%x\n",pPort->PortCfg.bAKMwSHA256, pAd->StaCfg.PmfCfg.PmfControl.AKMwSHA256));
                        }
                    CipherTmp[20] = (UCHAR)pAd->StaCfg.PmfCfg.RSNCapability;
                    //CipherTmp[21] = (UCHAR)pAd->StaCfg.PmfCfg.RSNCapability >> 8;

            // Make outgoing frame
            MakeOutgoingFrame(pOutBuffer + FrameLen,    &tmp,
                              CipherTmpLen,         &CipherTmp[0],
                              END_OF_ARGS);

            FrameLen += tmp;

        }

        NdisCommonMiniportMMRequest(pAd, pOutBuffer, FrameLen);

    }
    else if(pAd->StaCfg.PmfCfg.PmfControl.SAQReq)
    {
        DBGPRINT(RT_DEBUG_TRACE, ("MFP: Send SA Query Request!!\n"));
        PmfSendSAQuery(pAd, pPort, PMF_ACTION_SAQUERY_REQ, &pAd->StaCfg.PmfCfg.TransactionID);
    }

    pAd->StaCfg.PmfCfg.PmfControl.Unprotected = 0; // reset Flag here or driver will always send unprotected frames.
    pAd->StaCfg.PmfCfg.PmfControl.Bogus = 0; //reset Flag here or driver will always send bogus frames

}

/*
    ========================================================================

    Routine Description:
        Construct BIP AAD header.

    Arguments:

    Return Value:

    Note:
        It's described in IEEE Std 802.11w d8.0.
        The AAD is constructed from the MPDU header.

    ========================================================================
*/
VOID PmfConstructBIPAad(
    IN  PUCHAR          pHdr,
    OUT UCHAR           *aad_hdr)
{
    UINT8   aad_len = 0;

    /*  Frame control -
        Retry bit (bit 11) masked to 0
        PwrMgt bit (bit 12) masked to 0
        MoreData bit (bit 13) masked to 0 */
    aad_hdr[0] = (*pHdr);
    aad_hdr[1] = (*(pHdr + 1)) & 0xc7;
    aad_len = 2;

    /* Append Addr 1, 2 & 3 */
    PlatformMoveMemory(&aad_hdr[aad_len], pHdr + 4, 3 * MAC_ADDR_LEN);
    aad_len += (3 * MAC_ADDR_LEN);

}

/*
    ========================================================================
    Routine Description:

    Arguments:

    Return Value:

    Note:
        Compute AES-128-CMAC over the concatenation of
        (AAD || Management Frame Body including MMIE),
        and insert the 64-bit output into the MMIE MIC field.

    ========================================================================
*/
BOOLEAN PmfCalculateBIPMIC(
    IN  PMP_ADAPTER   pAd,
    IN  PUCHAR          pAadHdr,
    IN  PUCHAR          pFrameBuf,
    IN  UINT32          FrameLen,
    IN  PUCHAR          pKey,
    OUT PUCHAR          pBipMic)
{
    UCHAR   *m_buf;
    UINT32  total_len;
    UCHAR   cmac_output[16];
    UINT    mlen = LEN_AES_KEY;

    /* Allocate memory for MIC calculation */
    MlmeAllocateMemory(pAd, (PUCHAR *)&m_buf);
    if (m_buf == NULL)
    {
        DBGPRINT(RT_DEBUG_ERROR, ("%s : out of resource.\n", __FUNCTION__));
        return FALSE;
    }

    /* Initialize the buffer */
    PlatformZeroMemory(m_buf, MGMT_DMA_BUFFER_SIZE);

    /* Construct the concatenation */
    PlatformMoveMemory(m_buf, pAadHdr, LEN_PMF_BIP_AAD_HDR);
    total_len = LEN_PMF_BIP_AAD_HDR;

    /* Append the Mgmt frame into the concatenation */
    PlatformMoveMemory(&m_buf[total_len], pFrameBuf, FrameLen);
    total_len += FrameLen;

    /* Compute AES-128-CMAC over the concatenation */
    AES_128_CMAC(pKey, m_buf, total_len, cmac_output);

    /* Truncate the first 64-bits */
    PlatformMoveMemory(pBipMic, cmac_output, LEN_PMF_BIP_MIC);

    MlmeFreeMemory(pAd, m_buf);

    return TRUE;

}

/*
    ========================================================================

    Routine Description:
        Derive IGTK randomly
        IGTK, a hierarchy consisting of a single key to provide integrity
        protection for broadcast and multicast Robust Management frames

    Arguments:

    Return Value:

    Note:
        It's defined in IEEE 802.11w-D10.0 8.5.1.3a

    ========================================================================
*/
VOID PmfDeriveIGTK(
    IN  PMP_ADAPTER   pAd,
    OUT UCHAR           *output)
{
    INT     i;

    for(i = 0; i < LEN_TK; i++)
        output[i] = RandomByte(pAd);
}


/*
    ========================================================================

    Routine Description:
        Insert MMIE.

    Arguments:

    Return Value:

    Note:
        It's defined in IEEE 802.11w-D8.0 7.3.2.55

    ========================================================================
*/
VOID PmfInsertMMIE(
    IN  PMP_ADAPTER   pAd,
    IN  UINT8           keyIdx,
    IN  PUINT8          pPn,
    IN  PUCHAR          pFrameBuf,
    OUT PULONG          pFrameLen)
{
    ULONG       TempLen;
    UINT8       mmie_len;
    PMF_MMIE    pmf_mmie;
    UCHAR       MMIE = IE_PMF_MMIE;

    mmie_len = LEN_PMF_MMIE;
    PlatformZeroMemory(&pmf_mmie, LEN_PMF_MMIE);

    /*  Bits 0-11 define a value in the range 0-4095.
        Bits 12 - 15 are reserved and set to 0 on transmission and ignored on reception. 
        The IGTK Key ID is either 4 or 5. The remaining Key IDs are reserved. */
    pmf_mmie.KeyID[0] = keyIdx;

    /* Fill in the IPN field */
    PlatformMoveMemory(pmf_mmie.IPN, pPn, LEN_PN);
    
    MakeOutgoingFrame(pFrameBuf,        &TempLen,
                      1,                &MMIE,
                      1,                &mmie_len,
                      mmie_len,         &pmf_mmie,
                      END_OF_ARGS);

    *pFrameLen = *pFrameLen + TempLen;

    return; 
}

/*
    ========================================================================
    
    Routine Description:
        Extract IGTK KDE.

    Arguments:
        
    Return Value:

    Note:       
        
    ========================================================================
*/
BOOLEAN PmfExtractIGTKKDE(
    IN  PMP_ADAPTER       pAd,
    IN  PUCHAR              pBuf,
    IN  INT                 buf_len)
{
    PPMF_IGTK_KDE   igtk_kde_ptr;
    UINT8           idx = 0;
    UINT8           offset = 0;
    PPMF_CFG        pPmfCfg = NULL;


    if (pPmfCfg == NULL)
        return FALSE;
    
    igtk_kde_ptr = (PPMF_IGTK_KDE) pBuf;
    pPmfCfg->IGTKKeyIdx = igtk_kde_ptr->KeyID[0];
    if (pPmfCfg->IGTKKeyIdx == 5)
        idx = 1;
    offset += 2;

    PlatformMoveMemory(&pPmfCfg->IPN[idx][0], igtk_kde_ptr->IPN, LEN_WPA_TSC);  
    offset += LEN_WPA_TSC;

    if ((buf_len - offset) == LEN_AES_GTK)
    {
        PlatformMoveMemory(&pPmfCfg->IGTK[idx][0], igtk_kde_ptr->IGTK, LEN_AES_GTK);
    }
    else
    {
        DBGPRINT(RT_DEBUG_ERROR, ("%s : the IGTK length(%d) is invalid\n", 
                                 __FUNCTION__, (buf_len - offset)));    
        return FALSE;
    }
    
    DBGPRINT(RT_DEBUG_TRACE, ("%s : IGTK_Key_ID=%d\n", 
                                __FUNCTION__, pPmfCfg->IGTKKeyIdx));    
    return TRUE;
}

INT PmfExtractBIPAction(
    IN      PMP_ADAPTER   pAd,
    IN OUT  PUCHAR          pMgmtFrame,
    IN      UINT            mgmt_len)
{
    PPMF_CFG    pPmfCfg = NULL;
    PPMF_MMIE   pMMIE;
    INT         idx = 0;
    PUCHAR      pKey = NULL;
    UCHAR       aad_hdr[LEN_PMF_BIP_AAD_HDR];
    UCHAR       rcvd_mic[LEN_PMF_BIP_MIC];
    UCHAR       cal_mic[LEN_PMF_BIP_MIC];
    UINT32      body_len = mgmt_len - LENGTH_802_11;    

    /* Sanity check the total frame body length */
    DBGPRINT(RT_DEBUG_ERROR, ("%s : the total length(%d)\n", 
                                    __FUNCTION__, body_len));   
    if (body_len < (2 + LEN_PMF_MMIE))
    {
        DBGPRINT(RT_DEBUG_ERROR, ("%s : the total length(%d) is too short\n", 
                                    __FUNCTION__, body_len));   
        return PMF_EXTRACT_BIP_FAILURE; 
    }

    pPmfCfg = &pAd->StaCfg.PmfCfg;

    /* Check the PMF capable */
    if (pPmfCfg == NULL)
    {
        DBGPRINT(RT_DEBUG_ERROR, ("%s : No related PMF configuation\n", __FUNCTION__)); 
        return PMF_EXTRACT_BIP_FAILURE;
    }

    if (pPmfCfg && pPmfCfg->MFPC == FALSE)
    {
        DBGPRINT(RT_DEBUG_ERROR, ("%s : PMF is disabled \n", __FUNCTION__));    
        return PMF_EXTRACT_BIP_FAILURE;
    }

    /* Pointer to the position of MMIE */
    pMMIE = (PPMF_MMIE)(pMgmtFrame + (mgmt_len - LEN_PMF_MMIE));


    /*  Select the IGTK currently active for transmission of frames to 
        the intended group of recipients and construct the MMIE (see 7.3.2.55) 
        with the MIC field masked to zero and the KeyID field set to the
        corresponding IGTK KeyID value. */
    if (pMMIE->KeyID[0] == 5)
        idx = 1;    
    pKey = &pPmfCfg->IGTK[idx][0];

    /* check the IPN. If valid, record it */
    // Todo
    
    /* store the MIC value of the received frame */
    PlatformMoveMemory(rcvd_mic, pMMIE->MIC, LEN_PMF_BIP_MIC);
    PlatformZeroMemory(pMMIE->MIC, LEN_PMF_BIP_MIC);

    /* Compute AAD  */
    PmfConstructBIPAad((PUCHAR)pMgmtFrame, aad_hdr);

    /* Calculate BIP MIC */
    PmfCalculateBIPMIC(pAd, aad_hdr, 
                        pMgmtFrame + LENGTH_802_11, //Mgmt body include MMIE
                        body_len, pKey, cal_mic);

    if (!PlatformEqualMemory(rcvd_mic, cal_mic, LEN_PMF_BIP_MIC))
    {
        DBGPRINT(RT_DEBUG_ERROR, ("%s : MIC Different !\n", __FUNCTION__));
        DBGPRINT(RT_DEBUG_ERROR, ("rcvd_mic\n"));
        DumpFrame((PUCHAR)rcvd_mic, 8);
        DBGPRINT(RT_DEBUG_ERROR, ("cal_mic\n"));
        DumpFrame((PUCHAR)cal_mic, 8);
        return PMF_EXTRACT_BIP_FAILURE; 
    }

    return PMF_STATUS_SUCCESS;
}

