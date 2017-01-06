/*
 ***************************************************************************
 * Ralink Tech Inc.
 * 4F, No. 2 Technology 5th Rd.
 * Science-based Industrial Park
 * Hsin-chu, Taiwan, R.O.C.
 *
 * (c) Copyright 2009, Ralink Technology, Inc.
 *
 * All rights reserved. Ralink's source code is an unpublished work and the
 * use of a copyright notice does not imply otherwise. This source code
 * contains confidential trade secret material of Ralink Tech. Any attemp
 * or participation in deciphering, decoding, reverse engineering or in any
 * way altering the source code is stricitly prohibited, unless the prior
 * written consent of Ralink Technology, Inc. is obtained.
 ***************************************************************************

    Module Name:
    tdls.c

    Abstract:
    Handle TDLS state machine specified in IEEE 802.11z

    Revision History:
    Who         When            What
    --------    ----------      ----------------------------------------------
    Rita        2009-3-17       Tunneled Direct Link Setup
*/
#include "MtConfig.h"

#define DRIVER_FILE         0x01300000

UCHAR   TDLS_LLC_SNAP[] = {0xaa, 0xaa, 0x03, 0x00, 0x00, 0x00, 0x89, 0x0d};
UCHAR   TDLS_LLC_SNAP_WITH_CATEGORY[] = {0xaa, 0xaa, 0x03, 0x00, 0x00, 0x00, 0x89, 0x0d, PROTO_NAME_TDLS, CATEGORY_TDLS};
UCHAR   TDLS_ETHERTYPE[] = {0x89, 0x0d};

UCHAR   LinkIdIe            = IE_TDLS_LINK_ID;
UCHAR   QosCapIe            = IE_QOS_CAPABILITY;
UCHAR   FastTransIe         = IE_FAST_BSS_TRANSITION;
UCHAR   TimeoutIntIe        = IE_TIMEOUT_INTERVAL;

UCHAR   CipherSuiteTDLSWpa2PskTkip[] = {
        0x30,                   // RSN IE
        0x14,                   // Length   
        0x01, 0x00,             // Version
        0x00, 0x00, 0x00, 0x00, // no group cipher
        0x01, 0x00,             // number of pairwise
        0x00, 0x0f, 0xac, 0x02, // unicast, TKIP
        0x01, 0x00,             // number of authentication method
        0x00, 0x0f, 0xac, TDLS_AKM_SUITE,   // TDLS authentication
        0x02, 0x00,             // RSN capability, peer key enabled
        };

UCHAR   CipherSuiteTDLSWpa2PskAes[] = {
        0x30,                   // RSN IE
        0x14,                   // Length   
        0x01, 0x00,             // Version
        0x00, 0x00, 0x00, 0x00, // no group cipher
        0x01, 0x00,             // number of pairwise
        0x00, 0x0f, 0xac, 0x04, // unicast, AES
        0x01, 0x00,             // number of authentication method
        0x00, 0x0f, 0xac, TDLS_AKM_SUITE,   // TDLS authentication
        0x02, 0x00,             // RSN capability, peer key enabled
        };
UCHAR   CipherSuiteTDLSLen = sizeof(CipherSuiteTDLSWpa2PskAes)/ sizeof(UCHAR);

/*
    ==========================================================================
    Description:
        TDLS state machine init, including state transition and timer init
    Parameters:
        Sm - pointer to the TDLS state machine
        
    IRQL = PASSIVE_LEVEL

    ==========================================================================
 */
VOID TdlsStateMachineInit(
    IN PMP_ADAPTER pAd, 
    IN STATE_MACHINE *Sm, 
    OUT STATE_MACHINE_FUNC Trans[]) 
{
//  UCHAR   i;
    
    StateMachineInit(Sm, Trans, MAX_TDLS_STATE, MAX_TDLS_MSG, Drop, TDLS_IDLE, TDLS_MACHINE_BASE);
     
    StateMachineSetAction(Sm, TDLS_IDLE, MT2_MLME_TDLS_SETUP_REQ, TdlsSetupReqAction);
    StateMachineSetAction(Sm, TDLS_IDLE, MT2_PEER_TDLS_MSG, TdlsPeerMsgAction); 
    StateMachineSetAction(Sm, TDLS_IDLE, MT2_MLME_TDLS_TEAR_DOWN, TdlsTearDownAction);

}

/*
    ==========================================================================
    Description:
        
    IRQL = DISPATCH_LEVEL

    ==========================================================================
 */
VOID TdlsSetupReqAction(
    IN PMP_ADAPTER pAd, 
    IN MLME_QUEUE_ELEM *Elem) 
{
    PUCHAR          pOutBuffer = NULL;
    NDIS_STATUS     NStatus;
    ULONG           FrameLen = 0;
    HEADER_802_11   Header80211;
    PRT_802_11_TDLS pTDLS = NULL, pMlmeTdls = NULL;
    TDLS_ACTION_HEADER  TdlsActHdr;
    ULONG           tmp;
    ULONG           VhtAidIeLen, VhtCapIeLen;
    USHORT          reason;
    ULONG           Timeout = TDLS_TIMEOUT;
    BOOLEAN         TimerCancelled;
    UCHAR           Length;
    UCHAR           LengthQosPAD;
    PMP_PORT      pPort = pAd->PortList[Elem->PortNum];
    INT             LinkId;
    
    if(!MlmeTdlsReqSanity(pAd, Elem->Msg, Elem->MsgLen, &pMlmeTdls, &reason))
        return;
        
    DBGPRINT(RT_DEBUG_TRACE,("TDLS - TdlsSetupReqAction() \n"));

    if (pMlmeTdls->Status != TDLS_MODE_NONE)
    {
        DBGPRINT(RT_DEBUG_ERROR,("TDLS - TdlsSetupReqAction() not at the initial state\n"));
        return;
    }
    if ((pPort->PortCfg.AuthMode == Ndis802_11AuthModeWPA2) || (pPort->PortCfg.AuthMode == Ndis802_11AuthModeWPA2PSK))
    {
        if ((pPort->PortCfg.WepStatus != Ralink802_11Encryption2Enabled) && (pPort->PortCfg.WepStatus != Ralink802_11Encryption3Enabled))
        {
            DBGPRINT(RT_DEBUG_ERROR,("TDLS - TdlsSetupReqAction() didn't support NONE- TKIP/AES cipher suite \n"));
            return;
        }   
    }

    LinkId = TdlsSearchLinkId(pAd, pMlmeTdls->MacAddr);
    if ((LinkId == -1) || (LinkId == MAX_NUM_OF_TDLS_ENTRY) || MAC_ADDR_EQUAL(pMlmeTdls->MacAddr, ZERO_MAC_ADDR))
    {
        DBGPRINT(RT_DEBUG_ERROR,("TDLS - TdlsSetupReqAction() invalid entry\n"));
        return;
    }
    // Point to the current Link ID
    pTDLS = (PRT_802_11_TDLS)&pAd->StaCfg.TDLSEntry[LinkId];

    //==========================================================================
    // Start to Send TDLS Setup Request to the Responder
    //==========================================================================
    
    NStatus = MlmeAllocateMemory(pAd, &pOutBuffer);  //Get an unused nonpaged memory
    if (NStatus != NDIS_STATUS_SUCCESS) 
    {
        DBGPRINT(RT_DEBUG_ERROR,("TDLS - TdlsSetupReqAction() allocate memory failed \n"));
        return;
    }

    // Qos will be add in HardTx
    LengthQosPAD = 0;

    // 1. Make 802.11 header for Vista, where Addr1: AP, Addr2: SA, Addr3: DA
    // Sequence Number and WEP bit will be updated later
    PlatformZeroMemory(&Header80211, sizeof(HEADER_802_11));
    Header80211.FC.Type = BTYPE_DATA;   
    COPY_MAC_ADDR(Header80211.Addr1, pPort->PortCfg.Bssid); 
    COPY_MAC_ADDR(Header80211.Addr2, pAd->HwCfg.CurrentAddress);
    COPY_MAC_ADDR(Header80211.Addr3, pTDLS->MacAddr);       
    Header80211.FC.ToDs = 1; // through AP
    
    // 2. Prepare LLC_SNAP and TDLS Action header
    TdlsActHdr.Protocol = PROTO_NAME_TDLS;
    TdlsActHdr.Category = CATEGORY_TDLS;
    TdlsActHdr.Action   = ACTION_TDLS_SETUP_REQ;

    /* Count Dialog Token */
    pAd->StaCfg.TDLSDialogToken++;
    if (pAd->StaCfg.TDLSDialogToken == 0)
        pAd->StaCfg.TDLSDialogToken++;
    pTDLS->Token = pAd->StaCfg.TDLSDialogToken;

    // 3. Build basic frame first
    Length = 18; // Length of LinkIdentifier = BSSID + Initiator MAC + Responder MAC
    MakeOutgoingFrame(pOutBuffer,               &FrameLen,
                    sizeof(HEADER_802_11),      &Header80211,
                    LENGTH_802_1_H,             TDLS_LLC_SNAP,
                    sizeof(TDLS_ACTION_HEADER), &TdlsActHdr,
                    1,                          &pTDLS->Token,
                    1,                          &LinkIdIe,
                    1,                          &Length,
                    6,                          pPort->PortCfg.Bssid,
                    6,                          pAd->HwCfg.CurrentAddress,
                    6,                          pTDLS->MacAddr,
                    2,                          &pAd->StaActive.CapabilityInfo,
                    1,                          &SupRateIe,
                    1,                          &pPort->CommonCfg.SupRateLen,
                    pPort->CommonCfg.SupRateLen,  pPort->CommonCfg.SupRate,
                    END_OF_ARGS);

    // order:7
    //
    // Optional fields
    //
    
    // Extended supported rates (7.3.2.14)
    if (pPort->CommonCfg.ExtRateLen != 0)
    {
        MakeOutgoingFrame(pOutBuffer + FrameLen,    &tmp,
                          1,                        &ExtRateIe,
                          1,                        &pPort->CommonCfg.ExtRateLen,
                          pPort->CommonCfg.ExtRateLen,    pPort->CommonCfg.ExtRate,
                          END_OF_ARGS);
        FrameLen += tmp;
    }

    // order:10
    //  Extended Capabilities (7.3.2.27)
    if (pPort->CommonCfg.ExtCapIE.DoubleWord > 0)
    {
        Length = sizeof(EXT_CAP_ELEMT);
        MakeOutgoingFrame(pOutBuffer + FrameLen,        &tmp,
                      1,                                &ExtHtCapIe,
                      1,                                &Length,
                      sizeof(EXT_CAP_ELEMT),            &pPort->CommonCfg.ExtCapIE.DoubleWord,
                      END_OF_ARGS);
        FrameLen += tmp;
    }

    // order:11
    // QoS Capability (7.3.2.35)
    if ((pPort->CommonCfg.PhyMode >= PHY_11ABGN_MIXED) || (pPort->CommonCfg.bWmmCapable))
    {
        QBSS_STA_INFO_PARM QosInfo;

        PlatformZeroMemory(&QosInfo, sizeof(QBSS_STA_INFO_PARM));
        /*if (pPort->CommonCfg.bAPSDCapable && pPort->CommonCfg.ExtCapIE.field.UAPSD)
        {
            QosInfo.UAPSD_AC_BE = pPort->CommonCfg.bAPSDAC_BE;
            QosInfo.UAPSD_AC_BK = pPort->CommonCfg.bAPSDAC_BK;
            QosInfo.UAPSD_AC_VI = pPort->CommonCfg.bAPSDAC_VI;
            QosInfo.UAPSD_AC_VO = pPort->CommonCfg.bAPSDAC_VO;
            QosInfo.MaxSPLength = pPort->CommonCfg.MaxSPLength;
        }*/
        Length = 1;
        MakeOutgoingFrame(pOutBuffer + FrameLen,        &tmp,
                          1,                            &QosCapIe,
                          1,                            &Length,
                          1,                            &QosInfo,
                          END_OF_ARGS);
        FrameLen += tmp;
        
    }


    //  802.11 N Mode
    if (pPort->CommonCfg.PhyMode >= PHY_11ABGN_MIXED)
    {
        // HT capability IE (7.3.2.61)
        HT_CAPABILITY_IE HtCap;
        PMAC_TABLE_ENTRY pBssidMacTabEntry = MlmeSyncMacTabMatchedRoleType(pPort, ROLE_WLANIDX_BSSID);

        if(pBssidMacTabEntry == NULL)
        {
            DBGPRINT(RT_DEBUG_TRACE,("%s(), Line:%d, Mac Entry is NULL\n", __FUNCTION__, __LINE__));
            return;
        }
        
        PlatformMoveMemory(&HtCap, &pPort->CommonCfg.HtCapability, sizeof(HT_CAPABILITY_IE));
        
        // Use base channel BW
        if ((READ_PHY_CFG_BW(pAd, &pBssidMacTabEntry->TxPhyCfg) == BW_40) ||
             (READ_PHY_CFG_BW(pAd, &pBssidMacTabEntry->TxPhyCfg) == BW_80))
        {
            HtCap.HtCapInfo.ChannelWidth = BW_40;
        }
        else
        {
            HtCap.HtCapInfo.ChannelWidth = BW_20;
        }
        
        // Off RDG
        HtCap.ExtHtCapInfo.RDGSupport = 0;

        // Can we select MpduDensity and MaxRAampduFactor?
        
        // order:15     
        Length = sizeof(HT_CAPABILITY_IE);
        MakeOutgoingFrame(pOutBuffer + FrameLen,            &tmp,
                            1,                              &HtCapIe,
                            1,                              &Length,
                            sizeof(HT_CAPABILITY_IE),       &HtCap,
                            END_OF_ARGS);
        FrameLen += tmp;

        // order:16
        // 20/40 BSS Coexistence (7.3.2.60)
        if (OPSTATUS_TEST_FLAG(pAd, fOP_STATUS_SCAN_2040) && OPSTATUS_TEST_FLAG(pPort, fOP_STATUS_SCAN_2040))
        {
            Length = 1;
            MakeOutgoingFrame(pOutBuffer + FrameLen,    &tmp,
                          1,                            &BssCoexistIe,
                          1,                            &Length,
                          1,                            &pPort->CommonCfg.BSSCoexist2040.word,
                          END_OF_ARGS);
            FrameLen += tmp;
        }
    }

    
    // TPK Handshake if RSNA Enabled
    // Pack TPK Message 1 here! 
    if (((pPort->PortCfg.AuthMode == Ndis802_11AuthModeWPA2) || (pPort->PortCfg.AuthMode == Ndis802_11AuthModeWPA2PSK)) &&
        ((pPort->PortCfg.WepStatus == Ralink802_11Encryption2Enabled) || (pPort->PortCfg.WepStatus == Ralink802_11Encryption3Enabled)))
    {       
        UCHAR           CipherTmp[64] = {0};
        UCHAR           CipherTmpLen = 0;
        FT_IE           FtIe;
        ULONG           KeyLifetime = TDLS_KEY_TIMEOUT; // sec
        
        // RSNIE (7.3.2.25)
        CipherTmpLen = CipherSuiteTDLSLen;  
        if (pPort->PortCfg.WepStatus == Ralink802_11Encryption2Enabled)
            PlatformMoveMemory(CipherTmp, CipherSuiteTDLSWpa2PskTkip, CipherTmpLen);
        else
            PlatformMoveMemory(CipherTmp, CipherSuiteTDLSWpa2PskAes, CipherTmpLen);
        
        // order:9      
        // Insert RSN_IE to outgoing frame
        MakeOutgoingFrame(pOutBuffer + FrameLen,    &tmp,
                CipherTmpLen,                       &CipherTmp,
                END_OF_ARGS);
        FrameLen += tmp;
    
            
        // FTIE (7.3.2.48)
        PlatformZeroMemory(&FtIe, sizeof(FtIe));
        Length =  sizeof(FtIe) - 256;

        // generate SNonce
        GenRandom(pAd, FtIe.SNonce);
        PlatformMoveMemory(pTDLS->SNonce, FtIe.SNonce, 32);

#if 0   // FT_InsertXXX shall be defined to Dot11R

        // FTIE (7.3.2.48)
        PlatformZeroMemory(&FtIe, sizeof(FtIe));
        Length =  sizeof(FtIe) - 256;

        // generate SNonce
        GenRandom(pAd, FtIe.SNonce);
        PlatformMoveMemory(pTDLS->SNonce, FtIe.SNonce, 32);

        FT_InsertFTIE(
                pAd, 
                pOutBuffer + FrameLen, 
                &FrameLen, 
                Length, 
                FtIe.MICControl, 
                FtIe.MIC, 
                FtIe.ANonce, 
                FtIe.SNonce);

        // Timeout Interval (7.3.2.49)
        FT_InsertTimeoutIntervalIE(
                pAd, 
                pOutBuffer + FrameLen, 
                &FrameLen, 
                2, /* key lifetime interval */
                KeyLifetime);

        pTDLS->KeyLifetime = KeyLifetime;
#else
    {
        UCHAR   ValueIE;
        UINT16  MICCtrBuf;
        UINT32  TimeoutValueBuf;
        UCHAR   TimeoutType;
        
        // FTIE (7.3.2.48)
        ValueIE = IE_FAST_BSS_TRANSITION;
        Length =  sizeof(FtIe) - 256;

        // generate SNonce, and others are zeros
        PlatformZeroMemory(&FtIe, sizeof(FtIe));
        GenRandom(pAd, FtIe.SNonce);
        PlatformMoveMemory(pTDLS->SNonce, FtIe.SNonce, 32);
        MICCtrBuf = cpu2le16(FtIe.MICControl.word);
        
        // order:12
        MakeOutgoingFrame(  pOutBuffer + FrameLen,  &tmp,
                            1,                      &ValueIE,
                            1,                      &Length,
                            2,                      (PUCHAR)&MICCtrBuf,
                            16,                     (PUCHAR)&FtIe.MIC,
                            32,                     (PUCHAR)FtIe.ANonce,
                            32,                     (PUCHAR)FtIe.SNonce,
                            END_OF_ARGS);
        FrameLen += tmp;


        // Timeout Interval (7.3.2.49)
        ValueIE = IE_TIMEOUT_INTERVAL;
        Length = 5;
        TimeoutType = 2;    // type: key lifetime interval
        TimeoutValueBuf = cpu2le32(KeyLifetime);

        // order:13
        MakeOutgoingFrame(  pOutBuffer + FrameLen,  &tmp,
                            1,                      &ValueIE,
                            1,                      &Length,
                            1,                      (PUCHAR)&TimeoutType,
                            4,                      (PUCHAR)&TimeoutValueBuf,
                            END_OF_ARGS);
        FrameLen += tmp;
        
    }
#endif
    }

    if(pPort->Channel >14)
    {
        if(pPort->CommonCfg.PhyMode == PHY_11VHT)
        {       
            //
            // order: 18, AID_IE
            //
            // AID: bit_15:14 suppose to be b'11
            VhtAidIeLen = SIZE_OF_VHT_AID_IE;
            MakeOutgoingFrame(pOutBuffer + FrameLen, &tmp,
                                1,                   &VhtAidIe,
                                1,                   &VhtAidIeLen,
                                SIZE_OF_VHT_AID_IE,  &pAd->StaActive.Aid, 
                                END_OF_ARGS);

            
            
            FrameLen += tmp;
            //
            // order: 19, VHT_CAPABILITIES_IE
            //
            VhtCapIeLen = SIZE_OF_VHT_CAP_IE;
            MakeOutgoingFrame(pOutBuffer+FrameLen,   &tmp,
                                1,                   &VhtCapIe,
                                1,                   &VhtCapIeLen,
                                SIZE_OF_VHT_CAP_IE,  &pPort->CommonCfg.DesiredVhtPhy.VhtCapability,
                                END_OF_ARGS);
            FrameLen += tmp;
        }
    }

    // TODO: Currently, we don't suppot TDLS channel switching feature
    if (0)
    {
        // Supported Channels (7.3.2.19)

        // Supported Regulatory Classes(7.3.2.51)
    }

    // ==>> Set sendout timer
    PlatformCancelTimer(&pTDLS->Timer, &TimerCancelled);
    PlatformSetTimer(pPort,&pTDLS->Timer, Timeout);
    // ==>> State Change
    pTDLS->Status = TDLS_MODE_WAIT_RESULT;

    // Send a packet via HardTransmit
    TdlsSendPacket(pAd, pOutBuffer, FrameLen);
    MlmeFreeMemory(pAd, pOutBuffer);
}


VOID TdlsPeerMsgAction(
    IN PMP_ADAPTER pAd, 
    IN MLME_QUEUE_ELEM *Elem) 
{
    UCHAR   Action;

    // Message contains 802.11 header (24 bytes), LLC_SNAP (8 bytes), TDLS Action header(3 bytes) and Payload (variable)
    if(Elem->MsgLen < (LENGTH_802_11 + LENGTH_802_1_H + sizeof(TDLS_ACTION_HEADER))) 
    {
        DBGPRINT(RT_DEBUG_ERROR, ("TdlsPeerMsgAction: Received a message with the invalid length!!!\n"));
        return; 
    }
    
    Action = Elem->Msg[LENGTH_802_11 + LENGTH_802_1_H + 2];

    switch(Action)
    {
        case ACTION_TDLS_SETUP_REQ:
            DBGPRINT(RT_DEBUG_TRACE,("TDLS - TdlsPeerMsgAction() Action = ACTION_TDLS_SETUP_REQ \n"));
            TdlsPeerSetupReqAction(pAd,Elem);
            break;
        case ACTION_TDLS_SETUP_RSP:
            DBGPRINT(RT_DEBUG_TRACE,("TDLS - TdlsPeerMsgAction() Action = ACTION_TDLS_SETUP_RSP \n"));          
            TdlsPeerSetupRspAction(pAd,Elem);
            break;
        case ACTION_TDLS_SETUP_CONF:
            DBGPRINT(RT_DEBUG_TRACE,("TDLS - TdlsPeerMsgAction() Action = ACTION_TDLS_SETUP_CONF \n"));         
            TdlsPeerSetupConfAction(pAd,Elem);
            break;
        case ACTION_TDLS_TEARDOWN:
            DBGPRINT(RT_DEBUG_TRACE,("TDLS - TdlsPeerMsgAction() Action = ACTION_TDLS_TEARDOWN \n"));           
            TdlsPeerTearDownAction(pAd,Elem);
            break;
        default:
            break;
    
    }

}

/*
    ==========================================================================
    Description:
        
    IRQL = DISPATCH_LEVEL

    ==========================================================================
 */
