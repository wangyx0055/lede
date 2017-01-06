/****************************************************************************
 * Ralink Tech Inc.
 * Taiwan, R.O.C.
 *
 * (c) Copyright 2002, Ralink Technology, Inc.
 *
 * All rights reserved. Ralink's source code is an unpublished work and the
 * use of a copyright notice does not imply otherwise. This source code
 * contains confidential trade secret material of Ralink Tech. Any attemp
 * or participation in deciphering, decoding, reverse engineering or in any
 * way altering the source code is stricitly prohibited, unless the prior
 * written consent of Ralink Technology, Inc. is obtained.
 ***************************************************************************/

#include	"rt_config.h"

static NDIS_STATUS NICInitRecv (
    IN PRTMP_ADAPTER prAd);
static NDIS_STATUS NICInitTransmit(
    IN PRTMP_ADAPTER prAd);

#ifdef RESOURCE_PRE_ALLOC

static NDIS_STATUS NICInitRecv (
        IN PRTMP_ADAPTER prAd
)
{
    NDIS_STATUS rStatus = NDIS_STATUS_SUCCESS;

    MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("<-- NICInitRecv(Status=%d)\n", rStatus));
    MTSDIORxInitialize(prAd);

    return rStatus;
}

static NDIS_STATUS NICInitTransmit(
        IN PRTMP_ADAPTER prAd
)
{
    NDIS_STATUS rStatus = NDIS_STATUS_SUCCESS;
    RTMP_MGMT_RING  *prMgmtRing = NULL;
    PVOID           *prRingBaseVa = NULL;
    UINT32          u4i = 0;
#ifdef MT_MAC
    RTMP_BCN_RING   *prBcnRing = NULL;
#endif
#if 0
    P_MSDU_INFO_T   *prMsduInfo = NULL;
#endif

    MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("<-- NICInitTransmit(Status=%d)\n", rStatus));

    MTSDIOTxInitialize(prAd);

    /* <1> allocate sw queue per AC */
    rtmp_tx_swq_init(prAd);

    do {
        /* <2> Init MGMT ring descriptor's memory */
        NdisZeroMemory(prAd->MgmtDescRing.AllocVa, prAd->MgmtDescRing.AllocSize);
        prRingBaseVa = prAd->MgmtDescRing.AllocVa;

        prMgmtRing = &prAd->MgmtRing;

        for (u4i = 0; u4i < MGMT_RING_SIZE; u4i++) {
            //prMgmtRing->Cell[u4i].AllocSize = sizeof(MSDU_INFO_T);
            prMgmtRing->Cell[u4i].AllocSize = 0;
            prMgmtRing->Cell[u4i].AllocVa = prRingBaseVa;
            prMgmtRing->Cell[u4i].pNdisPacket = NULL;
            prMgmtRing->Cell[u4i].pNextNdisPacket = NULL;
            /* fill the CONTAINER_CONTEXT */
            //prMsduInfo = (P_MSDU_INFO_T) prAd->MgmtRing.Cell[u4i].AllocVa;
            //prMsduInfo-> u4TxLength = 0;
            /* Offset to next ring descriptor address*/
            //prRingBaseVa = (PVOID) ((PUCHAR) (prRingBaseVa + sizeof(MSDU_INFO_T)));
}

        prAd->MgmtRing.TxSwFreeIdx = MGMT_RING_SIZE;
        prAd->MgmtRing.TxCpuIdx = 0;
        prAd->MgmtRing.TxDmaIdx = 0;

#ifdef MT_MAC
        /* Init  BCN ring descriptor's memory*/
        NdisZeroMemory(prAd->BcnDescRing.AllocVa, prAd->BcnDescRing.AllocSize);
        prRingBaseVa = prAd->BcnDescRing.AllocVa;

        /* Initialize BCN Ring and associated buffer memory*/
        prBcnRing = &prAd->BcnRing;
        for (u4i = 0; u4i < BCN_RING_SIZE; u4i++)
{
            /* link the pre-allocated Bcn buffer to MgmtRing.Cell*/
            //prBcnRing->Cell[u4i].AllocSize = sizeof(MSDU_INFO_T);
            prBcnRing->Cell[u4i].AllocSize = 0;
            prBcnRing->Cell[u4i].AllocVa = prRingBaseVa;
            prBcnRing->Cell[u4i].pNdisPacket = NULL;
            prBcnRing->Cell[u4i].pNextNdisPacket = NULL;

            // Fill container
            //prMsduInfo = (P_MSDU_INFO_T) prAd->BcnRing.Cell[u4i].AllocVa;
            //prMsduInfo->u4TxLength = 0;
            /* Offset to next ring descriptor address*/
            //prRingBaseVa = (PVOID) ((PUCHAR) prRingBaseVa + sizeof(MSDU_INFO_T));
        }
        MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("BCN Ring: total %d entry allocated\n", u4i));

        prAd->BcnRing.TxSwFreeIdx = BCN_RING_SIZE;
        prAd->BcnRing.TxCpuIdx = 0;
        prAd->BcnRing.TxDmaIdx = 0;

