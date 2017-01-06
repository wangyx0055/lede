/*
 ***************************************************************************
 * Ralink Technology, Corp.
 * 5F, No.5, Tai-Yuan 1st Street
 * Jhubei City, Hsinchu County 30265
 * Taiwan, R.O.C.
 *
 * (c) Copyright 2011, Ralink Technology, Corp.
 *
 * All rights reserved. Ralink's source code is an unpublished work and the
 * use of a copyright notice does not imply otherwise. This source code
 * contains confidential trade secret material of Ralink Tech. Any attemp
 * or participation in deciphering, decoding, reverse engineering or in any
 * way altering the source code is stricitly prohibited, unless the prior
 * written consent of Ralink Technology, Inc. is obtained.
 ***************************************************************************

    Module Name:
        ms_p2p.h

    Abstract:
        Wi-Fi Direct implementation based on Microsoft Windows 8 WLAN OID 
        Specification
*/

#ifndef __P2PMS_H__
#define __P2PMS_H__

// Public action frame address3 
#define USE_P2P_SPEC                                    1

// Timeout definitions
#define WFD_COMPLETE_SEND_TIMEOUT                   10  // unit: 100 ms, value >= (WFD_CHANNEL_SWITCH_TIMEOUT+WFD_PEER_CHANNEL_ACTION_TIMEOUT)
#define WFD_BLOCKING_SCAN_TIMEOUT                   300//600    // unit: 100 ms
#define WFD_CHANNEL_SWITCH_TIMEOUT                  1   // unit: 100 ms
#define WFD_PEER_CHANNEL_ACTION_TIMEOUT             5   // unit: 100 ms

#define MULTI_CHANNEL_NEW_CONNECTION_TIMEOUT        150 // unit: 100ms --> 15 sec
#define MULTI_CHANNEL_STOP_CONNECTION_TIMEOUT       50  // unit: 100ms --> 5 sec

#ifdef WFD_NEW_PUBLIC_ACTION
#define WFD_PUBLIC_ACTION_FRAME_RETRY_COUNT_MAX 15 
#define WFD_WAIT_ACK_TIMEOUT                            100 // unit: ms --> 100 msec
#define WFD_WAIT_RSP_TIMEOUT                            100 // unit: ms --> 100 msec
#endif

//Win8 Wi-Fi Direct Indications

VOID 
P2pMsEnable(
    IN PMP_ADAPTER pAd,
    IN PMP_PORT     pPort);

VOID
P2pMsIndicateDiscoverComplete(
    IN  PMP_ADAPTER pAd,
    IN  PMP_PORT pPort);

VOID
P2pMsIndicateGONegoReqSendComplete(
    IN  PMP_ADAPTER pAd,
    IN  PMP_PORT pPort,
    IN  PUCHAR  pOutBuffer,
    IN  ULONG   OutBufferLen,
    IN  NDIS_STATUS NdisStaus);


VOID
P2pMsIndicateGONegoRspSendComplete(
    IN  PMP_ADAPTER pAd,
    IN  PMP_PORT pPort,
    IN  PUCHAR  pOutBuffer,
    IN  ULONG   OutBufferLen,
    IN  NDIS_STATUS NdisStaus);


VOID
P2pMsIndicateGONegoConfirmSendComplete(
    IN  PMP_ADAPTER pAd,
    IN  PMP_PORT pPort,
    IN  PUCHAR  pOutBuffer,
    IN  ULONG   OutBufferLen,
    IN  NDIS_STATUS NdisStaus);

VOID
P2pMsIndicateInviteReqSendComplete(
    IN  PMP_ADAPTER pAd,
    IN  PMP_PORT pPort,
    IN  PUCHAR  pOutBuffer,
    IN  ULONG   OutBufferLen,
    IN  NDIS_STATUS NdisStaus);

VOID
P2pMsIndicateInviteRspSendComplete(
    IN  PMP_ADAPTER pAd,
    IN  PMP_PORT pPort,
    IN  PUCHAR  pOutBuffer,
    IN  ULONG   OutBufferLen,
    IN  NDIS_STATUS NdisStaus);

VOID
P2pMsIndicateProvDiscoReqSendComplete(
    IN  PMP_ADAPTER pAd,
    IN  PMP_PORT pPort,
    IN  PUCHAR  pOutBuffer,
    IN  ULONG   OutBufferLen,
    IN  NDIS_STATUS NdisStaus);

