/*
 ***************************************************************************
 * Ralink Tech Inc.
 * 4F, No. 2 Technology	5th	Rd.
 * Science-based Industrial	Park
 * Hsin-chu, Taiwan, R.O.C.
 *
 * (c) Copyright 2002-2006, Ralink Technology, Inc.
 *
 * All rights reserved.	Ralink's source	code is	an unpublished work	and	the
 * use of a	copyright notice does not imply	otherwise. This	source code
 * contains	confidential trade secret material of Ralink Tech. Any attemp
 * or participation	in deciphering,	decoding, reverse engineering or in	any
 * way altering	the	source code	is stricitly prohibited, unless	the	prior
 * written consent of Ralink Technology, Inc. is obtained.
 ***************************************************************************

	Module Name:
	rtusb_bulk.c

	Abstract:

	Revision History:
	Who			When		What
	--------	----------	----------------------------------------------
	Name		Date		Modification logs

*/

#ifdef RTMP_MAC_USB


#include	"rt_config.h"
/* Match total 6 bulkout endpoint to corresponding queue.*/
UCHAR	EpToQueue[6]={FIFO_EDCA, FIFO_EDCA, FIFO_EDCA, FIFO_EDCA, FIFO_EDCA, FIFO_MGMT};


#ifdef INF_AMAZON_SE
UINT16 MaxBulkOutsSizeLimit[5][4] =
{
#ifdef RELEASE_EXCLUDE
/*
	our chip might hang for unknown reason when we bulk out multiACs with bulkout size > 10k
*/
#endif /* RELEASE_EXCLUDE */
	/* Priority high -> low*/
	{ 24576, 2048, 2048, 2048 },	/* 0 AC	*/
	{ 24576, 2048, 2048, 2048 },	/* 1 AC	 */
	{ 24576, 2048, 2048, 2048 }, 	/* 2 ACs*/
	{ 24576, 6144, 2048, 2048 }, 	/* 3 ACs*/
	{ 24576, 6144, 4096, 2048 }		/* 4 ACs*/
};


VOID SoftwareFlowControl(
	IN PRTMP_ADAPTER pAd)
{
	BOOLEAN ResetBulkOutSize=FALSE;
	UCHAR i=0,RunningQueueNo=0,QueIdx=0,HighWorkingAcCount=0;
	UINT PacketsInQueueSize=0;
	UCHAR Priority[]={1,0,2,3};

	for (i=0;i<NUM_OF_TX_RING;i++)
	{

		if (pAd->TxContext[i].CurWritePosition>=pAd->TxContext[i].ENextBulkOutPosition)
		{
			PacketsInQueueSize=pAd->TxContext[i].CurWritePosition-pAd->TxContext[i].ENextBulkOutPosition;
		}
		else
		{
			PacketsInQueueSize=MAX_TXBULK_SIZE-pAd->TxContext[i].ENextBulkOutPosition+pAd->TxContext[i].CurWritePosition;
		}

		if (pAd->BulkOutDataSizeCount[i]>20480 || PacketsInQueueSize>6144)
		{
			RunningQueueNo++;
			pAd->BulkOutDataFlag[i]=TRUE;
		}
		else
			pAd->BulkOutDataFlag[i]=FALSE;

		pAd->BulkOutDataSizeCount[i]=0;
	}

	if (RunningQueueNo>pAd->LastRunningQueueNo)
	{
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_INFO,("SoftwareFlowControl  reset %d > %d \n",RunningQueueNo,pAd->LastRunningQueueNo));

		ResetBulkOutSize=TRUE;
		 pAd->RunningQueueNoCount=0;
		 pAd->LastRunningQueueNo=RunningQueueNo;
	}
	else if (RunningQueueNo==pAd->LastRunningQueueNo)
	{
pAd->RunningQueueNoCount=0;
	}
	else if (RunningQueueNo<pAd->LastRunningQueueNo)
	{
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_INFO,("SoftwareFlowControl  reset %d < %d \n",RunningQueueNo,pAd->LastRunningQueueNo));
		pAd->RunningQueueNoCount++;
		if (pAd->RunningQueueNoCount>=6)
		{
			ResetBulkOutSize=TRUE;
			pAd->RunningQueueNoCount=0;
			pAd->LastRunningQueueNo=RunningQueueNo;
		}
	}

	if (ResetBulkOutSize==TRUE)
	{
		for (QueIdx=0;QueIdx<NUM_OF_TX_RING;QueIdx++)
		{
			HighWorkingAcCount=0;
			for (i=0;i<NUM_OF_TX_RING;i++)
			{
				if (QueIdx==i)
					continue;

				if (pAd->BulkOutDataFlag[i]==TRUE && Priority[i]>Priority[QueIdx])
						HighWorkingAcCount++;

			}
			pAd->BulkOutDataSizeLimit[QueIdx]=MaxBulkOutsSizeLimit[RunningQueueNo][HighWorkingAcCount];
		}

			MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("Reset bulkout size AC0(BE):%7d AC1(BK):%7d AC2(VI):%7d AC3(VO):%7d %d\n",
						pAd->BulkOutDataSizeLimit[0], pAd->BulkOutDataSizeLimit[1]
						,pAd->BulkOutDataSizeLimit[2], pAd->BulkOutDataSizeLimit[3], RunningQueueNo));
	}
}
#endif /* INF_AMAZON_SE */


VOID	RTUSBInitTxDesc(
	IN	PRTMP_ADAPTER	pAd,
	IN	PTX_CONTEXT		pTxContext,
	IN	UCHAR			BulkOutPipeId,
	IN	usb_complete_t	Func)
{
	PURB				pUrb;
	PUCHAR				pSrc = NULL;
	POS_COOKIE			pObj = (POS_COOKIE) pAd->OS_Cookie;
	RTMP_CHIP_CAP *pChipCap = &pAd->chipCap;

	pUrb = pTxContext->pUrb;
	ASSERT(pUrb);

	/* Store BulkOut PipeId*/
	pTxContext->BulkOutPipeId = BulkOutPipeId;

	if (pTxContext->bAggregatible)
	{
		pSrc = &pTxContext->TransferBuffer->Aggregation[2];

		RTUSB_FILL_TX_BULK_URB(pUrb,
						pObj->pUsb_Dev,
						pChipCap->WMM0ACBulkOutAddr[BulkOutPipeId],
						pSrc,
						pTxContext->BulkOutSize,
						Func,
						pTxContext,
						(pTxContext->data_dma + TX_BUFFER_NORMSIZE + 2));
	}
	else
	{
		pSrc = (PUCHAR) pTxContext->TransferBuffer->field.WirelessPacket;

#ifdef MT_MAC
		if ((pAd->chipCap.hif_type == HIF_MT) && (pTxContext == (PTX_CONTEXT)pAd->MgmtRing.Cell[pAd->MgmtRing.TxDmaIdx].AllocVa))
		{
			RTUSB_FILL_TX_BULK_URB(pUrb,
						pObj->pUsb_Dev,
						pChipCap->CommandBulkOutAddr,
						pSrc,
						pTxContext->BulkOutSize,
						Func,
						pTxContext,
						pTxContext->data_dma);
		}
        else if ((pAd->chipCap.hif_type == HIF_MT) && (pTxContext == (PTX_CONTEXT)pAd->BcnRing.Cell[pAd->BcnRing.TxDmaIdx].AllocVa))
		{
			RTUSB_FILL_TX_BULK_URB(pUrb,
						pObj->pUsb_Dev,
						pChipCap->CommandBulkOutAddr,
						pSrc,
						pTxContext->BulkOutSize,
						Func,
						pTxContext,
						pTxContext->data_dma);
		}
		else
#endif
		RTUSB_FILL_TX_BULK_URB(pUrb,
						pObj->pUsb_Dev,
						pChipCap->WMM0ACBulkOutAddr[BulkOutPipeId],
						pSrc,
						pTxContext->BulkOutSize,
						Func,
						pTxContext,
						pTxContext->data_dma);
	}
}

#ifdef MULTI_WMM_SUPPORT
VOID	RTUSBInitHTTxWmm1Desc(
	IN	PRTMP_ADAPTER	pAd,
	IN	PHT_TX_CONTEXT	pTxContext,
	IN	UCHAR			BulkOutPipeId,
	IN	ULONG			BulkOutSize,
	IN	usb_complete_t	Func)
{
	PURB				pUrb;
	PUCHAR				pSrc = NULL;
	POS_COOKIE			pObj = (POS_COOKIE) pAd->OS_Cookie;
	RTMP_CHIP_CAP *pChipCap = &pAd->chipCap;

	pUrb = pTxContext->pUrb;
	ASSERT(pUrb);

	/* Store BulkOut PipeId*/
	pTxContext->BulkOutPipeId = BulkOutPipeId;

	pSrc = &pTxContext->TransferBuffer->field.WirelessPacket[pTxContext->NextBulkOutPosition];

	RTUSB_FILL_HTTX_BULK_URB(pUrb,
						pObj->pUsb_Dev,
						pChipCap->WMM1ACBulkOutAddr,
						pSrc,
						BulkOutSize,
						Func,
						pTxContext,
						(pTxContext->data_dma + pTxContext->NextBulkOutPosition));
}
#endif /* MULTI_WMM_SUPPORT */

VOID	RTUSBInitHTTxDesc(
	IN	PRTMP_ADAPTER	pAd,
	IN	PHT_TX_CONTEXT	pTxContext,
	IN	UCHAR			BulkOutPipeId,
	IN	ULONG			BulkOutSize,
	IN	usb_complete_t	Func)
{
	PURB				pUrb;
	PUCHAR				pSrc = NULL;
	POS_COOKIE			pObj = (POS_COOKIE) pAd->OS_Cookie;
	RTMP_CHIP_CAP *pChipCap = &pAd->chipCap;

	pUrb = pTxContext->pUrb;
	ASSERT(pUrb);

	/* Store BulkOut PipeId*/
	pTxContext->BulkOutPipeId = BulkOutPipeId;

	pSrc = &pTxContext->TransferBuffer->field.WirelessPacket[pTxContext->NextBulkOutPosition];

	RTUSB_FILL_HTTX_BULK_URB(pUrb,
						pObj->pUsb_Dev,
						pChipCap->WMM0ACBulkOutAddr[BulkOutPipeId],
						pSrc,
						BulkOutSize,
						Func,
						pTxContext,
						(pTxContext->data_dma + pTxContext->NextBulkOutPosition));
}

VOID	RTUSBInitRxDesc(
	IN	PRTMP_ADAPTER	pAd,
	IN	PRX_CONTEXT		pRxContext)
{
	PURB				pUrb;
	POS_COOKIE			pObj = (POS_COOKIE) pAd->OS_Cookie;
	ULONG				RX_bulk_size;
	RTMP_CHIP_CAP *pChipCap = &pAd->chipCap;

	pUrb = pRxContext->pUrb;
	ASSERT(pUrb);

	if (pAd->BulkInMaxPacketSize == 64)
		RX_bulk_size = 4096;
	else
		RX_bulk_size = MAX_RXBULK_SIZE;

	RTUSB_FILL_RX_BULK_URB(pUrb,
					pObj->pUsb_Dev,
					pChipCap->DataBulkInAddr,
					&(pRxContext->TransferBuffer[0]),
					RX_bulk_size,
					RtmpUsbBulkRxComplete,
					(void *)pRxContext,
					pRxContext->data_dma);
}


#if 0
VOID RTUSBInitCmdRspEventDesc(
	PRTMP_ADAPTER pAd,
	PCMD_RSP_CONTEXT pCmdRspEventContext)
{
	PURB pUrb = pCmdRspEventContext->pUrb;
	POS_COOKIE pObj = (POS_COOKIE)pAd->OS_Cookie;
	RTMP_CHIP_CAP *pChipCap = &pAd->chipCap;

	RTUSB_FILL_RX_BULK_URB(pUrb,
						   pObj->pUsb_Dev,
						   pChipCap->CommandRspBulkInAddr,
						   pCmdRspEventContext->CmdRspBuffer,
						   CMD_RSP_BULK_SIZE,
						   RtmpUsbBulkCmdRspEventComplete,
						   (void *)pCmdRspEventContext,
						   pCmdRspEventContext->data_dma);

}
#endif


/*
	========================================================================

	Routine Description:

	Arguments:

	Return Value:

	Note:

	========================================================================
*/

#define BULK_OUT_LOCK(pLock, IrqFlags)	\
		if(1 /*!(in_interrupt() & 0xffff0000)*/)	\
			RTMP_IRQ_LOCK((pLock), IrqFlags);

#define BULK_OUT_UNLOCK(pLock, IrqFlags)	\
		if(1 /*!(in_interrupt() & 0xffff0000)*/)	\
			RTMP_IRQ_UNLOCK((pLock), IrqFlags);