#endif
        MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("MGMT Ring: total %d entry allocated\n", u4i));

    } while (FALSE);

    return rStatus;
err:
    /* MGMT Ring*/
    if (prAd->MgmtDescRing.AllocVa)
	{
        os_free_mem(prAd, prAd->MgmtDescRing.AllocVa);
        prAd->MgmtDescRing.AllocVa = NULL;
	}
#ifdef MT_MAC
    if (prAd->BcnDescRing.AllocVa)
    {
        os_free_mem(prAd, prAd->BcnDescRing.AllocVa);
        prAd->BcnDescRing.AllocVa = NULL;
}
#endif

    if (prAd->FragFrame.pFragPacket)
        RELEASE_NDIS_PACKET(prAd, prAd->FragFrame.pFragPacket, NDIS_STATUS_SUCCESS);

    if (prAd->pucCoalescingBufCached)
        os_free_mem(prAd, prAd->pucCoalescingBufCached);

    return rStatus;
}

VOID RTMPResetTxRxRingMemory(
        IN PRTMP_ADAPTER prAd
)
{
    ASSERT(prAd);

    rtmp_tx_swq_exit(prAd, WCID_ALL);

    NdisFreeSpinLock(&prAd->CmdQLock);
#ifdef CONFIG_ATE
    NdisFreeSpinLock(&prAd->GenericLock);
#endif /* CONFIG_ATE */
    MTSDIOTxUninitialize(prAd);
    MTSDIORxUninitialize(prAd);
}

VOID RTMPFreeTxRxRingMemory(
        IN PRTMP_ADAPTER prAd
)
	{
    MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("---> RTMPFreeTxRxRingMemory\n"));

    if (prAd->MgmtDescRing.AllocVa)
        os_free_mem(prAd, prAd->MgmtDescRing.AllocVa);

#ifdef MT_MAC
    if (prAd->BcnDescRing.AllocVa)
        os_free_mem(prAd, prAd->BcnDescRing.AllocVa);
#endif /* MT_MAC */

    if (prAd->FragFrame.pFragPacket)
        RELEASE_NDIS_PACKET(prAd, prAd->FragFrame.pFragPacket, NDIS_STATUS_SUCCESS);

    if (prAd->pucCoalescingBufCached)
        os_free_mem(prAd, prAd->pucCoalescingBufCached);

    MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("<--- RTMPFreeTxRxRingMemory\n"));
}


NDIS_STATUS
RTMPAllocTxRxRingMemory(
        IN PRTMP_ADAPTER prAd
)
{
    NDIS_STATUS rStatus = NDIS_STATUS_SUCCESS;

    MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("-->RTMPAllocTxRxRingMemory\n"));

    prAd->pucCoalescingBufCached = (PUINT8) NULL;

    /* Allocate memory for the common coalescing buffer. */
