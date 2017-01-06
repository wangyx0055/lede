/***************************************************************************
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


/****************************************************************************

    Abstract:

    All related WMM UAPSD definitions & function prototype.

***************************************************************************/

#ifndef __UAPSD_H__
#define __UAPSD_H__


/* max UAPSD buffer queue size */
//#define MAX_PACKETS_IN_UAPSD_QUEUE    16  /* for each AC = 16*4 = 64 */
#define MAX_PACKETS_IN_UAPSD_QUEUE  1024    /* for each AC = 256*4 = 1024 */
#define UAPSD_IDLE_TIME         10  /* sec */
#define UAPSD_WIFI_IDLE_TIME        15  /* sec */
#define UAPSD_EPT_SP_INT            (100 * ONE_MILLISECOND_TIME) /* 100ms */


#define InsertTailQueueAc(pAd, pEntry, QueueHeader, QueueEntry)         \
{                                                                       \
    ((PQUEUE_ENTRY)QueueEntry)->Next = NULL;                            \
    if ((QueueHeader)->Tail)                                                \
        (QueueHeader)->Tail->Next = (PQUEUE_ENTRY)(QueueEntry);         \
    else                                                                    \
        (QueueHeader)->Head = (PQUEUE_ENTRY)(QueueEntry);               \
    (QueueHeader)->Tail = (PQUEUE_ENTRY)(QueueEntry);                   \
    (QueueHeader)->Number++;                                            \
}


/* Curently, only support UAPSD feature for P2PGO */
#define IS_AP_SUPPORT_UAPSD(__pAd, __pPort) \
    (((__pAd)->CommonCfg.bAPSDCapable)&&((__pAd)->CommonCfg.bWmmCapable)&&((__pPort)->PortSubtype == PORTSUBTYPE_P2PGO))

/* Check if the entry is  accessible to use TxSwUapsdQueue */
#define UAPSD_MR_IS_ENTRY_ACCESSIBLE( __pMacEntry)      \
    ((((__pMacEntry)->UAPSDQAid) < NUM_OF_UAPSD_CLI) && (CLIENT_STATUS_TEST_FLAG((__pMacEntry), fCLIENT_STATUS_APSD_CAPABLE)))

/* In the absent period of P2P NoA */
#define UAPSD_MR_IN_P2P_ABSENT(__pPort, __pMacEntry)      \
    (((__pMacEntry)->ValidAsCLI) && ((__pPort)->P2PCfg.GONoASchedule.bValid) && ((__pPort)->P2PCfg.GONoASchedule.bWMMPSInAbsent == TRUE))

/* In the present period of P2P NoA */
#define UAPSD_MR_IN_P2P_PRESENT(__pAd, __pMacEntry)     \
    (((__pMacEntry)->ValidAsCLI) && ((__pAd)->pP2pCtrll->P2PCfg.GONoASchedule.bValid) && ((__pAd)->pP2pCtrll->P2PCfg.GONoASchedule.bWMMPSInAbsent == FALSE))

/*
    Init MAC entry UAPSD parameters;
    purpose: initialize UAPSD PS queue and control parameters
*/
#define UAPSD_MR_ENTRY_INIT(__pEntry)                                       \
{                                                                           \
        UCHAR   __IdAc;                                                     \
        for(__IdAc=0; __IdAc<WMM_NUM_OF_AC; __IdAc++)                       \
            InitializeQueueHeader(&(__pEntry)->UAPSDQueue[__IdAc]);         \
        (__pEntry)->UAPSDTxNum = 0;                                         \
        (__pEntry)->bAPSDFlagSPStart = 0;                                   \
        (__pEntry)->MaxSPLength = 0;                                        \
        (__pEntry)->UAPSDQAid = 0xff;                                       \
}


/* Check if the AC is UAPSD delivery-enabled AC */
#define UAPSD_MR_IS_UAPSD_AC(__pMacEntry, __AcId)                           \
        (CLIENT_STATUS_TEST_FLAG((__pMacEntry), fCLIENT_STATUS_APSD_CAPABLE) && \
            ((0 <= (__AcId)) && ((__AcId) < WMM_NUM_OF_AC)) && /* 0 ~ 3 */  \
            (__pMacEntry)->bAPSDDeliverEnabledPerAC[(__AcId)])

