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
	cmm_data_sdio.c
*/


#include "rt_config.h"
#include "rtmp.h"

#ifdef CONFIG_STA_SUPPORT
VOID ComposePsPoll(RTMP_ADAPTER *pAd)
{

}
/* IRQL = DISPATCH_LEVEL */
VOID ComposeNullFrame(RTMP_ADAPTER *pAd)
{

}

#endif


USHORT MtSDIO_WriteSubTxResource(struct _RTMP_ADAPTER *pAd, struct _TX_BLK *pTxBlk, BOOLEAN bIsLast, USHORT *freeCnt)
{

	return 0;

}

USHORT MtSDIO_WriteFragTxResource(
	RTMP_ADAPTER *pAd,
	struct _TX_BLK *pTxBlk,
	UCHAR fragNum,
	USHORT *freeCnt)
{
	return(0);
}


USHORT
MtSDIO_WriteSingleTxResource(
        IN PRTMP_ADAPTER prAd,
        IN TX_BLK *prTxBlk,
        IN BOOLEAN fgIsLast,
        OUT USHORT *u2freeCnt)
{
    TXINFO_STRUC *prTxInfo = NULL;
    UCHAR *pucOutputBuf = NULL;
    UCHAR *prBuf = NULL;
    UCHAR ucQueIdx = 0;
    //NDIS_STATUS rStatus = NDIS_STATUS_SUCCESS;
    UINT32 hdr_copy_len, hdr_len, dma_len = 0;
    //UINT8 ucTXWISize = prAd->chipCap.TXWISize;
    P_TX_CTRL_T prTxCtrl = &prAd->rTxCtrl;
    UINT16 u2PgCnt = 0;
    NDIS_STATUS Status = 0;

    pucOutputBuf = prTxCtrl->pucTxCoalescingBufPtr;

    /* get Tx Ring Resource & Dma Buffer address*/
    ucQueIdx = prTxBlk->QueIdx;
    {
        do
        {
            prBuf = &prTxBlk->HeaderBuf[0];
            prTxInfo = (TXINFO_STRUC *) prBuf;

            /* Build our URB for USBD */
#ifdef MT_MAC
            hdr_len = prTxBlk->MpduHeaderLen + prTxBlk->HdrPadLen;
            if (prTxBlk->hw_rsv_len == 0) // Long
            {
                hdr_copy_len = sizeof(TMAC_TXD_L) + hdr_len;
            }
            else //Short
            {
                hdr_copy_len = sizeof(TMAC_TXD_S) + hdr_len;
                prTxBlk->HeaderBuf += prTxBlk->hw_rsv_len;
            }

            dma_len = hdr_copy_len + prTxBlk->SrcBufLen;
#endif /* MT_MAC */
            //padding = ((4-(dma_len%4))&0x3); /* round up to 4 byte alignment*/
            //dma_len += padding;
            dma_len = ((dma_len + 0x3)&~0x3);/* round up to 4 byte alignment*/

#ifdef MT_MAC
            prTxBlk->Priv = (dma_len);
#endif /* MT_MAC */
#if CFG_SDIO_TX_AGG
            /*
                        To check MTSDIOTxAcquireResource(IN PRTMP_ADAPTER prAd,IN UINT8	ucTC,IN UINT8 ucPageCount)
                        */
            /*
               prTxCtrl->pucAggIdx lock/unlock is neccessary or not?
               */
            MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("(%s) buf(%p) offset:%p len = %d\n", __FUNCTION__, prTxBlk->HeaderBuf, prTxCtrl->pucAggOffset, hdr_copy_len));

            NdisMoveMemory(	prTxCtrl->pucAggOffset, prTxBlk->HeaderBuf, hdr_copy_len);

            prTxCtrl->pucAggOffset += (hdr_copy_len);
            {
                //NdisMoveMemory(	prTxCtrl->pucAggIdx, prTxBlk->pSrcBufData, prTxBlk->SrcBufLen);
                //NdisMoveMemory(	prTxCtrl->pucAggOffset, prTxBlk->pSrcBufData, ((prTxBlk->SrcBufLen+ 0x3)&~0x3));
                NdisMoveMemory(	prTxCtrl->pucAggOffset, prTxBlk->pSrcBufData, ((prTxBlk->SrcBufLen)));
                prTxCtrl->pucAggOffset += ((prTxBlk->SrcBufLen + 0x3)&~0x3);

                MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("(%s) buf(%p) offset:%p len = %d\n", __FUNCTION__, prTxBlk->pSrcBufData, prTxCtrl->pucAggOffset, ((prTxBlk->SrcBufLen + 0x3) &~0x3)));
            }

            prTxCtrl->u4AggLen +=dma_len;

            //MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("(%s) each agg len %d\n", __FUNCTION__, dma_len));
            u2PgCnt = MTSDIOTxGetPageCount((hdr_copy_len + prTxBlk->SrcBufLen), TRUE);

            MTSDIOTxAcquireResource(prAd, prTxBlk->QueIdx, u2PgCnt);
#else
            NdisMoveMemory(pucOutputBuf, prTxBlk->HeaderBuf, hdr_copy_len);
            pucOutputBuf += (hdr_copy_len);
            {
                NdisMoveMemory(pucOutputBuf, prTxBlk->pSrcBufData, prTxBlk->SrcBufLen);
                pucOutputBuf += prTxBlk->SrcBufLen;
            }
#endif

        } while (FALSE);
    }
    /* succeed and release the skb buffer*/
    RELEASE_NDIS_PACKET(prAd, prTxBlk->pPacket, NDIS_STATUS_SUCCESS);
    return Status;
}