#if((CFG_SDIO_RX_AGG == 1) && (CFG_SDIO_INTR_ENHANCE == 1))||(CFG_SDIO_TX_AGG==1)

    prAd->u4CoalescingBufCachedSize = CFG_TX_COALESCING_BUFFER_SIZE > CFG_RX_COALESCING_BUFFER_SIZE ?
         CFG_TX_COALESCING_BUFFER_SIZE : CFG_RX_COALESCING_BUFFER_SIZE;

#else
    prAd->u4CoalescingBufCachedSize = MAX_AGGREGATION_SIZE;
#endif
    os_alloc_mem(prAd, (PUCHAR *) &prAd->pucCoalescingBufCached, prAd->u4CoalescingBufCachedSize);

    if(prAd->pucCoalescingBufCached == NULL)
        DBGPRINT_ERR(("Failed to allocate a big buffer for Coalescing buffer !\n"));

    /* Allocate MGMT ring descriptor's memory*/
#if 0
    prAd->MgmtDescRing.AllocSize = MGMT_RING_SIZE * sizeof(MSDU_INFO_T);
    os_alloc_mem(prAd, (PUCHAR *)(&prAd->MgmtDescRing.AllocVa), prAd->MgmtDescRing.AllocSize);
    if (prAd->MgmtDescRing.AllocVa == NULL)
{
        DBGPRINT_ERR(("Failed to allocate a big buffer for MgmtDescRing!\n"));
        rStatus = NDIS_STATUS_RESOURCES;
        goto err;
    }
#endif

#ifdef MT_MAC
    /* Initialize BCN Ring and associated buffer memory */
    /* Allocate BCN ring descriptor's memory*/
#if 0
    prAd->BcnDescRing.AllocSize = BCN_RING_SIZE * sizeof(MSDU_INFO_T);
    os_alloc_mem(prAd, (PUCHAR *)(&prAd->BcnDescRing.AllocVa), prAd->BcnDescRing.AllocSize);
    if (prAd->BcnDescRing.AllocVa == NULL)
	{
        DBGPRINT_ERR(("Failed to allocate a big buffer for MgmtDescRing!\n"));
        rStatus = NDIS_STATUS_RESOURCES;
        goto err;
    }
#endif
#endif /* MT_MAC */

    prAd->FragFrame.pFragPacket =  RTMP_AllocateFragPacketBuffer(prAd, RX_BUFFER_NORMSIZE);

    if (prAd->FragFrame.pFragPacket == NULL)
    {
        rStatus = NDIS_STATUS_RESOURCES;
        goto err;
    }

    return rStatus;
err:
    RTMPFreeTxRxRingMemory(prAd);

    return rStatus;
	}


NDIS_STATUS
RTMPInitTxRxRingMemory(
        IN PRTMP_ADAPTER prAd
)
{
    NDIS_STATUS rStatus = NDIS_STATUS_SUCCESS;

    /* Init the CmdQ and CmdQLock*/
    NdisAllocateSpinLock(prAd, &prAd->CmdQLock);
    NdisAcquireSpinLock(&prAd->CmdQLock);
    RTInitializeCmdQ(&prAd->CmdQ);
    NdisReleaseSpinLock(&prAd->CmdQLock);

#ifdef CONFIG_ATE
    NdisAllocateSpinLock(prAd, &prAd->GenericLock);
#endif /* CONFIG_ATE */


    NICInitRecv(prAd);
    rStatus = NICInitTransmit(prAd);

    return rStatus;
}
#else
#error "not yet implemented"
#endif


/*
========================================================================
Routine Description:
    Disable DMA.

Arguments:
	*pAd				the raxx interface data pointer

Return Value:
	None

Note:
========================================================================
*/
VOID RT28XXDMADisable(
	IN RTMP_ADAPTER 		*pAd)
{
	/* no use*/
}


/*
========================================================================
Routine Description:
    Enable DMA.

Arguments:
	*pAd				the raxx interface data pointer

Return Value:
	None

Note:
========================================================================
*/
VOID RT28XXDMAEnable(RTMP_ADAPTER *pAd)
{

}


/********************************************************************
  *
  *	SDIO Radio on/off Related functions.
  *
  ********************************************************************/
