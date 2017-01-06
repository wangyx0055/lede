#ifdef RTMP_USB_SUPPORT
#include "config.h"
#include "FW7603.h"
#include "fw7636.h"
#include "fw7636rompatch.h"
#endif

#ifdef RTMP_SDIO_SUPPORT
#include "sdio_config.h"
#include "fw7636.h"
#include "fw7636rompatch.h"

NTSTATUS	Cmd_Access_Reg(
	IN	RTMP_ADAPTER	*pAdapter,
	IN	UINT32			Address,
	IN	UINT32			Value,
	IN	BOOLEAN			IsWrite){
	NTSTATUS	ntStatus = STATUS_SUCCESS;
	LARGE_INTEGER       TimeOut1Second;
	ROM_CMD_ACCESS_REG	CmdAccCR;

	TimeOut1Second.QuadPart = -(3000* 10000);	
	RtlZeroMemory(&CmdAccCR, sizeof(ROM_CMD_ACCESS_REG));
	CmdAccCR.FwTxD.CID = CMD_ROM_ACCESS_REG;
	CmdAccCR.FwTxD.Length= sizeof(ROM_CMD_ACCESS_REG);
	CmdAccCR.FwTxD.PQ_ID= 0x8000;
	CmdAccCR.FwTxD.PktTypeID= 0xA0;
	CmdAccCR.FwTxD.SeqNum= SEQ_CMD_ROM_ACCESS_REG;
	CmdAccCR.SetQuery= IsWrite;	//true: write, false: read
	CmdAccCR.Address= Address;
	if(IsWrite == WRITE)
		CmdAccCR.Data= Value;
	pAdapter->FwCmdSeqNum= SEQ_CMD_ROM_ACCESS_REG;

	ntStatus = FWTxCmd(pAdapter, &CmdAccCR, sizeof(ROM_CMD_ACCESS_REG), &pAdapter->FWCREvent);
	if(ntStatus != STATUS_SUCCESS)
	{		
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DEBUG_ERROR, ("!!!!! CR access failed !!!!!\n"));
	}	
	return ntStatus;
}

NTSTATUS	Cmd_Ram_Access_Reg(
	IN	RTMP_ADAPTER	*pAdapter,
	IN	UINT32			Address,
	IN	UINT32			Value,
	IN	BOOLEAN			IsWrite){
	NTSTATUS	ntStatus = STATUS_SUCCESS;
	LARGE_INTEGER       TimeOut1Second;
	RAM_CMD_ACCESS_REG	CmdAccCR;
	
	TimeOut1Second.QuadPart = -(3000* 10000);	
	RtlZeroMemory(&CmdAccCR, sizeof(RAM_CMD_ACCESS_REG));
	CmdAccCR.FwTxD.CID = CMD_EXT_CMD;
	CmdAccCR.FwTxD.Length= sizeof(RAM_CMD_ACCESS_REG);
	CmdAccCR.FwTxD.PQ_ID= 0x8000;
	CmdAccCR.FwTxD.PktTypeID= 0xA0;
	CmdAccCR.FwTxD.SeqNum= SEQ_CMD_RAM_ACCESS_REG_READ;
	CmdAccCR.FwTxD.SetQuery= IsWrite;	//true: write, false: read
	CmdAccCR.FwTxD.ExtenCID = EXT_CMD_MULTIPLE_REG_ACCESS;
	CmdAccCR.FwTxD.ExtCmdOption= 1;
	CmdAccCR.Type = MAC_CR;
	CmdAccCR.Address= Address;
	if(IsWrite == WRITE)
	{
		CmdAccCR.Data= Value;
		CmdAccCR.FwTxD.SeqNum = SEQ_CMD_RAM_ACCESS_REG_WRITE;
	}
	
	pAdapter->FwCmdSeqNum= CmdAccCR.FwTxD.SeqNum;
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DEBUG_TRACE, ("Address = 0x%x\n", CmdAccCR.Address));
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DEBUG_TRACE, ("SetQuery = 0x%x\n", CmdAccCR.FwTxD.SetQuery));
#if 0	
	
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DEBUG_TRACE, ("CID = 0x%x\n", CmdAccCR.FwTxD.CID));
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DEBUG_TRACE, ("Length = 0x%x\n", CmdAccCR.FwTxD.Length));
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DEBUG_TRACE, ("PQ_ID = 0x%x\n", CmdAccCR.FwTxD.PQ_ID));
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DEBUG_TRACE, ("PktTypeID = 0x%x\n", CmdAccCR.FwTxD.PktTypeID));
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DEBUG_TRACE, ("SeqNum = 0x%x\n", CmdAccCR.FwTxD.SeqNum));
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DEBUG_TRACE, ("ExtCmdOption = 0x%x\n", CmdAccCR.FwTxD.ExtCmdOption));
#endif	
	ntStatus = FWTxCmd(pAdapter, &CmdAccCR, sizeof(RAM_CMD_ACCESS_REG) , &pAdapter->FWCREvent);


	//SEQ_CMD_RAM_ACCESS_REG;CMD_RAM_ACCESS_REG
	if(ntStatus != STATUS_SUCCESS)
	{		
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DEBUG_ERROR, ("!!!!! CR access failed !!!!!\n"));
	}		
	return ntStatus;
}
NTSTATUS	Cmd_SDIO_Write_Reg(
	IN	RTMP_ADAPTER	*pAdapter,
	IN	UINT32			Address,
	IN	UINT32			Value,
	IN	BOOLEAN			mapping)
{
	NTSTATUS	ntStatus = STATUS_SUCCESS;
	if(pAdapter->FWMode == FWRAM)
	{
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DEBUG_TRACE, ("Access_Reg   Long format\n"));
		if(mapping)
			Address = mt_physical_addr_map(Address);
		ntStatus = Cmd_Ram_Access_Reg(pAdapter, Address, Value,WRITE);
	}
	else//FWROM
	{
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DEBUG_TRACE, ("Access_Reg   Short format\n"));
		ntStatus = Cmd_Access_Reg(pAdapter, Address, Value,WRITE);
	}	
	return ntStatus;
}

NTSTATUS	Cmd_SDIO_Read_Reg(
	IN	RTMP_ADAPTER	*pAdapter,
	IN	UINT32			Address,
	IN	UINT32			*pValue,
	IN	BOOLEAN			mapping)
{
	NTSTATUS	ntStatus = STATUS_SUCCESS;
	if(pAdapter->FWMode == FWRAM)
	{
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DEBUG_TRACE, ("Access_Reg   Long format\n"));
		if(mapping)
			Address = mt_physical_addr_map(Address);
		ntStatus = Cmd_Ram_Access_Reg(pAdapter, Address, *pValue,READ);
	}
	else//FWROM
	{
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DEBUG_TRACE, ("Access_Reg   Short format\n"));
		ntStatus = Cmd_Access_Reg(pAdapter, Address, *pValue,READ);
	}
	if(ntStatus == STATUS_SUCCESS)
	{
		RtlCopyMemory(pValue, &pAdapter->RegValueRead, sizeof(pAdapter->RegValueRead));
	}
	return ntStatus;
}

 NTSTATUS	Cmd_Read_Efuse(
	IN	RTMP_ADAPTER	*pAdapter,
	IN	UINT32			Address,
	IN	UINT32			*pValue)
 {
	NTSTATUS	ntStatus = STATUS_SUCCESS;
	RAM_CMD_EFUSE_ACCESS cmd;
	RtlZeroMemory(&cmd, sizeof(RAM_CMD_EFUSE_ACCESS));
	cmd.FwTxD.CID = CMD_EXT_CMD;
	cmd.FwTxD.Length= sizeof(RAM_CMD_EFUSE_ACCESS);
	cmd.FwTxD.PQ_ID= 0x8000;
	cmd.FwTxD.PktTypeID= 0xA0;
	cmd.FwTxD.SeqNum= SEQ_CMD_RAM_ACCESS_REG_READ;
	cmd.FwTxD.SetQuery= READ;	//true: write, false: read
	cmd.FwTxD.ExtenCID = SEQ_CMD_RAM_ACCESS_EFUSE_READ;
	cmd.FwTxD.ExtCmdOption= 1;
//	cmd.Type = MAC_CR;
	cmd.Address = Address;
	//EfuseAcc.Data= Address;
	//RtlCopyMemory(pValue, &pAdapter->EfuseRead, sizeof(pAdapter->EfuseRead));
	return ntStatus;
 }

NTSTATUS	Cmd_Write_Efuse(
	IN	RTMP_ADAPTER	*pAdapter,
	IN	UINT32			Address,
	IN	UINT32			Value)
{
	NTSTATUS	ntStatus = STATUS_SUCCESS;
	RAM_CMD_EFUSE_ACCESS EfuseAcc;
	RtlZeroMemory(&EfuseAcc, sizeof(RAM_CMD_EFUSE_ACCESS));
	
	
	return ntStatus;
}
#endif  //RTMP_SDIO_SUPPORT

