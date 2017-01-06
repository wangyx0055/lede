/*
 ***************************************************************************
 * MediaTek Inc.
 *
 * All rights reserved. source code is an unpublished work and the
 * use of a copyright notice does not imply otherwise. This source code
 * contains confidential trade secret material of MediaTek. Any attemp
 * or participation in deciphering, decoding, reverse engineering or in any
 * way altering the source code is stricitly prohibited, unless the prior
 * written consent of MediaTek, Inc. is obtained.
 ***************************************************************************

	Module Name:
	mtsdio_data.c
*/

#include	"rt_config.h"


/* <-------------- to be removed to another header file */
void MTSDIOProcessSoftwareInterrupt(IN PRTMP_ADAPTER prAd);
void MTSDIOProcessAbnormalInterrupt(IN PRTMP_ADAPTER prAd);
void MTSDIOProcessTxInterrupt(IN PRTMP_ADAPTER prAd);
void MTSDIOProcessRxInterrupt(IN PRTMP_ADAPTER prAd);

typedef struct _INT_EVENT_MAP_T {
    UINT32      u4Int;
    UINT32      u4Event;
} INT_EVENT_MAP_T, *P_INT_EVENT_MAP_T;

typedef VOID (*IST_EVENT_FUNCTION)(PRTMP_ADAPTER);

enum ENUM_INT_EVENT_T {
    INT_EVENT_ABNORMAL,
    INT_EVENT_SW_INT,
    INT_EVENT_TX,
    INT_EVENT_RX,
    INT_EVENT_NUM
};

#define PORT_INDEX_LMAC                         0
#define PORT_INDEX_MCU                          1

/* MCU quque index */
typedef enum _ENUM_MCU_Q_INDEX_T {
    MCU_Q0_INDEX = 0,
    MCU_Q1_INDEX,
    MCU_Q2_INDEX,
    MCU_Q3_INDEX,
    MCU_Q_NUM
} ENUM_MCU_Q_INDEX_T;

/* LMAC Tx queue index */
typedef enum _ENUM_MAC_TXQ_INDEX_T {
    MAC_TXQ_AC0_INDEX = 0,
    MAC_TXQ_AC1_INDEX,
    MAC_TXQ_AC2_INDEX,
    MAC_TXQ_AC3_INDEX,
    MAC_TXQ_AC4_INDEX,
    MAC_TXQ_AC5_INDEX,
    MAC_TXQ_AC6_INDEX,
    MAC_TXQ_BCN_INDEX,
    MAC_TXQ_BMC_INDEX,
    MAC_TXQ_AC10_INDEX,
    MAC_TXQ_AC11_INDEX,
    MAC_TXQ_AC12_INDEX,
    MAC_TXQ_AC13_INDEX,
    MAC_TXQ_AC14_INDEX,
    MAC_TXQ_NUM
} ENUM_MAC_TXQ_INDEX_T;


typedef struct _TX_RESOURCE_CONTROL_T {
    /* HW TX queue definition */
    UINT8      ucDestPortIndex;
    UINT8      ucDestQueueIndex;
    /* HIF Interrupt status index*/
    UINT8      ucHifTxQIndex;
} TX_RESOURCE_CONTROL_T, *PTX_RESOURCE_CONTROL_T;

static const TX_RESOURCE_CONTROL_T arTcResourceControl[TC_NUM] = {
    /* dest port index, dest queue index,   HIF TX queue index */
    /* First HW queue */
    {PORT_INDEX_LMAC,   MAC_TXQ_AC0_INDEX,  HIF_TX_AC1_INDEX},
    {PORT_INDEX_LMAC,   MAC_TXQ_AC1_INDEX,  HIF_TX_AC0_INDEX},
    {PORT_INDEX_LMAC,   MAC_TXQ_AC2_INDEX,  HIF_TX_AC2_INDEX},
    {PORT_INDEX_LMAC,   MAC_TXQ_AC3_INDEX,  HIF_TX_AC3_INDEX},
    {PORT_INDEX_LMAC,   MAC_TXQ_BMC_INDEX,  HIF_TX_BMC_INDEX},
    {PORT_INDEX_LMAC,   MAC_TXQ_AC4_INDEX,  HIF_TX_AC4_INDEX},
    {PORT_INDEX_MCU,    MCU_Q1_INDEX,       HIF_TX_CPU_INDEX},
    {PORT_INDEX_LMAC,   MAC_TXQ_BCN_INDEX,  HIF_TX_BCN_INDEX},
#if 0 /* second interface not used currently */
    /* Second HW queue */
    {PORT_INDEX_LMAC,   MAC_TXQ_AC10_INDEX, HIF_TX_AC10_INDEX},
    {PORT_INDEX_LMAC,   MAC_TXQ_AC11_INDEX, HIF_TX_AC11_INDEX},
    {PORT_INDEX_LMAC,   MAC_TXQ_AC12_INDEX, HIF_TX_AC12_INDEX},
    {PORT_INDEX_LMAC,   MAC_TXQ_AC13_INDEX, HIF_TX_AC13_INDEX},
    {PORT_INDEX_LMAC,   MAC_TXQ_AC14_INDEX, HIF_TX_AC14_INDEX},
#endif
};

static INT_EVENT_MAP_T arIntEventMapTable[] = {
    {ABNORMAL_INT,                         INT_EVENT_ABNORMAL},
    {D2H_SW_INT,                           INT_EVENT_SW_INT},
    {TX_DONE_INT,                          INT_EVENT_TX},
    {(RX0_DONE_INT | RX1_DONE_INT),        INT_EVENT_RX}

};


/* --------------- end ----------------------------------> */

static const UINT8 ucIntEventMapSize = (sizeof(arIntEventMapTable) / sizeof(INT_EVENT_MAP_T));

static IST_EVENT_FUNCTION apfnEventFuncTable[] = {
    MTSDIOProcessAbnormalInterrupt,            /*!< INT_EVENT_ABNORMAL */
    MTSDIOProcessSoftwareInterrupt,            /*!< INT_EVENT_SW_INT   */
    MTSDIOProcessTxInterrupt,                  /*!< INT_EVENT_TX       */
    MTSDIOProcessRxInterrupt,                  /*!< INT_EVENT_RX       */
};


VOID
MTSDIORxInitialize (
        IN PRTMP_ADAPTER prAd
)
{
	P_RX_CTRL_T prRxCtrl = NULL;
	P_SW_RFB_T prSwRfb;
	UINT32 i;
	unsigned long flags = 0;

    ASSERT(prAd);
    prRxCtrl = &prAd->rRxCtrl;

	prRxCtrl->pucRxCoalescingBufPtr = prAd->pucCoalescingBufCached;

#if (CFG_SDIO_INTR_ENHANCE == 1)

	os_alloc_mem(prAd, (UCHAR **)&prAd->penhanced_status, sizeof(ENHANCE_MODE_DATA_STRUCT_T));

	InitializeQueueHeader(&prRxCtrl->rFreeSwRfbList);
	InitializeQueueHeader(&prRxCtrl->rReceivedRfbList);

	NdisAllocateSpinLock(prAd, &prRxCtrl->rFreeSwRfbLock);
	NdisAllocateSpinLock(prAd, &prRxCtrl->rReceivedRfbLock);

	for (i = SDIO_RX_RFB_NUM; i != 0; i--) {

		os_alloc_mem(prAd, (UCHAR **)&prSwRfb, sizeof(SW_RFB_T));
		if (!prSwRfb)
		{
			MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("%s: MEM ALLOC ERROR\n", __FUNCTION__));
			break;
		}

		prSwRfb->prRxPacket = RTMP_AllocateFragPacketBuffer(prAd, MAX_FRAME_SIZE);
		if (prSwRfb->prRxPacket == NULL)
		{
			MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,("%s():Cannot Allocate sk buffer for this Bulk-In buffer!\n", __FUNCTION__));
			break;
		}
		RTMP_IRQ_LOCK(&prRxCtrl->rFreeSwRfbLock, flags);
		InsertTailQueue(&prRxCtrl->rFreeSwRfbList, &prSwRfb->rQueEntry);
		RTMP_IRQ_UNLOCK(&prRxCtrl->rFreeSwRfbLock, flags);
	}

#endif

    return;
}

VOID
MTSDIOTxInitialize (
        IN PRTMP_ADAPTER prAd
)
{
	P_TX_CTRL_T prTxCtrl = NULL;

	ASSERT(prAd);

	prTxCtrl = &prAd->rTxCtrl;

	//   os_alloc_mem(prAd, &prTxCtrl->pucTxCoalescingBufPtr, SDIO_TX_MAX_SIZE_PER_FRAME);
	prTxCtrl->pucTxCoalescingBufPtr = prAd->pucCoalescingBufCached;

	NdisAllocateSpinLock(prAd, &prAd->TcCountLock);

	/* Reset the TC quota for F/W DL phase */
	MTSDIOTxInitResetResource(prAd);

	return;
}

VOID
MTSDIOTxUninitialize(
        IN PRTMP_ADAPTER prAd
)
{
	P_TX_CTRL_T prTxCtrl = NULL;

	ASSERT(prAd);
	prTxCtrl = &prAd->rTxCtrl;
	prTxCtrl->pucTxCoalescingBufPtr = NULL;

    	return;
}

VOID
MTSDIORxUninitialize(
        IN PRTMP_ADAPTER prAd
)
{
	P_RX_CTRL_T prRxCtrl = NULL;
	ULONG IrqFlags = 0;
	P_SW_RFB_T prSwRfb;
	QUEUE_ENTRY *pEntry;

	ASSERT(prAd);
	prRxCtrl = &prAd->rRxCtrl;

#if (CFG_SDIO_INTR_ENHANCE == 1)
    os_free_mem(prAd, prAd->penhanced_status);

	do {
		RTMP_IRQ_LOCK(&prRxCtrl->rFreeSwRfbLock, IrqFlags);
		pEntry = RemoveHeadQueue(&prRxCtrl->rFreeSwRfbList);
		prSwRfb = (P_SW_RFB_T)pEntry;
		RTMP_IRQ_UNLOCK(&prRxCtrl->rFreeSwRfbLock, IrqFlags);

		if (prSwRfb){
			if (prSwRfb->prRxPacket) {
				os_free_mem(prAd, prSwRfb->prRxPacket);
			}
			prSwRfb->prRxPacket = NULL;
		}
		else {
			break;
		}
	}while (TRUE);

	do {
		RTMP_IRQ_LOCK(&prRxCtrl->rReceivedRfbLock, IrqFlags);
		pEntry = RemoveHeadQueue(&prRxCtrl->rReceivedRfbList);
		prSwRfb = (P_SW_RFB_T)pEntry;
		RTMP_IRQ_UNLOCK(&prRxCtrl->rReceivedRfbLock, IrqFlags);

		if (prSwRfb){
			if (prSwRfb->prRxPacket) {
				os_free_mem(prAd, prSwRfb->prRxPacket);
			}
			prSwRfb->prRxPacket = NULL;
		}
		else {
			break;
		}
	}while (TRUE);


	NdisFreeSpinLock(&prRxCtrl->rFreeSwRfbLock);
	NdisFreeSpinLock(&prRxCtrl->rReceivedRfbLock);

#endif
	prRxCtrl->pucRxCoalescingBufPtr = NULL;

/*
	free prRxCtrl->pucRxCoalescingBufPtr(prAd->pucCoalescingBufCached)
	in RTMPFreeTxRxRingMemory
*/
    return;
}


