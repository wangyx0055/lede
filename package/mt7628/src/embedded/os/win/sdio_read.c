
#include "sdio_config.h"
//#include "sdio.h"
//#include "sdio_read.h"
//#include "bufpool.h"


#ifdef ALLOC_PRAGMA 
	#pragma alloc_text (PAGE, SDIOEvtIoRead) 
    	#pragma alloc_text (PAGE, ReadRx) 
	#pragma alloc_text (PAGE, ReadRxIdx) 
#endif
 

//
// read routine, to serve ReadFile()
//
VOID
SDIOEvtIoRead (
              WDFQUEUE      Queue,
              WDFREQUEST    Request,
              size_t         Length
              )
{

    WDFDEVICE        device;
    RTMP_ADAPTER  *pAdapter=NULL; 
    WDF_REQUEST_PARAMETERS  parameters;
    NTSTATUS         status=0;
    ULONG            bytesRead=0; 
    ULONG            BufferLength=0; 
    ULONG            RequestReadLength=0; 
	PUCHAR           pSysBuffer=NULL; 
	KIRQL irql=0;


	PAGED_CODE();

	//for test
	//WdfRequestCompleteWithInformation(Request, 0, Length);
	//return;

	//MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, "\n>>enter SDIOEvtIoRead()\n");
	
    device = WdfIoQueueGetDevice(Queue);
    pAdapter = SDIOpAdGetData(device);
 	
	//status = IoAcquireRemoveLock(&(pAdapter->io_remove_lock), NULL);
	if(status != STATUS_SUCCESS )
	{
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("IoAcquireRemoveLock failed in EvtIoRead, 0x%x\n", status));
		WdfRequestComplete(Request, STATUS_CANCELLED);
		return;
	}

	//debug code snippet
	//irql = KeGetCurrentIrql();
	//MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, "current irql = %d (0x%x)\n", (ULONG)irql, (ULONG)irql);


    WDF_REQUEST_PARAMETERS_INIT(&parameters);
    WdfRequestGetParameters(Request, &parameters);

	RequestReadLength = parameters.Parameters.Read.Length;
	//MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, "SDIOEvtIoRead: Request Read Length = %d (0x%x)\n", RequestReadLength, RequestReadLength);
	    
	status = WdfRequestRetrieveOutputBuffer(Request, 0, &pSysBuffer, &BufferLength);
    if (!NT_SUCCESS(status)) {
	    MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("WdfRequestRetrieveOutputBuffer fail, status = %d(0x%x)\n", status, status));
      	WdfRequestComplete(Request, status);
		//IoReleaseRemoveLock(&(pAdapter->io_remove_lock), NULL);
		return ;
    }
    
	if(BufferLength < RequestReadLength)
	{
		WdfRequestComplete(Request, STATUS_INSUFFICIENT_RESOURCES);
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("the System BufferLength(%d) less than the request read length(%d)\n", BufferLength, RequestReadLength));
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("Completed Request = 0x%p\n", Request));	
		//IoReleaseRemoveLock(&(pAdapter->io_remove_lock), NULL);
		return ;
	}

#if DBG
	//BufferPoolPrintf(&(pAdapter->bufferpool));
#endif

	//status = BufferPoolPullEventData(&(pAdapter->bufferpool), pSysBuffer, RequestReadLength, &bytesRead );
    if (!NT_SUCCESS(status)) {
      	WdfRequestComplete(Request, status);
		//IoReleaseRemoveLock(&(pAdapter->io_remove_lock), NULL);
        return ;
    }
 
	if(bytesRead) //for less log
	{
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("SDIOEvtIoRead: bytesRead: %d (0x%x)\n", bytesRead, bytesRead));		
	}

	WdfRequestCompleteWithInformation(Request, status, bytesRead);
	//IoReleaseRemoveLock(&(pAdapter->io_remove_lock), NULL);
    
	//MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, "leave SDIOEvtIoRead()\n");
	 
    return;
}