//for FW download, fw run in Rom 
 NTSTATUS	FWInitCmdResultEvent(
	IN	RTMP_ADAPTER	*pAdapter,
	IN	PVOID			pBuffer,
	IN	UINT32			Length){
	NTSTATUS	ntStatus = STATUS_SUCCESS;
	FwCMDRspTxD_STRUC *pEventRxD;
	pEventRxD = (FwCMDRspTxD_STRUC *)pBuffer;
	if(pEventRxD->ucStatus != INIT_EVENT_CMD_RESULT_SUCCESS){
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DEBUG_ERROR, ("!!!!! INIT_EVENT_CMD_RESULT failed !!!!! seq = 0x%x, ucStatus = 0x%x\n", pEventRxD->FwEventTxD.ucSeqNum, pEventRxD->ucStatus));
		ntStatus = STATUS_UNSUCCESSFUL;
	}
	switch(pEventRxD->FwEventTxD.ucSeqNum)
	{
		case SEQ_CMD_ROM_PATCH_STARTREQ:
		{
			MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DEBUG_TRACE, (" #####SEQ_CMD_ROM_PATCH_STARTREQ\n"));
			KeSetEvent(&pAdapter->WaitFWEvent, 0, FALSE);
			break;
		}
		case SEQ_CMD_ROM_PATCH_FINISH:
		{
			MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DEBUG_TRACE, ("#####SEQ_CMD_ROM_PATCH_FINISH\n"));
			KeSetEvent(&pAdapter->WaitFWEvent, 0, FALSE);
			break;
		}
		case SEQ_CMD_FW_STARTREQ:
		{
			MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DEBUG_TRACE, (" #####SEQ_CMD_FW_STARTREQ\n"));
			KeSetEvent(&pAdapter->WaitFWEvent, 0, FALSE);
			break;
		}
		case SEQ_CMD_FW_STARTTORUN:
		{
			MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DEBUG_TRACE, (" #####SEQ_CMD_FW_STARTTORUN\n"));
			KeSetEvent(&pAdapter->WaitFWEvent, 0, FALSE);
			break;
		}
		case SEQ_CMD_LOOPBACK_TEST:
		{
			MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE,("@#@# #######SEQ_CMD_LOOPBACK_TEST  response########\n"));
			KeSetEvent(&pAdapter->WaitFWEvent, 0, FALSE);
		}
			break;
		default:
			MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DEBUG_TRACE, ("unhandled Seqnum, 0x%x\n", pEventRxD->FwEventTxD.ucSeqNum));
	}

	return ntStatus;
 }

NTSTATUS	FWCmdEventHandler(
	IN	RTMP_ADAPTER	*pAdapter,
	IN	PVOID			pBuffer,
	IN	UINT32			PktLen){
	NTSTATUS	ntStatus = STATUS_SUCCESS;
	FwCMDRspTxD_STRUC *pEvent;
	pEvent = (FwCMDRspTxD_STRUC *)pBuffer;

	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("Fw Response event\n"));
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("byte count = 0x%x\n", pEvent->FwEventTxD.u2RxByteCount));
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("Pkt Type = 0x%x\n", pEvent->FwEventTxD.u2PacketType));
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("EID = 0x%x\n", pEvent->FwEventTxD.ucEID));
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("seq number = 0x%x\n", pEvent->FwEventTxD.ucSeqNum));
#ifdef RTMP_SDIO_SUPPORT	// UI seq will mismatch with driver seq in SDIO because of the CR access with fw cmd
	if(pEvent->FwEventTxD.ucSeqNum != SEQ_CMD_RAM_ACCESS_REG_WRITE && pEvent->FwEventTxD.ucSeqNum != SEQ_CMD_RAM_ACCESS_REG_READ && pEvent->FwEventTxD.ucSeqNum != SEQ_CMD_RAM_ACCESS_EFUSE_WRITE && pEvent->FwEventTxD.ucSeqNum != SEQ_CMD_RAM_ACCESS_EFUSE_READ && pEvent->FwEventTxD.ucSeqNum != SEQ_CMD_FW_STARTTORUN && pEvent->FwEventTxD.ucSeqNum != SEQ_FW_EVENT)
#endif		
	{
		pAdapter->PacketCMDSeqMCU = (UINT8)pEvent->FwEventTxD.ucSeqNum;			
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("PacketCMDSeqMCU = 0x%x\n", pAdapter->PacketCMDSeqMCU));
		pAdapter->FWRspStatus = (UINT8)pEvent->ucStatus;
		if(PktLen >PKTCMD_EVENT_BUFFER)
			pAdapter->FWRspContentLength = PKTCMD_EVENT_BUFFER;
		else
			pAdapter->FWRspContentLength = PktLen;
			RtlCopyMemory(pAdapter->FWRspContent, pBuffer, pAdapter->FWRspContentLength);
	}
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("PacketCMDSeqMCU = 0x%x\n", pAdapter->PacketCMDSeqMCU));
	switch(pEvent->FwEventTxD.ucEID)
	{
		case INIT_EVENT_CMD_RESULT:
		{					
			MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DEBUG_TRACE,("INIT_EVENT_CMD_RESULT\n"));
			ntStatus = FWInitCmdResultEvent(pAdapter, pBuffer, PktLen);
			break;
		}
		case INIT_EVENT_ACCESS_REG:
		{
			FW_EVENT_ACCESS_REG *pEventRegData;	
			MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DEBUG_TRACE,("ROM INIT_EVENT_ACCESS_REG\n"));
			pEventRegData = (FW_EVENT_ACCESS_REG *)pBuffer;
			pAdapter->RegValue= pEventRegData->u4Data;
			pAdapter->RegAddress= pEventRegData->u4Address;
			KeSetEvent(&pAdapter->FWCREvent, 0, FALSE);				
			break;
		}
		case INIT_EVENT_PATCH_SEMA_CTRL:
		{	
			MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DEBUG_TRACE,("INIT_EVENT_PATCH_SEMA_CTRL\n"));
			MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DEBUG_TRACE,("ucStatus = 0x%x, Seq = 0x%x\n", pEvent->ucStatus, pEvent->FwEventTxD.ucSeqNum));
			pAdapter->RomPatchSemStatus = (UINT8)pEvent->ucStatus;				
			KeSetEvent(&pAdapter->WaitFWEvent, 0, FALSE);
			break;
		}
		case EVENT_RESTART_DONWLOAD:
		{
			MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DEBUG_TRACE,("EVENT_RESTART_DONWLOAD\n"));
			MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DEBUG_TRACE,("ucStatus = 0x%x, Seq = 0x%x\n", pEvent->ucStatus, pEvent->FwEventTxD.ucSeqNum));
			pAdapter->RomPatchSemStatus = (UINT8)pEvent->ucStatus;				
			KeSetEvent(&pAdapter->WaitFWEvent, 0, FALSE);					
			break;
		}			
		case EVENT_EXT_CMD:
		{					
			MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DEBUG_TRACE,("EVENT_EXT_CMD\n"));					
			ntStatus = FWExtCmdEvent(pAdapter, pBuffer, PktLen);
			break;
		}
		default:
			MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DEBUG_TRACE,("unhandled EID\n"));
	}
	return ntStatus;
 }

 NTSTATUS	FWExtCmdEvent(
	IN	RTMP_ADAPTER	*pAdapter,
	IN	PVOID			pBuffer,
	IN	UINT32			Length){
	NTSTATUS	ntStatus = STATUS_SUCCESS;
	//UINT8 *pPtr = (UINT8 *)pBuffer;
	FwRamEventTxD_STRUC *pEventRamRxD;
	pEventRamRxD = (FwRamEventTxD_STRUC *)pBuffer;	
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DEBUG_TRACE, (" ======EXT_EVENT======\n"));
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DEBUG_TRACE, (" eCID = 0x%x\n", pEventRamRxD->ExtenEID));
	switch(pEventRamRxD->ExtenEID)
	{
		case EXT_EVENT_MULTI_CR_ACCESS:
		{
			
			PEXT_EVENT_MULTI_CR_ACCESS_T CRAccess = (PEXT_EVENT_MULTI_CR_ACCESS_T)pBuffer ;
			UINT32 count = 0;
//			pPtr+=sizeof(FwRamEventTxD_STRUC);
//			CRAccess = (FwRamEventTxD_STRUC *)pPtr;

			MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DEBUG_TRACE, (" #####EXT_EVENT_MULTI_CR_ACCESS\n"));
			MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DEBUG_TRACE, (" buffer size = %d\n", Length - sizeof(FwRamEventTxD_STRUC)));
			MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DEBUG_TRACE, (" Seq = 0x%x", CRAccess->FwEvent.ucSeqNum));
			MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DEBUG_TRACE, (" RxByteCount = 0x%x", CRAccess->FwEvent.u2RxByteCount));
			MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DEBUG_TRACE, (" EID = 0x%x", CRAccess->FwEvent.ucEID));
			MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DEBUG_TRACE, (" v1 = 0x%x", CRAccess->v1));
			MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DEBUG_TRACE, (" v2 = 0x%x", CRAccess->v2));
			MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DEBUG_TRACE, (" v3 = 0x%x", CRAccess->v3));
			if(CRAccess->FwEvent.ucSeqNum == SEQ_CMD_RAM_ACCESS_REG_READ)
			{				
				//RtlCopyMemory(pValue, &pAdapter->RegValueRead, sizeof(pAdapter->RegValueRead));
				pAdapter->RegValueRead = CRAccess->v3;
				MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DEBUG_TRACE, (" Reg Read = 0x%x",pAdapter->RegValueRead));
			}
			if(CRAccess->v1== MAC_CR)
				KeSetEvent(&pAdapter->FWCREvent, 0, FALSE);
			break;
		}
		case EXT_EVENT_TXBF_SOUNDING:
		{
			MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DEBUG_TRACE, (" EXT_EVENT_TXBF_SOUNDING\n"));
			KeSetEvent(&pAdapter->WaitFWEvent, 0, FALSE);
		}
			break;
		default:
			MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DEBUG_TRACE, ("unhandled Seqnum, 0x%x\n", pEventRamRxD->ucSeqNum));
	}

	return ntStatus;
 }


