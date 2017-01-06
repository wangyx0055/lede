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
    assoc.c
 
    Abstract:
    Handle association related requests either from WSTA or from local MLME
 
    Revision History:
    Who         When          What
    --------    ----------    ----------------------------------------------
    John Chang  08-04-2003    created for 11g soft-AP
 */

#include "MtConfig.h"

#define DRIVER_FILE         0x00200000

/*  
    ==========================================================================
    Description: 
        association state machine init, including state transition and timer init
    Parameters: 
        S - pointer to the association state machine
    Note:
        The state machine looks like the following 
        
                                    AP_ASSOC_IDLE             
        APMT2_MLME_DISASSOC_REQ    mlme_disassoc_req_action 
        APMT2_PEER_DISASSOC_REQ    peer_disassoc_action     
        APMT2_PEER_ASSOC_REQ       drop                     
        APMT2_PEER_REASSOC_REQ     drop                     
        APMT2_CLS3ERR              cls3err_action           
    ==========================================================================
 */
VOID ApMlmeAssocStateMachineInit(
    IN  PMP_ADAPTER   pAd, 
    IN  STATE_MACHINE *S, 
    OUT STATE_MACHINE_FUNC Trans[]) 
{
    StateMachineInit(S, Trans, AP_MAX_ASSOC_STATE, AP_MAX_ASSOC_MSG, Drop, AP_ASSOC_IDLE, AP_ASSOC_MACHINE_BASE);

    StateMachineSetAction(S, AP_ASSOC_IDLE, APMT2_MLME_DISASSOC_REQ, ApAssocMlmeDisassocReqAction);
    StateMachineSetAction(S, AP_ASSOC_IDLE, APMT2_PEER_DISASSOC_REQ, ApAssocPeerDisassocReqAction);
    StateMachineSetAction(S, AP_ASSOC_IDLE, APMT2_PEER_ASSOC_REQ,    ApAssocPeerAssocReqAction);
    StateMachineSetAction(S, AP_ASSOC_IDLE, APMT2_PEER_REASSOC_REQ,  ApAssocPeerReassocReqAction);
//  StateMachineSetAction(S, AP_ASSOC_IDLE, APMT2_CLS3ERR,           ApAssocMlmeAssocCls3errAction);
}

/*
    ==========================================================================
    Description:
        peer assoc req handling procedure
    Parameters:
        Adapter - Adapter pointer
        Elem - MLME Queue Element
    Pre:
        the station has been authenticated and the following information is stored
    Post  :
        -# An association response frame is generated and sent to the air
    ==========================================================================
 */
