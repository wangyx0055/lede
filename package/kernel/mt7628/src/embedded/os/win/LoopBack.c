#ifdef RTMP_SDIO_SUPPORT
#include "sdio_config.h"
#endif

#ifdef RTMP_USB_SUPPORT
#include "config.h"
#endif

void LoopBack_Start(RTMP_ADAPTER *pAd, LOOPBACK_SETTING *pSetting)
{
	NTSTATUS ntStatus = STATUS_SUCCESS;
	UINT16 BreakCount = 0;
	UINT8 Qid = 0;

	U3DMA_WLCFG             UDMACfg;
	UINT32				UDMACfg1;
	UINT32				Length = 0;
	UINT32				RepeatIdx = 0;
	UINT32				RxQ = 1;

	RtlCopyMemory(&pAd->LoopBackSetting, pSetting,sizeof(LOOPBACK_SETTING));
	
	RtlZeroMemory(&pAd->LoopBackTxRaw, LOOPBACK_SIZE);
	RtlZeroMemory(&pAd->LoopBackRxRaw, LOOPBACK_SIZE);
	RtlZeroMemory(&pAd->LoopBackResult, sizeof(LOOPBACK_RESULT));
	if(pSetting->StartLen < sizeof(TX_WI_STRUC))
	{
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DEBUG_WARN,("%s startLength(0x%x) is shorter than sizeof(TX_WI_STRUC) (0x%x)\n",__FUNCTION__, pSetting->StartLen, sizeof(TX_WI_STRUC)));	
		return;
	}
	
	pAd->LoopBackWaitRx = FALSE;
	pAd->LoopBackRunning = TRUE;
	pAd->LoopBackResult.Status = RUNNING;	
	
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DEBUG_TRACE,("%s\n",__FUNCTION__));
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DEBUG_TRACE,("IsDefaultPattern 	%d\n", pSetting->IsDefaultPattern));
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DEBUG_TRACE,("RepeatTimes	 	%d\n", pSetting->RepeatTimes));
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DEBUG_TRACE,("StartLen	 		%d\n", pSetting->StartLen));
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DEBUG_TRACE,("StopLen		 	%d\n", pSetting->StopLen));
	
	pAd->LoopBackDefaultPattern = (BOOLEAN)pSetting->IsDefaultPattern;
	ntStatus = FWSwitchToROM(pAd);
	if(ntStatus != STATUS_SUCCESS)
	{
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DEBUG_ERROR,("%s, Switch to ROM failed (0x%x) \n",__FUNCTION__, ntStatus));
	}
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DEBUG_TRACE,("%s FWSwitchToROM\n",__FUNCTION__));
	//pAd->LoopBackWaitRx = FALSE;
	//pAd->LoopBackRunning = TRUE;
	//pAd->LoopBackResult.Status = RUNNING;	
#ifdef RTMP_USB_SUPPORT
	//Step II config dma scheduler
	Qid = (UINT8)pSetting->BulkOutNumber;
	ntStatus = ConfigDMAScheduler(pAd, Qid);
	if(ntStatus != STATUS_SUCCESS)
	{
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DEBUG_ERROR,("%s, PowerOnWiFiSys \n",__FUNCTION__));
	}
	pAd->LoopBackBulkoutNumber = GetBulkoutNumber(pAd);
	//Step III program UDMA 
	UDMACfg1 = pSetting->RxAggPktLmt;
	RTMP_IO_WRITE32(pAd, UDMA_WLCFG_1, UDMACfg1);
	
	RTMP_IO_READ32(pAd, RA_U3DMA_WLCFG, &UDMACfg.word);
	UDMACfg.Default.WL_TX_EN = 1;
	UDMACfg.Default.WL_RX_EN = 1;
	UDMACfg.Default.WL_RX_AGG_TO = pSetting->RxAggTO;
	UDMACfg.Default.WL_RX_AGG_LMT = pSetting->RxAggLmt;
	UDMACfg.Default.WL_RX_AGG_EN = pSetting->RxAggEnable;
	RTMP_IO_WRITE32(pAd, RA_U3DMA_WLCFG, UDMACfg.word);
	RXStartStop(pAd, START, BULKIN_PIPENUMBER_0);
	RXStartStop(pAd, START, BULKIN_PIPENUMBER_1);
	RxQ = pSetting->BulkInNumber;
	
