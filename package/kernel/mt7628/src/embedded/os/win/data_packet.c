#ifdef RTMP_SDIO_SUPPORT
#include "sdio_config.h"
#endif
#ifdef RTMP_USB_SUPPORT
#include "config.h"
#endif
#include <Ntstrsafe.h>

//pData id rx packet head.
VOID CheckSecurityResult(
	IN PULONG   pData,
	IN	RTMP_ADAPTER *pAdapter
	)
{
	RX_INFO_DW0 RxInfoDW0;
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DEBUG_TRACE,("%s --->>\n",__FUNCTION__));
	
	memcpy(&RxInfoDW0,pData,sizeof(RX_INFO_DW0));
	if(RxInfoDW0.RxByteCount<=12)
	{
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DEBUG_TRACE,("%s, RxByteCount = %d, <12, return !! \n",__FUNCTION__,RxInfoDW0.RxByteCount));
		return;
	}	

	if(RxInfoDW0.PKT_TYPE==RX_PKT_TYPE_RX_NORMAL || RxInfoDW0.PKT_TYPE==RX_PKT_TYPE_RX_DUP_FRB)
	{
		
		if(*(pData+2) & 0x007C00FF)
		{
			MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DEBUG_TRACE,("SecurityFail\n"));
			if(*(pData+7) & 0x00000000)
			{//for SLT security, only set source mac 00:00:00:00:00:00
				MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DEBUG_TRACE,("Set SecurityFail on\n"));
				pAdapter->SecurityFail = TRUE;
			}
			else
			{
				MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DEBUG_TRACE,("from %08X ,%08X\n",*(pData+6),*(pData+7)));//print source MAC address
				MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DEBUG_TRACE,("Don't set SecurityFail\n"));
			}
		}
	}
	else
	{
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DEBUG_TRACE,("%s, RxInfoDW0.PKT_TYPE = %d,  don't check SecurityFail !! \n",__FUNCTION__,RxInfoDW0.PKT_TYPE));
	}

	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DEBUG_TRACE,("%s ---<<\n",__FUNCTION__));
}

VOID RxPacket(
	IN UINT8   *pData,
	IN RTMP_ADAPTER *pAdapter,
	IN UINT32  pktlen
	)
{
	UINT32 *pHeaderDW0 = (UINT32 *)pData;
	KIRQL Irql;

	CheckSecurityResult((UINT32 *)pData,pAdapter);
//Loopback
	if(pAdapter->LoopBackRunning||pAdapter->LoopBackWaitRx)
	{
		UINT32 LPLength = 0;
		INT32 TotalLength = (INT32)pktlen;
		UINT8  *ptr = pData;
		FwCMDRspTxD_STRUC FwCMDRspTxD;	
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE,("Driver Rx LoopBackRunning\n"));
		RtlCopyMemory(&FwCMDRspTxD,ptr,sizeof(FwCMDRspTxD));
		LPLength = FwCMDRspTxD.FwEventTxD.u2RxByteCount;
		if( LPLength >LOOPBACK_SIZE)
		{
			LPLength = LOOPBACK_SIZE;
			MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DEBUG_TRACE,("%s, max length is %d\n",__FUNCTION__, LOOPBACK_SIZE));
		}
		
		//do
		//{
			//UINT32 padding = 4;
/*			
			RtlCopyMemory(&FwCMDRspTxD,ptr,sizeof(FwCMDRspTxD));
			MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DEBUG_TRACE,("%s, Total Length = %d, LPLength is %d, bytecount = %d\n",__FUNCTION__, TotalLength, LPLength, FwCMDRspTxD.FwEventTxD.u2RxByteCount));
			LPLength = FwCMDRspTxD.FwEventTxD.u2RxByteCount;
			pAdapter->LoopBackResult.RxByteCount += LPLength;
			pAdapter->LoopBackResult.RxPktCount ++;
			//if(LPLength % 4 !=0)
			//	padding = 4 + 4 - LPLength % 4;
			RtlCopyMemory(&pAdapter->LoopBackRxRaw, ptr, LPLength);
			pAdapter->LoopBackRxRawLen = LPLength;			
*/			
			//TotalLength = TotalLength - LPLength - padding;
			//MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DEBUG_TRACE,("%s, Total Length = %d, padding is %d\n",__FUNCTION__, TotalLength, padding));
			//ptr += LPLength + padding;
		//}while(TotalLength>0);
