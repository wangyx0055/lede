
#include "config.h"

NTSTATUS	USBVendorRequest(
	IN	RTMP_ADAPTER	*pAdapter,
	IN	PVOID			pData,
	IN	UINT16			Length,
	IN	UINT8			Request,
	IN	UINT8			Value,
	IN	UINT8			index,
	IN	BOOLEAN			InOut)
{
	NTSTATUS	ntStatus = STATUS_SUCCESS;
	UINT32 TransferFlags;
       UINT8 ReservedBits;
	// Allocate memory for URB
	USHORT UrbSize = sizeof(struct _URB_CONTROL_VENDOR_OR_CLASS_REQUEST);
	PURB urb = (PURB)ExAllocatePool(NonPagedPool, UrbSize);
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("-->%s\n", __FUNCTION__));
	if( urb==NULL)
	{
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE,("No URB memory\n"));
		return STATUS_INSUFFICIENT_RESOURCES;
	}
	if(pData == NULL)
	{
		Length = 0;
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DEBUG_WARN,("%s, no pData\n", __FUNCTION__));
	}
	if(InOut == VendorReqOut)
	{
		TransferFlags = USBD_TRANSFER_DIRECTION_OUT;
		ReservedBits = DEVICE_VENDOR_REQUEST_OUT;
	}
	else//VendorReqIn
	{
		TransferFlags = USBD_TRANSFER_DIRECTION_IN | USBD_SHORT_TRANSFER_OK;
		ReservedBits = DEVICE_VENDOR_REQUEST_IN;
	}
	

	// Build URB to send vendor control request on Default pipe
	UsbBuildVendorRequest(urb,
		URB_FUNCTION_VENDOR_DEVICE, UrbSize,
		TransferFlags,
		ReservedBits,						// Reserved bits
		Request,						// Request
		Value,					// Value
		index,			// index
		(UINT16 *)pData, NULL, Length,				// Output data
		NULL);

	// Call the USB driver
	ntStatus = CallUSBDI( pAdapter, urb, IOCTL_INTERNAL_USB_SUBMIT_URB, 0);
	// Check statuses
	if( !NT_SUCCESS(ntStatus) || !USBD_SUCCESS( urb->UrbHeader.Status))
	{
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE,("status %x URB status %x\n", ntStatus, urb->UrbHeader.Status));
		ntStatus = STATUS_UNSUCCESSFUL;
	}
	ExFreePool(urb);	

	return ntStatus;
}

NTSTATUS	FwCmdUSBBulkout(
	IN	RTMP_ADAPTER	*pAdapter,
	IN	PUCHAR			pCmd,
	IN	UINT32			Length)
{
	NTSTATUS	ntStatus = STATUS_SUCCESS;
	UINT16		BreakCount = 0;
	KEVENT *Waitevent;
	UINT8 BulkoutNum = BULKOUT_PIPENUMBER_1;

	if(pAdapter->FWMode == FWRAM)
	{
		BulkoutNum = BULKOUT_PIPENUMBER_1;
	}
	else
	{
		BulkoutNum = BULKOUT_PIPENUMBER_0;
	}
	//loop back
	if(pAdapter->LoopBackRunning)
	{
		BulkoutNum = pAdapter->LoopBackBulkoutNumber;
	}	
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DEBUG_TRACE,("%s @#@#-> bulkout num = %d\n", __FUNCTION__, BulkoutNum));

	RtlZeroMemory(pAdapter->TxBuffer[BulkoutNum],  USB_BULK_BUFFER_COUNT*USB_BULK_BUFFER_SIZE );
	/*************************************************************/
	// Setup packet
	// driver must pad 4 bytes 0 at the ending of  frame
	pAdapter->TxBufferLength[BulkoutNum] = Length + 4;	
	RtlMoveMemory(pAdapter->TxBuffer[BulkoutNum], pCmd, Length);
	/*************************************************************/
	/*************************************************************/
	while(1)
	{
		if(pAdapter->bBulkOutRunning[BulkoutNum] == FALSE)
			break;
		RTMPusecDelay(100);
		if( 1000 == BreakCount++)
			break;
	}
	/*
	if(pAdapter->bLoadingFW)
	{
		Waitevent = &pAdapter->LoadFWEvent;
	}
	else
	{
		Waitevent = &pAdapter->WaitFWEvent;
	}
	*/
	if(pAdapter->bBulkOutRunning[BulkoutNum] == TRUE)
	{
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE,("BulkOut [%d] is still running\n", BulkoutNum));
		ntStatus = STATUS_UNSUCCESSFUL;	
	}
	else
	{
		pAdapter->bBulkOutRunning[BulkoutNum] = TRUE;
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE,("BulkOut [%d]\n", BulkoutNum));
		//Kick bulk out
		pAdapter->bIsWaitFW = TRUE;
		ntStatus = UsbDoBulkOutTransfer(pAdapter,BulkoutNum);	
	}
	
	return ntStatus;
}