USHORT MtSDIO_WriteMultiTxResource(
	RTMP_ADAPTER *pAd,
	struct _TX_BLK *pTxBlk,
	UCHAR frmNum,
	USHORT *freeCnt)
{
	NDIS_STATUS Status = 0;

	return(Status);
}


VOID MtSDIO_FinalWriteTxResource(
	RTMP_ADAPTER	*pAd,
	struct _TX_BLK *pTxBlk,
	USHORT totalMPDUSize,
	USHORT TxIdx)
{

}


VOID MtSDIODataLastTxIdx(
	RTMP_ADAPTER *pAd,
	UCHAR QueIdx,
	USHORT TxIdx)
{
	/* DO nothing for USB.*/
}

UINT32	MtSDIOTxResourceRequest(
	IN RTMP_ADAPTER *pAd,
	IN UCHAR QueIdx,
    IN UINT16 u2FreePgCnt,
	IN UINT32 req_cnt)
{
#if CFG_SDIO_TX_AGG
	UINT32	result = NDIS_STATUS_FAILURE;

	//MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("(%s) each real len %d\n", __FUNCTION__, req_cnt));

    /* margin: 1 page */
    /* TODO: mtk03034 calculate exactly size */

    if (u2FreePgCnt >= (MTSDIOTxGetPageCount(req_cnt, FALSE) + 1)) {
            result = NDIS_STATUS_SUCCESS;
    }

	return result;

#else
	return NDIS_STATUS_SUCCESS;
#endif
}

VOID
MtSDIODataKickOut(
        IN PRTMP_ADAPTER prAd,
        IN TX_BLK *prTxBlk,
        IN UCHAR QueIdx)
{
#if !CFG_SDIO_TX_AGG

    P_TX_CTRL_T prTxCtrl = &prAd->rTxCtrl;
    UCHAR *pucOutputBuf = NULL;

    MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("(%s) \n", __FUNCTION__));

    pucOutputBuf = prTxCtrl->pucTxCoalescingBufPtr;
    MTSDIOMultiWrite(prAd, WTDR1, pucOutputBuf, prTxBlk->Priv);

    hex_dump("xxx", pucOutputBuf, prTxBlk->Priv);

#endif

}

VOID
MtSdioSubmitDataToSDIOWorker(
        IN PRTMP_ADAPTER prAd
)
{
	MTSDIOWorkerThreadEventNotify(prAd, SDIO_EVENT_TX_DATA_REQ_BIT);
	return;
}


