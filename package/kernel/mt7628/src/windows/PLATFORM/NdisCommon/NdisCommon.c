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
    NdisCommon.c

    Abstract:
    Miniport generic portion header file

    Revision History:
    Who         When          What
    --------    ----------    ----------------------------------------------
    */
    
#include    "MtConfig.h"

#pragma prefast(suppress: 28167, "The original IRQL will be restored later by calling PlatformReleaseOldRWLock")
VOID PlatformAcquireOldRWLockForWrite(
    PNDIS_RW_LOCK       Lock,
    PMP_RW_LOCK_STATE       LockState) 
{   
    LockState->ReadLock = FALSE;
#pragma prefast(suppress: 28103)    
    NdisAcquireReadWriteLock(Lock, TRUE, &(LockState->OldLockState));
}

#pragma prefast(suppress: 28167, "The original IRQL will be restored later by calling PlatformReleaseOldRWLock")
VOID PlatformAcquireOldRWLockForRead(
    PNDIS_RW_LOCK     Lock,
    PMP_RW_LOCK_STATE       LockState) 
{
    LockState->ReadLock = TRUE;
#pragma prefast(suppress: 28103)
    NdisAcquireReadWriteLock(Lock, FALSE, &(LockState->OldLockState));
}

#pragma prefast(suppress: 28167, "NdisReleaseReadWriteLock will restore the original IRQL used by its caller before the call to NdisAcquireReadWriteLock was made")
VOID PlatformReleaseOldRWLock(
    PNDIS_RW_LOCK       Lock,
    PMP_RW_LOCK_STATE       LockState)
{
    NdisReleaseReadWriteLock(Lock, &(LockState->OldLockState));
}

/*
    ========================================================================
    
    Routine Description:
        Create ControlThread and MLMEThread to run at PASSIVE_LEVEL.

    Arguments:
        Adapter                     Pointer to our adapter

    Return Value:
        NDIS_STATUS_SUCCESS
        NDIS_STATUS_FAILURE

    IRQL = PASSIVE_LEVEL

    Note:
        
    ========================================================================
*/
NDIS_STATUS 
NdisCommonCreateAllThreads(
    IN  PMP_ADAPTER   pAd
    )
{
    NTSTATUS    Status = NDIS_STATUS_SUCCESS;

        //
        // Create a ControlThread to run at PASSIVE_LEVEL.
        //  
        PlatformInitThread(pAd, &pAd->pHifCfg->hControlThread, (MT_THREAD_CALL_BACK)MTUSBCmdThread, "MTUsbCmdThread");
        Status = PlatformRunThread(pAd, &pAd->pHifCfg->hControlThread);
        
        if (Status != STATUS_SUCCESS)
        {
            DBGPRINT(RT_DEBUG_TRACE, ("NdisCommonCreateAllThreads fail  (hControlThread) Status= 0x%x\n",Status));
            Status = NDIS_STATUS_FAILURE;
            return Status;
        }

        //
        // Create a P2PThread to run at PASSIVE_LEVEL.
        //  
        PlatformInitThread(pAd, &pAd->pP2pCtrll->hP2PThread, (MT_THREAD_CALL_BACK)P2PCmdThread, "P2PCmdThread");
        Status = PlatformRunThread(pAd, &pAd->pP2pCtrll->hP2PThread);
        
        if (Status != STATUS_SUCCESS)
        {
            DBGPRINT(RT_DEBUG_TRACE, ("NdisCommonCreateAllThreads fail  (hP2PThread) Status= 0x%x\n",Status));
            Status = NDIS_STATUS_FAILURE;
            return Status;
        }

        //
        // Create a MultiChannelThread to run at PASSIVE_LEVEL.
        //  
        PlatformInitThread(pAd, &pAd->MccCfg.hMultiChannelThread, (MT_THREAD_CALL_BACK)MultiChannelCmdThread, "MultiChannelCmdThread");
        Status = PlatformRunThread(pAd, &pAd->MccCfg.hMultiChannelThread);
        
        if (Status != STATUS_SUCCESS)
        {
            DBGPRINT(RT_DEBUG_TRACE, ("NdisCommonCreateAllThreads fail  (hMultiChannelThread) Status= 0x%x\n",Status));
            Status = NDIS_STATUS_FAILURE;
            return Status;
        }        

        //
        // Create a MLMEThread to run at PASSIVE_LEVEL.
        //  
        PlatformInitThread(pAd, &pAd->pHifCfg->hMLMEThread, (MT_THREAD_CALL_BACK)MlmeThread, "MlmeThread");
        Status = PlatformRunThread(pAd, &pAd->pHifCfg->hMLMEThread);
        
        if (Status != STATUS_SUCCESS)
        {
            DBGPRINT(RT_DEBUG_TRACE, ("NdisCommonCreateAllThreads fail  (hMLMEThread) Status= 0x%x\n",Status));
            Status = NDIS_STATUS_FAILURE;
            return Status;
        }
      
        //
        // Create a DequeueDataThread to run at PASSIVE_LEVEL.
        //
        PlatformInitThread(pAd, &pAd->HwCfg.hLedThread, (MT_THREAD_CALL_BACK)LedCtrlBlinkThread, "LedCtrlBlinkThread");
        Status = PlatformRunThread(pAd, &pAd->HwCfg.hLedThread);
        
        if (Status != STATUS_SUCCESS)
        {
            DBGPRINT(RT_DEBUG_TRACE, ("NdisCommonCreateAllThreads fail  (hLedThread) Status= 0x%x\n",Status));
            Status = NDIS_STATUS_FAILURE;
            return Status;
        }  

    //Create tx data thread
    NdisCommonCreateTxDataThread(pAd);
    
        return Status;
}

VOID 
NdisCommonFreeAllThreads(
    IN  PMP_ADAPTER   pAd
    )
{
     if (pAd == NULL)
        return;

    pAd->TerminateThreads = TRUE;

    PlatformCancelThread(pAd, &pAd->MccCfg.hMultiChannelThread);
    PlatformFreeThread(pAd,&pAd->MccCfg.hMultiChannelThread);
   
    DBGPRINT(RT_DEBUG_TRACE, ("Terminate [MultiChannelCtrl] Thread Ok\n"));

    PlatformCancelThread(pAd, &pAd->HwCfg.hLedThread);
    PlatformFreeThread(pAd,&pAd->HwCfg.hLedThread);  

    DBGPRINT(RT_DEBUG_TRACE, ("Terminate [LedBlink] Thread Ok\n"));        

    if (pAd->pP2pCtrll != NULL)
    {
        PlatformCancelThread(pAd, &pAd->pP2pCtrll->hP2PThread);
        PlatformFreeThread(pAd,&pAd->pP2pCtrll->hP2PThread);

        DBGPRINT(RT_DEBUG_TRACE, ("Terminate [P2PCtrl] Thread Ok\n"));
    }

    if (pAd->pHifCfg != NULL)
    {            
        PlatformCancelThread(pAd, &pAd->pHifCfg->hControlThread);
        PlatformFreeThread(pAd,&pAd->pHifCfg->hControlThread);

        DBGPRINT(RT_DEBUG_TRACE, ("Terminate [Control] Thread Ok\n"));

        PlatformCancelThread(pAd, &pAd->pHifCfg->hMLMEThread);
        PlatformFreeThread(pAd,&pAd->pHifCfg->hMLMEThread);

        DBGPRINT(RT_DEBUG_TRACE, ("Terminate [Mlme] Thread Ok\n"));

        // BulkoutPipeId 0
        PlatformCancelThread(pAd, &pAd->pHifCfg->hBulkOutDataThread[0]);
        PlatformFreeThread(pAd,&pAd->pHifCfg->hBulkOutDataThread[0]);    

        DBGPRINT(RT_DEBUG_TRACE, ("Terminate [BulkOutData 0] Thread Ok\n"));
        // BulkoutPipeId 1
        PlatformCancelThread(pAd, &pAd->pHifCfg->hBulkOutDataThread[1]);
        PlatformFreeThread(pAd,&pAd->pHifCfg->hBulkOutDataThread[1]);  

        DBGPRINT(RT_DEBUG_TRACE, ("Terminate [BulkOutData 1] Thread Ok\n"));

        // BulkoutPipeId 2
        PlatformCancelThread(pAd, &pAd->pHifCfg->hBulkOutDataThread[2]);
        PlatformFreeThread(pAd,&pAd->pHifCfg->hBulkOutDataThread[2]);  

        DBGPRINT(RT_DEBUG_TRACE, ("Terminate [BulkOutData 2] Thread Ok\n"));

        // BulkoutPipeId 3
        PlatformCancelThread(pAd, &pAd->pHifCfg->hBulkOutDataThread[3]);
        PlatformFreeThread(pAd,&pAd->pHifCfg->hBulkOutDataThread[3]);      

        DBGPRINT(RT_DEBUG_TRACE, ("Terminate [BulkOutData 3] Thread Ok\n"));

        PlatformCancelThread(pAd, &pAd->pHifCfg->hDequeueDataThread);
        PlatformFreeThread(pAd,&pAd->pHifCfg->hDequeueDataThread);  
 
        DBGPRINT(RT_DEBUG_TRACE, ("Terminate [DequeueData] Thread Ok\n"));    
    }
}