#ifdef MULTI_WMM_SUPPORT
VOID RTUSBBulkOutWmm1DataPacket(RTMP_ADAPTER *pAd, UCHAR BulkOutPipeId, UCHAR Index)
{
	PHT_TX_CONTEXT pHTTXContext;
	PURB pUrb;
	int ret = 0;
	TXINFO_STRUC *pTxInfo, *pLastTxInfo = NULL;
	TXWI_STRUC *pTxWI;
	USHORT txwi_pkt_len = 0;
	UCHAR ampdu = 0, phy_mode = 0, pid;
	ULONG TmpBulkEndPos, ThisBulkSize;
	ULONG	IrqFlags = 0, IrqFlags2 = 0;
	UCHAR *pWirelessPkt, *pAppendant;
	UINT32 aggregation_num = 0, padding = 0;

	BOOLEAN	 bTxQLastRound = FALSE;
	UCHAR allzero[4]= {0x0,0x0,0x0,0x0};

	BULK_OUT_LOCK(&pAd->BulkOutWmm1Lock[BulkOutPipeId], IrqFlags);
	if ((pAd->BulkOutWmm1Pending[BulkOutPipeId] == TRUE) || RTMP_TEST_FLAG(pAd, fRTMP_ADAPTER_NEED_STOP_TX))
	{
		BULK_OUT_UNLOCK(&pAd->BulkOutWmm1Lock[BulkOutPipeId], IrqFlags);
#ifdef CONFIG_DVT_MODE
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("%s::BulkOutPending[%d]=%d, NEED_STOP_TX(%d)\n", __FUNCTION__, BulkOutPipeId, pAd->BulkOutPending[BulkOutPipeId], RTMP_TEST_FLAG(pAd, fRTMP_ADAPTER_NEED_STOP_TX)));
#endif /* CONFIG_DVT_MODE */
		return;
	}
	pAd->BulkOutWmm1Pending[BulkOutPipeId] = TRUE;
	pid = 0;

	if (((!OPSTATUS_TEST_FLAG(pAd, fOP_AP_STATUS_MEDIA_STATE_CONNECTED)) &&
		( !OPSTATUS_TEST_FLAG(pAd, fOP_STATUS_MEDIA_STATE_CONNECTED)))
#ifdef MESH_SUPPORT
			&& !MESH_ON(pAd)
#endif /* MESH_SUPPORT */
#ifdef P2P_SUPPORT
			&& !(P2P_GO_ON(pAd) || P2P_CLI_ON(pAd))
#endif /* P2P_SUPPORT */
#ifdef RT_CFG80211_P2P_CONCURRENT_DEVICE
			&& !(RTMP_CFG80211_VIF_P2P_GO_ON(pAd) || RTMP_CFG80211_VIF_P2P_CLI_ON(pAd))
#endif /* RT_CFG80211_P2P_CONCURRENT_DEVICE */
	)
	{
		pAd->BulkOutWmm1Pending[BulkOutPipeId] = FALSE;
		BULK_OUT_UNLOCK(&pAd->BulkOutWmm1Lock[BulkOutPipeId], IrqFlags);
		return;
	}
	BULK_OUT_UNLOCK(&pAd->BulkOutWmm1Lock[BulkOutPipeId], IrqFlags);


	pHTTXContext = &(pAd->TxContextWmm1[BulkOutPipeId]);

	BULK_OUT_LOCK(&pAd->TxContextQueueWmm1Lock[BulkOutPipeId], IrqFlags2);
	if ((pHTTXContext->ENextBulkOutPosition == pHTTXContext->CurWritePosition)
		|| ((pHTTXContext->ENextBulkOutPosition-8) == pHTTXContext->CurWritePosition)
		)  /* druing writing. */
	{
		BULK_OUT_UNLOCK(&pAd->TxContextQueueWmm1Lock[BulkOutPipeId], IrqFlags2);

		BULK_OUT_LOCK(&pAd->BulkOutWmm1Lock[BulkOutPipeId], IrqFlags);
		pAd->BulkOutWmm1Pending[BulkOutPipeId] = FALSE;

		/* Clear Data flag*/
		RTUSB_CLEAR_BULK_FLAG(pAd, (fRTUSB_BULK_OUT_DATA_WMM1_FRAG << BulkOutPipeId));
		RTUSB_CLEAR_BULK_FLAG(pAd, (fRTUSB_BULK_OUT_DATA_WMM1_NORMAL << BulkOutPipeId));

		BULK_OUT_UNLOCK(&pAd->BulkOutWmm1Lock[BulkOutPipeId], IrqFlags);
		return;
	}

	/* Clear Data flag*/
	RTUSB_CLEAR_BULK_FLAG(pAd, (fRTUSB_BULK_OUT_DATA_WMM1_FRAG << BulkOutPipeId));
	RTUSB_CLEAR_BULK_FLAG(pAd, (fRTUSB_BULK_OUT_DATA_WMM1_NORMAL << BulkOutPipeId));

	/*
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE,("BulkOut-B:I=0x%lx, CWPos=%ld, CWRPos=%ld, NBPos=%ld, ENBPos=%ld, bCopy=%d!\n",
					in_interrupt(),
					pHTTXContext->CurWritePosition, pHTTXContext->CurWriteRealPos,
					pHTTXContext->NextBulkOutPosition, pHTTXContext->ENextBulkOutPosition,
					pHTTXContext->bCopySavePad));
	*/
	pHTTXContext->NextBulkOutPosition = pHTTXContext->ENextBulkOutPosition;
	ThisBulkSize = 0;
	TmpBulkEndPos = pHTTXContext->NextBulkOutPosition;
	pWirelessPkt = &pHTTXContext->TransferBuffer->field.WirelessPacket[0];

#ifndef USB_BULK_BUF_ALIGMENT
	if ((pHTTXContext->bCopySavePad == TRUE))
	{
		if (RTMPEqualMemory(pHTTXContext->SavedPad, allzero,4))
		{
			DBGPRINT_RAW(DBG_CAT_ALL, DBG_LVL_ERROR,("e1, allzero : %x  %x  %x  %x  %x  %x  %x  %x \n",
				pHTTXContext->SavedPad[0], pHTTXContext->SavedPad[1], pHTTXContext->SavedPad[2],pHTTXContext->SavedPad[3]
				,pHTTXContext->SavedPad[4], pHTTXContext->SavedPad[5], pHTTXContext->SavedPad[6],pHTTXContext->SavedPad[7]));
		}
		NdisMoveMemory(&pWirelessPkt[TmpBulkEndPos], pHTTXContext->SavedPad, 8);
		pHTTXContext->bCopySavePad = FALSE;
		if (pAd->bForcePrintTX == TRUE)
			MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE,("RTUSBBulkOutDataPacket --> COPY PAD. CurWrite = %ld, NextBulk = %ld.   ENextBulk = %ld.\n",
						pHTTXContext->CurWritePosition, pHTTXContext->NextBulkOutPosition, pHTTXContext->ENextBulkOutPosition));
	}
#endif /* USB_BULK_BUF_ALIGMENT */

	do
	{
#ifdef MT_MAC
		TMAC_TXD_L *txd_l = (TMAC_TXD_L *)&pWirelessPkt[TmpBulkEndPos];
		TMAC_TXD_S *txd_s = (TMAC_TXD_S *)&pWirelessPkt[TmpBulkEndPos];
                TMAC_TXD_0 *txd_0 = &txd_s->txd_0;
		TMAC_TXD_1 *txd_1 = &txd_s->txd_1;
		TMAC_TXD_7 *txd_7;

		if (txd_1->ft == TMI_FT_SHORT)
			txd_7 = &txd_s->txd_7;
		else
			txd_7 = &txd_l->txd_7;
#else
		pTxInfo = (TXINFO_STRUC *)&pWirelessPkt[TmpBulkEndPos];
		pTxWI = (TXWI_STRUC *)&pWirelessPkt[TmpBulkEndPos + TXINFO_SIZE];
#endif /* MT_MAC */

#ifdef RLT_MAC
		if (pAd->chipCap.hif_type == HIF_RLT) {
#ifdef MT7601
			if (IS_MT7601(pAd)) {
				ampdu = pTxWI->TXWI_X.AMPDU;
				phy_mode = pTxWI->TXWI_X.PHYMODE;
				pid = pTxWI->TXWI_X.PacketId;
				txwi_pkt_len = pTxWI->TXWI_X.MPDUtotalByteCnt;
			}
			else
#endif /* MT7601 */
			{
				ampdu = pTxWI->TXWI_N.AMPDU;
				phy_mode = pTxWI->TXWI_N.PHYMODE;
				pid = pTxWI->TXWI_N.TxPktId;
				txwi_pkt_len = pTxWI->TXWI_N.MPDUtotalByteCnt;
			}
		}
#endif /* RLT_MAC */

#ifdef RTMP_MAC
		if (pAd->chipCap.hif_type == HIF_RTMP) {
			ampdu = pTxWI->TXWI_O.AMPDU;
			phy_mode = pTxWI->TXWI_O.PHYMODE;
			pid = pTxWI->TXWI_O.PacketId;
			txwi_pkt_len = pTxWI->TXWI_O.MPDUtotalByteCnt;
		}
#endif /* RTMP_MAC */

		if (pAd->bForcePrintTX == TRUE)
			MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("RTUSBBulkOutDataPacket AMPDU = %d.\n",   ampdu));

		if (((ThisBulkSize & 0xffff8000) != 0) || ((ThisBulkSize&0x6000) == 0x6000))
		{	/* Limit BulkOut size to about 24k bytes.*/
			pHTTXContext->ENextBulkOutPosition = TmpBulkEndPos;
			break;
		}
		else if (((pAd->BulkOutMaxPacketSize < 512) && ((ThisBulkSize&0xfffff800) != 0) ) /*|| ( (ThisBulkSize != 0)  && (pTxWI->AMPDU == 0))*/)
		{	/* For USB 1.1 or peer which didn't support AMPDU, limit the BulkOut size. */
			/* For performence in b/g mode, now just check for USB 1.1 and didn't care about the APMDU or not! 2008/06/04.*/
			pHTTXContext->ENextBulkOutPosition = TmpBulkEndPos;
			break;
		}
#ifdef CONFIG_MULTI_CHANNEL
		else if  ((pAd->Mlme.bStartMcc == TRUE) &&  ((ThisBulkSize&0xfffff800) != 0) )
		{
			pHTTXContext->ENextBulkOutPosition = TmpBulkEndPos;
			break;
		}
#endif /* CONFIG_MULTI_CHANNEL */

		if (TmpBulkEndPos == pHTTXContext->CurWritePosition)
		{
			pHTTXContext->ENextBulkOutPosition = TmpBulkEndPos;
			break;
		}

		padding = (4 - (txd_0->tx_byte_cnt % 4)) & 0x03;
		ThisBulkSize += txd_0->tx_byte_cnt+padding;
		TmpBulkEndPos += txd_0->tx_byte_cnt+padding;

		if (txd_7->sw_field == 1)
		{
			txd_7->sw_field = 0;
			bTxQLastRound = TRUE;
			pHTTXContext->ENextBulkOutPosition = 8;
			break;
		}

		aggregation_num++;

		if ((aggregation_num == 1) && (!pAd->usb_ctl.usb_aggregation)) {
			pHTTXContext->ENextBulkOutPosition = TmpBulkEndPos;
			break;
		}
	}while (TRUE);

#ifdef CONFIG_DVT_MODE
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("%s::aggregation_num(%d), usb_aggregation(%d)\n", __FUNCTION__, aggregation_num, pAd->usb_ctl.usb_aggregation));
#endif /* CONFIG_DVT_MODE */

	/*
		We need to copy SavedPad when following condition matched!
			1. Not the last round of the TxQueue and
			2. any match of following cases:
				(1). The End Position of this bulk out is reach to the Currenct Write position and
						the TxInfo and related header already write to the CurWritePosition.
			   		=>(ENextBulkOutPosition == CurWritePosition) && (CurWriteRealPos > CurWritePosition)

				(2). The EndPosition of the bulk out is not reach to the Current Write Position.
					=>(ENextBulkOutPosition != CurWritePosition)
	*/
#ifndef USB_BULK_BUF_ALIGMENT
	if ((bTxQLastRound == FALSE) &&
		 (((pHTTXContext->ENextBulkOutPosition == pHTTXContext->CurWritePosition) && (pHTTXContext->CurWriteRealPos > pHTTXContext->CurWritePosition)) ||
		  (pHTTXContext->ENextBulkOutPosition != pHTTXContext->CurWritePosition))
		)
	{
		NdisMoveMemory(pHTTXContext->SavedPad, &pWirelessPkt[pHTTXContext->ENextBulkOutPosition], 8);
		pHTTXContext->bCopySavePad = TRUE;
		if (RTMPEqualMemory(pHTTXContext->SavedPad, allzero,4))
		{
			PUCHAR	pBuf = &pHTTXContext->SavedPad[0];
			DBGPRINT_RAW(DBG_CAT_ALL, DBG_LVL_ERROR,("WARNING-Zero-3:%02x%02x%02x%02x%02x%02x%02x%02x,CWPos=%ld, CWRPos=%ld, bCW=%d, NBPos=%ld, TBPos=%ld, TBSize=%ld\n",
				pBuf[0], pBuf[1], pBuf[2],pBuf[3],pBuf[4], pBuf[5], pBuf[6],pBuf[7], pHTTXContext->CurWritePosition, pHTTXContext->CurWriteRealPos,
				pHTTXContext->bCurWriting, pHTTXContext->NextBulkOutPosition, TmpBulkEndPos, ThisBulkSize));

			pBuf = &pWirelessPkt[pHTTXContext->CurWritePosition];
			DBGPRINT_RAW(DBG_CAT_ALL, DBG_LVL_ERROR,("\tCWPos=%02x%02x%02x%02x%02x%02x%02x%02x\n", pBuf[0], pBuf[1], pBuf[2],pBuf[3],pBuf[4], pBuf[5], pBuf[6],pBuf[7]));
		}
		/*MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_LOUD,("ENPos==CWPos=%ld, CWRPos=%ld, bCSPad=%d!\n", pHTTXContext->CurWritePosition, pHTTXContext->CurWriteRealPos, pHTTXContext->bCopySavePad));*/
	}
#endif /* USB_BULK_BUF_ALIGMENT */

	if (pAd->bForcePrintTX == TRUE)
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE,("BulkOut-A:Size=%ld, CWPos=%ld, NBPos=%ld, ENBPos=%ld, bCopy=%d!\n", ThisBulkSize, pHTTXContext->CurWritePosition, pHTTXContext->NextBulkOutPosition, pHTTXContext->ENextBulkOutPosition, pHTTXContext->bCopySavePad));
	/*MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_LOUD,("BulkOut-A:Size=%ld, CWPos=%ld, CWRPos=%ld, NBPos=%ld, ENBPos=%ld, bCopy=%d, bLRound=%d!\n", ThisBulkSize, pHTTXContext->CurWritePosition, pHTTXContext->CurWriteRealPos, pHTTXContext->NextBulkOutPosition, pHTTXContext->ENextBulkOutPosition, pHTTXContext->bCopySavePad, bTxQLastRound));*/

		/* USB DMA engine requires to pad extra 4 bytes. This pad doesn't count into real bulkoutsize.*/
	pAppendant = &pWirelessPkt[TmpBulkEndPos];
	NdisZeroMemory(pAppendant, 8);
		ThisBulkSize += 4;
		pHTTXContext->LastOne = TRUE;

	pHTTXContext->BulkOutSize = ThisBulkSize;

	pAd->watchDogWmm1TxPendingCnt[BulkOutPipeId] = 1;
	BULK_OUT_UNLOCK(&pAd->TxContextQueueWmm1Lock[BulkOutPipeId], IrqFlags2);

	/* Init Tx context descriptor*/
	RTUSBInitHTTxWmm1Desc(pAd, pHTTXContext, EDCA_WMM1_AC0_PIPE+BulkOutPipeId, ThisBulkSize, (usb_complete_t)RtmpUsbBulkOutDataPacketComplete);

	pUrb = pHTTXContext->pUrb;
	if((ret = RTUSB_SUBMIT_URB(pUrb))!=0)
	{
#ifdef RELEASE_EXCLUDE
		/*
			Bug history:
			1. Webcom
				When submit non-BE packet, the function will fail here.
				After EEPROM content is updated, everything is ok.
		*/
#endif /* RELEASE_EXCLUDE */
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("RTUSBBulkOutDataPacket: Submit Tx URB failed %d\n", ret));

		BULK_OUT_LOCK(&pAd->BulkOutWmm1Lock[BulkOutPipeId], IrqFlags);
		pAd->BulkOutWmm1Pending[BulkOutPipeId] = FALSE;
		pAd->watchDogWmm1TxPendingCnt[BulkOutPipeId] = 0;
		BULK_OUT_UNLOCK(&pAd->BulkOutWmm1Lock[BulkOutPipeId], IrqFlags);

		return;
	}

	BULK_OUT_LOCK(&pAd->BulkOutWmm1Lock[BulkOutPipeId], IrqFlags);
	pHTTXContext->IRPPending = TRUE;
	BULK_OUT_UNLOCK(&pAd->BulkOutWmm1Lock[BulkOutPipeId], IrqFlags);
	pAd->BulkOutReq++;
}
#endif /* MULTI_WMM_SUPPORT */

