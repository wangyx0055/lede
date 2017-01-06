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
    Thread.c

    Abstract:
    Miniport generic portion header file

    Revision History:
    Who         When          What
    --------    ----------    ----------------------------------------------
    */
    
#include    "MtConfig.h"

//=============================================
// Timer related fucntin
//=============================================

/*
    ========================================================================
    
    Routine Description:
        Timer callback function.

    Arguments:
        pAd         Pointer to our adapter
        pTimer              Timer structure
        pTimerFunc          Function to execute when timer expired
        Repeat              Ture for period timer

    Return Value:
        None

    Note:
        
    ========================================================================
*/
VOID
    PlatformTimerCallBackFunc(
    IN  PVOID   SystemSpecific1,
    IN  PVOID   FunctionContext,
    IN  PVOID   SystemSpecific2,
    IN  PVOID   SystemSpecific3
    )
{
    IN  PMTK_TIMER_STRUCT    pTimer = (PMTK_TIMER_STRUCT)FunctionContext;
    IN  PMP_PORT   pPort= (PMP_PORT)pTimer->pContext;

    if((pTimer == NULL) ||(pPort ==NULL))
    {
        DBGPRINT(RT_DEBUG_TRACE,("PlatformTimerCallBackFunc(), pTimer or pAd == NULL\n"));
        return;
    }
    
    if (MT_TEST_FLAG(pPort->pAd, fRTMP_ADAPTER_HALT_IN_PROGRESS) ||(MT_TEST_FLAG(pPort->pAd, fRTMP_ADAPTER_SURPRISE_REMOVED)))
    {
        DBGPRINT(RT_DEBUG_TRACE,("PlatformTimerCallBackFunc(), halt ...........\n"));
        return;
    }

    pTimer->CallBackFunc(NULL, pPort, pTimer, NULL);
}


/*
    ========================================================================
    
    Routine Description:
        Init timer objects

    Arguments:
        pAd         Pointer to our adapter
        pTimer              Timer structure
        pTimerFunc          Function to execute when timer expired
        Repeat              Ture for period timer

    Return Value:
        None

    Note:
        
    ========================================================================
*/
VOID
PlatformInitTimer(
    IN  PMP_PORT   pPort,
    IN  PMTK_TIMER_STRUCT    pTimer,
    IN  MT_TIMER_CALL_BACK  pTimerFunc,
    IN  BOOLEAN Repeat,
    IN  const char* szID
    )
{
    NDIS_STATUS ndisStatus = NDIS_STATUS_SUCCESS;
    NDIS_TIMER_CHARACTERISTICS  TimerCharacteristics;

    if(pTimer->Valid == TRUE)
    {
        DBGPRINT(RT_DEBUG_TRACE,("PlatformInitTimer() %s  already inited\n", szID));
        return;
    }
    //
    // Set Valid to TRUE for later used.
    // It will crash if we cancel a timer or set a timer 
    // that we haven't initialize before.
    //
    pTimer->Repeat     = Repeat;
    pTimer->State      = FALSE;
    pTimer->pContext = pPort;
    pTimer->CallBackFunc = pTimerFunc;
    pTimer->bFreed = FALSE;

    if(pTimer->bInputOption ==FALSE)
    {
        pTimer->pOption = NULL;
    }

    PlatformZeroMemory(pTimer->szID, 40);


    if(szID != NULL)
    {
     STR_COPY_WITH_ASCII(pTimer->szID, szID, 40);
    }


#if(COMPILE_WIN7_ABOVE(CURRENT_OS_NDIS_VER))  

    TimerCharacteristics.Header.Type = NDIS_OBJECT_TYPE_TIMER_CHARACTERISTICS;
    TimerCharacteristics.Header.Revision  = NDIS_TIMER_CHARACTERISTICS_REVISION_1;
    TimerCharacteristics.Header.Size = NDIS_SIZEOF_TIMER_CHARACTERISTICS_REVISION_1;

    TimerCharacteristics.AllocationTag = '3067';
    TimerCharacteristics.TimerFunction = PlatformTimerCallBackFunc;
    TimerCharacteristics.FunctionContext = pTimer;

    ndisStatus = NdisAllocateTimerObject(
        pPort->pAd->AdapterHandle,
        &TimerCharacteristics,
        &pTimer->TimerObj
        );

    if(ndisStatus == NDIS_STATUS_SUCCESS)
    {
        pPort->pAd->TimerAllociateCnt++;
        DBGPRINT(RT_DEBUG_TRACE,("PlatformInitTimer ()  %s  ,   pAd->TimerAllociateCnt %d\n",pTimer->szID,pPort->pAd->TimerAllociateCnt)); 
        pTimer->Valid      = TRUE;
    }
#else

    NdisMInitializeTimer(&pTimer->TimerObj,
        pPort->pAd->AdapterHandle,
        PlatformTimerCallBackFunc,
        pTimer);
        pTimer->Valid      = TRUE;
#endif

}