/*
    Check if we do NOT need to control TIM bit for the station;
    note: we control TIM bit only when all AC are UAPSD AC
*/
#define UAPSD_MR_IS_NOT_TIM_BIT_NEEDED_HANDLED(__pMacEntry, __QueIdx)       \
        (CLIENT_STATUS_TEST_FLAG((__pMacEntry), fCLIENT_STATUS_APSD_CAPABLE) && \
            (!(__pMacEntry)->bAPSDDeliverEnabledPerAC[QID_AC_VO] ||         \
            !(__pMacEntry)->bAPSDDeliverEnabledPerAC[QID_AC_VI] ||          \
            !(__pMacEntry)->bAPSDDeliverEnabledPerAC[QID_AC_BE] ||          \
            !(__pMacEntry)->bAPSDDeliverEnabledPerAC[QID_AC_BK]) &&         \
        (__pMacEntry)->bAPSDDeliverEnabledPerAC[__QueIdx])


/* Check if all AC are UAPSD delivery-enabled AC */
#define UAPSD_MR_IS_ALL_AC_UAPSD(__FlgIsActive, __pMacEntry)                \
        (((__FlgIsActive) == FALSE) && ((__pMacEntry)->bAPSDAllAC == 1))


/*
    Enable or disable UAPSD flag in WMM element in beacon frame;
    purpose: set UAPSD enable/disable bit
*/
#define UAPSD_MR_IE_FILL(__QosCtrlField, __pAd)                             \
        (__QosCtrlField) |= ((__pAd)->CommonCfg.bAPSDCapable) ? 0x80 : 0x00;

//
// Prototype
//
VOID ApUapsdServicePeriodClose(
    IN  PMP_ADAPTER       pAd,
    IN  MAC_TABLE_ENTRY *pEntry,
    IN  BOOLEAN             bNoAInterrupt);

VOID ApUapsdAssocParse(
    IN  PMP_ADAPTER       pAd,
    IN PMP_PORT     pPort,
    IN  MAC_TABLE_ENTRY     *pEntry,
    IN  UCHAR               *pElm);

NDIS_STATUS 
ApUapsdPacketEnqueue(
    IN  PMP_ADAPTER       pAd,
    IN  MAC_TABLE_ENTRY *pEntry,
    IN  PMT_XMIT_CTRL_UNIT             pXcu,
    IN  UINT32              IdAc
    );

VOID ApUapsdQueueMaintenance(
    IN  PMP_ADAPTER       pAd,
    IN  MAC_TABLE_ENTRY     *pEntry);

VOID ApUapsdAllPacketDeliver(
    IN  PMP_ADAPTER       pAd,
    IN  MAC_TABLE_ENTRY     *pEntry);

VOID ApUapsdTriggerFrameHandle(
    IN  PMP_ADAPTER       pAd,
    IN  PMP_PORT          pPort,
    IN  MAC_TABLE_ENTRY     *pEntry,
    IN  UCHAR               UpOfFrame);

VOID ApUapsdCleanupUapsdQueue(
    IN  PMP_ADAPTER       pAd,
    IN  PMAC_TABLE_ENTRY    pMacEntry,
    IN  BOOLEAN             bResetAll);

ULONG ApUapsdGetNextPktNum(
    IN  PMP_ADAPTER       pAd,
    IN  PMP_PORT            pPort,
    IN  PMAC_TABLE_ENTRY    pMacEntry);

VOID    ApUapsdDeQueueUAPSDPacket(
    IN  PMP_ADAPTER   pAd,    
    IN  PMP_PORT            pPort,
    IN  UCHAR           BulkOutPipeId,
    PMAC_TABLE_ENTRY    pMacEntry);

BOOLEAN 
ApUapsdTagFrame(
    IN  MP_ADAPTER        *pAd,
    IN  PMT_XMIT_CTRL_UNIT     pXcu,
    IN  UCHAR               Wcid,
    IN  UINT32              PktOffset
    );

VOID ApUapsdUnTagFrame(
    IN  MP_ADAPTER    *pAd,
    IN  UCHAR           AcQueId,
    IN  UINT32          bulkStartPos,
    IN  UINT32          bulkEnPos);

VOID 
ApUapsdChangeFlagInNoA(
    IN  PMP_ADAPTER       pAd,
    IN  ULONG               RestTxNum,
    IN  PMT_XMIT_CTRL_UNIT     pXcu
    );

VOID ApUapsdSendNullFrame(
    IN  PMP_ADAPTER   pAd,
    IN  PMP_PORT      pPort,  
    IN  PUCHAR              pAddr,
    IN  UCHAR           TxRate, 
    IN  BOOLEAN         bQosNull,
    IN      BOOLEAN         bEOSP,
    IN  UCHAR           OldUP);

#endif // __UAPSD_H__