VOID TdlsPeerSetupReqAction(
    IN PMP_ADAPTER pAd, 
    IN MLME_QUEUE_ELEM *Elem) 
{
    PUCHAR          pOutBuffer = NULL;
    NDIS_STATUS     NStatus;
    ULONG           FrameLen = 0;
    USHORT          StatusCode = MLME_SUCCESS;
    ULONG           tmp;
    ULONG           VhtAidIeLen,VhtCapIeLen;
    SHORT           i;
    ULONG           Timeout = TDLS_TIMEOUT;
    BOOLEAN         TimerCancelled;
    PRT_802_11_TDLS pTDLS = NULL;
    UCHAR           Token;
    UCHAR           PeerAddr[MAC_ADDR_LEN], SA[MAC_ADDR_LEN];
    USHORT          CapabilityInfo;
    UCHAR           SupRate[MAX_LEN_OF_SUPPORTED_RATES], ExtRate[MAX_LEN_OF_SUPPORTED_RATES];
    UCHAR           SupRateLen, ExtRateLen, HtCapLen, RsnLen, FTLen, TILen;
    UCHAR           RsnIe[64], FTIe[128], TIIe[7];
    UCHAR           QosCapability;
    HT_CAPABILITY_IE    HtCap;
    EXT_CAP_ELEMT       ExtCap;
    HEADER_802_11       Header80211;
    TDLS_ACTION_HEADER  TdlsActHdr;
    UCHAR               Length;
    UCHAR               LinkIdentifier[20];
    BOOLEAN             bWmmCapable;
    INT                 LinkId =0xff;
    UCHAR               LengthQosPAD;
    PMP_PORT          pPort = pAd->PortList[Elem->PortNum];
    MTK_TIMER_STRUCT backupTimer;
    
    // Not TDLS Capable, ignore it
    if(!TDLS_ON(pAd))
        return;
    
    // Not BSS mode, ignore it
    if (!INFRA_ON(pPort))
        return;

    // Init all kinds of fields within the packet
    PlatformZeroMemory(&CapabilityInfo, sizeof(CapabilityInfo));
    PlatformZeroMemory(&HtCap, sizeof(HtCap));
    PlatformZeroMemory(&ExtCap, sizeof(ExtCap));
    PlatformZeroMemory(RsnIe, sizeof(RsnIe));
    PlatformZeroMemory(FTIe, sizeof(FTIe));
    PlatformZeroMemory(TIIe, sizeof(TIIe));
    PlatformZeroMemory(PeerAddr, MAC_ADDR_LEN); 
    PlatformZeroMemory(SA, MAC_ADDR_LEN);


    if (!PeerTdlsSetupReqSanity(
                            pAd,
                            pPort,
                            Elem->Msg, 
                            Elem->MsgLen,
                            &Token,
                            SA,
                            PeerAddr,
                            &CapabilityInfo,
                            &SupRateLen,    
                            SupRate,
                            &ExtRateLen,
                            ExtRate,
                            &bWmmCapable,                           
                            &QosCapability,
                            &HtCapLen,
                            &HtCap,
                            &ExtCap,
                            &RsnLen,    
                            RsnIe,
                            &FTLen,
                            FTIe,
                            &TILen,
                            TIIe))
    {
        StatusCode = MLME_REQUEST_DECLINED;
    }


    DBGPRINT(RT_DEBUG_TRACE,("TDLS - TdlsPeerSetupReqAction() received a request from %02x:%02x:%02x:%02x:%02x:%02x\n", PeerAddr[0], PeerAddr[1], PeerAddr[2], PeerAddr[3], PeerAddr[4], PeerAddr[5]));

    if (MAC_ADDR_EQUAL(PeerAddr, ZERO_MAC_ADDR) || MAC_ADDR_EQUAL(PeerAddr, pPort->PortCfg.Bssid))
    {
        DBGPRINT(RT_DEBUG_ERROR,("TDLS - TdlsPeerSetupReqAction() unknown or invalid address\n"));
        return;
    }

    /*The TDLS setup request frame is received after sending a TDLS Setup Request frame and before
     receiving the corresponding TDLS Setup Response frame, and the source address of the received
     TDLS Setup Request frame is lower than its own MAC address. In this case, the TDLS responder
     STA shall terminate the TDLS setup it initiated. The TDLS responder STA shall send a response
     according to item 1 or 2 above in this case.*/
    if (((LinkId = TdlsSearchLinkId(pAd, PeerAddr)) > 0) &&
        (pAd->StaCfg.TDLSEntry[LinkId].bInitiator == TRUE) &&
        (pAd->StaCfg.TDLSEntry[LinkId].Status == TDLS_MODE_WAIT_RESULT))
    {
        //My MacAddr is lower. Ignore this Tdls Setup Request frame
        if (MtkCompareMemory(PeerAddr, pAd->HwCfg.CurrentAddress, MAC_ADDRESS_LENGTH) == 1)
        { 
            DBGPRINT(RT_DEBUG_TRACE,("TDLS - TdlsPeerSetupReqAction(), My MacAddr is lower, drop packet\n"));
            return;
        }
        //My MacAddr memory is larger. Accept this Tdls Setup Request frame
        else if (MtkCompareMemory(PeerAddr, pAd->HwCfg.CurrentAddress, MAC_ADDRESS_LENGTH) == 2)
        {
            DBGPRINT(RT_DEBUG_TRACE,("TDLS - TdlsPeerSetupReqAction(), My MacAddr is larger, change to Responder Client.\n"));
            // Let's terminate the last procedure.
            PlatformCancelTimer(&pPort->StaCfg.TDLSEntry[LinkId].Timer, &TimerCancelled);
            
            PlatformMoveMemory(&backupTimer, &pAd->StaCfg.TDLSEntry[LinkId], sizeof(MTK_TIMER_STRUCT));
            PlatformZeroMemory(&pAd->StaCfg.TDLSEntry[LinkId], sizeof(RT_802_11_DLS_UI));
            PlatformMoveMemory(&pAd->StaCfg.TDLSEntry[LinkId], &backupTimer, sizeof(MTK_TIMER_STRUCT));
            
            
        }
    }

    
    //
    // Find table to update parameters.
    //
    if (StatusCode == MLME_SUCCESS)
    {
        for (i=MAX_NUM_OF_TDLS_ENTRY-1; i>=0; i--)
        {
            if (pAd->StaCfg.TDLSEntry[i].Valid && MAC_ADDR_EQUAL(PeerAddr, pAd->StaCfg.TDLSEntry[i].MacAddr))
            {
                PlatformCancelTimer(&pPort->StaCfg.TDLSEntry[i].Timer, &TimerCancelled);
                pTDLS = &pAd->StaCfg.TDLSEntry[i];
                DBGPRINT(RT_DEBUG_TRACE,("TDLS - TdlsPeerSetupReqAction() find the same entry \n"));
                break;
            }
        }

        // Can not find in table, create a new one
        if (i < 0)
        {
            for (i=(MAX_NUM_OF_TDLS_ENTRY-1); i>=0; i--)
            {
                if (!pAd->StaCfg.TDLSEntry[i].Valid)
                {
                    PlatformCancelTimer(&pPort->StaCfg.TDLSEntry[i].Timer, &TimerCancelled);
                    pAd->StaCfg.TDLSEntry[i].Valid = TRUE;
                    PlatformMoveMemory(pAd->StaCfg.TDLSEntry[i].MacAddr, PeerAddr, MAC_ADDR_LEN);
                    pTDLS = &pAd->StaCfg.TDLSEntry[i];
                    DBGPRINT(RT_DEBUG_TRACE,("TDLS - TdlsPeerSetupReqAction() create a new entry \n"));
                    
                    break;
                }
            }
        }


        if (i < 0)
        {
            // Table full !!!!!
            StatusCode = MLME_REQUEST_DECLINED;
            DBGPRINT(RT_DEBUG_ERROR,("TDLS - TdlsPeerSetupReqAction() TDLSEntry table full(only can support %d TDLS session) \n", MAX_NUM_OF_TDLS_ENTRY));
        }
        else if (pTDLS)
        {   
            //
            // Process TPK Handshake Message 1 here!
            //

            // Consider that AP is in the mixed mode with WPA1+WPA2 or TKIP+AES.
            if ((pPort->PortCfg.AuthMode >= Ndis802_11AuthModeWPA) && (pPort->PortCfg.WepStatus != Ralink802_11WEPDisabled))
            {
                USHORT Result;
                
                // RSNIE (7.3.2.25), FTIE (7.3.2.48), Timeout Interval (7.3.2.49)
                Result = TdlsTPKMsg1Process(pAd, pPort, pTDLS, RsnIe, RsnLen, FTIe, FTLen, TIIe, TILen);
                if (Result != MLME_SUCCESS)
                {
                    DBGPRINT(RT_DEBUG_ERROR,("TDLS - TdlsTPKMsg1Process() Failed, reason=%d \n", Result));
                    if (Result == MLME_UNSPECIFY_FAIL)
                    {
                        return;
                    }
                    else
                    {
                        StatusCode = Result;
                        goto send_out;
                    }
                }

                // Copy SNonce, Key lifetime
                pTDLS->KeyLifetime = le2cpu32(*((PULONG) (TIIe + 3)));
                PlatformMoveMemory(pTDLS->SNonce, &FTIe[52], 32);
            }


            //
            // Update peer settings.
            //
            pTDLS->Token = Token;
            pTDLS->bInitiator = FALSE;
            pTDLS->CapabilityInfo = CapabilityInfo & SUPPORTED_CAPABILITY_INFO;

            // Copy Initiator's supported rate
            PlatformZeroMemory(pTDLS->SupRate, sizeof(pTDLS->SupRate));
            PlatformZeroMemory(pTDLS->ExtRate, sizeof(pTDLS->ExtRate));
            pTDLS->SupRateLen = SupRateLen;
            PlatformMoveMemory(pTDLS->SupRate, SupRate, SupRateLen);
            pTDLS->ExtRateLen = ExtRateLen;
            PlatformMoveMemory(pTDLS->ExtRate, ExtRate, ExtRateLen);

            // Copy Initiator's ht rates
            if ((HtCapLen > 0) && (pPort->CommonCfg.PhyMode >= PHY_11ABGN_MIXED))
            {
                PlatformZeroMemory(&pTDLS->HtCapability, SIZE_HT_CAP_IE);
                pTDLS->HtCapabilityLen = SIZE_HT_CAP_IE;
                PlatformMoveMemory(&pTDLS->HtCapability, &HtCap, pTDLS->HtCapabilityLen);
            }
            else
            {
                pTDLS->HtCapabilityLen = 0;
                PlatformZeroMemory(&pTDLS->HtCapability, SIZE_HT_CAP_IE);
            }

            // Copy extended capability
            PlatformMoveMemory(&pTDLS->ExtCap, &ExtCap, sizeof(EXT_CAP_ELEMT));

            // Copy QOS related information
            pTDLS->QosCapability = QosCapability;
            pTDLS->bWmmCapable = bWmmCapable;
            
        }
    }

send_out:   
    //==========================================================================
    // Start to Send TDLS Setup Response to the Initiator
    //==========================================================================

    NStatus = MlmeAllocateMemory(pAd, &pOutBuffer);  //Get an unused nonpaged memory
    if (NStatus != NDIS_STATUS_SUCCESS) 
    {
        DBGPRINT(RT_DEBUG_ERROR,("TDLS - TdlsPeerSetupReqAction() allocate memory failed \n"));
        return;
    }
    
    // Qos will be add in HardTx
    LengthQosPAD = 0;

    // 1. Make 802.11 header for Vista, where Addr1: AP, Addr2: SA, Addr3: DA
    // Sequence Number and WEP bit will be updated later
    PlatformZeroMemory(&Header80211, sizeof(HEADER_802_11));
    Header80211.FC.Type = BTYPE_DATA;   
    COPY_MAC_ADDR(Header80211.Addr1, pPort->PortCfg.Bssid); 
    COPY_MAC_ADDR(Header80211.Addr2, pAd->HwCfg.CurrentAddress);
    COPY_MAC_ADDR(Header80211.Addr3, PeerAddr);
    Header80211.FC.ToDs = 1; // through AP

    // 2. Prepare LLC_SNAP and TDLS Action header
    TdlsActHdr.Protocol = PROTO_NAME_TDLS;
    TdlsActHdr.Category = CATEGORY_TDLS;
    TdlsActHdr.Action   = ACTION_TDLS_SETUP_RSP;

    // Build basic frame first
    if (StatusCode == MLME_SUCCESS && pTDLS)
    {
        // Construct LinkIdentifier (IE + Length + BSSID + Initiator MAC + Responder MAC)
        PlatformZeroMemory(LinkIdentifier, 20);
        LinkIdentifier[0] = IE_TDLS_LINK_ID;
        LinkIdentifier[1] = 18;
        PlatformMoveMemory(&LinkIdentifier[2], pPort->PortCfg.Bssid, 6);
        PlatformMoveMemory(&LinkIdentifier[8], pTDLS->MacAddr, 6);
        PlatformMoveMemory(&LinkIdentifier[14], pAd->HwCfg.CurrentAddress, 6);

        MakeOutgoingFrame(pOutBuffer,               &FrameLen,
                        sizeof(HEADER_802_11),      &Header80211,
                        LENGTH_802_1_H,             TDLS_LLC_SNAP,
                        sizeof(TDLS_ACTION_HEADER), &TdlsActHdr,
                        2,                          &StatusCode,
                        1,                          &pTDLS->Token,
                        sizeof(LinkIdentifier),     LinkIdentifier,                     
                        2,                          &pAd->StaActive.CapabilityInfo,
                        1,                          &SupRateIe,
                        1,                          &pPort->CommonCfg.SupRateLen,
                        pPort->CommonCfg.SupRateLen,  pPort->CommonCfg.SupRate,
                        END_OF_ARGS);

        // order:8
        //
        // Optional fields
        //
        
        // Extended supported rates (7.3.2.14)
        if (pPort->CommonCfg.ExtRateLen != 0)
        {
            MakeOutgoingFrame(pOutBuffer + FrameLen,    &tmp,
                              1,                        &ExtRateIe,
                              1,                        &pPort->CommonCfg.ExtRateLen,
                              pPort->CommonCfg.ExtRateLen, pPort->CommonCfg.ExtRate,
                              END_OF_ARGS);
            FrameLen += tmp;
        }

        // order:12
        // QoS Capability (7.3.2.35)
        if ((pPort->CommonCfg.PhyMode >= PHY_11ABGN_MIXED) || (pPort->CommonCfg.bWmmCapable))
        {
            QBSS_STA_INFO_PARM QosInfo;

            PlatformZeroMemory(&QosInfo, sizeof(QBSS_STA_INFO_PARM));
            /*if (pPort->CommonCfg.bAPSDCapable && pPort->CommonCfg.ExtCapIE.field.UAPSD)
            {
                QosInfo.UAPSD_AC_BE = pPort->CommonCfg.bAPSDAC_BE;
                QosInfo.UAPSD_AC_BK = pPort->CommonCfg.bAPSDAC_BK;
                QosInfo.UAPSD_AC_VI = pPort->CommonCfg.bAPSDAC_VI;
                QosInfo.UAPSD_AC_VO = pPort->CommonCfg.bAPSDAC_VO;
                QosInfo.MaxSPLength = pPort->CommonCfg.MaxSPLength;
            }*/
                        
            Length = 1;
            MakeOutgoingFrame(pOutBuffer + FrameLen,        &tmp,
                              1,                            &QosCapIe,
                              1,                            &Length,
                              1,                            &QosInfo,
                              END_OF_ARGS);         
            FrameLen += tmp;
        }


        //  802.11 N Mode
        if (pPort->CommonCfg.PhyMode >= PHY_11ABGN_MIXED)
        {
            // HT capability IE (7.3.2.61)
            HT_CAPABILITY_IE    SendOutHtCap;
            PMAC_TABLE_ENTRY pBssidMacTabEntry = MlmeSyncMacTabMatchedRoleType(pPort, ROLE_WLANIDX_BSSID);

            if(pBssidMacTabEntry == NULL)
            {
                DBGPRINT(RT_DEBUG_TRACE,("%s(), Line:%d, Mac Entry is NULL\n", __FUNCTION__, __LINE__));
                return;
            }
            
            PlatformMoveMemory(&SendOutHtCap, &pPort->CommonCfg.HtCapability, sizeof(HT_CAPABILITY_IE));
            // Send out with my desired TX MCSSet  ( Assume it's always MCS_AUTO )
            //PlatformMoveMemory(SendOutHtCap.MCSSet, pPort->CommonCfg.DesiredHtPhy.MCSSet, 16);
            
            // Use base channel BW
            if ((READ_PHY_CFG_BW(pAd, &pBssidMacTabEntry->TxPhyCfg) == BW_40) || 
                 (READ_PHY_CFG_BW(pAd, &pBssidMacTabEntry->TxPhyCfg) == BW_80))
            {
                SendOutHtCap.HtCapInfo.ChannelWidth = BW_40;
            }
            else
            {
                SendOutHtCap.HtCapInfo.ChannelWidth = BW_20;
            }
            
            // Off RDG
            SendOutHtCap.ExtHtCapInfo.RDGSupport = 0;
            
            // Can we select MpduDensity and MaxRAampduFactor?

            // Desired requirements
            SendOutHtCap.HtCapInfo.AMsduSize = pPort->CommonCfg.DesiredHtPhy.AmsduSize;
            SendOutHtCap.HtCapParm.MaxRAmpduFactor = pPort->CommonCfg.DesiredHtPhy.MaxRAmpduFactor;
            SendOutHtCap.HtCapInfo.ShortGIfor20 = pPort->CommonCfg.DesiredHtPhy.ShortGIfor20;
            SendOutHtCap.HtCapInfo.ShortGIfor40 = pPort->CommonCfg.DesiredHtPhy.ShortGIfor40;
            SendOutHtCap.HtCapInfo.TxSTBC = pPort->CommonCfg.DesiredHtPhy.TxSTBC;
            SendOutHtCap.HtCapInfo.RxSTBC = pPort->CommonCfg.DesiredHtPhy.RxSTBC;
            
            // order:16
            Length = sizeof(HT_CAPABILITY_IE);
            MakeOutgoingFrame(pOutBuffer + FrameLen,            &tmp,
                                1,                              &HtCapIe,
                                1,                              &Length,
                                sizeof(HT_CAPABILITY_IE),       &SendOutHtCap,
                                END_OF_ARGS);
            FrameLen += tmp;
 
            // order:17
            // 20/40 BSS Coexistence (7.3.2.60)
            if ((OPSTATUS_TEST_FLAG(pAd, fOP_STATUS_SCAN_2040)) && (OPSTATUS_TEST_FLAG(pPort, fOP_STATUS_SCAN_2040)))
            {
                Length = 1;
                MakeOutgoingFrame(pOutBuffer + FrameLen,    &tmp,
                              1,                            &BssCoexistIe,
                              1,                            &Length,
                              1,                            &pPort->CommonCfg.BSSCoexist2040.word,
                              END_OF_ARGS);
                FrameLen += tmp;
            }

            // order:11
            //  Extended Capabilities (7.3.2.27)
            if (pPort->CommonCfg.ExtCapIE.DoubleWord > 0)
            {
                Length = sizeof(EXT_CAP_ELEMT);
                MakeOutgoingFrame(pOutBuffer + FrameLen,        &tmp,
                              1,                                &ExtHtCapIe,
                              1,                                &Length,
                              sizeof(EXT_CAP_ELEMT),            &pPort->CommonCfg.ExtCapIE.DoubleWord,
                              END_OF_ARGS);
                FrameLen += tmp;
            }
        
        }
        
        // TPK Handshake if RSNA Enabled
        // Pack TPK Message 2 here! 
        if (((pPort->PortCfg.AuthMode == Ndis802_11AuthModeWPA2) || (pPort->PortCfg.AuthMode == Ndis802_11AuthModeWPA2PSK)) &&
            ((pPort->PortCfg.WepStatus == Ralink802_11Encryption2Enabled) || (pPort->PortCfg.WepStatus == Ralink802_11Encryption3Enabled)))
        {       
            FT_IE   *ft;

            // RSNIE (7.3.2.25)         
            // Insert RSN_IE of the Peer TDLS to outgoing frame
            MakeOutgoingFrame(pOutBuffer + FrameLen,    &tmp,
                    RsnLen,                             &RsnIe,
                    END_OF_ARGS);
            FrameLen += tmp;
        
                
            // FTIE (7.3.2.48)
            // Construct FTIE (IE + Length + MIC Control + MIC + ANonce + SNonce)
            
            // point to the element of IE
            ft = (FT_IE *)&FTIe[2]; 
            // generate ANonce
            GenRandom(pAd, ft->ANonce);
            DumpFrameMessage(ft->ANonce, 32,("TDLS - Generate ANonce\n"));
            // set MIC field to zero before MIC calculation
            PlatformZeroMemory(ft->MIC, 16);
            // copy SNonce from peer TDLS
            PlatformMoveMemory(ft->SNonce, pTDLS->SNonce, 32);
            // copy ANonce to TDLS entry
            PlatformMoveMemory(pTDLS->ANonce, ft->ANonce, 32);

            // %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
            // Derive TPK-KCK for MIC key, TPK-TK for direct link data
            TdlsDeriveTPK(
                    pTDLS->MacAddr, /* MAC Address of Initiator */
                    pAd->HwCfg.CurrentAddress, /* I am Responder */
                    pTDLS->ANonce, 
                    pTDLS->SNonce, 
                    pPort->PortCfg.Bssid,
                    pTDLS->TPK,
                    pTDLS->TPKName);

            // %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%

            ////////////////////////////////////////////////////////////////////////
            // The MIC field of FTIE shall be calculated on the concatenation, in the following order, of
            // 1. MAC_I (6 bytes)
            // 2. MAC_R (6 bytes)
            // 3. Transaction Sequence = 2 (1 byte)
            // 4. Link Identifier (20 bytes)
            // 5. RSN IE without the IE header (20 bytes)
            // 6. Timeout Interval IE (7 bytes)
            // 7. FTIE without the IE header, with the MIC field of FTIE set to zero (82 bytes) 
            {
                UCHAR   content[512];
                ULONG   c_len = 0;
                ULONG   tmp_len = 0;
                UCHAR   Seq = 2;
                UCHAR   mic[16];
                
                PlatformZeroMemory(mic, sizeof(mic));
                    
                /* make a header frame for calculating MIC. */
                MakeOutgoingFrame(content,                  &tmp_len,
                                  MAC_ADDR_LEN,             pTDLS->MacAddr,
                                  MAC_ADDR_LEN,             pAd->HwCfg.CurrentAddress,
                                  1,                        &Seq,
                                  END_OF_ARGS);
                c_len += tmp_len;                   

                /* concatenate Link Identifier */           
                MakeOutgoingFrame(content + c_len,      &tmp_len,
                                  20,                   LinkIdentifier,
                                  END_OF_ARGS);
                c_len += tmp_len;                   

                
                /* concatenate RSNIE */
                MakeOutgoingFrame(content + c_len,      &tmp_len,
                                  20,                   &RsnIe[2],
                                  END_OF_ARGS);
                c_len += tmp_len;                   

                /* concatenate Timeout Interval IE */
                MakeOutgoingFrame(content + c_len,     &tmp_len,
                                  7,                    TIIe,
                                  END_OF_ARGS);
                c_len += tmp_len;
                
                
                /* concatenate FTIE */          
                MakeOutgoingFrame(content + c_len,      &tmp_len,
                                  sizeof(FT_IE) - 256,  (PUCHAR)ft,
                                  END_OF_ARGS);
                c_len += tmp_len;

                
                /* Calculate MIC */             
                AES_128_CMAC(pTDLS->TPK, content, c_len, mic);

                // Fill Mic to ft struct
                PlatformMoveMemory(ft->MIC, mic, 16);

            }
            ////////////////////////////////////////////////////////////////////////

#if 0   // FT_InsertXXX shall be defined to Dot11R              
            // Insert FT_IE to outgoing frame
            FT_InsertFTIE(
                    pAd, 
                    pOutBuffer + FrameLen, 
                    &FrameLen, 
                    sizeof(FT_IE) - 256, 
                    ft->MICControl, 
                    ft->MIC, 
                    ft->ANonce, 
                    ft->SNonce);

            // Timeout Interval (7.3.2.49)
            // Insert TI_IE to outgoing frame
            FT_InsertTimeoutIntervalIE(
                    pAd, 
                    pOutBuffer + FrameLen, 
                    &FrameLen, 
                    2, /* key lifetime interval */
                    pTDLS->KeyLifetime);

#else
        {
            UCHAR   ValueIE;
            UINT16  MICCtrBuf;
            UINT32  TimeoutValueBuf;
            UCHAR   TimeoutType;
            //UCHAR Length;
            
            // Insert FT_IE to outgoing frame
            ValueIE = IE_FAST_BSS_TRANSITION;
            Length =  sizeof(FT_IE) - 256;
            MICCtrBuf = cpu2le16(ft->MICControl.word);
            
            MakeOutgoingFrame(  pOutBuffer + FrameLen,  &tmp,
                                1,                      &ValueIE,
                                1,                      &Length,
                                2,                      (PUCHAR)&MICCtrBuf,
                                16,                     (PUCHAR)&ft->MIC,
                                32,                     (PUCHAR)ft->ANonce,
                                32,                     (PUCHAR)ft->SNonce,
                                END_OF_ARGS);
            FrameLen += tmp;


            // Timeout Interval (7.3.2.49)
            // Insert TI_IE to outgoing frame
            ValueIE = IE_TIMEOUT_INTERVAL;          
            Length = 5;
            TimeoutType = 2;    // type: key lifetime interval
            TimeoutValueBuf = cpu2le32(pTDLS->KeyLifetime);

            MakeOutgoingFrame(  pOutBuffer + FrameLen,  &tmp,
                                1,                      &ValueIE,
                                1,                      &Length,
                                1,                      (PUCHAR)&TimeoutType,
                                4,                      (PUCHAR)&TimeoutValueBuf,
                                END_OF_ARGS);
            FrameLen += tmp;
            
        }
#endif
        }

        if(pPort->Channel >14)
        {
            if(pPort->CommonCfg.PhyMode == PHY_11VHT)
            {       
                //
                // order: 19, AID_IE
                //
                // AID: bit_15:14 suppose to be b'11
                VhtAidIeLen = SIZE_OF_VHT_AID_IE;
                MakeOutgoingFrame(pOutBuffer + FrameLen, &tmp,
                                    1,                   &VhtAidIe,
                                    1,                   &VhtAidIeLen,
                                    SIZE_OF_VHT_AID_IE,  &pAd->StaActive.Aid, 
                                    END_OF_ARGS);

                
                
                FrameLen += tmp;
                //
                // order: 20, VHT_CAPABILITIES_IE
                //
                VhtCapIeLen = SIZE_OF_VHT_CAP_IE;
                MakeOutgoingFrame(pOutBuffer+FrameLen,   &tmp,
                                    1,                   &VhtCapIe,
                                    1,                   &VhtCapIeLen,
                                    SIZE_OF_VHT_CAP_IE,  &pPort->CommonCfg.DesiredVhtPhy.VhtCapability,
                                    END_OF_ARGS);
                FrameLen += tmp;
            }
        }

        // TODO: Currently, we don't suppot TDLS channel switching feature
        if (0)
        {
            // Supported Channels (7.3.2.19)

            // Supported Regulatory Classes(7.3.2.51)
        }
        
        // ==>> Set sendout timer
        PlatformCancelTimer(&pTDLS->Timer, &TimerCancelled);
        PlatformSetTimer(pPort,&pTDLS->Timer, Timeout);
        // ==>> State Change
        pTDLS->Status = TDLS_MODE_WAIT_ADD;
            
    }
    else
    {
        MakeOutgoingFrame(pOutBuffer,               &FrameLen,
                        sizeof(HEADER_802_11),      &Header80211,
                        LENGTH_802_1_H,             TDLS_LLC_SNAP,
                        sizeof(TDLS_ACTION_HEADER), &TdlsActHdr,
                        2,                          &StatusCode,
                        1,                          &Token,
                      END_OF_ARGS);
    }

    DBGPRINT(RT_DEBUG_TRACE,("TDLS - TdlsPeerSetupReqAction() SEND OUT Setup Response Frame with StatusCode=%d \n", StatusCode));
    // Send a packet via HardTransmit
    TdlsSendPacket(pAd, pOutBuffer, FrameLen);
    MlmeFreeMemory(pAd, pOutBuffer);
}


/*
    ==========================================================================
    Description:
        
    IRQL = DISPATCH_LEVEL

    ==========================================================================
 */