/*
    ========================================================================
    
    Routine Description:
        Init timer objects

    Arguments:
        pTimer              Timer structure
        Value               Timer value in milliseconds

    Return Value:
        None

    Note:
        To use this routine, must call PlatformInitTimer before.
        
    ========================================================================
*/
VOID
PlatformSetTimer(
    IN  PMP_PORT   pPort,
    IN  PMTK_TIMER_STRUCT    pTimer,
    IN  ULONG   MillisecondsPeriod
    )
{

    if(pTimer ==NULL || pPort == NULL)
    {
        DBGPRINT(RT_DEBUG_TRACE,("PlatformSetTimer(), pTimer or pAd == NULL\n"));
        return;
    }
    //
    // We should not set a timer when driver is on Halt state.
    // 
    if (MT_TEST_FLAG(pPort->pAd, fRTMP_ADAPTER_HALT_IN_PROGRESS) ||(MT_TEST_FLAG(pPort->pAd, fRTMP_ADAPTER_SURPRISE_REMOVED)))
     return;

    if (pTimer->Valid)
    {
        pTimer->TimerValue = MillisecondsPeriod;
        pTimer->State      = FALSE;

        if (pTimer->Repeat == TRUE)
        {
#if(COMPILE_WIN7_ABOVE(CURRENT_OS_NDIS_VER))  
            LARGE_INTEGER               fireTime;
            fireTime.QuadPart = Int32x32To64((LONG)MillisecondsPeriod, -10000);
            NdisSetTimerObject(pTimer->TimerObj, fireTime, MillisecondsPeriod,NULL);
#else
            NdisMSetPeriodicTimer(&pTimer->TimerObj, MillisecondsPeriod);

#endif
        }
        else
        {
#if(COMPILE_WIN7_ABOVE(CURRENT_OS_NDIS_VER))  
            LARGE_INTEGER               fireTime;
            fireTime.QuadPart = Int32x32To64((LONG)MillisecondsPeriod, -10000);
            NdisSetTimerObject(pTimer->TimerObj, fireTime, 0,NULL);
#else

#pragma prefast(suppress: __WARNING_USE_OTHER_FUNCTION, " should not ue this function") 
            NdisMSetTimer(&pTimer->TimerObj, MillisecondsPeriod);
#endif
        }
    }
    else
    {
     DBGPRINT_ERR(("PlatformSetTimer failed, Timer hasn't been initialize!\n"));
    }
}


/*
    ========================================================================
    
    Routine Description:
        Cancel timer objects

    Arguments:
        Adapter                     Pointer to our adapter

    Return Value:
        None

    IRQL = PASSIVE_LEVEL
    IRQL = DISPATCH_LEVEL
    
    Note:
        1.) To use this routine, must call PlatformInitTimer before.
        2.) Reset NIC to initial state AS IS system boot up time.
        
    ========================================================================
*/
VOID    
PlatformCancelTimer(
    IN  PMTK_TIMER_STRUCT    pTimer,
    OUT BOOLEAN *pCancelled
    )
{
    if(pTimer ==NULL )
    {
        DBGPRINT(RT_DEBUG_TRACE,("PlatformCancelTimer(), pTimer == NULL\n"));
        return;
    }
    
    if (pTimer->Valid)
    {
        if (pTimer->State == FALSE)
        {
#if(COMPILE_WIN7_ABOVE(CURRENT_OS_NDIS_VER))  
            BOOLEAN bCanceled =FALSE;
            bCanceled = NdisCancelTimerObject(pTimer->TimerObj);

            if(!bCanceled)
            {
                //DBGPRINT_ERR(("PlatformCancelTimer() failed, %s\n",pTimer->szID));
            }
            else
            {
                pTimer->State = TRUE;
                //DBGPRINT(RT_DEBUG_TRACE,("PlatformCancelTimer ()  done %s  \n",pTimer->szID));
            }
#else
            NdisMCancelTimer(&pTimer->TimerObj, pCancelled);
#endif
        }
        if (*pCancelled == TRUE)
         pTimer->State = TRUE;
    }
    else
    {
        //
        // NdisMCancelTimer just canced the timer and not mean release the timer.
        // And don't set the "Valid" to False. So that we can use this timer again. 
        //
        //DBGPRINT_ERR(("NdisMCancelTimer failed, Timer hasn't been initialize!  %s\n",pTimer->szID));
    }
}