VOID RTUSBBulkOutDataPacket(RTMP_ADAPTER *pAd, UCHAR BulkOutPipeId, UCHAR Index)
{
	PHT_TX_CONTEXT pHTTXContext;
	PURB pUrb;
	int ret = 0;
	TXINFO_STRUC *pTxInfo, *pLastTxInfo;
	TXWI_STRUC *pTxWI;
	USHORT txwi_pkt_len;
	UCHAR ampdu = 0, phy_mode, pid;
	ULONG TmpBulkEndPos, ThisBulkSize;
	ULONG	IrqFlags = 0, IrqFlags2 = 0;
	UCHAR *pWirelessPkt, *pAppendant;
	UINT32 aggregation_num = 0, padding = 0;
#ifdef USB_BULK_BUF_ALIGMENT
	BOOLEAN bLasAlignmentsectiontRound = FALSE;
#else
	BOOLEAN	 bTxQLastRound = FALSE;
	UCHAR allzero[4]= {0x0,0x0,0x0,0x0};
#endif /* USB_BULK_BUF_ALIGMENT */

	BULK_OUT_LOCK(&pAd->BulkOutLock[BulkOutPipeId], IrqFlags);
	if ((pAd->BulkOutPending[BulkOutPipeId] == TRUE) || RTMP_TEST_FLAG(pAd, fRTMP_ADAPTER_NEED_STOP_TX))
	{
		BULK_OUT_UNLOCK(&pAd->BulkOutLock[BulkOutPipeId], IrqFlags);
#ifdef CONFIG_DVT_MODE
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("%s::BulkOutPending[%d]=%d, NEED_STOP_TX(%d)\n", __FUNCTION__, BulkOutPipeId, pAd->BulkOutPending[BulkOutPipeId], RTMP_TEST_FLAG(pAd, fRTMP_ADAPTER_NEED_STOP_TX)));
#endif /* CONFIG_DVT_MODE */
		return;
	}
	pAd->BulkOutPending[BulkOutPipeId] = TRUE;
	phy_mode = pid = txwi_pkt_len = 0;
	pLastTxInfo = NULL;

	if (((!OPSTATUS_TEST_FLAG(pAd, fOP_AP_STATUS_MEDIA_STATE_CONNECTED)) &&
		( !OPSTATUS_TEST_FLAG(pAd, fOP_STATUS_MEDIA_STATE_CONNECTED)))
#ifdef MESH_SUPPORT
			&& !MESH_ON(pAd)
#endif /* MESH_SUPPORT */
#ifdef P2P_SUPPORT
			&& !(P2P_GO_ON(pAd) || P2P_CLI_ON(pAd))
#endif /* P2P_SUPPORT */
#ifdef RT_CFG80211_P2P_CONCURRENT_DEVICE
			&& !(RTMP_CFG80211_VIF_P2P_GO_ON(pAd) || RTMP_CFG80211_VIF_P2P_CLI_ON(pAd))
#endif /* RT_CFG80211_P2P_CONCURRENT_DEVICE */

		)
	{
		pAd->BulkOutPending[BulkOutPipeId] = FALSE;
		BULK_OUT_UNLOCK(&pAd->BulkOutLock[BulkOutPipeId], IrqFlags);
		return;
	}
	BULK_OUT_UNLOCK(&pAd->BulkOutLock[BulkOutPipeId], IrqFlags);


	pHTTXContext = &(pAd->TxContext[BulkOutPipeId]);

	BULK_OUT_LOCK(&pAd->TxContextQueueLock[BulkOutPipeId], IrqFlags2);
	if ((pHTTXContext->ENextBulkOutPosition == pHTTXContext->CurWritePosition)
#ifdef USB_BULK_BUF_ALIGMENT
		|| ((pHTTXContext->CurWriteRealPos > pHTTXContext->CurWritePosition) &&(pHTTXContext->NextBulkIdx == pHTTXContext->CurWriteIdx) )
#else
		|| ((pHTTXContext->ENextBulkOutPosition-8) == pHTTXContext->CurWritePosition)
#endif /* USB_BULK_BUF_ALIGMENT */
		)  /* druing writing. */
	{
		BULK_OUT_UNLOCK(&pAd->TxContextQueueLock[BulkOutPipeId], IrqFlags2);

		BULK_OUT_LOCK(&pAd->BulkOutLock[BulkOutPipeId], IrqFlags);
		pAd->BulkOutPending[BulkOutPipeId] = FALSE;

		/* Clear Data flag*/
		RTUSB_CLEAR_BULK_FLAG(pAd, (fRTUSB_BULK_OUT_DATA_FRAG << BulkOutPipeId));
		RTUSB_CLEAR_BULK_FLAG(pAd, (fRTUSB_BULK_OUT_DATA_NORMAL << BulkOutPipeId));

		BULK_OUT_UNLOCK(&pAd->BulkOutLock[BulkOutPipeId], IrqFlags);
		return;
	}

	/* Clear Data flag*/
	RTUSB_CLEAR_BULK_FLAG(pAd, (fRTUSB_BULK_OUT_DATA_FRAG << BulkOutPipeId));
	RTUSB_CLEAR_BULK_FLAG(pAd, (fRTUSB_BULK_OUT_DATA_NORMAL << BulkOutPipeId));

	/*
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE,("BulkOut-B:I=0x%lx, CWPos=%ld, CWRPos=%ld, NBPos=%ld, ENBPos=%ld, bCopy=%d!\n",
					in_interrupt(),
					pHTTXContext->CurWritePosition, pHTTXContext->CurWriteRealPos,
					pHTTXContext->NextBulkOutPosition, pHTTXContext->ENextBulkOutPosition,
					pHTTXContext->bCopySavePad));
	*/
	pHTTXContext->NextBulkOutPosition = pHTTXContext->ENextBulkOutPosition;
	ThisBulkSize = 0;
	TmpBulkEndPos = pHTTXContext->NextBulkOutPosition;
	pWirelessPkt = &pHTTXContext->TransferBuffer->field.WirelessPacket[0];

#ifndef USB_BULK_BUF_ALIGMENT
	if ((pHTTXContext->bCopySavePad == TRUE))
	{
		if (RTMPEqualMemory(pHTTXContext->SavedPad, allzero,4))
		{
			DBGPRINT_RAW(DBG_CAT_ALL, DBG_LVL_ERROR,("e1, allzero : %x  %x  %x  %x  %x  %x  %x  %x \n",
				pHTTXContext->SavedPad[0], pHTTXContext->SavedPad[1], pHTTXContext->SavedPad[2],pHTTXContext->SavedPad[3]
				,pHTTXContext->SavedPad[4], pHTTXContext->SavedPad[5], pHTTXContext->SavedPad[6],pHTTXContext->SavedPad[7]));
		}
		NdisMoveMemory(&pWirelessPkt[TmpBulkEndPos], pHTTXContext->SavedPad, 8);
		pHTTXContext->bCopySavePad = FALSE;
		if (pAd->bForcePrintTX == TRUE)
			MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE,("RTUSBBulkOutDataPacket --> COPY PAD. CurWrite = %ld, NextBulk = %ld.   ENextBulk = %ld.\n",
						pHTTXContext->CurWritePosition, pHTTXContext->NextBulkOutPosition, pHTTXContext->ENextBulkOutPosition));
	}
#endif /* USB_BULK_BUF_ALIGMENT */

#ifdef RELEASE_EXCLUDE
#ifdef RTMP_MAC_USB
#ifdef DOT11_N_SUPPORT
#ifdef PLATFORM_RALINK_3052
{
	/*
		Do not use WLAN USB in RT3052 board due to Synopsys host controller
		problem.

		This is for WMM test when any WLAN USB is pluged into RT3052 board to
		do AT/STA device.

		1. One dnlink BK (18Mbps), Two uplinks BE (18Mbps) and BK (22Mbps)
			When we do aggregation for BK traffic, there will have many NAKs
			from USB device, only one pipe for Synopsys host controller,
			these NAKs will cause we can not receive all packets from uplinks.

			And the throughput of uplink BE will be smaller than dnlink BK.

			Solution: No aggregation for dnlink in the case.

		2. Two dnlink BE (18Mbps) and BK (22Mbps), one uplink BK (18Mbps)
			We need to use aggregation function to get higher throughput for
			BE traffic.

		So some cases need aggregation and some cases do not need it.

		==> Final solution:

		1. Only for 20MHz and no BLOCK ACK mechanism in RT3052 platform.

		2. Check how many ACs are used before aggregation.
			If only one AC is used, aggregation function is disabled.
			If more than one AC are used, aggregation function is enabled.
	*/
}
#endif /* PLATFORM_RALINK_3052 */
#endif /* DOT11_N_SUPPORT */
#endif /* RTMP_MAC_USB */
#endif /* RELEASE_EXCLUDE */

	do
	{
#ifdef MT_MAC
		TMAC_TXD_L *txd_l = (TMAC_TXD_L *)&pWirelessPkt[TmpBulkEndPos];
		TMAC_TXD_S *txd_s = (TMAC_TXD_S *)&pWirelessPkt[TmpBulkEndPos];
                TMAC_TXD_0 *txd_0 = &txd_s->txd_0;
		TMAC_TXD_1 *txd_1 = &txd_s->txd_1;
		TMAC_TXD_7 *txd_7;

		if (txd_1->ft == TMI_FT_SHORT)
			txd_7 = &txd_s->txd_7;
		else
			txd_7 = &txd_l->txd_7;
#else
		pTxInfo = (TXINFO_STRUC *)&pWirelessPkt[TmpBulkEndPos];
		pTxWI = (TXWI_STRUC *)&pWirelessPkt[TmpBulkEndPos + TXINFO_SIZE];
#endif /* MT_MAC */

#ifdef RLT_MAC
		if (pAd->chipCap.hif_type == HIF_RLT) {
#ifdef MT7601
			if (IS_MT7601(pAd)) {
				ampdu = pTxWI->TXWI_X.AMPDU;
				phy_mode = pTxWI->TXWI_X.PHYMODE;
				pid = pTxWI->TXWI_X.PacketId;
				txwi_pkt_len = pTxWI->TXWI_X.MPDUtotalByteCnt;
			}
			else
#endif /* MT7601 */
			{
				ampdu = pTxWI->TXWI_N.AMPDU;
				phy_mode = pTxWI->TXWI_N.PHYMODE;
				pid = pTxWI->TXWI_N.TxPktId;
				txwi_pkt_len = pTxWI->TXWI_N.MPDUtotalByteCnt;
			}
		}
#endif /* RLT_MAC */

#ifdef RTMP_MAC
		if (pAd->chipCap.hif_type == HIF_RTMP) {
			ampdu = pTxWI->TXWI_O.AMPDU;
			phy_mode = pTxWI->TXWI_O.PHYMODE;
			pid = pTxWI->TXWI_O.PacketId;
			txwi_pkt_len = pTxWI->TXWI_O.MPDUtotalByteCnt;
		}
#endif /* RTMP_MAC */

		if (pAd->bForcePrintTX == TRUE)
			MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("RTUSBBulkOutDataPacket AMPDU = %d.\n",   ampdu));

		/* add by Iverson, limit BulkOut size to 4k to pass WMM b mode 2T1R test items*/
		/*if ((ThisBulkSize != 0)  && (pTxWI->AMPDU == 0))*/
