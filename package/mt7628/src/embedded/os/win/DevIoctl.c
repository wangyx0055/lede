#ifdef RTMP_SDIO_SUPPORT
#include "sdio_config.h"
#endif
#ifdef RTMP_USB_SUPPORT
#include "config.h"
#endif

NTSTATUS DevIoctlHandler(RTMP_ADAPTER *pAdapter, UINT32 inputBufferLength, PVOID ioBuffer, UINT32 outputBufferLength, PVOID outBuffer, UINT32 ioControlCode, UINT32 *BytesRet){
	NTSTATUS ntStatus = STATUS_SUCCESS;
	UINT32 InfoLength = 0;//Irp->IoStatus.Information = info;
	//BytesRet = &InfoLength;
	//outBuffer = ioBuffer;
	switch(ioControlCode)
	{
	///////	Set Start Rx0

		case IOCTL_START_RX0://turn on WHIER RX0_DONE_INT_EN
		{
			MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE,("IOCTL_START_RX0\n"));		
			ntStatus = StartRx0(pAdapter);
			InfoLength = 0;	
		}
		break;

		///////	Set Start Rx1
		case IOCTL_START_RX1://turn on WHIER RX1_DONE_INT_EN
		{
			MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE,("IOCTL_START_RX1\n"));		
			ntStatus = StartRx1(pAdapter);
			InfoLength = 0;	
		}
		break;
		///////	Set Stop Rx0
		case IOCTL_STOP_RX0://turn off WHIER RX0_DONE_INT_EN
		{			
			MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE,("IOCTL_STOP_RX0\n"));		
			ntStatus = StopRx0(pAdapter);
			InfoLength = 0;	
		}
		break;

		///////	Set Stop Rx1
		case IOCTL_STOP_RX1://turn off WHIER RX1_DONE_INT_EN
		{
			MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE,("IOCTL_STOP_RX1\n"));		
			ntStatus = StopRx1(pAdapter);
			InfoLength = 0;				
		}
		break;		
		///////	Setup Tx Packet
		case IOCTL_SETUP_TX_PACKET:// packet content
		{	
			UINT32* pInBuf = (ULONG*)ioBuffer;
			UINT32 txBufferNum = pInBuf[0];//only in USB, ignore in SDIO
			UINT32 Length = pInBuf[1]&0X0000FFFF;//refernece TX D, txbytecount
			UINT32 *pDataX = &pInBuf[1];
			UCHAR *pUc = (UCHAR *)ioBuffer;
			UINT8 cntidx;
			for( cntidx = 1; cntidx<9;cntidx++)
			{
				MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DEBUG_TRACE,("0x%x ",pInBuf[cntidx]));
			}
			

			MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DEBUG_TRACE,("%s IOCTL_SETUP_TX_PACKET\n",__FUNCTION__));
			MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DEBUG_TRACE,("%s tx length= %d\n",__FUNCTION__, Length));

			// driver must check 4 bytes alignment
			if(Length % 4 != 0)
				Length  = Length + 4 - (Length % 4);		
			InfoLength = 0;				
			ntStatus = SetupTxPacket(pAdapter, (UCHAR *)pDataX, Length, txBufferNum);	
		}
		break;

		///////	Set Start Bulk Out
		case IOCTL_START_TX:
		{
			UINT32* pInBuf = (ULONG*)ioBuffer;
			UINT32 TxRemained = pInBuf[0];	
			UINT32 txBufferNum = pInBuf[1];
			pAdapter->Seq_dbg = (BOOLEAN)pInBuf[4];
			MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DEBUG_TRACE,("%s IOCTL_START_TX\n",__FUNCTION__));
			ntStatus = StartTx(pAdapter, TxRemained, txBufferNum);
			//Tx data
			InfoLength = 0;
		}
		break;

		case IOCTL_STOP_TX:
		{			
			MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DEBUG_TRACE,("IOCTL_STOP_TX\n"));
			ntStatus = StopTx(pAdapter);
			RTMPusecDelay(1000);
			InfoLength = 0;
		}
		break;

		case IOCTL_GET_FWD_SEQNUM:
		{
			UINT8 *pOutBuf = (UINT8 *)outBuffer;
			MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE,("IOCTL_QA_GET_FW_STATUS g_FWSeqMCU= %d\n", pAdapter->PacketCMDSeqMCU));
			InfoLength = sizeof(UINT8);
			pOutBuf[0] = pAdapter->PacketCMDSeqMCU;
			ntStatus = STATUS_SUCCESS;
		}
		break;

		case IOCTL_GET_FWD_STATUS:
		{
			//UCHAR* pInBuf = (UCHAR*)ioBuffer;
			UCHAR* pOutBuf = (UCHAR*)outBuffer;
			MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DEBUG_TRACE,("IOCTL_GET_FWD_STATUS FWRspStatus= %d\n", pAdapter->FWRspStatus));
			pOutBuf[0] = pAdapter->FWRspStatus;
			InfoLength = sizeof(UINT8);			
			ntStatus = STATUS_SUCCESS;
		}
		break;

		case IOCTL_GET_PACKETCMD_SEQNUM:
		{

			UCHAR* pOutBuf = (UCHAR*)outBuffer;
			InfoLength = sizeof(UINT8);
			MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DEBUG_TRACE,("IOCTL_GET_PACKETCMD_SEQNUM PacketCMDSeqMCU= %d\n", pAdapter->PacketCMDSeqMCU));
			if(outputBufferLength >= InfoLength)
			{
				pOutBuf[0] = pAdapter->PacketCMDSeqMCU;
				//RtlCopyMemory(ioBuffer, &pAdapter->PacketCMDSeqMCU,sizeof(pAdapter->PacketCMDSeqMCU));
			}
			
			else
			{
				MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DEBUG_TRACE,("size too small= %d\n"));	
			}			
			ntStatus = STATUS_SUCCESS;
		}
		break;

		case IOCTL_GET_PACKETCMD_RESPONSE:
		{
			//UCHAR* pInBuf = (UCHAR*)ioBuffer;
			UCHAR* pOutBuf = (UCHAR*)outBuffer;
			MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DEBUG_TRACE,("IOCTL_GET_PACKETCMD_RESPONSE FWSeqMCU= %d\n", pAdapter->PacketCMDSeqMCU));
			InfoLength = pAdapter->FWRspContentLength;
			RtlCopyMemory(pOutBuf, &pAdapter->FWRspContent,pAdapter->FWRspContentLength);
			ntStatus = STATUS_SUCCESS;
		}
		break;
		//0: BufferMode, 1: Efuse, 2: EEPROM
		case IOCTL_SET_EFUSE_MODE:
		{			
			UCHAR* pInBuf = (UCHAR*)ioBuffer;
			MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DEBUG_TRACE,(" NOT SUPPORT_____________DeviceIoControl: IOCTL_SET_EFUSE_MODE\n"));

			pAdapter->EfuseMode = pInBuf[0];

			// pcie pDevExt->EfuseMode = pInBuf[0];
			InfoLength = 0;
			ntStatus = STATUS_SUCCESS;
	    	}
		break;

		case IOCTL_WRITE_EEPROM_BUFFER:
		{			
			ULONG* pInBuf = (ULONG*)ioBuffer;			
			ULONG addr = (ULONG)pInBuf[0];
			UCHAR size = (UCHAR)pInBuf[1];
			UCHAR idx = 0;
			UCHAR idx2 = 0;	
			//MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DEBUG_TRACE,("WRITE_EEPROM_BUFFER, addr = 0x%x, size = %d\n", addr, size ));
		
			if (size == 16)
			{				
				for (idx=0; idx<4; idx++)
				{
				//	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DEBUG_TRACE,("0x%x \n", pInBuf[2+idx] ));
					pAdapter->EfuseContent[addr+idx*4] = (UCHAR)(pInBuf[2+idx]);
					pAdapter->EfuseContent[addr+idx*4+1] = (UCHAR)(pInBuf[2+idx] >> 8);
					pAdapter->EfuseContent[addr+idx*4+2] = (UCHAR)(pInBuf[2+idx] >> 16);
					pAdapter->EfuseContent[addr+idx*4+3] = (UCHAR)(pInBuf[2+idx] >> 24);
					//MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DEBUG_TRACE,("EfuseContent[0x%x] = 0x%x \n", addr+idx*4,  (UCHAR)(pInBuf[2+idx] )));
					//MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DEBUG_TRACE,("EfuseContent[0x%x] = 0x%x \n", addr+idx*4+1, (UCHAR)(pInBuf[2+idx] >> 8)));
					//MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DEBUG_TRACE,("EfuseContent[0x%x] = 0x%x \n", addr+idx*4+2, (UCHAR)(pInBuf[2+idx] >> 16)));
					//MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DEBUG_TRACE,("EfuseContent[0x%x] = 0x%x \n", addr+idx*4+3, (UCHAR)(pInBuf[2+idx] >> 24)));
				}
				
			}
			else
			{
				MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DEBUG_TRACE,("size != 16 \n"));				
			}
			InfoLength = 0;
			ntStatus = STATUS_SUCCESS;
		}
				
		break;
		
		//(write Firmware image to driver) 
		case IOCTL_WRITE_FW:
		{
			//pInBuf[0]: address
			//pInBuf[1]: size
			//pInBuf[2]: buffer
			ULONG* pInBuf = (ULONG*)ioBuffer;			
			ULONG addr = (ULONG)pInBuf[0];
			ULONG size = (ULONG)pInBuf[1];
			ULONG idx = 0;			
			//MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DEBUG_TRACE,("WRITE FW image, addr = 0x%x, size = %d\n", addr, size ));
			if(pAdapter->FWSize + size > MAXFIRMWARESIZE)
			{
				pAdapter->IsUISetFW = FALSE;
				ntStatus = STATUS_INSUFFICIENT_RESOURCES;
			}
			else
			{		
				pAdapter->FWSize += size;
				if (size == 16)
				{					
					for (idx=0; idx<4; idx++)
					{
						//MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DEBUG_TRACE,("0x%x \n", pInBuf[2+idx] ));						
						pAdapter->FWImage[addr+idx*4] = (UCHAR)(pInBuf[2+idx]);
						pAdapter->FWImage[addr+idx*4+1] = (UCHAR)(pInBuf[2+idx] >> 8);
						pAdapter->FWImage[addr+idx*4+2] = (UCHAR)(pInBuf[2+idx] >> 16);
						pAdapter->FWImage[addr+idx*4+3] = (UCHAR)(pInBuf[2+idx] >> 24);				
					}
				}
				else
				{			
					ULONG RemainSize = size;
					MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DEBUG_TRACE,("size != 16 sholud be last data, %d bytes, addr = 0x%x\n", size, addr));
					idx = 0;
					do
					{
						
						if(RemainSize > 4)
						{
							pAdapter->FWImage[addr+idx*4] = (UCHAR)(pInBuf[2+idx]);
							pAdapter->FWImage[addr+idx*4+1] = (UCHAR)(pInBuf[2+idx] >> 8);
							pAdapter->FWImage[addr+idx*4+2] = (UCHAR)(pInBuf[2+idx] >> 16);
							pAdapter->FWImage[addr+idx*4+3] = (UCHAR)(pInBuf[2+idx] >> 24);				
							RemainSize -= 4;
							idx++;
						}
						else
						{
							UCHAR Count = 0;
							for(;Count<RemainSize;Count++)
							{
								pAdapter->FWImage[addr+idx*4+Count] = (UCHAR)(pInBuf[2+idx] >> Count*8);
								MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DEBUG_TRACE,("Last %dth byte = 0x%x\n", RemainSize-Count,pAdapter->FWImage[addr+idx*4+Count]));
							}
							RemainSize = 0;
						}
					}while(RemainSize > 0);
					for(idx = 0; idx<size; idx++)
					{
						MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DEBUG_TRACE,("Last %dth byte = 0x%x\n", size-idx,pAdapter->FWImage[addr+idx]));
					}
				}
				pAdapter->IsUISetFW = TRUE;
				ntStatus = STATUS_SUCCESS;
			}
			InfoLength = 0;

		}
		break;
		case IOCTL_CLEAR_FW:
		{			
			pAdapter->FWSize = 0;
			RtlZeroMemory(&pAdapter->FWImage, MAXFIRMWARESIZE);
			InfoLength = 0;
			ntStatus = STATUS_SUCCESS;
		}
		break;
		//(Trigger FW download) 
		
		case IOCTL_DOWNLOAD_FW:
		{
			ULONG* pInBuf = (ULONG*)ioBuffer;
			ULONG FWsize = (ULONG)pInBuf[0];
			ULONG Type = (ULONG)pInBuf[1];
			StartRx0(pAdapter);
			if(pAdapter->FWon == TRUE)
			{
				MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("FW is on\n"));
				ntStatus = STATUS_SUCCESS;
			}
			else if(!pAdapter->IsFWImageInUse && ((FWsize == pAdapter->FWSize)  || !(pAdapter->IsUISetFW)))
			{//make sure FW size is correct, or IsUISetFW to check if the fw has been set
				pAdapter->IsFWImageInUse = TRUE;
				switch(Type)
				{
					case MT7603_RAM:
					{
						if(!IS_MT7603(pAdapter))
						{
							ntStatus = STATUS_UNSUCCESSFUL;
							MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DEBUG_ERROR,("Load 7603 RAM but not 7603!!!\n"));							
						}
						else
						{
							MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DEBUG_TRACE,("IOCTL load 7603 RAM\n"));
							ntStatus = NICLoadFirmware(pAdapter);
						}
					}
					break;
					case MT7636_RAM:
					{					
						if(!IS_MT76x6(pAdapter))
						{
							ntStatus = STATUS_UNSUCCESSFUL;
							MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DEBUG_ERROR,("Load 7636 RAM but not 7636!!!\n"));
						}
						else
						{
							MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DEBUG_TRACE,("IOCTL load 7636 RAM\n"));
							ntStatus = NICLoadFirmware(pAdapter);
							pAdapter->FWon = TRUE;
							//need to write CR after load 7636 FW
							ResetCounter(pAdapter);
							mt7603_DoOtherConfig(pAdapter);
						}
					}
					break;
					case MT7636_ROM_PATCH:
					{
						if(!IS_MT76x6(pAdapter))
						{
							ntStatus = STATUS_UNSUCCESSFUL;
							MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DEBUG_ERROR,("Load 7636 ROM Patch but not 7636!!!\n"));							
						}
						else
						{
							MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DEBUG_TRACE,("IOCTL load 7636 ROM\n"));
							ntStatus = NICLoadRomPatch(pAdapter);							
						}
					}
					break;
					default:
						MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DEBUG_WARN,("Type not implemented, type = %d\n", Type));
						ntStatus = STATUS_NOT_IMPLEMENTED;
				}
				pAdapter->IsFWImageInUse = FALSE;
			}
			else
			{
				ntStatus = STATUS_INSUFFICIENT_RESOURCES;
				MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DEBUG_ERROR,("FW size does not match, command size = %d, image size = %d!!!\n", FWsize, pAdapter->FWSize));
			}
			StopRx0(pAdapter);
			//Clear Image
			RtlZeroMemory(&pAdapter->FWImage, MAXFIRMWARESIZE);
			pAdapter->FWSize = 0;
			InfoLength = 0;
		}
		break;
