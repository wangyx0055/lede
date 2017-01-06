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
    connect.c

    Abstract:

    Revision History:
    Who         When            What
    --------    ----------      ----------------------------------------------
    John        2004-08-08      Major modification from RT2560
*/
#include "MtConfig.h"

extern UCHAR    EpToQueue[];

UCHAR   CipherSuiteWpaNoneTkip[] = {
        0x00, 0x50, 0xf2, 0x01, // oui
        0x01, 0x00,             // Version
        0x00, 0x50, 0xf2, 0x02, // Multicast
        0x01, 0x00,             // Number of unicast
        0x00, 0x50, 0xf2, 0x02, // unicast
        0x01, 0x00,             // number of authentication method
        0x00, 0x50, 0xf2, 0x00  // authentication
        };
UCHAR   CipherSuiteWpaNoneTkipLen = (sizeof(CipherSuiteWpaNoneTkip) / sizeof(UCHAR));

UCHAR   CipherSuiteWpaNoneAes[] = {
        0x00, 0x50, 0xf2, 0x01, // oui
        0x01, 0x00,             // Version
        0x00, 0x50, 0xf2, 0x04, // Multicast
        0x01, 0x00,             // Number of unicast
        0x00, 0x50, 0xf2, 0x04, // unicast
        0x01, 0x00,             // number of authentication method
        0x00, 0x50, 0xf2, 0x00  // authentication
        };
UCHAR   CipherSuiteWpaNoneAesLen = (sizeof(CipherSuiteWpaNoneAes) / sizeof(UCHAR));

UCHAR   CipherSuiteAdHocWpa2Aes[] = {
        0x01, 0x00,             // Version
        0x00, 0x0F, 0xAC, 0x04, // Multicast
        0x01, 0x00,             // Number of unicast
        0x00, 0x0F, 0xAC, 0x04, // unicast
        0x01, 0x00,             // number of authentication method
        0x00, 0x0F, 0xAC, 0x02, // authentication
        0x00, 0x00              // capability
        };
UCHAR   CipherSuiteAdHocWpa2AesLen = (sizeof(CipherSuiteAdHocWpa2Aes) / sizeof(UCHAR));

// The following MACRO is called after 1. starting an new IBSS, 2. succesfully JOIN an IBSS,
// or 3. succesfully ASSOCIATE to a BSS, 4. successfully RE_ASSOCIATE to a BSS
// All settings successfuly negotiated furing MLME state machines become final settings
// and are copied to pAd->StaActive
#define COPY_SETTINGS_FROM_MLME_AUX_TO_ACTIVE_CFG(_pAd,_pPort)                                 \
{                                                                                       \
    _pPort->PortCfg.SsidLen = _pAd->MlmeAux.SsidLen;                                    \
    PlatformMoveMemory(_pPort->PortCfg.Ssid, _pAd->MlmeAux.Ssid, _pAd->MlmeAux.SsidLen);    \
    COPY_MAC_ADDR(_pPort->PortCfg.Bssid, _pAd->MlmeAux.Bssid);                          \
    _pAd->StaActive.Aid = _pAd->MlmeAux.Aid;                                            \
    _pAd->StaActive.AtimWin = _pAd->MlmeAux.AtimWin;                                    \
    _pAd->StaActive.CapabilityInfo = _pAd->MlmeAux.CapabilityInfo;                      \
    _pPort->CommonCfg.BeaconPeriod = _pAd->MlmeAux.BeaconPeriod;                          \
    _pAd->StaActive.CfpMaxDuration = _pAd->MlmeAux.CfpMaxDuration;                      \
    _pAd->StaActive.CfpPeriod = _pAd->MlmeAux.CfpPeriod;                                \
    _pAd->StaActive.SupRateLen = _pAd->MlmeAux.SupRateLen;                              \
    PlatformMoveMemory(_pAd->StaActive.SupRate, _pAd->MlmeAux.SupRate, _pAd->MlmeAux.SupRateLen);   \
    _pAd->StaActive.ExtRateLen = _pAd->MlmeAux.ExtRateLen;                                      \
    PlatformMoveMemory(_pAd->StaActive.ExtRate, _pAd->MlmeAux.ExtRate, _pAd->MlmeAux.ExtRateLen);   \
    PlatformMoveMemory(&_pPort->CommonCfg.APEdcaParm, &_pAd->MlmeAux.APEdcaParm, sizeof(EDCA_PARM));  \
    PlatformMoveMemory(&_pPort->CommonCfg.APQosCapability, &_pAd->MlmeAux.APQosCapability, sizeof(QOS_CAPABILITY_PARM));  \
    PlatformMoveMemory(&_pPort->CommonCfg.APQbssLoad, &_pAd->MlmeAux.APQbssLoad, sizeof(QBSS_LOAD_PARM)); \
}

/*
    ==========================================================================
    Description:

    IRQL = PASSIVE_LEVEL

    ==========================================================================
*/
VOID MlmeCntlInit(
    IN PMP_ADAPTER pAd, 
    IN STATE_MACHINE *S, 
    OUT STATE_MACHINE_FUNC Trans[]) 
{
    // Control state machine differs from other state machines, the interface 
    // follows the standard interface
    S->CurrState = CNTL_IDLE;
}

VOID MlmeCntPortCntlInit(
    IN PMP_PORT pPort, 
    IN STATE_MACHINE *S, 
    OUT STATE_MACHINE_FUNC Trans[]) 
{
    // Control state machine differs from other state machines, the interface 
    // follows the standard interface
    pPort->Mlme.CntlMachine.CurrState = CNTL_IDLE;
}

/*
    ==========================================================================
    Description:

    IRQL = DISPATCH_LEVEL
    
    ==========================================================================
*/
VOID MlmeCntlMachinePerformAction(
    IN PMP_ADAPTER pAd, 
    IN STATE_MACHINE *S, 
    IN MLME_QUEUE_ELEM *Elem) 
{
    NDIS_STATUS  ndisStatus = NDIS_STATUS_SUCCESS;
    PMP_PORT pPort = pAd->PortList[Elem->PortNum];
    USHORT      Wcid = MlmeSyncGetWlanIdxByPort(pPort, ROLE_WLANIDX_BSSID);
    BOOLEAN         bCanRadioOff = TRUE;

    if (pPort == NULL)
    {
        DBGPRINT(RT_DEBUG_ERROR, ("%s - pPort is  NULL, Error, and return this function", __FUNCTION__));
        return;
    }

#if DBG
    if (  Elem->MsgType == OID_DOT11_SCAN_REQUEST)
        DBGPRINT(RT_DEBUG_TRACE, ("MLME: %s, scan req issue, cntl State =%x\n",__FUNCTION__, pPort->Mlme.CntlMachine.CurrState));
#endif
    //
    // Indicate scan confirm if scan request can't be handled.
    //
    if ((Elem->MsgType == OID_DOT11_SCAN_REQUEST) && (pPort->Mlme.CntlMachine.CurrState != CNTL_IDLE))
    {
        PlatformIndicateScanStatus(pAd, pPort, NDIS_STATUS_SUCCESS, TRUE, FALSE);
        return;
    }
    
    switch(pPort->Mlme.CntlMachine.CurrState) 
    {       
        case CNTL_IDLE:
            // Normal Ssid connection operation
            MlmeCntlIdleProc(pAd, Elem);
            break;
        case CNTL_WAIT_DISASSOC:
            MlmeCntlWaitDisassocProc(pAd, Elem);
            break;
        case CNTL_WAIT_JOIN:
            MlmeCntlWaitJoinProc(pAd, Elem);
            break;

        // CNTL_WAIT_REASSOC is the only state in CNTL machine that does
        // not triggered directly or indirectly by "RTMPSetInformation(OID_xxx)". 
        // Therefore not protected by NDIS's "only one outstanding OID request" 
        // rule. Which means NDIS may SET OID in the middle of ROAMing attempts.
        // Current approach is to block new SET request at RTMPSetInformation()
        // when CntlMachine.CurrState is not CNTL_IDLE
        case CNTL_WAIT_REASSOC:
            MlmeCntlWaitReassocProc(pAd, Elem);
            break;

        case CNTL_WAIT_START:
            MlmeCntlWaitStartProc(pAd, Elem);
            break;
        case CNTL_WAIT_AUTH:
            MlmeCntlWaitAuthProc(pAd, Elem);
            break;
        case CNTL_WAIT_AUTH2:
            MlmeCntlWaitAuthProc2(pAd, Elem);
            break;
        case CNTL_WAIT_ASSOC:
            MlmeCntlWaitAssocProc(pAd, Elem);
            break;

        case CNTL_WAIT_OID_LIST_SCAN:
            if(Elem->MsgType == MT2_SCAN_CONF) 
            {
                pAd->MlmeAux.bActiveScanForHiddenAP = FALSE; // Reset to default
                pAd->MlmeAux.bRetainBSSEntriesInScanTable = FALSE;

                PlatformIndicateScanStatus(pAd, pPort, ndisStatus, TRUE, FALSE);

                // Resume TxRing after SCANING complete. We hope the out-of-service time
                // won't be too long to let upper layer time-out the waiting frames
                XmitResumeMsduTransmission(pAd);

#if 0
                if(pAd->MlmeAux.bQueueScan)
                {
                    pAd->MlmeAux.bQueueScan = FALSE;
                    PlatformIndicateDot11Status(pAd,
                                        pAd->PortList[pAd->ucQueueScanPortNum],
                                        NDIS_STATUS_DOT11_SCAN_CONFIRM,
                                        pAd->pNicCfg->ScanRequestID,
                                        &ndisStatus,
                                        sizeof(NDIS_STATUS));
                    DBGPRINT(RT_DEBUG_TRACE, ("MLME: MlmeCntlMachinePerformAction, Indicate Queued scan confirm \n"));
                }
#endif

#if(COMPILE_WIN8_ABOVE(CURRENT_OS_NDIS_VER))
                if (NDIS_WIN8_ABOVE(pAd) && (pAd->StaCfg.NLOEntry.NLOEnable) && (pAd->StaCfg.NLOEntry.IndicateNLO))
                {
                    pAd->StaCfg.NLOEntry.IndicateNLO = FALSE;
                    N6PlatformIndicateNLOStatus(pAd, pPort);
                }
#endif
                pPort->Mlme.CntlMachine.CurrState = CNTL_IDLE;

                //
                // Set LED status to previous status.
                //
                if (pAd->HwCfg.bLedOnScanning)
                {
                    pAd->HwCfg.bLedOnScanning = FALSE;
                    if(pAd->HwCfg.LedStatus == LED_WPS_SUCCESS)
                    {
                        pPort->StaCfg.WscControl.bWPSLEDSetTimer = FALSE;
                    }
                    if((!OPSTATUS_TEST_FLAG(pPort, fOP_STATUS_MEDIA_STATE_CONNECTED)) && (pAd->HwCfg.LedStatus == LED_NORMAL_CONNECTION_WITH_SECURITY))
                    {
                        //Do nothing
                    }
                    else
                    {
                        LedCtrlSetLed(pAd, pAd->HwCfg.LedStatus);
                    }
                }
                
                // IF this AP support 20/40 BSS Coex, send back the response periodically.
                if ((pPort->CommonCfg.BSSCoexist2040.field.InfoReq == 1) && (INFRA_ON(pPort)) && (OPSTATUS_TEST_FLAG(pAd, fOP_STATUS_SCAN_2040)) && (OPSTATUS_TEST_FLAG(pPort, fOP_STATUS_SCAN_2040)))
                {
                    if (IS_P2P_CON_CLI(pAd, pPort) || IS_P2P_MS_CLI(pAd, pPort) || IS_P2P_MS_CLI2(pAd, pPort))
                    {
                        Wcid = P2pGetClientWcid(pAd, pPort);
                    }
                    
                    Update2040CoexistFrameAndNotify(pAd, pPort, (UCHAR)Wcid, TRUE);
                }
                // Indicate to request pairwise master key identifiers (PMKIDs) for basic service set (BSS) identifiers (BSSIDs) 
                // that the 802.11 station can potentially roam to.
                MlmeCntCheckPMKIDCandidate(pPort); 
                
                // scan completed, init to not FastRoamingScan/FastScan
                pAd->StaCfg.bFastRoamingScan = FALSE;
                //pAd->StaCfg.bFastUpdateScanTab = FALSE;

                DBGPRINT(RT_DEBUG_TRACE, ("KKKK : %d %d %d %d %d %d %d %d %d %d %d\n",
                (bCanRadioOff == TRUE),
                (pAd->StaCfg.WindowsPowerMode != DOT11_POWER_SAVING_NO_POWER_SAVING),
                (pAd->StaCfg.PSControl.field.DisablePS == FALSE),
                (pPort->PortSubtype == PORTSUBTYPE_STA) && (IDLE_ON(pPort)),
                (P2P_OFF(pPort)),
                (pPort->Mlme.SyncMachine.CurrState == SYNC_IDLE),
                (pPort->Mlme.CntlMachine.CurrState == CNTL_IDLE),
                (pPort->StaCfg.WscControl.WscState == WSC_STATE_OFF),
                (!MT_TEST_FLAG(pAd, fRTMP_ADAPTER_IDLE_RADIO_OFF)),
                (pPort->PortCfg.OperationMode != DOT11_OPERATION_MODE_NETWORK_MONITOR),
                (pPort->SoftAP.bAPStart == FALSE)
                ));

                //AsicRadioOff(pAd); // for stress test

                // If station is idle, go to sleep
                // don't perform idle-power-save mechanism when P2P turns on
                if ((bCanRadioOff == TRUE)
                    /*&& (pPort->PortNumber == NDIS_DEFAULT_PORT_NUMBER)*/
                    && (pAd->StaCfg.WindowsPowerMode != DOT11_POWER_SAVING_NO_POWER_SAVING)
                    && (pAd->StaCfg.PSControl.field.DisablePS == FALSE)     // not disable PS
                    && (pPort->PortSubtype == PORTSUBTYPE_STA) && (IDLE_ON(pPort))
                    && (P2P_OFF(pPort))
                    && (pPort->Mlme.SyncMachine.CurrState == SYNC_IDLE)
                    && (pPort->Mlme.CntlMachine.CurrState == CNTL_IDLE)
                    && (pPort->StaCfg.WscControl.WscState == WSC_STATE_OFF)
                    && (!MT_TEST_FLAG(pAd, fRTMP_ADAPTER_IDLE_RADIO_OFF))
                    && (pPort->PortCfg.OperationMode != DOT11_OPERATION_MODE_NETWORK_MONITOR)
                    && (pPort->SoftAP.bAPStart == FALSE))
                {
                    if ((((pAd->HwCfg.LedCntl.field.LedMode & LED_MODE) == WPS_LED_MODE_14))&&(pAd->HwCfg.FirmwareVersion == 222))
                    {
                        DBGPRINT(RT_DEBUG_TRACE, ("[%s][LINE_%d]Turn off Led Firmware version =%d)\n",__FUNCTION__,__LINE__, pAd->HwCfg.LedCntl.field.LedMode));
                        LedCtrlSetLed(pAd, LED_RADIO_OFF);
                    }
                    if (pAd->StaCfg.PSControl.field.EnablePSinIdle == TRUE)     // stop Rx BulkIn for USB SS
                    {
                        pAd->CountDowntoRadioOff = 0;
                        DBGPRINT(RT_DEBUG_TRACE, ("AsicRadioOff....1\n"));
                        AsicRadioOff(pAd);
                    }
                    else
                    {
                        AsicSendCommanToMcu(pAd, SLEEP_MCU_CMD, 0xff, 0xff, 0x02);   // send POWER-SAVE command to MCU. Timeout unit:40us.
                        OPSTATUS_SET_FLAG(pAd, fOP_STATUS_DOZE);
                    }
                    DBGPRINT(RT_DEBUG_TRACE, ("[%s][LINE_%d]PSM - Issue Sleep command, USB SS=%d)\n",__FUNCTION__,__LINE__, pAd->StaCfg.PSControl.field.EnablePSinIdle));
                }
            }
            break;

        case CNTL_WAIT_OID_DISASSOC:
            if (Elem->MsgType == MT2_DISASSOC_CONF) 
            {
                if ( !MT_TEST_FLAG(pAd, fRTMP_ADAPTER_SLEEP_IN_PROGRESS))
                    MlmeCntLinkDown(pPort, FALSE);
                pPort->Mlme.CntlMachine.CurrState = CNTL_IDLE;
            }
            break;
        //
        // This state is for that we want to connect to an AP but
        // it didn't find on BSS List table. So we need to scan the air first,
        // after that we can try to connect to the desired AP if available.
        //
        case CNTL_WAIT_SCAN_FOR_CONNECT:
            if(Elem->MsgType == MT2_SCAN_CONF) 
            {
                pAd->MlmeAux.bActiveScanForHiddenAP = FALSE; // Reset to default

                PlatformIndicateScanStatus(pAd, pPort, ndisStatus, TRUE, FALSE);

#if 0
                if(pAd->MlmeAux.bQueueScan)
                {
                    pAd->MlmeAux.bQueueScan = FALSE;
                    PlatformIndicateDot11Status(pAd,
                                        pAd->PortList[pAd->ucQueueScanPortNum],
                                        NDIS_STATUS_DOT11_SCAN_CONFIRM,
                                        pAd->pNicCfg->ScanRequestID,
                                        &ndisStatus,
                                        sizeof(NDIS_STATUS));
                    DBGPRINT(RT_DEBUG_TRACE, ("MLME: MlmeCntlMachinePerformAction, Indicate Queued scan confirm \n"));
                }
#endif              
                // Resume TxRing after SCANING complete. We hope the out-of-service time
                // won't be too long to let upper layer time-out the waiting frames
                XmitResumeMsduTransmission(pAd);
                pPort->Mlme.CntlMachine.CurrState = CNTL_IDLE;

                pAd->StaCfg.KeepReconnectCnt = 3;

                //
                // Check if we can connect to.
                //
                // Use pAd->MlmeAux.Ssid instead of pAd->MlmeAux.AutoReconnectSsid since pAd->MlmeAux.AutoReconnectSsid is used by STA in MLME periodic
                //BssTableSsidSort(pAd,pPort, &pAd->MlmeAux.SsidBssTab, pAd->MlmeAux.AutoReconnectSsid, pAd->MlmeAux.AutoReconnectSsidLen);
                BssTableSsidSort(pAd,pPort, &pAd->MlmeAux.SsidBssTab, pAd->MlmeAux.Ssid, pAd->MlmeAux.SsidLen);
                if (( pAd->MlmeAux.SsidBssTab.BssNr == 0) && ( pAd->StaCfg.BssType == BSS_INFRA))
                {   
                    if ( pAd->MlmeAux.bNeedIndicateConnectStatus  && (pAd->MlmeAux.ScanForConnectCnt >= pAd->StaCfg.KeepReconnectCnt))
                    {
                        //Auto connection is invalid after indicating connection fail, neither
                        //"connection_start" nor "roaming_start" will make the auto connection valid.
                        //So disable it.
                        pAd->MlmeAux.AutoReconnectSsidLen= 32;
                        PlatformZeroMemory(pAd->MlmeAux.AutoReconnectSsid, pAd->MlmeAux.AutoReconnectSsidLen);

                        pAd->MlmeAux.bNeedIndicateConnectStatus = FALSE;
                        DBGPRINT_ERR(("!ERROR! CNTL - No bss found indicate fail "));
                        PlatformIndicateConnectionStart(pAd,pPort, pAd->StaCfg.BssType, NULL, NULL, 0);
                        PlatformIndicateConnectionCompletion(pAd,pPort, DOT11_CONNECTION_STATUS_CANDIDATE_LIST_EXHAUSTED);
                    }
                    else
                    {
                        //prevent soft AP disturbed by auto connect process
                        if (pPort->SoftAP.bAPStart == FALSE)
                        {
                            MlmeAutoReconnectLastSSID(pAd, pPort);
                        }
                    }
                    LedCtrlConnectionCompletion(pAd,pPort, FALSE);
                    DBGPRINT(RT_DEBUG_TRACE, ("[%s][LINE_%d]autoconnection debug\n",__FUNCTION__,__LINE__));
                }
                else
                {
                    DBGPRINT(RT_DEBUG_TRACE, ("[%s][LINE_%d]autoconnection debug, PortNum(%d)\n",__FUNCTION__,__LINE__, pPort->PortNumber));
                    MlmeAutoReconnectLastSSID(pAd, pPort);
                }

                // scan completed, init to not FastRoamingScan/FastScan
                pAd->StaCfg.bFastRoamingScan = FALSE;
                //pAd->StaCfg.bFastUpdateScanTab = FALSE;
            }
            break;

        default:
            DBGPRINT_ERR(("!ERROR! CNTL - Illegal message type(=%d)", Elem->MsgType));
            break;
    }
}


/*
    ==========================================================================
    Description:

    IRQL = DISPATCH_LEVEL
    
    ==========================================================================
*/
VOID MlmeCntlIdleProc(
    IN PMP_ADAPTER pAd, 
    IN MLME_QUEUE_ELEM *Elem) 
{
    MLME_DISASSOC_REQ_STRUCT   DisassocReq;
    PMP_PORT pPort = pAd->PortList[Elem->PortNum];

    if (MT_TEST_FLAG(pAd, fRTMP_ADAPTER_RADIO_OFF))
        return;
    
    //
    // reset the counter to 5 seconds to avoid AsicRadioOff() in 5 seconds.
    //
    pAd->CountDowntoRadioOff = STAY_IN_AWAKE;

    switch(Elem->MsgType) 
    {
        //case OID_802_11_SSID:
        case OID_DOT11_CONNECT_REQUEST:
#if(COMPILE_WIN8_ABOVE(CURRENT_OS_NDIS_VER))
        case OID_DOT11_WFD_CONNECT_TO_GROUP_REQUEST:
#endif          
            // do actions only if Radio is ON
            if (pAd->StaCfg.bRadio)
                MlmeCntlOidSsidProc(pAd, Elem);
            break;

        case MTK_OID_N5_SET_BSSID:
            // do actions only if Radio is ON
            if (pAd->StaCfg.bRadio)
                MlmeCntlOidRTBssidProc(pAd,Elem);
            break;

        case OID_DOT11_SCAN_REQUEST:
        case OID_802_11_BSSID_LIST_SCAN:
            // do actions only if Radio is ON
            if (pAd->StaCfg.bRadio)
                MlmeCntlOidScanProc(pAd,Elem);
            break;

        case OID_802_11_DISASSOCIATE:
            MlmeCntDisassocParmFill(pAd, &DisassocReq, pPort->PortCfg.Bssid, REASON_DEAUTH_STA_LEAVING);
            MlmeEnqueue(pAd,pPort, ASSOC_STATE_MACHINE, MT2_MLME_DISASSOC_REQ, sizeof(MLME_DISASSOC_REQ_STRUCT), &DisassocReq);
            pPort->Mlme.CntlMachine.CurrState = CNTL_WAIT_OID_DISASSOC;
            break;

        case MT2_MLME_ROAMING_REQ:
            MlmeCntlMlmeRoamingProc(pAd, Elem);
            break;

        case MTK_OID_N6_SET_DLS_PARAM:
            if (DLS_ON(pAd))
                MlmeCntlOidDLSSetupProc(pAd, Elem);
            else if (TDLS_ON(pAd))
                MlmeCntlOidTDLSRequestProc(pAd, Elem);
            break;

        default:
            DBGPRINT(RT_DEBUG_TRACE, ("CNTL - Illegal message in MlmeCntlIdleProc(MsgType=%d)\n",Elem->MsgType));
            break;
    }
}

// IRQL = DISPATCH_LEVEL
VOID MlmeCntlOidScanProc(
    IN PMP_ADAPTER pAd,
    IN MLME_QUEUE_ELEM *Elem)
{
    MLME_SCAN_REQ_STRUCT    ScanReq;    
    PDOT11_SCAN_REQUEST_V2  pDot11ScanRequest = (PDOT11_SCAN_REQUEST_V2)Elem->Msg;
    PDOT11_SSID             pDot11SSID = NULL;
    ULONG                   i = 0, BytesParsed = 0;
    BOOLEAN         brc;
    NDIS_STATUS ndiStatus=NDIS_STATUS_SUCCESS;	
    PMP_PORT              pPort = pAd->PortList[Elem->PortNum];
    DBGPRINT(RT_DEBUG_TRACE, ("===>MLME: MlmeCntlOidScanProc\n"));
    PlatformZeroMemory(&ScanReq, sizeof(MLME_SCAN_REQ_STRUCT));
    
    // Make sure Radio ON
    if(MT_TEST_FLAG(pAd, fRTMP_ADAPTER_IDLE_RADIO_OFF))
        AsicRadioOn(pAd);
    if (OPSTATUS_TEST_FLAG(pAd, fOP_STATUS_DOZE))
        AsicForceWakeup(pAd);

    //
    // We don't need to clean up previous SCAN result, OID_DOT11_FLUSH_BSS_LIST will do this job.
    //

    //
    // Init ScanReq
    //
    ScanReq.BssType = pDot11ScanRequest->dot11BSSType;
    if ((pDot11ScanRequest->dot11ScanType == dot11_scan_type_active) ||
        (pDot11ScanRequest->dot11ScanType == (dot11_scan_type_active | dot11_scan_type_forced)) ||
        (pDot11ScanRequest->dot11ScanType == dot11_scan_type_auto) ||
        (pDot11ScanRequest->dot11ScanType == (dot11_scan_type_auto | dot11_scan_type_forced)))  
    {
        ScanReq.ScanType = SCAN_ACTIVE;
        
        DBGPRINT(RT_DEBUG_TRACE, ("MlmeCntlOidScanProc uNumOfdot11SSIDs =%d\n",pDot11ScanRequest->uNumOfdot11SSIDs));
        //
        // Copy SSID
        //
        for (i = 0; i < pDot11ScanRequest->uNumOfdot11SSIDs; i++)
        {
            pDot11SSID = (PDOT11_SSID) (pDot11ScanRequest->ucBuffer + pDot11ScanRequest->udot11SSIDsOffset + BytesParsed);
            DBGPRINT(RT_DEBUG_TRACE, ("index =%x, scan SSID len=%d\n",i,pDot11SSID->uSSIDLength));
            if (pDot11SSID->uSSIDLength > 0)
            {
                PlatformMoveMemory(ScanReq.Dot11Ssid[i].ucSSID, pDot11SSID->ucSSID, pDot11SSID->uSSIDLength);
                ScanReq.Dot11Ssid[i].uSSIDLength = pDot11SSID->uSSIDLength;             
                DBGPRINT(RT_DEBUG_TRACE, ("Copy SSID for active scan SSID =%s\n",pDot11SSID->ucSSID));
            }

            BytesParsed += sizeof(DOT11_SSID);
        }
        ScanReq.NumOfdot11SSIDs = pDot11ScanRequest->uNumOfdot11SSIDs;      
    }
    else if (pDot11ScanRequest->dot11ScanType == dot11_scan_type_passive)
    {
        ScanReq.ScanType = SCAN_PASSIVE;
        ScanReq.NumOfdot11SSIDs = 1;
    }
    else    
    {
        ScanReq.ScanType = SCAN_ACTIVE;
        ScanReq.NumOfdot11SSIDs = 1;
    }

    if ((pPort->P2PCfg.P2PDiscoProvState == P2P_LISTEN_COMMAND) || (pPort->P2PCfg.P2PDiscoProvState == P2P_SEARCH_COMMAND))
    {
        DBGPRINT(RT_DEBUG_TRACE, ("%s::P2P::Scan Type = %s, P2PDiscoProvState = %s\n", __FUNCTION__, decodeP2PState(pPort->P2PCfg.ScanType), decodeP2PState(pPort->P2PCfg.P2PDiscoProvState)));
        // 3 situations to start P2P Scan. 1. Scan phase, 2. Search state in Find phase, 3. listen state in Find phase 
        if(pPort->P2PCfg.ScanType == P2P_SCAN_TYPE_PASSIVE)
            MlmeCntScanParmFill(pAd, &ScanReq, "", 0, BSS_ANY, SCAN_P2P_PASSIVE);
        else
            MlmeCntScanParmFill(pAd, &ScanReq, "", 0, BSS_ANY, SCAN_P2P);
        if ( pPort->P2PCfg.P2PDiscoProvState == P2P_SEARCH_COMMAND)
            pPort->P2PCfg.P2PDiscoProvState = P2P_SEARCH;
        else if ( pPort->P2PCfg.P2PDiscoProvState == P2P_LISTEN_COMMAND)
            pPort->P2PCfg.P2PDiscoProvState = P2P_LISTEN;
    }
    //if this is autoscan
    else if (ScanReq.NumOfdot11SSIDs == 0)
    {
        if (pAd->StaCfg.bFastRoamRescan == TRUE)
        {
            MlmeCntScanParmFill(pAd, &ScanReq, pAd->MlmeAux.Ssid, pAd->MlmeAux.SsidLen, BSS_ANY, SCAN_ACTIVE);
        }
        else                
            MlmeCntScanParmFill(pAd, &ScanReq, "", 0, BSS_ANY, SCAN_ACTIVE);
        DBGPRINT(RT_DEBUG_TRACE, ("%s: bFastRoamRescan =%d\n", __FUNCTION__, pAd->StaCfg.bFastRoamRescan));         
    }

    if (pDot11ScanRequest->uIEsLength > 0)
    {
        if ((pPort->PortCfg.AdditionalRequestIEData != NULL) && (pPort->PortCfg.AdditionalRequestIESize > 0))
        {
            PlatformFreeMemory(pPort->PortCfg.AdditionalRequestIEData, pPort->PortCfg.AdditionalRequestIESize);
            pPort->PortCfg.AdditionalRequestIEData = NULL;
            pPort->PortCfg.AdditionalRequestIESize = 0;
        }
	ndiStatus = PlatformAllocateMemory(pAd, (PVOID)&pPort->PortCfg.AdditionalRequestIEData, pDot11ScanRequest->uIEsLength);
        if (ndiStatus == NDIS_STATUS_SUCCESS)
        {
            DBGPRINT(RT_DEBUG_TRACE, ("Store the Additional IE for Probe Request usage!!\n"));
            PlatformMoveMemory(pPort->PortCfg.AdditionalRequestIEData, &pDot11ScanRequest->ucBuffer[pDot11ScanRequest->uIEsOffset], pDot11ScanRequest->uIEsLength);
            pPort->PortCfg.AdditionalRequestIESize = pDot11ScanRequest->uIEsLength;
            DBGPRINT(RT_DEBUG_TRACE, ("The Additional IE length = %d!!\n", pDot11ScanRequest->uIEsLength));
        }
    }
    
    brc = FALSE;
    if (IS_P2P_GO_OP(pPort) && IS_P2P_SIGMA_OFF(pPort))
    {
        brc = P2pActionMlmeCntlOidScanProc(pAd, pPort, &ScanReq);
    }

    if (brc == FALSE)
    {
        MlmeEnqueue(pAd,pPort, SYNC_STATE_MACHINE, MT2_MLME_SCAN_REQ, 
            sizeof(MLME_SCAN_REQ_STRUCT), &ScanReq);    // To Run MlmeSyncScanReqAction.
        pPort->Mlme.CntlMachine.CurrState = CNTL_WAIT_OID_LIST_SCAN;
        
        pAd->HwCfg.bLedOnScanning = TRUE;
        LedCtrlSetLed(pAd, LED_ON_SITE_SURVEY);
    }
    else
    {
        // Prevent scan idle
        MT_CLEAR_FLAG(pAd, fRTMP_ADAPTER_BSS_SCAN_IN_PROGRESS);
    }
    
    DBGPRINT(RT_DEBUG_TRACE, ("<====MLME: MlmeCntlOidScanProc\n"));
}

