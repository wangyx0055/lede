#include "sdio_config.h"
//#include "sdio.h"

//#include "sdio_write.h"

#ifdef ALLOC_PRAGMA
//	#pragma alloc_text (PAGE, MT6620EvtDeviceAdd)
//	#pragma alloc_text (PAGE, MT6620EvtIoDeviceControl)
#endif
 
//
// write routine, to serve WriteFile()
//

VOID
SDIOEvtIoWrite (
               WDFQUEUE      Queue,
               WDFREQUEST    Request,
               size_t         Length
               )
{    
    WDFDEVICE        device;
    RTMP_ADAPTER	*pAdapter;
    NTSTATUS        status;
	//SLEEPWAKEUP      chipsleepstatus= STATUS_SLEEP;
 	PREQUEST_CONTEXT		pReqContext = NULL;

     

	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("\n>>enter SDIOEvtIoWrite(), Request = 0x%p\n", Request));
	

    device = WdfIoQueueGetDevice(Queue);
    pAdapter = SDIOpAdGetData(device);
	pReqContext = GetRequestContext(Request);
 	/*
	status = IoAcquireRemoveLock(&(pAdapter->io_remove_lock), NULL);
	if(status != STATUS_SUCCESS )
	{
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, "IoAcquireRemoveLock failed in EvtIoWrite, 0x%x\n", status);
		WdfRequestComplete(Request, STATUS_CANCELLED);
		return;
	}
    */
    
	//check chip sleep or wakeup, if sleeping, then cannot write
	//SleepStatusGetStatus(&(pAdapter->sleepstatus), &chipsleepstatus); 
/*	if( STATUS_SLEEP == chipsleepstatus)
	{   // The device is not in a valid state to perform this request.
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, "[error] chip is in sleep status, please wake up it before write Tx");
	    MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, "[error] leave SDIOEvtIoWrite()\n");
		WdfRequestComplete(Request, STATUS_DEVICE_IS_SLEEPING);
		//IoReleaseRemoveLock(&(pAdapter->io_remove_lock), NULL);
	    MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, "Completed Request = 0x%p\n", Request);
		return;
	}*/
 
	//write 0 byte
	if(Length <=0)
	{
		WdfRequestCompleteWithInformation(Request, STATUS_SUCCESS, 0);
	    MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("Completed Request = 0x%p\n", Request));
		//IoReleaseRemoveLock(&(pAdapter->io_remove_lock), NULL);
		return;
	}
	if(Length > MAX_TX_FIFO_SIZE)
	{	
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("[error] The written data length (%d) great than MAX_TX_FIFO_SIZE(%d)", Length, MAX_TX_FIFO_SIZE));
	    MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("[error] leave SDIOEvtIoWrite()\n"));
		WdfRequestComplete(Request, STATUS_DATA_OVERRUN);
	    MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("Completed Request = 0x%p\n", Request));
		//IoReleaseRemoveLock(&(pAdapter->io_remove_lock), NULL);
		return;
	}
 
	//set information
	pReqContext->Length = Length;
	pReqContext->status=0;
	pReqContext->pAdapter = pAdapter;
	pReqContext->SystemBuffer=NULL;

	if(pAdapter->useSingleTx)
	{
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE,("use single Tx\n"));
		WriteTx(pAdapter, Request, Length);
	}
	else
	{
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("use overlapped Tx\n"));
		
		if(!IsChipWriteable(pAdapter, Length))
		{
			//interrupt call back will deal with this Request
			//when Tx Empty or Tx FifoUnderThold
			WdfRequestMarkCancelable(Request, EvtRequestCancelWrite );

			///add for bug fix:			
			//should lock to protect
			if(pAdapter->PendingWriteRequest==NULL)
			{
				pAdapter->PendingWriteRequest = Request;
			}
			else
			{
				//never get here
				MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("Lost Request!!!!!\n"));
			}
            

			MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("chip is not writable, wait interrupt to deal with request\n"));
			//IoReleaseRemoveLock(&(pAdapter->io_remove_lock), NULL);
			return;
		}
		
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("chip is writable, write Tx\n"));
		WriteTx(pAdapter, Request, Length);
	}

	//IoReleaseRemoveLock(&(pAdapter->io_remove_lock), NULL);

	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("leave SDIOEvtIoWrite()\n"));
    
	return;  
}

