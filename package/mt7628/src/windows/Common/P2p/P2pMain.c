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
    Jan Lee         2009-10-05    created for Peer-to-Peer(Wifi Direct)
*/
#include "MtConfig.h"

#define DRIVER_FILE         0x00E00000

// Vendor Specific OUI for P2P defined by WFA.
UCHAR   P2POUIBYTE[4] = {0x50, 0x6f, 0x9a, 0x9};
extern UCHAR ZERO_MAC_ADDR[];
UCHAR   WILDP2PSSID[7] = {'D', 'I', 'R', 'E', 'C', 'T','-'};
UCHAR   WILDP2PSSIDLEN = 7;
UCHAR   WIFIDIRECT_OUI[] = {0x50, 0x6f, 0x9a, 0x09};
//UCHAR DEFAULTWPAPSKEY[8] = {0x33, 0x35, 0x33, 0x34, 0x33, 0x36, 0x31, 0x34};
// Like to key in 12345678
UCHAR   DEFAULTWPAPSKEY[8] = {0x31, 0x32, 0x33, 0x34, 0x35, 0x36, 0x37, 0x38};
//UCHAR   P2PCLIWCID[NUM_OF_CONCURRENT_CLIENT_ROLE] = {WLANINDEX_P2P_CLIENT_0, WLANINDEX_P2P_CLIENT_1};
#ifdef MULTI_CHANNEL_SUPPORT
PMP_PORT  pPortForTimer = NULL;
#endif /*MULTI_CHANNEL_SUPPORT*/

/*  
    ==========================================================================
    Description: 
        Called once when the card is being initialized.
        
    Parameters: 

    Note:

    ==========================================================================
 */
VOID        RTMPInitP2P(
    IN PMP_ADAPTER pAd) 
{
    PMP_PORT    pPort = pAd->PortList[FXXK_PORT_0];
    
    //////////////////////////////////////////////////////////////////////////
    // [Microsoft P2P]
    // OS: Win8 and later OS
    // PortType: WFD_DEVICE_PORT/WFD_CLIENT_PORT/WFD_GO_PORT
    // PortNumber: Port1, Port2, Port3 ... etc
    // OpMode: OPMODE_STA
    // PortSubType:
    //      Set PORTSUBTYPE_STA as init subtype
    //      When GO starts to send beacon, set as PORTSUBTYPE_P2PGO
    //      When client role port is created, set as PORTSUBTYPE_P2PClient
    //
    // [Ralink P2P]
    // OS: XP, Vista, Win7, Win8 and later OS   
    // PortType: EXTSTA_PORT
    // PortNumber: Port0 (XP/Vista/Sigma), Port2 (Win7), Port-non-zero(Win8)
    // OpMode: OPMODE_STA(XP/Vista/Sigma), OPMODE_STAP2P(Win7/Win8)
    // PortSubType:
    //      Set PORTSUBTYPE_P2PClient as init subtype
    //      When GO starts to send beacon, set as PORTSUBTYPE_P2PGO
    //////////////////////////////////////////////////////////////////////////

    pPort->P2PCfg.P2pEvent = NULL;
    pPort->P2PCfg.bKeepSlient = FALSE;
    pPort->P2PCfg.NoAIndex = MAX_LEN_OF_MAC_TABLE;
    pPort->P2PCfg.PortNumber = PORT_0;
    pPort->P2PCfg.ClientPortNumber = PORT_0;
    pPort->P2PCfg.Client2PortNumber = PORT_0;
    pPort->P2PCfg.GOPortNumber = PORT_0;

    pPort->P2PCfg.ListenChannel   = P2P_DEFAULT_LISTEN_CHANNEL;
    pPort->P2PCfg.GroupChannel    = P2P_DEFAULT_LISTEN_CHANNEL;
    pPort->P2PCfg.GroupOpChannel  = P2P_DEFAULT_LISTEN_CHANNEL;
    P2pSetListenIntBias(pAd, 3);
    pPort->P2PCfg.DeviceNameLen = 10;
    pPort->P2PCfg.DeviceName[0] = 'R';
    pPort->P2PCfg.DeviceName[1] = 'a';
    pPort->P2PCfg.DeviceName[2] = 'l';
    pPort->P2PCfg.DeviceName[3] = 'i';
    pPort->P2PCfg.DeviceName[4] = 'n';
    pPort->P2PCfg.DeviceName[5] = 'k';
    pPort->P2PCfg.DeviceName[6] = '-';
    pPort->P2PCfg.DeviceName[7] = 'P';
    pPort->P2PCfg.DeviceName[8] = 0x32;
    pPort->P2PCfg.DeviceName[9] = 'P';
    pPort->P2PCfg.ServiceTransac = 0;
    pPort->P2PCfg.MyGOwcid = (UCHAR)MlmeSyncGetWlanIdxByPort(pPort, ROLE_WLANIDX_BSSID);
    // state/status
    pPort->P2PCfg.P2PDiscoProvState = P2P_DISABLE;
    pPort->P2PCfg.P2PConnectState = P2P_CONNECT_NOUSE;
    pPort->P2PCfg.P2PChannelState = P2P_DIFF_CHANNEL_OFF;
    pPort->P2PCfg.P2pMsSatetPhase = P2pMs_STATE_NOUSE;
    pAd->pP2pCtrll->P2pMsConnectedStatus = P2pMs_CONNECTED_STATUS_OFF;
    // Set Dpid to "not specified". it means, GUI doesn't set for connection yet.
    pPort->P2PCfg.Dpid = DEV_PASS_ID_NOSPEC;
    pPort->P2PCfg.P2pManagedParm.APP2pManageability = 0xff;
    pPort->P2PCfg.P2pManagedParm.ICSStatus = ICS_STATUS_DISABLED; 
    pPort->P2PCfg.P2pCounter.ScanIntervalBias = P2P_RANDOM_BASE;
    pPort->P2PCfg.P2pCounter.ScanIntervalBias2 = 0;
    //P2pEdcaDefault(pAd);
    P2pGroupTabInit(pAd); // Ralink P2P
    P2pDiscoTabInit(pAd); // MS P2P
    P2pCrednTabClean(pAd);
    P2pScanChannelDefault(pAd, pPort);
    PlatformMoveMemory(pPort->P2PCfg.SSID, WILDP2PSSID, WILDP2PSSIDLEN);
    PlatformMoveMemory(pPort->P2PCfg.Bssid, pAd->PortList[PORT_0]->CurrentAddress, MAC_ADDR_LEN);
    PlatformZeroMemory(pPort->P2PCfg.GroupDevAddr, MAC_ADDR_LEN);
    pPort->P2PCfg.SSIDLen = WILDP2PSSIDLEN;
    pPort->P2PCfg.GONoASchedule.bValid = FALSE;
    pPort->P2PCfg.GONoASchedule.bInAwake = TRUE;
    pPort->P2PCfg.GONoASchedule.bWMMPSInAbsent = FALSE; // Set to FALSE if changes state to Awake
    pPort->P2PCfg.GONoASchedule.Token = 0;
    pPort->P2PCfg.GoIntentIdx = (UCHAR)pPort->CommonCfg.P2pControl.field.DefaultIntentIndex;
    pPort->P2PCfg.P2pGOAllPsm = FALSE;
    pPort->P2PCfg.bOppsOn = FALSE;
    pPort->P2PCfg.bNoAOn = FALSE;
    pPort->P2PCfg.bP2pAckReq = FALSE;
    pPort->P2PCfg.bPendingNullFrame = FALSE;
    pPort->P2PCfg.bSigmaFakePkt = FALSE;
    pPort->P2PCfg.CentralChannel = 0;// not used
    pPort->P2PCfg.P2pPhyMode = P2P_PHYMODE_ENABLE_11N_20;
    pPort->P2PCfg.ScanPeriod  = P2P_SCAN_PERIOD;
    pPort->P2PCfg.DiscoverType = P2P_DISCO_TYPE_AUTO;
    pPort->P2PCfg.ScanType = P2P_SCAN_TYPE_AUTO;
    pPort->P2PCfg.bGroupsScanOnce = FALSE;
    pPort->P2PCfg.bGOStart = FALSE;
    pPort->P2PCfg.LockNego = FALSE;
    // initial wpa2pskcounter100ms value for allowing site survey when initialize adapter
    pPort->P2PCfg.P2pCounter.Wpa2pskCounter100ms = 300;
    pPort->P2PCfg.P2pCounter.CounterAfterConnectCommand = 300;

    // Init Sigma Parameters
    P2PInitSigmaParam(pAd, pPort);

    //
    // Init Microsoft P2P
    //
#if(COMPILE_WIN8_ABOVE(CURRENT_OS_NDIS_VER))  

#ifdef MULTI_CHANNEL_SUPPORT
    pAd->MccCfg.MultiChannelEnable = FALSE;
    pAd->pP2pCtrll->MultiChannelStartOrEndProcess = 0;
    pAd->MccCfg.SwQSelect = NUM_OF_TX_RING;//QID_AC_BE;
    pAd->MccCfg.TxOp = IFS_HTTXOP;
    
    // 40M
    // Central channel
    pPort->P2PCfg.InfraCentralChannel = CHANNEL_OFF;
    pPort->P2PCfg.P2PCentralChannel = CHANNEL_OFF;
    pPort->P2PCfg.P2PGOCentralChannel = CHANNEL_OFF;
    // Seconday channel offset (1: above primary ch, 3: below primary ch, 0xff: deault)
    pPort->P2PCfg.InfraExtChanOffset = EXTCHA_NONE;
    pPort->P2PCfg.P2PExtChanOffset = EXTCHA_NONE;
    pPort->P2PCfg.P2PGOExtChanOffset = EXTCHA_NONE;

    // Use to control MlmeDynamicTxRateSwitching (23->1: ok) (1->23: NG)
    pPort->P2PCfg.CurrentWcid = RESERVED_WCID;

#endif /*MULTI_CHANNEL_SUPPORT*/

#ifdef WFD_NEW_PUBLIC_ACTION
    pPort->P2PCfg.PrevState = P2P_NULL_STATE;
    pPort->P2PCfg.CurrState = P2P_NULL_STATE;
    pPort->P2PCfg.Signal = P2P_NULL_STATE;
    pPort->P2PCfg.LostAckRetryCounter = 0;
    pPort->P2PCfg.LostRspRetryCounter = 0;
    pPort->P2PCfg.PrevSentFrame = (P2P_PROVISION_RSP + 1);
#endif /*WFD_NEW_PUBLIC_ACTION*/

    // ==============>>
    // Init parameters with WIN8 interactions 
    pPort->P2PCfg.bAvailability = FALSE;
    pPort->P2PCfg.DeviceDiscoverable = DOT11_WFD_DEVICE_NOT_DISCOVERABLE;
    pPort->P2PCfg.WFDStatus = P2PSTATUS_SUCCESS;

    pPort->P2PCfg.MSDialogToken = 1;
    pPort->P2PCfg.WaitForMSCompletedFlags = P2pMs_WAIT_COMPLETE_NOUSE;

    // Handle ready to connection on client side.
    pPort->P2PCfg.InGroupFormation = FALSE;
    pPort->P2PCfg.WaitForWPSReady = FALSE;
    pPort->P2PCfg.ReadyToConnect = TRUE;

    pPort->P2PCfg.WFDProprietaryIEsLen = 0;
    PlatformZeroMemory(pPort->P2PCfg.WFDProprietaryIEs, sizeof(pPort->P2PCfg.WFDProprietaryIEs));
    pPort->P2PCfg.IsValidProprietaryIE = FALSE;
    // <<==============

#endif
    pPort->P2PCfg.P2PMode = P2P_MODE_WIN8;
}

/*  
    ==========================================================================
    Description: 
        Periodic Routine for P2P. 
        
    Parameters: 
         
    Note:
         
    ==========================================================================
 */
VOID P2pPeriodicExec(
    IN PMP_ADAPTER    pAd,
    IN PMP_PORT pPort) 
{
    ULONG       BssIdx = BSS_NOT_FOUND;
    UCHAR       i;
    //PMP_PORT  pPort = pAd->PortList[pPort->P2PCfg.PortNumber];
    ULONG       CurrentCounterAftrScanButton = 0;
    BOOLEAN     bMaintainDiscoverable = FALSE;
    UCHAR       ZeroMac[6] = {0,0,0,0,0,0};
    BOOLEAN     TimerCancelled = FALSE;
    
    // Counting per 100 ms
    //
    pPort->P2PCfg.P2pCounter.Counter100ms++;
    pPort->P2PCfg.P2pCounter.CounterAftrScanButton++;
    pPort->P2PCfg.P2pCounter.GoScanBeginCounter100ms++;
    pPort->P2PCfg.P2pCounter.CounterAfterConnectCommand++;
    pPort->P2PCfg.P2pCounter.TopologyUpdateCounter++;

    if (pPort->P2PCfg.P2pCounter.CounterAftrSetEvent  != 0xffffffff)
        pPort->P2PCfg.P2pCounter.CounterAftrSetEvent++;

    if (pPort->P2PCfg.P2pCounter.ManageAPEnReconCounter > 0)
        pPort->P2PCfg.P2pCounter.ManageAPEnReconCounter--;

    if (pPort->P2PCfg.P2pCounter.CounterAftrWpsDone > 0)
        pPort->P2PCfg.P2pCounter.CounterAftrWpsDone--;


    if (pPort->P2PCfg.P2pCounter.CounterAfterSetWscMode > 0)
        pPort->P2PCfg.P2pCounter.CounterAfterSetWscMode--;

    if(pPort->P2PCfg.P2pCounter.CounterP2PConnectionFail != 0xffffffff)
        pPort->P2PCfg.P2pCounter.CounterP2PConnectionFail++;
    
    if(pPort->P2PCfg.P2pCounter.CounterP2PAcceptReinvoke != 0xff)
        pPort->P2PCfg.P2pCounter.CounterP2PAcceptReinvoke--;
    
    pPort->P2PCfg.P2pCounter.Wpa2pskCounter100ms++;



    // win8
    pPort->P2PCfg.P2pCounter.CounterAfterMSSendCommand++;
    pPort->P2PCfg.P2pCounter.SwitchingChannelCounter++;
    pPort->P2PCfg.P2pCounter.StartRetryCounter++;
    if (pPort->P2PCfg.P2pCounter.QuickDiscoveryListUpdateCounter > 0)
        pPort->P2PCfg.P2pCounter.QuickDiscoveryListUpdateCounter--;

    if(pPort->P2PCfg.P2pCounter.CounterP2PConnectionFail == P2P_CONNECTION_FAIL)
    {
        pPort->P2PCfg.Dpid = DEV_PASS_ID_NOSPEC;
        P2pSetEvent(pAd, pPort,  P2PEVENT_CONNECT_FAIL);
    
    }
    
    if(pPort->P2PCfg.P2pCounter.CounterP2PAcceptReinvoke == 0)
    {
        pPort->P2PCfg.LockNego = FALSE;
        pPort->P2PCfg.P2pCounter.CounterP2PAcceptReinvoke = 0xff;
        DBGPRINT(RT_DEBUG_TRACE, ("Release LockNego and accept next one"));
    }

    // Action
    //
    // Update topology after one second after new stations are found
    if(pPort->P2PCfg.P2pCounter.TopologyUpdateCounter == 10)
        pPort->P2PCfg.P2pEventQueue.bP2pTopologyUpdate = TRUE;

    // Check TxFIFO if ACK is required
    if(pPort->P2PCfg.bP2pAckReq)
    {
        DBGPRINT(RT_DEBUG_ERROR, ("%s::Failed to get ACK from H/W, covered it by checking again\n", __FUNCTION__));
        MTEnqueueInternalCmd(pAd, pPort, MT_CMD_P2P_READ_TXFIFO, NULL, 0);
    }

#if(COMPILE_WIN8_ABOVE(CURRENT_OS_NDIS_VER))  
    if (NDIS_WIN8_ABOVE(pAd) &&
        (pPort->P2PCfg.P2PMode == P2P_MODE_WIN8) &&
        (MT_TEST_BIT(pPort->P2PCfg.WaitForMSCompletedFlags, P2pMs_WAIT_COMPLETE_DISCO_REQ)))
    {
        CurrentCounterAftrScanButton = pPort->P2PCfg.ScanPeriod;
    }
    else if (pPort->P2PCfg.P2PMode == P2P_MODE_WIN7)
#endif
    {
        CurrentCounterAftrScanButton = P2P_SCAN_PERIOD;

        // If I am alreayd operating, shorten the search time.
    if(!PlatformEqualMemory(&pPort->P2PCfg.ConnectingMAC[0][0], ZeroMac, MAC_ADDRESS_LENGTH))
        {
        CurrentCounterAftrScanButton = P2P_SCAN_FOR_CONNECTION;
    
        DBGPRINT(RT_DEBUG_TRACE, ("P2P_NEGOTIATION at most scan 2 mins"));

        }
        else if(!PlatformEqualMemory(&pPort->P2PCfg.ConnectingMAC[0][0], ZeroMac, MAC_ADDRESS_LENGTH))
        {
            CurrentCounterAftrScanButton = P2P_SCAN_FOR_CONNECTION;
    
            DBGPRINT(RT_DEBUG_TRACE, ("P2P_NEGOTIATION at most scan 2 mins"));

        }
    }

        DBGPRINT(RT_DEBUG_TRACE, ("%s  port %d P2pCounter.CounterAftrScanButton= %d CurrentCounterAftrScanButton = %d\n ", __FUNCTION__, pPort->PortNumber, pPort->P2PCfg.P2pCounter.CounterAftrScanButton, CurrentCounterAftrScanButton));
    

    if (pPort->P2PCfg.P2PChannelState == P2P_ENTER_GOTOSCAN)
    {
        AsicPauseBssSync(pAd);
        if (pPort->P2PCfg.QueuedScanReq.ScanType == 0)
        {
            // Let BBP register at 20MHz to do scan     
            BbSetTxRxBwCtrl(pAd, BW_20);    
            MlmeSyncScanMlmeSyncNextChannel(pAd, pAd->PortList[PORT_0]); // this routine will stop if pPort->ScaningChannel == 0
        }
        else
        {
            pPort->P2PCfg.P2pCounter.Wpa2pskCounter100ms = 0;
            pAd->HwCfg.bLedOnScanning = TRUE;
            LedCtrlSetLed(pAd, LED_ON_SITE_SURVEY);
        }
        pPort->P2PCfg.P2PChannelState = P2P_DIFF_CHANNEL_OFF;
    }
    // Scan period expired. Return to listen state. Only do once. So check value equal.
    if (pPort->P2PCfg.P2pCounter.CounterAftrScanButton >= CurrentCounterAftrScanButton)
    {
        if (pPort->P2PCfg.P2PDiscoProvState != P2P_ENABLE_LISTEN_ONLY)
        {
            DBGPRINT(RT_DEBUG_TRACE, ("!!!Goback to state P2P_ENABLE_LISTEN_ONLY (scantimeout=%d)!!!\n", CurrentCounterAftrScanButton));

            pPort->P2PCfg.P2PDiscoProvState = P2P_ENABLE_LISTEN_ONLY;
        
            P2pSetListenIntBias(pAd, 3);
            for (i = 0;i < MAX_P2P_GROUP_SIZE;i++)
            {
                if ((pAd->pP2pCtrll->P2PTable.Client[i].P2pClientState == P2PSTATE_INVITE_COMMAND) 
                    ||(pAd->pP2pCtrll->P2PTable.Client[i].P2pClientState == P2PSTATE_CONNECT_COMMAND))
                {
                    P2pGroupTabDelete(pAd, i, pAd->pP2pCtrll->P2PTable.Client[i].addr);
                    DBGPRINT(RT_DEBUG_TRACE, ("!!! P2P Device = %d Connect command timeout. Return state to %s \n", i, decodeP2PClientState(pAd->pP2pCtrll->P2PTable.Client[i].P2pClientState)));
                }
            }
        }
#if(COMPILE_WIN8_ABOVE(CURRENT_OS_NDIS_VER))  
        //Indicate to discovery complete to OS once scan period expired
        if (NDIS_WIN8_ABOVE(pAd) && MT_TEST_BIT(pPort->P2PCfg.WaitForMSCompletedFlags, P2pMs_WAIT_COMPLETE_DISCO_REQ)
            && (pPort->P2PCfg.P2pCounter.CounterAftrScanButton >= CurrentCounterAftrScanButton))
        {
            // Stop p2p scan
            DBGPRINT(RT_DEBUG_TRACE, ("SYNC -  ScanType(%d), Mlme.SyncMachine.CurrState(%d)\n", pAd->MlmeAux.ScanType, pAd->PortList[PORT_0]->Mlme.SyncMachine.CurrState));
#if 0           
            if ((pAd->MlmeAux.ScanType == SCAN_P2P) && (pAd->PortList[PORT_0]->Mlme.SyncMachine.CurrState == SCAN_LISTEN))
            {
                // Set scan channel to Last one to stop Scan Phase. Because SCannextchannel will use channel to judge if it should stop scan.

                if((pPort->ScaningChannel != 0) && (pPort->ScaningChannel != pPort->P2PCfg.P2pProprietary.ListenChanel[pPort->P2PCfg.P2pProprietary.ListenChanelCount-1]))
                    pPort->ScaningChannel = pPort->P2PCfg.P2pProprietary.ListenChanel[pPort->P2PCfg.P2pProprietary.ListenChanelCount-1];
                
                DBGPRINT(RT_DEBUG_TRACE, ("P2P Scan Timeout! End of switching channel\n"));

                // temp solution for common channel affected by scan
                AsicSwitchChannel(pAd, pPort->Channel, FALSE);
                DBGPRINT(RT_DEBUG_TRACE, ("P2P Scan Timeout! Restore to common channel(%d)\n", pPort->Channel));                
            }
            P2pMsIndicateDiscoverComplete(pAd, pPort);
            MT_CLEAR_BIT(pPort->P2PCfg.WaitForMSCompletedFlags, P2pMs_WAIT_COMPLETE_DISCO_REQ);         
#else
            MT_CLEAR_BIT(pPort->P2PCfg.WaitForMSCompletedFlags, P2pMs_WAIT_COMPLETE_DISCO_REQ);

           // if ((pAd->MlmeAux.ScanType == SCAN_P2P) || (pAd->MlmeAux.ScanType == SCAN_P2P_PASSIVE))
            {
                PlatformCancelTimer(&pPort->Mlme.ScanTimer, &TimerCancelled);
                pPort->ScaningChannel = 0;

                // Use port 0 for scan state machine control
                // TODO: Sync to general MlmeSyncScanDoneAction
                P2pMsScanDone(pAd, pAd->PortList[PORT_0]);
                //pAd->PortList[PORT_0]->Mlme.SyncMachine.CurrState = SYNC_IDLE;
                //pAd->PortList[PORT_0]->Mlme.CntlMachine.CurrState = CNTL_IDLE;
                //AsicSwitchChannel(pAd, pPort->Channel, FALSE);
                //RTMPResumeMsduTransmission(pAd, pPort);

                // Use p2p device port as indication port
                P2pMsIndicateDiscoverComplete(pAd, pPort);
            }
#endif
        }

        // Send out the  buffering of NEGO request frame
        if ((pPort->P2PCfg.pRequesterBufferedFrame) && MT_TEST_BIT(pPort->P2PCfg.WaitForMSCompletedFlags, P2pMs_WAIT_COMPLETE_NEGO_REQ_BUFFERED_AFTER_SCAN))
        {
            DBGPRINT(RT_DEBUG_TRACE, ("%s - TO %x %x %x %x %x %x. \n",
                    __FUNCTION__, pPort->P2PCfg.ConnectingMAC[0][0], pPort->P2PCfg.ConnectingMAC[0][1], pPort->P2PCfg.ConnectingMAC[0][2],
                    pPort->P2PCfg.ConnectingMAC[0][3], pPort->P2PCfg.ConnectingMAC[0][4], pPort->P2PCfg.ConnectingMAC[0][5]));

            // Update flags
            MT_CLEAR_BIT(pPort->P2PCfg.WaitForMSCompletedFlags, P2pMs_WAIT_COMPLETE_NEGO_REQ_BUFFERED_AFTER_SCAN);
            MT_SET_BIT(pPort->P2PCfg.WaitForMSCompletedFlags, P2pMs_WAIT_COMPLETE_NEGO_REQ);
            DBGPRINT(RT_DEBUG_TRACE, ("%s - Send Nego Request (len = %d, flags = %d) \n",__FUNCTION__, pPort->P2PCfg.RequesterBufferedSize, pPort->P2PCfg.WaitForMSCompletedFlags));
                
            // Kickoff TX
            NdisCommonMiniportMMRequest(pAd, pPort->P2PCfg.pRequesterBufferedFrame, pPort->P2PCfg.RequesterBufferedSize);

            //Free Buffered Data
            P2pMsFreeIeMemory(&pPort->P2PCfg.pRequesterBufferedFrame, (PLONG)&pPort->P2PCfg.RequesterBufferedSize);
        }

#endif
    }

    // ====================================>
    //  P2P connect state maintain
    if ((pPort->P2PCfg.P2PConnectState == P2P_DO_WPS_DONE_NEED_SETEVENT) 
        && (pPort->P2PCfg.P2pCounter.CounterAftrWpsDone == 0))
    {
        pPort->P2PCfg.P2PConnectState = P2P_DO_WPS_ENROLLEE_DONE;
        P2pSetEvent(pAd, pPort,  P2PEVENT_STATUS_WPS_DONE);
    }
    else if (((pPort->P2PCfg.P2PConnectState == P2P_DO_GO_SCAN_BEGIN) 
        || (pPort->P2PCfg.P2PConnectState == P2P_DO_GO_NEG_DONE_CLIENT)
        || (pPort->P2PCfg.P2PConnectState == P2P_DO_GO_SCAN_OP_BEGIN))
        && (pPort->P2PCfg.P2pCounter.Counter100ms == pPort->P2PCfg.P2pCounter.NextScanRound))
    {
        DOT11_SCAN_REQUEST_V2 Dot11ScanRequest = {0};
        // in P2P_DO_GO_SCAN_OP_BEGIN mode, only scan opchannel to speed up connection time.
        // But 2nd time scan should include all supported channel in case GO doesn't choose opchannel at last.
        // Spec allow GO NOT to choose  opchannel for its p2p group.
        DBGPRINT(RT_DEBUG_TRACE, ("P2P P2pPeriodicExec Scan Begin NextScanRound = %d \n", pPort->P2PCfg.P2pCounter.NextScanRound));
        Dot11ScanRequest.dot11ScanType = (dot11_scan_type_active | dot11_scan_type_forced);
        Dot11ScanRequest.uNumOfdot11SSIDs = 0;

        // Only Port0 support BSSID LIST SCAN behavior, So use Port 0 to enqueu scan command.
        MlmeEnqueue(pAd,
                    pPort,
                    MLME_CNTL_STATE_MACHINE, 
                    OID_802_11_BSSID_LIST_SCAN, 
                    sizeof(DOT11_SCAN_REQUEST_V2), 
                    &Dot11ScanRequest);
        //pPort->P2PCfg.P2pCounter.NextScanRound = (RandomByte(pAd) % P2P_RANDOM_WPS_BASE) + 4;
    }
    else if ((pPort->P2PCfg.P2PConnectState == P2P_DO_GO_SCAN_DONE) 
        || (pPort->P2PCfg.P2PConnectState == P2P_DO_GO_SCAN_OP_DONE))
    {           
        if(pPort->PortSubtype != PORTSUBTYPE_P2PGO)
        {
            BssIdx = BssTableSearch(pAd, pPort, &pAd->ScanTab, pPort->P2PCfg.Bssid, pPort->StaCfg.WscControl.WscAPChannel);
        }
        // Since can't find the target AP in the list.
        // Go back to scan state again to scan the target AP.
        if (BssIdx == BSS_NOT_FOUND) 
        {
            if(pPort->P2PCfg.P2pCounter.HoldAtNegoDoneClinetTimes < 1)
            {
                pPort->P2PCfg.P2PConnectState = P2P_DO_GO_NEG_DONE_CLIENT;
                pPort->P2PCfg.P2pCounter.Counter100ms = 0;
                pPort->P2PCfg.P2pCounter.NextScanRound = 1;   // start scan after 0.1 s jesse
                DBGPRINT(RT_DEBUG_TRACE, ("First time CNTL -Nr= %d. Channel = %d. BSSID not found.  %x %x %x %x %x %x  \n", pAd->ScanTab.BssNr, pPort->StaCfg.WscControl.WscAPChannel, pPort->P2PCfg.Bssid[0], pPort->P2PCfg.Bssid[1],pPort->P2PCfg.Bssid[2],pPort->P2PCfg.Bssid[3],pPort->P2PCfg.Bssid[4],pPort->P2PCfg.Bssid[5]));
                DBGPRINT(RT_DEBUG_TRACE, ("First time CNTL - 1. BSSID not found.  Goback to %s  \n", decodeP2PState(pPort->P2PCfg.P2PConnectState)));
                pPort->P2PCfg.P2pCounter.HoldAtNegoDoneClinetTimes++;
            }
            else
            {
                pPort->P2PCfg.P2PConnectState = P2P_DO_GO_SCAN_BEGIN;
                pPort->P2PCfg.P2pCounter.Counter100ms = 0;
                pPort->P2PCfg.P2pCounter.NextScanRound = 3;   // start scan after 0.3 s jesse
                DBGPRINT(RT_DEBUG_TRACE, ("Second time CNTL -Nr= %d. Channel = %d. BSSID not found.  %x %x %x %x %x %x  \n", pAd->ScanTab.BssNr, pPort->StaCfg.WscControl.WscAPChannel, pPort->P2PCfg.Bssid[0], pPort->P2PCfg.Bssid[1],pPort->P2PCfg.Bssid[2],pPort->P2PCfg.Bssid[3],pPort->P2PCfg.Bssid[4],pPort->P2PCfg.Bssid[5]));
                DBGPRINT(RT_DEBUG_TRACE, ("Second time CNTL - 1. BSSID not found.  Goback to %s  \n", decodeP2PState(pPort->P2PCfg.P2PConnectState)));
                
            }
            return;
        }
        else if (pAd->ScanTab.BssEntry[BssIdx].SsidLen < 9)
        {
            pPort->P2PCfg.P2PConnectState = P2P_DO_GO_SCAN_BEGIN;
            
            pPort->P2PCfg.P2pCounter.Counter100ms = 0;
            pPort->P2PCfg.P2pCounter.NextScanRound = 5;   // start scan after 500ms
            DBGPRINT(RT_DEBUG_TRACE, ("CNTL -Nr= %d. Channel = %d.  SSID is  %c%c%c%c%c%c \n", pAd->ScanTab.BssNr, pPort->StaCfg.WscControl.WscAPChannel, pPort->P2PCfg.SSID[0], pPort->P2PCfg.Bssid[1],pPort->P2PCfg.SSID[2],pPort->P2PCfg.SSID[3],pPort->P2PCfg.SSID[4],pPort->P2PCfg.SSID[5]));
            DBGPRINT(RT_DEBUG_TRACE, ("CNTL -2  Goback to %s  \n", decodeP2PState(pPort->P2PCfg.P2PConnectState)));
            return;
        }

        // Now copy the scanned SSID to my CommonCfg.Ssid
        PlatformMoveMemory(pAd->PortList[pPort->P2PCfg.PortNumber]->PortCfg.Ssid, pAd->ScanTab.BssEntry[BssIdx].Ssid,32);
        pAd->PortList[pPort->P2PCfg.PortNumber]->PortCfg.SsidLen = pAd->ScanTab.BssEntry[BssIdx].SsidLen;
        DBGPRINT(RT_DEBUG_TRACE, ("P2P P2pPeriodicExec P2P_DO_GO_SCAN_DONE. Find BssIdx = %d\n", BssIdx));
        pPort->P2PCfg.P2pEventQueue.bP2pGoNegoDone = FALSE;
        // Set Go Negociation Done event to GUI. GUI will start WSC connection process to driver
        // bying setting WSC_MODE OID.
        pPort->P2PCfg.P2PConnectState = P2P_DO_WPS_ENROLLEE;
        if (FALSE == P2pSetEvent(pAd, pPort,  P2PEVENT_STATUS_GO_NEGO_DONE))
            pPort->P2PCfg.P2pEventQueue.bP2pGoNegoDone = TRUE;

    }
    // <<====================================
    P2pGroupMaintain(pAd, pPort);

#if(COMPILE_WIN8_ABOVE(CURRENT_OS_NDIS_VER))  
    if(NDIS_WIN8_ABOVE(pAd))
        P2pMsTimeoutAction(pAd, pPort);
#endif

    // P2P_ANY_IN_FORMATION_AS_GO means I am AutoGO. AutoGo also need to do scan. So don't return here.
    if (IS_P2P_CONNECTING(pPort) && (pPort->P2PCfg.P2PConnectState != P2P_ANY_IN_FORMATION_AS_GO))
    {
        if((pPort->P2PCfg.P2PConnectState == P2P_I_AM_CLIENT_ASSOC_AUTH) && 
                        //(pPort->CommonCfg.P2pControl.field.EnablePresistent == 1) && 
                ((pAd->Mlme.PeriodicRound % 30) == 0)&&
                (pAd->PortList[pPort->P2PCfg.PortNumber]->PortSubtype == PORTSUBTYPE_P2PClient))
        {
            
            if(pPort->P2PCfg.P2pCounter.CounterAutoReconnectForP2pClient < 5)
            {
                DBGPRINT(RT_DEBUG_TRACE, ("case2 : pPort->P2PCfg.P2pCounter.CounterAutoReconnectForP2pClient = %d \n", pPort->P2PCfg.P2pCounter.CounterAutoReconnectForP2pClient));
                DBGPRINT(RT_DEBUG_TRACE, ("case2 : pPort->P2PCfg.PersistentGOAddr = %02x %02x %02x %02x %02x %02x\n", pPort->P2PCfg.PersistentGOAddr[0], pPort->P2PCfg.PersistentGOAddr[1], pPort->P2PCfg.PersistentGOAddr[2], pPort->P2PCfg.PersistentGOAddr[3], pPort->P2PCfg.PersistentGOAddr[4], pPort->P2PCfg.PersistentGOAddr[5]));
                MlmeEnqueue(pAd,
                            pPort,
                            MLME_CNTL_STATE_MACHINE, 
                            OID_DOT11_CONNECT_REQUEST,
                            0,
                            NULL);
                pPort->P2PCfg.P2pCounter.CounterAutoReconnectForP2pClient ++;
            }
        }
        return;
    }

    // <<====================================
    // ====================================>
    // Check whether to start a  P2P scan/search process.
    if ((pPort->P2PCfg.P2PDiscoProvState == P2P_IDLE) && (pPort->P2PCfg.P2pCounter.Counter100ms == pPort->P2PCfg.P2pCounter.NextScanRound))
    {
        DBGPRINT(RT_DEBUG_TRACE, ("!!GroupOpChannel = %d. Counter100ms = %d  NextScanRound = %d  \n", pPort->P2PCfg.GroupOpChannel,
                                            pPort->P2PCfg.P2pCounter.Counter100ms,pPort->P2PCfg.P2pCounter.NextScanRound));
        // If I just finish group formation as GO. don't do scan . If I am auto GO, I should support P2P scan too. So check GoIntentIdx != 16.
        if ((pPort->P2PCfg.P2PConnectState == P2P_ANY_IN_FORMATION_AS_GO || pPort->P2PCfg.P2PConnectState == P2P_WPS_REGISTRA || pPort->P2PCfg.P2PConnectState == P2P_DO_WPS_ENROLLEE)
            && (IS_P2P_NOT_AUTOGO(pPort)))
        {
            pPort->P2PCfg.P2pCounter.Counter100ms = 0;
        }
        else
        {
#if(COMPILE_WIN8_ABOVE(CURRENT_OS_NDIS_VER))  
            if (NDIS_WIN8_ABOVE(pAd) && 
                MT_TEST_BIT(pPort->P2PCfg.ScanWithFilter, P2pMs_SCAN_FILTER_DEVICE_ID) &&
                (pPort->P2PCfg.P2pProprietary.ListenChanelCount < 3))
            {
                // for quck p2p connection
                if (pAd->LogoTestCfg.OnTestingWHQL == TRUE)
                {
                    P2pSetListenIntBias(pAd,  0);
                    pPort->P2PCfg.P2pCounter.ListenInterval = 1/*(RandomByte(pAd) % 3)*/ + pPort->P2PCfg.P2pCounter.ListenIntervalBias;  // 1~3
                    pPort->P2PCfg.ScanPeriod = ((pPort->P2PCfg.P2pProprietary.ListenChanelCount * FAST_ACTIVE_SCAN_TIME)/ONETU + 3) + pPort->P2PCfg.P2pCounter.NextScanRound;
                }
                else
                {
                    // Spend more time in the peer channel
                    P2pSetListenIntBias(pAd,  3); // 300 ms
                    // Total scan time from OS is 50,000 ms
                    pPort->P2PCfg.P2pCounter.ListenInterval = (RandomByte(pAd) % 3) + pPort->P2PCfg.P2pCounter.ListenIntervalBias;  // 1~3
                    pPort->P2PCfg.ScanPeriod = min(50, (pPort->P2PCfg.P2pProprietary.ListenChanelCount + 1) * (pPort->P2PCfg.P2pCounter.ListenInterval) + pPort->P2PCfg.P2pCounter.NextScanRound);
                }
                DBGPRINT(RT_DEBUG_TRACE, ("Scan Filter::pPort->P2PCfg.ScanPeriod=%d\n", pPort->P2PCfg.ScanPeriod));
            }
            else
#endif
            {
                pPort->P2PCfg.P2pCounter.ListenInterval = (RandomByte(pAd) % 3) + pPort->P2PCfg.P2pCounter.ListenIntervalBias;  // 1~3
            }


            // ExtListenInterval is in ms. So /100
#if(COMPILE_WIN8_ABOVE(CURRENT_OS_NDIS_VER))  
            if (IS_P2P_SUPPORT_EXT_LISTEN(pAd, pPort) && (!MT_TEST_BIT(pPort->P2PCfg.WaitForMSCompletedFlags, P2pMs_WAIT_COMPLETE_DISCO_REQ)))
#else
            if (IS_P2P_SUPPORT_EXT_LISTEN(pAd, pPort))
#endif
                pPort->P2PCfg.P2pCounter.ListenInterval = pPort->P2PCfg.ExtListenPeriod/100;
            
            DBGPRINT(RT_DEBUG_TRACE, ("Start P2P Scan. ListenInterval = %d \n", pPort->P2PCfg.P2pCounter.ListenInterval));
            if ((pPort->Mlme.CntlMachine.CurrState != CNTL_IDLE) ||
                (pPort->Mlme.SyncMachine.CurrState != SYNC_IDLE))
            {
                MlmeRestartStateMachine(pAd, pPort);
                DBGPRINT(RT_DEBUG_TRACE, ("!!! MLME busy, reset MLME state machine !!!\n"));
            }
            pPort->P2PCfg.P2PDiscoProvState = P2P_SEARCH;
            
            P2pScan(pAd, pPort);
        }
    }
    else if ((pPort->P2PCfg.P2PDiscoProvState == P2P_LISTEN_COMPLETE) ||(pPort->P2PCfg.P2PDiscoProvState == P2P_SEARCH_COMPLETE) )
    {
        DBGPRINT(RT_DEBUG_TRACE, ("!!GroupOpChannel = %d.    \n", pPort->P2PCfg.GroupOpChannel));
        pPort->P2PCfg.P2PDiscoProvState = P2P_IDLE;
        pPort->P2PCfg.P2pCounter.Counter100ms = 0;
        pPort->P2PCfg.P2pCounter.ListenInterval = (RandomByte(pAd) % 3) +  pPort->P2PCfg.P2pCounter.ListenIntervalBias;  // 1~3 
        pPort->P2PCfg.P2pCounter.NextScanRound = (RandomByte(pAd) % P2P_RANDOM_BASE) + pPort->P2PCfg.P2pCounter.ScanIntervalBias +  pPort->P2PCfg.P2pCounter.ScanIntervalBias2;
        // ExtListenInterval is in ms. So /100
#if(COMPILE_WIN8_ABOVE(CURRENT_OS_NDIS_VER))  
        if (IS_P2P_SUPPORT_EXT_LISTEN(pAd, pPort) && (!MT_TEST_BIT(pPort->P2PCfg.WaitForMSCompletedFlags, P2pMs_WAIT_COMPLETE_DISCO_REQ)))
#else       
        if (IS_P2P_SUPPORT_EXT_LISTEN(pAd, pPort))
#endif          
            pPort->P2PCfg.P2pCounter.NextScanRound = pPort->P2PCfg.ExtListenInterval/100;

        // If use station + P2P concurrent mode and already associate to an AP, and after 5 min after activating p2p profile,
        // We can consider to decrease the discovery site survey frequency to let normal WLAN Infra has better performance. 
        DBGPRINT(RT_DEBUG_TRACE, ("P2P :: Stop P2P Scan::Next Scan Round Info::Counter100ms(%d), ListenInterval(%d), NextScanRound(%d), CounterAftrScanButton(%d),ScanPeriod(%d)\n", 
                                      pPort->P2PCfg.P2pCounter.Counter100ms,
                                      pPort->P2PCfg.P2pCounter.ListenInterval, 
                                      pPort->P2PCfg.P2pCounter.NextScanRound,
                                      pPort->P2PCfg.P2pCounter.CounterAftrScanButton,
                                      pPort->P2PCfg.ScanPeriod));
        // In concurrent mode, don't need to scan all supported channel for other GO in other channel.
        // Because we can only become GO in concurrent mode now, won't never connect to other GO.
        if (IS_P2P_SIGMA_OFF(pPort))
            P2pScanChannelUpdate(pAd, pPort);
    }
    else if (pPort->P2PCfg.P2PDiscoProvState == P2P_SEARCH)
    {
        // handle below code after channel switch
#if 0
        // Send more than one probe request during search state to increase discovery possibility.
        if (pAd->PortList[pPort->P2PCfg.PortNumber]->PortSubtype == PORTSUBTYPE_P2PGO)
            P2pSendProbeReq(pAd, PORTSUBTYPE_P2PGO);
        else
            P2pSendProbeReq(pAd, PORTSUBTYPE_P2PClient);
#endif          
    }
    // <<====================================
    
    // This bMaintainDiscoverable can force me to go to listen state for 100~300ms
    // every 10 seconds in average.
    if ((pAd->OpMode == OPMODE_STAP2P) && (INFRA_ON(pAd->PortList[PORT_0]))
        && (pPort->P2PCfg.ListenChannel != pPort->Channel))
        bMaintainDiscoverable = TRUE;

    // When I am in P2P_CLIENT_IN_IDLE, I should not go to idle power save state.
    
    // Maintain listen state when in Concurrent mode. STA+P2P
    if ((bMaintainDiscoverable == TRUE)
        && (pPort->P2PCfg.P2PDiscoProvState == P2P_ENABLE_LISTEN_ONLY))
    {
        // Don't go to listen state when traffic is a little high.
        // don't go to listen state when doing WPS provistioning.
        if (((pAd->Counter.MTKCounters.LastOneSecTotalTxCount + pAd->Counter.MTKCounters.OneSecRxOkDataCnt > 80))
            || (pPort->StaCfg.WscControl.WscState > WSC_STATE_INIT))
        {
        }
        else            
        {
            if ((pPort->P2PCfg.P2pCounter.Counter100ms%100) == 41)
            {
                // Start go to listen channel 
                pPort->P2PCfg.P2pProprietary.ListenChanel[0] = pPort->P2PCfg.ListenChannel;
                pPort->P2PCfg.P2pProprietary.ListenChanelIndex = 0;
                pPort->P2PCfg.P2pProprietary.ListenChanelCount = 1;
                P2pSetListenIntBias(pAd, 5);    // 500ms
                pPort->P2PCfg.P2PDiscoProvState = P2P_LISTEN;
                pPort->P2PCfg.P2pCounter.ListenInterval = (RandomByte(pAd) % 3) + pPort->P2PCfg.P2pCounter.ListenIntervalBias;  // 1~3
                DBGPRINT(RT_DEBUG_TRACE, ("P2P  Go to listen channel for a while = %d \n",pPort->P2PCfg.P2pCounter.ListenInterval));
                P2pScan(pAd, pPort);
            }
        }

    }

    // check if need to resume NoA Schedule.
    if (pPort->P2PCfg.GONoASchedule.bNeedResumeNoA == TRUE)
    {
        pPort->P2PCfg.GONoASchedule.bNeedResumeNoA = FALSE;
        P2pGOStartNoA(pAd, pPort, FALSE);
        P2pUpdateNoABeacon(pAd, pAd->pTxCfg->BeaconBufLen);
    }

        // check if need to autoreconnect for client, every 3 secs
    if((pPort->P2PCfg.P2PConnectState == P2P_CONNECT_IDLE) && 
        (pPort->CommonCfg.P2pControl.field.EnablePresistent == 1) && 
        ((pAd->Mlme.PeriodicRound % 30) == 0) && 
        (!IS_P2P_FINDING(pPort)) && 
        (!MT_TEST_FLAG(pAd, fRTMP_ADAPTER_BSS_SCAN_IN_PROGRESS)) &&
        (pAd->PortList[pPort->P2PCfg.PortNumber]->PortSubtype == PORTSUBTYPE_P2PClient))
    {
        if(pPort->P2PCfg.P2pCounter.CounterAutoReconnectForP2pClient < 10)
        {
            DBGPRINT(RT_DEBUG_TRACE, ("pPort->P2PCfg.P2pCounter.CounterAutoReconnectForP2pClient = %d \n", pPort->P2PCfg.P2pCounter.CounterAutoReconnectForP2pClient));
            DBGPRINT(RT_DEBUG_TRACE, ("pPort->P2PCfg.PersistentGOAddr = %02x %02x %02x %02x %02x %02x\n", pPort->P2PCfg.PersistentGOAddr[0], pPort->P2PCfg.PersistentGOAddr[1], pPort->P2PCfg.PersistentGOAddr[2], pPort->P2PCfg.PersistentGOAddr[3], pPort->P2PCfg.PersistentGOAddr[4], pPort->P2PCfg.PersistentGOAddr[5]));
            P2pAutoReconnectPerstClient(pAd, pPort->P2PCfg.PersistentGOAddr, pPort->P2PCfg.P2pCounter.CounterAutoReconnectForP2pClient);
            pPort->P2PCfg.P2pCounter.CounterAutoReconnectForP2pClient ++;
        }
    }
}

/*
    ========================================================================
    
    Routine Description:
        During the period that GUI can't GetEvent, Driver need to queue event command. EventMaintain function can Set those
        queued events to GUI.

    Arguments:
        pAdapter    - NIC Adapter pointer
        
    Return Value:
        None

    IRQL = DISPATCH_LEVEL
    
    Note:
    ========================================================================
*/
VOID P2pEventMaintain(
    IN PMP_ADAPTER pAd)
{
    PMP_PORT    pPort = pAd->PortList[FXXK_PORT_0];
    
    // P2P and SoftapAP/VwifiAP can not co-exist.
    if (pAd->OpMode == OPMODE_AP || (pAd->OpMode == OPMODE_APCLIENT))
        return;

    //If GUI doesn't set event handler, return.
    if (pPort->P2PCfg.P2pEvent == NULL)
        return;
    
    // Check if GUI stays in the state that can receive Event.
    if (pPort->P2PCfg.P2PQueryStatusOid.P2PStatus == P2PEVENT_STATUS_IDLE)
    {
        // Check if there are queued event that needs to be set.
        // 
        // bDiscoveryUpdate must be first
        if (pPort->P2PCfg.P2pEventQueue.bDiscoveryUpdate == TRUE)
        {
            pPort->P2PCfg.P2pEventQueue.bDiscoveryUpdate = FALSE;
            P2pSetEvent(pAd, pPort,  P2PEVENT_STATUS_UPDATE_DISCOVERY);
        }
        else if(pPort->P2PCfg.P2pEventQueue.bPopWindow == TRUE)
        {
            pPort->P2PCfg.P2pEventQueue.bPopWindow = FALSE;
            P2pSetEvent(pAd, pPort,  P2PEVENT_POPUP_SETTING_WINDOWS);
        }
        else if(pPort->P2PCfg.P2pEventQueue.bPopWindowForGO == TRUE)
        {
            pPort->P2PCfg.P2pEventQueue.bPopWindowForGO = FALSE;
            P2pSetEvent(pAd, pPort,  P2PEVENT_POPUP_SETTING_WINDOWS_FOR_AUTOGO);
        }
        else if(pPort->P2PCfg.P2pEventQueue.bShowPinCode== TRUE)
        {
            pPort->P2PCfg.P2pEventQueue.bShowPinCode = FALSE;
            P2pSetEvent(pAd, pPort,  P2PEVENT_DISPLAY_PIN);
        }
        else if (pPort->P2PCfg.P2pEventQueue.bDeleteWirelessCon == TRUE)
        {
            if(P2pSetEvent(pAd, pPort,  P2PEVENT_DISCONNECT))
                pPort->P2PCfg.P2pEventQueue.bDeleteWirelessCon = FALSE;
        }
        else if (pPort->P2PCfg.P2pEventQueue.bP2pGoNegoDone == TRUE)
        {
            pPort->P2PCfg.P2pEventQueue.bP2pGoNegoDone = FALSE;
            P2pSetEvent(pAd, pPort,  P2PEVENT_STATUS_GO_NEGO_DONE);
        }
        else if (pPort->P2PCfg.P2pEventQueue.bP2pGenSeudoAdhoc == TRUE)
        {
            if(P2pSetEvent(pAd, pPort,  P2PEVENT_GEN_SEUDO_ADHOC_PROFILE))
                pPort->P2PCfg.P2pEventQueue.bP2pGenSeudoAdhoc = FALSE;
        }
        else if (pPort->P2PCfg.P2pEventQueue.bWpaDone)
        {
            if(P2pSetEvent(pAd, pPort,  P2PEVENT_CONNECT_COMPLETE))
                pPort->P2PCfg.P2pEventQueue.bWpaDone = FALSE;
        }
        else if (pPort->P2PCfg.P2pEventQueue.bEnableDhcp == TRUE)
        {       
            if(P2pSetEvent(pAd, pPort,  P2PEVENT_ENABLE_DHCP_SERVER))
                pPort->P2PCfg.P2pEventQueue.bEnableDhcp = FALSE;
        }
        else if (pPort->P2PCfg.P2pEventQueue.bDisableDhcp== TRUE)
        {
            if(P2pSetEvent(pAd, pPort,  P2PEVENT_DISABLE_DHCP_SERVER))
                pPort->P2PCfg.P2pEventQueue.bDisableDhcp = FALSE;
        }
        else if (pPort->P2PCfg.P2pEventQueue.bWpsNack == TRUE)
        {
            pPort->P2PCfg.P2pEventQueue.bWpsNack = FALSE;
            P2pSetEvent(pAd, pPort,  P2PEVENT_STATUS_WPS_FAIL);
        }
        else if (pPort->P2PCfg.P2pEventQueue.bErrorCode == TRUE)
        {
            if(P2pSetEvent(pAd, pPort,  P2PEVENT_ERROR_CODE))
                pPort->P2PCfg.P2pEventQueue.bErrorCode = FALSE;
        }
        else if (pPort->P2PCfg.P2pEventQueue.bP2pTopologyUpdate == TRUE)
        {                       
            if(P2pSetEvent(pAd, pPort,  P2PEVENT_P2PGROUP_TOPOLOGY_UPDATE))
                pPort->P2PCfg.P2pEventQueue.bP2pTopologyUpdate = FALSE;
        }
        else if (pPort->P2PCfg.P2pEventQueue.bSwitchToSTA == TRUE)
        {                       
            pPort->P2PCfg.P2pEventQueue.bSwitchToSTA = FALSE;
            P2pSetEvent(pAd, pPort,  P2PEVENT_SWITCH_OPMODE_STA);
        }
        else if (pPort->P2PCfg.P2pEventQueue.bSwitchToConcurrent == TRUE)
        {                       
            pPort->P2PCfg.P2pEventQueue.bSwitchToConcurrent = FALSE;
            P2pSetEvent(pAd, pPort,  P2PEVENT_SWITCH_OPMODE_STAP2P);
        }       
        else if (pPort->P2PCfg.P2pEventQueue.bDisableIcs == TRUE)
        {                       
            pPort->P2PCfg.P2pEventQueue.bDisableIcs = FALSE;
            P2pSetEvent(pAd, pPort,  P2PEVENT_DISABLE_ICS);
        }
        else if (pPort->P2PCfg.P2pEventQueue.bSigmaIsOn)
        {
            pPort->P2PCfg.P2pEventQueue.bSigmaIsOn = FALSE;
            P2pSetEvent(pAd, pPort,  P2PEVENT_SIGMA_IS_ON);
        }
        
    }
    else if ((pPort->P2PCfg.P2pCounter.CounterAftrSetEvent > 1200) 
        && (pPort->P2PCfg.P2PQueryStatusOid.P2PStatus != P2PEVENT_STATUS_IDLE))
    {
        // If UI is unable to create seudo adhoc profile, we need to turn off ResetInProgress
        

        // Because GUI set EvetnReady and then need a little time to call WaitForObject().
        // During the little period, dirver might call KeSetEvent, and thus cause GUI to miss this event.
        // To release this locked status, driver unlock itself by Set Event to idle.
        // Driver doesn't keep last event, So miss a event. 
        DBGPRINT(RT_DEBUG_TRACE,("GUI missed KeSetEvent last time because driver may SetEvent before GUI call WaitForObject.!Status = %d.\n", pPort->P2PCfg.P2PQueryStatusOid.P2PStatus));
        DBGPRINT(RT_DEBUG_TRACE,("Let driver set event to ready state myself! .CounterAftrSetEvent = %d. \n", pPort->P2PCfg.P2pCounter.CounterAftrSetEvent));
        P2pSetEvent(pAd, pPort,  P2PEVENT_STATUS_IDLE);
        // 
        DBGPRINT(RT_DEBUG_TRACE,("Then Reset Last unsuccessful event = %d. \n", pPort->P2PCfg.P2pEventQueue.LastSetEvent));
        if((pPort->P2PCfg.P2pEventQueue.LastSetEvent != P2PEVENT_POPUP_SETTING_WINDOWS) && (pPort->P2PCfg.P2pEventQueue.LastSetEvent != P2PEVENT_POPUP_SETTING_WINDOWS_FOR_AUTOGO))
            P2pSetEvent(pAd, pPort,  pPort->P2PCfg.P2pEventQueue.LastSetEvent);
    }
}

/*  
    ==========================================================================
    Description: 
        Be called everytime the RaUI Enable P2P function..
        
    Parameters: 
         
    Note:
         
    ==========================================================================
 */
VOID P2pEnable(
    IN PMP_ADAPTER pAd)
{
    PMP_PORT  pPort;
    UCHAR       module, i, j;
    UCHAR       ListenArray[3];

    ListenArray[0] = 1;
    ListenArray[1] = 6;
    ListenArray[2] = 11;

    pPort = pAd->PortList[FXXK_PORT_0];
        
    pPort->PortSubtype = PORTSUBTYPE_P2PClient;

    pPort->P2PCfg.P2PMode = P2P_MODE_WIN7;
    
    //Stop Scan. So set CounterAftrScanButton to MAX.
    pPort->P2PCfg.P2pCounter.CounterAftrScanButton = P2P_SCAN_PERIOD;
    // Reset MS WFD scan period (to MAX)
    pPort->P2PCfg.ScanPeriod = P2P_SCAN_PERIOD;

    pPort->P2PCfg.P2pCapability[0] = 0;
    pPort->P2PCfg.P2pCapability[1] = 0;
    
    pPort->P2PCfg.ListenChannel = P2P_DEFAULT_LISTEN_CHANNEL;
    j = 0;
    // Add 1, 6, 11 to ListenArray Array.
    for (i = 0;i < pAd->HwCfg.ChannelListNum;i++)
    {
        if (IS_SOCIAL_CHANNEL(pAd->HwCfg.ChannelList[i].Channel))
        {
            ListenArray[j] = pAd->HwCfg.ChannelList[i].Channel;
            j++;
        }
    }
    DBGPRINT(RT_DEBUG_TRACE, ("P2pEnable: how many social channels are in our supported channel ? Ans : %d !!! \n", j));

    if ( j > 0)
    {
        module = RandomByte(pAd)%j;
        if (module < 3)
            pPort->P2PCfg.ListenChannel = ListenArray[module];
        DBGPRINT(RT_DEBUG_TRACE, ("1: pPort->P2PCfg.ListenChannel: %d !!! \n", pPort->P2PCfg.ListenChannel));
    }
    else
    {
        module = RandomByte(pAd)%3;
        if (module < 3)
            pPort->P2PCfg.ListenChannel = ListenArray[module];
        DBGPRINT(RT_DEBUG_TRACE, ("2: pPort->P2PCfg.ListenChannel: %d !!! \n", pPort->P2PCfg.ListenChannel));
    }

    //Follow INFRA channel when PORT_0 is already connected
    if ((pAd->OpMode == OPMODE_STAP2P) && (INFRA_ON(pAd->PortList[PORT_0])))
    {
        pPort->P2PCfg.ListenChannel = pPort->Channel;
        DBGPRINT(RT_DEBUG_TRACE, ("3: !!Concurrent Mode!! pPort->P2PCfg.ListenChannel: %d !!! \n", pPort->P2PCfg.ListenChannel));
    }
        
    PlatformZeroMemory(pPort->P2PCfg.PhraseKey, 64);
    pPort->P2PCfg.PhraseKeyLen = sizeof(DEFAULTWPAPSKEY);
    pPort->P2PCfg.PhraseKeyChanged = FALSE;

    PlatformMoveMemory(pPort->P2PCfg.PhraseKey, DEFAULTWPAPSKEY, 8);
    // Always enable Invite.  Always enable intra-BSS. Because no reason to disable it. :)
    if (IS_P2P_SIGMA_OFF(pPort))
    {
        pPort->CommonCfg.P2pControl.field.EnableInvite = 1;
        pPort->CommonCfg.P2pControl.field.EnableIntraBss = 1;
    }
    P2PUpdateCapability(pAd, pPort);
    
    // Driver hard fixed to PC.
    PlatformMoveMemory(pPort->StaCfg.WscControl.RegData.EnrolleeInfo.PriDeviceType, Wsc_Pri_Dev_Type, 8);
    PlatformMoveMemory(pPort->StaCfg.WscControl.RegData.RegistrarInfo.PriDeviceType, Wsc_Pri_Dev_Type, 8);
    PlatformMoveMemory(pPort->P2PCfg.PriDeviceType, Wsc_Pri_Dev_Type, 8);

    pPort->P2PCfg.ConfigTimeout[0] = 40;
    pPort->P2PCfg.ConfigTimeout[1] = 30;

    pPort->CommonCfg.P2pControl.field.SwBasedNoA = 1;
    
    // Construct AP RSN_IE. 
    RTMPAPMakeRSNIE(pAd, pPort, Ralink802_11AuthModeWPA2PSK, Ralink802_11Encryption3Enabled);
    
    // Get correct Register setting.
    if (pAd->OpMode == OPMODE_STAP2P)
    {
        // Set multi-bssid
        MtAsicSetMultiBssidEx(pAd, pPort->CurrentAddress, (UCHAR)pPort->PortNumber);
        // win7 doesn't allow adhoc connection when virtula port is active. So doesn't check adhoc.
        if (INFRA_ON(pAd->PortList[PORT_0]))
        {
            P2pDecidePhyMode(pAd, pPort);
        }
        DBGPRINT(RT_DEBUG_TRACE, ("STA+P2P: PortNumber = %x. CurrentAddress = %x:%x:%x:%x:%x:%x !!! \n",
            pPort->P2PCfg.PortNumber, pPort->CurrentAddress[0], pPort->CurrentAddress[1],
            pPort->CurrentAddress[2], pPort->CurrentAddress[3], pPort->CurrentAddress[4], pPort->CurrentAddress[5]));
    }
    // default GO enable 11n
    P2pStopNoA(pAd, NULL);
    P2pStopOpPS(pAd, pPort);
    P2pScanChannelDefault(pAd, pPort);
    pPort->P2PCfg.P2PConnectState = P2P_CONNECT_IDLE;
    pPort->P2PCfg.P2PChannelState = P2P_DIFF_CHANNEL_OFF;
    P2pGotoIdle(pAd, pPort);
    P2pDefaultListenChannel(pAd, pPort);
    P2pSetListenIntBias(pAd, 2);
    pPort->P2PCfg.P2pCounter.CounterAfterConnectCommand = 300;
    pPort->P2PCfg.P2pCounter.Wpa2pskCounter100ms = 300;
    pPort->P2PCfg.P2pCounter.CounterAfterSetWscMode = 0;
    pPort->P2PCfg.bOppsOn = FALSE;
    pPort->P2PCfg.bNoAOn = FALSE;
    pPort->P2PCfg.bPendingNullFrame = FALSE;
    pPort->P2PCfg.CentralChannel = 0;// not used
    pPort->P2PCfg.P2pPhyMode = P2P_PHYMODE_ENABLE_11N_20;
    pPort->P2PCfg.bP2pAckReq = FALSE;
    pPort->P2PCfg.LinkState = 0;
    PlatformZeroMemory( pPort->P2PCfg.PinCode, 8);
    PlatformZeroMemory( pPort->StaCfg.WscControl.RegData.PIN, 8);
    pPort->StaCfg.WscControl.RegData.PINLen = 0;
    pPort->P2PCfg.EapolLastSecquence = 0xff;
    pPort->P2PCfg.P2pCounter.CounterP2PConnectionFail = 0xffffffff;
    pPort->P2PCfg.P2pCounter.CounterP2PAcceptReinvoke = 0xff;
    pPort->P2PCfg.P2pCounter.CounterAutoReconnectForP2pClient = 10;
    pPort->P2PCfg.ConnectingListenChannel = 0;

    // init wps supported version from registry
    if (pAd->StaCfg.WSCVersion2 >= 0x20)
        pPort->P2PCfg.WPSVersionsEnabled = 2;
    else
        pPort->P2PCfg.WPSVersionsEnabled = 1;


    // Reset MS WFD (exclusive mode)
#if(COMPILE_WIN8_ABOVE(CURRENT_OS_NDIS_VER))  
    pPort->P2PCfg.DeviceDiscoverable = DOT11_WFD_DEVICE_NOT_DISCOVERABLE;
#endif

    DBGPRINT(RT_DEBUG_TRACE, ("!5 OpStatusFlags = %x. CentralChannel = %d. Channel  = %d. !!! \n", pPort->CommonCfg.OpStatusFlags,pPort->CentralChannel,pPort->Channel));
}

VOID
P2pShutdown(
    IN PMP_ADAPTER pAd
    )
{
    //if (P2P_ON(pPort)) 
    {
#if(COMPILE_WIN8_ABOVE(CURRENT_OS_NDIS_VER))  
        {
            UCHAR   PortNum;
            PMP_PORT  pP2PPort = NULL;
            for(PortNum = 0; PortNum < RTMP_MAX_NUMBER_OF_PORT/*pAd->NumberOfPorts*/; PortNum++)
            {
                pP2PPort = (PMP_PORT) pAd->PortList[PortNum];
            
                if ((pP2PPort != NULL) && (pP2PPort->bActive == TRUE))
                {
                    if ((pP2PPort->PortType == WFD_DEVICE_PORT) || 
                        (pP2PPort->PortType == WFD_GO_PORT) || 
                        (pP2PPort->PortType == WFD_CLIENT_PORT))
                        P2pMsDown(pAd, pAd->PortList[pP2PPort->PortNumber]);
                }
            }
        }
#endif
#if 0
        if ((pAd->PortList[pPort->P2PCfg.PortNumber]) 
            && (pAd->PortList[pPort->P2PCfg.PortNumber]->bActive) 
            && (pAd->PortList[pPort->P2PCfg.PortNumber]->PortType == EXTSTA_PORT)
            && ((pAd->PortList[pPort->P2PCfg.PortNumber]->PortSubtype == PORTSUBTYPE_P2PClient) 
            || (pAd->PortList[pPort->P2PCfg.PortNumber]->PortSubtype == PORTSUBTYPE_P2PGO)))
        {
            P2pDown(pAd, pAd->PortList[pPort->P2PCfg.PortNumber]);

        }
        // disable p2p
        pPort->P2PCfg.P2PDiscoProvState = P2P_DISABLE;
#endif 
    }
}

/* 
    ==========================================================================
    Description:
        Reset P2P state machine. Not Disable.
    Return:

    NOTE:
         
    ==========================================================================
*/
NDIS_STATUS
    P2pDown(
    IN PMP_ADAPTER pAd,
    IN PMP_PORT   pPort
    )
{
    NDIS_STATUS ndisStatus = NDIS_STATUS_SUCCESS;
    BOOLEAN Cancelled = FALSE;

    if (pPort->PortNumber != pPort->P2PCfg.PortNumber)
        return ndisStatus;

    // Send disassociate frame to all of peers.
    P2pGroupMacTabDisconnect(pAd, pPort, 0);
    
    pPort->P2PCfg.P2PChannelState = P2P_DIFF_CHANNEL_OFF;
    pPort->Mlme.P2PMachine.CurrState = P2P_IDLE_STATE;  
    pPort->P2PCfg.Dpid = DEV_PASS_ID_NOSPEC;
    pPort->P2PCfg.GONoASchedule.bNeedResumeNoA = FALSE;
    pPort->SoftAP.ApCfg.bSoftAPReady = FALSE;

    // ==> WPS
    // Turn off WSC Actions before a new connection
    PlatformCancelTimer(&pPort->StaCfg.WscControl.WscConnectTimer, &Cancelled);
    PlatformCancelTimer(&pPort->StaCfg.WscControl.WscScanTimer, &Cancelled);
    PlatformCancelTimer(&pPort->StaCfg.WscControl.WscPBCTimer, &Cancelled);
    PlatformCancelTimer(&pPort->StaCfg.WscControl.WscPINTimer, &Cancelled);
    PlatformCancelTimer(&pPort->StaCfg.WscControl.WscEapRxTimer, &Cancelled);
    PlatformCancelTimer(&pPort->StaCfg.WscControl.WscM8WaitEapFailTimer, &Cancelled);
    PlatformCancelTimer(&pPort->StaCfg.WscControl.WscEapM2Timer, &Cancelled);
    // Cancel the WPS successful connection timer.
    PlatformCancelTimer(&pPort->StaCfg.WscControl.WscLEDTimer, &Cancelled);
    PlatformCancelTimer(&pPort->P2PCfg.P2pStopGoTimer, &Cancelled);
    PlatformCancelTimer(&pPort->P2PCfg.P2pStopClientTimer, &Cancelled);
    // Reset WscMode
    pPort->StaCfg.WscControl.WscConfMode = WSC_ConfMode_DISABLE;

    if (pPort->StaCfg.WscControl.WscState != WSC_STATE_OFF)
    {
        WscStop(pAd, pPort);
        if (!MT_TEST_FLAG(pAd, fRTMP_ADAPTER_HALT_IN_PROGRESS))
        {
            LedCtrlSetBlinkMode(pAd, LED_WPS_TURN_LED_OFF);
        }
    }
    // <== WPS

    // Set to 0xff means to connect to first Connecting MAC
    PlatformZeroMemory(pPort->P2PCfg.ConnectingMAC, MAX_P2P_GROUP_SIZE*6);
    pPort->P2PCfg.ConnectingIndex = 0xff;
    // clean temporarily credentials
    P2pCrednTabClean(pAd);

    // Init WPA state machine at Supplicant
    pAd->StaCfg.WpaState = SS_NOTUSE;

#if 0
    // Set the AutoReconnectSsid to prevent it reconnect to old SSID
    // Since calling this indicate user don't want to connect to that SSID anymore.
    // TODO: It might influence Port0 reconnection action in concurrent mode
    {
        pAd->MlmeAux.AutoReconnectSsidLen= 32;
        pAd->MlmeAux.AutoReconnectStatus = FALSE;
        PlatformZeroMemory(pAd->MlmeAux.AutoReconnectSsid, pAd->MlmeAux.AutoReconnectSsidLen);
        PlatformZeroMemory(pPort->PortCfg.Ssid, MAX_LEN_OF_SSID);
        pPort->PortCfg.SsidLen = 0;
    }
#endif

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
    

    // ==> P2P Event
    // Queue a P2PEVENT_GEN_SEUDO_ADHOC_PROFILE event.
    pPort->P2PCfg.P2pEventQueue.bDeleteWirelessCon = FALSE;   
    // Set Disable dhcp server event to let GUI to return IP setting from fixed to auto.
    pPort->P2PCfg.P2pEventQueue.bDisableDhcp = FALSE;
    pPort->P2PCfg.P2pEventQueue.bWpsNack = FALSE;
    pPort->P2PCfg.P2pEventQueue.bWpaDone = FALSE;

    pPort->P2PCfg.P2pCapability[1] &= (~GRPCAP_OWNER);
    pPort->P2PCfg.P2pCapability[1] &= (~GRPCAP_GROUP_FORMING);
    pPort->P2PCfg.P2pCapability[1] &= (~GRPCAP_LIMIT);

    if (pPort->PortSubtype == PORTSUBTYPE_P2PGO)
    {
        if (P2pSetEvent(pAd, pPort,  P2PEVENT_DISABLE_DHCP_SERVER) == FALSE)
            pPort->P2PCfg.P2pEventQueue.bDisableDhcp = TRUE;
    }
    
    if (IS_P2P_GO_OP(pPort) || (IS_P2P_GO_WPA2PSKING(pPort)))
    {
        // Always Queue a P2PEVENT_GEN_SEUDO_ADHOC_PROFILE event.
        // to notify GUI to close Finder application.
        pPort->P2PCfg.P2pEventQueue.bDeleteWirelessCon = FALSE;   
        // Queue Disable dhcp server event in STA mode.
        // In STAP2P mode, dhcp use ICS feature. so doesn't need to "disable dhcp"
        if (P2pSetEvent(pAd, pPort,  P2PEVENT_DISCONNECT) == FALSE)
        {
            pPort->P2PCfg.P2pEventQueue.bDeleteWirelessCon = TRUE;    
        }

        // GO in sta mode: update extra information to link is down
        pAd->UiCfg.ExtraInfo = GENERAL_LINK_DOWN;
    }
    
    // <== P2P Event
    
    pPort->P2PCfg.MyIp = 0;
    // Reset Sigma Parameters
    P2PInitSigmaParam(pAd, pPort);
    P2PDefaultConfigM(pAd, pPort);
    // Reset blocking 4-way assocation
    pPort->P2PCfg.P2pGOAllPsm = FALSE;
    pPort->P2PCfg.PhraseKeyChanged = FALSE;
    pPort->P2PCfg.bP2pAckReq = FALSE;
    pPort->P2PCfg.P2pCounter.CounterAfterSetWscMode = 0;
    
    // p2p network is down.
    pPort->P2PCfg.LinkState = 0;

#if UAPSD_AP_SUPPORT    
    // Record from WMM-PS GO, reset them
    pPort->P2PCfg.LastMoreDataFromGO[QID_AC_BE]   = FALSE;
    pPort->P2PCfg.LastMoreDataFromGO[QID_AC_BK]   = FALSE;
    pPort->P2PCfg.LastMoreDataFromGO[QID_AC_VI]   = FALSE;
    pPort->P2PCfg.LastMoreDataFromGO[QID_AC_VO]   = FALSE;
#endif      



    // Free Active Port Number to default port
    if ((pAd->OpMode == OPMODE_STAP2P) && 
        (pAd->ucActivePortNum != NDIS_DEFAULT_PORT_NUMBER))
    {
        pAd->ucActivePortNum = NDIS_DEFAULT_PORT_NUMBER;
        DBGPRINT(RT_DEBUG_TRACE, ("Reset ucActivePortNum to NDIS_DEFAULT_PORT_NUMBER\n"));
    }   

    DBGPRINT(RT_DEBUG_TRACE, ("1    P2pDown: portnum=%d. subtype=%d. connectstate= %s \n", 
        pPort->PortNumber, pPort->PortSubtype, decodeP2PState(pPort->P2PCfg.P2PConnectState)));
    
    if (IS_P2P_CONNECT_IDLE(pPort))
    {
        if(IS_P2P_NOT_AUTOGO(pPort))
        {
            // Remove Bssid after link down or group dissolve
            PlatformZeroMemory(pPort->PortCfg.Bssid, MAC_ADDR_LEN);
            PlatformZeroMemory(pPort->P2PCfg.Bssid, MAC_ADDR_LEN);
            PlatformZeroMemory(pPort->P2PCfg.GroupDevAddr, MAC_ADDR_LEN);
            // 
            PlatformMoveMemory(pPort->P2PCfg.SSID, WILDP2PSSID, WILDP2PSSIDLEN);
            pPort->P2PCfg.SSIDLen = WILDP2PSSIDLEN;

            pPort->P2PCfg.CentralChannel = 0;// not used
            pPort->P2PCfg.P2pPhyMode = P2P_PHYMODE_ENABLE_11N_20;
        }
        // Just stop the Role as GO and then exit.
        P2pStopGo(pAd, pAd->PortList[pPort->P2PCfg.PortNumber]);
        return ndisStatus;
    }
    
    if (IS_P2P_CLIENT_OP(pPort))
    {
        pAd->MlmeAux.AssocState = dot11_assoc_state_unauth_unassoc;
        //Indicate disassociate as I am Client
        PlatformIndicateDisassociation(pAd, pPort, pPort->PortCfg.Bssid, DOT11_ASSOC_STATUS_UNREACHABLE);

        //Set this flag and in MlmeCntLinkDown, the bssid will be deleted from scan table and
        //the station will not keep trying to connect to the old bssid.
        pAd->MlmeAux.CurrReqIsFromNdis = FALSE;
        MlmeCntLinkDown(pPort, FALSE);
    }

    //Also clear Mac Table. GO's entry id starts from 2 and Client entry id is 1.
    MacTableReset(pAd, pPort->P2PCfg.PortNumber);
    // P2PStropGO() will check if I am Really GO. Then perform the StopGo function. 
    P2pStopGo(pAd, pAd->PortList[pPort->P2PCfg.PortNumber]);
    
    // Although StopGO might call this two stop function. But the following two stop function is
    // for when I am Client. So still call these two functions here.
    P2pStopNoA(pAd, NULL);
    P2pStopOpPS(pAd, pPort);

    // Intent 16 means I am autoGO. If I am Auto GO. I don't need disable Sync.
    if (IS_P2P_NOT_AUTOGO(pPort))
    {
            
        
        // Remove Bssid after link down or group dissolve
        PlatformZeroMemory(pPort->PortCfg.Bssid, MAC_ADDR_LEN);
        PlatformZeroMemory(pPort->P2PCfg.Bssid, MAC_ADDR_LEN);
        PlatformZeroMemory(pPort->P2PCfg.GroupDevAddr, MAC_ADDR_LEN);
        // 
        PlatformMoveMemory(pPort->P2PCfg.SSID, WILDP2PSSID, WILDP2PSSIDLEN);
        pPort->P2PCfg.SSIDLen = WILDP2PSSIDLEN;

        pPort->P2PCfg.CentralChannel = 0;// not used
        pPort->P2PCfg.P2pPhyMode = P2P_PHYMODE_ENABLE_11N_20;
        
        if (!MT_TEST_FLAG(pAd, fRTMP_ADAPTER_NIC_NOT_EXIST))
        {
            pPort->bBeaconing = FALSE;
            MlmeUpdateBeacon(pAd);
        }
        pPort->P2PCfg.P2PConnectState = P2P_CONNECT_IDLE;

        // Rest to BW=20MHz, Channel = ListenChannel
        P2pDefaultListenChannel(pAd, pPort);
    }
    DBGPRINT(RT_DEBUG_TRACE, ("2    P2pDown:  %s \n", decodeP2PState(pPort->P2PCfg.P2PConnectState)));                

    return ndisStatus;
}

VOID P2pMlmeCntLinkUp(
    IN PMP_ADAPTER pAd,
    IN PMP_PORT pPort)
{
    ULONG       MACValue;
    ULONG       MACValue2;
    MAC_DW0_STRUC StaMacReg0;
    MAC_DW1_STRUC StaMacReg1;
    UCHAR       Wcid = (UCHAR)MlmeSyncGetWlanIdxByPort(pPort, ROLE_WLANIDX_BSSID);
    UCHAR       Index = 0;
    BOOLEAN     StopGroup = FALSE;
    ULONG       GroupFormErrCode = 0;
    PMAC_TABLE_ENTRY pEntry = MlmeSyncMacTabMatchedRoleType(pPort, ROLE_WLANIDX_P2P_CLIENT);
    PMAC_TABLE_ENTRY pWcidMacTabEntry = NULL;

    if(pEntry == NULL)
    {
        DBGPRINT(RT_DEBUG_TRACE,("%s(), Line:%d, Mac Entry is NULL  Wcid = %d\n", __FUNCTION__, __LINE__, Wcid));
        return;
    }
    
    if (P2P_OFF(pPort))
    {
        DBGPRINT(RT_DEBUG_TRACE, ("P2pMlmeCntLinkUp return. P2P is OFF  !!! \n"));
        return;
    }
    // P2P and SoftapAP/VwifiAP can not co-exist.
    if (pAd->OpMode == OPMODE_AP || pAd->OpMode == OPMODE_APCLIENT)
    {
        DBGPRINT(RT_DEBUG_TRACE, ("P2pMlmeCntLinkUp return. Non P2P Opmode \n"));
        return;
    }

    P2PPrintState(pAd);

    //
    // If this is Managed AP . check whether we need to do something.
    //
    if ((pAd->OpMode == OPMODE_STAP2P) && 
        (pAd->PortList[pPort->P2PCfg.PortNumber]->PortSubtype == PORTSUBTYPE_P2PGO))
    {
        DBGPRINT(RT_DEBUG_TRACE, ("As  STA in a BSS, INFRA AP assign WMM parm are ::    !! \n"));
        DBGPRINT(RT_DEBUG_TRACE, ("EdcaParm ===>   !! \n"));
        DBGPRINT(RT_DEBUG_TRACE, (" Cwmin= %x !!Cwmax= %x !!  \n", *(PULONG)&pPort->CommonCfg.APEdcaParm.Cwmin[0], *(PULONG)&pPort->CommonCfg.APEdcaParm.Cwmax[0]));
        DBGPRINT(RT_DEBUG_TRACE, (" Txop= %x !! Aifsn= %x !!\n", *(PULONG)&pPort->CommonCfg.APEdcaParm.Txop[0], *(PULONG)&pPort->CommonCfg.APEdcaParm.Aifsn[0]));

        DBGPRINT(RT_DEBUG_TRACE, ("GO Bss EdcaParm setting are ::   !! \n"));
        DBGPRINT(RT_DEBUG_TRACE, (" Cwmin= %x !!  Cwmax= %x !! \n", *(PULONG)&pPort->SoftAP.ApCfg.BssEdcaParm.Cwmin[0], *(PULONG)&pPort->SoftAP.ApCfg.BssEdcaParm.Cwmax[0]));
        DBGPRINT(RT_DEBUG_TRACE, (" Txop= %x !!Aifsn= %x !! \n", *(PULONG)&pPort->SoftAP.ApCfg.BssEdcaParm.Txop[0], *(PULONG)&pPort->SoftAP.ApCfg.BssEdcaParm.Aifsn[0]));

        P2pEdcaDefault(pAd, pPort);
    }
    if (INFRA_ON(pAd->PortList[PORT_0]) &&
        (PlatformEqualMemory(pAd->PortList[PORT_0]->PortCfg.Bssid, pPort->P2PCfg.P2pManagedParm.ManageAPBSsid, MAC_ADDR_LEN)) &&
        (pAd->PortList[pPort->P2PCfg.PortNumber]->PortSubtype == PORTSUBTYPE_P2PGO))
    {
        // If this is a manged AP. update to the ManageAPBSsid.
        PlatformMoveMemory(pPort->P2PCfg.P2pManagedParm.ManageAPSsid, pAd->PortList[PORT_0]->PortCfg.Ssid, 32);
        pPort->P2PCfg.P2pManagedParm.ManageAPSsidLen = pAd->PortList[PORT_0]->PortCfg.SsidLen;
        DBGPRINT(RT_DEBUG_TRACE, ("  -I am associated with a Managed AP.  \n"));
        DBGPRINT(RT_DEBUG_TRACE, ("!P2pMlmeCntLinkUp!!  Record This SSID because this is a managed AP. SsidLen = %d .\n",  pPort->P2PCfg.P2pManagedParm.ManageAPSsidLen));

        // the AP enables Managed function. (bit 0  == 1)
        if ((pPort->P2PCfg.P2pManagedParm.APP2pManageability != 0xff) 
            && (MT_TEST_BIT(pPort->P2PCfg.P2pManagedParm.APP2pManageability, P2PMANAGED_ENABLE_BIT)))
        {
            DBGPRINT(RT_DEBUG_TRACE, ("  -this Managed AP enables Managed.  \n"));
            // If bit 1 is set. it means AP allows the P2P to enable Cross Connect.
            if (MT_TEST_BIT(pPort->P2PCfg.P2pManagedParm.APP2pManageability, P2PMANAGED_ICS_ENABLE_BIT))
            {
                DBGPRINT(RT_DEBUG_TRACE, ("  -this Managed AP enables permits cross connect.  \n"));
            }
            // If bit 1 is not set. it means AP disallows the P2P Cross Connect.
            else
            {
                DBGPRINT(RT_DEBUG_TRACE, ("  -this Managed AP enables disallows cross connect!!Our ICSStatus = %d.   \n", pPort->P2PCfg.P2pManagedParm.ICSStatus));
                pPort->P2PCfg.P2pCapability[1] &= (~GRPCAP_CROSS_CONNECT);
                // We should disable ICS in concurrent mode nowif the ICS is currently enabled by use.
                if ((pPort->P2PCfg.P2pManagedParm.ICSStatus == ICS_STATUS_ENABLED)
                    && (pAd->OpMode == OPMODE_STAP2P))
                {
                    pPort->P2PCfg.P2pEventQueue.bDisableIcs = FALSE;
                    if (FALSE == P2pSetEvent(pAd, pPort,  P2PEVENT_DISABLE_ICS))
                        pPort->P2PCfg.P2pEventQueue.bDisableIcs = TRUE;
                }
            }
            DBGPRINT(RT_DEBUG_TRACE, ("-I am connecting to a Managed AP. Save this Ssid. \n"));
        }
        // If this managed AP set coexist is not optional. I must follow this.
        if (!MT_TEST_BIT(pPort->P2PCfg.P2pManagedParm.APP2pManageability, P2PMANAGED_COEXIST_OPT_BIT))
        {
            PlatformMoveMemory(&pPort->SoftAP.ApCfg.BssEdcaParm, &pPort->CommonCfg.APEdcaParm, sizeof(EDCA_PARM));
            DBGPRINT(RT_DEBUG_TRACE, ("2     !! \n"));
            DBGPRINT(RT_DEBUG_TRACE, ("EdcaParm ===>   !! \n"));
            DBGPRINT(RT_DEBUG_TRACE, (" Cwmin= %x !! \n", *(PULONG)&pPort->CommonCfg.APEdcaParm.Cwmin[0]));
            DBGPRINT(RT_DEBUG_TRACE, (" Cwmax= %x !! \n", *(PULONG)&pPort->CommonCfg.APEdcaParm.Cwmax[0]));
            DBGPRINT(RT_DEBUG_TRACE, (" Txop= %x !! \n", *(PULONG)&pPort->CommonCfg.APEdcaParm.Txop[0]));
            DBGPRINT(RT_DEBUG_TRACE, (" Aifsn= %x !! \n", *(PULONG)&pPort->CommonCfg.APEdcaParm.Aifsn[0]));

            DBGPRINT(RT_DEBUG_TRACE, ("AP Bss EdcaParm ===>   !! \n"));
            DBGPRINT(RT_DEBUG_TRACE, (" Cwmin= %x !! \n", *(PULONG)&pPort->SoftAP.ApCfg.BssEdcaParm.Cwmin[0]));
            DBGPRINT(RT_DEBUG_TRACE, (" Cwmax= %x !! \n", *(PULONG)&pPort->SoftAP.ApCfg.BssEdcaParm.Cwmax[0]));
            DBGPRINT(RT_DEBUG_TRACE, (" Txop= %x !! \n", *(PULONG)&pPort->SoftAP.ApCfg.BssEdcaParm.Txop[0]));
            DBGPRINT(RT_DEBUG_TRACE, (" Aifsn= %x !! \n", *(PULONG)&pPort->SoftAP.ApCfg.BssEdcaParm.Aifsn[0]));
            P2pMakeBssBeacon(pAd, pAd->PortList[pPort->P2PCfg.PortNumber]);
        }
    }
    
    // Set this value to very large when linkup, so is like to disable the listening function. 
    pPort->P2PCfg.P2pCounter.NextScanRound = P2P_SCAN_PERIOD;
    // 
    P2PPrintState(pAd);
    // Listening function is to switch to listening channel listening.
    if ((pAd->OpMode == OPMODE_STAP2P) && (IS_P2P_CONNECT_IDLE(pPort)))
    {
        pPort->P2PCfg.GroupChannel = pPort->Channel;
        pPort->P2PCfg.GroupOpChannel = pPort->Channel;
        // we can change listen channel to this working channel.
        pPort->P2PCfg.ListenChannel = pPort->Channel;
        DBGPRINT(RT_DEBUG_TRACE, ("!P2pMlmeCntLinkUp!!Follow INFRA channel setting.becomes  ListenChannel = %d.  GroupChannel = %d.!!!!!!!!! \n", pPort->P2PCfg.ListenChannel, pPort->P2PCfg.GroupChannel));
    }
    else
        pPort->P2PCfg.ListenChannel = pPort->Channel;
    
    // Case 1: P2P network(Port2) is earlier than WlanSta network(Port0)
    //
    if ((pAd->OpMode == OPMODE_STAP2P) && 
        (IS_P2P_GO_OP(pPort) || IS_P2P_CLIENT_OP(pPort) || IS_P2P_AUTOGO(pPort)) &&
        (pPort->PortNumber == PORT_0))
    {
        // If WlanSta start an adhoc network, need to stop Group.
        if (ADHOC_ON(pPort) && (pAd->StaCfg.BssType == BSS_ADHOC))
        {
            StopGroup = TRUE;
            GroupFormErrCode = P2P_ERRCODE_ADHOCON;
        }   
        // If WlanSta and P2pCli/P2pGO use different channels, need to stop Group.
        else if (pPort->P2PCfg.GroupOpChannel != pPort->Channel)
        {
            StopGroup = TRUE;
            GroupFormErrCode = P2P_ERRCODE_CHANNELCONFLICT;
        }
        // If WlanSta and P2pCli use different PhyMode, need to stop Group.
        // GO can coexist between 11N-20MHz and BG mode. 
        else if ((IS_P2P_CLIENT_OP(pPort)) &&
                (pPort->P2PCfg.P2pPhyMode == P2P_PHYMODE_LEGACY_ONLY) && (pAd->StaActive.SupportedHtPhy.bHtEnable == TRUE))
        {
            StopGroup = TRUE;
            GroupFormErrCode = P2P_ERRCODE_PHYMODECONFLICT;
        }   
        else if (IS_P2P_CLIENT_OP(pPort) &&
                (pPort->P2PCfg.P2pPhyMode != P2P_PHYMODE_LEGACY_ONLY) && (pAd->StaActive.SupportedHtPhy.bHtEnable == FALSE))
        {
            StopGroup = TRUE;
            GroupFormErrCode = P2P_ERRCODE_PHYMODECONFLICT;
        }       
        // If WlanSta and P2pCli/P2pGO use different central channels, need to stop Group.
        else if ((IS_P2P_CLIENT_OP(pPort) || IS_P2P_GO_OP(pPort) || IS_P2P_AUTOGO(pPort)) &&
            (pPort->P2PCfg.CentralChannel != pPort->CentralChannel) && 
            (pPort->P2PCfg.CentralChannel != 0))
        {
            StopGroup = TRUE;
            GroupFormErrCode = P2P_ERRCODE_CHANNELCONFLICT;
        }   
        // The control channel is the same.
        else if (pPort->P2PCfg.GroupOpChannel == pPort->Channel)
        {       
            DBGPRINT(RT_DEBUG_TRACE, ("Same control channel  = %d.!!!!!!!!! \n", pPort->P2PCfg.GroupOpChannel));
            StopGroup = FALSE;
            if (pAd->PortList[pPort->P2PCfg.PortNumber]->PortSubtype == PORTSUBTYPE_P2PGO)
            {
                if ((pAd->StaActive.SupportedHtPhy.HtChannelWidth == 1) && (pAd->StaActive.SupportedHtPhy.bHtEnable == TRUE))
                {
                    if ((pAd->StaActive.SupportedHtPhy.ExtChanOffset == pPort->SoftAP.ApCfg.AddHTInfoIe.AddHtInfo.ExtChanOffset))
                    {
                        DBGPRINT(RT_DEBUG_TRACE, ("GO keeps as 40MHz !ExtChanOffset = %d \n", pAd->StaActive.SupportedHtPhy.ExtChanOffset ));
                    }
                    else if (pPort->SoftAP.ApCfg.HtCapability.HtCapInfo.ChannelWidth == 0)
                    {
                        DBGPRINT(RT_DEBUG_TRACE, ("INFRA use 40MHz. GO keeps as 20MHz ! \n"));
                    }
                    else
                    {   // Might not happen, we already filter out different BW before.
                        DBGPRINT(RT_DEBUG_TRACE, ("(Might not happen) 2nd channel not match.\n"));
                    }
                }
                else
                {
                    DBGPRINT(RT_DEBUG_TRACE, ("INFRA use 20MHz. So GO Use 20MHz too !! \n"));
                }
            }
        }

        if (StopGroup == TRUE)
        {
            DBGPRINT(RT_DEBUG_TRACE, ("Case 1: Resume group due to different channel or phymode or adhoc(control=%d %d. central=%d %d. P2pPhyMode =%d. bHtEnable=%d. adhoc=%d, GroupFormErrCode=%d)\n", 
                pPort->P2PCfg.GroupOpChannel, pPort->Channel, pPort->P2PCfg.CentralChannel, pPort->CentralChannel,
                pPort->P2PCfg.P2pPhyMode, pAd->StaActive.SupportedHtPhy.bHtEnable, ADHOC_ON(pPort), GroupFormErrCode));

            // If I am autoGo and no clinet connects to me, don't indicate error message.
            if (IS_P2P_AUTOGO(pPort) && 
                (pAd->PortList[pPort->P2PCfg.PortNumber]->PortSubtype == PORTSUBTYPE_P2PGO) &&
                (INFRA_ON(pAd->PortList[PORT_0])) && 
                (pPort->MacTab.Size <= 1) &&
                (GroupFormErrCode != P2P_ERRCODE_ADHOCON))
            {
                GroupFormErrCode = 0;
            }
            if (GroupFormErrCode != 0)
            {
                pPort->P2PCfg.GroupFormErrCode = GroupFormErrCode;
                pPort->P2PCfg.P2pEventQueue.bErrorCode = TRUE;
            }

            // Resume my group without clients
            if (IS_P2P_AUTOGO(pPort) && 
                (pAd->PortList[pPort->P2PCfg.PortNumber]->PortSubtype == PORTSUBTYPE_P2PGO) &&
                (GroupFormErrCode != P2P_ERRCODE_ADHOCON))
            {
                if (P2pSetEvent(pAd, pPort,  P2PEVENT_DISCONNECT) == FALSE)
                    pPort->P2PCfg.P2pEventQueue.bDeleteWirelessCon = TRUE;    
                
                P2PResumeGroup(pAd, pAd->PortList[pPort->P2PCfg.PortNumber], P2P_IS_GO, FALSE);
            }
            // Dissolve my group
            else
            {
                // I must set GoIntentIdx to a vlaue !=16 to dissolve my group.
                pPort->P2PCfg.GoIntentIdx = 5;
                P2pDown(pAd, pAd->PortList[pPort->P2PCfg.PortNumber]);
                // Delete P2P Device that I previously tried to connect.
                for (Index = 0; Index < MAX_P2P_GROUP_SIZE; Index++)
                {
                    if (pAd->pP2pCtrll->P2PTable.Client[Index].P2pClientState > P2PSTATE_DISCOVERY_UNKNOWN)
                        P2pGroupTabDelete(pAd, Index, pAd->pP2pCtrll->P2PTable.Client[Index].addr);
                }
                P2pGotoScan(pAd, pPort, P2P_DISCO_TYPE_DEFAULT, 2);
            }
            
            return;
        }
        
    }

    if (IS_P2P_CONNECT_IDLE(pPort) ||
        IS_P2P_REGISTRA(pPort) ||
        IS_P2P_GO_WPA2PSKING(pPort) ||
        IS_P2P_GO_OP(pPort) ||
        (pPort->P2PCfg.PortNumber != pPort->PortNumber))
    {
        // If WlanSta start an adhoc network and p2p is in idle mode, tell UI to disable p2p.
        if (ADHOC_ON(pPort) && (pAd->StaCfg.BssType == BSS_ADHOC))
        {
            pPort->P2PCfg.GroupFormErrCode = P2P_ERRCODE_ADHOCON;
            pPort->P2PCfg.P2pEventQueue.bErrorCode = TRUE;
        }   

        DBGPRINT(RT_DEBUG_TRACE, ("P2P link up return. state=%s. port number=%d  %d \n", decodeP2PState(pPort->P2PCfg.P2PConnectState), pPort->P2PCfg.PortNumber, pPort->PortNumber));
        return;
    }

    // Use the owner of P2P wcid in concurrent Client if this entry has inserted
    if ((IS_P2P_CON_CLI(pAd, pPort)) && pEntry->ValidAsCLI)
        Wcid = (UCHAR)MlmeSyncGetWlanIdxByPort(pPort, ROLE_WLANIDX_P2P_CLIENT);


    pWcidMacTabEntry = MlmeSyncMacTabMatchedWlanIdx(pPort, Wcid); 

    if(pWcidMacTabEntry == NULL)
    {
        DBGPRINT(RT_DEBUG_TRACE,("%s(), Line:%d, Mac Entry is NULL  Wcid = %d\n", __FUNCTION__, __LINE__, Wcid));
        return;
    }
    
    // P2P_I_AM_CLIENT_ASSOC_AUTH state alreayd implies opmode is opmode_sta. So doesn't need to check opmode.
    if (pPort->P2PCfg.P2PConnectState == P2P_I_AM_CLIENT_ASSOC_AUTH)
    {
        pPort->P2PCfg.P2PConnectState = P2P_I_AM_CLIENT_OP;
        // Reset Scan Counter to prevent go to P2P scan during the following 4-way handshake.
        pPort->P2PCfg.P2pCounter.NextScanRound = 60;
        pPort->P2PCfg.P2pCounter.Counter100ms = 0;
        // Init the CTWindows value.  Later when I get the beacon, 
        // This CTWindows will be set to the value as specified in beacon.
        // See P2pHandleNoAAttri()
        pPort->P2PCfg.CTWindows = 0;
        // Stop do P2P search.
        pPort->P2PCfg.P2pCounter.CounterAftrScanButton = P2P_SCAN_PERIOD;
    }
    else if (pPort->P2PCfg.P2PConnectState == P2P_DO_WPS_ENROLLEE 
        || (pPort->P2PCfg.P2PConnectState == P2P_DO_WPS_ENROLLEE_DONE))
    {
        // Reset Scan Counter to prevent go to P2P scan during the following 4-way handshake.
        pPort->P2PCfg.P2pCounter.NextScanRound = 80;
        pPort->P2PCfg.P2pCounter.Counter100ms = 0;
    }

    if (pPort->P2PCfg.P2PConnectState == P2P_DO_WPS_ENROLLEE)
    {
        PlatformMoveMemory(pPort->P2PCfg.SSID, pPort->PortCfg.Ssid, 32);
        pPort->P2PCfg.SSIDLen = pPort->PortCfg.SsidLen;
    }
    
    //Need to write BSSID with current GO address. need HW update TSF Timer.
    if (pAd->OpMode == OPMODE_STA)
    {
        StaMacReg1.word = 0;
        StaMacReg0.field.Byte0 = pAd->PortList[PORT_0]->PortCfg.Bssid[0];
        StaMacReg0.field.Byte1 = pAd->PortList[PORT_0]->PortCfg.Bssid[1];
        StaMacReg0.field.Byte2 = pAd->PortList[PORT_0]->PortCfg.Bssid[2];
        StaMacReg0.field.Byte3 = pAd->PortList[PORT_0]->PortCfg.Bssid[3];
        StaMacReg1.field.Byte4 = pAd->PortList[PORT_0]->PortCfg.Bssid[4];
        StaMacReg1.field.Byte5 = (pAd->PortList[PORT_0]->PortCfg.Bssid[5]);
    //  StaMacReg1.field.U2MeMask = 0x1;
        RTUSBWriteMACRegister(pAd, MAC_BSSID_DW0, StaMacReg0.word);
        RTUSBWriteMACRegister(pAd, MAC_BSSID_DW1, StaMacReg1.word);
    }
    
    // Default is 11N-20, update p2p phy mode once link-up
    if ((pPort->Channel != pPort->CentralChannel) && 
        (pPort->P2PCfg.P2pPhyMode != P2P_PHYMODE_ENABLE_11N_40))
    {
        pPort->P2PCfg.P2pPhyMode = P2P_PHYMODE_ENABLE_11N_40;
    }   
    else if((pPort->Channel == pPort->CentralChannel) && 
        (pAd->StaActive.SupportedHtPhy.bHtEnable == FALSE) &&
        (pPort->P2PCfg.P2pPhyMode != P2P_PHYMODE_LEGACY_ONLY))
    {
        pPort->P2PCfg.P2pPhyMode = P2P_PHYMODE_LEGACY_ONLY;
    }
    
    // Save the current central channel as P2P's Port linkup
    if (pPort->P2PCfg.P2pPhyMode == P2P_PHYMODE_ENABLE_11N_40)
        pPort->P2PCfg.CentralChannel = pPort->CentralChannel;
    else
        pPort->P2PCfg.CentralChannel = pPort->Channel;
    
    DBGPRINT(RT_DEBUG_TRACE, ("!!!P2pMlmeCntLinkUp. State = %s !!!WscState = %d. P2pPhyMode = %d\n", decodeP2PState(pPort->P2PCfg.P2PConnectState), pPort->StaCfg.WscControl.WscState, pPort->P2PCfg.P2pPhyMode));
    //DBGPRINT(RT_DEBUG_TRACE, ("ClientFlags = [1]  = %x. [2] = %x. [3] = %x  \n", pPort->MacTab.Content[1].ClientStatusFlags,  pPort->MacTab.Content[2].ClientStatusFlags,  pPort->MacTab.Content[3].ClientStatusFlags));
    //DBGPRINT(RT_DEBUG_TRACE, ("!!!pAd Flags = %x !!!GroupOpChannel = %d.  CommonCfg.Channel = %d. P2PCfg.CentralChannel=%d\n", pAd->Flags, pPort->P2PCfg.GroupOpChannel, pPort->CommonCfg.Channel, pPort->P2PCfg.CentralChannel));    

    RTUSBReadMACRegister(pAd, 0x1808, &MACValue);
    RTUSBReadMACRegister(pAd, 0x180c, &MACValue2);
    DBGPRINT(RT_DEBUG_TRACE, ("!!!0x1808/0c = %x %x !!! \n", MACValue, MACValue2));
    RTUSBReadMACRegister(pAd, 0x1810, &MACValue);
    RTUSBReadMACRegister(pAd, 0x1814, &MACValue2);
    DBGPRINT(RT_DEBUG_TRACE, ("!!!0x1810/14 = %x %x !!! \n", MACValue, MACValue2));
    pWcidMacTabEntry->ValidAsP2P = TRUE;
    pPort->P2PCfg.P2pCapability[0] &= (~DEVCAP_DEVICE_LIMIT);
    pPort->P2PCfg.MyGOwcid = Wcid;

    // tell GUI I am associated with p2p network.
    pPort->P2PCfg.LinkState = Ralink802_11NetworkTypeP2P;

    if(pPort->CommonCfg.P2pControl.field.EnablePresistent == 1)
    {
        // reset retry counter for p2p persistent client
        pPort->P2PCfg.P2pCounter.CounterAutoReconnectForP2pClient = 0;

        // keep GO addr
        PlatformMoveMemory(pPort->P2PCfg.PersistentGOAddr, pPort->P2PCfg.Bssid, MAC_ADDR_LEN);
    }

}

VOID P2pMlmeCntLinkDown(
    IN PMP_ADAPTER pAd,
    IN PMP_PORT    pPort,
    IN  BOOLEAN      IsReqFromAP)
{
    UCHAR       p2pindex;
    UCHAR       Wcid = (UCHAR)MlmeSyncGetWlanIdxByPort(pPort, ROLE_WLANIDX_BSSID);
    PMAC_TABLE_ENTRY pEntry = MlmeSyncMacTabMatchedRoleType(pPort, ROLE_WLANIDX_P2P_CLIENT);
    PMAC_TABLE_ENTRY pWcidMacTabEntry = NULL;

    if(pWcidMacTabEntry == NULL)
    {
        DBGPRINT(RT_DEBUG_TRACE,("%s(), Line:%d, Mac Entry is NULL  Wcid = %d\n", __FUNCTION__, __LINE__, Wcid));
        return;
    }

    if (P2P_OFF(pPort))
    {
        DBGPRINT(RT_DEBUG_TRACE, ("P2pMlmeCntLinkDown return. P2P is OFF  !!! \n"));
        return;
    }
    // P2P and SoftapAP/VwifiAP can not co-exist.
    if (pAd->OpMode == OPMODE_AP || pAd->OpMode == OPMODE_APCLIENT)
    {
        DBGPRINT(RT_DEBUG_TRACE, ("P2pMlmeCntLinkDown return. Non P2P Opmode \n"));
        return;
    }

    if(pPort->P2PCfg.PortNumber != pPort->PortNumber)
    {
        // Restore ListenChannel to a social channel when existing ListenChannel is not a social channel and port 2 is not operating
        if((!IS_SOCIAL_CHANNEL(pPort->P2PCfg.ListenChannel)) && (!IS_P2P_OP(pPort)) && (IS_P2P_SIGMA_OFF(pPort)) && (pAd->LogoTestCfg.OnTestingWHQL == FALSE))
        {
            if (MlmeSyncIsValidChannel(pAd, 11))
                pPort->P2PCfg.ListenChannel = 11;
            else if (MlmeSyncIsValidChannel(pAd, 6))
                pPort->P2PCfg.ListenChannel = 6;
            else if (MlmeSyncIsValidChannel(pAd, 1))
                pPort->P2PCfg.ListenChannel = 1;
            P2pDefaultListenChannel(pAd, pPort);
        }
        DBGPRINT(RT_DEBUG_TRACE, ("P2pMlmeCntLinkDown return. Different port number (%d %d) \n", pPort->P2PCfg.PortNumber, pPort->PortNumber));
        return;
    }

    // Use the owner of P2P wcid in concurrent Client if this entry has inserted
    if ((IS_P2P_CON_CLI(pAd, pPort)) && (pEntry->ValidAsCLI))
        Wcid = (UCHAR)MlmeSyncGetWlanIdxByPort(pPort, ROLE_WLANIDX_P2P_CLIENT);
    
    pWcidMacTabEntry = MlmeSyncMacTabMatchedWlanIdx(pPort, Wcid);

    if(pWcidMacTabEntry == NULL)
    {
        DBGPRINT(RT_DEBUG_TRACE,("%s(), Line:%d, Mac Entry is NULL  Wcid = %d\n", __FUNCTION__, __LINE__, Wcid));
        return;
    }
    
    // Set This counter to init value. So If P2P listening is enabled, we will keep doing P2P listening after link down.
    // While connected, we disable listening function.
    pPort->P2PCfg.P2pCounter.Counter100ms = 0;
    pPort->P2PCfg.P2pCounter.NextScanRound = (RandomByte(pAd) % P2P_RANDOM_BASE) + P2P_RANDOM_BIAS;
    pPort->P2PCfg.Dpid = DEV_PASS_ID_NOSPEC;

    P2pDefaultListenChannel(pAd, pPort);
    
    pWcidMacTabEntry->ValidAsP2P = FALSE;

    // When I am GO in concurrent mode, do't need to call link down from here.
    if ((pAd->OpMode == OPMODE_STAP2P) && 
        (pAd->PortList[pPort->P2PCfg.PortNumber]->PortSubtype == PORTSUBTYPE_P2PGO))
    {
        P2pScanChannelDefault(pAd, pPort);
        DBGPRINT(RT_DEBUG_TRACE, ("!!!GroupOpChannel = %d.  !!! \n", pPort->P2PCfg.GroupOpChannel));

        DBGPRINT(RT_DEBUG_TRACE, ("!!!P2pMlmeCntLinkDown. Return when in concurrent mode . State = %s !!! \n", decodeP2PState(pPort->P2PCfg.P2PConnectState)));
        return;
    }
    // P2P_CONNECT_IDLE may means P2P is OFF.  Or this is not P2P Client linkdown. So return.
    if (pPort->P2PCfg.P2PConnectState == P2P_CONNECT_IDLE)
        return;

    // Reset Sigma Parameters
    P2PInitSigmaParam(pAd, NULL);


    if (pPort->P2PCfg.P2pEventQueue.bWpsDone == TRUE)
        pPort->P2PCfg.P2pEventQueue.bWpsDone = FALSE;
    if(pPort->P2PCfg.P2pEventQueue.bWpaDone == TRUE)
        pPort->P2PCfg.P2pEventQueue.bWpaDone = FALSE;

    P2pStopNoA(pAd, NULL);
    P2pStopOpPS(pAd, pPort);
    // P2P_I_AM_CLIENT_OP state already means in OPMODE_STA. so doesn't need to check.
    // Doesn't need to do this when linkdown() is called after WPS done, 
    if (IS_P2P_CLIENT_OP(pPort))
    {
        // go back to idle mode. If GUI want to stops P2P function, will call P2P OID to stop. 
        // So driver doesn't need to stop P2P function in Linkdown().
        if (IS_P2P_CLIENT_OP(pPort))
        {
            if (IsReqFromAP == TRUE)
            {
                pPort->P2PCfg.P2pEventQueue.bDeleteWirelessCon = TRUE;
                DBGPRINT(RT_DEBUG_TRACE, ("!!!P2pMlmeCntLinkDown. Send P2PEVENT_DISCONNECT . State = %s !!! \n", decodeP2PState(pPort->P2PCfg.P2PConnectState)));
                p2pindex = P2pGroupTabSearch(pAd, pPort->P2PCfg.Bssid);
                if (p2pindex < MAX_P2P_GROUP_SIZE)
                {
                    pAd->pP2pCtrll->P2PTable.Client[p2pindex].P2pClientState = P2PSTATE_DISCOVERY;
                    pAd->pP2pCtrll->P2PTable.Client[p2pindex].Rule = P2P_IS_CLIENT;
                }
            }
            P2pGroupTabDelete(pAd, MAX_P2P_GROUP_SIZE, pPort->P2PCfg.Bssid);
        }
        // Init SSID to P2P wildcard SSID.
        PlatformMoveMemory(pPort->P2PCfg.SSID, WILDP2PSSID, WILDP2PSSIDLEN);
        pPort->P2PCfg.P2pEventQueue.bP2pTopologyUpdate = TRUE;
        pPort->P2PCfg.MyIp = 0;
        // P2P link down should disable autoreconnect.
        pAd->MlmeAux.AutoReconnectSsidLen= 32;
        pAd->MlmeAux.AutoReconnectStatus = FALSE;
        PlatformZeroMemory(pAd->MlmeAux.AutoReconnectSsid, pAd->MlmeAux.AutoReconnectSsidLen);

        if (IS_P2P_SIGMA_OFF(pPort))
        {
            pPort->P2PCfg.P2pCounter.CounterAftrScanButton = 0;
            pPort->P2PCfg.P2PConnectState = P2P_CONNECT_IDLE;
            P2pGotoScan(pAd, pPort, P2P_DISCO_TYPE_DEFAULT, 2);
            pPort->P2PCfg.Dpid = DEV_PASS_ID_NOSPEC;
        }
        DBGPRINT(RT_DEBUG_TRACE, ("!!!P2pMlmeCntLinkDown. State turn to = %s !!! \n", decodeP2PState(pPort->P2PCfg.P2PDiscoProvState)));
    }
    P2pScanChannelDefault(pAd, pPort);
    P2pSetListenIntBias(pAd, 1);

    if (IS_P2P_GO_OP(pPort) 
        || IS_P2P_AUTOGO(pPort)
        || ((pAd->OpMode == OPMODE_STAP2P) && (INFRA_ON(pAd->PortList[PORT_0]))))
    {
    }
    else
        pPort->Channel = pPort->P2PCfg.ListenChannel;

    pPort->P2PCfg.CentralChannel = 0;// not used
    pPort->P2PCfg.P2pPhyMode = P2P_PHYMODE_ENABLE_11N_20;

    // Upgrade the Txop parameter. to make a fair discoverable chance.
    // Make sure probe response is tx on a fair paramter.
    if (IDLE_ON(pAd->PortList[PORT_0]))
    {
        RTUSBWriteMACRegister(pAd, EDCA_AC0_CFG, 0xa4300); 
    }

    if (IS_P2P_SIGMA_OFF(pPort))
    {
        if (FALSE == P2pSetEvent(pAd, pPort,  P2PEVENT_P2PGROUP_TOPOLOGY_UPDATE))
            pPort->P2PCfg.P2pEventQueue.bP2pTopologyUpdate = TRUE;
            
    }

    P2PDefaultConfigM(pAd, pPort);
#if UAPSD_AP_SUPPORT
    // Record from WMM-PS GO, reset them
    pPort->P2PCfg.LastMoreDataFromGO[QID_AC_BE]   = FALSE;
    pPort->P2PCfg.LastMoreDataFromGO[QID_AC_BK]   = FALSE;
    pPort->P2PCfg.LastMoreDataFromGO[QID_AC_VI]   = FALSE;
    pPort->P2PCfg.LastMoreDataFromGO[QID_AC_VO]   = FALSE;    
#endif

    // p2p network is down.
    pPort->P2PCfg.LinkState = 0;

    // Stop to maintain Opps flag here
    // Retry limit has also restored when called linkdown()
    pPort->P2PCfg.bOppsOn = FALSE;
}

/*  
    ==========================================================================
    Description: 
        Start P2P Search State
        
    Parameters: 
         
    Note:
         
    ==========================================================================
 */
VOID P2pScan(
    IN PMP_ADAPTER pAd,
    IN PMP_PORT   pPort)
{
    DOT11_SCAN_REQUEST_V2 Dot11ScanRequest = {0};
    CHAR i = 0;

    Dot11ScanRequest.dot11ScanType = (dot11_scan_type_active | dot11_scan_type_forced);
    Dot11ScanRequest.uNumOfdot11SSIDs = 0;
    
    DBGPRINT(RT_DEBUG_TRACE, (" %s - CntlState= %d, SyncState= %d .\n",
        __FUNCTION__, pAd->PortList[PORT_0]->Mlme.CntlMachine.CurrState, pAd->PortList[PORT_0]->Mlme.SyncMachine.CurrState));

    // check CntlMachine.CurrState to avoid collision with NDIS SetOID request
    if (pAd->PortList[PORT_0]->Mlme.CntlMachine.CurrState == CNTL_IDLE && 
        (pAd->PortList[PORT_0]->Mlme.SyncMachine.CurrState == SYNC_IDLE))
    {
        // Set State to xx_COMMAND. So we know this is a P2P SCAN Command in MLME state machine.
        if ( pPort->P2PCfg.P2PDiscoProvState == P2P_SEARCH)
        {
            pPort->P2PCfg.P2PDiscoProvState = P2P_SEARCH_COMMAND;
        }
        else if (pPort->P2PCfg.P2PDiscoProvState == P2P_LISTEN)
        {
            pPort->P2PCfg.P2PDiscoProvState = P2P_LISTEN_COMMAND;
        }

        if(pPort->P2PCfg.P2pCounter.CounterAutoReconnectForP2pClient < 10)
        {
            pPort->P2PCfg.P2pProprietary.ListenChanelIndex = 0;
            pPort->P2PCfg.P2pProprietary.ListenChanelCount = pAd->HwCfg.ChannelListNum;
            // Limit to scan 2.4 GHZ when I am GO. 
            
            for (i = 0; i < pPort->P2PCfg.P2pProprietary.ListenChanelCount; i++)
            {
                pPort->P2PCfg.P2pProprietary.ListenChanel[i] = pAd->HwCfg.ChannelList[i].Channel;
            }
        }

        // Only Port0 support BSSID LIST SCAN behavior, So use Port 0 to enqueu scan command.
        DBGPRINT(RT_DEBUG_TRACE, ("Enqueue P2P Scan command. ListenInterval = %d \n", pPort->P2PCfg.P2pCounter.ListenInterval));
        MlmeEnqueue(pAd,
                    pPort,
                    MLME_CNTL_STATE_MACHINE, 
                    OID_802_11_BSSID_LIST_SCAN, 
                    sizeof(DOT11_SCAN_REQUEST_V2), 
                    &Dot11ScanRequest);
    }
    else
    {
        // Force to restart state machine
        MlmeRestartStateMachine(pAd, pAd->PortList[PORT_0]);
        // Doesn't do Scan.  Set back the P2P State to Idle.
        if (pPort->PortType == WFD_DEVICE_PORT)
            P2pGotoScan(pAd, pPort, P2P_DISCO_TYPE_AUTO, 2);
        else
            P2pGotoScan(pAd, pPort, P2P_DISCO_TYPE_DEFAULT, 2);
        DBGPRINT(RT_DEBUG_TRACE, (" P2P - P2pScan fail when mlme state not idle   %d %d .\n", pAd->PortList[PORT_0]->Mlme.CntlMachine.CurrState
            , pAd->PortList[PORT_0]->Mlme.SyncMachine.CurrState));
    }
}

/*  
    ==========================================================================
    Description: 
        Refresh P2P table and then start p2p scan for GUI display.
        
    Parameters: 
         
    Note:
         
    ==========================================================================
 */
VOID P2pRefreshTableThenScan(
    IN PMP_ADAPTER pAd,
    IN ULONG        CounterAftrScanButton)
{
    UCHAR   p2pindex;
    PMP_PORT    pPort = pAd->PortList[FXXK_PORT_0];
    PMAC_TABLE_ENTRY pMacEntry = NULL;
    PMAC_TABLE_ENTRY pNextMacEntry = NULL;    
    PQUEUE_HEADER pHeader;

    if (IS_P2P_GO_OP(pPort))
    {
        pHeader = &pPort->MacTab.MacTabList;
        pNextMacEntry = (PMAC_TABLE_ENTRY)(pHeader->Head);
        while (pNextMacEntry != NULL)
        {
            pMacEntry = (PMAC_TABLE_ENTRY)pNextMacEntry;
            
            if(pMacEntry == NULL)
            {
                break;
            }

            if ((pMacEntry->ValidAsCLI == TRUE) 
                && (pMacEntry->ValidAsP2P == TRUE))
            {
                p2pindex = P2pGroupTabInsert(pAd, pMacEntry->P2pInfo.DevAddr, pMacEntry->P2pInfo.P2pClientState, pPort->P2PCfg.SSID, pPort->P2PCfg.SSIDLen);
                P2pCopyMacTabtoP2PTab(pAd, p2pindex, (UCHAR)pMacEntry->wcid);
                P2PPrintP2PEntry(pAd, p2pindex);
            }
            
            pNextMacEntry = pNextMacEntry->Next;   
            pMacEntry = NULL;
        }
    }
    P2pSetListenIntBias(pAd,  1);
    pPort->P2PCfg.P2pCounter.CounterAftrScanButton = CounterAftrScanButton;
    P2pGotoScan(pAd, pPort, P2P_DISCO_TYPE_DEFAULT, 2);
}

/*  
    ==========================================================================
    Description: 
        Stop connect command to connect with current MAC becuase the connect process already bagan.
        
    Parameters: 
         
    Note:
         
    ==========================================================================
 */
VOID P2pStopConnectThis(
    IN PMP_ADAPTER pAd,
    IN PMP_PORT     pPort)
{
    // Zero all connecting MAC list, so will stop connect command
    PlatformZeroMemory(&pPort->P2PCfg.ConnectingMAC[pPort->P2PCfg.ConnectingIndex][0], MAC_ADDR_LEN);
}

/*  
    ==========================================================================
    Description: 
        Stop the connect command action.
        
    Parameters: 
         
    Note:
         
    ==========================================================================
 */
VOID P2pStopConnectAction(
    IN PMP_ADAPTER pAd,
    IN PMP_PORT     pPort)
{
    DBGPRINT(RT_DEBUG_TRACE, ("P2pStopConnectAction =  %s \n",  decodeP2PState(pPort->P2PCfg.P2PConnectState)));
    PlatformZeroMemory(pPort->P2PCfg.ConnectingMAC, MAX_P2P_GROUP_SIZE*MAC_ADDR_LEN);
    pPort->P2PCfg.ConnectingIndex = MAX_P2P_GROUP_SIZE;
}

/*  
    ==========================================================================
    Description: 
        Set parameter stop P2P Search State when P2P has started Group Formation.
        
    Parameters: 
         
    Note:
         
    ==========================================================================
 */
VOID P2pStopScan(
    IN PMP_ADAPTER pAd,
    IN  PMP_PORT    pPort)
{
    DBGPRINT(RT_DEBUG_TRACE, (" P2P - P2pStopScan @channel = %d.\n", pPort->ScaningChannel));
    // Set scan channel to Last one to stop Scan Phase. Because SCannextchannel will use channel to judge if it should stop scan.
    pPort->ScaningChannel = pPort->P2PCfg.P2pProprietary.ListenChanel[pPort->P2PCfg.P2pProprietary.ListenChanelCount-1];
    DBGPRINT(RT_DEBUG_TRACE, ("pPort->P2PCfg.P2pProprietary.ListenChanel[pPort->P2PCfg.P2pProprietary.ListenChanelCount-1]; = %d", pPort->P2PCfg.P2pProprietary.ListenChanel[pPort->P2PCfg.P2pProprietary.ListenChanelCount-1]));
    // Extend scan time to 4 seconds, this allows STAs to have more time to listen for response
    pPort->P2PCfg.P2pCounter.ScanIntervalBias = 40;
    P2pSetListenIntBias(pAd,  3);
}

/*  
    ==========================================================================
    Description: 
         Goto Idle state. Update necessary parameters.
        
    Parameters: 
    Note:
         
    ==========================================================================
 */
VOID P2pGotoIdle(
    IN PMP_ADAPTER pAd,
    IN PMP_PORT     pPort) 
{

    pPort->P2PCfg.P2PDiscoProvState = P2P_ENABLE_LISTEN_ONLY;
    pPort->P2PCfg.P2pCounter.Counter100ms = 0;
    // Set a randon period to start next Listen State.
    pPort->P2PCfg.P2pCounter.NextScanRound = (RandomByte(pAd) % P2P_RANDOM_BASE) + P2P_RANDOM_BIAS;
}

/*  
    ==========================================================================
    Description: 
         Goto Scan/Search state. Update necessary parameters.
        
    Parameters: 
    Note:
         
    ==========================================================================
 */
VOID P2pGotoScan(
    IN PMP_ADAPTER pAd,
    IN PMP_PORT     pPort,
    IN P2P_DISCOVERY_TYPE DiscoverType,
    IN ULONG StartScanRound)
{
    //PMP_PORT    pPort = pAd->PortList[pPort->P2PCfg.PortNumber];
    
    // Set a short time to start next search State for the 1st time.
    if (StartScanRound > 0)
        pPort->P2PCfg.P2pCounter.NextScanRound = StartScanRound;
    else
        pPort->P2PCfg.P2pCounter.NextScanRound = 2;

    // Update discovery type and scan channel
    pPort->P2PCfg.DiscoverType = DiscoverType;
    pPort->P2PCfg.P2pProprietary.ChannelScanRound = 0;
    P2pScanChannelUpdate(pAd, pPort);

    pPort->P2PCfg.P2PDiscoProvState = P2P_IDLE;

    // Reset 100ms Counter to zero.
    pPort->P2PCfg.P2pCounter.Counter100ms = 0;

    DBGPRINT(RT_DEBUG_TRACE, ("P2pGotoScan! Set NextScanRound = %d, DiscoverType = 0x%x  Here!!!\n", 
        pPort->P2PCfg.P2pCounter.NextScanRound,  pPort->P2PCfg.DiscoverType));
}

/*  
    ==========================================================================
    Description: 
        When I am GO, start a P2P NoA schedule.
        
    Parameters: 
        S - pointer to the association state machine
    Note:
        The state machine looks like the following as name implies its function
    ==========================================================================
 */
VOID        P2pGOStartNoA(
    IN PMP_ADAPTER pAd,
    IN PMP_PORT     pPort,
    IN BOOLEAN      bOffChannel)
{
    ULONG   Value;
    ULONG   TimeTillTbtt;
    ULONG   temp;
    PQUEUE_ENTRY    pQEntry = NULL;
    PNOAMGMT_STRUC  pMgmt = NULL;
    ULONG   StartTimeOffset = 25;   // As Testbed, Sigma set Starttimmeoffset is how many ms after a beacom timepoint.

    if (pPort->P2PCfg.GONoASchedule.Duration == 0)
    {
        DBGPRINT(RT_DEBUG_TRACE,("   .!!!!wrong command to start NoA. Duration is =  %d  \n", pPort->P2PCfg.GONoASchedule.Duration));
        return;
    }
    
    if (pPort->P2PCfg.GONoASchedule.Interval== 0)
    {
        DBGPRINT(RT_DEBUG_TRACE,("   .!!!!wrong command to start NoA. Interval is =  %d  \n", pPort->P2PCfg.GONoASchedule.Interval));
        return;
    }
    
    if (pPort->P2PCfg.GONoASchedule.Count == 0)
    {
        DBGPRINT(RT_DEBUG_TRACE,("   .!!!!wrong command to start NoA. Count is =  %d  \n", pPort->P2PCfg.GONoASchedule.Count));
        return;
    }

    //Clear existing NoA queue before starting NoA
    NdisAcquireSpinLock(&pAd->pP2pCtrll->TxSwNoAMgmtQueueLock);
    while(pAd->pP2pCtrll->TxSwNoAMgmtQueue.Head != NULL)
    {
        pQEntry = RemoveHeadQueue(&pAd->pP2pCtrll->TxSwNoAMgmtQueue);
        pMgmt = CONTAINING_RECORD(pQEntry, NOAMGMT_STRUC, NoAQEntry);   
        MlmeFreeMemory(pAd, pMgmt->NoAQEntry.pBuffer);
        pMgmt->NoAQEntry.pBuffer = NULL;
        pMgmt->NoAQEntry.Valid = FALSE;
    }
    NdisReleaseSpinLock(&pAd->pP2pCtrll->TxSwNoAMgmtQueueLock);
    
    StartTimeOffset = 25600;        // 25ms
    if (pPort->P2PCfg.GONoASchedule.Count != 255)
    {
        StartTimeOffset = 51200;        // 50ms
    }
    
    DBGPRINT(RT_DEBUG_INFO,("Start Time Offset relative to beacon time point = %d ms \n", StartTimeOffset/1024));
    P2pStopOpPS(pAd, pPort);
    pPort->P2PCfg.GONoASchedule.Token++;
    // Don't "REALLY" enable NoA. Becuase this maybe a offchannel scan request. So I justn eed to 
    // broadcast in my beacon to tell others don't send me any packet during my off-channel period.
    if (bOffChannel == FALSE)
        pPort->P2PCfg.GONoASchedule.bValid = TRUE;
    // Now don't have rule to turn on oboth NoA and CTWindows at the same time. 
    // so if Start NoA, turn off CTWindows here.
    // Start Time
    RTUSBReadMACRegister(pAd, TSF_TIMER_DW1, &pPort->P2PCfg.GONoASchedule.TsfHighByte);
    DBGPRINT(RT_DEBUG_INFO,("P2pGOStartNoA parameter.!!!!HighByte = %x \n", pPort->P2PCfg.GONoASchedule.TsfHighByte));
    RTUSBReadMACRegister(pAd, TBTT_TIMER, &TimeTillTbtt);
    TimeTillTbtt = TimeTillTbtt&0x1ffff;
    DBGPRINT(RT_DEBUG_INFO,("   .!!!!TimeTillTbtt =  %d  \n", TimeTillTbtt));
    DBGPRINT(RT_DEBUG_INFO,("pPort->P2PCfg.CTWindows = %x \n", pPort->P2PCfg.CTWindows));
    
    RTUSBReadMACRegister(pAd, TSF_TIMER_DW0, &Value);
    DBGPRINT(RT_DEBUG_INFO,("   .!!!!Current Tsf LSB = = %d \n",  Value));
    temp = TimeTillTbtt*64+Value;
    DBGPRINT(RT_DEBUG_INFO,("   .!!!!Tsf LSB + TimeTillTbtt= %d \n", temp));
    // Wait five beacon 0x7d00 for 5 beacon interval.  0x6400 is set to 25%*beacon interval
    //pPort->P2PCfg.GONoASchedule.StartTime = (Value/102400)*102400 + 0x7d000 + 0x6400;
    //pPort->P2PCfg.GONoASchedule.NextTargetTimePoint = ((Value/102400) + 1)*102400 + 0x7d000 + 0x6400  + pPort->P2PCfg.GONoASchedule.Interval -pPort->P2PCfg.GONoASchedule.Duration;
    //P2pSetGP(pAd, (Value%102400 + 0x7D000 + 0x6400));
    // in general case, we start absence period 25ms after beacon transmission (us)
    if (bOffChannel == FALSE)
    {
        pPort->P2PCfg.GONoASchedule.StartTime = Value + TimeTillTbtt*64 + 512000 + StartTimeOffset;
        pPort->P2PCfg.GONoASchedule.CurrentTargetTimePoint = pPort->P2PCfg.GONoASchedule.StartTime;
        // IF duration is > 70ms, start absence period right after beacon transmission.
        if (pPort->P2PCfg.GONoASchedule.Duration > 0x11800)
            pPort->P2PCfg.GONoASchedule.StartTime = Value + TimeTillTbtt*64 + 512000;
        pPort->P2PCfg.GONoASchedule.NextTargetTimePoint = Value + TimeTillTbtt*64 + 512000 + StartTimeOffset + pPort->P2PCfg.GONoASchedule.Duration;
        pPort->P2PCfg.GONoASchedule.NextTimePointForWMMPSCounting = Value + TimeTillTbtt*64 + 512000 + StartTimeOffset;
        pPort->P2PCfg.GONoASchedule.ThreToWrapAround = pPort->P2PCfg.GONoASchedule.StartTime + 0x7fffffff;
        // If off channel request is true, 
        P2pResetNoATimer(pAd, (TimeTillTbtt*64 + 512000 + StartTimeOffset), bOffChannel);
    }
    else
    {
        pPort->P2PCfg.GONoASchedule.StartTime = Value + 512000;
        pPort->P2PCfg.GONoASchedule.CurrentTargetTimePoint = pPort->P2PCfg.GONoASchedule.StartTime;
        pPort->P2PCfg.GONoASchedule.NextTargetTimePoint = Value + 512000 + pPort->P2PCfg.GONoASchedule.Duration;
        pPort->P2PCfg.GONoASchedule.NextTimePointForWMMPSCounting = Value + 512000;
        pPort->P2PCfg.GONoASchedule.ThreToWrapAround = pPort->P2PCfg.GONoASchedule.StartTime + 0x7fffffff;
        P2pResetNoATimer(pAd, 512000, bOffChannel);
    }
    DBGPRINT(RT_DEBUG_TRACE,("   .!!!!pPort->P2PCfg.GONoASchedule.Duration = %d  \n", pPort->P2PCfg.GONoASchedule.Duration));

    pPort->P2PCfg.GONoASchedule.bInAwake = TRUE;
    pPort->P2PCfg.GONoASchedule.bWMMPSInAbsent = FALSE; // Set to FALSE if changes state to Awake

    // For print message to debug.. Use 16 digit
    temp = Value + TimeTillTbtt*64 + 0x7D000 + 0x6400;
    DBGPRINT(RT_DEBUG_INFO,("   .!!!!Expect Starttime= %d. ThreToWrapAround = %d. \n", temp, pPort->P2PCfg.GONoASchedule.ThreToWrapAround));
    temp = temp - Value;
    pPort->P2PCfg.GONoASchedule.SwTimerTickCounter = 0;
    DBGPRINT(RT_DEBUG_INFO,("   .!!!!more = %d  to start time \n", temp));

}

/*  
    ==========================================================================
    Description: 
        Stop NoA.
        
    Parameters: 
         
    Note:
         
    ==========================================================================
 */
VOID     P2pStopNoA(
    IN PMP_ADAPTER pAd, 
    IN PMAC_TABLE_ENTRY pMacClient)
{
    ULONG   Value;
    BOOLEAN Cancelled;
    PMP_PORT    pPort = pAd->PortList[FXXK_PORT_0];
    DBGPRINT(RT_DEBUG_TRACE,("P2pStopNoA.!!!! \n"));

    PlatformCancelTimer(&pPort->P2PCfg.P2pPreAbsenTimer, &Cancelled);
    pPort->P2PCfg.bKeepSlient = FALSE;
    pPort->P2PCfg.bPreKeepSlient = FALSE;
    pPort->P2PCfg.NoAIndex = MAX_LEN_OF_MAC_TABLE;

    //Clear queued packets
    if (pAd->pP2pCtrll->TxSwNoAMgmtQueue.Number > 0 )
    {
        DBGPRINT(RT_DEBUG_TRACE,("TxSwNoAMgmtQueue.Number = %d. Dequeue.!!!! \n", pAd->pP2pCtrll->TxSwNoAMgmtQueue.Number));
        MTDeQueueNoAMgmtPacket(pAd);
    }
    NdisSetEvent(&(GET_THREAD_EVENT(&pAd->pHifCfg->hDequeueDataThread)));

    //Restore RTY/RTS setting
    if (pPort->P2PCfg.bNoAOn == TRUE)
    {
        pPort->P2PCfg.bNoAOn = FALSE;
        MTEnqueueInternalCmd(pAd, pPort, MT_CMD_P2P_SET_PSM_CFG, NULL, 0);
    }
    
    if (pMacClient != NULL)
    {
        pMacClient->P2pInfo.NoADesc[0].Count = 0xf3;
        pMacClient->P2pInfo.NoADesc[0].bValid = FALSE;
        pMacClient->P2pInfo.NoADesc[0].bInAwake = TRUE;
        // Try set Token to a value that has smallest chane the same as the Next Token GO will use.
        // So decrease 1
        pMacClient->P2pInfo.NoADesc[0].Token--;
    }
    PlatformCancelTimer(&pPort->P2PCfg.P2pSwNoATimer, &Cancelled);
    pPort->P2PCfg.GONoASchedule.bValid = FALSE;
    pPort->P2PCfg.GONoASchedule.bInAwake = TRUE;
    pPort->P2PCfg.GONoASchedule.bWMMPSInAbsent = FALSE; // Set to FALSE if changes state to Awake
    // If need not resume NoA. Can reset all parameters.
    if (pPort->P2PCfg.GONoASchedule.bNeedResumeNoA == FALSE)
    {
        pPort->P2PCfg.GONoASchedule.Count = 1;
        pPort->P2PCfg.GONoASchedule.Interval = 0x19000;
    }

    if (!MT_TEST_FLAG(pAd, fRTMP_ADAPTER_NIC_NOT_EXIST))
    {
        RTUSBReadMACRegister(pAd, INT_TIMER_EN, &Value);
        Value &= (0xfffffffd);
        RTUSBWriteMACRegister(pAd, INT_TIMER_EN, Value);
    }
    pPort->P2PCfg.GONoASchedule.SwTimerTickCounter = 0;

    // Set to false again.  
    pPort->P2PCfg.bPreKeepSlient = FALSE;
}
    
/*  
    ==========================================================================
    Description: 
        Start Opputunistic Power Save.
        
    Parameters: 
         
    Note:
         
    ==========================================================================
 */
VOID     P2pStartOpPS(
    IN PMP_ADAPTER pAd,
    IN PMP_PORT     pPort)
{
    if (pPort->P2PCfg.GONoASchedule.bValid == TRUE)
        P2pStopNoA(pAd, NULL);
    
    DBGPRINT(RT_DEBUG_TRACE,("P2P : !! P2pStartOpPS \n"));
    // default use CTWindows as 50ms = 0x32.  So 0x32 + 0x80 = 0xb2.
    pPort->P2PCfg.CTWindows = 0xb2;
    if ((pPort->CommonCfg.P2pControl.field.OpPSAlwaysOn == 1) && 
        (IS_P2P_SIGMA_ON(pPort)) && (pPort->P2PCfg.SigmaSetting.CTWindow >= 5 ))
        pPort->P2PCfg.CTWindows = 0x80 + pPort->P2PCfg.SigmaSetting.CTWindow;
    // Wait next beacon period to really start queue packet.
    pPort->P2PCfg.bKeepSlient = FALSE;
    pPort->P2PCfg.bPreKeepSlient = FALSE;

}

/*  
    ==========================================================================
    Description: 
        Stop Opputunistic Power Save.
        
    Parameters: 
         
    Note:
         
    ==========================================================================
 */
VOID     P2pStopOpPS(
    IN PMP_ADAPTER pAd,
    IN PMP_PORT     pPort)
{
    if (pPort->P2PCfg.GONoASchedule.bValid == FALSE)
        pPort->P2PCfg.bKeepSlient = FALSE;

    if (MT_TEST_BIT(pPort->P2PCfg.CTWindows, P2P_OPPS_BIT))
        pPort->P2PCfg.bFirstTimeCancelOpps = TRUE;
    pPort->P2PCfg.CTWindows = 0;
}

/*  
    ==========================================================================
    Description: 
        Set a timer that let me to go to Absence earlier. So call Pre-Absence timer.
        
    Parameters: 
        S - pointer to the association state machine
    Note:
         
    ==========================================================================
 */
VOID P2pPreAbsenTimeOutTimerCallback(
    IN PVOID SystemSpecific1, 
    IN PVOID FunctionContext, 
    IN PVOID SystemSpecific2, 
    IN PVOID SystemSpecific3) 
{
    PMP_PORT pPort = (PMP_PORT )FunctionContext;
    PMP_ADAPTER  pAd = pPort->pAd;
    pPort->P2PCfg.bPreKeepSlient = TRUE;
}

#if(COMPILE_WIN8_ABOVE(CURRENT_OS_NDIS_VER))  
VOID P2pSendProReqTimeOutTimerCallback(
    IN PVOID SystemSpecific1,
    IN PVOID FunctionContext,
    IN PVOID SystemSpecific2,
    IN PVOID SystemSpecific3)
{
    PMP_PORT pPort = (PMP_PORT )FunctionContext;
    PMP_ADAPTER  pAd = pPort->pAd;
    PUCHAR          pOutBuffer = NULL;
    NDIS_STATUS     NStatus;
    
    if((MT_TEST_FLAG(pAd, fRTMP_ADAPTER_BSS_SCAN_IN_PROGRESS))  
        && (pPort->P2PCfg.SetPobeReqTimer == TRUE)
        /*&& ( pPort->P2PCfg.pPobeReqBuffer != NULL)*/
        && (pPort->P2PCfg.PobeReqBufferLen != 0))
    {
        // NdisCommonMiniportMMRequest will send out OutBuffer packet and then free it in NdisCommonUsbDequeueMLMEPacket
        // So we need to allocate a new one for new probe request buffer
        NStatus = MlmeAllocateMemory(pAd, &pOutBuffer);
        if (NStatus == NDIS_STATUS_SUCCESS)
        {
            PlatformMoveMemory(pOutBuffer, pPort->P2PCfg.pPobeReqBuffer, pPort->P2PCfg.PobeReqBufferLen);
            NdisCommonMiniportMMRequest(pAd, pOutBuffer, pPort->P2PCfg.PobeReqBufferLen);
            PlatformSetTimer(pPort, &pPort->P2PCfg.P2pSendProReqTimer, 6);  // resolution = 10ms
            pPort->P2PCfg.SetPobeReqTimer = TRUE;
            DBGPRINT(RT_DEBUG_TRACE, ("%s : Send P2P Probe Req again\n", __FUNCTION__));
        }
        else
        {
            pPort->P2PCfg.SetPobeReqTimer = FALSE;
            DBGPRINT(RT_DEBUG_TRACE, ("%s : Memory alloc failed. Stop P2P Probe Req\n", __FUNCTION__));
        }
    }
    else
    {   
        pPort->P2PCfg.SetPobeReqTimer = FALSE;
        DBGPRINT(RT_DEBUG_TRACE, ("%s : Stop P2P Probe Req", __FUNCTION__));
    }
        
}

#endif

/*  
    ==========================================================================
    Description: 
        When I am P2P Client , Handle NoA Attribute.
        
    Parameters: 
        S - pointer to the association state machine
    Note:
        The state machine looks like the following as name implies its function
    ==========================================================================
 */
VOID P2pSwNoATimeOutTimerCallback(
    IN PVOID SystemSpecific1, 
    IN PVOID FunctionContext, 
    IN PVOID SystemSpecific2, 
    IN PVOID SystemSpecific3) 
{
    PMP_PORT pPort = (PMP_PORT )FunctionContext;
    PMP_ADAPTER  pAd = pPort->pAd;
    MTEnqueueInternalCmd(pAd, pPort, MT_CMD_P2P_SWNOATIMEOUT, NULL, 0); 
}

VOID SwTBTTTimeOutTimerCallback(
    IN PVOID SystemSpecific1, 
    IN PVOID FunctionContext, 
    IN PVOID SystemSpecific2, 
    IN PVOID SystemSpecific3) 
{
    PMP_PORT pPort = (PMP_PORT )FunctionContext;
    PMP_ADAPTER  pAd = pPort->pAd;

    // Software based timer means don't use GP interrupt to get precise timer calculation. 
    // So need to check time offset caused by software timer.
    if (pPort->CommonCfg.P2pControl.field.SwBasedNoA == 1)
    {
        MTEnqueueInternalCmd(pAd, pPort, MT_CMD_SET_TBTT_TIMER, NULL, 0);   
    }
}

/*  
    ==========================================================================
    Description: 
        When I am P2P Client , Handle NoA Attribute will start NoA or Stop NoA. B.
        
    Parameters: 
        S - pointer to the association state machine
    Note:
        The state machine looks like the following as name implies its function
    ==========================================================================
 */
BOOLEAN P2pHandleNoAAttri(
    IN PMP_ADAPTER pAd, 
    IN PMP_PORT     pPort,
    IN PMAC_TABLE_ENTRY pMacClient,
    IN PUCHAR pData) 
{
    PP2P_NOA_DESC   pNoADesc;
    ULONG       Value, GPDiff;
    UCHAR       index;
    ULONG       NoALen;
    ULONG       StartTime;
    
    if (pMacClient == NULL)
        return FALSE;

    if (*pData == SUBID_P2P_NOA)
    {
        NoALen = *(pData+1);
        if (NoALen == 2)
        {
            pMacClient->P2pInfo.CTWindow = *(pData+4); 
            if (pMacClient->P2pInfo.NoADesc[0].bValid == TRUE)
                P2pStopNoA(pAd, pMacClient);
            // Copy my GO's CTWindow to P2Pcfg.CTWindow parameters, 
            // Then As Client, I don't need to search for Client when I want to use CTWindow Value.
            pPort->P2PCfg.CTWindows = *(pData+4); 
            return FALSE;
        }
            
        index = *(pData+3);
        pMacClient->P2pInfo.CTWindow = *(pData+4); 
        // Copy my GO's CTWindow to P2Pcfg.CTWindow parameters, 
        // Then As Client, I don't need to search for Client when I want to use CTWindow Value.
        pPort->P2PCfg.CTWindows = *(pData+4); 
        pNoADesc = (PP2P_NOA_DESC)(pData+5);
        pMacClient->P2pInfo.NoADesc[0].Count = pNoADesc->Count;
        pMacClient->P2pInfo.NoADesc[0].Duration = *(PULONG)&pNoADesc->Duration[0];
        pMacClient->P2pInfo.NoADesc[0].Interval = *(PULONG)&pNoADesc->Interval[0];
        pMacClient->P2pInfo.NoADesc[0].StartTime = *(PULONG)&pNoADesc->StartTime[0];
        StartTime = *(PULONG)&pNoADesc->StartTime[0];
    
        if (pMacClient->P2pInfo.NoADesc[0].Token == index)
        {
            // The same NoA. Doesn't need to set this NoA again.
            return FALSE;
        }
        
        DBGPRINT(RT_DEBUG_TRACE,("P2P : !!!NEW NOA Here =[%d, %d] Count = %d. Duration =  %d \n", pMacClient->P2pInfo.NoADesc[0].Token, index, pNoADesc->Count, pMacClient->P2pInfo.NoADesc[0].Duration));
        DBGPRINT(RT_DEBUG_TRACE,("P2P : !!!NEW NOA Here =  CTWindow =  %x \n", pMacClient->P2pInfo.CTWindow));
        pMacClient->P2pInfo.NoADesc[0].Token = index;
        RTUSBReadMACRegister(pAd, TSF_TIMER_DW0, &Value);
        DBGPRINT(RT_DEBUG_TRACE,("Interval = %d. StartTime = %d. TSF timer Register = %d\n", pMacClient->P2pInfo.NoADesc[0].Interval, pMacClient->P2pInfo.NoADesc[0].StartTime, Value));
        if ((pMacClient->P2pInfo.NoADesc[0].Duration <= 0x40) || (pMacClient->P2pInfo.NoADesc[0].Interval <= 0x40))
        {
            DBGPRINT(RT_DEBUG_TRACE,("!!!!!Interval or Duration too small. ignore.  = %x return 1\n", Value));
            return FALSE;
        }
        else if ((pMacClient->P2pInfo.NoADesc[0].Duration >= pMacClient->P2pInfo.NoADesc[0].Interval)
            && (pMacClient->P2pInfo.NoADesc[0].Count > 1))
        {
            DBGPRINT(RT_DEBUG_TRACE,("!!!!!Duration > Inveral.  return 2\n"));
            return FALSE;
        }
        // if Start time point is in the future.
        pPort->P2PCfg.GONoASchedule.CurrentTargetTimePoint = pMacClient->P2pInfo.NoADesc[0].StartTime;
        if (Value < StartTime)
        {
            GPDiff = pMacClient->P2pInfo.NoADesc[0].StartTime - Value;
            pMacClient->P2pInfo.NoADesc[0].NextTargetTimePoint = pMacClient->P2pInfo.NoADesc[0].StartTime + pMacClient->P2pInfo.NoADesc[0].Duration;
            pPort->P2PCfg.GONoASchedule.OngoingAwakeTime = pMacClient->P2pInfo.NoADesc[0].NextTargetTimePoint;
            pPort->P2PCfg.GONoASchedule.NextTimePointForWMMPSCounting = pMacClient->P2pInfo.NoADesc[0].StartTime;
            DBGPRINT(RT_DEBUG_TRACE,("!!!!! GPDiff = %d = 0x%x. NextTargetTimePoint = %d\n", GPDiff, GPDiff, pMacClient->P2pInfo.NoADesc[0].NextTargetTimePoint));
            // try to set General Timer.
            if (P2pResetNoATimer(pAd, GPDiff, FALSE))
            {
                DBGPRINT(RT_DEBUG_TRACE,("!!!!!Start NoA 1  GPDiff = %d \n", GPDiff));
                pMacClient->P2pInfo.NoADesc[0].bValid = TRUE;
                pMacClient->P2pInfo.NoADesc[0].bInAwake = TRUE;
                pMacClient->P2pInfo.NoADesc[0].Token = index;
                return TRUE;
            }
        }
        //else if Start time point is in the past.
        else if (Value >= StartTime)
        {
            do
            {
                StartTime += pMacClient->P2pInfo.NoADesc[0].Interval;
                if ((StartTime > Value) && ((StartTime-Value) > 0x80))
                {
                    GPDiff = StartTime - Value;
                    pMacClient->P2pInfo.NoADesc[0].NextTargetTimePoint = StartTime + pMacClient->P2pInfo.NoADesc[0].Interval - pMacClient->P2pInfo.NoADesc[0].Duration;
                    pPort->P2PCfg.GONoASchedule.OngoingAwakeTime = pMacClient->P2pInfo.NoADesc[0].NextTargetTimePoint;
                    if (P2pResetNoATimer(pAd, GPDiff, FALSE))
                    {
                        DBGPRINT(RT_DEBUG_TRACE,("!!!!!Start NoA 2  GPDiff = %d\n", GPDiff));
                        pMacClient->P2pInfo.NoADesc[0].bValid = TRUE;
                        pMacClient->P2pInfo.NoADesc[0].bInAwake = TRUE;
                        pMacClient->P2pInfo.NoADesc[0].Token = index;
                        return TRUE;
                    }
                }
            }while(TRUE);

        }
        else
        {
            DBGPRINT(RT_DEBUG_TRACE,("Start time !!..!!Check \n"));
        }
    }
    return FALSE;

}

/*  
    ==========================================================================
    Description: 
        this function is for software based NoA method use. to Set a timer for the coming period.
        No matter going to awake period or going to absence period.
        
    Parameters: 
        S - pointer to the association state machine
    Note:
         
    ==========================================================================
 */
BOOLEAN     P2pResetNoATimer(
    IN PMP_ADAPTER pAd,
    IN  ULONG   DiffTimeInus,
    IN  BOOLEAN     bOffChannel)
{
    ULONG   GPDiff;
    BOOLEAN brc = FALSE;
    PMP_PORT pPort = pAd->PortList[FXXK_PORT_0];

    // Software based timer means don't use GP interrupt to get precise timer calculation. 
    // So need to check time offset caused by software timer.
    if (pPort->CommonCfg.P2pControl.field.SwBasedNoA == 1)
    {
        GPDiff = (DiffTimeInus>>10) & 0xffff;
        if (GPDiff > 0)
        {
            GPDiff++;
            DBGPRINT(RT_DEBUG_TRACE,("[%s] Set NoA Timer = %ld \n",__FUNCTION__,GPDiff));
            PlatformSetTimer(pPort, &pPort->P2PCfg.P2pSwNoATimer, GPDiff);
            // Increase timer tick counter.
            pPort->P2PCfg.GONoASchedule.SwTimerTickCounter++;
            brc = TRUE;
            // Will go to awake later. Set a pre-enter-absence timer that the time out is smaller the GPDiff.
            if (pPort->P2PCfg.GONoASchedule.bInAwake == FALSE && (bOffChannel == FALSE))
            {
                if (GPDiff > 10)
                {
                    PlatformSetTimer(pPort, &pPort->P2PCfg.P2pPreAbsenTimer, (GPDiff - 10));
                }
            }
        }
    }
    else
    {
        DBGPRINT(RT_DEBUG_TRACE,("No GP support in USB device\n"));
        brc = FALSE;
    }
    return brc;

}



/*  
    ==========================================================================
    Description: 
        Software Based NoA method might have time shift.  So need to adjust it as time goes by.
        
    Parameters: 
        S - pointer to the association state machine
    Note:
         
    ==========================================================================
 */
BOOLEAN     P2pAdjustSwNoATimer(
    IN PMP_ADAPTER pAd,
    IN PMP_PORT     pPort,
    IN ULONG        CurrentTimeStamp, 
    IN ULONG        NextTimePoint) 
{
    ULONG   AwakeDuration, NewStartTime;
    UCHAR       FakeNoAAttribute[32];
    //PMP_PORT pPort = pAd->PortList[pPort->P2PCfg.PortNumber];
    PMAC_TABLE_ENTRY pWcidMacTabEntry = NULL;
    pWcidMacTabEntry = MlmeSyncMacTabMatchedWlanIdx(pPort, pPort->P2PCfg.MyGOwcid);  

    if(pWcidMacTabEntry == NULL)
    {
        DBGPRINT(RT_DEBUG_TRACE,("%s(), Line:%d, Mac Entry is NULL  Wcid = %d\n", __FUNCTION__, __LINE__, pPort->P2PCfg.MyGOwcid));
        return FALSE;
    }
    
    PlatformZeroMemory(FakeNoAAttribute, 32);
    AwakeDuration = pPort->P2PCfg.GONoASchedule.Interval - pPort->P2PCfg.GONoASchedule.Duration;
    if (CurrentTimeStamp >= pPort->P2PCfg.GONoASchedule.CurrentTargetTimePoint)
    {
        // If offset is more than 1/4 of duration.
        if ((pPort->P2PCfg.GONoASchedule.OngoingAwakeTime) >= (AwakeDuration>> 2))
        {
            DBGPRINT(RT_DEBUG_TRACE,("P2pAdjustSwNoATimer HERE HERE!!!! \n"));
            DBGPRINT(RT_DEBUG_TRACE,("OngoingAwakeTime = %d. CurrentTimeStamp = %d.!!!! \n", pPort->P2PCfg.GONoASchedule.OngoingAwakeTime, CurrentTimeStamp));
            P2pStopNoA(pAd, pWcidMacTabEntry);
            FakeNoAAttribute[0] = SUBID_P2P_NOA;
            NewStartTime = pPort->P2PCfg.GONoASchedule.StartTime + (pPort->P2PCfg.GONoASchedule.SwTimerTickCounter - 1)*(pPort->P2PCfg.GONoASchedule.Interval);
            P2PMakeFakeNoATlv(pAd, pPort, NewStartTime, &FakeNoAAttribute[0]);
            pWcidMacTabEntry->P2pInfo.NoADesc[0].Token--;
            P2pHandleNoAAttri(pAd, pPort, pWcidMacTabEntry, &FakeNoAAttribute[0]);
        }
        // Update expected next Current Target Time Point with NextTimePoint
        pPort->P2PCfg.GONoASchedule.CurrentTargetTimePoint = NextTimePoint;
        // Can immediately dequeue packet because peer already in awake period.
        return TRUE;
    }
    else
    {
        // Update expected next Current Target Time Point with NextTimePoint
        pPort->P2PCfg.GONoASchedule.CurrentTargetTimePoint = NextTimePoint;
        return FALSE;   
    }
}

/*  
    ==========================================================================
    Description: 
        this is the routine for  every time out handling of period (absence period or awake period)
        
    Parameters: 
        S - pointer to the association state machine
    Note:
         
    ==========================================================================
 */
VOID P2pGPTimeOutHandle(
    IN PMP_ADAPTER pAd) 
{
    MAC_TABLE_ENTRY *pEntry;
    ULONG       MacValue;
    ULONG       Value = 0;
    ULONG       GPDiff;
    ULONG       NextDiff;
    ULONG       SavedNextTargetTimePoint;
    NTSTATUS    status;
    
    PMP_PORT    pPort = pAd->PortList[FXXK_PORT_0];
    
    PMAC_TABLE_ENTRY pWcidMacTabEntry = NULL;
    PMAC_TABLE_ENTRY pMacEntry = NULL;
    PMAC_TABLE_ENTRY pNextMacEntry = NULL;  
    PQUEUE_HEADER pHeader;
    
    // GO operating or Autonomous GO 
    if (IS_P2P_GO_OP(pPort) || (pPort->P2PCfg.P2PConnectState == P2P_ANY_IN_FORMATION_AS_GO))
    {
        if ((pPort->P2PCfg.GONoASchedule.bValid == TRUE)
            || (pPort->P2PCfg.P2PChannelState == P2P_PRE_NOATHEN_GOTOSCAN_STATE)
            || (pPort->P2PCfg.P2PChannelState == P2P_NOATHEN_GOTOSCAN_STATE))
        {
            if ((pPort->P2PCfg.GONoASchedule.Count > 0) && (pPort->P2PCfg.GONoASchedule.Count < 255))
            {
                // Sometimes go to awake, sometime go to silence. Two state counts One count down.
                // so only minus Count when I change from Sleep to Awake
                if (pPort->P2PCfg.GONoASchedule.bInAwake == FALSE)
                    pPort->P2PCfg.GONoASchedule.Count--;
            }
            if (pPort->P2PCfg.GONoASchedule.Count == 0)
            {
                P2pStopNoA(pAd, NULL);
                if (FALSE == P2pCheckBssSync(pAd))
                {
                    AsicResumeBssSync(pAd);
                }
                DBGPRINT(RT_DEBUG_TRACE,("P2pGPTimeOutHandle.!!StopGP.  return.1 \n"));
                return;
            }
            
            if (pPort->P2PCfg.P2PChannelState == P2P_NOATHEN_GOTOSCAN_STATE)
            {
                if (pPort->P2PCfg.QueuedScanReq.ScanType == 0)
                {
                    DBGPRINT(RT_DEBUG_TRACE, ("1 P2pGPTimeOutHandle..ScanType = %d.\n",pPort->P2PCfg.QueuedScanReq.ScanType ));
                }
                else
                {
                    DBGPRINT(RT_DEBUG_TRACE, ("2 P2pGPTimeOutHandle..Enqueue ScanType = %d.\n",pPort->P2PCfg.QueuedScanReq.ScanType ));
                    MlmeEnqueue(pAd, pAd->PortList[PORT_0], SYNC_STATE_MACHINE, MT2_MLME_SCAN_REQ, 
                        sizeof(MLME_SCAN_REQ_STRUCT), &pPort->P2PCfg.QueuedScanReq);    // To Run MlmeSyncScanReqAction.
                    pAd->PortList[PORT_0]->Mlme.CntlMachine.CurrState = CNTL_WAIT_OID_LIST_SCAN;
                }
                pPort->P2PCfg.P2PChannelState = P2P_ENTER_GOTOSCAN;
            }

            if (pPort->P2PCfg.GONoASchedule.bInAwake == TRUE)
                NextDiff = pPort->P2PCfg.GONoASchedule.Duration;
            else
                NextDiff = pPort->P2PCfg.GONoASchedule.Interval - pPort->P2PCfg.GONoASchedule.Duration;
                
            // Prepare next time.
            status = RTUSBReadMACRegister(pAd, TSF_TIMER_DW0, &Value);
            if(status != STATUS_SUCCESS)
                DBGPRINT(RT_DEBUG_TRACE,("[%s] UNABLE TO READ REGISTER %x \n",__FUNCTION__,Value));

            DBGPRINT(RT_DEBUG_TRACE,("Current Time =%x, ThreToWrapAround = %x. StartTime = %x. \n",Value, pPort->P2PCfg.GONoASchedule.ThreToWrapAround, pPort->P2PCfg.GONoASchedule.StartTime));
            // Check whether we should to renew the NoA because at least 2^31 us should update once according to spec.
            if (pPort->P2PCfg.GONoASchedule.ThreToWrapAround > pPort->P2PCfg.GONoASchedule.StartTime)
            {
                if (Value > pPort->P2PCfg.GONoASchedule.ThreToWrapAround && (pPort->P2PCfg.GONoASchedule.Count == 255))
                {
                    pPort->P2PCfg.GONoASchedule.bNeedResumeNoA = TRUE;
                    P2pStopNoA(pAd, NULL);
                    DBGPRINT(RT_DEBUG_TRACE,("P2pGPTimeOutHandle.!!StopGP.  return.3. will resume.\n"));
                    return;
                }
            }
            else
            {
                if ((Value > pPort->P2PCfg.GONoASchedule.ThreToWrapAround) && (Value < pPort->P2PCfg.GONoASchedule.StartTime) && (pPort->P2PCfg.GONoASchedule.Count == 255))
                {
                    pPort->P2PCfg.GONoASchedule.bNeedResumeNoA = TRUE;
                    P2pStopNoA(pAd, NULL);
                    DBGPRINT(RT_DEBUG_TRACE,("P2pGPTimeOutHandle.!!StopGP.  return.4. will resume. \n"));
                    return;
                }
            }

            SavedNextTargetTimePoint = pPort->P2PCfg.GONoASchedule.NextTargetTimePoint;
            if (Value <= pPort->P2PCfg.GONoASchedule.NextTargetTimePoint)
            {
                GPDiff = pPort->P2PCfg.GONoASchedule.NextTargetTimePoint - Value;
                pPort->P2PCfg.GONoASchedule.NextTimePointForWMMPSCounting = pPort->P2PCfg.GONoASchedule.NextTargetTimePoint;
                pPort->P2PCfg.GONoASchedule.NextTargetTimePoint += NextDiff;
                P2pResetNoATimer(pAd, GPDiff, FALSE);
                DBGPRINT(RT_DEBUG_TRACE,(" NextTargetTimePoint = %d. \n", pPort->P2PCfg.GONoASchedule.NextTargetTimePoint));
                DBGPRINT(RT_DEBUG_TRACE,("  Value = %d.  GPDiff = %d.\n", Value, GPDiff));
            }
            else
            {
                // driver restart NoA due to our GP timer's delay. 
                if (pPort->P2PCfg.GONoASchedule.Count == 255)
                {
                    pPort->P2PCfg.GONoASchedule.bNeedResumeNoA = TRUE;
                    DBGPRINT(RT_DEBUG_TRACE,(" Prepare resume NoA. \n"));
                }
                P2pStopNoA(pAd, NULL);
                DBGPRINT(RT_DEBUG_TRACE,("3 NextTargetTimePoint = %d. \n", pPort->P2PCfg.GONoASchedule.NextTargetTimePoint));
                DBGPRINT(RT_DEBUG_TRACE,(" 3 Value = %d= 0x%x NextDiff = %d.\n", Value,  Value,NextDiff));
                DBGPRINT(RT_DEBUG_TRACE,("3 P2pGPTimeOutHandle.!!StopGP.  return.2 \n"));
                return;
            }
                
            if (pPort->P2PCfg.GONoASchedule.bInAwake == TRUE)
            {
                DBGPRINT(RT_DEBUG_TRACE,(" ----------------------->>> NoA  Go to SLEEP \n"));       
                pPort->P2PCfg.GONoASchedule.bWMMPSInAbsent = TRUE; // Set to FALSE if changes state to absent
                pPort->P2PCfg.bKeepSlient = TRUE;
                pPort->P2PCfg.bPreKeepSlient = TRUE;
                pPort->P2PCfg.GONoASchedule.bInAwake = FALSE; 
                // roughly check that if duration > 100ms, we should call Pause beacon.
                if ((pPort->P2PCfg.GONoASchedule.Duration >= 0x19000)
                    && (pPort->P2PCfg.P2PChannelState != P2P_PRE_NOATHEN_GOTOSCAN_STATE)
                    && (pPort->P2PCfg.P2PChannelState != P2P_NOATHEN_GOTOSCAN_STATE))
                {
                    AsicPauseBssSync(pAd);
                }

#if UAPSD_AP_SUPPORT
                // Force to close SPs of ALL Entry once entered NoA Absent, starIdx=2
                pHeader = &pPort->MacTab.MacTabList;
                pNextMacEntry = (PMAC_TABLE_ENTRY)(pHeader->Head);
                while (pNextMacEntry != NULL)
{
                    pMacEntry = (PMAC_TABLE_ENTRY)pNextMacEntry;

                    if(pMacEntry == NULL)
                    {
                        break;
                    }
                    
                    if((pMacEntry->WlanIdxRole == ROLE_WLANIDX_BSSID) ||(pMacEntry->WlanIdxRole == ROLE_WLANIDX_MBCAST))
                    {
                        pNextMacEntry = pNextMacEntry->Next;   
                        pMacEntry = NULL;
                        continue; 
                    }

                    if  (pMacEntry->ValidAsCLI) 
                        ApUapsdServicePeriodClose(pAd, pMacEntry, TRUE);
                    
                    pNextMacEntry = pNextMacEntry->Next;   
                    pMacEntry = NULL;
}               
#endif
                
            }
            else
            {
                DBGPRINT(RT_DEBUG_TRACE,(" NoA  Wake UP ----------------------->>> \n"));
                pPort->P2PCfg.GONoASchedule.bInAwake = TRUE;
                pPort->P2PCfg.GONoASchedule.bWMMPSInAbsent = FALSE; // Set to FALSE if changes state to Awake
                pPort->P2PCfg.bKeepSlient = FALSE;    
                pPort->P2PCfg.bPreKeepSlient = FALSE;
                
                if (FALSE == P2pCheckBssSync(pAd))
                {
                    AsicResumeBssSync(pAd);
                }
                if ((pPort->CommonCfg.P2pControl.field.SwBasedNoA == 1)
                    && (pPort->P2PCfg.GONoASchedule.Count > 100))
                {
                    if (TRUE == P2pAdjustSwNoATimer(pAd, pPort, Value, SavedNextTargetTimePoint))
                    {
                        DBGPRINT(RT_DEBUG_TRACE,("SwBasedNoA : Dequeue here. %d\n", pAd->pP2pCtrll->TxSwNoAMgmtQueue.Number));
                        MTDeQueueNoAMgmtPacket(pAd);
                        NdisSetEvent(&(GET_THREAD_EVENT(&pAd->pHifCfg->hDequeueDataThread)));
                    }
                }
                else
                {
                    MTDeQueueNoAMgmtPacket(pAd);
                    NdisSetEvent(&(GET_THREAD_EVENT(&pAd->pHifCfg->hDequeueDataThread)));
                }
            }

        }
        else if (pPort->P2PCfg.GONoASchedule.bValid == FALSE)
        {
            P2pStopNoA(pAd, NULL);
        }

    }
    else if (IS_P2P_CLIENT_OP(pPort))
    {
        if (pPort->P2PCfg.NoAIndex >= MAX_LEN_OF_MAC_TABLE)
            return;

        if (pPort->P2PCfg.NoAIndex != MlmeSyncGetWlanIdxByPort(pPort, ROLE_WLANIDX_BSSID))
            DBGPRINT(RT_DEBUG_TRACE,("P2pGPTimeOutHandle. !bug, please check driver %d. \n", pPort->P2PCfg.NoAIndex));
            
        pWcidMacTabEntry = MlmeSyncMacTabMatchedWlanIdx(pPort, pPort->P2PCfg.NoAIndex); 
        if(pWcidMacTabEntry == NULL)
        {
            DBGPRINT(RT_DEBUG_TRACE,("%s(), Line:%d, Mac Entry is NULL  Wcid = %d\n", __FUNCTION__, __LINE__, pPort->P2PCfg.NoAIndex));
            return;
        }
        
        pEntry = pWcidMacTabEntry;
        if (pEntry->P2pInfo.NoADesc[0].bValid == TRUE)
        {
            if ((pEntry->P2pInfo.NoADesc[0].Count > 0) && (pEntry->P2pInfo.NoADesc[0].Count < 255))
            {
                // Sometimes go to awake, sometime go to silence. Two state counts One count down.
                // so only minus Count when I change from Sleep to Awake
                if (pEntry->P2pInfo.NoADesc[0].bInAwake == FALSE)
                    pEntry->P2pInfo.NoADesc[0].Count--;
            }
            if (pEntry->P2pInfo.NoADesc[0].Count == 0)
            {
                P2pStopNoA(pAd, pEntry);
                DBGPRINT(RT_DEBUG_TRACE,("P2pGPTimeOutHandle. Count down to zero!!StopGP.  return.1 \n"));
                return;
            }

            // To enter absence period, stop transmission a little bit earlier to leave HW to clean the queue.
            if (pEntry->P2pInfo.NoADesc[0].bInAwake == FALSE)
                NextDiff = pEntry->P2pInfo.NoADesc[0].Duration - 0x200;
            else
                NextDiff = pEntry->P2pInfo.NoADesc[0].Interval - pEntry->P2pInfo.NoADesc[0].Duration + 0x200;
                
            // Prepare next time.
            MacValue = 0x333;
            RTUSBReadMACRegister(pAd, TSF_TIMER_DW0, &MacValue);
            DBGPRINT(RT_DEBUG_INFO,("2 Tsf  Timer  = %d= 0x%x    NextTargetTimePoint = %d.\n", MacValue,  MacValue,pEntry->P2pInfo.NoADesc[0].NextTargetTimePoint));
            SavedNextTargetTimePoint = pEntry->P2pInfo.NoADesc[0].NextTargetTimePoint;
            if (MacValue <= pEntry->P2pInfo.NoADesc[0].NextTargetTimePoint)
            {
                GPDiff = pEntry->P2pInfo.NoADesc[0].NextTargetTimePoint - MacValue;
                pPort->P2PCfg.GONoASchedule.NextTimePointForWMMPSCounting = pEntry->P2pInfo.NoADesc[0].NextTargetTimePoint;
                pEntry->P2pInfo.NoADesc[0].NextTimePointForWMMPSCounting = pEntry->P2pInfo.NoADesc[0].NextTargetTimePoint;
                pEntry->P2pInfo.NoADesc[0].NextTargetTimePoint += NextDiff;
                P2pResetNoATimer(pAd, GPDiff, FALSE);
                DBGPRINT(RT_DEBUG_INFO,("3  Continue next NOA NextTargetTimePoint = %x. \n", pEntry->P2pInfo.NoADesc[0].NextTargetTimePoint));
                DBGPRINT(RT_DEBUG_INFO,("3  Value = %x.  NextDiff = %x.\n", MacValue, NextDiff));
            }
            else
            {
                P2pStopNoA(pAd, pEntry);
                DBGPRINT(RT_DEBUG_TRACE,("4  P2pGPTimeOutHandle.!!StopGP.  Error Case. shoudn't happen! \n"));
                DBGPRINT(RT_DEBUG_TRACE,("4  NOA NextTargetTimePoint = %d. \n", pEntry->P2pInfo.NoADesc[0].NextTargetTimePoint));
                DBGPRINT(RT_DEBUG_TRACE,("4  Value = %d = 0x%x.  NextDiff = %d.\n", MacValue,  MacValue, NextDiff));
                return;
            }
                
            if (pEntry->P2pInfo.NoADesc[0].bInAwake == TRUE)
            {
                pEntry->P2pInfo.NoADesc[0].bInAwake = FALSE;
                pPort->P2PCfg.bKeepSlient = TRUE;
                pPort->P2PCfg.bPreKeepSlient = TRUE;
                DBGPRINT(RT_DEBUG_TRACE,("Enter Absence now ======> %d\n", pPort->P2PCfg.bKeepSlient));
            }
            else
            {
                pEntry->P2pInfo.NoADesc[0].bInAwake = TRUE;
                pPort->P2PCfg.bKeepSlient = FALSE;
                pPort->P2PCfg.bPreKeepSlient = FALSE;
#if UAPSD_AP_SUPPORT
{
                INT         i;

                // [Testplan 7.1.5] WMM-PS + NoA
                // When Client wake up and receive MoreData before the last sleep, Client must send a QoS Null frame to GO as a re-triggered frame.
                if (pPort->CommonCfg.bAPSDCapable && pPort->CommonCfg.APEdcaParm.bAPSDCapable)
                {
                    for (i=0; i<WMM_NUM_OF_AC; i++)
                    {
                        if (pPort->P2PCfg.LastMoreDataFromGO[i] == TRUE)
                        {
                            DBGPRINT(RT_DEBUG_TRACE,("P2pGPTimeOutHandle!WMM-PS!Wakeup! QueId#%d sends a Qos Null frame to GO due to MoreData \n", i));             
                            XmitSendNullFrame(pAd, pAd->PortList[pPort->P2PCfg.PortNumber], pPort->CommonCfg.TxRate, TRUE);
                            // After sent a triggered frame, reset this record.
                            pPort->P2PCfg.LastMoreDataFromGO[i] = FALSE;
                        }   
                    }
                }
}
#endif              
                if ((pPort->CommonCfg.P2pControl.field.SwBasedNoA == 1)
                    && (pPort->P2PCfg.GONoASchedule.Count > 100))
                {
                    if (TRUE == P2pAdjustSwNoATimer(pAd, pPort, Value, SavedNextTargetTimePoint))
                    {
                        DBGPRINT(RT_DEBUG_TRACE,("SwBasedNoA : Dequeue here. %d\n", pAd->pP2pCtrll->TxSwNoAMgmtQueue.Number));
                        MTDeQueueNoAMgmtPacket(pAd);
                        NdisSetEvent(&(GET_THREAD_EVENT(&pAd->pHifCfg->hDequeueDataThread)));
                    }
                }
                else
                {
                    MTDeQueueNoAMgmtPacket(pAd);
                    NdisSetEvent(&(GET_THREAD_EVENT(&pAd->pHifCfg->hDequeueDataThread)));
                }
                DBGPRINT(RT_DEBUG_TRACE,("Enter Awake now ======= %d\n", pPort->P2PCfg.bKeepSlient));

            }

        }
                
    }
}


/*  
    ==========================================================================
    Description: 
        Check if turn on bBeaconGen bit.
        
    Parameters: 
         
    Note:
         
    ==========================================================================
 */
BOOLEAN P2pCheckBssSync(
    IN PMP_ADAPTER pAd)
{
    BCN_TIME_CFG_STRUC csr;
    BOOLEAN     Result = FALSE;

    csr.word = 0;
    RTUSBReadMACRegister(pAd, BCN_TIME_CFG, &csr.word);
    if (csr.field.bBeaconGen == 1)
        Result = TRUE;
    return Result;
}


/*  
    ==========================================================================
    Description: 
        OppPS CTWindows timer. 
        
    Parameters: 
    Note:
         
    ==========================================================================
 */
VOID P2PCTWindowTimerCallback(
    IN PVOID    SystemSpecific1, 
    IN PVOID    FunctionContext, 
    IN PVOID    SystemSpecific2, 
    IN PVOID    SystemSpecific3) 
{
    PMP_PORT pPort = (PMP_PORT )FunctionContext;
    PMP_ADAPTER  pAd = pPort->pAd;
    
    if (MT_TEST_BIT(pPort->P2PCfg.CTWindows, P2P_OPPS_BIT))
    {
        // Don't enter the slient period when CLI is in non-doze mode and GO has not all psm clients.
        if(IS_P2P_CLIENT_OP(pPort) && (pAd->StaCfg.WindowsPowerMode == DOT11_POWER_SAVING_NO_POWER_SAVING))
        {
            DBGPRINT(RT_DEBUG_TRACE, ("%s: CLI, Disable OPPS in non-doze mode\n", __FUNCTION__));
        }
        else if(IS_P2P_GO_OP(pPort) && (pPort->P2PCfg.P2pGOAllPsm == FALSE))
        {
            DBGPRINT(RT_DEBUG_TRACE, ("%s: GO, Not all Psm. Disable OPPS\n", __FUNCTION__));
        }
        else
        {
            DBGPRINT(RT_DEBUG_TRACE, ("%s: Enable OPPS (CTWindows= %d)\n", __FUNCTION__, pPort->P2PCfg.CTWindows));
            pPort->P2PCfg.bKeepSlient = TRUE;
        }   
    }
}

/*  
    ==========================================================================
    Description: 
        Maintain P2P State after scan. no matter this scan is P2P Search or normal Station site survey. 
        
    Parameters: 
        S - pointer to the association state machine
    Note:
        The state machine looks like the following as name implies its function
    ==========================================================================
 */
VOID P2pResumeMsduAction(
    IN PMP_ADAPTER pAd,
    IN PMP_PORT     pPort)
{
    if (P2P_OFF(pPort))
        return;

    if ((pPort->P2PCfg.P2PDiscoProvState == P2P_SEARCH_COMMAND) ||(pPort->P2PCfg.P2PDiscoProvState == P2P_SEARCH) )
        pPort->P2PCfg.P2PDiscoProvState = P2P_SEARCH_COMPLETE;
    else if ((pPort->P2PCfg.P2PDiscoProvState == P2P_LISTEN_COMMAND)||(pPort->P2PCfg.P2PDiscoProvState == P2P_LISTEN))
        pPort->P2PCfg.P2PDiscoProvState = P2P_LISTEN_COMPLETE;
    else if ((pPort->P2PCfg.P2PDiscoProvState == P2P_ENABLE_LISTEN_ONLY) || (pPort->P2PCfg.P2PDiscoProvState == P2P_IDLE))
    {
        // Normal scan and P2P discovery scan mighe be interleaved, 
        // To Reset Counter after scan.  
        pPort->P2PCfg.P2pCounter.Counter100ms = 0;
    }
    
    if (pPort->P2PCfg.P2PConnectState == P2P_DO_GO_NEG_DONE_CLIENT)
    {
        //pPort->P2PCfg.P2PConnectState = P2P_DO_GO_SCAN_OP_BEGIN;
        //pPort->P2PCfg.P2pCounter.Counter100ms = 0;
        pPort->P2PCfg.P2PConnectState = P2P_DO_GO_SCAN_DONE;
    }
    else if (pPort->P2PCfg.P2PConnectState == P2P_DO_GO_SCAN_OP_BEGIN)
    {
        pPort->P2PCfg.P2PConnectState = P2P_DO_GO_SCAN_OP_DONE;
        pPort->P2PCfg.P2pCounter.Counter100ms = 0;
    }
    else if (pPort->P2PCfg.P2PConnectState == P2P_DO_GO_SCAN_BEGIN)
        pPort->P2PCfg.P2PConnectState = P2P_DO_GO_SCAN_DONE;

    DBGPRINT(RT_DEBUG_TRACE,("SCAN done,  %s. \n", decodeP2PState(pPort->P2PCfg.P2PDiscoProvState)));
}

/*  
    ==========================================================================
    Description: 
        Before reinvoke a persistent group, copy persistent parameter to pPort->P2PCfg.. 
        
    Parameters: 
        Perstindex : the index for entry in Persistent Table.
    Note:
         
    ==========================================================================
 */
VOID P2pCopyPerstParmToCfg(
    IN PMP_ADAPTER pAd,
    IN PMP_PORT       pPort,
    IN UCHAR        Perstindex)
{
    RT_P2P_PERSISTENT_ENTRY *pEntry;
    
    if (Perstindex >= MAX_P2P_TABLE_SIZE)
        return;
    // p2pCfg   
    pEntry = &pAd->pP2pCtrll->P2PTable.PerstEntry[Perstindex];
    PlatformMoveMemory(pPort->P2PCfg.SSID, pEntry->Profile.SSID.Ssid, 32);
    pPort->P2PCfg.SSIDLen = (UCHAR)pEntry->Profile.SSID.SsidLength;
    PlatformMoveMemory(pPort->P2PCfg.PhraseKey, pEntry->Profile.Key, 64);
    pPort->P2PCfg.PhraseKeyLen = (UCHAR)pEntry->Profile.KeyLength;

    // WscControl   
    PlatformMoveMemory(&pPort->StaCfg.WscControl.WscProfile.Profile[0], &pEntry->Profile, sizeof(WSC_CREDENTIAL));
    PlatformMoveMemory(pPort->StaCfg.WscControl.WscAPBssid, &pEntry->Profile.MacAddr, MAC_ADDR_LEN);
    
    PlatformMoveMemory(pAd->PortList[pPort->P2PCfg.PortNumber]->PortCfg.Ssid, pEntry->Profile.SSID.Ssid, 32);
    pAd->PortList[pPort->P2PCfg.PortNumber]->PortCfg.SsidLen = (UCHAR)pEntry->Profile.SSID.SsidLength;

    if(pEntry->MyRule == P2P_IS_CLIENT)
    {
        PlatformMoveMemory(pPort->P2PCfg.Bssid, &pEntry->Addr[0], MAC_ADDR_LEN);// jesse
    }
}

/*  
    ==========================================================================
    Description: 
        Copy parameters from GUI to P2PCfg.. 
        
    Parameters: 
        S - pointer to the association state machine
    Note:
         
    ==========================================================================
 */
VOID P2pSetProfileParm(
    IN PMP_ADAPTER pAd, 
    IN PMP_PORT     pPort,
    IN PRT_OID_SET_P2P_STRUCT pP2pStruc) 
{
    UCHAR       RandomValue = 1;
    UCHAR       i = 0;
    
    DBGPRINT(RT_DEBUG_TRACE,("P2pSetProfile ====>  \n"));
    // Start as autonomous GO. 16 is defined by ourselves.
    if (pP2pStruc->GoIntentIdx <=15)
    {
        pPort->P2PCfg.GoIntentIdx = pP2pStruc->GoIntentIdx;
    }
    
    if ((INFRA_ON(pAd->PortList[PORT_0])) && (pAd->OpMode == OPMODE_STAP2P))
    {
        // When infra on in Concurrent mode, Op Channel follows Infra AP's channel.
        pPort->P2PCfg.GroupChannel = pPort->Channel;
        pPort->P2PCfg.GroupOpChannel = pPort->Channel;

        if (pP2pStruc->GoIntentIdx < 15)
        {
            // Elevate GO Intent to 14 when my port_0 is connected
            pPort->P2PCfg.GoIntentIdx = 14;
        }
        DBGPRINT(RT_DEBUG_TRACE,("P2pSetProfile . follow INFRA channel= %d intent = %d\n", pPort->Channel,pPort->P2PCfg.GoIntentIdx));
    }
    else if (pP2pStruc->OperatinChannel == 0)
    {
        //When the operation channel is zero, use a default value instead.
        DBGPRINT(RT_DEBUG_TRACE,("P2pSetProfile .  keep last channel = %d \n", pPort->P2PCfg.GroupChannel));
    }
    else
    {
        pPort->P2PCfg.GroupChannel = pP2pStruc->OperatinChannel;
        pPort->P2PCfg.GroupOpChannel = pP2pStruc->OperatinChannel;
        DBGPRINT(RT_DEBUG_TRACE,("P2pSetProfile . set op channel= %d \n", pPort->Channel));
    }

    if((INFRA_ON(pAd->PortList[PORT_0])) && (pAd->OpMode == OPMODE_STAP2P))
    {
        pPort->P2PCfg.ListenChannel = pPort->Channel;
    }
    else if ((pP2pStruc->ListenChannel == 1) ||(pP2pStruc->ListenChannel == 6) || (pP2pStruc->ListenChannel == 11))
    {
        // at first p2p enable the listen channel had set so here no need set again
        if(IS_P2P_SIGMA_ON(pPort))
            pPort->P2PCfg.ListenChannel = pP2pStruc->ListenChannel;
    }
    // Must be 9 according to P2P Spec.
    pPort->P2PCfg.SSIDLen = pP2pStruc->SSIDLen;
    pPort->P2PCfg.WscMode = pP2pStruc->WscMode;
    if (pPort->P2PCfg.WscMode == WSC_PIN_MODE_USER_SPEC)
    {
        pPort->P2PCfg.Dpid = DEV_PASS_ID_USER;
        pPort->P2PCfg.ConfigMethod = CONFIG_METHOD_KEYPAD;
        DBGPRINT(RT_DEBUG_TRACE,("P2pSetProfile .User Spec  Dpid = %x \n", pPort->P2PCfg.Dpid));
    }
    else if (pPort->P2PCfg.WscMode == WSC_PIN_MODE)
    {
        pPort->P2PCfg.Dpid = DEV_PASS_ID_PIN;
        pPort->P2PCfg.ConfigMethod =  CONFIG_METHOD_LABEL;
        DBGPRINT(RT_DEBUG_TRACE,("P2pSetProfile .Default PIN  Dpid = %x \n", pPort->P2PCfg.Dpid));
    }
    else if (pPort->P2PCfg.WscMode == WSC_PBC_MODE)
    {
        pPort->P2PCfg.Dpid = DEV_PASS_ID_PBC;
        pPort->P2PCfg.ConfigMethod = CONFIG_METHOD_PUSHBUTTON;
        DBGPRINT(RT_DEBUG_TRACE,("P2pSetProfile PBC  Dpid = %x \n", pPort->P2PCfg.Dpid));
    }
    else if (pPort->P2PCfg.WscMode == WSC_PIN_MODE_REGISTRA_SPEC)
    {
        pPort->P2PCfg.Dpid = DEV_PASS_ID_REG;
        pPort->P2PCfg.ConfigMethod = CONFIG_METHOD_DISPLAY;
        DBGPRINT(RT_DEBUG_TRACE,("P2pSetProfile Registra Spec  Dpid = %x \n", pPort->P2PCfg.Dpid));
    }
    else
        DBGPRINT(RT_DEBUG_TRACE,("P2pSetProfile   DPID is currently unavailable.  WscMode= %x \n", pP2pStruc->WscMode));
    
    DBGPRINT(RT_DEBUG_TRACE,("P2pSetProfile .ConfigMode = %x \n", pP2pStruc->ConfigMode));
        
    pPort->P2PCfg.P2pPhyMode = P2P_PHYMODE_ENABLE_11N_20;

    // Copy PIN Code with 8 digits or 4 digits
    if (pP2pStruc->WscMode == WSC_PBC_MODE)
    {
        pPort->P2PCfg.PINLen = 4; // 8 digits
        PlatformMoveMemory(pPort->P2PCfg.PinCode, pP2pStruc->PinCode, 8);
    }
    if ((pP2pStruc->WscMode == WSC_PIN_MODE_USER_SPEC)||
        (pP2pStruc->WscMode == WSC_PIN_MODE_REGISTRA_SPEC) ||
        (pP2pStruc->WscMode == WSC_PIN_MODE))
    {
        if((pP2pStruc->PinCode[0] == 0xff) && (pP2pStruc->PinCode[1] == 0xff) &&
            (pP2pStruc->PinCode[2] == 0xff) && (pP2pStruc->PinCode[3] == 0xff))
        {
            pPort->P2PCfg.PINLen = 2; // 4 digits
            PlatformMoveMemory(pPort->P2PCfg.PinCode, &pP2pStruc->PinCode[4], 4);
        }
        else
        {
            pPort->P2PCfg.PINLen = 4; // 8 digits
            PlatformMoveMemory(pPort->P2PCfg.PinCode, pP2pStruc->PinCode, 8);
        }   
    }

    //when persistent on and I have been GO
    //I will use last time GO SSID
    if((pPort->CommonCfg.P2pControl.field.EnablePresistent == 1) && (pAd->pP2pCtrll->P2PTable.PerstNumber != 0)  )
    {   
        for(i = 0; i < pAd->pP2pCtrll->P2PTable.PerstNumber; i++)
        {
            DBGPRINT(RT_DEBUG_TRACE, ("MyRule = %d", pAd->pP2pCtrll->P2PTable.PerstEntry[i].MyRule));
            if( pAd->pP2pCtrll->P2PTable.PerstEntry[i].MyRule == P2P_IS_GO)
            {
                 pPort->P2PCfg.SSID[8] = pAd->pP2pCtrll->P2PTable.PerstEntry[i].Profile.SSID.Ssid[8];
                 pPort->P2PCfg.SSID[9] = pAd->pP2pCtrll->P2PTable.PerstEntry[i].Profile.SSID.Ssid[9];
                 PlatformMoveMemory(pPort->P2PCfg.SSID, pAd->pP2pCtrll->P2PTable.PerstEntry[i].Profile.SSID.Ssid, 32);
                DBGPRINT(RT_DEBUG_TRACE, ("%d ssid %c %c", i, pAd->pP2pCtrll->P2PTable.PerstEntry[i].Profile.SSID.Ssid[8], pAd->pP2pCtrll->P2PTable.PerstEntry[i].Profile.SSID.Ssid[9]));
                break;
            }
        }
    }
    else
        PlatformMoveMemory(pPort->P2PCfg.SSID, pP2pStruc->SSID, 32);

    PlatformMoveMemory(pPort->P2PCfg.ConnectingDeviceName, pP2pStruc->ConnectingDeviceName, MAX_P2P_GROUP_SIZE*32);
    PlatformMoveMemory(pPort->StaCfg.WscControl.RegData.EnrolleeInfo.DeviceName, pP2pStruc->DeviceName, 32);
    PlatformMoveMemory(pPort->StaCfg.WscControl.RegData.RegistrarInfo.DeviceName, pP2pStruc->DeviceName, 32);
    
    DBGPRINT(RT_DEBUG_TRACE,("P2pSetProfile  preferred OpChannel = %d. Listen Channel in use= %d pP2pStruc->ListenChannel = %d\n", pPort->P2PCfg.GroupChannel, pPort->P2PCfg.ListenChannel, pP2pStruc->ListenChannel));
    DBGPRINT(RT_DEBUG_TRACE,("P2pSetProfile  Intent = %d \n", pP2pStruc->GoIntentIdx));
    DBGPRINT(RT_DEBUG_TRACE,("P2pSetProfile  P2PGroupMode = %d \n", pP2pStruc->P2PGroupMode));
    DBGPRINT(RT_DEBUG_TRACE,("P2pSetProfile  SsidLen = %d. Ssid = %s \n", pPort->P2PCfg.SSIDLen, pPort->P2PCfg.SSID));
    DBGPRINT(RT_DEBUG_TRACE,("P2pSetProfile  GroupMode = %d \n", pP2pStruc->P2PGroupMode));
    DBGPRINT(RT_DEBUG_TRACE,("P2pSetProfile  WscMode = %x \n", pP2pStruc->WscMode));
    DBGPRINT(RT_DEBUG_TRACE,("P2pSetProfile  Pin = %x %x \n", *(PULONG)&pP2pStruc->PinCode[0], *(PULONG)&pP2pStruc->PinCode[4]));
    DBGPRINT(RT_DEBUG_TRACE,("P2pSetProfile  KeyLen = %x. Key =  %x %x\n", pPort->P2PCfg.PhraseKeyLen, *(PULONG)&pPort->P2PCfg.PhraseKey[0], *(PULONG)&pPort->P2PCfg.PhraseKey[4]));
    DBGPRINT(RT_DEBUG_TRACE,("P2pSetProfile  MacList =  %x %x %x %x %x %x \n", pPort->P2PCfg.ConnectingMAC[0][0], pPort->P2PCfg.ConnectingMAC[0][1], 
        pPort->P2PCfg.ConnectingMAC[0][2], pPort->P2PCfg.ConnectingMAC[0][3], pPort->P2PCfg.ConnectingMAC[0][4], pPort->P2PCfg.ConnectingMAC[0][5]));
    DBGPRINT(RT_DEBUG_TRACE,("P2pSetProfile  DeviceList =  %c %c %c %c %c %c ..\n", pPort->P2PCfg.ConnectingDeviceName[0][0], pPort->P2PCfg.ConnectingDeviceName[0][1], 
        pPort->P2PCfg.ConnectingDeviceName[0][2], pPort->P2PCfg.ConnectingDeviceName[0][3], pPort->P2PCfg.ConnectingDeviceName[0][4], pPort->P2PCfg.ConnectingDeviceName[0][5]));
    DBGPRINT(RT_DEBUG_TRACE,("P2pSetProfile  My Device Name %c %c %c %c %c %c %c %c %c %c... \n", pPort->P2PCfg.DeviceName[0], pPort->P2PCfg.DeviceName[1], 
        pPort->P2PCfg.DeviceName[2], pPort->P2PCfg.DeviceName[3], pPort->P2PCfg.DeviceName[4], pPort->P2PCfg.DeviceName[5]
        , pPort->P2PCfg.DeviceName[6], pPort->P2PCfg.DeviceName[7], pPort->P2PCfg.DeviceName[8], pPort->P2PCfg.DeviceName[9]));
}

/*  
    ==========================================================================
    Description: 
        Copy parameters from GUI to P2PCfg.. 
        
    Parameters: 
        S - pointer to the association state machine
    Note:
         
    ==========================================================================
 */
VOID P2pGOSetProfileParm(
    IN PMP_ADAPTER pAd, 
    IN PMP_PORT     pPort,
    IN PRT_OID_SET_P2P_STRUCT pP2pStruc) 
{
    UCHAR       RandomValue = 1;

    DBGPRINT(RT_DEBUG_TRACE,("P2pGOSetProfileParm ====>  \n"));

    pPort->P2PCfg.WscMode = pP2pStruc->WscMode;
    if (pPort->P2PCfg.WscMode == WSC_PIN_MODE_USER_SPEC)
    {
        pPort->P2PCfg.Dpid = DEV_PASS_ID_USER;
        pPort->P2PCfg.ConfigMethod = CONFIG_METHOD_KEYPAD;
        DBGPRINT(RT_DEBUG_TRACE,("P2pGOSetProfileParm .User Spec  Dpid = %x \n", pPort->P2PCfg.Dpid));
    }
    else if (pPort->P2PCfg.WscMode == WSC_PIN_MODE)
    {
        pPort->P2PCfg.Dpid = DEV_PASS_ID_PIN;
        pPort->P2PCfg.ConfigMethod =  CONFIG_METHOD_LABEL;
        DBGPRINT(RT_DEBUG_TRACE,("P2pGOSetProfileParm .Default PIN  Dpid = %x \n", pPort->P2PCfg.Dpid));
    }
    else if (pPort->P2PCfg.WscMode == WSC_PIN_MODE_REGISTRA_SPEC)
    {
        pPort->P2PCfg.Dpid = DEV_PASS_ID_REG;
        pPort->P2PCfg.ConfigMethod = CONFIG_METHOD_DISPLAY;
        DBGPRINT(RT_DEBUG_TRACE,("P2pGOSetProfileParm Registra Spec  Dpid = %x \n", pPort->P2PCfg.Dpid));
    }
    else
    {
        pPort->P2PCfg.Dpid = DEV_PASS_ID_PBC;
        pPort->P2PCfg.ConfigMethod = CONFIG_METHOD_PUSHBUTTON;
        DBGPRINT(RT_DEBUG_TRACE,("P2pGOSetProfileParm PBC  Dpid = %x \n", pPort->P2PCfg.Dpid));
    }
    
    DBGPRINT(RT_DEBUG_TRACE,("P2pSetProfile .ConfigMode = %x \n", pP2pStruc->ConfigMode));
        
    pPort->P2PCfg.P2pPhyMode = P2P_PHYMODE_ENABLE_11N_20;

    // Copy PIN Code with 8 digits or 4 digits
    if (pP2pStruc->WscMode == WSC_PBC_MODE)
    {
        pPort->P2PCfg.PINLen = 4; // 8 digits
        PlatformMoveMemory(pPort->P2PCfg.PinCode, pP2pStruc->PinCode, 8);
    }
    if ((pP2pStruc->WscMode == WSC_PIN_MODE_USER_SPEC)||
        (pP2pStruc->WscMode == WSC_PIN_MODE_REGISTRA_SPEC) ||
        (pP2pStruc->WscMode == WSC_PIN_MODE))
    {
        if((pP2pStruc->PinCode[0] == 0xff) && (pP2pStruc->PinCode[1] == 0xff) &&
            (pP2pStruc->PinCode[2] == 0xff) && (pP2pStruc->PinCode[3] == 0xff))
        {
            pPort->P2PCfg.PINLen = 2; // 4 digits
            PlatformMoveMemory(pPort->P2PCfg.PinCode, &pP2pStruc->PinCode[4], 4);
        }
        else
        {
            pPort->P2PCfg.PINLen = 4; // 8 digits
            PlatformMoveMemory(pPort->P2PCfg.PinCode, pP2pStruc->PinCode, 8);
        }   
    }

    PlatformMoveMemory(pPort->P2PCfg.ConnectingDeviceName, pP2pStruc->ConnectingDeviceName, MAX_P2P_GROUP_SIZE*32);
    
    DBGPRINT(RT_DEBUG_TRACE,("P2pSetProfile  OpChannel = %d. Listen Channel = %d \n", pPort->P2PCfg.GroupChannel, pPort->P2PCfg.ListenChannel));
    DBGPRINT(RT_DEBUG_TRACE,("P2pSetProfile  Intent = %d \n", pP2pStruc->GoIntentIdx));
    DBGPRINT(RT_DEBUG_TRACE,("P2pSetProfile  P2PGroupMode = %d \n", pP2pStruc->P2PGroupMode));
    DBGPRINT(RT_DEBUG_TRACE,("P2pSetProfile  SsidLen = %d. Ssid = %s \n", pPort->P2PCfg.SSIDLen, pPort->P2PCfg.SSID));
    DBGPRINT(RT_DEBUG_TRACE,("P2pSetProfile  GroupMode = %d \n", pP2pStruc->P2PGroupMode));
    DBGPRINT(RT_DEBUG_TRACE,("P2pSetProfile  WscMode = %x \n", pP2pStruc->WscMode));
    DBGPRINT(RT_DEBUG_TRACE,("P2pSetProfile  Pin = %x %x \n", *(PULONG)&pP2pStruc->PinCode[0], *(PULONG)&pP2pStruc->PinCode[4]));
    DBGPRINT(RT_DEBUG_TRACE,("P2pSetProfile  KeyLen = %x. Key =  %x %x\n", pPort->P2PCfg.PhraseKeyLen, *(PULONG)&pPort->P2PCfg.PhraseKey[0], *(PULONG)&pPort->P2PCfg.PhraseKey[4]));
    DBGPRINT(RT_DEBUG_TRACE,("P2pSetProfile  MacList =  %x %x %x %x %x %x \n", pPort->P2PCfg.ConnectingMAC[0][0], pPort->P2PCfg.ConnectingMAC[0][1], 
        pPort->P2PCfg.ConnectingMAC[0][2], pPort->P2PCfg.ConnectingMAC[0][3], pPort->P2PCfg.ConnectingMAC[0][4], pPort->P2PCfg.ConnectingMAC[0][5]));
    DBGPRINT(RT_DEBUG_TRACE,("P2pSetProfile  DeviceList =  %c %c %c %c %c %c ..\n", pPort->P2PCfg.ConnectingDeviceName[0][0], pPort->P2PCfg.ConnectingDeviceName[0][1], 
        pPort->P2PCfg.ConnectingDeviceName[0][2], pPort->P2PCfg.ConnectingDeviceName[0][3], pPort->P2PCfg.ConnectingDeviceName[0][4], pPort->P2PCfg.ConnectingDeviceName[0][5]));
    DBGPRINT(RT_DEBUG_TRACE,("P2pSetProfile  My Device Name %c %c %c %c %c %c %c %c %c %c... \n", pPort->P2PCfg.DeviceName[0], pPort->P2PCfg.DeviceName[1], 
        pPort->P2PCfg.DeviceName[2], pPort->P2PCfg.DeviceName[3], pPort->P2PCfg.DeviceName[4], pPort->P2PCfg.DeviceName[5]
        , pPort->P2PCfg.DeviceName[6], pPort->P2PCfg.DeviceName[7], pPort->P2PCfg.DeviceName[8], pPort->P2PCfg.DeviceName[9]));
}

/*  
    ==========================================================================
    Description: 
        Called when GUI apply a P2P profile. 
        
    Parameters: 
        S - pointer to the association state machine
    Note:
        The state machine looks like the following as name implies its function
    ==========================================================================
 */
VOID P2pSigmaSetProfile(
    IN PMP_ADAPTER pAd,
    IN PMP_PORT     pPort,
    IN ULONG InformationBufferLength,
    IN PVOID pInformationBuffer) 
{
    PRT_OID_SET_P2P_STRUCT      pP2pStruc;
//    PMP_PORT          pPort;
    UCHAR       i;
    UCHAR       p2pindex = 0;
    ULONG       Value;
    BOOLEAN     IsFromSigma = FALSE;
    UCHAR       allzero[6];
    
    
    
    pP2pStruc = (PRT_OID_SET_P2P_STRUCT)pInformationBuffer;

    PlatformZeroMemory(allzero, MAC_ADDR_LEN);
    IsFromSigma = ((pP2pStruc->ConfigMode) & 0xf0) > 0 ? TRUE : FALSE;
    
    pPort->P2PCfg.SigmaSetting.ConfigMode = pP2pStruc->ConfigMode;
    switch (pP2pStruc->ConfigMode)
    {   
        case CONFIG_MODE_SIGMA_SEND_P2P_SRVDISCO_REQ:
            DBGPRINT(RT_DEBUG_TRACE,("P2pSetProfile :: ServiceReqCmd \n"));
            p2pindex = P2pGroupTabSearch(pAd, &pP2pStruc->ConnectingMAC[0][0]);
            if (p2pindex < MAX_P2P_GROUP_SIZE)
            {
                pAd->pP2pCtrll->P2PTable.Client[p2pindex].P2pClientState = P2PSTATE_SERVICE_DISCO_COMMAND;
            }
            else
            {
                p2pindex = P2pGroupTabInsert(pAd, &pP2pStruc->ConnectingMAC[0][0],P2PSTATE_SERVICE_DISCO_COMMAND, NULL, 0);
            }
            pPort->P2PCfg.P2pCounter.CounterAftrScanButton = 0;
            P2pGotoScan(pAd, pPort, P2P_DISCO_TYPE_DEFAULT, 2);

            break;  
        case CONFIG_MODE_SIGMA_CHANGE_P2PCTRL_SETTING:
            DBGPRINT(RT_DEBUG_TRACE,("P2pSetProfile ::CONFIG_MODE_SIGMA_CHANGE_P2PCTRL_SETTING\n"));
            P2PUpdateCapability(pAd, pPort);
            break;
        case CONFIG_MODE_SIGMA_SET_WPS:
            DBGPRINT(RT_DEBUG_TRACE,("P2pSetProfile ::CONFIG_MODE_SIGMA_SET_WPS\n"));
            P2pSetGoWps(pAd, pP2pStruc);

            if (IS_P2P_CONNECT_IDLE(pPort))
            {
                if (!PlatformEqualMemory(allzero, pPort->P2PCfg.ConnectingMAC, MAC_ADDR_LEN))
                {
                    P2pConnect(pAd);
                }
            }
            DBGPRINT(RT_DEBUG_TRACE,("P2pSetProfile :: Set P2P WPS. My Dpid = %x \n", pPort->P2PCfg.Dpid));
            break;

        case CONFIG_MODE_SIGMA_SEND_P2P_PROVISION_DIS_REQ:
            DBGPRINT(RT_DEBUG_TRACE,("P2pSetProfile :: Sigma provisioning P2P. My Dpid = %x \n",  pPort->P2PCfg.Dpid));
            P2pProvision(pAd, pPort, &pP2pStruc->ConnectingMAC[0][0]);
            break;          

        case CONFIG_MODE_SIGMA_SET_P2P_DISABLE:
            DBGPRINT(RT_DEBUG_TRACE,("P2pSetProfile ::Sigma Disable P2P.  Opmode = %d. \n", pAd->OpMode));

            // we often use GoIntentIdx == 16 to know I was set to be AutoGo. Since this is a disconnet comand.
            // I must set GoIntentIdx to a vlaue !=16.
            pPort->P2PCfg.GoIntentIdx = 5;

            P2pDown(pAd, pAd->PortList[pPort->P2PCfg.PortNumber]);
            // disable P2P function.
            P2pStopGo(pAd, pAd->PortList[pPort->P2PCfg.PortNumber]); 
            P2pGroupTabInit(pAd);
            pPort->P2PCfg.P2PConnectState = P2P_CONNECT_NOUSE;
            pPort->P2PCfg.P2PDiscoProvState = P2P_DISABLE;
            DBGPRINT(RT_DEBUG_TRACE,("P2pSetProfile ::  Disable P2P <==== \n"));
            RTUSBReadMACRegister(pAd, INT_TIMER_EN, &Value);
            Value &= (0xfffffffd);
            RTUSBWriteMACRegister(pAd, INT_TIMER_EN, Value);
            // Reset port type
            if ((pAd->OpMode == OPMODE_STA) || (pAd->OpMode == OPMODE_STAP2P))
            {
                pAd->PortList[pPort->P2PCfg.PortNumber]->PortType = EXTSTA_PORT;
                pAd->PortList[pPort->P2PCfg.PortNumber]->PortSubtype = PORTSUBTYPE_STA;
                DBGPRINT(RT_DEBUG_TRACE,("Port[%d] subtype is %d.  \n", pPort->P2PCfg.PortNumber, pAd->PortList[pPort->P2PCfg.PortNumber]->PortSubtype));
            }
            break;

        case CONFIG_MODE_SIGMA_P2P_DISSOLVE:
            DBGPRINT(RT_DEBUG_TRACE,("P2pSetProfile : Sigma Dissolve.====> \n"));

            // If Device is Client then disconnect from current P2P Group. 
            // If the Device is Group Owner then disconnect all the Clients connected to it and dissolve the P2P Group.
            DBGPRINT(RT_DEBUG_TRACE,("P2pSetProfile : SIGMA_P2P_DISSOLVE [GroupBssid=%02x:%02x:%02x:%02x:%02x:%02x, GroupSsidLen = %d, GroupSsid=%s]\n", 
                            pP2pStruc->ConnectingMAC[1][0], pP2pStruc->ConnectingMAC[1][1], pP2pStruc->ConnectingMAC[1][2], pP2pStruc->ConnectingMAC[1][3], 
                            pP2pStruc->ConnectingMAC[1][4], pP2pStruc->ConnectingMAC[1][5], pP2pStruc->SSIDLen, pP2pStruc->SSID)); 

            // we often use GoIntentIdx == 16 to know I was set to be AutoGo. Since this is a disconnet comand.
            // I must set GoIntentIdx to a vlaue !=16.
            pPort->P2PCfg.GoIntentIdx = 5;

            P2pDown(pAd, pAd->PortList[pPort->P2PCfg.PortNumber]);
            if (P2pSetEvent(pAd, pPort,  P2PEVENT_DISCONNECT) == FALSE)
            {
                pPort->P2PCfg.P2pEventQueue.bDeleteWirelessCon = TRUE;    
            }
            if (pAd->PortList[pPort->P2PCfg.PortNumber]->PortSubtype == PORTSUBTYPE_P2PGO)
                P2pStopGo(pAd, pAd->PortList[pPort->P2PCfg.PortNumber]);
            // Delete P2P Device that I previously tried to connect.
            for (i = 0;i < MAX_P2P_GROUP_SIZE;i++)
            {
                if (pAd->pP2pCtrll->P2PTable.Client[i].P2pClientState > P2PSTATE_DISCOVERY_UNKNOWN)
                    P2pGroupTabDelete(pAd, i, pAd->pP2pCtrll->P2PTable.Client[i].addr);
            }
    
            pPort->P2PCfg.P2PDiscoProvState = P2P_ENABLE_LISTEN_ONLY;
            pPort->P2PCfg.P2pCounter.NextScanRound = (RandomByte(pAd) % P2P_RANDOM_BASE) + 3*P2P_RANDOM_BIAS;
            // stay in listen channel           
            if ((pAd->OpMode == OPMODE_STA) && (!INFRA_ON(pAd->PortList[PORT_0])) && (!ADHOC_ON(pAd->PortList[PORT_0])))
            {
                //AsicSwitchChannel(pAd, pPort->P2PCfg.ListenChannel, FALSE);
                AsicSwitchChannel(pPort, pPort->P2PCfg.ListenChannel, pPort->P2PCfg.ListenChannel, BW_20, FALSE);
            }
            break;
    
        case CONFIG_MODE_SIGMA_START_AUTONOMOUS_GO: 
        case CONFIG_MODE_SIGMA_START_GROUP_FORMATION_REQ:
        case CONFIG_MODE_SIGMA_WAIT_GROUP_FORMATION_REQ:
        case CONFIG_MODE_SIGMA_P2P_CONNECT:
        case CONFIG_MODE_SIGMA_SEND_P2P_INVITATION_REQ:
            // GUI set this when press "connect" button.
            DBGPRINT(RT_DEBUG_TRACE,("P2pSetProfile : Sigma Connect to a P2P Device. ====> \n"));

            // For Invitation Request check
            if (pP2pStruc->ConfigMode == CONFIG_MODE_SIGMA_SEND_P2P_INVITATION_REQ)
            {
                DBGPRINT(RT_DEBUG_TRACE,("P2pSetProfile : Send P2P Invitation Request [EnablePresistent=%d, EnableInvite=%d, P2PGroupMode=%d]\n", 
                            pPort->CommonCfg.P2pControl.field.EnablePresistent, pPort->CommonCfg.P2pControl.field.EnableInvite, pP2pStruc->P2PGroupMode));

                DBGPRINT(RT_DEBUG_TRACE,("P2pSetProfile :   [GroupBssid=%02x:%02x:%02x:%02x:%02x:%02x,  GroupSsidLen = %d, GroupSsid=%s]\n", 
                            pP2pStruc->ConnectingMAC[1][0], pP2pStruc->ConnectingMAC[1][1], pP2pStruc->ConnectingMAC[1][2], pP2pStruc->ConnectingMAC[1][3], pP2pStruc->ConnectingMAC[1][4], pP2pStruc->ConnectingMAC[1][5],
                            pP2pStruc->SSIDLen, pP2pStruc->SSID)); 

                if (pPort->CommonCfg.P2pControl.field.EnableInvite == 0)
                    break;
                
            }

            // Before we do every test, Sigam will stop all working P2P and reset to default. 
            // So no need to delete GroupTable in the connecting action.
            
            // ### Update SSID.
            if (pP2pStruc->SSIDLen != 0)
            {
                PlatformMoveMemory(pPort->P2PCfg.SSID, pP2pStruc->SSID, 32);
                pPort->P2PCfg.SSIDLen= pP2pStruc->SSIDLen;
                DBGPRINT(RT_DEBUG_TRACE,("*** SsidLen = %d. Ssid = %s \n", pPort->P2PCfg.SSIDLen, pPort->P2PCfg.SSID));
            }
            
            // For Group Formation...   
            if ((pP2pStruc->ConfigMode == CONFIG_MODE_SIGMA_START_GROUP_FORMATION_REQ) ||
                (pP2pStruc->ConfigMode == CONFIG_MODE_SIGMA_WAIT_GROUP_FORMATION_REQ))
            {
                // ### Update intent
                if (pP2pStruc->GoIntentIdx <=15)
                {
                    pPort->P2PCfg.GoIntentIdx = pP2pStruc->GoIntentIdx;
                }

                if (pP2pStruc->ConfigMode == CONFIG_MODE_SIGMA_WAIT_GROUP_FORMATION_REQ)
                    pPort->P2PCfg.SigmaSetting.bWaitGoNegReq = TRUE;
                else
                    pPort->P2PCfg.SigmaSetting.bWaitGoNegReq = FALSE;

                DBGPRINT(RT_DEBUG_TRACE,("P2pSetProfile : SigmaSetting.bWaitGoNegReq = %d\n", pPort->P2PCfg.SigmaSetting.bWaitGoNegReq));
            }

            // ### Update Operation Channel

            if ((INFRA_ON(pAd->PortList[PORT_0])) && (pAd->OpMode == OPMODE_STAP2P))
            {
                // When infra on in Concurrent mode, Op Channel follows Infra AP's channel.
            }
            else if (pP2pStruc->OperatinChannel == 0)
            {
                //When the operation channel is zero, use a default value instead.
            }
            else
            {
                pPort->P2PCfg.GroupChannel = pP2pStruc->OperatinChannel;
            }
            DBGPRINT(RT_DEBUG_TRACE, ("2PortSubtype = %d \n", pPort->PortSubtype));

            // ### Update P2P Cap in concurrent mode
            if (pAd->OpMode == OPMODE_STAP2P)
            {
                pPort->P2PCfg.P2pCapability[0] |= DEVCAP_CLIENT_CONCURRENT;
                if (IS_P2P_GO_OP(pPort))
                    pPort->P2PCfg.P2pCapability[1] |= GRPCAP_OWNER;
            }

            // 
            if ((!IS_P2P_GO_OP(pPort)) && (!IS_P2P_CLIENT_OP(pPort)))
            {
                // Prepare WPA2PSK AES IE.
                RTMPAPMakeRSNIE(pAd, pPort, Ralink802_11AuthModeWPA2PSK, Ralink802_11Encryption3Enabled);

                if (pAd->OpMode == OPMODE_STAP2P)
                {
                    // Set multi-bssid
                    MtAsicSetMultiBssidEx(pAd, pPort->CurrentAddress, (UCHAR)pPort->PortNumber);
                    // win7 doesn't allow adhoc connection when virtula port is active. So doesn't check adhoc.
                    if (INFRA_ON(pAd->PortList[PORT_0]))
                    {
                        P2pDecidePhyMode(pAd, pPort);
                    }
                }

                P2pScanChannelDefault(pAd, pPort);
            }
            P2PPrintState(pAd);
            
            // Start Autonomous GO mode.
            if (pP2pStruc->GoIntentIdx == 16)
            {
                pPort->P2PCfg.GoIntentIdx = pP2pStruc->GoIntentIdx;
                pPort->P2PCfg.GroupOpChannel = pPort->P2PCfg.GroupChannel;
                P2pStartAutoGo(pAd, pPort);
            }
            else
            {
                PlatformZeroMemory(pPort->P2PCfg.ConnectingMAC, MAX_P2P_GROUP_SIZE*6);
                // Sigma only set one mac address at a time. So only move 6 bytes
                PlatformMoveMemory(&pPort->P2PCfg.ConnectingMAC[0][0], pP2pStruc->ConnectingMAC, 6);
                // Set to 0xff means to connect to first Connecting MAC
                pPort->P2PCfg.ConnectingIndex = 0xff;
                DBGPRINT(RT_DEBUG_TRACE,("P2pSetProfile : Connect to the P2P %x %x %x %x %x %x \n", pP2pStruc->ConnectingMAC[0][0],pP2pStruc->ConnectingMAC[0][1],pP2pStruc->ConnectingMAC[0][2],pP2pStruc->ConnectingMAC[0][3],pP2pStruc->ConnectingMAC[0][4],pP2pStruc->ConnectingMAC[0][5]));
                P2pConnect(pAd);
            }
            break;
                                                

        case CONFIG_MODE_SIGMA_P2P_RESET_DEFAULT:
            DBGPRINT(RT_DEBUG_TRACE,("P2pSetProfile : SIGMA_P2P_RESET_DEFAULT====> \n"));

            // Before that, disable P2P via CONFIG_MODE_SIGMA_SET_P2P_DISABLE first.
            //
            //  ~~~~~~~~~~~~ Stay in DISABLE state ~~~~~~~~~~~~
            
            // Queue 'Delete Wireless Connection' event when We start to each wifi test item.
            // Make sure there is no profile in the 'wireless network connection'.
            DBGPRINT(RT_DEBUG_TRACE,(">>>current bDeleteWirelessCon = %d \n", pPort->P2PCfg.P2pEventQueue.bDeleteWirelessCon));
            P2pSetEvent(pAd, pPort,  P2PEVENT_SIGMA_IS_ON);
            if (P2pSetEvent(pAd, pPort,  P2PEVENT_DISCONNECT) == FALSE)
            {
                pPort->P2PCfg.P2pEventQueue.bDeleteWirelessCon = TRUE;    
            }

            // Clear previous Opps/NoA state
            pPort->P2PCfg.bKeepSlient = FALSE;
            
            // Here, delete Persisten Table that stored in Driver !!!
            P2pPerstTabClean(pAd);

            // Set the default ProfileParm
            P2pSetProfileParm(pAd, pPort, pP2pStruc);
            P2pSetDefaultGOHt(pAd, pPort);
            // Init Sigma Parameters
            P2PInitSigmaParam(pAd, pPort);
            P2pStopConnectAction(pAd, pPort);

            // Disable ACK check for Sigma tests
            pPort->CommonCfg.bP2pAckCheck = FALSE;

            // For Sigma Test 6.1.10
            pPort->P2PCfg.DevDisReqCount = 0;
            // For Sigma Test 6.1.11
            pPort->P2PCfg.bSigmaFakePkt = FALSE;
            break;
        case CONFIG_MODE_SIGMA_SET_P2P_IDLE:    
        case CONFIG_MODE_SIGMA_SET_P2P_LISTEN:  
        case CONFIG_MODE_SIGMA_SET_P2P_DISCOVERY:
            DBGPRINT(RT_DEBUG_TRACE,("P2pSetProfile : SIGMA_SET_P2P_LISTEN & DISCOVERY ====> \n"));
            if  (P2P_OFF(pPort))
            {
                // Init Token
                pPort->P2PCfg.bKeepSlient = FALSE;
                pPort->P2PCfg.Token = 0;
                // Only Device Name is passed from GUI when Config mode is Enable P2P.
                // Listen 
                PlatformMoveMemory(pPort->P2PCfg.DeviceName, pP2pStruc->DeviceName, 32);
                pPort->P2PCfg.DeviceNameLen = pP2pStruc->DeviceNameLen;

                P2pEnable(pAd);
                // Update ListenChannel
                if (pP2pStruc->ListenChannel != 0)
                {
                    pPort->P2PCfg.ListenChannel = (UCHAR) pP2pStruc->ListenChannel;
                    P2pDefaultListenChannel(pAd, pPort);
                    DBGPRINT(RT_DEBUG_TRACE, ("Sigma Set:  ListenChannel: %d \n", pPort->P2PCfg.ListenChannel));
                }
    
                DBGPRINT(RT_DEBUG_TRACE,("P2pSetProfile ::  Enable P2P.  DeviceNameLen = %d.  \n", pPort->P2PCfg.DeviceNameLen));
                DBGPRINT(RT_DEBUG_TRACE,("P2pSetProfile ::  Enable.  = %c%c%c%c%c%c.\n", pPort->P2PCfg.DeviceName[0], pPort->P2PCfg.DeviceName[1],pPort->P2PCfg.DeviceName[2],pPort->P2PCfg.DeviceName[3],pPort->P2PCfg.DeviceName[4],pPort->P2PCfg.DeviceName[5]));        

                // Do scanning when p2p enables.
                if (pPort->P2PCfg.P2pCounter.CounterAftrScanButton >= P2P_SCAN_PERIOD)
                {
                    DBGPRINT(RT_DEBUG_TRACE,("Scan....after Sigma enables P2P. \n"));

                    for (i = 0;i < MAX_P2P_GROUP_SIZE;i++)
                    {
                        if ((pAd->pP2pCtrll->P2PTable.Client[i].P2pClientState >= P2PSTATE_DISCOVERY)
                            && (pAd->pP2pCtrll->P2PTable.Client[i].P2pClientState <= P2PSTATE_DISCOVERY_CLIENT))
                            P2pGroupTabDelete(pAd, i, pAd->pP2pCtrll->P2PTable.Client[i].addr);
                    }
                    P2pSetEvent(pAd, pPort,  P2PEVENT_P2PGROUP_TOPOLOGY_UPDATE);
                }
                P2pScanChannelDefault(pAd, pPort);

            }
            
            if (pP2pStruc->ConfigMode == CONFIG_MODE_SIGMA_SET_P2P_LISTEN)
            {
                DBGPRINT(RT_DEBUG_TRACE,("P2pSetProfile : CONFIG_MODE_SIGMA_SET_P2P_LISTEN  ====> \n"));
    
                pPort->P2PCfg.P2PDiscoProvState = P2P_ENABLE_LISTEN_ONLY;
                pPort->P2PCfg.P2pCounter.NextScanRound = (RandomByte(pAd) % P2P_RANDOM_BASE) + 3*P2P_RANDOM_BIAS;
                if (IS_P2P_SUPPORT_EXT_LISTEN(pAd, pPort))
                    pPort->P2PCfg.P2pCounter.NextScanRound = pPort->P2PCfg.ExtListenInterval/100;

                // Update ListenChannel
                if (pP2pStruc->ListenChannel != 0)
                    pPort->P2PCfg.ListenChannel = (UCHAR) pP2pStruc->ListenChannel;

                // stay in listen channel
                if ((pAd->OpMode == OPMODE_STA) && (!INFRA_ON(pAd->PortList[PORT_0])) && (!ADHOC_ON(pAd->PortList[PORT_0])))
                {
                    //AsicSwitchChannel(pAd, pPort->P2PCfg.ListenChannel, FALSE);
                    AsicSwitchChannel(pPort, pPort->P2PCfg.ListenChannel, pPort->P2PCfg.ListenChannel, BW_20, FALSE);
                }
            }
            else if (pP2pStruc->ConfigMode == CONFIG_MODE_SIGMA_SET_P2P_DISCOVERY)
            {
                DBGPRINT(RT_DEBUG_TRACE,("P2pSetProfile : CONFIG_MODE_SIGMA_SET_P2P_DISCOVERY  ====> \n"));

                for (i = 0;i < MAX_P2P_GROUP_SIZE;i++)
                {
                    if ((pAd->pP2pCtrll->P2PTable.Client[i].P2pClientState >= P2PSTATE_DISCOVERY)
                        && (pAd->pP2pCtrll->P2PTable.Client[i].P2pClientState <= P2PSTATE_DISCOVERY_CLIENT))
                        P2pGroupTabDelete(pAd, i, pAd->pP2pCtrll->P2PTable.Client[i].addr);
                }
                //P2pSetEgtvent(pAd, P2PEVENT_P2PGROUP_TOPOLOGY_UPDATE);

                P2pScanChannelDefault(pAd, pPort);
                pPort->P2PCfg.P2pCounter.CounterAftrScanButton = 0;
                P2pGotoScan(pAd, pPort, P2P_DISCO_TYPE_DEFAULT, 2);
            }
            break;
        case CONFIG_MODE_SIGMA_P2P_SWITCH_OP_STA:
            DBGPRINT(RT_DEBUG_TRACE,("P2pSetProfile : SIGMA_P2P_SWITCH_OP_STA ====> \n"));
            if (P2pSetEvent(pAd, pPort,  P2PEVENT_SWITCH_OPMODE_STA) == FALSE)
            {
                pPort->P2PCfg.P2pEventQueue.bSwitchToSTA = TRUE;  
            }
            break;
        case CONFIG_MODE_SIGMA_P2P_SWITCH_OP_STAP2P:
            DBGPRINT(RT_DEBUG_TRACE,("P2pSetProfile : SIGMA_P2P_SWITCH_OP_STAP2P ====> \n"));
            if (P2pSetEvent(pAd, pPort,  P2PEVENT_SWITCH_OPMODE_STAP2P) == FALSE)
            {
                pPort->P2PCfg.P2pEventQueue.bSwitchToConcurrent = TRUE;   
            }
            break;
        case CONFIG_MODE_SIGMA_SEND_P2P_PRESENCE_REQ:
            DBGPRINT(RT_DEBUG_TRACE,("P2pSetProfile : SIGMA_SEND_P2P_PRESENCE_REQ ====> \n"));
            {
                UCHAR wcid = (UCHAR)MlmeSyncGetWlanIdxByPort(pPort, ROLE_WLANIDX_BSSID);
                
                DBGPRINT(RT_DEBUG_TRACE,("P2pSetProfile : Send the presence request to my GO. %x %x %x %x %x %x \n", 
                        pP2pStruc->ConnectingMAC[0][0],pP2pStruc->ConnectingMAC[0][1],pP2pStruc->ConnectingMAC[0][2],pP2pStruc->ConnectingMAC[0][3],pP2pStruc->ConnectingMAC[0][4],pP2pStruc->ConnectingMAC[0][5]));
                P2pActionSendPresenceReqCmd(pAd, pPort, wcid);
            }
            break;
        default:
            break;

    }

    P2PPrintState(pAd);

}

/*  
    ==========================================================================
    Description: 
        Called when GUI apply a P2P profile. 
        
    Parameters: 
        S - pointer to the association state machine
    Note:
        The state machine looks like the following as name implies its function
    ==========================================================================
 */
NDIS_STATUS P2pSetProfile(
    IN PMP_ADAPTER pAd, 
    IN PMP_PORT     pPort,
    IN ULONG InformationBufferLength,
    IN PVOID pInformationBuffer) 
{
    PRT_OID_SET_P2P_STRUCT      pP2pStruc;
    UCHAR       i;
    UCHAR       p2pindex = 0;
    ULONG       Value;
    BOOLEAN     Cancelled;
    PMAC_TABLE_ENTRY pMacEntry = NULL;
    PMAC_TABLE_ENTRY pNextMacEntry = NULL;   
    PQUEUE_HEADER pHeader;
    
    pP2pStruc = (PRT_OID_SET_P2P_STRUCT)pInformationBuffer;
    
    // P2PDiscoProvState
    if (P2P_OFF(pPort) && (pP2pStruc->ConfigMode != CONFIG_MODE_ENABLE_P2P) && 
        (pP2pStruc->ConfigMode != CONFIG_MODE_SIGMA_SET_P2P_LISTEN) && 
        (pP2pStruc->ConfigMode != CONFIG_MODE_SIGMA_P2P_RESET_DEFAULT) &&
        (pP2pStruc->ConfigMode != CONFIG_MODE_SIGMA_SET_P2P_DISCOVERY) &&
        (pP2pStruc->ConfigMode != CONFIG_MODE_SIGMA_P2P_SWITCH_OP_STA) &&
        (pP2pStruc->ConfigMode != CONFIG_MODE_SIGMA_P2P_SWITCH_OP_STAP2P)&& 
        (pP2pStruc->ConfigMode != CONFIG_MODE_NOTIFY_ICS_IS_ENABLED)&& 
        (pP2pStruc->ConfigMode != CONFIG_MODE_NOTIFY_ICS_IS_DISABLED)&& 
        (pP2pStruc->ConfigMode != CONFIG_MODE_SIGMA_SET_P2P_IDLE))
    {
        DBGPRINT(RT_DEBUG_TRACE,("P2pSetProfile :: P2P is ff. Reject P2P action %d. Please Enable P2P first. <==== \n", pP2pStruc->ConfigMode));
        return NDIS_STATUS_INVALID_LENGTH;
    }   
    else if (P2P_ON(pPort) && (pP2pStruc->ConfigMode == CONFIG_MODE_ENABLE_P2P))
    {
        DBGPRINT(RT_DEBUG_TRACE,("P2pSetProfile :: P2P is On. Reject a dual P2P On action %d.   \n", pP2pStruc->ConfigMode));
        return NDIS_STATUS_INVALID_LENGTH;
    }
    
    P2PPrintStateOnly(pAd, pPort);

    // We need to notify GUI we are under WiFi Sigma auto testing.
    if (IS_P2P_SIGMA_OFF(pPort) && (pP2pStruc->ConfigMode >= CONFIG_MODE_SIGMA_P2P_RESET_DEFAULT ))
    {
        if (FALSE == P2pSetEvent(pAd, pPort,  P2PEVENT_SIGMA_IS_ON))
            pPort->P2PCfg.P2pEventQueue.bSigmaIsOn = TRUE;
    }
    pPort->P2PCfg.LastConfigMode = pP2pStruc->ConfigMode;
    
    switch (pP2pStruc->ConfigMode)
    {
        case CONFIG_MODE_NOTIFY_ICS_IS_ENABLED:
            pPort->P2PCfg.P2pManagedParm.ICSStatus = ICS_STATUS_ENABLED;
            DBGPRINT(RT_DEBUG_TRACE,("P2pSetProfile ::CONFIG_MODE_NOTIFY_ICS_IS_ENABLED  \n"));
            if (pAd->OpMode == OPMODE_STAP2P)
            {
                pPort->P2PCfg.P2pCapability[1] |= GRPCAP_CROSS_CONNECT;
                DBGPRINT(RT_DEBUG_TRACE,("P2pSetProfile ::Turn on Cross Connect bit  \n"));
            }
            break;
        case CONFIG_MODE_NOTIFY_ICS_IS_DISABLED:
            pPort->P2PCfg.P2pManagedParm.ICSStatus = ICS_STATUS_DISABLED;
            DBGPRINT(RT_DEBUG_TRACE,("P2pSetProfile :: CONFIG_MODE_NOTIFY_ICS_IS_DISABLED \n"));
            break;

        case CONFIG_MODE_SET_GO_WPS:
            if (IS_P2P_SIGMA_ON(pPort))
                break;
            pPort->P2PCfg.P2pCounter.CounterP2PConnectionFail = 0;
            P2pSetGoWps(pAd, pP2pStruc);
            DBGPRINT(RT_DEBUG_TRACE,("P2pSetProfile :: Set P2P WPS. My Dpid = %x \n", pPort->P2PCfg.Dpid));
            break;
            
        case CONFIG_MODE_PROVISION_THIS:
            DBGPRINT(RT_DEBUG_TRACE,("P2pSetProfile :: provisioning P2P. My Dpid = %x \n",  pPort->P2PCfg.Dpid));
            P2pProvision(pAd, pPort, &pP2pStruc->ConnectingMAC[0][0]);
            break;          
        case CONFIG_MODE_DISABLE_P2P_WPSE:
            DBGPRINT(RT_DEBUG_TRACE,("P2pSetProfile :: Disable P2P.  Opmode = %d. \n", pAd->OpMode));

            // we often use GoIntentIdx == 16 to know I was set to be AutoGo. Since this is a disconnet comand.
            // I must set GoIntentIdx to a vlaue !=16.
            pPort->P2PCfg.GoIntentIdx = 5;
            P2pDown(pAd, pAd->PortList[pPort->P2PCfg.PortNumber]);
            // disable P2P function.
            P2pGroupTabInit(pAd);
            P2pSetEvent(pAd, pPort,  P2PEVENT_P2PGROUP_TOPOLOGY_UPDATE);
            pPort->P2PCfg.P2PConnectState = P2P_CONNECT_NOUSE;
            pPort->P2PCfg.P2PDiscoProvState = P2P_DISABLE;

            DBGPRINT(RT_DEBUG_TRACE,("P2pSetProfile ::  Disable P2P <=============================== \n"));
            RTUSBReadMACRegister(pAd, INT_TIMER_EN, &Value);
            Value &= (0xfffffffd);
            RTUSBWriteMACRegister(pAd, INT_TIMER_EN, Value);

            // Reset port type
            if ((pAd->OpMode == OPMODE_STA) || (pAd->OpMode == OPMODE_STAP2P))
            {
                pAd->PortList[pPort->P2PCfg.PortNumber]->PortType = EXTSTA_PORT;
                pAd->PortList[pPort->P2PCfg.PortNumber]->PortSubtype = PORTSUBTYPE_STA;
                DBGPRINT(RT_DEBUG_TRACE,("Port[%d] subtype is %d.  \n", pPort->P2PCfg.PortNumber, pAd->PortList[pPort->P2PCfg.PortNumber]->PortSubtype));
            }
            break;
        case CONFIG_MODE_ENABLE_P2P:
            // Init Token
            pPort->P2PCfg.bKeepSlient = FALSE;
            pPort->P2PCfg.Token = 0;
            // Only Device Name is passed from GUI when Config mode is Enable P2P.
            // Listen 
            PlatformMoveMemory(pPort->P2PCfg.DeviceName, pP2pStruc->DeviceName, 32);
            pPort->P2PCfg.DeviceNameLen = pP2pStruc->DeviceNameLen;
            P2pEnable(pAd);
            DBGPRINT(RT_DEBUG_TRACE,("P2pSetProfile ::  Enable P2P.  DeviceNameLen = %d.  \n", pPort->P2PCfg.DeviceNameLen));
            DBGPRINT(RT_DEBUG_TRACE,("P2pSetProfile ::  Enable.  = %c%c%c%c%c%c. <==== \n", pPort->P2PCfg.DeviceName[0], pPort->P2PCfg.DeviceName[1],pPort->P2PCfg.DeviceName[2],pPort->P2PCfg.DeviceName[3],pPort->P2PCfg.DeviceName[4],pPort->P2PCfg.DeviceName[5]));
            DBGPRINT(RT_DEBUG_TRACE,("P2pSetProfile ::  Enable P2P <=============================== \n"));

            break;

        case CONFIG_MODE_P2P_SCAN:
            DBGPRINT(RT_DEBUG_TRACE,("P2pSetProfile : Scan. ====> \n" ));

            P2PPrintState(pAd);
            for (i = 0; i< pAd->pP2pCtrll->P2PTable.ClientNumber;i++)
                P2PPrintP2PEntry(pAd,(UCHAR) i);

            for (i = 0;i < MAX_P2P_GROUP_SIZE;i++)
            {
                if (((pAd->pP2pCtrll->P2PTable.Client[i].P2pClientState >= P2PSTATE_DISCOVERY)
                    && (pAd->pP2pCtrll->P2PTable.Client[i].P2pClientState <= P2PSTATE_DISCOVERY_CLIENT))
                    || (IS_P2P_PEER_CLIENT_OP(&pAd->pP2pCtrll->P2PTable.Client[i]))
                    || (IS_P2P_PEER_GO_OP(&pAd->pP2pCtrll->P2PTable.Client[i])))
                { 
                    P2pGroupTabDelete(pAd, i, pAd->pP2pCtrll->P2PTable.Client[i].addr);
                }
            }
            for (i = 0; i< pAd->pP2pCtrll->P2PTable.ClientNumber;i++)
                P2PPrintP2PEntry(pAd,(UCHAR) i);
            // If I am GO, My Mactab has all peers. So I Can copy to P2P tabe in the beginning before Scan.
            if (IS_P2P_GO_OP(pPort))
            {
                pHeader = &pPort->MacTab.MacTabList;
                pNextMacEntry = (PMAC_TABLE_ENTRY)(pHeader->Head);
                while (pNextMacEntry != NULL)
                {
                    pMacEntry = (PMAC_TABLE_ENTRY)pNextMacEntry;
                    
                    if(pMacEntry == NULL)
                    {
                        break;
                    }

                    if ((pMacEntry->ValidAsCLI == TRUE) 
                        && (pMacEntry->ValidAsP2P == TRUE))
                    {
                        p2pindex = P2pGroupTabInsert(pAd, pMacEntry->P2pInfo.DevAddr, pMacEntry->P2pInfo.P2pClientState, pPort->P2PCfg.SSID, pPort->P2PCfg.SSIDLen);
                        P2pCopyMacTabtoP2PTab(pAd, p2pindex, (UCHAR)pMacEntry->wcid);
                    }
                    
                    pNextMacEntry = pNextMacEntry->Next;   
                    pMacEntry = NULL;
                }

            }
            else if (IS_P2P_CLIENT_OP(pPort))
            {
            }
            
            P2pSetEvent(pAd, pPort,  P2PEVENT_P2PGROUP_TOPOLOGY_UPDATE);
            P2pScanChannelDefault(pAd, pPort);
            P2pSetListenIntBias(pAd, 1);
            pPort->P2PCfg.P2pCounter.CounterAftrScanButton = 0;
            P2pGotoScan(pAd, pPort, P2P_DISCO_TYPE_DEFAULT, 2);
            break;
            
        case CONFIG_MODE_DISCONNECT_P2P:
            DBGPRINT(RT_DEBUG_TRACE,("============================ ====> \n"));
            DBGPRINT(RT_DEBUG_TRACE,("P2pSetProfile : Disconnect.====> \n"));
            // we often use GoIntentIdx == 16 to know I was set to be AutoGo. Since this is a disconnet comand.
            // I must set GoIntentIdx to a vlaue !=16.
            pPort->P2PCfg.GoIntentIdx = 5;
            pPort->P2PCfg.ConnectingListenChannel = 0;
            if( (pAd->PortList[pPort->P2PCfg.PortNumber]->PortSubtype == PORTSUBTYPE_P2PClient) && (pPort->CommonCfg.P2pControl.field.EnablePresistent == 1))
            {
                pPort->P2PCfg.P2pCounter.CounterAutoReconnectForP2pClient = 10;
            }
            
            P2pDown(pAd, pAd->PortList[pPort->P2PCfg.PortNumber]);
            if (pAd->PortList[pPort->P2PCfg.PortNumber]->PortSubtype == PORTSUBTYPE_P2PGO)
                P2pStopGo(pAd, pAd->PortList[pPort->P2PCfg.PortNumber]);
            // Delete P2P Device that I previously tried to connect.
            for (i = 0;i < MAX_P2P_GROUP_SIZE;i++)
            {
                if (pAd->pP2pCtrll->P2PTable.Client[i].P2pClientState > P2PSTATE_DISCOVERY_UNKNOWN)
                    P2pGroupTabDelete(pAd, i, pAd->pP2pCtrll->P2PTable.Client[i].addr);
            }
    
            pPort->P2PCfg.P2PDiscoProvState = P2P_ENABLE_LISTEN_ONLY;
            // go to scan since no conenction right now.
            if ((!INFRA_ON(pAd->PortList[PORT_0])) && (!ADHOC_ON(pAd->PortList[PORT_0])))
            {
                // Don't need to scan too much. 
                pPort->P2PCfg.P2pCounter.CounterAftrScanButton = P2P_SCAN_PERIOD/2;
                pPort->P2PCfg.P2PConnectState = P2P_CONNECT_IDLE;
                P2pGotoScan(pAd, pPort, P2P_DISCO_TYPE_DEFAULT, 2);
                pPort->P2PCfg.Dpid = DEV_PASS_ID_NOSPEC;
            }
            else if ((pAd->OpMode == OPMODE_STA) && (!INFRA_ON(pAd->PortList[PORT_0])) && (!ADHOC_ON(pAd->PortList[PORT_0])))
            {
                //AsicSwitchChannel(pAd, pPort->P2PCfg.ListenChannel, FALSE);
                AsicSwitchChannel(pPort, pPort->P2PCfg.ListenChannel, pPort->P2PCfg.ListenChannel, BW_20, FALSE);
            }

            if (IS_P2P_AUTOGO(pPort))
            {
                pPort->P2PCfg.P2pEventQueue.bP2pTopologyUpdate = TRUE;
            }

            PlatformZeroMemory( pPort->P2PCfg.PinCode, 8);
            PlatformZeroMemory( pPort->StaCfg.WscControl.RegData.PIN, 8);
            pPort->StaCfg.WscControl.RegData.PINLen = 0;
            break;
            
        case CONFIG_MODE_CONNECT_P2P:
            // GUI set this when press "connect" button.
            DBGPRINT(RT_DEBUG_TRACE,("============================ ====> \n"));
            DBGPRINT(RT_DEBUG_TRACE,("P2pSetProfile : Connect to a P2P Device. ====> \n"));

            PlatformCancelTimer(&pPort->P2PCfg.P2pStopGoTimer, &Cancelled);
            PlatformCancelTimer(&pPort->P2PCfg.P2pStopClientTimer, &Cancelled);

            // Disallow p2p connection while we start an adhoc network
            if ((pAd->OpMode == OPMODE_STAP2P) && 
                (ADHOC_ON(pAd->PortList[PORT_0]) && 
                (pAd->StaCfg.BssType == BSS_ADHOC)))
            {
                pPort->P2PCfg.GroupFormErrCode = P2P_ERRCODE_ADHOCON;
                pPort->P2PCfg.P2pEventQueue.bErrorCode = TRUE;
                break;
            }
        
            // Disallow p2p connection while Sta conencts to B-only AP
            if ((pAd->OpMode == OPMODE_STAP2P) &&
                (INFRA_ON(pAd->PortList[PORT_0])) &&
                (pAd->StaActive.SupportedHtPhy.bHtEnable == FALSE) &&
                (pAd->StaActive.SupRateLen == 4) && 
                (pAd->StaActive.ExtRateLen == 0) && 
                (pAd->StaActive.SupportedHtPhy.MCSSet[0] == 0) && 
                (pAd->StaActive.SupportedHtPhy.MCSSet[1] == 0))
            {
                pPort->P2PCfg.GroupFormErrCode = P2P_ERRCODE_PHYMODECONFLICT;
                pPort->P2PCfg.P2pEventQueue.bErrorCode = TRUE;
                break;
            }

            // set 2 minutes to connection fail
            if((pP2pStruc->GoIntentIdx != 16) && (!IS_P2P_CLIENT_OP(pPort)))
                pPort->P2PCfg.P2pCounter.CounterP2PConnectionFail = 0;

            if (pPort->PortSubtype == PORTSUBTYPE_P2PGO)
            {
                // This case happend when I am Auto GO. And then I invite a P2P device to join my group.
                // In such situation, I don't need to update some parameters from pP2pStruc.                
                P2pSetGoWps(pAd, pP2pStruc);
            }
            else
            {
                // Because Win7 may keep previous wireless connection profile and set to connect to some AP.
                // Since I already get command to connect to a P2P device. I should reset my state to idle
                // to do the connect command.
                if (pPort->P2PCfg.P2PConnectState == P2P_I_AM_CLIENT_ASSOC_AUTH)
                {
                    pPort->P2PCfg.P2PConnectState = P2P_CONNECT_IDLE;
                    pPort->P2PCfg.P2pEventQueue.bDeleteWirelessCon = TRUE;
                    DBGPRINT(RT_DEBUG_TRACE,("I am in associating state. reset to Idle first. \n"));
                }

                // "Connect-Button" means invite DEV to join its group while I am in Client Operation Mode.
                // So we set "ConnectingMAC address" only and other paameters are no changes.
                if ((!IS_P2P_CLIENT_OP(pPort)))
                {
                    PlatformSetTimer(pPort, &pPort->P2PCfg.P2pStopClientTimer, P2P_CHECK_CLIENT_TIMER);
                    P2pSetProfileParm(pAd, pPort, pP2pStruc);
                }
                
                pPort->P2PCfg.P2pCounter.CounterAfterConnectCommand = 0;
            }
            
            if (pAd->OpMode == OPMODE_STAP2P)
            {
                // Set multi-bssid
                MtAsicSetMultiBssidEx(pAd, pPort->CurrentAddress, (UCHAR)pPort->PortNumber);
                // win7 doesn't allow adhoc connection when virtula port is active. So doesn't check adhoc.
                if (INFRA_ON(pAd->PortList[PORT_0]))
                {
                    P2pDecidePhyMode(pAd, pPort);

                }
            }
            // Prepare WPA2PSK AES IE.
            RTMPAPMakeRSNIE(pAd, pPort, Ralink802_11AuthModeWPA2PSK, Ralink802_11Encryption3Enabled);
            if (pAd->OpMode == OPMODE_STAP2P)
            {
                pPort->P2PCfg.P2pCapability[0] |= DEVCAP_CLIENT_CONCURRENT;
                if (IS_P2P_GO_OP(pPort))
                    pPort->P2PCfg.P2pCapability[1] |= GRPCAP_OWNER;
            }   

            P2pScanChannelDefault(pAd, pPort);
            // Start Autonomous GO mode.
            if (pP2pStruc->GoIntentIdx == 16)
            {
                pPort->P2PCfg.GroupOpChannel = pPort->P2PCfg.GroupChannel;
                P2pStartAutoGo(pAd, pPort);
            }
            else
            {
                DBGPRINT(RT_DEBUG_TRACE,("P2pSetProfile : Connect to the P2P . %x %x %x %x %x %x \n", pP2pStruc->ConnectingMAC[0][0],pP2pStruc->ConnectingMAC[0][1],pP2pStruc->ConnectingMAC[0][2],pP2pStruc->ConnectingMAC[0][3],pP2pStruc->ConnectingMAC[0][4],pP2pStruc->ConnectingMAC[0][5]));
                // If I choose to use enter Pin code... I'd like to use provistion method first.
                // So the peer will display thePinCode.
                if ((pPort->PortSubtype != PORTSUBTYPE_P2PGO))
                {
                    p2pindex = P2pGroupTabSearch(pAd, &pP2pStruc->ConnectingMAC[0][0]);
                    DBGPRINT(RT_DEBUG_TRACE,("peer p2pindex =  %d \n", p2pindex));
                    if (p2pindex < MAX_P2P_GROUP_SIZE)
                    {
                        DBGPRINT(RT_DEBUG_TRACE,("peer P2pFlag =  %x \n", pAd->pP2pCtrll->P2PTable.Client[p2pindex].P2pFlag));
                        /*if (pAd->pP2pCtrll->P2PTable.Client[p2pindex].Rule == P2P_IS_GO)
                        {
                            DBGPRINT(RT_DEBUG_TRACE,("peer GO Opchannel =  %d \n", pAd->pP2pCtrll->P2PTable.Client[p2pindex].OpChannel));
                            // Add GO's operating channel in my search channel. in case GO is not in 1, 6, 11.
                            // Default listen channel is based on spec, using Social channels 1, 6, 11.
                        }*/
                        if ((pAd->pP2pCtrll->P2PTable.Client[p2pindex].OpChannel != 0) && (pAd->pP2pCtrll->P2PTable.Client[p2pindex].OpChannel != 1)
                             && (pAd->pP2pCtrll->P2PTable.Client[p2pindex].OpChannel != 6) 
                             && (pAd->pP2pCtrll->P2PTable.Client[p2pindex].OpChannel != 11))
                        {
                            pPort->P2PCfg.P2pProprietary.ListenChanel[0] = pAd->pP2pCtrll->P2PTable.Client[p2pindex].OpChannel;
                            pPort->P2PCfg.P2pProprietary.ListenChanelCount = 4;
                            pPort->P2PCfg.P2pProprietary.ListenChanel[1] = 1;
                            pPort->P2PCfg.P2pProprietary.ListenChanel[2] = 6;
                            pPort->P2PCfg.P2pProprietary.ListenChanel[3] = 11;
                            pPort->P2PCfg.P2pProprietary.ListenChanelIndex = 0;
                        }
                        // Listen channel is supposed to in 1, 6, or 11.  .. So this "else if" may not happen
                        else if ((pAd->pP2pCtrll->P2PTable.Client[p2pindex].ListenChannel != 0) && (pAd->pP2pCtrll->P2PTable.Client[p2pindex].ListenChannel != 1)
                             && (pAd->pP2pCtrll->P2PTable.Client[p2pindex].ListenChannel != 6) 
                             && (pAd->pP2pCtrll->P2PTable.Client[p2pindex].ListenChannel != 11))
                        {
                            pPort->P2PCfg.P2pProprietary.ListenChanel[0] = pAd->pP2pCtrll->P2PTable.Client[p2pindex].ListenChannel;
                            pPort->P2PCfg.P2pProprietary.ListenChanelCount = 4;
                            pPort->P2PCfg.P2pProprietary.ListenChanel[1] = 1;
                            pPort->P2PCfg.P2pProprietary.ListenChanel[2] = 6;
                            pPort->P2PCfg.P2pProprietary.ListenChanel[3] = 11;
                            pPort->P2PCfg.P2pProprietary.ListenChanelIndex = 0;
                        }
                        else
                        {
                            pPort->P2PCfg.P2pProprietary.ListenChanelCount = 3;
                            pPort->P2PCfg.P2pProprietary.ListenChanel[0] = 1;
                            pPort->P2PCfg.P2pProprietary.ListenChanel[1] = 6;
                            pPort->P2PCfg.P2pProprietary.ListenChanel[2] = 11;
                            pPort->P2PCfg.P2pProprietary.ListenChanelIndex = 0;
                        }
                        
                        if (((pP2pStruc->WscMode == WSC_PBC_MODE) ||(pP2pStruc->WscMode == WSC_PIN_MODE_REGISTRA_SPEC) || (pP2pStruc->WscMode == WSC_PIN_MODE_USER_SPEC) || pAd->pP2pCtrll->P2PTable.Client[p2pindex].Rule == P2P_IS_GO)
                            && !P2P_TEST_FLAG(&pAd->pP2pCtrll->P2PTable.Client[p2pindex], P2PFLAG_PROVISIONED)
                            && !P2P_TEST_FLAG(&pAd->pP2pCtrll->P2PTable.Client[p2pindex], P2PFLAG_PASSED_NEGO_REQ)
                            && (!IS_P2P_CLIENT_OP(pPort))
                            &&(pPort->CommonCfg.P2pControl.field.EnablePresistent == 0))
                        {
                            if (TRUE == P2pProvision(pAd, pPort, &pP2pStruc->ConnectingMAC[0][0]))
                                break;
                        }
                        
                    }
                }
                
                // ======================================>
                // The 4 lines are standard connect command.
                PlatformZeroMemory(pPort->P2PCfg.ConnectingMAC, MAX_P2P_GROUP_SIZE*6);
                PlatformMoveMemory(pPort->P2PCfg.ConnectingMAC, pP2pStruc->ConnectingMAC, MAX_P2P_GROUP_SIZE*6);
                // record the peer listen channel and stay the peer listen channel for long time(5 sec)
                pPort->P2PCfg.ConnectingListenChannel = pAd->pP2pCtrll->P2PTable.Client[p2pindex].ListenChannel;
                if((pAd->PortList[pPort->P2PCfg.PortNumber]->PortSubtype != PORTSUBTYPE_P2PGO) && !(INFRA_ON(pAd->PortList[PORT_0])) && (pAd->pP2pCtrll->P2PTable.Client[p2pindex].Rule != P2P_IS_GO) && (pPort->P2PCfg.P2PConnectState != P2P_I_AM_CLIENT_OP))
                    pPort->P2PCfg.P2PConnectState = P2P_NEGOTIATION;
                // Set to 0xff means to connect to first Connecting MAC
                pPort->P2PCfg.ConnectingIndex = 0xff;

                // for persistent on, if client beacon lost without connecting back
                // GO need to reinvoke it back and need change the state of client
                p2pindex = P2pGroupTabSearch(pAd, &pP2pStruc->ConnectingMAC[0][0]);
                DBGPRINT(RT_DEBUG_TRACE, ("p2pindex = %d change client state %d", p2pindex, pAd->pP2pCtrll->P2PTable.Client[p2pindex].P2pClientState));
                if(p2pindex != P2P_NOT_FOUND )
                {
                    if((pPort->CommonCfg.P2pControl.field.EnablePresistent == 1)  && (pAd->pP2pCtrll->P2PTable.Client[p2pindex].P2pClientState == P2PSTATE_CLIENT_OPERATING))
                        pAd->pP2pCtrll->P2PTable.Client[p2pindex].P2pClientState = P2PSTATE_DISCOVERY;
                    DBGPRINT(RT_DEBUG_TRACE, ("change client state %d", pAd->pP2pCtrll->P2PTable.Client[p2pindex].P2pClientState));
                }
                P2pConnect(pAd);
                // <======================================
            }
            break;
            
        case CONFIG_MODE_DELETE_ACTIVE_P2P:
            DBGPRINT(RT_DEBUG_TRACE,("P2pSetProfile  Delete the active Profile.  \n"));

            P2pDown(pAd, pAd->PortList[pPort->P2PCfg.PortNumber]);
    
            pPort->P2PCfg.P2PDiscoProvState = P2P_ENABLE_LISTEN_ONLY;
            if ((pAd->OpMode == OPMODE_STA) && (!INFRA_ON(pAd->PortList[PORT_0])) && (!ADHOC_ON(pAd->PortList[PORT_0])))
            {
                //AsicSwitchChannel(pAd, pPort->P2PCfg.ListenChannel, FALSE);
                AsicSwitchChannel(pPort, pPort->P2PCfg.ListenChannel, pPort->P2PCfg.ListenChannel, BW_20, FALSE);
            }
            break;
        case CONFIG_MODE_SIGMA_SEND_P2P_SRVDISCO_REQ:
        case CONFIG_MODE_SIGMA_SET_WPS:     
        case CONFIG_MODE_SIGMA_SEND_P2P_PROVISION_DIS_REQ:          
        case CONFIG_MODE_SIGMA_SET_P2P_DISABLE:
        case CONFIG_MODE_SIGMA_P2P_DISSOLVE:            
        case CONFIG_MODE_SIGMA_START_AUTONOMOUS_GO: 
        case CONFIG_MODE_SIGMA_START_GROUP_FORMATION_REQ:
        case CONFIG_MODE_SIGMA_WAIT_GROUP_FORMATION_REQ:
        case CONFIG_MODE_SIGMA_P2P_CONNECT:
        case CONFIG_MODE_SIGMA_SEND_P2P_INVITATION_REQ:
        case CONFIG_MODE_SIGMA_P2P_RESET_DEFAULT:
        case CONFIG_MODE_SIGMA_SET_P2P_LISTEN:  
        case CONFIG_MODE_SIGMA_SET_P2P_DISCOVERY:
        case CONFIG_MODE_SIGMA_P2P_SWITCH_OP_STA:
        case CONFIG_MODE_SIGMA_P2P_SWITCH_OP_STAP2P:
        case CONFIG_MODE_SIGMA_SEND_P2P_PRESENCE_REQ:
        case CONFIG_MODE_SIGMA_CHANGE_P2PCTRL_SETTING:
        case CONFIG_MODE_SIGMA_SET_P2P_IDLE:    
            P2pSigmaSetProfile(pAd, pPort, InformationBufferLength, pInformationBuffer);
            break;
        default:
            break;

    }

    P2PPrintState(pAd);
    return NDIS_STATUS_SUCCESS;
}

/*  
    ==========================================================================
    Description: 
    Two case needs this.
    1. Maintain Wps State When I am GO. Accept individual WPS setting for individual client that want to connects to me.
    2. If I am a P2P device and receive provistion request to request me to display the PIN. then GUI will set PIN to me. So
    when later that peer want to use WPS with me, I can immediately connects with him.
        
    Parameters: 
    Note:
    ==========================================================================
 */
VOID P2pSetGoWps(
    IN PMP_ADAPTER pAd,
    IN PRT_OID_SET_P2P_STRUCT       pP2pStruc)
{
    BOOLEAN     Cancelled;
    UCHAR ZeroPinCode4[4] = {0, 0, 0, 0};
    UCHAR ZeroPinCode8[8] = {0, 0, 0, 0, 0, 0, 0, 0};
    PMP_PORT    pPort  = pAd->PortList[FXXK_PORT_0];
    
    PlatformCancelTimer(&pPort->P2PCfg.P2pStopWpsTimer, &Cancelled);
    PlatformSetTimer(pPort, &pPort->P2PCfg.P2pStopWpsTimer, WSC_REG_SESSION_TIMEOUT);
    if (pP2pStruc->WscMode == WSC_PBC_MODE)
    {
        pPort->P2PCfg.SigmaSetting.MyConfigMethod = CONFIG_METHOD_PUSHBUTTON;
        if (IS_P2P_SIGMA_ON(pPort))
            pPort->CommonCfg.P2pControl.field.DefaultConfigMethod = P2P_REG_CM_PBC;   
        DBGPRINT(RT_DEBUG_TRACE,("P2pSetGoWps. pbc \n"));
        pPort->P2PCfg.Dpid = DEV_PASS_ID_PBC;
        pPort->P2PCfg.ConfigMethod = CONFIG_METHOD_PUSHBUTTON;
        pPort->P2PCfg.WscMode = WSC_PBC_MODE;
        // For PBC, the PIN is all '0'
        pPort->StaCfg.WscControl.RegData.PIN[0] = '0';
        pPort->StaCfg.WscControl.RegData.PIN[1] = '0';
        pPort->StaCfg.WscControl.RegData.PIN[2] = '0';
        pPort->StaCfg.WscControl.RegData.PIN[3] = '0';
        pPort->StaCfg.WscControl.RegData.PIN[4] = '0';
        pPort->StaCfg.WscControl.RegData.PIN[5] = '0';
        pPort->StaCfg.WscControl.RegData.PIN[6] = '0';
        pPort->StaCfg.WscControl.RegData.PIN[7] = '0';
        PlatformMoveMemory(pPort->P2PCfg.PinCode, pPort->StaCfg.WscControl.RegData.PIN, 8);
        pPort->StaCfg.WscControl.WscMode = WSC_PBC_MODE;  
        // Make sure I am GO or Registrar.

        //default is using PIN 8 digit.
        pPort->StaCfg.WscControl.RegData.PINLen = 4;
        pPort->P2PCfg.PINLen = 4;
        
    }
    else if ((pP2pStruc->WscMode == WSC_PIN_MODE_USER_SPEC)||
            (pP2pStruc->WscMode == WSC_PIN_MODE_REGISTRA_SPEC) ||
            (pP2pStruc->WscMode == WSC_PIN_MODE))
    {
        if((pP2pStruc->PinCode[0] == 0xff) && (pP2pStruc->PinCode[1] == 0xff) &&
            (pP2pStruc->PinCode[2] == 0xff) && (pP2pStruc->PinCode[3] == 0xff))
        {
            PlatformMoveMemory(pPort->StaCfg.WscControl.RegData.PIN, &pP2pStruc->PinCode[4], 4);
            PlatformMoveMemory(pPort->P2PCfg.PinCode, &pP2pStruc->PinCode[4], 4);
            if(PlatformEqualMemory(&pP2pStruc->PinCode[4], &ZeroPinCode4[0], 4))
            {
                pPort->StaCfg.WscControl.RegData.PINLen = 0;
                pPort->P2PCfg.PINLen = 0;
            }
            else
            {
                pPort->StaCfg.WscControl.RegData.PINLen = 2;
                pPort->P2PCfg.PINLen = 2;
            }
            
            DBGPRINT(RT_DEBUG_TRACE, ("P2pSetGoWps: Wsc_Set_Mode PIN is 4 digit pin length = %d!!!\n", pPort->StaCfg.WscControl.RegData.PINLen ));
        }
        else
        {
            PlatformMoveMemory(pPort->StaCfg.WscControl.RegData.PIN, pP2pStruc->PinCode, 8);
            PlatformMoveMemory(pPort->P2PCfg.PinCode, pP2pStruc->PinCode, 8);
            if(PlatformEqualMemory(&pP2pStruc->PinCode[0], &ZeroPinCode8[0], 8))
            {
                pPort->StaCfg.WscControl.RegData.PINLen = 0;
                pPort->P2PCfg.PINLen = 0;
            }
            else
            {
                pPort->StaCfg.WscControl.RegData.PINLen = 4;
                pPort->P2PCfg.PINLen = 4;
                
            }
            
            DBGPRINT(RT_DEBUG_TRACE, ("P2pSetGoWps: Wsc_Set_Mode PIN is 8 digit pin length = %d !!!\n", pPort->StaCfg.WscControl.RegData.PINLen ));
        }

        if (pP2pStruc->WscMode == WSC_PIN_MODE_USER_SPEC)
        {
            pPort->P2PCfg.SigmaSetting.MyConfigMethod = CONFIG_METHOD_KEYPAD;
            if (IS_P2P_SIGMA_ON(pPort))
                pPort->CommonCfg.P2pControl.field.DefaultConfigMethod = P2P_REG_CM_KEYPAD;    
            DBGPRINT(RT_DEBUG_TRACE,("======> User Spec Mode\n"));
            pPort->P2PCfg.Dpid = DEV_PASS_ID_USER;
            pPort->P2PCfg.ConfigMethod = CONFIG_METHOD_KEYPAD;
            pPort->P2PCfg.WscMode = WSC_PIN_MODE_USER_SPEC;
            pPort->StaCfg.WscControl.WscMode = WSC_PIN_MODE_USER_SPEC;    
        }
        else if (pP2pStruc->WscMode == WSC_PIN_MODE)
        {
            pPort->P2PCfg.SigmaSetting.MyConfigMethod = CONFIG_METHOD_LABEL;
            if (IS_P2P_SIGMA_ON(pPort))
                pPort->CommonCfg.P2pControl.field.DefaultConfigMethod = P2P_REG_CM_LABEL; 
            DBGPRINT(RT_DEBUG_TRACE,("======> Pin Mode\n"));
            pPort->P2PCfg.Dpid = DEV_PASS_ID_PIN;
            pPort->P2PCfg.ConfigMethod = CONFIG_METHOD_LABEL;
            pPort->P2PCfg.WscMode = WSC_PIN_MODE;
            pPort->StaCfg.WscControl.WscMode = WSC_PIN_MODE;  
        }
        else if (pP2pStruc->WscMode == WSC_PIN_MODE_REGISTRA_SPEC)
        {
            pPort->P2PCfg.SigmaSetting.MyConfigMethod = CONFIG_METHOD_DISPLAY;
            if (IS_P2P_SIGMA_ON(pPort))
                pPort->CommonCfg.P2pControl.field.DefaultConfigMethod = P2P_REG_CM_DISPLAY;   
            DBGPRINT(RT_DEBUG_TRACE,("======> Registra Spec Mode\n"));
            pPort->P2PCfg.Dpid = DEV_PASS_ID_REG;
            pPort->P2PCfg.ConfigMethod = CONFIG_METHOD_DISPLAY;
            pPort->P2PCfg.WscMode = WSC_PIN_MODE_REGISTRA_SPEC;
            pPort->StaCfg.WscControl.WscMode = WSC_PIN_MODE_REGISTRA_SPEC;    
        }
    }
    
    // Reflash this flag on the next Invitation or DPID setting.
    pPort->P2PCfg.SigmaQueryStatus.bInvitationSucced = FALSE; 
    DBGPRINT(RT_DEBUG_TRACE,("P2pSetGoWps. Dpid = %x. ConfigMethod = %x \n", pPort->P2PCfg.Dpid, pPort->P2PCfg.ConfigMethod));
    DBGPRINT(RT_DEBUG_TRACE,("P2pSetGoWps. 1 PIN = %x  %x\n", *(PULONG)&pPort->P2PCfg.PinCode[0], *(PULONG)&pPort->P2PCfg.PinCode[4]));
    DBGPRINT(RT_DEBUG_TRACE,("P2pSetGoWps. 2 PIN = %x  %x\n", *(PULONG)&pPort->StaCfg.WscControl.RegData.PIN[0], *(PULONG)&pPort->StaCfg.WscControl.RegData.PIN[4]));
}




/*  
    ==========================================================================
    Description: 
        
    Parameters: 
    Note:
         
    ==========================================================================
 */
VOID P2pSetListenIntBias(
    IN PMP_ADAPTER pAd,
    IN UCHAR        Bias)
{
#if 0
    pPort->P2PCfg.P2pCounter.ListenIntervalBias = Bias;
    if (INFRA_ON(pAd->PortList[PORT_0]))
        pPort->P2PCfg.P2pCounter.ListenIntervalBias = 1;
 #endif
}

/*  
    ==========================================================================
    Description: 
        The routine that decide my Rule as GO or Client? 
        And then do necessary setting : update channel, Bssid, SSID etc.
        
    Parameters: 
    Note:
         
    ==========================================================================
 */
VOID P2pSetRule(
    IN PMP_ADAPTER pAd,
    IN UCHAR        Index,
    IN PMP_PORT   pPort,
    IN PUCHAR       PeerBssid,
    IN UCHAR        PeerGOIntentAttri,
    IN UCHAR        Channel,
    IN PUCHAR       PeerDevAddr)
{
    PRT_P2P_CLIENT_ENTRY pP2pEntry = &pAd->pP2pCtrll->P2PTable.Client[Index];
    UCHAR       RealIntent;

    DBGPRINT(RT_DEBUG_TRACE, (" P2pSetRule - Channel = %d. \n", Channel));
    RealIntent = PeerGOIntentAttri>>1;
    if (RealIntent > pPort->P2PCfg.GoIntentIdx)
    {
        pP2pEntry->Rule = P2P_IS_GO;
        pP2pEntry->GoIntent = RealIntent;
        // Use peer interface address as bssid
        PlatformMoveMemory(pPort->PortCfg.Bssid, PeerBssid, MAC_ADDR_LEN);
        PlatformMoveMemory(pPort->P2PCfg.Bssid, PeerBssid, MAC_ADDR_LEN);
        PlatformMoveMemory(pPort->P2PCfg.GroupDevAddr, PeerDevAddr, MAC_ADDR_LEN);
        PlatformMoveMemory(pPort->PortCfg.Ssid, pP2pEntry->Ssid, 32);
        PlatformMoveMemory(pPort->P2PCfg.SSID, pP2pEntry->Ssid, 32);
        pPort->P2PCfg.SSIDLen = pP2pEntry->SsidLen;
        pPort->PortCfg.SsidLen = pP2pEntry->SsidLen;
        pPort->P2PCfg.P2PConnectState = P2P_ANY_IN_FORMATION_AS_CLIENT;
        pPort->P2PCfg.GroupOpChannel = Channel;
        
        if (pAd->OpMode == OPMODE_STA)
            MtAsicSetBssid(pAd, pPort->PortCfg.Bssid, (UINT8)pPort->PortNumber);
        DBGPRINT(RT_DEBUG_TRACE, (" P2P - I become Internal Enrollee!!    Enrollee. !! GOGOGO\n"));
        DBGPRINT(RT_DEBUG_TRACE, (" P2P - I become Internal Enrollee!!    Enrollee. !! GOGOGO\n"));
    }
    else if (RealIntent < pPort->P2PCfg.GoIntentIdx)
    {
        pP2pEntry->Rule = P2P_IS_CLIENT;
        pP2pEntry->GoIntent = RealIntent;
        // Use my addr as bssid and device address in GroupID
        PlatformMoveMemory(pPort->P2PCfg.Bssid, pPort->CurrentAddress, MAC_ADDR_LEN);
        PlatformMoveMemory(pPort->P2PCfg.GroupDevAddr, pPort->CurrentAddress, MAC_ADDR_LEN);
        PlatformMoveMemory(pPort->PortCfg.Bssid, pPort->CurrentAddress, MAC_ADDR_LEN);
        pPort->P2PCfg.P2PConnectState = P2P_ANY_IN_FORMATION_AS_GO;
        pPort->P2PCfg.GroupOpChannel = pPort->P2PCfg.GroupChannel;
        if (pAd->OpMode == OPMODE_STA)
            MtAsicSetBssid(pAd, pPort->PortCfg.Bssid, (UINT8)pPort->PortNumber);
        DBGPRINT(RT_DEBUG_TRACE, (" P2P - I become Internal REGISTRA!!    REGISTRA. !! GOGOGO\n"));
        DBGPRINT(RT_DEBUG_TRACE, (" P2P - I become Internal REGISTRA!!    REGISTRA. !! GOGOGO\n"));
    }
    else if (((PeerGOIntentAttri&1) == 1) && (RealIntent == pPort->P2PCfg.GoIntentIdx))
    {
        DBGPRINT(RT_DEBUG_TRACE, ("P2pSetRule Peer Tie Breaker bit is On.  %x \n", RealIntent));
        pP2pEntry->Rule = P2P_IS_GO;
        pP2pEntry->GoIntent = RealIntent;
        // Use peer interface address as bssid
        PlatformMoveMemory(pPort->PortCfg.Bssid, PeerBssid, MAC_ADDR_LEN);
        PlatformMoveMemory(pPort->P2PCfg.Bssid, PeerBssid, MAC_ADDR_LEN);
        PlatformMoveMemory(pPort->P2PCfg.GroupDevAddr, PeerDevAddr, MAC_ADDR_LEN);
        PlatformMoveMemory(pPort->PortCfg.Ssid, pP2pEntry->Ssid, 32);
        PlatformMoveMemory(pPort->P2PCfg.SSID, pP2pEntry->Ssid, 32);
        pPort->P2PCfg.SSIDLen = pP2pEntry->SsidLen;
        pPort->PortCfg.SsidLen = pP2pEntry->SsidLen;
        pPort->P2PCfg.P2PConnectState = P2P_ANY_IN_FORMATION_AS_CLIENT;
        pPort->P2PCfg.GroupOpChannel = Channel;
        
        if (pAd->OpMode == OPMODE_STA)
            MtAsicSetBssid(pAd, pPort->PortCfg.Bssid, (UINT8)pPort->PortNumber);
        DBGPRINT(RT_DEBUG_TRACE, (" P2P - I become Internal Enrollee!!    Enrollee. !! GOGOGO\n"));
        DBGPRINT(RT_DEBUG_TRACE, (" P2P - I become Internal Enrollee!!    Enrollee. !! GOGOGO\n"));
    }
    else if (((PeerGOIntentAttri&1) == 0) && (RealIntent == pPort->P2PCfg.GoIntentIdx))
    {
        DBGPRINT(RT_DEBUG_TRACE, ("P2pSetRule Peer Tie Breaker bit is Off.  %x \n", RealIntent));
        pP2pEntry->Rule = P2P_IS_CLIENT;
        pP2pEntry->GoIntent = RealIntent;
        // Use my addr as bssid and device address in GroupID
        PlatformMoveMemory(pPort->P2PCfg.Bssid, pPort->CurrentAddress, MAC_ADDR_LEN);
        PlatformMoveMemory(pPort->P2PCfg.GroupDevAddr, pPort->CurrentAddress, MAC_ADDR_LEN);
        PlatformMoveMemory(pPort->PortCfg.Bssid, pPort->CurrentAddress, MAC_ADDR_LEN);
        pPort->P2PCfg.P2PConnectState = P2P_ANY_IN_FORMATION_AS_GO;
        pPort->P2PCfg.GroupOpChannel = pPort->P2PCfg.GroupChannel;
        if (pAd->OpMode == OPMODE_STA)
            MtAsicSetBssid(pAd, pPort->PortCfg.Bssid, (UINT8)pPort->PortNumber);

        DBGPRINT(RT_DEBUG_TRACE, (" P2P - I become Internal REGISTRA!!    REGISTRA. !! GOGOGO\n"));
        DBGPRINT(RT_DEBUG_TRACE, (" P2P - I become Internal REGISTRA!!    REGISTRA. !! GOGOGO\n"));
    }
    DBGPRINT(RT_DEBUG_TRACE, ("P2pSetRule pPort Bssid = %x %x %x %x %x %x  \n",  pPort->PortCfg.Bssid[0], pPort->PortCfg.Bssid[1],pPort->PortCfg.Bssid[2],pPort->PortCfg.Bssid[3],pPort->PortCfg.Bssid[4],pPort->PortCfg.Bssid[5]));
    DBGPRINT(RT_DEBUG_TRACE, ("GroupOpChannel =  %x  \n",pPort->P2PCfg.GroupOpChannel));
    DBGPRINT(RT_DEBUG_TRACE, (" P2pSetRule - pP2pEntry->Rule = %d. \n", pP2pEntry->Rule));

}

/*  
    ==========================================================================
    Description: 
        Start Provisionint process.  Send out Provision Discovery Request frame.
        
    Parameters: 
    Note:
         
    ==========================================================================
 */
BOOLEAN P2pProvision(
    IN PMP_ADAPTER pAd, 
    IN PMP_PORT     pPort,
    IN PUCHAR   Addr)
{
    UCHAR       p2pindex;
    
    DBGPRINT(RT_DEBUG_TRACE, ("P2pProvision = %x %x %x %x %x %x  \n",  Addr[0], Addr[1],Addr[2],Addr[3],Addr[4],Addr[5]));
    p2pindex = P2pGroupTabSearch(pAd, Addr);
    if (p2pindex < MAX_P2P_GROUP_SIZE)
    {
        DBGPRINT(RT_DEBUG_TRACE, ("Start P2pProvision = %x %x %x %x %x %x  \n",  Addr[0], Addr[1],Addr[2],Addr[3],Addr[4],Addr[5]));
        pAd->pP2pCtrll->P2PTable.Client[p2pindex].GeneralToken++;
        pAd->pP2pCtrll->P2PTable.Client[p2pindex].P2pClientState = P2PSTATE_PROVISION_COMMAND;

        PlatformZeroMemory(pPort->P2PCfg.ConnectingMAC, MAX_P2P_GROUP_SIZE*6);
        PlatformMoveMemory(&pPort->P2PCfg.ConnectingMAC[0][0], Addr, 6);
        // record the peer listen channel and stay the peer listen channel for long time(5 sec)
        pPort->P2PCfg.ConnectingListenChannel = pAd->pP2pCtrll->P2PTable.Client[p2pindex].ListenChannel;
        if((pAd->PortList[pPort->P2PCfg.PortNumber]->PortSubtype != PORTSUBTYPE_P2PGO) && !(INFRA_ON(pAd->PortList[PORT_0])))
            pPort->P2PCfg.P2PConnectState = P2P_NEGOTIATION;
        // Set to 0xff means to connect to first Connecting MAC
        pPort->P2PCfg.ConnectingIndex = 0xff;
        
        // for persistent on, if client beacon lost without connecting back
        // GO need to reinvoke it back and need change the state of client
        p2pindex = P2pGroupTabSearch(pAd, &pPort->P2PCfg.ConnectingMAC[0][0]);
        DBGPRINT(RT_DEBUG_TRACE, ("p2pindex = %d change client state %d", p2pindex, pAd->pP2pCtrll->P2PTable.Client[p2pindex].P2pClientState));
        if(p2pindex != P2P_NOT_FOUND )
        {
            if((pPort->CommonCfg.P2pControl.field.EnablePresistent == 1)  && (pAd->pP2pCtrll->P2PTable.Client[p2pindex].P2pClientState == P2PSTATE_CLIENT_OPERATING))
                pAd->pP2pCtrll->P2PTable.Client[p2pindex].P2pClientState = P2PSTATE_DISCOVERY;
            DBGPRINT(RT_DEBUG_TRACE, ("change client state %d", pAd->pP2pCtrll->P2PTable.Client[p2pindex].P2pClientState));
        }

        P2pSetListenIntBias(pAd, 4);
        pPort->P2PCfg.P2pCounter.CounterAftrScanButton = 0;
        P2pConnect(pAd);
        return TRUE;
    }
    return FALSE;
}


/*  
    ==========================================================================
    Description: 
        Prepare to connect to Connecting MAC. ConnectingMAC might contain several MAC address that I can connect to One after one.
        
    Parameters: 
    Note:
         
    ==========================================================================
 */
VOID P2pConnect(
    IN PMP_ADAPTER pAd)
{
    UCHAR   i = 0;
#if 0
    DBGPRINT(RT_DEBUG_TRACE, ("P2pConnect =  \n"));
    DBGPRINT(RT_DEBUG_TRACE, (" %x %x %x %x %x %x  \n",  pPort->P2PCfg.ConnectingMAC[0][0], pPort->P2PCfg.ConnectingMAC[0][1],pPort->P2PCfg.ConnectingMAC[0][2],pPort->P2PCfg.ConnectingMAC[0][3],pPort->P2PCfg.ConnectingMAC[0][4],pPort->P2PCfg.ConnectingMAC[0][5]));
    // Clear all previous client that is in "COMMAND" state;
    for (i = 0; i < MAX_P2P_GROUP_SIZE;i++)
    {
        if (pAd->pP2pCtrll->P2PTable.Client[i].P2pClientState == P2PSTATE_NONE)
            continue;
        
        P2PPrintP2PEntry(pAd, i);   
        if (pAd->pP2pCtrll->P2PTable.Client[i].P2pClientState == P2PSTATE_INVITE_COMMAND)
            pAd->pP2pCtrll->P2PTable.Client[i].P2pClientState = P2PSTATE_DISCOVERY;
        if (pAd->pP2pCtrll->P2PTable.Client[i].P2pClientState == P2PSTATE_CONNECT_COMMAND)
            pAd->pP2pCtrll->P2PTable.Client[i].P2pClientState = P2PSTATE_DISCOVERY;
        if (pAd->pP2pCtrll->P2PTable.Client[i].P2pClientState == P2PSTATE_CLIENT_DISCO_COMMAND)
            pAd->pP2pCtrll->P2PTable.Client[i].P2pClientState = P2PSTATE_DISCOVERY_CLIENT;
        if (pAd->pP2pCtrll->P2PTable.Client[i].P2pClientState == P2PSTATE_GO_DISCO_COMMAND)
            pAd->pP2pCtrll->P2PTable.Client[i].P2pClientState = P2PSTATE_DISCOVERY_GO;
        if (pAd->pP2pCtrll->P2PTable.Client[i].P2pClientState == P2PSTATE_SERVICE_DISCO_COMMAND)
            pAd->pP2pCtrll->P2PTable.Client[i].P2pClientState = P2PSTATE_DISCOVERY;
        if (pAd->pP2pCtrll->P2PTable.Client[i].P2pClientState == P2PSTATE_SENT_INVITE_REQ)
            pAd->pP2pCtrll->P2PTable.Client[i].P2pClientState = P2PSTATE_DISCOVERY;
        
        // === Error handling when the previous connection didn't complete ===
        // Don't care about SigmaTest because it would reset to default in each case.
        if (IS_P2P_SIGMA_OFF(pPort))
        {
            if (PlatformEqualMemory(&pPort->P2PCfg.ConnectingMAC[0], pAd->pP2pCtrll->P2PTable.Client[i].addr, 6))
            {
                if (pAd->PortList[pPort->P2PCfg.PortNumber]->PortSubtype == PORTSUBTYPE_P2PGO)
                {
                    // case #1: GO received InviteRsp and then Device didn't finish WPS.
                    if (pAd->pP2pCtrll->P2PTable.Client[i].P2pClientState == P2PSTATE_GOT_GO_COMFIRM)
                        pAd->pP2pCtrll->P2PTable.Client[i].P2pClientState = P2PSTATE_DISCOVERY;
                }
            }

            // Clear client's bssid & interfaceAddr when it goes back to p2p device
            if (pAd->pP2pCtrll->P2PTable.Client[i].P2pClientState == P2PSTATE_DISCOVERY)
            {
                PlatformZeroMemory(pAd->pP2pCtrll->P2PTable.Client[i].bssid, 6);
                PlatformZeroMemory(pAd->pP2pCtrll->P2PTable.Client[i].InterfaceAddr, 6);
            }
        } 
    }

    P2pConnectForward(pAd);
    P2pSetListenIntBias(pAd,  3);
    pPort->P2PCfg.P2pCounter.CounterAftrScanButton = 0;
    P2pGotoScan(pAd, P2P_DISCO_TYPE_DEFAULT, 2);
 #endif
}

/*  
    ==========================================================================
    Description: 
        Step forward to prepare to connect to next Connecting MAC. 
        
    Parameters: 
    Note:
         
    ==========================================================================
 */
VOID P2pConnectForward(
    IN PMP_ADAPTER pAd)
{
    UCHAR       allzero[6] = {0,0,0,0,0,0};
#if 0
    //if (pPort->P2PCfg.ConnectingIndex == 0xff)
    //{
        pPort->P2PCfg.ConnectingIndex = 0;
    //}
    /*else if (pPort->P2PCfg.ConnectingIndex < (MAX_P2P_GROUP_SIZE - 1))
    {
        pPort->P2PCfg.ConnectingIndex++;
    }*/

    DBGPRINT(RT_DEBUG_TRACE, ("P2pConnectForward. ConnectingIndex %d  \n", pPort->P2PCfg.ConnectingIndex));
    // If all zero, stop.
    if (PlatformEqualMemory(&pPort->P2PCfg.ConnectingMAC[pPort->P2PCfg.ConnectingIndex][0], allzero, MAC_ADDR_LEN))
        P2pStopConnectAction(pAd, pPort);
#endif
}

/*  
    ==========================================================================
    Description: 
        Ready to connect to Addr. Include all "Connect" cenario. GO_nego, invite, etc. Addr might be P2P GO, or a P2P Device.
        
    Parameters: 
    Note:
         
    ==========================================================================
 */
BOOLEAN P2pConnectAfterScan(
    IN PMP_ADAPTER pAd, 
    IN PMP_PORT     pPort,
    IN BOOLEAN  bBeacon,
    IN UCHAR        idx)
{
    BOOLEAN brc = TRUE;
    UCHAR       index;
    UCHAR       GrpIndex = idx, bresult = FALSE;
//    PMP_PORT  pPort;
    BOOLEAN     bAction = FALSE;
    pPort = pAd->PortList[pPort->P2PCfg.PortNumber];
    
    DBGPRINT(RT_DEBUG_TRACE, (" P2pConnectAfterScan   %d.  P2pClientState = %s ConfigMethod = %d\n", pAd->pP2pCtrll->P2PTable.Client[idx].P2pClientState, decodeP2PClientState(pAd->pP2pCtrll->P2PTable.Client[idx].P2pClientState), pAd->pP2pCtrll->P2PTable.Client[idx].ConfigMethod));
    DBGPRINT(RT_DEBUG_TRACE, ("my GroupOpChannel  = %d  \n", pPort->P2PCfg.GroupOpChannel));

    index = P2pPerstTabSearch(pAd, pAd->pP2pCtrll->P2PTable.Client[GrpIndex].addr);
    
    if (GrpIndex >= MAX_P2P_GROUP_SIZE)
    {
        DBGPRINT(RT_DEBUG_TRACE, ("P2pConnectAfterScan Check - Reach Device Limit. return.  \n"));
        return FALSE;
    }

    if (pAd->pP2pCtrll->P2PTable.Client[GrpIndex].P2pClientState == P2PSTATE_PROVISION_COMMAND)
    {
        ULONG   FrameLen;
        // Note: Broadcom doesn't accpt provistion request with toekn is zero.
        pAd->pP2pCtrll->P2PTable.Client[GrpIndex].GeneralToken ++;
        if (pAd->pP2pCtrll->P2PTable.Client[GrpIndex].GeneralToken == 0)
            pAd->pP2pCtrll->P2PTable.Client[GrpIndex].GeneralToken++;
        if (IS_P2P_SIGMA_ON(pPort))
        {
            if ((pPort->P2PCfg.SigmaSetting.MyConfigMethod == CONFIG_METHOD_DISPLAY))
            {
                pAd->pP2pCtrll->P2PTable.Client[GrpIndex].P2pClientState = P2PSTATE_SENT_PROVISION_REQ;
                P2PSendProvisionReq(pAd, CONFIG_METHOD_KEYPAD, pAd->pP2pCtrll->P2PTable.Client[GrpIndex].GeneralToken, pAd->pP2pCtrll->P2PTable.Client[GrpIndex].addr, &FrameLen);
                DBGPRINT(RT_DEBUG_TRACE, ("Request : CONFIG_METHOD_DISPLAY  \n"));
            }
            else if ((pPort->P2PCfg.SigmaSetting.MyConfigMethod == CONFIG_METHOD_PUSHBUTTON))
            {
                pAd->pP2pCtrll->P2PTable.Client[GrpIndex].P2pClientState = P2PSTATE_SENT_PROVISION_REQ;
                P2PSendProvisionReq(pAd, CONFIG_METHOD_PUSHBUTTON, pAd->pP2pCtrll->P2PTable.Client[GrpIndex].GeneralToken, pAd->pP2pCtrll->P2PTable.Client[GrpIndex].addr, &FrameLen);
                DBGPRINT(RT_DEBUG_TRACE, ("Request : CONFIG_METHOD_LABEL  \n"));
            }
            else if ((pPort->P2PCfg.SigmaSetting.MyConfigMethod == CONFIG_METHOD_KEYPAD))
            {
                pAd->pP2pCtrll->P2PTable.Client[GrpIndex].P2pClientState = P2PSTATE_SENT_PROVISION_REQ;
                P2PSendProvisionReq(pAd, CONFIG_METHOD_DISPLAY, pAd->pP2pCtrll->P2PTable.Client[GrpIndex].GeneralToken, pAd->pP2pCtrll->P2PTable.Client[GrpIndex].addr, &FrameLen);
                DBGPRINT(RT_DEBUG_TRACE, ("Request : my CONFIG_METHOD_KEYPAD  \n"));
            }
            else
                P2PSendProvisionReq(pAd, pAd->pP2pCtrll->P2PTable.Client[GrpIndex].ConfigMethod, pAd->pP2pCtrll->P2PTable.Client[GrpIndex].GeneralToken, pAd->pP2pCtrll->P2PTable.Client[GrpIndex].addr, &FrameLen);
        }
        else
        {
            if (pPort->P2PCfg.Dpid != DEV_PASS_ID_NOSPEC)
            {
                if ((pPort->P2PCfg.Dpid == DEV_PASS_ID_USER) && ((pAd->pP2pCtrll->P2PTable.Client[GrpIndex].ConfigMethod&CONFIG_METHOD_DISPLAY) != 0))
                {
                    pAd->pP2pCtrll->P2PTable.Client[GrpIndex].P2pClientState = P2PSTATE_SENT_PROVISION_REQ;
                    P2PSendProvisionReq(pAd, CONFIG_METHOD_DISPLAY, pAd->pP2pCtrll->P2PTable.Client[GrpIndex].GeneralToken, pAd->pP2pCtrll->P2PTable.Client[GrpIndex].addr, &FrameLen);
                    DBGPRINT(RT_DEBUG_TRACE, ("Request : CONFIG_METHOD_DISPLAY  \n"));
                }
                else if ((pPort->P2PCfg.Dpid == DEV_PASS_ID_PBC) && ((pAd->pP2pCtrll->P2PTable.Client[GrpIndex].ConfigMethod&CONFIG_METHOD_PUSHBUTTON) != 0))
                {
                    pAd->pP2pCtrll->P2PTable.Client[GrpIndex].P2pClientState = P2PSTATE_SENT_PROVISION_REQ;
                    P2PSendProvisionReq(pAd, CONFIG_METHOD_PUSHBUTTON, pAd->pP2pCtrll->P2PTable.Client[GrpIndex].GeneralToken, pAd->pP2pCtrll->P2PTable.Client[GrpIndex].addr, &FrameLen);
                    DBGPRINT(RT_DEBUG_TRACE, ("Request : CONFIG_METHOD_PUSHBUTTON  \n"));
                }
                else if ((pPort->P2PCfg.Dpid == DEV_PASS_ID_REG) && ((pAd->pP2pCtrll->P2PTable.Client[GrpIndex].ConfigMethod&CONFIG_METHOD_KEYPAD) != 0))
                {
                    pAd->pP2pCtrll->P2PTable.Client[GrpIndex].P2pClientState = P2PSTATE_SENT_PROVISION_REQ;
                    P2PSendProvisionReq(pAd, CONFIG_METHOD_KEYPAD, pAd->pP2pCtrll->P2PTable.Client[GrpIndex].GeneralToken, pAd->pP2pCtrll->P2PTable.Client[GrpIndex].addr, &FrameLen);
                    DBGPRINT(RT_DEBUG_TRACE, ("Request : CONFIG_METHOD_KEYPAD  \n"));
                }
                else
                    P2PSendProvisionReq(pAd, pAd->pP2pCtrll->P2PTable.Client[GrpIndex].ConfigMethod, pAd->pP2pCtrll->P2PTable.Client[GrpIndex].GeneralToken, pAd->pP2pCtrll->P2PTable.Client[GrpIndex].addr, &FrameLen);
            }
            else
            {
                if ((pAd->pP2pCtrll->P2PTable.Client[GrpIndex].ConfigMethod&CONFIG_METHOD_DISPLAY) != 0)
                {
                    pAd->pP2pCtrll->P2PTable.Client[GrpIndex].P2pClientState = P2PSTATE_SENT_PROVISION_REQ;
                    P2PSendProvisionReq(pAd, CONFIG_METHOD_DISPLAY, pAd->pP2pCtrll->P2PTable.Client[GrpIndex].GeneralToken, pAd->pP2pCtrll->P2PTable.Client[GrpIndex].addr, &FrameLen);
                    DBGPRINT(RT_DEBUG_TRACE, ("Request : CONFIG_METHOD_DISPLAY  \n"));
                }
                else if ((pAd->pP2pCtrll->P2PTable.Client[GrpIndex].ConfigMethod&CONFIG_METHOD_LABEL) != 0)
                {
                    pAd->pP2pCtrll->P2PTable.Client[GrpIndex].P2pClientState = P2PSTATE_SENT_PROVISION_REQ;
                    P2PSendProvisionReq(pAd, CONFIG_METHOD_LABEL, pAd->pP2pCtrll->P2PTable.Client[GrpIndex].GeneralToken, pAd->pP2pCtrll->P2PTable.Client[GrpIndex].addr, &FrameLen);
                    DBGPRINT(RT_DEBUG_TRACE, ("Request : CONFIG_METHOD_LABEL  \n"));
                }
                else if ((pAd->pP2pCtrll->P2PTable.Client[GrpIndex].ConfigMethod&CONFIG_METHOD_KEYPAD) != 0)
                {
                    pAd->pP2pCtrll->P2PTable.Client[GrpIndex].P2pClientState = P2PSTATE_SENT_PROVISION_REQ;
                    P2PSendProvisionReq(pAd, CONFIG_METHOD_KEYPAD, pAd->pP2pCtrll->P2PTable.Client[GrpIndex].GeneralToken, pAd->pP2pCtrll->P2PTable.Client[GrpIndex].addr, &FrameLen);
                    DBGPRINT(RT_DEBUG_TRACE, ("Request : CONFIG_METHOD_KEYPAD  \n"));
                }
                else
                    P2PSendProvisionReq(pAd, pAd->pP2pCtrll->P2PTable.Client[GrpIndex].ConfigMethod, pAd->pP2pCtrll->P2PTable.Client[GrpIndex].GeneralToken, pAd->pP2pCtrll->P2PTable.Client[GrpIndex].addr, &FrameLen);
                }
        }
        DBGPRINT(RT_DEBUG_TRACE, ("pPort->P2PCfg.Dpid = %d \n", pPort->P2PCfg.Dpid));
        P2pStopScan(pAd, pPort);
    }

    
    // Peer is a P2P Device. Need to have Go Nego procedure first.
    // Or can reinvoke a persistent 
    if ((pAd->pP2pCtrll->P2PTable.Client[GrpIndex].P2pClientState == P2PSTATE_DISCOVERY)
        // || pAd->pP2pCtrll->P2PTable.Client[GrpIndex].P2pClientState == P2PSTATE_SENT_PROVISION_REQ
        || (pPort->P2PCfg.SigmaSetting.ConfigMode == CONFIG_MODE_SIGMA_SEND_P2P_INVITATION_REQ))
    {
        DBGPRINT(RT_DEBUG_TRACE, (" P2pConnectAfterScan   PortSubtype = %d \n",pPort->PortSubtype));
        // Decide connect method when I am in Connect_Idle state. This means I am a P2P Device.
        if ((IS_P2P_CLIENT_OP(pPort) ||(pPort->PortSubtype == PORTSUBTYPE_P2PGO)) && (!P2P_TEST_FLAG(&pAd->pP2pCtrll->P2PTable.Client[GrpIndex], P2PFLAG_PASSED_NEGO_REQ)) )
        {
            // Invite Case 1: I am GO or I am client invite other deveice join us
            pAd->pP2pCtrll->P2PTable.Client[GrpIndex].P2pClientState = P2PSTATE_INVITE_COMMAND;
            DBGPRINT(RT_DEBUG_TRACE, (" P2pConnectAfterScan -  Use Invite %d.   PortSubtype = %d\n", GrpIndex, pPort->PortSubtype));
            bAction = TRUE;
        }
        else if (IS_P2P_CONNECT_IDLE(pPort) || (pPort->P2PCfg.SigmaSetting.ConfigMode == CONFIG_MODE_SIGMA_SEND_P2P_INVITATION_REQ))
        {
            
            DBGPRINT(RT_DEBUG_TRACE, ("P2pConnectAfterScan - Perst index %d.  \n", index));
            DBGPRINT(RT_DEBUG_TRACE, ("Search addr is %x %x %x %x %x %x.  \n", pAd->pP2pCtrll->P2PTable.Client[GrpIndex].addr[0],
                pAd->pP2pCtrll->P2PTable.Client[GrpIndex].addr[1], pAd->pP2pCtrll->P2PTable.Client[GrpIndex].addr[2], pAd->pP2pCtrll->P2PTable.Client[GrpIndex].addr[3],
                pAd->pP2pCtrll->P2PTable.Client[GrpIndex].addr[4], pAd->pP2pCtrll->P2PTable.Client[GrpIndex].addr[5]));
            DBGPRINT(RT_DEBUG_TRACE, (" P2pConnectAfterScan - EnablePresistent %d. P2PFLAG_PASSED_NEGO_REQ = %d\n", pPort->CommonCfg.P2pControl.field.EnablePresistent,(P2P_TEST_FLAG(&pAd->pP2pCtrll->P2PTable.Client[GrpIndex], P2PFLAG_PASSED_NEGO_REQ))));
            if ((index < MAX_P2P_TABLE_SIZE) 
                && (pPort->CommonCfg.P2pControl.field.EnablePresistent == 1)
                && (!P2P_TEST_FLAG(&pAd->pP2pCtrll->P2PTable.Client[GrpIndex], P2PFLAG_PASSED_NEGO_REQ))
                && ((pAd->pP2pCtrll->P2PTable.Client[GrpIndex].GroupCapability & GRPCAP_PERSISTENT) != 0))
            {
                // Invite Case 3:  just device reinvoke device
                pAd->pP2pCtrll->P2PTable.Client[GrpIndex].P2pClientState = P2PSTATE_INVITE_COMMAND;
                DBGPRINT(RT_DEBUG_TRACE, (" P2pConnectAfterScan -  Use reinvoke Invite index is %d.  \n", index));
                bAction = TRUE;
            }
            else if (IS_P2P_CONNECT_IDLE(pPort) && (pAd->pP2pCtrll->P2PTable.Client[GrpIndex].Rule != P2P_IS_GO))
            {
                DBGPRINT(RT_DEBUG_TRACE, (" P2pConnectAfterScan -  Start From Group Forming   \n"));

                // Start Scan and Then go to Group Forming process.
                pAd->pP2pCtrll->P2PTable.Client[idx].P2pClientState = P2PSTATE_CONNECT_COMMAND;
                bAction = TRUE;
            }
        }
        if (bAction == TRUE)
        {
            P2pStopScan(pAd, pPort);
            // Now only support connect to ONE. So set ConnectingIndex to MAX_P2P_GROUP_SIZE to stop connect further MAC.
            P2pStopConnectThis(pAd, pPort);
            P2pConnectAction(pAd, pPort, bBeacon, idx);
        }
    }
    // peer is a P2P GO.
    else if ((pAd->pP2pCtrll->P2PTable.Client[GrpIndex].P2pClientState == P2PSTATE_DISCOVERY_GO)
        || pAd->pP2pCtrll->P2PTable.Client[GrpIndex].P2pClientState == P2PSTATE_SENT_PROVISION_REQ)
    {
        DBGPRINT(RT_DEBUG_TRACE, ("Jesse in case 5566 = %d test flag = %d index = %d", (pAd->pP2pCtrll->P2PTable.Client[GrpIndex].GroupCapability & GRPCAP_PERSISTENT), P2P_TEST_FLAG(&pAd->pP2pCtrll->P2PTable.Client[idx], P2PFLAG_GO_INVITE_ME), index));
        if(IS_P2P_SIGMA_ON(pPort))
        {
            DBGPRINT(RT_DEBUG_TRACE, (" sigma use peer is go  \n"));
            //P2pStopConnectThis(pPort);
            P2pConnectP2pGo(pAd, pPort, GrpIndex);
        }
        
        if((pAd->pP2pCtrll->P2PTable.Client[GrpIndex].P2pClientState == P2PSTATE_DISCOVERY_GO)
             && (pPort->CommonCfg.P2pControl.field.EnablePresistent == 1)
             && ((pAd->pP2pCtrll->P2PTable.Client[GrpIndex].GroupCapability & GRPCAP_PERSISTENT) != 0)
             /*&& (!P2P_TEST_FLAG(&pAd->pP2pCtrll->P2PTable.Client[idx], P2PFLAG_GO_INVITE_ME))*/)
        {
            if(index < MAX_P2P_TABLE_SIZE)
            {
                // Invite Case 
                pAd->pP2pCtrll->P2PTable.Client[GrpIndex].P2pClientState = P2PSTATE_INVITE_COMMAND;
                    DBGPRINT(RT_DEBUG_TRACE, ("case 0 = not use provision : P2pConnectAfterScan : both have persistent on -  Use reinvoke Invite %d. is beacon = %d \n", index, bBeacon));
                P2pStopScan(pAd, pPort);
                // Now only support connect to ONE. So set ConnectingIndex to MAX_P2P_GROUP_SIZE to stop connect further MAC.
                P2pStopConnectThis(pAd, pPort);
                bresult = P2pInvite(pAd, pPort, pAd->pP2pCtrll->P2PTable.Client[GrpIndex].addr, index, GrpIndex);
                if (bresult == TRUE)
                    pPort->P2PCfg.P2PConnectState = P2P_INVITE;
            }
            else if(index == P2P_NOT_FOUND)
            {
                ULONG   FrameLen;
                DBGPRINT(RT_DEBUG_TRACE, (" case 1 = Both persistent on I do not have persistent table. Use peovision\n"));
                if ((pPort->P2PCfg.Dpid == DEV_PASS_ID_USER) && ((pAd->pP2pCtrll->P2PTable.Client[GrpIndex].ConfigMethod&CONFIG_METHOD_DISPLAY) != 0))
                {
                    pAd->pP2pCtrll->P2PTable.Client[GrpIndex].P2pClientState = P2PSTATE_SENT_PROVISION_REQ;
                    P2PSendProvisionReq(pAd, CONFIG_METHOD_DISPLAY, pAd->pP2pCtrll->P2PTable.Client[GrpIndex].GeneralToken, pAd->pP2pCtrll->P2PTable.Client[GrpIndex].addr, &FrameLen);
                    DBGPRINT(RT_DEBUG_TRACE, ("Request : CONFIG_METHOD_DISPLAY  \n"));
                }
                else if ((pPort->P2PCfg.Dpid == DEV_PASS_ID_PBC) && ((pAd->pP2pCtrll->P2PTable.Client[GrpIndex].ConfigMethod&CONFIG_METHOD_PUSHBUTTON) != 0))
                {
                    pAd->pP2pCtrll->P2PTable.Client[GrpIndex].P2pClientState = P2PSTATE_SENT_PROVISION_REQ;
                    P2PSendProvisionReq(pAd, CONFIG_METHOD_PUSHBUTTON, pAd->pP2pCtrll->P2PTable.Client[GrpIndex].GeneralToken, pAd->pP2pCtrll->P2PTable.Client[GrpIndex].addr, &FrameLen);
                    DBGPRINT(RT_DEBUG_TRACE, ("Request : CONFIG_METHOD_PUSHBUTTON  \n"));
                }
                else if ((pPort->P2PCfg.Dpid == DEV_PASS_ID_REG) && ((pAd->pP2pCtrll->P2PTable.Client[GrpIndex].ConfigMethod&CONFIG_METHOD_KEYPAD) != 0))
                {
                    pAd->pP2pCtrll->P2PTable.Client[GrpIndex].P2pClientState = P2PSTATE_SENT_PROVISION_REQ;
                    P2PSendProvisionReq(pAd, CONFIG_METHOD_KEYPAD, pAd->pP2pCtrll->P2PTable.Client[GrpIndex].GeneralToken, pAd->pP2pCtrll->P2PTable.Client[GrpIndex].addr, &FrameLen);
                    DBGPRINT(RT_DEBUG_TRACE, ("Request : CONFIG_METHOD_KEYPAD  \n"));
                }
                else
                {
                            P2PSendProvisionReq(pAd, pAd->pP2pCtrll->P2PTable.Client[GrpIndex].ConfigMethod, pAd->pP2pCtrll->P2PTable.Client[GrpIndex].GeneralToken, pAd->pP2pCtrll->P2PTable.Client[GrpIndex].addr, &FrameLen);
                }
            }
            
        }
        else if((pAd->pP2pCtrll->P2PTable.Client[GrpIndex].P2pClientState == P2PSTATE_DISCOVERY_GO)
             && (pPort->CommonCfg.P2pControl.field.EnablePresistent == 1)
             && ((pAd->pP2pCtrll->P2PTable.Client[GrpIndex].GroupCapability & GRPCAP_PERSISTENT) == 0)
             /* && (!P2P_TEST_FLAG(&pAd->pP2pCtrll->P2PTable.Client[idx], P2PFLAG_GO_INVITE_ME))*/)
        {
            ULONG   FrameLen;
            DBGPRINT(RT_DEBUG_TRACE, (" case 2 = My persistent on but peer(GO) is off. Use peovision\n"));
            if ((pPort->P2PCfg.Dpid == DEV_PASS_ID_USER) && ((pAd->pP2pCtrll->P2PTable.Client[GrpIndex].ConfigMethod&CONFIG_METHOD_DISPLAY) != 0))
            {
                pAd->pP2pCtrll->P2PTable.Client[GrpIndex].P2pClientState = P2PSTATE_SENT_PROVISION_REQ;
                P2PSendProvisionReq(pAd, CONFIG_METHOD_DISPLAY, pAd->pP2pCtrll->P2PTable.Client[GrpIndex].GeneralToken, pAd->pP2pCtrll->P2PTable.Client[GrpIndex].addr, &FrameLen);
                DBGPRINT(RT_DEBUG_TRACE, ("Request : CONFIG_METHOD_DISPLAY  \n"));
            }
            else if ((pPort->P2PCfg.Dpid == DEV_PASS_ID_PBC) && ((pAd->pP2pCtrll->P2PTable.Client[GrpIndex].ConfigMethod&CONFIG_METHOD_PUSHBUTTON) != 0))
            {
                pAd->pP2pCtrll->P2PTable.Client[GrpIndex].P2pClientState = P2PSTATE_SENT_PROVISION_REQ;
                P2PSendProvisionReq(pAd, CONFIG_METHOD_PUSHBUTTON, pAd->pP2pCtrll->P2PTable.Client[GrpIndex].GeneralToken, pAd->pP2pCtrll->P2PTable.Client[GrpIndex].addr, &FrameLen);
                DBGPRINT(RT_DEBUG_TRACE, ("Request : CONFIG_METHOD_PUSHBUTTON  \n"));
            }
            else if ((pPort->P2PCfg.Dpid == DEV_PASS_ID_REG) && ((pAd->pP2pCtrll->P2PTable.Client[GrpIndex].ConfigMethod&CONFIG_METHOD_KEYPAD) != 0))
            {
                pAd->pP2pCtrll->P2PTable.Client[GrpIndex].P2pClientState = P2PSTATE_SENT_PROVISION_REQ;
                P2PSendProvisionReq(pAd, CONFIG_METHOD_KEYPAD, pAd->pP2pCtrll->P2PTable.Client[GrpIndex].GeneralToken, pAd->pP2pCtrll->P2PTable.Client[GrpIndex].addr, &FrameLen);
                DBGPRINT(RT_DEBUG_TRACE, ("Request : CONFIG_METHOD_KEYPAD  \n"));
            }
            else
            {
                P2PSendProvisionReq(pAd, pAd->pP2pCtrll->P2PTable.Client[GrpIndex].ConfigMethod, pAd->pP2pCtrll->P2PTable.Client[GrpIndex].GeneralToken, pAd->pP2pCtrll->P2PTable.Client[GrpIndex].addr, &FrameLen);
            }
                    
        }
        
    }
    // peer is a P2P client in a P2P Group.
    else if ((pAd->pP2pCtrll->P2PTable.Client[GrpIndex].P2pClientState == P2PSTATE_DISCOVERY_CLIENT))
    {
        DBGPRINT(RT_DEBUG_TRACE, (" case 2  = peer is client  \n"));
        P2pStopConnectThis(pAd, pPort);
        P2pConnectP2pClient(pAd,GrpIndex);
    }
    else
        DBGPRINT(RT_DEBUG_TRACE, (" P2pConnectAfterScan - Peer state %s \n", decodeP2PClientState(pAd->pP2pCtrll->P2PTable.Client[GrpIndex].P2pClientState)));

    return brc;
}

/*  
    ==========================================================================
    Description: 
        Prepare to connect to Connecting MAC. ConnectingMAC might contain several MAC address that I can connect to One after one.
        
    Parameters: 
    Note:
         
    ==========================================================================
 */
VOID P2pConnectAction(
    IN PMP_ADAPTER pAd, 
    IN PMP_PORT     pPort,
    IN BOOLEAN  bBeacon,
    IN UCHAR        index)
{
    BOOLEAN     bresult = FALSE;
//    PMP_PORT pPort;
    UCHAR       perstindex;

    DBGPRINT(RT_DEBUG_TRACE, ("P2pConnectAction   %d. %s \n", pAd->pP2pCtrll->P2PTable.Client[index].P2pClientState, decodeP2PClientState(pAd->pP2pCtrll->P2PTable.Client[index].P2pClientState)));
   // pPort = pAd->PortList[pPort->P2PCfg.PortNumber];
    if (bBeacon == TRUE)
    {
        DBGPRINT(RT_DEBUG_TRACE, (" P2pConnectAction from bBeacon \n"));
        //Check If ever have a command to connect to this peer.
        if (pAd->pP2pCtrll->P2PTable.Client[index].P2pClientState == P2PSTATE_CONNECT_COMMAND
            ||(pAd->pP2pCtrll->P2PTable.Client[index].P2pClientState == P2PSTATE_INVITE_COMMAND))
        {
            pPort->P2PCfg.P2PConnectState = P2P_ANY_IN_FORMATION_AS_CLIENT;
            P2pGoNegoDone(pAd, pPort, &pAd->pP2pCtrll->P2PTable.Client[index]);
        }
        else if (pAd->pP2pCtrll->P2PTable.Client[index].P2pClientState == P2PSTATE_GO_DISCO_COMMAND)
        {
            bresult = P2pClientDiscovery(pAd, pPort, pAd->pP2pCtrll->P2PTable.Client[index].addr, index);
        }
    }
    else
    {       
        DBGPRINT(RT_DEBUG_TRACE, (" P2pConnectAction  not from bBeacon \n"));
        if ((pAd->pP2pCtrll->P2PTable.Client[index].P2pClientState == P2PSTATE_CONNECT_COMMAND)
            || (pAd->pP2pCtrll->P2PTable.Client[index].P2pClientState == P2PSTATE_INVITE_COMMAND))
        {
            // The check sequence must be the same as in where we set P2PSTATE_INVITE_COMMAND in P2PConnect()
            if ((pPort->PortSubtype == PORTSUBTYPE_P2PGO) ||(IS_P2P_CLIENT_OP(pPort)))
            {
                DBGPRINT(RT_DEBUG_TRACE, ("  case 1 \n"));
                // Invite Case 1 : I am Auto GO to invite a P2P Device or when I am P2P Client
                perstindex = P2pPerstTabSearch(pAd, pAd->pP2pCtrll->P2PTable.Client[index].addr); 
                DBGPRINT(RT_DEBUG_TRACE, ("invite perstindex = %d pPort->CommonCfg.P2pControl.field.EnablePresistent = %d \n", perstindex, pPort->CommonCfg.P2pControl.field.EnablePresistent));
                DBGPRINT(RT_DEBUG_TRACE, ("0x02%x - 0x02%x - 0x02%x - 0x02%x - 0x02%x - 0x02%x", pAd->pP2pCtrll->P2PTable.Client[index].addr[0],pAd->pP2pCtrll->P2PTable.Client[index].addr[1]
                    ,pAd->pP2pCtrll->P2PTable.Client[index].addr[2], pAd->pP2pCtrll->P2PTable.Client[index].addr[3], pAd->pP2pCtrll->P2PTable.Client[index].addr[4], pAd->pP2pCtrll->P2PTable.Client[index].addr[5]));
                    
                if ((perstindex < MAX_P2P_TABLE_SIZE) 
                    && (pPort->CommonCfg.P2pControl.field.EnablePresistent == 1)
                    && ((pAd->pP2pCtrll->P2PTable.Client[index].GroupCapability & GRPCAP_PERSISTENT) != 0)) 
                {
                    DBGPRINT(RT_DEBUG_TRACE, (" case 1-1 : I am GO and I want to invite persistent client. This Client also support Persistent", perstindex, pPort->CommonCfg.P2pControl.field.EnablePresistent));
                    bresult = P2pInvite(pAd, pPort, pAd->pP2pCtrll->P2PTable.Client[index].addr, perstindex, index);
                    if (bresult == TRUE) 
                        pPort->P2PCfg.P2PConnectState = P2P_INVITE;
                }
                else    
                {
                    DBGPRINT(RT_DEBUG_TRACE, ("  case 1-2 : My PortSubType is GO use invite without reinvoke\n"));
                    bresult = P2pInvite(pAd, pPort, pAd->pP2pCtrll->P2PTable.Client[index].addr, MAX_P2P_TABLE_SIZE, index);
                }
            }
            else if (pAd->pP2pCtrll->P2PTable.Client[index].Rule == P2P_IS_GO)
            {

                DBGPRINT(RT_DEBUG_TRACE, ("  case 2 : I am client and I want to connect GO. \n"));
                // directly associate to  GO.
                pPort->P2PCfg.P2PConnectState = P2P_ANY_IN_FORMATION_AS_CLIENT;
                P2pGoNegoDone(pAd, pPort, &pAd->pP2pCtrll->P2PTable.Client[index]);
            }
            else if (IS_P2P_CONNECT_IDLE(pPort))
            {
                DBGPRINT(RT_DEBUG_TRACE, ("  case 3 \n"));
                // since I am idle, 
                perstindex = P2pPerstTabSearch(pAd, pAd->pP2pCtrll->P2PTable.Client[index].addr);
                if ((perstindex < MAX_P2P_TABLE_SIZE) 
                    && ( IS_P2P_SIGMA_ON(pPort) ? (TRUE) : (pPort->CommonCfg.P2pControl.field.EnablePresistent == 1))
                    &&( IS_P2P_SIGMA_ON(pPort) ? (TRUE) : ((pAd->pP2pCtrll->P2PTable.Client[index].GroupCapability & GRPCAP_PERSISTENT) != 0))
                    && (!P2P_TEST_FLAG(&pAd->pP2pCtrll->P2PTable.Client[index], P2PFLAG_PASSED_NEGO_REQ)))
                {
                    // I have credential, my persistent is enabled, peer 's persistent is enabled.  
                    // So use Reinvoke method to start P2P group.
                    DBGPRINT(RT_DEBUG_TRACE, ("  case 3 : I have credential, my persistent is enabled, peer 's persistent is enabled.\n"));
                    
                    bresult = P2pInvite(pAd, pPort, pAd->pP2pCtrll->P2PTable.Client[index].addr, perstindex, index);
                    if (bresult == TRUE)
                        pPort->P2PCfg.P2PConnectState = P2P_INVITE;
                }
                else
                {
                    DBGPRINT(RT_DEBUG_TRACE, ("  case 3-1 P2pStartGroupForm \n"));
                    bresult = P2pStartGroupForm(pAd, pPort, pAd->pP2pCtrll->P2PTable.Client[index].addr, index);
                }
            }
            else
                DBGPRINT(RT_DEBUG_TRACE, ("  case 4 ?????? \n"));
            
            if (bresult == TRUE)
            {
            }
        }
        
    }
    pPort->P2PCfg.P2pCounter.GoScanBeginCounter100ms = 0;
}

/*  
    ==========================================================================
    Description: 
        Connect to Addr. Addr is already in a P2P Group.
        
    Parameters: 
    Note:
         
    ==========================================================================
 */
VOID P2pConnectP2pClient(
    IN PMP_ADAPTER pAd, 
    IN UCHAR        GrpIndex)
{
    BOOLEAN     bGoToScan = FALSE;
    UCHAR       GoP2pIndex;
    PMP_PORT    pPort = pAd->PortList[FXXK_PORT_0];
    
    DBGPRINT(RT_DEBUG_TRACE,("P2pConnect to P2pClient====>.  \n"));
    
    // Decide connect method when I am in COnnect_Idle state. This means I am a P2P Device.
    if (IS_P2P_CONNECT_IDLE(pPort))
    {
        GoP2pIndex = pAd->pP2pCtrll->P2PTable.Client[GrpIndex].MyGOIndex;
        if ((pAd->pP2pCtrll->P2PTable.Client[GrpIndex].DevCapability & DEVCAP_CLIENT_DISCOVER) == 0)
        {
            DBGPRINT(RT_DEBUG_TRACE,("P2P Client Not support Discoverability  %x \n", pAd->pP2pCtrll->P2PTable.Client[GrpIndex].DevCapability));
        }
        else
            DBGPRINT(RT_DEBUG_TRACE,("P2P Client CAN Support Discoverability  %x \n", pAd->pP2pCtrll->P2PTable.Client[GrpIndex].DevCapability));

        DBGPRINT(RT_DEBUG_TRACE,("P2P Client DevCapability  %x. GoP2pIndex = %d. \n", pAd->pP2pCtrll->P2PTable.Client[GrpIndex].DevCapability, GoP2pIndex));
        if ((GoP2pIndex < MAX_P2P_GROUP_SIZE) && (pAd->pP2pCtrll->P2PTable.Client[GoP2pIndex].P2pClientState == P2PSTATE_DISCOVERY_GO))
        {
            pAd->pP2pCtrll->P2PTable.Client[GrpIndex].P2pClientState = P2PSTATE_CLIENT_DISCO_COMMAND;
            pAd->pP2pCtrll->P2PTable.Client[GoP2pIndex].P2pClientState = P2PSTATE_GO_DISCO_COMMAND;
            DBGPRINT(RT_DEBUG_TRACE,("P2P  GrpIndex  %d . GoP2pIndex = %d\n", GrpIndex, GoP2pIndex));
            bGoToScan = TRUE;   
        }
        DBGPRINT(RT_DEBUG_TRACE, (" P2pConnectIdle - peer device's state %s \n", decodeP2PClientState(pAd->pP2pCtrll->P2PTable.Client[GrpIndex].P2pClientState)));
    }
    // Decide connect method when I am in Connect_Idle state. This means I am a P2P GO, or a P2P Client.
    else if (IS_P2P_GO_OP(pPort))
    {
        
    }
    else
        DBGPRINT(RT_DEBUG_TRACE, (" P2pConnectP2pClient - P2P state %s \n", decodeP2PState(pPort->P2PCfg.P2PConnectState)));
        
    if (bGoToScan == TRUE)
    {
        P2pSetListenIntBias(pAd,  12);
        pPort->P2PCfg.P2pCounter.CounterAftrScanButton = 0;
        P2pGotoScan(pAd, pPort, P2P_DISCO_TYPE_DEFAULT, 2);
    }
}

/*  
    ==========================================================================
    Description: 
        Connect to Addr. Addr is already in a P2P Group.
        
    Parameters: 
    Note:
         
    ==========================================================================
 */
BOOLEAN P2pConnectP2pGo(
    IN PMP_ADAPTER pAd, 
    IN PMP_PORT     pPort,
    IN UCHAR        idx)
{
    BOOLEAN brc = FALSE;
    
    DBGPRINT(RT_DEBUG_TRACE,("P2pConnectP2pGo.  %s\n", decodeP2PState(pPort->P2PCfg.P2PConnectState)));
    DBGPRINT(RT_DEBUG_TRACE, ("pPort->P2PCfg.Dpid = %d \n", pPort->P2PCfg.Dpid));
    // Decide connect method when I am in COnnect_Idle state. This means I am a P2P Device.
    
    if (pPort->P2PCfg.P2PConnectState == P2P_CONNECT_IDLE || pPort->P2PCfg.P2PConnectState == P2P_NEGOTIATION)
    {
        if(IS_P2P_SIGMA_ON(pPort))
        {
            // If this GO invited me before..and we already choose a config method.. Then I don't want to provision discovery this time.
            // start associate to this GO now~
            if (P2P_TEST_FLAG(&pAd->pP2pCtrll->P2PTable.Client[idx], P2PFLAG_GO_INVITE_ME)
                && ((pAd->pP2pCtrll->P2PTable.Client[idx].ConfigMethod == CONFIG_METHOD_DISPLAY)
                //||(pAd->pP2pCtrll->P2PTable.Client[idx].ConfigMethod == CONFIG_METHOD_KEYPAD)
                ||(pAd->pP2pCtrll->P2PTable.Client[idx].ConfigMethod == CONFIG_METHOD_PUSHBUTTON)))
            {
                DBGPRINT(RT_DEBUG_TRACE, ("Decide to Join P2p group? when I am %s \n", decodeP2PState(pPort->P2PCfg.P2PConnectState)));
                if (pPort->P2PCfg.P2PConnectState == P2P_CONNECT_IDLE || pPort->P2PCfg.P2PConnectState == P2P_NEGOTIATION)
                {
                    DBGPRINT(RT_DEBUG_TRACE,("P2p :connecting to GO with Bssid   %x.  %x. %x.  %x. %x.  %x. \n", pAd->pP2pCtrll->P2PTable.Client[idx].bssid[0], pAd->pP2pCtrll->P2PTable.Client[idx].bssid[1],pAd->pP2pCtrll->P2PTable.Client[idx].bssid[2],pAd->pP2pCtrll->P2PTable.Client[idx].bssid[3],pAd->pP2pCtrll->P2PTable.Client[idx].bssid[4],pAd->pP2pCtrll->P2PTable.Client[idx].bssid[5]));
                    DBGPRINT(RT_DEBUG_TRACE,("P2p : its GroupCapability= %x.  DevCapability= %x. \n", pAd->pP2pCtrll->P2PTable.Client[idx].GroupCapability, pAd->pP2pCtrll->P2PTable.Client[idx].DevCapability));
                    pPort->P2PCfg.P2PConnectState = P2P_ANY_IN_FORMATION_AS_CLIENT;
                    pAd->pP2pCtrll->P2PTable.Client[idx].P2pClientState = P2PSTATE_GO_WPS;
                    PlatformMoveMemory(pAd->PortList[pPort->P2PCfg.PortNumber]->PortCfg.Bssid, pAd->pP2pCtrll->P2PTable.Client[idx].bssid, MAC_ADDR_LEN);
                    PlatformMoveMemory(pPort->P2PCfg.Bssid, pAd->pP2pCtrll->P2PTable.Client[idx].bssid, MAC_ADDR_LEN);
                    PlatformMoveMemory(pAd->PortList[pPort->P2PCfg.PortNumber]->PortCfg.Ssid, pAd->pP2pCtrll->P2PTable.Client[idx].Ssid, 32);
                    PlatformMoveMemory(pPort->P2PCfg.SSID, pAd->pP2pCtrll->P2PTable.Client[idx].Ssid, 32);
                    pPort->P2PCfg.SSIDLen = pAd->pP2pCtrll->P2PTable.Client[idx].SsidLen;
                    pAd->PortList[pPort->P2PCfg.PortNumber]->PortCfg.SsidLen = pAd->pP2pCtrll->P2PTable.Client[idx].SsidLen;
                    pPort->P2PCfg.GroupOpChannel = pAd->pP2pCtrll->P2PTable.Client[idx].OpChannel;
                    P2pGoNegoDone(pAd, pAd->PortList[pPort->P2PCfg.PortNumber], &pAd->pP2pCtrll->P2PTable.Client[idx]);
                }
                P2P_SET_FLAG(&pAd->pP2pCtrll->P2PTable.Client[idx], P2PFLAG_PROVISIONED);
            }
            else
            {
                PlatformMoveMemory(pPort->P2PCfg.SSID, pAd->pP2pCtrll->P2PTable.Client[idx].Ssid, 32);
                pPort->P2PCfg.SSIDLen = pAd->pP2pCtrll->P2PTable.Client[idx].SsidLen;
                pAd->pP2pCtrll->P2PTable.Client[idx].ConfigTimeOut = 250;  // retry 5 times, every 5 second
                pAd->pP2pCtrll->P2PTable.Client[idx].P2pClientState = P2PSTATE_PROVISION_COMMAND;
                DBGPRINT(RT_DEBUG_TRACE,("P2p : Use Provision first before connecting to GO with Bssid   %x.  %x. %x.  %x. %x.  %x. \n", pAd->pP2pCtrll->P2PTable.Client[idx].bssid[0], pAd->pP2pCtrll->P2PTable.Client[idx].bssid[1],pAd->pP2pCtrll->P2PTable.Client[idx].bssid[2],pAd->pP2pCtrll->P2PTable.Client[idx].bssid[3],pAd->pP2pCtrll->P2PTable.Client[idx].bssid[4],pAd->pP2pCtrll->P2PTable.Client[idx].bssid[5]));
            }
            brc = TRUE;
            DBGPRINT(RT_DEBUG_TRACE,("P2p : its GroupCapability= %x.  DevCapability= %x. \n", pAd->pP2pCtrll->P2PTable.Client[idx].GroupCapability, pAd->pP2pCtrll->P2PTable.Client[idx].DevCapability));
        }
        else
        {
            // If this GO invited me before..and we already choose a config method.. Then I don't want to provision discovery this time.
            // start associate to this GO now~
            PlatformMoveMemory(pPort->P2PCfg.SSID, pAd->pP2pCtrll->P2PTable.Client[idx].Ssid, 32);
            pPort->P2PCfg.SSIDLen = pAd->pP2pCtrll->P2PTable.Client[idx].SsidLen;
            pAd->pP2pCtrll->P2PTable.Client[idx].ConfigTimeOut = 250;  // retry 5 times, every 5 second
            pAd->pP2pCtrll->P2PTable.Client[idx].P2pClientState = P2PSTATE_PROVISION_COMMAND;
            DBGPRINT(RT_DEBUG_TRACE,("P2p : Use Provision first before connecting to GO with Bssid   %x.  %x. %x.  %x. %x.  %x. \n", pAd->pP2pCtrll->P2PTable.Client[idx].bssid[0], pAd->pP2pCtrll->P2PTable.Client[idx].bssid[1],pAd->pP2pCtrll->P2PTable.Client[idx].bssid[2],pAd->pP2pCtrll->P2PTable.Client[idx].bssid[3],pAd->pP2pCtrll->P2PTable.Client[idx].bssid[4],pAd->pP2pCtrll->P2PTable.Client[idx].bssid[5]));
            DBGPRINT(RT_DEBUG_TRACE,("P2p : its GroupCapability= %x.  DevCapability= %x. \n", pAd->pP2pCtrll->P2PTable.Client[idx].GroupCapability, pAd->pP2pCtrll->P2PTable.Client[idx].DevCapability));

            brc = TRUE;
        }
        
    }
    else if (IS_P2P_CLIENT_OP(pPort))
    {
    }
    else
        DBGPRINT(RT_DEBUG_TRACE, ("invalid P2pConnectP2pGo command when %s \n", decodeP2PState(pPort->P2PCfg.P2PConnectState)));

    return brc;
}

NDIS_STATUS P2pDisconnectOneConnection(
    IN PMP_ADAPTER pAd, 
    IN PMP_PORT     pPort,
    IN ULONG InformationBufferLength,
    IN PVOID pInformationBuffer) 
{
    UCHAR DisconnectAddr[6];
    UCHAR i, P2pinex;
    MLME_DEAUTH_REQ_STRUCT      DeAuthRequest;
    MLME_DISASSOC_REQ_STRUCT  DeAssocRequest;
    MAC_TABLE_ENTRY *pEntry;
    //PMP_PORT          pPort = pAd->PortList[pPort->P2PCfg.PortNumber];
    ULONG               DeleteBssTab;
    
    PlatformMoveMemory(&DisconnectAddr[0], pInformationBuffer,InformationBufferLength);

    pEntry = MacTableLookup(pAd,  pPort, DisconnectAddr);
    P2pinex = P2pGroupTabSearch(pAd,DisconnectAddr);
    DeleteBssTab = BssTableSearch(pAd, pPort, &pAd->ScanTab, DisconnectAddr, pPort->Channel);
    
    DBGPRINT(RT_DEBUG_TRACE, ("P2pDisconnectOneConnection pEntry = %d P2pinex = %d",pEntry, P2pinex));

    
    if(pPort->PortSubtype == PORTSUBTYPE_P2PGO) 
    {
        
        DBGPRINT(RT_DEBUG_TRACE, ("I am P2P GO and disconnect %x %x %x %x %x %x", DisconnectAddr[0], DisconnectAddr[1], DisconnectAddr[2], DisconnectAddr[3], DisconnectAddr[4], DisconnectAddr[5]));
        if( (pAd->PortList[pPort->P2PCfg.PortNumber]->PortSubtype == PORTSUBTYPE_P2PClient) && (pPort->CommonCfg.P2pControl.field.EnablePresistent == 1))
        {
            pPort->P2PCfg.P2pCounter.CounterAutoReconnectForP2pClient = 10;
        }

        if((pEntry != NULL) && 
            (IS_P2P_SIGMA_OFF(pPort)) && 
            (pPort->CommonCfg.P2pControl.field.EnablePresistent == TRUE))
        {
            // manual disconnect and persistent need use special reason call to tell Client not auto reconnect back 
            DeAuthRequest.Reason = REASON_P2P_PEER_MANUAL_DEAUTH;
            DeAssocRequest.Reason = REASON_P2P_PEER_MANUAL_DEAUTH;
            DBGPRINT(RT_DEBUG_TRACE, ("REASON_P2P_PEER_MANUAL_DEAUTH 51 "));
        }
        else
        {
            DeAuthRequest.Reason = REASON_DEAUTH_STA_LEAVING;
            DeAssocRequest.Reason = REASON_DISASSOC_STA_LEAVING;
            DBGPRINT(RT_DEBUG_TRACE, ("REASON_DEAUTH_STA_LEAVING 3 "));
        }
        
        GoPeerDisassocReq(pAd, pEntry, DisconnectAddr);
        
        if(pEntry != NULL)
            MacTableDeleteAndResetEntry(pAd,pPort, pEntry->Aid, DisconnectAddr, TRUE);
        else
        {
            if (pPort->MacTab.Size)
                pPort->P2PCfg.P2PConnectState = P2P_I_AM_GO_OP;
            else 
                pPort->P2PCfg.P2PConnectState = P2P_ANY_IN_FORMATION_AS_GO;   
            
        }
        
        
        PlatformMoveMemory(&DeAssocRequest.Addr[0], DisconnectAddr,MAC_ADDRESS_LENGTH);
        PlatformMoveMemory(&DeAuthRequest.Addr[0], DisconnectAddr,MAC_ADDRESS_LENGTH);
        
        MlmeEnqueue(pAd, pPort, AP_AUTH_STATE_MACHINE, APMT2_MLME_DEAUTH_REQ, sizeof(MLME_DEAUTH_REQ_STRUCT), &DeAuthRequest);
        MlmeEnqueue(pAd, pPort, AP_ASSOC_STATE_MACHINE, APMT2_MLME_DISASSOC_REQ, sizeof(MLME_DISASSOC_REQ_STRUCT), &DeAssocRequest);
        pPort->P2PCfg.Dpid = DEV_PASS_ID_NOSPEC;
        PlatformZeroMemory( pPort->P2PCfg.PinCode, 8);
        PlatformZeroMemory( pPort->StaCfg.WscControl.RegData.PIN, 8);
        pPort->StaCfg.WscControl.RegData.PINLen = 0;
        pPort->P2PCfg.P2pCounter.CounterP2PConnectionFail = 0xffffffff;
        P2P_CLEAR_FLAG(&pAd->pP2pCtrll->P2PTable.Client[P2pinex], P2PFLAG_PROVISIONED);
        P2P_CLEAR_FLAG(&pAd->pP2pCtrll->P2PTable.Client[P2pinex], P2PFLAG_PASSED_NEGO_REQ);

        return NDIS_STATUS_SUCCESS;
    }
    else if(pPort->PortSubtype == PORTSUBTYPE_P2PClient)
    {
        DBGPRINT(RT_DEBUG_TRACE, ("I am P2P Client and disconnect %x %x %x %x %x %x", DisconnectAddr[0], DisconnectAddr[1], DisconnectAddr[2], DisconnectAddr[3], DisconnectAddr[4], DisconnectAddr[5]));
        P2pDown(pAd, pPort);
        
        for (i = 0;i < MAX_P2P_GROUP_SIZE;i++)
        {
            if (pAd->pP2pCtrll->P2PTable.Client[i].P2pClientState > P2PSTATE_DISCOVERY_UNKNOWN)
                P2pGroupTabDelete(pAd, i, pAd->pP2pCtrll->P2PTable.Client[i].addr);
        }

        pPort->P2PCfg.P2PDiscoProvState = P2P_ENABLE_LISTEN_ONLY;
        // go to scan since no conenction right now.
        if ((!INFRA_ON(pAd->PortList[PORT_0])) && (!ADHOC_ON(pAd->PortList[PORT_0])))
        {
            // Don't need to scan too much. 
            pPort->P2PCfg.P2pCounter.CounterAftrScanButton = P2P_SCAN_PERIOD/2;
            pPort->P2PCfg.P2PConnectState = P2P_CONNECT_IDLE;
            P2pGotoScan(pAd, pPort, P2P_DISCO_TYPE_AUTO, 2);
            pPort->P2PCfg.Dpid = DEV_PASS_ID_NOSPEC;
        }
        else if ((pAd->OpMode == OPMODE_STA) && (!INFRA_ON(pAd->PortList[PORT_0])) && (!ADHOC_ON(pAd->PortList[PORT_0])))
        {
            //AsicSwitchChannel(pAd, pPort->P2PCfg.ListenChannel, FALSE);
            AsicSwitchChannel(pPort, pPort->P2PCfg.ListenChannel, pPort->P2PCfg.ListenChannel, BW_20, FALSE);
        }

        if(DeleteBssTab != BSS_NOT_FOUND)
        {
            BssTableDeleteEntry(&pAd->ScanTab, DisconnectAddr, pAd->ScanTab.BssEntry[DeleteBssTab].Channel);
            DBGPRINT(RT_DEBUG_TRACE, ("delete scan table ====> ssid = %s channel = %d addr %x %x", pAd->ScanTab.BssEntry[DeleteBssTab].Ssid, pAd->ScanTab.BssEntry[DeleteBssTab].Channel, pAd->ScanTab.BssEntry[DeleteBssTab].Bssid[4],pAd->ScanTab.BssEntry[DeleteBssTab].Bssid[5]));
        }

            PlatformZeroMemory( pPort->P2PCfg.PinCode, 8);
            PlatformZeroMemory( pPort->StaCfg.WscControl.RegData.PIN, 8);
            pPort->StaCfg.WscControl.RegData.PINLen = 0;
            pPort->P2PCfg.P2pCounter.CounterP2PConnectionFail = 0xffffffff;

            P2P_CLEAR_FLAG(&pAd->pP2pCtrll->P2PTable.Client[P2pinex], P2PFLAG_PROVISIONED);
            P2P_CLEAR_FLAG(&pAd->pP2pCtrll->P2PTable.Client[P2pinex], P2PFLAG_PASSED_NEGO_REQ);
            return NDIS_STATUS_SUCCESS;
    }
    else 
    {
        DBGPRINT(RT_DEBUG_TRACE, ("Error Not define subtype"));
        return NDIS_STATUS_FAILURE;
    }
        
    
}

VOID P2pAutoReconnectPerstClient(
    IN PMP_ADAPTER pAd, 
    IN PUCHAR       Addr,
    IN ULONG        CounterAutoReconnectForP2pClient)
{
#if 0
    UCHAR perstindex;

    RT_P2P_PERSISTENT_ENTRY *pEntry;
    RT_P2P_CLIENT_ENTRY *pClient;
    PMP_PORT      pPort;
    ULONG       FrameLen;
    BOOLEAN     brc = FALSE;
    UCHAR P2pTabIdx;

    perstindex = P2pPerstTabSearch(pAd, Addr);

    P2pTabIdx = P2pGroupTabSearch(pAd, Addr);

    DBGPRINT(RT_DEBUG_TRACE, ("P2pConnectPerstClient : perstindex = %d, P2pTabIdx = %d Addr = %x %x %x %x %x %x\n"
        , perstindex, P2pTabIdx, Addr[0], Addr[1], Addr[2], Addr[3], Addr[4], Addr[5]));

    if(CounterAutoReconnectForP2pClient >= 9)
    {
        // auto reconnect fail so change the original state
        pAd->pP2pCtrll->P2PTable.Client[P2pTabIdx].P2pClientState = P2PSTATE_DISCOVERY;
        return;
    }
    
    pPort = pAd->PortList[pPort->P2PCfg.PortNumber];
    // Get one index that is valid to use during invitation procedure.
    if (perstindex < MAX_P2P_TABLE_SIZE)    
    {
        // those who can save to persistent table must also support persistent . So can use invite procedure now.
        pEntry = &pAd->pP2pCtrll->P2PTable.PerstEntry[perstindex];
        if (P2pTabIdx < MAX_P2P_GROUP_SIZE)
        {
            pPort->P2PCfg.PhraseKeyLen = (UCHAR)pEntry->Profile.KeyLength;
            PlatformMoveMemory(pPort->P2PCfg.PhraseKey, pEntry->Profile.Key, pEntry->Profile.KeyLength);
            pClient = &pAd->pP2pCtrll->P2PTable.Client[P2pTabIdx];

             if((INFRA_ON(pAd->PortList[PORT_0]) || ADHOC_ON(pAd->PortList[PORT_0])) && (pPort->Channel != pClient->OpChannel)) // only accect when no INFRA or no ADHOC or Port0's channel is the same with Go channel
                                return;
             
            PlatformZeroMemory(pPort->P2PCfg.SSID, 32);
            PlatformMoveMemory(pPort->P2PCfg.SSID, pEntry->Profile.SSID.Ssid, pEntry->Profile.SSID.SsidLength);
            pPort->P2PCfg.SSIDLen = (UCHAR)pEntry->Profile.SSID.SsidLength;
            PlatformZeroMemory(pPort->PortCfg.Ssid, 32);
            PlatformMoveMemory(pPort->PortCfg.Ssid, pEntry->Profile.SSID.Ssid, pEntry->Profile.SSID.SsidLength);
            pPort->PortCfg.SsidLen = (UCHAR)pEntry->Profile.SSID.SsidLength;
            PlatformMoveMemory(&pPort->StaCfg.WscControl.WscProfile.Profile[0], &pEntry->Profile, sizeof(WSC_CREDENTIAL));
            if (pEntry->MyRule == P2P_IS_CLIENT)
            {
                pClient->Rule = P2P_IS_GO;
                pClient->P2pClientState = P2PSTATE_GO_WPS;
                PlatformMoveMemory(pPort->P2PCfg.Bssid, pEntry->Addr, MAC_ADDR_LEN);
            }
            else
            {
                pClient->Rule = P2P_IS_CLIENT;
                pClient->P2pClientState = P2PSTATE_CLIENT_WPS;
                PlatformMoveMemory(pPort->P2PCfg.Bssid, pPort->CurrentAddress, MAC_ADDR_LEN);
            }

            DBGPRINT(RT_DEBUG_TRACE, (" pClient->OpChannel = %d. \n", pClient->OpChannel));
            pPort->Channel = pClient->OpChannel;
            //AsicSwitchChannel(pAd, pPort->Channel, FALSE);
            AsicSwitchChannel(pPort, pPort->Channel, pPort->Channel, BW_20, FALSE);

            P2PMakeInviteReq(pAd, pPort, pEntry->MyRule, P2P_INVITE_FLAG_REINVOKE, pEntry->Addr, pPort->CurrentAddress, &FrameLen);

            if (FrameLen > 0)
            {
                brc = TRUE;
                pClient->P2pClientState = P2PSTATE_SENT_INVITE_REQ;
            }
        }
        else if((P2pTabIdx == P2P_NOT_FOUND) && (pPort->P2PCfg.P2PDiscoProvState == P2P_ENABLE_LISTEN_ONLY))
        {
            // maybe GO setup not yet, continute scan
            pPort->P2PCfg.P2pCounter.CounterAftrScanButton = 0;
            P2pGotoScan(pAd, pPort, P2P_DISCO_TYPE_DEFAULT, 2);
        }
    }
#endif
}

/*  
    ==========================================================================
    Description: 
        Try to connect to a P2P Client.  So Use Client discovery first. CLient Discvoery frame is 
        forwarded by the help of GO.  
        
    Parameters: 
        P2pTabIdx is GO's index in P2P table.
    Note:
         
    ==========================================================================
 */
BOOLEAN P2pClientDiscovery(
    IN PMP_ADAPTER pAd, 
    IN PMP_PORT     pPort,
    IN PUCHAR   Addr,
    IN UCHAR    GoP2pTabIdx)
{
    UCHAR       ClientP2PIndex = P2P_NOT_FOUND;
    UCHAR       i;
    ULONG       TotalFrameLen;
    
    DBGPRINT(RT_DEBUG_TRACE, ("P2pClientDiscovery  GO index is %d\n", GoP2pTabIdx));
    if (GoP2pTabIdx >= MAX_P2P_GROUP_SIZE)
        return FALSE;

    // Search what is the P2P client that I was about to ask it's existence. 
    for (i = 0; i < MAX_P2P_GROUP_SIZE;i++)
    {
        if (pAd->pP2pCtrll->P2PTable.Client[i].P2pClientState == P2PSTATE_CLIENT_DISCO_COMMAND)
            ClientP2PIndex = i;
    }
    DBGPRINT(RT_DEBUG_TRACE, ("P2pClientDiscovery.  ClientP2PIndex= %d\n", ClientP2PIndex));
    if (ClientP2PIndex == P2P_NOT_FOUND)
        return FALSE;

    P2PSendDevDisReq(pAd, pPort, pAd->pP2pCtrll->P2PTable.Client[GoP2pTabIdx].addr, pAd->pP2pCtrll->P2PTable.Client[GoP2pTabIdx].addr, pAd->pP2pCtrll->P2PTable.Client[ClientP2PIndex].addr, &TotalFrameLen);
    if (TotalFrameLen > 0)
    {
        DBGPRINT(RT_DEBUG_TRACE, ("P2pClientDiscovery - pAd->pP2pCtrll->P2PTable.Client[%d], ClientP2PIndex = %d\n", GoP2pTabIdx, ClientP2PIndex));
        return TRUE;
    }
    else    
        return FALSE;
    
}

/*  
    ==========================================================================
    Description: 
        Invite can be used for reinvoke persistent entry or just to connect to an P2P client. So 
        Has 2 index for those choices. Only one is used.  
        invitation procedure is  an optional procedure used for the following :
        1. A P2P group owner inviting a P2P Device to become a P2P Client in its P2P Group.
        2. A P2P client invitin another P2P Device to join the P2P Group of which the P2P Client is a member 
        because it wished to use some service of the P2P Device.
        3. Requesting to invoke a Persistent P2P Group for which both P2P Devices have previouslt been provisioned.
        
    Parameters: 
    Note:
         
    ==========================================================================
 */
BOOLEAN P2pInviteAsRule(
    IN PMP_ADAPTER pAd, 
    IN PMP_PORT     pPort,
    IN UCHAR        MyRule,
    IN UCHAR        P2pTabIdx)
{
    RT_P2P_CLIENT_ENTRY *pClient;
//    PMP_PORT      pPort;
    ULONG       FrameLen;

    // Reflash this flag on the next Invitation or DPID setting.
    if ((MyRule == P2P_IS_GO) || (MyRule == P2P_IS_CLIENT))
        pPort->P2PCfg.SigmaQueryStatus.bInvitationSucced = FALSE;

    pPort = pAd->PortList[pPort->P2PCfg.PortNumber];
    DBGPRINT(RT_DEBUG_TRACE, ("P2pInviteAsRule MyRule = %d. P2pTabIdx= %d \n",  MyRule, P2pTabIdx));
    pClient = &pAd->pP2pCtrll->P2PTable.Client[P2pTabIdx];
    P2PMakeInviteReq(pAd, pPort, MyRule, 0, pClient->addr, pPort->P2PCfg.Bssid/*pPort->CurrentAddress*/, &FrameLen);

    DBGPRINT(RT_DEBUG_TRACE, ("P2pInviteAsRule FrameLen = %d.  GroupOpChannel = %d.\n", FrameLen, pPort->P2PCfg.GroupOpChannel));
    if (FrameLen > 0)
        return TRUE;

    return FALSE;
}

/*  
    ==========================================================================
    Description: 
        Invite can be used for reinvoke persistent entry or just to connect to an P2P client. So 
        Has 2 index for those choices. Only one is used.  
        invitation procedure is  an optional procedure used for the following :
        1. A P2P group owner inviting a P2P Device to become a P2P Client in its P2P Group.
        2. A P2P client invitin another P2P Device to join the P2P Group of which the P2P Client is a member 
        because it wished to use some service of the P2P Device.
        3. Requesting to invoke a Persistent P2P Group for which both P2P Devices have previouslt been provisioned.
        
    Parameters: 
    Note:
         
    ==========================================================================
 */
BOOLEAN P2pInvite(
    IN PMP_ADAPTER pAd, 
    IN PMP_PORT     pPort,
    IN PUCHAR   Addr,
    IN UCHAR        PersistentTabIdx, 
    IN UCHAR        P2pTabIdx)
{
    RT_P2P_PERSISTENT_ENTRY *pEntry;
    RT_P2P_CLIENT_ENTRY *pClient;
//    PMP_PORT      pPort;
    ULONG       FrameLen;
    BOOLEAN     brc = FALSE;
    
//    pPort = pAd->PortList[pPort->P2PCfg.PortNumber];
    // Get one index that is valid to use during invitation procedure.
    if (PersistentTabIdx < MAX_P2P_TABLE_SIZE)  
    {
        // those who can save to persistent table must also support persistent . So can use invite procedure now.
        pEntry = &pAd->pP2pCtrll->P2PTable.PerstEntry[PersistentTabIdx];
        if (P2pTabIdx < MAX_P2P_GROUP_SIZE)
        {
            pPort->P2PCfg.PhraseKeyLen = (UCHAR)pEntry->Profile.KeyLength;
            PlatformMoveMemory(pPort->P2PCfg.PhraseKey, pEntry->Profile.Key, pEntry->Profile.KeyLength);
            pClient = &pAd->pP2pCtrll->P2PTable.Client[P2pTabIdx];
            PlatformZeroMemory(pPort->P2PCfg.SSID,32);
            PlatformMoveMemory(pPort->P2PCfg.SSID, pEntry->Profile.SSID.Ssid, pEntry->Profile.SSID.SsidLength);
            pPort->P2PCfg.SSIDLen = (UCHAR)pEntry->Profile.SSID.SsidLength;
            PlatformZeroMemory(pPort->PortCfg.Ssid,32);
            PlatformMoveMemory(pPort->PortCfg.Ssid, pEntry->Profile.SSID.Ssid,  pEntry->Profile.SSID.SsidLength);
            pPort->PortCfg.SsidLen = (UCHAR)pEntry->Profile.SSID.SsidLength;
            PlatformMoveMemory(&pPort->StaCfg.WscControl.WscProfile.Profile[0], &pEntry->Profile, sizeof(WSC_CREDENTIAL));
            if (pEntry->MyRule == P2P_IS_CLIENT)
            {
                pClient->Rule = P2P_IS_GO;
                pClient->P2pClientState = P2PSTATE_GO_WPS;
                PlatformMoveMemory(pPort->P2PCfg.Bssid, pEntry->Addr, MAC_ADDR_LEN);
            }
            else
            {
                pClient->Rule = P2P_IS_CLIENT;
                pClient->P2pClientState = P2PSTATE_CLIENT_WPS;
                PlatformMoveMemory(pPort->P2PCfg.Bssid, pPort->CurrentAddress, MAC_ADDR_LEN);
            }
            P2PMakeInviteReq(pAd, pPort, pEntry->MyRule, P2P_INVITE_FLAG_REINVOKE, pEntry->Addr, pPort->CurrentAddress, &FrameLen);
            DBGPRINT(RT_DEBUG_TRACE, ("p2p send invite with REINVOKE"));
            if (FrameLen > 0)
            {
                brc = TRUE;
                pClient->P2pClientState = P2PSTATE_SENT_INVITE_REQ;
            }
        }
    }
    else if (P2pTabIdx < MAX_P2P_GROUP_SIZE)
    {
        if ((pAd->pP2pCtrll->P2PTable.Client[P2pTabIdx].DevCapability & DEVCAP_INVITE) == DEVCAP_INVITE)
        {
            if (pPort->PortSubtype == PORTSUBTYPE_P2PGO)
            {
                brc = P2pInviteAsRule(pAd, pPort, P2P_IS_GO, P2pTabIdx);
            }
            else if (IS_P2P_CLIENT_OP(pPort))
                brc = P2pInviteAsRule(pAd, pPort, P2P_IS_CLIENT, P2pTabIdx);
                
        }
        else
            DBGPRINT(RT_DEBUG_TRACE, ("Peer doesn't support Invite. DevCapability = %x \n", pAd->pP2pCtrll->P2PTable.Client[P2pTabIdx].DevCapability));
    }
    else
        DBGPRINT(RT_DEBUG_TRACE, ("invalid P2pInvite command. %d %d \n",  PersistentTabIdx, P2pTabIdx));

    return  brc;
}


/*  
    ==========================================================================
    Description: 
        Start Group Formation Process.   will send out Go Negociation Request frame.
        
    Parameters: 
    Note:
         
    ==========================================================================
 */
BOOLEAN P2pStartGroupForm(
    IN PMP_ADAPTER pAd, 
    IN PMP_PORT     pPort,
    IN PUCHAR   Addr,
    IN UCHAR        idx)
{
    NDIS_STATUS   NStatus;
    PUCHAR        pOutBuffer = NULL;
    ULONG         FrameLen = 0;
    
    // Insert this candidate to p2p client table.
    if (idx < MAX_P2P_GROUP_SIZE)
    {
        DBGPRINT(RT_DEBUG_TRACE,("P2pStartGroupForm. bWaitGoNegReq = %d. \n", pPort->P2PCfg.SigmaSetting.bWaitGoNegReq));
        if ((pAd->pP2pCtrll->P2PTable.Client[idx].P2pClientState == P2PSTATE_CONNECT_COMMAND)|| (pAd->pP2pCtrll->P2PTable.Client[idx].P2pClientState == P2PSTATE_DISCOVERY))
        {
            // Reset Scan Counter to Zero . So Won't do Scan too sooon because group forming is started.
            pPort->P2PCfg.P2pCounter.Counter100ms = 0;
            if (pPort->CommonCfg.P2pControl.field.ExtendListen != 0)
            {
                pPort->P2PCfg.ExtListenInterval = P2P_EXT_LISTEN_INTERVAL;
                pPort->P2PCfg.ExtListenPeriod = P2P_EXT_LISTEN_PERIOD;
                DBGPRINT(RT_DEBUG_TRACE,("P2p extended listen is enabled.  .  \n"));
            } 

            if (pPort->P2PCfg.SigmaSetting.bWaitGoNegReq == FALSE)
            {
                pPort->P2PCfg.P2PConnectState = P2P_ANY_IN_FORMATION_AS_CLIENT;
                DBGPRINT(RT_DEBUG_TRACE,("P2p Start GroupForm .  \n"));
                // allocate and send out ProbeRsp frame
                NStatus = MlmeAllocateMemory(pAd, (PVOID)&pOutBuffer);  //Get an unused nonpaged memory
                if (NStatus != NDIS_STATUS_SUCCESS)
                    return FALSE;
                pAd->pP2pCtrll->P2PTable.Client[idx].P2pClientState = P2PSTATE_SENT_GO_NEG_REQ;
                pAd->pP2pCtrll->P2PTable.Client[idx].StateCount = 0;

                // Set as Client temporarily. Later when I get the GO Rsp, will update this based on correct Intent.
                P2PMakeGoNegoReq(pAd, pPort, idx, Addr, pOutBuffer, &FrameLen);
                NdisCommonMiniportMMRequest(pAd, pOutBuffer, FrameLen);
                //MlmeFreeMemory(pAd, pOutBuffer);
                DBGPRINT(RT_DEBUG_TRACE, (" P2P - Make GO Negociation Req FrameLen  = %d. \n", FrameLen));  
                P2pStopScan(pAd, pPort);
            }   
            // Once dwell in a listen and start group formation process, should stop scan right away.   
            return TRUE;
        }
    }

    return FALSE;
}

BOOLEAN P2pGoPeerAuthAtAuthRspIdleAction(
    IN PMP_ADAPTER pAd, 
    IN PUCHAR       Addr2)
{
//  UCHAR   index;
//  RT_P2P_CLIENT_ENTRY *pP2pEntry = NULL;
    BOOLEAN     Cancelled;
    PMP_PORT    pPort  = pAd->PortList[FXXK_PORT_0];
    // a P2P tries to connect, restart the P2pStopGoTimer. 
    PlatformCancelTimer(&pPort->P2PCfg.P2pStopGoTimer, &Cancelled);
    if (IS_P2P_NOT_AUTOGO(pPort))
        PlatformSetTimer(pPort, &pPort->P2PCfg.P2pStopGoTimer, P2P_CHECK_GO_TIMER);
    DBGPRINT(RT_DEBUG_TRACE,("AUTH_RSP -   P2pGoPeerAuthAtAuthRspIdleAction\n"));
        
    return TRUE;
}

/*  
    ==========================================================================
    Description: 
        Check if the addr is in my MAC wish list if any.
        
    Parameters: 
    Note:
         
    ==========================================================================
 */
BOOLEAN P2PDeviceMatch(
    IN PMP_ADAPTER pAd, 
    IN PUCHAR   Addr,
    IN PUCHAR   DeviceName,
    IN ULONG        DeviceNameLen)
{
    // TODO ! now return TRUE first. 
    return TRUE;
}

/*  
    ==========================================================================
    Description: 
        fill the event content for P2P event : P2PEVENT_P2PGROUP_TOPOLOGY_UPDATE
        
    Parameters: 
    Note:
         
    ==========================================================================
 */
VOID P2pFillTopologyContent(
    IN  PMP_ADAPTER   pAd) 
{
#if 0
    UCHAR   i, k, index ;
    POID_P2PENTRY   pP2pEntry;
    PMAC_TABLE_ENTRY pMacEntry = NULL;
    PMAC_TABLE_ENTRY pNextMacEntry = NULL; 
    PQUEUE_HEADER pHeader;
   
    PMP_PORT    pPort = pAd->PortList[ pPort->P2PCfg.PortNumber];

    pPort->P2PCfg.P2PQueryStatusOid.P2PStateContext.P2PGroup.NumberOfPeer = 0;
    index = 0;
    pP2pEntry = &pPort->P2PCfg.P2PQueryStatusOid.P2PStateContext.P2PGroup.P2PEntry[index];
    
    PlatformMoveMemory(pP2pEntry->MacAddr, pAd->PortList[pPort->P2PCfg.PortNumber]->CurrentAddress, MAC_ADDR_LEN);
    pP2pEntry->ThisPeerIp[0] = (pPort->P2PCfg.MyIp>>24)&0xff;
    pP2pEntry->ThisPeerIp[1] = (pPort->P2PCfg.MyIp>>16)&0xff;
    pP2pEntry->ThisPeerIp[2] = (pPort->P2PCfg.MyIp>>8)&0xff;
    pP2pEntry->ThisPeerIp[3] = (pPort->P2PCfg.MyIp)&0xff;

    PlatformMoveMemory(pP2pEntry->DeviceName, pPort->P2PCfg.DeviceName, P2P_DEVICE_NAME_LEN);
    PlatformMoveMemory(pP2pEntry->PrimaryDeviceType, pPort->StaCfg.WscControl.CustomizeDevInfo.PriDeviceType, P2P_DEVICE_TYPE_LEN);
    DBGPRINT(RT_DEBUG_TRACE, ("PrimaryDeviceType  = %x, %x   \n", *(PULONG)&pP2pEntry->PrimaryDeviceType[0],  *(PULONG)&pP2pEntry->PrimaryDeviceType[4]));      

    pP2pEntry->EntryRule = P2P_IS_CLIENT;
    pP2pEntry->EntryStatus = 0;
    pP2pEntry->DeviceNameLen = pPort->P2PCfg.DeviceNameLen;
    pPort->P2PCfg.P2PQueryStatusOid.P2PStateContext.P2PGroup.NumberOfPeer = 1;
    // case 1: I am GO.
    if ((IS_P2P_GO_OP(pPort)) || IS_P2P_GO_WPA2PSKING(pPort)|| (pPort->P2PCfg.P2PConnectState == P2P_ANY_IN_FORMATION_AS_GO) || IS_P2P_AUTOGO(pPort))
    {
        index = 1;
        pP2pEntry->EntryRule = P2P_IS_GO;
        pP2pEntry->EntryStatus |= ENTRYSTATUS_GOREADY;
        DBGPRINT(RT_DEBUG_TRACE, ("My Rule  = GO.   \n"));      
        // When I am GO. I already know all my Client in my group. just serach the P2pClientState >= P2PSTATE_CLIENT_OPERATING
        pHeader = &pPort->MacTab.MacTabList;
        pNextMacEntry = (PMAC_TABLE_ENTRY)(pHeader->Head);
        while (pNextMacEntry != NULL)
        {
            pMacEntry = (PMAC_TABLE_ENTRY)pNextMacEntry;
            
            if(pMacEntry == NULL)
            {
                break;
            }
            
            if (pMacEntry->P2pInfo.P2pClientState >= P2PSTATE_CLIENT_OPERATING)
            {
                pPort->P2PCfg.P2PQueryStatusOid.P2PStateContext.P2PGroup.NumberOfPeer++;
                pP2pEntry = &pPort->P2PCfg.P2PQueryStatusOid.P2PStateContext.P2PGroup.P2PEntry[index];
                pP2pEntry->EntryRule = P2P_IS_CLIENT;
                pP2pEntry->EntryStatus = 0;
                if ((pMacEntry->P2pInfo.P2pClientState == P2PSTATE_NONP2P_PSK)
                    || (pMacEntry->P2pInfo.P2pClientState == P2PSTATE_NONP2P_WPS))
                    pP2pEntry->EntryStatus |= ENTRYSTATUS_LEGACYSTATION;

                if (pMacEntry->PsMode == PWR_SAVE)
                    pP2pEntry->EntryStatus |= ENTRYSTATUS_PSM;
                if (READ_PHY_CFG_MODE(pAd, &pMacEntry->TxPhyCfg) >= MODE_HTMIX)
                    pP2pEntry->EntryStatus |= ENTRYSTATUS_11N;
                pP2pEntry->Dbm = pMacEntry->LastRssi;
                PlatformMoveMemory(pP2pEntry->MacAddr, pMacEntry->Addr, MAC_ADDR_LEN);
                PlatformMoveMemory(pP2pEntry->PrimaryDeviceType, pMacEntry->P2pInfo.PrimaryDevType, P2P_DEVICE_TYPE_LEN);
                pP2pEntry->ThisPeerIp[0] = 0;
                PlatformMoveMemory(pP2pEntry->DeviceName, pMacEntry->P2pInfo.DeviceName, P2P_DEVICE_NAME_LEN);
                pP2pEntry->DeviceNameLen = pMacEntry->P2pInfo.DeviceNameLen;
                index ++;
            }
            pNextMacEntry = pNextMacEntry->Next;   
            pMacEntry = NULL;
        }
    }
    // case 2 : I am Client
    else if (IS_P2P_CLIENT_OP(pPort))
    {
        pPort->P2PCfg.P2PQueryStatusOid.P2PStateContext.P2PGroup.NumberOfPeer = 0;
        // START FROM 0. 
        index = 0;
        DBGPRINT(RT_DEBUG_TRACE, ("My Rule  = Client.   \n"));          
        for (i = 0;i < MAX_P2P_GROUP_SIZE;i++)
        {
            //When I am a P2P Client and want to tell v4 GUI my topology. I need to Find My GO first. Then from my P2P table, find all the P2P client
            // in my P2P group. 
            if ((PlatformEqualMemory(pPort->P2PCfg.Bssid, pAd->pP2pCtrll->P2PTable.Client[i].bssid, MAC_ADDR_LEN))
                || (PlatformEqualMemory(pPort->P2PCfg.Bssid, pAd->pP2pCtrll->P2PTable.Client[i].addr, MAC_ADDR_LEN)))
            {
                DBGPRINT(RT_DEBUG_TRACE, ("Find my GO %d \n", i));          
                pPort->P2PCfg.P2PQueryStatusOid.P2PStateContext.P2PGroup.NumberOfPeer++;
                pP2pEntry = &pPort->P2PCfg.P2PQueryStatusOid.P2PStateContext.P2PGroup.P2PEntry[index];
                pP2pEntry->EntryRule = P2P_IS_GO;
                PlatformMoveMemory(pP2pEntry->MacAddr, pAd->pP2pCtrll->P2PTable.Client[i].addr, MAC_ADDR_LEN);
                PlatformMoveMemory(pP2pEntry->PrimaryDeviceType, pAd->pP2pCtrll->P2PTable.Client[i].PrimaryDevType, P2P_DEVICE_TYPE_LEN);
                pP2pEntry->ThisPeerIp[0] = (pAd->pP2pCtrll->P2PTable.Client[i].Peerip >> 24) &0xff;
                pP2pEntry->ThisPeerIp[1] = (pAd->pP2pCtrll->P2PTable.Client[i].Peerip >> 16) &0xff;
                pP2pEntry->ThisPeerIp[2] = (pAd->pP2pCtrll->P2PTable.Client[i].Peerip >> 8) &0xff;
                pP2pEntry->ThisPeerIp[3] = (pAd->pP2pCtrll->P2PTable.Client[i].Peerip) &0xff;
                PlatformMoveMemory(pP2pEntry->DeviceName, pAd->pP2pCtrll->P2PTable.Client[i].DeviceName, 32);
                pP2pEntry->DeviceNameLen = pAd->pP2pCtrll->P2PTable.Client[i].DeviceNameLen;
                pPort->P2PCfg.P2PQueryStatusOid.P2PStateContext.P2PGroup.NumberOfPeer = 1;
                index ++;
                k = 0;
                while(k < (MAX_P2P_GROUP_SIZE - i - 1))
                {
                    if (pAd->pP2pCtrll->P2PTable.Client[i + k + 1].P2pClientState == P2PSTATE_DISCOVERY_CLIENT)
                    {
                        k ++;
                        PlatformMoveMemory(pPort->P2PCfg.P2PQueryStatusOid.P2PStateContext.P2PGroup.P2PEntry[index].MacAddr, pAd->pP2pCtrll->P2PTable.Client[i+k].addr, MAC_ADDR_LEN);
                        PlatformMoveMemory(pPort->P2PCfg.P2PQueryStatusOid.P2PStateContext.P2PGroup.P2PEntry[index].DeviceName, pAd->pP2pCtrll->P2PTable.Client[i+k].DeviceName, P2P_DEVICE_NAME_LEN);
                        PlatformMoveMemory(pPort->P2PCfg.P2PQueryStatusOid.P2PStateContext.P2PGroup.P2PEntry[index].PrimaryDeviceType, pAd->pP2pCtrll->P2PTable.Client[i+k].PrimaryDevType, P2P_DEVICE_TYPE_LEN);
                        pPort->P2PCfg.P2PQueryStatusOid.P2PStateContext.P2PGroup.P2PEntry[index].DeviceNameLen = pAd->pP2pCtrll->P2PTable.Client[i+k].DeviceNameLen;
                        pPort->P2PCfg.P2PQueryStatusOid.P2PStateContext.P2PGroup.P2PEntry[index].EntryRule = P2P_IS_CLIENT;
                        pPort->P2PCfg.P2PQueryStatusOid.P2PStateContext.P2PGroup.NumberOfPeer++;
                        index++;
                    }
                    else
                        break;
                };
                // Already find my group. this is topology update, not discovery update. stop here.
                break;
            }
        }
        pP2pEntry = &pPort->P2PCfg.P2PQueryStatusOid.P2PStateContext.P2PGroup.P2PEntry[index];
        
        PlatformMoveMemory(pP2pEntry->MacAddr, pAd->PortList[pPort->P2PCfg.PortNumber]->CurrentAddress, MAC_ADDR_LEN);
        pP2pEntry->ThisPeerIp[0] = (pPort->P2PCfg.MyIp>>24)&0xff;
        pP2pEntry->ThisPeerIp[1] = (pPort->P2PCfg.MyIp>>16)&0xff;
        pP2pEntry->ThisPeerIp[2] = (pPort->P2PCfg.MyIp>>8)&0xff;
        pP2pEntry->ThisPeerIp[3] = (pPort->P2PCfg.MyIp)&0xff;
         
        PlatformMoveMemory(pP2pEntry->DeviceName, pPort->P2PCfg.DeviceName, P2P_DEVICE_NAME_LEN);
        PlatformMoveMemory(pP2pEntry->PrimaryDeviceType, pPort->StaCfg.WscControl.CustomizeDevInfo.PriDeviceType, P2P_DEVICE_TYPE_LEN);
        DBGPRINT(RT_DEBUG_TRACE, ("PrimaryDeviceType  = %x, %x   \n", *(PULONG)&pP2pEntry->PrimaryDeviceType[0],  *(PULONG)&pP2pEntry->PrimaryDeviceType[4]));      

        pP2pEntry->EntryRule = P2P_IS_CLIENT;
        pP2pEntry->EntryStatus = 0;
        // After discuss with GUI team.  when in concurrent mode, our softap host is always enabled.

        pP2pEntry->DeviceNameLen = pPort->P2PCfg.DeviceNameLen;
        pPort->P2PCfg.P2PQueryStatusOid.P2PStateContext.P2PGroup.NumberOfPeer++;
    }
    // case 3: not connected. Only indicate me.
    else
    {
        // Already add myself in the case beginning.
    }
    DBGPRINT(RT_DEBUG_TRACE, ("P2PTable. Number = %d.  Update topology \n", pPort->P2PCfg.P2PQueryStatusOid.P2PStateContext.P2PGroup.NumberOfPeer));          
#endif
}

/*  
    ==========================================================================
    Description: 
        fill the event content for P2P event : P2PEVENT_STATUS_UPDATE_DISCOVERY
        
    Parameters: 
    Note:
         
    ==========================================================================
 */
VOID P2pFillDiscoveryContent(
    IN  PMP_ADAPTER   pAd) 
{
#if 0
    UCHAR   i, k, index, tempuse;
    OID_DISCOVERYENTRY  *pDiscovery;
    for (i = 0;i < MAX_P2P_GROUP_SIZE;)
    {
        pPort->P2PCfg.P2PQueryStatusOid.P2PStateContext.Discovery.DiscoveryEntry[i].NoOfThisGroup = 0;
        // Discvoery only report "un-connected" device. Once it's connected, should put this to Topology update.
        if ((pAd->pP2pCtrll->P2PTable.Client[i].P2pClientState > P2PSTATE_NONE) 
            && (!IS_P2P_PEER_CLIENT_OP(&pAd->pP2pCtrll->P2PTable.Client[i]))
            && (!IS_P2P_PEER_GO_OP(&pAd->pP2pCtrll->P2PTable.Client[i]))
            && (pPort->P2PCfg.P2PQueryStatusOid.P2PStateContext.Discovery.TotalNumber < (MAX_P2P_GROUP_SIZE -1)))
        {
            if (pAd->pP2pCtrll->P2PTable.Client[i].MyGOIndex < MAX_P2P_GROUP_SIZE)
            {
                // check if this peer already put to Topology report.  don't need to add in discovery list.
                if (PlatformEqualMemory(pAd->pP2pCtrll->P2PTable.Client[pAd->pP2pCtrll->P2PTable.Client[i].MyGOIndex].bssid, pPort->P2PCfg.Bssid, MAC_ADDR_LEN))
                {
                    i++;
                    continue;                           
                }
            }
            tempuse = pPort->P2PCfg.P2PQueryStatusOid.P2PStateContext.Discovery.TotalNumber;
            pDiscovery = &pPort->P2PCfg.P2PQueryStatusOid.P2PStateContext.Discovery.DiscoveryEntry[tempuse];
            PlatformMoveMemory(pDiscovery->MacAddr, pAd->pP2pCtrll->P2PTable.Client[i].addr, MAC_ADDR_LEN);
            PlatformMoveMemory(pDiscovery->PrimaryDeviceType, pAd->pP2pCtrll->P2PTable.Client[i].PrimaryDevType, P2P_DEVICE_TYPE_LEN);
            PlatformMoveMemory(pPort->P2PCfg.P2PQueryStatusOid.P2PStateContext.Discovery.DiscoveryEntry[tempuse].DeviceName, pAd->pP2pCtrll->P2PTable.Client[i].DeviceName, 32);
            pDiscovery->ConfigMethod = pAd->pP2pCtrll->P2PTable.Client[i].ConfigMethod;

            pDiscovery->DeviceNameLen = pAd->pP2pCtrll->P2PTable.Client[i].DeviceNameLen;
            pDiscovery->P2PFlags = pAd->pP2pCtrll->P2PTable.Client[i].P2pFlag;
            pDiscovery->NoOfThisGroup = 1;
            pDiscovery->MyRule = P2P_IS_CLIENT;
            pPort->P2PCfg.P2PQueryStatusOid.P2PStateContext.Discovery.TotalNumber++;
            // following the Discovery _ GO are Discovery_Client that is in the same P2P group. So parse next here.
            if ((pAd->pP2pCtrll->P2PTable.Client[i].Rule == P2P_IS_GO) || (pAd->pP2pCtrll->P2PTable.Client[i].P2pClientState == P2PSTATE_DISCOVERY_GO))
            {
                pDiscovery->MyRule = P2P_IS_GO;
                k = 0;
                while(k < (MAX_P2P_GROUP_SIZE - i - 1))
                {
                    if (pAd->pP2pCtrll->P2PTable.Client[i + k + 1].P2pClientState == P2PSTATE_DISCOVERY_CLIENT)
                    {
                        k ++;
                        PlatformMoveMemory(pPort->P2PCfg.P2PQueryStatusOid.P2PStateContext.Discovery.DiscoveryEntry[tempuse + k].MacAddr, pAd->pP2pCtrll->P2PTable.Client[i+k].addr, MAC_ADDR_LEN);
                        PlatformMoveMemory(pPort->P2PCfg.P2PQueryStatusOid.P2PStateContext.Discovery.DiscoveryEntry[tempuse + k].DeviceName, pAd->pP2pCtrll->P2PTable.Client[i+k].DeviceName, 32);
                        pPort->P2PCfg.P2PQueryStatusOid.P2PStateContext.Discovery.DiscoveryEntry[tempuse + k].DeviceNameLen = pAd->pP2pCtrll->P2PTable.Client[i+k].DeviceNameLen;
                        PlatformMoveMemory(pPort->P2PCfg.P2PQueryStatusOid.P2PStateContext.Discovery.DiscoveryEntry[tempuse + k].PrimaryDeviceType, pAd->pP2pCtrll->P2PTable.Client[i+k].PrimaryDevType, P2P_DEVICE_TYPE_LEN);       
                        // Increase GO's "number of this group" parameter.
                        pPort->P2PCfg.P2PQueryStatusOid.P2PStateContext.Discovery.DiscoveryEntry[tempuse].NoOfThisGroup++;
                        pPort->P2PCfg.P2PQueryStatusOid.P2PStateContext.Discovery.DiscoveryEntry[tempuse + k].MyRule = P2P_IS_CLIENT;
                        pPort->P2PCfg.P2PQueryStatusOid.P2PStateContext.Discovery.TotalNumber++;
                    }
                    else
                        break;
                };
                i += k;
                // Update All client's "number of this group" to the same value.
                for (index = 0; index < k; index++)
                {
                    pPort->P2PCfg.P2PQueryStatusOid.P2PStateContext.Discovery.DiscoveryEntry[tempuse + index].NoOfThisGroup = k + 1;
                }
            }
        }
        i++;
    }
    #endif
}

/*  
    ==========================================================================
    Description: 
        fill the event content for P2P event : P2PEVENT_ERROR_CODE
        
    Parameters: 
    Note:
         
    ==========================================================================
 */
VOID P2pFillErrCodeContent(
    IN  PMP_ADAPTER   pAd,
    IN PMP_PORT     pPort) 
{
    pPort->P2PCfg.P2PQueryStatusOid.P2PStateContext.ErrorCode.ErrorStatusCode = pPort->P2PCfg.GroupFormErrCode;
    
    switch(pPort->P2PCfg.GroupFormErrCode)
    {
        case P2P_ERRCODE_CHANNELDISMATCH:
            pPort->P2PCfg.P2PQueryStatusOid.P2PStateContext.ErrorCode.PeerChannel = pPort->P2PCfg.GroupOpChannel;
            break;
        case P2P_ERRCODE_NOCONFIGMETHOD:
            pPort->P2PCfg.P2PQueryStatusOid.P2PStateContext.ErrorCode.PeerConfigMethod = pPort->P2PCfg.GroupOpChannel;
            break;
        case P2P_ERRCODE_CHANNELCONFLICT:
        case P2P_ERRCODE_PHYMODECONFLICT:
            if (INFRA_ON(pAd->PortList[PORT_0]))
                pPort->P2PCfg.P2PQueryStatusOid.P2PStateContext.ErrorCode.PeerChannel = pPort->Channel;
            else
                pPort->P2PCfg.P2PQueryStatusOid.P2PStateContext.ErrorCode.PeerChannel = pPort->P2PCfg.GroupOpChannel;
            break;
        default :
            break;
    }
}

/*  
    ==========================================================================
    Description: 
        call KeSetEvent to inform GUI our state. GUI will do coressponding behavior.
        
    Parameters: 
    Note:
         
    ==========================================================================
 */
BOOLEAN P2pSetEvent(
    IN  PMP_ADAPTER   pAd, 
    IN PMP_PORT     pPort,
    IN    UCHAR     Status) 
{
#if 0
    PWSC_CREDENTIAL pProfile;
    UCHAR           index = 0;
    MAC_TABLE_ENTRY *pEntry = NULL;

    if ((pAd->OpMode != OPMODE_STA) && (pAd->OpMode != OPMODE_STAP2P))
        return TRUE;

    DBGPRINT(RT_DEBUG_TRACE, ("Current Status = %d\n", pPort->P2PCfg.P2PQueryStatusOid.P2PStatus));

    // P2PEVENT_CONNECT_FAIL is the highest priority
    // if call this driver will cancel this connection
    if(Status == P2PEVENT_CONNECT_FAIL)
    {
        pPort->P2PCfg.P2PQueryStatusOid.P2PStatus = P2PEVENT_CONNECT_FAIL;
        if (pPort->P2PCfg.P2pEvent != NULL)
        {
            DBGPRINT(RT_DEBUG_TRACE, ("P2pKeSetEvent %d.  \n", Status));            
            DBGPRINT(RT_DEBUG_TRACE, ("P2pKeSetEvent P2PEVENT_CONNECT_FAIL \n"));           
            KeSetEvent((PKEVENT)pPort->P2PCfg.P2pEvent, 0, FALSE);
            pPort->P2PCfg.P2pEventQueue.LastSetEvent = P2PEVENT_CONNECT_FAIL;
            return TRUE;
        }
    }
    
    if ((pPort->P2PCfg.P2PQueryStatusOid.P2PStatus != P2PEVENT_STATUS_IDLE) && (Status != P2PEVENT_STATUS_IDLE))
    {
        DBGPRINT(RT_DEBUG_TRACE, ("P2pKeSetEvent Invalid State. = %d . return. \n", Status));           
        // If GUI not ready for WaitEvent, we shouldn't SetKeEvent now. 
        // When GUI is ready for receiveing event, he notice use with OID  RT_OID_SET_P2P_EVENT_READY
        return FALSE;
    }

    if (((pPort->P2PCfg.P2PConnectState == P2P_DO_GO_SCAN_BEGIN) || IS_P2P_ENROLLEE(pPort)) 
        && ((Status == P2PEVENT_P2PGROUP_TOPOLOGY_UPDATE)))
    {
        DBGPRINT(RT_DEBUG_TRACE, ("P2pKeSetEvent  Ignore low priority discovery event  when %s. \n", decodeP2PState(pPort->P2PCfg.P2PConnectState)));         
        return TRUE;
    }
    if ((pPort->P2PCfg.P2PConnectState == P2P_DO_WPS_DONE_NEED_SETEVENT)
        && ((Status != P2PEVENT_STATUS_WPS_DONE)))
    {
        DBGPRINT(RT_DEBUG_TRACE, ("P2pKeSetEvent  Ignore other event when %s. \n", decodeP2PState(pPort->P2PCfg.P2PConnectState)));           
        return TRUE;
    }
    // =====================>
    // Prepare Status Report via OID.
    switch(Status)
    {
        case P2PEVENT_CONNECT_COMPLETE:
            pPort->P2PCfg.P2PQueryStatusOid.P2PStatus = P2PEVENT_CONNECT_COMPLETE;
            if (pPort->P2PCfg.P2pEvent != NULL)
            {
                DBGPRINT(RT_DEBUG_TRACE, ("P2pKeSetEvent %d.  \n", Status));            
                DBGPRINT(RT_DEBUG_TRACE, ("P2pKeSetEvent Finish p2p conenction \n"));           
                KeSetEvent((PKEVENT)pPort->P2PCfg.P2pEvent, 0, FALSE);
                pPort->P2PCfg.P2pEventQueue.LastSetEvent = P2PEVENT_CONNECT_COMPLETE;
                pPort->P2PCfg.P2pCounter.CounterAftrSetEvent = 0;
                return TRUE;
            }
            break;
        case P2PEVENT_ERROR_CODE:
            pPort->P2PCfg.P2PQueryStatusOid.P2PStatus = P2PEVENT_ERROR_CODE;
            if (pPort->P2PCfg.P2pEvent != NULL)
            {
                DBGPRINT(RT_DEBUG_TRACE, ("P2pKeSetEvent %d.  \n", Status));            
                DBGPRINT(RT_DEBUG_TRACE, (" P2PEVENT_ERROR_CODE   %s \n", decodeP2PErrCode(pPort->P2PCfg.GroupFormErrCode))); 

                pPort->P2PCfg.P2pCounter.CounterAutoReconnectForP2pClient = 10;
                P2pFillErrCodeContent(pAd, pPort);
                KeSetEvent((PKEVENT)pPort->P2PCfg.P2pEvent, 0, FALSE);
                pPort->P2PCfg.P2pEventQueue.LastSetEvent = P2PEVENT_ERROR_CODE;
                pPort->P2PCfg.P2pCounter.CounterAftrSetEvent = 0;
                PlatformZeroMemory( pPort->P2PCfg.PinCode, 8);
                PlatformZeroMemory( pPort->StaCfg.WscControl.RegData.PIN, 8);
                pPort->StaCfg.WscControl.RegData.PINLen = 0;
                return TRUE;
            }
            break;
        case P2PEVENT_SIGMA_IS_ON:
            pPort->P2PCfg.P2PQueryStatusOid.P2PStatus = P2PEVENT_SIGMA_IS_ON;
            if (pPort->P2PCfg.P2pEvent != NULL)
            {
                DBGPRINT(RT_DEBUG_TRACE, ("P2pKeSetEvent %d.  \n", Status));            
                DBGPRINT(RT_DEBUG_TRACE, ("P2pKeSetEvent SIGMA ON  \n"));           
                KeSetEvent((PKEVENT)pPort->P2PCfg.P2pEvent, 0, FALSE);
                pPort->P2PCfg.P2pEventQueue.LastSetEvent = P2PEVENT_SIGMA_IS_ON;
                pPort->P2PCfg.P2pCounter.CounterAftrSetEvent = 0;
                return TRUE;
            }
            break;
        case P2PEVENT_DISPLAY_PIN:
            if (pPort->P2PCfg.PopUpIndex < MAX_P2P_GROUP_SIZE)
            {
                pPort->P2PCfg.P2PQueryStatusOid.P2PStatus = P2PEVENT_DISPLAY_PIN;
                PlatformMoveMemory(pPort->P2PCfg.P2PQueryStatusOid.P2PStateContext.DisplayPin.MacAddr, pAd->pP2pCtrll->P2PTable.Client[pPort->P2PCfg.PopUpIndex].addr, 6);
                if (pPort->P2PCfg.P2pEvent != NULL)
                {
                    DBGPRINT(RT_DEBUG_TRACE, ("P2pKeSetEvent %d.  \n", Status));            
                    KeSetEvent((PKEVENT)pPort->P2PCfg.P2pEvent, 0, FALSE);
                    pPort->P2PCfg.P2pEventQueue.LastSetEvent = P2PEVENT_DISPLAY_PIN;
                    pPort->P2PCfg.P2pCounter.CounterAftrSetEvent = 0;
                    return TRUE;
                }
            }
            break;
        case P2PEVENT_POPUP_SETTING_WINDOWS:
            //If driver enabled listening, when a P2P peer tries to connect to me, driver need to inform GUI
            // to pop up a setting windows, (PIN, PBC..)
            if ((pPort->P2PCfg.PopUpIndex < MAX_P2P_GROUP_SIZE )
                && (pPort->P2PCfg.LastConfigMode < CONFIG_MODE_SIGMA_P2P_RESET_DEFAULT))
            {
                pPort->P2PCfg.P2PQueryStatusOid.P2PStatus = P2PEVENT_POPUP_SETTING_WINDOWS;
                PlatformMoveMemory(pPort->P2PCfg.P2PQueryStatusOid.P2PStateContext.PopUpParm.GroupId, pAd->pP2pCtrll->P2PTable.Client[pPort->P2PCfg.PopUpIndex].addr, 6);
                PlatformMoveMemory(pPort->P2PCfg.P2PQueryStatusOid.P2PStateContext.PopUpParm.DeviceName,  pAd->pP2pCtrll->P2PTable.Client[pPort->P2PCfg.PopUpIndex].DeviceName, 32);
                pPort->P2PCfg.P2PQueryStatusOid.P2PStateContext.PopUpParm.DeviceNameLen = pAd->pP2pCtrll->P2PTable.Client[pPort->P2PCfg.PopUpIndex].DeviceNameLen;
                pPort->P2PCfg.P2PQueryStatusOid.P2PStateContext.PopUpParm.ConfigMethod = pAd->pP2pCtrll->P2PTable.Client[pPort->P2PCfg.PopUpIndex].ConfigMethod;
                // before pop window, we clear the pin code
                PlatformZeroMemory( pPort->P2PCfg.PinCode, 8);
                PlatformZeroMemory( pPort->StaCfg.WscControl.RegData.PIN, 8);
                pPort->StaCfg.WscControl.RegData.PINLen = 0;
                if (pPort->P2PCfg.P2pEvent != NULL)
                {
                    DBGPRINT(RT_DEBUG_TRACE, ("P2pKeSetEvent %d.  %s \n", Status, decodeConfigMethod(pAd->pP2pCtrll->P2PTable.Client[pPort->P2PCfg.PopUpIndex].ConfigMethod)));          
                    KeSetEvent((PKEVENT)pPort->P2PCfg.P2pEvent, 0, FALSE);
                    pPort->P2PCfg.P2pEventQueue.LastSetEvent = P2PEVENT_POPUP_SETTING_WINDOWS;
                    pPort->P2PCfg.P2pCounter.CounterAftrSetEvent = 0;
                    return TRUE;
                }
            }
            break;

        case P2PEVENT_POPUP_SETTING_WINDOWS_FOR_AUTOGO:
            //If driver enabled listening, when a P2P peer tries to connect to me, driver need to inform GUI for AutoGO
            // to pop up a setting windows, (PIN, PBC..)
            if ((pPort->P2PCfg.PopUpIndex < MAX_P2P_GROUP_SIZE )
                && (pPort->P2PCfg.LastConfigMode < CONFIG_MODE_SIGMA_P2P_RESET_DEFAULT))
            {
                pPort->P2PCfg.P2PQueryStatusOid.P2PStatus = P2PEVENT_POPUP_SETTING_WINDOWS_FOR_AUTOGO;
                PlatformMoveMemory(pPort->P2PCfg.P2PQueryStatusOid.P2PStateContext.PopUpParm.GroupId, pAd->pP2pCtrll->P2PTable.Client[pPort->P2PCfg.PopUpIndex].addr, 6);
                PlatformMoveMemory(pPort->P2PCfg.P2PQueryStatusOid.P2PStateContext.PopUpParm.DeviceName,  pAd->pP2pCtrll->P2PTable.Client[pPort->P2PCfg.PopUpIndex].DeviceName, 32);
                pPort->P2PCfg.P2PQueryStatusOid.P2PStateContext.PopUpParm.DeviceNameLen = pAd->pP2pCtrll->P2PTable.Client[pPort->P2PCfg.PopUpIndex].DeviceNameLen;
                pPort->P2PCfg.P2PQueryStatusOid.P2PStateContext.PopUpParm.ConfigMethod = pAd->pP2pCtrll->P2PTable.Client[pPort->P2PCfg.PopUpIndex].ConfigMethod;
                // before pop window, we clear the pin code
                PlatformZeroMemory( pPort->P2PCfg.PinCode, 8);
                PlatformZeroMemory( pPort->StaCfg.WscControl.RegData.PIN, 8);
                pPort->StaCfg.WscControl.RegData.PINLen = 0;
                if (pPort->P2PCfg.P2pEvent != NULL)
                {
                    DBGPRINT(RT_DEBUG_TRACE, ("P2pKeSetEvent %d.  %s \n", Status, decodeConfigMethod(pAd->pP2pCtrll->P2PTable.Client[pPort->P2PCfg.PopUpIndex].ConfigMethod)));          
                    KeSetEvent((PKEVENT)pPort->P2PCfg.P2pEvent, 0, FALSE);
                    pPort->P2PCfg.P2pEventQueue.LastSetEvent = P2PEVENT_POPUP_SETTING_WINDOWS_FOR_AUTOGO;
                    pPort->P2PCfg.P2pCounter.CounterAftrSetEvent = 0;
                    return TRUE;
                }
            }
            break;

        case P2PEVENT_CONFIRM_ENTER_PIN_FROM_WPS_M2:
                pPort->P2PCfg.P2PQueryStatusOid.P2PStatus = P2PEVENT_CONFIRM_ENTER_PIN_FROM_WPS_M2;
                if (pPort->P2PCfg.P2pEvent != NULL)
                {
                    DBGPRINT(RT_DEBUG_TRACE, ("P2pKeSetEvent %d.  %s \n", Status, decodeConfigMethod(pAd->pP2pCtrll->P2PTable.Client[pPort->P2PCfg.PopUpIndex].ConfigMethod)));

                    pPort->P2PCfg.P2PQueryStatusOid.P2PStateContext.PopUpParm.ConfigMethod = pAd->pP2pCtrll->P2PTable.Client[pPort->P2PCfg.PopUpIndex].ConfigMethod;
                    KeSetEvent((PKEVENT)pPort->P2PCfg.P2pEvent, 0, FALSE);
                    pPort->P2PCfg.P2pEventQueue.LastSetEvent = P2PEVENT_CONFIRM_ENTER_PIN_FROM_WPS_M2;
                    pPort->P2PCfg.P2pCounter.CounterAftrSetEvent = 0;
                    return TRUE;
                }
            break;
            
        case P2PEVENT_STATUS_GO_NEGO_DONE:
            pPort->P2PCfg.P2PQueryStatusOid.P2PStatus = P2PEVENT_STATUS_GO_NEGO_DONE;
            // This is a client event. So Must be Client,
            pPort->P2PCfg.P2PQueryStatusOid.P2PStateContext.GoNegoDone.MyRule = P2P_IS_CLIENT;
            pPort->P2PCfg.P2PQueryStatusOid.P2PStateContext.GoNegoDone.WscMode = pPort->P2PCfg.WscMode;

            // Display PIN Code with 8 digits or 4 digits
            PlatformZeroMemory(pPort->P2PCfg.P2PQueryStatusOid.P2PStateContext.GoNegoDone.PIN, 8);
            if (pPort->P2PCfg.PINLen == 2)    // 4 digits
            {
                pPort->P2PCfg.P2PQueryStatusOid.P2PStateContext.GoNegoDone.PIN[0] = 0xff;
                pPort->P2PCfg.P2PQueryStatusOid.P2PStateContext.GoNegoDone.PIN[1] = 0xff;
                pPort->P2PCfg.P2PQueryStatusOid.P2PStateContext.GoNegoDone.PIN[2] = 0xff;
                pPort->P2PCfg.P2PQueryStatusOid.P2PStateContext.GoNegoDone.PIN[3] = 0xff;
                PlatformMoveMemory(&pPort->P2PCfg.P2PQueryStatusOid.P2PStateContext.GoNegoDone.PIN[4], pPort->P2PCfg.PinCode, 4);
            }
            else    // 8 digits
            {
                PlatformMoveMemory(pPort->P2PCfg.P2PQueryStatusOid.P2PStateContext.GoNegoDone.PIN, pPort->P2PCfg.PinCode, 8);
            }
            pProfile = &pPort->P2PCfg.P2PQueryStatusOid.P2PStateContext.GoNegoDone.WscProfile;
            PlatformMoveMemory(pProfile->SSID.Ssid, pAd->PortList[pPort->P2PCfg.PortNumber]->PortCfg.Ssid, 32);
            PlatformMoveMemory(pProfile->MacAddr, pAd->PortList[pPort->P2PCfg.PortNumber]->PortCfg.Bssid, MAC_ADDR_LEN);
            DBGPRINT(RT_DEBUG_TRACE, ("Connect to WPS AP Bssid = %x %x %x %x %x %x   \n", pProfile->MacAddr[0], pProfile->MacAddr[1],pProfile->MacAddr[2],pProfile->MacAddr[3],pProfile->MacAddr[4],pProfile->MacAddr[5]));         
            DBGPRINT(RT_DEBUG_TRACE, ("WscMode %x.  =    \n", pPort->P2PCfg.WscMode));            

            pProfile->SSID.SsidLength = pAd->PortList[pPort->P2PCfg.PortNumber]->PortCfg.SsidLen;
            if (pPort->P2PCfg.P2pEvent != NULL)
            {
                DBGPRINT(RT_DEBUG_TRACE, ("P2pKeSetEvent %d.  =  Ssidlen = %d  \n", Status, pProfile->SSID.SsidLength));            
                KeSetEvent((PKEVENT)pPort->P2PCfg.P2pEvent, 0, FALSE);
                pPort->P2PCfg.P2pEventQueue.LastSetEvent = P2PEVENT_STATUS_GO_NEGO_DONE;
                pPort->P2PCfg.P2pCounter.CounterAftrSetEvent = 0;
                return TRUE;
            }
            break;
        case P2PEVENT_STATUS_WPS_FAIL:
            pPort->P2PCfg.P2PQueryStatusOid.P2PStatus = P2PEVENT_STATUS_WPS_FAIL;
            pPort->P2PCfg.P2PQueryStatusOid.P2PStateContext.WpsFail.Reason = pPort->StaCfg.WscControl.WscStatus;
            if (pPort->P2PCfg.P2pEvent != NULL)
            {
                DBGPRINT(RT_DEBUG_TRACE, ("P2pKeSetEvent %d. WPS Fail reason=  %x  \n", Status, pPort->StaCfg.WscControl.WscStatus));         
                KeSetEvent((PKEVENT)pPort->P2PCfg.P2pEvent, 0, FALSE);
                pPort->P2PCfg.P2pEventQueue.LastSetEvent = P2PEVENT_STATUS_WPS_FAIL;
                pPort->P2PCfg.P2pCounter.CounterAftrSetEvent = 0;
                return TRUE;
            }
            break;
        case P2PEVENT_DISCONNECT:
            pPort->P2PCfg.P2PQueryStatusOid.P2PStatus = P2PEVENT_DISCONNECT;
            if (pPort->P2PCfg.P2pEvent != NULL)
            {
                DBGPRINT(RT_DEBUG_TRACE, ("P2pKeSetEvent %d. disconnect\n", Status));           
                KeSetEvent((PKEVENT)pPort->P2PCfg.P2pEvent, 0, FALSE);
                pPort->P2PCfg.P2pEventQueue.LastSetEvent = P2PEVENT_DISCONNECT;
                pPort->P2PCfg.P2pCounter.CounterAftrSetEvent = 0;
                return TRUE;
            }
            break;
        case P2PEVENT_GEN_SEUDO_ADHOC_PROFILE:
            pPort->P2PCfg.P2PQueryStatusOid.P2PStatus = P2PEVENT_GEN_SEUDO_ADHOC_PROFILE;
            PlatformMoveMemory(pPort->P2PCfg.P2PQueryStatusOid.P2PStateContext.SeudoAdhoc.uSsid.Ssid, pPort->P2PCfg.SSID, 32);
            pPort->P2PCfg.P2PQueryStatusOid.P2PStateContext.SeudoAdhoc.uSsid.SsidLength = pPort->P2PCfg.SSIDLen;
            pPort->P2PCfg.P2PQueryStatusOid.P2PStateContext.SeudoAdhoc.Channel  = pPort->P2PCfg.GroupOpChannel;
            DBGPRINT(RT_DEBUG_TRACE, ("=SsidLen = 0x%x.   \n", pPort->P2PCfg.SSIDLen));           
            DBGPRINT(RT_DEBUG_TRACE, ("=Ssid = %x. %x %x %x %x %x  \n",pPort->P2PCfg.SSID[0], pPort->P2PCfg.SSID[1],pPort->P2PCfg.SSID[2],pPort->P2PCfg.SSID[3],pPort->P2PCfg.SSID[4],pPort->P2PCfg.SSID[5]));          
            if (pPort->P2PCfg.P2pEvent != NULL)
            {
                DBGPRINT(RT_DEBUG_TRACE, ("P2pKeSetEvent %d : seudo adhoc \n", Status));            
                KeSetEvent((PKEVENT)pPort->P2PCfg.P2pEvent, 0, FALSE);
                pPort->P2PCfg.P2pEventQueue.LastSetEvent = P2PEVENT_GEN_SEUDO_ADHOC_PROFILE;
                pPort->P2PCfg.P2pCounter.CounterAftrSetEvent = 0;
                return TRUE;
            }
            break;
        // this event will tell GUI the list of other P2P device that is in other P2P group.
        case P2PEVENT_STATUS_UPDATE_DISCOVERY:
            {
                pPort->P2PCfg.P2PQueryStatusOid.P2PStatus = P2PEVENT_STATUS_UPDATE_DISCOVERY;
                pPort->P2PCfg.P2PQueryStatusOid.P2PStateContext.Discovery.TotalNumber  = 0;
                P2pFillDiscoveryContent(pAd);
                if (pPort->P2PCfg.P2pEvent != NULL)
                {
                    DBGPRINT(RT_DEBUG_TRACE, ("P2pKeSetEvent %d.  Update discovery num = %d \n", Status, pPort->P2PCfg.P2PQueryStatusOid.P2PStateContext.Discovery.TotalNumber));         
                    KeSetEvent((PKEVENT)pPort->P2PCfg.P2pEvent, 0, FALSE);
                    // discovery must be preceded by topology update event, 
                    pPort->P2PCfg.P2pEventQueue.LastSetEvent = P2PEVENT_P2PGROUP_TOPOLOGY_UPDATE;
                    pPort->P2PCfg.P2pCounter.CounterAftrSetEvent = 0;
                    return TRUE;
                }
            }
            break;
        // this event will tell GUI the list of all P2P device that is in My P2P group.
        case P2PEVENT_P2PGROUP_TOPOLOGY_UPDATE:
            if (IS_P2P_SIGMA_ON(pPort))
            {
                // Following Topology update. update discovery too.
                pPort->P2PCfg.P2pEventQueue.bDiscoveryUpdate = TRUE;
                return TRUE;
            }
            // The P2P Group topology changed.  Notify GUI to redraw the topology.
            pPort->P2PCfg.P2PQueryStatusOid.P2PStatus = P2PEVENT_P2PGROUP_TOPOLOGY_UPDATE;
            P2pFillTopologyContent(pAd);
            DBGPRINT(RT_DEBUG_TRACE, ("P2PTable. Number = %d.  Update topology \n", pPort->P2PCfg.P2PQueryStatusOid.P2PStateContext.P2PGroup.NumberOfPeer));          
            if (pPort->P2PCfg.P2pEvent != NULL)
            {
                DBGPRINT(RT_DEBUG_TRACE, ("P2pKeSetEvent %d. Update topology \n", Status));         
                KeSetEvent((PKEVENT)pPort->P2PCfg.P2pEvent, 0, FALSE);
                pPort->P2PCfg.P2pEventQueue.LastSetEvent = P2PEVENT_P2PGROUP_TOPOLOGY_UPDATE;
                pPort->P2PCfg.P2pCounter.CounterAftrSetEvent = 0;
                // Following Topology update. update discovery too.
                pPort->P2PCfg.P2pEventQueue.bDiscoveryUpdate = TRUE;
                return TRUE;
            }
            break;
            
        case P2PEVENT_STATUS_WPS_DONE:
        case P2PEVENT_DRIVER_INTERNAL_USE:
            pPort->P2PCfg.P2PQueryStatusOid.P2PStatus = P2PEVENT_STATUS_WPS_DONE;

            pPort->P2PCfg.P2PQueryStatusOid.P2PStateContext.WpsDone.bIsPersistent = FALSE;
            // If Sigma auto test tool is used, don't need to tell GUI to record this credential.
            if (IS_PERSISTENT_ON(pAd) && (IS_P2P_SIGMA_OFF(pPort) &&(Status == P2PEVENT_STATUS_WPS_DONE))) // internal use not UI do not need save profile
                pPort->P2PCfg.P2PQueryStatusOid.P2PStateContext.WpsDone.bIsPersistent = TRUE;
            pProfile = &pPort->P2PCfg.P2PQueryStatusOid.P2PStateContext.WpsDone.WpsDoneContent.Profile;
            pPort->P2PCfg.P2PQueryStatusOid.P2PStateContext.WpsDone.WpsDoneContent.bValid = TRUE;
            if ((pPort->P2PCfg.P2PConnectState == P2P_ANY_IN_FORMATION_AS_GO) ||IS_P2P_REGISTRA(pPort) ||IS_P2P_GO_WPA2PSKING(pPort) || IS_P2P_GO_OP(pPort))
            {
                DBGPRINT(RT_DEBUG_TRACE, ("GO \n"));            
                pPort->P2PCfg.P2PQueryStatusOid.P2PStateContext.WpsDone.WpsDoneContent.MyRule = P2P_IS_GO;
            }
            else
            {
                DBGPRINT(RT_DEBUG_TRACE, ("CLIENT \n"));            
                pPort->P2PCfg.P2PQueryStatusOid.P2PStateContext.WpsDone.WpsDoneContent.MyRule = P2P_IS_CLIENT;
            }
            if (pPort->StaCfg.WscControl.WscProfile.Profile[0].SSID.SsidLength > 0)
            {
                PlatformMoveMemory(pProfile->SSID.Ssid, pPort->StaCfg.WscControl.WscProfile.Profile[0].SSID.Ssid, 32);
                pProfile->SSID.SsidLength = pPort->StaCfg.WscControl.WscProfile.Profile[0].SSID.SsidLength;
            }
            else
            {
                DBGPRINT(RT_DEBUG_TRACE, ("Error case ??  Why credential SSID Len is zero .. \n"));         
                PlatformMoveMemory(pProfile->SSID.Ssid, pAd->PortList[pPort->P2PCfg.PortNumber]->PortCfg.Ssid, 32);
                pProfile->SSID.SsidLength = pAd->PortList[pPort->P2PCfg.PortNumber]->PortCfg.SsidLen;
            }
            DBGPRINT(RT_DEBUG_TRACE, ("---->P2P pPort->P2PCfg.SsidLength = %d. [%x %x %x..]\n", pPort->P2PCfg.SSIDLen, 
                pPort->P2PCfg.SSID[0], pPort->P2PCfg.SSID[1], pPort->P2PCfg.SSID[2]));            
            DBGPRINT(RT_DEBUG_TRACE, ("---->P2P Profile->SsidLength = %d. [%x %x %x..]\n", pProfile->SSID.SsidLength, 
                pProfile->SSID.Ssid[0], pProfile->SSID.Ssid[1], pProfile->SSID.Ssid[2]));           

            pProfile->AuthType = 0x20;
            pProfile->EncrType = 8;
            pProfile->KeyIndex = pPort->StaCfg.WscControl.WscProfile.Profile[0].KeyIndex;
            if (Status == P2PEVENT_DRIVER_INTERNAL_USE)
            {
                pProfile->KeyLength = pPort->P2PCfg.PhraseKeyLen;
                PlatformMoveMemory(pProfile->Key, pPort->P2PCfg.PhraseKey, 64);
            }
            else
            {
                pProfile->KeyLength = pPort->StaCfg.WscControl.WscProfile.Profile[0].KeyLength;
                PlatformMoveMemory(pProfile->Key, pPort->StaCfg.WscControl.WscProfile.Profile[0].Key, 64);
            }
            DBGPRINT(RT_DEBUG_TRACE, ("---->P2P Profile->Key[%d]Length = %d. [%x %x  %x.]\n", pProfile->KeyIndex, pProfile->KeyLength, 
                *(PULONG)&pProfile->Key[0], *(PULONG)&pProfile->Key[4], *(PULONG)&pProfile->Key[8]));           
            PlatformMoveMemory(pProfile->MacAddr, pPort->StaCfg.WscControl.WscProfile.Profile[0].MacAddr, MAC_ADDR_LEN);
            DBGPRINT(RT_DEBUG_TRACE, ("---->P2P Profile->MacAddr [%x %x %x %x %x %x..]\n",  
                pProfile->MacAddr[0], pProfile->MacAddr[1], pProfile->MacAddr[2], pProfile->MacAddr[3], pProfile->MacAddr[4], pProfile->MacAddr[5]));           

            if ((pPort->P2PCfg.P2pEvent != NULL))
            {
                DBGPRINT(RT_DEBUG_TRACE, ("P2pKeSetEvent %d . WPS done.\n", Status));           
                KeSetEvent((PKEVENT)pPort->P2PCfg.P2pEvent, 0, FALSE);
                pPort->P2PCfg.P2pEventQueue.bWpsDone = FALSE;             
                pPort->P2PCfg.P2pEventQueue.LastSetEvent = P2PEVENT_STATUS_WPS_DONE;
                pPort->P2PCfg.P2pCounter.CounterAftrSetEvent = 0;
                return TRUE;
            }
        break;
        case P2PEVENT_ENABLE_DHCP_SERVER:
            if (pPort->P2PCfg.P2pEvent != NULL)
            {
                DBGPRINT(RT_DEBUG_TRACE, ("P2pKeSetEvent %d. enable dhcp server. OpMode = %d. \n", Status, pAd->OpMode));           
                pPort->P2PCfg.P2PQueryStatusOid.P2PStatus = P2PEVENT_ENABLE_DHCP_SERVER;
                KeSetEvent((PKEVENT)pPort->P2PCfg.P2pEvent, 0, FALSE);
                pPort->P2PCfg.P2pEventQueue.LastSetEvent = P2PEVENT_ENABLE_DHCP_SERVER;
                pPort->P2PCfg.P2pCounter.CounterAftrSetEvent = 0;
                return TRUE;
            }
            break;
            
        case P2PEVENT_DISABLE_DHCP_SERVER:
            if (pPort->P2PCfg.P2pEvent != NULL)
            {
                DBGPRINT(RT_DEBUG_TRACE, ("P2pKeSetEvent %d. disable dhcp server.  OpMode = %d.\n", Status, pAd->OpMode));          
                pPort->P2PCfg.P2PQueryStatusOid.P2PStatus = P2PEVENT_DISABLE_DHCP_SERVER;
                KeSetEvent((PKEVENT)pPort->P2PCfg.P2pEvent, 0, FALSE);
                pPort->P2PCfg.P2pEventQueue.LastSetEvent = P2PEVENT_DISABLE_DHCP_SERVER;
                pPort->P2PCfg.P2pCounter.CounterAftrSetEvent = 0;
                return TRUE;
            }
            break;
        case P2PEVENT_SWITCH_OPMODE_STAP2P:
            if (pAd->OpMode == OPMODE_STA)
            {
                pPort->P2PCfg.P2PQueryStatusOid.P2PStatus = P2PEVENT_SWITCH_OPMODE_STAP2P;
                if (pPort->P2PCfg.P2pEvent != NULL)
                {
                    DBGPRINT(RT_DEBUG_TRACE, ("P2pKeSetEvent %d. switch to STA+WFD mode\n", Status));           
                    KeSetEvent((PKEVENT)pPort->P2PCfg.P2pEvent, 0, FALSE);
                    pPort->P2PCfg.P2pEventQueue.LastSetEvent = P2PEVENT_SWITCH_OPMODE_STAP2P;
                    pPort->P2PCfg.P2pCounter.CounterAftrSetEvent = 0;
                    return TRUE;
                }
            }
            break;
        case P2PEVENT_SWITCH_OPMODE_STA:
            if (pAd->OpMode == OPMODE_STAP2P)
            {
                pPort->P2PCfg.P2PQueryStatusOid.P2PStatus = P2PEVENT_SWITCH_OPMODE_STA;
                if (pPort->P2PCfg.P2pEvent != NULL)
                {
                    DBGPRINT(RT_DEBUG_TRACE, ("P2pKeSetEvent %d. switch to STA mode\n", Status));           
                    KeSetEvent((PKEVENT)pPort->P2PCfg.P2pEvent, 0, FALSE);
                    pPort->P2PCfg.P2pEventQueue.LastSetEvent = P2PEVENT_SWITCH_OPMODE_STA;
                    pPort->P2PCfg.P2pCounter.CounterAftrSetEvent = 0;
                    return TRUE;
                }
            }
            break;
        case P2PEVENT_DISABLE_ICS:
            if (pAd->OpMode == OPMODE_STAP2P)
            {
                pPort->P2PCfg.P2PQueryStatusOid.P2PStatus = P2PEVENT_DISABLE_ICS;
                if (pPort->P2PCfg.P2pEvent != NULL)
                {
                    DBGPRINT(RT_DEBUG_TRACE, ("P2pKeSetEvent %d. Disable ICS\n", Status));          
                    KeSetEvent((PKEVENT)pPort->P2PCfg.P2pEvent, 0, FALSE);
                    pPort->P2PCfg.P2pEventQueue.LastSetEvent = P2PEVENT_DISABLE_ICS;
                    pPort->P2PCfg.P2pCounter.CounterAftrSetEvent = 0;
                    return TRUE;
                }
            }
            break;
        case P2PEVENT_STATUS_IDLE:
            pPort->P2PCfg.P2PQueryStatusOid.P2PStatus = P2PEVENT_STATUS_IDLE;
            pPort->P2PCfg.P2pCounter.CounterAftrSetEvent = 0;
            break;
        default :
            pPort->P2PCfg.P2PQueryStatusOid.P2PStatus = P2PEVENT_STATUS_IDLE;
            break;
    }
    #endif
    return FALSE;
    
}

/*  
    ==========================================================================
    Description: 
        Set WPS related parameters
        
    Parameters: 
    Note:
         
    ==========================================================================
 */
VOID P2pSetWps(
    IN  PMP_ADAPTER   pAd, 
    IN  PMP_PORT  pPort) 
{             
    BOOLEAN     Cancelled;
    UCHAR ZeroPinCode4[4] = {0, 0, 0, 0};
    UCHAR ZeroPinCode8[8] = {0, 0, 0, 0, 0, 0, 0, 0};
    
    DBGPRINT(RT_DEBUG_TRACE, (" P2pSetWps ==> P2P= %d \n", pPort->P2PCfg.P2PConnectState));

    DBGPRINT(RT_DEBUG_TRACE, ("---->P2P pPort->P2PCfg.SsidLength = %d. [DIRECT%c%c%c..]\n", pPort->P2PCfg.SSIDLen, 
        pPort->P2PCfg.SSID[6], pPort->P2PCfg.SSID[7], pPort->P2PCfg.SSID[8]));            

    // Save WSC ConfigMode
    //TODO: WSC_ConfMode_REGISTRAR_UPNP is unavailable !!!
    if ((pPort->P2PCfg.P2PConnectState == P2P_ANY_IN_FORMATION_AS_CLIENT) 
        || (pPort->P2PCfg.P2PConnectState == P2P_DO_WPS_ENROLLEE))
        pPort->StaCfg.WscControl.WscConfMode = WSC_ConfMode_ENROLLEE;
    
    if ((pPort->P2PCfg.P2PConnectState == P2P_ANY_IN_FORMATION_AS_GO) 
        || (pPort->P2PCfg.P2PConnectState == P2P_WPS_REGISTRA))
        pPort->StaCfg.WscControl.WscConfMode = WSC_ConfMode_REGISTRAR;
    
    DBGPRINT(RT_DEBUG_TRACE, ("---->P2P pPort->P2PCfg.SsidLength = %d. [%x %x %x..]\n", pPort->P2PCfg.SSIDLen, 
        pPort->P2PCfg.SSID[0], pPort->P2PCfg.SSID[1], pPort->P2PCfg.SSID[2]));            
    DBGPRINT(RT_DEBUG_TRACE, ("Bssid = %x %x %x %x %x %x  \n", pPort->PortCfg.Bssid[0], pPort->PortCfg.Bssid[1],pPort->PortCfg.Bssid[2],pPort->PortCfg.Bssid[3],pPort->PortCfg.Bssid[4],pPort->PortCfg.Bssid[5]));

    // Transform RT_P2P_CONFIG to Wsc WSC_MODE Structure.
    // Save WSC Mode inforamtion
    pPort->StaCfg.WscControl.WscMode = (pPort->P2PCfg.WscMode & 0xf0000000);

    // Disable HwPBC status
    pAd->StaCfg.HwPBCState = FALSE;
    
    // For WSC, scan action shall be controlled by WSC state machine.
    pAd->StaCfg.ScanCnt = 3;
    pPort->StaCfg.WscControl.WscStatus = STATUS_WSC_IDLE;

    // Reset AP's BSSID.
    PlatformZeroMemory(pPort->StaCfg.WscControl.WscAPBssid, (sizeof(UCHAR) * MAC_ADDR_LEN));

    // TODO: We might to disable autoconnect here, and reenable it after getting the config file
    
    // Backup the AP's BSSID
    PlatformMoveMemory(pPort->StaCfg.WscControl.WscAPBssid, pPort->PortCfg.Bssid, MAC_ADDR_LEN);
    // 1. Cancel old timer to prevent use push continuously
    PlatformCancelTimer(&pPort->StaCfg.WscControl.WscConnectTimer, &Cancelled);
    PlatformCancelTimer(&pPort->StaCfg.WscControl.WscScanTimer, &Cancelled);
    PlatformCancelTimer(&pPort->StaCfg.WscControl.WscPBCTimer, &Cancelled);
    PlatformCancelTimer(&pPort->StaCfg.WscControl.WscPINTimer, &Cancelled);
    PlatformCancelTimer(&pPort->StaCfg.WscControl.WscEapRxTimer, &Cancelled);
    PlatformCancelTimer(&pPort->StaCfg.WscControl.WscM8WaitEapFailTimer, &Cancelled);
    PlatformCancelTimer(&pPort->StaCfg.WscControl.WscEapM2Timer, &Cancelled);
    PlatformCancelTimer(&pPort->StaCfg.WscControl.WscLEDTimer, &Cancelled);
    // Set WSC state to WSC_STATE_INIT
    pPort->StaCfg.WscControl.WscState = WSC_STATE_INIT;

    // Init Registrar pair structures
    // RegData will be empty including PIN code by called this func.
    WscInitRegistrarPair(pAd, pPort);
    // Store channel for BssidTable sort
    pPort->ScaningChannel = pPort->P2PCfg.GroupOpChannel;
    // Saved the WSC AP channel.
    // This is because some dual-band APs have the same BSSID in different bands and 
    // the Ralink UI needs this information to connect the AP with correct channel.
    pPort->StaCfg.WscControl.WscAPChannel = pPort->P2PCfg.GroupOpChannel;
    if ((pPort->StaCfg.WscControl.WscMode == WSC_PIN_MODE_USER_SPEC) ||(pPort->StaCfg.WscControl.WscMode == WSC_PIN_MODE)
        ||(pPort->StaCfg.WscControl.WscMode == WSC_PIN_MODE_REGISTRA_SPEC))
    {
        // 1. PIN mode, verify SSID sanity
        //    UI must set a SSID, no "any" allowed
        // Reset allowed scan retries
        // For WSC, this AP already in SSID table, scan shall not be allowed
        pAd->StaCfg.ScanCnt = 3;
        
        DBGPRINT(RT_DEBUG_TRACE, ("---->P2P pPort->P2PCfg.SsidLength = %d. [%x %x %x..]\n", pPort->P2PCfg.SSIDLen, 
            pPort->P2PCfg.SSID[0], pPort->P2PCfg.SSID[1], pPort->P2PCfg.SSID[2]));            

        // Make sure PIN haas been copied from profile settings
        if (pPort->P2PCfg.PINLen == 2 && !PlatformEqualMemory(pPort->P2PCfg.PinCode, ZeroPinCode4, 4))// 4 digits
        {
            PlatformMoveMemory(pPort->StaCfg.WscControl.RegData.PIN, pPort->P2PCfg.PinCode, 4);
            pPort->StaCfg.WscControl.RegData.PINLen = 2;
        }
        else     if(pPort->P2PCfg.PINLen == 4  && !PlatformEqualMemory(pPort->P2PCfg.PinCode, ZeroPinCode8, 8))// 8 digits
        {
            PlatformMoveMemory(pPort->StaCfg.WscControl.RegData.PIN, pPort->P2PCfg.PinCode, 8);
            pPort->StaCfg.WscControl.RegData.PINLen = 4;
        }

        // 2. Set 2 min timout routine
        PlatformSetTimer(pPort, &pPort->StaCfg.WscControl.WscConnectTimer, WSC_REG_SESSION_TIMEOUT);

        pPort->StaCfg.WscControl.WscState = WSC_STATE_START;
        pPort->StaCfg.WscControl.WscStatus = STATUS_WSC_LINK_UP;

        //kill WscEapM2Timer
        PlatformCancelTimer(&pPort->StaCfg.WscControl.WscEapM2Timer, &Cancelled);

        // The protocol is connecting to a partner.
        LedCtrlSetBlinkMode(pAd, LED_WPS_IN_PROCESS);
        
        DBGPRINT(RT_DEBUG_TRACE, ("%s:  (SsidLen=%d,Ssid=%c%c..., Bssid=%02x:%02x:%02x:%02x:%02x:%02x)\n", __FUNCTION__, 
                pPort->P2PCfg.SSIDLen, pPort->P2PCfg.SSID[0],  pPort->P2PCfg.SSID[1], 
                pPort->StaCfg.WscControl.WscAPBssid[0], pPort->StaCfg.WscControl.WscAPBssid[1], pPort->StaCfg.WscControl.WscAPBssid[2], pPort->StaCfg.WscControl.WscAPBssid[3], pPort->StaCfg.WscControl.WscAPBssid[4], pPort->StaCfg.WscControl.WscAPBssid[5]));
        DBGPRINT(RT_DEBUG_TRACE, ("WscMode = %x. WscConfigMode = %x. Channel = %x. SsidLength = %x. PinCode = %x %x  \n", 
            pPort->StaCfg.WscControl.WscMode, pPort->StaCfg.WscControl.WscConfMode, pPort->StaCfg.WscControl.WscAPChannel, 
            pPort->P2PCfg.SSIDLen, *(PULONG)&pPort->StaCfg.WscControl.RegData.PIN[0], *(PULONG)&pPort->StaCfg.WscControl.RegData.PIN[4]));
        
    }
    else if (pPort->StaCfg.WscControl.WscMode == WSC_PBC_MODE)
    {

        // For PBC, the PIN is all '0'
        pPort->StaCfg.WscControl.RegData.PIN[0] = '0';
        pPort->StaCfg.WscControl.RegData.PIN[1] = '0';
        pPort->StaCfg.WscControl.RegData.PIN[2] = '0';
        pPort->StaCfg.WscControl.RegData.PIN[3] = '0';
        pPort->StaCfg.WscControl.RegData.PIN[4] = '0';
        pPort->StaCfg.WscControl.RegData.PIN[5] = '0';
        pPort->StaCfg.WscControl.RegData.PIN[6] = '0';
        pPort->StaCfg.WscControl.RegData.PIN[7] = '0';
        PlatformMoveMemory(pPort->P2PCfg.PinCode, pPort->StaCfg.WscControl.RegData.PIN, 8);

        //default is using PIN 8 digit.
        pPort->StaCfg.WscControl.RegData.PINLen = 4;
        pPort->P2PCfg.PINLen = 4;
        
        // 2. Set 2 min timout routine
        PlatformSetTimer(pPort, &pPort->StaCfg.WscControl.WscConnectTimer, WSC_REG_SESSION_TIMEOUT);
        pPort->StaCfg.WscControl.WscState = WSC_STATE_START;
        pPort->StaCfg.WscControl.WscStatus = STATUS_WSC_LINK_UP;
    }

    pPort->PortCfg.AuthMode = Ralink802_11AuthModeWPA2PSK;
    pPort->PortCfg.CipherAlg = CIPHER_AES;
    pPort->PortCfg.WepStatus = DOT11_CIPHER_ALGO_CCMP;

}

/*  
    ==========================================================================
    Description: 
        Call this function after a Go negociation process fails. 
        
    Parameters: 
    Note:
         
    ==========================================================================
 */
VOID P2pGroupFormFailHandle(
    IN PMP_ADAPTER pAd) 
{
#if 0
    PMP_PORT    pPort = pAd->PortList[pPort->P2PCfg.PortNumber];
    
    pPort->P2PCfg.P2pCapability[1] &= ~(GRPCAP_GROUP_FORMING);
    P2pConnectStateUpdate(pAd);
    if (pPort->P2PCfg.P2PConnectState == P2P_ANY_IN_FORMATION_AS_GO
        ||pPort->P2PCfg.P2PConnectState == P2P_ANY_IN_FORMATION_AS_CLIENT
        || (pPort->P2PCfg.P2PConnectState ==  P2P_DO_GO_SCAN_BEGIN))
    {
        pPort->P2PCfg.P2PConnectState = P2P_CONNECT_IDLE;
        pPort->P2PCfg.Dpid = DEV_PASS_ID_NOSPEC;
        if (pPort->CommonCfg.P2pControl.field.DefaultConfigMethod == P2P_REG_CM_DISPLAY)
        {
            pPort->P2PCfg.ConfigMethod = CONFIG_METHOD_DISPLAY;
            DBGPRINT(RT_DEBUG_TRACE,(" Display = %x \n", pPort->P2PCfg.ConfigMethod));
        }
        else if (pPort->CommonCfg.P2pControl.field.DefaultConfigMethod == P2P_REG_CM_KEYPAD)
        {
            pPort->P2PCfg.ConfigMethod = CONFIG_METHOD_KEYPAD;
            DBGPRINT(RT_DEBUG_TRACE,(" Keypad = %x \n", pPort->P2PCfg.ConfigMethod));
        }
        else if (pPort->CommonCfg.P2pControl.field.DefaultConfigMethod == P2P_REG_CM_LABEL)
        {
            pPort->P2PCfg.ConfigMethod = CONFIG_METHOD_LABEL;
            DBGPRINT(RT_DEBUG_TRACE,(" use Label = %x \n", pPort->P2PCfg.ConfigMethod));
        }
        else if (pPort->CommonCfg.P2pControl.field.DefaultConfigMethod == P2P_REG_CM_PBC)
        {
            pPort->P2PCfg.ConfigMethod = CONFIG_METHOD_PUSHBUTTON;
            DBGPRINT(RT_DEBUG_TRACE,("Default use PBC = %x \n", pPort->P2PCfg.ConfigMethod));
        }
        else
        {
            pPort->P2PCfg.ConfigMethod = (CONFIG_METHOD_DISPLAY | CONFIG_METHOD_PUSHBUTTON | CONFIG_METHOD_KEYPAD);
            DBGPRINT(RT_DEBUG_TRACE,("Default all support = %x \n", pPort->P2PCfg.ConfigMethod));
        }
    }
#endif
}


/*  
    ==========================================================================
    Description: 
        Call this function after a successful Go negociation. will do coreessponding action as  GO or Client.
        
    Parameters: 
    Note:
         
    ==========================================================================
 */
VOID P2pGoNegoDone(
    IN PMP_ADAPTER pAd,
    IN PMP_PORT       pPort,
    IN PRT_P2P_CLIENT_ENTRY pP2pEntry) 
{
    DOT11_SCAN_REQUEST_V2 Dot11ScanRequest = {0};
    UCHAR                   MyRule = 0;
    BOOLEAN         Cancelled;

    Dot11ScanRequest.dot11ScanType = (dot11_scan_type_active | dot11_scan_type_forced);
    Dot11ScanRequest.uNumOfdot11SSIDs = 0;
    DBGPRINT(RT_DEBUG_TRACE, ("P2pGoNegoDone \n"));
    PlatformCancelTimer(&pPort->P2PCfg.P2pStopGoTimer, &Cancelled);
    PlatformCancelTimer(&pPort->P2PCfg.P2pStopClientTimer, &Cancelled);
    pPort->P2PCfg.ConnectingListenChannel = 0;

    if (pPort->P2PCfg.P2PConnectState == P2P_ANY_IN_FORMATION_AS_GO)
    {
        P2pStopScan(pAd, pPort);
        pPort->StaCfg.WscControl.WscConfMode = WSC_ConfMode_REGISTRAR;
        
        // GO's OOB configuation is set to un-configured to initiate a new WPS session excluding autonomous GO case
        if (IS_P2P_NOT_AUTOGO(pPort))
            pPort->P2PCfg.bConfiguredAP = WSC_APSTATE_UNCONFIGURED;
        
        P2pSetWps(pAd, pPort);
        // When AP buildAssociation () is called. and bWPSEnable is TRUE, know this is WPS association.

        // Always use 6Mbps to Ack EAPOL to enhance robustness.
        
        RTUSBWriteMACRegister(pAd, LEGACY_BASIC_RATE, 1);

        P2pStartGo(pAd, pAd->PortList[pPort->P2PCfg.PortNumber]);
        // Set MyRule in P2P GroupFormat
        MyRule = P2P_IS_GO;
    }
    else if (pPort->P2PCfg.P2PConnectState == P2P_ANY_IN_FORMATION_AS_CLIENT)
    {
        DBGPRINT(RT_DEBUG_TRACE, ("pPort->P2PCfg.Dpid = %d \n", pPort->P2PCfg.Dpid));
        pPort->StaCfg.WscControl.WscConfMode = WSC_ConfMode_ENROLLEE;
        // If WPS not triggered, don't so WPS AP now. Wait until P2PPRofile called
        if (pPort->P2PCfg.Dpid != DEV_PASS_ID_NOSPEC)
        {
            // Update Per client state
            if (pP2pEntry != NULL)
                pP2pEntry->P2pClientState = P2PSTATE_GO_WPS;
            // Update Global State. Prepare to scan for GO beacon. So as to connect using WPS.
            pPort->P2PCfg.P2PConnectState = P2P_DO_GO_NEG_DONE_CLIENT;
            pPort->P2PCfg.P2pCounter.Counter100ms = 0;
            // When Counter100ms reaches NextScanRound in P2P_DO_GO_NEG_DONE_CLIENT State,
            // Will start a site survey again. Because P2P GO start beacon after Go Negociation.
            // Some AP requires more time to start GO function.  ConfigTimeOut is in unit 10ms.
            if (pP2pEntry != NULL)
            {
                if (pP2pEntry->ConfigTimeOut <= 20)
                {
                    pPort->P2PCfg.P2pCounter.NextScanRound = 1;
                }
                else
                    pPort->P2PCfg.P2pCounter.NextScanRound = pP2pEntry->ConfigTimeOut/10;
            }
            pPort->StaCfg.WscControl.WscAPChannel = pPort->P2PCfg.GroupOpChannel;
            PlatformSetTimer(pPort, &pPort->P2PCfg.P2pStopClientTimer, P2P_CHECK_CLIENT_TIMER);
            // Set MyRule in P2P GroupFormat
            MyRule = P2P_IS_CLIENT;
            pPort->P2PCfg.P2pCounter.Counter100ms = 0;
            P2pStopScan(pAd, pPort); 
            
            DBGPRINT(RT_DEBUG_TRACE, ("P2pGoNegoDone. NextScanRound= %d\n", pPort->P2PCfg.P2pCounter.NextScanRound));
            DBGPRINT(RT_DEBUG_TRACE, ("2 WscMode = %x     \n", pPort->P2PCfg.WscMode));           
        }
    }
    else
        DBGPRINT(RT_DEBUG_TRACE, ("P2pGoNegoDone-  invalid p2pstate = %d\n", pPort->P2PCfg.P2PConnectState));
        
    // Update status and contents to SIGMA
    if (MyRule != 0)    
        P2PSetSigmaStatus(pAd, pPort, MyRule);

    pPort->P2PCfg.P2pCounter.GoScanBeginCounter100ms = 0;
    pPort->P2PCfg.ConnectingListenChannel = 0;
    DBGPRINT(RT_DEBUG_TRACE, ("P2pGoNegoDone<==: P2PState= %s\n", decodeP2PState(pPort->P2PCfg.P2PConnectState)));
    P2PPrintState(pAd);

}

/*  
    ==========================================================================
    Description: 
        Call this function after a successful WPS provisioning.  do coreessponding action as  GO or Client.
        
    Parameters: 
    Note:
         
    ==========================================================================
 */
VOID P2pWpsDone(
    IN PMP_ADAPTER pAd,
    IN PUCHAR       pAddr,
    IN PMP_PORT       pPort) 
{
#if 0
    BOOLEAN     Cancelled;
    UCHAR       MyRule = 0;
    
    DBGPRINT(RT_DEBUG_TRACE, ("---->P2pWpsDone. SSIDLen = %d. P2PState = %s. \n", pPort->P2PCfg.SSIDLen, decodeP2PState(pPort->P2PCfg.P2PConnectState)));
    
    // Backup the AP's BSSID for the UI.
    // Note that the STA supports only one profile.
    PlatformMoveMemory(pPort->StaCfg.WscControl.WscProfile.Profile[0].MacAddr,
        pPort->StaCfg.WscControl.WscAPBssid, MAC_ADDR_LEN);
    
    P2pCrednTabInsert(pAd, pAddr, &pPort->StaCfg.WscControl.WscProfile.Profile[0]);
    DBGPRINT(RT_DEBUG_TRACE, ("P2P WscAPBssid = %x %x %x %x %x %x. \n", pPort->StaCfg.WscControl.WscAPBssid[0], 
        pPort->StaCfg.WscControl.WscAPBssid[1], pPort->StaCfg.WscControl.WscAPBssid[2],pPort->StaCfg.WscControl.WscAPBssid[3],
        pPort->StaCfg.WscControl.WscAPBssid[4],pPort->StaCfg.WscControl.WscAPBssid[5]));            

    DBGPRINT(RT_DEBUG_TRACE, ("---->P2P WscAPChannel = %d. \n", pPort->StaCfg.WscControl.WscAPChannel));          

    // Receive Eap-Fail, cancel timer
    PlatformCancelTimer(&pPort->StaCfg.WscControl.WscM8WaitEapFailTimer, &Cancelled);
    
    // Reset the scanning counter.
    pAd->StaCfg.ScanCnt = 0;                    

    // The protocol is finished.
    LedCtrlSetBlinkMode(pAd, LED_WPS_SUCCESS);
    
    if (IS_P2P_ENROLLEE(pPort))
    {
        // Set P2PState to P2P_I_AM_CLIENT_ASSOC_AUTH @ CONNECT_REQUEST OID.
        DBGPRINT(RT_DEBUG_TRACE, ("Now I am in Client Associating state.  \n"));            
        pPort->PortCfg.AuthMode = Ralink802_11AuthModeWPA2PSK;
        pPort->PortCfg.CipherAlg = CIPHER_AES;
        pPort->PortCfg.WepStatus = DOT11_CIPHER_ALGO_CCMP;
        pPort->PortCfg.OrigWepStatus = DOT11_CIPHER_ALGO_CCMP;
        pPort->PortCfg.PairCipher    = DOT11_CIPHER_ALGO_CCMP;
        pPort->PortCfg.GroupCipher   = DOT11_CIPHER_ALGO_CCMP;
        pPort->PortCfg.MixedModeGroupCipher = Cipher_Type_NONE;
        pPort->SharedKey[BSS0][0].CipherAlg = CIPHER_AES;
        pPort->SharedKey[BSS0][1].CipherAlg = CIPHER_AES;
        pPort->SharedKey[BSS0][2].CipherAlg = CIPHER_AES;
        pPort->SharedKey[BSS0][3].CipherAlg = CIPHER_AES;
        // This event will cause GUI to set a Autoconfig profile.
        pPort->P2PCfg.P2pEventQueue.bWpsDone = TRUE;
        pPort->P2PCfg.P2PConnectState = P2P_DO_WPS_DONE_NEED_SETEVENT;
        // doesn't need auto reconnect after a successful WPS for P2P client case.
        pAd->MlmeAux.AutoReconnectSsidLen = 32;
        pAd->MlmeAux.AutoReconnectStatus = FALSE;
        PlatformZeroMemory(pAd->MlmeAux.AutoReconnectSsid, MAX_LEN_OF_SSID);
        pPort->P2PCfg.P2pCounter.CounterAftrWpsDone = 1;  // delay 0.1 sec
        // Need to copy to WpsDone structure. the Mac Addr is to tell GUI this profile is for which mac addr.
        // Although profile might be the same for all clients. we still copy profile everytime.
        PlatformMoveMemory(pPort->P2PCfg.P2PQueryStatusOid.P2PStateContext.WpsDone.WpsDoneContent.Addr, pAddr, MAC_ADDR_LEN);
        //P2pSetEvent(pAd, pPort,  P2PEVENT_STATUS_WPS_DONE);
        // Set MyRule in Wps Done
        MyRule = P2P_IS_CLIENT; 
        PlatformCancelTimer(&pPort->P2PCfg.P2pStopClientTimer, &Cancelled);
        PlatformSetTimer(pPort, &pPort->P2PCfg.P2pStopClientTimer, P2P_CHECK_CLIENT_TIMER);
        if(pAd->OpMode == OPMODE_STAP2P)
        {   
            PMAC_TABLE_ENTRY pEntry = MlmeSyncMacTabMatchedRoleType(pPort, ROLE_WLANIDX_P2P_CLIENT);
            if(pEntry == NULL)
            {
                DBGPRINT(RT_DEBUG_TRACE,("%s(), Line:%d, Mac Entry is NULL\n", __FUNCTION__, __LINE__));
                return;
            }
            
            pEntry->PortSecured = WPA_802_1X_PORT_NOT_SECURED;
            DBGPRINT(RT_DEBUG_TRACE, (" port secured = %d", pEntry->PortSecured));

        }
    }
    else if (IS_P2P_REGISTRA(pPort) || IS_P2P_GO_OP(pPort))
    {
        DBGPRINT(RT_DEBUG_TRACE, ("Now I am in Go Associating state.  pPort->MacTab.Size = %d.\n", pPort->MacTab.Size));
        PlatformMoveMemory(pPort->PortCfg.Bssid, pPort->CurrentAddress, MAC_ADDR_LEN);
        PlatformMoveMemory(pPort->PortCfg.Ssid, pPort->P2PCfg.SSID, 32);
        pPort->PortCfg.SsidLen = pPort->P2PCfg.SSIDLen;
        // This event will cause GUI to Add PMK.
        // Need to copy to WpsDone structure. the Mac Addr is to tell GUI this profile is for which mac addr.
        // Although profile might be the same for all clients. we still copy profile everytime.
        PlatformMoveMemory(pPort->P2PCfg.P2PQueryStatusOid.P2PStateContext.WpsDone.WpsDoneContent.Addr, pAddr, MAC_ADDR_LEN);
        /*if (IS_P2P_SIGMA_OFF(pPort)&& (pAd->OpMode == OPMODE_STAP2P) && (pPort->CommonCfg.P2pControl.field.P2PGroupLimit == 0))
        {
            DBGPRINT(RT_DEBUG_TRACE, ("Case 1: Concurrnet P2PGroupLimit is 'no limit'. Don't set WPS Done event.\n"));
        }*/
        if ((pAd->OpMode == OPMODE_STAP2P) && (pPort->CommonCfg.P2pControl.field.P2PGroupLimit == 1))
        {
            DBGPRINT(RT_DEBUG_TRACE, ("Case 2: Concurrnet P2PGroupLimit is 'limit only one'.  .\n"));
            DBGPRINT(RT_DEBUG_TRACE, ("So set WPS Done event to restart host AP with WPS random 64Byte PMK.\n"));
        }
        else if  (pAd->OpMode == OPMODE_STAP2P)
        {
            DBGPRINT(RT_DEBUG_TRACE, ("Case 0:  \n"));
            if (INFRA_ON(pAd->PortList[PORT_0]) && (pPort->MacTab.Size <=1 ))
            {
                DBGPRINT(RT_DEBUG_TRACE, ("Case 3: INFRA ON . set WPS Done event.\n"));
                P2pSetEvent(pAd, pPort,  P2PEVENT_STATUS_WPS_DONE);
            }
            else if (!INFRA_ON(pAd->PortList[PORT_0]) && (pPort->MacTab.Size <=1 ))
            {
                DBGPRINT(RT_DEBUG_TRACE, ("Case 4: INFRA OFF. set WPS Done event.\n"));
                P2pSetEvent(pAd, pPort,  P2PEVENT_STATUS_WPS_DONE);
            }
            else
                P2pSetEvent(pAd, pPort,  P2PEVENT_STATUS_WPS_DONE);
                //DBGPRINT(RT_DEBUG_TRACE, ("doesn't call WPS done event . pPort->MacTab.Size = %d. \n", pPort->MacTab.Size));
        }
        else if  (pAd->OpMode == OPMODE_STA)
        {
            DBGPRINT(RT_DEBUG_TRACE, ("Case 1: Station mode.  .\n"));
            P2pSetEvent(pAd, pPort,  P2PEVENT_STATUS_WPS_DONE);
        }

        // Once WPS is success. Set Dpid to default unspecified. this means need to push PIN or PBC again to start a new WPS M1~M8.
        // Reset WscState and Timer. 
        pPort->P2PCfg.Dpid = DEV_PASS_ID_NOSPEC;
        pPort->StaCfg.WscControl.WscState = WSC_STATE_OFF;
        pPort->StaCfg.WscControl.WscStatus = STATUS_WSC_EAP_CONFIGURED;
        PlatformCancelTimer(&pPort->P2PCfg.P2pStopWpsTimer, &Cancelled);

        // Set MyRule in Wps Done
        MyRule = P2P_IS_GO;     
        pPort->P2PCfg.P2pCapability[1] &= (~(GRPCAP_GROUP_FORMING));
        P2pUpdateBeaconP2pCap(pAd, pPort->P2PCfg.P2pCapability);

        // Always use 6Mbps to Ack EAPOL to enhance robustness. So after WPS finish, restore it.        
        RTUSBWriteMACRegister(pAd, LEGACY_BASIC_RATE, pPort->CommonCfg.BasicRateBitmap);

    }

    // Update status and contents to SIGMA
    if (MyRule != 0)
        P2PSetSigmaStatus(pAd, pPort, MyRule);

    PlatformZeroMemory( pPort->P2PCfg.PinCode, 8);
    PlatformZeroMemory( pPort->StaCfg.WscControl.RegData.PIN, 8);
    pPort->StaCfg.WscControl.RegData.PINLen = 0;
    DBGPRINT(RT_DEBUG_TRACE, ("<----P2pWpsDone. AuthMode = %d \n", pPort->PortCfg.AuthMode));           
    DBGPRINT(RT_DEBUG_TRACE, ("P2pWpsDone-  pPort->PortType = %d \n",  pPort->PortType));
    DBGPRINT(RT_DEBUG_TRACE, ("=====================================================\n"));
    P2PPrintState(pAd);
#endif
}

/*  
    ==========================================================================
    Description: 
        Call this function after a successful WPA2PSK AES 4-way provisioning. do coreessponding action as  GO or Client.
        
    Parameters: 
    Note:
         Atheros use Byte0, bit 4 to assign interface addr and device addr.
         Broadcom uses Byte4, bit ? to assign interface addr and device addr.
    ==========================================================================
 */
VOID P2pWPADone(
    IN PMP_ADAPTER pAd,
    PMP_PORT  pPort,
    IN MAC_TABLE_ENTRY  *pEntry,
    IN BOOLEAN      bGO)
{

    UCHAR       index = 0xff;
    BOOLEAN     Cancelled;
    PMAC_TABLE_ENTRY    pP2pEntry = pEntry;

    // P2P and SoftapAP/VwifiAP can not co-exist.
    if (pAd->OpMode == OPMODE_AP || (pAd->OpMode == OPMODE_APCLIENT))
        return;
    
    if (P2P_OFF(pPort))
    {
        DBGPRINT(RT_DEBUG_TRACE, ("P2pWPADone return. P2P is OFF  !!! \n"));
        return;
    }

    // Not use in MS WFD
    if (pPort->P2PCfg.P2PConnectState == P2P_CONNECT_NOUSE)
        return;

    if (pPort->PortNumber != pPort->P2PCfg.PortNumber)
    {
        DBGPRINT(RT_DEBUG_TRACE, ("P2pWPADone return. Different PortNum (%d %d)  !!! \n", pPort->PortNumber, pPort->P2PCfg.PortNumber));
        return;
    }
    
    // Use the owner of entry in concurrent Client if this entry has inserted
    if ((IS_P2P_CON_CLI(pAd, pPort)) && (pEntry->ValidAsCLI))
        pP2pEntry = pEntry;


    index = P2pGroupTabSearch(pAd, pP2pEntry->Addr);
    
    P2P_CLEAR_FLAG(&pAd->pP2pCtrll->P2PTable.Client[index], P2PFLAG_PASSED_NEGO_REQ);
    P2P_CLEAR_FLAG(&pAd->pP2pCtrll->P2PTable.Client[index], P2PFLAG_GO_INVITE_ME);
    if (bGO == FALSE)
    {
        // =========== CLIENT Case ==============
        if (IS_P2P_CLIENT_OP(pPort))
        {
            DBGPRINT(RT_DEBUG_TRACE, ("P2pWPADone- I am P2P Client.  P2pindex = %d. Aid=%d\n", index, pP2pEntry->Aid));
            if (index < MAX_P2P_GROUP_SIZE)
            {
                pAd->pP2pCtrll->P2PTable.Client[index].P2pClientState = P2PSTATE_GO_OPERATING;
                P2P_CLEAR_FLAG(&pAd->pP2pCtrll->P2PTable.Client[index], P2PFLAG_PROVISIONED);
                //P2P_CLEAR_FLAG(&pAd->pP2pCtrll->P2PTable.Client[index], P2PFLAG_GO_INVITE_ME);
                P2pCopyP2PTabtoMacTab(pAd, index, (UCHAR)pP2pEntry->Aid);
            }
            pPort->P2PCfg.P2pEventQueue.bDeleteWirelessCon = FALSE;
        }
        PlatformCancelTimer(&pPort->P2PCfg.P2pStopClientTimer, &Cancelled);
    }
    else
    {
        //For concurrent mode GO, we create seudo infra profile after WPA is completed
        if((pPort->P2PCfg.bSetProfile == TRUE) && (pAd->OpMode == OPMODE_STAP2P))
        {
            pPort->P2PCfg.P2pEventQueue.bP2pGenSeudoAdhoc = FALSE;
            if (P2pSetEvent(pAd, pPort,  P2PEVENT_GEN_SEUDO_ADHOC_PROFILE) == FALSE)
                pPort->P2PCfg.P2pEventQueue.bP2pGenSeudoAdhoc = TRUE;
        }
        
        // =========== GO Case ==============
        // Need to enable dhcp server because this is the first client that connect to me.
        pPort->P2PCfg.P2pCounter.Wpa2pskCounter100ms = 0;
        DBGPRINT(RT_DEBUG_TRACE, ("P2pWPADone-  P2pindex = %d\n", index));
        if (pAd->OpMode == OPMODE_STA)
        {
            pPort->SoftAP.ApCfg.bSoftAPReady = TRUE;
        }
        pPort->PortCfg.AuthMode = Ralink802_11AuthModeWPA2PSK;
        pPort->PortCfg.CipherAlg = CIPHER_AES;
        pPort->PortCfg.WepStatus = DOT11_CIPHER_ALGO_CCMP;
        pPort->PortCfg.OrigWepStatus = DOT11_CIPHER_ALGO_CCMP;
        pPort->PortCfg.PairCipher    = DOT11_CIPHER_ALGO_CCMP;
        pPort->PortCfg.GroupCipher   = DOT11_CIPHER_ALGO_CCMP;
        pPort->PortCfg.MixedModeGroupCipher = Cipher_Type_NONE;
        pAd->StaCfg.WpaState = SS_NOTUSE;
        pPort->P2PCfg.P2PConnectState = P2P_I_AM_GO_OP;
        
        // =========== GO Case ==============
        if (index < MAX_P2P_GROUP_SIZE)
        {
            P2P_CLEAR_FLAG(&pAd->pP2pCtrll->P2PTable.Client[index], P2PFLAG_PROVISIONED);
            P2P_CLEAR_FLAG(&pAd->pP2pCtrll->P2PTable.Client[index], P2PFLAG_PASSED_NEGO_REQ);
            pAd->pP2pCtrll->P2PTable.Client[index].P2pClientState = P2PSTATE_CLIENT_OPERATING;
            P2pCopyP2PTabtoMacTab(pAd, index, (UCHAR)pP2pEntry->Aid);
            if (IS_P2P_STA_GO(pAd, pPort))
            {
                GoIndicateAssociationStatus(pAd, pPort->SoftAP.ApCfg.AdhocBssid, (UCHAR)pEntry->Aid);
            }
            P2PPrintP2PEntry(pAd, index);
        }
        else                
        {
            // If can't find this peer in P2Ptable. this must be legacy client that only in MACtab. 
            if (IS_P2P_STA_GO(pAd, pPort))
            {
                GoIndicateAssociationStatus(pAd, pPort->SoftAP.ApCfg.AdhocBssid, (UCHAR)pP2pEntry->Aid);
            }
        }
        
        PlatformCancelTimer(&pPort->P2PCfg.P2pStopGoTimer, &Cancelled);
        // =========== GO Case ==============
        DBGPRINT(RT_DEBUG_TRACE, ("P2pWPADone- ICSStatus = %d. \n", pPort->P2PCfg.P2pManagedParm.ICSStatus));
        
        DBGPRINT(RT_DEBUG_TRACE, ("P2pWPADone-  Aid = %d. WepStatus = %d. DefaultKeyId = %d. \n", pP2pEntry->Aid, pP2pEntry->WepStatus, pPort->PortCfg.DefaultKeyId));
        DBGPRINT(RT_DEBUG_TRACE, ("P2pWPADone-  pPort->PortType = %d.  MacTab.Size = %d. dhcp ? %d \n",  pPort->PortType, pPort->MacTab.Size, pPort->P2PCfg.P2pEventQueue.bEnableDhcp));
        // Also update discovery, because the one that is just connect with me is not belong to discovery table again.

        // =========== GO Case ==============
        pPort->P2PCfg.P2pCapability[1] |= (GRPCAP_OWNER);
        if (pPort->P2PCfg.SigmaSetting.StartNoaWhenGO != 0)
        {
            pPort->P2PCfg.GONoASchedule.Count = 255;
            pPort->P2PCfg.GONoASchedule.Duration = 0xc800;    // 50ms
            pPort->P2PCfg.GONoASchedule.Interval = 0x19000;   // 100ms

            pPort->P2PCfg.SigmaSetting.StartNoaWhenGO = 0;
            P2pGOStartNoA(pAd, pPort, FALSE);
            P2pUpdateNoABeacon(pAd, pAd->pTxCfg->BeaconBufLen);
        }
        // spend 50*100ms to do P2P scan to have good P2P topology in GUI.
        P2pScanChannelDefault(pAd, pPort);
        P2pSetListenIntBias(pAd,  1);
        pPort->P2PCfg.P2PDiscoProvState = P2P_ENABLE_LISTEN_ONLY;
            
    }

    // default set extended listening to zero for each connection. If this is persistent, will set it.
    pPort->P2PCfg.ExtListenInterval = 0;
    pPort->P2PCfg.ExtListenPeriod = 0;
    DBGPRINT(RT_DEBUG_TRACE, ("P2pWPADone-   . GrpCap= %x \n", pP2pEntry->P2pInfo.GroupCapability));
    if (IS_PERSISTENT_ON(pAd) /*&& MT_TEST_BIT(pP2pEntry->P2pInfo.GroupCapability , GRPCAP_PERSISTENT)*/)
    {
        DBGPRINT(RT_DEBUG_TRACE, ("-pP2pEntry->ValidAsP2P = %x \n", pP2pEntry->ValidAsP2P));

        DBGPRINT(RT_DEBUG_TRACE, ("P2pWPADone- Save to persistent entry. GrpCap= %x \n", pAd->pP2pCtrll->P2PTable.Client[index].GroupCapability));
        P2pPerstTabInsert(pAd, pP2pEntry->P2pInfo.DevAddr);
        // this is a persistent connection.
        pPort->P2PCfg.ExtListenInterval = P2P_EXT_LISTEN_INTERVAL;
        pPort->P2PCfg.ExtListenPeriod = P2P_EXT_LISTEN_PERIOD;
        DBGPRINT(RT_DEBUG_TRACE, ("P2pWPADone-  Set Extended timing !!!!!!!\n"));
    }

    // Update status and contents to SIGMA
    P2PSetSigmaStatus(pAd, pPort, (bGO ? P2P_IS_GO : P2P_IS_CLIENT));
    // Don't do too much scan. So Don't set CounterAftrScanButton to zero.
    
    P2PDefaultConfigM(pAd, pPort);
    // Zero the mac address, becuase I am successfully connected .
    PlatformZeroMemory(&pPort->P2PCfg.ConnectingMAC[pPort->P2PCfg.ConnectingIndex][0], MAC_ADDR_LEN);
    P2pConnectForward(pAd);
    P2PPrintState(pAd);

    //Diable Auto Reconnect once P2P Client finished 4way
    if (IS_P2P_CLIENT_OP(pPort))
    {
        pAd->MlmeAux.AutoReconnectSsidLen = 32;
        pAd->MlmeAux.AutoReconnectStatus = FALSE;
        PlatformZeroMemory(pAd->MlmeAux.AutoReconnectSsid, MAX_LEN_OF_SSID);
    }

    // Indicate a successful connection once wpa done
    if (index < MAX_P2P_GROUP_SIZE)
    {
        if (FALSE == P2pSetEvent(pAd, pPort,  P2PEVENT_CONNECT_COMPLETE))
            pPort->P2PCfg.P2pEventQueue.bWpaDone = TRUE;
    }
    
    if(bGO)
    {
        if ((pAd->OpMode == OPMODE_STAP2P) && (INFRA_ON(pAd->PortList[PORT_0]))
            && (pPort->MacTab.Size < 3)
            && (pPort->P2PCfg.P2pManagedParm.ICSStatus == ICS_STATUS_DISABLED))
        {
            pPort->P2PCfg.P2pEventQueue.bEnableDhcp = FALSE;
            if (P2pSetEvent(pAd, pPort,  P2PEVENT_ENABLE_DHCP_SERVER) == FALSE)
                pPort->P2PCfg.P2pEventQueue.bEnableDhcp = TRUE;
        }
        else if (pPort->MacTab.Size < 2)
        {
            pPort->P2PCfg.P2pEventQueue.bEnableDhcp = FALSE;
            if (P2pSetEvent(pAd, pPort,  P2PEVENT_ENABLE_DHCP_SERVER) == FALSE)
                pPort->P2PCfg.P2pEventQueue.bEnableDhcp = TRUE;
        }
    }
    
    //pPort->P2PCfg.P2pEventQueue.bP2pTopologyUpdate = TRUE;
    pPort->P2PCfg.P2pCounter.TopologyUpdateCounter = 0;
    // persistent no need wps stage so after wpa we should clear the pin code
    PlatformZeroMemory( pPort->P2PCfg.PinCode, 8);
    PlatformZeroMemory( pPort->StaCfg.WscControl.RegData.PIN, 8);
    pPort->StaCfg.WscControl.RegData.PINLen = 0;
}

/*  
    ==========================================================================
    Description: 
        Call this function for OID Conntect_Request.
        
    Parameters: 
    Note:
         
    ==========================================================================
 */
VOID P2pIndicateGo(
    IN PMP_ADAPTER pAd,
    IN  PMP_PORT pPort) 
{   
    BSS_ENTRY   bss;
    DBGPRINT(RT_DEBUG_TRACE, ("P2pIndicateGo : WscControl.WscState = %d \n", pPort->StaCfg.WscControl.WscState));         

    OPSTATUS_TEST_FLAG(pPort, fOP_STATUS_MEDIA_STATE_CONNECTED);

    pPort->PortCfg.DesiredSSID.uSSIDLength = pPort->P2PCfg.SSIDLen;
    PlatformMoveMemory(pPort->PortCfg.DesiredSSID.ucSSID, pPort->P2PCfg.SSID, 32);
//      PlatformIndicateAssociationCompletion(pAd, pPort, DOT11_ASSOC_STATUS_SUCCESS);
    PlatformMoveMemory(pPort->SoftAP.ApCfg.AdhocBssid, pPort->CurrentAddress, MAC_ADDR_LEN);

    //Vista only
    // A fake BSSID
    pPort->SoftAP.ApCfg.AdhocBssid[0] = 0xF0;
    pPort->SoftAP.ApCfg.AdhocBssid[1] = 0x50;
    pPort->SoftAP.ApCfg.AdhocBssid[2] = 0x40;
    pPort->SoftAP.ApCfg.AdhocBssid[3] = 0x30;
    pPort->SoftAP.ApCfg.AdhocBssid[4] = 0x20;
    pPort->SoftAP.ApCfg.AdhocBssid[5] = 0x10;
    pPort->PortCfg.AuthMode = Ralink802_11AuthModeWPA2PSK;
    pPort->PortCfg.CipherAlg = CIPHER_AES;
    pPort->PortCfg.WepStatus = DOT11_CIPHER_ALGO_CCMP;
    pPort->PortCfg.OrigWepStatus = DOT11_CIPHER_ALGO_CCMP;
    pPort->PortCfg.PairCipher    = DOT11_CIPHER_ALGO_CCMP;
    pPort->PortCfg.GroupCipher   = DOT11_CIPHER_ALGO_CCMP;
    pPort->PortCfg.MixedModeGroupCipher = Cipher_Type_NONE;
    pPort->SharedKey[BSS0][0].CipherAlg = CIPHER_AES;
    pPort->SharedKey[BSS0][1].CipherAlg = CIPHER_AES;
    pPort->SharedKey[BSS0][2].CipherAlg = CIPHER_AES;
    pPort->SharedKey[BSS0][3].CipherAlg = CIPHER_AES;
    pPort->PortCfg.DefaultKeyId = 1;

    //Indicate FAKE BSSID
    if(IS_P2P_CON_GO(pAd, pPort))
    {
        PlatformMoveMemory(bss.Bssid, pPort->CurrentAddress, MAC_ADDR_LEN);
        PlatformMoveMemory(pAd->MlmeAux.Bssid, pPort->CurrentAddress, MAC_ADDR_LEN);
        pAd->MlmeAux.BssType = BSS_INFRA;
    }
    else
    {
        PlatformMoveMemory(pAd->MlmeAux.Bssid, pPort->SoftAP.ApCfg.AdhocBssid, MAC_ADDR_LEN);
    }
    
    pPort->SoftAP.ApCfg.bSoftAPReady = TRUE;

    //Concurrent Mode GO --> Pseudo Infra Mode
    //Station Mode GO --> Pseudo Ad Hoc Mode
    if(IS_P2P_CON_GO(pAd, pPort))
    {
        pAd->StaCfg.BeaconFrameSize = 237; // in win8 the beacon frame size can not be 0 or the connect state will be Disconnect
        PlatformIndicateConnectionStart(pAd, pPort, BSS_INFRA, NULL, NULL, 0);
        PlatformIndicateAssociationStart(pAd, pPort, &bss);
        PlatformIndicateAssociationCompletion(pAd, pPort, DOT11_ASSOC_STATUS_SUCCESS);
        PlatformIndicateConnectionCompletion(pAd, pPort, DOT11_ASSOC_STATUS_SUCCESS);
    }
    else
    {
        GoIndicateConnectStatus(pAd);
        // the first one must use index = BSSID_WCID
        GoIndicateAssociationStatus(pAd, pPort->SoftAP.ApCfg.AdhocBssid, BSSID_WCID+1);
    }
    
    PlatformIndicateNewLinkSpeed(pAd, pPort->PortNumber, OPMODE_STA);

    DBGPRINT(RT_DEBUG_TRACE, ("P2pIndicateGo : AuthMode = %d \n", pAd->PortList[pPort->P2PCfg.PortNumber]->PortCfg.AuthMode));            

}

/*  
    ==========================================================================
    Description: 
        After finishing Invitation process, P2P Device should go back to Operating channel.
        
    Parameters: 
    Note:
         
    ==========================================================================
 */
VOID P2pBackToOpChannel(
    IN PMP_ADAPTER pAd,
    IN PMP_PORT     pPort)
{
    DBGPRINT(RT_DEBUG_TRACE, ("%s.  ==> \n", __FUNCTION__));
    if (pAd->PortList[pPort->P2PCfg.PortNumber]->PortSubtype == PORTSUBTYPE_P2PGO)
    {
        P2pGoSwitchChannel(pAd, pAd->PortList[pPort->P2PCfg.PortNumber]);

        if (FALSE == P2pCheckBssSync(pAd))
            AsicResumeBssSync(pAd);
    }
}

VOID P2pGoSwitchChannel(
    IN PMP_ADAPTER pAd,
    IN  PMP_PORT pPort)
{   
    DBGPRINT(RT_DEBUG_TRACE, ("%s.  ==> \n", __FUNCTION__));
    DBGPRINT(RT_DEBUG_TRACE, ("BW = %d. Cha = %d . CenCha = %d. \n",  pPort->BBPCurrentBW, pPort->Channel, pPort->CentralChannel));
    
    // Change to AP channel
    if (pPort->CentralChannel > pPort->Channel)
    {   
        if ((pAd->StaCfg.Feature11n&0x3) != 0x03) // 0x3 means we have switch to HT40
        {
            // Must using 40MHz.
            pPort->BBPCurrentBW = BW_40;
            SwitchBandwidth(pAd, TRUE, pPort->CentralChannel, BW_40, EXTCHA_ABOVE);

#ifdef MULTI_CHANNEL_SUPPORT
            MultiChannelSetCentralCh(pAd, pPort, pPort->CentralChannel, EXTCHA_ABOVE, pPort->BBPCurrentBW);
#endif /*#ifdef MULTI_CHANNEL_SUPPORT*/
            DBGPRINT(RT_DEBUG_TRACE, ("!!!40MHz Lower LINK UP !!! Control Channel at Below. Central = %d \n", pPort->CentralChannel ));
        }
    }
    else if (pPort->CentralChannel < pPort->Channel)
    {
        if ((pAd->StaCfg.Feature11n&0x3) != 0x03) // 0x3 means we have switch to HT40
        {
            // Must using 40MHz.
            pPort->BBPCurrentBW = BW_40;
            SwitchBandwidth(pAd, TRUE, pPort->CentralChannel, BW_40, EXTCHA_BELOW);
            
#ifdef MULTI_CHANNEL_SUPPORT
            MultiChannelSetCentralCh(pAd, pPort, pPort->CentralChannel, EXTCHA_BELOW, pPort->BBPCurrentBW);
#endif /*#ifdef MULTI_CHANNEL_SUPPORT*/

            DBGPRINT(RT_DEBUG_TRACE, ("!!!40MHz Upper LINK UP !!! Control Channel at UpperCentral = %d \n", pPort->CentralChannel ));
        }
    }
    else
    {
        pPort->BBPCurrentBW = BW_20;
// 2011-11-23 For P2pMs GO multi-channel
#ifdef MULTI_CHANNEL_SUPPORT
        DBGPRINT(RT_DEBUG_TRACE, ("%s - GO starts at GroupOpChannel(%d) and assign Channel to (%d)\n", 
                                    __FUNCTION__,
                                    pPort->P2PCfg.GroupOpChannel,
                                    pPort->Channel));

        //AsicSwitchChannel(pAd, pPort->P2PCfg.GroupOpChannel, FALSE);
        AsicSwitchChannel(pPort, pPort->P2PCfg.GroupOpChannel, pPort->P2PCfg.GroupOpChannel, pPort->BBPCurrentBW, FALSE);
        //AsicLockChannel(pAd, pPort->P2PCfg.GroupOpChannel);
        pPort->Channel = pPort->P2PCfg.GroupOpChannel;
#else
       // AsicSwitchChannel(pAd, pPort->Channel, FALSE);
#endif /* MULTI_CHANNEL_SUPPORT */
        
        SwitchBandwidth(pAd, FALSE, pPort->Channel, BW_20, EXTCHA_NONE);
        
        DBGPRINT(RT_DEBUG_TRACE, ("!!!20MHz LINK UP !!! \n" ));
    }
    
    DBGPRINT(RT_DEBUG_TRACE, ("%s: Feature11n = 0x%0x, reset bit1\n",__FUNCTION__ ,pAd->StaCfg.Feature11n));
    pAd->StaCfg.Feature11n &= (~0x2);       
    if (pAd->HwCfg.LatchRfRegs.Channel <= 14)
    {
    }
    else
    {  
        if (pPort->BBPCurrentBW == BW_20)
        {
            RTUSBWriteBBPRegister(pAd, BBP_R66, 0x40);
        }
        else
        {
            RTUSBWriteBBPRegister(pAd, BBP_R66, 0x48);
        }
    }
    
    DBGPRINT(RT_DEBUG_TRACE, ("<== BW = %d. Cha = %d . CenCha = %d. GroupOpChannel = %d. \n",  pPort->BBPCurrentBW, pPort->Channel, pPort->CentralChannel, pPort->P2PCfg.GroupOpChannel));

}

/*  
    ==========================================================================
    Description: 
        Decide P2P's Default HT capability parameters in ApCfg. If concurrent mode and infra is connected, HT should follow
        Infra AP's channel and bandwidth.
    Parameters: 
         
    Note:
         
    ==========================================================================
 */
VOID P2pSetDefaultGOHt(
    IN PMP_ADAPTER pAd,
    IN PMP_PORT     pPort)
{   
    PMP_PORT  pStaPort = NULL, pClientPort = NULL, pClient2Port = NULL;
    BOOLEAN     bUpperBand = FALSE, bLowerBand = FALSE;
    BOOLEAN     bUse40MHz = FALSE;

    // Found out all of clients
    pStaPort = pAd->PortList[PORT_0];
    
#if(COMPILE_WIN8_ABOVE(CURRENT_OS_NDIS_VER))  
    if (NDIS_WIN8_ABOVE(pAd))
    {
        pClientPort = MlmeSyncGetP2pClientPort(pAd, 0);
        pClient2Port= MlmeSyncGetP2pClientPort(pAd, 1);
    }
    else
#endif
    {
        pClientPort = NULL;
        pClient2Port = NULL;
    }
    
    DBGPRINT(RT_DEBUG_TRACE, ("P2pSetDefaultGOHt: OpStatusFlags = %08x, bHtEnable = %d\n", pStaPort->PortCfg.OpStatusFlags, pAd->StaActive.SupportedHtPhy.bHtEnable));

    // P2P ON, ApCfg is only for P2P GO. So I can set here.
    pPort->SoftAP.ApCfg.HtCapability.HtCapInfo.ShortGIfor40 = GI_400;
    pPort->SoftAP.ApCfg.HtCapability.HtCapInfo.ShortGIfor20 = GI_400;
    pPort->SoftAP.ApCfg.HtCapability.HtCapInfo.TxSTBC = STBC_USE;
    pPort->SoftAP.ApCfg.HtCapability.HtCapInfo.RxSTBC = STBC_USE;
    pPort->SoftAP.ApCfg.HtCapability.HtCapParm.MaxRAmpduFactor = 2;
    pPort->SoftAP.ApCfg.HtCapability.ExtHtCapInfo.RDGSupport = 1;
    pPort->SoftAP.ApCfg.HtCapability.ExtHtCapInfo.PlusHTC = 1;
    pPort->SoftAP.ApCfg.HtCapability.MCSSet[0] = 0xff;
    pPort->SoftAP.ApCfg.AddHTInfoIe.ControlChan = pPort->P2PCfg.GroupOpChannel;
    pPort->P2PCfg.P2pPhyMode = P2P_PHYMODE_ENABLE_11N_20;
    pPort->SoftAP.ApCfg.HtCapability.HtCapInfo.ChannelWidth = BW_20;
    pPort->SoftAP.ApCfg.AddHTInfoIe.AddHtInfo.RecomWidth = 0;
    pPort->SoftAP.ApCfg.AddHTInfoIe.AddHtInfo.ExtChanOffset = EXTCHA_NONE;

    //If station port or client port is connected with 11N AP/GO, station's HT setting will be used as softap's HT setting.
    if ((INFRA_ON(pStaPort) && (pStaPort->PortSubtype == PORTSUBTYPE_STA)) ||
        (pClientPort && INFRA_ON(pClientPort)) ||
        (pClient2Port && INFRA_ON(pClient2Port)))
    {
// 2011-11-28 For P2pMs GO multi-channel
#ifdef MULTI_CHANNEL_SUPPORT
        // In multi-channel case, GO will use 1. STA+CLI: CLI ch 2. STA: STA ch 3. CLI: CLI ch
#else       
        pPort->P2PCfg.GroupChannel = pPort->Channel;
        pPort->P2PCfg.GroupOpChannel = pPort->Channel;
#endif /* MULTI_CHANNEL_SUPPORT */
        // set control channel again
        pPort->SoftAP.ApCfg.AddHTInfoIe.ControlChan = pPort->P2PCfg.GroupOpChannel;
        // MlmeCntLinkUp will assign Channdl and CentralChannel. So don't need assign those channels here.
        DBGPRINT(RT_DEBUG_TRACE, ("case 1 : STA port is connected \n"));
        if (pAd->StaActive.SupportedHtPhy.bHtEnable == FALSE)
        {
            DBGPRINT(RT_DEBUG_TRACE, ("case 1-1:  STA port doesn't use 11n \n"));
        }
        else
        {
            DBGPRINT(RT_DEBUG_TRACE, ("case 1-2 STA port uses 11n \n"));
            DBGPRINT(RT_DEBUG_TRACE, ("His ChannelWidth = %d, ExtChanOffset = %d\n", pAd->StaActive.SupportedHtPhy.HtChannelWidth, pAd->StaActive.SupportedHtPhy.ExtChanOffset));
            pPort->SoftAP.ApCfg.HtCapability.HtCapInfo.ChannelWidth = pAd->StaActive.SupportedHtPhy.HtChannelWidth;
            pPort->SoftAP.ApCfg.AddHTInfoIe.AddHtInfo.ExtChanOffset = pAd->StaActive.SupportedHtPhy.ExtChanOffset;
            pPort->SoftAP.ApCfg.AddHTInfoIe.AddHtInfo.RecomWidth = (UCHAR)pAd->StaActive.SupportedHtPhy.HtChannelWidth;
            if (pPort->SoftAP.ApCfg.AddHTInfoIe.AddHtInfo.RecomWidth == 1)
            {
                pPort->P2PCfg.P2pPhyMode = P2P_PHYMODE_ENABLE_11N_40;
                pPort->BBPCurrentBW = BW_40;
            }
        }
    }
    else
    {
        //since Station port is not connected with INFRA, I can set its value here.
        pPort->BBPCurrentBW = BW_20;
        pPort->Channel = pPort->P2PCfg.GroupOpChannel;
        pPort->CentralChannel = pPort->P2PCfg.GroupOpChannel;
        DBGPRINT(RT_DEBUG_TRACE, ("case 2 : STA port is NOT connected. Use GO 's own rule. \n"));
        //ExtChanOffset
        if (pPort->P2PCfg.GroupOpChannel > 14)
        {
            DBGPRINT(RT_DEBUG_TRACE, ("case 2-1 :  A Band   GroupOpChannel = %d \n", pPort->P2PCfg.GroupOpChannel));
            // Set BW20 when UI(SoftAP) set to BW20/EEPROM set to BW20
            if (pAd->HwCfg.NicConfig2.field.bDisableBW40ForA)
            {
                DBGPRINT(RT_DEBUG_TRACE, ("case 2-1-1 :  20MHz \n"));
                // BW40 for A band is NOT allowed
            }
            else
            {
                DBGPRINT(RT_DEBUG_TRACE, ("case 2-1-2 :   40MHz  \n"));
                switch (pPort->Channel)
                {
                    case 36:
                    case 44:
                    case 52:
                    case 60:
                    case 100:
                    case 108:
                    case 116:
                    case 124:
                    case 132:
                    case 149:
                    case 157:
                        pPort->SoftAP.ApCfg.HtCapability.HtCapInfo.ChannelWidth = BW_40;         
                        pPort->SoftAP.ApCfg.AddHTInfoIe.AddHtInfo.RecomWidth = 1;
                        pPort->BBPCurrentBW = BW_40;
                        pPort->P2PCfg.P2pPhyMode = P2P_PHYMODE_ENABLE_11N_40;
                        pPort->CentralChannel = pPort->P2PCfg.GroupOpChannel + 2;
                        pPort->SoftAP.ApCfg.AddHTInfoIe.AddHtInfo.ExtChanOffset = EXTCHA_ABOVE;
                        break;
                        
                    case 140:
                    case 165:
                    case 169:
                    case 173:
                        //BW_20 in these channels
                        pPort->CentralChannel = pPort->P2PCfg.GroupOpChannel;
                        break;

                    default:
                        pPort->SoftAP.ApCfg.HtCapability.HtCapInfo.ChannelWidth = BW_40;         
                        pPort->SoftAP.ApCfg.AddHTInfoIe.AddHtInfo.RecomWidth = 1;
                        pPort->BBPCurrentBW = BW_40;
                        pPort->P2PCfg.P2pPhyMode = P2P_PHYMODE_ENABLE_11N_40;
                        pPort->CentralChannel = pPort->P2PCfg.GroupOpChannel - 2;
                        pPort->SoftAP.ApCfg.AddHTInfoIe.AddHtInfo.ExtChanOffset = EXTCHA_BELOW;
                        break;
                }
            }
        }
        else // pPort->P2PCfg.groupOpChannel <= 14
        {
            DBGPRINT(RT_DEBUG_TRACE, ("case 2-2 :  G Band   GroupOpChannel = %d \n", pPort->P2PCfg.GroupOpChannel));
            // Set BW20 when UI(SoftAP) set to BW20/EEPROM set to BW20
            if(pAd->HwCfg.NicConfig2.field.bDisableBW40ForG || 
                (pPort->P2PCfg.GroupOpChannel > 11))
            {
                DBGPRINT(RT_DEBUG_TRACE, ("case 2-2-1 :   20MHz \n"));
                // BW40 for 2.4G band is NOT allowed
            }
            else
            {
                bUpperBand = RTMPCheckChannel(pAd, pPort->Channel + 2, pPort->Channel);
                if (pPort->Channel > 2)
                {
                    bLowerBand = RTMPCheckChannel(pAd, pPort->Channel - 2, pPort->Channel);
                }

                if (bUpperBand)
                {
                    DBGPRINT(RT_DEBUG_TRACE, ("case 2-2-2 :   40MHz EXTCHA_ABOVE \n"));
                    pPort->SoftAP.ApCfg.HtCapability.HtCapInfo.ChannelWidth = BW_40;         
                    pPort->SoftAP.ApCfg.AddHTInfoIe.AddHtInfo.RecomWidth = 1;
                    pPort->P2PCfg.P2pPhyMode = P2P_PHYMODE_ENABLE_11N_40;
                    pPort->SoftAP.ApCfg.AddHTInfoIe.AddHtInfo.ExtChanOffset = EXTCHA_ABOVE;
                    pPort->CentralChannel = pPort->Channel + 2;
                }
                else if (bLowerBand)
                {
                    DBGPRINT(RT_DEBUG_TRACE, ("case 2-2-2 :   40MHz  EXTCHA_BELOW\n"));
                    pPort->SoftAP.ApCfg.HtCapability.HtCapInfo.ChannelWidth = BW_40;         
                    pPort->SoftAP.ApCfg.AddHTInfoIe.AddHtInfo.RecomWidth = 1;
                    pPort->P2PCfg.P2pPhyMode = P2P_PHYMODE_ENABLE_11N_40;
                    pPort->SoftAP.ApCfg.AddHTInfoIe.AddHtInfo.ExtChanOffset =  EXTCHA_BELOW;
                    pPort->CentralChannel = pPort->Channel - 2;
                }
                else
                {                   
                    DBGPRINT(RT_DEBUG_TRACE, ("case 2-2-2 :   20MHz  EXTCHA_NONE\n"));
                    pPort->CentralChannel = pPort->Channel;
                }
            }
        }
    }

    if (pAd->HwCfg.Antenna.field.RxPath > 2)
    {
        pPort->SoftAP.ApCfg.HtCapability.MCSSet[1] = 0xff;
        pPort->SoftAP.ApCfg.HtCapability.MCSSet[2] = 0xff;
    }
    else if (pAd->HwCfg.Antenna.field.RxPath > 1)
    {
        pPort->SoftAP.ApCfg.HtCapability.MCSSet[1] = 0xff;
    }
                
    DBGPRINT(RT_DEBUG_TRACE,("%s: GO start with 40MHz (%s)  \n", __FUNCTION__, (bUse40MHz ? "yes" : "no")));
                
    if (IS_P2P_SIGMA_ON(pPort) || (pPort->CommonCfg.InBoxMode == TRUE) || (bUse40MHz == FALSE))
    {
        pPort->P2PCfg.P2pPhyMode = P2P_PHYMODE_ENABLE_11N_20;
        pPort->SoftAP.ApCfg.HtCapability.HtCapInfo.ChannelWidth = 0;
        pPort->SoftAP.ApCfg.HtCapability.MCSSet[0] = 0xff;
        pPort->SoftAP.ApCfg.AddHTInfoIe.AddHtInfo.RecomWidth = 0;
        pPort->SoftAP.ApCfg.AddHTInfoIe.ControlChan = pPort->P2PCfg.GroupChannel;
        pPort->CommonCfg.DesiredHtPhy.HtChannelWidth   = 0;
        pPort->CommonCfg.DesiredHtPhy.RecomWidth = 0;
        pPort->CommonCfg.DesiredHtPhy.ShortGIfor40 = 0;
        pPort->SoftAP.ApCfg.AddHTInfoIe.AddHtInfo.ExtChanOffset = EXTCHA_NONE;
        pPort->Channel = pPort->P2PCfg.GroupOpChannel;
        pPort->CentralChannel = pPort->P2PCfg.GroupOpChannel;
        DBGPRINT(RT_DEBUG_TRACE,("P2pSetDefaultGOHt===> SIGMA ON. BW is 0. Cha = %d, CenCha = %d  \n", pPort->Channel, pPort->CentralChannel));
    }
    // Save central channel to GO's centralCh
    pPort->P2PCfg.CentralChannel = pPort->CentralChannel;

}

VOID P2pSetGOTxRateSwitch(
    IN PMP_ADAPTER pAd,
    IN PMP_PORT   pPort)
{
    int i, num;
    UCHAR Rate = RATE_6, MaxDesire = RATE_1;

    DBGPRINT(RT_DEBUG_TRACE, ("(Port#%d) P2pSetGOTxRateSwitch: OpStatusFlags = %08x\n", pPort->PortNumber, pPort->PortCfg.OpStatusFlags));

    // find max desired rate
    num = 0;
    for (i=0; i<MAX_LEN_OF_SUPPORTED_RATES; i++)
    {
        switch (pPort->CommonCfg.DesireRate[i] & 0x7f)
        {
            case 2:  Rate = RATE_1;   num++;   break;
            case 4:  Rate = RATE_2;   num++;   break;
            case 11: Rate = RATE_5_5; num++;   break;
            case 22: Rate = RATE_11;  num++;   break;
            case 12: Rate = RATE_6;   num++;   break;
            case 18: Rate = RATE_9;   num++;   break;
            case 24: Rate = RATE_12;  num++;   break;
            case 36: Rate = RATE_18;  num++;   break;
            case 48: Rate = RATE_24;  num++;   break;
            case 72: Rate = RATE_36;  num++;   break;
            case 96: Rate = RATE_48;  num++;   break;
            case 108: Rate = RATE_54; num++;   break;
            //default: Rate = RATE_1;   break;
        }
        if (MaxDesire < Rate)  MaxDesire = Rate;
    }

    // Auto rate switching is enabled only if more than one DESIRED RATES are 
    // specified; otherwise disabled
    if (num <= 1)
    {       
        OPSTATUS_CLEAR_FLAG(pAd, fOP_STATUS_TX_RATE_SWITCH_ENABLED);
        pPort->CommonCfg.bAutoTxRateSwitch    = FALSE;
    }
    else
    {   
        OPSTATUS_SET_FLAG(pAd, fOP_STATUS_TX_RATE_SWITCH_ENABLED); 
        pPort->CommonCfg.bAutoTxRateSwitch    = TRUE;
    }

}

/*  
    ==========================================================================
    Description: 
        Decide P2P's Phy mode when operates as concurrent mode P2P+WLAN STA. 
        WLAN STA should have more priority to decide channel and bandwidth than P2P profile setting. 
        
    Parameters: 
         
    Note:
         
    ==========================================================================
 */
VOID P2pDecidePhyMode(
    IN PMP_ADAPTER pAd,
    IN PMP_PORT     pPort)
{
    if (pAd->OpMode == OPMODE_STA)
        return;

    // Init PhyMode as P2P_PHYMODE_ENABLE_11N_20

    if (INFRA_ON(pAd->PortList[PORT_0]))
    {
        pPort->P2PCfg.GroupChannel = pPort->Channel;
        if (pPort->CentralChannel != pPort->Channel)
        {
            // 11n 40MHz
            pPort->P2PCfg.P2pPhyMode = P2P_PHYMODE_ENABLE_11N_40;
            DBGPRINT(RT_DEBUG_TRACE, ("%s: Change to P2P_PHYMODE_ENABLE_11N_40\n", __FUNCTION__));
        }
        else if (pAd->StaActive.SupportedHtPhy.bHtEnable == FALSE)
        {
            // legacy
            pPort->P2PCfg.P2pPhyMode = P2P_PHYMODE_LEGACY_ONLY;
            DBGPRINT(RT_DEBUG_TRACE, ("%s: Change to P2P_PHYMODE_LEGACY_ONLY\n", __FUNCTION__));
        }
        else
        {
            // 11n 20MHz
            if (pPort->P2PCfg.P2pPhyMode != P2P_PHYMODE_ENABLE_11N_20)
            {
                pPort->P2PCfg.P2pPhyMode = P2P_PHYMODE_ENABLE_11N_20;
                DBGPRINT(RT_DEBUG_TRACE, ("%s: Change to P2P_PHYMODE_ENABLE_11N_20\n", __FUNCTION__));
            }   
        }
        DBGPRINT(RT_DEBUG_TRACE,("P2pDecidePhyMode===> Cha = %d, CenCha = %d  \n", pPort->Channel, pPort->CentralChannel)); 
    }       
}

/*  
    ==========================================================================
    Description: 
        Decide P2P's Bandwidth when operates as concurrent mode P2P+WLAN STA. 
        WLAN STA should have more priority to decide channel and bandwidth than P2P profile setting. 
        
    Parameters: 
         
    Note:
         
    ==========================================================================
 */
BOOLEAN P2pDecideHtBw(
    IN PMP_ADAPTER    pAd,
    IN PMP_PORT       pPort)
{
    if (IS_P2P_CON_CLI(pAd, pPort) || IS_P2P_MS_CLI(pAd, pPort) || IS_P2P_MS_CLI2(pAd, pPort))
    {
    }
    else
    {
            return TRUE;
    }
    
    // Case 2: WlanSta network(Port0) is earlier than P2P network(Port2)
    // If InfraSta(Port0) has formed the network before, P2pClient(Port2) should follow it.
    if (INFRA_ON(pAd->PortList[PORT_0]) && (pAd->PortList[PORT_0]->PortSubtype == PORTSUBTYPE_STA))
    {
        // Check if Port2 can connect as 40MHz
        if (pAd->MlmeAux.HtCapability.HtCapInfo.ChannelWidth == BW_40)
        {    
            if (/*(pPort->ScaningChannel == pPort->Channel) &&*/ 
                (pPort->Channel == pPort->CentralChannel))
            {
                pAd->MlmeAux.HtCapability.HtCapInfo.ChannelWidth = BW_20;
                //pPort->CentralChannel = pPort->CentralChannel;
                pPort->P2PCfg.P2pPhyMode = P2P_PHYMODE_ENABLE_11N_20;
                DBGPRINT(RT_DEBUG_TRACE, ("P2pDecideHtBw: P2pClient keeps 20MHz as the same as InfraSta\n"));
            }
            // P2pPhyMode has ever been decided when call P2pDecidePhyMode()
            else if((pPort->P2PCfg.P2pPhyMode == P2P_PHYMODE_ENABLE_11N_40) /* && 
                   (pPort->ScaningChannel == pPort->Channel) && 
                    (pAd->MlmeAux.CentralChannel == pPort->CentralChannel)*/)
            {
                DBGPRINT(RT_DEBUG_TRACE, ("P2pDecideHtBw: P2pClient keeps 40MHz as the same as InfraSta (CentralChannel=%d)\n", pPort->CentralChannel));
            }
            // P2pPhyMode has ever been decided when call P2pDecidePhyMode()
            else if((pPort->P2PCfg.P2pPhyMode == P2P_PHYMODE_ENABLE_11N_40) /* && 
                   (pPort->ScaningChannel == pPort->Channel) && 
                    (pAd->MlmeAux.CentralChannel != pPort->CentralChannel)*/)
            {
                DBGPRINT(RT_DEBUG_TRACE, ("P2pDecideHtBw: Check Failed !!! P2pClient and InfraSta use different CentralChannels in 40MHz (CentralChannel= %d  %d)\n", pPort->CentralChannel, pPort->CentralChannel));
                return FALSE;
            }
        }
        // Check if Port2 can connect as 20MHz
        else if ((pAd->MlmeAux.HtCapability.HtCapInfo.ChannelWidth == BW_20) &&
                (pPort->Channel != pPort->CentralChannel))
        {
            DBGPRINT(RT_DEBUG_TRACE, ("P2pDecideHtBw: Check Failed !!! P2pClient try to use 20MHz, but InfraSta contected to 40MHz AP\n"));
            return FALSE;
        }
            
        DBGPRINT(RT_DEBUG_TRACE,("P2pDecideHtBw===> MlmeAux Bw = %d, Cha = %d, CenCha = %d  \n", 
            pAd->MlmeAux.HtCapability.HtCapInfo.ChannelWidth,
            pPort->Channel,
            pPort->CentralChannel));
        
    }

    return TRUE;
}


/*  
    ==========================================================================
    Description: 
        Called from OID_CONNECT_REQUEST
        
    Parameters: 
    Note:
         
    ==========================================================================
 */
VOID  P2pConnectRequest(
    IN PMP_ADAPTER pAd,
    IN PMP_PORT      pPort) 
{
    //PMP_PORT      pPort = pAd->PortList[pPort->P2PCfg.PortNumber];
    
    if (P2P_OFF(pPort))
    {
        DBGPRINT(RT_DEBUG_TRACE, ("P2pConnectRequest return. P2P is OFF  !!! \n"));
        return;
    }

    DBGPRINT(RT_DEBUG_TRACE, ("%s:  (PortCfg.DesiredSSID. = %c%c%c%c%c%c%c%c%c...).\n", 
        __FUNCTION__, pPort->PortCfg.DesiredSSID.ucSSID[0], 
        pPort->PortCfg.DesiredSSID.ucSSID[1], pPort->PortCfg.DesiredSSID.ucSSID[2],
        pPort->PortCfg.DesiredSSID.ucSSID[3], pPort->PortCfg.DesiredSSID.ucSSID[4], 
        pPort->PortCfg.DesiredSSID.ucSSID[5], pPort->PortCfg.DesiredSSID.ucSSID[6],
        pPort->PortCfg.DesiredSSID.ucSSID[7], pPort->PortCfg.DesiredSSID.ucSSID[8]));
    DBGPRINT(RT_DEBUG_TRACE, ("PortCfg.DesiredSSID.uSSIDLength = %d  !!! \n", pPort->PortCfg.DesiredSSID.uSSIDLength));
    if (PlatformEqualMemory(pPort->P2PCfg.SSID, pPort->PortCfg.DesiredSSID.ucSSID, pPort->P2PCfg.SSIDLen))
    {
        //This connect request from OID means I should go to Client_Assoc_Auth state.
        // Change state when there is a Connect request OID.
        if (pPort->P2PCfg.P2PConnectState == P2P_DO_WPS_ENROLLEE || pPort->P2PCfg.P2PConnectState == P2P_DO_WPS_ENROLLEE_DONE
            || (pPort->PortCfg.DesiredSSID.uSSIDLength >= 9))
        {
            pPort->P2PCfg.P2PConnectState = P2P_I_AM_CLIENT_ASSOC_AUTH;
            // Reset this counter to zero when I first become
            pPort->P2PCfg.P2pCounter.Wpa2pskCounter100ms = 0; 
        }

        P2PPrintState(pAd);
    }
}

/*  
    ==========================================================================
    Description: 
        If my p2p group is forced to change channel in the middle..  I try to resume my group.
        Only need to do so when I am GO.
        
    Parameters: 
    Note:
         
    ==========================================================================
 */
VOID P2PResumeGroup(
    IN PMP_ADAPTER pAd,
    IN  PMP_PORT pPort,   
    IN UCHAR        MyRule,
    IN BOOLEAN      AutoSelect) 
{
    BOOLEAN     bAtleastOneDevice = FALSE;
    PMAC_TABLE_ENTRY pMacEntry = NULL;
    PMAC_TABLE_ENTRY pNextMacEntry = NULL;  
    PQUEUE_HEADER pHeader;
    DBGPRINT(RT_DEBUG_TRACE, ("P2PResumeGroup. !!!MyRule = %s \n", decodeP2PRule(MyRule)));
    DBGPRINT(RT_DEBUG_TRACE, ("GroupOpChannel = %d . Channel = %d !!! \n", pPort->P2PCfg.GroupOpChannel, pPort->Channel));
    if (MyRule == P2P_IS_GO)
    {
        if ((pPort->P2PCfg.GroupOpChannel != pPort->Channel) ||
            ((pPort->SoftAP.ApCfg.HtCapability.HtCapInfo.ChannelWidth == BW_40) &&
            (pPort->P2PCfg.CentralChannel != pPort->CentralChannel)))
        {
            /*if (pPort->P2PCfg.P2pManagedParm.ICSStatus == ICS_STATUS_DISABLED)
            {
                if (P2pSetEvent(pAd, pPort,  P2PEVENT_DISABLE_DHCP_SERVER) == FALSE)
                    pPort->P2PCfg.P2pEventQueue.bDisableDhcp = TRUE;
            }*/

#if(COMPILE_WIN8_ABOVE(CURRENT_OS_NDIS_VER))  
            if (NDIS_WIN8_ABOVE(pAd) && (pPort->PortType != WFD_GO_PORT))
            {
                // do nothing
            }
            else            
#endif
            {
                pPort->P2PCfg.P2pCapability[1] &= (~GRPCAP_OWNER);
                pPort->P2PCfg.P2pCapability[1] &= (~GRPCAP_GROUP_FORMING);
                pPort->P2PCfg.P2pCapability[1] &= (~GRPCAP_LIMIT);
            }

            // Auto-resume with the previous member
            if (AutoSelect == TRUE)
            {
                // Copy current P2P group memter to my connecting lists . So I can try to 
                // reconnect them when I like to reconstruct my group.
                pHeader = &pPort->MacTab.MacTabList;
                pNextMacEntry = (PMAC_TABLE_ENTRY)(pHeader->Head);
                while (pNextMacEntry != NULL)
                {
                    pMacEntry = (PMAC_TABLE_ENTRY)pNextMacEntry;
                    
                    if(pMacEntry == NULL)
                    {
                        break;
                    }
                    
                    if((pMacEntry->WlanIdxRole == ROLE_WLANIDX_BSSID) ||(pMacEntry->WlanIdxRole == ROLE_WLANIDX_MBCAST))
                    {
                        pNextMacEntry = pNextMacEntry->Next;   
                        pMacEntry = NULL;
                        continue; 
                    }
                    
                    if (pMacEntry->ValidAsP2P == TRUE)
                    {
                        // ======================================>
                        // The 4 lines are standard connect command.
                        PlatformZeroMemory(pPort->P2PCfg.ConnectingMAC, MAX_P2P_GROUP_SIZE*6);
                        PlatformMoveMemory(&pPort->P2PCfg.ConnectingMAC[0][0], pMacEntry->P2pInfo.DevAddr, MAC_ADDR_LEN);
                        bAtleastOneDevice = TRUE;
                        break;
                        // <======================================
                    }
                    
                    pNextMacEntry = pNextMacEntry->Next;   
                    pMacEntry = NULL;
                }
            }
            MacTableReset(pAd, pPort->PortNumber);
            P2pStopGo(pAd, pPort);
            pPort->P2PCfg.P2PConnectState = P2P_CONNECT_IDLE;
            pPort->P2PCfg.GroupOpChannel = pPort->Channel;
            pPort->P2PCfg.GroupChannel = pPort->Channel;
            // Send Channel Switch Announcement IE after link up .
            // to make sure the second channel
    //          SendChannelSwitchAction(pAd, pAd->PortList[pPort->P2PCfg.PortNumber], pPort->Channel, pPort->CentralChannel);
            P2pSetDefaultGOHt(pAd, pPort);
            if (IS_P2P_AUTOGO(pPort))
            {
                P2pStartAutoGo(pAd, pPort);
            }
        }
    }
    else if (MyRule == P2P_IS_CLIENT)
    {
        // don't need to add for this case yet.
    }
    
    if (bAtleastOneDevice == TRUE)
    {
        // Set to 0xff means to connect to first Connecting MAC
        pPort->P2PCfg.ConnectingIndex = 0xff;
        P2pConnect(pAd);
    }
    P2PPrintState(pAd);
}

/*  
    ==========================================================================
    Description: 
        Start autonomous GO function/
        
    Parameters: 
    Note:
         
    ==========================================================================
 */
VOID P2pStartAutoGo(
    IN PMP_ADAPTER pAd,
    IN PMP_PORT     pPort
    ) 
{
    DBGPRINT(RT_DEBUG_TRACE, ("P2pStartAutoGo. Port = %d, SsidLen = %d. )\n", pPort->P2PCfg.PortNumber, pPort->P2PCfg.SSIDLen));
    // Autonomously GO mode.
    if (pPort->P2PCfg.SSIDLen > 0)
    {
        PlatformMoveMemory(pAd->PortList[pPort->P2PCfg.PortNumber]->PortCfg.Ssid, pPort->P2PCfg.SSID, 32);
        pAd->PortList[pPort->P2PCfg.PortNumber]->PortCfg.SsidLen = pPort->P2PCfg.SSIDLen;
    }
    pPort->P2PCfg.P2PConnectState = P2P_ANY_IN_FORMATION_AS_GO;
    if((pPort->Channel != pPort->P2PCfg.GroupChannel) && !(INFRA_ON(pAd->PortList[PORT_0])))
    {
            pPort->P2PCfg.GroupOpChannel = pPort->P2PCfg.GroupChannel;
        pPort->Channel = pPort->P2PCfg.GroupChannel;
        pPort->P2PCfg.ListenChannel = pPort->P2PCfg.GroupChannel;
    }
    else
        pPort->P2PCfg.GroupOpChannel = pPort->P2PCfg.GroupChannel;
    // Set to 16 to distinguish that I am Autonomous GO. Don't need time out GO.
    pPort->P2PCfg.GoIntentIdx = 16;
    P2pGoNegoDone(pAd, pAd->PortList[pPort->P2PCfg.PortNumber],  NULL);
    pPort->P2PCfg.P2pCapability[1] &= (~GRPCAP_GROUP_FORMING);
    pPort->P2PCfg.P2pCapability[1] |= (GRPCAP_OWNER);
    if ((pPort->P2PCfg.P2pManagedParm.ICSStatus == ICS_STATUS_ENABLED)
        && (pAd->OpMode == OPMODE_STAP2P))
    {
        pPort->P2PCfg.P2pCapability[1] |= (GRPCAP_CROSS_CONNECT);
    }
    else
    {
        pPort->P2PCfg.P2pCapability[1] &= ~(GRPCAP_CROSS_CONNECT);
    }
    pAd->PortList[pPort->P2PCfg.PortNumber]->PortSubtype = PORTSUBTYPE_P2PGO;
    P2pDefaultListenChannel(pAd, pPort);
    DBGPRINT(RT_DEBUG_TRACE, ("GroupOpChannel = %d. )\n", pPort->P2PCfg.GroupOpChannel ));

    // Always set to configured mode in autonomous GO
    pPort->P2PCfg.bConfiguredAP = WSC_APSTATE_CONFIGURED; 
    
    if (IS_PERSISTENT_ON(pAd))
    {
        pPort->P2PCfg.P2pCapability[0] |= (DEVCAP_INVITE);
        pPort->P2PCfg.P2pCapability[1] |= (GRPCAP_PERSISTENT_RECONNECT);
    }
        
    P2pUpdateBeaconP2pCap(pAd, pPort->P2PCfg.P2pCapability);
    // Stop P2P search phase.
    pPort->P2PCfg.P2pCounter.CounterAftrScanButton = P2P_SCAN_PERIOD;
    pPort->P2PCfg.P2PDiscoProvState = P2P_ENABLE_LISTEN_ONLY;
    // Let GUI knows I become GO now.  So it will enable the WPS and PassPhrase setting icon on GUI.
    pPort->P2PCfg.P2pEventQueue.bP2pTopologyUpdate = TRUE;
    DBGPRINT(RT_DEBUG_TRACE, ("Set::  = %s. )\n", decodeP2PState(pPort->P2PCfg.P2PConnectState)));
}

/*  
    ==========================================================================
    Description: 
        GO Start function/
        
    Parameters: 
    Note:
         
    ==========================================================================
 */
VOID P2pStartGo(
    IN PMP_ADAPTER pAd,
    IN  PMP_PORT pPort) 
{
    ULONG       Value;
    BOOLEAN     Cancelled;

    // Use my addr as bssid and device address
    PlatformMoveMemory(pPort->PortCfg.Bssid, pPort->CurrentAddress, MAC_ADDR_LEN);
    PlatformMoveMemory(pPort->P2PCfg.Bssid, pPort->CurrentAddress, MAC_ADDR_LEN);
    PlatformMoveMemory(pPort->P2PCfg.GroupDevAddr, pPort->CurrentAddress, MAC_ADDR_LEN);
    pPort->PortCfg.AuthMode = Ralink802_11AuthModeWPA2PSK;
    pPort->PortCfg.CipherAlg = CIPHER_AES;
    pPort->PortCfg.WepStatus = DOT11_CIPHER_ALGO_CCMP;
    pPort->PortCfg.OrigWepStatus = DOT11_CIPHER_ALGO_CCMP;
    pPort->PortCfg.PairCipher    = DOT11_CIPHER_ALGO_CCMP;
    pPort->PortCfg.GroupCipher   = DOT11_CIPHER_ALGO_CCMP;
    pPort->PortCfg.MixedModeGroupCipher = Cipher_Type_NONE;
    pPort->SharedKey[BSS0][0].CipherAlg = CIPHER_AES;
    pPort->SharedKey[BSS0][1].CipherAlg = CIPHER_AES;
    pPort->SharedKey[BSS0][2].CipherAlg = CIPHER_AES;
    pPort->SharedKey[BSS0][3].CipherAlg = CIPHER_AES;
    pPort->PortCfg.DefaultKeyId = 1;
    pPort->StaCfg.WscControl.WscConfMode = WSC_ConfMode_REGISTRAR;
    pPort->P2PCfg.P2pCapability[1] |= GRPCAP_OWNER;

    // make sure Radio is On
    if(MT_TEST_FLAG(pAd, fRTMP_ADAPTER_IDLE_RADIO_OFF))
        AsicRadioOn(pAd);

    P2pSetGOTxRateSwitch(pAd, pPort);
    P2pSetDefaultGOHt(pAd, pPort);

    if (MT_TEST_BIT(pPort->P2PCfg.P2pManagedParm.APP2pManageability, P2PMANAGED_COEXIST_OPT_BIT))
        P2pEdcaDefault(pAd, pPort);

    pPort->PortSubtype = PORTSUBTYPE_P2PGO;
    PlatformMoveMemory(pPort->StaCfg.WscControl.WscProfile.Profile[0].SSID.Ssid, pPort->P2PCfg.SSID, 32);
    pPort->StaCfg.WscControl.WscProfile.Profile[0].SSID.SsidLength = pPort->P2PCfg.SSIDLen;
    P2pSetEvent(pAd, pPort,  P2PEVENT_DRIVER_INTERNAL_USE);

    pAd->MlmeAux.AutoReconnectSsidLen= 32;
    pAd->MlmeAux.AutoReconnectStatus = FALSE;
    PlatformZeroMemory(pAd->MlmeAux.AutoReconnectSsid, pAd->MlmeAux.AutoReconnectSsidLen);
    P2pStopScan(pAd, pPort);
    P2pGoSwitchChannel(pAd, pPort);
    // I am SoftAP
    if (pAd->OpMode == OPMODE_STA)
    {
        MtAsicSetBssid(pAd, pPort->PortCfg.Bssid, (UINT8)pPort->PortNumber);
    }

#if UAPSD_AP_SUPPORT    
    pAd->UAPSD.UapsdSWQBitmap = 0;
#endif

    RTUSBWriteMACRegister(pAd, RX_FILTR_CFG, APNORMAL);     // enable RX of DMA block
    pPort->bBeaconing = TRUE;
    MlmeUpdateBeacon(pAd);

    MtAsicEnableBssSync(pPort->pAd, pPort->CommonCfg.BeaconPeriod);
    // Pre-tbtt interrupt setting.
    RTUSBReadMACRegister(pAd, INT_TIMER_CFG, &Value);
    Value &= 0xffff0000;
    Value |= 6 << 4; // Pre-TBTT is 6ms before TBTT interrupt. 1~10 ms is reasonable.
    RTUSBWriteMACRegister(pAd, INT_TIMER_CFG, Value);
    // Enable pre-tbtt interrupt
    RTUSBReadMACRegister(pAd, INT_TIMER_EN, &Value);
    Value |=0x1;
    RTUSBWriteMACRegister(pAd, INT_TIMER_EN, Value);

    // consistent with current operation channel
    if (IS_SOCIAL_CHANNEL(pPort->Channel))
        pPort->P2PCfg.ListenChannel = pPort->Channel;

    // Init BBP R66 on GO
    // 
    if (pAd->HwCfg.LatchRfRegs.Channel <= 14)
    {
    }
    else
    {  
        if (pPort->BBPCurrentBW == BW_20)
        {
            RTUSBWriteBBPRegister(pAd, BBP_R66, 0x40);
        }
        else
        {
            RTUSBWriteBBPRegister(pAd, BBP_R66, 0x48);
        }
    }

    DBGPRINT(RT_DEBUG_TRACE, ("--->P2pStartGo  Channel = %d. CentralChannel = %d.\n", pPort->Channel, pPort->CentralChannel));
    DBGPRINT(RT_DEBUG_TRACE, ("!!GroupOpChannel = %d.  PortSubtype = %d. \n", pPort->P2PCfg.GroupOpChannel, pAd->PortList[pPort->P2PCfg.PortNumber]->PortSubtype));
    PlatformCancelTimer(&pPort->P2PCfg.P2pStopWpsTimer, &Cancelled);
        
    ApSetState(pPort, AP_STATE_STARTED);
    // When GoIntentIdx is below 16, it means it's a GO after Group Forming.  So set a timeout value. 
    // Auto GO doesn't need to timeout in current design. 
    if (IS_P2P_NOT_AUTOGO(pPort))
        PlatformSetTimer(pPort, &pPort->P2PCfg.P2pStopGoTimer, P2P_CHECK_GO_TIMER);

    // like bAPStart, maintain the coexistence between AP and Client.
    pPort->P2PCfg.bGOStart = TRUE;

}

/*  
    ==========================================================================
    Description: 
        Go Stop function timer to check whether to stop GO.
        [Win8] Called by peer disassociation. we should maintain GO's connected status here.
        
    Parameters: 
    Note:
         
    ==========================================================================
 */
VOID P2PStopGoTimerCallback(
    IN PVOID    SystemSpecific1, 
    IN PVOID    FunctionContext, 
    IN PVOID    SystemSpecific2, 
    IN PVOID    SystemSpecific3) 
{
    PMP_PORT pPort = (PMP_PORT )FunctionContext;
    PMP_ADAPTER  pAd = pPort->pAd;
    UCHAR       ConnectedNum = 0;
    UCHAR       i;
    BOOLEAN     bReset = FALSE;
    BOOLEAN         bConnected = FALSE, bSecured = FALSE;
    PMAC_TABLE_ENTRY pMacEntry = NULL;
    PMAC_TABLE_ENTRY pNextMacEntry = NULL;  

    DBGPRINT(RT_DEBUG_TRACE, ("%s:  P2PClientNumber = %d. MacTab Size = %d. \n", __FUNCTION__, pAd->pP2pCtrll->P2PTable.ClientNumber, pPort->MacTab.Size));  
    DBGPRINT(RT_DEBUG_TRACE, ("%s:  \n", decodeP2PState(pPort->P2PCfg.P2PConnectState))); 

    // Assume we only support ONE group owner at the same time
    // 1. MS-GO: Just clear local connected status and update group capability. And then NDIS will set infoReset later to stop MS-GO.
    // 2. Ralink-GO: Stop GO's all actions
    
#if(COMPILE_WIN8_ABOVE(CURRENT_OS_NDIS_VER))  
    if (NDIS_WIN8_ABOVE(pAd) &&
        (IS_P2P_MS_GO(pAd, pAd->PortList[pPort->P2PCfg.GOPortNumber])))
    {
        PQUEUE_HEADER pHeader;
        // Try to search all Mac Table to find out how many clients connect me.
        // After that, update connected status  with new outcome.
        pHeader = &pPort->MacTab.MacTabList;
        pNextMacEntry = (PMAC_TABLE_ENTRY)(pHeader->Head);
        while (pNextMacEntry != NULL)
        {
            pMacEntry = (PMAC_TABLE_ENTRY)pNextMacEntry;
            
            if(pMacEntry == NULL)
            {
                break;
            }
            
            if((pMacEntry->WlanIdxRole == ROLE_WLANIDX_BSSID) ||(pMacEntry->WlanIdxRole == ROLE_WLANIDX_MBCAST))
            {
                pNextMacEntry = pNextMacEntry->Next;   
                pMacEntry = NULL;
                continue; 
            }

            if (pMacEntry->ValidAsCLI == FALSE)
            {
                pNextMacEntry = pNextMacEntry->Next;   
                pMacEntry = NULL;
                continue; 
            }

            if ((pMacEntry->P2pInfo.P2pClientState == P2PSTATE_NONP2P_PSK)
                || (pMacEntry->P2pInfo.P2pClientState == P2PSTATE_NONP2P_WPS)
                || (pMacEntry->P2pInfo.P2pClientState == P2PSTATE_CLIENT_ASSOC && pMacEntry->ValidAsP2P == TRUE)
                || (pMacEntry->P2pInfo.P2pClientState == P2PSTATE_CLIENT_OPERATING && pMacEntry->ValidAsP2P == TRUE)
                || (pMacEntry->P2pInfo.P2pClientState == P2PSTATE_CLIENT_WPS && pMacEntry->ValidAsP2P == TRUE))
            {
                bConnected = TRUE;

                if (pMacEntry->PortSecured == WPA_802_1X_PORT_SECURED)
                {
                    bSecured = TRUE;
                }
            }

            pNextMacEntry = pNextMacEntry->Next;   
            pMacEntry = NULL;
        }

        if (bSecured == FALSE)
            MT_CLEAR_BIT(pAd->pP2pCtrll->P2pMsConnectedStatus, P2pMs_CONNECTED_STATUS_GO_OP);
        if (bConnected == FALSE)
            MT_CLEAR_BIT(pAd->pP2pCtrll->P2pMsConnectedStatus, P2pMs_CONNECTED_STATUS_GO_ASSOC);

        // Update group limit bit on GO port
        P2pMsUpdateGroupLimitCap(pAd, pPort);

        DBGPRINT(RT_DEBUG_TRACE, ("%s:  P2pMsConnectedStatus = %d.\n", __FUNCTION__, pAd->pP2pCtrll->P2pMsConnectedStatus));    
    }
#endif

    // try to search all P2P table. if NONE is connected, should stop GO.
    // Don't care legacy client that use WPA2PSK to connect to GO. If no P2P device connected to GO.
    // Still stop GO no matter how many legacy clients still use WPA2PSK to connect to me.
    for (i = 0 ; i< MAX_P2P_GROUP_SIZE;i++)
    {
        if ((pAd->pP2pCtrll->P2PTable.Client[i].P2pClientState == P2PSTATE_CLIENT_OPERATING)
            || (pAd->pP2pCtrll->P2PTable.Client[i].P2pClientState == P2PSTATE_NONP2P_PSK)
            || (pAd->pP2pCtrll->P2PTable.Client[i].P2pClientState == P2PSTATE_NONP2P_WPS))
        {
            ConnectedNum++;
            DBGPRINT(RT_DEBUG_TRACE, ("[%s]: Connected Peer \n", __FUNCTION__));    
            P2PPrintP2PEntry(pAd,i);
        }
        // Because in GoPeerDisassoc(), set Client State to CLIENT_WPS. So now check if it still stay in this state,
        // We can consider to remove it to allow this peer to reconnect.
        if (pAd->pP2pCtrll->P2PTable.Client[i].P2pClientState == P2PSTATE_CLIENT_WPS)
        {
            P2pGroupTabDelete(pAd, i, pAd->pP2pCtrll->P2PTable.Client[i].addr);            
        }
        P2P_CLEAR_FLAG(&pAd->pP2pCtrll->P2PTable.Client[i], P2PFLAG_PASSED_NEGO_REQ);
        P2P_CLEAR_FLAG(&pAd->pP2pCtrll->P2PTable.Client[i], P2PFLAG_GO_INVITE_ME);
    }

#if(COMPILE_WIN8_ABOVE(CURRENT_OS_NDIS_VER))  
    if (NDIS_WIN8_ABOVE(pAd) &&
        (IS_P2P_MS_GO(pAd, pAd->PortList[pPort->P2PCfg.GOPortNumber])))
    {// do nothing
    }
    else
#endif
    {
        ConnectedNum += pPort->MacTab.Size;
    
        if (ConnectedNum == 0)
        {
            DBGPRINT(RT_DEBUG_TRACE, ("ConnectedNum = %d. Stop GO function \n", ConnectedNum)); 
            bReset = TRUE;
        }
        else if ((ConnectedNum == 1) && (INFRA_ON(pAd->PortList[PORT_0])) && (pAd->OpMode == OPMODE_STAP2P))
        {
            DBGPRINT(RT_DEBUG_TRACE, ("ConnectedNum = %d. Stop GO function \n", ConnectedNum)); 
            bReset = TRUE;
        }

        if (bReset == TRUE)
        {
            // Stop GO
            MTEnqueueInternalCmd(pAd, pPort, MT_CMD_P2P_STOP_GO, NULL, 0);
        }
        else
        {
            // Keep to be GO. 
            pPort->P2PCfg.P2PConnectState = P2P_I_AM_GO_OP;
            P2pSetEvent(pAd, pPort,  P2PEVENT_P2PGROUP_TOPOLOGY_UPDATE);
        }

        PlatformZeroMemory( pPort->P2PCfg.PinCode, 8);
        PlatformZeroMemory( pPort->StaCfg.WscControl.RegData.PIN, 8);
        pPort->StaCfg.WscControl.RegData.PINLen = 0;
    }
}

/*  
    ==========================================================================
    Description: 
        GO Stopunction/
        
    Parameters: 
    Note:
         
    ==========================================================================
 */
VOID P2pStopGo(
    IN PMP_ADAPTER pAd,
    IN  PMP_PORT pPort) 
{
    ULONG       Value;
    UCHAR       i;
    BOOLEAN     Cancelled;
    
    if (pPort->PortSubtype != PORTSUBTYPE_P2PGO)
        return;
    
    DBGPRINT(RT_DEBUG_TRACE, ("P2P P2pStopGo==> \n"));

    pPort->P2PCfg.GONoASchedule.bNeedResumeNoA = FALSE;
    pPort->P2PCfg.P2PChannelState = P2P_DIFF_CHANNEL_OFF;
    // Can't reset BSSID when in concurrent mode. 
    if (pAd->OpMode == OPMODE_STA)
    {
        RTUSBWriteMACRegister(pAd, MAC_BSSID_DW0, 0);
        RTUSBWriteMACRegister(pAd, MAC_BSSID_DW1, 0);
    }
    pPort->P2PCfg.P2pCapability[1] &= (~(GRPCAP_GROUP_FORMING));
    pPort->P2PCfg.P2pCapability[1] &= (~(GRPCAP_LIMIT));
    pPort->SoftAP.ApCfg.bSoftAPReady = FALSE;

    // GO reset to un-configured OOB.
    pPort->P2PCfg.bConfiguredAP = WSC_APSTATE_UNCONFIGURED;   
    P2pUpdateBeaconP2pCap(pAd, pPort->P2PCfg.P2pCapability);
    if (IS_P2P_NOT_AUTOGO(pPort) && (!MT_TEST_FLAG(pAd, fRTMP_ADAPTER_NIC_NOT_EXIST)))
    {
        pPort->P2PCfg.P2pCapability[1] &= (~(GRPCAP_OWNER));
        pPort->bBeaconing = FALSE;
        MlmeUpdateBeacon(pAd);

        // Pre-tbtt interrupt setting.
        // Disable pre-tbtt interrupt
        RTUSBReadMACRegister(pAd, INT_TIMER_EN, &Value);
        Value &= 0xfffffffe;
        RTUSBWriteMACRegister(pAd, INT_TIMER_EN, Value);
        pPort->PortSubtype = PORTSUBTYPE_P2PClient;
        RTUSBWriteMACRegister(pAd, RX_FILTR_CFG, STANORMAL);     // enable RX of DMA block
    }
    else
    {
        pPort->PortSubtype = PORTSUBTYPE_P2PGO;
        DBGPRINT(RT_DEBUG_TRACE, ("P2P Auto Go on. Still send out beacon.==> \n"));
        DBGPRINT(RT_DEBUG_TRACE, ("P2P Auto Go on. portsubtye Still PORTSUBTYPE_P2PGO.==> \n"));
    }
        
    
    pPort->P2PCfg.MyIp = 0;
    pPort->P2PCfg.P2pEventQueue.bWpsNack = FALSE;
    pPort->P2PCfg.P2pEventQueue.bWpaDone = FALSE;
    pPort->P2PCfg.PhraseKeyChanged = FALSE;
    pPort->P2PCfg.P2pGOAllPsm = FALSE;

    PlatformCancelTimer(&pPort->P2PCfg.P2pStopWpsTimer, &Cancelled);
    PlatformCancelTimer(&pPort->P2PCfg.P2pStopGoTimer,  &Cancelled);
    PlatformCancelTimer(&pPort->Mlme.SwTbttTimer, &Cancelled);

    PlatformZeroMemory(pPort->PortCfg.Ssid, 32);
    // Reset Sigma Parameters
    P2PInitSigmaParam(pAd, pPort);

    P2pStopNoA(pAd, NULL);
    P2pStopOpPS(pAd, pPort);
    pPort->P2PCfg.Dpid = DEV_PASS_ID_NOSPEC;
    // Delete P2P Device that I previously tried to connect.
    for (i = 0;i < MAX_P2P_GROUP_SIZE;i++)
    {
        if (pAd->pP2pCtrll->P2PTable.Client[i].P2pClientState > P2PSTATE_DISCOVERY_UNKNOWN)
            P2pGroupTabDelete(pAd, i, pAd->pP2pCtrll->P2PTable.Client[i].addr);
    }

    P2PDefaultConfigM(pAd, pPort);
    if (IS_P2P_SIGMA_OFF(pPort))
    {
        P2pSetEvent(pAd, pPort,  P2PEVENT_P2PGROUP_TOPOLOGY_UPDATE);
    }
    pPort->P2PCfg.P2pEventQueue.bP2pTopologyUpdate = TRUE;

    // Reset bGOStart if GO disappeared.
    if (IS_P2P_NOT_AUTOGO(pPort) || (pPort->PortSubtype != PORTSUBTYPE_P2PGO))
    {
        pPort->P2PCfg.bGOStart = FALSE;
    }
    pPort->P2PCfg.ListenChannel = pPort->Channel;
}

/*  
    ==========================================================================
    Description: 
        P2P Client Stop function timer to check whether to stop P2P Client and go back to P2P Device that is in discovery phase. 
        
    Parameters: 
    Note:
         
    ==========================================================================
 */
VOID P2PStopClientTimerCallback(
    IN PVOID    SystemSpecific1, 
    IN PVOID    FunctionContext, 
    IN PVOID    SystemSpecific2, 
    IN PVOID    SystemSpecific3) 
{
    PMP_PORT pPort = (PMP_PORT )FunctionContext;
    PMP_ADAPTER  pAd = pPort->pAd;
    BOOLEAN     bReset = FALSE;
    
    DBGPRINT(RT_DEBUG_TRACE, ("P2PStopClientTimerCallback=> %s:  \n", decodeP2PState(pPort->P2PCfg.P2PConnectState)));    

    // Not use in MS WFD
    if (pPort->P2PCfg.P2PConnectState == P2P_CONNECT_NOUSE)
        return;
    
    if (!IS_P2P_CLIENT_OP(pPort) && !IS_P2P_GO_OP(pPort))
    {
        bReset = TRUE;
    }

    if (bReset == TRUE)
    {
        MTEnqueueInternalCmd(pAd, pPort, MT_CMD_P2P_STOP_CLIENT, NULL, 0);
    }
}

VOID P2PStopWpsTimerCallback(
    IN PVOID    SystemSpecific1, 
    IN PVOID    FunctionContext, 
    IN PVOID    SystemSpecific2, 
    IN PVOID    SystemSpecific3) 
{
    PMP_PORT pPort = (PMP_PORT )FunctionContext;
    PMP_ADAPTER  pAd = pPort->pAd;
    UCHAR       i;
    UCHAR       PortSecured = WPA_802_1X_PORT_NOT_SECURED;
    PMAC_TABLE_ENTRY pEntry = MlmeSyncMacTabMatchedRoleType(pPort, ROLE_WLANIDX_P2P_CLIENT);
    if (P2P_OFF(pPort))
        return;
        
    if(pEntry == NULL)
    {
        DBGPRINT(RT_DEBUG_TRACE,("%s(), Line:%d, Mac Entry is NULL\n", __FUNCTION__, __LINE__));
        return;
    }
    
    // Not use in MS WFD
    if (pPort->P2PCfg.P2PConnectState == P2P_CONNECT_NOUSE)
        return;
    
    DBGPRINT(RT_DEBUG_TRACE, ("P2PStopWpsTimerCallback -->   !\n"));
    pPort->P2PCfg.Dpid = DEV_PASS_ID_NOSPEC;
    pPort->StaCfg.WscControl.WscState = WSC_STATE_OFF;
    pPort->StaCfg.WscControl.WscStatus = STATUS_WSC_NOTUSED;  
    for (i = 0;i < MAX_P2P_GROUP_SIZE;i++)
    {   
        // Can't finish WPS. delete this peer.
        if (IS_P2P_PEER_PROVISIONING(&pAd->pP2pCtrll->P2PTable.Client[i]))
        {
            P2pGroupTabDelete(pAd, i, pAd->pP2pCtrll->P2PTable.Client[i].addr);
        }
    }


    if ((IS_P2P_CON_CLI(pAd, pAd->PortList[pPort->P2PCfg.PortNumber])) && (pEntry->ValidAsCLI))
        PortSecured = (UCHAR)pEntry->PortSecured;
    else
        PortSecured = pAd->StaCfg.PortSecured;
    
    if (!((IS_P2P_CLIENT_OP(pPort) && (PortSecured == WPA_802_1X_PORT_SECURED)) || IS_P2P_GO_OP(pPort)))
        P2pConnectStateUpdate(pAd, pPort);
    
    P2PDefaultConfigM(pAd, pPort);

}

/*  ==========================================================================
    Description:
        GoIndicateConnectStatus.
    Return:

    NOTE:
        Vista SoftaP
    ==========================================================================
 */
VOID GoIndicateConnectStatus(
    IN PMP_ADAPTER        pAd)
{
    DOT11_CONNECTION_START_PARAMETERS           connStart;
    DOT11_CONNECTION_COMPLETION_PARAMETERS      connComp;

    PMP_PORT pPort;
    pPort = pAd->PortList[PORT_0];

    //
    // Indicate connection start. If the connection status is not successful, 
    // use all-zeros as the BSSID in the connection start structure.
    //
    MP_ASSIGN_NDIS_OBJECT_HEADER(connStart.Header, 
                                 NDIS_OBJECT_TYPE_DEFAULT,
                                 DOT11_CONNECTION_START_PARAMETERS_REVISION_1,
                                 sizeof(DOT11_CONNECTION_START_PARAMETERS));
    connStart.BSSType = dot11_BSS_type_independent;
    PlatformMoveMemory(connStart.AdhocBSSID, pPort->SoftAP.ApCfg.AdhocBssid, sizeof(DOT11_MAC_ADDRESS));

    DBGPRINT(RT_DEBUG_TRACE, ("%s: GOGO BSSID = %02x:%02x:%02x:%02x:%02x:%02x\n",
        __FUNCTION__,  connStart.AdhocBSSID[0], connStart.AdhocBSSID[1],  connStart.AdhocBSSID[2], 
        connStart.AdhocBSSID[3], connStart.AdhocBSSID[4], connStart.AdhocBSSID[5]));

    PlatformMoveMemory(connStart.AdhocSSID.ucSSID, pPort->P2PCfg.SSID, pPort->P2PCfg.SSIDLen);
    connStart.AdhocSSID.uSSIDLength = pPort->P2PCfg.SSIDLen;
    DBGPRINT(RT_DEBUG_TRACE, ("%s: Len = %d. SSID = %02x:%02x:%02x:%02x:%02x:%02x:%x\n",
        __FUNCTION__,  connStart.AdhocSSID.uSSIDLength, connStart.AdhocSSID.ucSSID[0], connStart.AdhocSSID.ucSSID[1], connStart.AdhocSSID.ucSSID[2], 
        connStart.AdhocSSID.ucSSID[3], connStart.AdhocSSID.ucSSID[4], connStart.AdhocSSID.ucSSID[5], connStart.AdhocSSID.ucSSID[6]));

    PlatformIndicateDot11Status(pAd,
                        pPort,
                        NDIS_STATUS_DOT11_CONNECTION_START,
                        NULL,
                        &connStart,
                        sizeof(DOT11_CONNECTION_START_PARAMETERS));

    //
    // Indicate connection complete regardless of the connection status
    //
    MP_ASSIGN_NDIS_OBJECT_HEADER(connComp.Header, 
                                 NDIS_OBJECT_TYPE_DEFAULT,
                                 DOT11_CONNECTION_COMPLETION_PARAMETERS_REVISION_1,
                                 sizeof(DOT11_CONNECTION_COMPLETION_PARAMETERS));

    connComp.uStatus = DOT11_CONNECTION_STATUS_SUCCESS;

    PlatformIndicateDot11Status(pAd, 
                        pPort,
                        NDIS_STATUS_DOT11_CONNECTION_COMPLETION,
                        NULL,
                        &connComp,
                        sizeof(DOT11_CONNECTION_COMPLETION_PARAMETERS));
}

/* 
    ==========================================================================
    Description:
        GoIndicateAssociationStatus.
    Return:

    NOTE:
        Vista SoftaP
    ==========================================================================
*/
VOID GoIndicateAssociationStatus(
    IN PMP_ADAPTER    pAd,
    IN PUCHAR           pAddr,
    IN UCHAR            Macidx)
{
    DOT11_ASSOCIATION_START_PARAMETERS          assocStart;
    ULONG                                       assocCompSize;
    PDOT11_ASSOCIATION_COMPLETION_PARAMETERS    assocComp;
    PUCHAR              pTempPtr = NULL;
    UCHAR           i;
    PULONG          ptemp;
    
    //Vista only, using default port is OK
    PMP_PORT pPort = pAd->PortList[FXXK_PORT_0];
    PMAC_TABLE_ENTRY pWcidMacTabEntry = NULL;
    pWcidMacTabEntry = MlmeSyncMacTabMatchedWlanIdx(pPort, Macidx); 

    if(pWcidMacTabEntry == NULL)
    {
        DBGPRINT(RT_DEBUG_TRACE,("%s(), Line:%d, Mac Entry is NULL  Wcid = %d\n", __FUNCTION__, __LINE__, Macidx));
        return;
    }
    //
    // Allocate enough memory for ASSOCIATION_COMPLETE indication. If allocation fails,
    // we skip this beaconing station.
    //
    assocCompSize = sizeof(DOT11_ASSOCIATION_COMPLETION_PARAMETERS) +
                    pAd->pP2pCtrll->GOBeaconBufLen +   // for beacon
                    sizeof(ULONG);                  // for single entry PHY list

    PlatformAllocateMemory(pAd,  &assocComp, assocCompSize);
    if (assocComp == NULL)
    {
        DBGPRINT(RT_DEBUG_ERROR, ("APIndicateAssociationStatus, Allocate assocComp for indication failed\n"));
        return;
    }

    //
    // Indicate ASSOCIATION_START
    //
    MP_ASSIGN_NDIS_OBJECT_HEADER(assocStart.Header, 
                                NDIS_OBJECT_TYPE_DEFAULT,
                                DOT11_ASSOCIATION_START_PARAMETERS_REVISION_1,
                                sizeof(DOT11_ASSOCIATION_START_PARAMETERS));

    assocStart.uIHVDataOffset = 0;
    assocStart.uIHVDataSize = 0;

    pPort->PortCfg.DesiredSSID.uSSIDLength = pPort->P2PCfg.SSIDLen;
    PlatformMoveMemory(pPort->PortCfg.DesiredSSID.ucSSID, pPort->P2PCfg.SSID, 32);
    PlatformMoveMemory(&assocStart.SSID, &pPort->PortCfg.DesiredSSID, sizeof(DOT11_SSID));
    COPY_MAC_ADDR(&assocStart.MacAddr, pWcidMacTabEntry->Addr);
    DBGPRINT(RT_DEBUG_ERROR, ("GOIndicateAssociationStatus, [%d]MacAddr = %x %x %x %x %x %x \n", Macidx, pWcidMacTabEntry->Addr[0]
    ,pWcidMacTabEntry->Addr[1],pWcidMacTabEntry->Addr[2],pWcidMacTabEntry->Addr[3],pWcidMacTabEntry->Addr[4],pWcidMacTabEntry->Addr[5]));

    PlatformIndicateDot11Status(pAd,
                        pPort,
                        NDIS_STATUS_DOT11_ASSOCIATION_START,
                        NULL,
                        &assocStart,
                        sizeof(DOT11_ASSOCIATION_START_PARAMETERS));

    DBGPRINT(RT_DEBUG_TRACE, ("GOGO   IndicateAssociationStatus: NDIS_STATUS_DOT11_ASSOCIATION_START, MAC[%02x:%02x:%02x:%02x:%02x:%02x]\n",
                    pAddr[0], pAddr[1], pAddr[2],
                    pAddr[3], pAddr[4], pAddr[5]));
    //
    // Indicate ASSOCIATION_COMPLETE
    //
    MP_ASSIGN_NDIS_OBJECT_HEADER(assocComp->Header, 
                                NDIS_OBJECT_TYPE_DEFAULT,
                                DOT11_ASSOCIATION_COMPLETION_PARAMETERS_REVISION_1,
                                sizeof(DOT11_ASSOCIATION_COMPLETION_PARAMETERS));

    COPY_MAC_ADDR(&assocComp->MacAddr, pWcidMacTabEntry->Addr);
    assocComp->uStatus = 0;

    assocComp->bReAssocReq = FALSE;
    assocComp->bReAssocResp = FALSE;
    assocComp->uAssocReqOffset = 0;
    assocComp->uAssocReqSize = 0;
    assocComp->uAssocRespOffset = 0;
    assocComp->uAssocRespSize = 0;

    //
    // Append the beacon information of this beaconing station. 
    //
    pTempPtr = Add2Ptr(assocComp, sizeof(DOT11_ASSOCIATION_COMPLETION_PARAMETERS));
    PlatformMoveMemory(pTempPtr, pAd->pP2pCtrll->GOBeaconBuf, pAd->pP2pCtrll->GOBeaconBufLen);

    DBGPRINT_RAW(RT_DEBUG_TRACE, ("  Beacon :  Len = %d \n",pAd->pP2pCtrll->GOBeaconBufLen));
    for (i = 0; i< pAd->pP2pCtrll->GOBeaconBufLen; )
    {
        ptemp = (PULONG)&pAd->pP2pCtrll->GOBeaconBuf[i];
        DBGPRINT_RAW(RT_DEBUG_TRACE, ("%08x:: %08x:: %08x:: %08x:: %08x:: %08x:: %08x:: %08x:\n", *(ptemp), *(ptemp+1), *(ptemp+2), *(ptemp+3), *(ptemp+4), *(ptemp+5), *(ptemp+6), *(ptemp+7)));
        i += 32;
    }
    // Update TA on Beacon frame.
    assocComp->uBeaconOffset = sizeof(DOT11_ASSOCIATION_COMPLETION_PARAMETERS);
    assocComp->uBeaconSize = pAd->pP2pCtrll->GOBeaconBufLen;

    assocComp->uIHVDataOffset = 0;
    assocComp->uIHVDataSize = 0;

    //
    // Set the auth and cipher algorithm.
    //
    assocComp->AuthAlgo = DOT11_AUTH_ALGO_80211_OPEN;
    assocComp->UnicastCipher = DOT11_CIPHER_ALGO_NONE; 
    assocComp->MulticastCipher = DOT11_CIPHER_ALGO_NONE; 
    
    //
    // Set the PHY list. It just contains our active phy id.
    //
    assocComp->uActivePhyListOffset = sizeof(DOT11_ASSOCIATION_COMPLETION_PARAMETERS) + 
                                        pAd->pP2pCtrll->GOBeaconBufLen;
    assocComp->uActivePhyListSize = sizeof(ULONG);
    *((ULONG UNALIGNED *)Add2Ptr(assocComp, assocComp->uActivePhyListOffset)) = pAd->StaCfg.ActivePhyId;

    assocComp->bFourAddressSupported = FALSE;
    assocComp->bPortAuthorized = FALSE;
    assocComp->DSInfo = DOT11_DS_UNKNOWN;

    assocComp->uEncapTableOffset = 0;
    assocComp->uEncapTableSize = 0;

    PlatformIndicateDot11Status(pAd, 
                        pPort,
                        NDIS_STATUS_DOT11_ASSOCIATION_COMPLETION,
                        NULL,
                        assocComp,
                        assocCompSize);

    DBGPRINT(RT_DEBUG_TRACE, ("PlatformIndicateAdhocAssociation: NDIS_STATUS_DOT11_ASSOCIATION_COMPLETION, MAC[%02x:%02x:%02x:%02x:%02x:%02x] phy = %d.\n",
                    pAddr[0], pAddr[1], pAddr[2],
                    pAddr[3], pAddr[4], pAddr[5], pAd->StaCfg.ActivePhyId));
    //
    // Free the preallocated ASSOCIATION_COMPLETE indication structure.
    //
    PlatformFreeMemory(assocComp, assocCompSize);
}

VOID P2pEdcaDefault(
    IN PMP_ADAPTER pAd,
    IN  PMP_PORT pPort) 
{
    // EDCA parameters used for AP's own transmission
    pPort->SoftAP.ApCfg.BssEdcaParm.bValid = TRUE;
    pPort->SoftAP.ApCfg.BssEdcaParm.Aifsn[0] = 3;
    pPort->SoftAP.ApCfg.BssEdcaParm.Aifsn[1] = 7;
    pPort->SoftAP.ApCfg.BssEdcaParm.Aifsn[2] = 2;
    pPort->SoftAP.ApCfg.BssEdcaParm.Aifsn[3] = 2;

    pPort->SoftAP.ApCfg.BssEdcaParm.Cwmin[0] = 4;
    pPort->SoftAP.ApCfg.BssEdcaParm.Cwmin[1] = 4;
    pPort->SoftAP.ApCfg.BssEdcaParm.Cwmin[2] = 3;
    pPort->SoftAP.ApCfg.BssEdcaParm.Cwmin[3] = 2;

    pPort->SoftAP.ApCfg.BssEdcaParm.Cwmax[0] = 10;
    pPort->SoftAP.ApCfg.BssEdcaParm.Cwmax[1] = 10;
    pPort->SoftAP.ApCfg.BssEdcaParm.Cwmax[2] = 4;
    pPort->SoftAP.ApCfg.BssEdcaParm.Cwmax[3] = 3;

    pPort->SoftAP.ApCfg.BssEdcaParm.Txop[0]  = 0;
    pPort->SoftAP.ApCfg.BssEdcaParm.Txop[1]  = 0;
    pPort->SoftAP.ApCfg.BssEdcaParm.Txop[2]  = AC2_DEF_TXOP;
    pPort->SoftAP.ApCfg.BssEdcaParm.Txop[3]  = AC3_DEF_TXOP;

    MtAsicSetEdcaParm(pAd, /*pAd->PortList[pPort->P2PCfg.PortNumber]*/pPort, &pPort->SoftAP.ApCfg.BssEdcaParm, TRUE);

}
/*  
    ==========================================================================
    Description: 
        Reset the P2P config Method to default initial value..
        
    Parameters: 
         
    Note:
         
    ==========================================================================
 */

VOID P2PDefaultConfigM(
    IN PMP_ADAPTER pAd,
    IN PMP_PORT     pPort)
{
    if (pPort->CommonCfg.P2pControl.field.DefaultConfigMethod == P2P_REG_CM_DISPLAY)
    {
        pPort->P2PCfg.ConfigMethod = CONFIG_METHOD_DISPLAY;
        DBGPRINT(RT_DEBUG_TRACE,(" Display = %x \n", pPort->P2PCfg.ConfigMethod));
    }
    else if (pPort->CommonCfg.P2pControl.field.DefaultConfigMethod == P2P_REG_CM_KEYPAD)
    {
        pPort->P2PCfg.ConfigMethod = CONFIG_METHOD_KEYPAD;
        DBGPRINT(RT_DEBUG_TRACE,(" Keypad = %x \n", pPort->P2PCfg.ConfigMethod));
    }
    else if (pPort->CommonCfg.P2pControl.field.DefaultConfigMethod == P2P_REG_CM_LABEL)
    {
        pPort->P2PCfg.ConfigMethod = CONFIG_METHOD_LABEL;
        DBGPRINT(RT_DEBUG_TRACE,(" use Label = %x \n", pPort->P2PCfg.ConfigMethod));
    }
    else if (pPort->CommonCfg.P2pControl.field.DefaultConfigMethod == P2P_REG_CM_PBC)
    {
        pPort->P2PCfg.ConfigMethod = CONFIG_METHOD_PUSHBUTTON;
        DBGPRINT(RT_DEBUG_TRACE,("Default use PBC = %x \n", pPort->P2PCfg.ConfigMethod));
    }
    else
    {
        pPort->P2PCfg.ConfigMethod = (CONFIG_METHOD_DISPLAY | CONFIG_METHOD_PUSHBUTTON | CONFIG_METHOD_KEYPAD);
        DBGPRINT(RT_DEBUG_TRACE,("Default all support = %x \n", pPort->P2PCfg.ConfigMethod));
    }
}

/*  
    ==========================================================================
    Description: 
        If no infrastructure connect and not GO, set Channel and CentralChannel to P2P's listen channel. This can 
        let P2P device stays most of time in listen state.
        
    Parameters: 
         
    Note:
         
    ==========================================================================
 */
VOID P2pDefaultListenChannel(
    IN PMP_ADAPTER pAd,
    IN PMP_PORT      pPort)
{
    //PMP_PORT          pPort;
    PMP_PORT          pCheckPort = NULL;

    if (P2P_OFF(pPort))
        return;
    
    pPort = pAd->PortList[pPort->P2PCfg.PortNumber];

    // in concurrent mode, should check other port's connection status.
    if ((pAd->OpMode == OPMODE_STAP2P) && (PORTV2_P2P_ON(pAd, pPort)))
        pCheckPort = pAd->PortList[PORT_0];
    
    // I can set Channel and CentralChannel to any value. 
    // Because they are not "working channel" yet.
    // Set to Listen channel can improve the discovery time.
    if (((!pCheckPort) || (pCheckPort && IDLE_ON(pCheckPort))) && 
        (!IS_P2P_GO_OP(pPort)) && 
        (!IS_P2P_REGISTRA(pPort)) &&
        (!IS_P2P_GO_WPA2PSKING(pPort)) &&
        (pPort->P2PCfg.P2PConnectState != P2P_ANY_IN_FORMATION_AS_GO) &&
        (IS_P2P_NOT_AUTOGO(pPort)))
    {
        pPort->Channel = pPort->P2PCfg.ListenChannel;
        pPort->CentralChannel = pPort->P2PCfg.ListenChannel;
        // We want use 20MHz for the same Listen Channel. So set BW_20.
        pPort->BBPCurrentBW = BW_20;
        DBGPRINT(RT_DEBUG_TRACE, ("%s - ListenChannel becomes = %d. !!! \n", __FUNCTION__, pPort->P2PCfg.ListenChannel));
    }
    // Sometimes, I already finish Group forming and I become GO. But still has a P2P scan.
    // Then After scan, I should swtich to correct working channel.
    // TODO : still need to check 40MHz case. This is 20MHz temp solution.
    else if (((!pCheckPort) || (pCheckPort && IDLE_ON(pCheckPort))) &&
        ((pPort->PortSubtype == PORTSUBTYPE_P2PGO)
        ||(pPort->P2PCfg.P2PConnectState == P2P_ANY_IN_FORMATION_AS_GO)))
    {
    }
    DBGPRINT(RT_DEBUG_TRACE, ("%s - Now becomes CentralChannel = %d. Channel  = %d. !!! \n", __FUNCTION__, pPort->CentralChannel,pPort->Channel));

}

/*  
    ==========================================================================
    Description: 
        
    Parameters: 
         
    Note:
         
    ==========================================================================
 */
BOOLEAN P2pScanNextchannel(
    IN PMP_ADAPTER pAd)
{
    PMP_PORT pPort = pAd->PortList[FXXK_PORT_0];
    
    if ((pAd->MlmeAux.MaxScanChannelCnt % 4) == 0)  
    {
        if (pPort->BBPCurrentBW == BW_40)
        {
            BbSetTxRxBwCtrl(pAd, BW_40);
            //AsicSwitchChannel(pAd, pPort->CentralChannel,FALSE);
            DBGPRINT(RT_DEBUG_TRACE, ("SYNC - AP beacon, restore to  40MHz channel %d, Total BSS[%02d]\n",pPort->CentralChannel, pAd->ScanTab.BssNr));
        }
        else
        {
           // AsicSwitchChannel(pAd, pPort->Channel, FALSE);
            DBGPRINT(RT_DEBUG_TRACE, ("SYNC - AP beacon, restore to  20MHz channel %d, Total BSS[%02d]\n",pPort->Channel, pAd->ScanTab.BssNr));
        }           
        NdisSetEvent(&(GET_THREAD_EVENT(&pAd->pHifCfg->hDequeueDataThread)));
        PlatformSetTimer(pPort, &pPort->Mlme.ChannelTimer, RESTORE_CHANNEL_TIME/2);
        AsicResumeBssSync(pAd);
        
        return TRUE;
    }

    return FALSE;
}

/*  
    ==========================================================================
    Description: 
        Modify P2P's listen channel when in Concurrent Operation to get better performance. 
        Can be called from MlmeCntLinkUp and MlmeCntLinkDown;
        
    Parameters: 
         
    Note:
         
    ==========================================================================
 */
VOID P2pScanChannelDefault(
    IN PMP_ADAPTER pAd,
    IN PMP_PORT     pPort)
{
    // Default listen channel is based on spec, using Social channels 1, 6, 11.
    pPort->P2PCfg.P2pProprietary.ListenChanel[0] = 1;
    pPort->P2PCfg.P2pProprietary.ListenChanel[1] = 6;
    pPort->P2PCfg.P2pProprietary.ListenChanel[2] = 11;
    pPort->P2PCfg.P2pProprietary.ListenChanelIndex = 0;
    pPort->P2PCfg.P2pProprietary.ListenChanelCount = 3;
    pPort->P2PCfg.P2pCounter.ScanIntervalBias = P2P_RANDOM_BASE;

    DBGPRINT(RT_DEBUG_TRACE,("P2pScanChannelDefault <=== count = %d, Channels are %d, %d,%d separately   \n", 
        pPort->P2PCfg.P2pProprietary.ListenChanelCount, pPort->P2PCfg.P2pProprietary.ListenChanel[0], pPort->P2PCfg.P2pProprietary.ListenChanel[1], pPort->P2PCfg.P2pProprietary.ListenChanel[2]));
}

/*  
    ==========================================================================
    Description: 
        Modify P2P's scan channel list;
        
    Parameters: 
         
    Note:
         
    ==========================================================================
 */
VOID P2pScanChannelUpdate(
    IN PMP_ADAPTER pAd,
    IN PMP_PORT     pPort)
{
    UCHAR   chIdx;
    UCHAR   Index = 0;
    UCHAR   ChannelNumber = 0;
    UCHAR   ChannelIndex = 0;
    ULONG   ChannelBitmap = 0;
    
    switch (pPort->P2PCfg.DiscoverType & 0xf)
    {
        case P2P_DISCO_TYPE_SCAN:
#if(COMPILE_WIN8_ABOVE(CURRENT_OS_NDIS_VER))  
            //  Scaning on the specific channel, only for p2p quck scan
            if ((pPort->P2PCfg.ScanWithFilter != P2pMs_SCAN_FILTER_NULL) && (pPort->P2PCfg.ScanPeriod <= 10))
            {
                UCHAR DevIndexArray[6];
                INT i;
                
                DevIndexArray[0] = P2P_NOT_FOUND;
                DevIndexArray[1] = P2P_NOT_FOUND;
                DevIndexArray[2] = P2P_NOT_FOUND;
                DevIndexArray[3] = P2P_NOT_FOUND;
                DevIndexArray[4] = P2P_NOT_FOUND;
                DevIndexArray[5] = P2P_NOT_FOUND;

                DBGPRINT(RT_DEBUG_TRACE, ("%s:: (P2P_DISCO_TYPE_SCAN) state=%d, filter=0x%x,  mask=%d, num=%d, ssidlen=%d, GoIOpChannel= %d\n", 
                        __FUNCTION__, pPort->P2PCfg.P2pMsSatetPhase, pPort->P2PCfg.ScanWithFilter, 
                        pPort->P2PCfg.DeviceFilterList[0].ucBitmask, pPort->P2PCfg.uNumDeviceFilters,
                        pPort->P2PCfg.DeviceFilterList[0].GroupSSID.uSSIDLength,
                        pPort->P2PCfg.GoIOpChannel));

                //////////////////////////////////////////
                // Filter = Device ID
                // 
                if (MT_TEST_BIT(pPort->P2PCfg.ScanWithFilter, P2pMs_SCAN_FILTER_DEVICE_ID) &&
                    (pPort->P2PCfg.uNumDeviceFilters > 0) &&
                    ((pPort->P2PCfg.DeviceFilterList[0].ucBitmask == DISCOVERY_FILTER_BITMASK_DEVICE) ||
                    ((pPort->P2PCfg.DeviceFilterList[0].ucBitmask == DISCOVERY_FILTER_BITMASK_GO)))&&
                    (!MT_TEST_BIT(pPort->P2PCfg.ScanWithFilter, P2pMs_SCAN_FILTER_FORCE_LEGACY_NETWORK)))
                {

                    //
                    // Filter peer listen channels
                    //                  
                    for (Index  = 0; Index < pPort->P2PCfg.uNumDeviceFilters; Index++)
                    {
                        P2pDiscoTabSearch(pAd, 
                                            NULL, 
                                            Add2Ptr(pPort->P2PCfg.DeviceFilterList[Index].DeviceID, sizeof(DOT11_WFD_DISCOVER_DEVICE_FILTER) * Index), 
                                            DevIndexArray);

                        for (i = 0; i < sizeof(DevIndexArray); i++)
                        {
                            if (DevIndexArray[i] == P2P_NOT_FOUND)
                                break;

                            if ((DevIndexArray[i] < MAX_P2P_DISCOVERY_SIZE) && (pAd->pP2pCtrll->P2PTable.DiscoEntry[DevIndexArray[i]].bLegacyNetworks == FALSE))
                            {
                                // If it's GroupID bitmask, check SSID.
                                if ((pPort->P2PCfg.DeviceFilterList[Index].ucBitmask != DISCOVERY_FILTER_BITMASK_GO) ||
                                    (!SSID_EQUAL(pPort->P2PCfg.DeviceFilterList[Index].GroupSSID.ucSSID, 
                                                pPort->P2PCfg.DeviceFilterList[Index].GroupSSID.uSSIDLength, 
                                                pAd->pP2pCtrll->P2PTable.DiscoEntry[DevIndexArray[i]].Ssid, 
                                                pAd->pP2pCtrll->P2PTable.DiscoEntry[DevIndexArray[i]].SsidLen)))
                                {
                                    ChannelNumber++;
                                    ChannelBitmap |= (1 << (pAd->pP2pCtrll->P2PTable.DiscoEntry[DevIndexArray[i]].ListenChannel));
                                }
                                break;
                            }
                            
                            // TODO: Only one DeviceID in probe request for searching
                            if (Index == 0)
                                break;
                        }
                    }

                    // Before PD, there is no desired GO stored in my Discover Table.
                    // Use GOOpChannel instaed and sitesuvey again !
                    if ((pPort->P2PCfg.DeviceFilterList[0].ucBitmask == DISCOVERY_FILTER_BITMASK_GO) && 
                        (ChannelNumber == 0) && (ChannelBitmap == 0) && (MlmeSyncIsValidChannel(pAd, pPort->P2PCfg.GoIOpChannel)))
                    {
                        ChannelNumber = 1;
                        ChannelBitmap |= (1 << (pPort->P2PCfg.GoIOpChannel));
                    }
                    
                    DBGPRINT(RT_DEBUG_TRACE, ("%s::P2pMs Scan Filter(device ID), Index = %d, ChannelBitmap = 0x%x, ChannelNumber = %d\n", __FUNCTION__, Index, ChannelBitmap, ChannelNumber));
                }

            }
#endif          
            //
            // Build scan channel
            //  
            if ((ChannelNumber > 0) && (ChannelBitmap > 0))
            {
                for (Index  = 1; Index <= 11; Index++)
                {
                    if (ChannelBitmap & (1 << Index))
                    {
                        pPort->P2PCfg.P2pProprietary.ListenChanel[ChannelIndex] = Index;
                        DBGPRINT(RT_DEBUG_TRACE, ("%s::P2pMs Scan Filter(device ID) build scan array, ListenChanel[%d] = %d\n", __FUNCTION__, ChannelIndex, Index));
                        ChannelIndex++;
                    }
                    
                }
                
                pPort->P2PCfg.P2pProprietary.ListenChanelIndex = 0;
                pPort->P2PCfg.P2pProprietary.ListenChanelCount = ChannelIndex;    
            }
            else
            {
                //run a complete channel scan if not find the peer before
                pPort->P2PCfg.P2pProprietary.ListenChanelIndex = 0;
                //test WHQL force G band
                if(pAd->LogoTestCfg.OnTestingWHQL)
                {
                    UCHAR   WHQLChannelOrder[11] = {11, 8, 6, 3, 1, 7, 5, 4, 2, 9, 10};
                    
                    pPort->P2PCfg.P2pProprietary.ListenChanelCount =11;
                    for (chIdx = 0; chIdx < 11; chIdx++)
                    {
                        //pPort->P2PCfg.P2pProprietary.ListenChanel[chIdx] = (11 - chIdx );
                        pPort->P2PCfg.P2pProprietary.ListenChanel[chIdx] = WHQLChannelOrder[chIdx];
                        DBGPRINT(RT_DEBUG_TRACE, ("OnTestingWHQL P2P scan ch %d\n", pPort->P2PCfg.P2pProprietary.ListenChanel[chIdx]));
                    }
                }
                // If scan period is less than 1 sec, scan some of more important channels at first (default listen channel)
                else if ((pPort->P2PCfg.ScanPeriod < 10) && (pAd->HwCfg.ChannelListNum > 0) && (pAd->HwCfg.ChannelListNum <= 11) &&
                         ((pAd->HwCfg.ChannelList[0].Channel != P2P_DEFAULT_LISTEN_CHANNEL) || 
                         (pAd->HwCfg.ChannelList[pAd->HwCfg.ChannelListNum - 1].Channel == P2P_DEFAULT_LISTEN_CHANNEL)))
                {
                    // scan in anti-clockwise
                    pPort->P2PCfg.P2pProprietary.ListenChanelCount = pAd->HwCfg.ChannelListNum;
                    for (chIdx = 0; chIdx < pAd->HwCfg.ChannelListNum; chIdx++)
                    {
                        pPort->P2PCfg.P2pProprietary.ListenChanel[chIdx] = pAd->HwCfg.ChannelList[pAd->HwCfg.ChannelListNum - chIdx - 1].Channel;
                        DBGPRINT(RT_DEBUG_TRACE, ("Not OnTestingWHQL P2P scan ch in anti-clockwise %d \n", pPort->P2PCfg.P2pProprietary.ListenChanel[chIdx]));
                    }
                }
                else
                {
                    // todo A band would not scan all channels
                    pPort->P2PCfg.P2pProprietary.ListenChanelCount = pAd->HwCfg.ChannelListNum;
                    for (chIdx = 0; chIdx < pPort->P2PCfg.P2pProprietary.ListenChanelCount; chIdx++)
                    {
                        pPort->P2PCfg.P2pProprietary.ListenChanel[chIdx] = pAd->HwCfg.ChannelList[chIdx].Channel;
                        DBGPRINT(RT_DEBUG_TRACE, ("Not OnTestingWHQL P2P scan ch %d \n", pPort->P2PCfg.P2pProprietary.ListenChanel[chIdx]));
                    }
                }
            }
            break;

        case P2P_DISCO_TYPE_FIND:
            if (pPort->P2PCfg.P2pProprietary.ListenChanelCount != 3)
            {
                P2pScanChannelDefault(pAd, pPort);
            }
    
            break;

        case P2P_DISCO_TYPE_AUTO:
            //
            // Build scan channel
            //  
            
            //run a complete channel scan after 3 rounds of social channel scan in AUTO SCAN mode
            if (pPort->P2PCfg.P2pProprietary.ChannelScanRound % P2P_AUTO_SCAN_CYCLE == 0)
            {
                pPort->P2PCfg.P2pProprietary.ListenChanelIndex = 0;
                DBGPRINT(RT_DEBUG_TRACE,("[WFDDBG] SCAN AUTO/FORCED::Channel Count = %d OnTestingWHQL = %d\n", pAd->HwCfg.ChannelListNum, pAd->LogoTestCfg.OnTestingWHQL)); 
                if(pAd->LogoTestCfg.OnTestingWHQL)
                {
                    UCHAR   WHQLChannelOrder[11] = {11, 8, 6, 3, 1, 7, 5, 4, 2, 9, 10};
                    
                    pPort->P2PCfg.P2pProprietary.ListenChanelCount = 11;
                    for (chIdx = 0; chIdx < 11; chIdx++)
                    {
                        //pPort->P2PCfg.P2pProprietary.ListenChanel[chIdx] = (11 - chIdx );
                        pPort->P2PCfg.P2pProprietary.ListenChanel[chIdx] = WHQLChannelOrder[chIdx];
                        DBGPRINT(RT_DEBUG_TRACE, ("OnTestingWHQL P2P scan ch %d \n", pPort->P2PCfg.P2pProprietary.ListenChanel[chIdx]));
                    }
                    
                }
                // If scan period is less than 1 sec, scan some of more important channels at first (default listen channel)
                else if ((pPort->P2PCfg.ScanPeriod < 10) && (pAd->HwCfg.ChannelListNum > 0) && (pAd->HwCfg.ChannelListNum <= 11) &&
                         ((pAd->HwCfg.ChannelList[0].Channel != P2P_DEFAULT_LISTEN_CHANNEL) || 
                         (pAd->HwCfg.ChannelList[pAd->HwCfg.ChannelListNum - 1].Channel == P2P_DEFAULT_LISTEN_CHANNEL)))
                {
                    // scan in anti-clockwise
                    pPort->P2PCfg.P2pProprietary.ListenChanelCount = pAd->HwCfg.ChannelListNum;
                    for (chIdx = 0; chIdx < pAd->HwCfg.ChannelListNum; chIdx++)
                    {
                        pPort->P2PCfg.P2pProprietary.ListenChanel[chIdx] = pAd->HwCfg.ChannelList[pAd->HwCfg.ChannelListNum - chIdx - 1].Channel;
                        DBGPRINT(RT_DEBUG_TRACE, ("Not OnTestingWHQL P2P scan ch in anti-clockwise %d \n", pPort->P2PCfg.P2pProprietary.ListenChanel[chIdx]));
                    }
                }
                else
                {
                    // todo A band would not scan all channels
                    pPort->P2PCfg.P2pProprietary.ListenChanelCount = pAd->HwCfg.ChannelListNum;
                    DBGPRINT(RT_DEBUG_TRACE,("[WFDDBG] SCAN AUTO/FORCED::Channel Count = %d \n", pAd->HwCfg.ChannelListNum)); 
                    for (chIdx = 0; chIdx < pPort->P2PCfg.P2pProprietary.ListenChanelCount; chIdx++)
                    {
                        pPort->P2PCfg.P2pProprietary.ListenChanel[chIdx] = pAd->HwCfg.ChannelList[chIdx].Channel;
                        DBGPRINT(RT_DEBUG_TRACE, ("Not OnTestingWHQL P2P scan ch %d \n", pPort->P2PCfg.P2pProprietary.ListenChanel[chIdx]));
                    }
                }
            }
            else
            {
                P2pScanChannelDefault(pAd, pPort);
                DBGPRINT(RT_DEBUG_TRACE,("[WFDDBG] SCAN Default::Channel Count = %d \n", pPort->P2PCfg.P2pProprietary.ListenChanelCount));
            }

            break;
            
        case P2P_DISCO_TYEP_SCAN_SOCIAL_CHANNEL:
            {
                P2pScanChannelDefault(pAd, pPort);
                DBGPRINT(RT_DEBUG_TRACE,("[WFDDBG] SCAN ONLY on socail channel\n"));                
            }
            break;

        // Ralink P2P scan
        case P2P_DISCO_TYPE_DEFAULT:
        default:
        {
            UCHAR       i;

            // The rule can be changed.
            // In first 10 seconds, only scan socials channels, it can collect more P2P device more quickly.
            // and make user feels better.
            // But after 10 seconds, we need to scan all other supported channels too. because we might want 
            // to search for GO that is not in social channels.
            DBGPRINT(RT_DEBUG_TRACE, ("update switchchannel ??? P2pScanChannelUpdate CounterAftrScanButton = %d > P2P_SCAN_EARLYSTAGE %d ??", pPort->P2PCfg.P2pCounter.CounterAftrScanButton, P2P_SCAN_EARLYSTAGE));
            if ((pPort->P2PCfg.P2pCounter.CounterAftrScanButton <= P2P_SCAN_EARLYSTAGE) && 
                (PORTV2_P2P_ON(pAd, pAd->PortList[pPort->P2PCfg.PortNumber])) &&
                (INFRA_ON(pAd->PortList[PORT_0])))
            {
                if(IS_SOCIAL_CHANNEL(pPort->Channel))
                {
                    pPort->P2PCfg.P2pProprietary.ListenChanel[0] = 1;
                    pPort->P2PCfg.P2pProprietary.ListenChanel[1] = 6;
                    pPort->P2PCfg.P2pProprietary.ListenChanel[2] = 11;
                    pPort->P2PCfg.P2pProprietary.ListenChanelIndex = 0;
                    pPort->P2PCfg.P2pProprietary.ListenChanelCount = 3;
                    pPort->P2PCfg.P2pCounter.ScanIntervalBias = P2P_RANDOM_BASE;
                    DBGPRINT(RT_DEBUG_TRACE, ("infa port 0 CommonCfg channel is %d equal 1 6 11 so just scan ", pPort->Channel));
                }
                else
                {
                    pPort->P2PCfg.P2pProprietary.ListenChanel[0] = 1;
                    pPort->P2PCfg.P2pProprietary.ListenChanel[1] = 6;
                    pPort->P2PCfg.P2pProprietary.ListenChanel[2] = 11;
                    pPort->P2PCfg.P2pProprietary.ListenChanel[3] = pPort->Channel;
                    pPort->P2PCfg.P2pProprietary.ListenChanelIndex = 0;
                    pPort->P2PCfg.P2pProprietary.ListenChanelCount = 4;
                    pPort->P2PCfg.P2pCounter.ScanIntervalBias = P2P_RANDOM_BASE;
                    DBGPRINT(RT_DEBUG_TRACE, ("infa port 0 CommonCfg channel is %d not 1 6 11 so scan 1 6 11 commonChannel", pPort->Channel));
                }
        
                return;
        
            }
    
            if (pPort->P2PCfg.P2pCounter.CounterAftrScanButton <= P2P_SCAN_EARLYSTAGE)
                return;

            // Do not update scan channels during connection setup
            /*if (pPort->P2PCfg.P2PConnectState > P2P_CONNECT_IDLE && pPort->P2PCfg.P2PConnectState < P2P_I_AM_CLIENT_OP)
            {
                //only scan social channels during connection setup
                if (pPort->P2PCfg.P2pProprietary.ListenChanelCount > 4)
                    P2pScanChannelDefault(pAd, pPort);
                return;
            }*/
    
            if ((pAd->Counter.MTKCounters.LastOneSecTotalTxCount < 80) && (pAd->Counter.MTKCounters.OneSecRxOkDataCnt < 80))
            {
                // Default listen channel is based on spec, using Social channels 1, 6, 11.
                if ((pPort->P2PCfg.P2pProprietary.ListenChanelCount == 3) || (pPort->P2PCfg.P2pProprietary.ListenChanelCount == 4))
                {
                    DBGPRINT(RT_DEBUG_TRACE, ("update switchchannel to scan all channel"));
                    pPort->P2PCfg.P2pProprietary.ListenChanelIndex = 0;
                    pPort->P2PCfg.P2pProprietary.ListenChanelCount = pAd->HwCfg.ChannelListNum;
                    // Limit to scan 2.4 GHZ when I am GO. 
                    if (IS_P2P_GO_OP(pPort) && (pPort->P2PCfg.P2pProprietary.ListenChanelCount > 11))
                        pPort->P2PCfg.P2pProprietary.ListenChanelCount = 11;

                    for (i = 0; i < pPort->P2PCfg.P2pProprietary.ListenChanelCount; i++)
                    {
                        pPort->P2PCfg.P2pProprietary.ListenChanel[i] = pAd->HwCfg.ChannelList[i].Channel;
                    }
                }
                /*else if (pPort->P2PCfg.P2pProprietary.ListenChanelCount == 4)
                {
                    // do nothing.  When channel number is 4, it means I have specific channel to scan before connecting.
                    // so don't need scan all supported channel because it will make connectin process longer.
                }*/ 
                // If I am Client... don't need to scan so many channels.
                else if ((pPort->P2PCfg.P2pProprietary.ListenChanelCount > 4) && (!IS_P2P_CLIENT_OP(pPort)))
                {
                    P2pScanChannelDefault(pAd, pPort);
                }
        
            }
        }
        break;
    }

    // Update Scan Interval Bias
    if (pPort->P2PCfg.DiscoverType == P2P_DISCO_TYPE_DEFAULT)
    {
        if (pPort->P2PCfg.P2pProprietary.ListenChanelCount > 11)
            pPort->P2PCfg.P2pCounter.ScanIntervalBias = 40;
        else if (pPort->P2PCfg.P2pProprietary.ListenChanelCount > 3)
            pPort->P2PCfg.P2pCounter.ScanIntervalBias = 25;
        else
            pPort->P2PCfg.P2pCounter.ScanIntervalBias = 20;
    }
    else
    {
        // Total scan time in Win8 is less than 5 sec
        // So shorten the time stayed in listen channel
        pPort->P2PCfg.P2pCounter.ScanIntervalBias = 20; // 2 sec

        pPort->P2PCfg.P2pProprietary.ChannelScanRound++;
    }

    DBGPRINT(RT_DEBUG_TRACE, ("P2pScanChannelUpdate <=== round = %d, count = %d, scan-bias=%d, scan-bias2=%d, Channels are ", 
                                 pPort->P2PCfg.P2pProprietary.ChannelScanRound,
                                pPort->P2PCfg.P2pProprietary.ListenChanelCount,
                                pPort->P2PCfg.P2pProprietary.ListenChanelIndex,
                                 pPort->P2PCfg.P2pCounter.ScanIntervalBias,
                                 pPort->P2PCfg.P2pCounter.ScanIntervalBias2,
                                 pPort->P2PCfg.P2pProprietary.ListenChanelCount));
    
    for (chIdx = 0; chIdx < pPort->P2PCfg.P2pProprietary.ListenChanelCount; chIdx++)
    {
        DBGPRINT(RT_DEBUG_TRACE, ("%d ",  pPort->P2PCfg.P2pProprietary.ListenChanel[chIdx]));
    }

    DBGPRINT(RT_DEBUG_TRACE, ("\n"));
    
}

/*  
    ==========================================================================
    Description: 
        when timeout or error case happend, need to update P2PConnectState to correctstate.
    Parameters: 
         
    Note:
         
    ==========================================================================
 */
VOID P2pConnectStateUpdate(
    IN PMP_ADAPTER pAd,
    IN PMP_PORT      pPort)
{
//    PMP_PORT    pPort = pAd->PortList[pPort->P2PCfg.PortNumber];
    
    // P2p state might be set to off by other means.
    if (P2P_OFF(pPort))
    {
        if (pPort->P2PCfg.P2PConnectState > P2P_CONNECT_NOUSE)
            pPort->P2PCfg.P2PConnectState = P2P_CONNECT_IDLE;
        
        DBGPRINT(RT_DEBUG_TRACE, ("P2pConnectStateUpdate-  \n"));
        P2PPrintState(pAd);
        return;
    }


    // Don't consider I am Client case. because linkdown function will set back the state for WPS fail.
    if (pAd->PortList[pPort->P2PCfg.PortNumber]->PortSubtype == PORTSUBTYPE_P2PGO)
    {
        if (pPort->MacTab.Size > 0)
        {
            pPort->P2PCfg.P2PConnectState = P2P_I_AM_GO_OP;
        }
        // This is Auto GO mode. So don't turn down AUto GO when some connection timed out.
        else if ((pPort->MacTab.Size == 0) && (IS_P2P_AUTOGO(pPort)))
        {
            P2PDefaultConfigM(pAd, pPort);
            pPort->P2PCfg.P2PConnectState = P2P_ANY_IN_FORMATION_AS_GO;
        }
        else
        {
            P2pStopGo(pAd, pAd->PortList[pPort->P2PCfg.PortNumber]);
            pPort->P2PCfg.P2PConnectState = P2P_CONNECT_IDLE;
        }
    }
    else
    {
        P2PDefaultConfigM(pAd, pPort);
        pPort->P2PCfg.P2PConnectState = P2P_CONNECT_IDLE;
    }

    DBGPRINT(RT_DEBUG_TRACE, ("P2pConnectStateUpdate<==  pPort->MacTab.Size = %d \n", pPort->MacTab.Size));
    P2PPrintState(pAd);
}

/*
    ========================================================================
    
    Routine Description:
        If.

    Arguments:
            - NIC Adapter pointer
        
    Return Value:
        FALSE - None of channel in ChannelList Match any channel in pAd->HwCfg.ChannelList[] array

    IRQL = DISPATCH_LEVEL
    
    Note:
    ========================================================================

*/
BOOLEAN P2pCheckChannelList(
    IN PMP_ADAPTER pAd,
    IN PUCHAR   pChannelList)
{
    UCHAR       i, k;
    UCHAR       NumOfDismatch = 0;
    
    // Check if new (control) channel is in our channellist which we currently agree to operate in.
    for (k = 0;k < MAX_NUM_OF_CHANNELS;k++)
    {
        if (*(pChannelList + k) == 0)
            break;
        
        for (i = 0;i < pAd->HwCfg.ChannelListNum;i++)
        {
            if (pAd->HwCfg.ChannelList[i].Channel == *(pChannelList + k))
            {
                break;
            }
        }
        if ( i == pAd->HwCfg.ChannelListNum)
            NumOfDismatch++;            
    }

    if ((NumOfDismatch == k) && (k != 0))
    {
        DBGPRINT(RT_DEBUG_TRACE, (" P2P - no common channel = %d...\n", *pChannelList));
        return FALSE;
    }

    return TRUE;

}

VOID P2pCopySettingsWhenLinkup(
    IN PMP_ADAPTER pAd,
    IN PMP_PORT pPort)
{
    PMAC_TABLE_ENTRY    pEntry;
    PMAC_TABLE_ENTRY pWcidMacTabEntry = NULL;
    pWcidMacTabEntry = MlmeSyncMacTabMatchedWlanIdx(pPort, P2pGetClientWcid(pAd, pPort));     
    if(pWcidMacTabEntry == NULL)
    {
        DBGPRINT(RT_DEBUG_TRACE,("%s(), Line:%d, Mac Entry is NULL  Wcid = %d\n", __FUNCTION__, __LINE__, P2pGetClientWcid(pAd, pPort)));
        return;
    }
    
    if (IS_P2P_CON_CLI(pAd, pPort) || IS_P2P_MS_CLI(pAd, pPort) || IS_P2P_MS_CLI2(pAd, pPort))
    {
    }
    else
        return;

    pEntry = pWcidMacTabEntry;
    
    // Save related parameters to each Port for each association.
    // "COPY_SETTINGS_FROM_MLME_AUX_TO_ACTIVE_CFG" also contains them.
    pPort->PortCfg.SsidLen = pAd->MlmeAux.SsidLen;
    PlatformMoveMemory(pPort->PortCfg.Ssid, pAd->MlmeAux.Ssid, pAd->MlmeAux.SsidLen);
    COPY_MAC_ADDR(pPort->PortCfg.Bssid, pAd->MlmeAux.Bssid);
    pPort->CommonCfg.BeaconPeriod = pAd->MlmeAux.BeaconPeriod;

    // Supported rates
    pEntry->SupRateLen = pAd->MlmeAux.SupRateLen + pAd->MlmeAux.ExtRateLen;
    if (pEntry->SupRateLen <= MAX_LEN_OF_SUPPORTED_RATES)
    {
        PlatformMoveMemory(&pEntry->SupRate[0], pAd->MlmeAux.SupRate, pAd->MlmeAux.SupRateLen);
        PlatformMoveMemory(&pEntry->SupRate[pAd->MlmeAux.SupRateLen], pAd->MlmeAux.ExtRate, pAd->MlmeAux.ExtRateLen);
    }
    else
        pEntry->SupRateLen = 0;     
    pEntry->RateLen = pEntry->SupRateLen;

    DBGPRINT(RT_DEBUG_TRACE, ("%s: RateLen=%d. MCSSet[0]=0x%2x. MCSSet[1]=0x%2x \n", 
        __FUNCTION__, pEntry->RateLen, pEntry->HTCapability.MCSSet[0], pEntry->HTCapability.MCSSet[1]));    


}

VOID P2pCalculateChannelQuality(
    IN PMP_ADAPTER pAd,
    IN PMP_PORT pPort,
    IN ULONGLONG Now64)
{
    ULONG TxOkCnt, TxCnt, TxPER, TxPRR;
    ULONG RxCnt, RxPER;
    UCHAR NorRssi;
    CHAR  MaxRssi;
    PMAC_TABLE_ENTRY pWcidMacTabEntry = NULL;
    pWcidMacTabEntry = MlmeSyncMacTabMatchedWlanIdx(pPort, P2pGetClientWcid(pAd, pPort)); 

    if(pWcidMacTabEntry == NULL)
    {
        DBGPRINT(RT_DEBUG_TRACE,("%s(), Line:%d, Mac Entry is NULL  Wcid = %d\n", __FUNCTION__, __LINE__, P2pGetClientWcid(pAd, pPort)));
        return;
    }
        
    MaxRssi = RTMPMaxRssi(pAd, pAd->StaCfg.RssiSample.LastRssi[0], pAd->StaCfg.RssiSample.LastRssi[1], pAd->StaCfg.RssiSample.LastRssi[2]);

    //
    // calculate TX packet error ratio and TX retry ratio - if too few TX samples, skip TX related statistics
    //
    TxOkCnt = pAd->Counter.MTKCounters.OneSecTxNoRetryOkCount + pAd->Counter.MTKCounters.OneSecTxRetryOkCount;
    TxCnt = TxOkCnt + (pAd->Counter.MTKCounters.OneSecTxFailCount>>2);
    if (TxCnt < 5) 
    {
        TxPER = 0;
        TxPRR = 0;
    }
    else 
    {
        TxPER = (pAd->Counter.MTKCounters.OneSecTxFailCount * 100) / TxCnt; 
        TxPRR = ((TxCnt - pAd->Counter.MTKCounters.OneSecTxNoRetryOkCount) * 100) / TxCnt;
    }

    //
    // calculate RX PER - don't take RxPER into consideration if too few sample
    //
    RxCnt = pAd->Counter.MTKCounters.OneSecRxOkCnt + pAd->Counter.MTKCounters.OneSecRxFcsErrCnt;
    if (RxCnt < 5)
        RxPER = 0;
    else
        RxPER = (pAd->Counter.MTKCounters.OneSecRxFcsErrCnt * 100) / RxCnt;

    //
    // decide ChannelQuality based on: 1)last BEACON received time, 2)last RSSI, 3)TxPER, and 4)RxPER
    //
    if (INFRA_ON(pPort) &&
        ((pWcidMacTabEntry->LastBeaconRxTime + P2P_BEACON_LOST_TIME) < Now64))
    {
        DBGPRINT(RT_DEBUG_TRACE, ("(Wcid=%d) BEACON lost > %d msec with TxOkCnt=%d -> CQI=0\n", P2pGetClientWcid(pAd, pPort), SHORT_BEACON_LOST_TIME, TxOkCnt)); 
        pPort->Mlme.ChannelQuality = 0;
    }
    else
    {
        // Normalize Rssi
        if (MaxRssi > -40)
            NorRssi = 100;
        else if (MaxRssi < -90)
            NorRssi = 0;
        else
            NorRssi = (MaxRssi + 90) * 2;
        
        // ChannelQuality = W1*RSSI + W2*TxPRR + W3*RxPER    (RSSI 0..100), (TxPER 100..0), (RxPER 100..0)
        pPort->Mlme.ChannelQuality = (RSSI_WEIGHTING * NorRssi + 
                                   TX_WEIGHTING * (100 - TxPRR) + 
                                   RX_WEIGHTING* (100 - RxPER)) / 100;
        if (pPort->Mlme.ChannelQuality >= 100)
            pPort->Mlme.ChannelQuality = 100;
    }

    DBGPRINT(RT_DEBUG_INFO, ("(Wcid=%d) MMCHK - CQI= %d (Tx Fail=%d/Retry=%d/Total=%d, Rx Fail=%d/Total=%d, RSSI=%d dbm)\n",
        P2pGetClientWcid(pAd, pPort),
        pPort->Mlme.ChannelQuality, 
        pAd->Counter.MTKCounters.OneSecTxFailCount, 
        pAd->Counter.MTKCounters.OneSecTxRetryOkCount, 
        TxCnt, 
        pAd->Counter.MTKCounters.OneSecRxFcsErrCnt, 
        RxCnt, pAd->StaCfg.RssiSample.LastRssi[0]));

}

BOOLEAN IsP2pFirstMacSmaller(
    IN PUCHAR       Firststaddr,
    IN PUCHAR       SecondAddr)
{
    UCHAR       i;
    for (i=0 ; i< 6;i++)
    {
        if (*(Firststaddr+i) > *(SecondAddr+i))
            return FALSE;
    }

    return TRUE;
}


BOOLEAN P2PIsScanAllowed (
    IN PMP_ADAPTER    pAd,
    IN PMP_PORT     pPort)
{
    BOOLEAN bScanAllowed = TRUE;

    if(P2P_OFF(pPort))
        return bScanAllowed;

    if((pPort->P2PCfg.P2pMsSatetPhase > P2pMs_STATE_IDLE) ||
        (pPort->P2PCfg.P2pCounter.CounterAfterConnectCommand < WFD_BLOCKING_SCAN_TIMEOUT))
    {
        DBGPRINT(RT_DEBUG_TRACE, ("[%s] Win8 (as %s) - Skip this scan request when within %d sec after a new connection\n",  __FUNCTION__ , decodeP2PState(pPort->P2PCfg.P2pMsSatetPhase), WFD_BLOCKING_SCAN_TIMEOUT/10));
        bScanAllowed = FALSE;
    }
    else if (pPort->P2PCfg.P2PDiscoProvState > P2P_ENABLE_LISTEN_ONLY)
    {
        DBGPRINT(RT_DEBUG_TRACE, ("[%s] (as %s) - Skip this scan request during p2p scanning",__FUNCTION__, decodeP2PState(pPort->P2PCfg.P2PDiscoProvState)));
        bScanAllowed = FALSE;
    }
    else if (IS_P2P_SIGMA_ON(pPort) && 
        ((pPort->P2PCfg.P2PDiscoProvState == P2P_IDLE) ||
        (pPort->P2PCfg.P2PConnectState == P2P_ANY_IN_FORMATION_AS_GO) ||
        (pPort->P2PCfg.GONoASchedule.bValid == TRUE) ||
        MT_TEST_BIT(pPort->P2PCfg.CTWindows, P2P_OPPS_BIT)))
    {
        // This is for WiFi Direct Test Case, don't do normal scan to decrease our search possibility. 
        // When doing Sigma testing, 
        // don't do scan if Sigma always set us to Find/Search/Discvoery Phase.
        // don't do scan if NoA/CTWindows turns on.
        DBGPRINT(RT_DEBUG_TRACE, ("[%s] (as %s) skip scan request. Sigma testing.\n",__FUNCTION__ ,decodeP2PState(pPort->P2PCfg.P2PDiscoProvState)));
        bScanAllowed = FALSE;
    }
    else if(pPort->StaCfg.WscControl.WscState >= WSC_STATE_LINK_UP)
    {
        DBGPRINT(RT_DEBUG_TRACE, ("[%s] Skipped scan under WPS.\n",__FUNCTION__));
        bScanAllowed = FALSE;
    }
    else if ((pAd->OpMode == OPMODE_STA) && (IS_P2P_NOT_AUTOGO(pPort)) &&
        (IS_P2P_FINDING(pPort) || (IS_P2P_GO_NEG(pPort))|| 
        (IS_P2P_REGISTRA(pPort)) 
        || (IS_P2P_GO_WPA2PSKING(pPort))
        || (IS_P2P_ENROLLEE(pPort))))
    {
        DBGPRINT(RT_DEBUG_TRACE, ("[%s] (as %s) skip scan request\n",__FUNCTION__ ,decodeP2PState(pPort->P2PCfg.P2PConnectState)));
        bScanAllowed = FALSE;
    }
    else if ((pAd->OpMode == OPMODE_STA) && (IS_P2P_AUTOGO(pPort)) &&
        (IS_P2P_FINDING(pPort) ||
        (IS_P2P_REGISTRA(pPort)) 
        || (IS_P2P_GO_WPA2PSKING(pPort))))
    {
        DBGPRINT(RT_DEBUG_TRACE, ("[%s] (as %s) skip this request\n",__FUNCTION__ ,decodeP2PState(pPort->P2PCfg.P2PConnectState)));
        bScanAllowed = FALSE;
    }
    else if ((pAd->OpMode == OPMODE_STAP2P) && (IS_P2P_AUTOGO(pPort)) &&
        (IS_P2P_FINDING(pPort) 
        || (IS_P2P_REGISTRA(pPort)) 
        || (IS_P2P_GO_WPA2PSKING(pPort))))
    {
        DBGPRINT(RT_DEBUG_TRACE, ("[%s] Concurernt mode AutoGO important state skip this request\n",__FUNCTION__));
        bScanAllowed = FALSE;
    }
    else if ((pAd->OpMode == OPMODE_STAP2P) && (IS_P2P_GROUP_FORMING(pPort)) &&
        (IS_P2P_FINDING(pPort)
        || (IS_P2P_REGISTRA(pPort)) 
        || (IS_P2P_GO_WPA2PSKING(pPort))))
    {
        DBGPRINT(RT_DEBUG_TRACE, ("[%s] Concurernt mode and GO important state skip this request\n",__FUNCTION__));
        bScanAllowed = FALSE;
    }
//#ifndef NDIS630_MINIPORT  // win8+win7 ??
//  else if ((pPort->P2PCfg.P2pCounter.Wpa2pskCounter100ms < 300)
//      && (pAd->OpMode == OPMODE_STA))
//  {
//      DBGPRINT(RT_DEBUG_TRACE, ("[%s] Skip this request\n when within 30 sec after a new connection",__FUNCTION__));
//      bScanAllowed = FALSE;
//  }
//#endif
    else if (pPort->P2PCfg.P2PConnectState == P2P_DO_WPS_ENROLLEE_DONE)
    {
        DBGPRINT(RT_DEBUG_TRACE, ("[%s] ignore first Scan request after WPS done\n",__FUNCTION__));
        bScanAllowed = FALSE;
    }
    else if ((pPort->P2PCfg.P2pCounter.CounterAfterConnectCommand  < 300)
        && (pPort->P2PCfg.P2PConnectState < P2P_ANY_IN_FORMATION_AS_GO)
        && (pPort->P2PCfg.P2PConnectState > P2P_CONNECT_NOUSE))
    {
        DBGPRINT(RT_DEBUG_TRACE, ("[%s] ignore scan request during GO negotiation\n",__FUNCTION__));
        bScanAllowed = FALSE;
    }

    return bScanAllowed;
}


VOID    P2PSetSigmaStatus(
    IN PMP_ADAPTER    pAd,
    IN PMP_PORT       pPort,
    IN UCHAR            Rule)
{
    PlatformMoveMemory(pPort->P2PCfg.SigmaQueryStatus.Bssid, pPort->PortCfg.Bssid, 6);
    PlatformZeroMemory(pPort->P2PCfg.SigmaQueryStatus.Ssid, 32);
    PlatformMoveMemory(pPort->P2PCfg.SigmaQueryStatus.Ssid, pPort->PortCfg.Ssid, pPort->PortCfg.SsidLen);
    pPort->P2PCfg.SigmaQueryStatus.SsidLen = pPort->PortCfg.SsidLen;
    pPort->P2PCfg.SigmaQueryStatus.MyRule = Rule;
    pPort->P2PCfg.SigmaQueryStatus.ConfigMethod = pPort->P2PCfg.ConfigMethod;
    pPort->P2PCfg.SigmaQueryStatus.P2PConnectState = (UCHAR)pPort->P2PCfg.P2PConnectState;

    DBGPRINT(RT_DEBUG_TRACE, ("P2P: Update Sigma Status to State=%s, SsidLen=%d, ConfigMethod=%s, Rule=%d\n", 
        decodeP2PState(pPort->P2PCfg.SigmaQueryStatus.P2PConnectState), 
        pPort->P2PCfg.SigmaQueryStatus.SsidLen,
        decodeConfigMethod(pPort->P2PCfg.SigmaQueryStatus.ConfigMethod),
        pPort->P2PCfg.SigmaQueryStatus.MyRule));  
}

VOID P2PInitSigmaParam(
    IN PMP_ADAPTER    pAd,
    IN PMP_PORT       pPort)
{
    // Reste SIGMA QueryStatus & Commands
    DBGPRINT(RT_DEBUG_TRACE, ("P2PInitSigmaParam==> intent = %d \n", pPort->P2PCfg.GoIntentIdx));

    PlatformZeroMemory(&pPort->P2PCfg.SigmaQueryStatus, sizeof(RT_OID_SIGMA_QUERY_P2P_STATUS_STRUCT));
    if (IS_P2P_NOT_AUTOGO(pPort))
    {
        PlatformZeroMemory(&pPort->P2PCfg.SigmaSetting, sizeof(P2P_SIGMA_SETTING_STRUCT));
    }

    pPort->P2PCfg.SigmaQueryStatus.ConfigMethod = (USHORT) pPort->CommonCfg.P2pControl.field.DefaultConfigMethod;
    pPort->P2PCfg.SigmaSetting.MyConfigMethod = 0xffff;
    pPort->P2PCfg.SigmaSetting.StartNoaWhenGO = 0;
    if (IS_P2P_NOT_AUTOGO(pPort))
    {
        pPort->P2PCfg.SigmaSetting.CTWindow = 0;
    }
    DBGPRINT(RT_DEBUG_TRACE, ("P2PInitSigmaParam==>  %d \n", pPort->P2PCfg.SigmaSetting.CTWindow));
}

VOID P2pCheckAndUpdateGroupChannel(
    IN PMP_ADAPTER    pAd,
    IN PMP_PORT         pPort,
    IN PUCHAR           ChannelList)
{
    UCHAR chIdx, newChannel;
    BOOLEAN bUpdateChannel = TRUE;
    UCHAR       tempChannelList[MAX_NUM_OF_CHANNELS];
    CHAR  i = 0, count = 0;

    PlatformZeroMemory(tempChannelList, MAX_NUM_OF_CHANNELS);
    
    //check if group channel is in the supported channel list
    for(chIdx = 0; chIdx < MAX_NUM_OF_CHANNELS; chIdx++)
    {
        if(pPort->P2PCfg.GroupChannel == ChannelList[chIdx])
        {
            bUpdateChannel = FALSE;
            break;
        }
        else if(ChannelList[chIdx] == 0)
        {
            break;
        }
    }

    //randomly select an available channel if existing group channel is not valid
    if(bUpdateChannel == TRUE)
    {
        if (chIdx == 0 && ChannelList[0] == 0)
        {
            return;
        }   
        else if (chIdx == 0 && ChannelList[0] != 0)
        {
            newChannel = ChannelList[0];
        }   
        else
        {
            if(pPort->P2PCfg.GroupChannel <= 14)
            {
                for(i = 0; i < chIdx; i++)
                {
                    if(ChannelList[i] <= 14)
                    {
                        tempChannelList[i] = ChannelList[i];
                        DBGPRINT(RT_DEBUG_TRACE, ("2.4 G ==> tempChannelList= %d\n",tempChannelList[i]));
                    }

                    count ++;
                }
            }
            else
            {
                for(i = 0; i < chIdx; i++)
                {
                    if(ChannelList[i] > 14)
                    {
                        tempChannelList[i] = ChannelList[i];
                        DBGPRINT(RT_DEBUG_TRACE, ("5 G ==> tempChannelList= %d\n",tempChannelList[i]));
                    }   

                    count ++;
                }
            }
    
            newChannel = tempChannelList[RandomByte(pAd) % count];
        }

        DBGPRINT(RT_DEBUG_TRACE, ("%s==> Updating channel original = %d new = %d\n", __FUNCTION__, pPort->P2PCfg.GroupChannel,newChannel));
        pPort->P2PCfg.GroupChannel = newChannel;
    }
}

/*  
    ==========================================================================
    Description: 
        Before calling this function, make sure the pPort->CommonCfg.P2pControl.word already has the right setting. 
        
    Parameters: 
    Note:
         
    ==========================================================================
 */
VOID P2PUpdateCapability(
    IN PMP_ADAPTER    pAd,
    IN PMP_PORT         pPort)
{
    // Reste SIGMA QueryStatus & Commands
    DBGPRINT(RT_DEBUG_TRACE, ("P2PUpdateCapability==> P2pControl = %x  \n", pPort->CommonCfg.P2pControl.word));
    
    if (pPort->CommonCfg.P2pControl.field.EnablePresistent == 1)
    {
        pPort->P2PCfg.P2pCapability[1] |= GRPCAP_PERSISTENT;
        pPort->P2PCfg.P2pCapability[1] |= GRPCAP_PERSISTENT_RECONNECT;
        pPort->P2PCfg.P2pCapability[0] |= DEVCAP_INVITE;
        pPort->P2PCfg.ExtListenInterval = P2P_EXT_LISTEN_INTERVAL;
        pPort->P2PCfg.ExtListenPeriod = P2P_EXT_LISTEN_PERIOD;
        DBGPRINT(RT_DEBUG_TRACE, ("P2PUpdateCapability  : Enable Persistent. \n"));
    }
    else
    {
        pPort->P2PCfg.P2pCapability[1] &= (~GRPCAP_PERSISTENT);
        pPort->P2PCfg.P2pCapability[1] &= (~GRPCAP_PERSISTENT_RECONNECT);
        // When Persistent is disabled, I won't support extended listening, so set to Zero.
        DBGPRINT(RT_DEBUG_TRACE, ("P2PUpdateCapability  : Disable Persistent. \n"));
    }

    if (pPort->CommonCfg.P2pControl.field.ClientDiscovery == 1)
    {
        pPort->P2PCfg.P2pCapability[0] |= DEVCAP_CLIENT_DISCOVER;
        DBGPRINT(RT_DEBUG_TRACE, ("P2PUpdateCapability  : Enable Client Discovery. \n"));
    }
    else
        pPort->P2PCfg.P2pCapability[0] &= (~DEVCAP_CLIENT_DISCOVER);
        
    
    if (pPort->CommonCfg.P2pControl.field.EnableInvite == 1)
    {
        pPort->P2PCfg.P2pCapability[0] |= DEVCAP_INVITE;
        DBGPRINT(RT_DEBUG_TRACE, ("P2PUpdateCapability  : Enable Invite. \n"));
    }
    else
        pPort->P2PCfg.P2pCapability[0] &=  (~DEVCAP_INVITE);
        

    if (pPort->CommonCfg.P2pControl.field.Managed == 1)
    {
        pPort->P2PCfg.P2pCapability[0] |= DEVCAP_INFRA_MANAGED;
        DBGPRINT(RT_DEBUG_TRACE, ("P2PUpdateCapability  : Enable Managed. \n"));
    }
    else
        pPort->P2PCfg.P2pCapability[0] &= (~ DEVCAP_INFRA_MANAGED);
        
    if (pPort->CommonCfg.P2pControl.field.ServiceDiscovery == 1)
    {
        pPort->P2PCfg.P2pCapability[0] |= DEVCAP_SD;
        DBGPRINT(RT_DEBUG_TRACE, ("P2PUpdateCapability  : Enable Service Discovery. \n"));
    }
    else
        pPort->P2PCfg.P2pCapability[0] &=  (~DEVCAP_SD);
        

    if (pPort->CommonCfg.P2pControl.field.EnableIntraBss == 1)
    {
        pPort->P2PCfg.P2pCapability[1] |= GRPCAP_INTRA_BSS;
        pPort->SoftAP.ApCfg.bIsolateInterStaTraffic = FALSE;

        DBGPRINT(RT_DEBUG_TRACE, ("P2PUpdateCapability  : Enable Intra BSS. \n"));
    }
    else
    {
        pPort->P2PCfg.P2pCapability[1] &=  (~GRPCAP_INTRA_BSS);
        pPort->SoftAP.ApCfg.bIsolateInterStaTraffic = TRUE;
        DBGPRINT(RT_DEBUG_TRACE, ("P2PUpdateCapability  : Disable Intra BSS. \n"));
    }
        

    if ((pAd->OpMode == OPMODE_STAP2P) && 
        (pAd->PortList[pPort->P2PCfg.PortNumber]->PortSubtype == PORTSUBTYPE_P2PGO))
    {
    
        if (pPort->CommonCfg.P2pControl.field.ForceDisableCrossConnect == 1)
        {
            pPort->P2PCfg.P2pCapability[1] &= GRPCAP_CROSS_CONNECT;
            DBGPRINT(RT_DEBUG_TRACE, ("Cross Connect Force Disabled.  GrpCap = %x\n", pPort->P2PCfg.P2pCapability[1]));
        }
        if (pPort->P2PCfg.P2pManagedParm.ICSStatus == ICS_STATUS_ENABLED)
        {
            pPort->P2PCfg.P2pCapability[1] |= GRPCAP_CROSS_CONNECT;
            DBGPRINT(RT_DEBUG_TRACE, ("Cross Connect Enabled.  GrpCap = %x\n", pPort->P2PCfg.P2pCapability[1]));
        }
        else
        {
            pPort->P2PCfg.P2pCapability[1] &= (~GRPCAP_CROSS_CONNECT);
            DBGPRINT(RT_DEBUG_TRACE, ("Concurrent mode : Cross Connect Disabled.  GrpCap = %x\n", pPort->P2PCfg.P2pCapability[1]));
        }
            
        pPort->P2PCfg.P2pCapability[0] |= DEVCAP_CLIENT_CONCURRENT;
        DBGPRINT(RT_DEBUG_TRACE, ("Concurrent Enabled.  GrpCap = %x\n", pPort->P2PCfg.P2pCapability[1]));
    }
    else
    {
        pPort->P2PCfg.P2pCapability[0] &= (~DEVCAP_CLIENT_CONCURRENT);
        pPort->P2PCfg.P2pCapability[1] &= (~GRPCAP_CROSS_CONNECT);
        DBGPRINT(RT_DEBUG_TRACE, ("Cross Connect Disabled.  GrpCap = %x\n", pPort->P2PCfg.P2pCapability[1]));
        DBGPRINT(RT_DEBUG_TRACE, ("Concurrent Disabled.  GrpCap = %x\n", pPort->P2PCfg.P2pCapability[1]));
    }
        
    if (pPort->CommonCfg.P2pControl.field.DefaultConfigMethod == P2P_REG_CM_DISPLAY)
    {
        pPort->P2PCfg.ConfigMethod = CONFIG_METHOD_DISPLAY;
        pPort->StaCfg.WscControl.RegData.EnrolleeInfo.ConfigMethods = cpu2be16(CONFIG_METHOD_DISPLAY);
        pPort->StaCfg.WscControl.RegData.RegistrarInfo.ConfigMethods = cpu2be16(CONFIG_METHOD_DISPLAY);
        pPort->StaCfg.WscControl.RegData.EnrolleeInfo.DevPwdId = cpu2be16(DEV_PASS_ID_PIN);
        pPort->StaCfg.WscControl.RegData.RegistrarInfo.DevPwdId = cpu2be16(DEV_PASS_ID_PIN);
        DBGPRINT(RT_DEBUG_TRACE,(" Display = %x \n", pPort->P2PCfg.ConfigMethod));
    }
    else if (pPort->CommonCfg.P2pControl.field.DefaultConfigMethod == P2P_REG_CM_KEYPAD)
    {
        pPort->P2PCfg.ConfigMethod = CONFIG_METHOD_KEYPAD;
        pPort->StaCfg.WscControl.RegData.EnrolleeInfo.ConfigMethods =cpu2be16(CONFIG_METHOD_KEYPAD);
        pPort->StaCfg.WscControl.RegData.RegistrarInfo.ConfigMethods = cpu2be16(CONFIG_METHOD_KEYPAD);
        pPort->StaCfg.WscControl.RegData.EnrolleeInfo.DevPwdId = cpu2be16(DEV_PASS_ID_USER);
        pPort->StaCfg.WscControl.RegData.RegistrarInfo.DevPwdId = cpu2be16(DEV_PASS_ID_USER);
        DBGPRINT(RT_DEBUG_TRACE,(" Keypad = %x \n", pPort->P2PCfg.ConfigMethod));
    }
    else if (pPort->CommonCfg.P2pControl.field.DefaultConfigMethod == P2P_REG_CM_LABEL)
    {
        pPort->P2PCfg.ConfigMethod = CONFIG_METHOD_LABEL;
        pPort->StaCfg.WscControl.RegData.EnrolleeInfo.ConfigMethods = cpu2be16(CONFIG_METHOD_LABEL);
        pPort->StaCfg.WscControl.RegData.RegistrarInfo.ConfigMethods = cpu2be16(CONFIG_METHOD_LABEL);
        pPort->StaCfg.WscControl.RegData.EnrolleeInfo.DevPwdId = cpu2be16(DEV_PASS_ID_PIN);
        pPort->StaCfg.WscControl.RegData.RegistrarInfo.DevPwdId = cpu2be16(DEV_PASS_ID_PIN);
        DBGPRINT(RT_DEBUG_TRACE,(" use Label = %x \n", pPort->P2PCfg.ConfigMethod));
    }
    else if (pPort->CommonCfg.P2pControl.field.DefaultConfigMethod == P2P_REG_CM_PBC)
    {
        pPort->P2PCfg.ConfigMethod = CONFIG_METHOD_PUSHBUTTON;
        pPort->StaCfg.WscControl.RegData.EnrolleeInfo.ConfigMethods = cpu2be16(CONFIG_METHOD_PUSHBUTTON);
        pPort->StaCfg.WscControl.RegData.RegistrarInfo.ConfigMethods = cpu2be16(CONFIG_METHOD_PUSHBUTTON);
        pPort->StaCfg.WscControl.RegData.EnrolleeInfo.DevPwdId = cpu2be16(DEV_PASS_ID_PBC);
        pPort->StaCfg.WscControl.RegData.RegistrarInfo.DevPwdId = cpu2be16(DEV_PASS_ID_PBC);
        DBGPRINT(RT_DEBUG_TRACE,("Default use PBC = %x \n", pPort->P2PCfg.ConfigMethod));
    }
    else
    {
        // Use all. but not include Label
        pPort->P2PCfg.ConfigMethod = (CONFIG_METHOD_DISPLAY | CONFIG_METHOD_KEYPAD | CONFIG_METHOD_PUSHBUTTON);
        pPort->StaCfg.WscControl.RegData.EnrolleeInfo.ConfigMethods = cpu2be16(CONFIG_METHOD_PUSHBUTTON);
        pPort->StaCfg.WscControl.RegData.RegistrarInfo.ConfigMethods = cpu2be16(CONFIG_METHOD_PUSHBUTTON);
        pPort->StaCfg.WscControl.RegData.EnrolleeInfo.DevPwdId = cpu2be16(DEV_PASS_ID_PBC);
        pPort->StaCfg.WscControl.RegData.RegistrarInfo.DevPwdId = cpu2be16(DEV_PASS_ID_PBC);
        DBGPRINT(RT_DEBUG_TRACE,("Default use all = %x \n", pPort->P2PCfg.ConfigMethod));
    }
}




/*  
    ==========================================================================
    Description: 
    0x6f is reserved for debug use command for P2P.
        
    Parameters: 
    0~1 :  print debug message
    2~5 :  NoA environemtn setting and start or stop command.
    6 : Disable ICS Event test.
    8 : Stop win8 p2p scan
    Note:
         
    ==========================================================================
 */
VOID P2PDebugUseCmd(
    IN  PMP_ADAPTER pAd,
    IN  PRT_802_11_HARDWARE_REGISTER pHardwareRegister)
{                   
    UCHAR   DebugMode;
    UCHAR   i;
    PMP_PORT    pPort = pAd->PortList[FXXK_PORT_0];
    
    if ((pHardwareRegister->Data&0xff) == 0x6f)
    {
        DebugMode = (pHardwareRegister->Data&0xff00) >>8;
        switch(DebugMode)
        {
            case 0:
                for (i = 0; i< pAd->pP2pCtrll->P2PTable.ClientNumber;i++)
                    P2PPrintP2PEntry(pAd, i);
                for (i = 1; i< (pPort->MacTab.Size + 3);i++)
                    P2PPrintMac(pAd, i);
                break;
            case 1:
                P2PPrintPort(pAd);
                P2PPrintState(pAd);
                DBGPRINT(RT_DEBUG_TRACE,("SyncMachine  CurrState = %d. )\n",pAd->PortList[PORT_0]->Mlme.SyncMachine.CurrState));
                break;
                
            case 2:
                pPort->CommonCfg.P2pControl.field.SwBasedNoA = 1;
                DBGPRINT(RT_DEBUG_TRACE,("Set:: SwBasedNoA = %d .\n", pPort->CommonCfg.P2pControl.field.SwBasedNoA));
                break;
            case 3:
                pPort->CommonCfg.P2pControl.field.SwBasedNoA = 0;
                DBGPRINT(RT_DEBUG_TRACE,("Set:: SwBasedNoA = %d .\n", pPort->CommonCfg.P2pControl.field.SwBasedNoA));
                break;
            case 4:
                pPort->P2PCfg.GONoASchedule.Count = 255;
                pPort->P2PCfg.GONoASchedule.Duration = 0xc800;
                pPort->P2PCfg.GONoASchedule.Interval = 0x19000;
                P2pGOStartNoA(pAd, pPort, FALSE);
                break;
            case 5:
                P2pStopNoA(pAd, NULL);
                break;
                            
            case 6:
                P2pSetEvent(pAd, pPort,  P2PEVENT_DISABLE_ICS);
                break;

            case 7:
                if (P2pSetEvent(pAd, pPort,  P2PEVENT_GEN_SEUDO_ADHOC_PROFILE) == FALSE)
                    pPort->P2PCfg.P2pEventQueue.bP2pGenSeudoAdhoc = TRUE;

                break;

            case 8:
            {
                if ((pPort->P2PCfg.P2pCounter.CounterAftrScanButton + 2) < pPort->P2PCfg.ScanPeriod)
                {
                    // Assign shoter timeout to timeout immediatedly after 200 ms during the ongoing discovery
                    pPort->P2PCfg.ScanPeriod = (pPort->P2PCfg.P2pCounter.CounterAftrScanButton + 2);
                }

                DBGPRINT(RT_DEBUG_TRACE, ("%s::OID_DOT11_WFD_STOP_DISCOVERY at (%d)\n", __FUNCTION__, pPort->P2PCfg.ScanPeriod));     
            }
            break;
            
            default:
                break;
        }
    }

}
/*  
    ==========================================================================
    Description: 
    Set specific hardware registers by Sigma Control Agent,  
    where 0x60~0x6f reserved by P2P Plugfest.
        
    Parameters: 
    Note:
         
    ==========================================================================
 */
VOID P2PSetHwReg(
    IN  PMP_ADAPTER pAd,
    IN  PRT_802_11_HARDWARE_REGISTER pHardwareRegister)
{                   
    UCHAR       CTWindow;
    PMP_PORT    pPort = pAd->PortList[FXXK_PORT_0];
    
    if ((pHardwareRegister->Data&0xff) == 0x60)
    {
        if ((pHardwareRegister->Data&0xf00) == 0x100)
            pPort->CommonCfg.P2pControl.field.EnablePresistent = 1;
        else
            pPort->CommonCfg.P2pControl.field.EnablePresistent = 0;
        DBGPRINT_RAW(RT_DEBUG_TRACE, ("Set:: P2pControl=>EnablePresistent = %d \n", pPort->CommonCfg.P2pControl.field.EnablePresistent));
        P2PUpdateCapability(pAd, pPort);
    }
    else if ((pHardwareRegister->Data&0xff) == 0x61)
    {
        if ((pHardwareRegister->Data&0xf00) == 0x100)
            pPort->CommonCfg.P2pControl.field.EnableInvite = 1;
        else
            pPort->CommonCfg.P2pControl.field.EnableInvite = 0;
        DBGPRINT_RAW(RT_DEBUG_TRACE, ("Set:: P2pControl=>EnableInvite = %d \n", pPort->CommonCfg.P2pControl.field.EnableInvite));
        P2PUpdateCapability(pAd, pPort);
    }
    else if ((pHardwareRegister->Data&0xff) == 0x62)
    {
        if ((pHardwareRegister->Data&0xf00) == 0x100)
        {
            pPort->CommonCfg.P2pControl.field.EnableIntraBss = 1;
            pPort->SoftAP.ApCfg.bIsolateInterStaTraffic = FALSE;
        }
        else
        {
            pPort->CommonCfg.P2pControl.field.EnableIntraBss = 0;
            pPort->SoftAP.ApCfg.bIsolateInterStaTraffic = TRUE;
        }
        DBGPRINT_RAW(RT_DEBUG_TRACE, ("Set:: P2pControl=>EnableIntraBss = %d \n", pPort->CommonCfg.P2pControl.field.EnableIntraBss));
        P2PUpdateCapability(pAd, pPort);
    }
    else if ((pHardwareRegister->Data&0xff) == 0x63)
    {
        if ((pHardwareRegister->Data&0xf00) == 0x100)
            pPort->CommonCfg.P2pControl.field.ClientDiscovery = 1;
        else
            pPort->CommonCfg.P2pControl.field.ClientDiscovery = 0;
        DBGPRINT_RAW(RT_DEBUG_TRACE, ("Set:: P2pControl=>ClientDiscovery = %d \n", pPort->CommonCfg.P2pControl.field.ClientDiscovery));
        P2PUpdateCapability(pAd, pPort);
    }                   
    else if ((pHardwareRegister->Data&0xff) == 0x64)
    {
        if ((pHardwareRegister->Data&0xf00) == 0x100)
            pPort->CommonCfg.P2pControl.field.ServiceDiscovery = 1;
        else
            pPort->CommonCfg.P2pControl.field.ServiceDiscovery = 0;
        DBGPRINT_RAW(RT_DEBUG_TRACE, ("Set:: P2pControl=>ServiceDiscovery = %d \n", pPort->CommonCfg.P2pControl.field.ServiceDiscovery));
        P2PUpdateCapability(pAd, pPort);
    }
    else if ((pHardwareRegister->Data&0xff) == 0x65)
    {
        // 3. Trigger NoA schedule by NoACount
        if ((pHardwareRegister->Data&0xff00))
        {
            pPort->P2PCfg.GONoASchedule.Count= (pHardwareRegister->Data >> 8);
        
            DBGPRINT_RAW(RT_DEBUG_TRACE, ("Set:: NoA_Count=%d(%dTU) \n", pPort->P2PCfg.GONoASchedule.Count,  (pHardwareRegister->Data >> 8)));
            if (pPort->P2PCfg.GONoASchedule.Count == 0)
            {
                P2pStopNoA(pAd, NULL);
            //  pPort->P2PCfg.SigmaSetting.StartNoaWhenGO = 50;
            }
            else if (IS_P2P_GO_OP(pPort))
            {
                P2pGOStartNoA(pAd, pPort, FALSE);
                P2pUpdateNoABeacon(pAd, pAd->pTxCfg->BeaconBufLen);
                pPort->P2PCfg.SigmaSetting.StartNoaWhenGO = 0;
            }
            else
            {
                // Set this flag means GO automatically start NoA after it becomes active. (GO operating)
                pPort->P2PCfg.SigmaSetting.StartNoaWhenGO = 1;
            }
            
            DBGPRINT_RAW(RT_DEBUG_TRACE, ("Set:: Start NoA=>NoA_duration=%d, NoA_Interval=%d, NoA_Count=%d \n", 
                            pPort->P2PCfg.GONoASchedule.Duration, pPort->P2PCfg.GONoASchedule.Interval, pPort->P2PCfg.GONoASchedule.Count));
            
        }
        else
        {
            P2pStopNoA(pAd, NULL);
            P2pUpdateNoABeacon(pAd, pAd->pTxCfg->BeaconBufLen);
        }
    }
    else if ((pHardwareRegister->Data&0xff) == 0x66)
    {
        if ((pHardwareRegister->Data&0xf00) == 0x300)
        {
            pPort->P2PCfg.SigmaSetting.MyConfigMethod = CONFIG_METHOD_KEYPAD;
            DBGPRINT_RAW(RT_DEBUG_TRACE, ( " MyConfigMethod = Keypad  \n"));
        }
        else if ((pHardwareRegister->Data&0xf00) == 0x200)
        {
            pPort->P2PCfg.SigmaSetting.MyConfigMethod = CONFIG_METHOD_DISPLAY;
            DBGPRINT_RAW(RT_DEBUG_TRACE, ( " MyConfigMethod = display  \n"));
        }
        else if ((pHardwareRegister->Data&0xf00) == 0x100)
        {
            pPort->P2PCfg.SigmaSetting.MyConfigMethod = CONFIG_METHOD_LABEL;
            DBGPRINT_RAW(RT_DEBUG_TRACE, ( " MyConfigMethod = label  \n"));
        }
    }
    else if ((pHardwareRegister->Data&0xff) == 0x67)
    {
        if ((pHardwareRegister->Data&0xf00) == 0x100)
        {
            pPort->CommonCfg.P2pControl.field.ExtendListen = 1;
            DBGPRINT_RAW(RT_DEBUG_TRACE, ( ">>> Ext_Listen_Time_Interval=%d, Ext_Listen_Time_Period=%d\n", P2P_EXT_LISTEN_INTERVAL, P2P_EXT_LISTEN_PERIOD));
        }
        else
        {
            pPort->CommonCfg.P2pControl.field.ExtendListen = 0;
        }
        DBGPRINT_RAW(RT_DEBUG_TRACE, ("Set:: P2pControl=>ExtendListen = %d \n", pPort->CommonCfg.P2pControl.field.ExtendListen));
    }
    else if ((pHardwareRegister->Data&0xff) == 0x68)
    {
        if ((pHardwareRegister->Data&0xf00) == 0x100)
            pPort->CommonCfg.P2pControl.field.Managed = 1;
        else
            pPort->CommonCfg.P2pControl.field.Managed = 0;
        DBGPRINT_RAW(RT_DEBUG_TRACE, ("Set:: P2pControl=>Managed = %d \n", pPort->CommonCfg.P2pControl.field.Managed));
        P2PUpdateCapability(pAd, pPort);
    }
    else if ((pHardwareRegister->Data&0xff) == 0x69)
    {
        if ((pHardwareRegister->Data&0xf00) == 0x100)
            pPort->CommonCfg.P2pControl.field.OpPSAlwaysOn = 1;
        else
            pPort->CommonCfg.P2pControl.field.OpPSAlwaysOn = 0;
        pPort->P2PCfg.SigmaSetting.CTWindow = 50;

        if ((pHardwareRegister->Data&0xf000) != 0)
        {
            CTWindow = ((pHardwareRegister->Data) >> 12) & 0xf;
            if (pPort->CommonCfg.P2pControl.field.OpPSAlwaysOn == 1)
                pPort->P2PCfg.SigmaSetting.CTWindow = CTWindow*5;
        }

        // boundary check.
        if (pPort->P2PCfg.SigmaSetting.CTWindow > 90)
            pPort->P2PCfg.SigmaSetting.CTWindow = 90;
        DBGPRINT_RAW(RT_DEBUG_TRACE, ("Set:: 2P2pControl=>OpPSAlwaysOn = %d. CTWindows =  %d \n", pPort->CommonCfg.P2pControl.field.OpPSAlwaysOn, pPort->P2PCfg.SigmaSetting.CTWindow));
        P2PUpdateCapability(pAd, pPort);
    }                   
    else if ((pHardwareRegister->Data&0xff) == 0x6a)
    {
        if ((pHardwareRegister->Data&0xf00) == 0x100)
            pPort->CommonCfg.P2pControl.field.ForceDisableCrossConnect = 1;
        else
            pPort->CommonCfg.P2pControl.field.ForceDisableCrossConnect = 0;
        DBGPRINT_RAW(RT_DEBUG_TRACE, ("Set:: P2pControl=>ForceDisableCrossConnect = %d \n", pPort->CommonCfg.P2pControl.field.ForceDisableCrossConnect));
        P2PUpdateCapability(pAd, pPort);
    }
    
    else if ((pHardwareRegister->Data&0xff) == 0x6b)
    {
        ULONG       PSmode;
        
        if ((pHardwareRegister->Data&0xf00) == 0x100)   // PSPoll
            PSmode = DOT11_POWER_SAVING_MAX_PSP;
        else if ((pHardwareRegister->Data&0xf00) == 0x200)  // Fast
            PSmode = DOT11_POWER_SAVING_FAST_PSP;
        else        // off      
            PSmode = DOT11_POWER_SAVING_NO_POWER_SAVING;
        
        DBGPRINT_RAW(RT_DEBUG_TRACE, ("Set:: Set legacy power saving mode\n"));
        
        if (pAd->StaCfg.WindowsBatteryPowerMode != PSmode)
            MlmeInfoPnpSetPowerMgmtMode(pAd, PSmode);
    }
    else if ((pHardwareRegister->Data&0xff) == 0x6c)
    {
            // Set the STA into legacy sleep mode.

        if ((pHardwareRegister->Data&0xf00) == 0x100)
        {
            DBGPRINT(RT_DEBUG_TRACE, ("Set::PWR_SAVE. %d / %d. \n", pPort->CommonCfg.bAPSDCapable, pPort->CommonCfg.APEdcaParm.bAPSDCapable));
            // WMM-PS
            if ((pPort->CommonCfg.bAPSDCapable) && (pPort->CommonCfg.APEdcaParm.bAPSDCapable))
            {
                pPort->CommonCfg.bAPSDForcePowerSave = TRUE;
                DBGPRINT(RT_DEBUG_TRACE, ("Set :: APSD_PSM \n"));
            }
            else
            {
                pPort->CommonCfg.bAPSDForcePowerSave  = FALSE;
                DBGPRINT(RT_DEBUG_TRACE, ("Set::PWR_SAVE. \n"));
            }
            MlmeInfoPnpSetPowerMgmtMode(pAd, DOT11_POWER_SAVING_MAX_PSP);
        }
        else
        {
                pPort->CommonCfg.bAPSDForcePowerSave  = FALSE;
            MlmeInfoPnpSetPowerMgmtMode(pAd, DOT11_POWER_SAVING_NO_POWER_SAVING);
            DBGPRINT(RT_DEBUG_TRACE, ("Set::PWR_ACTIVE. )\n"));
        }
        DBGPRINT(RT_DEBUG_TRACE, ("Set::pAd->StaCfg.Psm = %d . )\n", pAd->StaCfg.Psm));

    }
    else if ((pHardwareRegister->Data&0xff) == 0x6d)
    {
        // 1. NoADuration
        if ((pHardwareRegister->Data&0xffffff00))
        {
            pPort->P2PCfg.GONoASchedule.Duration = (pHardwareRegister->Data >> 8) * 1024;
            DBGPRINT_RAW(RT_DEBUG_TRACE, ("Set:: NoA_duration=%d(%dTU) \n", pPort->P2PCfg.GONoASchedule.Duration,  (pHardwareRegister->Data >> 8)));
        }
    }
    else if ((pHardwareRegister->Data&0xff) == 0x6e)
    {
        // 2. NoAInterval
        // If NoA Interval is specified as 0 then the STA shall configure the NoA Interval equal to the Beacon Interval  
        //pPort->BeaconPeriod
        if ((pHardwareRegister->Data&0xffffff00))
        {
            pPort->P2PCfg.GONoASchedule.Interval = (pHardwareRegister->Data >> 8) * 1024;
            DBGPRINT_RAW(RT_DEBUG_TRACE, ("Set:: NoA_Interval=%d(%dTU) \n", pPort->P2PCfg.GONoASchedule.Interval,  (pHardwareRegister->Data >> 8)));
        }
        else
            pPort->P2PCfg.GONoASchedule.Interval = 100 * 1024; // If beacon interval is 100ms
    }
    else 
    {
        P2PDebugUseCmd(pAd, pHardwareRegister);
    }

}

UCHAR P2pGetClientWcid(
    IN  PMP_ADAPTER   pAd,
    IN  PMP_PORT      pPort)
{
    PMAC_TABLE_ENTRY pWcidMacTabEntry = NULL;
    UCHAR Wcid =0xff;
#if 0 //(COMPILE_WIN8_ABOVE(CURRENT_OS_NDIS_VER))  
    if (NDIS_WIN8_ABOVE(pAd))
    {
        if (IS_P2P_MS_CLI(pAd, pPort))
            return (P2PCLIWCID[0]);
        //if (IS_P2P_MS_CLI2(pAd, pPort))
            return (P2PCLIWCID[1]);
    }
#endif
    if(pPort->PortNumber == 0)
    {
        return Wcid; //0xff
    }

    if(pPort->PortType == WFD_CLIENT_PORT)
    {
    pWcidMacTabEntry = MlmeSyncMacTabMatchedRoleType(pPort, ROLE_WLANIDX_P2P_CLIENT);

    if(pWcidMacTabEntry != NULL)
    {
        Wcid = (UCHAR)pWcidMacTabEntry->wcid;
    }
    else
    {
            pWcidMacTabEntry = MacTableInsertEntry(pAd, pPort, pPort->CurrentAddress, FALSE);
            Wcid = (UCHAR)pWcidMacTabEntry->wcid;
            pWcidMacTabEntry->WlanIdxRole = ROLE_WLANIDX_P2P_CLIENT;
            }
            }

    
    return Wcid;

}

/* 
    ==========================================================================
    Description:
        decode Dpid 
    Return:
        State string
    NOTE:
    ==========================================================================
*/
 LPSTR decodeDpid (USHORT dpid)
{
    LPSTR retval = "                                 ";
    switch (dpid) 
    {
        case DEV_PASS_ID_PIN:               retval = "DEV_PASS_ID_PIN    ";                 break;
        case DEV_PASS_ID_USER:               retval = "DEV_PASS_ID_USER_SPECIFIED";                 break;
        case DEV_PASS_ID_REG:             retval = "DEV_PASS_ID_REGISTRA_SPECIFIED";               break;
        case DEV_PASS_ID_PBC:               retval = "DEV_PASS_ID_PBC";                 break;
        default:
            retval = "***UNKNOWN dpid***";
    }
    return(retval);
}
/* 
    ==========================================================================
    Description:
        decode ConfigMethod
    Return:
        State string
    NOTE:
    ==========================================================================
*/
 LPSTR decodeP2PRule (USHORT Rule)
{
    LPSTR retval = "                                 ";

    switch (Rule) 
    {
        case P2P_IS_CLIENT:                 retval = "P2P_IS_CLIENT ";                 break;
        case P2P_IS_GO:               retval = "P2P_IS_GO   ";                 break;
        case P2P_IS_DEVICE:               retval = "P2P_IS_DEVICE";                 break;
        default:
            retval = "*** Wrong Rule Code***";
    }
    return(retval);     
}

/* 
    ==========================================================================
    Description:
        decode Internal use error code. for RaUI do display error message for end user.
    Return:
        State string
    NOTE:
    ==========================================================================
*/
LPSTR decodeP2PErrCode (ULONG Code)
{
    LPSTR retval = "                                                        ";

    switch (Code) 
    {
        case P2P_ERRCODE_BOTHINTENT15:                  retval = "Both Intent 15 ";                 break;
        case P2P_ERRCODE_CHANNELDISMATCH:               retval = "No common channel   ";                 break;
        case P2P_ERRCODE_WPSNACK:               retval = "PinCode dismatch";                 break;
        case P2P_ERRCODE_CANTFIND:               retval = "Can't Find peer device  ";                 break;
        case P2P_ERRCODE_NOCONFIGMETHOD:               retval = "Config Method dismatch";                 break;
        case P2P_ERRCODE_CHANNELCONFLICT:   retval = "P2P_ERRCODE_CHANNELCONFLICT";                 break;
        case P2P_ERRCODE_PHYMODECONFLICT:   retval = "P2P_ERRCODE_PHYMODECONFLICT";     break;
        case P2P_ERRCODE_ADHOCON:           retval = "P2P_ERRCODE_ADHOCON";             break;
        default:
            retval = "*** Wrong Error Code***";
    }
    return(retval);     
}

 LPSTR decodeConfigMethod (USHORT ConfigMethos)
{
    LPSTR retval = "                                                  ";
    switch (ConfigMethos) 
    {
        case 0:                             retval = "NOT SPECIFY CONFIG METHOD ";                 break;
        case CONFIG_METHOD_LABEL:               retval = "LABEL  LABEL ";                 break;
        case CONFIG_METHOD_DISPLAY:               retval = "DISPLAY  DISPLAY";                 break;
        case CONFIG_METHOD_KEYPAD:             retval = "KEYPAD  KEYPAD";               break;
        case CONFIG_METHOD_PUSHBUTTON:               retval = "PBC   PBC";                 break;
        default:
            retval = "***Multiple ConfigMethod***";
    }
    return(retval);
}

/* 
    ==========================================================================
    Description:
        decode P2P state
    Return:
        State string
    NOTE:
    ==========================================================================
*/
 LPSTR decodeP2PState (UCHAR P2pState)
{
    LPSTR retval = "                                                                       ";
    switch (P2pState) {
        case P2P_DISABLE:                       retval = "P2P_DISABLE";                     break;
    case P2P_ENABLE_LISTEN_ONLY:            retval = "P2P_ENABLE_LISTEN_ONLY";          break;
        case P2P_IDLE:                          retval = "P2P_IDLE";                        break;
        case P2P_SEARCH_COMMAND:                retval = "P2P_SEARCH_COMMAND";              break;
        case P2P_SEARCH:                        retval = "P2P_SEARCH";                      break;
        case P2P_SEARCH_COMPLETE:               retval = "P2P_SEARCH_COMPLETE";             break;
        case P2P_LISTEN_COMMAND:                retval = "P2P_LISTEN_COMMAND";              break;
        case P2P_LISTEN:                        retval = "P2P_LISTEN";                      break;
        case P2P_LISTEN_COMPLETE:               retval = "P2P_LISTEN_COMPLETE";             break;
    case P2P_CONNECT_IDLE:                  retval = "P2P_CONNECT_IDLE";                break;
    case P2P_INVITE:                        retval = "P2P_INVITE";                      break;
        case P2P_ANY_IN_FORMATION_AS_CLIENT:    retval = "GroupForming as Client";          break;
        case P2P_ANY_IN_FORMATION_AS_GO:        retval = "P2P_ANY_IN_FORMATION_AS_GO";      break;
        case P2P_DO_GO_NEG_DONE_CLIENT:         retval = "P2P_DO_GO_NEG_DONE_CLIENT";       break;
        case P2P_DO_GO_SCAN_OP_BEGIN:           retval = "P2P_DO_GO_SCAN_OP_BEGIN";         break;
        case P2P_DO_GO_SCAN_OP_DONE:            retval = "P2P_DO_GO_SCAN_OP_DONE";          break;
        case P2P_DO_GO_SCAN_BEGIN:              retval = "P2P_DO_GO_SCAN_BEGIN";            break;
        case P2P_DO_GO_SCAN_DONE:               retval = "P2P_DO_GO_SCAN_DONE";             break;
        case P2P_WPS_REGISTRA:                  retval = "P2P_WPS_REGISTRA";                break;
        case P2P_DO_WPS_ENROLLEE:               retval = "P2P_DO_WPS_ENROLLEE";             break;
        case P2P_DO_WPS_DONE_NEED_SETEVENT:     retval = "P2P_DO_WPS_DONE_NEED_SETEVENT";   break;
    case P2P_DO_WPS_ENROLLEE_DONE:          retval = "P2P_DO_WPS_ENROLLEE_DONE";        break;
    case P2P_GO_ASSOC_AUTH:                 retval = "P2P_GO_ASSOC_AUTH";               break;
        case P2P_I_AM_CLIENT_ASSOC_AUTH:        retval = "P2P_I_AM_CLIENT_ASSOC_AUTH";      break;
        case P2P_I_AM_GO_OP:                    retval = "P2P_I_AM_GO_OP";                  break;
        case P2P_I_AM_CLIENT_OP:                retval = "P2P_I_AM_CLIENT_OP";              break;

    // Win8 P2P Sate Machine
    case P2pMs_STATE_IDLE:              retval = "P2pMs_STATE_IDLE";    break;
    case P2pMs_STATE_START_PROGRESS:    retval = "P2pMs_STATE_START_PROGRESS";  break;      
    case P2pMs_STATE_FORMATION:         retval = "P2pMs_STATE_FORMATION";   break;      
    case P2pMs_STATE_INVITATION:        retval = "P2pMs_STATE_INVITATION";  break;
    case P2pMs_STATE_PROVISIONING:      retval = "P2pMs_STATE_PROVISIONING";    break;  

        default:                                    retval = "***UNKNOWN state***";
        }
    return(retval);
}

/* 
    ==========================================================================
    Description:
        decode P2P client state
    Return:
        State string
    NOTE:
    ==========================================================================
*/
 LPSTR decodeP2PClientState (P2P_CLIENT_STATE P2pClientState)
{
    LPSTR retval = "                                                                        ";
    switch (P2pClientState) {
        case P2PSTATE_NONE:                     retval = "P2PSTATE_NONE";                   break;
        case P2PSTATE_DISCOVERY:                retval = "P2PSTATE_DISCOVERY";              break;
        case P2PSTATE_DISCOVERY_GO:             retval = "P2PSTATE_DISCOVERY_GO";           break;
        case P2PSTATE_DISCOVERY_CLIENT:         retval = "P2PSTATE_DISCOVERY_CLIENT";       break;
        case P2PSTATE_DISCOVERY_UNKNOWN:        retval = "P2PSTATE_DISCOVERY_UNKNOWN";      break;
        case P2PSTATE_CLIENT_DISCO_COMMAND:     retval = "P2PSTATE_CLIENT_DISCO_COMMAND";   break;
        case P2PSTATE_WAIT_GO_DISCO_ACK:        retval = "P2PSTATE_WAIT_GO_DISCO_ACK";      break;
        case P2PSTATE_WAIT_GO_DISCO_ACK_SUCCESS:    retval = "P2PSTATE_WAIT_GO_DISCO_ACK_SUCCESS";  break;
        case P2PSTATE_GO_DISCO_COMMAND:         retval = "P2PSTATE_GO_DISCO_COMMAND";       break;
        case P2PSTATE_INVITE_COMMAND:           retval = "P2PSTATE_INVITE_COMMAND";         break;
        case P2PSTATE_CONNECT_COMMAND:          retval = "P2PSTATE_CONNECT_COMMAND";        break;
        case P2PSTATE_PROVISION_COMMAND:        retval = "P2PSTATE_PROVISION_COMMAND";      break;
        case P2PSTATE_SERVICE_DISCO_COMMAND:    retval = "P2PSTATE_SERVICE_DISCO_COMMAND";  break;
        case P2PSTATE_SERVICE_COMEBACK_COMMAND: retval = "P2PSTATE_SERVICE_COMEBACK_COMMAND";   break;
        case P2PSTATE_SENT_INVITE_REQ:          retval = "P2PSTATE_SENT_INVITE_REQ";        break;
        case P2PSTATE_SENT_PROVISION_REQ:       retval = "P2PSTATE_SENT_PROVISION_REQ";     break;
        case P2PSTATE_WAIT_REVOKEINVITE_RSP_ACK:    retval = "P2PSTATE_WAIT_REVOKEINVITE_RSP_ACK";      break;
        case P2PSTATE_REVOKEINVITE_RSP_ACK_SUCCESS: retval = "P2PSTATE_REVOKEINVITE_RSP_ACK_SUCCESS";   break;
        case P2PSTATE_SENT_GO_NEG_REQ:          retval = "P2PSTATE_SENT_GO_NEG_REQ";        break;
        case P2PSTATE_GOT_GO_RSP_INFO_UNAVAI:   retval = "P2PSTATE_GOT_GO_RSP_INFO_UNAVAI"; break;
        case P2PSTATE_WAIT_GO_COMFIRM:          retval = "P2PSTATE_WAIT_GO_COMFIRM";        break;  
        case P2PSTATE_WAIT_GO_COMFIRM_ACK:      retval = "P2PSTATE_WAIT_GO_COMFIRM_ACK";    break;  
        case P2PSTATE_GOT_GO_COMFIRM:           retval = "P2PSTATE_GOT_GO_COMFIRM";         break;  
        case P2PSTATE_GO_COMFIRM_ACK_SUCCESS:   retval = "P2PSTATE_GO_COMFIRM_ACK_SUCCESS"; break;
        case P2PSTATE_REINVOKEINVITE_TILLCONFIGTIME:    retval = "P2PSTATE_REINVOKEINVITE_TILLCONFIGTIME";   break;     
        case P2PSTATE_GO_DONE:                  retval = "P2PSTATE_GO_DONE";                break;  
        case P2PSTATE_GO_WPS:                   retval = "P2PSTATE_GO_WPS";                 break;  
        case P2PSTATE_GO_AUTH:                  retval = "P2PSTATE_GO_AUTH";                break;
        case P2PSTATE_GO_ASSOC:                 retval = "P2PSTATE_GO_ASSOC";               break;
        case P2PSTATE_CLIENT_WPS:               retval = "P2PSTATE_CLIENT_WPS";             break;
        case P2PSTATE_CLIENT_WPS_DONE:          retval = "P2PSTATE_CLIENT_WPS_DONE";        break;
        case P2PSTATE_CLIENT_AUTH:              retval = "P2PSTATE_CLIENT_AUTH";            break;
        case P2PSTATE_CLIENT_ASSOC:             retval = "P2PSTATE_CLIENT_ASSOC";           break;
        case P2PSTATE_CLIENT_OPERATING:         retval = "P2PSTATE_CLIENT_OPERATING";       break;
        case P2PSTATE_GO_OPERATING:             retval = "P2PSTATE_GO_OPERATING";           break;  
        case P2PSTATE_NONP2P_PSK:               retval = "P2PSTATE_NONP2P_PSK";             break;
        case P2PSTATE_NONP2P_WPS:               retval = "P2PSTATE_NONP2P_WPS";             break;
        default:    
        retval = "***UNKNOWN state***";
        }
    return(retval);
}

#ifdef MULTI_CHANNEL_SUPPORT
LPSTR decodeMultiChannelMode (USHORT MultiChannelCurrentMode)
{
    LPSTR retval = "                                                  ";
    switch (MultiChannelCurrentMode) 
    {
        case MULTI_CH_OP_MODE_INFRA:                retval = "MULTI_CH_OP_MODE_INFRA";      break;
        case MULTI_CH_OP_MODE_MSCLI:                retval = "MULTI_CH_OP_MODE_MSCLI";      break;
        case MULTI_CH_OP_MODE_MSGO:                 retval = "MULTI_CH_OP_MODE_MSGO";       break;
        default:
            retval = "***Unknown MultiChannelMode***";
    }
    return(retval);
}
#endif /*MULTI_CHANNEL_SUPPORT*/

/* 
    ==========================================================================
    Description:
        debug print function for P2P.
    Return:

    NOTE:
         
    ==========================================================================
*/
VOID P2PPrintPort(
    IN PMP_ADAPTER pAd)
{
    PMP_PORT pPort;
    pPort = pAd->PortList[FXXK_PORT_0];

     if (P2P_OFF(pPort) || (pPort == NULL))
        return;
     
    DBGPRINT(RT_DEBUG_TRACE, ("!!P2P PortNumber = %d,   .\n", pPort->P2PCfg.PortNumber));
    DBGPRINT(RT_DEBUG_TRACE, ("!!APcfg.ChannelWidth = %d, RecomWidth = %d.  ExtChanOffset = %d.\n", pPort->SoftAP.ApCfg.HtCapability.HtCapInfo.ChannelWidth, pPort->SoftAP.ApCfg.AddHTInfoIe.AddHtInfo.RecomWidth, pPort->SoftAP.ApCfg.AddHTInfoIe.AddHtInfo.ExtChanOffset));

    DBGPRINT(RT_DEBUG_TRACE, ("!!Bssid = %x %x %x %x %x %x \n",  pPort->PortCfg.Bssid[0], pPort->PortCfg.Bssid[1], 
        pPort->PortCfg.Bssid[2],pPort->PortCfg.Bssid[3], pPort->PortCfg.Bssid[4],pPort->PortCfg.Bssid[5]));
    DBGPRINT(RT_DEBUG_TRACE, ("!!CurrentAddress = %x %x %x %x %x %x \n",  pPort->CurrentAddress[0], pPort->CurrentAddress[1], 
        pPort->CurrentAddress[2],pPort->CurrentAddress[3], pPort->CurrentAddress[4],pPort->CurrentAddress[5]));
    DBGPRINT(RT_DEBUG_TRACE, ("!!authmode = %d, WepStatus = %d.   \n", pPort->PortCfg.AuthMode, pPort->PortCfg.WepStatus));
    DBGPRINT(RT_DEBUG_TRACE, ("!!PortSubtype = %d. SsidLen = %d,  \n", pPort->PortSubtype, pPort->PortCfg.SsidLen));
}
/* 
    ==========================================================================
    Description:
        debug print function for P2P.
    Return:

    NOTE:
         
    ==========================================================================
*/
VOID P2PPrintMac(
    IN PMP_ADAPTER pAd, 
    IN UCHAR    macindex)
{
    MAC_TABLE_ENTRY *pEntry;
    PMP_PORT    pPort = pAd->PortList[FXXK_PORT_0];
    PMAC_TABLE_ENTRY pWcidMacTabEntry = NULL;
    pWcidMacTabEntry = MlmeSyncMacTabMatchedWlanIdx(pPort, macindex);      
    if(pWcidMacTabEntry == NULL)
    {
        DBGPRINT(RT_DEBUG_TRACE,("%s(), Line:%d, Mac Entry is NULL  Wcid = %d\n", __FUNCTION__, __LINE__, macindex));
        return;
    }
    
    DBGPRINT(RT_DEBUG_TRACE, (" ====================================================>P2PPrintMac i = %d,   \n", macindex));
    pEntry = pWcidMacTabEntry;
    DBGPRINT(RT_DEBUG_TRACE, ("!!ClientStatusFlags = %x,  PhyCfg = %x, WepStatus = %d, AuthMode = %d. \n", pEntry->ClientStatusFlags, READ_PHY_CFG_DOUBLE_WORD(pAd, &pEntry->TxPhyCfg), pEntry->WepStatus, pEntry->AuthMode));

    DBGPRINT(RT_DEBUG_TRACE, ("ValidAsCLI = %d. ValidAsP2P = %d. Psm = %d. ClientStatusFlags = %x, \n", pEntry->ValidAsCLI, pEntry->ValidAsP2P, pEntry->PsMode, pEntry->ClientStatusFlags));
    DBGPRINT(RT_DEBUG_TRACE, (" %s, \n", decodeP2PClientState(pEntry->P2pInfo.P2pClientState)));

    DBGPRINT(RT_DEBUG_TRACE, ("devaddr = %x %x %x %x %x %x\n", pEntry->P2pInfo.DevAddr[0], pEntry->P2pInfo.DevAddr[1], pEntry->P2pInfo.DevAddr[2],pEntry->P2pInfo.DevAddr[3],pEntry->P2pInfo.DevAddr[4],pEntry->P2pInfo.DevAddr[5]));
    DBGPRINT(RT_DEBUG_TRACE, ("interface addr =   %x %x %x %x %x %x\n", pEntry->P2pInfo.InterfaceAddr[0], pEntry->P2pInfo.InterfaceAddr[1], pEntry->P2pInfo.InterfaceAddr[2],pEntry->P2pInfo.InterfaceAddr[3],pEntry->P2pInfo.InterfaceAddr[4],pEntry->P2pInfo.InterfaceAddr[5]));
    DBGPRINT(RT_DEBUG_TRACE, ("DeviceType = %x %x %x %x %x %x %x %x\n", pEntry->P2pInfo.PrimaryDevType[0], pEntry->P2pInfo.PrimaryDevType[1], pEntry->P2pInfo.PrimaryDevType[2],pEntry->P2pInfo.PrimaryDevType[3],pEntry->P2pInfo.PrimaryDevType[4],pEntry->P2pInfo.PrimaryDevType[5],pEntry->P2pInfo.PrimaryDevType[6],pEntry->P2pInfo.PrimaryDevType[7]));
    DBGPRINT(RT_DEBUG_TRACE, ("NumSecondaryType = %d\n", pEntry->P2pInfo.NumSecondaryType));
    DBGPRINT(RT_DEBUG_TRACE, ("DeviceName = %c%c%c%c%c%c%c%c%c\n", pEntry->P2pInfo.DeviceName[0], pEntry->P2pInfo.DeviceName[1], pEntry->P2pInfo.DeviceName[2],pEntry->P2pInfo.DeviceName[3],pEntry->P2pInfo.DeviceName[4],pEntry->P2pInfo.DeviceName[5],pEntry->P2pInfo.DeviceName[6],pEntry->P2pInfo.DeviceName[7],pEntry->P2pInfo.DeviceName[8]));
    DBGPRINT(RT_DEBUG_TRACE, ("DevCap = %x, GrpCap = %x, CTWindow = %d . \n", pEntry->P2pInfo.DevCapability, pEntry->P2pInfo.GroupCapability, pEntry->P2pInfo.CTWindow));
}

/* 
    ==========================================================================
    Description:
        debug print function for P2P Table entry.
    Return:

    NOTE:
         
    ==========================================================================
*/
VOID P2PPrintP2PEntry(
    IN PMP_ADAPTER pAd,
    IN UCHAR        p2pindex)
{
    RT_P2P_CLIENT_ENTRY *pClient;

    if (p2pindex >= MAX_P2P_GROUP_SIZE)
        return;
    
    DBGPRINT(RT_DEBUG_TRACE, (" ====================================================>P2PPrintP2PEntry i = %d,   \n", p2pindex));
    pClient = &pAd->pP2pCtrll->P2PTable.Client[p2pindex];
    DBGPRINT(RT_DEBUG_TRACE, ("P2pClientState = %s  \n", decodeP2PClientState(pClient->P2pClientState)));
    DBGPRINT(RT_DEBUG_TRACE, ("MyGOIndex = %d. rule = %d  , addr = %x %x %x %x %x %x\n", pClient->MyGOIndex, pClient->Rule, pClient->addr[0], pClient->addr[1], pClient->addr[2],pClient->addr[3],pClient->addr[4],pClient->addr[5]));
    DBGPRINT(RT_DEBUG_TRACE, ("bssid =   %x %x %x %x %x %x\n", pClient->bssid[0], pClient->bssid[1], pClient->bssid[2],pClient->bssid[3],pClient->bssid[4],pClient->bssid[5]));
    DBGPRINT(RT_DEBUG_TRACE, ("interface addr =   %x %x %x %x %x %x\n", pClient->InterfaceAddr[0], pClient->InterfaceAddr[1], pClient->InterfaceAddr[2],pClient->InterfaceAddr[3],pClient->InterfaceAddr[4],pClient->InterfaceAddr[5]));
    DBGPRINT(RT_DEBUG_TRACE, ("DeviceType = %x %x %x %x %x %x %x %x\n", pClient->PrimaryDevType[0], pClient->PrimaryDevType[1], pClient->PrimaryDevType[2],pClient->PrimaryDevType[3],pClient->PrimaryDevType[4],pClient->PrimaryDevType[5],pClient->PrimaryDevType[6],pClient->PrimaryDevType[7]));
    DBGPRINT(RT_DEBUG_TRACE, ("NumSecondaryType = %d\n", pClient->NumSecondaryType));
    DBGPRINT(RT_DEBUG_TRACE, ("Len = %d. DeviceName = %c%c%c%c%c%c%c%c%c\n", pClient->DeviceNameLen, pClient->DeviceName[0], pClient->DeviceName[1], pClient->DeviceName[2],pClient->DeviceName[3],pClient->DeviceName[4],pClient->DeviceName[5],pClient->DeviceName[6],pClient->DeviceName[7],pClient->DeviceName[8]));
    DBGPRINT(RT_DEBUG_TRACE, ("DevCap = %x, GrpCap = %x, ConfigMethod = %x . %s \n", pClient->DevCapability, pClient->GroupCapability, pClient->ConfigMethod, decodeConfigMethod(pClient->ConfigMethod)));
    DBGPRINT(RT_DEBUG_TRACE, ("Opchannel = %x, Listenchannel = %x,NumSecondaryType = %x, RegClass = %x. ConfigTimeOut = %x. \n", pClient->OpChannel, pClient->ListenChannel, pClient->NumSecondaryType, pClient->RegClass, pClient->ConfigTimeOut));


}

/* 
    ==========================================================================
    Description:
        debug print function for P2P Table entry.
    Return:

    NOTE:
         
    ==========================================================================
*/
VOID P2PPrintDiscoEntry(
    IN PMP_ADAPTER pAd,
    IN UCHAR        p2pindex)
{
    RT_P2P_DISCOVERY_ENTRY      *pClient;
//  UCHAR       i;

    if (p2pindex >= MAX_P2P_DISCOVERY_SIZE)
        return;
    
    if (pAd->pP2pCtrll->P2PTable.DiscoEntry[p2pindex].bValid == FALSE)
        return;
    
    DBGPRINT(RT_DEBUG_TRACE, ("P2PPrintDiscoEntry [#%d] =========================================> \n", p2pindex));
    pClient = &pAd->pP2pCtrll->P2PTable.DiscoEntry[p2pindex];
    DBGPRINT(RT_DEBUG_TRACE, ("SSID =       %s\n", pClient->Ssid)); 
    DBGPRINT(RT_DEBUG_TRACE, ("TxAddr =     %x %x %x %x %x %x\n", pClient->TransmitterAddr[0], pClient->TransmitterAddr[1], pClient->TransmitterAddr[2], pClient->TransmitterAddr[3], pClient->TransmitterAddr[4], pClient->TransmitterAddr[5]));
    DBGPRINT(RT_DEBUG_TRACE, ("DevAddr =    %x %x %x %x %x %x\n", pClient->DeviceId[0], pClient->DeviceId[1], pClient->DeviceId[2],pClient->DeviceId[3],pClient->DeviceId[4],pClient->DeviceId[5]));
    DBGPRINT(RT_DEBUG_TRACE, ("Bssid =      %x %x %x %x %x %x\n", pClient->Bssid[0], pClient->Bssid[1], pClient->Bssid[2],pClient->Bssid[3],pClient->Bssid[4],pClient->Bssid[5]));
    DBGPRINT(RT_DEBUG_TRACE, ("DeviceType = %x %x %x %x %x %x %x %x\n", pClient->PrimaryDevType[0], pClient->PrimaryDevType[1], pClient->PrimaryDevType[2],pClient->PrimaryDevType[3],pClient->PrimaryDevType[4],pClient->PrimaryDevType[5],pClient->PrimaryDevType[6],pClient->PrimaryDevType[7]));
    DBGPRINT(RT_DEBUG_TRACE, ("NumSecondaryType =   %d\n", pClient->NumSecondaryType));
    DBGPRINT(RT_DEBUG_TRACE, ("Opchannel = %d, Listenchannel = %d.\n", pClient->OpChannel, pClient->ListenChannel));
    DBGPRINT(RT_DEBUG_TRACE, ("BssType = %d\n", pClient->BssType));
    DBGPRINT(RT_DEBUG_TRACE, ("CapabilityInfo = %d, DevCapability = 0x%x, GroupCapability = 0x%x\n", pClient->CapabilityInfo, pClient->DevCapability, pClient->GroupCapability));   
    DBGPRINT(RT_DEBUG_TRACE, ("Rssi = %d\n", pClient->Rssi));
    DBGPRINT(RT_DEBUG_TRACE, ("BeaconFrameLen = %d\n", pClient->BeaconFrameLen));
    DBGPRINT(RT_DEBUG_TRACE, ("ProbeRspFrameLen = %d\n", pClient->ProbeRspFrameLen));   
    DBGPRINT(RT_DEBUG_TRACE, ("BeaconPeriod = %d\n", pClient->BeaconPeriod));
    DBGPRINT(RT_DEBUG_TRACE, ("ProbeRspTimestamp = %lld\n", pClient->ProbeRspTimestamp));
    DBGPRINT(RT_DEBUG_TRACE, ("BeaconTimestamp = %lld\n", pClient->BeaconTimestamp));
    DBGPRINT(RT_DEBUG_TRACE, ("LifeTime = %d\n", pClient->LifeTime));
    DBGPRINT(RT_DEBUG_TRACE, ("IsLegacyNetwork = %d\n", pClient->bLegacyNetworks));
    DBGPRINT(RT_DEBUG_TRACE, ("\n"));

}

/* 
    ==========================================================================
    Description:
        debug print function for P2P Persistent Table entry.
    Return:

    NOTE:
         
    ==========================================================================
*/
VOID P2PPrintP2PPerstEntry(
    IN PMP_ADAPTER pAd,
    IN UCHAR        p2pindex)
{
    RT_P2P_PERSISTENT_ENTRY *pClient;

    if (p2pindex >= MAX_P2P_TABLE_SIZE)
        return;
    
    DBGPRINT(RT_DEBUG_TRACE, (" P2PPrintP2PPerstEntry i = %d,   \n", p2pindex));
    pClient = &pAd->pP2pCtrll->P2PTable.PerstEntry[p2pindex];
    DBGPRINT(RT_DEBUG_TRACE, (" addr = %x %x %x %x %x %x\n",  pClient->Addr[0], pClient->Addr[1], pClient->Addr[2],pClient->Addr[3],pClient->Addr[4],pClient->Addr[5]));
    DBGPRINT(RT_DEBUG_TRACE, ("Key = %x %x %x %x %x %x %x %x\n", pClient->Profile.Key[0], pClient->Profile.Key[1], pClient->Profile.Key[2],pClient->Profile.Key[3],pClient->Profile.Key[4],pClient->Profile.Key[5],pClient->Profile.Key[6],pClient->Profile.Key[7]));
    DBGPRINT(RT_DEBUG_TRACE, ("MacAddr = %x %x %x %x %x %x\n", pClient->Profile.MacAddr[0], pClient->Profile.MacAddr[1], pClient->Profile.MacAddr[2],pClient->Profile.MacAddr[3],pClient->Profile.MacAddr[4],pClient->Profile.MacAddr[5]));
    DBGPRINT(RT_DEBUG_TRACE, ("Ssid %d = DIRECT-%c%c\n", pClient->Profile.SSID.SsidLength,pClient->Profile.SSID.Ssid[7], pClient->Profile.SSID.Ssid[8]));
    DBGPRINT(RT_DEBUG_TRACE, ("MyRule = %x, bValid = %x,    \n", pClient->MyRule, pClient->bValid));


}

/* 
    ==========================================================================
    Description:
        debug print function for P2P.
    Return:

    NOTE:
         
    ==========================================================================
*/
VOID P2PPrintState(
    IN PMP_ADAPTER pAd)
{
    PMP_PORT    pPort = pAd->PortList[FXXK_PORT_0];
     if (P2P_ON(pPort))
    {       
        DBGPRINT(RT_DEBUG_TRACE, ("!!=======================================>. \n"));
        if (pAd->PortList[pPort->P2PCfg.PortNumber])
            DBGPRINT(RT_DEBUG_TRACE,("Port Number-Type-SubType = %d-%d-%d \n", pPort->P2PCfg.PortNumber, pAd->PortList[pPort->P2PCfg.PortNumber]->PortType, pAd->PortList[pPort->P2PCfg.PortNumber]->PortSubtype));
        DBGPRINT(RT_DEBUG_TRACE, ("bWMMPSInAbsent = %d. dpid = %x, config = %x , BW = %d. Cha = %d . CenCha = %d.  OpCh = %d. ListenCh= %d. \n",  pPort->P2PCfg.GONoASchedule.bWMMPSInAbsent, pPort->P2PCfg.Dpid,pPort->P2PCfg.ConfigMethod,pPort->BBPCurrentBW, pPort->Channel, pPort->CentralChannel, pPort->P2PCfg.GroupOpChannel, pPort->P2PCfg.ListenChannel));
        DBGPRINT(RT_DEBUG_TRACE, (" CTWindows = %x. GONoAScheduleValid = %d. Capability = %x. GrpCap = %x. P2Pcontrol = %x.\n", pPort->P2PCfg.CTWindows, pPort->P2PCfg.GONoASchedule.bValid, pPort->P2PCfg.P2pCapability[0], pPort->P2PCfg.P2pCapability[1], pPort->CommonCfg.P2pControl.word));
        DBGPRINT(RT_DEBUG_TRACE, (" SSIDLen = %d/%d. 1 PIN = %x  %x. 2 PIN = %x  %x\n", pPort->P2PCfg.SSIDLen, pAd->PortList[pPort->P2PCfg.PortNumber]->PortCfg.SsidLen, *(PULONG)&pPort->P2PCfg.PinCode[0], *(PULONG)&pPort->P2PCfg.PinCode[4], *(PULONG)&pPort->StaCfg.WscControl.RegData.PIN[0], *(PULONG)&pPort->StaCfg.WscControl.RegData.PIN[4]));
        DBGPRINT(RT_DEBUG_TRACE, ("!! %d  = %s.   %d  = %s. \n", pPort->P2PCfg.P2PConnectState, decodeP2PState(pPort->P2PCfg.P2PConnectState), pPort->P2PCfg.P2PDiscoProvState,decodeP2PState(pPort->P2PCfg.P2PDiscoProvState)));

        DBGPRINT(RT_DEBUG_TRACE, ("!!<======================================== \n"));
    }
}

/* 
    ==========================================================================
    Description:
        debug print function for P2P.
    Return:

    NOTE:
         
    ==========================================================================
*/
VOID P2PPrintStateOnly(
    IN PMP_ADAPTER pAd,
    IN PMP_PORT     pPort)
{    
     if (P2P_ON(pPort))
    {       
        DBGPRINT(RT_DEBUG_TRACE, ("!! %d  = %s. \n", pPort->P2PCfg.P2PConnectState, decodeP2PState(pPort->P2PCfg.P2PConnectState)));
        DBGPRINT(RT_DEBUG_TRACE, ("!! %d  = %s. \n", pPort->P2PCfg.P2PDiscoProvState,decodeP2PState(pPort->P2PCfg.P2PDiscoProvState)));
    }
}

/* 
    ==========================================================================
    Description:
        Exit whck cpu check 
    Return:

    NOTE:
         
    ==========================================================================
*/
VOID P2PExitWhckCpuCheck(
    IN PMP_ADAPTER pAd)
{
    BOOLEAN         Cancelled = FALSE;
    PMP_PORT    pPort = pAd->PortList[FXXK_PORT_0];
    DBGPRINT(RT_DEBUG_TRACE, ("%s ===>\n", __FUNCTION__));
    PlatformCancelTimer(&pPort->Mlme.WHCKCPUUtilCheckTimer, &Cancelled);
    pAd->LogoTestCfg.WhckDataCntForPast40Sec = 0;
    pAd->LogoTestCfg.WhckDataCntForPast60Sec = 0;
    pAd->LogoTestCfg.WhckFirst40Secs = TRUE;
    pAd->LogoTestCfg.WhckCpuUtilTestRunning = FALSE;
    RTUSBWriteMACRegister(pAd, RX_FILTR_CFG, STANORMAL);
    DBGPRINT(RT_DEBUG_TRACE, ("%s <===\n", __FUNCTION__));
}

#ifdef MULTI_CHANNEL_SUPPORT
/* 
    ==========================================================================
    Description:
        This function is the entry point for multi channel
    Return:

    NOTE:
         
    ==========================================================================
*/
VOID MultiChannelSwitchTimerCallback(
    IN PVOID SystemSpecific1, 
    IN PVOID FunctionContext, 
    IN PVOID SystemSpecific2, 
    IN PVOID SystemSpecific3)
{
    PMP_PORT pPort = (PMP_PORT )FunctionContext;
    PMP_ADAPTER  pAd = pPort->pAd;

    MTEnqueueInternalCmd(pAd, pPort, MT_CMD_MULTI_CHANNEL_SWITCH_EXEC, NULL, 0);
}

/* 
    ==========================================================================
    Description:
        This fucntion is the body of multi channel which handles session of start/stop/fail, state and channel switch
    Return:

    NOTE:
         
    ==========================================================================
*/
VOID MultiChannelSwitchExec(
    IN PMP_PORT pPort,
    IN PUCHAR pInBuffer)
{
    PMP_ADAPTER pAd = pPort->pAd;
    
    if (pAd == NULL)
    {
        DBGPRINT(RT_DEBUG_TRACE, ("%s - pAd = NULL, Error and  RETURN\n", __FUNCTION__));
        return;
    }

    // Handle new multi-channel state with end conenction
    if (MT_TEST_BIT(pAd->pP2pCtrll->MultiChannelStartOrEndProcess, MULTI_CHANNEL_GO_STOP) ||
        MT_TEST_BIT(pAd->pP2pCtrll->MultiChannelStartOrEndProcess, MULTI_CHANNEL_CLI_STOP) ||
        MT_TEST_BIT(pAd->pP2pCtrll->MultiChannelStartOrEndProcess, MULTI_CHANNEL_STA_STOP))
    {
        if (MT_TEST_BIT(pAd->pP2pCtrll->MultiChannelStartOrEndProcess, MULTI_CHANNEL_GO_STOP))
        {
            MT_CLEAR_BIT(pAd->pP2pCtrll->MultiChannelStartOrEndProcess, MULTI_CHANNEL_GO_STOP);
            DBGPRINT(RT_DEBUG_TRACE, ("%s - MultiChannelSwitchTimerCallback : Stop GO and start to handle the multi-channel state\n", __FUNCTION__));
            MultiChannelGOStop(pAd);
        }
        else if (MT_TEST_BIT(pAd->pP2pCtrll->MultiChannelStartOrEndProcess, MULTI_CHANNEL_CLI_STOP))
        {
            MT_CLEAR_BIT(pAd->pP2pCtrll->MultiChannelStartOrEndProcess, MULTI_CHANNEL_CLI_STOP);
            DBGPRINT(RT_DEBUG_TRACE, ("%s - MultiChannelSwitchTimerCallback : Stop CLI and start to handle the multi-channel state\n", __FUNCTION__));
            MultiChannelCLIStop(pAd);
        }
        else if (MT_TEST_BIT(pAd->pP2pCtrll->MultiChannelStartOrEndProcess, MULTI_CHANNEL_STA_STOP))
        {
            MT_CLEAR_BIT(pAd->pP2pCtrll->MultiChannelStartOrEndProcess, MULTI_CHANNEL_STA_STOP);
            DBGPRINT(RT_DEBUG_TRACE, ("%s - MultiChannelSwitchTimerCallback : Stop STA and start to handle the multi-channel state\n", __FUNCTION__));
            MultiChannelSTAStop(pAd);
        }

        MultiChannelDeferRequestIndication(pAd, pInBuffer);
    }
    // Handle new multi-channel state with new conenction
    else if (MT_TEST_BIT(pAd->pP2pCtrll->MultiChannelStartOrEndProcess, MULTI_CHANNEL_GO_NEW_CONNECTION_START) ||
        MT_TEST_BIT(pAd->pP2pCtrll->MultiChannelStartOrEndProcess, MULTI_CHANNEL_CLI_OUT_NEW_CONNECTION_START) ||
        MT_TEST_BIT(pAd->pP2pCtrll->MultiChannelStartOrEndProcess, MULTI_CHANNEL_CLI_IN_NEW_CONNECTION_START) ||
        MT_TEST_BIT(pAd->pP2pCtrll->MultiChannelStartOrEndProcess, MULTI_CHANNEL_STA_NEW_CONNECTION_START))
    {       
        if (MT_TEST_BIT(pAd->pP2pCtrll->MultiChannelStartOrEndProcess, MULTI_CHANNEL_GO_NEW_CONNECTION_START))
        {
            MT_CLEAR_BIT(pAd->pP2pCtrll->MultiChannelStartOrEndProcess, MULTI_CHANNEL_GO_NEW_CONNECTION_START);
            DBGPRINT(RT_DEBUG_TRACE, ("%s - MultiChannelSwitchTimerCallback : GO starts to handle multi-channel state with new conenction\n", __FUNCTION__));
            MultiChannelGOStartNewConnection(pAd);
            MultiChannelDeferRequestIndication(pAd, pInBuffer);
        }
        else if (MT_TEST_BIT(pAd->pP2pCtrll->MultiChannelStartOrEndProcess, MULTI_CHANNEL_CLI_OUT_NEW_CONNECTION_START))
        {
            MT_CLEAR_BIT(pAd->pP2pCtrll->MultiChannelStartOrEndProcess, MULTI_CHANNEL_CLI_OUT_NEW_CONNECTION_START);
            DBGPRINT(RT_DEBUG_TRACE, ("%s - MultiChannelSwitchTimerCallback : CLI starts to handle multi-channel state with new OUT conenction\n", __FUNCTION__));
            MultiChannelCLIOutStartNewConnection(pPort);
        }
        else if (MT_TEST_BIT(pAd->pP2pCtrll->MultiChannelStartOrEndProcess, MULTI_CHANNEL_CLI_IN_NEW_CONNECTION_START))
        {
            MT_CLEAR_BIT(pAd->pP2pCtrll->MultiChannelStartOrEndProcess, MULTI_CHANNEL_CLI_IN_NEW_CONNECTION_START);
            DBGPRINT(RT_DEBUG_TRACE, ("%s - MultiChannelSwitchTimerCallback : CLI starts to handle multi-channel state with new IN conenction\n", __FUNCTION__));
            MultiChannelCLIInStartNewConnection(pAd, pPort);
        }
        else if (MT_TEST_BIT(pAd->pP2pCtrll->MultiChannelStartOrEndProcess, MULTI_CHANNEL_STA_NEW_CONNECTION_START))
        {
            MT_CLEAR_BIT(pAd->pP2pCtrll->MultiChannelStartOrEndProcess, MULTI_CHANNEL_STA_NEW_CONNECTION_START);
            DBGPRINT(RT_DEBUG_TRACE, ("%s - MultiChannelSwitchTimerCallback : STA starts to handle multi-channel state with new conenction\n", __FUNCTION__));
            MultiChannelSTAStartNewConnection(pAd, pPort);
        }
    }
    else if (MT_TEST_BIT(pAd->pP2pCtrll->MultiChannelStartOrEndProcess, MULTI_CHANNEL_NEW_P2P_CONNECTION_FAIL))
    {
        MT_CLEAR_BIT(pAd->pP2pCtrll->MultiChannelStartOrEndProcess, MULTI_CHANNEL_NEW_P2P_CONNECTION_FAIL);
        DBGPRINT(RT_DEBUG_TRACE, ("%s - MultiChannelSwitchTimerCallback : Starts to handle MULTI_CHANNEL_NEW_P2P_CONNECTION_FAIL.\n", __FUNCTION__));
        MultiChannelNewP2pConnectionFail(pAd);
    }
    else if (MT_TEST_BIT(pAd->pP2pCtrll->MultiChannelStartOrEndProcess, MULTI_CHANNEL_CLI_DEBUG))
    {
        MT_CLEAR_BIT(pAd->pP2pCtrll->MultiChannelStartOrEndProcess, MULTI_CHANNEL_CLI_DEBUG);
        MultiChannelDeferDisconnectRequest(pAd, pAd->MccCfg.pDisconnectPort);
    }
    // Handle normal multi-channel state
    else
    {
        DBGPRINT(RT_DEBUG_TRACE, ("%s - Default(%d) ===>\n", __FUNCTION__, MT_TEST_BIT(pAd->pP2pCtrll->MultiChannelStartOrEndProcess, MULTI_CHANNEL_IN_PROCESS)));
        
        if (pAd->MccCfg.MultiChannelEnable == TRUE)
        {
            if (MT_TEST_BIT(pAd->pP2pCtrll->MultiChannelStartOrEndProcess, MULTI_CHANNEL_IN_PROCESS))
            {
                DBGPRINT(RT_DEBUG_TRACE, ("%s - MultiChSts : MULTI_CHANNEL_IN_PROCESS is running, RETURN!\n", __FUNCTION__));
                return;
            }

            MT_SET_BIT(pAd->pP2pCtrll->MultiChannelStartOrEndProcess, MULTI_CHANNEL_IN_PROCESS);
            MultiChannelSwitchKernel(pAd);
            MT_CLEAR_BIT(pAd->pP2pCtrll->MultiChannelStartOrEndProcess, MULTI_CHANNEL_IN_PROCESS);
        }
        
        DBGPRINT(RT_DEBUG_TRACE, ("%s - Default(%d) <===\n", __FUNCTION__, MT_TEST_BIT(pAd->pP2pCtrll->MultiChannelStartOrEndProcess, MULTI_CHANNEL_IN_PROCESS)));
    }
}

/* 
    ==========================================================================
    Description:
        This is multi channel kernel which maintains state and channel switch
    Return:

    NOTE:
         
    ==========================================================================
*/
VOID MultiChannelSwitchKernel(
    IN PMP_ADAPTER pAd) 
{
    PMP_PORT  pPort = NULL;
    //BOOLEAN       TimerCancelled;
    UCHAR       Channel = pPort->P2PCfg.InfraChannel;
    ULONG       HwQSel = FIFO_EDCA;
    UCHAR       SwitchStatus = 0;
    UINT32      Count = 0;
    PBF_CFG_STRUC_EXT               PbfCfg = {0};
    TXRXQ_PCNT_STRUC            TxRxQPcnt = {0};
    PBF_DBG_STRUC               PbfDbg = {0};
    MULTI_CH_OP_MODE    PreviousMultiChannelMode = pAd->MccCfg.MultiChannelCurrentMode;
    UCHAR       ExtChanOffset = EXTCHA_NONE;
    NDIS_STATUS ndisStatus = NDIS_STATUS_SUCCESS;
    MULTI_CH_DEFER_OID_STRUCT   MultiChDeferOid;
    
    DBGPRINT(RT_DEBUG_TRACE, ("%s ===>\n", __FUNCTION__));

    // Set next state timer
    PlatformSetTimer(pPort, &pPort->Mlme.MultiChannelTimer, pPort->P2PCfg.SwitchChTime);

    // No this part in USB
#if 0
    if (!MT_TEST_FLAG(pAd, fRTMP_ADAPTER_START_UP))
    {
        DBGPRINT(RT_DEBUG_TRACE, ("%s - MultiChSts: SYNC - MlmeSyncScanReqAction before Startup\n", __FUNCTION__));
        return;
    }
#endif

    //
    // Handle the end task before switch to new channel
    //
    if (/*(INFRA_ON(pAd->PortList[PORT_0]) || MT_TEST_BIT(pAd->pP2pCtrll->P2pMsConnectedStatus, P2pMs_CONNECTED_STATUS_CLI_OP)) ||
        ((pAd->MccCfg.MultiChannelCurrentMode == MULTI_CH_OP_MODE_INFRA) || (pAd->MccCfg.MultiChannelCurrentMode == MULTI_CH_OP_MODE_MSCLI))*/1)
    {
        //RTMPSuspendMsduTransmission(pAd);
        MultiChannelSuspendMsduTransmission(pAd);

        // Display H/W Queue status
        RTUSBReadMACRegister(pAd, TXRXQ_PCNT, &TxRxQPcnt.word);
        RTUSBReadMACRegister(pAd, PBF_DBG, &PbfDbg.word);
        DBGPRINT(RT_DEBUG_TRACE, ("TXRXQ_PCNT_1_(R=%d, EDCA=%d, HCCA=%d), PBF_DBG(%d)\n", 
                                        TxRxQPcnt.field.Rx0QPcnt/*(Value1 >> 24) & 0xff*/,
                                        TxRxQPcnt.field.Tx2QPcnt/*(Value1 >> 16) & 0xff*/,
                                        TxRxQPcnt.field.Tx1QPcnt/*(Value1 >> 8) & 0xff*/,
                                        PbfDbg.field.FreePcnt/*Value2 & 0xff*/));

        // Wait for sending out NULL frame (PWR_SAVE)
        if ((pAd->MccCfg.MultiChannelEnable == TRUE) && (pAd->MccCfg.MultiChannelCurrentMode == MULTI_CH_OP_MODE_INFRA))
        {
            pPort = pAd->PortList[PORT_0];
            DBGPRINT(RT_DEBUG_TRACE, ("%s - MultiChSts: SYNC - Port(%d) send out NULL Frame(PWR_SAVE) at ch(%d)\n", __FUNCTION__, pPort->PortNumber, pAd->HwCfg.LatchRfRegs.Channel));
            XmitSendNullFrameForChSwitch(pAd, 
                                pPort,
                                pPort->CommonCfg.TxRate,
                                FALSE,
                                PWR_SAVE);
            //MultiChannelWaitTxRingEmpty(pAd, /*QID_AC_BE*/MultiChannelGetSwQ(pPort->P2PCfg.STAHwQSEL));
            MultiChannelWaitTxRingEmpty(pAd);
            MultiChannelWaitHwQEmpty(pAd, /*FIFO_EDCA*/pPort->P2PCfg.STAHwQSEL, 0);
        }
        else if ((pAd->MccCfg.MultiChannelEnable == TRUE) && (pAd->MccCfg.MultiChannelCurrentMode == MULTI_CH_OP_MODE_MSCLI))
        {
            pPort = pAd->PortList[pPort->P2PCfg.ClientPortNumber];
            DBGPRINT(RT_DEBUG_TRACE, ("%s - MultiChSts: SYNC - Port(%d) send out NULL Frame(PWR_SAVE) at ch(%d)\n", __FUNCTION__, pPort->PortNumber, pAd->HwCfg.LatchRfRegs.Channel));
            XmitSendNullFrameForChSwitch(pAd,
                                pPort,
                                pPort->CommonCfg.TxRate, 
                                FALSE,
                                PWR_SAVE);
            //MultiChannelWaitTxRingEmpty(pAd, /*QID_HCCA*/MultiChannelGetSwQ(pPort->P2PCfg.CLIHwQSEL));  
            MultiChannelWaitTxRingEmpty(pAd);   
            MultiChannelWaitHwQEmpty(pAd, /*FIFO_HCCA*/pPort->P2PCfg.CLIHwQSEL, 0);
        }
        else if ((pAd->MccCfg.MultiChannelEnable == TRUE) && (pAd->MccCfg.MultiChannelCurrentMode == MULTI_CH_OP_MODE_MSGO))
        {
            // No need to do for GO case
        }

        // Stop all PBF out Queue
        RTUSBReadMACRegister(pAd, PBF_CFG(pAd), &PbfCfg.word);
        WRITE_PBF_CFG_TX2QENABLE(pAd, &PbfCfg, 0);
        WRITE_PBF_CFG_TX1QENABLE(pAd, &PbfCfg, 0);
        //PbfCfg.field.Tx2QEnable = 0;      // EDCA
        //PbfCfg.field.Tx1QEnable = 0;      // HCCA
        RTUSBWriteMACRegister(pAd, PBF_CFG(pAd), PbfCfg.word);

        // Delay for AP data
        DBGPRINT(RT_DEBUG_TRACE, ("Count(%d), Delay(%d) u sec\n", Count, pPort->P2PCfg.PwrSaveDelayTime * 1000));
        NdisCommonGenericDelay(pPort->P2PCfg.PwrSaveDelayTime * 1000);

        // Display H/W Queue status
        RTUSBReadMACRegister(pAd, TXRXQ_PCNT, &TxRxQPcnt.word);
        RTUSBReadMACRegister(pAd, PBF_DBG, &PbfDbg.word);
        DBGPRINT(RT_DEBUG_TRACE, ("TXRXQ_PCNT_2_(R=%d, EDCA=%d, HCCA=%d), PBF_DBG(%d)\n", 
                                        TxRxQPcnt.field.Rx0QPcnt/*(Value1 >> 24) & 0xff*/,
                                        TxRxQPcnt.field.Tx2QPcnt/*(Value1 >> 16) & 0xff*/,
                                        TxRxQPcnt.field.Tx1QPcnt/*(Value1 >> 8) & 0xff*/,
                                        PbfDbg.field.FreePcnt/*Value2 & 0xff*/));
    }

    // 
    // Select Next mode, H/W q, channel
    //
#if 0   
    if (pPort->BBPCurrentBW == BW_40)
    {
        // Toggle h/w queue and channel
        if (pAd->MccCfg.MultiChannelCurrentMode == MULTI_CH_OP_MODE_INFRA)
        {
            HwQSel = FIFO_HCCA;
            Channel = pPort->P2PCfg.P2PCentralChannel;
        }
        else
        {
            HwQSel = FIFO_EDCA;
            Channel = pPort->P2PCfg.InfraCentralChannel;
        }
    }
    else
#endif      
    {
        DBGPRINT(RT_DEBUG_TRACE, ("%s - MultiChSts: Current Mode(%d), STA ch(%d), CLI ch(%d), GO ch(%d), Connected(%d : %d : %d), GOHwQSEL(%d) and switch to\n", 
                                        __FUNCTION__, 
                                        pAd->MccCfg.MultiChannelCurrentMode,
                                        pPort->P2PCfg.InfraChannel,
                                        pPort->P2PCfg.P2PChannel,
                                        pPort->P2PCfg.GroupOpChannel,
                                        INFRA_ON(pAd->PortList[PORT_0]),
                                        MT_TEST_BIT(pAd->pP2pCtrll->P2pMsConnectedStatus, P2pMs_CONNECTED_STATUS_CLI_OP),
                                        MT_TEST_BIT(pAd->pP2pCtrll->P2pMsConnectedStatus, P2pMs_CONNECTED_STATUS_GO_ON),
                                        pPort->P2PCfg.GOHwQSEL));

        // Toggle h/w queue and channel
        // Current state = MULTI_CH_OP_MODE_INFRA
        if (pAd->MccCfg.MultiChannelCurrentMode == MULTI_CH_OP_MODE_INFRA)
        {
            if (MT_TEST_BIT(pAd->pP2pCtrll->P2pMsConnectedStatus, P2pMs_CONNECTED_STATUS_CLI_OP))
            {
                //HwQSel = /*FIFO_HCCA*/pPort->P2PCfg.CLIHwQSEL;
                //Channel = pPort->P2PCfg.P2PChannel;
                pAd->MccCfg.MultiChannelCurrentMode = MULTI_CH_OP_MODE_MSCLI;
                MultiChannelGetHwQAndCh(pAd, &HwQSel, &Channel, &ExtChanOffset);
            }
            else if (MT_TEST_BIT(pAd->pP2pCtrll->P2pMsConnectedStatus, P2pMs_CONNECTED_STATUS_GO_ON))
            {
                //HwQSel = /*FIFO_HCCA*/pPort->P2PCfg.GOHwQSEL;
                //Channel = pPort->P2PCfg.GroupOpChannel;                 
                pAd->MccCfg.MultiChannelCurrentMode = MULTI_CH_OP_MODE_MSGO;
                MultiChannelGetHwQAndCh(pAd, &HwQSel, &Channel, &ExtChanOffset);
            }
            else
            {
                SwitchStatus = 1;
            }
        }
        // Current state = MULTI_CH_OP_MODE_MSCLI
        else if (pAd->MccCfg.MultiChannelCurrentMode == MULTI_CH_OP_MODE_MSCLI)
        {
            if (MT_TEST_BIT(pAd->pP2pCtrll->P2pMsConnectedStatus, P2pMs_CONNECTED_STATUS_GO_ON))
            {
                //HwQSel = pPort->P2PCfg.GOHwQSEL;
                //Channel = pPort->P2PCfg.GroupOpChannel;                 
                pAd->MccCfg.MultiChannelCurrentMode = MULTI_CH_OP_MODE_MSGO;
                MultiChannelGetHwQAndCh(pAd, &HwQSel, &Channel, &ExtChanOffset);
            }
            else if (INFRA_ON(pAd->PortList[PORT_0]))
            {
                //HwQSel = /*FIFO_EDCA*/pPort->P2PCfg.STAHwQSEL;
                //Channel = pPort->P2PCfg.InfraChannel;
                pAd->MccCfg.MultiChannelCurrentMode = MULTI_CH_OP_MODE_INFRA;
                MultiChannelGetHwQAndCh(pAd, &HwQSel, &Channel, &ExtChanOffset);
            }
            else
            {
                SwitchStatus = 2;
            }
        }
        // Current state = MULTI_CH_OP_MODE_MSGO
        else if (pAd->MccCfg.MultiChannelCurrentMode == MULTI_CH_OP_MODE_MSGO)
        {
            if (INFRA_ON(pAd->PortList[PORT_0]))
            {
                //HwQSel = /*FIFO_EDCA*/pPort->P2PCfg.STAHwQSEL;
                //Channel = pPort->P2PCfg.InfraChannel;
                pAd->MccCfg.MultiChannelCurrentMode = MULTI_CH_OP_MODE_INFRA;
                MultiChannelGetHwQAndCh(pAd, &HwQSel, &Channel, &ExtChanOffset);
            }
            else if (MT_TEST_BIT(pAd->pP2pCtrll->P2pMsConnectedStatus, P2pMs_CONNECTED_STATUS_CLI_OP))
            {
                //HwQSel = /*FIFO_HCCA*/pPort->P2PCfg.CLIHwQSEL;
                //Channel = pPort->P2PCfg.P2PChannel;                 
                pAd->MccCfg.MultiChannelCurrentMode = MULTI_CH_OP_MODE_MSCLI;;
                MultiChannelGetHwQAndCh(pAd, &HwQSel, &Channel, &ExtChanOffset);
            }
            else
            {
                SwitchStatus = 3;                   
            }
        }
        else
        {
            SwitchStatus = 4;
        }

        if (SwitchStatus == 0)
        {
            DBGPRINT(RT_DEBUG_TRACE, ("%s - MultiChSts: Next Mode(%d), ch(%d), h/w queue(%d)\n", __FUNCTION__, pAd->MccCfg.MultiChannelCurrentMode, Channel, HwQSel));
        }
        else
        {
            DBGPRINT(RT_DEBUG_TRACE, ("%s - MultiChSts: Next mode error status(%d)\n", __FUNCTION__, SwitchStatus));
        }
    }   


    // Pause beacon if GO's chaneel is different to next multi channel mode.
    if ((pAd->MccCfg.MultiChannelEnable == TRUE)
        && (pPort->P2PCfg.bGOStart == TRUE)
        && (PreviousMultiChannelMode == MULTI_CH_OP_MODE_MSGO)
        && (pPort->P2PCfg.GOHwQSEL != 0)
        && (TRUE == P2pCheckBssSync(pAd)))
    {   
        if ((pAd->MccCfg.MultiChannelCurrentMode == MULTI_CH_OP_MODE_MSCLI) && (pPort->P2PCfg.GOHwQSEL != pPort->P2PCfg.CLIHwQSEL))
            AsicPauseBssSync(pAd);
        else if ((pAd->MccCfg.MultiChannelCurrentMode == MULTI_CH_OP_MODE_INFRA) && (pPort->P2PCfg.GOHwQSEL != pPort->P2PCfg.STAHwQSEL))
            AsicPauseBssSync(pAd);
    }

    // 
    // Handle scan task during multichannel here!
    //
    if (MT_TEST_BIT(pAd->pP2pCtrll->MultiChannelStartOrEndProcess, MULTI_CHANNEL_EXT_SCAN_PRE_START))
    {
        // Stop timer
        MultiChannelSwitchStop(pAd);

        // Enable EDCA h/w queue to scan
        RTUSBReadMACRegister(pAd, PBF_CFG(pAd), &PbfCfg.word);
        WRITE_PBF_CFG_TX2QENABLE(pAd, &PbfCfg, 1);
        WRITE_PBF_CFG_TX1QENABLE(pAd, &PbfCfg, 0);
        //PbfCfg.field.Tx2QEnable = 1;      // EDCA
        //PbfCfg.field.Tx1QEnable = 0;      // HCCA
        RTUSBWriteMACRegister(pAd, PBF_CFG(pAd), PbfCfg.word);      

        // Process defer task
        ndisStatus = MultiChannelDeferScanRequest(pAd, pAd->MccCfg.pScanRequestPort);
        
        // Clear flag
        MT_CLEAR_BIT(pAd->pP2pCtrll->MultiChannelStartOrEndProcess, MULTI_CHANNEL_EXT_SCAN_PRE_START);

        MultiChDeferOid.Type = PENDED_SCAN_OID_REQUEST;
        MultiChDeferOid.ndisStatus = ndisStatus;
        MTEnqueueInternalCmd(pAd, pPort, MT_CMD_MULTI_CHANNEL_SCAN_START_EXEC, &MultiChDeferOid, sizeof(MULTI_CH_DEFER_OID_STRUCT));        

        return;
    }
    
    MultiChannelSwicthHwQAndCh(pAd, HwQSel, Channel, ExtChanOffset);

    //
    // Handle start GO case
    //
    if (MT_TEST_BIT(pAd->pP2pCtrll->MultiChannelStartOrEndProcess, MULTI_CHANNEL_GO_PRE_START))
    {
        // Stop timer
        MultiChannelSwitchStop(pAd);

        // Process defer task
        MultiChannelDeferStartGoRequest(pAd, pAd->MccCfg.pGoPort);

        // Clear flag
        MT_CLEAR_BIT(pAd->pP2pCtrll->MultiChannelStartOrEndProcess, MULTI_CHANNEL_GO_PRE_START);
    }

    //
    // Handle disconnection
    //
    // Disconnect from OS
    if (MT_TEST_BIT(pAd->pP2pCtrll->MultiChannelStartOrEndProcess, MULTI_CHANNEL_STA_PRE_STOP) &&
        (pAd->MccCfg.MultiChannelCurrentMode == MULTI_CH_OP_MODE_INFRA))
    {
        // Stop timer
        MultiChannelSwitchStop(pAd);

        // Handle disconnect oid
        ndisStatus = MultiChannelDeferDisconnectRequest(pAd, pAd->MccCfg.pDisconnectPort);

        // Clear flag
        MT_CLEAR_BIT(pAd->pP2pCtrll->MultiChannelStartOrEndProcess, MULTI_CHANNEL_STA_PRE_STOP);

        // Trigger next new state
        MT_SET_BIT(pAd->pP2pCtrll->MultiChannelStartOrEndProcess, MULTI_CHANNEL_STA_STOP);
        MultiChDeferOid.Type = PENDED_DISCONNECT_OID_REQUEST;
        MultiChDeferOid.ndisStatus = ndisStatus;
        MTEnqueueInternalCmd(pAd, pPort, MT_CMD_MULTI_CHANNEL_SWITCH_EXEC, &MultiChDeferOid, sizeof(MULTI_CH_DEFER_OID_STRUCT));        

        //return;
    }
    else if (MT_TEST_BIT(pAd->pP2pCtrll->MultiChannelStartOrEndProcess, MULTI_CHANNEL_CLI_PRE_STOP) &&
        (pAd->MccCfg.MultiChannelCurrentMode == MULTI_CH_OP_MODE_MSCLI))
    {
        // Stop timer
        MultiChannelSwitchStop(pAd);

        // Handle disconnect oid
        ndisStatus = MultiChannelDeferDisconnectRequest(pAd, pAd->MccCfg.pDisconnectPort);

        // Clear flag
        MT_CLEAR_BIT(pAd->pP2pCtrll->MultiChannelStartOrEndProcess, MULTI_CHANNEL_CLI_PRE_STOP);

        // Trigger next new state
        MT_SET_BIT(pAd->pP2pCtrll->MultiChannelStartOrEndProcess, MULTI_CHANNEL_CLI_STOP);
        MultiChDeferOid.Type = PENDED_DISCONNECT_OID_REQUEST;
        MultiChDeferOid.ndisStatus = ndisStatus;
        MTEnqueueInternalCmd(pAd, pPort, MT_CMD_MULTI_CHANNEL_SWITCH_EXEC, &MultiChDeferOid, sizeof(MULTI_CH_DEFER_OID_STRUCT));        

        //return;
    }
    // Reset Request from OS
    else if (MT_TEST_BIT(pAd->pP2pCtrll->MultiChannelStartOrEndProcess, MULTI_CHANNEL_GO_PRE_STOP) &&
        (pAd->MccCfg.MultiChannelCurrentMode == MULTI_CH_OP_MODE_MSGO))
    {
        // Stop timer
        MultiChannelSwitchStop(pAd);

        // Process defer task
        ndisStatus = MultiChannelDeferStopGoRequest(pAd, pAd->MccCfg.pGoPort);

        // Clear flag
        MT_CLEAR_BIT(pAd->pP2pCtrll->MultiChannelStartOrEndProcess, MULTI_CHANNEL_GO_PRE_STOP);
        
        // Trigger next new state
        MT_SET_BIT(pAd->pP2pCtrll->MultiChannelStartOrEndProcess, MULTI_CHANNEL_GO_STOP);
        MultiChDeferOid.Type = PENDED_STOP_GO_OID_REQUEST;
        MultiChDeferOid.ndisStatus = ndisStatus;
        MTEnqueueInternalCmd(pAd, pPort, MT_CMD_MULTI_CHANNEL_SWITCH_EXEC, &MultiChDeferOid, sizeof(MULTI_CH_DEFER_OID_STRUCT));        

    }
    else if (MT_TEST_BIT(pAd->pP2pCtrll->MultiChannelStartOrEndProcess, MULTI_CHANNEL_CLI_PRE_RESET_REQ) &&
        (pAd->MccCfg.MultiChannelCurrentMode == MULTI_CH_OP_MODE_MSCLI))
    {
        // Stop timer
        MultiChannelSwitchStop(pAd);
        
        ndisStatus = MultiChannelDeferResetRequest(pAd, pAd->MccCfg.pResetRequestPort);

        // Clear flag
        MT_CLEAR_BIT(pAd->pP2pCtrll->MultiChannelStartOrEndProcess, MULTI_CHANNEL_CLI_PRE_RESET_REQ);

        // Trigger next new state
        MT_SET_BIT(pAd->pP2pCtrll->MultiChannelStartOrEndProcess, MULTI_CHANNEL_CLI_STOP);
        MultiChDeferOid.Type = PENDED_RESET_OID_REQUEST;
        MultiChDeferOid.ndisStatus = ndisStatus;
        MTEnqueueInternalCmd(pAd, pPort, MT_CMD_MULTI_CHANNEL_SWITCH_EXEC, &MultiChDeferOid, sizeof(MULTI_CH_DEFER_OID_STRUCT));        
    }
    else if (MT_TEST_BIT(pAd->pP2pCtrll->MultiChannelStartOrEndProcess, MULTI_CHANNEL_STA_PRE_RESET_REQ) &&
        (pAd->MccCfg.MultiChannelCurrentMode == MULTI_CH_OP_MODE_INFRA))
    {
        // Stop timer
        MultiChannelSwitchStop(pAd);
        
        ndisStatus = MultiChannelDeferResetRequest(pAd, pAd->MccCfg.pResetRequestPort);

        // Clear flag
        MT_CLEAR_BIT(pAd->pP2pCtrll->MultiChannelStartOrEndProcess, MULTI_CHANNEL_STA_PRE_RESET_REQ);

        // Trigger next new state
        MT_SET_BIT(pAd->pP2pCtrll->MultiChannelStartOrEndProcess, MULTI_CHANNEL_STA_STOP);
        MultiChDeferOid.Type = PENDED_RESET_OID_REQUEST;
        MultiChDeferOid.ndisStatus = ndisStatus;
        MTEnqueueInternalCmd(pAd, pPort, MT_CMD_MULTI_CHANNEL_SWITCH_EXEC, &MultiChDeferOid, sizeof(MULTI_CH_DEFER_OID_STRUCT));        
    }

    // Resume beacon her if GO stays in multi channnel GO mode or in the same channel(queue).
    if (FALSE == P2pCheckBssSync(pAd))
    {
        if (pAd->MccCfg.MultiChannelCurrentMode == MULTI_CH_OP_MODE_MSGO)
        {
            AsicResumeBssSync(pAd);
        }
        else if ((pAd->MccCfg.MultiChannelEnable == TRUE)
            && (pPort->P2PCfg.bGOStart == TRUE)
            && (PreviousMultiChannelMode != MULTI_CH_OP_MODE_MSGO)
            && (pPort->P2PCfg.GOHwQSEL != 0))
        {
            if ((pAd->MccCfg.MultiChannelCurrentMode == MULTI_CH_OP_MODE_MSCLI) && (pPort->P2PCfg.GOHwQSEL == pPort->P2PCfg.CLIHwQSEL))
                AsicResumeBssSync(pAd);
            else if ((pAd->MccCfg.MultiChannelCurrentMode == MULTI_CH_OP_MODE_INFRA) && (pPort->P2PCfg.GOHwQSEL == pPort->P2PCfg.STAHwQSEL))
                AsicResumeBssSync(pAd);
        }
    }

    DBGPRINT(RT_DEBUG_TRACE, ("%s <===\n", __FUNCTION__));
}

/* 
    ==========================================================================
    Description:
        This fucntion is for channel and hw queue switch, it supports both 20M and 40M
    Return:

    NOTE:
         
    ==========================================================================
*/
VOID MultiChannelSwicthHwQAndCh(
    IN PMP_ADAPTER pAd,
    IN ULONG HwQSel,
    IN UCHAR Channel,
    IN UCHAR ExtChanOffset)
{
    static  ULONG       PreviousHwQSel = 0;
    static  UCHAR       PreviousChannel = 0;
    PMP_PORT  pPort = NULL;
    UINT32      Count = 0;
    UCHAR       BBPValue = 0;
    UCHAR       ChannelWidth = BW_20;
    PBF_CFG_STRUC_EXT   PbfCfg = {0};
    TXRXQ_PCNT_STRUC    TxRxQPcnt = {0};
    PBF_DBG_STRUC       PbfDbg = {0};

    DBGPRINT(RT_DEBUG_TRACE, ("%s ====>\n", __FUNCTION__));

    MultiChannelSuspendMsduTransmission(pAd);
    
    //
    // Switch channel
    //
    if ((ExtChanOffset == EXTCHA_ABOVE) || (ExtChanOffset == EXTCHA_BELOW))
    {
        ChannelWidth = BW_40;
    }
    
    MlmeCntlChannelWidth(pAd, Channel, Channel, ChannelWidth, ExtChanOffset);

    // Display H/W Queue status
    RTUSBReadMACRegister(pAd, TXRXQ_PCNT, &TxRxQPcnt.word);
    RTUSBReadMACRegister(pAd, PBF_DBG, &PbfDbg.word);
    DBGPRINT(RT_DEBUG_TRACE, ("TXRXQ_PCNT_3_(R=%d, EDCA=%d, HCCA=%d), PBF_DBG(%d)\n", 
                                    TxRxQPcnt.field.Rx0QPcnt/*(Value1 >> 24) & 0xff*/,
                                    TxRxQPcnt.field.Tx2QPcnt/*(Value1 >> 16) & 0xff*/,
                                    TxRxQPcnt.field.Tx1QPcnt/*(Value1 >> 8) & 0xff*/,
                                    PbfDbg.field.FreePcnt/*Value2 & 0xff*/));

    //
    // Select H/W Q
    //
    if (HwQSel != PreviousHwQSel)
    {
        RTUSBReadMACRegister(pAd, PBF_CFG(pAd), &PbfCfg.word);
        WRITE_PBF_CFG_TX2QENABLE(pAd, &PbfCfg, 0);
        WRITE_PBF_CFG_TX1QENABLE(pAd, &PbfCfg, 0);
        //PbfCfg.field.Tx2QEnable = 0;      // EDCA
        //PbfCfg.field.Tx1QEnable = 0;      // HCCA

        if (HwQSel == FIFO_HCCA)    
        {
            WRITE_PBF_CFG_TX1QENABLE(pAd, &PbfCfg, 1);
            //PbfCfg.field.Tx1QEnable = 1;  
            DBGPRINT(RT_DEBUG_TRACE, ("%s - MultiChSts : HCCA Tx1Q enable, Value = %x\n", __FUNCTION__, PbfCfg.word));
        }
        else if (HwQSel == FIFO_EDCA)
        {
            WRITE_PBF_CFG_TX2QENABLE(pAd, &PbfCfg, 1);
            //PbfCfg.field.Tx2QEnable = 1;
            DBGPRINT(RT_DEBUG_TRACE, ("%s - MultiChSts : EDCA Tx2Q enable, Value = %x\n", __FUNCTION__, PbfCfg.word));
        }
        else
        {
            WRITE_PBF_CFG_TX2QENABLE(pAd, &PbfCfg, 1);
            //PbfCfg.field.Tx2QEnable = 1;
            DBGPRINT(RT_DEBUG_TRACE, ("%s - MultiChSts : Deafult EDCA Tx2Q enable, Value = %x\n", __FUNCTION__, PbfCfg.word));
        }

        RTUSBWriteMACRegister(pAd, PBF_CFG(pAd), PbfCfg.word);
    }

    // Send out PWR_ACTIVE frame
    if ((pAd->MccCfg.MultiChannelEnable == TRUE) && (pAd->MccCfg.MultiChannelCurrentMode == MULTI_CH_OP_MODE_INFRA))
    {
        pPort = pAd->PortList[PORT_0];
        DBGPRINT(RT_DEBUG_TRACE, ("%s - MultiChSts: SYNC - Port(%d) send out NULL Frame(PWR_ACTIVE) at ch(%d)\n", __FUNCTION__, pPort->PortNumber, pAd->HwCfg.LatchRfRegs.Channel));
        //AsicSendNullFrameForMultiCh(pAd, pPort, pAd->HwCfg.LatchRfRegs.Channel, PWR_ACTIVE);
        XmitSendNullFrameForChSwitch(pAd, 
                            pPort,
                            pPort->CommonCfg.TxRate,
                            FALSE,//PORT_OPSTATUS_TEST_FLAG(pPort, fOP_PORT_STATUS_WMM_INUSED) ? TRUE : FALSE,
                            PWR_ACTIVE);
        //MultiChannelWaitTxRingEmpty(pAd, /*QID_AC_BE*/MultiChannelGetSwQ(pPort->P2PCfg.STAHwQSEL));
        MultiChannelWaitTxRingEmpty(pAd);
        MultiChannelWaitHwQEmpty(pAd, /*FIFO_EDCA*/pPort->P2PCfg.STAHwQSEL, 0);
        pAd->MccCfg.SwQSelect = /*QID_AC_BE*/MultiChannelGetSwQ(pPort->P2PCfg.STAHwQSEL);
        DBGPRINT(RT_DEBUG_TRACE, ("%s - MultiChSts: SwQSelect(%d), STAHwQSEL(%d)\n", __FUNCTION__, pAd->MccCfg.SwQSelect, pPort->P2PCfg.STAHwQSEL));
    }
    else if ((pAd->MccCfg.MultiChannelEnable == TRUE) && (pAd->MccCfg.MultiChannelCurrentMode == MULTI_CH_OP_MODE_MSCLI))
    {
        pPort = pAd->PortList[pPort->P2PCfg.ClientPortNumber];
        DBGPRINT(RT_DEBUG_TRACE, ("%s - MultiChSts: SYNC - Port(%d) send out NULL Frame(PWR_ACTIVE) at ch(%d)\n", __FUNCTION__, pPort->PortNumber, pAd->HwCfg.LatchRfRegs.Channel));
        //AsicSendNullFrameForMultiCh(pAd, pPort, pAd->HwCfg.LatchRfRegs.Channel, PWR_ACTIVE);
        XmitSendNullFrameForChSwitch(pAd, 
                            pPort,
                            pPort->CommonCfg.TxRate,
                            FALSE,//PORT_OPSTATUS_TEST_FLAG(pPort, fOP_PORT_STATUS_WMM_INUSED) ? TRUE : FALSE,
                            PWR_ACTIVE);
        //MultiChannelWaitTxRingEmpty(pAd, /*QID_HCCA*/MultiChannelGetSwQ(pPort->P2PCfg.CLIHwQSEL));
        MultiChannelWaitTxRingEmpty(pAd);
        MultiChannelWaitHwQEmpty(pAd, /*FIFO_HCCA*/pPort->P2PCfg.CLIHwQSEL, 0);   
        pAd->MccCfg.SwQSelect = /*QID_HCCA*/MultiChannelGetSwQ(pPort->P2PCfg.CLIHwQSEL);
        DBGPRINT(RT_DEBUG_TRACE, ("%s - MultiChSts: SwQSelect(%d), CLIHwQSEL(%d)\n", __FUNCTION__, pAd->MccCfg.SwQSelect, pPort->P2PCfg.CLIHwQSEL));
    }
    else if ((pAd->MccCfg.MultiChannelEnable == TRUE) && (pAd->MccCfg.MultiChannelCurrentMode == MULTI_CH_OP_MODE_MSGO))
    {
        pAd->MccCfg.SwQSelect = MultiChannelGetSwQ(pPort->P2PCfg.GOHwQSEL);
    }

    //RTMPResumeMsduTransmission(pAd, pPort);
    MultiChannelResumeMsduTransmission(pAd);

    PreviousHwQSel = HwQSel;
    PreviousChannel = Channel;

    DBGPRINT(RT_DEBUG_TRACE, ("%s <===\n", __FUNCTION__));  
}

/* 
    ==========================================================================
    Description:
        This function will handle the multi-channel state after stopping GO
    Return:
        None
    NOTE:
    ==========================================================================
*/
VOID MultiChannelGOStop(
    IN PMP_ADAPTER pAd)
{
    UCHAR NewStatus = 0;
    PMP_PORT pPort = pAd->PortList[FXXK_PORT_0];
    //
    // Consider MultiChannelEnable, MultiChannelCurrentMode, channel, h/w queue and below case      
    //
    OPSTATUS_CLEAR_FLAG(pAd, fOP_STATUS_MEDIA_STATE_MULTI_CHANNEL_SWITCHING);
    OPSTATUS_CLEAR_FLAG(pAd, fOP_STATUS_MEDIA_STATE_MULTI_CHANNEL_WITH_NULLFRAME);
    
    // 1. STA+CLI different channel
    if (INFRA_ON(pAd->PortList[PORT_0]) && 
        MT_TEST_BIT(pAd->pP2pCtrll->P2pMsConnectedStatus, P2pMs_CONNECTED_STATUS_CLI_OP) &&
        (pPort->P2PCfg.InfraChannel != pPort->P2PCfg.P2PChannel))
    {
        pAd->MccCfg.MultiChannelEnable = TRUE;
        pAd->MccCfg.MultiChannelCurrentMode = MULTI_CH_OP_MODE_INFRA;
        pAd->MccCfg.SwQSelect = MultiChannelGetSwQ(pPort->P2PCfg.STAHwQSEL);
        MultiChannelSwicthHwQAndCh(pAd, pPort->P2PCfg.STAHwQSEL, pPort->P2PCfg.InfraChannel, EXTCHA_NONE);
        PlatformSetTimer(pPort, &pPort->Mlme.MultiChannelTimer, pPort->P2PCfg.SwitchChTime);
        pPort->Channel = pPort->P2PCfg.InfraChannel;
        NewStatus = 1;
    }
    // 2. STA+CLI the same channel
    else if (INFRA_ON(pAd->PortList[PORT_0]) && 
        MT_TEST_BIT(pAd->pP2pCtrll->P2pMsConnectedStatus, P2pMs_CONNECTED_STATUS_CLI_OP) &&
        (pPort->P2PCfg.InfraChannel == pPort->P2PCfg.P2PChannel))
    {
        pAd->MccCfg.MultiChannelEnable = FALSE;
        pAd->MccCfg.MultiChannelCurrentMode = MULTI_CH_OP_MODE_INFRA;
        MultiChannelSwicthHwQAndCh(pAd, /*FIFO_EDCA*/pPort->P2PCfg.STAHwQSEL, pPort->P2PCfg.InfraChannel, EXTCHA_NONE);
        pPort->Channel = pPort->P2PCfg.InfraChannel;
        NewStatus = 2;
    }
    // 3. STA at channel X
    else if (INFRA_ON(pAd->PortList[PORT_0]) && 
        (MT_TEST_BIT(pAd->pP2pCtrll->P2pMsConnectedStatus, P2pMs_CONNECTED_STATUS_CLI_OP) == FALSE))
    {
        pAd->MccCfg.MultiChannelEnable = FALSE;
        pAd->MccCfg.MultiChannelCurrentMode = MULTI_CH_OP_MODE_INFRA;
        pPort->P2PCfg.STAHwQSEL = FIFO_EDCA;
        MultiChannelSwicthHwQAndCh(pAd, /*FIFO_EDCA*/pPort->P2PCfg.STAHwQSEL, pPort->P2PCfg.InfraChannel, EXTCHA_NONE);
        pPort->Channel = pPort->P2PCfg.InfraChannel;
        NewStatus = 3;
    }
    // 4. CLI at channel Y
    else if ((INFRA_ON(pAd->PortList[PORT_0]) == FALSE) && 
        MT_TEST_BIT(pAd->pP2pCtrll->P2pMsConnectedStatus, P2pMs_CONNECTED_STATUS_CLI_OP))
    {
        pAd->MccCfg.MultiChannelEnable = FALSE;
        pAd->MccCfg.MultiChannelCurrentMode = MULTI_CH_OP_MODE_MSCLI;
        pPort->P2PCfg.CLIHwQSEL = FIFO_EDCA;
        MultiChannelSwicthHwQAndCh(pAd, /*FIFO_EDCA*/pPort->P2PCfg.CLIHwQSEL, pPort->P2PCfg.P2PChannel, EXTCHA_NONE);
        pPort->Channel = pPort->P2PCfg.P2PChannel;
        NewStatus = 4;
    }
    else
    {
        NewStatus = 5;
    }

    DBGPRINT(RT_DEBUG_TRACE, ("%s - Adjust mulit-channel, InfraChannel(%d), P2PChannel(%d), MultiChannelEnable(%d), MultiChannelCurrentMode(%d), NewStatus(%d)\n",
                                __FUNCTION__,
                                pPort->P2PCfg.InfraChannel,
                                pPort->P2PCfg.P2PChannel,
                                pAd->MccCfg.MultiChannelEnable,
                                pAd->MccCfg.MultiChannelCurrentMode,
                                NewStatus));
}

/* 
    ==========================================================================
    Description:
        This function will handle the multi-channel state after stopping CLI
    Return:
        None
    NOTE:
    ==========================================================================
*/
VOID MultiChannelCLIStop(
    IN PMP_ADAPTER pAd)
{
    UCHAR NewStatus = 0;
    UCHAR       Channel = 0;
    ULONG       HwQSel = FIFO_EDCA;
    UCHAR       ExtChanOffset = EXTCHA_NONE;
    PMP_PORT pPort = pAd->PortList[FXXK_PORT_0];
    Channel = pPort->P2PCfg.InfraChannel;
    //
    // Consider MultiChannelEnable, MultiChannelCurrentMode, channel, h/w queue and below case      
    //
    OPSTATUS_CLEAR_FLAG(pAd, fOP_STATUS_MEDIA_STATE_MULTI_CHANNEL_SWITCHING);
    OPSTATUS_CLEAR_FLAG(pAd, fOP_STATUS_MEDIA_STATE_MULTI_CHANNEL_WITH_NULLFRAME);

    // 1. STA+GO different channel
    if (INFRA_ON(pAd->PortList[PORT_0]) && 
        MT_TEST_BIT(pAd->pP2pCtrll->P2pMsConnectedStatus, P2pMs_CONNECTED_STATUS_GO_ON) &&
        (pPort->P2PCfg.InfraChannel != pPort->P2PCfg.GroupOpChannel))
    {
        pAd->MccCfg.MultiChannelEnable = TRUE;
        pAd->MccCfg.MultiChannelCurrentMode = MULTI_CH_OP_MODE_INFRA;
        pAd->MccCfg.SwQSelect = MultiChannelGetSwQ(pPort->P2PCfg.STAHwQSEL);
        MultiChannelSwicthHwQAndCh(pAd, pPort->P2PCfg.STAHwQSEL, pPort->P2PCfg.InfraChannel, EXTCHA_NONE);
        pPort->Channel = pPort->P2PCfg.InfraChannel;
        PlatformSetTimer(pPort, &pPort->Mlme.MultiChannelTimer, pPort->P2PCfg.SwitchChTime);
        //AsicResumeBssSync(pAd);
        NewStatus = 1;
    }
    // 2. STA+GO the same channel
    else if (INFRA_ON(pAd->PortList[PORT_0]) && 
        MT_TEST_BIT(pAd->pP2pCtrll->P2pMsConnectedStatus, P2pMs_CONNECTED_STATUS_GO_ON) &&
        (pPort->P2PCfg.InfraChannel == pPort->P2PCfg.GroupOpChannel))
    {
        pAd->MccCfg.MultiChannelEnable = FALSE;
        pAd->MccCfg.MultiChannelCurrentMode = MULTI_CH_OP_MODE_INFRA;
        MultiChannelSwicthHwQAndCh(pAd, pPort->P2PCfg.STAHwQSEL, pPort->P2PCfg.InfraChannel, EXTCHA_NONE);
        pPort->Channel = pPort->P2PCfg.InfraChannel;
        AsicResumeBssSync(pAd);
        NewStatus = 2;
    }
    // 3. STA at channel X
    else if (INFRA_ON(pAd->PortList[PORT_0]) && 
        (MT_TEST_BIT(pAd->pP2pCtrll->P2pMsConnectedStatus, P2pMs_CONNECTED_STATUS_GO_ON) == FALSE))
    {
        pAd->MccCfg.MultiChannelEnable = FALSE;
        pAd->MccCfg.MultiChannelCurrentMode = MULTI_CH_OP_MODE_INFRA;
        pPort->P2PCfg.STAHwQSEL = FIFO_EDCA;

        // Get h/w q and channel
        MultiChannelGetHwQAndCh(pAd, &HwQSel, &Channel, &ExtChanOffset);
        
        MultiChannelSwicthHwQAndCh(pAd, /*pPort->P2PCfg.STAHwQSEL*/HwQSel, /*pPort->P2PCfg.InfraChannel*/Channel, /*EXTCHA_NONE*/ExtChanOffset);
        //pPort->Channel = Channel;

        if ((ExtChanOffset == EXTCHA_ABOVE) || (ExtChanOffset == EXTCHA_BELOW))
            pPort->CentralChannel = Channel;
        else
            pPort->Channel = Channel;

        pPort->P2PCfg.CurrentWcid = (UCHAR)MlmeSyncGetWlanIdxByPort(pPort, ROLE_WLANIDX_BSSID);
            
        NewStatus = 3;
    }
    // 4. GO at channel Y
    else if ((INFRA_ON(pAd->PortList[PORT_0]) == FALSE) && 
        MT_TEST_BIT(pAd->pP2pCtrll->P2pMsConnectedStatus, P2pMs_CONNECTED_STATUS_GO_ON))
    {
        pAd->MccCfg.MultiChannelEnable = FALSE;
        pAd->MccCfg.MultiChannelCurrentMode = MULTI_CH_OP_MODE_MSCLI;
        pPort->P2PCfg.GOHwQSEL = FIFO_EDCA;
        MultiChannelSwicthHwQAndCh(pAd, pPort->P2PCfg.GOHwQSEL, pPort->P2PCfg.GroupOpChannel, EXTCHA_NONE);
        pPort->Channel = pPort->P2PCfg.GroupOpChannel;
        AsicResumeBssSync(pAd);
        NewStatus = 4;
    }
    else
    {
        NewStatus = 5;
    }

    DBGPRINT(RT_DEBUG_TRACE, ("%s(%d) - Adjust mulit-channel, CH(%d, %d, %d), MultiChannelEnable(%d), MultiChannelCurrentMode(%d), NewStatus(%d)\n",
                                __FUNCTION__,
                                __LINE__,
                                pPort->P2PCfg.InfraChannel,
                                pPort->P2PCfg.P2PChannel,
                                pPort->P2PCfg.GroupOpChannel,
                                pAd->MccCfg.MultiChannelEnable,
                                pAd->MccCfg.MultiChannelCurrentMode,
                                NewStatus));    
}

/* 
    ==========================================================================
    Description:
        This function will handle the multi-channel state after stopping STA
    Return:
        None
    NOTE:
    ==========================================================================
*/
VOID MultiChannelSTAStop(
    IN PMP_ADAPTER pAd)
{
    UCHAR NewStatus = 0;
    UCHAR       Channel = 0;
    ULONG       HwQSel = FIFO_EDCA;
    UCHAR       ExtChanOffset = EXTCHA_NONE;
    PMP_PORT pPort = pAd->PortList[FXXK_PORT_0];

    Channel = pPort->P2PCfg.InfraChannel;
    //
    // Consider MultiChannelEnable, MultiChannelCurrentMode, channel, h/w queue and below case      
    //
    OPSTATUS_CLEAR_FLAG(pAd, fOP_STATUS_MEDIA_STATE_MULTI_CHANNEL_SWITCHING);
    OPSTATUS_CLEAR_FLAG(pAd, fOP_STATUS_MEDIA_STATE_MULTI_CHANNEL_WITH_NULLFRAME);

    // 1. CLI+GO different channel
    if (MT_TEST_BIT(pAd->pP2pCtrll->P2pMsConnectedStatus, P2pMs_CONNECTED_STATUS_CLI_OP) && 
        MT_TEST_BIT(pAd->pP2pCtrll->P2pMsConnectedStatus, P2pMs_CONNECTED_STATUS_GO_ON) &&
        (pPort->P2PCfg.P2PChannel != pPort->P2PCfg.GroupOpChannel))
    {
        pAd->MccCfg.MultiChannelEnable = TRUE;
        pAd->MccCfg.MultiChannelCurrentMode = MULTI_CH_OP_MODE_MSCLI;
        pAd->MccCfg.SwQSelect = MultiChannelGetSwQ(pPort->P2PCfg.CLIHwQSEL);
        MultiChannelSwicthHwQAndCh(pAd, pPort->P2PCfg.CLIHwQSEL, pPort->P2PCfg.P2PChannel, EXTCHA_NONE);
        pPort->Channel = pPort->P2PCfg.P2PChannel;
        PlatformSetTimer(pPort, &pPort->Mlme.MultiChannelTimer, pPort->P2PCfg.SwitchChTime);
        //AsicResumeBssSync(pAd);
        NewStatus = 1;
    }
    // 2. CLI+GO the same channel
    else if (MT_TEST_BIT(pAd->pP2pCtrll->P2pMsConnectedStatus, P2pMs_CONNECTED_STATUS_CLI_OP) && 
        MT_TEST_BIT(pAd->pP2pCtrll->P2pMsConnectedStatus, P2pMs_CONNECTED_STATUS_GO_ON) &&
        (pPort->P2PCfg.P2PChannel == pPort->P2PCfg.GroupOpChannel))
    {
        pAd->MccCfg.MultiChannelEnable = FALSE;
        pAd->MccCfg.MultiChannelCurrentMode = MULTI_CH_OP_MODE_MSCLI;
        MultiChannelSwicthHwQAndCh(pAd, /*FIFO_EDCA*/pPort->P2PCfg.CLIHwQSEL, pPort->P2PCfg.P2PChannel, EXTCHA_NONE);
        pPort->Channel = pPort->P2PCfg.P2PChannel;
        AsicResumeBssSync(pAd);
        NewStatus = 2;
    }
    // 3. CLI at channel X
    else if (MT_TEST_BIT(pAd->pP2pCtrll->P2pMsConnectedStatus, P2pMs_CONNECTED_STATUS_CLI_OP) && 
        (MT_TEST_BIT(pAd->pP2pCtrll->P2pMsConnectedStatus, P2pMs_CONNECTED_STATUS_GO_ON) == FALSE))
    {
        pAd->MccCfg.MultiChannelEnable = FALSE;
        pAd->MccCfg.MultiChannelCurrentMode = MULTI_CH_OP_MODE_MSCLI;
        pPort->P2PCfg.CLIHwQSEL = FIFO_EDCA;

#if 0       
        MultiChannelSwicthHwQAndCh(pAd, /*FIFO_EDCA*/pPort->P2PCfg.CLIHwQSEL, pPort->P2PCfg.P2PChannel, EXTCHA_NONE);
        pPort->Channel = pPort->P2PCfg.P2PChannel;
#else
        // Get h/w q and channel
        MultiChannelGetHwQAndCh(pAd, &HwQSel, &Channel, &ExtChanOffset);
        
        MultiChannelSwicthHwQAndCh(pAd, HwQSel, Channel, ExtChanOffset);

        if ((ExtChanOffset == EXTCHA_ABOVE) || (ExtChanOffset == EXTCHA_BELOW))
            pPort->CentralChannel = Channel;
        else
            pPort->Channel = Channel;

#endif

        pPort->P2PCfg.CurrentWcid = (UCHAR)MlmeSyncGetWlanIdxByPort(pPort, ROLE_WLANIDX_P2P_CLIENT);
        
        NewStatus = 3;
    }
    // 4. GO at channel Y
    else if ((MT_TEST_BIT(pAd->pP2pCtrll->P2pMsConnectedStatus, P2pMs_CONNECTED_STATUS_CLI_OP) == FALSE) && 
        MT_TEST_BIT(pAd->pP2pCtrll->P2pMsConnectedStatus, P2pMs_CONNECTED_STATUS_GO_ON))
    {
        pAd->MccCfg.MultiChannelEnable = FALSE;
        pAd->MccCfg.MultiChannelCurrentMode = MULTI_CH_OP_MODE_MSGO;
        pPort->P2PCfg.GOHwQSEL = FIFO_EDCA;
        MultiChannelSwicthHwQAndCh(pAd, /*FIFO_EDCA*/pPort->P2PCfg.GOHwQSEL, pPort->P2PCfg.GroupOpChannel, EXTCHA_NONE);
        pPort->Channel = pPort->P2PCfg.GroupOpChannel;
        AsicResumeBssSync(pAd);
        NewStatus = 4;
    }
    else
    {
        NewStatus = 5;
    }

    DBGPRINT(RT_DEBUG_TRACE, ("%s(%d) - Adjust mulit-channel, CH(%d, %d, %d), MultiChannelEnable(%d), MultiChannelCurrentMode(%d), NewStatus(%d)\n",
                                __FUNCTION__,
                                __LINE__,
                                pPort->P2PCfg.InfraChannel,
                                pPort->P2PCfg.P2PChannel,
                                pPort->P2PCfg.GroupOpChannel,
                                pAd->MccCfg.MultiChannelEnable,
                                pAd->MccCfg.MultiChannelCurrentMode,
                                NewStatus));    
}


/* 
    ==========================================================================
    Description:
        This function will handle the multi-channel state after GO invites a new p2p device to join its group
    Return:
        None
    NOTE:
    ==========================================================================
*/
VOID MultiChannelGOStartNewConnection(
    IN PMP_ADAPTER pAd)
{
    UCHAR       Status = 0;
    UCHAR       Channel = 0;
    ULONG       HwQSel = FIFO_EDCA;
    UCHAR       ExtChanOffset = EXTCHA_NONE;
    PMP_PORT pPort = pAd->PortList[FXXK_PORT_0];
    Channel = pPort->P2PCfg.GroupOpChannel;
    // If MultiChannelEnable == TRUE, set the current mode directly
    if (pAd->MccCfg.MultiChannelEnable == TRUE)
    {   
        // Assign multi-channel start state
        pAd->MccCfg.MultiChannelCurrentMode = MULTI_CH_OP_MODE_MSGO;

        // Select proper h/w queue
        if (pPort->P2PCfg.GroupOpChannel == pPort->P2PCfg.InfraChannel)
        {
            pPort->P2PCfg.GOHwQSEL = pPort->P2PCfg.STAHwQSEL;
        }
        else if (pPort->P2PCfg.GroupOpChannel == pPort->P2PCfg.P2PChannel)
        {
            pPort->P2PCfg.GOHwQSEL = pPort->P2PCfg.CLIHwQSEL;
        }
        else
        {
            pPort->P2PCfg.GOHwQSEL = FIFO_EDCA;
        }

        pAd->MccCfg.SwQSelect = MultiChannelGetSwQ(pPort->P2PCfg.GOHwQSEL);

        // Switch channel and h/w queue
        MultiChannelSwicthHwQAndCh(pAd, pPort->P2PCfg.GOHwQSEL, pPort->P2PCfg.GroupOpChannel, EXTCHA_NONE); 
        
        // Start multichannel switch
        PlatformSetTimer(pPort, &pPort->Mlme.MultiChannelTimer, pPort->P2PCfg.SwitchChTime);  

        Status = 1;
    }
    else
    {
        if (INFRA_ON(pAd->PortList[PORT_0]) == TRUE)
        {
            //if (pPort->P2PCfg.GroupOpChannel != pPort->P2PCfg.InfraChannel)
            if (MultiChannelDecision(
                                    pPort,
                                    pPort->P2PCfg.InfraChannel, 
                                    pPort->P2PCfg.GroupOpChannel, 
                                    pPort->P2PCfg.InfraCentralChannel, 
                                    pPort->P2PCfg.InfraExtChanOffset, 
                                    pPort->P2PCfg.P2PGOCentralChannel, 
                                    pPort->P2PCfg.P2PGOExtChanOffset))            
            {
                // Enable muitl-channel state machine
                pAd->MccCfg.MultiChannelEnable = TRUE;

                // Assign multi-channel start state
                pAd->MccCfg.MultiChannelCurrentMode = MULTI_CH_OP_MODE_MSGO;

                // Select proper h/w queue
                pPort->P2PCfg.GOHwQSEL = FIFO_HCCA;

                pAd->MccCfg.SwQSelect = MultiChannelGetSwQ(pPort->P2PCfg.GOHwQSEL);

                // Get h/w q and channel
                MultiChannelGetHwQAndCh(pAd, &HwQSel, &Channel, &ExtChanOffset);

                // Switch channel and h/w queue
                //MultiChannelSwicthHwQAndCh(pAd, pPort->P2PCfg.GOHwQSEL, pPort->P2PCfg.GroupOpChannel, EXTCHA_NONE);
                MultiChannelSwicthHwQAndCh(pAd, HwQSel, Channel, ExtChanOffset);

                // Start multichannel switch
                PlatformSetTimer(pPort, &pPort->Mlme.MultiChannelTimer, pPort->P2PCfg.SwitchChTime);  

                Status = 2;
            }
        }

        if (MT_TEST_BIT(pAd->pP2pCtrll->P2pMsConnectedStatus, P2pMs_CONNECTED_STATUS_CLI_OP)) 
        {
            //if (pPort->P2PCfg.GroupOpChannel != pPort->P2PCfg.P2PChannel)
            if (MultiChannelDecision(
                                    pPort,
                                    pPort->P2PCfg.P2PChannel, 
                                    pPort->P2PCfg.GroupOpChannel, 
                                    pPort->P2PCfg.P2PCentralChannel, 
                                    pPort->P2PCfg.P2PExtChanOffset, 
                                    pPort->P2PCfg.P2PGOCentralChannel, 
                                    pPort->P2PCfg.P2PGOExtChanOffset))
            {
                // Enable muitl-channel state machine
                pAd->MccCfg.MultiChannelEnable = TRUE;

                // Assign multi-channel start state
                pAd->MccCfg.MultiChannelCurrentMode = MULTI_CH_OP_MODE_MSGO;

                // Select proper h/w queue
                pPort->P2PCfg.GOHwQSEL = FIFO_HCCA;

                pAd->MccCfg.SwQSelect = MultiChannelGetSwQ(pPort->P2PCfg.GOHwQSEL);

                // Get h/w q and channel
                MultiChannelGetHwQAndCh(pAd, &HwQSel, &Channel, &ExtChanOffset);

                // Switch channel and h/w queue
                //MultiChannelSwicthHwQAndCh(pAd, pPort->P2PCfg.GOHwQSEL, pPort->P2PCfg.GroupOpChannel, EXTCHA_NONE);
                MultiChannelSwicthHwQAndCh(pAd, HwQSel, Channel, ExtChanOffset);

                // Start multichannel switch
                PlatformSetTimer(pPort, &pPort->Mlme.MultiChannelTimer, pPort->P2PCfg.SwitchChTime);  

                Status = 3;
            }
        }
    }

    DBGPRINT(RT_DEBUG_TRACE, ("%s[%d] -MultiChSts(%d), ListenCh(%d), CH(%d, %d, %d)(%d, %d, %d), Q(%d, %d, %d), ExtChOffset(%d, %d, %d)\n", 
                                __FUNCTION__, 
                                __LINE__, 
                                Status,
                                pPort->P2PCfg.ListenChannel,
                                pPort->P2PCfg.InfraChannel, 
                                pPort->P2PCfg.P2PChannel,
                                pPort->P2PCfg.GroupOpChannel,
                                pPort->P2PCfg.InfraCentralChannel, 
                                pPort->P2PCfg.P2PCentralChannel,
                                pPort->P2PCfg.P2PGOCentralChannel,
                                pPort->P2PCfg.STAHwQSEL,
                                pPort->P2PCfg.CLIHwQSEL,
                                pPort->P2PCfg.GOHwQSEL,
                                pPort->P2PCfg.InfraExtChanOffset,
                                pPort->P2PCfg.P2PExtChanOffset,
                                pPort->P2PCfg.P2PGOExtChanOffset));   
}

/* 
    ==========================================================================
    Description:
        This function will handle the multi-channel state after p2p device becomes a CLI
    Return:
        None
    NOTE:
    ==========================================================================
*/
VOID MultiChannelCLIOutStartNewConnection(
    IN PMP_PORT pPort)
{
    PMP_ADAPTER pAd = pPort->pAd;
    UCHAR       Status = 0;
    UCHAR       Channel = 0;
    ULONG       HwQSel = FIFO_EDCA;
    UCHAR       ExtChanOffset = EXTCHA_NONE;
    PMP_PORT pInfraPort = pAd->pP2pCtrll->pInfraPort;
    Channel = pPort->P2PCfg.P2PChannel;
    // timer
    // MultiChannelCurrentMode
    // MultiChannelEnable
    // channel
    // HwQ
    // switch channel/queue
    // common channel

    if (pAd->MccCfg.MultiChannelEnable == TRUE)
    {
        // Assign multi-channel start state
        pAd->MccCfg.MultiChannelCurrentMode = MULTI_CH_OP_MODE_MSCLI;

        // Select proper h/w queue
        if (pPort->P2PCfg.P2PChannel == pPort->P2PCfg.InfraChannel)
        {
            pPort->P2PCfg.CLIHwQSEL = pPort->P2PCfg.STAHwQSEL;
        }
        else if (pPort->P2PCfg.P2PChannel == pPort->P2PCfg.GroupOpChannel)
        {
            pPort->P2PCfg.CLIHwQSEL = pPort->P2PCfg.GOHwQSEL;
        }
        else
        {
            pPort->P2PCfg.CLIHwQSEL = FIFO_EDCA;
        }

        // Switch channel and h/w queue
        MultiChannelSwicthHwQAndCh(pAd, pPort->P2PCfg.CLIHwQSEL, pPort->P2PCfg.P2PChannel, EXTCHA_NONE);

        PlatformSetTimer(pPort, &pPort->Mlme.MultiChannelTimer, pPort->P2PCfg.SwitchChTime);

        Status = 1;
    }
    else
    {
        if (INFRA_ON(pInfraPort))
        {
            DBGPRINT(RT_DEBUG_TRACE, ("%s- INFRA starts before CLI, multi-channel starts from MultiChannelCurrentMode(MULTI_CH_OP_MODE_MSCLI)\n", __FUNCTION__));
#if 0
            if (pAd->StaActive.SupportedHtPhy.ChannelWidth == BW_40)
            {
                if(pPort->P2PCfg.P2PCentralChannel != pPort->P2PCfg.InfraCentralChannel)
                {
                    // TODO: 
                }
            }
            else
#endif              
            {
                //if (pPort->P2PCfg.P2PChannel != pPort->P2PCfg.InfraChannel)
                if (MultiChannelDecision(
                                        pPort,
                                        pPort->P2PCfg.InfraChannel, 
                                        pPort->P2PCfg.P2PChannel, 
                                        pPort->P2PCfg.InfraCentralChannel, 
                                        pPort->P2PCfg.InfraExtChanOffset, 
                                        pPort->P2PCfg.P2PCentralChannel, 
                                        pPort->P2PCfg.P2PExtChanOffset))
                {
                    // Enable muitl-channel state machine
                    pAd->MccCfg.MultiChannelEnable = TRUE;

                    // Assign multi-channel start state
                    pAd->MccCfg.MultiChannelCurrentMode = MULTI_CH_OP_MODE_MSCLI;

                    // Select proper h/w queue
                    pPort->P2PCfg.CLIHwQSEL = FIFO_HCCA;

                    pAd->MccCfg.SwQSelect = MultiChannelGetSwQ(pPort->P2PCfg.CLIHwQSEL);

                    // Get h/w q and channel
                    MultiChannelGetHwQAndCh(pAd, &HwQSel, &Channel, &ExtChanOffset);

                    // PCI: Empty EDCA TX Q to avoid potential issue (during connection phase, it will use QID_HCCA, and FIFO_EDCA)
                    // USB: During connection phase, it will use MLMEContext and FIFO_EDCA
                    //MultiChannelWaitTxRingEmpty(pAd, QID_HCCA);   
                    MultiChannelWaitTxRingEmpty(pAd);   
                    MultiChannelWaitHwQEmpty(pAd, FIFO_EDCA, 0);

                    // Switch channel and h/w queue
                    MultiChannelSwicthHwQAndCh(pAd, HwQSel, Channel, ExtChanOffset);    
                    
                    // Start multichannel switch
                    PlatformSetTimer(pPort, &pPort->Mlme.MultiChannelTimer, pPort->P2PCfg.SwitchChTime);  

                    Status = 2;
                }
            }
        }

        if (MT_TEST_BIT(pAd->pP2pCtrll->P2pMsConnectedStatus, P2pMs_CONNECTED_STATUS_GO_ON))      
        {
            // Consdier only 20M case
            //if (pPort->P2PCfg.GroupOpChannel != pPort->P2PCfg.InfraChannel)
            if (MultiChannelDecision(
                                    pPort,
                                    pPort->P2PCfg.GroupOpChannel, 
                                    pPort->P2PCfg.P2PChannel, 
                                    pPort->P2PCfg.P2PGOCentralChannel, 
                                    pPort->P2PCfg.P2PGOExtChanOffset, 
                                    pPort->P2PCfg.P2PCentralChannel, 
                                    pPort->P2PCfg.P2PExtChanOffset))      
            {
                // Enable muitl-channel state machine
                pAd->MccCfg.MultiChannelEnable = TRUE;

                // Assign multi-channel start state
                pAd->MccCfg.MultiChannelCurrentMode = MULTI_CH_OP_MODE_MSCLI;

                // Select proper h/w queue
                pPort->P2PCfg.CLIHwQSEL = FIFO_HCCA;

                pAd->MccCfg.SwQSelect = MultiChannelGetSwQ(pPort->P2PCfg.CLIHwQSEL);

                // Get h/w q and channel
                MultiChannelGetHwQAndCh(pAd, &HwQSel, &Channel, &ExtChanOffset);

                // Switch channel and h/w queue
                //MultiChannelSwicthHwQAndCh(pAd, pPort->P2PCfg.CLIHwQSEL, pPort->P2PCfg.P2PChannel, EXTCHA_NONE);  
                MultiChannelSwicthHwQAndCh(pAd, HwQSel, Channel, ExtChanOffset);    

                // Start multichannel switch
                PlatformSetTimer(pPort, &pPort->Mlme.MultiChannelTimer, pPort->P2PCfg.SwitchChTime);  

                Status = 3;
            }           
        }   
    }

    // Trigger finish Infra/P2pMs session if necessary
    if (MT_TEST_BIT(pAd->pP2pCtrll->MultiChannelStartOrEndProcess, MULTI_CHANNEL_IN_NEW_SESSION))
    {
        BOOLEAN     TimerCancelled = FALSE;
        PMP_PORT    pPort = pAd->PortList[FXXK_PORT_0];
        MultiChannelResumeMsduTransmission(pAd);
        MT_CLEAR_BIT(pAd->pP2pCtrll->MultiChannelStartOrEndProcess, MULTI_CHANNEL_IN_NEW_SESSION);
        PlatformCancelTimer(&pPort->Mlme.ChannelRestoreTimer, &TimerCancelled);
    }

    MT_SET_BIT(pAd->pP2pCtrll->MultiChannelStartOrEndProcess, MULTI_CHANNEL_CLI_DHCP_DONE);

    DBGPRINT(RT_DEBUG_TRACE, ("%s[%d] -MultiChSts(%d), ListenCh(%d), CH(%d, %d, %d)(%d, %d, %d), Q(%d, %d, %d), ExtChOffset(%d, %d, %d)\n", 
                                __FUNCTION__, 
                                __LINE__, 
                                Status,
                                pPort->P2PCfg.ListenChannel,
                                pPort->P2PCfg.InfraChannel, 
                                pPort->P2PCfg.P2PChannel,
                                pPort->P2PCfg.GroupOpChannel,
                                pPort->P2PCfg.InfraCentralChannel, 
                                pPort->P2PCfg.P2PCentralChannel,
                                pPort->P2PCfg.P2PGOCentralChannel,
                                pPort->P2PCfg.STAHwQSEL,
                                pPort->P2PCfg.CLIHwQSEL,
                                pPort->P2PCfg.GOHwQSEL,
                                pPort->P2PCfg.InfraExtChanOffset,
                                pPort->P2PCfg.P2PExtChanOffset,
                                pPort->P2PCfg.P2PGOExtChanOffset));   
}

/* 
    ==========================================================================
    Description:
        This function will handle the multi-channel state after a p2p CLI join this group
    Return:
        None
    NOTE:
    ==========================================================================
*/
VOID MultiChannelCLIInStartNewConnection(
    IN PMP_ADAPTER pAd,
    IN PMP_PORT     pPort)
{
    
    // timer
    // MultiChannelCurrentMode
    // MultiChannelEnable
    // channel
    // HwQ
    // common channel

    // This will follow GO's rule --> Go's channel and queue
    
    if (pAd->MccCfg.MultiChannelEnable)
    {
        
    }
    else
    {
        
    }

    // Trigger finish Infra/P2pMs session if necessary
    if (MT_TEST_BIT(pAd->pP2pCtrll->MultiChannelStartOrEndProcess, MULTI_CHANNEL_IN_NEW_SESSION))
    {
        BOOLEAN     TimerCancelled = FALSE;
        PMP_PORT    pPort = pAd->PortList[FXXK_PORT_0];
        MultiChannelResumeMsduTransmission(pAd);
        MT_CLEAR_BIT(pAd->pP2pCtrll->MultiChannelStartOrEndProcess, MULTI_CHANNEL_IN_NEW_SESSION);
        PlatformCancelTimer(&pPort->Mlme.ChannelRestoreTimer, &TimerCancelled);
    }
}

/* 
    ==========================================================================
    Description:
        This function will handle the multi-channel state after STA conenction
    Return:
        None
    NOTE:
    ==========================================================================
*/
VOID MultiChannelSTAStartNewConnection(
    IN PMP_ADAPTER pAd,
    IN PMP_PORT     pPort)
{
    UCHAR       Status = 0;
    UCHAR       Channel = pPort->P2PCfg.InfraChannel;
    ULONG       HwQSel = FIFO_EDCA;
    UCHAR       ExtChanOffset = EXTCHA_NONE;
    //PMP_PORT pPort = pAd->PortList[FXXK_PORT_0];
    
    if (pAd->MccCfg.MultiChannelEnable == TRUE)
    {
        // Assign multi-channel start state
        pAd->MccCfg.MultiChannelCurrentMode = MULTI_CH_OP_MODE_INFRA;

        // Select proper h/w queue
        if (pPort->P2PCfg.InfraChannel == pPort->P2PCfg.P2PChannel)
        {
            pPort->P2PCfg.STAHwQSEL = pPort->P2PCfg.CLIHwQSEL;
        }
        else if (pPort->P2PCfg.InfraChannel == pPort->P2PCfg.GroupOpChannel)
        {
            pPort->P2PCfg.STAHwQSEL = pPort->P2PCfg.GOHwQSEL;
        }
        else
        {
            pPort->P2PCfg.STAHwQSEL = FIFO_EDCA;
        }

        // Switch channel and h/w queue
        MultiChannelSwicthHwQAndCh(pAd, pPort->P2PCfg.STAHwQSEL, pPort->P2PCfg.InfraChannel, EXTCHA_NONE);

        Status = 1;     
    }
    else
    {
        if (MT_TEST_BIT(pAd->pP2pCtrll->P2pMsConnectedStatus, P2pMs_CONNECTED_STATUS_CLI_OP))
        {
            //pAd->MccCfg.MultiChannelCurrentMode = MULTI_CH_OP_MODE_INFRA;
            DBGPRINT(RT_DEBUG_TRACE, ("%s- CLI starts before INFRA, multi-channel starts from MultiChannelCurrentMode(MULTI_CH_OP_MODE_INFRA)\n", __FUNCTION__));
#if 0           
            if (pAd->StaActive.SupportedHtPhy.ChannelWidth == BW_40)
            {
                if(pPort->P2PCfg.P2PCentralChannel != pPort->P2PCfg.InfraCentralChannel)
                {
                    pAd->MccCfg.MultiChannelCurrentMode = MULTI_CH_OP_MODE_INFRA;
                    pAd->MccCfg.MultiChannelEnable = TRUE;
                    DBGPRINT(RT_DEBUG_TRACE, ("%s[%d] -MultiChSts, MultiChannelEnable(BW_40)\n", __FUNCTION__, __LINE__));
                }
            }
            else
#endif              
            {
                //if ((pPort->P2PCfg.P2PChannel != pPort->P2PCfg.InfraChannel) || (pPort->P2PCfg.P2PCentralChannel != pPort->P2PCfg.InfraCentralChannel))
                if (MultiChannelDecision(
                                        pPort,
                                        pPort->P2PCfg.P2PChannel, 
                                        pPort->P2PCfg.InfraChannel, 
                                        pPort->P2PCfg.P2PCentralChannel, 
                                        pPort->P2PCfg.P2PExtChanOffset, 
                                        pPort->P2PCfg.InfraCentralChannel, 
                                        pPort->P2PCfg.InfraExtChanOffset))
                {
                    // Enable muitl-channel state machine
                    pAd->MccCfg.MultiChannelEnable = TRUE;

                    // Assign multi-channel start state
                    pAd->MccCfg.MultiChannelCurrentMode = MULTI_CH_OP_MODE_INFRA;

                    // Select proper h/w queue
                    pPort->P2PCfg.STAHwQSEL = FIFO_HCCA;

                    pAd->MccCfg.SwQSelect = /*QID_HCCA*/MultiChannelGetSwQ(pPort->P2PCfg.STAHwQSEL);

                    // Get h/w q and channel
                    MultiChannelGetHwQAndCh(pAd, &HwQSel, &Channel, &ExtChanOffset);

                    // Empty EDCA TX Q to avoid potential issue (during connection phase, it will use QID_HCCA, and FIFO_EDCA)
                    // USB: During connection phase, it will use MLMEContext and FIFO_EDCA
                    //MultiChannelWaitTxRingEmpty(pAd, QID_HCCA);   
                    MultiChannelWaitTxRingEmpty(pAd);   
                    MultiChannelWaitHwQEmpty(pAd, FIFO_EDCA, 0);

                    // Switch channel and h/w queue
                    MultiChannelSwicthHwQAndCh(pAd, HwQSel, Channel, ExtChanOffset);    
                    
                    // Start multichannel switch
                    PlatformSetTimer(pPort, &pPort->Mlme.MultiChannelTimer, pPort->P2PCfg.SwitchChTime);  

                    Status = 2;                 
                }
            }
        }

        if (MT_TEST_BIT(pAd->pP2pCtrll->P2pMsConnectedStatus, P2pMs_CONNECTED_STATUS_GO_ON))      
        {
            // Consdier only 20M case
            //if ((pPort->P2PCfg.GroupOpChannel != pPort->P2PCfg.InfraChannel) || (pPort->P2PCfg.GroupOpChannel != pPort->P2PCfg.InfraCentralChannel))
            if (MultiChannelDecision(
                                    pPort,
                                    pPort->P2PCfg.GroupOpChannel, 
                                    pPort->P2PCfg.InfraChannel, 
                                    pPort->P2PCfg.P2PGOCentralChannel, 
                                    pPort->P2PCfg.P2PGOExtChanOffset, 
                                    pPort->P2PCfg.InfraCentralChannel, 
                                    pPort->P2PCfg.InfraExtChanOffset))
            {
                // Enable muitl-channel state machine
                pAd->MccCfg.MultiChannelEnable = TRUE;

                // Assign multi-channel start state
                pAd->MccCfg.MultiChannelCurrentMode = MULTI_CH_OP_MODE_INFRA;

                // Select proper h/w queue
                pPort->P2PCfg.STAHwQSEL = FIFO_HCCA;

                pAd->MccCfg.SwQSelect = MultiChannelGetSwQ(pPort->P2PCfg.STAHwQSEL);

                MultiChannelGetHwQAndCh(pAd, &HwQSel, &Channel, &ExtChanOffset);

                // Switch channel and h/w queue
                MultiChannelSwicthHwQAndCh(pAd, HwQSel, Channel, ExtChanOffset);

                // Start multichannel switch
                PlatformSetTimer(pPort, &pPort->Mlme.MultiChannelTimer, pPort->P2PCfg.SwitchChTime);  

                Status = 3;
            }           
        }
    }

    // Trigger finish Infra/P2pMs session if necessary
    if (MT_TEST_BIT(pAd->pP2pCtrll->MultiChannelStartOrEndProcess, MULTI_CHANNEL_IN_NEW_SESSION))
    {
        BOOLEAN     TimerCancelled = FALSE;
        PMP_PORT    pPort = pAd->PortList[FXXK_PORT_0];
        MultiChannelResumeMsduTransmission(pAd);
        MT_CLEAR_BIT(pAd->pP2pCtrll->MultiChannelStartOrEndProcess, MULTI_CHANNEL_IN_NEW_SESSION);
        PlatformCancelTimer(&pPort->Mlme.ChannelRestoreTimer, &TimerCancelled);
    }
    
    // Stop channel switch
    //MultiChannelSwitchStop(pAd);

    MT_SET_BIT(pAd->pP2pCtrll->MultiChannelStartOrEndProcess, MULTI_CHANNEL_STA_DHCP_DONE);

    DBGPRINT(RT_DEBUG_TRACE, ("%s[%d] -MultiChSts(%d), ListenCh(%d), CH(%d, %d, %d)(%d, %d, %d), Q(%d, %d, %d), ExtChOffset(%d, %d, %d)\n", 
                                __FUNCTION__, 
                                __LINE__, 
                                Status,
                                pPort->P2PCfg.ListenChannel,
                                pPort->P2PCfg.InfraChannel, 
                                pPort->P2PCfg.P2PChannel,
                                pPort->P2PCfg.GroupOpChannel,
                                pPort->P2PCfg.InfraCentralChannel, 
                                pPort->P2PCfg.P2PCentralChannel,
                                pPort->P2PCfg.P2PGOCentralChannel,
                                pPort->P2PCfg.STAHwQSEL,
                                pPort->P2PCfg.CLIHwQSEL,
                                pPort->P2PCfg.GOHwQSEL,
                                pPort->P2PCfg.InfraExtChanOffset,
                                pPort->P2PCfg.P2PExtChanOffset,
                                pPort->P2PCfg.P2PGOExtChanOffset));   
}

/* 
    ==========================================================================
    Description:
        This fucntion stops multi-channel switch
    Return:

    NOTE:
         
    ==========================================================================
*/
VOID MultiChannelSwitchStop(
    IN PMP_ADAPTER pAd) 
{
    BOOLEAN TimerCancelled = FALSE;
    PMP_PORT    pPort = pAd->PortList[FXXK_PORT_0];

    if (pAd->MccCfg.MultiChannelEnable == TRUE)
    {
        // Stop multichannel switch timer
        PlatformCancelTimer(&pPort->Mlme.MultiChannelTimer, &TimerCancelled);
        
        OPSTATUS_CLEAR_FLAG(pAd, fOP_STATUS_MEDIA_STATE_MULTI_CHANNEL_SWITCHING);
        OPSTATUS_CLEAR_FLAG(pAd, fOP_STATUS_MEDIA_STATE_MULTI_CHANNEL_WITH_NULLFRAME);
        DBGPRINT(RT_DEBUG_TRACE, ("%s \n", __FUNCTION__));
    }
    else
    {
        DBGPRINT(RT_DEBUG_TRACE, ("%s - There is no MultiChannelSwitchTimerCallback.\n", __FUNCTION__));
    }
}

/* 
    ==========================================================================
    Description:
        This function enables all TX H/Q Q
    Return:

    NOTE:
         
    ==========================================================================
*/
VOID MultiChannelEnabeAllHwQ(
    IN PMP_ADAPTER pAd) 
{
    PBF_CFG_STRUC_EXT       PbfCfg = {0};
    MAC_SYS_CTRL_STRUC  MacSysCtrl = {0};
    //
    // Select H/W Q
    //
    RTUSBReadMACRegister(pAd, PBF_CFG(pAd), &PbfCfg.word);
    WRITE_PBF_CFG_TX2QENABLE(pAd, &PbfCfg, 1);
    WRITE_PBF_CFG_TX1QENABLE(pAd, &PbfCfg, 1);
    WRITE_PBF_CFG_TX0QENABLE(pAd, &PbfCfg, 1);
    //PbfCfg.field.Tx2QEnable = 1;
    //PbfCfg.field.Tx1QEnable = 1;
    //PbfCfg.field.Tx0QEnable = 1;

    DBGPRINT(RT_DEBUG_TRACE, ("%s - MultiChSts : Enable all HW Q, Value = %x\n", __FUNCTION__, PbfCfg.word));       

    RTUSBWriteMACRegister(pAd, PBF_CFG(pAd), PbfCfg.word);

    //RTMPResumeMsduTransmission(pAd, pPortForTimer);
    // TODO: 2012-04-23 need to check
    MultiChannelResumeMsduTransmission(pAd);

    //
    // Emable MAC Tx/Rx
    //
    RTUSBReadMACRegister(pAd, MAC_SYS_CTRL, &MacSysCtrl.word);
    MacSysCtrl.field.MacRxEna = 1;
    MacSysCtrl.field.MacTxEna = 1;
    RTUSBWriteMACRegister(pAd, MAC_SYS_CTRL, MacSysCtrl.word);  
}

/* 
    ==========================================================================
    Description:
        This function maintain the failure case during mulit-channel state
    Return:

    NOTE:
         
    ==========================================================================
*/
VOID MultiChannelNewP2pConnectionFail(
    IN PMP_ADAPTER pAd) 
{
    UCHAR NewStatus = 0;
    PMP_PORT pPort = pAd->PortList[FXXK_PORT_0];
    // Start from new state
    if (pAd->MccCfg.MultiChannelEnable == TRUE)
    {
        // STA
        if (INFRA_ON(pAd->PortList[PORT_0]))
        {
            pAd->MccCfg.MultiChannelCurrentMode = MULTI_CH_OP_MODE_INFRA;
            MultiChannelSwicthHwQAndCh(pAd, pPort->P2PCfg.STAHwQSEL, pPort->P2PCfg.InfraChannel, EXTCHA_NONE);
            pPort->Channel = pPort->P2PCfg.InfraChannel;
            NewStatus = 1;
        }
        // CLI
        else if (MT_TEST_BIT(pAd->pP2pCtrll->P2pMsConnectedStatus, P2pMs_CONNECTED_STATUS_CLI_OP))
        {
            pAd->MccCfg.MultiChannelCurrentMode = MULTI_CH_OP_MODE_MSCLI;
            MultiChannelSwicthHwQAndCh(pAd, pPort->P2PCfg.CLIHwQSEL, pPort->P2PCfg.P2PChannel, EXTCHA_NONE);
            pPort->Channel = pPort->P2PCfg.P2PChannel;
            NewStatus = 2;
        }
        // GO
        else if (MT_TEST_BIT(pAd->pP2pCtrll->P2pMsConnectedStatus, P2pMs_CONNECTED_STATUS_GO_ON))
        {
            pAd->MccCfg.MultiChannelCurrentMode = MULTI_CH_OP_MODE_MSGO;
            MultiChannelSwicthHwQAndCh(pAd, pPort->P2PCfg.GOHwQSEL, pPort->P2PCfg.GroupOpChannel, EXTCHA_NONE);
            pPort->Channel = pPort->P2PCfg.GroupOpChannel;
            NewStatus = 3;
        }

        // Stop channel switch
        //MultiChannelSwitchStop(pAd);
        // Set next state timer
        PlatformSetTimer(pPort, &pPort->Mlme.MultiChannelTimer, pPort->P2PCfg.SwitchChTime);      
    }
    else 
    {
        // STA
        if (INFRA_ON(pAd->PortList[PORT_0]))
        {
            MultiChannelSwicthHwQAndCh(pAd, pPort->P2PCfg.STAHwQSEL, pPort->P2PCfg.InfraChannel, EXTCHA_NONE);
            NewStatus = 4;
        }
        // CLI
        else if (MT_TEST_BIT(pAd->pP2pCtrll->P2pMsConnectedStatus, P2pMs_CONNECTED_STATUS_CLI_OP))
        {
            MultiChannelSwicthHwQAndCh(pAd, pPort->P2PCfg.CLIHwQSEL, pPort->P2PCfg.P2PChannel, EXTCHA_NONE);
            NewStatus = 5;
        }
        // GO
        else if (MT_TEST_BIT(pAd->pP2pCtrll->P2pMsConnectedStatus, P2pMs_CONNECTED_STATUS_GO_ON))
        {
            MultiChannelSwicthHwQAndCh(pAd, pPort->P2PCfg.GOHwQSEL, pPort->P2PCfg.GroupOpChannel, EXTCHA_NONE);
            NewStatus = 6;
        }
    }

    DBGPRINT(RT_DEBUG_TRACE, ("%s - MultiChSts : NewStatus(%d)\n", __FUNCTION__, NewStatus));
}

/* 
    ==========================================================================
    Description:
        This function defers the OID of disconnect request for STA/CLI during multi-channel phase
    Return:

    NOTE:
         
    ==========================================================================
*/
NDIS_STATUS MultiChannelDeferDisconnectRequest( 
    IN PMP_ADAPTER pAd, 
    IN  PMP_PORT  pPort)
{
    NDIS_STATUS ndisStatus = NDIS_STATUS_SUCCESS;

    if (pAd->MccCfg.PendedDisconnectOidRequest == NULL)
    {
        DBGPRINT(RT_DEBUG_TRACE, ("%s - PendedDisconnectOidRequest(NULL), Return\n", __FUNCTION__));
        return NDIS_STATUS_FAILURE;
    }

    if (pPort == NULL)
    {
        DBGPRINT(RT_DEBUG_TRACE, ("%s - pPort(NULL), Return\n", __FUNCTION__));
        return NDIS_STATUS_FAILURE;
    }
    
    DBGPRINT(RT_DEBUG_TRACE, ("%s - ===>\n", __FUNCTION__));

    // Stop channel switch
    MultiChannelSwitchStop(pAd);
    
    // Do disconnection
    ndisStatus = DisconnectRequest(
                                pAd, 
                                pPort, 
                                (PULONG)&pAd->MccCfg.PendedDisconnectOidRequest->DATA.SET_INFORMATION.BytesRead, 
                                (PULONG)&pAd->MccCfg.PendedDisconnectOidRequest->DATA.SET_INFORMATION.BytesNeeded, 
                                FALSE);
#if 0 // OS will trigger port0 disconnect after wfd disconnection, port0 disconnection might fail to be executed, move indication to end of MultiChannelXXXStop to avoid race condition 
    // Complete this OID to the OS  
    NdisMOidRequestComplete(
                        pAd->AdapterHandle,
                        pAd->MccCfg.PendedDisconnectOidRequest,
                        ndisStatus);

    pAd->MccCfg.PendedDisconnectOidRequest = NULL;
    pAd->MccCfg.pDisconnectPort = NULL;
#endif

    DBGPRINT(RT_DEBUG_TRACE, ("%s - <===\n", __FUNCTION__));

    return ndisStatus;
}

/* 
    ==========================================================================
    Description:
        This function defers the OID of StartGo during multi-channel phase
    Return:

    NOTE:
         
    ==========================================================================
*/
NDIS_STATUS MultiChannelDeferStartGoRequest( 
    IN PMP_ADAPTER pAd, 
    IN  PMP_PORT  pPort)
{
    NDIS_STATUS ndisStatus = NDIS_STATUS_SUCCESS;

    if (pAd->MccCfg.PendedGoOidRequest == NULL)
    {
        DBGPRINT(RT_DEBUG_TRACE, ("%s - PendedGoOidRequest(NULL), Return\n", __FUNCTION__));
        return NDIS_STATUS_FAILURE;
    }

    if (pPort == NULL)
    {
        DBGPRINT(RT_DEBUG_TRACE, ("%s - pPort(NULL), Return\n", __FUNCTION__));
        return NDIS_STATUS_FAILURE;
    }
    
    DBGPRINT(RT_DEBUG_TRACE, ("%s - ===>\n", __FUNCTION__));

#if(COMPILE_WIN8_ABOVE(CURRENT_OS_NDIS_VER))  
    // Do disconnection
    ndisStatus = P2pMsStartGOReq(pAd, pPort);
#endif

#if 0
    // Complete this OID to the OS  
    NdisMOidRequestComplete(
                        pAd->AdapterHandle,
                        pAd->MccCfg.PendedGoOidRequest,
                        ndisStatus);    

    pAd->MccCfg.PendedGoOidRequest = NULL;
    pPort = NULL;
#endif

    DBGPRINT(RT_DEBUG_TRACE, ("%s - <===\n", __FUNCTION__));

    return ndisStatus;
}

/* 
    ==========================================================================
    Description:
        This function defers the OID of Scan during multi-channel phase
    Return:

    NOTE:
         
    ==========================================================================
*/
NDIS_STATUS MultiChannelDeferScanRequest( 
    IN PMP_ADAPTER pAd, 
    IN  PMP_PORT  pPort)
{
    NDIS_STATUS ndisStatus = NDIS_STATUS_SUCCESS;

    if (pAd->pNicCfg->PendedScanRequest == NULL)
    {
        DBGPRINT(RT_DEBUG_TRACE, ("%s - PendedScanRequest(NULL), Return\n", __FUNCTION__));
        return NDIS_STATUS_FAILURE;
    }

    if (pPort == NULL)
    {
        DBGPRINT(RT_DEBUG_TRACE, ("%s - pPort(NULL), Return\n", __FUNCTION__));
        return NDIS_STATUS_FAILURE;
    }
    
    DBGPRINT(RT_DEBUG_TRACE, ("%s - ===>\n", __FUNCTION__));

    // Do scan here
    ndisStatus = MpScanRequest(
                        pAd,
                        pPort,
                        pAd->pNicCfg->PendedScanRequest->DATA.SET_INFORMATION.InformationBuffer,
                        pAd->pNicCfg->PendedScanRequest->DATA.SET_INFORMATION.InformationBufferLength,
                        (PULONG)&pAd->pNicCfg->PendedScanRequest->DATA.SET_INFORMATION.BytesRead,
                        (PULONG)&pAd->pNicCfg->PendedScanRequest->DATA.SET_INFORMATION.BytesNeeded   
                        );

    DBGPRINT(RT_DEBUG_TRACE, ("%s - <===\n", __FUNCTION__));

    return ndisStatus;
}


/* 
    ==========================================================================
    Description:
        This function defers the OID of StopGo during multi-channel phase
    Return:

    NOTE:
         
    ==========================================================================
*/
NDIS_STATUS MultiChannelDeferStopGoRequest( 
    IN PMP_ADAPTER pAd, 
    IN  PMP_PORT  pPort)
{
    NDIS_STATUS ndisStatus = NDIS_STATUS_SUCCESS;

    if (pAd->MccCfg.PendedGoOidRequest == NULL)
    {
        DBGPRINT(RT_DEBUG_TRACE, ("%s - PendedGoOidRequest(NULL), Return\n", __FUNCTION__));
        return NDIS_STATUS_FAILURE;
    }

    if (pPort == NULL)
    {
        DBGPRINT(RT_DEBUG_TRACE, ("%s - pPort(NULL), Return\n", __FUNCTION__));
        return NDIS_STATUS_FAILURE;
    }
    
    DBGPRINT(RT_DEBUG_TRACE, ("%s - ===>\n", __FUNCTION__));

    // Do disconnection
    ndisStatus = RTMPInfoResetRequest(
                            pAd,
                            pPort,
                            pAd->MccCfg.PendedGoOidRequest->DATA.METHOD_INFORMATION.InformationBuffer,
                            pAd->MccCfg.PendedGoOidRequest->DATA.METHOD_INFORMATION.InputBufferLength,
                            pAd->MccCfg.PendedGoOidRequest->DATA.METHOD_INFORMATION.OutputBufferLength,
                            (PULONG)&pAd->MccCfg.PendedGoOidRequest->DATA.METHOD_INFORMATION.BytesWritten,
                            (PULONG)&pAd->MccCfg.PendedGoOidRequest->DATA.METHOD_INFORMATION.BytesRead,
                            (PULONG)&pAd->MccCfg.PendedGoOidRequest->DATA.METHOD_INFORMATION.BytesNeeded
                            );

#if 0   
    // Complete this OID to the OS  
    NdisMOidRequestComplete(
                        pAd->AdapterHandle,
                        pAd->MccCfg.PendedGoOidRequest,
                        ndisStatus);    

    pAd->MccCfg.PendedGoOidRequest = NULL;
    pPort = NULL;
#endif

    DBGPRINT(RT_DEBUG_TRACE, ("%s - <===\n", __FUNCTION__));

    return ndisStatus;
}

/* 
    ==========================================================================
    Description:
        This function defers the OID of reset request for STA/CLI during multi-channel phase
    Return:

    NOTE:
         
    ==========================================================================
*/
NDIS_STATUS MultiChannelDeferResetRequest( 
    IN PMP_ADAPTER pAd, 
    IN  PMP_PORT  pPort)
{
    NDIS_STATUS ndisStatus = NDIS_STATUS_SUCCESS;

    if (pAd->MccCfg.PendedResetRequest == NULL)
    {
        DBGPRINT(RT_DEBUG_TRACE, ("%s - PendedResetRequest(NULL), Return\n", __FUNCTION__));
        return NDIS_STATUS_FAILURE;
    }

    if (pPort == NULL)
    {
        DBGPRINT(RT_DEBUG_TRACE, ("%s - pPort(NULL), Return\n", __FUNCTION__));
        return NDIS_STATUS_FAILURE;
    }
    
    DBGPRINT(RT_DEBUG_TRACE, ("%s - ===>\n", __FUNCTION__));

    // Do disconnection
    ndisStatus = RTMPInfoResetRequest(
                            pAd,
                            pPort,
                            pAd->MccCfg.PendedResetRequest->DATA.METHOD_INFORMATION.InformationBuffer,
                            pAd->MccCfg.PendedResetRequest->DATA.METHOD_INFORMATION.InputBufferLength,
                            pAd->MccCfg.PendedResetRequest->DATA.METHOD_INFORMATION.OutputBufferLength,
                            (PULONG)&pAd->MccCfg.PendedResetRequest->DATA.METHOD_INFORMATION.BytesWritten,
                            (PULONG)&pAd->MccCfg.PendedResetRequest->DATA.METHOD_INFORMATION.BytesRead,
                            (PULONG)&pAd->MccCfg.PendedResetRequest->DATA.METHOD_INFORMATION.BytesNeeded
                            );  
#if 0   
    // Complete this OID to the OS  
    NdisMOidRequestComplete(
                        pAd->AdapterHandle,
                        pAd->MccCfg.PendedResetRequest,
                        ndisStatus);    

    pAd->MccCfg.PendedResetRequest = NULL;
    pPort = NULL;
#endif

    DBGPRINT(RT_DEBUG_TRACE, ("%s - <===", __FUNCTION__));

    return ndisStatus;
}

/* 
    ==========================================================================
    Description:
        Indicate the defer request oid
    Return:

    NOTE:
         
    ==========================================================================
*/
VOID MultiChannelDeferRequestIndication( 
    IN PMP_ADAPTER pAd, 
    IN PUCHAR   pInBuffer)
{
    UCHAR                           Status = 0;
    UCHAR                           Type = PENDED_DISCONNECT_OID_REQUEST;
    NDIS_STATUS                     ndisStatus = NDIS_STATUS_SUCCESS;   
    PMULTI_CH_DEFER_OID_STRUCT      pMultiChDeferOid = NULL;

    if (pInBuffer != NULL)
    {
        pMultiChDeferOid = (PMULTI_CH_DEFER_OID_STRUCT)pInBuffer;
        Type = pMultiChDeferOid->Type;
        ndisStatus = pMultiChDeferOid->ndisStatus;
    }
    else
    {
        DBGPRINT(RT_DEBUG_TRACE, ("%s - ERROR\n", __FUNCTION__));
        return;
    }

    if (Type == PENDED_DISCONNECT_OID_REQUEST)
    {
        // Complete this OID to the OS  
        NdisMOidRequestComplete(
                            pAd->AdapterHandle,
                            pAd->MccCfg.PendedDisconnectOidRequest,
                            ndisStatus);

        pAd->MccCfg.PendedDisconnectOidRequest = NULL;
        pAd->MccCfg.pDisconnectPort = NULL;

        Status = 1;
    }
    else if (Type == PENDED_START_GO_OID_REQUEST)
    {
        NdisMOidRequestComplete(
                            pAd->AdapterHandle,
                            pAd->MccCfg.PendedGoOidRequest,
                            ndisStatus);    

        pAd->MccCfg.PendedGoOidRequest = NULL;
        pAd->MccCfg.pGoPort = NULL;
        
        Status = 2;
    }
    else if (Type == PENDED_STOP_GO_OID_REQUEST)
    {
        NdisMOidRequestComplete(
                            pAd->AdapterHandle,
                            pAd->MccCfg.PendedGoOidRequest,
                            ndisStatus);    

        pAd->MccCfg.PendedGoOidRequest = NULL;
        pAd->MccCfg.pGoPort = NULL;    
        
        Status = 3;
    }
    else if (Type == PENDED_RESET_OID_REQUEST)
    {
        NdisMOidRequestComplete(
                            pAd->AdapterHandle,
                            pAd->MccCfg.PendedResetRequest,
                            ndisStatus);    

        pAd->MccCfg.PendedResetRequest = NULL;
        pAd->MccCfg.pResetRequestPort = NULL;
        
        Status = 4;
    }
    else if (Type == PENDED_SCAN_OID_REQUEST)
    {       
        NdisMOidRequestComplete(
                            pAd->AdapterHandle,
                            pAd->pNicCfg->PendedScanRequest,
                            ndisStatus);    

        pAd->pNicCfg->PendedScanRequest = NULL;
        pAd->MccCfg.pScanRequestPort = NULL;
        
        Status = 5;
    }

    DBGPRINT(RT_DEBUG_TRACE, ("%s - Status(%d)\n", __FUNCTION__, Status));  
}

/* 
    ==========================================================================
    Description:
        This function decides the op channel of GO during after STA/CLI connection
    Return:

    NOTE:
         
    ==========================================================================
*/
VOID MultiChannelDecideGOOpCh(
    IN PMP_ADAPTER pAd,
    IN PMP_PORT     pPort)
{
    UCHAR   Status = 0;

    if (MT_TEST_BIT(pAd->pP2pCtrll->P2pMsConnectedStatus, P2pMs_CONNECTED_STATUS_GO_ON) == FALSE)
    {
        // CLI is connected, GO doesn't on, GO follows CLI ch info
        if (MT_TEST_BIT(pAd->pP2pCtrll->P2pMsConnectedStatus, P2pMs_CONNECTED_STATUS_CLI_ASSOC))
        {           
            // STA connects and has different ch with CLI
            if (INFRA_ON(pAd->PortList[PORT_0]) &&
                (pPort->P2PCfg.InfraChannel != pPort->P2PCfg.P2PChannel))
            {
                // GO follow CLI ch info.
                pPort->P2PCfg.GroupChannel = pPort->P2PCfg.P2PChannel;
                pPort->P2PCfg.GroupOpChannel = pPort->P2PCfg.P2PChannel;
                // We can change listen channel to this working channel if sta is connecting.
                pPort->P2PCfg.ListenChannel = pPort->P2PCfg.P2PChannel;
                pPort->P2PCfg.GOHwQSEL = pPort->P2PCfg.CLIHwQSEL;
                Status = 1;
            }
            // STA connects and has the same ch with CLI
            else if (INFRA_ON(pAd->PortList[PORT_0]) &&
                (pPort->P2PCfg.InfraChannel == pPort->P2PCfg.P2PChannel))
            {
                // GO follow CLI ch info.
                pPort->P2PCfg.GroupChannel = pPort->P2PCfg.P2PChannel;
                pPort->P2PCfg.GroupOpChannel = pPort->P2PCfg.P2PChannel;
                // We can change listen channel to this working channel if sta is connecting.
                pPort->P2PCfg.ListenChannel = pPort->P2PCfg.P2PChannel;
                pPort->P2PCfg.GOHwQSEL = FIFO_EDCA;
                Status = 2; 
            }
            // STA doesn't connect
            else if (INFRA_ON(pAd->PortList[PORT_0]) == FALSE)
            {
                // GO follow its own rule --> GO follow CLI ch info.
                pPort->P2PCfg.GroupChannel = pPort->P2PCfg.P2PChannel;
                pPort->P2PCfg.GroupOpChannel = pPort->P2PCfg.P2PChannel;
                // We can change listen channel to this working channel if sta is connecting.
                pPort->P2PCfg.ListenChannel = pPort->P2PCfg.P2PChannel;
                pPort->P2PCfg.GOHwQSEL = FIFO_EDCA;
                Status = 3;
            }
            else
            {
                // Other case
                Status = 4;
            }

            DBGPRINT(RT_DEBUG_TRACE, ("%s - [CLI Linkup] GO will follow New channel setting(%d). ListenCh(%d), GroupOpCh(%d), GOHwQSEL(%d)\n", 
                                        __FUNCTION__,
                                        Status,
                                        pPort->P2PCfg.ListenChannel, 
                                        pPort->P2PCfg.GroupOpChannel,
                                        pPort->P2PCfg.GOHwQSEL));
        }
        // STA linkup
        else if (INFRA_ON(pAd->PortList[PORT_0]))
        {           
            // CLI connects and has different ch with STA
            if (MT_TEST_BIT(pAd->pP2pCtrll->P2pMsConnectedStatus, P2pMs_CONNECTED_STATUS_CLI_ASSOC) &&
            (pPort->P2PCfg.InfraChannel != pPort->P2PCfg.P2PChannel))
            {
            // GO follow CLI ch info.
            pPort->P2PCfg.GroupChannel = pPort->P2PCfg.P2PChannel;
            pPort->P2PCfg.GroupOpChannel = pPort->P2PCfg.P2PChannel;
            // We can change listen channel to this working channel if sta is connecting.
            pPort->P2PCfg.ListenChannel = pPort->P2PCfg.P2PChannel;
            pPort->P2PCfg.GOHwQSEL = FIFO_HCCA;
            Status = 1;
        }
        // CLI connects and has the same ch with STA
        else if (MT_TEST_BIT(pAd->pP2pCtrll->P2pMsConnectedStatus, P2pMs_CONNECTED_STATUS_CLI_ASSOC) &&
            (pPort->P2PCfg.InfraChannel == pPort->P2PCfg.P2PChannel))
        {
            // GO follow CLI ch info.
            pPort->P2PCfg.GroupChannel = pPort->P2PCfg.P2PChannel;
            pPort->P2PCfg.GroupOpChannel = pPort->P2PCfg.P2PChannel;
            // We can change listen channel to this working channel if sta is connecting.
            pPort->P2PCfg.ListenChannel = pPort->P2PCfg.P2PChannel;
            pPort->P2PCfg.GOHwQSEL = FIFO_EDCA;
            Status = 2; 
        }
        // CLI doesn't connect
        else if (MT_TEST_BIT(pAd->pP2pCtrll->P2pMsConnectedStatus, P2pMs_CONNECTED_STATUS_CLI_ASSOC) == FALSE)
        {
            // GO follow its own rule --> GO follow STA ch info.
            pPort->P2PCfg.GroupChannel = pPort->P2PCfg.InfraChannel;
            pPort->P2PCfg.GroupOpChannel = pPort->P2PCfg.InfraChannel;
            // We can change listen channel to this working channel if sta is connecting.
            pPort->P2PCfg.ListenChannel = pPort->P2PCfg.InfraChannel;
            pPort->P2PCfg.GOHwQSEL = FIFO_EDCA;
            Status = 3;
        }
        else
        {
            // Other case
            Status = 4;
        }

        DBGPRINT(RT_DEBUG_TRACE, ("%s - [STA Linkup] GO will follow New channel setting(%d). ListenCh(%d), GroupOpCh(%d), GOHwQSEL(%d)\n", 
                                __FUNCTION__,
                                Status,
                                pPort->P2PCfg.ListenChannel, 
                                pPort->P2PCfg.GroupOpChannel,
                                pPort->P2PCfg.GOHwQSEL));
        }
    }
}

/* 
    ==========================================================================
    Description:
        This function waits for the empty of specific TX HW Q
    Return:

    NOTE:
         
    ==========================================================================
*/
VOID MultiChannelWaitHwQEmpty(
    IN PMP_ADAPTER pAd,
    IN UCHAR            QueIdx,
    IN UINT32           NewI)
{
    UCHAR   i = 0;
    UCHAR   SwQIdx = MultiChannelGetSwQ(QueIdx);
    TXRXQ_PCNT_STRUC    TxRxQPcnt = {0};
    UINT32  EndOfI = 140;
    ULONG   FreeNum = 0;    

    if (NewI == 0)
    {
        EndOfI = 140;
    }
    else
    {
        EndOfI = NewI;
        DBGPRINT(RT_DEBUG_TRACE, ("%s : EndOfI(%d)\n", __FUNCTION__, EndOfI));
    }

    do
    {
        i++;

#if 0
        if (pAd->TxRing[SwQIdx].TxSwFreeIdx > pAd->TxRing[SwQIdx].TxCpuIdx)
            FreeNum = pAd->TxRing[SwQIdx].TxSwFreeIdx - pAd->TxRing[SwQIdx].TxCpuIdx -1;
        else
            FreeNum = pAd->TxRing[SwQIdx].TxSwFreeIdx + TX_RING_SIZE - pAd->TxRing[SwQIdx].TxCpuIdx -1;
#endif      
        
        RTUSBReadMACRegister(pAd, TXRXQ_PCNT, &TxRxQPcnt.word);

        if (QueIdx == FIFO_EDCA)
        {
            DBGPRINT(RT_DEBUG_TRACE, ("%s : [%d] EDCA(%d)\n", __FUNCTION__, i, TxRxQPcnt.field.Tx2QPcnt));

            if (TxRxQPcnt.field.Tx2QPcnt != 0)
            {
                NdisCommonGenericDelay(100);
            }
            else
            {
                break;
            }
        }
        else if (QueIdx == FIFO_HCCA) 
        {
            DBGPRINT(RT_DEBUG_TRACE, ("%s : [%d] HCCA(%d)\n", __FUNCTION__, i, TxRxQPcnt.field.Tx1QPcnt));

            if (TxRxQPcnt.field.Tx1QPcnt != 0)
            {
                NdisCommonGenericDelay(100);
            }
            else
            {
                break;
            }
            
        }
        else
        {
            break;
        }
    } while (i < EndOfI);
}

/* 
    ==========================================================================
    Description:
        This function waits for the empty of TX ring
        Data ring BulkOutPipeId: QID_AC_BE(0), QID_AC_BK(1), QID_AC_VI(2), QID_AC_VO(3)
        MLME ring: 
    Return:

    NOTE:
         
    ==========================================================================
*/
VOID MultiChannelWaitTxRingEmpty(
    IN PMP_ADAPTER    pAd)
{
    UCHAR   i = 0;

#if 0   // Data Ring
    do
    {
        i++;

        if ((pAd->pHifCfg->TxContext[BulkOutPipeId].ENextBulkOutPosition == pAd->pHifCfg->TxContext[BulkOutPipeId].CurWritePosition)
            || ((pAd->pHifCfg->TxContext[BulkOutPipeId].ENextBulkOutPosition-8) == pAd->pHifCfg->TxContext[BulkOutPipeId].CurWritePosition))
        {
            // Ring is empty, exit delay loop
            break;
        }
        else
        {
            // Ring is not empty, wait for empty
            NdisCommonGenericDelay(100);
        }
        
    } while (i < 10);
#else
    do  // MLME(Magagement) Ring
    {
        i++;

        if (pAd->pHifCfg->MLMEContext[pAd->pHifCfg->NextMLMEIndex].InUse == FALSE)
        {
            // Ring is empty, exit delay loop
            DBGPRINT(RT_DEBUG_TRACE, ("%s : [%d] MLME Ring is empty\n", __FUNCTION__, i));
            break;
        }
        else
        {
            // Ring is not empty, wait for empty
            DBGPRINT(RT_DEBUG_TRACE, ("%s : [%d] MLME Ring is not empty\n", __FUNCTION__, i));
            NdisCommonGenericDelay(100);
        }
        
    } while (i < 10);

#endif
}

/* 
    ==========================================================================
    Description:
        This function gets the SW queue with specific H/W Q
    Return:

    NOTE:
         
    ==========================================================================
*/
UCHAR   MultiChannelGetSwQ(
    IN UCHAR    HwQ)
{
    if (HwQ == FIFO_EDCA)
    {
        return QID_AC_BE;
    }
    else if (HwQ == FIFO_HCCA)
    {
        return (QID_HCCA - 1);
    }
    else
    {
        return QID_AC_BE;
    }
}

/* 
    ==========================================================================
    Description:
        This function gets TX H/W Q of specific port
    Return:

    NOTE:
         
    ==========================================================================
*/
UCHAR   MultiChannelGetHwQ(
     IN PMP_ADAPTER pAd,
    IN  PMP_PORT  pPort)
{
    
    if (pPort->PortType == WFD_GO_PORT)
    {
        return pPort->P2PCfg.GOHwQSEL;
    }
    else if (pPort->PortType == WFD_CLIENT_PORT)
    {
        return pPort->P2PCfg.CLIHwQSEL;
    }
    else
    {
        return pPort->P2PCfg.STAHwQSEL;;
    }
}

/* 
    ==========================================================================
    Description:
        This function reset the defualt H/W Q for port
    Return:

    NOTE:
         
    ==========================================================================
*/
VOID MultiChannelSetDeafultHwQ(
     IN PMP_ADAPTER pAd,
    IN  PMP_PORT  pPort)
{
    UCHAR Status = 0;
    
    if (pPort->PortType == EXTSTA_PORT)
    {
        pPort->P2PCfg.STAHwQSEL = FIFO_EDCA;
        MT_CLEAR_BIT(pAd->pP2pCtrll->MultiChannelStartOrEndProcess, MULTI_CHANNEL_STA_DHCP_DONE);
        pPort->P2PCfg.InfraCentralChannel = CHANNEL_OFF;
        pPort->P2PCfg.InfraExtChanOffset = EXTCHA_NONE;
        Status = 1;
    }
    else if (pPort->PortType == WFD_CLIENT_PORT)
    {
        pPort->P2PCfg.CLIHwQSEL = FIFO_EDCA;
        MT_CLEAR_BIT(pAd->pP2pCtrll->MultiChannelStartOrEndProcess, MULTI_CHANNEL_CLI_DHCP_DONE);
        pPort->P2PCfg.P2PCentralChannel = CHANNEL_OFF;
        pPort->P2PCfg.P2PExtChanOffset = EXTCHA_NONE;
        Status = 2;
    }
    else if (pPort->PortType == WFD_GO_PORT)
    {
        pPort->P2PCfg.GOHwQSEL = FIFO_EDCA;
        pPort->P2PCfg.P2PGOCentralChannel = CHANNEL_OFF;
        pPort->P2PCfg.P2PGOExtChanOffset = EXTCHA_NONE;
        Status = 3;
    }
    else
    {
        Status = 4; 
    }
    
    DBGPRINT(RT_DEBUG_TRACE,("%s - Reset Port(%d) HwQ to default, Status(%d)\n", __FUNCTION__, pPort->PortNumber, Status));
}

/* 
    ==========================================================================
    Description:
        This function suspends the TX MSDU
    Return:

    NOTE:
         
    ==========================================================================
*/
VOID MultiChannelSuspendMsduTransmission(
    IN  PMP_ADAPTER   pAd)
{
    TX_RTS_CFG_STRUC RtsCfg;
    PMP_PORT pPort = pAd->PortList[FXXK_PORT_0];

    if (MT_TEST_BIT(pAd->pP2pCtrll->MultiChannelStartOrEndProcess, MULTI_CHANNEL_MSDU_TX_SUSPEND) == FALSE)
    {
        DBGPRINT(RT_DEBUG_TRACE,("%s...\n", __FUNCTION__));
        
        // no carrier detection when scanning
        if (pPort->CommonCfg.bCarrierDetect && pPort->CommonCfg.bCarrierDetectPhase)
        {
            AsicSetCarrierDetectAction(pAd, CARRIER_ACTION_STOP, 0);
        }

        //
        // Before BSS_SCAN_IN_PROGRESS, we need to keep Current R66 value and
        // use Lowbound as R66 value on MlmeSyncScanMlmeSyncNextChannel(...)
        //
        RTUSBReadBBPRegister(pAd, BBP_R66, &pAd->HwCfg.BbpTuning.R66CurrentValue);

        // set BBP_R66 to 0x30/0x40 when scanning (AsicSwitchChannel will set R66 according to channel when scanning)
        RTMPSetAGCInitValue(pAd, BW_20);

        MT_SET_BIT(pAd->pP2pCtrll->MultiChannelStartOrEndProcess, MULTI_CHANNEL_MSDU_TX_SUSPEND);

        // Switch to Main Antenna for diversity solution
        if (pAd->HwCfg.NicConfig2.field.AntDiv != ANT_DIV_CONFIG_DISABLE) 
            AsicSetRxAnt(pAd, 0);
        
        pAd->Mlme.ChannelStayCount = 0;
        //
        // Update RTS threshold, disable RTS threshold function while scan on progress.
        //
#if 1
#ifdef MULTI_CHANNEL_SUPPORT    
        if (pAd->MccCfg.MultiChannelEnable == FALSE)
#endif /*MULTI_CHANNEL_SUPPORT*/    
#endif
        {
            RTUSBReadMACRegister(pAd, TX_RTS_CFG, &RtsCfg.word);
            RtsCfg.field.RtsThres = 0xffff;
            RTUSBWriteMACRegister(pAd, TX_RTS_CFG, RtsCfg.word);        
        }
    }
}

/* 
    ==========================================================================
    Description:
        This function resumes the TX MSDU
    Return:

    NOTE:
         
    ==========================================================================
*/
VOID MultiChannelResumeMsduTransmission(
    IN  PMP_ADAPTER   pAd)
{
    TX_RTS_CFG_STRUC RtsCfg;
    PMP_PORT pPort = pAd->PortList[FXXK_PORT_0];

    if (MT_TEST_BIT(pAd->pP2pCtrll->MultiChannelStartOrEndProcess, MULTI_CHANNEL_MSDU_TX_SUSPEND) == TRUE)
    {
        DBGPRINT(RT_DEBUG_TRACE,("%s...\n", __FUNCTION__));
        
        if (pPort->CommonCfg.bCarrierDetect && pPort->CommonCfg.bCarrierDetectPhase)
        {
            if (pPort->CommonCfg.bCarrierAssert)
            {
                // Carrier assertion, driver can not send anything, change detection with no cts-to-self
                AsicSetCarrierDetectAction(pAd, CARRIER_ACTION_NO_CTS, 1000);
            }
            else
            {
                AsicSetCarrierDetectAction(pAd, CARRIER_ACTION_ONE_CTS, 1000);
            }
        }
        
        //
        // After finish BSS_SCAN_IN_PROGRESS, we need to restore Current R66 value
        // R66 should not be 0
        //
        //ASSERT(pAd->HwCfg.BbpTuning.R66CurrentValue != 0);
        if (pAd->HwCfg.BbpTuning.R66CurrentValue == 0)
        {
            pAd->HwCfg.BbpTuning.R66CurrentValue = 0x38;
            DBGPRINT_ERR(("RTMPResumeMsduTransmission, R66CurrentValue=0...\n"));
        }
        
        if (pAd->HwCfg.BbpTuning.bEnable == TRUE)
        {
            RTUSBWriteBBPRegister(pAd, BBP_R66, pAd->HwCfg.BbpTuning.R66CurrentValue);    
        }

        MT_CLEAR_BIT(pAd->pP2pCtrll->MultiChannelStartOrEndProcess, MULTI_CHANNEL_MSDU_TX_SUSPEND);
        
        //  
        // Update RTS threshold, enable RTS threshold function after scan
        //
#if 0
#ifdef MULTI_CHANNEL_SUPPORT    
        if (pAd->MccCfg.MultiChannelEnable == FALSE)
#endif /*MULTI_CHANNEL_SUPPORT*/
#endif  
        {
            if (pPort->CommonCfg.RtsThreshold < 2347)
            {
                RTUSBReadMACRegister(pAd, TX_RTS_CFG, &RtsCfg.word);
                RtsCfg.field.RtsThres = pPort->CommonCfg.RtsThreshold;
                RTUSBWriteMACRegister(pAd, TX_RTS_CFG, RtsCfg.word);                
            }
        }

        P2pResumeMsduAction(pAd, pPort);
        
        NdisSetEvent(&(GET_THREAD_EVENT(&pAd->pHifCfg->hDequeueDataThread)));
    }
}

/* 
    ==========================================================================
    Description:
        This function gets TX H/W Q/channel/ExtchannelOffset infromation with a specific MultiChannelCurrentMode
    Return:

    NOTE:
         
    ==========================================================================
*/
VOID MultiChannelGetHwQAndCh(
    IN PMP_ADAPTER pAd,
    IN PULONG pHwQSel,
    IN PUCHAR pChannel,
    IN PUCHAR pExtChanOffset)
{
    UCHAR Status = 0;
    PMP_PORT pPort = pAd->PortList[FXXK_PORT_0];
    *pExtChanOffset = EXTCHA_NONE;
    
    if (pAd->MccCfg.MultiChannelCurrentMode == MULTI_CH_OP_MODE_INFRA)
    {
        *pHwQSel = pPort->P2PCfg.STAHwQSEL;
        

        if ((pPort->P2PCfg.InfraCentralChannel != CHANNEL_OFF) && ((pPort->P2PCfg.InfraExtChanOffset == EXTCHA_ABOVE) || (pPort->P2PCfg.InfraExtChanOffset == EXTCHA_BELOW)))
        {
            *pChannel = pPort->P2PCfg.InfraCentralChannel;
            *pExtChanOffset = pPort->P2PCfg.InfraExtChanOffset;
        Status = 1;
        }
        else
        {
            *pChannel = pPort->P2PCfg.InfraChannel;
            Status = 2;
        }

        // For dynamic tx rate control
        pPort->P2PCfg.CurrentWcid = (UCHAR)MlmeSyncGetWlanIdxByPort(pPort, ROLE_WLANIDX_BSSID);
    }
    else if (pAd->MccCfg.MultiChannelCurrentMode == MULTI_CH_OP_MODE_MSCLI)
    {
        PMP_PORT    pPort = pAd->PortList[FXXK_PORT_0];
        *pHwQSel = pPort->P2PCfg.CLIHwQSEL;


        if ((pPort->P2PCfg.P2PCentralChannel != CHANNEL_OFF) && ((pPort->P2PCfg.P2PExtChanOffset == EXTCHA_ABOVE) || (pPort->P2PCfg.P2PExtChanOffset == EXTCHA_BELOW)))
        {
            *pChannel = pPort->P2PCfg.P2PCentralChannel;
            *pExtChanOffset = pPort->P2PCfg.P2PExtChanOffset;
            Status = 3;
        }
        else
        {
            *pChannel = pPort->P2PCfg.P2PChannel;
            Status = 4;
        }

        pPort->P2PCfg.CurrentWcid = (UCHAR)MlmeSyncGetWlanIdxByPort(pPort, ROLE_WLANIDX_P2P_CLIENT);    
    }
    else if (pAd->MccCfg.MultiChannelCurrentMode == MULTI_CH_OP_MODE_MSGO)
    {
        *pHwQSel = pPort->P2PCfg.GOHwQSEL;
        //*pChannel = pPort->P2PCfg.P2PGOChannel;

        if ((pPort->P2PCfg.P2PGOCentralChannel != CHANNEL_OFF) && ((pPort->P2PCfg.P2PGOExtChanOffset == EXTCHA_ABOVE) || (pPort->P2PCfg.P2PGOExtChanOffset == EXTCHA_BELOW)))
        {
            *pChannel = pPort->P2PCfg.P2PGOCentralChannel;
            *pExtChanOffset = pPort->P2PCfg.P2PGOExtChanOffset;
            Status = 5;
        }
        else
        {
            *pChannel = pPort->P2PCfg.GroupOpChannel;
            Status = 6;
        }

        pPort->P2PCfg.CurrentWcid = RESERVED_WCID;
    }

    DBGPRINT(RT_DEBUG_TRACE,("%s - Status(%d), MultiChannelCurrentMode(%d),  HwQSe(%d), Ch(%d), ExtChOffset(%d)\n", 
                            __FUNCTION__,
                            Status,
                            pAd->MccCfg.MultiChannelCurrentMode,
                            *pHwQSel,
                            *pChannel,
                            *pExtChanOffset));
}

VOID RtmpSetSwTSF(
    IN PMP_ADAPTER pAd,
    IN PMP_PORT pPort,
    ULONGLONG     TimeStamp)
{
    LARGE_INTEGER   Now;

    NdisGetCurrentSystemTime(&Now);
    
    if (pPort->PortType == EXTSTA_PORT)
    {
        pPort->P2PCfg.STABaseTime = (Now.QuadPart / 10);
        pPort->P2PCfg.STAOffsetime = TimeStamp;
        DBGPRINT(RT_DEBUG_TRACE,("%s - Update STA TSF to (%I64d)\n", __FUNCTION__, TimeStamp));
    }
    else if (pPort->PortType == WFD_CLIENT_PORT)
    {
        pPort->P2PCfg.CLIBaseTime = (Now.QuadPart / 10);
        pPort->P2PCfg.CLIOffsetTime = TimeStamp;
        DBGPRINT(RT_DEBUG_TRACE,("%s - Update CLI TSF to (%I64d)\n", __FUNCTION__, TimeStamp));
    }
}

VOID RtmpGetSwTSF(
    IN PMP_ADAPTER pAd,
    IN PMP_PORT pPort,
    IN ULONGLONG *pTSF)
{
    LARGE_INTEGER   Now;

    NdisGetCurrentSystemTime(&Now);
    
    if (pPort->PortType == EXTSTA_PORT)
    {
        *pTSF = (Now.QuadPart / 10 - pPort->P2PCfg.STABaseTime + pPort->P2PCfg.STAOffsetime + 5000);
        DBGPRINT(RT_DEBUG_TRACE,("%s - Display STA TSF(%I64d)\n", __FUNCTION__, *pTSF));
    }
    else if (pPort->PortType == WFD_CLIENT_PORT)
    {
        *pTSF = (Now.QuadPart / 10 - pPort->P2PCfg.CLIBaseTime + pPort->P2PCfg.CLIOffsetTime + 5000);
        DBGPRINT(RT_DEBUG_TRACE,("%s - Display CLI TSF (%I64d)\n", __FUNCTION__, *pTSF));
    }
    
}

BOOLEAN MultiChannelDecision(
    IN PMP_PORT pPort,
    IN UCHAR PrimaryCh1,
    IN UCHAR PrimaryCh2,
    IN UCHAR CentralCh1,
    IN UCHAR ExtChanOffset1,
    IN UCHAR CentralCh2,
    IN UCHAR ExtChanOffset2)
{
    PMP_ADAPTER pAd = pPort->pAd;
    UCHAR       Status = 0;
    BOOLEAN     MultiChannel = FALSE;
    PMP_PORT    pInfraPort = pAd->pP2pCtrll->pInfraPort;
    PMP_PORT    pWfdGOPort = pAd->pP2pCtrll->pWfdGOPort;
    PMP_PORT    pWfdCLTPort = pAd->pP2pCtrll->pWfdCLTPort;
    PMP_PORT    pWfdCLT2Port = pAd->pP2pCtrll->pWfdCLT2Port;

    DBGPRINT(RT_DEBUG_TRACE, ("%s    ==> Port %d \n", __FUNCTION__, pPort->PortNumber));

    if (INFRA_ON(pInfraPort))
    {
        if (pInfraPort->Channel != pPort->Channel)
        {
            MultiChannel = TRUE;
            Status = 1;
            DBGPRINT(RT_DEBUG_TRACE, ("%s(%d) MultiChannel = TRUE  Port[%d]->Channel = %d pWfdGOPort[%d]->Channel = %d \n", __FUNCTION__, __LINE__, pPort->PortNumber, pPort->Channel, pInfraPort->PortNumber, pInfraPort->Channel));
        }
        else if (pInfraPort->CentralChannel != pPort->CentralChannel)
        {
            MultiChannel = TRUE;
            Status = 2;
            DBGPRINT(RT_DEBUG_TRACE, ("%s(%d) MultiChannel = TRUE  Port[%d]->CentralChannel = %d pWfdGOPort[%d]->CentralChannel = %d \n", __FUNCTION__, __LINE__, pPort->PortNumber, pPort->CentralChannel, pInfraPort->PortNumber, pInfraPort->CentralChannel));
        }
        else if (pInfraPort->BBPCurrentBW != pPort->BBPCurrentBW)
        {
            MultiChannel = TRUE;
            Status = 3;
            DBGPRINT(RT_DEBUG_TRACE, ("%s(%d) MultiChannel = TRUE  Port[%d]->BBPCurrentBW = %d pWfdGOPort[%d]->BBPCurrentBW = %d \n", __FUNCTION__, __LINE__, pPort->PortNumber, pPort->BBPCurrentBW, pInfraPort->PortNumber, pInfraPort->BBPCurrentBW));
        }
        
    }
    else if (INFRA_ON(pWfdGOPort))
    {
        if (pWfdGOPort->Channel != pPort->Channel)
        {
            MultiChannel = TRUE;
            Status = 4;
            DBGPRINT(RT_DEBUG_TRACE, ("%s(%d) MultiChannel = TRUE  Port[%d]->Channel = %d pInfraPort[%d]->Channel = %d \n", __FUNCTION__, __LINE__, pPort->PortNumber, pPort->Channel, pWfdGOPort->PortNumber, pWfdGOPort->Channel));
        }
        else if (pWfdGOPort->CentralChannel != pPort->CentralChannel)
        {
            MultiChannel = TRUE;
            Status = 5;
            DBGPRINT(RT_DEBUG_TRACE, ("%s(%d) MultiChannel = TRUE  Port[%d]->CentralChannel = %d pInfraPort[%d]->CentralChannel = %d \n", __FUNCTION__, __LINE__, pPort->PortNumber, pPort->CentralChannel, pWfdGOPort->PortNumber, pWfdGOPort->CentralChannel));
        }
        else if (pWfdGOPort->BBPCurrentBW != pPort->BBPCurrentBW)
        {
            MultiChannel = TRUE;
            Status = 6;
            DBGPRINT(RT_DEBUG_TRACE, ("%s(%d) MultiChannel = TRUE  Port[%d]->BBPCurrentBW = %d pInfraPort[%d]->BBPCurrentBW = %d \n", __FUNCTION__, __LINE__, pPort->PortNumber, pPort->BBPCurrentBW, pWfdGOPort->PortNumber, pWfdGOPort->BBPCurrentBW));
        }

    }
    else if (INFRA_ON(pWfdCLTPort))
    {
        if (pWfdCLTPort->Channel != pPort->Channel)
        {
            MultiChannel = TRUE;
            Status = 7;
            DBGPRINT(RT_DEBUG_TRACE, ("%s(%d) MultiChannel = TRUE  Port[%d]->Channel = %d pWfdCLTPort[%d]->Channel = %d \n", __FUNCTION__, __LINE__, pPort->PortNumber, pPort->Channel, pWfdCLTPort->PortNumber, pWfdCLTPort->Channel));
        }
        else if (pWfdCLTPort->CentralChannel != pPort->CentralChannel)
        {
            MultiChannel = TRUE;
            Status = 8;
            DBGPRINT(RT_DEBUG_TRACE, ("%s(%d) MultiChannel = TRUE  Port[%d]->CentralChannel = %d pWfdCLTPort[%d]->CentralChannel = %d \n", __FUNCTION__, __LINE__, pPort->PortNumber, pPort->CentralChannel, pWfdCLTPort->PortNumber, pWfdCLTPort->CentralChannel));
        }
        else if (pWfdCLTPort->BBPCurrentBW != pPort->BBPCurrentBW)
        {
            MultiChannel = TRUE;
            Status = 9;
            DBGPRINT(RT_DEBUG_TRACE, ("%s(%d) MultiChannel = TRUE  Port[%d]->BBPCurrentBW = %d pWfdCLTPort[%d]->BBPCurrentBW = %d \n", __FUNCTION__, __LINE__, pPort->PortNumber, pPort->BBPCurrentBW, pWfdCLTPort->PortNumber, pWfdCLTPort->BBPCurrentBW));
        }
        
    }
    else if (INFRA_ON(pWfdCLT2Port))
    {
        if (pWfdCLT2Port->Channel != pPort->Channel)
        {
            MultiChannel = TRUE;
            Status = 7;
            DBGPRINT(RT_DEBUG_TRACE, ("%s(%d) MultiChannel = TRUE  Port[%d]->Channel = %d pWfdCLT2Port[%d]->Channel = %d \n", __FUNCTION__, __LINE__, pPort->PortNumber, pPort->Channel, pWfdCLT2Port->PortNumber, pWfdCLT2Port->Channel));
        }
        else if (pWfdCLT2Port->CentralChannel != pPort->CentralChannel)
        {
            MultiChannel = TRUE;
            Status = 8;
            DBGPRINT(RT_DEBUG_TRACE, ("%s(%d) MultiChannel = TRUE  Port[%d]->CentralChannel = %d pWfdCLT2Port[%d]->CentralChannel = %d \n", __FUNCTION__, __LINE__, pPort->PortNumber, pPort->CentralChannel, pWfdCLT2Port->PortNumber, pWfdCLT2Port->CentralChannel));
        }
        else if (pWfdCLT2Port->BBPCurrentBW != pPort->BBPCurrentBW)
        {
            MultiChannel = TRUE;
            Status = 9;
            DBGPRINT(RT_DEBUG_TRACE, ("%s(%d) MultiChannel = TRUE  Port[%d]->BBPCurrentBW = %d pWfdCLT2Port[%d]->BBPCurrentBW = %d \n", __FUNCTION__, __LINE__, pPort->PortNumber, pPort->BBPCurrentBW, pWfdCLT2Port->PortNumber, pWfdCLT2Port->BBPCurrentBW));
        }
        
    }





#if 0
    if ((ExtChanOffset1 == EXTCHA_NONE) && (ExtChanOffset2 == EXTCHA_NONE))
    {
        if (PrimaryCh1 != PrimaryCh2)
        {
            MultiChannel = TRUE;
            Status = 1;
        }
        else
        {
            Status = 2;
        }
    }
    else if ((ExtChanOffset1 == EXTCHA_NONE) && (ExtChanOffset2 != EXTCHA_NONE))
    {
        MultiChannel = TRUE;
        Status = 3;
    }
    else if ((ExtChanOffset1 != EXTCHA_NONE) && (ExtChanOffset2 == EXTCHA_NONE))
    {
        MultiChannel = TRUE;
        Status = 4;
    }
    else if ((ExtChanOffset1 != EXTCHA_NONE) && (ExtChanOffset2 != EXTCHA_NONE))
    {
        if (ExtChanOffset1 != ExtChanOffset2)
        {
            MultiChannel = TRUE;
            Status = 5;
        }
        else if ((ExtChanOffset1 == ExtChanOffset2) && (CentralCh1 != CentralCh2))
        {
            MultiChannel = TRUE;
            Status = 6;
        }
        else
        {
            Status = 7;
        }
    }
#endif
    DBGPRINT(RT_DEBUG_TRACE,("%s - Status(%d), MultiChannel = %d\n", __FUNCTION__, Status, MultiChannel));

    return MultiChannel;
}

VOID MultiChannelResumeNextMode(
    IN PMP_ADAPTER pAd,
    IN PMP_PORT     pPort)
{
    UCHAR       Channel = pPort->P2PCfg.InfraChannel;
    ULONG       HwQSel = FIFO_EDCA;
    UCHAR       ExtChanOffset = EXTCHA_NONE;
//    PMP_PORT pPort = pAd->PortList[FXXK_PORT_0];

    DBGPRINT(RT_DEBUG_TRACE, ("%s - Current Mode(%d) to be resumed.\n", __FUNCTION__, pAd->MccCfg.MultiChannelCurrentMode));

    // Switch to next mode
    MultiChannelGetHwQAndCh(pAd, &HwQSel, &Channel, &ExtChanOffset);
    MultiChannelSwicthHwQAndCh(pAd, HwQSel, Channel, ExtChanOffset);

    // Stop current timer if necessary
    MultiChannelSwitchStop(pAd);
    
    // Set next state timer
    PlatformSetTimer(pPort, &pPort->Mlme.MultiChannelTimer, pPort->P2PCfg.SwitchChTime);
}

BOOLEAN
MultiChannelCmdHandler(
    IN PMP_ADAPTER pAd,
    IN NDIS_PORT_NUMBER PortNumber,
    IN UINT     Cmd,
    IN PUCHAR      buffer,
    IN ULONG    bufflength
    )
{
    PMP_PORT pPort = pAd->PortList[PortNumber];
    switch (Cmd)
    {
#ifdef MULTI_CHANNEL_SUPPORT
        case MT_CMD_MULTI_CHANNEL_SWITCH_EXEC:
            {
                MultiChannelSwitchExec(pPort, buffer);
            }
            break;
#if(COMPILE_WIN8_ABOVE(CURRENT_OS_NDIS_VER))                   
        case MT_CMD_MULTI_CHANNEL_P2pMs_START_NEW_SESSION:
            {
                P2pMsStartActionFrameExec(pAd, pPort);
            }
            break;
        case MT_CMD_MULTI_CHANNEL_P2pMs_CH_RESTORE:
            {
                P2pMsChannelRestoreExec(pAd);
            }
            break;
#endif                      
            case MT_CMD_MLME_PERIODIC_EXEC_PASSIVE_LEVEL:
            {
             MlmePeriodicExecPassivLevel(pAd);
            }
            break;

                     
        case MT_CMD_MULTI_CHANNEL_SCAN_START_EXEC:
            {
                MultiChannelDeferRequestIndication(pAd, buffer);
            }
            break;
        case MT_CMD_MULTI_CHANNEL_SCAN_DONE_EXEC:
            {
                MultiChannelResumeNextMode(pAd, pPort);
            }
        break;
#endif /*MULTI_CHANNEL_SUPPORT*/        
    }

    return TRUE;
}

VOID
MultiChannelCmdThread(
    IN PMP_ADAPTER pAd
    )
{
    PCmdQElmt   cmdqelmt;
    PUCHAR      pData;
    ULONG       DataLen = 0;
    
    FUNC_ENTER;
    
    KeSetPriorityThread(KeGetCurrentThread(), LOW_REALTIME_PRIORITY);

    ASSERT(KeGetCurrentIrql() == PASSIVE_LEVEL);        

    while (pAd->TerminateThreads != TRUE)
    {
        PlatformWaitEventTriggeredAndReset(&(GET_THREAD_EVENT(&pAd->MccCfg.hMultiChannelThread)),0);   
        DBGPRINT(RT_DEBUG_INFO, ("P2PCmdThread Thread Triggered\n"));
        
        while (pAd->MccCfg.MultiChannelCmdQ.size > 0)
        {
            NdisAcquireSpinLock(&pAd->MccCfg.MultiChannelCmdQLock);
            RTUSBDequeueCmd(&pAd->MccCfg.MultiChannelCmdQ, &cmdqelmt);
            NdisReleaseSpinLock(&pAd->MccCfg.MultiChannelCmdQLock);
            
            if (cmdqelmt == NULL)
                break;   
                
                pData = cmdqelmt->buffer;
                DataLen = cmdqelmt->bufferlength;

                if ((!MT_TEST_FLAG(pAd, fRTMP_ADAPTER_NIC_NOT_EXIST))    && 
                    (!MT_TEST_FLAG(pAd, fRTMP_ADAPTER_HALT_IN_PROGRESS)) &&
                    (!MT_TEST_FLAG(pAd, fRTMP_ADAPTER_SLEEP_IN_PROGRESS)))
                {
                    MultiChannelCmdHandler(pAd, cmdqelmt->PortNum, cmdqelmt->command, pData, DataLen);
                }
                
            MtReturnInternalCmdBuffer(pAd, cmdqelmt);         
       }  //while (pAd->MccCfg.MultiChannelCmdQ.size > 0)
    }   
    
    FUNC_LEAVE;
}

#endif /* MULTI_CHANNEL_SUPPORT */

/* 
    ==========================================================================
    Description:
        This function sets the central channel for multi-channel
    Return:

    NOTE:
         
    ==========================================================================
*/
VOID MultiChannelSetCentralCh(
    IN PMP_ADAPTER pAd,
    IN PMP_PORT pPort,
    IN UCHAR Channel,
    IN UCHAR ExtChanOffset,
    IN USHORT ChannelWidth)
{
    UCHAR Status = 0;
    
    if ((ExtChanOffset != EXTCHA_NONE) && (ChannelWidth == BW_40))
    {
        if (pPort->PortType == EXTSTA_PORT)
        {
            pPort->P2PCfg.InfraCentralChannel = Channel;
            pPort->P2PCfg.InfraExtChanOffset = ExtChanOffset;
            Status = 1;
        }
        else if (pPort->PortType == WFD_CLIENT_PORT)
        {
            pPort->P2PCfg.P2PCentralChannel = Channel;
            pPort->P2PCfg.P2PExtChanOffset = ExtChanOffset;
            Status = 2;         
        }
        else if (pPort->PortType == WFD_GO_PORT)
        {
            pPort->P2PCfg.P2PGOCentralChannel = Channel;
            pPort->P2PCfg.P2PGOExtChanOffset = ExtChanOffset;
            Status = 3;         
        }
        else
        {
            // Error
            Status = 4;
        }

        pPort->CentralChannel = Channel;
    }

    DBGPRINT(RT_DEBUG_TRACE,("%s - Assign Port(%d) with CentralCh(%d), ExtChanOffset(%d), Status(%d)\n", 
                            __FUNCTION__,
                            pPort->PortNumber,
                            Channel,
                            ExtChanOffset,
                            Status));
}

BOOLEAN
P2PCmdHandler(
    IN PMP_ADAPTER pAd,
    IN PMP_PORT     pPort,
    IN UINT     Cmd,
    IN PUCHAR      buffer,
    IN ULONG    bufflength
    )
{
    USHORT  i=0;
    
    switch (Cmd)
    {
        case MT_CMD_P2P_PERIODIC_EXECUT:
            P2pEventMaintain(pAd);
            DBGPRINT(RT_DEBUG_TRACE, ("%s   port[%d]->P2PDiscoProvState = %d", __FUNCTION__, pPort->PortNumber, pPort->P2PCfg.P2PDiscoProvState));
            if (P2P_ON(pPort))
                P2pPeriodicExec(pAd, pPort);
            break;
        case MT_CMD_P2P_SWNOATIMEOUT:
            P2pGPTimeOutHandle(pAd);
            break;
        case MT_CMD_P2P_STOP_GO:
            P2pDown(pAd, pAd->PortList[pPort->P2PCfg.PortNumber]);
            P2pStopGo(pAd, pAd->PortList[pPort->P2PCfg.PortNumber]);
            pPort->P2PCfg.P2PConnectState = P2P_CONNECT_IDLE;
            P2pDefaultListenChannel(pAd, pPort);
            // If GoIntentIdx is 16, it means I am configured to be AutoGO. Unless press "disconnect" command
            // from GUI. In other case, for example, all station leaves my group, should not really stop GO.
            // So restart AutoGO now.
            if (pPort->P2PCfg.P2PDiscoProvState != P2P_DISABLE)
            {
                if (!INFRA_ON(pAd->PortList[PORT_0]))
                    pPort->P2PCfg.P2pCounter.CounterAftrScanButton = 0;
                pPort->P2PCfg.P2PConnectState = P2P_CONNECT_IDLE;
                P2pGotoScan(pAd, pPort, P2P_DISCO_TYPE_DEFAULT, 2);
                pPort->P2PCfg.Dpid = DEV_PASS_ID_NOSPEC;
            }
            if (IS_P2P_AUTOGO(pPort))
            {
                P2pStartAutoGo(pAd, pPort);
            }
            break;
        case MT_CMD_P2P_STOP_CLIENT:
            P2pDown(pAd, pAd->PortList[pPort->P2PCfg.PortNumber]);
            pPort->P2PCfg.P2PConnectState = P2P_CONNECT_IDLE;
            pPort->P2PCfg.P2pCounter.Counter100ms = 0;
            // Set a randon period to start next Listen State.
            pPort->P2PCfg.P2pCounter.NextScanRound = (RandomByte(pAd) % P2P_RANDOM_BASE) + P2P_RANDOM_BIAS;

            for (i = 0;i < MAX_P2P_GROUP_SIZE;i++)
            {
                if (pAd->pP2pCtrll->P2PTable.Client[i].P2pClientState >= P2PSTATE_INVITE_COMMAND)
                    P2pGroupTabDelete(pAd, (UCHAR)i, pAd->pP2pCtrll->P2PTable.Client[i].addr);

                P2P_CLEAR_FLAG(&pAd->pP2pCtrll->P2PTable.Client[i], P2PFLAG_PASSED_NEGO_REQ);
                P2P_CLEAR_FLAG(&pAd->pP2pCtrll->P2PTable.Client[i], P2PFLAG_GO_INVITE_ME);
            }

            if((pPort->P2PCfg.P2PConnectState >= P2P_INVITE) &&
                ((pPort->P2PCfg.P2PConnectState != P2P_I_AM_CLIENT_OP) &&(pPort->P2PCfg.P2PConnectState != P2P_I_AM_GO_OP)) )
            {
                DBGPRINT(RT_DEBUG_TRACE, ("not complete connection so p2pconnectState from %s to %s", decodeP2PState(pPort->P2PCfg.P2PConnectState), decodeP2PState(pPort->P2PCfg.P2PConnectState)));
                pPort->P2PCfg.P2PConnectState = P2P_CONNECT_IDLE;
                    
            }
            
            if (pPort->P2PCfg.P2PDiscoProvState != P2P_DISABLE)
            {
                pPort->P2PCfg.P2pCounter.CounterAftrScanButton = 0;
                pPort->P2PCfg.P2PConnectState = P2P_CONNECT_IDLE;
                P2pGotoScan(pAd, pPort, P2P_DISCO_TYPE_DEFAULT, 2);
                pPort->P2PCfg.Dpid = DEV_PASS_ID_NOSPEC;
            }
            PlatformZeroMemory( pPort->P2PCfg.PinCode, 8);
            PlatformZeroMemory( pPort->StaCfg.WscControl.RegData.PIN, 8);
            pPort->StaCfg.WscControl.RegData.PINLen = 0;
            P2pConnectForward(pAd);
            break; 

        case MT_CMD_P2pMs_RADIO_ON:
            {
                // In STAMlmePeriodicExec, it waits for 30sec if P2pMs is set to be available.
                pAd->Mlme.OneSecPeriodicRound = 0;
                AsicRadioOn(pAd);
            }
            break;

        case MT_CMD_P2P_UPDATE_BEACON:
            if (IS_P2P_MS_GO(pAd, pAd->PortList[pPort->P2PCfg.GOPortNumber]))
            {
                //stop beacon
                //AsicPauseBssSync(pAd);
                //[Win8] update GO beacon
                GOUpdateBeaconFrame(pAd, pAd->PortList[pPort->P2PCfg.GOPortNumber]);
                //activate beacon again
                //AsicResumeBssSync(pAd);
            }
            break;
            
        case MT_CMD_P2P_SET_PSM_CFG:
            
            if(IS_P2P_SIGMA_ON(pPort))
            {
                TX_RTY_CFG_STRUC    TxRtyCfg;
                TX_RTS_CFG_STRUC    TxRtsCfg;
                if(IS_P2P_PSM(pPort))
                {                           
                    RTUSBReadMACRegister(pAd, TX_RTY_CFG, &TxRtyCfg.word);
                    TxRtyCfg.field.LongRtyLimit = 0;
                    TxRtyCfg.field.ShortRtyLimit = 0;
                    RTUSBWriteMACRegister(pAd, TX_RTY_CFG, TxRtyCfg.word);

                    RTUSBReadMACRegister(pAd, TX_RTS_CFG, &TxRtsCfg.word);
                    TxRtsCfg.field.RtsThres = 0xFFFF;
                    TxRtsCfg.field.AutoRtsRetryLimit = 0;
                    RTUSBWriteMACRegister(pAd, TX_RTS_CFG, TxRtsCfg.word);
                }
                else
                {
                    RTUSBReadMACRegister(pAd, TX_RTY_CFG, &TxRtyCfg.word);
                    TxRtyCfg.field.LongRtyLimit = 0x1f;
                    TxRtyCfg.field.ShortRtyLimit = 0x1f;
                    RTUSBWriteMACRegister(pAd, TX_RTY_CFG, TxRtyCfg.word);

                    RTUSBReadMACRegister(pAd, TX_RTS_CFG, &TxRtsCfg.word);
                    TxRtsCfg.field.RtsThres = pPort->CommonCfg.RtsThreshold;
                    TxRtsCfg.field.AutoRtsRetryLimit = 7;
                    RTUSBWriteMACRegister(pAd, TX_RTS_CFG, TxRtsCfg.word);
                }
            } 
            break;

            case MT_CMD_P2P_READ_TXFIFO:
                {
                    BOOLEAN     GotAck = FALSE;
                    TX_STA_FIFO_STRUC   StaFifo;
                    TX_STAT_FIFO_EXT_STRUC TxStatFifoExt = {0};
                    UCHAR               TxPktId = 0;
                    UCHAR               pid = 0, wcid = 0;
                    //ULONG     temp;
                    // monitor txdone status to detect ACK
                    DBGPRINT(RT_DEBUG_TRACE, ("===> MT_CMD_P2P_READ_TXFIFO\n"));
                    i = 0;
                      
                    do
                    {
                
                        RTUSBReadMACRegister(pAd, TX_STA_FIFO, &StaFifo.word);
                        pid = (UCHAR)StaFifo.field.PidType;
                        wcid = (UCHAR)StaFifo.field.wcid;

                        TxPktId = (UCHAR)(StaFifo.field.PidType);

                        if (StaFifo.field.bValid == 0)
                        {
                           // break;
                        }
                            
#if(COMPILE_WIN8_ABOVE(CURRENT_OS_NDIS_VER))  
                        pPort->P2PCfg.bP2pAckReq = TRUE;
                        
                        if ( (pPort->P2PCfg.bP2pAckReq == TRUE) 
                            /*&&(NDIS_WIN8_ABOVE(pAd)) && (TxPktId == PID_P2pMs_INDICATE) && (StaFifo.field.TxSuccess)*/)
                        {
                            //DBGPRINT(RT_DEBUG_ERROR, ("PID_P2pMs_INDICATE and TxSuccess(%d) wcid = %d, i = %d\n",StaFifo.field.TxSuccess, wcid, i));

                            // send below command to queue to avoid race condition with action frame handle mechanism
                            // Got ACK and tirigger Got ACK state machine
                            MTEnqueueInternalCmd(pAd, pPort, MT_CMD_P2pMs_SEND_COMPLETE_IND, NULL, 0);

                            // Disable periodic ACK check
                            pPort->P2PCfg.bP2pAckReq = FALSE;                                       
                            GotAck = TRUE;                                   
                        }
#endif  /* NDIS630_MINIPORT */
                        if ((TxPktId == PID_REQUIRE_ACK) && (StaFifo.field.TxSuccess))
                        {
                            DBGPRINT(RT_DEBUG_ERROR, ("PID_REQUIRE_ACK and TxSuccess wcid = %d, i = %d\n", wcid, i));
                            P2pClientStateUpdate(pAd, pPort);
                            // disable periodic ACK check
                            pPort->P2PCfg.bP2pAckReq = FALSE;
                            GotAck = TRUE;
                        }
        
                        DBGPRINT(RT_DEBUG_ERROR, (">>>TX_STA_FIFO<<< word = 0x%x , TxSuccess[%d] TxTxAggre[%d] TxAckRequired[%d] SuccessRate[%d]\n", 
                                StaFifo.word, 
                                StaFifo.field.TxSuccess, 
                                StaFifo.field.TxAggre,
                                StaFifo.field.TxAckRequired,
                                StaFifo.field.SuccessRate));

                        if (GotAck == TRUE)
                        {
                            break;
                        }
                        
                        i++;
                        // ASIC store 16 stack
                    }while (i < 16);

                    if (i == 16)
                    {
                        DBGPRINT(RT_DEBUG_ERROR, ("Can't find Ack of P2P public action frame, handle this Ack by periodic check OR received P2P public action frame\n"));
                    }
                }                       
                DBGPRINT(RT_DEBUG_TRACE, ("<=== MT_CMD_P2P_READ_TXFIFO\n"));                        
                    break;

#if(COMPILE_WIN8_ABOVE(CURRENT_OS_NDIS_VER))  
            case MT_CMD_P2pMs_SEND_COMPLETE_IND:
                {                           
                    if(pPort->P2PCfg.pSendOutBufferedFrame != NULL)
                    {

                        //MlmeP2pMsPublicActionFrameGotAck(pAd);
#ifdef DBG                              
                        P2pMsIndicateFrameSendComplete(pAd, pPort, 
                                                        (PUCHAR) pPort->P2PCfg.pSendOutBufferedFrame, pPort->P2PCfg.SendOutBufferedSize, NDIS_STATUS_SUCCESS, __LINE__);
#else
                        P2pMsIndicateFrameSendComplete(pAd, pPort, 
                                                        (PUCHAR) pPort->P2PCfg.pSendOutBufferedFrame, pPort->P2PCfg.SendOutBufferedSize, NDIS_STATUS_SUCCESS);
#endif
                        PlatformFreeMemory(pPort->P2PCfg.pSendOutBufferedFrame, pPort->P2PCfg.SendOutBufferedSize);
                        pPort->P2PCfg.pSendOutBufferedFrame = NULL;
                        pPort->P2PCfg.SendOutBufferedSize = 0;
                                
                    }
                    else
                    {
                        DBGPRINT(RT_DEBUG_TRACE,("%s::Handle MT_CMD_P2pMs_SEND_COMPLETE_IND, already handled\n", __FUNCTION__));
                    }
                }
                    break;

            case MT_CMD_P2pMs_PUBLIC_ACTION_FRAME_RCV:
                {
                    DBGPRINT(RT_DEBUG_TRACE,("%s::Handle MT_CMD_P2pMs_PUBLIC_ACTION_FRAME_RCV\n", __FUNCTION__));
                    P2pMsIndicatePublicActionFrameRecv(pAd, pPort, (PUCHAR)(buffer), bufflength);
                }
                break;
                
            // P2pMs handle new state machine
            case MT_CMD_P2pMs_PUBLIC_ACTION_FRAME_EXEC:
                {
                    MlmeP2pMsPublicActionFrameStateMachineExec(pAd, pPort, buffer, bufflength);
                }
                break;
                
            case MT_CMD_P2pMs_PUBLIC_ACTION_FRAME_SEND_EXEC:
                {
                    PP2P_PUBLIC_FRAME pFrame = (PP2P_PUBLIC_FRAME)buffer;

                    // 1. Set current state
                    pPort->P2PCfg.CurrState = P2P_SEND_STATE;
                    
                    // 2. Set Signal
                    pPort->P2PCfg.Signal = pFrame->Subtype;

                    MlmeP2pMsPublicActionFrameStateMachineExec(pAd, pPort, buffer, bufflength);
                }
                break;

            case MT_CMD_P2pMs_PUBLIC_ACTION_FRAME_GOT_ACK_EXEC:
                {
                    PP2P_PUBLIC_FRAME pFrame = (PP2P_PUBLIC_FRAME)buffer;
                    
                    pPort->P2PCfg.Signal = pFrame->Subtype;
                    MlmeP2pMsPublicActionFrameStateMachineExec(pAd, pPort, buffer, bufflength);                            
                }
                break;

            case MT_CMD_P2pMs_PUBLIC_ACTION_FRAME_TIMEOUT_EXEC:
                {
                    P2pMsPublicActionFrameWaitTimerTimeoutExec(pPort);
                }
                break;
#endif /* NDIS630_MINIPORT*/
            
    }

    return TRUE;
}

VOID
P2PCmdThread(
    IN PMP_ADAPTER pAd
    )
{
    PCmdQElmt   cmdqelmt;
    PUCHAR      pData;
    ULONG       DataLen = 0;
    PMP_PORT    pPort = NULL;
    
    FUNC_ENTER;
    
    KeSetPriorityThread(KeGetCurrentThread(), LOW_REALTIME_PRIORITY);

    ASSERT(KeGetCurrentIrql() == PASSIVE_LEVEL);        

    while (pAd->TerminateThreads != TRUE)
    {
        PlatformWaitEventTriggeredAndReset(&(GET_THREAD_EVENT(&pAd->pP2pCtrll->hP2PThread)),0);   
        DBGPRINT(RT_DEBUG_TRACE, ("P2PCmdThread Thread Triggered\n"));
        
        while (pAd->pP2pCtrll->P2PCmdQ.size > 0)
        {
            NdisAcquireSpinLock(&pAd->pP2pCtrll->P2PCmdQLock);
            RTUSBDequeueCmd(&pAd->pP2pCtrll->P2PCmdQ, &cmdqelmt);
            NdisReleaseSpinLock(&pAd->pP2pCtrll->P2PCmdQLock);      
            
            if (cmdqelmt == NULL)
                break;       

                DBGPRINT(RT_DEBUG_TRACE, ("%s   cmdqelmt->PortNum = %d", __FUNCTION__, cmdqelmt->PortNum));
                pData = cmdqelmt->buffer;
                DataLen = cmdqelmt->bufferlength;
                pPort = pAd->PortList[cmdqelmt->PortNum];
                
                if ((!MT_TEST_FLAG(pAd, fRTMP_ADAPTER_NIC_NOT_EXIST))    && 
                    (!MT_TEST_FLAG(pAd, fRTMP_ADAPTER_HALT_IN_PROGRESS)) &&
                    (!MT_TEST_FLAG(pAd, fRTMP_ADAPTER_SLEEP_IN_PROGRESS)))
                {
                    P2PCmdHandler(pAd, pPort, cmdqelmt->command, pData, DataLen);
                }

            MtReturnInternalCmdBuffer(pAd, cmdqelmt);
        }  //while (pAd->pP2pCtrll->P2PCmdQ.size > 0)
    }   
    
    FUNC_LEAVE;
}