VOID
PlatformFreeTimer(
    IN  PMTK_TIMER_STRUCT    pTimer
    )
{
#if(COMPILE_WIN7_ABOVE(CURRENT_OS_NDIS_VER))  
    IN  PMP_PORT pPort = (PMP_PORT)pTimer->pContext;
    BOOLEAN bCanceled =FALSE;
    
    if(pTimer->bFreed == TRUE)   
    {
        DBGPRINT(RT_DEBUG_TRACE,("PlatformFreeTimer()  pTimer->bFreed == TRUE \n"));
        return;    
    }
    
    if(pTimer ==NULL)
    {
        DBGPRINT(RT_DEBUG_TRACE,("PlatformFreeTimer()  pTimer ==NULL \n"));
        return;
    }
    if(pTimer->TimerObj ==NULL)
    {
        DBGPRINT(RT_DEBUG_TRACE,("PlatformFreeTimer()  pTimer->TimerObj ==NULL \n"));
        return;
    }

    bCanceled = NdisCancelTimerObject(pTimer->TimerObj);

    if(!bCanceled)
    {
        KeFlushQueuedDpcs();
    }

    NdisFreeTimerObject(pTimer->TimerObj);

    pTimer->bFreed = TRUE;
    pTimer->Valid =FALSE;

    if(pPort->pAd->TimerAllociateCnt >0)
        pPort->pAd->TimerAllociateCnt --;
    DBGPRINT(RT_DEBUG_TRACE,("PlatformFreeTimer()  %s , pAd->TimerAllociateCnt  %d \n",pTimer->szID, pPort->pAd->TimerAllociateCnt ));
    
#endif  
}


//=============================================
// Thread related fucntin
//=============================================


BOOLEAN
PlatformWaitEventTriggeredAndReset(
    IN PNDIS_EVENT pEvent,
    IN UINT   Wait_ms
    )
{
    BOOLEAN ret =FALSE;    
    ret = NdisWaitEvent(pEvent,Wait_ms);   
    NdisResetEvent(pEvent);   

    return ret;
}

VOID
PlatformThreadCallBackFunc(
    IN PVOID Context
    )
{
    PMTK_THREAD_STRUCT    pThread = (PMTK_THREAD_STRUCT)Context;
    PMP_ADAPTER   pAd = (PMP_ADAPTER)pThread->pContext;

    if(Context ==NULL || pAd ==NULL)
    {
        DBGPRINT(RT_DEBUG_TRACE,("PlatformThreadCallBackFunc(), Context == NULL,  or pAd == NULL\n"));
        return;
    }
    

    if (MT_TEST_FLAG(pAd, fRTMP_ADAPTER_HALT_IN_PROGRESS) ||(MT_TEST_FLAG(pAd, fRTMP_ADAPTER_SURPRISE_REMOVED)))
    {
        DBGPRINT(RT_DEBUG_TRACE,("PlatformThreadCallBackFunc(), halt ...........\n"));
        return;
    }
    DBGPRINT(RT_DEBUG_TRACE,("===> PlatformThreadCallBackFunc(),     %s\n", pThread->szID));
    pThread->State  = THREAD_STATUS_RUNNING;
    
    pThread->CallBackFunc(pAd);

    pThread->State = THREAD_STATUS_INITED;
    
    NdisSetEvent(&(pThread->CompleteEvent));

    DBGPRINT(RT_DEBUG_TRACE,("<=== PlatformThreadCallBackFunc(),     %s\n", pThread->szID));
    PsTerminateSystemThread(STATUS_SUCCESS);    
}