BOOLEAN IsChipWriteable(RTMP_ADAPTER *pAdapter, size_t originalLen)
{	
	size_t paddinglen=0;
	USHORT fifobytes=0;
	UCHAR  txpktcnt=0;

	//KeWaitForSingleObject(&(pAdapter->firmwareinfo.hMutex), Executive, KernelMode, FALSE, NULL); //
	//fifobytes = pAdapter->firmwareinfo.fifo_free_bytes;
	//txpktcnt = pAdapter->firmwareinfo.TxPacketCount;
	//KeReleaseMutex(&(pAdapter->firmwareinfo.hMutex), FALSE);

	//fifo free count 
	//paddinglen = GetPaddingSizeByMode(pAdapter, originalLen);
    if(fifobytes < paddinglen )
	{
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("pAdapter->firmwareinfo.fifo_free_bytes = %d < paddinglen=%d\n", fifobytes, paddinglen));
		return FALSE;
	}

	//chip sleep? this is filtered early
    //

	//TxCompleteCount > 7 ? 

	if(txpktcnt >= MAX_TX_PACKET_COUNT)
	{
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("pAdapter->firmwareinfo.TxPacketCount = %d >= MAX_TX_PACKET_COUNT\n", txpktcnt));
		return FALSE;
	}

	return TRUE;
}

VOID ContinuousWriteTx(RTMP_ADAPTER *pAdapter)
{ 
    NTSTATUS    status;
	WDFREQUEST	Request = NULL;
	size_t      Length=0;
	USHORT      fifo_free_bytes;  
	PREQUEST_CONTEXT		pReqContext = NULL;
	
	if(pAdapter->PendingWriteRequest)
	{
		Request = pAdapter->PendingWriteRequest;
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("Deal with pending write request: Request = 0x%p\n", Request));

		pReqContext = GetRequestContext(Request);

        Length = pReqContext->Length;
        if(!IsChipWriteable(pAdapter, Length))
		{
            MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("chip is not writeable, stop dealing : Request = 0x%p\n", Request));
			return;
		}
		//unmark calcelable
		status = WdfRequestUnmarkCancelable(Request);
		if( STATUS_CANCELLED == status )
		{
            MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("Request (0x%p) is (going to be) cancelled in cancel routing, stop WriteTx \n", Request));
			return;
		}

		WriteTx(pAdapter, Request, Length);
		pAdapter->PendingWriteRequest = NULL;
	}
/*  
    //this code is valid only when the write queue is manual queue
	//
    while(1)
	{	
		if(IsQueueEmpty(pAdapter->wdfWriteQueue))
		{
			MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, "[infor] ContinuousWriteTx(): WriteQueue is idle\n");
			break;
		}

		status = WdfIoQueueRetrieveNextRequest(pAdapter->wdfWriteQueue,&Request);
		if(STATUS_NO_MORE_ENTRIES == status)
		{
			MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, "[infor] no more entry\n");
			break;
		}
		else if(!NT_SUCCESS(status))
		{ 
			MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, "[error] WdfIoQueueRetrieveNextRequest failed: %d(0x%x)\n", status, status);
			break;
		}
        
		pReqContext = GetRequestContext(Request);

        Length = pReqContext->Length;
        if(!IsChipWriteable(pAdapter, Length))
		{
			//
			break;
		}
		WriteTx(pAdapter, Request, Length);
	}
*/
	return;
}

/*
BOOLEAN IsQueueEmpty(IN WDFQUEUE Queue)
{
	WDF_IO_QUEUE_STATE QState = WdfIoQueueGetState( Queue, NULL, NULL);
	return (WDF_IO_QUEUE_IDLE(QState)) ? TRUE : FALSE;
}
*/

//write one data
VOID WriteTx(RTMP_ADAPTER *pAdapter, WDFREQUEST Request, size_t Length)
{
    //WDFDEVICE        device;

    PMDL            mdlAddress;
    //WDF_REQUEST_PARAMETERS  parameters;
    NTSTATUS        status;
    ULONG            bytesRead=0;
	size_t          fifoFreeCnt=0;
	ULONG            alignsize=0;
	PUCHAR pNewBuffer=NULL; //
	PMDL prMdl= (PMDL)NULL;//
	PUCHAR pBuffer=NULL;
	
	UINT32 Value=0;
	UINT32 oldLen=0, newLen=0;

	SDIOHeader sdioheader;

    MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("\n>>enter WriteTx()\n"));