VOID ApAssocPeerAssocReqAction(
    IN PMP_ADAPTER pAd, 
    IN MLME_QUEUE_ELEM *Elem) 
{
    UCHAR           Addr2[MAC_ADDR_LEN];
    USHORT          ListenInterval;
    USHORT          CapabilityInfo;
    USHORT          StatusCode = 0;
    USHORT          Aid;
    PUCHAR          pOutBuffer = NULL;
    NDIS_STATUS     NStatus;
    ULONG           FrameLen = 0;
    char            Ssid[MAX_LEN_OF_SSID];
    UCHAR           SsidLen;
    
    UCHAR           SupportedRatesLen;
    UCHAR           SupportedRates[MAX_LEN_OF_SUPPORTED_RATES];
    UCHAR           MaxSupportedRate = 0;
    UCHAR           i;
    MAC_TABLE_ENTRY *pEntry;
    UCHAR           RSNIE_Len;
    UCHAR           RSN_IE[AP_MAX_LEN_OF_RSNIE];
    BOOLEAN         bWmmCapable;
    ULONG           RalinkIe;

    HT_CAPABILITY_IE        HTCapability;
    UCHAR             HTCapability_Len=0;
    VHT_CAP_IE        VhtCapability;
    UCHAR             VhtCapabilityLen=0;

       
            
    PMP_PORT pPort= pAd->PortList[Elem->PortNum]; 
    
    UCHAR           RSNIe=IE_WPA2;
    //******************************************************************************
    // NDIS indications
    DOT11_INCOMING_ASSOC_STARTED_PARAMETERS assocStartPara;
    PDOT11_INCOMING_ASSOC_REQUEST_RECEIVED_PARAMETERS assocReqPara = NULL;  
    ULONG assocReqParaSize = 0;
    PDOT11_INCOMING_ASSOC_COMPLETION_PARAMETERS assocCompletePara = NULL;
    ULONG assocCompleteParaSize = 0;
    BOOLEAN         indicateAssocStart = FALSE;
    BOOLEAN         indicateAssocReqReceived = FALSE;
    BOOLEAN         indicateAssocComplete = FALSE;
    UCHAR           errorSource = 0; //DOT11_ASSOC_ERROR_SOURCE_OTHER;
    ULONG           errorStatus = 0;
    //******************************************************************************
    ULONG           P2PSubelementLen;
    UCHAR           *P2pSubelement;
    ULONG           WpsLen;
    UCHAR           *WpsElement;
    
    P2pSubelement = NULL;
    WpsElement = NULL;
    PlatformAllocateMemory(pAd, &P2pSubelement, MAX_VIE_LEN);
    if (P2pSubelement == NULL)
        goto CleanUp;
     PlatformAllocateMemory(pAd, &WpsElement, MAX_VIE_LEN);
    if (WpsElement == NULL)
        goto CleanUp;
    
    PlatformZeroMemory(&HTCapability, sizeof(HT_CAPABILITY_IE));
    // 1. frame sanity check
    if (! ApSanityPeerAssocReqSanity(pAd,
                            Elem->PortNum,
                            Elem->Msg,
                            Elem->MsgLen,
                            Addr2,
                            &CapabilityInfo,
                            &ListenInterval,
                            &SsidLen,
                            &Ssid[0],
                            &SupportedRatesLen,
                            &SupportedRates[0],
                            RSN_IE, &RSNIE_Len,
                            &bWmmCapable,
                            &RalinkIe,
                            &WpsLen,
                            WpsElement,
                            &P2PSubelementLen,
                            P2pSubelement,
                            &HTCapability_Len, 
                            &HTCapability,
                            &VhtCapabilityLen,
                            &VhtCapability)) 
        goto CleanUp;
    DBGPRINT(RT_DEBUG_ERROR, ("ASSOC - 0 receive ASSOC request from %02x:%02x:%02x:%02x:%02x:%02x\n", Addr2[0],Addr2[1],Addr2[2],Addr2[3],Addr2[4],Addr2[5]));
    DBGPRINT(RT_DEBUG_ERROR, ("ASSOC - SsidLen = %d,  %d. Port= %d\n", SsidLen, pPort->PortCfg.SsidLen, Elem->PortNum));
    DBGPRINT(RT_DEBUG_ERROR, ("ASSOC -1 Ssid = %c%c%c%c%c%c%c%c%c \n", Ssid[0],Ssid[1],Ssid[2],Ssid[3],Ssid[4],Ssid[5],Ssid[6],Ssid[7],Ssid[8]));
    DBGPRINT(RT_DEBUG_ERROR, ("ASSOC - 2 Ssid = %c%c%c%c%c%c%c%c%c \n", pPort->PortCfg.Ssid[0],pPort->PortCfg.Ssid[1],pPort->PortCfg.Ssid[2],pPort->PortCfg.Ssid[3],pPort->PortCfg.Ssid[4],pPort->PortCfg.Ssid[5],pPort->PortCfg.Ssid[6],pPort->PortCfg.Ssid[7],pPort->PortCfg.Ssid[8]));   
    // for hidden SSID sake, SSID in AssociateRequest should be fully verified
    if ((SsidLen != pPort->PortCfg.SsidLen) || (PlatformEqualMemory(Ssid, pPort->PortCfg.Ssid, SsidLen)==0))
        goto CleanUp;

    DBGPRINT(RT_DEBUG_ERROR, ("ASSOC - 1 receive ASSOC request Ssid = %c%c%c%c%c%c \n", Ssid[0],Ssid[1],Ssid[2],Ssid[3],Ssid[4],Ssid[5]));
    // ignore request from unwanted STA
    if (! ApCheckAccessControlList(pAd, Addr2))
        goto CleanUp;
    
    pEntry = MacTableLookup(pAd, pPort, Addr2);
    if (!pEntry)
    {
        DBGPRINT(RT_DEBUG_TRACE,("[%s][%d] NO entry found in the software-based MAC table. return.\n",__FUNCTION__,__LINE__));
        goto CleanUp;
    }
    
    DBGPRINT(RT_DEBUG_ERROR, ("ASSOC - receive ASSOC request from %02x:%02x:%02x:%02x:%02x:%02x\n", Addr2[0],Addr2[1],Addr2[2],Addr2[3],Addr2[4],Addr2[5]));
    //******************************************************************************
    //
    // Initialize association decision.
    // It will be overwritten by the decision made by OS.
    // The initial values are used for the error case.
    //
    pEntry->AcceptAssoc = FALSE;
    pEntry->Reason = DOT11_FRAME_STATUS_FAILURE;

    if (APAllocAssocCompletePara(pAd, DOT11_MAX_MSDU_SIZE, DOT11_MAX_MSDU_SIZE, &assocCompletePara, &assocCompleteParaSize) != NDIS_STATUS_SUCCESS)
    {
        DBGPRINT(RT_DEBUG_ERROR, ("ApAssocPeerAssocReqAction:Unable to allocate association completion parameters.\n"));
        goto CleanUp;
    }

    //
    // [Win8] GO role port should follow the same rule of VwifiAP to communicate with NDIS
    //
    
    if ((pPort->PortSubtype == PORTSUBTYPE_P2PGO) && (pPort->PortType != WFD_GO_PORT))
        ;   // GO with Ralink's P2P in Win7, do nothing
    else
    {
        //
        // Send NDIS indications: NDIS_STATUS_DOT11_INCOMING_ASSOC_STARTED
        //
        MP_ASSIGN_NDIS_OBJECT_HEADER(assocStartPara.Header,
                                    NDIS_OBJECT_TYPE_DEFAULT,
                                    DOT11_INCOMING_ASSOC_STARTED_PARAMETERS_REVISION_1,
                                    sizeof(DOT11_INCOMING_ASSOC_STARTED_PARAMETERS));
        COPY_MAC_ADDR(&assocStartPara.PeerMacAddr, Addr2);

        assocReqParaSize = sizeof(DOT11_INCOMING_ASSOC_REQUEST_RECEIVED_PARAMETERS) + Elem->MsgLen - LENGTH_802_11;
         PlatformAllocateMemory(pAd, &assocReqPara, assocReqParaSize);
        if (assocReqPara == NULL)
        {
            DBGPRINT(RT_DEBUG_ERROR, ("ApAssocPeerAssocReqAction:Unable to allocate %u bytes for association request received parameters.\n", assocReqParaSize));
            goto CleanUp;
        }
        PlatformZeroMemory(assocReqPara, assocReqParaSize);

        MP_ASSIGN_NDIS_OBJECT_HEADER(assocReqPara->Header, 
                                    NDIS_OBJECT_TYPE_DEFAULT,
                                    DOT11_INCOMING_ASSOC_REQUEST_RECEIVED_PARAMETERS_REVISION_1,
                                    sizeof(DOT11_INCOMING_ASSOC_REQUEST_RECEIVED_PARAMETERS));

        COPY_MAC_ADDR(&assocReqPara->PeerMacAddr, Addr2);
        assocReqPara->bReAssocReq = FALSE;
        assocReqPara->uAssocReqSize = Elem->MsgLen - LENGTH_802_11;
        assocReqPara->uAssocReqOffset = sizeof(DOT11_INCOMING_ASSOC_REQUEST_RECEIVED_PARAMETERS);
        DBGPRINT(RT_DEBUG_ERROR, ("ApAssocPeerAssocReqAction:size =%d\n",assocReqPara->uAssocReqSize));
        //
        // Copy association request
        //
        PlatformMoveMemory(Add2Ptr(assocReqPara, assocReqPara->uAssocReqOffset), Add2Ptr(Elem->Msg, LENGTH_802_11), Elem->MsgLen - LENGTH_802_11);
    }

    //******************************************************************************

    // supported rates array may not be sorted. sort it and find the maximum rate
    for (i=0; i<SupportedRatesLen; i++)
    {
        if (MaxSupportedRate < (SupportedRates[i] & 0x7f)) 
            MaxSupportedRate = SupportedRates[i] & 0x7f;
    }            
    pEntry->RateLen = SupportedRatesLen;

    pEntry->usListenInterval = ListenInterval;
    
    PlatformZeroMemory(pEntry->SupRate, MAX_LEN_OF_SUPPORTED_RATES);

    PlatformMoveMemory(pEntry->SupRate, SupportedRates, SupportedRatesLen);
    pEntry->SupRateLen = SupportedRatesLen;

    // For ApMlmeSelectTxRateTable
    ApMlmeSetSupportedLegacyMCS(pEntry);
    
    // 2. qualify this STA's auth_asoc status in the MAC table, decide StatusCode
    DBGPRINT(RT_DEBUG_TRACE, ("ASSOC - RalinkIe (0x%04x)...\n", RalinkIe));
    StatusCode = APBuildAssociation(pAd,
                                pPort->PortNumber,
                                pEntry, CapabilityInfo,
                                MaxSupportedRate,
                                RSN_IE,
                                &RSNIE_Len,
                                bWmmCapable,
                                RalinkIe,
                                P2pSubelement,
                                (UCHAR)P2PSubelementLen,
                                WpsElement,
                                (UCHAR)WpsLen,
                                &HTCapability,
                                HTCapability_Len,
                                &VhtCapability,
                                VhtCapabilityLen,
                                &Aid);
    //******************************************************************************

    // For the 1st P2P peer to connect me, I need to create a adhoc profile for OS.
    // For the 2nd and following P2P peers to connect me, needn't do this.
    if (StatusCode == MLME_SUCCESS)
    {
        if (pPort->StaCfg.WscControl.WscState == WSC_STATE_LINK_UP)
        {
            DBGPRINT(RT_DEBUG_ERROR, ("%s - Set WscState = LinkUP. doesn't indicate association now.\n", __FUNCTION__));
        }
        else if ((pPort->PortSubtype == PORTSUBTYPE_VwifiAP) || (pPort->PortType == WFD_GO_PORT))
        {
            indicateAssocStart = TRUE;
            indicateAssocReqReceived = TRUE;
            DBGPRINT(RT_DEBUG_TRACE, ("!!!indicateAssocStart  indicateAssocReqReceived TRUE!!! \n"));
        }
    }

    DBGPRINT(RT_DEBUG_ERROR, ("%s status =%d. . P2P State = %s.\n", __FUNCTION__, StatusCode, decodeP2PState(pPort->P2PCfg.P2PConnectState)));
    DBGPRINT(RT_DEBUG_ERROR, ("%s WscState =%d.  PortSubtype = %d. .\n", __FUNCTION__, pPort->StaCfg.WscControl.WscState, pPort->PortSubtype));   
    DBGPRINT(RT_DEBUG_ERROR, ("%s status =%d\n", __FUNCTION__, StatusCode));

#if(COMPILE_WIN7_ABOVE(CURRENT_OS_NDIS_VER))    
    errorStatus = StatusCode;
    //
    // Indicate NDIS_STATUS_DOT11_INCOMING_ASSOC_STARTED
    //
    if (indicateAssocStart)
    {
        indicateAssocComplete = TRUE;  // assoc start and assoc completion is a pair.
        ApPlatformIndicateDot11Status(pAd, Elem->PortNum,NDIS_STATUS_DOT11_INCOMING_ASSOC_STARTED, NULL, &assocStartPara, sizeof(assocStartPara));
    }

    //
    // Indicate NDIS_STATUS_DOT11_INCOMING_ASSOC_REQUEST_RECEIVED
    //
    if (indicateAssocReqReceived)
    {
        DBGPRINT(RT_DEBUG_TRACE, ("NDIS_STATUS_DOT11_INCOMING_ASSOC_REQUEST_RECEIVED.\n"));
        ApPlatformIndicateDot11Status(pAd, Elem->PortNum, NDIS_STATUS_DOT11_INCOMING_ASSOC_REQUEST_RECEIVED, NULL, assocReqPara, assocReqParaSize);

        //
        // OS will make the direct OID call OID_DOT11_INCOMING_ASSOCIATION_DECISION
        // in the status indication thread.
        // So after the status indication returns, the association decision shall be
        // cached in the station entry already. We need to process it now.
        //
        DBGPRINT(RT_DEBUG_TRACE, ("ApAssocPeerAssocReqAction:pEntry->AcceptAssoc = %d, pEntry->Reason= %d, PORT =%d\n", pEntry->AcceptAssoc, pEntry->Reason, Elem->PortNum));
        
        if (pEntry->AcceptAssoc)
        {
            pEntry->AssocState = dot11_assoc_state_auth_assoc;
            StatusCode = pEntry->Reason;

            //
            // Reset statistics
            //
            PlatformZeroMemory(&pEntry->Statistics, sizeof(DOT11_PEER_STATISTICS));

            //
            // Association up time
            //
            NdisGetCurrentSystemTime(&pEntry->AssocUpTime);
        }
        else
        {
            errorStatus = DOT11_FRAME_STATUS_FAILURE;
            errorSource = DOT11_ASSOC_ERROR_SOURCE_OS;
            StatusCode = MLME_ASSOC_REJ_UNABLE_HANDLE_STA;
        }
        DBGPRINT(RT_DEBUG_ERROR, ("ApAssocPeerAssocReqAction:pEntry->AcceptAssoc = %d, pEntry->Reason= %d, PORT =%d\n", pEntry->AcceptAssoc, pEntry->Reason, Elem->PortNum));
    }
#endif
    //******************************************************************************

    // Check write TXOP != 0.
    if ((RalinkIe != 0) &&(pPort->CommonCfg.bRdg))
    {
        AsicEnableRDG(pAd);
    }
    else
    {
        AsicDisableRDG(pAd, pPort);
    }
    /*
    pEntry->RateLen = SupportedRatesLen;

    pEntry->usListenInterval = ListenInterval;

    PlatformZeroMemory(pEntry->SupRate, MAX_LEN_OF_SUPPORTED_RATES);

    PlatformMoveMemory(pEntry->SupRate, SupportedRates, SupportedRatesLen);
    pEntry->SupRateLen = SupportedRatesLen;

    // For ApMlmeSelectTxRateTable
    ApMlmeSetSupportedLegacyMCS(pEntry);
*/
    // 3. send Association Response
    NStatus = MlmeAllocateMemory(pAd, &pOutBuffer);
    if (NStatus != NDIS_STATUS_SUCCESS) 
        goto CleanUp;
        
    DBGPRINT(RT_DEBUG_TRACE, ("ASSOC - Send ASSOC response (Status=%d)...\n", StatusCode));
    DBGPRINT(RT_DEBUG_TRACE, ("ASSOC - Send ASSOC response to %02x:%02x:%02x:%02x:%02x:%02x\n", Addr2[0],Addr2[1],Addr2[2],Addr2[3],Addr2[4],Addr2[5]));
    Aid |= 0xc000; // 2 most significant bits should be ON
    
    MgntPktConstructAPPeerAssocActionFrame(pPort, 
                                                                CapabilityInfo, 
                                                                pEntry, 
                                                                StatusCode, 
                                                                Aid, 
                                                                RalinkIe, 
                                                                P2PSubelementLen, 
                                                                HTCapability_Len, 
                                                                indicateAssocComplete, 
                                                                errorSource, 
                                                                errorStatus, 
                                                                assocCompletePara, 
                                                                assocCompleteParaSize, 
                                                                Addr2,
                                                                TRUE,
                                                                FALSE,
                                                                Elem, 
                                                                pOutBuffer, 
                                                                &FrameLen);
   
    NdisCommonMiniportMMRequest(pAd, pOutBuffer, FrameLen);

    DBGPRINT(RT_DEBUG_TRACE, ("ASSOC -PortSubtype = %d. P2PSubelemLen = %d.->AuthMode = %d.(P2PConnectState = %s)\n", pPort->PortSubtype, P2PSubelementLen, pEntry->AuthMode, decodeP2PState(pPort->P2PCfg.P2PConnectState)));
    DBGPRINT(RT_DEBUG_TRACE, ("ASSOC -Dpid = %d. WpsLen = %d. )\n", pPort->P2PCfg.Dpid, WpsLen));
    DBGPRINT(RT_DEBUG_TRACE, ("ASSOCC -WpaState = %d. WscState = %d. )\n", pAd->StaCfg.WpaState,pPort->StaCfg.WscControl.WscState));
    if ((pPort->PortSubtype == PORTSUBTYPE_P2PGO) && (pPort->PortType != WFD_GO_PORT))
    {
        if ((StatusCode == MLME_SUCCESS)
            && (pAd->StaCfg.WpaState == SS_START))
        {
            DBGPRINT(RT_DEBUG_TRACE, ("ASSOCC -Enqueue WPA State machine. )\n"));
            // Enqueue a EAPOL-start message with the pEntry
            MlmeEnqueue(pAd,pPort, AP_WPA_STATE_MACHINE, APMT2_EAPOLStart, 6, Addr2);
        }
    }
    else if (pPort->PortSubtype == PORTSUBTYPE_SoftapAP)
    {
        DBGPRINT(RT_DEBUG_TRACE, ("ASSOCC -SoftapAP Enqueue WPA State machine. )\n"));

        if (((pEntry->AuthMode == Ralink802_11AuthModeWPAPSK) || (pEntry->AuthMode == Ralink802_11AuthModeWPA2PSK)) && (StatusCode == MLME_SUCCESS))
        {
            // Enqueue a EAPOL-start message with the pEntry
            MlmeEnqueue(pAd,pPort, AP_WPA_STATE_MACHINE, APMT2_EAPOLStart, 6, Addr2);
        }
    }

CleanUp:
    if (P2pSubelement)
        PlatformFreeMemory(P2pSubelement, MAX_VIE_LEN);

    if (WpsElement)
        PlatformFreeMemory(WpsElement,MAX_VIE_LEN);

    if (assocReqPara)
        PlatformFreeMemory(assocReqPara, assocReqParaSize);

    if (assocCompletePara)
        PlatformFreeMemory(assocCompletePara, assocCompleteParaSize);
}

/*
    ==========================================================================
    Description:
        mlme reassoc req handling procedure
    Parameters:
        Elem - 
    Pre:
        -# SSID  (Adapter->ApCfg.ssid[])
        -# BSSID (AP address, Adapter->ApCfg.bssid)
        -# Supported rates (Adapter->ApCfg.supported_rates[])
        -# Supported rates length (Adapter->ApCfg.supported_rates_len)
        -# Tx power (Adapter->ApCfg.tx_power)
    ==========================================================================
 */
