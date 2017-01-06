/*
 ***************************************************************************
 * MediaTek Inc.
 * 5F, No.5, Tai-Yuan 1st St., 
 * Chupei City, Hsinchu County 30265, Taiwan, R.O.C.
 *
 * (c) Copyright 2014-2016, MediaTek, Inc.
 *
 * All rights reserved. MediaTek's source code is an unpublished work and the
 * use of a copyright notice does not imply otherwise. This source code
 * contains confidential trade secret material of MediaTek. Any attemp
 * or participation in deciphering, decoding, reverse engineering or in any
 * way altering the source code is stricitly prohibited, unless the prior
 * written consent of MediaTek, Inc. is obtained.
 ***************************************************************************

    Module Name:
    Receive.h

    Abstract:
    Miniport generic portion header file

    Revision History:
    Who         When          What
    --------    ----------    ----------------------------------------------
    */

#ifndef __RECV_H__
#define __RECV_H__
#include    "MtConfig.h"


BOOLEAN 
RecvFwCmd7603(
    IN  PMP_ADAPTER  pAd,
    IN  PRX_CONTEXT     pRxContext
    );

VOID
RecvProcessUnicastDataPkt(
    IN  PMP_ADAPTER  pAd, 
    IN  PMP_PORT       pPort,
    IN  PHEADER_802_11  pHeader,
    IN  PUCHAR               pData,
    IN  PRXDSCR_BASE_STRUC pRxD,
    IN  SHORT                 PacketSize,
    IN  SHORT                 DataSize,
    IN  UCHAR                 UserPriority,
    IN  USHORT               Msdu2Size,
    IN  BOOLEAN             EAPOLFrame,
    IN  BOOLEAN             bAMsdu,
    IN  BOOLEAN             QueueEAPOLFrame,
    IN  BOOLEAN             WAIFrame,
    IN  struct rx_signal_info *rx_signal
    );

VOID
RecvProcessBroadMcastDataPkt(
    IN  PMP_ADAPTER  pAd, 
    IN  PMP_PORT      pPort,
    IN  PHEADER_802_11  pHeader,
    IN  PRXDSCR_BASE_STRUC pRxD,
    IN  SHORT               DataSize,
    IN  UCHAR           UserPriority
    );

VOID
RecvProcessDataPkt(
    IN  PMP_ADAPTER  pAd, 
    IN  PMP_PORT      pPort,
    IN  PHEADER_802_11  pHeader,
    IN  PRXDSCR_BASE_STRUC pRxD,
    IN  PUCHAR              pData,
    IN  SHORT               PacketSize,
    IN  SHORT               DataSize,
    IN  UCHAR               This80211HdrLen,
    IN  PUCHAR             pQoSField,
    IN  struct rx_signal_info *rx_signal
    );

VOID
RecvProcessMgntPkt(
    IN  PMP_ADAPTER  pAd, 
    IN  PMP_PORT      pPort,
    IN  PHEADER_802_11  pHeader,
    IN  PRXDSCR_BASE_STRUC pRxD,
    IN  PUCHAR              pData,
    IN  SHORT               PacketSize,
    IN  SHORT               DataSize,
    IN  struct rx_signal_info *rx_signal
    );

VOID
RecvProcessCtrlPkt(
    IN  PMP_ADAPTER  pAd, 
    IN  PMP_PORT      pPort,
    IN  PHEADER_802_11  pHeader,
    IN  PRXDSCR_BASE_STRUC pRxD,
    IN  SHORT               PacketSize
    );

VOID
RecvUpdateLastRxBeaconTimeSamp(
    IN  PMP_ADAPTER  pAd,
    IN  PMP_PORT      pPort,
    IN  PHEADER_802_11  pHeader,   
    IN  PRXDSCR_BASE_STRUC pRxD    
    );

VOID
RecvProcessPacket(
    IN  PMP_ADAPTER   pAd,
    IN  PMP_PORT      pPort,
    IN  PRXDSCR_BASE_STRUC  pRxD,
    IN  PHEADER_802_11  pHeader,
    IN  PUCHAR          pData,
    IN  USHORT              PacketSize,
    IN  UCHAR               QosPadding,
    IN  PUCHAR             pQoSField,
    IN  struct rx_signal_info *rx_signal
    );