#endif
	FirmwareCommnadHIFLoopBackTest(pAd, 1, (UINT8)RxQ);
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DEBUG_TRACE,("***************************************%s @#@# Rx[%d]***************************************\n",__FUNCTION__, RxQ));
	pAd->LoopBackTxThread = NULL;
	
	ntStatus =  PsCreateSystemThread(&pAd->LoopBackTxThread,
									(ACCESS_MASK) 0L,
									NULL,
									NULL,
									NULL,
									LoopBack_TxThread,
									pAd);
	if (ntStatus != STATUS_SUCCESS)
	{
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DEBUG_ERROR,("%s, thread create fail\n",__FUNCTION__));	
	}
	
}
void LoopBack_Stop(RTMP_ADAPTER *pAd)
{		
	NTSTATUS ntStatus = STATUS_SUCCESS;
	UINT32 BreakCount = 0;
	UINT32 Idx = 0;
	while(pAd->LoopBackWaitRx == TRUE)
	{
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DEBUG_TRACE,("%s, @#@#BreakCount = %d\n",__FUNCTION__, BreakCount));
		if(BreakCount > 100)
			break;
		RTMPusecDelay(300);
		BreakCount++;		
	}
	
	//pAd->LoopBackRunning = FALSE;
	if(pAd->LoopBackResult.Status == RUNNING)
	{
		pAd->LoopBackResult.Status = PASS;
		pAd->LoopBackResult.FailReason= NO_ERROR;
	}
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DEBUG_TRACE,("***************************************%s @#@#***************************************\n",__FUNCTION__));
	pAd->LoopBackRunning = FALSE;
#ifdef RTMP_USB_SUPPORT
	//restore CR
	if(pAd->LoopBackSetting.BulkOutNumber == EPALLQID)
	{
		if(pAd->LoopBackResult.Status == PASS)
		{
			for (Idx = 0; Idx<35; Idx++)
			{			
				RTMP_IO_WRITE32(pAd, 0x50000540 + Idx*4, pAd->BackupCR[Idx]);
				MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,("%s @@BACKUP 0x50000540 + %d*4 = 0x%x\n", __FUNCTION__, Idx, pAd->BackupCR[Idx]));
			}
			ntStatus = ConfigDMAScheduler(pAd, EP8QID);
		}
	}
	else
	{	
		ntStatus = ConfigDMAScheduler(pAd, EP8QID);
	}
#endif
	FirmwareCommnadHIFLoopBackTest(pAd, 0, 0);

	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DEBUG_TRACE,("%s, \n",__FUNCTION__));
}
void LoopBack_Status(RTMP_ADAPTER *pAd, LOOPBACK_RESULT *pResult)
{
	RtlCopyMemory(pResult, &pAd->LoopBackResult,sizeof(LOOPBACK_RESULT));
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DEBUG_TRACE,("%s\n",__FUNCTION__));
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DEBUG_TRACE,("%s, Status: %d\n",__FUNCTION__, pAd->LoopBackResult.Status));
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DEBUG_TRACE,("%s, FailReason: %d\n",__FUNCTION__, pAd->LoopBackResult.FailReason));
	//MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DEBUG_TRACE,("%s, LastBulkOut: %d\n",__FUNCTION__, pAd->LoopBackResult.LastBulkOut));
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DEBUG_TRACE,("%s, RxByteCount: %d\n",__FUNCTION__, pAd->LoopBackResult.RxByteCount));
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DEBUG_TRACE,("%s, RxPktCount: %d\n",__FUNCTION__, pAd->LoopBackResult.RxPktCount));	
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DEBUG_TRACE,("%s, TxByteCount: %d\n",__FUNCTION__, pAd->LoopBackResult.TxByteCount));
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DEBUG_TRACE,("%s, TxPktCount: %d\n",__FUNCTION__, pAd->LoopBackResult.TxPktCount));
}
void LoopBack_RawData(RTMP_ADAPTER *pAd, UINT32  *pLength, BOOLEAN IsTx, UCHAR *RawData )
{
	if(*pLength >LOOPBACK_SIZE)
	{
		*pLength = LOOPBACK_SIZE;
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DEBUG_TRACE,("%s, max length is %d\n",__FUNCTION__, LOOPBACK_SIZE));
	}	
	if(IsTx)
	{
		*pLength = pAd->LoopBackTxRawLen;
		RtlCopyMemory(RawData, &pAd->LoopBackTxRaw,*pLength);
	}
	else
	{
		*pLength = pAd->LoopBackRxRawLen;
		RtlCopyMemory(RawData, &pAd->LoopBackRxRaw,*pLength);
	}
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DEBUG_TRACE,("%s, Length = 0x%x\n",__FUNCTION__, *pLength));
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DEBUG_TRACE,("%s, LoopBackRxRawLen = 0x%x\n",__FUNCTION__, pAd->LoopBackRxRawLen));
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DEBUG_TRACE,("%s, LoopBackTxRawLen = 0x%x\n",__FUNCTION__, pAd->LoopBackTxRawLen));
}
void LoopBack_ExpectRx(RTMP_ADAPTER *pAd, UINT32 Length, UINT8 *pRawData)
{
	PULONG ptr;
	UINT8 	i = 0;
	if(Length >LOOPBACK_SIZE)
	{
		Length = LOOPBACK_SIZE;
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DEBUG_TRACE,("%s, max length is %d\n",__FUNCTION__, LOOPBACK_SIZE));
	}
	RtlCopyMemory(&pAd->LoopBackExpectRx, pRawData, Length);
	
	ptr = (PULONG)(&pAd->LoopBackExpectRx);
	//Length = ptr[0] & 0xffff;
	pAd->LoopBackExpectRxLen = Length;
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DEBUG_TRACE,("%s, Length = %d\n",__FUNCTION__, Length));
	for(i=0;i<20;i++)
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE,("TXD(TXWI) %d 0x%08X\n",i, *(ptr+i)));
	
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DEBUG_TRACE,("%s, \n",__FUNCTION__));
}