VOID MTSDIOTxInitResetResource (
        IN PRTMP_ADAPTER prAd
)
{
    P_TX_CTRL_T prTxCtrl = NULL;
    unsigned long flags = 0;
	int count=0;

    ASSERT(prAd);

    prTxCtrl = &prAd->rTxCtrl;

    RTMP_SPIN_LOCK_IRQSAVE(&prAd->TcCountLock, &flags);

    NdisZeroMemory(prTxCtrl->rTc.au2TxDonePageCount, sizeof(prTxCtrl->rTc.au2TxDonePageCount));

    prTxCtrl->rTc.ucNextTcIdx = TC0_INDEX;
    prTxCtrl->rTc.u2AvailiablePageCount = 0;
    prTxCtrl->rTc.au2MaxNumOfPage[TC0_INDEX] = SDIO_TX_PAGE_COUNT_TC0;
    prTxCtrl->rTc.au2FreePageCount[TC0_INDEX] = SDIO_TX_PAGE_COUNT_TC0;

    prTxCtrl->rTc.au2MaxNumOfPage[TC1_INDEX] = SDIO_TX_PAGE_COUNT_TC1;
    prTxCtrl->rTc.au2FreePageCount[TC1_INDEX] = SDIO_TX_PAGE_COUNT_TC1;

    prTxCtrl->rTc.au2MaxNumOfPage[TC2_INDEX] = SDIO_TX_PAGE_COUNT_TC2;
    prTxCtrl->rTc.au2FreePageCount[TC2_INDEX] = SDIO_TX_PAGE_COUNT_TC2;

    prTxCtrl->rTc.au2MaxNumOfPage[TC3_INDEX] = SDIO_TX_PAGE_COUNT_TC3;
    prTxCtrl->rTc.au2FreePageCount[TC3_INDEX] = SDIO_TX_PAGE_COUNT_TC3;

    prTxCtrl->rTc.au2MaxNumOfPage[TC4_INDEX] = SDIO_TX_PAGE_COUNT_TC4;
    prTxCtrl->rTc.au2FreePageCount[TC4_INDEX] = SDIO_TX_PAGE_COUNT_TC4;

    prTxCtrl->rTc.au2MaxNumOfPage[TC5_INDEX] = SDIO_TX_PAGE_COUNT_TC5;
    prTxCtrl->rTc.au2FreePageCount[TC5_INDEX] = SDIO_TX_PAGE_COUNT_TC5;

	prTxCtrl->rTc.au2MaxNumOfPage[TC6_INDEX] = SDIO_TX_PAGE_COUNT_TC6;
	prTxCtrl->rTc.au2FreePageCount[TC6_INDEX] = SDIO_TX_PAGE_COUNT_TC6;

    prTxCtrl->rTc.au2MaxNumOfPage[TC7_INDEX] = SDIO_TX_PAGE_COUNT_TC7;
    prTxCtrl->rTc.au2FreePageCount[TC7_INDEX] = SDIO_TX_PAGE_COUNT_TC7;

#if 0
    prTxCtrl->rTc.au2MinReservedTcResource[TC0_INDEX] = SDIO_MIN_RESERVED_TC0_RESOURCE;
   	prTxCtrl->rTc.au2MinReservedTcResource[TC1_INDEX] = SDIO_MIN_RESERVED_TC1_RESOURCE;
   	prTxCtrl->rTc.au2MinReservedTcResource[TC2_INDEX] = SDIO_MIN_RESERVED_TC2_RESOURCE;
   	prTxCtrl->rTc.au2MinReservedTcResource[TC3_INDEX] = SDIO_MIN_RESERVED_TC3_RESOURCE;
   	prTxCtrl->rTc.au2MinReservedTcResource[TC4_INDEX] = SDIO_MIN_RESERVED_TC4_RESOURCE;
   	prTxCtrl->rTc.au2MinReservedTcResource[TC5_INDEX] = SDIO_MIN_RESERVED_TC5_RESOURCE;
	prTxCtrl->rTc.au2MinReservedTcResource[TC6_INDEX] = SDIO_MIN_RESERVED_TC6_RESOURCE;
#endif
#ifdef MT_SDIO_ADAPTIVE_TC_RESOURCE_CTRL
	prTxCtrl->rTc.au2GuaranteedTcResource[TC0_INDEX] = SDIO_GUARANTEED_TC0_RESOURCE;
	prTxCtrl->rTc.au2GuaranteedTcResource[TC1_INDEX] = SDIO_GUARANTEED_TC1_RESOURCE;
	prTxCtrl->rTc.au2GuaranteedTcResource[TC2_INDEX] = SDIO_GUARANTEED_TC2_RESOURCE;
	prTxCtrl->rTc.au2GuaranteedTcResource[TC3_INDEX] = SDIO_GUARANTEED_TC3_RESOURCE;
	prTxCtrl->rTc.au2GuaranteedTcResource[TC4_INDEX] = SDIO_GUARANTEED_TC4_RESOURCE;
	prTxCtrl->rTc.au2GuaranteedTcResource[TC5_INDEX] = SDIO_GUARANTEED_TC5_RESOURCE;
	prTxCtrl->rTc.au2GuaranteedTcResource[TC6_INDEX] = SDIO_GUARANTEED_TC6_RESOURCE;
    prTxCtrl->rTc.au2GuaranteedTcResource[TC7_INDEX] = SDIO_GUARANTEED_TC7_RESOURCE;
    for( count=0; count < NUM_ADJUSTABLE_TC; count++)
	{
		prTxCtrl->rTc.au2AvgDemand[count]=0;
		prTxCtrl->rTc.au2CurrentTcResource[count]=0;
	}
	prTxCtrl->rTc.u2TimeToAdjustTcResource = SDIO_INIT_TIME_TO_ADJUST_TC_RSC;
	prTxCtrl->rTc.u2TimeToUpdateQueLen = SDIO_INIT_TIME_TO_UPDATE_QUE_LEN;

	prTxCtrl->rTc.fgTcResourcePostAnnealing = FALSE;
#endif /* !MT_SDIO_ADAPTIVE_TC_RESOURCE_CTRL */

    RTMP_SPIN_UNLOCK_IRQRESTORE(&prAd->TcCountLock, &flags);

    return;
}

VOID
MTSDIOTxResetResource (
        IN PRTMP_ADAPTER prAd
)
{
    P_TX_CTRL_T prTxCtrl = NULL;
    unsigned long flags = 0;

    ASSERT(prAd);

    prTxCtrl = &prAd->rTxCtrl;

    RTMP_SPIN_LOCK_IRQSAVE(&prAd->TcCountLock, &flags);

    NdisZeroMemory(prTxCtrl->rTc.au2TxDonePageCount, sizeof(prTxCtrl->rTc.au2TxDonePageCount));
    prTxCtrl->rTc.ucNextTcIdx = TC0_INDEX;
    prTxCtrl->rTc.u2AvailiablePageCount = 0;

    prTxCtrl->rTc.au2MaxNumOfPage[TC0_INDEX] = SDIO_TX_PAGE_COUNT_TC0;
    prTxCtrl->rTc.au2FreePageCount[TC0_INDEX] = SDIO_TX_PAGE_COUNT_TC0;

    prTxCtrl->rTc.au2MaxNumOfPage[TC1_INDEX] = SDIO_TX_PAGE_COUNT_TC1;
    prTxCtrl->rTc.au2FreePageCount[TC1_INDEX] = SDIO_TX_PAGE_COUNT_TC1;

    prTxCtrl->rTc.au2MaxNumOfPage[TC2_INDEX] = SDIO_TX_PAGE_COUNT_TC2;
    prTxCtrl->rTc.au2FreePageCount[TC2_INDEX] = SDIO_TX_PAGE_COUNT_TC2;

    prTxCtrl->rTc.au2MaxNumOfPage[TC3_INDEX] = SDIO_TX_PAGE_COUNT_TC3;
    prTxCtrl->rTc.au2FreePageCount[TC3_INDEX] = SDIO_TX_PAGE_COUNT_TC3;

    prTxCtrl->rTc.au2MaxNumOfPage[TC4_INDEX] = SDIO_TX_PAGE_COUNT_TC4;
    prTxCtrl->rTc.au2FreePageCount[TC4_INDEX] = SDIO_TX_PAGE_COUNT_TC4;

    prTxCtrl->rTc.au2MaxNumOfPage[TC5_INDEX] = SDIO_TX_PAGE_COUNT_TC5;
    prTxCtrl->rTc.au2FreePageCount[TC5_INDEX] = SDIO_TX_PAGE_COUNT_TC5;

    prTxCtrl->rTc.au2MaxNumOfPage[TC6_INDEX] = SDIO_TX_PAGE_COUNT_TC6;
    prTxCtrl->rTc.au2FreePageCount[TC6_INDEX] = SDIO_TX_PAGE_COUNT_TC6;

    prTxCtrl->rTc.au2MaxNumOfPage[TC7_INDEX] = SDIO_TX_PAGE_COUNT_TC7;
    prTxCtrl->rTc.au2FreePageCount[TC7_INDEX] = SDIO_TX_PAGE_COUNT_TC7;

    RTMP_SPIN_UNLOCK_IRQRESTORE(&prAd->TcCountLock, &flags);

    return;
}

UINT8
MTSDIOTxGetCmdResourceType(
        IN PRTMP_ADAPTER prAd
)
{
    return TC6_INDEX;
}

INT32
MTSDIOTxAcquireResource(
        IN PRTMP_ADAPTER prAd,
    IN UINT8       ucTC,
    IN UINT8       ucPageCount
)
{
    P_TX_CTRL_T prTxCtrl = NULL;
    INT32 u4Ret = NDIS_STATUS_FAILURE;
    unsigned long flags = 0;

    prTxCtrl = &prAd->rTxCtrl;
    MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_INFO, ("b MTSDIOTxAcquireResource ucTC:%x ucPageCount: %x fp:%x\n",
                            ucTC,ucPageCount,prTxCtrl->rTc.au2FreePageCount[ucTC]));

    RTMP_SPIN_LOCK_IRQSAVE(&prAd->TcCountLock, &flags);
    if (prTxCtrl->rTc.au2FreePageCount[ucTC] >= ucPageCount) {
        prTxCtrl->rTc.au2FreePageCount[ucTC] -= ucPageCount;
#ifdef MT_SDIO_ADAPTIVE_TC_RESOURCE_CTRL
#if DEBUG_ADAPTIVE_QUOTA
        MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRAC, ("%s: ucTC:%x ucPageCount: %x fp:%x\n",
                                __func__, ucTC, ucPageCount, prTxCtrl->rTc.au2FreePageCount[ucTC]));
#endif
#endif
        u4Ret = NDIS_STATUS_SUCCESS;
    } else {
        MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("%s: No Tx resource!!! ucTC:%x ucPageCount: %x fp:%x\n",
                                __func__, ucTC, ucPageCount, prTxCtrl->rTc.au2FreePageCount[ucTC]));
    }
    RTMP_SPIN_UNLOCK_IRQRESTORE(&prAd->TcCountLock, &flags);
    MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_INFO, ("a MTSDIOTxAcquireResource ucTC:%x ucPageCount: %x fp:%x\n",
                            ucTC,ucPageCount,prTxCtrl->rTc.au2FreePageCount[ucTC]));

    return u4Ret;
}

ULONG
MTSDIOTxQueryPageResource(
        IN PRTMP_ADAPTER prAd,
    IN UINT8       ucTC
)
{
    P_TX_CTRL_T prTxCtrl = NULL;
    //unsigned long flags = 0;

    prTxCtrl = &prAd->rTxCtrl;

    MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_INFO, ("%s: Query TC:%d ,Free Page Count %d\n", __func__, ucTC, prTxCtrl->rTc.au2FreePageCount[ucTC]));

    /* For read, we don't need to acquire spin lock */
    return prTxCtrl->rTc.au2FreePageCount[ucTC];
}

EXPORT_SYMBOL(MTSDIOTxQueryPageResource);


UINT8
MTSDIOTxGetPageCount (
    IN UINT32 u4FrameLength,
    IN BOOLEAN fgIncludeDesc
    )
{
    UINT32 u4RequiredBufferSize = 0;
    UINT8  ucPageCount = 0;

  /* Frame Buffer
   *  |<--Tx Descriptor-->|<--Tx descriptor padding-->|<--802.3/802.11 Header-->|<--Header padding-->|<--Payload-->|
   */
    if(fgIncludeDesc) {
        u4RequiredBufferSize = u4FrameLength;
    }
    else {
        u4RequiredBufferSize =
            SDIO_TX_DESC_LONG_FORMAT_LENGTH +
            SDIO_TX_DESC_PADDING_LENGTH +
            u4FrameLength;
    }

    if(SDIO_TX_PAGE_SIZE_IS_POWER_OF_2) {
        ucPageCount = (UINT8)((u4RequiredBufferSize + (SDIO_TX_PAGE_SIZE - 1)) >> SDIO_TX_PAGE_SIZE_IN_POWER_OF_2);
    }
    else {
        ucPageCount = (UINT8)((u4RequiredBufferSize + (SDIO_TX_PAGE_SIZE - 1)) / SDIO_TX_PAGE_SIZE);
    }

    return ucPageCount;
}