VOID
MtSdioSubmitMgmtToSDIOWorker(
        IN PRTMP_ADAPTER prAd,
        IN UCHAR ucQueIdx,
        IN PNDIS_PACKET prPacket,
        IN UCHAR *prSrcBufVA,
        IN UINT u4SrcBufLen
        )
{
    /* caller will help to hold MgmtRing lock */
    //`P_MSDU_INTO_T prMsduInfo = NULL;

    /* Decrease the TxSwFreeIdx and Increase the TX_CTX_IDX*/
#ifdef MT_MAC
	if (ucQueIdx == Q_IDX_BCN) {
		prAd->BcnRing.Cell[prAd->BcnRing.TxCpuIdx].pNdisPacket
				= prPacket;
		prAd->BcnRing.Cell[prAd->BcnRing.TxCpuIdx].u4TxLength
				= u4SrcBufLen;
		prAd->BcnRing.TxSwFreeIdx--;
		INC_RING_INDEX(prAd->BcnRing.TxCpuIdx, BCN_RING_SIZE);
		MTSDIOWorkerThreadEventNotify(prAd, SDIO_EVENT_TX_BCN_BIT);
	}
	else
#endif
	{
		prAd->MgmtRing.Cell[prAd->MgmtRing.TxCpuIdx].pNdisPacket = prPacket;
		prAd->MgmtRing.Cell[prAd->MgmtRing.TxCpuIdx].u4TxLength = u4SrcBufLen;
		prAd->MgmtRing.TxSwFreeIdx--;
		INC_RING_INDEX(prAd->MgmtRing.TxCpuIdx, MGMT_RING_SIZE);
		MTSDIOWorkerThreadEventNotify(prAd, SDIO_EVENT_TXREQ_BIT);
	}

	return;
}

