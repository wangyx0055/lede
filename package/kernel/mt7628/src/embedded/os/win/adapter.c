#ifdef RTMP_SDIO_SUPPORT
#include "sdio_config.h"
#endif

#ifdef RTMP_USB_SUPPORT
#include "config.h"
#endif

NTSTATUS InitAdapterCommon(RTMP_ADAPTER *pAd)
{
	NTSTATUS ntStatus = STATUS_SUCCESS;
	UINT32	Idx = 0;

	//for Efuse using
	pAd->EfuseMode = MODE_BUFFER;// Buffer:0, Efuse:1, EEPROM:2
	RtlZeroMemory(&pAd->EfuseContent, EFUSESIZE);
	//FW	


	RtlZeroMemory(&pAd->ScatterBuf, SCATTER_SIZE);
	RtlZeroMemory(&pAd->OtherCounters, sizeof(OTHER_STATISTICS));
	pAd->SecurityFail = 0;
	pAd->IsTxSetFrequency = FALSE;// check if is if first time to run TxSetFrequencyOffset
	pAd->MACVersion = 0;	
	pAd->ChipID = 0;
	pAd->TransmittedCount = 0;

	//FW download	
	RtlZeroMemory(&pAd->FWImage, MAXFIRMWARESIZE);
	pAd->FWSize = 0;//From UI
	pAd->IsFWImageInUse = FALSE;
	pAd->IsUISetFW = FALSE;
	pAd->LoadingFWCount = 0;
	pAd->bLoadingFW = FALSE;
	//FW
	pAd->bIsWaitFW = FALSE;
	pAd->RomPatchSemStatus = 0;	
	pAd->FwCmdSeqNum = 0xffff;
	pAd->RegValue = 0;//FW cmd use
	pAd->RegAddress = 0;//FW cmd use
	pAd->RegValueRead = 0;//FW cmd use
	pAd->FWRspStatus = 0;
	pAd->PacketCMDSeqMCU = 0;
	RtlZeroMemory(&pAd->FWRspContent, PKTCMD_EVENT_BUFFER);
	pAd->FWRspContentLength = 0;
	pAd->FWMode = FWROM;
	pAd->FWon = FALSE;	
	RtlZeroMemory(&pAd->TxCmdBuffer, BUFFER_SIZE);
	pAd->TxCmdBufferLength = 0;
/*=================Tx Data==================================*/
	RtlZeroMemory(&pAd->TxDataBuffer, BUFFER_SIZE);
	pAd->TxDataBufferLength = 0;
	//pAd->bMinLen = 0;
	//pAd->bCurrlength = 0;
	for(Idx=0; Idx<MAX_TX_BUFEER_NUM; Idx++)
	{	
		pAd->TxRemained[Idx] = 0;
	}
	pAd->TxProcessing = FALSE;
	pAd->TxTheradRunning = FALSE;
	KeInitializeSpinLock(&pAd->TxLock);
	
/*=================Tx Data==================================*/
//BF	
	RtlZeroMemory(&pAd->prof, sizeof(PFMU_PROFILE));
	pAd->iTxBf = FALSE;
	pAd->eTxBf = FALSE;	
//loop back
	RtlZeroMemory(&pAd->LoopBackResult, sizeof(LOOPBACK_RESULT));
	pAd->LoopBackRunning = FALSE;
	pAd->LoopBackWaitRx = FALSE;
	KeInitializeSpinLock(&pAd->LoopBackLock);
	
	RtlZeroMemory(&pAd->RecordRssi0, sizeof(pAd->RecordRssi0));
	RtlZeroMemory(&pAd->RecordRssi1, sizeof(pAd->RecordRssi1));
	pAd->RssiCount = 0;
	pAd->RssiLimit = 100;
	pAd->IsBfStored = FALSE;
	InitAdapterInterface(pAd);

	return ntStatus;
}
NTSTATUS InitAdapterInterface(RTMP_ADAPTER *pAd)
{
	NTSTATUS ntStatus = STATUS_SUCCESS;
#ifdef RTMP_USB_SUPPORT
	ntStatus = USBInitAdapter(pAd);
#endif
#ifdef RTMP_SDIO_SUPPORT
	ntStatus = SDIOInitAdapter(pAd);
#endif
#ifdef RTMP_PCI_SUPPORT
//todo
#endif
	
	
	return ntStatus;
}
NTSTATUS InitAdapterEvent(RTMP_ADAPTER *pAd)
{
	UINT8 idx = 0;
	NTSTATUS ntStatus = STATUS_SUCCESS;
	//event for load fw
	KeInitializeEvent(&pAd->LoadFWEvent,
                      SynchronizationEvent,     // auto-clearing event
                      FALSE);                   // event initially non-signalled

	//event for fw bulkout
	KeInitializeEvent(&pAd->WaitFWEvent,
                      SynchronizationEvent,     // auto-clearing event
                      FALSE);                   // event initially non-signalled
	
	//event for Rom Patch bulkout
	KeInitializeEvent(&pAd->RomPatchEvent,
                      SynchronizationEvent,     // auto-clearing event
                      FALSE);                   // event initially non-signalled	

	//event for CR cmd access
	KeInitializeEvent(&pAd->FWCREvent,
                      SynchronizationEvent,     // auto-clearing event
                      FALSE);                   // event initially non-signalled

	//event for Tx Complete
	KeInitializeEvent(&pAd->TxDataEvent,
                      SynchronizationEvent,     // auto-clearing event
                      FALSE);                   // event initially non-signalled
#ifdef RTMP_USB_SUPPORT
	for(idx=0; idx<MAX_TX_BULK_PIPE_NUM; idx++)
	{
		KeInitializeEvent(&pAd->LoopBackTxEvent[idx],
                      SynchronizationEvent,     // auto-clearing event
                      FALSE);                   // event initially non-signalled
	}	
#endif
#ifdef RTMP_SDIO_SUPPORT	
	KeInitializeEvent(&pAd->LoopBackSDIOTxEvent,
                      SynchronizationEvent,     // auto-clearing event
                      FALSE);                   // event initially non-signalled
#endif
	KeInitializeEvent(&pAd->LoopBackEvent,
                      SynchronizationEvent,     // auto-clearing event
                      FALSE);                   // event initially non-signalled

	return ntStatus;
}

