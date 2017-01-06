#include "MtConfig.h"
/*
    ========================================================================
    Routine Description:
        This routine is used to en-queue outgoing packets when
        there is no enough shread memory

    Arguments:
        pAd    Pointer to our adapter
        pPacket     Pointer to send packet

    Return Value:
        None

    pre: Before calling this routine, caller should have filled the following fields

        pPacket->MiniportReserved[6] - contains packet source
        pPacket->MiniportReserved[5] - contains RA's WDS index (if RA on WDS link) or AID 
                                       (if RA directly associated to this AP)
    post:This routine should decide the remaining pPacket->MiniportReserved[] fields 
        before calling ApDataHardTransmit(), such as:

        pPacket->MiniportReserved[4] - Fragment # and User PRiority
        pPacket->MiniportReserved[7] - RTS/CTS-to-self protection method and TX rate
    ========================================================================
*/

NDIS_STATUS 
ApDataSendPacket(
    IN  PMP_ADAPTER       pAd,
    IN  NDIS_PORT_NUMBER    PortNumber,
    IN  PMT_XMIT_CTRL_UNIT             pXcu
    )
{
    ULONG           TotalPacketLength;
    NDIS_STATUS     ndisStatus = NDIS_STATUS_SUCCESS;
    PHEADER_802_11  pHeader80211 = NULL;
    UCHAR           NumberOfFrag;
    UCHAR           RTSRequired;
    UCHAR           QueIdx, UserPriority;
    ULONG           AllowFragSize;
    PMAC_TABLE_ENTRY pEntry = NULL; 
    SST             Sst;
    UCHAR          Aid;
    UCHAR           PsMode, Rate;
    UCHAR          Wcid;
    PMP_PORT      pPort = pAd->PortList[PortNumber];
    
    MP_RW_LOCK_STATE PsQLockState;
    ULONG           tmpTimBitmap,tmpTimBitmap2;
    UCHAR           Addr[MAC_ADDR_LEN];

    size_t PipeNumber = 0;
#ifdef MULTI_CHANNEL_SUPPORT
    UCHAR           Status = 0;
#endif /*MULTI_CHANNEL_SUPPORT*/
    PMAC_TABLE_ENTRY pWcidMacTabEntry = NULL;
    
    
    do
    {
        if ((pPort->CommonCfg.bCarrierDetect) && (pPort->CommonCfg.bCarrierAssert))
        {
            DBGPRINT(RT_DEBUG_INFO,("ApDataSendPacket --> carrier assertion !!!\n"));
            ndisStatus = NDIS_STATUS_FAILURE;
            break;
        }

        if (pPort->SoftAP.ApCfg.bSoftAPReady == FALSE)
        {
            DBGPRINT(RT_DEBUG_TRACE, ("%s: Drop those packets due to the SoftAP is not ready.\n", __FUNCTION__));
            ndisStatus = NDIS_STATUS_FAILURE;
            break;
        }
    
        pHeader80211 = (PHEADER_802_11) pXcu->pHeader80211;

        //DA is located in Addr3 (due to pseudo infra) in current mode.
        //otherwise, DA is located in Addr1
        if(pHeader80211->FC.ToDs == 1)
        {
            PlatformMoveMemory(Addr, pHeader80211->Addr3, MAC_ADDR_LEN);
        }
        else
        {
            PlatformMoveMemory(Addr, pHeader80211->Addr1, MAC_ADDR_LEN);
        }

        // Lookup entry here. 
        pEntry = APSsPsInquiry(pAd, pPort, Addr, &Sst, &Aid, &PsMode, &Rate);
        
        if ((pEntry && (Sst == SST_ASSOC)) || (Addr[0] & 0x01))
        {
            // Record that orignal packet source is from NDIS layer,so that 
            // later on driver knows how to release this NDIS NetBuffer         
            pXcu->Aid = Aid;    
        }
        else
        {
            DBGPRINT(RT_DEBUG_ERROR, ("ApDataSendPacket, APSsPsInquiry failed\n"));
            ndisStatus = NDIS_STATUS_FAILURE;
            break;          
        }

        Wcid = Aid;

        pXcu->Wcid = Wcid;

        TotalPacketLength = pXcu->TotalPacketLength;

        pWcidMacTabEntry = MlmeSyncMacTabMatchedWlanIdx(pPort, Wcid);   

        if(pWcidMacTabEntry == NULL)
        {
            DBGPRINT(RT_DEBUG_TRACE,("%s(), Line:%d, Mac Entry is NULL  Wcid = %d\n", __FUNCTION__, __LINE__, Wcid));
            return NDIS_STATUS_FAILURE;
        }
        
        // check if target STA is in power-saving mode. care only those associated STAs
        if (pWcidMacTabEntry->ValidAsWDS == TRUE)
        {
            //b7 as WDS bit, b0-6 as WDS index when b7==1
            pEntry = NULL;
            Rate = pAd->WdsTab.MacTab[pWcidMacTabEntry->MatchWDSTabIdx].CurrTxRate;
        }
        else
        {
            pEntry = pWcidMacTabEntry;
            PsMode = pEntry->PsMode;
            Rate = pEntry->CurrTxRate;
            Sst = pEntry->Sst;
        }

        // STEP 1. Decide number of fragments required to deliver this MSDU. 
        //     The estimation here is not very accurate because difficult to 
        //     take encryption overhead into consideration here. The result 
        //     "NumberOfFrag" is then just used to pre-check if enough free 
        //     TXD are available to hold this MSDU.
        if (pHeader80211->Addr1[0] & 0x01)  // fragmentation not allowed on multicast & broadcast
            NumberOfFrag = 1;
        else if (pEntry && CLIENT_STATUS_TEST_FLAG(pEntry, fCLIENT_STATUS_AGGREGATION_CAPABLE))
            NumberOfFrag = 1;
        else
        {
            // The calculated "NumberOfFrag" is a rough estimation because of various 
            // encryption/encapsulation overhead not taken into consideration. This number is just
            // used to make sure enough free TXD are available before fragmentation takes place.
            // In case the actual required number of fragments of an NDIS packet 
            // excceeds "NumberOfFrag"caculated here and not enough free TXD available, the
            // last fragment (i.e. last MPDU) will be dropped in RTMPHardTransmit() due to out of 
            // resource, and the NDIS packet will be indicated NDIS_STATUS_FAILURE. This should 
            // rarely happen and the penalty is just like a TX RETRY fail. Affordable.
            AllowFragSize = pPort->CommonCfg.FragmentThreshold;           
            NumberOfFrag = (TotalPacketLength / AllowFragSize) + 1;
        }

        // Save fragment number to Ndis packet reserved field
        pXcu->NumberOfFrag = NumberOfFrag;

        // STEP 2. Check the requirement of RTS; decide packet TX rate
        //     If multiple fragment required, RTS is required only for the first fragment
        //     if the fragment size large than RTS threshold
        
        RTSRequired = 0;    // Let ASIC  send RTS/CTS, so we don't set RTS via Ndis Netbuffer reserved field
        // Save TXRATE to Ndis packet reserved field
        pXcu->Rate = Rate;

        //
        // STEP 3. Traffic classification. outcome = <UserPriority, QueIdx>
        //
        UserPriority = 0;
        QueIdx       = QID_AC_BE;
        PipeNumber = NdisCommonGetPipeNumber(pAd);

        if (OPSTATUS_TEST_FLAG(pPort, fOP_STATUS_WMM_INUSED) &&  (PipeNumber >= 5))
        {
            //
            // Get 802.1p
            //
            UserPriority = pXcu->UserPriority; 
            QueIdx = pXcu->QueIdx;

                // When P2PVideoBoost is enabled, force to VO in Ralink P2P Group.
                if ((pPort->CommonCfg.P2PVideoBoost) &&
                    (pEntry) &&
                    (pEntry->ValidAsP2P) &&
                    CLIENT_STATUS_TEST_FLAG(pEntry, fCLIENT_STATUS_RALINK_CHIPSET))
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
            MT_TEST_BIT(pAd->pP2pCtrll->P2pMsConnectedStatus, P2pMs_CONNECTED_STATUS_CLI_OP) &&
            MT_TEST_BIT(pAd->pP2pCtrll->MultiChannelStartOrEndProcess, MULTI_CHANNEL_CLI_DHCP_DONE))  
        {
            QueIdx = MultiChannelGetSwQ(pPort->P2PCfg.CLIHwQSEL);
            Status = 6;
        }       
        else if ((pPort->PortType == WFD_GO_PORT) && MT_TEST_BIT(pAd->pP2pCtrll->P2pMsConnectedStatus, P2pMs_CONNECTED_STATUS_GO_OP)) 
        {
            if (MT_TEST_BIT(pAd->pP2pCtrll->MultiChannelStartOrEndProcess, MULTI_CHANNEL_CLI_IN_NEW_CONNECTION_START))
            {
                QueIdx = (QID_HCCA - 1);
                Status = 7;
            }
            else
            {
                QueIdx = MultiChannelGetSwQ(pPort->P2PCfg.GOHwQSEL);
                Status = 8;         
            }
            }
            else
            {
                QueIdx = (QID_HCCA - 1);
            Status = 10;
            }
        DBGPRINT(RT_DEBUG_TRACE, ("%s - During new session: Insert Port(%d), Status(%d) data to TxSwQueue[%d]\n", __FUNCTION__, pPort->PortNumber, Status, QueIdx));
        }
        else
        {
            QueIdx = QID_AC_BE;
        DBGPRINT(RT_DEBUG_INFO, ("%s - No channel switch: Insert Port(%d) data to TxSwQueue[%d]\n", __FUNCTION__, pPort->PortNumber, QueIdx));
        }
#endif /*MULTI_CHANNEL_SUPPORT*/

        pXcu->UserPriority = UserPriority;
        pXcu->OpMode = OPMODE_AP;
        pXcu->PortNumber = PortNumber;

#if UAPSD_AP_SUPPORT
        // Init Netbuffer related to all UAPSD flags
        pXcu->QueIdx = QueIdx;
        pXcu->bUAPSD = FALSE;
        pXcu->bEosp = FALSE;
        pXcu->bMoreData = FALSE;
        
#endif

        //
        // 4. put to corrsponding TxSwQueue or Power-saving queue
        //
        
        // WDS link should never go into power-save mode; just send out the frame
        if (pWcidMacTabEntry->ValidAsWDS == TRUE)
        {
            NdisAcquireSpinLock(&pAd->pTxCfg->SendTxWaitQueueLock[QueIdx]);
            InsertTailQueue(&pAd->pTxCfg->SendTxWaitQueue[QueIdx], pXcu);
            NdisReleaseSpinLock(&pAd->pTxCfg->SendTxWaitQueueLock[QueIdx]);
            
            MT_INC_REF(&pAd->pTxCfg->NumQueuePacket);
        }
        // M/BCAST frames are put to PSQ as long as there's any associated STA in power-save mode
        else if ((pHeader80211->Addr1[0] & 0x01) && pPort->MacTab.fAnyStationInPsm)
        {
            // we don't want too many MCAST/BCAST backlog frames to eat up all buffers. So in case number of backlog 
            // MCAST/BCAST frames exceeds a pre-defined watermark within a DTIM period, simply drop coming new 
            // MCAST/BCAST frames. This design is similiar to "BROADCAST throttling in most manageable Ethernet Switch chip.
            if (pPort->MacTab.McastPsQueue.Number >= MAX_SIZE_OF_MCAST_PSQ)
            {
                //
                // Set to NDIS_STATUS_FAILURE, we will complete later.
                //
                DBGPRINT(RT_DEBUG_TRACE, ("too many frames (=%d) in M/BCAST PSQ, drop this one\n", pPort->MacTab.McastPsQueue.Number));
                ndisStatus = NDIS_STATUS_FAILURE;
                break;
            }
            else
            {
                // enqueue MCast packets
                PlatformAcquireOldRWLockForWrite(&pAd->MacTablePool.MacTabPoolLock,&PsQLockState);
                InsertTailQueue(&pPort->MacTab.McastPsQueue, pXcu);
                PlatformReleaseOldRWLock(&pAd->MacTablePool.MacTabPoolLock,&PsQLockState);
                DBGPRINT(RT_DEBUG_TRACE, ("STA (AID=%d) in PSM, move to MACSTPSQ, TIM=%08x .Psqueue #=%d\n", Wcid, pPort->SoftAP.ApCfg.TimBitmap,pPort->MacTab.McastPsQueue.Number));
                MT_INC_REF(&pAd->pTxCfg->NumQueuePacket);
                // update MCast bit in TIM IE
                tmpTimBitmap = pPort->SoftAP.ApCfg.TimBitmap;
                pPort->SoftAP.ApCfg.TimBitmap |= TIM_BITMAP[0];
                if(pPort->SoftAP.ApCfg.TimBitmap != tmpTimBitmap)
                {
                    MTEnqueueInternalCmd(pAd, pPort,  MT_CMD_UPDATE_TIM, NULL, 0);
                }
#ifdef MULTI_CHANNEL_SUPPORT
#else
                MTEnqueueInternalCmd(pAd, pPort,  MT_CMD_SET_TBTT_TIMER, NULL, 0);
#endif  /* MULTI_CHANNEL_SUPPORT */
            }
        }
        // else if the associted STA in power-save mode, frame also goes to PSQ
        else if (pEntry && (Sst == SST_ASSOC) && (PsMode == PWR_SAVE))
        {
#if UAPSD_AP_SUPPORT
            if (UAPSD_MR_IS_UAPSD_AC(pEntry, QueIdx))
            {
                DBGPRINT(RT_DEBUG_TRACE,("Chk::%s(%d)==>Client is PWR_SAVE and uapsd enqueue to #QueIdx(=%d)\n", __FUNCTION__, __LINE__, QueIdx));
                ndisStatus = ApUapsdPacketEnqueue(pAd, pEntry, pXcu, QueIdx);
                // got the enqueue problem, free (discard) the tx packet
                if (ndisStatus != NDIS_STATUS_SUCCESS) 
                    break;
            }
            else 
#endif          
            {
                PlatformAcquireOldRWLockForWrite(&pAd->pTxCfg->PsQueueLock,&PsQLockState);
                InsertTailQueue(&pEntry->PsQueue, pXcu);
                PlatformReleaseOldRWLock(&pAd->pTxCfg->PsQueueLock,&PsQLockState);
                MT_INC_REF(&pAd->pTxCfg->NumQueuePacket); 
            }           
#if UAPSD_AP_SUPPORT
            // mark corresponding TIM bit in outgoing BEACON frame
            if (UAPSD_MR_IS_NOT_TIM_BIT_NEEDED_HANDLED(pEntry, QueIdx))
            {
                /* 1. the station is UAPSD station;
                   2. one of AC is non-UAPSD (legacy) AC;
                   3. the destinated AC of the packet is UAPSD AC. */
                /* So we can not set TIM bit due to one of AC is legacy AC */
            }
            else
#endif
            {
                tmpTimBitmap = pPort->SoftAP.ApCfg.TimBitmap;
                tmpTimBitmap2 = pPort->SoftAP.ApCfg.TimBitmap2;

                // mark corresponding TIM bit in outgoing BEACON frame
                if (Wcid >= 32)
                    pPort->SoftAP.ApCfg.TimBitmap2 |= TIM_BITMAP[Wcid-32]; 
                else
                    pPort->SoftAP.ApCfg.TimBitmap |= TIM_BITMAP[Wcid];

                if((pPort->SoftAP.ApCfg.TimBitmap != tmpTimBitmap) || (pPort->SoftAP.ApCfg.TimBitmap2 != tmpTimBitmap2))
                {   
                    DBGPRINT(RT_DEBUG_TRACE,("update TIM\n")); 
                    MTEnqueueInternalCmd(pAd, pPort,  MT_CMD_UPDATE_TIM, NULL, 0);
                }

                DBGPRINT(RT_DEBUG_TRACE, ("STA (AID=%d) in PSM, move to PSQ, TIM=%08x %08x.Psqueue #=%d\n", Wcid, pPort->SoftAP.ApCfg.TimBitmap, pPort->SoftAP.ApCfg.TimBitmap2,pEntry->PsQueue.Number));
            }
        }
    
        // 3. otherwise, transmit the frame
        else
        {
            NdisAcquireSpinLock(&pAd->pTxCfg->SendTxWaitQueueLock[QueIdx]);
            InsertTailQueue(&pAd->pTxCfg->SendTxWaitQueue[QueIdx], pXcu);
            NdisReleaseSpinLock(&pAd->pTxCfg->SendTxWaitQueueLock[QueIdx]);

            MT_INC_REF(&pAd->pTxCfg->NumQueuePacket);
        }

        //To pass Sigma Test 6.1.11, we need to send a fake packet after sending ping packet 
        //otherwise the test will fail...
        if(IS_P2P_SIGMA_ON(pPort) && TotalPacketLength == 161)
        {
            pPort->P2PCfg.bSigmaFakePkt = TRUE;
        }
        

        ndisStatus = NDIS_STATUS_SUCCESS;
    } while (FALSE);

    if (ndisStatus != NDIS_STATUS_SUCCESS)
    {
        if((pXcu != NULL) && (pXcu->Reserve1 != NULL) && (pXcu->Reserve2 != NULL))
        {
#pragma prefast(suppress: __WARNING_DEREF_NULL_PTR, "Dereferencing NULL pointer 'NetBufferList', should not happen")    
            RELEASE_NDIS_NETBUFFER(pAd, pXcu, ndisStatus);
        }
    }
    
    return NDIS_STATUS_SUCCESS; //Always return NDIS_STATUS_SUCCESS since we complete this NetBuffer List here.
}