#if 0	
	oldLen = Length;
	status = WdfRequestRetrieveInputBuffer(Request, oldLen, &pBuffer, 0); 
	if (!NT_SUCCESS(status)) {
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, "[error] SDIOEvtIoWrite: RetrieveInputBuffer fail: %d (0x%08x)\n", status, status);
		WdfRequestComplete(Request, status);
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, "Completed Request = 0x%p\n", Request);
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, "[error] leave SDIOEvtIoWrite()\n");
		return;
	} 
 
	//newLen = GetPaddingSizeByMode(pAdapter, oldLen);	
	if(newLen >= 0x0000FFFF) //??
	{	//error
		WdfRequestComplete(Request, STATUS_INSUFFICIENT_RESOURCES);
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, "Completed Request = 0x%p\n", Request);
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, "[error] : internal error, block size should not great than 0xFFFF: %d (0x%x)\n", alignsize, alignsize);
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, "[error] leave SDIOEvtIoWrite()\n");
		return;
	}
			
	//here should use non paged pool
	pNewBuffer = (PUCHAR)ExAllocatePoolWithTag(NonPagedPool, newLen, 'etrW');
	if(!pNewBuffer)
	{
		WdfRequestComplete(Request, STATUS_INSUFFICIENT_RESOURCES);
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, "Completed Request = 0x%p\n", Request);
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, "[error] : allocate poolwithtag fail, length = %d (0x%x)\n", newLen, newLen);
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, "[error] leave SDIOEvtIoWrite()\n");
		return;
	}
	RtlZeroMemory(pNewBuffer, newLen);

	//construct SDIO header
	sdioheader.packetlen= (USHORT)(oldLen+4);
	sdioheader.reserved=0;
	RtlCopyMemory(pNewBuffer, &sdioheader, sizeof(SDIOHeader));
	RtlCopyMemory(pNewBuffer+sizeof(SDIOHeader), pBuffer, oldLen);
        
    
	prMdl = IoAllocateMdl(pNewBuffer, newLen, FALSE, FALSE, NULL);
	MmBuildMdlForNonPagedPool(prMdl);
		
	mdlAddress = prMdl;
 
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, "SDIOEvtIoWrite: length: %d (0x%x)\n", newLen, newLen);
	
	//KeWaitForSingleObject(&(pAdapter->firmwareinfo.hMutex), Executive, KernelMode, FALSE, NULL); //

	KeWaitForSingleObject(&pAdapter->hMutexWrite, Executive, KernelMode, FALSE, NULL);
	status = SdioReadWriteBuffer(pAdapter->WdfDevice, pAdapter->FunctionFocus,
		mdlAddress, 
		CTDR, 
		(ULONG)newLen, 
		FIFOMODE, 
		WRITETODEVICE, 
		&bytesRead);
    KeReleaseMutex(&pAdapter->hMutexWrite, FALSE);
	
	//WdfRequestCompleteWithInformation(Request, status, bytesRead);
	WdfRequestCompleteWithInformation(Request, status, Length);

	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, "Completed Request = 0x%p\n", Request);
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, "SDIOEvtIoWrite: status = %d (0x%x)   bytesWrite= %d (0x%x)\n", 
		 status, status, Length, Length);
	
	
	//pAdapter->firmwareinfo.TxPacketCount++;
	//pAdapter->firmwareinfo.fifo_free_bytes -= (USHORT)bytesRead;
	//KeReleaseMutex(&(pAdapter->firmwareinfo.hMutex), FALSE);

	if(prMdl)
	{
	    IoFreeMdl(prMdl);
		prMdl=NULL;
	}
	if(pNewBuffer)
	{
		ExFreePool(pNewBuffer);
		pNewBuffer=NULL;
	}
 #endif
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("leave WriteTx()\n"));

    return;  
}