static BOOLEAN
MTSDIOTxCalculateResource(
        IN PRTMP_ADAPTER prAd,
    IN UINT16 *au2TxRlsCnt,
    OUT UINT16 *au2FreeTcResource
)
{
    P_TX_TCQ_STATUS_T prTcqStatus = NULL;
    UINT16 u2TotalTxDonePageCount = 0;
    UINT8 ucTcIdx = 0;
    BOOLEAN bStatus = FALSE;

    prTcqStatus = &prAd->rTxCtrl.rTc;

    prTcqStatus->u2AvailiablePageCount += au2TxRlsCnt[HIF_TX_FFA_INDEX];

    for (ucTcIdx = TC0_INDEX; ucTcIdx < TC_NUM; ucTcIdx++) {
        prTcqStatus->au2TxDonePageCount[ucTcIdx] += au2TxRlsCnt[arTcResourceControl[ucTcIdx].ucHifTxQIndex];
        u2TotalTxDonePageCount += prTcqStatus->au2TxDonePageCount[ucTcIdx];
    }

    MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("Tx Done INT result, FFA[%u] AC[%u:%u:%u:%u:%u] BCN[%u] BMC[%u] CPU[%u]\n",
                au2TxRlsCnt[HIF_TX_FFA_INDEX],
                au2TxRlsCnt[HIF_TX_AC0_INDEX],
                au2TxRlsCnt[HIF_TX_AC1_INDEX],
                au2TxRlsCnt[HIF_TX_AC2_INDEX],
                au2TxRlsCnt[HIF_TX_AC3_INDEX],
                au2TxRlsCnt[HIF_TX_AC4_INDEX],
                au2TxRlsCnt[HIF_TX_BCN_INDEX],
                au2TxRlsCnt[HIF_TX_BMC_INDEX],
                au2TxRlsCnt[HIF_TX_CPU_INDEX]));


    if (prTcqStatus->u2AvailiablePageCount) {
        if (prTcqStatus->u2AvailiablePageCount >= u2TotalTxDonePageCount) {

            NdisCopyMemory(au2FreeTcResource, prTcqStatus->au2TxDonePageCount,
                    sizeof(prTcqStatus->au2TxDonePageCount));
            NdisZeroMemory(prTcqStatus->au2TxDonePageCount,
                    sizeof(prTcqStatus->au2TxDonePageCount));
            prTcqStatus->u2AvailiablePageCount -= u2TotalTxDonePageCount;

            if (prTcqStatus->u2AvailiablePageCount) {
            }
        } else {
            ucTcIdx = prTcqStatus->ucNextTcIdx;
            while (prTcqStatus->u2AvailiablePageCount) {
                if (prTcqStatus->u2AvailiablePageCount >= prTcqStatus->au2TxDonePageCount[ucTcIdx]) {
                    au2FreeTcResource[ucTcIdx] = prTcqStatus->au2TxDonePageCount[ucTcIdx];
                    prTcqStatus->u2AvailiablePageCount -= prTcqStatus->au2TxDonePageCount[ucTcIdx];
                    prTcqStatus->au2TxDonePageCount[ucTcIdx] = 0;
                    ucTcIdx++;
                    ucTcIdx %= TC_NUM;
                } else {
                    au2FreeTcResource[ucTcIdx] = prTcqStatus->u2AvailiablePageCount;
                    prTcqStatus->au2TxDonePageCount[ucTcIdx] -= prTcqStatus->u2AvailiablePageCount;
                    prTcqStatus->u2AvailiablePageCount = 0;
                }
            }
            prTcqStatus->ucNextTcIdx = ucTcIdx;

        }
		bStatus = TRUE;
    } else {
        MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("No FFA count to release\n"));
    }

    return bStatus;
}

BOOLEAN
MTSDIOTxReleaseResource(
        IN PRTMP_ADAPTER prAd,
    IN UINT16       *au2TxRlsCnt
)
{
    P_TX_TCQ_STATUS_T prTcqStatus = NULL;
    BOOLEAN bStatus = FALSE;
    UINT16 au2FreeTcResource[TC_NUM] = {0};
    //UINT32 u4BufferCountToBeFeed = 0;
    UINT8 i = 0;
    unsigned long flags = 0;

    prTcqStatus = &prAd->rTxCtrl.rTc;

    if (MTSDIOTxCalculateResource(prAd, au2TxRlsCnt, au2FreeTcResource)) {

        RTMP_SPIN_LOCK_IRQSAVE(&prAd->TcCountLock, &flags);

        for (i = TC0_INDEX; i < TC_NUM; i++) {
            /* Real page counter */
            prTcqStatus->au2FreePageCount[i] += au2FreeTcResource[i];
#if 0
            /* Buffer counter. For development only */
            /* Convert page count to buffer count */
            u4BufferCountToBeFeed = (prTcqStatus->au2FreePageCount[i] / SDIO_TX_MAX_PAGE_PER_FRAME);
            prTcqStatus->au2FreeBufferCount[i] = u4BufferCountToBeFeed;

            if (au2FreeTcResource[i]) {
                MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("Release: TC%u ReturnPageCnt[%u] FreePageCnt[%u] FreeBufferCnt[%u]\n",
                            i,
                            au2FreeTcResource[i],
                            prTcqStatus->au2FreePageCount[i],
                            prTcqStatus->au2FreeBufferCount[i]));
            }
#endif
            if (au2FreeTcResource[i]) {
                MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("Release: TC%u ReturnPageCnt[%u] FreePageCnt[%u]\n",
                            i,
                            au2FreeTcResource[i],
                            prTcqStatus->au2FreePageCount[i]));
            }
        }
        RTMP_SPIN_UNLOCK_IRQRESTORE(&prAd->TcCountLock, &flags);

        bStatus = TRUE;
    }

    return bStatus;
}

#if CFG_SDIO_INTR_ENHANCE

void  MTSDIOReadIntStatus(IN PRTMP_ADAPTER prAd,OUT PUINT32 pu4IntStatus)
{

	INT32 u4Ret = 0;
	P_ENHANCE_MODE_DATA_STRUCT_T pebancestatus;

	pebancestatus = prAd->penhanced_status;

	u4Ret = MTSDIOMultiRead(prAd, WHISR, (PUCHAR)pebancestatus
			, sizeof(ENHANCE_MODE_DATA_STRUCT_T));

	//      pebancestatus = (P_ENHANCE_MODE_DATA_STRUCT_T)prAd->enhanced_status;

	//*pu4IntStatus = pebancestatus->u4WHISR;

	if((*pu4IntStatus)){

		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_INFO, (" ==>%s hisr:%x num0:%x num1:%x r0len0:%x r0len1:%x r1len0:%x r1len1:%x txqcnt: %x %x %x %x %x %x %x %x\n"
					, __FUNCTION__,pebancestatus->u4WHISR
					,pebancestatus->rRxInfo.u.u2NumValidRx0Len
					,pebancestatus->rRxInfo.u.u2NumValidRx1Len
					,pebancestatus->rRxInfo.u.au2Rx0Len[0]
					,pebancestatus->rRxInfo.u.au2Rx0Len[1]
					,pebancestatus->rRxInfo.u.au2Rx1Len[0]
					,pebancestatus->rRxInfo.u.au2Rx1Len[1]
					,pebancestatus->rTxInfo.au4WTSR[0]
					,pebancestatus->rTxInfo.au4WTSR[1]
					,pebancestatus->rTxInfo.au4WTSR[2]
					,pebancestatus->rTxInfo.au4WTSR[3]
					,pebancestatus->rTxInfo.au4WTSR[4]
					,pebancestatus->rTxInfo.au4WTSR[5]
					,pebancestatus->rTxInfo.au4WTSR[6]
					,pebancestatus->rTxInfo.au4WTSR[7]
					));
	}

    /* Workaround:
        When RX done (without TX done happened), TX INT comes later
       . We already read the TC and cause TX INT disappear.
      We need to OR the TX_DONE_INT bit manually to workaound this corner case
    */
    if ((pebancestatus->u4WHISR & TX_DONE_INT) == 0 &&
                (pebancestatus->rTxInfo.au4WTSR[0] |
                pebancestatus->rTxInfo.au4WTSR[1] |
                pebancestatus->rTxInfo.au4WTSR[2] |
                pebancestatus->rTxInfo.au4WTSR[3] |
                pebancestatus->rTxInfo.au4WTSR[4] |
                pebancestatus->rTxInfo.au4WTSR[5] |
                pebancestatus->rTxInfo.au4WTSR[6] |
                pebancestatus->rTxInfo.au4WTSR[7])) {
        pebancestatus->u4WHISR |= TX_DONE_INT;
    }
	*pu4IntStatus = pebancestatus->u4WHISR;


    /* mtk03034 TODO: add for mailbox case, consider if mailbox INT. is read clear */

	return;

}
#endif

VOID
MTSDIOProcessIST_impl (
        IN PRTMP_ADAPTER prAd,
    IN UINT32 u4IntStatus
)
{
    UINT32 u4IntCount = 0;
    P_INT_EVENT_MAP_T prIntEventMap = NULL;

    ASSERT(prAd);

    prAd->IntStatus = u4IntStatus;

    prIntEventMap = &arIntEventMapTable[0];
    for (u4IntCount = 0; u4IntCount < ucIntEventMapSize; prIntEventMap++, u4IntCount++) {
	    if (prIntEventMap->u4Int & prAd->IntStatus) {

		    if (apfnEventFuncTable[prIntEventMap->u4Event] != NULL) {
			    MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("Process IST !"));
			    apfnEventFuncTable[prIntEventMap->u4Event](prAd);
		    } else {
			    MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("Empty INT handler !"));
			    ASSERT(0);
		    }

		    prAd->IntStatus &= ~prIntEventMap->u4Int;
	    }
    }
}


VOID
MTSDIOProcessIST(
        IN PRTMP_ADAPTER prAd
)
{
	UINT32 u4IntStatus = 0;

#if CFG_SDIO_INTR_ENHANCE
	MTSDIOReadIntStatus(prAd, &u4IntStatus);
#else
	MTSDIORead32(prAd, WHISR, &u4IntStatus);
#endif
//	MTSDIORead32(prAd, WHIER, &u4Val);

//	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_INFO, ("MTSDIOProcessIST prAd:%p prAd->IntStatus: %x WHIER: %x\n", prAd, u4IntStatus,u4Val));

	if (u4IntStatus & ~(WHIER_DEFAULT | FW_OWN_BACK_INT_EN)) {
		u4IntStatus &= WHIER_DEFAULT;
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("(%s): unknown interrupt: %x\n", __FUNCTION__, u4IntStatus));
	}
	MTSDIOProcessIST_impl(prAd, u4IntStatus);
}

VOID
MTSDIOProcessSoftwareInterrupt(
        IN PRTMP_ADAPTER prAd
)
{
    return;
}

VOID
MTSDIOProcessAbnormalInterrupt(
        IN PRTMP_ADAPTER prAd
)
{
    return;
}

VOID
MTSDIOProcessTxInterrupt(
        IN PRTMP_ADAPTER prAd
)
{
	struct MCU_CTRL *prCtl = NULL;
#if CFG_SDIO_INTR_ENHANCE
	P_ENHANCE_MODE_DATA_STRUCT_T prEnhance = prAd->penhanced_status;
#else
	INT32 au4TxCount[8];
	UINT8 ucTcIndex = 0;
	UINT32 u4Cnt = 0;
#endif
	struct cmd_msg *prMsg = NULL;

	ASSERT(prAd);
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_INFO, ("prAd %p\n", prAd));
	prCtl = &prAd->MCUCtrl;
#if CFG_SDIO_INTR_ENHANCE
	MTSDIOTxReleaseResource(prAd, (PUINT16) &(prEnhance->rTxInfo));

#ifdef CONFIG_ATE
	if (ATE_ON(prAd))
		prAd->ATECtrl.txed_pg = prEnhance->rTxInfo.u.u2TQ0Cnt;
