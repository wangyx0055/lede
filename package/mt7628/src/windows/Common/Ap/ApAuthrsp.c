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
    auth_rsp.c
 
    Abstract:
    Handle auth/de-auth requests from WSTA
 
    Revision History:
    Who         When          What
    --------    ----------    ----------------------------------------------
    John Chang  08-04-2003    created for 11g soft-AP
 */

#include "MtConfig.h"

#define DRIVER_FILE         0x00400000
/*
    ==========================================================================
    Description:
        authentication state machine init procedure
    Parameters:
        Sm - the state machine
    Note:
        the state machine looks like the following 
        
                                        AP_AUTH_RSP_IDLE                   
    APMT2_AUTH_CHALLENGE_TIMEOUT      auth_rsp_challenge_timeout_action    
    APMT2_PEER_AUTH_ODD               peer_auth_at_auth_rsp_idle_action 
    APMT2_PEER_DEAUTH                 peer_deauth_action         
    ==========================================================================
 */
VOID ApMlmeAuthRspStateMachineInit(
    IN PMP_ADAPTER pAd, 
    IN PSTATE_MACHINE Sm, 
    IN STATE_MACHINE_FUNC Trans[]) 
{
    LARGE_INTEGER        NOW;

    StateMachineInit(Sm, Trans, AP_MAX_AUTH_RSP_STATE, AP_MAX_AUTH_RSP_MSG, Drop, AP_AUTH_RSP_IDLE, AP_AUTH_RSP_MACHINE_BASE);

    // column 1
    StateMachineSetAction(Sm, AP_AUTH_RSP_IDLE, APMT2_PEER_AUTH_ODD, ApAuthPeerAuthAtAuthRspIdleAction);
    StateMachineSetAction(Sm, AP_AUTH_RSP_IDLE, APMT2_PEER_DEAUTH, ApAuthPeerDeauthReqAction);

    // initialize the random number generator
    NdisGetCurrentSystemTime(&NOW);
    LfsrInit(pAd, NOW.LowPart);
}