//_SDIO		
#ifdef RTMP_SDIO_SUPPORT
		case IOCTL_SDIO_FBR_ACCESS_READ:
		{
			ULONG* pInBuf = (ULONG*)ioBuffer;
			//~~~~~~~~~
			//ULONG* pOutBuf = (ULONG*)ioBuffer;
			ULONG* pOutBuf = (ULONG*)outBuffer;
			ULONG HighByte = pInBuf[0];				//addr
			ULONG LowByte =  pInBuf[1];
			//ULONG Length =  pInBuf[2];

			//ULONG Offset = pInBuf[0];
			//MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE,"%s: IOCTL_USBQA_USB_MAC_READ_LONG47\n",__FUNCTION__);
			//USBHwHal_ReadMacRegister47(deviceExtension, (USHORT)Offset, &pOutBuf[0]);
			
			ntStatus = RTMP_SDIO_READ32(pAdapter, HighByte<<16 |LowByte, &pOutBuf[0]);
			MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DEBUG_TRACE,("%s: IOCTL_SDIO_FBR_ACCESS_READ offset=0x%04x, Value=0x%08x\n",__FUNCTION__, HighByte<<16 |LowByte, pOutBuf[0]));
			InfoLength = sizeof(UINT32);
	    	}
		break;
//_SDIO
		case IOCTL_SDIO_FBR_ACCESS_WRITE:
		{
			UINT32* pInBuf = (UINT32*)ioBuffer;
			//~~~~~~~~~	
			ULONG HighByte = pInBuf[0];				//addr
			ULONG LowByte =  pInBuf[1];
			ULONG Value =  pInBuf[2];

			//UINT32 Offset = pInBuf[0];
			//UINT32 Value = pInBuf[1];
			//ULONG Length =  pInBuf[2];

			MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DEBUG_TRACE,("%s: IOCTL_SDIO_FBR_ACCESS_WRITE offset=0x%04x, Value=0x%08x\n",__FUNCTION__, HighByte<<16 |LowByte, Value));

			//USBHwHal_ReadMacRegister63(deviceExtension, (USHORT)HighByte,(USHORT)LowByte, &pOutBuf[0],Length);
			ntStatus = RTMP_SDIO_WRITE32(pAdapter, HighByte<<16 |LowByte, Value);
			InfoLength = 0;		

	    	}
		break;