VOID 
NdisCommonCreateAllWorkitems(
    IN  PMP_ADAPTER   pAd
    )
{
    FUNC_ENTER;
    
   // PlatformInitWorktiem(pAd, &pAd->ScanTimeoutWorkitem, MlmeSyncMlmeSyncScanTimeoutActionWorkitemCallBack, "MlmeSyncScanTimeoutActionWorkitem");
    
    //PlatformInitWorktiem(pAd, &pAd->AsicSwitchChannelWorkitem, AsicSwitchChannelWorkitemCallback, "AsicSwitchChannelWorkitem");
    
    FUNC_LEAVE;
}


VOID 
NdisCommonFreeAllWorkitems(
    IN  PMP_ADAPTER   pAd
    )
{
    FUNC_ENTER;
   // PlatformFreeWorkitem(&pAd->ScanTimeoutWorkitem);
   // PlatformFreeWorkitem(&pAd->AsicSwitchChannelWorkitem);
    FUNC_LEAVE;
}

/*
    ========================================================================

    Routine Description:
        A generic fucntion for delay

    Arguments:
        usec                            delay usec

    Return Value:
        None

    IRQL = PASSIVE_LEVEL OR DISPATCH_LEVEL

    Note:

    ========================================================================
*/
VOID NdisCommonGenericDelay(
    IN      ULONG   usec)
{
    if (KeGetCurrentIrql() == PASSIVE_LEVEL)        
    {
        NdisMSleep(usec);
    }
    else
    {
        Delay_us(usec); 
    }   
}

NTSTATUS
NdisCommonCreateTxDataThread(
    IN  PMP_ADAPTER   pAd
    )
{
    NTSTATUS    Status = NDIS_STATUS_SUCCESS;
#if IS_BUS_TYPE_USB(DEV_BUS_TYPE)
    //
    // Create a BulkOutDataThread to run at PASSIVE_LEVEL.
    //  
    PlatformInitThread(pAd, &pAd->pHifCfg->hBulkOutDataThread[0], (MT_THREAD_CALL_BACK)USBBulkOutDataThread0, "USBBulkOutDataThread0");
    Status = PlatformRunThread(pAd, &pAd->pHifCfg->hBulkOutDataThread[0]);
    
    if (Status != STATUS_SUCCESS)
    {
        DBGPRINT(RT_DEBUG_TRACE, ("NdisCommonCreateAllThreads fail  (hBulkOutDataThread 0) Status= 0x%x\n",Status));
        Status = NDIS_STATUS_FAILURE;
        return Status;
    }

    //
    // Create a BulkOutDataThread to run at PASSIVE_LEVEL.
    //          
    PlatformInitThread(pAd, &pAd->pHifCfg->hBulkOutDataThread[1], (MT_THREAD_CALL_BACK)USBBulkOutDataThread1, "USBBulkOutDataThread1");
    Status = PlatformRunThread(pAd, &pAd->pHifCfg->hBulkOutDataThread[1]);
    
    if (Status != STATUS_SUCCESS)
    {
        DBGPRINT(RT_DEBUG_TRACE, ("NdisCommonCreateAllThreads fail  (hBulkOutDataThread 1) Status= 0x%x\n",Status));
        Status = NDIS_STATUS_FAILURE;
        return Status;
    }

    //
    // Create a BulkOutDataThread to run at PASSIVE_LEVEL.
    //          
    PlatformInitThread(pAd, &pAd->pHifCfg->hBulkOutDataThread[2], (MT_THREAD_CALL_BACK)USBBulkOutDataThread2, "USBBulkOutDataThread2");
    Status = PlatformRunThread(pAd, &pAd->pHifCfg->hBulkOutDataThread[2]);
    
    if (Status != STATUS_SUCCESS)
    {
        DBGPRINT(RT_DEBUG_TRACE, ("NdisCommonCreateAllThreads fail  (hBulkOutDataThread 2) Status= 0x%x\n",Status));
        Status = NDIS_STATUS_FAILURE;
        return Status;
    }

    //
    // Create a BulkOutDataThread to run at PASSIVE_LEVEL.
    //          
    PlatformInitThread(pAd, &pAd->pHifCfg->hBulkOutDataThread[3], (MT_THREAD_CALL_BACK)USBBulkOutDataThread3, "USBBulkOutDataThread3");
    Status = PlatformRunThread(pAd, &pAd->pHifCfg->hBulkOutDataThread[3]);
    
    if (Status != STATUS_SUCCESS)
    {
        DBGPRINT(RT_DEBUG_TRACE, ("NdisCommonCreateAllThreads fail  (hBulkOutDataThread 3) Status= 0x%x\n",Status));
        Status = NDIS_STATUS_FAILURE;
        return Status;
    }  

    //
    // Create a DequeueDataThread to run at PASSIVE_LEVEL.
    //
    PlatformInitThread(pAd, &pAd->pHifCfg->hDequeueDataThread, (MT_THREAD_CALL_BACK)DequeueDataThread, "DequeueDataThread");
    Status = PlatformRunThread(pAd, &pAd->pHifCfg->hDequeueDataThread);
    
    if (Status != STATUS_SUCCESS)
    {
        DBGPRINT(RT_DEBUG_TRACE, ("NdisCommonCreateAllThreads fail  (hDequeueDataThread) Status= 0x%x\n",Status));
        Status = NDIS_STATUS_FAILURE;
        return Status;
    }  
    
#endif

    return Status;
}


#if IS_BUS_TYPE_USB(DEV_BUS_TYPE)
/*
    ==========================================================================
    Description:
        A thread for calling MlmeHandler
    Parameters:
        Context - NIC Adapter pointer
    Note:

    IRQL = PASSIVE_LEVEL
    
    ==========================================================================
 */
VOID USBBulkOutDataThread0(
    IN PMP_ADAPTER       pAd
    )
{
    DBGPRINT(RT_DEBUG_TRACE, ("BulkOutData Thread 0 Start\n"));

    KeSetPriorityThread(KeGetCurrentThread(), LOW_REALTIME_PRIORITY);

    ASSERT(KeGetCurrentIrql() == PASSIVE_LEVEL);        

    while (pAd->TerminateThreads != TRUE)
    {
        PlatformWaitEventTriggeredAndReset(&(GET_THREAD_EVENT(&pAd->pHifCfg->hBulkOutDataThread[0])),0);   

        DBGPRINT(RT_DEBUG_INFO, ("BulkOutData Thread 0 Triggered\n"));

        if (!(MT_TEST_FLAG(pAd, fRTMP_ADAPTER_NIC_NOT_EXIST)) &&
            !(MT_TEST_FLAG(pAd, fRTMP_ADAPTER_HALT_IN_PROGRESS)) &&
            !(MT_TEST_FLAG(pAd, fRTMP_ADAPTER_RESET_IN_PROGRESS)) &&
            !(MT_TEST_FLAG(pAd, fRTMP_ADAPTER_BULKOUT_RESET)) &&
            !(MT_TEST_FLAG(pAd, fRTMP_ADAPTER_RADIO_OFF)) &&
            !MT_TEST_FLAG(pAd, fRTMP_ADAPTER_SLEEP_IN_PROGRESS))
        {
            N6USBBulkOutDataPacket(pAd, 0, pAd->pHifCfg->NextBulkOutIndex[0]);
        }
    }   
    DBGPRINT(RT_DEBUG_TRACE, ("BulkOutData Thread 0 Terminated\n"));
}

/*
    ==========================================================================
    Description:
        A thread for calling MlmeHandler
    Parameters:
        Context - NIC Adapter pointer
    Note:

    IRQL = PASSIVE_LEVEL
    
    ==========================================================================
 */
VOID USBBulkOutDataThread1(
    IN PMP_ADAPTER       pAd
    )
{
    DBGPRINT(RT_DEBUG_TRACE, ("BulkOutData Thread 1 Start\n"));

    KeSetPriorityThread(KeGetCurrentThread(), LOW_REALTIME_PRIORITY);

    ASSERT(KeGetCurrentIrql() == PASSIVE_LEVEL);        

    while (pAd->TerminateThreads != TRUE)
    {
        PlatformWaitEventTriggeredAndReset(&(GET_THREAD_EVENT(&pAd->pHifCfg->hBulkOutDataThread[1])),0);   

        DBGPRINT(RT_DEBUG_INFO, ("BulkOutData Thread 1 Triggered\n"));

        if (!(MT_TEST_FLAG(pAd, fRTMP_ADAPTER_NIC_NOT_EXIST)) &&
            !(MT_TEST_FLAG(pAd, fRTMP_ADAPTER_HALT_IN_PROGRESS)) &&
            !(MT_TEST_FLAG(pAd, fRTMP_ADAPTER_RESET_IN_PROGRESS)) &&
            !(MT_TEST_FLAG(pAd, fRTMP_ADAPTER_BULKOUT_RESET)) &&
            !(MT_TEST_FLAG(pAd, fRTMP_ADAPTER_RADIO_OFF)) &&
            !MT_TEST_FLAG(pAd, fRTMP_ADAPTER_SLEEP_IN_PROGRESS))
        {
            N6USBBulkOutDataPacket(pAd, 1, pAd->pHifCfg->NextBulkOutIndex[1]);
        }
    }   
    DBGPRINT(RT_DEBUG_TRACE, ("BulkOutData Thread 1 Terminated\n"));
}