/*
    ==========================================================================
    Description:
        Before calling this routine, user desired SSID should already been
        recorded in CommonCfg.Ssid[]
    IRQL = DISPATCH_LEVEL
    
    ==========================================================================
*/
VOID MlmeCntlOidSsidProc(
    IN PMP_ADAPTER pAd, 
    IN MLME_QUEUE_ELEM * Elem) 
{
    MLME_DISASSOC_REQ_STRUCT   DisassocReq;
    ULONGLONG                  Now64;
    MLME_SCAN_REQ_STRUCT       ScanReq; 
    PMP_PORT pPort = pAd->PortList[Elem->PortNum];
    UCHAR   PortSecured = pAd->StaCfg.PortSecured;
        BOOLEAN                TimerCancelled = FALSE;
    PMAC_TABLE_ENTRY pWcidMacTabEntry = MlmeSyncMacTabMatchedWlanIdx(pPort, P2pGetClientWcid(pAd, pPort));   


    if (pPort == NULL)
    {
        DBGPRINT(RT_DEBUG_ERROR, ("%s - pPort is  NULL, Error, and return this function", __FUNCTION__));
        return;
    }

    //wake up since we want to connect with ssid.
    // Make sure Radio ON
    if(MT_TEST_FLAG(pAd, fRTMP_ADAPTER_IDLE_RADIO_OFF))
        AsicRadioOn(pAd);
    if (OPSTATUS_TEST_FLAG(pAd, fOP_STATUS_DOZE))
        AsicForceWakeup(pAd);
    
    pAd->StaCfg.BeaconFrameSize = 0;
    pAd->StaCfg.AssocRequestLength = 0;
    pAd->StaCfg.AssocResponseLength = 0;
    pPort->P2PCfg.P2pCounter.Counter100ms = 0;

    // [Workaround] Fix the next conenction enters an invalid state due to long execution time of MlmeSyncScanReqAction.
        if ((pAd->LogoTestCfg.OnTestingWHQL == FALSE) && (pAd->StaCfg.bStopScanForNextConnect == TRUE) && (pPort->PortNumber == PORT_0))
    {
        DBGPRINT(RT_DEBUG_TRACE, ("[%s][LINE_%d] Stop scan before connection (current sync state=%d)\n", 
            __FUNCTION__, __LINE__, pPort->Mlme.SyncMachine.CurrState));
        
                PlatformCancelTimer(&pPort->Mlme.ScanTimer, &TimerCancelled);
                
                
        MlmeSyncScanDoneAction(pAd, pAd->PortList[PORT_0]);
                // don't start the scanCount before connection
                if (pAd->MlmeAux.ScanForConnectCnt > 0)
                        pAd->MlmeAux.ScanForConnectCnt--;
        pAd->StaCfg.bStopScanForNextConnect = FALSE;
    }

    
    // Step 1. record the desired SSID
    if (pAd->MlmeAux.AutoReconnectStatus == TRUE) // Automatically reconnection.
    {
        DBGPRINT(RT_DEBUG_TRACE, ("[%s][LINE_%d] Automatically reconnection (SSID = %s , length = %d)\n", __FUNCTION__, __LINE__, 
            pAd->MlmeAux.AutoReconnectSsid, pAd->MlmeAux.AutoReconnectSsidLen));

        PlatformZeroMemory(pAd->MlmeAux.Ssid, MAX_LEN_OF_SSID);
        PlatformMoveMemory(pAd->MlmeAux.Ssid, pAd->MlmeAux.AutoReconnectSsid, pAd->MlmeAux.AutoReconnectSsidLen);
        pAd->MlmeAux.SsidLen = pAd->MlmeAux.AutoReconnectSsidLen;

        //In CLI case,  pAd->MlmeAux.AutoReconnectSsid is empty so we use pPort->PortCfg.DesiredSSID.ucSSID to write pAd->MlmeAux.Ssid
        if (pPort->PortType == WFD_CLIENT_PORT)
        {
            PlatformZeroMemory(pAd->MlmeAux.Ssid, MAX_LEN_OF_SSID);
            PlatformMoveMemory(pAd->MlmeAux.Ssid, pPort->PortCfg.DesiredSSID.ucSSID, pPort->PortCfg.DesiredSSID.uSSIDLength);
            pAd->MlmeAux.SsidLen = (UCHAR) pPort->PortCfg.DesiredSSID.uSSIDLength;          
        }
    }
    else // Normal connection.
    {
        DBGPRINT(RT_DEBUG_TRACE, ("[%s][LINE_%d] Normal conenction (SSID = %s , length = %d)\n", __FUNCTION__, __LINE__, 
            pPort->PortCfg.DesiredSSID.ucSSID, pPort->PortCfg.DesiredSSID.uSSIDLength));

        PlatformZeroMemory(pAd->MlmeAux.Ssid, MAX_LEN_OF_SSID);
        PlatformMoveMemory(pAd->MlmeAux.Ssid, pPort->PortCfg.DesiredSSID.ucSSID, pPort->PortCfg.DesiredSSID.uSSIDLength);
        pAd->MlmeAux.SsidLen = (UCHAR) pPort->PortCfg.DesiredSSID.uSSIDLength;
    }
    
    PlatformZeroMemory(pAd->MlmeAux.Bssid, MAC_ADDR_LEN);
    pAd->MlmeAux.BssType = pAd->StaCfg.BssType;

    pAd->MlmeAux.AssocState = dot11_assoc_state_unauth_unassoc;
    
    DBGPRINT(RT_DEBUG_TRACE, ("[%s][LINE_%d] PortNum=%d, Len=%d, SSID = %s\n", __FUNCTION__, __LINE__, 
            Elem->PortNum, pAd->MlmeAux.SsidLen, pAd->MlmeAux.Ssid));
    
    //
    // Update Reconnect Ssid, that user desired to connect. This is ONLY used by STA port autoconnection in Mlme periodic phase.
    //
    if (pPort->PortType == EXTSTA_PORT)
    {
        PlatformZeroMemory(pAd->MlmeAux.AutoReconnectSsid, MAX_LEN_OF_SSID);
        PlatformMoveMemory(pAd->MlmeAux.AutoReconnectSsid, pAd->MlmeAux.Ssid, pAd->MlmeAux.SsidLen);
        pAd->MlmeAux.AutoReconnectSsidLen = pAd->MlmeAux.SsidLen;
    }

    // step 2. find all matching BSS in the lastest SCAN result (inBssTab) 
    //    & log them into MlmeAux.SsidBssTab for later-on iteration. Sort by RSSI order
    BssTableSsidSort(pAd, pPort,&pAd->MlmeAux.SsidBssTab, pAd->MlmeAux.Ssid, pAd->MlmeAux.SsidLen);

    // If P2P Managed Not on, we need to check not to connect to Managed AP too often that deauth us with reason code 2 or 4.
    if ((P2P_ON(pPort)) && (pPort->P2PCfg.P2pManagedParm.APP2pMinorReason == MINOR_REASON_BCZ_MANAGED_BIT_ZERO) 
        && SSID_EQUAL(pAd->MlmeAux.Ssid, pAd->MlmeAux.SsidLen, pPort->P2PCfg.P2pManagedParm.ManageAPSsid, pPort->P2PCfg.P2pManagedParm.ManageAPSsidLen)
        && (pPort->P2PCfg.P2pCounter.ManageAPEnReconCounter > 0)
        && (MT_TEST_BIT(pPort->P2PCfg.P2pManagedParm.APP2pManageability, P2PMANAGED_ENABLE_BIT)))
    {
        DBGPRINT(RT_DEBUG_TRACE, ("MlmeCntlOidSsidProc :1 Don't reconnect to the Managed AP too much. Wait count down = %d. SSID=%c%c%c....\n", 
                pPort->P2PCfg.P2pCounter.ManageAPEnReconCounter, pAd->MlmeAux.Ssid[0], pAd->MlmeAux.Ssid[1], pAd->MlmeAux.Ssid[2]));
        return;
    }
        
    // If P2P Managed on, we need to check not to connect to Managed AP too often that deauth us with reason code 2.
    if ((P2P_ON(pPort)) && (pPort->P2PCfg.P2pManagedParm.APP2pMinorReason == MINOR_REASON_OUTSIDE_IT_DEFINED) 
        && SSID_EQUAL(pAd->MlmeAux.Ssid, pAd->MlmeAux.SsidLen, pPort->P2PCfg.P2pManagedParm.ManageAPSsid, pPort->P2PCfg.P2pManagedParm.ManageAPSsidLen)
        && (pPort->P2PCfg.P2pCounter.ManageAPEnReconCounter > 0)
        && (MT_TEST_BIT(pPort->P2PCfg.P2pManagedParm.APP2pManageability, P2PMANAGED_ENABLE_BIT)))
    {
        DBGPRINT(RT_DEBUG_TRACE, ("MlmeCntlOidSsidProc :2 Don't reconnect to the Managed AP too much. Wait count down = %d. SSID=%c%c%c....\n", 
                pPort->P2PCfg.P2pCounter.ManageAPEnReconCounter, pAd->MlmeAux.Ssid[0], pAd->MlmeAux.Ssid[1], pAd->MlmeAux.Ssid[2]));
        return;
    }

    // Reset the desired BSSID list that set by the Ralink Vista UI with MTK_OID_N6_SET_DESIRED_BSSID_LIST.
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

        // Reset to FALSE.
        pAd->StaCfg.bSetDesiredBSSIDListFromRalinkUI = FALSE;
    }

    DBGPRINT(RT_DEBUG_TRACE, ("[%s][LINE_%d] CNTL - %d BSS of %d BSS match the desire SSID - Len=%d, %s\n", __FUNCTION__, __LINE__,
            pAd->MlmeAux.SsidBssTab.BssNr, pAd->ScanTab.BssNr, pAd->MlmeAux.SsidLen, pAd->MlmeAux.Ssid));

    NdisGetCurrentSystemTime((PLARGE_INTEGER)&Now64);

    // Use P2P's PortSecured in concurrent Client
    if (IS_P2P_CON_CLI(pAd, pPort) || IS_P2P_MS_CLI(pAd, pPort) || IS_P2P_MS_CLI2(pAd, pPort))
    {
        if(pWcidMacTabEntry != NULL)
        {
            PortSecured = (UCHAR)pWcidMacTabEntry->PortSecured;
        }
    }

    if (OPSTATUS_TEST_FLAG(pPort, fOP_STATUS_MEDIA_STATE_CONNECTED) && 
        (pPort->PortCfg.SsidLen == pAd->MlmeAux.SsidBssTab.BssEntry[0].SsidLen) &&
        PlatformEqualMemory(pPort->PortCfg.Ssid, pAd->MlmeAux.SsidBssTab.BssEntry[0].Ssid, pPort->PortCfg.SsidLen) &&
        MAC_ADDR_EQUAL(pPort->PortCfg.Bssid, pAd->MlmeAux.SsidBssTab.BssEntry[0].Bssid))
    {   
        // Case 1. already connected with an AP who has the desired SSID 
        //         with highest RSSI

        if (((pPort->PortCfg.AuthMode == Ralink802_11AuthModeWPA) ||
             (pPort->PortCfg.AuthMode == Ralink802_11AuthModeWPAPSK) ||
             (pPort->PortCfg.AuthMode == Ralink802_11AuthModeWPA2) ||
             (pPort->PortCfg.AuthMode == Ralink802_11AuthModeWPA2PSK)) &&
             (PortSecured == WPA_802_1X_PORT_NOT_SECURED))
        {
            // case 1.1 For WPA, WPA-PSK, if the 1x port is not secured, we have to redo 
            //          connection process
            DBGPRINT(RT_DEBUG_TRACE, ("[%s][LINE_%d] CNTL - disassociate with current AP...\n", __FUNCTION__, __LINE__));
            MlmeCntDisassocParmFill(pAd, &DisassocReq, pPort->PortCfg.Bssid, REASON_DEAUTH_STA_LEAVING);
            MlmeEnqueue(pAd,pPort, ASSOC_STATE_MACHINE, MT2_MLME_DISASSOC_REQ, 
                        sizeof(MLME_DISASSOC_REQ_STRUCT), &DisassocReq);
            pPort->Mlme.CntlMachine.CurrState = CNTL_WAIT_DISASSOC;
        }
        else if (pAd->pNicCfg->bConfigChanged == TRUE)
        {
            // case 1.2 Important Config has changed, we have to reconnect to the same AP
            DBGPRINT(RT_DEBUG_TRACE, ("[%s][LINE_%d] CNTL - disassociate with current AP Because config changed...\n", __FUNCTION__, __LINE__));
            MlmeCntDisassocParmFill(pAd, &DisassocReq, pPort->PortCfg.Bssid, REASON_DEAUTH_STA_LEAVING);
            MlmeEnqueue(pAd,pPort, ASSOC_STATE_MACHINE, MT2_MLME_DISASSOC_REQ, 
                        sizeof(MLME_DISASSOC_REQ_STRUCT), &DisassocReq);
            pPort->Mlme.CntlMachine.CurrState = CNTL_WAIT_DISASSOC;
        }
        else
        {
            // case 1.3. already connected to the SSID with highest RSSI.
            DBGPRINT(RT_DEBUG_TRACE, ("[%s][LINE_%d] CNTL - already with this BSSID. ignore this SET_SSID request\n", __FUNCTION__, __LINE__));
            //
            // (HCT 12.1) 1c_wlan_mediaevents required
            // media connect events are indicated when associating with the same AP          
            //
            if (INFRA_ON(pPort))
            {
                //
                // Since MediaState already is NdisMediaStateConnected
                // We just indicate the connect event again to meet the WHQL required.
                //
            }

            pPort->Mlme.CntlMachine.CurrState = CNTL_IDLE;
        } 
    } 
    else if (INFRA_ON(pPort)) 
    {
        //
        // For WHQL
        // [88888] OID_802_11_SSID should have returned NDTEST_WEP_AP2(Returned: )
        // Driver may lost SSID, and not connect to NDTEST_WEP_AP2 and will connect to NDTEST_WEP_AP2 by Autoreconnect
        // But media status is connected, so the SSID not report correctly.
        //
        if (!SSID_EQUAL(pPort->PortCfg.Ssid, pPort->PortCfg.SsidLen, pAd->MlmeAux.Ssid, pAd->MlmeAux.SsidLen))
        {
            //
            // Different SSID means not Roaming case, so we let MlmeCntLinkDown() to Indicate a disconnect event.
            // 
            pAd->MlmeAux.CurrReqIsFromNdis = TRUE;
        }
        // case 2. active INFRA association existent
        //    roaming is done within miniport driver, nothing to do with configuration
        //    utility. so upon a new SET(OID_802_11_SSID) is received, we just 
        //    disassociate with the current associated AP, 
        //    then perform a new association with this new SSID, no matter the 
        //    new/old SSID are the same or not.
        DBGPRINT(RT_DEBUG_TRACE, ("[%s][LINE_%d] CNTL - disassociate with current AP...\n", __FUNCTION__, __LINE__));
        //MlmeCntDisassocParmFill(pPort, &DisassocReq, pPort->PortCfg.Bssid, REASON_DEAUTH_STA_LEAVING);
        //MlmeEnqueue(pPort, ASSOC_STATE_MACHINE, MT2_MLME_DISASSOC_REQ, 
        //          sizeof(MLME_DISASSOC_REQ_STRUCT), &DisassocReq);

        //NDIS test
        //Disassociation immediately, instead of enqueuing.
        MlmeCntDisassocParmFill(pAd, &DisassocReq, pPort->PortCfg.Bssid, REASON_DEAUTH_STA_LEAVING);
        MlmeEnqueue(pAd,pPort, ASSOC_STATE_MACHINE, MT2_MLME_DISASSOC_REQ, 
                    sizeof(MLME_DISASSOC_REQ_STRUCT), &DisassocReq);
        pPort->Mlme.CntlMachine.CurrState = CNTL_WAIT_DISASSOC;
#if DEBUG_MLME
    DBGPRINT(RT_DEBUG_TRACE, ("[%s][LINE_%d] MLME: MlmeCntlOidSsidProc, INFRA_ON ==> MT2_MLME_DISASSOC_REQ\n", __FUNCTION__, __LINE__));
#endif          
    }
    else
    {
        if (ADHOC_ON(pPort))
        {
            DBGPRINT(RT_DEBUG_TRACE, ("[%s][LINE_%d] CNTL - drop current ADHOC\n", __FUNCTION__, __LINE__));
            MlmeCntLinkDown(pPort, FALSE);
            OPSTATUS_CLEAR_FLAG(pPort, fOP_STATUS_MEDIA_STATE_CONNECTED);
            DBGPRINT(RT_DEBUG_TRACE, ("[%s][LINE_%d] NDIS_STATUS_MEDIA_DISCONNECT Event C!\n", __FUNCTION__, __LINE__));
        }

        if (pAd->MlmeAux.BssType == BSS_ADHOC)
        {
            if (pAd->MlmeAux.SsidBssTab.BssNr > 0)
            {
                //join the ibss
                pAd->MlmeAux.BssIdx = 0;                
                MlmeCntIterateOnBssTab(pPort);         
            }
            else
            {
                MLME_SCAN_REQ_STRUCT ScanReq2;
                if (pAd->StaCfg.IBSSJoinOnly)
                {
                    DBGPRINT(RT_DEBUG_TRACE, ("[%s][LINE_%d] CNTL - No matching IBSS, start a new scan...\n", __FUNCTION__, __LINE__));
                    MlmeCntScanParmFill(pAd, &ScanReq2, pAd->MlmeAux.Ssid, pAd->MlmeAux.SsidLen, BSS_ANY, FAST_SCAN_ACTIVE);
                    MlmeEnqueue(pAd,pPort, SYNC_STATE_MACHINE, MT2_MLME_SCAN_REQ, sizeof(MLME_SCAN_REQ_STRUCT), &ScanReq2);
                    pPort->Mlme.CntlMachine.CurrState = CNTL_WAIT_SCAN_FOR_CONNECT;
                    pAd->StaCfg.LastScanTime = Now64;
                }
                else
                {
                    if (pAd->StaCfg.bIbssScanOnce)
                    {
                        //scan once before starting a new adhoc
                        pAd->StaCfg.bIbssScanOnce = FALSE;

                        DBGPRINT(RT_DEBUG_TRACE, ("[%s][LINE_%d] CNTL - No matching IBSS, start a new scan...\n", __FUNCTION__, __LINE__));
                        MlmeCntScanParmFill(pAd, &ScanReq2, pAd->MlmeAux.Ssid, pAd->MlmeAux.SsidLen, BSS_ADHOC, FAST_SCAN_ACTIVE);
                        MlmeEnqueue(pAd,pPort, SYNC_STATE_MACHINE, MT2_MLME_SCAN_REQ, sizeof(MLME_SCAN_REQ_STRUCT), &ScanReq2);
                        pPort->Mlme.CntlMachine.CurrState = CNTL_WAIT_SCAN_FOR_CONNECT;
                        pAd->StaCfg.LastScanTime = Now64;
                        // In case of the FAST_SCAN_ACTIVE, the miniport driver has to retain all BSS entries in the scan table.
                        // Otherwise, the site survey of the UI would have few APs since the NIC using FAST_SCAN_ACTIVE type spends less time during scan operation.
                        pAd->MlmeAux.bRetainBSSEntriesInScanTable = TRUE;

                        DBGPRINT(RT_DEBUG_TRACE, ("CNTL - No matching BSS, pPort->P2PCfg.P2PConnectState = %d\n", pPort->P2PCfg.P2PConnectState));
                        
                    }
                    else
                    {
                        //this will start a new ibss
                        pAd->MlmeAux.BssIdx = 0;
                        MlmeCntIterateOnBssTab(pPort);
                    }
                }
            }
        }
        else
        {
            DBGPRINT(RT_DEBUG_TRACE, ("CNTL - BssNr(%d), bAutoReconnect(%d), BssType(%d), bGroupsScanOnce(%d)\n", pAd->MlmeAux.SsidBssTab.BssNr, pAd->StaCfg.bAutoReconnect, pAd->MlmeAux.BssType, pPort->P2PCfg.bGroupsScanOnce));

            if ((pAd->MlmeAux.SsidBssTab.BssNr == 0) &&
                ((pAd->StaCfg.bAutoReconnect == TRUE) || (pPort->P2PCfg.bGroupsScanOnce == TRUE))&& 
                (pAd->MlmeAux.BssType == BSS_INFRA) &&
                (MlmeValidateSSID(pAd->MlmeAux.Ssid, pAd->MlmeAux.SsidLen) == TRUE))
            {
                //scan once before starting a new Group network
                pPort->P2PCfg.bGroupsScanOnce = FALSE;

                DBGPRINT(RT_DEBUG_TRACE, ("[%s][LINE_%d] CNTL - No matching BSS, start a new scan\n", __FUNCTION__, __LINE__));
                MlmeCntScanParmFill(pAd, &ScanReq, pAd->MlmeAux.Ssid, pAd->MlmeAux.SsidLen, BSS_ANY, FAST_SCAN_ACTIVE);
                MlmeEnqueue(pAd,pPort, SYNC_STATE_MACHINE, MT2_MLME_SCAN_REQ, sizeof(MLME_SCAN_REQ_STRUCT), &ScanReq);
                pPort->Mlme.CntlMachine.CurrState = CNTL_WAIT_SCAN_FOR_CONNECT;
                // Reset Missed scan number
                pAd->StaCfg.LastScanTime = Now64;
            
                // In case of the FAST_SCAN_ACTIVE, the miniport driver has to retain all BSS entries in the scan table.
                // Otherwise, the site survey of the UI would have few APs since the NIC using FAST_SCAN_ACTIVE type spends less time during scan operation.
                pAd->MlmeAux.bRetainBSSEntriesInScanTable = TRUE;
                if (pPort->P2PCfg.P2PConnectState == P2P_I_AM_CLIENT_ASSOC_AUTH)
                {
                    //pPort->P2PCfg.P2PConnectState = P2P_CONNECT_IDLE; 
                    pPort->P2PCfg.P2pCounter.CounterAutoReconnectForP2pClient = 0;
                }
            }
            else
            {   
                DBGPRINT(RT_DEBUG_TRACE, ("[%s][LINE_%d] NDIS_STATUS_DOT11_CONNECTION_START [Type=%d,Len=%d,%c%c%c..]\n", __FUNCTION__, __LINE__,
                    pAd->StaCfg.BssType, pPort->PortCfg.DesiredSSID.uSSIDLength, pPort->PortCfg.DesiredSSID.ucSSID[0], pPort->PortCfg.DesiredSSID.ucSSID[1], pPort->PortCfg.DesiredSSID.ucSSID[2]));                    

                //
                // Indicate the connection start status indication
                //
                if (pAd->MlmeAux.BssType == BSS_INFRA)
                {
                    if (pAd->MlmeAux.bStaCanRoam)
                    {
                        NdisAcquireSpinLock(&pAd->pNicCfg->PendingOIDLock);
                        if(pAd->bWakeupFromS3S4 && pAd->LogoTestCfg.OnTestingWHQL && (pAd->pNicCfg->PendOidEntry.PendingNdisRequest != NULL))
                        {
                            pAd->bWakeupFromS3S4 = FALSE;
                            DBGPRINT(RT_DEBUG_TRACE, ("ASSOC - Indicating OID_PNP_SET_POWER before associaction when testing WHCK\n"));
                            NdisReleaseSpinLock(&pAd->pNicCfg->PendingOIDLock);
                            
                            N6CompletePendingOID( pAd, PENDING_PNP_SET_POWER, NDIS_STATUS_SUCCESS);
                        }
                        else
                        {
                            NdisReleaseSpinLock(&pAd->pNicCfg->PendingOIDLock);
                        }

                        PlatformIndicateRoamingStart(pAd,pPort, DOT11_ROAMING_REASON_ASSOCIATION_LOST);
                    }
                    else
                    {
#if(COMPILE_WIN8_ABOVE(CURRENT_OS_NDIS_VER))   
                        // Delay ConnectionStart until ReadyToConnect for wps connection request.
                        if ((pPort->PortType == WFD_CLIENT_PORT) && (pPort->P2PCfg.WaitForWPSReady == TRUE) && (pPort->P2PCfg.ReadyToConnect == FALSE))
                            ;
                        else
#endif
                            PlatformIndicateConnectionStart(pAd, pPort, pAd->StaCfg.BssType, NULL, NULL, 0);
                    }
                }
                pAd->MlmeAux.BssIdx = 0;
                if (MlmeValidateSSID(pAd->MlmeAux.Ssid, pAd->MlmeAux.SsidLen))
                {
                    MlmeCntIterateOnBssTab(pPort);
                }
            }
        }
    } 
}

/*
    ==========================================================================
    Description:

    IRQL = DISPATCH_LEVEL
    
    ==========================================================================
*/
VOID MlmeCntlWscIterate(
    IN PMP_ADAPTER    pAd,
    IN PMP_PORT       pPort) 
{
    // Connect to the WPS-enabled AP by its BSSID directly.
    // Note: When WscState is equal to WSC_STATE_START,
    //       pPort->StaCfg.WscControl.WscAPBssid has been filled with valid BSSID.
    if (pPort->StaCfg.WscControl.WscState >= WSC_STATE_START)
    {
        // Set WSC state to WSC_STATE_START
        pPort->StaCfg.WscControl.WscState = WSC_STATE_START;
        pPort->StaCfg.WscControl.WscStatus = STATUS_WSC_START_ASSOC;
        
        // Reset auto-connect limitations
        pAd->MlmeAux.ScanForConnectCnt = 0;

        // The protocol is connecting to a partner.
        LedCtrlSetBlinkMode(pAd, LED_WPS_IN_PROCESS);
    
        MlmeEnqueue(pAd,
                    pPort,
                    MLME_CNTL_STATE_MACHINE,
                    MTK_OID_N5_SET_BSSID,
                    MAC_ADDR_LEN,
                    pPort->StaCfg.WscControl.WscAPBssid);
            
        pPort->Mlme.CntlMachine.CurrState = CNTL_IDLE;
            
    }
}

