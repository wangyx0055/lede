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
*/
#include    "MtConfig.h"

NDIS_STATUS N6UsbSetPowerbyPort(
    IN PMP_PORT               pPort ,
    IN NDIS_DEVICE_POWER_STATE  NewDeviceState)
{
    NDIS_STATUS                 ndisStatus = NDIS_STATUS_SUCCESS;
    MLME_DISASSOC_REQ_STRUCT    DisReq;
    PMLME_QUEUE_ELEM            pMsgElem = NULL;
    BOOLEAN                     Cancelled;
    ULONG                       Index, num, MACVersion = 0;
    UCHAR                       acidx;
    PRTMP_REORDERBUF            pReorderBuf;    
//  WPDMA_GLO_CFG_STRUC         GloCfg;
    ULONG                       /*Value, */i, j, BssIndex;
//**    H2M_MAILBOX_STRUC           H2MMailbox;
    PMP_PORT                  ApPort;
    PMP_ADAPTER               pAd = pPort->pAd;
    PTX_CONTEXT                 pNullContext   = &pAd->pHifCfg->NullContext;
    PTX_CONTEXT                 pPsPollContext = &pAd->pHifCfg->PsPollContext;
    PTX_CONTEXT                 pRTSContext    = &pAd->pHifCfg->RTSContext;
    PTX_CONTEXT                 pPktCmdContext = &pAd->pHifCfg->PktCmdContext;
    PTX_CONTEXT                 pFwPktContext = &pAd->pHifCfg->FwPktContext;
    //UCHAR                     BBPValue;
    ULONG                       TxPinCfg = 0x00050F0F;
    BOOLEAN                     bRoamingAfterResume = FALSE;
//  ULONG                       SysCtlValue,tmpSysCtlValue; 
    PIRP pIrp = NULL;
//**    UCHAR    RfValue;
    BOOLEAN bDone;
    PMP_PORT                  pTmpPort = NULL;

    do
    {
        //
        // Remember the device power state we are transitioning to.
        //
        pAd->NextDevicePowerState = NewDeviceState;

        DBGPRINT(RT_DEBUG_TRACE, ("%s: Enter D3(%d) or D2(%d)\n", __FUNCTION__, (NewDeviceState == NdisDeviceStateD3), (NewDeviceState == NdisDeviceStateD2)));


        if (NewDeviceState != NdisDeviceStateD0)
        {           

            if (NewDeviceState == NdisDeviceStateD2)
            {
                pAd->DevicePowerState = NewDeviceState;
                DBGPRINT(RT_DEBUG_TRACE, ("%s: Enter D2 state. Do nothing!!", __FUNCTION__));
                break;
            }

            DBGPRINT(RT_DEBUG_TRACE, ("%s: The OS is entering standby/hibernation.\n", __FUNCTION__));

            DBGPRINT(RT_DEBUG_TRACE,("Radio status = %d !!\n", MT_TEST_FLAG(pAd, fRTMP_ADAPTER_RADIO_OFF)));

#if(COMPILE_WIN8_ABOVE(CURRENT_OS_NDIS_VER))  
            if(NDIS_WIN8_ABOVE(pAd))
            {
                if ((pAd->StaCfg.NLOEntry.NLOEnable == TRUE)  && (pAd->StaCfg.NLOEntry.uNumOfEntries != 0))
                {
                    pAd->StaCfg.NLOEntry.bNLOAfterResume = TRUE;

                    DBGPRINT(RT_DEBUG_TRACE, ("%s: Perform NLO after system resume\n", __FUNCTION__));
                }

                    // For supporting "No Pause On Suspend", wait for all pending ndis tx packets until they are all sent.
                if (pAd->Counter.MTKCounters.PendingNdisPacketCount > 0)
                    {
                        DBGPRINT(RT_DEBUG_TRACE, ("Wait for Tx Queue empty ... queue data count = %d\n",pAd->Counter.MTKCounters.PendingNdisPacketCount));
                        Ndis6CommonRejectPendingPackets(pAd);
                    }
                }
#endif

            pAd->DevicePowerState = NewDeviceState;

            //
            // We are going to sleep
            // we should send a disassoc frame to our AP.
            //
            if (!MT_TEST_FLAG(pAd, fRTMP_ADAPTER_NIC_NOT_EXIST))
            {
                bRoamingAfterResume = FALSE;
                for(i = 0 ; i < RTMP_MAX_NUMBER_OF_PORT/*pAd->NumberOfPorts*/; i++)
                {
                    pTmpPort = pAd->PortList[i];
                    if ((pTmpPort == NULL) || (pTmpPort == FALSE))
                    {
                        continue;
                    }
                    
                    if (OPSTATUS_TEST_FLAG(pTmpPort, fOP_STATUS_MEDIA_STATE_CONNECTED))
                    {
                        if (INFRA_ON(pTmpPort)) 
                        {
                            /****************************************************************/
                            //for NdisTest 
                            //In Roaming_cmn test , when current AP is disappear , station have to into roaming
                            //state.
                            /****************************************************************/
                            COPY_MAC_ADDR(DisReq.Addr, pTmpPort->PortCfg.Bssid);
                            DisReq.Reason =  REASON_DISASSOC_STA_LEAVING;

                            PlatformAllocateMemory(pAd, &pMsgElem, sizeof(MLME_QUEUE_ELEM));
                            if (pMsgElem != NULL)
                            {
                                pMsgElem->Machine = ASSOC_STATE_MACHINE;
                                pMsgElem->MsgType = MT2_MLME_DISASSOC_REQ;
                                pMsgElem->MsgLen = sizeof(MLME_DISASSOC_REQ_STRUCT);                    
                                pMsgElem->PortNum = pTmpPort->PortNumber;                   
                                PlatformMoveMemory(pMsgElem->Msg, &DisReq, sizeof(MLME_DISASSOC_REQ_STRUCT));
                            
                                pTmpPort->Mlme.CntlMachine.CurrState = CNTL_WAIT_OID_DISASSOC;      
                                MlmeDisassocReqActionforPowerSave(pAd, pMsgElem);                   

                                PlatformFreeMemory(pMsgElem, sizeof(MLME_QUEUE_ELEM));
                            }
                            else
                            {
                                DBGPRINT(RT_DEBUG_ERROR, ("%s: Allocate memory failed for MT2_MLME_DISASSOC_REQ\n", __FUNCTION__));
                            }
                            
                            //
                            // Indicate disconnect status
                            //
                            NdisCommonGenericDelay(1000);
                            
                            //Keep current bssid for roaming after waking up
                            BssIndex = BssTableSearch(pAd, pTmpPort, &pAd->ScanTab, pTmpPort->PortCfg.Bssid, pPort->Channel);
                            if (BssIndex != BSS_NOT_FOUND)
                            {
                                DBGPRINT(RT_DEBUG_TRACE, ("[080821]%02x %02x %02x %02x %02x %02x found...\n",
                                    pTmpPort->PortCfg.Bssid[0],
                                    pTmpPort->PortCfg.Bssid[1],
                                    pTmpPort->PortCfg.Bssid[2],
                                    pTmpPort->PortCfg.Bssid[3],
                                    pTmpPort->PortCfg.Bssid[4],
                                    pTmpPort->PortCfg.Bssid[5]
                                    ));

                                PlatformMoveMemory(&pTmpPort->CurrentBssEntry, &pAd->ScanTab.BssEntry[BssIndex], sizeof(BSS_ENTRY));
                                pTmpPort->CurrentBssEntry.LastBeaconRxTime = 0;
                                bRoamingAfterResume = TRUE;
                            }
                            //Remove keys, this avoid handshaking failure after hibernate
                            if ((pTmpPort->PortCfg.AuthMode >= Ralink802_11AuthModeWPA) && (pTmpPort->PortCfg.AuthMode != Ralink802_11AuthModeAutoSwitch))
                            {
                                for (j = 0; j < SHARE_KEY_NUM; j++)
                                {
                                    pTmpPort->SharedKey[BSS0][j].CipherAlg = CIPHER_NONE;
                                    pTmpPort->SharedKey[BSS0][j].KeyLen = 0;
                                }
                            }
                            
                            PlatformIndicateDisassociation(pAd,pTmpPort, DisReq.Addr, DOT11_DISASSOC_REASON_OS);
                            DBGPRINT(RT_DEBUG_TRACE, ("TONDIS:  N6UsbPnpSetPower leave D0, PlatformIndicateDisassociation [%02x:%02x:%02x:%02x:%02x:%02x]\n",
                                        DisReq.Addr[0], DisReq.Addr[1], DisReq.Addr[2], DisReq.Addr[3], DisReq.Addr[4], DisReq.Addr[5]));                           
                        }               
                    
                        if ((ADHOC_ON(pTmpPort) && (pTmpPort->PortCfg.AuthMode == Ralink802_11AuthModeWPA2PSK)) ||
                            IS_WEP_STATUS_ON(pTmpPort->PortCfg.WepStatus))
                        {
                            pPort->CommonCfg.bRestoreKey = TRUE;
                        }
                        MlmeCntLinkDown( pTmpPort,FALSE);
                        //clean flag fOP_STATUS_MEDIA_STATE_CONNECTED flag; therefore,  the sta can create adhoc again when it is back from hibernation
                        OPSTATUS_CLEAR_FLAG(pPort, fOP_STATUS_MEDIA_STATE_CONNECTED);
                    }
                }
                MT_SET_FLAG(pAd, fRTMP_ADAPTER_SLEEP_IN_PROGRESS);
            }
            
            if (P2P_ON(pPort))
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
                if ((pAd->PortList[pPort->P2PCfg.PortNumber]) 
                    && (pAd->PortList[pPort->P2PCfg.PortNumber]->bActive) 
                    && (pAd->PortList[pPort->P2PCfg.PortNumber]->PortType == EXTSTA_PORT)
                    && ((pAd->PortList[pPort->P2PCfg.PortNumber]->PortSubtype == PORTSUBTYPE_P2PClient) 
                    || (pAd->PortList[pPort->P2PCfg.PortNumber]->PortSubtype == PORTSUBTYPE_P2PGO)))
                {
                        pPort->P2PCfg.P2pEventQueue.bDeleteWirelessCon = TRUE;    
                    P2pDown(pAd, pAd->PortList[pPort->P2PCfg.PortNumber]);
                    pPort->P2PCfg.P2PConnectState = P2P_CONNECT_NOUSE;
                    // need disable p2p and resume UI will enable
                    pPort->P2PCfg.P2PDiscoProvState = P2P_DISABLE;
                }
            }

            //sync UI display (We should set back to STA only when doing S3/S4)                  
            if(pAd->OpMode == OPMODE_STA || pAd->OpMode == OPMODE_APCLIENT)
                pAd->OpMode = OPMODE_STA;

            if (pAd->OpMode == OPMODE_AP)
            {
                DBGPRINT(RT_DEBUG_TRACE,("[%s][LINE_%d] System exiting D0, stop Ralink-SoftAP ...\n",__FUNCTION__,__LINE__));
                APStop(pAd,pPort);
                pPort->SoftAP.ApCfg.bSoftAPReady = FALSE; // STA ==> Standby/hibernation
            }
            else // pPort->PortSubtype == PORTSUBTYPE_VwifiAP
            {
                for(i = 0 ; i < RTMP_MAX_NUMBER_OF_PORT/*pAd->NumberOfPorts*/; i++)
                {
                    if ((pAd->PortList[i] == NULL) || (pAd->PortList[i]->bActive == FALSE))
                    {
                        continue;
                    }

                    ApPort = pAd->PortList[i];
                    if(ApPort && ApPort->PortType == EXTAP_PORT && pPort->SoftAP.bAPStart == TRUE)
                    {
                        DBGPRINT(RT_DEBUG_TRACE,("[%s][LINE_%d] System exiting D0, stop VWiFi-SoftAP ...\n",__FUNCTION__,__LINE__));
                        APStop(pAd,ApPort);
                        pPort->SoftAP.ApCfg.bSoftAPReady = FALSE;
                        break;
                    }
                }
            }
            
            
//**            MtAsicDisableSync(pAd);
//**            LedCtrlSetLed(pAd, LED_HALT);  
            //
            // Force signal strength Led to be turned off, firmware is not done it.
            //
//**            LedCtrlSetSignalLed(pAd, -100);

            // Complete all pending packets.
            //
            Ndis6CommonRejectPendingPackets(pAd);
            NdisCommonUsbCleanUpMLMEWaitQueue(pAd);
            NdisCommonUsbCleanUpMLMEBulkOutQueue(pAd);
            //
            // Cancel pending timers
            //
            PlatformCancelTimer(&pPort->Mlme.AssocTimer,       &Cancelled);
            PlatformCancelTimer(&pPort->Mlme.ReassocTimer,     &Cancelled);
            PlatformCancelTimer(&pPort->Mlme.DisassocTimer,    &Cancelled);
            PlatformCancelTimer(&pPort->Mlme.AuthTimer,        &Cancelled);
            PlatformCancelTimer(&pPort->Mlme.BeaconTimer,      &Cancelled);
            PlatformCancelTimer(&pPort->Mlme.ScanTimer,        &Cancelled);
            PlatformCancelTimer(&pPort->Mlme.RxAntEvalTimer,      &Cancelled); 
            PlatformCancelTimer(&pPort->Mlme.ChannelTimer, &Cancelled);
            PlatformCancelTimer(&pPort->Mlme.PeriodicTimer,       &Cancelled);

            //
            // Do this MlmeRadioOff will cause scan fail after resume.
            //
            //          MlmeRadioOff(pAd);
            //
            
            
            for(i = 0 ; i < RTMP_MAX_NUMBER_OF_PORT/*pAd->NumberOfPorts*/; i++)
            {               
                pTmpPort = pAd->PortList[i];
                if (pTmpPort == NULL)
                    continue;               
                PlatformCancelTimer(&pTmpPort->Mlme.MlmeCntLinkUpTimer,        &Cancelled);
                PlatformCancelTimer(&pTmpPort->Mlme.MlmeCntLinkDownTimer,       &Cancelled);
                PlatformCancelTimer(&pTmpPort->Mlme.RxAntEvalTimer,     &Cancelled);
            }
            
            NdisCommonGenericDelay(5000);
            
            // Clear timer for Wsc
            WscStop(pAd, pPort);

            for (Index = 0; Index < MAX_NUM_OF_DLS_ENTRY; Index++)
            {
                PlatformCancelTimer(&pPort->StaCfg.DLSEntry[Index].Timer, &Cancelled);
            }   


//**
#if 0
            // Disable MAC Tx/Rx
            RTUSBReadMACRegister(pAd, MAC_SYS_CTRL, &Value);
            Value &= (0xfffffff3);
            if (pAd->bBTCoexistence)
            {
                if(IS_ENABLE_BT_WIFI_ACTIVE_PULL_LOW_BY_FORCE(pAd))
                {
                    Value |= 0x0240;            
                }
                else
                {
                    Value |= 0x1240;
                }
            }
            DBGPRINT(RT_DEBUG_TRACE,("%s: LINE_%d, set MAC 0x1004 = 0x%x\n",__FUNCTION__,__LINE__,Value));
            RTUSBWriteMACRegister(pAd, MAC_SYS_CTRL, Value);
            //AsicSendCommanToMcu(pAd, SLEEP_MCU_CMD, 0xff, 0xff, 0x02);

            NdisCommonGenericDelay(5000);

            {
                i =0;
                do
                {
                    RTUSBReadMACRegister(pAd, H2M_MAILBOX_CSR, &H2MMailbox.word);
                    if (H2MMailbox.field.Owner == 0)
                        break;
    
                    //NdisCommonGenericDelay(1000);
                    Delay_us(1000);
                    DBGPRINT(RT_DEBUG_INFO, ("AsicSendCommanToMcu::Mail box is busy\n"));
                } while(i++ < 100);
    
                if (i >= 100)
                {
                    DBGPRINT_ERR(("N6UsbPnpSetPower::H2M_MAILBOX still hold by MCU. command fail\n"));
                }
            }
#endif
            //      
            // We are going to sleep. Unless we are connecting to a hidden network, we wont
            // add the SSID to the probe request
            //
            //Temporarily remove for WLK 1.1

            pPort->CommonCfg.bSSIDInProbeRequest = FALSE;

            if(!MT_TEST_FLAG(pAd, fRTMP_ADAPTER_RADIO_OFF))
            {
                // Disable Rx
                SendDisableRxCMD(pAd);

                // Polling Rx Empty
                if (1)
                {
                    U3DMA_WLCFG         U3DMAWLCFG;
                    int i = 0;

                    do
                    {
                        HW_IO_READ32(pAd, UDMA_WLCFG_0, (UINT32 *)&U3DMAWLCFG.word);
                        DBGPRINT(RT_DEBUG_TRACE,("%d Wait Tx/Rx to be idle, Rx = %d, Tx = %d\n", i, U3DMAWLCFG.field.WL_RX_BUSY, U3DMAWLCFG.field.WL_TX_BUSY));

                        i++;
                        if (i == 1000)
                            break;
                    } while ((U3DMAWLCFG.field.WL_RX_BUSY == 1) || (U3DMAWLCFG.field.WL_TX_BUSY == 1));
                }

                SendRadioOnOffCMD(pAd, PWR_RadioOff);
                MT_SET_FLAG(pAd, fRTMP_ADAPTER_RADIO_OFF);

                // cancel all pending irp
                N6UsbXmitStop(pAd);
                N6UsbRecvStop(pAd);
            }

            while (TRUE) 
            {
                bDone = TRUE;
        
                if (pAd->pHifCfg->PendingRx > 0)
                {
                    DBGPRINT(RT_DEBUG_TRACE, ("PendingRx = %d\n", pAd->pHifCfg->PendingRx));
                    bDone = FALSE;
                    
                    //Clear Pending Request
                    for(i = 0 ; i < RX_RING_SIZE ; i++)
                    {
                        PRX_CONTEXT  pRxContext = &pAd->pHifCfg->RxContext[i];
                        if (pRxContext->IRPPending == TRUE)
                        {
                            WdfRequestCancelSentRequest(pRxContext->Request);
                        }
                    }
                }
        
                if (bDone)
                {
                    break;
                }
                
                NdisCommonGenericDelay(1000);
            }

            pPort->CommonCfg.bInitializationState = FALSE;
            
            if(bRoamingAfterResume)
                pAd->PortList[PORT_0]->bRoamingAfterResume = TRUE;
            
        }
        else if (NewDeviceState == NdisDeviceStateD0)
        {
            UCHAR           index;

            UCHAR           BulkInIndex = 0;

            TOP_MISC_CONTROLS2  TopMiscControls2;

            NDIS_DEVICE_POWER_STATE previousDevicePowerState = pAd->DevicePowerState;

            DBGPRINT(RT_DEBUG_TRACE, ("%s: The OS is entering full-power mode.\n", __FUNCTION__));

            if (previousDevicePowerState == NdisDeviceStateD2)
            {
                if (!MT_TEST_FLAG(pAd, fRTMP_ADAPTER_IDLE_RADIO_OFF) && !MT_TEST_FLAG(pAd, fRTMP_ADAPTER_RADIO_OFF))
                {
                    for (BulkInIndex = 0; BulkInIndex < pAd->pNicCfg->NumOfBulkInIRP; BulkInIndex++ )
                    {
                        N6USBBulkReceive(pAd);
                    }

                    N6USBBulkReceiveCMD(pAd);
                }

                pAd->DevicePowerState = NewDeviceState;
                DBGPRINT(RT_DEBUG_TRACE, ("%s: Leave D2 state. Do nothing!!", __FUNCTION__));
                break;
            }

            pAd->TrackInfo.CountDownFromS3S4 = 4;
            pAd->bWakeupFromS3S4 = TRUE;
           
            pAd->DevicePowerState = NdisDeviceStateD0;

            InitializeQueueHeader(&pAd->pRxCfg->IndicateRxPktsQueue);
            for (index = 0; index < NUM_OF_LOCAL_RX_PKT_BUFFER; index++)
            {
                PMP_LOCAL_RX_PKTS pIndicateRxPkts = &pAd->pRxCfg->IndicateRxPkts[index];
                InsertTailQueue(&pAd->pRxCfg->IndicateRxPktsQueue, pIndicateRxPkts);
            }

            pAd->pHifCfg->PendingRx = 0;
            pAd->pHifCfg->NextRxBulkInReadIndex = 0; // Next Rx Read index
            pAd->pHifCfg->NextRxBulkInIndex      = RX_RING_SIZE_END_INDEX-2; // Rx Bulk pointer
            pAd->pHifCfg->NextRxBulkInPosition = 0;

            pAd->pHifCfg->NextMLMEIndex         = 0;
            pAd->pHifCfg->PushMgmtIndex         = 0;
            pAd->pHifCfg->PopMgmtIndex          = 0;
            pAd->pHifCfg->MgmtQueueSize         = 0;

            pAd->pHifCfg->PrioRingFirstIndex    = 0;
            pAd->pHifCfg->PrioRingTxCnt         = 0;
            //clear this flag to prevent scan request ignore by driver
            pAd->pHifCfg->BulkLastOneSecCount  = 0;


            if (pPort->bRoamingAfterResume)
            {
                pPort->CurrentBssEntry.LastBeaconRxTime = 0;
                pPort->Channel = pPort->CurrentBssEntry.Channel;
                DBGPRINT(RT_DEBUG_TRACE, ("%s:bRoamingAfterResume true\n", __FUNCTION__));
            }
            //
            // Reset TX:
            //
            for (Index = 0; Index < 4; Index++)
            {
                pAd->pHifCfg->NextBulkOutIndex[Index]    = 0;        // Next Local tx ring pointer waiting for buck out
                pAd->pHifCfg->BulkOutPending[Index]      = FALSE;    // Buck Out control flag    
            }

            // TX_RING_SIZE
            for (acidx = 0; acidx < 4; acidx++)
            {
                PHT_TX_CONTEXT  pTxContext = &pAd->pHifCfg->TxContext[acidx];

                //
                // Reset All variables.
                // 
                pTxContext->IRPPending[0] = FALSE;
                pTxContext->IRPPending[1] = FALSE;
                pTxContext->NextBulkOutPosition = 0;
                pTxContext->ENextBulkOutPosition = 0;
                pTxContext->CurWritePosition = 0;
                pTxContext->BulkOutSize = 0;
                pTxContext->BulkOutPipeId = acidx;          
                pTxContext->bCopySavePad = FALSE;   
            }

            //¡@PRIO_RING_SIZE
            for (Index = 0; Index < PRIO_RING_SIZE; Index++)
            {
                PTX_CONTEXT pMLMEContext = &pAd->pHifCfg->MLMEContext[Index];

                pMLMEContext->InUse = FALSE;
                pMLMEContext->IRPPending = FALSE;               
                pMLMEContext->ReadyToBulkOut = FALSE;
            }
        
            for (Index = 0; Index < RTMP_MAX_NUMBER_OF_PORT; Index ++)
            {
                pAd->pRxCfg->pLastWaitToIndicateRxPkts[Index] = NULL;
                pAd->pRxCfg->pWaitToIndicateRxPkts[Index] = NULL;
                pAd->pRxCfg->nWaitToIndicateRxPktsNum[Index] = 0;
            }
        
            // NullContext
            pNullContext->InUse = FALSE;
            pNullContext->IRPPending = FALSE;
            pNullContext->ReadyToBulkOut = FALSE;


            // RTSContext
            pRTSContext->InUse = FALSE;
            pRTSContext->IRPPending = FALSE;
            pRTSContext->ReadyToBulkOut = FALSE;

            // PsPollContext
            pPsPollContext->InUse = FALSE;
            pPsPollContext->IRPPending = FALSE;
            pPsPollContext->bAggregatible = FALSE;
            pPsPollContext->LastOne = TRUE;
            pPsPollContext->ReadyToBulkOut = FALSE;

            // PktCmdContext
            pFwPktContext->InUse = FALSE;
            pFwPktContext->IRPPending = FALSE;
            pFwPktContext->ReadyToBulkOut = FALSE;            

            // PktCmdContext
            NdisAcquireSpinLock(&pAd->pHifCfg->PktCmdLock);
            pPktCmdContext->InUse = FALSE;
            NdisReleaseSpinLock(&pAd->pHifCfg->PktCmdLock);
            pPktCmdContext->IRPPending = FALSE;
            pPktCmdContext->ReadyToBulkOut = FALSE;            

            //=======================================================
            // Reset RX
            //
                        
            for (Index = 0; Index < RX_RING_SIZE; Index++)
            {
                PRX_CONTEXT  pRxContext = &(pAd->pHifCfg->RxContext[Index]);

                pRxContext->InUse       = FALSE;
                pRxContext->IRPPending  = FALSE;
                pRxContext->Readable    = FALSE;            
                pRxContext->ReadPosOffset = 0;      
            }


            for (num = 0; num < Max_BARECI_SESSION; num++)
            {
                pAd->pRxCfg->LocalRxReorderBuf[num].InUse = FALSE;
                for (Index = 0; Index < Max_RX_REORDERBUF; Index++)
                {
                    pReorderBuf = &pAd->pRxCfg->LocalRxReorderBuf[num].MAP_RXBuf[Index];
                    pReorderBuf->IsFull = FALSE;
                }
            }
    
            MlmeQueueInit(&pAd->Mlme.Queue);

            //Clear this flag  to prevent this flag to be set before wake up
            MT_CLEAR_FLAG(pAd, fRTMP_ADAPTER_MLME_RESET_IN_PROGRESS);
            MT_CLEAR_FLAG(pAd, fRTMP_ADAPTER_NIC_NOT_EXIST);
            MT_CLEAR_FLAG(pAd, fRTMP_ADAPTER_BULKIN_RESET);
            MT_CLEAR_FLAG(pAd, fRTMP_ADAPTER_BULKOUT_RESET);
            MT_CLEAR_FLAG(pAd, fRTMP_ADAPTER_SLEEP_IN_PROGRESS);
            //MT_CLEAR_FLAG(pAd, fRTMP_ADAPTER_RADIO_OFF);
            MT_CLEAR_FLAG(pAd, fRTMP_ADAPTER_IDLE_RADIO_OFF);

//**
#if 0
            //
            // Make sure MAC gets ready.
            //
            Index = 0;
            do 
            {
                GET_ASIC_VERSION_ID(pAd);
                MACVersion = pAd->HwCfg.MACVersion;

                if ((MACVersion != 0x00) && (MACVersion != 0xFFFFFFFF))
                    break;

                NdisCommonGenericDelay(10);             
            } while (Index++ < 10);

            DBGPRINT(RT_DEBUG_ERROR, ("N6UsbPnpSetPower: State = NdisDeviceStateD0, MACVersion=0x%08x\n", MACVersion));

            if ((MACVersion == 0x00) || (MACVersion == 0xFFFFFFFF))
            {
                NdisCommonGenericDelay(100000);             
                RTUSB_ResetDevice(pAd);
            }
#endif

            N6UsbXmitStart(pAd);

            N6UsbRecvStart(pAd);
            for (BulkInIndex = 0; BulkInIndex < pAd->pNicCfg->NumOfBulkInIRP; BulkInIndex++ )
            {
                N6USBBulkReceive(pAd);
            }

            N6USBBulkReceiveCMD(pAd);
#if 1
            HW_IO_READ32(pAd, TOP_MISC2, &TopMiscControls2.word);
            if (TopMiscControls2.field.FwIsRunning == 1)
            //if (previousDevicePowerState == NdisDeviceStateD2)
            {
                DBGPRINT(RT_DEBUG_TRACE,("Radio status = %d !!\n", MT_TEST_FLAG(pAd, fRTMP_ADAPTER_RADIO_OFF)));
                if (MT_TEST_FLAG(pAd, fRTMP_ADAPTER_RADIO_OFF))
                {
                    MT_CLEAR_FLAG(pAd, fRTMP_ADAPTER_RADIO_OFF);

                    SendRadioOnOffCMD(pAd, PWR_RadioOn);
                }

            }
            else
#endif
            {
#if 1
                if (MT_TEST_FLAG(pAd, fRTMP_ADAPTER_RADIO_OFF))
                {
                    MT_CLEAR_FLAG(pAd, fRTMP_ADAPTER_RADIO_OFF);
                }
#endif
                DBGPRINT(RT_DEBUG_TRACE,("Start to load firmware!!\n"));
                ndisStatus = LoadAndesFW(pAd);
                
                if (ndisStatus != NDIS_STATUS_SUCCESS)  // reload on time.
                {
                    DBGPRINT_RAW(RT_DEBUG_ERROR, ("Load firmware failed. \n")); 
                    break;
                }           
            }

            NICResetFromError(pAd);
            
            //
            // Driver will not unload but may cut off device's power in NB.
            // Need to reolad 8051 firmware and MAC/BBP/RF default settings.
            //
//**
#if 0           
            // Load 8051 firmware;  
            ndisStatus = NICLoadFirmware(pAd);
            if (ndisStatus != NDIS_STATUS_SUCCESS)
            {
                //
                // Flush BSS table
                //
                DBGPRINT(RT_DEBUG_TRACE, ("FLUSH_BSS_LIST if FW load failed: device is gone\n"));
                pAd->pNicCfg->bIsClearScanTab = TRUE;
                BssTableInit(&pAd->ScanTab); 
                ResetRepositBssTable(pAd);

                DBGPRINT_ERR(("NICLoadFirmware failed, ndisStatus[=0x%08x]\n", ndisStatus));
                break;
            }
#endif

            if (MT_TEST_FLAG(pAd, fRTMP_ADAPTER_NIC_NOT_EXIST))
            {
                DBGPRINT(RT_DEBUG_ERROR, ("N6UsbPnpSetPower: Device not exist\n"));
            }
            
            //
            // Force to clear some flags. Like fRTMP_ADAPTER_BULKOUT_RESET, we will set this flag
            // then put it into queue for Cmdthread. But Cmdthread will not clear it when driver handling S1~S4 level. 
            // The flag will block N6USBKickBulkOut routine after S1~S4.
            //
            //MT_CLEAR_FLAG(pAd, fRTMP_ADAPTER_BULKIN_RESET);   
            //MT_CLEAR_FLAG(pAd, fRTMP_ADAPTER_BULKOUT_RESET);
            
            // Set mlme periodic timer
            pPort->Mlme.CntlMachine.CurrState = CNTL_IDLE;
            pAd->Mlme.OneSecPeriodicRound = 0;
            PlatformSetTimer(pPort, &pPort->Mlme.PeriodicTimer, MLME_TASK_EXEC_INTV);

            //resume ADHOC beacon
            if( ADHOC_ON(pPort))
            {
                MtAsicSetBssid(pAd, pPort->PortCfg.Bssid, (UINT8)pPort->PortNumber);
                MgntPktConstructIBSSBeaconFrame(pAd,pPort);
                MtAsicEnableIbssSync(pAd);
            }
        
            // init the scan time, let driver can scan from S3/S4
            pAd->StaCfg.LastScanTime = 0;

            pPort->CommonCfg.bInitializationState = TRUE;

//**
#if 0           
            XmitSendNullFrameForWake(pAd, pPort,pPort->CommonCfg.TxRate, FALSE);
            if (pAd->StaCfg.bRadio)
            {
                int i = 0;
                MgntPktSendProbeRequest(pPort);

#if(COMPILE_WIN8_ABOVE(CURRENT_OS_NDIS_VER))  
                while(i++ < 10 && (FALSE == pAd->StaCfg.NLOEntry.bNLOMlmeSyncPeerBeaconFound)) //while loop for thread context switch.
                {
                    NdisCommonGenericDelay(10); //sleep a beacon period (100ms) to wait if peer beacon is presented. 
                }

                //Ndistest request driver have to complete scan within 5 scan after resum from s3/s4
                //NLO: if peer beacon from lastest AP were not found,
                //issue a full scan to reflash BSSID list.
                if(pPort->bRoamingAfterResume == FALSE ||
                ((pAd->StaCfg.NLOEntry.NLOEnable == TRUE) && (pAd->StaCfg.NLOEntry.bNLOAfterResume == TRUE) && (pAd->StaCfg.NLOEntry.bNLOMlmeSyncPeerBeaconFound == FALSE)))
                {
                    if(pAd->StaCfg.NLOEntry.NLOEnable == TRUE)
                        DBGPRINT(RT_DEBUG_TRACE, ("NLO: Full Scan since NLO AP no longer exist\n"));
                    MlmeAutoScan(pAd);
                }

                if ((TRUE == pAd->StaCfg.NLOEntry.NLOEnable) && (TRUE == pAd->StaCfg.NLOEntry.bNLOMlmeSyncPeerBeaconFound))
                {
                    i = 0;
                    DBGPRINT(RT_DEBUG_TRACE, ("NLOEnable: reconnect last SSID\n"));
                    MlmeAutoReconnectLastSSID(pAd, pPort);

                    do
                    {
                        NdisCommonGenericDelay(10); // make contect switch occur til assoc rsp was received before wake up complete.
                    }while((i++ < 100) && (FALSE == pAd->StaCfg.NLOEntry.bNLOAssocRspToMe)); //i to protect forever loop.

                }
                else if ((pAd->StaCfg.bAutoReconnect == TRUE) && (pAd->LogoTestCfg.OnTestingWHQL == TRUE) && (pAd->StaCfg.NLOEntry.uNumOfEntries == 0) &&
                    (MlmeValidateSSID(pAd->MlmeAux.AutoReconnectSsid, pAd->MlmeAux.AutoReconnectSsidLen) == TRUE))
                {
                    if (pPort->bRoamingAfterResume && pPort->CurrentBssEntry.LastBeaconRxTime != 0)
                    {
                        DBGPRINT(RT_DEBUG_TRACE, ("For WHCK . .. MlmeAutoReconnectLastSSID...\n")); 
                        PlatformMoveMemory(&pAd->ScanTab.BssEntry[0], &pPort->CurrentBssEntry, sizeof(BSS_ENTRY));
                        // If ScanTab is empty, add one for CurrentBssEntry
                        if (pAd->ScanTab.BssNr == 0)
                            pAd->ScanTab.BssNr += 1;

                        MlmeAutoReconnectLastSSID(pAd, pPort);
                    }
                }               
#else
                //Ndistest request driver have to complete scan within 5 scan after resum from s3/s4
                if (MlmeValidateSSID(pAd->MlmeAux.AutoReconnectSsid, pAd->MlmeAux.AutoReconnectSsidLen) == FALSE)
                {
                    MlmeAutoScan(pAd);
                }
#endif
            }

#endif
            
#if(COMPILE_WIN8_ABOVE(CURRENT_OS_NDIS_VER))  
            //KeSetCoalescableTimer(&pPort->Mlme.PeriodicTimer, dueTime, MLME_TASK_EXEC_INTV, 10, &pAd->Mlme.MlmeDpc);
#endif
        }

        DBGPRINT(RT_DEBUG_TRACE, ("<== %s\n", __FUNCTION__));

    } while (FALSE);

    return ndisStatus;

}