#ifndef MT_MAC
		if ((ThisBulkSize != 0) && (phy_mode == MODE_CCK))
		{
#ifdef INF_AMAZON_SE
			/*Iverson Add for AMAZON USB (RT2070 &&  RT3070) to pass WMM A2-T4 ~ A2-T10*/
			if(OPSTATUS_TEST_FLAG(pAd, fOP_STATUS_WMM_INUSED))
			{
				/*Iverson patch for WMM A5-T07 ,WirelessStaToWirelessSta do not bulk out aggregate*/
				if(pid == 6)
                {
		            pHTTXContext->ENextBulkOutPosition = TmpBulkEndPos;
					break;
                }
				else if (((ThisBulkSize&0xffff8000) != 0) || ((ThisBulkSize&pAd->BulkOutDataSizeLimit[BulkOutPipeId]) == pAd->BulkOutDataSizeLimit[BulkOutPipeId]))
				{
					pHTTXContext->ENextBulkOutPosition = TmpBulkEndPos;
					break;
				}

#if 0
				else if (BulkOutPipeId == 1)
				{
					/*BK  No Limit BulkOut size .*/
					pHTTXContext->ENextBulkOutPosition = TmpBulkEndPos;
					break;
				}
				else if (((ThisBulkSize&0xffff8000) != 0) || (((ThisBulkSize&0x1000) == 0x1000) &&  (BulkOutPipeId == 0)  ))
				{
					/*BE  Limit BulkOut size to about 4k bytes.*/
					pHTTXContext->ENextBulkOutPosition = TmpBulkEndPos;
					break;
				}
				else if (((ThisBulkSize&0xffff8000) != 0) || (((ThisBulkSize&0x1c00) == 0x1c00) &&  (BulkOutPipeId == 2)  ))
				{
					/*VI Limit BulkOut size to about 7k bytes.*/
					pHTTXContext->ENextBulkOutPosition = TmpBulkEndPos;
					break;
				}
				else if (((ThisBulkSize&0xffff8000) != 0) || (((ThisBulkSize&0x2500) == 0x2500) &&  (BulkOutPipeId == 3)  ))
				{
					/*VO Limit BulkOut size to about 9k bytes.*/
					pHTTXContext->ENextBulkOutPosition = TmpBulkEndPos;
					break;
				}
#endif /*iverson mark*/
			}
			else if (((ThisBulkSize&0xffff8000) != 0) || ((ThisBulkSize&0x1000) == 0x1000))
			{
				/* Limit BulkOut size to about 4k bytes.*/
				pHTTXContext->ENextBulkOutPosition = TmpBulkEndPos;
				break;
			}
#endif /* INF_AMAZON_SE */
#ifndef INF_AMAZON_SE
#ifndef USB_BULK_BUF_ALIGMENT
			if (((ThisBulkSize&0xffff8000) != 0) || ((ThisBulkSize&0x1000) == 0x1000))
			{
				/* Limit BulkOut size to about 4k bytes.*/
				pHTTXContext->ENextBulkOutPosition = TmpBulkEndPos;
				break;
			}
#else
			if (((ThisBulkSize&0xffff8000) != 0) || ((ThisBulkSize&0x6000) == 0x6000))
			{
				/* Limit BulkOut size to about 24k bytes.*/
				pHTTXContext->ENextBulkOutPosition = TmpBulkEndPos;

				/* when bulk size is > 6000, it mean that this is the lasttround at this alignmnet section. */
				bLasAlignmentsectiontRound = TRUE;
				break;
			}

#endif /* USB_BULK_BUF_ALIGMENT */
#endif /* INF_AMAZON_SE */
#ifndef USB_BULK_BUF_ALIGMENT
			else if (((pAd->BulkOutMaxPacketSize < 512) && ((ThisBulkSize&0xfffff800) != 0) ) /*|| ( (ThisBulkSize != 0)  && (pTxWI->AMPDU == 0))*/)
			{
				/* For USB 1.1 or peer which didn't support AMPDU, limit the BulkOut size. */
				/* For performence in b/g mode, now just check for USB 1.1 and didn't care about the APMDU or not! 2008/06/04.*/
				pHTTXContext->ENextBulkOutPosition = TmpBulkEndPos;
				break;
			}

#ifdef CONFIG_MULTI_CHANNEL
			else if  ((pAd->Mlme.bStartMcc == TRUE) &&  ((ThisBulkSize&0xfffff800) != 0) )
			{
				pHTTXContext->ENextBulkOutPosition = TmpBulkEndPos;
				break;
			}
#endif /* CONFIG_MULTI_CHANNEL */

#else
			else if (((pAd->BulkOutMaxPacketSize < 512) && (((ThisBulkSize&0xffff8000) != 0) || ((ThisBulkSize&0x6000) == 0x6000)) ))
			{
				/* Limit BulkOut size to about 24k bytes.*/
				pHTTXContext->ENextBulkOutPosition = TmpBulkEndPos;

				/* when bulk size is > 6000, it mean that this is the lasttround at this alignmnet section. */
				bLasAlignmentsectiontRound = TRUE;
				break;
			}

#endif /* USB_BULK_BUF_ALIGMENT */

		}
		/* end Iverson*/
		else
#endif /* MT_MAC */
		{

#if 0 /*iverson*/
#ifdef INF_AMAZON_SE
#if 0
#ifdef DOT11_N_SUPPORT
			if(((ThisBulkSize&0xffff8000) != 0) || ((ThisBulkSize&0x6000) == 0x6000) || ( (ThisBulkSize != 0)  && (pTxWI->AMPDU == 0)))
			{
				/* AMAZON_SE: BG mode Disable BulkOut Aggregate, N mode BulkOut Aggregaet size 24K */
				pHTTXContext->ENextBulkOutPosition = TmpBulkEndPos;
               break;
			}
			else
#endif /* DOT11_N_SUPPORT */
#endif
/*			{*/
				if(OPSTATUS_TEST_FLAG(pAd, fOP_STATUS_WMM_INUSED) && (pTxWI->AMPDU == 0))
				{
#if 0
					if (((pAd->BulkOutMaxPacketSize < 512) && ((ThisBulkSize&0xfffff800) != 0)) ||
						(ThisBulkSize != 0))
					{
						/* AMAZON_SE: RT2070  Disable BulkOut Aggregate when WMM for USB issue */
						pHTTXContext->ENextBulkOutPosition = TmpBulkEndPos;
						break;
					}
#endif
					if (((ThisBulkSize&0x1400) == 0x1400) && (BulkOutPipeId == 0))
					{
						/*BE  BulkOut Aggregate 5K for AMAZON B/G throughput and PASS WMM (need usb host patch) */
						pHTTXContext->ENextBulkOutPosition = TmpBulkEndPos;
						break;
					}
					else if (((ThisBulkSize&0x2500) == 0x2500) && (BulkOutPipeId == 2))
					{
						/*VI  BulkOut Aggregate 9K for AMAZON B/G  PASS WMM (need usb host patch) */
						pHTTXContext->ENextBulkOutPosition = TmpBulkEndPos;
						break;
					}
					else if (((ThisBulkSize&0x6000) == 0x6000) && (BulkOutPipeId == 3))
					{
						pHTTXContext->ENextBulkOutPosition = TmpBulkEndPos;
						/*V0  BulkOut Aggregate 12K for AMAZON B/G  PASS WMM (need usb host patch) */
						break;
					}
                    else if (((ThisBulkSize != 0) && (BulkOutPipeId == 1) ) )
                    {
						/*BK  BulkOut Disable  Aggregate for AMAZON B/G  PASS WMM (need usb host patch) */
                        pHTTXContext->ENextBulkOutPosition = TmpBulkEndPos;
                        break;
                    }
/*
                                        else if(((pAd->BulkOutMaxPacketSize < 512) && ((ThisBulkSize&0xfffff800) != 0)))
                                        {
                                                pHTTXContext->ENextBulkOutPosition = TmpBulkEndPos;
                                                break;
                                        }

*/
				}
#if 0
				else if (((ThisBulkSize&0xffff8000) != 0) || ((ThisBulkSize&0x6000) == 0x6000))
				{
					/* Limit BulkOut size to about 24k bytes.*/
					pHTTXContext->ENextBulkOutPosition = TmpBulkEndPos;
					break;
				}
			}
#endif
#endif /* INF_AMAZON_SE */
#endif /*iverson*/

		if (((ThisBulkSize & 0xffff8000) != 0) || ((ThisBulkSize&0x6000) == 0x6000))
		{	/* Limit BulkOut size to about 24k bytes.*/
			pHTTXContext->ENextBulkOutPosition = TmpBulkEndPos;
#ifdef USB_BULK_BUF_ALIGMENT
				/* when bulk size is > 0x6000, it mean that this is the lasttround at this alignmnet section. */
				bLasAlignmentsectiontRound = TRUE;
#endif /* USB_BULK_BUF_ALIGMENT */

			break;
		}
#ifdef INF_AMAZON_SE
		else if (((ThisBulkSize&0xffff8000) != 0) || ((ThisBulkSize&pAd->BulkOutDataSizeLimit[BulkOutPipeId]) == pAd->BulkOutDataSizeLimit[BulkOutPipeId]))
		{
			pHTTXContext->ENextBulkOutPosition = TmpBulkEndPos;
			break;
		}
#endif /* INF_AMAZON_SE */
#ifndef USB_BULK_BUF_ALIGMENT
		else if (((pAd->BulkOutMaxPacketSize < 512) && ((ThisBulkSize&0xfffff800) != 0) ) /*|| ( (ThisBulkSize != 0)  && (pTxWI->AMPDU == 0))*/)
		{	/* For USB 1.1 or peer which didn't support AMPDU, limit the BulkOut size. */
			/* For performence in b/g mode, now just check for USB 1.1 and didn't care about the APMDU or not! 2008/06/04.*/
			pHTTXContext->ENextBulkOutPosition = TmpBulkEndPos;
			break;
		}
#else
			else if (((pAd->BulkOutMaxPacketSize < 512) && (((ThisBulkSize&0xffff8000) != 0) || ((ThisBulkSize&0x6000) == 0x6000)) ))
			{
				/* Limit BulkOut size to about 24k bytes. */
				pHTTXContext->ENextBulkOutPosition = TmpBulkEndPos;

				/* when bulk size is > 6000, it mean that this is the lasttround at this alignmnet section. */
				bLasAlignmentsectiontRound = TRUE;
				break;
			}

#endif /* USB_BULK_BUF_ALIGMENT */

		}

		if (TmpBulkEndPos == pHTTXContext->CurWritePosition)
		{
			pHTTXContext->ENextBulkOutPosition = TmpBulkEndPos;
			break;
		}

#ifndef MT_MAC
		if (pTxInfo->TxInfoQSEL != FIFO_EDCA)
		{
			MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("%s(): ====> pTxInfo->QueueSel(%d)!= FIFO_EDCA!!!!\n",
										__FUNCTION__, pTxInfo->TxInfoQSEL));
			MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("\tCWPos=%ld, NBPos=%ld, ENBPos=%ld, bCopy=%d!\n",
										pHTTXContext->CurWritePosition, pHTTXContext->NextBulkOutPosition,
										pHTTXContext->ENextBulkOutPosition, pHTTXContext->bCopySavePad));
			hex_dump("Wrong QSel Pkt:", (PUCHAR)&pWirelessPkt[TmpBulkEndPos], (pHTTXContext->CurWritePosition - pHTTXContext->NextBulkOutPosition));
		}

		if (pTxInfo->TxInfoPktLen <= 8)
		{
			BULK_OUT_UNLOCK(&pAd->TxContextQueueLock[BulkOutPipeId], IrqFlags2);
			MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR /*DBG_LVL_TRACE*/,("e2, TxInfoPktLen==0, Size=%ld, bCSPad=%d, CWPos=%ld, NBPos=%ld, CWRPos=%ld!\n",
					pHTTXContext->BulkOutSize, pHTTXContext->bCopySavePad, pHTTXContext->CurWritePosition, pHTTXContext->NextBulkOutPosition, pHTTXContext->CurWriteRealPos));
			{
				DBGPRINT_RAW(DBG_CAT_ALL, DBG_LVL_ERROR /*DBG_LVL_TRACE*/,("%x  %x  %x  %x  %x  %x  %x  %x \n",
					pHTTXContext->SavedPad[0], pHTTXContext->SavedPad[1], pHTTXContext->SavedPad[2],pHTTXContext->SavedPad[3]
					,pHTTXContext->SavedPad[4], pHTTXContext->SavedPad[5], pHTTXContext->SavedPad[6],pHTTXContext->SavedPad[7]));
			}
			pAd->bForcePrintTX = TRUE;
			BULK_OUT_LOCK(&pAd->BulkOutLock[BulkOutPipeId], IrqFlags);
			pAd->BulkOutPending[BulkOutPipeId] = FALSE;
			BULK_OUT_UNLOCK(&pAd->BulkOutLock[BulkOutPipeId], IrqFlags);
			/*MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_LOUD,("Out:pTxInfo->TxInfoPktLen=%d!\n", pTxInfo->TxInfoPktLen));*/
			return;
		}

		/* Increase Total transmit byte counter*/
		pAd->RalinkCounters.OneSecTransmittedByteCount +=  txwi_pkt_len;
		pAd->RalinkCounters.TransmittedByteCount +=  txwi_pkt_len;

		pLastTxInfo = pTxInfo;

		/* Make sure we use EDCA QUEUE.  */
		pTxInfo->TxInfoQSEL = FIFO_EDCA;
		ThisBulkSize += (pTxInfo->TxInfoPktLen+4);
		TmpBulkEndPos += (pTxInfo->TxInfoPktLen+4);

		if (TmpBulkEndPos != pHTTXContext->CurWritePosition)
			pTxInfo->TxInfoUDMANextVld = 1;
#else
		padding = (4 - (txd_0->tx_byte_cnt % 4)) & 0x03;
		ThisBulkSize += txd_0->tx_byte_cnt+padding;
		TmpBulkEndPos += txd_0->tx_byte_cnt+padding;
#endif /* MT_MAC */


#ifdef USB_BULK_BUF_ALIGMENT
/*
		this is for frag packet , because it will finish this section
		when ((((pHTTXContext->CurWritePosition + 3906)& 0x00007fff) & 0xffff6000) == 0x00006000)
*/
		if (pTxInfo->bFragLasAlignmentsectiontRound == 1)
		{
			bLasAlignmentsectiontRound = TRUE;
			break;
		}
#else
#ifndef MT_MAC
		if (pTxInfo->TxInfoSwLstRnd == 1)
		{
			if (pHTTXContext->CurWritePosition == 8)
				pTxInfo->TxInfoUDMANextVld = 0;
			pTxInfo->TxInfoSwLstRnd = 0;

			bTxQLastRound = TRUE;
			pHTTXContext->ENextBulkOutPosition = 8;

	#ifdef RT_BIG_ENDIAN
			RTMPDescriptorEndianChange((PUCHAR)pTxInfo, TYPE_TXINFO);
			RTMPWIEndianChange(pAd, (PUCHAR)pTxWI, TYPE_TXWI);
	#endif /* RT_BIG_ENDIAN */

			break;
		}
#else
		if (txd_7->sw_field == 1)
		{
			txd_7->sw_field = 0;
			bTxQLastRound = TRUE;
			pHTTXContext->ENextBulkOutPosition = 8;
			break;
		}
#endif /* MT_MAC */
#endif /* USB_BULK_BUF_ALIGMENT */
#ifdef RT_BIG_ENDIAN
		RTMPDescriptorEndianChange((PUCHAR)pTxInfo, TYPE_TXINFO);
		RTMPWIEndianChange(pAd, (PUCHAR)pTxWI, TYPE_TXWI);
#endif /* RT_BIG_ENDIAN */

		aggregation_num++;

		if ((aggregation_num == 1) && (!pAd->usb_ctl.usb_aggregation)) {
			pHTTXContext->ENextBulkOutPosition = TmpBulkEndPos;
			break;
		}
	}while (TRUE);