VOID
PlatformInitThread(
    IN  PMP_ADAPTER   pAd,
    IN  PMTK_THREAD_STRUCT    pThread,
    IN  MT_THREAD_CALL_BACK  pThreadFunc,
    IN  const char* szID
    )
{
    if(pThread ==NULL || pAd ==NULL)
    {
        DBGPRINT(RT_DEBUG_TRACE,("PlatformInitThread(), Context == NULL,  or pAd == NULL\n"));
        return;
    }

    if((pThread->InitCnt !=0) && (pThread->State != THREAD_STATUS_NOT_INIT))
    {
        DBGPRINT(RT_DEBUG_TRACE, ("PlatformInitThread fail - pThread->InitCnt !=0. The thread already init before (%s)\n",pThread->szID));
        return;
    }
    pThread->State      = THREAD_STATUS_INITED;
    pThread->pContext = (PVOID)pAd;
    pThread->CallBackFunc = pThreadFunc;
    pThread->bFreed = FALSE;
    pThread->InitCnt++;

    NdisInitializeEvent(&(GET_THREAD_EVENT(pThread)));
    NdisInitializeEvent(&(pThread->CompleteEvent));
    
    PlatformZeroMemory(pThread->szID, 64);

    if(szID != NULL)
    {
         STR_COPY_WITH_ASCII(pThread->szID, szID, 64);
    }
}

NDIS_STATUS
PlatformRunThread(
    IN  PMP_ADAPTER   pAd,
    IN  PMTK_THREAD_STRUCT    pThread
    )
{
    NDIS_STATUS ndisStatus = NDIS_STATUS_SUCCESS;
    OBJECT_ATTRIBUTES   ObjectAttribs;

    if(pThread ==NULL || pAd ==NULL)
    {
        DBGPRINT(RT_DEBUG_TRACE,("PlatformRunThread(), Context == NULL,  or pAd == NULL\n"));
        return NDIS_STATUS_FAILURE;
    }
    
    if(pThread->State != THREAD_STATUS_INITED)
    {
        DBGPRINT(RT_DEBUG_TRACE, ("PlatformRunThread fail - The thread not init. (%s)\n",pThread->szID));
        return NDIS_STATUS_FAILURE;
    }
    DBGPRINT(RT_DEBUG_TRACE, ("PlatformRunThread (%s)\n",pThread->szID));
    
    InitializeObjectAttributes(&ObjectAttribs, 
                                            NULL,
                                            OBJ_KERNEL_HANDLE, 
                                            NULL,
                                            NULL);

    ndisStatus = PsCreateSystemThread(&pThread->handle,
                                                    (ACCESS_MASK) 0L,
                                                    &ObjectAttribs,
                                                    NULL,
                                                    NULL,
                                                    PlatformThreadCallBackFunc,
                                                    pThread);

    if (!NT_SUCCESS(ndisStatus))
    {
        pThread->State      = THREAD_STATUS_INITED;
        DBGPRINT(RT_DEBUG_TRACE, ("PlatformRunThread fail - PsCreateSystemThread fail status (%x). (%s)\n",ndisStatus, pThread->szID));
        return ndisStatus;
    }

    ndisStatus = ObReferenceObjectByHandle(pThread->handle,
                                                    (ACCESS_MASK) 0L,
                                                    NULL,
                                                    KernelMode,
                                                    &pThread->ThreadObj,
                                                    NULL);
                                                    
    if (!NT_SUCCESS(ndisStatus))
    {
        pThread->State      = THREAD_STATUS_INITED;
        DBGPRINT(RT_DEBUG_TRACE, ("PlatformRunThread fail - ObReferenceObjectByHandle fail status (%x). (%s)\n",ndisStatus, pThread->szID));
        return ndisStatus;
    }
    
    ZwClose(pThread->handle);
    return ndisStatus;
}

VOID
PlatformCancelThread(
    IN  PMP_ADAPTER   pAd,
    IN  PMTK_THREAD_STRUCT    pThread
    )
{
    if(pThread ==NULL || pAd ==NULL)
    {
        DBGPRINT(RT_DEBUG_TRACE,("PlatformCancelThread(), Context == NULL,  or pAd == NULL\n"));
        return;
    }

    if(pThread->State  == THREAD_STATUS_NOT_INIT)
    {
        DBGPRINT(RT_DEBUG_TRACE, ("PlatformCancelThread fail - Thread not inited\n"));
        return;
    }
    
    pThread->State  = THREAD_STATUS_CANCEL;
    pAd->TerminateThreads = TRUE;
    
    NdisSetEvent(&(GET_THREAD_EVENT(pThread)));
    NdisWaitEvent(&(pThread->CompleteEvent),0);    
}

