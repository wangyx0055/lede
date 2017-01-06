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
    Thread.h

    Abstract:
    Miniport generic portion header file

    Revision History:
    Who         When          What
    --------    ----------    ----------------------------------------------
    */

#ifndef __THREAD_H__
#define __THREAD_H__


typedef struct _RTMP_ADAPTER MP_ADAPTER, *PMP_ADAPTER;
typedef struct _MP_PORT MP_PORT, *PMP_PORT;

#define THREAD_STATUS_NOT_INIT        0x00
#define THREAD_STATUS_INITED            0x01
#define THREAD_STATUS_RUNNING         0x02
#define THREAD_STATUS_CANCEL           0x04

#define GET_THREAD_EVENT(_TH)   ((_TH)->TrigerEvent)

typedef VOID
(*MT_TIMER_CALL_BACK)(
    PVOID   Reserved1,
    PVOID   Context,
    PVOID   Reserved2,
    PVOID   Reserved3
    );

// MTK timer control block
typedef struct  _MTK_TIMER_STRUCT    {
    BOOLEAN Valid;  // Set to True when call PlatformInitTimer
#if(COMPILE_WIN7_ABOVE(CURRENT_OS_NDIS_VER))  
    NDIS_HANDLE TimerObj;
#else
    NDIS_MINIPORT_TIMER TimerObj;   // Ndis Timer object
#endif
    ULONG                       TimerValue; // Timer value in milliseconds
    BOOLEAN                     State;  // True if timer cancelled
    BOOLEAN                      Repeat; // True if periodic timer
    MT_TIMER_CALL_BACK  CallBackFunc;
    PVOID                           pContext;   //pAd
    char                            szID[64];
    BOOLEAN                     bFreed; // if true, means it had be freed
    PVOID                         pOption;
    BOOLEAN                     bInputOption;
}   MTK_TIMER_STRUCT, *PMTK_TIMER_STRUCT;

typedef VOID
(*MT_THREAD_CALL_BACK)(
    IN  PVOID   pAd
    );

// MTK Thread control block
typedef struct  _MTK_THREAD_STRUCT    {
    HANDLE                  handle;
    BOOLEAN                 Valid;  // Set to True when call RTMPInitThrea
    BOOLEAN                 State;  // True if timer cancelled
    UCHAR                   InitCnt;
    MT_THREAD_CALL_BACK     CallBackFunc;
    PVOID               pContext;   //pAd
    char                    szID[64];
    BOOLEAN             bFreed; // if true, means it had be freed
    PKTHREAD        ThreadObj;
    NDIS_EVENT      CompleteEvent;
    NDIS_EVENT      TrigerEvent;
    
}   MTK_THREAD_STRUCT, *PMTK_THREAD_STRUCT;


typedef VOID
(*MTK_WORKITEM_CALLBACK)(
    PVOID   Context
    );

typedef struct _MTK_WORKITEM
{
#if (COMPILE_VISTA_ABOVE(CURRENT_OS_NDIS_VER))
    NDIS_HANDLE         handle;
#else
    NDIS_WORK_ITEM   handle;
#endif
    MTK_WORKITEM_CALLBACK   CallBackFunc;
    UCHAR                   RefCnt;
    PVOID                   pContext;
    char                    szID[64];
    BOOLEAN                 bFreed; // if true, means it had be freed
    NDIS_EVENT              TrigerEvent;
    NDIS_SPIN_LOCK          WorkItemLock;
    //
    // TODO: Trace if workItem has not excuted the CallBackFunc;
    //
    ULONG                   FiredCount;
}MTK_WORKITEM, *PMTK_WORKITEM;

BOOLEAN
PlatformWaitEventTriggeredAndReset(
    IN PNDIS_EVENT pEvent,
    IN UINT   Wait_ms
    );

VOID
    PlatformTimerCallBackFunc(
    IN  PVOID   SystemSpecific1,
    IN  PVOID   FunctionContext,
    IN  PVOID   SystemSpecific2,
    IN  PVOID   SystemSpecific3
    );

VOID
PlatformInitTimer(
    IN  PMP_PORT   pPort,
    IN  PMTK_TIMER_STRUCT    pTimer,
    IN  MT_TIMER_CALL_BACK  pTimerFunc,
    IN  BOOLEAN Repeat,
    IN  const char* szID
    );

VOID
PlatformSetTimer(
    IN  PMP_PORT   pPort,
    IN  PMTK_TIMER_STRUCT    pTimer,
    IN  ULONG   MillisecondsPeriod
    );

VOID    
PlatformCancelTimer(
    IN  PMTK_TIMER_STRUCT    pTimer,
    OUT BOOLEAN *pCancelled
    );

VOID
PlatformFreeTimer(
    IN  PMTK_TIMER_STRUCT    pTimer
    ); 
    
VOID 
MTUSBCmdHander(
    IN PMP_PORT      pPort,
    IN UINT             Cmd,
    IN PUCHAR         pBuff,
    IN ULONG          BuffLength
    );
    
VOID 
MTUSBCmdThread(
    IN  PMP_ADAPTER   pAd
    );    

VOID 
USBBulkOutDataThread0(
    IN  PMP_ADAPTER   pAd
    );

VOID 
USBBulkOutDataThread1(
    IN  PMP_ADAPTER   pAd
    );

VOID 
USBBulkOutDataThread2(
    IN  PMP_ADAPTER   pAd
    );

VOID 
USBBulkOutDataThread3(
    IN  PMP_ADAPTER   pAd
    );

VOID 
DequeueDataThread(
    IN  PMP_ADAPTER   pAd
    );    

//KSTART_ROUTINE  PlatformThreadCallBackFunc;   

VOID
PlatformThreadCallBackFunc(
    IN OUT PVOID Context
    );

VOID
PlatformInitThread(
    IN  PMP_ADAPTER   pAd,
    IN  PMTK_THREAD_STRUCT    pThread,
    IN  MT_THREAD_CALL_BACK  pThreadFunc,
    IN  const char* szID
    );

NDIS_STATUS
PlatformRunThread(
    IN  PMP_ADAPTER   pAd,
    IN  PMTK_THREAD_STRUCT    pThread
    );
    
VOID
PlatformCancelThread(
    IN  PMP_ADAPTER   pAd,
    IN  PMTK_THREAD_STRUCT    pThread
    );

VOID
PlatformFreeThread(
    IN  PMP_ADAPTER   pAd,
    IN  PMTK_THREAD_STRUCT    pThread
    );    

VOID
PlatformWorkitemCallBackFunc(
    IN PVOID Context,
    IN NDIS_HANDLE  handle
    );

NDIS_STATUS
PlatformInitWorktiem(
    IN  PMP_PORT   pAd,
    IN  PMTK_WORKITEM   pWorkItem,
    IN  MTK_WORKITEM_CALLBACK CallBackFunc,
    IN  const char* szID
    );   

BOOLEAN
PlatformScheduleWorktiem(
    IN  PMTK_WORKITEM   pWorkItem
    );

VOID
PlatformFreeWorkitem(
    IN  PMTK_WORKITEM   pWorkItem
    );    
#endif