/*
    ==========================================================================
    Description:

    IRQL = DISPATCH_LEVEL
    
    ==========================================================================
*/
VOID MlmeCntlOidRTBssidProc(
    IN PMP_ADAPTER pAd, 
    IN MLME_QUEUE_ELEM * Elem) 
{
    ULONG       BssIdx;
    PUCHAR      pOidBssid = (PUCHAR)Elem->Msg;
    MLME_DISASSOC_REQ_STRUCT    DisassocReq;
    MLME_JOIN_REQ_STRUCT        JoinReq;
    PMP_PORT pPort = pAd->PortList[Elem->PortNum];

    //wake up since we want to connect with ssid.
    // Make sure Radio ON
    if(MT_TEST_FLAG(pAd, fRTMP_ADAPTER_IDLE_RADIO_OFF))
        AsicRadioOn(pAd);
    if (OPSTATUS_TEST_FLAG(pAd, fOP_STATUS_DOZE))
        AsicForceWakeup(pAd);

    DBGPRINT(RT_DEBUG_TRACE, ("CNTL - MlmeCntlOidRTBssidProc ...\n"));
    // record user desired settings
    COPY_MAC_ADDR(pAd->MlmeAux.Bssid, pOidBssid);
    pAd->MlmeAux.BssType = pAd->StaCfg.BssType;
    pPort->P2PCfg.P2pCounter.Counter100ms = 0;
    
    if ( (pPort->CommonCfg.bIEEE80211H) && (pAd->StaCfg.BssType == BSS_ADHOC) && (JapanChannelCheck(pPort->ScaningChannel)) )
    {       
        DBGPRINT(RT_DEBUG_TRACE, ("CNTL - We can't join ad hoc on Japan channel=%d\n", pPort->ScaningChannel));
        pPort->Mlme.CntlMachine.CurrState = CNTL_IDLE;
        return;     
    }

    pAd->MlmeAux.AssocState = dot11_assoc_state_unauth_unassoc;

    // find the desired BSS in the latest SCAN result table
    // if doing WPS, use the channel of WPS AP to find AP.
    if(pPort->StaCfg.WscControl.WscState != WSC_STATE_OFF)
    {
        BssIdx = BssTableSearch(pAd, pPort, &pAd->ScanTab, pOidBssid, pPort->StaCfg.WscControl.WscAPChannel);
    }
    else
    {
        BssIdx = BssTableSearch(pAd, pPort, &pAd->ScanTab, pOidBssid, pPort->ScaningChannel);
    }   
    if (BssIdx == BSS_NOT_FOUND) 
    {
        DBGPRINT(RT_DEBUG_TRACE, ("CNTL - BSSID not found. reply NDIS_STATUS_NOT_ACCEPTED\n"));
        pPort->Mlme.CntlMachine.CurrState = CNTL_IDLE;
        return;
    }

    // Set the STA state to operating state.
    MP_SET_STATE(pPort, OP_STATE);    

    // copy the matched BSS entry from ScanTab to MlmeAux.SsidBssTab. Why?
    // Because we need this entry to become the JOIN target in later on SYNC state machine
    pAd->MlmeAux.BssIdx = 0;
    pAd->MlmeAux.SsidBssTab.BssNr = 1;
    PlatformMoveMemory(&pAd->MlmeAux.SsidBssTab.BssEntry[0], &pAd->ScanTab.BssEntry[BssIdx], sizeof(BSS_ENTRY));

    //
    // Update Reconnect Ssid, that user desired to connect.
    //
    pAd->MlmeAux.AutoReconnectSsidLen = pAd->ScanTab.BssEntry[BssIdx].SsidLen;
    PlatformMoveMemory(pAd->MlmeAux.AutoReconnectSsid, pAd->ScanTab.BssEntry[BssIdx].Ssid, pAd->ScanTab.BssEntry[BssIdx].SsidLen);

    // Add SSID into MlmeAux for site surey joining hidden SSID
    pAd->MlmeAux.SsidLen = pAd->ScanTab.BssEntry[BssIdx].SsidLen;
    PlatformMoveMemory(pAd->MlmeAux.Ssid, pAd->ScanTab.BssEntry[BssIdx].Ssid, pAd->MlmeAux.SsidLen);    

    // 2002-11-26 skip the following checking. i.e. if user wants to re-connect to same AP
    //   we just follow normal procedure. The reason of user doing this may because he/she changed
    //   AP to another channel, but we still received BEACON from it thus don't claim Link Down.
    //   Since user knows he's changed AP channel, he'll re-connect again. By skipping the following
    //   checking, we'll disassociate then re-do normal association with this AP at the new channel.
    // 2003-1-6 Re-enable this feature based on microsoft requirement which prefer not to re-do
    //   connection when setting the same BSSID.
    if (OPSTATUS_TEST_FLAG(pPort, fOP_STATUS_MEDIA_STATE_CONNECTED) && 
        MAC_ADDR_EQUAL(pPort->PortCfg.Bssid, pOidBssid))
    {
        // already connected to the same BSSID, go back to idle state directly
        DBGPRINT(RT_DEBUG_TRACE, ("CNTL - already in this BSSID. ignore this SET_BSSID request\n"));
        pPort->Mlme.CntlMachine.CurrState = CNTL_IDLE;
    } 
    else 
    {
        if (INFRA_ON(pPort))
        {
            // disassoc from current AP first
            DBGPRINT(RT_DEBUG_TRACE, ("CNTL - disassociate with current AP ...\n"));
            MlmeCntDisassocParmFill(pAd, &DisassocReq, pPort->PortCfg.Bssid, REASON_DEAUTH_STA_LEAVING);
            MlmeEnqueue(pAd,pPort, ASSOC_STATE_MACHINE, MT2_MLME_DISASSOC_REQ, 
                        sizeof(MLME_DISASSOC_REQ_STRUCT), &DisassocReq);

            pPort->Mlme.CntlMachine.CurrState = CNTL_WAIT_DISASSOC;
        }
        else
        {
            if (ADHOC_ON(pPort))
            {
                DBGPRINT(RT_DEBUG_TRACE, ("CNTL - drop current ADHOC\n"));
                MlmeCntLinkDown(pPort, FALSE);
                OPSTATUS_CLEAR_FLAG(pPort, fOP_STATUS_MEDIA_STATE_CONNECTED);
                DBGPRINT(RT_DEBUG_TRACE, ("NDIS_STATUS_MEDIA_DISCONNECT Event C!\n"));
            }

            // Change the wepstatus to original wepstatus
            
            pPort->PortCfg.WepStatus   = pPort->PortCfg.OrigWepStatus;
            pPort->PortCfg.PairCipher  = pPort->PortCfg.OrigWepStatus;
            pPort->PortCfg.GroupCipher = pPort->PortCfg.OrigWepStatus;
            pPort->PortCfg.MixedModeGroupCipher = Cipher_Type_NONE;

            // Check cipher suite, AP must have more secured cipher than station setting
            // Set the Pairwise and Group cipher to match the intended AP setting
            // We can only connect to AP with less secured cipher setting
            if ((pPort->PortCfg.AuthMode == Ralink802_11AuthModeWPA) || (pPort->PortCfg.AuthMode == Ralink802_11AuthModeWPAPSK))
            {
                pPort->PortCfg.GroupCipher = pAd->ScanTab.BssEntry[BssIdx].WPA.GroupCipher;
        
                if (pPort->PortCfg.WepStatus == pAd->ScanTab.BssEntry[BssIdx].WPA.PairCipher)
                    pPort->PortCfg.PairCipher = pAd->ScanTab.BssEntry[BssIdx].WPA.PairCipher;
                else if (pAd->ScanTab.BssEntry[BssIdx].WPA.PairCipherAux != Ralink802_11WEPDisabled)
                    pPort->PortCfg.PairCipher = pAd->ScanTab.BssEntry[BssIdx].WPA.PairCipherAux;
                else    // There is no PairCipher Aux, downgrade our capability to TKIP
                    pPort->PortCfg.PairCipher = Ralink802_11Encryption2Enabled;         
                
                // Set the mixed mode group cipher suite
                pPort->PortCfg.MixedModeGroupCipher = pAd->ScanTab.BssEntry[BssIdx].MixedModeGroupCipher;
            }
            else if ((pPort->PortCfg.AuthMode == Ralink802_11AuthModeWPA2) || (pPort->PortCfg.AuthMode == Ralink802_11AuthModeWPA2PSK))
            {
                pPort->PortCfg.GroupCipher = pAd->ScanTab.BssEntry[BssIdx].WPA2.GroupCipher;
                
                if (pPort->PortCfg.WepStatus == pAd->ScanTab.BssEntry[BssIdx].WPA2.PairCipher)
                    pPort->PortCfg.PairCipher = pAd->ScanTab.BssEntry[BssIdx].WPA2.PairCipher;
                else if (pAd->ScanTab.BssEntry[BssIdx].WPA2.PairCipherAux != Ralink802_11WEPDisabled)
                    pPort->PortCfg.PairCipher = pAd->ScanTab.BssEntry[BssIdx].WPA2.PairCipherAux;
                else    // There is no PairCipher Aux, downgrade our capability to TKIP
                    pPort->PortCfg.PairCipher = Ralink802_11Encryption2Enabled;         

                // RSN capability
                pPort->PortCfg.RsnCapability = pAd->ScanTab.BssEntry[BssIdx].WPA2.RsnCapability;

                // 11w: STA should use SHA256 to do 4-way if AP's AKM is 5 or 6
                pPort->PortCfg.bAKMwSHA256 = pAd->ScanTab.BssEntry[BssIdx].WPA2.bAKMwSHA256;

            }
                
            // Set Mix cipher flag
            if (pPort->PortCfg.PairCipher != pPort->PortCfg.GroupCipher)
                pPort->PortCfg.bMixCipher = TRUE;       

            // No active association, join the BSS immediately
            DBGPRINT(RT_DEBUG_TRACE, ("CNTL - joining %02x:%02x:%02x:%02x:%02x:%02x ...\n",
                    pOidBssid[0],pOidBssid[1],pOidBssid[2],pOidBssid[3],pOidBssid[4],pOidBssid[5]));

            MlmeCntJoinParmFill(pAd, &JoinReq, pAd->MlmeAux.BssIdx);
            MlmeEnqueue(pAd,pPort, SYNC_STATE_MACHINE, MT2_MLME_JOIN_REQ, sizeof(MLME_JOIN_REQ_STRUCT), &JoinReq);

            pPort->Mlme.CntlMachine.CurrState = CNTL_WAIT_JOIN;
        } 
        
        if(pPort->StaCfg.WscControl.WscState != WSC_STATE_OFF)
        {
            pPort->PortCfg.AuthMode = pAd->ScanTab.BssEntry[BssIdx].AuthMode;
            pPort->PortCfg.WepStatus = pAd->ScanTab.BssEntry[BssIdx].WepStatus; 

            if(pAd->ScanTab.BssEntry[BssIdx].WPA2.bMixMode)
            {
                pPort->PortCfg.AuthMode = Ralink802_11AuthModeWPA2PSK;
            }
            
            if (pPort->PortCfg.AuthMode >= Ralink802_11AuthModeWPA2)
            {
                pPort->PortCfg.PairCipher = pAd->ScanTab.BssEntry[BssIdx].WPA2.PairCipher;
                pPort->PortCfg.GroupCipher = pAd->ScanTab.BssEntry[BssIdx].WPA2.GroupCipher;
            }
            else if (pPort->PortCfg.AuthMode >= Ralink802_11AuthModeWPA)
            {
                pPort->PortCfg.PairCipher = pAd->ScanTab.BssEntry[BssIdx].WPA.PairCipher;
                pPort->PortCfg.GroupCipher = pAd->ScanTab.BssEntry[BssIdx].WPA.GroupCipher;
            }
        }
    }
}

// Roaming is the only external request triggering CNTL state machine
// despite of other "SET OID" operation. All "SET OID" related oerations 
// happen in sequence, because no other SET OID will be sent to this device
// until the the previous SET operation is complete (successful o failed).
// So, how do we quarantee this ROAMING request won't corrupt other "SET OID"?
// or been corrupted by other "SET OID"?
//
// IRQL = DISPATCH_LEVEL
VOID MlmeCntlMlmeRoamingProc(
    IN PMP_ADAPTER pAd, 
    IN MLME_QUEUE_ELEM *Elem) 
{
    PMP_PORT pPort = pAd->PortList[Elem->PortNum];

    if(pPort->CommonCfg.bRoamingInProgress == TRUE)
        return;

    PlatformIndicateRoamingStart(pAd,pPort, DOT11_ROAMING_REASON_ASSOCIATION_LOST);
    // Make sure Radio ON
    if(MT_TEST_FLAG(pAd, fRTMP_ADAPTER_IDLE_RADIO_OFF))
        AsicRadioOn(pAd);
    if (OPSTATUS_TEST_FLAG(pAd, fOP_STATUS_DOZE))
        AsicForceWakeup(pAd);

    // TODO: 
    // AP in different channel may show lower RSSI than actual value??
    // should we add a weighting factor to compensate it?

    DBGPRINT(RT_DEBUG_TRACE,("CNTL - Roaming in MlmeAux.RoamTab...\n"));
    
    PlatformMoveMemory(&pAd->MlmeAux.SsidBssTab, &pAd->MlmeAux.RoamTab, sizeof(pAd->MlmeAux.RoamTab));
    pAd->MlmeAux.SsidBssTab.BssNr = pAd->MlmeAux.RoamTab.BssNr;

    BssTableSortByRssi(pAd, &pAd->MlmeAux.SsidBssTab);
    pAd->MlmeAux.BssIdx = 0;
    
    

    MlmeCntIterateOnBssTab(pPort);
}


/*
    ==========================================================================
    Description:

    IRQL = DISPATCH_LEVEL
    
    ==========================================================================
*/
VOID MlmeCntlOidDLSSetupProc(
    IN PMP_ADAPTER pAd,
    IN MLME_QUEUE_ELEM *Elem)
{
    PRT_802_11_DLS      pDLS = (PRT_802_11_DLS)Elem->Msg;
    MLME_DLS_REQ_STRUCT MlmeDlsReq;
    INT                 i;
    USHORT              reason = REASON_UNSPECIFY;
    PMP_PORT          pPort = pAd->PortList[Elem->PortNum];
    DBGPRINT(RT_DEBUG_TRACE,("CNTL - (OID set %02x:%02x:%02x:%02x:%02x:%02x with Valid=%d, Status=%d, TimeOut=%d, CountDownTimer=%d)\n",
        pDLS->MacAddr[0], pDLS->MacAddr[1], pDLS->MacAddr[2], pDLS->MacAddr[3], pDLS->MacAddr[4], pDLS->MacAddr[5],
        pDLS->Valid, pDLS->Status, pDLS->TimeOut, pDLS->CountDownTimer));

    if (!DLS_ON(pAd))
        return;

    // DLS will not be supported when Adhoc mode
    if (INFRA_ON(pPort))
    {
        for (i = (MAX_NUM_OF_DLS_ENTRY-1); i >= 0; i--)
        {
            if (pDLS->Valid && pAd->StaCfg.DLSEntry[i].Valid && (pAd->StaCfg.DLSEntry[i].Status == DLS_FINISH) &&
                (pDLS->TimeOut == pAd->StaCfg.DLSEntry[i].TimeOut) && MAC_ADDR_EQUAL(pDLS->MacAddr, pAd->StaCfg.DLSEntry[i].MacAddr)) 
            {
                // 1. Same setting, just drop it
                DBGPRINT(RT_DEBUG_TRACE,("CNTL - setting unchanged\n"));
                break;
            }
            else if (!pDLS->Valid && pAd->StaCfg.DLSEntry[i].Valid && (pAd->StaCfg.DLSEntry[i].Status == DLS_FINISH) &&
                MAC_ADDR_EQUAL(pDLS->MacAddr, pAd->StaCfg.DLSEntry[i].MacAddr)) 
            {
                // 2. Disable DLS link case, just tear down DLS link
                reason = REASON_QOS_UNWANTED_MECHANISM;
                pAd->StaCfg.DLSEntry[i].Valid   = FALSE;
                pAd->StaCfg.DLSEntry[i].Status  = DLS_NONE;
                MlmeCntDlsParmFill(pAd, &MlmeDlsReq, &pAd->StaCfg.DLSEntry[i], reason);
                MlmeEnqueue(pAd,pPort, DLS_STATE_MACHINE, MT2_MLME_DLS_TEAR_DOWN, sizeof(MLME_DLS_REQ_STRUCT), &MlmeDlsReq);
                DBGPRINT(RT_DEBUG_TRACE,("CNTL - start tear down procedure\n"));
                break;
            }
            else if ((i < MAX_NUM_OF_INIT_DLS_ENTRY) && pDLS->Valid && !pAd->StaCfg.DLSEntry[i].Valid) 
            {
                // 3. Enable case, start DLS setup procedure
                PlatformMoveMemory(&pAd->StaCfg.DLSEntry[i], pDLS, sizeof(RT_802_11_DLS_UI));
                pAd->StaCfg.DLSEntry[i].CountDownTimer = pAd->StaCfg.DLSEntry[i].TimeOut;
                MlmeCntDlsParmFill(pAd, &MlmeDlsReq, &pAd->StaCfg.DLSEntry[i], reason);
                MlmeEnqueue(pAd,pPort, DLS_STATE_MACHINE, MT2_MLME_DLS_REQ, sizeof(MLME_DLS_REQ_STRUCT), &MlmeDlsReq);
                DBGPRINT(RT_DEBUG_TRACE,("CNTL - DLS setup case\n"));
                break;
            }
            else if ((i < MAX_NUM_OF_INIT_DLS_ENTRY) && pDLS->Valid && pAd->StaCfg.DLSEntry[i].Valid &&
                (pAd->StaCfg.DLSEntry[i].Status == DLS_FINISH) && !MAC_ADDR_EQUAL(pDLS->MacAddr, pAd->StaCfg.DLSEntry[i].MacAddr)) 
            {
                // 4. update mac case, tear down old DLS and setup new DLS
                reason = REASON_QOS_UNWANTED_MECHANISM;
                pAd->StaCfg.DLSEntry[i].Valid   = FALSE;
                pAd->StaCfg.DLSEntry[i].Status  = DLS_NONE;
                MlmeCntDlsParmFill(pAd, &MlmeDlsReq, &pAd->StaCfg.DLSEntry[i], reason);
                MlmeEnqueue(pAd,pPort, DLS_STATE_MACHINE, MT2_MLME_DLS_TEAR_DOWN, sizeof(MLME_DLS_REQ_STRUCT), &MlmeDlsReq);
                PlatformMoveMemory(&pAd->StaCfg.DLSEntry[i], pDLS, sizeof(RT_802_11_DLS_UI));
                MlmeCntDlsParmFill(pAd, &MlmeDlsReq, &pAd->StaCfg.DLSEntry[i], reason);
                MlmeEnqueue(pAd,pPort, DLS_STATE_MACHINE, MT2_MLME_DLS_REQ, sizeof(MLME_DLS_REQ_STRUCT), &MlmeDlsReq);
                DBGPRINT(RT_DEBUG_TRACE,("CNTL - DLS tear down and restart case\n"));
                break;
            }
            else if (pDLS->Valid && pAd->StaCfg.DLSEntry[i].Valid && 
                MAC_ADDR_EQUAL(pDLS->MacAddr, pAd->StaCfg.DLSEntry[i].MacAddr) && (pAd->StaCfg.DLSEntry[i].TimeOut != pDLS->TimeOut)) 
            {
                // 5. update timeout case, start DLS setup procedure (no tear down)
                pAd->StaCfg.DLSEntry[i].TimeOut = pDLS->TimeOut;
                pAd->StaCfg.DLSEntry[i].CountDownTimer = pDLS->TimeOut;
                MlmeCntDlsParmFill(pAd, &MlmeDlsReq, &pAd->StaCfg.DLSEntry[i], reason);
                MlmeEnqueue(pAd,pPort, DLS_STATE_MACHINE, MT2_MLME_DLS_REQ, sizeof(MLME_DLS_REQ_STRUCT), &MlmeDlsReq);
                DBGPRINT(RT_DEBUG_TRACE,("CNTL - DLS update timeout case\n"));
                break;
            }
            else if (pDLS->Valid && pAd->StaCfg.DLSEntry[i].Valid &&
                (pAd->StaCfg.DLSEntry[i].Status != DLS_FINISH) && MAC_ADDR_EQUAL(pDLS->MacAddr, pAd->StaCfg.DLSEntry[i].MacAddr)) 
            {
                // 6. re-setup case, start DLS setup procedure (no tear down)
                MlmeCntDlsParmFill(pAd, &MlmeDlsReq, &pAd->StaCfg.DLSEntry[i], reason);
                MlmeEnqueue(pAd,pPort, DLS_STATE_MACHINE, MT2_MLME_DLS_REQ, sizeof(MLME_DLS_REQ_STRUCT), &MlmeDlsReq);
                DBGPRINT(RT_DEBUG_TRACE,("CNTL - DLS retry setup procedure\n"));
                break;
            }
            else
            {
                DBGPRINT(RT_DEBUG_WARN,("CNTL - DLS not changed in entry - %d - Valid=%d, Status=%d, TimeOut=%d\n",
                    i, pAd->StaCfg.DLSEntry[i].Valid, pAd->StaCfg.DLSEntry[i].Status, pAd->StaCfg.DLSEntry[i].TimeOut));
            }
        }
    }
}

/*
    ==========================================================================
    Description:

    IRQL = DISPATCH_LEVEL
    
    ==========================================================================
*/
VOID MlmeCntlWaitDisassocProc(
    IN PMP_ADAPTER pAd, 
    IN MLME_QUEUE_ELEM *Elem) 
{
    MLME_START_REQ_STRUCT     StartReq;
    PMP_PORT pPort = pAd->PortList[Elem->PortNum];

    if (Elem->MsgType == MT2_DISASSOC_CONF) 
    {   
        DBGPRINT(RT_DEBUG_TRACE, ("CNTL - Dis-associate successful\n"));
        MlmeCntLinkDown(pPort, FALSE);
        
        // case 1. no matching BSS, and user wants ADHOC, so we just start a new one        
        if ((pAd->MlmeAux.SsidBssTab.BssNr==0) && (pAd->StaCfg.BssType == BSS_ADHOC))
        {   
            DBGPRINT(RT_DEBUG_TRACE, ("CNTL - No matching BSS, start a new ADHOC (Ssid=%s)...\n",pAd->MlmeAux.Ssid));
            MlmeCntStartParmFill(pPort, &StartReq, pAd->MlmeAux.Ssid, pAd->MlmeAux.SsidLen);
            MlmeEnqueue(pAd,pPort, SYNC_STATE_MACHINE, MT2_MLME_START_REQ, sizeof(MLME_START_REQ_STRUCT), &StartReq);
            pPort->Mlme.CntlMachine.CurrState = CNTL_WAIT_START;            
        }
        // case 2. try each matched BSS
        else
        {           
            pAd->MlmeAux.BssIdx = 0;
            // If WSC is on, try WSC connect machine which did not check security sanity.
            if (pPort->StaCfg.WscControl.WscState == WSC_STATE_START)
            {           
                MlmeCntlWscIterate(pAd, pPort);
            }
            else
            {
                if(!(((pPort->StaCfg.WscControl.WscState == WSC_STATE_INIT)||
                    (pPort->StaCfg.WscControl.WscState == WSC_STATE_OFF))&&
                    (pPort->StaCfg.WscControl.bWPSSession)))
                {
                    MlmeCntIterateOnBssTab(pPort);
                }
            }
        }
    }
}

/*
    ==========================================================================
    Description:

    IRQL = DISPATCH_LEVEL
    
    ==========================================================================
*/
VOID MlmeCntlWaitJoinProc(
    IN PMP_ADAPTER pAd, 
    IN MLME_QUEUE_ELEM *Elem) 
{
    USHORT                      Reason;
    MLME_AUTH_REQ_STRUCT        AuthReq;
    PMP_PORT pPort = pAd->PortList[Elem->PortNum];
    
    if (Elem->MsgType == MT2_JOIN_CONF) 
    {
        PlatformMoveMemory(&Reason, Elem->Msg, sizeof(USHORT));
        if (Reason == MLME_SUCCESS) 
        {
            // 1. joined an IBSS, we are pretty much done here
            if (pAd->MlmeAux.BssType == BSS_ADHOC)
            {
                MlmeCntLinkUp(pPort, BSS_ADHOC);
                pPort->Mlme.CntlMachine.CurrState = CNTL_IDLE;
                DBGPRINT(RT_DEBUG_TRACE, ("CNTL - join the IBSS = %02x:%02x:%02x:%02x:%02x:%02x ...\n", 
                        pPort->PortCfg.Bssid[0],pPort->PortCfg.Bssid[1],pPort->PortCfg.Bssid[2],
                        pPort->PortCfg.Bssid[3],pPort->PortCfg.Bssid[4],pPort->PortCfg.Bssid[5]));
                OPSTATUS_SET_FLAG(pPort, fOP_STATUS_MEDIA_STATE_CONNECTED);
            } 
            // 2. joined a new INFRA network, start from authentication
            else 
            {
                // Add AuthMode "Network-EAP" for CCX, (only for LEAP)
                // StaCfg.CCXEnable is set after Assoc, or fail with Ralink802_11AuthModeOpen

                // either Ndis802_11AuthModeShared or Ndis802_11AuthModeAutoSwitch, try shared key first
                if (pPort->PortCfg.AuthMode == DOT11_AUTH_ALGO_80211_SHARED_KEY)
                {
                    MlmeCntAuthParmFill(pPort, &AuthReq, pAd->MlmeAux.Bssid, AUTH_SHARED_KEY);
                }
                else
                {
                    MlmeCntAuthParmFill(pPort, &AuthReq, pAd->MlmeAux.Bssid, AUTH_OPEN_SYSTEM);
                }

                MlmeEnqueue(pAd,pPort, AUTH_STATE_MACHINE, MT2_MLME_AUTH_REQ, 
                            sizeof(MLME_AUTH_REQ_STRUCT), &AuthReq);   //Invoke MlmeAuthReqAction

                pPort->Mlme.CntlMachine.CurrState = CNTL_WAIT_AUTH;
            }
        }
        else
        {
        #if 1
            //check if scan in progress
            if ( Reason == MLME_STATE_MACHINE_REJECT)
                XmitResumeMsduTransmission(pAd);
#if DBG
            if (Elem->MsgType == OID_DOT11_SCAN_REQUEST)
            {
                DBGPRINT(RT_DEBUG_TRACE, ("illeagal msg OID_DOT11_SCAN_REQUEST \n")); 
            }
#endif
        #endif
            // 3. failed, try next BSS

            // When WPS is enabled, the station reconnects by AP's BSSID in order to
            // deal with the multiple APs with the same SSID.
            if (pPort->StaCfg.WscControl.WscState >= WSC_STATE_START)
            {           
                MlmeCntlWscIterate(pAd, pPort);
            }
            else
            {
                pAd->MlmeAux.BssIdx++;
                
                if (pAd->MlmeAux.BssType == BSS_INFRA)
                {
#if(COMPILE_WIN8_ABOVE(CURRENT_OS_NDIS_VER))
                    // No AssociationCompletion with failure for wps connection request.
                    // Because we didn't indicate ConnectionStart yet.
                    if ((pPort->PortType == WFD_CLIENT_PORT) && (pPort->P2PCfg.WaitForWPSReady == TRUE) && (pPort->P2PCfg.ReadyToConnect == FALSE))
                        ;
                    else
#endif
                        PlatformIndicateAssociationCompletion(pAd, pPort, DOT11_ASSOC_STATUS_UNREACHABLE);
                }
                MlmeCntIterateOnBssTab(pPort);
            }
        }
    }
}


/*
    ==========================================================================
    Description:

    IRQL = DISPATCH_LEVEL
    
    ==========================================================================
*/
VOID MlmeCntlWaitStartProc(
    IN PMP_ADAPTER pAd, 
    IN MLME_QUEUE_ELEM *Elem) 
{
    USHORT      Result;
    PMP_PORT pPort = pAd->PortList[Elem->PortNum];
    
    if (Elem->MsgType == MT2_START_CONF) 
    {
        PlatformMoveMemory(&Result, Elem->Msg, sizeof(USHORT));
        if (Result == MLME_SUCCESS) 
        {
            MlmeCntLinkUp(pPort, BSS_ADHOC);
            pPort->Mlme.CntlMachine.CurrState = CNTL_IDLE;

            // Before send beacon, driver need do radar detection
            if ((pPort->CommonCfg.bIEEE80211H == 1) && RadarChannelCheck(pAd, pPort->Channel))
            {
                pPort->CommonCfg.RadarDetect.RDMode = RD_SILENCE_MODE;
                pPort->CommonCfg.RadarDetect.RDCount = 0;
                RadarDetectionStart(pAd);
            }

            PlatformIndicateAdhocAssociation(pAd, pPort, pPort->PortCfg.Bssid);

            DBGPRINT(RT_DEBUG_TRACE, ("CNTL - start a new IBSS = %02x:%02x:%02x:%02x:%02x:%02x ...\n", 
                pPort->PortCfg.Bssid[0],pPort->PortCfg.Bssid[1],pPort->PortCfg.Bssid[2],
                pPort->PortCfg.Bssid[3],pPort->PortCfg.Bssid[4],pPort->PortCfg.Bssid[5]));
        }
        else
        {
            DBGPRINT(RT_DEBUG_TRACE, ("CNTL - Start IBSS fail. BUG!!!!!\n"));
            pPort->Mlme.CntlMachine.CurrState = CNTL_IDLE;
        }
    }
}

/*
    ==========================================================================
    Description:

    IRQL = DISPATCH_LEVEL
    
    ==========================================================================
*/
VOID MlmeCntlWaitAuthProc(
    IN PMP_ADAPTER pAd, 
    IN MLME_QUEUE_ELEM *Elem) 
{
    USHORT                  Reason;
    MLME_ASSOC_REQ_STRUCT   AssocReq;
    MLME_AUTH_REQ_STRUCT    AuthReq;
    PMP_PORT              pPort = pAd->PortList[Elem->PortNum];

    if (Elem->MsgType == MT2_AUTH_CONF) 
    {
        PlatformMoveMemory(&Reason, Elem->Msg, sizeof(USHORT));
        if (Reason == MLME_SUCCESS) 
        {
            DBGPRINT(RT_DEBUG_TRACE, ("MlmeCntlWaitAuthProc - AUTH OK\n"));
            pAd->MlmeAux.AssocState = dot11_assoc_state_auth_unassoc;   
            pAd->MlmeAux.AssocRetryCount = 0;

            if(pPort->StaCfg.WscControl.WscState != WSC_STATE_OFF)
            {
                if(IS_DISABLE_WSC20TB_RSNIE(pAd))
                {
                    pAd->MlmeAux.CapabilityInfo = ((pAd->MlmeAux.CapabilityInfo) & 0xFFEF);
                    DBGPRINT(RT_DEBUG_TRACE, ("%s Privacy OFF in CapabilityInfo.\n", __FUNCTION__));

                }
            }
            
            MlmeCntAssocParmFill(pAd, &AssocReq, pAd->MlmeAux.Bssid, pAd->MlmeAux.CapabilityInfo, 
                      ASSOC_TIMEOUT, pAd->StaCfg.DefaultListenCount);

            MlmeEnqueue(pAd,pPort, ASSOC_STATE_MACHINE, MT2_MLME_ASSOC_REQ, 
                        sizeof(MLME_ASSOC_REQ_STRUCT), &AssocReq);

            pPort->Mlme.CntlMachine.CurrState = CNTL_WAIT_ASSOC;
        } 
        else
        {
            // This fail may because of the AP already keep us in its MAC table without 
            // ageing-out. The previous authentication attempt must have let it remove us.
            // so try Authentication again may help. For D-Link DWL-900AP+ compatibility.
            DBGPRINT(RT_DEBUG_TRACE, ("%s, MlmeCntlWaitAuthProc - AUTH FAIL, try again, Reason=%d...\n", __FUNCTION__, Reason));

            if (pPort->PortCfg.AuthMode == DOT11_AUTH_ALGO_80211_SHARED_KEY)
            {
                // either Ndis802_11AuthModeShared or Ndis802_11AuthModeAutoSwitch, try shared key first
                MlmeCntAuthParmFill(pPort, &AuthReq, pAd->MlmeAux.Bssid, AUTH_SHARED_KEY);
            }
            else
            {
                MlmeCntAuthParmFill(pPort, &AuthReq, pAd->MlmeAux.Bssid, AUTH_OPEN_SYSTEM);
            }

            MlmeEnqueue(pAd,pPort, AUTH_STATE_MACHINE, MT2_MLME_AUTH_REQ, 
                        sizeof(MLME_AUTH_REQ_STRUCT), &AuthReq);

            pPort->Mlme.CntlMachine.CurrState = CNTL_WAIT_AUTH2;            
        }
    }
}