VOID ReadRxIdx(RTMP_ADAPTER *pAdapter, size_t RxPktLen, UINT32 Idx)
{
    WDFDEVICE  device;

	PMDL       prMdl=NULL; 
    NTSTATUS   status=0;
    ULONG      bytesRead=0;
	size_t     pktlen=0; 
	PUCHAR     pNewBuffer=NULL; 
	ULONG      alignsize=0;
	UINT32	  idx=0;
	FwCMDRspTxD_STRUC *pRxD;
	NTSTATUS ntStatus = STATUS_SUCCESS;
	

	//MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, "\n>>enter ReadRx()\n");

	PAGED_CODE();

#if DBG
	{///debug code snippet
		//KIRQL irql;
		//irql = KeGetCurrentIrql();
		//MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, "current irql = %d (0x%x)\n", (ULONG)irql, (ULONG)irql);
	}///debug code snippet
#endif

    device = pAdapter->WdfDevice; 
     
	///{ 
	//DelayToReadRx(pAdapter);
	///}
	//MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, "pktlen = %d\n" ,pktlen);
    pktlen = RxPktLen;
	//MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, "pktlen = %d\n" ,pktlen);
    //MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, "SDIOEvtIoRead: pktlen = %d (0x%x)\n", pktlen, pktlen);
	if(pktlen <= SDIO_HEADER_LEN || pktlen > SDIO_MAX_PACKET_LEN)
	{  
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("[warning] invalid RxPktLen:  %d (0x%x)\n", pktlen, pktlen));
	    	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("[warning] leave ReadRx()\n")); 
		return;
	}

	///  
	if(pAdapter->BlockMode == 1)
	{	//block mode, block size align
		alignsize = 512;
	}
	else
	{//byte mode, four bytes align	
		alignsize = BYTE_ALIGN; 
	}    
 	if(pktlen>alignsize)
 	{
		pktlen = NBytesAlign(pktlen, alignsize);
 	}
#if 1
	else
	{
		if(RxPktLen %4 != 0)
			pktlen = RxPktLen + 4 - RxPktLen %4;
	}