/*
    ==========================================================================
    Description:
        A thread for calling MlmeHandler
    Parameters:
        Context - NIC Adapter pointer
    Note:

    IRQL = PASSIVE_LEVEL
    
    ==========================================================================
 */
VOID USBBulkOutDataThread2(
    IN PMP_ADAPTER       pAd
    )
{
    DBGPRINT(RT_DEBUG_TRACE, ("BulkOutData Thread 2 Start\n"));

    KeSetPriorityThread(KeGetCurrentThread(), LOW_REALTIME_PRIORITY);

    ASSERT(KeGetCurrentIrql() == PASSIVE_LEVEL);        

    while (pAd->TerminateThreads != TRUE)
    {
        PlatformWaitEventTriggeredAndReset(&(GET_THREAD_EVENT(&pAd->pHifCfg->hBulkOutDataThread[2])),0);   

        DBGPRINT(RT_DEBUG_INFO, ("BulkOutData Thread 2 Triggered\n"));

        if (!(MT_TEST_FLAG(pAd, fRTMP_ADAPTER_NIC_NOT_EXIST)) &&
            !(MT_TEST_FLAG(pAd, fRTMP_ADAPTER_HALT_IN_PROGRESS)) &&
            !(MT_TEST_FLAG(pAd, fRTMP_ADAPTER_RESET_IN_PROGRESS)) &&
            !(MT_TEST_FLAG(pAd, fRTMP_ADAPTER_BULKOUT_RESET)) &&
            !(MT_TEST_FLAG(pAd, fRTMP_ADAPTER_RADIO_OFF)) &&
            !MT_TEST_FLAG(pAd, fRTMP_ADAPTER_SLEEP_IN_PROGRESS))
        {
            N6USBBulkOutDataPacket(pAd, 2, pAd->pHifCfg->NextBulkOutIndex[2]);
        }
    }   
    DBGPRINT(RT_DEBUG_TRACE, ("BulkOutData Thread 2 Terminated\n"));
}

/*
    ==========================================================================
    Description:
        A thread for calling MlmeHandler
    Parameters:
        Context - NIC Adapter pointer
    Note:

    IRQL = PASSIVE_LEVEL
    
    ==========================================================================
 */
VOID USBBulkOutDataThread3(
    IN PMP_ADAPTER       pAd
    )
{
    DBGPRINT(RT_DEBUG_TRACE, ("BulkOutData Thread 3 Start\n"));

    KeSetPriorityThread(KeGetCurrentThread(), LOW_REALTIME_PRIORITY);

    ASSERT(KeGetCurrentIrql() == PASSIVE_LEVEL);        

    while (pAd->TerminateThreads != TRUE)
    {
        PlatformWaitEventTriggeredAndReset(&(GET_THREAD_EVENT(&pAd->pHifCfg->hBulkOutDataThread[3])),0);   

        DBGPRINT(RT_DEBUG_INFO, ("BulkOutData Thread 3 Triggered\n"));

        if (!(MT_TEST_FLAG(pAd, fRTMP_ADAPTER_NIC_NOT_EXIST)) &&
            !(MT_TEST_FLAG(pAd, fRTMP_ADAPTER_HALT_IN_PROGRESS)) &&
            !(MT_TEST_FLAG(pAd, fRTMP_ADAPTER_RESET_IN_PROGRESS)) &&
            !(MT_TEST_FLAG(pAd, fRTMP_ADAPTER_BULKOUT_RESET)) &&
            !(MT_TEST_FLAG(pAd, fRTMP_ADAPTER_RADIO_OFF)) &&
            !MT_TEST_FLAG(pAd, fRTMP_ADAPTER_SLEEP_IN_PROGRESS))
        {
            N6USBBulkOutDataPacket(pAd, 3, pAd->pHifCfg->NextBulkOutIndex[3]);
        }
    }   
    DBGPRINT(RT_DEBUG_TRACE, ("BulkOutData Thread 3 Terminated\n"));
}

/*
    ==========================================================================
    Description:
        A thread for calling MlmeHandler
    Parameters:
        Context - NIC Adapter pointer
    Note:

    IRQL = PASSIVE_LEVEL
    
    ==========================================================================
 */
VOID DequeueDataThread(
    IN PMP_ADAPTER       pAd
    )
{
    UCHAR   Index;
    DBGPRINT(RT_DEBUG_TRACE, ("DequeueData Thread Start\n"));

    KeSetPriorityThread(KeGetCurrentThread(), LOW_REALTIME_PRIORITY);

    ASSERT(KeGetCurrentIrql() == PASSIVE_LEVEL);        

    while (pAd->TerminateThreads != TRUE)
    {
        PlatformWaitEventTriggeredAndReset(&(GET_THREAD_EVENT(&pAd->pHifCfg->hDequeueDataThread)),0);   

        DBGPRINT(RT_DEBUG_INFO, ("DequeueData Thread Triggered\n"));

        for (Index = 0; Index < 4; Index++)
        {
            if(pAd->pTxCfg->SendTxWaitQueue[Index].Number > 0)
                NdisCommonDeQueuePacket(pAd, Index);
        }

        // Kick bulk out
        N6USBKickBulkOut(pAd);
    }   
    DBGPRINT(RT_DEBUG_TRACE, ("DequeueData Thread Terminated\n"));
}

#endif


NDIS_STATUS
NdisCommonStartTxRx(
    IN PMP_ADAPTER pAd,
    OUT PADAPTER_INITIAL_STAGE  pInitialStage
    )
{
    NDIS_STATUS     NdisStatus = NDIS_STATUS_SUCCESS;
     // Before Load firmware, need to start N6UsbXmitStart & N6UsbRecvStart first.
    NdisStatus = NdisCommonStartTx(pAd);

    if (NdisStatus != NDIS_STATUS_SUCCESS)
    {
        DBGPRINT(RT_DEBUG_ERROR, ("%s   NdisCommonStartTx   Failed", __FUNCTION__));
        return NdisStatus;
    }

    pInitialStage->bEnableXmit = TRUE;

    NdisStatus = NdisCommonStartRx(pAd);

     if (NdisStatus != NDIS_STATUS_SUCCESS)
    {   
        DBGPRINT(RT_DEBUG_ERROR, ("%s   NdisCommonStartRx   Failed", __FUNCTION__));
        return NdisStatus;
    }

    pInitialStage->bEnableRecv = TRUE;


    return NdisStatus;

}


NDIS_STATUS
NdisCommonStartRx(
    IN PMP_ADAPTER pAd
    )
{
    NDIS_STATUS     ndisStatus = NDIS_STATUS_SUCCESS;
#if IS_BUS_TYPE_USB(DEV_BUS_TYPE)
    UCHAR           BulkInIndex = 0;
    PMP_PORT    pPort = pAd->PortList[FXXK_PORT_0];

    ndisStatus = N6UsbRecvStart(pAd);

    for (BulkInIndex = 0; BulkInIndex < pAd->pNicCfg->NumOfBulkInIRP; BulkInIndex++ )
    {
        NdisCommonBulkRx(pAd);
    }

    NdisCommonBulkRxCmd(pAd);
#elif IS_BUS_TYPE_PCI(DEV_BUS_TYPE)
    // TODO:
#elif IS_BUS_TYPE_SDIO(DEV_BUS_TYPE)    
    // TODO:
#endif

    return ndisStatus;
}

NDIS_STATUS
NdisCommonStartTx(
    IN PMP_ADAPTER pAd
    )
{
    NDIS_STATUS     ndisStatus = NDIS_STATUS_SUCCESS;
#if IS_BUS_TYPE_USB(DEV_BUS_TYPE)
    ndisStatus = NdisCommonUsbStartTx(pAd);
#elif IS_BUS_TYPE_PCI(DEV_BUS_TYPE)
    // TODO:
#elif IS_BUS_TYPE_SDIO(DEV_BUS_TYPE)    
    // TODO:
#endif
    return ndisStatus;
}

VOID
NdisCommonStopTx(
    IN PMP_ADAPTER pAd
    )
{
#if IS_BUS_TYPE_USB(DEV_BUS_TYPE)
    NdisCommonUsbStopTx(pAd);
#elif IS_BUS_TYPE_PCI(DEV_BUS_TYPE)
    // TODO:
#elif IS_BUS_TYPE_SDIO(DEV_BUS_TYPE)    
    // TODO:
#endif
}

VOID
NdisCommonStopRx(
    IN PMP_ADAPTER pAd
    )
{
#if IS_BUS_TYPE_USB(DEV_BUS_TYPE)
    NdisCommonUsbStopRx(pAd);
#elif IS_BUS_TYPE_PCI(DEV_BUS_TYPE)
    // TODO:
#elif IS_BUS_TYPE_SDIO(DEV_BUS_TYPE)    
    // TODO:
#endif
}