#endif /* CONFIG_ATE */

	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("==== TC %x %x %x %x %x %x %x %x %x %x %x %x %x %x %x %x===\n",
				prEnhance->rTxInfo.u.u2TQ0Cnt,
				prEnhance->rTxInfo.u.u2TQ1Cnt,
				prEnhance->rTxInfo.u.u2TQ2Cnt,
				prEnhance->rTxInfo.u.u2TQ3Cnt,
				prEnhance->rTxInfo.u.u2TQ4Cnt,
				prEnhance->rTxInfo.u.u2TQ5Cnt,
				prEnhance->rTxInfo.u.u2TQ6Cnt,
				prEnhance->rTxInfo.u.u2TQ7Cnt,
				prEnhance->rTxInfo.u.u2TQ8Cnt,
				prEnhance->rTxInfo.u.u2TQ9Cnt,
				prEnhance->rTxInfo.u.u2TQ10Cnt,
				prEnhance->rTxInfo.u.u2TQ11Cnt,
				prEnhance->rTxInfo.u.u2TQ12Cnt,
				prEnhance->rTxInfo.u.u2TQ13Cnt,
				prEnhance->rTxInfo.u.u2TQ14Cnt,
				prEnhance->rTxInfo.u.u2TQ15Cnt));

	NdisZeroMemory(&(prEnhance->rTxInfo), sizeof(prEnhance->rTxInfo));
#else
	MTSDIORead32(prAd, WTQCR0, &au4TxCount[0]);
	MTSDIORead32(prAd, WTQCR1, &au4TxCount[1]);
	MTSDIORead32(prAd, WTQCR2, &au4TxCount[2]);
	MTSDIORead32(prAd, WTQCR3, &au4TxCount[3]);
	MTSDIORead32(prAd, WTQCR4, &au4TxCount[4]);
	MTSDIORead32(prAd, WTQCR5, &au4TxCount[5]);
	MTSDIORead32(prAd, WTQCR6, &au4TxCount[6]);
	MTSDIORead32(prAd, WTQCR7, &au4TxCount[7]);
	MTSDIORead32(prAd, WHCR, &u4Cnt);

#ifdef CONFIG_ATE
	if (ATE_ON(prAd))
		prAd->ATECtrl.txed_pg = au4TxCount[0];
#endif /* CONFIG_ATE */

	MTSDIOTxReleaseResource(prAd, (PUINT16) au4TxCount);

	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("==== TC %x %x %x %x %x %x %x %x WHCR: %x===\n",
				au4TxCount[0],
				au4TxCount[1],
				au4TxCount[2],
				au4TxCount[3],
				au4TxCount[4],
				au4TxCount[5],
				au4TxCount[6],
				au4TxCount[7],u4Cnt));
#endif
#ifdef MT_SDIO_ADAPTIVE_TC_RESOURCE_CTRL
	MTAdjustTcQuotas(prAd);
#endif
	// TODO: check ackq or kickq
	if (AndesQueueLen(prCtl, &prCtl->ackq) > 0) {
		prMsg = AndesDequeueCmdMsg(prCtl, &prCtl->ackq);
		if (prMsg) {
			AndesQueueHeadCmdMsg(&prCtl->ackq, prMsg, prMsg->state);
			SdioKickOutCmdMsgComplete(prMsg);
		}
	} else if (AndesQueueLen(prCtl, &prCtl->kickq) > 0) {
		prMsg = AndesDequeueCmdMsg(prCtl, &prCtl->kickq);
		if (prMsg) {
			AndesQueueHeadCmdMsg(&prCtl->kickq, prMsg, prMsg->state);
			SdioKickOutCmdMsgComplete(prMsg);
			//RTMP_OS_COMPLETION(&prMsg->tx_sdio_done);
		}
	}


	/* pending CMD */
    #if 0
	if (AndesQueueLen(prCtl, &prCtl->txq_sdio) > 0) {
		MTSDIOWorkerThreadEventNotify(prAd, SDIO_EVENT_CMD_BIT);
	}
    #endif


	/* pending Mgmt. frame */
	if ((MGMT_RING_SIZE - prAd->MgmtRing.TxSwFreeIdx) > 0) {
		MTSDIOWorkerThreadEventNotify(prAd, SDIO_EVENT_TXREQ_BIT);
	}

	/* TODO: mtk03034 pending Tx frame (need to review)*/
    {
        ULONG u4IrqFlags = 0;
        INT i4deq_id = 0, i4enq_id = 0;
        UINT32 u4i = 0;

        RTMP_IRQ_LOCK(&prAd->irq_lock, u4IrqFlags);

        for (u4i = 0; u4i < NUM_OF_TX_RING; u4i++) {
            i4deq_id = prAd->tx_swq[u4i].deqIdx;
            i4enq_id = prAd->tx_swq[u4i].enqIdx;
            if (prAd->tx_swq[u4i].swq[i4deq_id] != 0) {
		        MTSDIOWorkerThreadEventNotify(prAd, SDIO_EVENT_TX_DATA_REQ_BIT);
                break;
            }
        }

        RTMP_IRQ_UNLOCK(&prAd->irq_lock, u4IrqFlags);
    }

#ifdef CONFIG_ATE
	if (ATE_ON(prAd)){
		HEADER_802_11 *pHeader80211;
		UINT8 TXWISize = prAd->chipCap.TXWISize;
		P_TX_CTRL_T prTxCtrl = &prAd->rTxCtrl;
			ATE_CTRL *ATECtrl = &prAd->ATECtrl;
			BOOLEAN transmitted = FALSE;

			MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("%s, ATECtrl.Mode:%x,TxCount=%u, DoneCount=%u, Txed_pg:0x%x, tx_pg:0x%x\n",__FUNCTION__,
								ATECtrl->Mode,ATECtrl->TxCount, ATECtrl->TxDoneCount, ATECtrl->txed_pg, ATECtrl->tx_pg));

			if((ATECtrl->txed_pg >= ATECtrl->tx_pg)
				&& (ATECtrl->txed_pg != 0)){
				/* QID_AC_BK is mapped to TQ0 */
				ATECtrl->TxDoneCount++;
				ATECtrl->tx_pg = 0;
				transmitted = TRUE;pHeader80211 = (PHEADER_802_11)((UCHAR *)(ATECtrl->pAtePacket + TXWISize));
				pHeader80211->Sequence = ++ATECtrl->seq;
			}

			if((ATECtrl->Mode & ATE_TXFRAME)
					&& (ATECtrl->TxDoneCount < ATECtrl->TxCount)
					&& transmitted){
				UINT16 u2PgCnt;
				UCHAR *pucOutputBuf = NULL;
				INT32 ret;

				pucOutputBuf = prTxCtrl->pucTxCoalescingBufPtr;
				NdisMoveMemory(pucOutputBuf, ATECtrl->pAtePacket, (ATECtrl->TxLength + TXWISize));
				pucOutputBuf += (ATECtrl->TxLength + TXWISize);

				u2PgCnt = MTSDIOTxGetPageCount((ATECtrl->TxLength + TXWISize), TRUE);
				ATECtrl->tx_pg = u2PgCnt;
				MTSDIOTxAcquireResource(prAd, QID_AC_BK, u2PgCnt);
				ret = MTSDIOMultiWrite(prAd, WTDR1, prTxCtrl->pucTxCoalescingBufPtr, (ATECtrl->TxLength + TXWISize));
				MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE,("Tx another, PgCnt:0x%x, Bus ret:%d\n",u2PgCnt, ret));
			}else if((ATECtrl->TxStatus == 1)
					&& (ATECtrl->TxDoneCount == ATECtrl->TxCount)){
			MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE,("All Tx is done\n"));
			/* Tx status enters idle mode.*/
				ATECtrl->TxStatus = 0;
		}else{
				MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE,("Not ATE Packet Test TxQ Event, tc:0x%x\n", prEnhance->rTxInfo.u.u2TQ0Cnt));
		}
	}
#endif /* CONFIG_ATE */

	return;
}

VOID
MTSDIOProcessRxInterrupt(
        IN PRTMP_ADAPTER prAd
)
{
    ASSERT(prAd);

#if(CFG_SDIO_RX_AGG == 0) && (CFG_SDIO_INTR_ENHANCE == 0)
    MTSDIORxReadBuffer(prAd);
#elif(CFG_SDIO_RX_AGG == 0) && (CFG_SDIO_INTR_ENHANCE == 1)
	MTSDIORxEnhancedReadBuffer(prAd);
#elif(CFG_SDIO_RX_AGG == 1) && (CFG_SDIO_INTR_ENHANCE == 1)
	MTSDIORxEnhancedAggReadBuffer(prAd);
#endif

}


INT32
MTSDIOCmdTx(
        IN PRTMP_ADAPTER prAd,
        IN UCHAR *prBuf,
        IN UINT32 u4Length
)
{
	//printk("(%s) enter\n", __FUNCTION__);

	INT32 u4Ret = 0;

	u4Ret = MTSDIOMultiWrite(prAd, WTDR1, prBuf, u4Length);

	return u4Ret;
}


INT32
MTSDIODataTx(
        IN PRTMP_ADAPTER prAd,
        IN UCHAR *prBuf,
        IN UINT32 u4Length
)
{
	INT32 u4Ret = 0;

	u4Ret = MTSDIOMultiWrite(prAd, WTDR1, prBuf, u4Length);

	return u4Ret;
}


INT32
MTSDIODataRx0(
        IN PRTMP_ADAPTER prAd,
        OUT UCHAR *prBuf,
        IN UINT32 u4Length
)
{
	INT32 u4Ret = 0;

	u4Ret = MTSDIOMultiRead(prAd, WRDR0, prBuf, u4Length);

	return u4Ret;
}


INT32
MTSDIODataRx1(
        IN PRTMP_ADAPTER prAd,
        OUT UCHAR *prBuf,
        IN UINT32 u4Length
)
{
	INT32 u4Ret = 0;

	u4Ret = MTSDIOMultiRead(prAd, WRDR1, prBuf, u4Length);

	return u4Ret;
}


#if(CFG_SDIO_RX_AGG == 0) && (CFG_SDIO_INTR_ENHANCE == 0)

INT32 MTSDIORxReadBuffer(
        IN PRTMP_ADAPTER prAd
)
{
    UINT32 u4i = 0;
    PUCHAR prBuf = NULL;
    UINT32 u4Value = 0;
    UINT32 u4RX0PacketLen = 0, u4RX1PacketLen = 0;
    UINT32 u4Length = 0;
    UINT32 u4ReadLength = 0;
    UINT8  ucRxNum = 0;

    ASSERT(prAd);

    MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("%s: =============>\n", __FUNCTION__));

	   if ((RTMP_TEST_FLAG(prAd, (fRTMP_ADAPTER_RADIO_OFF |
		   fRTMP_ADAPTER_RESET_IN_PROGRESS |
		   fRTMP_ADAPTER_HALT_IN_PROGRESS |
		   fRTMP_ADAPTER_NIC_NOT_EXIST)) ||
		   (!RTMP_TEST_FLAG(prAd, fRTMP_ADAPTER_START_UP)))
	   && (!RTMP_TEST_FLAG(prAd, fRTMP_ADAPTER_POLL_IDLE)))
	   {
		   break;
	   }

    MTSDIORead32(prAd, WRPLR, &u4Value);
    u4RX0PacketLen = GET_RX0_PACKET_LENGTH(u4Value);
    u4RX1PacketLen = GET_RX1_PACKET_LENGTH(u4Value);

    if (u4RX0PacketLen > 0) {
        ucRxNum = 0;
        u4Length = u4RX0PacketLen;
    } else {
        ucRxNum = 1;
        u4Length = u4RX1PacketLen;
    }

    if (u4Length == 0) {
        MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("(%s): RX done, but No RX pkt\n", __FUNCTION__));
        return -1;
    }

#if 0
    if (u4Length >= prAd->BlockSize)
        u4Length = (u4Length + ((-u4Length) & (prAd->BlockSize - 1)));
#endif

    prAd->SDIORxPacket = RTMP_AllocateFragPacketBuffer(prAd, SDIO_TX_MAX_SIZE_PER_FRAME);
    MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("%s: =============>\n", __FUNCTION__));

    if (!prAd->SDIORxPacket)
	{
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("%s: allocate net packet fail\n",
														__FUNCTION__));

        return -1;
	}

#if 0
    /* Read Entire RFB and packet, include HW appended DW (Checksum Status) */
    u4ReadLength = (((u4Length) + 3) & ~3u) + 4;
#else
    u4ReadLength = (((u4Length) + 3) & ~3u);