void LoopBack_ExpectTx(RTMP_ADAPTER *pAd, UINT32 Length, UINT8 *pRawData)
{
	PULONG ptr;
	UINT8 	i = 0;
	if(Length >LOOPBACK_SIZE)
	{
		Length = LOOPBACK_SIZE;
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DEBUG_TRACE,("%s, max length is %d\n",__FUNCTION__, LOOPBACK_SIZE));
	}
	RtlCopyMemory(&pAd->LoopBackExpectTx, pRawData, Length);	
	
	ptr = (PULONG)(&pAd->LoopBackExpectTx);
	//Length = ptr[0] & 0xffff;

	pAd->LoopBackExpectTxLen = Length;
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DEBUG_TRACE,("%s, Length = %d\n",__FUNCTION__, Length));
	for(i=0;i<20;i++)
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE,("TXD(TXWI) %d 0x%08X\n",i, *(ptr+i)));
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DEBUG_TRACE,("%s, \n",__FUNCTION__));
}

void LoopBack_Run(RTMP_ADAPTER *pAd, LOOPBACK_SETTING *pSetting, UINT32 Length)
{
	if(Length> LOOPBACK_SIZE)
	{
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DEBUG_TRACE,("%s, LOOPBACK length too long\n",__FUNCTION__));	
		return;
	}
#ifdef RTMP_USB_SUPPORT
	USBLoopBack_Run(pAd, pSetting, Length);
#endif
#ifdef RTMP_SDIO_SUPPORT
	SDIOLoopBack_Run(pAd, pSetting, Length);
#endif

	
}
void LoopBack_BitTrueCheck(RTMP_ADAPTER *pAd)
{	
	if(!pAd->LoopBackDefaultPattern)//Rx compare expect Rx
	{
		if(pAd->LoopBackExpectRxLen!=pAd->LoopBackRxRawLen)
		{
			LoopBack_Fail(pAd, BIT_TRUE_FAIL);
			MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DEBUG_TRACE,("%s, ####################### TX/RX Length not equal ####################\n",__FUNCTION__));			
			MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DEBUG_TRACE,("%s, ExpectRxLen = %d, RxRawLen = %d\n",__FUNCTION__, pAd->LoopBackExpectRxLen, pAd->LoopBackRxRawLen));
		}
		else if(0 == RTMPEqualMemory((PVOID)&pAd->LoopBackExpectRx, (PVOID)&pAd->LoopBackRxRaw, pAd->LoopBackRxRawLen))
		{
			LoopBack_Fail(pAd, BIT_TRUE_FAIL);
			MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DEBUG_TRACE,("%s, ####################### BIT_TRUE_FAIL ####################\n",__FUNCTION__));
		}
		else
			pAd->LoopBackResult.FailReason = NO_ERROR;
	}
	else//Rx compare Tx
	{
		if(pAd->LoopBackTxRawLen!=pAd->LoopBackRxRawLen)
		{
			LoopBack_Fail(pAd, BIT_TRUE_FAIL);
			MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DEBUG_TRACE,("%s, ####################### TX/RX Length not equal ####################\n",__FUNCTION__));
			MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DEBUG_TRACE,("%s, TxRawLen = %d, RxRawLen = %d\n",__FUNCTION__, pAd->LoopBackTxRawLen, pAd->LoopBackRxRawLen));
		}
		else if(0 == RTMPEqualMemory((PVOID)&pAd->LoopBackTxRaw, (PVOID)&pAd->LoopBackRxRaw, pAd->LoopBackTxRawLen))
		{
			LoopBack_Fail(pAd, BIT_TRUE_FAIL);
			MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DEBUG_TRACE,("%s, ####################### BIT_TRUE_FAIL ####################\n",__FUNCTION__));
		}
		else
		{
			pAd->LoopBackResult.FailReason = NO_ERROR;
			MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DEBUG_TRACE,("%s, ####################### BIT_TRUE OK ####################\n",__FUNCTION__));
		}
	}
}