/*
	========================================================================
	
	Routine Description:
		Used in load by driver self not by UI

	Arguments:
		Adapter						Pointer to our adapter

	Return Value:
		NDIS_STATUS_SUCCESS         firmware image load ok
		NDIS_STATUS_FAILURE         image not found

	IRQL = PASSIVE_LEVEL
		
	========================================================================
*/
NTSTATUS NICLoadRomPatch(
	IN RTMP_ADAPTER *pAdapter)
{
	NTSTATUS 				LoadStatus = STATUS_SUCCESS;
	NTSTATUS 				CRStatus = STATUS_SUCCESS;
	PUCHAR 					pRomPatchImage = NULL;
	ULONG  					RomPatchLength  = 0;
	UINT16					BreakCount = 0;
	UINT32					CRValue = 0;
	SCHEDULER_REGISTER4 	SchedulerRegister4, backupSchedulerRegister4;

#ifdef RTMP_USB_SUPPORT
	//UDMA Tx/Rx already been set in init
	//step 1  power on Wi-Fi sys
	LoadStatus = ROMPatchPowerOnWiFiSys(pAdapter);
	if(LoadStatus != STATUS_SUCCESS)
	{
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,("%s [FWDL] ROMPatchPowerOnWiFiSys failed\n", __FUNCTION__));
		return LoadStatus;
	}
	else
	{
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE,("%s [FWDL]ROMPatchPowerOnWiFiSys succeed\n", __FUNCTION__));
	}
  
	//step 2 Set DMA scheduler to Bypass mode.
	CRStatus = RTMP_IO_READ32(pAdapter, SCH_REG4, &SchedulerRegister4.word);
	
	if (CRStatus != STATUS_SUCCESS)
	{
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,("%s [FWDL]Read Mac Register SCH_REG4 failed\n", __FUNCTION__));
		return STATUS_UNSUCCESSFUL;
	}
	    
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE,("1 [FWDL]SchedulerRegister4 = %x\n", SchedulerRegister4.word));
	backupSchedulerRegister4.word = SchedulerRegister4.word;
	SchedulerRegister4.Default.BypassMode = 1;
	SchedulerRegister4.Default.ForceQid = 8;
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE,("2 [FWDL]SchedulerRegister4 = %x\n", SchedulerRegister4.word));	
	CRStatus = RTMP_IO_WRITE32(pAdapter, SCH_REG4, SchedulerRegister4.word);
	
	if (CRStatus != STATUS_SUCCESS)
	{
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,("%s [FWDL]Write Mac Register SCH_REG4 failed\n", __FUNCTION__));
		return STATUS_UNSUCCESSFUL;
	}
#endif
	//step 3 Polling 0x81021250 = 0x01, and then can download patch
	do
	{
		ULONG CRValue = 0;		
		RTMP_IO_READ32(pAdapter, SW_SYNC, &CRValue);
		if(CRValue == 0x01)
		{			
			MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DEBUG_TRACE, ("[FWDL]!!!!! Ready to download Rom Patch !!!!!\n"));
			break;
		}	
		RTMPusecDelay(50);
		
		BreakCount ++;
		if (BreakCount > 100)
		{
			MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DEBUG_ERROR,("[FWDL] Polling ready to download Rom Patch failed SW_SYNC(0x%x) = 0x%x\n", SW_SYNC, CRValue));
			break;
		}
	        
	} while(1);	
	//step 4 download ROM patch image check semaphore
	LoadStatus = FirmwareCommnadGetPatchSemaphore(pAdapter);
	if (LoadStatus != STATUS_SUCCESS)
	{
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DEBUG_ERROR, ("[FWDL] !!!!! FirmwareCommnadGetPatchSemaphore failed. !!!!!\n"));
		return LoadStatus;
	}	
	//step 5 if RomPatchSemStatus != 0x2 => no need to download rom patch
	if(pAdapter->RomPatchSemStatus == 0x2)
	{
		if(pAdapter->IsUISetFW)
		{
			LoadStatus = LoadROMPatchProcess(pAdapter, pAdapter->FWImage, pAdapter->FWSize);
		}
		else
		{
			LoadStatus = LoadROMPatchProcess(pAdapter, FW7636_ROMPATCH, FW7636_ROMPATCH_LENGTH);
		}
		if (LoadStatus != STATUS_SUCCESS)
		{
			MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("[FWDL] !!!!! Load ROM Patch failed. !!!!!\n"));
			return STATUS_UNSUCCESSFUL;
		}
	}
	else
	{
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DEBUG_TRACE, ("[FWDL] !!!!! No need to load ROM sem = 0x%x !!!!!\n", pAdapter->RomPatchSemStatus));
	}

	
#ifdef RTMP_USB_SUPPORT
	// step 6 Set DMA scheduler to original mode.
	SchedulerRegister4.Default.BypassMode = backupSchedulerRegister4.Default.BypassMode;
	CRStatus = USBHwHal_WriteMacRegister(pAdapter, SCH_REG4, SchedulerRegister4.word);
	if (CRStatus != STATUS_SUCCESS)
	{
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,("[FWDL] %s Set DMA scheduler to original mode failed\n", __FUNCTION__));
		return STATUS_UNSUCCESSFUL;
	}	
#endif		     
	return LoadStatus;
}

//
// Load Andes ROM PATCH
// MT7636 FPGA
//
NTSTATUS LoadROMPatchProcess(
	IN RTMP_ADAPTER *pAdapter,
	IN PUCHAR	pFileName,
	UINT32		FileLength
	)
{
	NTSTATUS Status = STATUS_SUCCESS;
	PUCHAR pFirmwareImage = pFileName;
	ULONG  FWImageLength  = 0;
	BOOLEAN bLoadExternalFW = FALSE;
	PROM_PATCH_BIN_HEADER_STRUC	pROMInfo = NULL;
	ROMPATCH_CAL_CHECKSUM_DATA	CalChksumData;
	//checksume return by FW
	USHORT	FWChecksum = 0, ImgChecksum = 0;	
	UCHAR BreakCount = 0;
	UINT32 WHIERValue = 0;
	
	if(FileLength <= ROM_PATCH_INFO_LENGTH)
	{
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DEBUG_ERROR,("invalid ROM patch, size < 30 bytes\n"));
		return STATUS_INSUFFICIENT_RESOURCES;
	}

	pROMInfo = (PROM_PATCH_BIN_HEADER_STRUC)(pFirmwareImage);
	FWImageLength = FileLength - ROM_PATCH_INFO_LENGTH;
	ImgChecksum	 =  pROMInfo->Checksum;

	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DEBUG_TRACE,("%s: FileLength = 0x%08X, ROM patch Length = 0x%08X\n", __FUNCTION__, FileLength, FWImageLength));
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DEBUG_TRACE,("pROMInfo->Checksum = 0x%04X\n", pROMInfo->Checksum));	
		
	do
	{
		// Send command packet to FW to set "start to load FW"
		Status = FirmwareCommnadStartToLoadROMPatch(pAdapter, ROM_PATCH_ADDRESS, FWImageLength);
		if (Status != STATUS_SUCCESS)
		{
			MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DEBUG_ERROR, ("[FWDL] !!!!! LoadROMPatchProcess failed. !!!!!\n"));
			return STATUS_UNSUCCESSFUL;
		}

		RTMPusecDelay(1000);

		pAdapter->bLoadingFW = TRUE;

		// Start to load FW
		Status = FirmwareScatters(pAdapter, pFirmwareImage+ROM_PATCH_INFO_LENGTH, FWImageLength);
		if (Status != STATUS_SUCCESS)
		{
			MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DEBUG_ERROR, ("[FWDL] !!!!! FirmwareScatters failed. !!!!!\n"));
			Status = STATUS_UNSUCCESSFUL;
		}
		pAdapter->bLoadingFW = FALSE;

	} while(FALSE);	
	

	// send CMD patch finish
	Status = FirmwareCommnadLoadROMPatchFinish(pAdapter);
	if (Status != STATUS_SUCCESS)
	{
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("[FWDL] !!!!! FirmwareCommnadLoadROMPatchFinish failed. !!!!!\n"));
		//return STATUS_SUCCESS;
	}	