#ifdef CONFIG_DVT_MODE
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("%s::aggregation_num(%d), usb_aggregation(%d)\n", __FUNCTION__, aggregation_num, pAd->usb_ctl.usb_aggregation));
#endif /* CONFIG_DVT_MODE */

#ifndef MT_MAC
	/* adjust the pTxInfo->TxInfoUDMANextVld value of last pTxInfo.*/
	if (pLastTxInfo)
	{
#ifdef RT_BIG_ENDIAN
		RTMPDescriptorEndianChange((PUCHAR)pLastTxInfo, TYPE_TXINFO);
#endif /* RT_BIG_ENDIAN */
		pLastTxInfo->TxInfoUDMANextVld = 0;
#ifdef RT_BIG_ENDIAN
		RTMPDescriptorEndianChange((PUCHAR)pLastTxInfo, TYPE_TXINFO);
#endif /* RT_BIG_ENDIAN */
	}
#endif /* MT_MAC */

	/*
		We need to copy SavedPad when following condition matched!
			1. Not the last round of the TxQueue and
			2. any match of following cases:
				(1). The End Position of this bulk out is reach to the Currenct Write position and
						the TxInfo and related header already write to the CurWritePosition.
			   		=>(ENextBulkOutPosition == CurWritePosition) && (CurWriteRealPos > CurWritePosition)

				(2). The EndPosition of the bulk out is not reach to the Current Write Position.
					=>(ENextBulkOutPosition != CurWritePosition)
	*/
#ifndef USB_BULK_BUF_ALIGMENT
	if ((bTxQLastRound == FALSE) &&
		 (((pHTTXContext->ENextBulkOutPosition == pHTTXContext->CurWritePosition) && (pHTTXContext->CurWriteRealPos > pHTTXContext->CurWritePosition)) ||
		  (pHTTXContext->ENextBulkOutPosition != pHTTXContext->CurWritePosition))
		)
	{
		NdisMoveMemory(pHTTXContext->SavedPad, &pWirelessPkt[pHTTXContext->ENextBulkOutPosition], 8);
		pHTTXContext->bCopySavePad = TRUE;
		if (RTMPEqualMemory(pHTTXContext->SavedPad, allzero,4))
		{
			PUCHAR	pBuf = &pHTTXContext->SavedPad[0];
			DBGPRINT_RAW(DBG_CAT_ALL, DBG_LVL_ERROR,("WARNING-Zero-3:%02x%02x%02x%02x%02x%02x%02x%02x,CWPos=%ld, CWRPos=%ld, bCW=%d, NBPos=%ld, TBPos=%ld, TBSize=%ld\n",
				pBuf[0], pBuf[1], pBuf[2],pBuf[3],pBuf[4], pBuf[5], pBuf[6],pBuf[7], pHTTXContext->CurWritePosition, pHTTXContext->CurWriteRealPos,
				pHTTXContext->bCurWriting, pHTTXContext->NextBulkOutPosition, TmpBulkEndPos, ThisBulkSize));

			pBuf = &pWirelessPkt[pHTTXContext->CurWritePosition];
			DBGPRINT_RAW(DBG_CAT_ALL, DBG_LVL_ERROR,("\tCWPos=%02x%02x%02x%02x%02x%02x%02x%02x\n", pBuf[0], pBuf[1], pBuf[2],pBuf[3],pBuf[4], pBuf[5], pBuf[6],pBuf[7]));
		}
		/*MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_LOUD,("ENPos==CWPos=%ld, CWRPos=%ld, bCSPad=%d!\n", pHTTXContext->CurWritePosition, pHTTXContext->CurWriteRealPos, pHTTXContext->bCopySavePad));*/
	}
#endif /* USB_BULK_BUF_ALIGMENT */

	if (pAd->bForcePrintTX == TRUE)
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE,("BulkOut-A:Size=%ld, CWPos=%ld, NBPos=%ld, ENBPos=%ld, bCopy=%d!\n", ThisBulkSize, pHTTXContext->CurWritePosition, pHTTXContext->NextBulkOutPosition, pHTTXContext->ENextBulkOutPosition, pHTTXContext->bCopySavePad));
	/*MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_LOUD,("BulkOut-A:Size=%ld, CWPos=%ld, CWRPos=%ld, NBPos=%ld, ENBPos=%ld, bCopy=%d, bLRound=%d!\n", ThisBulkSize, pHTTXContext->CurWritePosition, pHTTXContext->CurWriteRealPos, pHTTXContext->NextBulkOutPosition, pHTTXContext->ENextBulkOutPosition, pHTTXContext->bCopySavePad, bTxQLastRound));*/

		/* USB DMA engine requires to pad extra 4 bytes. This pad doesn't count into real bulkoutsize.*/
	pAppendant = &pWirelessPkt[TmpBulkEndPos];
	NdisZeroMemory(pAppendant, 8);
		ThisBulkSize += 4;
		pHTTXContext->LastOne = TRUE;

	pHTTXContext->BulkOutSize = ThisBulkSize;
#ifdef USB_BULK_BUF_ALIGMENT
	/*
		if it is the last alignment section round,that we just need to add nextbulkindex,
		otherwise we both need to add  nextbulkindex and CurWriteIdx
		(because when alignment section round happened, the CurWriteIdx is added at function writing resource.)
	*/
	if(bLasAlignmentsectiontRound == TRUE)
	{
			pHTTXContext->ENextBulkOutPosition = ((CUR_WRITE_IDX_INC(pHTTXContext->NextBulkIdx, BUF_ALIGMENT_RINGSIZE)) * 0x8000);
	}
	else
	{
			pHTTXContext->ENextBulkOutPosition = ((CUR_WRITE_IDX_INC(pHTTXContext->NextBulkIdx, BUF_ALIGMENT_RINGSIZE)) * 0x8000);
			pHTTXContext->CurWritePosition = ((CUR_WRITE_IDX_INC(pHTTXContext->CurWriteIdx, BUF_ALIGMENT_RINGSIZE)) * 0x8000);
	 }

#endif /* USB_BULK_BUF_ALIGMENT */


	pAd->watchDogTxPendingCnt[BulkOutPipeId] = 1;
	BULK_OUT_UNLOCK(&pAd->TxContextQueueLock[BulkOutPipeId], IrqFlags2);

	/* Init Tx context descriptor*/
	RTUSBInitHTTxDesc(pAd, pHTTXContext, BulkOutPipeId, ThisBulkSize, (usb_complete_t)RtmpUsbBulkOutDataPacketComplete);

	pUrb = pHTTXContext->pUrb;
	if((ret = RTUSB_SUBMIT_URB(pUrb))!=0)
	{
#ifdef RELEASE_EXCLUDE
		/*
			Bug history:
			1. Webcom
				When submit non-BE packet, the function will fail here.
				After EEPROM content is updated, everything is ok.
		*/
#endif /* RELEASE_EXCLUDE */
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("RTUSBBulkOutDataPacket: Submit Tx URB failed %d\n", ret));

		BULK_OUT_LOCK(&pAd->BulkOutLock[BulkOutPipeId], IrqFlags);
		pAd->BulkOutPending[BulkOutPipeId] = FALSE;
		pAd->watchDogTxPendingCnt[BulkOutPipeId] = 0;
		BULK_OUT_UNLOCK(&pAd->BulkOutLock[BulkOutPipeId], IrqFlags);

		return;
	}

	BULK_OUT_LOCK(&pAd->BulkOutLock[BulkOutPipeId], IrqFlags);
	pHTTXContext->IRPPending = TRUE;
	BULK_OUT_UNLOCK(&pAd->BulkOutLock[BulkOutPipeId], IrqFlags);
	pAd->BulkOutReq++;
	pTxWI = pTxInfo = NULL;
}


USBHST_STATUS RTUSBBulkOutDataPacketComplete(URBCompleteStatus Status, purbb_t pURB, pregs *pt_regs)
{
	PHT_TX_CONTEXT	pHTTXContext;
	PRTMP_ADAPTER	pAd;
	POS_COOKIE 		pObj;
	UCHAR			BulkOutPipeId;


	pHTTXContext	= (PHT_TX_CONTEXT)RTMP_OS_USB_CONTEXT_GET(pURB);
	pAd 			= pHTTXContext->pAd;
	pObj 			= (POS_COOKIE) pAd->OS_Cookie;

	/* Store BulkOut PipeId*/
	BulkOutPipeId	= pHTTXContext->BulkOutPipeId;
	pAd->BulkOutDataOneSecCount++;

	switch (BulkOutPipeId)
	{
		case EDCA_AC0_PIPE:
#ifdef CONFIG_ATE
				if (!ATE_ON(pAd))
				{
#endif /* CONFIG_ATE */
					RTMP_NET_TASK_DATA_ASSIGN(&pObj->ac0_dma_done_task, (unsigned long)pURB);
					RTMP_OS_TASKLET_SCHE(&pObj->ac0_dma_done_task);
#ifdef CONFIG_ATE
				}
				else
				{
					RTMP_NET_TASK_DATA_ASSIGN(&pObj->ate_ac0_dma_done_task, (unsigned long)pURB);
					RTMP_OS_TASKLET_SCHE(&pObj->ate_ac0_dma_done_task);
				}
#endif /* CONFIG_ATE */

				break;
		case EDCA_AC1_PIPE:
				RTMP_NET_TASK_DATA_ASSIGN(&pObj->ac1_dma_done_task, (unsigned long)pURB);
				RTMP_OS_TASKLET_SCHE(&pObj->ac1_dma_done_task);
				break;
		case EDCA_AC2_PIPE:
				RTMP_NET_TASK_DATA_ASSIGN(&pObj->ac2_dma_done_task, (unsigned long)pURB);
				RTMP_OS_TASKLET_SCHE(&pObj->ac2_dma_done_task);
				break;
		case EDCA_AC3_PIPE:
				RTMP_NET_TASK_DATA_ASSIGN(&pObj->ac3_dma_done_task, (unsigned long)pURB);
				RTMP_OS_TASKLET_SCHE(&pObj->ac3_dma_done_task);
				break;
		case HCCA_PIPE:
				RTMP_NET_TASK_DATA_ASSIGN(&pObj->hcca_dma_done_task, (unsigned long)pURB);
				RTMP_OS_TASKLET_SCHE(&pObj->hcca_dma_done_task);
				break;
#ifdef MULTI_WMM_SUPPORT
		case EDCA_WMM1_AC0_PIPE:
				RTMP_NET_TASK_DATA_ASSIGN(&pObj->wmm1_ac0_dma_done_task, (unsigned long)pURB);
				RTMP_OS_TASKLET_SCHE(&pObj->wmm1_ac0_dma_done_task);
				break;
#endif /* MULTI_WMM_SUPPORT */
	}

#ifdef RELEASE_EXCLUDE
	DBGPRINT_RAW(DBG_CAT_ALL, DBG_LVL_INFO, ("<---RTUSBBulkOutDataPacketComplete\n"));
#endif /* RELEASE_EXCLUDE */

}


/*
	========================================================================

	Routine Description:

	Arguments:

	Return Value:

	Note: NULL frame use BulkOutPipeId = 0

	========================================================================
*/
VOID	RTUSBBulkOutNullFrame(
	IN	PRTMP_ADAPTER	pAd)
{
	PTX_CONTEXT		pNullContext = &(pAd->NullContext);
	PURB			pUrb;
	int				ret = 0;
	ULONG	IrqFlags = 0;

	RTMP_IRQ_LOCK(&pAd->BulkOutLock[0], IrqFlags);
	if ((pAd->BulkOutPending[0] == TRUE) || RTMP_TEST_FLAG(pAd, fRTMP_ADAPTER_NEED_STOP_TX))
	{
		RTMP_IRQ_UNLOCK(&pAd->BulkOutLock[0], IrqFlags);
		return;
	}
	pAd->BulkOutPending[0] = TRUE;
	pAd->watchDogTxPendingCnt[0] = 1;
	pNullContext->IRPPending = TRUE;
	RTMP_IRQ_UNLOCK(&pAd->BulkOutLock[0], IrqFlags);

	/* Increase Total transmit byte counter*/
	pAd->RalinkCounters.TransmittedByteCount +=  pNullContext->BulkOutSize;


	/* Clear Null frame bulk flag*/
	RTUSB_CLEAR_BULK_FLAG(pAd, fRTUSB_BULK_OUT_DATA_NULL);

#ifdef RT_BIG_ENDIAN
	RTMPDescriptorEndianChange((PUCHAR)pNullContext->TransferBuffer, TYPE_TXINFO);
#endif /* RT_BIG_ENDIAN */

	/* Init Tx context descriptor*/
	RTUSBInitTxDesc(pAd, pNullContext, 0, (usb_complete_t)RtmpUsbBulkOutNullFrameComplete);

	pUrb = pNullContext->pUrb;
	if((ret = RTUSB_SUBMIT_URB(pUrb))!=0)
	{
		RTMP_IRQ_LOCK(&pAd->BulkOutLock[0], IrqFlags);
		pAd->BulkOutPending[0] = FALSE;
		pAd->watchDogTxPendingCnt[0] = 0;
		pNullContext->IRPPending = FALSE;
		RTMP_IRQ_UNLOCK(&pAd->BulkOutLock[0], IrqFlags);

		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("RTUSBBulkOutNullFrame: Submit Tx URB failed %d\n", ret));
		return;
	}

}

/* NULL frame use BulkOutPipeId = 0*/
USBHST_STATUS RTUSBBulkOutNullFrameComplete(URBCompleteStatus Status, purbb_t pURB, pregs *pt_regs)
{
	PRTMP_ADAPTER		pAd;
	PTX_CONTEXT			pNullContext;
	NTSTATUS			Status;
	POS_COOKIE			pObj;


#ifdef RELEASE_EXCLUDE
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_INFO, ("--->RTUSBBulkOutNullFrameComplete\n"));
#endif /* RELEASE_EXCLUDE */
	pNullContext	= (PTX_CONTEXT)RTMP_OS_USB_CONTEXT_GET(pURB);
	pAd 			= pNullContext->pAd;
	Status 			= RTMP_OS_USB_STATUS_GET(pURB); /*->rtusb_urb_status;*/

	pObj = (POS_COOKIE) pAd->OS_Cookie;
	RTMP_NET_TASK_DATA_ASSIGN(&pObj->null_frame_complete_task, (unsigned long)pURB);
	RTMP_OS_TASKLET_SCHE(&pObj->null_frame_complete_task);