VOID
PlatformFreeThread(
    IN  PMP_ADAPTER   pAd,
    IN  PMTK_THREAD_STRUCT    pThread
    )
{
    if(pThread ==NULL || pAd ==NULL)
    {
        DBGPRINT(RT_DEBUG_TRACE,("PlatformFreeThread(), Context == NULL,  or pAd == NULL\n"));
        return;
    }

    if(pThread->State  == THREAD_STATUS_NOT_INIT)
    {
        DBGPRINT(RT_DEBUG_TRACE, ("PlatformCancelThread fail - Thread not inited\n"));
        return;
    }
    
    pThread->State  = THREAD_STATUS_NOT_INIT;
    pAd->TerminateThreads = TRUE;
    
    KeWaitForSingleObject(pThread->ThreadObj,Executive, KernelMode, FALSE, NULL);
    ObDereferenceObject(pThread->ThreadObj);      
}

/**
 * This routine will run the workitem call back function 
 *
 * @param Context pointer to MTK_WORKITEM structure.
 * @param handle pointer to the Handle of this WorkItem.
 *
 * @return None
 */
VOID
PlatformWorkitemCallBackFunc(
    IN PVOID Context,
    IN NDIS_HANDLE  handle
    )
{
    PMTK_WORKITEM   pWorkItem = (PMTK_WORKITEM)Context;
    PMP_ADAPTER     pAd = NULL;
    PMP_PORT        pPort = NULL;

    if (pWorkItem ==NULL)
    {
        DBGPRINT(RT_DEBUG_TRACE,("PlatformWorkitemCallBackFunc(), pWorkItem == NULL\n"));
        return;
    }

    do 
    {
        pPort = (PMP_PORT)pWorkItem->pContext;    

        if(pPort ==NULL)
        {
            DBGPRINT(RT_DEBUG_TRACE,("PlatformWorkitemCallBackFunc(), pPort == NULL\n"));
            break;
        }

        pAd = pPort->pAd;
    
        if (MT_TEST_FLAG(pAd, fRTMP_ADAPTER_HALT_IN_PROGRESS) ||(MT_TEST_FLAG(pAd, fRTMP_ADAPTER_SURPRISE_REMOVED)))
        {
            DBGPRINT(RT_DEBUG_TRACE,("PlatformWorkitemCallBackFunc(), halt or surprise remove\n"));
            break;
        }

        pWorkItem->CallBackFunc(pPort);
        pWorkItem->FiredCount++;

    } while (FALSE);

    NdisAcquireSpinLock(&(pWorkItem->WorkItemLock));
    pWorkItem->RefCnt --;
    NdisSetEvent(&(pWorkItem->TrigerEvent));
    NdisReleaseSpinLock(&(pWorkItem->WorkItemLock));
        
}


NDIS_STATUS
PlatformInitWorktiem(
    IN  PMP_PORT   pPort,
    IN  PMTK_WORKITEM   pWorkItem,
    IN  MTK_WORKITEM_CALLBACK CallBackFunc,
    IN  const char* szID
    )
{
    PMP_ADAPTER pAd = pPort->pAd;
    
    if(pWorkItem ==NULL || pAd ==NULL || pPort == NULL)
    {
        DBGPRINT(RT_DEBUG_TRACE,("PlatformInitWorktiem(), pWorkItem == NULL,  or pAd == NULL\n"));
        return NDIS_STATUS_FAILURE;
    }

    if((pWorkItem->handle != NULL) && pWorkItem->RefCnt !=0)
    {
        DBGPRINT(RT_DEBUG_TRACE, ("PlatformInitWorktiem fail - Because Workitem already initialized,    %s\n", szID));
        return NDIS_STATUS_FAILURE;
    }
    pWorkItem->bFreed = FALSE;
    pWorkItem->RefCnt = 1;
    pWorkItem->CallBackFunc = CallBackFunc;
    pWorkItem->pContext = (PVOID)pPort;
    pWorkItem->handle = NdisAllocateIoWorkItem(pAd->AdapterHandle);
    pWorkItem->FiredCount = 0;

    if(pWorkItem->handle == NULL)
    {
        pWorkItem->bFreed = TRUE;
        PlatformZeroMemory(pWorkItem, sizeof(MTK_WORKITEM));
        
        return NDIS_STATUS_RESOURCES;
    }
    
    NdisInitializeEvent(&(pWorkItem->TrigerEvent));
    NdisAllocateSpinLock(&(pWorkItem->WorkItemLock));
    
    PlatformZeroMemory(pWorkItem->szID, 64);

    if(szID != NULL)
    {
         STR_COPY_WITH_ASCII(pWorkItem->szID, szID, 64);
    }

    return NDIS_STATUS_SUCCESS;
}

