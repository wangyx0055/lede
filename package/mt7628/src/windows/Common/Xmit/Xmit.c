#include    "MtConfig.h"

#define DRIVER_FILE         0x01600000

// Match total 6 bulkout endpoint to corresponding queue.
UCHAR   EpToQueue[6]={FIFO_EDCA, FIFO_EDCA, FIFO_EDCA, FIFO_EDCA, FIFO_EDCA, FIFO_MGMT};

UCHAR   IPX[] = {0x81, 0x37};
UCHAR   APPLE_TALK[] = {0x80, 0xf3};

UCHAR    OfdmRateToRxwiMCS[12] = {
    0,  0,  0,  0,   
    0,  1,  2,  3,  // OFDM rate 6,9,12,18 = rxwi mcs 0,1,2,3
    4,  5,  6,  7,  // OFDM rate 24,36,48,54 = rxwi mcs 4,5,6,7
};

UCHAR MapUserPriorityToAccessCategory[8] = {QID_AC_BE, QID_AC_BK, QID_AC_BK, QID_AC_BE, QID_AC_VI, QID_AC_VI, QID_AC_VO, QID_AC_VO};



NDIS_STATUS
XmitSendPacket(
    IN  PMP_ADAPTER       pAd,    
    IN  NDIS_PORT_NUMBER    PortNumber,
    IN  PMT_XMIT_CTRL_UNIT             pXcu
    )
{
    PMP_PORT          pPort = NULL;
    NDIS_STATUS         ndisStatus;
    UCHAR               OpMode = OPMODE_STA;
    

    pPort = pAd->PortList[PortNumber];
    if (pPort == NULL)
    {
        DBGPRINT(RT_DEBUG_ERROR, ("%s - pPort is  NULL, Error, and return this function", __FUNCTION__));
        return (NDIS_STATUS_FAILURE);
    }

    if (pPort->PortSubtype == PORTSUBTYPE_SoftapAP || pPort->PortSubtype == PORTSUBTYPE_VwifiAP || (pPort->PortSubtype == PORTSUBTYPE_P2PGO))
        OpMode = OPMODE_AP;

    if (OpMode == OPMODE_AP)
        ndisStatus = ApDataSendPacket(pAd, PortNumber, pXcu);
    else
        ndisStatus = XmitStaSendPacket(pAd, PortNumber, pXcu, FALSE);

    return ndisStatus;
}

NDIS_STATUS
XmitStaSendPacket(
    IN  PMP_ADAPTER       pAd,    
    IN  NDIS_PORT_NUMBER    PortNumber,
    IN  PMT_XMIT_CTRL_UNIT             pXcu,
    IN  BOOLEAN            bAironetFrame
    )
{   
    PQUEUE_HEADER   pTxQueue;
    UCHAR           QueIdx, UserPriority;
    UCHAR           NumberOfFrag;
    UCHAR           RTSRequired;
    UINT            AllowFragSize;
    ULONG           TotalPacketLength = pXcu->TotalPacketLength;
    PMP_PORT      pPort = pAd->PortList[PortNumber];
    size_t  PipeNumber =0;
    BOOLEAN         bTdlsActive = FALSE;
    PMAC_TABLE_ENTRY pWfdMacTabEntry = MlmeSyncMacTabMatchedRoleType(pPort, ROLE_WLANIDX_P2P_CLIENT);
#ifdef MULTI_CHANNEL_SUPPORT
    UCHAR           Status = 0;
#endif /*MULTI_CHANNEL_SUPPORT*/

    pPort = pAd->PortList[PortNumber];

    ASSERT(TotalPacketLength <= MAX_FRAME_SIZE);
    
    if (OPSTATUS_TEST_FLAG(pAd, fOP_STATUS_AGGREGATION_INUSED))
    {
        NumberOfFrag = 1;   // Aggregation overwhelms fragmentation
    }
    else if ((READ_PHY_CFG_MODE(pAd, &pPort->CommonCfg.TxPhyCfg) == MODE_HTMIX) || 
                (READ_PHY_CFG_MODE(pAd, &pPort->CommonCfg.TxPhyCfg) == MODE_HTGREENFIELD) || 
                (READ_PHY_CFG_MODE(pAd, &pPort->CommonCfg.TxPhyCfg) == MODE_VHT))
    {
        NumberOfFrag = 1;   // MIMO RATE overwhelms fragmentation
    }
    else
    {
        // Check for payload allowed for each fragment
        AllowFragSize = pPort->CommonCfg.FragmentThreshold - LENGTH_802_11 - LENGTH_CRC;

        // Calculate fragments required
        NumberOfFrag = ((TotalPacketLength - LENGTH_802_11) / AllowFragSize) + 1;
        // Minus 1 if the size just match to allowable fragment size
        if (((TotalPacketLength - LENGTH_802_11) % AllowFragSize) == 0)
        {
            NumberOfFrag--;
        }
    }

    // Check for requirement of RTS 
    if (NumberOfFrag > 1)
    {
        // If multiple fragment required, RTS is required only for the first fragment
        // if the fragment size large than RTS threshold
        RTSRequired = (pPort->CommonCfg.FragmentThreshold > pPort->CommonCfg.RtsThreshold) ? 1 : 0;
    }
    else
    {
        RTSRequired = (TotalPacketLength > pPort->CommonCfg.RtsThreshold) ? 1 : 0;
    }
    DBGPRINT(RT_DEBUG_INFO, ("Number of fragments include RTS :%d\n", NumberOfFrag + RTSRequired));

    // RTS/CTS may also be required in order to protect OFDM frame
    if ((pPort->CommonCfg.TxRate >= RATE_FIRST_OFDM_RATE) && OPSTATUS_TEST_FLAG(pAd, fOP_STATUS_BG_PROTECTION_INUSED))
    {
        RTSRequired = 0;
    }
    // Save fragment number to Ndis packet reserved field
    pXcu->NumberOfFrag = NumberOfFrag;

    RTSRequired = 0;    // Let ASIC  send RTS/CTS, so we don't set RTS via Ndis Netbuffer reserved field

    //
    // STEP 3. Traffic classification. outcome = <UserPriority, QueIdx>
    //
    UserPriority = 0;
    QueIdx       = QID_AC_BE;


    // Check if there is any active peer TDLS STA.
    if (TDLS_ON(pAd) && INFRA_ON(pPort))
    {   
        UCHAR       i;

        for (i=0; i<MAX_NUM_OF_TDLS_ENTRY; i++)
        {
            if ((pAd->StaCfg.TDLSEntry[i].Valid == TRUE) && (pAd->StaCfg.TDLSEntry[i].Status >= TDLS_MODE_CONNECTED))
            {
                bTdlsActive = TRUE;
                break;
            }
        }
    }

    PipeNumber = NdisCommonGetPipeNumber(pAd);
    // just comment for emulation
    if ((OPSTATUS_TEST_FLAG(pPort, fOP_STATUS_WMM_INUSED) || ( bTdlsActive == TRUE)) && (PipeNumber >= 5))
    {
        //
        // Get 802.1p
        //
        UserPriority = pXcu->UserPriority; 
        QueIdx = pXcu->QueIdx;


            // When P2PVideoBoost is enabled, force to VO in Ralink P2P Group.
            if ((pPort->CommonCfg.P2PVideoBoost) &&
                (pPort->PortNumber == pPort->P2PCfg.PortNumber) &&
                (IS_P2P_OP(pPort)) &&((pWfdMacTabEntry !=NULL) &&
                (CLIENT_STATUS_TEST_FLAG(pWfdMacTabEntry, fCLIENT_STATUS_RALINK_CHIPSET))))
            {
                UserPriority = 7;
                QueIdx = QID_AC_VO;
            }
        DBGPRINT(RT_DEBUG_INFO, ("RTXmitSendPacket(UserPriority, QueIdx) = (%d, %d)\n", UserPriority, QueIdx));
    }

#ifdef MULTI_CHANNEL_SUPPORT
    // Assign correct SW queue number for multichannel operation
    if (pAd->MccCfg.MultiChannelEnable == TRUE)
    {
        if (pPort->PortType == EXTSTA_PORT)
        {
            QueIdx = MultiChannelGetSwQ(pPort->P2PCfg.STAHwQSEL);
            Status = 1;
        }
        else if (pPort->PortType == WFD_CLIENT_PORT)
        {
            QueIdx = MultiChannelGetSwQ(pPort->P2PCfg.CLIHwQSEL);
            Status= 2;
        }
        else if (pPort->PortType == WFD_GO_PORT)
        {
            QueIdx = MultiChannelGetSwQ(pPort->P2PCfg.GOHwQSEL);
            Status = 3;
        }       
        else
        {
            QueIdx = QID_AC_BE;
            Status = 4;
        }
        DBGPRINT(RT_DEBUG_TRACE, ("%s - In channel switch: Insert Port(%d), Status(%d) data to TxSwQueue[%d]\n", __FUNCTION__, pPort->PortNumber, Status, QueIdx));
    }
    // STA in TX/RX and P2pMs session
    else if (MT_TEST_BIT(pAd->pP2pCtrll->MultiChannelStartOrEndProcess, MULTI_CHANNEL_IN_NEW_SESSION))
    {
        if ((pPort->PortType == EXTSTA_PORT) && 
            INFRA_OP(pAd->PortList[PORT_0]) &&
            MT_TEST_BIT(pAd->pP2pCtrll->MultiChannelStartOrEndProcess, MULTI_CHANNEL_STA_DHCP_DONE))
        {
            QueIdx = MultiChannelGetSwQ(pPort->P2PCfg.STAHwQSEL);
            Status = 5;
        }
        else if ((pPort->PortType == WFD_CLIENT_PORT) && 
            MT_TEST_BIT(pPort->P2PCfg.P2pMsConnectedStatus, P2pMs_CONNECTED_STATUS_CLI_OP) &&
            MT_TEST_BIT(pAd->pP2pCtrll->MultiChannelStartOrEndProcess, MULTI_CHANNEL_CLI_DHCP_DONE))  
        {
            QueIdx = MultiChannelGetSwQ(pPort->P2PCfg.CLIHwQSEL);
            Status = 6;
        }
        else if ((pPort->PortType == WFD_GO_PORT) && MT_TEST_BIT(pPort->P2PCfg.P2pMsConnectedStatus, P2pMs_CONNECTED_STATUS_GO_OP)) 
        {
#if 0           
#else
            if (MT_TEST_BIT(pAd->pP2pCtrll->MultiChannelStartOrEndProcess, MULTI_CHANNEL_CLI_IN_NEW_CONNECTION_START))
            {
                QueIdx = QID_HCCA - 1;
                Status = 7;
            }
            else
            {
                QueIdx = MultiChannelGetSwQ(pPort->P2PCfg.GOHwQSEL);
                Status = 8;         
            }
#endif
        }
        else
        {
            QueIdx = QID_HCCA - 1;
            Status = 10;
        }
        DBGPRINT(RT_DEBUG_TRACE, ("%s - During new session: Insert Port(%d), Status(%d) data to TxSwQueue[%d]\n", __FUNCTION__, pPort->PortNumber, Status, QueIdx));
    }
    else
    {
        // Mark for TGn WMM test
        //QueIdx = QID_AC_BE;
        DBGPRINT(RT_DEBUG_INFO, ("%s - No channel switch: Insert Port(%d) data to TxSwQueue[%d]\n", __FUNCTION__, pPort->PortNumber, QueIdx));
    }

#endif /*MULTI_CHANNEL_SUPPORT*/

    // emulation
    //QueIdx = 3,
    //UserPriority = 0;
    //
    pXcu->UserPriority = UserPriority; 
    pXcu->QueIdx = QueIdx;
    pXcu->OpMode = OPMODE_STA;
    pXcu->PortNumber = PortNumber;

    // Make sure SendTxWait queue resource won't be used by other threads
    NdisAcquireSpinLock(&pAd->pTxCfg->SendTxWaitQueueLock[QueIdx]);

    pTxQueue = &pAd->pTxCfg->SendTxWaitQueue[QueIdx];
    //
    // Because of the timing criteria of CCX test, Aironet frame shall be sent as soon as possible
    // 
    if (bAironetFrame)
    {
        // Enqueue Aironet packet to head of Tx wait queue
        InsertHeadQueue(pTxQueue, pXcu);
        DBGPRINT(RT_DEBUG_TRACE, ("RTXmitSendPacket -> Enqueue Aironet frame to head of Tx wait queue!\n"));
    }
    else
    {
        InsertTailQueue(pTxQueue, pXcu);
    }
    NdisReleaseSpinLock(&pAd->pTxCfg->SendTxWaitQueueLock[QueIdx]);

    MT_INC_REF(&pAd->pTxCfg->NumQueuePacket);
    pAd->Counter.MTKCounters.OneSecOsTxCount[QueIdx]++;      
    return (NDIS_STATUS_SUCCESS);   
}

VOID
XmitSendMlmeCmdPkt(
    IN  PMP_ADAPTER       pAd,
    IN  MLME_PKT_TYPE    PktType
    )
{
  
#if IS_BUS_TYPE_PCI(DEV_BUS_TYPE)

#elif IS_BUS_TYPE_USB(DEV_BUS_TYPE)
    N6USBXmitSendMlmeCmdPkt(pAd, PktType);
#elif IS_BUS_TYPE_SDIO(DEV_BUS_TYPE)
    
#endif    
}