#endif		
		case IOCTL_MAC_READ_LONG47:
		{
#if 0			
			ULONG* pInBuf = (ULONG*)ioBuffer;
			//~~~~~~~~~
			//ULONG* pOutBuf = (ULONG*)ioBuffer;
			ULONG* pOutBuf = (ULONG*)outBuffer;

			ULONG Offset = pInBuf[0];					//addr
			MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DEBUG_TRACE,("%s: IOCTL_MAC_READ_LONG47\n",__FUNCTION__));
			InfoLength = 0;
			//USBHwHal_ReadMacRegister47(deviceExtension, (USHORT)Offset, &pOutBuf[0]);		
			//RTMP_IO_READ32(pAdapter, Offset,  &pOutBuf[0]);

			//Cmd_SDIO_Read_Reg(pAdapter, Offset,  &pOutBuf[0],FALSE);
			RTMP_IO_READ32(pAdapter, Offset,  &pOutBuf[0]);
			InfoLength += 4;
#endif			
			ntStatus = STATUS_SUCCESS;
	    	}
		break;

		case IOCTL_MAC_READ_LONG63:
		{
			ULONG* pInBuf = (ULONG*)ioBuffer;
			ULONG* pOutBuf = (ULONG*)outBuffer;

			ULONG HighByte = pInBuf[0];				//addr
			ULONG LowByte =  pInBuf[1];
			ULONG Length =  pInBuf[2];
			InfoLength = 0;
			MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DEBUG_TRACE,("%s: IOCTL_MAC_READ_LONG63 pInBuf[0]=0x%08x, HighByte=0x%08x, LowByte=0x%08x\n",__FUNCTION__,pInBuf[0],HighByte,LowByte));
			//multi check
			if(Length!= 4)
				MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DEBUG_TRACE,("%s Length = %d\n",__FUNCTION__, Length));