VOID
NdisCommonBulkRx(
    IN PMP_ADAPTER pAd
    )
{
#if IS_BUS_TYPE_USB(DEV_BUS_TYPE)
    NdisCommonUsbBulkRx(pAd);
#elif IS_BUS_TYPE_PCI(DEV_BUS_TYPE)
    // TODO:
#elif IS_BUS_TYPE_SDIO(DEV_BUS_TYPE)    
    // TODO:
#endif
}

VOID
NdisCommonBulkRxCmd(
    IN PMP_ADAPTER pAd
    )
{
#if IS_BUS_TYPE_USB(DEV_BUS_TYPE)
    NdisCommonUsbBulkRxCmd(pAd);
#elif IS_BUS_TYPE_PCI(DEV_BUS_TYPE)
    // TODO:
#elif IS_BUS_TYPE_SDIO(DEV_BUS_TYPE)    
    // TODO:
#endif
}

NDIS_STATUS
NdisCommonInitTransmit(
    IN PMP_ADAPTER pAd
    )
{
    NDIS_STATUS     ndisStatus = NDIS_STATUS_SUCCESS;
#if IS_BUS_TYPE_USB(DEV_BUS_TYPE)
    ndisStatus = NdisCommonUsbInitTransmit(pAd);
#elif IS_BUS_TYPE_PCI(DEV_BUS_TYPE)
    // TODO:
#elif IS_BUS_TYPE_SDIO(DEV_BUS_TYPE)    
    // TODO:
#endif
    return ndisStatus;
}

NDIS_STATUS
NdisCommonInitRecv(
    IN PMP_ADAPTER pAd
    )
{
    NDIS_STATUS     ndisStatus = NDIS_STATUS_SUCCESS;
#if IS_BUS_TYPE_USB(DEV_BUS_TYPE)
    ndisStatus = NdisCommonUsbInitRecv(pAd);
#elif IS_BUS_TYPE_PCI(DEV_BUS_TYPE)
    // TODO:
#elif IS_BUS_TYPE_SDIO(DEV_BUS_TYPE)    
    // TODO:
#endif
    return ndisStatus;
}

VOID
NdisCommonFreeNicRecv(
    IN PMP_ADAPTER pAd
    )
{
#if IS_BUS_TYPE_USB(DEV_BUS_TYPE)
    NdisCommonUsbFreeNicRecv(pAd);
#elif IS_BUS_TYPE_PCI(DEV_BUS_TYPE)
    // TODO:
#elif IS_BUS_TYPE_SDIO(DEV_BUS_TYPE)    
    // TODO:
#endif
}

VOID
NdisCommonFreeNICTransmit(
    IN PMP_ADAPTER pAd
    )
{
#if IS_BUS_TYPE_USB(DEV_BUS_TYPE)
    NdisCommonUsbFreeNICTransmit(pAd);
#elif IS_BUS_TYPE_PCI(DEV_BUS_TYPE)
    // TODO:
#elif IS_BUS_TYPE_SDIO(DEV_BUS_TYPE)    
    // TODO:
#endif
}

VOID
NdisCommonCleanUpMLMEWaitQueue(
    IN PMP_ADAPTER pAd
    )
{
#if IS_BUS_TYPE_USB(DEV_BUS_TYPE)
    NdisCommonUsbCleanUpMLMEWaitQueue(pAd);
#elif IS_BUS_TYPE_PCI(DEV_BUS_TYPE)
    // TODO:
#elif IS_BUS_TYPE_SDIO(DEV_BUS_TYPE)    
    // TODO:
#endif
}

VOID
NdisCommonCleanUpMLMEBulkOutQueue(
    IN PMP_ADAPTER pAd
    )
{
#if IS_BUS_TYPE_USB(DEV_BUS_TYPE)
    NdisCommonUsbCleanUpMLMEBulkOutQueue(pAd);
#elif IS_BUS_TYPE_PCI(DEV_BUS_TYPE)
    // TODO:
#elif IS_BUS_TYPE_SDIO(DEV_BUS_TYPE)    
    // TODO: 
#endif
}

VOID
NdisCommonWaitPendingRequest(
    IN PMP_ADAPTER pAd
    )
{
#if IS_BUS_TYPE_USB(DEV_BUS_TYPE)
    N6USBWaitPendingRequest(pAd);
#elif IS_BUS_TYPE_PCI(DEV_BUS_TYPE)
    // TODO:
#elif IS_BUS_TYPE_SDIO(DEV_BUS_TYPE)    
    // TODO:
#endif    
}

VOID
NdisCommonDeQueuePacket(
    IN  PMP_ADAPTER   pAd,    
    IN  UCHAR           BulkOutPipeId
    )
{
#if IS_BUS_TYPE_USB(DEV_BUS_TYPE)
    NdisCommonUsbDeQueuePacket(pAd, BulkOutPipeId);
#elif IS_BUS_TYPE_PCI(DEV_BUS_TYPE)
    // TODO:
#elif IS_BUS_TYPE_SDIO(DEV_BUS_TYPE)    
    // TODO:
#endif
    
}

NDIS_STATUS 
NdisCommonFreeDescriptorRequest(
    IN  PMP_ADAPTER   pAd,
    IN  UCHAR           RingType,
    IN  UCHAR           BulkOutPipeId,
    IN  ULONG           NumberRequired
    )
{
    NDIS_STATUS     ndisStatus = NDIS_STATUS_SUCCESS;
#if IS_BUS_TYPE_USB(DEV_BUS_TYPE)
    ndisStatus = NdisCommonUsbFreeDescriptorRequest(pAd, RingType, BulkOutPipeId, NumberRequired);
#elif IS_BUS_TYPE_PCI(DEV_BUS_TYPE)
    // TODO:
#elif IS_BUS_TYPE_SDIO(DEV_BUS_TYPE)    
    // TODO:
#endif    
    return ndisStatus;
}

VOID    
NdisCommonMlmeHardTransmit(
    IN  PMP_ADAPTER   pAd,
    IN  PMGMT_STRUC     pMgmt
    )
{
#if IS_BUS_TYPE_USB(DEV_BUS_TYPE)
    NdisCommonUsbMlmeHardTransmit(pAd, pMgmt);
#elif IS_BUS_TYPE_PCI(DEV_BUS_TYPE)
    // TODO:
#elif IS_BUS_TYPE_SDIO(DEV_BUS_TYPE)    
    // TODO:
#endif    
}

VOID    
NdisCommonDequeueMLMEPacket(
    IN  PMP_ADAPTER   pAd
    )  
{
#if IS_BUS_TYPE_USB(DEV_BUS_TYPE)
    NdisCommonUsbDequeueMLMEPacket(pAd);
#elif IS_BUS_TYPE_PCI(DEV_BUS_TYPE)
    // TODO:
#elif IS_BUS_TYPE_SDIO(DEV_BUS_TYPE)    
    // TODO:
#endif
}   

VOID    
NdisCommonMiniportMMRequest(
    IN  PMP_ADAPTER   pAd,    
    IN  PVOID           pBuffer,
    IN  ULONG           Length
    )
{
#if IS_BUS_TYPE_USB(DEV_BUS_TYPE)
    NdisCommonUsbMiniportMMRequest(pAd, pBuffer, Length);
#elif IS_BUS_TYPE_PCI(DEV_BUS_TYPE)
    // TODO:
#elif IS_BUS_TYPE_SDIO(DEV_BUS_TYPE)    
    // TODO:
#endif    
}    

VOID    
NdisCommonRejectPendingPackets(
    IN  PMP_ADAPTER   pAd
    )
{
 #if (COMPILE_VISTA_ABOVE(CURRENT_OS_NDIS_VER))
    Ndis6CommonRejectPendingPackets(pAd);
#elif IS_BUS_TYPE_PCI(DEV_BUS_TYPE)
    // TODO:
#elif IS_BUS_TYPE_SDIO(DEV_BUS_TYPE)    
    // TODO:
#endif 
}

NDIS_STATUS
NdisCommonPortInitandCfg(
    IN  PMP_ADAPTER pAd
    )
{
    NDIS_STATUS NdisStatus = NDIS_STATUS_SUCCESS;
 #if (COMPILE_VISTA_ABOVE(CURRENT_OS_NDIS_VER))
    NdisStatus = N6PortInitandCfg(pAd);
 #endif  
    return NdisStatus;
}

