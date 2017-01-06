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
	mt_sdio.h
*/


#ifndef __MT_SDIO_H__
#define __MT_SDIO_H__

#include "rt_config.h"
#include "mtsdio_io.h"
#include "mtsdio_data.h"
#include "rtsdio_io.h"

struct _RTMP_ADAPTER;
struct _TX_BLK;

void rt_sdio_interrupt(struct sdio_func *func);
void RtmpInitSDIODevice(VOID *ad_src);

USHORT MtSDIO_WriteSubTxResource(struct _RTMP_ADAPTER *pAd, struct _TX_BLK *pTxBlk, BOOLEAN bIsLast, USHORT *freeCnt);
USHORT MtSDIO_WriteSingleTxResource(struct _RTMP_ADAPTER *pAd, struct _TX_BLK *pTxBlk, BOOLEAN bIsLast, USHORT *freeCnt);
USHORT MtSDIO_WriteFragTxResource(struct _RTMP_ADAPTER *pAd, struct _TX_BLK *pTxBlk, UCHAR fragNum, USHORT *freeCnt);
USHORT MtSDIO_WriteMultiTxResource(struct _RTMP_ADAPTER *pAd, struct _TX_BLK *pTxBlk, UCHAR frmNum, USHORT *freeCnt);
VOID MtSDIO_FinalWriteTxResource(struct _RTMP_ADAPTER *pAd, struct _TX_BLK *pTxBlk, USHORT mpdu_len, USHORT TxIdx);

VOID MtSDIODataLastTxIdx(struct _RTMP_ADAPTER *pAd, UCHAR QueIdx, USHORT TxIdx);
UINT32 MtSDIOTxResourceRequest(struct _RTMP_ADAPTER *pAd, UCHAR QueIdx, UINT16 u2PgCnt, UINT32 req_cnt);
VOID MtSDIODataKickOut(struct _RTMP_ADAPTER *pAd, struct _TX_BLK *pTxBlk, UCHAR QueIdx);
int MtSDIOMgmtKickOut(struct _RTMP_ADAPTER *pAd, UCHAR *pSrcBufVA, UINT SrcBufLen);
VOID MtSDIONullFrameKickOut(struct _RTMP_ADAPTER *pAd, UCHAR QIdx, UCHAR *pNullFrm, UINT32 frmLen);

VOID MTSDIOBssBeaconExit(struct _RTMP_ADAPTER *pAd);
VOID MTSDIOBssBeaconStop(struct _RTMP_ADAPTER *pAd);
VOID MTSDIOBssBeaconStart(struct _RTMP_ADAPTER * pAd);
VOID MTSDIOBssBeaconInit(struct _RTMP_ADAPTER *pAd);


VOID
MtSdioSubmitDataToSDIOWorker(
        IN struct _RTMP_ADAPTER *prAd
);

VOID
MtSdioSubmitMgmtToSDIOWorker(
        IN struct _RTMP_ADAPTER *prAd,
        IN UCHAR ucQueIdx,
        IN PNDIS_PACKET prPacket,
        IN UCHAR *prSrcBufVA,
        IN UINT u4SrcBufLen
        );

#if CFG_SDIO_TX_AGG
VOID MtSDIOAggTXInit(struct _RTMP_ADAPTER *pAd);
VOID MtSDIOAggTXKickout(struct _RTMP_ADAPTER *pAd);
#endif

#ifdef CONFIG_STA_SUPPORT
VOID MtSDIOStaAsicForceWakeupTimeout(
	IN PVOID SystemSpecific1, 
	IN PVOID FunctionContext, 
	IN PVOID SystemSpecific2, 
	IN PVOID SystemSpecific3);

VOID MtSDIOStaAsicForceWakeup(struct _RTMP_ADAPTER *pAd, BOOLEAN bFromTx);

VOID MtSDIOStaAsicSleepThenAutoWakeup(
	IN struct _RTMP_ADAPTER *pAd, 
	IN USHORT TbttNumToNextWakeUp);
#endif

#endif