NTSTATUS USBGetChipID(RTMP_ADAPTER *pAd)
{
	NTSTATUS	ntStatus = STATUS_SUCCESS;
	UINT32 		Value = 0;
	ntStatus = RTMP_IO_READ32(pAd, MCU_CFG_HW_CODE, &Value);
	pAd->ChipID = Value & 0x0000ffff;
	return ntStatus;
}

//init USB only adapter block
NTSTATUS USBInitAdapter(RTMP_ADAPTER *pAd)
{
	NTSTATUS	ntStatus = STATUS_SUCCESS;

	//todo

	return ntStatus;
}

//init USB only interface
NTSTATUS USBInitInterface(RTMP_ADAPTER *pAd)
{
	NTSTATUS	ntStatus = STATUS_SUCCESS;

	//todo

	return ntStatus;
}

NTSTATUS USBSetupTxPacket(RTMP_ADAPTER *pAd, UCHAR *pData, UINT32 Length, UINT32 BulkOutWhichPipeNeedRun)
{
	NTSTATUS	ntStatus = STATUS_SUCCESS;
	UINT32 PipeIdx = 0;
	for(PipeIdx = 0; PipeIdx<MAX_TX_BULK_PIPE_NUM ;PipeIdx++)
	{
		if(BulkOutWhichPipeNeedRun & (0x1 << PipeIdx))
		{
			pAd->TxBufferLength[PipeIdx] = Length +4;
			RtlZeroMemory(pAd->TxBuffer[PipeIdx],  USB_BULK_BUFFER_COUNT*USB_BULK_BUFFER_SIZE );
			RtlMoveMemory(pAd->TxBuffer[PipeIdx], pData, Length);
		}
	}

	//pAd->bMinLen = 44;
	//pAd->bCurrlength= pAd->bMinLen-1;

	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE,("dx->TxBufferLength[%d] = %d",PipeIdx, pAd->TxBufferLength[PipeIdx]));
	return ntStatus;

}
NTSTATUS USBStartTxPacket(RTMP_ADAPTER *pAd, UINT32 BulkOutWhichPipeNeedRun, UINT32 TxRemained)
{
	NTSTATUS	ntStatus = STATUS_SUCCESS;
	UINT32		BreakCount = 0;
	UINT32 PipeIdx = 0;
		
	pAd->BulkOutWhichPipeNeedRun = BulkOutWhichPipeNeedRun;
	RtlZeroMemory(&pAd->TransmittedCount, sizeof(UINT32));

	for(PipeIdx = 0; PipeIdx<MAX_TX_BULK_PIPE_NUM ;PipeIdx++)
	{		
		if(BulkOutWhichPipeNeedRun & (0x1 << PipeIdx))
		{
			MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE,("@#@# %s bBulkOutRunning[%d] = %d", __FUNCTION__, PipeIdx, pAd->bBulkOutRunning[PipeIdx]));
			if(PipeIdx != TX_CMD_BUFEER_NUM)//FW Cmd
			{
				if (TxRemained == 0)
				{		
					pAd->TxProcessing = TRUE;//infinite Tx 
				}
				else
					pAd->TxProcessing = FALSE;
			}
			RtlZeroMemory(&pAd->TxBulkCount[PipeIdx], sizeof(UINT32));
			pAd->TxRemained[PipeIdx] = TxRemained;
			do
			{
				if(pAd->bBulkOutRunning[PipeIdx] == FALSE)
					break;
				BreakCount++;
				if(BreakCount >= 1000)
				{
					MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE,("bBulkOutRunning 1000 times\n"));
					break;
				}
				RTMPusecDelay(10);
			}while(1);
			if(pAd->bBulkOutRunning[PipeIdx] == TRUE)
			{
				MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE,("bBulkOutRunning\n"));
			}
			else
			{
				pAd->bBulkOutRunning[PipeIdx] = TRUE;
				MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE,("BulkOutWhichPipeNeedRun = %d\n", BulkOutWhichPipeNeedRun));
				//Kick bulk out
				UsbDoBulkOutTransfer(pAd,PipeIdx);					
			}
			
		}
	}