VOID ApAssocPeerReassocReqAction(
    IN PMP_ADAPTER pAd, 
    IN MLME_QUEUE_ELEM *Elem) 
{
    UCHAR           ApAddr[MAC_ADDR_LEN], Addr2[MAC_ADDR_LEN];
    USHORT          CapabilityInfo, ListenInterval;
    USHORT          StatusCode = MLME_SUCCESS;
    USHORT          Aid = 1;
    ULONG           FrameLen = 0;
    NDIS_STATUS     NStatus;
    PUCHAR          pOutBuffer = NULL;
    char            Ssid[MAX_LEN_OF_SSID];
    UCHAR           SsidLen;
    UCHAR           SupportedRatesLen;
    UCHAR           SupportedRates[MAX_LEN_OF_SUPPORTED_RATES];
    UCHAR           MaxSupportedRate = 0;
    int             i;
    MAC_TABLE_ENTRY *pEntry;
    UCHAR           RSNIE_Len;
    UCHAR           RSN_IE[AP_MAX_LEN_OF_RSNIE];
    BOOLEAN         bWmmCapable;
    ULONG           RalinkIe;
    HT_CAPABILITY_IE        HTCapability;
    UCHAR           HTCapability_Len = 0;

    VHT_CAP_IE        VhtCapability;
    UCHAR             VhtCapabilityLen = 0;
  
    //******************************************************************************
    // NDIS indications
    DOT11_INCOMING_ASSOC_STARTED_PARAMETERS assocStartPara;
    PDOT11_INCOMING_ASSOC_REQUEST_RECEIVED_PARAMETERS assocReqPara = NULL;  
    ULONG assocReqParaSize = 0;
    PDOT11_INCOMING_ASSOC_COMPLETION_PARAMETERS assocCompletePara = NULL;
    ULONG assocCompleteParaSize = 0;
    BOOLEAN         indicateAssocStart = FALSE;
    BOOLEAN         indicateAssocReqReceived = FALSE;
    BOOLEAN         sendResponse = FALSE;
    BOOLEAN         indicateAssocComplete = FALSE;
    UCHAR           errorSource = 0; //DOT11_ASSOC_ERROR_SOURCE_OTHER;
    ULONG           errorStatus = 0;
    //******************************************************************************
    UCHAR       tmpExtRateLen = 0;
    UCHAR       SupRateLen = 0;
    ULONG       P2PSubelementLen;
    UCHAR       *P2pSubelement = NULL;
    ULONG       WpsLen;
    UCHAR       *WpsElement = NULL;

    PMP_PORT pPort= pAd->PortList[Elem->PortNum];
     PlatformAllocateMemory(pAd, &P2pSubelement, MAX_VIE_LEN);
    if (P2pSubelement == NULL)
        goto CleanUp;
    PlatformAllocateMemory(pAd, &WpsElement, MAX_VIE_LEN);
    if (WpsElement == NULL)
        goto CleanUp;

    PlatformZeroMemory(&HTCapability, sizeof(HT_CAPABILITY_IE));
    // 1. frame sanity check
    if (! ApSanityPeerReassocReqSanity(pAd,
                            pPort->PortNumber,
                            Elem->Msg,
                            Elem->MsgLen, 
                            Addr2, 
                            &CapabilityInfo,
                            &ListenInterval,
                            ApAddr, &SsidLen,
                            &Ssid[0],
                            &SupportedRatesLen,
                            &SupportedRates[0],
                            RSN_IE,
                            &RSNIE_Len,
                            &bWmmCapable,
                            &RalinkIe,
                            &WpsLen,
                            WpsElement,
                            &P2PSubelementLen,
                            P2pSubelement,
                            &HTCapability_Len, 
                            &HTCapability,
                            &VhtCapabilityLen, 
                            &VhtCapability)) 
        goto CleanUp;

    // for hidden SSID sake, SSID in AssociateRequest should be fully verified
    if ((SsidLen != pPort->PortCfg.SsidLen) || (PlatformEqualMemory(Ssid, pPort->PortCfg.Ssid, SsidLen)==0))
        goto CleanUp;

    // ignore request from unwanted STA
    if (! ApCheckAccessControlList(pAd, Addr2))
        goto CleanUp;

    pEntry = MacTableLookup(pAd, pPort, Addr2);
    if (!pEntry)
    {
        DBGPRINT(RT_DEBUG_TRACE,("[%s][%d] NO entry found in the software-based MAC table.\n",__FUNCTION__,__LINE__));
        goto CleanUp;
    }

    DBGPRINT(RT_DEBUG_TRACE, ("ASSOC - receive RE-ASSOC request from %02x:%02x:%02x:%02x:%02x:%02x\n", Addr2[0],Addr2[1],Addr2[2],Addr2[3],Addr2[4],Addr2[5]));
    //******************************************************************************
    //
    // Initialize association decision.
    // It will be overwritten by the decision made by OS.
    // The initial values are used for the error case.
    //
    pEntry->AcceptAssoc = FALSE;
    pEntry->Reason = DOT11_FRAME_STATUS_FAILURE;

    //
    // [Win8] GO role port should follow the same rule of VwifiAP to communicate with NDIS
    //
    
    if ((pPort->PortSubtype == PORTSUBTYPE_P2PGO) && (pPort->PortType != WFD_GO_PORT))
        ;   // GO with Ralink's P2P in Win7, do nothing
    else
    {
        if (APAllocAssocCompletePara(pAd, DOT11_MAX_MSDU_SIZE, DOT11_MAX_MSDU_SIZE, &assocCompletePara, &assocCompleteParaSize) != NDIS_STATUS_SUCCESS)
        {
            DBGPRINT(RT_DEBUG_ERROR, ("ApAssocPeerAssocReqAction:Unable to allocate association completion parameters.\n"));
            goto CleanUp;
        }
        //
        // Send NDIS indications: NDIS_STATUS_DOT11_INCOMING_ASSOC_STARTED
        //
        MP_ASSIGN_NDIS_OBJECT_HEADER(assocStartPara.Header,
                                    NDIS_OBJECT_TYPE_DEFAULT,
                                    DOT11_INCOMING_ASSOC_STARTED_PARAMETERS_REVISION_1,
                                    sizeof(DOT11_INCOMING_ASSOC_STARTED_PARAMETERS));
        COPY_MAC_ADDR(&assocStartPara.PeerMacAddr, Addr2);

        assocReqParaSize = sizeof(DOT11_INCOMING_ASSOC_REQUEST_RECEIVED_PARAMETERS) + Elem->MsgLen - LENGTH_802_11;
         PlatformAllocateMemory(pAd, &assocReqPara, assocReqParaSize);
        if (assocReqPara == NULL)
        {
            DBGPRINT(RT_DEBUG_ERROR, ("ApAssocPeerAssocReqAction:Unable to allocate %u bytes for association request received parameters.\n", assocReqParaSize));
            goto CleanUp;
        }
        PlatformZeroMemory(assocReqPara, assocReqParaSize);

        MP_ASSIGN_NDIS_OBJECT_HEADER(assocReqPara->Header, 
                                    NDIS_OBJECT_TYPE_DEFAULT,
                                    DOT11_INCOMING_ASSOC_REQUEST_RECEIVED_PARAMETERS_REVISION_1,
                                    sizeof(DOT11_INCOMING_ASSOC_REQUEST_RECEIVED_PARAMETERS));

        COPY_MAC_ADDR(&assocReqPara->PeerMacAddr, Addr2);
        assocReqPara->bReAssocReq = TRUE;
        assocReqPara->uAssocReqSize = Elem->MsgLen - LENGTH_802_11;
        assocReqPara->uAssocReqOffset = sizeof(DOT11_INCOMING_ASSOC_REQUEST_RECEIVED_PARAMETERS);
        DBGPRINT(RT_DEBUG_ERROR, ("ApAssocPeerAssocReqAction:size =%d\n",assocReqPara->uAssocReqSize));
        //
        // Copy association request
        //
        PlatformMoveMemory(Add2Ptr(assocReqPara, assocReqPara->uAssocReqOffset), Add2Ptr(Elem->Msg, LENGTH_802_11), Elem->MsgLen - LENGTH_802_11);
    }

    //******************************************************************************
        
    // supported rates array may not be sorted. sort it and find the maximum rate
    for (i=0; i<SupportedRatesLen; i++)
    {
        if (MaxSupportedRate < (SupportedRates[i] & 0x7f))
            MaxSupportedRate = SupportedRates[i] & 0x7f;
    }            
    
    // 2. qualify this STA's auth_asoc status in the MAC table, decide StatusCode
    StatusCode = APBuildAssociation(pAd,
                                pPort->PortNumber,
                                pEntry,
                                CapabilityInfo,
                                MaxSupportedRate,
                                RSN_IE,
                                &RSNIE_Len,
                                bWmmCapable,
                                RalinkIe,
                                P2pSubelement,
                                (UCHAR)P2PSubelementLen,
                                WpsElement,
                                (UCHAR)WpsLen,
                                &HTCapability, 
                                HTCapability_Len, 
                                &VhtCapability,
                                VhtCapabilityLen,
                                &Aid);

    pEntry->RateLen = SupportedRatesLen;
    //******************************************************************************
    if (StatusCode == MLME_SUCCESS)
    {
        if (pPort->StaCfg.WscControl.WscState == WSC_STATE_LINK_UP)
        {
            DBGPRINT(RT_DEBUG_ERROR, ("%s - Set WscState = LinkUP. doesn't indicate association now.\n", __FUNCTION__));
        }
        else if ((pPort->PortSubtype == PORTSUBTYPE_VwifiAP) || (pPort->PortType == WFD_GO_PORT))
        {
                indicateAssocStart = TRUE;
                indicateAssocReqReceived = TRUE;
                DBGPRINT(RT_DEBUG_TRACE, ("!!!indicateAssocStart  indicateAssocReqReceived TRUE!!! \n"));
        }
    }
    DBGPRINT(RT_DEBUG_ERROR, ("%s status =%d\n", __FUNCTION__, StatusCode));
    
#if(COMPILE_WIN7_ABOVE(CURRENT_OS_NDIS_VER))
    errorStatus = StatusCode;
    //
    // Indicate NDIS_STATUS_DOT11_INCOMING_ASSOC_STARTED
    //
    if (indicateAssocStart)
    {
        indicateAssocComplete = TRUE;
        ApPlatformIndicateDot11Status(pAd, Elem->PortNum,NDIS_STATUS_DOT11_INCOMING_ASSOC_STARTED, NULL, &assocStartPara, sizeof(assocStartPara));
    }

    //
    // Indicate NDIS_STATUS_DOT11_INCOMING_ASSOC_REQUEST_RECEIVED
    //
    if (indicateAssocReqReceived)
    {
        ApPlatformIndicateDot11Status(pAd, Elem->PortNum, NDIS_STATUS_DOT11_INCOMING_ASSOC_REQUEST_RECEIVED, NULL, assocReqPara, assocReqParaSize);

        //
        // OS will make the direct OID call OID_DOT11_INCOMING_ASSOCIATION_DECISION
        // in the status indication thread.
        // So after the status indication returns, the association decision shall be
        // cached in the station entry already. We need to process it now.
        //
        if (pEntry->AcceptAssoc)
        {
            pEntry->AssocState = dot11_assoc_state_auth_assoc;

            //
            // Reset statistics
            //
            PlatformZeroMemory(&pEntry->Statistics, sizeof(DOT11_PEER_STATISTICS));

            //
            // Association up time
            //
            NdisGetCurrentSystemTime(&pEntry->AssocUpTime);
        }
        else
        {
            errorStatus = DOT11_FRAME_STATUS_FAILURE;
            errorSource = DOT11_ASSOC_ERROR_SOURCE_OS;
            StatusCode = MLME_ASSOC_REJ_UNABLE_HANDLE_STA;
        }
        DBGPRINT(RT_DEBUG_ERROR, ("ApAssocPeerAssocReqAction:pEntry->AcceptAssoc = %d PORT =%d\n", pEntry->AcceptAssoc,Elem->PortNum));
    }
    //******************************************************************************
#endif
    // RDG will set 0x80 to AC0_TXOP and TxBurst will also set 0x60 to the same place.
    // So, we should change it dynamically. 
    if ((RalinkIe != 0) &&(pPort->CommonCfg.bRdg))
    {
        AsicEnableRDG(pAd);
    }
    else
    {
        AsicDisableRDG(pAd, pPort);
    }

    PlatformZeroMemory(pEntry->SupRate, MAX_LEN_OF_SUPPORTED_RATES);

    // For ApMlmeSelectTxRateTable
    ApMlmeSetSupportedLegacyMCS(pEntry);

    // 3. reply Re-association Response
    NStatus = MlmeAllocateMemory(pAd, &pOutBuffer);
    if (NStatus != NDIS_STATUS_SUCCESS) 
        goto CleanUp;

    DBGPRINT(RT_DEBUG_TRACE, ("ASSOC - Send RE-ASSOC response (Status = %d)...\n", StatusCode));
    Aid |= 0xc000; // 2 most significant bits should be ON
   
    MgntPktConstructAPPeerAssocActionFrame(pPort, 
                                                                CapabilityInfo, 
                                                                pEntry, 
                                                                StatusCode, 
                                                                Aid, 
                                                                RalinkIe, 
                                                                P2PSubelementLen, 
                                                                HTCapability_Len, 
                                                                indicateAssocComplete, 
                                                                errorSource, 
                                                                errorStatus, 
                                                                assocCompletePara, 
                                                                assocCompleteParaSize, 
                                                                Addr2,
                                                                FALSE,
                                                                TRUE,
                                                                Elem, 
                                                                pOutBuffer, 
                                                                &FrameLen);    
                                                                
    NdisCommonMiniportMMRequest(pAd, pOutBuffer, FrameLen);

    if ((pPort->PortSubtype == PORTSUBTYPE_P2PGO) && (pPort->PortType != WFD_GO_PORT))
    {
        if ((StatusCode == MLME_SUCCESS)
            && (pAd->StaCfg.WpaState == SS_START))
        {
            DBGPRINT(RT_DEBUG_TRACE, ("ASSOCC -Enqueue WPA State machine. )\n"));
            // Enqueue a EAPOL-start message with the pEntry
            MlmeEnqueue(pAd, pPort, AP_WPA_STATE_MACHINE, APMT2_EAPOLStart, 6, Addr2);
        }
    }
    else if (pPort->PortSubtype == PORTSUBTYPE_SoftapAP)
    {

        if (((pEntry->AuthMode == Ralink802_11AuthModeWPAPSK) || (pEntry->AuthMode == Ralink802_11AuthModeWPA2PSK)) && (StatusCode == MLME_SUCCESS))
        {
            // Enqueue a EAPOL-start message with the pEntry
            MlmeEnqueue(pAd, pPort,AP_WPA_STATE_MACHINE, APMT2_EAPOLStart, 6, Addr2);
        }
    }

CleanUp:
    if (P2pSubelement)
        PlatformFreeMemory(P2pSubelement, MAX_VIE_LEN);

    if (WpsElement)
        PlatformFreeMemory(WpsElement,MAX_VIE_LEN);

    if (assocReqPara)
        PlatformFreeMemory(assocReqPara, assocReqParaSize);

    if (assocCompletePara)
        PlatformFreeMemory(assocCompletePara, assocCompleteParaSize);
}

