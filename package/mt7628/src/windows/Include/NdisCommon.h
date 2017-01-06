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
    NdisCommon.h

    Abstract:
    Miniport generic portion header file

    Revision History:
    Who         When          What
    --------    ----------    ----------------------------------------------
    */

#ifndef __NDISCOMMON_H__
#define __NDISCOMMON_H__

#define IS_SUPPORT_THREE_MBSSID(_pAd) (IS_MT7603(_pAd))

typedef struct _RTMP_ADAPTER MP_ADAPTER, *PMP_ADAPTER;
typedef struct  _MGMT_STRUC MGMT_STRUC, *PMGMT_STRUC;
typedef struct _ADAPTER_INITIAL_STAGE   ADAPTER_INITIAL_STAGE,  *PADAPTER_INITIAL_STAGE;
typedef struct _MP_PORT MP_PORT, *PMP_PORT;
typedef struct _MP_XMIT_CTRL_UNIT MT_XMIT_CTRL_UNIT, *PMT_XMIT_CTRL_UNIT;

typedef struct _MP_RW_LOCK_STATE MP_RW_LOCK_STATE, *PMP_RW_LOCK_STATE;

// Unify all delay routine by using NdisStallExecution
__inline    VOID    Delay_us(
    IN      ULONG   usec)
{
    ULONG   i;

    for (i = 0; i < (usec / 50); i++)
        NdisStallExecution(50);

    if (usec % 50)
        NdisStallExecution(usec % 50);
}

VOID PlatformAcquireOldRWLockForWrite(
    PNDIS_RW_LOCK       Lock,
    PMP_RW_LOCK_STATE       LockState) ;

VOID PlatformAcquireOldRWLockForRead(
    PNDIS_RW_LOCK     Lock,
    PMP_RW_LOCK_STATE       LockState) ;

VOID PlatformReleaseOldRWLock(
    PNDIS_RW_LOCK       Lock,
    PMP_RW_LOCK_STATE       LockState);

NDIS_STATUS 
NdisCommonCreateAllThreads(
    IN  PMP_ADAPTER   pAd
    );

VOID 
NdisCommonFreeAllThreads(
    IN  PMP_ADAPTER   pAd
    );

VOID 
NdisCommonCreateAllWorkitems(
    IN  PMP_ADAPTER   pAd
    );

VOID 
NdisCommonFreeAllWorkitems(
    IN  PMP_ADAPTER   pAd
    );    

VOID 
NdisCommonGenericDelay(
    IN      ULONG   usec
    );  

NTSTATUS
NdisCommonCreateTxDataThread(
    IN  PMP_ADAPTER   pAd
    );


NDIS_STATUS
NdisCommonStartTxRx(
    IN PMP_ADAPTER pAd,
    OUT PADAPTER_INITIAL_STAGE  pInitialStage
    );
    
NDIS_STATUS
NdisCommonStartTx(
    IN PMP_ADAPTER pAd
    );

NDIS_STATUS
NdisCommonStartRx(
    IN PMP_ADAPTER pAd
    );    

VOID
NdisCommonStopTx(
    IN PMP_ADAPTER pAd
    );

VOID
NdisCommonStopRx(
    IN PMP_ADAPTER pAd
    );

VOID
NdisCommonBulkRx(
    IN PMP_ADAPTER pAd
    );

VOID
NdisCommonBulkRxCmd(
    IN PMP_ADAPTER pAd
    );    

NDIS_STATUS
NdisCommonInitTransmit(
    IN PMP_ADAPTER pAd
    );

NDIS_STATUS
NdisCommonInitRecv(
    IN PMP_ADAPTER pAd
    );    

VOID
NdisCommonFreeNicRecv(
    IN PMP_ADAPTER pAd
    );   

VOID
NdisCommonFreeNICTransmit(
    IN PMP_ADAPTER pAd
    );  