#if 0	
	do
	{
		if(pAd->bBulkOutRunning == FALSE)
			break;
		BreakCount++;
		if(BreakCount >= 1000)
		{
			MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE,("bBulkOutRunning 1000 times\n"));
			break;
		}
		RTMPusecDelay(10);
	}while(1);
	if(pAd->bBulkOutRunning == TRUE)
	{
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE,("bBulkOutRunning\n"));
	}
	else
	{
		pAd->bBulkOutRunning = TRUE;

		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE,("BulkOutWhichPipeNeedRun = %d\n", BulkOutWhichPipeNeedRun));
		//Kick bulk out
		if(BulkOutWhichPipeNeedRun & 0x0001)
			UsbDoBulkOutTransfer(pAd,0);	

		if(BulkOutWhichPipeNeedRun & 0x0002)
			UsbDoBulkOutTransfer(pAd,1);

		if(BulkOutWhichPipeNeedRun & 0x0004)
			UsbDoBulkOutTransfer(pAd,2);

		if(BulkOutWhichPipeNeedRun & 0x0008)
			UsbDoBulkOutTransfer(pAd,3);

		if(BulkOutWhichPipeNeedRun & 0x0010)
			UsbDoBulkOutTransfer(pAd,4);

		if(BulkOutWhichPipeNeedRun & 0x0020)
			UsbDoBulkOutTransfer(pAd,5);
	}
#endif	
	return ntStatus;
}
NTSTATUS USBStopTxPacket(RTMP_ADAPTER *pAd)
{
	NTSTATUS	ntStatus = STATUS_SUCCESS;
	pAd->TxRemained[BULKOUT_PIPENUMBER_0] = 0;
	pAd->TxProcessing= FALSE;
	return ntStatus;
}

NTSTATUS RXStartStop(
	IN  RTMP_ADAPTER   *pAdapter,
       IN  BOOLEAN 	bIsStart,
    	IN  UINT16	Pipe
	)
{
	NTSTATUS	Status = STATUS_SUCCESS;

	if(pAdapter->bRxEnable[Pipe] == bIsStart)
	{
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("no need to do, status is the same = %d\n", bIsStart));
		return Status;
	}
		
	if(bIsStart)
	{
		Status = RXStart_USB(pAdapter, Pipe);
	}
	else
	{
		Status = RXStop_USB(pAdapter, Pipe);
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("Stop Rx %d\n", Pipe));
	}
	return Status;
}
UINT8 GetBulkoutNumber(IN  RTMP_ADAPTER   *pAdapter)
{
	UINT8 BulkOutNumber = 0;
	UINT32 Reg = 0;
	RTMP_IO_READ32(pAdapter, SCH_REG4, &Reg);
	Reg = Reg&0xf; //get Qid
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("%s Qid = %d\n", __FUNCTION__,Reg));
	switch (Reg)
	{
		case EP4QID:			
			BulkOutNumber = 1;
		break;	
		case EP5QID:
			BulkOutNumber = 2;
		break;	
		case EP6QID:
			BulkOutNumber = 3; 
		break;	
		case EP7QID:
			BulkOutNumber = 4;
		break;	
		case EP8QID:
			BulkOutNumber = 0;
		break;				
		case EP9QID:
			BulkOutNumber = 5;
		break;	
		case EPALLQID:
			BulkOutNumber = 0;
		break;
		default:
			BulkOutNumber = 0;
	}	
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("%s BulkOutNumber = %d\n", __FUNCTION__,BulkOutNumber));
	return BulkOutNumber;
}