/*
    ==========================================================================
    Description:
        left part of IEEE 802.11/1999 p.374
    Parameters:
        Elem - MLME message containing the received frame
    ==========================================================================
 */
VOID ApAssocPeerDisassocReqAction(
    IN PMP_ADAPTER pAd,
    IN MLME_QUEUE_ELEM *Elem)
{
    UCHAR         Addr2[MAC_ADDR_LEN];
    USHORT        Reason;
    DOT11_DISASSOCIATION_PARAMETERS disassocPara;
    PMAC_TABLE_ENTRY    pEntry;
    PMP_PORT                        pPort = pAd->PortList[Elem->PortNum];

    DBGPRINT(RT_DEBUG_TRACE, ("ASSOC - 1 receive DIS-ASSOC request \n"));
    if (! ApSanityPeerDisassocReqSanity(pAd, Elem->Msg, Elem->MsgLen, Addr2, &Reason))
        return;

    DBGPRINT(RT_DEBUG_TRACE, ("ASSOC - receive DIS-ASSOC request from %02x:%02x:%02x:%02x:%02x:%02x, reason=%d\n", Addr2[0],Addr2[1],Addr2[2],Addr2[3],Addr2[4],Addr2[5],Reason));

    pEntry = MacTableLookup(pAd, pPort, Addr2);
    if(!pEntry)
    {
        DBGPRINT(RT_DEBUG_TRACE,("[%s][%d] DIS-ASSOC request, but cannot find entry in MacTable\n",__FUNCTION__,__LINE__));
        return;
    }


    if (Elem->Wcid < MAX_LEN_OF_MAC_TABLE && (pEntry != NULL) && (pEntry->Aid != 0) )
    {
        DBGPRINT(RT_DEBUG_TRACE, ("DIS-ASS Elem->Wcid = %d\n",Elem->Wcid));
        if (((pPort->PortSubtype == PORTSUBTYPE_VwifiAP) || (pPort->PortType == WFD_GO_PORT)) && pEntry->AcceptAssoc)
        {
            APPrepareDisassocPara(pAd, Addr2, DOT11_DISASSOC_REASON_PEER_DISASSOCIATED | Reason, &disassocPara);

            //
            // Send NDIS indication
            //
            ApPlatformIndicateDot11Status(pAd, Elem->PortNum, NDIS_STATUS_DOT11_DISASSOCIATION, NULL, &disassocPara, sizeof(DOT11_DISASSOCIATION_PARAMETERS));
            DBGPRINT(RT_DEBUG_TRACE, ("Indicate - DOT11_DISASSOC_REASON_PEER_DISASSOCIATED\n"));
        }
        else
        {
            APPlatformIndicateDisassociationStatus(pAd, pPort, Addr2, DOT11_DISASSOC_REASON_PEER_DISASSOCIATED | Reason);
        }

        ApLogEvent(pAd, Addr2, EVENT_DISASSOCIATED);

        if ((pPort->PortSubtype == PORTSUBTYPE_P2PGO) &&
            (IS_P2P_GO_WPA2PSKING(pPort) || IS_P2P_GO_OP(pPort) ||
            (IS_P2P_MS_GO(pAd, pPort) && MT_TEST_BIT(pAd->pP2pCtrll->P2pMsConnectedStatus, P2pMs_CONNECTED_STATUS_GO_ASSOC))))
        {
            GoPeerDisassocReq(pAd, pEntry, Addr2);
        }
        MacTableDeleteAndResetEntry(pAd, pPort, Elem->Wcid, Addr2, TRUE);
    }
}

/*
    ==========================================================================
    Description:
        Upper layer orders to disassoc s STA
    Parameters:
        Elem -
    ==========================================================================
 */
VOID ApAssocMlmeDisassocReqAction(
    IN PMP_ADAPTER pAd, 
    IN MLME_QUEUE_ELEM *Elem) 
{
    MLME_DISASSOC_REQ_STRUCT *DisassocReq;
    HEADER_802_11        DisassocHdr;
    PUCHAR                pOutBuffer = NULL;
    ULONG                 FrameLen = 0;
    NDIS_STATUS           NStatus;  
    PMP_PORT pPort= pAd->PortList[Elem->PortNum];

    DisassocReq = (MLME_DISASSOC_REQ_STRUCT *)(Elem->Msg);  

    // 2. send out a DISASSOC request frame
    NStatus = MlmeAllocateMemory(pAd, &pOutBuffer);
    if (NStatus != NDIS_STATUS_SUCCESS) 
        return;
    
    DBGPRINT(RT_DEBUG_TRACE, ("ASSOC - MLME disassociates %02x:%02x:%02x:%02x:%02x:%02x; Send DISASSOC request\n",
        DisassocReq->Addr[0],DisassocReq->Addr[1],DisassocReq->Addr[2],
        DisassocReq->Addr[3],DisassocReq->Addr[4],DisassocReq->Addr[5]));
    MgtMacHeaderInit(pAd, pPort,&DisassocHdr, SUBTYPE_DISASSOC, 0, DisassocReq->Addr, pPort->PortCfg.Bssid);
    MakeOutgoingFrame(pOutBuffer,            &FrameLen, 
                      sizeof(HEADER_802_11), &DisassocHdr, 
                      2,                     &DisassocReq->Reason, 
                      END_OF_ARGS);
    NdisCommonMiniportMMRequest(pAd,  pOutBuffer, FrameLen);
}


/*
    ==========================================================================
    Description:
        right part of IEEE 802.11/1999 page 374
    Note: 
        This event should never cause ASSOC state machine perform state
        transition, and has no relationship with CNTL machine. So we separate
        this routine as a service outside of ASSOC state transition table.
    ==========================================================================
 */
VOID ApAssocMlmeAssocCls3errAction(
    IN PMP_ADAPTER pAd, 
    IN PUCHAR        pAddr) 
{
    HEADER_802_11         DisassocHdr;
    PUCHAR                pOutBuffer = NULL;
    ULONG                 FrameLen = 0;
    NDIS_STATUS           NStatus;
    USHORT                Reason = REASON_CLS3ERR;
    MAC_TABLE_ENTRY       *pEntry;
    PMP_PORT pPort= pAd->PortList[PORT_0]; 
    pEntry = MacTableLookup(pAd, pPort, pAddr);
    if (pEntry)
    {
        MacTableDeleteAndResetEntry(pAd,pPort,pEntry->Aid, pAddr, TRUE);
    }
    
    // 2. send out a DISASSOC request frame
    NStatus = MlmeAllocateMemory(pAd, &pOutBuffer);
    if (NStatus != NDIS_STATUS_SUCCESS) 
        return;
    
    DBGPRINT(RT_DEBUG_TRACE, ("ASSOC - Class 3 Error, Send DISASSOC frame to %02x:%02x:%02x:%02x:%02x:%02x\n", pAddr[0],pAddr[1],pAddr[2],pAddr[3],pAddr[4],pAddr[5]));
    MgtMacHeaderInit(pAd,pPort, &DisassocHdr, SUBTYPE_DISASSOC, 0, pAddr, pPort->PortCfg.Bssid);
    MakeOutgoingFrame(pOutBuffer,            &FrameLen, 
                      sizeof(HEADER_802_11), &DisassocHdr, 
                      2,                     &Reason, 
                      END_OF_ARGS);
    NdisCommonMiniportMMRequest(pAd,  pOutBuffer, FrameLen);
}
 