#endif

    if (ucRxNum == 0) {
        MTSDIODataRx0(prAd, GET_OS_PKT_DATAPTR(prAd->SDIORxPacket), u4ReadLength);
    } else {
        MTSDIODataRx1(prAd, GET_OS_PKT_DATAPTR(prAd->SDIORxPacket), u4ReadLength);
    }

    //SET_OS_PKT_LEN(prAd->SDIORxPacket, u4ReadLength);
    SET_OS_PKT_LEN(prAd->SDIORxPacket, u4Length);
    prBuf = GET_OS_PKT_DATAPTR(prAd->SDIORxPacket);
    prAd->fgSDIORxPacKetInUsed = TRUE;

    MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("%s: ================len = %x ===\n", __FUNCTION__, u4Length));
#if 0
    for(u4i = 0; u4i < u4Length; u4i++){
        printk("%x,", *(prBuf + u4i));
	}
	printk("\n");
#endif
    if (RTMP_TEST_FLAG(prAd, fRTMP_ADAPTER_HALT_IN_PROGRESS |
									fRTMP_ADAPTER_NIC_NOT_EXIST))
	{
        RTMPFreeNdisPacket(prAd, prAd->SDIORxPacket);
        return -1;
	}

    rtmp_rx_done_handle(prAd);

    return 0;
}
#elif(CFG_SDIO_RX_AGG == 0) && (CFG_SDIO_INTR_ENHANCE == 1)

INT32 MTSDIORxEnhancedReadBuffer(IN PRTMP_ADAPTER prAd)
{
	UINT32 u4i = 0;
	UINT32 u4k = 0;
	PUCHAR prBuf = NULL;
	UINT32 u4Value = 0;
	UINT32 u4RX0PacketLen = 0, u4RX1PacketLen = 0;
	UINT32 u4Length = 0;
	UINT32 u4ReadLength = 0;
	UINT8  ucRxNum = 0;
	UINT32 rxNum;
	UINT16 u2RxPktNum;
	P_ENHANCE_MODE_DATA_STRUCT_T prSDIOCtrl;
	P_SW_RFB_T prSwRfb;
	P_RX_CTRL_T prRxCtrl = NULL;
	ULONG IrqFlags = 0;
	QUEUE_HEADER *pEntry;

	ASSERT(prAd);

	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_INFO, ("%s: =============>\n", __FUNCTION__));
	prSDIOCtrl = prAd->penhanced_status;
	prRxCtrl = &prAd->rRxCtrl;

	   if ((RTMP_TEST_FLAG(prAd, (fRTMP_ADAPTER_RADIO_OFF |
		   fRTMP_ADAPTER_RESET_IN_PROGRESS |
		   fRTMP_ADAPTER_HALT_IN_PROGRESS |
		   fRTMP_ADAPTER_NIC_NOT_EXIST)) ||
		   (!RTMP_TEST_FLAG(prAd, fRTMP_ADAPTER_START_UP)))
	   && (!RTMP_TEST_FLAG(prAd, fRTMP_ADAPTER_POLL_IDLE)))
	   {
		   break;
	   }

	for (rxNum = 0 ; rxNum < 2 ; rxNum++) {
		u2RxPktNum = (rxNum == 0 ? prSDIOCtrl->rRxInfo.u.u2NumValidRx0Len : prSDIOCtrl->rRxInfo.u.u2NumValidRx1Len);

		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_INFO, ("%s: ====rxNum: %x u2RxPktNum: %x\n", __FUNCTION__,rxNum,u2RxPktNum));

		if(u2RxPktNum == 0) {
			continue;
		}
		for (u4i = 0; u4i < u2RxPktNum; u4i++) {

#if 0
			MTSDIORead32(prAd, WRPLR, &u4Value);
			MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("%s: WRPLR u4Value: %x\n", __FUNCTION__,u4Value));
#endif

#if 0
			if (rxNum == 0) {
				u4RX0PacketLen = GET_RX0_PACKET_LENGTH(u4Value);
				u4Length = u4RX0PacketLen;
			}else{
				//u4RX1PacketLen = GET_RX1_PACKET_LENGTH(u4Value);
				u4RX1PacketLen= u4Value >>16;
				u4Length = u4RX1PacketLen;
			}
#else
			if (rxNum == 0) {
				u4Length = prSDIOCtrl->rRxInfo.u.au2Rx0Len[0];
			}else{
				u4Length = prSDIOCtrl->rRxInfo.u.au2Rx1Len[0];
			}
#endif

			//		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("%s: WRPLR u4RX0PacketLen: %x u4RX1PacketLen: %x u4Length: %x\n", __FUNCTION__,u4RX0PacketLen,u4RX1PacketLen,u4Length));

			if (u4Length == 0) {
				MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("(%s): RX done, but No RX pkt\n", __FUNCTION__));
				break;
			}
			u4ReadLength = (((u4Length) + 3) & ~3);

			//MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("%s: ====u4Length: %x u4ReadLength: %x num: %x\n",__FUNCTION__,u4Length, u4ReadLength ,prRxCtrl->rFreeSwRfbList.Number));


			RTMP_IRQ_LOCK(&prRxCtrl->rFreeSwRfbLock, IrqFlags);
			pEntry = RemoveHeadQueue(&prRxCtrl->rFreeSwRfbList);
			prSwRfb = (P_SW_RFB_T)pEntry;
			RTMP_IRQ_UNLOCK(&prRxCtrl->rFreeSwRfbLock, IrqFlags);

			if (!prSwRfb) {
				MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("%s: No More RFB\n", __FUNCTION__));
				break;
			}
			//		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("%s: ====prSwRfb != NULL\n",__FUNCTION__));

			if (rxNum == 0) {
				if (MTSDIODataRx0(prAd, GET_OS_PKT_DATAPTR(prSwRfb->prRxPacket), u4ReadLength)){
					MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("%s: MTSDIORxEnhancedReadBuffer rx0 failed\n", __FUNCTION__));
					RTMP_IRQ_LOCK(&prRxCtrl->rFreeSwRfbLock, IrqFlags)
						InsertTailQueue(&prRxCtrl->rFreeSwRfbList, &prSwRfb->rQueEntry);
					RTMP_IRQ_UNLOCK(&prRxCtrl->rFreeSwRfbLock, IrqFlags);
					break;
				}
			} else {
				if (MTSDIODataRx1(prAd, GET_OS_PKT_DATAPTR(prSwRfb->prRxPacket), u4ReadLength)){
					MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("%s: MTSDIORxEnhancedReadBuffer rx1 failed\n", __FUNCTION__));
					RTMP_IRQ_LOCK(&prRxCtrl->rFreeSwRfbLock, IrqFlags)
						InsertTailQueue(&prRxCtrl->rFreeSwRfbList, &prSwRfb->rQueEntry);
					RTMP_IRQ_UNLOCK(&prRxCtrl->rFreeSwRfbLock, IrqFlags);
					break;
				}
			}
			SET_OS_PKT_LEN(prSwRfb->prRxPacket, u4Length);

			//prSDIOCtrl->au4RxLength[i] = 0;
			RTMP_IRQ_LOCK(&prRxCtrl->rReceivedRfbLock, IrqFlags);
			InsertTailQueue(&prRxCtrl->rReceivedRfbList, &prSwRfb->rQueEntry);
			RTMP_IRQ_UNLOCK(&prRxCtrl->rReceivedRfbLock, IrqFlags);
			MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_INFO, ("%s: InsertTailQueue rReceivedRfbList\n",__FUNCTION__));

		}
	}

	rtmp_rx_done_handle(prAd);

	return 0;

}

#elif(CFG_SDIO_RX_AGG == 1) && (CFG_SDIO_INTR_ENHANCE == 1)

INT32 MTSDIORxEnhancedAggReadBuffer(IN PRTMP_ADAPTER prAd)
{
    UINT32 u4i = 0;
    UINT32 u4j = 0;
    UINT32 u4RxLength;

    UINT32 rxNum;
    UINT16 u2RxPktNum;
    P_ENHANCE_MODE_DATA_STRUCT_T prSDIOCtrl;
    P_SW_RFB_T prSwRfb;
    P_RX_CTRL_T prRxCtrl = NULL;
    ULONG IrqFlags = 0;
    QUEUE_ENTRY *pEntry;
    UINT32 u4RxLengthAlign;
    UINT32 u4RxAvailAggLen;
    UINT32 u4CurrAvailFreeRfbCnt;
    UINT32 u4RxAggCount;
    UINT32 u4RxAggLength;
    PUINT8 pucSrcAddr;
    UINT32 u4RxPktLen;
    UINT16 u2Rxbyte;
    BOOLEAN fReadFail = FALSE;
    BOOLEAN fgIsRxEnhanceMode = FALSE;
#if CFG_SDIO_RX_ENHANCE
    UINT32 u4MaxLoopCount = CFG_MAX_RX_ENHANCE_LOOP_COUNT;
#endif

    ASSERT(prAd);

    //MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("%s: =============>\n", __FUNCTION__));
    prSDIOCtrl = prAd->penhanced_status;
    prRxCtrl = &prAd->rRxCtrl;

#if CFG_SDIO_RX_ENHANCE
    fgIsRxEnhanceMode = TRUE;
#else
    fgIsRxEnhanceMode = FALSE;
#endif

    do {
#if CFG_SDIO_RX_ENHANCE
        /* to limit maximum loop for RX */
        u4MaxLoopCount--;
        if (u4MaxLoopCount == 0) {
            break;
        }
#endif

	   if ((RTMP_TEST_FLAG(prAd, (fRTMP_ADAPTER_RADIO_OFF |
		   fRTMP_ADAPTER_RESET_IN_PROGRESS |
		   fRTMP_ADAPTER_HALT_IN_PROGRESS |
		   fRTMP_ADAPTER_NIC_NOT_EXIST)) ||
		   (!RTMP_TEST_FLAG(prAd, fRTMP_ADAPTER_START_UP)))
	   && (!RTMP_TEST_FLAG(prAd, fRTMP_ADAPTER_POLL_IDLE)))
	   {
		   break;
	   }

        if(prSDIOCtrl->rRxInfo.u.u2NumValidRx0Len == 0 &&
                prSDIOCtrl->rRxInfo.u.u2NumValidRx1Len == 0) {
            break;
        }

        for (rxNum = 0 ; rxNum < 2 ; rxNum++) {
            u2RxPktNum = (rxNum == 0 ? prSDIOCtrl->rRxInfo.u.u2NumValidRx0Len : prSDIOCtrl->rRxInfo.u.u2NumValidRx1Len);

            //		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("%s: ====rxNum: %x u2RxPktNum: %x\n", __FUNCTION__,rxNum,u2RxPktNum));

            if (u2RxPktNum > 16)
                continue;
            if(u2RxPktNum == 0){
                continue;
            }

            //		u4RxAvailAggLen = CFG_RX_COALESCING_BUFFER_SIZE - sizeof(ENHANCE_MODE_DATA_STRUCT_T) + 4/* extra HW padding */;
#if CFG_SDIO_RX_ENHANCE
            u4RxAvailAggLen = CFG_RX_COALESCING_BUFFER_SIZE - sizeof(ENHANCE_MODE_DATA_STRUCT_T) - 4 ;
#else
            u4RxAvailAggLen = CFG_RX_COALESCING_BUFFER_SIZE;
#endif


            /*why to add 4 */

            u4CurrAvailFreeRfbCnt = prRxCtrl->rFreeSwRfbList.Number;
            u4RxAggCount = 0;
            u4RxAggLength = 0;
            if(u4CurrAvailFreeRfbCnt < u2RxPktNum) {
                MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("%s: u4CurrAvailFreeRfbCnt < u2RxPktNum\n", __FUNCTION__));
                continue;
            }

            for (u4i = 0; u4i < u2RxPktNum; u4i++) {

                u4RxLength = (rxNum == 0 ?
                        (UINT32)prSDIOCtrl->rRxInfo.u.au2Rx0Len[u4i] :
                        (UINT32)prSDIOCtrl->rRxInfo.u.au2Rx1Len[u4i]);

                if (!u4RxLength) {
                    ASSERT(0);
                    break;
                }
                u4RxLengthAlign = (((u4RxLength + HIF_RX_HW_APPENDED_LEN) + 3) & ~3);

                if (u4RxLengthAlign < u4RxAvailAggLen) {
                    if (u4RxAggCount < u4CurrAvailFreeRfbCnt) {
                        u4RxAvailAggLen -= u4RxLengthAlign;
                        u4RxAggLength += u4RxLengthAlign;
                        u4RxAggCount++;
                    }
                    else {
                        // no FreeSwRfb for rx packet
                        ASSERT(0);
                        break;
                    }
                }
                else {
                    // CFG_RX_COALESCING_BUFFER_SIZE is not large enough
                    ASSERT(0);
                    break;
                }

            }
            //		u4RxAggLength = (CFG_RX_COALESCING_BUFFER_SIZE - u4RxAvailAggLen);
            if(u4RxAggCount>1){
                MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("%s: u4RxAggLength: %x u4RxAggCount: %x\n", __FUNCTION__,u4RxAggLength,u4RxAggCount));
            }

#if CFG_SDIO_RX_ENHANCE
            u4RxAggLength =  u4RxAggLength + 4 + sizeof(ENHANCE_MODE_DATA_STRUCT_T);
#else
            u4RxAggLength = u4RxAggLength;
#endif

            if (rxNum == 0) {
                u4j = MTSDIODataRx0(prAd, prRxCtrl->pucRxCoalescingBufPtr, u4RxAggLength);
                if (u4j){
					MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("%s: u4RxAggLength: %x u4RxAggCount: %x\n", __FUNCTION__,u4RxAggLength,u4RxAggCount));
					MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("%s: MTSDIORxEnhancedReadBuffer rx0 failed: err = %x\n", __FUNCTION__, u4j));
					fReadFail = TRUE;
					break;
                }
            } else {
                u4j = MTSDIODataRx1(prAd, prRxCtrl->pucRxCoalescingBufPtr, u4RxAggLength);
                if (u4j){
					MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("%s: u4RxAggLength: %x u4RxAggCount: %x\n", __FUNCTION__,u4RxAggLength,u4RxAggCount));
					MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("%s: MTSDIORxEnhancedReadBuffer rx1 failed: err = %x\n", __FUNCTION__, u4j));
					fReadFail = TRUE;
					break;
                }
            }
            pucSrcAddr = prRxCtrl->pucRxCoalescingBufPtr;

            for (u4j = 0; u4j < u4RxAggCount; u4j++) {

                u4RxPktLen = (rxNum == 0 ?
                        (UINT32)prSDIOCtrl->rRxInfo.u.au2Rx0Len[u4j] :
                        (UINT32)prSDIOCtrl->rRxInfo.u.au2Rx1Len[u4j]);

                RTMP_IRQ_LOCK(&prRxCtrl->rFreeSwRfbLock, IrqFlags);
                pEntry = RemoveHeadQueue(&prRxCtrl->rFreeSwRfbList);
                prSwRfb = (P_SW_RFB_T)pEntry;
                RTMP_IRQ_UNLOCK(&prRxCtrl->rFreeSwRfbLock, IrqFlags);
                u2Rxbyte = *(PUINT16)pucSrcAddr;

                //			MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("%s: u4j: %x u4RxPktLen: %x u4RxAggLength: %x\n", __FUNCTION__,u4j,u4RxPktLen,u4RxAggLength));

                NdisCopyMemory(GET_OS_PKT_DATAPTR(prSwRfb->prRxPacket),pucSrcAddr,(((u4RxPktLen + HIF_RX_HW_APPENDED_LEN) + 3) & ~3));
                pucSrcAddr = pucSrcAddr + (((u4RxPktLen + HIF_RX_HW_APPENDED_LEN) + 3) & ~3);

                SET_OS_PKT_LEN(prSwRfb->prRxPacket, u4RxPktLen);

                //prSDIOCtrl->au4RxLength[i] = 0;
                RTMP_IRQ_LOCK(&prRxCtrl->rReceivedRfbLock, IrqFlags);
                InsertTailQueue(&prRxCtrl->rReceivedRfbList, &prSwRfb->rQueEntry);
                RTMP_IRQ_UNLOCK(&prRxCtrl->rReceivedRfbLock, IrqFlags);

                //MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("%s: InsertTailQueue rReceivedRfbList\n",__FUNCTION__));

                //			kalMemCopy(prSwRfb->pucRecvBuff, pucSrcAddr,ALIGN_4(u2PktLength + HIF_RX_HW_APPENDED_LEN));

            }