#ifdef RTMP_SDIO_SUPPORT			
			if(HighByte == 0xD000)
			{
				if(LowByte == 0)
				{
					*pOutBuf = pAdapter->TxPageCount;
					MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DEBUG_TRACE,("%s TxPageCount = %d\n",__FUNCTION__, pAdapter->TxPageCount));
				}
			}
			else
#endif					
			{
				ntStatus = ReadReg(pAdapter, HighByte<<16 |LowByte,  &pOutBuf[0], Length);
			}
		
			InfoLength += Length;
			
			//ntStatus = STATUS_SUCCESS;
	    	}
		break;


		case IOCTL_MAC_WRITE_LONG66:
		{
			ULONG* pInBuf = (ULONG*)ioBuffer;
//			ULONG* pOutBuf = (ULONG*)ioBuffer;

			ULONG HighByte = pInBuf[0];					//addr
			ULONG LowByte =  pInBuf[1];
		
			ULONG Value = pInBuf[2];				
			InfoLength = 0;
			MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DEBUG_TRACE,("%s: IOCTL_MAC_WRITE_LONG66 pInBuf[0]=0x%08x, HighByte=0x%08x, LowByte=0x%08x\n",__FUNCTION__,pInBuf[0],HighByte,LowByte));
#ifdef RTMP_USB_SUPPORT
			if(HighByte == 0xC000)
			{
				if(LowByte == 0x0)
				{
					//StopTx(pAdapter);
					StopRx1(pAdapter);
					//StopRx0(pAdapter);
					SubmitIdleRequestIrp(pAdapter);
				}
				else if(LowByte == 0x4)
					CancelSelectSuspend(pAdapter);
				else if(LowByte == 0x8)
					StartRx0(pAdapter);
				else if(LowByte == 0xc)
					StartRx1(pAdapter);
				else if(LowByte == 0x10)
					StopRx0(pAdapter);
				else if(LowByte == 0x14)
					StopRx1(pAdapter);
				else if(LowByte == 0x18)
				{
					MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_WARN,("%s  pAdapter->bRxEnable[0] = %d \n",__FUNCTION__,pAdapter->bRxEnable[0])); 
				}
				else if(LowByte == 0x1c)
				{
					MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_WARN,("%s  @#@# bBulkOutRunning 1 = %d \n",__FUNCTION__, pAdapter->bBulkOutRunning[0])); 
					MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_WARN,("%s  @#@# bBulkOutRunning 2 = %d \n",__FUNCTION__, pAdapter->bBulkOutRunning[1])); 
					MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_WARN,("%s  @#@# bBulkOutRunning 3 = %d \n",__FUNCTION__, pAdapter->bBulkOutRunning[2])); 
					MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_WARN,("%s  @#@# bBulkOutRunning 4 = %d \n",__FUNCTION__, pAdapter->bBulkOutRunning[3])); 
					MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_WARN,("%s  @#@# bBulkOutRunning 5 = %d \n",__FUNCTION__, pAdapter->bBulkOutRunning[4])); 
					MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_WARN,("%s  @#@# bBulkOutRunning 6 = %d \n",__FUNCTION__, pAdapter->bBulkOutRunning[5])); 
				}
				else if(LowByte == 0x20)
				{
					MT_SetATETxBfProc(pAdapter, 3, 1);
				}
				else if(LowByte == 0x24)
				{
					FirmwareCommnadRestartDownloadFW(pAdapter);
				}
				else if(LowByte == 0x2c)
				{
					UINT32 IsChanged = FALSE;
					IsPfmuChanged(pAdapter, BW_20, &IsChanged);
					MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_WARN,("%s  IsChanged = %d \n",__FUNCTION__, IsChanged)); 
				}
				else if(LowByte == 0x28)
				{
					RSSI_DATA test;
					GetAvgRxvRSSI(pAdapter, &test);
				}
				
			}
			else