// MT7603 FPGA
NDIS_STATUS 
XmitHardTransmit(
    IN  PMP_ADAPTER   pAd,
    IN  PMT_XMIT_CTRL_UNIT     pXcu
    )
{   
//==================================
    NDIS_STATUS                         ndisStatus = NDIS_STATUS_SUCCESS;
    PMP_PORT                      pPort = pAd->PortList[pXcu->PortNumber];
    HEADER_802_11                   Header_802_11;
    ULONG                           Remain;
    UINT                            BytesCopied;
    UINT                            TxSize;
    UINT                            InitFreeMpduSize, FreeMpduSize;
    INT                             SrcRemainingBytes;
    ULONG                           TotalPacketLength;
    PUCHAR                          pDA;
    PHEADER_802_11                  pHeader80211 = NULL;    
    PUCHAR                          pDest;
    PHEADER_802_11                  pDest80211Hdr = NULL;   
    PHT_TX_CONTEXT                  pTxContext;
    PTXDSCR_SHORT_STRUC             pShortFormatTxD = NULL;
    PTXDSCR_LONG_STRUC          pLongFormatTxD = NULL;
    PTXDSCR_DW0                 pTxDDW0 = NULL;
    PTXDSCR_DW7                 pTxDDW7 = NULL;
    BOOLEAN                         StartOfFrame;
    BOOLEAN                         MICFrag;
    PCIPHER_KEY                     pKey = NULL ;
    ULONG                           SrcBufLen= 0;
    PUCHAR                          pExtraLlcSnapEncap = NULL; // NULL: no extra LLC/SNAP is required
    PUCHAR                          pWirelessPacket;
    ULONG                           DataOffSet = 0;
    ULONG                           NextMpduSize;
    ULONG                           FillOffset;
    BOOLEAN                         bIncreaseCurWriPos = FALSE;
    PDOT11_EXTSTA_SEND_CONTEXT  pDot11SendContext = NULL;
    PUCHAR                          SysVa,SysVaRaTxAgg = NULL; //pSrcBufVA
    ULONG                           SafeModeEnable_MDLOffset = 0;   
    int                             DlsEntryIndex, TdlsEntryIndex;
    //WAPI
    PUCHAR                          pEncrypt = NULL;

    // AdhocN Support
    PMAC_TABLE_ENTRY                pEntry = NULL;
    BOOLEAN                         IsPeerExist = FALSE;                            


    PTXWI_STRUC     pTxWI = NULL;   // to remove later
    PTXINFO_STRUC   pTxInfo = NULL;  // to remove later

    ULONG temp = 0;
    USHORT  BufferIdx =0;
    PMAC_TABLE_ENTRY pWcidMacTabEntry = NULL;
    UCHAR   QueIdx =0;

//==================================
    HARD_TRANSMIT_INFO              TxInfo;
//==================================

     // Instead p2p wcid if I am concurrent P2P Client.
   

    PlatformZeroMemory((PVOID)&TxInfo, sizeof(HARD_TRANSMIT_INFO));

     if ((IS_P2P_CON_CLI(pAd, pPort) || IS_P2P_MS_CLI(pAd, pPort) || IS_P2P_MS_CLI2(pAd, pPort))/* && (INFRA_ON(pPort))*/)
    {
        TxInfo.Wcid = P2pGetClientWcid(pAd, pPort);
        pWcidMacTabEntry = MlmeSyncMacTabMatchedWlanIdx(pPort, TxInfo.Wcid);
        if(pWcidMacTabEntry == NULL)
        {
            DBGPRINT(RT_DEBUG_TRACE,("%s(), Line:%d, Mac Entry is NULL  Wcid = %d\n", __FUNCTION__, __LINE__,TxInfo.Wcid));
            return NDIS_STATUS_FAILURE;
        }
        
        TxInfo.PortSecured = (UCHAR)pWcidMacTabEntry->PortSecured;
    }
    else
    {
        TxInfo.Wcid = (UCHAR)MlmeSyncGetWlanIdxByPort(pPort, ROLE_WLANIDX_BSSID);
    }

    TxInfo.QueIdx = pXcu->QueIdx;
    TxInfo.PortNumber = (UCHAR)pXcu->PortNumber;
    TxInfo.FrameGap = IFS_HTTXOP;
    TxInfo.LengthQosPAD = 0;
    TxInfo.This80211HdrLen = LENGTH_802_11;
    TxInfo.Protocol = 0;
    TxInfo.bEAPOLFrame = FALSE;
    TxInfo.AckDuration = 0;
    TxInfo.EncryptionOverhead = 0;
    TxInfo.MimoPs = MMPS_ENABLE;
    TxInfo.bTXBA = FALSE; // TRUE if use block ack.
    TxInfo.BAWinSize = 0;
    TxInfo.bHTC = FALSE; // TRUE if HT AMSDU used for this MPDU.
    TxInfo.bWMM = FALSE; // TRUE if HT AMSDU used for this MPDU.
    TxInfo.bHtVhtFrame = FALSE;    
    TxInfo.RABAOriIdx = 0;  //The RA's BA Originator table index. 
    TxInfo.bPiggyBack = FALSE;
    TxInfo.bAMSDU = FALSE;// use HT AMSDU
    TxInfo.bGroupMsg2 = FALSE; // is this a group msg 2? for CCX CCKM   
    TxInfo.bDHCPv6 = FALSE;
    TxInfo.bDHCPFrame = FALSE;
    TxInfo.NextValid = 0;
    TxInfo.bDLSFrame = FALSE;
    TxInfo.bTDLSFrame = FALSE;
    TxInfo.ExemptionActionType = DOT11_EXEMPT_NO_EXEMPTION;
    TxInfo.bWAIFrame = FALSE;
    TxInfo.bIsUnicastFrame = TRUE;
    TxInfo.bIsLastFrag = FALSE;
    TxInfo.bIsFrag = FALSE;
    TxInfo.bWiv = FALSE;
    TxInfo.PortSecured = pAd->StaCfg.PortSecured;
    TxInfo.SwUseSegIdx = 1;
    TxInfo.FragmentByteForTxD = 0;
    TxInfo.CipherAlg = CIPHER_NONE;
    TxInfo.TxRate = RATE_1;
    TxInfo.MpduRequired = 0;
    TxInfo.UserPriority = 0;

//==================================

    DBGPRINT(RT_DEBUG_INFO,("RTMPHardTransmit --> pPort->CommonCfg.bAutoTxRateSwitch = %d\n", pPort->CommonCfg.bAutoTxRateSwitch));

    if ((pPort->CommonCfg.bIEEE80211H == 1) && (pPort->CommonCfg.RadarDetect.RDMode != RD_NORMAL_MODE))
    {
        DBGPRINT(RT_DEBUG_INFO,("XmitHardTransmit --> radar detect not in normal mode !!!\n"));
        return (NDIS_STATUS_FAILURE);
    }

    if ((pPort->CommonCfg.bCarrierDetect) && (pPort->CommonCfg.bCarrierAssert))
    {
        DBGPRINT(RT_DEBUG_INFO,("RTMPHardTransmit --> carrier assertion !!!\n"));
        return (NDIS_STATUS_FAILURE);
    }

    // there's packet to be sent, keep awake for 1200ms
    if (pAd->TrackInfo.CountDowntoPsm < 12)
    {
        DBGPRINT(RT_DEBUG_TRACE,("RTMPHardTransmit --> pAd->TrackInfo.CountDowntoPsm = 12 !!!\n"));
        pAd->TrackInfo.CountDowntoPsm = 12;
    }

    TxInfo.TxRate = pPort->CommonCfg.TxRate;
    TxInfo.MpduRequired = pXcu->FragmentRequired;
    TxInfo.UserPriority = pXcu->UserPriority;
    TotalPacketLength = pXcu->TotalPacketLength;

    if ((TotalPacketLength <= LENGTH_802_11) || (TotalPacketLength > MAX_FRAME_SIZE))
    {
        DBGPRINT(RT_DEBUG_ERROR, ("XmitHardTransmit, size(=%d) less than 802.11 header or large than 2346\n", TotalPacketLength));
        return (NDIS_STATUS_FAILURE);
    }

    pHeader80211 = (PHEADER_802_11) pXcu->pHeader80211;
    //SysVa += LENGTH_802_11;

    if (pHeader80211->FC.ToDs)
        pDA = pHeader80211->Addr3;
    else
        pDA = pHeader80211->Addr1;

    // Check if the frame can be sent through DLS direct link interface
    // If packet can be sent through DLS, then force aggregation disable. (Hard to determine peer STA's capability)
    DlsEntryIndex = RTMPCheckDLSFrame(pAd, pPort, pDA);
    if (DlsEntryIndex >= 0)
    {
        TxInfo.bDLSFrame = TRUE;    
    }
    else
    {
        TxInfo.bDLSFrame = FALSE;
    }

    // TDLS only for None and RSN (802.11z/D5.0 @8.5.9)
    if (((pPort->PortCfg.AuthMode == Ralink802_11AuthModeOpen) && (pPort->PortCfg.WepStatus == Ralink802_11EncryptionDisabled)) || 
        (((pPort->PortCfg.AuthMode == Ralink802_11AuthModeWPA2) || (pPort->PortCfg.AuthMode == Ralink802_11AuthModeWPA2PSK)) && 
         ((pPort->PortCfg.WepStatus == Ralink802_11Encryption2Enabled) || (pPort->PortCfg.WepStatus == Ralink802_11Encryption3Enabled))))
    {   
        // Check if the frame can be sent through TDLS direct link interface
        // If packet can be sent through TDLS, then force aggregation disable. (Hard to determine peer STA's capability)
        if (TDLS_ON(pAd) && INFRA_ON(pPort) && ((*pDA & 0x01) == 0) &&
            (pXcu->PktSource!= PKTSRC_INTERNAL) &&
            (pXcu->PktSource != PKTSRC_TDLS))
        {           
            // Vista: Addr3 represents SA in ToDs frame
            TdlsEntryIndex = TdlsCheckTdlsFrame(pAd, pDA);
            if (TdlsEntryIndex >= 0)
            {   
                // It's the direct link packet, send to peer TDLS directly(Add1 = perr address)
                TxInfo.bTDLSFrame = TRUE;
            }
            else if (!PlatformEqualMemory(pDA, pPort->PortCfg.Bssid, 6))
            {
                // Enter to discovery DirectLink peer =====>>>>
                TxInfo.bTDLSFrame = FALSE;
                TdlsDiscoveryPacketLearnAndCheck(pAd, pDA);
            }
        }
    }

    // In in HT rate adhoc mode, A-MPDU is often used. So need to lookup BA Table and MAC Entry. 
    // Note multicast packets in adhoc also use BSSID_WCID index. 
    if (ADHOC_ON(pPort) ||  (TxInfo.bDLSFrame) || (TxInfo.bTDLSFrame))      // only unicast data use Block Ack
    {
        pEntry = MacTableLookup(pAd, pPort, pDA);
        if (pEntry)
        {
            TxInfo.Wcid = (UCHAR)pEntry->Aid;
            // update Originator index in adhoc mode. 
            // Becasue adhoc mode set many BA seesion with many stations.  
            // The RA's BA Originator index.
            TxInfo.RABAOriIdx = pEntry->BAOriWcidArray[TxInfo.UserPriority];
    
            IsPeerExist = TRUE;

#if 0
            // TDLS - Use this Wcid as cipher entry if security handshake has been supported.
            if (TDLS_ON(pAd) && (TxInfo.bTDLSFrame) &&
                ((pPort->PortCfg.AuthMode == Ralink802_11AuthModeWPA2) ||
                (pPort->PortCfg.AuthMode == Ralink802_11AuthModeWPA2PSK)) && 
                ((pPort->PortCfg.WepStatus == Ralink802_11Encryption2Enabled) ||
                (pPort->PortCfg.WepStatus == Ralink802_11Encryption3Enabled)))
            {
                TxInfo.CipherWcid = (UCHAR)pEntry->Aid;         
            }
#endif

        }
        else if ((*pDA & 0x01) && (ADHOC_ON(pPort))) // Multicast or Broadcast
        {
            ULONG       LowestPhyRate = 572;
            UCHAR       LowestRateRAWcid = 0;
            PMAC_TABLE_ENTRY pMacEntry = NULL;
            PMAC_TABLE_ENTRY pNextMacEntry = NULL;     
            PQUEUE_HEADER pHeader;

            LowestRateRAWcid = (UCHAR)MlmeSyncGetWlanIdxByPort(pPort, ROLE_WLANIDX_BSSID);
            // [ADHOC PER ENTRY]
            // Find out the lowest tx rate in MacEntry for multicast use                
            pHeader = &pPort->MacTab.MacTabList;
            pNextMacEntry = (PMAC_TABLE_ENTRY)(pHeader->Head);
            while (pNextMacEntry != NULL)
            {   
                pMacEntry = (PMAC_TABLE_ENTRY)pNextMacEntry;
                
                if(pMacEntry == NULL)
                {
                    break;
                }
                
                if(pMacEntry->WlanIdxRole == ROLE_WLANIDX_MBCAST)
                {
                    pNextMacEntry = pNextMacEntry->Next;   
                    pMacEntry = NULL;
                    continue; 
                }
                
                if (pMacEntry->ValidAsCLI == TRUE)
                {                       
                    if (LowestPhyRate > XmitMCSToPhyRateIn500kps(READ_PHY_CFG_MCS(pAd, &pMacEntry->TxPhyCfg), 
                                                                                               READ_PHY_CFG_MODE(pAd, &pMacEntry->TxPhyCfg), 
                                                                                               READ_PHY_CFG_BW(pAd, &pMacEntry->TxPhyCfg), 
                                                                                               0))
                    {
                        LowestPhyRate = XmitMCSToPhyRateIn500kps(READ_PHY_CFG_MCS(pAd, &pMacEntry->TxPhyCfg), 
                                                                                              READ_PHY_CFG_MODE(pAd, &pMacEntry->TxPhyCfg), 
                                                                                              READ_PHY_CFG_BW(pAd, &pMacEntry->TxPhyCfg), 
                                                                                              0);                       
                        LowestRateRAWcid = (UCHAR)pMacEntry->wcid;                           
                    }
                }
                
                pNextMacEntry = pNextMacEntry->Next;   
                pMacEntry = NULL;
            }
            
            TxInfo.Wcid = LowestRateRAWcid;

            pWcidMacTabEntry = MlmeSyncMacTabMatchedWlanIdx(pPort, TxInfo.Wcid);
            if(pWcidMacTabEntry != NULL)
            {
                DBGPRINT_RAW(RT_DEBUG_TRACE, (" ADHOC MCAST: find out the lowest tx rate used TxInfo.Wcid[%d], MODE[%d], MCS[%d], in fixed 20MHz BW\n", 
                                                TxInfo.Wcid, 
                                                READ_PHY_CFG_MODE(pAd, &pWcidMacTabEntry->TxPhyCfg), 
                                                READ_PHY_CFG_MCS(pAd, &pWcidMacTabEntry->TxPhyCfg)));  
            }
            
      
        }

        if (ADHOC_ON(pPort))
        {
            if ((pPort->PortCfg.AuthMode == Ralink802_11AuthModeWPA2PSK) && 
                (pPort->PortCfg.WepStatus == Ralink802_11Encryption3Enabled) &&
                (*pDA & 0x01))
            {
                
               TxInfo.Wcid = (UCHAR)MlmeSyncGetWlanIdxByPort(pPort, ROLE_WLANIDX_ADHOC_MCAST);          
            }
        }
    }


#if 0 // for FPGA
    // To pass P2P SIGMA tests, don't need HT Rate
    TxInfo.bHtVhtFrame = FALSE;
    if ((READ_PHY_CFG_MODE(pAd, &pPort->MacTab.Content[TxInfo.Wcid].TxPhyCfg) >= MODE_HTMIX) && (IS_P2P_SIGMA_OFF(pPort)))
    {
        TxInfo.bHtVhtFrame = TRUE;
        TxInfo.MimoPs = pPort->MacTab.Content[TxInfo.Wcid].MmpsMode;
    }
#else
    TxInfo.bHtVhtFrame = TRUE;
#endif

    //
    // Only Broadcast packet could be DHCP or ARP.
    // If DHCP datagram or ARP datagram , we need to send it as Low rates.
    //
    if (XmitCheckDHCPFrame(pAd, pXcu, &TxInfo.Protocol, &TxInfo.bGroupMsg2, &TxInfo.bDHCPv6))
    {
        if ((pDA[0] & 0x01) )
        {
            TxInfo.bDHCPFrame = TRUE;

            DBGPRINT(RT_DEBUG_ERROR, ("HardTX: DHCP frame\n",   pAd->StaCfg.MicErrCnt));
        }
    }
    else if (TxInfo.Protocol == 0x888e)
    {
        DBGPRINT(RT_DEBUG_ERROR, ("HardTX: eapol frame MicErrCnt=%d ...\n",   pAd->StaCfg.MicErrCnt));

        // Wifi Mic Err chk
        if (pAd->StaCfg.MicErrCnt >= 2)
        {
            pAd->StaCfg.MicErrCnt++;
        }
    }

    if (pPort->Channel <= 14)
    {
        //
        // Case 802.11 b/g
        // basic channel means that we can use CCKM's low rate as RATE_1.
        //      
        if (TxInfo.bDHCPFrame && (TxInfo.TxRate != RATE_1))
            TxInfo.TxRate = RATE_1;     
    }
    else
    {
        //
        // Case 802.11a
        // We must used OFDM's low rate as RATE_6, note RATE_1 is not allow
        // Only OFDM support on Channel > 14
        //
        if (TxInfo.bDHCPFrame && (TxInfo.TxRate != RATE_6))
            TxInfo.TxRate = RATE_6;
    }

    //
    // STEP 0.1 Add 802.1x protocol check.
    //
    if (TxInfo.Protocol == 0x888e)
    {
        TxInfo.bEAPOLFrame = TRUE;
        if (pAd->StaCfg.MicErrCnt >= 2)
            pAd->StaCfg.MicErrCnt++;

        QueIdx = 0;
        //TxInfo.QueIdx = 0;
        DBGPRINT(RT_DEBUG_TRACE, ("%s: bEAPOLFrame = %d. length=%d\n", __FUNCTION__, TxInfo.bEAPOLFrame, TotalPacketLength));       
    }
    else
    {
        TxInfo.bEAPOLFrame = FALSE;
    }

    // [WAPI]
    //

    pWcidMacTabEntry = MlmeSyncMacTabMatchedWlanIdx(pPort, TxInfo.Wcid);

    if(pWcidMacTabEntry == NULL)
    {
        DBGPRINT(RT_DEBUG_TRACE,("%s(), Line:%d, Mac Entry is NULL  Wcid = %d\n", __FUNCTION__, __LINE__,TxInfo.Wcid));
        return NDIS_STATUS_FAILURE;
    }
    
    if (TxInfo.Protocol == 0x0806)
        TxInfo.bDHCPFrame = TRUE;
    
    // Marvell WiFi test bed AP will have MIC error when update new session key in heavy traffic. Others AP maybe drop the packets that using wrong key to decrypt.
    // It may fail chariot's endurace test. Safety, just block it. 
    if(MT_TEST_FLAG(pAd, fRTMP_ADAPTER_ADDKEY_IN_PROGRESS) && (TxInfo.bEAPOLFrame == FALSE) )
    {
        DBGPRINT(RT_DEBUG_ERROR, ("!!!! fRTMP_ADAPTER_ADDKEY_IN_PROGRESS and is not EAPOL Frame !!!!\n"));  
        pAd->pHifCfg->BulkOutDataOneSecCount++;
        return NDIS_STATUS_ADAPTER_NOT_READY;   
    }

    if(TxInfo.bEAPOLFrame)
        TxInfo.TxRate = RATE_1;
    
    // we using lowest basic rate when DHCP and EAPOL packet
    if ((TxInfo.bDHCPFrame == TRUE) || (TxInfo.bEAPOLFrame == TRUE) ||(TxInfo.bWAIFrame == TRUE))
        TxInfo.bHtVhtFrame = FALSE;

    //
    // WPA 802.1x secured port control - drop all non-802.1x frame before port secured
    //
    //
    // TODO: ("RTMPHardEncrypt --> Drop packet before port secured !!!\n")
    //  
    if (((pWcidMacTabEntry->TXAutoBAbitmap&(1<<TxInfo.UserPriority)) != 0) &&
        (TxInfo.bHtVhtFrame == TRUE)&& (INFRA_ON(pPort) || (( *pDA & 0x01) == 0)))      // infrastructure mode is always unicast. pHeader80211->Addr1[0] for Adhoc DA
    {
        if (((pPort->BBPCurrentBW == BW_40) && (READ_PHY_CFG_MCS(pAd, &pWcidMacTabEntry->TxPhyCfg) > MCS_0)) 
            ||((pPort->BBPCurrentBW == BW_20) && (READ_PHY_CFG_MCS(pAd, &pWcidMacTabEntry->TxPhyCfg) > MCS_1))
            || (pPort->BBPCurrentBW == BW_80))
        {
            pWcidMacTabEntry->TXAutoBAbitmap &= ~(1<<TxInfo.UserPriority);
            pPort->BATable.BAOriEntry[pWcidMacTabEntry->BAOriWcidArray[TxInfo.UserPriority]].ORI_BA_Status = Originator_WaitAdd;
            DBGPRINT_RAW(RT_DEBUG_TRACE, ("RTMPHardEncrypt --> Originator_WaitAdd!!! idx =%x\n",pWcidMacTabEntry->BAOriWcidArray[TxInfo.UserPriority]));
        }
    }
    else if(((pWcidMacTabEntry->TXBAbitmap&(1<<TxInfo.UserPriority)) != 0) &&
            (INFRA_ON(pPort)) && (TxInfo.bHtVhtFrame == TRUE))      // infrastructure mode is always unicast.
    {
        TxInfo.bTXBA = TRUE;
    }
    else if(((pWcidMacTabEntry->TXBAbitmap&(1<<TxInfo.UserPriority)) != 0) &&
            ((*pDA & 0x01) == 0)&& (TxInfo.bHtVhtFrame == TRUE))        // only unicast data use Block Ack.
    {
        TxInfo.bTXBA = TRUE;
    }

    if (TxInfo.bTXBA == TRUE)
    {
        if ((pPort->BBPCurrentBW == BW_40) && (READ_PHY_CFG_MCS(pAd, &pWcidMacTabEntry->TxPhyCfg) <= MCS_0))
        {
            TxInfo.bTXBA = FALSE;
            BATableTearORIEntry(pAd,pPort,0, (UCHAR)TxInfo.Wcid, FALSE, TRUE);
            DBGPRINT(RT_DEBUG_TRACE, ("%s - BATableTearORIEntry(Line=%d)\n", __FUNCTION__, __LINE__));
        }

        if ((pPort->BBPCurrentBW == BW_20) && (READ_PHY_CFG_MCS(pAd, &pWcidMacTabEntry->TxPhyCfg) <= MCS_1))
        {
            TxInfo.bTXBA = FALSE;
            BATableTearORIEntry(pAd,pPort, 0, (UCHAR)TxInfo.Wcid, FALSE, TRUE);
            DBGPRINT(RT_DEBUG_TRACE, ("%s - BATableTearORIEntry(Line=%d)\n", __FUNCTION__, __LINE__));
        }

        DBGPRINT_RAW(RT_DEBUG_INFO, ("BATableTearORIEntry :MCS=%d, bTXBA=%d \n", 
            READ_PHY_CFG_MCS(pAd, &pWcidMacTabEntry->TxPhyCfg), TxInfo.bTXBA));
    }
    // decide is the need of piggy-back
    if ((TxInfo.bHtVhtFrame == FALSE) && CLIENT_STATUS_TEST_FLAG(pWcidMacTabEntry, fCLIENT_STATUS_PIGGYBACK_CAPABLE))
    {
        TxInfo.bPiggyBack = TRUE;
    }

    if (pPort->CommonCfg.AckPolicy[TxInfo.QueIdx] != NORMAL_ACK)
    {
        TxInfo.bAckRequired = FALSE;
        TxInfo.PID = 0;
        TxInfo.Cipher = pPort->PortCfg.PairCipher; // Cipher for Unicast
        TxInfo.bMcast = FALSE;
    }
    else if (TxInfo.bTXBA)
    {
        TxInfo.bAckRequired = TRUE;
        TxInfo.PID = 0;
        TxInfo.Cipher = pPort->PortCfg.PairCipher; // Cipher for Unicast
        TxInfo.bMcast = FALSE;
    }
    else if ((pDA[0] & 0x01) && ((ADHOC_ON(pPort)||(IS_P2P_GO_OP(pPort))))) // Multicast or Broadcast
    {
        TxInfo.bAckRequired = FALSE;
        TxInfo.PID = 0;
        INC_COUNTER64(pAd->Counter.WlanCounters.MulticastTransmittedFrameCount);
        TxInfo.Cipher = pPort->PortCfg.GroupCipher; // Cipher for Multicast or Broadcast
        TxInfo.bMcast = TRUE;
    }
    // just sending WDS packet for Aironet PLM use
    else if ((pHeader80211->FC.ToDs == 1) && (pHeader80211->FC.FrDs == 1))
    {
        DBGPRINT(RT_DEBUG_TRACE, ("Sending WDS packet for Aironet PLM use! \n"));
        TxInfo.bAckRequired = FALSE;
        TxInfo.PID = 0;
        INC_COUNTER64(pAd->Counter.WlanCounters.MulticastTransmittedFrameCount);
        TxInfo.Cipher = pPort->PortCfg.GroupCipher; // Cipher for Multicast or Broadcast
        TxInfo.bMcast = TRUE;
    }
    else
    {
        TxInfo.bAckRequired = TRUE;
        TxInfo.PID = 0;
        TxInfo.Cipher = pPort->PortCfg.PairCipher; // Cipher for Unicast
        TxInfo.bMcast = FALSE;
    }

    TxInfo.bAdhocN_ChangeMulticastRate = (ADHOC_ON(pPort) && (TxInfo.bMcast) && ((pAd->StaCfg.AdhocNForceMulticastRate & ALLOW_CHANGE_MCAST_RATE) >> 28)) ? TRUE : FALSE;

    // AdhocNForceMulticatRate: Should not use HTRate in legacy mode and manual mode that using non-HTMIX mode
    if (TxInfo.bAdhocN_ChangeMulticastRate)
    {
        // Manual mode that using non-HTMIX mode
        if ((pAd->StaCfg.AdhocNForceMulticastRate & 0xFFFF0000) == USER_DEFINE_MCAST_RATE)//If bit16 on, bit[0~15] means HTTRANSMIT_SETTING
        {
            PPHY_CFG pPhyCfg = (PPHY_CFG)(&pAd->StaCfg.AdhocNForceMulticastRate);           
            
            if (READ_PHY_CFG_MODE(pAd, pPhyCfg) < MODE_HTMIX)
            {
                TxInfo.bHtVhtFrame = FALSE;
        }
        }
        // Legacy mode
        else if ((pAd->StaCfg.AdhocNForceMulticastRate & 0xFFFF0000) == USE_LEGACY_MCAST_RATE) //If bit21 on, means Force Use Legacy Rate for Multicast
            TxInfo.bHtVhtFrame = FALSE;
    }

#if 0
// 
// Hardware has concern on changing the FramgGap, it should always use IFS_HTTXOP and can't be dynamic changed.
// It apply to 2860C/D/E, 2880C/D (MAC version ID: 0x2860_0100, 0101, 0102, 0103) 
// And 2872, 2880E no this issue
//
    // 1. traditional TX burst
    if (TxInfo.bHtVhtFrame == TRUE)
        TxInfo.FrameGap = IFS_HTTXOP;
    else if (TotalPacketLength > pPort->CommonCfg.RtsThreshold)
        TxInfo.FrameGap = IFS_HTTXOP;
    else if (OPSTATUS_TEST_FLAG(pAd, fOP_STATUS_BG_PROTECTION_INUSED))
        TxInfo.FrameGap = IFS_HTTXOP;
    else if (OPSTATUS_TEST_FLAG(pAd, fOP_STATUS_WMM_INUSED))
    {
       if (!pPort->CommonCfg.APEdcaParm.Txop[TxInfo.QueIdx])
            TxInfo.FrameGap = IFS_HTTXOP;  
        else
            TxInfo.FrameGap = IFS_BACKOFF;
    }
    else if (pPort->CommonCfg.bEnableTxBurst)
        TxInfo.FrameGap = IFS_HTTXOP;
    // 3. otherwise, always BACKOFF before transmission
    else
        TxInfo.FrameGap = IFS_BACKOFF;      // Default frame gap mode
#endif

    // ------------------------------------------------------------------
    // STEP 1. WAKE UP PHY
    //      outgoing frame always wakeup PHY to prevent frame lost and 
    //      turn off PSM bit to improve performance
    // ------------------------------------------------------------------
    // not to change PSM bit, just send this frame out?
    //if (OPSTATUS_TEST_FLAG(pAd, fOP_STATUS_DOZE))
    //{
    //  DBGPRINT(RT_DEBUG_TRACE,("########### Tx PM4 Wake up ###########\n"));
    //  OPSTATUS_CLEAR_FLAG(pAd, fOP_STATUS_DOZE);
    //}

    // It should not change PSM bit, when APSD turn on.
    if ((!(pPort->CommonCfg.bAPSDCapable && pPort->CommonCfg.APEdcaParm.bAPSDCapable) && (pPort->CommonCfg.bAPSDForcePowerSave == FALSE)) || (TxInfo.bEAPOLFrame||TxInfo.bWAIFrame))
    {
        if (pAd->StaCfg.Psm == PWR_SAVE)
        {
            MlmeSetPsm(pAd, PWR_ACTIVE);
        }
    }

    // -----------------------------------------------------------------
    // STEP 2. MAKE A COMMON 802.11 HEADER SHARED BY ENTIRE FRAGMENT BURST.
    // -----------------------------------------------------------------
    PlatformMoveMemory(&Header_802_11, pHeader80211, sizeof(HEADER_802_11));

    // Update PM bit for UAPSD
    if (pPort->CommonCfg.bAPSDForcePowerSave)
    {
        Header_802_11.FC.PwrMgmt = PWR_SAVE;
    }
    if (TxInfo.bDLSFrame || TxInfo.bTDLSFrame)
    {
        UCHAR   temp[MAC_ADDR_LEN];
        
        Header_802_11.FC.ToDs = 0;
        Header_802_11.FC.FrDs = 0;

        COPY_MAC_ADDR(temp, Header_802_11.Addr1);
        COPY_MAC_ADDR(Header_802_11.Addr1, Header_802_11.Addr3);
        COPY_MAC_ADDR(Header_802_11.Addr3, temp);

        // Sequence filled to TxInfo.Wcid here for direct link frame,
        // because we might get incorrect WirelessCliID(TxInfo.Wcid) from pTxWI.
        if (TxInfo.bDLSFrame || TxInfo.bTDLSFrame)
        {
            Header_802_11.Sequence = (pWcidMacTabEntry->BAOriSequence[TxInfo.UserPriority]++);
            pWcidMacTabEntry->BAOriSequence[TxInfo.UserPriority] %= 4096;
        }
    }
#if 0
    if (ADHOC_ON(pAd) && RX_FILTER_TEST_FLAG(pAd, fRX_FILTER_ACCEPT_PROMISCUOUS))
    {
        COPY_MAC_ADDR(Header_802_11.Addr2, pSrcBufVA + 6);
        if(pPort->CommonCfg.PSPXlink)
            TxInfo.bAckRequired = FALSE;
    }
#endif

    DBGPRINT(RT_DEBUG_INFO,("%s - PortNum(%d), WMM(%d), bWmmCapable(%d), bValid(%d)\n",
                                __FUNCTION__, 
                                pPort->PortNumber, 
                                OPSTATUS_TEST_FLAG(pPort, fOP_STATUS_WMM_INUSED),
                                pPort->CommonCfg.bWmmCapable,
                                pPort->CommonCfg.APEdcaParm.bValid)); 

        // ===============================================
        // Ui Tool Feature: Fixed Tx rate in MACVer2 later
        // ===============================================
        // Disable HT/VHT fame if the setting of "Fixed Tx rate" is 11b or 11g
        if(pAd->UiCfg.UtfFixedTxRateEn && pAd->UiCfg.UtfFixedTxRateCfg.Ext.Mode <= MODE_OFDM)
            TxInfo.bHtVhtFrame = FALSE;

    if (((OPSTATUS_TEST_FLAG(pPort, fOP_STATUS_WMM_INUSED)) && (pPort->CommonCfg.bWmmCapable == TRUE) && (pPort->CommonCfg.APEdcaParm.bValid == TRUE)) ||
        (TxInfo.bTDLSFrame && pEntry && CLIENT_STATUS_TEST_FLAG(pEntry, fCLIENT_STATUS_WMM_CAPABLE)))
    {
        Header_802_11.FC.SubType = SUBTYPE_QDATA;
        TxInfo.bWMM = TRUE;
        if (CLIENT_STATUS_TEST_FLAG(pWcidMacTabEntry, fCLIENT_STATUS_RDG_CAPABLE)
            && (TxInfo.bHtVhtFrame == TRUE) && (TxInfo.bDHCPFrame == FALSE))
        {
            Header_802_11.FC.Order = 1; // HTC bit share with order bit
            TxInfo.bHTC = TRUE;
        }
        else if ((CLIENT_STATUS_TEST_FLAG(pWcidMacTabEntry, fCLIENT_STATUS_RDG_CAPABLE)) 
            && (CLIENT_STATUS_TEST_FLAG(pWcidMacTabEntry, fCLIENT_STATUS_RALINK_CHIPSET)) 
            && ((TxInfo.bDHCPFrame == TRUE) || (TxInfo.bEAPOLFrame == TRUE)))
        {
            // To patch rt2860E and before HW bug,
            // Use non-QoS format to send (TxInfo.bDHCPFrame == TRUE) frames if other 11n data frames are RDG. 
            Header_802_11.FC.SubType = SUBTYPE_DATA;
            TxInfo.bWMM = FALSE;
            DBGPRINT(RT_DEBUG_TRACE, ("TxInfo.bWMM = FALSE\n"));
        }
        else if ((CLIENT_STATUS_TEST_FLAG(pWcidMacTabEntry, fCLIENT_STATUS_RALINK_CHIPSET))
            &&((TxInfo.bDHCPFrame == TRUE) || (TxInfo.bEAPOLFrame == TRUE))
            && (READ_PHY_CFG_MODE(pAd, &pWcidMacTabEntry->TxPhyCfg) >= MODE_HTMIX))
        {
            // Low rate packet will use nonQoS when connecting to N-AP(Ralink)
            Header_802_11.FC.SubType = SUBTYPE_DATA;
            TxInfo.bWMM = FALSE;
            DBGPRINT(RT_DEBUG_TRACE,("Low rate packet will use nonQoS.\n"));
        }
        else if( TxInfo.bWAIFrame == TRUE)
        {
            // Low rate packet will use nonQoS when connecting to N-AP(Ralink)
            Header_802_11.FC.SubType = SUBTYPE_DATA;
            TxInfo.bWMM = FALSE;
            DBGPRINT(RT_DEBUG_TRACE,("bWAIFrame packet will use nonQoS.\n"));       
        }
    }

    // --------------------------------------------------------
    // STEP 3. FIND ENCRYPT KEY AND DECIDE CIPHER ALGORITHM
    //      Find the WPA key, either Group or Pairwise Key
    //      LEAP + TKIP also use WPA key.
    // --------------------------------------------------------
    // Decide WEP bit and cipher suite to be used. Same cipher suite should be used for whole fragment burst
    // In Cisco CCX 2.0 Leap Authentication
    //         WepStatus is Ndis802_11Encryption1Enabled but the key will use PairwiseKey
    //         Instead of the SharedKey, SharedKey Length may be Zero.
    TxInfo.KeyIdx = 0xff;
    if (TxInfo.bEAPOLFrame)
    {
        if ((pPort->SharedKey[BSS0][0].CipherAlg) &&
            (pPort->SharedKey[BSS0][0].KeyLen) && 
            (pAd->StaCfg.PortSecured == WPA_802_1X_PORT_SECURED))
        {
            TxInfo.CipherAlg = pPort->SharedKey[BSS0][0].CipherAlg;
            TxInfo.KeyIdx = 0;
        }
    }
    else if ((TxInfo.Cipher == DOT11_CIPHER_ALGO_WEP40) || (TxInfo.Cipher == DOT11_CIPHER_ALGO_WEP104) || (TxInfo.Cipher == DOT11_CIPHER_ALGO_WEP))
    {
        TxInfo.KeyIdx = pPort->PortCfg.DefaultKeyId;
    }
    else if ((TxInfo.Cipher == DOT11_CIPHER_ALGO_TKIP) || (TxInfo.Cipher == DOT11_CIPHER_ALGO_CCMP))
    {
        if (Header_802_11.Addr1[0] & 0x01) // multicast
            TxInfo.KeyIdx = pPort->PortCfg.DefaultKeyId;
        else if (pPort->SharedKey[BSS0][0].KeyLen)
            TxInfo.KeyIdx = 0;
        else
            TxInfo.KeyIdx = pPort->PortCfg.DefaultKeyId;
    }
    
    if (pXcu->pDot11SendContext)
    {
        TxInfo.ExemptionActionType = pXcu->pDot11SendContext->usExemptionActionType;
    }
    
    if ((pPort->PortCfg.WepStatus !=  DOT11_CIPHER_ALGO_NONE)
        && (Header_802_11.FC.Type == DOT11_FRAME_TYPE_DATA) && 
        (pPort->CommonCfg.bSafeModeEnabled == FALSE))
    {
        switch (TxInfo.ExemptionActionType) 
        {
            case DOT11_EXEMPT_NO_EXEMPTION:

                //
                // We want to encrypt this frame.
                //
                //
// Both session timeout (re-EAP) and re-key procedures use encrypted frames 
// in all autunomous/unified AP, MFP/non-MFP, and CCKM/non-CCKM cases.
// Remove CCKM_ON(pAd) conditions in RTMPHardTransmit() to send ciphertext frames in CCKM cases.
//
#if 0

                if (CCKM_ON(pPort))
                {
                    if (TxInfo.bEAPOLFrame)
                        Header_802_11.FC.Wep = (TxInfo.bGroupMsg2)?1:0;
                    else
                        Header_802_11.FC.Wep = 1;
                }
                else
#endif
                    Header_802_11.FC.Wep = 1;
                break;

            case DOT11_EXEMPT_ALWAYS:

                //
                // We don't encrypt this frame.
                //
                Header_802_11.FC.Wep = 0;
                break;

            case DOT11_EXEMPT_ON_KEY_MAPPING_KEY_UNAVAILABLE:

                //
                // We encrypt this frame if and only if a key mapping key is set.
                //
                if (TRUE == MlmeInfoFindKeyMappingKey(pAd,pPort, Header_802_11.Addr1))
                {
                //
// Both session timeout (re-EAP) and re-key procedures use encrypted frames 
// in all autunomous/unified AP, MFP/non-MFP, and CCKM/non-CCKM cases.
// Remove CCKM_ON(pAd) conditions in RTMPHardTransmit() to send ciphertext frames in CCKM cases.
//
#if 0

                    if (CCKM_ON(pPort))
                    {
                        if (TxInfo.bEAPOLFrame)
                            Header_802_11.FC.Wep = (TxInfo.bGroupMsg2)?1:0;
                        else
                            Header_802_11.FC.Wep = 1;
                    }
                    else
#endif
                    {
                        // ----- MFP may not set key to ASIC, KeylLen will be 0
                        if ( pPort->SharedKey[BSS0][TxInfo.KeyIdx].KeyLen != 0)
                            Header_802_11.FC.Wep = 1;
                        else
                            Header_802_11.FC.Wep = 0;
                    }
                    //Header_802_11.FC.Wep = 1;

                    if ((ADHOC_ON(pPort)) && (pPort->PortCfg.AuthMode == Ralink802_11AuthModeWPA2PSK))
                    {
                        TxInfo.KeyIdx = 0;
                    }
                }
                else
                {
                    Header_802_11.FC.Wep = 0;
                }
                break;
            default:
                ASSERT(0);
                Header_802_11.FC.Wep = 1;
                break;
        }

        DBGPRINT(RT_DEBUG_INFO, ("<-- %s, Exempt=%d\n", __FUNCTION__, TxInfo.ExemptionActionType)); 

        if ((TxInfo.KeyIdx != 0xff /* for CCKM-EAPOL-Frame exception*/) &&
            (TxInfo.PortSecured == WPA_802_1X_PORT_SECURED) && 
            ((pPort->PortCfg.WepStatus == Ralink802_11Encryption2Enabled) ||
             (pPort->PortCfg.WepStatus == Ralink802_11Encryption3Enabled)))
        {
            Header_802_11.FC.Wep = 1;
        }
        else 
        {
            //
            // If the frame is to be encrypted, but no key is not set, either reject the frame 
            // or clear the WEP bit.
            //
            if (Header_802_11.FC.Wep && (pPort->SharedKey[BSS0][TxInfo.KeyIdx].KeyLen == 0)) 
            {
                //
                // If this is a unicast frame or if the BSSPrivacy is on, reject the frame. Otherwise,
                // clear the WEP bit so we will not encrypt the frame.
                //
                if (ETH_IS_UNICAST(Header_802_11.Addr1)) //(CAP_IS_PRIVACY_ON(pAd->StaActive.CapabilityInfo)) 
                {
                    DBGPRINT(RT_DEBUG_TRACE,("no key install RTMPHardTransmit NDIS_STATUS_FAILURE "));
                    //
                    // Return NDIS_STATUS_NOT_ACCEPTED, so we can keep continuing to dequeue the NetBuffer
                    //
                        return (NDIS_STATUS_NOT_ACCEPTED);
                }
                else 
                {
                    Header_802_11.FC.Wep = 0;
                }
            }
        }
    }

    if (TxInfo.KeyIdx == 0xff)
        TxInfo.CipherAlg = CIPHER_NONE;
    else if (pPort->SharedKey[BSS0][TxInfo.KeyIdx].KeyLen == 0)
        TxInfo.CipherAlg = CIPHER_NONE;
    else
    {
        //Header_802_11.FC.Wep = 1;
        TxInfo.CipherAlg = pPort->SharedKey[BSS0][TxInfo.KeyIdx].CipherAlg;
        pKey = &pPort->SharedKey[BSS0][TxInfo.KeyIdx];
        DBGPRINT(RT_DEBUG_INFO,("RTMPHardTransmit(bEAP=%d) - %s key#%d, KeyLen=%d, TxInfo.CipherWcid = %x\n",
                    TxInfo.bEAPOLFrame, DecodeCipherName(TxInfo.CipherAlg), TxInfo.KeyIdx, pPort->SharedKey[BSS0][TxInfo.KeyIdx].KeyLen,TxInfo.Wcid));
    }

    // STEP 3.1 if TKIP is used and fragmentation is required. Driver has to
    //          append TKIP MIC at tail of the scatter buffer (This must be the
    //          ONLY scatter buffer in the NDIS PACKET). 
    //          MAC ASIC will only perform IV/EIV/ICV insertion but no TKIP MIC
    if ((TxInfo.MpduRequired > 1) && (TxInfo.CipherAlg == CIPHER_TKIP))
    {
        TotalPacketLength += 8;
        TxInfo.CipherAlg = CIPHER_TKIP_NO_MIC;  
    }
    
    // ----------------------------------------------------------------
    // STEP 4. Make RTS frame or CTS-to-self frame if required
    // ----------------------------------------------------------------

    //
    // calcuate the overhead bytes that encryption algorithm may add. This
    // affects the calculate of "duration" field
    //
    if ((TxInfo.CipherAlg == CIPHER_WEP64) || (TxInfo.CipherAlg == CIPHER_WEP128)) 
        TxInfo.EncryptionOverhead = 8; //WEP: IV[4] + ICV[4];
    else if (TxInfo.CipherAlg == CIPHER_TKIP_NO_MIC)
        TxInfo.EncryptionOverhead = 12;//TKIP: IV[4] + EIV[4] + ICV[4], MIC will be added to TotalPacketLength
    else if (TxInfo.CipherAlg == CIPHER_TKIP)
        TxInfo.EncryptionOverhead = 20;//TKIP: IV[4] + EIV[4] + ICV[4] + MIC[8]
    else if (TxInfo.CipherAlg == CIPHER_AES)
        TxInfo.EncryptionOverhead = 16;    // AES: IV[4] + EIV[4] + MIC[8]
    else
        TxInfo.EncryptionOverhead = 0;

    // decide how much time an ACK/CTS frame will consume in the air
    TxInfo.AckDuration = XmitCalcDuration(pAd, pPort->CommonCfg.ExpectedACKRate[TxInfo.TxRate], 14);

    // If fragment required, MPDU size is maximum fragment size
    // Else, MPDU size should be frame with 802.11 header & CRC
    if (TxInfo.MpduRequired > 1)
        NextMpduSize = pPort->CommonCfg.FragmentThreshold;
    else
    {
        NextMpduSize = TotalPacketLength;
    }

    // --------------------------------------------------------
    // STEP 5. START MAKING MPDU(s)
    //      Start Copy Ndis Packet into Ring buffer.
    //      For frame required more than one ring buffer (fragment), all ring buffers
    //      have to be filled before kicking start tx bit.
    //      Make sure TX ring resource won't be used by other threads
    // --------------------------------------------------------
    if (TxInfo.bTXBA)
    {
        TxInfo.BAWinSize = pPort->BATable.BAOriEntry[TxInfo.RABAOriIdx].BAWinSize - 1;
        Header_802_11.Frag = 0;
    }
    SrcRemainingBytes = TotalPacketLength - LENGTH_802_11;
    SrcBufLen        -= LENGTH_802_11;
    Remain = SrcRemainingBytes;

    //
    // Fragmentation is not allowed on multicast & broadcast
    // So, we need to used the MAX_FRAG_THRESHOLD instead of pPort->CommonCfg.FragmentThreshold
    // otherwise if PacketInfo.TotalPacketLength > pPort->CommonCfg.FragmentThreshold then
    // packet will be fragment on multicast & broadcast.
    //
    // TxInfo.MpduRequired equals to 1 means this could be Aggretaion case.
    //

    if ((*pDA & 0x01) || (READ_PHY_CFG_MODE(pAd, &pPort->CommonCfg.TxPhyCfg)>= MODE_HTMIX) || (TxInfo.MpduRequired == 1))
    {
        InitFreeMpduSize = MAX_FRAG_THRESHOLD;
    }
    else
    {
        InitFreeMpduSize = pPort->CommonCfg.FragmentThreshold - sizeof(HEADER_802_11) - LENGTH_CRC;
    }

    // When TKIP + fragmentation in use, previous calculation may be wrong, since
    // 8-bytes software MIC was not taken into consideration.
    if (TxInfo.CipherAlg == CIPHER_TKIP_NO_MIC)
    {
        ULONG AllowFragSize = InitFreeMpduSize;
        if (OPSTATUS_TEST_FLAG(pPort, fOP_STATUS_WMM_INUSED))
        {
            AllowFragSize -= 2;
        }
        if (Remain > (AllowFragSize * pXcu->FragmentRequired))
        {
            pXcu->FragmentRequired++;
        }
    }

    StartOfFrame = TRUE;
    MICFrag = FALSE;    // Flag to indicate MIC shall spread into two MPDUs
    // Start Copy Ndis Packet into Ring buffer.
    // For frame required more than one ring buffer (fragment), all ring buffers
    // have to be filled before kicking start tx bit.
    do
    {
        //
        // STEP 5.1 Get the Tx Ring descriptor & Dma Buffer address
        //
        NdisAcquireSpinLock(&pAd->pTxCfg->TxContextQueueLock[TxInfo.QueIdx]);
        pTxContext  = &pAd->pHifCfg->TxContext[TxInfo.QueIdx];
        FillOffset = pTxContext->CurWritePosition;
        // Check ring full.
        if (((pTxContext->CurWritePosition) < pTxContext->NextBulkOutPosition) && (pTxContext->CurWritePosition + LOCAL_TXBUF_SIZE + ONE_SEGMENT_UNIT) > pTxContext->NextBulkOutPosition)
        {
            DBGPRINT(RT_DEBUG_TRACE,("XmitHardTransmit c1 --> CurWritePosition = %d, NextBulkOutPosition = %d. \n", pTxContext->CurWritePosition, pTxContext->NextBulkOutPosition));
            MTUSB_SET_BULK_FLAG(pAd, (fRTUSB_BULK_OUT_DATA_NORMAL << TxInfo.QueIdx));
            NdisReleaseSpinLock(&pAd->pTxCfg->TxContextQueueLock[TxInfo.QueIdx]);
            return (NDIS_STATUS_RESOURCES);
        }
        else if ((pTxContext->CurWritePosition > (ONE_SEGMENT_UNIT * (SEGMENT_TOTAL - 1))) && (pTxContext->NextBulkOutPosition < (ONE_SEGMENT_UNIT)))
        {
            DBGPRINT(RT_DEBUG_TRACE,("XmitHardTransmit c4 --> CurWritePosition = %d, NextBulkOutPosition = %d. \n", pTxContext->CurWritePosition, pTxContext->NextBulkOutPosition));
            MTUSB_SET_BULK_FLAG(pAd, (fRTUSB_BULK_OUT_DATA_NORMAL << TxInfo.QueIdx));
            NdisReleaseSpinLock(&pAd->pTxCfg->TxContextQueueLock[TxInfo.QueIdx]);
            return (NDIS_STATUS_RESOURCES);
        }
        else if ((pTxContext->CurWritePosition == 8) && (pTxContext->NextBulkOutPosition < (LOCAL_TXBUF_SIZE + ONE_SEGMENT_UNIT)))
        {
            DBGPRINT(RT_DEBUG_TRACE,("XmitHardTransmit c2 --> CurWritePosition = %d, NextBulkOutPosition = %d. \n", pTxContext->CurWritePosition, pTxContext->NextBulkOutPosition));
            MTUSB_SET_BULK_FLAG(pAd, (fRTUSB_BULK_OUT_DATA_NORMAL << TxInfo.QueIdx));
            NdisReleaseSpinLock(&pAd->pTxCfg->TxContextQueueLock[TxInfo.QueIdx]);
            return (NDIS_STATUS_RESOURCES);
        }
        else if (pTxContext->bCurWriting == TRUE)
        {
            DBGPRINT(RT_DEBUG_TRACE,("XmitHardTransmit c3 --> TxInfo.QueIdx = %d, CurWritePosition = %d, NextBulkOutPosition = %d. \n", 
                    TxInfo.QueIdx, pTxContext->CurWritePosition, pTxContext->NextBulkOutPosition));
            NdisReleaseSpinLock(&pAd->pTxCfg->TxContextQueueLock[TxInfo.QueIdx]);
            return (NDIS_STATUS_RESOURCES);
        }
        
        if ((pTxContext->ENextBulkOutPosition == pTxContext->CurWritePosition))
        {
            bIncreaseCurWriPos = TRUE;
            pTxContext->ENextBulkOutPosition += 8;
            FillOffset += 8;
        }

        pTxContext->bCurWriting = TRUE;

        //
        // allocate TxInfo
        //
/*      
        pTxInfo= (PTXINFO_STRUC)&pTxContext->TransferBuffer->WirelessPacket[FillOffset];
        PlatformZeroMemory(pTxInfo, TXINFO_SIZE);

        //
        // allocate TxWI
        //      
        pTxWI= (PTXWI_STRUC)&pTxContext->TransferBuffer->WirelessPacket[FillOffset+TXINFO_SIZE];
        PlatformZeroMemory(pTxWI, pAd->HwCfg.TXWI_Length);
*/

        // allocate Tx Descriptor

        if ((pPort->CommonCfg.bAutoTxRateSwitch == FALSE) || (TxInfo.bDHCPFrame == TRUE) /* || (NumberRequired > 1)*/)
        {
            pLongFormatTxD = (PTXDSCR_LONG_STRUC)&pTxContext->TransferBuffer->WirelessPacket[FillOffset];
            PlatformZeroMemory(pLongFormatTxD, TXD_LONG_SIZE);
            pTxDDW0 = &pLongFormatTxD->TxDscrDW0;
            pTxDDW7 = &pLongFormatTxD->TxDscrDW7;
            PlatformZeroMemory(pTxDDW7, 4);
            pWirelessPacket = &pTxContext->TransferBuffer->WirelessPacket[FillOffset + TXD_LONG_SIZE];
        }
        else
        {
            pShortFormatTxD = (PTXDSCR_SHORT_STRUC)&pTxContext->TransferBuffer->WirelessPacket[FillOffset];
            PlatformZeroMemory(pShortFormatTxD, TXD_LONG_SIZE);
            pTxDDW0 = &pShortFormatTxD->TxDscrDW0;
            pTxDDW7 = &pShortFormatTxD->TxDscrDW7;
            PlatformZeroMemory(pTxDDW7, 4);
            pWirelessPacket = &pTxContext->TransferBuffer->WirelessPacket[FillOffset + TXD_SHORT_SIZE];
        }
            

        NdisReleaseSpinLock(&pAd->pTxCfg->TxContextQueueLock[TxInfo.QueIdx]);

        //
        // STEP 5.2 COPY 802.11 HEADER INTO 1ST DMA BUFFER
        //
        pDest = pWirelessPacket;        
        PlatformMoveMemory(pDest, &Header_802_11, sizeof(HEADER_802_11));
        pDest80211Hdr = (PHEADER_802_11)pDest;
        pDest       += sizeof(HEADER_802_11);
        DataOffSet  += sizeof(HEADER_802_11);

        // Init FreeMpduSize
        FreeMpduSize = InitFreeMpduSize;
        //Cisco AP 1200 ,only accept even number of octect fragment packet
        if((FreeMpduSize % 2) != 0 )
        {
            FreeMpduSize++;
        }
        
        TxInfo.This80211HdrLen        = LENGTH_802_11;
        
        if (TxInfo.bWMM == TRUE)
        {
            // copy QOS CONTROL bytes
            // HT rate must has QOS CONTROL bytes
            if (TxInfo.bTXBA)
                *pDest        =  (TxInfo.UserPriority & 0x0f) ;
            else
                *pDest        =  (TxInfo.UserPriority & 0x0f) | pPort->CommonCfg.AckPolicy[TxInfo.QueIdx];
            *(pDest+1)    =  0;
            pDest         += 2;
            TxInfo.This80211HdrLen        = LENGTH_802_11 + 2;
            FreeMpduSize  -= 2;
            TxInfo.LengthQosPAD = 2;
            // Pad 2 bytes to make 802.11 header 4-byte alignment

#if 0  // auto filled by HW         
            if (TxInfo.bHTC == TRUE)
            {
                // HTC Control field is following QOS field.
                PlatformZeroMemory(pDest, 6);
                TxInfo.This80211HdrLen        += 4;
                if (CLIENT_STATUS_TEST_FLAG(&pPort->MacTab.Content[TxInfo.Wcid], fCLIENT_STATUS_RDG_CAPABLE) )
                    *(pDest+3)|=0x80;
                pDest         += 6;
            }
            else
#endif          
            {
                PlatformZeroMemory(pDest, 2);
                pDest         += 2;
            }
        }

#if 0
        //
        // STEP 5.4 COPY LLC/SNAP, CKIP MIC INTO 1ST DMA BUFFER ONLY WHEN THIS 
        //          MPDU IS THE 1ST OR ONLY FRAGMENT 
        //
        // If send AMSDU, we follow 802.11n D2.0  AMSDU foramt here.
        // Dlink(Atheros), Broadcom can't receive AMSDU+AMPDU together. So we don't use them at the same time.
        if ((TxInfo.bWMM == TRUE)
            && (CLIENT_STATUS_TEST_FLAG(&pPort->MacTab.Content[BSSID_WCID], fCLIENT_STATUS_AMSDU_INUSED)) 
            && (TxInfo.bHtVhtFrame == TRUE) && (TxInfo.bTXBA == FALSE))
        {
            TxInfo.bAMSDU = TRUE;
        }
#endif

        if ((MFP_ON(pAd, pPort)) &&
            (TxInfo.KeyIdx != 0xff /* for CCKM-EAPOL-Frame exception*/) &&
            (TxInfo.PortSecured == WPA_802_1X_PORT_SECURED) && 
            ((pPort->PortCfg.WepStatus == Ralink802_11Encryption2Enabled) ||
             (pPort->PortCfg.WepStatus == Ralink802_11Encryption3Enabled)) &&
             (TxInfo.bMcast == FALSE))
        {
            FreeMpduSize -= TxInfo.EncryptionOverhead;
        }

        if (Header_802_11.Frag == 0)
        {
            if ((TxInfo.CipherAlg == CIPHER_TKIP_NO_MIC) && (pKey != NULL))
            {           
                DBGPRINT_RAW(RT_DEBUG_TRACE, ("RTMPCalculateMICValue...\n"));  

                // Use the key in pairwise key table if the packet is direct to Peer Entry.
                if ((MlmeSyncGetRoleTypeByWlanIdx(pPort, TxInfo.Wcid) == ROLE_WLANIDX_CLIENT) && ETH_IS_UNICAST(Header_802_11.Addr1) && (pWcidMacTabEntry->ValidAsCLI == TRUE))
                    pKey = &pWcidMacTabEntry->PairwiseKey;
                
                // 
                // RTMPHardTransmit may reentry and cause MIC error
                // Add Spinlock to protect MIC variables.
                //              
                NdisAcquireSpinLock(&pAd->pNicCfg->PrivateInfo.CalMicLock);              
                RTMPCalculateMICValue(pAd, pDA, Header_802_11.Addr2, pXcu, pKey, pXcu->UserPriority);
                PlatformMoveMemory(TxInfo.TxMICValue, pAd->pNicCfg->PrivateInfo.Tx.MIC, 8);
                NdisReleaseSpinLock(&pAd->pNicCfg->PrivateInfo.CalMicLock);
            }
        }

        // Start copying payload
        BytesCopied = 0;
        
        BufferIdx =1;
        SysVa = pXcu->BufferList[BufferIdx].VirtualAddr;
        SrcBufLen = pXcu->BufferList[BufferIdx].Length;
        
        do 
        {
            if (SrcBufLen >= FreeMpduSize)  
            {
                // Avoid the blank bytes added after Data. Thus, we only copy the remain byte.
                if(SrcBufLen > Remain)
                {               
                    SrcBufLen = Remain;
                    PlatformMoveMemory(pDest, SysVa, SrcBufLen);
                    BytesCopied += SrcBufLen;
                    SysVa       += SrcBufLen;
                    pDest       += SrcBufLen;
                    SrcBufLen   -= SrcBufLen;
                    Remain      -= SrcBufLen;

                    TxInfo.bIsLastFrag = TRUE;
                    //
                    // No more data, check if we need to add SW MIC.
                    //
                    if ((TxInfo.CipherAlg == CIPHER_TKIP_NO_MIC) &&
                        (MICFrag == FALSE) &&
                        (pKey != NULL))
                    {
                        SrcBufLen = 8;      // Set length to MIC length
                                    
                        if (FreeMpduSize >= SrcBufLen)
                        {
                            PlatformMoveMemory(pDest, TxInfo.TxMICValue, SrcBufLen);
                            BytesCopied  += SrcBufLen;
                            pDest        += SrcBufLen;
                            FreeMpduSize -= SrcBufLen;
                            SrcBufLen = 0;
                            DBGPRINT(RT_DEBUG_ERROR, ("XmitHardTransmit, Copy MICFrag\n"));
                        }
                        else
                        {
                            PlatformMoveMemory(pDest, TxInfo.TxMICValue, FreeMpduSize);
                            BytesCopied += FreeMpduSize;
                            SysVa       = TxInfo.TxMICValue + FreeMpduSize;
                            pDest       += FreeMpduSize;
                            SrcBufLen   -= FreeMpduSize;
                            MICFrag     = TRUE;
                            DBGPRINT(RT_DEBUG_ERROR, ("XmitHardTransmit, Copy data  MICFrag FRAG\n"));
                        }                       
                    }
                }
                else
                {
                    // Copy only the free fragment size, and save the pointer
                    // of current buffer descriptor for next fragment buffer.
                    PlatformMoveMemory(pDest, SysVa, FreeMpduSize);
                    BytesCopied += FreeMpduSize;
                    SysVa       += FreeMpduSize;
                    pDest       += FreeMpduSize;
                    SrcBufLen   -= FreeMpduSize;
                    Remain      -= FreeMpduSize;
                }
                
                TxInfo.bIsFrag  = TRUE;     
                break;
            }
            else if (SrcBufLen > 0)
            {
                if (!SysVa)
                {
                    DBGPRINT(RT_DEBUG_ERROR, ("XmitHardTransmit, Copy data failed\n"));
                    pTxContext->bCurWriting = FALSE;
                    return NDIS_STATUS_RESOURCES;
                }

                // Avoid the blank bytes added after Data. Thus, we only copy the remain byte.
                if(SrcBufLen > Remain)
                {
                    SrcBufLen = Remain;
                }
                //
                // Copy the rest of this buffer descriptor pointed data into ring buffer.
                //
                PlatformMoveMemory(pDest, SysVa, SrcBufLen);
                BytesCopied  += SrcBufLen;
                pDest        += SrcBufLen;
                FreeMpduSize -= SrcBufLen;
                Remain       -= SrcBufLen;
            }

            if ((BufferIdx<(pXcu->NumOfBuf-1)) && (Remain > 0))
            {
                BufferIdx++;
                SysVa = pXcu->BufferList[BufferIdx].VirtualAddr;
                SrcBufLen = pXcu->BufferList[BufferIdx].Length;
                if (SysVa == NULL) 
                {
                    DBGPRINT(RT_DEBUG_ERROR, ("XmitHardTransmit, Copy data failed\n"));
                    pTxContext->bCurWriting = FALSE;
                    return NDIS_STATUS_RESOURCES;
                }
            }
            else
            {
                TxInfo.bIsLastFrag = TRUE;
                //
                // No more data, check if we need to add SW MIC.
                //
                if ((TxInfo.CipherAlg == CIPHER_TKIP_NO_MIC) &&
                    (MICFrag == FALSE) &&
                    (pKey != NULL))
                {
                    SrcBufLen = 8;      // Set length to MIC length
                                
                    if (FreeMpduSize >= SrcBufLen)
                    {
                        PlatformMoveMemory(pDest, TxInfo.TxMICValue, SrcBufLen);
                        BytesCopied  += SrcBufLen;
                        pDest        += SrcBufLen;
                        FreeMpduSize -= SrcBufLen;
                        SrcBufLen = 0;
                    }
                    else
                    {
                        PlatformMoveMemory(pDest, TxInfo.TxMICValue, FreeMpduSize);
                        BytesCopied += FreeMpduSize;
                        SysVa       = TxInfo.TxMICValue + FreeMpduSize;
                        pDest       += FreeMpduSize;
                        SrcBufLen   -= FreeMpduSize;
                        MICFrag     = TRUE;
                    }                       
                }
                break;
            }
        } while (TRUE);

        //
        // Software encryption instead in the case of TKIP/AES for CCX_MFP_ON.
        // Because AAD and NONCE construction in AES and Priority field in TKIP are different between data frame and mgmt frame.
        if ((MFP_ON(pAd, pPort)) &&
            (TxInfo.KeyIdx != 0xff /* for CCKM-EAPOL-Frame exception*/) &&
            (TxInfo.PortSecured == WPA_802_1X_PORT_SECURED) && 
            ((pPort->PortCfg.WepStatus == Ralink802_11Encryption2Enabled) ||
             (pPort->PortCfg.WepStatus == Ralink802_11Encryption3Enabled)) &&
             (TxInfo.bMcast == FALSE) && (TxInfo.bIsFrag  == FALSE/*fragment, not ready */))
        {
            pEncrypt = pWirelessPacket + TxInfo.This80211HdrLen + TxInfo.LengthQosPAD; // pointer to LLC start byte

            if (pExtraLlcSnapEncap == NULL)
            //if (TxInfo.FrameGap != IFS_HTTXOP)
            {
                // middle or last frag packet
                TxSize =  BytesCopied + TxInfo.This80211HdrLen + TxInfo.EncryptionOverhead;             
            }
            else
            {
                TxSize = BytesCopied + TxInfo.This80211HdrLen + TxInfo.EncryptionOverhead + LENGTH_802_1_H;
            }

            //PlatformMoveMemory(pDest80211Hdr, &Header_802_11, LENGTH_802_11);
            // add length of {IV/EIV, MIC, ICV...}
            pDest += TxInfo.EncryptionOverhead;

//**
            if (pPort->PortCfg.WepStatus == Ralink802_11Encryption3Enabled)
            {
                // AES input = {MacHeader | Plaintext | zeros in 16 bytes }
                // AES output= {MacHeader | CCMP Header | Ciphertext | MIC} 
                if (!RTMPSoftEncryptAES(
                                pAd, 
                                pPort,
                                (PUCHAR)pDest80211Hdr,
                                pEncrypt,
                                TxSize - TxInfo.This80211HdrLen, 
                                TxInfo.KeyIdx, 
                                pPort->SharedKey[BSS0], 
                                FALSE))
                {
                    DBGPRINT(RT_DEBUG_ERROR, ("CCX-MFP: Data, RTMPSoftEncryptAES Failed\n"));           
                    //return NDIS_STATUS_FAILURE;   // No return, just report this error status.
                }
            }
            else
            {   
                // TKIP input = {MacHeader | Plainttext | zeros in 20 bytes }
                // TKIP output= {MacHeader | IV Header | Ciphertext | ICV | MIC}
                if (!RTMPSoftEncryptTKIP(                               
                                pAd, 
                                pPort,
                                (PUCHAR)pDest80211Hdr,
                                pEncrypt,
                                TxSize - TxInfo.This80211HdrLen, 
                                TxInfo.UserPriority,
                                TxInfo.KeyIdx, 
                                pPort->SharedKey[BSS0], 
                                FALSE))
                {
                    DBGPRINT(RT_DEBUG_ERROR, ("CCX-MFP: Data, RTMPSoftEncryptTKIP Failed\n"));          
//                  return NDIS_STATUS_FAILURE;
                }           
            }
        }       
        else
            TxSize = BytesCopied + TxInfo.This80211HdrLen;
        
        SrcRemainingBytes -=  BytesCopied;

        //
        // STEP 5.6 MODIFY MORE_FRAGMENT BIT & DURATION FIELD. WRITE TXD
        //
        NdisAcquireSpinLock(&pAd->pTxCfg->TxContextQueueLock[TxInfo.QueIdx]);

        if ((pPort->CommonCfg.bAutoTxRateSwitch == FALSE) || (TxInfo.bDHCPFrame == TRUE) /* || (NumberRequired > 1)*/)
        {
            TxInfo.TransferBufferLength = TxSize + TXD_LONG_SIZE+ TxInfo.LengthQosPAD;
        }
        else
        {
            TxInfo.TransferBufferLength = TxSize + TXD_SHORT_SIZE+ TxInfo.LengthQosPAD;
        }
        
        if (bIncreaseCurWriPos == TRUE)
        {
            pTxContext->CurWritePosition += 8;
            bIncreaseCurWriPos = FALSE;
        }
        else if (pTxContext->ENextBulkOutPosition == pTxContext->CurWritePosition)
        {
            PUCHAR      TempData = pAd->pHifCfg->BulkOutTempBuf[TxInfo.QueIdx];

            ASSERT(TempData != NULL);
            
            PlatformMoveMemory(TempData, &pTxContext->TransferBuffer->WirelessPacket[pTxContext->CurWritePosition], TxInfo.TransferBufferLength);
            PlatformMoveMemory(&pTxContext->TransferBuffer->WirelessPacket[pTxContext->CurWritePosition + 8], TempData, TxInfo.TransferBufferLength);
            PlatformZeroMemory(&pTxContext->TransferBuffer->WirelessPacket[pTxContext->CurWritePosition], 8);
            
            if ((pPort->CommonCfg.bAutoTxRateSwitch == FALSE) || (TxInfo.bDHCPFrame == TRUE)/* || (NumberRequired > 1)*/)
            {
                pLongFormatTxD = (PTXDSCR_LONG_STRUC)&pTxContext->TransferBuffer->WirelessPacket[pTxContext->CurWritePosition + 8];
                pTxDDW0 = &pLongFormatTxD->TxDscrDW0;
                pTxDDW7 = &pLongFormatTxD->TxDscrDW7;
                PlatformZeroMemory(pTxDDW7, 4);
            }
            else
            {
                pShortFormatTxD = (PTXDSCR_SHORT_STRUC)&pTxContext->TransferBuffer->WirelessPacket[pTxContext->CurWritePosition + 8];
                pTxDDW0 = &pShortFormatTxD->TxDscrDW0;
                pTxDDW7 = &pShortFormatTxD->TxDscrDW7;
                PlatformZeroMemory(pTxDDW7, 4);
            }
            
            pTxContext->CurWritePosition += 8;
            pTxContext->ENextBulkOutPosition += 8;
            pDest += 8;         
        }
        
        //re-caculate wireless header to prevent  TKIP MIC error
        if ((pPort->CommonCfg.bAutoTxRateSwitch == FALSE) || (TxInfo.bDHCPFrame == TRUE) /* || (NumberRequired > 1)*/)
        {
            pWirelessPacket = &pTxContext->TransferBuffer->WirelessPacket[pTxContext->CurWritePosition + TXD_LONG_SIZE];
        }
        else
        {
            pWirelessPacket = &pTxContext->TransferBuffer->WirelessPacket[pTxContext->CurWritePosition + TXD_SHORT_SIZE];
        }

    if (0)
    {
        int i = 0;
        PUCHAR p = (PUCHAR)&pTxContext->TransferBuffer->WirelessPacket[pTxContext->CurWritePosition];
        //for (i = 0; i < (int)pRxD->RxDscrDW0.RxByteCount; i = i + 8)
        for (i = 0; i < 32; i = i + 8)
        {
            DBGPRINT_RAW(RT_DEBUG_ERROR, ("[%d] %x %x %x %x %x %x %x %x\n", i, p[i], p[i+1], p[i+2], p[i+3], p[i+4], p[i+5], p[i+6], p[i+7]));
        }
    }           
        pHeader80211 = (PHEADER_802_11)pWirelessPacket;

        TxInfo.TotalPacketLength = TxSize + TxInfo.LengthQosPAD;

        DBGPRINT(RT_DEBUG_INFO, ("TxSize = %d, TxInfo.LengthQosPAD = %d\n", TxSize, TxInfo.LengthQosPAD));

        if (SrcRemainingBytes > 0) // more fragment is required
        {
             ULONG ThisNextMpduSize;
             pHeader80211->FC.MoreFrag = 1;
             ThisNextMpduSize = min(SrcRemainingBytes, pPort->CommonCfg.FragmentThreshold);

             if (ThisNextMpduSize < pPort->CommonCfg.FragmentThreshold)
             {
                // In this case, we need to include LENGTH_802_11 and LENGTH_CRC for calculating Duration.
                pHeader80211->Duration = (3 * pPort->CommonCfg.Dsifs) + 
                                    (2 * TxInfo.AckDuration) + 
                                    XmitCalcDuration(pAd, TxInfo.TxRate, ThisNextMpduSize + TxInfo.EncryptionOverhead + LENGTH_802_11 + LENGTH_CRC);
             }
             else
             {
                pHeader80211->Duration = (3 * pPort->CommonCfg.Dsifs) + 
                                (2 * TxInfo.AckDuration) + 
                                XmitCalcDuration(pAd, TxInfo.TxRate, ThisNextMpduSize + TxInfo.EncryptionOverhead);
             }

            if (Header_802_11.Frag == 0)
            {
                TxInfo.FragmentByteForTxD = 0x01;
            }
            else
            {
                TxInfo.FragmentByteForTxD = 0x10;
            }

            TxInfo.MoreData = (UCHAR)pHeader80211->FC.MoreFrag;
            TxInfo.bTxBurst = FALSE;

            if ((pPort->CommonCfg.bAutoTxRateSwitch == FALSE) || (TxInfo.bDHCPFrame == TRUE) /* || (NumberRequired > 1)*/)
            {
                XmitWriteTxD(pAd, pPort, NULL, pLongFormatTxD, TxInfo);
            }
            else
            {
                XmitWriteTxD(pAd, pPort, pShortFormatTxD, NULL, TxInfo);
            }

            // Just beginning TxWI's txop set to IFS_HTTXOP. So, change it to IFS_SIFS now.
            TxInfo.FrameGap = IFS_SIFS;     // use SIFS for all subsequent fragments
            Header_802_11.Frag++;   // increase Frag #

        }
        else
        {

            if (Header_802_11.Frag == 0)
            {
                TxInfo.FragmentByteForTxD = 0x00;
            }
            else
            {
                TxInfo.FragmentByteForTxD = 0x11;
            }
            
            pHeader80211->FC.MoreFrag = 0;
            if (pHeader80211->Addr1[0] & 0x01) // multicast/broadcast
                pHeader80211->Duration = 0;
            else
                pHeader80211->Duration = pPort->CommonCfg.Dsifs + TxInfo.AckDuration;

            if ((TxInfo.bEAPOLFrame) && (TxInfo.TxRate > RATE_6))
                TxInfo.TxRate = RATE_6;

            // To not disturb the Opps test, set psm bit if I use power save mode. 
            if (IS_P2P_SIGMA_ON(pPort))
            {
                // don't let DA broadcast packet to be WMMPS trigger frame. Test Case : 7.1.5
                if (pPort->CommonCfg.bAPSDCapable && pPort->CommonCfg.APEdcaParm.bAPSDCapable)
                {
                    if ((pHeader80211->Addr3[0]&0x01) == 0x01)
                        pHeader80211->FC.PwrMgmt = 0;
                }
                else if ((MT_TEST_BIT(pPort->P2PCfg.CTWindows, P2P_OPPS_BIT ))
                    && (pAd->StaCfg.WindowsPowerMode != DOT11_POWER_SAVING_NO_POWER_SAVING))
                    pHeader80211->FC.PwrMgmt = 1;

            }
        
            if ((TxInfo.bDHCPFrame == TRUE) || (TxInfo.bEAPOLFrame == TRUE) || (TxInfo.bWAIFrame == TRUE))
            {
                //(WLK1.1 SendRecv1x_cmn)
                //If low data rate(1M) is used, the test will fail, patched this way will not hurt normal operation.
                PHY_CFG PhyCfg = {0};

                PMAC_TABLE_ENTRY pMacTabEntry = MlmeSyncMacTabMatchedRoleType(pPort, ROLE_WLANIDX_MBCAST);

                if(pMacTabEntry != NULL)
                {
                    WRITE_PHY_CFG_DOUBLE_WORD(pAd, &PhyCfg, READ_PHY_CFG_DOUBLE_WORD(pAd, &pMacTabEntry->TxPhyCfg));
                }
                if (((PORT_P2P_ON(pPort)) || (pPort->Channel > 14)) && (READ_PHY_CFG_MODE(pAd, &PhyCfg)== MODE_CCK))
                {
                    WRITE_PHY_CFG_MODE(pAd, &PhyCfg, MODE_OFDM);
                    WRITE_PHY_CFG_MCS(pAd, &PhyCfg, OfdmRateToRxwiMCS[RATE_6]);
                }

                DBGPRINT(RT_DEBUG_TRACE, ("XmitHardTransmit, TxInfo.bEAPOLFrame = %d, TxInfo.Wcid = %x, wep = %d, mode = %d\n",
                    TxInfo.bEAPOLFrame, 
                    TxInfo.Wcid, 
                    pHeader80211->FC.Wep, 
                    READ_PHY_CFG_MODE(pAd, &PhyCfg)));

#if 1   
                TxInfo.MoreData = (UCHAR)pHeader80211->FC.MoreFrag;

                pWcidMacTabEntry = MlmeSyncMacTabMatchedWlanIdx(pPort, TxInfo.Wcid);

                if ((pPort->CommonCfg.bAutoTxRateSwitch == FALSE) || (TxInfo.bDHCPFrame == TRUE) /* || (NumberRequired > 1)*/)
                {
                    XmitWriteTxD(pAd, pPort, NULL, pLongFormatTxD, TxInfo);
                }
                else
                {
                    XmitWriteTxD(pAd, pPort, pShortFormatTxD, NULL, TxInfo);
                }
#else
                if (ADHOC_ON(pPort) && 
                     IS_HW_WAPI(pAd) && 
                     (pPort->PortCfg.WepStatus == Ndis802_11EncryptionWPI_SMS4) && 
                     (TxInfo.bMcast == TRUE))
                {
                    //
                    // The broadcast Tx key is stored in the 0xFE entry.
                    //
                    XmitWriteTxWI(pAd, 
                                    pPort, 
                                    pTxWI,  
                                    FALSE, 
                                    FALSE, 
                                    FALSE, 
                                    TxInfo.bAckRequired, 
                                    FALSE,
                                    TxInfo.BAWinSize, 
                                    BSS7Mcast_WCID, 
                                    TxSize, 
                                    TxInfo.PID, 
                                    TxInfo.MimoPs, 
                                    IFS_HTTXOP, 
                                    TxInfo.bPiggyBack, 
                                    PhyCfg,
                                    FALSE,
                                    FALSE, 
                                    FALSE);                 
                }
                else
                {
                    XmitWriteTxWI(pAd, 
                                    pPort, 
                                    pTxWI,  
                                    FALSE, 
                                    FALSE, 
                                    FALSE, 
                                    TxInfo.bAckRequired, 
                                    FALSE, 
                                    TxInfo.BAWinSize, 
                                    TxInfo.Wcid, 
                                    TxSize, 
                                    TxInfo.PID, 
                                    TxInfo.MimoPs, 
                                    IFS_HTTXOP, 
                                    TxInfo.bPiggyBack, 
                                    PhyCfg, 
                                    FALSE, 
                                    FALSE,
                                    FALSE);//pPort->MacTab.Content[MCAST_WCID].HTPhyMode);
                }

                XmitWriteTxInfo(pAd, 
                                pPort, 
                                pTxInfo, 
                                (TxSize+pAd->HwCfg.TXWI_Length + TxInfo.LengthQosPAD), 
                                TxInfo.bWiv,
                                TxInfo.QueueSel, 
                                FALSE,  
                                FALSE);     
#endif              
            }
            else if (ADHOC_ON(pPort) && (TxInfo.bMcast))//(bAdhocN_ChangeMulticastRate)
            {
                PHY_CFG PhyCfg;
                WRITE_PHY_CFG_DOUBLE_WORD(pAd, &PhyCfg, 0);
                //
                // AdhocNForceMulticastRate
                //
                // Bit[0-15]: HTTRANSMIT_SETTING
                //          Bit[0-6]: MCS, Bit[7]: BW, Bit[8]: ShortGI, Bit[9-10]: STBC, Bit[11-13]: Reserve, Bit[14-15]: PHY mode
                // Bit[16-31]: 
                //          0x1000: ALLOW_CHANGE_MCAST_RATE
                //          0x1001: ALLOW_CHANGE_MCAST_RATE and USER_DEFINE_MCAST_RATE
                //          0x1010: ALLOW_CHANGE_MCAST_RATE and USE_LEGACY_MCAST_RATE
                //
                if ((pAd->StaCfg.AdhocNForceMulticastRate & 0xFFFF0000) == USER_DEFINE_MCAST_RATE) //If bit16 on, bit[0~15] means HTTRANSMIT_SETTING
                {
                    // Error handling
                    // 1. MODE is CCK, MCS allows 0-3
                    // 2. MODE is OFDM, MCS allows 0-7
                    // 3. MODE is HTMIX or HTGF, MCS allows 0-15
                    // 4. STBC is 0 if MCS >= 8
                    if (((((pAd->StaCfg.AdhocNForceMulticastRate & 0x0000C000) >> 14) == MODE_CCK) && ((pAd->StaCfg.AdhocNForceMulticastRate & 0x0000007F) > 4))
                        || ((((pAd->StaCfg.AdhocNForceMulticastRate & 0x0000C000) >> 14) == MODE_OFDM) && ((pAd->StaCfg.AdhocNForceMulticastRate & 0x0000007F) > 8))
                        || (((((pAd->StaCfg.AdhocNForceMulticastRate & 0x0000C000) >> 14) == MODE_HTMIX) || (((pAd->StaCfg.AdhocNForceMulticastRate & 0x0000C000) >> 14) == MODE_HTGREENFIELD)) && (pAd->StaCfg.AdhocNForceMulticastRate & 0x0000007F) > 15)
                        || (((pAd->StaCfg.AdhocNForceMulticastRate & 0x0000007F) >= 8) && (((pAd->StaCfg.AdhocNForceMulticastRate & 0x00000600) >> 9) != 0)))
                    {
                        if (pPort->Channel > 14)
                        {
                            WRITE_PHY_CFG_MCS(pAd, &PhyCfg, OfdmRateToRxwiMCS[RATE_24]);
                            DBGPRINT(RT_DEBUG_TRACE,("Error: Force multicast rate to 24Mbps, AdhocNForceMulticastRate[0-15]=0x%x, MCS=%d, BW=%d, ShortGI=%d, STBC=%d, Rsv=%d, MODE=%d\n", (pAd->StaCfg.AdhocNForceMulticastRate & 0x0000FFFF), (pAd->StaCfg.AdhocNForceMulticastRate & 0x0000007F) /* MCS */,  (pAd->StaCfg.AdhocNForceMulticastRate & 0x00000080) >> 7 /* BW */,  (pAd->StaCfg.AdhocNForceMulticastRate & 0x00000100) >> 8 /* SGI */,  (pAd->StaCfg.AdhocNForceMulticastRate & 0x00000600) >> 9 /* STBC */,  (pAd->StaCfg.AdhocNForceMulticastRate & 0x00003800) >> 11 /* RSV */,  (pAd->StaCfg.AdhocNForceMulticastRate & 0x0000C000) >> 14/* MODE */));                      
                        }
                        else
                        {
                            WRITE_PHY_CFG_MCS(pAd, &PhyCfg, OfdmRateToRxwiMCS[RATE_1]);
                            DBGPRINT(RT_DEBUG_TRACE,("Error: Force multicast rate to 1Mbps, AdhocNForceMulticastRate[0-15]=0x%x, MCS=%d, BW=%d, ShortGI=%d, STBC=%d, Rsv=%d, MODE=%d\n", (pAd->StaCfg.AdhocNForceMulticastRate & 0x0000FFFF), (pAd->StaCfg.AdhocNForceMulticastRate & 0x0000007F) /* MCS */,  (pAd->StaCfg.AdhocNForceMulticastRate & 0x00000080) >> 7 /* BW */,  (pAd->StaCfg.AdhocNForceMulticastRate & 0x00000100) >> 8 /* SGI */,  (pAd->StaCfg.AdhocNForceMulticastRate & 0x00000600) >> 9 /* STBC */,  (pAd->StaCfg.AdhocNForceMulticastRate & 0x00003800) >> 11 /* RSV */,  (pAd->StaCfg.AdhocNForceMulticastRate & 0x0000C000) >> 14/* MODE */));                       
                        }
                    }
                    else
                    {
                        // Set user define multicast rate
                        WRITE_PHY_CFG_DOUBLE_WORD(pAd, &PhyCfg, (pAd->StaCfg.AdhocNForceMulticastRate & 0x0000FFFF));
                        DBGPRINT(RT_DEBUG_INFO,("User define: SoftAPForceMulticastRate[0-15]=0x%x, MCS=%d, BW=%d, ShortGI=%d, STBC=%d, Rsv=%d, MODE=%d\n", (pAd->StaCfg.AdhocNForceMulticastRate & 0x0000FFFF), (pAd->StaCfg.AdhocNForceMulticastRate & 0x0000007F) /* MCS */,  (pAd->StaCfg.AdhocNForceMulticastRate & 0x00000080) >> 7 /* BW */,  (pAd->StaCfg.AdhocNForceMulticastRate & 0x00000100) >> 8 /* SGI */,  (pAd->StaCfg.AdhocNForceMulticastRate & 0x00000600) >> 9 /* STBC */,  (pAd->StaCfg.AdhocNForceMulticastRate & 0x00003800) >> 11 /* RSV */,  (pAd->StaCfg.AdhocNForceMulticastRate & 0x0000C000) >> 14/* MODE */));
                    }
                }
                else if((pWcidMacTabEntry !=NULL) &&((pAd->StaCfg.AdhocNForceMulticastRate & 0xFFFF0000) == USE_LEGACY_MCAST_RATE))   //If bit21 on, means Force Use Legacy Multicast Rate
                {
                    WRITE_PHY_CFG_DOUBLE_WORD(pAd, &PhyCfg, READ_PHY_CFG_DOUBLE_WORD(pAd, &pWcidMacTabEntry->TxPhyCfg));
                    
                    if ((READ_PHY_CFG_MODE(pAd, &pWcidMacTabEntry->TxPhyCfg) == MODE_OFDM) && (READ_PHY_CFG_MCS(pAd, &pWcidMacTabEntry->TxPhyCfg) > MCS_4))
                    {
                        // If UnicastRate>=24, MulticastRate will fix to 24Mbps else MulticastRate will be the same with UnicastRate
                        WRITE_PHY_CFG_MODE(pAd, &PhyCfg, MODE_OFDM);
                        WRITE_PHY_CFG_MCS(pAd, &PhyCfg, OfdmRateToRxwiMCS[RATE_24]);
                        DBGPRINT(RT_DEBUG_INFO,("Auto: MODE=OFDM, MCS=%d, Force MCS to 24Mbps\n", READ_PHY_CFG_MCS(pAd, &pWcidMacTabEntry->TxPhyCfg)));
                    }
                    else if ((READ_PHY_CFG_MODE(pAd, &pWcidMacTabEntry->TxPhyCfg) >= MODE_HTMIX))
                    {
                        //Default use 24Mbps, but should not exceed Unicast Rate    
                        UCHAR uFallBackRATE = XmitMulticastRateNRateToOFDMRate(READ_PHY_CFG_MCS(pAd, &pWcidMacTabEntry->TxPhyCfg), READ_PHY_CFG_BW(pAd, &pWcidMacTabEntry->TxPhyCfg));
                        
                        WRITE_PHY_CFG_MODE(pAd, &PhyCfg, MODE_OFDM);
                        WRITE_PHY_CFG_MCS(pAd, &PhyCfg, OfdmRateToRxwiMCS[uFallBackRATE]);
                        DBGPRINT(RT_DEBUG_INFO,("Auto: MODE=HTMIX or HTGF, MCS=%d, BW=%d, Force MCS to 24Mbps\n", 
                                                READ_PHY_CFG_MCS(pAd, &pWcidMacTabEntry->TxPhyCfg), 
                                                READ_PHY_CFG_BW(pAd, &pWcidMacTabEntry->TxPhyCfg)));
                    }
                }
                else
                {
                    // Multicast rate is the same with Unicast rate.
                    // But switch off 40MHz, shortGI and STBC
                    WRITE_PHY_CFG_DOUBLE_WORD(pAd, &PhyCfg, READ_PHY_CFG_DOUBLE_WORD(pAd, &pWcidMacTabEntry->TxPhyCfg));
                    WRITE_PHY_CFG_BW(pAd, &PhyCfg, BW_20);
                    WRITE_PHY_CFG_SHORT_GI(pAd, &PhyCfg, GI_800);
                    WRITE_PHY_CFG_STBC(pAd, &PhyCfg, STBC_NONE);

                    if(pWcidMacTabEntry != NULL)
                    {
                        DBGPRINT(RT_DEBUG_INFO,("Multicast rate is the same with Unicast rate: MCS=%d, BW=%d\n", 
                                                READ_PHY_CFG_MCS(pAd, &pWcidMacTabEntry->TxPhyCfg), 
                                                READ_PHY_CFG_BW(pAd, &pWcidMacTabEntry->TxPhyCfg))); 
                     }
                }

#if 1   
                pWcidMacTabEntry = MlmeSyncMacTabMatchedWlanIdx(pPort, TxInfo.Wcid);
                
                if ( pAd->pTxCfg->SendTxWaitQueue[TxInfo.QueIdx].Number > 1  )
                    TxInfo.NextValid= 1;        
                else
                    TxInfo.NextValid = 0;           

                if ((pPort->CommonCfg.bAutoTxRateSwitch == FALSE) || (TxInfo.bDHCPFrame == TRUE) /* || (NumberRequired > 1)*/)
                {
                    XmitWriteTxD(pAd, pPort, NULL, pLongFormatTxD, TxInfo);
                }
                else
                {
                    XmitWriteTxD(pAd, pPort, pShortFormatTxD, NULL, TxInfo);
                }
#else

                if (ADHOC_ON(pPort) && 
                     IS_HW_WAPI(pAd) && 
                     (pPort->PortCfg.WepStatus == Ndis802_11EncryptionWPI_SMS4) && 
                     (TxInfo.bMcast == TRUE))
                {
                    //
                    // The broadcast Tx key is stored in the 0xFE entry.
                    //
                    XmitWriteTxWI(pAd, pPort, pTxWI,  FALSE, FALSE, TxInfo.bAMPDU, TxInfo.bAckRequired, FALSE, 
                        TxInfo.BAWinSize, BSS7Mcast_WCID, TxSize, 
                        TxInfo.PID, TxInfo.MimoPs, TxInfo.FrameGap, TxInfo.bPiggyBack, PhyCfg, FALSE, FALSE, TRUE);
                }
                else
                {
                    XmitWriteTxWI(pAd, pPort, pTxWI, FALSE, FALSE, TxInfo.bAMPDU, TxInfo.bAckRequired, FALSE, 
                        TxInfo.BAWinSize, TxInfo.Wcid, TxSize, 
                        TxInfo.PID, TxInfo.MimoPs, TxInfo.FrameGap, TxInfo.bPiggyBack, PhyCfg, FALSE ,FALSE, TRUE);
                }
                #if DBG
                //pPort->LastTxRate = (USHORT)(HTPhyMode.word);
                //WRITE_PHY_CFG_DOUBLE_WORD(pAd, &pPort->LastTxRatePhyCfg, READ_PHY_CFG_DOUBLE_WORD(pAd, &pPort->MacTab.Content[TxInfo.Wcid].TxPhyCfg));
                #endif
                
                if ( pAd->pTxCfg->SendTxWaitQueue[TxInfo.QueIdx].Number > 1  )
                    TxInfo.NextValid= 1;        
                else
                    TxInfo.NextValid = 0;           
                XmitWriteTxInfo(pAd, pPort, pTxInfo, (TxSize+pAd->HwCfg.TXWI_Length + TxInfo.LengthQosPAD), FALSE, TxInfo.QueueSel, TxInfo.NextValid,  FALSE);            
#endif              
            }
            else if (pHeader80211->Frag != 0)
            {

#if 1   
                if (!ADHOC_ON(pPort))
                {
                    //
                    // Cache the last Tx rate
                    //
                    if(pWcidMacTabEntry != NULL)
                    {
                        WRITE_PHY_CFG_DOUBLE_WORD(pAd, &pPort->LastTxRatePhyCfg, READ_PHY_CFG_DOUBLE_WORD(pAd, &pWcidMacTabEntry->TxPhyCfg));
                    }
                }
                
                if ( pAd->pTxCfg->SendTxWaitQueue[TxInfo.QueIdx].Number > 1  )
                    TxInfo.NextValid= 1;        
                else
                    TxInfo.NextValid = 0;           

                if ((pPort->CommonCfg.bAutoTxRateSwitch == FALSE) || (TxInfo.bDHCPFrame == TRUE) /* || (NumberRequired > 1)*/)
                {
                    XmitWriteTxD(pAd, pPort, NULL, pLongFormatTxD, TxInfo);
                }
                else
                {
                    XmitWriteTxD(pAd, pPort, pShortFormatTxD, NULL, TxInfo);
                }
#else
                XmitWriteTxWI(pAd, pPort, pTxWI, TRUE, FALSE, TxInfo.bAMPDU, TxInfo.bAckRequired, FALSE, 
                    TxInfo.BAWinSize, TxInfo.Wcid, TxSize, 
                    TxInfo.PID, TxInfo.MimoPs, TxInfo.FrameGap, TxInfo.bPiggyBack, pPort->MacTab.Content[TxInfo.Wcid].TxPhyCfg, FALSE, FALSE, TRUE);

                if (!ADHOC_ON(pPort))
                {
                    //
                    // Cache the last Tx rate
                    //
                    WRITE_PHY_CFG_DOUBLE_WORD(pAd, &pPort->LastTxRatePhyCfg, READ_PHY_CFG_DOUBLE_WORD(pAd, &pPort->MacTab.Content[TxInfo.Wcid].TxPhyCfg));
                }

                if (pAd->pTxCfg->SendTxWaitQueue[QueIdx].Number > 1)
                    TxInfo.NextValid = 1;
                else
                    TxInfo.NextValid = 0;               

                XmitWriteTxInfo(pAd, pPort, pTxInfo, (TxSize + pAd->HwCfg.TXWI_Length + TxInfo.LengthQosPAD), TxInfo.bWiv, TxInfo.QueueSel, TxInfo.NextValid,  FALSE);
#endif              
            }
            else
            {
                PHY_CFG PhyCfg = {0};
                 if(pWcidMacTabEntry != NULL)
                {
                    WRITE_PHY_CFG_DOUBLE_WORD(pAd, &PhyCfg, READ_PHY_CFG_DOUBLE_WORD(pAd, &pWcidMacTabEntry->TxPhyCfg));
                }
                
                //To pass SIGMA tests, we need to reduce rate to 802.11g rate
                if (IS_P2P_SIGMA_ON(pPort))
                {
                    WRITE_PHY_CFG_DOUBLE_WORD(pAd, &PhyCfg, 0);
                    WRITE_PHY_CFG_MODE(pAd, &PhyCfg, MODE_OFDM);
                    WRITE_PHY_CFG_MCS(pAd, &PhyCfg, OfdmRateToRxwiMCS[RATE_54]);
                }
                
                // Update the last tx count for Adhoc's link-speed
                if (ADHOC_ON(pPort) && IsPeerExist && (pWcidMacTabEntry != NULL))
                    pWcidMacTabEntry->LastKickTxCount ++ ;

#if 1   
                if (!ADHOC_ON(pPort) &&(pWcidMacTabEntry != NULL))
                {
                    //
                    // Cache the last Tx rate
                    //
                    WRITE_PHY_CFG_DOUBLE_WORD(pAd, &pPort->LastTxRatePhyCfg, READ_PHY_CFG_DOUBLE_WORD(pAd, &pWcidMacTabEntry->TxPhyCfg));
                }

                if (pAd->pTxCfg->SendTxWaitQueue[TxInfo.QueIdx].Number > 1)
                    TxInfo.NextValid = 1;
                else
                    TxInfo.NextValid = 0;
                
                if ((pPort->CommonCfg.bAutoTxRateSwitch == FALSE) || (TxInfo.bDHCPFrame == TRUE) /* || (NumberRequired > 1)*/)
                {
                    XmitWriteTxD(pAd, pPort, NULL, pLongFormatTxD, TxInfo);
                }
                else
                {
                    XmitWriteTxD(pAd, pPort, pShortFormatTxD, NULL, TxInfo);
                }
#else
                XmitWriteTxWI(pAd, pPort, pTxWI, FALSE, FALSE, TxInfo.bAMPDU, TxInfo.bAckRequired, FALSE, 
                    TxInfo.BAWinSize, TxInfo.CipherWcid, TxSize, 
                    TxInfo.PID, TxInfo.MimoPs, TxInfo.FrameGap, TxInfo.bPiggyBack, PhyCfg, FALSE, FALSE, TRUE);

                if (!ADHOC_ON(pPort))
                {
                    //
                    // Cache the last Tx rate
                    //
                    WRITE_PHY_CFG_DOUBLE_WORD(pAd, &pPort->LastTxRatePhyCfg, READ_PHY_CFG_DOUBLE_WORD(pAd, &pPort->MacTab.Content[TxInfo.Wcid].TxPhyCfg));
                }

                if (pAd->pTxCfg->SendTxWaitQueue[TxInfo.QueIdx].Number > 1)
                    TxInfo.NextValid = 1;
                else
                    TxInfo.NextValid = 0;               

                XmitWriteTxInfo(pAd, pPort, pTxInfo, (TxSize+pAd->HwCfg.TXWI_Length + TxInfo.LengthQosPAD), FALSE, TxInfo.QueueSel, TxInfo.NextValid,  FALSE);            

                DBGPRINT(RT_DEBUG_INFO,("RTMPHardTransmit(bEAP=%d) - %s key#%d, KeyLen=%d, TxInfo.CipherWcid = %x\n", 
                    TxInfo.bEAPOLFrame, DecodeCipherName(TxInfo.CipherAlg), TxInfo.KeyIdx, pPort->SharedKey[BSS0][TxInfo.KeyIdx].KeyLen,TxInfo.CipherWcid));        
#endif
            }
        }

        if ((pHeader80211->Frag == 0) && (pTxDDW7->SwUseNonQoS == 1))
        {
            pHeader80211->Sequence = pAd->pTxCfg->NonQosDataSeq++;
            pAd->pTxCfg->NonQosDataSeq %= 4096;
            pAd->pTxCfg->TxFragSequence = pHeader80211->Sequence;
        }
        else
        {
            if ((!ADHOC_ON(pPort)) &&
                (DLS_ON(pAd) || TDLS_ON(pAd)) &&
                (pHeader80211->FC.FrDs == 0) &&
                (pHeader80211->FC.ToDs == 0))
            {
                //DLS frame, seq no already assigned.
            }
            else
            {
                if (pHeader80211->Frag == 0)
                {
                    // Out of sequence causes the long ping issue on AdHoc 802.11n.
                    // To fix the issue, multicast/broadcast packet and unicast packet are assigned using different sequence number.
                    if(pWcidMacTabEntry !=NULL)
                    {
                        if (ADHOC_ON(pPort) && (!ETH_IS_UNICAST(pHeader80211->Addr1)))
                        {       
                            pHeader80211->Sequence = pWcidMacTabEntry->BAOriSequence[TxInfo.UserPriority]++;
                            pWcidMacTabEntry->BAOriSequence[TxInfo.UserPriority] %= 4096;
                        }       
                        else
                        {               
                            pHeader80211->Sequence = pWcidMacTabEntry->BAOriSequence[TxInfo.UserPriority]++;                   
                            pWcidMacTabEntry->BAOriSequence[TxInfo.UserPriority] %= 4096;        
                        }
                    }
                    pAd->pTxCfg->TxFragSequence = pHeader80211->Sequence;
                }
                else
                    pHeader80211->Sequence = pAd->pTxCfg->TxFragSequence;
            }

            DBGPRINT(RT_DEBUG_INFO, ("Bulk out : p80211->Sequence = %d\n", pHeader80211->Sequence));
        }

       
        
        if ((pPort->CommonCfg.bAutoTxRateSwitch == FALSE) || (TxInfo.bDHCPFrame == TRUE) /* || (NumberRequired > 1)*/)
            TxInfo.TransferBufferLength = TxSize + TXD_LONG_SIZE + TxInfo.LengthQosPAD;
        else
            TxInfo.TransferBufferLength = TxSize + TXD_SHORT_SIZE + TxInfo.LengthQosPAD;
            

        pTxDDW7->SwUseUSB4ByteAlign = 0;
        if ((TxInfo.TransferBufferLength % 4) == 1) 
        {
            PlatformZeroMemory(pDest, 11);
            TxInfo.TransferBufferLength  += 3;
            pTxDDW7->SwUseUSB4ByteAlign = 3;
            pDest  += 3;
        }
        else if ((TxInfo.TransferBufferLength % 4) == 2)    
        {
            PlatformZeroMemory(pDest, 10);
            TxInfo.TransferBufferLength  += 2;
            pTxDDW7->SwUseUSB4ByteAlign = 2;
            pDest  += 2;
        }
        else if ((TxInfo.TransferBufferLength % 4) == 3)    
        {
            PlatformZeroMemory(pDest, 9);
            TxInfo.TransferBufferLength  += 1;
            pTxDDW7->SwUseUSB4ByteAlign = 1;
            pDest  += 1;
        }

        //PlatformZeroMemory(pTxContext->TransferBuffer->Aggregation, 4);

        pTxContext->TxRate = TxInfo.TxRate;
        // Set frame gap for the rest of fragment burst.
        // It won't matter if there is only one fragment (single fragment frame).
        StartOfFrame = FALSE;
        pXcu->FragmentRequired--;

        //use remaining byte to caculate mpdu count
        if (SrcRemainingBytes <= 0)
        {
            pTxContext->LastOne = TRUE;
        }
        else
        {
            pTxContext->LastOne = FALSE;
            pTxDDW0->USBTxBurst = 1;
        }

        temp = pTxContext->CurWritePosition;
        pTxContext->CurWritePosition += TxInfo.TransferBufferLength;
        TxInfo.SwUseSegIdx = MTDecideSegmentEnd(pTxContext->CurWritePosition);
        
        pTxDDW7->SwUseSegIdx = TxInfo.SwUseSegIdx;

        if (TxInfo.SwUseSegIdx == SEGMENT_TOTAL)
        {
            pTxContext->CurWritePosition = 8;
            pTxDDW7->SwUseSegmentEnd = 1;
        }
        else if (TxInfo.SwUseSegIdx < SEGMENT_TOTAL)
        {
            pTxContext->CurWritePosition = ONE_SEGMENT_UNIT * TxInfo.SwUseSegIdx;
            pTxDDW7->SwUseSegmentEnd = 1;
        }
        else 
        {
            pTxDDW7->SwUseSegmentEnd = 0;
        }       

        DBGPRINT(RT_DEBUG_INFO, ("HArd, pTxDDW7->SwUseSegIdx = %x, TxInfo.SwUseSegIdx = %x, pTxDDW7->SwUseSegmentEnd = %x\n", pTxDDW7->SwUseSegIdx, TxInfo.SwUseSegIdx, pTxDDW7->SwUseSegmentEnd));

    if (0)
    {
        int i = 0;
        PUCHAR p = (PUCHAR)&pTxContext->TransferBuffer->WirelessPacket[temp];
        DBGPRINT(RT_DEBUG_TRACE, ("HArd, pTxDDW0->TxByteCount = %d\n", pTxDDW0->TxByteCount));
        //for (i = 0; i < (int)pRxD->RxDscrDW0.RxByteCount; i = i + 8)
        for (i = 0; i < 32; i = i + 8)
        {
            DBGPRINT_RAW(RT_DEBUG_ERROR, ("[%d] %x %x %x %x %x %x %x %x\n", i, p[i], p[i+1], p[i+2], p[i+3], p[i+4], p[i+5], p[i+6], p[i+7]));
        }
    }   
    
        pTxContext->bCurWriting = FALSE;
        MTUSB_SET_BULK_FLAG(pAd, (fRTUSB_BULK_OUT_DATA_NORMAL << TxInfo.QueIdx));

        if (TxInfo.bWMM == FALSE) 
        {
            pTxDDW7->SwUseNonQoS = 1;
        }

        NdisReleaseSpinLock(&pAd->pTxCfg->TxContextQueueLock[TxInfo.QueIdx]);
        
    }   while (SrcRemainingBytes > 0); //use remaining byte to caculate mpdu count  

    //
    // Check if we need to complete packet.
    //  
    if((pXcu != NULL) && (pXcu->Reserve1 != NULL) && (pXcu->Reserve2 != NULL))
    {
        RELEASE_NDIS_NETBUFFER(pAd, pXcu, NDIS_STATUS_SUCCESS); 
    }  

    if (pHeader80211->Addr1[0] & 0x01)
    {
        pAd->Counter.StatisticsInfo.McastCounters.ullTransmittedFrameCount++;
    }
    else
    {
        pAd->Counter.StatisticsInfo.UcastCounters.ullTransmittedFrameCount++;
    }

    // For WHCK Test
    {
        UCHAR   idx;

        for (idx = 0; idx < pAd->StaCfg.pSupportedPhyTypes->uNumOfEntries; idx++)
        {
            pAd->Counter.StatisticsInfo.PhyCounters[idx].ullTransmittedFrameCount++;
            pAd->Counter.StatisticsInfo.PhyCounters[idx].ullTransmittedFragmentCount++;
        }
        
    }

    // Check for EAPOL frame sent after MIC countermeasures
    if (pAd->StaCfg.MicErrCnt >= 3)
    {
        MLME_DISASSOC_REQ_STRUCT    DisassocReq;

        // disassoc from current AP first
        DBGPRINT(RT_DEBUG_WARN, ("MLME - disassociate with current AP after sending second continuous EAPOL frame\n"));
        MlmeCntDisassocParmFill(pAd, &DisassocReq, pPort->PortCfg.Bssid, REASON_MIC_FAILURE);
        MlmeEnqueue(pAd,pPort, ASSOC_STATE_MACHINE, MT2_MLME_DISASSOC_REQ, sizeof(MLME_DISASSOC_REQ_STRUCT), &DisassocReq);

        pPort->Mlme.CntlMachine.CurrState = CNTL_WAIT_DISASSOC;
        pAd->StaCfg.bBlockAssoc = TRUE;
    }
    
    return ndisStatus;
}