//write one data
VOID IOWriteTx(RTMP_ADAPTER *pAdapter, PUCHAR pBuffer, size_t Length, BOOLEAN Async)
{
    //WDFDEVICE        device;   
    PMDL            mdlAddress;
    //WDF_REQUEST_PARAMETERS  parameters;
    NTSTATUS        status;
    ULONG            bytesRead=0;
	size_t          fifoFreeCnt=0;
//	ULONG            alignsize=0;
	PUCHAR pNewBuffer=NULL; //
	PMDL prMdl= (PMDL)NULL;//
	//PUCHAR pBuffer=NULL;
	PFIRMWARE_TXDSCR  pFwTxd=NULL;	
	UINT32 CountTmp = 0;
	
	UINT32 Value=0;
	UINT32 oldLen=0, newLen=0;
	


	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("%s, =>\n", __FUNCTION__));

	if(!pBuffer)
		return;
	oldLen = Length;
/*	
	status = WdfRequestRetrieveInputBuffer(Request, oldLen, &pBuffer, 0); 
	if (!NT_SUCCESS(status)) {
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, "[error] MT6620EvtIoWrite: RetrieveInputBuffer fail: %d (0x%08x)\n", status, status);
		WdfRequestComplete(Request, status);
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, "Completed Request = 0x%p\n", Request);
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, "[error] leave MT6620EvtIoWrite()\n");
		return;
	} 
*/
 	if(oldLen > pAdapter->BlockLength/* || (pAdapter->FWMode == FWRAM)*/)
		newLen = GetPaddingSizeByMode(pAdapter, oldLen);	
	else
		newLen = oldLen;

	if(newLen >= 0x0000FFFF) //??
	{	//error
		//WdfRequestComplete(Request, STATUS_INSUFFICIENT_RESOURCES);
		//MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, "Completed Request = 0x%p\n", Request);
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("[error] : internal error, block size should not great than 0xFFFF\n"));
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("[error] leave %s()\n", __FUNCTION__));
		return;
	}
/*	
	if(pAdapter->FWMode == FWRAM)
	{
		UINT32 pageused = 0;
		//Tx page 
		pageused = newLen/TX_PAGE_SIZE;
		if(newLen%TX_PAGE_SIZE != 0)
			pageused++;
		pAdapter->TxPageCount -= pageused;
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DEBUG_TRACE,("%s(): TxPageCount = %d used = %d\n",__FUNCTION__, pAdapter->TxPageCount, pageused));	
	}
*/			
	//here should use non paged pool
	pNewBuffer = (PUCHAR)ExAllocatePoolWithTag(NonPagedPool, newLen, 'etrW');
	if(!pNewBuffer)
	{
		//WdfRequestComplete(Request, STATUS_INSUFFICIENT_RESOURCES);
		//MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, "Completed Request = 0x%p\n", Request);
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("[error] : allocate poolwithtag fail, length = %d (0x%x)\n", newLen, newLen));
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE,("[error] leave %s()\n", __FUNCTION__));
		return;
	}
	
	//for (CountTmp=0; CountTmp<oldLen; CountTmp++)
	//{		
	//	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, "WriteTx buffer = 0x%x\n", pBuffer[CountTmp]);
	//}

	//pCmdAccessReg = pBuffer+sizeof(FIRMWARE_TXDSCR);
	//MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, "size = %d\n", sizeof(CMD_ACCESS_REG));
	//MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, " SetQuery = 0x%x\n", pCmdAccessReg->ucSetQuery);
	//MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, " u4Address = 0x%x\n", pCmdAccessReg->u4Address);
	//MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, " u4Data = 0x%x\n", pCmdAccessReg->u4Data);
	
	RtlZeroMemory(pNewBuffer, newLen);

 	RtlCopyMemory(pNewBuffer, pBuffer, newLen);
	//MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, "IoWrite: length: %d (0x%x)\n", newLen, newLen);
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("len = %d, new len = %d\n", oldLen, newLen));
	//KeWaitForSingleObject(&(pAdapter->firmwareinfo.hMutex), Executive, KernelMode, FALSE, NULL); //

	//KeWaitForSingleObject(&pAdapter->hMutexWrite, Executive, KernelMode, FALSE, NULL);
	prMdl = IoAllocateMdl(pNewBuffer, newLen, FALSE, FALSE, NULL);
	MmBuildMdlForNonPagedPool(prMdl);		
	mdlAddress = prMdl;
	if(pAdapter->bLoadingFW)// ||pAdapter->TxProcessing)
	{
		Async = SDIO_ASYNC;
	}
	if(Async == SDIO_ASYNC)
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DEBUG_TRACE, ("%s TxProcessing = %d\n", __FUNCTION__, pAdapter->TxProcessing ));
#if 1	
	if(Async)
	{
		
		status = SdioReadWriteBuffer_BlockMode_Async(pAdapter->WdfDevice, pAdapter->FunctionNumber,//
			mdlAddress, 
			WTDR1, 
			(ULONG)newLen,
			FIFOMODE, 
			WRITETODEVICE, 
			&bytesRead,
			pAdapter->BlockMode);
		
	}
	else