VOID TdlsPeerSetupRspAction(
    IN PMP_ADAPTER pAd, 
    IN MLME_QUEUE_ELEM *Elem) 
{
    PUCHAR          pOutBuffer = NULL;
    NDIS_STATUS     NStatus;    
    ULONG           FrameLen = 0;
    USHORT          StatusCode = MLME_SUCCESS, LocalStatusCode = MLME_SUCCESS;
    ULONG           tmp;
    ULONG           VhtOpIeLen;
    SHORT           i;
    ULONG           Timeout = TDLS_TIMEOUT;
    BOOLEAN         TimerCancelled;
    PRT_802_11_TDLS pTDLS = NULL;
    UCHAR           Token;
    UCHAR           PeerAddr[MAC_ADDR_LEN], SA[MAC_ADDR_LEN];
    USHORT          CapabilityInfo;
    UCHAR           SupRate[MAX_LEN_OF_SUPPORTED_RATES], ExtRate[MAX_LEN_OF_SUPPORTED_RATES];
    UCHAR           SupRateLen, ExtRateLen, HtCapLen, RsnLen, FTLen, TILen;
    UCHAR           RsnIe[64], FTIe[128], TIIe[7];
    UCHAR           QosCapability;
    HT_CAPABILITY_IE    HtCap;
    EXT_CAP_ELEMT       ExtCap;
    HEADER_802_11       Header80211;
    TDLS_ACTION_HEADER  TdlsActHdr;
    INT                 LinkId = 0xff;
    UCHAR               LinkIdentifier[20]; 
    UCHAR               TPK[LEN_PMK], TPKName[LEN_PMK_NAME];
    BOOLEAN             bWmmCapable;
    UCHAR               LengthQosPAD;
    PMP_PORT          pPort = pAd->PortList[Elem->PortNum];
    
    // Not TDLS Capable, ignore it
    if(!TDLS_ON(pAd))
        return;
    
    // Not BSS mode, ignore it
    if (!INFRA_ON(pPort))
        return;
        
    // Init all kinds of fields within the packet
    PlatformZeroMemory(&CapabilityInfo, sizeof(CapabilityInfo));
    PlatformZeroMemory(&HtCap, sizeof(HtCap));
    PlatformZeroMemory(&ExtCap, sizeof(ExtCap));
    PlatformZeroMemory(RsnIe, sizeof(RsnIe));
    PlatformZeroMemory(FTIe, sizeof(FTIe));
    PlatformZeroMemory(TIIe, sizeof(TIIe));
    PlatformZeroMemory(PeerAddr, MAC_ADDR_LEN); 
    PlatformZeroMemory(SA, MAC_ADDR_LEN);

    if (!PeerTdlsSetupRspSanity(
                            pAd,
                            pPort,
                            Elem->Msg, 
                            Elem->MsgLen,
                            &Token,
                            SA,
                            PeerAddr,
                            &CapabilityInfo,
                            &SupRateLen,    
                            SupRate,
                            &ExtRateLen,
                            ExtRate,
                            &bWmmCapable,                           
                            &QosCapability,
                            &HtCapLen,
                            &HtCap,
                            &ExtCap,
                            &StatusCode,
                            &RsnLen,    
                            RsnIe,
                            &FTLen,
                            FTIe,
                            &TILen,
                            TIIe))
    {
        LocalStatusCode = MLME_REQUEST_DECLINED;
    }


    DBGPRINT(RT_DEBUG_TRACE,("TDLS - TdlsPeerSetupRspAction() received a response from %02x:%02x:%02x:%02x:%02x:%02x with StatusCode=%d\n", SA[0], SA[1], SA[2], SA[3], SA[4], SA[5], StatusCode));


    // An AP may discard TDLS Setup Request frames to prevent direct links from 
    // being set up in its BSS. In this case, the AP shall send a TDLS Setup Response frame
    // with status code 4 and insert Bssid into the Address 3 field in the Link Identifier element.
    if ((StatusCode == MLEM_TDLS_NOT_ALLOW_IN_BSS) && MAC_ADDR_EQUAL(PeerAddr, pPort->PortCfg.Bssid))
    {
        DBGPRINT(RT_DEBUG_ERROR,("TDLS - TdlsPeerSetupRspAction() - MLEM_TDLS_NOT_ALLOW_IN_BSS, terminate the setup procedure\n"));
        return;
    }
    // Received a error code from the Peer TDLS.
    // Let's terminate the setup procedure right now.
    else if (StatusCode != MLME_SUCCESS)
    {
        LinkId = TdlsSearchLinkId(pAd, SA);
        if ((LinkId >= 0) && (LinkId < MAX_NUM_OF_TDLS_ENTRY))
        {
            // Clear my local entry
            pAd->StaCfg.TDLSEntry[LinkId].Status = TDLS_MODE_NONE;
            pAd->StaCfg.TDLSEntry[LinkId].Valid = FALSE;
            PlatformCancelTimer(&pPort->StaCfg.TDLSEntry[LinkId].Timer, &TimerCancelled);
        }
        DBGPRINT(RT_DEBUG_ERROR,("TDLS - TdlsPeerSetupRspAction() received a failed StatusCode, terminate the setup procedure #%d\n", LinkId));
        return;
    }


    // Drop not within my TDLS Table that created before !
    LinkId = TdlsSearchLinkId(pAd, PeerAddr);
    if ((LinkId == -1) || (LinkId == MAX_NUM_OF_TDLS_ENTRY) || MAC_ADDR_EQUAL(PeerAddr, ZERO_MAC_ADDR))
    {
        DBGPRINT(RT_DEBUG_ERROR,("TDLS - TdlsPeerSetupRspAction() invalid entry\n"));
        return;
    }

    // Point to the current Link ID
    pTDLS = (PRT_802_11_TDLS)&pAd->StaCfg.TDLSEntry[LinkId];
    // Cancel the timer since the received packet to me.
    PlatformCancelTimer(&pTDLS->Timer, &TimerCancelled);
    // Update status code to results in parsing
    StatusCode = LocalStatusCode;
    
    // 
    // Validate the content on Setup Response Frame
    //
    while (StatusCode == MLME_SUCCESS)
    {       
        // Invalid TDLS State
        if (pTDLS->Status != TDLS_MODE_WAIT_RESULT)
        {
            DBGPRINT(RT_DEBUG_ERROR,("TDLS - TdlsPeerSetupRspAction() Not in TDLS_MODE_WAIT_RESULT STATE\n"));
            StatusCode =  MLME_REQUEST_DECLINED;
            break;
        }

        // Is the same Dialog Token?
        if (pTDLS->Token != Token)
        {
            DBGPRINT(RT_DEBUG_ERROR,("TDLS - TdlsPeerSetupRspAction() Not match with Dialig Token \n"));        
            StatusCode =  MLME_REQUEST_DECLINED;
            break;
        }
        
        // Process TPK Handshake Message 2 here!
        if (((pPort->PortCfg.AuthMode == Ndis802_11AuthModeWPA2) || (pPort->PortCfg.AuthMode == Ndis802_11AuthModeWPA2PSK)) &&
            ((pPort->PortCfg.WepStatus == Ralink802_11Encryption2Enabled) || (pPort->PortCfg.WepStatus == Ralink802_11Encryption3Enabled)))
        {   
            USHORT Result;

            // RSNIE (7.3.2.25), FTIE (7.3.2.48), Timeout Interval (7.3.2.49)
            Result = TdlsTPKMsg2Process(pAd, pPort, pTDLS, RsnIe, RsnLen, FTIe, FTLen, TIIe, TILen, TPK, TPKName);
            if (Result != MLME_SUCCESS)
            {
                DBGPRINT(RT_DEBUG_ERROR,("TDLS - TdlsTPKMsg2Process() Failed, reason=%d \n", Result));
                if (Result == MLME_UNSPECIFY_FAIL)  // if mic error , ignore
                {
                    return;
                }
                else
                {
                    StatusCode = Result;
                    goto send_out;
                }
            }
            // Copy ANonce, Key lifetime, TPK, TPK Name
            pTDLS->KeyLifetime =  le2cpu32(*((PULONG) (TIIe + 3)));
            PlatformMoveMemory(pTDLS->ANonce, &FTIe[20], 32);
            PlatformMoveMemory(pTDLS->TPK, TPK, LEN_PMK);
            PlatformMoveMemory(pTDLS->TPKName, TPKName, LEN_PMK_NAME);
        }

        // Update parameters
        if (StatusCode == MLME_SUCCESS)
        {
            // I am Initiator.
            pTDLS->bInitiator = TRUE;
            // Capabilities
            pTDLS->CapabilityInfo = CapabilityInfo;

            // Copy Responder's supported rate
            PlatformZeroMemory(pTDLS->SupRate, sizeof(pTDLS->SupRate));
            PlatformZeroMemory(pTDLS->ExtRate, sizeof(pTDLS->ExtRate));
            pTDLS->SupRateLen = SupRateLen;
            PlatformMoveMemory(pTDLS->SupRate, SupRate, SupRateLen);
            pTDLS->ExtRateLen = ExtRateLen;
            PlatformMoveMemory(pTDLS->ExtRate, ExtRate, ExtRateLen);

            // Copy Responder's ht rates
            if ((HtCapLen > 0) && (pPort->CommonCfg.PhyMode >= PHY_11ABGN_MIXED))
            {
                PlatformZeroMemory(&pTDLS->HtCapability, SIZE_HT_CAP_IE);
                pTDLS->HtCapabilityLen = SIZE_HT_CAP_IE;
                PlatformMoveMemory(&pTDLS->HtCapability, &HtCap, pTDLS->HtCapabilityLen);
            }
            else
            {
                pTDLS->HtCapabilityLen = 0;
                PlatformZeroMemory(&pTDLS->HtCapability, SIZE_HT_CAP_IE);
            }

            // Copy extended capability
            PlatformMoveMemory(&pTDLS->ExtCap, &ExtCap, sizeof(EXT_CAP_ELEMT));

            // Copy QOS related information
            pTDLS->QosCapability = QosCapability;
            pTDLS->bWmmCapable = bWmmCapable;
            
            // Copy TPK related information
            if (((pPort->PortCfg.AuthMode == Ndis802_11AuthModeWPA2) || (pPort->PortCfg.AuthMode == Ndis802_11AuthModeWPA2PSK)) &&
                ((pPort->PortCfg.WepStatus == Ralink802_11Encryption2Enabled) || (pPort->PortCfg.WepStatus == Ralink802_11Encryption3Enabled)))
            {
                // SNonce, Key lifetime
            }

        }

        
        break;
    }

    
send_out:   
    //==========================================================================
    // Start to Send TDLS Setup Confirm to the Initiator
    //==========================================================================
    
    NStatus = MlmeAllocateMemory(pAd, &pOutBuffer);  //Get an unused nonpaged memory
    if (NStatus != NDIS_STATUS_SUCCESS) 
    {
        DBGPRINT(RT_DEBUG_ERROR,("TDLS - TdlsPeerSetupRspAction() allocate memory failed \n"));
        return;
    }

    // Qos will be add in HardTx
    LengthQosPAD = 0;

    // 1. Make 802.11 header for Vista, where Addr1: AP, Addr2: SA, Addr3: DA
    // Sequence Number and WEP bit will be updated later
    PlatformZeroMemory(&Header80211, sizeof(HEADER_802_11));
    Header80211.FC.Type = BTYPE_DATA;   
    COPY_MAC_ADDR(Header80211.Addr1, pPort->PortCfg.Bssid); 
    COPY_MAC_ADDR(Header80211.Addr2, pAd->HwCfg.CurrentAddress);
    COPY_MAC_ADDR(Header80211.Addr3, PeerAddr);
    Header80211.FC.ToDs = 1; // through AP

    // 2. Prepare LLC_SNAP and TDLS Action header
    TdlsActHdr.Protocol = PROTO_NAME_TDLS;
    TdlsActHdr.Category = CATEGORY_TDLS;
    TdlsActHdr.Action   = ACTION_TDLS_SETUP_CONF;

    // Build basic frame first
    if ((StatusCode == MLME_SUCCESS) && pTDLS)
    {
        // Construct LinkIdentifier (IE + Length + BSSID + Initiator MAC + Responder MAC)
        PlatformZeroMemory(LinkIdentifier, 20);
        LinkIdentifier[0] = IE_TDLS_LINK_ID;
        LinkIdentifier[1] = 18;
        PlatformMoveMemory(&LinkIdentifier[2], pPort->PortCfg.Bssid, 6);
        PlatformMoveMemory(&LinkIdentifier[8], pAd->HwCfg.CurrentAddress, 6);
        PlatformMoveMemory(&LinkIdentifier[14], pTDLS->MacAddr, 6);


        MakeOutgoingFrame(pOutBuffer,               &FrameLen,
                        sizeof(HEADER_802_11),      &Header80211,
                        LENGTH_802_1_H,             TDLS_LLC_SNAP,
                        sizeof(TDLS_ACTION_HEADER), &TdlsActHdr,
                        2,                          &StatusCode,
                        1,                          &pTDLS->Token,  // order: 4
                        sizeof(LinkIdentifier),     LinkIdentifier, // order: 10
                        END_OF_ARGS);

        //
        // Optional fields
        //
        // order: 6
        // EDCA Parameters (7.3.2.29)
        if (((pPort->CommonCfg.PhyMode >= PHY_11ABGN_MIXED) || (pPort->CommonCfg.bWmmCapable)) && (pTDLS->bWmmCapable))
        {
            /* The EDCA Parameter Set element inside the TDLS Setup Confirm frame shall be used 
             * by the TDLS peer STAs on the base channel when the BSS is not QoS capable, and/or on the off-channel.
             * The ACM subfields inside the EDCA Parameter Set element shall be set to 0.
             */
             
            /* When the BSS is QoS capable, then the BSS QoS parameters shall be
             * used by the TDLS peer STAs on the AP's channel, and the values 
             * indicated inside the TDLS Setup Confirm frame apply only for the 
             * off-channel. The EDCA parameters for the off-channel should be 
             * the same as those on the AP's channel when QoS is supported by the BSS, 
             * because this may optimize the channel switching process.
             */

            UCHAR WmeParmIe[26] = {IE_VENDOR_SPECIFIC, 24, 0x00, 0x50, 0xf2, 0x02, 0x01, 0x01, 0, 0}; 

            //  Init EdcaParam
            PlatformZeroMemory(&pTDLS->IrEdcaParm, sizeof(EDCA_PARM));

            if (OPSTATUS_TEST_FLAG(pPort, fOP_STATUS_WMM_INUSED))
            {
                // Copy EDCA parameters of the current associated AP.
                PlatformMoveMemory(&pTDLS->IrEdcaParm, &pPort->CommonCfg.APEdcaParm, sizeof(EDCA_PARM));
            }
            else
            {
                // Zero ACM
                
                // Enable EdcaParm used in non-QBSS.
                pTDLS->IrEdcaParm.bValid = TRUE;

                pTDLS->IrEdcaParm.bQAck        = FALSE;
                pTDLS->IrEdcaParm.bQueueRequest   = FALSE;
                pTDLS->IrEdcaParm.bTxopRequest    = FALSE;
                //pTDLS->IrEdcaParm.bMoreDataAck     = FALSE;

                pTDLS->IrEdcaParm.EdcaUpdateCount = 1;

                /*if (pPort->CommonCfg.bAPSDCapable && pPort->CommonCfg.ExtCapIE.field.UAPSD && pTDLS->ExtCap.field.UAPSD)
                    pTDLS->IrEdcaParm.bAPSDCapable = TRUE;
                else*/
                    pTDLS->IrEdcaParm.bAPSDCapable = FALSE;

                // By hardcoded ----------->STA-->AP
                pTDLS->IrEdcaParm.Aifsn[0] = 3;// 3
                pTDLS->IrEdcaParm.Aifsn[1] = 7;// 7
                pTDLS->IrEdcaParm.Aifsn[2] = 2;// 1
                pTDLS->IrEdcaParm.Aifsn[3] = 2;// 1

                pTDLS->IrEdcaParm.Cwmin[0] = 4;// 4
                pTDLS->IrEdcaParm.Cwmin[1] = 4;// 4
                pTDLS->IrEdcaParm.Cwmin[2] = 3;// 3
                pTDLS->IrEdcaParm.Cwmin[3] = 2;// 2

                pTDLS->IrEdcaParm.Cwmax[0] = 10;// 6
                pTDLS->IrEdcaParm.Cwmax[1] = 10;// 10
                pTDLS->IrEdcaParm.Cwmax[2] = 4;// 4
                pTDLS->IrEdcaParm.Cwmax[3] = 3;// 3

                pTDLS->IrEdcaParm.Txop[0]  = 0;// 0
                pTDLS->IrEdcaParm.Txop[1]  = 0;// 0
                pTDLS->IrEdcaParm.Txop[2]  = 96;// 96
                pTDLS->IrEdcaParm.Txop[3]  = 48;// 48
            
            }


            WmeParmIe[2] =  ((UCHAR)pTDLS->IrEdcaParm.bQAck << 4) + 
                ((UCHAR)pTDLS->IrEdcaParm.bQueueRequest << 5) + 
                ((UCHAR)pTDLS->IrEdcaParm.bTxopRequest << 6);

            WmeParmIe[8] = pTDLS->IrEdcaParm.EdcaUpdateCount & 0x0f;
            WmeParmIe[8] |= pTDLS->IrEdcaParm.bAPSDCapable << 7;

            for (i=QID_AC_BE; i<=QID_AC_VO; i++)
            {
                WmeParmIe[10+ (i*4)] = (i << 5)                                 +     // b5-6 is ACI
                                       ((UCHAR)pTDLS->IrEdcaParm.bACM[i] << 4)  +     // b4 is ACM
                                       (pTDLS->IrEdcaParm.Aifsn[i] & 0x0f);           // b0-3 is AIFSN
                WmeParmIe[11+ (i*4)] = (pTDLS->IrEdcaParm.Cwmax[i] << 4)        +     // b5-8 is CWMAX
                                       (pTDLS->IrEdcaParm.Cwmin[i] & 0x0f);           // b0-3 is CWMIN
                WmeParmIe[12+ (i*4)] = (UCHAR)(pTDLS->IrEdcaParm.Txop[i] & 0xff);         // low byte of TXOP
                WmeParmIe[13+ (i*4)] = (UCHAR)(pTDLS->IrEdcaParm.Txop[i] >> 8);       // high byte of TXOP
            }
            
            
            MakeOutgoingFrame(pOutBuffer+FrameLen,      &tmp,
                              26,                       WmeParmIe,
                              END_OF_ARGS);
            FrameLen += tmp;
        }

        // order: 9
        // HT Operation (7.3.2.57)
        if ((pTDLS->HtCapabilityLen > 0) && (pPort->CommonCfg.PhyMode >= PHY_11ABGN_MIXED))
        {
            UCHAR       AddHtLen = sizeof(ADD_HT_INFO_IE);
            
            MakeOutgoingFrame(pOutBuffer + FrameLen,    &tmp,
                            1,                          &AddHtInfoIe,
                            1,                          &AddHtLen,
                            sizeof(ADD_HT_INFO_IE),     &pPort->CommonCfg.AddHTInfo, 
                            END_OF_ARGS);
            FrameLen += tmp;    
        }
        
        // TPK Handshake if RSNA Enabled
        // Pack TPK Message 3 here! 
        // order: 7
        if (((pPort->PortCfg.AuthMode == Ndis802_11AuthModeWPA2) || (pPort->PortCfg.AuthMode == Ndis802_11AuthModeWPA2PSK)) &&
            ((pPort->PortCfg.WepStatus == Ralink802_11Encryption2Enabled) || (pPort->PortCfg.WepStatus == Ralink802_11Encryption3Enabled)))
        {       
            FT_IE   *ft;
            
            // RSNIE (7.3.2.25)         
            // Insert RSN_IE of the Peer TDLS to outgoing frame
            MakeOutgoingFrame(pOutBuffer + FrameLen,    &tmp,
                    RsnLen,                             &RsnIe,
                    END_OF_ARGS);
            FrameLen += tmp;
        
                
            // FTIE (7.3.2.48)
            // Construct FTIE (IE + Length + MIC Control + MIC + ANonce + SNonce)
            
            // point to the element of IE
            ft = (FT_IE *)&FTIe[2]; 
            // set MIC field to zero before MIC calculation
            PlatformZeroMemory(ft->MIC, 16);

            ////////////////////////////////////////////////////////////////////////
            // The MIC field of FTIE shall be calculated on the concatenation, in the following order, of
            // 1. MAC_I (6 bytes)
            // 2. MAC_R (6 bytes)
            // 3. Transaction Sequence = 2 (1 byte)
            // 4. Link Identifier (20 bytes)
            // 5. RSN IE without the IE header (20 bytes)
            // 6. Timeout Interval IE (7 bytes)
            // 7. FTIE without the IE header, with the MIC field of FTIE set to zero (82 bytes) 
            {
                UCHAR   content[512];
                ULONG   c_len = 0;
                ULONG   tmp_len = 0;
                UCHAR   Seq = 3;
                UCHAR   mic[16];
                
                PlatformZeroMemory(mic, sizeof(mic));
                    
                /* make a header frame for calculating MIC. */
                MakeOutgoingFrame(content,                  &tmp_len,
                                  MAC_ADDR_LEN,             pAd->HwCfg.CurrentAddress,
                                  MAC_ADDR_LEN,             pTDLS->MacAddr,
                                  1,                        &Seq,
                                  END_OF_ARGS);
                c_len += tmp_len;                   

                /* concatenate Link Identifier */           
                MakeOutgoingFrame(content + c_len,      &tmp_len,
                                  20,                   LinkIdentifier,
                                  END_OF_ARGS);
                c_len += tmp_len;                   

                
                /* concatenate RSNIE */
                MakeOutgoingFrame(content + c_len,      &tmp_len,
                                  20,                   &RsnIe[2],
                                  END_OF_ARGS);
                c_len += tmp_len;                   

                /* concatenate Timeout Interval IE */
                MakeOutgoingFrame(content + c_len,     &tmp_len,
                                  7,                    TIIe,
                                  END_OF_ARGS);
                c_len += tmp_len;
                
                
                /* concatenate FTIE */          
                MakeOutgoingFrame(content + c_len,      &tmp_len,
                                  sizeof(FT_IE) - 256,  (PUCHAR)ft,
                                  END_OF_ARGS);
                c_len += tmp_len;
                

                /* Calculate MIC */             
                AES_128_CMAC(pTDLS->TPK, content, c_len, mic);

                // Fill Mic to ft struct
                PlatformMoveMemory(ft->MIC, mic, 16);

            }
            ////////////////////////////////////////////////////////////////////////

#if 0   // FT_InsertXXX shall be defined to Dot11R  
            // Insert FT_IE to outgoing frame
            FT_InsertFTIE(
                    pAd, 
                    pOutBuffer + FrameLen, 
                    &FrameLen, 
                    sizeof(FT_IE) - 256, 
                    ft->MICControl, 
                    ft->MIC, 
                    ft->ANonce, 
                    ft->SNonce);

            // Timeout Interval (7.3.2.49)
            // Insert TI_IE to outgoing frame
            FT_InsertTimeoutIntervalIE(
                    pAd, 
                    pOutBuffer + FrameLen, 
                    &FrameLen, 
                    2, /* key lifetime interval */
                    pTDLS->KeyLifetime);

#else
        {
            UCHAR   ValueIE;
            UINT16  MICCtrBuf;
            UINT32  TimeoutValueBuf;
            UCHAR   TimeoutType;
            UCHAR   Length;
            
            // Insert FT_IE to outgoing frame
            ValueIE = IE_FAST_BSS_TRANSITION;
            Length =  sizeof(FT_IE) - 256;
            MICCtrBuf = cpu2le16(ft->MICControl.word);
            
            MakeOutgoingFrame(  pOutBuffer + FrameLen,  &tmp,
                                1,                      &ValueIE,
                                1,                      &Length,
                                2,                      (PUCHAR)&MICCtrBuf,
                                16,                     (PUCHAR)&ft->MIC,
                                32,                     (PUCHAR)ft->ANonce,
                                32,                     (PUCHAR)ft->SNonce,
                                END_OF_ARGS);
            FrameLen += tmp;


            // Timeout Interval (7.3.2.49)
            // Insert TI_IE to outgoing frame
            ValueIE = IE_TIMEOUT_INTERVAL;          
            Length = 5;
            TimeoutType = 2;    // type: key lifetime interval
            TimeoutValueBuf = cpu2le32(pTDLS->KeyLifetime);

            MakeOutgoingFrame(  pOutBuffer + FrameLen,  &tmp,
                                1,                      &ValueIE,
                                1,                      &Length,
                                1,                      (PUCHAR)&TimeoutType,
                                4,                      (PUCHAR)&TimeoutValueBuf,
                                END_OF_ARGS);
            FrameLen += tmp;
            
        }
#endif
        }

        //
        // order: 11, VHT_OP_IE
        //      
        VhtOpIeLen = SIZE_OF_VHT_OP_IE;
        MakeOutgoingFrame(pOutBuffer + FrameLen, &tmp,
                            1,                   &VhtOpIe,
                            1,                   &VhtOpIeLen,
                            SIZE_OF_VHT_OP_IE,   &pPort->CommonCfg.DesiredVhtPhy.VhtOperation, 
                            END_OF_ARGS);

        DBGPRINT(RT_DEBUG_TRACE,("TDLS - TdlsPeerSetupRspAction() SEND OUT Setup Confirm Frame with StatusCode=%d \n", StatusCode));
        // Send a packet via HardTransmit
        TdlsSendPacket(pAd, pOutBuffer, FrameLen);
        MlmeFreeMemory(pAd, pOutBuffer);


        // Procedure in Succeed
        // ==>> State Change
        pTDLS->Status = TDLS_MODE_CONNECTED;

                
        DBGPRINT(RT_DEBUG_TRACE, ("*************** TDLS Setup Procedure - Success ***************\n\n"));
    }
    else
    {
        if ((StatusCode != MLME_SUCCESS) && pTDLS)
        {
            // Construct LinkIdentifier (IE + Length + BSSID + Initiator MAC + Responder MAC)
            PlatformZeroMemory(LinkIdentifier, 20);
            LinkIdentifier[0] = IE_TDLS_LINK_ID;
            LinkIdentifier[1] = 18;
            PlatformMoveMemory(&LinkIdentifier[2], pPort->PortCfg.Bssid, 6);
            PlatformMoveMemory(&LinkIdentifier[8], pAd->HwCfg.CurrentAddress, 6);
            PlatformMoveMemory(&LinkIdentifier[14], pTDLS->MacAddr, 6);


            MakeOutgoingFrame(pOutBuffer,               &FrameLen,
                            sizeof(HEADER_802_11),      &Header80211,
                            LENGTH_802_1_H,             TDLS_LLC_SNAP,
                            sizeof(TDLS_ACTION_HEADER), &TdlsActHdr,
                            2,                          &StatusCode,
                            1,                          &pTDLS->Token,
                            sizeof(LinkIdentifier),     LinkIdentifier,
                            END_OF_ARGS);

            // Send a packet via HardTransmit
            TdlsSendPacket(pAd, pOutBuffer, FrameLen);
            DBGPRINT(RT_DEBUG_ERROR,("TDLS - TdlsPeerSetupRspAction() SEND OUT Setup Confirm Frame with ERROR StatusCode=%d \n", StatusCode));

        }
        MlmeFreeMemory(pAd, pOutBuffer);
    
        // TDLS setup procedure failed. Let's terminate the setup procedure.
        pTDLS->Status = TDLS_MODE_NONE;
        pTDLS->Valid    = FALSE;

        DBGPRINT(RT_DEBUG_ERROR,("TDLS - TdlsPeerSetupRspAction() process failed with StatusCode=%d \n", StatusCode));
    }



    ////////////////////////////////////////////////////////////////////////////
    // Insert into mac table
    //
    if ((StatusCode == MLME_SUCCESS) && (pTDLS) && (pTDLS->Status == TDLS_MODE_CONNECTED))
    {
        UCHAR           MaxSupportedRateIn500Kbps = 0, MinSupportedRateIn500Kbps = 108; 
        UCHAR           MinSupportedRate= RATE_1, MaxSupportedRate= RATE_11;
        UCHAR           idx, j;
        PMAC_TABLE_ENTRY    pMacEntry = NULL;
        PUCHAR              pTable;
        UCHAR               TableSize;
        UCHAR               InitTxRateIdx;
        PRTMP_TX_RATE_SWITCH    pTxRate;
        ULONG               MinimumAMPDUSize = 1; //DInit value is 1 at MACRegTable
        ULONG               Value;

        // look up the existing table
        pMacEntry = MacTableLookup(pAd,  pPort, pTDLS->MacAddr);
        // not exist, add it
        if (!pMacEntry)
        {
            pMacEntry = MacTableInsertEntry(pAd, pPort, pTDLS->MacAddr, FALSE);
        }

        if (pMacEntry)
        {
            //This has to be done, different with windows driver.

            //Keep wcid
            pTDLS->Wcid = (UCHAR)pMacEntry->Aid;
            DBGPRINT(RT_DEBUG_TRACE, ("TDLS - Insert/Update MacEntry (Aid= %d)\n", pMacEntry->Aid));

            //Set state or DRS will not work
            pMacEntry->Sst = SST_ASSOC;

            pMacEntry->ClientStatusFlags = 0;

            // Set WMM capability
            if (((pPort->CommonCfg.PhyMode >= PHY_11ABGN_MIXED) || (pPort->CommonCfg.bWmmCapable)) && (pTDLS->bWmmCapable))
            {
                CLIENT_STATUS_SET_FLAG(pMacEntry, fCLIENT_STATUS_WMM_CAPABLE);
                DBGPRINT(RT_DEBUG_TRACE, ("TDLS -  WMM Capable\n"));

                // Set EDCA to Asic in non-QBSS
                if (!OPSTATUS_TEST_FLAG(pPort, fOP_STATUS_WMM_INUSED) && pTDLS->IrEdcaParm.bValid)
                {
                    MtAsicSetEdcaParm(pAd, pPort, &pTDLS->IrEdcaParm, FALSE);
                }

            }
            else
            {
                CLIENT_STATUS_CLEAR_FLAG(pMacEntry, fCLIENT_STATUS_WMM_CAPABLE);
            }

            //
            // Rate Settings >>>>>>>
            //
            for (idx=0; idx<pTDLS->SupRateLen; idx++)
            {
                if (MaxSupportedRateIn500Kbps < (pTDLS->SupRate[idx] & 0x7f)) 
                    MaxSupportedRateIn500Kbps = pTDLS->SupRate[idx] & 0x7f;
                if (MinSupportedRateIn500Kbps > (pTDLS->SupRate[idx] & 0x7f)) 
                    MinSupportedRateIn500Kbps = pTDLS->SupRate[idx] & 0x7f;

            }
            for (idx=0; idx<pTDLS->ExtRateLen; idx++)
            {
                if (MaxSupportedRateIn500Kbps < (pTDLS->ExtRate[idx] & 0x7f)) 
                    MaxSupportedRateIn500Kbps = pTDLS->ExtRate[idx] & 0x7f;
                if (MinSupportedRateIn500Kbps > (pTDLS->ExtRate[idx] & 0x7f)) 
                    MinSupportedRateIn500Kbps = pTDLS->ExtRate[idx] & 0x7f;

            }

            switch (MaxSupportedRateIn500Kbps)
            {
                case 108: MaxSupportedRate = RATE_54;   break;
                case 96:  MaxSupportedRate = RATE_48;   break;
                case 72:  MaxSupportedRate = RATE_36;   break;
                case 48:  MaxSupportedRate = RATE_24;   break;
                case 36:  MaxSupportedRate = RATE_18;   break;
                case 24:  MaxSupportedRate = RATE_12;   break;
                case 18:  MaxSupportedRate = RATE_9;    break;
                case 12:  MaxSupportedRate = RATE_6;    break;
                case 22:  MaxSupportedRate = RATE_11;   break;
                case 11:  MaxSupportedRate = RATE_5_5;  break;
                case 4:   MaxSupportedRate = RATE_2;    break;
                case 2:   MaxSupportedRate = RATE_1;    break;
                default:  MaxSupportedRate = RATE_11;   break;
            }
            switch (MinSupportedRateIn500Kbps)
            {
                case 108: MinSupportedRate = RATE_54;   break;
                case 96:  MinSupportedRate = RATE_48;   break;
                case 72:  MinSupportedRate = RATE_36;   break;
                case 48:  MinSupportedRate = RATE_24;   break;
                case 36:  MinSupportedRate = RATE_18;   break;
                case 24:  MinSupportedRate = RATE_12;   break;
                case 18:  MinSupportedRate = RATE_9;    break;
                case 12:  MinSupportedRate = RATE_6;    break;
                case 22:  MinSupportedRate = RATE_11;   break;
                case 11:  MinSupportedRate = RATE_5_5;  break;
                case 4:   MinSupportedRate = RATE_2;    break;
                case 2:   MinSupportedRate = RATE_1;    break;
                default:  MinSupportedRate = RATE_11;   break;
            }
        
            if ((MaxSupportedRate < RATE_6) || (pPort->CommonCfg.PhyMode == PHY_11B))
            {
                pMacEntry->RateLen = 4;
                if (MaxSupportedRate >= RATE_6)
                    MaxSupportedRate = RATE_11;
            }
            else if ((MinSupportedRate >= RATE_6) && (MaxSupportedRate <= RATE_54))
            {
                pMacEntry->RateLen = 8;
            }
            else if (((pPort->CommonCfg.SupRateLen + pPort->CommonCfg.ExtRateLen) == 8) && (MinSupportedRate < RATE_6))
            {
                pMacEntry->RateLen = 8;
                MinSupportedRate = RATE_6;
            }
            else
            {
                pMacEntry->RateLen = pTDLS->SupRateLen + pTDLS->ExtRateLen;
            }
            
            WRITE_PHY_CFG_DOUBLE_WORD(pAd, &pMacEntry->MaxPhyCfg, 0);
            WRITE_PHY_CFG_DOUBLE_WORD(pAd, &pMacEntry->MinPhyCfg, 0);
            WRITE_PHY_CFG_DOUBLE_WORD(pAd, &pMacEntry->TxPhyCfg, 0);
            pMacEntry->MaxSupportedRate = MaxSupportedRate;
            
            // CCK Rate
            if (pMacEntry->MaxSupportedRate < RATE_FIRST_OFDM_RATE)
            {
                WRITE_PHY_CFG_MODE(pAd, &pMacEntry->MaxPhyCfg, MODE_CCK);
                WRITE_PHY_CFG_MCS(pAd, &pMacEntry->MaxPhyCfg, pMacEntry->MaxSupportedRate);
                WRITE_PHY_CFG_MODE(pAd, &pMacEntry->MinPhyCfg, MODE_CCK);
                WRITE_PHY_CFG_MCS(pAd, &pMacEntry->MinPhyCfg, MinSupportedRate);
                DBGPRINT(RT_DEBUG_TRACE, ("TDLS -  CCK, MinSupportedRate=%d\n", MinSupportedRate));
            }
            else // OFDM Rate
            {
                WRITE_PHY_CFG_MODE(pAd, &pMacEntry->MaxPhyCfg, MODE_OFDM);
                WRITE_PHY_CFG_MCS(pAd, &pMacEntry->MaxPhyCfg, OfdmRateToRxwiMCS[pMacEntry->MaxSupportedRate]);
                WRITE_PHY_CFG_MODE(pAd, &pMacEntry->MinPhyCfg, MODE_OFDM);
                WRITE_PHY_CFG_MCS(pAd, &pMacEntry->MinPhyCfg, OfdmRateToRxwiMCS[MinSupportedRate]);
                DBGPRINT(RT_DEBUG_TRACE, ("TDLS -  OFDM, MinSupportedRate=%d\n", MinSupportedRate));
            }

            pMacEntry->CapabilityInfo = CapabilityInfo;

            //
            //HT will be used only when both parties are HT capable
            //
            if ((pTDLS->HtCapabilityLen > 0) && (pPort->CommonCfg.PhyMode >= PHY_11ABGN_MIXED))
            {
                PMAC_TABLE_ENTRY pBssidMacTabEntry = MlmeSyncMacTabMatchedRoleType(pPort, ROLE_WLANIDX_BSSID);
                if(pBssidMacTabEntry == NULL)
                {
                    DBGPRINT(RT_DEBUG_TRACE,("%s(), Line:%d, Mac Entry is NULL\n", __FUNCTION__, __LINE__));
                    return;
                }
                
                // Always use HTMIX mode
                WRITE_PHY_CFG_MODE(pAd, &pMacEntry->MaxPhyCfg, MODE_HTMIX);

                /* The channel bandwidth of the TDLS direct link on the base channel(AP's channel)
                 * shall not exceed the channel bandwidth of the BSS to which the TDLS peer STAs are associated.
                 */
                 
                //BW40 will be used only both parties uses BW40
                //They are as the same Offset band as AP. No need to change Upper/Lower band.
                if ((pTDLS->HtCapability.HtCapInfo.ChannelWidth) && (READ_PHY_CFG_BW(pAd, &pBssidMacTabEntry->TxPhyCfg)))
                {
                    WRITE_PHY_CFG_BW(pAd, &pMacEntry->MaxPhyCfg, BW_40);
                    WRITE_PHY_CFG_SHORT_GI(pAd, &pMacEntry->MaxPhyCfg, (pPort->CommonCfg.DesiredHtPhy.ShortGIfor40 & pTDLS->HtCapability.HtCapInfo.ShortGIfor40));
                }
                else
                {
                    WRITE_PHY_CFG_BW(pAd, &pMacEntry->MaxPhyCfg, BW_20);
                    WRITE_PHY_CFG_SHORT_GI(pAd, &pMacEntry->MaxPhyCfg, (pPort->CommonCfg.DesiredHtPhy.ShortGIfor20 & pTDLS->HtCapability.HtCapInfo.ShortGIfor20));
                }


                // ##### Assume it is auto rate, not support fixed rate ###
                
                // I am Initiator !
                WRITE_PHY_CFG_STBC(pAd, &pMacEntry->MaxPhyCfg, (pTDLS->HtCapability.HtCapInfo.RxSTBC & pPort->CommonCfg.DesiredHtPhy.TxSTBC));
                
                // ===>>> copy Responder's HTSETTINGS
                pMacEntry->MpduDensity = pTDLS->HtCapability.HtCapParm.MpduDensity;
                pMacEntry->MaxRAmpduFactor = pTDLS->HtCapability.HtCapParm.MaxRAmpduFactor;
                pMacEntry->MmpsMode = (UCHAR)pTDLS->HtCapability.HtCapInfo.MimoPs;
                pMacEntry->AMsduSize = (UCHAR)pTDLS->HtCapability.HtCapInfo.AMsduSize;
                // <<<===
                WRITE_PHY_CFG_DOUBLE_WORD(pAd, &pMacEntry->TxPhyCfg, READ_PHY_CFG_DOUBLE_WORD(pAd, &pMacEntry->MaxPhyCfg));
                

                if (pPort->CommonCfg.DesiredHtPhy.AmsduEnable && (pPort->CommonCfg.REGBACapability.field.AutoBA == FALSE))
                    CLIENT_STATUS_SET_FLAG(pMacEntry, fCLIENT_STATUS_AMSDU_INUSED);
                if (pTDLS->HtCapability.HtCapInfo.ShortGIfor20 && pPort->CommonCfg.DesiredHtPhy.ShortGIfor20)
                    CLIENT_STATUS_SET_FLAG(pMacEntry, fCLIENT_STATUS_SGI20_CAPABLE);
                if (pTDLS->HtCapability.HtCapInfo.ShortGIfor40 && pPort->CommonCfg.DesiredHtPhy.ShortGIfor40)
                    CLIENT_STATUS_SET_FLAG(pMacEntry, fCLIENT_STATUS_SGI40_CAPABLE);
                if (pTDLS->HtCapability.HtCapInfo.TxSTBC && pPort->CommonCfg.DesiredHtPhy.RxSTBC)
                    CLIENT_STATUS_SET_FLAG(pMacEntry, fCLIENT_STATUS_HT_TX_STBC_CAPABLE);
                if (pTDLS->HtCapability.HtCapInfo.RxSTBC && pPort->CommonCfg.DesiredHtPhy.TxSTBC)
                    CLIENT_STATUS_SET_FLAG(pMacEntry, fCLIENT_STATUS_HT_RX_STBC_CAPABLE);

                // BA parameters
                pMacEntry->BaSizeInUse = (UCHAR)pPort->CommonCfg.BACapability.field.TxBAWinLimit;


                // Write minimum AMPDU size to MAC register
//              if (pMacEntry->MaxRAmpduFactor < MinimumAMPDUSize)
//                  MinimumAMPDUSize = pMacEntry->MaxRAmpduFactor;
                MinimumAMPDUSize = (MAC_VALUE_MAX_LEN_CFG_PSDU8K(pAd) | (MinimumAMPDUSize << 12));

                RTUSBReadMACRegister(pAd, MAX_LEN_CFG, &Value);
                if (MinimumAMPDUSize < Value)
                {
                    RTUSBWriteMACRegister(pAd, MAX_LEN_CFG, MinimumAMPDUSize);
                    DBGPRINT(RT_DEBUG_TRACE, ("TDLS - Write 0x%x to MAC register MAX_LEN_CFG (offset: 0x%x)\n", MinimumAMPDUSize, MAX_LEN_CFG));
                }

                // Save HtCapability
                PlatformMoveMemory(&pMacEntry->HTCapability, &pTDLS->HtCapability, pTDLS->HtCapabilityLen);             
                //pMacEntry->HTPhyMode.word = pMacEntry->MaxHTPhyMode.word;
                //  <<<============================= All HT parameters are ready
                
                // Start BA session
                if ((pPort->CommonCfg.BACapability.field.AutoBA == TRUE) && (pMacEntry->HTCapability.MCSSet[0] != 0))
                {
                    for (j = 0; j <NUM_OF_TID ; j++)
                    {
                        BATableInsertEntry(pAd, pPort, pMacEntry->Aid, 0, 0x1, j, pMacEntry->BaSizeInUse, Originator_SetAutoAdd, FALSE);
                        pMacEntry->TXAutoBAbitmap |= (1<< j);
                    }
                    DBGPRINT(RT_DEBUG_TRACE, ("TDLS - BA BaSizeInUse = %d, TXBAbitmap=%x, RXBAbitmap=%x \n",
                            pMacEntry->BaSizeInUse,
                            pMacEntry->TXBAbitmap,
                            pMacEntry->RXBAbitmap));
                }

                DBGPRINT(RT_DEBUG_TRACE,("TDLS - HT cap found, mode (%d), ch width(%d), MCS[0] = %x, MCS[1] = %x\n",
                        READ_PHY_CFG_MODE(pAd, &pMacEntry->TxPhyCfg), 
                        READ_PHY_CFG_BW(pAd, &pMacEntry->TxPhyCfg), 
                        pMacEntry->HTCapability.MCSSet[0],
                        pMacEntry->HTCapability.MCSSet[1]));

            }
        
            pMacEntry->CurrTxRate = pMacEntry->MaxSupportedRate;


            //
            //Initialize data rate, BA will be setup earlier.
            //
            MlmeSelectTxRateTable(pAd, pMacEntry, &pTable, &TableSize, &InitTxRateIdx);
            AsicUpdateAutoFallBackTable(pAd, pTable, pMacEntry);

            // Select rate in DRS
            pMacEntry->CurrTxRateIndex = TableSize - 1;
            
            if (IS_AGS_RATE_TABLE(pTable))
            {
                pTxRate = (PRTMP_TX_RATE_SWITCH)&pTable[(pMacEntry->CurrTxRateIndex + 1) * SIZE_OF_AGS_RATE_TABLE_ENTRY];
            }
            else
            {
            pTxRate = (PRTMP_TX_RATE_SWITCH)&pTable[(pMacEntry->CurrTxRateIndex + 1)*SIZE_OF_RATE_TABLE_ENTRY];
            }

            MlmeSetTxRate(pAd, pMacEntry, pTxRate, pTable);

            DBGPRINT(RT_DEBUG_TRACE,("TDLS - (SupRateLen+ExtRateLen) = %d, init MCS %d, ClientStatusFlags= 0x%x, AMsduSize=%d, MpduDensity=%d, MaxRAmpduFactor=%d, MmpsMode=%d\n", 
                    pMacEntry->RateLen, 
                    READ_PHY_CFG_MCS(pAd, &pMacEntry->TxPhyCfg), 
                    pMacEntry->ClientStatusFlags, 
                    pMacEntry->AMsduSize, 
                    pMacEntry->MpduDensity, 
                    pMacEntry->MaxRAmpduFactor, 
                    pMacEntry->MmpsMode));

            //
            // Install Peer Key if RSNA Enabled
            //
            if (((pPort->PortCfg.AuthMode == Ndis802_11AuthModeWPA2) || (pPort->PortCfg.AuthMode == Ndis802_11AuthModeWPA2PSK)) &&
                ((pPort->PortCfg.WepStatus == Ralink802_11Encryption2Enabled) || (pPort->PortCfg.WepStatus == Ralink802_11Encryption3Enabled)))
            {   
                // Write to ASIC on-chip table.
                if ( pMacEntry->Aid > 1)
                {
                    RT_SET_DLS_WCID_KEY     TDLS_Cipher;
                    //RT_SET_ASIC_WCID      SetAsicWcid;
                    RT_SET_ASIC_WCID_ATTRI  SetAsicWcidAttri;

                    pMacEntry->AuthMode = pPort->PortCfg.AuthMode;
                    pMacEntry->WepStatus = pPort->PortCfg.WepStatus;

                    // Set WCID Attribute
                    SetAsicWcidAttri.WCID = pMacEntry->Aid;
                    SetAsicWcidAttri.Cipher = pPort->SharedKey[BSS0][pPort->PortCfg.DefaultKeyId].CipherAlg;
                    SetAsicWcidAttri.PortNum = Elem->PortNum;
                    MTEnqueueInternalCmd(pAd, pPort, MT_CMD_SET_ASIC_WCID_CIPHER, &SetAsicWcidAttri, sizeof(RT_SET_ASIC_WCID_ATTRI));
                    
                    // Set Peer Key
                    pMacEntry->PairwiseKey.KeyLen = LEN_TKIP_EK;
                    PlatformMoveMemory(pMacEntry->PairwiseKey.Key, &pTDLS->TPK[16], LEN_TKIP_EK);
                    pMacEntry->PairwiseKey.CipherAlg = pPort->SharedKey[BSS0][pPort->PortCfg.DefaultKeyId].CipherAlg;

                    // Init TxTsc to one based on WiFi WPA specs
                    pMacEntry->PairwiseKey.TxTsc[0] = 1;
                    pMacEntry->PairwiseKey.TxTsc[1] = 0;
                    pMacEntry->PairwiseKey.TxTsc[2] = 0;
                    pMacEntry->PairwiseKey.TxTsc[3] = 0;
                    pMacEntry->PairwiseKey.TxTsc[5] = 0;

                    TDLS_Cipher.WCID = pTDLS->Wcid;         
                    PlatformMoveMemory(&TDLS_Cipher.CipherKey, &pMacEntry->PairwiseKey, sizeof(CIPHER_KEY));        
                    MTEnqueueInternalCmd(pAd, pPort, MT_CMD_SET_DLS_KEY_TABLE, &TDLS_Cipher, sizeof(RT_SET_DLS_WCID_KEY));
                    /*
                    // Set RX WCID 
                    SetAsicWcid.WCID = pTDLS->Wcid;                 
                    PlatformMoveMemory(SetAsicWcid.Addr,pTDLS->MacAddr,6);
                    MTEnqueueInternalCmd(pAd, pPort, MT_CMD_SET_DLS_RX_WCID_TABLE, &SetAsicWcid, sizeof(RT_SET_ASIC_WCID));
                    */
                    pMacEntry->PortSecured = WPA_802_1X_PORT_SECURED;
                    pMacEntry->PrivacyFilter = Ndis802_11PrivFilterAcceptAll;
                }   
                
            }
        }
        else
        {
            DBGPRINT(RT_DEBUG_ERROR,("TDLS - TdlsPeerSetupRspAction() MacTableInsertEntry failed\n"));
        }

    }
    ////////////////////////////////////////////////////////////////////////////

    if ((StatusCode == MLME_SUCCESS) && (pTDLS) && (pTDLS->Status == TDLS_MODE_CONNECTED))
    {
        TdlsSearchEntryDelete(pAd, pTDLS->MacAddr);
        DBGPRINT(RT_DEBUG_TRACE, ("TDLS : Success, delete this search entry \n"));
    }
        
}

