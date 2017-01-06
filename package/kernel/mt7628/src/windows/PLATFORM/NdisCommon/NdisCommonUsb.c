/*
 ***************************************************************************
 * MediaTek Inc.
 * 5F, No.5, Tai-Yuan 1st St., 
 * Chupei City, Hsinchu County 30265, Taiwan, R.O.C.
 *
 * (c) Copyright 2014-2016, MediaTek, Inc.
 *
 * All rights reserved. MediaTek's source code is an unpublished work and the
 * use of a copyright notice does not imply otherwise. This source code
 * contains confidential trade secret material of MediaTek. Any attemp
 * or participation in deciphering, decoding, reverse engineering or in any
 * way altering the source code is stricitly prohibited, unless the prior
 * written consent of MediaTek, Inc. is obtained.
 ***************************************************************************

    Module Name:
    NdisCommon_Usb.c

    Abstract:
    Miniport generic portion header file

    Revision History:
    Who         When          What
    --------    ----------    ----------------------------------------------
    */
    
#include    "MtConfig.h"


/*
    ========================================================================
    
    Routine Description:

    Arguments:

    Return Value:

    IRQL = 
    
    Note:
    
    ========================================================================
*/
VOID    NdisCommonUsbCleanUpMLMEWaitQueue(
    IN  PMP_ADAPTER   pAd)
{
    PMGMT_STRUC     pMgmt;

    DBGPRINT_RAW(RT_DEBUG_TRACE, ("--->CleanUpMLMEWaitQueue\n"));

    NdisAcquireSpinLock(&pAd->pHifCfg->MLMEWaitQueueLock);
    while (pAd->pHifCfg->PopMgmtIndex != pAd->pHifCfg->PushMgmtIndex)
    {
        pMgmt = (PMGMT_STRUC)&pAd->pHifCfg->MgmtRing[pAd->pHifCfg->PopMgmtIndex];
        MlmeFreeMemory(pAd, pMgmt->pBuffer);
        pMgmt->pBuffer = NULL;
        pMgmt->bNDPAnnouncement = FALSE;
        pMgmt->pMgmtPhyCfg = NULL;
        pMgmt->Valid = FALSE;
        InterlockedDecrement(&pAd->pHifCfg->MgmtQueueSize);

        pAd->pHifCfg->PopMgmtIndex++;
        if (pAd->pHifCfg->PopMgmtIndex >= MGMT_RING_SIZE)
        {
            pAd->pHifCfg->PopMgmtIndex = 0;
        }
    }
    NdisReleaseSpinLock(&pAd->pHifCfg->MLMEWaitQueueLock);

    DBGPRINT_RAW(RT_DEBUG_TRACE, ("<---CleanUpMLMEWaitQueue\n"));
}


/*
    ========================================================================
    
    Routine Description:

    Arguments:

    Return Value:

    IRQL = 
    
    Note:
    
    ========================================================================
*/
VOID    NdisCommonUsbCleanUpMLMEBulkOutQueue(
    IN  PMP_ADAPTER   pAd)
{
    DBGPRINT(RT_DEBUG_TRACE, ("--->CleanUpMLMEBulkOutQueue\n"));

    NdisAcquireSpinLock(&pAd->pHifCfg->MLMEQLock);
    while (pAd->pHifCfg->PrioRingTxCnt > 0)
    {
        pAd->pHifCfg->MLMEContext[pAd->pHifCfg->PrioRingFirstIndex].InUse = FALSE;
            
        pAd->pHifCfg->PrioRingFirstIndex++;
        if (pAd->pHifCfg->PrioRingFirstIndex >= PRIO_RING_SIZE)
        {
            pAd->pHifCfg->PrioRingFirstIndex = 0;
        }

        pAd->pHifCfg->PrioRingTxCnt--;
    }
    NdisReleaseSpinLock(&pAd->pHifCfg->MLMEQLock);

    DBGPRINT(RT_DEBUG_TRACE, ("<---CleanUpMLMEBulkOutQueue\n"));
}

NDIS_STATUS
NdisCommonUsbStartRx(
    IN PMP_ADAPTER pAd
    )
{
    NDIS_STATUS     ndisStatus = NDIS_STATUS_SUCCESS;
    
#if (COMPILE_VISTA_ABOVE(CURRENT_OS_NDIS_VER))
    ndisStatus = N6UsbRecvStart(pAd);
#endif

    return ndisStatus;
}

NDIS_STATUS
NdisCommonUsbStartTx(
    IN PMP_ADAPTER pAd
    )
{
    NDIS_STATUS     ndisStatus = NDIS_STATUS_SUCCESS;
    
#if (COMPILE_VISTA_ABOVE(CURRENT_OS_NDIS_VER))
    ndisStatus = N6UsbXmitStart(pAd);
#endif
    return ndisStatus;
}

VOID
NdisCommonUsbStopTx(
    IN PMP_ADAPTER pAd
    )
{
#if (COMPILE_VISTA_ABOVE(CURRENT_OS_NDIS_VER))
    N6UsbXmitStop(pAd);
#endif
}

VOID
NdisCommonUsbStopRx(
    IN PMP_ADAPTER pAd
    )
{
#if (COMPILE_VISTA_ABOVE(CURRENT_OS_NDIS_VER))
    N6UsbRecvStop(pAd);
#endif
}

VOID
NdisCommonUsbBulkRx(
    IN PMP_ADAPTER pAd
    )
{
#if (COMPILE_VISTA_ABOVE(CURRENT_OS_NDIS_VER))
    N6USBBulkReceive(pAd);
#endif
}

VOID
NdisCommonUsbBulkRxCmd(
    IN PMP_ADAPTER pAd
    )
{
#if (COMPILE_VISTA_ABOVE(CURRENT_OS_NDIS_VER))
    N6USBBulkReceiveCMD(pAd);
#endif
}

NDIS_STATUS
NdisCommonUsbInitTransmit(
    IN PMP_ADAPTER pAd
    )
{
    NDIS_STATUS     ndisStatus = NDIS_STATUS_SUCCESS;
    
#if (COMPILE_VISTA_ABOVE(CURRENT_OS_NDIS_VER))
    ndisStatus = N6UsbInitTransmit(pAd);
#endif
    return ndisStatus;
}

NDIS_STATUS
NdisCommonUsbInitRecv(
    IN PMP_ADAPTER pAd
    )
{
    NDIS_STATUS     ndisStatus = NDIS_STATUS_SUCCESS;
    
#if (COMPILE_VISTA_ABOVE(CURRENT_OS_NDIS_VER))
    ndisStatus = N6UsbInitRecv(pAd);
#endif
    return ndisStatus;
}

VOID
NdisCommonUsbFreeNicRecv(
    IN PMP_ADAPTER pAd
    )
{
#if (COMPILE_VISTA_ABOVE(CURRENT_OS_NDIS_VER))
    N6UsbFreeNICRecv(pAd);
#endif
}

VOID
NdisCommonUsbFreeNICTransmit(
    IN PMP_ADAPTER pAd
    )
{
#if (COMPILE_VISTA_ABOVE(CURRENT_OS_NDIS_VER))
    N6UsbFreeNICTransmit(pAd);
#endif
}