/*
    ==========================================================================
    Description:

    IRQL = DISPATCH_LEVEL
    
    ==========================================================================
*/
VOID MlmeCntlWaitAuthProc2(
    IN PMP_ADAPTER pAd, 
    IN MLME_QUEUE_ELEM *Elem) 
{
    USHORT                  Reason;
    MLME_ASSOC_REQ_STRUCT   AssocReq;
    PMP_PORT              pPort = pAd->PortList[Elem->PortNum];
        
    if (Elem->MsgType == MT2_AUTH_CONF) 
    {
        PlatformMoveMemory(&Reason, Elem->Msg, sizeof(USHORT));
        if (Reason == MLME_SUCCESS) 
        {
            DBGPRINT(RT_DEBUG_TRACE, ("CNTL - AUTH OK\n"));
            pAd->MlmeAux.AssocState = dot11_assoc_state_auth_unassoc;   
            pAd->MlmeAux.AssocRetryCount = 0;
            
            MlmeCntAssocParmFill(pAd, &AssocReq, pAd->MlmeAux.Bssid, pAd->MlmeAux.CapabilityInfo, 
                          ASSOC_TIMEOUT, pAd->StaCfg.DefaultListenCount);
            MlmeEnqueue(pAd,pPort, ASSOC_STATE_MACHINE, MT2_MLME_ASSOC_REQ, 
                        sizeof(MLME_ASSOC_REQ_STRUCT), &AssocReq);

            pPort->Mlme.CntlMachine.CurrState = CNTL_WAIT_ASSOC;
        } 
        else
        {       
// Disable authentication auto switch mode.
#if 0               
            else if ((pPort->CommonCfg.AuthMode == DOT11_AUTH_ALGO_80211_SHARED_KEY) &&
                 (pAd->MlmeAux.Alg == AUTH_SHARED_KEY))
            {
                DBGPRINT(RT_DEBUG_TRACE, ("CNTL - AUTH FAIL, try OPEN system...\n"));
                MlmeCntAuthParmFill(pPort, &AuthReq, pAd->MlmeAux.Bssid, AUTH_OPEN_SYSTEM);
                MlmeEnqueue(pPort, AUTH_STATE_MACHINE, MT2_MLME_AUTH_REQ, 
                            sizeof(MLME_AUTH_REQ_STRUCT), &AuthReq);

                pPort->Mlme.CntlMachine.CurrState = CNTL_WAIT_AUTH2;
            }
#endif                      
            {
                // not success, try next BSS
                DBGPRINT(RT_DEBUG_TRACE, ("CNTL - AUTH FAIL, give up; try next BSS\n"));

                // When WPS is enabled, the station reconnects by AP's BSSID in order to
                // deal with the multiple APs with the same SSID.
                if (pPort->StaCfg.WscControl.WscState >= WSC_STATE_START)
                {               
                    MlmeCntlWscIterate(pAd, pPort);
                }
                else
                {
                    if(P2P_ON(pPort) && (pPort->P2PCfg.P2PConnectState == P2P_I_AM_CLIENT_ASSOC_AUTH))
                    {   
                        // Reset auto-connect limitations
                        pAd->MlmeAux.ScanForConnectCnt = 0;

                        MlmeEnqueue(pAd,
                                    pPort,
                                    MLME_CNTL_STATE_MACHINE,
                                    MTK_OID_N5_SET_BSSID,
                                    MAC_ADDR_LEN,
                                    pPort->StaCfg.WscControl.WscAPBssid);

                        pPort->Mlme.CntlMachine.CurrState = CNTL_IDLE;
                    }
                    else
                    {
                        pPort->Mlme.CntlMachine.CurrState = CNTL_IDLE; //???????
                        // keep in same BssIdx one time
                        PlatformIndicateAssociationCompletion(pAd, pPort, DOT11_ASSOC_STATUS_UNREACHABLE);
                        MlmeCntIterateOnBssTab(pPort);
                        pAd->MlmeAux.BssIdx++;
                    }
                }
            }
        }
    }
}

/*
    ==========================================================================
    Description:

    IRQL = DISPATCH_LEVEL
    
    ==========================================================================
*/
VOID MlmeCntlWaitAssocProc(
    IN PMP_ADAPTER pAd, 
    IN MLME_QUEUE_ELEM *Elem) 
{
    NDIS_STATUS     ndisStatus = NDIS_STATUS_FAILURE;
    USHORT          Reason;
    MAC_TABLE_ENTRY *pEntry = NULL; 
    PMP_PORT      pPort = pAd->PortList[Elem->PortNum];
    ULONGLONG       Now64;
    PMAC_TABLE_ENTRY pP2pClientEntry = NULL;
    PMAC_TABLE_ENTRY pBssidEntry = MlmeSyncMacTabMatchedRoleType(pPort, ROLE_WLANIDX_BSSID);
    PMAC_TABLE_ENTRY pWcidMacTabEntry = NULL;       
    
    if (Elem->MsgType == MT2_ASSOC_CONF) 
    {
        PlatformMoveMemory(&Reason, Elem->Msg, sizeof(USHORT));
        if (Reason == MLME_SUCCESS) 
        {
            PlatformIndicateAssociationCompletion(pAd,pPort, DOT11_ASSOC_STATUS_SUCCESS);

            //
            // On WPA mode, Remove All Keys if not connect to the last BSSID
            // Key will be set after 4-way handshake.
            //
            // Considering CCKM, currently it may discard the 4-way handshaking on fast roaming.
            //       The key may be set during Reassociation, so we can't delete the key on this time.
            //
             if (IS_P2P_CON_CLI(pAd, pPort) || IS_P2P_MS_CLI(pAd, pPort) || IS_P2P_MS_CLI2(pAd, pPort))
             {
                pWcidMacTabEntry = MlmeSyncMacTabMatchedWlanIdx(pPort, P2pGetClientWcid(pAd, pPort));
             }
            
            if ((pPort->PortCfg.AuthMode >= Ralink802_11AuthModeWPA) && 
                (!PlatformEqualMemory(pPort->PortCfg.LastBssid, pPort->PortCfg.Bssid, MAC_ADDR_LEN)))
            {
                // Remove all WPA keys 
                //do not do this ,because it will remove  AP mode key table and mac table
                //MlmeInfoWPARemoveAllKeys(pAd,pPort);

                if (IS_P2P_CON_CLI(pAd, pPort) || IS_P2P_MS_CLI(pAd, pPort) || IS_P2P_MS_CLI2(pAd, pPort))
                {
                    if(pWcidMacTabEntry != NULL)
                    {
                        pWcidMacTabEntry->PortSecured = WPA_802_1X_PORT_NOT_SECURED;
                    }
                }
                else    
                    pAd->StaCfg.PortSecured = WPA_802_1X_PORT_NOT_SECURED;          
            }

            pEntry = MlmeSyncMacTabMatchedRoleType(pPort, GetWlanRoleType(pPort));
            
            if (pEntry != NULL)
            {
                MacTableDeleteAndResetEntry(pAd, pPort, pEntry->wcid,pEntry->Addr, FALSE);
                COPY_MAC_ADDR(pEntry->Addr, pAd->MlmeAux.Bssid);
            }
            else
            {
                pEntry = MacTableInsertEntry(pAd, pPort, pAd->MlmeAux.Bssid, FALSE);
            }
            
            MlmeInfoInitCipherAttrib(pAd, pPort, pEntry->Addr);
            
            pEntry->Aid = (UCHAR)pAd->MlmeAux.Aid;

            //
            // Update Last RX Timestamp once new entry inserted to mac table
            // NdisTest: Occur beacon lost between assoc process and linkup.
            NdisGetCurrentSystemTime((PLARGE_INTEGER)&Now64);
            if(IS_P2P_CON_CLI(pAd, pPort) || IS_P2P_MS_CLI(pAd, pPort) || IS_P2P_MS_CLI2(pAd, pPort))
            {
                pP2pClientEntry = MlmeSyncMacTabMatchedRoleType(pPort, ROLE_WLANIDX_P2P_CLIENT);
            }
            
            if ((pP2pClientEntry != NULL) &&(IS_P2P_CON_CLI(pAd, pPort) || IS_P2P_MS_CLI(pAd, pPort) || IS_P2P_MS_CLI2(pAd, pPort)) && pP2pClientEntry->ValidAsCLI)
            {
                pWcidMacTabEntry->LastBeaconRxTime = Now64;
            }
            else if ((pBssidEntry != NULL)&&(pBssidEntry->ValidAsCLI == TRUE))
            {
                pAd->StaCfg.LastBeaconRxTime = Now64;
            }   

            //
            // Update special flags here that XmitHardTransmit may use.
            //
            if ((pAd->MlmeAux.APRalinkIe & 0x00000002) == 2)
            {
                if(pBssidEntry != NULL)
                {
                    CLIENT_STATUS_SET_FLAG(pBssidEntry, fCLIENT_STATUS_PIGGYBACK_CAPABLE);  
                }
                AsicEnablePiggyB(pAd);
                DBGPRINT(RT_DEBUG_TRACE, ("Turn on Piggy-Back\n"));
            }
            if (pAd->MlmeAux.APRalinkIe & 0x00000001)
            {
                if(pBssidEntry != NULL)
                {
                    CLIENT_STATUS_SET_FLAG(pBssidEntry, fOP_STATUS_AGGREGATION_INUSED);  
                }            
                OPSTATUS_SET_FLAG(pAd, fOP_STATUS_AGGREGATION_INUSED);
                DBGPRINT(RT_DEBUG_TRACE, ("Turn on Ralink Aggregation\n"));             
            }
            if (pAd->MlmeAux.APRalinkIe != 0x0)
            {
                if(pBssidEntry != NULL)
                {
                    CLIENT_STATUS_SET_FLAG(pBssidEntry, fCLIENT_STATUS_RALINK_CHIPSET);  
                }            
                DBGPRINT(RT_DEBUG_TRACE, ("--->Assoc with Ralink Chipset \n"));
            }
            
            // indicate link speed here to make sure the information for netsh wlan command is correct
            N6PlatformIndicateLinkSpeedForNetshCmd(pAd);

            DBGPRINT(RT_DEBUG_TRACE, ("TONDIS:  MlmeCntlWaitAssocProc, NDIS_STATUS_DOT11_ASSOCIATION_COMPLETION SUCCESS[%02x:%02x:%02x:%02x:%02x:%02x]\n",
                        pAd->MlmeAux.Bssid[0], pAd->MlmeAux.Bssid[1], pAd->MlmeAux.Bssid[2], pAd->MlmeAux.Bssid[3], pAd->MlmeAux.Bssid[4], pAd->MlmeAux.Bssid[5]));                 

            MlmeCntLinkUp(pPort, BSS_INFRA);
            pPort->Mlme.CntlMachine.CurrState = CNTL_IDLE;
            //
            // Patch some AP on WPA, WPA-PSK, WPA2 and WPA2-PSK mode that they send first 4Way EAPOL frame before station indicate connect event to OS.
            // In this case, we are on disconnect state so that 802.1x protocol driver will dropt this packet.
            // But if AP didn't retransmit first 4way packet, we will not connect to this AP anymore.
            // For example: Linksys model: BEFW11S4 Ver.4 it send first 4way packet very fast within 2 ms.
            //
            pAd->pRxCfg->bQueueEAPOL = FALSE;
            if (pAd->pRxCfg->RxEAPOLLen != 0)
            {
                // Indicate the queued EAPOL packet after link up.
                NdisCommonReportEthFrameToLLC(pAd, pPort,(PUCHAR)(pAd->pRxCfg->pRxEAPOL), (USHORT)(pAd->pRxCfg->RxEAPOLLen), pAd->pRxCfg->UserPriorityOfQueuedEAPOL, FALSE, TRUE);

                pAd->pRxCfg->RxEAPOLLen = 0;
                pAd->pRxCfg->UserPriorityOfQueuedEAPOL = 0;
                INC_COUNTER64(pAd->Counter.Counters8023.GoodReceives);
                DBGPRINT(RT_DEBUG_TRACE, ("%s: Indicate the queued EAPOL packet.\n", __FUNCTION__));
                MlmeFreeMemory(pAd, pAd->pRxCfg->pRxEAPOL);
            }           

            DBGPRINT(RT_DEBUG_TRACE, ("CNTL - Association successful on BSS #%d\n",pAd->MlmeAux.BssIdx));
        } 
        else 
        {
#ifdef MULTI_CHANNEL_SUPPORT
            // not success, try next BSS
            DBGPRINT(RT_DEBUG_TRACE, ("CNTL - Association fails(%d) and try next BSS\n", Reason));
#endif /*MULTI_CHANNEL_SUPPORT*/            

            // When WPS is enabled, the station reconnects by AP's BSSID in order to
            // deal with the multiple APs with the same SSID.
            if (pPort->StaCfg.WscControl.WscState >= WSC_STATE_START)
            {           
                MlmeCntlWscIterate(pAd, pPort);
            }
            else
            {
                // not success, try next BSS
                DBGPRINT(RT_DEBUG_TRACE, ("CNTL - Association fails on BSS #%d\n",pAd->MlmeAux.BssIdx));
                PlatformIndicateAssociationCompletion(pAd,pPort, DOT11_ASSOC_STATUS_UNREACHABLE);
                MlmeCntIterateOnBssTab(pPort);
                if (pPort->PortSubtype == EXTSTA_PORT)
                {
                    pAd->MlmeAux.BssIdx++;
                }
                   
            }
        }
    }
}

/*
    ==========================================================================
    Description:

    IRQL = DISPATCH_LEVEL
    
    ==========================================================================
*/
VOID MlmeCntlWaitReassocProc(
    IN PMP_ADAPTER pAd, 
    IN MLME_QUEUE_ELEM *Elem) 
{
    USHORT      Result;
    PMP_PORT  pPort = pAd->PortList[Elem->PortNum];

    if (Elem->MsgType == MT2_REASSOC_CONF) 
    {
        PlatformMoveMemory(&Result, Elem->Msg, sizeof(USHORT));
        if (Result == MLME_SUCCESS) 
        {
            PlatformIndicateAssociationCompletion(pAd,pPort,DOT11_ASSOC_STATUS_SUCCESS);

            // 
            // NDIS requires a new Link UP indication but no Link Down for RE-ASSOC
            //
            MlmeCntLinkUp(pPort, BSS_INFRA);

            pPort->Mlme.CntlMachine.CurrState = CNTL_IDLE;
            DBGPRINT(RT_DEBUG_TRACE, ("CNTL - Re-assocition successful on BSS #%d\n", pAd->MlmeAux.RoamIdx));
        } 
        else 
        {
            // reassoc failed, try to pick next BSS in the BSS Table
            DBGPRINT(RT_DEBUG_TRACE, ("CNTL - Re-assocition fails on BSS #%d\n", pAd->MlmeAux.RoamIdx));
            pAd->MlmeAux.RoamIdx++;
            PlatformIndicateAssociationCompletion(pAd,pPort, DOT11_ASSOC_STATUS_UNREACHABLE);
            MlmeCntIterateOnBssTab2(pPort);
        }
    }
}
/*
    ==========================================================================
    Description:

    IRQL = DISPATCH_LEVEL
    
    ==========================================================================
*/
VOID MlmeCntlOidTDLSRequestProc(
    IN PMP_ADAPTER pAd,
    IN MLME_QUEUE_ELEM *Elem)
{
    PRT_802_11_TDLS     pTDLS = (PRT_802_11_TDLS)Elem->Msg;
    MLME_TDLS_REQ_STRUCT    MlmeTdlsReq;
    USHORT              Reason = REASON_UNSPECIFY;
    BOOLEAN             TimerCancelled;
    INT                 Idx, i;
    PMP_PORT          pPort = pAd->PortList[Elem->PortNum];
    
    DBGPRINT(RT_DEBUG_TRACE,("CNTL - MlmeCntlOidTDLSRequestProc set %02x:%02x:%02x:%02x:%02x:%02x with Valid=%d, Status=%d TimeOut=%d\n",
        pTDLS->MacAddr[0], pTDLS->MacAddr[1], pTDLS->MacAddr[2], pTDLS->MacAddr[3], pTDLS->MacAddr[4], pTDLS->MacAddr[5],
        pTDLS->Valid, pTDLS->Status, pTDLS->TimeOut));

    if (!TDLS_ON(pAd))
        return;

    if (!INFRA_ON(pPort))
        return;

    Idx = TdlsSearchLinkId(pAd, pTDLS->MacAddr);
    
    if (Idx == -1) // not found and the entry is not full
    {
        if (pTDLS->Valid) 
        {
            // 1. Enable case, start TDLS setup procedure
            for (i = 0; i < MAX_NUM_OF_TDLS_ENTRY; i++)
            {
                if (!pAd->StaCfg.TDLSEntry[i].Valid)
                {
                    PlatformMoveMemory(&pAd->StaCfg.TDLSEntry[i], pTDLS, sizeof(RT_802_11_DLS_UI));
                    MlmeCntTdlsParmFill(pAd, &MlmeTdlsReq, &pAd->StaCfg.TDLSEntry[i], Reason);
                    MlmeEnqueue(pAd, pPort, TDLS_STATE_MACHINE, MT2_MLME_TDLS_SETUP_REQ, sizeof(MLME_TDLS_REQ_STRUCT), &MlmeTdlsReq);
                    DBGPRINT(RT_DEBUG_TRACE,("CNTL - TDLS setup case\n"));
                    break;
                }
            }   
        }
        else
            DBGPRINT(RT_DEBUG_WARN,("CNTL - TDLS not changed in Idx = -1 (Valid=%d)\n", pTDLS->Valid));

    }
    else  if (Idx == MAX_NUM_OF_TDLS_ENTRY)     // not found and full
    {
        /*if (pTDLS->Valid) 
        {
            // 2. table full, cancel the non-finished entry and restart a new one
            for (i = 0; i < MAX_NUM_OF_TDLS_ENTRY; i++)
            {
                if ((pAd->StaCfg.TDLSEntry[i].Valid) &&(pAd->StaCfg.TDLSEntry[i].Status < TDLS_MODE_CONNECTED))
                {
                    // update mac case
                    PlatformMoveMemory(&pAd->StaCfg.TDLSEntry[i], pTDLS, sizeof(RT_802_11_TDLS));
                    MlmeCntTdlsParmFill(pAd, &MlmeTdlsReq, &pAd->StaCfg.TDLSEntry[i], Reason);
                    MlmeEnqueue(pAd, pPort, TDLS_STATE_MACHINE, MT2_MLME_TDLS_SETUP_REQ, sizeof(MLME_TDLS_REQ_STRUCT), &MlmeTdlsReq);
                    DBGPRINT(RT_DEBUG_TRACE,("CNTL - TDLS restart case\n"));
                    break;
                }
            }
        }
        else */
            DBGPRINT(RT_DEBUG_WARN,("CNTL - TDLS not changed in Idx = MAX_NUM_OF_TDLS_ENTRY (Valid=%d)\n", pTDLS->Valid));
    }
    else    // found one in entry
    {
        if ((!pTDLS->Valid) && (pAd->StaCfg.TDLSEntry[Idx].Status == TDLS_MODE_CONNECTED))
        {
            // If not the base channel, the TDLS peer STA shall switch back 
            // to the base channel before transmitting the Teardown frame.
            
            // 3. Disable TDLS link case, just tear down TDLS link
            Reason = REASON_TDLS_UNSPECIFY;
            pAd->StaCfg.TDLSEntry[Idx].Valid    = FALSE;
            pAd->StaCfg.TDLSEntry[Idx].Status   = TDLS_MODE_NONE;
            MlmeCntTdlsParmFill(pAd, &MlmeTdlsReq, &pAd->StaCfg.TDLSEntry[Idx], Reason);
            MlmeEnqueue(pAd, pPort, TDLS_STATE_MACHINE, MT2_MLME_TDLS_TEAR_DOWN, sizeof(MLME_TDLS_REQ_STRUCT), &MlmeTdlsReq);
            DBGPRINT(RT_DEBUG_TRACE,("CNTL - start tear down procedure\n"));
        }
        else if ((pTDLS->Valid) && (pAd->StaCfg.TDLSEntry[Idx].Status == TDLS_MODE_CONNECTED)) 
        {
            // If not the base channel, the TDLS peer STA shall switch back 
            // to the base channel before transmitting the Teardown frame.

            // 4. re-setup case, tear down old link and re-start TDLS setup procedure 
            Reason = REASON_TDLS_UNSPECIFY;
            pAd->StaCfg.TDLSEntry[Idx].Valid    = FALSE;
            pAd->StaCfg.TDLSEntry[Idx].Status   = TDLS_MODE_NONE;
            MlmeCntTdlsParmFill(pAd, &MlmeTdlsReq, &pAd->StaCfg.TDLSEntry[Idx], Reason);
            MlmeEnqueue(pAd, pPort, TDLS_STATE_MACHINE, MT2_MLME_TDLS_TEAR_DOWN, sizeof(MLME_TDLS_REQ_STRUCT), &MlmeTdlsReq);
        
            PlatformCancelTimer(&pPort->StaCfg.TDLSEntry[Idx].Timer, &TimerCancelled);
            PlatformMoveMemory(&pAd->StaCfg.TDLSEntry[Idx], pTDLS, sizeof(RT_802_11_DLS_UI));
            MlmeCntTdlsParmFill(pAd, &MlmeTdlsReq, &pAd->StaCfg.TDLSEntry[Idx], Reason);
            MlmeEnqueue(pAd, pPort, TDLS_STATE_MACHINE, MT2_MLME_TDLS_SETUP_REQ, sizeof(MLME_TDLS_REQ_STRUCT), &MlmeTdlsReq);
            DBGPRINT(RT_DEBUG_TRACE,("CNTL - TDLS retry setup procedure\n"));
        }
        else
        {
            DBGPRINT(RT_DEBUG_WARN,("CNTL - TDLS not changed in entry - %d - Valid=%d, Status=%d\n",
                Idx, pAd->StaCfg.TDLSEntry[Idx].Valid, pAd->StaCfg.TDLSEntry[Idx].Status));
        }
    }

}

/*
    ========================================================================
    
    Routine Description:
        Control Primary, Central Channel, ChannelWidth and Second Channel Offset

    Arguments:
        pAd                     Pointer to our adapter
        PrimaryChannel          Primary Channel
        CentralChannel          Central Channel
        ChannelWidth                BW_20 or BW_40
        SecondaryChannelOffset  EXTCHA_NONE, EXTCHA_ABOVE and EXTCHA_BELOW
        
    Return Value:
        None
        
    Note:
        
    ========================================================================
*/

VOID MlmeCntlChannelWidth(
        IN PMP_ADAPTER    pAd,
        IN UCHAR            PrimaryChannel,
        IN UCHAR            CentralChannel,      
        IN UCHAR            ChannelWidth,
        IN UCHAR            SecondaryChannelOffset) 
{
}