#ifdef RTMP_USB_SUPPORT
	// request FW to calculate download ROM image checksum
	CalChksumData.Address = ROM_PATCH_ADDRESS;
	CalChksumData.Length = FWImageLength;
	USBROMPatchCalChecksum(pAdapter, &CalChksumData, sizeof(ROMPATCH_CAL_CHECKSUM_DATA));
	USBROMPatchGetChecksum(pAdapter, &FWChecksum, sizeof(FWChecksum));

	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,("[FWDL] FWChecksum = 0x%04X, ImgChecksum = 0x%04X\n", FWChecksum, ImgChecksum));

	if(FWChecksum != ImgChecksum)
	{
		Status = STATUS_UNSUCCESSFUL;
	}	
	else
	{
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("[FWDL]!!!!! Rom Patch download succeed !!!!!\n"));
	}
	
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("[FWDL] %s: <--\n", __FUNCTION__));
#endif
	return Status;
}

/*
	========================================================================
	
	Routine Description:
		Used in load by driver self not by UI

	Arguments:
		Adapter						Pointer to our adapter

	Return Value:
		NDIS_STATUS_SUCCESS         firmware image load ok
		NDIS_STATUS_FAILURE         image not found

	IRQL = PASSIVE_LEVEL
		
	========================================================================
*/
NTSTATUS NICLoadFirmware(
    IN RTMP_ADAPTER *pAdapter)
{
	NTSTATUS 				LoadStatus = STATUS_SUCCESS;
	NTSTATUS 				CRStatus = STATUS_SUCCESS;
	PUCHAR 					pFirmwareImage = NULL;
	ULONG  					FWImageLength  = 0;
	PFW_BIN_INFO_STRUC_7603 pFwInfo = NULL;
	TOP_MISC_CONTROLS2 	TopMiscControls2;
	U3DMA_WLCFG         		U3DMAWLCFG;
	SCHEDULER_REGISTER4 	SchedulerRegister4, backupSchedulerRegister4;
	ULONG                  		BreakCount = 0;
	ULONG					FileLength = 0;
	FW_BIN_INFO_STRUC_7636 ILMBinInfo, DLMBinInfo;
	ULONG					MACValue = 0;
	BOOLEAN					bIsReDownload = FALSE;
	UINT32 CRValue = 0;
	UINT32 uWHLPCR = 0, uWHISR=0, uWHIER=0;

	RtlZeroMemory(&ILMBinInfo, sizeof(FW_BIN_INFO_STRUC_7636));
	//FW Image choose
	//IsUISetFW: Load from UI
	//!IsUISetFW: Load from default mt7603.h 
	if(pAdapter->IsUISetFW)
	{
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DEBUG_TRACE,("%s [FWDL] Load FW from UI, Image size = %d should be the same as FWSize = %d\n", __FUNCTION__, sizeof(pAdapter->FWImage), pAdapter->FWSize));
		pFirmwareImage = pAdapter->FWImage;	
		FileLength = pAdapter->FWSize;
	}
	else
	{
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DEBUG_TRACE,("%s [FWDL] Load FW from driver\n", __FUNCTION__));
		if(IS_MT7603(pAdapter))
		{
#ifdef RTMP_USB_SUPPORT	
			pFirmwareImage = FW7603;	
			FileLength = FW7603_LENGTH;
#endif			
		}
		else if(IS_MT76x6(pAdapter))
		{
			pFirmwareImage = FW7636;	
			FileLength = FW7636_LENGTH;
		}
	}
	//MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("%s: Load the internal firmware file, Length = %d\n", __FUNCTION__, FW7603_LENGTH));
//	
	if(IS_MT7603(pAdapter))
	{
		// check if need to reload FW
#ifdef RTMP_USB_SUPPORT
		CRStatus = RTMP_IO_READ32(pAdapter, TOP_MISC2, &TopMiscControls2.word);
		if (CRStatus != STATUS_SUCCESS)
		{
			MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DEBUG_ERROR,("%s [FWDL] Read Mac Register TOP_MISC2 failed\n", __FUNCTION__));
			return STATUS_UNSUCCESSFUL;
		}
		if (TopMiscControls2.Default.FwIsRunning)
		{
			bIsReDownload = TRUE;
		}
		else
		{
			bIsReDownload = FALSE;
		}
			
		if(FileLength <= FW_INFO_LENGTH_7603)
		{
			MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DEBUG_ERROR,("[FWDL] invliad firmware, size < 64 bytes\n"));
			return STATUS_INSUFFICIENT_RESOURCES;
		}
#endif		
	}	
	else if(IS_MT76x6(pAdapter))
	{	
		ULONG CRValue = 0;		
		// check if need to reload FW
		//_ SDIO
		RTMP_IO_READ32(pAdapter, SW_SYNC, &CRValue);				
		if (CRValue == 3)
		{
			bIsReDownload = TRUE;
		}
		else
		{
			bIsReDownload = FALSE;
		}
	}
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DEBUG_TRACE,("[FWDL] Is Need to redownload?, %d\n", bIsReDownload));

#ifdef RTMP_USB_SUPPORT
//USB and PCIe
	// Set DMA scheduler to Bypass mode.
	CRStatus = RTMP_IO_READ32(pAdapter, SCH_REG4, &SchedulerRegister4.word);
	
	if (CRStatus != STATUS_SUCCESS)
	{
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,("%s [FWDL] Read Mac Register SCH_REG4 failed\n", __FUNCTION__));
		return STATUS_UNSUCCESSFUL;
	}
	    
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE,("[FWDL] 1 SchedulerRegister4 = %x\n", SchedulerRegister4.word));
	backupSchedulerRegister4.Default.BypassMode = SchedulerRegister4.Default.BypassMode;
	SchedulerRegister4.Default.BypassMode = 1;
	SchedulerRegister4.Default.ForceQid = 8;
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE,("[FWDL] 2 SchedulerRegister4 = %x\n", SchedulerRegister4.word));	
	CRStatus = RTMP_IO_WRITE32(pAdapter, SCH_REG4, SchedulerRegister4.word);
	
	if (CRStatus != STATUS_SUCCESS)
	{
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,("%s Write Mac Register SCH_REG4 failed\n", __FUNCTION__));
		return STATUS_UNSUCCESSFUL;
	}
//~USB and PCIe
#endif
	//step 2 check if need to restart download, ram to rom
	if (bIsReDownload)
	{
		LoadStatus = FirmwareCommnadRestartDownloadFW(pAdapter);
		if (LoadStatus != STATUS_SUCCESS)
		{               
			MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DEBUG_ERROR,("[FWDL] %s FirmwareCommnadRestartDownloadFW failed\n", __FUNCTION__));
			return STATUS_UNSUCCESSFUL;
		}
	}

	if(IS_MT7603(pAdapter))
	{
#ifdef RTMP_USB_SUPPORT
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE,("[FWDL] TOP_MISC2 = %x\n", TopMiscControls2.word));	
		//pAdapter->BulkOutRemained = 0;
		//pAdapter->ContinBulkOut = FALSE;	

		BreakCount = 0;//reset count to 0
		do
		{
			ULONG CRValue = 0;
			CRStatus = RTMP_IO_READ32(pAdapter, TOP_MISC2, &TopMiscControls2.word);
			if (TopMiscControls2.Default.ReadyToLoadFW)
			{
				MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("!!!!! [7603] Ready to download ram !!!!!\n"));
				break;
			}
		        
			RTMPusecDelay(100000);// 1 ms		        

			BreakCount ++;
			if (BreakCount > 100)
			{
				MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,("[FWDL] !!!!! TOP is not ready to load FW. !!!!!\n"));
				break;
			}		        
		} while(1);
		pFwInfo           = (PFW_BIN_INFO_STRUC_7603)(pFirmwareImage + (FW7603_LENGTH - sizeof(FW_BIN_INFO_STRUC_7603)));
		FWImageLength    = pFwInfo->ImageLength + 4;  // 4 is CRC length
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE,("%s: FileLength = 0x%08X, FWImageLength = 0x%08X\n", __FUNCTION__, FileLength, FWImageLength));
		// Send command packet to FW to set "start to load FW"	
		
		LoadStatus = FirmwareCommnadStartToLoadFW(pAdapter, FWImageLength, 0x00100000,0x80000000);
		if (LoadStatus != STATUS_SUCCESS)
		{
			MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("!!!!! FirmwareCommnadStartToLoadFW failed. !!!!!\n"));
			return STATUS_UNSUCCESSFUL;
		}

		pAdapter->bLoadingFW = TRUE;

		// Start to load FW
		LoadStatus = FirmwareScatters(pAdapter, pFirmwareImage, FWImageLength);
		if (LoadStatus != STATUS_SUCCESS)
		{
			MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("!!!!! FirmwareScatters failed. !!!!!\n"));
			return STATUS_UNSUCCESSFUL;
		}

		pAdapter->bLoadingFW = FALSE;