NDIS_STATUS
NdisCommonInitializeSWCtrl (
    IN  PMP_ADAPTER pAd,
    OUT PADAPTER_INITIAL_STAGE  pInitialStage
    )
 {
    UCHAR i = 0;
    NDIS_STATUS NdisStatus = NDIS_STATUS_SUCCESS;
    
    NdisCommonPortInitandCfg(pAd);
        
    UserCfgInit(pAd);   
    APInitialize(pAd);

    //
    // For Reposit Bssid List Scan test
    //
    pAd->pNicCfg->RepositBssTable.pBssEntry = NULL;
    pAd->pNicCfg->RepositBssTable.BssNr = 0;

    NdisStatus = PlatformAllocateMemory(pAd, (VOID *) &pAd->pNicCfg->RepositBssTable.pBssEntry, MAX_LEN_OF_BSS_TABLE*sizeof(BSS_ENTRY));

    if(NdisStatus != NDIS_STATUS_SUCCESS)
    {
        DBGPRINT_RAW(RT_DEBUG_WARN,("**** Can not allocate memory for Reposit BssEntry \n"));
    }
    else
    {
        for (i = 0; i < MAX_LEN_OF_BSS_TABLE; i++) 
        {
            PlatformZeroMemory(&pAd->pNicCfg->RepositBssTable.pBssEntry[i], sizeof(BSS_ENTRY));
        }

        DBGPRINT(RT_DEBUG_TRACE, ("pAd->pNicCfg->RepositBssTable.pBssEntry initialize successfully\n"));
        
        for(i = 0; i < MAX_LEN_OF_BSS_TABLE; i++)
        {
            pAd->pNicCfg->RepositBssTable.ulTime[i].LowPart = 0;
            pAd->pNicCfg->RepositBssTable.ulTime[i].HighPart = 0;
        }
    }
    
    pAd->pNicCfg->TempRepositBssTable.pBssEntry = NULL;

    NdisStatus = PlatformAllocateMemory(pAd, (VOID *) &pAd->pNicCfg->TempRepositBssTable.pBssEntry, MAX_LEN_OF_BSS_TABLE*sizeof(BSS_ENTRY));

    if(NdisStatus != NDIS_STATUS_SUCCESS)
    {
        DBGPRINT_RAW(RT_DEBUG_WARN,("**** Can not allocate memory for Reposit BssEntry \n"));
    }
    

    MlmeSyncInitMacTabPool(pAd);
    MlmeSyncInitMacTab(pAd->PortList[0]);    
     //
    // Create Thread
    //
    NdisStatus = NdisCommonCreateAllThreads(pAd);
    if (NdisStatus != NDIS_STATUS_SUCCESS)
    {
        DBGPRINT(RT_DEBUG_ERROR, ("NdisCommonCreateAllThreads failed\n"));
        return NdisStatus;
    }       

    return  NdisStatus;
 }

VOID    
NdisCommonAllocateAllSpinLock(
    IN PMP_ADAPTER pAd
    )
{
    UCHAR       index = 0;
    
    NdisInitializeEvent(&pAd->HwCfg.WaitFWEvent);
    NdisInitializeEvent(&pAd->HwCfg.LoadFWEvent);
    ////////////////////////
    // Spinlock
    NdisAllocateSpinLock(&pAd->pRxCfg->IndicateRxPktsQueueLock);
    NdisAllocateSpinLock(&pAd->pTxCfg->LocalNetBufferListQueueLock);
    NdisAllocateSpinLock(&pAd->pHifCfg->MLMEQLock);      
    NdisAllocateSpinLock(&pAd->pHifCfg->BulkOutMlmeLock);
    NdisAllocateSpinLock(&pAd->pHifCfg->BulkInLock);
    NdisAllocateSpinLock(&pAd->pHifCfg->BulkInReadLock);
    NdisAllocateSpinLock(&pAd->pHifCfg->ControlLock);
    NdisAllocateSpinLock(&pAd->pHifCfg->BBPControlLock);     
    NdisAllocateSpinLock(&pAd->pHifCfg->CmdQLock);
    NdisAllocateSpinLock(&pAd->pHifCfg->PktCmdLock);
    NdisAllocateSpinLock(&pAd->pP2pCtrll->P2PCmdQLock);   
    NdisAllocateSpinLock(&pAd->MccCfg.MultiChannelCmdQLock);           
    NdisAllocateSpinLock(&pAd->pHifCfg->DeMGMTQueueLock);
    NdisAllocateSpinLock(&pAd->HwCfg.RadioStatusChangeLock);
    NdisAllocateSpinLock(&pAd->pP2pCtrll->TxSwNoAMgmtQueueLock);
    NdisAllocateSpinLock(& pAd->pNicCfg->PendingOIDLock);
    NdisAllocateSpinLock(&pAd->pTxCfg->SendTxWaitQueueLock[0]);
    NdisAllocateSpinLock(&pAd->pTxCfg->SendTxWaitQueueLock[1]);
    NdisAllocateSpinLock(&pAd->pTxCfg->SendTxWaitQueueLock[2]);
    NdisAllocateSpinLock(&pAd->pTxCfg->SendTxWaitQueueLock[3]);
    NdisAllocateSpinLock(&pAd->pTxCfg->TxContextQueueLock[0]);
    NdisAllocateSpinLock(&pAd->pTxCfg->TxContextQueueLock[1]);
    NdisAllocateSpinLock(&pAd->pTxCfg->TxContextQueueLock[2]);
    NdisAllocateSpinLock(&pAd->pTxCfg->TxContextQueueLock[3]);
    NdisAllocateSpinLock(&pAd->pTxCfg->DeQueueLock[0]);
    NdisAllocateSpinLock(&pAd->pTxCfg->DeQueueLock[1]);
    NdisAllocateSpinLock(&pAd->pTxCfg->DeQueueLock[2]);
    NdisAllocateSpinLock(&pAd->pTxCfg->DeQueueLock[3]);
    NdisAllocateSpinLock(&pAd->pTxCfg->XcuIdelQueueLock);
    
#if UAPSD_AP_SUPPORT
    NdisAllocateSpinLock(&pAd->UAPSD.UAPSDEOSPLock);
    NdisAllocateSpinLock(&pAd->UAPSD.TxSwUapsdQueueLock[0]);
    NdisAllocateSpinLock(&pAd->UAPSD.TxSwUapsdQueueLock[1]);
    NdisAllocateSpinLock(&pAd->UAPSD.TxSwUapsdQueueLock[2]);
    NdisAllocateSpinLock(&pAd->UAPSD.TxSwUapsdQueueLock[3]);      
#endif
    NdisAllocateSpinLock(&pAd->pHifCfg->MLMEWaitQueueLock);
    NdisAllocateSpinLock(&pAd->MemLock);        
    //NdisAllocateSpinLock(&pAd->MacTablePool.MacTabPoolLock);
     NdisInitializeReadWriteLock(&pAd->MacTablePool.MacTabPoolLock);    
     NdisInitializeReadWriteLock(&pAd->pTxCfg->PsQueueLock);    
    NdisAllocateSpinLock(&pAd->BATabLock);

    NdisAllocateSpinLock(&pAd->AdapterStateLock);
    NdisAllocateSpinLock(&pAd->StaCfg.AdhocListLock);       

    NdisAllocateSpinLock(&pAd->pNicCfg->PrivateInfo.CalMicLock);

    for (index = 0; index < RTMP_MAX_NUMBER_OF_PORT; index ++)
    {
        NdisAllocateSpinLock(&pAd->pRxCfg->IndicateRxPktsLock[index]);
    }

    for (index = 0; index < 6; index ++)
    {
        NdisAllocateSpinLock(&pAd->pHifCfg->MT7603EPLock[index]);
    }
}

