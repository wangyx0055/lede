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
    rtmp_info.c

    Abstract:
    Miniport Query information related subroutines

    Revision History:
    Who         When          What
    --------    ----------    ----------------------------------------------
    Paul Lin    2002-08-10    created
    John Chang  2004-08-20    modified for RT2561/2661
*/
#include    "MtConfig.h"

extern PCHAR    DbgGetOidName( ULONG Oid);

NDIS_STATUS
MpScanRequest(
    IN PMP_ADAPTER pAd ,
    IN  PMP_PORT pPort ,
    IN PVOID InformationBuffer,
    IN ULONG InformationBufferLength,
    OUT PULONG BytesRead,
    OUT PULONG BytesNeeded
    )
{
    NDIS_STATUS                 ndisStatus = NDIS_STATUS_SUCCESS;
    PDOT11_SCAN_REQUEST_V2      pDot11ScanRequest = InformationBuffer;
    ULONG                       i, BytesParsed = 0;
    PDOT11_SSID             pDot11SSID;
    BOOLEAN                     bReturn = FALSE;
    
    DBGPRINT(RT_DEBUG_TRACE, ("--> MpScanRequest\n"));

    do
    {
        *BytesRead = 0;

        if (MT_TEST_FLAG(pAd, fRTMP_ADAPTER_RADIO_OFF))
        {
            DBGPRINT(RT_DEBUG_TRACE, ("Nic is currently turned off\n"));
            ndisStatus = NDIS_STATUS_DOT11_POWER_STATE_INVALID ;
            break;
        }

        if (IS_P2P_SIGMA_ON(pPort) && IS_P2P_GO_OP(pPort) && (pAd->OpMode == OPMODE_STA))
        {
            DBGPRINT(RT_DEBUG_TRACE, ("Nic is currently as GO. Don't support scan request from GUI. \n"));
            ndisStatus = NDIS_STATUS_INVALID_LENGTH;
            break;
        }

        *BytesNeeded = sizeof(DOT11_SCAN_REQUEST_V2);
        if (InformationBufferLength < sizeof(DOT11_SCAN_REQUEST_V2))
        {
            DBGPRINT(RT_DEBUG_ERROR, ("Bad length provided for scan. Length needed: %d, Length provided: %d\n", sizeof(DOT11_SCAN_REQUEST_V2), InformationBufferLength));
            ndisStatus = NDIS_STATUS_INVALID_LENGTH;
            *BytesRead = InformationBufferLength;
            break;
        }
        *BytesRead = sizeof(DOT11_SCAN_REQUEST_V2);

        //
        // Perform some validation on the scan request.
        //
        if (pDot11ScanRequest->uNumOfdot11SSIDs == 0)
        {
            DBGPRINT(RT_DEBUG_ERROR, ("No SSID found in the scan data\n"));
            ndisStatus = NDIS_STATUS_INVALID_DATA;
            break;
        }

        for (i = 0; i < pDot11ScanRequest->uNumOfdot11SSIDs; i++)
        {
            pDot11SSID = (PDOT11_SSID) (pDot11ScanRequest->ucBuffer + pDot11ScanRequest->udot11SSIDsOffset + BytesParsed);
            if (pDot11SSID->uSSIDLength > DOT11_SSID_MAX_LENGTH)
            {
                DBGPRINT(RT_DEBUG_ERROR, ("The SSID length provided (%d) is greater than max SSID length (%d)\n", pDot11SSID->uSSIDLength, DOT11_SSID_MAX_LENGTH));
                ndisStatus = NDIS_STATUS_INVALID_LENGTH;
                return ndisStatus;
            }
            BytesParsed += sizeof(DOT11_SSID);
            DBGPRINT(RT_DEBUG_TRACE, ("ScanRequest::SSID(Len=%d,%c%c%c...)\n", 
                    pDot11SSID->uSSIDLength, pDot11SSID->ucSSID[0], pDot11SSID->ucSSID[1], pDot11SSID->ucSSID[2]));
        }

        if (pDot11ScanRequest->dot11BSSType != dot11_BSS_type_infrastructure &&
            pDot11ScanRequest->dot11BSSType != dot11_BSS_type_independent &&
            pDot11ScanRequest->dot11BSSType != dot11_BSS_type_any)
        {
            DBGPRINT(RT_DEBUG_ERROR, ("BSS Type %d not supported\n", pDot11ScanRequest->dot11BSSType));
            ndisStatus = NDIS_STATUS_NOT_SUPPORTED;
            break;
        }

        switch (pDot11ScanRequest->dot11ScanType)
        {
            case dot11_scan_type_active:
            case dot11_scan_type_active | dot11_scan_type_forced:
            case dot11_scan_type_passive:
            case dot11_scan_type_passive | dot11_scan_type_forced:
            case dot11_scan_type_auto:
            case dot11_scan_type_auto | dot11_scan_type_forced:
                break;

            default:
                DBGPRINT(RT_DEBUG_ERROR, ("Dot11 scan type %d not supported\n", pDot11ScanRequest->dot11ScanType));
                ndisStatus = NDIS_STATUS_NOT_SUPPORTED;
                return ndisStatus;
        }

        //
        // Save this ScanReuestID.
        // Reference to pAd->PendedRequest->RequestId later will crash cause by reference to free memory, happen once.
        // It seems that NDIS has completed this NDIS Request. 
        //
        pAd->pNicCfg->ScanRequestID = pAd->pNicCfg->PendedScanRequest->RequestId;
        
        if ((pAd->pHifCfg->BulkLastOneSecCount > 30) && (pAd->pNicCfg->bIsClearScanTab != TRUE))
        {
            DBGPRINT(RT_DEBUG_TRACE, ("Set::OID_DOT11_SCAN_REQUEST, Heavy traffic on TX/RX ignore this set::OID_DOT11_SCAN_REQUEST\n"));
            PlatformIndicateScanStatus(pAd, pPort, ndisStatus, TRUE, FALSE);
            
            break;
        }

    
        if ((pAd->StaCfg.SmartScan && (pAd->pNicCfg->bIsClearScanTab != TRUE) && (INFRA_ON(pPort) || ADHOC_ON(pPort) || (pPort->SoftAP.bAPStart == TRUE))) ||
            ((pAd->StaCfg.OzmoDeviceSuppressScan != 0) && (pAd->pNicCfg->bIsClearScanTab != TRUE)) ||
            (pAd->LogoTestCfg.WhckCpuUtilTestRunning == TRUE))
        {
            ULONGLONG Now;

            NdisGetCurrentSystemTime((PLARGE_INTEGER)&Now);
            Now = Now/10000;    // convert as ms
        
            if ((((Now - pAd->StaCfg.LastScanTimeFromNdis) > 55*1000) && ((Now - pAd->StaCfg.LastScanTimeFromNdis) < 65*1000)) ||
                (((Now - pAd->StaCfg.LastScanTimeFromNdis) > 115*1000) && ((Now - pAd->StaCfg.LastScanTimeFromNdis) < 125*1000)) ||
                (((Now - pAd->StaCfg.LastScanTimeFromNdis) > 175*1000) && ((Now - pAd->StaCfg.LastScanTimeFromNdis) < 185*1000)))
            {
                DBGPRINT(RT_DEBUG_TRACE, ("Ignore zero config periodic scan, LastScanTimeFromNdis=%I64d, Now=%I64d, diff=%I64d \n",
                    pAd->StaCfg.LastScanTimeFromNdis, Now, (Now - pAd->StaCfg.LastScanTimeFromNdis)));
                pAd->StaCfg.LastScanTimeFromNdis = Now;
                PlatformIndicateScanStatus(pAd, pPort, ndisStatus, TRUE, FALSE);
                break;
            }
            else if (pAd->StaCfg.OzmoDeviceSuppressScan == 2)
            {
                DBGPRINT(RT_DEBUG_TRACE, ("Ignore zero config OzmoDevice, LastScanTimeFromNdis=%I64d, Now=%I64d, diff=%I64d \n",
                    pAd->StaCfg.LastScanTimeFromNdis, Now, (Now - pAd->StaCfg.LastScanTimeFromNdis)));
                PlatformIndicateScanStatus(pAd, pPort, ndisStatus, TRUE, FALSE);
                break;          
            }
            else if (pAd->LogoTestCfg.WhckCpuUtilTestRunning == TRUE)
            {
                DBGPRINT(RT_DEBUG_TRACE, ("Ignore zero config OzmoDevice, CPU Util\n"));
                PlatformIndicateScanStatus(pAd, pPort, ndisStatus, TRUE, FALSE);
                break;
            }

            pAd->StaCfg.LastScanTimeFromNdis = Now;
        }


        // To decrease the possibility of ping loss, use 2-step site survey mechanism==>After every 7 channels, go back to working channel
        // for a while, then continue scan again until finish scaning all the valid channels. 
        // Because the duration stayed in every channel is 30ms, so this kind of scan mechanism only apply to when traffic is very low (ping test).
        if ((pAd->Counter.MTKCounters.Last30SecTotalRxCount < 100) 
            && (pAd->Counter.MTKCounters.Last30SecTotalRxCount > 24) 
            && (pPort->PortType == EXTSTA_PORT)  
            && (INFRA_ON(pPort)))
        {
            pAd->StaCfg.bFastRoamingScan = TRUE;
            pAd->StaCfg.bImprovedScan = TRUE;
            pAd->StaCfg.ScanChannelCnt = 0; // reset channel counter to 0
            DBGPRINT(RT_DEBUG_INFO, ("Set:: 30sRxCnt = %d . SsidLen = %d\n", pAd->Counter.MTKCounters.Last30SecTotalRxCount, pAd->MlmeAux.SsidLen));
        }
            
        //WLK scan not confirm, so force reset state machine.
        if (pPort->Mlme.CntlMachine.CurrState != CNTL_IDLE)
        {
            MlmeRestartStateMachine(pAd,pPort);
            DBGPRINT(RT_DEBUG_TRACE, ("!!! MLME busy, reset MLME state machine !!!\n"));
        }
        
        pAd->pNicCfg->bIsClearScanTab = FALSE;   
        pAd->MlmeAux.bNeedPlatformIndicateScanStatus = TRUE;
        pAd->pNicCfg->bUpdateBssList = TRUE;

        // For p2p fast discovery, reduce to stay in non-listen channel or non-operation channel
        if(pAd->LogoTestCfg.OnTestingWHQL && pPort->PortNumber == 0)
        {
            UCHAR   WHQLChannelOrder[11] = {1, 3, 6, 8, 11, 2, 4, 5, 7, 9, 10};

            for(i = 0; i < MAX_NUM_OF_CHANNELS; i++)
            {
                if(i < 11)
                    pAd->HwCfg.ChannelList[i].Channel = WHQLChannelOrder[i];
                else
                    pAd->HwCfg.ChannelList[i].Channel = 0;

                DBGPRINT(RT_DEBUG_TRACE, ("port 0 scan channel = %d", pAd->HwCfg.ChannelList[i].Channel));
            }
            
            pAd->HwCfg.ChannelListNum = 11;
        }

        
        //
        // Pass the scan request
        //  
        bReturn = MlmeEnqueue(pAd,
                    pPort,
                    MLME_CNTL_STATE_MACHINE,
                    OID_DOT11_SCAN_REQUEST,
                    InformationBufferLength,
                    InformationBuffer);
        if(bReturn)
        {
            //set this flag to prevent another port scan issue
            MT_SET_FLAG(pAd, fRTMP_ADAPTER_BSS_SCAN_IN_PROGRESS); 
        }
        else
        {
            // Prevent scan request can't fire again.
            DBGPRINT(RT_DEBUG_TRACE, ("############## Enqueue scan request failed. ##############\n"));

            PlatformIndicateScanStatus(pAd, pPort, NDIS_STATUS_SUCCESS, FALSE, FALSE);
        }
        
    } while (FALSE);

    DBGPRINT(RT_DEBUG_TRACE, ("<-- MpScanRequest (Scan type:0x%08x)\n", pDot11ScanRequest->dot11ScanType));

    return ndisStatus;
}