BOOLEAN
PlatformScheduleWorktiem(
    IN  PMTK_WORKITEM   pWorkItem
    )
{
    PMP_PORT   pPort = (PMP_PORT)pWorkItem->pContext;
    PMP_ADAPTER pAd = pPort->pAd;
    
    BOOLEAN bSuccess = FALSE;
    
    if(pWorkItem == NULL || pAd == NULL)
    {
        DBGPRINT(RT_DEBUG_TRACE, ("PlatformShceduleWorktiem fail - pWorkItem == NULL\n"));
        return FALSE;
    }

    if (MT_TEST_FLAG(pAd, fRTMP_ADAPTER_HALT_IN_PROGRESS) ||(MT_TEST_FLAG(pAd, fRTMP_ADAPTER_SURPRISE_REMOVED)))
    {
        DBGPRINT(RT_DEBUG_TRACE,("PlatformShceduleWorktiem(),  Halt or surprise remove\n"));
        return FALSE;
    }

    NdisAcquireSpinLock(&(pWorkItem->WorkItemLock));
    
    if(pWorkItem->bFreed == TRUE)
    {
        NdisReleaseSpinLock(&(pWorkItem->WorkItemLock));
        DBGPRINT(RT_DEBUG_TRACE, ("PlatformShceduleWorktiem fail - Workitem already Free\n"));
        return FALSE; 
    }
    
    if (pWorkItem->RefCnt ==2)
    {
        NdisReleaseSpinLock(&(pWorkItem->WorkItemLock));
        DBGPRINT(RT_DEBUG_TRACE, ("PlatformShceduleWorktiem fail - Workitem already Fired\n"));
        return FALSE; 
    }
    else if (pWorkItem->RefCnt ==1)
    {
        pWorkItem->RefCnt ++;
        //
        // Before enqueue this workitem, we have to reset this TrigerEvent to let PlatformFreeWorkitem to wait for it to complete.
        //
        NdisResetEvent(&pWorkItem->TrigerEvent);
        NdisQueueIoWorkItem(pWorkItem->handle, PlatformWorkitemCallBackFunc, pWorkItem);
        NdisReleaseSpinLock(&(pWorkItem->WorkItemLock));  
        bSuccess = TRUE;
    }
    else
    {
        DBGPRINT(RT_DEBUG_TRACE, ("PlatformShceduleWorktiem fail - pWorkItem->RefCnt == %d\n",pWorkItem->RefCnt));
        NdisReleaseSpinLock(&(pWorkItem->WorkItemLock));  
        return FALSE; 
    }
    
    return bSuccess;   
}

VOID
PlatformFreeWorkitem(
    IN  PMTK_WORKITEM   pWorkItem
    )
{
    ULONG       LoopCount = 0;

    if(pWorkItem == NULL)
    {
        DBGPRINT(RT_DEBUG_TRACE, ("PlatformShceduleWorktiem fail - pWorkItem == NULL\n"));
        return;
    }
    
    NdisAcquireSpinLock(&(pWorkItem->WorkItemLock));

    if((pWorkItem->pContext==NULL) || (pWorkItem->handle==NULL) ||(pWorkItem->RefCnt ==0))
    {
        NdisReleaseSpinLock(&(pWorkItem->WorkItemLock));
        DBGPRINT(RT_DEBUG_TRACE, ("PlatformFreeWorkitem Workitem already Free\n"));    
        return;
    }
    
    if(pWorkItem->bFreed == TRUE)
    {
        NdisReleaseSpinLock(&(pWorkItem->WorkItemLock));
        DBGPRINT(RT_DEBUG_TRACE, ("PlatformFreeWorkitem Workitem already Free\n"));
        return;
    }
    pWorkItem->RefCnt --;

    //Waiting for Workitem callbakc complete.

    if(pWorkItem->RefCnt >0)
    {
        NdisReleaseSpinLock(&(pWorkItem->WorkItemLock));
        
        while(TRUE )
        {
            LoopCount++;
            if(NdisWaitEvent(&(pWorkItem->TrigerEvent), 100))
            {
                NdisAcquireSpinLock(&(pWorkItem->WorkItemLock));
                NdisResetEvent(&(pWorkItem->TrigerEvent));          
                NdisReleaseSpinLock(&(pWorkItem->WorkItemLock)); 
                break;
            }
        }
    }
    else
    {
        NdisReleaseSpinLock(&(pWorkItem->WorkItemLock));
    }
    
    NdisFreeSpinLock(&(pWorkItem->WorkItemLock));
    
    NdisFreeIoWorkItem(pWorkItem->handle);
    pWorkItem->bFreed = TRUE;
}