/*
    ==========================================================================
    Description:
        Process the received Authnetication frame from client
    ==========================================================================
*/
VOID ApAuthPeerAuthAtAuthRspIdleAction(
    IN PMP_ADAPTER pAd, 
    IN PMLME_QUEUE_ELEM Elem) 
{
    USHORT          Seq, Alg, RspReason, i, Status;
    UCHAR           Addr2[MAC_ADDR_LEN];
    PHEADER_802_11  pRcvHdr;
    HEADER_802_11   AuthHdr;
    CHAR            Chtxt[CIPHER_TEXT_LEN];
    PUCHAR          pOutBuffer = NULL;
    NDIS_STATUS     NStatus;
    UINT            FrameLen = 0;
    MAC_TABLE_ENTRY *pEntry;
    UCHAR           ChTxtIe = 16, ChTxtLen = CIPHER_TEXT_LEN;
    /** NDIS indications */
    BOOLEAN         indicateAssocCompleteFailed = FALSE;
    DOT11_INCOMING_ASSOC_STARTED_PARAMETERS assocStartPara = {0};
    PDOT11_INCOMING_ASSOC_COMPLETION_PARAMETERS assocCompletePara = NULL;
    ULONG           assocCompleteParaSize;
    UCHAR           errorSource = DOT11_ASSOC_ERROR_SOURCE_OS;
    ULONG           errorStatus = 0;
    USHORT        Reason = REASON_DISASSOC_INACTIVE;
    DOT11_DISASSOCIATION_PARAMETERS disassocPara;
    PMP_PORT  pPort = pAd->PortList[Elem->PortNum];
    DBGPRINT(RT_DEBUG_INFO,("AUTH_RSP - ApAuthPeerAuthAtAuthRspIdleAction\n"));

#ifdef MULTI_CHANNEL_SUPPORT
    if(MT_TEST_FLAG(pAd,fRTMP_ADAPTER_BSS_SCAN_IN_PROGRESS) && 
        (!MT_TEST_BIT(pAd->pP2pCtrll->MultiChannelStartOrEndProcess, MULTI_CHANNEL_IN_NEW_SESSION)))
#else
    if(MT_TEST_FLAG(pAd,fRTMP_ADAPTER_BSS_SCAN_IN_PROGRESS))
#endif
        return;
    
    if (! ApSanityPeerAuthSanity(pAd, Elem->Msg, Elem->MsgLen, Addr2, &Alg, &Seq, &Status, Chtxt)) 
        return;
    
    // ignore requests from unwanted STA
    if (! ApCheckAccessControlList(pAd, Addr2))
        return;
    
    pEntry = NULL;

    // Search the desired entry from the software-based MAC table.
    pEntry = MacTableLookup(pAd,  pPort, Addr2);
    
    if (pEntry)
    {       
        DBGPRINT(RT_DEBUG_TRACE, ("%s: Found the desired entry from the software-based MAC table.\n", __FUNCTION__));

        if (pEntry->ValidAsCLI == TRUE)
        {           
            if (pEntry->bIAmBadAtheros == TRUE)
            {
                MtAsicUpdateProtect(pAd, HT_ATHEROS, ALLN_SETPROTECT, FALSE, FALSE);
                DBGPRINT(RT_DEBUG_TRACE, ("%s, LINE%d, Atheros Problem. Turn on RTS/CTS!!!\n",__FUNCTION__,__LINE__));
                pEntry->bIAmBadAtheros = FALSE;
            }
        }

        //in retry auth  case ,already association ,we need to indicate disassociate event to os,in case of data port off
        if(pEntry->AcceptAssoc)
        {
            APPrepareDisassocPara(pAd, Addr2, DOT11_DISASSOC_REASON_PEER_DISASSOCIATED | Reason, &disassocPara);

            //
            // Send NDIS indication
            //
            ApPlatformIndicateDot11Status(pAd, Elem->PortNum, NDIS_STATUS_DOT11_DISASSOCIATION, NULL, &disassocPara, sizeof(DOT11_DISASSOCIATION_PARAMETERS));
        }       
    }
    else
        DBGPRINT(RT_DEBUG_TRACE,("[%s][%d] NO entry found in the software-based MAC table.\n",__FUNCTION__,__LINE__));

#if UAPSD_AP_SUPPORT
    // This entry has existed.
    if (IS_AP_SUPPORT_UAPSD(pAd, pPort) && pEntry && (pEntry->UAPSDQAid < NUM_OF_UAPSD_CLI))
    {
        pAd->UAPSD.UapsdSWQBitmap &= (~(1 << (pEntry->UAPSDQAid)));
        DBGPRINT(RT_DEBUG_TRACE, ("ApAuthPeerAuthAtAuthRspIdleAction: Reset UapsdSWQBitmap to %d before link up\n", pAd->UAPSD.UapsdSWQBitmap));      
    }
#endif

    pRcvHdr = (PHEADER_802_11)(Elem->Msg);
    DBGPRINT(RT_DEBUG_TRACE, ("AUTH_RSP - Rcv AUTH seq#%d, Alg=%d, Status=%d from [wcid=%d]%02x:%02x:%02x:%02x:%02x:%02x\n",
        Seq,Alg,Status,Elem->Wcid, Addr2[0],Addr2[1],Addr2[2],Addr2[3],Addr2[4],Addr2[5]));

    if (Seq == 1) 
    {
        if ((Alg == AUTH_OPEN_SYSTEM) && 
            (pPort->PortCfg.AuthMode != Ralink802_11AuthModeShared)) 
        {
            if (!pEntry) // This is an empty entry.
            {
                DBGPRINT(RT_DEBUG_TRACE, ("ApAuthPeerAuthAtAuthRspIdleAction: This is an empty entry (!pEntry).\n"));
                pEntry = MacTableInsertEntry(pAd,pPort, Addr2, TRUE);            
            }

            if (pEntry)
            {
                pEntry->AuthState = AS_AUTH_OPEN;
                pEntry->Sst = SST_AUTH; // what if it already in SST_ASSOC ???????
                BATableTearRECEntry(pAd, pPort,0x0, (UCHAR)pEntry->Aid,TRUE);
                BATableTearORIEntry(pAd,pPort,0x0, (UCHAR)pEntry->Aid,TRUE, TRUE);
                ApAuthPeerAuthSimpleRspGenAndSend(pAd, pPort,pRcvHdr, Alg, Seq + 1, MLME_SUCCESS);
            }
            else
            {
                DBGPRINT(RT_DEBUG_ERROR, ("ApAuthPeerAuthAtAuthRspIdleAction: mac table full\n"));
                ; // MAC table full, what should we respond ?????
            }
        } 
        else if ((Alg == AUTH_SHARED_KEY) && 
            ((pPort->PortCfg.AuthMode == Ralink802_11AuthModeShared) || (pPort->PortCfg.AuthMode == Ralink802_11AuthModeAutoSwitch)))
        {
            if (!pEntry)
            {
                pEntry = MacTableInsertEntry(pAd, pPort, Addr2, TRUE);
            }

            if (pEntry)
            {
                pEntry->AuthState = AS_AUTHENTICATING;
                pEntry->Sst = SST_NOT_AUTH; // what if it already in SST_ASSOC ???????

                BATableTearRECEntry(pAd,pPort, 0x0, (UCHAR)pEntry->Aid,TRUE);
                BATableTearORIEntry(pAd,pPort, 0x0, (UCHAR)pEntry->Aid,TRUE, TRUE);
                // log this STA in AuthRspAux machine, only one STA is stored. If two STAs using
                // SHARED_KEY authentication mingled together, then the late comer will win.
                COPY_MAC_ADDR(&pPort->SoftAP.ApMlmeAux.Addr, Addr2);
                for(i = 0; i < CIPHER_TEXT_LEN; i++) 
                    pPort->SoftAP.ApMlmeAux.Challenge[i] = RandomByte(pAd);

                RspReason = 0;
                Seq++;

                NStatus = MlmeAllocateMemory(pAd, &pOutBuffer);
                if(NStatus != NDIS_STATUS_SUCCESS) 
                    return;  // if no memory, can't do anything

                DBGPRINT(RT_DEBUG_TRACE, ("AUTH_RSP - Send AUTH seq#2 (Challenge)\n"));
                MgtMacHeaderInit(pAd, pPort,&AuthHdr, SUBTYPE_AUTH, 0, Addr2, pPort->PortCfg.Bssid);
                MakeOutgoingFrame(pOutBuffer,            &FrameLen,
                                  sizeof(HEADER_802_11), &AuthHdr,
                                  2,                     &Alg,
                                  2,                     &Seq,
                                  2,                     &RspReason,
                                  1,                     &ChTxtIe,
                                  1,                     &ChTxtLen,
                                  CIPHER_TEXT_LEN,       pPort->SoftAP.ApMlmeAux.Challenge,
                                  END_OF_ARGS);
                NdisCommonMiniportMMRequest(pAd, pOutBuffer, FrameLen);
            }
            else
                ; // MAC table full, what should we respond ????
        } 
        else 
        {
            // wrong algorithm
            ApAuthPeerAuthSimpleRspGenAndSend(pAd,pPort, pRcvHdr, Alg, Seq + 1, MLME_ALG_NOT_SUPPORT);
            indicateAssocCompleteFailed = TRUE;
            errorSource = DOT11_ASSOC_ERROR_SOURCE_REMOTE;
            errorStatus = DOT11_FRAME_STATUS_UNSUPPORTED_AUTH_ALGO;

            DBGPRINT(RT_DEBUG_TRACE, ("AUTH_RSP - Alg=%d, Seq=%d, AuthMode=%s\n",
                Alg, Seq, decodeAuthAlgorithm(pPort->PortCfg.AuthMode)));
        }
    } 
    else if (Seq == 3)
    {
        if (pEntry && MAC_ADDR_EQUAL(Addr2, pPort->SoftAP.ApMlmeAux.Addr)) 
        {
            if ((pRcvHdr->FC.Wep == 1) && PlatformEqualMemory(Chtxt, pPort->SoftAP.ApMlmeAux.Challenge, CIPHER_TEXT_LEN)) 
            {
                // Successful
                ApAuthPeerAuthSimpleRspGenAndSend(pAd, pPort,pRcvHdr, Alg, Seq + 1, MLME_SUCCESS);
                pEntry->AuthState = AS_AUTH_KEY;
                pEntry->Sst = SST_AUTH;
            } 
            else 
            {
                // fail - wep bit is not set or challenge text is not equal
                ApAuthPeerAuthSimpleRspGenAndSend(pAd,pPort, pRcvHdr, Alg, Seq + 1, MLME_REJ_CHALLENGE_FAILURE);
                indicateAssocCompleteFailed = TRUE;
                errorSource = DOT11_ASSOC_ERROR_SOURCE_REMOTE;
                errorStatus = DOT11_FRAME_STATUS_INVALID_AUTH_CHALLENGE;
                MacTableDeleteAndResetEntry(pAd, pPort ,pEntry->Aid, pEntry->Addr, TRUE);
                Chtxt[127]='\0';
                pPort->SoftAP.ApMlmeAux.Challenge[127]='\0';
                DBGPRINT(RT_DEBUG_TRACE, ("pRcvHdr->Wep= %d\n",pRcvHdr->FC.Wep));
                DBGPRINT(RT_DEBUG_TRACE, ("Sent Challenge = %s\n",&pPort->SoftAP.ApMlmeAux.Challenge[100]));
                DBGPRINT(RT_DEBUG_TRACE, ("Rcv Challenge = %s\n",&Chtxt[100]));
            }
        } 
        else 
        {
            // fail for unknown reason. most likely is AuthRspAux machine be overwritten by another
            // STA also using SHARED_KEY authentication
            ApAuthPeerAuthSimpleRspGenAndSend(pAd,pPort, pRcvHdr, Alg, Seq + 1, MLME_UNSPECIFY_FAIL);
            indicateAssocCompleteFailed = TRUE;
            errorSource = DOT11_ASSOC_ERROR_SOURCE_REMOTE;
            errorStatus = DOT11_MGMT_REASON_UPSPEC_REASON;
        }
    }
    else 
    {
        // fail - wrong sequence number
        ApAuthPeerAuthSimpleRspGenAndSend(pAd,pPort, pRcvHdr, Alg, Seq + 1, MLME_SEQ_NR_OUT_OF_SEQUENCE);
        indicateAssocCompleteFailed = TRUE;
        errorSource = DOT11_ASSOC_ERROR_SOURCE_REMOTE;
        errorStatus = DOT11_FRAME_STATUS_INVALID_AUTH_XID;      
    }

    if (((pPort->PortSubtype == PORTSUBTYPE_VwifiAP) || (pPort->PortType == WFD_GO_PORT)) && (indicateAssocCompleteFailed))
    {
        do
        {
            //
            // Prepare association start and complete indications
            //
            if (indicateAssocCompleteFailed)
            {
                APPrepareAssocStartPara(Addr2, &assocStartPara);
                
                //
                // Allocate association completion parameters
                // 
                if (APAllocAssocCompletePara(pAd, DOT11_MAX_MSDU_SIZE, DOT11_MAX_MSDU_SIZE, &assocCompletePara, &assocCompleteParaSize) != NDIS_STATUS_SUCCESS)
                {
                    DBGPRINT(RT_DEBUG_ERROR, ("ApAuthPeerAuthAtAuthRspIdleAction:Unable to allocate association completion parameters.\n"));
                    break;
                }

                //
                // Fill in information
                //
                APPrepareAssocCompletePara(pAd, NULL, Addr2,  errorStatus, errorSource, FALSE, NULL, 0, NULL, 0, assocCompletePara, &assocCompleteParaSize);

                ApPlatformIndicateDot11Status(pAd, Elem->PortNum, NDIS_STATUS_DOT11_INCOMING_ASSOC_STARTED, NULL, &assocStartPara, sizeof(DOT11_INCOMING_ASSOC_STARTED_PARAMETERS));

                ApPlatformIndicateDot11Status(pAd, Elem->PortNum, NDIS_STATUS_DOT11_INCOMING_ASSOC_COMPLETION, NULL, assocCompletePara, assocCompleteParaSize);

                PlatformFreeMemory(assocCompletePara, assocCompleteParaSize);
            }
        } while (FALSE);
    }
}