NDIS_STATUS
RTMPInfoResetRequest(
    IN  PMP_ADAPTER pAd,
    IN  PMP_PORT pPort,
    IN OUT PVOID InformationBuffer,
    IN  ULONG InputBufferLength,
    IN  ULONG OutputBufferLength,
    OUT PULONG BytesRead,
    OUT PULONG BytesWritten,
    OUT PULONG BytesNeeded
    )
{   
    PDOT11_RESET_REQUEST pDot11ResetRequest = InformationBuffer;
    PDOT11_STATUS_INDICATION pDot11StatusIndication = InformationBuffer;

    DBGPRINT(RT_DEBUG_TRACE, ("==> RTMPInfoResetRequest pPort = %d\n",pPort->PortNumber));

    //avoid inforeset  between 4wayHS m1 and m2
    if(pPort->P2PCfg.P2PConnectState == P2P_GO_ASSOC_AUTH) 
    {
        DBGPRINT(RT_DEBUG_TRACE, ("P2P_GO_ASSOC_AUTH  return....\n"));
        return NDIS_STATUS_SUCCESS;
    }
    //
    // First make sure the input buffer is large enough to
    // hold a RESET_CONFIRM
    //
    if (OutputBufferLength < sizeof(DOT11_STATUS_INDICATION)) {
        *BytesNeeded = sizeof(DOT11_STATUS_INDICATION);
        return NDIS_STATUS_INVALID_LENGTH;
    }

    //
    // Validate the buffer length
    //
    if (InputBufferLength < sizeof(DOT11_RESET_REQUEST)) {
        *BytesNeeded = sizeof(DOT11_RESET_REQUEST);
        return NDIS_STATUS_INVALID_LENGTH;
    }

    //
    // Validate the buffer
    //
    switch (pDot11ResetRequest->dot11ResetType) {
        case dot11_reset_type_phy:
        case dot11_reset_type_mac:
        case dot11_reset_type_phy_and_mac:
            break;

        default:
            DBGPRINT(RT_DEBUG_ERROR, ("RTMPInfoResetRequest: Unknown ResetType=%d\n", pDot11ResetRequest->dot11ResetType));
            return NDIS_STATUS_INVALID_DATA;
    }

    //for WFD NDIStest, GO should not reset by other reset type excepts dot11_reset_type_phy_and_mac
    if ((pPort->PortType == WFD_GO_PORT) && (pDot11ResetRequest->dot11ResetType != dot11_reset_type_phy_and_mac))
    {
        return NDIS_STATUS_INVALID_DATA;
    }

    *BytesRead = sizeof(DOT11_RESET_REQUEST);
    // Note  The miniport driver must not set the value of the BytesWritten member of the OidRequest parameter. 
    //*BytesWritten = sizeof(DOT11_STATUS_INDICATION);
    
        
    //1.)Pause the beaconing
    //2.)Stop periodic scan
    //3.)Cleanup connection context
    //4.)Reset the state we had maintained about for roaming
    //5.)Reset the AdHoc station Info but do not clear AdHoc station list.
    //6.)Reset configuration (including resetting cipher and key on hardware) [by calling N6StaCfgInit()]
    //7.)Clears PMKID cache
    
    if ((pPort->PortType == EXTSTA_PORT) && (pPort->PortNumber == PORT_0))
    {
        N6StaCfgInit(pAd, pDot11ResetRequest->bSetDefaultMIB);
    }

    //Clear the desired SSID
    PlatformZeroMemory(&pPort->PortCfg.DesiredSSID, sizeof(DOT11_SSID));

    // Clear the desired BSSID list
    pPort->PortCfg.DesiredBSSIDList[0][0] = 0xFF;
    pPort->PortCfg.DesiredBSSIDList[0][1] = 0xFF;
    pPort->PortCfg.DesiredBSSIDList[0][2] = 0xFF;
    pPort->PortCfg.DesiredBSSIDList[0][3] = 0xFF;
    pPort->PortCfg.DesiredBSSIDList[0][4] = 0xFF;
    pPort->PortCfg.DesiredBSSIDList[0][5] = 0xFF;
    pPort->PortCfg.DesiredBSSIDCount = 1;
    pPort->PortCfg.AcceptAnyBSSID = TRUE;
    DBGPRINT(RT_DEBUG_TRACE, ("%s(%d): Reset DesiredBSSIDList and AcceptAnyBSSID.\n", __FUNCTION__, pPort->PortNumber));

    // Clear Additional IE data by each port.
    Ndis6CommonPortFreeVariableIE(pPort);
    // Clear PrivacyExemptionList data by each port.
    Ndis6CommonPortFreePrivacyExemptionList(pPort);
    
    N6PortN6StaCfgInit( pPort);
    RTMPSetDefaultAuthAlgo(pAd,pPort);

    PlatformIndicateResetAllStatus(pAd, pPort);

    //put  i/o operation into command thread
    if(KeGetCurrentIrql() == DISPATCH_LEVEL)
    {
        MTEnqueueInternalCmd(pAd, pPort, MT_CMD_STA_RESET_REQUEST,&(pPort->PortNumber),sizeof(NDIS_PORT_NUMBER));   
    }   
    else
    {
        NdisCommonInfoResetHandler(pPort);
    }

    //
    // Complete the reset request with appropriate status to NDIS
    //
    pDot11StatusIndication->uStatusType = DOT11_STATUS_RESET_CONFIRM;
    pDot11StatusIndication->ndisStatus = NDIS_STATUS_SUCCESS;
    
    DBGPRINT(RT_DEBUG_TRACE, ("<== RTMPInfoResetRequest\n"));
    return NDIS_STATUS_SUCCESS;
}