VOID
P2pMsIndicateProvDiscoRspSendComplete(
    IN  PMP_ADAPTER pAd,
    IN  PMP_PORT pPort,
    IN  PUCHAR  pOutBuffer,
    IN  ULONG   OutBufferLen,
    IN  NDIS_STATUS NdisStaus);

VOID
P2pMsIndicateGONegoReqRecv(
    IN  PMP_ADAPTER pAd,
    IN  PMP_PORT pPort,
    IN  PUCHAR  pInBuffer,
    IN  ULONG   InBufferLen);

VOID
P2pMsIndicateGONegoRspRecv(
    IN  PMP_ADAPTER pAd,
    IN  PMP_PORT pPort,
    IN  PUCHAR  pInBuffer,
    IN  ULONG   InBufferLen);

VOID
P2pMsIndicateGONegoConfirmRecv(
    IN  PMP_ADAPTER pAd,
    IN  PMP_PORT pPort,
    IN  PUCHAR  pInBuffer,
    IN  ULONG   InBufferLen);

VOID
P2pMsIndicateInviteReqRecv(
    IN  PMP_ADAPTER pAd,
    IN  PMP_PORT pPort,
    IN  PUCHAR  pInBuffer,
    IN  ULONG   InBufferLen);

VOID
P2pMsIndicateInviteRspRecv(
    IN  PMP_ADAPTER pAd,
    IN  PMP_PORT pPort,
    IN  PUCHAR  pInBuffer,
    IN  ULONG   InBufferLen);

VOID
P2pMsIndicateProvDiscoReqRecv(
    IN  PMP_ADAPTER pAd,
    IN  PMP_PORT pPort,
    IN  PUCHAR  pInBuffer,
    IN  ULONG   InBufferLen);

VOID
P2pMsIndicateProvDiscoRspRecv(
    IN  PMP_ADAPTER pAd,
    IN  PMP_PORT pPort,
    IN  PUCHAR  pInBuffer,
    IN  ULONG   InBufferLen);

VOID
P2pMsIndicatePublicActionFrameRecv(
    IN PMP_ADAPTER pAd,
    IN PMP_PORT     pPort,
    IN  PUCHAR  InformationBuffer,
    IN  ULONG   InformationBufferLength);

NDIS_STATUS
P2pMsEnumerateDeviceList(
    IN  PMP_ADAPTER pAd,
    IN  PMP_PORT pPort,
    IN  PVOID InformationBuffer,
    IN  ULONG OutputBufferLength,
    OUT PULONG BytesWritten,
    OUT PULONG BytesNeeded);

NDIS_STATUS
P2pMsSetDeviceCapability(
    IN  PMP_ADAPTER pAd,
    IN  PMP_PORT pPort,
    IN  PVOID   InformationBuffer,
    IN  ULONG   InformationBufferLength,
    OUT PULONG  BytesRead,
    OUT PULONG  BytesNeeded);

NDIS_STATUS
P2pMsSetGOCapability(
    IN  PMP_ADAPTER pAd,
    IN  PMP_PORT pPort,
    IN  PVOID   InformationBuffer,
    IN  ULONG   InformationBufferLength,
    OUT PULONG  BytesRead,
    OUT PULONG  BytesNeeded);

VOID P2pMsUpdateGOCapability(
    IN PMP_ADAPTER                pAd,
    IN PMP_PORT                   pPort,
    IN DOT11_WFD_GROUP_CAPABILITY   GroupCap);

#if 0
NDIS_STATUS
P2pMsSetListenChannel(
    IN  PMP_ADAPTER pAd,
    IN  PMP_PORT pPort,
    IN  PVOID   InformationBuffer,
    IN  ULONG   InformationBufferLength,
    OUT PULONG  BytesRead,
    OUT PULONG  BytesNeeded);
#endif

NDIS_STATUS
P2pMsSetDeviceDiscovery(
    IN  PMP_ADAPTER pAd,
    IN  PMP_PORT pPort,
    IN  PVOID   InformationBuffer,
    IN  ULONG   InformationBufferLength,
    OUT PULONG  BytesRead,
    OUT PULONG  BytesNeeded);