#if CFG_SDIO_RX_ENHANCE
            NdisCopyMemory(prSDIOCtrl, (pucSrcAddr + 4), sizeof(ENHANCE_MODE_DATA_STRUCT_T));

            /* do the same thing what nicSDIOReadIntStatus() does */
            if((prSDIOCtrl->u4WHISR & TX_DONE_INT) == 0 &&
			(prSDIOCtrl->rTxInfo.au4WTSR[0] | prSDIOCtrl->rTxInfo.au4WTSR[1]
			|prSDIOCtrl->rTxInfo.au4WTSR[2] | prSDIOCtrl->rTxInfo.au4WTSR[3]
			|prSDIOCtrl->rTxInfo.au4WTSR[4] | prSDIOCtrl->rTxInfo.au4WTSR[5]
			|prSDIOCtrl->rTxInfo.au4WTSR[6] | prSDIOCtrl->rTxInfo.au4WTSR[7])) {
                prSDIOCtrl->u4WHISR |= TX_DONE_INT;
            }

            /* FIXME: TODO: mtk3034: handle mailbox */
#if 0
            if((prSDIOCtrl->u4WHISR & BIT(31)) == 0 &&
                    HAL_GET_MAILBOX_READ_CLEAR(prAdapter) == TRUE &&
                    (prSDIOCtrl->u4RcvMailbox0 != 0 || prSDIOCtrl->u4RcvMailbox1 != 0)) {
                prSDIOCtrl->u4WHISR |= BIT(31);
            }
#endif

            /* dispatch to interrupt handler with RX bits masked */
            MTSDIOProcessIST_impl(prAd, prSDIOCtrl->u4WHISR& (~(RX0_DONE_INT | RX1_DONE_INT)));
#endif
        }

#if !CFG_SDIO_RX_ENHANCE
        prSDIOCtrl->rRxInfo.u.u2NumValidRx0Len = 0;
        prSDIOCtrl->rRxInfo.u.u2NumValidRx1Len = 0;
#endif
    }while ((prSDIOCtrl->rRxInfo.u.u2NumValidRx0Len
                || prSDIOCtrl->rRxInfo.u.u2NumValidRx1Len) && fgIsRxEnhanceMode &&!fReadFail);

    rtmp_rx_done_handle(prAd);

    return 0;
}

#endif

/*
 * ========================================================================
 * Routine Description:
 *     SDIO thread for handling TX request (DATA/CMD) and interrupt
 *
 * Arguments:
 *     ulContext            Pointer to RTMP_ADAPTER
 *
 * Return Value:
 *
 * Note:
 * ========================================================================
 */

VOID
MTSDIOWorkerThreadEventNotify(
        IN PRTMP_ADAPTER prAd,
        IN UINT32 u4EventBit
)
{
    //TODO: Replaced with RtmpOSxx
    set_bit(u4EventBit, &prAd->u4TaskEvent);
    RtmpOsTaskWakeUp(&prAd->rSDIOTask);
}

/*
 * ========================================================================
 * Routine Description:
 *     SDIO thread for handling TX request (DATA/CMD) and interrupt
 *
 * Arguments:
 *     ulContext            Pointer to RTMP_ADAPTER
 *
 * Return Value:
 *
 * Note:
 * ========================================================================
 */

INT
MTSDIOWorkerThread(
        IN ULONG ulContext
)
{
	RTMP_OS_TASK *prTask = (RTMP_OS_TASK *) ulContext;
	PRTMP_ADAPTER prAd = (PRTMP_ADAPTER) RTMP_OS_TASK_DATA_GET(prTask);
	UINT32 u4Status = 0;

	ASSERT(prAd);

	RtmpOSTaskCustomize(prTask);

	while (!RTMP_OS_TASK_IS_KILLED(prTask)) {
		if (!RtmpOSTaskWaitCond(prAd, prTask, (prAd->u4TaskEvent != 0), &u4Status)) {
			MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("%s(): leave SDIO worker thread\n", __FUNCTION__));
			break;
		}

		/* Handle interrupt */
		/* TODO: replaced with RtmpOSxxx */
		if (test_and_clear_bit(SDIO_EVENT_INT_BIT, &prAd->u4TaskEvent)) {
			MTSDIOProcessIST(prAd);
			/* Enable Interrupt */
			RTMP_SDIO_WRITE32(prAd, WHLPCR, W_INT_EN_SET);
		}

		/* Handle CMD request */
		if (test_and_clear_bit(SDIO_EVENT_CMD_BIT, &prAd->u4TaskEvent)) {
			AndesMTSendCmdMsgToSdio(prAd);
		}

		if (test_and_clear_bit(SDIO_EVENT_TX_DATA_REQ_BIT, &prAd->u4TaskEvent)) {
#if CFG_SDIO_TX_AGG
			MtSDIOAggTXInit(prAd);
#endif
			_RTMPDeQueuePacket(prAd, FALSE, NUM_OF_TX_RING, WCID_ALL, 64);

#if CFG_SDIO_TX_AGG
			MtSDIOAggTXKickout(prAd);
#endif

		}

		if (test_and_clear_bit(SDIO_EVENT_TXREQ_BIT, &prAd->u4TaskEvent)) {
			UINT32 u4i = 0;
			UINT32 u4j = 0;
			ULONG ulFlags = 0;

			u4j = prAd->MgmtRing.TxDmaIdx;
			for (u4i = 0; u4i < (MGMT_RING_SIZE - prAd->MgmtRing.TxSwFreeIdx); u4i++) {

				//hex_dump("mgmt packet", GET_OS_PKT_DATAPTR(prAd->MgmtRing.Cell[u4j].pNdisPacket), prAd->MgmtRing.Cell[u4j].u4TxLength);

				if (!MTSDIOTxAcquireResource(prAd, TC5_INDEX, MTSDIOTxGetPageCount(prAd->MgmtRing.Cell[u4j].u4TxLength, TRUE))) {
					MtSDIOMgmtKickOut(
							prAd,
							GET_OS_PKT_DATAPTR(prAd->MgmtRing.Cell[u4j].pNdisPacket),
							prAd->MgmtRing.Cell[u4j].u4TxLength
							);
					RELEASE_NDIS_PACKET(prAd, prAd->MgmtRing.Cell[u4j].pNdisPacket, NDIS_STATUS_SUCCESS);

					RTMP_IRQ_LOCK(&prAd->MgmtRingLock, ulFlags);
					INC_RING_INDEX(u4j, MGMT_RING_SIZE);
					prAd->MgmtRing.TxSwFreeIdx++;
					INC_RING_INDEX(prAd->MgmtRing.TxDmaIdx, MGMT_RING_SIZE);
					RTMP_IRQ_UNLOCK(&prAd->MgmtRingLock, ulFlags);

				}
			}
		}

		if (test_and_clear_bit(SDIO_EVENT_TX_BCN_BIT, &prAd->u4TaskEvent)) {
			//printk("SDIO_EVENT_TX_BCN_BIT\n");
			UINT32 u4i = 0;
			UINT32 u4j = 0;
			ULONG ulFlags = 0;

			u4j = prAd->BcnRing.TxDmaIdx;
			//for (u4i = 0; u4i < (BCN_RING_SIZE - prAd->BcnRing.TxSwFreeIdx); u4i++) {

				//hex_dump("bcn packet", GET_OS_PKT_DATAPTR(prAd->BcnRing.Cell[u4j].pNdisPacket), prAd->BcnRing.Cell[u4j].u4TxLength);

				if (!MTSDIOTxAcquireResource(prAd, TC7_INDEX, MTSDIOTxGetPageCount(prAd->BcnRing.Cell[u4j].u4TxLength, TRUE))) {
					MtSDIOMgmtKickOut(
							prAd,
							GET_OS_PKT_DATAPTR(prAd->BcnRing.Cell[u4j].pNdisPacket),
							prAd->BcnRing.Cell[u4j].u4TxLength
							);
					RTMP_IRQ_LOCK(&prAd->BcnRingLock, ulFlags);
					INC_RING_INDEX(u4j, BCN_RING_SIZE);
					prAd->BcnRing.TxSwFreeIdx++;
					INC_RING_INDEX(prAd->BcnRing.TxDmaIdx, BCN_RING_SIZE);
					RTMP_IRQ_UNLOCK(&prAd->BcnRingLock, ulFlags);
				}
			//}
		}
	}

	RtmpOSTaskNotifyToExit(prTask);

	return 0;
}