/*
    ==========================================================================
    Description:
        
    IRQL = DISPATCH_LEVEL

    ==========================================================================
 */
VOID TdlsPeerSetupConfAction(
    IN PMP_ADAPTER pAd, 
    IN MLME_QUEUE_ELEM *Elem) 
{
    USHORT          StatusCode = MLME_SUCCESS, LocalStatusCode = MLME_SUCCESS;
    BOOLEAN         TimerCancelled; 
    PRT_802_11_TDLS pTDLS = NULL;
    UCHAR           Token;
    UCHAR           PeerAddr[MAC_ADDR_LEN], SA[MAC_ADDR_LEN];
    USHORT          CapabilityInfo;
    EDCA_PARM       EdcaParm;
    INT             LinkId = 0xff;
    UCHAR           RsnLen, FTLen, TILen;
    UCHAR           RsnIe[64], FTIe[128], TIIe[7];
    PMP_PORT      pPort = pAd->PortList[Elem->PortNum];
    
    // Not TDLS Capable, ignore it
    if(!TDLS_ON(pAd))
        return;
    
    // Not BSS mode, ignore it
    if (!INFRA_ON(pPort))
        return;
        
    // Init all kinds of fields within the packet
    PlatformZeroMemory(&EdcaParm, sizeof(EdcaParm));
    PlatformZeroMemory(&CapabilityInfo, sizeof(CapabilityInfo));
    PlatformZeroMemory(RsnIe, sizeof(RsnIe));
    PlatformZeroMemory(FTIe, sizeof(FTIe));
    PlatformZeroMemory(TIIe, sizeof(TIIe));
    PlatformZeroMemory(PeerAddr, MAC_ADDR_LEN);
    PlatformZeroMemory(SA, MAC_ADDR_LEN);


    if (!PeerTdlsSetupConfSanity(
                            pAd,
                            pPort,
                            Elem->Msg, 
                            Elem->MsgLen,
                            &Token,
                            SA,
                            PeerAddr,
                            &CapabilityInfo,
                            &EdcaParm,
                            &StatusCode,
                            &RsnLen,    
                            RsnIe,
                            &FTLen,
                            FTIe,
                            &TILen,
                            TIIe))
    {
        LocalStatusCode = MLME_REQUEST_DECLINED;
    }


    DBGPRINT(RT_DEBUG_TRACE,("TDLS - TdlsPeerSetupConfAction() received a confirm from %02x:%02x:%02x:%02x:%02x:%02x with StatusCode=%d\n", PeerAddr[0], PeerAddr[1], PeerAddr[2], PeerAddr[3], PeerAddr[4], PeerAddr[5], StatusCode));

    // Drop not within my TDLS Table that created before !
    LinkId = TdlsSearchLinkId(pAd, PeerAddr);
    if ((LinkId == -1) || (LinkId == MAX_NUM_OF_TDLS_ENTRY) || MAC_ADDR_EQUAL(PeerAddr, ZERO_MAC_ADDR))
    {
        DBGPRINT(RT_DEBUG_ERROR,("TDLS - TdlsPeerSetupConfAction() invalid entry\n"));
        return;
    }

    // Point to the current Link ID
    pTDLS = (PRT_802_11_TDLS)&pAd->StaCfg.TDLSEntry[LinkId];
    // Cancel the timer since the received packet to me.
    PlatformCancelTimer(&pTDLS->Timer, &TimerCancelled);



    // Received a error code from the Peer TDLS.
    // Let's terminate the setup procedure right now.
    if (StatusCode != MLME_SUCCESS)
    {
        pTDLS->Status = TDLS_MODE_NONE;
        pTDLS->Valid    = FALSE;
        
        DBGPRINT(RT_DEBUG_ERROR,("TDLS - TdlsPeerSetupConfAction() received a failed StatusCode, terminate the setup procedure #%d\n", LinkId));
        return;
    }
    else
        StatusCode = LocalStatusCode;


    // 
    // Validate the content on Setup Confirm Frame
    //
    while (StatusCode == MLME_SUCCESS)
    {       
        // Invalid TDLS State
        if (pTDLS->Status == TDLS_MODE_CONNECTED)
        {
            DBGPRINT(RT_DEBUG_ERROR,("TDLS - TdlsPeerSetupConfAction() protocol handshake has done, ignore this confirm packet \n"));
            return;
        }
        if (pTDLS->Status != TDLS_MODE_WAIT_ADD)
        {
            DBGPRINT(RT_DEBUG_ERROR,("TDLS - TdlsPeerSetupConfAction() Not in TDLS_MODE_WAIT_ADD STATE\n"));        
            StatusCode =  MLME_REQUEST_DECLINED;
            break;
        }

        // Is the same Dialog Token?
        if (pTDLS->Token != Token)
        {
            DBGPRINT(RT_DEBUG_ERROR,("TDLS - TdlsPeerSetupConfAction() Not match with Dialig Token \n"));
            StatusCode =  MLME_REQUEST_DECLINED;
            break;
        }
        
        // Process TPK Handshake Message 3 here!
        if ((pPort->PortCfg.AuthMode == Ndis802_11AuthModeWPA2) || (pPort->PortCfg.AuthMode == Ndis802_11AuthModeWPA2PSK))
        {       
            USHORT Result;

            // RSNIE (7.3.2.25), FTIE (7.3.2.48), Timeout Interval (7.3.2.49)
            Result = TdlsTPKMsg3Process(pAd, pPort, pTDLS, RsnIe, RsnLen, FTIe, FTLen, TIIe, TILen);
            if (Result != MLME_SUCCESS)
            {
                DBGPRINT(RT_DEBUG_ERROR,("TDLS - TdlsTPKMsg3Process() Failed, reason=%d \n", Result));
                StatusCode = Result;
                break;
            }
        }

        // Update parameters
        if (StatusCode == MLME_SUCCESS)
        {
            // I am Responder.
            pTDLS->bInitiator = FALSE;
            
            // Copy EDCA Parameters
            if (((pPort->CommonCfg.PhyMode >= PHY_11ABGN_MIXED) || (pPort->CommonCfg.bWmmCapable)) && (EdcaParm.bValid))
                PlatformMoveMemory(&pTDLS->IrEdcaParm, &EdcaParm, sizeof(EDCA_PARM));   
            else
                PlatformZeroMemory(&pTDLS->IrEdcaParm, sizeof(EDCA_PARM));

            // Copy TPK related information
            if (((pPort->PortCfg.AuthMode == Ndis802_11AuthModeWPA2) || (pPort->PortCfg.AuthMode == Ndis802_11AuthModeWPA2PSK)) &&
                ((pPort->PortCfg.WepStatus == Ralink802_11Encryption2Enabled) || (pPort->PortCfg.WepStatus == Ralink802_11Encryption3Enabled)))
            {
                // SNonce, Key lifetime
            }

        }

        break;
        
    }


    if (StatusCode == MLME_SUCCESS)
    {
        // Procedure in Succeed
        // ==>> State Change
        pTDLS->Status = TDLS_MODE_CONNECTED;

        DBGPRINT(RT_DEBUG_TRACE, ("*************** TDLS Setup Procedure - Success ***************\n\n"));
    }
    else
    {
        // TDLS setup procedure failed. Let's terminate the setup procedure.
        pTDLS->Status = TDLS_MODE_NONE;
        pTDLS->Valid = FALSE;
        DBGPRINT(RT_DEBUG_ERROR,("TDLS - TdlsPeerSetupConfAction() process failed with StatusCode=%d \n", StatusCode));
    }


    ////////////////////////////////////////////////////////////////////////////
    // Insert into mac table
    //
    if ((StatusCode == MLME_SUCCESS) && (pTDLS) && (pTDLS->Status == TDLS_MODE_CONNECTED))
    {
        UCHAR           MaxSupportedRateIn500Kbps = 0, MinSupportedRateIn500Kbps = 108; 
        UCHAR           MinSupportedRate= RATE_1, MaxSupportedRate= RATE_11;
        UCHAR           idx, j;
        PMAC_TABLE_ENTRY    pMacEntry = NULL;
        PUCHAR              pTable;
        UCHAR               TableSize;
        UCHAR               InitTxRateIdx;
        PRTMP_TX_RATE_SWITCH    pTxRate;
        ULONG               MinimumAMPDUSize = 1; //DInit value is 1 at MACRegTable 
        ULONG               Value;

        // look up the existing table
        pMacEntry = MacTableLookup(pAd,  pPort, pTDLS->MacAddr);
        // not exist, add it
        if (!pMacEntry)
        {
            pMacEntry = MacTableInsertEntry(pAd, pPort, pTDLS->MacAddr, FALSE);
        }

        if (pMacEntry)
        {
            //This has to be done, different with windows driver.

            //Keep wcid
            pTDLS->Wcid = (UCHAR)pMacEntry->Aid;
            DBGPRINT(RT_DEBUG_TRACE, ("TDLS - Insert/Update MacEntry (Aid= %d)\n", pMacEntry->Aid));

            //Set state or DRS will not work
            pMacEntry->Sst = SST_ASSOC;

            pMacEntry->ClientStatusFlags = 0;

            // Set WMM capability
            if (((pPort->CommonCfg.PhyMode >= PHY_11ABGN_MIXED) || (pPort->CommonCfg.bWmmCapable)) && (pTDLS->bWmmCapable))
            {
                CLIENT_STATUS_SET_FLAG(pMacEntry, fCLIENT_STATUS_WMM_CAPABLE);
                DBGPRINT(RT_DEBUG_TRACE, ("TDLS -  WMM Capable\n"));

                // Set EDCA to Asic in non-QBSS
                if (!OPSTATUS_TEST_FLAG(pPort, fOP_STATUS_WMM_INUSED) && pTDLS->IrEdcaParm.bValid)
                {
                    MtAsicSetEdcaParm(pAd, pPort, &pTDLS->IrEdcaParm, FALSE);
                }
            }
            else
            {
                CLIENT_STATUS_CLEAR_FLAG(pMacEntry, fCLIENT_STATUS_WMM_CAPABLE);
            }

            //
            // Rate Settings >>>>>>>
            //
            for (idx=0; idx<pTDLS->SupRateLen; idx++)
            {
                if (MaxSupportedRateIn500Kbps < (pTDLS->SupRate[idx] & 0x7f)) 
                    MaxSupportedRateIn500Kbps = pTDLS->SupRate[idx] & 0x7f;
                if (MinSupportedRateIn500Kbps > (pTDLS->SupRate[idx] & 0x7f)) 
                    MinSupportedRateIn500Kbps = pTDLS->SupRate[idx] & 0x7f;

            }
            for (idx=0; idx<pTDLS->ExtRateLen; idx++)
            {
                if (MaxSupportedRateIn500Kbps < (pTDLS->ExtRate[idx] & 0x7f)) 
                    MaxSupportedRateIn500Kbps = pTDLS->ExtRate[idx] & 0x7f;
                if (MinSupportedRateIn500Kbps > (pTDLS->ExtRate[idx] & 0x7f)) 
                    MinSupportedRateIn500Kbps = pTDLS->ExtRate[idx] & 0x7f;

            }

            switch (MaxSupportedRateIn500Kbps)
            {
                case 108: MaxSupportedRate = RATE_54;   break;
                case 96:  MaxSupportedRate = RATE_48;   break;
                case 72:  MaxSupportedRate = RATE_36;   break;
                case 48:  MaxSupportedRate = RATE_24;   break;
                case 36:  MaxSupportedRate = RATE_18;   break;
                case 24:  MaxSupportedRate = RATE_12;   break;
                case 18:  MaxSupportedRate = RATE_9;    break;
                case 12:  MaxSupportedRate = RATE_6;    break;
                case 22:  MaxSupportedRate = RATE_11;   break;
                case 11:  MaxSupportedRate = RATE_5_5;  break;
                case 4:   MaxSupportedRate = RATE_2;    break;
                case 2:   MaxSupportedRate = RATE_1;    break;
                default:  MaxSupportedRate = RATE_11;   break;
            }
            switch (MinSupportedRateIn500Kbps)
            {
                case 108: MinSupportedRate = RATE_54;   break;
                case 96:  MinSupportedRate = RATE_48;   break;
                case 72:  MinSupportedRate = RATE_36;   break;
                case 48:  MinSupportedRate = RATE_24;   break;
                case 36:  MinSupportedRate = RATE_18;   break;
                case 24:  MinSupportedRate = RATE_12;   break;
                case 18:  MinSupportedRate = RATE_9;    break;
                case 12:  MinSupportedRate = RATE_6;    break;
                case 22:  MinSupportedRate = RATE_11;   break;
                case 11:  MinSupportedRate = RATE_5_5;  break;
                case 4:   MinSupportedRate = RATE_2;    break;
                case 2:   MinSupportedRate = RATE_1;    break;
                default:  MinSupportedRate = RATE_11;   break;
            }
        
            if ((MaxSupportedRate < RATE_6) || (pPort->CommonCfg.PhyMode == PHY_11B))
            {
                pMacEntry->RateLen = 4;
                if (MaxSupportedRate >= RATE_6)
                    MaxSupportedRate = RATE_11;
            }
            else if ((MinSupportedRate >= RATE_6) && (MaxSupportedRate <= RATE_54))
            {
                pMacEntry->RateLen = 8;
            }
            else if (((pPort->CommonCfg.SupRateLen + pPort->CommonCfg.ExtRateLen) == 8) && (MinSupportedRate < RATE_6))
            {
                pMacEntry->RateLen = 8;
                MinSupportedRate = RATE_6;
            }
            else
            {
                pMacEntry->RateLen = pTDLS->SupRateLen + pTDLS->ExtRateLen;
            }

            WRITE_PHY_CFG_DOUBLE_WORD(pAd, &pMacEntry->MaxPhyCfg, 0);
            WRITE_PHY_CFG_DOUBLE_WORD(pAd, &pMacEntry->MinPhyCfg, 0);
            WRITE_PHY_CFG_DOUBLE_WORD(pAd, &pMacEntry->TxPhyCfg, 0);
            pMacEntry->MaxSupportedRate = MaxSupportedRate;
            
            // CCK Rate
            if (pMacEntry->MaxSupportedRate < RATE_FIRST_OFDM_RATE)
            {
                WRITE_PHY_CFG_MODE(pAd, &pMacEntry->MaxPhyCfg, MODE_CCK);
                WRITE_PHY_CFG_MCS(pAd, &pMacEntry->MaxPhyCfg, pMacEntry->MaxSupportedRate);
                WRITE_PHY_CFG_MODE(pAd, &pMacEntry->MinPhyCfg, MODE_CCK);
                WRITE_PHY_CFG_MCS(pAd, &pMacEntry->MinPhyCfg, MinSupportedRate);
                DBGPRINT(RT_DEBUG_TRACE, ("TDLS -  CCK, MinSupportedRate=%d\n", MinSupportedRate));
            }
            else // OFDM Rate
            {
                WRITE_PHY_CFG_MODE(pAd, &pMacEntry->MaxPhyCfg, MODE_OFDM);
                WRITE_PHY_CFG_MCS(pAd, &pMacEntry->MaxPhyCfg, OfdmRateToRxwiMCS[pMacEntry->MaxSupportedRate]);
                WRITE_PHY_CFG_MODE(pAd, &pMacEntry->MinPhyCfg, MODE_OFDM);
                WRITE_PHY_CFG_MCS(pAd, &pMacEntry->MinPhyCfg, OfdmRateToRxwiMCS[MinSupportedRate]);
                DBGPRINT(RT_DEBUG_TRACE, ("TDLS -  OFDM, MinSupportedRate=%d\n", MinSupportedRate));                
            }
            
            pMacEntry->CapabilityInfo = CapabilityInfo;

            //
            //HT will be used only when both parties are HT capable
            //
            if ((pTDLS->HtCapabilityLen > 0) && (pPort->CommonCfg.PhyMode >= PHY_11ABGN_MIXED))
            {
                PMAC_TABLE_ENTRY pBssidMacTabEntry = MlmeSyncMacTabMatchedRoleType(pPort, ROLE_WLANIDX_BSSID);
                if(pBssidMacTabEntry == NULL)
                {
                    DBGPRINT(RT_DEBUG_TRACE,("%s(), Line:%d, Mac Entry is NULL\n", __FUNCTION__, __LINE__));
                    return;
                }
                
                // Always use HTMIX mode
                WRITE_PHY_CFG_MODE(pAd, &pMacEntry->MaxPhyCfg, MODE_HTMIX);


                /* The channel bandwidth of the TDLS direct link on the base channel(AP's channel)
                 * shall not exceed the channel bandwidth of the BSS to which the TDLS peer STAs are associated.
                 */

                //BW40 will be used only both parties uses BW40
                //They are as the same Offset band as AP. No need to change Upper/Lower band.
                if ((pTDLS->HtCapability.HtCapInfo.ChannelWidth) && 
                     ((READ_PHY_CFG_BW(pAd, &pBssidMacTabEntry->TxPhyCfg) == BW_40) || 
                       (READ_PHY_CFG_BW(pAd, &pBssidMacTabEntry->TxPhyCfg) == BW_80)))
                {
                    WRITE_PHY_CFG_BW(pAd, &pMacEntry->MaxPhyCfg, BW_40);
                    WRITE_PHY_CFG_SHORT_GI(pAd, &pMacEntry->MaxPhyCfg, (pPort->CommonCfg.DesiredHtPhy.ShortGIfor40 & pTDLS->HtCapability.HtCapInfo.ShortGIfor40));                
                }
                else
                {
                    WRITE_PHY_CFG_BW(pAd, &pMacEntry->MaxPhyCfg, BW_20);
                    WRITE_PHY_CFG_SHORT_GI(pAd, &pMacEntry->MaxPhyCfg, (pPort->CommonCfg.DesiredHtPhy.ShortGIfor20 & pTDLS->HtCapability.HtCapInfo.ShortGIfor20));
                }


                // ##### Assume it is auto rate, not support fixed rate ###
                
                // I am Responder !
                WRITE_PHY_CFG_STBC(pAd, &pMacEntry->MaxPhyCfg, (pTDLS->HtCapability.HtCapInfo.RxSTBC & pPort->CommonCfg.DesiredHtPhy.TxSTBC));

                
                // ===>>> copy Initiator's HTSETTINGS
                pMacEntry->MpduDensity = pTDLS->HtCapability.HtCapParm.MpduDensity;
                pMacEntry->MaxRAmpduFactor = pTDLS->HtCapability.HtCapParm.MaxRAmpduFactor;
                pMacEntry->MmpsMode = (UCHAR)pTDLS->HtCapability.HtCapInfo.MimoPs;
                pMacEntry->AMsduSize = (UCHAR)pTDLS->HtCapability.HtCapInfo.AMsduSize;
                // <<<===
                
                WRITE_PHY_CFG_DOUBLE_WORD(pAd, &pMacEntry->TxPhyCfg, READ_PHY_CFG_DOUBLE_WORD(pAd, &pMacEntry->MaxPhyCfg));


                if (pPort->CommonCfg.DesiredHtPhy.AmsduEnable && (pPort->CommonCfg.REGBACapability.field.AutoBA == FALSE))
                    CLIENT_STATUS_SET_FLAG(pMacEntry, fCLIENT_STATUS_AMSDU_INUSED);
                if (pTDLS->HtCapability.HtCapInfo.ShortGIfor20 && pPort->CommonCfg.DesiredHtPhy.ShortGIfor20)
                    CLIENT_STATUS_SET_FLAG(pMacEntry, fCLIENT_STATUS_SGI20_CAPABLE);
                if (pTDLS->HtCapability.HtCapInfo.ShortGIfor40 && pPort->CommonCfg.DesiredHtPhy.ShortGIfor40)
                    CLIENT_STATUS_SET_FLAG(pMacEntry, fCLIENT_STATUS_SGI40_CAPABLE);
                if (pTDLS->HtCapability.HtCapInfo.TxSTBC && pPort->CommonCfg.DesiredHtPhy.RxSTBC)
                    CLIENT_STATUS_SET_FLAG(pMacEntry, fCLIENT_STATUS_HT_TX_STBC_CAPABLE);
                if (pTDLS->HtCapability.HtCapInfo.RxSTBC && pPort->CommonCfg.DesiredHtPhy.TxSTBC)
                    CLIENT_STATUS_SET_FLAG(pMacEntry, fCLIENT_STATUS_HT_RX_STBC_CAPABLE);

                // BA parameters
                pMacEntry->BaSizeInUse = (UCHAR)pPort->CommonCfg.BACapability.field.TxBAWinLimit;
                

                // Write minimum AMPDU size to MAC register
//              if (pMacEntry->MaxRAmpduFactor < MinimumAMPDUSize)
                    MinimumAMPDUSize = pMacEntry->MaxRAmpduFactor;
                MinimumAMPDUSize = (0x000E0fff | (MinimumAMPDUSize << 12));
                RTUSBReadMACRegister(pAd, MAX_LEN_CFG, &Value);
                if (MinimumAMPDUSize < Value)
                {
                    RTUSBWriteMACRegister(pAd, MAX_LEN_CFG, MinimumAMPDUSize);
                    DBGPRINT(RT_DEBUG_TRACE, ("TDLS - Write 0x%x to MAC register MAX_LEN_CFG (offset: 0x%x)\n", MinimumAMPDUSize, MAX_LEN_CFG));
                }

                // Save HtCapability
                PlatformMoveMemory(&pMacEntry->HTCapability, &pTDLS->HtCapability, pTDLS->HtCapabilityLen);             
                //pMacEntry->HTPhyMode.word = pMacEntry->MaxHTPhyMode.word;
                //  <<<============================= All HT parameters are ready
                
                // Start BA session
                if ((pPort->CommonCfg.BACapability.field.AutoBA == TRUE) && (pMacEntry->HTCapability.MCSSet[0] != 0))
                {
                    for (j = 0; j <NUM_OF_TID ; j++)
                    {
                        BATableInsertEntry(pAd, pPort, pMacEntry->Aid, 0, 0x1, j, pMacEntry->BaSizeInUse, Originator_SetAutoAdd, FALSE);
                        pMacEntry->TXAutoBAbitmap |= (1<< j);
                    }
                    DBGPRINT(RT_DEBUG_TRACE, ("TDLS - BA BaSizeInUse = %d, TXBAbitmap=%x, RXBAbitmap=%x \n",
                            pMacEntry->BaSizeInUse,
                            pMacEntry->TXBAbitmap,
                            pMacEntry->RXBAbitmap));
                }

                DBGPRINT(RT_DEBUG_TRACE,("TDLS - HT cap found, mode (%d), ch width(%d), MCS[0] = %x, MCS[1] = %x\n",
                        READ_PHY_CFG_MODE(pAd, &pMacEntry->TxPhyCfg), 
                        READ_PHY_CFG_BW(pAd, &pMacEntry->TxPhyCfg), 
                        pMacEntry->HTCapability.MCSSet[0],
                        pMacEntry->HTCapability.MCSSet[1]));

            }
        
            pMacEntry->CurrTxRate = pMacEntry->MaxSupportedRate;


            //
            //Initialize data rate, BA will be setup earlier.
            //
            MlmeSelectTxRateTable(pAd, pMacEntry, &pTable, &TableSize, &InitTxRateIdx);
            AsicUpdateAutoFallBackTable(pAd, pTable, pMacEntry);

            // Select rate in DRS
            pMacEntry->CurrTxRateIndex = TableSize - 1;
            
            if (IS_AGS_RATE_TABLE(pTable))
            {
                pTxRate = (PRTMP_TX_RATE_SWITCH)&pTable[(pMacEntry->CurrTxRateIndex + 1) * SIZE_OF_AGS_RATE_TABLE_ENTRY];
            }
            else
            {
                pTxRate = (PRTMP_TX_RATE_SWITCH)&pTable[(pMacEntry->CurrTxRateIndex + 1) * SIZE_OF_RATE_TABLE_ENTRY];
            }

            MlmeSetTxRate(pAd, pMacEntry, pTxRate, pTable);

            DBGPRINT(RT_DEBUG_TRACE,("TDLS - (SupRateLen+ExtRateLen) = %d, init MCS %d, ClientStatusFlags= 0x%x, AMsduSize=%d, MpduDensity=%d, MaxRAmpduFactor=%d, MmpsMode=%d\n", 
                    pMacEntry->RateLen, 
                    READ_PHY_CFG_MCS(pAd, &pMacEntry->TxPhyCfg), 
                    pMacEntry->ClientStatusFlags, 
                    pMacEntry->AMsduSize, 
                    pMacEntry->MpduDensity, 
                    pMacEntry->MaxRAmpduFactor, 
                    pMacEntry->MmpsMode));

            //
            // Install Peer Key if RSNA Enabled
            //
            if (((pPort->PortCfg.AuthMode == Ndis802_11AuthModeWPA2) || (pPort->PortCfg.AuthMode == Ndis802_11AuthModeWPA2PSK)) &&
                ((pPort->PortCfg.WepStatus == Ralink802_11Encryption2Enabled) || (pPort->PortCfg.WepStatus == Ralink802_11Encryption3Enabled)))
            {   
                // Write to ASIC on-chip table.
                if ( pMacEntry->Aid > 1)
                {
                    RT_SET_DLS_WCID_KEY     TDLS_Cipher;
                    //RT_SET_ASIC_WCID      SetAsicWcid;
                    RT_SET_ASIC_WCID_ATTRI  SetAsicWcidAttri;

                    pMacEntry->AuthMode = pPort->PortCfg.AuthMode;
                    pMacEntry->WepStatus = pPort->PortCfg.WepStatus;

                    // Set WCID Attribute
                    SetAsicWcidAttri.WCID = pMacEntry->Aid;
                    SetAsicWcidAttri.Cipher = pPort->SharedKey[BSS0][pPort->PortCfg.DefaultKeyId].CipherAlg;
                    SetAsicWcidAttri.PortNum = Elem->PortNum;
                
                    MTEnqueueInternalCmd(pAd, pPort, MT_CMD_SET_ASIC_WCID_CIPHER, &SetAsicWcidAttri, sizeof(RT_SET_ASIC_WCID_ATTRI));
                    
                    // Set Peer Key
                    pMacEntry->PairwiseKey.KeyLen = LEN_TKIP_EK;
                    PlatformMoveMemory(pMacEntry->PairwiseKey.Key, &pTDLS->TPK[16], LEN_TKIP_EK);
                    pMacEntry->PairwiseKey.CipherAlg = pPort->SharedKey[BSS0][pPort->PortCfg.DefaultKeyId].CipherAlg;

                    // Init TxTsc to one based on WiFi WPA specs
                    pMacEntry->PairwiseKey.TxTsc[0] = 1;
                    pMacEntry->PairwiseKey.TxTsc[1] = 0;
                    pMacEntry->PairwiseKey.TxTsc[2] = 0;
                    pMacEntry->PairwiseKey.TxTsc[3] = 0;
                    pMacEntry->PairwiseKey.TxTsc[5] = 0;

                    TDLS_Cipher.WCID = pTDLS->Wcid;         
                    PlatformMoveMemory(&TDLS_Cipher.CipherKey, &pMacEntry->PairwiseKey, sizeof(CIPHER_KEY));        
                    MTEnqueueInternalCmd(pAd, pPort, MT_CMD_SET_DLS_KEY_TABLE, &TDLS_Cipher, sizeof(RT_SET_DLS_WCID_KEY));
                    /*
                    // Set RX WCID 
                    SetAsicWcid.WCID = pTDLS->Wcid;                 
                    PlatformMoveMemory(SetAsicWcid.Addr,pTDLS->MacAddr,6);
                    MTEnqueueInternalCmd(pAd, pPort, MT_CMD_SET_DLS_RX_WCID_TABLE, &SetAsicWcid, sizeof(RT_SET_ASIC_WCID));
                    */
                    pMacEntry->PortSecured = WPA_802_1X_PORT_SECURED;
                    pMacEntry->PrivacyFilter = Ndis802_11PrivFilterAcceptAll;
                }   
                
            }
        }
        else
        {
            DBGPRINT(RT_DEBUG_ERROR,("TDLS - TdlsPeerSetupConfAction() MacTableInsertEntry failed\n"));
        }

    }
    ////////////////////////////////////////////////////////////////////////////

    if ((StatusCode == MLME_SUCCESS) && (pTDLS) && (pTDLS->Status == TDLS_MODE_CONNECTED))
    {
        TdlsSearchEntryDelete(pAd, pTDLS->MacAddr);
        DBGPRINT(RT_DEBUG_TRACE, ("TDLS : Success, delete this search entry \n"));
    }

}