VOID RT28xxSdioMlmeRadioOn(
	IN PRTMP_ADAPTER pAd)
{
}


VOID RT28xxSdioMlmeRadioOFF(
	IN PRTMP_ADAPTER pAd)
{
}


VOID RT28xxSdioAsicRadioOff(RTMP_ADAPTER *pAd)
{
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("--> %s\n", __FUNCTION__));

	RTMP_SET_FLAG(pAd, fRTMP_ADAPTER_IDLE_RADIO_OFF);

	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("<== %s\n", __FUNCTION__));

}


VOID RT28xxSdioAsicRadioOn(RTMP_ADAPTER *pAd)
{
	/* make some traffic to invoke EvtDeviceD0Entry callback function*/

	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("<== %s\n", __FUNCTION__));
}


VOID RT28xx_UpdateBeaconToAsic(
	RTMP_ADAPTER *pAd,
	INT apidx,
	ULONG FrameLen,
	ULONG UpdatePos)
{
	BCN_BUF_STRUC *bcn_buf = NULL;
	UCHAR *buf;
	INT len;
	PNDIS_PACKET *pkt = NULL;

#ifdef CONFIG_AP_SUPPORT
	if ((pAd->OpMode == OPMODE_AP)
#ifdef RT_CFG80211_P2P_SUPPORT
	    || (pAd->cfg80211_ctrl.isCfgInApMode == RT_CMD_80211_IFTYPE_AP)
#endif /* RT_CFG80211_P2P_SUPPORT */
        )
	{
		bcn_buf = &pAd->ApCfg.MBSSID[apidx].bcn_buf;
	}
#endif /* CONFIG_AP_SUPPORT */

#ifdef CONFIG_STA_SUPPORT
	IF_DEV_CONFIG_OPMODE_ON_STA(pAd) {
		if (pAd->StaCfg.BssType == BSS_ADHOC)
			bcn_buf = &pAd->StaCfg.bcn_buf;
	}
#endif /* CONFIG_STA_SUPPORT */

	if (!bcn_buf) {
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
			("%s(): bcn_buf is NULL!\n", __FUNCTION__));
		return;
	}

	pkt = bcn_buf->BeaconPkt;
	if (pkt) {
		buf = (UCHAR *)GET_OS_PKT_DATAPTR(pkt);
		len = FrameLen + pAd->chipCap.tx_hw_hdr_len;
		SET_OS_PKT_LEN(pkt, len);
#ifdef RT_BIG_ENDIAN
		MTMacInfoEndianChange(pAd, buf, TYPE_TMACINFO,
			sizeof(TMAC_TXD_L));
#endif /* RT_BIG_ENDIAN */

#ifdef RTMP_SDIO_SUPPORT
		//hex_dump("bcn before MtSdioSubmitMgmtToSDIOWorker",buf,len);
		ULONG Flags = 0;
		RTMP_IRQ_LOCK(&pAd->MgmtRingLock, Flags);
    		MtSdioSubmitMgmtToSDIOWorker(pAd, Q_IDX_BCN, pkt, buf,
					len);
		RTMP_IRQ_UNLOCK(&pAd->MgmtRingLock, Flags);
#else
		/* Now do hardware-depened kick out.*/
		HAL_KickOutMgmtTx(pAd, Q_IDX_BCN, pkt, buf, len);
#endif
		bcn_buf->bcn_state = BCN_TX_WRITE_TO_DMA;
	} else {
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
			("%s(): BeaconPkt is NULL!\n", __FUNCTION__));
	}
}



/*
    ========================================================================
    Routine Description:
        For device work as AP mode but didn't have TBTT interrupt event, we need a mechanism
        to update the beacon context in each Beacon interval. Here we use a periodical timer
        to simulate the TBTT interrupt to handle the beacon context update.

    Arguments:
        SystemSpecific1         - Not used.
        FunctionContext         - Pointer to our Adapter context.
        SystemSpecific2         - Not used.
        SystemSpecific3         - Not used.

    Return Value:
        None

    ========================================================================
*/