#ifdef RELEASE_EXCLUDE
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_INFO, ("<---RTUSBBulkOutNullFrameComplete\n"));
#endif /* RELEASE_EXCLUDE */
}


/*
	========================================================================

	Routine Description:

	Arguments:

	Return Value:

	Note: MLME use BulkOutPipeId = 0

	========================================================================
*/
VOID	RTUSBBulkOutMLMEPacket(
	IN	PRTMP_ADAPTER	pAd,
	IN	UCHAR			Index)
{
	PTX_CONTEXT		pMLMEContext;
	PURB			pUrb;
	int				ret = 0;
	ULONG	IrqFlags = 0;

	pMLMEContext = (PTX_CONTEXT)pAd->MgmtRing.Cell[pAd->MgmtRing.TxDmaIdx].AllocVa;
	pUrb = pMLMEContext->pUrb;

	if ((pAd->MgmtRing.TxSwFreeIdx >= MGMT_RING_SIZE) ||
		(pMLMEContext->InUse == FALSE) ||
		(pMLMEContext->bWaitingBulkOut == FALSE))
	{
#ifdef RELEASE_EXCLUDE
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_INFO, ("Try to BulkOut coherent MgmtPkt(D=%d, C=%d, F=%d, InUse=%d, bWaitingBulkOut=%d)!\n",
				pAd->MgmtRing.TxDmaIdx, pAd->MgmtRing.TxCpuIdx, pAd->MgmtRing.TxSwFreeIdx,
				pMLMEContext->InUse, pMLMEContext->bWaitingBulkOut));
#endif /* RELEASE_EXCLUDE */

#ifdef CONFIG_DVT_MODE
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("%s::TxSwFreeIdx(%d,%d), InUse(%d), bWaitingBulkOut(%d)\n",
								__FUNCTION__,
								pAd->MgmtRing.TxSwFreeIdx,
								MGMT_RING_SIZE,
								pMLMEContext->InUse,
								pMLMEContext->bWaitingBulkOut));
#endif /* CONFIG_DVT_MODE */

		/* Clear MLME bulk flag*/
		RTUSB_CLEAR_BULK_FLAG(pAd, fRTUSB_BULK_OUT_MLME);

		return;
	}


	RTMP_IRQ_LOCK(&pAd->BulkOutLock[MGMTPIPEIDX], IrqFlags);
	if ((pAd->BulkOutPending[MGMTPIPEIDX] == TRUE) || RTMP_TEST_FLAG(pAd, fRTMP_ADAPTER_NEED_STOP_TX))
	{
		RTMP_IRQ_UNLOCK(&pAd->BulkOutLock[MGMTPIPEIDX], IrqFlags);
		return;
	}

	pAd->BulkOutPending[MGMTPIPEIDX] = TRUE;
	pAd->watchDogTxPendingCnt[MGMTPIPEIDX] = 1;
	pMLMEContext->IRPPending = TRUE;
	pMLMEContext->bWaitingBulkOut = FALSE;
	RTMP_IRQ_UNLOCK(&pAd->BulkOutLock[MGMTPIPEIDX], IrqFlags);

	/* Increase Total transmit byte counter*/
	pAd->RalinkCounters.TransmittedByteCount +=  pMLMEContext->BulkOutSize;

	/* Clear MLME bulk flag*/
	RTUSB_CLEAR_BULK_FLAG(pAd, fRTUSB_BULK_OUT_MLME);

#ifdef RT_BIG_ENDIAN
	RTMPDescriptorEndianChange((PUCHAR)pMLMEContext->TransferBuffer, TYPE_TXINFO);
#endif /* RT_BIG_ENDIAN */

	/* Init Tx context descriptor*/
	RTUSBInitTxDesc(pAd, pMLMEContext, MGMTPIPEIDX, (usb_complete_t)RtmpUsbBulkOutMLMEPacketComplete);

	RTUSB_URB_DMA_MAPPING(pUrb);

	pUrb = pMLMEContext->pUrb;
	if((ret = RTUSB_SUBMIT_URB(pUrb))!=0)
	{
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("RTUSBBulkOutMLMEPacket: Submit MLME URB failed %d\n", ret));
		RTMP_IRQ_LOCK(&pAd->BulkOutLock[MGMTPIPEIDX], IrqFlags);
		pAd->BulkOutPending[MGMTPIPEIDX] = FALSE;
		pAd->watchDogTxPendingCnt[MGMTPIPEIDX] = 0;
		pMLMEContext->IRPPending = FALSE;
		pMLMEContext->bWaitingBulkOut = TRUE;
		RTMP_IRQ_UNLOCK(&pAd->BulkOutLock[MGMTPIPEIDX], IrqFlags);

		return;
	}
}


USBHST_STATUS RTUSBBulkOutMLMEPacketComplete(URBCompleteStatus Status, purbb_t pURB, pregs *pt_regs)
{
	PTX_CONTEXT			pMLMEContext;
	PRTMP_ADAPTER		pAd;
	NTSTATUS			Status;
	POS_COOKIE 			pObj;
	int					index;

	pMLMEContext	= (PTX_CONTEXT)RTMP_OS_USB_CONTEXT_GET(pURB);
	pAd 			= pMLMEContext->pAd;
	pObj 			= (POS_COOKIE)pAd->OS_Cookie;
	Status			= RTMP_OS_USB_STATUS_GET(pURB);
	index 			= pMLMEContext->SelfIdx;

	RTMP_NET_TASK_DATA_ASSIGN(&pObj->mgmt_dma_done_task, (unsigned long)pURB);
	RTMP_OS_TASKLET_SCHE(&pObj->mgmt_dma_done_task);
}

#ifdef MT_MAC
VOID	RTUSBBulkOutBCNPacket(
	IN	PRTMP_ADAPTER	pAd,
	IN	UCHAR			Index)
{
	PTX_CONTEXT		pBcnContext;
	PURB			pUrb;
	int				ret = 0;
	ULONG	IrqFlags = 0;

	pBcnContext = (PTX_CONTEXT)pAd->BcnRing.Cell[pAd->BcnRing.TxDmaIdx].AllocVa;
	pUrb = pBcnContext->pUrb;

	if ((pAd->BcnRing.TxSwFreeIdx >= BCN_RING_SIZE) ||
		(pBcnContext->InUse == FALSE) ||
		(pBcnContext->bWaitingBulkOut == FALSE))
	{
#ifdef RELEASE_EXCLUDE
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_INFO, ("Try to BulkOut coherent BcnPkt(D=%d, C=%d, F=%d, InUse=%d, bWaitingBulkOut=%d)!\n",
				pAd->BcnRing.TxDmaIdx, pAd->BcnRing.TxCpuIdx, pAd->BcnRing.TxSwFreeIdx,
				pBcnContext->InUse, pBcnContext->bWaitingBulkOut));
#endif /* RELEASE_EXCLUDE */


		/* Clear BCN bulk flag*/
		RTUSB_CLEAR_BULK_FLAG(pAd, fRTUSB_BULK_OUT_BCN);

		return;
	}


	RTMP_IRQ_LOCK(&pAd->BulkOutLock[MGMTPIPEIDX], IrqFlags);
	if ((pAd->BulkOutPending[MGMTPIPEIDX] == TRUE) || RTMP_TEST_FLAG(pAd, fRTMP_ADAPTER_NEED_STOP_TX))
	{
		RTMP_IRQ_UNLOCK(&pAd->BulkOutLock[MGMTPIPEIDX], IrqFlags);
		return;
	}

	pAd->BulkOutPending[MGMTPIPEIDX] = TRUE;
	pAd->watchDogTxPendingCnt[MGMTPIPEIDX] = 1;
	pBcnContext->IRPPending = TRUE;
	pBcnContext->bWaitingBulkOut = FALSE;
	RTMP_IRQ_UNLOCK(&pAd->BulkOutLock[MGMTPIPEIDX], IrqFlags);

	/* Increase Total transmit byte counter*/
	pAd->RalinkCounters.TransmittedByteCount +=  pBcnContext->BulkOutSize;

	/* Clear MLME bulk flag*/
	RTUSB_CLEAR_BULK_FLAG(pAd, fRTUSB_BULK_OUT_BCN);

#ifdef RT_BIG_ENDIAN
	RTMPDescriptorEndianChange((PUCHAR)pBcnContext->TransferBuffer, TYPE_TXINFO);
#endif /* RT_BIG_ENDIAN */

	/* Init Tx context descriptor*/
	RTUSBInitTxDesc(pAd, pBcnContext, MGMTPIPEIDX, (usb_complete_t)RTUSBBulkOutBCNPacketComplete);

	RTUSB_URB_DMA_MAPPING(pUrb);

	pUrb = pBcnContext->pUrb;
	if((ret = RTUSB_SUBMIT_URB(pUrb))!=0)
	{
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("RTUSBBulkOutBCNPacket: Submit BCN URB failed %d\n", ret));
		RTMP_IRQ_LOCK(&pAd->BulkOutLock[MGMTPIPEIDX], IrqFlags);
		pAd->BulkOutPending[MGMTPIPEIDX] = FALSE;
		pAd->watchDogTxPendingCnt[MGMTPIPEIDX] = 0;
		pBcnContext->IRPPending = FALSE;
		pBcnContext->bWaitingBulkOut = TRUE;
		RTMP_IRQ_UNLOCK(&pAd->BulkOutLock[MGMTPIPEIDX], IrqFlags);
		return;
	}
}

USBHST_STATUS RTUSBBulkOutBCNPacketComplete(URBCompleteStatus Status, purbb_t pURB, pregs *pt_regs)
{
	PTX_CONTEXT			pBcnContext;
	PRTMP_ADAPTER		pAd;
	NTSTATUS			Status;
	POS_COOKIE 			pObj;

	pBcnContext	= (PTX_CONTEXT)RTMP_OS_USB_CONTEXT_GET(pURB);
	pAd 			= pBcnContext->pAd;
	pObj 			= (POS_COOKIE)pAd->OS_Cookie;
	Status			= RTMP_OS_USB_STATUS_GET(pURB);

	RTMP_NET_TASK_DATA_ASSIGN(&pObj->bcn_dma_done_task, (unsigned long)pURB);
	RTMP_OS_TASKLET_SCHE(&pObj->bcn_dma_done_task);
}
#endif /* MT_MAC */
/*
	========================================================================

	Routine Description:

	Arguments:

	Return Value:

	Note: PsPoll use BulkOutPipeId = 0

	========================================================================
*/
VOID	RTUSBBulkOutPsPoll(
	IN	PRTMP_ADAPTER	pAd)
{
	PTX_CONTEXT		pPsPollContext = &(pAd->PsPollContext);
	PURB			pUrb;
	int				ret = 0;
	ULONG	IrqFlags = 0;

	RTMP_IRQ_LOCK(&pAd->BulkOutLock[0], IrqFlags);
	if ((pAd->BulkOutPending[0] == TRUE) || RTMP_TEST_FLAG(pAd, fRTMP_ADAPTER_NEED_STOP_TX))
	{
		RTMP_IRQ_UNLOCK(&pAd->BulkOutLock[0], IrqFlags);
		return;
	}
	pAd->BulkOutPending[0] = TRUE;
	pAd->watchDogTxPendingCnt[0] = 1;
	pPsPollContext->IRPPending = TRUE;
	RTMP_IRQ_UNLOCK(&pAd->BulkOutLock[0], IrqFlags);


	/* Clear PS-Poll bulk flag*/
	RTUSB_CLEAR_BULK_FLAG(pAd, fRTUSB_BULK_OUT_PSPOLL);

#ifdef RT_BIG_ENDIAN
	RTMPDescriptorEndianChange((PUCHAR)pPsPollContext->TransferBuffer, TYPE_TXINFO);
#endif /* RT_BIG_ENDIAN */

	/* Init Tx context descriptor*/
	RTUSBInitTxDesc(pAd, pPsPollContext, /*MGMTPIPEIDX*/0, (usb_complete_t)RtmpUsbBulkOutPsPollComplete);

	pUrb = pPsPollContext->pUrb;
	if((ret = RTUSB_SUBMIT_URB(pUrb))!=0)
	{
		RTMP_IRQ_LOCK(&pAd->BulkOutLock[0], IrqFlags);
		pAd->BulkOutPending[0] = FALSE;
		pAd->watchDogTxPendingCnt[0] = 0;
		pPsPollContext->IRPPending = FALSE;
		RTMP_IRQ_UNLOCK(&pAd->BulkOutLock[0], IrqFlags);

		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("RTUSBBulkOutPsPoll: Submit Tx URB failed %d\n", ret));
		return;
	}

}

/* PS-Poll frame use BulkOutPipeId = 0*/
USBHST_STATUS RTUSBBulkOutPsPollComplete(URBCompleteStatus Status, purbb_t pURB, pregs *pt_regs)
{
	PRTMP_ADAPTER		pAd;
	PTX_CONTEXT			pPsPollContext;
	NTSTATUS			Status;
	POS_COOKIE			pObj;


#ifdef RELEASE_EXCLUDE
	DBGPRINT_RAW(DBG_CAT_ALL, DBG_LVL_INFO, ("--->RTUSBBulkOutPsPollComplete\n"));
#endif /* RELEASE_EXCLUDE */
	pPsPollContext= (PTX_CONTEXT)RTMP_OS_USB_CONTEXT_GET(pURB);
	pAd = pPsPollContext->pAd;
	Status = RTMP_OS_USB_STATUS_GET(pURB);

	pObj = (POS_COOKIE) pAd->OS_Cookie;
	RTMP_NET_TASK_DATA_ASSIGN(&pObj->pspoll_frame_complete_task, (unsigned long)pURB);
	RTMP_OS_TASKLET_SCHE(&pObj->pspoll_frame_complete_task);

#ifdef RELEASE_EXCLUDE
	DBGPRINT_RAW(DBG_CAT_ALL, DBG_LVL_INFO, ("<---RTUSBBulkOutPsPollComplete\n"));
#endif /* RELEASE_EXCLUDE */
}


NDIS_SPIN_LOCK *RTUSBGetSpinLock(PRTMP_ADAPTER pAd, MT_DL_LIST *list)
{
	NDIS_SPIN_LOCK *Lock = NULL;

	if (list == &pAd->RxFreeQ)
		Lock = &pAd->RxFreeQLock;
	else if (list == &pAd->RxProcessingQ)
		Lock = &pAd->RxProcessingQLock;
	else if (list == &pAd->RxBulkInQ)
		Lock = &pAd->RxBulkInQLock;
	else
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("%s:illegal list\n", __FUNCTION__));

	return Lock;
}