/*
==========================================================================
    Description:
        
    IRQL = PASSIVE_LEVEL
==========================================================================
*/
VOID TdlsTearDownAction(
    IN PMP_ADAPTER pAd, 
    IN MLME_QUEUE_ELEM *Elem) 
{
    PUCHAR          pOutBuffer = NULL;
    NDIS_STATUS     NStatus;
    ULONG           FrameLen = 0;
    USHORT          ReasonCode;
    HEADER_802_11   Header80211;
    TDLS_ACTION_HEADER  TdlsActHdr;
    PRT_802_11_TDLS pTDLS = NULL;
    BOOLEAN         TimerCancelled;
    UCHAR           LinkIdentifier[20];
    UCHAR           LengthQosPAD;
    PMP_PORT      pPort = pAd->PortList[Elem->PortNum];
    
    if(!MlmeTdlsReqSanity(pAd, Elem->Msg, Elem->MsgLen, &pTDLS, &ReasonCode))
        return;

    DBGPRINT(RT_DEBUG_TRACE,("TDLS - TdlsTearDownAction() with ReasonCode=%d \n", ReasonCode));
    
    NStatus = MlmeAllocateMemory(pAd, &pOutBuffer);  //Get an unused nonpaged memory
    if (NStatus != NDIS_STATUS_SUCCESS) 
    {
        DBGPRINT(RT_DEBUG_ERROR,("TDLS - TdlsTearDownAction() allocate memory failed \n"));
        return;
    }
    
    // Qos will be add in HardTx
    LengthQosPAD = 0;

    // 1. Make 802.11 header for Vista, where Addr1: DA, Addr2: SA, Addr3: Bssid
    // Sequence Number and WEP bit will be updated later
    PlatformZeroMemory(&Header80211, sizeof(HEADER_802_11));
    Header80211.FC.Type = BTYPE_DATA;   

    // More MSDUs are discarded on the direct link. 
    // In this case, the TDLS Teardown frame shall be sent through the AP.
    if (ReasonCode == REASON_TDLS_UNACCEPTABLE_FRAME_LOSS)
    {
        COPY_MAC_ADDR(Header80211.Addr1, pPort->PortCfg.Bssid); 
        COPY_MAC_ADDR(Header80211.Addr2, pAd->HwCfg.CurrentAddress);
        COPY_MAC_ADDR(Header80211.Addr3, pTDLS->MacAddr);
        Header80211.FC.ToDs = 1; // through AP  
    }
    else // direct to Peer
    {
        COPY_MAC_ADDR(Header80211.Addr1, pTDLS->MacAddr);
        COPY_MAC_ADDR(Header80211.Addr2, pAd->HwCfg.CurrentAddress);
        COPY_MAC_ADDR(Header80211.Addr3, pPort->PortCfg.Bssid); 
    }

    // 2. Prepare LLC_SNAP and TDLS Action header
    TdlsActHdr.Protocol = PROTO_NAME_TDLS;
    TdlsActHdr.Category = CATEGORY_TDLS;
    TdlsActHdr.Action   = ACTION_TDLS_TEARDOWN;

    // 3. Build basic frame first

    // Construct LinkIdentifier (IE + Length + BSSID + Initiator MAC + Responder MAC)
    PlatformZeroMemory(LinkIdentifier, 20);
    LinkIdentifier[0] = IE_TDLS_LINK_ID;
    LinkIdentifier[1] = 18;
    PlatformMoveMemory(&LinkIdentifier[2], pPort->PortCfg.Bssid, 6);
    if (pTDLS->bInitiator)
    {
        PlatformMoveMemory(&LinkIdentifier[8], pAd->HwCfg.CurrentAddress, 6);
        PlatformMoveMemory(&LinkIdentifier[14], pTDLS->MacAddr, 6);
    }
    else
    {
        PlatformMoveMemory(&LinkIdentifier[8], pTDLS->MacAddr, 6);
        PlatformMoveMemory(&LinkIdentifier[14], pAd->HwCfg.CurrentAddress, 6);
    }
    

    MakeOutgoingFrame(pOutBuffer,               &FrameLen,
                    sizeof(HEADER_802_11),      &Header80211,
                    LENGTH_802_1_H,             TDLS_LLC_SNAP,
                    sizeof(TDLS_ACTION_HEADER), &TdlsActHdr,
                    2,                          &ReasonCode,
                    sizeof(LinkIdentifier),     LinkIdentifier,                                             
                    END_OF_ARGS);

    // FTIE includes if RSNA Enabled
    if (((pPort->PortCfg.AuthMode == Ndis802_11AuthModeWPA2) || (pPort->PortCfg.AuthMode == Ndis802_11AuthModeWPA2PSK)) &&
        ((pPort->PortCfg.WepStatus == Ralink802_11Encryption2Enabled) || (pPort->PortCfg.WepStatus == Ralink802_11Encryption3Enabled)))
    {   
        UCHAR               FTIe[128];  
        FT_IE               *ft = NULL;
        UCHAR               content[256];
        ULONG               c_len = 0;
        ULONG               tmp_len = 0;
        UCHAR               seq = 4;
        UCHAR               mic[16];
        
        // FTIE (7.3.2.48)
        // The contents of FTIE in the TDLS Teardown frame shall be the same as that included
        // in the TPK Handshake Message3 with the exception of the MIC field.

        // Construct FTIE (IE + Length + MIC Control + MIC + ANonce + SNonce)
            
        // point to the element of IE
        PlatformZeroMemory(FTIe, sizeof(FTIe));
        FTIe[0] = IE_FAST_BSS_TRANSITION;
        FTIe[1] = sizeof(FT_IE) - 256;
        ft = (PFT_IE)&FTIe[2];  
        PlatformMoveMemory(ft->ANonce, pTDLS->ANonce, 32);
        PlatformMoveMemory(ft->SNonce, pTDLS->SNonce, 32);

        ////////////////////////////////////////////////////////////////////////
        // The MIC field of FTIE shall be calculated on the concatenation, in the following order, of
        // 1. Link Identifier (20 bytes)
        // 2. Reason Code (2 bytes)
        // 3. Dialog token (1 byte)
        // 4. Transaction Sequence = 4 (1 byte)
        // 5. FTIE with the MIC field of FTIE set to zero (84 bytes)    
        
        /* concatenate Link Identifier, Reason Code, Dialog token, Transaction Sequence */
        MakeOutgoingFrame(content,                  &tmp_len,
                        sizeof(LinkIdentifier),     LinkIdentifier, 
                        2,                          &ReasonCode,
                        1,                          &pTDLS->Token,
                        1,                          &seq,
                        END_OF_ARGS);
        c_len += tmp_len;                   

        /* concatenate FTIE */
        MakeOutgoingFrame(content + c_len,      &tmp_len,
                            FTIe[1] + 2,        FTIe,  
                        END_OF_ARGS);
        c_len += tmp_len;                   
        
        /* Calculate MIC */
        PlatformZeroMemory(mic, sizeof(mic));
        AES_128_CMAC(pTDLS->TPK, content, c_len, mic);
        
        /* Fill Mic to ft struct */
        PlatformMoveMemory(ft->MIC, mic, 16);
        ////////////////////////////////////////////////////////////////////////

#if 0   // FT_InsertXXX shall be defined to Dot11R  
        // Insert FT_IE to outgoing frame
        FT_InsertFTIE(
                pAd, 
                pOutBuffer + FrameLen, 
                &FrameLen, 
                FTIe[1], 
                ft->MICControl, 
                ft->MIC, 
                ft->ANonce, 
                ft->SNonce);
#else
    {
        UCHAR   ValueIE;
        UINT16  MICCtrBuf;
        UCHAR   Length;
        ULONG   tmp;
        
        // Insert FT_IE to outgoing frame
        ValueIE = IE_FAST_BSS_TRANSITION;
        Length =  sizeof(FT_IE) - 256;
        MICCtrBuf = cpu2le16(ft->MICControl.word);
        
        MakeOutgoingFrame(  pOutBuffer + FrameLen,  &tmp,
                            1,                      &ValueIE,
                            1,                      &Length,
                            2,                      (PUCHAR)&MICCtrBuf,
                            16,                     (PUCHAR)&ft->MIC,
                            32,                     (PUCHAR)ft->ANonce,
                            32,                     (PUCHAR)ft->SNonce,
                            END_OF_ARGS);
        FrameLen += tmp;
    }
#endif
        
    }
    
    // Send a packet via HardTransmit
    TdlsSendPacket(pAd, pOutBuffer, FrameLen);
    MlmeFreeMemory(pAd, pOutBuffer);
    PlatformCancelTimer(&pTDLS->Timer, &TimerCancelled);


    // clear this tdls entry
    if(pTDLS)
    {
        MtAsicDelWcidTab(pAd, pTDLS->Wcid);
        AsicRemoveKeyEntry(pAd, pTDLS->Wcid, 0, TRUE);
        TdlsDeleteMacTableEntry(pAd, pPort, pTDLS->MacAddr);
    }

}