INT32
MtSDIOMgmtKickOut(
        IN PRTMP_ADAPTER prAd,
        IN UCHAR *prSrcBufVA,
        IN UINT u4SrcBufLen)
{
    MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("(%s) enter\n", __FUNCTION__));

    MTSDIOMultiWrite(prAd, WTDR1, prSrcBufVA, u4SrcBufLen);

    return 0;
}
VOID MtSDIONullFrameKickOut(struct _RTMP_ADAPTER *pAd, UCHAR QIdx, UCHAR *pNullFrm, UINT32 frmLen)
{

	//MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("(%s) enter\n", __FUNCTION__));
	//RTMP_IRQ_LOCK(&pAd->MgmtRingLock, Flags); ?? lock is need or not

	//if (pAd->pNULLPacketInUse == FALSE)
	{
//		PTX_CONTEXT pNullContext;
		PNDIS_PACKET pPacket;
		TXINFO_STRUC *pTxInfo;
		TXWI_STRUC *pTxWI;
		UCHAR *pWirelessPkt;
//		UINT8 TXWISize = pAd->chipCap.TXWISize;
		UINT8 tx_hw_hdr_len = pAd->chipCap.tx_hw_hdr_len;
		MAC_TX_INFO mac_info;
//		ULONG FreeNum;
		PACKET_INFO PacketInfo;
		UCHAR *pSrcBufVA;
		UCHAR *pBuf;
		UINT SrcBufLen;
		NDIS_STATUS Status = NDIS_STATUS_SUCCESS;
		UINT32 uLen;
		UINT32 hw_len;
		UCHAR rtmpHwHdr[40];
		ULONG Flags = 0;
		UCHAR *tmac_info;
		PHEADER_802_11 pHeader_802_11;

		RTMP_IRQ_LOCK(&pAd->MgmtRingLock, Flags);

		pHeader_802_11 = (HEADER_802_11 *)(pNullFrm);
	

		hw_len = pAd->chipCap.tx_hw_hdr_len;
		ASSERT((sizeof(rtmpHwHdr) > hw_len));
		/* We need to reserve space for rtmp hardware header. i.e., TxWI for RT2860 and TxInfo+TxWI for RT2870*/
		NdisZeroMemory(&rtmpHwHdr, hw_len);
//		Status = RTMPAllocateNdisPacket(pAd, &pPacket, (UCHAR *)&rtmpHwHdr[0], hw_len, pData, Length); //where to release it?
		Status = RTMPAllocateNdisPacket(pAd, &pPacket, (UCHAR *)&rtmpHwHdr[0], hw_len, pNullFrm, frmLen); //where to release it?
		
		if (Status != NDIS_STATUS_SUCCESS)
		{
			MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("MiniportMMRequest (error:: can't allocate NDIS PACKET)\n"));
			RTMP_IRQ_UNLOCK(&pAd->MgmtRingLock, Flags);
			return;
		}

		RTMP_QueryPacketInfo(pPacket, &PacketInfo, &pSrcBufVA, &SrcBufLen);

//		pNullContext = &(pAd->NullContext);

		/* Set the in use bit*/


//		pWirelessPkt = (PUCHAR)&pNullContext->TransferBuffer->field.WirelessPacket[0];
		pWirelessPkt = pSrcBufVA;

		RTMPZeroMemory(&pWirelessPkt[0], SrcBufLen);
		pTxInfo = (TXINFO_STRUC *)&pWirelessPkt[0];
//		rlt_usb_write_txinfo(pAd, pTxInfo, (USHORT)(frameLen + TXWISize + TSO_SIZE), TRUE, EpToQueue[MGMTPIPEIDX], FALSE,  FALSE);

		pTxWI = (TXWI_STRUC *)&pWirelessPkt[TXINFO_SIZE]; //?
		NdisZeroMemory((UCHAR *)&mac_info, sizeof(mac_info));

#ifdef MT_MAC
		tmac_info = &pWirelessPkt[0];
		if (pHeader_802_11->FC.Type == FC_TYPE_DATA) {
			switch (pHeader_802_11->FC.SubType) {
			case SUBTYPE_DATA_NULL:
				mac_info.hdr_len = 24;
				break;
			case SUBTYPE_QOS_NULL:
				mac_info.hdr_len = 26;
				break;
			default:
				break;
			}
		}
#endif

		mac_info.FRAG = FALSE;
		
		mac_info.CFACK = FALSE;
		mac_info.InsTimestamp = FALSE;
		mac_info.AMPDU = FALSE;

		mac_info.BM = 0;
		mac_info.Ack = TRUE;
		mac_info.NSeq = FALSE;
		mac_info.BASize = 0;
		
		mac_info.WCID = BSSID_WCID;
//		mac_info.Length = frameLen;
		mac_info.Length = frmLen;
#ifdef MT_MAC
		/* 7636 psm*/
		if (pHeader_802_11->FC.PwrMgmt == PWR_ACTIVE)
		{
			mac_info.PID = PID_NULL_FRAME_PWR_ACTIVE;
		}
		else
		{
			mac_info.PID = PID_NULL_FRAME_PWR_SAVE;
		}

		//mac_info.TxSFormat = 0;

		if (RTMP_TEST_FLAG(pAd, fRTMP_ADAPTER_BSS_SCAN_IN_PROGRESS))
		{
			/* In scan initial phase, driver needs not handle PID_NULL_FRAME_PWR_SAVE 
			since driver will wakeup H/W later at ScanNextChannel() to do scan */
			//mac_info.TxS2Host = 0;
		}
		else
		{
			//mac_info.TxS2Host = 1;
		}
#endif /* MT_MAC */		
		mac_info.TID = 0;
		mac_info.TxRate = (UCHAR)pAd->CommonCfg.MlmeTransmit.field.MCS;
		mac_info.Txopmode = IFS_HTTXOP;
		mac_info.Preamble = LONG_PREAMBLE;	
#ifdef MT_MAC
		mac_info.PsmBySw = 1;
		mac_info.Type = FC_TYPE_DATA; 
		mac_info.SubType = SUBTYPE_DATA_NULL; 
		mac_info.q_idx = Q_IDX_AC4;
		write_tmac_info(pAd, (UCHAR *)tmac_info, &mac_info, &pAd->CommonCfg.MlmeTransmit);
		RTMPMoveMemory(&pWirelessPkt[tx_hw_hdr_len], pNullFrm, frmLen);


//        	pAd->NullContext.BulkOutSize =  tx_hw_hdr_len + frameLen + 4;
#else
#if 0
		write_tmac_info(pAd, (UCHAR *)pTxWI, &mac_info, &pAd->CommonCfg.MlmeTransmit);
#ifdef RT_BIG_ENDIAN
		RTMPWIEndianChange(pAd, (PUCHAR)pTxWI, TYPE_TXWI);
#endif /* RT_BIG_ENDIAN */
		RTMPMoveMemory(&pWirelessPkt[TXWISize + TXINFO_SIZE + TSO_SIZE], pNullFrame, frameLen);
#ifdef RT_BIG_ENDIAN
		RTMPFrameEndianChange(pAd, (PUCHAR)&pWirelessPkt[TXINFO_SIZE + TXWISize + TSO_SIZE], DIR_WRITE, FALSE);
#endif /* RT_BIG_ENDIAN */
		pAd->NullContext.BulkOutSize =  TXINFO_SIZE + TXWISize + TSO_SIZE + frameLen + 4;
#endif
#endif /* MT_MAC */

//		pAd->NullContext.BulkOutSize = ( pAd->NullContext.BulkOutSize + 3) & (~3);

#ifdef MT_MAC
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("%s - Send NULL Frame @%d Mbps...(PwrMgmt, PID)(%d, %d)\n", __FUNCTION__, RateIdToMbps[pAd->CommonCfg.TxRate], pHeader_802_11->FC.PwrMgmt, mac_info.PID));
#else
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("%s - Send NULL Frame @%d Mbps...\n", __FUNCTION__, RateIdToMbps[pAd->CommonCfg.TxRate]));
#endif
//		RTUSB_SET_BULK_FLAG(pAd, fRTUSB_BULK_OUT_DATA_NULL);

		pAd->Sequence = (pAd->Sequence+1) & MAXSEQ;

		uLen = frmLen + pAd->chipCap.tx_hw_hdr_len;
		SET_OS_PKT_LEN(pPacket, uLen);

		pBuf= GET_OS_PKT_DATAPTR(pPacket);
		uLen= GET_OS_PKT_LEN(pPacket);