NDIS_STATUS
P2pMsStopDeviceDiscovery(
    IN  PMP_ADAPTER pAd,
    IN  PMP_PORT pPort,
    IN  PVOID   InformationBuffer,
    IN  ULONG   InformationBufferLength,
    OUT PULONG  BytesRead,
    OUT PULONG  BytesNeeded);

NDIS_STATUS
P2pMsSetAvailability (
    IN  PMP_ADAPTER pAd,
    IN  PMP_PORT pPort,
    IN  PVOID   InformationBuffer,
    IN  ULONG   InformationBufferLength,
    OUT PULONG  BytesRead,
    OUT PULONG  BytesNeeded);

NDIS_STATUS
P2pMsSetDeviceInfo(
    IN  PMP_ADAPTER pAd,
    IN  PMP_PORT pPort,
    IN  PVOID   InformationBuffer,
    IN  ULONG   InformationBufferLength,
    OUT PULONG  BytesRead,
    OUT PULONG  BytesNeeded);

NDIS_STATUS
P2pMsSetSecondaryDeviceTypeList(
    IN  PMP_ADAPTER pAd,
    IN  PMP_PORT pPort,
    IN  PVOID   InformationBuffer,
    IN  ULONG   InformationBufferLength,
    OUT PULONG  BytesRead,
    OUT PULONG  BytesNeeded);

NDIS_STATUS
P2pMsSetAdditionalIe(
    IN  PMP_ADAPTER pAd,
    IN  PMP_PORT pPort,
    IN  PVOID   InformationBuffer,
    IN  ULONG   InformationBufferLength,
    OUT PULONG  BytesRead,
    OUT PULONG  BytesNeeded);

NDIS_STATUS
P2pMsSetGroupStartParameters(
    IN  PMP_ADAPTER pAd,
    IN  PMP_PORT pPort,   
    IN  PVOID   InformationBuffer,
    IN  ULONG   InformationBufferLength,
    OUT PULONG  BytesRead,
    OUT PULONG  BytesNeeded);

NDIS_STATUS
P2pMsSetGroupJoinParameters(
    IN  PMP_ADAPTER pAd,
    IN  PMP_PORT pPort,   
    IN  PVOID   InformationBuffer,
    IN  ULONG   InformationBufferLength,
    OUT PULONG  BytesRead,
    OUT PULONG  BytesNeeded);

NDIS_STATUS
P2pMsSendGONegoReq(
    IN  PMP_ADAPTER pAd,
    IN  PMP_PORT pPort,
    IN  PVOID   InformationBuffer,
    IN  ULONG   InformationBufferLength,
    OUT PULONG  BytesRead,
    OUT PULONG  BytesNeeded);

NDIS_STATUS
P2pMsSendGONegoRsp(
    IN  PMP_ADAPTER pAd,
    IN  PMP_PORT pPort,
    IN  PVOID   InformationBuffer,
    IN  ULONG   InformationBufferLength,
    OUT PULONG  BytesRead,
    OUT PULONG  BytesNeeded);

NDIS_STATUS
P2pMsSendGONegoConfirm(
    IN  PMP_ADAPTER pAd,
    IN  PMP_PORT pPort,
    IN  PVOID   InformationBuffer,
    IN  ULONG   InformationBufferLength,
    OUT PULONG  BytesRead,
    OUT PULONG  BytesNeeded);


NDIS_STATUS
P2pMsSendInviteReq(
    IN  PMP_ADAPTER pAd,
    IN  PMP_PORT pPort,
    IN  PVOID   InformationBuffer,
    IN  ULONG   InformationBufferLength,
    OUT PULONG  BytesRead,
    OUT PULONG  BytesNeeded);


NDIS_STATUS
P2pMsSendInviteRsp(
    IN  PMP_ADAPTER pAd,
    IN  PMP_PORT pPort,
    IN  PVOID   InformationBuffer,
    IN  ULONG   InformationBufferLength,
    OUT PULONG  BytesRead,
    OUT PULONG  BytesNeeded);

NDIS_STATUS
P2pMsSendProvDiscoReq(
    IN  PMP_ADAPTER pAd,
    IN  PMP_PORT pPort,
    IN  PVOID   InformationBuffer,
    IN  ULONG   InformationBufferLength,
    OUT PULONG  BytesRead,
    OUT PULONG  BytesNeeded);