NTSTATUS USBCheckFWROMWiFiSysOn(IN  RTMP_ADAPTER   *pAd)
{
	NTSTATUS ntStatus = STATUS_SUCCESS;
	UINT32 BreakCount = 0;
	//Step 1 polling 0x81021250 to 0
	
	do
	{
		ULONG CRValue = 0;
		RTMP_IO_READ32(pAd, SW_SYNC, &CRValue);
		if (CRValue == 0x0)
		{
			MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DEBUG_TRACE, ("!!!!! WiFi sys off !!!!!\n"));
			pAd->FWMode = FWROM;
			break;
		}
		        
		RTMPusecDelay(10000);// 100 ms		        
			BreakCount ++;
		if (BreakCount > 100)
		{
			MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DEBUG_ERROR,("Polling WiFi sys off failed 0x81021250 = 0x%x\n", CRValue));
			break;
		}		        
	} while(1);
	//Step 2 power on wifi sys
	ntStatus = ROMPatchPowerOnWiFiSys(pAd);
	if(ntStatus != STATUS_SUCCESS)
	{
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DEBUG_ERROR,("%s, PowerOnWiFiSys \n",__FUNCTION__));
	}
	//Step 3 polling 0x81021250 to 1
	BreakCount = 0;//reset count to 0
	do
	{
		ULONG CRValue = 0;
		RTMP_IO_READ32(pAd, SW_SYNC, &CRValue);
		if (CRValue == 0x1)
		{
			MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DEBUG_TRACE, ("!!!!! WiFi sys on !!!!!\n"));
			pAd->FWMode = FWROM;
			break;
		}
		        
		RTMPusecDelay(10000);// 100 ms		        
			BreakCount ++;
		if (BreakCount > 100)
		{
			MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DEBUG_ERROR,("Polling WiFi sys on failed 0x81021250 = 0x%x\n", CRValue));
			break;
		}		        
	} while(1);
	return ntStatus;
}
void USBLoopBack_Run(RTMP_ADAPTER *pAd, LOOPBACK_SETTING *pSetting, UINT32 length)
{
	TX_WI_STRUC pTxWI;
	UINT32 bulkoutpipe = 0, pipenum = 0;	
	PUCHAR ptr;// = pAd->TxBuffer[pSetting->BulkOutNumber] + sizeof(TX_WI_STRUC);	
	UINT32 count = 0;
	//UINT32 length = pSetting->StartLen;
	//UINT32 Startlength = pSetting->StartLen;
	//UINT32 Stoplength = pSetting->StopLen;
	UINT8 alignment = 0;
	LARGE_INTEGER       TimeOut1Second;		
	TimeOut1Second.QuadPart = 0;
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DEBUG_TRACE,("%s, \n",__FUNCTION__));
	//length = 1024;
	switch (pSetting->BulkOutNumber)
	{
		case EP4QID:
			bulkoutpipe = 0x02;
			pipenum = 1;
		break;	
		case EP5QID:
			bulkoutpipe = 0x04;
			pipenum = 2;
		break;	
		case EP6QID:
			bulkoutpipe = 0x08;
			pipenum = 3; 
		break;	
		case EP7QID:
			bulkoutpipe = 0x10;
			pipenum = 4;
		break;	
		case EP8QID:
			bulkoutpipe = 0x01;
			pipenum = 0;
		break;				
		case EP9QID:
			bulkoutpipe = 0x20;
			pipenum = 5;
		break;	
		case EPALLQID:
			bulkoutpipe = 0x3f;
			//bulkoutpipe = 0x01;
			pipenum = 0;
		break;
		default:
			bulkoutpipe = 0;
	}	
	//Packet formation for Loopback Tx
	if(pSetting->IsDefaultPattern)
	{		
		RtlZeroMemory(pAd->TxBuffer[pipenum], USB_BULK_BUFFER_COUNT*USB_BULK_BUFFER_SIZE);
		RtlZeroMemory(&pTxWI, sizeof(TX_WI_STRUC));
		ptr = pAd->TxBuffer[pipenum] + sizeof(TX_WI_STRUC);	
		for(count = 0; count<length - sizeof(TX_WI_STRUC); count++)
		{
			ptr[count] = count%16;	
		}
		//set pkt content
		pTxWI.P_IDX= 1;
		pTxWI.Q_IDX= 0;		
		pTxWI.TXByteCount =  length;
		pTxWI.FT = 1;
		pTxWI.HF = 1;
		RtlCopyMemory(pAd->TxBuffer[pipenum], &pTxWI, sizeof(TX_WI_STRUC));
		
		RtlCopyMemory(&pAd->LoopBackTxRaw, pAd->TxBuffer[pipenum], length);
		pAd->LoopBackTxRawLen = length;
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DEBUG_TRACE,("%s, length = %d\n",__FUNCTION__, length));		
	}
	else
	{
		RtlCopyMemory(pAd->TxBuffer[pipenum], &pAd->LoopBackExpectTx, LOOPBACK_SIZE);
		RtlZeroMemory(pAd->TxBuffer[pipenum] + length , LOOPBACK_SIZE - length);
		RtlCopyMemory(&pAd->LoopBackTxRaw, pAd->TxBuffer[pipenum], length);	
		pAd->LoopBackTxRawLen = pAd->LoopBackExpectTxLen;
	}
	//Tx Aggregation only in IsDefaultPattern
	if(pSetting->IsDefaultPattern)
	{
		if(pSetting->TxAggNumber>1 && pSetting->TxAggNumber<=16)
		{
			ptr = pAd->TxBuffer[pipenum] + length;
			for(count = 1; count < pSetting->TxAggNumber; count++)
			{			
				if(length * count > USB_BULK_BUFFER_COUNT*USB_BULK_BUFFER_SIZE -7 )
					break;// exceed size limit			
				RtlCopyMemory(ptr, pAd->TxBuffer[pipenum], length);
				ptr+=length;
			}
			length = pSetting->TxAggNumber * length;
			MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DEBUG_TRACE,("%s, Loopback agg number = %d, size = %d\n",__FUNCTION__, count, length));
			//Tx Agg need to update for Bit true check			
			RtlCopyMemory(&pAd->LoopBackTxRaw, pAd->TxBuffer[pipenum], length);
			pAd->LoopBackTxRawLen = length;			
		}	
		if(length % 4 != 0)
			alignment = 4 - (length % 4);
	}
	if(pSetting->BulkOutNumber == EPALLQID)
	{
		UINT8 idx = 0;
		for(idx = 0; idx <6;idx++)
		{
			//pAd->TxBufferLength[idx] = sizeof(TX_WI_STRUC) + length + alignment + 4 ;
			MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DEBUG_TRACE,("%s, start tx idx = %d\n",__FUNCTION__, idx));
			if(pSetting->IsDefaultPattern)
				pAd->TxBufferLength[idx] = length + alignment + 4 ;
			else if(!pSetting->IsDefaultPattern)
				pAd->TxBufferLength[idx] = length;
			RtlCopyMemory(pAd->TxBuffer[idx], pAd->TxBuffer[pipenum], length);
			//ptr = pAd->TxBuffer[idx] + sizeof(TX_WI_STRUC);
			//ptr[0] = idx;
			MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DEBUG_TRACE,("%s, LP length = %d, alignment = %d\n",__FUNCTION__, length, alignment));
			//pAd->TxBufferLength[idx] = length + alignment + 4 ;
			pAd->LoopBackResult.LastBulkOut = pSetting->BulkOutNumber;
			pAd->LoopBackResult.TxByteCount += length;//pAd->TxBufferLength[pipenum] - 4;
			if(pSetting->IsDefaultPattern)
				pAd->LoopBackResult.TxPktCount += pSetting->TxAggNumber;
			else
				pAd->LoopBackResult.TxPktCount ++;
			StartTx(pAd, 1, 0x1 << idx);	
			if(STATUS_SUCCESS != Event_Wait(pAd, &pAd->LoopBackTxEvent[idx],TimeOut1Second))
			{		
				LoopBack_Fail(pAd, TX_TIMEOUT);
				MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DEBUG_TRACE,("%s, ####################### TX_TIMEOUT ####################\n",__FUNCTION__));
				break;
			}			
			if(STATUS_SUCCESS != Event_Wait(pAd, &pAd->LoopBackEvent,TimeOut1Second))
			{		
				LoopBack_Fail(pAd, RX_TIMEOUT);
				MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DEBUG_TRACE,("%s, ####################### RX_TIMEOUT ####################\n",__FUNCTION__));
				break;
			}
		}
	}
	else
	{
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DEBUG_TRACE,("%s, LP length = %d, alignment = %d\n",__FUNCTION__, length, alignment));
		if(pSetting->IsDefaultPattern)
			pAd->TxBufferLength[pipenum] = length + alignment + 4 ;
		else if(!pSetting->IsDefaultPattern)
			pAd->TxBufferLength[pipenum] = length;
		pAd->LoopBackResult.LastBulkOut = pSetting->BulkOutNumber;
		pAd->LoopBackResult.TxByteCount += length;//pAd->TxBufferLength[pipenum] - 4;
		if(pSetting->IsDefaultPattern)
			pAd->LoopBackResult.TxPktCount += pSetting->TxAggNumber;
		else
			pAd->LoopBackResult.TxPktCount ++;
		StartTx(pAd, 1, bulkoutpipe);	
		if(STATUS_SUCCESS != Event_Wait(pAd, &pAd->LoopBackTxEvent[pipenum],TimeOut1Second))
		{		
			LoopBack_Fail(pAd, TX_TIMEOUT);
			MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DEBUG_TRACE,("%s, ####################### TX_TIMEOUT ####################\n",__FUNCTION__));
		}
	}

}