#endif		
	}
	else if(IS_MT76x6(pAdapter))
	{
		//step 3 Polling 0x81021250 = 0x01, and then can download FW

		BreakCount = 0;//reset count to 0
		do
		{
			ULONG CRValue = 0;
			RTMP_IO_READ32(pAdapter, SW_SYNC, &CRValue);
			if (CRValue == 0x1)
			{
				MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DEBUG_TRACE, ("[FWDL] !!!!! Ready to download Ram !!!!!\n"));
				break;
			}
		        
			RTMPusecDelay(100000);// 1 ms		        

			BreakCount ++;
			if (BreakCount > 100)
			{
				MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DEBUG_ERROR,("[FWDL] Polling ready to download Ram failed 0x81021250 = 0x%x\n", CRValue));
				break;
			}
		        
		} while(1);
		BreakCount = 0;//reset count to 0
		
		// download ILM RAM image
		LoadStatus = LoadFwImageProcess(pAdapter, pFirmwareImage, FileLength, ILM);
		if (LoadStatus != STATUS_SUCCESS)
		{
			MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DEBUG_TRACE, ("[FWDL] !!!!! Load ILM failed. !!!!!\n"));
			return STATUS_UNSUCCESSFUL;
		}
		else
		{
			MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DEBUG_TRACE, ("[FWDL] !!!!! Load ILM succeed. !!!!!\n"));
		}

		// download DLM RAM image
		LoadStatus = LoadFwImageProcess(pAdapter, pFirmwareImage, FileLength, DLM);
		if (LoadStatus != STATUS_SUCCESS)
		{
			MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DEBUG_TRACE, ("[FWDL] !!!!! Load DLM failed. !!!!!\n"));
			return STATUS_UNSUCCESSFUL;
		}
		else
		{
			MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DEBUG_TRACE, ("[FWDL] !!!!! Load DLM Succeed. !!!!!\n"));
		}

		RTMPusecDelay(10000);// 1 ms	
	}

	// FW Starting
	// difference of 7603 and 7636 difference in the function FirmwareCommnadStartToRunFW
	LoadStatus = FirmwareCommnadStartToRunFW(pAdapter);
	if (LoadStatus != STATUS_SUCCESS)
	{
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("[FWDL] !!!!! FirmwareCommnadStartToRunFW failed. !!!!!\n"));
		//return STATUS_UNSUCCESSFUL;
	}
	RTMPusecDelay(100000);//100ms
	
#ifdef RTMP_USB_SUPPORT
	// Set DMA scheduler to original mode.
	SchedulerRegister4.Default.BypassMode = backupSchedulerRegister4.Default.BypassMode;
	CRStatus = RTMP_IO_WRITE32(pAdapter, SCH_REG4, SchedulerRegister4.word);
	if (CRStatus != STATUS_SUCCESS)
	{
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,("%s [FWDL] Set DMA scheduler to original mode failed\n", __FUNCTION__));
		return STATUS_UNSUCCESSFUL;
	}
#endif
	if(IS_MT7603(pAdapter))
	{
#ifdef RTMP_USB_SUPPORT 
		// Polling 0x80021134 [1] = 1, and then can read/write CRs
		do
		{
			CRStatus = RTMP_IO_READ32(pAdapter, TOP_MISC2, &TopMiscControls2.word);		
			if (TopMiscControls2.Default.FwIsRunning == 1)
			{
				MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("[FWDL] !!!!! Load firmware succeed !!!!!\n"));
				break;
			}
	        
			RTMPusecDelay(100000);// 1 ms	        

			BreakCount ++;
			if (BreakCount > 100)
			{
				MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,("[FWDL] Polling FW ready bit failed.\n"));
				return STATUS_UNSUCCESSFUL;
				break;
			}
	        
		} while(1);
		pAdapter->FWMode = FWRAM;
#endif		
	}

	else if(IS_MT76x6(pAdapter))	
	{
		UINT32 Value =0;
		BreakCount = 0;
#ifdef RTMP_SDIO_SUPPORT		
		while(!(Value & W_FUNC_RDY)) {		
			RTMP_SDIO_READ32(pAdapter, WCIR, &Value);
			MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("%s(): WCIR: 0x%x\n",__FUNCTION__,Value));
			BreakCount++;
			if(BreakCount >100){
				MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("W_FUNC_RDY != 1\n"));
				return STATUS_UNSUCCESSFUL;
				break;
			}
			
			RTMPusecDelay(30000);// 30ms			
		}
#endif	
		
#ifdef RTMP_USB_SUPPORT
		// Polling 0x81021250 = 0x03, and then can read/write CRs
		do
		{			
			BreakCount ++;		
		
			CRStatus = RTMP_IO_READ32(pAdapter, SW_SYNC, &CRValue);				
			if (CRValue == 0x3)
			{
				MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DEBUG_TRACE, ("[FWDL] !!!!! Load firmware succeed !!!!!\n"));
				break;
			}
	        
			RTMPusecDelay(10000);
			
			if (BreakCount > 20)
			{
				MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,("[FWDL] Polling FW ready bit failed 0x81021250 = 0x%x\n", CRValue));
				break;
			}

		} while(1);
#endif
		pAdapter->FWMode = FWRAM;
	}	
	
	pAdapter->LoadingFWCount ++;
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DEBUG_TRACE, ("[FWDL] ####################### pAd->LoadingFWCount = %d ###################\n", pAdapter->LoadingFWCount));

	return LoadStatus;
}

	// ===============================================
	// Load Firmware 7636 
	// ===============================================
	//  Andes FW loading architecture
	//
	//  Bin file format
	//    +--------------------------+
	//    |       FW ILM part        	| 4N or (16N-4) bytes
	//    +--------------------------+
	//    |       ILM CRC       	| 4 bytes
	//    +--------------------------+
	//    |       FW DLM part	| 4N or (16N-4) bytes
	//    +--------------------------+		
	//    |       DLM CRC       	| 4 bytes
	//    +--------------------------+	
	//    |       ILM Tailer (Info)	| 36 bytes
	//    +--------------------------+		
	//    |       DLM Tailer (Info)	| 36 bytes
	//    +--------------------------+		
//
// Load Andes Ram code
// MT7603 FPGA
//
NTSTATUS LoadFwImageProcess(
	IN RTMP_ADAPTER *pAdapter,
	IN PUCHAR	pFileName,
	IN UINT32	FileLength,
	IN UCHAR	Type
	)
{
	NTSTATUS Status = STATUS_SUCCESS;
	//UINT32 FileLength = 0;
	PUCHAR pFirmwareImage = pFileName;
	ULONG  FWImageLength  = 0;
	PFW_BIN_INFO_STRUC_7636 pFwInfo = NULL;
	UINT16 BreakCount = 0;	

	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DEBUG_TRACE, ("[FWDL] %s: file length = %d\n", __FUNCTION__,FileLength));
	
	if(FileLength <= FW_INFO_LENGTH_7636)
	{
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DEBUG_ERROR,("[FWDL] invliad firmware, size < 36 bytes\n"));
		return STATUS_UNSUCCESSFUL;
	}
	//ILM
	if(ILM == Type)
	{
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DEBUG_TRACE,("[FWDL] %s  ===ILM===\n", __FUNCTION__));
		pFwInfo = (PFW_BIN_INFO_STRUC_7636)(pFirmwareImage + (FileLength - 2*sizeof(FW_BIN_INFO_STRUC_7636) ));
		
	}
	//DLM
	else if(DLM == Type)
	{
		PFW_BIN_INFO_STRUC_7636 ILMInfo = NULL;
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DEBUG_TRACE,("[FWDL] %s  ===DLM===\n", __FUNCTION__));
		ILMInfo = (PFW_BIN_INFO_STRUC_7636)(pFirmwareImage + (FileLength - 2*sizeof(FW_BIN_INFO_STRUC_7636) ));
	
		pFwInfo = (PFW_BIN_INFO_STRUC_7636)(pFirmwareImage + (FileLength - sizeof(FW_BIN_INFO_STRUC_7636)));
		pFirmwareImage = pFirmwareImage + ILMInfo->ImageLength + 4;
	}		
	FWImageLength    = pFwInfo->ImageLength + 4;  // 4 is CRC length

	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DEBUG_TRACE,("%s: FileLength = %d, FWImageLength = %d\n", __FUNCTION__, FileLength, FWImageLength));	
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DEBUG_TRACE,("=== FW Info ===\n"));
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DEBUG_TRACE,("%s: Address = 0x%x\n", __FUNCTION__, pFwInfo->Address));
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DEBUG_TRACE,("=== Address = 0x%x ===\n", pFwInfo->Address));
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DEBUG_TRACE,("=== Length = %d ===\n", FWImageLength));
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DEBUG_TRACE,("=== Encryption = %d ===\n", pFwInfo->FeatureSet.Encryption));	
#ifdef RTMP_USB_SUPPORT	
	do
	{
		UINT32 CRValue = 0;
		Status = RTMP_IO_READ32(pAdapter, SW_SYNC, &CRValue);		
		if (CRValue == 0x1)
		{
			MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("[FWDL] !!!!! Ready to download Ram !!!!!\n"));
			break;
		}
	        
		RTMPusecDelay(100000);// 1 ms		        
		BreakCount ++;
		if (BreakCount > 100)
		{
			MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,("[FWDL] Polling ready to download Ram failed 0x81021250 = 0x%x\n", CRValue));
			break;
		}
		        
	} while(1);