/*
    ==========================================================================
    Description:
        Process De-authentication request frame received from client
    ==========================================================================
*/
VOID ApAuthPeerDeauthReqAction(
    IN PMP_ADAPTER pAd, 
    IN PMLME_QUEUE_ELEM Elem) 
{
    UCHAR       Addr2[MAC_ADDR_LEN];
    DOT11_DISASSOCIATION_PARAMETERS disassocPara;
    USHORT      Reason;
    MAC_TABLE_ENTRY       *pEntry;
    PMP_PORT      pPort = pAd->PortList[Elem->PortNum];
    PMAC_TABLE_ENTRY pWcidMacTabEntry = NULL;
    pWcidMacTabEntry = MlmeSyncMacTabMatchedWlanIdx(pPort, Elem->Wcid);   
    DBGPRINT(RT_DEBUG_TRACE,("AUTH_RSP - ApAuthPeerDeauthReqAction\n"));
    if (! ApSanityPeerDeauthReqSanity(pAd, Elem->Msg, Elem->MsgLen, Addr2, &Reason)) 
        return;
    DBGPRINT(RT_DEBUG_TRACE,("AUTH_RSP - ApAuthPeerDeauthReqAction 2\n"));
    pEntry = NULL;

    if(pWcidMacTabEntry == NULL)
    {
        DBGPRINT(RT_DEBUG_TRACE,("%s(), Line:%d, Mac Entry is NULL  Wcid = %d\n", __FUNCTION__, __LINE__, Elem->Wcid));
        return;
    }

    if (Elem->Wcid < MAX_LEN_OF_MAC_TABLE)
    {
        pEntry = pWcidMacTabEntry;
        if (pEntry->CMTimerRunning == TRUE)
        {
            // if one who initilized Counter Measure deauth itself,  AP doesn't log the MICFailTime
            pPort->SoftAP.ApCfg.aMICFailTime = pPort->SoftAP.ApCfg.PrevaMICFailTime;
        }

        ApLogEvent(pAd, Addr2, EVENT_DISASSOCIATED);
        if (((pPort->PortSubtype == PORTSUBTYPE_VwifiAP) || (pPort->PortType == WFD_GO_PORT)) && pEntry->AcceptAssoc) 
        {
            APPrepareDisassocPara(pAd, Addr2, DOT11_DISASSOC_REASON_PEER_DISASSOCIATED | Reason, &disassocPara);

            //
            // Send NDIS indication
            //
            ApPlatformIndicateDot11Status(pAd, Elem->PortNum, NDIS_STATUS_DOT11_DISASSOCIATION, NULL, &disassocPara, sizeof(DOT11_DISASSOCIATION_PARAMETERS));
            DBGPRINT(RT_DEBUG_TRACE, ("Recv DE-AUTH Indicate - DOT11_DISASSOC_REASON_PEER_DISASSOCIATED\n"));
        }

        if ((pPort->PortSubtype == PORTSUBTYPE_P2PGO) && 
            (IS_P2P_GO_WPA2PSKING(pPort) || IS_P2P_GO_OP(pPort) ||
            (IS_P2P_MS_GO(pAd, pPort) && MT_TEST_BIT(pAd->pP2pCtrll->P2pMsConnectedStatus, P2pMs_CONNECTED_STATUS_GO_ASSOC))))
        {
            GoPeerDisassocReq(pAd, pEntry, Addr2);
        }

        MacTableDeleteAndResetEntry(pAd, pPort, Elem->Wcid, Addr2, TRUE);
        DBGPRINT(RT_DEBUG_TRACE,("AUTH_RSP - receive DE-AUTH. delete MAC entry %02x:%02x:%02x:%02x:%02x:%02x\n", Addr2[0],Addr2[1],Addr2[2],Addr2[3],Addr2[4],Addr2[5] ));
    }
}