NDIS_STATUS
Hw11QueryDot11Statistics(
    IN  PMP_ADAPTER pAd,
    IN  PDOT11_STATISTICS pDot11Stats,
    IN  ULONG InformationBufferLength,
    OUT PULONG BytesWritten,
    OUT PULONG BytesNeeded
    )
{
    ULONG                           statsSize;
    PDOT11_PHY_FRAME_STATISTICS     PhyStats;
    ULONG                           PhyId = 0;

    statsSize = sizeof(DOT11_STATISTICS) + (pAd->StaCfg.pSupportedPhyTypes->uNumOfEntries - 1) * sizeof(DOT11_PHY_FRAME_STATISTICS);
    if (InformationBufferLength < statsSize)
    {
        *BytesNeeded = statsSize;
        return NDIS_STATUS_BUFFER_OVERFLOW;
    }

    // TKIP MIC Error Counter
    pDot11Stats->ullTKIPCounterMeasuresInvoked = pAd->Counter.StatisticsInfo.ullTKIPCounterMeasuresInvoked;

    // Unicast counters
    PlatformMoveMemory(&pDot11Stats->MacUcastCounters, 
                    &pAd->Counter.StatisticsInfo.UcastCounters, 
                    sizeof(DOT11_MAC_FRAME_STATISTICS)
                    );

    // Multicast counters
    PlatformMoveMemory(&pDot11Stats->MacMcastCounters, 
                    &pAd->Counter.StatisticsInfo.McastCounters, 
                    sizeof(DOT11_MAC_FRAME_STATISTICS)
                    );

    // Phy counters
    for (PhyId = 0, PhyStats = pDot11Stats->PhyCounters; 
            PhyId < pAd->StaCfg.pSupportedPhyTypes->uNumOfEntries; 
            PhyId++, PhyStats++
        )
    {
        PlatformMoveMemory(PhyStats, 
                        &pAd->Counter.StatisticsInfo.PhyCounters[PhyId], 
                        sizeof(DOT11_PHY_FRAME_STATISTICS)
                        );
    }

    *BytesWritten = statsSize;

    return NDIS_STATUS_SUCCESS;
}