/*
    ==========================================================================
    Description:
    Return:
    ==========================================================================
*/
VOID ApAssocGoBuildAssociation(
    IN PMP_ADAPTER pAd,
    IN PMP_PORT   pPort,
    IN MAC_TABLE_ENTRY *pEntry,
    IN UCHAR         *RSN,
    IN UCHAR         *pRSNLen,
    IN UCHAR        *pP2pElement,
    IN UCHAR         P2pElementLen,
    IN UCHAR        *pWpsElement,
    IN UCHAR         WpsElementLen,
    IN HT_CAPABILITY_IE     *pHtCapability,
    IN UCHAR         HtCapabilityLen)
{
    UCHAR           p2pindex = P2P_NOT_FOUND;
    UCHAR           GroupCap = 0xff,DeviceCap = 0xff, DevAddr[6], DeviceType[8], DeviceName[32], DeviceNameLen = 0;
    USHORT          Dpid, ConfigMethod;
    UCHAR           StatusCode;
    UCHAR           CrednIndex;
    UCHAR           NumSecondaryType = 0, SecondarDevTypes[MAX_P2P_SECONDARY_DEVTYPE_LIST][P2P_DEVICE_TYPE_LEN];

    // Case 1. P2P Device that connects to me 
    if (P2pElementLen != 0)
    {
        PlatformZeroMemory(DevAddr, 6);
        PlatformZeroMemory(DeviceType, 8);
        PlatformZeroMemory(SecondarDevTypes, sizeof(SecondarDevTypes));
        
        P2pParseSubElmt(pAd, (PVOID)pP2pElement, P2pElementLen, 
            FALSE, &Dpid, &GroupCap, &DeviceCap, DeviceName, &DeviceNameLen, DevAddr, NULL, NULL, NULL, NULL, &ConfigMethod, &ConfigMethod, DeviceType, &NumSecondaryType, (PUCHAR)&SecondarDevTypes[0][0], NULL, NULL, NULL, NULL, &StatusCode, NULL);
        // This Station is using P2P protocol to connect to me. 
        p2pindex = P2pGroupTabSearch(pAd, DevAddr);
        DBGPRINT(RT_DEBUG_TRACE, (" %d. DeviceCap = %x, DevAddr = %x %x %x %x %x %x\n", p2pindex, DeviceCap, DevAddr[0], DevAddr[1], DevAddr[2],DevAddr[3],DevAddr[4],DevAddr[5]));
        DBGPRINT(RT_DEBUG_TRACE, ("ConfigMethod = %x, DeviceType = %x %x %x %x %x %x %x %x\n", ConfigMethod , DeviceType[0], DeviceType[1], DeviceType[2],DeviceType[3],DeviceType[4],DeviceType[5],DeviceType[6],DeviceType[7]));
        DBGPRINT(RT_DEBUG_TRACE, ("DeviceNameLen = %d, DeviceName = %c %c %c %c %c %c %c %c\n", DeviceNameLen , DeviceName[0], DeviceName[1], DeviceName[2],DeviceName[3],DeviceName[4],DeviceName[5],DeviceName[6],DeviceName[7]));
        // Add flag to show this support P2P 
        if (p2pindex < MAX_P2P_GROUP_SIZE)
        {
            pEntry->ValidAsP2P = TRUE;
            if ((DeviceCap&DEVCAP_CLIENT_DISCOVER) == DEVCAP_CLIENT_DISCOVER)
            {
                DBGPRINT(RT_DEBUG_TRACE,("This Device is discoverable. \n"));
                P2P_SET_FLAG(&pAd->pP2pCtrll->P2PTable.Client[p2pindex], P2PFLAG_DEVICE_DISCOVERABLE);
            }
        
            if ((pEntry->RSNIE_Len > 0) && (WpsElementLen == 0))
            {
                pPort->P2PCfg.P2PConnectState = P2P_GO_ASSOC_AUTH;
                pAd->pP2pCtrll->P2PTable.Client[p2pindex].P2pClientState = P2PSTATE_CLIENT_ASSOC;
                pPort->StaCfg.WscControl.WscState = WSC_STATE_OFF;
                pAd->StaCfg.WpaState = SS_START;
                DBGPRINT(RT_DEBUG_TRACE,("case 2-1 : Support P2P, use WPA2PSK \n"));
            }
            else
            {
                pPort->P2PCfg.P2PConnectState = P2P_WPS_REGISTRA;
                pAd->pP2pCtrll->P2PTable.Client[p2pindex].P2pClientState = P2PSTATE_CLIENT_WPS;
                pPort->StaCfg.WscControl.WscState = WSC_STATE_LINK_UP;
                pAd->StaCfg.WpaState = SS_NOTUSE;
                DBGPRINT(RT_DEBUG_TRACE,("case 2-2 : Support P2P, use WPS \n"));
            }
        }
        else 
        {
            if ((pEntry->RSNIE_Len > 0) && (WpsElementLen == 0))
            {
                p2pindex = P2pGroupTabInsert(pAd, DevAddr, P2PSTATE_CLIENT_ASSOC, pPort->PortCfg.Ssid, pPort->PortCfg.SsidLen);
                pPort->StaCfg.WscControl.WscState = WSC_STATE_OFF;
                pAd->StaCfg.WpaState = SS_START;
                pPort->P2PCfg.P2PConnectState = P2P_GO_ASSOC_AUTH;
                if (p2pindex >= MAX_P2P_GROUP_SIZE)
                {
                    p2pindex = P2pGroupTabInsert(pAd, DevAddr, P2PSTATE_CLIENT_ASSOC, pPort->PortCfg.Ssid, pPort->PortCfg.SsidLen);
                    if (p2pindex < MAX_P2P_GROUP_SIZE)
                        pAd->pP2pCtrll->P2PTable.Client[p2pindex].P2pClientState = P2PSTATE_CLIENT_ASSOC;
                }
                DBGPRINT(RT_DEBUG_TRACE,("case 1-1 : Support P2P, direct connect. use WPA2PSK \n"));
                // Concurrnet mode doesn't need to restart host AP. So doesn't need to set event.
                if (FALSE == P2pCrednEntrySearch(pAd, pEntry->Addr, &CrednIndex))
                {
                    DBGPRINT(RT_DEBUG_TRACE,("case 1-1-1 : Can't Find WPS Credential. USe pass phrase as PMK \n"));
                    P2pSetEvent(pAd, pPort, P2PEVENT_DRIVER_INTERNAL_USE);
                }
            }
            else
            {
                p2pindex = P2pGroupTabInsert(pAd, DevAddr, P2PSTATE_CLIENT_WPS, pPort->PortCfg.Ssid, pPort->PortCfg.SsidLen);
                pPort->StaCfg.WscControl.WscState = WSC_STATE_LINK_UP;
                pAd->StaCfg.WpaState = SS_NOTUSE;
                pPort->P2PCfg.P2PConnectState = P2P_WPS_REGISTRA;
                DBGPRINT(RT_DEBUG_TRACE,("case 1-2 : Support P2P, direct connect. use WPS \n"));
            }
        }
        
        // Set flag that this entry support P2P.
        pEntry->ValidAsP2P = TRUE;
        if (p2pindex < MAX_P2P_GROUP_SIZE)
        {
            if ((DeviceCap&DEVCAP_CLIENT_DISCOVER) == DEVCAP_CLIENT_DISCOVER)
            {
                DBGPRINT(RT_DEBUG_TRACE,("This Device is discoverable. \n"));
                P2P_SET_FLAG(&pAd->pP2pCtrll->P2PTable.Client[p2pindex], P2PFLAG_DEVICE_DISCOVERABLE);
            }
            if (DeviceCap != 0xff)
                pAd->pP2pCtrll->P2PTable.Client[p2pindex].DevCapability = DeviceCap;
            if (GroupCap != 0xff)
                pAd->pP2pCtrll->P2PTable.Client[p2pindex].GroupCapability= GroupCap;
            if (DeviceNameLen > 0)
            {
                PlatformMoveMemory(pAd->pP2pCtrll->P2PTable.Client[p2pindex].DeviceName, DeviceName, 32);
                pAd->pP2pCtrll->P2PTable.Client[p2pindex].DeviceNameLen = DeviceNameLen;
                
            }
            if (!PlatformEqualMemory(DevAddr, ZERO_MAC_ADDR, MAC_ADDR_LEN))
            {
                PlatformMoveMemory(pAd->pP2pCtrll->P2PTable.Client[p2pindex].addr, DevAddr, MAC_ADDR_LEN);
                DBGPRINT(RT_DEBUG_TRACE, ("Copy Devaddr to table, DevAddr = %x %x %x %x %x %x\n",  DevAddr[0], DevAddr[1], DevAddr[2],DevAddr[3],DevAddr[4],DevAddr[5]));
                
            }
            PlatformMoveMemory(pAd->pP2pCtrll->P2PTable.Client[p2pindex].InterfaceAddr, pEntry->Addr, MAC_ADDR_LEN);
            PlatformMoveMemory(pAd->pP2pCtrll->P2PTable.Client[p2pindex].bssid, pEntry->Addr, MAC_ADDR_LEN);
            PlatformMoveMemory(pAd->pP2pCtrll->P2PTable.Client[p2pindex].PrimaryDevType, DeviceType, 8);
            pAd->pP2pCtrll->P2PTable.Client[p2pindex].NumSecondaryType = NumSecondaryType;
            PlatformMoveMemory(pAd->pP2pCtrll->P2PTable.Client[p2pindex].SecondaryDevType, SecondarDevTypes, NumSecondaryType * 8);
            pAd->pP2pCtrll->P2PTable.Client[p2pindex].ConfigMethod = ConfigMethod;
            pAd->pP2pCtrll->P2PTable.Client[p2pindex].Dpid = Dpid;
        }
        DBGPRINT(RT_DEBUG_TRACE,("[%d]P2pClientState is %s \n", p2pindex, decodeP2PClientState(pAd->pP2pCtrll->P2PTable.Client[p2pindex].P2pClientState)));
    }
    // Case 2. This is legacy device connection.  P2PIELen = 0
    else
    {
        // Set flag that this entry support P2P.
        pEntry->ValidAsP2P = FALSE;
        do 
        {
            // No WPA IE. must use WPS provisioning.
            // Case 2: this entry is first connected, WPS IE Len is 0. must be WPA2PSK
            if (((pEntry->RSNIE_Len > 0)) && (WpsElementLen == 0))
            {
                pPort->P2PCfg.P2PConnectState = P2P_GO_ASSOC_AUTH;
                pEntry->P2pInfo.P2pClientState = P2PSTATE_NONP2P_PSK;
                pPort->StaCfg.WscControl.WscState = WSC_STATE_OFF;
                pAd->StaCfg.WpaState = SS_START;
                DBGPRINT(RT_DEBUG_TRACE,("case 3-1 : Legacy STA,  use WPA2PSK \n"));
                // Concurrnet mode doesn't need to restart host AP. So doesn't need to set event.
                if (FALSE == P2pCrednEntrySearch(pAd, pEntry->Addr, &CrednIndex))
                {
                    DBGPRINT(RT_DEBUG_TRACE,("case 3-1 : Can't Find WPS Credential. USe pass phrase as PMK \n"));
                    pPort->P2PCfg.PhraseKeyLen = pPort->P2PCfg.LegacyPSKLen;
                    PlatformMoveMemory(pPort->P2PCfg.PhraseKey, pPort->P2PCfg.LegacyPSK, 64);
                    P2pSetEvent(pAd, pPort,  P2PEVENT_DRIVER_INTERNAL_USE);
                }

                // This Station has ever been in my group table.
                // And now it connects as legacy sta.
                p2pindex = P2pGroupTabSearch(pAd, pEntry->Addr);
                // If it uses legacy connection, we shall delete it here.
                if (p2pindex < MAX_P2P_GROUP_SIZE)
                {
                    DBGPRINT(RT_DEBUG_TRACE, (" %s. Delete it from Group table due to legacy connection\n", __FUNCTION__));
                    P2pGroupTabDelete(pAd, p2pindex, pEntry->Addr);
                }
            }
            else
            {
                //station will use open-wep to associate wih WPS AP
                pEntry->AuthMode = Ralink802_11AuthModeOpen;
                pPort->StaCfg.WscControl.WscState = WSC_STATE_LINK_UP;
                pAd->StaCfg.WpaState = SS_NOTUSE;
                pEntry->P2pInfo.P2pClientState = P2PSTATE_NONP2P_WPS;

                pPort->P2PCfg.P2PConnectState = P2P_WPS_REGISTRA;
                DBGPRINT(RT_DEBUG_TRACE,("case 3-2 : Legacy STA,  use WPS \n"));
                break;
            }

        }while(FALSE);
    }
    DBGPRINT(RT_DEBUG_TRACE,("P2pConnectState is %d. %s \n", pPort->P2PCfg.P2PConnectState, decodeP2PState(pPort->P2PCfg.P2PConnectState)));
    pPort->P2PCfg.P2pEventQueue.bP2pGenSeudoAdhoc = FALSE;

    // For the 1st P2P peer to connect me, I need to create a adhoc profile for OS.
    // For the 2nd and following P2P peers to connect me, needn't do this.
    if ((pPort->StaCfg.WscControl.WscState == WSC_STATE_OFF)&& (pPort->PortSubtype == PORTSUBTYPE_P2PGO))
    {
        if ((pPort->P2PCfg.P2pEventQueue.LastSetEvent == P2PEVENT_GEN_SEUDO_ADHOC_PROFILE))
            DBGPRINT(RT_DEBUG_ERROR, ("Last event is set seudo adhoc profile. MacTab Size = %d \n", pPort->MacTab.Size));

        DBGPRINT(RT_DEBUG_ERROR, ("  MacTab Size = %d \n", pPort->MacTab.Size));

        pPort->P2PCfg.bSetProfile = FALSE;
        //Check if the client is the first peer connecting to GO
        if(((pPort->MacTab.Size < 2) && (!INFRA_ON(pAd->PortList[PORT_0]))) ||
           ((pPort->MacTab.Size < 3) && (INFRA_ON(pAd->PortList[PORT_0])))) 
        {
                pPort->P2PCfg.bSetProfile = TRUE;
        }

        //For STA mode, we create profile here
        //For concurrent mode, we defer profile creation at P2pWPADone
        if ((pPort->P2PCfg.bSetProfile == TRUE) && (pAd->OpMode == OPMODE_STA)
            && (pPort->P2PCfg.P2pEventQueue.LastSetEvent != P2PEVENT_GEN_SEUDO_ADHOC_PROFILE))
        {
            DBGPRINT(RT_DEBUG_ERROR, ("ASSOC - try set seudo adhoc profile.\n"));
            if (P2pSetEvent(pAd, pPort,  P2PEVENT_GEN_SEUDO_ADHOC_PROFILE) == FALSE)
                pPort->P2PCfg.P2pEventQueue.bP2pGenSeudoAdhoc = TRUE;
            //When generating a seudo ad hoc profile, OS will call a inforeset request
            //GO must wait for *end of reset* or *reset timeout* before processing any packet
            //Set inforeset timeout to 15 seconds
        }
    }
    pPort->SoftAP.ApCfg.bSoftAPReady = TRUE;
}