/*
    ==========================================================================
    Description:
        Send out a Authentication (response) frame
    ==========================================================================
*/
VOID ApAuthPeerAuthSimpleRspGenAndSend(
    IN PMP_ADAPTER pAd, 
    IN PMP_PORT pPort, 
    IN PHEADER_802_11 pHdr, 
    IN USHORT Alg, 
    IN USHORT Seq, 
    IN USHORT StatusCode) 
{
    HEADER_802_11     AuthHdr;
    UINT              FrameLen = 0;
    PUCHAR            pOutBuffer = NULL;
    NDIS_STATUS       NStatus;

    NStatus = MlmeAllocateMemory(pAd, &pOutBuffer);
    if (NStatus != NDIS_STATUS_SUCCESS) 
    {
        DBGPRINT(RT_DEBUG_TRACE,("[%s][%d] MlmeAllocateMemory failed. return.\n",__FUNCTION__,__LINE__));
        return;
    }

    if (StatusCode == MLME_SUCCESS)
    {
        DBGPRINT(RT_DEBUG_TRACE, ("AUTH_RSP - Send AUTH response (SUCCESS)...\n"));
        MgtMacHeaderInit(pAd,pPort, &AuthHdr, SUBTYPE_AUTH, 0, pHdr->Addr2, pPort->PortCfg.Bssid);
        MakeOutgoingFrame(pOutBuffer,            &FrameLen, 
                          sizeof(HEADER_802_11), &AuthHdr, 
                          2,                     &Alg, 
                          2,                     &Seq, 
                          2,                     &StatusCode, 
                          END_OF_ARGS);
        NdisCommonMiniportMMRequest(pAd, pOutBuffer, FrameLen);       
    }
    else
    {       
        DBGPRINT(RT_DEBUG_TRACE, ("AUTH_RSP - Peer AUTH fail (Status = %d)...\n", StatusCode));
        MgtMacHeaderInit(pAd, pPort,&AuthHdr, SUBTYPE_AUTH, 0, pHdr->Addr2, pPort->PortCfg.Bssid);
            MakeOutgoingFrame(pOutBuffer,            &FrameLen, 
                          sizeof(HEADER_802_11), &AuthHdr, 
                          2,                     &Alg, 
                          2,                     &Seq, 
                          2,                     &StatusCode, 
                          END_OF_ARGS);
            NdisCommonMiniportMMRequest(pAd, pOutBuffer, FrameLen);
    }
}