/*
    ==========================================================================
    Description:

    IRQL = DISPATCH_LEVEL
    
    ==========================================================================
*/
VOID MlmeCntLinkUp(
    IN PMP_PORT pPort,
    IN UCHAR BssType) 
{
    ULONGLONG   Now64;
    BOOLEAN Cancelled = FALSE;
    UCHAR   idx;
    ULONG   Value = BW_40;
    MAC_TABLE_ENTRY *pEntry;
//  TX_RTY_CFG_STRUC    TxRtyCfg;
//  TX_RTS_CFG_STRUC RtsCfg;
    PMP_ADAPTER   pAd = pPort->pAd;
    PMP_PORT  pAPPort = NULL;
    MP_RW_LOCK_STATE LockState;
    //MAC_DW0_STRUC       StaMacReg0;
    //MAC_DW1_STRUC       StaMacReg1;
    UCHAR               BssIndex = 0;
    UCHAR               Wcid = BSSID_WCID, CheckPortWcid = RESERVED_WCID, CheckPort2Wcid = RESERVED_WCID;
    PMP_PORT          pCheckPort = NULL, pCheckPort2 = NULL;
    BOOLEAN             bCoexistPiggyBack = TRUE, bCoexistAggregation = TRUE, bDisableRDG = FALSE;
    PMAC_TABLE_ENTRY pBssidMacTabEntry = MlmeSyncMacTabMatchedRoleType(pPort, ROLE_WLANIDX_BSSID);
    PMAC_TABLE_ENTRY pWcidMacTabEntry = NULL;
//  ULONG               MinimumAMPDUSize, Value32;

    Wcid = MlmeSyncGetWlanIdxByPort(pPort, ROLE_WLANIDX_BSSID);
    // roam is succeed
    pAd->StaCfg.bFastRoamingScan = FALSE;

    // Init ChannelQuality to prevent DEAD_CQI at initial MlmeCntLinkUp
    pPort->Mlme.ChannelQuality = 50;
    
    // init GPIO checking round
    //pAd->Mlme.GPIORound = 0;

    pPort->CommonCfg.DelBACount = 0;  // for delBA

    // Reset it. MlmeSelectTxRateTable will update LimitTxRateTable.
    pPort->CommonCfg.LimitTxRateTableSize = 0;
    // Init set as 50.  to meet 2 step site survey criteria in OID_802_11_BSSID_LIST_SCAN
    pAd->Counter.MTKCounters.Last30SecTotalRxCount = 50;
    
    // Within 10 seconds after link up, don't allow to go to sleep.
    pAd->TrackInfo.CountDowntoPsm = STAY_10_SECONDS_AWAKE;
    //
    // ASSOC - MlmeAssocDisAssocTimeoutTimerCallbackAction
    // CNTL - Dis-associate successful
    // !!! LINK DOWN !!!
    // [88888] OID_802_11_SSID should have returned NDTEST_WEP_AP2(Returned: )
    // 
    // To prevent MlmeAssocDisAssocTimeoutTimerCallbackAction to call Link down after we link up,
    // cancel the DisassocTimer no matter what it start or not.
    //
    PlatformCancelTimer(&pPort->Mlme.DisassocTimer,  &Cancelled);  

    // Within 10 seconds after link up, don't allow to go to sleep.
    //pAd->TrackInfo.CountDowntoPsm = STAY_10_SECONDS_AWAKE;

    // === Multiple STA+Clients: Found out all possible CheckPort(s) =======
    // Current connecting port is sta port. Check other two clients.
    if (pPort->PortSubtype == PORTSUBTYPE_STA)
    {
        // get the current wcid
        Wcid = MlmeSyncGetWlanIdxByPort(pPort, ROLE_WLANIDX_BSSID);
        pCheckPort = MlmeSyncGetP2pClientPort(pAd, 0);
        if (pCheckPort)
            CheckPortWcid = P2pGetClientWcid(pAd, pCheckPort);
        
        pCheckPort2 = MlmeSyncGetP2pClientPort(pAd, 1);
        if (pCheckPort2)
            CheckPort2Wcid = P2pGetClientWcid(pAd, pCheckPort2);
    }
    // Current connecting port is p2p client port. Check sta and the other p2p client.
    else if (pPort->PortSubtype == PORTSUBTYPE_P2PClient)
    {
        PMP_PORT      pTmpPort = NULL;

        // get the current wcid
        Wcid = P2pGetClientWcid(pAd, pPort);

        pCheckPort = Ndis6CommonGetStaPort(pAd);
        if (pCheckPort)
            CheckPortWcid =MlmeSyncGetWlanIdxByPort(pPort, ROLE_WLANIDX_BSSID);

        for (idx = 0; idx < NUM_OF_CONCURRENT_CLIENT_ROLE; idx++)
        {
            pTmpPort = MlmeSyncGetP2pClientPort(pAd, idx);
            // this port is not the current port, found it
            if (pTmpPort && (pTmpPort->PortNumber != pPort->PortNumber))
                break;
            else
                pTmpPort = NULL;
        }
        pCheckPort2 = pTmpPort;
        if (pCheckPort2)
            CheckPort2Wcid = P2pGetClientWcid(pAd, pCheckPort2);
    }
    DBGPRINT(RT_DEBUG_TRACE, ("LinkUP(): Wcid=%d.  concurrent clients - 1'st Wcid=%d,  2'nd Wcid=%d\n", Wcid, CheckPortWcid, CheckPort2Wcid));

    pWcidMacTabEntry = MlmeSyncMacTabMatchedWlanIdx(pPort, Wcid);
    Wcid = (UCHAR)pWcidMacTabEntry->wcid;
    
    //When fast roaming takes place, the MlmeCntLinkDown() function, which will
    //remove last connected BSSID, will never be called, so we have to 
    //remove it here before calling COPY_SETTINGS_FROM_MLME_AUX_TO_ACTIVE_CFG() macro.

    //We will do MacTableDeleteAndResetEntry in MlmeCntlWaitAssocProc in INFRA mode
    if( BssType == BSS_ADHOC )
    {
        pEntry = MacTableLookup(pAd, pPort, pPort->PortCfg.LastBssid);
        if (pEntry)
        {
            MacTableDeleteAndResetEntry(pAd, pPort, pEntry->Aid, pPort->PortCfg.LastBssid , TRUE);
        }
    }

    // Copy MlmeAux to ActiveCfg when it's the first association or it's StaPort.
    if ((pPort->PortNumber == PORT_0) || (IDLE_ON(pAd->PortList[PORT_0])))
    {
        DBGPRINT(RT_DEBUG_TRACE, ("%s: Copy settings from mlme aux to active cfg \n", __FUNCTION__));
        COPY_SETTINGS_FROM_MLME_AUX_TO_ACTIVE_CFG(pAd, pPort);
        COPY_HTSETTINGS_FROM_MLME_AUX_TO_ACTIVE_CFG(pAd);
#ifdef MULTI_CHANNEL_SUPPORT
        //pPort->P2PCfg.InfraCentralChannel = pPort->CentralChannel;
        pPort->P2PCfg.InfraChannel = pPort->Channel;
        DBGPRINT(RT_DEBUG_TRACE, ("%s[%d] - MultiChSts, STA CentralChannel=%d, STA common Channel=%d\n", __FUNCTION__, __LINE__, pPort->CentralChannel, pPort->Channel));
#endif /* MULTI_CHANNEL_SUPPORT */
        if((pPort->CommonCfg.PhyMode==PHY_11VHT)   // 1. we support VHT
            && (pAd->MlmeAux.bVhtCapable==TRUE)) // 2. the otherside support VHT
        {
            pAd->StaActive.SupportedVhtPhy.bVhtEnable = TRUE;
            PlatformMoveMemory(&pAd->StaActive.SupportedVhtPhy.VhtCapability, &pAd->MlmeAux.VhtCapability, SIZE_OF_VHT_CAP_IE);
            PlatformMoveMemory(&pAd->StaActive.SupportedVhtPhy.VhtOperation, &pAd->MlmeAux.VhtOperation, SIZE_OF_VHT_OP_IE);
            VhtDecideTheMaxMCSForSpatialStream(
                    &pAd->StaActive.SupportedVhtPhy.VhtOperation.VhtBasicMcsSet,
                    &pPort->CommonCfg.DesiredVhtPhy.VhtOperation.VhtBasicMcsSet,
                    &pAd->MlmeAux.VhtOperation.VhtBasicMcsSet);
        }
    }
    // Copy association settings to my Entry and my Port when I am concurrent P2PClient.
    P2pCopySettingsWhenLinkup(pAd, pPort);
    
    pAd->MlmeAux.bWCN = FALSE;

    /*
    if(pPort->CommonCfg.PhyMode == PHY_11VHT)
    {
        
    }// todo: store primay chnnel, bw, central channel information
    else
    */
    if(pPort->CommonCfg.PhyMode >= PHY_11ABGN_MIXED)
    {
    //
    // check if BW40 is disable in EEPROM
    //
        if(pPort->Channel <= 14) // 2.4G
        {
            // BW40 is NOT allowed in 2.4G band
            if(pAd->HwCfg.NicConfig2.field.bDisableBW40ForG)
            {   
                pPort->CentralChannel = pPort->Channel;
                //pPort->CentralChannel = pPort->Channel;
                pPort->CommonCfg.DesiredHtPhy.HtChannelWidth = BW_20;
                DBGPRINT(RT_DEBUG_TRACE,("%s(): BW40 is NOT allowed in G band,Set pPort->CentralChannel = pPort->Channel;\n",__FUNCTION__));
                DBGPRINT(RT_DEBUG_TRACE,("%s(): SET pPort->CommonCfg.DesiredHtPhy.HtChannelWidth = BW_20\n",__FUNCTION__));
            }
    
        }
        else // 5G
        {
            // BW40 is NOT allowed in 5G band
            if(pAd->HwCfg.NicConfig2.field.bDisableBW40ForA)
            {
                pPort->CentralChannel = pPort->Channel;
                
                pPort->CommonCfg.DesiredHtPhy.HtChannelWidth = BW_20;
                DBGPRINT(RT_DEBUG_TRACE,("%s(): BW40 is NOT allowed in A band,Set pPort->CentralChannel = pPort->Channel;\n",__FUNCTION__));
                DBGPRINT(RT_DEBUG_TRACE,("%s(): SET pPort->CommonCfg.DesiredHtPhy.HtChannelWidth = BW_20\n",__FUNCTION__));
            }
        }
    }

    if(pBssidMacTabEntry != NULL)
    {
        DBGPRINT(RT_DEBUG_TRACE, ("!!! LINK UP !!(MaxRAmpduFactor=%d,)MpduDensity (CommonCfg=%d, StaActive=%d, )\n", 
        pBssidMacTabEntry->MaxRAmpduFactor,pPort->CommonCfg.SupportedHtPhy.MpduDensity, pAd->StaActive.SupportedHtPhy.MpduDensity));
    }
    // It's quite difficult to tell if a newly added KEY is WEP or CKIP until a new BSS
    // is formed (either ASSOC/RE-ASSOC done or IBSS started. LinkUP should be a safe place
    // to examine if cipher algorithm switching is required.
#ifdef MULTI_CHANNEL_SUPPORT
    DBGPRINT(RT_DEBUG_TRACE,("LinkUP(): pPort->SoftAP.bAPStart(%d), pPort->P2PCfg.bGOStart(%d)\n", pPort->SoftAP.bAPStart ? 1: 0, pPort->P2PCfg.bGOStart ? 1 : 0));
#endif /*MULTI_CHANNEL_SUPPORT*/
    if ((pPort->SoftAP.bAPStart == FALSE) && (pPort->P2PCfg.bGOStart == FALSE))
        MlmeAssocSwitchBetweenWepAndCkip(pAd, pPort);

    if (BssType == BSS_ADHOC)
    {
        OPSTATUS_SET_FLAG(pPort, fOP_STATUS_ADHOC_ON);
        OPSTATUS_CLEAR_FLAG(pPort, fOP_STATUS_INFRA_ON);
        
        // AdhocN Support:
        // IBSS Joiner might not have the correct central channel. 
        // Calculate it again according to the selected channel.
        if ((pAd->StaCfg.bAdhocNMode == TRUE) && (pPort->CommonCfg.PhyMode >= PHY_11ABGN_MIXED))
        {
            if(pPort->CommonCfg.PhyMode == PHY_11VHT)
            {
                // todo: set primary channel, bw, and central channel information
                DBGPRINT(RT_DEBUG_TRACE,("ADHOC is not support for VHT yet!!\n"));
            }

            if (pAd->StaCfg.AdhocJoiner == TRUE)
            {
                MlmeSyncCheckBWOffset(pAd, pPort->Channel);
                if (pPort->CommonCfg.HtCapability.HtCapInfo.ChannelWidth == BW_40)
                {
                    if (pPort->CommonCfg.DesiredHtPhy.ExtChanOffset == EXTCHA_ABOVE)
                    {
                        pPort->CentralChannel = pPort->Channel + 2;
                    }
                    else
                    {
                        if (pPort->Channel == 14)
                            pPort->CentralChannel = pPort->Channel - 1;
                        else
                            pPort->CentralChannel = pPort->Channel - 2;
                    }
                }
                else // BW_20
                    pPort->CentralChannel = pPort->Channel;
            }
            else // I am Creator.
            {   // The generation of CentralChannel is called by MlmeSyncStartReqAction()
            }

        }
    
        DBGPRINT(RT_DEBUG_TRACE, ("Adhoc MlmeCntLinkUp - bAdhocNMode=%d, AdhocMode=%d, MlmeAux.ChannelWidth = %d , ChannelWidth=%d, AdhocCreator=%d, AdhocJoiner=%d, CentralChannel=%d, Channel=%d\n", 
            pAd->StaCfg.bAdhocNMode,
            pAd->StaCfg.AdhocMode,
            pAd->MlmeAux.HtCapability.HtCapInfo.ChannelWidth,
            pPort->CommonCfg.HtCapability.HtCapInfo.ChannelWidth,
            pAd->StaCfg.AdhocCreator,
            pAd->StaCfg.AdhocJoiner,
            pPort->CentralChannel,
            pPort->Channel));
        
        // Change channel to BBP
        if(pPort->CommonCfg.PhyMode == PHY_11VHT)
            {
            // todo: set primary channel, bw, and central channel information
            DBGPRINT(RT_DEBUG_TRACE,("ADHOC is not support for VHT yet!!\n"));
            }

        if ((pAd->StaCfg.bAdhocNMode == TRUE) && (pPort->CommonCfg.PhyMode >= PHY_11ABGN_MIXED) &&
            (((pAd->StaCfg.AdhocCreator == TRUE) && (pAd->MlmeAux.HtCapability.HtCapInfo.ChannelWidth == BW_40)) ||
            ((pAd->StaCfg.AdhocJoiner == TRUE) && (pPort->CommonCfg.HtCapability.HtCapInfo.ChannelWidth == BW_40))) &&
            (pPort->CentralChannel != pPort->Channel))
        {   
            // Must using 40MHz.
            pPort->BBPCurrentBW = BW_40;
            SwitchBandwidth(pAd, TRUE, pPort->CentralChannel, BW_40, (pPort->CentralChannel > pPort->Channel)? EXTCHA_ABOVE:EXTCHA_BELOW);
            

            DBGPRINT(RT_DEBUG_TRACE, ("!!!40MHz Adhoc LINK UP !!! Control Channel = %d. Central = %d \n", pPort->Channel, pPort->CentralChannel ));
        }
        else
        {
            pPort->BBPCurrentBW = BW_20;
            SwitchBandwidth(pAd, TRUE, pPort->Channel, BW_20, EXTCHA_NONE);

            // Patch BW=40MHz with the same control and central channel.
            // Reset ChannelWidth info carried in beacon Ie
            if ((pPort->CommonCfg.HtCapability.HtCapInfo.ChannelWidth == BW_40) &&
                (pPort->CentralChannel == pPort->Channel))
            {
                pPort->CommonCfg.HtCapability.HtCapInfo.ChannelWidth = BW_20;
                pPort->CommonCfg.AddHTInfo.AddHtInfo.RecomWidth = BW_20;
                pPort->CommonCfg.AddHTInfo.AddHtInfo.ExtChanOffset = EXTCHA_NONE;
                DBGPRINT(RT_DEBUG_WARN, ("The same control channel and central channel --> Roll back to BW = 20MHz  \n" ));
            }

            DBGPRINT(RT_DEBUG_TRACE, ("!!!20MHz Adhoc LINK UP !!! \n" ));

        }

        // No carrier detection when adhoc
        AsicSetCarrierDetectAction(pAd, CARRIER_ACTION_STOP, 0);
        pPort->CommonCfg.bCarrierDetectPhase = FALSE;
        pPort->CommonCfg.bCarrierAssert = FALSE;
        

        if ((pAd->StaCfg.bAdhocNMode == TRUE) && (pPort->CommonCfg.PhyMode >= PHY_11ABGN_MIXED))
        {
            // Turn on QOs if use HT rate.
            if (pPort->CommonCfg.APEdcaParm.bValid == FALSE)
            {
                pPort->CommonCfg.APEdcaParm.bValid = TRUE;
                pPort->CommonCfg.APEdcaParm.Aifsn[0] = 3;
                pPort->CommonCfg.APEdcaParm.Aifsn[1] = 7;
                pPort->CommonCfg.APEdcaParm.Aifsn[2] = 1;
                pPort->CommonCfg.APEdcaParm.Aifsn[3] = 1;

                pPort->CommonCfg.APEdcaParm.Cwmin[0] = 4;
                pPort->CommonCfg.APEdcaParm.Cwmin[1] = 4;
                pPort->CommonCfg.APEdcaParm.Cwmin[2] = 3;
                pPort->CommonCfg.APEdcaParm.Cwmin[3] = 2;

                pPort->CommonCfg.APEdcaParm.Cwmax[0] = 10;
                pPort->CommonCfg.APEdcaParm.Cwmax[1] = 6;
                pPort->CommonCfg.APEdcaParm.Cwmax[2] = 4;
                pPort->CommonCfg.APEdcaParm.Cwmax[3] = 3;

                pPort->CommonCfg.APEdcaParm.Txop[0]  = 0;
                pPort->CommonCfg.APEdcaParm.Txop[1]  = 0;
                pPort->CommonCfg.APEdcaParm.Txop[2]  = 96;
                pPort->CommonCfg.APEdcaParm.Txop[3]  = 48;
            }
        }
        else
        {
            pPort->CommonCfg.APEdcaParm.bValid = FALSE;
        }

    }
    else // BssType != BSS_ADHOC
    {
        OPSTATUS_SET_FLAG(pPort, fOP_STATUS_INFRA_ON);
        OPSTATUS_CLEAR_FLAG(pPort, fOP_STATUS_ADHOC_ON);

        {
            // WiFi n 5.2.16 (PMKIDCache): 
            // Make sure the first message of 4way from BrcmAP has been received b4 SwitchBandwidth()
            // M1 from BrcmAP arrivals at sta after AssocRsp as 10~30ms.
            // M1 from RalonkAP arrivals at sta after AssocRsp as 200~300ms.
            // 7610U needs 70 ms for bandwidth switch, 7601 needs 35 ms.
            // The solution is given 15 ms delay for USB. (temp) 
            // If FW can switch bw to shorten the operation time in the future, it can remove this delay.
            UCHAR           PMKID_SSID[] = "Pcache";

            if ((pAd->StaCfg.PMKIDCount > 0) &&         
                (pPort->PortCfg.AuthMode == Ralink802_11AuthModeWPA2) &&
                (SSID_EQUAL(pAd->MlmeAux.Ssid, pAd->MlmeAux.SsidLen, PMKID_SSID, 6)) &&
                (pAd->MlmeAux.bVhtCapable == FALSE) &&
                (pPort->Channel <= 14)) // 2.4G
            {               
                // Search chched PMKID
                for (idx = 0; idx < pAd->StaCfg.PMKIDCount; idx++)
                {
                    if (PlatformEqualMemory(pAd->MlmeAux.Bssid, &pAd->StaCfg.PMKIDList[idx].BSSID, 6))
                    {
                        // Found a cached PMKID for the desired AP.
                        DBGPRINT(RT_DEBUG_TRACE,("%s: Use PMKID. Give 15 ms delay for rx 1'st message of 4 way\n", __FUNCTION__));
                        NdisCommonGenericDelay(15);

                        break;
                    }
                }
            }
        }


        // Switch to AP channel
        if (pAd->MlmeAux.bVhtCapable)
        {
            //UCHAR PrimaryChannelLocation;
            UCHAR Bandwidth = BW_20;
            UCHAR CentralChannel = 0;
            UCHAR ExtOffset = 0;

            if(pAd->MlmeAux.VhtOperation.VhtOpInfo.ChannelWidth == VHT_BW_80)
            {           
                Bandwidth = BW_80;
                CentralChannel = pAd->MlmeAux.VhtOperation.VhtOpInfo.ChannelCenterFreqSeg1;
                
                GetPrimaryChannelLocation(
                    pAd,
                    pPort->Channel,
                    Bandwidth,
                    pAd->MlmeAux.VhtOperation.VhtOpInfo.ChannelCenterFreqSeg1,
                    &ExtOffset
                    );
            }
            else if (pAd->MlmeAux.VhtOperation.VhtOpInfo.ChannelWidth == VHT_BW_20_40)
            {
                Bandwidth = (UCHAR)pAd->MlmeAux.HtCapability.HtCapInfo.ChannelWidth;                
                CentralChannel = pPort->CentralChannel;
                ExtOffset = pAd->MlmeAux.AddHtInfo.AddHtInfo.ExtChanOffset;
            }
            else
                DBGPRINT(RT_DEBUG_ERROR,("ERROR!! unsupported VhtOpInfo.ChannelWidth = %d\n",pAd->MlmeAux.VhtOperation.VhtOpInfo.ChannelWidth));

            DBGPRINT(RT_DEBUG_TRACE,("%s, LINE_%d, Bandwidth = %d\n",__FUNCTION__,__LINE__,Bandwidth));
            
            //
            // Update per-entry bandwidth control based on the HT IE
            //

            if(pWcidMacTabEntry != NULL)
            {
                WRITE_PHY_CFG_BW(pAd, &pWcidMacTabEntry->TxPhyCfg, Bandwidth);

                if (VHT_CAPABLE(pAd) && 
                    (pAd->LogoTestCfg.SigmaCfg.bAddOperatingModeNotificationInAssociationRequest == TRUE))
                {
                    pWcidMacTabEntry->VhtPeerStaCtrl.MaxBW = CH_WIDTH_BW20;
                    pWcidMacTabEntry->VhtPeerStaCtrl.MaxNss = RX_NSS_1;
                }
                else
                {
                    pWcidMacTabEntry->VhtPeerStaCtrl.MaxBW = Bandwidth;
                    pWcidMacTabEntry->VhtPeerStaCtrl.MaxNss = NSS_ALL;
                }           
            }

            pPort->BBPCurrentBW = Bandwidth;
            pPort->CentralChannel = CentralChannel;

            DBGPRINT(RT_DEBUG_TRACE,("%s, LINE_%d, channel switching to Primary channel = %d, bandwidth = %d, Central channel = %d, PrimaryLocation = %d\n",
                    __FUNCTION__,
                    __LINE__,
                    pPort->Channel,
                    Bandwidth,
                    CentralChannel,
                    ExtOffset
                    ));
            
            SwitchBandwidth(
                    pAd, 
                    TRUE, 
                    CentralChannel,
                    Bandwidth,  // bandwidth
                    ExtOffset // promary channel location
                    );
        }
        else if (pAd->MlmeAux.HtCapability.HtCapInfo.ChannelWidth == BW_40)
        // Turn on BBP 20MHz mode by request here.
        {
            if ((pAd->StaCfg.Feature11n&0x3) != 0x03) // 0x3 means we have switch to HT40
            {
                pPort->BBPCurrentBW = BW_40;
                DBGPRINT(RT_DEBUG_TRACE, ("1 !!!40MHz LINK UP !!! Control Channel = %d, Central Channel = %d \n", pPort->Channel, pPort->CentralChannel));
            }
            DBGPRINT(RT_DEBUG_TRACE, ("2 !!!40MHz LINK UP !!! Control Channel = %d, Central Channel = %d \n", pPort->Channel, pPort->CentralChannel));
        }
        else
        {
            DBGPRINT(RT_DEBUG_TRACE, ("%s - MultiChSts : 20Mhz, Assign pPort->Channel(%d), CentralChannel(%d) to pPort->ScaningChannel(%d)\n", __FUNCTION__, pPort->Channel, pPort->CentralChannel, pPort->ScaningChannel));

            pPort->BBPCurrentBW = BW_20;
            if (pPort->CentralChannel != pPort->Channel)
            {
                pPort->CentralChannel = pPort->Channel;
                //AsicSwitchChannel(pAd, pPort->Channel,FALSE);
            }

            DBGPRINT(RT_DEBUG_TRACE, ("!!!20MHz LINK UP !!! \n" ));
        }

        AsicSwitchChannel(pPort, pPort->Channel, pPort->CentralChannel, pPort->BBPCurrentBW, FALSE);

        DBGPRINT(RT_DEBUG_TRACE, ("%s: Feature11n = 0x%0x, reset bit1\n",__FUNCTION__, pAd->StaCfg.Feature11n));
        pAd->StaCfg.Feature11n &= (~0x2);
    }

    pWcidMacTabEntry->ValidAsCLI = TRUE;


    // For WHCK DPC ISR Test
    if ((pAd->LogoTestCfg.OnTestingWHQL == TRUE) && (pPort->PortType == EXTSTA_PORT) )
    {
        UCHAR       PortIdx;
        BOOLEAN     DoDPCISRTest = TRUE;
        
        for (PortIdx = 0; PortIdx < RTMP_MAX_NUMBER_OF_PORT; PortIdx++)
        {
            if ((pAd->PortList[PortIdx] == NULL) || (pAd->PortList[PortIdx]->bActive == FALSE))
                continue;

            if ( (pAd->PortList[PortIdx]->PortType == WFD_GO_PORT) || (pAd->PortList[PortIdx]->PortType == WFD_CLIENT_PORT) )
            {
                DoDPCISRTest = FALSE;
                break;
            }
        }

        if (DoDPCISRTest == TRUE)
        {
            PlatformCancelTimer(&pPort->Mlme.WHCKCPUUtilCheckTimer, &Cancelled);
            PlatformSetTimer(pPort, &pPort->Mlme.WHCKCPUUtilCheckTimer, 35000);
            pAd->LogoTestCfg.WhckDataCntForPast40Sec = 0;
            pAd->LogoTestCfg.WhckDataCntForPast60Sec = 0;
            pAd->LogoTestCfg.WhckFirst40Secs = TRUE;
            pAd->LogoTestCfg.WhckCpuUtilTestRunning = FALSE;
        }
    }

    /**************************************************************************/
    
    /**************************************************************************/
    
    MtAsicSetSlotTime(pAd, TRUE, pPort->Channel); //FALSE);
    MtAsicSetEdcaParm(pAd, pPort, &pPort->CommonCfg.APEdcaParm, TRUE);

    // Call this for RTS protection for legacy rate, we will always enable RTS threshold, but normally it will not hit
    MtAsicUpdateProtect(pAd, 0, (ALLN_SETPROTECT|CCKSETPROTECT|OFDMSETPROTECT), TRUE, FALSE);
    
    DBGPRINT(RT_DEBUG_TRACE, ("!!! %s !!! (Infra=%d, AID=%d, ssid=%s, Channel=%d, CentralChannel = %d) (Density =%d, )n", __FUNCTION__,
        BssType, pAd->StaActive.Aid, pPort->PortCfg.Ssid, pPort->Channel, pPort->CentralChannel, pWcidMacTabEntry->MpduDensity));
    
    // For WHCK DPC ISR Test
    if ((pAd->LogoTestCfg.OnTestingWHQL == TRUE) && (pPort->PortType == EXTSTA_PORT) )
    {
        PlatformCancelTimer(&pPort->Mlme.WHCKCPUUtilCheckTimer, &Cancelled);
        PlatformSetTimer(pPort, &pPort->Mlme.WHCKCPUUtilCheckTimer, 60000);
        pAd->LogoTestCfg.WhckDataCntForPast40Sec = 0;
        pAd->LogoTestCfg.WhckDataCntForPast60Sec = 0;
        pAd->LogoTestCfg.WhckFirst40Secs = TRUE;
        pAd->LogoTestCfg.WhckCpuUtilTestRunning = FALSE;
    }

    
    //MlmeUpdateTxRates(pAd,pPort, TRUE);
    //MlmeUpdateHtVhtTxRates(pAd, pPort, Wcid);
    
    PlatformZeroMemory(&pAd->Counter.DrsCounters, sizeof(COUNTER_DRS));

    NdisGetCurrentSystemTime((PLARGE_INTEGER)&Now64);

    // Last RX Timestamp ==>
    // If p2p concurrent client has connected, save LastBeaconRxTime into its own wcid
    if (IS_P2P_MS_CLI_WCID(pPort, Wcid) && pWcidMacTabEntry !=NULL)
        pWcidMacTabEntry->LastBeaconRxTime = Now64;
    else
        pAd->StaCfg.LastBeaconRxTime = Now64;
    
    if ((pPort->CommonCfg.TxPreamble != Rt802_11PreambleLong) &&
        CAP_IS_SHORT_PREAMBLE_ON(pAd->StaActive.CapabilityInfo))
    {

        DBGPRINT(RT_DEBUG_INFO, ("CNTL - !!! Set to short preamble!!!\n"));
        MlmeSetTxPreamble(pAd, Rt802_11PreambleShort);
    }

    if (pPort->CommonCfg.RadarDetect.RDMode == RD_SILENCE_MODE)
    {
        RadarDetectionStop(pAd);
    }
    pPort->CommonCfg.RadarDetect.RDMode = RD_NORMAL_MODE;

    if (BssType == BSS_ADHOC)
    {
        //
        // We also need to cancel the MlmeCntLinkDownTimer, no matter it was been set or not.
        // It may be set when we start an Infrastructure mode.
        // And not be canceled yet then we switch to Adohc at meanwhile.
        //
        PlatformCancelTimer(&pPort->Mlme.MlmeCntLinkDownTimer, &Cancelled);

        OPSTATUS_CLEAR_FLAG(pPort, fOP_STATUS_AGGREGATION_INUSED);

        // AdhocN Support:
        // IBSS Joiner will not have correct ControlChannel when calling RTMPSetHT          
        if(pPort->CommonCfg.PhyMode == PHY_11VHT)
        {
            // todo: set primary channel, bw, and central channel information
            DBGPRINT(RT_DEBUG_TRACE,("ADHOC is not support for VHT yet!!\n"));
        }
        else if ((pAd->StaCfg.bAdhocNMode) && (pPort->CommonCfg.PhyMode >= PHY_11ABGN_MIXED))
        {
            pPort->CommonCfg.AddHTInfo.ControlChan = pPort->Channel; 
        }
        else // Legacy mode
        {
            // When the STA is at the legacy mode and the peer STA is at HT mode, 
            // the Ralink UI would show the channel and the central channel at the same time.
            // The channel and the central channel should be the same.
            pPort->CentralChannel = pPort->Channel;
        }

        MgntPktConstructIBSSBeaconFrame(pAd,pPort);

        //We won't check the phy mode, for DFS, the correct way is check the Radar channel. 
        //if (((pPort->CommonCfg.PhyMode == PHY_11A) || (pPort->CommonCfg.PhyMode == PHY_11ABG_MIXED))&& (pPort->CommonCfg.bIEEE80211H == 1) && RadarChannelCheck(pAd, pPort->Channel))
        if ((pPort->CommonCfg.bIEEE80211H == 1) && RadarChannelCheck(pAd, pPort->Channel))
        {
            ; //Do nothing
        }
        else
        {
            MtAsicEnableIbssSync(pAd);
        }
        
        //restore tx group key for WPA2+AES after waking up
        if (pPort->CommonCfg.bRestoreKey)
        {
            pPort->CommonCfg.bRestoreKey = FALSE;

            if (pPort->PortCfg.AuthMode == Ralink802_11AuthModeWPA2PSK)
            {
                UCHAR WlanIdx = MlmeSyncGetWlanIdxByPort(pPort, ROLE_WLANIDX_ADHOC_MCAST);
                
                pPort->SharedKey[BSS0][pPort->PortCfg.DefaultKeyId].TxTsc[0] = 0;
                pPort->SharedKey[BSS0][pPort->PortCfg.DefaultKeyId].TxTsc[1] = 0;
                pPort->SharedKey[BSS0][pPort->PortCfg.DefaultKeyId].TxTsc[2] = 0;
                pPort->SharedKey[BSS0][pPort->PortCfg.DefaultKeyId].TxTsc[3] = 0;
                pPort->SharedKey[BSS0][pPort->PortCfg.DefaultKeyId].TxTsc[4] = 0;
                pPort->SharedKey[BSS0][pPort->PortCfg.DefaultKeyId].TxTsc[5] = 0;
                DBGPRINT(RT_DEBUG_TRACE, ("adhoc, restore tx group key...\n"));
                AsicAddKeyEntry(pAd,
                    pPort,
                    WlanIdx,
                    BSS0,
                    pPort->PortCfg.DefaultKeyId,
                    &pPort->SharedKey[BSS0][pPort->PortCfg.DefaultKeyId],
                    TRUE,
                    TRUE);
            }
            else
            {
                DBGPRINT(RT_DEBUG_TRACE, ("adhoc, restore wep key...\n"));
                DBGPRINT(RT_DEBUG_ERROR, ("addkey, key index = %d, key = %02x, %02x, %02x\n",
                    pPort->PortCfg.DefaultKeyId,
                    pPort->SharedKey[BSS0][pPort->PortCfg.DefaultKeyId].Key[0],
                    pPort->SharedKey[BSS0][pPort->PortCfg.DefaultKeyId].Key[1],
                    pPort->SharedKey[BSS0][pPort->PortCfg.DefaultKeyId].Key[2]
                    ));

                MtAsicUpdateRxWCIDTable(pAd, Wcid, pPort->PortCfg.Bssid);

                AsicAddKeyEntry(pAd, pPort, Wcid, BSS0, (UCHAR)pPort->PortCfg.DefaultKeyId, 
                    &pPort->SharedKey[BSS0][pPort->PortCfg.DefaultKeyId], FALSE, TRUE);
            }
        }
#ifdef  SINGLE_ADHOC_LINKUP
        // Although this did not follow microsoft's recommendation.
        //Change based on customer's request
        DBGPRINT(RT_DEBUG_TRACE, ("MlmeCntLinkUp - !!!SINGLE_ADHOC_LINKUP!!!\n"));
        OPSTATUS_SET_FLAG(pPort, fOP_STATUS_MEDIA_STATE_CONNECTED); 
#endif

    }
    else // BSS_INFRA
    {
        // First cancel linkdown timer
        PlatformCancelTimer(&pPort->Mlme.MlmeCntLinkDownTimer, &Cancelled);
        // Check the new SSID with last SSID
        while (Cancelled == TRUE)
        {
            if (pPort->PortCfg.LastSsidLen == pPort->PortCfg.SsidLen)
            {
                if (MtkCompareMemory(pPort->PortCfg.LastSsid, pPort->PortCfg.Ssid, pPort->PortCfg.LastSsidLen) == 0)
                {
                    // Link to the old one no linkdown is required.
                    break;
                }
            }
            // Send link down event before set to link up
            DBGPRINT(RT_DEBUG_TRACE, ("NDIS_STATUS_MEDIA_DISCONNECT Event AA!\n"));
            break;
        }

#if 0
        //if resume from sleep ,we need to retore wep key
        if ((pPort->CommonCfg.bRestoreKey) && (IS_WEP_STATUS_ON(pPort->PortCfg.WepStatus)))
        {
            pPort->CommonCfg.bRestoreKey = FALSE; 
            if (pAd->NumberOfPorts > 1)
            {
                BssIndex = Ndis6CommonGetBssidIndex(pAd, pPort, MULTI_BSSID_MODE);
            }
            AsicAddKeyEntry(pAd,pPort, Wcid, BssIndex, (UCHAR)pPort->PortCfg.DefaultKeyId, 
                &pPort->SharedKey[BSS0][pPort->PortCfg.DefaultKeyId], FALSE, TRUE);
            MtAsicUpdateRxWCIDTable(pAd, Wcid, pPort->PortCfg.Bssid);
        }
#endif

        DBGPRINT(RT_DEBUG_TRACE, ("Link up:  Wcid = %d, pPort->PortCfg.Bssid = %02x %02x %02x %02x %02x %02x", Wcid, pPort->PortCfg.Bssid[0], pPort->PortCfg.Bssid[1], pPort->PortCfg.Bssid[2], pPort->PortCfg.Bssid[3], pPort->PortCfg.Bssid[4], pPort->PortCfg.Bssid[5]));
        MtAsicUpdateRxWCIDTable(pAd, Wcid, pPort->PortCfg.Bssid);     

        if (IS_WEP_STATUS_ON(pPort->PortCfg.WepStatus))
        {
            DBGPRINT(RT_DEBUG_TRACE, ("re-set WEP key\n"));
            if (pAd->NumberOfPorts > 1)
            {
                BssIndex = Ndis6CommonGetBssidIndex(pAd, pPort, MULTI_BSSID_MODE);
            }
            
            AsicAddKeyEntry(pAd,pPort, Wcid, BssIndex, (UCHAR)pPort->PortCfg.DefaultKeyId, &pPort->SharedKey[BSS0][pPort->PortCfg.DefaultKeyId], FALSE, TRUE);
        }

        MlmeUpdateHtVhtTxRates(pAd, pPort, (UCHAR)Wcid);


        NdisCommonMappingPortToBssIdx(pPort, DO_NOTHING, UPDATE_MAC);
        
        // NOTE:
        // the decision of using "short slot time" or not may change dynamically due to
        // new STA association to the AP. so we have to decide that upon parsing BEACON, not here

        // NOTE:
        // the decision to use "RTC/CTS" or "CTS-to-self" protection or not may change dynamically
        // due to new STA association to the AP. so we have to decide that upon parsing BEACON, not here

        MlmeCntComposePsPoll(pAd,pPort);
        MlmeCntComposeNullFrame(pAd,pPort);

        //do not set bss sync when ap already start
        if ((pAd->PortList[pPort->P2PCfg.PortNumber]->PortSubtype == PORTSUBTYPE_P2PGO) && (pAd->OpMode == OPMODE_STAP2P))
        {
            // GO already set related Bss/Bssid. doesn't need to change.
        }
        else
        {
            //do not set bss sync when ap already start
            if (pAd->HwCfg.BeaconNum == 0)
            {
                MtAsicEnableBssSync(pPort->pAd, pPort->CommonCfg.BeaconPeriod);
            }
        }
        
        // only INFRASTRUCTURE mode need to indicate connectivity immediately; ADHOC mode
        // should wait until at least 2 active nodes in this BSSID.
        OPSTATUS_SET_FLAG(pPort, fOP_STATUS_MEDIA_STATE_CONNECTED); 
        pPort->StaCfg.WscControl.bWPSSuccessandLinkup = TRUE; 
        
        if (pAd->MlmeAux.bStaCanRoam)
        {
            PlatformIndicateRoamingCompletion(pAd,pPort, DOT11_ASSOC_STATUS_SUCCESS);
        }
        else
        {
            PlatformIndicateConnectionCompletion(pAd, pPort, DOT11_CONNECTION_STATUS_SUCCESS);
            pAd->MlmeAux.bStaCanRoam = TRUE;
            pAd->MlmeAux.bNeedIndicateConnectStatus = FALSE;
        }
        DBGPRINT(RT_DEBUG_TRACE, ("TONDIS:  Info Link UP, PlatformIndicateConnectionCompletion  [%02x:%02x:%02x:%02x:%02x:%02x]\n",
                    pPort->PortCfg.Bssid[0], pPort->PortCfg.Bssid[1], pPort->PortCfg.Bssid[2], 
                    pPort->PortCfg.Bssid[3], pPort->PortCfg.Bssid[4], pPort->PortCfg.Bssid[5]));
        
        if (pPort->StaCfg.WscControl.bWPSSession == FALSE)
        {
            int i;
            BOOLEAN WepStatusEqualAP = FALSE;
            for (i = 0; i < pAd->ScanTab.BssNr; i++) 
            {
                BSS_ENTRY *pInBss = &pAd->ScanTab.BssEntry[i];
                // pick up the first available vacancy
                if ((MAC_ADDR_EQUAL(pInBss->Bssid, pPort->PortCfg.Bssid)) && (pInBss->WepStatus == pPort->PortCfg.WepStatus))
                {
                    WepStatusEqualAP = TRUE;
                    break;
                }
            }
            if ((pAd->HwCfg.LedCntl.field.LedMode & LED_MODE) == WPS_LED_MODE_9) // LED mode 9.
            {           
                // The AP uses OPEN-WEP.
                if ((pPort->PortCfg.AuthMode == Ralink802_11AuthModeOpen) && (IS_WEP_STATUS_ON(pPort->PortCfg.WepStatus)) && (pPort->PortCfg.ExcludeUnencrypted) && WepStatusEqualAP)
                {
                    LedCtrlSetLed(pAd, LED_NORMAL_CONNECTION_WITH_SECURITY);
                    DBGPRINT(RT_DEBUG_TRACE, ("%s: LED_NORMAL_CONNECTION_WITH_SECURITY\n", __FUNCTION__));
                }
                //MlmeInfoAddKey will turn on LED if using other security.
            }
        
        }


#if 0
        //
        // Patch some AP on WPAPSK & WPA2PSK mode that they send first 4Way EAPOL frame before station indicate connect event to OS.
        // In this case, we are on disconnect state so that 802.1x protocol driver will dropt this packet.
        // But if AP didn't retransmit first 4way packet, we will not connect to this AP anymore.
        // For example: Linksys model: BEFW11S4 Ver.4 it send first 4way packet very fast within 2 ms.
        //
        pAd->pRxCfg->bQueueEAPOL = FALSE;
        if (pAd->pRxCfg->RxEAPOLLen != 0)
        {
            pAd->pRxCfg->pRxData = pAd->pRxCfg->pRxEAPOL + LENGTH_802_3;
            pAd->pRxCfg->RxEAPOLLen = 0;
            //pAd->Counter.Counters8023.GoodReceives++;
            DBGPRINT(RT_DEBUG_TRACE, ("Indicate previous queued EAPOL frame\n"));
        }   
#endif
        //when doing fast roamming ,never call link down ,so we have to remove last connected  bssid
        //pEntry = MacTableLookup(pPort, pPort, pPort->CommonCfg.LastBssid);
        //if (pEntry)
        //  MacTableDeleteAndResetEntry(pPort,pEntry->Aid,pPort->CommonCfg.LastBssid );
        // Add BSSID in my MAC Table.
#if 0   
        //We will do this in MlmeCntlWaitAssocProc
        MacTableInsertEntry(pPort, pPort->CommonCfg.Bssid, FALSE, FALSE);
#endif

        if(pWcidMacTabEntry == NULL)
            return;
               //NdisAcquireSpinLock(&pAd->MacTablePool.MacTabPoolLock);
        PlatformAcquireOldRWLockForWrite(&pAd->MacTablePool.MacTabPoolLock,&LockState);
        PlatformMoveMemory(pWcidMacTabEntry->Addr, pPort->PortCfg.Bssid, MAC_ADDR_LEN);
        PlatformMoveMemory(pWcidMacTabEntry->PairwiseKey.BssId, pPort->PortCfg.Bssid, MAC_ADDR_LEN);
        pWcidMacTabEntry->Aid = Wcid;
        pWcidMacTabEntry->pAd = pAd;
        //pPort->MacTab.Content[BSSID_WCID].pPort = pPort;
        pWcidMacTabEntry->ValidAsCLI = TRUE;    //Although this is bssid..still set ValidAsCl
        pWcidMacTabEntry->Sst = SST_ASSOC;
        pWcidMacTabEntry->AuthState = SST_ASSOC;
        pWcidMacTabEntry->BaSizeInUse = (UCHAR)pPort->CommonCfg.BACapability.field.TxBAWinLimit;
        // add this BSSID entry into HASH table
        //NdisReleaseSpinLock(&pAd->MacTablePool.MacTabPoolLock);
        PlatformReleaseOldRWLock(&pAd->MacTablePool.MacTabPoolLock,&LockState);
        // For emulation, add BA session.       
        if ((pPort->CommonCfg.BACapability.field.AutoBA == TRUE) && (pAd->StaActive.SupportedHtPhy.bHtEnable == TRUE))
        {               
            for (idx = 0; idx <NUM_OF_TID; idx++)
            {               
                // If add myself as one Recipient of peer (pPort->PortCfg.Bssid), the sequence 0xffff indicates the BA
                // session not started yet. 
                BATableInsertEntry(pAd, pPort, Wcid,  0, 0x1, idx, pWcidMacTabEntry->BaSizeInUse,  Originator_SetAutoAdd, FALSE);
                pWcidMacTabEntry->TXAutoBAbitmap |= (1<<idx);
            }           
        }
        DBGPRINT(RT_DEBUG_TRACE, ("ClientStatusFlags = %x!\n", pWcidMacTabEntry->ClientStatusFlags));

        // If another connected port is non-piggyback, don't open piggyback.
        if (((pAd->MlmeAux.APRalinkIe & 0x00000002) == 2) && (pPort->CommonCfg.bPiggyBackCapable))
        {
            pWcidMacTabEntry = MlmeSyncMacTabMatchedWlanIdx(pPort, CheckPortWcid); 
            if(pWcidMacTabEntry == NULL)
            {
                DBGPRINT(RT_DEBUG_TRACE,("%s(), Line:%d, Mac Entry is NULL  Wcid = %d\n", __FUNCTION__, __LINE__, CheckPortWcid));
                return;
            }
            
            if ((pCheckPort) && (INFRA_ON(pCheckPort)) && (!CLIENT_STATUS_TEST_FLAG(pWcidMacTabEntry, fCLIENT_STATUS_RALINK_CHIPSET)))
                bCoexistPiggyBack = FALSE;
            if ((pCheckPort2) && (INFRA_ON(pCheckPort2)) && (!CLIENT_STATUS_TEST_FLAG(pWcidMacTabEntry, fCLIENT_STATUS_RALINK_CHIPSET)))
                bCoexistPiggyBack = FALSE;

            if (bCoexistPiggyBack == TRUE)
            {
                CLIENT_STATUS_SET_FLAG(pWcidMacTabEntry, fCLIENT_STATUS_RALINK_CHIPSET);
                AsicEnablePiggyB(pAd);
                DBGPRINT(RT_DEBUG_TRACE, ("Turn on Piggy-Back\n"));
            }   
        }

        // If another connected port is non-aggregation, don't open aggregation.
        if ((pCheckPort) && (INFRA_ON(pCheckPort)) && (!OPSTATUS_TEST_FLAG(pAd, fOP_STATUS_AGGREGATION_INUSED)))
            bCoexistAggregation = FALSE;
        if ((pCheckPort2) && (INFRA_ON(pCheckPort2)) && (!OPSTATUS_TEST_FLAG(pAd, fOP_STATUS_AGGREGATION_INUSED)))
            bCoexistAggregation = FALSE;
        
        if ((pAd->MlmeAux.APRalinkIe & 0x00000001) && (pPort->CommonCfg.bAggregationCapable) &&(bCoexistAggregation))
        {
            CLIENT_STATUS_CLEAR_FLAG(pWcidMacTabEntry, fCLIENT_STATUS_RALINK_CHIPSET);
            OPSTATUS_SET_FLAG(pPort, fOP_STATUS_AGGREGATION_INUSED);                        
        }
        if (pAd->MlmeAux.APRalinkIe != 0x0)
        {
            CLIENT_STATUS_SET_FLAG(pWcidMacTabEntry, fCLIENT_STATUS_RALINK_CHIPSET);
            DBGPRINT(RT_DEBUG_TRACE, ("--->Assoc with Ralink Chipset \n"));
        }
    }

    if ((pAd->StaActive.SupportedHtPhy.bHtEnable == TRUE))
    {

        // ===========>>>> RDG Feature
        if ((pAd->StaActive.SupportedHtPhy.bHtEnable == TRUE) &&
            (CLIENT_STATUS_TEST_FLAG(MlmeSyncMacTabMatchedWlanIdx(pPort, Wcid), fCLIENT_STATUS_RDG_CAPABLE)))
        {
            if((pCheckPort) && INFRA_ON(pCheckPort) && (!CLIENT_STATUS_TEST_FLAG(MlmeSyncMacTabMatchedWlanIdx(pPort, CheckPortWcid), fCLIENT_STATUS_RDG_CAPABLE)))
            {
                bDisableRDG = TRUE;
            }
            if((pCheckPort2) && INFRA_ON(pCheckPort2) && (!CLIENT_STATUS_TEST_FLAG(MlmeSyncMacTabMatchedWlanIdx(pPort, CheckPort2Wcid), fCLIENT_STATUS_RDG_CAPABLE)))
            {
                bDisableRDG = TRUE;
            }
        }
       // else if (!CLIENT_STATUS_TEST_FLAG(MlmeSyncMacTabMatchedWlanIdx(pPort, Wcid), fCLIENT_STATUS_RDG_CAPABLE))
          //  bDisableRDG = TRUE;
#if 0
        if (bDisableRDG == TRUE)
        {
            DBGPRINT(RT_DEBUG_TRACE, ("!!!Disable RDG\n"));
            AsicDisableRDG(pAd, pPort);
            
            // If another connected port uses RDG, clear its status once disable RDG function.
            if ((pCheckPort) && INFRA_ON(pCheckPort))
                CLIENT_STATUS_CLEAR_FLAG(MlmeSyncMacTabMatchedWlanIdx(pPort, CheckPortWcid), fCLIENT_STATUS_RDG_CAPABLE);
            if ((pCheckPort2) && INFRA_ON(pCheckPort2))
                CLIENT_STATUS_CLEAR_FLAG(MlmeSyncMacTabMatchedWlanIdx(pPort, CheckPort2Wcid), fCLIENT_STATUS_RDG_CAPABLE);
        }
        else
        {
            DBGPRINT(RT_DEBUG_TRACE, ("!!!Enable RDG\n"));
            AsicEnableRDG(pAd);
        }
        //  <<<<===========
#endif
        if (pAd->MlmeAux.AddHtInfo.AddHtInfo2.NonGfPresent == 1)
        {
            MtAsicUpdateProtect(pAd, pAd->MlmeAux.AddHtInfo.AddHtInfo2.OperaionMode,  ALLN_SETPROTECT, FALSE, TRUE);
        }
        else
            MtAsicUpdateProtect(pAd, pAd->MlmeAux.AddHtInfo.AddHtInfo2.OperaionMode,  ALLN_SETPROTECT, FALSE, FALSE);
        DBGPRINT(RT_DEBUG_TRACE, ("AP set OperaionMode = %d,  NonGfPresent = %d\n", pAd->MlmeAux.AddHtInfo.AddHtInfo2.OperaionMode, pAd->MlmeAux.AddHtInfo.AddHtInfo2.NonGfPresent));
    }
    
    DBGPRINT(RT_DEBUG_TRACE, ("AP set OperaionMode = %d,  NonGfPresent = %d\n", pAd->MlmeAux.AddHtInfo.AddHtInfo2.OperaionMode, pAd->MlmeAux.AddHtInfo.AddHtInfo2.NonGfPresent));   

    // Set LED
    if(pPort->StaCfg.WscControl.bSkipWPSTurnOffLED == FALSE)
        LedCtrlSetLed(pAd, LED_LINK_UP);

    //
    // Enable OFDM TX rate auto fallback to CCK, if need.
    //
    /*RTUSBReadMACRegister(pPort, TXRX_CSR4, &CurTxRxCsr4.word);
    NewTxRxCsr4.word = CurTxRxCsr4.word;
    if ((pPort->Channel <= 14) && 
        ((pPort->CommonCfg.PhyMode == PHY_11B) ||
         (pPort->CommonCfg.PhyMode == PHY_11BG_MIXED) ||
         (pPort->CommonCfg.PhyMode == PHY_11ABG_MIXED)))
    {
        NewTxRxCsr4.field.OfdmTxFallbacktoCCK = 1;   //Enable OFDM TX rate auto fallback to CCK 1M, 2M
    }
    else
    {
        NewTxRxCsr4.field.OfdmTxFallbacktoCCK = 0;   //Disable OFDM TX rate auto fallback to CCK 1M, 2M 
    }
*/
    
    // Set initial tx rate
    if (pPort->CommonCfg.bAutoTxRateSwitch == TRUE)
    {
        PUCHAR                  pTable;
        UCHAR                   TableSize = 0;
        pEntry = pWcidMacTabEntry;

        MlmeSelectTxRateTable(pAd, pEntry, &pTable, &TableSize, &pWcidMacTabEntry->CurrTxRateIndex);
        AsicUpdateAutoFallBackTable(pAd, pTable, pEntry);
    }
    
    //if (pPort->CommonCfg.bAutoTxRateSwitch == FALSE)
    {
        pEntry = pWcidMacTabEntry;
        
        WRITE_PHY_CFG_MODE(pAd, &pEntry->TxPhyCfg, READ_PHY_CFG_MODE(pAd, &pPort->CommonCfg.TxPhyCfg));
        WRITE_PHY_CFG_MCS(pAd, &pEntry->TxPhyCfg, READ_PHY_CFG_MCS(pAd, &pPort->CommonCfg.TxPhyCfg));
        WRITE_PHY_CFG_BW(pAd, &pEntry->TxPhyCfg, READ_PHY_CFG_BW(pAd, &pPort->CommonCfg.TxPhyCfg));
        WRITE_PHY_CFG_STBC(pAd, &pEntry->TxPhyCfg, READ_PHY_CFG_STBC(pAd, &pPort->CommonCfg.TxPhyCfg));
        WRITE_PHY_CFG_SHORT_GI(pAd, &pEntry->TxPhyCfg, READ_PHY_CFG_SHORT_GI(pAd, &pPort->CommonCfg.TxPhyCfg));
        
        if (READ_PHY_CFG_MCS(pAd, &pEntry->TxPhyCfg) == MCS_32)
        {
            WRITE_PHY_CFG_SHORT_GI(pAd, &pEntry->TxPhyCfg, GI_800);
        }       

        if ((READ_PHY_CFG_STBC(pAd, &pEntry->TxPhyCfg) == STBC_USE) && 
             (pAd->HwCfg.Antenna.field.TxPath >= 2))
        {
            WRITE_PHY_CFG_STBC(pAd, &pEntry->TxPhyCfg, STBC_USE);
        }
        else
        {
            WRITE_PHY_CFG_STBC(pAd, &pEntry->TxPhyCfg, STBC_NONE);
        }

        if(VHT_CAPABLE(pAd))
        {
            if ((READ_PHY_CFG_NSS(pAd, &pEntry->TxPhyCfg) < NSS_1))
            {
                WRITE_PHY_CFG_STBC(pAd, &pEntry->TxPhyCfg, STBC_NONE);
            }
        }
        else
        if ((READ_PHY_CFG_MCS(pAd, &pEntry->TxPhyCfg) > MCS_7) || (READ_PHY_CFG_MCS(pAd, &pEntry->TxPhyCfg) == MCS_32))
        {
            WRITE_PHY_CFG_STBC(pAd, &pEntry->TxPhyCfg, STBC_NONE);
        }
    }
    //  Let Link Status Page display first initial rate.
    WRITE_PHY_CFG_DOUBLE_WORD(pAd, &pPort->LastTxRatePhyCfg, READ_PHY_CFG_DOUBLE_WORD(pAd, &pWcidMacTabEntry->TxPhyCfg));
    WRITE_PHY_CFG_DOUBLE_WORD(pAd, &pPort->LastRxRatePhyCfg, READ_PHY_CFG_DOUBLE_WORD(pAd, &pAd->UiCfg.LastMgmtRxPhyCfg));

    pAd->StaCfg.LastWcid = (UCHAR)Wcid;
    pAd->Mlme.PeriodicRound = 0;        // re-schedule MlmePeriodicExecTimerCallback()
    pAd->Mlme.OneSecPeriodicRound = 0;
    pAd->pNicCfg->bConfigChanged = FALSE;        // Reset config flag
    pAd->UiCfg.ExtraInfo = GENERAL_LINK_UP;   // Update extra information to link is up

    //Indicate link quality
    pPort->LastLinkQuality = 0;
    PlatformIndicateLinkQuality(pAd, pPort->PortNumber);

    pPort->LastIndicateRate = 0;
    //This avoid data rate display issue.
    PlatformIndicateNewLinkSpeed(pAd,pPort->PortNumber,OPMODE_STA);

    // Select DAC according to HT or Legacy
    if (pAd->StaActive.SupportedHtPhy.MCSSet[0] != 0x00)
    {
        BbSetTxDacCtrlByTxPath(pAd);
    }

    if ((pPort->CommonCfg.IOTestParm.bLastAtheros == TRUE))
    {
        if ((WEP_ON(pPort))||(TKIP_ON(pPort)))
        {
            pPort->CommonCfg.IOTestParm.bNextDisableRxBA = TRUE;
        }
    }
    else
    {
        pPort->CommonCfg.IOTestParm.bNextDisableRxBA = FALSE;
    }
    
    pPort->CommonCfg.IOTestParm.bLastAtheros = FALSE;
    COPY_MAC_ADDR(pPort->PortCfg.LastBssid, pPort->PortCfg.Bssid);
    DBGPRINT(RT_DEBUG_TRACE, ("!!!pPort->bNextDisableRxBA= %d \n", pPort->CommonCfg.IOTestParm.bNextDisableRxBA));

#if 0   // ToDo :  check later :Lens
    //
    // Patch Atheros AP TX will breakdown issue. (MAC version: 2860)
    // AP Model: DLink DWL-8200AP
    //
    if ((pAd->HwCfg.MACVersion & 0xffff0000) == 0x28600000)
    {
        if (INFRA_ON(pPort) && OPSTATUS_TEST_FLAG(pPort, fOP_STATUS_WMM_INUSED) && (pPort->PortCfg.WepStatus == Ralink802_11Encryption2Enabled))
        {
            RTUSBWriteMACRegister(pAd, RX_PARSER_CFG, 0x01);
        }
    
        else
        {
            RTUSBWriteMACRegister(pAd, RX_PARSER_CFG, 0x00);
        }
    }
#endif

    // WSC initial connect to AP, jump to Wsc start action and set the correct parameters
    if (pPort->StaCfg.WscControl.WscState == WSC_STATE_START)
    {
        pPort->StaCfg.WscControl.WscState = WSC_STATE_LINK_UP;
        pPort->StaCfg.WscControl.WscStatus = STATUS_WSC_LINK_UP;
        //kill WscEapM2Timer
        PlatformCancelTimer(&pPort->StaCfg.WscControl.WscEapM2Timer, &Cancelled);

        LedCtrlSetLed(pAd, LED_WPS);

        // Patch: When the Buffalo AP (Model: WHR-HP-AMPGV) receives the EAPOL-Start frame from the Enrollee,
        // it sometimes cannot send the EAP-Response/identity frame immediately.
        Delay_us(100000);
        WscStartAction(pAd, pPort);
    }   

    pPort->StaCfg.WscControl.bQueueEAPOL = FALSE;
    if (pPort->StaCfg.WscControl.RxEAPOLLen != 0)
    {       
        // Indicate the queued EAPOL packet after link up.
        REPORT_MGMT_FRAME_TO_MLME(pAd, 
                                    pPort, 
                                    READ_RXWI_WCID(pAd, &pPort->StaCfg.WscControl.RxWI),
                                    pPort->StaCfg.WscControl.pRxEAPOL,
                                    READ_RXWI_MPDU_TOTAL_BYTE_COUNT(pAd, &pPort->StaCfg.WscControl.RxWI),
                                    READ_RXWI_RSSI0(pAd, &pPort->StaCfg.WscControl.RxWI),
                                    READ_RXWI_RSSI1(pAd, &pPort->StaCfg.WscControl.RxWI),
                                    READ_RXWI_RSSI2(pAd, &pPort->StaCfg.WscControl.RxWI),
                                    READ_RXWI_AntSel(pAd, &pPort->StaCfg.WscControl.RxWI)
                                    );
        
        pPort->StaCfg.WscControl.RxEAPOLLen = 0;
        INC_COUNTER64(pAd->Counter.Counters8023.GoodReceives);
        DBGPRINT(RT_DEBUG_TRACE, ("%s: Indicate the queued EAP-Identify packet.\n", __FUNCTION__));
    }

#if 0   // ToDo : Check later : Lens
    //
    // Change retry
    //
    if (BssType == BSS_INFRA)
    {
        RTUSBReadMACRegister(pAd, TX_RTY_CFG, &TxRtyCfg.word);

        // shorten the retry limit for CCX voice test
        if ((pAd->StaCfg.CCXControl.field.RFRoamEnable) || (pAd->StaCfg.CCXControl.field.VoiceRate))
        {
            TxRtyCfg.field.LongRtyLimit = 0x03;
            TxRtyCfg.field.ShortRtyLimit = 0x03;
        }
        else
        {
            TxRtyCfg.field.LongRtyLimit = 0x7f;
            TxRtyCfg.field.ShortRtyLimit = 0x7f;
        }

        RTUSBWriteMACRegister(pAd, TX_RTY_CFG, TxRtyCfg.word);
        OPSTATUS_SET_FLAG(pAd, fOP_STATUS_LONG_RETRY);
    }

    //
    // Update RTS retry limit
    //
    RTUSBReadMACRegister(pAd, TX_RTS_CFG, &RtsCfg.word);
    if (pAd->StaCfg.CCXControl.field.VoiceRate)
        RtsCfg.field.AutoRtsRetryLimit = 0x0;
    else
        RtsCfg.field.AutoRtsRetryLimit = 0x20;
    if ((BssType == BSS_INFRA) && (pPort->CommonCfg.RtsThreshold < 2347))
        RtsCfg.field.RtsThres = pPort->CommonCfg.RtsThreshold;        
    RTUSBWriteMACRegister(pAd, TX_RTS_CFG, RtsCfg.word);
#endif

    // Check if the policy is to support 802.11nD4.0 2040Coexistence features
    // And this features requires scan in 2.4GHz only .
    // Support 2040Coexistence requirement from the first AP that requests the feature, disallow the second client use that.
    if ((pPort->CommonCfg.BACapability.field.b2040CoexistScanSup) && (pPort->Channel <= 14)
        && (pAd->StaActive.SupportedHtPhy.bHtEnable == TRUE) && (pAd->MlmeAux.ExtCapIE.field.BssCoexstSup == 1)
        && (!OPSTATUS_TEST_FLAG(pAd, fOP_STATUS_SCAN_2040)))
    {
        OPSTATUS_SET_FLAG(pAd, fOP_STATUS_SCAN_2040);
        OPSTATUS_SET_FLAG(pPort, fOP_STATUS_SCAN_2040);
        MlmeSyncBuildEffectedChannelList(pAd);
        DBGPRINT(RT_DEBUG_TRACE, ("LinkUP AP supports 20/40 BSS COEX(=%d) !!! \n", pAd->MlmeAux.ExtCapIE.field.BssCoexstSup));
    }
    
    if(pPort->SoftAP.bAPStart && (pAd->OpMode != OPMODE_STAP2P))
    {
        pAPPort = MlmeSyncGetApPort( pAd);
        if (pAPPort!= NULL)
        {
            if(pPort->SoftAP.ApCfg.StaHtEnable != pAd->StaActive.SupportedHtPhy.bHtEnable)
            {
                APStop(pAd, pAPPort);
            }
            else if( pAd->StaActive.SupportedHtPhy.bHtEnable)
            {
                if ((pPort->SoftAP.ApCfg.StaChannel != pPort->Channel) ||
                            (pPort->SoftAP.ApCfg.StaCentralChannel != pPort->CentralChannel))
                {
                    APStop(pAd, pAPPort);
                }
            }
            else
            {
                if ((pPort->SoftAP.ApCfg.StaChannel != pPort->Channel) ||
                            (pPort->CommonCfg.PhyMode != pPort->CommonCfg.LastPhyMode))
                {
                    APStop(pAd, pAPPort);
                }
            }
            APStartUp( pAd, pAPPort);
        }
        else
        {
            DBGPRINT(RT_DEBUG_WARN, ("AP port is NULL\n"));
        }
    }
    pPort->SoftAP.ApCfg.StaChannel = pPort->Channel;
    pPort->SoftAP.ApCfg.StaCentralChannel = pPort->CentralChannel;
    pPort->SoftAP.ApCfg.StaHtEnable = pAd->StaActive.SupportedHtPhy.bHtEnable;
    pPort->CommonCfg.LastPhyMode = pPort->CommonCfg.PhyMode;

    pAd->MlmeAux.bNeedIndicateRoamStatus = FALSE;

    pAd->StaCfg.bDropPower = FALSE; 

#if(COMPILE_WIN8_ABOVE(CURRENT_OS_NDIS_VER))
    //if (pPort->PortType == WFD_CLIENT_PORT)
    {
        P2pMsMlmeCntLinkUp(pAd, pPort);
    }
  
#endif
   
#ifdef MULTI_CHANNEL_SUPPORT
        MultiChannelDecideGOOpCh(pAd, pPort);
#endif /* MULTI_CHANNEL_SUPPORT */      

    pAd->LogoTestCfg.TurnOffBeaconListen = TRUE;
    pAd->LogoTestCfg.CounterForTOBL = 0;


#if(COMPILE_WIN8_ABOVE(CURRENT_OS_NDIS_VER))
    //
    // NLO indication
    //
    if (NDIS_WIN8_ABOVE(pAd)  && (pAd->StaCfg.NLOEntry.NLOEnable == TRUE) && (pAd->StaCfg.NLOEntry.IndicateNLO == TRUE))
    {
        pAd->StaCfg.NLOEntry.IndicateNLO = FALSE;
        N6PlatformIndicateNLOStatus(pAd, pPort);
    }
#endif

#if 0  // ToDo : check later : Lens
    //
    // Select the table of the per-rate Tx power control over MAC
    //
    SelectPerRateTxPowerCtrlOverMacTable76XXTssi(pAd, TRUE);

    // Thermal tuning for MT76XX
    RTUSBReadMACRegister(pAd, EDCA_AC0_CFG, &(pAd->HwCfg.MAC1300));
#endif


    AsicSwitchChannel(pPort, pPort->Channel, pPort->CentralChannel, pPort->BBPCurrentBW, FALSE);
    //pAd->TrackInfo.bStopCMDEventReceive = TRUE;

    //mt7603_tx_pwr_gain(pAd, pPort->Channel);

    DBGPRINT(RT_DEBUG_TRACE, ("%s TempSenser76XX - thermal record MAC1300 = 0x%x\n", __FUNCTION__, pAd->HwCfg.MAC1300));      

    DBGPRINT(RT_DEBUG_TRACE, ("Linkup End <======\n"));
}