VOID MTSDIOBssBeaconStop(
	IN RTMP_ADAPTER *pAd)
{
	printk("%s\n", __func__);
	BEACON_SYNC_STRUCT	*pBeaconSync;
	int i, offset;
	BOOLEAN	Cancelled = TRUE;
	UINT8 TXWISize = pAd->chipCap.TXWISize;

	pBeaconSync = pAd->CommonCfg.pBeaconSync;
	if (pBeaconSync && pBeaconSync->EnableBeacon)
	{
		INT NumOfBcn = 0;

#ifdef CONFIG_AP_SUPPORT
		IF_DEV_CONFIG_OPMODE_ON_AP(pAd)
		{
			NumOfBcn = pAd->ApCfg.BssidNum + MAX_MESH_NUM;
		}
#endif /* CONFIG_AP_SUPPORT */

#ifdef CONFIG_STA_SUPPORT
		IF_DEV_CONFIG_OPMODE_ON_STA(pAd)
		{
			NumOfBcn = MAX_MESH_NUM;
#ifdef P2P_SUPPORT
			NumOfBcn +=  MAX_P2P_NUM;
#endif /* P2P_SUPPORT */
		}
#endif /* CONFIG_STA_SUPPORT */

		RTMPCancelTimer(&pAd->CommonCfg.BeaconUpdateTimer, &Cancelled);

		for(i=0; i<NumOfBcn; i++)
		{
			NdisZeroMemory(pBeaconSync->BeaconBuf[i], HW_BEACON_OFFSET);
			NdisZeroMemory(pBeaconSync->BeaconTxWI[i], TXWISize);

			for (offset=0; offset<HW_BEACON_OFFSET; offset+=4)
				RTMP_CHIP_UPDATE_BEACON(pAd, pAd->BeaconOffset[i] + offset, 0x00, 4);

			pBeaconSync->CapabilityInfoLocationInBeacon[i] = 0;
			pBeaconSync->TimIELocationInBeacon[i] = 0;
		}
		pBeaconSync->BeaconBitMap = 0;
		pBeaconSync->DtimBitOn = 0;
	}
}