#ifdef RTMP_USB_SUPPORT			//Rx Agg
		if(TotalLength > (INT32)LPLength && pAdapter->LoopBackSetting.IsDefaultPattern && pAdapter->LoopBackSetting.RxAggEnable)
		{
			INT32 padding = 4;
			UINT32 RawLength = 0;
			UINT8 *Rawptr = pAdapter->LoopBackRxRaw;
			TotalLength += 4;
			do{
				RtlCopyMemory(&FwCMDRspTxD,ptr,sizeof(FwCMDRspTxD));
				LPLength = FwCMDRspTxD.FwEventTxD.u2RxByteCount;
				if(LPLength % 4 !=0)
					padding = 4 + 4 - (INT32)LPLength % 4;
				pAdapter->LoopBackResult.RxByteCount += LPLength;
				pAdapter->LoopBackResult.RxPktCount ++;
				RtlCopyMemory(Rawptr, ptr, LPLength);
				TotalLength = TotalLength - (INT32)LPLength - padding;				
				RawLength += LPLength;
				ptr += LPLength + padding;
				Rawptr += LPLength;
				MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DEBUG_TRACE,("%s, Total Length = %d, padding is %d\n",__FUNCTION__, TotalLength, padding));
			}while(TotalLength>0);
			MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DEBUG_TRACE,("%s, RawLength = %d\n",__FUNCTION__, RawLength));
			pAdapter->LoopBackRxRawLen = RawLength;
		}
		else
#endif			
		{
			if(!pAdapter->LoopBackSetting.IsDefaultPattern)
				LPLength = TotalLength+4;
			//RtlCopyMemory(&FwCMDRspTxD,ptr,sizeof(FwCMDRspTxD));
			MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DEBUG_TRACE,("%s, Total Length = %d, LPLength is %d, bytecount = %d\n",__FUNCTION__, TotalLength, LPLength, FwCMDRspTxD.FwEventTxD.u2RxByteCount));
			//LPLength = FwCMDRspTxD.FwEventTxD.u2RxByteCount;
			pAdapter->LoopBackResult.RxByteCount += LPLength;
			pAdapter->LoopBackResult.RxPktCount ++;
			RtlCopyMemory(&pAdapter->LoopBackRxRaw, ptr, LPLength);
			pAdapter->LoopBackRxRawLen = LPLength;
		}
		LoopBack_BitTrueCheck(pAdapter);
		KeAcquireSpinLock(&pAdapter->LoopBackLock, &Irql);
		pAdapter->LoopBackWaitRx = FALSE;
		KeReleaseSpinLock(&pAdapter->LoopBackLock, Irql);
#ifdef RTMP_USB_SUPPORT		
		if(pAdapter->LoopBackSetting.BulkOutNumber == EPALLQID)
			KeSetEvent(&pAdapter->LoopBackEvent, 0, FALSE);	
#endif			
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DEBUG_TRACE,("%s, RxPktCount = %d\n",__FUNCTION__, pAdapter->LoopBackResult.RxPktCount));
		
	}
//~Loopback

	if(*pHeaderDW0&0x70000000)
	{
		pAdapter->OtherCounters.Rx0ReceivedCount++;
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DEBUG_TRACE,("Driver Rx Count = %d\n",pAdapter->OtherCounters.Rx0ReceivedCount));
		HandleRXVector(pData, pAdapter);
	}
	else
	{
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DEBUG_TRACE,("Unknown pkt!!!\n"));
	}
	//pULONG =(PULONG)pBuffer;
}