#endif				
				ntStatus = WriteReg(pAdapter, HighByte<<16 |LowByte,  pInBuf[2]);
			
			//ntStatus = STATUS_SUCCESS;
		}
		break;
		case IOCTL_HAL_CMD:
		{
			ULONG* pInBuf = (ULONG*)ioBuffer;
			ULONG* pOutBuf = (ULONG*)outBuffer;
			ULONG BytesTxd = 0;
			ULONG iCmd = pInBuf[0];					//addr
			ULONG iType = pInBuf[1];
			//ULONG Length = pInBuf[1];					//length
			ULONG temp = 0;
			UCHAR TempChar = 0;
			int i;

			MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DEBUG_TRACE,("iCmd = %d\n", iCmd));
			
			switch(iCmd)
			{			
				case CMD_ASIC_TOPINIT:
					MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DEBUG_TRACE,("CMD_ASIC_TOPINIT\n"));
					MtAsicTOPInit(pAdapter);
					break;

				case CMD_HIF_INIT:
					MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DEBUG_TRACE,("CMD_HIF_INIT\n"));
					mt_hif_sys_init(pAdapter);
					break;

				case CMD_TMAC_INIT:
					MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DEBUG_TRACE,("CMD_TMAC_INIT\n"));
					asic_set_tmac_info_template(pAdapter);
					break;

				case CMD_INITMAC:
					MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DEBUG_TRACE,("CMD_INITMAC\n"));
					mt7603_init_mac_cr(pAdapter);					
					break;

				case CMD_CONTI_TX_START:
					StartContinuesTXTest(pAdapter, (UCHAR)pInBuf[1], (UCHAR)pInBuf[2], (UCHAR)pInBuf[3], (UCHAR)pInBuf[4], (UCHAR)pInBuf[5]);
					break;

				case CMD_CONTI_TX_STOP:
					StopContinuTXTest(pAdapter);
					break;

				case CMD_CONTI_TONE_START:
					StartTXToneTest(pAdapter,pInBuf[1]);
					break;

				case CMD_CONTI_TONE_STOP:
					StopContinuTXTest(pAdapter);
					break;

				case CMD_GET_MAC_MDRDY:
					GetStaticNumber(pAdapter,&temp);
					RtlCopyMemory(pOutBuf, &temp,sizeof(temp));

					BytesTxd = 4;	
					break;

				case  CMD_SET_TXPOWER:
					MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DEBUG_TRACE,("%d ,%d ,%d \n",pInBuf[1], pInBuf[2], pInBuf[3]));
					TxSetFramePower(pAdapter, pInBuf[1], pInBuf[2], (UCHAR)pInBuf[3]);
					break;

				case CMD_SET_AIFS://(IPG)
					for(i=AIFS_0;i<=AIFS_14;i++)//set all rings
					{
						SetAIFS(pAdapter,i,pInBuf[1]);
					}
					break;

				case CMD_FREQ_OFFSET:
					TxSetFrequencyOffset (pAdapter, pInBuf[1]);
					break;

				case CMD_TXPOWER_RANGE:
					TxConfigPowerRange(pAdapter, (UCHAR)pInBuf[1],(UCHAR) pInBuf[2]);
					break;				

				case CMD_GET_DRIVER_RX_COUNT:
					MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DEBUG_TRACE,("CMD_GET_DRIVER_RX_COUNT \n"));
					RtlCopyMemory(pOutBuf, &pAdapter->OtherCounters.Rx0ReceivedCount,sizeof(pAdapter->OtherCounters.Rx0ReceivedCount));
					BytesTxd = sizeof(pAdapter->OtherCounters.Rx0ReceivedCount);
					break;

				case CMD_GET_RXSNR:
					pOutBuf[0] = pAdapter->OtherCounters.SNR0;
					pOutBuf[1] = pAdapter->OtherCounters.SNR1;
					BytesTxd = 8;
					break;

				case CMD_GET_RCPI:
					MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE,("RCPI0 %d ,RCPI1 %d\n", pAdapter->OtherCounters.RCPI0, pAdapter->OtherCounters.RCPI1));
					pOutBuf[0] = pAdapter->OtherCounters.RCPI0;
					pOutBuf[1] = pAdapter->OtherCounters.RCPI1;
					BytesTxd = 8;
					break;

				case CMD_GET_FREQOFFSETFROM_RX:
					MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE,("FreqOffstFromRX %d \n", pAdapter->OtherCounters.FreqOffstFromRX));
					pOutBuf[0] = pAdapter->OtherCounters.FreqOffstFromRX;
					BytesTxd = 4;
					break;

				case CMD_GET_TX_COUNT:
					MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE,("CMD_GET_TX_COUNT %d \n", pAdapter->OtherCounters.Ac0TxedCount));
					pOutBuf[0] = pAdapter->OtherCounters.Ac0TxedCount;
					BytesTxd = sizeof(pAdapter->OtherCounters.Ac0TxedCount);
					break;

				case CMD_SET_TSSI:
					MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE,("CMD_SET_TSSI\n"));
					SetTSSI(pAdapter, pInBuf[1], (UCHAR)pInBuf[2]);
					BytesTxd = 0;
					break;	
				
				case CMD_SET_DPD:
					MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE,("CMD_SET_DPD\n"));
					SetDPD(pAdapter, pInBuf[1], (UCHAR)pInBuf[2]);
					BytesTxd = 0;
					break;	

				case CMD_GET_MACFCSERRORCNT_COUNT:
					GetMACFCSErrCnt(pAdapter, &pOutBuf[0]);
					BytesTxd = 4;
					break;

				case CMD_GET_PHY_MDRDYCOUNT:
					GetPHYMdrdyErrCnt(pAdapter, &pOutBuf[0], &pOutBuf[1]);
					BytesTxd = 8;
					break;

				case CMD_GET_PHY_FCSERRCOUNT:
					GetPHYFCSErrCnt(pAdapter, &pOutBuf[0], &pOutBuf[1]);
					BytesTxd = 8;
					break;

				case CMD_GET_PD:
					GetPhyPD(pAdapter, &pOutBuf[0], &pOutBuf[1]);
					BytesTxd = 8;
					break;

				case CMD_GET_CCK_SIG_SFD:
					GetPhyCCK_SIG_SFD(pAdapter, &pOutBuf[0], &pOutBuf[1]);
					BytesTxd = 8;
					break;

				case CMD_GET_OFDM_ERR:
					GetPhyOFDMErr(pAdapter, &pOutBuf[0], &pOutBuf[1]);
					BytesTxd = 8;
					break;

				case CMD_GET_RSSI:
					GetPhyRSSI(pAdapter, &pOutBuf[0], &pOutBuf[1], &pOutBuf[2], &pOutBuf[3]);
					BytesTxd = 32;
					break;
				
				case CMD_TMR_SETTING:
					SetTMR(pAdapter, (UCHAR)pInBuf[1]); 
					BytesTxd = 0;
					break;

				case CMD_GET_SECURITY_RESULT:
					pOutBuf[0] = pAdapter->SecurityFail;
					BytesTxd = 4;
					break;

				case CMD_GET_RXV_RSSI:
					GetRxvRSSI(pAdapter, &pInBuf[0], &pInBuf[1]);
					BytesTxd = 8;
					break;
				case CMD_READ_TXBF_PROFILETAG:
				{
					PFMU_PROFILE	*prof = (PFMU_PROFILE *)outBuffer;					
					TxBfProfileTagRead(pAdapter, (UCHAR)pInBuf[1], prof);
					BytesTxd = sizeof(PFMU_PROFILE);
				}
					break;
				case CMD_WRITE_TXBF_PROFILETAG:
				{
					PFMU_PROFILE	*prof = (PFMU_PROFILE *)&pInBuf[2];
					TxBfProfileTagWrite(pAdapter, (UCHAR)pInBuf[1], prof);
					BytesTxd = 0;
				}
					break;
				case CMD_READ_TXBF_PROFILEDATA:
				{
					PFMU_DATA *pPfmuD = (PFMU_DATA *)outBuffer;
					TxBfProfileDataRead(pAdapter, (UCHAR)pInBuf[1], (UCHAR)pInBuf[2], pPfmuD);
					BytesTxd = sizeof(PFMU_DATA);
				}
					break;
				case CMD_WRITE_TXBF_PROFILEDATA:
				{
					PFMU_DATA *pPfmuD = (PFMU_DATA *)&pInBuf[3];
					TxBfProfileDataWrite(pAdapter, (UCHAR)pInBuf[1], (UCHAR)pInBuf[2], pPfmuD);
					BytesTxd = 0;
				}	
					break;
				case CMD_VALID_TXBF_PROFILETAG:
					TxBfProfileTagValid(pAdapter, (UCHAR)pInBuf[1], (UCHAR)pInBuf[2]);
					BytesTxd = 0;
					break;	
				case CMD_WRITEALL_TXBF_PROFILEDATA:
				{
					PFMU_DATA *pPfmuD = (PFMU_DATA *)&pInBuf[4];
					TxBfProfileDataWriteAll(pAdapter, (UCHAR)pInBuf[1], (UCHAR)pInBuf[2], (UCHAR)pInBuf[3], pPfmuD);
					BytesTxd = 0;
				}
					break;
				case CMD_READALL_TXBF_PROFILEDATA:
				{
					PFMU_DATA *pPfmuD = (PFMU_DATA *)outBuffer;
					TxBfProfileDataReadAll(pAdapter,  (UCHAR)pInBuf[1],  (UCHAR)pInBuf[2],  (UCHAR)pInBuf[3], pPfmuD, &BytesTxd);
				}
					break;
				case CMD_TXBF_PROFILETAG_MCS:
					TxBfProfileTag_MCS(pAdapter,  (UCHAR)pInBuf[1],  (UCHAR)pInBuf[2]);
					BytesTxd = 0;
					break;
				case CMD_TXBF_PROFILETAG_MATRIX:
					TxBfProfileTag_Matrix(pAdapter,  (UCHAR)pInBuf[1],  (UCHAR)pInBuf[2],  (UCHAR)pInBuf[3],  (UCHAR)pInBuf[4],  (UCHAR)pInBuf[5]);
					BytesTxd = 0;
					break;
				case CMD_TXBF_PROFILETAG_SNR:
					TxBfProfileTag_SNR(pAdapter,  (UCHAR)pInBuf[1],  (UCHAR)pInBuf[2]);
					BytesTxd = 0;
					break;	
				case CMD_TXBF_PROFILETAG_TXSCALE:
					TxBfProfileTag_TxScale(pAdapter,  (UCHAR)pInBuf[1],  (UCHAR)pInBuf[2],  (UCHAR)pInBuf[3],  (UCHAR)pInBuf[4]);
					BytesTxd = 0;
					break;
				case CMD_TXBF_PROFILETAG_MAC:
					TxBfProfileTag_MAC(pAdapter,  (UCHAR *)&pInBuf[1]);
					BytesTxd = 4;
					break;
				case CMD_TXBF_PROFILETAG_FLG:
					TxBfProfileTag_Flg(pAdapter,  (UCHAR)pInBuf[1],  (UCHAR)pInBuf[2],  (UCHAR)pInBuf[3],  (UCHAR)pInBuf[4]);
					BytesTxd = 0;
					break;
				case CMD_TXBF_BYPASS_MAC:
				{
					TxBfBypassMacAddress(pAdapter,  (BOOLEAN)pInBuf[1]);
					BytesTxd = 0;					
				}
					break;
				case CMD_TXBF_POWER_NORMALIZE:
				{
					TxBFPowerNormalize(pAdapter,  (BOOLEAN)pInBuf[1]);
					BytesTxd = 0;	
				}
					break;
				case CMD_TXBF_ENABLE:
				{
					//pInBuf[1]: TxbfEnFlg
					//pInBuf[2]: BW
					MT_SetATETxBfProc(pAdapter, (UINT8)pInBuf[1], pInBuf[2]);
					BytesTxd = 0;	
				}
					break;
				case CMD_LOOPBACK_START:	
				{
					LoopBack_Start(pAdapter, (LOOPBACK_SETTING *) &pInBuf[1]);										
					BytesTxd = 0;
				}
					break;
				case CMD_LOOPBACK_STOP:
					MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DEBUG_TRACE,("%s, LoopBack_Stop\n",__FUNCTION__));
										
					pAdapter->LoopBackRunning = FALSE;
					pAdapter->LoopBackResult.Status = PASS;
					pAdapter->LoopBackResult.FailReason= NO_ERROR;
					LoopBack_Stop(pAdapter);
					
					BytesTxd = 0;
					break;
				case CMD_LOOPBACK_STATUS:
					LoopBack_Status(pAdapter, (LOOPBACK_RESULT *)&pOutBuf[0]);					
					BytesTxd = sizeof(LOOPBACK_RESULT);
					break;
				case CMD_LOOPBACK_RAWDATA:
				{
					UINT32 Length = outputBufferLength;
					BOOLEAN IsTx = (BOOLEAN)pInBuf[1];
					LoopBack_RawData(pAdapter, &Length, IsTx, (UCHAR *)outBuffer);
					BytesTxd = Length;
				}
					break;
				case CMD_LOOPBACK_EXPECTRX:
				{					
					UINT32 Length = pInBuf[1];					
					MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DEBUG_TRACE,("%s, CMD_LOOPBACK_EXPECTRX Length = %d\n",__FUNCTION__, Length));
					
					MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DEBUG_TRACE,("%s, pInBuf[1] = %d, pInBuf[2] = %d\n",__FUNCTION__, pInBuf[1], pInBuf[2]));
					LoopBack_ExpectRx(pAdapter, Length, (UCHAR *)ioBuffer + 2*sizeof(UINT32));
					BytesTxd = 0;
				}
					break;
				case CMD_LOOPBACK_EXPECTTX:
				{					
					UINT32 Length = pInBuf[1];
					MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DEBUG_TRACE,("%s, CMD_LOOPBACK_EXPECTTX Length = %d\n",__FUNCTION__, Length));
					MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DEBUG_TRACE,("%s, pInBuf[1] = %d, pInBuf[2] = %d\n",__FUNCTION__, pInBuf[1], pInBuf[2]));
					LoopBack_ExpectTx(pAdapter, Length, (UCHAR *)ioBuffer + 2*sizeof(UINT32));
					BytesTxd = 0;
				}
					break;
				case CMD_GET_RXV_RSSI_AVG:
				{
					GetAvgRxvRSSI(pAdapter, (RSSI_DATA *)&pOutBuf[0]);
					BytesTxd = sizeof(RSSI_DATA);
				}
					break;
				case CMD_SET_RXV_RSSI_LIMIT:
				{
					SetRxvRSSILimit(pAdapter, pInBuf[1]);
					BytesTxd = 0;
				}
					break;
				case CMD_IS_PFMU_CHANGE:
				{					
					IsPfmuChanged(pAdapter, pInBuf[1], &pOutBuf[0]);
					BytesTxd = 4;
				}
					break;	
				case CMD_IS_BFPROF_STORE:
				{					
					IsBFProfileStore(pAdapter, pInBuf[1]);
					BytesTxd = 0;
				}					
					break;	
//loop back				
				default:
					MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DEBUG_WARN,("Unhandle ioctlcode\n"));
					ntStatus = STATUS_INVALID_DEVICE_REQUEST;

			}				

			InfoLength= BytesTxd;
		}
		break;
		case IOCTL_RESET_TXRX_COUNTER:
			MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE,("%s IOCTL_RESET_TXRX_COUNTER\n",__FUNCTION__));
			ResetCounter(pAdapter);
			RtlZeroMemory(&pAdapter->OtherCounters, sizeof(OTHER_STATISTICS));
			pAdapter->SecurityFail = 0;
			ntStatus = STATUS_SUCCESS;
			break;			
	
		default:
		{
			MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_WARN,("Unhandle ioctlcode\n"));
			ntStatus = STATUS_INVALID_DEVICE_REQUEST;
		}
	}
	if(InfoLength <= outputBufferLength)
	{
		*BytesRet = InfoLength;
		//MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DEBUG_TRACE,("bytes return = %d\n", *BytesRet));
	}
	else
	{
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DEBUG_ERROR,("outputBufferLength = %d < BytesRet = %d\n", outputBufferLength, *BytesRet));
		ntStatus = STATUS_INSUFFICIENT_RESOURCES;
	}
	
	return ntStatus;
}