VOID    
NdisCommonUsbDeQueuePacket(
    IN  PMP_ADAPTER   pAd,    
    IN  UCHAR           BulkOutPipeId
    )
{
    PQUEUE_ENTRY    pEntry;
    PMT_XMIT_CTRL_UNIT    pXcu = NULL;
    UCHAR           FragmentRequired;
    NDIS_STATUS     ndisStatus;
    UCHAR           Count = 0;
    PQUEUE_HEADER   pQueue;
    UCHAR           QueIdx;
    UCHAR           OpMode = OPMODE_STA;
    UCHAR           PortNumber = 0;
    UCHAR           TxPacketNum;
    PMP_PORT    pPort = pAd->PortList[FXXK_PORT_0];
    BOOLEAN         bDoingRaTxAgg = FALSE;  

    // Reset is in progress, stop immediately
    if (IS_P2P_GO_OP(pPort) || IS_P2P_CLIENT_OP(pPort))
    {
        if (((IS_P2P_SIGMA_ON(pPort))&& (pAd->HwCfg.LatchRfRegs.Channel != pPort->CentralChannel))
            || (MT_TEST_FLAG(pAd, fRTMP_ADAPTER_RADIO_OFF))
            || (MT_TEST_FLAG(pAd, fRTMP_ADAPTER_RESET_IN_PROGRESS))
            || (MT_TEST_FLAG(pAd, fRTMP_ADAPTER_HALT_IN_PROGRESS))
            || (MT_TEST_FLAG(pAd, fRTMP_ADAPTER_NIC_NOT_EXIST))
            || (IS_P2P_ABSENCE(pPort)) 
            || (pPort->P2PCfg.P2PChannelState == P2P_ENTER_GOTOSCAN) 
            || ((pPort->P2PCfg.GONoASchedule.bValid == TRUE) && (pPort->P2PCfg.GONoASchedule.bWMMPSInAbsent == TRUE))
            )
        {
            DBGPRINT(RT_DEBUG_TRACE, ("%s - RETURN(1)\n", __FUNCTION__));
            return;
        }
    }
    else if (MT_TEST_FLAG(pAd, fRTMP_ADAPTER_RESET_IN_PROGRESS)
            || MT_TEST_FLAG(pAd, fRTMP_ADAPTER_BSS_SCAN_IN_PROGRESS)
            || MT_TEST_FLAG(pAd, fRTMP_ADAPTER_HALT_IN_PROGRESS)
            || (IS_P2P_ABSENCE(pPort))
            || ((pPort->P2PCfg.GONoASchedule.bValid == TRUE) && (pPort->P2PCfg.GONoASchedule.bWMMPSInAbsent == TRUE))
#ifdef MULTI_CHANNEL_SUPPORT            
            || (MT_TEST_BIT(pAd->pP2pCtrll->MultiChannelStartOrEndProcess, MULTI_CHANNEL_MSDU_TX_SUSPEND) && (MT_TEST_BIT(pAd->pP2pCtrll->MultiChannelStartOrEndProcess, MULTI_CHANNEL_IN_NEW_SESSION) == FALSE))
#endif /*MULTI_CHANNEL_SUPPORT*/            
            )
    {
        DBGPRINT(RT_DEBUG_ERROR,("--NdisCommonUsbDeQueuePacket %d reset-in-progress pAd->flags = %x!!--\n", BulkOutPipeId,pAd->Flags));
        DBGPRINT(RT_DEBUG_ERROR,("%d  %d  %d %d !!--\n", pPort->P2PCfg.bPreKeepSlient, pPort->P2PCfg.bKeepSlient, pPort->P2PCfg.GONoASchedule.bValid, pPort->P2PCfg.GONoASchedule.bWMMPSInAbsent));

#ifdef MULTI_CHANNEL_SUPPORT        
        DBGPRINT(RT_DEBUG_TRACE, ("%s - RETURN(2)(%d, %d, %d, %d, %d, %d)\n", __FUNCTION__,
                                    MT_TEST_FLAG(pAd, fRTMP_ADAPTER_RESET_IN_PROGRESS),
                                    MT_TEST_FLAG(pAd, fRTMP_ADAPTER_BSS_SCAN_IN_PROGRESS) ,
                                    MT_TEST_FLAG(pAd, fRTMP_ADAPTER_HALT_IN_PROGRESS),
                                    (IS_P2P_ABSENCE(pPort)),
                                    ((pPort->P2PCfg.GONoASchedule.bValid == TRUE) && (pPort->P2PCfg.GONoASchedule.bWMMPSInAbsent == TRUE)),
                                    (MT_TEST_BIT(pAd->pP2pCtrll->MultiChannelStartOrEndProcess, MULTI_CHANNEL_MSDU_TX_SUSPEND) && (MT_TEST_BIT(pAd->pP2pCtrll->MultiChannelStartOrEndProcess, MULTI_CHANNEL_IN_NEW_SESSION) == FALSE))
                                    ));
#endif /*MULTI_CHANNEL_SUPPORT*/
        
        return;
    }

    QueIdx = BulkOutPipeId;

#ifdef MULTI_CHANNEL_SUPPORT
    if (MT_TEST_BIT(pAd->pP2pCtrll->MultiChannelStartOrEndProcess, MULTI_CHANNEL_IN_NEW_SESSION))
    {
        DBGPRINT(RT_DEBUG_TRACE, ("%s - MultiChSts: Return QID_HCCA - 1(3)\n", __FUNCTION__));
        QueIdx = (QID_HCCA - 1);
    }
    else if (pAd->MccCfg.MultiChannelEnable == TRUE)
    {       
        if (pAd->MccCfg.SwQSelect == QID_AC_BE)
        {
            DBGPRINT(RT_DEBUG_TRACE, ("%s - MultiChSts: Return QID_AC_BE(0)\n", __FUNCTION__));
            QueIdx = QID_AC_BE;
        }
        else if (pAd->MccCfg.SwQSelect == (QID_HCCA - 1))
        {
            DBGPRINT(RT_DEBUG_TRACE, ("%s - MultiChSts: Return (QID_HCCA-1)(3)\n", __FUNCTION__));
            QueIdx = (QID_HCCA - 1);
        }
    }
    else
    {
        DBGPRINT(RT_DEBUG_INFO, ("%s - No channel switch: Return SWQ(%d)(%d)\n", __FUNCTION__, BulkOutPipeId, QueIdx));
    }
        
    BulkOutPipeId = QueIdx;
#endif /*MULTI_CHANNEL_SUPPORT*/
    
    if ((NdisCommonUsbFreeDescriptorRequest(pAd, TX_RING, BulkOutPipeId, 1) != NDIS_STATUS_SUCCESS)) 
    {
        DBGPRINT(RT_DEBUG_TRACE,("--NdisCommonUsbDeQueuePacket %d queue full !!  early return\n", BulkOutPipeId));
        return;
    }

    NdisAcquireSpinLock(&pAd->pTxCfg->DeQueueLock[BulkOutPipeId]);
    if (pAd->pTxCfg->DeQueueRunning[BulkOutPipeId])
    {
        DBGPRINT(RT_DEBUG_TRACE,("--NdisCommonUsbDeQueuePacket(DeQueueRunning[%d]), RETURN\n", BulkOutPipeId));
        NdisReleaseSpinLock(&pAd->pTxCfg->DeQueueLock[BulkOutPipeId]);
        return;
    }
    else
    {
        pAd->pTxCfg->DeQueueRunning[BulkOutPipeId] = TRUE;
        NdisReleaseSpinLock(&pAd->pTxCfg->DeQueueLock[BulkOutPipeId]);
    }

    // Make sure SendTxWait queue resource won't be used by other threads
    NdisAcquireSpinLock(&pAd->pTxCfg->SendTxWaitQueueLock[BulkOutPipeId]);

    // Select Queue
    pQueue = &pAd->pTxCfg->SendTxWaitQueue[BulkOutPipeId];

    pPort = pAd->PortList[NDIS_DEFAULT_PORT_NUMBER];

    TxPacketNum = (pPort->P2PCfg.bOppsOn) ? ((pPort->P2PCfg.CTWindows & 0x7f) / 10) : (MAX_TX_PROCESS);
    
    // Check queue before dequeue
    while ((pQueue->Head != NULL) && (Count < TxPacketNum))
    {
// Move to above
#if 0   
        // Reset is in progress, stop immediately
        if (IS_P2P_GO_OP(pPort) || IS_P2P_CLIENT_OP(pPort))
        {
            if (((IS_P2P_SIGMA_ON(pPort))&& (pAd->HwCfg.LatchRfRegs.Channel != pPort->CentralChannel))  ||
                (MT_TEST_FLAG(pAd, fRTMP_ADAPTER_RADIO_OFF))              ||
                (MT_TEST_FLAG(pAd, fRTMP_ADAPTER_RESET_IN_PROGRESS))      ||
                (MT_TEST_FLAG(pAd, fRTMP_ADAPTER_HALT_IN_PROGRESS))       ||
                (MT_TEST_FLAG(pAd, fRTMP_ADAPTER_NIC_NOT_EXIST))
                || (IS_P2P_ABSENCE(pPort)) 
                || (pPort->P2PCfg.P2PChannelState == P2P_ENTER_GOTOSCAN) 
                || ((pPort->P2PCfg.GONoASchedule.bValid == TRUE) && (pPort->P2PCfg.GONoASchedule.bWMMPSInAbsent == TRUE)))
            {
                DBGPRINT(RT_DEBUG_TRACE, ("%s - RETURN(1)\n", __FUNCTION__));
                break;
            }
        }
        else if (MT_TEST_FLAG(pAd, fRTMP_ADAPTER_RESET_IN_PROGRESS)                                           ||
            MT_TEST_FLAG(pAd, fRTMP_ADAPTER_BSS_SCAN_IN_PROGRESS)                                         ||
            MT_TEST_FLAG(pAd, fRTMP_ADAPTER_HALT_IN_PROGRESS)                                             ||
            (IS_P2P_ABSENCE(pPort))                                                                               ||
            ((pPort->P2PCfg.GONoASchedule.bValid == TRUE) && (pPort->P2PCfg.GONoASchedule.bWMMPSInAbsent == TRUE))      ||
            (MT_TEST_BIT(pAd->pP2pCtrll->MultiChannelStartOrEndProcess, MULTI_CHANNEL_MSDU_TX_SUSPEND) && (MT_TEST_BIT(pAd->pP2pCtrll->MultiChannelStartOrEndProcess, MULTI_CHANNEL_IN_NEW_SESSION) == FALSE)))
        {
            DBGPRINT(RT_DEBUG_ERROR,("--NdisCommonUsbDeQueuePacket %d reset-in-progress pAd->flags = %x!!--\n", BulkOutPipeId,pAd->Flags));
            DBGPRINT(RT_DEBUG_ERROR,("%d  %d  %d %d !!--\n", pPort->P2PCfg.bPreKeepSlient, pPort->P2PCfg.bKeepSlient, pPort->P2PCfg.GONoASchedule.bValid, pPort->P2PCfg.GONoASchedule.bWMMPSInAbsent));
            DBGPRINT(RT_DEBUG_TRACE, ("%s - RETURN(2)(%d, %d, %d, %d, %d, %d)\n", __FUNCTION__,
                                        MT_TEST_FLAG(pAd, fRTMP_ADAPTER_RESET_IN_PROGRESS),
                                        MT_TEST_FLAG(pAd, fRTMP_ADAPTER_BSS_SCAN_IN_PROGRESS) ,
                                        MT_TEST_FLAG(pAd, fRTMP_ADAPTER_HALT_IN_PROGRESS),
                                        (IS_P2P_ABSENCE(pPort)),
                                        ((pPort->P2PCfg.GONoASchedule.bValid == TRUE) && (pPort->P2PCfg.GONoASchedule.bWMMPSInAbsent == TRUE)),
                                        (MT_TEST_BIT(pAd->pP2pCtrll->MultiChannelStartOrEndProcess, MULTI_CHANNEL_MSDU_TX_SUSPEND) && (MT_TEST_BIT(pAd->pP2pCtrll->MultiChannelStartOrEndProcess, MULTI_CHANNEL_IN_NEW_SESSION) == FALSE))
                                        ));
            
            break;
        }
#endif

        // Suppport RaTxAggregation
        if ((pPort->CommonCfg.bAggregationCapable) &&
            (pAd->pHifCfg->BulkLastOneSecCount >= RATXAGG_TH_EN) &&          
            (pQueue->Number >= 2))
        {
            PMAC_TABLE_ENTRY pBssidMacTabEntry = MlmeSyncMacTabMatchedRoleType(pPort, ROLE_WLANIDX_BSSID);
            if(pBssidMacTabEntry == NULL)
            {
                DBGPRINT(RT_DEBUG_TRACE,("%s(), Line:%d, Mac Entry is NULL\n", __FUNCTION__, __LINE__));
                return;
            } 
            
            // SoftAP just check BSSID_WCID entry to support RaTxAggregation
            if ((pAd->OpMode == OPMODE_AP) &&
                CLIENT_STATUS_TEST_FLAG(pBssidMacTabEntry, fCLIENT_STATUS_AGGREGATION_CAPABLE) &&
                (READ_PHY_CFG_MODE(pAd, &pBssidMacTabEntry->TxPhyCfg) >= MODE_OFDM))
            {
                bDoingRaTxAgg = TRUE;
            }       
            else if (((pAd->OpMode == OPMODE_STA) || (pAd->OpMode == OPMODE_STAP2P)) && INFRA_ON(pPort) &&
                OPSTATUS_TEST_FLAG(pAd, fOP_STATUS_AGGREGATION_INUSED) &&
                (READ_PHY_CFG_MODE(pAd, &pBssidMacTabEntry->TxPhyCfg) >= MODE_OFDM))
            {
                bDoingRaTxAgg = TRUE;
            }
            else
                bDoingRaTxAgg = FALSE;
            DBGPRINT(RT_DEBUG_LOUD,("RaTxAgg: %s bDoingRaTxAgg[%d] Count[%d] pQueue->Number[%d]\n", __FUNCTION__,bDoingRaTxAgg,Count,pQueue->Number));          
        }

        // Dequeue the first entry from head of queue list
        pEntry = RemoveHeadQueue(pQueue);

        // For VHT Throughput tuning (ToDo: Remove)
        if (pEntry == NULL)
            DBGPRINT(RT_DEBUG_ERROR, ("[2*2] pEntry == NULL in %s\n", __FUNCTION__));

        // Retrieve Ndis NET BUFFER pointer from MiniportReserved field
        
        pXcu = (PMT_XMIT_CTRL_UNIT)pEntry;
        
        // RTS or CTS-to-self for B/G protection mode has been set already.
        // There is no need to re-do it here. 
        // Total fragment required = number of fragment + RST if required
        FragmentRequired = pXcu->FragmentRequired;//MT_GET_NETBUFFER_FRAGMENTS(NetBuffer);

        // Suppport RaTxAggregation
        // Get next NDIS Packet from SWQ
        if (bDoingRaTxAgg && (pQueue->Head != NULL) && (FragmentRequired ==1))
        {
            bDoingRaTxAgg = FALSE;
            DBGPRINT(RT_DEBUG_LOUD,("RaTxAgg: %s get  NetBufferRaTxAgg \n", __FUNCTION__));         
        }

        OpMode = pXcu->OpMode;//MT_GET_NETBUFFER_OPMODE(NetBuffer);
        PortNumber = (UCHAR)pXcu->PortNumber;//MT_GET_NETBUFFER_PORT(NetBuffer);
        
        if ((NdisCommonUsbFreeDescriptorRequest(pAd, TX_RING, BulkOutPipeId, FragmentRequired) == NDIS_STATUS_SUCCESS)) 
        {
            // Avaliable ring descriptors are enough for this frame
            // Call hard transmit
            // Nitro mode / Normal mode selection
            NdisReleaseSpinLock(&pAd->pTxCfg->SendTxWaitQueueLock[BulkOutPipeId]);

            //pXcu->QueIdx = QueIdx;
            //pXcu->FragmentRequired = FragmentRequired;
            
            if (pXcu->PktSource== PKTSRC_BAR)
            {
                ndisStatus = XmitSendBAR(pAd, pXcu, QueIdx);   
            }
            else if (OpMode == OPMODE_AP )
                ndisStatus = ApDataHardTransmit(pAd, pXcu);
            else if(OpMode == OPMODE_STA)
            {
                ndisStatus = XmitHardTransmit(pAd, pXcu);
            }
            else
            {
                DBGPRINT(RT_DEBUG_ERROR,("%s ==> Unknown OpMode [%d]\n", __FUNCTION__,OpMode)); 
                ndisStatus = NDIS_STATUS_FAILURE;
            }

            //
            //Should not hold the spinlock(SendTxWaitQueueLock) before RELEASE NDIS Packet
            //
            if (ndisStatus == NDIS_STATUS_FAILURE)
            {
                if((pXcu != NULL) && (pXcu->Reserve1 != NULL) && (pXcu->Reserve2 != NULL))
                {
                    RELEASE_NDIS_NETBUFFER(pAd, pXcu, NDIS_STATUS_FAILURE);
                }      
                
                NdisAcquireSpinLock(&pAd->pTxCfg->DeQueueLock[BulkOutPipeId]);
                pAd->pTxCfg->DeQueueRunning[BulkOutPipeId] = FALSE;
                NdisReleaseSpinLock(&pAd->pTxCfg->DeQueueLock[BulkOutPipeId]);
                return;
            }
            else if (ndisStatus == NDIS_STATUS_NOT_ACCEPTED)
            {
                if((pXcu != NULL) && (pXcu->Reserve1 != NULL) && (pXcu->Reserve2 != NULL))
                {
                    RELEASE_NDIS_NETBUFFER(pAd, pXcu, NDIS_STATUS_FAILURE);
                }      
            }

            // Make sure SendTxWait queue resource won't be used by other threads
            NdisAcquireSpinLock(&pAd->pTxCfg->SendTxWaitQueueLock[BulkOutPipeId]);
            if (ndisStatus == NDIS_STATUS_RESOURCES)
            {
                NdisSetEvent(&(GET_THREAD_EVENT(&pAd->pHifCfg->hBulkOutDataThread[BulkOutPipeId])));
                // Not enough free tx ring, it might happen due to free descriptor inquery might be not correct
                // It also might change to NDIS_STATUS_FAILURE to simply drop the frame
                // Put the frame back into head of queue
                DBGPRINT(RT_DEBUG_ERROR,("RESOURSE NOT ENOUGH, PipeId = %d, NetBuffer = %X\n", BulkOutPipeId, pXcu->Reserve2));
               
                InsertHeadQueue(pQueue, pXcu);
                break;
            }
            else if (ndisStatus == NDIS_STATUS_ADAPTER_NOT_READY)
            {
                // If fRTMP_ADAPTER_ADDKEY_IN_PROGRESS happens, we blocks packets except EAPOL frame.
                // We must put it into TailQueue because it is possible that EAPOL frame is in next position.
                DBGPRINT(RT_DEBUG_ERROR,("NDIS_STATUS_ADAPTER_NOT_READY\n"));
                InsertTailQueue(pQueue, pXcu);               
                
                break;
            }
            else if( ndisStatus == NDIS_STATUS_NOT_ACCEPTED)
            {
                //no key install ,fail this netbuffer list
                // In this case, we keep continuing to dequeue the NetBuffer, 
                // Otherwise will happen packet not completing issue
                //
            }
            else if( ndisStatus == NDIS_STATUS_NOT_COPIED )
            {
                // Support RaTxAggregation: If we can't do aggregation in HardTransmit function, put it into SendTxWaitQueue
                DBGPRINT(RT_DEBUG_TRACE,("NDIS_STATUS_NOT_COPIED\n"));              
                Count++;    
            }
            else
            {
                Count++;
            }
        }
        else
        {
            DBGPRINT(RT_DEBUG_ERROR,("--NdisCommonUsbDeQueuePacket %d queue full !!\n", BulkOutPipeId));    
            InsertHeadQueue(pQueue, pXcu);
            break;
        }
    }

    // Release TxSwQueue0 resources
    NdisReleaseSpinLock(&pAd->pTxCfg->SendTxWaitQueueLock[BulkOutPipeId]);

    NdisAcquireSpinLock(&pAd->pTxCfg->DeQueueLock[BulkOutPipeId]);
    pAd->pTxCfg->DeQueueRunning[BulkOutPipeId] = FALSE;
    NdisReleaseSpinLock(&pAd->pTxCfg->DeQueueLock[BulkOutPipeId]);
}