//		hex_dump("MtSDIONullFrameKickOut",pBuf,uLen);
		
		MtSdioSubmitMgmtToSDIOWorker(pAd, mac_info.q_idx, pPacket, pSrcBufVA, uLen); //SrcBufLen --> uLen
		
		RTMP_IRQ_UNLOCK(&pAd->MgmtRingLock, Flags);
			
		/* Kick bulk out */
//		RTUSBKickBulkOut(pAd);
	}
	
}

PNDIS_PACKET
GetPacketFromRxRing(
        IN PRTMP_ADAPTER prAd,
        IN RX_BLK *prRxBlk,
        OUT BOOLEAN  *pbReschedule,
        OUT UINT32 *pRxPending,
        IN UCHAR RxRingNo
)
{
	UINT8 rx_hw_hdr_len = prAd->chipCap.RXWISize;
	PNDIS_PACKET prRxPacket;
	unsigned long flags;

#if(CFG_SDIO_RX_AGG == 0) && (CFG_SDIO_INTR_ENHANCE == 0)

	prRxPacket = prAd->SDIORxPacket;
	PUCHAR prBuf = GET_OS_PKT_DATAPTR(prRxPacket);
	UINT32 u4len = GET_OS_PKT_LEN(prRxPacket);
	UINT32 u4i = 0;

	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("%s===========start\n",__FUNCTION__));

	prRxBlk->Flags = 0;

	if(!prRxPacket){
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("%s=====return NULL !!!!!\n",__FUNCTION__));
		return NULL;
	}

    if (!prAd->fgSDIORxPacKetInUsed) {
        return NULL;
    }

#if 0
	for(u4i = 0; u4i < u4len; u4i++){
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("%x", *(prBuf + u4i)));
	}
#endif
	//printk("before parse_rx_packet_type \n");


	rx_hw_hdr_len = parse_rx_packet_type(prAd, prRxBlk, prRxPacket);

	if (rx_hw_hdr_len == 0)
	{
		prRxBlk->DataSize = 0;
	}

	if (RX_BLK_TEST_FLAG(prRxBlk, fRX_CMD_RSP))
	{
		RELEASE_NDIS_PACKET(prAd, prRxPacket, NDIS_STATUS_SUCCESS);
		return NULL;
	}

	prRxBlk->pRxPacket = prRxPacket;
	prRxBlk->pData = (UCHAR *) GET_OS_PKT_DATAPTR(prRxPacket);
	prRxBlk->pHeader = (HEADER_802_11 *) (prRxBlk->pData);

    prAd->fgSDIORxPacKetInUsed = FALSE;
//#elif(CFG_SDIO_RX_AGG == 0) && (CFG_SDIO_INTR_ENHANCE == 1)
#else
	P_RX_CTRL_T prRxCtrl = NULL;
	ULONG IrqFlags = 0;
	QUEUE_ENTRY *pEntry;
	P_SW_RFB_T prSwRfb;
	PUCHAR prBuf;
	UINT32 u4len;