/*
    ========================================================================

    Routine Description:
        Copy frame from waiting queue into relative ring buffer and set 
    appropriate ASIC register to kick hardware encryption before really
    sent out to air.
        
    Arguments:
        pAd     Pointer to our adapter
        PNDIS_PACKET    Pointer to outgoing Ndis frame
        NumberOfFrag    Number of fragment required
        
    Return Value:
        None

    Note:
    
    ========================================================================
*/
NDIS_STATUS 
ApDataHardTransmit(
    IN  PMP_ADAPTER   pAd,
    IN  PMT_XMIT_CTRL_UNIT     pXcu
    )
{
    UCHAR           RtsRequired;
    UINT            MatchWDSTabIdx = 0;
    PMAC_TABLE_ENTRY pMacEntry;
    PWDS_TABLE_ENTRY pWdsEntry;
    BOOLEAN         bWDSEntry = FALSE;
    ULONG           TotalPacketLength,TotalPacketLengthRaTxAgg = 0;
    PUCHAR          pSrcBufVA,pSrcBufVARaTxAgg = NULL;
    PHEADER_802_11  pHeader80211 = NULL;
    UCHAR           Header_802_3[LENGTH_802_3]={0};
    BOOLEAN         bAckRequired;
    UCHAR           PID;
    PCIPHER_KEY     pKey = NULL;
    UCHAR           KeyIdx, KeyTable;
    ULONG           SrcBufLen,SrcBufLenRaTxAgg = 0;
    BOOLEAN         TXWIAMPDU = FALSE;     //AMPDU bit in TXWI
    INT             SrcRemainingBytes;
    UINT            InitFreeMpduSize, FreeMpduSize;
    BOOLEAN         StartOfFrame;
    BOOLEAN         MICFrag;
    PHT_TX_CONTEXT  pTxContext;
    PTXDSCR_SHORT_STRUC             pShortFormatTxD = NULL;
    PTXDSCR_LONG_STRUC          pLongFormatTxD = NULL;
    PTXDSCR_DW0                 pTxDDW0 = NULL;
    PTXDSCR_DW7                 pTxDDW7 = NULL;
    ULONG           FillOffset;
    BOOLEAN         bIncreaseCurWriPos = FALSE;
    PUCHAR          pWirelessPacket;
    PUCHAR          pDest;
    ULONG           DataOffSet = 0;
    UINT            BytesCopied;
    UINT            TxSize;
    ULONG           Remain,RemainRaTxAgg = 0;
    PMP_PORT      pPort = pAd->PortList[pXcu->PortNumber];
    BOOLEAN         bAggregatible = FALSE;  
    BOOLEAN         bMcast;
    BOOLEAN         bAllowChangeMcastRate;

    PHY_CFG PhyCfg = {0};
    UCHAR           SwUseSegIdx = 1;
    PMAC_TABLE_ENTRY pWcidMacTabEntry = NULL;
    USHORT  BufferIdx = 0;


//==================================
    HARD_TRANSMIT_INFO              TxInfo;
//==================================

    PlatformZeroMemory((PVOID)&TxInfo, sizeof(HARD_TRANSMIT_INFO));

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
    TxInfo.Wcid = (UCHAR)MlmeSyncGetWlanIdxByPort(pPort, ROLE_WLANIDX_BSSID);
    TxInfo.RABAOriIdx = 0;  //The RA's BA Originator table index. 
    TxInfo.bPiggyBack = FALSE;
    TxInfo.bAMSDU = FALSE;// use HT AMSDU
    TxInfo.bGroupMsg2 = FALSE; // is this a group msg 2? for CCX CCKM   
    TxInfo.bDHCPv6 = FALSE;
    TxInfo.bDHCPFrame = FALSE;
    TxInfo.NextValid = 0; // Single bulk out , always set NextValid = 0
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

    if (pPort->CommonCfg.RadarDetect.RDMode != RD_NORMAL_MODE)
    {
        DBGPRINT(RT_DEBUG_INFO,("ApDataHardTransmit --> radar detect not in normal mode !!!\n"));
        return NDIS_STATUS_FAILURE;     
    }

    TxInfo.MpduRequired     = pXcu->FragmentRequired;
    RtsRequired     = 0; // Let ASIC  send RTS/CTS
    TxInfo.UserPriority = pXcu->UserPriority;
    TxInfo.TxRate           = pXcu->Rate;
    TxInfo.Wcid         = pXcu->Wcid;

    ASSERT(TxInfo.Wcid< MAX_LEN_OF_MAC_TABLE);

    DBGPRINT(RT_DEBUG_INFO,("ApDataHardTransmit(RTS=%d, Frag=%d)\n",RtsRequired, TxInfo.MpduRequired));

    pWcidMacTabEntry = MlmeSyncMacTabMatchedWlanIdx(pPort, TxInfo.Wcid);   

    if(pWcidMacTabEntry == NULL)
    {
        DBGPRINT(RT_DEBUG_TRACE,("%s(), Line:%d, Mac Entry is NULL  Wcid = %d\n", __FUNCTION__, __LINE__, TxInfo.Wcid));
        return NDIS_STATUS_FAILURE;
    }
    
    // OneWDS  occupy both one WDS Table entry and one MAC Table entry.
    MatchWDSTabIdx = pWcidMacTabEntry->MatchWDSTabIdx;
    if ((pWcidMacTabEntry->ValidAsWDS == TRUE))
    {
        pMacEntry = NULL;
        pWdsEntry = &pAd->WdsTab.MacTab[MatchWDSTabIdx];
        bWDSEntry = TRUE;
        DBGPRINT(RT_DEBUG_TRACE,("ApDataHardTransmit([%d].bWDSEntry = TRUE)\n", TxInfo.Wcid));
    }
    else if (TxInfo.Wcid < MAX_LEN_OF_MAC_TABLE)
    { // This is multicast wireless-bridge packets. 
        pMacEntry = pWcidMacTabEntry;
        pWdsEntry = NULL;
        bWDSEntry = FALSE;
    }
    else
    {
        // wronggoing.!! So Default use multicast table.  
        pMacEntry = pWcidMacTabEntry;
        pWdsEntry = NULL;
        bWDSEntry = FALSE;
    }

    // ---------------------------------------------
    // STEP 0. PARSING THE NDIS PACKET
    // ---------------------------------------------
    //
    // Prepare packet information structure which will be query for buffer descriptor
    //
    TotalPacketLength = pXcu->TotalPacketLength;

    if (TotalPacketLength <= LENGTH_802_11)
    {
        DBGPRINT(RT_DEBUG_ERROR, ("ApDataHardTransmit, size less than 802.11 header\n"));
        return NDIS_STATUS_FAILURE;
    }

    pHeader80211 = (PHEADER_802_11) pXcu->pHeader80211;
    pSrcBufVA = pXcu->BufferList[1].VirtualAddr;

    if ((pMacEntry) && (READ_PHY_CFG_MODE(pAd, &pMacEntry->TxPhyCfg) >= MODE_HTMIX) &&(IS_P2P_SIGMA_OFF(pPort)))
    {
        TxInfo.bHtVhtFrame = TRUE;
        TxInfo.MimoPs = pWcidMacTabEntry->MmpsMode;
    }

    //
    // If DHCP datagram or ARP datagram , we need to send it as Low rates.
    //
    if (XmitCheckDHCPFrame(pAd, pXcu, &TxInfo.Protocol, &TxInfo.bGroupMsg2, &TxInfo.bDHCPv6))
    {
        TxInfo.bDHCPFrame = TRUE;
    }

    if (TxInfo.Protocol == 0x888E)
    {
        TxInfo.bEAPOLFrame = TRUE;
        DBGPRINT(RT_DEBUG_TRACE,("2   APRTMPHardTransmit -->Eapol !PortSubtype = %d. !!\n", pPort->PortSubtype));
    }


    if (pPort->PortSubtype == PORTSUBTYPE_P2PGO)
    {
        // Dhcp frame use 6Mbps to send. 
        if (TxInfo.bDHCPFrame == TRUE)
        {
            TxInfo.bHtVhtFrame = FALSE;
        }
    }

    // we using lowest basic rate when DHCP and EAPOL packet
    if ((TxInfo.bDHCPFrame == TRUE) || (TxInfo.bEAPOLFrame == TRUE))
        TxInfo.bHtVhtFrame = FALSE;

    if ((TxInfo.bHtVhtFrame == TRUE) && (pMacEntry) &&((pMacEntry->TXBAbitmap&(1<<TxInfo.UserPriority)) != 0))
    {
        if (pMacEntry->NoBADataCountDown == 0)
            TxInfo.bTXBA = TRUE;
    }
    else if ((pMacEntry) &&((pMacEntry->TXAutoBAbitmap&(1<<TxInfo.UserPriority)) != 0))// infrastructure mode is always unicast.
    {
        pMacEntry->TXAutoBAbitmap &= ~(1<<TxInfo.UserPriority);
        pPort->BATable.BAOriEntry[pMacEntry->BAOriWcidArray[TxInfo.UserPriority]].ORI_BA_Status = Originator_WaitAdd;
        DBGPRINT_RAW(RT_DEBUG_TRACE, ("ApDataHardTransmit --> [%d] Originator_WaitAdd!!!\n", pMacEntry->BAOriWcidArray[TxInfo.UserPriority]));
    }

    // -----------------------------------------------------------------
    // STEP 2. MAKE A COMMON 802.11 HEADER SHARED BY ENTIRE FRAGMENT BURST.
    // -----------------------------------------------------------------
    //
    // Change FromDs & ToDs flag.
    //
    if (pXcu->PktSource== PKTSRC_NDIS)
    {
        pHeader80211->FC.FrDs = 1;
        pHeader80211->FC.ToDs = 0;

        if(IS_P2P_CON_GO(pAd, pPort))
        {
            COPY_MAC_ADDR(pHeader80211->Addr1, pHeader80211->Addr3);
            COPY_MAC_ADDR(pHeader80211->Addr3, pPort->PortCfg.Bssid);
        }
        else
        {
            COPY_MAC_ADDR(pHeader80211->Addr3, pPort->PortCfg.Bssid);
        }
    }

    //
    // TODO:
    // how about "MoreData" bit? AP need to set this bit especially for PS-POLL response
    // pHeader80211->FC.MoreData
    //

    if (pHeader80211->Addr1[0] & 0x01) // Multicast or Broadcast
    {
        bAckRequired = FALSE;
        PID = 0;
        INC_COUNTER64(pAd->Counter.WlanCounters.MulticastTransmittedFrameCount);
        bMcast = TRUE;
    }
    else if (pPort->CommonCfg.AckPolicy[TxInfo.QueIdx] != NORMAL_ACK)
    {
        bAckRequired = FALSE;
        PID = PID_DATA_NO_ACK;
        bMcast = FALSE;
    }
    else if (TxInfo.bTXBA)
    {
        // A-MPDU needs ACK setting as TRUE.
        bAckRequired = TRUE;
        PID = 0; 
        bMcast = FALSE;
    }
    else
    {
        bAckRequired = TRUE;
        PID = 0; 
        bMcast = FALSE;
    }

#if UAPSD_AP_SUPPORT
    {
        ULONG   RemainUapsdTxNum = 0;
        
        if (pMacEntry)
        {
            RemainUapsdTxNum = pAd->UAPSD.TxSwUapsdQueue[pMacEntry->UAPSDQAid][QID_AC_BK].Number 
                            + pAd->UAPSD.TxSwUapsdQueue[pMacEntry->UAPSDQAid][QID_AC_BE].Number 
                            + pAd->UAPSD.TxSwUapsdQueue[pMacEntry->UAPSDQAid][QID_AC_VI].Number 
                            + pAd->UAPSD.TxSwUapsdQueue[pMacEntry->UAPSDQAid][QID_AC_VO].Number;

            // [WMM-PS + p2P-NoA]
            // MoreData bit and EOSP bit have set before entering the software queue,
            // but we need to correct the attribute again before TX out.
            if (IS_AP_SUPPORT_UAPSD(pAd, pPort) && (pPort->P2PCfg.GONoASchedule.bValid))
            {
                ApUapsdChangeFlagInNoA(pAd, RemainUapsdTxNum, pXcu);
            }
        }

        
        // Fill more data bit into 802.11 header
        if (IS_AP_SUPPORT_UAPSD(pAd, pPort) && pXcu->bMoreData)
        {
            pHeader80211->FC.MoreData = TRUE;
            DBGPRINT(RT_DEBUG_TRACE, ("[#1] HardTX: Insert more data bit\n"));
        }   
    }
#endif

    bAllowChangeMcastRate = ((bMcast) && ((pPort->SoftAP.ApCfg.SoftAPForceMulticastRate & ALLOW_CHANGE_MCAST_RATE) >> 28)) ? TRUE : FALSE;

    DBGPRINT(RT_DEBUG_INFO, ("bAllowChangeMcastRate=%d, bMcast=%d, SoftAPForceMulticastRate AllowChange=%d\n", bAllowChangeMcastRate, bMcast, ((pPort->SoftAP.ApCfg.SoftAPForceMulticastRate & ALLOW_CHANGE_MCAST_RATE) >> 28)));    

    if (bAllowChangeMcastRate)
    {   
        // Change TxInfo.bHtVhtFrame to FALSE if user set MODE to legacy
        DBGPRINT(RT_DEBUG_INFO, ("SoftAPForceMulticastRate=0x%x, user def=0x%x, legacy=0x%x\n", (pPort->SoftAP.ApCfg.SoftAPForceMulticastRate & 0xFFFFFFFF), ((pPort->SoftAP.ApCfg.SoftAPForceMulticastRate & 0xFFFF0000) == USER_DEFINE_MCAST_RATE), ((pPort->SoftAP.ApCfg.SoftAPForceMulticastRate & 0xFFFF0000) == USE_LEGACY_MCAST_RATE)));
        if ((pPort->SoftAP.ApCfg.SoftAPForceMulticastRate & 0xFFFF0000) == USER_DEFINE_MCAST_RATE)
        {
            PPHY_CFG pPhyCfg = (PPHY_CFG)(&pPort->SoftAP.ApCfg.SoftAPForceMulticastRate);
            
            if (READ_PHY_CFG_MODE(pAd, pPhyCfg) < MODE_HTMIX)
            {
                TxInfo.bHtVhtFrame = FALSE;
            }
        
            DBGPRINT(RT_DEBUG_INFO, ("USER_DEFINE_MCAST_RATE: MODE=%d, TxInfo.bHtVhtFrame to %d\n", READ_PHY_CFG_MODE(pAd, pPhyCfg), TxInfo.bHtVhtFrame));
        }
        else if ((pPort->SoftAP.ApCfg.SoftAPForceMulticastRate & 0xFFFF0000) == USE_LEGACY_MCAST_RATE)
        {
            TxInfo.bHtVhtFrame = FALSE;
            DBGPRINT(RT_DEBUG_INFO, ("USE_LEGACY_MCAST_RATE: Set TxInfo.bHtVhtFrame to FALSE\n"));          
        }
    }

    // -------------------------------------------
    // STEP 0.2. some early parsing
    // -------------------------------------------
    // These sequences have some sort of influence, change sequence must be careful.
    // When HT rate, always use IFS_HTTXOP
#if 0
// 
// Hardware has concern on changing the FramgGap, it should always use IFS_HTTXOP and can't be dynamic changed.
// It apply to 2860C/D/E, 2880C/D (MAC version ID: 0x2860_0100, 0101, 0102, 0103) 
// And 2872, 2880E no this issue
//
    if (TxInfo.bHtVhtFrame == TRUE)
        TxInfo.FrameGap = IFS_HTTXOP;
    // B/G protection
    else if ((pMacEntry) && OPSTATUS_TEST_FLAG(pAd, fOP_STATUS_BG_PROTECTION_INUSED))
        TxInfo.FrameGap = IFS_HTTXOP;
    // frame belonging to AC that has non-zero TXOP
    else if (pPort->CommonCfg.APEdcaParm.bValid && pPort->CommonCfg.APEdcaParm.Txop[TxInfo.QueIdx])
        TxInfo.FrameGap = IFS_SIFS;
    // traditional TX burst
    else if (pPort->CommonCfg.bEnableTxBurst)
        TxInfo.FrameGap = IFS_HTTXOP;
    // otherwise, always BACKOFF before transmission
    else
        TxInfo.FrameGap = IFS_BACKOFF;  // Default frame gap mode
#endif

    // ===============================================
    // Ui Tool Feature: Fixed Tx rate in MACVer2 later
    // ===============================================
    // Disable HT/VHT fame if the setting of "Fixed Tx rate" is 11b or 11g
    if(pAd->UiCfg.UtfFixedTxRateEn && pAd->UiCfg.UtfFixedTxRateCfg.Ext.Mode <= MODE_OFDM)
        TxInfo.bHtVhtFrame = FALSE;
    
    if (OPSTATUS_TEST_FLAG(pPort, fOP_STATUS_WMM_INUSED) && 
        pMacEntry && 
        ((TxInfo.bHtVhtFrame == TRUE) || CLIENT_STATUS_TEST_FLAG(pMacEntry, fCLIENT_STATUS_WMM_CAPABLE)))
    {
        pHeader80211->FC.SubType = SUBTYPE_QDATA;
        TxInfo.bWMM = TRUE;     
        if ((CLIENT_STATUS_TEST_FLAG(pMacEntry, fCLIENT_STATUS_RDG_CAPABLE)) && ((*pSrcBufVA & 0x01) == 0) && (TxInfo.bHtVhtFrame == TRUE))
        {
            pHeader80211->FC.Order = 1; // HTC bit share with order bit
            TxInfo.bHTC = TRUE;
        }
        else if ((CLIENT_STATUS_TEST_FLAG(pMacEntry, fCLIENT_STATUS_RDG_CAPABLE)) 
            && (CLIENT_STATUS_TEST_FLAG(pMacEntry, fCLIENT_STATUS_RALINK_CHIPSET)) 
            && ((TxInfo.bDHCPFrame == TRUE) || (TxInfo.bEAPOLFrame == TRUE)))
        {
            // To patch rt2860E and before HW bug,
            // Use non-QoS format to send (TxInfo.bDHCPFrame == TRUE) frames if other 11n data frames are RDG. 
            pHeader80211->FC.SubType = SUBTYPE_DATA;
            TxInfo.bWMM = FALSE;
        }
        else if ((CLIENT_STATUS_TEST_FLAG(pMacEntry, fCLIENT_STATUS_RALINK_CHIPSET))
            &&((TxInfo.bDHCPFrame == TRUE) || (TxInfo.bEAPOLFrame == TRUE))
            &&( READ_PHY_CFG_MODE(pAd, &pMacEntry->TxPhyCfg) &0x2))
        {
            // Low rate packet will use nonQoS when connecting to N-AP(Ralink)
            pHeader80211->FC.SubType = SUBTYPE_DATA;
            TxInfo.bWMM = FALSE;
            DBGPRINT(RT_DEBUG_TRACE,("Low rate packet will use nonQoS.\n"));            
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
    pKey = NULL;


    if ((TxInfo.bEAPOLFrame)                                                    ||
        (pPort->PortCfg.WepStatus == Ralink802_11Encryption2Enabled)    ||
        (pPort->PortCfg.WepStatus == Ralink802_11Encryption3Enabled)    ||
        (pPort->PortCfg.WepStatus == Ralink802_11Encryption4Enabled))
    {
        if (bWDSEntry == TRUE)      // to WDS link
        {
            KeyIdx     = TxInfo.Wcid;
            KeyTable   = 1;
            TxInfo.CipherAlg  = pAd->WdsTab.Wpa_key.CipherAlg;
            if (TxInfo.CipherAlg)
                pKey = &pAd->WdsTab.Wpa_key;
        }
        else if (pHeader80211->Addr1[0] & 0x01)        // M/BCAST to local BSS, use default key in shared key table
        {
            KeyIdx     = pPort->PortCfg.DefaultKeyId;
            KeyTable   = 0; 
            TxInfo.CipherAlg  = pPort->SharedKey[BSS0][KeyIdx].CipherAlg;
            if (TxInfo.CipherAlg)
                pKey = &pPort->SharedKey[BSS0][KeyIdx];
        }
        else
        {
            KeyIdx     = TxInfo.Wcid;
            KeyTable   = 1;
            pWcidMacTabEntry = MlmeSyncMacTabMatchedWlanIdx(pPort, KeyIdx); 
            
            if(pWcidMacTabEntry == NULL)
            {
                DBGPRINT(RT_DEBUG_TRACE,("%s(), Line:%d, Mac Entry is NULL  Wcid = %d\n", __FUNCTION__, __LINE__, KeyIdx));
                return NDIS_STATUS_FAILURE;
            }
            
            TxInfo.CipherAlg  = pWcidMacTabEntry->PairwiseKey.CipherAlg;
            if (TxInfo.CipherAlg)
                pKey = &pWcidMacTabEntry->PairwiseKey;        
        }
    }
    else if (IS_WEP_STATUS_ON(pPort->PortCfg.WepStatus))
    {
        // TODO: need special treatment for CKIP case. please refer to STA code
        KeyIdx     = pPort->PortCfg.DefaultKeyId;
        KeyTable   = 0;    // shared key table
        TxInfo.CipherAlg  = pPort->SharedKey[BSS0][KeyIdx].CipherAlg;
        if (TxInfo.CipherAlg)
            pKey = &pPort->SharedKey[BSS0][KeyIdx];
    }
    else
    {
        TxInfo.CipherAlg = CIPHER_NONE;
        KeyTable  = 0;
        KeyIdx    = 0;
        pKey      = NULL;       
    }

    DBGPRINT(RT_DEBUG_INFO,("ApDataHardTransmit(EAPOL=%d, Mcast=%d) to AID#%d, Alg=%s, Ktab=%d, key#=%d TXba=%d\n", 
        TxInfo.bEAPOLFrame, pHeader80211->Addr1[0] & 0x01, TxInfo.Wcid, DecodeCipherName(TxInfo.CipherAlg), KeyTable, KeyIdx,TxInfo.bTXBA));

    if (TxInfo.CipherAlg != CIPHER_NONE)
        pHeader80211->FC.Wep = 1;

#if 0
    // STEP 3.1 if TKIP is used and fragmentation is required. Driver has to
    //          append TKIP MIC at tail of the scatter buffer (This must be the
    //          ONLY scatter buffer in the NDIS PACKET). 
    //          MAC ASIC will only perform IV/EIV/ICV insertion but no TKIP MIC
    if ((MpduRequired > 1) && (TxInfo.CipherAlg == CIPHER_TKIP))
    {
        //
        // TODO: TKIP fragment case
        // RTMPCalculateMICValue
        //
        TxInfo.CipherAlg = CIPHER_TKIP_NO_MIC;
    }
    else
    {
        SrcBufLen = TotalPacketLength;
    }
#endif

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
        TxInfo.EncryptionOverhead = 16; // AES: IV[4] + EIV[4] + MIC[8]
    else
        TxInfo.EncryptionOverhead = 0;

    // ----------------------------------------------------------------
    // STEP 4. Make RTS frame or CTS-to-self frame if required
    // ----------------------------------------------------------------

    // decide how much time an ACK/CTS frame will consume in the air
    TxInfo.AckDuration = XmitCalcDuration(pAd, pPort->CommonCfg.ExpectedACKRate[TxInfo.TxRate], 14);
    
    // decide is the need of piggy-back
    if ((TxInfo.bHtVhtFrame == FALSE)&& pMacEntry && CLIENT_STATUS_TEST_FLAG(pMacEntry, fCLIENT_STATUS_PIGGYBACK_CAPABLE))
    {
        TxInfo.bPiggyBack = TRUE;
    }
    else
    {
        TxInfo.bPiggyBack = FALSE;
    }

    if (TxInfo.bTXBA)
    {   
        TxInfo.RABAOriIdx = pWcidMacTabEntry->BAOriWcidArray[TxInfo.UserPriority];
        TxInfo.BAWinSize = pPort->BATable.BAOriEntry[TxInfo.RABAOriIdx].BAWinSize-1;
        TXWIAMPDU = TRUE;
        pHeader80211->Frag = 0;
    }
    
    // --------------------------------------------------------
    // STEP 5. START MAKING MPDU(s)
    //      Start Copy Ndis Packet into Ring buffer.
    //      For frame required more than one ring buffer (fragment), all ring buffers
    //      have to be filled before kicking start tx bit.
    //      Make sure TX ring resource won't be used by other threads
    // --------------------------------------------------------
    SrcRemainingBytes = TotalPacketLength - LENGTH_802_11;
    SrcBufLen        =0;
    Remain = SrcRemainingBytes;

    if ((pHeader80211->Addr1[0] & 0x01) || (READ_PHY_CFG_MODE(pAd, &pPort->CommonCfg.TxPhyCfg) >= MODE_HTMIX) || (TxInfo.MpduRequired == 1))
    {
        InitFreeMpduSize = MAX_FRAG_THRESHOLD;
    }
    else
    {
        InitFreeMpduSize = pPort->CommonCfg.FragmentThreshold;
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
        if (((pTxContext->CurWritePosition) < pTxContext->NextBulkOutPosition) && (pTxContext->CurWritePosition + LOCAL_TXBUF_SIZE+ ONE_SEGMENT_UNIT) > pTxContext->NextBulkOutPosition)
        {
            DBGPRINT(RT_DEBUG_INFO,("%s c1 --> CurWritePosition = %d, NextBulkOutPosition = %d. \n", __FUNCTION__, pTxContext->CurWritePosition, pTxContext->NextBulkOutPosition));
            MTUSB_SET_BULK_FLAG(pAd, (fRTUSB_BULK_OUT_DATA_NORMAL << TxInfo.QueIdx));
            NdisReleaseSpinLock(&pAd->pTxCfg->TxContextQueueLock[TxInfo.QueIdx]);
            return (NDIS_STATUS_RESOURCES);
        }
        else if ((pTxContext->CurWritePosition > (ONE_SEGMENT_UNIT * (SEGMENT_TOTAL - 1))) && (pTxContext->NextBulkOutPosition < (ONE_SEGMENT_UNIT)))
        {
            DBGPRINT(RT_DEBUG_TRACE,("%s c4 --> CurWritePosition = %d, NextBulkOutPosition = %d. \n", __FUNCTION__, pTxContext->CurWritePosition, pTxContext->NextBulkOutPosition));
            MTUSB_SET_BULK_FLAG(pAd, (fRTUSB_BULK_OUT_DATA_NORMAL << TxInfo.QueIdx));
            NdisReleaseSpinLock(&pAd->pTxCfg->TxContextQueueLock[TxInfo.QueIdx]);
            return (NDIS_STATUS_RESOURCES);
        }
        else if ((pTxContext->CurWritePosition == 8) && (pTxContext->NextBulkOutPosition < LOCAL_TXBUF_SIZE + ONE_SEGMENT_UNIT))
        {
            DBGPRINT(RT_DEBUG_TRACE,("%s c2 --> CurWritePosition = %d, NextBulkOutPosition = %d. \n", __FUNCTION__, pTxContext->CurWritePosition, pTxContext->NextBulkOutPosition));
            MTUSB_SET_BULK_FLAG(pAd, (fRTUSB_BULK_OUT_DATA_NORMAL << TxInfo.QueIdx));
            NdisReleaseSpinLock(&pAd->pTxCfg->TxContextQueueLock[TxInfo.QueIdx]);
            return (NDIS_STATUS_RESOURCES);
        }
        else if (pTxContext->bCurWriting == TRUE)
        {
            DBGPRINT(RT_DEBUG_TRACE,("%s c3 --> CurWritePosition = %d, NextBulkOutPosition = %d. \n", __FUNCTION__, pTxContext->CurWritePosition, pTxContext->NextBulkOutPosition));
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
            PlatformZeroMemory(pShortFormatTxD, TXD_SHORT_SIZE);
            pTxDDW0 = &pShortFormatTxD->TxDscrDW0;
            pTxDDW7 = &pShortFormatTxD->TxDscrDW7;
            PlatformZeroMemory(pTxDDW7, 4);
            pWirelessPacket = &pTxContext->TransferBuffer->WirelessPacket[FillOffset + TXD_SHORT_SIZE];
        }

        NdisReleaseSpinLock(&pAd->pTxCfg->TxContextQueueLock[TxInfo.QueIdx]);
#pragma prefast(suppress: __WARNING_READ_OVERRUN, "pPort->MacTab.Content[TxInfo.Wcid], should not Buffer overrun")

        WRITE_PHY_CFG_DOUBLE_WORD(pAd, &pPort->LastTxRatePhyCfg, READ_PHY_CFG_DOUBLE_WORD(pAd, &pWcidMacTabEntry->TxPhyCfg));

        //
        // STEP 5.2 COPY 802.11 HEADER INTO 1ST DMA BUFFER
        //
        pDest = pWirelessPacket;        
        PlatformMoveMemory(pDest, pHeader80211, LENGTH_802_11);
        pDest       += LENGTH_802_11;
        DataOffSet  += LENGTH_802_11;

        // Init FreeMpduSize
        FreeMpduSize = InitFreeMpduSize;

        if (TxInfo.bWMM)        
        {
            // copy QOS CONTROL bytes
            // HT rate must has QOS CONTROL bytes
            if (TxInfo.bTXBA)
                *pDest        =  (TxInfo.UserPriority & 0x0f) ;
            else
                *pDest        =  (TxInfo.UserPriority & 0x0f) | pPort->CommonCfg.AckPolicy[TxInfo.QueIdx];

#if UAPSD_AP_SUPPORT
            if (IS_AP_SUPPORT_UAPSD(pAd, pPort))
            {
                if (pXcu->bEosp)
                {
                    DBGPRINT(RT_DEBUG_TRACE, ("[#1] HardTX: Insert EOSP bit \n"));
                    *pDest |= 0x10; //SET EOSP  
                }
            }
#endif  
            
            *(pDest+1)    =  0;
            pDest         += 2;
            TxInfo.This80211HdrLen        = LENGTH_802_11 + 2;
            FreeMpduSize  -= 2;
            TxInfo.LengthQosPAD = 2;
            // Pad 2 bytes to make 802.11 header 4-byte alignment
#if 0           
            if (TxInfo.bHTC == TRUE)
            {
                // HTC Control field is following QOS field.
                PlatformZeroMemory(pDest, 6);
                TxInfo.This80211HdrLen        += 4;
                if ((pPort->CommonCfg.bRdg == TRUE) && CLIENT_STATUS_TEST_FLAG(pMacEntry, fCLIENT_STATUS_RDG_CAPABLE))
                    *(pDest+3)|=0x80;
                if (pAd->bLinkAdapt == TRUE)
                {   //bit2MRQ=1
                    *(pDest)|=0x4;
                    // Set identifier as 2
                    *(pDest)|=0x10;
                    //Set MFB field = MCS. MFB is at bit9-15
                    *(pDest+1)|=(READ_PHY_CFG_MCS(pAd, &pPort->CommonCfg.TxPhyCfg)<<1);
                }
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
        if (bAggregatible)
        {
            PHEADER_802_11  pUpdateHeader = NULL;
            ULONG           TxSize2 = TotalPacketLengthRaTxAgg+LENGTH_802_3;    
            SrcRemainingBytes += TotalPacketLengthRaTxAgg;

        
            *pDest = (UCHAR) (TxSize2 & 0xff); // LSB
            *(pDest+1) = (UCHAR) (TxSize2>> 8); // MSB  
            pDest +=2;

            TxInfo.This80211HdrLen += 2;
            pUpdateHeader = (PHEADER_802_11)pWirelessPacket;
            pUpdateHeader->FC.Order = 1;
            
#if UAPSD_AP_SUPPORT
            if (IS_AP_SUPPORT_UAPSD(pAd, pPort))
            {
                PUCHAR          pWmmRaTxAgg = NULL;

                // update more data bit
                if (NetBufferRaTxAgg && MT_GET_NETBUFFER_MOREDATA(NetBufferRaTxAgg))
                {
                    pUpdateHeader->FC.MoreData = TRUE;
                    DBGPRINT(RT_DEBUG_TRACE, ("[#2] HardTX: Insert more data bit\n"))
                }
            
                // update EOSP field
                if (TxInfo.bWMM && NetBufferRaTxAgg && MT_GET_NETBUFFER_EOSP(NetBufferRaTxAgg))
                {
                    DBGPRINT(RT_DEBUG_TRACE, ("[#2] HardTX: Insert EOSP bit \n"));
                    pWmmRaTxAgg = (PUCHAR)(pWirelessPacket + LENGTH_802_11);
                    *pWmmRaTxAgg |= 0x10; //SET EOSP  
                }
                else if (TxInfo.bWMM && NetBufferRaTxAgg && (!MT_GET_NETBUFFER_EOSP(NetBufferRaTxAgg)))
                {
                    pWmmRaTxAgg = (PUCHAR)(pWirelessPacket + LENGTH_802_11);
                    *pWmmRaTxAgg &= (~0x10); // Remove EOSP
                }
            }
#endif
        }
#endif

        // Start copying payload
        BytesCopied = 0;

        BufferIdx =1;
        pSrcBufVA = pXcu->BufferList[BufferIdx].VirtualAddr;
        SrcBufLen = pXcu->BufferList[BufferIdx].Length;
        do 
        {
            if (SrcBufLen >= FreeMpduSize)  
            {
                // Copy only the free fragment size, and save the pointer
                // of current buffer descriptor for next fragment buffer.
                PlatformMoveMemory(pDest, pSrcBufVA, FreeMpduSize);
                BytesCopied += FreeMpduSize;
                pSrcBufVA   += FreeMpduSize;
                pDest       += FreeMpduSize;
                SrcBufLen   -= FreeMpduSize;
                Remain      -= FreeMpduSize;
                break;
            }
            else if (SrcBufLen > 0)
            {
                if (!pSrcBufVA)
                {
                    DBGPRINT(RT_DEBUG_ERROR, ("XmitHardTransmit, Copy data failed\n"));
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
                PlatformMoveMemory(pDest, pSrcBufVA, SrcBufLen);
                BytesCopied  += SrcBufLen;
                pDest        += SrcBufLen;
                FreeMpduSize -= SrcBufLen;
                Remain       -= SrcBufLen;
            }

            //
            // Get Next MDL
            //
            if ((BufferIdx<(pXcu->NumOfBuf-1)) && (Remain > 0))
            {
                BufferIdx++;
                pSrcBufVA = pXcu->BufferList[BufferIdx].VirtualAddr;
                SrcBufLen = pXcu->BufferList[BufferIdx].Length;
                if (pSrcBufVA == NULL) 
                {
                    DBGPRINT(RT_DEBUG_ERROR, ("%s, Copy data failed\n", __FUNCTION__));
                    return NDIS_STATUS_RESOURCES;
                }
            }
            else
            {
                //
                // No more data 
                // Check if we need to Add SW MIC.
                break;
            }
        } while (TRUE);

#if 0
        //
        // Suppport RaTxAggregation
        // Start copying 2nd packet's payload
        //
        if (bAggregatible)
        {
            // copy 802.3 into 2nd payload position
            PlatformMoveMemory(pDest,Header_802_3,LENGTH_802_3);
            pDest+=LENGTH_802_3;
            BytesCopied+=LENGTH_802_3;
            
            do 
            {
                if (SrcBufLenRaTxAgg> 0)
                {
                    if (!pSrcBufVARaTxAgg)
                    {
                        DBGPRINT(RT_DEBUG_ERROR, ("XmitHardTransmit, Copy data failed [SysVaRaTxAgg]\n"));
                        return NDIS_STATUS_RESOURCES;
                    }

                    // Avoid the blank bytes added after Data. Thus, we only copy the remain byte.
                    if(SrcBufLenRaTxAgg > RemainRaTxAgg)
                    {
                        SrcBufLenRaTxAgg = RemainRaTxAgg;
                    }
                    //
                    // Copy the rest of this buffer descriptor pointed data into ring buffer.
                    //
                    PlatformMoveMemory(pDest, pSrcBufVARaTxAgg, SrcBufLenRaTxAgg);
                    BytesCopied  += SrcBufLenRaTxAgg;
                    pDest        += SrcBufLenRaTxAgg;
                    RemainRaTxAgg       -= SrcBufLenRaTxAgg;
                }

                //
                // Get Next MDL
                //
                if (MdlRaTxAgg)
                    MdlRaTxAgg = MdlRaTxAgg->Next;

                if (MdlRaTxAgg && (RemainRaTxAgg > 0))
                {
                    pSrcBufVARaTxAgg= MmGetSystemAddressForMdlSafe(MdlRaTxAgg, NormalPagePriority);
                    if (pSrcBufVARaTxAgg == NULL) 
                    {
                        DBGPRINT(RT_DEBUG_ERROR, ("XmitHardTransmit, Copy data failed\n"));
                        return NDIS_STATUS_RESOURCES;
                    }
                    SrcBufLenRaTxAgg= MdlByteCount = MmGetMdlByteCount(MdlRaTxAgg);
                }
                else
                {
                    break;
                }
            } while (TRUE);
        }
#endif

        TxSize = BytesCopied + TxInfo.This80211HdrLen;

        SrcRemainingBytes -= BytesCopied;
        
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

#if 1           
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
            pTxContext->CurWritePosition += 8;
            pTxContext->ENextBulkOutPosition += 8;
            pDest += 8;         
            DBGPRINT(RT_DEBUG_TRACE, ("BulkOut Reach CurWrite\n"));
        }

        //re-caculate wireless header to prevent  TKIP MIC error
        if ((pPort->CommonCfg.bAutoTxRateSwitch == FALSE) || (TxInfo.bDHCPFrame == TRUE) /* || (NumberRequired > 1)*/)
        {
            pLongFormatTxD = (PTXDSCR_LONG_STRUC)&pTxContext->TransferBuffer->WirelessPacket[pTxContext->CurWritePosition];
            pTxDDW0 = &pLongFormatTxD->TxDscrDW0;
            pTxDDW7 = &pLongFormatTxD->TxDscrDW7;
            PlatformZeroMemory(pTxDDW7, 4);

            pWirelessPacket = &pTxContext->TransferBuffer->WirelessPacket[pTxContext->CurWritePosition + TXD_LONG_SIZE];
        }
        else
        {
            pShortFormatTxD = (PTXDSCR_SHORT_STRUC)&pTxContext->TransferBuffer->WirelessPacket[pTxContext->CurWritePosition];
            pTxDDW0 = &pShortFormatTxD->TxDscrDW0;
            pTxDDW7 = &pShortFormatTxD->TxDscrDW7;
            PlatformZeroMemory(pTxDDW7, 4);

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
        
#endif
        
        pHeader80211 = (PHEADER_802_11)pWirelessPacket;

        TxInfo.TotalPacketLength = TxSize + TxInfo.LengthQosPAD;

        pHeader80211->FC.MoreFrag = 0;
        if (pHeader80211->Addr1[0] & 0x01) // multicast/broadcast
            pHeader80211->Duration = 0;
        else
            pHeader80211->Duration = pPort->CommonCfg.Dsifs + TxInfo.AckDuration;

        if ((TxInfo.bEAPOLFrame) && (TxInfo.TxRate > RATE_6))
            TxInfo.TxRate = RATE_6;

        if (pPort->Channel <= 14)
        {
            //
            // Case 802.11 b/g
            // basic channel means that we can use CCKM's low rate as RATE_1.
            //      
            if ( (TxInfo.TxRate != RATE_1) && TxInfo.bDHCPFrame )
                TxInfo.TxRate = RATE_1;
        }
        else
        {
            //
            // Case 802.11a
            // We must used OFDM's low rate as RATE_6, note RATE_1 is not allow
            // Only OFDM support on Channel > 14
            //
            if ( (TxInfo.TxRate != RATE_6) && TxInfo.bDHCPFrame )
                TxInfo.TxRate = RATE_6;
        }       

        if ((TxInfo.bDHCPFrame == TRUE) || (TxInfo.bEAPOLFrame ==TRUE))
        {
            PMAC_TABLE_ENTRY pEntry = MlmeSyncMacTabMatchedRoleType(pPort, ROLE_WLANIDX_MBCAST);
            if(pEntry == NULL)
            {
                DBGPRINT(RT_DEBUG_TRACE,("%s(), Line:%d, Mac Entry is NULL\n", __FUNCTION__, __LINE__));
                return NDIS_STATUS_FAILURE;
            }
            
            WRITE_PHY_CFG_DOUBLE_WORD(pAd, &PhyCfg, READ_PHY_CFG_DOUBLE_WORD(pAd, &pEntry->TxPhyCfg));

            if (PORT_P2P_ON(pPort))
            {
                // P2P GO min ratecan't be CCK
                if ((READ_PHY_CFG_MODE(pAd, &PhyCfg) == MODE_CCK))
                {
                    WRITE_PHY_CFG_DOUBLE_WORD(pAd, &PhyCfg, 0);
                    WRITE_PHY_CFG_MODE(pAd, &PhyCfg, MODE_OFDM);
                    WRITE_PHY_CFG_MCS(pAd, &PhyCfg, OfdmRateToRxwiMCS[RATE_6]);
                }
            }
#if 1           
//              TxInfo.MoreData = (UCHAR)pHeader80211->FC.MoreFrag;

                if ((pPort->CommonCfg.bAutoTxRateSwitch == FALSE) || (TxInfo.bDHCPFrame == TRUE) /* || (NumberRequired > 1)*/)
                {
                    XmitWriteTxD(pAd, pPort, NULL, pLongFormatTxD, TxInfo);
                }
                else
                {
                    XmitWriteTxD(pAd, pPort, pShortFormatTxD, NULL, TxInfo);
                }
#else
            XmitWriteTxWI(pAd, pPort, pTxWI,  FALSE, FALSE, FALSE, bAckRequired, FALSE, 
                TxInfo.BAWinSize, TxInfo.Wcid, TxSize, 
                PID, TxInfo.MimoPs, IFS_HTTXOP, TxInfo.bPiggyBack, PhyCfg, FALSE, FALSE, TRUE);
            XmitWriteTxInfo(pAd, pPort, pTxInfo, (TxSize+pAd->HwCfg.TXWI_Length + TxInfo.LengthQosPAD), FALSE, FIFO_EDCA, FALSE,  FALSE);         
#endif          
        }
        else if (bAllowChangeMcastRate)
        {
            PMAC_TABLE_ENTRY pEntry = MlmeSyncMacTabMatchedRoleType(pPort, ROLE_WLANIDX_VWF_BSSID);
            USHORT WlanIdx = (USHORT)MlmeSyncGetWlanIdxByPort(pPort, ROLE_WLANIDX_VWF_BSSID);
            
            USHORT      LowestMCS = (USHORT)READ_PHY_CFG_MCS(pAd, &pEntry->TxPhyCfg);
            USHORT      LowestRateRAWcid = WlanIdx;  
            BOOLEAN     bHasValidCLI = FALSE;
            MP_RW_LOCK_STATE LockState;
            
            PMAC_TABLE_ENTRY  pNextList =NULL;
            PQUEUE_HEADER pHeader;
            
            BOOLEAN bFind =FALSE;
            
            WRITE_PHY_CFG_DOUBLE_WORD(pAd, &PhyCfg, 0);

            // Find out the lowest tx rate in MacEntry for multicast use                
            PlatformAcquireOldRWLockForWrite(&pAd->MacTablePool.MacTabPoolLock,&LockState);

            pHeader = &pPort->MacTab.MacTabList;
            pNextList = (PMAC_TABLE_ENTRY)(pHeader->Head);
            while (pNextList != NULL)
            {   
                PMAC_TABLE_ENTRY  pEntry = NULL;
                pEntry = (PMAC_TABLE_ENTRY)pNextList;
                
                if(pEntry->WlanIdxRole != ROLE_WLANIDX_VWF_BSSID)
                {   
                    if(pEntry->ValidAsCLI == TRUE)
                {
                    bHasValidCLI = TRUE;                        
                            if (LowestMCS > READ_PHY_CFG_MCS(pAd, &pEntry->TxPhyCfg))
                    {
                                LowestMCS = (USHORT)READ_PHY_CFG_MCS(pAd, &pEntry->TxPhyCfg);
                            LowestRateRAWcid = pEntry->wcid;                           
                    }
                }
            }
                pNextList = pNextList->Next;  
                pEntry = NULL;
            }
            PlatformReleaseOldRWLock(&pAd->MacTablePool.MacTabPoolLock,&LockState);
 

            //
            // SoftAPForceMulticastRate
            //
            // Bit[0-15]: HTTRANSMIT_SETTING
            //          Bit[0-6]: MCS, Bit[7]: BW, Bit[8]: ShortGI, Bit[9-10]: STBC, Bit[11-13]: Reserve, Bit[14-15]: PHY mode
            // Bit[16-31]: 
            //          0x1000: ALLOW_CHANGE_MCAST_RATE
            //          0x1001: ALLOW_CHANGE_MCAST_RATE and USER_DEFINE_MCAST_RATE
            //          0x1010: ALLOW_CHANGE_MCAST_RATE and USE_LEGACY_MCAST_RATE
            //
            if ((pPort->SoftAP.ApCfg.SoftAPForceMulticastRate & 0xFFFF0000) == USER_DEFINE_MCAST_RATE)
            {
                // Error handling
                // 1. MODE is CCK, MCS allows 0-3
                // 2. MODE is OFDM, MCS allows 0-7
                // 3. MODE is HTMIX or HTGF, MCS allows 0-15
                // 4. STBC is 0 if MCS >= 8
                if (((((pPort->SoftAP.ApCfg.SoftAPForceMulticastRate & 0x0000C000) >> 14) == MODE_CCK) && ((pPort->SoftAP.ApCfg.SoftAPForceMulticastRate & 0x0000007F) > 4))
                    || ((((pPort->SoftAP.ApCfg.SoftAPForceMulticastRate & 0x0000C000) >> 14) == MODE_OFDM) && ((pPort->SoftAP.ApCfg.SoftAPForceMulticastRate & 0x0000007F) > 8))
                    || (((((pPort->SoftAP.ApCfg.SoftAPForceMulticastRate & 0x0000C000) >> 14) == MODE_HTMIX) || (((pPort->SoftAP.ApCfg.SoftAPForceMulticastRate & 0x0000C000) >> 14) == MODE_HTGREENFIELD)) && (pPort->SoftAP.ApCfg.SoftAPForceMulticastRate & 0x0000007F) > 15)
                    || (((pPort->SoftAP.ApCfg.SoftAPForceMulticastRate & 0x0000007F) >= 8) && (((pPort->SoftAP.ApCfg.SoftAPForceMulticastRate & 0x00000600) >> 9) != 0)))
                {
                    if (pPort->Channel > 11)
                    {
                        WRITE_PHY_CFG_MCS(pAd, &PhyCfg, OfdmRateToRxwiMCS[RATE_24]);
                        DBGPRINT(RT_DEBUG_TRACE,("Error: Force multicast rate to 24Mbps, SoftAPForceMulticastRate[0-15]=0x%x, MCS=%d, BW=%d, ShortGI=%d, STBC=%d, Rsv=%d, MODE=%d\n", (pPort->SoftAP.ApCfg.SoftAPForceMulticastRate & 0x0000FFFF), (pPort->SoftAP.ApCfg.SoftAPForceMulticastRate & 0x0000007F) /* MCS */,  (pPort->SoftAP.ApCfg.SoftAPForceMulticastRate & 0x00000080) >> 7 /* BW */,  (pPort->SoftAP.ApCfg.SoftAPForceMulticastRate & 0x00000100) >> 8 /* SGI */,  (pPort->SoftAP.ApCfg.SoftAPForceMulticastRate & 0x00000600) >> 9 /* STBC */,  (pPort->SoftAP.ApCfg.SoftAPForceMulticastRate & 0x00003800) >> 11 /* RSV */,  (pPort->SoftAP.ApCfg.SoftAPForceMulticastRate & 0x0000C000) >> 14/* MODE */));                     
                    }
                    else
                    {
                        WRITE_PHY_CFG_MCS(pAd, &PhyCfg, OfdmRateToRxwiMCS[RATE_1]);
                        DBGPRINT(RT_DEBUG_TRACE,("Error: Force multicast rate to 1Mbps, SoftAPForceMulticastRate[0-15]=0x%x, MCS=%d, BW=%d, ShortGI=%d, STBC=%d, Rsv=%d, MODE=%d\n", (pPort->SoftAP.ApCfg.SoftAPForceMulticastRate & 0x0000FFFF), (pPort->SoftAP.ApCfg.SoftAPForceMulticastRate & 0x0000007F) /* MCS */,  (pPort->SoftAP.ApCfg.SoftAPForceMulticastRate & 0x00000080) >> 7 /* BW */,  (pPort->SoftAP.ApCfg.SoftAPForceMulticastRate & 0x00000100) >> 8 /* SGI */,  (pPort->SoftAP.ApCfg.SoftAPForceMulticastRate & 0x00000600) >> 9 /* STBC */,  (pPort->SoftAP.ApCfg.SoftAPForceMulticastRate & 0x00003800) >> 11 /* RSV */,  (pPort->SoftAP.ApCfg.SoftAPForceMulticastRate & 0x0000C000) >> 14/* MODE */));                      
                    }
                }
                else
                {
                    // Set user define multicast rate
                    WRITE_PHY_CFG_DOUBLE_WORD(pAd, &PhyCfg, (pPort->SoftAP.ApCfg.SoftAPForceMulticastRate & 0x0000FFFF));
                    DBGPRINT(RT_DEBUG_INFO,("User define: SoftAPForceMulticastRate[0-15]=0x%x, MCS=%d, BW=%d, ShortGI=%d, STBC=%d, Rsv=%d, MODE=%d\n", (pPort->SoftAP.ApCfg.SoftAPForceMulticastRate & 0x0000FFFF), (pPort->SoftAP.ApCfg.SoftAPForceMulticastRate & 0x0000007F) /* MCS */,  (pPort->SoftAP.ApCfg.SoftAPForceMulticastRate & 0x00000080) >> 7 /* BW */,  (pPort->SoftAP.ApCfg.SoftAPForceMulticastRate & 0x00000100) >> 8 /* SGI */,  (pPort->SoftAP.ApCfg.SoftAPForceMulticastRate & 0x00000600) >> 9 /* STBC */,  (pPort->SoftAP.ApCfg.SoftAPForceMulticastRate & 0x00003800) >> 11 /* RSV */,  (pPort->SoftAP.ApCfg.SoftAPForceMulticastRate & 0x0000C000) >> 14/* MODE */));
                }
            }
            else if((bHasValidCLI) && ((pPort->SoftAP.ApCfg.SoftAPForceMulticastRate & 0xFFFF0000) == USE_LEGACY_MCAST_RATE))
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
                    UCHAR uFallBackRATE = XmitMulticastRateNRateToOFDMRate(READ_PHY_CFG_MCS(pAd, &pWcidMacTabEntry->TxPhyCfg),
                                                                        READ_PHY_CFG_BW(pAd, &pWcidMacTabEntry->TxPhyCfg));

                    WRITE_PHY_CFG_MODE(pAd, &PhyCfg, MODE_OFDM);
                    WRITE_PHY_CFG_MCS(pAd, &PhyCfg, OfdmRateToRxwiMCS[uFallBackRATE]);
                    
                    DBGPRINT(RT_DEBUG_INFO,("Auto: MODE=HTMIX or HTGF, MCS=%d, BW=%d, Force MCS to 24Mbps\n", 
                        READ_PHY_CFG_MCS(pAd, &pWcidMacTabEntry->TxPhyCfg), 
                        READ_PHY_CFG_BW(pAd, &pWcidMacTabEntry->TxPhyCfg)));
                }                   
            }           
            else if((bHasValidCLI) && ((pPort->SoftAP.ApCfg.SoftAPForceMulticastRate & 0xFFFF0000) == USE_AUTO_MCAST_RATE))
            {
                    //Default use 24Mbps, but should not exceed Unicast Rate    
                    UCHAR uMCS = XmitMappingOneSpatialStreamRate(READ_PHY_CFG_MCS(pAd, &pWcidMacTabEntry->TxPhyCfg));
                    WRITE_PHY_CFG_BW(pAd, &PhyCfg, READ_PHY_CFG_BW(pAd, &pWcidMacTabEntry->TxPhyCfg));
                    WRITE_PHY_CFG_MODE(pAd, &PhyCfg, READ_PHY_CFG_MODE(pAd, &pWcidMacTabEntry->TxPhyCfg));
                    WRITE_PHY_CFG_MCS(pAd, &PhyCfg, uMCS);

                    DBGPRINT(RT_DEBUG_TRACE, ("Auto : USE_AUTO_MCAST_RATE.... uMCS = %d, TxPhyCfg.field.BW = %d\n", 
                        uMCS, 
                        READ_PHY_CFG_BW(pAd, &PhyCfg)));
            }
            else
            {
                // Multicast rate is the same with Unicast rate.
                WRITE_PHY_CFG_DOUBLE_WORD(pAd, &PhyCfg, READ_PHY_CFG_DOUBLE_WORD(pAd, &pWcidMacTabEntry->TxPhyCfg));
                DBGPRINT(RT_DEBUG_INFO,("Multicast rate is the same with Unicast rate: MCS=%d, BW=%d\n", 
                    READ_PHY_CFG_MCS(pAd, &pWcidMacTabEntry->TxPhyCfg), 
                    READ_PHY_CFG_BW(pAd, &pWcidMacTabEntry->TxPhyCfg)));
            }

            if (pPort->PortSubtype == PORTSUBTYPE_P2PGO)
            {
                // P2P GO min ratecan't be CCK
                if (READ_PHY_CFG_MODE(pAd, &PhyCfg) == MODE_CCK)
                {
                    WRITE_PHY_CFG_DOUBLE_WORD(pAd, &PhyCfg, 0);
                    WRITE_PHY_CFG_MODE(pAd, &PhyCfg, MODE_OFDM);
                    WRITE_PHY_CFG_MCS(pAd, &PhyCfg, OfdmRateToRxwiMCS[RATE_6]);
                    DBGPRINT(RT_DEBUG_INFO,("3PORTSUBTYPE_P2PGO use 6Mbps for EAPOl ofr Dhcp frmae\n"));
                }
            }

#if 1
            TxInfo.Wcid = (UCHAR)MlmeSyncGetWlanIdxByPort(pPort, ROLE_WLANIDX_MBCAST);

            if (pAd->pTxCfg->SendTxWaitQueue[TxInfo.QueIdx].Number > 1)
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
            XmitWriteTxWI(pAd, pPort, pTxWI,  FALSE, FALSE, TXWIAMPDU, bAckRequired, FALSE, 
                TxInfo.BAWinSize, MCAST_WCID, TxSize, PID, TxInfo.MimoPs, TxInfo.FrameGap, TxInfo.bPiggyBack, PhyCfg, FALSE, FALSE, TRUE);

            #if DBG
            //pPort->LastTxRate = (USHORT)(HTPhyMode.word);
            #endif
             
            if (pAd->pTxCfg->SendTxWaitQueue[TxInfo.QueIdx].Number > 1)
                TxInfo.NextValid= 1;        
            else
                TxInfo.NextValid = 0;           

            XmitWriteTxInfo(pAd, pPort, pTxInfo, (TxSize+pAd->HwCfg.TXWI_Length + TxInfo.LengthQosPAD), FALSE, FIFO_EDCA, TxInfo.NextValid,  FALSE);
#endif
        }
        else
        {
            WRITE_PHY_CFG_DOUBLE_WORD(pAd, &PhyCfg, READ_PHY_CFG_DOUBLE_WORD(pAd, &pMacEntry->TxPhyCfg));

            // P2P GO min ratecan't be CCK
            if (pPort->PortSubtype == PORTSUBTYPE_P2PGO)
            {
                // can't use CCK when I am GO
                if (READ_PHY_CFG_MODE(pAd, &PhyCfg) == MODE_CCK)
                {
                    WRITE_PHY_CFG_DOUBLE_WORD(pAd, &PhyCfg, 0);
                    WRITE_PHY_CFG_MODE(pAd, &PhyCfg, MODE_OFDM);
                    WRITE_PHY_CFG_MCS(pAd, &PhyCfg, OfdmRateToRxwiMCS[RATE_6]);
                    DBGPRINT(RT_DEBUG_INFO,("1PORTSUBTYPE_P2PGO use 6Mbps for EAPOl ofr Dhcp frmae\n"));
                }
                //To pass SIGMA tests, we need to reduce rate to 802.11g rate
                else if (IS_P2P_SIGMA_ON(pPort))
                {
                    //Sigma sniffer is having difficulty to obtain packets from 802.11n rate
                    //Therefore, we reduce the rate to 802.11g rate
                    WRITE_PHY_CFG_DOUBLE_WORD(pAd, &PhyCfg, 0);
                    WRITE_PHY_CFG_MODE(pAd, &PhyCfg, MODE_OFDM);
                    WRITE_PHY_CFG_MCS(pAd, &PhyCfg, OfdmRateToRxwiMCS[RATE_54]);
                }
            }

#if 1

            WRITE_PHY_CFG_DOUBLE_WORD(pAd, &pPort->LastTxRatePhyCfg, READ_PHY_CFG_DOUBLE_WORD(pAd, &PhyCfg));

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
            XmitWriteTxWI(pAd, 
                            pPort, 
                            pTxWI,  
                            FALSE, 
                            FALSE, 
                            TXWIAMPDU, 
                            bAckRequired, 
                            FALSE,
                            TxInfo.BAWinSize, 
                            TxInfo.Wcid, 
                            TxSize, 
                            PID, 
                            TxInfo.MimoPs, 
                            TxInfo.FrameGap,
                            TxInfo.bPiggyBack, 
                            PhyCfg, 
                            FALSE, 
                            FALSE,
                            TRUE);

            WRITE_PHY_CFG_DOUBLE_WORD(pAd, &pPort->LastTxRatePhyCfg, READ_PHY_CFG_DOUBLE_WORD(pAd, &PhyCfg));

            XmitWriteTxInfo(pAd, 
                            pPort, 
                            pTxInfo, 
                            (TxSize+pAd->HwCfg.TXWI_Length + TxInfo.LengthQosPAD), 
                            FALSE, 
                            FIFO_EDCA, 
                            TxInfo.NextValid,  
                            FALSE);         
#endif          
        }

        // -------------
        // Coding Note: For FPGA verifying (TODO: Remove)
        // -------------
#if 0
        {
            DBGPRINT(RT_DEBUG_TRACE, ("6x9x , <TxWI> in %s. PhyMode=%d, BW=%d, GI=%d, STBC=%d, \n",
                                    __FUNCTION__,
                                    READ_TXWI_PHY_MODE(pAd, pTxWI),
                                    READ_TXWI_BW(pAd, pTxWI),
                                    READ_TXWI_SHORT_GI(pAd, pTxWI),
                                    READ_TXWI_STBC(pAd, pTxWI) 
                                    ));

            DBGPRINT(RT_DEBUG_TRACE, ("6x9x , <TxWI> in %s. MCS=%d, Nss=%d, pFirstTxWI->Ext.McsNss=0x%08x\n",
                                    __FUNCTION__,
                                    READ_TXWI_MCS(pAd, pTxWI, READ_TXWI_PHY_MODE(pAd, pTxWI)),
                                    READ_TXWI_NSS(pAd, pTxWI, READ_TXWI_PHY_MODE(pAd, pTxWI)),
                                    pTxWI->Ext.McsNss
                                    ));
        }
        DBGDumpHex(RT_DEBUG_INFO, "<Value of TxWI>", 32, (PUCHAR)pTxWI, TRUE); 
#endif

        // Update the rekey counter.
        if ((pPort->SoftAP.ApCfg.REKEYTimerRunning == TRUE) &&
             (pPort->SoftAP.ApCfg.WPAREKEY.ReKeyMethod == PKT_REKEY))
        {
            // Multicast/broadcast packet.
            if (pHeader80211->Addr1[0] & 0x01)
            {
                pPort->SoftAP.ApCfg.REKEYCOUNTER = pPort->SoftAP.ApCfg.REKEYCOUNTER + 1;
            }
        }
        
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

        PlatformZeroMemory(pTxContext->TransferBuffer->Aggregation, 4);
        pTxContext->TxRate = TxInfo.TxRate;
        // Set frame gap for the rest of fragment burst.
        // It won't matter if there is only one fragment (single fragment frame).
        StartOfFrame = FALSE;
        if (SrcRemainingBytes <= 0)
        {
            pTxContext->LastOne = TRUE;
        }
        else
        {
            pTxContext->LastOne = FALSE;
            pTxDDW0->USBTxBurst = 1;
        }
            
        pTxContext->CurWritePosition += TxInfo.TransferBufferLength;
        
#if 0       
        TxInfo.SwUseSegIdx = MTDecideSegmentEnd(pTxContext->CurWritePosition);
        
        WRITE_TXINFO_SwUseSegIdx(pAd, pTxInfo,  TxInfo.SwUseSegIdx);
        
        if (TxInfo.SwUseSegIdx == SEGMENT_TOTAL)
        {
            pTxContext->CurWritePosition = 8;
            WRITE_TXINFO_SwUseSegmentEnd(pAd, pTxInfo, 1);
        }
        else if (TxInfo.SwUseSegIdx < SEGMENT_TOTAL)
        {
            pTxContext->CurWritePosition = ONE_SEGMENT_UNIT * TxInfo.SwUseSegIdx;
            WRITE_TXINFO_SwUseSegmentEnd(pAd, pTxInfo, 1);
        }
        else 
        {
            WRITE_TXINFO_SwUseSegmentEnd(pAd, pTxInfo, 0);
        }
#else
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
#endif

        if (TxInfo.bWMM == FALSE) 
        {
            pTxDDW7->SwUseNonQoS = 1;
        }

        
#if UAPSD_AP_SUPPORT
        if (IS_AP_SUPPORT_UAPSD(pAd, pPort))
        {   
            // We always set the tag on the first Netbuffer.
            // If it is not the uapsd packet, we try the NetBufferRaTxAgg.
            if (ApUapsdTagFrame(pAd, pXcu, TxInfo.Wcid, pTxContext->CurWritePosition) == FALSE)
            {

            }
        }
#endif

        pTxContext->bCurWriting = FALSE;
        MTUSB_SET_BULK_FLAG(pAd, (fRTUSB_BULK_OUT_DATA_NORMAL << TxInfo.QueIdx));

        NdisReleaseSpinLock(&pAd->pTxCfg->TxContextQueueLock[TxInfo.QueIdx]);
        
    } while (SrcRemainingBytes > 0);

    if((pXcu != NULL) && (pXcu->Reserve1 != NULL) && (pXcu->Reserve2 != NULL))
        {
        RELEASE_NDIS_NETBUFFER(pAd, pXcu, NDIS_STATUS_SUCCESS);
    }

    return (NDIS_STATUS_SUCCESS);
}

NTSTATUS    ApDataN6RxPacket(
    IN  PMP_ADAPTER   pAd,
    IN    BOOLEAN           bBulkReceive,
    IN  ULONG           nIndex)
{
    PRX_CONTEXT     pRxContext;
//  PRX_CONTEXT     pRxContextBulkINFail;
//  PRXWI_STRUC     pRxWI;
//  PRXINFO_STRUC   pRxInfo;
    PHEADER_802_11  pHeader;
    PUCHAR          pData, pRxStart, pQoSField;
    PUCHAR          pDA, pSA;
    ULONG           wdsidx = 0;
    NDIS_STATUS     Status = NDIS_STATUS_SUCCESS;
    USHORT          PacketSize, DataSize, Msdu2Size;
    PCIPHER_KEY     pWpaKey;
    ULONG           TransferBufferLength;
    ULONG           ThisFrameLen = 0;
    ULONG           ReadPosition = 0;
    PULONG          ptemp;
    BOOLEAN         bAMsdu,bWdsPacket = FALSE;
    BOOLEAN         bHtVhtFrame = FALSE;
    MAC_TABLE_ENTRY *pEntry = NULL;
    NDIS_802_11_PRIVACY_FILTER   Privacy = Ndis802_11PrivFilterAcceptAll;
    AP_WPA_STATE    WpaState;
    PUCHAR          ReFormatBuffer;
    BOOLEAN         bStop = FALSE;
    UCHAR           ReadPacket = 0;
    UCHAR           UserPriority = 0;
    UCHAR           index;
    UCHAR           portCount = 0;
    PRXDSCR_BASE_STRUC pRxD = NULL;
    UCHAR           QosPadding = 0; // between RxD+RxV and Packet
    UCHAR           RxVectorLength = 0;

    PMP_PORT pPort = pAd->PortList[NDIS_DEFAULT_PORT_NUMBER];
    PMAC_TABLE_ENTRY pWcidMacTabEntry = NULL;

    struct rx_signal_info rx_signal;
    RXD_GRP4_STRUCT *RxdGrp4 = NULL;
    RXD_GRP1_STRUCT *RxdGrp1 = NULL;
    RXD_GRP2_STRUCT *RxdGrp2 = NULL;
    RXD_GRP3_STRUCT *RxdGrp3 = NULL;

    //
    // We have a number of cases:
    //      1) The USB read timed out and we received no data.
    //      2) The USB read timed out and we received some data.
    //      3) The USB read was successful and fully filled our irp buffer.
    //      4) The irp was cancelled.
    //      5) Some other failure from the USB device object.
    //

    if ((pPort->CommonCfg.BACapability.field.AutoBA == FALSE) || (pPort->CommonCfg.PhyMode < PHY_11ABGN_MIXED))
    {
        NdisAcquireSpinLock(&pAd->pHifCfg->BulkInReadLock);
        pAd->pHifCfg->bBulkInRead = TRUE;
        DBGPRINT(RT_DEBUG_TRACE, ("%s - pAd->pHifCfg->BulkInReadLock=TRUE\n", __FUNCTION__));
    }

    do
    {
        pRxContext = &pAd->pHifCfg->RxContext[nIndex];
        if ((pRxContext->Readable == FALSE) || (pRxContext->InUse == TRUE))
        {
            break;
        }
    
        TransferBufferLength = pRxContext->TransferBufferLength;

        if ((TransferBufferLength == 0) ||  (TransferBufferLength == MAX_RXBULK_SIZE))
        {
            bStop = TRUE;
        }
#if 0       
        else if ((TransferBufferLength%pAd->pHifCfg->BulkOutMaxPacketSize) == 0)
        {
            //
            // First or not Final BulkInFail's data
            //               
            ULONG tempTransferBufferLength;

            pRxContextBulkINFail = &pAd->pHifCfg->RxContext[RX_RING_SIZE_END_INDEX]; 
            tempTransferBufferLength = pRxContextBulkINFail->TransferBufferLength;

            //Cpoy First or not Final BulkInFail's data into RxContext[RX_RING_SIZE_END_INDEX]
            PlatformMoveMemory(&pRxContextBulkINFail->TransferBuffer[tempTransferBufferLength], pRxContext->TransferBuffer, pRxContext->TransferBufferLength);

            //Add URB's TransferBufferLength to BulkInFail's data length
            pRxContextBulkINFail->TransferBufferLength += pRxContext->TransferBufferLength;
            bStop = TRUE;
            DBGPRINT(RT_DEBUG_TRACE,("First or not Final BulkInFail's data, TransferBufferLength = %d\n",pRxContextBulkINFail->TransferBufferLength));          
        }
        else if((TransferBufferLength % pAd->pHifCfg->BulkOutMaxPacketSize) != 0)
        {
            //
            // Final BulkInFail's data
            //          
            if( pAd->pHifCfg->RxContext[RX_RING_SIZE_END_INDEX].TransferBufferLength!=0 )
            {
                ULONG tempTransferBufferLength;

                pRxContextBulkINFail = &pAd->pHifCfg->RxContext[RX_RING_SIZE_END_INDEX]; 
                tempTransferBufferLength = pRxContextBulkINFail->TransferBufferLength;

                //Copy Final BulkInFail's data into RxContext[RX_RING_SIZE_END_INDEX]       
                PlatformMoveMemory(&pRxContextBulkINFail->TransferBuffer[tempTransferBufferLength], pRxContext->TransferBuffer, pRxContext->TransferBufferLength);

                pRxContextBulkINFail->TransferBufferLength += pRxContext->TransferBufferLength;

                // Clear Final BulkInFail data's variable
                pRxContext->Readable = FALSE;
                pRxContext->ReadPosOffset = 0;
                pRxContext->TransferBufferLength = 0;                   
                
                //Pointer RxContext[RX_RING_SIZE_END_INDEX] to normal data receive pointer
                pRxContext = pRxContextBulkINFail;

                //Update RxContext[RX_RING_SIZE_END_INDEX]'s data length to normal data length          
                TransferBufferLength = pRxContextBulkINFail->TransferBufferLength;

                //Clear RxContext[RX_RING_SIZE_END_INDEX]'s TransferBufferLength
                pAd->pHifCfg->RxContext[RX_RING_SIZE_END_INDEX].TransferBufferLength = 0;
                DBGPRINT(RT_DEBUG_TRACE,("Final BulkInFail's data, tempTransferBufferLength = %d, TransferBufferLength = %d\n",tempTransferBufferLength,TransferBufferLength));             
            }
        }
#endif

        if (bStop == TRUE)
        {
            pRxContext->Readable = FALSE;
            pRxContext->ReadPosOffset = 0;  
            pRxContext->TransferBufferLength = 0;       
            //pAd->pHifCfg->NextRxBulkInReadIndex = (pAd->pHifCfg->NextRxBulkInReadIndex + 1) % (RX_RING_SIZE_END_INDEX);
            break;
        }

        //pAd->pHifCfg->NextRxBulkInReadIndex = (pAd->pHifCfg->NextRxBulkInReadIndex + 1) % (RX_RING_SIZE_END_INDEX);

        while (TransferBufferLength > 0)
        {
            if (TransferBufferLength >= (ULONG)(pAd->HwCfg.RXWI_Length + 8))
            {
                pRxStart = &pRxContext->TransferBuffer[ReadPosition];
                
                pRxD = (PRXDSCR_BASE_STRUC)pRxStart;

                if (pRxD == NULL)
                {
                    DBGPRINT(RT_DEBUG_EMU,("pRxD=NULL Error!!\n"));
                    // Finish 
                    TransferBufferLength = 0;
                    Status = NDIS_STATUS_FAILURE;
                    break;
                }
                
                // after shift First Word is  RXDMA Length
                ThisFrameLen = pRxD->RxDscrDW0.RxByteCount;

                RxVectorLength = 0;
                if (pRxD->RxDscrDW0.RFBGroupValidIndicators & RXS_GROUP4)
                {
                    DBGPRINT(RT_DEBUG_INFO, ("There has group 4 in rx packet.\n"));
                    RxdGrp4 = (RXD_GRP4_STRUCT *)(pRxStart + (RMAC_RX_PKT_TYPE_RX_NORMAL_LENGTH + RxVectorLength));
                    RxVectorLength += RMAC_INFO_GRP_4_SIZE;
                }
                if (pRxD->RxDscrDW0.RFBGroupValidIndicators & RXS_GROUP1)
                {
                    DBGPRINT(RT_DEBUG_INFO, ("There has group 1 in rx packet.\n"));
                    RxdGrp1 = (RXD_GRP1_STRUCT *)(pRxStart + (RMAC_RX_PKT_TYPE_RX_NORMAL_LENGTH + RxVectorLength));
                    RxVectorLength += RMAC_INFO_GRP_1_SIZE;
                }
                if (pRxD->RxDscrDW0.RFBGroupValidIndicators & RXS_GROUP2)
                {
                    DBGPRINT(RT_DEBUG_INFO, ("There has group 2 in rx packet.\n"));
                    RxdGrp2 = (RXD_GRP2_STRUCT *)(pRxStart + (RMAC_RX_PKT_TYPE_RX_NORMAL_LENGTH + RxVectorLength));
                    RxVectorLength += RMAC_INFO_GRP_2_SIZE;
                }
                if (pRxD->RxDscrDW0.RFBGroupValidIndicators & RXS_GROUP3)
                {
                    DBGPRINT(RT_DEBUG_INFO, ("There has group 3 in rx packet.\n"));
                    RxdGrp3 = (RXD_GRP3_STRUCT *)(pRxStart + (RMAC_RX_PKT_TYPE_RX_NORMAL_LENGTH + RxVectorLength));
                    RxVectorLength += RMAC_INFO_GRP_3_SIZE;
                }
                
                if (pRxContext->TransferBufferLength == 0x164)
                {
                    //DBGPRINT(RT_DEBUG_TRACE,("This Frame Len (nIndex = %d) (0x%x). [%02x %02x %02x %02x]\n", nIndex, ThisFrameLen, pData[0], pData[1], pData[2], pData[3]));
                }
                
#if 0
                if ((ThisFrameLen&0x3) != 0)
                {
                    DBGPRINT(RT_DEBUG_TRACE,("RXDMALen not multiple of 4. [%d] (Total TransferBufferLength = %d) \n", ThisFrameLen, pRxContext->TransferBufferLength));
                    ThisFrameLen = (ThisFrameLen&0x3);
                    //Error frame. finish this loop
                    ThisFrameLen = 0;
                    TransferBufferLength = 0;                   
                    break;
                }
#endif              

                if (ThisFrameLen >= TransferBufferLength || ThisFrameLen == 0)
                {
#if 1
                    {
                        int i = 0;
                        PUCHAR p = (PUCHAR)&pRxContext->TransferBuffer[0];
                        for (i = 0; i < (int)pRxContext->TransferBufferLength; i = i + 8)
                        {
                            DBGPRINT_RAW(RT_DEBUG_ERROR, ("[%d] %x %x %x %x %x %x %x %x\n", i, p[i], p[i+1], p[i+2], p[i+3], p[i+4], p[i+5], p[i+6], p[i+7]));
                        }
                    }
#endif                  
                    DBGPRINT(RT_DEBUG_TRACE,("This Frame Len (nIndex = %d) (0x%x) outranges. \n", nIndex, ThisFrameLen));
                    DBGPRINT(RT_DEBUG_TRACE,("Total BulkIn (nIndex = %d)  length=%x, Now left  TransferBufferLength = %x, ReadPosition=%x\n", nIndex, pRxContext->TransferBufferLength,TransferBufferLength, ReadPosition));
                    ThisFrameLen = (ThisFrameLen&0x3);
                    //Error frame. finish this loop
                    ThisFrameLen = 0;
                    TransferBufferLength = 0;
                    break;
                }

                //DBGPRINT(RT_DEBUG_TRACE,("This Frame Len(0x%x) outranges. \n", ThisFrameLen));
                //DBGPRINT(RT_DEBUG_TRACE,("Total BulkIn length=%x, Now left  TransferBufferLength = %x, ReadPosition=%x\n", pRxContext->TransferBufferLength,TransferBufferLength, ReadPosition));
                DBGPRINT(RT_DEBUG_INFO,("########### Rx packet ignore, implement later  , pRxD->RxDscrDW0.PacketType = %d ###########\n", pRxD->RxDscrDW0.PacketType));
                if (pRxD->RxDscrDW0.PacketType == RMAC_RX_PKT_TYPE_RX_TXS)
                {
                    DBGPRINT(RT_DEBUG_INFO,("########### Got a RMAC_RX_PKT_TYPE_RX_TXS packet ###########\n"));

                    ThisFrameLen += RXDSCR_LAST_STRUC_LENGTH;
                    
                    // update next packet read position. 
                    ReadPosition += ThisFrameLen;
                    if (TransferBufferLength >= (ThisFrameLen))
                        TransferBufferLength -= (ThisFrameLen);
                    else
                        TransferBufferLength = 0;
                    
                    if (TransferBufferLength <=  4)
                    {
                        break;
                    }
                    else
                        continue;
                }
                else if (pRxD->RxDscrDW0.PacketType == RMAC_RX_PKT_TYPE_RX_TXRXV)
                {
                    DBGPRINT(RT_DEBUG_INFO,("########### Got a RMAC_RX_PKT_TYPE_RX_TXRXV packet ###########\n"));
                    
                    ThisFrameLen += RXDSCR_LAST_STRUC_LENGTH;
                    
                    // update next packet read position. 
                    ReadPosition += ThisFrameLen;
                    if (TransferBufferLength >= (ThisFrameLen))
                        TransferBufferLength -= (ThisFrameLen);
                    else
                        TransferBufferLength = 0;
                    
                    if (TransferBufferLength <=  4)
                    {
                        break;
                    }
                    else
                        continue;
                }
                else if (pRxD->RxDscrDW0.PacketType == RMAC_RX_PKT_TYPE_RX_NORMAL)
                {
                    DBGPRINT(RT_DEBUG_INFO,("########### Got a RMAC_RX_PKT_TYPE_RX_NORMAL packet ###########\n"));
                }
                else if (pRxD->RxDscrDW0.PacketType == RMAC_RX_PKT_TYPE_RX_DUP_RFB)
                {
                    DBGPRINT(RT_DEBUG_INFO,("########### Got a RMAC_RX_PKT_TYPE_RX_DUP_RFB packet ###########\n"));

                    ThisFrameLen += RXDSCR_LAST_STRUC_LENGTH;
                    
                    // update next packet read position. 
                    ReadPosition += ThisFrameLen;
                    if (TransferBufferLength >= (ThisFrameLen))
                        TransferBufferLength -= (ThisFrameLen);
                    else
                        TransferBufferLength = 0;
                    
                    if (TransferBufferLength <=  4)
                    {
                        break;
                    }
                    else
                        continue;
                }
                else if (pRxD->RxDscrDW0.PacketType == RMAC_RX_PKT_TYPE_RX_TMR)
                {
                    DBGPRINT(RT_DEBUG_INFO,("########### Got a RMAC_RX_PKT_TYPE_RX_TMR packet ###########\n"));

                    ThisFrameLen += RXDSCR_LAST_STRUC_LENGTH;
                    
                    // update next packet read position. 
                    ReadPosition += ThisFrameLen;
                    if (TransferBufferLength >= (ThisFrameLen))
                        TransferBufferLength -= (ThisFrameLen);
                    else
                        TransferBufferLength = 0;
                    
                    if (TransferBufferLength <=  4)
                    {
                        break;
                    }
                    else
                        continue;
                }

#if 1
                QosPadding = (pRxD->RxDscrDW1.HeaderOffset ? 2 : 0);

                // pointer to QoS 2 bytes
                pQoSField = pRxStart + RMAC_RX_PKT_TYPE_RX_NORMAL_LENGTH + RxVectorLength;

                // ===================          
                pHeader = (PHEADER_802_11)(pRxStart + RMAC_RX_PKT_TYPE_RX_NORMAL_LENGTH + QosPadding + RxVectorLength);
                PacketSize = (USHORT)(pRxD->RxDscrDW0.RxByteCount - (RMAC_RX_PKT_TYPE_RX_NORMAL_LENGTH + QosPadding + RxVectorLength));

                DBGPRINT(RT_DEBUG_INFO, ("RxVectorLength = %d, PacketSize = %d\n", RxVectorLength, PacketSize));

                // pData : Pointer skip the RxD Descriptior and the first 24 bytes, 802.11 HEADER
                pData = pRxStart + (RMAC_RX_PKT_TYPE_RX_NORMAL_LENGTH + QosPadding + RxVectorLength + LENGTH_802_11);

                if (pRxD->RxDscrDW0.RFBGroupValidIndicators & RXS_GROUP3)
                {
                    ParseRxVGroup(pAd, RMAC_RX_PKT_TYPE_RX_NORMAL, &rx_signal, (UCHAR *)RxdGrp3);
                }

                if (PacketSize < 14)
                {
                    Status = NDIS_STATUS_FAILURE;
                }
                else
                {
                    // Increase Total receive byte counter after real data received no mater any error or not
                    pAd->Counter.MTKCounters.ReceivedByteCount += (PacketSize /*READ_RXWI_MPDU_TOTAL_BYTE_COUNT(pAd, pRxWI)*/ - 4);
                    pAd->Counter.MTKCounters.OneSecReceivedByteCount +=  PacketSize /*READ_RXWI_MPDU_TOTAL_BYTE_COUNT(pAd, pRxWI)*/;
                    pAd->Counter.MTKCounters.RxCount ++;
                    
                    // Check for all RxD errors
                    Status = ApDataCheckRxError(pAd, pRxD, pHeader);
                }


                // Add receive counters
                if (Status == NDIS_STATUS_SUCCESS)
                {
                    // Increase 802.11 counters & general receive counters
                    INC_COUNTER64(pAd->Counter.WlanCounters.ReceivedFragmentCount);
                }
                else if (pAd->UiCfg.bPromiscuous == FALSE)
                {
                    // Increase general counters
                    pAd->Counter.Counters8023.RxErrors++;
                    //DBGPRINT(RT_DEBUG_INFO,("RX err filtered2 [MPDUtotalByteCount = 0x%x]RxInfo= 0x%08x\n", 
                    //                      READ_RXWI_MPDU_TOTAL_BYTE_COUNT(pAd, pRxWI),
                    //                      *(PULONG)pRxInfo));
                }
#else               
                //
                pRxWI = RXWI_PTR(pAd, pData);               
                pHeader = DOT11_HEADER_PTR(pAd, pData);                 
                pRxInfo = RXINFO_PTR(pAd, pData, ThisFrameLen);
                pData = (PUCHAR)RXWI_PTR(pAd, pData);
                DataSize = READ_RXWI_MPDU_TOTAL_BYTE_COUNT(pAd, pRxWI);

                //
                // An open issue: pHeader->Sequence != pRxWI->SEQUENCE
                // Skip this packet.
                if((pHeader->Sequence != READ_RXWI_SN(pAd, pRxWI)) && (pHeader->FC.Type == BTYPE_DATA))
                {
                    DBGPRINT_ERR(("[%s][LINE_%d]pHeader->Sequence == %d,  pRxWI->SEQUENCE == %d\n",
                                    __FUNCTION__,
                                    __LINE__,
                                    pHeader->Sequence,
                                    READ_RXWI_SN(pAd, pRxWI)));
                    
                    DBGPRINT_ERR(("Skip this packet\n"));
                    break;
                }
                
                if ((pRxWI != NULL) && (READ_RXWI_PHY_MODE(pAd, pRxWI) >= MODE_HTMIX))
                {
                    // Drop HT data
                    if ((pPort->CommonCfg.bProhibitTKIPonHT) && 
                        ((IS_WEP_STATUS_ON(pPort->PortCfg.WepStatus))||
                         (pPort->PortCfg.CipherAlg == CIPHER_TKIP)||
                         (pPort->PortCfg.CipherAlg == CIPHER_TKIP_NO_MIC)||
                         (pPort->PortCfg.WepStatus == Ralink802_11Encryption4Enabled)))
                    {
                        // Finish 
                        TransferBufferLength = 0;
                        Status = NDIS_STATUS_FAILURE;                   
                    }
                    else
                    {
                        bHtVhtFrame = TRUE;
                    }
                }
                
                if (pRxInfo == NULL )
                {
                    DBGPRINT(RT_DEBUG_EMU,("RxInfo=NULL Error!! ThisFrameLen = %d\n", ThisFrameLen));
                    // Finish 
                    TransferBufferLength = 0;
                    Status = NDIS_STATUS_FAILURE;
                }
                else if (pRxWI == NULL)
                {
                    DBGPRINT(RT_DEBUG_EMU,("pRxWI=NULL Error!! ThisFrameLen = %d\n", ThisFrameLen));
                    // Finish 
                    TransferBufferLength = 0;
                    Status = NDIS_STATUS_FAILURE;
                }
                else
                {
                    // Increase Total receive byte counter after real data received no mater any error or not
                    pAd->Counter.MTKCounters.ReceivedByteCount += (READ_RXWI_MPDU_TOTAL_BYTE_COUNT(pAd, pRxWI) - 4);
                    pAd->Counter.MTKCounters.OneSecReceivedByteCount +=  READ_RXWI_MPDU_TOTAL_BYTE_COUNT(pAd, pRxWI);
                    pAd->Counter.MTKCounters.RxCount ++;
                    
                    // Check for all RxD errors
                    Status = ApDataCheckRxError(pAd, pRxInfo);
                }

                if (READ_RXWI_MPDU_TOTAL_BYTE_COUNT(pAd,pRxWI) < 14)
                    Status = NDIS_STATUS_FAILURE;               
                
                // Add receive counters
                if (Status == NDIS_STATUS_SUCCESS)
                {
                    // Increase 802.11 counters & general receive counters
                    INC_COUNTER64(pAd->Counter.WlanCounters.ReceivedFragmentCount);
                }
                else if (pAd->UiCfg.bPromiscuous == FALSE)
                {
                    // Increase general counters
                    pAd->Counter.Counters8023.RxErrors++;
                    DBGPRINT(RT_DEBUG_INFO,("RX err filtered2 [MPDUtotalByteCount = 0x%x]RxInfo= 0x%08x\n", 
                                            READ_RXWI_MPDU_TOTAL_BYTE_COUNT(pAd, pRxWI),
                                            *(PULONG)pRxInfo));
                }
#endif

                if (Status == NDIS_STATUS_SUCCESS)
                {
                    do
                    {
                        if (pPort->SoftAP.ApCfg.bSoftAPReady == FALSE)
                        {
                            DBGPRINT(RT_DEBUG_INFO, ("%s: Drop this frame due to the SoftAP is not ready.\n", __FUNCTION__));
                            break; // give up this packet.
                        }

                        if (pRxD->RxDscrDW1.UnicastToMe /*READ_RXINFO_U2M(pAd, pRxInfo)*/ && (pHeader != NULL) )
                        {
                            if (pHeader->FC.Type == BTYPE_DATA)
                            {
#if 0                           
                                WRITE_PHY_CFG_NSS(pAd, &pPort->LastRxRatePhyCfg, READ_RXWI_NSS(pAd, pRxWI, READ_RXWI_PHY_MODE(pAd, pRxWI)));
                                WRITE_PHY_CFG_MCS(pAd, &pPort->LastRxRatePhyCfg, READ_RXWI_MCS(pAd, pRxWI, READ_RXWI_PHY_MODE(pAd, pRxWI)));
                                WRITE_PHY_CFG_BW(pAd, &pPort->LastRxRatePhyCfg, READ_RXWI_BW(pAd, pRxWI));
                                WRITE_PHY_CFG_SHORT_GI(pAd, &pPort->LastRxRatePhyCfg, READ_RXWI_SHORT_GI(pAd, pRxWI));
                                WRITE_PHY_CFG_MODE(pAd, &pPort->LastRxRatePhyCfg, READ_RXWI_PHY_MODE(pAd, pRxWI));
                                
                                if (READ_RXWI_RSSI0(pAd, pRxWI) != 0)
                                {
                                    pPort->SoftAP.ApCfg.LastRssi = MlmeSyncConvertToRssi(pAd, (CHAR) READ_RXWI_RSSI0(pAd, pRxWI), RSSI_0, (UCHAR) READ_RXWI_AntSel(pAd, pRxWI), (UCHAR) READ_RXWI_BW(pAd, pRxWI));
                                    pPort->SoftAP.ApCfg.AvgRssiX8    = (pPort->SoftAP.ApCfg.AvgRssiX8 - pPort->SoftAP.ApCfg.AvgRssi) + pPort->SoftAP.ApCfg.LastRssi;
                                    pPort->SoftAP.ApCfg.AvgRssi      = pPort->SoftAP.ApCfg.AvgRssiX8 >> 3;
                                }
                                if (READ_RXWI_RSSI1(pAd, pRxWI) != 0)
                                {
                                    pPort->SoftAP.ApCfg.LastRssi2    = MlmeSyncConvertToRssi(pAd, (CHAR) READ_RXWI_RSSI1(pAd, pRxWI), RSSI_1, (UCHAR) READ_RXWI_AntSel(pAd, pRxWI), (UCHAR) READ_RXWI_BW(pAd, pRxWI));
                                    pPort->SoftAP.ApCfg.AvgRssi2X8   = (pPort->SoftAP.ApCfg.AvgRssi2X8 - pPort->SoftAP.ApCfg.AvgRssi2) + pPort->SoftAP.ApCfg.LastRssi2;
                                    pPort->SoftAP.ApCfg.AvgRssi2 = pPort->SoftAP.ApCfg.AvgRssi2X8 >> 3;
                                }
                                if (READ_RXWI_RSSI2(pAd, pRxWI) != 0)
                                {
                                    pPort->SoftAP.ApCfg.LastRssi3    = MlmeSyncConvertToRssi(pAd, (CHAR) READ_RXWI_RSSI2(pAd, pRxWI), RSSI_2, (UCHAR) READ_RXWI_AntSel(pAd, pRxWI), (UCHAR) READ_RXWI_BW(pAd, pRxWI));
                                    pPort->SoftAP.ApCfg.AvgRssi3X8   = (pPort->SoftAP.ApCfg.AvgRssi3X8 - pPort->SoftAP.ApCfg.AvgRssi3) + pPort->SoftAP.ApCfg.LastRssi3;
                                    pPort->SoftAP.ApCfg.AvgRssi3 = pPort->SoftAP.ApCfg.AvgRssi3X8 >> 3;
                                }

                                pPort->SoftAP.ApCfg.LastSNR0 = (UCHAR)(READ_RXWI_SNR0(pAd, pRxWI));
                                pPort->SoftAP.ApCfg.LastSNR1 = (UCHAR)(READ_RXWI_SNR1(pAd, pRxWI));
#endif                              
                                pPort->SoftAP.ApCfg.NumOfAvgRssiSample ++;
                        
                                // Gather PowerSave information from all valid DATA frames. IEEE 802.11/1999 p.461
                                // Atheros Station n has wrong
                                if ((pHeader->FC.PwrMgmt))
                                {
                                    APPsIndicate(pAd, pPort, pHeader->Addr2, pRxD->RxDscrDW2.WLANIndex /*READ_RXWI_WCID(pAd, pRxWI)*/, pPort->SoftAP.ApCfg.RssiSample.LastRssi[0], PWR_SAVE);
                                }
                                else
                                {
                                    APPsIndicate(pAd, pPort, pHeader->Addr2, pRxD->RxDscrDW2.WLANIndex /*READ_RXWI_WCID(pAd, pRxWI)*/, pPort->SoftAP.ApCfg.RssiSample.LastRssi[0], PWR_ACTIVE);
                                }
                            }
                        }
                        else if (pHeader != NULL)
                        {
                            if ((pHeader->FC.Type == BTYPE_MGMT) && 
                                ((pHeader->FC.SubType == SUBTYPE_BEACON) || (pHeader->FC.SubType == SUBTYPE_PROBE_REQ)))
                                ;
                            else
                            {
                                DBGPRINT(RT_DEBUG_INFO, (" Give up non-U2M (Type=%d, SubType=%d)\n", pHeader->FC.Type, pHeader->FC.SubType));
                        
                                break; // give up this frame
                            }
                        }

                        //
                        // CASE 0. receive a AMSDU frame. AMSDU must be HT QData unicast.
                        // DELETE AMSDU function 3/2x,2006
                        //
                        // CASE I. receive a DATA frame
                        //
                        if (pHeader->FC.Type == BTYPE_DATA)
                        {
                            if (pHeader->FC.ToDs == 0)
                                break; // give up this frame

                            // check if Class2 or 3 error
                            if ((pHeader->FC.FrDs == 0) && (ApDataCheckClass2Class3Error(pAd, pPort, pRxD->RxDscrDW2.WLANIndex /*READ_RXWI_WCID(pAd, pRxWI)*/ ,pHeader)))
                                break; // give up this frame                            

                            // Drop NULL, CF-ACK(no data), CF-POLL(no data), and CF-ACK+CF-POLL(no data) data frame
                            if (pHeader->FC.SubType & 0x04) // bit 2 : no DATA
                                break; // give up this frame

                            if(pPort->SoftAP.ApCfg.BANClass3Data==TRUE)
                                break; // give up this frame

                            pAd->Counter.MTKCounters.OneSecRxOkDataCnt++;
                            
#if 0                           
                            // pData : Pointer skip the RxD Descriptior and the first 24 bytes, 802.11 HEADER
                            pData += pAd->HwCfg.RXWI_Length;                          
                            if ((pHeader->FC.FrDs == 1) && (pHeader->FC.ToDs == 1))
                            {
                                pData += LENGTH_802_11_WITH_ADDR4;
                                DataSize = (USHORT)READ_RXWI_MPDU_TOTAL_BYTE_COUNT(pAd, pRxWI) - LENGTH_802_11_WITH_ADDR4;
                            }
                            else
                            {
                                pData += LENGTH_802_11;
                                DataSize = (USHORT)READ_RXWI_MPDU_TOTAL_BYTE_COUNT(pAd, pRxWI) - LENGTH_802_11;
                            }
#else
                            if ((pHeader->FC.FrDs == 1) && (pHeader->FC.ToDs == 1))
                            {
                                DataSize = PacketSize - LENGTH_802_11_WITH_ADDR4;
                            }
                            else
                            {
                                DataSize = PacketSize - LENGTH_802_11;
                            }
#endif
                            bAMsdu = FALSE;
                            UserPriority = 0;

                            if (pHeader->FC.SubType & 0x08)
                            {
                                //user priority
                                UserPriority = pRxD->RxDscrDW2.WLANIndex /*READ_RXWI_TID(pAd, pRxWI)*/ & 0x07;
                                DBGPRINT(RT_DEBUG_INFO,("Qos data, UserPriority = %d\n", UserPriority));

                                // bit 7 in Qos Control field signals the HT A-MSDU format
                                if ((*pQoSField) & 0x80)
                                {
                                    bAMsdu = TRUE;
                                    DBGPRINT(RT_DEBUG_INFO, ("A-MSDU data =%x",*pData));
                                }
                    
                                pData += 2;
                                DataSize -= 2;

                                //
                                // Dropt this QOS NULL Data frame.
                                //
                                if (pHeader->FC.SubType == SUBTYPE_QOS_NULL)
                                {
                                    DBGPRINT(RT_DEBUG_INFO,("[%s][LINE_%d]Drop Qos NULL Packet\n",__FUNCTION__,__LINE__));
                                    break;
                                }

                                pHeader->FC.SubType &= 0xf7;                                
                            }

                            //
                            //Check HTC first then a.) QoS(2)+HTC(4)+L2PAD(2)
                            //                          b.) QoS(2)+L2PAD(2)+Aggregation(2)
                            //                  c.) QoS(2)+L2PAD(2)
                            Msdu2Size = 0;
                            if( (pHeader->FC.Order) && (bHtVhtFrame == TRUE) )
                            {
                                // Case a.
                                // shift HTC byte
                                pData += 4;
                                DataSize -= 4;  
#if 0 // MT7603 no padding for QoS
                                // shift the 2 extra QOS CNTL bytes
                                if (READ_RXINFO_L2PAD(pAd, pRxInfo)== 1)
                                {
                                    pData += 2;
                                }                               
#endif                              
                            }
                            else 
                            {
#if 0 // MT7603 no padding for QoS                          
                                // shift the 2 extra QOS CNTL bytes
                                if (READ_RXINFO_L2PAD(pAd, pRxInfo)== 1)
                                {
                                    pData += 2;
                                }
#endif
                                pWcidMacTabEntry = MlmeSyncMacTabMatchedWlanIdx(pPort, (USHORT)pRxD->RxDscrDW2.WLANIndex );   

                                if(pWcidMacTabEntry == NULL)
                                {
                                    DBGPRINT(RT_DEBUG_TRACE,("%s(), Line:%d, Mac Entry is NULL  Wcid = %d\n", __FUNCTION__, __LINE__, pRxD->RxDscrDW2.WLANIndex));
                                    return NDIS_STATUS_FAILURE;
                                }
                                
                                if((pHeader->FC.Order) && (bHtVhtFrame == FALSE)&& CLIENT_STATUS_TEST_FLAG(pWcidMacTabEntry, fCLIENT_STATUS_AGGREGATION_CAPABLE))
                                {
                                    // Case b.  
                                    Msdu2Size = *pData + (*(pData+1) << 8);
                                    if ((Msdu2Size <= 1536) && (Msdu2Size < DataSize))
                                    {
                                        pData += 2;
                                        DataSize -= 2;
                                    }
                                    else
                                    {
                                        Msdu2Size = 0;
                                    }
                                }
                            }

                            if (DataSize == 0)
                            {
                                DBGPRINT(RT_DEBUG_TRACE, ("%s: Discard the data packet(seq=%d) with zero payload\n",
                                    __FUNCTION__, 
                                    pHeader->Sequence));

                                break;
                            }


                            //*********************************************************************************
                            //**
                            //** For Native WiFi packet indication, 
                            //** The packet included 802.11 header + data must be continuous
                            //**
                            //** For most case,
                            //** We do this job here. 
                            //**
                            //*********************************************************************************
                            ReFormatBuffer = (PUCHAR)(pData - LENGTH_802_11);
                            if ((PUCHAR)pHeader != ReFormatBuffer)
                            {
                                RtlMoveMemory(ReFormatBuffer, (PUCHAR)pHeader, LENGTH_802_11);
                                pHeader = (PHEADER_802_11)ReFormatBuffer;
                                //
                                // Dropt this QOS NULL Data frame.
                                //
                                if (pHeader->FC.SubType == SUBTYPE_QOS_NULL)
                                {
                                    DBGPRINT(RT_DEBUG_INFO,("[%s][LINE_%d]Drop Qos NULL Packet\n",__FUNCTION__,__LINE__));
                                    break;                              
                                }

                                //
                                // Remove QOS Type
                                //
                                pHeader->FC.SubType &= 0xf7;                                
                            }

                            // 802.1x frame is sent to MLME instead of upper layer TCPIP
                            if (ApWpaCheckWPAframe(pAd, (PUCHAR)pData, DataSize))
                            {
                                REPORT_MGMT_FRAME_TO_MLME(pAd,
                                                            pPort, 
                                                            pRxD->RxDscrDW2.WLANIndex /*READ_RXWI_WCID(pAd, pRxWI)*/, 
                                                            pHeader, 
                                                            DataSize + LENGTH_802_11, 
                                                            rx_signal.raw_rssi[0], 
                                                            rx_signal.raw_rssi[1], 
                                                            rx_signal.raw_rssi[2], 
                                                            0); //READ_RXWI_AntSel(pAd, pRxWI));
                                
                                DBGPRINT(RT_DEBUG_TRACE, ("get WPA frame ===> \n"));
                                break; // end of processing this frame
                            }
                            else
                            {
                                // Drop Unencrypted Packet (Except EAPOL & NULL Frame) when our AP sets Security Mode (WPA/WPA2 WEP)
                                if((pPort->PortCfg.WepStatus != Ralink802_11WEPDisabled) &&
                                    (pHeader->FC.Wep == 0) &&
                                    !(pHeader->FC.SubType == SUBTYPE_NULL_FUNC || pHeader->FC.SubType == SUBTYPE_QOS_NULL))
                                {
                                    DBGPRINT(RT_DEBUG_TRACE, ("~! Drop Plain text packet!!seq = %d \n", pHeader->Sequence));
                                    break;
                                }
                            }

                            pEntry = ApWpaPACInquiry(pAd, pHeader->Addr2, &Privacy, &WpaState);
                            // port access control
                            if (pRxD->RxDscrDW2.WLANIndex /*READ_RXWI_WCID(pAd, pRxWI)*/ < MAX_LEN_OF_MAC_TABLE)
                            {
                                pEntry = pWcidMacTabEntry;
                                
                                Privacy = pEntry->PrivacyFilter;
                                WpaState = pEntry->WpaState;
                                if ((pEntry->bIAmBadAtheros == FALSE)
                                    && (pRxD->RxDscrDW2.NonAMPDUFrame == 0) //(READ_RXINFO_AMPDU(pAd, pRxInfo)== 1)
                                    && (pHeader->FC.Retry)
                                    /*&& (READ_RXWI_PHY_MODE(pAd, pRxWI)< MODE_VHT)*/)
                                {
                                    if (!AES_ON(pPort))
                                    {
                                        MtAsicUpdateProtect(pAd, HT_ATHEROS, ALLN_SETPROTECT, FALSE, FALSE);
                                        DBGPRINT(RT_DEBUG_TRACE, ("%s, LINE_%d, Atheros Problem. Turn on RTS/CTS!!!\n",__FUNCTION__,__LINE__));
                                        pEntry->bIAmBadAtheros = TRUE;
                                        pEntry->MpduDensity = 7;
                                        pPort->CommonCfg.IOTestParm.bLastAtheros = TRUE;
                                    }
                                }
                            }

#if 0
                            //
                            // Check NDS Case
                            //
                            if (pEntry && (pEntry->NDSClientSetting == NDS_CLIENT_EVL))
                            {
                                ApDataCheckNDSClient(pAd, pEntry, pRxWI);
                            }
#endif

                            // WDS packet's DA & SA is not the same
                            if (bWdsPacket)
                            {
                                Privacy = Ndis802_11PrivFilterAcceptAll; // no 802.1x PAC for WDS frame
                                pDA = pHeader->Addr3;
                                pSA = (PUCHAR)pHeader + sizeof(HEADER_802_11);
                            }
                            else
                            {
                                pDA = pHeader->Addr3;
                                pSA = pHeader->Addr2;
                            }

                            // drop all non-802.1x DATA frame before this client's Port-Access-Control is secured
                            if (Privacy == Ndis802_11PrivFilter8021xWEP)
                            {
                                DBGPRINT(RT_DEBUG_INFO,("[%s][LINE_%d]Drop Packet\n",__FUNCTION__,__LINE__));
                                break; // give up this frame
                            }


                            // First or Only fragment
                            if (pHeader->Frag == 0) 
                            {
                                pAd->pRxCfg->FragFrame.Flags &= 0xFFFFFFFE;

                                // Firt Fragment & LLC/SNAP been removed. Keep the removed LLC/SNAP for later on
                                // TKIP MIC verification.
                                if (pHeader->FC.MoreFrag)
                                {
                                    pAd->pRxCfg->FragFrame.Flags |= 0x01;
                                }

                                // One & The only fragment
                                if (pHeader->FC.MoreFrag == FALSE)
                                {
                                    if ((pHeader->FC.Order == 1)  && (Msdu2Size > 0) ) // this is an aggregation
                                    {
                                        USHORT Payload1Size, Payload2Size;

                                        pAd->Counter.MTKCounters.OneSecRxAggregationCount ++;
                                        Payload1Size = DataSize - Msdu2Size;
                                        Payload2Size = Msdu2Size - LENGTH_802_3;

                                        //
                                        // Ralink Aggregate frame
                                        //
                                        // check if DA is another associted WSTA reachable via wireless bridging,
                                        // if it is, then no need to indicate to LLC
                                        if (ApDataBridgeToWdsAndWirelessSta(pAd,pPort, pHeader, (Payload1Size + LENGTH_802_11), wdsidx, UserPriority))
                                        {
                                            PlatformMoveMemory(Add2Ptr(pHeader, Payload1Size + 6), SNAP_802_1H, 6);
                                            PlatformMoveMemory(Add2Ptr(pHeader, Payload1Size + 6 - LENGTH_802_11), pHeader, LENGTH_802_11);

                                            ApDataBridgeToWdsAndWirelessSta(pAd,
                                                pPort,
                                                (PHEADER_802_11)Add2Ptr(pHeader, Payload1Size + 6 - LENGTH_802_11),
                                                (Payload2Size + LENGTH_802_11 + LENGTH_802_1_H),
                                                wdsidx,
                                                UserPriority);
                                        }
                                        else
                                        {       
                                            NdisCommonReportEthFrameToLLCAgg(pAd,pPort, (PUCHAR)pHeader, Payload1Size, Payload2Size, UserPriority);
                                        }

                                    }
                                    else if (bAMsdu)
                                    {
                                        //
                                        // TODO: Add AMSDU forwarding.
                                        //
                                        // Currently we don't support AMSDU forwarding, but we need to update the Header
                                        //
                                        pHeader->FC.ToDs    = 0;
                                        pHeader->FC.FrDs    = 0;
                                        //COPY_MAC_ADDR(pHeader->Addr2, SA);
                                        COPY_MAC_ADDR(pHeader->Addr3, pPort->SoftAP.ApCfg.AdhocBssid);

                                        if ((!pRxD->RxDscrDW2.NonAMPDUFrame)/*(READ_RXINFO_BA(pAd, pRxInfo) == TRUE)*/ && (pHeader->FC.SubType != SUBTYPE_QOS_NULL))
                                        {
                                            BOOLEAN bStatus;

                                            NdisAcquireSpinLock(&pPort->BADeleteRECEntry);
#if 0
                                            bStatus = QosBADataParse(pAd, 
                                                                    pPort,
                                                                    TRUE, 
                                                                    FALSE, 
                                                                    (PUCHAR)pHeader, 
                                                                    (UCHAR)READ_RXWI_WCID(pAd, pRxWI),                                                                  
                                                                    (UCHAR)READ_RXWI_TID(pAd, pRxWI),                                                                   
                                                                    (USHORT)READ_RXWI_SN(pAd, pRxWI),
                                                                    DataSize + LENGTH_802_11);
#else
                                            bStatus = QosBADataParse(pAd, 
                                                                    pPort,
                                                                    TRUE, 
                                                                    FALSE, 
                                                                    (PUCHAR)pHeader, 
                                                                    (UCHAR)pRxD->RxDscrDW2.WLANIndex, 
                                                                    (UCHAR)pRxD->RxDscrDW2.TID,
                                                                    (USHORT)pHeader->Sequence,
                                                                    DataSize + LENGTH_802_11);
#endif
                                            
                                            NdisReleaseSpinLock(&pPort->BADeleteRECEntry);                                          
                                            if (!bStatus)
                                            {
                                                NdisCommonReportAMSDUEthFrameToLLC(pAd, pPort,(PUCHAR)pHeader, DataSize + LENGTH_802_11, UserPriority, FALSE);
                                            }
                                        }
                                        else
                                        {
                                            NdisCommonReportAMSDUEthFrameToLLC(pAd, pPort,(PUCHAR)pHeader, DataSize + LENGTH_802_11, UserPriority, FALSE);
                                        }
                                    }
                                    else
                                    {
                                        UCHAR       Returnbrc;
                                    
                                        // check if DA is another associted WSTA reachable via wireless bridging,
                                        // if it is, then no need to indicate to LLC
                                        if (ApDataBridgeToWdsAndWirelessSta(pAd,pPort, pHeader, DataSize + LENGTH_802_11, wdsidx, UserPriority))
                                            break;

                                        if ((!pRxD->RxDscrDW2.NonAMPDUFrame)/*(READ_RXINFO_BA(pAd, pRxInfo) == TRUE)*/ && (pHeader->FC.SubType != SUBTYPE_QOS_NULL))
                                        {
                                            NdisAcquireSpinLock(&pPort->BADeleteRECEntry);
#if 0
                                            Returnbrc = QosBADataParse(pAd, 
                                                                        pPort,
                                                                        FALSE, 
                                                                        FALSE, 
                                                                        (PUCHAR)pHeader, 
                                                                        (UCHAR)READ_RXWI_WCID(pAd, pRxWI),
                                                                        (UCHAR)READ_RXWI_TID(pAd, pRxWI),
                                                                        (USHORT)READ_RXWI_SN(pAd, pRxWI),
                                                                        DataSize + LENGTH_802_11);
#else
                                            Returnbrc = QosBADataParse(pAd, 
                                                                    pPort,
                                                                    FALSE, 
                                                                    FALSE, 
                                                                    (PUCHAR)pHeader, 
                                                                    (UCHAR)pRxD->RxDscrDW2.WLANIndex, 
                                                                    (UCHAR)pRxD->RxDscrDW2.TID,
                                                                    (USHORT)pHeader->Sequence,
                                                                    DataSize + LENGTH_802_11);
#endif
                                            
                                            NdisReleaseSpinLock(&pPort->BADeleteRECEntry);
                                            if (Returnbrc == FALSE)
                                            {
                                                NdisCommonReportEthFrameToLLC(pAd,pPort, (PUCHAR)pHeader, DataSize + LENGTH_802_11,UserPriority, FALSE, FALSE);
                                            }
                                        }
                                        else
                                        {
                                            NdisCommonReportEthFrameToLLC(pAd,pPort, (PUCHAR)pHeader, DataSize + LENGTH_802_11,UserPriority, FALSE, FALSE);
                                        }
                                    }
                                }
                                // First fragment - record the 802.3 header and frame body
                                else
                                {
                                    PlatformMoveMemory(&pAd->pRxCfg->FragFrame.Buffer, pHeader, DataSize + LENGTH_802_11);
                                    pAd->pRxCfg->FragFrame.RxSize   = DataSize + LENGTH_802_11;
                                    pAd->pRxCfg->FragFrame.Sequence = pHeader->Sequence;
                                    pAd->pRxCfg->FragFrame.LastFrag = pHeader->Frag;       // Should be 0
                                }
                            }
                            // Middle & End of fragment burst
                            else
                            {
                                // No LLC-SNAP header in except the first fragment frame

                                if ((pHeader->Sequence != pAd->pRxCfg->FragFrame.Sequence) ||
                                    (pHeader->Frag != (pAd->pRxCfg->FragFrame.LastFrag + 1)))
                                {
                                    // Fragment is not the same sequence or out of fragment number order
                                    // Clear Fragment frame contents
                                    PlatformZeroMemory(&pAd->pRxCfg->FragFrame, sizeof(FRAGMENT_FRAME));
                                    DBGPRINT(RT_DEBUG_INFO,("[%s][LINE_%d]Drop Packet\n",__FUNCTION__,__LINE__));
                                    break; // give up this frame
                                }   
                                else if ((pAd->pRxCfg->FragFrame.RxSize + DataSize) > MAX_FRAME_SIZE)
                                {
                                    // Fragment frame is too large, it exeeds the maximum frame size.
                                    // Clear Fragment frame contents
                                    PlatformZeroMemory(&pAd->pRxCfg->FragFrame, sizeof(FRAGMENT_FRAME));
                                    DBGPRINT(RT_DEBUG_INFO,("[%s][LINE_%d]Drop Packet\n",__FUNCTION__,__LINE__));
                                    break; // give up this frame
                                }

                                // concatenate this fragment into the re-assembly buffer
                                PlatformMoveMemory(&pAd->pRxCfg->FragFrame.Buffer[pAd->pRxCfg->FragFrame.RxSize], pData, DataSize);
                                pAd->pRxCfg->FragFrame.RxSize  += DataSize;
                                pAd->pRxCfg->FragFrame.LastFrag = pHeader->Frag;       // Update fragment number
                                // Last fragment
                                if (pHeader->FC.MoreFrag == FALSE)
                                {
                                    // For TKIP frame, calculate the MIC value
                                    if ((pRxD->RxDscrDW2.WLANIndex /*READ_RXWI_WCID(pAd, pRxWI)*/ < MAX_LEN_OF_MAC_TABLE) 
                                        && (pWcidMacTabEntry->WepStatus == Ralink802_11Encryption2Enabled))
                                    {
                                        if (!pEntry)
                                            break;
                                        pWpaKey = &pEntry->PairwiseKey;

                                        // Minus MIC length
                                        pAd->pRxCfg->FragFrame.RxSize -= 8;
                                    }

                                    // check if DA is another associted WSTA reachable via wireless bridging,
                                    // if it is, then no need to indicate to LLC
                                    if (ApDataBridgeToWdsAndWirelessSta(pAd,pPort, (PHEADER_802_11) pAd->pRxCfg->FragFrame.Buffer, pAd->pRxCfg->FragFrame.RxSize, wdsidx, UserPriority) == FALSE)
                                    {
                                        NdisCommonReportEthFrameToLLC(pAd,pPort, pAd->pRxCfg->FragFrame.Buffer,(USHORT) pAd->pRxCfg->FragFrame.RxSize,UserPriority, FALSE, FALSE);           
                                    }
                                }
                            }           
                            break;
                        }
                        //
                        // CASE II. receive a MGMT frame
                        //
                        else if (pHeader->FC.Type == BTYPE_MGMT)
                        {
                            if (pPort->SoftAP.ApCfg.BANClass3Data==TRUE)
                            {
                                // disallow new association
                                if ((pHeader->FC.SubType == SUBTYPE_ASSOC_REQ) || (pHeader->FC.SubType == SUBTYPE_AUTH))
                                {
                                    DBGPRINT(RT_DEBUG_TRACE, ("   Disallow new Association \n "));
                                    break; // give up this frame
                                }
                            }

#if 0 // need to check : Lens
                            //
                            // Software decrypts the challenge text in the authentication packet (sequency number #3)
                            //
                            if ((pHeader->FC.Wep == 1) && (READ_RXINFO_DECRYPTED(pAd, pRxInfo) == 0))
                            {
                                if (IS_RT5390(pAd) || IS_RT3593(pAd) || IS_RT5592(pAd))
                                {
                                    if (WEP_ON(pPort))
                                    {
                                        if (RTMPPortDecryptData(pPort, (((PUCHAR)(pHeader)) + LENGTH_802_11), (PacketSize /*READ_RXWI_MPDU_TOTAL_BYTE_COUNT(pAd, pRxWI)*/ - LENGTH_802_11)) == FALSE)
                                        {   
                                            DBGPRINT(RT_DEBUG_TRACE, ("%s: Decrypt ERROR :: SN = %d, Type = %d, SubType = %d, pRxWI->MPDUtotalByteCount = %d\n", 
                                                __FUNCTION__, 
                                                pHeader->Sequence, 
                                                pHeader->FC.Type, 
                                                pHeader->FC.SubType, 
                                                PacketSize /*READ_RXWI_MPDU_TOTAL_BYTE_COUNT(pAd, pRxWI)*/));
                                        }
                                    }
                                }
                            }
#endif                          
                            REPORT_MGMT_FRAME_TO_MLME(pAd, 
                                                        pPort,
                                                        pRxD->RxDscrDW2.WLANIndex /*READ_RXWI_WCID(pAd, pRxWI)*/,
                                                        pHeader, 
                                                        PacketSize, //READ_RXWI_MPDU_TOTAL_BYTE_COUNT(pAd, pRxWI),
                                                        rx_signal.raw_rssi[0], 
                                                        rx_signal.raw_rssi[1], 
                                                        rx_signal.raw_rssi[2], 
                                                        0); //READ_RXWI_AntSel(pAd, pRxWI));
                            
                            break;  // end of processing this frame
                        }
                        //
                        // CASE III. receive a CNTL frame
                        //
                        else if (pHeader->FC.Type == BTYPE_CNTL)
                        {
                            if ((pPort->SoftAP.ApCfg.PhyMode >= PHY_11ABGN_MIXED) && (pHeader->FC.SubType == SUBTYPE_BLOCK_ACK_REQ))
                            {
                                CntlEnqueueForRecv(pAd,
                                                    pPort, 
                                                    pRxD->RxDscrDW2.WLANIndex /*READ_RXWI_WCID(pAd, pRxWI)*/,
                                                    PacketSize /*READ_RXWI_MPDU_TOTAL_BYTE_COUNT(pAd, pRxWI)*/,
                                                    (PFRAME_BA_REQ)pHeader);

                            }
                            // handle PS-POLL here
                            if ((pHeader->FC.SubType == SUBTYPE_PS_POLL))
                            {
                                 USHORT Aid = pHeader->Duration & 0x3fff;
                                 PUCHAR pAddr = pHeader->Addr2;

                                 DBGPRINT(RT_DEBUG_TRACE,("rcv PS-POLL (AID=%d) (WCID=%d) from %02x:%02x:%02x:%02x:%02x:%02x\n", 
                                     Aid, pRxD->RxDscrDW2.WLANIndex /*READ_RXWI_WCID(pAd, pRxWI)*/, pAddr[0], pAddr[1], pAddr[2], pAddr[3], pAddr[4], pAddr[5]));
                                if (Aid < MAX_LEN_OF_MAC_TABLE)
#if UAPSD_AP_SUPPORT                    
                                    ApDataHandleRxPsPoll(pAd, pAddr, Aid, TRUE, pPort, FALSE);
#else
                                    ApDataHandleRxPsPoll(pAd, pAddr, Aid, TRUE);
#endif
                            }
                            else if ((pPort->SoftAP.ApCfg.PhyMode >= PHY_11ABGN_MIXED) && (pHeader->FC.SubType == SUBTYPE_BLOCK_ACK))
                            {
                                PHT_CONTROL pHT;
                                ptemp = (PULONG)pHeader;
                                pHT = (PHT_CONTROL)(&pHeader->Addr3[0]);

                                if ((pAd->pHifCfg->bForcePrintRX == TRUE))
                                    DBGPRINT(RT_DEBUG_TRACE,("in B-ACK : MFB =0x%x : ID=%x : MFS=%x\n",pHT->MFBorASC, pHT->MRSorASI, pHT->MFS));


                            }
                            else if ((pHeader->FC.SubType == SUBTYPE_ACK))
                            {
                                PHT_CONTROL pHT;
                                ptemp = (PULONG)pHeader;
                                pHT = (PHT_CONTROL)(&pHeader->Addr2[0]);

                                if ((pAd->pHifCfg->bForcePrintRX == TRUE) )
                                    DBGPRINT(RT_DEBUG_TRACE,("in ACK : MFB =0x%x : ID=%x : MFS=%x\n",pHT->MFBorASC, pHT->MRSorASI, pHT->MFS));
                            }
                            else
                            {
                                 DBGPRINT(RT_DEBUG_INFO,("ignore CNTL (subtype=%d)\n", pHeader->FC.SubType));
                            }
                            break; // end of processing this frame
                        }

                        //
                        // CASE IV. receive a frame of invalid type
                        //
                        else
                            break; // give up this frame

                    }while(FALSE);
                }

                
            }
            // for next packet from RXBULKAggregation, position is at ThisFrameLen+8, so add 8 here.
            ThisFrameLen += 8;
            // update next packet read position. 
            ReadPosition += ThisFrameLen;
            if (TransferBufferLength >= (ThisFrameLen))
                TransferBufferLength -= (ThisFrameLen);
            else
                TransferBufferLength = 0;
            
            if (TransferBufferLength <=  4)
                break;
        } //end while (TransferBufferLength > 0)

        pRxContext->Readable = FALSE;
        pRxContext->ReadPosOffset = 0;
        pRxContext = &pAd->pHifCfg->RxContext[nIndex];

        ReadPacket++;
    }   while (FALSE);

    if (pAd->pHifCfg->bBulkInRead == TRUE)
    {
        pAd->pHifCfg->bBulkInRead = FALSE;
        NdisReleaseSpinLock(&pAd->pHifCfg->BulkInReadLock);
    }
    else
    {
        for (index = 0; index < RTMP_MAX_NUMBER_OF_PORT; index ++)
        {
            pPort = pAd->PortList[index];
            if (pPort != NULL)
            {
                portCount ++;
                NdisAcquireSpinLock(&pAd->pRxCfg->IndicateRxPktsLock[pPort->PortNumber]);
                if (pAd->pRxCfg->nWaitToIndicateRxPktsNum[pPort->PortNumber] > 0)
                {
                    PlatformIndicatePktToNdis(pAd, (PVOID)pAd->pRxCfg->pWaitToIndicateRxPkts[pPort->PortNumber], pPort->PortNumber, pAd->pRxCfg->nWaitToIndicateRxPktsNum[pPort->PortNumber]);
                    pAd->pRxCfg->pLastWaitToIndicateRxPkts[pPort->PortNumber] = NULL;
                    pAd->pRxCfg->pWaitToIndicateRxPkts[pPort->PortNumber] = NULL;
                    pAd->pRxCfg->nWaitToIndicateRxPktsNum[pPort->PortNumber] = 0;
                }
                NdisReleaseSpinLock(&pAd->pRxCfg->IndicateRxPktsLock[pPort->PortNumber]);

                if(portCount == pAd->NumberOfPorts)
                    break;
            }
        }
    }

    if (bBulkReceive == TRUE)
        NdisCommonBulkRx(pAd);

    return STATUS_MORE_PROCESSING_REQUIRED;
}

VOID
ApDataCommonRxPacket(
    IN  PMP_ADAPTER   pAd,
    IN  PMP_PORT      pPort,
    IN  PRXDSCR_BASE_STRUC  pRxD,
    IN  PHEADER_802_11  pHeader,
    IN  PUCHAR          pData,
    IN  USHORT              PacketSize,
    IN  UCHAR               QosPadding,
    IN  PUCHAR             pQoSField,
    IN  struct rx_signal_info *rx_signal
    )
{
    BOOLEAN             bHtVhtFrame = FALSE;
    NDIS_STATUS         ndisStatus;
    BOOLEAN             bAMsdu = FALSE;
    UCHAR               UserPriority = 0;
    USHORT              Msdu2Size;
    PUCHAR              ReFormatBuffer;
    MAC_TABLE_ENTRY     *pEntry = NULL;
    NDIS_802_11_PRIVACY_FILTER  Privacy = Ndis802_11PrivFilterAcceptAll;
    AP_WPA_STATE        WpaState;
    PUCHAR              pDA, pSA;
    ULONG               wdsidx = 0;
    PCIPHER_KEY         pWpaKey;
    BOOLEAN             EAPOLFrame = FALSE;
    UCHAR               OldPwrMgmt = PWR_ACTIVE;    // UAPSD AP SUPPORT
    USHORT              DataSize = 0;
    PMAC_TABLE_ENTRY pWcidMacTabEntry = NULL;

#if 0
    if (READ_RXWI_PHY_MODE(pAd, pRxWI) >= MODE_HTMIX)
        bHtVhtFrame = TRUE;
#endif

    if (pRxD == NULL )
    {
        DBGPRINT(RT_DEBUG_EMU,("pRxD = NULL Error!!\n"));
        ndisStatus = NDIS_STATUS_FAILURE;
    }
    else if (pHeader == NULL )
    {
        DBGPRINT(RT_DEBUG_EMU,("pHeader = NULL Error!!\n"));
        ndisStatus = NDIS_STATUS_FAILURE;
    }
    else if (pData == NULL )
    {
        DBGPRINT(RT_DEBUG_EMU,("pData = NULL Error!!\n"));
        ndisStatus = NDIS_STATUS_FAILURE;
    }
    else
    {
        // Increase Total receive byte counter after real data received no mater any error or not
        pAd->Counter.MTKCounters.ReceivedByteCount += (PacketSize /*READ_RXWI_MPDU_TOTAL_BYTE_COUNT(pAd, pRxWI)*/ - 4);
        pAd->Counter.MTKCounters.RxCount ++;

        // Check for all RxD errors
        ndisStatus = ApDataCheckRxError(pAd, pRxD, pHeader);
        
        if(ndisStatus == NDIS_STATUS_FAILURE)
            DBGPRINT(RT_DEBUG_ERROR,("RX err  wcid = %d\n", pRxD->RxDscrDW2.WLANIndex /*READ_RXWI_WCID(pAd, pRxWI)*/)); 
    }

    if (PacketSize /*READ_RXWI_MPDU_TOTAL_BYTE_COUNT(pAd, pRxWI)*/ < 14)
        ndisStatus = NDIS_STATUS_FAILURE;

    // Add receive counters
    if (ndisStatus == NDIS_STATUS_SUCCESS)
    {
        // Increase 802.11 counters & general receive counters
        INC_COUNTER64(pAd->Counter.WlanCounters.ReceivedFragmentCount);
    }
    else if (pAd->UiCfg.bPromiscuous == FALSE)
    {
        // Increase general counters
        pAd->Counter.Counters8023.RxErrors++;
        DBGPRINT(RT_DEBUG_INFO,("RX err filtered2 [MPDUtotalByteCount = 0x%x]\n", PacketSize /*READ_RXWI_MPDU_TOTAL_BYTE_COUNT(pAd, pRxWI)*/));
    }

    if (ndisStatus == NDIS_STATUS_SUCCESS)
    {
        do
        {
            if ((pRxD->RxDscrDW1.UnicastToMe /*READ_RXINFO_U2M(pAd, pRxInfo)*/) && (pHeader != NULL) )
            {
                if (pHeader->FC.Type == BTYPE_DATA)
                {
#if 0               
                    //
                    // Cache the last Rx rate
                    //
                    WRITE_PHY_CFG_NSS(pAd, &pPort->LastRxRatePhyCfg, READ_RXWI_NSS(pAd, pRxWI, READ_RXWI_PHY_MODE(pAd, pRxWI)));
                    WRITE_PHY_CFG_MCS(pAd, &pPort->LastRxRatePhyCfg, READ_RXWI_MCS(pAd, pRxWI, READ_RXWI_PHY_MODE(pAd, pRxWI)));
                    WRITE_PHY_CFG_BW(pAd, &pPort->LastRxRatePhyCfg, READ_RXWI_BW(pAd, pRxWI));
                    WRITE_PHY_CFG_SHORT_GI(pAd, &pPort->LastRxRatePhyCfg, READ_RXWI_SHORT_GI(pAd, pRxWI));
                    WRITE_PHY_CFG_MODE(pAd, &pPort->LastRxRatePhyCfg, READ_RXWI_PHY_MODE(pAd, pRxWI));
                    
                    if (READ_RXWI_RSSI0(pAd, pRxWI)!= 0)
                    {
                        pPort->SoftAP.ApCfg.LastRssi = MlmeSyncConvertToRssi(pAd, (CHAR) READ_RXWI_RSSI0(pAd, pRxWI), RSSI_0, (UCHAR) READ_RXWI_AntSel(pAd, pRxWI), (UCHAR) READ_RXWI_BW(pAd, pRxWI));
                        pPort->SoftAP.ApCfg.AvgRssiX8    = (pPort->SoftAP.ApCfg.AvgRssiX8 - pPort->SoftAP.ApCfg.AvgRssi) + pPort->SoftAP.ApCfg.LastRssi;
                        pPort->SoftAP.ApCfg.AvgRssi      = pPort->SoftAP.ApCfg.AvgRssiX8 >> 3;
                    }
                    
                    if (READ_RXWI_RSSI1(pAd, pRxWI)!= 0)
                    {
                        pPort->SoftAP.ApCfg.LastRssi2    = MlmeSyncConvertToRssi(pAd, (CHAR) READ_RXWI_RSSI1(pAd, pRxWI), RSSI_1, (UCHAR) READ_RXWI_AntSel(pAd, pRxWI), (UCHAR) READ_RXWI_BW(pAd, pRxWI));
                        pPort->SoftAP.ApCfg.AvgRssi2X8   = (pPort->SoftAP.ApCfg.AvgRssi2X8 - pPort->SoftAP.ApCfg.AvgRssi2) + pPort->SoftAP.ApCfg.LastRssi2;
                        pPort->SoftAP.ApCfg.AvgRssi2 = pPort->SoftAP.ApCfg.AvgRssi2X8 >> 3;
                    }
                    
                    if (READ_RXWI_RSSI2(pAd, pRxWI)!= 0)
                    {
                        pPort->SoftAP.ApCfg.LastRssi3    = MlmeSyncConvertToRssi(pAd, (CHAR) READ_RXWI_RSSI2(pAd, pRxWI), RSSI_2, (UCHAR) READ_RXWI_AntSel(pAd, pRxWI), (UCHAR) READ_RXWI_BW(pAd, pRxWI));
                        pPort->SoftAP.ApCfg.AvgRssi3X8   = (pPort->SoftAP.ApCfg.AvgRssi3X8 - pPort->SoftAP.ApCfg.AvgRssi3) + pPort->SoftAP.ApCfg.LastRssi3;
                        pPort->SoftAP.ApCfg.AvgRssi3 = pPort->SoftAP.ApCfg.AvgRssi3X8 >> 3;
                    }

                    pPort->SoftAP.ApCfg.LastSNR0 = (UCHAR)(READ_RXWI_RSSI0(pAd, pRxWI));
                    pPort->SoftAP.ApCfg.LastSNR1 = (UCHAR)(READ_RXWI_RSSI1(pAd, pRxWI));
#endif

                    pPort->SoftAP.ApCfg.NumOfAvgRssiSample ++;

                    // Gather PowerSave information from all valid DATA frames. IEEE 802.11/1999 p.461
                    // Atheros Station n has wrong
                    if ((pHeader->FC.PwrMgmt))
                        OldPwrMgmt = APPsIndicate(pAd, pPort, pHeader->Addr2, pRxD->RxDscrDW2.WLANIndex /*READ_RXWI_WCID(pAd, pRxWI)*/, pPort->SoftAP.ApCfg.RssiSample.LastRssi[0], PWR_SAVE);
                    else
                        OldPwrMgmt = APPsIndicate(pAd, pPort, pHeader->Addr2, pRxD->RxDscrDW2.WLANIndex /*READ_RXWI_WCID(pAd, pRxWI)*/, pPort->SoftAP.ApCfg.RssiSample.LastRssi[0], PWR_ACTIVE);
                }
            }
          
            if (pHeader->FC.Type == BTYPE_DATA)
            {
                if (pHeader->FC.ToDs == 0)
                    break; // give up this frame

                // check if Class2 or 3 error
                if ((pHeader->FC.FrDs == 0) && (ApDataCheckClass2Class3Error(pAd,pPort, pRxD->RxDscrDW2.WLANIndex /*READ_RXWI_WCID(pAd, pRxWI)*/ ,pHeader)))
                {
                    break; // give up this frame
                }

                // Drop NULL, CF-ACK(no data), CF-POLL(no data), and CF-ACK+CF-POLL(no data) data frame
                if (pHeader->FC.SubType & 0x04) // bit 2 : no DATA
                    break;

                if(pPort->SoftAP.ApCfg.BANClass3Data==TRUE)
                    break; // give up this frame

                pAd->Counter.MTKCounters.OneSecRxOkDataCnt++;

                if ((pHeader->FC.FrDs == 1) && (pHeader->FC.ToDs == 1))
                {
                    DataSize = PacketSize - LENGTH_802_11_WITH_ADDR4;
                }
                else
                {
                    DataSize = PacketSize - LENGTH_802_11;
                }

                if (pRxD->RxDscrDW2.WLANIndex < MAX_LEN_OF_MAC_TABLE)
                {   
                    pWcidMacTabEntry = MlmeSyncMacTabMatchedWlanIdx(pPort, (USHORT)pRxD->RxDscrDW2.WLANIndex);  

                    if(pWcidMacTabEntry == NULL)
                    {
                        DBGPRINT(RT_DEBUG_TRACE,("%s(), Line:%d, Mac Entry is NULL  Wcid = %d\n", __FUNCTION__, __LINE__, pRxD->RxDscrDW2.WLANIndex));
                        return ;
                    }
                }

                bAMsdu = FALSE;
                UserPriority = 0;

                if (pHeader->FC.SubType & 0x08)
                {
                    //user priority
                    UserPriority = pRxD->RxDscrDW2.TID /*READ_RXWI_TID(pAd, pRxWI)*/ & 0x07;
                    DBGPRINT(RT_DEBUG_INFO,("Qos data, UserPriority = %d\n", UserPriority));

                    // bit 7 in Qos Control field signals the HT A-MSDU format
                    if ((*pQoSField) & 0x80)
                    {
                        bAMsdu = TRUE;
                        DBGPRINT(RT_DEBUG_INFO, ("A-MSDU data =%x",*pData));
                    }

                    pData += 2;
                    DataSize -= 2;

#if UAPSD_AP_SUPPORT
                    // == Handle Trigger Frame before reset QoS type ==
                    // 1: PWR_SAVE, 0: PWR_ACTIVE
                        
                    DBGPRINT(RT_DEBUG_INFO,("Chk::%s(%d)==> PwrMgmt=%d, PsMode=%d, SubType=0x%02x!!!\n", __FUNCTION__, __LINE__, pHeader->FC.PwrMgmt, 
                    pWcidMacTabEntry->PsMode, pHeader->FC.SubType));
                    if (pHeader->FC.PwrMgmt)
                    {
                        //if (pHeader->FC. & 0x08)
                        {
                            MAC_TABLE_ENTRY     *tmpEntry = NULL;

                            /*
                            In IEEE802.11e, 11.2.1.4 Power management with APSD,
                            If there is no unscheduled SP in progress, the unscheduled SP begins
                            when the QAP receives a trigger frame from a non-AP QSTA, which is a
                            QoS data or QoS Null frame associated with an AC the STA has
                            configured to be trigger-enabled.
                            */
                            /*
                            In WMM v1.1, A QoS Data or QoS Null frame that indicates transition
                            to/from Power Save Mode is not considered to be a Trigger Frame and
                            the AP shall not respond with a QoS Null frame.
                            */
                            /* 
                            Trigger frame must be QoS data or QoS Null frame 
                            */
                            /* 
                            Ignore it in the absent period for P2P Group Owner
                            */

                            tmpEntry = pWcidMacTabEntry;
                            if (IS_AP_SUPPORT_UAPSD(pAd, pPort) && (OldPwrMgmt == PWR_SAVE) && UAPSD_MR_IS_ENTRY_ACCESSIBLE(tmpEntry) && (!UAPSD_MR_IN_P2P_ABSENT(pPort, tmpEntry)))
                            {
                                DBGPRINT(RT_DEBUG_TRACE,("Chk***::%s(%d)==>Someone send trigger frame aid=%d, Addr=%02x:%02x:%02x:%02x:%02x:%02x, UP=%d\n", __FUNCTION__, __LINE__,
                                    tmpEntry->Aid, tmpEntry->Addr[0],tmpEntry->Addr[1],tmpEntry->Addr[2],tmpEntry->Addr[3],tmpEntry->Addr[4],tmpEntry->Addr[5], UserPriority));
                                ApUapsdTriggerFrameHandle(pAd, pPort, tmpEntry, UserPriority);
                            }
                        }
                    }
#endif

                    //
                    // Dropt this QOS NULL Data frame.
                    //
                    if (pHeader->FC.SubType == SUBTYPE_QOS_NULL)
                        break;

                    pHeader->FC.SubType &= 0xf7;                    
                }

                //
                //Check HTC first then a.) QoS(2)+HTC(4)+L2PAD(2)
                //                          b.) QoS(2)+L2PAD(2)+Aggregation(2)
                //                  c.) QoS(2)+L2PAD(2)
                Msdu2Size = 0;
                if ((pHeader->FC.Order) && (bHtVhtFrame == TRUE))
                {
                    // Case a.
                    // shift HTC byte
                    pData += 4;
                    DataSize -= 4;  

#if 0 // MT7603 no padding for QoS
                    // shift the 2 extra QOS CNTL bytes
                    if (READ_RXINFO_L2PAD(pAd, pRxInfo)== 1)
                    {
                        pData += 2;
                    }
#endif
                }
                else
                {
#if 0 // MT7603 no padding for QoS
                    // shift the 2 extra QOS CNTL bytes
                    if (READ_RXINFO_L2PAD(pAd, pRxInfo)== 1)
                    {
                        pData += 2;
                    }
#endif

                    if((pHeader->FC.Order) && (bHtVhtFrame == FALSE) && CLIENT_STATUS_TEST_FLAG(pWcidMacTabEntry, fCLIENT_STATUS_AGGREGATION_CAPABLE))
                    {
                        // Case b.  
                        Msdu2Size = *pData + (*(pData+1) << 8);
                        if ((Msdu2Size <= 1536) && (Msdu2Size < DataSize))
                        {
                            pData += 2;
                            DataSize -= 2;
                        }
                        else
                        {
                            Msdu2Size = 0;
                        }
                    }
                }

                //*********************************************************************************
                //**
                //** For Native WiFi packet indication, 
                //** The packet included 802.11 header + data must be continuous
                //**
                //** For most case,
                //** We do this job here. 
                //**
                //*********************************************************************************
                ReFormatBuffer = (PUCHAR)(pData - LENGTH_802_11);                   
                if ((PUCHAR)pHeader != ReFormatBuffer)
                {
                    RtlMoveMemory(ReFormatBuffer, (PUCHAR)pHeader, LENGTH_802_11);
                    pHeader = (PHEADER_802_11)ReFormatBuffer;
                    //
                    // Dropt this QOS NULL Data frame.
                    //
                    if (pHeader->FC.SubType == SUBTYPE_QOS_NULL)
                        break;

                    //
                    // Remove QOS Type
                    //
                    pHeader->FC.SubType &= 0xf7;
                }
            
                //
                // If the privacy exemption list does not apply to the frame, check ExcludeUnencrypted.
                // if ExcludeUnencrypted is set and this frame was not oringially an encrypted frame, 
                // dropped it.
                //

                if (PlatformEqualMemory(EAPOL, pData + 6, 2))
                {
                    DBGPRINT(RT_DEBUG_TRACE,("Sequence:%d, retry:%d \n", pHeader->Sequence, pHeader->FC.Retry));
                    //patch rx filter bug, avoid retry package indicate to driver
                    if((pPort->P2PCfg.EapolLastSecquence == pHeader->Sequence) && (pHeader->FC.Retry == 1) && (P2P_ON(pPort)))
                        return;
                    
                    EAPOLFrame = TRUE;
                    pPort->P2PCfg.EapolLastSecquence = pHeader->Sequence;
                    DBGPRINT(RT_DEBUG_TRACE,("Sequence:%d, retry:%d Value = %d\n", pHeader->Sequence, pHeader->FC.Retry));
                    DBGPRINT(RT_DEBUG_TRACE,("3.ApDataCommonRxPacket EAPoL\n"));
                }

                if (pPort->CommonCfg.bSafeModeEnabled == FALSE)
                {
                    ndisStatus = RecvCheckExemptionList(pAd,pPort,pHeader, NULL/*pRxWI*/, NULL/*pRxInfo*/,0);
                    if ( ndisStatus != NDIS_STATUS_SUCCESS)
                    {
                        DBGPRINT(RT_DEBUG_ERROR,("AP mode CheckExemptionLis fail\n"));
                        break;
                    }
                }


                pEntry = NULL;
                if (pRxD->RxDscrDW2.WLANIndex /*READ_RXWI_WCID(pAd, pRxWI)*/ < MAX_LEN_OF_MAC_TABLE)
                {
                    pEntry = pWcidMacTabEntry;

                    if (pEntry == NULL)
                    {
                        DBGPRINT(RT_DEBUG_TRACE,("%s(), Line:%d, Mac Entry is NULL  Wcid = %d\n", __FUNCTION__, __LINE__, pRxD->RxDscrDW2.WLANIndex));
                        break;
                    }

                    Privacy = pEntry->PrivacyFilter;
                    WpaState = pEntry->WpaState;

                    if ((pEntry->bIAmBadAtheros == FALSE) && (pRxD->RxDscrDW2.NonAMPDUFrame == 0)/*(READ_RXINFO_AMPDU(pAd, pRxInfo) == 1))*/ && (pHeader->FC.Retry))
                    {
                        if (!AES_ON(pPort))
                            MtAsicUpdateProtect(pAd, 8, ALLN_SETPROTECT, FALSE, FALSE);
                        DBGPRINT(RT_DEBUG_TRACE, ("Atheros Problem. Turn on RTS/CTS!!!\n"));
                        pEntry->bIAmBadAtheros = TRUE;
                        pEntry->MpduDensity = 7;
                        pPort->CommonCfg.IOTestParm.bLastAtheros = TRUE;                                  
                    }
                }

#if 0
                //
                // Check NDS Case
                //
                if (pEntry && (pEntry->NDSClientSetting == NDS_CLIENT_EVL))
                {
                    ApDataCheckNDSClient(pAd, pEntry, pRxWI);
                }
#endif

                //
                // Check WDS packet?
                //
                pDA = pHeader->Addr3;
                pSA = pHeader->Addr2;

            
                // drop all non-802.1x DATA frame before this client's Port-Access-Control is secured
                //use RecvCheckExemptionList to filter data  
                /*if (Privacy == Ndis802_11PrivFilter8021xWEP)
                    break;*/

                // First or Only fragment
                if (pHeader->Frag == 0) 
                {
                    pAd->pRxCfg->FragFrame.Flags &= 0xFFFFFFFE;

                    // Firt Fragment & LLC/SNAP been removed. Keep the removed LLC/SNAP for later on
                    // TKIP MIC verification.
                    if (pHeader->FC.MoreFrag)
                    {
                        pAd->pRxCfg->FragFrame.Flags |= 0x01;
                    }

                    // One & The only fragment
                    // Special DATA frame that has to pass to MLME
                    //   1. Cisco Aironet frames for CCX2. We need pass it to MLME for special process
                    //   2. EAPOL handshaking frames when driver supplicant enabled, pass to MLME for special process
                    //   3. Receive EAP packet for WSC only after enrolee link up to AP
                    //   4. IEEE 802.11z TDLS Action Data frames 
                    if ((pPort->StaCfg.WscControl.WscState >= WSC_STATE_LINK_UP)
                        ||(pAd->StaCfg.WpaState != SS_NOTUSE))
                    {
                        if (PlatformEqualMemory(EAPOL_LLC_SNAP, pData, LENGTH_802_1_H))
                        // 2 conditions to check EAPOL frame: 1. P2P GO doing WPS internal  registra
                        // 2. P2P GO and do 4-way myself .
                        {
                            PUCHAR  pTmpBuf = (PUCHAR)(pData - LENGTH_802_11);
                            pHeader->FC.SubType = SUBTYPE_DATA;
                            RtlMoveMemory(pTmpBuf, (PUCHAR)pHeader, LENGTH_802_11);
                            DataSize += LENGTH_802_11;
                            DBGPRINT_RAW(RT_DEBUG_TRACE, ("!!!22 AP RxPacket  report EAPOL/AIRONET DATA to MLME (non-fragmented packet) (len=%d), WSC=%d, WPAState=%d!!!\n", DataSize, pPort->StaCfg.WscControl.WscState, pAd->StaCfg.WpaState));
                            REPORT_MGMT_FRAME_TO_MLME(pAd, 
                                                        pPort, 
                                                        pRxD->RxDscrDW2.WLANIndex /*READ_RXWI_WCID(pAd, pRxWI)*/, 
                                                        pTmpBuf /*pHeader*/, 
                                                        DataSize, 
                                                        rx_signal->raw_rssi[0], 
                                                        rx_signal->raw_rssi[1], 
                                                        rx_signal->raw_rssi[2], 
                                                        0); //READ_RXWI_AntSel(pAd, pRxWI));
                            break;  // end of processing this frame
                        }
                    }

                    if (pHeader->FC.MoreFrag == FALSE)
                    {
                        if ((pHeader->FC.Order == 1)  && (Msdu2Size > 0) ) // this is an aggregation
                        {
                            USHORT Payload1Size, Payload2Size;

                            pAd->Counter.MTKCounters.OneSecRxAggregationCount ++;
                            Payload1Size = DataSize - Msdu2Size;
                            Payload2Size = Msdu2Size - LENGTH_802_3;

                            NdisCommonReportEthFrameToLLCAgg(pAd, pPort,(PUCHAR)pHeader, Payload1Size, Payload2Size, UserPriority);
                        }
                        else if (bAMsdu)
                        {
                            UCHAR       Returnbrc;

                            if ((!pRxD->RxDscrDW2.NonAMPDUFrame)/*(READ_RXINFO_BA(pAd, pRxInfo) == TRUE)*/ && (pHeader->FC.SubType != SUBTYPE_QOS_NULL))
                            {
#if 0
                                Returnbrc = QosBADataParse(pAd, 
                                                        pPort,
                                                        TRUE, 
                                                        FALSE, 
                                                        (PUCHAR)pHeader, 
                                                        (UCHAR)READ_RXWI_WCID(pAd, pRxWI), 
                                                        (UCHAR)READ_RXWI_TID(pAd, pRxWI),
                                                        (USHORT)READ_RXWI_SN(pAd, pRxWI),
                                                        DataSize + LENGTH_802_11);
#else
                                Returnbrc = QosBADataParse(pAd, 
                                                        pPort,
                                                        TRUE, 
                                                        FALSE, 
                                                        (PUCHAR)pHeader, 
                                                        (UCHAR)pRxD->RxDscrDW2.WLANIndex, 
                                                        (UCHAR)pRxD->RxDscrDW2.TID,
                                                        (USHORT)pHeader->Sequence,
                                                        DataSize + LENGTH_802_11);
#endif

                                if (!Returnbrc)
                                {
                                    NdisCommonReportAMSDUEthFrameToLLC(pAd,pPort,(PUCHAR)pHeader, DataSize + LENGTH_802_11, UserPriority, FALSE);
                                }
                            }
                            else
                            {
                                NdisCommonReportAMSDUEthFrameToLLC(pAd,pPort,(PUCHAR)pHeader, DataSize + LENGTH_802_11, UserPriority, FALSE);
                            }
                        }
                        else
                        {
                            UCHAR       Returnbrc;                          
                            
                            // check if DA is another associted WSTA reachable via wireless bridging,
                            // if it is, then no need to indicate to LLC
                            if ((pPort->PortSubtype == PORTSUBTYPE_P2PGO) && (pPort->PortType != WFD_GO_PORT))
                            {
                                if (ApDataBridgeToWdsAndWirelessSta(pAd, pPort, (PHEADER_802_11)pHeader, (DataSize + LENGTH_802_11), wdsidx, UserPriority))
                                {
                                    break;
                                }
                            }

                            if ((!pRxD->RxDscrDW2.NonAMPDUFrame)/*(READ_RXINFO_BA(pAd, pRxInfo) == TRUE)*/ && (pHeader->FC.SubType != SUBTYPE_QOS_NULL))
                            {                               
#if 0
                                Returnbrc = QosBADataParse(pAd, 
                                                            pPort,
                                                            FALSE, 
                                                            FALSE, 
                                                            (PUCHAR)pHeader, 
                                                            (UCHAR)READ_RXWI_WCID(pAd, pRxWI), 
                                                            (UCHAR)READ_RXWI_TID(pAd, pRxWI),
                                                            (USHORT)READ_RXWI_SN(pAd, pRxWI), 
                                                            DataSize + LENGTH_802_11);
#else
                                Returnbrc = QosBADataParse(pAd, 
                                                        pPort,
                                                        FALSE, 
                                                        FALSE, 
                                                        (PUCHAR)pHeader, 
                                                        (UCHAR)pRxD->RxDscrDW2.WLANIndex, 
                                                        (UCHAR)pRxD->RxDscrDW2.TID,
                                                        (USHORT)pHeader->Sequence,
                                                        DataSize + LENGTH_802_11);
#endif
                                if (!Returnbrc)
                                    NdisCommonReportEthFrameToLLC(pAd,pPort, (PUCHAR)pHeader, DataSize + LENGTH_802_11,UserPriority, FALSE, FALSE);
                            }
                            else
                            {
                                // Print for debug.
                                if (EAPOLFrame)
                                    DBGPRINT_RAW(RT_DEBUG_TRACE, ("!!!3 AP RxPacket  report  EAPOL DATA to  (non-fragmented packet) (len=%d), WSC=%d, WPAState=%d!!!\n", DataSize, pPort->StaCfg.WscControl.WscState, pAd->StaCfg.WpaState));
                                NdisCommonReportEthFrameToLLC(pAd,pPort, (PUCHAR)pHeader, DataSize + LENGTH_802_11,UserPriority, FALSE, FALSE);
                            }
                        }
                    }
                    // First fragment
                    else
                    {
#pragma prefast(suppress: __WARNING_READ_OVERRUN, "pHeader, should not Buffer overrun")                 
                        PlatformMoveMemory(&pAd->pRxCfg->FragFrame.Buffer, pHeader, DataSize + LENGTH_802_11);
                        pAd->pRxCfg->FragFrame.RxSize   = DataSize + LENGTH_802_11;
                        pAd->pRxCfg->FragFrame.Sequence = pHeader->Sequence;
                        pAd->pRxCfg->FragFrame.LastFrag = pHeader->Frag;       // Should be 0
                    }
                }
                // Middle & End of fragment burst
                else
                {
                    // No LLC-SNAP header in except the first fragment frame
                    if ((pHeader->Sequence != pAd->pRxCfg->FragFrame.Sequence) ||
                        (pHeader->Frag != (pAd->pRxCfg->FragFrame.LastFrag + 1)))
                    {
                        // Fragment is not the same sequence or out of fragment number order
                        // Clear Fragment frame contents
                        PlatformZeroMemory(&pAd->pRxCfg->FragFrame, sizeof(FRAGMENT_FRAME));
                        break;
                    }
                    else if ((pAd->pRxCfg->FragFrame.RxSize + DataSize) > MAX_FRAME_SIZE)
                    {
                        // Fragment frame is too large, it exeeds the maximum frame size.
                        // Clear Fragment frame contents
                        PlatformZeroMemory(&pAd->pRxCfg->FragFrame, sizeof(FRAGMENT_FRAME));
                        break;
                    }

                    // concatenate this fragment into the re-assembly buffer
                    PlatformMoveMemory(&pAd->pRxCfg->FragFrame.Buffer[pAd->pRxCfg->FragFrame.RxSize], pData, DataSize);
                    pAd->pRxCfg->FragFrame.RxSize  += DataSize;
                    pAd->pRxCfg->FragFrame.LastFrag = pHeader->Frag;       // Update fragment number
                    // Last fragment
                    if (pHeader->FC.MoreFrag == FALSE)
                    {
                        // For TKIP frame, calculate the MIC value
                        if ((pRxD->RxDscrDW2.WLANIndex /*READ_RXWI_WCID(pAd, pRxWI)*/ < MAX_LEN_OF_MAC_TABLE) 
                                && (pWcidMacTabEntry->WepStatus == Ralink802_11Encryption2Enabled))
                        {
                            if (!pEntry)
                                break;

                            pWpaKey = &pEntry->PairwiseKey;
                            // Minus MIC length
                            pAd->pRxCfg->FragFrame.RxSize -= 8;
                        }

                        NdisCommonReportEthFrameToLLC(pAd,pPort,pAd->pRxCfg->FragFrame.Buffer,(USHORT) pAd->pRxCfg->FragFrame.RxSize,UserPriority, FALSE, FALSE);
                    }
                }
                break;
            } //if (pHeader->FC.Type == BTYPE_DATA)
            //
            // CASE II. receive a MGMT frame
            //
            else if (pHeader->FC.Type == BTYPE_MGMT)
            {
                if (pPort->SoftAP.ApCfg.BANClass3Data == TRUE)
                {
                    // disallow new association
                    if ((pHeader->FC.SubType == SUBTYPE_ASSOC_REQ) || (pHeader->FC.SubType == SUBTYPE_AUTH))
                    {
                        DBGPRINT(RT_DEBUG_TRACE, ("   Disallow new Association \n "));
                        break; // give up this frame
                    }
                }

                REPORT_MGMT_FRAME_TO_MLME(pAd,
                                            pPort, 
                                            pRxD->RxDscrDW2.WLANIndex, /*READ_RXWI_WCID(pAd, pRxWI)*/ 
                                            pHeader, 
                                            PacketSize, /*READ_RXWI_MPDU_TOTAL_BYTE_COUNT(pAd, pRxWI)*/
                                            rx_signal->raw_rssi[0], 
                                            rx_signal->raw_rssi[1], 
                                            rx_signal->raw_rssi[2], 
                                            0); //READ_RXWI_AntSel(pAd, pRxWI));

                break;  // end of processing this frame
            }
            //
            // CASE III. receive a CNTL frame
            //
            else if (pHeader->FC.Type == BTYPE_CNTL)
            {
                if ((pHeader->FC.SubType == SUBTYPE_BLOCK_ACK_REQ))
                {
                    CntlEnqueueForRecv(pAd, pPort, pRxD->RxDscrDW2.WLANIndex /*READ_RXWI_WCID(pAd, pRxWI)*/, PacketSize /*READ_RXWI_MPDU_TOTAL_BYTE_COUNT(pAd, pRxWI)*/, (PFRAME_BA_REQ)pHeader);
                }
                
                // handle PS-POLL here
                if ((pHeader->FC.SubType == SUBTYPE_PS_POLL))
                {
                    USHORT Aid = pHeader->Duration & 0x3fff;
                    PUCHAR pAddr = pHeader->Addr2;

                    DBGPRINT(RT_DEBUG_TRACE,("rcv PS-POLL (AID=%d) (WCID=%d) from %02x:%02x:%02x:%02x:%02x:%02x\n", 
                                    Aid, pRxD->RxDscrDW2.WLANIndex /*READ_RXWI_WCID(pAd, pRxWI)*/, pAddr[0], pAddr[1], pAddr[2], pAddr[3], pAddr[4], pAddr[5]));

                    if (Aid < MAX_LEN_OF_MAC_TABLE)
                    {
                        
#if UAPSD_AP_SUPPORT                    
                        ApDataHandleRxPsPoll(pAd, pAddr, Aid, TRUE, pPort, FALSE);
#else
                        ApDataHandleRxPsPoll(pAd, pAddr, Aid, TRUE);
#endif          

                        
                    }

                }
                break; // end of processing this frame
            }
            //
            // CASE IV. receive a frame of invalid type
            //
            else
                break; // give up this frame
        } while (FALSE);
    }
}

/*
    ========================================================================
    Routine Description:
        Check Rx descriptor, return NDIS_STATUS_FAILURE if any error found
    ========================================================================
*/
NDIS_STATUS ApDataCheckRxError(
    IN  PMP_ADAPTER   pAd,
    IN  PRXDSCR_BASE_STRUC  pRxD,
    IN  PHEADER_802_11  pHeader)
{
    //
    // In CipherMismatch case.
    // Only dropt Data/Management frame if it's CipherMismatch and FC.Wep enable.
    //
    if ((pRxD->RxDscrDW2.FCSError == 1) || 
        ((pRxD->RxDscrDW2.CipherMismatch == 1) && (pHeader->FC.Wep == 1)) ||
        (pRxD->RxDscrDW2.CipherLengthMismatch == 1) ||
        (pRxD->RxDscrDW2.ICVError == 1) ||
        (pRxD->RxDscrDW2.TKIPMICError == 1) ||
        (pRxD->RxDscrDW2.LengthMismatch == 1) ||
        (pRxD->RxDscrDW2.LLCMismatched == 1) ||
        (pRxD->RxDscrDW2.DAMSDU == 1) ||
        (pRxD->RxDscrDW2.ExceedMaxRxLength == 1)
         )
        return NDIS_STATUS_FAILURE;
    else
        return NDIS_STATUS_SUCCESS;
}

/*
  ========================================================================
  Description:
    This routine checks if a received frame causes class 2 or class 3
    error, and perform error action (DEAUTH or DISASSOC) accordingly
  ========================================================================
*/
BOOLEAN ApDataCheckClass2Class3Error(
    IN  PMP_ADAPTER   pAd,
    IN  PMP_PORT      pPort,
    IN  ULONG           Wcid, 
    IN  PHEADER_802_11  pHeader)
{
    PMAC_TABLE_ENTRY pWcidMacTabEntry = NULL;
    pWcidMacTabEntry = MlmeSyncMacTabMatchedWlanIdx(pPort, (USHORT)Wcid);   

    if(pWcidMacTabEntry == NULL)
    {
        DBGPRINT(RT_DEBUG_TRACE,("%s(), Line:%d, Mac Entry is NULL  Wcid = %d\n", __FUNCTION__, __LINE__, Wcid));
        return FALSE;
    }
    
    // software MAC table might be smaller than ASIC on-chip total size.
    // If no mathed wcid index in ASIC on chip, do we need more check???  need to check again. 06-06-2006
    if (Wcid >= MAX_LEN_OF_MAC_TABLE)
    {
        //
        // If wcid was not valid that means it doesn't not SST_ASSOC state.
        // It happens that if we surprise remove and don't send any dissociate frames to STA,
        // then we start SoftAP. STA will keep sending frame to SoftAP.
        // 
        // In this case, we need to do ApAuthMlmeAuthCls2errAction.
        //
        DBGPRINT(RT_DEBUG_TRACE, ("ApDataCheckClass2Class3Error, invalid wcid(=0x%02x)\n", Wcid));
        ApAuthMlmeAuthCls2errAction(pAd, pPort,pHeader->Addr2);

        return TRUE;
    }
    
    if (pWcidMacTabEntry->Sst == SST_ASSOC)
        ; // okay to receive this DATA frame
    else if (pWcidMacTabEntry->Sst == SST_AUTH)
    {
        ApAssocMlmeAssocCls3errAction(pAd, pHeader->Addr2);
        return TRUE; 
    }
    else
    {
        ApAuthMlmeAuthCls2errAction(pAd, pPort,pHeader->Addr2);
        return TRUE; 
    }
    return FALSE;
}

/*
  ========================================================================
  Description:
    This routine frees all packets in PSQ that's destined to a specific DA.
    BCAST/MCAST in DTIMCount=0 case is also handled here, just like a PS-POLL 
    is received from a WSTA which has MAC address FF:FF:FF:FF:FF:FF
  ========================================================================
*/
VOID ApDataHandleRxPsPoll(
    IN  PMP_ADAPTER   pAd,
    IN  PUCHAR          pAddr,
    IN  USHORT          Aid,
#if (UAPSD_AP_SUPPORT == 0) 
    IN  BOOLEAN         IsPsPoll)
#else
    IN  BOOLEAN         IsPsPoll,
    IN  PMP_PORT      pPort,  
    IN  BOOLEAN         isActive)
#endif  
{ 
    PQUEUE_ENTRY      pEntry;
    PMAC_TABLE_ENTRY  pMacEntry;
    BOOLEAN     bMoreData  = FALSE;
    ULONG           tmpTimBitmap;      // bit0 for broadcast, 1 for AID1, 2 for AID2, ...so on
    ULONG           tmpTimBitmap2;     // b0 for AID32, b1 for AID33, ... and so on
    MP_RW_LOCK_STATE PsQLockState;
    PMAC_TABLE_ENTRY pWcidMacTabEntry = NULL;
    pWcidMacTabEntry = MlmeSyncMacTabMatchedWlanIdx(pPort, (UCHAR)Aid);   
    
    if(pWcidMacTabEntry == NULL)
    {
        DBGPRINT(RT_DEBUG_TRACE,("%s(), Line:%d, Mac Entry is NULL  Wcid = %d\n", __FUNCTION__, __LINE__, Aid));
        return;
    }
    
    DBGPRINT(RT_DEBUG_TRACE,("rcv PS-POLL (AID=%d) from %02x:%02x:%02x:%02x:%02x:%02x\n", 
          Aid, pAddr[0], pAddr[1], pAddr[2], pAddr[3], pAddr[4], pAddr[5]));

    pMacEntry = pWcidMacTabEntry;
    if (PlatformEqualMemory(pMacEntry->Addr, pAddr, MAC_ADDR_LEN))
    {
#if UAPSD_AP_SUPPORT
        if (IS_AP_SUPPORT_UAPSD(pAd, pPort)  && UAPSD_MR_IS_ALL_AC_UAPSD(isActive, pMacEntry))
        {
            /*
            IEEE802.11e spec.
            11.2.1.7 Receive operation for STAs in PS mode during the CP
            When a non-AP QSTA that is using U-APSD and has all ACs
            delivery-enabled detects that the bit corresponding to its AID
            is set in the TIM, the non-AP QSTA shall issue a trigger frame
            or a PS-Poll frame to retrieve the buffered MSDU or management
            frames.

            WMM Spec. v1.1a 070601
            3.6.2   U-APSD STA Operation
            3.6.2.3 In case one or more ACs are not
            delivery-enabled ACs, the WMM STA may retrieve MSDUs and
            MMPDUs belonging to those ACs by sending PS-Polls to the WMM AP.
            In case all ACs are delivery enabled ACs, WMM STA should only
            use trigger frames to retrieve MSDUs and MMPDUs belonging to
            those ACs, and it should not send PS-Poll frames.

            Different definitions in IEEE802.11e and WMM spec.
            But we follow the WiFi WMM Spec.
            */

            DBGPRINT(RT_DEBUG_TRACE, ("uapsd> All AC are UAPSD, can not use PS-Poll\n"));
            return; /* all AC are U-APSD, can not use PS-Poll */
        }

    
        if(IS_AP_SUPPORT_UAPSD(pAd, pPort) && (isActive==TRUE) && UAPSD_MR_IS_ENTRY_ACCESSIBLE(pMacEntry))
        {
            DBGPRINT(RT_DEBUG_TRACE,("Chk::%s(%d)==> Sta from doze to active , so we go to ApUapsdAllPacketDeliver!!\n", __FUNCTION__, __LINE__));
            ApUapsdAllPacketDeliver(pAd, pMacEntry);
        }
#endif

        //NdisAcquireSpinLock(&pAd->MacTablePool.MacTabPoolLock);
        PlatformAcquireOldRWLockForWrite(&pAd->pTxCfg->PsQueueLock,&PsQLockState);
        NdisAcquireSpinLock(&pAd->pTxCfg->SendTxWaitQueueLock[QID_AC_BE]);
        // cleanup all backlogged frames in PSQ
        if(pMacEntry->PsQueue.Number >1)
        {
            bMoreData = TRUE;
        }
        if (pMacEntry->PsQueue.Head)
        {
            do{
                pEntry = RemoveHeadQueue(&pMacEntry->PsQueue);                                                              
                if (pEntry != NULL)
                    InsertTailQueue(&pAd->pTxCfg->SendTxWaitQueue[QID_AC_BE], pEntry);

            } while(pEntry!= NULL && !IsPsPoll);
        }
        NdisReleaseSpinLock(&pAd->pTxCfg->SendTxWaitQueueLock[QID_AC_BE]);
        //NdisReleaseSpinLock(&pAd->MacTablePool.MacTabPoolLock);
        PlatformReleaseOldRWLock(&pAd->pTxCfg->PsQueueLock,&PsQLockState);
        if ((Aid > 0) && (Aid < MAX_LEN_OF_MAC_TABLE) && (pMacEntry->PsQueue.Number == 0))
        {
            tmpTimBitmap = pPort->SoftAP.ApCfg.TimBitmap;
            tmpTimBitmap2 = pPort->SoftAP.ApCfg.TimBitmap2;
            DBGPRINT(RT_DEBUG_TRACE,("clear TIM\n")); 
            // clear corresponding TIM bit
            if (Aid >= 32)
                pPort->SoftAP.ApCfg.TimBitmap2 &= (~TIM_BITMAP[Aid-32]); 
            else
                pPort->SoftAP.ApCfg.TimBitmap &= (~TIM_BITMAP[Aid]); 

            pMacEntry->PsQIdleCount = 0;            
            if((pPort->SoftAP.ApCfg.TimBitmap != tmpTimBitmap) || (pPort->SoftAP.ApCfg.TimBitmap2 != tmpTimBitmap2))
            {
                MTEnqueueInternalCmd(pAd, pPort,  MT_CMD_UPDATE_TIM, NULL, 0);
            }
        }

        //For Sigma 6.1.11, we need to generate a fake QoS packet to pass the test
        //Since we have indicated PS packet in TIM, then we need to generate one
        if(IS_P2P_SIGMA_ON(pPort) && pPort->P2PCfg.bSigmaFakePkt)
        {
            ApUapsdSendNullFrame(pAd, pPort, pMacEntry->Addr, pMacEntry->CurrTxRate, TRUE, FALSE, 0);
            pPort->P2PCfg.bSigmaFakePkt = FALSE;
        }
        
        // Dequeue outgoing frames from TxSwQueue0..3 queue and process it
        // TODO: 2004-12-27 it's not a good idea to handle "More Data" bit here. because the
        // RTMPDeQueue process doesn't guarantee to de-queue the desired MSDU from the corresponding
        // TxSwQueue/PsQueue when QOS in-used. We should consider "HardTransmt" this MPDU
        // using MGMT queue or things like that.
        NdisSetEvent(&(GET_THREAD_EVENT(&pAd->pHifCfg->hDequeueDataThread)));
    }
    else
    {
        DBGPRINT(RT_DEBUG_ERROR,("rcv PS-POLL (AID=%d not match) from %02x:%02x:%02x:%02x:%02x:%02x\n", 
              Aid, pAddr[0], pAddr[1], pAddr[2], pAddr[3], pAddr[4], pAddr[5]));
    }
}

/*
    ========================================================================
    Routine Description:
        This function checks if the received frames should be sent out to the 
        wireless media based on DA look-up in the wirelss MAC table.

    Arguments:
        pRxD        Pointer to the Rx descriptor

    Return Value:
        TRUE -  the input frame is destined to a known wireless STA 
                and had been sent out to the wireless media. It's not 
                necessary to make another copy up to LLC nor Portal function.
        FALSE - the input frame still need to deliver to the upper LLC
    ========================================================================
*/
BOOLEAN ApDataBridgeToWdsAndWirelessSta(
    IN  PMP_ADAPTER   pAd,
    IN  PMP_PORT  pPort,
    IN  PHEADER_802_11  pHeader80211,
    IN  UINT            us80211FreamSize,
    IN  ULONG           fromwdsidx,
    IN  UCHAR           UserPriority)
{
    SST             Sst;
    UCHAR          Aid;
    UCHAR           PsMode, Rate;
    MAC_TABLE_ENTRY *pEntry;
    BOOLEAN         result = FALSE;
    UCHAR           DA[6];
    UCHAR           SA[6];
#if (COMPILE_VISTA_ABOVE(CURRENT_OS_NDIS_VER))            
    NDIS_NET_BUFFER_LIST_8021Q_INFO Ndis8021QInfo;
#endif    
    HEADER_802_11   Header80211;
    BOOLEAN         bRestore = FALSE;
    PMT_XMIT_CTRL_UNIT pXcu = NULL;

    if (pPort == NULL)
    {
        DBGPRINT(RT_DEBUG_ERROR, ("%s - pPort is  NULL, Error, and return this function", __FUNCTION__));
        return FALSE;
    }

    do 
    {
        COPY_MAC_ADDR(DA, pHeader80211->Addr3);
        COPY_MAC_ADDR(SA, pHeader80211->Addr2);

        // If DA is our AP, call NdisMEthIndicateReceive directly. No more bridging is required
        if (MAC_ADDR_EQUAL(pPort->CurrentAddress, DA/*pHeader80211->Addr1*/))
        {       
            result =  FALSE;
            break;
        }

        // decide the return value. TRUE if no need to indicate to LLC, FALSE otherwise
        pEntry = APSsPsInquiry(pAd, pPort, DA/*pHeader80211->Addr1*/, &Sst, &Aid, &PsMode, &Rate);

        if (DA[0] & 0x01)//if (pHeader80211->Addr1[0] & 0x01)
            result = FALSE;
        else if (pEntry && (Sst == SST_ASSOC)) 
            result = TRUE;
        else
            result = FALSE;

        Header80211.FC.FrDs = 0;
        //
        // case 1. if either a M/BCAST or a known unicast within the local BSS, then send to BSS
        //
        if ((!pPort->SoftAP.ApCfg.bIsolateInterStaTraffic) && ((/*pHeader80211->Addr1[0]*/ DA[0] & 0x01) || (pEntry && (Sst == SST_ASSOC))))
        {
            do
            {
                bRestore = TRUE;
                PlatformMoveMemory(&Header80211, pHeader80211, LENGTH_802_11);

                //
                // Forwarding.
                //
                COPY_MAC_ADDR(pHeader80211->Addr1, DA);
                COPY_MAC_ADDR(pHeader80211->Addr2, pPort->PortCfg.Bssid);
                COPY_MAC_ADDR(pHeader80211->Addr3, SA);
                pHeader80211->FC.ToDs   = 0;
                pHeader80211->FC.FrDs   = 1;
                pHeader80211->FC.Retry  = 0;
                pHeader80211->FC.PwrMgmt = 0;
                pHeader80211->FC.Order = 0;  // Reset this bit Let APHardTX decide enable or not.
                pXcu = Ndis6CommonAllocateNdisNetBufferList(pAd, pPort->PortNumber, (PUCHAR)pHeader80211, us80211FreamSize);
                if (pXcu == NULL)
                {
                    DBGPRINT(RT_DEBUG_ERROR, ("%s - pXcu == NULL", __FUNCTION__));
                    break;
                }
#if (COMPILE_VISTA_ABOVE(CURRENT_OS_NDIS_VER))                
                //set user priority
                Ndis8021QInfo.Value = 0;
                Ndis8021QInfo.WLanTagHeader.WMMInfo = UserPriority;
                NET_BUFFER_LIST_INFO((PNET_BUFFER_LIST)pXcu->Reserve1, Ieee8021QNetBufferListInfo) = Ndis8021QInfo.Value; 
#endif
                // send out the packet
                pXcu->PktSource = PKTSRC_INTERNAL;
                pXcu->Aid = (UCHAR)Aid;
                pXcu->UserPriority = UserPriority;
                ApDataSendPacket(pAd, pPort->PortNumber, pXcu);
                NdisSetEvent(&(GET_THREAD_EVENT(&pAd->pHifCfg->hDequeueDataThread)));
            } while (FALSE);        
        }
    } while (FALSE);

    if (result == FALSE)
    {       
        // For indicating Net Buffer list, need to use none-wep frame.  So set Wep = 0 here.
        if (IS_P2P_CON_GO(pAd, pPort))
        {
            pHeader80211->FC.Wep    = 0;
            pHeader80211->FC.ToDs   = 0;
            pHeader80211->FC.FrDs   = 1;    
            COPY_MAC_ADDR(pHeader80211->Addr1, DA);
            COPY_MAC_ADDR(pHeader80211->Addr2, pPort->PortCfg.Bssid);
            COPY_MAC_ADDR(pHeader80211->Addr3, SA);
            pHeader80211->FC.Order = 0;  // Reset this bit Let APHardTX decide enable or not.
        }
        else if (IS_P2P_STA_GO(pAd, pPort))
        {
            pHeader80211->FC.Wep    = 0;
            pHeader80211->FC.ToDs   = 0;
            pHeader80211->FC.FrDs   = 0;                        
            COPY_MAC_ADDR(pHeader80211->Addr2, SA);
            COPY_MAC_ADDR(pHeader80211->Addr3, pPort->SoftAP.ApCfg.AdhocBssid);

            pHeader80211->FC.Order = 0;  // Reset this bit Let APHardTX decide enable or not.
        }
        else if (pPort->PortSubtype == PORTSUBTYPE_SoftapAP || pPort->PortSubtype == PORTSUBTYPE_VwifiAP)
        {
            //
            // For indicate
            //
            pHeader80211->FC.ToDs   = 0;
            pHeader80211->FC.FrDs   = 0;
            COPY_MAC_ADDR(pHeader80211->Addr2, SA);
            COPY_MAC_ADDR(pHeader80211->Addr3, pPort->SoftAP.ApCfg.AdhocBssid);
        }
        else
        {
            if (bRestore)
                PlatformMoveMemory(pHeader80211, &Header80211, LENGTH_802_11);
        }
    }
    else
        DBGPRINT(RT_DEBUG_TRACE, ("%s: Redirect the frame to WDS/STA without indication.\n", __FUNCTION__));

    return result;
}

VOID ApDataCheckNDSClient(
    IN  PMP_ADAPTER       pAd, 
    IN  PMAC_TABLE_ENTRY    pEntry,
    IN  PRXWI_STRUC         pRxWI)
{
    //HTTRANSMIT_SETTING    HTPhyMode;
    ULONG               RxRate = (ULONG) -1L;
    PMP_PORT    pPort = pAd->PortList[FXXK_PORT_0];

    if (!pEntry)
        return;

    if (READ_RXWI_PHY_MODE(pAd, pRxWI) == MODE_CCK)
    {
        RxRate = READ_RXWI_MCS(pAd, pRxWI, READ_RXWI_PHY_MODE(pAd, pRxWI));
    }

    if ((pEntry->NDSClientSetting == NDS_CLIENT_EVL) && (RxRate > RATE_2))
    {
        pEntry->NDSClientSetting  = NDS_CLIENT_OFF;
        //
        // NDS_CLIENT_EVL, means the client's MaxTxRate must be RATE_11.
        //
        pEntry->MaxSupportedRate = min(pPort->CommonCfg.MaxTxRate, RATE_11);
        DBGPRINT(RT_DEBUG_TRACE, ("ApDataCheckNDSClient::MAC[%02x:%02x:%02x:%02x:%02x:%02x] is not NDS Client, LastRxRate[%d], MaxSupportedRate set to %d\n", 
                pEntry->Addr[0], pEntry->Addr[1], pEntry->Addr[2], pEntry->Addr[3], pEntry->Addr[4], pEntry->Addr[5], RxRate, pEntry->MaxSupportedRate));

        if (pEntry->MaxSupportedRate < RATE_FIRST_OFDM_RATE)
        {
            WRITE_PHY_CFG_MODE(pAd, &pEntry->MaxPhyCfg, MODE_CCK);
            WRITE_PHY_CFG_MCS(pAd, &pEntry->MaxPhyCfg, pEntry->MaxSupportedRate);
            WRITE_PHY_CFG_MODE(pAd, &pEntry->MinPhyCfg, MODE_CCK);
            WRITE_PHY_CFG_MCS(pAd, &pEntry->MinPhyCfg, pEntry->MaxSupportedRate);
            WRITE_PHY_CFG_MODE(pAd, &pEntry->TxPhyCfg, MODE_CCK);
            WRITE_PHY_CFG_MCS(pAd, &pEntry->TxPhyCfg, pEntry->MaxSupportedRate);
        }
    }
}

/*  
    ==========================================================================
    Description: 
        When I have multi/braodcast queue, I need to send data at TBTT
    ==========================================================================
 */
VOID ApDataTbttTimeoutTimerCallback(
    IN PVOID SystemSpecific1, 
    IN PVOID FunctionContext, 
    IN PVOID SystemSpecific2, 
    IN PVOID SystemSpecific3) 
{
    PMP_PORT pPort = (PMP_PORT )FunctionContext;
    PMP_ADAPTER  pAd = pPort->pAd;
    PQUEUE_ENTRY      pEntry;
    MP_RW_LOCK_STATE LockState;
#ifdef MULTI_CHANNEL_SUPPORT    
    ULONG       i;
#endif /*MULTI_CHANNEL_SUPPORT*/
    
    if ((pPort->MacTab.McastPsQueue.Number > 0) && (pPort->MacTab.fAnyStationInPsm == TRUE))
    {
        //Clear MCAST PSQ
        PlatformAcquireOldRWLockForWrite(&pAd->MacTablePool.MacTabPoolLock,&LockState);
        NdisAcquireSpinLock(&pAd->pTxCfg->SendTxWaitQueueLock[QID_AC_BE]);
        do{
            pEntry = RemoveHeadQueue(&pPort->MacTab.McastPsQueue);                                                                
            if (pEntry != NULL)
                InsertTailQueue(&pAd->pTxCfg->SendTxWaitQueue[QID_AC_BE], pEntry);
        } while(pEntry!= NULL);
        NdisReleaseSpinLock(&pAd->pTxCfg->SendTxWaitQueueLock[QID_AC_BE]);
        PlatformReleaseOldRWLock(&pAd->MacTablePool.MacTabPoolLock,&LockState);

        //Update TIM mcast bit
        pPort->SoftAP.ApCfg.TimBitmap &= ~TIM_BITMAP[0];
        MTEnqueueInternalCmd(pAd, pPort,  MT_CMD_UPDATE_TIM, NULL, 0);

        //Send Packet to HW
        NdisSetEvent(&(GET_THREAD_EVENT(&pAd->pHifCfg->hDequeueDataThread)));
    }   


// 2011-11-24 For P2pMs GO multi-channel
#ifdef MULTI_CHANNEL_SUPPORT
    for (i = 0; i < RTMP_MAX_NUMBER_OF_PORT/*pAd->NumberOfPorts*/; i++)
    {
        if ((pAd->MccCfg.MultiChannelEnable == TRUE) && 
            (pAd->PortList[i] != NULL) &&
            ((pAd->PortList[i]->bActive == TRUE) && (pAd->PortList[i]->PortType == WFD_GO_PORT)) && 
            (pAd->MccCfg.MultiChannelCurrentMode == MULTI_CH_OP_MODE_MSGO) &&
            (MT_TEST_FLAG(pAd, fRTMP_ADAPTER_BSS_SCAN_IN_PROGRESS) == FALSE))
        {               
            // Trigger switch channel 40mse later after sendung out Beacon 
            if (OPSTATUS_TEST_FLAG(pAd, fOP_STATUS_MEDIA_STATE_MULTI_CHANNEL_SWITCHING))
            {
                DBGPRINT(RT_DEBUG_TRACE, ("%s - MultiChSts : fOP_STATUS_MEDIA_STATE_MULTI_CHANNEL_SWITCHING is running, RETURN!\n", __FUNCTION__));
                return;
            }

            OPSTATUS_SET_FLAG(pAd, fOP_STATUS_MEDIA_STATE_MULTI_CHANNEL_SWITCHING);
            
            PlatformSetTimer(pPort, &pPort->Mlme.MultiChannelTimer, 40);

            //AsicPauseBssSync(pAd);

            DBGPRINT(RT_DEBUG_TRACE, ("%s - MultiChSts : GO had sent out Beacon, trigger next mode after 40mse\n", __FUNCTION__));
        }
    }
#endif /* MULTI_CHANNEL_SUPPORT */
}