/*
========================================================================
Routine Description:
    SDIO command kernel thread.

Arguments:
	*Context			the pAd, driver control block pointer

Return Value:
    0					close the thread

Note:
========================================================================
*/
INT
MTSDIOCmdThread(
	IN ULONG Context)
{
	RTMP_ADAPTER *pAd;
	RTMP_OS_TASK *pTask;
	int status;
	status = 0;

	pTask = (RTMP_OS_TASK *)Context;
	pAd = (PRTMP_ADAPTER)RTMP_OS_TASK_DATA_GET(pTask);

#if 0
	/*
		If we don't check here, coverity will complain.
	*/
#endif
	if (pAd == NULL)
		return 0;

	RtmpOSTaskCustomize(pTask);

	NdisAcquireSpinLock(&pAd->CmdQLock);
	pAd->CmdQ.CmdQState = RTMP_TASK_STAT_RUNNING;
	NdisReleaseSpinLock(&pAd->CmdQLock);

	while (pAd->CmdQ.CmdQState == RTMP_TASK_STAT_RUNNING)
	{
		if (RtmpOSTaskWait(pAd, pTask, &status) == FALSE)
		{
			RTMP_SET_FLAG(pAd, fRTMP_ADAPTER_HALT_IN_PROGRESS);
			break;
		}

		if (pAd->CmdQ.CmdQState == RTMP_TASK_STAT_STOPED)
			break;

		if (!pAd->PM_FlgSuspend)
			CMDHandler(pAd);
	}

	if (!pAd->PM_FlgSuspend)
	{	/* Clear the CmdQElements. */
		CmdQElmt	*pCmdQElmt = NULL;

		NdisAcquireSpinLock(&pAd->CmdQLock);
		pAd->CmdQ.CmdQState = RTMP_TASK_STAT_STOPED;
		while(pAd->CmdQ.size)
		{
			RTThreadDequeueCmd(&pAd->CmdQ, &pCmdQElmt);
			if (pCmdQElmt)
			{
				if (pCmdQElmt->CmdFromNdis == TRUE)
				{
					if (pCmdQElmt->buffer != NULL)
						os_free_mem(pAd, pCmdQElmt->buffer);
					os_free_mem(pAd, (PUCHAR)pCmdQElmt);
				}
				else
				{
					if ((pCmdQElmt->buffer != NULL) && (pCmdQElmt->bufferlength != 0))
						os_free_mem(pAd, pCmdQElmt->buffer);
					os_free_mem(pAd, (PUCHAR)pCmdQElmt);
				}
			}
		}

		NdisReleaseSpinLock(&pAd->CmdQLock);
	}
	/* notify the exit routine that we're actually exiting now
	 *
	 * complete()/wait_for_completion() is similar to up()/down(),
	 * except that complete() is safe in the case where the structure
	 * is getting deleted in a parallel mode of execution (i.e. just
	 * after the down() -- that's necessary for the thread-shutdown
	 * case.
	 *
	 * complete_and_exit() goes even further than this -- it is safe in
	 * the case that the thread of the caller is going away (not just
	 * the structure) -- this is necessary for the module-remove case.
	 * This is important in preemption kernels, which transfer the flow
	 * of execution immediately upon a complete().
	 */
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,( "<---MTSDIOCmdThread\n"));

	RtmpOSTaskNotifyToExit(pTask);
	return 0;

}


/*
 * ========================================================================
 * Routine Description:
 *     SDIO thread for handling TX request (DATA/CMD) and interrupt
 *
 * Arguments:
 *     *prAd            Pointer to RTMP_ADAPTER
 *
 * Return Value:
 *
 * Note:
 * ========================================================================
 */

VOID
MTSDIODataIsr(
    IN PRTMP_ADAPTER prAd)
{
    ASSERT(prAd);

	/* Disable Interrupt */
	RTMP_SDIO_WRITE32(prAd, WHLPCR, W_INT_EN_CLR);

    /* Notify the sdio worker thread */
    MTSDIOWorkerThreadEventNotify(prAd, SDIO_EVENT_INT_BIT);
}
#ifdef MT_SDIO_ADAPTIVE_TC_RESOURCE_CTRL
VOID MTAdaptiveResourceCheckFastAdjustment(IN RTMP_ADAPTER *pAd, IN INT wcid, IN UCHAR queue)
{
	P_TX_CTRL_T prTxCtrl = NULL;
	STA_TR_ENTRY	*tr_entry = NULL;
	UCHAR	ucTc;

	tr_entry = &pAd->MacTab.tr_entry[wcid];

	if (tr_entry->EntryType == ENTRY_CAT_MCAST)
	{
		if (pAd->MacTab.fAnyStationInPsm)
			ucTc = TC4_INDEX;
		else
			ucTc = TC0_INDEX;
	}
	else
	{
		ucTc = queue;
	}

	prTxCtrl = &pAd->rTxCtrl;

	/* In MtSDIOTxResourceRequest() an additional page is made
		mandatory for transmission of a frame to PSE.
		We need to incorporate the same margin here.
		TODO: Do we actually need the extra page? */

	if (prTxCtrl->rTc.au2MaxNumOfPage[ucTc] < (SDIO_TX_MAX_PAGE_PER_FRAME + 1))
	{
		prTxCtrl->rTc.fQuotaAdjustFast = TRUE;
		prTxCtrl->rTc.u2TimeToUpdateQueLen = 1;
		prTxCtrl->rTc.u2TimeToAdjustTcResource = 1;
#if DEBUG_ADAPTIVE_QUOTA
		{
			UINT16 *p;

			DBGPRINT(RT_DEBUG_OFF, ("%s: ucTc %d :set fast = 1\n", __func__, ucTc));

			p = &prTxCtrl->rTc.au2MaxNumOfPage[0];
			DBGPRINT(RT_DEBUG_OFF, ("%s: Max Page %d %d %d %d %d\n", __func__, p[0], p[1], p[2], p[3], p[4]));
			p = &prTxCtrl->rTc.au2FreePageCount[0];
			DBGPRINT(RT_DEBUG_OFF, ("%s: FreePage %d %d %d %d %d\n", __func__, p[0], p[1], p[2], p[3], p[4]));
		}
#endif /* DEBUG_ADAPTIVE_QUOTA */
	}
	return;
}


VOID MTAdaptiveResourceAllocation(IN RTMP_ADAPTER *pAd, IN INT wcid, IN UCHAR queue)
{
	UINT16 demand[NUM_ADJUSTABLE_TC];
	STA_TR_ENTRY *tr_entry;
	UINT16 total_demand;
	int qid;
	int wcid_id;
	P_TX_CTRL_T prTxCtrl = NULL;
	//ULONG IrqFlags = 0;

	ASSERT(pAd);
	prTxCtrl = &pAd->rTxCtrl;

	/* check if time to update average demand */
	if(--(prTxCtrl->rTc.u2TimeToUpdateQueLen))
		return;

	prTxCtrl->rTc.u2TimeToUpdateQueLen = SDIO_INIT_TIME_TO_UPDATE_QUE_LEN;

	for(qid = 0; qid < NUM_ADJUSTABLE_TC; qid++)
	{
		demand[qid] = 0;
	}


	/* calculate current demand for all queueus */
	for ( wcid_id = 0; wcid_id < MAX_LEN_OF_TR_TABLE; wcid_id++)
	{
		tr_entry = &pAd->MacTab.tr_entry[wcid_id];
		if ( IS_VALID_ENTRY(tr_entry))
		{
			if( tr_entry->EntryType == ENTRY_CAT_MCAST )
			{
				if( pAd->MacTab.fAnyStationInPsm == 0)
				{
#if TC_PAGE_BASED_DEMAND
					demand[TC0_INDEX] += tr_entry->TotalPageCount[TC0_INDEX];
#else
					demand[TC0_INDEX] += tr_entry->tx_queue[TC0_INDEX].Number;
#endif
				}
				else
				{
#if TC_PAGE_BASED_DEMAND
					demand[TC4_INDEX] += tr_entry->TotalPageCount[TC0_INDEX];
#else
					demand[TC4_INDEX] += tr_entry->tx_queue[TC0_INDEX].Number;
#endif
				}
			}
			else
			{
				/* TODO Soumik: may need to check other conditions ps_state for PSM STA
				 * for AP mode operation */
				if((tr_entry->PsMode != PWR_SAVE) || (tr_entry->wcid == wcid_id))
				{
					for(qid = 0; qid <= TC3_INDEX; qid++)
					{
#if TC_PAGE_BASED_DEMAND
						demand[qid] += tr_entry->TotalPageCount[qid];
#else
						demand[qid] += tr_entry->tx_queue[qid].Number;
#endif
					}
				}
			}
		}
	}

#if !TC_PAGE_BASED_DEMAND
	/* converting demand into the page demand */
	for(qid = 0; qid < NUM_ADJUSTABLE_TC; qid++)
	{
		demand[qid] *= SDIO_TX_MAX_PAGE_PER_FRAME;
	}
#endif /* !TC_PAGE_BASED_DEMAND */

#if DEBUG_ADAPTIVE_QUOTA
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("%s: demand %d %d %d %d %d\n", __func__, demand[0], demand[1],
		demand[2], demand[3], demand[4]));
#endif /* DEBUG_ADAPTIVE_QUOTA */
#if DEBUG_ADAPTIVE_QUOTA
	{
		UINT16 *p = &prTxCtrl->rTc.au2AvgDemand[0];
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("%s: AvgDemand %d %d %d %d %d\n", __func__, p[0], p[1],
			p[2], p[3], p[4]));
	}
#endif
	//evaluating the moving avg
	for( qid = 0; qid < NUM_ADJUSTABLE_TC; qid++)
	{
		/* TODO: soumik - replace the divison with bit shift operation */
		if (demand[qid] && (prTxCtrl->rTc.au2AvgDemand[qid] == 0))
			prTxCtrl->rTc.au2AvgDemand[qid] = demand[qid];
		else
			prTxCtrl->rTc.au2AvgDemand[qid] = (((prTxCtrl->rTc.au2AvgDemand[qid] * (MOVING_AVARAGE_WINDOW_SIZE - 1)) + demand[qid]) / MOVING_AVARAGE_WINDOW_SIZE );

		demand[qid] = prTxCtrl->rTc.au2AvgDemand[qid];
	}
#if DEBUG_ADAPTIVE_QUOTA
	{
		UINT16 *p = &prTxCtrl->rTc.au2AvgDemand[0];
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("%s: AvgDemand %d %d %d %d %d\n",
                __func__, p[0], p[1], p[2], p[3], p[4]));
	}
#endif /* DEBUG_ADAPTIVE_QUOTA */

	/* Check if it is time for resource re-assignment */
	if(--prTxCtrl->rTc.u2TimeToAdjustTcResource)
		return;

	/* Check if last quota re-adjustement is complete */
	if(prTxCtrl->rTc.fgTcResourcePostAnnealing)
	{
		prTxCtrl->rTc.u2TimeToAdjustTcResource = 1;
		return;
	}
	prTxCtrl->rTc.u2TimeToAdjustTcResource = SDIO_INIT_TIME_TO_ADJUST_TC_RSC;

	// Calculate the total demand
	total_demand = 0;
	for( qid = 0; qid < NUM_ADJUSTABLE_TC; qid++)
	{
		total_demand += demand[qid];
	}

#if DEBUG_ADAPTIVE_QUOTA
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("%s: total_demand %d total adjustable %d\n",
                __func__, total_demand, NIC_TOTAL_ADJUSTABLE_RESOURCES));