NDIS_STATUS
P2pMsSendProvDiscoRsp(
    IN  PMP_ADAPTER pAd,
    IN  PMP_PORT pPort,
    IN  PVOID   InformationBuffer,
    IN  ULONG   InformationBufferLength,
    OUT PULONG  BytesRead,
    OUT PULONG  BytesNeeded);

VOID
P2pMsMakeGONegoReq(
    IN PMP_ADAPTER        pAd,
    IN PMP_PORT           pPort,
    IN DOT11_MAC_ADDRESS    PeerAddr,
    IN DOT11_DIALOG_TOKEN   Token,
    IN DOT11_WFD_GO_INTENT  GOIntent,
    IN DOT11_MAC_ADDRESS    InterfaceAddr,
    IN DOT11_WFD_GROUP_CAPABILITY   GroupCapability,    
    IN PUCHAR               pOutBuffer,
    OUT PULONG              pTotalFrameLen);

VOID 
P2pMsMakeGONegoRsp(
    IN PMP_ADAPTER        pAd,
    IN PMP_PORT           pPort,
    IN DOT11_MAC_ADDRESS    PeerAddr,
    IN DOT11_DIALOG_TOKEN   Token,
    IN DOT11_WFD_STATUS_CODE Status,
    IN DOT11_WFD_GO_INTENT  GOIntent,
    IN DOT11_MAC_ADDRESS    InterfaceAddr,
    IN BOOLEAN              bUseGroupID,
    IN PDOT11_WFD_GROUP_ID  pGroupID,
    IN DOT11_WFD_GROUP_CAPABILITY   GroupCapability,
    IN PUCHAR               pOutBuffer,
    OUT PULONG              pTotalFrameLen);

VOID 
P2pMsMakeGONegoConfirm(
    IN PMP_ADAPTER        pAd,
    IN PMP_PORT           pPort,
    IN DOT11_MAC_ADDRESS    PeerAddr,
    IN DOT11_DIALOG_TOKEN   Token,
    IN DOT11_WFD_STATUS_CODE Status,
    IN BOOLEAN              bUseGroupID,
    IN PDOT11_WFD_GROUP_ID  pGroupID,
    IN DOT11_WFD_GROUP_CAPABILITY   GroupCapability,    
    IN PUCHAR               pOutBuffer,
    OUT PULONG              pTotalFrameLen);

VOID 
P2pMsMakeInviteReq(
    IN PMP_ADAPTER        pAd,
    IN PMP_PORT           pPort,
    IN DOT11_MAC_ADDRESS    PeerAddr,
    IN DOT11_DIALOG_TOKEN   Token,
    IN DOT11_WFD_INVITATION_FLAGS InviteFlag,
    IN BOOLEAN              bUseSpecificOperatingChannel,
    IN DOT11_WFD_CHANNEL    OperatingChannel,
    IN BOOLEAN              bUseGroupBSSID,
    IN DOT11_MAC_ADDRESS    GroupBSSID,
    IN PDOT11_WFD_GROUP_ID  pGroupID,
    IN PUCHAR               pOutBuffer,
    OUT PULONG              pTotalFrameLen);

VOID 
P2pMsMakeInviteRsp(
    IN PMP_ADAPTER        pAd,
    IN PMP_PORT           pPort,
    IN DOT11_MAC_ADDRESS    PeerAddr,
    IN DOT11_DIALOG_TOKEN   Token,
    IN DOT11_WFD_STATUS_CODE Status,
    IN BOOLEAN              bUseSpecificOperatingChannel,
    IN DOT11_WFD_CHANNEL    OperatingChannel,
    IN BOOLEAN              bUseGroupBSSID,
    IN DOT11_MAC_ADDRESS    GroupBSSID,
    IN PUCHAR               pOutBuffer,
    OUT PULONG              pTotalFrameLen);

VOID 
P2pMsMakeProvDiscoReq(
    IN PMP_ADAPTER        pAd,
    IN PMP_PORT           pPort,
    IN DOT11_MAC_ADDRESS    PeerAddr,
    IN DOT11_DIALOG_TOKEN   Token,
    IN BOOLEAN              bUseGroupID,
    IN PDOT11_WFD_GROUP_ID  pGroupID,
    IN DOT11_WFD_GROUP_CAPABILITY   GroupCapability,    
    IN PUCHAR               pOutBuffer,
    OUT PULONG              pTotalFrameLen);