/*
    ========================================================================

    Routine Description:
        Copy BAR frame from TX queue into relative ring buffer and set 
    appropriate ASIC register to kick hardware encryption before really
    sent out to air.
        
    Arguments:
        pAd     Pointer to our adapter
        pPacket Pointer to outgoing BAR frame
        QueIdx  bulkout pipe
        
    Return Value:
        NDIS_STATUS_SUCCESS     No err
        NDIS_STATUS_RESOURCES       Error then InsertHeadQueue
        NDIS_STATUS_FAILURE         Error then RELEASE_NDIS_PACKET

    Note:
    
    ========================================================================
*/
NDIS_STATUS 
XmitSendBAR(
    IN  PMP_ADAPTER   pAd,
    IN  PMT_XMIT_CTRL_UNIT     pXcu,
    IN    UCHAR         QueIdx  
    )
{
    NDIS_STATUS         Status = NDIS_STATUS_SUCCESS;
    ULONG               SrcBufLen = 0;
    PHT_TX_CONTEXT      pTxContext;
    PTXWI_STRUC     pTxWI;
    PTXINFO_STRUC       pTxInfo;
    ULONG               FillOffset;
    BOOLEAN             bIncreaseCurWriPos = FALSE;
    PUCHAR              pDest;
    UCHAR               RAWcid ;
    ULONG               TotalPacketLength,TransferBufferLength;
    //HTTRANSMIT_SETTING    MlmeTransmit;
    PHY_CFG             MgmtPhyCfg;
    PFRAME_BAR          pBAR;
    UCHAR               PortNumber = 0;
    PMP_PORT          pPort;
    UCHAR               SwUseSegIdx = 1;
    PMAC_TABLE_ENTRY pWcidMacTabEntry = NULL;

    PortNumber = (UCHAR)pXcu->PortNumber;
    pPort = pAd->PortList[PortNumber];

    RAWcid = (UCHAR)MlmeSyncGetWlanIdxByPort(pPort, ROLE_WLANIDX_BSSID);
    
    DBGPRINT(RT_DEBUG_TRACE,("%s: --> QueIdx[%d]\n",__FUNCTION__,QueIdx));

    TotalPacketLength = pXcu->TotalPacketLength;
    
    if ((TotalPacketLength < sizeof(FRAME_BAR)) || (TotalPacketLength > MAX_FRAME_SIZE))
    {
        DBGPRINT(RT_DEBUG_ERROR, ("XmitSendBAR, size(=%d) less than 802.11 BAR or large than 2346\n", TotalPacketLength));
        return (NDIS_STATUS_FAILURE);
    }

    WRITE_PHY_CFG_DOUBLE_WORD(pAd, &MgmtPhyCfg, READ_PHY_CFG_DOUBLE_WORD(pAd, &pPort->CommonCfg.MgmtPhyCfg));

    NdisAcquireSpinLock(&pAd->pTxCfg->TxContextQueueLock[QueIdx]);
    pTxContext  = &pAd->pHifCfg->TxContext[QueIdx];
    FillOffset = pTxContext->CurWritePosition;
    
    // Check ring full.
    if (((pTxContext->CurWritePosition) < pTxContext->NextBulkOutPosition) && (pTxContext->CurWritePosition + LOCAL_TXBUF_SIZE + ONE_SEGMENT_UNIT) > pTxContext->NextBulkOutPosition)
    {
        DBGPRINT(RT_DEBUG_INFO,("XmitSendBAR c1 --> CurWritePosition = %d, NextBulkOutPosition = %d. \n", pTxContext->CurWritePosition, pTxContext->NextBulkOutPosition));
        MTUSB_SET_BULK_FLAG(pAd, (fRTUSB_BULK_OUT_DATA_NORMAL << QueIdx));
        NdisReleaseSpinLock(&pAd->pTxCfg->TxContextQueueLock[QueIdx]);
        return (NDIS_STATUS_RESOURCES);
    }
    else if ((pTxContext->CurWritePosition > (ONE_SEGMENT_UNIT * (SEGMENT_TOTAL - 1))) && (pTxContext->NextBulkOutPosition < (ONE_SEGMENT_UNIT)))
    {
        DBGPRINT(RT_DEBUG_TRACE,("XmitSendBAR c4 --> CurWritePosition = %d, NextBulkOutPosition = %d. \n", pTxContext->CurWritePosition, pTxContext->NextBulkOutPosition));
        MTUSB_SET_BULK_FLAG(pAd, (fRTUSB_BULK_OUT_DATA_NORMAL << QueIdx));
        NdisReleaseSpinLock(&pAd->pTxCfg->TxContextQueueLock[QueIdx]);
        return (NDIS_STATUS_RESOURCES);
    }
    else if ((pTxContext->CurWritePosition == 8) && (pTxContext->NextBulkOutPosition < LOCAL_TXBUF_SIZE + ONE_SEGMENT_UNIT))
    {
        DBGPRINT(RT_DEBUG_TRACE,("XmitSendBAR c2 --> CurWritePosition = %d, NextBulkOutPosition = %d. \n", pTxContext->CurWritePosition, pTxContext->NextBulkOutPosition));
        MTUSB_SET_BULK_FLAG(pAd, (fRTUSB_BULK_OUT_DATA_NORMAL << QueIdx));
        NdisReleaseSpinLock(&pAd->pTxCfg->TxContextQueueLock[QueIdx]);
        return (NDIS_STATUS_RESOURCES);
    }
    else if (pTxContext->bCurWriting == TRUE)
    {
        DBGPRINT(RT_DEBUG_TRACE,("XmitSendBAR c3 --> CurWritePosition = %d, NextBulkOutPosition = %d. \n", pTxContext->CurWritePosition, pTxContext->NextBulkOutPosition));
        NdisReleaseSpinLock(&pAd->pTxCfg->TxContextQueueLock[QueIdx]);
        return (NDIS_STATUS_RESOURCES);
    }

    if ((pTxContext->ENextBulkOutPosition == pTxContext->CurWritePosition))
    {
        bIncreaseCurWriPos = TRUE;
        pTxContext->ENextBulkOutPosition += 8;
        FillOffset += 8;
    }
    pTxContext->bCurWriting = TRUE;
    NdisReleaseSpinLock(&pAd->pTxCfg->TxContextQueueLock[QueIdx]);
    
    pTxInfo= (PTXINFO_STRUC)&pTxContext->TransferBuffer->WirelessPacket[FillOffset];
    PlatformZeroMemory(pTxInfo, TXINFO_SIZE);
    pTxWI= (PTXWI_STRUC)&pTxContext->TransferBuffer->WirelessPacket[FillOffset+TXINFO_SIZE];
    PlatformZeroMemory(pTxWI, pAd->HwCfg.TXWI_Length);
    pDest = &pTxContext->TransferBuffer->WirelessPacket[FillOffset+TXINFO_SIZE+pAd->HwCfg.TXWI_Length];

    // Copy BAR frame
    {
        ULONG    Length = NdisCommonCopyBufferFromXmitBuffer(pDest, pXcu);
        if( Length != TotalPacketLength) 
        {
            DBGPRINT(RT_DEBUG_TRACE, ("%s: Copy Pkt buffer Less than TotalPacketLength = %d, Copy Length = %d\n",__FUNCTION__,TotalPacketLength, Length));
        }
        
    }

    //Get Wcid from StartSeq;
    pBAR = (PFRAME_BAR)pXcu->BufferList[0].VirtualAddr;
    RAWcid = (UCHAR)pBAR->StartingSeq.field.StartSeq;

    pWcidMacTabEntry = MlmeSyncMacTabMatchedWlanIdx(pPort, RAWcid);  
    
    if ((pWcidMacTabEntry != NULL) && (READ_PHY_CFG_MCS(pAd, &pWcidMacTabEntry->TxPhyCfg)>= MCS_4))
    {
        WRITE_PHY_CFG_MODE(pAd, &MgmtPhyCfg, MODE_OFDM);
        WRITE_PHY_CFG_MCS(pAd, &MgmtPhyCfg, OfdmRateToRxwiMCS[RATE_24]);
    }
    
    XmitWriteTxWI(pAd, pPort, pTxWI,  FALSE, FALSE, FALSE, FALSE, FALSE, 0, RAWcid, TotalPacketLength,
        0, 0, IFS_HTTXOP, FALSE, MgmtPhyCfg, FALSE, TRUE, FALSE);
    XmitWriteTxInfo(pAd, pPort, pTxInfo, (USHORT)((USHORT)(TotalPacketLength)+pAd->HwCfg.TXWI_Length), TRUE, FIFO_EDCA, (pAd->pTxCfg->SendTxWaitQueue[QueIdx].Number > 1)?TRUE:FALSE,  FALSE);

    NdisAcquireSpinLock(&pAd->pTxCfg->TxContextQueueLock[QueIdx]);
    TransferBufferLength = TotalPacketLength + pAd->HwCfg.TXWI_Length + TXINFO_SIZE;

    if (bIncreaseCurWriPos == TRUE)
    {
        pTxContext->CurWritePosition += 8;
        bIncreaseCurWriPos = FALSE;
    }
    else if (pTxContext->ENextBulkOutPosition == pTxContext->CurWritePosition)
    {
        PUCHAR      TempData = pAd->pHifCfg->BulkOutTempBuf[QueIdx];

        ASSERT(TempData != NULL);
        
        PlatformMoveMemory(TempData, &pTxContext->TransferBuffer->WirelessPacket[pTxContext->CurWritePosition], TransferBufferLength);
        PlatformMoveMemory(&pTxContext->TransferBuffer->WirelessPacket[pTxContext->CurWritePosition + 8], TempData, TransferBufferLength);
        PlatformZeroMemory(&pTxContext->TransferBuffer->WirelessPacket[pTxContext->CurWritePosition], 8);
        pTxInfo= (PTXINFO_STRUC)&pTxContext->TransferBuffer->WirelessPacket[pTxContext->CurWritePosition + 8];
        pTxWI= (PTXWI_STRUC)&pTxContext->TransferBuffer->WirelessPacket[pTxContext->CurWritePosition + 8 +TXINFO_SIZE];
        pTxContext->CurWritePosition += 8;
        pTxContext->ENextBulkOutPosition += 8;
        pDest += 8;
        DBGPRINT(RT_DEBUG_TRACE, ("%s:BulkOut Reach CurWrite\n",__FUNCTION__));
    }

    pTxInfo= (PTXINFO_STRUC)&pTxContext->TransferBuffer->WirelessPacket[pTxContext->CurWritePosition];
    pTxWI= (PTXWI_STRUC)&pTxContext->TransferBuffer->WirelessPacket[pTxContext->CurWritePosition+TXINFO_SIZE];

    TransferBufferLength = TotalPacketLength + pAd->HwCfg.TXWI_Length + TXINFO_SIZE;
    if ((TransferBufferLength % 4) == 1)    
    {
        PlatformZeroMemory(pDest, 11);
        TransferBufferLength  += 3;
        pDest  += 3;
        //pTxInfo->SwUsepad = 3;
    }
    else if ((TransferBufferLength % 4) == 2)   
    {
        PlatformZeroMemory(pDest, 10);
        TransferBufferLength  += 2;
        pDest  += 2;
        //pTxInfo->SwUsepad = 2;
    }
    else if ((TransferBufferLength % 4) == 3)   
    {
        PlatformZeroMemory(pDest, 9);
        TransferBufferLength  += 1;
        pDest  += 1;
        //pTxInfo->SwUsepad = 1;
    }

    // USBDMATxPktLen should be multiple of 4-bytes. And it doesn't include sizeof (TXINFO_STRUC).
    WRITE_TXINFO_USBDMATxPktLen(pAd, pTxInfo, (TransferBufferLength - TXINFO_SIZE));
    pTxContext->LastOne = TRUE;
    pTxContext->CurWritePosition += TransferBufferLength;

    SwUseSegIdx = MTDecideSegmentEnd(pTxContext->CurWritePosition);
    
    WRITE_TXINFO_SwUseSegIdx(pAd, pTxInfo,  SwUseSegIdx);
    
    if (SwUseSegIdx == SEGMENT_TOTAL)
    {
        pTxContext->CurWritePosition = 8;
        WRITE_TXINFO_SwUseSegmentEnd(pAd, pTxInfo, 1);
    }
    else if (SwUseSegIdx < SEGMENT_TOTAL)
    {
        pTxContext->CurWritePosition = ONE_SEGMENT_UNIT * SwUseSegIdx;
        WRITE_TXINFO_SwUseSegmentEnd(pAd, pTxInfo, 1);
    }
    else 
    {
        WRITE_TXINFO_SwUseSegmentEnd(pAd, pTxInfo, 0);
    }
            
    pTxContext->bCurWriting = FALSE;
    MTUSB_SET_BULK_FLAG(pAd, (fRTUSB_BULK_OUT_DATA_NORMAL << QueIdx));  
    WRITE_TXINFO_SwUseNonQoS(pAd, pTxInfo, 1);

    NdisReleaseSpinLock(&pAd->pTxCfg->TxContextQueueLock[QueIdx]);

    if((pXcu != NULL) && (pXcu->Reserve1 != NULL) && (pXcu->Reserve2 != NULL))
    {
        RELEASE_NDIS_NETBUFFER(pAd, pXcu, NDIS_STATUS_SUCCESS);
    }    
    return  Status;
}