//	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("enter %s=====!!!!\n",__FUNCTION__));

	prRxCtrl = &prAd->rRxCtrl;
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_INFO, ("enter %s=====num: %x!!!!\n",__FUNCTION__, prRxCtrl->rReceivedRfbList.Number));

	if(prRxCtrl->rReceivedRfbList.Number==0)
	{
		return NULL;
	}
	RTMP_IRQ_LOCK(&prRxCtrl->rReceivedRfbLock, IrqFlags);
	pEntry = RemoveHeadQueue(&prRxCtrl->rReceivedRfbList);
	RTMP_IRQ_UNLOCK(&prRxCtrl->rReceivedRfbLock, IrqFlags)

	if(!pEntry){
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_INFO, ("enter ==>%s NULL!!!!\n",__FUNCTION__));
		return NULL;
	}
	prSwRfb = (P_SW_RFB_T)pEntry;

	prRxPacket = prSwRfb->prRxPacket;//prAd->SDIORxPacket;
	if(!prRxPacket){
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_INFO, ("%s=====return NULL !!!!!\n",__FUNCTION__));
		return NULL;
	}

	prBuf = GET_OS_PKT_DATAPTR(prRxPacket);
	u4len = GET_OS_PKT_LEN(prRxPacket);

	prRxBlk->Flags = 0;

/*
	printk("rx_packet_content: \n");
	for(u4i = 0; u4i < u4len; u4i++){
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("%x", *(prBuf + u4i)));
	}
*/
	rx_hw_hdr_len = parse_rx_packet_type(prAd, prRxBlk, prRxPacket);

	/*reallocate Packet buffer to rFreeSwRfbList*/
	prSwRfb->prRxPacket = RTMP_AllocateFragPacketBuffer(prAd, MAX_FRAME_SIZE);
	if (prSwRfb->prRxPacket == NULL)
	{
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,("%s():Cannot Allocate sk buffer for this rx buffer!\n", __FUNCTION__));
		return NULL;

	}

	RTMP_IRQ_LOCK(&prRxCtrl->rFreeSwRfbLock, flags);
	InsertTailQueue(&prRxCtrl->rFreeSwRfbList, &prSwRfb->rQueEntry);
	RTMP_IRQ_UNLOCK(&prRxCtrl->rFreeSwRfbLock, flags);


	if (rx_hw_hdr_len == 0)
	{
		prRxBlk->DataSize = 0;
	}

	if (RX_BLK_TEST_FLAG(prRxBlk, fRX_CMD_RSP))
	{
		RELEASE_NDIS_PACKET(prAd, prRxPacket, NDIS_STATUS_SUCCESS);
		return NULL;
	}

	prRxBlk->pRxPacket = prRxPacket;
	prRxBlk->pData = (UCHAR *) GET_OS_PKT_DATAPTR(prRxPacket);
	prRxBlk->pHeader = (HEADER_802_11 *) (prRxBlk->pData);


/*
	RTMP_IRQ_LOCK(&prRxCtrl->rReceivedRfbLock, IrqFlags);
	pEntry = RemoveHeadQueue(&prRxCtrl->rReceivedRfbList);
	prSwRfb = (P_SW_RFB_T)pEntry;
	RTMP_IRQ_UNLOCK(&prRxCtrl->rReceivedRfbLock, IrqFlags);
*/

	/*when to release packet and return to rFreeSwRfbList*/
//#elif(CFG_SDIO_RX_AGG == 1) && (CFG_SDIO_INTR_ENHANCE == 1)


#endif


#ifdef CFG_RX_AGG
#else


#endif

	return prRxPacket;
}

#if CFG_SDIO_TX_AGG

VOID MtSDIOAggTXInit(RTMP_ADAPTER *pAd)
{
    P_TX_CTRL_T prTxCtrl = &pAd->rTxCtrl;

	prTxCtrl->pucAggOffset = prTxCtrl->pucTxCoalescingBufPtr;
	prTxCtrl->u4AggLen =0;

}

VOID MtSDIOAggTXKickout(RTMP_ADAPTER *pAd)
{

	P_TX_CTRL_T prTxCtrl = &pAd->rTxCtrl;
	UCHAR *pucOutputBuf = NULL;

	//MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("(%s) \n", __FUNCTION__));
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_INFO, ("(%s) addr: %p dma_len: %x\n", __FUNCTION__, pucOutputBuf, prTxCtrl->u4AggLen));

	pucOutputBuf = prTxCtrl->pucTxCoalescingBufPtr;

	if(prTxCtrl->u4AggLen !=0){
		MTSDIOMultiWrite(pAd, WTDR1, pucOutputBuf, prTxCtrl->u4AggLen);
	}
	//hex_dump("xxx", pucOutputBuf, prTxBlk->Priv);


}

