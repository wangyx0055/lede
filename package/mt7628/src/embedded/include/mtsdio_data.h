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
	mtsdio_data.h
*/

#include "rtmp_type.h"

INT32 MTSDIOCmdTx(struct _RTMP_ADAPTER *pAd, UCHAR *Buf, UINT32 Length);
INT32 MTSDIODataTx(struct _RTMP_ADAPTER *pAd, UCHAR *Buf, UINT32 Length);
VOID MTSDIODataIsr(struct _RTMP_ADAPTER *pAd);
INT
MTSDIOWorkerThread(
        IN ULONG ulContext
);

INT
MTSDIOCmdThread(
        IN ULONG ulContext
);

VOID
MTSDIOWorkerThreadEventNotify(
        IN struct _RTMP_ADAPTER *prAd,
        IN UINT32 u4EventBit
);

#if 1 /* SDIO flow control API */
VOID
MTSDIOTxResetResource (
        IN struct _RTMP_ADAPTER *prAd
);

VOID MTSDIOTxInitResetResource (
        IN struct _RTMP_ADAPTER *prAd
);

UINT8
MTSDIOTxGetCmdResourceType(
        IN struct _RTMP_ADAPTER *prAd
);

INT32
MTSDIOTxAcquireResource(
        IN struct _RTMP_ADAPTER *prAd,
        IN UINT8       ucTC,
        IN UINT8       ucPageCount
);
ULONG
MTSDIOTxQueryPageResource(
        IN struct _RTMP_ADAPTER *prAd,
    IN UINT8       ucTC
);

UINT8
MTSDIOTxGetPageCount(
        IN UINT32 u4FrameLength,
        IN BOOLEAN fgIncludeDesc
);

BOOLEAN
MTSDIOTxReleaseResource(
        IN struct _RTMP_ADAPTER *prAd,
        IN UINT16       *au2TxRlsCnt
);

VOID
MTSDIOTxInitialize(
        IN struct _RTMP_ADAPTER *prAd
);
VOID
MTSDIORxInitialize(
        IN struct _RTMP_ADAPTER *prAd
);
VOID
MTSDIOTxUninitialize(
        IN struct _RTMP_ADAPTER *prAd
);
VOID
MTSDIORxUninitialize(
        IN struct _RTMP_ADAPTER *prAd
);

#if(CFG_SDIO_RX_AGG == 0) && (CFG_SDIO_INTR_ENHANCE == 0)
INT32 MTSDIORxReadBuffer(IN struct _RTMP_ADAPTER *prAd);
#elif(CFG_SDIO_RX_AGG == 0) && (CFG_SDIO_INTR_ENHANCE == 1)
INT32 MTSDIORxEnhancedReadBuffer(IN struct _RTMP_ADAPTER *prAd);
#elif(CFG_SDIO_RX_AGG == 1) && (CFG_SDIO_INTR_ENHANCE == 1)
INT32 MTSDIORxEnhancedAggReadBuffer(IN struct _RTMP_ADAPTER *prAd);
#endif


#endif