VOID    
NdisCommonDeallocateAllSpinLock(
    IN PMP_ADAPTER pAd
    )
{
    UCHAR       index = 0;
    
    if (pAd == NULL)
    {
        DBGPRINT(RT_DEBUG_ERROR, ("%s: pAd is NULL\n", __FUNCTION__));  
        return;
    }

    UNREFERENCED_PARAMETER(pAd);    
    NdisFreeSpinLock(&pAd->StaCfg.AdhocListLock);
    NdisFreeSpinLock(&pAd->AdapterStateLock);
    NdisFreeSpinLock(&pAd->MemLock);
    NdisFreeSpinLock(&pAd->BATabLock);
    NdisFreeSpinLock(&pAd->MccCfg.MultiChannelCmdQLock);   
    NdisFreeSpinLock(&pAd->HwCfg.RadioStatusChangeLock);
    NdisFreeSpinLock(&pAd->Mlme.MlmeInternalCmdBufLock);

#if UAPSD_AP_SUPPORT
    NdisFreeSpinLock(&pAd->UAPSD.UAPSDEOSPLock);
    NdisFreeSpinLock(&pAd->UAPSD.TxSwUapsdQueueLock[0]);
    NdisFreeSpinLock(&pAd->UAPSD.TxSwUapsdQueueLock[1]);
    NdisFreeSpinLock(&pAd->UAPSD.TxSwUapsdQueueLock[2]);
    NdisFreeSpinLock(&pAd->UAPSD.TxSwUapsdQueueLock[3]);
#endif

#if _WIN8_USB_SS_SUPPORTED
    //Free spinlock
    NdisFreeSpinLock(&pAd->Ss.SsLock);
#endif

    if (pAd->pTxCfg != NULL)
    {
        NdisFreeSpinLock(&pAd->pTxCfg->DeQueueLock[3]);
        NdisFreeSpinLock(&pAd->pTxCfg->DeQueueLock[2]);
        NdisFreeSpinLock(&pAd->pTxCfg->DeQueueLock[1]);
        NdisFreeSpinLock(&pAd->pTxCfg->DeQueueLock[0]);
        NdisFreeSpinLock(&pAd->pTxCfg->XcuIdelQueueLock);
        NdisFreeSpinLock(&pAd->pTxCfg->SendTxWaitQueueLock[3]);
        NdisFreeSpinLock(&pAd->pTxCfg->SendTxWaitQueueLock[2]);
        NdisFreeSpinLock(&pAd->pTxCfg->SendTxWaitQueueLock[1]);
        NdisFreeSpinLock(&pAd->pTxCfg->SendTxWaitQueueLock[0]);
        NdisFreeSpinLock(&pAd->pTxCfg->TxContextQueueLock[3]);
        NdisFreeSpinLock(&pAd->pTxCfg->TxContextQueueLock[2]);
        NdisFreeSpinLock(&pAd->pTxCfg->TxContextQueueLock[1]);
        NdisFreeSpinLock(&pAd->pTxCfg->TxContextQueueLock[0]);
        NdisFreeSpinLock(&pAd->pTxCfg->LocalNetBufferListQueueLock);
    }

    if (pAd->pHifCfg != NULL)
    {
        NdisFreeSpinLock(&pAd->pHifCfg->MLMEWaitQueueLock);
        NdisFreeSpinLock(&pAd->pHifCfg->DeMGMTQueueLock);
        NdisFreeSpinLock(&pAd->pHifCfg->CmdQLock);   
        NdisFreeSpinLock(&pAd->pHifCfg->ControlLock);
        NdisFreeSpinLock(&pAd->pHifCfg->BBPControlLock); 
        NdisFreeSpinLock(&pAd->pHifCfg->BulkInLock);
        NdisFreeSpinLock(&pAd->pHifCfg->BulkInReadLock);
        NdisFreeSpinLock(&pAd->pHifCfg->BulkOutMlmeLock);    
        NdisFreeSpinLock(&pAd->pHifCfg->MLMEQLock);  
        NdisFreeSpinLock(&pAd->pHifCfg->PktCmdLock);
        
        for (index = 0; index < 6; index ++)
        {
            NdisFreeSpinLock(&pAd->pHifCfg->MT7603EPLock[index]);
        }
    }

    if (pAd->pP2pCtrll != NULL)
    {
        NdisFreeSpinLock(&pAd->pP2pCtrll->P2PCmdQLock);   
        NdisFreeSpinLock(&pAd->pP2pCtrll->TxSwNoAMgmtQueueLock);
    }

    if (pAd->pNicCfg != NULL)
    {
        NdisFreeSpinLock(&pAd->pNicCfg->PrivateInfo.CalMicLock); 
    }

    if (pAd->pNicCfg != NULL)
    {
        NdisFreeSpinLock(&pAd->pNicCfg->PendingOIDLock);
    }    

    if (pAd->pRxCfg != NULL)
    {
        NdisFreeSpinLock(&pAd->pRxCfg->IndicateRxPktsQueueLock);
        
        for (index = 0; index < RTMP_MAX_NUMBER_OF_PORT; index ++)
        {
            NdisFreeSpinLock(&pAd->pRxCfg->IndicateRxPktsLock[index]);
        }
    }   
}