void LoopBack_Fail(RTMP_ADAPTER *pAd, LOOPBACK_FAIL FailNum)
{
	if(pAd->LoopBackResult.Status == RUNNING)
	{
		pAd->LoopBackResult.FailReason = FailNum;
		pAd->LoopBackResult.Status = FAIL;
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DEBUG_TRACE,("%s, FailReason = %d\n",__FUNCTION__, FailNum));
	}
}

void LoopBack_TxThread(IN OUT PVOID Context)
{
	RTMP_ADAPTER	*pAd = (RTMP_ADAPTER *)Context;
	LOOPBACK_SETTING *pSetting = &pAd->LoopBackSetting;
	UINT32 RepeatIdx = 0;
	UINT32 Length = 0;
	UINT32 BreakCount = 0;
	UINT32 DbgCount = 0;
	KIRQL Irql;
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DEBUG_TRACE,("%s\n",__FUNCTION__));
	if(pSetting->RepeatTimes == 0)
		pSetting->RepeatTimes = 0xffffffff;
	
	for(RepeatIdx=0; RepeatIdx<pSetting->RepeatTimes ; RepeatIdx++)
	{
		if(pSetting->RepeatTimes == 0xffffffff)
			RepeatIdx=0;
		for(Length = pSetting->StartLen; Length<= pSetting->StopLen; Length++)
		{
		
			while(pAd->LoopBackWaitRx)
			{
				BreakCount++;
				RTMPusecDelay(50);
				if(BreakCount>2000)
				{
					MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DEBUG_ERROR,("%s, no Rx come back  Stop1!!!\n",__FUNCTION__));
					break;
				}
				MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DEBUG_ERROR,("%s, no Rx BreakCount = %d\n",__FUNCTION__, BreakCount));
			}
			if(BreakCount>2000)
			{
				MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DEBUG_ERROR,("%s, no Rx come back  Stop2!!!\n",__FUNCTION__));
				LoopBack_Fail(pAd, RX_TIMEOUT);				
				break;
			}
			MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DEBUG_ERROR,("%s, Length =  %d\n",__FUNCTION__, Length));
			BreakCount = 0;
			if(pAd->LoopBackRunning == FALSE || pAd->LoopBackResult.Status != RUNNING )
				break;			
			if(!pSetting->IsDefaultPattern)
			{
				Length = pAd->LoopBackExpectTxLen;
			}
			if(pAd->LoopBackResult.Status == RUNNING)
			{
				KeAcquireSpinLock(&pAd->LoopBackLock, &Irql);
				pAd->LoopBackWaitRx = TRUE;
				KeReleaseSpinLock(&pAd->LoopBackLock, Irql);
				LoopBack_Run(pAd, pSetting, Length);
			}
			RTMPusecDelay(200);
			
			DbgCount++;
			MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DEBUG_ERROR,("%s, DbgCount =  %d\n",__FUNCTION__, DbgCount));
			if(!pSetting->IsDefaultPattern)
			{
				// use script file does not need to auto increase length
				break;
			}
		}
		if(pAd->LoopBackRunning == FALSE || pAd->LoopBackResult.Status != RUNNING )
			break;
	}	
	if(pAd->LoopBackRunning)
		LoopBack_Stop(pAd);
	pAd->LoopBackTxThread = NULL;

	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_INFO, ("+ + + + Control Thread Terminated + + + + \n"));

	//DecrementIoCount(pAd);

	PsTerminateSystemThread(STATUS_SUCCESS);
}  



