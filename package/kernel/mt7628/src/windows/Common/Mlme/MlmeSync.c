/*
 ***************************************************************************
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
 ***************************************************************************

    Module Name:
    sync.c

    Abstract:

    Revision History:
    Who         When            What
    --------    ----------      ----------------------------------------------
*/
#include "MtConfig.h"

#define DRIVER_FILE         0x00C00000

// 2.4 Ghz channel plan index in the TxPower arrays.
#define BG_BAND_REGION_0_START  0           // 1,2,3,4,5,6,7,8,9,10,11  
#define BG_BAND_REGION_0_SIZE   11
#define BG_BAND_REGION_1_START  0           // 1,2,3,4,5,6,7,8,9,10,11,12,13
#define BG_BAND_REGION_1_SIZE   13
#define BG_BAND_REGION_2_START  9           // 10,11
#define BG_BAND_REGION_2_SIZE   2
#define BG_BAND_REGION_3_START  9           // 10,11,12,13
#define BG_BAND_REGION_3_SIZE   4
#define BG_BAND_REGION_4_START  13          // 14
#define BG_BAND_REGION_4_SIZE   1
#define BG_BAND_REGION_5_START  0           // 1,2,3,4,5,6,7,8,9,10,11,12,13,14 
#define BG_BAND_REGION_5_SIZE   14
#define BG_BAND_REGION_6_START  2           // 3,4,5,6,7,8,9
#define BG_BAND_REGION_6_SIZE   7
#define BG_BAND_REGION_7_START  4           // 5,6,7,8,9,10,11,12,13
#define BG_BAND_REGION_7_SIZE   9
#define BG_BAND_REGION_31_START 0           // 1,2,3,4,5,6,7,8,9,10,11,12,13,14 
#define BG_BAND_REGION_31_SIZE  14
#define BG_BAND_REGION_32_START 0           // 1,2,3,4,5,6,7,8,9,10,11,12,13
#define BG_BAND_REGION_32_SIZE  13
#define BG_BAND_REGION_33_START 0           // 1,2,3,4,5,6,7,8,9,10,11,12,13,14
#define BG_BAND_REGION_33_SIZE  14

#define ADHOC_ENTRY_BEACON_UPDATE_TIME  5   // 5 sec = ADHOC_ENTRY_BEACON_UPDATE_TIME * ONE_SECOND_TIME

// 5 Ghz channel plan index in the TxPower arrays.
UCHAR BG_BAND_REGION_WHQL_CHANNEL_LIST[]={1, 3, 6, 8, 11, 2, 4, 5, 7, 9, 10};
UCHAR BG_BAND_REGION_0_CHANNEL_LIST[]={1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11};
UCHAR BG_BAND_REGION_1_CHANNEL_LIST[]={1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13};
UCHAR BG_BAND_REGION_2_CHANNEL_LIST[]={10, 11};
UCHAR BG_BAND_REGION_3_CHANNEL_LIST[]={10, 11, 12, 13};
UCHAR BG_BAND_REGION_4_CHANNEL_LIST[]={14};
UCHAR BG_BAND_REGION_5_CHANNEL_LIST[]={1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14};
UCHAR BG_BAND_REGION_6_CHANNEL_LIST[]={3, 4, 5, 6, 7, 8, 9};
UCHAR BG_BAND_REGION_7_CHANNEL_LIST[]={5, 6, 7, 8, 9, 10, 11, 12, 13};
UCHAR BG_BAND_REGION_31_CHANNEL_LIST[]={1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14};
UCHAR BG_BAND_REGION_32_CHANNEL_LIST[]={1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13};
UCHAR BG_BAND_REGION_33_CHANNEL_LIST[]={1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14};


// 5 Ghz channel plan index in the TxPower arrays.
UCHAR A_BAND_REGION_0_CHANNEL_LIST[]={36, 40, 44, 48, 52, 56, 60, 64, 149, 153, 157, 161, 165};
UCHAR A_BAND_REGION_1_CHANNEL_LIST[]={36, 40, 44, 48, 52, 56, 60, 64, 100, 104, 108, 112, 116, 120, 124, 128, 132, 136, 140};
UCHAR A_BAND_REGION_2_CHANNEL_LIST[]={36, 40, 44, 48, 52, 56, 60, 64};
UCHAR A_BAND_REGION_3_CHANNEL_LIST[]={52, 56, 60, 64, 149, 153, 157, 161};
UCHAR A_BAND_REGION_4_CHANNEL_LIST[]={149, 153, 157, 161, 165};
UCHAR A_BAND_REGION_5_CHANNEL_LIST[]={149, 153, 157, 161};
UCHAR A_BAND_REGION_6_CHANNEL_LIST[]={36, 40, 44, 48};
UCHAR A_BAND_REGION_7_CHANNEL_LIST[]={36, 40, 44, 48, 52, 56, 60, 64, 100, 104, 108, 112, 116, 120, 124, 128, 132, 136, 140, 149, 153, 157, 161, 165};
UCHAR A_BAND_REGION_8_CHANNEL_LIST[]={52, 56, 60, 64};
UCHAR A_BAND_REGION_9_CHANNEL_LIST[]={36, 40, 44, 48, 52, 56, 60, 64, 100, 104, 108, 112, 116, 132, 136, 140, 149, 153, 157, 161, 165};
UCHAR A_BAND_REGION_10_CHANNEL_LIST[]={36, 40, 44, 48,149, 153, 157, 161, 165};
UCHAR A_BAND_REGION_11_CHANNEL_LIST[]={36, 40, 44, 48, 52, 56, 60, 64, 100, 104, 108, 112, 116, 120, 149, 153, 157, 161};
UCHAR A_BAND_REGION_12_CHANNEL_LIST[]={36, 40, 44, 48, 52, 56, 60, 64, 100, 104, 108, 112, 116, 120, 124, 128, 132, 136, 140};
UCHAR A_BAND_REGION_13_CHANNEL_LIST[]={52, 56, 60, 64, 100, 104, 108, 112, 116, 120, 124, 128, 132, 136, 140, 149, 153, 157, 161};
UCHAR A_BAND_REGION_14_CHANNEL_LIST[]={36, 40, 44, 48, 52, 56, 60, 64, 100, 104, 108, 112, 116, 136, 140, 149, 153, 157, 161, 165};
UCHAR A_BAND_REGION_15_CHANNEL_LIST[]={149, 153, 157, 161, 165, 169, 173};
UCHAR A_BAND_REGION_16_CHANNEL_LIST[]={36, 40, 44, 48, 52, 56, 60, 64, 100, 104, 108, 112, 116, 120, 124, 128, 132, 136, 140, 149, 153, 157, 161, 165, 169, 173};
UCHAR A_BAND_REGION_17_CHANNEL_LIST[]={36, 40, 44, 48, 52, 56, 60, 64, 132, 136, 140};

//UCHAR A_BAND_CHANNEL_MAPPING_LIST[]={36,44,52,60,100,108,116,124,132,149,157};        

//BaSizeArray follows the 802.11n definition as MaxRxFactor.  2^(13+factor) bytes. When factor =0, it's about Ba buffer size =8.
UCHAR   BaSizeArray[4] = {8,16,32,64};
extern UCHAR    OutMsgBuf;      // buffer to create message contents

#if(COMPILE_WIN8_ABOVE(CURRENT_OS_NDIS_VER))  
// Use this golbal buffer to avoid race condition
UCHAR   ProbeReqBuffer[512];
#endif /*NDIS630_MINIPORT*/
/*
    ==========================================================================
    Description:
        The sync state machine, 
    Parameters:
        Sm - pointer to the state machine
    Note:
        the state machine looks like the following

    ==========================================================================
 */
VOID MlmeSyncStateMachineInit(
    IN  PMP_ADAPTER       pAd, 
    IN  STATE_MACHINE       *Sm, 
    OUT STATE_MACHINE_FUNC  Trans[]) 
{
    StateMachineInit(Sm, Trans, MAX_SYNC_STATE, MAX_SYNC_MSG, Drop, SYNC_IDLE, SYNC_MACHINE_BASE);

    // column 1
    StateMachineSetAction(Sm, SYNC_IDLE, MT2_MLME_SCAN_REQ, MlmeSyncScanReqAction);
    StateMachineSetAction(Sm, SYNC_IDLE, MT2_MLME_JOIN_REQ, MlmeSyncJoinReqAction);
    StateMachineSetAction(Sm, SYNC_IDLE, MT2_MLME_START_REQ, MlmeSyncStartReqAction);
    StateMachineSetAction(Sm, SYNC_IDLE, MT2_PEER_BEACON, MlmeSyncPeerBeacon);
    StateMachineSetAction(Sm, SYNC_IDLE, MT2_PEER_PROBE_REQ, MlmeSyncPeerProbeReqAction); 
    StateMachineSetAction(Sm, SYNC_IDLE, MT2_PEER_PROBE_RSP, MlmeSyncPeerBeacon);

    //column 2
    StateMachineSetAction(Sm, JOIN_WAIT_BEACON, MT2_MLME_SCAN_REQ, MlmeSyncInvalidStateWhenScan);
    StateMachineSetAction(Sm, JOIN_WAIT_BEACON, MT2_MLME_JOIN_REQ, MlmeSyncInvalidStateWhenJoin);
    StateMachineSetAction(Sm, JOIN_WAIT_BEACON, MT2_MLME_START_REQ, MlmeSyncInvalidStateWhenStart);
    StateMachineSetAction(Sm, JOIN_WAIT_BEACON, MT2_PEER_BEACON, MlmeSyncMlmeSyncPeerBeaconAtJoinAction);
    StateMachineSetAction(Sm, JOIN_WAIT_BEACON, MT2_BEACON_TIMEOUT, MlmeSyncBeaconTimeoutAtJoinAction);
    StateMachineSetAction(Sm, JOIN_WAIT_BEACON, MT2_PEER_PROBE_RSP, MlmeSyncMlmeSyncPeerBeaconAtJoinAction);

    // column 3
    StateMachineSetAction(Sm, SCAN_LISTEN, MT2_MLME_SCAN_REQ, /*MlmeSyncInvalidStateWhenScan*/MlmeSyncScanReqAction2);
    StateMachineSetAction(Sm, SCAN_LISTEN, MT2_MLME_JOIN_REQ, MlmeSyncInvalidStateWhenJoin);
    StateMachineSetAction(Sm, SCAN_LISTEN, MT2_MLME_START_REQ, MlmeSyncInvalidStateWhenStart);
    StateMachineSetAction(Sm, SCAN_LISTEN, MT2_PEER_BEACON, MlmeSyncMlmeSyncPeerBeaconAtScanAction);
    StateMachineSetAction(Sm, SCAN_LISTEN, MT2_PEER_PROBE_RSP, MlmeSyncMlmeSyncPeerBeaconAtScanAction);
    StateMachineSetAction(Sm, SCAN_LISTEN, MT2_SCAN_TIMEOUT, MlmeSyncScanTimeoutAction);
    StateMachineSetAction(Sm, SCAN_LISTEN, MT2_PEER_PROBE_REQ, MlmeSyncPeerReqAtScanAction); 
    StateMachineSetAction(Sm, SCAN_LISTEN, MT2_RESTORE_CHANNEL_TIMEOUT, MlmeSyncRestoreChannelTimeoutAction);       
}

/* 
    ==========================================================================
    Description:
        Becaon timeout handler, executed in timer thread

    IRQL = DISPATCH_LEVEL

    ==========================================================================
 */
VOID MlmeSyncBeaconTimeoutTimerCallback(
    IN PVOID SystemSpecific1, 
    IN PVOID FunctionContext, 
    IN PVOID SystemSpecific2, 
    IN PVOID SystemSpecific3) 
{
    PMP_PORT pPort = (PMP_PORT )FunctionContext;
    PMP_ADAPTER  pAd = pPort->pAd;

	DBGPRINT(RT_DEBUG_TRACE,("SYNC - BeaconTimeout\n"));

    if (pPort == NULL)
    {
        DBGPRINT(RT_DEBUG_ERROR, ("%s: pPort can't be NULL", __FUNCTION__));
        return;
    }

    if (pPort->bStartJoin == TRUE)
    {
        pPort->bStartJoin = FALSE;
    }

    // Do nothing if the driver is starting halt state.
    // This might happen when timer already been fired before cancel timer with mlmehalt
    if (MT_TEST_FLAG(pAd, fRTMP_ADAPTER_HALT_IN_PROGRESS))
        return;

    MlmeEnqueue(pAd,pPort, SYNC_STATE_MACHINE, MT2_BEACON_TIMEOUT, 0, NULL);
}

VOID MlmeSyncRestoreChannelTimeoutTimerCallback(
    IN PVOID SystemSpecific1, 
    IN PVOID FunctionContext, 
    IN PVOID SystemSpecific2, 
    IN PVOID SystemSpecific3) 
{
    PMP_PORT pPort = (PMP_PORT )FunctionContext;
    PMP_ADAPTER  pAd = pPort->pAd;

    DBGPRINT(RT_DEBUG_INFO,("SYNC - Restore channel timeout \n"));  
    if(pPort == NULL)
        return;
    pPort->ScaningChannel = pAd->MlmeAux.LastScanChannel;
    MlmeEnqueue(pAd,pPort, SYNC_STATE_MACHINE, MT2_RESTORE_CHANNEL_TIMEOUT, 0, NULL);
}

/* 
    ==========================================================================
    Description:
        Scan timeout handler, executed in timer thread

    IRQL = DISPATCH_LEVEL

    ==========================================================================
 */
VOID MlmeSyncScanTimeoutTimerCallback(
    IN PVOID SystemSpecific1, 
    IN PVOID FunctionContext, 
    IN PVOID SystemSpecific2, 
    IN PVOID SystemSpecific3) 
{
    PMP_PORT pPort = (PMP_PORT )FunctionContext;
    PMP_ADAPTER  pAd = pPort->pAd;

    DBGPRINT(RT_DEBUG_INFO,("SYNC - Scan Timeout \n"));
    
    // Do nothing if the driver is starting halt state.
    // This might happen when timer already been fired before cancel timer with mlmehalt
    if (MT_TEST_FLAG(pAd, fRTMP_ADAPTER_HALT_IN_PROGRESS))
        return;
    if(pPort == NULL)
        return;

    if(PlatformScheduleWorktiem(&pPort->Mlme.ScanTimeoutWorkitem) == FALSE)
    {
        PlatformSetTimer(pPort, &pPort->Mlme.ScanTimer, 30);
    }
    
}

/* 
    ==========================================================================
    Description:
        MLME SCAN req state machine procedure
    ==========================================================================
 */
VOID MlmeSyncScanReqAction(
    IN PMP_ADAPTER pAd, 
    IN MLME_QUEUE_ELEM *Elem) 
{
    //UCHAR                 BBPValue;
    UCHAR                   ScanType;
    BOOLEAN                 TimerCancelled; 
    ULONGLONG               Now;
    PMP_PORT              pPort = pAd->PortList[Elem->PortNum];
    PMLME_SCAN_REQ_STRUCT   pScanReq = (PMLME_SCAN_REQ_STRUCT) Elem->Msg;
    NDIS_STATUS                 ndisStatus = NDIS_STATUS_SUCCESS;
    //ULONG             Value;

    DBGPRINT(RT_DEBUG_TRACE, ("-----> %s \n",__FUNCTION__));
    //
    // If we do OID_DOT11_FLUSH_BSS_LIST before, then we can't skip this scan request.
    // Otherwise, pAd->ScanTab will be Empty.
    //

    // Scan request action might spend more time(from-set-to-clear) for 11ac usb. 
    // For example, 7610U(11ac usb) needs 30~80ms.
    MT_SET_FLAG(pAd, fRTMP_ADAPTER_SCAN_REQ_ACTION_IN_PROGRESS);
    
    if(MT_TEST_FLAG(pAd, fRTMP_ADAPTER_IDLE_RADIO_OFF))
    {
        AsicRadioOn(pAd);
    }

    ScanType = pScanReq->ScanType;

    // Suspend MSDU transmission here
    MTSuspendMsduTransmission(pAd);

#ifdef MULTI_CHANNEL_SUPPORT    
    pAd->MccCfg.SwQSelect = NUM_OF_TX_RING;

    if (pAd->MccCfg.MultiChannelEnable == FALSE)
    {
#if 0 // need to check  
        i = 0;
        do
        {
            i++;
            RTUSBReadMACRegister(pAd, TXRXQ_PCNT, &Value);
            DBGPRINT(RT_DEBUG_TRACE, ("%s : [%d] EDCA(%d)\n", __FUNCTION__, i, ((Value >> 16) & 0xff)));

            if (((Value >> 16) & 0xff) != 0)
                Delay_us(100);  //Delay while TXQ not empty.
            else
                break;

        } while (i < 200);      
#endif      
    }   

    // Delay for 20ms
//  NdisCommonGenericDelay(20000);
#endif /*MULTI_CHANNEL_SUPPORT*/

    pAd->ucScanPortNum = pPort->PortNumber;

    NdisGetCurrentSystemTime((PLARGE_INTEGER)&Now);
    pAd->StaCfg.LastScanTime = Now;

    // reset all the timers
    PlatformCancelTimer(&pPort->Mlme.BeaconTimer, &TimerCancelled);
    PlatformCancelTimer(&pPort->Mlme.ScanTimer, &TimerCancelled);

    // Record parameters
    pAd->MlmeAux.BssType = pScanReq->BssType;
    pAd->MlmeAux.ScanType = pScanReq->ScanType;
	PlatformMoveMemory(&pAd->MlmeAux.Dot11Ssid, &pScanReq->Dot11Ssid, STA_MAX_SCAN_SSID_LIST_COUNT * sizeof(DOT11_SSID));
    pAd->MlmeAux.NumOfdot11SSIDs = pScanReq->NumOfdot11SSIDs;

    // Find the first channel
    if ((pPort->CommonCfg.bCarrierDetect) && (pPort->CommonCfg.bCarrierAssert))
    {
        // carrier assertion, no full channel scanning, only scan current channel
        pPort->ScaningChannel = pPort->Channel;
    }
    // Scanning was pending (for fast-roaming)
    else if ((pAd->StaCfg.bFastRoamingScan) && (pPort->Mlme.SyncMachine.CurrState == SCAN_PENDING))
    {
        pPort->ScaningChannel = pAd->StaCfg.LastScanChannel;
    }
    else if (IS_P2P_SEARCHING(pPort))
    {
        pPort->ScaningChannel = pPort->P2PCfg.P2pProprietary.ListenChanel[0];
    }
    else if (IS_P2P_LISTENING(pPort))
    {
        pPort->ScaningChannel = pPort->P2PCfg.ListenChannel;
    }
    else if ((pPort->P2PCfg.P2PConnectState == P2P_DO_GO_SCAN_OP_BEGIN) && (pPort->P2PCfg.GroupOpChannel != 0))
    {
        // start from the first channel
        pPort->ScaningChannel = pPort->P2PCfg.GroupOpChannel;
    }
    else if ((pPort->P2PCfg.P2PConnectState == P2P_DO_GO_NEG_DONE_CLIENT) && (pPort->P2PCfg.GroupOpChannel != 0))
    {
        // start from the first channel
        pPort->ScaningChannel = pPort->P2PCfg.GroupOpChannel;
    }
    else
    {
        if(P2P_ON(pPort) && ((ScanType == SCAN_P2P) || (ScanType == SCAN_P2P_PASSIVE)))
            pPort->ScaningChannel = pPort->P2PCfg.P2pProprietary.ListenChanel[0];
        else    // start from the first channel
            pPort->ScaningChannel = MlmeSyncFirstChannel(pAd);
    }   
    

    if (OPSTATUS_TEST_FLAG(pPort, fOP_STATUS_MEDIA_STATE_CONNECTED) && (INFRA_ON(pPort)))   
    {
        AsicSendNullFrame(pAd,pPort, pPort->Channel, PWR_SAVE);
    }

    //disable beacon before scan
    if ((pPort->SoftAP.bAPStart) || (pPort->P2PCfg.bGOStart))
    {
        AsicPauseBssSync(pAd);
    }

#ifdef NDIS630_MINIPORT
    // P2pMs scan only for operational channel
    if ((ScanType == FAST_SCAN_ACTIVE) && (pPort->P2PCfg.P2pMsSatetPhase == P2pMs_STATE_PROVISIONING))
    {
        P2pMsScanOpChannelDefault(pAd, pPort);
    }
#endif

    
    DBGPRINT(RT_DEBUG_TRACE, ("SYNC - BBP R4 to 20MHz.l\n"));
    // Before scan, reset trigger event table.
    TriEventInit(pAd, pPort);
    pAd->MlmeAux.MaxScanChannelCnt = 0;
    MlmeSyncScanMlmeSyncNextChannel(pAd,pPort);
    
    MT_CLEAR_FLAG(pAd, fRTMP_ADAPTER_SCAN_REQ_ACTION_IN_PROGRESS);
    DBGPRINT(RT_DEBUG_TRACE, ("<----- %s \n",__FUNCTION__));    
    
}

/*
    ==========================================================================
    Description:
        MLME JOIN req state machine procedure
    ==========================================================================
 */
VOID MlmeSyncJoinReqAction(
    IN PMP_ADAPTER pAd,
    IN MLME_QUEUE_ELEM *Elem)
{
    BSS_ENTRY       *pBss;
    BOOLEAN         TimerCancelled = FALSE;
	//HEADER_802_11	Hdr80211;
    NDIS_STATUS     NStatus;
    ULONG           FrameLen = 0;
    PUCHAR          pOutBuffer = NULL;
//  PUCHAR          pOutBuffer1 = NULL;
    PUCHAR          pSupRate = NULL;
    UCHAR           SupRateLen = 0;
    PUCHAR          pExtRate = NULL;
    UCHAR           ExtRateLen = 0;
//  ULONG           Value;
    //UCHAR         BBPValue;
    UCHAR           ASupRate[] = {0x8C, 0x12, 0x98, 0x24, 0xb0, 0x48, 0x60, 0x6C};
    UCHAR           ASupRateLen = sizeof(ASupRate)/sizeof(UCHAR);
    MLME_JOIN_REQ_STRUCT    *pInfo = (MLME_JOIN_REQ_STRUCT *)(Elem->Msg);
    PMP_PORT      pPort = pAd->PortList[Elem->PortNum];
	//PMP_PORT		pApPort;
	//ULONG Tmp;

    DBGPRINT(RT_DEBUG_TRACE, ("SYNC - MlmeSyncJoinReqAction(BSS #%d) port(%d)\n", pInfo->BssIdx, Elem->PortNum));

    // reset all the timers
    PlatformCancelTimer(&pPort->Mlme.ScanTimer, &TimerCancelled);
    PlatformCancelTimer(&pPort->Mlme.BeaconTimer, &TimerCancelled);

    pBss = &pAd->MlmeAux.SsidBssTab.BssEntry[pInfo->BssIdx];

    // record the desired SSID & BSSID we're waiting for
    COPY_MAC_ADDR(pAd->MlmeAux.Bssid, pBss->Bssid);
    PlatformMoveMemory(pAd->MlmeAux.Ssid, pBss->Ssid, pBss->SsidLen);
    pAd->MlmeAux.SsidLen = pBss->SsidLen;
    pAd->MlmeAux.BssType = pBss->BssType;
    pPort->Channel = pBss->Channel;
    pPort->CentralChannel = pBss->CentralChannel;

    //AsicSwitchChannel(pAd, pPort->Channel, FALSE);
    AsicSwitchChannel(pPort, pPort->Channel, pPort->CentralChannel, BW_20, FALSE);

    do
    {
        if ((pPort->CommonCfg.bIEEE80211H == TRUE) || (pPort->CommonCfg.bCarrierDetect == TRUE))
        {
            if (pBss->Hidden == TRUE)
            {
                // Send the ProbeReq packet.
                DBGPRINT(RT_DEBUG_INFO, ("SYNC - (802.11h and/or Carrier Detection is enabled.) Send ProbeReq packet for hidden AP  @ MlmeSyncJoinReqAction() ...\n" ));
            }
            else
            {
                if ((pPort->CommonCfg.bIEEE80211H == 1) && (pPort->Channel > 14) && (RadarChannelCheck(pAd, pPort->Channel) || (JapanChannelCheck(pPort->Channel))))
                {
                    //
                    // We can't send any Probe request frame to meet 802.11h.
                    //
                    break;
                }

                if (pPort->CommonCfg.bCarrierDetect)
                {
                    //
                    // We can't send any Probe request frame when carrier detection
                    //
                    break;
                }
            }
        }

        //
        // send probe request
        //
        NStatus = MlmeAllocateMemory(pAd, &pOutBuffer);  //Get an unused nonpaged memory
        if (NStatus == NDIS_STATUS_SUCCESS)
        {
            if (pPort->Channel <= 14)
            {
                // CH14 only supports 11b
                if (pPort->CommonCfg.Ch14BOnly && (pPort->Channel == CHANNEL_14))
                {
                    pSupRate = pPort->CommonCfg.SupRate;
                    SupRateLen = 4;
                    ExtRateLen = 0;
                }
                else
                {
                    pSupRate = pPort->CommonCfg.SupRate;
                    SupRateLen = pPort->CommonCfg.SupRateLen;
                    pExtRate = pPort->CommonCfg.ExtRate;
                    ExtRateLen = pPort->CommonCfg.ExtRateLen;
                }
            }
            else
            {
                //
                // Overwrite Support Rate, CCK rate are not allowed
                //
                pSupRate = ASupRate;
                SupRateLen = ASupRateLen;
                ExtRateLen = 0;
            }

#ifdef NDIS630_MINIPORT
            // p2p not support 11b rate
			if((pAd->pNicCfg->NdisVersion >= RTMP_NDIS630_VERSION) && pPort->PortType == WFD_CLIENT_PORT)
            {
                pSupRate = ASupRate;
                SupRateLen = ASupRateLen;
            }
#endif

            MgntPktConstructProbeReqFrame(pPort, pOutBuffer, &FrameLen);
           
            // To avoid that the first frame can't be sent out after radio-on,
            // send probe request twice to meet CCXv5 4.1.4.1 (for RT3572)
            NdisCommonMiniportMMRequest(pAd, pOutBuffer, FrameLen);

            pPort->bStartJoin = TRUE;

            //NdisCommonMiniportMMRequest(pAd, pOutBuffer, FrameLen);
            DBGPRINT(RT_DEBUG_INFO, ("SYNC - send ProbeReq @ channel=%d, Len=%d\n", pPort->Channel, FrameLen));
        }
    } while (FALSE);

    //
    // PlatformIndicateAssociationStart(pAd, pBss) will be done in MlmeSyncMlmeSyncPeerBeaconAtJoinAction()
    //

    if (pAd->MlmeAux.BssType == BSS_ADHOC)
    {
        if (pAd->MlmeAux.bStaCanRoam)
        {
            PlatformIndicateAdhocRoamingStart(pAd,pPort, pAd->MlmeAux.Bssid, pAd->MlmeAux.Ssid, pAd->MlmeAux.SsidLen);
            PlatformIndicateRoamingCompletion(pAd, pPort,DOT11_ASSOC_STATUS_SUCCESS);
        }
        else
        {
            PlatformIndicateConnectionStart(pAd, pPort,pAd->MlmeAux.BssType, pAd->MlmeAux.Bssid, pAd->MlmeAux.Ssid, pAd->MlmeAux.SsidLen);
            PlatformIndicateConnectionCompletion(pAd, pPort,DOT11_CONNECTION_STATUS_SUCCESS);
            pAd->MlmeAux.bStaCanRoam = TRUE;
            LedCtrlConnectionCompletion(pAd,pPort, TRUE);
        }
    }
    DBGPRINT(RT_DEBUG_TRACE, ("TONDIS:  MlmeSyncJoinReqAction, NDIS_STATUS_DOT11_ASSOCIATION_START [%02x:%02x:%02x:%02x:%02x:%02x]\n",
                pBss->Bssid[0], pBss->Bssid[1], pBss->Bssid[2], pBss->Bssid[3], pBss->Bssid[4], pBss->Bssid[5]));
    DBGPRINT(RT_DEBUG_TRACE, ("SYNC - Switch to ch %d, Wait BEACON from %02x:%02x:%02x:%02x:%02x:%02x\n",
        pBss->Channel, pBss->Bssid[0], pBss->Bssid[1], pBss->Bssid[2], pBss->Bssid[3], pBss->Bssid[4], pBss->Bssid[5]));

    PlatformSetTimer(pPort, &pPort->Mlme.BeaconTimer, JOIN_TIMEOUT);

    pPort->Mlme.SyncMachine.CurrState = JOIN_WAIT_BEACON; //Invoke MlmeSyncMlmeSyncPeerBeaconAtJoinAction
}

/* 
    ==========================================================================
    Description:
        MLME START Request state machine procedure, starting an IBSS
    ==========================================================================
 */
VOID MlmeSyncStartReqAction(
    IN PMP_ADAPTER pAd, 
    IN MLME_QUEUE_ELEM *Elem) 
{
    UCHAR         Ssid[MAX_LEN_OF_SSID], SsidLen; 
    BOOLEAN       TimerCancelled =FALSE;

    // New for WPA security suites
    PUCHAR                      VarIE = NULL;   // Total VIE length = MAX_VIE_LEN - -5
    NDIS_802_11_VARIABLE_IEs    *pVIE = NULL;
    LARGE_INTEGER               TimeStamp;
    BOOLEAN Privacy;
    USHORT Status;
#ifdef  SINGLE_ADHOC_LINKUP
    ULONG                       Bssidx;
    CF_PARM                     CfParm;

    CfParm.bValid = FALSE;
#endif
    PMP_PORT pPort = pAd->PortList[Elem->PortNum];

    // Init Variable IE structure

    PlatformAllocateMemory(pAd, &VarIE, MAX_VIE_LEN);
    if (VarIE == NULL)
    {
        DBGPRINT(RT_DEBUG_ERROR, ("MlmeSyncStartReqAction::Allocate memory size(=1024) failed\n"));
        return;
    }
    pVIE = (PNDIS_802_11_VARIABLE_IEs) VarIE;
    pVIE->Length = 0;
    TimeStamp.LowPart  = 0;
    TimeStamp.HighPart = 0;

    if (MlmeStartReqSanity(pAd, Elem->Msg, Elem->MsgLen, Ssid, &SsidLen)) 
    {
        // reset all the timers
        PlatformCancelTimer(&pPort->Mlme.ScanTimer, &TimerCancelled);
        PlatformCancelTimer(&pPort->Mlme.BeaconTimer, &TimerCancelled);

        //
        // Start a new IBSS. All IBSS parameters are decided now....
        //
        pAd->MlmeAux.BssType           = BSS_ADHOC;
        PlatformMoveMemory(pAd->MlmeAux.Ssid, Ssid, SsidLen); 
        pAd->MlmeAux.SsidLen           = SsidLen;

        if (!pPort->PortCfg.AcceptAnyBSSID)
        {
            COPY_MAC_ADDR(pAd->MlmeAux.Bssid, &pPort->PortCfg.DesiredBSSIDList[0]);
        }
        else
        {
            // generate a radom number as BSSID
            MacAddrRandomBssid(pAd, pAd->MlmeAux.Bssid);
        }

        Privacy = (IS_WEP_STATUS_ON(pPort->PortCfg.WepStatus)) || 
                  (pPort->PortCfg.WepStatus == Ralink802_11Encryption2Enabled) || 
                  (pPort->PortCfg.WepStatus == Ralink802_11Encryption3Enabled);

        if(pAd->StaCfg.WscControl.WscState != WSC_STATE_OFF)
        {
            if(IS_DISABLE_WSC20TB_RSNIE(pAd))
                Privacy = FALSE;
        }
        DBGPRINT(RT_DEBUG_TRACE, ("%s privacy:%d\n", __FUNCTION__, Privacy));
        pAd->MlmeAux.CapabilityInfo    = CAP_GENERATE(0,1,Privacy, (pPort->CommonCfg.TxPreamble == Rt802_11PreambleShort), 1);
        pAd->MlmeAux.BeaconPeriod      = pPort->CommonCfg.BeaconPeriod;
        pAd->MlmeAux.AtimWin           = pAd->StaCfg.AtimWin;


        // SupRate and ExtRate are derived from PhyMode. 
        pAd->MlmeAux.SupRateLen= pPort->CommonCfg.SupRateLen;
        PlatformMoveMemory(pAd->MlmeAux.SupRate, pPort->CommonCfg.SupRate, MAX_LEN_OF_SUPPORTED_RATES);
        RTMPCheckRates(pAd, pAd->MlmeAux.SupRate, &pAd->MlmeAux.SupRateLen);
        pAd->MlmeAux.ExtRateLen = pPort->CommonCfg.ExtRateLen;
        PlatformMoveMemory(pAd->MlmeAux.ExtRate, pPort->CommonCfg.ExtRate, MAX_LEN_OF_SUPPORTED_RATES);
        RTMPCheckRates(pAd, pAd->MlmeAux.ExtRate, &pAd->MlmeAux.ExtRateLen);

        // CH14 only supports 11b
        if (pPort->CommonCfg.Ch14BOnly && (pPort->Channel == CHANNEL_14))
        {
            pAd->MlmeAux.SupRateLen = 4;
            pAd->MlmeAux.ExtRateLen = 0;
        }

        if ((pPort->CommonCfg.PhyMode >= PHY_11ABGN_MIXED) && (!(pPort->CommonCfg.Ch14BOnly && (pPort->ScaningChannel == CHANNEL_14))))
        {
            // If support 40MHZ adhocN, calculate its central channel
            if (pAd->StaCfg.bAdhocNMode)
            {
                if(pPort->CommonCfg.PhyMode == PHY_11VHT)
                {
                    DBGPRINT(RT_DEBUG_WARN,("%s, LINE_%d, to be implement !!\n",__FUNCTION__,__LINE__));
                }

                MlmeSyncCheckBWOffset(pAd, pPort->Channel);

                // Set central channel to MlmeAux, and it shall be stored to CommonCfg after MlmeCntLinkUp
                if (pPort->CommonCfg.HtCapability.HtCapInfo.ChannelWidth == BW_40)
                {
                    if (pPort->CommonCfg.DesiredHtPhy.ExtChanOffset == EXTCHA_ABOVE)
                    {
                        pPort->CentralChannel = pPort->Channel+ 2;
                    }
                    else
                    {
                        if (pPort->ScaningChannel == 14)
                            pPort->CentralChannel = pPort->Channel - 1;
                        else
                            pPort->CentralChannel = pPort->Channel - 2;
                    }
                }
            }
        
            MlmeInfoUpdateHtIe(&pPort->CommonCfg.DesiredHtPhy, &pAd->MlmeAux.HtCapability, &pAd->MlmeAux.AddHtInfo);
            pAd->MlmeAux.HtCapabilityLen = sizeof(HT_CAPABILITY_IE);

            if (pPort->CommonCfg.PhyMode == PHY_11VHT)
            {
                //MlmeInfoUpdateVhtIe(&pPort->CommonCfg.DesiredVhtPhy, &pAd->MlmeAux.VhtCapability, &pAd->MlmeAux.VhtOperation);
            }

            // Choose BW from the result of MlmeSyncCheckBWOffset()
            if (pAd->StaCfg.bAdhocNMode)
            {
                pAd->MlmeAux.HtCapability.HtCapInfo.ChannelWidth    =  pPort->CommonCfg.HtCapability.HtCapInfo.ChannelWidth;
                pAd->MlmeAux.AddHtInfo.AddHtInfo.ExtChanOffset      =  pPort->CommonCfg.DesiredHtPhy.ExtChanOffset;
                pAd->MlmeAux.AddHtInfo.AddHtInfo.RecomWidth         =  (UCHAR)pPort->CommonCfg.HtCapability.HtCapInfo.ChannelWidth;
                pAd->MlmeAux.AddHtInfo.ControlChan                  =  pPort->Channel;
            }
            
            // Not turn pAd->StaActive.SupportedHtPhy.bHtEnable = TRUE here.
            DBGPRINT(RT_DEBUG_TRACE, ("SYNC -pAd->StaActive.SupportedHtPhy.bHtEnable = TRUE\n"));
        }
        else
        {
            pAd->MlmeAux.HtCapabilityLen = 0;
            pAd->StaActive.SupportedHtPhy.bHtEnable = FALSE;
        }
        // temporarily not support QOS in IBSS
        PlatformZeroMemory(&pAd->MlmeAux.APEdcaParm, sizeof(EDCA_PARM));
        PlatformZeroMemory(&pAd->MlmeAux.APQbssLoad, sizeof(QBSS_LOAD_PARM));
        PlatformZeroMemory(&pAd->MlmeAux.APQosCapability, sizeof(QOS_CAPABILITY_PARM));

        //AsicSwitchChannel(pAd, pPort->Channel, FALSE);
        AsicSwitchChannel(pPort, pPort->Channel, pPort->CentralChannel, pPort->BBPCurrentBW, FALSE);
        DBGPRINT(RT_DEBUG_TRACE, ("MlmeSyncStartReqAction pAd->MlmeAux.CentralChannel = %d\n", pPort->Channel));
        DBGPRINT(RT_DEBUG_TRACE, ("SYNC - MlmeSyncStartReqAction(ch= %d,sup rates= %d, ext rates=%d)\n",
            pPort->Channel, pAd->MlmeAux.SupRateLen, pAd->MlmeAux.ExtRateLen));

        if (pAd->MlmeAux.bStaCanRoam)
        {
            PlatformIndicateAdhocRoamingStart(pAd, pPort,pAd->MlmeAux.Bssid, pAd->MlmeAux.Ssid, pAd->MlmeAux.SsidLen);
            PlatformIndicateRoamingCompletion(pAd,pPort, DOT11_ASSOC_STATUS_SUCCESS);
        }
        else
        {
            PlatformIndicateConnectionStart(pAd,pPort, pAd->MlmeAux.BssType, pAd->MlmeAux.Bssid, pAd->MlmeAux.Ssid, pAd->MlmeAux.SsidLen);
            PlatformIndicateConnectionCompletion(pAd, pPort,DOT11_CONNECTION_STATUS_SUCCESS);
            pAd->MlmeAux.bStaCanRoam = TRUE;
            LedCtrlConnectionCompletion(pAd,pPort, TRUE);
        }

#ifdef  SINGLE_ADHOC_LINKUP
        // Add itself as the entry within BSS table
        Bssidx = BssTableSearch(pAd, pPort, &pAd->ScanTab, pAd->MlmeAux.Bssid, pPort->Channel);
        if (Bssidx == BSS_NOT_FOUND)
        {
            Bssidx = BssTableSetEntry(pAd, pPort, &pAd->ScanTab, pAd->MlmeAux.Bssid,Ssid, SsidLen, pAd->MlmeAux.BssType, 
                pAd->MlmeAux.BeaconPeriod, &CfParm, pAd->MlmeAux.AtimWin, pAd->MlmeAux.CapabilityInfo, 
                &pAd->MlmeAux.HtCapability, &pAd->MlmeAux.VhtCapability, &pAd->MlmeAux.VhtOperation, pAd->MlmeAux.HtCapabilityLen, &pPort->CommonCfg.AddHTInfo, SIZE_ADD_HT_INFO_IE, pAd->MlmeAux.NewExtChannelOffset,
                pAd->MlmeAux.SupRate, pAd->MlmeAux.SupRateLen, pAd->MlmeAux.ExtRate, pAd->MlmeAux.ExtRateLen,
                &pAd->MlmeAux.SupportedHtPhy, pPort->Channel, pPort->BbpRssiToDbmDelta - 30, TimeStamp, NULL, NULL, NULL, 0, NULL, 0, NULL, 0, NULL, TRUE);
        }
#endif
        //
        // To indicate now I am a AdHoc Creator. Not a Joiner.
        //
        pAd->StaCfg.AdhocCreator = TRUE;
        pAd->StaCfg.AdhocJoiner  = FALSE;

        pPort->Mlme.SyncMachine.CurrState = SYNC_IDLE;
        Status = MLME_SUCCESS;
        MlmeEnqueue(pAd,pPort, MLME_CNTL_STATE_MACHINE, MT2_START_CONF, 2, &Status);
    } 
    else 
    {
        DBGPRINT_ERR(("SYNC - MlmeSyncStartReqAction() sanity check fail.\n"));
        pPort->Mlme.SyncMachine.CurrState = SYNC_IDLE;
        Status = MLME_INVALID_FORMAT;
        MlmeEnqueue(pAd,pPort, MLME_CNTL_STATE_MACHINE, MT2_START_CONF, 2, &Status);
    }

    if (VarIE)
        PlatformFreeMemory(VarIE, MAX_VIE_LEN);
}

/* 
    ==========================================================================
    Description:
        peer sends beacon back when scanning
    ==========================================================================
 */
VOID MlmeSyncMlmeSyncPeerBeaconAtScanAction(
    IN PMP_ADAPTER pAd, 
    IN MLME_QUEUE_ELEM *Elem) 
{
    UCHAR           Bssid[MAC_ADDR_LEN], Addr2[MAC_ADDR_LEN];
    UCHAR           Ssid[MAX_LEN_OF_SSID], BssType, Channel = 0xff, NewChannel,
                    SsidLen, DtimCount, DtimPeriod, BcastFlag, MessageToMe;
    CF_PARM         CfParm;
    USHORT          BeaconPeriod, AtimWin, CapabilityInfo;
    PFRAME_802_11   pFrame;
    PHEADER_802_11   pHeader;
    ULONGLONG       TimeStamp;
    UCHAR           Erp;
    UCHAR           SupRate[MAX_LEN_OF_SUPPORTED_RATES], ExtRate[MAX_LEN_OF_SUPPORTED_RATES];
    UCHAR           SupRateLen, ExtRateLen;
    USHORT          LenVIE;
    EDCA_PARM       EdcaParm;
    QBSS_LOAD_PARM  QbssLoad;
    QOS_CAPABILITY_PARM QosCapability;
    ULONG           RalinkIe;
    PUCHAR                      VarIE = NULL;       // Total VIE length = MAX_VIE_LEN - -5
    NDIS_802_11_VARIABLE_IEs    *pVIE = NULL;
    HT_CAPABILITY_IE        HtCapability;
    ADD_HT_INFO_IE      AddHtInfo;  // AP might use this additional ht info IE 
    UCHAR             HtCapabilityLen;
    UCHAR             PreNHtCapabilityLen;
    UCHAR           AddHtInfoLen;

    // for VHT
    BOOLEAN           bVhtCapable = FALSE;
    VHT_CAP_IE        VhtCapability;
    VHT_OP_IE         VhtOperation;

    UCHAR           NewExtChannelOffset = 0xff;
    EXT_CAP_ELEMT               ExtCapIE;   // this is the extened capibility IE  
    UCHAR               SSIDL_VIE[256];     // SSIDL is limited to 256
    PMP_PORT      pPort = pAd->PortList[Elem->PortNum];
    PUCHAR          WSCInfoAtBeacons = NULL;
    USHORT          WSCInfoAtBeaconsLen;
    PUCHAR          WSCInfoAtProbeRsp = NULL;
    USHORT          WSCInfoAtProbeRspLen;
    BOOLEAN bHasOperatingModeField = FALSE;
    OPERATING_MODE_FIELD OperatingModeField = {0};

    SSIDL_IE            *pSSIDL_VIE = NULL;
    UCHAR               bConnectedToCiscoAp;
    UCHAR MaxTxPowerLevel = MDSM_ABSENT_COUNTRY_IE;
    UCHAR LocalPowerConstraint = MDSM_ABSENT_POWER_CONSTRAINT_IE;

    PlatformAllocateMemory(pAd, &WSCInfoAtBeacons, MAX_VIE_LEN);
    PlatformAllocateMemory(pAd, &WSCInfoAtProbeRsp, MAX_VIE_LEN);

    // NdisFillMemory(Ssid, MAX_LEN_OF_SSID, 0x00);
    pFrame = (PFRAME_802_11) Elem->Msg;
    pHeader = &pFrame->Hdr;

    DBGPRINT(RT_DEBUG_INFO, ("MacData_2_ [%d], ToPort(%d), Addr1(%x %x %x %x %x %x)Addr2(%x %x %x %x %x %x)Addr3(%x %x %x %x %x %x), LatchRfRegs.Channel(%d)\n", 
                                __LINE__,
                                pPort->PortNumber,
                                pHeader->Addr1[0], pHeader->Addr1[1], pHeader->Addr1[2], pHeader->Addr1[3], pHeader->Addr1[4], pHeader->Addr1[5],
                                pHeader->Addr2[0], pHeader->Addr2[1], pHeader->Addr2[2], pHeader->Addr2[3], pHeader->Addr2[4], pHeader->Addr2[5],
                                pHeader->Addr3[0], pHeader->Addr3[1], pHeader->Addr3[2], pHeader->Addr3[3], pHeader->Addr3[4], pHeader->Addr3[5],
                                pAd->HwCfg.LatchRfRegs.Channel));
    
    // Init Variable IE structure
    PlatformAllocateMemory(pAd, &VarIE, MAX_VIE_LEN);
    if (VarIE == NULL)
    {
        DBGPRINT(RT_DEBUG_ERROR, ("MlmeSyncMlmeSyncPeerBeaconAtScanAction::Allocate memory size(=1024) failed\n"));
        goto CleanUp;
    }
    pVIE = (PNDIS_802_11_VARIABLE_IEs) VarIE;
    pVIE->Length = 0;

    PlatformZeroMemory(&HtCapability, sizeof(HT_CAPABILITY_IE));    
    // init SSIDL IE
    PlatformZeroMemory(SSIDL_VIE, sizeof(SSIDL_VIE));
    pSSIDL_VIE = (PSSIDL_IE) SSIDL_VIE;

    if (MlmeSyncPeerBeaconAndProbeRspSanity(pAd, 
                                pPort,
                                Elem->Msg, 
                                Elem->MsgLen, 
                                Addr2, 
                                Bssid, 
                                Ssid, 
                                &SsidLen, 
                                &BssType, 
                                &BeaconPeriod, 
                                &Channel, 
                                &NewChannel,
                                &TimeStamp, 
                                &CfParm, 
                                &AtimWin, 
                                &CapabilityInfo, 
                                &Erp,
                                &DtimCount, 
                                &DtimPeriod, 
                                &BcastFlag, 
                                &MessageToMe, 
                                SupRate,
                                &SupRateLen,
                                ExtRate,
                                &ExtRateLen,
                                &EdcaParm,
                                &QbssLoad,
                                &QosCapability,
                                &RalinkIe,
                                &HtCapabilityLen,
                                &PreNHtCapabilityLen,
                                &HtCapability,
                                &bVhtCapable,
                                &VhtCapability,
                                &VhtOperation,
                                &AddHtInfoLen,
                                &AddHtInfo,
                                &ExtCapIE,
                                &NewExtChannelOffset,
                                &WSCInfoAtBeaconsLen,
                                WSCInfoAtBeacons,
                                &WSCInfoAtProbeRspLen,
                                WSCInfoAtProbeRsp,
                                pSSIDL_VIE,
                                &MaxTxPowerLevel, 
                                &LocalPowerConstraint,
                                &bConnectedToCiscoAp,
                                &bHasOperatingModeField,
                                &OperatingModeField,
                                &LenVIE,
                                pVIE)) 
    {
        ULONG   Idx;
        CHAR    RealRssi;
        BOOLEAN bUpdateRssi = TRUE;
        BOOLEAN containP2P = FALSE;


        if (pFrame->Hdr.FC.SubType == SUBTYPE_PROBE_RSP)
            DBGPRINT(RT_DEBUG_TRACE,("%s::Update ProbeRsp\n", __FUNCTION__));

        // bald eagle by SSID
        if(pAd->StaCfg.BaldEagle.EnableBladEagle)
        {
            if(SSID_EQUAL(Ssid, SsidLen, pAd->StaCfg.BaldEagle.Ssid, pAd->StaCfg.BaldEagle.SsidLen))
            {
                DBGPRINT(RT_DEBUG_TRACE,("The specific AP: %s , was found in MlmeSyncMlmeSyncPeerBeaconAtScanAction!!\n",pAd->StaCfg.BaldEagle.Ssid));
            }
            else
            {
                goto CleanUp;
            }
        }

        Idx = BssTableSearch(pAd, pPort, &pAd->ScanTab, Bssid, Channel);
        if (Idx != BSS_NOT_FOUND)
            RealRssi = pAd->ScanTab.BssEntry[Idx].Rssi;

        /*if (pAd->HwCfg.RxAnt.EvaluatePeriod == 0)
        {
            RealRssi = MlmeSyncConvertToRssi(pAd, Elem->Rssi, RSSI_0, Elem->AntSel, BW_20);         
        }*/
        RealRssi = RTMPMaxRssi(pAd, MlmeSyncConvertToRssi(pAd, Elem->Rssi, RSSI_0, Elem->AntSel, BW_20), MlmeSyncConvertToRssi(pAd, Elem->Rssi2, RSSI_1, Elem->AntSel, BW_20), MlmeSyncConvertToRssi(pAd, Elem->Rssi3, RSSI_2, Elem->AntSel, BW_20));

        if (P2P_ON(pPort))
        {
            PRT_P2P_CLIENT_ENTRY pP2PBssEntry = NULL;
            PRT_P2P_DISCOVERY_ENTRY pP2PDiscoEntry = NULL;
            containP2P = PeerP2pBeaconProbeRspAtScan(pAd, pPort, Elem, CapabilityInfo, Channel, &pP2PBssEntry, &pP2PDiscoEntry);

            // update other fileds to discovery table
#if(COMPILE_WIN8_ABOVE(CURRENT_OS_NDIS_VER))  
            if (((NDIS_WIN8_ABOVE(pAd) && (MT_TEST_BIT(pPort->P2PCfg.ScanWithFilter, P2pMs_SCAN_FILTER_FORCE_LEGACY_NETWORK))) ||
                (containP2P == TRUE)) && (pP2PDiscoEntry != NULL))
#else
            if((containP2P == TRUE) && (pP2PDiscoEntry != NULL))
#endif
            {
                pP2PDiscoEntry->Rssi = RealRssi;
                pP2PDiscoEntry->BeaconPeriod = BeaconPeriod;
                pP2PDiscoEntry->CapabilityInfo = CapabilityInfo;
 
                // INFRA mode for discovered WFD Devices and WFD GOs
                pP2PDiscoEntry->BssType = BssType;

                if (pFrame->Hdr.FC.SubType == SUBTYPE_BEACON)
                {
                    pP2PDiscoEntry->BeaconTimestamp = TimeStamp;
                    pP2PDiscoEntry->BeaconFrameLen = min(Elem->MsgLen - LENGTH_802_11 - MS_WFD_BYPASS_LEN, (USHORT)sizeof(pP2PDiscoEntry->BeaconFrame));//Elem->MsgLen - LENGTH_802_11 - MS_WFD_BYPASS_LEN;
                    PlatformMoveMemory(pP2PDiscoEntry->BeaconFrame, Add2Ptr(Elem->Msg, LENGTH_802_11 + MS_WFD_BYPASS_LEN), pP2PDiscoEntry->BeaconFrameLen);
                    //DumpFrameMessage((PUCHAR) Add2Ptr(Elem->Msg, LENGTH_802_11 + MS_WFD_BYPASS_LEN), Elem->MsgLen - LENGTH_802_11 - MS_WFD_BYPASS_LEN, ("WFDA_SUBTYPE_BEACON\n"));
                }
                else if (pFrame->Hdr.FC.SubType == SUBTYPE_PROBE_RSP)
                {
                    pP2PDiscoEntry->ProbeRspTimestamp = TimeStamp;
                    pP2PDiscoEntry->ProbeRspFrameLen =  min(Elem->MsgLen - LENGTH_802_11 - MS_WFD_BYPASS_LEN, (USHORT)sizeof(pP2PDiscoEntry->ProbeRspFrame));//Elem->MsgLen - LENGTH_802_11 - MS_WFD_BYPASS_LEN;
                    PlatformMoveMemory(pP2PDiscoEntry->ProbeRspFrame, Add2Ptr(Elem->Msg, LENGTH_802_11 + MS_WFD_BYPASS_LEN), pP2PDiscoEntry->ProbeRspFrameLen);
                    //DumpFrameMessage((PUCHAR) Add2Ptr(Elem->Msg, LENGTH_802_11 + MS_WFD_BYPASS_LEN), Elem->MsgLen - LENGTH_802_11 - MS_WFD_BYPASS_LEN, ("WFDA_SUBTYPE_PROBE_RSP\n"));
                    DBGPRINT(RT_DEBUG_TRACE,("%s::Update ProbeRsp\n", __FUNCTION__));
                }
            }
            else
            {
                DBGPRINT(RT_DEBUG_INFO,("%s::containP2P(%d), pP2PDiscoEntry(%d)\n", __FUNCTION__, (containP2P == TRUE)?1:0, (pP2PDiscoEntry != NULL)?1:0));
            }

            // We only process Beacon and ProbeRsp from P2P GO
            if(PlatformEqualMemory(Ssid, WILDP2PSSID, WILDP2PSSIDLEN) && SsidLen == WILDP2PSSIDLEN && containP2P)
                goto CleanUp;
        }
        else
        {
            DBGPRINT(RT_DEBUG_INFO,("%s::P2P not ON\n", __FUNCTION__));
        }

        // If driver receives a beacon at a channel which is different from DS parameter, the RSSI should not be update (it will be smaller than same channel).
        // However, we still update remainder fields in beacon frame.
        if (Channel != Elem->Channel)
        {
            bUpdateRssi = FALSE;
        }

//      DBGPRINT(RT_DEBUG_TRACE, ("SYNC - MlmeSyncMlmeSyncPeerBeaconAtScanAction (SubType=%d, SsidLen=%d, Ssid=%s)\n", pFrame->Hdr.FC.SubType, SsidLen,Ssid));
        if ((HtCapabilityLen > 0) || (PreNHtCapabilityLen > 0))
            HtCapabilityLen = SIZE_HT_CAP_IE;

        Idx = BssTableSetEntry(pAd, pPort, &pAd->ScanTab, Bssid, Ssid, SsidLen, BssType, BeaconPeriod,
                      &CfParm, AtimWin, CapabilityInfo, SupRate, SupRateLen, ExtRate, ExtRateLen,  &HtCapability, &VhtCapability, &VhtOperation,
                     &AddHtInfo, HtCapabilityLen, AddHtInfoLen, NewExtChannelOffset, Channel, RealRssi, TimeStamp, 
                     &EdcaParm, &QosCapability, &QbssLoad, WSCInfoAtBeaconsLen, WSCInfoAtBeacons, WSCInfoAtProbeRspLen, WSCInfoAtProbeRsp, LenVIE, pVIE, bUpdateRssi);
        // Check if this scan channel is the effeced channel 
        if ((pAd->HwCfg.ChannelList[pPort->CommonCfg.ChannelListIdx].bEffectedChannel == TRUE) &&
            (Channel == pAd->HwCfg.ChannelList[pPort->CommonCfg.ChannelListIdx].Channel))
        {
            UCHAR               RegClass;
            OVERLAP_BSS_SCAN_IE BssScan;
            // Read Beacon's Reg Class IE if any.
            MlmeSyncPeerBeaconAndProbeRspSanity2(pAd, Elem->Msg, Elem->MsgLen, &BssScan, &RegClass);
            TriEventTableSetEntry(pAd, pPort, &pPort->CommonCfg.TriggerEventTab, Bssid, &HtCapability, HtCapabilityLen, RegClass, Channel);
        }
        if (Idx != BSS_NOT_FOUND)
        {
            if(containP2P)
            {
                pAd->ScanTab.BssEntry[Idx].P2PDevice = TRUE;
                DBGPRINT(RT_DEBUG_INFO, ("this AP containP2P"));
            }
            else
            {
                pAd->ScanTab.BssEntry[Idx].P2PDevice = FALSE;
                DBGPRINT(RT_DEBUG_INFO, ("this AP not containP2P"));
            }

            //[070604]hidden or not determined only by beacon frame
            if (pFrame->Hdr.FC.SubType == SUBTYPE_BEACON)
            {
                // For hidden SSID AP, it might send beacon with SSID len equal to 0
                // Or send beacon /probe response with SSID len matching real SSID length,
                // but SSID is all zero. such as "00-00-00-00" with length 4.
                // We have to prevent this case overwrite correct table

                pAd->ScanTab.BssEntry[Idx].Hidden = 1;
                if (SsidLen > 0)
                {
                    //ssid is not equal to zero ssid
                    if (PlatformEqualMemory(Ssid, ZeroSsid, SsidLen) == 0)
                    {
                        pAd->ScanTab.BssEntry[Idx].Hidden = 0;
                    }
                }
                //[070805] In the air contain hidden ssid beacon ,reset this flag to prevent connect hidden AP issue,
                //              This issue impact with Ndistest packfilters_ext script test
                if ( SsidLen == 0)
                    pPort->CommonCfg.bSSIDInProbeRequest = TRUE;
            }
        }
#if(COMPILE_WIN8_ABOVE(CURRENT_OS_NDIS_VER))  
        if((P2P_ON(pPort)) && (pPort->P2PCfg.P2pMsSatetPhase == P2pMs_STATE_PROVISIONING)
            && (PlatformEqualMemory(pPort->P2PCfg.DeviceAddress, Bssid, MAC_ADDR_LEN))
            && (PlatformEqualMemory(pPort->P2PCfg.SSID, Ssid, SsidLen)))
        {
            pPort->P2PCfg.P2pProprietary.ListenChanelIndex = pPort->P2PCfg.P2pProprietary.ListenChanelCount;
            DBGPRINT(RT_DEBUG_TRACE, ("this probe response or beacon is I wanted ==> scan confirm right now"));
        }
#else
        if((P2P_ON(pPort)) && (pPort->P2PCfg.P2PConnectState == P2P_DO_GO_NEG_DONE_CLIENT) 
            && (PlatformEqualMemory(pPort->P2PCfg.Bssid, Bssid, MAC_ADDR_LEN)) 
            && (PlatformEqualMemory(pPort->P2PCfg.SSID, Ssid, SsidLen)))
        {
            USHORT  Status = 0;
            //pAd->StaCfg.BssNr = pAd->ScanTab.BssNr;
            pAd->StaCfg.bFastRoamingScan = FALSE;   // reset this which was set by FastRoaming
            pAd->StaCfg.bImprovedScan = FALSE;
            // used to check if driver has to reconnect immediatelly
            //pAd->StaCfg.bFastUpdateScanTab = FALSE;   // reset when scan complete

            pPort->Mlme.SyncMachine.CurrState = SYNC_IDLE;
            Status = MLME_SUCCESS;
            MlmeEnqueue(pAd, pPort, MLME_CNTL_STATE_MACHINE, MT2_SCAN_CONF, 2, &Status);
            DBGPRINT(RT_DEBUG_TRACE, ("ssid %s", pPort->P2PCfg.SSID));
        }
#endif
    }
    else
    {
        DBGPRINT(RT_DEBUG_TRACE, ("%s(ERROR)::MlmeSyncPeerBeaconAndProbeRspSanity FAILED!\n", __FUNCTION__));
    }
    // sanity check fail, ignored

CleanUp:
    if (VarIE)
        PlatformFreeMemory(VarIE, MAX_VIE_LEN);

    if (WSCInfoAtBeacons)
        PlatformFreeMemory(WSCInfoAtBeacons, MAX_VIE_LEN);

    if (WSCInfoAtProbeRsp)
        PlatformFreeMemory(WSCInfoAtProbeRsp, MAX_VIE_LEN);
}

/*
    ==========================================================================
    Description:
        When waiting joining the (I)BSS, beacon received from external
    ==========================================================================
 */
VOID MlmeSyncMlmeSyncPeerBeaconAtJoinAction(
    IN PMP_ADAPTER pAd,
    IN MLME_QUEUE_ELEM *Elem)
{
    UCHAR         Bssid[MAC_ADDR_LEN], Addr2[MAC_ADDR_LEN];
    UCHAR         Ssid[MAX_LEN_OF_SSID], SsidLen, BssType, Channel, MessageToMe, 
                  DtimCount, DtimPeriod, BcastFlag, NewChannel; 
    ULONGLONG     TimeStamp;
    USHORT        BeaconPeriod, AtimWin, CapabilityInfo, CurrentCapabilityInfo;
    CF_PARM       Cf;
    BOOLEAN       TimerCancelled;
    UCHAR         Erp;
    UCHAR         SupRate[MAX_LEN_OF_SUPPORTED_RATES], ExtRate[MAX_LEN_OF_SUPPORTED_RATES];
    UCHAR         SupRateLen, ExtRateLen;
    USHORT        LenVIE;
    EDCA_PARM       EdcaParm;
    QBSS_LOAD_PARM  QbssLoad;
    QOS_CAPABILITY_PARM QosCapability;
    USHORT        Status;
    PUCHAR          VarIE = NULL;       // Total VIE length = MAX_VIE_LEN - -5
    NDIS_802_11_VARIABLE_IEs    *pVIE = NULL;
    ULONG           RalinkIe;
    ULONG         Idx;
    UCHAR           PeerTxType;
    HT_CAPABILITY_IE        HtCapability;
    ADD_HT_INFO_IE      AddHtInfo;  // AP might use this additional ht info IE 
    UCHAR           HtCapabilityLen, PreNHtCapabilityLen;
    UCHAR           AddHtInfoLen;
    UCHAR           NewExtChannelOffset = 0xff, CentralChannel=0;
    PRTMP_ADHOC_LIST    pAdhocList;
    PFRAME_802_11       pFrame;
    EXT_CAP_ELEMT       ExtCapIE;   // this is the extened capibility IE  
    PMP_PORT          pPort = pAd->PortList[Elem->PortNum];
    PUCHAR          WSCInfoAtBeacons = NULL;
    USHORT          WSCInfoAtBeaconsLen;
    PUCHAR          WSCInfoAtProbeRsp = NULL;
    USHORT          WSCInfoAtProbeRspLen;

    UCHAR               SSIDL_VIE[256];     // SSIDL is limited to 256
    SSIDL_IE            *pSSIDL_VIE = NULL;
    UCHAR               bConnectedToCiscoAp;
    UCHAR MaxTxPowerLevel = MDSM_ABSENT_COUNTRY_IE;
    UCHAR LocalPowerConstraint = MDSM_ABSENT_POWER_CONSTRAINT_IE;
    UCHAR           Wcid = (UCHAR)MlmeSyncGetWlanIdxByPort(pPort, ROLE_WLANIDX_BSSID);
    BOOLEAN         P2pNModeEnable = TRUE;
    UCHAR           Index = 0;
    UCHAR           SelectedRegistrar = 0;
    PMAC_TABLE_ENTRY pWcidMacTabEntry = NULL;

    // for VHT
    VHT_CAP_IE      VhtCapability;
    VHT_OP_IE       VhtOperation;
    BOOLEAN         bVhtCapable = FALSE;
    BOOLEAN bHasOperatingModeField = FALSE;
    OPERATING_MODE_FIELD OperatingModeField = {0};

    PlatformAllocateMemory(pAd, &WSCInfoAtBeacons, MAX_VIE_LEN);
    PlatformAllocateMemory(pAd, &WSCInfoAtProbeRsp, MAX_VIE_LEN);

    pFrame = (PFRAME_802_11) Elem->Msg;
    // Init Variable IE structure
    PlatformAllocateMemory(pAd, &VarIE, MAX_VIE_LEN);
    if (VarIE == NULL)
    {
        if (WSCInfoAtBeacons)
            PlatformFreeMemory(WSCInfoAtBeacons, MAX_VIE_LEN);

        if (WSCInfoAtProbeRsp)
            PlatformFreeMemory(WSCInfoAtProbeRsp, MAX_VIE_LEN);
    
        DBGPRINT(RT_DEBUG_ERROR, ("MlmeSyncMlmeSyncPeerBeaconAtJoinAction::Allocate memory size(=1024) failed\n"));
        return;
    }

    pVIE = (PNDIS_802_11_VARIABLE_IEs) VarIE;
    pVIE->Length = 0;

    PlatformZeroMemory(&HtCapability, sizeof(HT_CAPABILITY_IE));    

    // init SSIDL IE
    PlatformZeroMemory(SSIDL_VIE, sizeof(SSIDL_VIE));
    pSSIDL_VIE = (PSSIDL_IE) SSIDL_VIE;

#if 0
    {
        int i = 0;
        PUCHAR p = (PUCHAR)&Elem->Msg[0];
        //for (i = 0; i < (int)pRxD->RxDscrDW0.RxByteCount; i = i + 8)
        for (i = 0; i < (int)Elem->MsgLen; i = i + 8)
        {
            DBGPRINT_RAW(RT_DEBUG_TRACE, ("Msg[%d] %x %x %x %x %x %x %x %x\n", i, p[i], p[i+1], p[i+2], p[i+3], p[i+4], p[i+5], p[i+6], p[i+7]));
        }
    }
#endif
    if (MlmeSyncPeerBeaconAndProbeRspSanity(pAd, 
                                pPort,
                                Elem->Msg, 
                                Elem->MsgLen, 
                                Addr2, 
                                Bssid, 
                                Ssid, 
                                &SsidLen, 
                                &BssType, 
                                &BeaconPeriod, 
                                &Channel, 
                                &NewChannel,
                                &TimeStamp, 
                                &Cf, 
                                &AtimWin, 
                                &CapabilityInfo, 
                                &Erp,
                                &DtimCount, 
                                &DtimPeriod, 
                                &BcastFlag, 
                                &MessageToMe, 
                                SupRate,
                                &SupRateLen,
                                ExtRate,
                                &ExtRateLen,
                                &EdcaParm,
                                &QbssLoad,
                                &QosCapability,
                                &RalinkIe,
                                &HtCapabilityLen,
                                &PreNHtCapabilityLen,
                                &HtCapability,
                                &bVhtCapable,
                                &VhtCapability,
                                &VhtOperation,
                                &AddHtInfoLen,
                                &AddHtInfo,
                                &ExtCapIE,
                                &NewExtChannelOffset,
                                &WSCInfoAtBeaconsLen,
                                WSCInfoAtBeacons,
                                &WSCInfoAtProbeRspLen,
                                WSCInfoAtProbeRsp,
                                pSSIDL_VIE,
                                &MaxTxPowerLevel, 
                                &LocalPowerConstraint, 
                                &bConnectedToCiscoAp,
                                &bHasOperatingModeField,
                                &OperatingModeField,
                                &LenVIE,
                                pVIE)) 
    {
        // bald eagle by SSID
        if(pAd->StaCfg.BaldEagle.EnableBladEagle)
        {
            if(SSID_EQUAL(Ssid, SsidLen, pAd->StaCfg.BaldEagle.Ssid, pAd->StaCfg.BaldEagle.SsidLen))
            {
                DBGPRINT(RT_DEBUG_TRACE,("The specific AP: %s , was found in MlmeSyncMlmeSyncPeerBeaconAtJoinAction!!\n",pAd->StaCfg.BaldEagle.Ssid));
            }
            else
            {
                if (VarIE)
                    PlatformFreeMemory(VarIE, MAX_VIE_LEN);
                if (WSCInfoAtBeacons)
                    PlatformFreeMemory(WSCInfoAtBeacons, MAX_VIE_LEN);
                if (WSCInfoAtProbeRsp)
                    PlatformFreeMemory(WSCInfoAtProbeRsp, MAX_VIE_LEN);
                return;
            }
        }
        
        // Disqualify 11b only adhoc when we are in 11g only adhoc mode
        if (BssType == BSS_ADHOC)
        {
            PeerTxType = PeerTxTypeInUseSanity(Channel, SupRate, SupRateLen, ExtRate, ExtRateLen);
            if ((pAd->StaCfg.AdhocMode == ADHOC_11G) && (PeerTxType == CCK_RATE))
            {
                if (VarIE)
                    PlatformFreeMemory(VarIE, MAX_VIE_LEN);

                if (WSCInfoAtBeacons)
                    PlatformFreeMemory(WSCInfoAtBeacons, MAX_VIE_LEN);

                if (WSCInfoAtProbeRsp)
                    PlatformFreeMemory(WSCInfoAtProbeRsp, MAX_VIE_LEN);

                return;
            }
            else if ((pAd->StaCfg.AdhocMode == ADHOC_11B) && (PeerTxType == OFDM_RATE))
            {
                if (VarIE)
                    PlatformFreeMemory(VarIE, MAX_VIE_LEN);

                if (WSCInfoAtBeacons)
                    PlatformFreeMemory(WSCInfoAtBeacons, MAX_VIE_LEN);

                if (WSCInfoAtProbeRsp)
                    PlatformFreeMemory(WSCInfoAtProbeRsp, MAX_VIE_LEN);

                return;
            }
        }

        if (P2P_ON(pPort) && MAC_ADDR_EQUAL(pAd->MlmeAux.Bssid, Bssid))
        {
            if(pFrame->Hdr.FC.SubType == SUBTYPE_BEACON)
            {
                P2PParseWPSIE(WSCInfoAtBeacons, WSCInfoAtBeaconsLen, NULL, NULL, NULL, NULL, &SelectedRegistrar, NULL, NULL);
            }
            else
            {
                P2PParseWPSIE(WSCInfoAtProbeRsp, WSCInfoAtProbeRspLen, NULL, NULL, NULL, NULL, &SelectedRegistrar, NULL, NULL);
            }

            // If GO status is not ready to join, wait for next beacon/probersp or JOIN_TIMEOUT.
            if (!P2pMlmeSyncMlmeSyncPeerBeaconAtJoinAction(pAd, Elem, Bssid, SelectedRegistrar))
            {
                if (VarIE)
                    PlatformFreeMemory(VarIE, MAX_VIE_LEN);
                if (WSCInfoAtBeacons)
                    PlatformFreeMemory(WSCInfoAtBeacons, MAX_VIE_LEN);
                if (WSCInfoAtProbeRsp)
                    PlatformFreeMemory(WSCInfoAtProbeRsp, MAX_VIE_LEN);
                return;
            }
        }

        // BEACON from desired BSS/IBSS found. We should be able to decide most
        // BSS parameters here.
        // Q. But what happen if this JOIN doesn't conclude a successful ASSOCIATEION?
        //    Do we need to receover back all parameters belonging to previous BSS?
        // A. Should be not. There's no back-door recover to previous AP. It still need
        //    a new JOIN-AUTH-ASSOC sequence.
        if ((MAC_ADDR_EQUAL(pAd->MlmeAux.Bssid, Bssid)) && 
            (SSID_EQUAL(pAd->MlmeAux.Ssid, pAd->MlmeAux.SsidLen, Ssid, SsidLen)))
        {
            DBGPRINT(RT_DEBUG_TRACE, ("SYNC - receive desired BEACON at JoinWaitBeacon... Channel = %d\n", Channel));
            PlatformCancelTimer(&pPort->Mlme.BeaconTimer, &TimerCancelled);

            // Update RSSI to prevent No signal display when cards first initialized
            pAd->StaCfg.RssiSample.LastRssi[0]  = MlmeSyncConvertToRssi(pAd, Elem->Rssi, RSSI_0, Elem->AntSel, BW_20);
            pAd->StaCfg.RssiSample.LastRssi[1]  = MlmeSyncConvertToRssi(pAd, Elem->Rssi2, RSSI_1, Elem->AntSel, BW_20);
            pAd->StaCfg.RssiSample.LastRssi[2]  = MlmeSyncConvertToRssi(pAd, Elem->Rssi3, RSSI_2, Elem->AntSel, BW_20);
            pAd->StaCfg.RssiSample.AvgRssi[0]       = pAd->StaCfg.RssiSample.LastRssi[0];
            pAd->StaCfg.RssiSample.AvgRssiX8[0] = pAd->StaCfg.RssiSample.AvgRssi[0] << 3;
            pAd->StaCfg.RssiSample.AvgRssi[1]   = pAd->StaCfg.RssiSample.LastRssi[1];
            pAd->StaCfg.RssiSample.AvgRssiX8[1] = pAd->StaCfg.RssiSample.AvgRssi[1] << 3;
            pAd->StaCfg.RssiSample.AvgRssi[2]   = pAd->StaCfg.RssiSample.LastRssi[2];
            pAd->StaCfg.RssiSample.AvgRssiX8[2] = pAd->StaCfg.RssiSample.AvgRssi[2] << 3;

            // Reset it. Not use RateSwitchTable in MlmeSelectTxRateTable.
            pPort->CommonCfg.LimitTxRateTableSize = 0;

            // Add the beacon into BssTab if this entry doesn't exist
            // Connect to the same AP, the entry may have been deleted when DisAssoc/DeAuth, and we can add it back here
            Idx = BSS_NOT_FOUND;
            if ((pFrame->Hdr.FC.SubType == SUBTYPE_BEACON) ||
                (pFrame->Hdr.FC.SubType == SUBTYPE_PROBE_RSP))
            {
                Idx = BssSsidTableSearch(&pAd->ScanTab, Bssid, pAd->MlmeAux.Ssid, pAd->MlmeAux.SsidLen, Channel);
                if (Idx == BSS_NOT_FOUND)
                {
                    DBGPRINT(RT_DEBUG_TRACE, ("SYNC - JoinWaitBeacon ... Beacon is not in BssTab, BssNr = %d; Add this entry \n", pAd->ScanTab.BssNr));
                    MlmeSyncMlmeSyncPeerBeaconAtScanAction(pAd, Elem);

                    // Get Idx again
                    Idx = BssSsidTableSearch(&pAd->ScanTab, Bssid, pAd->MlmeAux.Ssid, pAd->MlmeAux.SsidLen, Channel);
                }
            }

            //
            // We need to check if SSID only set to any, then we can record the current SSID.
            // Otherwise will cause hidden SSID association failed. 
            //
            if (pAd->MlmeAux.SsidLen == 0)
            {
                PlatformMoveMemory(pAd->MlmeAux.Ssid, Ssid, SsidLen);
                pAd->MlmeAux.SsidLen = SsidLen;
            }
            else
            {
                if (Idx != BSS_NOT_FOUND)
                {
                    CurrentCapabilityInfo = CapabilityInfo;
                    
                    //
                    // Multiple SSID case, used correct CapabilityInfo
                    //
                    CapabilityInfo = pAd->ScanTab.BssEntry[Idx].CapabilityInfo;

                    // check privacy in Capability Info
                    // some case in WHCK : Athreos SoftAP will change the privacy bit of probe rsp when we really to connect it.
                    if (CurrentCapabilityInfo != CapabilityInfo)
                    {
                        if (((CapabilityInfo & 0x0010) == 0) && ((CurrentCapabilityInfo & 0x0010) == 0x0010))
                        {
                            CapabilityInfo = CapabilityInfo | 0x0010;
                            pAd->ScanTab.BssEntry[Idx].CapabilityInfo = CapabilityInfo;  // update scan table at the same time
                        }
                    }

                    if (pFrame->Hdr.FC.SubType == SUBTYPE_BEACON)
                    {
                        // For hidden SSID AP, it might send beacon with SSID len equal to 0
                        // Or send beacon /probe response with SSID len matching real SSID length,
                        // but SSID is all zero. such as "00-00-00-00" with length 4.
                        // We have to prevent this case overwrite correct table

                        pAd->ScanTab.BssEntry[Idx].Hidden = 1;
                        if (SsidLen > 0)
                        {
                            //ssid is not equal to zero ssid
                            if (!PlatformEqualMemory(Ssid, ZeroSsid, SsidLen))
                            {
                                pAd->ScanTab.BssEntry[Idx].Hidden = 0;
                            }
                        }

                        if (bConnectedToCiscoAp)
                        {
                            pAd->ScanTab.BssEntry[Idx].bIsCiscoAP  = TRUE;
                            DBGPRINT(RT_DEBUG_TRACE,("2009 PF#2: IS CiscoAP channel is %d\n",pAd->ScanTab.BssEntry[Idx].Channel));
                        }
                        else
                            pAd->ScanTab.BssEntry[Idx].bIsCiscoAP  = FALSE;
                    }
                }
            }
            PlatformMoveMemory(pAd->MlmeAux.Bssid, Bssid, MAC_ADDR_LEN);
            pAd->MlmeAux.CapabilityInfo = CapabilityInfo & SUPPORTED_CAPABILITY_INFO;

            pAd->MlmeAux.BssType = BssType;
            pAd->MlmeAux.BeaconPeriod = BeaconPeriod;
            pPort->Channel = Channel;
            pAd->MlmeAux.AtimWin = AtimWin;
            pAd->MlmeAux.CfpPeriod = Cf.CfpPeriod;
            pAd->MlmeAux.CfpMaxDuration = Cf.CfpMaxDuration;
            pAd->MlmeAux.APRalinkIe = RalinkIe;

            // Copy AP's supported rate to MlmeAux for creating assoication request
            // Also filter out not supported rate
            pAd->MlmeAux.SupRateLen = SupRateLen;
            PlatformMoveMemory(pAd->MlmeAux.SupRate, SupRate, SupRateLen);
            RTMPCheckRates(pAd, pAd->MlmeAux.SupRate, &pAd->MlmeAux.SupRateLen);
            pAd->MlmeAux.ExtRateLen = ExtRateLen;
            PlatformMoveMemory(pAd->MlmeAux.ExtRate, ExtRate, ExtRateLen);
            RTMPCheckRates(pAd, pAd->MlmeAux.ExtRate, &pAd->MlmeAux.ExtRateLen);

            // CH14 only supports 11b
            if (pPort->CommonCfg.Ch14BOnly && (pPort->Channel == CHANNEL_14))
            {
                pAd->MlmeAux.SupRateLen = 4;
                pAd->MlmeAux.ExtRateLen = 0;

                HtCapabilityLen = 0;
                PreNHtCapabilityLen = 0;
            }

            pAd->MlmeAux.NewExtChannelOffset = NewExtChannelOffset;
            pAd->MlmeAux.HtCapabilityLen = HtCapabilityLen;

            // Use the owner of P2P wcid in concurrent Client
            if (IS_P2P_CON_CLI(pAd, pPort) || IS_P2P_MS_CLI(pAd, pPort) || IS_P2P_MS_CLI2(pAd, pPort))
            {
                Wcid = P2pGetClientWcid(pAd, pPort);

#ifdef MULTI_CHANNEL_SUPPORT
#else
                // P2P attempts to join BSS and Port0 connected with legacy AP
                if(INFRA_ON(pAd->PortList[PORT_0])
                    && (pAd->PortList[PORT_0]->PortSubtype == PORTSUBTYPE_STA)
                    && (pAd->StaActive.SupportedHtPhy.bHtEnable == FALSE))
                {
                    P2pNModeEnable = FALSE;
                    if (pPort->P2PCfg.P2pPhyMode != P2P_PHYMODE_LEGACY_ONLY)
                        pPort->P2PCfg.P2pPhyMode = P2P_PHYMODE_LEGACY_ONLY;
                    DBGPRINT(RT_DEBUG_TRACE, ("%s: P2pClient keeps legacy mode as the same as InfraSta\n", __FUNCTION__));
                }
#endif /*MULTI_CHANNEL_SUPPORT*/                
            }
            pWcidMacTabEntry = MlmeSyncMacTabMatchedWlanIdx(pPort, Wcid);   

            if(pWcidMacTabEntry == NULL)
            {
                DbgPrint("Entry is NULL  Wcid = %d\n",Wcid);
                return;
            }
            // filter out un-supported ht rates
            if (((HtCapabilityLen > 0) || (PreNHtCapabilityLen > 0)) &&
                (pPort->CommonCfg.PhyMode >= PHY_11ABGN_MIXED) &&
                ((BssType==BSS_ADHOC)?(pAd->StaCfg.bAdhocNMode==TRUE):(TRUE)) &&
                (P2pNModeEnable == TRUE))
            {
                PlatformZeroMemory(&pAd->MlmeAux.HtCapability, SIZE_HT_CAP_IE);
                PlatformMoveMemory(&pAd->MlmeAux.AddHtInfo, &AddHtInfo, SIZE_ADD_HT_INFO_IE);
                PlatformMoveMemory(&pAd->MlmeAux.ExtCapIE, &ExtCapIE, sizeof(ExtCapIE));

                if (pPort->CommonCfg.BACapability.field.b2040CoexistScanSup)
                    pPort->CommonCfg.ExtCapIE.field.BssCoexstSup = 1;

                // StaActive.SupportedHtPhy.MCSSet stores Peer AP's 11n Rx capability
                if (IDLE_ON(pAd->PortList[PORT_0]))
                    PlatformMoveMemory(pAd->StaActive.SupportedHtPhy.MCSSet, HtCapability.MCSSet, 16);
                // Also copy another MCSSet to pEntry
                PlatformMoveMemory(pWcidMacTabEntry->HTCapability.MCSSet, HtCapability.MCSSet, 16);

                pAd->MlmeAux.NewExtChannelOffset = NewExtChannelOffset;
                pAd->MlmeAux.HtCapabilityLen = SIZE_HT_CAP_IE;
                pAd->StaActive.SupportedHtPhy.bHtEnable = TRUE;
                if (PreNHtCapabilityLen > 0)
                    pAd->StaActive.SupportedHtPhy.bPreNHt = TRUE;

                if(bVhtCapable) // the target AP is VHT capable
                {
                    UCHAR bandwidth = 0;
                    UCHAR ExtChanOffset=0;
                    //UCHAR PrimaryChannelLocation;
                    if (VhtOperation.VhtOpInfo.ChannelWidth == VHT_BW_80)
                    {
                        CentralChannel = VhtOperation.VhtOpInfo.ChannelCenterFreqSeg1;
                        bandwidth = BW_80;
                        GetPrimaryChannelLocation(
                                pAd,
                                Channel,
                                BW_80,
                                CentralChannel,
                                &ExtChanOffset
                        );
                            
                    }
                    else if (VhtOperation.VhtOpInfo.ChannelWidth == VHT_BW_20_40)
                    {
                        // central channel and bandwidth depends on the setting of HT
                        if ((AddHtInfo.ControlChan > 2)&& (AddHtInfo.AddHtInfo.ExtChanOffset == EXTCHA_BELOW) && (HtCapability.HtCapInfo.ChannelWidth == BW_40))
                        {
                            CentralChannel = AddHtInfo.ControlChan - 2;
                            bandwidth = BW_40;
                        }
                        else if ((AddHtInfo.AddHtInfo.ExtChanOffset == EXTCHA_ABOVE) && (HtCapability.HtCapInfo.ChannelWidth == BW_40))
                        {
                            CentralChannel = AddHtInfo.ControlChan + 2;
                            bandwidth = BW_40;
                        }
                        else
                        {
                            CentralChannel = Channel;
                            bandwidth = BW_20;
                            DBGPRINT(RT_DEBUG_TRACE,("%s, LINE%d, bw=20 = %d\n",__FUNCTION__,__LINE__));
                        }
                        ExtChanOffset = AddHtInfo.AddHtInfo.ExtChanOffset;
                    }
                    else
                    {
                        DBGPRINT(RT_DEBUG_TRACE,("ERROR!! unsupported VhtOpInfo.ChannelWidth(=%d)\n",VhtOperation.VhtOpInfo.ChannelWidth));
                    }                   
                    pPort->CentralChannel = CentralChannel;                       
                    DBGPRINT(RT_DEBUG_TRACE,("%s, LINE%d, pAd->MlmeAux.CentralChannel = %d, bandwidth=%d\n",
                        __FUNCTION__,
                        __LINE__,
                        pPort->CentralChannel,
                        bandwidth));
                    /*
                    MlmeCntlChannelWidth(
                            pAd, 
                            Channel,                                    // primary
                            pAd->MlmeAux.CentralChannel,                // central
                            bandwidth,                                  // ChannelWidth
                            ExtChanOffset);                             // PrimaryChannelOffset for VHT, ExtChanOffset for HT
                    */
                }               
                else if (AddHtInfoLen > 0)
                {
                    CentralChannel = AddHtInfo.ControlChan;
                    // Check again the Bandwidth capability of this AP.
                    if ((AddHtInfo.ControlChan > 2)&& (AddHtInfo.AddHtInfo.ExtChanOffset == EXTCHA_BELOW) && (HtCapability.HtCapInfo.ChannelWidth == BW_40))
                    {
                        CentralChannel = AddHtInfo.ControlChan - 2;
                    }
                    else if ((AddHtInfo.AddHtInfo.ExtChanOffset == EXTCHA_ABOVE) && (HtCapability.HtCapInfo.ChannelWidth == BW_40))
                    {
                        CentralChannel = AddHtInfo.ControlChan + 2;
                    }

                    pPort->CentralChannel = CentralChannel;

//#ifdef MULTI_CHANNEL_SUPPORT
                    MultiChannelSetCentralCh(pAd, pPort, CentralChannel, AddHtInfo.AddHtInfo.ExtChanOffset, HtCapability.HtCapInfo.ChannelWidth);
//#endif /*MULTI_CHANNEL_SUPPORT*/

                    //
                    // Patch CiscoAP 1520: (AP setting: Just to support 1Mbps and MCS 0~2). Motorola AP support "BSS membership is HT-PHY"
                    // It will send Association Rsp using BW40. If we can't receive it, it will just use 1Mbps for Tx.
                    //
                    if( ((pPort->Channel > 14)  && (pAd->HwCfg.NicConfig2.field.bDisableBW40ForA)) ||  // BW40 is NOT allowed for A band
                        ((pPort->Channel <= 14) && (pAd->HwCfg.NicConfig2.field.bDisableBW40ForG)) )   // BW40 is NOT allowed for G band
                    {
                        // BW40 is NOT allowed
                        ;
                    }
                    else
                    {
                        // (1) WiFi 11n testplan 5.2.36 using Sigma. BRCM AP will use HT40 fro Assoc-Rsp
                        // (2) Fixed Rate CiscoAP
                        // (3) Moto AP: SupRate = 0xFF or SupRatelen = 0
                        if(bVhtCapable==TRUE)
                        {
                            // skip the following patch for VHT
                        }                       
                        else if ((((pAd->StaCfg.Feature11n&0x01) == 0x01)||(bConnectedToCiscoAp)||((pAd->MlmeAux.SupRateLen ==1)&&(pAd->MlmeAux.SupRate[0] == 0xFF))||(pAd->MlmeAux.SupRateLen ==0)) 
                            //(HtCapability.HtCapInfo.ChannelWidth == BW_40) && 
                            && (AddHtInfo.AddHtInfo.RecomWidth == BW_40) 
                            && (BssType == BSS_INFRA))
                        {
                            DBGPRINT(RT_DEBUG_TRACE, ("%s==>bConnectedToCiscoAp = %d, SupRateLen = %d, SupRate[0] = 0x%02x\n",
                                        __FUNCTION__,
                                        bConnectedToCiscoAp, 
                                        pAd->MlmeAux.SupRateLen, 
                                        pAd->MlmeAux.SupRate[0]));
                            DBGPRINT(RT_DEBUG_TRACE, ("%s==>HtCapability.HtCapInfo.ChannelWidth = %d, AddHtInfo.AddHtInfo.RecomWidth = %d, Feature11n = 0x%x\n",
                                        __FUNCTION__,
                                        HtCapability.HtCapInfo.ChannelWidth, 
                                        AddHtInfo.AddHtInfo.RecomWidth, 
                                        pAd->StaCfg.Feature11n));                           

                            MlmeCntlChannelWidth(pAd, AddHtInfo.ControlChan, pPort->CentralChannel, (UCHAR)HtCapability.HtCapInfo.ChannelWidth, (UCHAR)AddHtInfo.AddHtInfo.ExtChanOffset);    
                            pAd->StaCfg.Feature11n |= 0x02; //toggle flag for bit0
                        }
                    }
                    DBGPRINT(RT_DEBUG_TRACE, ("MlmeSyncMlmeSyncPeerBeaconAtJoinAction HT===>Central Channel = %d, Control Channel = %d,  .\n", CentralChannel, AddHtInfo.ControlChan));

                }

                if (pPort->CommonCfg.TgnControl.field.IntolerantAssocReq40MHz == TRUE)
                {
                    pAd->MlmeAux.HtCapability.HtCapInfo.Intolerant40 = 1;
                    DBGPRINT(RT_DEBUG_TRACE, ("2009 PF#1: MlmeSyncMlmeSyncPeerBeaconAtJoinAction HT===>Intolerant40 ON\n"));
                }

                RTMPCheckHt(pAd, pPort, Wcid, &HtCapability, &AddHtInfo, bVhtCapable, &VhtCapability);

                if ((pPort->CommonCfg.PhyMode==PHY_11VHT) // 1. we do support VHT
                    && (bVhtCapable==TRUE))             // 2. the other side support VHT
                {
                    RTMPCheckVht(pAd, pPort, Wcid, &VhtCapability, &VhtOperation);
                }

                // Decide bandwidth when P2pClient will connect to AP and Port0 has connected with AP.
                if (IS_P2P_CON_CLI(pAd, pPort) || (pPort->PortType == WFD_CLIENT_PORT)/*IS_P2P_MS_CLI(pAd, pPort) || IS_P2P_MS_CLI2(pAd, pPort)*/)
                {
                    if (FALSE == P2pDecideHtBw(pAd, pPort))
                    {
                        DBGPRINT(RT_DEBUG_TRACE, ("Case 2: Resume group due to different central channel(%d %d).\n", pPort->CentralChannel, pPort->CentralChannel));
                        pPort->P2PCfg.GroupFormErrCode = P2P_ERRCODE_CHANNELCONFLICT;
                        pPort->P2PCfg.P2pEventQueue.bErrorCode = TRUE;
#if(COMPILE_WIN8_ABOVE(CURRENT_OS_NDIS_VER))  
                        if(pPort->PortType == WFD_CLIENT_PORT)
                            P2pMsDown(pAd, pPort);
                        else
#endif
                            P2pDown(pAd, pPort);

                        // Delete P2P Device that I previously tried to connect.
                        for (Index = 0; Index < MAX_P2P_GROUP_SIZE; Index++)
                        {
                            if (pAd->pP2pCtrll->P2PTable.Client[Index].P2pClientState > P2PSTATE_DISCOVERY_UNKNOWN)
                                P2pGroupTabDelete(pAd, Index, pAd->pP2pCtrll->P2PTable.Client[Index].addr);
                        }
                        P2pGotoScan(pAd, pPort, P2P_DISCO_TYPE_DEFAULT, 2);

                        if (VarIE)
                            PlatformFreeMemory(VarIE, MAX_VIE_LEN);

                        if (WSCInfoAtBeacons)
                            PlatformFreeMemory(WSCInfoAtBeacons, MAX_VIE_LEN);

                        if (WSCInfoAtProbeRsp)
                            PlatformFreeMemory(WSCInfoAtProbeRsp, MAX_VIE_LEN);

                        return;

                    }

                }

                // Copy AP Parameter to StaActive.  This is also in MlmeCntLinkUp.
                DBGPRINT(RT_DEBUG_TRACE, ("%s!!!  ( MpduDensity=%d, MaxRAmpduFactor=%d,)\n", __FUNCTION__,
                    pAd->StaActive.SupportedHtPhy.MpduDensity, pAd->StaActive.SupportedHtPhy.MaxRAmpduFactor));
                DBGPRINT(RT_DEBUG_TRACE, ("%s!!!  ( BW=%d, MaxRAmpduFactor=%d,)\n", __FUNCTION__,
                    pAd->MlmeAux.HtCapability.HtCapInfo.ChannelWidth, pAd->StaActive.SupportedHtPhy.MaxRAmpduFactor));

            }
            else
            {
                // To prevent error, let legacy AP must have same CentralChannel and Channel.
                if ((HtCapabilityLen == 0) && (PreNHtCapabilityLen == 0))
                    pPort->CentralChannel = pPort->Channel;

                // To fix rate error when switch between N's AP and Legacy's AP
                PlatformZeroMemory(pAd->StaActive.SupportedHtPhy.MCSSet, 16);

                pAd->StaActive.SupportedHtPhy.bHtEnable = FALSE;
                if(pPort->CommonCfg.PhyMode < PHY_11ABGN_MIXED)
                    pAd->MlmeAux.HtCapabilityLen = 0;
                PlatformZeroMemory(&pAd->MlmeAux.HtCapability, SIZE_HT_CAP_IE);
                PlatformZeroMemory(&pAd->MlmeAux.AddHtInfo, SIZE_ADD_HT_INFO_IE);
                DBGPRINT(RT_DEBUG_TRACE, ("%s: HT Disable\n", __FUNCTION__)); 
            }

            PeerTxType = PeerTxTypeInUseSanity(Channel, SupRate, SupRateLen, ExtRate, ExtRateLen);
            RTMPUpdateMlmeRate(pAd,Channel);

            // copy QOS related information
            if ((pPort->CommonCfg.PhyMode >= PHY_11ABGN_MIXED) || (pPort->CommonCfg.bWmmCapable))
            {
                PlatformMoveMemory(&pAd->MlmeAux.APEdcaParm, &EdcaParm, sizeof(EDCA_PARM));
                PlatformMoveMemory(&pAd->MlmeAux.APQbssLoad, &QbssLoad, sizeof(QBSS_LOAD_PARM));
                PlatformMoveMemory(&pAd->MlmeAux.APQosCapability, &QosCapability, sizeof(QOS_CAPABILITY_PARM));
            }
            else
            {
                PlatformZeroMemory(&pAd->MlmeAux.APEdcaParm, sizeof(EDCA_PARM));
                PlatformZeroMemory(&pAd->MlmeAux.APQbssLoad, sizeof(QBSS_LOAD_PARM));
                PlatformZeroMemory(&pAd->MlmeAux.APQosCapability, sizeof(QOS_CAPABILITY_PARM));
            }

            //Vista SP1's FIPS mode should disable WMM. OS sets SafeModeQOS_PARAMS to zero(from OID_DOT11_QOS_PARAMS setting)
            if( (pPort->CommonCfg.bSafeModeEnabled == 1 ) && (pPort->CommonCfg.SafeModeQOS_PARAMS == 0) )
            {
                pPort->CommonCfg.bWmmCapable = 0;
                PlatformZeroMemory(&pAd->MlmeAux.APEdcaParm, sizeof(EDCA_PARM));
                PlatformZeroMemory(&pAd->MlmeAux.APQbssLoad, sizeof(QBSS_LOAD_PARM));
                PlatformZeroMemory(&pAd->MlmeAux.APQosCapability, sizeof(QOS_CAPABILITY_PARM));
                OPSTATUS_CLEAR_FLAG(pPort, fOP_STATUS_WMM_INUSED);
            }

            DBGPRINT(RT_DEBUG_TRACE, ("SYNC - EdcaParm.bValid=%d\n",  EdcaParm.bValid));
            DBGPRINT(RT_DEBUG_TRACE, ("SYNC - after JOIN, SupRateLen=%d, ExtRateLen=%d\n",
                pAd->MlmeAux.SupRateLen, pAd->MlmeAux.ExtRateLen));

            //
            // In AdHoc Mode, If join response occurs, it indicates I am a AdHoc Joiner. Not a Creator.
            //
            if( BssType == BSS_ADHOC)
            {
                pAd->StaCfg.AdhocCreator = FALSE;
                pAd->StaCfg.AdhocJoiner  = TRUE;
            }

            //
            // The maximum transmit power calculation is based on the maximum transmit power level specified for the channel in the country element and 
            // the local power constraint specified for the channel in the power constraint element.
            //
            // Note that the Cisco AP has the proprietary power control methodology.
            //
            if ((pAd->StaCfg.MDSMCtrl.dot11MultiDomainCapabilityEnabled == TRUE) && 
                 (MaxTxPowerLevel != MDSM_ABSENT_COUNTRY_IE))
            {
                if (MDSMCalculateMaxTxPower(pAd, MaxTxPowerLevel, LocalPowerConstraint) == TRUE)
                {
                    pAd->StaCfg.MDSMCtrl.associatedStaMSDMCapable.bMDSMCapable = TRUE;
                }
                else // Calculation failure
                {
                    pAd->StaCfg.MDSMCtrl.associatedStaMSDMCapable.bMDSMCapable = FALSE;

                    // Use normal transmit power instead
                    pPort->CommonCfg.TxPowerPercentage = pPort->CommonCfg.TxPowerDefault;
                }
            }
            else  //Used the default TX Power Percentage.
            {
                pAd->StaCfg.MDSMCtrl.associatedStaMSDMCapable.bMDSMCapable = FALSE;
                pPort->CommonCfg.TxPowerPercentage = pPort->CommonCfg.TxPowerDefault;
            }

            //
            // Save this beacon frame for NDIS_STATUS_DOT11_ASSOCIATION_COMPLETION
            //
            PlatformMoveMemory(pAd->StaCfg.BeaconFrame, Add2Ptr(Elem->Msg, LENGTH_802_11), Elem->MsgLen - LENGTH_802_11);
            pAd->StaCfg.BeaconFrameSize = Elem->MsgLen - LENGTH_802_11;
            
            if (pAd->StaCfg.BssType == BSS_ADHOC)
            {
                //
                // Add this Adhoc to AdhocList
                //
                NdisAcquireSpinLock(&pAd->StaCfg.AdhocListLock);
                pAdhocList = (PRTMP_ADHOC_LIST)RemoveHeadQueue(&pAd->StaCfg.FreeAdhocListQueue);
                if (pAdhocList)
                {
                    pAdhocList->AssocState = dot11_assoc_state_auth_assoc;
                    pAdhocList->BeaconInterval = BeaconPeriod;
                    pAdhocList->CapabilityInfo = CapabilityInfo;
                    COPY_MAC_ADDR(pAdhocList->Bssid, Bssid);
                    COPY_MAC_ADDR(pAdhocList->PeerMacAddress, Addr2);
                    NdisGetCurrentSystemTime((PLARGE_INTEGER)&pAdhocList->ullHostTimeStamp);
                    pAdhocList->bKeySet = FALSE;

                    InsertTailQueue(&pAd->StaCfg.ActiveAdhocListQueue, pAdhocList);                 
                }
                NdisReleaseSpinLock(&pAd->StaCfg.AdhocListLock);

                //fix for ndistest adhoc association info query
                OPSTATUS_SET_FLAG(pPort, fOP_STATUS_ADHOC_ON);
                OPSTATUS_CLEAR_FLAG(pPort, fOP_STATUS_INFRA_ON);
                pAd->StaCfg.ActivePhyId = MlmeInfoGetPhyIdByChannel(pAd, pPort->Channel);
            
                //indicate adhoc association
                OPSTATUS_SET_FLAG(pPort, fOP_STATUS_MEDIA_STATE_CONNECTED);
                    PlatformIndicateAdhocAssociation(pAd,pPort, Addr2);
            }

            pPort->Mlme.SyncMachine.CurrState = SYNC_IDLE;
            Status = MLME_SUCCESS;
            MlmeEnqueue(pAd,pPort, MLME_CNTL_STATE_MACHINE, MT2_JOIN_CONF, 2, &Status);

            if (pPort->bStartJoin == TRUE)
            {
                pPort->bStartJoin = FALSE;
            }
        }
        // not to me BEACON, ignored
        else /* if (MAC_ADDR_EQUAL(pAd->MlmeAux.Bssid, Bssid) && SsidLen == 0)*/
        {
            DBGPRINT(RT_DEBUG_INFO, ("re-send probe req for Hidden SSID pPort->bStartJoin = %d, pAd->pNicCfg->OutBufferForSendProbeReq = %x, pAd->pNicCfg->OutBufferForSendProbeReqLen = %d\n", pPort->bStartJoin, pAd->pNicCfg->OutBufferForSendProbeReq, pAd->pNicCfg->OutBufferForSendProbeReqLen));
            if ((pPort->bStartJoin == TRUE) && (pAd->pNicCfg->OutBufferForSendProbeReq != NULL) && (pAd->pNicCfg->OutBufferForSendProbeReqLen != 0))
            {
                NDIS_STATUS     NStatus;
                PUCHAR      pOutBuffer = NULL;
                DBGPRINT(RT_DEBUG_INFO, ("2 re-send probe req for Hidden SSID \n"));
                NStatus = MlmeAllocateMemory(pAd, &pOutBuffer);
                //RTMP_ALLOCATE_MEMORY(pAd->AdapterHandle, &pAd->pNicCfg->OutBufferForSendProbeReq, MAX_VIE_LEN, NIC_TAG);
                if (NStatus == NDIS_STATUS_SUCCESS)
                {
                    PlatformMoveMemory(pOutBuffer, pAd->pNicCfg->OutBufferForSendProbeReq, pAd->pNicCfg->OutBufferForSendProbeReqLen);
                    NdisCommonMiniportMMRequest(pAd, pOutBuffer, pAd->pNicCfg->OutBufferForSendProbeReqLen);
                }
            }
                
        }
    } 
    // sanity check fail, ignore this frame

    if (VarIE)
        PlatformFreeMemory(VarIE, MAX_VIE_LEN);

    if (WSCInfoAtBeacons)
        PlatformFreeMemory(WSCInfoAtBeacons, MAX_VIE_LEN);

    if (WSCInfoAtProbeRsp)
        PlatformFreeMemory(WSCInfoAtProbeRsp, MAX_VIE_LEN);
}

/* 
    ==========================================================================
    Description:
        receive BEACON from peer

    IRQL = PASSIVE_LEVEL

    ==========================================================================
 */
VOID MlmeSyncPeerBeacon(
    IN PMP_ADAPTER pAd, 
    IN MLME_QUEUE_ELEM *Elem) 
{
    UCHAR         Bssid[MAC_ADDR_LEN], Addr2[MAC_ADDR_LEN];
    CHAR          Ssid[MAX_LEN_OF_SSID];
    CF_PARM       CfParm;
    UCHAR         SsidLen, MessageToMe = 0, BssType, Channel, NewChannel, index = 0;
    UCHAR         DtimCount = 0, DtimPeriod = 0, BcastFlag = 0;
    USHORT        CapabilityInfo, AtimWin, BeaconPeriod;
    ULONGLONG     TimeStamp;
//  USHORT        TbttNumToNextWakeUp;
    UCHAR         Erp;
    UCHAR         SupRate[MAX_LEN_OF_SUPPORTED_RATES], ExtRate[MAX_LEN_OF_SUPPORTED_RATES];
    UCHAR         SupRateLen, ExtRateLen;
    USHORT        LenVIE;
    EDCA_PARM       EdcaParm;
    QBSS_LOAD_PARM  QbssLoad;
    QOS_CAPABILITY_PARM QosCapability;
    ULONG           RalinkIe;
    // New for WPA security suites
    PUCHAR                      VarIE = NULL; 
    NDIS_802_11_VARIABLE_IEs    *pVIE = NULL;
    HT_CAPABILITY_IE        HtCapability;
    ADD_HT_INFO_IE      AddHtInfo;  // AP might use this additional ht info IE 
    UCHAR           HtCapabilityLen, PreNHtCapabilityLen;
    UCHAR           AddHtInfoLen;
    UCHAR           NewExtChannelOffset = 0xff;
    PRTMP_ADHOC_LIST            pAdhocList;
    BOOLEAN             bfound = FALSE;
    BOOLEAN             bRoamingStart = FALSE;
    BOOLEAN             bIsMerge = FALSE;
    EXT_CAP_ELEMT       ExtCapIE;   // this is the extened capibility IE  
    SSIDL_IE            *pSSIDL_VIE = NULL;

    PUCHAR          WSCInfoAtBeacons = NULL;
    USHORT          WSCInfoAtBeaconsLen;
    PUCHAR          WSCInfoAtProbeRsp = NULL;
    USHORT          WSCInfoAtProbeRspLen;   
    UCHAR               SSIDL_VIE[256];     // SSIDL is limited to 256

    // New for VHT
    BOOLEAN           bVhtCapable=FALSE;
    VHT_CAP_IE        VhtCapability;
    VHT_OP_IE         VhtOperation;

    PMP_PORT      pPort = pAd->PortList[Elem->PortNum];
    INT i;
    BOOLEAN             bUpdateRssi;
    UCHAR               bConnectedToCiscoAp;
    UCHAR MaxTxPowerLevel = MDSM_ABSENT_COUNTRY_IE;
    UCHAR LocalPowerConstraint = MDSM_ABSENT_POWER_CONSTRAINT_IE;
    PMP_PORT pApPort;
    //UCHAR     BBPValue;
    BOOLEAN bHasOperatingModeField = FALSE;
    OPERATING_MODE_FIELD OperatingModeField = {0};
    
    if(pPort->PortType == EXTAP_PORT)
        return;

#if 0  // to test, so mark it
#ifdef MULTI_CHANNEL_SUPPORT
    if (!INFRA_ON(pPort) && !ADHOC_ON(pPort) && !IS_P2P_CLIENT_OP(pPort) && (!pPort->bRoamingAfterResume) && !IS_P2P_MS_CLI(pAd, pPort) && !IS_P2P_MS_CLI2(pAd, pPort))
    {
        return;
    }
#else
    if (!INFRA_ON(pPort) && !ADHOC_ON(pPort) && !IS_P2P_CLIENT_OP(pPort) && (!pPort->bRoamingAfterResume))
    {
        return;
    }
#endif /*MULTI_CHANNEL_SUPPORT*/
#endif

    // init SSIDL IE
    PlatformZeroMemory(SSIDL_VIE, sizeof(SSIDL_VIE));
    pSSIDL_VIE = (PSSIDL_IE) SSIDL_VIE;

    PlatformAllocateMemory(pAd, &WSCInfoAtBeacons, MAX_VIE_LEN);
    PlatformAllocateMemory(pAd, &WSCInfoAtProbeRsp, MAX_VIE_LEN);

    // Init Variable IE structure
   PlatformAllocateMemory(pAd, &VarIE, MAX_VIE_LEN);
    if (VarIE == NULL)
    {
        if (WSCInfoAtBeacons)
            PlatformFreeMemory(WSCInfoAtBeacons, MAX_VIE_LEN);

        if (WSCInfoAtProbeRsp)
            PlatformFreeMemory(WSCInfoAtProbeRsp, MAX_VIE_LEN);
    
        DBGPRINT(RT_DEBUG_ERROR, ("%s::Allocate memory size(=1024) failed\n", __FUNCTION__));   
        return;
    }
        
    pVIE = (PNDIS_802_11_VARIABLE_IEs) VarIE;
    pVIE->Length = 0;

    PlatformZeroMemory(&HtCapability, sizeof(HT_CAPABILITY_IE));    
    PlatformZeroMemory(&VhtCapability, sizeof(VHT_CAP_IE));
    PlatformZeroMemory(&VhtOperation, sizeof(VHT_OP_IE));

    // init SSIDL IE
    PlatformZeroMemory(SSIDL_VIE, sizeof(SSIDL_VIE));
    pSSIDL_VIE = (PSSIDL_IE) SSIDL_VIE;
    
    if (MlmeSyncPeerBeaconAndProbeRspSanity(pAd, 
                                pPort,
                                Elem->Msg, 
                                Elem->MsgLen, 
                                Addr2, 
                                Bssid, 
                                Ssid, 
                                &SsidLen, 
                                &BssType, 
                                &BeaconPeriod, 
                                &Channel, 
                                &NewChannel,
                                &TimeStamp, 
                                &CfParm, 
                                &AtimWin, 
                                &CapabilityInfo, 
                                &Erp,
                                &DtimCount, 
                                &DtimPeriod, 
                                &BcastFlag, 
                                &MessageToMe, 
                                SupRate,
                                &SupRateLen,
                                ExtRate,
                                &ExtRateLen,
                                &EdcaParm,
                                &QbssLoad,
                                &QosCapability,
                                &RalinkIe,
                                &HtCapabilityLen,
                                &PreNHtCapabilityLen,
                                &HtCapability,
                                &bVhtCapable,
                                &VhtCapability,
                                &VhtOperation,
                                &AddHtInfoLen,
                                &AddHtInfo,
                                &ExtCapIE,
                                &NewExtChannelOffset,
                                &WSCInfoAtBeaconsLen,
                                WSCInfoAtBeacons,
                                &WSCInfoAtProbeRspLen,
                                WSCInfoAtProbeRsp,
                                pSSIDL_VIE,
                                &MaxTxPowerLevel, 
                                &LocalPowerConstraint,
                                &bConnectedToCiscoAp,
                                &bHasOperatingModeField,
                                &OperatingModeField,
                                &LenVIE,
                                pVIE)) 
    {
        BOOLEAN     is_my_bssid, is_my_ssid;
        ULONG       Bssidx; 
        ULONGLONG Now64;
        BSS_ENTRY   *pBss;
        CHAR        RealRssi = RTMPMaxRssi(pAd, MlmeSyncConvertToRssi(pAd, Elem->Rssi, RSSI_0, Elem->AntSel, BW_20), MlmeSyncConvertToRssi(pAd, Elem->Rssi2, RSSI_1, Elem->AntSel, BW_20), MlmeSyncConvertToRssi(pAd, Elem->Rssi3, RSSI_2, Elem->AntSel, BW_20));       


        // If I already know my ip and not sent to AP yet. need to monitoring the beacon's request.
        if ((IS_P2P_CLIENT_OP(pPort) || MT_TEST_BIT(pAd->pP2pCtrll->P2pMsConnectedStatus, P2pMs_CONNECTED_STATUS_CLI_OP))
            && PORT_P2P_ON(pPort))
        {
            PeerP2pBeacon(pAd, pPort, Addr2, Elem);
        }
        
        //RealRssi = MlmeSyncConvertToRssi(pAd, Elem->Rssi, RSSI_0, Elem->AntSel, BW_20);
        if (pPort->bRoamingAfterResume)
        {           
            if (MAC_ADDR_EQUAL(Bssid, pPort->CurrentBssEntry.Bssid))
            {
                NdisGetCurrentSystemTime((PLARGE_INTEGER)&Now64);
                pPort->CurrentBssEntry.LastBeaconRxTime = Now64;
                DBGPRINT(RT_DEBUG_TRACE, ("[080821]%s, receive beacon for roaming...\n", __FUNCTION__));    

#if(COMPILE_WIN8_ABOVE(CURRENT_OS_NDIS_VER))  
                //set NLO Peer Beacon Flag if the Beacon was received after resume.
                if(NDIS_WIN8_ABOVE(pAd) && (pAd->StaCfg.NLOEntry.NLOEnable))
                    pAd->StaCfg.NLOEntry.bNLOMlmeSyncPeerBeaconFound = TRUE;    
#endif
            }
            else if(SSID_EQUAL(pPort->PortCfg.LastSsid, pPort->PortCfg.LastSsidLen, Ssid, SsidLen))
            {
                NdisGetCurrentSystemTime((PLARGE_INTEGER)&Now64);
                pPort->CurrentBssEntry.LastBeaconRxTime = Now64;
                PlatformMoveMemory(pPort->CurrentBssEntry.Bssid,Bssid,MAC_ADDR_LEN);
                DBGPRINT(RT_DEBUG_TRACE, ("[080821]MlmeSyncPeerBeacon, receive same ssid beacon for roaming...\n"));    
            }
            if (VarIE)
                PlatformFreeMemory(VarIE, MAX_VIE_LEN);

            if (WSCInfoAtBeacons)
                PlatformFreeMemory(WSCInfoAtBeacons, MAX_VIE_LEN);

            if (WSCInfoAtProbeRsp)
                PlatformFreeMemory(WSCInfoAtProbeRsp, MAX_VIE_LEN);    
            return;
        }
        is_my_bssid = MAC_ADDR_EQUAL(Bssid, pPort->PortCfg.Bssid)? TRUE : FALSE;
        is_my_ssid = SSID_EQUAL(Ssid, SsidLen, pPort->PortCfg.Ssid, pPort->PortCfg.SsidLen)? TRUE:FALSE;

        // ignore BEACON not for my SSID
        if ((! is_my_ssid) && (! is_my_bssid))
        {
            if (VarIE)
                PlatformFreeMemory(VarIE, MAX_VIE_LEN);

            if (WSCInfoAtBeacons)
                PlatformFreeMemory(WSCInfoAtBeacons, MAX_VIE_LEN);

            if (WSCInfoAtProbeRsp)
                PlatformFreeMemory(WSCInfoAtProbeRsp, MAX_VIE_LEN);    

            DBGPRINT(RT_DEBUG_INFO, ("%s --->  ignore BEACON not for my SSID\n", __FUNCTION__));    

            // for FPGA connection
            if (0)
            {

                ULONG       Bssidx; 

                CHAR        RealRssi = RTMPMaxRssi(pAd, MlmeSyncConvertToRssi(pAd, Elem->Rssi, RSSI_0, Elem->AntSel, BW_20), MlmeSyncConvertToRssi(pAd, Elem->Rssi2, RSSI_1, Elem->AntSel, BW_20), MlmeSyncConvertToRssi(pAd, Elem->Rssi3, RSSI_2, Elem->AntSel, BW_20));       

                //
                // Housekeeping "SsidBssTab" table for later-on ROAMing usage. 
                //
                Bssidx = BssTableSearch(pAd, pPort, &pAd->MlmeAux.SsidBssTab, Bssid, Channel);
                if (Bssidx == BSS_NOT_FOUND)
                {
                    // discover new AP of this network, create BSS entry
                    Bssidx = BssTableSetEntry(pAd, pPort, &pAd->ScanTab, Bssid, Ssid, SsidLen, BssType, BeaconPeriod,
                                 &CfParm, AtimWin, CapabilityInfo, SupRate, SupRateLen, ExtRate, ExtRateLen, 
                                &HtCapability, &VhtCapability, &VhtOperation, &AddHtInfo,HtCapabilityLen,AddHtInfoLen,NewExtChannelOffset, Channel, 
                                RealRssi + pAd->HwCfg.BbpRssiToDbmDelta, TimeStamp, &EdcaParm, &QosCapability, 
                                &QbssLoad, WSCInfoAtBeaconsLen, WSCInfoAtBeacons, WSCInfoAtProbeRspLen, WSCInfoAtProbeRsp, LenVIE, pVIE, TRUE);
                    if (Bssidx == BSS_NOT_FOUND) // return if BSS table full
                    {
                        if (VarIE)
                            PlatformFreeMemory(VarIE, MAX_VIE_LEN);

                        if (WSCInfoAtBeacons)
                            PlatformFreeMemory(WSCInfoAtBeacons, MAX_VIE_LEN);

                        if (WSCInfoAtProbeRsp)
                            PlatformFreeMemory(WSCInfoAtProbeRsp, MAX_VIE_LEN);    
                        return;  
                    }
                    
                    DBGPRINT(RT_DEBUG_INFO, ("SYNC - New AP added to SsidBssTab[%d], RSSI=%d, MAC=%02x:%02x:%02x:%02x:%02x:%02x\n", 
                        Bssidx, RealRssi, Bssid[0], Bssid[1], Bssid[2], Bssid[3], Bssid[4], Bssid[5]));
                }
            }
    
            return;
        }

        // No Ht and changing bandwidth when P2pClient connects as legacy mode
        if ((IS_P2P_CON_CLI(pAd, pPort) || IS_P2P_MS_CLI(pAd, pPort) || IS_P2P_MS_CLI2(pAd, pPort)) &&
            (pPort->P2PCfg.P2pPhyMode == P2P_PHYMODE_LEGACY_ONLY))
        {
            AddHtInfoLen = 0;
        }
        
        // Copy Control channel for this BSSID.     
        if (AddHtInfoLen != 0)
            Channel = AddHtInfo.ControlChan;

        if ((HtCapabilityLen > 0) || (PreNHtCapabilityLen > 0))
            HtCapabilityLen = SIZE_HT_CAP_IE;
        //
        // Housekeeping "SsidBssTab" table for later-on ROAMing usage. 
        //
        Bssidx = BssTableSearch(pAd, pPort, &pAd->MlmeAux.SsidBssTab, Bssid, Channel);
        if (Bssidx == BSS_NOT_FOUND)
        {
            // discover new AP of this network, create BSS entry
            Bssidx = BssTableSetEntry(pAd, pPort, &pAd->MlmeAux.SsidBssTab, Bssid, Ssid, SsidLen, BssType, BeaconPeriod,
                         &CfParm, AtimWin, CapabilityInfo, SupRate, SupRateLen, ExtRate, ExtRateLen, 
                        &HtCapability, &VhtCapability, &VhtOperation, &AddHtInfo,HtCapabilityLen,AddHtInfoLen,NewExtChannelOffset, Channel, 
                        RealRssi + pAd->HwCfg.BbpRssiToDbmDelta, TimeStamp, &EdcaParm, &QosCapability, 
                        &QbssLoad, WSCInfoAtBeaconsLen, WSCInfoAtBeacons, WSCInfoAtProbeRspLen, WSCInfoAtProbeRsp, LenVIE, pVIE, TRUE);
            if (Bssidx == BSS_NOT_FOUND) // return if BSS table full
            {
                if (VarIE)
                    PlatformFreeMemory(VarIE, MAX_VIE_LEN);

                if (WSCInfoAtBeacons)
                    PlatformFreeMemory(WSCInfoAtBeacons, MAX_VIE_LEN);

                if (WSCInfoAtProbeRsp)
                    PlatformFreeMemory(WSCInfoAtProbeRsp, MAX_VIE_LEN);    
                return;  
            }
            
            DBGPRINT(RT_DEBUG_INFO, ("SYNC - New AP added to SsidBssTab[%d], RSSI=%d, MAC=%02x:%02x:%02x:%02x:%02x:%02x\n", 
                Bssidx, RealRssi, Bssid[0], Bssid[1], Bssid[2], Bssid[3], Bssid[4], Bssid[5]));
        }

        if ((pPort->CommonCfg.bIEEE80211H == 1) && (NewChannel != 0) && (Channel != NewChannel))
        {
            // Switching to channel 1 can prevent from rescanning the current channel immediately (by auto reconnection).
            // In addition, clear the MLME queue and the scan table to discard the RX packets and previous scanning results.
            //AsicSwitchChannel(pAd, 1, FALSE);
            AsicSwitchChannel(pPort, pPort->Channel, pPort->CentralChannel, pPort->BBPCurrentBW, FALSE);
            MlmeCntLinkDown(pPort, FALSE);
            MlmeQueueInit(&pAd->Mlme.Queue);
            BssTableInit(&pAd->ScanTab);            
            
            //reset Reposit Bss Table
            if(pAd->pNicCfg->RepositBssTable.pBssEntry != NULL)
            {
                for (i = 0; i < MAX_LEN_OF_BSS_TABLE; i++) 
                {
                    PlatformZeroMemory(&pAd->pNicCfg->RepositBssTable.pBssEntry[i], sizeof(BSS_ENTRY));
                }

                for(i = 0; i < MAX_LEN_OF_BSS_TABLE; i++)
                {
                    pAd->pNicCfg->RepositBssTable.ulTime[i].LowPart = 0;
                    pAd->pNicCfg->RepositBssTable.ulTime[i].HighPart = 0;
                }
            }
            
            Delay_us(1000000);     // use delay to prevent STA do reassoc

            // channel sanity check
            for (index = 0 ; index < pAd->HwCfg.ChannelListNum; index++)
            {
                if (pAd->HwCfg.ChannelList[index].Channel == NewChannel)
                {
                    pAd->MlmeAux.SsidBssTab.BssEntry[Bssidx].Channel = NewChannel;
                    pPort->Channel = NewChannel;
                    //AsicSwitchChannel(pAd, pPort->Channel,FALSE);
                    AsicSwitchChannel(pPort, pPort->Channel, pPort->CentralChannel, pPort->BBPCurrentBW, FALSE);
                    DBGPRINT(RT_DEBUG_TRACE, ("%s - STA receive channel switch announcement IE (New Channel =%d)\n", __FUNCTION__, NewChannel));
                    break;
                }
            }

            if (index >= pAd->HwCfg.ChannelListNum)
            {
                DBGPRINT_ERR(("%s(can not find New Channel=%d in ChannelList[%d]\n", __FUNCTION__, pPort->Channel, pAd->HwCfg.ChannelListNum));
            }
        }

        // if the ssid matched & bssid unmatched, we should select the bssid with large value.
        // This might happened when two STA start at the same time
        if ((! is_my_bssid) && ADHOC_ON(pPort) && (BssType == BSS_ADHOC))
        {
            // Add to safe guard adhoc wep status mismatch
            if (pPort->PortCfg.WepStatus != pAd->MlmeAux.SsidBssTab.BssEntry[Bssidx].WepStatus)
            {
                DBGPRINT(RT_DEBUG_TRACE, ("SYNC - Not matched wep status %s != %s\n"
                    , decodeCipherAlgorithm(pPort->PortCfg.WepStatus), decodeCipherAlgorithm(pAd->MlmeAux.SsidBssTab.BssEntry[Bssidx].WepStatus)));
                if (VarIE)
                    PlatformFreeMemory(VarIE, MAX_VIE_LEN);

                if (WSCInfoAtBeacons)
                    PlatformFreeMemory(WSCInfoAtBeacons, MAX_VIE_LEN);

                if (WSCInfoAtProbeRsp)
                    PlatformFreeMemory(WSCInfoAtProbeRsp, MAX_VIE_LEN);    
                return;
            }

            // collapse into the ADHOC network which has bigger BSSID value.
            //if ( !OPSTATUS_TEST_FLAG(pPort, fOP_STATUS_MEDIA_STATE_CONNECTED))
            {
                for (i = 0; i < 6; i++)
                {
                    if (Bssid[i] > pPort->PortCfg.Bssid[i])
                    {
                        DBGPRINT(RT_DEBUG_TRACE, ("SYNC - merge to the IBSS with bigger BSSID=%02x:%02x:%02x:%02x:%02x:%02x\n", 
                            Bssid[0], Bssid[1], Bssid[2], Bssid[3], Bssid[4], Bssid[5]));
                        MtAsicDisableSync(pAd);
                        COPY_MAC_ADDR(pPort->PortCfg.Bssid, Bssid);
			MtAsicSetBssid(pAd, pPort->PortCfg.Bssid, (UINT8)pPort->PortNumber);
                        MgntPktConstructIBSSBeaconFrame(pAd,pPort);        // re-build BEACON frame
                        MtAsicEnableIbssSync(pAd);    // copy BEACON frame to on-chip memory

                        PlatformIndicateAdhocRoamingStart(pAd, pPort, pPort->PortCfg.Bssid, pPort->PortCfg.Ssid, pPort->PortCfg.SsidLen);
                        PlatformIndicateRoamingCompletion(pAd, pPort, DOT11_ASSOC_STATUS_SUCCESS);

                        is_my_bssid = TRUE;
                        bIsMerge = TRUE;
                        break;
                    }
                    else if ( Bssid[i] < pPort->PortCfg.Bssid[i])
                        break;
                
                
                }
            }
        }
        DBGPRINT(RT_DEBUG_TRACE, ("SYNC - %s from %02x:%02x:%02x:%02x:%02x:%02x - Dtim=%d/%d, Rssi=%02x\n",
            __FUNCTION__,
            Bssid[0], Bssid[1], Bssid[2], Bssid[3], Bssid[4], Bssid[5], 
            DtimCount, DtimPeriod, RealRssi));

        NdisGetCurrentSystemTime((PLARGE_INTEGER)&Now64);
        pBss = &pAd->MlmeAux.SsidBssTab.BssEntry[Bssidx];
        pBss->Rssi = RealRssi + pAd->HwCfg.BbpRssiToDbmDelta;       // lastest RSSI
        pBss->LastBeaconRxTime = Now64;   // last RX timestamp

        DBGPRINT(RT_DEBUG_TRACE, ("SYNC - run here ...1\n"));

        //
        // BEACON from my BSSID - either IBSS or INFRA network
        // 
        if (is_my_bssid)
        {
            OVERLAP_BSS_SCAN_IE     BssScan;
            UCHAR                   RegClass;
            BOOLEAN                 brc;
            PMP_PORT              pSTAPort = pAd->PortList[PORT_0];
            PMAC_TABLE_ENTRY pBssidMacTabEntry = MlmeSyncMacTabMatchedRoleType(pPort, ROLE_WLANIDX_BSSID);
            PMAC_TABLE_ENTRY pWcidMacTabEntry = MlmeSyncMacTabMatchedWlanIdx(pPort, P2pGetClientWcid(pAd, pPort));  

            //ULONGLONG                 SwTSF;

            //RtmpGetSwTSF(pAd, pPort, (ULONGLONG *)&SwTSF);
            //DBGPRINT(RT_DEBUG_TRACE, ("%s - Beacon TimeStamp (%I64d), Diff(%I64d)\n", __FUNCTION__, TimeStamp, TimeStamp - SwTSF));
            
            // Read Beacon's Reg Class IE if any.
            brc = MlmeSyncPeerBeaconAndProbeRspSanity2(pAd, Elem->Msg, Elem->MsgLen, &BssScan, &RegClass);
            if (brc == TRUE)
                UpdateBssScanParm(pAd, BssScan);

            // If p2p concurrent client has connected, save LastBeaconRxTime into its own wcid
            if ((pWcidMacTabEntry !=NULL) && (IS_P2P_CON_CLI(pAd, pPort) || IS_P2P_MS_CLI(pAd, pPort) || IS_P2P_MS_CLI2(pAd, pPort)) && IS_P2P_MS_CLI_WCID(pPort, Elem->Wcid))
                pWcidMacTabEntry->LastBeaconRxTime = Now64;
            else
                pAd->StaCfg.LastBeaconRxTime = Now64;

            if ((pBssidMacTabEntry !=NULL) &&VHT_CAPABLE(pAd) && 
                (bHasOperatingModeField == TRUE) && 
                INFRA_ON(pPort) && 
                (pBssidMacTabEntry->VhtPeerStaCtrl.bOperatingModeNotificationCapable == TRUE))
            {
                PeerVHTOperatingModeNotificationAction(pAd, Elem->Wcid, 
                    OperatingModeField.ChannelWidth, OperatingModeField.RxNss, 
                    OperatingModeField.RxNssType);
            }

            DBGPRINT(RT_DEBUG_INFO,("Rx My BEACON\n"));

            // We have collected RSSI in N6UsbRxStaPacket()
            // When collecting RSSI, adjacent channel traffic should be ignored
            //
            // The maximum transmit power calculation is based on the maximum transmit power level specified for the channel in the country element and 
            // the local power constraint specified for the channel in the power constraint element.
            //
            // Note that the Cisco AP has the proprietary power control methodology.
            //
            if ((pAd->StaCfg.MDSMCtrl.dot11MultiDomainCapabilityEnabled == TRUE) && 
                 (MaxTxPowerLevel != MDSM_ABSENT_COUNTRY_IE))
            {
                if (MDSMCalculateMaxTxPower(pAd, MaxTxPowerLevel, LocalPowerConstraint) == TRUE)
                {
                    pAd->StaCfg.MDSMCtrl.associatedStaMSDMCapable.bMDSMCapable = TRUE;
                }
                else // Calculation failure
                {
                    pAd->StaCfg.MDSMCtrl.associatedStaMSDMCapable.bMDSMCapable = FALSE;

                    // Use normal transmit power instead            
                    pPort->CommonCfg.TxPowerPercentage = pPort->CommonCfg.TxPowerDefault;               
                }
            }
            else
            {
                pAd->StaCfg.MDSMCtrl.associatedStaMSDMCapable.bMDSMCapable = FALSE;
                //
                // AironetCellPowerLimit equal to 0xFF means the Cisco (ccx) "TxPower Limit" not exist.
                // Used the default TX Power Percentage, that set from UI.  
                //
                pPort->CommonCfg.TxPowerPercentage = pPort->CommonCfg.TxPowerDefault;   
            }

            DBGPRINT(RT_DEBUG_TRACE, ("SYNC - run here ...2\n"));
            
            if (INFRA_ON(pPort)) // && (pPort->CommonCfg.PhyMode == PHY_11BG_MIXED))
            {
                BOOLEAN bUseShortSlot, bUseBGProtection;
                BOOLEAN         fNoisyEnvironment;              
                PMAC_TABLE_ENTRY pBssidMacTabEntry = MlmeSyncMacTabMatchedRoleType(pPort, GetWlanRoleType(pPort));
                // decide to use/change to - 
                //      1. long slot (20 us) or short slot (9 us) time
                //      2. turn on/off RTS/CTS and/or CTS-to-self protection
                //      3. short preamble

                if(pBssidMacTabEntry == NULL)
                {
                    DBGPRINT(RT_DEBUG_TRACE,("%s(), Line:%d, Mac Entry is NULL\n", __FUNCTION__, __LINE__));
                    return;
                }

                if (IS_P2P_CON_CLI(pAd, pPort) || IS_P2P_MS_CLI(pAd, pPort) || IS_P2P_MS_CLI2(pAd, pPort))
                {
                    UCHAR   wcid = P2pGetClientWcid(pAd, pPort);
                    PMAC_TABLE_ENTRY pWcidMacTabEntry = MlmeSyncMacTabMatchedWlanIdx(pPort, wcid);

                    if(pWcidMacTabEntry == NULL)
                    {
                        DBGPRINT(RT_DEBUG_TRACE,("%s(), Line:%d, Mac Entry is NULL  Wcid = %d\n", __FUNCTION__, __LINE__, wcid));
                        return;
                    }
                    
                    fNoisyEnvironment = pWcidMacTabEntry->fNoisyEnvironment;
                }
                else
                {
                    fNoisyEnvironment = pBssidMacTabEntry->fNoisyEnvironment;
                }

                bUseShortSlot = pPort->CommonCfg.bUseShortSlotTime && CAP_IS_SHORT_SLOT(CapabilityInfo);
                if (bUseShortSlot != OPSTATUS_TEST_FLAG(pAd, fOP_STATUS_SHORT_SLOT_INUSED))
                    MtAsicSetSlotTime(pAd, bUseShortSlot, pPort->Channel);

                bUseBGProtection = (pPort->CommonCfg.UseBGProtection == 1) ||    // always use
                                   ((pPort->CommonCfg.UseBGProtection == 0) && ERP_IS_USE_PROTECTION(Erp));

                if (pPort->Channel > 14) // always no BG protection in A-band. falsely happened when switching A/G band to a dual-band AP
                    bUseBGProtection = FALSE;

                //
                // Only using OFDM rate need to check ERP information protection bit.
                // Cisco AP(1242) attached ERP information even on 802.11b
                // 
                if ((pPort->CommonCfg.MaxTxRate > RATE_11) && (bUseBGProtection != OPSTATUS_TEST_FLAG(pAd, fOP_STATUS_BG_PROTECTION_INUSED)))
                {
                    if (!((pPort->PortSubtype == PORTSUBTYPE_STA) && (fNoisyEnvironment == TRUE)))
                    {
                        if (bUseBGProtection)
                        {
                            OPSTATUS_SET_FLAG(pAd, fOP_STATUS_BG_PROTECTION_INUSED);
                            MtAsicUpdateProtect(pAd, pAd->MlmeAux.AddHtInfo.AddHtInfo2.OperaionMode, (OFDMSETPROTECT|CCKSETPROTECT|ALLN_SETPROTECT),FALSE,(pAd->MlmeAux.AddHtInfo.AddHtInfo2.NonGfPresent == 1));
                            DBGPRINT(RT_DEBUG_INFO, ("SYNC - Turn On BG Protection \n"));
                        }
                        else
                        {
                            OPSTATUS_CLEAR_FLAG(pAd, fOP_STATUS_BG_PROTECTION_INUSED);
                            OPSTATUS_CLEAR_FLAG(pAd, fOP_STATUS_USECCK_PROTECTION_ENABLE);
                            MtAsicUpdateProtect(pAd, pAd->MlmeAux.AddHtInfo.AddHtInfo2.OperaionMode, (OFDMSETPROTECT|CCKSETPROTECT|ALLN_SETPROTECT),TRUE,(pAd->MlmeAux.AddHtInfo.AddHtInfo2.NonGfPresent == 1));
                            DBGPRINT(RT_DEBUG_INFO, ("SYNC - Turn Off BG Protection \n" ));
                        }
                    }
                    
                    DBGPRINT(RT_DEBUG_TRACE, ("SYNC - AP changed B/G protection to %d\n", bUseBGProtection));
                }
                
                if (OPSTATUS_TEST_FLAG(pAd, fOP_STATUS_BG_PROTECTION_INUSED) 
                    && ERP_IS_NON_ERP_PRESENT(Erp))
                    OPSTATUS_SET_FLAG(pAd, fOP_STATUS_USECCK_PROTECTION_ENABLE);
                
                // check Ht protection mode.
                if ((AddHtInfoLen != 0) && 
                    ((AddHtInfo.AddHtInfo2.OperaionMode != pAd->MlmeAux.AddHtInfo.AddHtInfo2.OperaionMode) ||
                    (AddHtInfo.AddHtInfo2.NonGfPresent != pAd->MlmeAux.AddHtInfo.AddHtInfo2.NonGfPresent)))
                {
                    pAd->MlmeAux.AddHtInfo.AddHtInfo2.OperaionMode = AddHtInfo.AddHtInfo2.OperaionMode;
                    pAd->MlmeAux.AddHtInfo.AddHtInfo2.NonGfPresent = AddHtInfo.AddHtInfo2.NonGfPresent;
                    if (AddHtInfo.AddHtInfo2.NonGfPresent == 1)
                    {
                        MtAsicUpdateProtect(pAd, pAd->MlmeAux.AddHtInfo.AddHtInfo2.OperaionMode,  ALLN_SETPROTECT, FALSE, TRUE);
                    }
                    else
                        MtAsicUpdateProtect(pAd, pAd->MlmeAux.AddHtInfo.AddHtInfo2.OperaionMode,  ALLN_SETPROTECT, FALSE, FALSE);
                    DBGPRINT(RT_DEBUG_TRACE, ("SYNC - AP change to OperaionMode = %d,  NonGfPresent = %d\n", pAd->MlmeAux.AddHtInfo.AddHtInfo2.OperaionMode, pAd->MlmeAux.AddHtInfo.AddHtInfo2.NonGfPresent));
                }
                if (OPSTATUS_TEST_FLAG(pAd, fOP_STATUS_SHORT_PREAMBLE_INUSED) && 
                    ERP_IS_USE_BARKER_PREAMBLE(Erp))
                {
                    MlmeSetTxPreamble(pAd, Rt802_11PreambleLong);
                    DBGPRINT(RT_DEBUG_TRACE, ("SYNC - AP forced to use LONG preamble\n"));
                }

                if (OPSTATUS_TEST_FLAG(pPort, fOP_STATUS_WMM_INUSED)    &&
                    (EdcaParm.bValid == TRUE)                          &&
                    (EdcaParm.EdcaUpdateCount != pPort->CommonCfg.APEdcaParm.EdcaUpdateCount))
                {
                    DBGPRINT(RT_DEBUG_TRACE, ("SYNC - AP change EDCA parameters(from %d to %d)\n", 
                        pPort->CommonCfg.APEdcaParm.EdcaUpdateCount,
                        EdcaParm.EdcaUpdateCount));
                    MtAsicSetEdcaParm(pAd, pPort, &EdcaParm, TRUE);
                }

                // copy QOS related information
                PlatformMoveMemory(&pPort->CommonCfg.APQbssLoad, &QbssLoad, sizeof(QBSS_LOAD_PARM));
                PlatformMoveMemory(&pPort->CommonCfg.APQosCapability, &QosCapability, sizeof(QOS_CAPABILITY_PARM));

                // if Supported MCS change, should re-create LimitTxRateTable.
                if ((pPort->CommonCfg.LimitTxRateTableSize != 0) && (HtCapabilityLen>0) && ((pAd->StaActive.SupportedHtPhy.MCSSet[0]!=HtCapability.MCSSet[0])||(pAd->StaActive.SupportedHtPhy.MCSSet[1]!=HtCapability.MCSSet[1])))
                {
                    pPort->CommonCfg.LimitTxRateTableSize = 0;
                    DBGPRINT(RT_DEBUG_TRACE, ("%s: if Supported MCS change, should re-create LimitTxRateTable\n",__FUNCTION__));
                }

                // 2009: PF#1: 20/40 Coexistence in 2.4 GHz Band
                // When AP changes "STA Channel Width" and "Secondary Channel Offset" fields of HT Operation Element in the Beacon to 0
#ifdef MULTI_CHANNEL_SUPPORT
                if ((AddHtInfoLen != 0) && INFRA_ON(pPort) && (pPort->CommonCfg.PhyMode >= PHY_11ABGN_MIXED) && (pPort->CommonCfg.TgnControl.field.DisableChangeChBW == FALSE) && (pAd->MccCfg.MultiChannelEnable == FALSE))
#else
                if ((AddHtInfoLen != 0) && INFRA_ON(pPort) && (pPort->CommonCfg.PhyMode >= PHY_11ABGN_MIXED) && (pPort->CommonCfg.TgnControl.field.DisableChangeChBW == FALSE))
#endif
                {
                    BOOLEAN bChangeBW = FALSE;

                    //
                    // 6x9x ToDo: Add BW_80 logic at here
                    //
                    if (pPort->BBPCurrentBW == BW_80)
                    {
                        // Do nothing in IC bring stage
                                }
                    //
                    // 40 -> 20 case                    
                    // 1) HT Information
                    // 2) Secondary Channel Offset Element
                    //              
                    else if (pPort->BBPCurrentBW == BW_40)
                    {
                        if (((AddHtInfo.AddHtInfo.ExtChanOffset == EXTCHA_NONE) && (AddHtInfo.AddHtInfo.RecomWidth == 0)) ||((AddHtInfoLen == 0)&&(NewExtChannelOffset==0x0)))
                        {
                            PMAC_TABLE_ENTRY pBssidMacTabEntry = MlmeSyncMacTabMatchedRoleType(pPort, ROLE_WLANIDX_BSSID);
                            if(pBssidMacTabEntry == NULL)
                            {
                                DBGPRINT(RT_DEBUG_TRACE,("%s(), Line:%d, Mac Entry is NULL\n", __FUNCTION__, __LINE__));
                                return;
                            }
                            
                            DBGPRINT(RT_DEBUG_TRACE, ("SYNC -%d concha = %d. Bssid Last 3 bytes= %x %x %x\n", AddHtInfoLen, AddHtInfo.ControlChan, pPort->PortCfg.Bssid[3], pPort->PortCfg.Bssid[4], pPort->PortCfg.Bssid[5]));
                            DBGPRINT(RT_DEBUG_TRACE, ("SYNC - case 1 bChangeBW 40->20.  NewExtChannelOffset= %x\n", NewExtChannelOffset));
                            bChangeBW = TRUE;
                            pPort->CentralChannel = pPort->Channel;                         
                            
                            WRITE_PHY_CFG_BW(pAd, &pBssidMacTabEntry->TxPhyCfg, BW_20);
                            
                            pAd->StaActive.SupportedHtPhy.HtChannelWidth = BW_20;
                            pAd->StaActive.SupportedHtPhy.ExtChanOffset = EXTCHA_NONE ;
                            MlmeCntlChannelWidth(pAd, pPort->Channel, pPort->CentralChannel, BW_20, 0);                     
                        }
                    }
                    //
                    // 20 -> 40 case
                    // 1.) Supported Channel Width Set Field of the HT Capabilities element of both STAs is set to a non-zero
                    // 2.) Secondary Channel Offset field is SCA or SCB
                    // 3.) 40MHzRegulatoryClass is TRUE (not implement it)
                    //
                    else if (((pPort->BBPCurrentBW == BW_20)||(NewExtChannelOffset!=0x0))&&(pPort->CommonCfg.DesiredHtPhy.HtChannelWidth != BW_20))
                    {
                        if ((AddHtInfo.AddHtInfo.ExtChanOffset != EXTCHA_NONE) && (AddHtInfo.AddHtInfo.RecomWidth == 1) && (HtCapabilityLen>0) && (HtCapability.HtCapInfo.ChannelWidth == 1))
                        {
                            PMAC_TABLE_ENTRY pBssidMacTabEntry = MlmeSyncMacTabMatchedRoleType(pPort, ROLE_WLANIDX_BSSID);
                            
                            if ((AddHtInfo.ControlChan > 2)&& (AddHtInfo.AddHtInfo.ExtChanOffset == EXTCHA_BELOW))
                            {
                                pPort->CentralChannel = AddHtInfo.ControlChan - 2;
                                pAd->StaActive.SupportedHtPhy.ExtChanOffset = EXTCHA_BELOW ;
                                DBGPRINT(RT_DEBUG_TRACE, ("SYNC - case 2  bChangeBW. 20->40. Secondary below \n"));
                                bChangeBW = TRUE;                               
                            }
                            else if ((AddHtInfo.AddHtInfo.ExtChanOffset == EXTCHA_ABOVE))
                            {
                                pPort->CentralChannel = AddHtInfo.ControlChan + 2;
                                pAd->StaActive.SupportedHtPhy.ExtChanOffset = EXTCHA_ABOVE;
                                DBGPRINT(RT_DEBUG_TRACE, ("SYNC - case 3  bChangeBW. 20->40. Secondary above\n"));
                                bChangeBW = TRUE;                               
                            }
                            
                            if (bChangeBW)
                            {
                                pPort->Channel = AddHtInfo.ControlChan;
                                MlmeCntlChannelWidth(pAd, pPort->Channel, pPort->CentralChannel, BW_40, AddHtInfo.AddHtInfo.ExtChanOffset);                         

                                if(pBssidMacTabEntry)
                                {
                                    WRITE_PHY_CFG_BW(pAd, &pBssidMacTabEntry->TxPhyCfg, BW_40);
                                }
                                
                                pAd->StaActive.SupportedHtPhy.HtChannelWidth = BW_40;
                            }
                        }
                    }

                    if (bChangeBW)
                    {
                        pPort->CommonCfg.BSSCoexist2040.word = 0;
                        TriEventInit(pAd, pPort);
                        MlmeSyncBuildEffectedChannelList(pAd);                  

                        //if AP started rebuild beacon
                        if (pPort->SoftAP.bAPStart && pAd->NumberOfPorts == 2 && P2P_OFF(pPort))
                        {
                            DBGPRINT(RT_DEBUG_TRACE, ("SYNC - -bChangeBW. Prepare to restart AP\n"));
                            pApPort = MlmeSyncGetApPort(pAd);
                            if (pApPort != NULL)
                            {
                                DBGPRINT(RT_DEBUG_TRACE,("[%s][LINE_%d] Parameters are changed, restart SoftAP @ port %d...\n",__FUNCTION__,__LINE__,pPort->PortNumber));
                                APStop( pAd, pApPort);
                                APStartUp( pAd, pApPort);
                            }
                            else
                            {
                                DBGPRINT(RT_DEBUG_WARN, ("AP port is NULL\n"));
                            }
                        }                           
                    }
                }

            }

            DBGPRINT(RT_DEBUG_TRACE, ("SYNC - run here ...3, INFRA_ON(pPort) = %d, (pAd->StaCfg.Psm == PWR_SAVE) = %d, MessageToMe  =%d, BcastFlag = %d, DtimCount = %d, OPSTATUS_TEST_FLAG(pAd, fOP_STATUS_RECEIVE_DTIM) = %d\n", INFRA_ON(pPort), (pAd->StaCfg.Psm == PWR_SAVE), MessageToMe, BcastFlag, DtimCount, OPSTATUS_TEST_FLAG(pAd, fOP_STATUS_RECEIVE_DTIM)));

            // only INFRASTRUCTURE mode support power-saving feature
            if (INFRA_ON(pPort) && (pAd->StaCfg.Psm == PWR_SAVE)) 
            {
                //  1. AP has backlogged unicast-to-me frame, stay AWAKE, send PSPOLL
                //  2. AP has backlogged broadcast/multicast frame and we want those frames, stay AWAKE
                //  3. we have outgoing frames in TxRing or MgmtRing, better stay AWAKE
                //  4. Psm change to PWR_SAVE, but AP not been informed yet, we better stay AWAKE
                //  5. otherwise, put PHY back to sleep to save battery.
                if (MessageToMe)
                {
                    //if (pAd->pHifCfg->bForcePrintRX)
                    {
                        DBGPRINT(RT_DEBUG_TRACE, ("SYNC - AP backlog unicast-to-me, stay AWAKE, send PSPOLL\n"));
                    }
                    
                    if (pPort->CommonCfg.bAPSDCapable && pPort->CommonCfg.APEdcaParm.bAPSDCapable &&
                        pPort->CommonCfg.bAPSDAC_BE && pPort->CommonCfg.bAPSDAC_BK && pPort->CommonCfg.bAPSDAC_VI && pPort->CommonCfg.bAPSDAC_VO)
                    {
                        // in APSD we should not Send PS Poll to AP.
                    }
                    else
                    {
                        pAd->TrackInfo.CountDowntoPsm = 12;
                        
                        if (OPSTATUS_TEST_FLAG(pAd, fOP_STATUS_DOZE))
                        {
                            DBGPRINT(RT_DEBUG_TRACE, ("SYNC - 1 fOP_STATUS_DOZE, RETURN!\n"));  
                            AsicForceWakeup(pAd);
                            MlmeSetPsm (pAd, PWR_ACTIVE);
                        }
                        
                        XmitSendPsPollFrame(pAd, pPort);
                        DBGPRINT(RT_DEBUG_TRACE, ("SYNC - need send ps-poll. later check\n"));
                    }
                }
                else if (BcastFlag && (DtimCount == 0) && OPSTATUS_TEST_FLAG(pAd, fOP_STATUS_RECEIVE_DTIM))
                {
                    //if (pAd->pHifCfg->bForcePrintRX)
                        DBGPRINT(RT_DEBUG_TRACE, ("SYNC - AP backlog broadcast/multicast, stay AWAKE\n"));
                } 
#if 1   // PM4
                else
                {
                    //pAd->ConsecutivelyBeaconLostCount = pAd->SleepTBTT - 1;
                    DBGPRINT(RT_DEBUG_TRACE, ("Enter AsicSleepThenAutoWakeup.....1 DtimCount = %d\n", DtimCount));
                    if (!OPSTATUS_TEST_FLAG(pAd, fOP_STATUS_DOZE))
                    {
                        DBGPRINT(RT_DEBUG_TRACE, ("Enter AsicSleepThenAutoWakeup.....2\n"));
                        AsicSleepThenAutoWakeup(pAd, DtimCount);
                    }
                    /*
                    else
                    {
                        pAd->TrackInfo.CountDowntoPsm = 12;
                        
                        DBGPRINT(RT_DEBUG_TRACE, ("SYNC - 2 fOP_STATUS_DOZE, RETURN!\n"));  
                        AsicForceWakeup(pAd);
                        MlmeSetPsm (pAd, PWR_ACTIVE);
                    }
                    */
                }
#else
                else 
                {
                    USHORT NextDtim = DtimCount;

                    if (NextDtim == 0) 
                        NextDtim = DtimPeriod;

                    TbttNumToNextWakeUp = pAd->StaCfg.DefaultListenCount;
                    if (OPSTATUS_TEST_FLAG(pAd, fOP_STATUS_RECEIVE_DTIM) && (TbttNumToNextWakeUp > NextDtim))
                        TbttNumToNextWakeUp = NextDtim;

                    if (pAd->pHifCfg->bForcePrintRX)
                    DBGPRINT(RT_DEBUG_TRACE, ("SYNC - PHY sleeps for %d TBTT, Dtim=%d/%d\n", TbttNumToNextWakeUp, DtimCount, DtimPeriod));
                    
                    pAd->ConsecutivelyBeaconLostCount = pAd->SleepTBTT - 1;

                    if(!MFP_ACT(pAd, pPort) && //protect SleepTBTT value large then DTIM, device will not wake up on time.
                        TbttNumToNextWakeUp < pAd->SleepTBTT)
                        AsicSleepThenAutoWakeup(pAd, pAd->SleepTBTT);
                    else
                        AsicSleepThenAutoWakeup(pAd, TbttNumToNextWakeUp);
                }
#endif
            }

#ifndef SINGLE_ADHOC_LINKUP
            // At least another peer in this IBSS, declare MediaState as CONNECTED
            if (ADHOC_ON(pPort) && 
                !OPSTATUS_TEST_FLAG(pPort, fOP_STATUS_MEDIA_STATE_CONNECTED))
            {
                OPSTATUS_SET_FLAG(pPort, fOP_STATUS_MEDIA_STATE_CONNECTED); 
                DBGPRINT(RT_DEBUG_TRACE, ("adhoc connect now\n"));
                MtAsicSetBssid(pAd, pPort->PortCfg.Bssid, (UINT8)pPort->PortNumber);

                //
                // Update/Initial Last RxRate for displaying on UI
                //
                WRITE_PHY_CFG_DOUBLE_WORD(pAd, &pPort->LastRxRatePhyCfg, READ_PHY_CFG_DOUBLE_WORD(pAd, &pAd->UiCfg.LastMgmtRxPhyCfg));
                

                if (bIsMerge)
                {
                    bRoamingStart = TRUE;
                    PlatformIndicateAdhocRoamingStart(pAd, pPort,pPort->PortCfg.Bssid, pPort->PortCfg.Ssid, pPort->PortCfg.SsidLen);

                    //
                    // To Patch WLK issue, before PlatformIndicateAdhocAssociation we can't indicate NDIS_STATUS_DOT11_ROAMING_START
                    // So we indicate NDIS_STATUS_DOT11_LINK_QUALITY to fake WLK.
                    //
                    PlatformIndicateLinkQuality(pAd,pPort->PortNumber);
                }

                // 2003/03/12 - john
                // Make sure this entry in "ScanTab" table, thus complies to Microsoft's policy that
                // "site survey" result should always include the current connected network. 
                //
                Bssidx = BssTableSearch(pAd, pPort, &pAd->ScanTab, Bssid, Channel);
                if (Bssidx == BSS_NOT_FOUND)
                {
                    Bssidx = BssTableSetEntry(pAd, pPort, &pAd->ScanTab, Bssid, Ssid, SsidLen, BssType, BeaconPeriod, 
                                &CfParm, AtimWin, CapabilityInfo, SupRate, SupRateLen, ExtRate, ExtRateLen, &HtCapability, &VhtCapability, &VhtOperation,
                                &AddHtInfo, HtCapabilityLen, AddHtInfoLen, NewExtChannelOffset, Channel, RealRssi + pAd->HwCfg.BbpRssiToDbmDelta, TimeStamp,  
                                &EdcaParm, &QosCapability, &QbssLoad, WSCInfoAtBeaconsLen, WSCInfoAtBeacons, WSCInfoAtProbeRspLen, WSCInfoAtProbeRsp, LenVIE, pVIE, TRUE);
                }

                //
                // Save this beacon frame for NDIS_STATUS_DOT11_ASSOCIATION_COMPLETION
                //
                PlatformMoveMemory(pAd->StaCfg.BeaconFrame, Add2Ptr(Elem->Msg, LENGTH_802_11), Elem->MsgLen - LENGTH_802_11);
                pAd->StaCfg.BeaconFrameSize = Elem->MsgLen - LENGTH_802_11;         
                
                //
                // Update RTS
                //
                if (pPort->CommonCfg.RtsThreshold < 2347)
                {
                    TX_RTS_CFG_STRUC RtsCfg;
                    RTUSBReadMACRegister(pAd, TX_RTS_CFG, &RtsCfg.word);
                    RtsCfg.field.RtsThres = pPort->CommonCfg.RtsThreshold;
                    RTUSBWriteMACRegister(pAd, TX_RTS_CFG, RtsCfg.word);                
                    DBGPRINT(RT_DEBUG_TRACE, ("!!! Adhoc LINK UP and Update RTS!!\n"));
                }                   
                
            }

            if ( ADHOC_ON(pPort) && ( !CAP_IS_ESS_ON(CapabilityInfo) || CAP_IS_IBSS_ON(CapabilityInfo)))
            {

                UCHAR           MaxSupportedRateIn500Kbps = 0, MinSupportedRateIn500Kbps = 108; //Rate 54*2
                UCHAR           idx;
                MAC_TABLE_ENTRY *pEntry = NULL;
                BOOLEAN             bfound = FALSE;


                // supported rates array may not be sorted. sort it and find the maximum rate
                for (idx=0; idx<SupRateLen; idx++)
                {
                    if (MaxSupportedRateIn500Kbps < (SupRate[idx] & 0x7f)) 
                        MaxSupportedRateIn500Kbps = SupRate[idx] & 0x7f;

                    if (MinSupportedRateIn500Kbps > (SupRate[idx] & 0x7f)) 
                        MinSupportedRateIn500Kbps = SupRate[idx] & 0x7f;
                }

                for (idx=0; idx<ExtRateLen; idx++)
                {
                    if (MaxSupportedRateIn500Kbps < (ExtRate[idx] & 0x7f)) 
                        MaxSupportedRateIn500Kbps = ExtRate[idx] & 0x7f;

                    if (MinSupportedRateIn500Kbps > (ExtRate[idx] & 0x7f))
                        MinSupportedRateIn500Kbps = ExtRate[idx] & 0x7f; 
                }
                        
                // look up the existing table
                pEntry = MacTableLookup(pAd,  pPort, Addr2);

                // update entry rx beacon time 
                if (pEntry && pEntry->ValidAsCLI)
                {
                    // Patch WiFi 11n: 3 STAs IBSS network. Use AP sync mode instead of IBSS sync mode.
                    if ((pPort->MacTab.Size>=2))
                    {
                        if ((pEntry->LastBeaconRxTime+(pPort->CommonCfg.BeaconPeriod+10)*ONE_MSECOND_TIME) > Now64)
                        {
                            if ( pAd->StaCfg.IBSSync.csr9Count >= 10 )
                                pAd->StaCfg.IBSSync.csr9Enable = TRUE; //Detect one STA of IBSS network may use AP sync mode 
                            else
                                pAd->StaCfg.IBSSync.csr9Count ++;
                        }
                        else
                        {
                            if ( pAd->StaCfg.IBSSync.csr9Count != 0 )
                                pAd->StaCfg.IBSSync.csr9Count --;
                        }
                    }
                    else if((pAd->StaCfg.IBSSync.csr9Enable == TRUE) && (pPort->MacTab.Size==1))
                    {
                        pAd->StaCfg.IBSSync.csr9Enable = FALSE;
                        pAd->StaCfg.IBSSync.csr9Count = 0;                      
                    }
                    
                    pEntry->LastBeaconRxTime = Now64;
                    
                    // vista: update time stamp to AdhocList
                    NdisAcquireSpinLock(&pAd->StaCfg.AdhocListLock);
                    pAdhocList = (PRTMP_ADHOC_LIST)pAd->StaCfg.ActiveAdhocListQueue.Head;
                    while (pAdhocList)
                    {
                        if (MAC_ADDR_EQUAL(&pAdhocList->PeerMacAddress, Addr2))
                        {
                            //NdisGetCurrentSystemTime((PLARGE_INTEGER)&pAdhocList->ullHostTimeStamp);
                            pAdhocList->ullHostTimeStamp = Now64;
                            break;
                        }
                        pAdhocList = (PRTMP_ADHOC_LIST)pAdhocList->Next;
                    }
                    NdisReleaseSpinLock(&pAd->StaCfg.AdhocListLock);
                }
                
                // Ad-hoc mode is using MAC address as BA session. So we need to continuously find newly joined adhoc station by receiving beacon.
                // To prevent always check this, we use wcid == RESERVED_WCID to recognize it as newly joined adhoc station.
                if ((ADHOC_ON(pPort) && (Elem->Wcid == WLANINDEX_MAX)) ||
                    (pEntry && ((pEntry->LastBeaconRefTime + ADHOC_ENTRY_BEACON_UPDATE_TIME * ONE_SECOND_TIME) < Now64)))
                {
                    // Another adhoc joining, add to our MAC table. 
                    if (pEntry == NULL) 
                    {
                        pEntry = MacTableInsertEntry(pAd, pPort, Addr2, FALSE);

                        if (!pEntry)
                        {
                            if (VarIE)
                                PlatformFreeMemory(VarIE, MAX_VIE_LEN);

                            if (WSCInfoAtBeacons)
                                PlatformFreeMemory(WSCInfoAtBeacons, MAX_VIE_LEN);

                            if (WSCInfoAtProbeRsp)
                                PlatformFreeMemory(WSCInfoAtProbeRsp, MAX_VIE_LEN);    
                        
                            return;
                        }
                        
                        //
                        // Add this Adhoc to AdhocList
                        //
                        NdisAcquireSpinLock(&pAd->StaCfg.AdhocListLock);
                        pAdhocList = (PRTMP_ADHOC_LIST)pAd->StaCfg.ActiveAdhocListQueue.Head;
                        while (pAdhocList)
                        {
                            if ( MAC_ADDR_EQUAL(pAdhocList->PeerMacAddress, Addr2))
                            {
                                bfound = TRUE;                          
                                break;
                            }
                            pAdhocList = (PRTMP_ADHOC_LIST)pAdhocList->Next;
                        }
                        if (!bfound)
                        {                   
                            pAdhocList = (PRTMP_ADHOC_LIST)RemoveHeadQueue(&pAd->StaCfg.FreeAdhocListQueue);
                            if (pAdhocList)
                            {
                                pAdhocList->AssocState = dot11_assoc_state_auth_assoc;
                                pAdhocList->BeaconInterval = BeaconPeriod;
                                pAdhocList->CapabilityInfo = CapabilityInfo;
                                COPY_MAC_ADDR(pAdhocList->Bssid, Bssid);
                                COPY_MAC_ADDR(pAdhocList->PeerMacAddress, Addr2);
                                //NdisGetCurrentSystemTime((PLARGE_INTEGER)&pAdhocList->ullHostTimeStamp);
                                pAdhocList->ullHostTimeStamp = Now64;
                                pAdhocList->bKeySet = FALSE;

                                InsertTailQueue(&pAd->StaCfg.ActiveAdhocListQueue, pAdhocList);
                                DBGPRINT(RT_DEBUG_TRACE, ("Increase adhoclist count =%d\n", pAd->StaCfg.ActiveAdhocListQueue.Number));
                            }
                        }
                        NdisReleaseSpinLock(&pAd->StaCfg.AdhocListLock);                
                        
                        // update entry rx beacon time
                        if (pEntry && pEntry->ValidAsCLI)
                            pEntry->LastBeaconRxTime = Now64;

                        // Select DAC according to HT or Legacy
                        if (((HtCapabilityLen > 0) || (PreNHtCapabilityLen > 0)) && (pPort->CommonCfg.PhyMode >= PHY_11ABGN_MIXED) && ((BssType==BSS_ADHOC)?(pAd->StaCfg.bAdhocNMode==TRUE):(TRUE)))
                        {
                            UCHAR WlanIdx = (UCHAR)MlmeSyncGetWlanIdxByPort(pPort, ROLE_WLANIDX_BSSID);
                            //RTMPCheckHt after update central channel to avoid RTMPCheckChannel return false when connecting OPEN-NONE after WEP/TKIP.
                            RTMPCheckHt(pAd, pPort, (UCHAR)WlanIdx, &HtCapability, &AddHtInfo, bVhtCapable, &VhtCapability);

                            if ((pPort->CommonCfg.PhyMode==PHY_11VHT) // 1. we do support VHT
                                && (bVhtCapable==TRUE))             // 2. the other side support VHT
                            {
                                RTMPCheckVht(pAd, pPort, WlanIdx, &VhtCapability, &VhtOperation);
                            }

                            // Copy AP Parameter to StaActive.  This is also in MlmeCntLinkUp.
                            // StaActive.SupportedHtPhy.MCSSet stores Peer AP's 11n Rx capability
                            PlatformMoveMemory(pAd->StaActive.SupportedHtPhy.MCSSet, HtCapability.MCSSet, 16);                  
                            pAd->StaActive.SupportedHtPhy.bHtEnable = TRUE;
                        
                            BbSetTxDacCtrlByTxPath(pAd);                        }
                            else
                            {
                            BbSetTxDacCtrl(pAd, DAC_0);
                        }
                        pAd->StaCfg.ActivePhyId = MlmeInfoGetPhyIdByChannel(pAd, pPort->Channel);
                        
                        //
                        // Indicate this peer adhoc to OS.
                        //
                        //if we already  indicate in MlmeSyncMlmeSyncPeerBeaconAtJoinAction(),we don't indicate again
                        if ((!bfound) && (pAd->StaCfg.AdhocCreator == FALSE))
                        {
                            PlatformIndicateAdhocAssociation(pAd, pPort, Addr2);
                        }

                        if (bRoamingStart)
                        {
                            PlatformIndicateRoamingCompletion(pAd, pPort, DOT11_ASSOC_STATUS_SUCCESS);
                        }
                        
                        DBGPRINT(RT_DEBUG_TRACE, ("%s: pAd->StaActive.SupportedHtPhy.MCSSet[0] = %x,  HtCapabilityLen = %d, pPort->CommonCfg.PhyMode = %d, pAd->StaCfg.bAdhocNMode = %d\n",
                            __FUNCTION__, pAd->StaActive.SupportedHtPhy.MCSSet[0], HtCapabilityLen, pPort->CommonCfg.PhyMode, pAd->StaCfg.bAdhocNMode));
                        DBGPRINT(RT_DEBUG_TRACE, ("ADHOC %x:%x:%x:%x:%x:%x  join in Entry #%d\n", Addr2[0],Addr2[1],Addr2[2],Addr2[3],Addr2[4],Addr2[5], pEntry->Aid));
                    }
                    
                    // Update Reference Time for each ADHOC_ENTRY_BEACON_UPDATE_TIME.
                    if ((Elem->Wcid != WLANINDEX_MAX) && ((pEntry->LastBeaconRefTime + ADHOC_ENTRY_BEACON_UPDATE_TIME * ONE_SECOND_TIME) < Now64))
                    {
                        pEntry->LastBeaconRefTime = Now64;
                    }
        
                    // Setup/Update its capability within ADHOC_ENTRY_BEACON_UPDATE_TIME
                    if (MlmeSyncStaAdhocUpdateMacTableEntry(pAd, 
                                                    pPort,
                                                    pEntry, 
                                                    MaxSupportedRateIn500Kbps,
                                                    MinSupportedRateIn500Kbps,
                                                    &HtCapability, 
                                                    HtCapabilityLen, 
                                                    &AddHtInfo,
                                                    AddHtInfoLen,
                                                    CapabilityInfo) == FALSE)
                    {
                        DBGPRINT(RT_DEBUG_TRACE, ("ADHOC - Add Entry failed.\n"));

                        if (VarIE)
                            PlatformFreeMemory(VarIE, MAX_VIE_LEN);

                        if (WSCInfoAtBeacons)
                            PlatformFreeMemory(WSCInfoAtBeacons, MAX_VIE_LEN);

                        if (WSCInfoAtProbeRsp)
                            PlatformFreeMemory(WSCInfoAtProbeRsp, MAX_VIE_LEN);    
                        
                        return;
                    }

                    // A new entry joins at the first time!!!!!
                    if (Elem->Wcid == WLANINDEX_MAX)
                    {       
                        if(RalinkIe != 0x0)
                            CLIENT_STATUS_SET_FLAG(pEntry, fCLIENT_STATUS_RALINK_CHIPSET);
                        else
                            CLIENT_STATUS_CLEAR_FLAG(pEntry, fCLIENT_STATUS_RALINK_CHIPSET);
                        
                        // Start BA session
                        if ((pPort->CommonCfg.BACapability.field.AutoBA == TRUE) && 
                            (READ_PHY_CFG_MODE(pAd, &pEntry->TxPhyCfg) >= MODE_HTMIX) &&
                            (pEntry->HTCapability.MCSSet[0] != 0))
                        {
                            pEntry->BaSizeInUse = (UCHAR)pPort->CommonCfg.BACapability.field.TxBAWinLimit;

                            for (idx = 0; idx <1 ; idx++)
                            {
                                BATableInsertEntry(pAd, pPort, pEntry->Aid, 0, 0x1, idx, BaSizeArray[pAd->StaActive.SupportedHtPhy.MaxRAmpduFactor], Originator_SetAutoAdd, FALSE);
                                pEntry->TXAutoBAbitmap |= (1<< idx);
                            }
                            DBGPRINT(RT_DEBUG_TRACE, ("ADHOC - BA BaSizeInUse = %d, TXBAbitmap=%x, RXBAbitmap=%x \n", pEntry->BaSizeInUse, pEntry->TXBAbitmap, pEntry->RXBAbitmap));
                        }

                        if(VHT_NIC(pAd))
                            pPort->CommonCfg.BaLimit = 30;
                        else if ((pAd->HwCfg.MACVersion & 0xFFFF0000) != 0x28600000)
                        {
                            pPort->CommonCfg.BaLimit = 14;
                        }
                        else
                        {
                            pPort->CommonCfg.BaLimit = 7;
                        }
                            
                        RTUSBWriteMACRegister(pAd, MAX_LEN_CFG, MAC_VALUE_MAX_LEN_CFG_MINMPDU14B(pAd));
                        DBGPRINT(RT_DEBUG_TRACE, ("%s: A new entry joins BaLimit = %d\n", 
                            __FUNCTION__, 
                            pPort->CommonCfg.BaLimit));
                                                
                    }
                }   
            }
#endif
        }
        // not my BSSID, ignore it
        else
        {
            // --------------------------------------------------------
            // CCX5, insert the new entry, reduce (AST) scanning
            if (is_my_ssid) // this will be always true
            {
                // for CCKM, we assuume that the same SSID but not my_bssid is the AP we may roam to.
                // Keep TimeStamp for Re-Association used.
                // and then we don't need to be at JOIN_WAIT_BEACON for fast-roaming
                // just set to CNTL_WAIT_AUTH, then send AUTH_REQ immediately
                // Potential issue: this one may be not the one we are going to roam to

                if (Channel != Elem->Channel)
                {
                    bUpdateRssi = FALSE;
                }
                else
                    bUpdateRssi = TRUE;

//              Bssidx = BssTableSearch(pAd, &pAd->ScanTab, Bssid, Channel);
//              if (Bssidx == BSS_NOT_FOUND)
                {
                    // insert the beacon to ScanTab, or update Rssi
                    Bssidx = BssTableSetEntry(pAd, pPort,&pAd->ScanTab, Bssid, Ssid, SsidLen, BssType, BeaconPeriod,
                                  &CfParm, AtimWin, CapabilityInfo, SupRate, SupRateLen, ExtRate, ExtRateLen,  &HtCapability, &VhtCapability, &VhtOperation,
                                 &AddHtInfo, HtCapabilityLen, AddHtInfoLen, NewExtChannelOffset, Channel, RealRssi, TimeStamp, 
                                 &EdcaParm, &QosCapability, &QbssLoad,WSCInfoAtBeaconsLen, WSCInfoAtBeacons, WSCInfoAtProbeRspLen, WSCInfoAtProbeRsp, LenVIE, pVIE, bUpdateRssi);

                    // for CCX drastic roaming case, avoid scanning to diminish the roaming time
                    if (INFRA_ON(pPort) &&
                        (pAd->StaCfg.CCXControl.field.VoiceRate) &&
                        (pAd->pHifCfg->BulkLastOneSecCount >= 50) &&
                        (RealRssi > (-1)*(CHAR)pAd->StaCfg.CCXControl.field.dBmToRoam))
                    {
                        DBGPRINT(RT_DEBUG_TRACE, ("Drastic Roaming Case: Update the last scan time \n"));
                        NdisGetCurrentSystemTime((PLARGE_INTEGER)&pAd->StaCfg.LastScanTime); 
                    }

                    if (Bssidx != BSS_NOT_FOUND)
                    {
                        DBGPRINT(RT_DEBUG_TRACE, ("%s - AP added to ScanTab[%d], RSSI=%d, MAC=%02x:%02x:%02x:%02x:%02x:%02x\n", __FUNCTION__,
                            Bssidx, RealRssi, Bssid[0], Bssid[1], Bssid[2], Bssid[3], Bssid[4], Bssid[5]));
                    }
                }
            }
            //--------------------------------------------------------
        }
    }
    // sanity check fail, ignore this frame

            // for FPGA connection
            if (0)
            {

                ULONG       Bssidx; 

                CHAR        RealRssi = RTMPMaxRssi(pAd, MlmeSyncConvertToRssi(pAd, Elem->Rssi, RSSI_0, Elem->AntSel, BW_20), MlmeSyncConvertToRssi(pAd, Elem->Rssi2, RSSI_1, Elem->AntSel, BW_20), MlmeSyncConvertToRssi(pAd, Elem->Rssi3, RSSI_2, Elem->AntSel, BW_20));       

                //
                // Housekeeping "SsidBssTab" table for later-on ROAMing usage. 
                //
                Bssidx = BssTableSearch(pAd, pPort, &pAd->MlmeAux.SsidBssTab, Bssid, Channel);
                if (Bssidx == BSS_NOT_FOUND)
                {
                    // discover new AP of this network, create BSS entry
                    Bssidx = BssTableSetEntry(pAd, pPort, &pAd->ScanTab, Bssid, Ssid, SsidLen, BssType, BeaconPeriod,
                                 &CfParm, AtimWin, CapabilityInfo, SupRate, SupRateLen, ExtRate, ExtRateLen, 
                                &HtCapability, &VhtCapability, &VhtOperation, &AddHtInfo,HtCapabilityLen,AddHtInfoLen,NewExtChannelOffset, Channel, 
                                RealRssi + pAd->HwCfg.BbpRssiToDbmDelta, TimeStamp, &EdcaParm, &QosCapability, 
                                &QbssLoad, WSCInfoAtBeaconsLen, WSCInfoAtBeacons, WSCInfoAtProbeRspLen, WSCInfoAtProbeRsp, LenVIE, pVIE, TRUE);
                    if (Bssidx == BSS_NOT_FOUND) // return if BSS table full
                    {
                        if (VarIE)
                            PlatformFreeMemory(VarIE, MAX_VIE_LEN);

                        if (WSCInfoAtBeacons)
                            PlatformFreeMemory(WSCInfoAtBeacons, MAX_VIE_LEN);

                        if (WSCInfoAtProbeRsp)
                            PlatformFreeMemory(WSCInfoAtProbeRsp, MAX_VIE_LEN);    
                        return;  
                    }
                    
                    DBGPRINT(RT_DEBUG_INFO, ("SYNC - New AP added to SsidBssTab[%d], RSSI=%d, MAC=%02x:%02x:%02x:%02x:%02x:%02x\n", 
                        Bssidx, RealRssi, Bssid[0], Bssid[1], Bssid[2], Bssid[3], Bssid[4], Bssid[5]));
                }
            }
    
    if (VarIE)
        PlatformFreeMemory(VarIE, MAX_VIE_LEN);

    if (WSCInfoAtBeacons)
        PlatformFreeMemory(WSCInfoAtBeacons, MAX_VIE_LEN);

    if (WSCInfoAtProbeRsp)
        PlatformFreeMemory(WSCInfoAtProbeRsp, MAX_VIE_LEN);    
}

/* 
    ==========================================================================
    Description:
        Receive PROBE REQ from remote peer when operating in IBSS mode
    ==========================================================================
 */
VOID MlmeSyncPeerProbeReqAction(
    IN PMP_ADAPTER pAd, 
    IN MLME_QUEUE_ELEM *Elem) 
{
    UCHAR               Addr2[MAC_ADDR_LEN];
    CHAR                Ssid[MAX_LEN_OF_SSID];
    UCHAR               SsidLen;
    NDIS_STATUS     NStatus;
    PUCHAR              pOutBuffer = NULL;
    ULONG               FrameLen = 0;
    UCHAR               DsLen = 1,  IbssLen = 2;
    UCHAR               LocalErpIe[3] = {IE_ERP, 1, 0};
    PMP_PORT          pPort = pAd->PortList[Elem->PortNum];
    UCHAR               SupRate[MAX_LEN_OF_SUPPORTED_RATES];
    UCHAR               SupRateLen = 0;
    UCHAR               ExtRate[MAX_LEN_OF_SUPPORTED_RATES];
    UCHAR               ExtRateLen = 0;

    if (IS_P2P_FINDING(pPort))
    {
        // Send Probe Response only in listen state, 
#if DBG     
        if (IS_P2P_LISTENING(pPort))
            PeerP2pProbeReq(pAd, Elem, TRUE, __LINE__);
        else
            PeerP2pProbeReq(pAd, Elem, FALSE, __LINE__);
#else
        if (IS_P2P_LISTENING(pPort))
            PeerP2pProbeReq(pAd, Elem, TRUE);
        else
            PeerP2pProbeReq(pAd, Elem, FALSE);
#endif
    }
#ifdef MULTI_CHANNEL_SUPPORT    
    else if (((IS_P2P_LISTEN_IDLE(pPort) || (pPort->P2PCfg.P2PDiscoProvState == P2P_IDLE)) && 
        /*(pAd->HwCfg.LatchRfRegs.Channel == pPort->P2PCfg.ListenChannel) && */ // replace this by below scan status
        (MT_TEST_FLAG(pAd, fRTMP_ADAPTER_BSS_SCAN_IN_PROGRESS) == FALSE)) ||
        (MT_TEST_BIT(pAd->pP2pCtrll->MultiChannelStartOrEndProcess, MULTI_CHANNEL_IN_NEW_SESSION) == TRUE))
#else
    else if (((IS_P2P_LISTEN_IDLE(pPort) || (pPort->P2PCfg.P2PDiscoProvState == P2P_IDLE)) && 
        /*(pAd->HwCfg.LatchRfRegs.Channel == pPort->P2PCfg.ListenChannel) && */ // replace this by below scan status
        (MT_TEST_FLAG(pAd, fRTMP_ADAPTER_BSS_SCAN_IN_PROGRESS) == FALSE)))
#endif
    {
#if DBG     
        PeerP2pProbeReq(pAd, Elem, TRUE, __LINE__);
#else
        PeerP2pProbeReq(pAd, Elem, TRUE);
#endif
    }
    // Always check port 0's connectivity status.
    else if ((INFRA_ON(pAd->PortList[PORT_0])) && (IS_P2P_CONNECT_IDLE(pPort)))
    {
        // Make me discoverablt even I am conencted in a INFRA AP BSS.
        // this includes concurrent opmode case and station opmode case.
#if DBG     
        PeerP2pProbeReq(pAd, Elem, TRUE, __LINE__);
#else
        PeerP2pProbeReq(pAd, Elem, TRUE);
#endif
    }

    if (! ADHOC_ON(pPort))
        return;

    if (PeerProbeReqSanity(pAd, Elem->Msg, Elem->MsgLen, Addr2, Ssid, &SsidLen))
    {
        if ((SsidLen == 0) || SSID_EQUAL(Ssid, SsidLen, pPort->PortCfg.Ssid, pPort->PortCfg.SsidLen))
        {
#if 0
            CSR15_STRUC Csr15;

            // we should respond a ProbeRsp only when we're the last BEACON transmitter 
            // in this ADHOC network.
            RTUSBReadMACRegister(pAd, CSR15, &Csr15.word);
            if (Csr15.field.BeaconSent == 0)
            {
                DBGPRINT(RT_DEBUG_INFO, ("SYNC - NOT last BEACON sender, no PROBE_RSP to %02x:%02x:%02x:%02x:%02x:%02x...\n",
                    Addr2[0],Addr2[1],Addr2[2],Addr2[3],Addr2[4],Addr2[5] ));
                return;
            }
#endif
            

			if (((pAd->StaCfg.AdhocMode == ADHOC_11B) && (pPort->Channel <= 14)) ||
				(pPort->CommonCfg.Ch14BOnly && (pPort->Channel == CHANNEL_14)))
			{
				//
				//CH14 also supports 11B only
				//
				SupRate[0] = 0x82; // 1 mbps
				SupRate[1] = 0x84; // 2 mbps
				SupRate[2] = 0x8b; // 5.5 mbps
				SupRate[3] = 0x96; // 11 mbps
				SupRateLen = 4;
				ExtRateLen = 0;
			}
			else if (pPort->Channel > 14)
			{
				SupRate[0]  = 0x8C;    // 6 mbps, in units of 0.5 Mbps, basic rate
				SupRate[1]  = 0x12;    // 9 mbps, in units of 0.5 Mbps
				SupRate[2]  = 0x98;    // 12 mbps, in units of 0.5 Mbps, basic rate
				SupRate[3]  = 0x24;    // 18 mbps, in units of 0.5 Mbps
				SupRate[4]  = 0xb0;    // 24 mbps, in units of 0.5 Mbps, basic rate
				SupRate[5]  = 0x48;    // 36 mbps, in units of 0.5 Mbps
				SupRate[6]  = 0x60;    // 48 mbps, in units of 0.5 Mbps
				SupRate[7]  = 0x6c;    // 54 mbps, in units of 0.5 Mbps
				SupRateLen  = 8;
				ExtRateLen  = 0;
			}
			else
			{
				SupRate[0] = 0x82; // 1 mbps
				SupRate[1] = 0x84; // 2 mbps
				SupRate[2] = 0x8b; // 5.5 mbps
				SupRate[3] = 0x96; // 11 mbps
				SupRateLen = 4;

				ExtRate[0]  = 0x0C;    // 6 mbps, in units of 0.5 Mbps,
				ExtRate[1]  = 0x12;    // 9 mbps, in units of 0.5 Mbps
				ExtRate[2]  = 0x18;    // 12 mbps, in units of 0.5 Mbps,
				ExtRate[3]  = 0x24;    // 18 mbps, in units of 0.5 Mbps
				ExtRate[4]  = 0x30;    // 24 mbps, in units of 0.5 Mbps,
				ExtRate[5]  = 0x48;    // 36 mbps, in units of 0.5 Mbps
				ExtRate[6]  = 0x60;    // 48 mbps, in units of 0.5 Mbps
				ExtRate[7]  = 0x6c;    // 54 mbps, in units of 0.5 Mbps		
				ExtRateLen  = 8;
			}

            // allocate and send out ProbeRsp frame
            NStatus = MlmeAllocateMemory(pAd, &pOutBuffer);  //Get an unused nonpaged memory
            if (NStatus != NDIS_STATUS_SUCCESS)
                return;

            //pAd->StaCfg.AtimWin = 0;  // ??????
            DBGPRINT(RT_DEBUG_INFO, ("SYNC - Send PROBE_RSP to %02x:%02x:%02x:%02x:%02x:%02x...\n", 
                Addr2[0],Addr2[1],Addr2[2],Addr2[3],Addr2[4],Addr2[5] ));
                
            MgntPktConstructIBSSProbRspPktFrame(pPort, Addr2, pOutBuffer, &FrameLen);
                    
            NdisCommonMiniportMMRequest(pAd, pOutBuffer, FrameLen);
        }
    }
}


/* 
    ==========================================================================
    Description:
        peer sends probe request when doing STA scanning. Add for P2P Discovery
    ==========================================================================
 */
VOID MlmeSyncPeerReqAtScanAction(
    IN PMP_ADAPTER pAd, 
    IN MLME_QUEUE_ELEM *Elem) 
{
    PMP_PORT  pPort = pAd->PortList[Elem->PortNum];

    DBGPRINT(RT_DEBUG_TRACE, ("pAd->HwCfg.LatchRfRegs.Channel = %d pPort->P2PCfg.ListenChannel = %d", pAd->HwCfg.LatchRfRegs.Channel, pPort->P2PCfg.ListenChannel));
    if (IS_P2P_FINDING(pPort))
    {
        // Send Probe Response only in listen state, 
#if DBG     
        if (IS_P2P_LISTENING(pPort))
            PeerP2pProbeReq(pAd, Elem, TRUE, __LINE__);
        else
            PeerP2pProbeReq(pAd, Elem, FALSE, __LINE__);
#else
        if (IS_P2P_LISTENING(pPort))
            PeerP2pProbeReq(pAd, Elem, TRUE);
        else
            PeerP2pProbeReq(pAd, Elem, FALSE);
#endif
    }
    else if ((pPort->PortType < WFD_DEVICE_PORT) && IS_P2P_LISTEN_IDLE(pPort) && (pAd->HwCfg.LatchRfRegs.Channel == pPort->P2PCfg.ListenChannel))
    {
        if ((IS_P2P_SIGMA_ON(pPort)) ||
            ((pAd->HwCfg.LatchRfRegs.Channel == pPort->P2PCfg.ListenChannel) && (IS_P2P_SIGMA_OFF(pPort))))
#if DBG     
        PeerP2pProbeReq(pAd, Elem, TRUE, __LINE__);
#else
        PeerP2pProbeReq(pAd, Elem, TRUE);
#endif
    }
    // Always check port 0's connectivity status.
    else if ((INFRA_ON(pAd->PortList[PORT_0])) && (IS_P2P_CONNECT_IDLE(pPort)))
    {
        // Make me discoverablt even I am conencted in a INFRA AP BSS.
        // this includes concurrent opmode case and station opmode case.
#if DBG     
        PeerP2pProbeReq(pAd, Elem, TRUE, __LINE__);
#else
        PeerP2pProbeReq(pAd, Elem, TRUE);
#endif
    }
}


VOID MlmeSyncPeerProbeRspAction(
    IN PMP_ADAPTER pAd, 
    IN MLME_QUEUE_ELEM *Elem)
{
    PMP_PORT      pPort = pAd->PortList[Elem->PortNum];
    PFRAME_802_11   pFrame = (PFRAME_802_11)Elem->Msg;

    DBGPRINT(RT_DEBUG_INFO, ("SYNC - MlmeSyncPeerProbeRspAction\n"));
    if (ADHOC_ON(pPort) && MAC_ADDR_EQUAL(pFrame->Hdr.Addr3, pPort->PortCfg.Bssid))
    {
        //update time stamp
        MlmeInfoUpdateAdhocList(pAd, pPort, pFrame->Hdr.Addr2);
    }
}

VOID MlmeSyncBeaconTimeoutAtJoinAction(
    IN PMP_ADAPTER pAd, 
    IN MLME_QUEUE_ELEM *Elem) 
{
    USHORT Status;
    PMP_PORT  pPort = pAd->PortList[Elem->PortNum];
    
    DBGPRINT(RT_DEBUG_TRACE, ("SYNC - MlmeSyncBeaconTimeoutAtJoinAction\n"));   

    pPort->Mlme.SyncMachine.CurrState = SYNC_IDLE;
    Status = MLME_REJ_TIMEOUT;
    MlmeEnqueue(pAd,pPort, MLME_CNTL_STATE_MACHINE, MT2_JOIN_CONF, 2, &Status);
}

VOID MlmeSyncRestoreChannelTimeoutAction(
    IN PMP_ADAPTER pAd, 
    IN MLME_QUEUE_ELEM *Elem) 
{
    //UCHAR BBPValue;
    PMP_PORT  pPort = pAd->PortList[Elem->PortNum];
    
    //Have to abort scan action if current state is doing reset request operation
    if (MT_TEST_FLAG(pAd, fRTMP_ADAPTER_RESET_IN_PROGRESS) || 
        MT_TEST_FLAG(pAd, fRTMP_ADAPTER_NIC_NOT_EXIST) ||
        MT_TEST_FLAG(pAd, fRTMP_ADAPTER_SLEEP_IN_PROGRESS)||
        MT_TEST_FLAG(pAd, fRTMP_ADAPTER_MLME_RESET_IN_PROGRESS))  
    {
        pPort->Mlme.SyncMachine.CurrState  = SYNC_IDLE;     
        return;
    }   
            
    pPort->ScaningChannel = MlmeSyncNextChannel(pAd, pPort, pAd->MlmeAux.LastScanChannel);

    //disable Soft AP beacon before scan. Don't need to check autoGO case. If I am in operating state.. it means 
    // at least one clients are in my group.  Then I need to set NoA before SCAN. Sometimes, when I am GO, but
    // no onee connects with me... I don't need to announce NoA before SCAN .
    if (IS_P2P_GO_OP(pPort) && (pPort->P2PCfg.GONoASchedule.Duration > 0))
    {
        if (TRUE == P2pActionMlmeCntlOidScanProc(pAd, pPort, NULL))
            return;

        AsicPauseBssSync(pAd);
    }
    else if ((pPort->SoftAP.bAPStart) || (pPort->P2PCfg.bGOStart))
    {
        AsicPauseBssSync(pAd);
    }
    
    // Let BBP register at 20MHz to do scan     
    BbSetTxRxBwCtrl(pAd, BW_20);
    MlmeSyncScanMlmeSyncNextChannel(pAd,pPort); // this routine will stop if pPort->ScaningChannel == 0
}

VOID
MlmeSyncMlmeSyncScanTimeoutActionWorkitemCallBack(
    PVOID   Context
    )
{
    PMP_PORT  pPort = (PMP_PORT)Context;
    PMP_ADAPTER pAd = pPort->pAd;
    
    FUNC_ENTER; 
    //Have to abort scan action if current state is doing reset request operation
    if (MT_TEST_FLAG(pAd, fRTMP_ADAPTER_RESET_IN_PROGRESS) || 
        MT_TEST_FLAG(pAd, fRTMP_ADAPTER_NIC_NOT_EXIST) ||
        MT_TEST_FLAG(pAd, fRTMP_ADAPTER_SLEEP_IN_PROGRESS)||
        MT_TEST_FLAG(pAd, fRTMP_ADAPTER_MLME_RESET_IN_PROGRESS))  
    {
        //if just bulk in reset keep on scan happend
        if(!MT_TEST_FLAG(pAd,fRTMP_ADAPTER_BULKIN_RESET))
        {
            pPort->Mlme.SyncMachine.CurrState  = SYNC_IDLE;     
            return;
        }
    }
    
    if ((pPort->CommonCfg.bCarrierDetect) && (pPort->CommonCfg.bCarrierAssert))
    {
        // carrier assertion, no full channel scanning, only scan current channel
        pPort->ScaningChannel = 0;
    }
    else
    {
        pPort->ScaningChannel = MlmeSyncNextChannel(pAd, pPort, pPort->ScaningChannel);
        DBGPRINT(RT_DEBUG_TRACE, ("SYNC - Assign next channel (pPort->ScaningChannel = %d)\n", pPort->ScaningChannel));
    }
    
    // The miniport driver always performs active scan for hidden AP after it scans all channel list.
    if ((pPort->ScaningChannel == SCAN_TERMINATION) && 
         (pAd->MlmeAux.bActiveScanForHiddenAP == FALSE) &&
         (pAd->MlmeAux.ScanType != SCAN_P2P) &&
         (!pPort->SoftAP.bAPStart))
    {
        if ((pAd->OpMode == OPMODE_STA) && (pPort->PortSubtype == PORTSUBTYPE_P2PGO))
        {
            DBGPRINT(RT_DEBUG_TRACE, ("SYNC - GO doesn't scan hidden \n"));
        }
        else
        {
            // Performing active scan on channel with at least one hidden AP.
            pAd->MlmeAux.bActiveScanForHiddenAP = TRUE;

            DBGPRINT(RT_DEBUG_TRACE, ("%s - Active scan for hidden APs only\n", __FUNCTION__));
            
            // Find the first channel
            pPort->ScaningChannel = MlmeSyncFirstChannel(pAd);
        }
    }
    
    MlmeSyncScanMlmeSyncNextChannel(pAd,pPort); // this routine will stop if pPort->ScaningChannel == 0

    FUNC_LEAVE;
    
}

/* 
    ==========================================================================
    Description:
        Scan timeout procedure. basically add channel index by 1 and rescan
    ==========================================================================
 */
VOID MlmeSyncScanTimeoutAction(
    IN PMP_ADAPTER pAd, 
    IN MLME_QUEUE_ELEM *Elem) 
{
    PMP_PORT  pPort = pAd->PortList[Elem->PortNum];
    
    //Have to abort scan action if current state is doing reset request operation
    if (MT_TEST_FLAG(pAd, fRTMP_ADAPTER_RESET_IN_PROGRESS) || 
        MT_TEST_FLAG(pAd, fRTMP_ADAPTER_NIC_NOT_EXIST) ||
        MT_TEST_FLAG(pAd, fRTMP_ADAPTER_SLEEP_IN_PROGRESS)||
        MT_TEST_FLAG(pAd, fRTMP_ADAPTER_MLME_RESET_IN_PROGRESS))  
    {
        //if just bulk in reset keep on scan happend
        if(!MT_TEST_FLAG(pAd,fRTMP_ADAPTER_BULKIN_RESET))
        {
            pPort->Mlme.SyncMachine.CurrState  = SYNC_IDLE;     
            return;
        }
    }
    
    if ((pPort->CommonCfg.bCarrierDetect) && (pPort->CommonCfg.bCarrierAssert))
    {
        // carrier assertion, no full channel scanning, only scan current channel
        pPort->ScaningChannel = 0;
    }
    else
    {
        pPort->ScaningChannel = MlmeSyncNextChannel(pAd, pPort, pPort->ScaningChannel);
        DBGPRINT(RT_DEBUG_TRACE, ("SYNC - Assign next channel (pPort->ScaningChannel = %d)\n", pPort->ScaningChannel));
    }
    
    // The miniport driver always performs active scan for hidden AP after it scans all channel list.
    if ((pPort->ScaningChannel == SCAN_TERMINATION) && 
         (pAd->MlmeAux.bActiveScanForHiddenAP == FALSE) &&
         (pAd->MlmeAux.ScanType != SCAN_P2P) &&
         (!pPort->SoftAP.bAPStart))
    {
        if ((pAd->OpMode == OPMODE_STA) && (pPort->PortSubtype == PORTSUBTYPE_P2PGO))
        {
            DBGPRINT(RT_DEBUG_TRACE, ("SYNC - GO doesn't scan hidden \n"));
        }
        else
        {
            // Performing active scan on channel with at least one hidden AP.
            pAd->MlmeAux.bActiveScanForHiddenAP = TRUE;

            DBGPRINT(RT_DEBUG_TRACE, ("%s - Active scan for hidden APs only\n", __FUNCTION__));
            
            // Find the first channel
            pPort->ScaningChannel = MlmeSyncFirstChannel(pAd);
        }
    }
    
    MlmeSyncScanMlmeSyncNextChannel(pAd,pPort); // this routine will stop if pPort->ScaningChannel == 0
}

BOOLEAN MlmeSyncHasHiddenAPAtThisChannel(
    IN PMP_ADAPTER pAd, 
    IN UCHAR channel,
    IN BOOLEAN bHasCiscoAP)
{
    UCHAR       i = 0;
    BOOLEAN     bHasHiddenAP = FALSE;
    PBSS_ENTRY  pBssEntry = NULL;

    for (i = 0; i < pAd->ScanTab.BssNr; i++)
    {
        pBssEntry = &(pAd->ScanTab.BssEntry[i]);

        if ((pBssEntry->Channel == channel) && (pBssEntry->Hidden == TRUE)) // This is a hidden AP..
        {
            bHasHiddenAP = TRUE;

            DBGPRINT_RAW(RT_DEBUG_INFO, ("%s: This is a hidden AP (Channel = %d; BSSID = %02X:%02X:%02X:%02X:%02X:%02X)", 
                __FUNCTION__, 
                pBssEntry->Channel, 
                pBssEntry->Bssid[0], pBssEntry->Bssid[1], pBssEntry->Bssid[2], pBssEntry->Bssid[3], pBssEntry->Bssid[4], pBssEntry->Bssid[5]));
            break;
        }
        else if((pBssEntry->Channel == channel) && bHasCiscoAP && (pBssEntry->bIsCiscoAP)) // Patch guest mode. This is a Cisco AP..
        {
            bHasHiddenAP = TRUE;

            DBGPRINT_RAW(RT_DEBUG_TRACE, ("%s: bIsCiscoAP %d:This is a hidden AP(%d) (Channel = %d/%d; BSSID = %02X:%02X:%02X:%02X:%02X:%02X)", 
                __FUNCTION__,
                pBssEntry->bIsCiscoAP,
                pBssEntry->Hidden,
                channel,
                pBssEntry->Channel, 
                pBssEntry->Bssid[0], pBssEntry->Bssid[1], pBssEntry->Bssid[2], pBssEntry->Bssid[3], pBssEntry->Bssid[4], pBssEntry->Bssid[5]));
            break;
        }       
    }

    return bHasHiddenAP;
}

BOOLEAN MlmeSyncHasWscAPAtThisChannel(
    IN PMP_ADAPTER pAd, 
    IN UCHAR channel)
{
    UCHAR       i = 0;
    BOOLEAN     bHasWscAP = FALSE;
    PBSS_ENTRY  pBssEntry = NULL;
    PMP_PORT    pPort = pAd->PortList[FXXK_PORT_0];

    if ((pPort->CommonCfg.bIEEE80211H == FALSE) && (pPort->CommonCfg.bCarrierDetect == FALSE))
        return FALSE; // needn't to check Wsc AP if  bIEEE80211H and bIEEE80211H are FALSE
        
    for (i = 0; i < pAd->ScanTab.BssNr; i++)
    {
        pBssEntry = &(pAd->ScanTab.BssEntry[i]);

        if ((pBssEntry->Channel == channel) && (pBssEntry->bWPSAP == TRUE)) // This is a hidden AP..
        {
            bHasWscAP = TRUE;

            DBGPRINT_RAW(RT_DEBUG_INFO, ("%s: This is a WSC AP (Channel = %d; BSSID = %02X:%02X:%02X:%02X:%02X:%02X)", 
                __FUNCTION__, 
                pBssEntry->Channel, 
                pBssEntry->Bssid[0], pBssEntry->Bssid[1], pBssEntry->Bssid[2], pBssEntry->Bssid[3], pBssEntry->Bssid[4], pBssEntry->Bssid[5]));
            break;
        }
    }

    return bHasWscAP;
}
PMP_PORT MlmeSyncGetApPort(IN PMP_ADAPTER pAd)
{
    UCHAR       PortNum;
    PMP_PORT  pPort =NULL;
    
    for (PortNum = 0; PortNum < RTMP_MAX_NUMBER_OF_PORT/*pAd->NumberOfPorts*/;PortNum++)
    {
        if (pAd->PortList[PortNum] == NULL)
        {
            continue;
        }

        if (pAd->PortList[PortNum]->bActive == FALSE)
        {
            continue;
        }

        if(pAd->PortList[PortNum]->PortType == EXTAP_PORT)
        {
            pPort = pAd->PortList[PortNum];
            break;
        }
    }
    return pPort;
}

PMP_PORT MlmeSyncGetActivePort(
    IN PMP_ADAPTER pAd)
{
    // Get Active Sta Port.
    // Port0 has high priority.
    // Don't use this function in assoc/connect/deauth timeout function.
    ULONG i;
    for (i = 0; i < RTMP_MAX_NUMBER_OF_PORT/*pAd->NumberOfPorts*/; i++)
    {
        if (pAd->PortList[i] == NULL)
        {
            continue;
        }

        if (pAd->PortList[i]->bActive == FALSE)
        {
            continue;
        }

        if (((pAd->PortList[i]->PortType == EXTSTA_PORT) || (pAd->PortList[i]->PortType == WFD_CLIENT_PORT))
            && (!IDLE_ON(pAd->PortList[i])))
            return  pAd->PortList[i];
    }
    return pAd->PortList[pAd->ucActivePortNum];
}

PMP_PORT
MlmeSyncGetP2pClientPort(
    IN PMP_ADAPTER pAd,
    IN UCHAR        CliIdx)
{
    UCHAR PortNum;
    for (PortNum = 0; PortNum < RTMP_MAX_NUMBER_OF_PORT/*pAd->NumberOfPorts*/; PortNum++)
    {
        if ((pAd->PortList[PortNum] == NULL) || (pAd->PortList[PortNum]->bActive == FALSE))
        {
            continue;
        }

        // Start to search the first or second client
        if (IS_P2P_MS_CLI(pAd, pAd->PortList[PortNum]))
        {
            if (CliIdx != 0)
                continue;
            
            return  pAd->PortList[PortNum];
        }
        if (IS_P2P_MS_CLI2(pAd, pAd->PortList[PortNum]))
        {
            if (CliIdx != 1)
                continue;

            return  pAd->PortList[PortNum];
        }
        
        if (IS_P2P_CON_CLI(pAd, pAd->PortList[PortNum]))
        {
            if (CliIdx != 0)
                continue;
            
            return  pAd->PortList[PortNum];
        }
    }
    
    return NULL;
}

VOID MlmeSyncScanDoneAction(
    IN PMP_ADAPTER pAd,
    IN PMP_PORT   pPort)
{
    USHORT      Status;
    ULONGLONG   Now64;
    UCHAR       ScanType;
#if(COMPILE_WIN8_ABOVE(CURRENT_OS_NDIS_VER))  
    ULONG       Bssidx;
#endif  


    ScanType = pAd->MlmeAux.ScanType;

#if(COMPILE_WIN8_ABOVE(CURRENT_OS_NDIS_VER))  
    if (NDIS_WIN8_ABOVE(pAd) && (pAd->StaCfg.NLOEntry.NLOEnable == TRUE) && (pAd->StaCfg.NLOEntry.bNLOAfterResume == TRUE))
    {
        pAd->StaCfg.NLOEntry.bNLOAfterResume = FALSE;
        DBGPRINT(RT_DEBUG_TRACE, ("SYNC - pAd->StaCfg.NLOEntry.uNumOfEntries = %d\n", pAd->StaCfg.NLOEntry.uNumOfEntries));
        if (pAd->LogoTestCfg.OnTestingWHQL)  // For InstantConnect Test
        {
            if (pAd->StaCfg.NLOEntry.IndicateNLO)
            {
                pAd->StaCfg.NLOEntry.IndicateNLO = FALSE;
                N6PlatformIndicateNLOStatus(pAd, pPort);
            }
                
            Bssidx = BssTableSearch(pAd, pPort, &pAd->ScanTab, pPort->CurrentBssEntry.Bssid, pPort->CurrentBssEntry.Channel);
            if(Bssidx == BSS_NOT_FOUND)
            {
                pAd->MlmeAux.bActiveScanForHiddenAP = FALSE;
                pPort->ScaningChannel = pAd->HwCfg.ChannelList[0].Channel; 
                AsicSwitchChannel(pPort, pPort->Channel, pPort->CentralChannel, pPort->BBPCurrentBW, FALSE);
                MgntPktSendProbeRequest(pPort);
                PlatformSetTimer(pPort, &pPort->Mlme.ScanTimer, pAd->LogoTestCfg.WHQLScanTime);
                return;
            }
        }
    }
#endif
    //change back to active port
    //pActivePort = MlmeSyncGetActivePort(pAd);

    if ((IS_P2P_GROUP_FORMING(pPort) || IS_P2P_INVITING(pPort)) && (pAd->PortList[pPort->P2PCfg.PortNumber]->PortSubtype != PORTSUBTYPE_P2PGO))
    {
        DBGPRINT(RT_DEBUG_TRACE, ("SYNC - End of SCAN, keep the same Channel. So don't need make a channel switch again. PortSubtype = %d\n", pPort->PortSubtype));
    }
    else if ((pPort->BBPCurrentBW == BW_80) && (INFRA_ON(pPort) || pPort->SoftAP.bAPStart || ADHOC_ON(pPort) ||(pAd->PortList[pPort->P2PCfg.PortNumber]->PortSubtype == PORTSUBTYPE_P2PGO)))
    {
        SwitchBandwidth(pAd, FALSE, pPort->CentralChannel, 
            pPort->CommonCfg.BwPri20Info.Bandwidth, 
            pPort->CommonCfg.BwPri20Info.Primary20Location);

        //AsicSwitchChannel(pAd, pPort->CentralChannel, FALSE);
       // AsicSwitchChannel(pPort, pPort->Channel, pPort->CentralChannel, pPort->BBPCurrentBW, FALSE);
        DBGPRINT(RT_DEBUG_TRACE, ("SYNC - End of SCAN, restore to 80MHz channel %d, Total BSS[%02d]\n",pPort->CentralChannel, pAd->ScanTab.BssNr));
    }
    else if (pPort->BBPCurrentBW == BW_40 && (INFRA_ON(pPort) || pPort->SoftAP.bAPStart || pPort->P2PCfg.bGOStart || ADHOC_ON(pPort)))
    {
        {
            //AsicSwitchChannel(pAd, pPort->CentralChannel, FALSE);
            //AsicSwitchChannel(pAd, pPort->Channel, FALSE); // MT7603

            //BbSetTxRxBwCtrl(pAd, BW_40);
        }

    }
    else if ((!OPSTATUS_TEST_FLAG(pPort, fOP_STATUS_MEDIA_STATE_CONNECTED))&&(pPort->BBPCurrentBW == BW_40))
    {
        // if not linkup; then fallback to BW20
        DBGPRINT(RT_DEBUG_TRACE, ("MlmeCntlChannelWidth: SYNC - End of SCAN: If not linkup, then fallback to BW20. restore to 20MHz channel %d\n",pPort->Channel));     
        //MlmeCntlChannelWidth(pAd, pPort->Channel, pPort->Channel, BW_20,EXTCHA_NONE);
    }
    else
    {   
        //AsicSwitchChannel(pAd, pPort->Channel, FALSE);

        DBGPRINT(RT_DEBUG_TRACE, ("SYNC - End of SCAN, restore to 20MHz channel %d, Total BSS[%02d]\n",pPort->Channel, pAd->ScanTab.BssNr));
    }

    AsicSwitchChannel(pPort, pPort->Channel, pPort->CentralChannel, pPort->BBPCurrentBW, FALSE);

    pAd->CountDowntoRadioOff = STAY_IN_AWAKE;
    
    // increase concurrent SoftAp beacon quantity.
    if ((pAd->BeaconPort.pApGoPort) && (pAd->BeaconPort.pApGoPort->SoftAP.bAPStart) && (pAd->BeaconPort.pApGoPort->PortSubtype == PORTSUBTYPE_VwifiAP))
    {
        MgntPktSendBeacon(pAd->BeaconPort.pApGoPort);
    }
            
    //
    // To prevent data lost.
    // Send an NULL data with turned PSM bit on to current associated AP before SCAN progress.
    // Now, we need to send an NULL data with turned PSM bit off to AP, when scan progress done 
    //
    if (OPSTATUS_TEST_FLAG(pPort, fOP_STATUS_MEDIA_STATE_CONNECTED) && (INFRA_ON(pPort)))
    {
        AsicSendNullFrame(pAd, pPort,pPort->Channel, (pAd->StaCfg.Psm == PWR_SAVE));
    }           

    //resume beacon after scan
    if ((pPort->SoftAP.bAPStart) || (pPort->P2PCfg.bGOStart))
    {
        AsicResumeBssSync(pAd);
    }

    if ((pPort->PortCfg.AdditionalRequestIEData != NULL) && (pPort->PortCfg.AdditionalRequestIESize > 0))
    {
        DBGPRINT(RT_DEBUG_TRACE,("Clear AdditionalIEData buffer!!\n"));
        PlatformFreeMemory(pPort->PortCfg.AdditionalRequestIEData, pPort->PortCfg.AdditionalRequestIESize);
        pPort->PortCfg.AdditionalRequestIEData = NULL;
        pPort->PortCfg.AdditionalRequestIESize = 0;
    }
    
    NdisGetCurrentSystemTime((PLARGE_INTEGER)&Now64);

        
    if (ScanType == SCAN_P2P)
    {
        if(INFRA_ON(pAd->PortList[PORT_0]))
        {
            pAd->StaCfg.LastBeaconRxTime = Now64;
            DBGPRINT(RT_DEBUG_TRACE, ("Scan Complete port 0 rece beacon"));
        }
        if (IS_P2P_CON_CLI(pAd, pAd->PortList[pPort->P2PCfg.PortNumber]) && INFRA_ON(pAd->PortList[pPort->P2PCfg.PortNumber]))
        {
            PMAC_TABLE_ENTRY pEntry = MlmeSyncMacTabMatchedRoleType(pAd->PortList[pPort->P2PCfg.PortNumber], ROLE_WLANIDX_P2P_CLIENT);
            if(pEntry == NULL)
            {
                DBGPRINT(RT_DEBUG_TRACE,("%s(), Line:%d, Mac Entry is NULL\n", __FUNCTION__, __LINE__));
                return;
            }            
            pEntry->LastBeaconRxTime = Now64;
            DBGPRINT(RT_DEBUG_TRACE, ("Scan Complete port %d rece beacon", pPort->P2PCfg.PortNumber));    
        }
        if (IS_P2P_MS_CLI(pAd, pAd->PortList[pPort->P2PCfg.ClientPortNumber]) && INFRA_ON(pAd->PortList[pPort->P2PCfg.ClientPortNumber]))
        {
            PMAC_TABLE_ENTRY pEntry = MlmeSyncMacTabMatchedRoleType(pAd->PortList[pPort->P2PCfg.PortNumber], ROLE_WLANIDX_P2P_CLIENT);
            if(pEntry == NULL)
            {
                DBGPRINT(RT_DEBUG_TRACE,("%s(), Line:%d, Mac Entry is NULL\n", __FUNCTION__, __LINE__));
                return;
            }   
            
            pEntry->LastBeaconRxTime = Now64;
            DBGPRINT(RT_DEBUG_TRACE, ("Scan Complete port %d rece beacon", pPort->P2PCfg.ClientPortNumber));  
        }
        if (IS_P2P_MS_CLI(pAd, pAd->PortList[pPort->P2PCfg.Client2PortNumber]) && INFRA_ON(pAd->PortList[pPort->P2PCfg.Client2PortNumber]))
        {
            PMAC_TABLE_ENTRY pEntry = MlmeSyncMacTabMatchedRoleType(pAd->PortList[pPort->P2PCfg.PortNumber], ROLE_WLANIDX_P2P_CLIENT);
            if(pEntry == NULL)
            {
                DBGPRINT(RT_DEBUG_TRACE,("%s(), Line:%d, Mac Entry is NULL\n", __FUNCTION__, __LINE__));
                return;
            }   
            
            pEntry->LastBeaconRxTime = Now64;
            DBGPRINT(RT_DEBUG_TRACE, ("Scan Complete port %d rece beacon", pPort->P2PCfg.Client2PortNumber)); 
        }
    }
    
    pAd->MlmeAux.ScanForConnectCnt++;
    pPort->Mlme.SyncMachine.CurrState = SYNC_IDLE;      
    Status = MLME_SUCCESS;

    PlatformIndicateScanStatus(pAd, pPort, NDIS_STATUS_SUCCESS, TRUE, FALSE);

    MlmeEnqueue(pAd,pPort, MLME_CNTL_STATE_MACHINE, MT2_SCAN_CONF, 2, &Status);
            
    if (pPort->SoftAP.ApCfg.StartSoftApAfterScan)
    {
        if (pPort->SoftAP.ApCfg.SoftApPort != NULL)
        {
            DBGPRINT(RT_DEBUG_TRACE, ("ApStartUp 10\n"));
            APStartUp(pAd, pPort->SoftAP.ApCfg.SoftApPort);
        }

        pPort->SoftAP.ApCfg.StartSoftApAfterScan = FALSE;
    }

    if (0)
    {
        ULONG   MACValue;
            // set rx filters
        //PlatformAccessCR(pAd, CR_READ, RMAC_RFCR, (PUCHAR)&MACValue, sizeof(ULONG));
        //HW_IO_READ32(pAd, RMAC_RFCR, &MACValue);
        MACValue = 0x0009e70b;
        HW_IO_WRITE32(pAd, RMAC_RFCR, MACValue);
    }           
}

/* 
    ==========================================================================
    Description:
        Scan next channel
    ==========================================================================
 */
VOID MlmeSyncScanMlmeSyncNextChannel(
    PMP_ADAPTER pAd,
    PMP_PORT pPort) 
{
    HEADER_802_11   Hdr80211;
    PUCHAR          pOutBuffer = NULL;
    PUCHAR          pOutBuffer2 = NULL, pOutBuffer3 = NULL;
    NDIS_STATUS     NStatus;
    NDIS_STATUS     NStatus2 = NDIS_STATUS_FAILURE, NStatus3 = NDIS_STATUS_FAILURE;
    ULONG           FrameLen = 0;
    UCHAR           SsidLen = 0;
    //UCHAR         BBPValue;
    UCHAR           HtLen;
    ULONG           Index;
    ULONG           Tmp;
    UCHAR           ScanType;
    UCHAR           NewScanType = SCAN_P2P;
    //ULONG         Value;
    UCHAR           SelMaxScanChannelCnt;
    
    if (MT_TEST_FLAG(pAd, fRTMP_ADAPTER_RADIO_OFF) ||
        MT_TEST_FLAG(pAd, fRTMP_ADAPTER_NIC_NOT_EXIST) ||
        MT_TEST_FLAG(pAd, fRTMP_ADAPTER_SLEEP_IN_PROGRESS))
    {
        // force to set end of scan
        pPort->ScaningChannel = 0;
    }
    
    ScanType = pAd->MlmeAux.ScanType;


#if(COMPILE_WIN8_ABOVE(CURRENT_OS_NDIS_VER))  
    // Stop Probe Request retry if driver prepare to switch to the next channel
    pPort->P2PCfg.SetPobeReqTimer = FALSE;
#endif

    if (pPort->ScaningChannel == 0) 
    {
        MlmeSyncScanDoneAction(pAd, pPort);
    } 
    else if (MT_TEST_FLAG(pAd, fRTMP_ADAPTER_NIC_NOT_EXIST))
    {
        pPort->Mlme.SyncMachine.CurrState = SYNC_IDLE;

        PlatformIndicateScanStatus(pAd, pAd->PortList[pAd->ucScanPortNum], NDIS_STATUS_SUCCESS, TRUE, FALSE);

        MlmeCntlConfirm(pAd, MT2_SCAN_CONF, MLME_FAIL_NO_RESOURCE);
    }   
    else 
    {

        // Performing active scan on channel with at least one hidden AP.
        if (((MlmeSyncHasWscAPAtThisChannel(pAd, pPort->ScaningChannel) == FALSE) &&
            (MlmeSyncHasHiddenAPAtThisChannel(pAd, pPort->ScaningChannel, TRUE) == FALSE)) && 
                     (pAd->MlmeAux.bActiveScanForHiddenAP == TRUE) &&
                     (pAd->MlmeAux.ScanType != SCAN_P2P))
        {
            DBGPRINT(RT_DEBUG_TRACE, ("SYNC - Skip the channel # %d\n", pPort->ScaningChannel));
            PlatformSetTimer(pPort, &pPort->Mlme.ScanTimer, 5);

            // save this for resumming from scan_pending
            pAd->StaCfg.LastScanChannel = pPort->ScaningChannel;

            pPort->Mlme.SyncMachine.CurrState = SCAN_LISTEN;

            return;
        }
        else
        {
            pAd->MlmeAux.MaxScanChannelCnt++;
            // BBP and RF are not accessible in PS mode, we has to wake them up first
            if (OPSTATUS_TEST_FLAG(pAd, fOP_STATUS_DOZE))
                AsicForceWakeup(pAd);
    
             // leave PSM during scanning. otherwise we may lost ProbeRsp & BEACON
            if (pAd->StaCfg.Psm == PWR_SAVE)
                MlmeSetPsm(pAd, PWR_ACTIVE);

            //if AP started , restore channel for about every 300 ms
            /*if ((pAd->OpMode == OPMODE_STA) && (pPort->PortSubtype == PORTSUBTYPE_P2PGO))
            {
                if (TRUE == P2pScanNextchannel(pAd))
                    return;
            }*/

            // No need to do channel restore frequently for quck p2p scan
            SelMaxScanChannelCnt = 4;
            if (((ScanType == SCAN_P2P) || (ScanType == SCAN_P2P_PASSIVE)) && (pPort->P2PCfg.ScanPeriod <= 10) && (pPort->P2PCfg.bGOStart))
                SelMaxScanChannelCnt = 11;

            if (((pAd->MlmeAux.MaxScanChannelCnt % SelMaxScanChannelCnt) == 0) && ((pPort->SoftAP.bAPStart) || (pPort->P2PCfg.bGOStart)))
            {
                pPort->BBPCurrentBW = pPort->CommonCfg.BwPri20Info.Bandwidth;

                if (pPort->BBPCurrentBW == BW_80/* && (INFRA_ON(pPort))*/)
                {
                    SwitchBandwidth(pAd, FALSE, pPort->CentralChannel, 
                        pPort->CommonCfg.BwPri20Info.Bandwidth, 
                        pPort->CommonCfg.BwPri20Info.Primary20Location);

                    //AsicSwitchChannel(pAd, pPort->CentralChannel,FALSE);
                    DBGPRINT(RT_DEBUG_TRACE, ("SYNC - AP beacon, restore to 80MHz channel %d, Total BSS[%02d]\n", pPort->SoftAP.ApCfg.VhtOperation.VhtOpInfo.ChannelCenterFreqSeg1, pAd->ScanTab.BssNr));
                }
                                //  concurrent AP/GO/MS-GO
                else if (pPort->BBPCurrentBW == BW_40 /*&& (INFRA_ON(pPort))*/)
                {
                    //SwitchBandwidth(pAd, TRUE, pPort->CentralChannel, BW_40, pPort->SoftAP.ApCfg.AddHTInfoIe.AddHtInfo.ExtChanOffset);                 DBGPRINT(RT_DEBUG_TRACE, ("SYNC - AP beacon, restore to 40MHz channel %d, Total BSS[%02d]\n",pPort->CentralChannel, pAd->ScanTab.BssNr));
                    {
                       // AsicSwitchChannel(pAd, pPort->CentralChannel, FALSE);

                        BbSetTxRxBwCtrl(pAd, BW_40);
                    }
                    DBGPRINT(RT_DEBUG_TRACE, ("SYNC - AP beacon, restore to 40MHz channel %d, Total BSS[%02d]\n",pPort->CentralChannel, pAd->ScanTab.BssNr, __FUNCTION__));
                }
                else
                {
                    {
                       // AsicSwitchChannel(pAd, pPort->Channel, FALSE);
                    }
                    DBGPRINT(RT_DEBUG_TRACE, ("SYNC - AP beacon, restore to  20MHz channel %d, Total BSS[%02d]\n",pPort->Channel, pAd->ScanTab.BssNr, __FUNCTION__));
                }           

                AsicSwitchChannel(pPort, pPort->Channel, pPort->CentralChannel, pPort->BBPCurrentBW, FALSE);

                // increase concurrent SoftAp beacon quantity.
                if ((pAd->BeaconPort.pApGoPort) && (pAd->BeaconPort.pApGoPort->SoftAP.bAPStart) && (pAd->BeaconPort.pApGoPort->PortSubtype == PORTSUBTYPE_VwifiAP))
                {
                    MgntPktSendBeacon(pAd->BeaconPort.pApGoPort);
                }

                //resume beacon after restore channel       
                AsicResumeBssSync(pAd);
                // Set this action timer
                PlatformSetTimer(pPort, &pPort->Mlme.ChannelTimer, RESTORE_CHANNEL_TIME);
                return;
            }
            else
            {           
                // If the miniport driver is performing active scan on the second phase scanning, it should not clear scan table.
                // This is because the scan table is used to determine that the miniport driver should send Probe Request or not.
                if (pAd->MlmeAux.bActiveScanForHiddenAP == FALSE)
                {
                    if (pAd->MlmeAux.bRetainBSSEntriesInScanTable == FALSE && ScanType != SCAN_P2P)
                    {
                        // Remove all BSS entries with the specified channel before active scan or passive scan.
                        BssTableDeleteEntryByChannel(pAd, &pAd->ScanTab, pPort->ScaningChannel);
                    }
                }

                
                AsicSwitchChannel(pPort, pPort->ScaningChannel, pPort->ScaningChannel, BW_20, TRUE);
                pAd->MlmeAux.LastScanChannel = pPort->ScaningChannel ;
            }       
        
            if (pPort->ScaningChannel > 14)
            {
                if ((pPort->CommonCfg.bIEEE80211H == 1) && (RadarChannelCheck(pAd, pPort->ScaningChannel) || (JapanChannelCheck(pPort->ScaningChannel))))
                    ScanType = SCAN_PASSIVE;
            }

            //Global country domain(ch1-11:active scan, ch12-14 passive scan)
            if ((pPort->ScaningChannel <= 14) && (pPort->ScaningChannel >= 12) && ((pPort->CommonCfg.CountryRegion  & 0x7f) == REGION_31_BG_BAND))
            {
                if ((MlmeSyncHasHiddenAPAtThisChannel(pAd, pPort->ScaningChannel, FALSE) == TRUE) && (pAd->MlmeAux.ScanType == FAST_SCAN_ACTIVE))
                {
                    ScanType = FAST_SCAN_ACTIVE;
                }
                else
                {
                    ScanType = SCAN_PASSIVE;
                }
                DBGPRINT(RT_DEBUG_INFO, ("SYNC - global domain, passive scan (ch=%d)\n", pPort->ScaningChannel));
            }
            
            //Country Region 32 (ch1-11:active scan, ch12-13 passive scan)
            if ((pPort->ScaningChannel <= 13) && (pPort->ScaningChannel >= 12) && ((pPort->CommonCfg.CountryRegion  & 0x7f) == REGION_32_BG_BAND))
            {
                if ((MlmeSyncHasHiddenAPAtThisChannel(pAd, pPort->ScaningChannel, FALSE) == TRUE) && (pAd->MlmeAux.ScanType == FAST_SCAN_ACTIVE))
                {
                    ScanType = FAST_SCAN_ACTIVE;
                }
                else
                {
                    ScanType = SCAN_PASSIVE;
                }
                DBGPRINT(RT_DEBUG_TRACE, ("SYNC - Country Region 32, scan type = %d passive scan (ch=%d)\n", ScanType, pPort->ScaningChannel));
            }
            
            // carrier detection
            if (pPort->CommonCfg.bCarrierDetect)
            {
                ScanType = SCAN_PASSIVE;
            }

            if ((pPort->CommonCfg.bIEEE80211H == TRUE) || (pPort->CommonCfg.bCarrierDetect == TRUE))
            {
                if ((MlmeSyncHasHiddenAPAtThisChannel(pAd, pPort->ScaningChannel, FALSE) == TRUE) &&
                    ((pAd->MlmeAux.ScanType == FAST_SCAN_ACTIVE) || (pAd->MlmeAux.ScanType == SCAN_ACTIVE)))
                {
                    ScanType = FAST_SCAN_ACTIVE;
                    DBGPRINT(RT_DEBUG_INFO, ("SYNC - (802.11h and/or Carrier Detection is enabled.) Send ProbeReq packet for hidden AP  @ MlmeSyncScanMlmeSyncNextChannel() ...\n" ));
                }
                else if ((MlmeSyncHasWscAPAtThisChannel(pAd, pPort->ScaningChannel) == TRUE) &&
                        ((pAd->MlmeAux.ScanType == FAST_SCAN_ACTIVE) || (pAd->MlmeAux.ScanType == SCAN_ACTIVE)))
                {
                    ScanType = FAST_SCAN_ACTIVE;
                    DBGPRINT(RT_DEBUG_TRACE, ("SYNC - (802.11h and/or Carrier Detection is enabled.) Send ProbeReq packet for WSC AP  @ MlmeSyncScanMlmeSyncNextChannel() ...\n" ));
                }

#if(COMPILE_WIN8_ABOVE(CURRENT_OS_NDIS_VER))  
                if ((pAd->pNicCfg->NdisVersion >= RTMP_NDIS630_VERSION) && pAd->StaCfg.NLOEntry.NLOEnable == TRUE)
                {
                    ScanType = SCAN_ACTIVE; //ACTIVE scan for NLO to make it faster.
                }
#endif
            }

            // We need to shorten active scan time in order for WZC connect issue
            // Chnage the channel scan time for CISCO stuff based on its IAPP announcement
            if (ScanType == FAST_SCAN_ACTIVE) 
            {
                PlatformSetTimer(pPort, &pPort->Mlme.ScanTimer, FAST_ACTIVE_SCAN_TIME); 
            }
            else if ((ScanType == SCAN_P2P) || (ScanType == SCAN_P2P_PASSIVE))
            {
#if(COMPILE_WIN8_ABOVE(CURRENT_OS_NDIS_VER))  
                //if (pAd->pNicCfg->NdisVersion >= RTMP_NDIS630_VERSION)
                if (pPort->P2PCfg.P2PConnectState == P2P_CONNECT_NOUSE)
                {
                ULONG       P2pScanTimeout = 0;

                if (pAd->LogoTestCfg.OnTestingWHQL == TRUE)
                {// OnTestingWHQL only support ch 1~11
                    if ((pPort->P2PCfg.ScanPeriod < 20)) // for quck p2p connectopn
                    {
                        if(pPort->P2PCfg.P2pProprietary.ListenChanelCount == 1)
                                P2pScanTimeout = 100;
                        else
                            P2pScanTimeout = FAST_ACTIVE_SCAN_TIME;
                        }
                        else if (ScanType == SCAN_P2P_PASSIVE)
                        {
                            P2pScanTimeout = MAX_CHANNEL_TIME;
                    }
                    else
                    {
                        P2pScanTimeout = SHORT_CHANNEL_TIME(pAd);
                    }
                        
                        PlatformSetTimer(pPort, &pPort->Mlme.ScanTimer, P2pScanTimeout);                     
                }
                else
                {     // todo A band would not scan all channels
                    if ((pPort->P2PCfg.P2pProprietary.ListenChanelCount > 11) && (pPort->P2PCfg.ScanPeriod < 10))
                    {
                        P2pScanTimeout = 20;
                        PlatformSetTimer(pPort, &pPort->Mlme.ScanTimer, P2pScanTimeout);
                        DBGPRINT(RT_DEBUG_TRACE, ("%s 1. ListenChanelCount = %d ScanPeriod = %d P2pScanTimeout = %d", __FUNCTION__, pPort->P2PCfg.P2pProprietary.ListenChanelCount, pPort->P2PCfg.ScanPeriod, P2pScanTimeout));
                    }
                    else if ((pPort->P2PCfg.P2pProprietary.ListenChanelCount > 11) && (pPort->P2PCfg.ScanPeriod > 10) && (pPort->P2PCfg.ScanPeriod < 50))
                    {
                        P2pScanTimeout = SHORT_CHANNEL_TIME(pAd);
                        PlatformSetTimer(pPort, &pPort->Mlme.ScanTimer, P2pScanTimeout);
                        DBGPRINT(RT_DEBUG_TRACE, ("%s 2. ListenChanelCount = %d ScanPeriod = %d P2pScanTimeout = %d", __FUNCTION__, pPort->P2PCfg.P2pProprietary.ListenChanelCount, pPort->P2PCfg.ScanPeriod, P2pScanTimeout));
                    }
                    else if((pPort->P2PCfg.P2pProprietary.ListenChanelCount <= 11) && (pPort->P2PCfg.ScanPeriod < 10))
                    {
                        P2pScanTimeout = FAST_ACTIVE_SCAN_TIME;
                        PlatformSetTimer(pPort, &pPort->Mlme.ScanTimer, P2pScanTimeout);
                        DBGPRINT(RT_DEBUG_TRACE, ("%s 3. ListenChanelCount = %d ScanPeriod = %d P2pScanTimeout = %d", __FUNCTION__, pPort->P2PCfg.P2pProprietary.ListenChanelCount, pPort->P2PCfg.ScanPeriod, P2pScanTimeout));
                    }
                    else if((pPort->P2PCfg.P2pProprietary.ListenChanelCount <= 11) && (pPort->P2PCfg.ScanPeriod > 10) && (pPort->P2PCfg.ScanPeriod < 50))
                    {
                        P2pScanTimeout = MAX_CHANNEL_TIME;
                        PlatformSetTimer(pPort, &pPort->Mlme.ScanTimer, P2pScanTimeout);
                        DBGPRINT(RT_DEBUG_TRACE, ("%s 4. ListenChanelCount = %d ScanPeriod = %d P2pScanTimeout = %d", __FUNCTION__, pPort->P2PCfg.P2pProprietary.ListenChanelCount, pPort->P2PCfg.ScanPeriod, P2pScanTimeout));
                    }
                    else if((pPort->P2PCfg.ScanPeriod >= 50))
                    {
                        P2pScanTimeout = MAX_CHANNEL_TIME;
                        PlatformSetTimer(pPort, &pPort->Mlme.ScanTimer, P2pScanTimeout);
                        DBGPRINT(RT_DEBUG_TRACE, ("%s 5. ListenChanelCount = %d ScanPeriod = %d P2pScanTimeout = %d", __FUNCTION__, pPort->P2PCfg.P2pProprietary.ListenChanelCount, pPort->P2PCfg.ScanPeriod, P2pScanTimeout));
                    }
                }
                
                DBGPRINT(RT_DEBUG_TRACE, ("%s - ch switch time (%d)ms\n", __FUNCTION__, P2pScanTimeout));
                }
                else
#endif
                {
                if((pPort->P2PCfg.P2PConnectState == P2P_NEGOTIATION) && (pPort->P2PCfg.ConnectingListenChannel == pPort->ScaningChannel) && (pAd->PortList[pPort->P2PCfg.PortNumber]->PortSubtype != PORTSUBTYPE_P2PGO) && (IS_P2P_SIGMA_OFF(pPort)))
                {   
                    DBGPRINT(RT_DEBUG_TRACE, ("Jesse in the peer listen channel will long stay"));
                    PlatformSetTimer(pPort, &pPort->Mlme.ScanTimer, 5000);
                }
                else if((pPort->P2PCfg.ConnectingListenChannel != 0) && (pPort->P2PCfg.ConnectingListenChannel == pPort->ScaningChannel))
                {
                    PlatformSetTimer(pPort, &pPort->Mlme.ScanTimer, 1500);
                    DBGPRINT(RT_DEBUG_TRACE, ("Jesse in the peer listen channel for Atheros"));
                }
                else if(pPort->P2PCfg.ConnectingListenChannel != 0)
                {
                    PlatformSetTimer(pPort, &pPort->Mlme.ScanTimer, 50/*pPort->P2PCfg.P2pCounter.ListenInterval*ONETU*/);
                    DBGPRINT(RT_DEBUG_TRACE, ("Jesse scan short 50"));
                }
                else if(pPort->P2PCfg.P2pProprietary.ListenChanelCount > 11)
                {
                    PlatformSetTimer(pPort, &pPort->Mlme.ScanTimer, SHORT_CHANNEL_TIME(pAd));
                }
                else
                {
                    PlatformSetTimer(pPort, &pPort->Mlme.ScanTimer, MAX_CHANNEL_TIME);
                    DBGPRINT(RT_DEBUG_TRACE, ("Jesse scan normal 140"));
                }
                }
            }
            else if ((pPort->P2PCfg.P2PConnectState == P2P_DO_GO_SCAN_OP_BEGIN) && (pPort->P2PCfg.GroupOpChannel != 0))
            {
                PlatformSetTimer(pPort, &pPort->Mlme.ScanTimer, HOLD_AT_OP_CHANNEL_TIME);
            }
            else if ((pPort->P2PCfg.P2PConnectState == P2P_DO_GO_NEG_DONE_CLIENT) && (pPort->P2PCfg.GroupOpChannel != 0))
            {
                PlatformSetTimer(pPort, &pPort->Mlme.ScanTimer, MAX_CHANNEL_TIME*3);
            }
            else // must be SCAN_PASSIVE or SCAN_ACTIVE
            {
                pAd->StaCfg.ScanChannelCnt++;
#if(COMPILE_WIN8_ABOVE(CURRENT_OS_NDIS_VER))         
                if (pAd->LogoTestCfg.OnTestingWHQL == TRUE)
                {
                    if ((pPort->P2PCfg.DeviceDiscoverable != DOT11_WFD_DEVICE_NOT_DISCOVERABLE) && (ScanType == SCAN_ACTIVE))
                    {
                        PlatformSetTimer(pPort, &pPort->Mlme.ScanTimer, P2P_LISTEN_PORT0_SCAN_TIME);
                    }
                    else
                    {
                        PlatformSetTimer(pPort, &pPort->Mlme.ScanTimer, pAd->LogoTestCfg.WHQLScanTime);
                    }
                }
                else
#endif
if ((pPort->CommonCfg.PhyMode == PHY_11ABG_MIXED) || 
                     (pPort->CommonCfg.PhyMode == PHY_11ABGN_MIXED) || 
                     (pPort->CommonCfg.PhyMode == PHY_11AGN_MIXED) || 
                     (pPort->CommonCfg.PhyMode == PHY_11VHT))
                {
                    if (pPort->ScaningChannel > 14)
                    {
                        if (pPort->ScaningChannel >= 157) // patch scan fail after channel 157
                        {
                            PlatformSetTimer(pPort, &pPort->Mlme.ScanTimer, SHORT_CHANNEL_TIME(pAd) * 2);
                        }
                        else
                        {
                            PlatformSetTimer(pPort, &pPort->Mlme.ScanTimer, SHORT_CHANNEL_TIME(pAd));
                        }
                    }
                    else    
                    {
                        PlatformSetTimer(pPort, &pPort->Mlme.ScanTimer, MIN_CHANNEL_TIME(pAd));
                    }
                }
                else
                {
                    PlatformSetTimer(pPort, &pPort->Mlme.ScanTimer, MAX_CHANNEL_TIME);
                }
            }       

            if ((ScanType == SCAN_P2P) && (IS_P2P_LISTENING(pPort)))
            {
                pPort->Mlme.SyncMachine.CurrState = SCAN_LISTEN;
                return;
            }
            // If I am GO and STA opmode, I don't send probe req to do scan... Not reasonable for a GO to send probe request.
            if ((pPort->PortSubtype == PORTSUBTYPE_P2PGO) && (pAd->OpMode == OPMODE_STA) && (ScanType != SCAN_P2P)) 
            {
                pPort->Mlme.SyncMachine.CurrState = SCAN_LISTEN;
                return;
            }
    
            // active scan, send ProbeReq
            if ((ScanType == SCAN_P2P) || (ScanType == SCAN_ACTIVE) || (ScanType == FAST_SCAN_ACTIVE) ||
                (ScanType == SCAN_WSC_ACTIVE) ||
                 (ScanType == SCAN_2040_BSS_COEXIST) )
            {
                //
                // Scan multiple SSID
                //      
                for (Index = 0; Index < pAd->MlmeAux.NumOfdot11SSIDs; Index++)
                {
                    NStatus = MlmeAllocateMemory(pAd, &pOutBuffer);  //Get an unused nonpaged memory
                    if (NStatus == NDIS_STATUS_SUCCESS)
                    {
                        PUCHAR        pSupRate = NULL;
                        UCHAR         SupRateLen = 0;
                        PUCHAR        pExtRate = NULL;
                        UCHAR         ExtRateLen = 0;   
                        UCHAR         ASupRate[] = {0x8C, 0x12, 0x98, 0x24, 0xb0, 0x48, 0x60, 0x6C};
                        UCHAR         ASupRateLen = sizeof(ASupRate)/sizeof(UCHAR); 

                        // get the proper support rate
                        if (pPort->ScaningChannel <= 14)
                        {
                            //CH14 only supports 11b
                            if (pPort->CommonCfg.Ch14BOnly && (pPort->ScaningChannel == CHANNEL_14))
                            {
                                pSupRate = pPort->CommonCfg.SupRate;
                                SupRateLen = 4;
                                ExtRateLen = 0;
                            }
                            else
                            {
                                pSupRate = pPort->CommonCfg.SupRate;
                                SupRateLen = pPort->CommonCfg.SupRateLen;
                                pExtRate = pPort->CommonCfg.ExtRate;
                                ExtRateLen = pPort->CommonCfg.ExtRateLen;
                            }
                        }
                        else
                        {
                            // Overwrite Support Rate, CCK rate are not allowed
                            pSupRate = ASupRate;
                            SupRateLen = ASupRateLen;
                            ExtRateLen = 0;
                        }
                        // There is no need to send broadcast probe request if active scan is in effect.
                        //for NdisTest 6.0, when resume from sleep ,if OID_DOT11_HIDDEN_NETWORK_ENABLED is TRUE,probe request
                        //contain non-zero length SSID esle contain 0 length SSID               
                        if (((ScanType == SCAN_ACTIVE) || (ScanType == FAST_SCAN_ACTIVE) || (ScanType == SCAN_WSC_ACTIVE))
                            && pPort->CommonCfg.bSSIDInProbeRequest)
                        {
                            if (pAd->MlmeAux.NumOfdot11SSIDs > 0)
                            {
                                SsidLen = (UCHAR)pAd->MlmeAux.Dot11Ssid[Index].uSSIDLength;
                                PlatformMoveMemory(pAd->MlmeAux.Ssid,pAd->MlmeAux.Dot11Ssid[Index].ucSSID,SsidLen); 
                            }
                            else
                            {
                                SsidLen = pAd->MlmeAux.SsidLen;
                            }                   
                        }
                        else
                        {
                            SsidLen = 0;
                        }

                        if (ScanType == SCAN_P2P)
                        {
                            UCHAR       SupRate[8];
                            // JANTEMP
                            // Use P2P Wildword SSID for both p2p device and legacy device
                            {
                                SsidLen = WILDP2PSSIDLEN;
                            }

                            SupRate[0]  = 0x8C;    // 6 mbps, in units of 0.5 Mbps, basic rate
                            SupRate[1]  = 0x12;    // 9 mbps, in units of 0.5 Mbps
                            SupRate[2]  = 0x98;    // 12 mbps, in units of 0.5 Mbps, basic rate
                            SupRate[3]  = 0x24;    // 18 mbps, in units of 0.5 Mbps
                            SupRate[4]  = 0xb0;    // 24 mbps, in units of 0.5 Mbps, basic rate
                            SupRate[5]  = 0x48;    // 36 mbps, in units of 0.5 Mbps
                            SupRate[6]  = 0x60;    // 48 mbps, in units of 0.5 Mbps
                            SupRate[7]  = 0x6c;    // 54 mbps, in units of 0.5 Mbps
                            SupRateLen  = 8;                            
                            // P2P scan must use P2P mac address.
                            MgtMacHeaderInit(pAd, pAd->PortList[pPort->P2PCfg.PortNumber], &Hdr80211, SUBTYPE_PROBE_REQ, 0, BROADCAST_ADDR, BROADCAST_ADDR);

                            MakeOutgoingFrame(pOutBuffer,               &FrameLen,
                                              sizeof(HEADER_802_11),    &Hdr80211,
                                              1,                        &SsidIe,
                                              1,                        &SsidLen,
                                              SsidLen,                        &WILDP2PSSID[0],
                                              1,                        &SupRateIe,
                                              1,                        &SupRateLen,
                                              SupRateLen,               SupRate, 
                                              END_OF_ARGS);

                        }
                        else
                        {
                                MgtMacHeaderInit(pAd,pPort, &Hdr80211, SUBTYPE_PROBE_REQ, 0, BROADCAST_ADDR, BROADCAST_ADDR);
        
                                MakeOutgoingFrame(pOutBuffer,               &FrameLen,
                                                sizeof(HEADER_802_11),    &Hdr80211,
                                                1,                        &SsidIe,
                                                1,                        &SsidLen,
                                                SsidLen,                    pAd->MlmeAux.Ssid,
                                                1,                        &SupRateIe,
                                                1,                        &SupRateLen,
                                                SupRateLen,  pSupRate, 
                                                END_OF_ARGS);           
                        

                            if (ExtRateLen)
                            {
                                ULONG Tmp;
                                MakeOutgoingFrame(pOutBuffer + FrameLen,            &Tmp,
                                                1,                                &ExtRateIe,
                                                1,                                &ExtRateLen,
                                                ExtRateLen,          pExtRate, 
                                                END_OF_ARGS);
                                FrameLen += Tmp;
                            }
                        }
                        
                        if (pPort->CommonCfg.Ch14BOnly && (pPort->ScaningChannel == CHANNEL_14))
                        {
                            //CH14 only supports 11B and will not carry any HT info on probe request
                            ;
                        }
                        else
                        {
                            if (pPort->CommonCfg.PhyMode >= PHY_11ABGN_MIXED)
                            {
                                ULONG Tmp;
                                if (!((pAd->StaCfg.BssType == BSS_ADHOC) && (pAd->StaCfg.bAdhocNMode==0)))
                                {
                                    HtLen = SIZE_HT_CAP_IE;
                                    MakeOutgoingFrame(pOutBuffer + FrameLen,            &Tmp,
                                                        1,                                &HtCapIe,
                                                        1,                                &HtLen,
                                                        HtLen,          &pPort->CommonCfg.HtCapability, 
                                                        END_OF_ARGS);
                                    FrameLen += Tmp;
                                }
                                
                                // If in 2.4G Channel, must carry ExtHtIE 
                                if ((pPort->ScaningChannel <= 14) && (pPort->CommonCfg.BACapability.field.b2040CoexistScanSup == 1))
                                {
                                    HtLen = sizeof(EXT_CAP_ELEMT);
                                    MakeOutgoingFrame(pOutBuffer + FrameLen,            &Tmp,
                                                      1,                                &ExtHtCapIe,
                                                      1,                                &HtLen,
                                                      HtLen,                            &pPort->CommonCfg.ExtCapIE.DoubleWord, 
                                                      END_OF_ARGS);

                                    FrameLen += Tmp;
                                }
                            }
                        }

#if(COMPILE_WIN8_ABOVE(CURRENT_OS_NDIS_VER))  
                        // For P2pMs, before connecting to group request, driver will trigger FAST scan when there is no GO available, 
                        // we need to change the ScanType to SCAN_P2P to use Microsoft WSC IE
                        NewScanType = ScanType;

                        if ((ScanType == FAST_SCAN_ACTIVE) && (pPort->P2PCfg.P2pMsSatetPhase == P2pMs_STATE_PROVISIONING))
                        {
                            NewScanType = SCAN_P2P;
                            pPort->P2PCfg.DiscoverTriggerType = P2pMs_DISCOVER_BY_DRIVER;
                        }
                                                
                        // 1. Probe req triggered by OS && WFDProprietaryIEsLen > 0                 --> use WFDProprietaryIEs
                        if (NDIS_WIN8_ABOVE(pAd) && 
                            (NewScanType == SCAN_P2P) && 
                            (pPort->P2PCfg.DiscoverTriggerType == P2pMs_DISCOVER_BY_OS) && 
                            (pPort->P2PCfg.WFDProprietaryIEsLen > 0))
                        {
                            DBGPRINT(RT_DEBUG_TRACE, ("[WFDDBG] %s - Port#%d: Use Microsoft IE (WFDProprietaryIEs =%d) \n", __FUNCTION__, pPort->P2PCfg.PortNumber, pPort->P2PCfg.WFDProprietaryIEsLen));

                            MakeOutgoingFrame(pOutBuffer + FrameLen,        &Tmp,
                                              pPort->P2PCfg.WFDProprietaryIEsLen, pPort->P2PCfg.WFDProprietaryIEs,
                                              END_OF_ARGS);
                            FrameLen += Tmp;
                        }
                        // 2. Probe req triggered by OS && WFDProprietaryIEsLen = 0 && DefalutIEs > 0   --> use DefalutIEs (provided by additional IE)
                        else if (NDIS_WIN8_ABOVE(pAd) && 
                            (NewScanType == SCAN_P2P) && 
                            (pPort->P2PCfg.DiscoverTriggerType == P2pMs_DISCOVER_BY_OS) && 
                            (pPort->P2PCfg.WFDProprietaryIEsLen == 0) &&
                            (pAd->PortList[pPort->P2PCfg.PortNumber]->PortCfg.P2PAdditionalRequestIESize > 0) && 
                            (pAd->PortList[pPort->P2PCfg.PortNumber]->PortCfg.P2PAdditionalRequestIEData != NULL))
                        {
                            DBGPRINT(RT_DEBUG_TRACE, ("[WFDDBG] %s - Use Microsoft IE (DefalutIEsLen =%d) \n", __FUNCTION__, pAd->PortList[pPort->P2PCfg.PortNumber]->PortCfg.P2PAdditionalRequestIESize));

                            MakeOutgoingFrame(pOutBuffer + FrameLen,        &Tmp,
                                              pAd->PortList[pPort->P2PCfg.PortNumber]->PortCfg.P2PAdditionalRequestIESize,    pAd->PortList[pPort->P2PCfg.PortNumber]->PortCfg.P2PAdditionalRequestIEData,
                                              END_OF_ARGS);
                            FrameLen += Tmp;                            
                        }
                        // 3. Probe req triggered by Driver && DefalutIEs > 0                       --> use DefalutIEs (provided by additional IE)
                        else if (NDIS_WIN8_ABOVE(pAd) && 
                            (NewScanType == SCAN_P2P) && 
                            (pPort->P2PCfg.DiscoverTriggerType == P2pMs_DISCOVER_BY_DRIVER) && 
                            (pAd->PortList[pPort->P2PCfg.PortNumber]->PortCfg.P2PAdditionalRequestIESize > 0) && 
                            (pAd->PortList[pPort->P2PCfg.PortNumber]->PortCfg.P2PAdditionalRequestIEData != NULL))
                        {
                            DBGPRINT(RT_DEBUG_TRACE, ("[WFDDBG] %s - Use Microsoft IE (DefalutIEsLen =%d) \n", __FUNCTION__, pAd->PortList[pPort->P2PCfg.PortNumber]->PortCfg.P2PAdditionalRequestIESize));

                            MakeOutgoingFrame(pOutBuffer + FrameLen,        &Tmp,
                                              pAd->PortList[pPort->P2PCfg.PortNumber]->PortCfg.P2PAdditionalRequestIESize,    pAd->PortList[pPort->P2PCfg.PortNumber]->PortCfg.P2PAdditionalRequestIEData,
                                              END_OF_ARGS);
                            FrameLen += Tmp;                            
                        }
                        else
#endif

                        // Append WSC information in probe request if WSC state is running
                        //if ((pAd->StaCfg.WscControl.WscEnProbeReqIE) && (pAd->StaCfg.WscControl.WscState != WSC_STATE_OFF))
                        {
                            UCHAR       WscBuf[512];
                            USHORT      WscIeLen;

                            DBGPRINT(RT_DEBUG_ERROR, ("[WFDDBG] %s - Use Ralink IE, ScanType=%d\n", __FUNCTION__, ScanType));

                            WscMakeProbeReqIE(pAd,
                                            pPort,
                                            (ScanType == SCAN_P2P) ? TRUE : FALSE,
                                            &WscBuf[0],
                                            &WscIeLen);
                            MakeOutgoingFrame(pOutBuffer + FrameLen, &Tmp, WscIeLen, WscBuf, END_OF_ARGS);

                            FrameLen += Tmp;
                        }

                        if ((pPort->PortCfg.AdditionalRequestIEData != NULL) && (pPort->PortCfg.AdditionalRequestIESize > 0))
                        {
                            DBGPRINT(RT_DEBUG_TRACE, ("Add Additional Request IE data to the Probe Request ...\n"));
                            MakeOutgoingFrame(pOutBuffer + FrameLen,        &Tmp,
                                                pPort->PortCfg.AdditionalRequestIESize, pPort->PortCfg.AdditionalRequestIEData,
                                                END_OF_ARGS);

                            FrameLen += Tmp;
                        }

                        //
                        // add IEs for VHT
                        //
                        if(pPort->ScaningChannel>14)
                        {
                            if(pPort->CommonCfg.PhyMode == PHY_11VHT)
                            {
                                ULONG Tmp;
                                UCHAR VhtCapIeLen = SIZE_OF_VHT_CAP_IE;
                                MakeOutgoingFrame(pOutBuffer+FrameLen,  &Tmp,
                                                    1,                  &VhtCapIe,
                                                    1,                  &VhtCapIeLen,
                                                    SIZE_OF_VHT_CAP_IE, &pPort->CommonCfg.DesiredVhtPhy.VhtCapability,
                                                    END_OF_ARGS);
                                FrameLen += Tmp;
                            }
                        }

                        // If CounterAftrScanButton < P2P_SCAN_SHORTPERIOD,
                        // also let normal scan to contain P2P IE. so that we can scan all supported channels instead only scan social channels.
                        if (ScanType == SCAN_P2P)
                        {
                            ULONG   P2PIeLen;
                            PUCHAR  ptr;
                            
                            // Put P2P IE to the last.
                            ptr = pOutBuffer + FrameLen;
                            P2pMakeP2pIE(pAd, pAd->PortList[pPort->P2PCfg.PortNumber],  PORTSUBTYPE_P2PClient, SUBTYPE_PROBE_REQ, ptr, &P2PIeLen);
    
                            FrameLen += P2PIeLen;

                        }
#if(COMPILE_WIN8_ABOVE(CURRENT_OS_NDIS_VER))  
                        // NdisCommonMiniportMMRequest will send out OutBuffer packet and then free it in NdisCommonUsbDequeueMLMEPacket
                        // Get more nonpaged memory for more probe request.
                        if (ScanType == SCAN_P2P)
                        {
                            // Allocate Buffer 2
                            NStatus2 = MlmeAllocateMemory(pAd, &pOutBuffer2);       
                            if (NStatus2 == NDIS_STATUS_SUCCESS)
                            {
                                PlatformMoveMemory(pOutBuffer2, pOutBuffer, FrameLen);
                            }

                            // Allocate Buffer 3
                            NStatus3 = MlmeAllocateMemory(pAd, &pOutBuffer3);       
                            if (NStatus3 == NDIS_STATUS_SUCCESS)
                            {
                                PlatformMoveMemory(pOutBuffer3, pOutBuffer, FrameLen);
                            }
                            
                        }
#endif                      
                        NdisCommonMiniportMMRequest(pAd, pOutBuffer, FrameLen);           
                        DBGPRINT(RT_DEBUG_TRACE, ("SYNC - send ProbeReq(1) @ channel=%d, Len=%d\n", pPort->ScaningChannel, FrameLen));


#if(COMPILE_WIN8_ABOVE(CURRENT_OS_NDIS_VER))  
                        //
                        // Send more times ProbeReq to increase the probability of finding peer p2p device
                        //
                        
                        // Above resend function might use different porbe req content compared to 1st time probe req, so use the same probe req for 2nd time
                        if  (ScanType == SCAN_P2P)
                        {
                            // 2nd probe request
                            if (NStatus2 == NDIS_STATUS_SUCCESS)
                            {
                                NdisCommonMiniportMMRequest(pAd, pOutBuffer2, FrameLen);          
                                DBGPRINT(RT_DEBUG_TRACE, ("SYNC - send ProbeReq(2) @ channel=%d, Len=%d\n", pPort->ScaningChannel, FrameLen));
                            }

                            // 3rd probe request
                            // more probe request to gain p2p device survey
                            if ((pAd->LogoTestCfg.OnTestingWHQL == TRUE) && (NStatus3 == NDIS_STATUS_SUCCESS))
                            {
                                Delay_us(1);

                                //P2pMsFreeIeMemory(&pPort->P2PCfg.pPobeReqBuffer, &pPort->P2PCfg.PobeReqBufferLen);
                                //pPort->P2PCfg.PobeReqBufferLen = FrameLen;
                                //RTMP_ALLOCATE_MEMORY_P2pMs(pAd->AdapterHandle, pPort->P2PCfg.pPobeReqBuffer, pPort->P2PCfg.PobeReqBufferLen, NIC_TAG_INFO);
                                //if (pPort->P2PCfg.pPobeReqBuffer != NULL) 
                                if ((pOutBuffer3 != NULL) && (FrameLen < 512))
                                {
                                    pPort->P2PCfg.pPobeReqBuffer = ProbeReqBuffer;
                                    PlatformMoveMemory(pPort->P2PCfg.pPobeReqBuffer, pOutBuffer3, FrameLen);
                                    pPort->P2PCfg.PobeReqBufferLen = FrameLen;
                                    
                                    DBGPRINT(RT_DEBUG_TRACE, ("SYNC - Buffer P2P Probe Req, FrameLen(%d)\n", FrameLen));
                                }
                                else
                                {
                                    pPort->P2PCfg.PobeReqBufferLen = 0;
                                    DBGPRINT(RT_DEBUG_TRACE, ("SYNC -pPobeReqBuffer = NULL\n"));
                                }

                                DBGPRINT(RT_DEBUG_TRACE, ("SYNC - send ProbeReq(3) @ channel=%d, Len=%d\n", pPort->ScaningChannel, FrameLen));
                                NdisCommonMiniportMMRequest(pAd, pOutBuffer3, FrameLen);

                                pPort->P2PCfg.SetPobeReqTimer = TRUE;
                                PlatformSetTimer(pPort, &pPort->P2PCfg.P2pSendProReqTimer, 6);  // resolution = 10ms

                            }
                            else if ((NStatus3 == NDIS_STATUS_SUCCESS) && pOutBuffer3)
                            {
                                MlmeFreeMemory(pAd, pOutBuffer3);
                            }
                        }
                        else
                        {
                            if ((NStatus2 == NDIS_STATUS_SUCCESS) && pOutBuffer2)
                                MlmeFreeMemory(pAd, pOutBuffer2);
                            if ((NStatus3 == NDIS_STATUS_SUCCESS) && pOutBuffer3)
                                MlmeFreeMemory(pAd, pOutBuffer3);
                        }
#endif                      
                    }           
                    else
                    {
                        DBGPRINT(RT_DEBUG_ERROR, ("SYNC - MlmeSyncScanMlmeSyncNextChannel() allocate memory fail\n"));
                    }
                }
            }
        }
        // For SCAN_CISCO_PASSIVE, do nothing and silently wait for beacon or other probe reponse
        
        pPort->Mlme.SyncMachine.CurrState = SCAN_LISTEN;        
            
    }
}

/* 
    ==========================================================================
    Description:
        Send more than one probe request during p2p search state to increase discovery possibility.
    ==========================================================================
 */
VOID MlmeSyncScanReqAction2(
    IN PMP_ADAPTER pAd, 
    IN MLME_QUEUE_ELEM *Elem) 
{
    PMP_PORT    pPort = pAd->PortList[Elem->PortNum];
    DBGPRINT(RT_DEBUG_TRACE,("%s::Handle MT_CMD_P2pMs_SEND_MORE_TIMES_PROBEREQ at ch(%d)\n", __FUNCTION__, pAd->HwCfg.LatchRfRegs.Channel));

    // Send more than one probe request during search state to increase discovery possibility.
    if (pAd->PortList[pPort->P2PCfg.PortNumber]->PortSubtype == PORTSUBTYPE_P2PGO)
        P2pSendProbeReq(pAd, pPort, PORTSUBTYPE_P2PGO);
    else
        P2pSendProbeReq(pAd, pPort, PORTSUBTYPE_P2PClient);
}


/* 
    ==========================================================================
    Description:
    ==========================================================================
 */
VOID MlmeSyncInvalidStateWhenScan(
    IN PMP_ADAPTER pAd, 
    IN MLME_QUEUE_ELEM *Elem) 
{
    USHORT      Status;
    PMP_PORT  pPort = pAd->PortList[Elem->PortNum];
    
    DBGPRINT(RT_DEBUG_TRACE, ("AYNC - MlmeSyncInvalidStateWhenScan(state=%d). Reset SYNC machine\n", pPort->Mlme.SyncMachine.CurrState));
    pPort->Mlme.SyncMachine.CurrState = SYNC_IDLE;
    Status = MLME_STATE_MACHINE_REJECT;

    PlatformIndicateScanStatus(pAd, pAd->PortList[pAd->ucScanPortNum], NDIS_STATUS_SUCCESS, TRUE, FALSE);

    MlmeEnqueue(pAd,pPort, MLME_CNTL_STATE_MACHINE, MT2_SCAN_CONF, 2, &Status);

    //
    // if scan is in progress, we need to clear it.
    //
    if (MT_TEST_FLAG(pAd, fRTMP_ADAPTER_BSS_SCAN_IN_PROGRESS))
        MT_CLEAR_FLAG(pAd, fRTMP_ADAPTER_BSS_SCAN_IN_PROGRESS);   
}

/* 
    ==========================================================================
    Description:
    ==========================================================================
 */
VOID MlmeSyncInvalidStateWhenJoin(
    IN PMP_ADAPTER pAd, 
    IN MLME_QUEUE_ELEM *Elem) 
{
    USHORT      Status;
    PMP_PORT  pPort = pAd->PortList[Elem->PortNum];
    
    DBGPRINT(RT_DEBUG_TRACE, ("MlmeSyncInvalidStateWhenJoin(state=%d). Reset SYNC machine\n", pPort->Mlme.SyncMachine.CurrState));
    pPort->Mlme.SyncMachine.CurrState = SYNC_IDLE;
    Status = MLME_STATE_MACHINE_REJECT;
    MlmeEnqueue(pAd,pPort, MLME_CNTL_STATE_MACHINE, MT2_JOIN_CONF, 2, &Status);

    //
    // if scan is in progress, we need to clear it.
    //
    if (MT_TEST_FLAG(pAd, fRTMP_ADAPTER_BSS_SCAN_IN_PROGRESS))
        MT_CLEAR_FLAG(pAd, fRTMP_ADAPTER_BSS_SCAN_IN_PROGRESS);   
}

/* 
    ==========================================================================
    Description:
    ==========================================================================
 */
VOID MlmeSyncInvalidStateWhenStart(
    IN PMP_ADAPTER pAd, 
    IN MLME_QUEUE_ELEM *Elem) 
{
    USHORT      Status;
    PMP_PORT  pPort = pAd->PortList[Elem->PortNum];
    
    DBGPRINT(RT_DEBUG_TRACE, ("MlmeSyncInvalidStateWhenStart(state=%d). Reset SYNC machine\n", pPort->Mlme.SyncMachine.CurrState));
    pPort->Mlme.SyncMachine.CurrState = SYNC_IDLE;
    Status = MLME_STATE_MACHINE_REJECT;
    MlmeEnqueue(pAd,pPort, MLME_CNTL_STATE_MACHINE, MT2_START_CONF, 2, &Status);

    //
    // if scan is in progress, we need to clear it.
    //
    if (MT_TEST_FLAG(pAd, fRTMP_ADAPTER_BSS_SCAN_IN_PROGRESS))
        MT_CLEAR_FLAG(pAd, fRTMP_ADAPTER_BSS_SCAN_IN_PROGRESS);   
}

#if 0
/* 
    ==========================================================================
    Description:
        Update StaCfg->ChannelList[] according to 1) Country Region 2) RF IC type,
        and 3) PHY-mode user selected.
        The outcome is used by driver when doing site survey.

    IRQL = PASSIVE_LEVEL
    IRQL = DISPATCH_LEVEL
    
    ==========================================================================
 */
VOID BuildChannelList(
    IN PMP_ADAPTER pAd)
{
    UCHAR i, j, index=0, num=0, size;
    PUCHAR  pChannelList = NULL;
    USHORT  usCh_List = 0;
    UCHAR   pManualChList[MAX_NUM_OF_CHANNELS] = {0};
    UCHAR   WHQLChannelOrder[BG_BAND_REGION_0_SIZE] = {0, 2, 5, 7, 10, 1, 3, 4, 6, 8, 9};

    DBGPRINT(RT_DEBUG_TRACE,("BuildChannelList(pPort->CommonCfg.PhyMode = %s)\n", DecodePhyMode(pPort->CommonCfg.PhyMode)));
    PlatformZeroMemory(pAd->HwCfg.ChannelList, MAX_NUM_OF_CHANNELS * sizeof(CHANNEL_TX_POWER));

    // if not 11a-only mode, channel list starts from 2.4Ghz band
    if ( ((pPort->CommonCfg.PhyMode != PHY_11A) && (pPort->CommonCfg.PhyMode != PHY_11AN_MIXED)) &&
        ((pAd->HwCfg.ChannelMode == ChMode_24G) || (pAd->HwCfg.ChannelMode == ChMode_24G_5G)) )
    {
        switch (pPort->CommonCfg.CountryRegion  & 0x7f)
        {
            case REGION_0_BG_BAND:  // 1 -11
                if(pAd->LogoTestCfg.OnTestingWHQL)
                {
                    for(i=0; i < BG_BAND_REGION_0_SIZE; i++)
                    {
                        PlatformMoveMemory(&pAd->HwCfg.ChannelList[i], &pAd->TxPower[WHQLChannelOrder[i]], sizeof(CHANNEL_TX_POWER));
                    }
                }
                else
                {
                    PlatformMoveMemory(&pAd->HwCfg.ChannelList[index], &pAd->TxPower[BG_BAND_REGION_0_START], sizeof(CHANNEL_TX_POWER) * BG_BAND_REGION_0_SIZE);
                }
                index += BG_BAND_REGION_0_SIZE;
                break;
            case REGION_1_BG_BAND:  // 1 - 13
                PlatformMoveMemory(&pAd->HwCfg.ChannelList[index], &pAd->TxPower[BG_BAND_REGION_1_START], sizeof(CHANNEL_TX_POWER) * BG_BAND_REGION_1_SIZE);
                index += BG_BAND_REGION_1_SIZE;
                break;
            case REGION_2_BG_BAND:  // 10 - 11
                PlatformMoveMemory(&pAd->HwCfg.ChannelList[index], &pAd->TxPower[BG_BAND_REGION_2_START], sizeof(CHANNEL_TX_POWER) * BG_BAND_REGION_2_SIZE);
                index += BG_BAND_REGION_2_SIZE;
                break;
            case REGION_3_BG_BAND:  // 10 - 13
                PlatformMoveMemory(&pAd->HwCfg.ChannelList[index], &pAd->TxPower[BG_BAND_REGION_3_START], sizeof(CHANNEL_TX_POWER) * BG_BAND_REGION_3_SIZE);
                index += BG_BAND_REGION_3_SIZE;
                break;
            case REGION_4_BG_BAND:  // 14
                PlatformMoveMemory(&pAd->HwCfg.ChannelList[index], &pAd->TxPower[BG_BAND_REGION_4_START], sizeof(CHANNEL_TX_POWER) * BG_BAND_REGION_4_SIZE);
                index += BG_BAND_REGION_4_SIZE;
                break;
            case REGION_5_BG_BAND:  // 1 - 14
                PlatformMoveMemory(&pAd->HwCfg.ChannelList[index], &pAd->TxPower[BG_BAND_REGION_5_START], sizeof(CHANNEL_TX_POWER) * BG_BAND_REGION_5_SIZE);
                index += BG_BAND_REGION_5_SIZE;
                break;
            case REGION_6_BG_BAND:  // 3 - 9
                PlatformMoveMemory(&pAd->HwCfg.ChannelList[index], &pAd->TxPower[BG_BAND_REGION_6_START], sizeof(CHANNEL_TX_POWER) * BG_BAND_REGION_6_SIZE);
                index += BG_BAND_REGION_6_SIZE;
                break;
            case REGION_7_BG_BAND:  // 5 - 13
                PlatformMoveMemory(&pAd->HwCfg.ChannelList[index], &pAd->TxPower[BG_BAND_REGION_7_START], sizeof(CHANNEL_TX_POWER) * BG_BAND_REGION_7_SIZE);
                index += BG_BAND_REGION_7_SIZE;
                break;
            case REGION_30_BG_BAND: //Manual
                DBGPRINT(RT_DEBUG_TRACE, ("BuildChannelList -- Region_30_BG_Band enter ...\n"));
                usCh_List  = pAd->HwCfg.EEPROMManualChList[EEPROM_BG_BAND_MANUAL_CH];
                DBGPRINT(RT_DEBUG_TRACE, ("EEPROMManualChannelList = %04x",usCh_List));
                for(i = 0; i < 14; i++)
                {
                    if((usCh_List >> i) & 0x0001)
                    {
                        DBGPRINT(RT_DEBUG_TRACE, ("index%d = TxPower%d",index, i));
                        PlatformMoveMemory(&pAd->HwCfg.ChannelList[index], &pAd->TxPower[i], sizeof(CHANNEL_TX_POWER));
                        index++;
                    }
                }

                break;
            case REGION_31_BG_BAND: // 1 - 14
                PlatformMoveMemory(&pAd->HwCfg.ChannelList[index], &pAd->TxPower[BG_BAND_REGION_31_START], sizeof(CHANNEL_TX_POWER) * BG_BAND_REGION_31_SIZE);
                index += BG_BAND_REGION_31_SIZE;
                break;
                
            //Country Region 32
            case REGION_32_BG_BAND: // 1 - 13
                PlatformMoveMemory(&pAd->HwCfg.ChannelList[index], &pAd->TxPower[BG_BAND_REGION_32_START], sizeof(CHANNEL_TX_POWER) * BG_BAND_REGION_32_SIZE);
                index += BG_BAND_REGION_32_SIZE;
                break;
                
            //Country Region 33
            case REGION_33_BG_BAND: // 1 - 14
                //Disallow CH14 in 802.11g/n mode 
                size = (pPort->CommonCfg.PhyMode == PHY_11B) ? BG_BAND_REGION_33_SIZE : BG_BAND_REGION_33_SIZE - 1;
                PlatformMoveMemory(&pAd->HwCfg.ChannelList[index], &pAd->TxPower[BG_BAND_REGION_33_START], sizeof(CHANNEL_TX_POWER) * size);
                index += size;
                break;
                
            default:            // Error. should never happen
                break;
        }
    }

    if ( ((pPort->CommonCfg.PhyMode == PHY_11A) || 
         (pPort->CommonCfg.PhyMode == PHY_11ABG_MIXED) || 
         (pPort->CommonCfg.PhyMode == PHY_11ABGN_MIXED) || 
         (pPort->CommonCfg.PhyMode == PHY_11AN_MIXED) || 
         (pPort->CommonCfg.PhyMode == PHY_11AGN_MIXED) || 
         (pPort->CommonCfg.PhyMode == PHY_11VHT)) &&
         ((pAd->HwCfg.ChannelMode == ChMode_24G_5G) ||
         (pAd->HwCfg.ChannelMode == ChMode_5G)) )
    {
        switch (pPort->CommonCfg.CountryRegionForABand & 0x7f)
        {
            case REGION_0_A_BAND:
                num = sizeof(A_BAND_REGION_0_CHANNEL_LIST)/sizeof(UCHAR);
                pChannelList = A_BAND_REGION_0_CHANNEL_LIST;
                break;
            case REGION_1_A_BAND:
                num = sizeof(A_BAND_REGION_1_CHANNEL_LIST)/sizeof(UCHAR);
                pChannelList = A_BAND_REGION_1_CHANNEL_LIST;
                break;
            case REGION_2_A_BAND:
                num = sizeof(A_BAND_REGION_2_CHANNEL_LIST)/sizeof(UCHAR);
                pChannelList = A_BAND_REGION_2_CHANNEL_LIST;
                break;
            case REGION_3_A_BAND:
                num = sizeof(A_BAND_REGION_3_CHANNEL_LIST)/sizeof(UCHAR);
                pChannelList = A_BAND_REGION_3_CHANNEL_LIST;
                break;
            case REGION_4_A_BAND:
                num = sizeof(A_BAND_REGION_4_CHANNEL_LIST)/sizeof(UCHAR);
                pChannelList = A_BAND_REGION_4_CHANNEL_LIST;
                break;
            case REGION_5_A_BAND:
                num = sizeof(A_BAND_REGION_5_CHANNEL_LIST)/sizeof(UCHAR);
                pChannelList = A_BAND_REGION_5_CHANNEL_LIST;
                break;
            case REGION_6_A_BAND:
                num = sizeof(A_BAND_REGION_6_CHANNEL_LIST)/sizeof(UCHAR);
                pChannelList = A_BAND_REGION_6_CHANNEL_LIST;
                break;
            case REGION_7_A_BAND:
                num = sizeof(A_BAND_REGION_7_CHANNEL_LIST)/sizeof(UCHAR);
                pChannelList = A_BAND_REGION_7_CHANNEL_LIST;
                break;
            case REGION_8_A_BAND:
                num = sizeof(A_BAND_REGION_8_CHANNEL_LIST)/sizeof(UCHAR);
                pChannelList = A_BAND_REGION_8_CHANNEL_LIST;
                break;
            case REGION_9_A_BAND:
                num = sizeof(A_BAND_REGION_9_CHANNEL_LIST)/sizeof(UCHAR);
                pChannelList = A_BAND_REGION_9_CHANNEL_LIST;
                break;              
            case REGION_10_A_BAND:
                num = sizeof(A_BAND_REGION_10_CHANNEL_LIST)/sizeof(UCHAR);
                pChannelList = A_BAND_REGION_10_CHANNEL_LIST;
                break;              
            case REGION_11_A_BAND:
                num = sizeof(A_BAND_REGION_11_CHANNEL_LIST)/sizeof(UCHAR);
                pChannelList = A_BAND_REGION_11_CHANNEL_LIST;
                break;              
            case REGION_12_A_BAND:
                num = sizeof(A_BAND_REGION_12_CHANNEL_LIST)/sizeof(UCHAR);
                pChannelList = A_BAND_REGION_12_CHANNEL_LIST;
                break;              
            case REGION_13_A_BAND:
                num = sizeof(A_BAND_REGION_13_CHANNEL_LIST)/sizeof(UCHAR);
                pChannelList = A_BAND_REGION_13_CHANNEL_LIST;
                break;
            case REGION_14_A_BAND:
                num = sizeof(A_BAND_REGION_14_CHANNEL_LIST)/sizeof(UCHAR);
                pChannelList = A_BAND_REGION_14_CHANNEL_LIST;
                break;
            case REGION_15_A_BAND:
                num = sizeof(A_BAND_REGION_15_CHANNEL_LIST)/sizeof(UCHAR);
                pChannelList = A_BAND_REGION_15_CHANNEL_LIST;
                break;
            case REGION_16_A_BAND:
                num = sizeof(A_BAND_REGION_16_CHANNEL_LIST)/sizeof(UCHAR);
                pChannelList = A_BAND_REGION_16_CHANNEL_LIST;               
                break;
            case REGION_17_A_BAND:
                num = sizeof(A_BAND_REGION_17_CHANNEL_LIST)/sizeof(UCHAR);
                pChannelList = A_BAND_REGION_17_CHANNEL_LIST;               
                break;
            case REGION_30_A_BAND:
                DBGPRINT(RT_DEBUG_TRACE, ("BuildChannelList -- Region_30_A_Band enter ...\n"));
                DBGPRINT(RT_DEBUG_TRACE, ("index = %d, num = %d\n",index,num));
                usCh_List  = pAd->HwCfg.EEPROMManualChList[EEPROM_A_BAND_MANUAL_CH_0];
                DBGPRINT(RT_DEBUG_TRACE, ("EEPROMManualChannelList_A_Band_0 = 0x%04x",usCh_List));

                // for 0x11A
                for(i = 0; i < 8; i++)
                {
                    if((usCh_List >> i) & 0x0001)
                    {
                        pManualChList[num] = i*4 + 36;
                        num++;
                    }
                }

                // for 0x11B
                for(i = 8; i < 16; i++)
                {
                    if((usCh_List >> i) & 0x0001)
                    {
                        pManualChList[num] = (i-8)*4 + 100;
                        num++;
                    }
                }
                usCh_List = pAd->HwCfg.EEPROMManualChList[EEPROM_A_BAND_MANUAL_CH_1];
                DBGPRINT(RT_DEBUG_TRACE, ("EEPROMManualChannelList_A_Band_1 = 0x%04x",usCh_List));  

                // for 0x11C
                for(i = 0; i < 3; i++)
                {
                    if((usCh_List >> i) & 0x0001)
                    {
                        pManualChList[num] = i*4 + 132;
                        num++;
                    }
                }

                for(i = 3; i < 8; i++)
                {
                    if((usCh_List >> i) & 0x0001)
                    {
                        pManualChList[num] = (i-3)*4+149;
                        num++;
                    }
                }

                // for 0x11D
                for(i = 8; i < 12; i++)
                {
                    if((usCh_List >> i) & 0x0001)
                    {
                        pManualChList[num] = (i-8)*2 + 167;
                        num++;
                    }
                }

                DBGPRINT(RT_DEBUG_TRACE, ("num = %d\n",num));
                
                break;
            default:            // Error. should never happen
                DBGPRINT(RT_DEBUG_WARN,("countryregion=%d not support", pPort->CommonCfg.CountryRegionForABand));
                break;
        }

        if (num != 0)
        {
            if((pPort->CommonCfg.CountryRegionForABand & 0x7f) == REGION_30_A_BAND)
            {
                for (i=0; i<num; i++)
                {
                    for (j=0; j<MAX_NUM_OF_CHANNELS; j++)
                    {
                        if (pManualChList[i] == pAd->TxPower[j].Channel)
                        {
                            DBGPRINT(RT_DEBUG_TRACE, ("pManualChList[%d] = %d\n",i,pManualChList[i]));
                            PlatformMoveMemory(&pAd->HwCfg.ChannelList[index+i], &pAd->TxPower[j], sizeof(CHANNEL_TX_POWER));
                        }
                    }
                }
            }
            else
            {
                // Must care the pChannelList = Null condition, when new add channel list
                if(pChannelList == NULL)
                {
                    DBGPRINT(RT_DEBUG_TRACE, ("[%s %d] Error: pChannelList = NULL\n",__FUNCTION__,__LINE__));
                }
                else
                {
                for (i = 0; i < num; i++)
                {
                    for (j = 0; j < MAX_NUM_OF_CHANNELS; j++)
                    {
                        if (pChannelList[i] == pAd->TxPower[j].Channel)
                        {
                            PlatformMoveMemory(&pAd->HwCfg.ChannelList[index+i], &pAd->TxPower[j], sizeof(CHANNEL_TX_POWER));
                        }
                    }
                }
            }
            }
            index += num;
        }
    }

    //rt2860b emulation set channellist. take away.
    pAd->HwCfg.ChannelListNum = index;
    DBGPRINT(RT_DEBUG_TRACE,("country code=%d/%d, RFIC=%d, PHY mode=%d, support %d channels\n", 
        pPort->CommonCfg.CountryRegion, pPort->CommonCfg.CountryRegionForABand, pAd->HwCfg.RfIcType, pPort->CommonCfg.PhyMode, pAd->HwCfg.ChannelListNum));
#if DBG 
    for (i = 0;i < pAd->HwCfg.ChannelListNum;i++)
    {
        DBGPRINT_RAW(RT_DEBUG_TRACE,(" #%d", pAd->HwCfg.ChannelList[i].Channel));
    }
#endif
}
#else
/* 
    ==========================================================================
    Description:
        Update StaCfg->ChannelList[] according to 1) Country Region 2) RF IC type,
        and 3) PHY-mode user selected.
        The outcome is used by driver when doing site survey.

    IRQL = PASSIVE_LEVEL
    IRQL = DISPATCH_LEVEL

    ==========================================================================
 */
 VOID BuildChannelList(
    IN PMP_ADAPTER pAd,
    IN PMP_PORT     pPort)
{
    UCHAR   i, j, index=0, num=0, size;
    PUCHAR  pChannelList = NULL;
    USHORT  usCh_List = 0;
    UCHAR   pManualChList[MAX_NUM_OF_CHANNELS] = {0};
    UCHAR   WHQLChannelOrder[BG_BAND_REGION_0_SIZE] = {0, 2, 5, 7, 10, 1, 3, 4, 6, 8, 9};

    DBGPRINT(RT_DEBUG_TRACE,("BuildChannelList(pPort->CommonCfg.PhyMode = %s)\n", DecodePhyMode(pPort->CommonCfg.PhyMode)));
    PlatformZeroMemory(pAd->HwCfg.ChannelList, MAX_NUM_OF_CHANNELS * sizeof(CHANNEL_TX_POWER));

    // if not 11a-only mode, channel list starts from 2.4Ghz band
    if ( ((pPort->CommonCfg.PhyMode != PHY_11A) && (pPort->CommonCfg.PhyMode != PHY_11AN_MIXED)) &&
        ((pAd->HwCfg.ChannelMode == ChMode_24G) || (pAd->HwCfg.ChannelMode == ChMode_24G_5G)) )
    {
        switch (pPort->CommonCfg.CountryRegion  & 0x7f)
        {
            case REGION_0_BG_BAND:  // 1 -11
                if(pAd->LogoTestCfg.OnTestingWHQL)
                {
                    for(i=0; i < BG_BAND_REGION_0_SIZE; i++)
                    {
                        pAd->HwCfg.ChannelList[i].Channel = BG_BAND_REGION_WHQL_CHANNEL_LIST[i];
                    }
                }
                else
                {
                    for(i=0; i < BG_BAND_REGION_0_SIZE; i++)
                    {
                        pAd->HwCfg.ChannelList[i].Channel = BG_BAND_REGION_0_CHANNEL_LIST[i];
                    }
                }
                index += BG_BAND_REGION_0_SIZE;
                break;
            case REGION_1_BG_BAND:  // 1 - 13
                for(i=0; i < BG_BAND_REGION_1_SIZE; i++)
                {
                    pAd->HwCfg.ChannelList[i].Channel = BG_BAND_REGION_1_CHANNEL_LIST[i];
                }
                index += BG_BAND_REGION_1_SIZE;
                break;
            case REGION_2_BG_BAND:  // 10 - 11
                for(i=0; i < BG_BAND_REGION_2_SIZE; i++)
                {
                    pAd->HwCfg.ChannelList[i].Channel = BG_BAND_REGION_2_CHANNEL_LIST[i];
                }
                index += BG_BAND_REGION_2_SIZE;
                break;
            case REGION_3_BG_BAND:  // 10 - 13
                for(i=0; i < BG_BAND_REGION_3_SIZE; i++)
                {
                    pAd->HwCfg.ChannelList[i].Channel = BG_BAND_REGION_3_CHANNEL_LIST[i];
                }
                index += BG_BAND_REGION_3_SIZE;
                break;
            case REGION_4_BG_BAND:  // 14
                for(i=0; i < BG_BAND_REGION_4_SIZE; i++)
                {
                    pAd->HwCfg.ChannelList[i].Channel = BG_BAND_REGION_4_CHANNEL_LIST[i];
                }
                index += BG_BAND_REGION_4_SIZE;
                break;
            case REGION_5_BG_BAND:  // 1 - 14
                for(i=0; i < BG_BAND_REGION_5_SIZE; i++)
                {
                    pAd->HwCfg.ChannelList[i].Channel = BG_BAND_REGION_5_CHANNEL_LIST[i];
                }
                index += BG_BAND_REGION_5_SIZE;
                break;
            case REGION_6_BG_BAND:  // 3 - 9
                for(i=0; i < BG_BAND_REGION_6_SIZE; i++)
                {
                    pAd->HwCfg.ChannelList[i].Channel = BG_BAND_REGION_6_CHANNEL_LIST[i];
                }
                index += BG_BAND_REGION_6_SIZE;
                break;
            case REGION_7_BG_BAND:  // 5 - 13
                for(i=0; i < BG_BAND_REGION_7_SIZE; i++)
                {
                pAd->HwCfg.ChannelList[i].Channel = BG_BAND_REGION_7_CHANNEL_LIST[i];
                }
                index += BG_BAND_REGION_7_SIZE;
                break;
            case REGION_30_BG_BAND: //Manual
                DBGPRINT(RT_DEBUG_TRACE, ("BuildChannelList -- Region_30_BG_Band enter ...\n"));
                usCh_List  = pAd->HwCfg.EEPROMManualChList[EEPROM_BG_BAND_MANUAL_CH];
                DBGPRINT(RT_DEBUG_TRACE, ("EEPROMManualChannelList = %04x",usCh_List));
                for(i = 0; i < 14; i++)
                {
                    if((usCh_List >> i) & 0x0001)
                    {
                        DBGPRINT(RT_DEBUG_TRACE, ("index%d = TxPower%d",index, i));
                        PlatformMoveMemory(&pAd->HwCfg.ChannelList[index], &pAd->TxPower[i], sizeof(CHANNEL_TX_POWER));
                        index++;
                    }
                }

                break;
            case REGION_31_BG_BAND: // 1 - 14
                for(i=0; i < BG_BAND_REGION_31_SIZE; i++)
                {
                    pAd->HwCfg.ChannelList[i].Channel = BG_BAND_REGION_31_CHANNEL_LIST[i];
                }
                index += BG_BAND_REGION_31_SIZE;
                break;

            //Country Region 32
            case REGION_32_BG_BAND: // 1 - 13
                for(i=0; i < BG_BAND_REGION_32_SIZE; i++)
                {
                    pAd->HwCfg.ChannelList[i].Channel = BG_BAND_REGION_32_CHANNEL_LIST[i];
                }
                index += BG_BAND_REGION_32_SIZE;
                break;

            //Country Region 33
            case REGION_33_BG_BAND: // 1 - 14
                //Disallow CH14 in 802.11g/n mode 
                size = (pPort->CommonCfg.PhyMode == PHY_11B) ? BG_BAND_REGION_33_SIZE : BG_BAND_REGION_33_SIZE - 1;
                for(i=0; i < size; i++)
                {
                    pAd->HwCfg.ChannelList[i].Channel = BG_BAND_REGION_33_CHANNEL_LIST[i];
                }
                index += size;
                break;

            default:            // Error. should never happen
                break;
            }
        }

    if ( ((pPort->CommonCfg.PhyMode == PHY_11A) || 
         (pPort->CommonCfg.PhyMode == PHY_11ABG_MIXED) || 
         (pPort->CommonCfg.PhyMode == PHY_11ABGN_MIXED) || 
         (pPort->CommonCfg.PhyMode == PHY_11AN_MIXED) || 
         (pPort->CommonCfg.PhyMode == PHY_11AGN_MIXED) || 
         (pPort->CommonCfg.PhyMode == PHY_11VHT)) &&
         ((pAd->HwCfg.ChannelMode == ChMode_24G_5G) ||
         (pAd->HwCfg.ChannelMode == ChMode_5G)) )
    {
        switch (pPort->CommonCfg.CountryRegionForABand & 0x7f)
        {
            case REGION_0_A_BAND:
                num = sizeof(A_BAND_REGION_0_CHANNEL_LIST)/sizeof(UCHAR);
                pChannelList = A_BAND_REGION_0_CHANNEL_LIST;
                break;
            case REGION_1_A_BAND:
                num = sizeof(A_BAND_REGION_1_CHANNEL_LIST)/sizeof(UCHAR);
                pChannelList = A_BAND_REGION_1_CHANNEL_LIST;
                break;
                case REGION_2_A_BAND:
                num = sizeof(A_BAND_REGION_2_CHANNEL_LIST)/sizeof(UCHAR);
                pChannelList = A_BAND_REGION_2_CHANNEL_LIST;
                break;
            case REGION_3_A_BAND:
                num = sizeof(A_BAND_REGION_3_CHANNEL_LIST)/sizeof(UCHAR);
                pChannelList = A_BAND_REGION_3_CHANNEL_LIST;
                break;
            case REGION_4_A_BAND:
                num = sizeof(A_BAND_REGION_4_CHANNEL_LIST)/sizeof(UCHAR);
                pChannelList = A_BAND_REGION_4_CHANNEL_LIST;
                break;
            case REGION_5_A_BAND:
                num = sizeof(A_BAND_REGION_5_CHANNEL_LIST)/sizeof(UCHAR);
                pChannelList = A_BAND_REGION_5_CHANNEL_LIST;
                break;
            case REGION_6_A_BAND:
                num = sizeof(A_BAND_REGION_6_CHANNEL_LIST)/sizeof(UCHAR);
                pChannelList = A_BAND_REGION_6_CHANNEL_LIST;
                break;
            case REGION_7_A_BAND:
                num = sizeof(A_BAND_REGION_7_CHANNEL_LIST)/sizeof(UCHAR);
                pChannelList = A_BAND_REGION_7_CHANNEL_LIST;
                break;
            case REGION_8_A_BAND:
                num = sizeof(A_BAND_REGION_8_CHANNEL_LIST)/sizeof(UCHAR);
                pChannelList = A_BAND_REGION_8_CHANNEL_LIST;
                break;
            case REGION_9_A_BAND:
                num = sizeof(A_BAND_REGION_9_CHANNEL_LIST)/sizeof(UCHAR);
                pChannelList = A_BAND_REGION_9_CHANNEL_LIST;
                break;
            case REGION_10_A_BAND:
                num = sizeof(A_BAND_REGION_10_CHANNEL_LIST)/sizeof(UCHAR);
                pChannelList = A_BAND_REGION_10_CHANNEL_LIST;
                break;
            case REGION_11_A_BAND:
                num = sizeof(A_BAND_REGION_11_CHANNEL_LIST)/sizeof(UCHAR);
                pChannelList = A_BAND_REGION_11_CHANNEL_LIST;
                break;
            case REGION_12_A_BAND:
                num = sizeof(A_BAND_REGION_12_CHANNEL_LIST)/sizeof(UCHAR);
                pChannelList = A_BAND_REGION_12_CHANNEL_LIST;
                break;
            case REGION_13_A_BAND:
                num = sizeof(A_BAND_REGION_13_CHANNEL_LIST)/sizeof(UCHAR);
                pChannelList = A_BAND_REGION_13_CHANNEL_LIST;
                break;
            case REGION_14_A_BAND:
                num = sizeof(A_BAND_REGION_14_CHANNEL_LIST)/sizeof(UCHAR);
                pChannelList = A_BAND_REGION_14_CHANNEL_LIST;
                break;
            case REGION_15_A_BAND:
                num = sizeof(A_BAND_REGION_15_CHANNEL_LIST)/sizeof(UCHAR);
                pChannelList = A_BAND_REGION_15_CHANNEL_LIST;
                break;
            case REGION_16_A_BAND:
                num = sizeof(A_BAND_REGION_16_CHANNEL_LIST)/sizeof(UCHAR);
                pChannelList = A_BAND_REGION_16_CHANNEL_LIST;
                break;
            case REGION_17_A_BAND:
                num = sizeof(A_BAND_REGION_17_CHANNEL_LIST)/sizeof(UCHAR);
                pChannelList = A_BAND_REGION_17_CHANNEL_LIST;
                break;
            case REGION_30_A_BAND:
                DBGPRINT(RT_DEBUG_TRACE, ("BuildChannelList -- Region_30_A_Band enter ...\n"));
                DBGPRINT(RT_DEBUG_TRACE, ("index = %d, num = %d\n",index,num));
                usCh_List  = pAd->HwCfg.EEPROMManualChList[EEPROM_A_BAND_MANUAL_CH_0];
                DBGPRINT(RT_DEBUG_TRACE, ("EEPROMManualChannelList_A_Band_0 = 0x%04x",usCh_List));

                // for 0x11A
                for(i = 0; i < 8; i++)
                {
                    if((usCh_List >> i) & 0x0001)
                    {
                        pManualChList[num] = i*4 + 36;
                        num++;
                    }
                }

                // for 0x11B
                for(i = 8; i < 16; i++)
                {
                    if((usCh_List >> i) & 0x0001)
                    {
                        pManualChList[num] = (i-8)*4 + 100;
                        num++;
                    }
                }
                usCh_List = pAd->HwCfg.EEPROMManualChList[EEPROM_A_BAND_MANUAL_CH_1];
                DBGPRINT(RT_DEBUG_TRACE, ("EEPROMManualChannelList_A_Band_1 = 0x%04x",usCh_List));	

                // for 0x11C
                for(i = 0; i < 3; i++)
                {
                    if((usCh_List >> i) & 0x0001)
                    {
                        pManualChList[num] = i*4 + 132;
                        num++;
                    }
                }

                for(i = 3; i < 8; i++)
                {
                    if((usCh_List >> i) & 0x0001)
                    {
                        pManualChList[num] = (i-3)*4+149;
                        num++;
                    }
                }

                // for 0x11D
                for(i = 8; i < 12; i++)
                {
                    if((usCh_List >> i) & 0x0001)
                    {
                        pManualChList[num] = (i-8)*2 + 167;
                        num++;
                    }
                }

                DBGPRINT(RT_DEBUG_TRACE, ("num = %d\n",num));

                break;
            default:            // Error. should never happen
                DBGPRINT(RT_DEBUG_WARN,("countryregion=%d not support", pPort->CommonCfg.CountryRegionForABand));
                break;
        }

        if (num != 0)
        {
            if((pPort->CommonCfg.CountryRegionForABand & 0x7f) == REGION_30_A_BAND)
            {
                for (i=0; i<num; i++)
                {
                    for (j=0; j<MAX_NUM_OF_CHANNELS; j++)
                    {
                        if (pManualChList[i] == pAd->TxPower[j].Channel)
                        {
                            DBGPRINT(RT_DEBUG_TRACE, ("pManualChList[%d] = %d\n",i,pManualChList[i]));
                            PlatformMoveMemory(&pAd->HwCfg.ChannelList[index+i], &pAd->TxPower[j], sizeof(CHANNEL_TX_POWER));
                        }
                    }
                }
            }
            else
            {
                // Must care the pChannelList = Null condition, when new add channel list
                if(pChannelList == NULL)
                {
                    DBGPRINT(RT_DEBUG_TRACE, ("[%s %d] Error: pChannelList = NULL\n",__FUNCTION__,__LINE__));
                }
                else
                {
                    for (i = 0; i < num; i++)
                    {
                        for (j = 0; j < MAX_NUM_OF_CHANNELS; j++)
                        {
                            if (pChannelList[i] == pAd->TxPower[j].Channel)
                            {
                                PlatformMoveMemory(&pAd->HwCfg.ChannelList[index+i], &pAd->TxPower[j], sizeof(CHANNEL_TX_POWER));
                            }
                        }
                    }
                }
            }
            index += num;
        }
    }

    //rt2860b emulation set channellist. take away.
    pAd->HwCfg.ChannelListNum = index;
    DBGPRINT(RT_DEBUG_TRACE,("country code=%d/%d, RFIC=%d, PHY mode=%d, support %d channels\n", 
    pPort->CommonCfg.CountryRegion, pPort->CommonCfg.CountryRegionForABand, pAd->HwCfg.RfIcType, pPort->CommonCfg.PhyMode, pAd->HwCfg.ChannelListNum));
#if DBG	
    for (i = 0;i < pAd->HwCfg.ChannelListNum;i++)
    {
        DBGPRINT_RAW(RT_DEBUG_TRACE,(" #%d", pAd->HwCfg.ChannelList[i].Channel));
    }
#endif
}
#endif

/* 
    ==========================================================================
    Description:
        This routine return the first channel number according to the country 
        code selection and RF IC selection (signal band or dual band). It is called
        whenever driver need to start a site survey of all supported channels.
    Return:
        ch - the first channel number of current country code setting

    IRQL = PASSIVE_LEVEL

    ==========================================================================
 */
UCHAR MlmeSyncFirstChannel(
    IN PMP_ADAPTER pAd)
{
    PMP_PORT    pPort = pAd->PortList[FXXK_PORT_0];

#if(COMPILE_WIN8_ABOVE(CURRENT_OS_NDIS_VER))  
    if (NDIS_WIN8_ABOVE(pAd) && (pAd->StaCfg.NLOEntry.NLOEnable == TRUE) && (pAd->StaCfg.NLOEntry.bNLOAfterResume == TRUE))
    {
        DBGPRINT(RT_DEBUG_TRACE, ("%s: Perform NLO after system resume\n", __FUNCTION__));
        
        pPort->CommonCfg.ChannelListIdx = 0;
        return pAd->StaCfg.NLOEntry.NLOHintedChannels[0].Channel;
    }
    else
#endif
    {
        pPort->CommonCfg.ChannelListIdx = 0;
        DBGPRINT(RT_DEBUG_TRACE, ("%s: ch[0]=%d\n", __FUNCTION__, pAd->HwCfg.ChannelList[0].Channel));
        return pAd->HwCfg.ChannelList[0].Channel;
    }
}

/* 
    ==========================================================================
    Description:
        This routine returns the next channel number. This routine is called
        during driver need to start a site survey of all supported channels.
    Return:
        next_channel - the next channel number valid in current country code setting.
    Note:
        return 0 if no more next channel
    ==========================================================================
 */
UCHAR MlmeSyncNextChannel(
    IN PMP_ADAPTER pAd,
    IN PMP_PORT     pPort,
    IN UCHAR channel)
{
    int i;
    UCHAR   CurrentChannel;
    UCHAR   next_channel = 0;
   // PMP_PORT    pPort = pAd->PortList[FXXK_PORT_0];
    
    CurrentChannel = channel;
    

    if ((pAd->MlmeAux.ScanType == FAST_SCAN_ACTIVE) && (pPort->P2PCfg.P2pMsSatetPhase == P2pMs_STATE_PROVISIONING))
    {
        if ((pPort->P2PCfg.P2pProprietary.ListenChanelIndex) < (pPort->P2PCfg.P2pProprietary.ListenChanelCount))
        {
            next_channel = pPort->P2PCfg.P2pProprietary.ListenChanel[pPort->P2PCfg.P2pProprietary.ListenChanelIndex];
        }
        else
        {
            next_channel = 0;
        }
        
        if (pPort->P2PCfg.P2pProprietary.ListenChanelIndex < sizeof(pPort->P2PCfg.P2pProprietary.ListenChanel))
            pPort->P2PCfg.P2pProprietary.ListenChanelIndex++;     

        return next_channel;
    }
    

    if (pAd->MlmeAux.ScanType == SCAN_P2P)
    {
        DBGPRINT(RT_DEBUG_INFO, ("SYNC - P2P Scan  next channel \n"));

        if (pPort->P2PCfg.P2PConnectState == P2P_CONNECT_NOUSE)
        {
        if (IS_P2P_LISTENING(pPort) || IS_P2P_LISTEN_IDLE(pPort))
            return next_channel;
        }
        else
        {
        if (IS_P2P_LISTENING(pPort))
            return next_channel;
        }

        for (i = 0; i < (pPort->P2PCfg.P2pProprietary.ListenChanelCount - 1); i++)
        {
            if (CurrentChannel == pPort->P2PCfg.P2pProprietary.ListenChanel[i])
            {
                next_channel = pPort->P2PCfg.P2pProprietary.ListenChanel[i+1];
                DBGPRINT(RT_DEBUG_TRACE, ("SYNC - P2P Scan [%d] next_channel = %d. . CurrentChannel= %d\n", i, next_channel, CurrentChannel));
                break;
            }
        }

        P2P_INC_CHA_INDEX(pPort->P2PCfg.P2pProprietary.ListenChanelIndex, pPort->P2PCfg.P2pProprietary.ListenChanelCount);
        DBGPRINT(RT_DEBUG_TRACE, ("SYNC - P2P Scan return CurrentChannel = %d. channel = %d. ListenChanelCount = %d\n", CurrentChannel, next_channel, pPort->P2PCfg.P2pProprietary.ListenChanelCount));
        DBGPRINT(RT_DEBUG_INFO, ("SYNC - P2P Scan   pPort->P2PCfg.P2pProprietary.ListenChanelIndex= %d\n", pPort->P2PCfg.P2pProprietary.ListenChanelIndex));

        //
        if (next_channel == CurrentChannel)
        {
            DBGPRINT(RT_DEBUG_TRACE, ("SYNC -  next_channel equals to CurrentChannel = %d\n", next_channel));
            DBGPRINT(RT_DEBUG_TRACE, ("SYNC -  %d  %d  %d\n", pPort->P2PCfg.P2pProprietary.ListenChanel[0], pPort->P2PCfg.P2pProprietary.ListenChanel[1], pPort->P2PCfg.P2pProprietary.ListenChanel[2]));
            
            next_channel = 0;
        }
        return next_channel;
    }
    else  if ((pPort->P2PCfg.P2PConnectState == P2P_DO_GO_SCAN_OP_BEGIN) && (pPort->P2PCfg.GroupOpChannel != 0))
    {
        DBGPRINT(RT_DEBUG_TRACE, ("SYNC - First P2P Scan for Registra = %d. channel = %d\n", CurrentChannel, next_channel));
        return next_channel;
    }
    else  if ((pPort->P2PCfg.P2PConnectState == P2P_DO_GO_NEG_DONE_CLIENT) && (pPort->P2PCfg.GroupOpChannel != 0))
    {
        DBGPRINT(RT_DEBUG_TRACE, ("SYNC - First P2P Scan for Registra = %d. channel = %d\n", CurrentChannel, next_channel));
        return next_channel;
    }

#if(COMPILE_WIN8_ABOVE(CURRENT_OS_NDIS_VER))  
    if (NDIS_WIN8_ABOVE(pAd) && (pAd->StaCfg.NLOEntry.NLOEnable == TRUE) && (pAd->StaCfg.NLOEntry.bNLOAfterResume == TRUE))
    {
        for (i = 0; i < (pAd->HwCfg.ChannelListNum - 1); i++)
        {
            if (CurrentChannel == pAd->StaCfg.NLOEntry.NLOHintedChannels[i].Channel)
            {
                // Only scan 2.4G channel if this is a SCAN_2040_BSS_COEXIST
                if ((pAd->MlmeAux.ScanType == SCAN_2040_BSS_COEXIST) && (pAd->StaCfg.NLOEntry.NLOHintedChannels[i+1].Channel >14))          
                {
                    CurrentChannel = pAd->StaCfg.NLOEntry.NLOHintedChannels[i+1].Channel;
                    continue;
                }
                else
                {
                    next_channel = pAd->StaCfg.NLOEntry.NLOHintedChannels[i+1].Channel;
                    // Record this channel's idx in ChannelList array.
                    pPort->CommonCfg.ChannelListIdx = i+1;
                    break;
                }
            }
        }
    }
    else
#endif
    {
        for (i = 0; i < (pAd->HwCfg.ChannelListNum - 1); i++)
        {
            if (channel == pAd->HwCfg.ChannelList[i].Channel)
            {
                // Only scan 2.4G channel if this is a SCAN_2040_BSS_COEXIST
                if ((pAd->MlmeAux.ScanType == SCAN_2040_BSS_COEXIST) && (pAd->HwCfg.ChannelList[i+1].Channel >14))            
                {
                    continue;
                }
                else
                {
                    next_channel = pAd->HwCfg.ChannelList[i+1].Channel;
                    // Record this channel's idx in ChannelList array.
                    pPort->CommonCfg.ChannelListIdx = i+1;
                    break;
                }
            }
        }
    }
    return next_channel;
}

CHAR    MlmeSyncConvertToRssi(
    IN  PMP_ADAPTER   pAd,
    IN  UCHAR           Rssi,
    IN  UCHAR           RssiNumber,
    IN  UCHAR           AntSel,
    IN  UCHAR           BW)
{

    UCHAR   RssiOffset, LNAGain;
    
    // Rssi equals to zero should be an invalid value
    if (Rssi == 0)
        return -99;

    {
        LNAGain = GET_LNA_GAIN(pAd);
        if (pAd->HwCfg.LatchRfRegs.Channel > 14)
        {
            if (RssiNumber == 0)
                RssiOffset = pAd->HwCfg.ARssiOffset1;
            else if (RssiNumber == 1)
                RssiOffset = pAd->HwCfg.ARssiOffset2;
            else
                RssiOffset = pAd->HwCfg.ARssiOffset3;
        }
        else
        {
            if (RssiNumber == 0)
                RssiOffset = pAd->HwCfg.BGRssiOffset1;
            else if (RssiNumber == 1)
                RssiOffset = pAd->HwCfg.BGRssiOffset2;
            else
                RssiOffset = pAd->HwCfg.BGRssiOffset3;
        }

        return (-12 - RssiOffset - LNAGain - Rssi);

    }
}

VOID MlmeSyncBuildEffectedChannelList(
    IN PMP_ADAPTER pAd)
{
    UCHAR       k;
    for (k = 0;k < pAd->HwCfg.ChannelListNum;k++)
    {
        if (pAd->HwCfg.ChannelList[k].Channel <=14 )
            pAd->HwCfg.ChannelList[k].bEffectedChannel = TRUE;
    }
    return; 
}

VOID MlmeSyncDeleteEffectedChannelList(
    IN PMP_ADAPTER pAd)
{
    UCHAR       i;
    // Clear all bEffectedChannel in ChannelList array.
    for (i = 0; i < pAd->HwCfg.ChannelListNum; i++)
    {
        pAd->HwCfg.ChannelList[i].bEffectedChannel = FALSE;
    }
}

VOID MlmeSyncProbeInactiveStation(
    IN PMP_ADAPTER pAd,
    IN PMP_PORT pPort,
    IN PUCHAR pDA)
{
    NDIS_STATUS     NState;
    PUCHAR          pOutBuffer;
    ULONG           FrameLen = 0;
    HEADER_802_11   Hdr80211;
    UCHAR           SupRateLen = 0;

    DBGPRINT(RT_DEBUG_TRACE, ("MlmeSyncProbeInactiveStation...\n"));

    NState = MlmeAllocateMemory(pAd, &pOutBuffer);  //Get an unused nonpaged memory
    if (NState == NDIS_STATUS_SUCCESS) 
    {
        MgtMacHeaderInit(pAd, pPort, &Hdr80211, SUBTYPE_PROBE_REQ, 0, pDA, pPort->PortCfg.Bssid);

        if (pPort->CommonCfg.Ch14BOnly && (pPort->Channel == CHANNEL_14))
        {
            SupRateLen = 4;
        }
        else
        {
            SupRateLen = pAd->StaActive.SupRateLen;
        }
        // this ProbeRequest explicitly specify SSID to reduce unwanted ProbeResponse
        MakeOutgoingFrame(pOutBuffer,                     &FrameLen,
                          sizeof(HEADER_802_11),          &Hdr80211,
                          1,                              &SsidIe,
                          1,                              &pPort->PortCfg.SsidLen,
                          pPort->PortCfg.SsidLen,         pPort->PortCfg.Ssid,
                          1,                              &SupRateIe,
                          1,                              &SupRateLen,
                          SupRateLen,                     pAd->StaActive.SupRate, 
                          END_OF_ARGS);

        //
        // add IEs for VHT
        //
        if (pPort->Channel>14)
        {
            //
            // add VHT_Cap IE for VHT
            //
            if(pPort->CommonCfg.PhyMode == PHY_11VHT)
            {
                ULONG Tmp;      
                UCHAR VhtCapIeLen = SIZE_OF_VHT_CAP_IE;
                MakeOutgoingFrame(pOutBuffer+FrameLen, &Tmp,
                                    1,                  &VhtCapIe,
                                    1,                  &VhtCapIeLen,
                                    SIZE_OF_VHT_CAP_IE, &pPort->CommonCfg.DesiredVhtPhy.VhtCapability,
                                    END_OF_ARGS);
                FrameLen += Tmp;
            }
        }

        NdisCommonMiniportMMRequest(pAd, pOutBuffer, FrameLen);
    }
}
BOOLEAN MlmeSyncIsValidChannel(
    IN PMP_ADAPTER pAd,
    IN UCHAR channel)

{
    INT i;

    for (i = 0; i < pAd->HwCfg.ChannelListNum; i++)
    {
        if (pAd->HwCfg.ChannelList[i].Channel == channel)
            break;
    }

    DBGPRINT(RT_DEBUG_TRACE, ("MlmeSyncIsValidChannel : i = %d, pAd->HwCfg.ChannelListNum = %d\n", pAd->HwCfg.ChannelListNum));
    
    if (i == pAd->HwCfg.ChannelListNum)
        return FALSE;
    else
        return TRUE;
}

/* For [ADHOC PER ENTRY] */
VOID MlmeSyncCheckBWOffset(
    IN PMP_ADAPTER    pAd,
    IN UCHAR            Channel)
{
    UCHAR Dir;
    BOOLEAN bChangeTo20MHz = FALSE;
    PMP_PORT    pPort = pAd->PortList[FXXK_PORT_0];

    /* (pPort->CommonCfg.DesiredHtPhy.HtChannelWidth) fixed on each active connection by UI */
    /* (pPort->CommonCfg.HtCapability.HtCapInfo.ChannelWidth) changing as the channel selection */
    if (pPort->CommonCfg.PhyMode >= PHY_11ABGN_MIXED)
    {
        if (pPort->CommonCfg.DesiredHtPhy.HtChannelWidth == BW_40)
        {
            if (Channel > 14)
            {
                do 
                {
                    if ((Channel == 36) || (Channel == 44) || (Channel == 52) || (Channel == 60) || (Channel == 100) || (Channel == 108) ||
                        (Channel == 116) || (Channel == 124) || (Channel == 132) || (Channel == 149) || (Channel == 157))
                    {
                        Dir = EXTCHA_ABOVE; // as default
                        if (MlmeSyncIsValidChannel(pAd, Channel + 4))
                            break;
                        
                        Dir = EXTCHA_BELOW;
                        if (MlmeSyncIsValidChannel(pAd, Channel - 4))
                            break;
                    }
                    else if ((Channel == 40) || (Channel == 48) || (Channel == 56) || (Channel == 64) || (Channel == 104) || (Channel == 112) || (Channel == 120) || 
                            (Channel == 128) || (Channel == 136) || (Channel == 153) || (Channel == 161))
                    {
                        Dir = EXTCHA_BELOW; // as default
                        if (MlmeSyncIsValidChannel(pAd, Channel - 4))
                            break;
                        
                        Dir = EXTCHA_ABOVE;
                        if (MlmeSyncIsValidChannel(pAd, Channel + 4))
                            break;
                    }

                    bChangeTo20MHz = TRUE;
                    Dir = EXTCHA_NONE;

                }while(FALSE);
            }
            else
            {
                do
                {   
                    Dir = EXTCHA_ABOVE; // as default
                    if (MlmeSyncIsValidChannel(pAd, Channel + 4))
                        break;

                    Dir = EXTCHA_BELOW;
                    if (MlmeSyncIsValidChannel(pAd, Channel - 4))
                        break;

                    bChangeTo20MHz = TRUE;
                    Dir = EXTCHA_NONE;
                    
                } while(FALSE);

                if ((!pAd->StaCfg.bAdhocN14Ch40MHzAllowed) && (Channel == 14))
                {
                    bChangeTo20MHz = TRUE;
                    Dir = EXTCHA_NONE;
                }
            }
            
            // Update ExtChanOffset
            pPort->CommonCfg.DesiredHtPhy.ExtChanOffset = Dir;
            
            // change bandwidth here  !!! 
            if (bChangeTo20MHz)
            {
                pPort->CommonCfg.HtCapability.HtCapInfo.ChannelWidth = BW_20;
                pPort->CommonCfg.AddHTInfo.AddHtInfo.RecomWidth = BW_20;
                pPort->CommonCfg.AddHTInfo.AddHtInfo.ExtChanOffset = EXTCHA_NONE;
            }
            else
            {
                pPort->CommonCfg.HtCapability.HtCapInfo.ChannelWidth = BW_40;
                pPort->CommonCfg.AddHTInfo.AddHtInfo.RecomWidth = BW_40;
                pPort->CommonCfg.AddHTInfo.AddHtInfo.ExtChanOffset = pPort->CommonCfg.DesiredHtPhy.ExtChanOffset;
            }
        }
        

    }


    DBGPRINT(RT_DEBUG_TRACE,("MlmeSyncCheckBWOffset:: Current Channel=%d, BW=%d, ExtChanOffset=%d\n", 
            Channel, pPort->CommonCfg.HtCapability.HtCapInfo.ChannelWidth, pPort->CommonCfg.DesiredHtPhy.ExtChanOffset));

}

/* For [ADHOC PER ENTRY] */
BOOLEAN MlmeSyncStaAdhocUpdateMacTableEntry(
    IN  PMP_ADAPTER       pAd,
    IN  PMP_PORT          pPort,
    IN  PMAC_TABLE_ENTRY    pEntry,
    IN  UCHAR               MaxSupportedRateIn500Kbps,
    IN  UCHAR               MinSupportedRateIn500Kbps,
    IN  HT_CAPABILITY_IE    *pHtCapability,
    IN  UCHAR               HtCapabilityLen,
    IN  ADD_HT_INFO_IE      *pAddHtInfo,
    IN  UCHAR               AddHtInfoLen,
    IN  USHORT              CapabilityInfo)
{
    UCHAR           MaxSupportedRate = RATE_11;
    UCHAR           MinSupportedRate = RATE_1;
    MP_RW_LOCK_STATE LockState;

    if (!pEntry)
    {
        DBGPRINT(RT_DEBUG_WARN, ("MlmeSyncStaAdhocUpdateMacTableEntry - NULL Entry\n"));
        return FALSE;
    }
    

    if (ADHOC_ON(pPort))
        CLIENT_STATUS_CLEAR_FLAG(pEntry, fCLIENT_STATUS_WMM_CAPABLE);

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

    switch (MinSupportedRateIn500Kbps)
    {
        case 108: MinSupportedRate = RATE_54;   break;
        case 96:  MinSupportedRate = RATE_48;   break;
        case 72:  MinSupportedRate = RATE_36;   break;
        case 48:  MinSupportedRate = RATE_24;   break;
        case 36:  MinSupportedRate = RATE_18;   break;
        case 24:  MinSupportedRate = RATE_12;   break;
        case 18:  MinSupportedRate = RATE_9;    break;
        case 12:  MinSupportedRate = RATE_6;    break;
        case 22:  MinSupportedRate = RATE_11;   break;
        case 11:  MinSupportedRate = RATE_5_5;  break;
        case 4:   MinSupportedRate = RATE_2;    break;
        case 2:   MinSupportedRate = RATE_1;    break;
        default:  MinSupportedRate = RATE_11;   break;
    }


    if((pAd->StaCfg.bAdhocNMode) && (pPort->CommonCfg.PhyMode >= PHY_11ABGN_MIXED))
    {
        // 11n only
        if ((pPort->CommonCfg.PhyMode == PHY_11N) && (HtCapabilityLen == 0))
        {
            DBGPRINT(RT_DEBUG_WARN, ("MlmeSyncStaAdhocUpdateMacTableEntry - No joint rates (Local:N ONLY, Peer:Legacy Mode) \n"));
            return FALSE;
        }
    }
    else if ((pPort->Channel > 14) && (MaxSupportedRate < RATE_FIRST_OFDM_RATE))
    {
        DBGPRINT(RT_DEBUG_WARN, ("MlmeSyncStaAdhocUpdateMacTableEntry - No joint rates (Local:G ONLY/A ONLY, Peer:B ONLY) \n"));
        return FALSE;
    }


    PlatformAcquireOldRWLockForRead(&pAd->MacTablePool.MacTabPoolLock, &LockState);
    if (pEntry)
    {
        if ((MaxSupportedRate < RATE_FIRST_OFDM_RATE) ||
            ((pAd->StaCfg.AdhocMode == ADHOC_11B) && (pPort->Channel <= 14)))
        {
            pEntry->RateLen = 4;
            if (MaxSupportedRate >= RATE_FIRST_OFDM_RATE)
                MaxSupportedRate = RATE_11;
        }
        else if ((MinSupportedRate >= RATE_6) && (MaxSupportedRate <= RATE_54))
            pEntry->RateLen = 8;
        else
            pEntry->RateLen = 12;

        WRITE_PHY_CFG_DOUBLE_WORD(pAd, &pEntry->MaxPhyCfg, 0);
        WRITE_PHY_CFG_DOUBLE_WORD(pAd, &pEntry->MinPhyCfg, 0);
        WRITE_PHY_CFG_DOUBLE_WORD(pAd, &pEntry->TxPhyCfg, 0);
        pEntry->MaxSupportedRate = MaxSupportedRate;

        // CCK Rate
        if (pEntry->MaxSupportedRate < RATE_FIRST_OFDM_RATE)
        {
            WRITE_PHY_CFG_MODE(pAd, &pEntry->MaxPhyCfg, MODE_CCK);
            WRITE_PHY_CFG_MCS(pAd, &pEntry->MaxPhyCfg, pEntry->MaxSupportedRate);
            WRITE_PHY_CFG_MODE(pAd, &pEntry->MinPhyCfg, MODE_CCK);
            WRITE_PHY_CFG_MCS(pAd, &pEntry->MinPhyCfg, MinSupportedRate);
        }
        else // OFDM Rate
        {
            WRITE_PHY_CFG_MODE(pAd, &pEntry->MaxPhyCfg, MODE_OFDM);
            WRITE_PHY_CFG_MCS(pAd, &pEntry->MaxPhyCfg, OfdmRateToRxwiMCS[pEntry->MaxSupportedRate]);
            WRITE_PHY_CFG_MODE(pAd, &pEntry->MinPhyCfg, MODE_OFDM);
            WRITE_PHY_CFG_MCS(pAd, &pEntry->MinPhyCfg, OfdmRateToRxwiMCS[MinSupportedRate]);
        }
        
        pEntry->CapabilityInfo = CapabilityInfo;
        CLIENT_STATUS_CLEAR_FLAG(pEntry, fCLIENT_STATUS_AGGREGATION_CAPABLE);
        CLIENT_STATUS_CLEAR_FLAG(pEntry, fCLIENT_STATUS_PIGGYBACK_CAPABLE);

    }

    // Support 802.11N
    if((pAd->StaCfg.bAdhocNMode) && (pPort->CommonCfg.PhyMode >= PHY_11ABGN_MIXED))
    {
        PlatformZeroMemory(&pEntry->HTCapability, sizeof(pEntry->HTCapability));
        // If this Entry supports 802.11n, upgrade to HT rate. 
        if ((HtCapabilityLen != 0) && (pPort->CommonCfg.PhyMode >= PHY_11ABGN_MIXED))
        {
            UCHAR   j, bitmask; //k,bitmask;
            CHAR    i;

            if (ADHOC_ON(pPort))
                CLIENT_STATUS_SET_FLAG(pEntry, fCLIENT_STATUS_WMM_CAPABLE);
            
            // Always use HTMIX mode
            WRITE_PHY_CFG_MODE(pAd, &pEntry->MaxPhyCfg, MODE_HTMIX);


            // Always set my Central Channel to BBP under BW=40MHZ.
            // But data packet shall downgrade to control channel if one peer is at upper and the other is at lower channel.
            if ((pHtCapability->HtCapInfo.ChannelWidth) && 
                (pPort->CommonCfg.DesiredHtPhy.HtChannelWidth) &&
                (pAddHtInfo->AddHtInfo.ExtChanOffset == pPort->CommonCfg.AddHTInfo.AddHtInfo.ExtChanOffset))
            {
                WRITE_PHY_CFG_BW(pAd, &pEntry->MaxPhyCfg, BW_40);
                WRITE_PHY_CFG_SHORT_GI(pAd, &pEntry->MaxPhyCfg, (pPort->CommonCfg.DesiredHtPhy.ShortGIfor40 & pHtCapability->HtCapInfo.ShortGIfor40));
            }
            else
            {   
                WRITE_PHY_CFG_BW(pAd, &pEntry->MaxPhyCfg, BW_20);
                WRITE_PHY_CFG_SHORT_GI(pAd, &pEntry->MaxPhyCfg, (pPort->CommonCfg.DesiredHtPhy.ShortGIfor20 & pHtCapability->HtCapInfo.ShortGIfor20));
            }

            // find max fixed rate
            for (i=23; i>=0; i--) // 3*3
            {   
                j = i/8;    
                bitmask = (1<<(i-(j*8)));
                if ((pPort->CommonCfg.DesiredHtPhy.MCSSet[j] & bitmask) && (pHtCapability->MCSSet[j] & bitmask))
                {
                    WRITE_PHY_CFG_MCS(pAd, &pEntry->MaxPhyCfg, i);
                    break;
                }
                if (i==0)
                    break;
            }


            if ((pPort->CommonCfg.RegTransmitSetting.field.HTMODE <= HTMODE_GF) && (pPort->CommonCfg.RegTransmitSetting.field.MCS != MCS_AUTO))
            {
                // STA supports fixed MCS only for HT mode
                WRITE_PHY_CFG_MODE(pAd, &pEntry->MaxPhyCfg, MODE_HTMIX);
                WRITE_PHY_CFG_MCS(pAd, &pEntry->MaxPhyCfg, pPort->CommonCfg.RegTransmitSetting.field.MCS);
                WRITE_PHY_CFG_BW(pAd, &pEntry->MaxPhyCfg, pPort->CommonCfg.RegTransmitSetting.field.BW);
                WRITE_PHY_CFG_STBC(pAd, &pEntry->MaxPhyCfg, pPort->CommonCfg.RegTransmitSetting.field.STBC);
                WRITE_PHY_CFG_SHORT_GI(pAd, &pEntry->MaxPhyCfg, pPort->CommonCfg.RegTransmitSetting.field.ShortGI);

                DBGPRINT(RT_DEBUG_TRACE, ("MlmeSyncStaAdhocUpdateMacTableEntry - Support fixed MCS (MODE=%d, MCS=%d, BW=%d, STBC=%d, SGI=%d) \n",
                    READ_PHY_CFG_MODE(pAd, &pEntry->MaxPhyCfg), 
                    READ_PHY_CFG_MCS(pAd, &pEntry->MaxPhyCfg), 
                    READ_PHY_CFG_BW(pAd, &pEntry->MaxPhyCfg), 
                    READ_PHY_CFG_STBC(pAd, &pEntry->MaxPhyCfg), 
                    READ_PHY_CFG_SHORT_GI(pAd, &pEntry->MaxPhyCfg)));

            }
            WRITE_PHY_CFG_STBC(pAd, &pEntry->MaxPhyCfg, (pHtCapability->HtCapInfo.RxSTBC & pPort->CommonCfg.DesiredHtPhy.TxSTBC));
            
            pEntry->MpduDensity = pHtCapability->HtCapParm.MpduDensity;
            pEntry->MaxRAmpduFactor = pHtCapability->HtCapParm.MaxRAmpduFactor;
            pEntry->MmpsMode = (UCHAR)pHtCapability->HtCapInfo.MimoPs;
            pEntry->AMsduSize = (UCHAR)pHtCapability->HtCapInfo.AMsduSize;              
            
            WRITE_PHY_CFG_DOUBLE_WORD(pAd, &pEntry->TxPhyCfg, READ_PHY_CFG_DOUBLE_WORD(pAd, &pEntry->MaxPhyCfg));

            if (pPort->CommonCfg.DesiredHtPhy.AmsduEnable && (pPort->CommonCfg.REGBACapability.field.AutoBA == FALSE))
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
                CLIENT_STATUS_SET_FLAG(pEntry, fCLIENT_STATUS_RDG_CAPABLE); 
            PlatformMoveMemory(&pEntry->HTCapability, pHtCapability, HtCapabilityLen);
        }
        
    }

    WRITE_PHY_CFG_DOUBLE_WORD(pAd, &pEntry->TxPhyCfg, READ_PHY_CFG_DOUBLE_WORD(pAd, &pEntry->MaxPhyCfg));
    
    pEntry->CurrTxRate = pEntry->MaxSupportedRate;

    
    // Set asic auto fall back if AutoTxRateSwitch is TRUE
    if (pPort->CommonCfg.bAutoTxRateSwitch == TRUE)
    {
        PUCHAR                  pTable;
        UCHAR                   TableSize = 0;
        
        MlmeSelectTxRateTable(pAd, pEntry, &pTable, &TableSize, &pEntry->CurrTxRateIndex);
    }

        
    pEntry->Sst = SST_ASSOC;
    pEntry->AuthState = AS_AUTH_OPEN;

    PlatformReleaseOldRWLock(&pAd->MacTablePool.MacTabPoolLock, &LockState);

    DBGPRINT(RT_DEBUG_INFO, ("MlmeSyncStaAdhocUpdateMacTableEntry - Aid=%d, RateLen=%d, MODE= %d, BW=%d, MCS=%d, GI=%d, STBC=%d, ExtChanOffset=%d\n",
        pEntry->Aid,
        pEntry->RateLen,
        READ_PHY_CFG_MODE(pAd, &pEntry->TxPhyCfg), 
        READ_PHY_CFG_BW(pAd, &pEntry->TxPhyCfg), 
        READ_PHY_CFG_MCS(pAd, &pEntry->TxPhyCfg), 
        READ_PHY_CFG_SHORT_GI(pAd, &pEntry->TxPhyCfg), 
        READ_PHY_CFG_STBC(pAd, &pEntry->TxPhyCfg),
        pAddHtInfo->AddHtInfo.ExtChanOffset));

    return TRUE;

}

#ifdef MULTI_CHANNEL_SUPPORT
VOID MlmeSyncForceToTriggerScanComplete(
    IN  PMP_ADAPTER   pAd)
{
    UCHAR           i = 0;
    PMP_PORT      pPort = NULL;
    //USHORT            Status;
    
    for (i = 0; i < RTMP_MAX_NUMBER_OF_PORT/*pAd->NumberOfPorts*/; i++)
    {
        if ((pAd->PortList[i] == NULL) || (pAd->PortList[i]->bActive == FALSE))
        {
            continue;
        }

        if (pAd->PortList[i]->PortType == EXTSTA_PORT/*WFD_DEVICE_PORT*/)
        {
            pPort = pAd->PortList[i];
            break;
        }
    }

    if(pPort == NULL)
    {
        DBGPRINT(RT_DEBUG_ERROR, ("!!!!! %s No Device pPort !!!!!\n", __FUNCTION__));
        return;
    }
    
    //DBGPRINT(RT_DEBUG_WARN, ("%s - **************** Scan Complete **** StaCfg.bFastUpdateScanTab =%d\n", __FUNCTION__, pAd->StaCfg.bFastUpdateScanTab));

#if 0
    if ((pPort->PortCfg.AdditionalRequestIEData != NULL) && (pPort->PortCfg.AdditionalRequestIESize > 0))
    {
        DBGPRINT(RT_DEBUG_TRACE,("Clear AdditionalIEData buffer!!\n"));
        PlatformFreeMemory(pPort->PortCfg.AdditionalRequestIEData, pPort->PortCfg.AdditionalRequestIESize, 0);
        pPort->PortCfg.AdditionalRequestIEData = NULL;
        pPort->PortCfg.AdditionalRequestIESize = 0;
    }

    pAd->StaCfg.BssNr = pAd->ScanTab.BssNr;
    pAd->StaCfg.bFastRoamingScan = FALSE;   // reset this which was set by FastRoaming
    pAd->StaCfg.bImprovedScan = FALSE;
    // used to check if driver has to reconnect immediatelly
    pAd->StaCfg.bFastUpdateScanTab = FALSE; // reset when scan complete

    pPort->Mlme.SyncMachine.CurrState = SYNC_IDLE;

    Status = MLME_SUCCESS;
    MlmeEnqueue(pAd, pPort, MLME_CNTL_STATE_MACHINE, MT2_SCAN_CONF, 2, &Status);    
#else
    pPort->Mlme.SyncMachine.CurrState = SYNC_IDLE;
    pPort->P2PCfg.P2PDiscoProvState = P2P_ENABLE_LISTEN_ONLY;
    XmitResumeMsduTransmission(pAd);
    DBGPRINT(RT_DEBUG_TRACE, ("%s(line:%d)Port(%d) Mlme.SyncMachine.CurrState(%d)\n", __FUNCTION__, __LINE__, pPort->PortNumber, pPort->Mlme.SyncMachine.CurrState));
#endif
}
#endif /*MULTI_CHANNEL_SUPPORT*/

UCHAR
MlmeSyncGetWlanIdxByPort(
    IN  PMP_PORT            pPort,
    IN USHORT  WlanIdxType
    )
{
    PMAC_TABLE_ENTRY  pEntry = MlmeSyncMacTabMatchedRoleType(pPort, WlanIdxType); 

    if(pEntry != NULL)
    {
        return pEntry->wcid;
    }
    
    return 0xff;
        }
        

USHORT
MlmeSyncGetRoleTypeByWlanIdx(
    IN PMP_PORT pPort,
    IN UCHAR    WlanIdx
    )
{  
    PMAC_TABLE_ENTRY  pEntry = MlmeSyncMacTabMatchedWlanIdx(pPort, WlanIdx);

    if(pEntry != NULL)
    {
        return pEntry->WlanIdxRole;
    }
    
    return ROLE_WLANIDX_UNKNOW;
}

VOID
MlmeSyncInitMacTabPool(
    IN PMP_ADAPTER    pAd
    )
{
    UCHAR   i=0;
    PMAC_TABLE_ENTRY  pEntry =NULL;
    MP_RW_LOCK_STATE LockState;
    
    InitializeQueueHeader(&pAd->MacTablePool.MacTabPoolList);
    //InitializeQueueHeader(&pAd->MacTablePool.MacTabPoolArray.McastPsQueue);
    pAd->MacTablePool.MacTabPoolListSize = 0;
    
   
    
    for(i=0;i <MAX_LEN_OF_MAC_TABLE;i++)
    {
        PlatformZeroMemory(&pAd->MacTabPoolArray[i], sizeof(MAC_TABLE_ENTRY));
          
        PlatformAcquireOldRWLockForWrite(&pAd->MacTablePool.MacTabPoolLock,&LockState);
        pEntry = &pAd->MacTabPoolArray[i];
        pEntry->WlanIdxRole = (USHORT)ROLE_WLANIDX_UNKNOW;
        pEntry->ValidAsCLI = FALSE;
        pEntry->ValidAsWDS = FALSE;
        pEntry->bIAmBadAtheros = FALSE;   
        pEntry->wcid = i;
        pEntry->Aid = i;
        pEntry->pAd = pAd;
        pEntry->pPort = pAd->PortList[0];
         //Reserve 0 for Port 0 MBCAST use.
        if(i>0)
        {
        InsertTailQueue(&pAd->MacTablePool.MacTabPoolList, pEntry);     
        pAd->MacTablePool.MacTabPoolListSize++;
        }
        
        PlatformReleaseOldRWLock(&pAd->MacTablePool.MacTabPoolLock,&LockState);   
    }

}

//Not Used
PMAC_TABLE_ENTRY
MlmeSyncInsertMacTabByRoleType(
    IN PMP_PORT         pPort,
    IN UCHAR  WlanIdxType
    )
{
    PMP_ADAPTER pAd = pPort->pAd;
    PMAC_TABLE_ENTRY  pEntry =NULL;
    UCHAR WlanIdx;
    MP_RW_LOCK_STATE LockState;
    
    PlatformAcquireOldRWLockForWrite(&pAd->MacTablePool.MacTabPoolLock,&LockState);
    WlanIdx = (UCHAR)MlmeSyncGetWlanIdxByPort(pPort, WlanIdxType);
    pEntry = (PMAC_TABLE_ENTRY)&(pAd->MacTabPoolArray[WlanIdx]);    //Because the specific Type are not in the linking list.  
    pEntry->WlanIdxRole = WlanIdxType;
    pEntry->wcid = WlanIdx;
    pEntry->pPort = pPort;
    InsertTailQueue(&pPort->MacTab.MacTabList, pEntry); 

    DBGPRINT(RT_DEBUG_TRACE, ("%s   port[%d] WlanIdxType = %d wcid = %d\n", __FUNCTION__, pPort->PortNumber, WlanIdxType, WlanIdx));
    PlatformReleaseOldRWLock(&pAd->MacTablePool.MacTabPoolLock,&LockState);   

    return pEntry;
}

PMAC_TABLE_ENTRY
MlmeSyncInsertMacTabByWlanIdx(
    IN PMP_PORT         pPort,
    IN UCHAR            WlanIdx
    )
{
    PMP_ADAPTER pAd = pPort->pAd;
    PMAC_TABLE_ENTRY  pEntry =NULL;
    MP_RW_LOCK_STATE LockState;
    PMAC_TABLE_ENTRY  pCurrentEntry =NULL, pNextList = NULL;
    PQUEUE_HEADER pHeader;    
    PlatformAcquireOldRWLockForWrite(&pAd->MacTablePool.MacTabPoolLock,&LockState);
    pHeader = &pAd->MacTablePool.MacTabPoolList;
    pNextList = (PMAC_TABLE_ENTRY)(pHeader->Head);
    while (pNextList != NULL)
    {
        pCurrentEntry = (PMAC_TABLE_ENTRY)pNextList;
        
        if( pCurrentEntry->wcid == WlanIdx)
        {           
            break;
        }
        pCurrentEntry = pNextList->Next;   
        pCurrentEntry = NULL;
    }

    if(pCurrentEntry != NULL)
    {
        pEntry = pCurrentEntry;    
    
        InsertTailQueue(&pPort->MacTab.MacTabList, pEntry);        
    }
    else
    {
        DBGPRINT(RT_DEBUG_ERROR, ("%s  the entry was not found !!!!\n", __FUNCTION__));
    }

    PlatformReleaseOldRWLock(&pAd->MacTablePool.MacTabPoolLock,&LockState);  

    DBGPRINT(RT_DEBUG_ERROR, ("%s  the entry was not found !!!!%d\n", __FUNCTION__,  pEntry->wcid));
    return pEntry;
}

//Done
VOID
MlmeSyncRemoveMacTab(
    IN PMP_PORT         pPort,
    IN PMAC_TABLE_ENTRY  pEntry
    )
{
    PMP_ADAPTER pAd = pPort->pAd;
    PMAC_TABLE_ENTRY  pCurrentEntry =NULL, pNextList = NULL;
    QUEUE_HEADER   TmpMacTabList;
    FUNC_ENTER;
    InitializeQueueHeader(&TmpMacTabList);

    pNextList = (PMAC_TABLE_ENTRY)RemoveHeadQueue(&pPort->MacTab.MacTabList);
    while (pNextList != NULL)
    {
        pCurrentEntry = (PMAC_TABLE_ENTRY)pNextList;
        
        if(PlatformEqualMemory(pEntry, pCurrentEntry, sizeof(MAC_TABLE_ENTRY)))
        {
            if((pCurrentEntry->WlanIdxRole == ROLE_WLANIDX_MBCAST) && (pPort->PortNumber ==0))
            {
                InsertTailQueue(&TmpMacTabList, pCurrentEntry);
            }
            else
            {
                InsertTailQueue(&pAd->MacTablePool.MacTabPoolList, pCurrentEntry);  
            }
            break;
        }
        InsertTailQueue(&TmpMacTabList, pCurrentEntry);
        pNextList = (PMAC_TABLE_ENTRY)RemoveHeadQueue(&pPort->MacTab.MacTabList);
    }

    pNextList = (PMAC_TABLE_ENTRY)RemoveHeadQueue(&TmpMacTabList);
    while (pNextList != NULL)
    {
        InsertTailQueue(&pPort->MacTab.MacTabList, pNextList);
        pNextList = (PMAC_TABLE_ENTRY)RemoveHeadQueue(&TmpMacTabList);
    }
    
    FUNC_LEAVE;
}

//Done
VOID
MlmeSyncInitMacTab(
    IN PMP_PORT         pPort
    )
{
    PMAC_TABLE_ENTRY  pEntry =NULL;
    UCHAR  WlanIdx = 0;
     FUNC_ENTER;
    InitializeQueueHeader(&pPort->MacTab.MacTabList);
    
    if(pPort->PortNumber ==0)
    {
        pEntry = (PMAC_TABLE_ENTRY)&(pPort->pAd->MacTabPoolArray[WLANIDX_MBCAST_PORT0]); 
        pEntry->WlanIdxRole = ROLE_WLANIDX_MBCAST;
        pEntry->pPort= pPort;
        pEntry->wcid= WLANIDX_MBCAST_PORT0;
        InsertTailQueue(&pPort->MacTab.MacTabList, pEntry);

        pEntry = (PMAC_TABLE_ENTRY)RemoveHeadQueue(&pPort->pAd->MacTablePool.MacTabPoolList); 
        pEntry->WlanIdxRole = ROLE_WLANIDX_BSSID;
        InsertTailQueue(&pPort->MacTab.MacTabList, pEntry);
        
    }
    else
    {
        pEntry = (PMAC_TABLE_ENTRY)RemoveHeadQueue(&pPort->pAd->MacTablePool.MacTabPoolList); 
        pEntry->WlanIdxRole = ROLE_WLANIDX_MBCAST;
        pEntry->pPort= pPort;
        InsertTailQueue(&pPort->MacTab.MacTabList, pEntry);
    }

    MlmeSyncMacTableDumpInfo(pPort);
    FUNC_LEAVE;
}


//Not Used.
VOID
MlmeSyncDeInitMacTab(
    IN PMP_PORT         pPort
    )
{
    PMP_ADAPTER pAd = pPort->pAd;
    PMAC_TABLE_ENTRY  pEntry =NULL;
    FUNC_ENTER;
    while (IsQueueNotEmpty(&pPort->MacTab.MacTabList))
    {
        pEntry = (PMAC_TABLE_ENTRY)RemoveHeadQueue(&pPort->MacTab.MacTabList);

        if(pEntry->WlanIdxRole == ROLE_WLANIDX_CLIENT)
        {
            InsertTailQueue(&pAd->MacTablePool.MacTabPoolList, pEntry);
        }
    } 
    FUNC_LEAVE;
}

PMAC_TABLE_ENTRY
MlmeSyncMacTabMatchedRoleType(
    IN PMP_PORT         pPort,
    IN USHORT  WlanIdxType
    )
{
    PMP_ADAPTER pAd = pPort->pAd;
    PMAC_TABLE_ENTRY  pNextList =NULL;
    PMAC_TABLE_ENTRY  pEntry = NULL;
    PQUEUE_HEADER pHeader;

    pHeader = &pPort->MacTab.MacTabList;
    pNextList = (PMAC_TABLE_ENTRY)(pHeader->Head);
    while (pNextList != NULL)
    {
        pEntry = (PMAC_TABLE_ENTRY)pNextList;
        
        if(pEntry->WlanIdxRole == WlanIdxType)
        {
            return pEntry;
        }
        else
        {
            pEntry = NULL;
        }
        pNextList = pNextList->Next;   
    }

    return NULL;
}

PMAC_TABLE_ENTRY
MlmeSyncMacTabMatchedBssid(
    IN PMP_PORT         pPort,
    IN PUCHAR  pBssid
    )
{
    PMP_ADAPTER pAd = pPort->pAd;
    PMAC_TABLE_ENTRY  pNextList =NULL;
    PMAC_TABLE_ENTRY  pEntry = NULL;
    BOOLEAN bFind =FALSE;
    PQUEUE_HEADER pHeader;

    pHeader = &pPort->MacTab.MacTabList;
    pNextList = (PMAC_TABLE_ENTRY)(pHeader->Head);
    while (pNextList != NULL)
    {
        
        pEntry = (PMAC_TABLE_ENTRY)pNextList;
        
        if(PlatformEqualMemory(pEntry->Addr, pBssid, 6))
        {
            bFind = TRUE;
            break;
        }
        pNextList = pNextList->Next;   
        pEntry = NULL;
    }
    return pEntry;
}

PMAC_TABLE_ENTRY
MlmeSyncMacTabMatchedWlanIdx(
    IN PMP_PORT         pPort,
    IN UCHAR  Wcid
    )
{
    PMP_ADAPTER pAd = pPort->pAd;
    PMAC_TABLE_ENTRY  pNextList =NULL;
    PMAC_TABLE_ENTRY  pEntry = NULL;
    BOOLEAN bFind =FALSE;
    PQUEUE_HEADER pHeader;
    pHeader = &pPort->MacTab.MacTabList;
    pNextList = (PMAC_TABLE_ENTRY)(pHeader->Head);
    while (pNextList != NULL)
    {
        pEntry = (PMAC_TABLE_ENTRY)pNextList;
        
        if(pEntry->wcid ==Wcid)
        {
            return pEntry;
        }
        pNextList = pNextList->Next;   
        
        pEntry = NULL;
    }
    return NULL;
}

VOID
MlmeSyncMacTableDumpInfo(
    IN PMP_PORT         pPort
    )
{
    PMP_ADAPTER pAd = pPort->pAd;
    PMAC_TABLE_ENTRY  pNextList =NULL;
    PMAC_TABLE_ENTRY  pEntry = NULL;
    PQUEUE_HEADER pHeader;
    DBGPRINT(RT_DEBUG_TRACE, ("Dump Start ========\n"));
    pHeader = &pPort->MacTab.MacTabList;
    pNextList = (PMAC_TABLE_ENTRY)(pHeader->Head);
    while (pNextList != NULL)
    {
        pEntry = (PMAC_TABLE_ENTRY)pNextList;
        
        DBGPRINT(RT_DEBUG_TRACE, ("pPort PortNum = %d, pEntry %p  pEntry->Next %p ,Wcid = %d, Role = %d, Entry PortNum = %d\n", pPort->PortNumber, pEntry , pEntry->Next, pEntry->wcid, pEntry->WlanIdxRole, pEntry->pPort->PortNumber));
        pNextList = pNextList->Next;   
    }  
    DBGPRINT(RT_DEBUG_TRACE, ("Dump End   ========\n"));
}