/*
	========================================================================
	
	Routine Description:
		handle RX vector, print 
		

	Arguments:
		pDest		Pointer to destination memory address
		pSrc		Pointer to source memory address
		Length		Copy size
		
	Return Value:
		None


	
	Note:
		
	========================================================================
*/
VOID HandleRXVector(
	IN PUCHAR   pData,
	IN	RTMP_ADAPTER *pAdapter
	)
{
	RX_INFO_DW0 RxInfoDW0;
	RXV_HEADER rxv_header;
	TX_STATUS tx_status;
	ULONG Temp = 0;
	char			rxvPrintBuffer[1024] = "";
	RXV			rxv;
	ULONG			i = 0;
	ULONG		TmpValue = 0;

	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE,("%s --->>\n",__FUNCTION__));

	RtlZeroMemory(&RxInfoDW0, sizeof(RxInfoDW0));
	RtlZeroMemory(&rxv_header, sizeof(rxv_header));
	RtlZeroMemory(&rxv, sizeof(rxv));
	memcpy(&RxInfoDW0,pData,sizeof(RX_INFO_DW0));
	if (RX_PKT_TYPE_RX_NORMAL==RxInfoDW0.PKT_TYPE || RX_PKT_TYPE_RX_DUP_FRB==RxInfoDW0.PKT_TYPE)
	{
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE,("Get RX_PKT_TYPE_RX_TXRXV or RX_PKT_TYPE_RX_DUP_FRB packet\n"));
	}
	else if (RX_PKT_TYPE_RX_DUP_FRB==RxInfoDW0.PKT_TYPE)
	{
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE,("TODO RX_PKT_TYPE_RX_DUP_FRB type\n"));
	}
	else if (RX_PKT_TYPE_RX_TXRXV==RxInfoDW0.PKT_TYPE)
	{
		memcpy(&rxv_header,pData,sizeof(rxv_header));
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE,("RXV_CNT = %d\n",rxv_header.RXV_CNT));
		Temp = rxv_header.RXV_CNT;
		pData+=sizeof(rxv_header);
		RtlZeroMemory(rxvPrintBuffer, sizeof(rxvPrintBuffer));
		for(i=0;i<Temp;i++)
		{
			memcpy(&rxv,pData,sizeof(rxv));
			RtlStringCbPrintfA(rxvPrintBuffer, sizeof(rxvPrintBuffer), 
							"txrate %d,HT_stbc %d,HT_adcode %d,\
HT_extitf %d,txmode %d,frmode %d,\
VHTA1_B22 %d,HT_aggregation %d,HT_shortgi %d,\
HT_smooth %d,HT_no_sound %d,VHTA2_B8_B1 %d,\
VHTA1_B5_B4 %d,\
Length %d,VHTA1_B16_B6 %d,\
VHTA1_B21_B17 %d,OFDM_FreqTrans_DET %d,ACI_DETx %d,\
SEL_ANT %d,RCPI0 %d,FAGC0_EQ_CAL %d,\
FGAC0_CAL_GAIN %d,RCPI1 %d,FAGC1_EQ_CAL %d,\
FGAC1_CAL_GAIN %d,\
IB_RSSIx %d,WB_RSSIx %d,FAGC_LPF_GAINx %d,\
IB_RSSI1 %d,FAGC_LPF_GAIN1 %d,\
FAGC_LNA_GAIN0 %d,FAGC_LNA_GAIN1 %d,cagc_state %d,\
FOE %d, LTF_PROC_TIME %d,LTF_SNR0 %d,\
NF0 %d,NF1 %d,WB_RSSI1 %d\r\n",
							rxv.vector1.RXV1.txrate,rxv.vector1.RXV1.HT_stbc,rxv.vector1.RXV1.HT_adcode,
							rxv.vector1.RXV1.HT_extitf,rxv.vector1.RXV1.txmode,rxv.vector1.RXV1.frmode,
							rxv.vector1.RXV1.VHTA1_B22,rxv.vector1.RXV1.HT_aggregation,rxv.vector1.RXV1.HT_shortgi,
							rxv.vector1.RXV1.HT_smooth,rxv.vector1.RXV1.HT_no_sound,rxv.vector1.RXV1.VHTA2_B8_B1,
							rxv.vector1.RXV1.VHTA1_B5_B4/*first cycle end*/,
							rxv.vector1.RXV2.Length,rxv.vector1.RXV2.VHTA1_B16_B6,
							rxv.vector1.RXV3.VHTA1_B21_B17,rxv.vector1.RXV3.OFDM_FreqTrans_DET,rxv.vector1.RXV3.ACI_DETx,
							rxv.vector1.RXV3.SEL_ANT,rxv.vector1.RXV3.RCPI0,rxv.vector1.RXV3.FAGC0_EQ_CAL,
							rxv.vector1.RXV3.FGAC0_CAL_GAIN, rxv.vector1.RXV3.RCPI1, rxv.vector1.RXV3.FAGC1_EQ_CAL,
							rxv.vector1.RXV3.FGAC1_CAL_GAIN,
							rxv.vector1.RXV4.IB_RSSIx, rxv.vector1.RXV4.WB_RSSIx ,rxv.vector1.RXV4.FAGC_LPF_GAINx,
							rxv.vector1.RXV4.IB_RSSI1 ,rxv.vector1.RXV4.FAGC_LPF_GAIN1,
							rxv.vector1.RXV5.FAGC_LNA_GAIN0 , rxv.vector1.RXV5.FAGC_LNA_GAIN1, rxv.vector1.RXV5.cagc_state,
							rxv.vector1.RXV5.FOE , rxv.vector1.RXV5.LTF_PROC_TIME , rxv.vector1.RXV5.LTF_SNR0,
							rxv.vector1.RXV6.NF0 ,rxv.vector1.RXV6.NF1 ,rxv.vector1.RXV6.WB_RSSI1);

						MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE,("%s",rxvPrintBuffer));
						RtlZeroMemory(rxvPrintBuffer, sizeof(rxvPrintBuffer));
						RtlStringCbPrintfA(rxvPrintBuffer, sizeof(rxvPrintBuffer),
							"RXValidIndicator %d,NSTSFiled %d,cagc_state0 %d,\
Reserved %d,\
Prim_ITFR_ENV %d,Sec_ITFR_EVN %d,Sec40_ITFR_EVN %d,\
BT_EVN %d,OFDM_BPSK_LQ %d,Reserved %d,\
Reserved1 %d,Capacity_LQ %d,BTD_NOTCH_LOC %d,\
OFDM_LTF_SNR1 %d,Reserved2 %d,DYN_BANDWIDTH_IN_NOT_HT %d,\
CH_BANDWIDTH_IN_NOT_HT %d,\
3th cycle Reserved2 %d,OFDM_CE_RMSD_Id %d,OFDM_CE_GIC_ENB %d,\
OFDM_CE_LTF_COMB %d,OFDM_DEW_MODE_DET %d,FCS_ERR %d\r\n",
							rxv.vector1.RXV6.RXValidIndicator ,rxv.vector1.RXV6.NSTSFiled ,rxv.vector1.RXV6.cagc_state0,
							rxv.vector1.RXV6.Reserved,
							rxv.vector2.FirstCycle.Prim_ITFR_ENV,rxv.vector2.FirstCycle.Sec_ITFR_EVN,rxv.vector2.FirstCycle.Sec40_ITFR_EVN,
							rxv.vector2.FirstCycle.BT_EVN ,rxv.vector2.FirstCycle.OFDM_BPSK_LQ ,rxv.vector2.FirstCycle.Reserved,
							rxv.vector2.SecondCycle.Reserved1 ,rxv.vector2.SecondCycle.Capacity_LQ ,rxv.vector2.SecondCycle.BTD_NOTCH_LOC,
							rxv.vector2.SecondCycle.OFDM_LTF_SNR1 , rxv.vector2.SecondCycle.Reserved2 , rxv.vector2.SecondCycle.DYN_BANDWIDTH_IN_NOT_HT,
							rxv.vector2.SecondCycle.CH_BANDWIDTH_IN_NOT_HT,
							rxv.vector2.ThreeCycle.Reserved2,rxv.vector2.ThreeCycle.OFDM_CE_RMSD_Id ,rxv.vector2.ThreeCycle.OFDM_CE_GIC_ENB,
							rxv.vector2.ThreeCycle.OFDM_CE_LTF_COMB, rxv.vector2.ThreeCycle.OFDM_DEW_MODE_DET, rxv.vector2.ThreeCycle.FCS_ERR);

						MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE,("%s",rxvPrintBuffer));

						pAdapter->OtherCounters.SNR0 = rxv.vector1.RXV5.LTF_SNR0;
						pAdapter->OtherCounters.SNR1 = rxv.vector2.SecondCycle.OFDM_LTF_SNR1;
						pAdapter->OtherCounters.RCPI0 = rxv.vector1.RXV3.RCPI0;
						pAdapter->OtherCounters.RCPI1 = rxv.vector1.RXV3.RCPI1;
						GatherRxvRSSI(pAdapter);

						if(1==rxv.vector1.RXV1.txmode)
						{//OFDM
							pAdapter->OtherCounters.FreqOffstFromRX = rxv.vector1.RXV5.FOE;
						}
						else if(0==rxv.vector1.RXV1.txmode)
						{//CCK 
							RtlCopyMemory(&TmpValue,&rxv.vector1.RXV5,	sizeof(rxv.vector1.RXV5));
							pAdapter->OtherCounters.FreqOffstFromRX =((TmpValue&=0x0001FF80)>>7);
						}
							
						if(i<Temp-1)
							pData+=sizeof(rxv);

		}
	}
	else if (RX_PKT_TYPE_RX_TXS==RxInfoDW0.PKT_TYPE)
	{
		memcpy(&rxv_header,pData,sizeof(rxv_header));
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE,("RXV_CNT = %d\n",rxv_header.RXV_CNT));
		Temp = rxv_header.RXV_CNT;
		pData+=sizeof(rxv_header);
		RtlZeroMemory(rxvPrintBuffer, sizeof(rxvPrintBuffer));
		for(i=0;i<Temp;i++)
		{
			memcpy(&tx_status,pData,sizeof(tx_status));
			RtlStringCbPrintfA(rxvPrintBuffer, sizeof(rxvPrintBuffer), 
							"TXSFM %d, TXPower %d,TSSI %d\r\n",
							tx_status.DW0.TXSFM,tx_status.DW2.TXPower,tx_status.DW4.TSSI);

			MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE,("%s",rxvPrintBuffer));
			pAdapter->OtherCounters.TXPower = tx_status.DW2.TXPower;
			pAdapter->OtherCounters.TSSI = tx_status.DW4.TSSI;			
		}

	}

	else if (RX_PKT_TYPE_RX_TMR==RxInfoDW0.PKT_TYPE)
	{	//7603 RX format 3.1 Timing Measurement Report Frame Format
		ULONG *pValue = (ULONG*)pData;//MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE,("TXD(TXWI) DW %d 0x%08X\n",j, *(ptr+j)));
		ULONG IorR = *pValue;
		int i;
		
		for(i=0;i<=6;i++)
		{
			MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE,("TMR DW %d 0x%08X\n",i, *(pValue+i)));
		}

		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("TMR Report: ir = %d\n", IorR & (0x1<<28)));
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("DWORD_0: ByteCnt=%d, NC=%d, TMF=%d, toa_vld=%d, tod_vld=%d\n",
                            					(*pValue) & 0x0000FFFF,  //[0:15]
                            					((*pValue) & 0x00040000) >> 18, //[18]
                            					((*pValue) & 0x00080000) >> 19,//[19]
                            					((*pValue) & 0x04000000) >> 26,//[26]
                            					((*pValue) & 0x08000000) >> 27));//[27]
		if( IorR & (0x1<<28))
		{
			MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE,("%s Get TMR Responder\n",__FUNCTION__));

			MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("TYPE=%x,SUB_TYPE=%x\n",
									((*pValue) & 0x00F00000) >> 20,
									((*pValue) & 0x03000000) >> 24));
			MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("DWORD_2: TA[0:15]=%x, SN=%x\n", 
									(*(pValue+2)) & 0x0000FFFF,
									(*(pValue+2)) >>16));
			MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("DWORD_3: TA[16:47]=%x\n", *(pValue+3)));
		}
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("DWORD_4: TOD[0:31]=0x%x\n", *(pValue+4)));
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("DWORD_6: TOD[32:47]=0x%x\n", (*(pValue+6)) & 0x0000FFFF));

		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("DWORD_5: TOA[0:31]=0x%x\n", *(pValue+5)));
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("DWORD_6: TOA[32:47]=0x%x\n", (*(pValue+6)) >>16));
	}
	else
	{
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE,("Unknow Type, warning\n",__FUNCTION__));
	}


	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE,("%s ---<<\n",__FUNCTION__));

}