NTSTATUS GetChipID(RTMP_ADAPTER *pAd)
{
	NTSTATUS ntStatus = STATUS_SUCCESS;
#ifdef RTMP_USB_SUPPORT
	ntStatus = USBGetChipID(pAd);
#endif
#ifdef RTMP_SDIO_SUPPORT
	ntStatus = SDIOGetChipID(pAd);
#endif
#ifdef RTMP_PCI_SUPPORT
//todo
#endif

	return ntStatus;
}

NTSTATUS ReadReg(RTMP_ADAPTER *pAd,UINT32 Offset, UINT32 *pValue, UINT32 Length)
{
	NTSTATUS ntStatus = STATUS_SUCCESS;
#ifdef RTMP_USB_SUPPORT
	if(Length > 4)// multiread
	{
		ntStatus = RTMP_IO_MULTIREAD(pAd, Offset, pValue, Length);
	}
	else
	{
		ntStatus = RTMP_IO_READ32(pAd, Offset, pValue);
	}
#endif
#ifdef RTMP_SDIO_SUPPORT
	//to remove after UI not access CR before FW download
	if(pAd->FWMode == FWROM)
		*pValue = 0x7636;
	else if(Offset >= 0xb0000000 && Offset <= 0xb0000200)
	{
		ntStatus = RTMP_SDIO_READ32(pAd, Offset & 0xfff, pValue);
	}
	else
	{
		RTMP_IO_READ32(pAd, Offset, pValue);
	}
#endif
#ifdef RTMP_PCI_SUPPORT
	ntStatus = RTMP_IO_READ32(pAd, Offset, pValue);
#endif

	return ntStatus;
}

NTSTATUS WriteReg(RTMP_ADAPTER *pAd,UINT32 Offset, UINT32 Value)
{
	NTSTATUS ntStatus = STATUS_SUCCESS;
#ifdef RTMP_USB_SUPPORT
	ntStatus = RTMP_IO_WRITE32(pAd, Offset, Value);
#endif
#ifdef RTMP_SDIO_SUPPORT
	//to remove after UI not access CR before FW download
	if(pAd->FWMode == FWROM)
	{
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DEBUG_TRACE,("%s: do nothing\n",__FUNCTION__));
	}
	else if(Offset >= 0xb0000000 && Offset <= 0xb0000200)
	{
		ntStatus = RTMP_SDIO_WRITE32(pAd, Offset & 0xfff, Value);
	}
	else
	{
		RTMP_IO_WRITE32(pAd, Offset, Value);
	}
#endif
#ifdef RTMP_PCI_SUPPORT
	ntStatus = RTMP_IO_WRITE32(pAd, Offset, Value);
#endif

	return ntStatus;
}