#endif
	//MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, "pktlen = %d\n" ,pktlen);
	if(pktlen <= SDIO_HEADER_LEN || pktlen > SDIO_MAX_PACKET_LEN)
	{	//error  
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("[error] : internal error, block size should not great than 0xFFFF: %d (0x%x)\n", alignsize, alignsize));
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("[error] leave ReadRx()\n"));
		return;
	}
	//////////////

	//allocate a buffer for read data
	//here should use non paged pool
	pNewBuffer = (PUCHAR)ExAllocatePoolWithTag(NonPagedPool, pktlen, 'daeR');
	if(!pNewBuffer)
	{ 
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("[error] allocate poolwithtag fail, length = %d (0x%x)\n", pktlen, pktlen));
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("[error] leave ReadRx()\n"));
		return;
	}
	RtlZeroMemory(pNewBuffer, pktlen);

	prMdl = IoAllocateMdl(pNewBuffer, pktlen, FALSE, FALSE, NULL);
	MmBuildMdlForNonPagedPool(prMdl);
	//KeWaitForSingleObject(&pAdapter->hMutexRead, Executive, KernelMode, FALSE, NULL);
	//Idx = RX0 or RX1
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DEBUG_TRACE, ("%s, RX@@ Read Len = %d, padding Len = %d\n", __FUNCTION__, RxPktLen, pktlen));
	status = SdioReadWriteBuffer(device, pAdapter->FunctionFocus, prMdl, Idx, 
		                        (ULONG)pktlen, FIFOMODE, READFROMDEVICE, &bytesRead);
	//KeReleaseMutex(&pAdapter->hMutexRead, FALSE);

	do{
		//ULONG  size=0;
		ULONG  length=0;
		size_t     datalen=0; 
		//SDIOHeader sdioheader;   
		PUCHAR pEventBuffer=NULL;  //buffer for put event data
		PUCHAR FirstByte=NULL;  //buffer for put event data
#if 0		
		CheckSecurityResult((PULONG)pNewBuffer,pAdapter);
	
		pRxD = (FwCMDRspTxD_STRUC *)pNewBuffer;
		if(pRxD->FwEventTxD.u2PacketType == 0xE000 && pktlen>=sizeof(FwCMDRspTxD_STRUC)){
			MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("Fw Response event\n"));
			MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("byte count = 0x%x\n", pRxD->FwEventTxD.u2RxByteCount));
			MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("Pkt Type = 0x%x\n", pRxD->FwEventTxD.u2PacketType));
			MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("EID = 0x%x\n", pRxD->FwEventTxD.ucEID));
			MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("seq number = 0x%x\n", pRxD->FwEventTxD.ucSeqNum));
			MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("FWRspStatus = 0x%x\n", pRxD->ucStatus));
			if(pRxD->FwEventTxD.ucSeqNum != SEQ_CMD_RAM_ACCESS_REG_WRITE && pRxD->FwEventTxD.ucSeqNum != SEQ_CMD_RAM_ACCESS_REG_READ && pRxD->FwEventTxD.ucSeqNum != SEQ_CMD_RAM_ACCESS_EFUSE_WRITE && pRxD->FwEventTxD.ucSeqNum != SEQ_CMD_RAM_ACCESS_EFUSE_READ && pRxD->FwEventTxD.ucSeqNum != SEQ_CMD_FW_STARTTORUN)
			{
				pAdapter->PacketCMDSeqMCU = (UCHAR)pRxD->FwEventTxD.ucSeqNum;			
				MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("PacketCMDSeqMCU = 0x%x\n", pAdapter->PacketCMDSeqMCU));
				pAdapter->FWRspStatus = (UCHAR)pRxD->ucStatus;
				if(pktlen >PKTCMD_EVENT_BUFFER)
					pAdapter->FWRspContentLength = PKTCMD_EVENT_BUFFER;
				else
					pAdapter->FWRspContentLength = pktlen;
				RtlCopyMemory(pAdapter->FWRspContent, pNewBuffer, pAdapter->FWRspContentLength);
			}
			MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("PacketCMDSeqMCU = 0x%x\n", pAdapter->PacketCMDSeqMCU));
			switch(pRxD->FwEventTxD.ucEID)
			{
				case INIT_EVENT_CMD_RESULT:
				{					
					MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DEBUG_TRACE,("><><INIT_EVENT_CMD_RESULT\n"));
					ntStatus = FWInitCmdResultEvent(pAdapter, pNewBuffer, pktlen);
					break;
				}
				case INIT_EVENT_ACCESS_REG:
				{
					FW_EVENT_ACCESS_REG *pEventRegData;	
					MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DEBUG_TRACE,("><><ROM INIT_EVENT_ACCESS_REG\n"));
					pEventRegData = (FW_EVENT_ACCESS_REG *)pNewBuffer;
					pAdapter->RegValue= pEventRegData->u4Data;
					pAdapter->RegAddress= pEventRegData->u4Address;
					KeSetEvent(&pAdapter->FWCREvent, 0, FALSE);				
					break;
				}
				case INIT_EVENT_PATCH_SEMA_CTRL:
				{	
					MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DEBUG_TRACE,("><><EVENT_PATCH_SEMA_CTRL\n"));
					MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DEBUG_TRACE,("><><ucStatus = 0x%x, Seq = 0x%x\n", pRxD->ucStatus, pRxD->FwEventTxD.ucSeqNum));
					pAdapter->RomPatchSemStatus = (UCHAR)pRxD->ucStatus;				
					KeSetEvent(&pAdapter->WaitFWEvent, 0, FALSE);
					break;
				}
				case EVENT_RESTART_DONWLOAD:
				{
					MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DEBUG_TRACE,("><><EVENT_RESTART_DONWLOAD\n"));
					MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DEBUG_TRACE,("ucStatus = 0x%x, Seq = 0x%x\n", pRxD->ucStatus, pRxD->FwEventTxD.ucSeqNum));
					pAdapter->RomPatchSemStatus = (UCHAR)pRxD->ucStatus;				
					KeSetEvent(&pAdapter->WaitFWEvent, 0, FALSE);					
					break;
				}			
				case EVENT_EXT_CMD:
				{					
					MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DEBUG_TRACE,("><><EVENT_EXT_CMD\n"));					
					ntStatus = FWExtCmdEvent(pAdapter, pNewBuffer, pktlen);
					
					break;
				}
				default:
					MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DEBUG_TRACE,("unhandled EID\n"));
			}
		}
#else
#if 1		
		RxHandler(pAdapter, pNewBuffer, pktlen, Idx);