#endif //RTMP_USB_SUPPORT	
	do
	{
		ULONG DataMode = 0x80000000;//parameter for start to load fw
		if (IS_MT76x6(pAdapter) && pFwInfo->FeatureSet.Encryption ==1)		
		{
			MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE,("=== Encryption = %d ===\n", pFwInfo->FeatureSet.Encryption));
			MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE,("=== KEY_INDEX = %d ===\n", pFwInfo->FeatureSet.KEY_INDEX));			
			DataMode |= pFwInfo->FeatureSet.Encryption;
			DataMode |= (pFwInfo->FeatureSet.KEY_INDEX << 1);
			DataMode |= 0x8;//turn on bit 3			
		}		
		// Send command packet to FW to set "start to load FW"
		Status = FirmwareCommnadStartToLoadFW(pAdapter, FWImageLength,  pFwInfo->Address, DataMode);
		if (Status != STATUS_SUCCESS)
		{
			MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("[FWDL] !!!!! FirmwareCommnadStartToLoadFW failed. !!!!!\n"));
			return STATUS_UNSUCCESSFUL;
		}

		RTMPusecDelay(1000);
#if 1
		pAdapter->bLoadingFW = TRUE;

		// Start to load FW
		Status = FirmwareScatters(pAdapter, pFirmwareImage, FWImageLength);
		if (Status != STATUS_SUCCESS)
		{
			MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("[FWDL] !!!!! FirmwareScatters failed. !!!!!\n"));
			return STATUS_UNSUCCESSFUL;
		}

		pAdapter->bLoadingFW = FALSE;
#endif	
	} while(FALSE);

	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("[FWDL] %s: <--\n", __FUNCTION__));
	
	return Status;
}

NTSTATUS	ROMPatchPowerOnWiFiSys(
	IN	RTMP_ADAPTER	*pAd)
{
	NTSTATUS ntStatus = STATUS_SUCCESS;
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DEBUG_TRACE, ("-->%s\n", __FUNCTION__));	
	//  parameters based on FW	
#ifdef RTMP_USB_SUPPORT
	ntStatus = USBVendorRequest(pAd, NULL, 0, 0x04, 0x0, 0x1, VendorReqOut);
#endif	
	return ntStatus;
}

NTSTATUS	USBROMPatchCalChecksum(
	IN	RTMP_ADAPTER	*pAd,
	IN	PVOID			pData,
	IN	USHORT			Length)
{
	NTSTATUS ntStatus = STATUS_SUCCESS;
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DEBUG_TRACE, ("-->%s\n", __FUNCTION__));	
#ifdef RTMP_USB_SUPPORT	
	ntStatus = USBVendorRequest(pAd, pData, Length, 0x01, 0x20, 0x0, VendorReqOut);
#endif
	return ntStatus;
}

NTSTATUS	USBROMPatchGetChecksum(
	IN	RTMP_ADAPTER	*pAd,
	IN	PVOID			pData,
	IN	USHORT			Length)
{
	NTSTATUS ntStatus = STATUS_SUCCESS;
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DEBUG_TRACE, ("-->%s\n", __FUNCTION__));
#ifdef RTMP_USB_SUPPORT	
	ntStatus = USBVendorRequest(pAd, pData, Length, 0x01, 0x21, 0x0, VendorReqIn);	
#endif
	return ntStatus;
}

NTSTATUS	FWTxCmd(
	IN	RTMP_ADAPTER	*pAdapter,
	IN	PVOID			pCmd,
	IN	UINT32			Length,
	IN	KEVENT			*pEvent){
	NTSTATUS	ntStatus = STATUS_SUCCESS;
	LARGE_INTEGER	TimeOut1Second;
	FIRMWARE_TXDSCR *pFwTxD;
	//TimeOut1Second.QuadPart =0;
	if(pAdapter->bLoadingFW)
		TimeOut1Second.QuadPart = -(1000* 1000);
	else
		TimeOut1Second.QuadPart = 0;
// SDIO
	
//Debug
	pFwTxD = (FIRMWARE_TXDSCR *)pCmd;
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DEBUG_TRACE, ("Cmd Length = 0x%x, CID = 0x%x, Seq = 0x%x\n", Length, pFwTxD->CID, pFwTxD->SeqNum));
//~Debug
	do{
#ifdef RTMP_SDIO_SUPPORT
		pAdapter->bIsWaitFW = TRUE;			
		IOWriteTx(pAdapter, (PUCHAR)pCmd, Length, SDIO_SYNC);
		pAdapter->bIsWaitFW = FALSE;
#endif
#ifdef RTMP_USB_SUPPORT
		FwCmdUSBBulkout(pAdapter, (PUCHAR)pCmd, Length);
#endif
		if(pEvent)
		{
			ntStatus = Event_Wait(pAdapter, pEvent, TimeOut1Second);
		}
	}while(FALSE);
	return ntStatus;

}

//
// MT7636 firmware command start to load patch
//
NTSTATUS FirmwareCommnadStartToLoadROMPatch(
	IN	RTMP_ADAPTER	*pAdapter,
	IN	UINT32			Address,
	IN	UINT32			Length
	)
{
	NTSTATUS 			ntStatus = STATUS_SUCCESS;
	CMD_START_TO_LOAD_FW_REQUEST	CmdStartToLoadFwRequest;	
	UINT32				BreakCount = 0;
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DEBUG_TRACE,("### %s ###\n", __FUNCTION__));

	// Fill setting
	RtlZeroMemory(&CmdStartToLoadFwRequest, sizeof(CMD_START_TO_LOAD_FW_REQUEST));
	CmdStartToLoadFwRequest.FwTxD.PQ_ID = 0x8000; //P_IDX[31] = 0x1, Q_IDX[31 : 27] = 0x0
	CmdStartToLoadFwRequest.FwTxD.CID = CMD_PATCH_START; // The ID of CMD w/ target address/length request
	CmdStartToLoadFwRequest.FwTxD.PktTypeID = 0xA0; //bit 15 = 1'b1, bit[14:13] = 0x1
	CmdStartToLoadFwRequest.FwTxD.Length = sizeof(CMD_START_TO_LOAD_FW_REQUEST);	
	CmdStartToLoadFwRequest.FwTxD.SeqNum = SEQ_CMD_ROM_PATCH_STARTREQ;
	CmdStartToLoadFwRequest.Address = Address;	
	CmdStartToLoadFwRequest.Length = Length;  // 4 is CRC length;
	CmdStartToLoadFwRequest.DataMode = 0x80000000;			// 0: plan mode
// SDIO

	ntStatus = FWTxCmd(pAdapter, &CmdStartToLoadFwRequest, sizeof(CMD_START_TO_LOAD_FW_REQUEST), &pAdapter->WaitFWEvent);//WaitFWEvent);
	if(ntStatus != STATUS_SUCCESS){
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DEBUG_ERROR, ("><>< CMD Start to load Rom Patch failed\n"));
		return ntStatus;
	}
	return ntStatus;
}

NTSTATUS FirmwareCommnadGetPatchSemaphore(
	IN	RTMP_ADAPTER	*pAdapter
	)
{
	NTSTATUS 			ntStatus = STATUS_SUCCESS;
	CMD_GET_PATCH_SEMAPHORE	CmdGetPatchSem;
	LARGE_INTEGER		TimeOut1Second;

	TimeOut1Second.QuadPart = -(500* 10000);
	RtlZeroMemory(&CmdGetPatchSem, sizeof(CMD_GET_PATCH_SEMAPHORE));
	CmdGetPatchSem.FwTxD.PQ_ID 	= 0x8000; //P_IDX[31] = 0x1, Q_IDX[31 : 27] = 0x0
	CmdGetPatchSem.FwTxD.Length 	= sizeof(CMD_GET_PATCH_SEMAPHORE);
	CmdGetPatchSem.FwTxD.CID 	= CMD_PATCH_SEMAPHORE_CONTROL;
	CmdGetPatchSem.FwTxD.PktTypeID 	= 0xA0;
	CmdGetPatchSem.FwTxD.SeqNum	= SEQ_CMD_ROM_PATCH_SEMAPHORE;
	CmdGetPatchSem.ucGetSemaphore	= 0x1;

	ntStatus = FWTxCmd(pAdapter, &CmdGetPatchSem, sizeof(CMD_GET_PATCH_SEMAPHORE), &pAdapter->WaitFWEvent);//RomPatchEvent);
	if(ntStatus != STATUS_SUCCESS){
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DEBUG_ERROR, ("><>< Get Rom semaphore failed\n"));
		return ntStatus;
	}

	return ntStatus;
}