VOID 
NdisCommonInfoResetHandler(
    IN  PMP_PORT pPort
    )
{
    PMP_ADAPTER               pAd = pPort->pAd;
    PMLME_QUEUE_ELEM            pMsgElem = NULL;
    MLME_DISASSOC_REQ_STRUCT    DisReq;
    UCHAR   Wcid = (UCHAR)MlmeSyncGetWlanIdxByPort(pPort, ROLE_WLANIDX_BSSID);

    // Send disassoc frame      
    if (INFRA_ON(pPort)) 
    {
        COPY_MAC_ADDR(&DisReq.Addr, pPort->PortCfg.Bssid);
        DisReq.Reason =  REASON_DISASSOC_STA_LEAVING;

       PlatformAllocateMemory(pAd,  &pMsgElem, sizeof(MLME_QUEUE_ELEM));
        if (pMsgElem)
        {
            pMsgElem->Machine = ASSOC_STATE_MACHINE;
            pMsgElem->MsgType = MT2_MLME_DISASSOC_REQ;
            pMsgElem->PortNum = pPort->PortNumber;
            pMsgElem->MsgLen = sizeof(MLME_DISASSOC_REQ_STRUCT);
            PlatformMoveMemory(pMsgElem->Msg, &DisReq, sizeof(MLME_DISASSOC_REQ_STRUCT));

            MlmeDisassocReqAction(pAd, pMsgElem);

            PlatformFreeMemory(pMsgElem, sizeof(MLME_QUEUE_ELEM));
        }
        else
        {
            DBGPRINT(RT_DEBUG_ERROR, ("%s:Allocate memory failed for MT2_MLME_DISASSOC_REQ\n", __FUNCTION__));
        }
        pPort->Mlme.CntlMachine.CurrState = CNTL_IDLE;

        pAd->MlmeAux.CurrReqIsFromNdis = TRUE;
        
        MlmeCntLinkDown(pPort, FALSE);
        
        if (IS_P2P_CON_CLI(pAd, pPort) || IS_P2P_MS_CLI(pAd, pPort) || IS_P2P_MS_CLI2(pAd, pPort))
        {
            Wcid = P2pGetClientWcid(pAd, pPort);
        }

        MtAsicUpdateRxWCIDTable(pAd, Wcid , ZERO_MAC_ADDR);
        //
        // Indicate disconnect status
        //
        PlatformIndicateDisassociation(pAd,pPort, DisReq.Addr, DOT11_DISASSOC_REASON_OS);
        DBGPRINT(RT_DEBUG_TRACE, ("TONDIS:  N6UsbPnpSetPower leave D0, PlatformIndicateDisassociation [%02x:%02x:%02x:%02x:%02x:%02x]\n",
                    DisReq.Addr[0], DisReq.Addr[1], DisReq.Addr[2], DisReq.Addr[3], DisReq.Addr[4], DisReq.Addr[5]));                   
    }
    else if (ADHOC_ON(pPort))
    {
        DBGPRINT(RT_DEBUG_TRACE, ("inforeset :: disconnect adhoc"));                    
        //
        // Set Deauthentication
        //
        MlmeCntLinkDown(pPort, FALSE); 

        //
        // Indicate disconnect status for all peer.
        // 
        PlatformIndicateDisassociation(pAd,pPort, BROADCAST_ADDR, DOT11_DISASSOC_REASON_OS);
    }   


#if(COMPILE_WIN8_ABOVE(CURRENT_OS_NDIS_VER))  
    if(NDIS_WIN8_ABOVE(pAd))
    {
        // Reset p2p actions
        P2pMsInfoResetRequest(pAd, pPort);
    }
#endif

    // Reset mlme state machine
    MlmeRestartStateMachine(pAd,pPort);

    // [Workaround] Fix the next conenction enters an invalid state due to long execution time of MlmeSyncScanReqAction.
    // For example, 7610U(11ac usb) spent 30 ~ 80 ms.
    // If scan action is still running, set a flag here and stop scan machine on the next connect_request.
    // Assume connect_request oid follows reset_request oid for windows auto-configuration. 
    // TODO: p2p port ??
    if (pPort->PortNumber == PORT_0)
    {
        pAd->StaCfg.bStopScanForNextConnect = FALSE;

        // Reset SCAN machine twice. 
        // One is in MlmeRestartStateMachine and the other is in MlmeCntlOidSsidProc
        if (MT_TEST_FLAG(pAd, fRTMP_ADAPTER_SCAN_REQ_ACTION_IN_PROGRESS))
        {
            pAd->StaCfg.bStopScanForNextConnect = TRUE;
            DBGPRINT(RT_DEBUG_TRACE, ("inforeset :: bStopScanForNextConnect is true"));
        }   
    }
        

    // Use Port0 to do p2p scan.
    // When called Port0 inforeset, stop p2p scan and indicate scan complete to NDIS.
#if(COMPILE_WIN8_ABOVE(CURRENT_OS_NDIS_VER))  
    if (NDIS_WIN8_ABOVE(pAd) &&
        (pPort->PortNumber == PORT_0) &&
        (IS_P2P_MS_DEV(pAd, pAd->PortList[pPort->P2PCfg.PortNumber])) &&
        MT_TEST_BIT(pPort->P2PCfg.WaitForMSCompletedFlags, P2pMs_WAIT_COMPLETE_DISCO_REQ))
    {
        // set to scan timeout
        pPort->P2PCfg.P2pCounter.CounterAftrScanButton = pPort->P2PCfg.ScanPeriod;
        // listen only
        if (pPort->P2PCfg.P2PDiscoProvState != P2P_ENABLE_LISTEN_ONLY)
            DBGPRINT(RT_DEBUG_TRACE, ("MlmeRestartStateMachine, Goback to state P2P_ENABLE_LISTEN_ONLY \n"));
        pPort->P2PCfg.P2PDiscoProvState = P2P_ENABLE_LISTEN_ONLY;
        
        P2pSetListenIntBias(pAd, 3);
        
        //Indicate to discovery complete to OS once scan period expired
        P2pMsIndicateDiscoverComplete(pAd, pAd->PortList[pPort->P2PCfg.PortNumber]);
        MT_CLEAR_BIT(pPort->P2PCfg.WaitForMSCompletedFlags, P2pMs_WAIT_COMPLETE_DISCO_REQ);

        // Stop p2p scan
        if ((pAd->MlmeAux.ScanType == SCAN_P2P) && (pAd->PortList[PORT_0]->Mlme.SyncMachine.CurrState == SCAN_LISTEN))
        {
            // Set scan channel to Last one to stop Scan Phase. Because SCannextchannel will use channel to judge if it should stop scan.
            pPort->ScaningChannel = pPort->P2PCfg.P2pProprietary.ListenChanel[pPort->P2PCfg.P2pProprietary.ListenChanelCount-1];
            DBGPRINT(RT_DEBUG_TRACE, ("MlmeRestartStateMachine, End of switching channel\n"));
        }
    }
#endif

    //
    // Reset Stuff here.
    //
    
    MP_SET_STATE(pPort, INIT_STATE);

    OPSTATUS_CLEAR_FLAG(pPort, fOP_STATUS_MEDIA_STATE_CONNECTED); 

    // 2->will not clear up accepted multicast poll
    if (pPort->CommonCfg.MPolicy != 2)
    {
        //for ndistest packetfilter_ext ,clear multicast list
        pPort->PortCfg.MCAddressCount  = 0;
        PlatformZeroMemory(pPort->PortCfg.MCAddressList, DOT11_ADDRESS_SIZE * HW_MAX_MCAST_LIST_SIZE);
    }

    // MSCLI start --> MSGO start --> MSGO stop, OS will 1. reset 2. set GO port to EXSTA 3. reset then trigger below code and stop connected MSCLI
    if (pPort->PortSubtype == PORTSUBTYPE_STA || 
        (pPort->PortSubtype == PORTSUBTYPE_P2PClient))
    {
// 2011-11-25 For P2pMs GO multi-channel
#ifdef MULTI_CHANNEL_SUPPORT        
        do 
        {
            if (INFRA_ON(pAd->PortList[PORT_0]))
            {
                DBGPRINT(RT_DEBUG_TRACE,("%s - Bypass MlmeInfoWPARemoveAllKeys sinec STA is still in operation\n", __FUNCTION__));
                break;
            }

            if (MT_TEST_BIT(pAd->pP2pCtrll->P2pMsConnectedStatus, P2pMs_CONNECTED_STATUS_CLI_OP))
            {
                DBGPRINT(RT_DEBUG_TRACE,("%s - Bypass MlmeInfoWPARemoveAllKeys sinec MSCLI is still in operation\n", __FUNCTION__));
                break;              
            }
            
            //need to verify when to remove key*/
            MlmeInfoWPARemoveAllKeys(pAd, pPort);
            pPort->PortCfg.DefaultKeyId = 0;
        } while (FALSE);
#else       
        //need to verify when to remove key
        pPort->PortCfg.DefaultKeyId = 0;
        MlmeInfoWPARemoveAllKeys(pAd,pPort);        
#endif /* MULTI_CHANNEL_SUPPORT */      
    }


    // Before entering hibernate, Port2 should be deleted.
    // After wakeup, InfoReset is earlier than p2p event handler.
    // So we don't need to check opmode here. 
#if(COMPILE_WIN8_ABOVE(CURRENT_OS_NDIS_VER))  
    if (NDIS_WIN8_ABOVE(pAd) && pPort->PortNumber != NDIS_DEFAULT_PORT_NUMBER)
    {
        DBGPRINT(RT_DEBUG_TRACE, ("<== RTMPInfoResetRequest on not ndis default port\n"));
        return;
    }
    else
#endif
    if (NDIS_WIN7(pAd) && /*(pAd->OpMode == OPMODE_STAP2P) &&*/ (PORTV2_P2P_ON(pAd, pPort)))
    {
        DBGPRINT(RT_DEBUG_TRACE, ("<== RTMPInfoResetRequest on not ndis default port\n"));
        return;
    }
    // <========== Before these are about port parameters


    //This avoid auto connection that causing scan not confirm
    pAd->MlmeAux.AutoReconnectSsidLen= 32;
    pAd->MlmeAux.AutoReconnectStatus = FALSE;
    PlatformZeroMemory(pAd->MlmeAux.AutoReconnectSsid, pAd->MlmeAux.AutoReconnectSsidLen);

    if(pAd->MlmeAux.bNeedIndicateRoamStatus)
    {
        PMP_PORT pIndicatePort = pAd->PortList[PORT_0];       
        if(pIndicatePort != NULL)
        {
            DBGPRINT_ERR(("NdisCommonInfoResetHandler: Indicate roam complete\n"))
            PlatformIndicateRoamingCompletion(pAd,pIndicatePort, DOT11_CONNECTION_STATUS_CANDIDATE_LIST_EXHAUSTED);             
        }
    }

    if( pPort->CommonCfg.bSafeModeEnabled )
    {   
        if( pPort->CommonCfg.SafeModeQOS_PARAMS == 0 )
        {
            // Never set phymode to 0xff. If you want to update channel list,  turn on paramater3 in MlmeInfoSetPhyMode routine. -- MlmeInfoSetPhyMode(xxx, xxx,TRUE)
            // pPort->CommonCfg.PhyMode = 0xff;
            // Build all corresponding channel information
            MlmeInfoSetPhyMode(pAd, pPort, pPort->CommonCfg.SafeModeRestorePhyMode,TRUE);     
        }
        pPort->CommonCfg.bSafeModeEnabled =0;
        pPort->CommonCfg.SafeModeQOS_PARAMS = 0xFF;
    }

    pAd->StaCfg.Psm = PWR_ACTIVE;

    pAd->MlmeAux.bStaCanRoam = FALSE;

    pAd->MlmeAux.AssocState = dot11_assoc_state_unauth_unassoc; 

    pAd->pHifCfg->BulkLastOneSecCount = 0;
    
    pPort->CommonCfg.bSSIDInProbeRequest = TRUE;

    // Clear PMKID cache.
    PlatformZeroMemory(&pPort->CommonCfg.PMKIDCache, sizeof(PMKID_CACHE));    
    pAd->StaCfg.PMKIDCount = 0;
    PlatformZeroMemory(pAd->StaCfg.PMKIDList, (sizeof(DOT11_PMKID_ENTRY) * STA_PMKID_MAX_COUNT));

    // Safety, clear it.
    MT_CLEAR_FLAG(pAd, fRTMP_ADAPTER_ADDKEY_IN_PROGRESS); 

    pPort->CommonCfg.AcceptICMPv6RA = FALSE;

}

size_t
NdisCommonGetPipeNumber(
    IN PMP_ADAPTER pAd
    )
{
    size_t PipeNumber =0;
#if IS_BUS_TYPE_USB(DEV_BUS_TYPE)
    PipeNumber = NdisCommonUsbGetPipeNumber(pAd);
#elif IS_BUS_TYPE_PCI(DEV_BUS_TYPE)
    // TODO:
#elif IS_BUS_TYPE_SDIO(DEV_BUS_TYPE)    
    // TODO:
#endif  
    return PipeNumber;
}