VOID
NdisCommonCleanUpMLMEWaitQueue(
    IN PMP_ADAPTER pAd
    );

VOID
NdisCommonCleanUpMLMEBulkOutQueue(
    IN PMP_ADAPTER pAd
    );   

VOID
NdisCommonWaitPendingRequest(
    IN PMP_ADAPTER pAd
    );  

VOID
NdisCommonDeQueuePacket(
    IN  PMP_ADAPTER   pAd,    
    IN  UCHAR           BulkOutPipeId
    );

NDIS_STATUS 
NdisCommonFreeDescriptorRequest(
    IN  PMP_ADAPTER   pAd,
    IN  UCHAR           RingType,
    IN  UCHAR           BulkOutPipeId,
    IN  ULONG           NumberRequired
    );    

VOID    
NdisCommonMlmeHardTransmit(
    IN  PMP_ADAPTER   pAd,
    IN  PMGMT_STRUC     pMgmt
    );    

VOID    
NdisCommonDequeueMLMEPacket(
    IN  PMP_ADAPTER   pAd
    );  

VOID    
NdisCommonMiniportMMRequest(
    IN  PMP_ADAPTER   pAd,    
    IN  PVOID           pBuffer,
    IN  ULONG           Length
    );

VOID    
NdisCommonRejectPendingPackets(
    IN  PMP_ADAPTER   pAd
    );   

NDIS_STATUS
NdisCommonPortInitandCfg(
    IN  PMP_ADAPTER pAd
    );

NDIS_STATUS
NdisCommonInitializeSWCtrl (
    IN  PMP_ADAPTER pAd,
    OUT PADAPTER_INITIAL_STAGE  pInitialStage
    );

VOID    
NdisCommonAllocateAllSpinLock(
    IN  PMP_ADAPTER   pAd
    );

VOID    
NdisCommonDeallocateAllSpinLock(
    IN PMP_ADAPTER pAd
    );     

VOID 
NdisCommonInfoResetHandler(
    IN  PMP_PORT pPort
    );    

size_t
NdisCommonGetPipeNumber(
    IN PMP_ADAPTER pAd
    );    

VOID
NdisCommonMappingPortToBssIdx(
    IN  PMP_PORT    pPort,
    IN  UCHAR     OwnMacOption,
    IN  UCHAR     BSSMacOption
    );

UCHAR
NdisCmmonMoreBSSID(
    IN  PMP_ADAPTER pAd
);

ULONG
NdisCommonCopyPayloadFromXmitBuffer(
    IN PVOID Dest,
    IN PMT_XMIT_CTRL_UNIT pXcu
    );

ULONG
NdisCommonCopyBufferFromXmitBuffer(
    IN PVOID Dest,
    IN PMT_XMIT_CTRL_UNIT pXcu
    );

VOID
NdisCommonReportEthFrameToLLC(
    IN  PMP_ADAPTER   pAd,
    IN  PMP_PORT      pPort,
    IN  PUCHAR          p80211DataFrame,
    IN  USHORT          us80211FrameSize,
    IN  UCHAR           UserPriority,
    IN  BOOLEAN         CRCError,
    IN  BOOLEAN         bSingleIndicate
    );

VOID
NdisCommonReportEthFrameToLLCAgg(
    IN  PMP_ADAPTER  pAd,
    IN  PMP_PORT  pPort,
    IN  PUCHAR  p80211Header,
    IN  USHORT  Payload1Size,
    IN  USHORT  Payload2Size,
    IN  UCHAR   UserPriority
    );

VOID
NdisCommonReportAMSDUEthFrameToLLC(
    IN  PMP_ADAPTER   pAd,
    IN  PMP_PORT      pPort,
    IN  PUCHAR          p80211DataFrame,
    IN  USHORT          us80211FrameSize,
    IN  UCHAR           UserPriority,
    IN  BOOLEAN         bSingleIndicate
    );
    
#endif    