// store rssi in array for date calcutale
NTSTATUS GatherRxvRSSI(RTMP_ADAPTER *pAd)
{
	NTSTATUS ntStatus = STATUS_SUCCESS;
	if(pAd->RssiCount < pAd->RssiLimit)
	{
		UINT32 Rssi0 = 0;
		UINT32 Rssi1 = 0;
		
		GetRxvRSSI(pAd, &Rssi0, &Rssi1);
		pAd->RecordRssi0[pAd->RssiCount] = (INT32)Rssi0;
		pAd->RecordRssi1[pAd->RssiCount] = (INT32)Rssi1;
		pAd->RssiCount++;
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE,("%s  RSSI counting , RssiCount = %d, RssiLimit = %d, Rssi0 = %d, Rssi1 = %d\n",__FUNCTION__, pAd->RssiCount, pAd->RssiLimit, Rssi0,Rssi1));	
	}
	else
	{
		//no action
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE,("%s Stop RSSI gathering, RssiCount = %d, RssiLimit = %d\n",__FUNCTION__,  pAd->RssiCount, pAd->RssiLimit));	
	}
	return ntStatus;
}

NTSTATUS GetAvgRxvRSSI(RTMP_ADAPTER *pAd, RSSI_DATA  *RssiData)
{
	NTSTATUS ntStatus = STATUS_SUCCESS;
	if(pAd->RssiCount > 0)
	{
		UINT32 idx = 0;
		UINT32 Count = pAd->RssiCount;
		INT32 SumRssi0 = 0;
		INT32 SumRssi1 = 0;
		UINT32 VarRssi0 = 0;
		UINT32 VarRssi1 = 0;
		for(idx=0; idx<Count;idx++)
		{
			SumRssi0 += pAd->RecordRssi0[idx];
			SumRssi1 += pAd->RecordRssi1[idx];			
			MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE,("%s idx = %d, SumRssi0 = %d, SumRssi1 = %d, RecordRssi0 %d, RecordRssi1 %d\n",__FUNCTION__,  idx, SumRssi0, SumRssi1, pAd->RecordRssi0[idx], pAd->RecordRssi1[idx]));	
		}
		//avg rssi calculate
		RssiData->RSSI0 = SumRssi0 /(INT32)pAd->RssiCount;
		RssiData->RSSI1 = SumRssi1 /(INT32)pAd->RssiCount;
		// var calculate
		for(idx=0; idx<Count;idx++)
		{
			if(pAd->RecordRssi0[idx] < RssiData->RSSI0)
			{
				VarRssi0 += (RssiData->RSSI0 -pAd->RecordRssi0[idx]);
			}
			else
			{
				VarRssi0 += (pAd->RecordRssi0[idx] - RssiData->RSSI0);
			}

			if(pAd->RecordRssi1[idx] < RssiData->RSSI1)
			{
				VarRssi1 += (RssiData->RSSI1 -pAd->RecordRssi1[idx]);
			}
			else
			{
				VarRssi1 += (pAd->RecordRssi1[idx] - RssiData->RSSI1);
			}
			MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE,("%s VarRssi0 = %d, VarRssi1 = %d, RecordRssi0 %d, RecordRssi1 %d\n",__FUNCTION__,  VarRssi0, VarRssi1, pAd->RecordRssi0[idx], pAd->RecordRssi1[idx]));	
		}
		RssiData->RSSI0var = VarRssi0 /pAd->RssiCount;
		RssiData->RSSI1var = VarRssi1 /pAd->RssiCount;
		if(IS_MT7636(pAd))
			RssiData->Numbers = 2;// 2 stream
		else if(IS_MT7603(pAd))
			RssiData->Numbers = 1;// 1 stream
		// todo
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE,("%s AVG RSSI0 = %d, RSSI1 = %d, var0 = %d, var1 = %d\n",__FUNCTION__,  RssiData->RSSI0, RssiData->RSSI1, RssiData->RSSI0var, RssiData->RSSI1var));	
	}
	else
	{
		//no action
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE,("%s wrong RssiCount = %d\n",__FUNCTION__,  pAd->RssiCount ));	
	}
	return ntStatus;
}