/*
    ==========================================================================

    Routine Description:
        Disconnect current BSSID

    Arguments:
        pPort               - Pointer to our adapter
        IsReqFromAP     - Request from AP
        
    Return Value:       
        None
        
    IRQL = DISPATCH_LEVEL

    Note:
        We need more information to know it's this requst from AP.
        If yes! we need to do extra handling, for example, remove the WPA key.
        Otherwise on 4-way handshaking will faied, since the WPA key didn't be
        remove while auto reconnect.
        Disconnect request from AP, it means we will start afresh 4-way handshaking 
        on WPA mode.

    ==========================================================================
*/
VOID MlmeCntLinkDown(
    IN PMP_PORT   pPort,
    IN  BOOLEAN     IsReqFromAP)
{
    ULONG           SlotTime;
    UCHAR           i;
    PQUEUE_ENTRY    pQueueEntry;
    //PCIPHER_KEY pCipheyKey;
    //PRTMP_ADHOC_LIST pAdhocList = NULL;
    TX_RTY_CFG_STRUC    TxRtyCfg;
    TX_RTS_CFG_STRUC RtsCfg;
    PMP_ADAPTER pAd = pPort->pAd;
    BOOLEAN Cancelled = FALSE;
    UCHAR           Wcid = BSSID_WCID, CheckPortWcid = RESERVED_WCID, CheckPort2Wcid = RESERVED_WCID; 
    PMP_PORT      pCheckPort = NULL, pCheckPort2 = NULL;
    PMAC_TABLE_ENTRY pMacEntry = NULL;
    PMAC_TABLE_ENTRY pNextMacEntry = NULL;   
    PMAC_TABLE_ENTRY pWcidMacTabEntry = NULL;
    PQUEUE_HEADER pHeader;
    
#if(COMPILE_WIN8_ABOVE(CURRENT_OS_NDIS_VER))
    ULONG           iCount;
    PDOT11_OFFLOAD_NETWORK  pNLO;
    BOOLEAN         NLOEntry = FALSE;

    for (iCount = 0; iCount < pAd->StaCfg.NLOEntry.uNumOfEntries; iCount++)
    {
        pNLO= &pAd->StaCfg.NLOEntry.OffloadNetwork[iCount];

        // check SSID, Cipher and AuthMode
        if (!SSID_EQUAL(pPort->PortCfg.Ssid, pPort->PortCfg.SsidLen, pNLO->Ssid.ucSSID, pNLO->Ssid.uSSIDLength))
            continue;

        if (pPort->PortCfg.AuthMode!= (ULONG)pNLO->AuthAlgo)
            continue;

        if (pPort->PortCfg.PairCipher!= (ULONG)pNLO->UnicastCipher)
            continue;

        // the new BssEntry is in NLO entry, need to indicate NLO event after scan done
        NLOEntry = TRUE;

        DBGPRINT(RT_DEBUG_TRACE, ("%s: NLO: This AP is in NLO list\n", __FUNCTION__));
    }
#endif

    Wcid = MlmeSyncGetWlanIdxByPort(pPort, ROLE_WLANIDX_BSSID);   
    DBGPRINT(RT_DEBUG_TRACE, ("!!! LINK DOWN !!!\n"));

    if (VHT_NIC(pAd))
    {
        //
        // STA may change the Tx/Rx path upon the receipt of the VHT Operating Mode Notification Action
        //
        pAd->HwCfg.Antenna.word = pAd->HwCfg.EEPROMDefaultValue[EEPROM_NIC_CFG1_OFFSET];
        pAd->HwCfg.NicConfig3.field.TxStream = pAd->HwCfg.Antenna.field.TxPath;
    }

    pPort->StaCfg.WscControl.bQueueEAPOL = FALSE;

    pAd->Mlme.PeriodicRound  =0;

    pPort->CommonCfg.DelBACount = 0;  // for delBA
    pAd->StaCfg.Feature11n &= (~0x2);

    // === Multiple STA+Clients: Found out all possible CheckPort(s) =======
    // Current connecting port is sta port. Check other two clients.
    if (pPort->PortSubtype == PORTSUBTYPE_STA)
    {
        // get the current wcid
        Wcid = MlmeSyncGetWlanIdxByPort(pPort, ROLE_WLANIDX_BSSID);

        pCheckPort = MlmeSyncGetP2pClientPort(pAd, 0);
        if (pCheckPort)
            CheckPortWcid = P2pGetClientWcid(pAd, pCheckPort);
        
        pCheckPort2 = MlmeSyncGetP2pClientPort(pAd, 1);
        if (pCheckPort2)
            CheckPort2Wcid = P2pGetClientWcid(pAd, pCheckPort2);
    }
    // Current connecting port is p2p client port. Check sta and the other p2p client.
    else if (pPort->PortSubtype == PORTSUBTYPE_P2PClient)
    {
        PMP_PORT      pTmpPort = NULL;

        // get the current wcid
        Wcid = P2pGetClientWcid(pAd, pPort);
        
        pCheckPort = Ndis6CommonGetStaPort(pAd);
        if (pCheckPort)
            CheckPortWcid = MlmeSyncGetWlanIdxByPort(pPort, ROLE_WLANIDX_BSSID);

        for (i = 0; i < NUM_OF_CONCURRENT_CLIENT_ROLE; i++)
        {
            pTmpPort = MlmeSyncGetP2pClientPort(pAd, i);
            // this port is not the current port, found it
            if (pTmpPort && (pTmpPort->PortNumber != pPort->PortNumber))
                break;
            else
                pTmpPort = NULL;
        }
        pCheckPort2 = pTmpPort;
        if (pCheckPort2)
            CheckPort2Wcid = P2pGetClientWcid(pAd, pCheckPort2);
    }
    DBGPRINT(RT_DEBUG_TRACE, ("LinkDOWN(): Wcid=%d.  concurrent clients - 1'st Wcid=%d,  2'nd Wcid=%d\n", Wcid, CheckPortWcid, CheckPort2Wcid));

    pWcidMacTabEntry = MlmeSyncMacTabMatchedWlanIdx(pPort, Wcid);
    
    if (ADHOC_ON(pPort))        // Adhoc mode link down
    {
        OPSTATUS_CLEAR_FLAG(pPort, fOP_STATUS_ADHOC_ON);

#ifdef  SINGLE_ADHOC_LINKUP
        // clean up previous SCAN result, add current BSS back to table if any
        BssTableDeleteEntry(&pPort->PortCfg.BssTab, &(pPort->PortCfg.Bssid));
#else       
        // clean up previous SCAN result, add current BSS back to table if any
        BssTableDeleteEntry(&pAd->ScanTab, pPort->PortCfg.Bssid, pPort->Channel);
        if (MT_TEST_FLAG(pAd, fRTMP_ADAPTER_RADIO_OFF))
        {
            OPSTATUS_CLEAR_FLAG(pPort, fOP_STATUS_MEDIA_STATE_CONNECTED);               
        }
#endif
        //  When linkdown, we clear AdhocJoiner and AdhocCreator 
        //  that are used to indicate I am Adhoc joiner or creator.
        pAd->StaCfg.AdhocJoiner  = FALSE;
        pAd->StaCfg.AdhocCreator = FALSE;

        // Disable supported Ht rate
        pAd->StaActive.SupportedHtPhy.bHtEnable = FALSE;
        
        // ad-hoc linkdown Reset MAC Table. Start from 1 or 0 ?
        pHeader = &pPort->MacTab.MacTabList;
        pNextMacEntry = (PMAC_TABLE_ENTRY)(pHeader->Head);
        while (pNextMacEntry != NULL)
        {
           if (pPort->MacTab.Size == 0)
                break;
            
            pMacEntry = (PMAC_TABLE_ENTRY)pNextMacEntry;

            if(pMacEntry == NULL)
            {
                break; 
            }
            
            if(pMacEntry->WlanIdxRole == ROLE_WLANIDX_MBCAST)
            {
                pNextMacEntry = pNextMacEntry->Next;   
                pMacEntry = NULL;
                continue; 
            }
            if (pMacEntry->ValidAsCLI == TRUE)
            {   
                BOOLEAN bDelete =FALSE;

                if((pPort->PortNumber ==0 && pMacEntry->WlanIdxRole == ROLE_WLANIDX_BSSID) || (pMacEntry->WlanIdxRole == ROLE_WLANIDX_MBCAST))
                {
                    bDelete = FALSE;
                }
                else
                {
                    bDelete = TRUE;
                }
                MtAsicDelWcidTab(pAd, (UCHAR)pMacEntry->wcid);
                MacTableDeleteAndResetEntry(pAd, pPort, pMacEntry->wcid, pMacEntry->Addr, bDelete);
            }
            
            pNextMacEntry = pNextMacEntry->Next;   
            pMacEntry = NULL;
        }
       
        //PlatformZeroMemory(&pPort->MacTab, sizeof(MAC_TABLE));

        //pCipheyKey = &pPort->SharedKey[BSS0][0];
        //pCipheyKey->KeyLen = 0;

        //
        // Clear Adhoc connect list.
        //
        NdisAcquireSpinLock(&pAd->StaCfg.AdhocListLock);
        pQueueEntry = RemoveHeadQueue(&pAd->StaCfg.ActiveAdhocListQueue);
        while (pQueueEntry)
        {
            InsertTailQueue(&pAd->StaCfg.FreeAdhocListQueue, pQueueEntry);
            pQueueEntry = RemoveHeadQueue(&pAd->StaCfg.ActiveAdhocListQueue);
        }
        NdisReleaseSpinLock(&pAd->StaCfg.AdhocListLock);
    }
    else if(INFRA_ON(pPort))// Infra structure mode             // Infra structure mode
    {
        ULONG latestBSSEntry = BSS_NOT_FOUND;

        latestBSSEntry = BssTableSearch(pAd, pPort, &pAd->ScanTab, pPort->PortCfg.Bssid, pPort->Channel);

        //
        // Cache the latest disassociated BSS entry that the STA can potentially roam to
        //
        if (latestBSSEntry != BSS_NOT_FOUND)
        {
            if (pAd->ScanTab.BssEntry[latestBSSEntry].AuthMode == Ralink802_11AuthModeWPA2)
            {
                PlatformMoveMemory(&pPort->CommonCfg.LatestBSSEntry, &pAd->ScanTab.BssEntry[latestBSSEntry], sizeof (BSS_ENTRY));

                DBGPRINT(RT_DEBUG_TRACE, ("%s: Latest BSSID, BSSID = %02X:%02X:%02X:%02X:%02X:%02X\n", 
                    __FUNCTION__, 
                    pPort->CommonCfg.LatestBSSEntry.Bssid[0], pPort->CommonCfg.LatestBSSEntry.Bssid[1], pPort->CommonCfg.LatestBSSEntry.Bssid[2], 
                    pPort->CommonCfg.LatestBSSEntry.Bssid[3], pPort->CommonCfg.LatestBSSEntry.Bssid[4], pPort->CommonCfg.LatestBSSEntry.Bssid[5]));
            }
        }

        DBGPRINT(RT_DEBUG_TRACE, ("!!! LINK DOWN (Infrastructure mode Port#%d - Wcid#%d)!!!\n", pPort->PortNumber, Wcid));

        PlatformCancelTimer(&pWcidMacTabEntry->RetryTimer, &Cancelled);

        // DLS tear down frame must be sent before link down
        // send DLS-TEAR_DOWN message
        if (DLS_ON(pAd))
        {
            DlsTearDown(pAd, pPort);
        }
        
        // send TDLS-TEAR_DOWN message
        if (TDLS_ON(pAd))
        {
            TdlsTearDown(pAd, TRUE);
        }

        OPSTATUS_CLEAR_FLAG(pPort, fOP_STATUS_INFRA_ON); 
        OPSTATUS_CLEAR_FLAG(pPort, fOP_STATUS_MEDIA_STATE_CONNECTED); 
        // Saved last SSID for linkup comparison
        pPort->PortCfg.LastSsidLen = pPort->PortCfg.SsidLen;
        PlatformZeroMemory(pPort->PortCfg.LastSsid, MAX_LEN_OF_SSID);
        PlatformMoveMemory(pPort->PortCfg.LastSsid, pPort->PortCfg.Ssid, pPort->PortCfg.LastSsidLen);
        COPY_MAC_ADDR(pPort->PortCfg.LastBssid, pPort->PortCfg.Bssid);
        if (pAd->MlmeAux.CurrReqIsFromNdis == TRUE)
        {   
            DBGPRINT(RT_DEBUG_TRACE, ("NDIS_STATUS_MEDIA_DISCONNECT Event A!\n"));
            pAd->MlmeAux.CurrReqIsFromNdis = FALSE;
        }
        else
        {
            // Set linkdown timer
            // do not need it now
            //PlatformSetTimer(pPort, &pAd->Mlme.MlmeCntLinkDownTimer, 10000);

            //
            // If disassociation request is from NDIS, then we don't need to delete BSSID from entry.
            // Otherwise lost beacon or receive De-Authentication from AP, 
            // then we should delete BSSID from BssTable.
            // If we don't delete from entry, roaming will fail.
            //          
#if(COMPILE_WIN8_ABOVE(CURRENT_OS_NDIS_VER))
            if(NLOEntry == TRUE)
            {
                DBGPRINT(RT_DEBUG_TRACE, ("Don't delete the BssEntry from BssTab because it is NLOEntry\n"));
            }
            else 
#endif
            {       
                BssTableDeleteEntry(&pAd->ScanTab, pPort->PortCfg.Bssid, pPort->Channel);           
            }
        }

        // restore back to - 
        //      1. long slot (20 us) or short slot (9 us) time
        //      2. turn on/off RTS/CTS and/or CTS-to-self protection
        //      3. short preamble
        OPSTATUS_CLEAR_FLAG(pAd, fOP_STATUS_BG_PROTECTION_INUSED);
        OPSTATUS_CLEAR_FLAG(pAd, fOP_STATUS_USECCK_PROTECTION_ENABLE);

        if ( !MAC_ADDR_EQUAL(pPort->PortCfg.Bssid,ZERO_MAC_ADDR ))
        {
            PMAC_TABLE_ENTRY pEntry = MacTableLookup(pAd,  pPort, pPort->PortCfg.Bssid);
            if (pEntry != NULL)
            {
                BOOLEAN bDelete = FALSE;
                if((pPort->PortNumber ==0 && pEntry->WlanIdxRole == ROLE_WLANIDX_BSSID) || (pEntry->WlanIdxRole == ROLE_WLANIDX_MBCAST))
            {
                    bDelete = FALSE;
            }
                else
                {
                    bDelete = TRUE;
                }
            
                MtAsicDelWcidTab(pAd, (UCHAR)pEntry->wcid);
                MacTableDeleteAndResetEntry(pAd, pPort, pEntry->Aid, pPort->PortCfg.Bssid, bDelete);
        }
        }

        MlmeSyncMacTableDumpInfo(pPort);

#ifdef MULTI_CHANNEL_SUPPORT
        // CLI might get AP's deauth, handle multichannel disconnect here. Stop multichannel phase2
        if ((pAd->MccCfg.MultiChannelEnable == TRUE) && 
            (pPort->PortType == EXTSTA_PORT) &&
            (MT_TEST_BIT(pAd->pP2pCtrll->MultiChannelStartOrEndProcess, MULTI_CHANNEL_STA_PRE_STOP) == FALSE) &&
            (MT_TEST_BIT(pAd->pP2pCtrll->MultiChannelStartOrEndProcess, MULTI_CHANNEL_STA_PRE_RESET_REQ) == FALSE))
        {
            MT_SET_BIT(pAd->pP2pCtrll->MultiChannelStartOrEndProcess, MULTI_CHANNEL_STA_STOP);
        }
        else if ((pAd->MccCfg.MultiChannelEnable == TRUE) &&
            (pPort->PortType == WFD_CLIENT_PORT) &&
            (MT_TEST_BIT(pAd->pP2pCtrll->MultiChannelStartOrEndProcess, MULTI_CHANNEL_CLI_PRE_STOP) == FALSE) &&
            (MT_TEST_BIT(pAd->pP2pCtrll->MultiChannelStartOrEndProcess, MULTI_CHANNEL_CLI_PRE_RESET_REQ) == FALSE))
        {
            MT_SET_BIT(pAd->pP2pCtrll->MultiChannelStartOrEndProcess, MULTI_CHANNEL_CLI_STOP);
        }       
#endif /* MULTI_CHANNEL_SUPPORT */          

       

        NdisCommonMappingPortToBssIdx(pPort, DO_NOTHING, DELETE_MAC);
    }
    else // Neither INFRA_ON nor ADHOC_ON
    {
        DBGPRINT(RT_DEBUG_WARN, ("Double link down ??\n"));
        return;
    }

#if 0
#ifdef MULTI_CHANNEL_SUPPORT
    // Let BA mgt pakcet could be sent out
    // Wait previous pakcet to be sent
    //MultiChannelWaitTxRingEmpty(pAd, MultiChannelGetSwQ(MultiChannelGetHwQ(pAd, pPort))); 
    MultiChannelWaitTxRingEmpty(pAd);   
    MultiChannelWaitHwQEmpty(pAd, MultiChannelGetHwQ(pAd, pPort), 500); 
#endif /*MULTI_CHANNEL_SUPPORT*/
#endif

    //
    // Reset CWMin & CWMax to default value
    // Since we reset the slot time to 0x14(long slot time), so we also need to  
    // Reset the flag fOP_STATUS_SHORT_SLOT_INUSED at the same time.
    // 
    SlotTime = 0x14;
    //rt2860c
    //RTUSBWriteMACRegister(pAd, BKOFF_SLOT_CFG, SlotTime);
    OPSTATUS_CLEAR_FLAG(pAd, fOP_STATUS_SHORT_SLOT_INUSED);
    
    MtAsicSetSlotTime(pAd, TRUE, pPort->Channel); //FALSE);
    MtAsicSetEdcaParm(pAd, pPort, NULL, TRUE);

    MtAsicSetRxFilter(pAd);

    // Set LED
    if ((pPort->SoftAP.bAPStart == FALSE) && (pPort->P2PCfg.bGOStart == FALSE))
    {
        if((pAd->HwCfg.LedCntl.field.LedMode & LED_MODE) != WPS_LED_MODE_6)
            LedCtrlSetLed(pAd, LED_LINK_DOWN);
        else
        {
            if(pPort->StaCfg.WscControl.WscState == WSC_STATE_OFF)
            {
                LedCtrlSetLed(pAd, LED_LINK_DOWN);
            }
        }
    }
    
    if((pPort->StaCfg.WscControl.WscState == WSC_STATE_OFF) && (pPort->StaCfg.WscControl.PeerDisconnect))
    {
        LedCtrlSetLed(pAd, LED_WPS_TURN_LED_OFF);
        pPort->StaCfg.WscControl.PeerDisconnect = FALSE;
    }
    
    pAd->HwCfg.LedIndicatorStregth = 0xF0;
    LedCtrlSetSignalLed(pAd, -100);    // Force signal strength Led to be turned off, firmware is not done it.
    pPort->CommonCfg.IOTestParm.bCurrentAtheros = FALSE; 
    
    pAd->Mlme.PeriodicRound = 0;
    pAd->Mlme.OneSecPeriodicRound = 0;
    
    if (pAd->HwCfg.BeaconNum == 0)
    {
        MtAsicDisableSync(pAd);
    }

    if ((pPort->BBPCurrentBW == BW_40) && (pPort->SoftAP.bAPStart == FALSE) && (pPort->P2PCfg.bGOStart == FALSE))
    {
        if ((pCheckPort) && (!IDLE_ON(pCheckPort)))
            ;
        else if ((pCheckPort2) && (!IDLE_ON(pCheckPort2)))
            ;
        else
        {
            pPort->BBPCurrentBW = BW_20;
            MtAsicSetBW(pAd, BW_20);
            DBGPRINT(RT_DEBUG_TRACE, ("!!!LINK DOWN set back to 20MHz !!! \n" ));
        }
    }

    // Remove StaCfg Information after link down
    PlatformZeroMemory(pPort->PortCfg.Bssid, MAC_ADDR_LEN);
    PlatformZeroMemory(pPort->PortCfg.Ssid, MAX_LEN_OF_SSID);
    PlatformZeroMemory(&pAd->MlmeAux.HtCapability, sizeof(HT_CAPABILITY_IE));
    PlatformZeroMemory(&pAd->MlmeAux.AddHtInfo, sizeof(ADD_HT_INFO_IE));

    // 
    pAd->MlmeAux.bVhtCapable = FALSE;
    PlatformZeroMemory(&pAd->MlmeAux.VhtCapability, sizeof(SIZE_OF_VHT_CAP_IE));
    PlatformZeroMemory(&pAd->MlmeAux.VhtOperation, sizeof(SIZE_OF_VHT_OP_IE));
    PlatformZeroMemory(&pAd->StaActive.SupportedVhtPhy, sizeof(RT_VHT_CAPABILITY));

    pAd->MlmeAux.ExtCapIE.DoubleWord = 0;
        // fix bug: p2p both concurrent only can connect one time
        // beacuse the bHTenable in StaActive will be modify after link down
    //PlatformZeroMemory(&pAd->StaActive, sizeof(STA_ACTIVE_CONFIG)); 
    pAd->MlmeAux.HtCapabilityLen = 0;
    pAd->MlmeAux.NewExtChannelOffset = 0xff;
    pPort->PortCfg.SsidLen = 0;
    pAd->MlmeAux.bWCN = FALSE;
    
    // Reset WPA-PSK state. Only reset when supplicant enabled
    if (pAd->StaCfg.WpaState != SS_NOTUSE && pAd->StaCfg.WpaState < SS_STATE_MAX)
    {
        pAd->StaCfg.WpaState = SS_START;
        // Clear Replay counter
        PlatformZeroMemory(pAd->StaCfg.ReplayCounter, 8);
    }
    
    //
    // if link down come from AP, we need to remove all WPA keys on WPA mode.
    // otherwise will cause 4-way handshaking failed, since the WPA key not empty.
    // It could be open wep with 802.1x
    //
    if ((IsReqFromAP) && (pPort->PortCfg.AuthMode >= Ralink802_11AuthModeWPA))
    {
        // Remove all WPA keys 
        MlmeInfoWPARemoveAllKeys(pAd,pPort);
    }
    
    // We should reset keys after Disassoc/Deauth mgmt frame was sent out.
    // Because Disassoc/Deauth mgmt frame MUST be encrypted with the unicast key.
    if (MFP_ON(pAd, pPort))
    {
        // Remove all WPA keys 
        MlmeInfoWPARemoveAllKeys(pAd,pPort);
    }

    // 802.1x port control
    // Use P2P's PortSecured in concurrent Client
    if (IS_P2P_MS_CLI_WCID(pPort, Wcid))
        pWcidMacTabEntry->PortSecured = WPA_802_1X_PORT_NOT_SECURED;
    else
        pAd->StaCfg.PortSecured = WPA_802_1X_PORT_NOT_SECURED;
    pAd->StaCfg.MicErrCnt = 0;

    // Update extra information to link is up
    pAd->UiCfg.ExtraInfo = GENERAL_LINK_DOWN;

    // Reset it to default(from registry value) 
    pPort->CommonCfg.bAggregationCapable = pAd->pHifCfg->bUsbTxBulkAggre;

    // Clean association information
    PlatformZeroMemory(&pAd->StaCfg.AssocInfo, sizeof(NDIS_802_11_ASSOCIATION_INFORMATION));
    pAd->StaCfg.AssocInfo.Length = sizeof(NDIS_802_11_ASSOCIATION_INFORMATION);
    pAd->StaCfg.ReqVarIELen = 0;
    pAd->StaCfg.ResVarIELen = 0;
    pAd->StaCfg.bIdle300M = FALSE;

    MtAsicSetTxStream(pAd, pAd->HwCfg.Antenna.field.TxPath);

    // Reset DAC
    BbSetTxDacCtrlByTxPath(pAd);
    RTUSBWriteMACRegister(pAd, MAX_LEN_CFG, MAC_VALUE_MAX_LEN_CFG_DEFAULT(pAd));
    //
    // Reset RSSI value after link down
    //
    pAd->StaCfg.RssiSample.AvgRssi[0] = 0;
    pAd->StaCfg.RssiSample.AvgRssiX8[0] = 0;
    pAd->StaCfg.RssiSample.AvgRssi[1] = 0;
    pAd->StaCfg.RssiSample.AvgRssiX8[1] = 0;
    
    if (!MT_TEST_FLAG(pAd, fRTMP_ADAPTER_SLEEP_IN_PROGRESS))
    {       
        MtAsicUpdateProtect(pAd, HT_NO_PROTECT, (ALLN_SETPROTECT|CCKSETPROTECT|OFDMSETPROTECT), TRUE, FALSE);
    }   

    // Disable aggregation as linkdown
    OPSTATUS_CLEAR_FLAG(pAd, fOP_STATUS_AGGREGATION_INUSED);
    pWcidMacTabEntry = MlmeSyncMacTabMatchedWlanIdx(pPort, CheckPortWcid); 

    if((pWcidMacTabEntry == NULL) && (CheckPortWcid != 0xff))
    {
        DBGPRINT(RT_DEBUG_TRACE,("%s(), Line:%d, Mac Entry is NULL  Wcid = %d\n", __FUNCTION__, __LINE__, CheckPortWcid));
        return;
    }
    
    // Clear other port status once disable aggregation function.
    if ((pCheckPort) && INFRA_ON(pCheckPort))
        CLIENT_STATUS_CLEAR_FLAG(pWcidMacTabEntry, fCLIENT_STATUS_AGGREGATION_CAPABLE);

    pWcidMacTabEntry = MlmeSyncMacTabMatchedWlanIdx(pPort, CheckPort2Wcid);

    if((pWcidMacTabEntry == NULL) && (CheckPort2Wcid !=0xff))
    {
        DBGPRINT(RT_DEBUG_TRACE,("%s(), Line:%d, Mac Entry is NULL  Wcid = %d\n", __FUNCTION__, __LINE__, CheckPort2Wcid));
        return;
    }
    
    if ((pCheckPort2) && INFRA_ON(pCheckPort2))
        CLIENT_STATUS_CLEAR_FLAG(pWcidMacTabEntry, fCLIENT_STATUS_AGGREGATION_CAPABLE);

    // If another connected port still lifes, don't turn off Piggyback and RDG.
    if ((pCheckPort) && (!IDLE_ON(pCheckPort)))
        ;
    else if ((pCheckPort2) && (!IDLE_ON(pCheckPort2)))
        ;
    else
    {
        AsicDisableRDG(pAd, pPort);
        AsicDisablePiggyB(pAd);
    }
    
    // Restore MlmeRate
    pPort->CommonCfg.MlmeRate = pPort->CommonCfg.BasicMlmeRate;

    // Restore LastWcid
    pAd->StaCfg.LastWcid = (UCHAR)Wcid;

    pPort->MacTab.fAnyBASession = FALSE;
    //pPort->CommonCfg.RtsRate = pPort->CommonCfg.BasicMlmeRate;
    DBGPRINT(RT_DEBUG_TRACE, ("!!! LINK DOWN <==!!!\n"));

    OPSTATUS_CLEAR_FLAG(pPort, fOP_STATUS_RTS_PROTECTION_ENABLE);

    //
    // Change retry setting
    //
    RTUSBReadMACRegister(pAd, TX_RTY_CFG, &TxRtyCfg.word);
    TxRtyCfg.field.LongRtyLimit = 0x1f;
    TxRtyCfg.field.ShortRtyLimit = 0x1f;
    RTUSBWriteMACRegister(pAd, TX_RTY_CFG, TxRtyCfg.word);
    OPSTATUS_CLEAR_FLAG(pAd, fOP_STATUS_LONG_RETRY);

    //
    // Update RTS retry limit
    //
    RTUSBReadMACRegister(pAd, TX_RTS_CFG, &RtsCfg.word);
    RtsCfg.field.AutoRtsRetryLimit = 0;
    RTUSBWriteMACRegister(pAd, TX_RTS_CFG, RtsCfg.word);

    // Reset to initial state.  
    MP_SET_STATE(pPort, INIT_STATE);  
    // ================  Below is for 802.11n D4.0  ====================
    if (OPSTATUS_TEST_FLAG(pPort, fOP_STATUS_SCAN_2040))
    {
        OPSTATUS_CLEAR_FLAG(pPort, fOP_STATUS_SCAN_2040);

        // If another connected port still lifes, don't turn off.
        if ((pCheckPort) && (!IDLE_ON(pCheckPort)))
            ;
        else if ((pCheckPort2) && (!IDLE_ON(pCheckPort2)))
            ;
        else
        {
            OPSTATUS_CLEAR_FLAG(pAd, fOP_STATUS_SCAN_2040);
            // Reset BSSCoexist2040 frame to zero.
            pPort->CommonCfg.BSSCoexist2040.word = 0;
            // Clear Triggere event table after link down.
            TriEventInit(pAd, pPort);
            // A new EffectedChannelList is required at every link up. Do delete this in linkdown.
            MlmeSyncDeleteEffectedChannelList(pAd);
        }
    }
    // ==========================================================

#if(COMPILE_WIN8_ABOVE(CURRENT_OS_NDIS_VER))
    if(pPort->PortType == WFD_CLIENT_PORT)
    {
        P2pMsMlmeCntLinkDown(pAd, pPort, IsReqFromAP);
    }
    else
#endif
    {
        P2pMlmeCntLinkDown(pAd, pPort, IsReqFromAP);
    }

#ifdef MULTI_CHANNEL_SUPPORT
    MultiChannelSetDeafultHwQ(pAd, pPort);
#endif /*MULTI_CHANNEL_SUPPORT*/

    //2009 PF#1:
    pPort->CommonCfg.RxSTBCCount = pPort->CommonCfg.TxSTBCCount = 0; //Check me!!

    if(VHT_NIC(pAd))
        pPort->CommonCfg.BaLimit = 30;
    else if ((pAd->HwCfg.MACVersion&0xffff0000) != 0x28600000)
    {
        pPort->CommonCfg.BaLimit = 14;
    }
    else
    {
        pPort->CommonCfg.BaLimit = 7;
    }
    
    StopFrequencyCalibration(pAd); // To stop the frequency calibration
    //
    pAd->CountDowntoRadioOff = STAY_IN_AWAKE;
    
    if((pPort->StaCfg.WscControl.WscConfMode == WSC_ConfMode_REGISTRAR)&&
        (pPort->StaCfg.WscControl.WscState == WSC_STATE_WAIT_DISCONN))
    {
        pPort->StaCfg.WscControl.WscStatus = STATUS_WSC_EAP_CONFIGURED;
        pPort->StaCfg.WscControl.WscState= WSC_STATE_OFF;
    }

    pAd->StaCfg.RssiSample.LastRssi[0]  = 0;
    pAd->StaCfg.RssiSample.LastRssi[1]  = 0;
    pAd->StaCfg.RssiSample.LastRssi[2]  = 0;
    pAd->StaCfg.RssiSample.AvgRssi[0]   = 0;
    pAd->StaCfg.RssiSample.AvgRssiX8[0] = 0;
    pAd->StaCfg.RssiSample.AvgRssi[1]   = 0;
    pAd->StaCfg.RssiSample.AvgRssiX8[1] = 0;
    pAd->StaCfg.RssiSample.AvgRssi[2]   = 0;
    pAd->StaCfg.RssiSample.AvgRssiX8[2] = 0;

    DBGPRINT(RT_DEBUG_TRACE, ("%s: pPort->State = INIT_STATE\n", __FUNCTION__));
}