VOID _RTUSBUnlinkRxContext(PRX_CONTEXT pRxContext, MT_DL_LIST *list)
{
	if (!pRxContext)
		return;

	DlListDel(&pRxContext->list);
}


VOID RTUSBUnlinkRxContext(PRTMP_ADAPTER pAd, PRX_CONTEXT pRxContext, MT_DL_LIST *list)
{
	unsigned long flags;
	NDIS_SPIN_LOCK *Lock;

	Lock = RTUSBGetSpinLock(pAd, list);

	RTMP_SPIN_LOCK_IRQSAVE(Lock, &flags);
	_RTUSBUnlinkRxContext(pRxContext, list);
	RTMP_SPIN_UNLOCK_IRQRESTORE(Lock, &flags);
}


PRX_CONTEXT _RTUSBDequeueRxContext(MT_DL_LIST *list)
{
	PRX_CONTEXT pRxContext;

	pRxContext = DlListFirst(list, RX_CONTEXT, list);

	_RTUSBUnlinkRxContext(pRxContext, list);

	return pRxContext;
}


PRX_CONTEXT RTUSBDequeueRxContext(PRTMP_ADAPTER pAd, MT_DL_LIST *list)
{
	unsigned long flags;
	PRX_CONTEXT *RxContext;
	NDIS_SPIN_LOCK *Lock;

	Lock = RTUSBGetSpinLock(pAd, list);

	RTMP_SPIN_LOCK_IRQSAVE(Lock, &flags);
	RxContext = _RTUSBDequeueRxContext(list);
	RTMP_SPIN_UNLOCK_IRQRESTORE(Lock, &flags);

	return RxContext;
}


VOID _RTUSBQueueTailRxContext(MT_DL_LIST *list, PRX_CONTEXT pRxContext)
{
	DlListAddTail(list, &pRxContext->list);
}


VOID RTUSBQueueTailRxContext(PRTMP_ADAPTER pAd, MT_DL_LIST *list, PRX_CONTEXT pRxContext)
{
	unsigned long flags;
	NDIS_SPIN_LOCK *Lock;

	Lock = RTUSBGetSpinLock(pAd, list);

	RTMP_SPIN_LOCK_IRQSAVE(Lock, &flags);
	_RTUSBQueueTailRxContext(list, pRxContext);
	RTMP_SPIN_UNLOCK_IRQRESTORE(Lock, &flags);
}


UINT32 RTUSBQueueLen(PRTMP_ADAPTER pAd, MT_DL_LIST *list)
{
	UINT32 qlen;
	unsigned long flags;
	NDIS_SPIN_LOCK *Lock;

	Lock = RTUSBGetSpinLock(pAd, list);

	RTMP_SPIN_LOCK_IRQSAVE(Lock, &flags);
	qlen = DlListLen(list);
	RTMP_SPIN_UNLOCK_IRQRESTORE(Lock, &flags);

	return qlen;
}

#if 0 /* defined but not used @20140519 */
static INT32 RTUSBQueueEmpty(PRTMP_ADAPTER pAd, MT_DL_LIST *list)
{
	unsigned long flags;
	int is_empty;
	NDIS_SPIN_LOCK *Lock;

	Lock = RTUSBGetSpinLock(pAd, list);

	RTMP_SPIN_LOCK_IRQSAVE(Lock, &flags);
	is_empty = DlListEmpty(list);
	RTMP_SPIN_UNLOCK_IRQRESTORE(Lock, &flags);

	return is_empty;
}
#endif

VOID DoBulkIn(IN RTMP_ADAPTER *pAd)
{
	PRX_CONTEXT pRxContext;
	PURB pUrb;
	int ret = 0;

	while ((RTUSBQueueLen(pAd, &pAd->RxBulkInQ) < 2) &&
			(pRxContext = RTUSBDequeueRxContext(pAd, &pAd->RxFreeQ)))
	{
		RTUSBInitRxDesc(pAd, pRxContext);

		pUrb = pRxContext->pUrb;

		RTUSBQueueTailRxContext(pAd, &pAd->RxBulkInQ, pRxContext);

		if ((ret = RTUSB_SUBMIT_URB(pUrb)) != 0)
		{
			//RTUSBQueueTailRxContext(pAd, &pAd->RxFreeQ, pRxContext);
			MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("RTUSBBulkReceive: Submit Rx URB failed %d\n", ret));
			RTUSBUnlinkRxContext(pAd, pRxContext, &pAd->RxBulkInQ);
		}
		else
		{

		}

#ifdef CONFIG_TRACE_SUPPORT
		TRACE_USB_BULK_IN_INFO(pAd, __FUNCTION__, pRxContext, ret);
#endif
	}
}


#define fRTMP_ADAPTER_NEED_STOP_RX		\
		(fRTMP_ADAPTER_NIC_NOT_EXIST | fRTMP_ADAPTER_HALT_IN_PROGRESS |	\
		 fRTMP_ADAPTER_RADIO_OFF | fRTMP_ADAPTER_RESET_IN_PROGRESS | \
		 fRTMP_ADAPTER_REMOVE_IN_PROGRESS | fRTMP_ADAPTER_BULKIN_RESET)

#define fRTMP_ADAPTER_NEED_STOP_HANDLE_RX	\
		(fRTMP_ADAPTER_NIC_NOT_EXIST | fRTMP_ADAPTER_HALT_IN_PROGRESS |	\
		 fRTMP_ADAPTER_RADIO_OFF | fRTMP_ADAPTER_RESET_IN_PROGRESS | \
		 fRTMP_ADAPTER_REMOVE_IN_PROGRESS)


VOID RTUSBBulkReceive(RTMP_ADAPTER *pAd)
{
	PRX_CONTEXT pRxContext;
	ULONG IrqFlags = 0;

	if (RTMP_TEST_FLAG(pAd, fRTMP_ADAPTER_NEED_STOP_HANDLE_RX)
					&& !RTMP_TEST_FLAG(pAd, fRTMP_ADAPTER_POLL_IDLE))
		return;

#ifdef CONFIG_TRACE_SUPPORT
	TRACE_USB_BULK_QLEN(pAd, __FUNCTION__, RX_FREEQ, &pAd->RxFreeQ);
	TRACE_USB_BULK_QLEN(pAd, __FUNCTION__, RX_BULKINQ, &pAd->RxBulkInQ);
	TRACE_USB_BULK_QLEN(pAd, __FUNCTION__, RX_PROCESSINGQ, &pAd->RxProcessingQ);
#endif

	if (!((RTMP_TEST_FLAG(pAd, fRTMP_ADAPTER_NEED_STOP_RX)
						&& (!RTMP_TEST_FLAG(pAd, fRTMP_ADAPTER_POLL_IDLE)))))
	{
#ifdef CONFIG_STA_SUPPORT
		if(RTMP_TEST_FLAG(pAd, fRTMP_ADAPTER_IDLE_RADIO_OFF))
			return;
#endif /* CONFIG_STA_SUPPORT */

		DoBulkIn(pAd);
	}

	while (pRxContext = RTUSBDequeueRxContext(pAd, &pAd->RxProcessingQ))
	{

#ifdef CONFIG_TRACE_SUPPORT
		TRACE_USB_BULK_IN_RECEIVE(pAd, __FUNCTION__, 0, pRxContext);
#endif

		RTMP_IRQ_LOCK(&pAd->BulkInLock, IrqFlags);
		pAd->RxBulkInReadIndex = pRxContext->Id;
		RTMP_IRQ_UNLOCK(&pAd->BulkInLock, IrqFlags);

		rtmp_rx_done_handle(pAd);

		pRxContext->BulkInOffset = 0;
		pRxContext->ReadPosition = 0;

#ifdef CONFIG_TRACE_SUPPORT
		TRACE_USB_BULK_IN_RECEIVE(pAd, __FUNCTION__, 1, pRxContext);
#endif

		RTUSBQueueTailRxContext(pAd, &pAd->RxFreeQ, pRxContext);
	}

	if (!((RTMP_TEST_FLAG(pAd, fRTMP_ADAPTER_NEED_STOP_RX)
						&& (!RTMP_TEST_FLAG(pAd, fRTMP_ADAPTER_POLL_IDLE)))))
	{
#ifdef CONFIG_STA_SUPPORT
		if(RTMP_TEST_FLAG(pAd, fRTMP_ADAPTER_IDLE_RADIO_OFF))
			return;
#endif /* CONFIG_STA_SUPPORT */

		DoBulkIn(pAd);
	}
}


USBHST_STATUS RTUSBBulkRxComplete(URBCompleteStatus Status, purbb_t pURB, pregs *pt_regs)
{
	PRX_CONTEXT pRxContext;
	PRTMP_ADAPTER pAd;
	POS_COOKIE pObj;
	NTSTATUS Status;
	Status = RTMP_USB_URB_STATUS_GET(pURB);

	pRxContext = (PRX_CONTEXT)RTMP_OS_USB_CONTEXT_GET(pURB);
	pAd = pRxContext->pAd;
	pObj = (POS_COOKIE)pAd->OS_Cookie;

	RTUSBUnlinkRxContext(pAd, pRxContext, &pAd->RxBulkInQ);

	pRxContext->BulkInOffset = RTMP_USB_URB_LEN_GET(pURB);

#ifdef CONFIG_TRACE_SUPPORT
	TRACE_USB_BULK_IN_RX_COMPLETE(__FUNCTION__, RTMP_OS_USB_STATUS_GET(pURB),
							RTMP_OS_USB_TRANSFER_LEN_GET(pURB),
							RTMP_OS_USB_ACTUAL_LEN_GET(pURB));
#endif

	if ((Status == 0) && (pRxContext->BulkInOffset != 0))
	{
		RTUSBQueueTailRxContext(pAd, &pAd->RxProcessingQ, pRxContext);
	}
	else
	{
		RTUSBQueueTailRxContext(pAd, &pAd->RxFreeQ, pRxContext);
	}

#ifdef CONFIG_TRACE_SUPPORT
	TRACE_USB_BULK_QLEN(pAd, __FUNCTION__, RX_FREEQ, &pAd->RxFreeQ);
	TRACE_USB_BULK_QLEN(pAd, __FUNCTION__, RX_BULKINQ, &pAd->RxBulkInQ);
	TRACE_USB_BULK_QLEN(pAd, __FUNCTION__, RX_PROCESSINGQ, &pAd->RxProcessingQ);
#endif

	if (!((RTMP_TEST_FLAG(pAd, fRTMP_ADAPTER_NEED_STOP_RX)
						&& (!RTMP_TEST_FLAG(pAd, fRTMP_ADAPTER_POLL_IDLE)))))
	{
#ifdef CONFIG_STA_SUPPORT
		if(RTMP_TEST_FLAG(pAd, fRTMP_ADAPTER_IDLE_RADIO_OFF))
			return;
#endif /* CONFIG_STA_SUPPORT */

		DoBulkIn(pAd);
	}

	RTMP_NET_TASK_DATA_ASSIGN(&pObj->rx_done_task, (unsigned long)pURB);
	RTMP_OS_TASKLET_SCHE(&pObj->rx_done_task);
}