NDIS_STATUS
HwQueryStatistics(   IN PMP_ADAPTER pAd,
    IN OUT PVOID InformationBuffer,
    IN ULONG InformationBufferLength,
    IN OUT PULONG BytesWritten,
    IN OUT PULONG BytesNeeded)
{
    NDIS_STATUS ndisStatus = NDIS_STATUS_SUCCESS;
    PDOT11_STATISTICS       pDot11Stats;

    do
    {
        *BytesWritten = 0;
        *BytesNeeded = 0;

        if (InformationBufferLength < sizeof(DOT11_STATISTICS))
        {
            *BytesNeeded = sizeof(DOT11_STATISTICS);
            ndisStatus = NDIS_STATUS_BUFFER_OVERFLOW;
            break;
        }
        PlatformZeroMemory(InformationBuffer, InformationBufferLength);

        pDot11Stats = (PDOT11_STATISTICS)InformationBuffer;

        MP_ASSIGN_NDIS_OBJECT_HEADER(pDot11Stats->Header, 
                                     NDIS_OBJECT_TYPE_DEFAULT,
                                     DOT11_STATISTICS_REVISION_1,
                                     sizeof(DOT11_STATISTICS));

        *BytesWritten = 0;
        *BytesNeeded = 0;   

        ndisStatus = Hw11QueryDot11Statistics(
            pAd,
            pDot11Stats,
            InformationBufferLength,
            BytesWritten,
            BytesNeeded
            );

        if (ndisStatus == NDIS_STATUS_SUCCESS)
        {
            // Excluded counts are kept by the station
            pDot11Stats->MacUcastCounters.ullWEPExcludedCount = pAd->Counter.StatisticsInfo.ullUcastWEPExcludedCount;
            pDot11Stats->MacMcastCounters.ullWEPExcludedCount = pAd->Counter.StatisticsInfo.ullMcastWEPExcludedCount;
        }

    } while(FALSE);

    return ndisStatus;
}