NTSTATUS ConfigDMAScheduler(RTMP_ADAPTER *pAd, UINT8 Qid )
{
	NTSTATUS 				ntStatus = STATUS_SUCCESS;
	SCHEDULER_REGISTER4 	SchedulerRegister4;//, backupSchedulerRegister4;
	UINT32					Idx = 0;

	if(Qid < EPALLQID)
	{
		ntStatus = RTMP_IO_READ32(pAd, SCH_REG4, &SchedulerRegister4.word);

		if (ntStatus != STATUS_SUCCESS)
		{
			MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,("%s Read Mac Register SCH_REG4 failed\n", __FUNCTION__));
			return STATUS_UNSUCCESSFUL;
		}
		    
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE,("SchedulerRegister4 = %x\n", SchedulerRegister4.word));
		
		{
			SchedulerRegister4.Default.ForceQid = Qid;
			//SchedulerRegister4.Default.ForceMode = 1;
		}
		SchedulerRegister4.Default.BypassMode = 1;	
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE,("2 SchedulerRegister4 = %x\n", SchedulerRegister4.word));
		//if(Qid == EP4QID)
		ntStatus = RTMP_IO_WRITE32(pAd, SCH_REG4, SchedulerRegister4.word);
		
		if (ntStatus != STATUS_SUCCESS)
		{
			MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,("%s Write Mac Register SCH_REG4 failed\n", __FUNCTION__));
			return STATUS_UNSUCCESSFUL;
		}
		
	}
	else if(Qid == EPALLQID)// all bulk out
	{
		//backup CR
		for (Idx = 0; Idx<35; Idx++)
		{			
			RTMP_IO_READ32(pAd, 0x50000540 + Idx*4, &pAd->BackupCR[Idx]);
			MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,("%s @@BACKUP 0x50000540 + %d*4 = 0x%x\n", __FUNCTION__, Idx, pAd->BackupCR[Idx]));
		}
		RTMP_IO_WRITE32(pAd, 0x50000540,0xe);
		RTMP_IO_WRITE32(pAd, 0x50000544,0xe);
		RTMP_IO_WRITE32(pAd, 0x50000548,0xe);
		RTMP_IO_WRITE32(pAd, 0x5000054c,0xe);
		RTMP_IO_WRITE32(pAd, 0x50000550,0xe);
		RTMP_IO_WRITE32(pAd, 0x50000554,0x20);
		RTMP_IO_WRITE32(pAd, 0x50000558,0x0);
		RTMP_IO_WRITE32(pAd, 0x5000055c,0xe);
		RTMP_IO_WRITE32(pAd, 0x50000560,0x32);
		RTMP_IO_WRITE32(pAd, 0x50000564,0x0);
		RTMP_IO_WRITE32(pAd, 0x50000568,0x0);
		RTMP_IO_WRITE32(pAd, 0x5000056c,0x0);
		RTMP_IO_WRITE32(pAd, 0x50000570,0x0);
		RTMP_IO_WRITE32(pAd, 0x50000574,0x0);
		RTMP_IO_WRITE32(pAd, 0x50000578,0x0);
		RTMP_IO_WRITE32(pAd, 0x5000057c,0x0);
		RTMP_IO_WRITE32(pAd, 0x50000580,0x2b1a096e);	
		RTMP_IO_WRITE32(pAd, 0x50000584,0x785f4d3c);
		RTMP_IO_WRITE32(pAd, 0x50000588,0x20000288);
		RTMP_IO_WRITE32(pAd, 0x5000058c,0xe);
		RTMP_IO_WRITE32(pAd, 0x50000590,0x0);
		RTMP_IO_WRITE32(pAd, 0x50000594,0x40);
		RTMP_IO_WRITE32(pAd, 0x50000598,0x27a);
		RTMP_IO_WRITE32(pAd, 0x5000059c,0xe);
		RTMP_IO_WRITE32(pAd, 0x500005a0,0x0);
		RTMP_IO_WRITE32(pAd, 0x500005a4,0x0);
		RTMP_IO_WRITE32(pAd, 0x500005a8,0x0);
		RTMP_IO_WRITE32(pAd, 0x500005ac,0x0);
		RTMP_IO_WRITE32(pAd, 0x500005b0,0x80ff7f);
		RTMP_IO_WRITE32(pAd, 0x500005b4,0x0);
		RTMP_IO_WRITE32(pAd, 0x500005b8,0x0);
		RTMP_IO_WRITE32(pAd, 0x500005bc,0x2b1a096e);
		RTMP_IO_WRITE32(pAd, 0x500005c0,0x785f4d3c);
		RTMP_IO_WRITE32(pAd, 0x500005c4,0xffffffff);
		RTMP_IO_WRITE32(pAd, 0x500005c8,0x1e2);

	}
	else
	{
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE,("Qid exceed limit\n" ));
	}
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DEBUG_TRACE,("%s, \n",__FUNCTION__));
	return ntStatus;
}