#endif

#ifdef CONFIG_STA_SUPPORT
VOID MtSDIOStaAsicForceWakeupTimeout(
	IN PVOID SystemSpecific1, 
	IN PVOID FunctionContext, 
	IN PVOID SystemSpecific2, 
	IN PVOID SystemSpecific3)
{
	RTMP_ADAPTER *pAd = (RTMP_ADAPTER *)FunctionContext;

#ifdef RELEASE_EXCLUDE
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_INFO, ("<-- RtmpUsbStaAsicForceWakeupTimeout\n"));
#endif /* RELEASE_EXCLUDE */
}


VOID MtSDIOStaAsicForceWakeup(RTMP_ADAPTER *pAd, BOOLEAN bFromTx)
{
	BOOLEAN	Canceled;

	/*7636 psm*/
#if defined (MT7636)
	RTMPOffloadPm(pAd, BSSID_WCID, PM4, EXIT_PM_STATE);
#else
	if (pAd->Mlme.AutoWakeupTimerRunning)
	{
		RTMPCancelTimer(&pAd->Mlme.AutoWakeupTimer, &Canceled);
		pAd->Mlme.AutoWakeupTimerRunning = FALSE;
	}

#ifdef MT7601
	if ( IS_MT7601(pAd) )
	{
		ASIC_RADIO_ON(pAd, DOT11_RADIO_ON);
	}
	else
#endif /* MT7601 */
	AsicSendCommandToMcu(pAd, 0x31, 0xff, 0x00, 0x02, FALSE);

	OPSTATUS_CLEAR_FLAG(pAd, fOP_STATUS_DOZE);
#endif /*7636 psm*/
}


VOID MtSDIOStaAsicSleepThenAutoWakeup(
	IN PRTMP_ADAPTER pAd, 
	IN USHORT TbttNumToNextWakeUp) 
{
	/*7636 psm*/
#if defined (MT7636)
	RTMPOffloadPm(pAd, BSSID_WCID, PM4, ENTER_PM_STATE);	
#else
#ifdef RELEASE_EXCLUDE
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_INFO, ("%s(): TbttNumToNextWakeUp=%d 1\n",
				__FUNCTION__, TbttNumToNextWakeUp));
#endif /* RELEASE_EXCLUDE */

	/* Not going to sleep if in the Count Down Time*/
	if (pAd->CountDowntoPsm > 0)
		return;

	/* we have decided to SLEEP, so at least do it for a BEACON period.*/
	if (TbttNumToNextWakeUp == 0)
		TbttNumToNextWakeUp = 1;

	RTMPSetTimer(&pAd->Mlme.AutoWakeupTimer, AUTO_WAKEUP_TIMEOUT);
	pAd->Mlme.AutoWakeupTimerRunning = TRUE;

#ifdef MT7601
	if ( IS_MT7601(pAd) )
	{
		ASIC_RADIO_OFF(pAd, DOT11_RADIO_OFF);
	}
	else
#endif /* MT7601 */
	{
		AsicSendCommandToMcu(pAd, 0x30, 0xff, 0xff, 0x02, FALSE);   /* send POWER-SAVE command to MCU. Timeout 40us.*/

		/* cancel bulk-in IRPs prevent blocking CPU enter C3.*/
		if((pAd->PendingRx > 0) && (!RTMP_TEST_FLAG(pAd, fRTMP_ADAPTER_NIC_NOT_EXIST)))
		{
			RTUSBCancelPendingBulkInIRP(pAd);
			/* resend bulk-in IRPs to receive beacons after a period of (pAd->CommonCfg.BeaconPeriod - 40) ms*/
			pAd->PendingRx = 0;
		}
	}

	OPSTATUS_SET_FLAG(pAd, fOP_STATUS_DOZE);
#ifdef RELEASE_EXCLUDE
	DBGPRINT(RT_DEBUG_INFO, ("<-- AsicSleepThenAutoWakeup, TbttNumToNextWakeUp=%d \n", TbttNumToNextWakeUp));
#endif /* RELEASE_EXCLUDE */
#endif /*7636 psm*/
}
#endif /* CONFIG_STA_SUPPORT */