VOID
NdisCommonMappingPortToBssIdx(
    IN  PMP_PORT    pPort,
    IN  UCHAR     OwnMacOption,
    IN  UCHAR     BSSMacOption
    )
{
    PMP_ADAPTER pAd = pPort->pAd;
    UCHAR   BssidIndex = DEFAULT_BSSID_INDEX;
    PMP_PORT    pWfdDevicePort = pAd->pP2pCtrll->pWfdDevicePort;

    DBGPRINT(RT_DEBUG_TRACE, ("%s   ==> port[%d]   PortType = %d OwnMacOption = %d  BSSMacOption = %d\n", __FUNCTION__, pPort->PortNumber, pPort->PortType, OwnMacOption, BSSMacOption));
    DBGPRINT(RT_DEBUG_TRACE, ("%s   port[%d]->CurrentAddress = %x %x %x %x %x %x", __FUNCTION__, pPort->PortNumber, pPort->CurrentAddress[0], pPort->CurrentAddress[1], pPort->CurrentAddress[2], pPort->CurrentAddress[3], pPort->CurrentAddress[4], pPort->CurrentAddress[5]));
    DBGPRINT(RT_DEBUG_TRACE, ("%s   port[%d]->Bssid = %x %x %x %x %x %x", __FUNCTION__, pPort->PortNumber, pPort->PortCfg.Bssid[0], pPort->PortCfg.Bssid[1], pPort->PortCfg.Bssid[2], pPort->PortCfg.Bssid[3], pPort->PortCfg.Bssid[4], pPort->PortCfg.Bssid[5]));

     switch (OwnMacOption)
    {
        case UPDATE_MAC:
            if (pPort->PortNumber == PORT_0)
            {
                pPort->OwnMACAddressIdx = BSSID_INDEX_0;
                pAd->MBssidTable[pPort->OwnMACAddressIdx] = TRUE;
                MtAsicSetDevMac(pAd, pPort->CurrentAddress, (UINT8)pPort->OwnMACAddressIdx);
            }
            // use WFD role port to replace WFD device port
            else if (pPort->PortType == WFD_CLIENT_PORT || pPort->PortType == WFD_GO_PORT)
           {
                //Step1  role port replace wfd device port own mac
                DBGPRINT(RT_DEBUG_TRACE, ("%s   This port is WFD role port so replace WFD device port OwnMACAddressIdx = %d \n", __FUNCTION__, pAd->pP2pCtrll->pWfdDevicePort->OwnMACAddressIdx));
                pPort->OwnMACAddressIdx = pWfdDevicePort->OwnMACAddressIdx;

                DBGPRINT(RT_DEBUG_TRACE, ("%s   port[%d]->OwnMACAddressIdx = %d \n", __FUNCTION__, pPort->PortNumber, pAd->pP2pCtrll->pWfdDevicePort->OwnMACAddressIdx));
                
                if (pPort->OwnMACAddressIdx < HW_MAX_SUPPORT_MBSSID)
                {               
                    pAd->MBssidTable[pPort->OwnMACAddressIdx] = TRUE;
                    MtAsicSetDevMac(pAd, pPort->CurrentAddress, (UINT8)pPort->OwnMACAddressIdx);
                }

                //Step2.
                // check if more Bssid for using
                // if yes, set device port to free index
                BssidIndex = NdisCmmonMoreBSSID(pAd);

                // Step3. move WFD device port to free own mac index
                if (BssidIndex != DEFAULT_BSSID_INDEX)
                {
                    pAd->pP2pCtrll->pWfdDevicePort->OwnMACAddressIdx = BssidIndex;
                    pAd->MBssidTable[BssidIndex] = TRUE;
                    MtAsicSetDevMac(pAd, pWfdDevicePort->CurrentAddress, (UINT8)pWfdDevicePort->OwnMACAddressIdx);
                }
                else
                {
                    pWfdDevicePort->OwnMACAddressIdx = DEFAULT_BSSID_INDEX;
                    DBGPRINT(RT_DEBUG_ERROR, ("%s(%d)  There is no more BSSID Index for using", __FUNCTION__, __LINE__));
                }
            }
            else
            {
                // check if more Bssid for using
                // if yes, set device port to free index
                BssidIndex = NdisCmmonMoreBSSID(pAd);
                if (BssidIndex != DEFAULT_BSSID_INDEX)
                {
                    pPort->OwnMACAddressIdx = BssidIndex;
                    pAd->MBssidTable[pPort->OwnMACAddressIdx] = TRUE;
                    MtAsicSetDevMac(pAd, pPort->CurrentAddress, (UINT8)pPort->OwnMACAddressIdx);
                }
                else
                {
                    DBGPRINT(RT_DEBUG_ERROR, ("%s(%d)   There is no more BSSID Index for using", __FUNCTION__, __LINE__));
                }
                
            }
        break;

        case DELETE_MAC:

            if ((pPort->PortNumber != PORT_0) && (pPort->OwnMACAddressIdx != DEFAULT_BSSID_INDEX))
            {
                pAd->MBssidTable[pPort->OwnMACAddressIdx] = FALSE;
                MtAsicSetDevMac(pAd, ZERO_MAC_ADDR, (UINT8)pPort->OwnMACAddressIdx);
                pPort->OwnMACAddressIdx = DEFAULT_BSSID_INDEX;

                if (pWfdDevicePort != NULL)
                {
                    if (pWfdDevicePort->OwnMACAddressIdx == DEFAULT_BSSID_INDEX)
                    {   
                        BssidIndex = NdisCmmonMoreBSSID(pAd);
                    
                        if (BssidIndex != DEFAULT_BSSID_INDEX)
                        {
                            pWfdDevicePort->OwnMACAddressIdx = BssidIndex;
                            pAd->MBssidTable[pPort->OwnMACAddressIdx] = TRUE;
                            MtAsicSetDevMac(pAd, pWfdDevicePort->CurrentAddress, (UINT8)pWfdDevicePort->OwnMACAddressIdx);
                        }
                        else
                        {
                            DBGPRINT(RT_DEBUG_ERROR, ("%s(%d)   There is no more BSSID Index for using", __FUNCTION__, __LINE__));
                        }
                    }
                    
                }
            }
            
        break;

        case DO_NOTHING:
            DBGPRINT(RT_DEBUG_TRACE, ("%s do nothing for OwnMAC \n"));
        break;
    }


    switch (BSSMacOption)
    {
        case UPDATE_MAC:
            MtAsicSetMultiBssidEx(pAd, pPort->PortCfg.Bssid, (UCHAR) pPort->OwnMACAddressIdx);
        break;

        case DELETE_MAC:
            MtAsicSetMultiBssidEx(pAd, ZERO_MAC_ADDR, (UCHAR) pPort->OwnMACAddressIdx);
        break;

        case DO_NOTHING:
            DBGPRINT(RT_DEBUG_TRACE, ("%s do nothing for BSSMAC \n"));
        break;
    }
     
}

UCHAR
NdisCmmonMoreBSSID(
    IN  PMP_ADAPTER pAd
)
{
    UCHAR   BssidIndex = DEFAULT_BSSID_INDEX;
    UCHAR   Index = 0;
    
    if (IS_SUPPORT_THREE_MBSSID(pAd))
    {
        // Index 0 always reserve for Port0
        for (Index = 1; Index < 3; Index++)
        {
            DBGPRINT(RT_DEBUG_TRACE, ("%s   pAd->MBssidTable[%d] = %d", __FUNCTION__, Index, pAd->MBssidTable[Index]));
            if (pAd->MBssidTable[Index]  == FALSE)
            {
                BssidIndex = Index;
                return BssidIndex;
            }
        }
    }

    
    return  BssidIndex;
}

ULONG
NdisCommonCopyPayloadFromXmitBuffer(
    IN PVOID Dest,
    IN PMT_XMIT_CTRL_UNIT pXcu
    )
{
    UCHAR  i =0;
    ULONG   CopyLen =0;
    for(i = 1;i< pXcu->NumOfBuf;i++)
    {
        PlatformMoveMemory(Dest, pXcu->BufferList[i].VirtualAddr, pXcu->BufferList[i].Length);
        CopyLen+=pXcu->BufferList[i].Length;
    }
    return CopyLen;
}

ULONG
NdisCommonCopyBufferFromXmitBuffer(
    IN PVOID Dest,
    IN PMT_XMIT_CTRL_UNIT pXcu
    )
{
    UCHAR  i =0;
    ULONG   CopyLen =0;
    for(i = 0;i< pXcu->NumOfBuf;i++)
    {
        PlatformMoveMemory(Dest, pXcu->BufferList[i].VirtualAddr, pXcu->BufferList[i].Length);
        CopyLen+=pXcu->BufferList[i].Length;
    }
    return CopyLen;
}

VOID
NdisCommonReportEthFrameToLLC(
    IN  PMP_ADAPTER   pAd,
    IN  PMP_PORT      pPort,
    IN  PUCHAR          p80211DataFrame,
    IN  USHORT          us80211FrameSize,
    IN  UCHAR           UserPriority,
    IN  BOOLEAN         CRCError,
    IN  BOOLEAN         bSingleIndicate
    )
{
 #if (COMPILE_VISTA_ABOVE(CURRENT_OS_NDIS_VER))
    Ndis6CommonReportEthFrameToLLC(pAd, pPort, p80211DataFrame, us80211FrameSize, UserPriority, CRCError, bSingleIndicate);
 #else
 #endif
}

VOID
NdisCommonReportEthFrameToLLCAgg(
    IN  PMP_ADAPTER  pAd,
    IN  PMP_PORT  pPort,
    IN  PUCHAR  p80211Header,
    IN  USHORT  Payload1Size,
    IN  USHORT  Payload2Size,
    IN  UCHAR   UserPriority
    )
{
 #if (COMPILE_VISTA_ABOVE(CURRENT_OS_NDIS_VER))
    Ndis6CommonReportEthFrameToLLCAgg(pAd, pPort, p80211Header, Payload1Size, Payload2Size, UserPriority);
 #else
 #endif
}

VOID
NdisCommonReportAMSDUEthFrameToLLC(
    IN  PMP_ADAPTER   pAd,
    IN  PMP_PORT      pPort,
    IN  PUCHAR          p80211DataFrame,
    IN  USHORT          us80211FrameSize,
    IN  UCHAR           UserPriority,
    IN  BOOLEAN         bSingleIndicate
    )
{
 #if (COMPILE_VISTA_ABOVE(CURRENT_OS_NDIS_VER))
    Ndis6CommonReportAMSDUEthFrameToLLC(pAd, pPort, p80211DataFrame, us80211FrameSize, UserPriority, bSingleIndicate);
 #else
 #endif
}