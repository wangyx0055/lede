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
    MgntPkt.h

    Abstract:
    Miniport generic portion header file

    Revision History:
    Who         When          What
    --------    ----------    ----------------------------------------------
    */

#ifndef __MLMEMGNTPKT_H__
#define __MLMEMGNTPKT_H__

typedef struct _RTMP_ADAPTER MP_ADAPTER, *PMP_ADAPTER;
typedef struct _ACM_WME_TSPEC ACM_WME_TSPEC, *PACM_WME_TSPEC;
typedef struct  _HEADER_802_11 HEADER_802_11, *PHEADER_802_11;
typedef struct PACKED _MAC_TABLE_ENTRY MAC_TABLE_ENTRY, *PMAC_TABLE_ENTRY;
typedef struct _MLME_QUEUE_ELEM MLME_QUEUE_ELEM, *PMLME_QUEUE_ELEM;

VOID
MgntPktConstructBeaconFrame(
    IN PMP_PORT  pPort,
    IN OUT PUCHAR        pOutBuffer,
    IN OUT ULONG    *FrameLen
    );

VOID 
MgntPktSendBeacon(
    IN PMP_PORT  pPort
    );

VOID
MgntPktConstructProbeReqFrame(
    IN PMP_PORT  pPort,
    IN OUT PUCHAR        pOutBuffer,
    IN OUT ULONG    *pFrameLen
    );

VOID 
MgntPktSendProbeRequest(
    IN PMP_PORT pPort
    );

VOID
MgntPktConstructAcmCmdAddTSReqFrame(
    IN PMP_PORT  pPort,
    IN  UINT16          DialogToken,
    IN  PACM_WME_TSPEC  pTSpec,    
    IN OUT PUCHAR        pOutBuffer,
    IN OUT ULONG    *pFrameLen
    );

VOID
MgntPktConstructAcmCmdDelTSFrame(
    IN PMP_PORT  pPort,
    IN  UINT16          DialogToken,
    IN  PACM_WME_TSPEC  pTSpec,    
    IN OUT PUCHAR        pOutBuffer,
    IN OUT ULONG    *pLen
    );  

VOID
MgntPktConstructAPPeerAssocActionFrame(
    IN PMP_PORT  pPort,
    IN USHORT               CapabilityInfo,
    IN MAC_TABLE_ENTRY *pEntry,
    IN USHORT               StatusCode,
    IN  USHORT       Aid,
    IN ULONG           RalinkIe,
    IN ULONG           P2PSubelementLen,
    IN UCHAR             HTCapability_Len,
    IN BOOLEAN         indicateAssocComplete,
    IN UCHAR           errorSource,//DOT11_ASSOC_ERROR_SOURCE_OTHER;
    IN ULONG           errorStatus,    
    IN PDOT11_INCOMING_ASSOC_COMPLETION_PARAMETERS assocCompletePara,
    IN ULONG assocCompleteParaSize,
    IN PUCHAR        Addr2,
    IN BOOLEAN      bAssocReq,
    IN BOOLEAN      bReAssocReq,    
    IN MLME_QUEUE_ELEM *Elem,
    IN OUT PUCHAR        pOutBuffer,
    IN OUT ULONG    *pLen
    );    

VOID MgntPktConstructAPBeaconFrame(
    IN PMP_ADAPTER pAd,
    IN PMP_PORT  pPort
    );

VOID
MgntPktConstructAPProbRspFrame(
    IN PMP_ADAPTER pAd,
    IN PMP_PORT  pPort,
    IN MLME_QUEUE_ELEM *Elem,
    IN PUCHAR Addr2, 
    IN CHAR                 *Ssid,
    IN UCHAR                 SsidLen,
    IN OUT PUCHAR        pOutBuffer,
    IN OUT ULONG    *pLen    
    );   

VOID
MgntPktConstructAsscReqFrame(
    IN PMP_ADAPTER pAd,
    IN PMP_PORT  pPort,
    IN BOOLEAN      bReassoc,
    IN USHORT  CapabilityInfo, 
    IN PUCHAR   ApAddr,
    IN USHORT          ListenIntv,
    IN USHORT           VarIesOffset,
    IN OUT PUCHAR        pOutBuffer,
    IN OUT ULONG    *pLen        
    );  

ULONG MgntPktConstructIBSSBeaconFrame(
    IN PMP_ADAPTER pAd,
    IN PMP_PORT pPort
    );    

VOID
MgntPktConstructIBSSProbRspPktFrame(
    IN PMP_PORT  pPort,
    IN PUCHAR         Addr2,
    IN OUT PUCHAR        pOutBuffer,
    IN OUT ULONG    *pFrameLen
    );    
#endif