VOID MTSDIOBssBeaconStart(
	IN RTMP_ADAPTER *pAd)
{
	printk("%s\n", __func__);
	int apidx;
	BEACON_SYNC_STRUCT	*pBeaconSync;
	UINT8 TXWISize = pAd->chipCap.TXWISize;
/*	LARGE_INTEGER 	tsfTime, deltaTime;*/

	pBeaconSync = pAd->CommonCfg.pBeaconSync;
	if (pBeaconSync && pBeaconSync->EnableBeacon)
	{
		INT NumOfBcn = 0;

#ifdef CONFIG_AP_SUPPORT
		IF_DEV_CONFIG_OPMODE_ON_AP(pAd)
		{
			NumOfBcn = pAd->ApCfg.BssidNum + MAX_MESH_NUM;
		}
#endif /* CONFIG_AP_SUPPORT */

#ifdef CONFIG_STA_SUPPORT
		IF_DEV_CONFIG_OPMODE_ON_STA(pAd)
		{
			NumOfBcn = MAX_MESH_NUM;
#ifdef P2P_SUPPORT
			NumOfBcn +=  MAX_P2P_NUM;
#endif /* P2P_SUPPORT */
		}
#endif /* CONFIG_STA_SUPPORT */

		for(apidx=0; apidx<NumOfBcn; apidx++)
		{
			UCHAR CapabilityInfoLocationInBeacon = 0;
			UCHAR TimIELocationInBeacon = 0;
#ifdef CONFIG_AP_SUPPORT
			IF_DEV_CONFIG_OPMODE_ON_AP(pAd)
			{
#ifdef MESH_SUPPORT
				if (apidx == MESH_BEACON_IDX(pAd))
				{
					CapabilityInfoLocationInBeacon = pAd->MeshTab.bcn_buf.cap_ie_pos;
					TimIELocationInBeacon = pAd->MeshTab.TimIELocationInBeacon;
				}
				else
#endif /* MESH_SUPPORT */
				{
					CapabilityInfoLocationInBeacon = pAd->ApCfg.MBSSID[apidx].bcn_buf.cap_ie_pos;
					TimIELocationInBeacon = pAd->ApCfg.MBSSID[apidx].TimIELocationInBeacon;
				}
			}
#endif /* CONFIG_AP_SUPPORT */

#ifdef CONFIG_STA_SUPPORT
#ifdef P2P_SUPPORT
#ifdef MESH_SUPPORT
				if (apidx == MESH_BEACON_IDX(pAd))
				{
					CapabilityInfoLocationInBeacon = pAd->MeshTab.bcn_buf.cap_ie_pos;
					TimIELocationInBeacon = pAd->MeshTab.TimIELocationInBeacon;
				}
				else
#endif /* MESH_SUPPORT */
				{
					CapabilityInfoLocationInBeacon = pAd->ApCfg.MBSSID[apidx].bcn_buf.cap_ie_pos;
					TimIELocationInBeacon = pAd->ApCfg.MBSSID[apidx].TimIELocationInBeacon;
				}
#else
#ifdef MESH_SUPPORT
			IF_DEV_CONFIG_OPMODE_ON_STA(pAd)
			{
				CapabilityInfoLocationInBeacon = pAd->MeshTab.bcn_buf.cap_ie_pos;
				TimIELocationInBeacon = pAd->MeshTab.TimIELocationInBeacon;
			}
#endif /* MESH_SUPPORT */
#endif /* P2P_SUPPORT */
#endif /* CONFIG_STA_SUPPORT */

			NdisZeroMemory(pBeaconSync->BeaconBuf[apidx], HW_BEACON_OFFSET);
			pBeaconSync->CapabilityInfoLocationInBeacon[apidx] = CapabilityInfoLocationInBeacon;
			pBeaconSync->TimIELocationInBeacon[apidx] = TimIELocationInBeacon;
			NdisZeroMemory(pBeaconSync->BeaconTxWI[apidx], TXWISize);
		}
		pBeaconSync->BeaconBitMap = 0;
		pBeaconSync->DtimBitOn = 0;
		pAd->CommonCfg.BeaconUpdateTimer.Repeat = TRUE;

		pAd->CommonCfg.BeaconAdjust = 0;
		pAd->CommonCfg.BeaconFactor = 0xffffffff / (pAd->CommonCfg.BeaconPeriod << 10);
		pAd->CommonCfg.BeaconRemain = (0xffffffff % (pAd->CommonCfg.BeaconPeriod << 10)) + 1;
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("MTSDIOBssBeaconStart:BeaconFactor=%d, BeaconRemain=%d!\n",
									pAd->CommonCfg.BeaconFactor, pAd->CommonCfg.BeaconRemain));
		RTMPSetTimer(&pAd->CommonCfg.BeaconUpdateTimer, 10 /*pAd->CommonCfg.BeaconPeriod*/);

	}
}


VOID MTSDIOBssBeaconInit(
	IN RTMP_ADAPTER *pAd)
{
	printk("%s\n", __func__);

	BEACON_SYNC_STRUCT	*pBeaconSync;
	int i, j;
	UINT8 TXWISize = pAd->chipCap.TXWISize;

	os_alloc_mem(pAd, (PUCHAR *)(&pAd->CommonCfg.pBeaconSync), sizeof(BEACON_SYNC_STRUCT));

	if (pAd->CommonCfg.pBeaconSync)
	{
		pBeaconSync = pAd->CommonCfg.pBeaconSync;
		NdisZeroMemory(pBeaconSync, sizeof(BEACON_SYNC_STRUCT));
		for(i=0; i < HW_BEACON_MAX_COUNT(pAd); i++)
		{
			NdisZeroMemory(pBeaconSync->BeaconBuf[i], HW_BEACON_OFFSET);
			pBeaconSync->CapabilityInfoLocationInBeacon[i] = 0;
			pBeaconSync->TimIELocationInBeacon[i] = 0;
			os_alloc_mem(pAd, &pBeaconSync->BeaconTxWI[i], TXWISize);
			if (pBeaconSync->BeaconTxWI[i])
				NdisZeroMemory(pBeaconSync->BeaconTxWI[i], TXWISize);
			else
				goto error2;
		}
		pBeaconSync->BeaconBitMap = 0;

		/*RTMPInitTimer(pAd, &pAd->CommonCfg.BeaconUpdateTimer, GET_TIMER_FUNCTION(BeaconUpdateExec), pAd, TRUE);*/
		pBeaconSync->EnableBeacon = TRUE;
	}else
		goto error1;

	return;

error2:
	for (j = 0; j < i; j++)
		os_free_mem(pAd, pBeaconSync->BeaconTxWI[j]);

	os_free_mem(pAd, pAd->CommonCfg.pBeaconSync);

error1:
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("memory are not available\n"));
}