/*
==========================================================================
    Description:

    IRQL = PASSIVE_LEVEL
==========================================================================
*/
VOID TdlsPeerTearDownAction(
    IN PMP_ADAPTER pAd, 
    IN MLME_QUEUE_ELEM *Elem) 
{
    UCHAR               PeerAddr[MAC_ADDR_LEN], SA[MAC_ADDR_LEN];
    USHORT              ReasonCode;
    PRT_802_11_TDLS     pTDLS = NULL;
    INT                 LinkId = 0xff;
    BOOLEAN             IsInitator;
    BOOLEAN             TimerCancelled;
    UCHAR               FTLen;
    UCHAR               FTIe[128];
    PMP_PORT          pPort = pAd->PortList[Elem->PortNum];

    // Not TDLS Capable, ignore it
    if(!TDLS_ON(pAd))
        return;

    if (!INFRA_ON(pPort))
        return;

    // Init FTIe
    PlatformZeroMemory(FTIe, sizeof(FTIe));
    PlatformZeroMemory(PeerAddr, MAC_ADDR_LEN);
    PlatformZeroMemory(SA, MAC_ADDR_LEN);
    
    if (!PeerTdlsTearDownSanity(pAd, pPort, Elem->Msg, Elem->MsgLen, SA, PeerAddr, &IsInitator, &ReasonCode, &FTLen, FTIe))
        return;

    DBGPRINT(RT_DEBUG_TRACE,("TDLS - TdlsPeerTearDownAction() from %02x:%02x:%02x:%02x:%02x:%02x with ReasonCode=%d\n", SA[0], SA[1], SA[2], SA[3], SA[4], SA[5], ReasonCode));

    // Drop not within my TDLS Table that created before !
    LinkId = TdlsSearchLinkId(pAd, PeerAddr);
    if ((LinkId == -1) || (LinkId == MAX_NUM_OF_TDLS_ENTRY) || MAC_ADDR_EQUAL(PeerAddr, ZERO_MAC_ADDR))
    {
        DBGPRINT(RT_DEBUG_ERROR,("TDLS - TdlsPeerTearDownAction() invalid entry\n"));
        return;
    }
    
    // Point to the current Link ID
    pTDLS = (PRT_802_11_TDLS)&pAd->StaCfg.TDLSEntry[LinkId];
    // Cancel the timer since the received packet to me.
    PlatformCancelTimer(&pTDLS->Timer, &TimerCancelled);


    // Drop mismatched identifier.
    if (pTDLS->bInitiator != IsInitator)
        return;

    // Process FTIE here!
    if (((pPort->PortCfg.AuthMode == Ndis802_11AuthModeWPA2) || (pPort->PortCfg.AuthMode == Ndis802_11AuthModeWPA2PSK)) &&
        ((pPort->PortCfg.WepStatus == Ralink802_11Encryption2Enabled) || (pPort->PortCfg.WepStatus == Ralink802_11Encryption3Enabled)))
    {   
        FT_IE       *ft = NULL;
        UCHAR       oldMic[16];
        UCHAR       LinkIdentifier[20];
        UCHAR       content[256];
        ULONG       c_len = 0;
        ULONG       tmp_len = 0;
        UCHAR       seq = 4;
        UCHAR       mic[16];
        
        // FTIE (7.3.2.48)
        // It's the same as that included in TPK Handshake Message3 with the exception of MIC field.

        // Validate FTIE and its MIC
        //

        // point to the element of IE
        ft = (PFT_IE)(FTIe + 2); 
        
        if ((FTLen != (sizeof(FT_IE) - 256 + 2)) || PlatformEqualMemory(&ft->MICControl, ZeroSsid, 2) == 0 || 
            (PlatformEqualMemory(ft->SNonce, pTDLS->SNonce, 32) == 0) || (PlatformEqualMemory(ft->ANonce, pTDLS->ANonce, 32) == 0))
        {
            DBGPRINT(RT_DEBUG_ERROR,("TDLS - TdlsPeerTearDownAction() Invalid FTIE, drop the teardown frame\n"));
            return;
        }
        
        // backup MIC from the peer TDLS
        PlatformMoveMemory(oldMic, ft->MIC, 16);

        
        // set MIC field to zero before MIC calculation
        PlatformZeroMemory(ft->MIC, 16);

        // Construct LinkIdentifier (IE + Length + BSSID + Initiator MAC + Responder MAC)
        PlatformZeroMemory(LinkIdentifier, 20);
        LinkIdentifier[0] = IE_TDLS_LINK_ID;
        LinkIdentifier[1] = 18;
        PlatformMoveMemory(&LinkIdentifier[2], pPort->PortCfg.Bssid, 6);
        if (pTDLS->bInitiator)
        {
            PlatformMoveMemory(&LinkIdentifier[8], pAd->HwCfg.CurrentAddress, 6);
            PlatformMoveMemory(&LinkIdentifier[14], pTDLS->MacAddr, 6);
        }
        else
        {
            PlatformMoveMemory(&LinkIdentifier[8], pTDLS->MacAddr, 6);
            PlatformMoveMemory(&LinkIdentifier[14], pAd->HwCfg.CurrentAddress, 6);
        }

        ////////////////////////////////////////////////////////////////////////
        // The MIC field of FTIE shall be calculated on the concatenation, in the following order, of
        // 1. Link Identifier (20 bytes)
        // 2. Reason Code (2 bytes)
        // 3. Dialog token (1 byte)
        // 4. Transaction Sequence = 4 (1 byte)
        // 5. FTIE with the MIC field of FTIE set to zero (84 bytes)    
        
        /* concatenate Link Identifier, Reason Code, Dialog token, Transaction Sequence */
        MakeOutgoingFrame(content,                  &tmp_len,
                        sizeof(LinkIdentifier),     LinkIdentifier, 
                        2,                          &ReasonCode,
                        1,                          &pTDLS->Token,
                        1,                          &seq,
                        END_OF_ARGS);
        c_len += tmp_len;                   

        /* concatenate FTIE */
        MakeOutgoingFrame(content + c_len,      &tmp_len,
                        FTIe[1] + 2,            FTIe,  
                        END_OF_ARGS);
        c_len += tmp_len;                   
        
        /* Calculate MIC */
        PlatformZeroMemory(mic, sizeof(mic));
        AES_128_CMAC(pTDLS->TPK, content, c_len, mic);

        ////////////////////////////////////////////////////////////////////////

        if (PlatformEqualMemory(oldMic, mic, 16) == 0)
        {
            DBGPRINT(RT_DEBUG_ERROR,("TDLS - TdlsPeerTearDownAction() MIC Error, drop the teardown frame\n"));
            return;
        }

    }
    

    // clear this tdls entry
    if(pTDLS)
    {
        pTDLS->Status   = TDLS_MODE_NONE;
        pTDLS->Valid    = FALSE;
        PlatformCancelTimer(&pTDLS->Timer, &TimerCancelled);
        
        MtAsicDelWcidTab(pAd, pTDLS->Wcid);
        AsicRemoveKeyEntry(pAd, pTDLS->Wcid, 0, TRUE);
        TdlsDeleteMacTableEntry(pAd, pPort, pTDLS->MacAddr);
    }

}

/*
    ========================================================================

    Routine Description:
        Check if the frame can be sent through tunneled direct link (TDLS) interface,
        excluding TDLS action packets from TDLS state machine.
        
    Arguments:
        pAd     Pointer to adapter
        
    Return Value:
        TDLS entry index
        
    Note:
        Get rid of TDLS payload contains a TDLS Action frame body filled by 
        TDLS State Machine.

    
    ========================================================================
*/
int TdlsCheckTdlsFrame(
    IN  PMP_ADAPTER   pAd,
    IN  PUCHAR          pDA)
{
    int rval = -1;
    int i;
    PMP_PORT  pPort = pAd->PortList[pAd->ucActivePortNum];
    
    //if (!TDLS_ON(pAd))
    //  return rval;

    //if (!INFRA_ON(pPort))
    //  return rval;

    if (pPort->PortSubtype != PORTSUBTYPE_STA)
        return rval;

    do{
        // check tdls table entry
        for (i=0; i<MAX_NUM_OF_TDLS_ENTRY; i++)
        {
            if (pAd->StaCfg.TDLSEntry[i].Valid && (pAd->StaCfg.TDLSEntry[i].Status >= TDLS_MODE_CONNECTED) &&
                MAC_ADDR_EQUAL(pDA, pAd->StaCfg.TDLSEntry[i].MacAddr))
            {
                rval = i;
                break;
            }
        }

    } while (FALSE);
    
    return rval;
}

/*
    ==========================================================================
    Description:
        
    IRQL = DISPATCH_LEVEL

    ==========================================================================
 */
VOID TdlsSendTdlsTearDownFrame(
    IN  PMP_ADAPTER   pAd,
    IN  PRT_802_11_TDLS pTDLS)
{
    PUCHAR          pOutBuffer = NULL;
    NDIS_STATUS     NStatus;
    HEADER_802_11       Header80211;
    TDLS_ACTION_HEADER  TdlsActHdr;
    ULONG           FrameLen = 0;
    USHORT          Reason = REASON_TDLS_UNSPECIFY;
    UCHAR           Category = CATEGORY_TDLS;
    UCHAR           Action = ACTION_TDLS_TEARDOWN;
    UCHAR           i = 0;
    UCHAR           LinkIdentifier[20];
    PMP_PORT      pPort = pAd->PortList[PORT_0];  
    UCHAR           LengthQosPAD;

    if (MT_TEST_FLAG(pAd, fRTMP_ADAPTER_RESET_IN_PROGRESS) ||
        MT_TEST_FLAG(pAd, fRTMP_ADAPTER_HALT_IN_PROGRESS))
        return;
    
    DBGPRINT(RT_DEBUG_TRACE, ("Send TDLS TearDown Frame \n"));

    NStatus = MlmeAllocateMemory(pAd, &pOutBuffer);  //Get an unused nonpaged memory
    if (NStatus != NDIS_STATUS_SUCCESS) 
    {
        DBGPRINT(RT_DEBUG_ERROR,("TdlsSendTdlsTearDownFrame() allocate memory failed \n"));
        return;
    }
    
    // Qos will be add in HardTx
    LengthQosPAD = 0;


    // 1. Make 802.11 header for Vista, where Addr1: AP Addr2: SA, Addr3: DA
    // Sequence Number and WEP bit will be updated later
    PlatformZeroMemory(&Header80211, sizeof(HEADER_802_11));
    Header80211.FC.Type = BTYPE_DATA;   
    COPY_MAC_ADDR(Header80211.Addr1, pPort->PortCfg.Bssid); 
    COPY_MAC_ADDR(Header80211.Addr2, pAd->HwCfg.CurrentAddress);
    COPY_MAC_ADDR(Header80211.Addr3, pTDLS->MacAddr);
    Header80211.FC.ToDs = 1; // through AP

    // 2. Prepare LLC_SNAP and TDLS Action header
    TdlsActHdr.Protocol = PROTO_NAME_TDLS;
    TdlsActHdr.Category = CATEGORY_TDLS;
    TdlsActHdr.Action   = ACTION_TDLS_TEARDOWN;

    // 3. Build basic frame first
    // Construct LinkIdentifier (IE + Length + BSSID + Initiator MAC + Responder MAC)
    PlatformZeroMemory(LinkIdentifier, 20);
    LinkIdentifier[0] = IE_TDLS_LINK_ID;
    LinkIdentifier[1] = 18;
    PlatformMoveMemory(&LinkIdentifier[2], pPort->PortCfg.Bssid, 6);
    if (pTDLS->bInitiator)
    {
        PlatformMoveMemory(&LinkIdentifier[8], pAd->HwCfg.CurrentAddress, 6);
        PlatformMoveMemory(&LinkIdentifier[14], pTDLS->MacAddr, 6);
    }
    else
    {
        PlatformMoveMemory(&LinkIdentifier[8], pTDLS->MacAddr, 6);
        PlatformMoveMemory(&LinkIdentifier[14], pAd->HwCfg.CurrentAddress, 6);
    }
    

    MakeOutgoingFrame(pOutBuffer,               &FrameLen,
                    sizeof(HEADER_802_11),      &Header80211,
                    LENGTH_802_1_H,             TDLS_LLC_SNAP,
                    sizeof(TDLS_ACTION_HEADER), &TdlsActHdr,
                    2,                          &Reason,
                    sizeof(LinkIdentifier),     LinkIdentifier,                                             
                    END_OF_ARGS);

    

    // FTIE includes if RSNA Enabled
    if (((pPort->PortCfg.AuthMode == Ndis802_11AuthModeWPA2) || (pPort->PortCfg.AuthMode == Ndis802_11AuthModeWPA2PSK)) &&
        ((pPort->PortCfg.WepStatus == Ralink802_11Encryption2Enabled) || (pPort->PortCfg.WepStatus == Ralink802_11Encryption3Enabled)))
    {   
        UCHAR               FTIe[128];  
        FT_IE               *ft = NULL;
        UCHAR               content[256];
        ULONG               c_len = 0;
        ULONG               tmp_len = 0;
        UCHAR               seq = 4;
        UCHAR               mic[16];
        
        // FTIE (7.3.2.48)
        // The contents of FTIE in the TDLS Teardown frame shall be the same as that included
        // in the TPK Handshake Message3 with the exception of the MIC field.

        // Construct FTIE (IE + Length + MIC Control + MIC + ANonce + SNonce)
            
        // point to the element of IE
        PlatformZeroMemory(FTIe, sizeof(FTIe));
        FTIe[0] = IE_FAST_BSS_TRANSITION;
        FTIe[1] = sizeof(FT_IE) - 256;
        ft = (PFT_IE)&FTIe[2];  
        PlatformMoveMemory(ft->ANonce, pTDLS->ANonce, 32);
        PlatformMoveMemory(ft->SNonce, pTDLS->SNonce, 32);

        ////////////////////////////////////////////////////////////////////////
        // The MIC field of FTIE shall be calculated on the concatenation, in the following order, of
        // 1. Link Identifier (20 bytes)
        // 2. Reason Code (2 bytes)
        // 3. Dialog token (1 byte)
        // 4. Transaction Sequence = 4 (1 byte)
        // 5. FTIE with the MIC field of FTIE set to zero (84 bytes)    
        
        /* concatenate Link Identifier, Reason Code, Dialog token, Transaction Sequence */
        MakeOutgoingFrame(content,                  &tmp_len,
                        sizeof(LinkIdentifier),     LinkIdentifier, 
                        2,                          &Reason,
                        1,                          &pTDLS->Token,
                        1,                          &seq,
                        END_OF_ARGS);
        c_len += tmp_len;                   

        /* concatenate FTIE */
        MakeOutgoingFrame(content + c_len,      &tmp_len,
                            FTIe[1] + 2,            FTIe,  
                        END_OF_ARGS);
        c_len += tmp_len;                   
        
        /* Calculate MIC */
        PlatformZeroMemory(mic, sizeof(mic));
        AES_128_CMAC(pTDLS->TPK, content, c_len, mic);
        
        /* Fill Mic to ft struct */
        PlatformMoveMemory(ft->MIC, mic, 16);
        ////////////////////////////////////////////////////////////////////////

#if 0   // FT_InsertXXX shall be defined to Dot11R  
        // Insert FT_IE to outgoing frame
        FT_InsertFTIE(
                pAd, 
                pOutBuffer + FrameLen, 
                &FrameLen, 
                FTIe[1], 
                ft->MICControl, 
                ft->MIC, 
                ft->ANonce, 
                ft->SNonce);
#else
    {
        UCHAR   ValueIE;
        UINT16  MICCtrBuf;
        UCHAR   Length;
        ULONG   tmp;
        
        // Insert FT_IE to outgoing frame
        ValueIE = IE_FAST_BSS_TRANSITION;
        Length =  sizeof(FT_IE) - 256;
        MICCtrBuf = cpu2le16(ft->MICControl.word);
        
        MakeOutgoingFrame(  pOutBuffer + FrameLen,  &tmp,
                            1,                      &ValueIE,
                            1,                      &Length,
                            2,                      (PUCHAR)&MICCtrBuf,
                            16,                     (PUCHAR)&ft->MIC,
                            32,                     (PUCHAR)ft->ANonce,
                            32,                     (PUCHAR)ft->SNonce,
                            END_OF_ARGS);
        FrameLen += tmp;
    }
#endif
    }
        
    // Send a packet via HardTransmit
    TdlsSendPacket(pAd, pOutBuffer, FrameLen);
    MlmeFreeMemory(pAd, pOutBuffer);
}