/*
    ==========================================================================
    Description:

    IRQL = PASSIVE_LEVEL

    ==========================================================================
*/
VOID MlmeCntlConfirm(
    IN PMP_ADAPTER pAd, 
    IN ULONG MsgType, 
    IN USHORT Msg) 
{
    PMP_PORT pPort = pAd->PortList[PORT_0];
    MlmeEnqueue(pAd,pPort, MLME_CNTL_STATE_MACHINE, MsgType, sizeof(USHORT), &Msg);
}

/*

/*
    ==========================================================================
    Description:

    IRQL = DISPATCH_LEVEL
    
    ==========================================================================
*/
VOID MlmeCntIterateOnBssTab(
    IN PMP_PORT pPort) 
{
    MLME_START_REQ_STRUCT   StartReq;
    MLME_JOIN_REQ_STRUCT    JoinReq;
    ULONG                   BssIdx;
    PMP_ADAPTER pAd = pPort->pAd;

    ULONG Value = 0;
    UCHAR ByteValue = 0;

    // Change the wepstatus to original wepstatus
    
    pPort->PortCfg.WepStatus   = pPort->PortCfg.OrigWepStatus;
    pPort->PortCfg.PairCipher  = pPort->PortCfg.OrigWepStatus;
    pPort->PortCfg.GroupCipher = pPort->PortCfg.OrigWepStatus;
    pPort->PortCfg.MixedModeGroupCipher = Cipher_Type_NONE;

    BssIdx = pAd->MlmeAux.BssIdx;
    if (BssIdx < pAd->MlmeAux.SsidBssTab.BssNr) 
    {
        // Check cipher suite, AP must have more secured cipher than station setting
        // Set the Pairwise and Group cipher to match the intended AP setting
        // We can only connect to AP with less secured cipher setting
        if ((pPort->PortCfg.AuthMode == DOT11_AUTH_ALGO_WPA) || (pPort->PortCfg.AuthMode == DOT11_AUTH_ALGO_WPA_PSK))
        {
            if (pAd->MlmeAux.SsidBssTab.BssEntry[BssIdx].WPA.GroupCipher!=Ralink802_11EncryptionDisabled)
                pPort->PortCfg.GroupCipher = pAd->MlmeAux.SsidBssTab.BssEntry[BssIdx].WPA.GroupCipher;
            else if (pAd->MlmeAux.SsidBssTab.BssEntry[BssIdx].WPA2.GroupCipher!=Ralink802_11EncryptionDisabled)
                pPort->PortCfg.GroupCipher = pAd->MlmeAux.SsidBssTab.BssEntry[BssIdx].WPA2.GroupCipher;
            else
            pPort->PortCfg.GroupCipher = pAd->MlmeAux.SsidBssTab.BssEntry[BssIdx].WPA.GroupCipher;
    
            if (pPort->PortCfg.WepStatus == pAd->MlmeAux.SsidBssTab.BssEntry[BssIdx].WPA.PairCipher)
                pPort->PortCfg.PairCipher = pAd->MlmeAux.SsidBssTab.BssEntry[BssIdx].WPA.PairCipher;
            else if (pAd->MlmeAux.SsidBssTab.BssEntry[BssIdx].WPA.PairCipherAux != DOT11_CIPHER_ALGO_NONE)
                pPort->PortCfg.PairCipher = pAd->MlmeAux.SsidBssTab.BssEntry[BssIdx].WPA.PairCipherAux;
            else    // There is no PairCipher Aux, downgrade our capability to TKIP
                pPort->PortCfg.PairCipher = DOT11_CIPHER_ALGO_TKIP;         

            // Set the mixed mode group cipher suite, may be Cipher_Type_NONE, Cipher_Type_WEP40 or Cipher_Type_WEP104
            pPort->PortCfg.MixedModeGroupCipher = pAd->MlmeAux.SsidBssTab.BssEntry[BssIdx].MixedModeGroupCipher;
        }
        else if ((pPort->PortCfg.AuthMode == DOT11_AUTH_ALGO_RSNA) || (pPort->PortCfg.AuthMode == DOT11_AUTH_ALGO_RSNA_PSK))
        {
            if (pAd->MlmeAux.SsidBssTab.BssEntry[BssIdx].WPA2.GroupCipher!=Ralink802_11EncryptionDisabled)
                pPort->PortCfg.GroupCipher = pAd->MlmeAux.SsidBssTab.BssEntry[BssIdx].WPA2.GroupCipher;
            else if (pAd->MlmeAux.SsidBssTab.BssEntry[BssIdx].WPA.GroupCipher!=Ralink802_11EncryptionDisabled)
                pPort->PortCfg.GroupCipher = pAd->MlmeAux.SsidBssTab.BssEntry[BssIdx].WPA.GroupCipher;
            else
            pPort->PortCfg.GroupCipher = pAd->MlmeAux.SsidBssTab.BssEntry[BssIdx].WPA2.GroupCipher;
            
            if (pPort->PortCfg.WepStatus == pAd->MlmeAux.SsidBssTab.BssEntry[BssIdx].WPA2.PairCipher)
                pPort->PortCfg.PairCipher = pAd->MlmeAux.SsidBssTab.BssEntry[BssIdx].WPA2.PairCipher;
            else if (pAd->MlmeAux.SsidBssTab.BssEntry[BssIdx].WPA2.PairCipherAux != DOT11_CIPHER_ALGO_NONE)
                pPort->PortCfg.PairCipher = pAd->MlmeAux.SsidBssTab.BssEntry[BssIdx].WPA2.PairCipherAux;
            else    // There is no PairCipher Aux, downgrade our capability to TKIP
                pPort->PortCfg.PairCipher = DOT11_CIPHER_ALGO_TKIP;         

            // RSN capability
            pPort->PortCfg.RsnCapability = pAd->MlmeAux.SsidBssTab.BssEntry[BssIdx].WPA2.RsnCapability;

            // 11w: STA should use SHA256 to do 4-way if AP's AKM is 5 or 6
            pPort->PortCfg.bAKMwSHA256 = pAd->MlmeAux.SsidBssTab.BssEntry[BssIdx].WPA2.bAKMwSHA256;

            // Set the mixed mode group cipher suite, may be Cipher_Type_NONE, Cipher_Type_WEP40 or Cipher_Type_WEP104
            pPort->PortCfg.MixedModeGroupCipher = pAd->MlmeAux.SsidBssTab.BssEntry[BssIdx].MixedModeGroupCipher;
        }

        // Set Mix cipher flag
        if (pPort->PortCfg.PairCipher != pPort->PortCfg.GroupCipher)
            pPort->PortCfg.bMixCipher = TRUE;       

        DBGPRINT(RT_DEBUG_TRACE, ("CNTL - iterate BSS %d of %d\n", BssIdx, pAd->MlmeAux.SsidBssTab.BssNr));
        MlmeCntJoinParmFill(pAd, &JoinReq, BssIdx);
        MlmeEnqueue(pAd,pPort, SYNC_STATE_MACHINE, MT2_MLME_JOIN_REQ, sizeof(MLME_JOIN_REQ_STRUCT), &JoinReq);  //Invoke MlmeSyncJoinReqAction
        pPort->Mlme.CntlMachine.CurrState = CNTL_WAIT_JOIN;     
    }
    else if (pAd->StaCfg.BssType == BSS_ADHOC)
    {
        //
        // Check if we want to start an ADHOC and this is on Jappan channel
        //
        if ((pPort->CommonCfg.bIEEE80211H) && JapanChannelCheck(pPort->Channel))
        {
            DBGPRINT(RT_DEBUG_TRACE, ("CNTL - All BSS failed; But we can't start an ADHOC on Jappan channel(=%d)\n", pPort->Channel));
            pPort->Mlme.CntlMachine.CurrState = CNTL_IDLE;
            return;
        }

        if (pAd->StaCfg.IBSSJoinOnly == FALSE)
        {
            DBGPRINT(RT_DEBUG_TRACE, ("CNTL - All BSS fail; start a new ADHOC (Ssid=%s)...\n",pAd->MlmeAux.Ssid));
            MlmeCntStartParmFill(pPort, &StartReq, pAd->MlmeAux.Ssid, pAd->MlmeAux.SsidLen);
            MlmeEnqueue(pAd,pPort, SYNC_STATE_MACHINE, MT2_MLME_START_REQ, sizeof(MLME_START_REQ_STRUCT), &StartReq);  //Invoke MlmeSyncStartReqAction
            pPort->Mlme.CntlMachine.CurrState = CNTL_WAIT_START;
        }
        else
        {
            pPort->Mlme.CntlMachine.CurrState = CNTL_IDLE;          
        }
    }
    else // no more BSS
    {
        DBGPRINT(RT_DEBUG_TRACE, ("CNTL - All roaming failed, stay @ ch #%d\n", pPort->Channel));
        //If softap port is started and station port's connection attemp fails, some values
        //have to be restored or connected clients will disconnect.
        if ((pPort->SoftAP.bAPStart) || (pPort->P2PCfg.bGOStart))
        {
#if 0  
            ADD_HTINFO HtInfo = pPort->SoftAP.ApCfg.AddHTInfoIe.AddHtInfo;

            if (HtInfo.RecomWidth == BW_40)
            {
                AsicSwitchChannel(pAd, pPort->CentralChannel, FALSE);
                

                if (HtInfo.ExtChanOffset == EXTCHA_ABOVE)
                {
                    SwitchBandwidth(pAd, FALSE, pPort->CentralChannel, BW_40, EXTCHA_ABOVE);
                }
                else if (HtInfo.ExtChanOffset == EXTCHA_BELOW)
                {
                    SwitchBandwidth(pAd, FALSE, pPort->CentralChannel, BW_40, EXTCHA_BELOW);
                }
                else
                {
                    AsicSwitchChannel(pAd, pPort->Channel, FALSE);
                }
            }
            else
            {
                AsicSwitchChannel(pAd, pPort->Channel, FALSE);
            }
#endif
            AsicSwitchChannel(pPort, pPort->Channel, pPort->CentralChannel, pPort->BBPCurrentBW, FALSE);
            //Resume beacon which is paused in MlmeSyncJoinReqAction().
            AsicResumeBssSync(pAd);
        }
        else
        {
            //AsicSwitchChannel(pAd, pPort->Channel,FALSE);
            AsicSwitchChannel(pPort, pPort->Channel, pPort->CentralChannel, pPort->BBPCurrentBW, FALSE);
        }

        pPort->Mlme.CntlMachine.CurrState = CNTL_IDLE;  
        if (pAd->MlmeAux.bStaCanRoam)
        {
            PlatformIndicateRoamingCompletion(pAd,pPort, DOT11_CONNECTION_STATUS_CANDIDATE_LIST_EXHAUSTED);
        }
        else
        {
#if(COMPILE_WIN8_ABOVE(CURRENT_OS_NDIS_VER))
            // No ConnectionCompletion with failure for wps connection request.
            // Because we didn't indicate ConnectionStart yet.
            if ((pPort->PortType == WFD_CLIENT_PORT) && (pPort->P2PCfg.WaitForWPSReady == TRUE) && (pPort->P2PCfg.ReadyToConnect == FALSE))
                ;
            else
#endif              
            {
                PlatformIndicateConnectionCompletion(pAd,pPort, DOT11_CONNECTION_STATUS_CANDIDATE_LIST_EXHAUSTED);      
                LedCtrlConnectionCompletion(pAd,pPort, FALSE);
            }
        }
    } 
}