NTSTATUS SetRxvRSSILimit(RTMP_ADAPTER *pAd, UINT32 Limit)	
{
	NTSTATUS ntStatus = STATUS_SUCCESS;
	if(Limit < RXV_RSSI_LIMIT)
	{
		pAd->RssiLimit = Limit;
	}
	else
	{
		pAd->RssiLimit = RXV_RSSI_LIMIT;
	}
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE,("%s Set RssiLimit = %d\n",__FUNCTION__, pAd->RssiLimit));	
	return ntStatus;
}
NTSTATUS SetupTxPacket(RTMP_ADAPTER *pAd, UCHAR *pData, UINT32 Length, UINT32 BufferNum)
{
	NTSTATUS ntStatus = STATUS_SUCCESS;
#ifdef RTMP_SDIO_SUPPORT
	//if(BufferNum)
	ntStatus = SDIOSetupTxPacket(pAd, pData, Length, BufferNum);
#endif
#ifdef RTMP_USB_SUPPORT
	ntStatus = USBSetupTxPacket(pAd, pData, Length, BufferNum);
#endif
#ifdef RTMP_PCI_SUPPORT
//todo
#endif	
	return ntStatus;
}

NTSTATUS StartTx(RTMP_ADAPTER *pAd, UINT32 TxRemained, UINT32 txBufferNum)
{
	NTSTATUS ntStatus = STATUS_SUCCESS;
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE,("%s TxRemained = %d, txBufferNum = 0x%x\n",__FUNCTION__, TxRemained, txBufferNum));
	