/*
    ========================================================================

    Routine Description:
        This subroutine will scan through releative ring descriptor to find
        out avaliable free ring descriptor and compare with request size.
        
    Arguments:
        pAd Pointer to our adapter
        RingType    Selected Ring
        
    Return Value:
        NDIS_STATUS_FAILURE     Not enough free descriptor
        NDIS_STATUS_SUCCESS     Enough free descriptor

    Note:
    
    ========================================================================
*/
NDIS_STATUS 
NdisCommonUsbFreeDescriptorRequest(
    IN  PMP_ADAPTER   pAd,
    IN  UCHAR           RingType,
    IN  UCHAR           BulkOutPipeId,
    IN  ULONG           NumberRequired
    )
{
    UCHAR           FreeNumber = 0;
    UINT            Index;
    NDIS_STATUS     Status = NDIS_STATUS_FAILURE;

    switch (RingType)
    {
        case TX_RING:
            NdisAcquireSpinLock(&pAd->pTxCfg->TxContextQueueLock[BulkOutPipeId]);
            if (((pAd->pHifCfg->TxContext[BulkOutPipeId].CurWritePosition - pAd->pHifCfg->TxContext[BulkOutPipeId].ENextBulkOutPosition) < ((SEGMENT_TOTAL - 2)*ONE_SEGMENT_UNIT))&& ((pAd->pHifCfg->TxContext[BulkOutPipeId].CurWritePosition) >= pAd->pHifCfg->TxContext[BulkOutPipeId].ENextBulkOutPosition))
            {
                DBGPRINT(RT_DEBUG_INFO,("RTUSBFreeD s1 --> CurWritePosition = %d, NextBulkOutPosition = %d. \n", pAd->pHifCfg->TxContext[BulkOutPipeId].CurWritePosition, pAd->pHifCfg->TxContext[BulkOutPipeId].NextBulkOutPosition));
                Status = NDIS_STATUS_SUCCESS;
            }
            else if (( (LONG)((LONG)pAd->pHifCfg->TxContext[BulkOutPipeId].NextBulkOutPosition - (LONG)pAd->pHifCfg->TxContext[BulkOutPipeId].CurWritePosition) > 2*ONE_SEGMENT_UNIT) && (pAd->pHifCfg->TxContext[BulkOutPipeId].CurWritePosition < pAd->pHifCfg->TxContext[BulkOutPipeId].NextBulkOutPosition))
            {
                DBGPRINT(RT_DEBUG_INFO,("s2 --> Cur = %d, Next = %d,Enext =%d ,Status =%x\n", 
                                    pAd->pHifCfg->TxContext[BulkOutPipeId].CurWritePosition,
                                    pAd->pHifCfg->TxContext[BulkOutPipeId].NextBulkOutPosition,
                                    pAd->pHifCfg->TxContext[BulkOutPipeId].ENextBulkOutPosition,
                                    Status));

                MTUSB_SET_BULK_FLAG(pAd, (fRTUSB_BULK_OUT_DATA_NORMAL << BulkOutPipeId));
                Status = NDIS_STATUS_SUCCESS;
            }
            else
            {
                NdisSetEvent(&(GET_THREAD_EVENT(&pAd->pHifCfg->hBulkOutDataThread[BulkOutPipeId])));

                DBGPRINT(RT_DEBUG_TRACE,("s3 --> PipeID = %d, Cur = %d, Next = %d,Enext =%d ,Status =%x\n", 
                                    BulkOutPipeId,
                                    pAd->pHifCfg->TxContext[BulkOutPipeId].CurWritePosition,
                                    pAd->pHifCfg->TxContext[BulkOutPipeId].NextBulkOutPosition,
                                    pAd->pHifCfg->TxContext[BulkOutPipeId].ENextBulkOutPosition,
                                    Status));
            }
            NdisReleaseSpinLock(&pAd->pTxCfg->TxContextQueueLock[BulkOutPipeId]);
            break;
            
        case PRIO_RING:
            Index = pAd->pHifCfg->NextMLMEIndex;
            do
            {
                PTX_CONTEXT pTxD  = &pAd->pHifCfg->MLMEContext[Index];
                
                // While Owner bit is NIC, obviously ASIC still need it.
                // If valid bit is TRUE, indicate that TxDone has not process yet
                // We should not use it until TxDone finish cleanup job
                if (pTxD->InUse == FALSE)
                {
                    // This one is free
                    FreeNumber++;
                }
                else
                {
                    break;
                }
                    
                Index = (Index + 1) % PRIO_RING_SIZE;               
            }   while (FreeNumber < NumberRequired);    // Quit here ! Free number is enough !

            if (FreeNumber >= NumberRequired)
            {
                Status = NDIS_STATUS_SUCCESS;
            }
            break;

        default:
            DBGPRINT_RAW(RT_DEBUG_ERROR, ("--->NdisCommonUsbFreeDescriptorRequest() -----!! \n"));
            
            break;
    }
    
    return (Status);
}