/*
    ========================================================================

    Routine Description:
        Calculates the duration which is required to transmit out frames 
    with given size and specified rate.
        
    Arguments:
        pAd     Pointer to our adapter
        Rate            Transmit rate
        Size            Frame size in units of byte
        
    Return Value:
        Duration number in units of usec

    IRQL = PASSIVE_LEVEL
    IRQL = DISPATCH_LEVEL
    
    Note:
    
    ========================================================================
*/
USHORT  XmitCalcDuration(
    IN  PMP_ADAPTER   pAd,
    IN  UCHAR           Rate,
    IN  ULONG           Size)
{
    ULONG   Duration = 0;

    if (Rate < RATE_FIRST_OFDM_RATE) // CCK
    {
        if ((Rate > RATE_1) && OPSTATUS_TEST_FLAG(pAd, fOP_STATUS_SHORT_PREAMBLE_INUSED))
            Duration = 96;  // 72+24 preamble+plcp
        else
            Duration = 192; // 144+48 preamble+plcp

        Duration += (USHORT)((Size << 4) / RateIdTo500Kbps[Rate]);
        if ((Size << 4) % RateIdTo500Kbps[Rate])
            Duration ++;
    }
    else if (Rate <= RATE_LAST_OFDM_RATE)// OFDM rates
    {
        Duration = 20 + 6;      // 16+4 preamble+plcp + Signal Extension
        Duration += 4 * (USHORT)((11 + Size * 4) / RateIdTo500Kbps[Rate]);
        if ((11 + Size * 4) % RateIdTo500Kbps[Rate])
            Duration += 4;
    }
    else    //mimo rate
    {
        Duration = 20 + 6;      // 16+4 preamble+plcp + Signal Extension
    }
    
    return (USHORT)Duration;
    
}