//
// MT7636 firmware command patch finish
//
NTSTATUS FirmwareCommnadLoadROMPatchFinish(
	IN	RTMP_ADAPTER	*pAdapter
	)
{
	NTSTATUS 			ntStatus = STATUS_SUCCESS;
	FIRMWARE_TXDSCR	CmdRomPatchFinish;
	LARGE_INTEGER		TimeOut1Second;

	TimeOut1Second.QuadPart = -(500* 10000);

	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE,("### %s ###\n", __FUNCTION__));

	// Fill setting
	RtlZeroMemory(&CmdRomPatchFinish, sizeof(FIRMWARE_TXDSCR));
	CmdRomPatchFinish.PQ_ID = 0x8000; //P_IDX[31] = 0x1, Q_IDX[31 : 27] = 0x0
	CmdRomPatchFinish.CID = CMD_PATCH_FINISH; // The ID of CMD w/ target address/length request
	CmdRomPatchFinish.PktTypeID = 0xA0; //bit 15 = 1'b1, bit[14:13] = 0x1
	CmdRomPatchFinish.Length = sizeof(FIRMWARE_TXDSCR);	
	CmdRomPatchFinish.SeqNum = SEQ_CMD_ROM_PATCH_FINISH;

	
	ntStatus = FWTxCmd(pAdapter, &CmdRomPatchFinish, sizeof(FIRMWARE_TXDSCR), &pAdapter->WaitFWEvent);//WaitFWEvent);
	if(ntStatus != STATUS_SUCCESS){
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DEBUG_ERROR, ("><><CMD load Rom Patch finish failed\n"));
	}
	/*************************************************************/
	
	return ntStatus;
}

NTSTATUS FirmwareCommnadStartToLoadFW(
	IN  RTMP_ADAPTER    *pAdapter,
	IN  ULONG		ImageLength,
	IN  ULONG		Address,
	IN  ULONG		DataMode
	)
{
	NTSTATUS	ntStatus = STATUS_SUCCESS;
	CMD_START_TO_LOAD_FW_REQUEST    CmdStartToLoadFwRequest;

	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE,("### FirmwareCommnadStartToLoadFW ###\n"));
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE,("=== FW Info ===\n"));
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE,("=== Address = 0x%x ===\n", Address));
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE,("=== Length = %d ===\n", ImageLength));

	// Fill setting
	RtlZeroMemory(&CmdStartToLoadFwRequest, sizeof(CMD_START_TO_LOAD_FW_REQUEST));
	CmdStartToLoadFwRequest.FwTxD.PQ_ID = 0x8000; //P_IDX[31] = 0x1, Q_IDX[31 : 27] = 0x0
	CmdStartToLoadFwRequest.FwTxD.CID = CMD_START_LOAD; // The ID of CMD w/ target address/length request
	CmdStartToLoadFwRequest.FwTxD.PktTypeID = 0xA0; //bit 15 = 1'b1, bit[14:13] = 0x1
	CmdStartToLoadFwRequest.FwTxD.Length = sizeof(CMD_START_TO_LOAD_FW_REQUEST);    
	CmdStartToLoadFwRequest.FwTxD.SeqNum = SEQ_CMD_FW_STARTREQ;
	CmdStartToLoadFwRequest.Address = Address;   
	CmdStartToLoadFwRequest.Length = ImageLength;  // FW    size
	CmdStartToLoadFwRequest.DataMode = DataMode;//0x80000000;
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE,("=== DataMode = 0x%x ===\n", CmdStartToLoadFwRequest.DataMode));
	ntStatus = FWTxCmd(pAdapter, &CmdStartToLoadFwRequest, sizeof(CMD_START_TO_LOAD_FW_REQUEST), &pAdapter->WaitFWEvent);
	if(ntStatus != STATUS_SUCCESS){
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DEBUG_ERROR, (" CMD Start to load FW failed\n"));
		return ntStatus;
	}
	return ntStatus;
}

NTSTATUS FirmwareCommnadRestartDownloadFW(
    IN  RTMP_ADAPTER   *pAdapter
    )
{
	NTSTATUS		ntStatus = STATUS_SUCCESS;
    	//NTSTATUS		FWStatus = STATUS_SUCCESS;    
	FIRMWARE_TXDSCR CmdRestartDownloadFwRequest;
	//LARGE_INTEGER       TimeOut1Second;

	//TimeOut1Second.QuadPart = -(1000* 10000);

	// Fill setting
	RtlZeroMemory(&CmdRestartDownloadFwRequest, sizeof(FIRMWARE_TXDSCR));
	CmdRestartDownloadFwRequest.PQ_ID = 0x8000; //P_IDX[31] = 0x1, Q_IDX[31 : 27] = 0x0
	CmdRestartDownloadFwRequest.CID = CMD_RESTART_DOWNLOAD; // The ID of CMD w/ target address/length request
	CmdRestartDownloadFwRequest.PktTypeID = 0xA0; //bit 15 = 1'b1, bit[14:13] = 0x1
	CmdRestartDownloadFwRequest.SeqNum= SEQ_CMD_FW_RESTART; //bit 15 = 1'b1, bit[14:13] = 0x1
	CmdRestartDownloadFwRequest.Length = sizeof(FIRMWARE_TXDSCR);

	ntStatus = FWTxCmd(pAdapter, &CmdRestartDownloadFwRequest, sizeof(FIRMWARE_TXDSCR), &pAdapter->WaitFWEvent);
	if(ntStatus != STATUS_SUCCESS){
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DEBUG_ERROR, (" CMD RestartDownloadFW failed\n"));
		return ntStatus;
	}
	pAdapter->FWMode = FWROM;

	return ntStatus;
}

NTSTATUS FirmwareScatters(
	IN  RTMP_ADAPTER *pAdapter,
	IN  PUCHAR          pFirmwareImage,
	IN  ULONG           ImageLength
	)
{
	NTSTATUS ntStatus = STATUS_SUCCESS;
	ULONG sentLen = 0, remainLen = 0, i = 0;
	ULONG MacValue=0;	
	ULONG busyCount = 0;
	PUCHAR          pCopyPointer;
	FIRMWARE_TXDSCR FwTxD;
	LARGE_INTEGER       TimeOut1Second;
	ULONG  UnitSize = SCATTER_SIZE - sizeof(FIRMWARE_TXDSCR);//14336 in 7603

	TimeOut1Second.QuadPart = -(3000* 10000);
        
	sentLen = 0;
	remainLen = ImageLength;
	pCopyPointer = pFirmwareImage;

	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DEBUG_TRACE,("### FirmwareScatters ###\n"));
	
	//
	// end out 14k bytes to Andes till end of firmware
	//      14336
    	RTMPusecDelay(10000);// 1 ms
	while (remainLen > 0)
	{		
		if(remainLen > UnitSize)
		{
			sentLen = UnitSize;  // copy 14k bytes at most
			remainLen = remainLen - UnitSize;
		}
		else
		{
			sentLen = remainLen;
			remainLen = 0;
		}
	
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DEBUG_TRACE,("copying the %d-th Kbytes....sentLen %d \n",++i, sentLen));
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DEBUG_TRACE,("Total FileLength %d len remain %d \n", ImageLength, remainLen));                
		
		RtlZeroMemory(&FwTxD, sizeof(FIRMWARE_TXDSCR));
		FwTxD.PQ_ID = 0xC000; //P_IDX[31] = 0x1, Q_IDX[30 : 27] = 0x0
		FwTxD.CID = 0xEE; // The ID of CMD w/ target address/length request
		FwTxD.PktTypeID = 0xA0; //bit 15 = 1'b1, bit[14:13] = 0x1
		FwTxD.Length = sizeof(FIRMWARE_TXDSCR) + sentLen;

		RtlZeroMemory(pAdapter->ScatterBuf, SCATTER_SIZE);
		RtlMoveMemory(pAdapter->ScatterBuf, &FwTxD, sizeof(FIRMWARE_TXDSCR));
		RtlMoveMemory(pAdapter->ScatterBuf + sizeof(FIRMWARE_TXDSCR), pCopyPointer, sentLen);		

		pCopyPointer += sentLen;
		ntStatus = FWTxCmd(pAdapter, pAdapter->ScatterBuf, sizeof(FIRMWARE_TXDSCR) + sentLen, &pAdapter->LoadFWEvent);
		if (ntStatus != STATUS_SUCCESS)
		{
			MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DEBUG_ERROR, ("!!!!! Scatter failed. !!!!!\n"));
		}
		RTMPusecDelay(1000);

	}    
	return ntStatus;
}