//
// Perform a disconnection operation from the BSS network with which the STA is connected.
//
NDIS_STATUS
DisconnectRequest(
    IN PMP_ADAPTER pAd,
    IN PMP_PORT   pPort,
    IN OUT PULONG BytesRead,
    IN OUT PULONG BytesNeeded, 
    IN BOOLEAN bHwRadioOff
    )
{
    NDIS_STATUS ndisStatus = NDIS_STATUS_SUCCESS;
    BOOLEAN Cancelled;
    
    *BytesRead = 0;
    *BytesNeeded = 0;

// Call it after sending disassociation frame
// 2011-11-27 For P2pMs GO multi-channel
#if 0//#ifdef MULTI_CHANNEL_SUPPORT
    MultiChannelStopCLIAction(pAd, pPort);
#endif /* MULTI_CHANNEL_SUPPORT */  

    DBGPRINT(RT_DEBUG_TRACE, ("Set::OID_DOT11_DISCONNECT_REQUEST (PortNum=%d)\n", pPort->PortNumber));              
    // Set the AutoReconnectSsid to prevent it reconnect to old SSID
    // Since calling this indicate user don't want to connect to that SSID anymore.
    pAd->MlmeAux.AutoReconnectSsidLen= 32;
    pAd->MlmeAux.AutoReconnectStatus = FALSE;
    PlatformZeroMemory(pAd->MlmeAux.AutoReconnectSsid, pAd->MlmeAux.AutoReconnectSsidLen);
    DBGPRINT(RT_DEBUG_TRACE, ("DisconnectRequest: Reset automatically reconnection first.\n"));

    // ==> WPS

    // Turn off WSC Actions before a new connection
    if (pPort->StaCfg.WscControl.WscState != WSC_STATE_OFF)
    {
        WscStop(pAd, pPort);
    }

    // Reset the desired BSSID list that set by the Ralink Vista UI with MTK_OID_N6_SET_DESIRED_BSSID_LIST.
    // In this way, other connection request is allowed again.
    if (pAd->StaCfg.bSetDesiredBSSIDListFromRalinkUI == TRUE)
    {
        pAd->PortList[PORT_0]->PortCfg.DesiredBSSIDList[0][0] = 0xFF;
        pAd->PortList[PORT_0]->PortCfg.DesiredBSSIDList[0][1] = 0xFF;
        pAd->PortList[PORT_0]->PortCfg.DesiredBSSIDList[0][2] = 0xFF;
        pAd->PortList[PORT_0]->PortCfg.DesiredBSSIDList[0][3] = 0xFF;
        pAd->PortList[PORT_0]->PortCfg.DesiredBSSIDList[0][4] = 0xFF;
        pAd->PortList[PORT_0]->PortCfg.DesiredBSSIDList[0][5] = 0xFF;
        pAd->PortList[PORT_0]->PortCfg.DesiredBSSIDCount = 1;
        pAd->PortList[PORT_0]->PortCfg.AcceptAnyBSSID = TRUE;
    }

        // Turn the WPS LEDs off.
        if (pPort->StaCfg.WscControl.bSkipWPSTurnOffLED == FALSE)
        {
            if((pPort->StaCfg.WscControl.bErrDeteWPSTermination == FALSE)&&(pPort->StaCfg.WscControl.bSessionOverlap == FALSE)&&(pPort->StaCfg.WscControl.bSkipWPSTurnOffLEDAfterSuccess ==  FALSE))
            {
                LedCtrlSetBlinkMode(pAd, LED_WPS_TURN_LED_OFF);
            }

            if(((pAd->HwCfg.LedCntl.field.LedMode & LED_MODE) == WPS_LED_MODE_9) && pPort->StaCfg.WscControl.bWPSSession && 
                (pPort->StaCfg.WscControl.WscStatus == STATUS_WSC_EAP_CONFIGURED))
            {
                    
            }
            else{
                if(!pPort->StaCfg.WscControl.WscLEDMode == LED_WPS_SESSION_OVERLAP_DETECTED)
                {
                    // Cancel the WPS LED timer.
                    PlatformCancelTimer(&pPort->StaCfg.WscControl.WscLEDTimer, &Cancelled);
                    pPort->StaCfg.WscControl.bIndicateConnNotTurnOff = FALSE;

                }
            }

            // Complete WPS session.
            pPort->StaCfg.WscControl.bWPSSession = FALSE;
        }

        if((pPort->StaCfg.WscControl.bSkipWPSTurnOffLED == TRUE) &&
            ((pAd->HwCfg.LedCntl.field.LedMode & LED_MODE) == WPS_LED_MODE_9) &&
            (pPort->StaCfg.WscControl.bWPSSession) &&
            (pPort->StaCfg.WscControl.WscStatus == STATUS_WSC_EAP_CONFIGURED))
        {
            pPort->StaCfg.WscControl.bWPSSession = FALSE;
            pPort->StaCfg.WscControl.bSkipWPSTurnOffLED = FALSE;
        }

    // <== WPS  
    
    if (!MP_VERIFY_STATE(pPort, OP_STATE))
    {
        ndisStatus = NDIS_STATUS_INVALID_STATE;
        DBGPRINT(RT_DEBUG_ERROR, ("%s: failed on Invalid state.\n", __FUNCTION__));

        return ndisStatus;
    }
    else
    {
        MLME_DISASSOC_REQ_STRUCT    DisReq;
        PMLME_QUEUE_ELEM            pMsgElem = NULL;

        MP_SET_STATE(pPort, INIT_STATE);
        pAd->MlmeAux.AssocState = dot11_assoc_state_unauth_unassoc;
        
        if (INFRA_ON(pPort)) 
        {
            // Set to immediately send the media disconnect event
            pAd->MlmeAux.CurrReqIsFromNdis = TRUE;

            if (pPort->Mlme.CntlMachine.CurrState != CNTL_IDLE)
            {
                MlmeRestartStateMachine(pAd,pPort);
            }

            if ((pPort->PortNumber == pPort->P2PCfg.PortNumber) && IS_P2P_CLIENT_OP(pPort))
                pPort->P2PCfg.P2PConnectState = P2P_DO_WPS_ENROLLEE;

            COPY_MAC_ADDR(&DisReq.Addr, pPort->PortCfg.Bssid);
            DisReq.Reason =  REASON_DISASSOC_STA_LEAVING;

           PlatformAllocateMemory(pAd,  &pMsgElem, sizeof(MLME_QUEUE_ELEM));
            if (pMsgElem != NULL)
            {
                pMsgElem->Machine = ASSOC_STATE_MACHINE;
                pMsgElem->MsgType = MT2_MLME_DISASSOC_REQ;
                pMsgElem->MsgLen = sizeof(MLME_DISASSOC_REQ_STRUCT);
                pMsgElem->PortNum = pPort->PortNumber;
                PlatformMoveMemory(pMsgElem->Msg, &DisReq, sizeof(MLME_DISASSOC_REQ_STRUCT));

                MlmeDisassocReqAction(pAd, pMsgElem);

                PlatformFreeMemory(pMsgElem, sizeof(MLME_QUEUE_ELEM));
                if (bHwRadioOff == TRUE)
                {
                    // Make sure that the ASIC sucessfully send the disauthentication/disassociation frame before radio off
                    Delay_us(400000); // 400 ms
                }
            }
            else
            {
                DBGPRINT(RT_DEBUG_ERROR, ("%s:Allocate memory failed for MT2_MLME_DISASSOC_REQ\n", __FUNCTION__));
            }
            
            MlmeCntLinkDown(pPort, FALSE);

            //
            // Indicate disconnect status
            //
            if (bHwRadioOff == TRUE)
            {
                PlatformIndicateDisassociation(pAd,pPort, DisReq.Addr, DOT11_DISASSOC_REASON_RADIO_OFF);
            }
            else
            {
                PlatformIndicateDisassociation(pAd,pPort, DisReq.Addr, DOT11_DISASSOC_REASON_OS);
            }
        }
        else if (ADHOC_ON(pPort))
        {
            PRTMP_ADHOC_LIST pAdhocEntry;
            MLME_DEAUTH_REQ_STRUCT      DeAuthRequest;
            OID_SET_HT_PHYMODE      HTPhymode;
            
            DeAuthRequest.Reason = REASON_DEAUTH_STA_LEAVING;
            NdisAcquireSpinLock(&pAd->StaCfg.AdhocListLock);
            pAdhocEntry = (PRTMP_ADHOC_LIST)pAd->StaCfg.ActiveAdhocListQueue.Head;
            while (pAdhocEntry)
            {
                COPY_MAC_ADDR(DeAuthRequest.Addr, pAdhocEntry->PeerMacAddress);
                MlmeAuthDeauthReqAction(pAd, pPort->PortNumber, &DeAuthRequest);
                pAdhocEntry = (PRTMP_ADHOC_LIST)pAdhocEntry->Next;
            }
            NdisReleaseSpinLock(&pAd->StaCfg.AdhocListLock);

            MlmeCntLinkDown(pPort, FALSE);

            //
            // Indicate disconnect status for all peer.
            // 
            if (bHwRadioOff == TRUE)
            {           
                PlatformIndicateDisassociation(pAd, pPort,BROADCAST_ADDR, DOT11_DISASSOC_REASON_RADIO_OFF);
            }
            else
            {
                PlatformIndicateDisassociation(pAd, pPort,BROADCAST_ADDR, DOT11_DISASSOC_REASON_OS);
            }

            //
            // set back to INFRA as default, otherwise, 20MHz will be set for next ConnectRequest   
            //
            pAd->StaCfg.BssType = BSS_INFRA;
        
            // fill up the HTphymode
            HTPhymode.PhyMode = pPort->CommonCfg.PhyMode;
            HTPhymode.TransmitNo = (UCHAR)pAd->HwCfg.Antenna.field.TxPath;
            HTPhymode.HtMode = (UCHAR)pPort->CommonCfg.RegTransmitSetting.field.HTMODE;
            HTPhymode.ExtOffset = (UCHAR)pPort->CommonCfg.RegTransmitSetting.field.EXTCHA;
            HTPhymode.MCS = (UCHAR)pPort->CommonCfg.RegTransmitSetting.field.MCS;
            HTPhymode.BW = (UCHAR)pPort->CommonCfg.RegTransmitSetting.field.BW;
            HTPhymode.STBC = (UCHAR)pPort->CommonCfg.RegTransmitSetting.field.STBC;
            HTPhymode.SHORTGI = (UCHAR)pPort->CommonCfg.RegTransmitSetting.field.ShortGI;
            // Don't need to store HTPhymode.VhtCentralChannel here, SetHtVht will decide it.
        
            pPort->CentralChannel = pPort->Channel;
        
            // set back HT again
            if (pPort->CommonCfg.PhyMode >= PHY_11ABGN_MIXED)
                SetHtVht(pAd, pPort,&HTPhymode);
        }

        *BytesRead = 0;
        *BytesNeeded = 0;
        DBGPRINT(RT_DEBUG_TRACE, ("Set::OID_DOT11_DISCONNECT_REQUEST\n"));              

        return ndisStatus;
    }
}

            