/*
    ========================================================================
    
    Routine Description:
        Send the TDLS Packet (802.3 format), via RTXmitSendPacket and HardTransmit.

    Arguments:
        pAd     Pointer to our adapter
        pBuffer 
        Length
                
    Return Value:
        NONE

    Note:
        RATE_1 is used. we may need to update this

    ========================================================================
*/
VOID    TdlsSendPacket(
    IN  PMP_ADAPTER   pAd,
    IN  PVOID           pBuffer,
    IN  ULONG           Length)
{
    NDIS_STATUS         NStatus = NDIS_STATUS_FAILURE;
    BOOLEAN             IsLastNetBuffer = FALSE;
    PMT_XMIT_CTRL_UNIT    pXcu = NULL;
    
    // Reset is in progress, stop immediately
    if (MT_TEST_FLAG(pAd, fRTMP_ADAPTER_RESET_IN_PROGRESS)    ||
        MT_TEST_FLAG(pAd, fRTMP_ADAPTER_HALT_IN_PROGRESS) ||
        MT_TEST_FLAG(pAd, fRTMP_ADAPTER_NIC_NOT_EXIST))
    {
        DBGPRINT(RT_DEBUG_ERROR, ("TdlsSendPacket return (flag = %x)\n",pAd->Flags));
        return;
    }

    DBGPRINT(RT_DEBUG_TRACE, ("TdlsSendPacket=======>  Length=%d \n", Length));

    if (OPSTATUS_TEST_FLAG(pAd, fOP_STATUS_DOZE))
        AsicForceWakeup(pAd);

    pXcu = (PMT_XMIT_CTRL_UNIT)Ndis6CommonAllocateNdisNetBufferList(pAd, 0, pBuffer, Length);
    if (pXcu != NULL)
    {
        pXcu->PktSource = PKTSRC_TDLS;

        XmitStaSendPacket(pAd, 0, pXcu, FALSE);
        NdisSetEvent(&(GET_THREAD_EVENT(&pAd->pHifCfg->hDequeueDataThread)));
    }

    N6USBKickBulkOut(pAd);

    DBGPRINT(RT_DEBUG_WARN, ("<=======TdlsSendPacket  Length=%d \n", Length));
}

VOID TdlsTimeoutTimerCallback(
    IN PVOID SystemSpecific1, 
    IN PVOID FunctionContext, 
    IN PVOID SystemSpecific2, 
    IN PVOID SystemSpecific3) 
{
    /*
    PMP_PORT pPort = (PMP_PORT )FunctionContext;
    PMP_ADAPTER  pAd = pPort->pAd;
    UCHAR                   i, EntryIdx = 0xff;
    
    for (i=0; i<MAX_NUM_OF_TDLS_ENTRY; i++)
    {
        if (pTDLS == &pAd->StaCfg.TDLSEntry[i])
        {   
            EntryIdx = i;
            break;
        }
    }
    
    DBGPRINT(RT_DEBUG_TRACE, ("TdlsTimeoutTimerCallback - Failed to wait for the response, terminate the setup procedure #%d (%02x:%02x:%02x:%02x:%02x:%02x)\n",
        EntryIdx, pTDLS->MacAddr[0], pTDLS->MacAddr[1], pTDLS->MacAddr[2], pTDLS->MacAddr[3], pTDLS->MacAddr[4], pTDLS->MacAddr[5]));

    if ((pTDLS) && (pTDLS->Valid) && (pTDLS->Status < TDLS_MODE_CONNECTED))
    {
        pTDLS->Valid    = FALSE;
        pTDLS->Status   = TDLS_MODE_NONE;
        
        if ((EntryIdx >= 0) && (EntryIdx < MAX_NUM_OF_TDLS_ENTRY))
        {
            PlatformZeroMemory(&pAd->StaCfg.TDLSEntry[EntryIdx], sizeof(RT_802_11_DLS_UI));
        }
    }
*/
}

/*
==========================================================================
    Description:
        
    IRQL = PASSIVE_LEVEL
==========================================================================
*/
VOID TdlsTearDown(
    IN PMP_ADAPTER pAd,
    IN BOOLEAN      bDeleteTable)
{
    int i;
    BOOLEAN     TimerCancelled;
    PMP_PORT  pPort = pAd->PortList[PORT_0];
    
    //
    // tear down tdls table entry
    //

    // Just sent out teardown frames. Tdls entries will be deleted later.
    if (bDeleteTable == FALSE)
    {
        for (i=0; i<MAX_NUM_OF_TDLS_ENTRY; i++)
        {
            if ((pAd->StaCfg.TDLSEntry[i].Valid) && 
                (pAd->StaCfg.TDLSEntry[i].Wcid > BSSID_WCID) &&
                (pAd->StaCfg.TDLSEntry[i].Status == TDLS_MODE_CONNECTED))
            {
                PlatformCancelTimer(&pPort->StaCfg.TDLSEntry[i].Timer, &TimerCancelled);
                TdlsSendTdlsTearDownFrame(pAd, &pAd->StaCfg.TDLSEntry[i]);
                // wait deleting tables in the next called teardown
                pAd->StaCfg.TDLSEntry[i].Status = TDLS_MODE_WAIT_DELETE;
            }
        }
    }   
    else
    {
        for (i=0; i<MAX_NUM_OF_TDLS_ENTRY; i++)
        {
            if (pAd->StaCfg.TDLSEntry[i].Valid)
            {
                PlatformCancelTimer(&pPort->StaCfg.TDLSEntry[i].Timer, &TimerCancelled);

                if (pAd->StaCfg.TDLSEntry[i].Wcid > BSSID_WCID)
                {
                    // sent out teardown frames
                    if (pAd->StaCfg.TDLSEntry[i].Status == TDLS_MODE_CONNECTED)
                    {
                        TdlsSendTdlsTearDownFrame(pAd, &pAd->StaCfg.TDLSEntry[i]);
                    }
                    // delete table and key
                    MtAsicDelWcidTab(pAd, pAd->StaCfg.TDLSEntry[i].Wcid);
                    AsicRemoveKeyEntry(pAd, pAd->StaCfg.TDLSEntry[i].Wcid, 0, TRUE);
                    TdlsDeleteMacTableEntry(pAd, pPort, pAd->StaCfg.TDLSEntry[i].MacAddr);
                }
                // reset status
                pAd->StaCfg.TDLSEntry[i].Status = TDLS_MODE_NONE;
                pAd->StaCfg.TDLSEntry[i].Valid  = FALSE;
                pAd->StaCfg.TDLSEntry[i].Wcid   = 0;
            }
        }
    
        // empty search table
        if (TDLS_ON(pAd) &&
            (pAd->StaCfg.pTdlsSearchEntryPool) && 
            (pAd->StaCfg.TdlsSearchEntryFreeList.Number != TDLS_SEARCH_POOL_SIZE))
        {
            int idx;
            PTDLS_SEARCH_ENTRY  pTdlsSearchEntry;

            NdisAcquireSpinLock(&pAd->StaCfg.TdlsSearchTabLock);
            for (idx = 0; idx < TDLS_SEARCH_HASH_TAB_SIZE; idx++)
            {
                while (pAd->StaCfg.TdlsSearchTab[idx].Head)
                {
                    pTdlsSearchEntry = (PTDLS_SEARCH_ENTRY)RemoveHeadQueue(&pAd->StaCfg.TdlsSearchTab[idx]);
                    
                    if (pAd->StaCfg.TdlsSearchEntryFreeList.Number < TDLS_SEARCH_POOL_SIZE)
                    {
                        DBGPRINT(RT_DEBUG_TRACE, ("TDLS - Remove %02x:%02x:%02x:%02x:%02x:%02x from TdlsSearchTab[%d] \n", pTdlsSearchEntry->Addr[0],pTdlsSearchEntry->Addr[1],pTdlsSearchEntry->Addr[2], pTdlsSearchEntry->Addr[3], pTdlsSearchEntry->Addr[4], pTdlsSearchEntry->Addr[5], idx));
                        InsertTailQueue(&pAd->StaCfg.TdlsSearchEntryFreeList, (PQUEUE_ENTRY)pTdlsSearchEntry);
                        DBGPRINT(RT_DEBUG_TRACE, ("Free to TdlsSearchEntryFreeList: (Number=%d) \n", pAd->StaCfg.TdlsSearchEntryFreeList.Number));
                    }
                    break;
                }
                InitializeQueueHeader(&pAd->StaCfg.TdlsSearchTab[idx]);
            }
            NdisReleaseSpinLock(&pAd->StaCfg.TdlsSearchTabLock);

        }
    }   
}

VOID TdlsDeleteMacTableEntry(
    IN PMP_ADAPTER pAd,
    IN  PMP_PORT  pPort,
    IN PUCHAR pMacAddr)
{
    PMAC_TABLE_ENTRY pMacEntry = MacTableLookup(pAd,  pPort, pMacAddr);
    if (pMacEntry)
    {
        MacTableDeleteAndResetEntry(pAd, pPort, pMacEntry->Aid, pMacAddr, TRUE);
    }
}

/* Not found and full : return MAX_NUM_OF_TDLS_ENTRY
 *  not found and the entry is not full : return -1
 */
INT TdlsSearchLinkId(
    IN  PMP_ADAPTER   pAd,
    IN  PUCHAR          pAddr)
{
    INT i = 0;
    UCHAR empty = 0;
    
    for (i = 0; i < MAX_NUM_OF_TDLS_ENTRY; i++)
    {
        if (!pAd->StaCfg.TDLSEntry[i].Valid)
            empty |= 1;
        
        if (pAd->StaCfg.TDLSEntry[i].Valid && MAC_ADDR_EQUAL(pAddr, pAd->StaCfg.TDLSEntry[i].MacAddr))
        {
            DBGPRINT(RT_DEBUG_INFO, ("TdlsSearchLinkId - Find Link ID with Peer Address %02x:%02x:%02x:%02x:%02x:%02x(%d) \n", 
                pAddr[0], pAddr[1], pAddr[2], pAddr[3], pAddr[4], pAddr[5], i));
            break;
        }
    }

    if (i == MAX_NUM_OF_TDLS_ENTRY)
    {
        if (empty == 0)
        {
            DBGPRINT(RT_DEBUG_INFO, ("TdlsSearchLinkId -  not found and full\n"));
            return MAX_NUM_OF_TDLS_ENTRY;
        }
        else
        {
            DBGPRINT(RT_DEBUG_INFO, ("TdlsSearchLinkId -  not found\n"));
            return -1;
        }   
    }

    return i;
}

USHORT  TdlsTPKMsg1Process(
    IN  PMP_ADAPTER       pAd, 
    PMP_PORT              pPort,
    IN  PRT_802_11_TDLS     pTDLS,
    IN  PUCHAR              pRsnIe, 
    IN  UCHAR               RsnLen, 
    IN  PUCHAR              pFTIe, 
    IN  UCHAR               FTLen, 
    IN  PUCHAR              pTIIe, 
    IN  UCHAR               TILen)
{
    USHORT          StatusCode = MLME_SUCCESS;
    UCHAR           CipherTmp[64] = {0};
    UCHAR           CipherTmpLen = 0;
    FT_IE           *ft = NULL;         

    // Validate RsnIE
    //

    // [AP: WPA1+WPA2 mixed]
    // The Peer is WPA2 and the Local is WPA1. Reject this message with error code.
    if ((pPort->PortCfg.AuthMode != Ndis802_11AuthModeWPA2) && (pPort->PortCfg.AuthMode != Ndis802_11AuthModeWPA2PSK))
        return MLME_SECURITY_DISABLED;
    
    if (RsnLen == 0) // RSN not exist
        return  MLME_INVALID_INFORMATION_ELEMENT;

    if (pRsnIe[2] < 1) // Smaller version, silently discard the message
        return  MLME_UNSPECIFY_FAIL;

    CipherTmpLen = CipherSuiteTDLSLen;
    if (pPort->PortCfg.WepStatus == Ralink802_11Encryption2Enabled)
        PlatformMoveMemory(CipherTmp, CipherSuiteTDLSWpa2PskTkip, CipherTmpLen);
    else
        PlatformMoveMemory(CipherTmp, CipherSuiteTDLSWpa2PskAes, CipherTmpLen);

    if ( PlatformEqualMemory(&pRsnIe[16], &CipherTmp[16], 4) == 0) // Invalid TDLS AKM
        return MLME_INVALID_AKMP;

    // [AP: TKIP+AES mixed]
    if ((RsnLen != 22) || (PlatformEqualMemory(pRsnIe, CipherTmp, RsnLen) == 0)) // Invalid Pairwise Cipher
        return REASON_UCIPHER_NOT_VALID;

    if ( PlatformEqualMemory(&pRsnIe[20], &CipherTmp[20], 2) == 0) // Invalid RSN capability
        return MLME_INVALID_RSN_CAPABILITIES;


    // Validate FTIE
    //
    ft = (PFT_IE)(pFTIe + 2); // point to the element of IE
    if ((FTLen != (sizeof(FT_IE) - 256 + 2)) || PlatformEqualMemory(&ft->MICControl, ZeroSsid, 2) == 0 || 
        (PlatformEqualMemory(ft->MIC, ZeroSsid, 16) == 0) || (PlatformEqualMemory(ft->ANonce, ZeroSsid, 32) == 0))
        return REASON_FT_INVALID_FTIE;

    // Validate TIIE
    //
    if ((TILen != 7) || (pTIIe[2] != 2) || ( le2cpu32(*((PULONG) (pTIIe + 3))) < TDLS_KEY_TIMEOUT))
        return MLME_UNACCEPTABLE_LIFETIME;

    return StatusCode;
}


USHORT  TdlsTPKMsg2Process(
    IN  PMP_ADAPTER       pAd,
    PMP_PORT              pPort,  
    IN  PRT_802_11_TDLS     pTDLS,
    IN  PUCHAR              pRsnIe, 
    IN  UCHAR               RsnLen, 
    IN  PUCHAR              pFTIe, 
    IN  UCHAR               FTLen, 
    IN  PUCHAR              pTIIe, 
    IN  UCHAR               TILen,
    OUT PUCHAR              pTPK,
    OUT PUCHAR              pTPKName)
{
    USHORT          StatusCode = MLME_SUCCESS;
    UCHAR           CipherTmp[64] = {0};
    UCHAR           CipherTmpLen = 0;
    FT_IE           *ft = NULL;         
    UCHAR           oldMic[16];
    UCHAR           LinkIdentifier[20];
    
    // Validate RsnIE
    //
    if (RsnLen == 0) // RSN not exist
        return  MLME_INVALID_INFORMATION_ELEMENT;

    if (pRsnIe[2] < 1) // Smaller version, silently discard the message
        return  MLME_UNSPECIFY_FAIL;

    CipherTmpLen = CipherSuiteTDLSLen;
    if (pPort->PortCfg.WepStatus == Ralink802_11Encryption2Enabled)
        PlatformMoveMemory(CipherTmp, CipherSuiteTDLSWpa2PskTkip, CipherTmpLen);
    else
        PlatformMoveMemory(CipherTmp, CipherSuiteTDLSWpa2PskAes, CipherTmpLen);

    if ( PlatformEqualMemory(&pRsnIe[16], &CipherTmp[16], 4) == 0) // Invalid TDLS AKM
        return MLME_INVALID_AKMP;

    if ( PlatformEqualMemory(&pRsnIe[20], &CipherTmp[20], 2) == 0) // Invalid RSN capability
        return MLME_INVALID_RSN_CAPABILITIES;

    if ((RsnLen != 22) || (PlatformEqualMemory(pRsnIe, CipherTmp, RsnLen) == 0)) // Invalid Pairwise Cipher
        return REASON_UCIPHER_NOT_VALID;

    // Validate FTIE
    //
    ft = (PFT_IE)(pFTIe + 2); // point to the element of IE
    if ((FTLen != (sizeof(FT_IE) - 256 + 2)) || PlatformEqualMemory(&ft->MICControl, ZeroSsid, 2) == 0 || 
        (PlatformEqualMemory(ft->SNonce, pTDLS->SNonce, 32) == 0))
        return REASON_FT_INVALID_FTIE;

    // Validate TIIE
    //
    if ((TILen != 7) || (pTIIe[2] != 2) || ( le2cpu32(*((PULONG) (pTIIe + 3))) < TDLS_KEY_TIMEOUT))
        return MLME_UNACCEPTABLE_LIFETIME;


    // Validate the MIC field of FTIE
    //
    
    // point to the element of IE
    ft = (PFT_IE)(pFTIe + 2);   
    // backup MIC fromm the peer TDLS
    PlatformMoveMemory(oldMic, ft->MIC, 16);

    
    // %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
    // Derive TPK-KCK for MIC key, TPK-TK for direct link data
    TdlsDeriveTPK(
            pAd->HwCfg.CurrentAddress, /* I am Initiator */
            pTDLS->MacAddr, /* MAC Address of Responder */
            ft->ANonce, 
            ft->SNonce, 
            pPort->PortCfg.Bssid,
            pTPK,
            pTPKName);
    
    // %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
    
    // set MIC field to zero before MIC calculation
    PlatformZeroMemory(ft->MIC, 16);

    // Construct LinkIdentifier (IE + Length + BSSID + Initiator MAC + Responder MAC)
    PlatformZeroMemory(LinkIdentifier, 20);
    LinkIdentifier[0] = IE_TDLS_LINK_ID;
    LinkIdentifier[1] = 18;
    PlatformMoveMemory(&LinkIdentifier[2], pPort->PortCfg.Bssid, 6);
    PlatformMoveMemory(&LinkIdentifier[8], pAd->HwCfg.CurrentAddress, 6);
    PlatformMoveMemory(&LinkIdentifier[14], pTDLS->MacAddr, 6);

    ////////////////////////////////////////////////////////////////////////
    // The MIC field of FTIE shall be calculated on the concatenation, in the following order, of
    // 1. MAC_I (6 bytes)
    // 2. MAC_R (6 bytes)
    // 3. Transaction Sequence = 2 (1 byte)
    // 4. Link Identifier (20 bytes)
    // 5. RSN IE without the IE header (20 bytes)
    // 6. Timeout Interval IE (7 bytes)
    // 7. FTIE without the IE header, with the MIC field of FTIE set to zero (82 bytes) 
    {
        UCHAR   content[512];
        ULONG   c_len = 0;
        ULONG   tmp_len = 0;
        UCHAR   Seq = 2;
        UCHAR   mic[16];
        
        PlatformZeroMemory(mic, sizeof(mic));
            
        /* make a header frame for calculating MIC. */
        MakeOutgoingFrame(content,                  &tmp_len,
                          MAC_ADDR_LEN,             pAd->HwCfg.CurrentAddress,
                          MAC_ADDR_LEN,             pTDLS->MacAddr,
                          1,                        &Seq,
                          END_OF_ARGS);
        c_len += tmp_len;                   

        /* concatenate Link Identifier */           
        MakeOutgoingFrame(content + c_len,      &tmp_len,
                          20,                   LinkIdentifier,
                          END_OF_ARGS);
        c_len += tmp_len;                   

        
        /* concatenate RSNIE */
        MakeOutgoingFrame(content + c_len,      &tmp_len,
                          20,                   pRsnIe + 2,
                          END_OF_ARGS);
        c_len += tmp_len;                   

        /* concatenate Timeout Interval IE */
        MakeOutgoingFrame(content + c_len,     &tmp_len,
                          7,                    pTIIe,
                          END_OF_ARGS);
        c_len += tmp_len;
        
        
        /* concatenate FTIE */          
        MakeOutgoingFrame(content + c_len,      &tmp_len,
                          sizeof(FT_IE) - 256,  (PUCHAR)ft,
                          END_OF_ARGS);
        c_len += tmp_len;
        

        /* Calculate MIC */             
        AES_128_CMAC(pTPK, content, c_len, mic);
        PlatformMoveMemory(ft->MIC, mic, 16);

    }
    ////////////////////////////////////////////////////////////////////////
    
    if (PlatformEqualMemory(oldMic, ft->MIC, 16) == 0)
    {
        DBGPRINT(RT_DEBUG_ERROR,("TdlsTPKMsg2Process() MIC Error!!! \n"));
        return MLME_UNSPECIFY_FAIL;
    }

    
    return StatusCode;
}


USHORT  TdlsTPKMsg3Process(
    IN  PMP_ADAPTER       pAd,
    PMP_PORT              pPort,
    IN  PRT_802_11_TDLS     pTDLS,
    IN  PUCHAR              pRsnIe, 
    IN  UCHAR               RsnLen, 
    IN  PUCHAR              pFTIe, 
    IN  UCHAR               FTLen, 
    IN  PUCHAR              pTIIe, 
    IN  UCHAR               TILen)
{
    USHORT          StatusCode = MLME_SUCCESS;
    UCHAR           CipherTmp[64] = {0};
    UCHAR           CipherTmpLen = 0;
    FT_IE           *ft = NULL;         
    UCHAR           oldMic[16];
    UCHAR           LinkIdentifier[20];
    
    // Validate RsnIE
    //
    if (RsnLen == 0) // RSN not exist
        return  MLME_INVALID_INFORMATION_ELEMENT;

    if (pRsnIe[2] < 1) // Smaller version, silently discard the message
        return  MLME_UNSPECIFY_FAIL;

    CipherTmpLen = CipherSuiteTDLSLen;
    if (pPort->PortCfg.WepStatus ==Ralink802_11Encryption2Enabled)
        PlatformMoveMemory(CipherTmp, CipherSuiteTDLSWpa2PskTkip, CipherTmpLen);
    else
        PlatformMoveMemory(CipherTmp, CipherSuiteTDLSWpa2PskAes, CipherTmpLen);

    if ( PlatformEqualMemory(&pRsnIe[16], &CipherTmp[16], 4) == 0) // Invalid TDLS AKM
        return MLME_INVALID_AKMP;

    if ( PlatformEqualMemory(&pRsnIe[20], &CipherTmp[20], 2) == 0) // Invalid RSN capability
        return MLME_INVALID_RSN_CAPABILITIES;

    if ((RsnLen != 22) || (PlatformEqualMemory(pRsnIe, CipherTmp, RsnLen) == 0)) // Invalid Pairwise Cipher
        return REASON_UCIPHER_NOT_VALID;

    // Validate FTIE
    //
    ft = (PFT_IE)(pFTIe + 2); // point to the element of IE
    if ((FTLen != (sizeof(FT_IE) - 256 + 2)) || PlatformEqualMemory(&ft->MICControl, ZeroSsid, 2) == 0 || 
        (PlatformEqualMemory(ft->SNonce, pTDLS->SNonce, 32) == 0) || (PlatformEqualMemory(ft->ANonce, pTDLS->ANonce, 32) == 0))
        return REASON_FT_INVALID_FTIE;

    // Validate TIIE
    //
    if ((TILen != 7) || (pTIIe[2] != 2) || ( le2cpu32(*((PULONG) (pTIIe + 3))) < pTDLS->KeyLifetime))
        return MLME_UNACCEPTABLE_LIFETIME;


    // Validate the MIC field of FTIE
    //
    
    // point to the element of IE
    ft = (PFT_IE)(pFTIe + 2);   
    // backup MIC fromm the peer TDLS
    PlatformMoveMemory(oldMic, ft->MIC, 16);

        
    // set MIC field to zero before MIC calculation
    PlatformZeroMemory(ft->MIC, 16);

    // Construct LinkIdentifier (IE + Length + BSSID + Initiator MAC + Responder MAC)
    PlatformZeroMemory(LinkIdentifier, 20);
    LinkIdentifier[0] = IE_TDLS_LINK_ID;
    LinkIdentifier[1] = 18;
    PlatformMoveMemory(&LinkIdentifier[2], pPort->PortCfg.Bssid, 6);
    PlatformMoveMemory(&LinkIdentifier[8], pTDLS->MacAddr, 6);
    PlatformMoveMemory(&LinkIdentifier[14], pAd->HwCfg.CurrentAddress, 6);

    ////////////////////////////////////////////////////////////////////////
    // The MIC field of FTIE shall be calculated on the concatenation, in the following order, of
    // 1. MAC_I (6 bytes)
    // 2. MAC_R (6 bytes)
    // 3. Transaction Sequence = 3 (1 byte)
    // 4. Link Identifier (20 bytes)
    // 5. RSN IE without the IE header (20 bytes)
    // 6. Timeout Interval IE (7 bytes)
    // 7. FTIE without the IE header, with the MIC field of FTIE set to zero (82 bytes)
    {
        UCHAR   content[512];
        ULONG   c_len = 0;
        ULONG   tmp_len = 0;
        UCHAR   Seq = 3;
        UCHAR   mic[16];
        
        PlatformZeroMemory(mic, sizeof(mic));
            
        /* make a header frame for calculating MIC. */
        MakeOutgoingFrame(content,                  &tmp_len,
                          MAC_ADDR_LEN,             pTDLS->MacAddr,
                          MAC_ADDR_LEN,             pAd->HwCfg.CurrentAddress,
                          1,                        &Seq,
                          END_OF_ARGS);
        c_len += tmp_len;                   

        /* concatenate Link Identifier */           
        MakeOutgoingFrame(content + c_len,      &tmp_len,
                          20,                   LinkIdentifier,
                          END_OF_ARGS);
        c_len += tmp_len;                   

        
        /* concatenate RSNIE */
        MakeOutgoingFrame(content + c_len,      &tmp_len,
                          20,                   pRsnIe + 2,
                          END_OF_ARGS);
        c_len += tmp_len;                   

        /* concatenate Timeout Interval IE */
        MakeOutgoingFrame(content + c_len,     &tmp_len,
                          7,                    pTIIe,
                          END_OF_ARGS);
        c_len += tmp_len;
        
        
        /* concatenate FTIE */          
        MakeOutgoingFrame(content + c_len,      &tmp_len,
                          sizeof(FT_IE) - 256,  (PUCHAR)ft,
                          END_OF_ARGS);
        c_len += tmp_len;
        

        /* Calculate MIC */             
        AES_128_CMAC(pTDLS->TPK, content, c_len, mic);
        PlatformMoveMemory(ft->MIC, mic, 16);

    }
    ////////////////////////////////////////////////////////////////////////
    
    if (PlatformEqualMemory(oldMic, ft->MIC, 16) == 0)
    {
        DBGPRINT(RT_DEBUG_ERROR,("TdlsTPKMsg3Process() MIC Error!!! \n"));
        return MLME_UNSPECIFY_FAIL;
    }

    
    return StatusCode;
}