#ifdef RTMP_SDIO_SUPPORT
	ntStatus = SDIOStartTxPacket(pAd, txBufferNum, TxRemained);
#endif
#ifdef RTMP_USB_SUPPORT
	ntStatus = USBStartTxPacket(pAd, txBufferNum, TxRemained);
#endif
#ifdef RTMP_PCI_SUPPORT
//todo
#endif		
	return ntStatus;
}

NTSTATUS StopTx(RTMP_ADAPTER *pAd)
{
	NTSTATUS ntStatus = STATUS_SUCCESS;
#ifdef RTMP_SDIO_SUPPORT
	ntStatus = SDIOStopTxPacket(pAd);
#endif
#ifdef RTMP_USB_SUPPORT
	ntStatus = USBStopTxPacket(pAd);
#endif
#ifdef RTMP_PCI_SUPPORT
//todo
#endif	
	return ntStatus;
}
NTSTATUS StopRx0(RTMP_ADAPTER *pAd)
{
	NTSTATUS ntStatus = STATUS_SUCCESS;
#ifdef RTMP_SDIO_SUPPORT

#endif
#ifdef RTMP_USB_SUPPORT
	ntStatus = RXStartStop(pAd, STOP, BULKIN_PIPENUMBER_0);
#endif	
	return ntStatus;
}