VOID MTSDIOBssBeaconExit(
	IN RTMP_ADAPTER *pAd)
{
	printk("%s\n", __func__);
	BEACON_SYNC_STRUCT	*pBeaconSync;
	BOOLEAN	Cancelled = TRUE;
	int i;

	if (pAd->CommonCfg.pBeaconSync)
	{
		pBeaconSync = pAd->CommonCfg.pBeaconSync;
		pBeaconSync->EnableBeacon = FALSE;
		RTMPCancelTimer(&pAd->CommonCfg.BeaconUpdateTimer, &Cancelled);
		pBeaconSync->BeaconBitMap = 0;

		for(i=0; i<HW_BEACON_MAX_COUNT(pAd); i++)
		{
			NdisZeroMemory(pBeaconSync->BeaconBuf[i], HW_BEACON_OFFSET);
			pBeaconSync->CapabilityInfoLocationInBeacon[i] = 0;
			pBeaconSync->TimIELocationInBeacon[i] = 0;
			os_free_mem(pAd, pBeaconSync->BeaconTxWI[i]);
		}

		os_free_mem(pAd, pAd->CommonCfg.pBeaconSync);
		pAd->CommonCfg.pBeaconSync = NULL;
	}
}

VOID BeaconUpdateExec(
    IN PVOID SystemSpecific1,
    IN PVOID FunctionContext,
    IN PVOID SystemSpecific2,
    IN PVOID SystemSpecific3)
{
	PRTMP_ADAPTER	pAd = (PRTMP_ADAPTER)FunctionContext;
	LARGE_INTEGER	tsfTime_a;/*, tsfTime_b, deltaTime_exp, deltaTime_ab;*/
	UINT32			delta, delta2MS, period2US, remain, remain_low, remain_high;
/*	BOOLEAN			positive;*/

	if (pAd->CommonCfg.IsUpdateBeacon==TRUE)
	{
#ifdef CONFIG_AP_SUPPORT
#ifdef RT_CFG80211_P2P_SUPPORT
		if (pAd->cfg80211_ctrl.isCfgInApMode == RT_CMD_80211_IFTYPE_AP)
#else
#ifdef P2P_SUPPORT
		if (P2P_INF_ON(pAd) && P2P_GO_ON(pAd))
#else
		IF_DEV_CONFIG_OPMODE_ON_AP(pAd)
#endif /* P2P_SUPPORT */
#endif /* RT_CFG80211_P2P_SUPPORT */
		{
			BEACON_SYNC_STRUCT *pBeaconSync = pAd->CommonCfg.pBeaconSync;
			ULONG UpTime;

			/* update channel utilization */
			NdisGetSystemUpTime(&UpTime);

#ifdef AP_QLOAD_SUPPORT
			QBSS_LoadUpdate(pAd, UpTime);
#endif /* AP_QLOAD_SUPPORT */

#ifdef DOT11K_RRM_SUPPORT
			RRM_QuietUpdata(pAd);
#endif /* DOT11K_RRM_SUPPORT */

//TODO: Carter, check how to use USE_BMC flag
			//if (pAd->ApCfg.DtimCount == 0 && pBeaconSync->DtimBitOn)
			//{
			//	POS_COOKIE pObj;

			//	pObj = (POS_COOKIE) pAd->OS_Cookie;
			//	RTMP_OS_TASKLET_SCHE(&pObj->tbtt_task);
			//}

#ifdef RT_CFG80211_P2P_SUPPORT
			RT_CFG80211_BEACON_TIM_UPDATE(pAd);
#else
			APUpdateAllBeaconFrame(pAd);
#endif /* RT_CFG80211_P2P_SUPPORT */
		}
#endif /* CONFIG_AP_SUPPORT */

#ifdef MESH_SUPPORT
		MeshUpdateBeaconFrame(pAd, MESH_BEACON_IDX(pAd));
#endif /* MESH_SUPPORT */
	}

	AsicGetTsfTime(pAd, &tsfTime_a.u.HighPart, &tsfTime_a.u.LowPart);

	/*
		Calculate next beacon time to wake up to update.

		BeaconRemain = (0xffffffff % (pAd->CommonCfg.BeaconPeriod << 10)) + 1;

		Background: Timestamp (us) % Beacon Period (us) shall be 0 at TBTT

		Formula:	(a+b) mod m = ((a mod m) + (b mod m)) mod m
					(a*b) mod m = ((a mod m) * (b mod m)) mod m

		==> ((HighPart * 0xFFFFFFFF) + LowPart) mod Beacon_Period
		==> (((HighPart * 0xFFFFFFFF) mod Beacon_Period) +
			(LowPart mod (Beacon_Period))) mod Beacon_Period
		==> ((HighPart mod Beacon_Period) * (0xFFFFFFFF mod Beacon_Period)) mod
			Beacon_Period

		Steps:
		1. Calculate the delta time between now and next TBTT;

			delta time = (Beacon Period) - ((64-bit timestamp) % (Beacon Period))

			(1) If no overflow for LowPart, 32-bit, we can calcualte the delta
				time by using LowPart;

				delta time = LowPart % (Beacon Period)

			(2) If overflow for LowPart, we need to care about HighPart value;

				delta time = (BeaconRemain * HighPart + LowPart) % (Beacon Period)

				Ex: if the maximum value is 0x00 0xFF (255), Beacon Period = 100,
					TBTT timestamp will be 100, 200, 300, 400, ...
					when TBTT timestamp is 300 = 1*56 + 44, means HighPart = 1,
					Low Part = 44

		2. Adjust next update time of the timer to (delta time + 10ms).
	*/

	/*positive=getDeltaTime(tsfTime_a, expectedTime, &deltaTime_exp);*/
	period2US = (pAd->CommonCfg.BeaconPeriod << 10);
	remain_high = pAd->CommonCfg.BeaconRemain * tsfTime_a.u.HighPart;
	remain_low = tsfTime_a.u.LowPart % (pAd->CommonCfg.BeaconPeriod << 10);
	remain = (remain_high + remain_low)%(pAd->CommonCfg.BeaconPeriod << 10);
	delta = (pAd->CommonCfg.BeaconPeriod << 10) - remain;

	delta2MS = (delta>>10);
	if (delta2MS > 150)
	{
		pAd->CommonCfg.BeaconUpdateTimer.TimerValue = 100;
		pAd->CommonCfg.IsUpdateBeacon=FALSE;
	}
	else
	{
		pAd->CommonCfg.BeaconUpdateTimer.TimerValue = delta2MS + 10;
		pAd->CommonCfg.IsUpdateBeacon=TRUE;
	}
#ifdef CONFIG_AP_SUPPORT
	IF_DEV_CONFIG_OPMODE_ON_AP(pAd)
	{
		if ((pAd->CommonCfg.Channel > 14)
			&& (pAd->CommonCfg.bIEEE80211H == 1)
			&& (pAd->Dot11_H.RDMode == RD_SWITCHING_MODE))
		{
			ChannelSwitchingCountDownProc(pAd);
		}
	}
#endif /* CONFIG_AP_SUPPORT */
}