VOID
RecvParseBuffer(
    IN  PMP_ADAPTER  pAd,
    IN  PRX_CONTEXT     pRxContext,
    IN  ULONG               TransferBufferLength
    );

VOID
RecvPreParseBuffer(
     IN  PMP_ADAPTER  pAd,
     IN  ULONG           nIndex
    );

#if 0
VOID
AP_RecvProcessDataPkt(
    IN  PMP_ADAPTER  pAd, 
    IN  PMP_PORT      pPort,
    IN  PHEADER_802_11  pHeader,
    IN  PUCHAR              pData,
    IN  SHORT               DataSize,
    IN  PRXINFO_STRUC   pRxInfo,
    IN  PRXWI_STRUC     pRxWI,
    IN  BOOLEAN           bHtVhtFrame
    );

VOID
AP_RecvProcessMgntPkt(
    IN  PMP_ADAPTER  pAd, 
    IN  PMP_PORT      pPort,
    IN  PHEADER_802_11  pHeader,
    IN  PRXINFO_STRUC   pRxInfo,
    IN  PRXWI_STRUC     pRxWI    
    );

VOID
AP_RecvProcessCtrlPkt(
    IN  PMP_ADAPTER  pAd, 
    IN  PMP_PORT      pPort,
    IN  PHEADER_802_11  pHeader,
    IN  PRXINFO_STRUC   pRxInfo,
    IN  PRXWI_STRUC     pRxWI    
    );

VOID
AP_RecvProcessPacket(
    IN  PMP_ADAPTER  pAd, 
    IN  PMP_PORT      pPort,
    IN  PHEADER_802_11  pHeader,
    IN  PRXINFO_STRUC   pRxInfo,
    IN  PUCHAR              pData,
    IN  USHORT              DataSize,
    IN  PRXWI_STRUC     pRxWI,
    IN  ULONG               TransferBufferLength
    );

VOID
AP_RecvParseBuffer(
    IN  PMP_ADAPTER  pAd,
    IN  PRX_CONTEXT     pRxContext,
    IN  ULONG               TransferBufferLength
    );

VOID
AP_RecvPreParseBuffer(
     IN  PMP_ADAPTER  pAd,
     IN  ULONG           nIndex
    );
#endif

VOID
RecordWaitToIndicateRxPktsNumbyPort(
    IN  PMP_ADAPTER  pAd,
    IN UCHAR      index
    );

 NDIS_STATUS    
 RecvApplyPacketFilter(
    IN  PMP_ADAPTER   pAd, 
    IN  PMP_PORT  pPort, 
    IN  PRXDSCR_BASE_STRUC  pRxD, 
    IN  PHEADER_802_11  pHeader
    );   

NDIS_STATUS 
RecvCheckExemptionList
    (IN PMP_ADAPTER   pAd,
    IN  PMP_PORT      pPort,
     IN PHEADER_802_11  pHeader,
     IN PRXWI_STRUC     pRxWI,
     IN PRXINFO_STRUC   pRxINFO,
     IN UCHAR           QOS_PAD
    );   

NDIS_STATUS 
RecvApplyPacketFilterforMGMTFrame(
    IN  PMP_ADAPTER   pAd, 
    IN  PMP_PORT      pPort,
    IN  PRXDSCR_BASE_STRUC  pRxD, 
    IN  PHEADER_802_11  pHeader
    );

NDIS_STATUS 
RecvCheckRxWi(
    IN  PMP_ADAPTER   pAd,
    IN  PMP_PORT      pPort,
    IN  PHEADER_802_11  pHeader,    
    IN  PRXWI_STRUC     pRxWI,  
    IN  PRXINFO_STRUC   pRxINFO
    );    

VOID    
RecvReportMicError(
    IN  PMP_ADAPTER   pAd, 
    IN  PCIPHER_KEY     pWpaKey
    );

VOID 
RecvGetMacContext(
    IN  PMP_ADAPTER  pAd,
    IN  PHEADER_802_11  pHeader,
    OUT PMP_PORT *ppPort
    );

#if DBG
VOID 
RecvPacketDebug(
    IN  PMP_PORT      pPort,
    IN  PUCHAR  pData
    );
#endif
    
#endif