VOID XmitSendPsPollFrame(
    IN  PMP_ADAPTER   pAd,
    IN  PMP_PORT      pPort) 
{
#if 0
    PTXINFO_STRUC   pTxInfo;
    PTXWI_STRUC     pTxWI;

    if (pAd->pHifCfg->PsPollContext.InUse == FALSE)
    {
        // Set the in use bit
        pAd->pHifCfg->PsPollContext.InUse = TRUE;
    
        PlatformZeroMemory(&pAd->pHifCfg->PsPollContext.TransferBuffer->WirelessPacket[0], 100);
        pTxInfo = (PTXINFO_STRUC)&pAd->pHifCfg->PsPollContext.TransferBuffer->WirelessPacket[0];
        XmitWriteTxInfo(pAd, pPort, pTxInfo, (USHORT)(sizeof(PSPOLL_FRAME)+pAd->HwCfg.TXWI_Length), TRUE, EpToQueue[MGMTPIPEIDX], FALSE,  FALSE);
        pTxWI = (PTXWI_STRUC)&pAd->pHifCfg->PsPollContext.TransferBuffer->WirelessPacket[TXINFO_SIZE];
        XmitWriteTxWI(pAd, pPort, pTxWI,  FALSE, FALSE, FALSE, TRUE, FALSE, 0, RESERVED_WCID, (sizeof(PSPOLL_FRAME)),
            0, 0, IFS_HTTXOP, FALSE ,pPort->CommonCfg.MgmtPhyCfg, FALSE, TRUE, FALSE);

        PlatformMoveMemory(&pAd->pHifCfg->PsPollContext.TransferBuffer->WirelessPacket[pAd->HwCfg.TXWI_Length+TXINFO_SIZE], &pPort->PsPollFrame, sizeof(PSPOLL_FRAME));
        // Append 4 extra zero bytes.
        pAd->pHifCfg->PsPollContext.BulkOutSize =  TXINFO_SIZE + pAd->HwCfg.TXWI_Length + sizeof(PSPOLL_FRAME) + 4;

        DBGPRINT(RT_DEBUG_ERROR, ("!!! XmitSendPsPollFrame !!! \n"));
        MTUSB_SET_BULK_FLAG(pAd, fRTUSB_BULK_OUT_PSPOLL);

        // Kick bulk out 
        N6USBKickBulkOut(pAd);
    }
#else
    PTX_CONTEXT             pPsPollContext;
    PTXDSCR_LONG_STRUC      pLongFormatTxD;

    PUCHAR          ConcatedPoint;

    DBGPRINT_RAW(RT_DEBUG_TRACE, ("--->XmitSendPsPollFrame %d, Endpoint = %d, Queue = %d, QueueID need to check later\n", pAd->TrackInfo.PacketCountFor7603Debug, 0, 0));

    pPsPollContext = &pAd->pHifCfg->PsPollContext;
    if (pPsPollContext->InUse == FALSE)
    {
        // Set the in use bit
        pAd->pHifCfg->PsPollContext.InUse = TRUE;
        pPsPollContext->BulkOutSize = 0;

        PlatformZeroMemory(&pPort->PsPollFrame, sizeof(PSPOLL_FRAME));
        //pPort->PsPollFrame.FC.ToDs = 1;
        //Set PM bit in ps-poll, to fix WLK PowerSaveMode_ext failure issue.
        pPort->PsPollFrame.FC.PwrMgmt = 1;
        pPort->PsPollFrame.FC.Type = BTYPE_CNTL;
        pPort->PsPollFrame.FC.SubType = SUBTYPE_PS_POLL;
        pPort->PsPollFrame.Aid = pAd->MlmeAux.Aid/* | 0xC000*/;
        COPY_MAC_ADDR(pPort->PsPollFrame.Bssid, pPort->PortCfg.Bssid);
        COPY_MAC_ADDR(pPort->PsPollFrame.Ta, pPort->CurrentAddress);
    
        PlatformZeroMemory(&pPsPollContext->TransferBuffer->WirelessPacket[0], 100);
        pPsPollContext->BulkOutSize = 0;
        
        ConcatedPoint = &pPsPollContext->TransferBuffer->WirelessPacket[0];

        pLongFormatTxD = (PTXDSCR_LONG_STRUC)&ConcatedPoint[0];

        pLongFormatTxD->TxDscrDW0.DestinationQueueID = 0; //QueueId; if there are 2 WMMs, the QueueId need change
        pLongFormatTxD->TxDscrDW0.DestinationPortID = 0;
        pLongFormatTxD->TxDscrDW0.TxByteCount = sizeof(TXDSCR_LONG_STRUC) + sizeof(PSPOLL_FRAME);
        pLongFormatTxD->TxDscrDW1.WLANIndex = 1;
        pLongFormatTxD->TxDscrDW1.HeaderLength = (sizeof(PSPOLL_FRAME) / 2); // word base
        pLongFormatTxD->TxDscrDW1.HeaderFormat = 0x2;
        pLongFormatTxD->TxDscrDW1.TxDescriptionFormat = TXD_LONG_FORMAT;
        pLongFormatTxD->TxDscrDW1.NoAck = 0;
        pLongFormatTxD->TxDscrDW2.BMPacket = 0;
        pLongFormatTxD->TxDscrDW2.FixedRate = 1;
        pLongFormatTxD->TxDscrDW2.BADisable = 1;
        pLongFormatTxD->TxDscrDW2.Duration = 1;   // for LMAC not update the duration field. PS-Poll use the duration field as AID
        pLongFormatTxD->TxDscrDW3.RemainingTxCount = 0x1f;
        pLongFormatTxD->TxDscrDW5.BARSSNContorl = 1;
        //pLongFormatTxD->TxDscrDW5.PowerManagement = 1; // PS-POLL with PS bit = 1, one PS-POLL gets one data packet from AP. PS bit = 0, one PS-POLL gets more data packets from AP.
        pLongFormatTxD->TxDscrDW6.RateToBeFixed = 0x0;  // BW_20
        pLongFormatTxD->TxDscrDW6.FixedBandwidthMode = 0x100;  // BW_20

        //pLongFormatTxD->TxDscrDW6.AntennaPriority = 2;  // BW_20
        //pLongFormatTxD->TxDscrDW6.SpetialExtenstionEnable = 1;  // BW_20
        ConcatedPoint += sizeof(TXDSCR_LONG_STRUC);
        pPsPollContext->BulkOutSize += sizeof(TXDSCR_LONG_STRUC);

        PlatformMoveMemory(ConcatedPoint, &pPort->PsPollFrame, sizeof(PSPOLL_FRAME));
        ConcatedPoint += sizeof(PSPOLL_FRAME);
        pPsPollContext->BulkOutSize += sizeof(PSPOLL_FRAME);

        if (pPsPollContext->BulkOutSize % 4 == 1)
        {
            //pLongFormatTxD->TxDscrDW0.TxByteCount += 3;
            //ConcatedPoint += 3;
            pPsPollContext->BulkOutSize += 3;
        }
        else if (pPsPollContext->BulkOutSize % 4 == 2) 
        {
            //pLongFormatTxD->TxDscrDW0.TxByteCount += 2;
            //ConcatedPoint += 2;
            pPsPollContext->BulkOutSize += 2;
        }
        else if (pPsPollContext->BulkOutSize % 4 == 3) 
        {
            //pLongFormatTxD->TxDscrDW0.TxByteCount += 1;
            //ConcatedPoint += 1;
            pPsPollContext->BulkOutSize += 1;
        }

        DBGPRINT(RT_DEBUG_TRACE, ("PS-Poll Frame total length = %d, pPort->PsPollFrame.Aid = %d\n", sizeof(TXDSCR_LONG_STRUC) + sizeof(PSPOLL_FRAME), pPort->PsPollFrame.Aid));

        pPsPollContext->BulkOutSize += 4; // Bulk out end padding

        XmitSendMlmeCmdPkt(pAd, TXPKT_PS_POLL_FRAME);
    }

#endif
}

/*
    ========================================================================

    Routine Description:
        Check the out going frame, if this is an DHCP or ARP datagram
    will be duplicate another frame at low data rate transmit.
        
    Arguments:
        pAd         Pointer to our adapter
        pPacket     Pointer to outgoing Ndis frame
        
    Return Value:       
        TRUE        To be duplicate at Low data rate transmit. (1mb)
        FALSE       Do nothing.

    IRQL = DISPATCH_LEVEL
    
    Note:

        MAC header + IP Header + UDP Header
          14 Bytes    20 Bytes
          
        UDP Header
        00|01|02|03|04|05|06|07|08|09|10|11|12|13|14|15|
                        Source Port
        16|17|18|19|20|21|22|23|24|25|26|27|28|29|30|31|
                    Destination Port

        port 0x43 means Bootstrap Protocol, server. 
        Port 0x44 means Bootstrap Protocol, client. 

    ========================================================================
*/
BOOLEAN 
XmitCheckDHCPFrame(
    IN  PMP_ADAPTER   pAd,
    IN  PMT_XMIT_CTRL_UNIT    pXcu,
    OUT PUSHORT         Protocol,
    OUT BOOLEAN         *pbGroupMsg2,
    OUT BOOLEAN         *pDHCPv6
    )   
{
    PHEADER_802_11  pHeader80211;   
    UCHAR           ProtocolByte0;
    UCHAR           ProtocolByte1;
    UCHAR           SourcePort = 0;
    UCHAR           DestPort = 0;
    ULONG           TotalPacketLength = pXcu->TotalPacketLength;
    ULONG           BiasCnt = 0;
    PUCHAR          header = NULL;
    UCHAR           NextHeader = 0;
    UCHAR           ICMPv6Type = 0;
    PMP_PORT    pPort = pAd->PortList[pXcu->PortNumber];
#if DBG 
    ULONG           TempOffset = 0;
#endif

    pHeader80211 = (PHEADER_802_11) pXcu->pHeader80211;

    if (pHeader80211->FC.SubType & 0x08)
        BiasCnt = 2;
    else
        BiasCnt = 0;

    // These two octets must be in first MDL.
    N6XmitGetByteFromMdl(pAd, pXcu, (30 + BiasCnt), &ProtocolByte0);
    N6XmitGetByteFromMdl(pAd, pXcu, (31 + BiasCnt), &ProtocolByte1);

    *Protocol =ProtocolByte0 * 256 + ProtocolByte1;
    *pbGroupMsg2 = FALSE;

#if DBG
    // IPv4
    if (*Protocol == 0x0800)
    {
        USHORT  Ver = 0;
        USHORT  HdrLen = 0;
        USHORT  TotalLen = 0;
        USHORT  Id = 0;
        
        UCHAR   Type = 0;
        UCHAR   Code = 0;
        USHORT  CheckSum = 0;
        USHORT  ICMPId = 0;
        USHORT  Seq = 0;
        UCHAR   temp = 0;
        UCHAR   NextProtocol = 0;
        
        // IPv4 start address
        TempOffset = (31 + BiasCnt + 1);

        N6XmitGetByteFromMdl(pAd, pXcu, (TempOffset + 9), &ProtocolByte0);

        // NextProtocol == ICMP 
        NextProtocol = ProtocolByte0;
        
        if (NextProtocol == 0x01)
        {
            // IPv4
            N6XmitGetByteFromMdl(pAd, pXcu, (TempOffset + 0), &ProtocolByte0);
            temp = ProtocolByte0;
            Ver = (ProtocolByte0 & 0xf0) >> 4;
            HdrLen = (ProtocolByte0 & 0x0f) * 4;
            N6XmitGetByteFromMdl(pAd, pXcu, (TempOffset + 2), &ProtocolByte0);
            N6XmitGetByteFromMdl(pAd, pXcu, (TempOffset + 3), &ProtocolByte1);
            TotalLen = (ProtocolByte0 * 256 + ProtocolByte1);
            N6XmitGetByteFromMdl(pAd, pXcu, (TempOffset + 4), &ProtocolByte0);
            N6XmitGetByteFromMdl(pAd, pXcu, (TempOffset + 5), &ProtocolByte1);
            Id = (ProtocolByte0 * 256 + ProtocolByte1);

            // ICMP start address
            TempOffset = TempOffset + 20;
            N6XmitGetByteFromMdl(pAd, pXcu, (TempOffset + 0), &ProtocolByte0);
            Type = ProtocolByte0;
            N6XmitGetByteFromMdl(pAd, pXcu, (TempOffset + 1), &ProtocolByte0);
            Code = ProtocolByte0;
            N6XmitGetByteFromMdl(pAd, pXcu, (TempOffset + 2), &ProtocolByte0);
            N6XmitGetByteFromMdl(pAd, pXcu, (TempOffset + 3), &ProtocolByte1);
            CheckSum = (ProtocolByte0 * 256 + ProtocolByte1);
            N6XmitGetByteFromMdl(pAd, pXcu, (TempOffset + 4), &ProtocolByte0);
            N6XmitGetByteFromMdl(pAd, pXcu, (TempOffset + 5), &ProtocolByte1);
            ICMPId = (ProtocolByte0 * 256 + ProtocolByte1);
            N6XmitGetByteFromMdl(pAd, pXcu, (TempOffset + 6), &ProtocolByte0);
            N6XmitGetByteFromMdl(pAd, pXcu, (TempOffset + 7), &ProtocolByte1);
            Seq = (ProtocolByte0 * 256 + ProtocolByte1);
            
            DBGPRINT(RT_DEBUG_INFO,("[Port %d] [IPv4 Tx] Ver(%d), HdrLen(%2d), TotalLen(%d), Id(%5d) [ICMP Tx] Type(%d), Code(%d), CheckSum(%d), ICMPId(%d), Seq(%d)\n", 
                                        pXcu->PortNumber,
                                        Ver,
                                        HdrLen,
                                        TotalLen,
                                        Id,
                                        Type, 
                                        Code, 
                                        CheckSum, 
                                        ICMPId,
                                        Seq));  
        }
        else
        {
            DBGPRINT(RT_DEBUG_INFO,("[Port %d] [IPv4 Tx NOT ICMP] Ver(%d), HdrLen(%2d), TotalLen(%d), Id(%5d), NextProtocol(%d)\n", 
                                        pXcu->PortNumber,
                                        Ver,
                                        HdrLen,
                                        TotalLen,
                                        Id,
                                        NextProtocol));         
        }
    }
#endif

    if (*Protocol == 0x888e)
    {
        UCHAR EAPOLType, KeyInfo[2];
        PKEY_INFO pKeyInfo;
    
        N6XmitGetByteFromMdl(pAd, pXcu, (33 + BiasCnt), &EAPOLType);

        // for Aegis_CCX (CCKM), to encrypt GroupMsg2 
        if (EAPOLType == EAPOLKey)
        {
            N6XmitGetByteFromMdl(pAd, pXcu, (37 + BiasCnt), &KeyInfo[0]);
            N6XmitGetByteFromMdl(pAd, pXcu, (38 + BiasCnt), &KeyInfo[1]);

            pKeyInfo = (PKEY_INFO)KeyInfo;

            *pbGroupMsg2 = (pKeyInfo->KeyType == 0)? TRUE:FALSE;
            DBGPRINT(RT_DEBUG_TRACE,("EAPOLKEY: KeyType=%d, *pbGroupMsg2=%d, TotalPacketLength=%d  \n", pKeyInfo->KeyType, *pbGroupMsg2, TotalPacketLength));

        }

        return FALSE;
    }
    else if (*Protocol == 0x86dd)
    {
        if (!N6XmitGetByteFromMdl(pAd, pXcu, (38 + BiasCnt), &NextHeader))
            return FALSE;

        if (NextHeader == 0x3A)
        {
            if (!N6XmitGetByteFromMdl(pAd, pXcu, (72 + BiasCnt), &ICMPv6Type))
                return FALSE;

            if (ICMPv6Type ==  128)
                *pDHCPv6 = FALSE;
            else if (ICMPv6Type == 129) 
                *pDHCPv6 = FALSE;
            else
            {
                DBGPRINT(RT_DEBUG_TRACE,("IPv6: ICMPv6Type = %d\n", ICMPv6Type));
                *pDHCPv6 = TRUE;

                // 1->disable "bypass RA packet" 
                if (pPort->CommonCfg.MPolicy != 1)
                    pPort->CommonCfg.AcceptICMPv6RA = TRUE;
            }
            return FALSE;   
        }
        return FALSE;
    }   

    if (!N6XmitGetByteFromMdl(pAd, pXcu, (53 + BiasCnt), &SourcePort))
        return FALSE;

    if (!N6XmitGetByteFromMdl(pAd, pXcu, (55 + BiasCnt), &DestPort))
        return FALSE;

    // Check for DHCP & BOOTP protocol
    if ((SourcePort != 0x44) || (DestPort != 0x43))
    {
        // 
        // 2054 (hex 0806) for ARP datagrams
        // if this packet is not ARP datagrams, then do nothing
        // ARP datagrams will also be duplicate at 1mb broadcast frames
        //
        if (*Protocol != 0x0806)
            return FALSE;
    }

    DBGPRINT(RT_DEBUG_INFO,("XmitCheckDHCPFrame - DHCP or ARP Packet \n"));

    return TRUE;
}

/*
    ========================================================================

    Routine Description:
        Resume MSDU transmission
        
    Arguments:
        pAd     Pointer to our adapter
        
    Return Value:
        None
        
    Note:
    
    ========================================================================
*/
VOID XmitResumeMsduTransmission(
    IN  PMP_ADAPTER   pAd)
{
    TX_RTS_CFG_STRUC RtsCfg;    
    PMP_PORT       pPort = pAd->PortList[PORT_0];

    DBGPRINT(RT_DEBUG_ERROR,("SCAN done, resume MSDU transmission ...\n"));

    MT_CLEAR_FLAG(pAd, fRTMP_ADAPTER_BSS_SCAN_IN_PROGRESS);

    if (pPort->CommonCfg.bCarrierDetect && pPort->CommonCfg.bCarrierDetectPhase)
    {
        if (pPort->CommonCfg.bCarrierAssert)
        {
            // Carrier assertion, driver can not send anything, change detection with no cts-to-self
            AsicSetCarrierDetectAction(pAd, CARRIER_ACTION_NO_CTS, 1000);
        }
        else
        {
            AsicSetCarrierDetectAction(pAd, CARRIER_ACTION_ONE_CTS, 1000);
        }
    }
    
    //
    // After finish BSS_SCAN_IN_PROGRESS, we need to restore Current R66 value
    // R66 should not be 0
    //
    //ASSERT(pAd->HwCfg.BbpTuning.R66CurrentValue != 0);
    if (pAd->HwCfg.BbpTuning.R66CurrentValue == 0)
    {
        pAd->HwCfg.BbpTuning.R66CurrentValue = 0x38;

        DBGPRINT_ERR(("RTMPResumeMsduTransmission, R66CurrentValue=0...\n"));
    }
    
    if (pAd->HwCfg.VhtRxAgcVgaTuningCtrl.bEnableVhtRxAgcVgaTuning == TRUE)
    {
        RTUSBWriteBBPRegister(pAd, BBP_R66, pAd->HwCfg.BbpTuning.R66CurrentValue);    
    }

    //  
    // Update RTS threshold, enable RTS threshold function after scan
    //
    if (pPort->CommonCfg.RtsThreshold < 2347)
    {
        RTUSBReadMACRegister(pAd, TX_RTS_CFG, &RtsCfg.word);
        RtsCfg.field.RtsThres = pPort->CommonCfg.RtsThreshold;
        RTUSBWriteMACRegister(pAd, TX_RTS_CFG, RtsCfg.word);                
    }

    P2pResumeMsduAction(pAd, pPort);
    
    if ((!MT_TEST_FLAG(pAd, fRTMP_ADAPTER_RESET_IN_PROGRESS)) &&
        (!MT_TEST_FLAG(pAd, fRTMP_ADAPTER_HALT_IN_PROGRESS)) &&
        (!MT_TEST_FLAG(pAd, fRTMP_ADAPTER_RADIO_OFF)))
    {
        NdisSetEvent(&(GET_THREAD_EVENT(&pAd->pHifCfg->hDequeueDataThread)));
    }

    // Kick bulk out
    N6USBKickBulkOut(pAd);
}