/*
	========================================================================

	Routine Description:

	Arguments:

	Return Value:

	Note:

	========================================================================
*/
VOID	RTUSBKickBulkOut(
	IN	PRTMP_ADAPTER pAd)
{
#ifdef RELEASE_EXCLUDE
	DBGPRINT_RAW(DBG_CAT_ALL, DBG_LVL_INFO, ("--->RTUSBKickBulkOut\n"));
#endif /* RELEASE_EXCLUDE */
	/* BulkIn Reset will reset whole USB PHY. So we need to make sure fRTMP_ADAPTER_BULKIN_RESET not flaged.*/
	if (!RTMP_TEST_FLAG(pAd ,fRTMP_ADAPTER_NEED_STOP_TX)
#ifdef CONFIG_ATE
		&& !(ATE_ON(pAd))
#endif /* CONFIG_ATE */
		)
	{
#if 0	/* not used now in RT28xx, but may used latter.*/
		/* 1. Data Fragment has highest priority*/
		if (RTUSB_TEST_BULK_FLAG(pAd, fRTUSB_BULK_OUT_DATA_FRAG))
		{
			if (((!RTMP_TEST_FLAG(pAd, fRTMP_ADAPTER_BSS_SCAN_IN_PROGRESS)) ||
				(!OPSTATUS_TEST_FLAG(pAd, fOP_STATUS_MEDIA_STATE_CONNECTED))
#ifdef MESH_SUPPORT
				|| MESH_ON(pAd)
#endif /* MESH_SUPPORT */
				))
			{
				RTUSBBulkOutDataPacket(pAd, 0, pAd->NextBulkOutIndex[0]);
			}
		}
		if (RTUSB_TEST_BULK_FLAG(pAd, fRTUSB_BULK_OUT_DATA_FRAG_2))
		{
			if (((!RTMP_TEST_FLAG(pAd, fRTMP_ADAPTER_BSS_SCAN_IN_PROGRESS)) ||
				(!OPSTATUS_TEST_FLAG(pAd, fOP_STATUS_MEDIA_STATE_CONNECTED))
#ifdef MESH_SUPPORT
				|| MESH_ON(pAd)
#endif /* MESH_SUPPORT */
				))
			{
				RTUSBBulkOutDataPacket(pAd, 1, pAd->NextBulkOutIndex[1]);
			}
		}
		if (RTUSB_TEST_BULK_FLAG(pAd, fRTUSB_BULK_OUT_DATA_FRAG_3))
		{
			if (((!RTMP_TEST_FLAG(pAd, fRTMP_ADAPTER_BSS_SCAN_IN_PROGRESS)) ||
				(!OPSTATUS_TEST_FLAG(pAd, fOP_STATUS_MEDIA_STATE_CONNECTED))
#ifdef MESH_SUPPORT
				|| MESH_ON(pAd)
#endif /* MESH_SUPPORT */
				))
			{
				RTUSBBulkOutDataPacket(pAd, 2, pAd->NextBulkOutIndex[2]);
			}
		}
		if (RTUSB_TEST_BULK_FLAG(pAd, fRTUSB_BULK_OUT_DATA_FRAG_4))
		{
			if (((!RTMP_TEST_FLAG(pAd, fRTMP_ADAPTER_BSS_SCAN_IN_PROGRESS)) ||
				(!OPSTATUS_TEST_FLAG(pAd, fOP_STATUS_MEDIA_STATE_CONNECTED))
#ifdef MESH_SUPPORT
				|| MESH_ON(pAd)
#endif /* MESH_SUPPORT */
				))
			{
				RTUSBBulkOutDataPacket(pAd, 3, pAd->NextBulkOutIndex[3]);
			}
		}
#endif /* 0 */

		/* 2. PS-Poll frame is next*/
		if (RTUSB_TEST_BULK_FLAG(pAd, fRTUSB_BULK_OUT_PSPOLL))
		{
			RTUSBBulkOutPsPoll(pAd);
		}

		/* 5. Mlme frame is next*/
		else if ((RTUSB_TEST_BULK_FLAG(pAd, fRTUSB_BULK_OUT_MLME)) ||
				 (pAd->MgmtRing.TxSwFreeIdx < MGMT_RING_SIZE))
		{
			RTUSBBulkOutMLMEPacket(pAd, pAd->MgmtRing.TxDmaIdx);
		}

#ifdef MT_MAC
		/* 6. BCN frame is next*/
		else if ((RTUSB_TEST_BULK_FLAG(pAd, fRTUSB_BULK_OUT_BCN)) ||
				 (pAd->BcnRing.TxSwFreeIdx < BCN_RING_SIZE))
		{
			RTUSBBulkOutBCNPacket(pAd, pAd->BcnRing.TxDmaIdx);
		}
#endif

		/* 7. Data frame normal is next*/
		if (RTUSB_TEST_BULK_FLAG(pAd, fRTUSB_BULK_OUT_DATA_NORMAL))
		{
			if (((!RTMP_TEST_FLAG(pAd, fRTMP_ADAPTER_BSS_SCAN_IN_PROGRESS)) ||
				(!OPSTATUS_TEST_FLAG(pAd, fOP_STATUS_MEDIA_STATE_CONNECTED))
#ifdef MESH_SUPPORT
				|| MESH_ON(pAd)
#endif /* MESH_SUPPORT */
#ifdef P2P_SUPPORT
				|| P2P_GO_ON(pAd) || P2P_CLI_ON(pAd)
#endif /* P2P_SUPPORT */
#ifdef RT_CFG80211_P2P_CONCURRENT_DEVICE
				 || RTMP_CFG80211_VIF_P2P_GO_ON(pAd) || RTMP_CFG80211_VIF_P2P_CLI_ON(pAd)
#endif /* RT_CFG80211_P2P_CONCURRENT_DEVICE */
				))
			{
				RTUSBBulkOutDataPacket(pAd, EDCA_AC0_PIPE, pAd->NextBulkOutIndex[EDCA_AC0_PIPE]);
			}
		}
		if (RTUSB_TEST_BULK_FLAG(pAd, fRTUSB_BULK_OUT_DATA_NORMAL_2))
		{
			if (((!RTMP_TEST_FLAG(pAd, fRTMP_ADAPTER_BSS_SCAN_IN_PROGRESS)) ||
				(!OPSTATUS_TEST_FLAG(pAd, fOP_STATUS_MEDIA_STATE_CONNECTED))
#ifdef MESH_SUPPORT
				|| MESH_ON(pAd)
#endif /* MESH_SUPPORT */
#ifdef P2P_SUPPORT
				|| P2P_GO_ON(pAd) || P2P_CLI_ON(pAd)
#endif /* P2P_SUPPORT */
#ifdef RT_CFG80211_P2P_CONCURRENT_DEVICE
				|| RTMP_CFG80211_VIF_P2P_GO_ON(pAd) || RTMP_CFG80211_VIF_P2P_CLI_ON(pAd)
#endif /* RT_CFG80211_P2P_CONCURRENT_DEVICE */

				))
			{
				RTUSBBulkOutDataPacket(pAd, EDCA_AC1_PIPE, pAd->NextBulkOutIndex[EDCA_AC1_PIPE]);
			}
		}
		if (RTUSB_TEST_BULK_FLAG(pAd, fRTUSB_BULK_OUT_DATA_NORMAL_3))
		{
			if (((!RTMP_TEST_FLAG(pAd, fRTMP_ADAPTER_BSS_SCAN_IN_PROGRESS)) ||
				(!OPSTATUS_TEST_FLAG(pAd, fOP_STATUS_MEDIA_STATE_CONNECTED))
#ifdef MESH_SUPPORT
				|| MESH_ON(pAd)
#endif /* MESH_SUPPORT */
#ifdef P2P_SUPPORT
				|| P2P_GO_ON(pAd) || P2P_CLI_ON(pAd)
#endif /* P2P_SUPPORT */
#ifdef RT_CFG80211_P2P_CONCURRENT_DEVICE
				|| RTMP_CFG80211_VIF_P2P_GO_ON(pAd) || RTMP_CFG80211_VIF_P2P_CLI_ON(pAd)
#endif /* RT_CFG80211_P2P_CONCURRENT_DEVICE */
				))
			{
				RTUSBBulkOutDataPacket(pAd, EDCA_AC2_PIPE, pAd->NextBulkOutIndex[EDCA_AC2_PIPE]);
			}
		}
		if (RTUSB_TEST_BULK_FLAG(pAd, fRTUSB_BULK_OUT_DATA_NORMAL_4))
		{
			if (((!RTMP_TEST_FLAG(pAd, fRTMP_ADAPTER_BSS_SCAN_IN_PROGRESS)) ||
				(!OPSTATUS_TEST_FLAG(pAd, fOP_STATUS_MEDIA_STATE_CONNECTED))
#ifdef MESH_SUPPORT
				|| MESH_ON(pAd)
#endif /* MESH_SUPPORT */
				))
			{
				RTUSBBulkOutDataPacket(pAd, EDCA_AC3_PIPE, pAd->NextBulkOutIndex[EDCA_AC3_PIPE]);
			}
		}
#ifdef MULTI_WMM_SUPPORT
		if (RTUSB_TEST_BULK_FLAG(pAd, fRTUSB_BULK_OUT_DATA_WMM1_NORMAL))
		{
			if (((!RTMP_TEST_FLAG(pAd, fRTMP_ADAPTER_BSS_SCAN_IN_PROGRESS)) ||
				(!OPSTATUS_TEST_FLAG(pAd, fOP_STATUS_MEDIA_STATE_CONNECTED))
#ifdef MESH_SUPPORT
				|| MESH_ON(pAd)
#endif /* MESH_SUPPORT */
				))
			{
				RTUSBBulkOutWmm1DataPacket(pAd, EDCA_AC0_PIPE, pAd->NextBulkOutWmm1Index[EDCA_AC0_PIPE]);
			}
		}
#endif /* MULTI_WMM_SUPPORT */
		/* 8. Null frame is the last*/
		else if (RTUSB_TEST_BULK_FLAG(pAd, fRTUSB_BULK_OUT_DATA_NULL))
		{

		#if 0 //need to remove this to let null frame can be sent
			if (!RTMP_TEST_FLAG(pAd, fRTMP_ADAPTER_BSS_SCAN_IN_PROGRESS))
		#endif
			{
				RTUSBBulkOutNullFrame(pAd);
			}
		}

		/* 8. No data avaliable*/
		else
		{

		}
	}
#ifdef CONFIG_ATE
	else if((ATE_ON(pAd)) &&
			!RTMP_TEST_FLAG(pAd , fRTMP_ADAPTER_NEED_STOP_TX))
	{
		if (RTUSB_TEST_BULK_FLAG(pAd, fRTUSB_BULK_OUT_DATA_ATE))
		{
			ATE_RTUSBBulkOutDataPacket(pAd, EDCA_AC0_PIPE);
		}
	}
#endif /* CONFIG_ATE */

#ifdef RELEASE_EXCLUDE
	DBGPRINT_RAW(DBG_CAT_ALL, DBG_LVL_INFO, ("<---RTUSBKickBulkOut\n"));
#endif /* RELEASE_EXCLUDE */
}

/*
	========================================================================

	Routine Description:
	Call from Reset action after BulkOut failed.
	Arguments:

	Return Value:

	Note:

	========================================================================
*/
VOID	RTUSBCleanUpDataBulkOutQueue(
	IN	PRTMP_ADAPTER	pAd)
{
	UCHAR			Idx;
	PHT_TX_CONTEXT	pTxContext;

	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("--->CleanUpDataBulkOutQueue\n"));

	for (Idx = 0; Idx < 4; Idx++)
	{
		pTxContext = &pAd->TxContext[Idx];

		pTxContext->CurWritePosition = pTxContext->NextBulkOutPosition;
		pTxContext->LastOne = FALSE;
		NdisAcquireSpinLock(&pAd->BulkOutLock[Idx]);
		pAd->BulkOutPending[Idx] = FALSE;
		NdisReleaseSpinLock(&pAd->BulkOutLock[Idx]);
	}

	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("<---CleanUpDataBulkOutQueue\n"));
}

/*
	========================================================================

	Routine Description:

	Arguments:

	Return Value:

	Note:

	========================================================================
*/
VOID	RTUSBCleanUpMLMEBulkOutQueue(
	IN	PRTMP_ADAPTER	pAd)
{
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("--->CleanUpMLMEBulkOutQueue\n"));

#if 0	/* Do nothing!*/
	NdisAcquireSpinLock(&pAd->MLMEBulkOutLock);
	while (pAd->PrioRingTxCnt > 0)
	{
		pAd->MLMEContext[pAd->PrioRingFirstIndex].InUse = FALSE;

		pAd->PrioRingFirstIndex++;
		if (pAd->PrioRingFirstIndex >= MGMT_RING_SIZE)
		{
			pAd->PrioRingFirstIndex = 0;
		}

		pAd->PrioRingTxCnt--;
	}
	NdisReleaseSpinLock(&pAd->MLMEBulkOutLock);
#endif

	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("<---CleanUpMLMEBulkOutQueue\n"));
}


/*
	========================================================================

	Routine Description:

	Arguments:

	Return Value:


	Note:

	========================================================================
*/
VOID	RTUSBCancelPendingIRPs(
	IN	PRTMP_ADAPTER	pAd)
{
	RTUSBCancelPendingBulkInIRP(pAd);
	RTUSBCancelPendingBulkOutIRP(pAd);
}

/*
	========================================================================

	Routine Description:

	Arguments:

	Return Value:

	Note:

	========================================================================
*/
VOID RTUSBCancelPendingBulkInIRP(RTMP_ADAPTER *pAd)
{
	UINT32 i;

	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("--->RTUSBCancelPendingBulkInIRP\n"));

	for (i=0; i < RX_RING_SIZE; i++)
	{
		PRX_CONTEXT  pRxContext = &(pAd->RxContext[i]);
			RTUSB_UNLINK_URB(pRxContext->pUrb);
	}
}


/*
	========================================================================

	Routine Description:

	Arguments:

	Return Value:

	Note:

	========================================================================
*/
VOID	RTUSBCancelPendingBulkOutIRP(
	IN	PRTMP_ADAPTER	pAd)
{
	PHT_TX_CONTEXT		pHTTXContext;
	PTX_CONTEXT			pMLMEContext;
	PTX_CONTEXT			pNullContext;
	PTX_CONTEXT			pPsPollContext;
	UINT				i, Idx;
/*	ULONG 		IrqFlags = 0;*/
/*	NDIS_SPIN_LOCK		*pLock;*/
/*	BOOLEAN				*pPending;*/


/*	pLock = &pAd->BulkOutLock[MGMTPIPEIDX];*/
/*	pPending = &pAd->BulkOutPending[MGMTPIPEIDX];*/

	for (Idx = 0; Idx < 4; Idx++)
	{
		pHTTXContext = &(pAd->TxContext[Idx]);

		if (pHTTXContext->IRPPending == TRUE)
		{

			/* Get the USB_CONTEXT and cancel it's IRP; the completion routine will itself*/
			/* remove it from the HeadPendingSendList and NULL out HeadPendingSendList*/
			/*	when the last IRP on the list has been	cancelled; that's how we exit this loop*/


			RTUSB_UNLINK_URB(pHTTXContext->pUrb);

			/* Sleep 200 microseconds to give cancellation time to work*/
			RtmpusecDelay(200);
		}

#ifdef CONFIG_ATE
		pHTTXContext->bCopySavePad = 0;
		pHTTXContext->CurWritePosition = 0;
		pHTTXContext->CurWriteRealPos = 0;
		pHTTXContext->bCurWriting = FALSE;
		pHTTXContext->NextBulkOutPosition = 0;
		pHTTXContext->ENextBulkOutPosition = 0;
#endif /* CONFIG_ATE */
		pAd->BulkOutPending[Idx] = FALSE;
	}

	/*RTMP_IRQ_LOCK(pLock, IrqFlags);*/
	for (i = 0; i < MGMT_RING_SIZE; i++)
	{
		pMLMEContext = (PTX_CONTEXT)pAd->MgmtRing.Cell[i].AllocVa;
		if(pMLMEContext && (pMLMEContext->IRPPending == TRUE))
		{

			/* Get the USB_CONTEXT and cancel it's IRP; the completion routine will itself*/
			/* remove it from the HeadPendingSendList and NULL out HeadPendingSendList*/
			/*	when the last IRP on the list has been	cancelled; that's how we exit this loop*/


			RTUSB_UNLINK_URB(pMLMEContext->pUrb);
			pMLMEContext->IRPPending = FALSE;

			/* Sleep 200 microsecs to give cancellation time to work*/
			RtmpusecDelay(200);
		}
	}
	pAd->BulkOutPending[MGMTPIPEIDX] = FALSE;
	/*RTMP_IRQ_UNLOCK(pLock, IrqFlags);*/

	pNullContext = &(pAd->NullContext);
	if (pNullContext->IRPPending == TRUE)
		RTUSB_UNLINK_URB(pNullContext->pUrb);

	pPsPollContext = &(pAd->PsPollContext);
	if (pPsPollContext->IRPPending == TRUE)
		RTUSB_UNLINK_URB(pPsPollContext->pUrb);

	for (Idx = 0; Idx < 4; Idx++)
	{
		NdisAcquireSpinLock(&pAd->BulkOutLock[Idx]);
		pAd->BulkOutPending[Idx] = FALSE;
		NdisReleaseSpinLock(&pAd->BulkOutLock[Idx]);
	}
}

#endif /* RTMP_MAC_USB */