/*
    ========================================================================

    Routine Description:
        Copy frame from waiting queue into relative ring buffer and set 
    appropriate ASIC register to kick hardware transmit function
    
    Arguments:
        pAd Pointer to our adapter
        pBuffer     Pointer to  memory of outgoing frame
        Length      Size of outgoing management frame
        
    Return Value:
        NDIS_STATUS_FAILURE
        NDIS_STATUS_PENDING
        NDIS_STATUS_SUCCESS

    Note:
    
    ========================================================================
*/
VOID    
NdisCommonUsbMlmeHardTransmit(
    IN  PMP_ADAPTER   pAd,
    IN  PMGMT_STRUC     pMgmt
    )
{
    PTX_CONTEXT     pMLMEContext;
//  PTXWI_STRUC     pTxWI;
//  PTXINFO_STRUC       pTxInfo;
    PUCHAR              pDest;  
    PHEADER_802_11      pHeader_802_11;
    PVOID               pBuffer = pMgmt->pBuffer;
    PPHY_CFG            pMgmtPhyCfg = pMgmt->pMgmtPhyCfg;
//  PULONG                  pData;
    PHY_CFG             MgmtPhyCfg = {0};
    ULONG               MfpEncryption;
    PMP_PORT          pPort = pAd->PortList[NDIS_DEFAULT_PORT_NUMBER];
    UCHAR               uPortNum, tmpSsidOffset;
    PEID_STRUCT         pSsidEid = NULL;
    PMAC_TABLE_ENTRY pWcidMacTabEntry = NULL;

//==================================
    PTXDSCR_LONG_STRUC          pLongFormatTxD = NULL;
    HARD_TRANSMIT_INFO              TxInfo = {0};
//==================================
     pHeader_802_11 = (PHEADER_802_11) pBuffer;

     //Get port
    if (pAd->NumberOfPorts > 1)
    {
        for (uPortNum = 0; uPortNum < pAd->NumberOfPorts; uPortNum++)
        {
            if (pAd->PortList[uPortNum] && pAd->PortList[uPortNum]->bActive)
            {
                if (MAC_ADDR_EQUAL(pAd->PortList[uPortNum]->CurrentAddress, pHeader_802_11->Addr2))
                {
                    pPort = pAd->PortList[uPortNum];
                    break;
                }
            }
        }
    }

    pWcidMacTabEntry = MlmeSyncMacTabMatchedBssid(pPort, pHeader_802_11->Addr2);

    DBGPRINT(RT_DEBUG_TRACE, ("%s   port %d\n", __FUNCTION__, pPort->PortNumber));

    TxInfo.QueIdx = 0x4;    // Q4 is for Mlme packet
    TxInfo.PortNumber = (UCHAR) pPort->PortNumber;
    TxInfo.FrameGap = IFS_HTTXOP;
    TxInfo.LengthQosPAD = 0;
    TxInfo.This80211HdrLen = LENGTH_802_11;
    TxInfo.Protocol = 0;
    TxInfo.bEAPOLFrame = FALSE;
    TxInfo.AckDuration = 0;
    TxInfo.EncryptionOverhead = 0;
    TxInfo.MimoPs = MMPS_ENABLE;
    TxInfo.bTXBA = FALSE; // TRUE if use block ack.
    TxInfo.BAWinSize = 0;
    TxInfo.bHTC = FALSE; // TRUE if HT AMSDU used for this MPDU.
    TxInfo.bWMM = FALSE; // TRUE if HT AMSDU used for this MPDU.
    TxInfo.bHtVhtFrame = FALSE;
    TxInfo.Wcid = (UCHAR)MlmeSyncGetWlanIdxByPort(pPort, ROLE_WLANIDX_MBCAST);   // need to check : Lens
    TxInfo.RABAOriIdx = 0;  //The RA's BA Originator table index. 
    TxInfo.bPiggyBack = FALSE;
    TxInfo.bAMSDU = FALSE;// use HT AMSDU
    TxInfo.bGroupMsg2 = FALSE; // is this a group msg 2? for CCX CCKM   
    TxInfo.bDHCPv6 = FALSE;
    TxInfo.bDHCPFrame = FALSE;
    TxInfo.NextValid = 0;
    TxInfo.bDLSFrame = FALSE;
    TxInfo.bTDLSFrame = FALSE;
    TxInfo.ExemptionActionType = DOT11_EXEMPT_NO_EXEMPTION;
    TxInfo.bWAIFrame = FALSE;
    TxInfo.bIsUnicastFrame = TRUE;
    TxInfo.bIsLastFrag = FALSE;
    TxInfo.bIsFrag = FALSE;
    TxInfo.bWiv = FALSE;
    TxInfo.PortSecured = pAd->StaCfg.PortSecured;
    TxInfo.SwUseSegIdx = 1;
    TxInfo.FragmentByteForTxD = 0;
    TxInfo.bNDPAnnouncement = pMgmt->bNDPAnnouncement;
    TxInfo.TotalPacketLength = pMgmt->Length;
    TxInfo.TxRate = RATE_1;
    TxInfo.MpduRequired = 0;
    TxInfo.UserPriority = 0;
    TxInfo.CipherAlg = 0;
    TxInfo.bBARPacket = FALSE;
    TxInfo.BARStartSequence = 0;


    
    if (pBuffer == NULL)
    {
        DBGPRINT_RAW(RT_DEBUG_ERROR, ("[%s] Buffer is NULL!! \n",__FUNCTION__));    
        return;
    }


    //DBGPRINT_RAW(RT_DEBUG_ERROR, ("[%s] subtype:%d \n",__FUNCTION__,pHeader_802_11->FC.SubType ));
    //DumpFrameMessage((PUCHAR)pHeader_802_11, LENGTH_802_11, ("Header la"));
    



    // win7 only if we use port0 as SA of probe request frame
    // change pPort to p2p port number (let port not use 11b rate)
    tmpSsidOffset = sizeof(HEADER_802_11); //=24
    pSsidEid = (PEID_STRUCT)((PUCHAR)pBuffer + tmpSsidOffset);
    if (P2P_ON(pPort) &&
        PORTV2_P2P_ON(pAd, pAd->PortList[pPort->P2PCfg.PortNumber]) &&
        (pHeader_802_11->FC.Type == BTYPE_MGMT) && 
        (pHeader_802_11->FC.SubType == SUBTYPE_PROBE_REQ) &&
        (TxInfo.TotalPacketLength >= (UINT)(tmpSsidOffset + 3)) &&
        (pSsidEid->Eid == IE_SSID) && 
        (pSsidEid->Len == WILDP2PSSIDLEN) && 
        PlatformEqualMemory(pSsidEid->Octet, WILDP2PSSID, WILDP2PSSIDLEN))
    {
        pPort = pAd->PortList[pPort->P2PCfg.PortNumber];
    }
    
    if (IS_P2P_ABSENCE(pPort) && (PORT_P2P_ON(pPort)) 
        && (pAd->HwCfg.LatchRfRegs.Channel == pPort->P2PCfg.GroupOpChannel))
    {
        NdisAcquireSpinLock(&pAd->pP2pCtrll->TxSwNoAMgmtQueueLock);
        InsertTailQueue(&pAd->pP2pCtrll->TxSwNoAMgmtQueue, pMgmt);     
        NdisReleaseSpinLock(&pAd->pP2pCtrll->TxSwNoAMgmtQueueLock);
        DBGPRINT(RT_DEBUG_TRACE, ("3 bKeepSlient = %d: Move to TxSwNoAMgmtQueue\n", pPort->P2PCfg.bKeepSlient));
        return;
    }
    
    // Before radar detection done, mgmt frame can not be sent but probe req
    // Because we need to use probe req to trigger driver to send probe req in passive scan
    if ((pHeader_802_11->FC.SubType != SUBTYPE_PROBE_REQ) && (pPort->CommonCfg.bIEEE80211H == 1) && (pPort->CommonCfg.RadarDetect.RDMode != RD_NORMAL_MODE))
    {
        DBGPRINT(RT_DEBUG_ERROR,("NdisCommonUsbMlmeHardTransmit --> radar detect not in normal mode !!!\n"));
        return;
    }

    if ((pPort->CommonCfg.bCarrierDetect) && (pPort->CommonCfg.bCarrierAssert) && (pHeader_802_11->FC.SubType != SUBTYPE_PROBE_REQ))
    {
        DBGPRINT(RT_DEBUG_INFO,("NdisCommonUsbMlmeHardTransmit --> carrier assertion !!!\n"));
        return;
    }

    TxInfo.PortNumber = (UCHAR)pPort->PortNumber;

    if (pHeader_802_11->Addr1[0] & 0x01)
    {
        DBGPRINT(RT_DEBUG_INFO,("NdisCommonUsbMlmeHardTransmit --> Broadcast/Multicast Frame !!!\n"));
        TxInfo.Wcid = (UCHAR)MlmeSyncGetWlanIdxByPort(pPort, ROLE_WLANIDX_MBCAST);
    }

    pMLMEContext = &pAd->pHifCfg->MLMEContext[pAd->pHifCfg->NextMLMEIndex];
    if (pMLMEContext->InUse == TRUE)
    {
        DBGPRINT(RT_DEBUG_TRACE,("NdisCommonUsbMlmeHardTransmit --> pAd->pHifCfg->MLMEContext[%d] in use !!!\n", pAd->pHifCfg->NextMLMEIndex));
        return;
    }
    
    pMLMEContext->InUse = TRUE; 

    // Increase & maintain Tx Ring Index
    pAd->pHifCfg->NextMLMEIndex++;
    if (pAd->pHifCfg->NextMLMEIndex >= PRIO_RING_SIZE)
    {
        pAd->pHifCfg->NextMLMEIndex = 0;
    }

    pLongFormatTxD = (PTXDSCR_LONG_STRUC)&pMLMEContext->TransferBuffer->WirelessPacket[0];
    PlatformZeroMemory(pLongFormatTxD, sizeof(TXDSCR_LONG_STRUC));
    
    pDest = &pMLMEContext->TransferBuffer->WirelessPacket[TXD_LONG_SIZE];              
    
    WRITE_PHY_CFG_DOUBLE_WORD(pAd, &MgmtPhyCfg, READ_PHY_CFG_DOUBLE_WORD(pAd, &pPort->CommonCfg.MgmtPhyCfg));
    
    // P2P don't use 11b rate.
    // Verify Mlme rate for a / g bands.
    TxInfo.TxRate = pPort->CommonCfg.MlmeRate;
    if ((PORT_P2P_ON(pPort) ||(pPort->PortType == WFD_DEVICE_PORT) || (pAd->HwCfg.LatchRfRegs.Channel > 14)) && (TxInfo.TxRate < RATE_6)) // 11A band
    {
        TxInfo.TxRate = RATE_6;
        WRITE_PHY_CFG_MODE(pAd, &MgmtPhyCfg, MODE_OFDM);
        WRITE_PHY_CFG_MCS(pAd, &MgmtPhyCfg, OfdmRateToRxwiMCS[RATE_6]);     
    }
    else if ((pAd->HwCfg.LatchRfRegs.Channel <= 14) && (pHeader_802_11->FC.SubType == SUBTYPE_PROBE_REQ))
    {
        WRITE_PHY_CFG_MODE(pAd, &MgmtPhyCfg, MODE_CCK);
        WRITE_PHY_CFG_MCS(pAd, &MgmtPhyCfg, OfdmRateToRxwiMCS[RATE_1]);     
    }

    if (isEnableETxBf(pAd))
    {
        if ((TxInfo.bNDPAnnouncement == TRUE) && (pMgmtPhyCfg != NULL))
        {
            WRITE_PHY_CFG_DOUBLE_WORD(pAd, &MgmtPhyCfg, READ_PHY_CFG_DOUBLE_WORD(pAd, pMgmtPhyCfg));
        }
    }

    if (TxInfo.bNDPAnnouncement == TRUE)
    {
        TxInfo.bSkipTxBF = FALSE;
    }
    else
    {
        TxInfo.bSkipTxBF = TRUE;
    }

    // In WMM-UAPSD, mlme frame should be set psm as power saving but probe request frame
    if (pHeader_802_11->FC.Type != BTYPE_DATA)
    {
        if ((pHeader_802_11->FC.SubType != SUBTYPE_PROBE_REQ) && 
            (pPort->CommonCfg.bAPSDCapable && pPort->CommonCfg.APEdcaParm.bAPSDCapable))
        {
            pHeader_802_11->FC.PwrMgmt = pPort->CommonCfg.bAPSDForcePowerSave;
        }
    }

    if (pHeader_802_11->FC.PwrMgmt != PWR_SAVE)
    {
        pHeader_802_11->FC.PwrMgmt = (pAd->StaCfg.Psm == PWR_SAVE);
    }
    
    TxInfo.bInsertTimestamp = FALSE;

    //Update P2P Client State
    if ((pHeader_802_11->FC.SubType == SUBTYPE_ACTION) && (pHeader_802_11->FC.Type == BTYPE_MGMT))
    {
        PP2P_PUBLIC_FRAME   pFrame = (PP2P_PUBLIC_FRAME)pHeader_802_11;
        PP2P_PUBLIC_FRAME   pPubFrame = (PP2P_PUBLIC_FRAME) pFrame;
        PP2P_ACTION_FRAME   pActFrame = (PP2P_ACTION_FRAME) pFrame;
        UCHAR               TempPid = 0;

#if(COMPILE_WIN8_ABOVE(CURRENT_OS_NDIS_VER))  
        if(pPort->PortType == WFD_DEVICE_PORT)
        {
            P2pMsAckRequiredCheck(pAd, pPort, pFrame, &TempPid);
            if (TempPid == PID_P2pMs_INDICATE)
            {
                TxInfo.PID = PID_P2pMs_INDICATE;

                //Free Old Buffered Data
                P2pMsFreeIeMemory(&pPort->P2PCfg.pSendOutBufferedFrame, (PLONG)&pPort->P2PCfg.SendOutBufferedSize);

                //Buffer frame for indication
                pPort->P2PCfg.SendOutBufferedSize = (USHORT)TxInfo.TotalPacketLength;
                PlatformAllocateMemory(pAd, &pPort->P2PCfg.pSendOutBufferedFrame, pPort->P2PCfg.SendOutBufferedSize);

                if (pPort->P2PCfg.pSendOutBufferedFrame != NULL)
                {
                    PlatformMoveMemory(pPort->P2PCfg.pSendOutBufferedFrame, pHeader_802_11, pPort->P2PCfg.SendOutBufferedSize);
                }
                else
                {
                    pPort->P2PCfg.SendOutBufferedSize = 0;
                }

                TxInfo.bAckRequired = TRUE;
                pPort->P2PCfg.bP2pAckReq = TRUE;
                
                //Preempt TXFIFO
                //MTEnqueueInternalCmd(pAd, pPort, MT_CMD_P2P_READ_TXFIFO, NULL, 0);                
            }
        }
        else
#endif
        {
            if(pPort->CommonCfg.bP2pAckCheck)
            {
                P2pAckRequiredCheck(pAd, pPort, pFrame, &TempPid);
                if (TempPid == PID_REQUIRE_ACK)
                {
                    TxInfo.PID = PID_REQUIRE_ACK;
                    TxInfo.bAckRequired = TRUE;
                    pPort->P2PCfg.bP2pAckReq = TRUE;
                    //Preempt TXFIFO
                    MTEnqueueInternalCmd(pAd, pPort, MT_CMD_P2P_READ_TXFIFO, NULL, 0);
                    DBGPRINT(RT_DEBUG_ERROR,("PID_REQUIRE_ACK Wait for ACK\n"));
                }
            }
            else if ((PlatformEqualMemory(&pPubFrame->OUI[0], P2POUIBYTE, 4)) ||
                 (PlatformEqualMemory(&pActFrame->OUI[0], P2POUIBYTE, 4)))
            {
                P2pClientStateUpdate(pAd, pPort);
            }
        }
    }

    if (pHeader_802_11->FC.Type == BTYPE_CNTL) // must be PS-POLL
    {
        TxInfo.bAckRequired = FALSE;
        //Set PM bit in ps-poll, and Ack is required
        if (pHeader_802_11->FC.SubType == SUBTYPE_PS_POLL)
        {
            pHeader_802_11->FC.PwrMgmt = PWR_SAVE;
            TxInfo.bAckRequired = TRUE;
        }
    }
    else // BTYPE_MGMT or BMGMT_DATA(must be NULL frame)
    {
        if ((pHeader_802_11->FC.SubType == SUBTYPE_BEACON) && (pHeader_802_11->FC.Type == BTYPE_MGMT))
        {
            TxInfo.QueIdx = 0x7;  // beacon queue
            DBGPRINT(RT_DEBUG_TRACE,("4 TxInfo.QueIdx = %d\n", TxInfo.QueIdx));
        }

        // The sequence of Eapol frame is filled in the sequence in MAC table since Eapol frame is a data frame.
        // MGMT frame use pAd->pTxCfg->Sequence.
        if (INFRA_ON(pPort)&& 
            (pHeader_802_11->FC.Type == BTYPE_DATA) && (pHeader_802_11->FC.SubType != SUBTYPE_NULL_FUNC))
        {
            PMAC_TABLE_ENTRY pBssidMacTabEntry = MlmeSyncMacTabMatchedRoleType(pPort, ROLE_WLANIDX_BSSID);
            if(pBssidMacTabEntry == NULL)
            {
                DBGPRINT(RT_DEBUG_TRACE,("%s(), Line:%d, Mac Entry is NULL\n", __FUNCTION__, __LINE__));
                return;
            } 
            
            pHeader_802_11->Sequence = pBssidMacTabEntry->BAOriSequence[0]++;
            pBssidMacTabEntry->BAOriSequence[0] %= 4096;
        }
        else
        {
            pAd->pTxCfg->Sequence = ((pAd->pTxCfg->Sequence) + 1) & (MAX_SEQ_NUMBER);
            pHeader_802_11->Sequence = pAd->pTxCfg->Sequence;
        }
    
        if (pHeader_802_11->Addr1[0] & 0x01) // MULTICAST, BROADCAST
        {
            INC_COUNTER64(pAd->Counter.WlanCounters.MulticastTransmittedFrameCount);
            TxInfo.bAckRequired = FALSE;
            pHeader_802_11->Duration = 0;
        }
        else
        {
            TxInfo.bAckRequired = TRUE;
            pHeader_802_11->Duration = XmitCalcDuration(pAd, TxInfo.TxRate, 14);
            if (pHeader_802_11->FC.SubType == SUBTYPE_PROBE_RSP)
            {
                TxInfo.bInsertTimestamp = TRUE;
                TxInfo.bAckRequired = FALSE;   // Disable ACK to prevent retry 0x1f for Probe Response
            }
            else if (pHeader_802_11->FC.SubType == SUBTYPE_PROBE_REQ)
            {
                TxInfo.bAckRequired = FALSE;  // Disable ACK to prevent retry 0x1f for Probe Request
            }
            else if ((pHeader_802_11->FC.Type == BTYPE_DATA) &&
                ((pHeader_802_11->FC.SubType == SUBTYPE_NULL_FUNC) || 
                (pHeader_802_11->FC.SubType == SUBTYPE_QOS_NULL)))
            {
                TxInfo.bAckRequired = FALSE; // Disable ACK to prevent retry 0x1f for NULL data
            }
            else
            {
                TxInfo.bAckRequired = TRUE;             
            }
        }
    }
    
    //
    // MFP encrypts the following management unicast frames. (Software Encryption)
    // 
    if (MFP_ACT(pAd, pPort) &&
        PlatformEqualMemory(pHeader_802_11->Addr1, pPort->PortCfg.Bssid, MAC_ADDR_LEN) &&
        (pHeader_802_11->FC.Type == BTYPE_MGMT) &&
        (pHeader_802_11->FC.SubType == SUBTYPE_DISASSOC ||
         pHeader_802_11->FC.SubType == SUBTYPE_DEAUTH   ||
         pHeader_802_11->FC.SubType == SUBTYPE_ACTION))
    {   
        // skip invalid type ====>
        BOOLEAN     bInvalidType = FALSE;

        // The protected management frame types are statisfied in the following lists: 
        // 1. Deauthentication frame
        // 2. Disassociation frame
        // 3. Action management frames of category: Spectrum management, QoS, Call Admission control

        if (pHeader_802_11->FC.SubType == SUBTYPE_ACTION)
        {
            PFRAME_ACTION_HDR   pActFrame = (PFRAME_ACTION_HDR)pHeader_802_11;
#if 0
            if (pActFrame->Category != CATEGORY_SPECTRUM &&
                pActFrame->Category != CATEGORY_QOS &&
                pActFrame->Category != ACM_CATEGORY_WME)
#endif
            //11w add a new column "Robust" for Action field,
            //Robust Action Frame should be protected when MFP is negotiated
            if (pActFrame->Category == CATEGORY_PUBLIC
                //&& pActFrame->Category != CATEGORY_VENDOR_SPECIFIC_PROTECT
            )
                bInvalidType = TRUE;    
        }

        if (bInvalidType == FALSE)
        {           
            // These fields must be muted.
            pHeader_802_11->FC.Retry    = 0;
            pHeader_802_11->FC.PwrMgmt  = 0;
            pHeader_802_11->FC.MoreData = 0;
            // The protected frame bit always set to 1.
            pHeader_802_11->FC.Wep = 1;

            MfpEncryption = pPort->PortCfg.WepStatus;
            
            // === TKIP Encapsulation ===
            //
            if (MfpEncryption/*pPort->CommonCfg.WepStatus*/ == Ralink802_11Encryption2Enabled) 
            {
                if (!RTMPSoftEncryptTKIP(pAd, 
                                        pPort,
                                        (PUCHAR)pBuffer, 
                                        (PUCHAR)pBuffer + LENGTH_802_11,
                                        TxInfo.TotalPacketLength - LENGTH_802_11, 
                                        0, 
                                        0, 
                                        pPort->SharedKey[BSS0],
                                        TRUE))
                {
                    DBGPRINT(RT_DEBUG_ERROR, ("CCX-MFP: Mgmt, RTMPSoftEncryptTKIP Failed\n"));
                    pMLMEContext->InUse = FALSE;
                    return;
                }
            }

            // === AES Encapsulation ===
            //
            if (MfpEncryption/*pPort->CommonCfg.WepStatus*/ == Ralink802_11Encryption3Enabled)
            {
#if 0 //old, not for 11w
                if (!RTMPSoftEncryptMgmtAES(pAd, 
                                        pPort,
                                        (PUCHAR)pBuffer, 
                                        (PUCHAR)pBuffer + LENGTH_802_11,
                                        TxInfo.TotalPacketLength - LENGTH_802_11, 
                                        0, 
                                        pPort->SharedKey[BSS0]))
                {
                    DBGPRINT(RT_DEBUG_ERROR, ("CCX-MFP: Mgmt, RTMPSoftEncryptMgmtAES Failed \n"));
                    pMLMEContext->InUse = FALSE;
                    return;
                }
#endif
                if(pAd->StaCfg.PmfCfg.PmfControl.Unprotected == 1 && (pHeader_802_11->FC.SubType == SUBTYPE_DISASSOC ||
                        pHeader_802_11->FC.SubType == SUBTYPE_DEAUTH || pHeader_802_11->FC.SubType == SUBTYPE_ACTION))
                {
                    DBGPRINT(RT_DEBUG_ERROR, ("MFP: Make Unprotected Fame\n"));
                    pHeader_802_11->FC.Wep = 0;
                }
                else
                {

                    if(pAd->StaCfg.PmfCfg.PmfControl.Bogus == 1 &&
                            (pHeader_802_11->FC.SubType == SUBTYPE_DISASSOC ||
                            pHeader_802_11->FC.SubType == SUBTYPE_DEAUTH || pHeader_802_11->FC.SubType == SUBTYPE_ACTION))
                    {
                        DBGPRINT(RT_DEBUG_ERROR, ("MFP: Make Bogus Fame\n"));
                        pPort->SharedKey[BSS0][0].Key[0] += 1;   // Make Key different if UI ask a Boug Frame

                        if (!RTMPSoftEncryptMgmtAES(pAd,
                                    pPort, 
                                    (PUCHAR)pBuffer,
                                    (PUCHAR)pBuffer + LENGTH_802_11,
                                    (TxInfo.TotalPacketLength - LENGTH_802_11), 
                                    0, 
                                    pPort->SharedKey[BSS0]))
                        {
                            DBGPRINT(RT_DEBUG_ERROR, ("CCX-MFP: Encrypt mgmt frame failed \n"));
                            pPort->SharedKey[BSS0][0].Key[0] -= 1;
                            pMLMEContext->InUse = FALSE;
                            return;
                        }
                        pPort->SharedKey[BSS0][0].Key[0] -= 1;
                    }
                    else
                    {
                        if (!RTMPSoftEncryptMgmtAES(pAd,
                                    pPort, 
                                    (PUCHAR)pBuffer,
                                    (PUCHAR)pBuffer + LENGTH_802_11,
                                    (TxInfo.TotalPacketLength - LENGTH_802_11), 
                                    0, 
                                    pPort->SharedKey[BSS0]))
                        {
                            DBGPRINT(RT_DEBUG_ERROR, ("CCX-MFP: Encrypt mgmt frame failed \n"));
                            pMLMEContext->InUse = FALSE;
                            return;
                        }
                    }
                }
            }
        }
    }

    PlatformMoveMemory(pDest, pBuffer, TxInfo.TotalPacketLength);

    // Initialize Priority Descriptor
    // For inter-frame gap, the number is for this frame and next frame
    // For MLME rate, we will fix as 2Mb to match other vendor's implement
#if 0   
    XmitWriteTxWI(pAd, pPort, pTxWI,  FALSE, InsertTimestamp, FALSE, AckRequired, FALSE, 0, MLME_WCID, (TxInfo.TotalPacketLength),
        Pid, 0, IFS_HTTXOP, FALSE, MgmtPhyCfg, bNDPAnnouncement, bSkipTxBF, FALSE);
    XmitWriteTxInfo(pAd, pPort, pTxInfo, (USHORT)(TxInfo.TotalPacketLength+pAd->HwCfg.TXWI_Length), TRUE, EpToQueue[MGMTPIPEIDX], FALSE,  FALSE);

    pData = (PULONG)pTxWI;
    DBGPRINT(RT_DEBUG_INFO,("Mlme txWI 0 = %x\n",*pData));
    DBGPRINT(RT_DEBUG_INFO,("Mlme txWI 1 = %x\n",*(pData + 1)));
    DBGPRINT(RT_DEBUG_INFO,("Mlme txWI 2 = %x\n",*(pData + 2)));
    DBGPRINT(RT_DEBUG_INFO,("Mlme txWI 3 = %x\n",*(pData + 3)));
#else
    DBGPRINT(RT_DEBUG_INFO, ("1====> XmitWriteTxD :  TxInfo.TotalPacketLength + TXD_LONG_SIZE = %d \n", TxInfo.TotalPacketLength + TXD_LONG_SIZE));
    XmitWriteTxD(pAd, pPort, NULL, pLongFormatTxD, TxInfo);
#endif
    // Always add 4 extra bytes at every packet 
    pDest+= TxInfo.TotalPacketLength;
    // Build our URB for USBD
    TxInfo.TransferBufferLength = TxInfo.TotalPacketLength + TXD_LONG_SIZE;
    
    // Patch TxQ0 or TxQ1 not empty issue. Need more padding.
    PlatformZeroMemory(pDest, 12);

    if ((TxInfo.TransferBufferLength % 4) == 1)             
        TxInfo.TransferBufferLength  += 3;  
    else if ((TxInfo.TransferBufferLength % 4) == 2)                
        TxInfo.TransferBufferLength  += 2;  
    else if ((TxInfo.TransferBufferLength % 4) == 3)            
        TxInfo.TransferBufferLength  += 1;  

//  WRITE_TXINFO_USBDMATxPktLen(pAd, pTxInfo, (TxInfo.TransferBufferLength - TXINFO_SIZE));
    

    TxInfo.TransferBufferLength += 4;

    if (0)
    {
        int i = 0;
        PUCHAR p = (PUCHAR)&pMLMEContext->TransferBuffer->WirelessPacket[0];
        //for (i = 0; i < (int)pRxD->RxDscrDW0.RxByteCount; i = i + 8)
        for (i = 0; i < (int)TxInfo.TransferBufferLength; i = i + 8)
        {
            DBGPRINT_RAW(RT_DEBUG_ERROR, ("NdisCommonUsbMlmeHardTransmit [%d] %x %x %x %x %x %x %x %x\n", i, p[i], p[i+1], p[i+2], p[i+3], p[i+4], p[i+5], p[i+6], p[i+7]));
        }
    }

     if ((pHeader_802_11->FC.Type == BTYPE_CNTL) && (pHeader_802_11->FC.SubType == SUBTYPE_BLOCK_ACK_REQ))
    {
        PFRAME_BAR  pFrameBar = (PFRAME_BAR)pHeader_802_11;
        USHORT      Sequence =0;   

        if(pWcidMacTabEntry != NULL)
        {
            Sequence = pWcidMacTabEntry->BAOriSequence[pFrameBar->BarControl.TID];
        }
        if (Sequence == 0)
            Sequence = 0xfff;
        else
            Sequence--;     
        pFrameBar->StartingSeq.field.StartSeq = Sequence;

        //DBGPRINT(RT_DEBUG_TRACE, ("BAR: New StartSeq = [0x%x] WirelessCliID[%d] TID[%d]\n",Sequence, pTxDDW1->WLANIndex, pFrameBar->BarControl.TID));

       
        pLongFormatTxD->TxDscrDW7.SwUseNonQoS = 0;
        
    }   

    //DBGPRINT_RAW(RT_DEBUG_ERROR, ("[%s] subtype:%d \n",__FUNCTION__,pHeader_802_11->FC.SubType ));
    //DumpFrameMessage((PUCHAR)pData, TransferBufferLength, ("whole packet la"));

    #if 0
    // If TransferBufferLength is multiple of 64, add extra 4 bytes again.
    if ((TransferBufferLength % pAd->pHifCfg->BulkOutMaxPacketSize) == 0)
    {       
        TransferBufferLength += 4;
    }
    #endif
    
    // Length in TxInfo should be 8 less than bulkout size.
    pMLMEContext->BulkOutSize = TxInfo.TransferBufferLength;
    NdisAcquireSpinLock(&pAd->pHifCfg->MLMEQLock);
    pAd->pHifCfg->PrioRingTxCnt++;
    NdisReleaseSpinLock(&pAd->pHifCfg->MLMEQLock);
    pMLMEContext->ReadyToBulkOut = TRUE;

    XmitSendMlmeCmdPkt(pAd, TXPKT_MLME_FRAME);

    DBGPRINT(RT_DEBUG_INFO, ("<---MlmeHardTransmit\n"));
}