VOID
P2pMsMakeProvDiscoRsp(
    IN PMP_ADAPTER        pAd,
    IN PMP_PORT           pPort,
    IN DOT11_MAC_ADDRESS    PeerAddr,
    IN DOT11_DIALOG_TOKEN   Token,
    IN PUCHAR               pOutBuffer,
    OUT PULONG              pTotalFrameLen);

#ifdef DBG
VOID
P2pMsIndicateFrameSendComplete(
    IN PMP_ADAPTER pAd,
    IN PMP_PORT pPort,
    IN PUCHAR pOutBuffer,
    IN ULONG OutBufferLen,
    IN NDIS_STATUS NdisStaus,
    IN ULONG Line);
#else
VOID
P2pMsIndicateFrameSendComplete(
    IN PMP_ADAPTER pAd,
    IN PMP_PORT pPort,
    IN PUCHAR pOutBuffer,
    IN ULONG OutBufferLen,
    IN NDIS_STATUS NdisStaus);
#endif

VOID
P2pMsBuildDeviceList(
    IN  PMP_ADAPTER pAd,
    IN PMP_PORT     pPort,
    IN  PUCHAR      pBuffer,
    IN  ULONG       BufferLength,
    OUT PULONG      BytesWritten,
    OUT PULONG      BytesNeeded,
    OUT PUCHAR      pTotalDeviceNumber);

VOID
P2pMsAckRequiredCheck(
    IN  PMP_ADAPTER       pAd,
    IN PMP_PORT     pPort,
    IN  PP2P_PUBLIC_FRAME   pFrame,
    OUT UCHAR       *TempPid);

VOID
P2pMsAllocateIeData(
    IN  PMP_ADAPTER pAd,
    IN  PUCHAR  pIeSrc,
    IN  ULONG   IeLength,
    OUT PUCHAR  *pIeMemory);

VOID
P2pMsFreeIeMemory(
    OUT PUCHAR  *pIeMemory,
    OUT PLONG   pIeLength);

VOID
P2PMSSyncListenChannelToPortChannel(
    IN PMP_ADAPTER  pAd,
    IN PMP_PORT     pPort
);

NDIS_STATUS
P2pMsSetDesiredGroupId(
    IN  PMP_ADAPTER pAd,
    IN  PMP_PORT pPort,
    IN  PVOID   InformationBuffer,
    IN  ULONG   InformationBufferLength,
    OUT PULONG  BytesRead,
    OUT PULONG  BytesNeeded);

NDIS_STATUS
P2pMsGetDialogToken(
    IN  PMP_ADAPTER pAd,
    IN  PMP_PORT pPort,
    IN  PVOID InformationBuffer,
    IN  ULONG InformationBufferLength,
    OUT PULONG BytesWritten,
    OUT PULONG BytesNeeded);

NDIS_STATUS
P2pMsStartGOReq(
    IN  PMP_ADAPTER pAd,
    IN  PMP_PORT pPort); 

NDIS_STATUS
P2pMsStartGo(
    IN PMP_ADAPTER pAd,
    IN  PMP_PORT pPort);

VOID 
P2pMsStopGo(
    IN PMP_ADAPTER pAd,
    IN  PMP_PORT pPort);

NDIS_STATUS
P2pMsConnectToGroupReq(
    IN  PMP_ADAPTER pAd,
    IN      PMP_PORT pPort);

NDIS_STATUS
    P2pMsDisconnectFromGroupReq(
    IN PMP_ADAPTER pAd,
    IN  PMP_PORT pPort,
    OUT PULONG  BytesRead,
    OUT PULONG  BytesNeeded);

NDIS_STATUS
P2pMsIncomingAssociationDecisionV2(
    IN  PMP_ADAPTER   pAd,
    IN  NDIS_PORT_NUMBER          PortNumber,        
    IN  PVOID           InformationBuffer,
    IN  ULONG           InformationBufferLength,
    IN  PULONG          pulBytesNeeded,
    IN  PULONG          pulBytesRead
    );