/*
    ========================================================================

    Routine Description:
        
    Arguments:
        
    Return Value:
        
    Note:
    
    ========================================================================
*/
USHORT  XmitCalcUsbDuration(
    IN  PMP_ADAPTER   pAd,
    IN  UCHAR           Rate,
    IN  ULONG           Size)
{
    ULONG   Duration = 0;
    PMP_PORT    pPort = pAd->PortList[FXXK_PORT_0];

    if (Rate < RATE_FIRST_OFDM_RATE) // CCK
    {
        if ((Rate > RATE_1) && (pPort->CommonCfg.TxPreamble == Rt802_11PreambleShort))
            Duration = 96;  // 72+24 preamble+plcp
        else
            Duration = 192; // 144+48 preamble+plcp
        
        Duration += (USHORT)((Size << 4) / RateIdTo500Kbps[Rate]);
        if ((Size << 4) % RateIdTo500Kbps[Rate])
            Duration ++;
    }
    else // OFDM rates
    {
        Duration = 20 + 6;      // 16+4 preamble+plcp + Signal Extension
        Duration += 4 * (USHORT)((11 + Size * 4) / RateIdTo500Kbps[Rate]);
        if ((11 + Size * 4) % RateIdTo500Kbps[Rate])
            Duration += 4;
    }
    
    return (USHORT)Duration;    
}

/* 
    ==========================================================================
    Description:
        Send out a NULL frame to AP. The prpose is to inform AP this client 
        current PSM bit.
    NOTE:
        This routine should only be used in infrastructure mode.
    ==========================================================================
 */
VOID 
XmitSendNullFrameForWake(
    IN  PMP_ADAPTER   pAd,
    IN  PMP_PORT      pPort,
    IN  UCHAR           TxRate,
    IN  BOOLEAN         bQosNull
    ) 
{
    PTX_CONTEXT     pNullContext;
    //PTXD_STRUC        pTxD;

    PTXINFO_STRUC   pTxInfo;
    PTXWI_STRUC     pTxWI;
    UCHAR   Wcid =0;
    UNREFERENCED_PARAMETER(TxRate); 

    // not sending NULL frame if it is not idle at last second
    // When APSD is on, we should send null packet to inform AP that STA is in PS mode.
    //if (!pPort->CommonCfg.bAPSDCapable && pAd->pHifCfg->BulkOutDataOneSecCount)
    //  return;

    if ((MT_TEST_FLAG(pAd, fRTMP_ADAPTER_RESET_IN_PROGRESS)) ||
        (MT_TEST_FLAG(pAd, fRTMP_ADAPTER_BULKOUT_RESET)) ||
        (MT_TEST_FLAG(pAd, fRTMP_ADAPTER_HALT_IN_PROGRESS)) ||
        (MT_TEST_FLAG(pAd, fRTMP_ADAPTER_BSS_SCAN_IN_PROGRESS)) ||        
        (MT_TEST_FLAG(pAd, fRTMP_ADAPTER_NIC_NOT_EXIST)))
    {
        return;
    }
    
    // WPA 802.1x secured port control
    if (((pPort->PortCfg.AuthMode == Ralink802_11AuthModeWPA) || 
        (pPort->PortCfg.AuthMode == Ralink802_11AuthModeWPAPSK) ||
        (pPort->PortCfg.AuthMode == Ralink802_11AuthModeWPA2) ||
        (pPort->PortCfg.AuthMode == Ralink802_11AuthModeWPA2PSK) ||
        (pPort->StaCfg.WscControl.WscState != WSC_STATE_OFF)) &&
        (pAd->StaCfg.PortSecured == WPA_802_1X_PORT_NOT_SECURED))
    {
        return;
    }

    pNullContext = &(pAd->pHifCfg->NullContext);
    if (pNullContext->InUse == FALSE)
    {
        // Set the in use bit
        pNullContext->InUse = TRUE;
    
        PlatformZeroMemory(&pPort->NullFrame, sizeof(HEADER_802_11));
        
        pPort->NullFrame.FC.Type = BTYPE_DATA;
        pPort->NullFrame.FC.SubType = SUBTYPE_NULL_FUNC;
        if (bQosNull == TRUE)
            pPort->NullFrame.FC.SubType = SUBTYPE_QOS_NULL;
        pPort->NullFrame.FC.ToDs = 1;
        if (pPort->CommonCfg.bAPSDForcePowerSave)
        {
            pPort->NullFrame.FC.PwrMgmt = PWR_SAVE;
        }
        else
        {
            pPort->NullFrame.FC.PwrMgmt = (pAd->StaCfg.Psm == PWR_SAVE);
        }
        pPort->NullFrame.Duration = pPort->CommonCfg.Dsifs + XmitCalcDuration(pAd, RateIdToMbps[READ_PHY_CFG_MCS(pAd, &pPort->CommonCfg.MgmtPhyCfg)], 14);

        pAd->pTxCfg->Sequence       = ((pAd->pTxCfg->Sequence) + 1) & (MAX_SEQ_NUMBER);
        pPort->NullFrame.Sequence = pAd->pTxCfg->Sequence;
        COPY_MAC_ADDR(pPort->NullFrame.Addr1, BROADCAST_ADDR);
        COPY_MAC_ADDR(pPort->NullFrame.Addr2, pPort->CurrentAddress);
        COPY_MAC_ADDR(pPort->NullFrame.Addr3, BROADCAST_ADDR);

        PlatformZeroMemory(&pAd->pHifCfg->NullContext.TransferBuffer->WirelessPacket[0], 100);
        pTxInfo = (PTXINFO_STRUC)&pAd->pHifCfg->NullContext.TransferBuffer->WirelessPacket[0];
        pTxWI = (PTXWI_STRUC)&pAd->pHifCfg->NullContext.TransferBuffer->WirelessPacket[TXINFO_SIZE];

        if (bQosNull == TRUE)
        {
            XmitWriteTxInfo(pAd, pPort, pTxInfo, (USHORT)(sizeof(HEADER_802_11)+ 2 + pAd->HwCfg.TXWI_Length), TRUE, EpToQueue[MGMTPIPEIDX], FALSE,  FALSE);
#ifdef MULTI_CHANNEL_SUPPORT
#else
            WRITE_TXINFO_QSEL(pAd, pTxInfo, FIFO_EDCA);
#endif  /* MULTI_CHANNEL_SUPPORT */
            Wcid = (UCHAR)MlmeSyncGetWlanIdxByPort(pPort, ROLE_WLANIDX_BSSID);
            XmitWriteTxWI(pAd, pPort, pTxWI,  FALSE, FALSE, FALSE, FALSE, FALSE, 0, Wcid, (sizeof(HEADER_802_11)+2),
                0, 0, IFS_HTTXOP, FALSE ,pPort->CommonCfg.MgmtPhyCfg, FALSE, FALSE, TRUE);
            pAd->pHifCfg->NullContext.BulkOutSize =  TXINFO_SIZE + pAd->HwCfg.TXWI_Length + sizeof(pPort->NullFrame) + 8;
            DBGPRINT(RT_DEBUG_ERROR, ("%s - send QoS NULL Frame @%d Mbps...\n", __FUNCTION__, RateIdToMbps[READ_PHY_CFG_MCS(pAd, &pPort->CommonCfg.MgmtPhyCfg)]));
        }
        else
        {
            XmitWriteTxInfo(pAd, pPort, pTxInfo, (USHORT)(sizeof(HEADER_802_11)+pAd->HwCfg.TXWI_Length), TRUE, EpToQueue[MGMTPIPEIDX], FALSE,  FALSE);
#ifdef MULTI_CHANNEL_SUPPORT
#else
            WRITE_TXINFO_QSEL(pAd, pTxInfo, FIFO_EDCA);
#endif  /* MULTI_CHANNEL_SUPPORT */
            Wcid = (UCHAR)MlmeSyncGetWlanIdxByPort(pPort, ROLE_WLANIDX_BSSID);
            XmitWriteTxWI(pAd, pPort, pTxWI,  FALSE, FALSE, FALSE, FALSE, FALSE, 0, Wcid, (sizeof(HEADER_802_11)),
                0, 0, IFS_HTTXOP, FALSE ,pPort->CommonCfg.MgmtPhyCfg, FALSE, FALSE, TRUE);
            pAd->pHifCfg->NullContext.BulkOutSize =  TXINFO_SIZE + pAd->HwCfg.TXWI_Length + sizeof(pPort->NullFrame) + 4;
        }
        // The last 2 bytes of QoS Null frame is zero. So don't need to movememory 2-bytes QoS fields .
        PlatformMoveMemory(&pAd->pHifCfg->NullContext.TransferBuffer->WirelessPacket[pAd->HwCfg.TXWI_Length+TXINFO_SIZE], &pPort->NullFrame, sizeof(HEADER_802_11));

        // Fill out frame length information for global Bulk out arbitor
        //pNullContext->BulkOutSize = TransferBufferLength;
        DBGPRINT(RT_DEBUG_ERROR, ("%s - send NULL Frame @%d Mbps...\n", __FUNCTION__, RateIdToMbps[TxRate]));
        XmitSendMlmeCmdPkt(pAd, TXPKT_NULL_FRAME);
    }
}

/*
    ==========================================================================
    Description:
        Send out a NULL frame to direct link Peer. The prpose is to inform DL Peer
        I am alive.
    NOTE:
        This routine should only be used in infrastructure mode.
    ==========================================================================
 */
VOID 
XmitSendNullFrameForDirect(
    IN  PMP_ADAPTER   pAd,
    IN  PMP_PORT      pPort,
    //IN    HTTRANSMIT_SETTING  MinTransmit,
    PHY_CFG             MinPhyCfg,
    IN  BOOLEAN         bQosNull,
    IN  PUCHAR          pDA
    ) 
{
    PTX_CONTEXT pNullContext;
    //PTXD_STRUC        pTxD;

    PTXINFO_STRUC       pTxInfo;
    PTXWI_STRUC     pTxWI;
    //HTTRANSMIT_SETTING    Transmit;
    PHY_CFG         PhyCfg;
    UCHAR   Wcid = 0;

    // not sending NULL frame if it is not idle at last second
    // When APSD is on, we should send null packet to inform AP that STA is in PS mode.
    //if (!pPort->CommonCfg.bAPSDCapable && pAd->pHifCfg->BulkOutDataOneSecCount)
    //  return;


    if ((MT_TEST_FLAG(pAd, fRTMP_ADAPTER_RESET_IN_PROGRESS)) ||
        (MT_TEST_FLAG(pAd, fRTMP_ADAPTER_BULKOUT_RESET)) ||
        (MT_TEST_FLAG(pAd, fRTMP_ADAPTER_HALT_IN_PROGRESS)) ||
        (MT_TEST_FLAG(pAd, fRTMP_ADAPTER_BSS_SCAN_IN_PROGRESS)) ||
        (MT_TEST_FLAG(pAd, fRTMP_ADAPTER_NIC_NOT_EXIST)))
    {
        return;
    }
    
    // WPA 802.1x secured port control
    if (((pPort->PortCfg.AuthMode == Ralink802_11AuthModeWPA) || 
        (pPort->PortCfg.AuthMode == Ralink802_11AuthModeWPAPSK) ||
        (pPort->PortCfg.AuthMode == Ralink802_11AuthModeWPA2) ||
        (pPort->PortCfg.AuthMode == Ralink802_11AuthModeWPA2PSK) ||
        (pPort->StaCfg.WscControl.WscState != WSC_STATE_OFF)) &&
        (pAd->StaCfg.PortSecured == WPA_802_1X_PORT_NOT_SECURED))
    {
        return;
    }

    if(READ_PHY_CFG_MODE(pAd, &MinPhyCfg)== MODE_CCK)
        WRITE_PHY_CFG_DOUBLE_WORD(pAd, &PhyCfg, READ_PHY_CFG_DOUBLE_WORD(pAd, &MinPhyCfg));
    else
        WRITE_PHY_CFG_DOUBLE_WORD(pAd, &PhyCfg, READ_PHY_CFG_DOUBLE_WORD(pAd, &pPort->CommonCfg.MgmtPhyCfg));

    pNullContext = &(pAd->pHifCfg->NullContext);
    if (pNullContext->InUse == FALSE)
    {
        // Set the in use bit
        pNullContext->InUse = TRUE;
    
        PlatformZeroMemory(&pPort->NullFrame, sizeof(HEADER_802_11));

        pPort->NullFrame.FC.Type = BTYPE_DATA;
        pPort->NullFrame.FC.SubType = SUBTYPE_NULL_FUNC;
        if (bQosNull == TRUE)
            pPort->NullFrame.FC.SubType = SUBTYPE_QOS_NULL;

        if (pPort->CommonCfg.bAPSDForcePowerSave)
        {
            pPort->NullFrame.FC.PwrMgmt = PWR_SAVE;
        }
        else
        {
            pPort->NullFrame.FC.PwrMgmt = (pAd->StaCfg.Psm == PWR_SAVE);
        }
        pPort->NullFrame.Duration = pPort->CommonCfg.Dsifs + XmitCalcDuration(pAd, RateIdToMbps[READ_PHY_CFG_MCS(pAd, &PhyCfg)], 14);

        pAd->pTxCfg->Sequence       = ((pAd->pTxCfg->Sequence) + 1) & (MAX_SEQ_NUMBER);
        pPort->NullFrame.Sequence = pAd->pTxCfg->Sequence;
        COPY_MAC_ADDR(pPort->NullFrame.Addr1,   pDA);
        COPY_MAC_ADDR(pPort->NullFrame.Addr2, pAd->HwCfg.CurrentAddress);
        COPY_MAC_ADDR(pPort->NullFrame.Addr3, pPort->PortCfg.Bssid);

        PlatformZeroMemory(&pAd->pHifCfg->NullContext.TransferBuffer->WirelessPacket[0], 100);
        pTxInfo = (PTXINFO_STRUC)&pAd->pHifCfg->NullContext.TransferBuffer->WirelessPacket[0];
        pTxWI = (PTXWI_STRUC)&pAd->pHifCfg->NullContext.TransferBuffer->WirelessPacket[TXINFO_SIZE];
        Wcid = (UCHAR)MlmeSyncGetWlanIdxByPort(pPort, ROLE_WLANIDX_BSSID);
        if (bQosNull == TRUE)
        {
            XmitWriteTxInfo(pAd, pPort, pTxInfo, (USHORT)(sizeof(HEADER_802_11)+ 2 + pAd->HwCfg.TXWI_Length), TRUE, EpToQueue[MGMTPIPEIDX], FALSE,  FALSE);
#ifdef MULTI_CHANNEL_SUPPORT
#else
            WRITE_TXINFO_QSEL(pAd, pTxInfo, FIFO_EDCA);
#endif  /* MULTI_CHANNEL_SUPPORT */
            
            XmitWriteTxWI(pAd, pPort, pTxWI,  FALSE, FALSE, FALSE, TRUE, FALSE, 0, Wcid, (sizeof(HEADER_802_11)+2),
                0, 0, IFS_HTTXOP, FALSE , PhyCfg, FALSE, TRUE, FALSE);
            pAd->pHifCfg->NullContext.BulkOutSize =  TXINFO_SIZE + pAd->HwCfg.TXWI_Length + sizeof(pPort->NullFrame) + 8;              
            DBGPRINT(RT_DEBUG_ERROR, ("XmitSendNullFrameForDirect - send QoS NULL Frame @%d Mbps...\n", RateIdToMbps[READ_PHY_CFG_MCS(pAd, &PhyCfg)]));
        }
        else
        {
            XmitWriteTxInfo(pAd, pPort, pTxInfo, (USHORT)(sizeof(HEADER_802_11) + pAd->HwCfg.TXWI_Length), TRUE, EpToQueue[MGMTPIPEIDX], FALSE,  FALSE);
#ifdef MULTI_CHANNEL_SUPPORT
#else
            WRITE_TXINFO_QSEL(pAd, pTxInfo, FIFO_EDCA);
#endif  /* MULTI_CHANNEL_SUPPORT */
            XmitWriteTxWI(pAd, pPort, pTxWI,  FALSE, FALSE, FALSE, TRUE, FALSE, 0, Wcid, (sizeof(HEADER_802_11)),
                0, 0, IFS_HTTXOP, FALSE ,PhyCfg, FALSE, TRUE, FALSE);
            pAd->pHifCfg->NullContext.BulkOutSize =  TXINFO_SIZE + pAd->HwCfg.TXWI_Length + sizeof(pPort->NullFrame) + 4;
            DBGPRINT(RT_DEBUG_TRACE, ("XmitSendNullFrameForDirect - send NULL Frame @%d Mbps...\n", RateIdToMbps[pPort->CommonCfg.TxRate]));
        }
        // The last 2 bytes of QoS Null frame is zero. So don't need to movememory 2-bytes QoS fields .
        PlatformMoveMemory(&pAd->pHifCfg->NullContext.TransferBuffer->WirelessPacket[pAd->HwCfg.TXWI_Length+TXINFO_SIZE], &pPort->NullFrame, sizeof(HEADER_802_11));

        // Fill out frame length information for global Bulk out arbitor
        //pNullContext->BulkOutSize = TransferBufferLength;
        XmitSendMlmeCmdPkt(pAd, TXPKT_NULL_FRAME);
    }
}

VOID 
XmitSendNullFrame(
    IN  PMP_ADAPTER   pAd,
    IN  PMP_PORT      pPort,
    IN  UCHAR           TxRate,
    IN  BOOLEAN         bQosNull
    ) 
{
#if 0
    PTX_CONTEXT     pNullContext;
    //PTXD_STRUC        pTxD;

    PTXINFO_STRUC   pTxInfo;
    PTXWI_STRUC     pTxWI;
    UCHAR           PortSecured = pAd->StaCfg.PortSecured;
    UNREFERENCED_PARAMETER(TxRate); 


    // Use P2P's PortSecured in concurrent Client
    if (IS_P2P_CON_CLI(pAd, pPort) || IS_P2P_MS_CLI(pAd, pPort) || IS_P2P_MS_CLI2(pAd, pPort))
    {
        PortSecured = (UCHAR)pPort->MacTab.Content[P2pGetClientWcid(pAd, pPort)].PortSecured;
    }

    // not sending NULL frame if it is not idle at last second
    // When APSD is on, we should send null packet to inform AP that STA is in PS mode.
    //if (!pPort->CommonCfg.bAPSDCapable && pAd->pHifCfg->BulkOutDataOneSecCount)
    //  return;

    // STA is not allowed to transmit during silence period
    // Defer NULL frame tranmission until next available period
    if(P2P_ON(pPort) && pPort->P2PCfg.bKeepSlient == TRUE)
    {
        pPort->P2PCfg.bPendingNullFrame = TRUE;
        return;
    }
    
    if ((MT_TEST_FLAG(pAd, fRTMP_ADAPTER_RESET_IN_PROGRESS)) ||
        (MT_TEST_FLAG(pAd, fRTMP_ADAPTER_BULKOUT_RESET)) ||
        (MT_TEST_FLAG(pAd, fRTMP_ADAPTER_HALT_IN_PROGRESS)) ||
        (MT_TEST_FLAG(pAd, fRTMP_ADAPTER_BSS_SCAN_IN_PROGRESS)) ||        
        (MT_TEST_FLAG(pAd, fRTMP_ADAPTER_NIC_NOT_EXIST)))
    {
        return;
    }
    
    // WPA 802.1x secured port control
    if (((pPort->PortCfg.AuthMode == Ralink802_11AuthModeWPA) || 
        (pPort->PortCfg.AuthMode == Ralink802_11AuthModeWPAPSK) ||
        (pPort->PortCfg.AuthMode == Ralink802_11AuthModeWPA2) ||
        (pPort->PortCfg.AuthMode == Ralink802_11AuthModeWPA2PSK) ||
        (pPort->PortCfg.AuthMode == Ralink802_11AuthModeWAI_PSK) ||
        (pPort->PortCfg.AuthMode == Ralink802_11AuthModeWAI_CERT) ||        
        (CCKM_ON(pPort)) ||
        (pPort->StaCfg.WscControl.WscState != WSC_STATE_OFF)) &&
        (PortSecured == WPA_802_1X_PORT_NOT_SECURED))
    {
        return;
    }

    pNullContext = &(pAd->pHifCfg->NullContext);
    if (pNullContext->InUse == FALSE)
    {
        // Set the in use bit
        pNullContext->InUse = TRUE;
    
        PlatformZeroMemory(&pPort->NullFrame, sizeof(HEADER_802_11));
        
        pPort->NullFrame.FC.Type = BTYPE_DATA;
        pPort->NullFrame.FC.SubType = SUBTYPE_NULL_FUNC;
        if (bQosNull == TRUE)
            pPort->NullFrame.FC.SubType = SUBTYPE_QOS_NULL;
        pPort->NullFrame.FC.ToDs = 1;
        if (pPort->CommonCfg.bAPSDForcePowerSave)
        {
            pPort->NullFrame.FC.PwrMgmt = PWR_SAVE;
        }
        else
        {
            pPort->NullFrame.FC.PwrMgmt = (pAd->StaCfg.Psm == PWR_SAVE);
        }
        pPort->NullFrame.Duration = pPort->CommonCfg.Dsifs + XmitCalcDuration(pAd, RateIdToMbps[READ_PHY_CFG_MCS(pAd, &pPort->CommonCfg.MgmtPhyCfg)], 14);

        pAd->pTxCfg->Sequence       = ((pAd->pTxCfg->Sequence) + 1) & (MAX_SEQ_NUMBER);
        pPort->NullFrame.Sequence = pAd->pTxCfg->Sequence;
        COPY_MAC_ADDR(pPort->NullFrame.Addr1, pPort->PortCfg.Bssid);
        COPY_MAC_ADDR(pPort->NullFrame.Addr2, pPort->CurrentAddress);
        COPY_MAC_ADDR(pPort->NullFrame.Addr3, pPort->PortCfg.Bssid);

        PlatformZeroMemory(&pAd->pHifCfg->NullContext.TransferBuffer->WirelessPacket[0], 100);
        pTxInfo = (PTXINFO_STRUC)&pAd->pHifCfg->NullContext.TransferBuffer->WirelessPacket[0];
        pTxWI = (PTXWI_STRUC)&pAd->pHifCfg->NullContext.TransferBuffer->WirelessPacket[TXINFO_SIZE];

        if (bQosNull == TRUE)
        {
            XmitWriteTxInfo(pAd, pPort, pTxInfo, (USHORT)(sizeof(HEADER_802_11)+ 2 + pAd->HwCfg.TXWI_Length), TRUE, EpToQueue[MGMTPIPEIDX], FALSE,  FALSE);
#ifdef MULTI_CHANNEL_SUPPORT
#else
            WRITE_TXINFO_QSEL(pAd, pTxInfo, FIFO_EDCA);
#endif  /* MULTI_CHANNEL_SUPPORT */
            XmitWriteTxWI(pAd, pPort, pTxWI,  FALSE, FALSE, FALSE, TRUE, FALSE, 0, BSSID_WCID, (sizeof(HEADER_802_11)+2),
                0, 0, IFS_HTTXOP, FALSE ,pPort->CommonCfg.MgmtPhyCfg, FALSE, TRUE, FALSE);
            pAd->pHifCfg->NullContext.BulkOutSize =  TXINFO_SIZE + pAd->HwCfg.TXWI_Length + sizeof(pPort->NullFrame) + 8;
            DBGPRINT(RT_DEBUG_ERROR, ("%s - send QoS NULL Frame @%d Mbps...\n", __FUNCTION__, RateIdToMbps[READ_PHY_CFG_MCS(pAd, &pPort->CommonCfg.MgmtPhyCfg)]));
        }
        else
        {
            XmitWriteTxInfo(pAd, pPort, pTxInfo, (USHORT)(sizeof(HEADER_802_11)+pAd->HwCfg.TXWI_Length), TRUE, EpToQueue[MGMTPIPEIDX], FALSE,  FALSE);
#ifdef MULTI_CHANNEL_SUPPORT
#else
            WRITE_TXINFO_QSEL(pAd, pTxInfo, FIFO_EDCA);
#endif  /* MULTI_CHANNEL_SUPPORT */
            XmitWriteTxWI(pAd, pPort, pTxWI,  FALSE, FALSE, FALSE, TRUE, FALSE, 0, BSSID_WCID, (sizeof(HEADER_802_11)),
                0, 0, IFS_HTTXOP, FALSE ,pPort->CommonCfg.MgmtPhyCfg, FALSE, TRUE, FALSE);
            pAd->pHifCfg->NullContext.BulkOutSize =  TXINFO_SIZE + pAd->HwCfg.TXWI_Length + sizeof(pPort->NullFrame) + 4;
        }
        // The last 2 bytes of QoS Null frame is zero. So don't need to movememory 2-bytes QoS fields .
        PlatformMoveMemory(&pAd->pHifCfg->NullContext.TransferBuffer->WirelessPacket[pAd->HwCfg.TXWI_Length+TXINFO_SIZE], &pPort->NullFrame, sizeof(HEADER_802_11));

        // Fill out frame length information for global Bulk out arbitor
        //pNullContext->BulkOutSize = TransferBufferLength;
        DBGPRINT(RT_DEBUG_ERROR, ("s - send NULL Frame @%d Mbps...\n", __FUNCTION__, RateIdToMbps[TxRate]));
        MTUSB_SET_BULK_FLAG(pAd, fRTUSB_BULK_OUT_DATA_NULL);

        // Kick bulk out
        N6USBKickBulkOut(pAd);
    }
#else   
    PTX_CONTEXT             pNullContext;
    PTXDSCR_LONG_STRUC      pLongFormatTxD;
    PUCHAR                      ConcatedPoint;
    UCHAR                       PortSecured = pAd->StaCfg.PortSecured;
    UCHAR tempAddress[MAC_ADDR_LEN] = {0xff, 0xff, 0xff, 0xff, 0xff, 0xff};
    PMAC_TABLE_ENTRY pWcidMacTabEntry = NULL;

    UNREFERENCED_PARAMETER(TxRate); 


    // Use P2P's PortSecured in concurrent Client
    if (IS_P2P_CON_CLI(pAd, pPort) || IS_P2P_MS_CLI(pAd, pPort) || IS_P2P_MS_CLI2(pAd, pPort))
    {
        pWcidMacTabEntry = MlmeSyncMacTabMatchedWlanIdx(pPort, P2pGetClientWcid(pAd, pPort));   

        if(pWcidMacTabEntry != NULL)
            PortSecured = (UCHAR)pWcidMacTabEntry->PortSecured;
    }

    // not sending NULL frame if it is not idle at last second
    // When APSD is on, we should send null packet to inform AP that STA is in PS mode.
    //if (!pPort->CommonCfg.bAPSDCapable && pAd->pHifCfg->BulkOutDataOneSecCount)
    //  return;

    // STA is not allowed to transmit during silence period
    // Defer NULL frame tranmission until next available period
    if(P2P_ON(pPort) && pPort->P2PCfg.bKeepSlient == TRUE)
    {
        DBGPRINT(RT_DEBUG_ERROR, ("%s - send NULL Frame return 0\n", __FUNCTION__));
        pPort->P2PCfg.bPendingNullFrame = TRUE;
        return;
    }
    
    if ((MT_TEST_FLAG(pAd, fRTMP_ADAPTER_RESET_IN_PROGRESS)) ||
        (MT_TEST_FLAG(pAd, fRTMP_ADAPTER_BULKOUT_RESET)) ||
        (MT_TEST_FLAG(pAd, fRTMP_ADAPTER_HALT_IN_PROGRESS)) ||
        (MT_TEST_FLAG(pAd, fRTMP_ADAPTER_BSS_SCAN_IN_PROGRESS)) ||        
        (MT_TEST_FLAG(pAd, fRTMP_ADAPTER_NIC_NOT_EXIST)))
    {
        return;
    }

    // WPA 802.1x secured port control
    if (((pPort->PortCfg.AuthMode == Ralink802_11AuthModeWPA) || 
        (pPort->PortCfg.AuthMode == Ralink802_11AuthModeWPAPSK) ||
        (pPort->PortCfg.AuthMode == Ralink802_11AuthModeWPA2) ||
        (pPort->PortCfg.AuthMode == Ralink802_11AuthModeWPA2PSK) ||
        (pPort->StaCfg.WscControl.WscState != WSC_STATE_OFF)) &&
        ((PortSecured == WPA_802_1X_PORT_NOT_SECURED)))
    {
        return;
    }

    pNullContext = &pAd->pHifCfg->NullContext;

    if (pNullContext->InUse == FALSE)
    {
        DBGPRINT(RT_DEBUG_ERROR, ("%s - send NULL Frame start\n", __FUNCTION__));
        // Set the in use bit
        pNullContext->InUse = TRUE;
        pNullContext->BulkOutSize = 0;
    
        PlatformZeroMemory(&pPort->NullFrame, sizeof(HEADER_802_11));
        
        pPort->NullFrame.FC.Type = BTYPE_DATA;
        pPort->NullFrame.FC.SubType = SUBTYPE_NULL_FUNC;
        
        if (bQosNull == TRUE)
            pPort->NullFrame.FC.SubType = SUBTYPE_QOS_NULL;

        pPort->NullFrame.FC.ToDs = 1;

        if (pPort->CommonCfg.bAPSDForcePowerSave)
        {
            DBGPRINT(RT_DEBUG_TRACE, ("%s - send NULL Frame pPort->CommonCfg.bAPSDForcePowerSave = %d\n", __FUNCTION__, pPort->CommonCfg.bAPSDForcePowerSave));
            pPort->NullFrame.FC.PwrMgmt = PWR_SAVE;
        }
        else
        {
            DBGPRINT(RT_DEBUG_TRACE, ("%s - send NULL Frame (pAd->StaCfg.Psm == PWR_SAVE) = %d\n", __FUNCTION__, (pAd->StaCfg.Psm == PWR_SAVE)));
            pPort->NullFrame.FC.PwrMgmt = (pAd->StaCfg.Psm == PWR_SAVE);
        }
        
        pPort->NullFrame.Duration = pPort->CommonCfg.Dsifs + XmitCalcDuration(pAd, RateIdToMbps[TxRate] /*RateIdToMbps[READ_PHY_CFG_MCS(pAd, &pPort->CommonCfg.MgmtPhyCfg)]*/, 14);

        pAd->pTxCfg->Sequence       = ((pAd->pTxCfg->Sequence) + 1) & (MAX_SEQ_NUMBER);
        pPort->NullFrame.Sequence = pAd->pTxCfg->Sequence;

        COPY_MAC_ADDR(pPort->NullFrame.Addr1, pPort->PortCfg.Bssid);
        COPY_MAC_ADDR(pPort->NullFrame.Addr2, pPort->CurrentAddress);
        COPY_MAC_ADDR(pPort->NullFrame.Addr3, pPort->PortCfg.Bssid);

        PlatformZeroMemory(&pNullContext->TransferBuffer->WirelessPacket[0], 100);

        ConcatedPoint = &pNullContext->TransferBuffer->WirelessPacket[0];

        pLongFormatTxD = (PTXDSCR_LONG_STRUC)&ConcatedPoint[0];

        pLongFormatTxD->TxDscrDW0.DestinationQueueID = 0; //QueueId; if there are 2 WMMs, the QueueId need change
        pLongFormatTxD->TxDscrDW0.DestinationPortID = 0;
        pLongFormatTxD->TxDscrDW0.TxByteCount = sizeof(TXDSCR_LONG_STRUC) + sizeof(HEADER_802_11); // sizeof(HEADER_802_11) is null frame length

        if (bQosNull == TRUE)
        {
            pLongFormatTxD->TxDscrDW0.TxByteCount += 2;
        }

        pLongFormatTxD->TxDscrDW1.WLANIndex = 1;
        pLongFormatTxD->TxDscrDW1.HeaderLength = (sizeof(HEADER_802_11) / 2); // word base
        pLongFormatTxD->TxDscrDW1.HeaderFormat = 0x2;
        pLongFormatTxD->TxDscrDW1.TxDescriptionFormat = TXD_LONG_FORMAT;
        pLongFormatTxD->TxDscrDW1.NoAck = 0;
        pLongFormatTxD->TxDscrDW2.BMPacket = 0;
        pLongFormatTxD->TxDscrDW2.FixedRate = 1;
        pLongFormatTxD->TxDscrDW2.BADisable = 1;
        pLongFormatTxD->TxDscrDW3.RemainingTxCount = 0x1f;
        pLongFormatTxD->TxDscrDW5.BARSSNContorl = 1;
        pLongFormatTxD->TxDscrDW5.PowerManagement = 1;
        pLongFormatTxD->TxDscrDW6.RateToBeFixed = 0x4B;  // BW_20
        pLongFormatTxD->TxDscrDW6.FixedBandwidthMode = 0x100;  // BW_20

        //pLongFormatTxD->TxDscrDW6.AntennaPriority = 2;  // BW_20
        //pLongFormatTxD->TxDscrDW6.SpetialExtenstionEnable = 1;  // BW_20

        ConcatedPoint += sizeof(TXDSCR_LONG_STRUC);
        pNullContext->BulkOutSize += sizeof(TXDSCR_LONG_STRUC);

        PlatformMoveMemory(ConcatedPoint, &pPort->NullFrame, sizeof(HEADER_802_11));
        ConcatedPoint += sizeof(HEADER_802_11);
        pNullContext->BulkOutSize += sizeof(HEADER_802_11);

        if (bQosNull == TRUE)
            pNullContext->BulkOutSize += 2; // Qos bytes

        if (pNullContext->BulkOutSize % 4 == 1)
        {
            //pLongFormatTxD->TxDscrDW0.TxByteCount += 3;
            //ConcatedPoint += 3;
            pNullContext->BulkOutSize += 3;
        }
        else if (pNullContext->BulkOutSize % 4 == 2) 
        {
            //pLongFormatTxD->TxDscrDW0.TxByteCount += 2;
            //ConcatedPoint += 2;
            pNullContext->BulkOutSize += 2;
        }
        else if (pNullContext->BulkOutSize % 4 == 3) 
        {
            //pLongFormatTxD->TxDscrDW0.TxByteCount += 1;
            //ConcatedPoint += 1;
            pNullContext->BulkOutSize += 1;
        }

        pNullContext->BulkOutSize += 4;

        DBGPRINT(RT_DEBUG_TRACE, ("%s - send NULL Frame pNullContext->BulkOutSize = %d...\n", __FUNCTION__, pNullContext->BulkOutSize));

        // Fill out frame length information for global Bulk out arbitor
        //pNullContext->BulkOutSize = TransferBufferLength;
        DBGPRINT(RT_DEBUG_TRACE, ("%s - send NULL Frame @%d Mbps...\n", __FUNCTION__, RateIdToMbps[TxRate]));
        XmitSendMlmeCmdPkt(pAd, TXPKT_NULL_FRAME);
    }
#endif  
}