#endif
	// Allocating resources for each queue
	if ((total_demand > 0) && (total_demand <= NIC_TOTAL_ADJUSTABLE_RESOURCES))
	{
		int total_resources = NIC_TOTAL_ADJUSTABLE_RESOURCES;
		int shareCount=0;
		int shareResourcesPerQueue = 0;

		for(qid = 0; qid < NUM_ADJUSTABLE_TC; qid++)
		{
			prTxCtrl->rTc.au2CurrentTcResource[qid] = demand[qid];

			if(demand[qid] > 0)
			{
				total_resources -= demand[qid];
				shareCount++;
			}
		}

		if (shareCount)
			shareResourcesPerQueue = (total_resources / shareCount);

		if(shareResourcesPerQueue)
		{
			for(qid = 0; qid < NUM_ADJUSTABLE_TC; qid++)
			{
				if(demand[qid])
				{
					prTxCtrl->rTc.au2CurrentTcResource[qid] += shareResourcesPerQueue;
					total_resources -= shareResourcesPerQueue;
				}
			}
		}

		//allocating the remaining resources to VO
		prTxCtrl->rTc.au2CurrentTcResource[TC3_INDEX] += total_resources;
	}
	else if( total_demand > NIC_TOTAL_ADJUSTABLE_RESOURCES )
	{
		int total_resources = NIC_TOTAL_ADJUSTABLE_RESOURCES;
		int totalDeficit=0;
		int totalUnallocatedResources;
		int shareResourcesPerQueue;

		for (qid = 0; qid < NUM_ADJUSTABLE_TC; qid++)
		{
		    if (demand[qid] > 0)
            {
                prTxCtrl->rTc.au2CurrentTcResource[qid] = prTxCtrl->rTc.au2GuaranteedTcResource[qid];
                total_resources -= prTxCtrl->rTc.au2GuaranteedTcResource[qid];
                if(demand[qid] <= prTxCtrl->rTc.au2GuaranteedTcResource[qid])
                {
                    demand[qid] = 0;
                }
                else
                {
                    demand[qid] -= prTxCtrl->rTc.au2GuaranteedTcResource[qid];
                }
            }
            else
            {
                prTxCtrl->rTc.au2CurrentTcResource[qid] = 0;
            }
            totalDeficit += demand[qid];
		}

		totalUnallocatedResources = total_resources;

		for(qid = NUM_ADJUSTABLE_TC - 1 ; qid >=0 ; qid--)
		{
			if(demand[qid] == 0)
				continue;

			shareResourcesPerQueue = ((demand[qid] * totalUnallocatedResources)/totalDeficit);

			if (shareResourcesPerQueue == 0)
				continue;

			if (shareResourcesPerQueue > demand[qid])
				shareResourcesPerQueue = demand[qid];

			prTxCtrl->rTc.au2CurrentTcResource[qid] += shareResourcesPerQueue;
			total_resources -= shareResourcesPerQueue;
			demand[qid] -= shareResourcesPerQueue;
		}

		if(total_resources > 0)
		{
			/* Excluding BMC queue from residual allocation */
			for(qid = TC3_INDEX; qid >= 0; qid--)
			{
				if(demand[qid]==0)
					continue;
				if(demand[qid] >=	 total_resources)
				{
					prTxCtrl->rTc.au2CurrentTcResource[qid] += total_resources;
					demand[qid] -= total_resources;
					total_resources = 0;
					break;
				}
				else
				{
					prTxCtrl->rTc.au2CurrentTcResource[qid] += demand[qid];
					demand[qid] = 0;
					total_resources =- demand[qid];
				}
			}
		}
	}
	prTxCtrl->rTc.fgTcResourcePostAnnealing = TRUE;

#if DEBUG_ADAPTIVE_QUOTA
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("%s: CurrentTcResources  %d %d %d %d %d %d %d %d\n", __func__,
							pAd->rTxCtrl.rTc.au2CurrentTcResource[0],
							pAd->rTxCtrl.rTc.au2CurrentTcResource[1],
							pAd->rTxCtrl.rTc.au2CurrentTcResource[2],
							pAd->rTxCtrl.rTc.au2CurrentTcResource[3],
							pAd->rTxCtrl.rTc.au2CurrentTcResource[4],
							pAd->rTxCtrl.rTc.au2CurrentTcResource[5],
							pAd->rTxCtrl.rTc.au2CurrentTcResource[6],
							pAd->rTxCtrl.rTc.au2CurrentTcResource[7]));
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("%s: FreeResources %d %d %d %d %d %d %d %d\n", __func__,
							pAd->rTxCtrl.rTc.au2FreePageCount[0],
							pAd->rTxCtrl.rTc.au2FreePageCount[1],
							pAd->rTxCtrl.rTc.au2FreePageCount[2],
							pAd->rTxCtrl.rTc.au2FreePageCount[3],
							pAd->rTxCtrl.rTc.au2FreePageCount[4],
							pAd->rTxCtrl.rTc.au2FreePageCount[5],
							pAd->rTxCtrl.rTc.au2FreePageCount[6],
							pAd->rTxCtrl.rTc.au2FreePageCount[7]));
#endif

	if (prTxCtrl->rTc.fQuotaAdjustFast)
	{
#if DEBUG_ADAPTIVE_QUOTA
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("%s: fQuotaAdjustFast == TRUE call MTAdjustTcQuotas\n", __func__));
#endif
		prTxCtrl->rTc.fQuotaAdjustFast = FALSE;
		MTAdjustTcQuotas(pAd);
	}

	return;
}

VOID MTAdjustTcQuotas(IN RTMP_ADAPTER *pAd)
{

	INT16 i2TotalExtraQuota = 0;
	INT16 ai2ExtraQuota[NUM_ADJUSTABLE_TC];
	BOOLEAN fgResourceRedistributed = TRUE;
	P_TX_CTRL_T prTxCtrl = NULL;
	int qid;
	unsigned long flags = 0;

	ASSERT(pAd);
	prTxCtrl = &pAd->rTxCtrl;

	if (!prTxCtrl->rTc.fgTcResourcePostAnnealing)
	{
			return;
	}

#if DEBUG_ADAPTIVE_QUOTA
	{
		UINT16 *p;

		p = &prTxCtrl->rTc.au2MaxNumOfPage[0];

		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("%s: MaxNumPage = %d %d %d %d %d %d %d %d\n", __func__,
			p[0], p[1], p[2], p[3], p[4], p[5], p[6], p[7]));

		p = &prTxCtrl->rTc.au2CurrentTcResource[0];
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("%s: CurTcResource = %d %d %d %d %d %d %d %d\n", __func__,
			p[0], p[1], p[2], p[3], p[4], p[5], p[6], p[7]));

		p = &prTxCtrl->rTc.au2FreePageCount[0];
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("%s: FreePageCount = %d %d %d %d %d %d %d %d\n", __func__,
			p[0], p[1], p[2], p[3], p[4], p[5], p[6], p[7]));
	}
#endif
	//Obtain the free-to-distribute resources
	for (qid = 0; qid < NUM_ADJUSTABLE_TC; qid++)
	{
		ai2ExtraQuota[qid] = (INT16)((UINT16)prTxCtrl->rTc.au2MaxNumOfPage[qid] - (UINT16)prTxCtrl->rTc.au2CurrentTcResource[qid]);

		if (ai2ExtraQuota[qid] > 0)
		{ /* The resource shall be reallocated to other TCs */
			if (ai2ExtraQuota[qid] > prTxCtrl->rTc.au2FreePageCount[qid])
			{
				ai2ExtraQuota[qid] = prTxCtrl->rTc.au2FreePageCount[qid];
				fgResourceRedistributed = FALSE;
#if DEBUG_ADAPTIVE_QUOTA
				MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("%s: qid %d fgResourceRedistributed = %d ExtraQuota %d FreePage %d\n",
					__func__, qid, fgResourceRedistributed, ai2ExtraQuota[qid], prTxCtrl->rTc.au2FreePageCount[qid]));
#endif /* DEBUG_ADAPTIVE_QUOTA */
			}
			i2TotalExtraQuota += ai2ExtraQuota[qid];
		}
	}

#if DEBUG_ADAPTIVE_QUOTA
	{
		INT16 *p = &ai2ExtraQuota[0];

		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("%s: ExtraQuota = %d %d %d %d %d TotalExtraQuota = %d \n", __func__,
			p[0], p[1], p[2], p[3], p[4], i2TotalExtraQuota));
	}
#endif /* DEBUG_ADAPTIVE_QUOTA */
	RTMP_SPIN_LOCK_IRQSAVE(&pAd->TcCountLock, &flags);

	for(qid = TC3_INDEX; qid >= 0; qid--)
	{
		if( ai2ExtraQuota[qid] < 0 )
		{
			if((-ai2ExtraQuota[qid]) <= i2TotalExtraQuota)
			{
				prTxCtrl->rTc.au2MaxNumOfPage[qid] += -ai2ExtraQuota[qid];
				prTxCtrl->rTc.au2FreePageCount[qid] += -ai2ExtraQuota[qid];
				i2TotalExtraQuota += ai2ExtraQuota[qid];
			}
			else
			{
				prTxCtrl->rTc.au2MaxNumOfPage[qid] += i2TotalExtraQuota;
				prTxCtrl->rTc.au2FreePageCount[qid] += i2TotalExtraQuota;
				i2TotalExtraQuota = 0;
				fgResourceRedistributed = FALSE;
			}
		}
		else
		{
			prTxCtrl->rTc.au2MaxNumOfPage[qid] -= ai2ExtraQuota[qid];
			prTxCtrl->rTc.au2FreePageCount[qid] -= ai2ExtraQuota[qid];
		}
	}

	if((i2TotalExtraQuota > 0) && ai2ExtraQuota[TC4_INDEX] < 0)
	{
		if((-ai2ExtraQuota[TC4_INDEX]) >= i2TotalExtraQuota)
		{
			fgResourceRedistributed = FALSE;
			prTxCtrl->rTc.au2MaxNumOfPage[TC4_INDEX] += i2TotalExtraQuota;
			prTxCtrl->rTc.au2FreePageCount[TC4_INDEX] += i2TotalExtraQuota;
			i2TotalExtraQuota = 0;
		}
		else
		{
			prTxCtrl->rTc.au2MaxNumOfPage[TC4_INDEX] += -ai2ExtraQuota[TC4_INDEX];
			prTxCtrl->rTc.au2FreePageCount[TC4_INDEX] += -ai2ExtraQuota[TC4_INDEX];
			i2TotalExtraQuota += ai2ExtraQuota[TC4_INDEX];
		}
	}
	else if(ai2ExtraQuota[TC4_INDEX] > 0)
	{
		prTxCtrl->rTc.au2MaxNumOfPage[TC4_INDEX] -= ai2ExtraQuota[TC4_INDEX];
		prTxCtrl->rTc.au2FreePageCount[TC4_INDEX] -= ai2ExtraQuota[TC4_INDEX];
        i2TotalExtraQuota += ai2ExtraQuota[TC4_INDEX];
	}

	if (i2TotalExtraQuota > 0)
	{
		prTxCtrl->rTc.au2MaxNumOfPage[TC3_INDEX] += i2TotalExtraQuota;
		prTxCtrl->rTc.au2FreePageCount[TC3_INDEX] += i2TotalExtraQuota;
	}

#if DEBUG_ADAPTIVE_QUOTA
	{
		UINT16 *p;

		p = &prTxCtrl->rTc.au2MaxNumOfPage[0];
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("%s: MaxNumPage = %d %d %d %d %d\n", __func__,
				p[0], p[1], p[2], p[3], p[4]));

		p = &prTxCtrl->rTc.au2CurrentTcResource[0];
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("%s: CurTcResource = %d %d %d %d %d\n", __func__,
				p[0], p[1], p[2], p[3], p[4]));

		p = &prTxCtrl->rTc.au2FreePageCount[0];
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("%s: FreePage = %d %d %d %d %d\n", __func__,
						p[0], p[1], p[2], p[3], p[4]));
	}
#endif /* DEBUG_ADAPTIVE_QUOTA */

	RTMP_SPIN_UNLOCK_IRQRESTORE(&pAd->TcCountLock, &flags);

	prTxCtrl->rTc.fgTcResourcePostAnnealing = (!fgResourceRedistributed);
#if DEBUG_ADAPTIVE_QUOTA
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("%s: fgTcResourcePostAnnealing = %d\n", __func__, prTxCtrl->rTc.fgTcResourcePostAnnealing));
#endif /* DEBUG_ADAPTIVE_QUOTA */
	return;
}
#endif /* MT_SDIO_ADAPTIVE_TC_RESOURCE_CTRL */