VOID    
P2pMsInfoResetRequest(  
    IN  PMP_ADAPTER pAd,
    IN  PMP_PORT pPort);

VOID
P2pMsDown(
    IN PMP_ADAPTER pAd,
    IN PMP_PORT   pPort);

VOID
P2pMsApAssocGoBuildAssociation(
    IN PMP_ADAPTER pAd,
    IN PMP_PORT   pPort,
    IN MAC_TABLE_ENTRY *pEntry,
    IN UCHAR         *RSN,
    IN UCHAR         *pRSNLen,
    IN UCHAR        *pP2pElement,
    IN UCHAR         P2pElementLen,
    IN UCHAR        *pWpsElement,
    IN UCHAR         WpsElementLen,
    IN HT_CAPABILITY_IE     *pHtCapability,
    IN UCHAR         HtCapabilityLen);

VOID 
P2pMsWPADone(
    IN PMP_ADAPTER pAd,
    IN PMP_PORT   pPort,
    IN PUCHAR       MacAddr);

VOID 
P2pMsMlmeCntLinkUp(
    IN PMP_ADAPTER pAd,
    IN PMP_PORT pPort);

VOID 
P2pMsMlmeCntLinkDown(
    IN PMP_ADAPTER    pAd,
    IN PMP_PORT       pPort,
    IN  BOOLEAN         IsReqFromAP);

VOID
P2pMsTimeoutAction(
    IN PMP_ADAPTER pAd,
    IN PMP_PORT     pPort);

VOID 
P2pMsRestoreChannel(
    IN PMP_ADAPTER pAd, 
    IN PMP_PORT       pPort,
    IN UCHAR    Channel);

VOID
P2pMsUpdateGroupLimitCap(
    IN PMP_ADAPTER pAd,
    IN PMP_PORT     pPort);

VOID 
P2pMsCopyP2PTabtoMacTab(
    IN PMP_ADAPTER pAd,
    IN UCHAR        P2pindex,
    IN UCHAR        Macindex,
    IN UCHAR        MyRule);

BOOLEAN
P2pMsDeviceAddrAndTypeMatch(
    IN  PMP_ADAPTER   pAd,
    IN  PMP_PORT      pPort,
    IN  PUCHAR          pDesiredAddr,
    IN  ULONG           NumDesiredTypes,    
    IN  PUCHAR          pDesiredTypes,
    IN  BOOLEAN         UseDevInfo,
    IN  PUCHAR          pMyAddr,
    IN  PUCHAR          pSsid,                  // SSID
    IN  UCHAR           SsidLen,                // Length of SSID               
    IN  PUCHAR          pMyPriType,
    IN  ULONG           NumMySecTypes,
    IN  PUCHAR          pMySecTypes,
    IN  UCHAR           MyRule,
    OUT PUCHAR          pMacIdx);

VOID P2pMsDefaultListenChannel(
    IN PMP_ADAPTER pAd,
    IN PMP_PORT     pPort);

VOID P2pMsSaveCommonSetChannelList(
    IN PMP_ADAPTER pAd,
    IN PMP_PORT     pPort,
    IN VOID *Msg, 
    IN ULONG MsgLen);

VOID P2pMsScanOpChannelDefault(
    IN PMP_ADAPTER pAd,
    IN PMP_PORT pPort);

#ifdef MULTI_CHANNEL_SUPPORT
VOID P2pMsStartActionFrame(
    IN PMP_ADAPTER pAd,
    IN PMP_PORT     pPort);

VOID P2pMsStartActionFrameExec(
    IN PMP_ADAPTER pAd,
    IN PMP_PORT     pPort);

VOID P2pMsChannelRestoreTimerCallback(
    IN PVOID SystemSpecific1, 
    IN PVOID FunctionContext, 
    IN PVOID SystemSpecific2, 
    IN PVOID SystemSpecific3);

VOID P2pMsChannelRestoreExec(
    IN PMP_ADAPTER pAd);
#endif /*MULTI_CHANNEL_SUPPORT*/

BOOLEAN
P2PMSCheckAnyWFDLinkUp(
    IN PMP_ADAPTER  pAd
);

VOID P2pMsScanDone(
    IN PMP_ADAPTER pAd,
    IN PMP_PORT   pPort);
#endif