NTSTATUS StopRx1(RTMP_ADAPTER *pAd)
{
	NTSTATUS ntStatus = STATUS_SUCCESS;
#ifdef RTMP_SDIO_SUPPORT

#endif
#ifdef RTMP_USB_SUPPORT
	ntStatus = RXStartStop(pAd, STOP, BULKIN_PIPENUMBER_1);
#endif	
	return ntStatus;
}

NTSTATUS StartRx0(RTMP_ADAPTER *pAd)
{
	NTSTATUS ntStatus = STATUS_SUCCESS;
	UINT32 size = 0;
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE,("%s\n", __FUNCTION__));
	pAd->RssiCount = 0;
	if(pAd->FWMode == FWRAM && pAd->IsBfStored)
	{
		
		TxBfProfileDataReadAll(pAd, 1, 9, BW_20, (PFMU_DATA *) &pAd->prePfmuDataBW20, &size);
		TxBfProfileDataReadAll(pAd, 1, 9, BW_40, (PFMU_DATA *) &pAd->prePfmuDataBW40, &size);
	}
#ifdef RTMP_SDIO_SUPPORT

#endif
#ifdef RTMP_USB_SUPPORT
	ntStatus = RXStartStop(pAd, START, BULKIN_PIPENUMBER_0);
#endif
	return ntStatus;
}

NTSTATUS StartRx1(RTMP_ADAPTER *pAd)
{
	NTSTATUS ntStatus = STATUS_SUCCESS;
#ifdef RTMP_SDIO_SUPPORT

#endif
#ifdef RTMP_USB_SUPPORT
	ntStatus = RXStartStop(pAd, START, BULKIN_PIPENUMBER_1);
#endif	
	return ntStatus;
}

NTSTATUS RxHandler(RTMP_ADAPTER *pAd, UCHAR *pRxBuf, UINT32 PktLen, UINT32 RxNum)
{
	NTSTATUS ntStatus = STATUS_SUCCESS;
	KIRQL Irql;
	FwCMDRspTxD_STRUC FwCMDRspTxD;
	UINT32 *pDW = (UINT32 *)pRxBuf;
	UINT32 PrintLen = PktLen>44 ? 44 : PktLen;

	//print buffer	
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE,("=========Rx[%d] input data=================\n",RxNum));
#if 1	
	if(PktLen < 44)
	{
		UINT32 i;
		for(i=0;i<PktLen/4;i++)
			MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE,("Get Data %d 0x%08X\n",i, *(pDW+i)));
	}
	else
	{
		UINT32 i;
		for(i=0;i<44/4;i++)
			MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE,("Get Data %d 0x%08X\n",i, *(pDW+i)));
	}
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE,("===========================================\n"));
#else
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE,("Get Data 0x%08X\n", *(pDW)));
#endif	
	
	//FW Event
	if(IsFwRspPkt(pAd, pRxBuf, PktLen))
	{
#if 0	
		memcpy(&FwCMDRspTxD,pRxBuf,sizeof(FwCMDRspTxD));
		pAd->PacketCMDSeqMCU = (UCHAR)FwCMDRspTxD.FwEventTxD.ucSeqNum;
		pAd->FWRspStatus = (UCHAR)FwCMDRspTxD.ucStatus;
		
		//Rom patch get sem event
		if(IS_MT76x6(pAd))
		{
			if(FwCMDRspTxD.FwEventTxD.ucEID == 0x4 && FwCMDRspTxD.FwEventTxD.ucSeqNum == SEQ_CMD_ROM_PATCH_SEMAPHORE)
			{
				pAd->RomPatchSemStatus = (UCHAR)FwCMDRspTxD.ucStatus;
				MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE,("u2RxByteCount = 0x%x, ucEID = 0x%X, ucStatus = 0x%x, Seq = 0x%x\n", FwCMDRspTxD.FwEventTxD.u2RxByteCount, FwCMDRspTxD.FwEventTxD.ucEID, FwCMDRspTxD.FwEventTxD.ucSeqNum));
			}
		}
		if(pAd->bIsWaitFW == TRUE)
		{				
			KeSetEvent(&pAd->WaitFWEvent, 0, FALSE);
			pAd->bIsWaitFW =FALSE;
		}
		switch(FwCMDRspTxD.FwEventTxD.ucSeqNum)
		{				
			case SEQ_CMD_FW_SCATTERS:
			{
				MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE,("#######SEQ_CMD_FW_SCATTERS  response########\n"));
			}
			break;
			case SEQ_CMD_FW_STARTREQ:
			{
				MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE,("#######SEQ_CMD_FW_STARTREQ  response########\n"));
			}
			break;
			case SEQ_CMD_FW_STARTTORUN:
			{
				MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE,("#######SEQ_CMD_FW_STARTTORUN  response########\n"));
			}					
			break;
			case SEQ_CMD_FW_RESTART:
			{
				MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE,("#######SEQ_CMD_FW_RESTART  response########\n"));
			}
			break;
			case SEQ_CMD_ROM_PATCH_SEMAPHORE:					
			{
				MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE,("#######SEQ_CMD_ROM_PATCH_SEMAPHORE  response########\n"));
			}
			break;
			case SEQ_CMD_ROM_PATCH_STARTREQ:					
			{
				MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE,("#######SEQ_CMD_ROM_PATCH_STARTREQ  response########\n"));
			}
			break;
			case SEQ_CMD_ROM_PATCH_FINISH:
			{
				MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE,("#######SEQ_CMD_ROM_PATCH_FINISH  response########\n"));				
			}
			break;
			case SEQ_CMD_LOOPBACK_TEST:
			{
				MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE,("#######SEQ_CMD_LOOPBACK_TEST  response######## ucstatus = 0x%x\n", FwCMDRspTxD.ucStatus));
			}
			break;
			default:
				MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE,("#######Unhandled response########\n"));
		}
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE,("u2RxByteCount = 0x%x, ucEID = 0x%X\n", FwCMDRspTxD.FwEventTxD.u2RxByteCount, FwCMDRspTxD.FwEventTxD.ucEID));
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE,("%s get seqnum = %d, from MCU  ,g_FWRspStatus =%d\n",__FUNCTION__,FwCMDRspTxD.FwEventTxD.ucSeqNum,FwCMDRspTxD.ucStatus));
#else
		FWCmdEventHandler(pAd, pRxBuf, PktLen);