#endif		
	{
		status = SdioReadWriteBuffer(pAdapter->WdfDevice, pAdapter->FunctionNumber,
			mdlAddress, 
			WTDR1, 
			(ULONG)newLen,
			FIFOMODE, 
			WRITETODEVICE, 
			&bytesRead);
	}
//    KeReleaseMutex(&pAdapter->hMutexWrite, FALSE);
	
	//WdfRequestCompleteWithInformation(Request, status, bytesRead);
	//WdfRequestCompleteWithInformation(Request, status, Length);

	//MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, "Completed Request = 0x%p\n", Request);
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("IoWrite: status = %d (0x%x)   bytesWrite= %d (0x%x)\n", 
		 status, status, Length, Length));
	
	
	//pAdapter->firmwareinfo.TxPacketCount++;
	//pAdapter->firmwareinfo.fifo_free_bytes -= (USHORT)bytesRead;
	//KeReleaseMutex(&(pAdapter->firmwareinfo.hMutex), FALSE);

	if(prMdl)
	{
	    IoFreeMdl(prMdl);
		prMdl=NULL;
	}
	if(pNewBuffer)
	{
		ExFreePool(pNewBuffer);
		pNewBuffer=NULL;
	}
 
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("leave WriteTx()\n"));

    return;  
}

size_t GetPaddingSizeByMode(RTMP_ADAPTER *pAdapter, size_t originalLen )
{
	size_t alignsize=0;

	//originalLen +=4; //extra for SDIO header

	if(pAdapter->BlockMode == 1)
	{	//block mode, block size align
		alignsize = pAdapter->BlockLength;
	}
	else
	{//byte mode, four bytes align	
		alignsize = 4; 
	}       
	
	return NBytesAlign(originalLen, alignsize);
}
VOID EvtIoStopWrite(IN WDFQUEUE  Queue, IN WDFREQUEST  Request, IN ULONG  ActionFlags)
{   	
	NTSTATUS	status	= STATUS_SUCCESS;
	 
	UNREFERENCED_PARAMETER(Queue);
	
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("EvtIoStopWrite(), Queue = 0x%p, Request = 0x%p, ActionFlags = 0x%x\n", Queue, Request, ActionFlags));

	if (ActionFlags & WdfRequestStopActionSuspend )  //0x01
	{ 
		WdfRequestStopAcknowledge(Request, FALSE); 

	}
	else if(ActionFlags & WdfRequestStopActionPurge) //0x2
	{
		WdfRequestCancelSentRequest(Request);
	}
	
	if(ActionFlags & WdfRequestStopRequestCancelable)
	{ 
		status = WdfRequestUnmarkCancelable(Request);
		if( status == STATUS_CANCELLED ) 
		{
			return;
		}	
		
	}
 	
	WdfRequestComplete(Request, STATUS_CANCELLED);
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE,("EvtIoStopWrite(): Completed Request = 0x%p\n", Request));

}

VOID EvtRequestCancelWrite (IN WDFREQUEST  Request)
{
	PREQUEST_CONTEXT		pReqContext = NULL;
	
	pReqContext = GetRequestContext(Request);

	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE,("EvtRequestCancelWrite()\n "));

	WdfRequestComplete(Request, STATUS_CANCELLED);	

	if(pReqContext->pAdapter->PendingWriteRequest == Request)
	{
		pReqContext->pAdapter->PendingWriteRequest = NULL;
	}

	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE,("EvtRequestCancelWrite(): Completed Request = 0x%p\n", Request));
}
/*
VOID SetPendingWriteRequest (IN WDFREQUEST  Request)
{
	PREQUEST_CONTEXT		pReqContext = NULL;
    RTMP_ADAPTER               *pAdapter = NULL;
	
	pReqContext = GetRequestContext(Request);
    pAdapter = pReqContext->pAdapter;

	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, "SetPendingWriteRequest()\n ");
	

	if(pAdapter->PendingWriteRequest != NULL)
	{
       ASSERT(0);
	}    
	else
	{
		pAdapter->PendingWriteRequest = Request;
	}
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, "Current Pending Write Request = 0x%p\n", Request);
	
}
*/