NTSTATUS FirmwareCommnadStartToRunFW(
	IN  RTMP_ADAPTER *pAdapter
	)
{
	NTSTATUS		ntStatus = STATUS_SUCCESS;
	//NTSTATUS		FWStatus = STATUS_SUCCESS;	
	CMD_START_FW_REQUEST    CmdStartFwRequest;
	//LARGE_INTEGER       TimeOut1Second;
	

	//TimeOut1Second.QuadPart = -(500* 10000);

	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DEBUG_TRACE,("### FirmwareCommnadStartToRunFW ###\n"));

	// Fill setting
	RtlZeroMemory(&CmdStartFwRequest, sizeof(CMD_START_FW_REQUEST));
	CmdStartFwRequest.FwTxD.PQ_ID = 0x8000; //P_IDX[31] = 0x1, Q_IDX[31 : 27] = 0x0
	CmdStartFwRequest.FwTxD.CID = CMD_RAM_START_RUN; // The ID of CMD w/ target address/length request
	CmdStartFwRequest.FwTxD.PktTypeID = 0xA0; //bit 15 = 1'b1, bit[14:13] = 0x1
	CmdStartFwRequest.FwTxD.Length = sizeof(CMD_START_FW_REQUEST);  
	CmdStartFwRequest.FwTxD.SeqNum = SEQ_CMD_FW_STARTTORUN;
	if(IS_MT7603(pAdapter))
	{
		CmdStartFwRequest.Override = 0x1;
		CmdStartFwRequest.Address = 0x00100000;//0x00100000;
	}
	else if(IS_MT76x6(pAdapter))
	{
		CmdStartFwRequest.Override = 0x0;
		CmdStartFwRequest.Address = 0x0;//Override =0 => ignore Address
	}
	ntStatus = FWTxCmd(pAdapter, &CmdStartFwRequest, sizeof(CMD_START_FW_REQUEST), &pAdapter->WaitFWEvent);
	if(ntStatus != STATUS_SUCCESS){
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DEBUG_ERROR, (" CMD Start to Run FW failed\n"));
		return ntStatus;
	}	
	RTMPusecDelay(10000);// 1 ms

	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE,("### FirmwareCommnadStartToRunFW - got CMD response ###\n"));        
        
	return ntStatus;
}

NTSTATUS FirmwareCommnadHIFLoopBackTest(
	IN  RTMP_ADAPTER *pAdapter, BOOLEAN IsEnable, UINT8 RxQ
	)
{
	NTSTATUS		ntStatus = STATUS_SUCCESS;
	CMD_ID_HIF_LOOPBACK_TEST  CmdMsg;
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DEBUG_TRACE,("### FirmwareCommnadHIFLoopBackTest ###\n"));

	// Fill setting
	RtlZeroMemory(&CmdMsg, sizeof(CMD_ID_HIF_LOOPBACK_TEST));
	CmdMsg.FwTxD.PQ_ID = 0x8000; //P_IDX[31] = 0x1, Q_IDX[31 : 27] = 0x0
	CmdMsg.FwTxD.CID = CMD_LOOPBACK_TEST; // The ID of CMD w/ target address/length request
	CmdMsg.FwTxD.PktTypeID = 0xA0; //bit 15 = 1'b1, bit[14:13] = 0x1
	CmdMsg.FwTxD.Length = sizeof(CMD_ID_HIF_LOOPBACK_TEST);  
	CmdMsg.FwTxD.SeqNum = SEQ_CMD_LOOPBACK_TEST;
	CmdMsg.Loopback_Enable = IsEnable;
	CmdMsg.DestinationQid= RxQ;
	ntStatus = FWTxCmd(pAdapter, &CmdMsg, sizeof(CMD_ID_HIF_LOOPBACK_TEST), &pAdapter->WaitFWEvent);
	if(ntStatus != STATUS_SUCCESS){
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DEBUG_ERROR, (" FirmwareCommnadHIFLoopBackTest failed\n"));
		return ntStatus;
	}	
	RTMPusecDelay(10000);// 1 ms

	//MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE,("### FirmwareCommnadHIFLoopBackTest - got CMD response ###\n"));        
        
	return ntStatus;
}

NTSTATUS ReDownloadFirmware(
	IN  RTMP_ADAPTER *pAdapter
	)
{
	NTSTATUS		ntStatus = STATUS_SUCCESS;
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DEBUG_TRACE,("%s\n", __FUNCTION__));

        
	return ntStatus;
}

NTSTATUS FWSwitchToROM(
	IN  RTMP_ADAPTER *pAd
	)
{
	NTSTATUS		ntStatus = STATUS_SUCCESS;
	UINT32			BreakCount = 0;
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DEBUG_TRACE,("%s\n", __FUNCTION__));

	if(pAd->FWMode == FWROM)
	{
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DEBUG_ERROR,("%s  FWMode is already ROM\n", __FUNCTION__));
	}
	else if(pAd->FWMode == FWRAM)
	{
		//Step 1 set firmware to ROM mode
		ntStatus = FirmwareCommnadRestartDownloadFW(pAd);
		if(ntStatus != STATUS_SUCCESS)
		{
			MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DEBUG_ERROR,("%s, RestartDownloadFW cmd failed \n",__FUNCTION__));
		}

		ntStatus = CheckFWROMWiFiSysOn(pAd);
		if(ntStatus != STATUS_SUCCESS)
		{
			MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DEBUG_ERROR,("%s, CheckFWROMWiFiSysOn  failed \n",__FUNCTION__));
		}
	}
        
	return ntStatus;
}

NTSTATUS CheckFWROMWiFiSysOn(IN  RTMP_ADAPTER   *pAd)
{
	NTSTATUS ntStatus = STATUS_SUCCESS;
#ifdef RTMP_SDIO_SUPPORT
		ntStatus = SDIOCheckFWROMWiFiSysOn(pAd);
#endif
#ifdef RTMP_USB_SUPPORT
		ntStatus = USBCheckFWROMWiFiSysOn(pAd);
#endif
	return ntStatus;
}

INT32 CmdETxBfSoundingPeriodicTriggerCtrl(RTMP_ADAPTER *pAd, UINT32 wlanidx, UINT8 On, UINT32 BW)
{
	EXT_CMD_ETXBf_SD_PERIODIC_TRIGGER_CTRL_T ETxBfSdPeriodicTriggerCtrl;
	INT32 ret = 0;

	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("%s: On = %d\n", __FUNCTION__, On));
       
	RtlZeroMemory(&ETxBfSdPeriodicTriggerCtrl, sizeof(ETxBfSdPeriodicTriggerCtrl));
	ETxBfSdPeriodicTriggerCtrl.FwTxD.CID = CMD_EXT_CMD;
	ETxBfSdPeriodicTriggerCtrl.FwTxD.Length= sizeof(ETxBfSdPeriodicTriggerCtrl);
	ETxBfSdPeriodicTriggerCtrl.FwTxD.PQ_ID= 0x8000;
	ETxBfSdPeriodicTriggerCtrl.FwTxD.PktTypeID= 0xA0;
	ETxBfSdPeriodicTriggerCtrl.FwTxD.SeqNum= SEQ_CMD_RAM_TXBF_SOUNDING;
	ETxBfSdPeriodicTriggerCtrl.FwTxD.SetQuery= 1;	
	ETxBfSdPeriodicTriggerCtrl.FwTxD.ExtCmdOption= 1;
	if (On)
	{
		ETxBfSdPeriodicTriggerCtrl.FwTxD.ExtenCID = EXT_CMD_BF_SOUNDING_START;
		ETxBfSdPeriodicTriggerCtrl.ucWlanIdx = (UINT8)wlanidx;
		ETxBfSdPeriodicTriggerCtrl.ucWMMIdx = 1;
		ETxBfSdPeriodicTriggerCtrl.ucBW = (UINT8)BW;
		ETxBfSdPeriodicTriggerCtrl.u2NDPARateCode = 2;  // MCS2
		ETxBfSdPeriodicTriggerCtrl.u2NDPRateCode = 8;      // MCS8
		ETxBfSdPeriodicTriggerCtrl.u4SoundingInterval = 500; // 500ms
	}
	else
		ETxBfSdPeriodicTriggerCtrl.FwTxD.ExtenCID = EXT_CMD_BF_SOUNDING_STOP;
	ret = FWTxCmd(pAd, &ETxBfSdPeriodicTriggerCtrl, sizeof(ETxBfSdPeriodicTriggerCtrl), &pAd->WaitFWEvent);//WaitFWEvent);
	if(ret != STATUS_SUCCESS){
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DEBUG_ERROR, (" CMD ETxBfSounding failed\n"));
		return ret;
	}
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("%s:(ret = %d)\n", __FUNCTION__, ret));
	return ret;
}