// for re-association only
// IRQL = DISPATCH_LEVEL
VOID MlmeCntIterateOnBssTab2(
    IN PMP_PORT pPort) 
{
    MLME_REASSOC_REQ_STRUCT ReassocReq;
    ULONG                   BssIdx;
    BSS_ENTRY               *pBss;
    PMP_ADAPTER pAd = pPort->pAd;

    BssIdx = pAd->MlmeAux.RoamIdx;
    pBss = &pAd->MlmeAux.RoamTab.BssEntry[BssIdx];

    if (BssIdx < pAd->MlmeAux.RoamTab.BssNr)
    {
        DBGPRINT(RT_DEBUG_TRACE, ("CNTL - iterate BSS %d of %d\n", BssIdx, pAd->MlmeAux.RoamTab.BssNr));

        //AsicSwitchChannel(pAd, pBss->Channel,FALSE);
        AsicSwitchChannel(pPort, pPort->Channel, pPort->CentralChannel, pPort->BBPCurrentBW, FALSE);

        // reassociate message has the same structure as associate message
        MlmeCntAssocParmFill(pAd, &ReassocReq, pBss->Bssid, pBss->CapabilityInfo, 
                      ASSOC_TIMEOUT, pAd->StaCfg.DefaultListenCount);
        MlmeEnqueue(pAd,pPort, ASSOC_STATE_MACHINE, MT2_MLME_REASSOC_REQ, 
                    sizeof(MLME_REASSOC_REQ_STRUCT), &ReassocReq);

        pPort->Mlme.CntlMachine.CurrState = CNTL_WAIT_REASSOC;
    }
    else // no more BSS
    {
        DBGPRINT(RT_DEBUG_TRACE, ("CNTL - All fast roaming failed, back to ch #%d\n",pPort->Channel));
        //AsicSwitchChannel(pAd, pPort->Channel,FALSE);
        AsicSwitchChannel(pPort, pPort->Channel, pPort->CentralChannel, pPort->BBPCurrentBW, FALSE);
        pPort->Mlme.CntlMachine.CurrState = CNTL_IDLE;
    } 
}

/*
    ==========================================================================
    Description:

    IRQL = DISPATCH_LEVEL
    
    ==========================================================================
*/
VOID MlmeCntJoinParmFill(
    IN PMP_ADAPTER pAd, 
    IN OUT MLME_JOIN_REQ_STRUCT *JoinReq, 
    IN ULONG BssIdx) 
{
    JoinReq->BssIdx = BssIdx;
}

/*
    ==========================================================================
    Description:

    IRQL = DISPATCH_LEVEL
    
    ==========================================================================
*/
VOID MlmeCntAssocParmFill(
    IN PMP_ADAPTER pAd, 
    IN OUT MLME_ASSOC_REQ_STRUCT *AssocReq, 
    IN PUCHAR                     pAddr, 
    IN USHORT                     CapabilityInfo, 
    IN ULONG                      Timeout, 
    IN USHORT                     ListenIntv) 
{
    COPY_MAC_ADDR(AssocReq->Addr, pAddr);
    // Add mask to support 802.11b mode only
    AssocReq->CapabilityInfo = CapabilityInfo & SUPPORTED_CAPABILITY_INFO; // not cf-pollable, not cf-poll-request
    AssocReq->Timeout = Timeout;
    AssocReq->ListenIntv = ListenIntv;
}

/*
    ==========================================================================
    Description:

    IRQL = DISPATCH_LEVEL
    
    ==========================================================================
*/
VOID MlmeCntScanParmFill(
    IN PMP_ADAPTER pAd, 
    IN OUT MLME_SCAN_REQ_STRUCT *ScanReq, 
    IN CHAR Ssid[], 
    IN UCHAR SsidLen, 
    IN UCHAR BssType, 
    IN UCHAR ScanType) 
{
    ScanReq->Dot11Ssid[0].uSSIDLength = SsidLen;
    PlatformMoveMemory(ScanReq->Dot11Ssid[0].ucSSID, Ssid, SsidLen);
    ScanReq->NumOfdot11SSIDs = 1;
    ScanReq->BssType = BssType;
    ScanReq->ScanType = ScanType;
}

//
// Bit 0 in the RSN Capabilities field.
// An AP sets the Pre-Authentication subfield of the RSN Capabilities field to 1 to signal it supports preauthentication, 
// and set the subfield to 0 when it does not support preauthentication.
//
#define RSNA_CAPABILITY_PRE_AUTH 0x01

//
// Request PMKIDs for BSS that the 802.11 station can potentially roam to.
//
VOID MlmeCntCheckPMKIDCandidate(
    IN PMP_PORT pPort)
{
    PDOT11_PMKID_CANDIDATE_LIST_PARAMETERS PMKIDParam = NULL;
    UCHAR Buffer[sizeof(DOT11_PMKID_CANDIDATE_LIST_PARAMETERS) + 
                       (STA_PMKID_MAX_COUNT * sizeof(DOT11_BSSID_CANDIDATE))] = {0};
    ULONG Count = 0, Size = 0;
    ULONG index1 = 0, index2 = 0;
    BOOLEAN bNeedIndicateAgain = FALSE;
    PBSS_ENTRY pBssEntry = NULL;
    PDOT11_BSSID_CANDIDATE pPMKIDCandidate = NULL;
    PMP_ADAPTER pAd = pPort->pAd;

    do {
        // Check the PMKID candidate list if and only if (a) link up, (b) instrastructure mode and (c) RSNA authentication.
        if ((!OPSTATUS_TEST_FLAG(pPort, fOP_STATUS_MEDIA_STATE_CONNECTED)) || 
            (!INFRA_ON(pPort)) || 
            (pPort->PortCfg.AuthMode != Ralink802_11AuthModeWPA2))
        {
            // do nothing.
            break;
        }

        // Get the desired AP candidates.
        BssTableSsidSort(pAd, pPort, &pAd->MlmeAux.SsidBssTab, pPort->PortCfg.Ssid, pPort->PortCfg.SsidLen);

        //
        // Add the latest disassociated BSS entry that the STA can potentially roam to
        //
        if (pAd->MlmeAux.SsidBssTab.BssNr < MAX_LEN_OF_BSS_TABLE)
        {
            ULONG latestBSSEntry = BSS_NOT_FOUND;

            latestBSSEntry = BssTableSearch(pAd, pPort, &pAd->MlmeAux.SsidBssTab, pPort->CommonCfg.LatestBSSEntry.Bssid, pPort->CommonCfg.LatestBSSEntry.Channel);
            
            if ((latestBSSEntry == BSS_NOT_FOUND) && (!MAC_ADDR_EQUAL(pPort->CommonCfg.LatestBSSEntry.Bssid, ZERO_MAC_ADDR)))
            {
                PlatformMoveMemory(&pAd->MlmeAux.SsidBssTab.BssEntry[pAd->MlmeAux.SsidBssTab.BssNr], &pPort->CommonCfg.LatestBSSEntry, sizeof (BSS_ENTRY));
                pAd->MlmeAux.SsidBssTab.BssNr++;

                DBGPRINT(RT_DEBUG_TRACE, ("%s: Latest BSSID, BSSID = %02X:%02X:%02X:%02X:%02X:%02X\n", 
                    __FUNCTION__, 
                    pPort->CommonCfg.LatestBSSEntry.Bssid[0], pPort->CommonCfg.LatestBSSEntry.Bssid[1], pPort->CommonCfg.LatestBSSEntry.Bssid[2], 
                    pPort->CommonCfg.LatestBSSEntry.Bssid[3], pPort->CommonCfg.LatestBSSEntry.Bssid[4], pPort->CommonCfg.LatestBSSEntry.Bssid[5]));
            }
        }

        if (pAd->MlmeAux.SsidBssTab.BssNr == 0)
        {
            break;
        }

        // If there is another new candidates, the station needs to indicate DOT11_PMKID_CANDIDATE_LIST_PARAMETERS.
        Count = pAd->MlmeAux.SsidBssTab.BssNr;

        if (Count > STA_PMKID_MAX_COUNT)
        {
            Count = STA_PMKID_MAX_COUNT;
        }

        // Decide the station needs to indicate DOT11_PMKID_CANDIDATE_LIST_PARAMETERS or not.
        if (Count == pPort->CommonCfg.PMKIDCache.Count)
        {
            bNeedIndicateAgain = FALSE;

            for (index1 = 0; index1 < Count; index1++)
            {
                pBssEntry = &pAd->MlmeAux.SsidBssTab.BssEntry[index1];
                pPMKIDCandidate = pPort->CommonCfg.PMKIDCache.Candidate;

                for (index2 = 0; index2 < pPort->CommonCfg.PMKIDCache.Count; index2++, pPMKIDCandidate++)
                {
                    if (PlatformEqualMemory(pBssEntry->Bssid, pPMKIDCandidate->BSSID, sizeof(DOT11_MAC_ADDRESS)) == 1)
                    {
                        // Already in the PMKID candidate list.
                        break;
                    }
                }

                // Not found in the old PMKID candidate list.
                if (index2 == pPort->CommonCfg.PMKIDCache.Count)
                {
                    bNeedIndicateAgain = TRUE;
                    break;
                }
            }
        }
        else
        {
            bNeedIndicateAgain = TRUE;
        }

        if (bNeedIndicateAgain == FALSE)
        {
            break;
        }

        // Prepare the DOT11_PMKID_CANDIDATE_LIST_PARAMETERS indication.
        pPMKIDCandidate = pPort->CommonCfg.PMKIDCache.Candidate;

        for (index1 = 0; index1 < Count; index1++)
        {
            pBssEntry = &pAd->MlmeAux.SsidBssTab.BssEntry[index1];

            // Copy this PMKID candidate.
            PlatformMoveMemory(pPMKIDCandidate->BSSID, pBssEntry->Bssid, sizeof(DOT11_MAC_ADDRESS));
            
            if (pBssEntry->WPA2.RsnCapability & RSNA_CAPABILITY_PRE_AUTH) // preauthentication
            {
                pPMKIDCandidate->uFlags = DOT11_PMKID_CANDIDATE_PREAUTH_ENABLED;
            }
            else
            {
                pPMKIDCandidate->uFlags = 0;
            }

            pPMKIDCandidate++;
        }

        // Update the number of the PMKID candidates.
        pPort->CommonCfg.PMKIDCache.Count = (ULONG)(pPMKIDCandidate - pPort->CommonCfg.PMKIDCache.Candidate);

        // The DOT11_PMKID_CANDIDATE_LIST_PARAMETERS indication.
        PMKIDParam = (PDOT11_PMKID_CANDIDATE_LIST_PARAMETERS)Buffer;
        MP_ASSIGN_NDIS_OBJECT_HEADER(PMKIDParam->Header, 
                                        NDIS_OBJECT_TYPE_DEFAULT,
                                        DOT11_PMKID_CANDIDATE_LIST_PARAMETERS_REVISION_1,
                                        sizeof(DOT11_PMKID_CANDIDATE_LIST_PARAMETERS));
        PMKIDParam->uCandidateListSize = pPort->CommonCfg.PMKIDCache.Count;
        PMKIDParam->uCandidateListOffset = sizeof(DOT11_PMKID_CANDIDATE_LIST_PARAMETERS);

        Size = pPort->CommonCfg.PMKIDCache.Count * sizeof(DOT11_BSSID_CANDIDATE);
        PlatformMoveMemory(Add2Ptr(PMKIDParam, sizeof(DOT11_PMKID_CANDIDATE_LIST_PARAMETERS)), 
                         pPort->CommonCfg.PMKIDCache.Candidate,
                         Size);

        PlatformIndicateDot11Status(pAd,
                            pPort, 
                            NDIS_STATUS_DOT11_PMKID_CANDIDATE_LIST,
                            NULL,
                            Buffer,
                            sizeof(DOT11_PMKID_CANDIDATE_LIST_PARAMETERS) + Size);

        DBGPRINT(RT_DEBUG_INFO,("%s: Indicate NDIS_STATUS_DOT11_PMKID_CANDIDATE_LIST.\n", __FUNCTION__));
        DBGPRINT(RT_DEBUG_INFO,("%s: pPort->CommonCfg.PMKIDCache.Count = %d\n", __FUNCTION__, pPort->CommonCfg.PMKIDCache.Count));

        for (index1 = 0; index1 < pPort->CommonCfg.PMKIDCache.Count; index1++)
        {
            pPMKIDCandidate = &pPort->CommonCfg.PMKIDCache.Candidate[index1];

            DBGPRINT(RT_DEBUG_INFO,("%s: BSSID = %02X:%02X:%02X:%02X:%02X:%02X; Preautentication = %d\n", 
                __FUNCTION__, 
                pPMKIDCandidate->BSSID[0], pPMKIDCandidate->BSSID[1], pPMKIDCandidate->BSSID[2], 
                pPMKIDCandidate->BSSID[3], pPMKIDCandidate->BSSID[4], pPMKIDCandidate->BSSID[5], 
                pPMKIDCandidate->uFlags));

            pPMKIDCandidate++;
        }
    } while(FALSE);
}

/*
    ==========================================================================
    Description:

    IRQL = DISPATCH_LEVEL
    
    ==========================================================================
*/
VOID MlmeCntDlsParmFill(
    IN PMP_ADAPTER pAd, 
    IN OUT MLME_DLS_REQ_STRUCT *pDlsReq,
    IN PRT_802_11_DLS pDls,
    IN USHORT reason) 
{
    pDlsReq->pDLS = pDls;
    pDlsReq->Reason = reason;
}

/*
    ==========================================================================
    Description:

    IRQL = DISPATCH_LEVEL
    
    ==========================================================================
*/
VOID MlmeCntTdlsParmFill(
    IN PMP_ADAPTER pAd, 
    IN OUT MLME_TDLS_REQ_STRUCT *pTdlsReq,
    IN PRT_802_11_TDLS pTdls,
    IN USHORT Reason) 
{
    pTdlsReq->pTDLS = pTdls;
    pTdlsReq->Reason = Reason;
}

/*
    ==========================================================================
    Description:

    IRQL = DISPATCH_LEVEL
    
    ==========================================================================
*/
VOID MlmeCntDisassocParmFill(
    IN PMP_ADAPTER pAd, 
    IN OUT MLME_DISASSOC_REQ_STRUCT *DisassocReq, 
    IN PUCHAR pAddr, 
    IN USHORT Reason) 
{
    COPY_MAC_ADDR(DisassocReq->Addr, pAddr);
    DisassocReq->Reason = Reason;
}

/*
    ==========================================================================
    Description:

    IRQL = DISPATCH_LEVEL
    
    ==========================================================================
*/
VOID MlmeCntStartParmFill(
    IN PMP_PORT pPort, 
    IN OUT MLME_START_REQ_STRUCT *StartReq, 
    IN CHAR Ssid[], 
    IN UCHAR SsidLen) 
{
    if (SsidLen <= MAX_LEN_OF_SSID)
    {
        PlatformMoveMemory(StartReq->Ssid, Ssid, SsidLen); 
        StartReq->SsidLen = SsidLen;
    }
    else
    {
        PlatformMoveMemory(StartReq->Ssid, Ssid, MAX_LEN_OF_SSID); 
        StartReq->SsidLen = MAX_LEN_OF_SSID;
    }
}

/*
    ==========================================================================
    Description:

    IRQL = DISPATCH_LEVEL
    
    ==========================================================================
*/
VOID MlmeCntAuthParmFill(
    IN PMP_PORT pPort, 
    IN OUT MLME_AUTH_REQ_STRUCT *AuthReq, 
    IN PUCHAR pAddr, 
    IN USHORT Alg) 
{
    COPY_MAC_ADDR(AuthReq->Addr, pAddr);
    AuthReq->Alg = Alg;
    AuthReq->Timeout = AUTH_TIMEOUT;
    DBGPRINT(RT_DEBUG_TRACE, ("MlmeCntAuthParmFill: Alg = %d (0:open:1:Shared)\n", AuthReq->Alg));
}

/*
    ==========================================================================
    Description:

    IRQL = DISPATCH_LEVEL
    
    ==========================================================================
 */
VOID MlmeCntComposePsPoll(
    IN PMP_ADAPTER pAd,
    IN PMP_PORT pPort)
{
//  PTXINFO_STRUC   pTxInfo;
//  PTXWI_STRUC     pTxWI;
    
    DBGPRINT(RT_DEBUG_TRACE, ("MlmeCntComposePsPoll\n"));
    PlatformZeroMemory(&pPort->PsPollFrame, sizeof(PSPOLL_FRAME));
    //pPort->PsPollFrame.FC.ToDs = 1;
    //Set PM bit in ps-poll, to fix WLK PowerSaveMode_ext failure issue.
    pPort->PsPollFrame.FC.PwrMgmt = 1;
    pPort->PsPollFrame.FC.Type = BTYPE_CNTL;
    pPort->PsPollFrame.FC.SubType = SUBTYPE_PS_POLL;
    pPort->PsPollFrame.Aid = pAd->MlmeAux.Aid | 0xC000;
    COPY_MAC_ADDR(pPort->PsPollFrame.Bssid, pPort->PortCfg.Bssid);
    COPY_MAC_ADDR(pPort->PsPollFrame.Ta, pPort->CurrentAddress);

    DBGPRINT(RT_DEBUG_TRACE, ("MlmeCntComposePsPoll : pAd->MlmeAux.Aid = 0x%x, pPort->PsPollFrame.Aid = 0x%x\n", pAd->MlmeAux.Aid, pPort->PsPollFrame.Aid));
    DBGPRINT(RT_DEBUG_TRACE, ("MlmeCntComposePsPoll : pPort->PsPollFrame.Bssid = %02x %02x %02x %02x %02x %02x\n", pPort->PsPollFrame.Bssid[0], pPort->PsPollFrame.Bssid[1], pPort->PsPollFrame.Bssid[2], pPort->PsPollFrame.Bssid[3], pPort->PsPollFrame.Bssid[4], pPort->PsPollFrame.Bssid[5]));
    DBGPRINT(RT_DEBUG_TRACE, ("MlmeCntComposePsPoll : pPort->PsPollFrame.Ta = %02x %02x %02x %02x %02x %02x\n", pPort->PsPollFrame.Ta[0], pPort->PsPollFrame.Ta[1], pPort->PsPollFrame.Ta[2], pPort->PsPollFrame.Ta[3], pPort->PsPollFrame.Ta[4], pPort->PsPollFrame.Ta[5]));

#if 0
    PlatformZeroMemory(&pAd->pHifCfg->PsPollContext.TransferBuffer->WirelessPacket[0], 100);
    pTxInfo = (PTXINFO_STRUC)&pAd->pHifCfg->PsPollContext.TransferBuffer->WirelessPacket[0];
    XmitWriteTxInfo(pAd, pPort, pTxInfo, (USHORT)(sizeof(PSPOLL_FRAME)+pAd->HwCfg.TXWI_Length), TRUE, EpToQueue[MGMTPIPEIDX], FALSE,  FALSE);
    pTxWI = (PTXWI_STRUC)&pAd->pHifCfg->PsPollContext.TransferBuffer->WirelessPacket[TXINFO_SIZE];
    XmitWriteTxWI(pAd, pPort, pTxWI,  FALSE, FALSE, FALSE, TRUE, FALSE, 0, RESERVED_WCID, (sizeof(PSPOLL_FRAME)),
        0, 0, IFS_HTTXOP, FALSE ,pPort->CommonCfg.MgmtPhyCfg, FALSE, TRUE, FALSE);

    PlatformMoveMemory(&pAd->pHifCfg->PsPollContext.TransferBuffer->WirelessPacket[pAd->HwCfg.TXWI_Length+TXINFO_SIZE], &pPort->PsPollFrame, sizeof(PSPOLL_FRAME));
    // Append 4 extra zero bytes.
    pAd->pHifCfg->PsPollContext.BulkOutSize =  TXINFO_SIZE + pAd->HwCfg.TXWI_Length + sizeof(PSPOLL_FRAME) + 4;
#endif  
}

// IRQL = DISPATCH_LEVEL
VOID MlmeCntComposeNullFrame(
    IN PMP_ADAPTER pAd,
    IN PMP_PORT   pPort)
{
//  PTXINFO_STRUC   pTxInfo;
//  PTXWI_STRUC     pTxWI;
    
    PlatformZeroMemory(&pPort->NullFrame, sizeof(HEADER_802_11));
    pPort->NullFrame.FC.Type = BTYPE_DATA;
    pPort->NullFrame.FC.SubType = SUBTYPE_NULL_FUNC;
    pPort->NullFrame.FC.ToDs = 1;
    COPY_MAC_ADDR(pPort->NullFrame.Addr1, pPort->PortCfg.Bssid);
    COPY_MAC_ADDR(pPort->NullFrame.Addr2, pPort->CurrentAddress);
    COPY_MAC_ADDR(pPort->NullFrame.Addr3, pPort->PortCfg.Bssid);
#if 0
    PlatformZeroMemory(&pAd->pHifCfg->NullContext.TransferBuffer->WirelessPacket[0], 100);
    pTxInfo = (PTXINFO_STRUC)&pAd->pHifCfg->NullContext.TransferBuffer->WirelessPacket[0];
    XmitWriteTxInfo(pAd, pPort, pTxInfo, (USHORT)(sizeof(HEADER_802_11)+pAd->HwCfg.TXWI_Length), TRUE, EpToQueue[MGMTPIPEIDX], FALSE,  FALSE);
    pTxWI = (PTXWI_STRUC)&pAd->pHifCfg->NullContext.TransferBuffer->WirelessPacket[TXINFO_SIZE];
    XmitWriteTxWI(pAd, pPort, pTxWI,  FALSE, FALSE, FALSE, TRUE, FALSE, 0, BSSID_WCID, (sizeof(HEADER_802_11)),
        0, 0, IFS_HTTXOP, FALSE ,pPort->CommonCfg.MgmtPhyCfg, FALSE, TRUE, FALSE);

    PlatformMoveMemory(&pAd->pHifCfg->NullContext.TransferBuffer->WirelessPacket[pAd->HwCfg.TXWI_Length+TXINFO_SIZE], &pPort->NullFrame, sizeof(HEADER_802_11));
    pAd->pHifCfg->NullContext.BulkOutSize =  TXINFO_SIZE + pAd->HwCfg.TXWI_Length + sizeof(pPort->NullFrame) + 4;
#endif
}