#else		
		if(IsFwRspPkt(pAdapter, pNewBuffer, pktlen))
		{
			/*MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DEBUG_TRACE, ("===pNewBuffer===\n"));
			for(idx=0;idx<pktlen;idx++){
				MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DEBUG_TRACE, ("0x%x\n",pNewBuffer[idx]));
			}*/
			FWCmdEventHandler(pAdapter, pNewBuffer, pktlen);
		}

		else
		{
			//MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DEBUG_TRACE,("%s Not FW response\n", __FUNCTION__));
			RxPacket(pNewBuffer, pAdapter, pktlen);
		}
#endif		
#endif		
		//MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, "pNewBuffer\n");
		//for(idx=0;idx<pktlen;idx++){
		//	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, "0x%x\n",pNewBuffer[idx]);
		//}

		///	 
		//debug code
		//MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, "<test> dumpbinary()\n");
		//DumpBinary(pEventBuffer, datalen);
		//MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, "</test> dumpbinary()\n");
	}while(0);
	//
 
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

    MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("leave ReadRx()\n"));
	
	return;
}
#if 0
VOID DelayToReadRx(RTMP_ADAPTER *pAdapter)
{	
	ULONG   totalbuffersize=0;
    ULONG   eventdatacount=0;
    ULONG   ulMicroSecond=0;  //ms

	totalbuffersize = BufferPoolGetTotalBufferSize(&(pAdapter->bufferpool));
	//eventdatacount = BufferPoolGetEventDataCount(&(pAdapter->bufferpool));

	if(totalbuffersize < 20 * 10000)
	{   //buffer pool is not full
		return;
	}
	if(totalbuffersize > 20 * 100000)
	{   //buffer pool is full, will discard the data
		return;
	}

	//buffer pool is not full, but need to put into data slowly
	//this is a strategy to protect the data eat up the system memory
    ulMicroSecond = 2 * (totalbuffersize/100000 + 1);
	
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, "DelayToReadRx(): sleep %ld ms\n", ulMicroSecond);
	WaitMicroSecond1(1000 * ulMicroSecond);

	return;
}

VOID ReadRx(RTMP_ADAPTER *pAdpater, size_t RxPktLen)
{
    WDFDEVICE  device;

	PMDL       prMdl=NULL; 
    NTSTATUS   status=0;
    ULONG      bytesRead=0;
	size_t     pktlen=0; 
	PUCHAR     pNewBuffer=NULL; 
	ULONG      alignsize=0;
	

	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, "\n>>enter ReadRx()\n");

	PAGED_CODE();

#if DBG
	{///debug code snippet
		//KIRQL irql;
		//irql = KeGetCurrentIrql();
		//MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, "current irql = %d (0x%x)\n", (ULONG)irql, (ULONG)irql);
	}///debug code snippet