/*
    ========================================================================
    
    Routine Description:

    Arguments:

    Return Value:

    IRQL = 
    
    Note:
    
    ========================================================================
*/
VOID    
NdisCommonUsbDequeueMLMEPacket(
    IN  PMP_ADAPTER   pAd
    )
{
    PMGMT_STRUC     pMgmt;

    NdisAcquireSpinLock(&pAd->pHifCfg->DeMGMTQueueLock);
    if (pAd->pHifCfg->DeMGMTQueueRunning)
    {
        DBGPRINT(RT_DEBUG_INFO, ("<---NdisCommonUsbDequeueMLMEPacket : DeMGMTQueueRunning\n"));
        NdisReleaseSpinLock(&pAd->pHifCfg->DeMGMTQueueLock);
        return;
    }
    else
    {
        pAd->pHifCfg->DeMGMTQueueRunning = TRUE;
        NdisReleaseSpinLock(&pAd->pHifCfg->DeMGMTQueueLock);
    }

    NdisAcquireSpinLock(&pAd->pHifCfg->MLMEWaitQueueLock);
    
    while ((pAd->pHifCfg->PopMgmtIndex != pAd->pHifCfg->PushMgmtIndex) || (pAd->pHifCfg->MgmtQueueSize > 0))
    {
        pMgmt = &pAd->pHifCfg->MgmtRing[pAd->pHifCfg->PopMgmtIndex];

        if (NdisCommonFreeDescriptorRequest(pAd, PRIO_RING, 0, 1) == NDIS_STATUS_SUCCESS)
        {
            InterlockedDecrement(&pAd->pHifCfg->MgmtQueueSize);
            pAd->pHifCfg->PopMgmtIndex = (pAd->pHifCfg->PopMgmtIndex + 1) % MGMT_RING_SIZE;
            NdisReleaseSpinLock(&pAd->pHifCfg->MLMEWaitQueueLock);

            NdisCommonMlmeHardTransmit(pAd, pMgmt);

            MlmeFreeMemory(pAd, pMgmt->pBuffer);
            pMgmt->pBuffer = NULL;
            pMgmt->bNDPAnnouncement = FALSE;
            pMgmt->pMgmtPhyCfg = NULL;
            pMgmt->Valid = FALSE;

            NdisAcquireSpinLock(&pAd->pHifCfg->MLMEWaitQueueLock);
        }
        else
        {
            DBGPRINT(RT_DEBUG_TRACE, ("not enough space in PrioRing[pAd->pHifCfg->MgmtQueueSize=%d]\n", pAd->pHifCfg->MgmtQueueSize));
            DBGPRINT(RT_DEBUG_TRACE, ("NdisCommonUsbDequeueMLMEPacket::PrioRingFirstIndex = %d, PrioRingTxCnt = %d, PopMgmtIndex = %d, PushMgmtIndex = %d, NextMLMEIndex = %d\n", 
                    pAd->pHifCfg->PrioRingFirstIndex, pAd->pHifCfg->PrioRingTxCnt, 
                    pAd->pHifCfg->PopMgmtIndex, pAd->pHifCfg->PushMgmtIndex, pAd->pHifCfg->NextMLMEIndex));            
            break;
        }
    }
    NdisReleaseSpinLock(&pAd->pHifCfg->MLMEWaitQueueLock);

    NdisAcquireSpinLock(&pAd->pHifCfg->DeMGMTQueueLock);
    pAd->pHifCfg->DeMGMTQueueRunning = FALSE;
    NdisReleaseSpinLock(&pAd->pHifCfg->DeMGMTQueueLock); 
}