VOID 
XmitFirmwareFrame(
    IN  PMP_ADAPTER   pAd,
    IN  PMP_PORT      pPort,
    IN  BOOLEAN         bQosNull
    ) 
{
    PTX_CONTEXT     pNullContext;


    PTXINFO_STRUC   pTxInfo;
    PTXWI_STRUC     pTxWI;
    UCHAR           PortSecured = pAd->StaCfg.PortSecured;


    pNullContext = &(pAd->pHifCfg->NullContext);
    if (pNullContext->InUse == FALSE)
    {
        // Set the in use bit
        pNullContext->InUse = TRUE;
    
        PlatformZeroMemory(&pPort->NullFrame, sizeof(HEADER_802_11));       

        PlatformZeroMemory(&pAd->pHifCfg->NullContext.TransferBuffer->WirelessPacket[0], 100);
        pTxInfo = (PTXINFO_STRUC)&pAd->pHifCfg->NullContext.TransferBuffer->WirelessPacket[0];
        pTxWI = (PTXWI_STRUC)&pAd->pHifCfg->NullContext.TransferBuffer->WirelessPacket[TXINFO_SIZE];

        pAd->pHifCfg->NullContext.BulkOutSize =  TXINFO_SIZE + pAd->HwCfg.TXWI_Length + sizeof(pPort->NullFrame) + 4;

        // The last 2 bytes of QoS Null frame is zero. So don't need to movememory 2-bytes QoS fields .
        PlatformMoveMemory(&pAd->pHifCfg->NullContext.TransferBuffer->WirelessPacket[pAd->HwCfg.TXWI_Length+TXINFO_SIZE], &pPort->NullFrame, sizeof(HEADER_802_11));

        XmitSendMlmeCmdPkt(pAd, TXPKT_NULL_FRAME);
    }
}

#ifdef MULTI_CHANNEL_SUPPORT
/*
    ==========================================================================
    Description:
        Send out a NULL frame to AP. The prpose is to inform AP this client
        current PSM bit.
    NOTE:
        This routine should only be used in infrastructure mode.
    ==========================================================================
 */
VOID 
XmitSendNullFrameForChSwitch(
    IN  PMP_ADAPTER   pAd,
    IN  PMP_PORT      pPort,
    IN  UCHAR           TxRate,
    IN  BOOLEAN         bQosNull,
    IN  USHORT          PwrMgmt
    )
{
#if 0
    PTX_CONTEXT     pNullContext;
    //PTXD_STRUC        pTxD;

    PTXINFO_STRUC   pTxInfo;
    PTXWI_STRUC     pTxWI;
    UCHAR           PortSecured = pAd->StaCfg.PortSecured;
    UNREFERENCED_PARAMETER(TxRate);


    // Use P2P's PortSecured in concurrent Client
    if (IS_P2P_CON_CLI(pAd, pPort) || IS_P2P_MS_CLI(pAd, pPort) || IS_P2P_MS_CLI2(pAd, pPort))
    {
        PortSecured = (UCHAR)pPort->MacTab.Content[P2pGetClientWcid(pAd, pPort)].PortSecured;
    }

    // not sending NULL frame if it is not idle at last second
    // When APSD is on, we should send null packet to inform AP that STA is in PS mode.
    //if (!pPort->CommonCfg.bAPSDCapable && pAd->pHifCfg->BulkOutDataOneSecCount)
    //  return;

    // STA is not allowed to transmit during silence period
    // Defer NULL frame tranmission until next available period
    if(P2P_ON(pPort) && pPort->P2PCfg.bKeepSlient == TRUE)
    {
        pPort->P2PCfg.bPendingNullFrame = TRUE;
        return;
    }

    if ((MT_TEST_FLAG(pAd, fRTMP_ADAPTER_RESET_IN_PROGRESS)) ||
        (MT_TEST_FLAG(pAd, fRTMP_ADAPTER_BULKOUT_RESET)) ||
        (MT_TEST_FLAG(pAd, fRTMP_ADAPTER_HALT_IN_PROGRESS)) ||
        (MT_TEST_FLAG(pAd, fRTMP_ADAPTER_BSS_SCAN_IN_PROGRESS)) ||
        (MT_TEST_FLAG(pAd, fRTMP_ADAPTER_NIC_NOT_EXIST)))
    {
        return;
    }

    // WPA 802.1x secured port control
    if (((pPort->PortCfg.AuthMode == Ralink802_11AuthModeWPA) ||
        (pPort->PortCfg.AuthMode == Ralink802_11AuthModeWPAPSK) ||
        (pPort->PortCfg.AuthMode == Ralink802_11AuthModeWPA2) ||
        (pPort->PortCfg.AuthMode == Ralink802_11AuthModeWPA2PSK) ||
        (pPort->PortCfg.AuthMode == Ralink802_11AuthModeWAI_PSK) ||
        (pPort->PortCfg.AuthMode == Ralink802_11AuthModeWAI_CERT) ||
        (CCKM_ON(pPort)) ||
        (pPort->StaCfg.WscControl.WscState != WSC_STATE_OFF)) &&
        (PortSecured == WPA_802_1X_PORT_NOT_SECURED))
    {
        return;
    }

    pNullContext = &(pAd->pHifCfg->NullContext);
    if (pNullContext->InUse == FALSE)
    {
        // Set the in use bit
        pNullContext->InUse = TRUE;

        PlatformZeroMemory(&pPort->NullFrame, sizeof(HEADER_802_11));

        pPort->NullFrame.FC.Type = BTYPE_DATA;
        pPort->NullFrame.FC.SubType = SUBTYPE_NULL_FUNC;
        if (bQosNull == TRUE)
            pPort->NullFrame.FC.SubType = SUBTYPE_QOS_NULL;
        pPort->NullFrame.FC.ToDs = 1;

#if 0       
        if (pPort->CommonCfg.bAPSDForcePowerSave)
        {
            pPort->NullFrame.FC.PwrMgmt = PWR_SAVE;
        }
        else
        {
            pPort->NullFrame.FC.PwrMgmt = (pAd->StaCfg.Psm == PWR_SAVE);
        }
#else
        pPort->NullFrame.FC.PwrMgmt = PwrMgmt;
#endif

        //pPort->NullFrame.Duration = pPort->CommonCfg.Dsifs + XmitCalcDuration(pAd, RateIdToMbps[pPort->CommonCfg.MlmeTransmit.field.MCS], 14);
        pPort->NullFrame.Duration = pPort->CommonCfg.Dsifs + XmitCalcDuration(pAd, RateIdToMbps[READ_PHY_CFG_MCS(pAd, &pPort->CommonCfg.MgmtPhyCfg)], 14);

        pAd->pTxCfg->Sequence       = ((pAd->pTxCfg->Sequence) + 1) & (MAX_SEQ_NUMBER);
        pPort->NullFrame.Sequence = pAd->pTxCfg->Sequence;
        COPY_MAC_ADDR(pPort->NullFrame.Addr1, pPort->PortCfg.Bssid);
        COPY_MAC_ADDR(pPort->NullFrame.Addr2, pPort->CurrentAddress);
        COPY_MAC_ADDR(pPort->NullFrame.Addr3, pPort->PortCfg.Bssid);

        PlatformZeroMemory(&pAd->pHifCfg->NullContext.TransferBuffer->WirelessPacket[0], 100);
        pTxInfo = (PTXINFO_STRUC)&pAd->pHifCfg->NullContext.TransferBuffer->WirelessPacket[0];
        pTxWI = (PTXWI_STRUC)&pAd->pHifCfg->NullContext.TransferBuffer->WirelessPacket[TXINFO_SIZE];

        if (bQosNull == TRUE)
        {
            XmitWriteTxInfo(pAd, pPort, pTxInfo, (USHORT)(sizeof(HEADER_802_11)+ 2 + pAd->HwCfg.TXWI_Length), TRUE, EpToQueue[MGMTPIPEIDX], FALSE,  FALSE);

            //pTxInfo->QSEL = FIFO_EDCA;
            WRITE_TXINFO_QSEL(pAd, pTxInfo, FIFO_EDCA);

            XmitWriteTxWI(pAd, pPort, pTxWI,  FALSE, FALSE, FALSE, TRUE, FALSE, 0, BSSID_WCID, (sizeof(HEADER_802_11)+2),
                0, 0, IFS_HTTXOP, FALSE ,pPort->CommonCfg.MgmtPhyCfg, FALSE, TRUE);
            pAd->pHifCfg->NullContext.BulkOutSize =  TXINFO_SIZE + pAd->HwCfg.TXWI_Length + sizeof(pPort->NullFrame) + 8;
            DBGPRINT(RT_DEBUG_ERROR, ("%s - send QoS NULL Frame @%d Mbps...\n", __FUNCTION__, RateIdToMbps[READ_PHY_CFG_MCS(pAd, &pPort->CommonCfg.MgmtPhyCfg)]));
        }
        else
        {
            XmitWriteTxInfo(pAd, pPort, pTxInfo, (USHORT)(sizeof(HEADER_802_11)+pAd->HwCfg.TXWI_Length), TRUE, EpToQueue[MGMTPIPEIDX], FALSE,  FALSE);

            //pTxInfo->QSEL = FIFO_EDCA;
            WRITE_TXINFO_QSEL(pAd, pTxInfo, FIFO_EDCA);

            XmitWriteTxWI(pAd, pPort, pTxWI,  FALSE, FALSE, FALSE, TRUE, FALSE, 0, BSSID_WCID, (sizeof(HEADER_802_11)),
                0, 0, IFS_HTTXOP, FALSE ,pPort->CommonCfg.MlmeTransmit, FALSE, TRUE);
            pAd->pHifCfg->NullContext.BulkOutSize =  TXINFO_SIZE + pAd->HwCfg.TXWI_Length + sizeof(pPort->NullFrame) + 4;
        }
        // The last 2 bytes of QoS Null frame is zero. So don't need to movememory 2-bytes QoS fields .
        PlatformMoveMemory(&pAd->pHifCfg->NullContext.TransferBuffer->WirelessPacket[pAd->HwCfg.TXWI_Length+TXINFO_SIZE], &pPort->NullFrame, sizeof(HEADER_802_11));

        // Fill out frame length information for global Bulk out arbitor
        //pNullContext->BulkOutSize = TransferBufferLength;
        DBGPRINT(RT_DEBUG_ERROR, ("%s - send NULL Frame @%d Mbps...\n", __FUNCTION__, RateIdToMbps[TxRate]));
        MTUSB_SET_BULK_FLAG(pAd, fRTUSB_BULK_OUT_DATA_NULL);

        // Kick bulk out
        N6USBKickBulkOut(pAd);
    }
#else
    //UCHAR NullFrame[48];
    ULONG   Length;
    PHEADER_802_11  pHeader_802_11;
    UCHAR           PortSecured = pAd->StaCfg.PortSecured;
    UCHAR           QueIdx = 0;
    PUCHAR         pOutBuffer = NULL;
    NDIS_STATUS     NStatus;

    NStatus = MlmeAllocateMemory(pAd, &pOutBuffer);  //Get an unused nonpaged memory
    if(NStatus != NDIS_STATUS_SUCCESS) 
    {
        DBGPRINT(RT_DEBUG_TRACE,("%s -Allocate memory failed \n"));
        return;
    }

    //PlatformZeroMemory(NullFrame, 48);
    Length = sizeof(HEADER_802_11);

    pHeader_802_11 = (PHEADER_802_11) pOutBuffer;
    
    pHeader_802_11->FC.Type = BTYPE_DATA;
    pHeader_802_11->FC.SubType = SUBTYPE_NULL_FUNC;
    pHeader_802_11->FC.ToDs = 1;
    COPY_MAC_ADDR(pHeader_802_11->Addr1, pPort->PortCfg.Bssid);
    COPY_MAC_ADDR(pHeader_802_11->Addr2, pPort->CurrentAddress);
    COPY_MAC_ADDR(pHeader_802_11->Addr3, pPort->PortCfg.Bssid); 

    pHeader_802_11->FC.PwrMgmt = PwrMgmt;   

    pHeader_802_11->Duration = pPort->CommonCfg.Dsifs + XmitCalcDuration(pAd, RateIdToMbps[READ_PHY_CFG_MCS(pAd, &pPort->CommonCfg.MgmtPhyCfg)], 14);

    pHeader_802_11->Sequence = pAd->pTxCfg->Sequence;
    pAd->pTxCfg->Sequence++;
    pAd->pTxCfg->Sequence &= MAXSEQ;

    // Prepare QosNull function frame
    if (bQosNull)
    {
        pHeader_802_11->FC.SubType = SUBTYPE_QOS_NULL;
        
        // copy QOS control bytes
        pOutBuffer[Length]  =  0;
        pOutBuffer[Length+1] =  0;
        Length += 2;// if pad with 2 bytes for alignment, APSD will fail
    }

    pAd->MccCfg.TxOp = IFS_PIFS;
    // For multi-channel, NdisCommonMiniportMMRequest will use Add2 in NullFrame to find out the port and select correct H/W Q
    NdisCommonMiniportMMRequest(pAd, pOutBuffer, Length);
    pAd->MccCfg.TxOp = IFS_HTTXOP;
#endif
}
#endif /*MULTI_CHANNEL_SUPPORT*/


ULONG 
XmitMCSToPhyRateIn500kps(ULONG uMCS, ULONG uMode, ULONG bBW, ULONG bSGI)
{
    ULONG uReturnRate;
    
    ULONG FallBackRate[][16] = 
    {   
                        /*MCS0  MCS1    MCS2    MCS3    MCS4    MCS5    MCS6    MCS7    MCS8    MCS9    MCS10   MCS11   MCS12   MCS13   MCS14   MCS=15*/
        /*CCK*/         {2,     4,      11,     22,     0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0},
        /*OFDM*/        {12,    18,     24,     36,     48,     72,     96,     108,    0,      0,      0,      0,      0,      0,      0,      0},

        /*HT,BW0*/      {13,    26,     39,     52,     78,     104,    117,    130,    26,     52,     78,     104,    156,    208,    234,    260},
        /*HT,BW0,SGI*/  {14,    28,     42,     57,     85,     114,    128,    143,    28,     57,     85,     114,    171,    228,    257,    286},

        /*HT,BW1*/      {26,    52,     78,     104,    156,    208,    234,    260,    52,     104,    156,    208,    312,    416,    468,    520},
        /*HT,BW1,SGI*/  {28,    57,     85,     114,    171,    228,    257,    286,    57,     114,    171,    228,    343,    457,    514,    572},
    };

    if (uMode >= MODE_HTMIX)
        uReturnRate = FallBackRate[uMode + bBW*2 + bSGI][uMCS];
    else
        uReturnRate = FallBackRate[uMode][uMCS];
    
    return uReturnRate;
}

#define DEFAULTRATE RATE_24
UCHAR XmitMulticastRateNRateToOFDMRate(ULONG uMCS, ULONG bBW)
{
    UCHAR uReturnRate;
    UCHAR FallBackRate[][16] = {    /*MCS=0*/   /*MCS=1*/       /*MCS=2*/       /*MCS=3*/       /*MCS=4*/       /*MCS=5*/       /*MCS=6*/       /*MCS=7*/       /*MCS=8*/       /*MCS=9*/       /*MCS=10*/      /*MCS=11*/      /*MCS=12*/      /*MCS=13*/      /*MCS=14*/      /*MCS=15*/
                            /*BW20*/    {RATE_6,        RATE_6,         RATE_12,                DEFAULTRATE,    DEFAULTRATE,    DEFAULTRATE,    DEFAULTRATE,    DEFAULTRATE,    RATE_12,                DEFAULTRATE,    DEFAULTRATE,    DEFAULTRATE,DEFAULTRATE,DEFAULTRATE,DEFAULTRATE,DEFAULTRATE},
                            /*BW40*/    {RATE_6,    DEFAULTRATE,    DEFAULTRATE,        DEFAULTRATE,    DEFAULTRATE,    DEFAULTRATE,    DEFAULTRATE,    DEFAULTRATE,    DEFAULTRATE,        DEFAULTRATE,    DEFAULTRATE,    DEFAULTRATE,DEFAULTRATE,DEFAULTRATE,DEFAULTRATE,DEFAULTRATE} };
    uReturnRate = FallBackRate[bBW][uMCS];
    return uReturnRate;
}

UCHAR XmitMappingOneSpatialStreamRate(ULONG uMCS)
{
    UCHAR uReturnMCS;

    UCHAR MCSMapping[][25] = {  /*MCS=0*/   /*MCS=1*/   /*MCS=2*/   /*MCS=3*/   /*MCS=4*/   /*MCS=5*/   /*MCS=6*/   /*MCS=7*/   /*MCS=8*/   /*MCS=9*/   /*MCS=10*/  /*MCS=11*/  /*MCS=12*/  /*MCS=13*/  /*MCS=14*/  /*MCS=15*/  /*MCS=16*/  /*MCS=17*/  /*MCS=18*/  /*MCS=19*/  /*MCS=20*/  /*MCS=21*/  /*MCS=22*/  /*MCS=23*/
                            {   MCS_0,      MCS_1,      MCS_2,      MCS_3,      MCS_4,      MCS_5,      MCS_6,      MCS_7,      MCS_1,      MCS_3,      MCS_4,      MCS_5,      MCS_7,      MCS_7,      MCS_7,      MCS_7,      MCS_2,      MCS_4,      MCS_6,      MCS_7,      MCS_7,      MCS_7,      MCS_7,      MCS_7} };

    uReturnMCS = MCSMapping[0][uMCS];
    return uReturnMCS;
}