/*
    ==========================================================================
    Description:
       assign a new AID to the newly associated/re-associated STA and
       decide its MaxSupportedRate and CurrTxRate. Both rates should not
       exceed AP's capapbility
    Return:
       MLME_SUCCESS - association successfully built
       others - association failed due to resource issue
    ==========================================================================
 */
USHORT APBuildAssociation(
    IN PMP_ADAPTER pAd,
    IN NDIS_PORT_NUMBER PortNumber,
    IN MAC_TABLE_ENTRY *pEntry,
    IN USHORT        CapabilityInfo,
    IN UCHAR         MaxSupportedRateIn500Kbps,
    IN UCHAR         *RSN,
    IN UCHAR         *pRSNLen,
    IN BOOLEAN       bWmmCapable,
    IN ULONG         ClientRalinkIe,
    IN UCHAR        *pP2pElement,
    IN UCHAR         P2pElementLen,
    IN UCHAR        *pWpsElement,
    IN UCHAR         WpsElementLen,
    IN HT_CAPABILITY_IE     *pHtCapability,
    IN UCHAR         HtCapabilityLen,
    IN VHT_CAP_IE       *pVhtCapability,
    IN UCHAR            VhtCapabilityLen,
    OUT USHORT       *pAid)
{
    USHORT           StatusCode = MLME_SUCCESS;
    UCHAR            MaxSupportedRate = RATE_11;
    BOOLEAN bAuthModeWPAXxxAP = FALSE;
    PMP_PORT  pPort  = pAd->PortList[PortNumber];
    PMP_PORT pStaPort;
    BOOLEAN bStaUse40 = FALSE;
    PUCHAR  pTable;
    UCHAR   TableSize = 0;

    
    switch (MaxSupportedRateIn500Kbps)
    {
        case 108: MaxSupportedRate = RATE_54;   break;
        case 96:  MaxSupportedRate = RATE_48;   break;
        case 72:  MaxSupportedRate = RATE_36;   break;
        case 48:  MaxSupportedRate = RATE_24;   break;
        case 36:  MaxSupportedRate = RATE_18;   break;
        case 24:  MaxSupportedRate = RATE_12;   break;
        case 18:  MaxSupportedRate = RATE_9;    break;
        case 12:  MaxSupportedRate = RATE_6;    break;
        case 22:  MaxSupportedRate = RATE_11;   break;
        case 11:  MaxSupportedRate = RATE_5_5;  break;
        case 4:   MaxSupportedRate = RATE_2;    break;
        case 2:   MaxSupportedRate = RATE_1;    break;
        default:  MaxSupportedRate = RATE_11;   break;
    }

    if ((pPort->SoftAP.ApCfg.PhyMode == PHY_11G) && (MaxSupportedRate < RATE_FIRST_OFDM_RATE))
        return MLME_ASSOC_REJ_DATA_RATE;

    if ((pPort->SoftAP.ApCfg.PhyMode == PHY_11N) && (HtCapabilityLen == 0))
        return MLME_ASSOC_REJ_DATA_RATE;

    if (!pEntry)
        return MLME_UNSPECIFY_FAIL;

    // The authentication mode of the AP is WPAXxx or not.
    switch (pPort->PortCfg.AuthMode)
    {
        case Ralink802_11AuthModeOpen:
        case Ralink802_11AuthModeShared:
        case Ralink802_11AuthModeAutoSwitch:
            bAuthModeWPAXxxAP = FALSE;
            break;
            
        case Ralink802_11AuthModeWPANone:
        case Ralink802_11AuthModeWPA:
        case Ralink802_11AuthModeWPAPSK:
        case Ralink802_11AuthModeWPA2:
        case Ralink802_11AuthModeWPA2PSK:
        case Ralink802_11AuthModeWPA1WPA2:
        case Ralink802_11AuthModeWPA1PSKWPA2PSK:
            bAuthModeWPAXxxAP = TRUE;
            break;
            
        default:
            DBGPRINT(RT_DEBUG_ERROR, ("%s: Invalid authentication mode (pPort->PortCfg.AuthMode = %s)\n", 
                __FUNCTION__, decodeAuthAlgorithm(pPort->PortCfg.AuthMode)));
            break;
    }

    if (pEntry && ((pEntry->Sst == SST_AUTH) || (pEntry->Sst == SST_ASSOC)))
    {
        // TODO:
        // should qualify other parameters, for example - capablity, supported rates, listen interval, ... etc
        // to decide the Status Code
        //*pAid = APAssignAid(pAd, pEntry);
        //pEntry->Aid = *pAid;
        *pAid = pEntry->Aid;
        
        PlatformMoveMemory(pEntry->RSN_IE, RSN, *pRSNLen);
        pEntry->RSNIE_Len = *pRSNLen;
        DBGPRINT(RT_DEBUG_TRACE, ("ASSOC - RSNIE_Len = 0x%x, RSNIE = 0x%x:%x:%x:%x\n", 
            pEntry->RSNIE_Len, pEntry->RSN_IE[0], pEntry->RSN_IE[1], pEntry->RSN_IE[2], pEntry->RSN_IE[3]));

        DBGPRINT(RT_DEBUG_TRACE, ("ASSOC - MaxSupportedRate = %x:pPort->CommonCfg.MaxTxRate = %x Aid = %d\n", 
             MaxSupportedRate, pPort->CommonCfg.MaxTxRate,*pAid ));
        if (*pAid == 0)
            StatusCode = MLME_ASSOC_REJ_UNABLE_HANDLE_STA;
        else if ((pEntry->RSNIE_Len == 0) && 
                (bAuthModeWPAXxxAP == TRUE) &&
                (pPort->SoftAP.ApCfg.bWPSEnable == FALSE) && 
                ((pPort->PortSubtype != PORTSUBTYPE_P2PGO) || (pPort->PortType == WFD_GO_PORT)))
        {
            // Check if NDIS already prepared for handling EAP/EAPOL frames
            // !! EAP/EAPOL frames in case of Ralink P2P are controled by driver's AP-WPA/WPS machine, excluded from this check. !!
            
            DBGPRINT(RT_DEBUG_TRACE, ("ASSOC - The authentication mode of the STA is inconsistent with the SoftAP's. (pEntry->RSNIE_Len = %d; pPort->PortCfg.AuthMode = %s).\n", 
                pEntry->RSNIE_Len, decodeAuthAlgorithm(pPort->PortCfg.AuthMode)));

            StatusCode = MLME_ASSOC_DENY_OUT_SCOPE;
        }
        else
        {
            if ((pPort->SoftAP.ApCfg.bWPSEnable == TRUE) && ((pPort->PortSubtype == PORTSUBTYPE_VwifiAP) || (pPort->PortType == WFD_GO_PORT)))
            {
                //station will use open-wep to associate wih WPS AP
                pEntry->AuthMode = Ralink802_11AuthModeOpen;
                if(pEntry->RSNIE_Len == 0 && (CAP_IS_PRIVACY_ON(CapabilityInfo )))
                {
                    pEntry->WepStatus = Ralink802_11WEPEnabled;
                }
                DBGPRINT(RT_DEBUG_TRACE, ("==>Assoc-WITH WPS Enabled.\n"));
            }

            // Update auth, wep, legacy transmit rate setting . 
            pEntry->Sst = SST_ASSOC;

            //
            // Patch for Nintendo DS support rate bug - it only support tx rate 1 and 2 
            //
            if ((MaxSupportedRate == RATE_11) &&
                (((pEntry->Addr[0]==0x00) && (pEntry->Addr[1]==0x09) && (pEntry->Addr[2]==0xBF)) || 
                ((pEntry->Addr[0]==0x00) && (pEntry->Addr[1]==0x16) && (pEntry->Addr[2]==0x56)) ||
                ((pEntry->Addr[0]==0x00) && (pEntry->Addr[1]==0x19) && (pEntry->Addr[2]==0xFD)) ||
                ((pEntry->Addr[0]==0x00) && (pEntry->Addr[1]==0x1A) && (pEntry->Addr[2]==0xE9)) ||
                ((pEntry->Addr[0]==0x00) && (pEntry->Addr[1]==0x1B) && (pEntry->Addr[2]==0x7A))))
            { 
                DBGPRINT(RT_DEBUG_TRACE, ("==>Assoc-Req from Nintendo DS client.\n"));
                MaxSupportedRate = RATE_2;
                pEntry->NDSClientSetting = NDS_CLIENT_ON;
            }
            else if (MaxSupportedRate == RATE_11)
            {
                MaxSupportedRate = RATE_2; // potential NDS Client case, will evaluate on RxData
                pEntry->NDSClientSetting = NDS_CLIENT_EVL;
                DBGPRINT(RT_DEBUG_TRACE, ("%s::MAC[%02x:%02x:%02x:%02x:%02x:%02x] is potential NDS Client, will evaluate later\n", __FUNCTION__,
                    pEntry->Addr[0], pEntry->Addr[1], pEntry->Addr[2], pEntry->Addr[3], pEntry->Addr[4], pEntry->Addr[5]));             
            }
            else
            {
                pEntry->NDSClientSetting = NDS_CLIENT_OFF;
            }

            pEntry->MaxSupportedRate = min(pPort->CommonCfg.MaxTxRate, MaxSupportedRate);
            if (pEntry->MaxSupportedRate < RATE_FIRST_OFDM_RATE)
            {
                WRITE_PHY_CFG_MODE(pAd, &pEntry->MaxPhyCfg, MODE_CCK);
                WRITE_PHY_CFG_MCS(pAd, &pEntry->MaxPhyCfg, pEntry->MaxSupportedRate);
                WRITE_PHY_CFG_MODE(pAd, &pEntry->MinPhyCfg, MODE_CCK);
                WRITE_PHY_CFG_MCS(pAd, &pEntry->MinPhyCfg, pEntry->MaxSupportedRate);
                WRITE_PHY_CFG_MODE(pAd, &pEntry->TxPhyCfg, MODE_CCK);
                WRITE_PHY_CFG_MCS(pAd, &pEntry->TxPhyCfg, pEntry->MaxSupportedRate);
            }
            else
            {
                WRITE_PHY_CFG_MODE(pAd, &pEntry->MaxPhyCfg, MODE_OFDM);
                WRITE_PHY_CFG_MCS(pAd, &pEntry->MaxPhyCfg, OfdmRateToRxwiMCS[pEntry->MaxSupportedRate]);
                WRITE_PHY_CFG_MODE(pAd, &pEntry->MinPhyCfg, MODE_OFDM);
                WRITE_PHY_CFG_MCS(pAd, &pEntry->MinPhyCfg, OfdmRateToRxwiMCS[pEntry->MaxSupportedRate]);
                WRITE_PHY_CFG_MODE(pAd, &pEntry->TxPhyCfg, MODE_OFDM);
                WRITE_PHY_CFG_MCS(pAd, &pEntry->TxPhyCfg, OfdmRateToRxwiMCS[pEntry->MaxSupportedRate]);             
            }
            pEntry->CapabilityInfo = CapabilityInfo;
            if (pEntry->AuthMode == Ralink802_11AuthModeWPAPSK)
            {
                pEntry->PrivacyFilter = Ndis802_11PrivFilter8021xWEP;
                pEntry->WpaState = AS_INITPSK;
                pEntry->PairwiseKey.CipherAlg = CIPHER_NONE;
                pEntry->GTKState = REKEY_NEGOTIATING;
            }
            else if ((pEntry->AuthMode >= Ralink802_11AuthModeWPA) && (pEntry->AuthMode != Ralink802_11AuthModeAutoSwitch))
            {
                pEntry->PrivacyFilter = Ndis802_11PrivFilter8021xWEP;
                pEntry->WpaState = AS_AUTHENTICATION;
                pEntry->PairwiseKey.CipherAlg = CIPHER_NONE;
                pEntry->GTKState = REKEY_NEGOTIATING;
            }

            if (bWmmCapable)
            {
                CLIENT_STATUS_SET_FLAG(pEntry, fCLIENT_STATUS_WMM_CAPABLE);
            }
            else
            {
                CLIENT_STATUS_CLEAR_FLAG(pEntry, fCLIENT_STATUS_WMM_CAPABLE);
            }

            if (ClientRalinkIe != 0x00)
                CLIENT_STATUS_SET_FLAG(pEntry, fCLIENT_STATUS_RALINK_CHIPSET);
            else
                CLIENT_STATUS_CLEAR_FLAG(pEntry, fCLIENT_STATUS_RALINK_CHIPSET);
            
            if (pPort->CommonCfg.bAggregationCapable)
            {
                if ((pPort->CommonCfg.bPiggyBackCapable) && (ClientRalinkIe & 0x00000003) == 3)
                {
                    CLIENT_STATUS_SET_FLAG(pEntry, fCLIENT_STATUS_AGGREGATION_CAPABLE);
                    CLIENT_STATUS_SET_FLAG(pEntry, fCLIENT_STATUS_PIGGYBACK_CAPABLE);
                    ApMlmeSetPiggyBack(pAd, TRUE);
                    DBGPRINT(RT_DEBUG_TRACE, ("ASSOC -PiggyBack= 1\n"));
                }
                else if (ClientRalinkIe & 0x00000001)
                {
                    CLIENT_STATUS_SET_FLAG(pEntry, fCLIENT_STATUS_AGGREGATION_CAPABLE);
                    CLIENT_STATUS_CLEAR_FLAG(pEntry, fCLIENT_STATUS_PIGGYBACK_CAPABLE);
                }
                else
                {
                    CLIENT_STATUS_CLEAR_FLAG(pEntry, fCLIENT_STATUS_AGGREGATION_CAPABLE);
                    CLIENT_STATUS_CLEAR_FLAG(pEntry, fCLIENT_STATUS_PIGGYBACK_CAPABLE);
                }
            }
            else
            {
                CLIENT_STATUS_CLEAR_FLAG(pEntry, fCLIENT_STATUS_AGGREGATION_CAPABLE);
                if ((pPort->CommonCfg.bPiggyBackCapable) && (ClientRalinkIe & 0x00000002) == 2)
                {
                    CLIENT_STATUS_SET_FLAG(pEntry, fCLIENT_STATUS_PIGGYBACK_CAPABLE);
                    ApMlmeSetPiggyBack(pAd, TRUE);
                    DBGPRINT(RT_DEBUG_TRACE, ("ASSOC -PiggyBack2= 1\n"));
                }
                else
                    CLIENT_STATUS_CLEAR_FLAG(pEntry, fCLIENT_STATUS_PIGGYBACK_CAPABLE);
            }
            
#if 0
            // If dynamic rate switching is enabled, starts from a more reliable rate to 
            // increase STA's DHCP succeed rate
            if (OPSTATUS_TEST_FLAG(pAd, fOP_STATUS_TX_RATE_SWITCH_ENABLED))
            {
                pEntry->CurrTxRate = min(pEntry->MaxSupportedRate, RATE_11); 
            }
            else
#endif                
                
            // If this Entry supports 802.11n, upgrade to HT rate. 
            if ((pPort->SoftAP.ApCfg.PhyMode >= PHY_11ABGN_MIXED) && (HtCapabilityLen != 0))
            {
                UCHAR   j, bitmask;
                CHAR    i;

                if ((pHtCapability->HtCapInfo.GF) && (pPort->CommonCfg.DesiredHtPhy.GF))
                {
                    WRITE_PHY_CFG_MODE(pAd, &pEntry->MaxPhyCfg, MODE_HTGREENFIELD);
                }
                else
                {   
                    WRITE_PHY_CFG_MODE(pAd, &pEntry->MaxPhyCfg, MODE_HTMIX);
                    pPort->MacTab.fAnyStationNonGF = TRUE;
                    pPort->CommonCfg.AddHTInfo.AddHtInfo2.NonGfPresent = 1;
                }
                /************************************************************************************************************/
                //Sync with station mode  when station connect with 20M mode AP
                //
                /************************************************************************************************************/
                if ((pPort->PortSubtype == PORTSUBTYPE_VwifiAP) || (pPort->PortSubtype == PORTSUBTYPE_P2PGO))           
                {
                    pStaPort = Ndis6CommonGetStaPort(pAd);
                    if(pStaPort != NULL)
                    {
                        if(OPSTATUS_TEST_FLAG(pStaPort,fOP_STATUS_MEDIA_STATE_CONNECTED))
                        {
                            if(pAd->StaActive.SupportedHtPhy.bHtEnable)
                            {
                                bStaUse40 =(BOOLEAN)pAd->StaActive.SupportedHtPhy.HtChannelWidth;                           
                            }
                            else
                            {
                                bStaUse40 = TRUE;
                            }
                        }
                        else
                        {
                            bStaUse40 = TRUE;
                        }
                    }
                }
                else
                {
                    bStaUse40 = TRUE;
                }

                // both softAP and STA are VHT capable.
                if((pPort->CommonCfg.PhyMode == PHY_11VHT)
                    && (pPort->Channel>14)        // just in case
                    && (VhtCapabilityLen != 0)) 
                {
                    // save VHT information of the connected client
                    pEntry->VhtPeerStaCtrl.bVhtCapable = TRUE;

                    // fixed the rate to maximum
                    WRITE_PHY_CFG_MCS(pAd, &pEntry->MaxPhyCfg, MCS_7);
                    WRITE_PHY_CFG_STBC(pAd, &pEntry->MaxPhyCfg,STBC_NONE);
                    WRITE_PHY_CFG_SHORT_GI(pAd, &pEntry->MaxPhyCfg, GI_400);
                    WRITE_PHY_CFG_MODE(pAd, &pEntry->MaxPhyCfg, MODE_VHT);
                    WRITE_PHY_CFG_BW(pAd, &pEntry->MaxPhyCfg,BW_80); 
                    WRITE_PHY_CFG_NSS(pAd, &pEntry->MaxPhyCfg,NSS_1);

                    PlatformMoveMemory(&pEntry->VhtPeerStaCtrl.VhtCapability,pVhtCapability,SIZE_OF_VHT_CAP_IE);
                }                               
                else if ((pHtCapability->HtCapInfo.ChannelWidth) && (pPort->CommonCfg.DesiredHtPhy.HtChannelWidth))
                {
                    WRITE_PHY_CFG_BW(pAd, &pEntry->MaxPhyCfg, BW_40);
                    WRITE_PHY_CFG_SHORT_GI(pAd, &pEntry->MaxPhyCfg, (pPort->CommonCfg.DesiredHtPhy.ShortGIfor40 && pHtCapability->HtCapInfo.ShortGIfor40));
                }
                else
                {   
                    WRITE_PHY_CFG_BW(pAd, &pEntry->MaxPhyCfg, BW_20);
                    WRITE_PHY_CFG_SHORT_GI(pAd, &pEntry->MaxPhyCfg, (pPort->CommonCfg.DesiredHtPhy.ShortGIfor20 && pHtCapability->HtCapInfo.ShortGIfor20));

                    pPort->MacTab.fAnyStation20Only  = TRUE;
                }

                if ((pPort->CommonCfg.DesiredHtPhy.MCSSet[4] != 0))
                {
                    WRITE_PHY_CFG_MCS(pAd, &pEntry->MaxPhyCfg, MCS_32);
                }
                for (i = 23 ;i >= 0;i--)
                {   
                    j = i/8;    
                    bitmask = (1<<(i-(j*8)));
                    if ( (pPort->CommonCfg.DesiredHtPhy.MCSSet[j]&bitmask) && (pHtCapability->MCSSet[j]&bitmask))
                    {
                        WRITE_PHY_CFG_MCS(pAd, &pEntry->MaxPhyCfg, i);
                        DBGPRINT(RT_DEBUG_TRACE,("ASSOC<=== Use Fixed MCS = %d\n", READ_PHY_CFG_MCS(pAd, &pEntry->MaxPhyCfg)));
                        break;
                    }
                    if (i==0)
                        break;
                }

                if ((READ_PHY_CFG_MCS(pAd, &pEntry->MaxPhyCfg) > READ_PHY_CFG_MCS(pAd, &pPort->SoftAP.ApCfg.TxPhyCfg)) && 
                     (READ_PHY_CFG_MCS(pAd, &pPort->SoftAP.ApCfg.TxPhyCfg) != MCS_AUTO))
                {
                    WRITE_PHY_CFG_MCS(pAd, &pEntry->MaxPhyCfg, READ_PHY_CFG_MCS(pAd, &pPort->SoftAP.ApCfg.TxPhyCfg));
                }               
                DBGPRINT(RT_DEBUG_TRACE,("ASSOC<=== pPort->CommonCfg.DesiredHtPhy.MCSSet[4] = %d\n", pPort->CommonCfg.DesiredHtPhy.MCSSet[4]));
                
                WRITE_PHY_CFG_STBC(pAd, &pEntry->MaxPhyCfg, (pHtCapability->HtCapInfo.RxSTBC && pPort->CommonCfg.DesiredHtPhy.TxSTBC));
                
                //
                // SVD team: Atheros AP/station will advertise 0 in HT capability.
                // For IOT issue, MPDU density will not set to less than 5(4us).
                //
                pEntry->MpduDensity = max(5, pHtCapability->HtCapParm.MpduDensity);
                pEntry->MaxRAmpduFactor = pHtCapability->HtCapParm.MaxRAmpduFactor;
                pEntry->BaSizeInUse =(UCHAR) pPort->CommonCfg.BACapability.field.TxBAWinLimit;
                
                // 11.2.3.1
                pEntry->MmpsMode = (UCHAR)pHtCapability->HtCapInfo.MimoPs;
                DBGPRINT(RT_DEBUG_TRACE,("ASSOC === pEntry->MmpsMode= %d\n", pEntry->MmpsMode));
                pEntry->AMsduSize = (UCHAR)pHtCapability->HtCapInfo.AMsduSize;

                WRITE_PHY_CFG_DOUBLE_WORD(pAd, &pEntry->TxPhyCfg, READ_PHY_CFG_DOUBLE_WORD(pAd, &pEntry->MaxPhyCfg));
                if (pPort->CommonCfg.DesiredHtPhy.AmsduEnable)
                    CLIENT_STATUS_SET_FLAG(pEntry, fCLIENT_STATUS_AMSDU_INUSED);
                if (pHtCapability->HtCapInfo.ShortGIfor20)
                    CLIENT_STATUS_SET_FLAG(pEntry, fCLIENT_STATUS_SGI20_CAPABLE);
                if (pHtCapability->HtCapInfo.ShortGIfor40)
                    CLIENT_STATUS_SET_FLAG(pEntry, fCLIENT_STATUS_SGI40_CAPABLE);
                if (pHtCapability->HtCapInfo.TxSTBC)
                    CLIENT_STATUS_SET_FLAG(pEntry, fCLIENT_STATUS_HT_TX_STBC_CAPABLE);
                if (pHtCapability->HtCapInfo.RxSTBC)
                    CLIENT_STATUS_SET_FLAG(pEntry, fCLIENT_STATUS_HT_RX_STBC_CAPABLE);
        
                if (pPort->CommonCfg.bRdg && pHtCapability->ExtHtCapInfo.RDGSupport)
                {
                    CLIENT_STATUS_SET_FLAG(pEntry, fCLIENT_STATUS_RDG_CAPABLE);
                    DBGPRINT(RT_DEBUG_TRACE, ("ASSOC, RDG enabled \n"));
                }
                if (pPort->CommonCfg.BACapability.field.AutoBA == TRUE)
                {
                    UCHAR   idx;
                    for (idx = 0; idx <NUM_OF_TID; idx++)
                    {
                        // If add myself as one Recipient of peer (pPort->PortCfg.Bssid), the sequence 0xffff indicates the BA
                        // session not started yet. 
                        BATableInsertEntry(pAd, pPort,(UCHAR)pEntry->Aid,  0, 0x1, idx, pEntry->BaSizeInUse,  Originator_SetAutoAdd, FALSE);
                        pEntry->TXAutoBAbitmap |= (1<<idx);
                    }
                    DBGPRINT(RT_DEBUG_ERROR, ("ASSOC - TXBAbitmap = 0x%x, RXBAbitmap = 0x%xd\n",   pEntry->TXBAbitmap, pEntry->RXBAbitmap));
                }
            }
            else
                pPort->MacTab.fAnyStationIsLegacy = TRUE;
                
            WRITE_PHY_CFG_DOUBLE_WORD(pAd, &pEntry->TxPhyCfg, READ_PHY_CFG_DOUBLE_WORD(pAd, &pEntry->MaxPhyCfg));

            PlatformMoveMemory(&pEntry->HTCapability, pHtCapability, sizeof(HT_CAPABILITY_IE));
        
            pEntry->CurrTxRate = pEntry->MaxSupportedRate;
            
            ApMlmeSelectTxRateTable(pAd, pPort,pEntry, &pTable, &TableSize, &pEntry->CurrTxRateIndex);
            AsicUpdateAutoFallBackTable(pAd, pTable, pEntry);
        
            if ((pEntry->AuthMode < Ralink802_11AuthModeWPA) || (pEntry->AuthMode == Ralink802_11AuthModeAutoSwitch))
                ApLogEvent(pAd, pEntry->Addr, EVENT_ASSOCIATED);
            
            APUpdateCapabilityAndErpIe(pAd);
            APUpdateOperationMode(pAd);

            pEntry->ReTryCounter = PEER_MSG1_RETRY_TIMER_CTR;
            
            DBGPRINT(RT_DEBUG_TRACE, ("ASSOC - Update AP OperaionMode=%d , fAnyStationIsLegacy=%d, fAnyStation20Only=%d, fAnyStationNonGF=%d \n", 
                pPort->CommonCfg.AddHTInfo.AddHtInfo2.OperaionMode, pPort->MacTab.fAnyStationIsLegacy, pPort->MacTab.fAnyStation20Only , pPort->MacTab.fAnyStationNonGF));
            DBGPRINT(RT_DEBUG_TRACE, ("ASSOC - WMM=%d, AuthMode=%s, WepStatus=%s, WpaState=%d, GroupKeyWepStatus=%d, flags =%x\n",
                bWmmCapable, decodeAuthAlgorithm(pEntry->AuthMode), decodeCipherAlgorithm(pEntry->WepStatus), pEntry->WpaState, pPort->PortCfg.GroupKeyWepStatus,pEntry->ClientStatusFlags));
            DBGPRINT(RT_DEBUG_TRACE, ("ASSOC - Assign AID=%d to STA %02x:%02x:%02x:%02x:%02x:%02x, MaxSupRate=%d Mbps, CurrTxRate=%d Mbps\n", 
                *pAid, pEntry->Addr[0], pEntry->Addr[1], pEntry->Addr[2], pEntry->Addr[3], pEntry->Addr[4], pEntry->Addr[5], 
                RateIdToMbps[pEntry->MaxSupportedRate], RateIdToMbps[pEntry->CurrTxRate]));

            MtAsicUpdateRxWCIDTable(pAd, *pAid,  pEntry->Addr);

            //
            // TODO: Remove this line to support AP dynamic TX Rate.
            //
            pEntry->pTable = RateSwitchTableAdapt11N2S;
            pEntry->LowestTxRateIndex = 19;            
            
            MtAsicMcsLutUpdate(pAd, pEntry);

            // If wep, set key here.
            if (IS_WEP_STATUS_ON(pEntry->WepStatus))
            {
                pEntry->PairwiseKey.CipherAlg = pPort->SharedKey[BSS0][pPort->PortCfg.DefaultKeyId].CipherAlg;
                pEntry->PairwiseKey.KeyLen = pPort->SharedKey[BSS0][pPort->PortCfg.DefaultKeyId].KeyLen;
                PlatformMoveMemory(pEntry->PairwiseKey.Key, pPort->SharedKey[BSS0][pPort->PortCfg.DefaultKeyId].Key, 16);
            }
        
            DBGPRINT(RT_DEBUG_TRACE, ("%s - CurrTxRate=%d, MODE=%d, BW=%d, MCS=%d, ShortGI=%d, MaxRxFactor=%d, MpduDensity=%d\n", 
                __FUNCTION__,
                pEntry->CurrTxRate, 
                READ_PHY_CFG_MODE(pAd, &pEntry->TxPhyCfg), 
                READ_PHY_CFG_BW(pAd, &pEntry->TxPhyCfg), 
                READ_PHY_CFG_MCS(pAd, &pEntry->TxPhyCfg), 
                READ_PHY_CFG_SHORT_GI(pAd, &pEntry->TxPhyCfg),
                pEntry->MaxRAmpduFactor, 
                pEntry->MpduDensity));
            
            DBGPRINT(RT_DEBUG_TRACE, ("%s - MaxRAmpduFactor=%d, MmpsMode=%d, AMsduSize=%d, BaSizeInUse=%d, ClientStatusFlags = %x \n", 
                __FUNCTION__, pEntry->MaxRAmpduFactor, pEntry->MmpsMode, pEntry->AMsduSize, pEntry->BaSizeInUse, pEntry->ClientStatusFlags));
            // automatically add BA session for this associated Client
            // ======================================================>>>>>>
            // WiFi Direct  GO
            if ((pPort->PortSubtype == PORTSUBTYPE_P2PGO))
            {
#if(COMPILE_WIN8_ABOVE(CURRENT_OS_NDIS_VER))
                if (pPort->PortType == WFD_GO_PORT)
                {
                        P2pMsApAssocGoBuildAssociation(pAd, pPort, pEntry, RSN, pRSNLen, pP2pElement, 
                        P2pElementLen, pWpsElement, WpsElementLen, pHtCapability, HtCapabilityLen);
                }
                else
#endif
                {
                    ApAssocGoBuildAssociation(pAd, pPort, pEntry, RSN, pRSNLen, pP2pElement, 
                        P2pElementLen, pWpsElement, WpsElementLen, pHtCapability, HtCapabilityLen);
                    // this LockNego = FALSE is for reinvoke to avoid peer connection interrupt
                    pPort->P2PCfg.LockNego = FALSE;
                }
            }
            // <<<<========================================================

            StatusCode = MLME_SUCCESS;
            if (pPort->PortSubtype == PORTSUBTYPE_SoftapAP)
                APIndicateAssociationStatus(pAd, pEntry->Addr);
        }
    }
    else // CLASS 3 error should have been handled beforehand; here should be MAC table full
        StatusCode = MLME_ASSOC_REJ_UNABLE_HANDLE_STA;

    return StatusCode;
}