/*
    ========================================================================

    Routine Description:
        API for MLME to transmit management frame to AP (BSS Mode)
    or station (IBSS Mode)
    
    Arguments:
        pAd Pointer to our adapter
        Buffer      Pointer to  memory of outgoing frame
        Length      Size of outgoing management frame
        
    Return Value:
        NDIS_STATUS_FAILURE
        NDIS_STATUS_PENDING
        NDIS_STATUS_SUCCESS

    Note:
    
    ========================================================================
*/
VOID    
NdisCommonUsbMiniportMMRequest(
    IN  PMP_ADAPTER   pAd,    
    IN  PVOID           pBuffer,
    IN  ULONG           Length
    )
{
    PHEADER_802_11  pHeader_802_11 = (PHEADER_802_11)pBuffer;
    ULONG           EncryptionOverhead = 0;
    ULONG           MfpEncryption;
    PMP_PORT      pPort = pAd->PortList[PORT_0];  
    BOOLEAN bNDPAnnouncement = FALSE;
    UCHAR           uPortNum;
    DBGPRINT_RAW(RT_DEBUG_INFO, ("---> NdisCommonUsbMiniportMMRequest\n"));

    //Get port
    if (pAd->NumberOfPorts > 1)
    {
        for (uPortNum = 0; uPortNum < pAd->NumberOfPorts; uPortNum++)
        {
            if (pAd->PortList[uPortNum] && pAd->PortList[uPortNum]->bActive)
            {
                if (MAC_ADDR_EQUAL(pAd->PortList[uPortNum]->CurrentAddress, pHeader_802_11->Addr2))
                {
                    pPort = pAd->PortList[uPortNum];
                    break;
                }
            }
        }
    }

    if (pBuffer)
    {
        PMGMT_STRUC pMgmt;

        // Check management ring free avaliability
        NdisAcquireSpinLock(&pAd->pHifCfg->MLMEWaitQueueLock);
        pMgmt = (PMGMT_STRUC)&pAd->pHifCfg->MgmtRing[pAd->pHifCfg->PushMgmtIndex];
        // This management cell has been occupied
        if (pMgmt->Valid == TRUE)
        {
            NdisReleaseSpinLock(&pAd->pHifCfg->MLMEWaitQueueLock);
            MlmeFreeMemory(pAd, pBuffer);
            pAd->Counter.MTKCounters.MgmtRingFullCount++;
            DBGPRINT_RAW(RT_DEBUG_WARN, ("NdisCommonUsbMiniportMMRequest (error:: MgmtRing full)\n"));
        }
        // Insert this request into software managemnet ring
        else
        {
            //
            // CCX MFP, calculate additional buffer size for IV/MIC/ICV
            //
            if ((MFP_ACT(pAd, pPort)) &&
                        (pAd->StaCfg.PmfCfg.PmfControl.Unprotected == 0) &&
                PlatformEqualMemory(pHeader_802_11->Addr1, pPort->PortCfg.Bssid, MAC_ADDR_LEN) &&
                (pHeader_802_11->FC.Type == BTYPE_MGMT) &&
                (pHeader_802_11->FC.SubType == SUBTYPE_DISASSOC ||
                pHeader_802_11->FC.SubType == SUBTYPE_DEAUTH    ||
                pHeader_802_11->FC.SubType == SUBTYPE_ACTION))
            {
                //skip invalid type ====>
                BOOLEAN     bInvalidType = FALSE;

                // The protected management frame types are statisfied in the following lists: 
                // 1. Deauthentication frame
                // 2. Disassociation frame
                // 3. Action management frames of category: Spectrum management, QoS, Call Admission control

                if (pHeader_802_11->FC.SubType == SUBTYPE_ACTION)
                {
                    PFRAME_ACTION_HDR   pActFrame = (PFRAME_ACTION_HDR)pHeader_802_11;
#if 0
                    if (pActFrame->Category != CATEGORY_SPECTRUM &&
                        pActFrame->Category != CATEGORY_QOS &&
                        pActFrame->Category != ACM_CATEGORY_WME)
#endif
                    if(pActFrame->Category == CATEGORY_PUBLIC)
                        bInvalidType = TRUE;    
                }


                if (bInvalidType == FALSE)
                {           
                    DBGPRINT(RT_DEBUG_WARN, ("NdisCommonUsbMiniportMMRequest, MFP is enabled ....... \n"));

                    // There is no QoS and Addr4 in 802.11 header
                    MfpEncryption = pPort->PortCfg.WepStatus;
        
                    if (MfpEncryption/*pPort->CommonCfg.WepStatus*/ == Ralink802_11Encryption2Enabled)
                        EncryptionOverhead = 34;//Extra length in TKIP:: IV[4] + EIV[4] + ICV[4] + MHDRIE{14} + MIC[8] 
                    else if (MfpEncryption/*pPort->CommonCfg.WepStatus*/ == Ralink802_11Encryption3Enabled) 
                        EncryptionOverhead = 16;//  Extra lengt in AES: IV[4] + EIV[4] + MIC[8]
                }
            }
            pMgmt->pBuffer = pBuffer;
            pMgmt->Length  = Length + EncryptionOverhead;
            if (EncryptionOverhead > 0)
                PlatformZeroMemory(pMgmt->pBuffer + Length, EncryptionOverhead);
            
            pMgmt->bNDPAnnouncement = FALSE;
            pMgmt->pMgmtPhyCfg = NULL;
            pMgmt->Valid   = TRUE;
            pAd->pHifCfg->PushMgmtIndex++;
            InterlockedIncrement(&pAd->pHifCfg->MgmtQueueSize);
            if (pAd->pHifCfg->PushMgmtIndex >= MGMT_RING_SIZE)
            {
                pAd->pHifCfg->PushMgmtIndex = 0;
            }
            NdisReleaseSpinLock(&pAd->pHifCfg->MLMEWaitQueueLock);
        }
    }
    else
        DBGPRINT(RT_DEBUG_WARN, ("NdisCommonUsbMiniportMMRequest (error:: NULL msg)\n"));
    
    NdisCommonUsbDequeueMLMEPacket(pAd);

    // If pAd->pHifCfg->PrioRingTxCnt is larger than 0, this means that prio_ring have something to transmit.
    // Then call KickBulkOut to transmit it
    if (pAd->pHifCfg->PrioRingTxCnt > 0)
    {
        if (OPSTATUS_TEST_FLAG(pAd, fOP_STATUS_DOZE))
            AsicForceWakeup(pAd);
        
        if(MT_TEST_FLAG(pAd, fRTMP_ADAPTER_IDLE_RADIO_OFF))
        {
            AsicRadioOn(pAd);
        }
        N6USBKickBulkOut(pAd);
    }
    
    DBGPRINT_RAW(RT_DEBUG_INFO, ("<--- NdisCommonUsbMiniportMMRequest\n"));
}

size_t
NdisCommonUsbGetPipeNumber(
    IN PMP_ADAPTER pAd
    )
{
#if (COMPILE_VISTA_ABOVE(CURRENT_OS_NDIS_VER))
    return N6UsbBulkGetPipeNumber(pAd);
#endif    
}