#endif

    device = pAdpater->WdfDevice; 
     
	///{ 
	//DelayToReadRx(pAdpater);
	///}

    pktlen = RxPktLen;
    //MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, "SDIOEvtIoRead: pktlen = %d (0x%x)\n", pktlen, pktlen);
	if(pktlen <= SDIO_HEADER_LEN || pktlen > SDIO_MAX_PACKET_LEN)
	{  
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, "[warning] invalid RxPktLen:  %d (0x%x)\n", pktlen, pktlen);
	    MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, "[warning] leave ReadRx()\n"); 
		return;
	}

	///  
	if(pAdpater->BlockMode == 1)
	{	//block mode, block size align
		alignsize = 512;
	}
	else
	{//byte mode, four bytes align	
		alignsize = BYTE_ALIGN; 
	}    
 
	pktlen = NBytesAlign(pktlen, alignsize);
	if(pktlen <= SDIO_HEADER_LEN || pktlen > SDIO_MAX_PACKET_LEN)
	{	//error  
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, "[error] : internal error, block size should not great than 0xFFFF: %d (0x%x)\n", alignsize, alignsize);
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, "[error] leave ReadRx()\n");
		return;
	}
	//////////////

	//allocate a buffer for read data
	//here should use non paged pool
	pNewBuffer = (PUCHAR)ExAllocatePoolWithTag(NonPagedPool, pktlen, 'daeR');
	if(!pNewBuffer)
	{ 
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, "[error] allocate poolwithtag fail, length = %d (0x%x)\n", pktlen, pktlen);
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, "[error] leave ReadRx()\n");
		return;
	}
	RtlZeroMemory(pNewBuffer, pktlen);

	prMdl = IoAllocateMdl(pNewBuffer, pktlen, FALSE, FALSE, NULL);
	MmBuildMdlForNonPagedPool(prMdl);
	//KeWaitForSingleObject(&pAdpater->hMutexRead, Executive, KernelMode, FALSE, NULL);
	status = SdioReadWriteBuffer(device, pAdpater->FunctionFocus, prMdl, WRDR0, 
		                        (ULONG)pktlen, FIFOMODE, READFROMDEVICE, &bytesRead);
	//KeReleaseMutex(&pAdpater->hMutexRead, FALSE);

	//deal with SDIO header and put data to data pool
	do{
		//ULONG  size=0;
		ULONG  length=0;
		size_t     datalen=0; 
		SDIOHeader sdioheader;   
		PUCHAR pEventBuffer=NULL;  //buffer for put event data
 
		if(bytesRead <= SDIO_HEADER_LEN)
		{	//error
			MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, "[error] : internal error, packet data too small: %d (0x%x)\n", bytesRead, bytesRead);
			MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, "[error] leave ReadRx()\n");
			break;
		}

		RtlCopyMemory(&sdioheader, pNewBuffer, SDIO_HEADER_LEN);
		if(sdioheader.packetlen > bytesRead )
		{
			MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, "[error] : sdioheader.packetlen > bytesRead:  pktlen: %d (0x%x), bytesRead: %d (0x%x)\n", sdioheader.packetlen, sdioheader.packetlen, bytesRead, bytesRead);
			MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, "[error] leave ReadRx()\n");
			break;
		}
		
		///put data to data pool
		//allocate a buffer for event data
		datalen = sdioheader.packetlen - SDIO_HEADER_LEN;
		pEventBuffer = (PUCHAR)ExAllocatePoolWithTag(NonPagedPool, datalen, 'daeR');
		if(!pEventBuffer)
		{ 
			MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, "[error] : allocate event poolwithtag fail, length = %d (0x%x)\n", datalen, datalen);
			MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, "[error] leave ReadRx()\n");			
			break;
		}
		RtlZeroMemory(pEventBuffer, datalen);				
		RtlCopyMemory(pEventBuffer, pNewBuffer+SDIO_HEADER_LEN, datalen);		 
		status = BufferPoolPushEventData(&(pAdpater->bufferpool), pEventBuffer, datalen);
		if (!NT_SUCCESS(status)) 
		{  
			ExFreePool(pEventBuffer);
			pEventBuffer=NULL;
			MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, "BufferPoolPushEventData failed with status %d(0x%x)\n", status, status);   	
			break;
		}
		///	 
		//debug code
		//MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, "<test> dumpbinary()\n");
		//DumpBinary(pEventBuffer, datalen);
		//MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, "</test> dumpbinary()\n");
	}while(0);
	//
 
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

    MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, "leave ReadRx()\n");
	
	return;
}
 #endif
/**
* @Routine Description:
*		informs the driver that an I/O request was canceled while it was in an I/O queue.
*
* @return VOID
*/
VOID EvtIoCanceledOnQueue(IN WDFQUEUE  Queue,IN WDFREQUEST  Request)
{
	NTSTATUS  status  = STATUS_SUCCESS;

	UNREFERENCED_PARAMETER(Queue);
	 
	status = WdfRequestGetStatus(Request);	
	
 	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("EvtIoCanceledOnQueue(): request status = %d(0x%x)", status, status));

	WdfRequestCompleteWithInformation(Request, status, 0);

	return;
}
 

VOID DftEvtIoStop(IN WDFQUEUE  Queue, IN WDFREQUEST  Request, IN ULONG  ActionFlags)
{   
	

	NTSTATUS	status	= STATUS_SUCCESS;
	 
	UNREFERENCED_PARAMETER(Queue);
	 
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("EvtIoStop for Read(), Queue = 0x%p, Request = 0x%p, ActionFlags = 0x%x\n", Queue, Request, ActionFlags));

	if(ActionFlags & WdfRequestStopRequestCancelable)
	{ 
		status = WdfRequestUnmarkCancelable(Request);
		if( status == STATUS_CANCELLED ) 
		{
			return;
		}	
		
	}
 	
	WdfRequestComplete(Request, STATUS_CANCELLED);
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE,("EvtIoStop for Read(): Completed Request = 0x%p\n", Request));


}