/*
    ========================================================================
    
    Routine Description:
        It is used to derive the TDLS Peer Key and its identifier TPK-Name.
        (IEEE 802.11z/D4.0, 8.5.9.1)
        
    Arguments:

    Return Value:

    Note:
        TPK = KDF-256(0, "TDLS PMK", min(MAC_I, MAC_R) || max(MAC_I, MAC_R) || min(SNonce, ANonce) || max(SNonce, ANonce) || BSSID || N_KEY)
        TPK-Name = Truncate-128(SHA-256(min(MAC_I, MAC_R) || max(MAC_I, MAC_R) || min(SNonce, ANonce) || max(SNonce, ANonce) || BSSID || 256))
    ========================================================================
*/
VOID TdlsDeriveTPK(
    IN  PUCHAR  mac_i,
    IN  PUCHAR  mac_r,
    IN  PUCHAR  a_nonce,
    IN  PUCHAR  s_nonce,
    IN  PUCHAR  bssid,
    OUT PUCHAR  tpk,
    OUT PUCHAR  tpk_name)
{   
    UCHAR   temp_result[64];
    UCHAR   context[128];
    UINT    c_len=0; 
    UCHAR   temp_var[32];
    UINT    key_len = LEN_PMK;
    USHORT  len_in_bits = (key_len << 3);
    UCHAR   key_material[LEN_PMK];
    
    // ===============================
    //      TPK derivation  
    // ===============================

    // construct the concatenated context for TPK
    // min(MAC_I, MAC_R)    (6 bytes)
    // max(MAC_I, MAC_R)    (6 bytes)
    // min(SNonce, ANonce)  (32 bytes)
    // max(SNonce, ANonce)  (32 bytes)
    // BSSID                (6 bytes)
    // Number of Key in bits(2 bytes)

    // Initial the related parameter
    PlatformZeroMemory(temp_result, 64);
    PlatformZeroMemory(context, 128);
    PlatformZeroMemory(temp_var, 32);
    c_len = 0;

    // concatenate min(MAC_I, MAC_R) with 6-octets 
    ApWpaGetSmall(mac_i, mac_r, temp_var, 6);
    PlatformMoveMemory(&context[c_len], temp_var, 6);
    c_len += 6;
    
    // concatenate max(MAC_I, MAC_R) with 6-octets
    ApWpaGetLarge(mac_i, mac_r, temp_var, 6);
    PlatformMoveMemory(&context[c_len], temp_var, 6);
    c_len += 6;

    // concatenate min(SNonce, ANonce) with 32-octets 
    ApWpaGetSmall(s_nonce, a_nonce, temp_var, 32);
    PlatformMoveMemory(&context[c_len], temp_var, 32);
    c_len += 32;
    
    // concatenate max(SNonce, ANonce) with 32-octets
    ApWpaGetLarge(s_nonce, a_nonce, temp_var, 32);
    PlatformMoveMemory(&context[c_len], temp_var, 32);
    c_len += 32;

    // concatenate the BSSID with 6-octets 
    PlatformMoveMemory(&context[c_len], bssid, MAC_ADDR_LEN);
    c_len += MAC_ADDR_LEN;

    // concatenate the N_KEY with 2-octets
    PlatformMoveMemory(&context[c_len], &len_in_bits, 2);
    c_len += 2;

    // Zero key material
    PlatformZeroMemory(key_material, LEN_PMK);
    
    // Calculate a key material through FT-KDF
    FT_KDF(key_material, 
            LEN_PMK, 
            (PUCHAR)"TDLS PMK", 
            8, 
            context, 
            c_len, 
            temp_result, 
            key_len);
    PlatformMoveMemory(tpk, temp_result, key_len);

    DumpFrameMessage(tpk, LEN_PMK,("TPK\n"));


    // ===============================
    //      TPK-Name derivation 
    // ===============================

    // construct the concatenated context for TPK-Name
    // min(MAC_I, MAC_R)    (6 bytes)
    // max(MAC_I, MAC_R)    (6 bytes)
    // min(SNonce, ANonce)  (32 bytes)
    // max(SNonce, ANonce)  (32 bytes)
    // BSSID                (6 bytes)
    // Number of Key in bits(2 bytes)

    // The context is the same as the contxex of TPK.
    
    // Initial the related parameter
    PlatformZeroMemory(temp_result, 64);

    // derive TPK-Name
    SHA256(context, c_len, temp_result);
    PlatformMoveMemory(tpk_name, temp_result, LEN_PMK_NAME);

    DumpFrameMessage(tpk_name, LEN_PMK_NAME,("TPK-Name\n"));

}

VOID
TdlsTableInit(
    IN  PMP_ADAPTER   pAd)
{
    UCHAR idx;
    PRT_802_11_TDLS pTDLS = NULL;

    for (idx = 0; idx < MAX_NUM_OF_TDLS_ENTRY; idx++)
    {
        pTDLS = &pAd->StaCfg.TDLSEntry[idx];

        pTDLS->Valid = FALSE;
        pTDLS->Status = TDLS_MODE_NONE;
    }

    TdlsSearchTabInit(pAd);

    pAd->StaCfg.TDLSDialogToken = 0;

}

VOID
TdlsSearchTabInit(
    IN PMP_ADAPTER pAd)
{
    INT             idx;
    ULONG           i;
    NDIS_STATUS     NStatus;

    NStatus = PlatformAllocateMemory(pAd, (VOID *) &pAd->StaCfg.pTdlsSearchEntryPool, sizeof(TDLS_SEARCH_ENTRY) * TDLS_SEARCH_POOL_SIZE);
    if (NStatus == NDIS_STATUS_SUCCESS)
    {
        PlatformZeroMemory(pAd->StaCfg.pTdlsSearchEntryPool, sizeof(TDLS_SEARCH_ENTRY) * TDLS_SEARCH_POOL_SIZE);
        InitializeQueueHeader(&pAd->StaCfg.TdlsSearchEntryFreeList);
        for (i = 0; i < TDLS_SEARCH_POOL_SIZE; i++)
            InsertTailQueue(&pAd->StaCfg.TdlsSearchEntryFreeList, (PQUEUE_ENTRY)(pAd->StaCfg.pTdlsSearchEntryPool + (ULONG)i));
        for (idx = 0; idx < TDLS_SEARCH_HASH_TAB_SIZE; idx++)
            InitializeQueueHeader(&pAd->StaCfg.TdlsSearchTab[idx]);
    }
    else
    {
        DBGPRINT(RT_DEBUG_ERROR, ("%s Fail to alloc memory for pAd->StaCfg.pTdlsSearchEntryPool", __FUNCTION__));
        PlatformZeroMemory(&pAd->StaCfg.TdlsSearchEntryFreeList, sizeof(QUEUE_HEADER));
        PlatformZeroMemory(&pAd->StaCfg.TdlsSearchTab[0], sizeof(QUEUE_HEADER)*TDLS_SEARCH_HASH_TAB_SIZE);
    }
}

VOID
TdlsSearchTabDestory(
    IN PMP_ADAPTER pAd)
{
    INT idx;
    PTDLS_SEARCH_ENTRY pTdlsSearchEntry;

    if (pAd->StaCfg.pTdlsSearchEntryPool == NULL)
    {
        DBGPRINT(RT_DEBUG_ERROR, ("%s : pAd->StaCfg.pTdlsSearchEntryPool = NULL", __FUNCTION__));
        return;
    }

    for (idx = 0; idx < TDLS_SEARCH_HASH_TAB_SIZE; idx++)
    {
        pTdlsSearchEntry =
            (PTDLS_SEARCH_ENTRY)pAd->StaCfg.TdlsSearchTab[idx].Head;
        while(pTdlsSearchEntry)
        {
            PTDLS_SEARCH_ENTRY pTdlsSearchEntryNext = pTdlsSearchEntry->pNext;
            TdlsSearchEntyFree(pAd, pTdlsSearchEntry);
            pTdlsSearchEntry = pTdlsSearchEntryNext;
            break;
        }
    }


    if (pAd->StaCfg.pTdlsSearchEntryPool)
        PlatformFreeMemory(pAd->StaCfg.pTdlsSearchEntryPool, sizeof(TDLS_SEARCH_ENTRY) * TDLS_SEARCH_POOL_SIZE);
    pAd->StaCfg.pTdlsSearchEntryPool = NULL;
    

}

PTDLS_SEARCH_ENTRY
TdlsSearchEntyAlloc(
    IN PMP_ADAPTER pAd)
{
    PTDLS_SEARCH_ENTRY pTdlsSearchEntry = NULL;

    NdisAcquireSpinLock(&pAd->StaCfg.TdlsSearchTabLock);
    if (pAd->StaCfg.TdlsSearchEntryFreeList.Number > 0)
    {
        pTdlsSearchEntry = (PTDLS_SEARCH_ENTRY)RemoveHeadQueue(&pAd->StaCfg.TdlsSearchEntryFreeList);
        DBGPRINT(RT_DEBUG_TRACE, ("TdlsSearchEntyAlloc: pAd->StaCfg.TdlsSearchEntryFreeList.Number=%d\n", pAd->StaCfg.TdlsSearchEntryFreeList.Number));
    }
    NdisReleaseSpinLock(&pAd->StaCfg.TdlsSearchTabLock);

    return pTdlsSearchEntry;
}

VOID
TdlsSearchEntyFree(
    IN PMP_ADAPTER pAd,
    IN PTDLS_SEARCH_ENTRY pTdlsSearchEntry)
{
    NdisAcquireSpinLock(&pAd->StaCfg.TdlsSearchTabLock);
    if (pAd->StaCfg.TdlsSearchEntryFreeList.Number < TDLS_SEARCH_POOL_SIZE)
    {
        InsertTailQueue(&pAd->StaCfg.TdlsSearchEntryFreeList, (PQUEUE_ENTRY)pTdlsSearchEntry);
        DBGPRINT(RT_DEBUG_TRACE, ("Free to TdlsSearchEntryFreeList: (Number=%d) \n", pAd->StaCfg.TdlsSearchEntryFreeList.Number));
    }
    NdisReleaseSpinLock(&pAd->StaCfg.TdlsSearchTabLock);
}

BOOLEAN
TdlsSearchEntryLookup(
    IN PMP_ADAPTER pAd,
    IN PUCHAR pMac)
{
    UINT8 HashId = MAC_ADDR_HASH(pMac) % TDLS_SEARCH_HASH_TAB_SIZE ;
    PTDLS_SEARCH_ENTRY pTdlsSearchEntry;

        

    pTdlsSearchEntry =
        (PTDLS_SEARCH_ENTRY)pAd->StaCfg.TdlsSearchTab[HashId].Head;
    while (pTdlsSearchEntry)
    {
        if (MAC_ADDR_EQUAL(pMac, pTdlsSearchEntry->Addr))
            return TRUE;

        pTdlsSearchEntry = pTdlsSearchEntry->pNext;
        break;
    }
    return FALSE;
}

BOOLEAN
TdlsSearchEntryUpdate(
    IN PMP_ADAPTER pAd,
    IN PUCHAR pMac)
{
    UINT8 HashId = MAC_ADDR_HASH(pMac) % TDLS_SEARCH_HASH_TAB_SIZE ;
    PTDLS_SEARCH_ENTRY pTdlsSearchEntry;


    // TODO:  We might get the same hash id from two different MACs!!!
    
    if (TdlsSearchEntryLookup(pAd, pMac) == TRUE)
        return FALSE;

    // This Id has been used. Skip it.
    if (pAd->StaCfg.TdlsSearchTab[HashId].Head)
        return FALSE;

    pTdlsSearchEntry = TdlsSearchEntyAlloc(pAd);
    if (pTdlsSearchEntry)
    {
        ULONGLONG           Now64;
        NdisGetCurrentSystemTime((PLARGE_INTEGER)&Now64);

        COPY_MAC_ADDR(pTdlsSearchEntry->Addr, pMac);
        pTdlsSearchEntry->LastRefTime = Now64;
        pTdlsSearchEntry->RetryCount = 0;   
        pTdlsSearchEntry->pNext = NULL;
        InsertTailQueue(&pAd->StaCfg.TdlsSearchTab[HashId], (PQUEUE_ENTRY)pTdlsSearchEntry);
        DBGPRINT(RT_DEBUG_TRACE, ("TDLS - Add %02x:%02x:%02x:%02x:%02x:%02x to TdlsSearchTab[%d] (RetryCount=%d)\n", pTdlsSearchEntry->Addr[0],pTdlsSearchEntry->Addr[1],pTdlsSearchEntry->Addr[2], pTdlsSearchEntry->Addr[3], pTdlsSearchEntry->Addr[4], pTdlsSearchEntry->Addr[5], HashId, pTdlsSearchEntry->RetryCount));

        return TRUE;
    }

    return FALSE;
}

BOOLEAN
TdlsSearchEntryDelete(
    IN PMP_ADAPTER pAd,
    IN PUCHAR pMac)
{
    UINT8 HashId = MAC_ADDR_HASH(pMac) % TDLS_SEARCH_HASH_TAB_SIZE ;
    PTDLS_SEARCH_ENTRY pTdlsSearchEntry;
    

    while (pAd->StaCfg.TdlsSearchTab[HashId].Head)
    {
        pTdlsSearchEntry = (PTDLS_SEARCH_ENTRY)(pAd->StaCfg.TdlsSearchTab[HashId].Head);
        
        if (MAC_ADDR_EQUAL(pMac, pTdlsSearchEntry->Addr))
        {
            NdisAcquireSpinLock(&pAd->StaCfg.TdlsSearchTabLock);
            pTdlsSearchEntry = (PTDLS_SEARCH_ENTRY)RemoveHeadQueue(&pAd->StaCfg.TdlsSearchTab[HashId]);
            NdisReleaseSpinLock(&pAd->StaCfg.TdlsSearchTabLock);
        
            DBGPRINT(RT_DEBUG_TRACE, ("TDLS - Remove %02x:%02x:%02x:%02x:%02x:%02x from TdlsSearchTab[%d] \n", pTdlsSearchEntry->Addr[0],pTdlsSearchEntry->Addr[1],pTdlsSearchEntry->Addr[2], pTdlsSearchEntry->Addr[3], pTdlsSearchEntry->Addr[4], pTdlsSearchEntry->Addr[5], HashId));
            TdlsSearchEntyFree(pAd, pTdlsSearchEntry);
            break;
        }
        else
        {
            DBGPRINT(RT_DEBUG_ERROR, ("TDLS - Got the same HashId (pMac = %02x:%02x:%02x:%02x:%02x:%02x )\n", *pMac,*(pMac+1),*(pMac+2), *(pMac+3), *(pMac+4), *(pMac+5)));
            DBGPRINT(RT_DEBUG_TRACE, ("TDLS - Remove %02x:%02x:%02x:%02x:%02x:%02x from TdlsSearchTab[%d] \n", pTdlsSearchEntry->Addr[0],pTdlsSearchEntry->Addr[1],pTdlsSearchEntry->Addr[2], pTdlsSearchEntry->Addr[3], pTdlsSearchEntry->Addr[4], pTdlsSearchEntry->Addr[5], HashId));
            TdlsSearchEntyFree(pAd, pTdlsSearchEntry);
            return FALSE;
        }
            
    }
    InitializeQueueHeader(&pAd->StaCfg.TdlsSearchTab[HashId]);


    return TRUE;

}


VOID
TdlsSearchTabMaintain(
    IN PMP_ADAPTER pAd)
{
    ULONG idx, j;
    PTDLS_SEARCH_ENTRY pTdlsSearchEntry;
    ULONGLONG           Now64;
    PTDLS_SEARCH_ENTRY pTdlsSearchEntryNext;
    PQUEUE_HEADER       pTdlsQueue;
    PMP_PORT          pPort = pAd->PortList[PORT_0];

    NdisGetCurrentSystemTime((PLARGE_INTEGER)&Now64);
    for (idx = 0; idx < TDLS_SEARCH_HASH_TAB_SIZE; idx++)
    {
        pTdlsQueue = (PQUEUE_HEADER)&pAd->StaCfg.TdlsSearchTab[idx];
        pTdlsSearchEntry = (PTDLS_SEARCH_ENTRY)(pTdlsQueue->Head);

        while (pTdlsSearchEntry)
        {
            pTdlsSearchEntryNext = pTdlsSearchEntry->pNext;

            if ((Now64 > (pTdlsSearchEntry->LastRefTime + (TDLS_SEARCH_ENTRY_AGEOUT * ONE_SECOND_TIME))) &&
                (pTdlsSearchEntry->RetryCount < TDLS_SEARCH_ENTRY_RETRY_TIMES))
            {
                ULONGLONG   Time64;
                RT_802_11_DLS_UI    TdlsUi;

                NdisGetCurrentSystemTime((PLARGE_INTEGER)&Time64);
                pTdlsSearchEntry->LastRefTime = Time64;

                DBGPRINT(RT_DEBUG_WARN, ("TDLS - ageout %02x:%02x:%02x:%02x:%02x:%02x from after %d-sec silence retry %d times\n",
                                            pTdlsSearchEntry->Addr[0],pTdlsSearchEntry->Addr[1],pTdlsSearchEntry->Addr[2],
                                            pTdlsSearchEntry->Addr[3],pTdlsSearchEntry->Addr[4],pTdlsSearchEntry->Addr[5], 
                                            TDLS_SEARCH_ENTRY_AGEOUT, pTdlsSearchEntry->RetryCount));

                PlatformZeroMemory(&TdlsUi, sizeof(RT_802_11_DLS_UI));
                TdlsUi.TimeOut = 0;
                COPY_MAC_ADDR(TdlsUi.MacAddr, pTdlsSearchEntry->Addr);
                TdlsUi.Valid = TRUE;

                MlmeEnqueue(pAd,
                            pPort,
                            MLME_CNTL_STATE_MACHINE, 
                            MTK_OID_N6_SET_DLS_PARAM, 
                            sizeof(RT_802_11_DLS_UI), 
                            &TdlsUi);
                pTdlsSearchEntry->RetryCount++;
            }
            else if (pTdlsSearchEntry->RetryCount == TDLS_SEARCH_ENTRY_RETRY_TIMES)
            {                   
                // clear this TDLS entry
                NdisAcquireSpinLock(&pAd->StaCfg.TDLSEntryLock);
                for (j=0; j<MAX_NUM_OF_TDLS_ENTRY; j++)
                {
                    if (MAC_ADDR_EQUAL(pAd->StaCfg.TDLSEntry[j].MacAddr, pTdlsSearchEntry->Addr) &&
                        (pAd->StaCfg.TDLSEntry[j].Valid == TRUE )&& 
                        (pAd->StaCfg.TDLSEntry[j].Status < TDLS_MODE_CONNECTED))
                    {
                        pAd->StaCfg.TDLSEntry[j].Valid = FALSE;
                        pAd->StaCfg.TDLSEntry[j].Status = TDLS_MODE_NONE;
                        break;
                    }   
                }
                NdisReleaseSpinLock(&pAd->StaCfg.TDLSEntryLock);

                // skip this condition  
                pTdlsSearchEntry->RetryCount++;

            }
            // We already reset searching table while link down         
            /*else if (Now64 > (pTdlsSearchEntry->LastRefTime + (ULONGLONG)TDLS_SEARCH_ENTRY_LIFTTIME * (ULONGLONG)ONE_SECOND_TIME))
            {
                DBGPRINT(RT_DEBUG_WARN, ("TDLS - lifetime expired on SearchTab %02x:%02x:%02x:%02x:%02x:%02x after %d-sec\n",
                                            pTdlsSearchEntry->Addr[0],pTdlsSearchEntry->Addr[1],pTdlsSearchEntry->Addr[2],
                                            pTdlsSearchEntry->Addr[3],pTdlsSearchEntry->Addr[4],pTdlsSearchEntry->Addr[5], 
                                            TDLS_SEARCH_ENTRY_LIFTTIME));
                NdisAcquireSpinLock(&pAd->StaCfg.TdlsSearchTabLock);
                pTdlsSearchEntryNext = (PTDLS_SEARCH_ENTRY)RemoveHeadQueue(pTdlsQueue);
                NdisReleaseSpinLock(&pAd->StaCfg.TdlsSearchTabLock);

                TdlsSearchEntyFree(pAd, pTdlsSearchEntry);
                break;
            }*/
 
            pTdlsSearchEntry = pTdlsSearchEntryNext;
            break;
        }
    }

}

/*
==========================================================================
    Description:
        
    IRQL = PASSIVE_LEVEL
==========================================================================
*/
VOID TdlsLinkMaintenance(
    IN PMP_ADAPTER pAd)
{
    UCHAR           idx;
    PRT_802_11_TDLS pTDLS = NULL;
    USHORT          Reason;
    PMP_PORT      pPort = pAd->PortList[PORT_0];
    
    for (idx = 0; idx < MAX_NUM_OF_TDLS_ENTRY; idx++)
    {
        pTDLS = (PRT_802_11_TDLS)&pAd->StaCfg.TDLSEntry[idx];

        if ((pTDLS->Valid == TRUE) && (pTDLS->Status >= TDLS_MODE_CONNECTED))
        {
            UCHAR wcid = pTDLS->Wcid;
            PMAC_TABLE_ENTRY pEntry = MlmeSyncMacTabMatchedWlanIdx(pPort, wcid);   
            MLME_TDLS_REQ_STRUCT    MlmeTdlsReq;
            MP_RW_LOCK_STATE    LockState;

            if(pEntry == NULL)
            {
                DBGPRINT(RT_DEBUG_TRACE,("%s(), Line:%d, Mac Entry is NULL  Wcid = %d\n", __FUNCTION__, __LINE__,wcid));
                return;
            }

            if ((pEntry->ValidAsCLI == FALSE) || (wcid != pEntry->Aid))
                continue;
            
            PlatformAcquireOldRWLockForWrite(&pAd->MacTablePool.MacTabPoolLock, &LockState);
            pEntry->NoDataIdleCount++;
            PlatformReleaseOldRWLock(&pAd->MacTablePool.MacTabPoolLock, &LockState);

            // delete those TDLS entry that has been idle for a long time
            if (pEntry->NoDataIdleCount >= TDLS_ENTRY_AGEOUT_TIME)
            {
                DBGPRINT(RT_DEBUG_WARN, ("ageout %02x:%02x:%02x:%02x:%02x:%02x from TDLS #%d after %d-sec silence\n",
                                            pEntry->Addr[0],pEntry->Addr[1],pEntry->Addr[2],pEntry->Addr[3],
                                            pEntry->Addr[4],pEntry->Addr[5], idx, TDLS_ENTRY_AGEOUT_TIME));

                NdisAcquireSpinLock(&pAd->StaCfg.TDLSEntryLock);
                pTDLS->Valid = FALSE;
                pTDLS->Status = TDLS_MODE_NONE;
                NdisReleaseSpinLock(&pAd->StaCfg.TDLSEntryLock);

                Reason = REASON_TDLS_UNACCEPTABLE_FRAME_LOSS;
                PlatformZeroMemory(&MlmeTdlsReq, sizeof(MLME_TDLS_REQ_STRUCT));
                MlmeCntTdlsParmFill(pAd, &MlmeTdlsReq, pTDLS, Reason);
                MlmeEnqueue(pAd, pPort, TDLS_STATE_MACHINE, MT2_MLME_TDLS_TEAR_DOWN, sizeof(MLME_TDLS_REQ_STRUCT), &MlmeTdlsReq);
            }
        }
    }
}

/*
    ==========================================================================
    Description:
        When the host starts to send a packet to the target, it shall 
        accompany with a TDLS setup request frame. We will try sereval times to 
        wait a TDLS setup response from the target.   
    ==========================================================================
 */
 VOID
TdlsDiscoveryPacketLearnAndCheck(
    IN  PMP_ADAPTER   pAd,
    IN  PUCHAR          pAddr)
{
    PTDLS_SEARCH_ENTRY  pEntry = NULL;
    BOOLEAN             bValid = FALSE;
    PMP_PORT          pPort = pAd->PortList[PORT_0];
    INT                 Idx = -1;
    
    if (pAd->StaCfg.pTdlsSearchEntryPool == NULL)
    {
        DBGPRINT(RT_DEBUG_ERROR, ("%s : pAd->StaCfg.pTdlsSearchEntryPool = NULL", __FUNCTION__));
        return;
    }
    
    DBGPRINT(RT_DEBUG_INFO, ("====> TdlsDiscoveryPacketLearnAndCheck\n"));


    // Make sure only one thread runs peer discovery at the same time.
    NdisAcquireSpinLock(&pAd->StaCfg.TdlsDiscoveyQueueLock);
    if (pAd->StaCfg.bTdlsDiscoveyInUse)
    {
        NdisReleaseSpinLock(&pAd->StaCfg.TdlsDiscoveyQueueLock);
        return;
    }
    else
    {
        pAd->StaCfg.bTdlsDiscoveyInUse = TRUE;
        NdisReleaseSpinLock(&pAd->StaCfg.TdlsDiscoveyQueueLock);
    }

    bValid = TdlsSearchEntryUpdate(pAd, pAddr);
    Idx = TdlsCheckTdlsFrame(pAd, pAddr);   // Check if non-connected TDLS again.
    
    if ((bValid) && (Idx < 0))
    {
        RT_802_11_DLS_UI    TdlsUi;

        PlatformZeroMemory(&TdlsUi, sizeof(RT_802_11_DLS_UI));
        TdlsUi.TimeOut = 0;
        COPY_MAC_ADDR(TdlsUi.MacAddr, pAddr);
        TdlsUi.Valid = TRUE;

        MlmeEnqueue(pAd,
                    pPort,
                    MLME_CNTL_STATE_MACHINE, 
                    MTK_OID_N6_SET_DLS_PARAM, 
                    sizeof(RT_802_11_DLS_UI), 
                    &TdlsUi);
    }


    NdisAcquireSpinLock(&pAd->StaCfg.TdlsDiscoveyQueueLock);
    pAd->StaCfg.bTdlsDiscoveyInUse = FALSE;
    NdisReleaseSpinLock(&pAd->StaCfg.TdlsDiscoveyQueueLock);

    DBGPRINT(RT_DEBUG_INFO, ("<==== TdlsDiscoveryPacketLearnAndCheck (Idx=%d, bValid=%d)\n", Idx, bValid));

}