/*
    ========================================================================
    
    Routine Description:
        Calculates the duration which is required to transmit out frames 
    with given size and specified rate.
        
    Arguments:
        pTxD        Pointer to transmit descriptor
        Ack         Setting for Ack requirement bit
        Fragment    Setting for Fragment bit
        RetryMode   Setting for retry mode
        Ifs         Setting for IFS gap
        Rate        Setting for transmit rate
        Service     Setting for service
        Length      Frame length
        TxPreamble  Short or Long preamble when using CCK rates
        QueIdx - 0-3, according to 802.11e/d4.4 June/2003
        
    Return Value:
        None
        
    IRQL = PASSIVE_LEVEL
    IRQL = DISPATCH_LEVEL
    
    ========================================================================
*/
VOID 
XmitWriteTxWI(
    IN  PMP_ADAPTER   pAd,
    IN      PMP_PORT      pPort,
    IN  PTXWI_STRUC     pTxWI,
    IN    BOOLEAN           FRAG,   
    IN    BOOLEAN           InsTimestamp,
    IN  BOOLEAN         AMPDU,
    IN  BOOLEAN         Ack,
    IN  BOOLEAN         NSeq,       // HW new a sequence.
    IN  UCHAR           BASize,
    IN  UCHAR           WCID,
    IN  ULONG           Length,
    IN    UCHAR         PID,
    IN  UCHAR           MIMOps,
    IN  UCHAR           Txopmode,   
    IN  BOOLEAN         CfAck,  
    IN  PHY_CFG PacketPhyCfg, 
    IN BOOLEAN bNDPAnnouncement, 
    IN BOOLEAN bSkipTxBF,
    IN BOOLEAN IsDataFrame
    ) // Indicate this TxWI is use for Date frame or Mgmt frame
{

    PMAC_TABLE_ENTRY    pMac = NULL;
    UCHAR               btAMPDUSize = 0;
    UCHAR               btAMPDUInterval = 0;
    PMAC_TABLE_ENTRY pWcidMacTabEntry = NULL;
    pWcidMacTabEntry = MlmeSyncMacTabMatchedWlanIdx(pPort, WCID); 
    
    pMac = pWcidMacTabEntry;

    // ===============================================
    // Ui Tool Feature: Fixed Tx rate in MACVer2 later
    // ===============================================
    // Replace the phy cfg from "Auto rate switch" to "UI configuration" if "Fixed Tx rate" feature is enable
    // This feature is caused by OID "MTK_OID_N6_SET_FIXED_RATE" and only supported in MACVer2 later.
    if(EXT_MAC_CAPABLE(pAd) && pAd->UiCfg.UtfFixedTxRateEn && IsDataFrame)
    {
        WRITE_PHY_CFG_STBC(pAd, &PacketPhyCfg, READ_PHY_CFG_STBC(pAd, &pAd->UiCfg.UtfFixedTxRateCfg));
        WRITE_PHY_CFG_SHORT_GI(pAd, &PacketPhyCfg, READ_PHY_CFG_SHORT_GI(pAd, &pAd->UiCfg.UtfFixedTxRateCfg));
        WRITE_PHY_CFG_MCS(pAd, &PacketPhyCfg, READ_PHY_CFG_MCS(pAd, &pAd->UiCfg.UtfFixedTxRateCfg));
        WRITE_PHY_CFG_MODE(pAd, &PacketPhyCfg, READ_PHY_CFG_MODE(pAd, &pAd->UiCfg.UtfFixedTxRateCfg));
        WRITE_PHY_CFG_BW(pAd, &PacketPhyCfg, READ_PHY_CFG_BW(pAd, &pAd->UiCfg.UtfFixedTxRateCfg));
        WRITE_PHY_CFG_NSS(pAd, &PacketPhyCfg, READ_PHY_CFG_NSS(pAd, &pAd->UiCfg.UtfFixedTxRateCfg));    
    }

    //
    // Always use Long preamble before verifiation short preamble functionality works well.
    // Todo: remove the following line if short preamble functionality works
    //
    OPSTATUS_CLEAR_FLAG(pAd, fOP_STATUS_SHORT_PREAMBLE_INUSED);

    WRITE_TXWI_FRAG(pAd, pTxWI, FRAG);
    WRITE_TXWI_TS(pAd, pTxWI, InsTimestamp);
    WRITE_TXWI_AMPDU(pAd, pTxWI, AMPDU);
    
    if (pMac)
    {
        WRITE_TXWI_MIMO_PS(pAd, pTxWI, (pMac->PsMode == PWR_MMPS)? 1:0);
        WRITE_TXWI_MPDU_DENSITY(pAd, pTxWI, pMac->MpduDensity);
    }
    
    WRITE_TXWI_ACK(pAd, pTxWI, Ack);
    WRITE_TXWI_TXOP(pAd, pTxWI, Txopmode);
    WRITE_TXWI_NSEQ(pAd, pTxWI, NSeq);

    if (BASize > pPort->CommonCfg.BaLimit) 
        BASize = pPort->CommonCfg.BaLimit;
    if ((INFRA_ON(pPort)||(pAd->OpMode == OPMODE_AP)) && (pPort->CommonCfg.BaLimit == MAX_AGG_3SS_BALIMIT))
    {
        // IOT: 3x3 AP/STA may fix this issue like BRCM AP.
    }
    else
    {
        // If security on, still use old setting "BaLimit = 7".
        // It's use to avoid IOT issue. For VHT, don't use this patch (Comment by Rory)
        if ((BASize > 7) && (!NO_SECURITY_ON(pPort)) && (!VHT_CAPABLE(pPort)) )

            BASize = 7;
    }
    WRITE_TXWI_BA_WINDOW_SIZE(pAd, pTxWI, BASize);

    WRITE_TXWI_WCID(pAd, pTxWI, WCID);
    WRITE_TXWI_MPDU_TOTAL_BYTE_COUNT(pAd, pTxWI, Length);
    WRITE_TXWI_PACKET_ID(pAd, pTxWI, PID);
        
    WRITE_TXWI_BW(pAd, pTxWI, ((READ_PHY_CFG_MODE(pAd, &PacketPhyCfg) <= MODE_OFDM) ? (BW_20) : (READ_PHY_CFG_BW(pAd, &PacketPhyCfg))));
    WRITE_TXWI_SHORT_GI(pAd, pTxWI, READ_PHY_CFG_SHORT_GI(pAd, &PacketPhyCfg));
    WRITE_TXWI_STBC(pAd, pTxWI, READ_PHY_CFG_STBC(pAd, &PacketPhyCfg));
    
    WRITE_TXWI_PHY_MODE(pAd, pTxWI, READ_PHY_CFG_MODE(pAd, &PacketPhyCfg));
    WRITE_TXWI_MCS(pAd, pTxWI, READ_PHY_CFG_MODE(pAd, &PacketPhyCfg), READ_PHY_CFG_MCS(pAd, &PacketPhyCfg));
    WRITE_TXWI_NSS(pAd, pTxWI, READ_PHY_CFG_MODE(pAd, &PacketPhyCfg), READ_PHY_CFG_NSS(pAd, &PacketPhyCfg));

#if 0 //disable fast rate change, since USB has bulk delay. for further use.
    // =================================
    // HW feature: Faster rate change
    // =================================
    //   Turn on Faster rate change will lead MAC to select Tx Rate by register instead of the rate of TxWI
    //   This feature can speed up the rate change. (fource all frames of PBF can change Tx rate directlly)
    //   This machine supported by MACVersion 2
    if(VHT_NIC(pAd))
    {
        FAST_RATE_CHANGE_STRUC RateInfo = {0};
        RateInfo.IsDataFrame = IsDataFrame;
        RateInfo.WCID = WCID;

        // Only use this feature on data frame
        if((IsDataFrame == TRUE) && (WCID < 128))
        {
            //For the data frame, turn on faster rate change machine
    
            MAC_PHYRATE_STRUC MacPhyRate;

            PlatformZeroMemory(&MacPhyRate, sizeof(MacPhyRate));
        
            // ----------------------------------------
            // 1. Turn on Txwi.LutEn
            // ----------------------------------------         
            WRITE_TXWI_LUT_EN(pAd, pTxWI, TRUE);

            // ----------------------------------------
            // 2. Change Lookup table 
            // ----------------------------------------         
            MacPhyRate.PhyMode = READ_PHY_CFG_MODE(pAd, &PacketPhyCfg);
            MacPhyRate.STBC =    READ_PHY_CFG_STBC(pAd, &PacketPhyCfg);
            MacPhyRate.ShortGI = READ_PHY_CFG_SHORT_GI(pAd, &PacketPhyCfg);
            MacPhyRate.BW =      READ_PHY_CFG_BW(pAd, &PacketPhyCfg);
            if(READ_PHY_CFG_MODE(pAd, &PacketPhyCfg) == MODE_VHT)
                MacPhyRate.McsNss =  (READ_PHY_CFG_NSS(pAd, &PacketPhyCfg) << 4) | READ_PHY_CFG_MCS(pAd, &PacketPhyCfg);
            else
                MacPhyRate.McsNss =  (READ_PHY_CFG_NSS(pAd, &PacketPhyCfg) << 3) | READ_PHY_CFG_MCS(pAd, &PacketPhyCfg);                

            PlatformMoveMemory(&RateInfo.MacPhyRate, &MacPhyRate, sizeof(MacPhyRate));

            if(READ_PHY_CFG_MCS(pAd, &pPort->LastTxRatePhyCfg) != READ_PHY_CFG_MCS(pAd, &PacketPhyCfg))
                MTEnqueueInternalCmd(pAd, pPort, MT_CMD_FAST_RATE_CHANGE_REQUEST, &RateInfo, sizeof(RateInfo));
        }
        else
        {
            // ----------------------------------------
            // Turn off Txwi.LutEn
            // ----------------------------------------         
            WRITE_TXWI_LUT_EN(pAd, pTxWI, FALSE);
            MTEnqueueInternalCmd(pAd, pPort, MT_CMD_FAST_RATE_CHANGE_REQUEST, &RateInfo, sizeof(RateInfo));
        }
    }
#endif //endif fast rate change.
        
    if(READ_TXWI_STBC(pAd, pTxWI))
    {
        pPort->CommonCfg.TxSTBCCount ++;
    }

    // MMPS is 802.11n features. Because TxWI->MCS > 7 must be HT mode, so need not check if it's HT rate.
    if ((MIMOps == MMPS_STATIC) && (READ_TXWI_MCS(pAd, pTxWI, READ_PHY_CFG_MODE(pAd, &PacketPhyCfg)) > 7))    
    {   
        WRITE_TXWI_MCS(pAd, pTxWI, READ_PHY_CFG_MODE(pAd, &PacketPhyCfg), MCS_7);
    }
    
    if ((MIMOps == MMPS_DYNAMIC) && (READ_TXWI_MCS(pAd, pTxWI, READ_PHY_CFG_MODE(pAd, &PacketPhyCfg)) > MCS_7)) // SMPS protect 2 spatial.
    {
        WRITE_TXWI_MIMO_PS(pAd, pTxWI, 1);  
    }
    
    WRITE_TXWI_CFACK(pAd, pTxWI, CfAck);    

    // legacy should be BW_20
    if (READ_PHY_CFG_MODE(pAd, &PacketPhyCfg) < MODE_HTMIX)
    {
        WRITE_TXWI_BW(pAd, pTxWI, BW_20);   
    }

    if (READ_TXWI_MCS(pAd, pTxWI, READ_PHY_CFG_MODE(pAd, &PacketPhyCfg))> MCS_7)
    {
        WRITE_TXWI_STBC(pAd, pTxWI, STBC_NONE);
    }
    
    if (isEnableETxBf(pAd) && (bSkipTxBF == FALSE))
    {
        if (bNDPAnnouncement == TRUE) // Send a +HTC MPDU with the NDP Announcement set to 1
        {
            WRITE_TXWI_NDP_STREAM(pAd, pTxWI, 2);
            WRITE_TXWI_NDP_BW(pAd, pTxWI, READ_PHY_CFG_BW(pAd, &PacketPhyCfg));
            WRITE_TXWI_SOUNDING(pAd, pTxWI, 0);

            WRITE_TXWI_ITXBF(pAd, pTxWI, 0);
            WRITE_TXWI_EXTBF(pAd, pTxWI, 0);
            WRITE_TXWI_STBC(pAd, pTxWI, 0);
            
            WRITE_TXWI_PHY_MODE(pAd, pTxWI, READ_PHY_CFG_MODE(pAd, &PacketPhyCfg));
            WRITE_TXWI_MCS(pAd, pTxWI, READ_PHY_CFG_MODE(pAd, &PacketPhyCfg), READ_PHY_CFG_MCS(pAd, &PacketPhyCfg));
            WRITE_TXWI_AMPDU(pAd, pTxWI, 0);

            WRITE_TXWI_WCID(pAd, pTxWI, RESERVED_WCID);         
        
            /*
            if (P2P_OFF(pPort))
            DBGPRINT(RT_DEBUG_TRACE, ("TxBf: %s: (bNDPAnnouncement), pTxWI->NDPS = %d, pTxWI->NDPBW = %d, pTxWI->ACK = %d, pTxWI->PHYMODE = %d, pTxWI->MCS = %d", 
                __FUNCTION__, 
                pTxWI->NDPS, 
                pTxWI->NDPBW, 
                pTxWI->ACK, 
                pTxWI->PHYMODE, 
                pTxWI->MCS));
            */
        }

        if ((READ_PHY_CFG_ETX_BF_PACKET(pAd, &PacketPhyCfg) == 1) && (bNDPAnnouncement == FALSE))
        {
            WRITE_TXWI_EXTBF(pAd, pTxWI, 1); // Apply the beamforming matrix of the explicit TxBf
            WRITE_TXWI_STBC(pAd, pTxWI, 0); // NIC cannot perform TxBf and STBC at the same time.

            if (P2P_OFF(pPort))
                DBGPRINT(RT_DEBUG_TRACE, ("TxBf: %s: pTxWI->ETxBf = 1", __FUNCTION__));
        }
    }
        
    if (bSkipTxBF == TRUE) // Beacon cannot enable the TxBf
    {
        WRITE_TXWI_ITXBF(pAd, pTxWI, 0);
        WRITE_TXWI_EXTBF(pAd, pTxWI, 0);
        WRITE_TXWI_SOUNDING(pAd, pTxWI, 0);
        
        if (P2P_OFF(pPort))
            DBGPRINT(RT_DEBUG_TRACE, ("TxBf: %s: pTxWI->ETxBf = 0", __FUNCTION__));
    }

    if (EXT_MAC_CAPABLE(pAd))
    {
        DBGPRINT(RT_DEBUG_TRACE, ("%s - FRAG(%d), MpduDensity(%d), TS(%d), AMPDU(%d), ACK(%d), txop(%d), NSEQ(%d), BAWinSize(%d), WirelessCliID(%d), MPDUtotalByteCount(%d), PacketId(%d), BW(%d), ShortGI(%d), STBC(%d), MCS(%d), PHYMODE(%d)\n",
                                __FUNCTION__,
                                pTxWI->Ext.Frag,pTxWI->Ext.MpduDensity, pTxWI->Ext.TS, pTxWI->Ext.AMPDU, pTxWI->Ext.CfAck, pTxWI->Ext.TXOP, pTxWI->Ext.NSEQ, pTxWI->Ext.BaWindowSize,
                                pTxWI->Ext.WCID, pTxWI->Ext.MpduTotalByteCount, pTxWI->Ext.PacketId, pTxWI->Ext.BW, pTxWI->Ext.ShortGI, pTxWI->Ext.STBC, pTxWI->Ext.McsNss, pTxWI->Ext.PhyMode));
    }
    else
    {
        DBGPRINT(RT_DEBUG_TRACE, ("%s - FRAG(%d), MpduDensity(%d), TS(%d), AMPDU(%d), ACK(%d), txop(%d), NSEQ(%d), BAWinSize(%d), WirelessCliID(%d), MPDUtotalByteCount(%d), PacketId(%d), BW(%d), ShortGI(%d), STBC(%d), MCS(%d), PHYMODE(%d)\n",
                                __FUNCTION__,
                                pTxWI->Default.Frag,pTxWI->Default.MpduDensity, pTxWI->Default.TS, pTxWI->Default.AMPDU, pTxWI->Default.CfAck, pTxWI->Default.TXOP, pTxWI->Default.NSEQ, pTxWI->Default.BaWindowSize,
                                pTxWI->Default.WCID, pTxWI->Default.MpduTotalByteCount, pTxWI->Default.PacketId, pTxWI->Default.BW, pTxWI->Default.ShortGI, pTxWI->Default.STBC, pTxWI->Default.MCS, pTxWI->Default.PhyMode));
    }
    
}

VOID 
XmitWriteTxInfo(
    IN  PMP_ADAPTER   pAd,
    IN      PMP_PORT      pPort,
    IN  PTXINFO_STRUC   pTxInfo,
    IN    USHORT        USBDMApktLen,
    IN    BOOLEAN       bWiv,
    IN    UCHAR         QueueSel,
    IN    UCHAR         NextValid,
    IN    UCHAR         TxBurst
    )
{
    UNREFERENCED_PARAMETER(pAd);

    WRITE_TXINFO_USBDMATxPktLen(pAd, pTxInfo, USBDMApktLen);

#ifdef MULTI_CHANNEL_SUPPORT
#if 0
    // Management 
    if (QueueSel == FIFO_MGMT)
    {
        pTxInfo->QSEL = QueueSel;
    }
    // Data
    else
#endif      
    {
        if (pAd->MccCfg.MultiChannelEnable == TRUE)
        {
            if (pPort->PortType == EXTSTA_PORT)
            {
                WRITE_TXINFO_QSEL(pAd, pTxInfo, pPort->P2PCfg.STAHwQSEL);
                //DBGPRINT(RT_DEBUG_TRACE,("%s - QSEL(%d)\n", __FUNCTION__, pTxInfo->QSEL));
            }
            else if (pPort->PortType == WFD_CLIENT_PORT)
            {
                WRITE_TXINFO_QSEL(pAd, pTxInfo, pPort->P2PCfg.CLIHwQSEL);
            }
            else if (pPort->PortType == WFD_GO_PORT)
            {
                    WRITE_TXINFO_QSEL(pAd, pTxInfo, pPort->P2PCfg.GOHwQSEL);
            }       
            else
            {
                WRITE_TXINFO_QSEL(pAd, pTxInfo, FIFO_EDCA);
            }
        }
        else if (MT_TEST_BIT(pAd->pP2pCtrll->MultiChannelStartOrEndProcess, MULTI_CHANNEL_IN_NEW_SESSION))
        {
            if ((pPort->PortType == EXTSTA_PORT) && INFRA_OP(pAd->PortList[PORT_0]))
            {
            WRITE_TXINFO_QSEL(pAd, pTxInfo, pPort->P2PCfg.STAHwQSEL);
                }
            else if ((pPort->PortType == WFD_CLIENT_PORT) && MT_TEST_BIT(pPort->P2PCfg.P2pMsConnectedStatus, P2pMs_CONNECTED_STATUS_CLI_OP))    
            {
            WRITE_TXINFO_QSEL(pAd, pTxInfo, pPort->P2PCfg.CLIHwQSEL);
            }
            else if ((pPort->PortType == WFD_GO_PORT) && MT_TEST_BIT(pPort->P2PCfg.P2pMsConnectedStatus, P2pMs_CONNECTED_STATUS_GO_ON)) 
            {
                WRITE_TXINFO_QSEL(pAd, pTxInfo, pPort->P2PCfg.GOHwQSEL);
            }   
            else
            {
                WRITE_TXINFO_QSEL(pAd, pTxInfo, FIFO_EDCA);
            }
        }
        else
        {
            WRITE_TXINFO_QSEL(pAd, pTxInfo, FIFO_EDCA);
        }   
    }
#else /*MULTI_CHANNEL_SUPPORT*/
    WRITE_TXINFO_QSEL(pAd, pTxInfo, QueueSel);

    if (QueueSel != FIFO_EDCA)
        DBGPRINT(RT_DEBUG_TRACE, ("====> QueueSel != FIFO_EDCA<============\n"));
#endif
    
    WRITE_TXINFO_USBDMANextVLD(pAd, pTxInfo, NextValid);
    WRITE_TXINFO_USBDMATxburst(pAd, pTxInfo, TxBurst);
    WRITE_TXINFO_WIV(pAd, pTxInfo, bWiv);

    WRITE_TXINFO_SwUseSegmentEnd(pAd, pTxInfo, 0);
    WRITE_TXINFO_SwUseSegIdx(pAd, pTxInfo, 0);
}   


VOID 
XmitWriteTxD(
    IN  PMP_ADAPTER           pAd,
    IN      PMP_PORT              pPort,
    IN  PTXDSCR_SHORT_STRUC pShortFormatTxD,
    IN  PTXDSCR_LONG_STRUC  pLongFormatTxD,
    IN  HARD_TRANSMIT_INFO      TxInfo
    )
{
    PTXDSCR_DW0     pTxDscrDW0 = NULL;
    PTXDSCR_DW1     pTxDscrDW1 = NULL;
    PTXDSCR_DW2     pTxDscrDW2 = NULL;
    PTXDSCR_DW3     pTxDscrDW3 = NULL;
    PTXDSCR_DW4     pTxDscrDW4 = NULL;
    PTXDSCR_DW5     pTxDscrDW5 = NULL;
    PTXDSCR_DW6     pTxDscrDW6 = NULL;
    PTXDSCR_DW7     pTxDscrDW7 = NULL;

    if ((pShortFormatTxD != NULL && pLongFormatTxD != NULL) || (pShortFormatTxD == NULL && pLongFormatTxD == NULL))
    {
        DBGPRINT(RT_DEBUG_INFO, ("====> XmitWriteTxD : Programming wrong. Short and Long can not exist at the same time Or can't be NULL at the same time \n"));
    }
    else if (pShortFormatTxD != NULL)
    {
        DBGPRINT(RT_DEBUG_INFO, ("====> XmitWriteTxD :  use Short Format TxD \n"));
        pTxDscrDW0 = &pShortFormatTxD->TxDscrDW0;
        pTxDscrDW1 = &pShortFormatTxD->TxDscrDW1;
        pTxDscrDW7 = &pShortFormatTxD->TxDscrDW7;

        pTxDscrDW1->TxDescriptionFormat = TXD_SHORT_FORMAT;

        pTxDscrDW0->TxByteCount = TxInfo.TotalPacketLength + TXD_SHORT_SIZE;
        DBGPRINT(RT_DEBUG_INFO, ("====> XmitWriteTxD :  TxInfo.TotalPacketLength + TXD_SHORT_SIZE = %d(%d ?= %d) \n", TxInfo.TotalPacketLength + TXD_SHORT_SIZE, sizeof(TXDSCR_LONG_STRUC), TXD_SHORT_SIZE));
    }
    else
    {
        DBGPRINT(RT_DEBUG_INFO, ("====> XmitWriteTxD :  use Long Format TxD \n"));
        pTxDscrDW0 = &pLongFormatTxD->TxDscrDW0;
        pTxDscrDW1 = &pLongFormatTxD->TxDscrDW1;
        pTxDscrDW2 = &pLongFormatTxD->TxDscrDW2;
        pTxDscrDW3 = &pLongFormatTxD->TxDscrDW3;
        pTxDscrDW4 = &pLongFormatTxD->TxDscrDW4;
        pTxDscrDW5 = &pLongFormatTxD->TxDscrDW5;
        pTxDscrDW6 = &pLongFormatTxD->TxDscrDW6;
        pTxDscrDW7 = &pLongFormatTxD->TxDscrDW7;

        pTxDscrDW1->TxDescriptionFormat = TXD_LONG_FORMAT;

        pTxDscrDW0->TxByteCount = TxInfo.TotalPacketLength + TXD_LONG_SIZE;
        DBGPRINT(RT_DEBUG_INFO, ("====> XmitWriteTxD :  TxInfo.TotalPacketLength + TXD_SHORT_SIZE = %d(%d ?= %d) \n", TxInfo.TotalPacketLength + TXD_SHORT_SIZE, sizeof(TXDSCR_LONG_STRUC), TXD_SHORT_SIZE));
    }
#if 0
                pLongFormatTxD->TxDscrDW0.DestinationQueueID = QueueId;
                pLongFormatTxD->TxDscrDW0.DestinationPortID = 0;
                pLongFormatTxD->TxDscrDW0.TxByteCount = sizeof(TXDSCR_LONG_STRUC) + sizeof(NullFrame) + PayloadLength;
                pLongFormatTxD->TxDscrDW1.WLANIndex = 0;
                pLongFormatTxD->TxDscrDW1.HeaderLength = (sizeof(HEADER_802_11) / 2); // word base
                pLongFormatTxD->TxDscrDW1.HeaderFormat = 0x2;
                pLongFormatTxD->TxDscrDW1.TxDescriptionFormat = TXD_LONG_FORMAT;
                pLongFormatTxD->TxDscrDW1.NoAck = 1;
                pLongFormatTxD->TxDscrDW2.BMPacket = 1;
                pLongFormatTxD->TxDscrDW2.FixedRate = 1;
                pLongFormatTxD->TxDscrDW2.BADisable = 1;
                pLongFormatTxD->TxDscrDW3.RemainingTxCount = 0x1f;
                pLongFormatTxD->TxDscrDW5.BARSSNContorl = 1;
                pLongFormatTxD->TxDscrDW6.RateToBeFixed = 0;
                pLongFormatTxD->TxDscrDW6.FixedBandwidthMode = 0x100;  // BW_20
#endif

#if  1
    //WRITE_TXINFO_USBDMATxPktLen(pAd, pTxInfo, USBDMApktLen);
    
    pTxDscrDW1->HeaderLength = (TxInfo.This80211HdrLen / 2); // word base
    pTxDscrDW1->HeaderFormat = 0x2;

#ifdef MULTI_CHANNEL_SUPPORT
#if 0
    // Management 
    if (TxInfo.QueueSel == FIFO_MGMT)
    {
        //pTxInfo->QSEL = TxInfo.QueueSel;
    }
    // Data
    else
#endif      
    {
        if (pAd->MccCfg.MultiChannelEnable == TRUE)
        {
            if (pPort->PortType == EXTSTA_PORT)
            {
                //WRITE_TXINFO_QSEL(pAd, pTxInfo, pPort->P2PCfg.STAHwQSEL);
                pTxDscrDW0->DestinationQueueID = pPort->P2PCfg.STAHwQSEL; // need to change driver Queue selection????????
                //DBGPRINT(RT_DEBUG_TRACE,("%s - QSEL(%d)\n", __FUNCTION__, pTxInfo->QSEL));
            }
            else if (pPort->PortType == WFD_CLIENT_PORT)
            {
                //WRITE_TXINFO_QSEL(pAd, pTxInfo, pPort->P2PCfg.CLIHwQSEL);
                pTxDscrDW0->DestinationQueueID = pPort->P2PCfg.CLIHwQSEL; // need to change driver Queue selection????????
            }
            else if (pPort->PortType == WFD_GO_PORT)
            {
                //WRITE_TXINFO_QSEL(pAd, pTxInfo, pPort->P2PCfg.GOHwQSEL);
                pTxDscrDW0->DestinationQueueID = pPort->P2PCfg.GOHwQSEL; // need to change driver Queue selection????????
            }       
            else
            {
                //WRITE_TXINFO_QSEL(pAd, pTxInfo, FIFO_EDCA);
                pTxDscrDW0->DestinationQueueID = FIFO_EDCA; // need to change driver Queue selection????????
            }
        }
        else if (MT_TEST_BIT(pAd->pP2pCtrll->MultiChannelStartOrEndProcess, MULTI_CHANNEL_IN_NEW_SESSION))
        {
            if ((pPort->PortType == EXTSTA_PORT) && INFRA_OP(pAd->PortList[PORT_0]))
            {
                //WRITE_TXINFO_QSEL(pAd, pTxInfo, pPort->P2PCfg.STAHwQSEL);
                pTxDscrDW0->DestinationQueueID = pPort->P2PCfg.STAHwQSEL; // need to change driver Queue selection????????
            }
            else if ((pPort->PortType == WFD_CLIENT_PORT) && MT_TEST_BIT(pPort->P2PCfg.P2pMsConnectedStatus, P2pMs_CONNECTED_STATUS_CLI_OP))    
            {
                //WRITE_TXINFO_QSEL(pAd, pTxInfo, pPort->P2PCfg.CLIHwQSEL);
                pTxDscrDW0->DestinationQueueID = pPort->P2PCfg.CLIHwQSEL; // need to change driver Queue selection????????
            }
            else if ((pPort->PortType == WFD_GO_PORT) && MT_TEST_BIT(pPort->P2PCfg.P2pMsConnectedStatus, P2pMs_CONNECTED_STATUS_GO_ON)) 
            {
                //WRITE_TXINFO_QSEL(pAd, pTxInfo, pPort->P2PCfg.GOHwQSEL);
                pTxDscrDW0->DestinationQueueID = pPort->P2PCfg.GOHwQSEL; // need to change driver Queue selection????????
            }   
            else
            {
                //WRITE_TXINFO_QSEL(pAd, pTxInfo, FIFO_EDCA);
                pTxDscrDW0->DestinationQueueID = FIFO_EDCA; // need to change driver Queue selection????????
            }
        }
        else
        {
            //WRITE_TXINFO_QSEL(pAd, pTxInfo, FIFO_EDCA);
            pTxDscrDW0->DestinationQueueID = FIFO_EDCA; // need to change driver Queue selection????????
        }   

        pTxDscrDW0->DestinationQueueID = TxInfo.QueIdx;
        pTxDscrDW0->DestinationPortID = TXD_LMAC_PORT;
    }
#else /*MULTI_CHANNEL_SUPPORT*/
    //WRITE_TXINFO_QSEL(pAd, pTxInfo, QueueSel);
    pTxDscrDW0->DestinationQueueID = TxInfo.QueIdx; // need to change driver Queue selection????????
    pTxDscrDW0->DestinationPortID = TXD_LMAC_PORT;

        DBGPRINT(RT_DEBUG_INFO, ("====> QueueSel != FIFO_EDCA<============ TxInfo.QueueSel  =%d\n", TxInfo.QueueSel));
#endif
    
    //WRITE_TXINFO_USBDMANextVLD(pAd, pTxInfo, NextValid);
    pTxDscrDW0->USBNextValid = TxInfo.NextValid;
    //WRITE_TXINFO_USBDMATxburst(pAd, pTxInfo, TxBurst);
    pTxDscrDW0->USBTxBurst = TxInfo.bTxBurst;

    DBGPRINT(RT_DEBUG_INFO, ("2 ====> XmitWriteTxD :  PID = %d, QID = %d, CSO = %d, burst = %d, valid = %d \n", pTxDscrDW0->DestinationPortID, pTxDscrDW0->DestinationQueueID, pTxDscrDW0->UDPTCPChecksumOffload, pTxDscrDW0->USBTxBurst, pTxDscrDW0->USBNextValid));

    //WRITE_TXINFO_WIV(pAd, pTxInfo, bWiv);


    //
    // Do not set WIV bit to 1 when WAPI mode is turned on.
    //
    //if (IS_HW_WAPI(pAd) && 
    //    (pPort->PortCfg.WepStatus == Ndis802_11EncryptionWPI_SMS4))
    //{
    //  WRITE_TXINFO_WIV(pAd, pTxInfo, 0);
    //}
        
    //WRITE_TXINFO_SwUseSegmentEnd(pAd, pTxInfo, 0);
    //WRITE_TXINFO_SwUseSegIdx(pAd, pTxInfo, 0);

    

    //pTxDscrDW1->MoreData = TxInfo.MoreData;
    pTxDscrDW1->WLANIndex = TxInfo.Wcid;
    pTxDscrDW1->NoAck = (TxInfo.bAckRequired ? 0 : 1);
    pTxDscrDW1->TID = TxInfo.UserPriority;

    pTxDscrDW1->OwnMACAddress = pPort->OwnMACAddressIdx;

    DBGPRINT(RT_DEBUG_TRACE, ("TxInfo.Wcid = %d\n", TxInfo.Wcid));
    DBGPRINT(RT_DEBUG_TRACE, ("TxInfo.bAckRequired = %d\n", TxInfo.bAckRequired));
    DBGPRINT(RT_DEBUG_TRACE, ("TxInfo.bWMM = %d\n", TxInfo.bWMM));
    DBGPRINT(RT_DEBUG_TRACE, ("TxInfo.UserPriority = %d\n", TxInfo.UserPriority));
    DBGPRINT(RT_DEBUG_TRACE, ("pTxDscrDW1->OwnMACAddress = %d\n", pTxDscrDW1->OwnMACAddress));

    //if(TxInfo.Wcid == BSSID_WCID)
        pTxDscrDW1->ProtectedFrameField = (TxInfo.CipherAlg ? 1 : 0);

    pTxDscrDW1->HeaderPadding = (TMI_HDR_PAD_MODE_TAIL << TMI_HDR_PAD_BIT_MODE) | TxInfo.LengthQosPAD; //0;//TxInfo.LengthQosPAD;

    if (pLongFormatTxD != NULL)
    {
        pTxDscrDW2->Fragment = TxInfo.FragmentByteForTxD;
        pTxDscrDW2->BADisable = 1; //TxInfo.bTXBA;
        pTxDscrDW2->MaxTxTime = 0;
        pTxDscrDW2->BMPacket = (TxInfo.Wcid == MlmeSyncGetWlanIdxByPort(pPort, ROLE_WLANIDX_MBCAST) ? 1 : 0);
        pTxDscrDW2->FixedRate = 1;   // implement later.....maybe
        if (TxInfo.QueIdx == 0x7)
        {
            pTxDscrDW3->RemainingTxCount = 0x1f; // retry to death!
        }
        else
        {
            pTxDscrDW3->RemainingTxCount = 0xf; // retry to death!
        }

        if (TxInfo.bBARPacket == TRUE)
        {
            pTxDscrDW3->SNValid = 1;
            pTxDscrDW3->SequnceNumber = TxInfo.BARStartSequence;
        }

        //pTxDscrDW2->PowerOffset = 0x08;

        pTxDscrDW5->TxStatusHost= 0; // 1 : report status to host

        if (TxInfo.QueIdx == 0x7)
        {
            DBGPRINT(RT_DEBUG_TRACE, ("2 TxInfo.QueIdx = %d, TxInfo.PortNumber = %d(%d)\n", TxInfo.QueIdx, TxInfo.PortNumber, NDIS_DEFAULT_PORT_NUMBER));
            pTxDscrDW5->TxStatusHost = 1;
            pTxDscrDW5->PacketID = PID_BEACON | TxInfo.PortNumber;
        }

//      if (pTxDscrDW5->TxStatusHost == 1)
//      {
//          pTxDscrDW5->PacketID = PID_DATA_NORMALUCAST;
//      }

        pTxDscrDW5->BARSSNContorl = TMI_BSN_CFG_BY_SW;  // HW designer said "It's no function"
        pTxDscrDW5->PowerManagement = TMI_PM_BIT_CFG_BY_HW;


        /* DWORD 6 */
        if (pTxDscrDW2->FixedRate == 1) {
            pTxDscrDW6->FixedRateMode = TMI_FIX_RATE_BY_TXD;
            //txd_6->ant_id = 0;
            //txd_6->ant_pri = 0;
            //txd_6->spe_en = 0;
            pTxDscrDW6->FixedBandwidthMode = 0x100;  // BW_20
            pTxDscrDW6->DynamicBandwidth = 0;
            pTxDscrDW6->ExplicitBeamforming = 0;
            pTxDscrDW6->ImplicitBeamforming = 0;
            //pTxDscrDW6->RateToBeFixed = 0x4B;
            //pTxDscrDW6->AntennaPriority = 2;  // BW_20
            //pTxDscrDW6->SpetialExtenstionEnable = 1;  // BW_20

//          pTxDscrDW6->LDPC = ldpc;
//          pTxDscrDW6->GI = sgi;

//          if (pTxDscrDW6->FixedRateMode == TMI_FIX_RATE_BY_TXD)
//              pTxDscrDW6->RateToBeFixed = tx_rate_to_tmi_rate(phy_mode, mcs, nss, stbc, preamble);
        }
    }

#endif  
}   


// should be called only when -
// 1. MEADIA_CONNECTED
// 2. AGGREGATION_IN_USED
// 3. Fragmentation not in used
// 4. either no previous frame (pPrevAddr1=NULL) .OR. previoud frame is aggregatible
BOOLEAN 
XmitFrameIsAggregatible(
    IN  PMP_ADAPTER   pAd,
    IN  PMP_PORT      pPort,
    IN  PUCHAR          pPrevAddr1,
    IN  PUCHAR          p80211hdr
    )
{
    //return FALSE;
    // can't aggregate EAPOL (802.1x) frame
    if ((p80211hdr[LENGTH_802_11 + 7] == 0x88) && (p80211hdr[LENGTH_802_11 + 8] == 0x8e))
        return FALSE;

    // can't aggregate multicast/broadcast frame
    if (p80211hdr[4] & 0x01)
        return FALSE;

    if (INFRA_ON(pPort)) // must be unicast to AP
        return TRUE;
    else if ((pPrevAddr1 == NULL) || MAC_ADDR_EQUAL(pPrevAddr1, p80211hdr + 4)) // unicast to same STA
    {
        return TRUE;
    }
    else
    {
        DBGPRINT(RT_DEBUG_TRACE, ("not unicast to same STA\n"));    
        if (pPrevAddr1!=NULL)
        {
            DumpFrameMessage(pPrevAddr1, 12, ("pPrevAddr1"));
            DumpFrameMessage(p80211hdr, 12, ("p8011hdr"));          
        }   
        return FALSE;
    }
}