#endif
	}
	else
	{
#if 1
		RxPacket(pRxBuf, pAd, PktLen);
#else
	
	//Data
	//
	CheckSecurityResult((UINT32 *)pRxBuf,pAd);
		
	if(*pDW&0x70000000)
	{
		pAd->OtherCounters.Rx0ReceivedCount++;
		//MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE,("Driver Rx Count = %d\n",pAd->OtherCounters.Rx0ReceivedCount));
	}
	//todo
	//for loopback
	if(pAd->LoopBackRunning || pAd->LoopBackWaitRx)
	{
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE,("@#@# Driver Rx[%d] LoopBack \n", RxNum));
		if( PktLen >LOOPBACK_SIZE)
		{
			PktLen = LOOPBACK_SIZE;
			MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DEBUG_TRACE,("%s, max length is %d\n",__FUNCTION__, LOOPBACK_SIZE));
		}
		KeAcquireSpinLock(&pAd->LoopBackLock, &Irql);		
		pAd->LoopBackWaitRx = FALSE;
		KeReleaseSpinLock(&pAd->LoopBackLock, Irql);
		memcpy(&FwCMDRspTxD,pRxBuf,sizeof(FwCMDRspTxD));
		if(!IsFwRspPkt(pAd, pRxBuf, PktLen))
		{				
			MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DEBUG_TRACE,("%s, LPLength is %d, bytecount = %d\n",__FUNCTION__, PktLen, FwCMDRspTxD.FwEventTxD.u2RxByteCount));
			PktLen = FwCMDRspTxD.FwEventTxD.u2RxByteCount;
			pAd->LoopBackResult.RxByteCount += PktLen;
			pAd->LoopBackResult.RxPktCount ++;
			RtlCopyMemory(&pAd->LoopBackRxRaw, pRxBuf, PktLen);
			pAd->LoopBackRxRawLen = PktLen;
			LoopBack_BitTrueCheck(pAd);
		}			
	}
#endif	
	}
	return ntStatus;
}

BOOLEAN IsFwRspPkt(RTMP_ADAPTER *pAd, UCHAR *pRxBuf, UINT32 PktLen)
{
	FwCMDRspTxD_STRUC FwCMDRspTxD;
	if(sizeof(FwCMDRspTxD) <= PktLen )//4 
	{
		memcpy(&FwCMDRspTxD,pRxBuf,sizeof(FwCMDRspTxD));
		if(0xE000==FwCMDRspTxD.FwEventTxD.u2PacketType)//0xE000 if fw event format
			return TRUE;
		else
			return FALSE;
	}
	else
	{
		//size too small
		return FALSE;
	}	

}