NTSTATUS SDIO_TxData(RTMP_ADAPTER *pAdapter){
	NTSTATUS ntStatus = STATUS_SUCCESS;
	LARGE_INTEGER	TimeOut1Second;
	TXD_STRUC	*pTxd;
	UINT32 TxTimeoutCount = 0;
	UINT32 BreakCount = 0;
	pTxd = (TXD_STRUC *)pAdapter->TxDataBuffer;	
	TimeOut1Second.QuadPart = 0;
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DEBUG_TRACE, ("%s, TxProcessing = %d Tx Buffer Length = 0x%x\n", __FUNCTION__, pAdapter->TxProcessing, pAdapter->TxDataBufferLength));
		
	do
	{	
		//if(pAdapter->LoopBackTxStoped == TRUE && pAdapter->LoopBackRunning == FALSE)
			//return STATUS_SUCCESS;
		if(BreakCount>100)
		{
			MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DEBUG_WARN, ("[TxQQ] %s pAdapter->TxTheradRunning == TRUE\n", __FUNCTION__));
			return STATUS_INSUFFICIENT_RESOURCES;
		}
		BreakCount++;
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DEBUG_WARN, ("[TxQQ] %s Break count = %d\n", __FUNCTION__, BreakCount));
		RTMPusecDelay(200);
	}while(pAdapter->TxTheradRunning == TRUE);
	pAdapter->TxTheradRunning = TRUE;
	pAdapter->TxDataThread = NULL;
	
	ntStatus =  PsCreateSystemThread(&pAdapter->TxDataThread,
									(ACCESS_MASK) 0L,
									NULL,
									NULL,
									NULL,
									Tx_DataTherad,
									pAdapter);
	if (ntStatus != STATUS_SUCCESS)
	{
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DEBUG_ERROR,("%s, thread create fail\n",__FUNCTION__));
		pAdapter->TxTheradRunning = FALSE;
	}
	
	RTMPusecDelay(100);	
	//MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DEBUG_TRACE, ("Total Tx sent = 0x%x, TX sent succeed = 0x%x, TX sent timeout = 0x%x\n", Txcount, Txcount - TxTimeoutCount, TxTimeoutCount));
	return ntStatus;
}

void Tx_DataTherad(IN OUT PVOID Context)
{
	NTSTATUS ntStatus = STATUS_SUCCESS;
	RTMP_ADAPTER	*pAdapter = (RTMP_ADAPTER *)Context;
	UINT32 Txcount = 0;
	UINT32 TxTimeoutCount = 0;
	UINT32   Remained = pAdapter->TxRemained[TX_DATA_BUFEER_NUM];
	LARGE_INTEGER	TimeOut1Second;	
	TimeOut1Second.QuadPart = 0;
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DEBUG_TRACE, ("%s TxProcessing = %d\n", __FUNCTION__, pAdapter->TxProcessing ));
	if(Remained == 0)
	{
		Remained = 0xffffffff;
	}
	pAdapter->TxProcessing = TRUE;	
	do{
		//MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DEBUG_TRACE, ("%s TxProcessing = %d,Remained = 0x%x\n", __FUNCTION__, pAdapter->TxProcessing, Remained));
		if(pAdapter->bIsWaitFW == TRUE)
			continue;
		IOWriteTx(pAdapter, (PUCHAR)pAdapter->TxDataBuffer, pAdapter->TxDataBufferLength, SDIO_ASYNC);
		ntStatus = Event_Wait(pAdapter, &pAdapter->TxDataEvent, TimeOut1Second);
		
		if(ntStatus != STATUS_SUCCESS)
		{
			TxTimeoutCount++;
			MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DEBUG_WARN, ("%s TX sent timeout\n", __FUNCTION__));
		}
		Txcount++;
		RTMPusecDelay(50);			
	}while(Txcount < Remained && pAdapter->TxProcessing);
	pAdapter->TxProcessing = FALSE;
	pAdapter->TxDataThread = NULL;

	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_INFO, ("+ + + + Control Thread Terminated + + + + \n"));

	//DecrementIoCount(pAd);
	pAdapter->TxTheradRunning = FALSE;
	PsTerminateSystemThread(STATUS_SUCCESS);
}
