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
    Receive.c

    Abstract:
    Miniport generic portion header file

    Revision History:
    Who         When          What
    --------    ----------    ----------------------------------------------
    */
    
#include    "Recv.h"

#define DRIVER_FILE         0x01200000

UCHAR   SNAP_802_1H[] = {0xaa, 0xaa, 0x03, 0x00, 0x00, 0x00};
UCHAR   SNAP_AMP_LLC[] = {0xaa, 0xaa, 0x03, 0x00, 0x19, 0x58};
UCHAR   SNAP_BRIDGE_TUNNEL[] = {0xaa, 0xaa, 0x03, 0x00, 0x00, 0xf8};
UCHAR   EAPOL_LLC_SNAP[]= {0xaa, 0xaa, 0x03, 0x00, 0x00, 0x00, 0x88, 0x8e};
UCHAR   EAPOL[] = {0x88, 0x8e};


BOOLEAN 
RecvFwCmd7603(
    IN  PMP_ADAPTER  pAd,
    IN  PRX_CONTEXT     pRxContext
    )
{
    PCMD_EVENT_FROM_FW pCmdEventFromFW = NULL;
    PCMD_RXDSCR pCmdRxDscr = NULL;
    
    // for MT7603. need to modify...
    pCmdEventFromFW = (PCMD_EVENT_FROM_FW)pRxContext->TransferBuffer;

    if (pCmdEventFromFW != NULL)
    {
        //DbgPrint("received packet from ANDES, TransferBufferLength = %d\n",TransferBufferLength);
        //DumpFrame((PUCHAR)pRxFceInfo, TransferBufferLength);
        //if (READ_RX_FCE_INFO_INFO_TYPE(pAd,pRxFceInfo) == INFO_TYPE_RSP_EVT)
        DBGPRINT(RT_DEBUG_INFO,("N6UsbRxStaPacket : pCmdEventFromFW->FwRxD.PacketType = %x. pCmdEventFromFW->FwRxD.EID = %x\n", pCmdEventFromFW->FwRxD.PacketType, pCmdEventFromFW->FwRxD.EID));
        if (pCmdEventFromFW->FwRxD.PacketType == 0xE000 && (pCmdEventFromFW->FwRxD.EID == 0x01 || pCmdEventFromFW->FwRxD.EID == 0xEF || pCmdEventFromFW->FwRxD.EID == 0x18 || pCmdEventFromFW->FwRxD.EID == 0xED))
        //if (pCmdEventFromFW->FwRxD.PacketType == 0xE000 && pCmdEventFromFW->FwRxD.EID == 0x01)
        {
            DBGPRINT(RT_DEBUG_INFO,("N6UsbRxStaPacket : Event Status = %d. EID = 0x%x\n", pCmdEventFromFW->Status, pCmdEventFromFW->FwRxD.EID));
            //RxRspEvtHandle(pAd, pRxFceInfo, 0);  // implement later                   

#if 0
            {
                int i = 0;
                PUCHAR p = (PUCHAR)&pRxContext->TransferBuffer[0];
                for (i = 0; i < (int)pRxContext->TransferBufferLength; i = i + 8)
                {
                    DBGPRINT_RAW(RT_DEBUG_ERROR, ("[%d] %x %x %x %x %x %x %x %x\n", i, p[i], p[i+1], p[i+2], p[i+3], p[i+4], p[i+5], p[i+6], p[i+7]));
                }
            }
#endif                  

            pCmdRxDscr = (PCMD_RXDSCR)pRxContext->TransferBuffer;

            //if (pCmdEventFromFW->FwRxD.Status != 0)
            if (pCmdRxDscr->EID == 0xED && pCmdRxDscr->ExtendEID == 0x0)
            {
                EVENT_EXT_CMD_RESULT_T *pEventChSwitch = (EVENT_EXT_CMD_RESULT_T *)pCmdRxDscr->Buffer;
                DBGPRINT(RT_DEBUG_TRACE,("Channel Switch Event Status = %d. \n", pEventChSwitch->u4Status));
            }
            else if (pCmdEventFromFW->Status != 0)
            {
                DBGPRINT(RT_DEBUG_TRACE,("Rx CMD Event Status = %d. \n", pCmdEventFromFW->Status));
            }

            pRxContext->Readable = FALSE;
            pRxContext->ReadPosOffset = 0;
            pRxContext = &pAd->pHifCfg->RxContext[pAd->pHifCfg->NextRxBulkInReadIndex];   

            NdisSetEvent(&(pAd->HwCfg.WaitFWEvent));
            return TRUE;
        }
    }  
    return FALSE;
}

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
    )
{
    DBGPRINT(RT_DEBUG_INFO, ("Receive [%d] Ucast [%d]\n", pHeader->Sequence, pRxD->RxDscrDW1.UnicastToMe));
    // there's packet sent to me, keep awake for 1200ms
    if (pAd->TrackInfo.CountDowntoPsm < 12)
    {
        DBGPRINT(RT_DEBUG_TRACE,("N6UsbRxStaPacket --> pAd->TrackInfo.CountDowntoPsm = 12 !!!\n"));
        pAd->TrackInfo.CountDowntoPsm = 12;
    }
    
    if (DataSize == 0)
    {
        DBGPRINT(RT_DEBUG_TRACE, ("%s: Discard the data packet with zero payload, seq(%d)\n",
            __FUNCTION__,
            pHeader->Sequence));

        return;
    }
    // Update OneSecRxOkDataCnt 
    pAd->Counter.MTKCounters.OneSecRxOkDataCnt++;


    // Non-fragmented packet.
    if ((pHeader->Frag == 0) && (pHeader->FC.MoreFrag == FALSE))
    {
        // Special DATA frame that has to pass to MLME
        //   1. EAPOL handshaking frames when driver supplicant enabled, pass to MLME for special process
        //   2. Receive EAP packet for WSC only after enrolee link up to AP
        //   3. IEEE 802.11z TDLS Action Data frames 
        if ((PlatformEqualMemory(EAPOL_LLC_SNAP, pData, LENGTH_802_1_H) && 
            ((pAd->StaCfg.WpaState != SS_NOTUSE) || (pPort->StaCfg.WscControl.WscState >= WSC_STATE_LINK_UP))) ||
            (PlatformEqualMemory(TDLS_LLC_SNAP_WITH_CATEGORY, pData, LENGTH_802_1_H + 2)))
        {
            DataSize += LENGTH_802_11;

            REPORT_MGMT_FRAME_TO_MLME(pAd, 
                                    pPort,
                                    pRxD->RxDscrDW2.WLANIndex,
                                    pHeader, 
                                    DataSize, 
                                    rx_signal->raw_rssi[0], 
                                    rx_signal->raw_rssi[1], 
                                    rx_signal->raw_rssi[2], 
                                    0);

            DBGPRINT_RAW(RT_DEBUG_TRACE, ("!!! report EAPOL/AIRONET DATA to MLME (non-fragmented packet) (len=%d) !!!\n", DataSize));
            return;  // end of processing this frame
        }
        else if(PlatformEqualMemory(EAPOL_LLC_SNAP, pData, LENGTH_802_1_H) && 
             (pPort->StaCfg.WscControl.WscState > WSC_STATE_OFF) &&
             (pPort->StaCfg.WscControl.WscState < WSC_STATE_LINK_UP))
        {
            PIEEE8021X_FRAME    p802_1x;
            p802_1x = (PIEEE8021X_FRAME)(pData + LENGTH_802_1_H);
            if(p802_1x->Type == 0)  //802.1x Auth packet type is EAP-Packet
            {
                EAP_FRAME   *EAP;
                EAP = (PEAP_FRAME)(pData + LENGTH_802_1_H + sizeof(IEEE8021X_FRAME));
                if(EAP->Type == 1)  //Identity
                {
                    //Receive EAP-Identity when we want to connect an AP to do WPS
                    //For Marvell test bed AP, queue EAP-identify packet
                    //Marvell AP will send EAP-identify right after send assoc_rsp,
                    //we haven't change the state to link up so we will ignore the EAP-identify since we are not ready to do WPS.
                    //However, the AP won't send EAP-identify again when we are ready to do WPS even AP receive our EAP-identify.
                    if (pPort->StaCfg.WscControl.bQueueEAPOL)
                    {
                        pPort->StaCfg.WscControl.RxEAPOLLen = 0;
                        if (pPort->StaCfg.WscControl.pRxEAPOL == NULL)
                        {
                            MlmeAllocateMemory(pAd, &pPort->StaCfg.WscControl.pRxEAPOL);
                        }

                        // Queue the EAPOL packet.
                        if ((pPort->StaCfg.WscControl.pRxEAPOL != NULL) && 
                            (DataSize <= MAX_LEN_OF_MLME_BUFFER))
                        {                                                       
                            PlatformMoveMemory((PVOID)pPort->StaCfg.WscControl.pRxEAPOL, (PCHAR)pHeader, PacketSize /*READ_RXWI_MPDU_TOTAL_BYTE_COUNT(pAd, pRxWI)*/); // Queue the Header.
                                                                                        
                            pPort->StaCfg.WscControl.RxEAPOLLen= PacketSize /*READ_RXWI_MPDU_TOTAL_BYTE_COUNT(pAd, pRxWI)*/; // The packet size including the 802.11 header and data payload.

                            // ToDo
                            //PlatformMoveMemory((PVOID)&(pPort->StaCfg.WscControl.RxWI), (PVOID)pRxWI, pAd->HwCfg.RXWI_Length);

                            //pAd->BtCfg.UserPriorityOfQueuedEAPOL = UserPriority; // The user priority of the queued EAPOL packet.
                            DBGPRINT(RT_DEBUG_TRACE, ("%s: Queue the EAP-Identify packet (pHeader->Sequence = %d; pAd->pRxCfg->RxEAPOLLen = %d)\n", 
                                __FUNCTION__, pHeader->Sequence, pPort->StaCfg.WscControl.RxEAPOLLen));

                        }
                    }
                    DBGPRINT(RT_DEBUG_TRACE, ("[Marvell WPS Testbed]Should Queue the EAP-Identify packet! We will indicate it just after Link Up\n"));
                    return;
                }
            }
        }

    }

    //In safe mode(bSafeModeEnabled), we indicate each fragment direct to OS. Safe mode's OS is responsible for handling this.                          
    if ( (pHeader->Frag == 0) || (pPort->CommonCfg.bSafeModeEnabled == 1) )   // First or Only fragment
    {
        pAd->pRxCfg->FragFrame.Flags &= 0xFFFFFFFE;

        // Firt Fragment & LLC/SNAP been removed. Keep the removed LLC/SNAP for later on
        // TKIP MIC verification.
        if (pHeader->FC.MoreFrag)
        {
            pAd->pRxCfg->FragFrame.Flags |= 0x01;
        }

        // One & The only fragment
        if ( (pHeader->FC.MoreFrag == FALSE) || (pPort->CommonCfg.bSafeModeEnabled == 1) )
        {
            if ((pHeader->FC.Order == 1)  && (Msdu2Size > 0)) // this is an aggregation
            {
                USHORT Payload1Size, Payload2Size;

                pAd->Counter.MTKCounters.OneSecRxAggregationCount ++;
                Payload1Size = DataSize - Msdu2Size;
                Payload2Size = Msdu2Size - LENGTH_802_3;
                NdisCommonReportEthFrameToLLCAgg(pAd,pPort, (PUCHAR)pHeader, Payload1Size, Payload2Size, UserPriority);
            }
            else if (bAMsdu)
            {
                BOOLEAN     bStatus;
                if ((!pRxD->RxDscrDW2.NonAMPDUFrame)/*(READ_RXINFO_BA(pAd, pRxInfo) == TRUE)*/ && (pHeader->FC.SubType != SUBTYPE_QOS_NULL))
                {
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
                        NdisCommonReportAMSDUEthFrameToLLC(pAd,pPort, (PUCHAR)pHeader, DataSize + LENGTH_802_11, UserPriority, FALSE);
                    }
                }
                else
                {
                    NdisCommonReportAMSDUEthFrameToLLC(pAd,pPort, (PUCHAR)pHeader, DataSize + LENGTH_802_11, UserPriority, FALSE);
                }
            }
            else
            {
                UCHAR   Returnbrc;
#if DBG     
                if(EAPOLFrame)
                    DBGPRINT(RT_DEBUG_TRACE, ("Indicate the EAPOL packet \n"));
#endif

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
                        NdisCommonReportEthFrameToLLC(pAd, pPort,(PUCHAR)pHeader, DataSize + LENGTH_802_11,UserPriority, FALSE, FALSE);
                    }
                }
                else
                {                                       
                    if (QueueEAPOLFrame)
                    {
// Allocate memory for queuing the EAPOL packet.
                        if (pAd->pRxCfg->pRxEAPOL == NULL)
                        {
                            MlmeAllocateMemory(pAd, &pAd->pRxCfg->pRxEAPOL);
                        }

                        // Queue the EAPOL packet.
                        if ((pAd->pRxCfg->pRxEAPOL != NULL) && (DataSize <= MAX_LEN_OF_MLME_BUFFER))
                        {
                            PlatformMoveMemory((PVOID)pAd->pRxCfg->pRxEAPOL, (PVOID)pHeader, (DataSize + LENGTH_802_11)); // Queue the 802.11 header and the data payload.
                            pAd->pRxCfg->RxEAPOLLen = (DataSize + LENGTH_802_11); // The packet size including the 802.11 header and data payload.
                            pAd->pRxCfg->UserPriorityOfQueuedEAPOL = UserPriority; // The user priority of the queued EAPOL packet.
                            DBGPRINT(RT_DEBUG_TRACE, ("%s: Queue the EAPOL packet (pHeader->Sequence = %d; pAd->pRxCfg->RxEAPOLLen = %d)\n", 
                                __FUNCTION__, pHeader->Sequence, pAd->pRxCfg->RxEAPOLLen));
                        }
                    }
                    else
                    {

#if DBG
                        if(WAIFrame)
                            DumpFrameMessage((PUCHAR)pHeader, (USHORT)DataSize, ("Indicate WAI packets to OS\n"))
                        if(EAPOLFrame)
                            DBGPRINT(RT_DEBUG_TRACE, ("Indicate the EAPOL packet \n"));
#endif
                        {
                            NdisCommonReportEthFrameToLLC(pAd, pPort,(PUCHAR)pHeader, DataSize + LENGTH_802_11,UserPriority, FALSE, FALSE);
                        }
                    }
                } 
            } 
        } //First or Only fragment
        else
        {
            PlatformMoveMemory(pAd->pRxCfg->FragFrame.Buffer, pHeader, DataSize + LENGTH_802_11);
            pAd->pRxCfg->FragFrame.RxSize    = DataSize + LENGTH_802_11;
            pAd->pRxCfg->FragFrame.Sequence = pHeader->Sequence;
            pAd->pRxCfg->FragFrame.LastFrag = pHeader->Frag;        // Should be 0
        }
    } //if (pHeader->Frag == 0)
    else
    {
        // No LLC-SNAP header in except the first fragment frame
        if ((pHeader->Sequence != pAd->pRxCfg->FragFrame.Sequence) ||
            (pHeader->Frag != (pAd->pRxCfg->FragFrame.LastFrag + 1)))
        {
            // Fragment is not the same sequence or out of fragment number order
            // Clear Fragment frame contents
            PlatformZeroMemory(&pAd->pRxCfg->FragFrame, sizeof(FRAGMENT_FRAME));
            return;
        }
        else if ((pAd->pRxCfg->FragFrame.RxSize + DataSize) > MAX_FRAME_SIZE)
        {
            // Fragment frame is too large, it exeeds the maximum frame size.
            // Clear Fragment frame contents
            PlatformZeroMemory(&pAd->pRxCfg->FragFrame, sizeof(FRAGMENT_FRAME));
            return; // give up this frame                                        
        }

        // concatenate this fragment into the re-assembly buffer
        PlatformMoveMemory(&pAd->pRxCfg->FragFrame.Buffer[pAd->pRxCfg->FragFrame.RxSize], pData, DataSize);
        pAd->pRxCfg->FragFrame.RxSize   += DataSize;
        pAd->pRxCfg->FragFrame.LastFrag = pHeader->Frag;        // Update fragment number


        // Last fragment
        if (pHeader->FC.MoreFrag == FALSE)
        {  
            pData = pAd->pRxCfg->FragFrame.Buffer;

            // Reconstructed fragmented packet.
            if ((PlatformEqualMemory(EAPOL_LLC_SNAP, (pData + LENGTH_802_11), LENGTH_802_1_H) && 
                ((pAd->StaCfg.WpaState != SS_NOTUSE) || (pPort->StaCfg.WscControl.WscState >= WSC_STATE_LINK_UP))))
            {
                REPORT_MGMT_FRAME_TO_MLME(pAd,
                                        pPort, 
                                        pRxD->RxDscrDW2.WLANIndex, 
                                        pData, 
                                        pAd->pRxCfg->FragFrame.RxSize, 
                                        rx_signal->raw_rssi[0], 
                                        rx_signal->raw_rssi[1], 
                                        rx_signal->raw_rssi[2], 
                                        0);

                DBGPRINT_RAW(RT_DEBUG_TRACE, ("!!! report EAPOL/AIRONET DATA to MLME (reconstructed fragmented packet) (len=%d) !!!\n", pAd->pRxCfg->FragFrame.RxSize));
                return; // end of processing this frame.
            }
            else
            {
                NdisCommonReportEthFrameToLLC(pAd, pPort,pData, (USHORT)pAd->pRxCfg->FragFrame.RxSize,UserPriority, FALSE, FALSE);
            }
        }
    }
}

VOID
RecvProcessBroadMcastDataPkt(
    IN  PMP_ADAPTER  pAd, 
    IN  PMP_PORT      pPort,
    IN  PHEADER_802_11  pHeader,
    IN  PRXDSCR_BASE_STRUC pRxD,
    IN  SHORT               DataSize,
    IN  UCHAR           UserPriority
    )
{   
        DBGPRINT(RT_DEBUG_INFO, ("Receive [%d] B/Mcast [%d/%d]\n", pHeader->Sequence, pRxD->RxDscrDW1.Broadcast, pRxD->RxDscrDW1.Multicast));
        INC_COUNTER64(pAd->Counter.WlanCounters.MulticastReceivedFrameCount);

        // Drop Mcast/Bcast frame with fragment bit on
        if (pHeader->FC.MoreFrag)
        {
            DBGPRINT(RT_DEBUG_ERROR, ("Drop Mcast/Bcast frame with fragment bit on\n"));
            return; // give up this frame
        }

        // Filter out Bcast frame which AP relayed for us
        if (pHeader->FC.FrDs && MAC_ADDR_EQUAL(pHeader->Addr3, pPort->CurrentAddress))
        {
            return; // give up this frame    
        }

       // stay awake for at least 1 second
        pAd->TrackInfo.CountDowntoPsm = (pAd->TrackInfo.CountDowntoPsm < 10) ? 10:pAd->TrackInfo.CountDowntoPsm;

        //DBGPRINT_RAW(RT_DEBUG_ERROR, ("!!! report  BCAST DATA to NDIS (len=%d)  decrypt = %d, wep= %s!!!\n", DataSize,READ_RXINFO_DECRYPTED(pAd, pRxInfo),decodeCipherAlgorithm(pPort->PortCfg.WepStatus)));                              
        NdisCommonReportEthFrameToLLC(pAd,pPort, (PUCHAR)pHeader, DataSize + LENGTH_802_11, UserPriority, FALSE, FALSE);
    }

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
    )
{    
    PUCHAR         ReFormatBuffer;
    PUCHAR         pDA, pSA;
    USHORT         Msdu2Size;
    BOOLEAN       EAPOLFrame = FALSE;    
    BOOLEAN       bAMsdu;    
    BOOLEAN       QueueEAPOLFrame = FALSE;    

    UCHAR           QosPadding = 0; // between RxD and Packet
    
    NDIS_STATUS     Status =0;
    BOOLEAN         bHtVhtFrame = FALSE;
    
    USHORT          SubType;
    UCHAR           UserPriority = 0;
    BOOLEAN         bQos = FALSE;
    BOOLEAN     WAIFrame = FALSE;
    PMAC_TABLE_ENTRY pBssidMacTabEntry = MlmeSyncMacTabMatchedRoleType(pPort, ROLE_WLANIDX_BSSID);

    // before LINK UP, all DATA frames are rejected
    if (!OPSTATUS_TEST_FLAG(pPort, fOP_STATUS_MEDIA_STATE_CONNECTED)
        && !IS_P2P_GROUP_FORMING(pPort) && (!IS_P2P_GO_WPA2PSKING(pPort)))
    {
        return;                          
    }


    // [TDLS] Maintain ageout counters. Reset Idle Count if received a (NULL)packet. 
    if ((INFRA_ON(pPort)) && (TDLS_ON(pAd)) && (pRxD->RxDscrDW1.UnicastToMe == 1)/*(READ_RXINFO_U2M(pAd, pRxInfo))*/ && (pHeader->FC.FrDs == 0) && (pHeader->FC.ToDs == 0) && (MlmeSyncGetRoleTypeByWlanIdx(pPort, (USHORT)pRxD->RxDscrDW2.WLANIndex)/*READ_RXWI_WCID(pAd, pRxWI)*/ == ROLE_WLANIDX_CLIENT))
    {
        MAC_TABLE_ENTRY *pTdlsEntry = NULL;
        MP_RW_LOCK_STATE LockState;
        
        pTdlsEntry = MacTableLookup(pAd,  pPort, pHeader->Addr2);
        if (pTdlsEntry && (pRxD->RxDscrDW2.WLANIndex/*READ_RXWI_WCID(pAd, pRxWI)*/ == pTdlsEntry->Aid))
        {
            PlatformAcquireOldRWLockForWrite(&pAd->MacTablePool.MacTabPoolLock,&LockState);
            pTdlsEntry->NoDataIdleCount = 0;
            PlatformReleaseOldRWLock(&pAd->MacTablePool.MacTabPoolLock,&LockState);
        }
    }

    // remove the 2 extra QOS CNTL bytes
    bAMsdu = FALSE;
    SubType = pHeader->FC.SubType;  
    UserPriority = 0;
    if (pHeader->FC.SubType & 0x08)
    {
        //user priority
        UserPriority = pRxD->RxDscrDW2.TID/*READ_RXWI_TID(pAd, pRxWI)*/ & 0x07;
        DBGPRINT(RT_DEBUG_INFO,("Qos data, UserPriority = %d\n", UserPriority));

        // Will disable Ralink Tx Aggregation. Will reset it to default(by registry value) in Linkdown function
        //if (UserPriority) 
        //  pPort->CommonCfg.bAggregationCapable = FALSE;

        // bit 7 in Qos Control field signals the HT A-MSDU format
        if ((*pQoSField) & 0x80)
        {
            bAMsdu = TRUE;
        }
        pData += 2;
        DataSize -= 2;
        This80211HdrLen += 2;

        //
        // Dropt this QOS NULL Data frame.
        //
        if (pHeader->FC.SubType == SUBTYPE_QOS_NULL)
        {
            return;
        }
        
        // [Vista] remove QoS type
        pHeader->FC.SubType &= 0xf7; 
        bQos = TRUE;
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
        This80211HdrLen += 4;

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

        if ((pHeader->FC.Order) && (bHtVhtFrame == FALSE)&& (OPSTATUS_TEST_FLAG(pAd, fOP_STATUS_AGGREGATION_INUSED)))
        {
                // Case b.  
            Msdu2Size = *pData + (*(pData+1) << 8);
            if ((Msdu2Size <= 1536) && (Msdu2Size < DataSize))
            {
                pData += 2;
                DataSize -= 2;
            }
            else
                Msdu2Size = 0;
        }
    }                           

    if (PlatformEqualMemory(EAPOL, pData + 6, 2))
    {
        EAPOLFrame = TRUE;
        DBGPRINT(RT_DEBUG_TRACE, ("RxDone- EAPOL Frame ((pHeader->Sequence = %d))  (pHeader->FC.Retry = %d) bQueueEAPOL = %x\n", pHeader->Sequence,pHeader->FC.Retry ,pAd->pRxCfg->bQueueEAPOL));

        if((pPort->StaCfg.WscControl.EapolLastSecquence == pHeader->Sequence) && (pHeader->FC.Retry == 1))
        {
            
            //DBGPRINT(RT_DEBUG_TRACE, ("Drop this retry EAPOL frame"));
            //return;
        }
        pPort->StaCfg.WscControl.EapolLastSecquence = pHeader->Sequence;
        //
        // On WPA, WPA-PSK, WPA2 and WPA2-PSK, it will not send EAPOL-Start to re-start 4way handshaking.
        // Since we didn't indicate media connect event to upper layer,
        // if we indicate this EAPOL frame to upper layer, this packet will be ignore
        // So we queue this packet and wait for media connect.
        //
        if (((pPort->PortCfg.AuthMode == Ralink802_11AuthModeWPA)   || 
             (pPort->PortCfg.AuthMode == Ralink802_11AuthModeWPAPSK)    || 
             (pPort->PortCfg.AuthMode == Ralink802_11AuthModeWPA2)  || 
             (pPort->PortCfg.AuthMode == Ralink802_11AuthModeWPA2PSK)) && 
             (pAd->pRxCfg->bQueueEAPOL == TRUE))
        {
            DBGPRINT(RT_DEBUG_TRACE, ("QueueEAPOLFrame true\n"));   
            QueueEAPOLFrame = TRUE; 
            pAd->pRxCfg->RxEAPOLLen = 0;
        }                                               
    }
    else
    {
        EAPOLFrame = FALSE;
    }

    if(INFRA_ON(pPort))
    {
        if((((pRxD->RxDscrDW1.BSSID == 0xf) ? 0 : 1)/*(READ_RXINFO_MY_BSS(pAd, pRxInfo))*/==  0) && (PlatformEqualMemory(&pHeader->Addr2, &pPort->PortCfg.Bssid, 6)))
        {                                   
            //WRITE_RXINFO_MY_BSS(pAd, pRxInfo, 1);
            //ToDo
        }
    }
    else if(ADHOC_ON(pPort))
    {
        if(((pRxD->RxDscrDW1.BSSID == 0xf) ? 0 : 1)/*(READ_RXINFO_MY_BSS(pAd, pRxInfo))*/ && (PlatformEqualMemory(&pHeader->Addr3, &pPort->PortCfg.Bssid, 6)))
        {
            //WRITE_RXINFO_MY_BSS(pAd, pRxInfo, 1);
            //ToDo
        }
    }                               

    // Drop not my BSS frame
    //
    // Not drop EAPOL frame, since this have happen on the first time that we link up
    // And need some more time to set BSSID to asic
    // So pRxD->MyBss may be 0
    //
    if ((((pRxD->RxDscrDW1.BSSID == 0xf) ? 0 : 1)/*(READ_RXINFO_MY_BSS(pAd, pRxInfo))*/ == 0) && (EAPOLFrame != TRUE) && (WAIFrame!=TRUE))
    {
         return; // give up this frame
    }

    // Drop NULL (+CF-POLL) (+CF-ACK) data frame
    if ((pHeader->FC.SubType & 0x04) == 0x04)
    {
        DBGPRINT(RT_DEBUG_TRACE,("RxDone- drop NULL frame(subtype=%d, wcid=%d, FrDs=%d, ToDs=%d)\n",
                                pHeader->FC.SubType, 
                                pRxD->RxDscrDW2.WLANIndex/*READ_RXWI_WCID(pAd, pRxWI)*/, 
                                pHeader->FC.FrDs, 
                                pHeader->FC.ToDs));
         return;
    }
    pAd->pHifCfg->BulkInDataOneSecCount++;

#if UAPSD_AP_SUPPORT
    if (pPort->CommonCfg.bAPSDCapable && pPort->CommonCfg.APEdcaParm.bAPSDCapable &&  (pHeader->FC.SubType & 0x08))
    {
        if((*pData >> 4) & 0x01)
        {
            DBGPRINT(RT_DEBUG_TRACE,("RxDone- Rcv EOSP frame, driver may fall into sleep\n"));

            // Force driver to fall into sleep mode when rcv EOSP frame
            if (!OPSTATUS_TEST_FLAG(pAd, fOP_STATUS_DOZE))
            {
                MlmeSetPsm(pAd, PWR_SAVE);
            }
        }

        // Record more data from WMM-PS GO
        if (IS_P2P_CLIENT_OP(pPort))
        {
            UCHAR   P2pQueIdx;

            UCHAR   P2pUserPriority = pRxD->RxDscrDW2.TID/*READ_RXWI_TID(pAd, pRxWI)*/ & 0x07;
            if (P2pUserPriority <= sizeof(MapUserPriorityToAccessCategory)) // size = 8
            {
                P2pQueIdx = MapUserPriorityToAccessCategory[P2pUserPriority];
                pPort->P2PCfg.LastMoreDataFromGO[P2pQueIdx] = (BOOLEAN)pHeader->FC.MoreData;  
            }
        }

    }
#endif
    if (!DLS_ON(pAd) && !TDLS_ON(pAd))
    {
        // Drop not my BSS frame (we can not only check the MyBss bit in RxD
        if (INFRA_ON(pPort))
        {
            // Infrastructure mode, check address 2 for BSSID
            // In case of roaming, the miniport driver does not call the MlmeCntLinkDown function.
            // Therefore, the INFRAN_ON() macro is TRUE.
            // In addition, the MtAsicSetBssid() function is called in the MlmeCntLinkUp function.
            // If the AP sends the EAPOL packet before the MlmeCntLinkUp function is called.
            // The miniport drver must not discard this EAPOL packet.
            if (!PlatformEqualMemory(&pHeader->Addr2, &pPort->PortCfg.Bssid, 6) &&
                    (QueueEAPOLFrame == FALSE))
            {
                DBGPRINT(RT_DEBUG_TRACE,("Drop this data frame,not my bss ,pPort = %d\n",pPort->PortNumber));
                // Receive frame not my BSSID
                 return;
            }
        }
        else //// Ad-Hoc mode or Not associated
        {
            // Ad-Hoc mode, check address 3 for BSSID
            // Patch: Buffalo Air Station Pro AP (WAPM-HP-AM54G54)
            // The Buffalo AP sends the first EAP packet of the four-way handshaking before 
            // the INFRA_ON() is TRUE.
            if (!PlatformEqualMemory(&pHeader->Addr3, &pPort->PortCfg.Bssid, 6) &&
                    (QueueEAPOLFrame == FALSE))
            {
                // Receive frame not my BSSID
                return;
            }
        }
    }
    else if (INFRA_ON(pPort) && 
            (DLS_ON(pAd) || TDLS_ON(pAd)) && 
            (MlmeSyncGetRoleTypeByWlanIdx(pPort, (USHORT)pRxD->RxDscrDW2.TID)/*READ_RXWI_WCID(pAd, pRxWI)*/ == ROLE_WLANIDX_CLIENT) && 
            (pRxD->RxDscrDW2.TID/*READ_RXWI_WCID(pAd, pRxWI)*/ != RESERVED_WCID) && 
            (pHeader->FC.ToDs == 0) && 
            (pHeader->FC.FrDs == 0))
    {
        UCHAR   temp[MAC_ADDR_LEN];
        
        pHeader->FC.ToDs = 0;
        pHeader->FC.FrDs = 1;

        COPY_MAC_ADDR(temp, pHeader->Addr2);
        COPY_MAC_ADDR(pHeader->Addr2, pHeader->Addr3);
        COPY_MAC_ADDR(pHeader->Addr3, temp);
    }

    if ((pBssidMacTabEntry !=NULL) && (pBssidMacTabEntry->bIAmBadAtheros == FALSE)&& (pRxD->RxDscrDW2.NonAMPDUFrame == 0)/*(READ_RXINFO_AMPDU(pAd, pRxInfo) == 1)*/ && (pHeader->FC.Retry))
    {
        pBssidMacTabEntry->bIAmBadAtheros = TRUE;
        pPort->CommonCfg.IOTestParm.bCurrentAtheros = TRUE;
        pPort->CommonCfg.IOTestParm.bLastAtheros = TRUE;
        DBGPRINT(RT_DEBUG_TRACE, ("Atheros Problem. Turn on RTS/CTS!!!\n"));    
    }

    // prepare 802.3 header: DA=addr1; SA=addr3 in INFRA mode, DA=addr2 in ADHOC mode
    pDA = pHeader->Addr1; 
    if (INFRA_ON(pPort))
        pSA = pHeader->Addr3;
    else
        pSA = pHeader->Addr2;

    //*********************************************************************************
    //**
    //** For Native WiFi packet indication, 
    //** The packet included 802.11 header + data must be continuous
    //**
    //** For most case,
    //** We do this job here. 
    //**
    //*********************************************************************************
    ReFormatBuffer = (PUCHAR)(pData  - LENGTH_802_11);
    if ((PUCHAR)pHeader != ReFormatBuffer)
    {
        RtlMoveMemory(ReFormatBuffer, (PUCHAR)pHeader, LENGTH_802_11);
        pHeader = (PHEADER_802_11)ReFormatBuffer;
    }

    // [Vista] remove QoS type again, since subtype may be changed.
    pHeader->FC.SubType &= 0xf7; 


    //
    // Update pDA & pSA here, since Header may be recombined.
    // prepare 802.3 header: DA=addr1; SA=addr3 in INFRA mode, DA=addr2 in ADHOC mode
    //
    pDA = pHeader->Addr1; 
    if (INFRA_ON(pPort))
        pSA = pHeader->Addr3;
    else
        pSA = pHeader->Addr2;

    //
    // If the privacy exemption list does not apply to the frame, check ExcludeUnencrypted.
    // if ExcludeUnencrypted is set and this frame was not oringially an encrypted frame, 
    // dropped it.
    //

    if (!EAPOLFrame && (pPort->CommonCfg.bSafeModeEnabled == FALSE) )
    {
        Status = RecvCheckExemptionList(pAd,pPort,pHeader,NULL/*pRxWI*/, NULL/*pRxInfo*/,0);
        if ( Status != NDIS_STATUS_SUCCESS)
        {
            DBGPRINT(RT_DEBUG_ERROR,("CheckExemptionLis fail\n"));
            return;
        }
    }

    // 
    // Case I.1  Process Broadcast & Multicast data frame
    //
    if (pRxD->RxDscrDW1.Broadcast/*READ_RXINFO_BCAST(pAd, pRxInfo)*/ | pRxD->RxDscrDW1.Multicast/*READ_RXINFO_MCAST(pAd, pRxInfo)*/)
    {   
        RecvProcessBroadMcastDataPkt(pAd, pPort, pHeader, pRxD, DataSize, UserPriority);
    }
    //
    // Case I.2  Process unicast-to-me DATA frame
    //
    else if (pRxD->RxDscrDW1.UnicastToMe/*READ_RXINFO_U2M(pAd, pRxInfo)*/)
    {
        RecvProcessUnicastDataPkt(pAd, pPort, pHeader, pData, pRxD, PacketSize, DataSize, UserPriority, Msdu2Size, EAPOLFrame, bAMsdu, QueueEAPOLFrame, WAIFrame, rx_signal);
    }// else if (pRxInfo->U2M)
    else
    {
        DBGPRINT(RT_DEBUG_ERROR, ("N6UsbRxStaPacket: should not be run here\n"));
    }
}

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
    )
{
    NDIS_STATUS     Status =0;
    PUCHAR pHeadertemp = (PUCHAR)pHeader;
    //      
    // Decrypt management frame before mlme enqueue
    //

    if (MFP_ACT(pAd, pPort)  && (pRxD->RxDscrDW1.UnicastToMe/*READ_RXINFO_U2M(pAd, pRxInfo)*/) &&
            (pHeader->FC.SubType == SUBTYPE_DISASSOC ||
             pHeader->FC.SubType == SUBTYPE_DEAUTH ||
            pHeader->FC.SubType == SUBTYPE_ACTION))
    {
        // The protected management frame types are statisfied in the following lists: 
        // 1. Deauthentication frame
        // 2. Disassociation frame
        // 3. Action management frames of category: Spectrum management, QoS, Call Admission control

        if (pHeader->FC.SubType == SUBTYPE_ACTION) // for action frame
        {
            PFRAME_ACTION_HDR   pActFrame = (PFRAME_ACTION_HDR)pHeader;

                        //11w add a new column "Robust" for Action field,
                        //Robust Action Frame should be protected when MFP is negotiated
            if (pHeader->FC.Wep == 0 
                && pActFrame->Category == CATEGORY_PUBLIC
                //&& pActFrame->Category != CATEGORY_VENDOR_SPECIFIC_PROTECT
                                    )
                        {
                                //Public Action Frame
                        }
                        else if(pHeader->FC.Wep == 1
                && pActFrame->Category != CATEGORY_PUBLIC)
            {
                //Robust Action Frame
            }
            else
            {
                // drop the non-specified Action mgmt frame 
                DBGPRINT(RT_DEBUG_ERROR, ("MFP: Drop unprotected ACTION mgmt frame......(Category= %d)\n", pActFrame->Category));
                return;
            }

        }
        else if (pHeader->FC.Wep == 0)  // for Deauth and disassoc frame
        {
            // pHeader->FC.Wep == 0 && (Deauth || Diassoc) , will start SA Query servic
            USHORT ReasonCode=0;
                            PlatformMoveMemory(&ReasonCode, pData, sizeof(USHORT));

            if(!(ReasonCode == 6 || ReasonCode == 7))
            {
                // drop the non-encrypted Mgmt frame    
                DBGPRINT(RT_DEBUG_ERROR, ("CCX-MFP: Drop non-encrypted mgmt frame......(SubType= %d, Reason Code = %d)\n", pHeader->FC.SubType, ReasonCode));
                return;
            }
            DBGPRINT(RT_DEBUG_ERROR, ("MFP: Disassoc/Deauth reason code %d", ReasonCode));
        }
    }

    //pPort = pAd->PortList[pAd->ucActivePortNum];
    if(pPort == NULL)
    {
        return;
    }

    // TODO: Workaround .........
    if (pRxD->RxDscrDW2.WLANIndex == 0xff)  // workaround......need to check with HW designer 2014.05.28
    {
        PMAC_TABLE_ENTRY pWcidMacTabEntry = MlmeSyncMacTabMatchedBssid(pPort, pHeader->Addr2);

        if (pWcidMacTabEntry)
            pRxD->RxDscrDW2.WLANIndex = pWcidMacTabEntry->wcid;
    }
    
    if(pHeader->Addr1[0] &  0x1)
    {
        // 11w Broadcast/Multicast Mgmt Frame
        if(MFP_ON(pAd, pPort) && (pHeader->FC.SubType == SUBTYPE_DISASSOC || pHeader->FC.SubType == SUBTYPE_DEAUTH))
        {       
            DBGPRINT(RT_DEBUG_TRACE, ("MFP: Broadacast/Multicast Mgmt frame..\n"));
            if (PmfExtractBIPAction(pAd, (PUCHAR)pHeader, PacketSize /*READ_RXWI_MPDU_TOTAL_BYTE_COUNT(pAd, pRxWI)*/) != PMF_STATUS_SUCCESS)
            {
                   return;
            }
            DBGPRINT(RT_DEBUG_ERROR, ("MFP: BIP Extract Success\n"));
        }

        if((pHeader->FC.SubType == SUBTYPE_BEACON) && (MAC_ADDR_EQUAL(pAd->PortList[0]->PortCfg.Bssid, pHeader->Addr2)))
        {
            pAd->Counter.MTKCounters.OneSecRxBeaconCnt++;
        }

        REPORT_MGMT_FRAME_TO_MLME(pAd,
                                pAd->PortList[PORT_0],
                                pRxD->RxDscrDW2.WLANIndex, 
                                pHeader, 
                                PacketSize, 
                                rx_signal->raw_rssi[0], 
                                rx_signal->raw_rssi[1], 
                                rx_signal->raw_rssi[2], 
                                0);

#if(COMPILE_WIN8_ABOVE(CURRENT_OS_NDIS_VER))  
        if(NDIS_WIN8_ABOVE(pAd))
        {
            UCHAR           PortIndex;

            // Addr1 of broadcast/multicast always direct to our Port 0
            // Enqueue broadcast/multicast like probe request to all of supported ports to process and do respective response.
            // Excluding packets to client port
            for (PortIndex  = 1; PortIndex < RTMP_MAX_NUMBER_OF_PORT; PortIndex++)
            {
                if ((pAd->PortList[PortIndex] != NULL) && (pAd->PortList[PortIndex]->bActive))
                {
                    if ((pAd->PortList[PortIndex]->PortType == EXTSTA_PORT) && (pAd->PortList[PortIndex]->PortSubtype == PORTSUBTYPE_STA))
                    {
                        // beacon and probe request to non-default-port sta mode
                    }
                    else if ((pAd->PortList[PortIndex]->PortType == WFD_CLIENT_PORT) &&
                            ((pHeader->FC.SubType == SUBTYPE_PROBE_REQ) ||
                            ((pHeader->FC.SubType == SUBTYPE_BEACON) &&
                            (INFRA_ON(pAd->PortList[PortIndex])) &&
                            (!MAC_ADDR_EQUAL(pAd->PortList[PortIndex]->PortCfg.Bssid, pHeader->Addr2)))))
                    {
                        // probe request to client port
                        // not my beacon to client port
                    }
                    else if ((pAd->PortList[PortIndex]->PortType == EXTSTA_PORT) &&
                        (pAd->PortList[PortIndex]->PortSubtype == PORTSUBTYPE_P2PClient) &&
                        (pHeader->FC.SubType == SUBTYPE_BEACON) &&
                        (INFRA_ON(pAd->PortList[PortIndex])) &&
                        (!MAC_ADDR_EQUAL(pAd->PortList[PortIndex]->PortCfg.Bssid, pHeader->Addr2)))
                    {
                        // not my beacon to virtual port
                    }
                    else
                    {
                        REPORT_MGMT_FRAME_TO_MLME(pAd,
                                                pAd->PortList[PortIndex],
                                                pRxD->RxDscrDW2.WLANIndex, 
                                                pHeader, 
                                                PacketSize, 
                                                rx_signal->raw_rssi[0], 
                                                rx_signal->raw_rssi[1], 
                                                rx_signal->raw_rssi[2], 
                                                0);
                    }
                }
            }
        }
        else
#endif
        {
            if ((pAd->PortList[PORT_1] != NULL) && (pAd->PortList[PORT_1]->bActive))
            {
            
                if (pHeader->FC.SubType == SUBTYPE_BEACON)
                {
                    if (MAC_ADDR_EQUAL(pHeader->Addr3, pAd->PortList[PORT_1]->PortCfg.Bssid))
                    {
                        DBGPRINT(RT_DEBUG_INFO,("[080713]beacon...\n"));
                    }
                }

                REPORT_MGMT_FRAME_TO_MLME(pAd,
                                        pAd->PortList[PORT_1],
                                        pRxD->RxDscrDW2.WLANIndex, 
                                        pHeader, 
                                        PacketSize, 
                                        rx_signal->raw_rssi[0], 
                                        rx_signal->raw_rssi[1], 
                                        rx_signal->raw_rssi[2], 
                                        0);
            
            }
                                                            
            if((pAd->PortList[PORT_2] != NULL) && ( pAd->PortList[PORT_2]->bActive))
            {
                if(((pHeadertemp[0] == 0x80) && (pHeadertemp[1] == 0x00)) && (pPort->PortNumber != PORT_2) && (IS_P2P_CLIENT_OP(pPort)))
                {
                    //DBGPRINT(RT_DEBUG_TRACE, ("Jesse Drop not port2 Beacon"));
                }
                else
                {
                    REPORT_MGMT_FRAME_TO_MLME(pAd,
                                            pAd->PortList[PORT_2],
                                            pRxD->RxDscrDW2.WLANIndex, 
                                            pHeader, 
                                            PacketSize, 
                                            rx_signal->raw_rssi[0], 
                                            rx_signal->raw_rssi[1], 
                                            rx_signal->raw_rssi[2], 
                                            0);
                }

            }

        }
    }
    else
    {
        //  Unicast packets sent to GO/AP go to the path of ApDataCommonRxPacket.
        //  Here are unicast mgmt packets for sta, p2p device and p2p client.
        //  (like probe response, auth, assoc, disassoc, action frames)
        if ((pPort->PortType == EXTSTA_PORT) || 
            (pPort->PortType == WFD_DEVICE_PORT) || 
            (pPort->PortType == WFD_CLIENT_PORT))
        {
            //drop not unicast to me packet
            //When P2P mode, We use should check both port1's and port0's Mac address.
            // Because receiving manage frame is needed when I am still a P2P Device.
            if ((pAd->OpMode == OPMODE_STAP2P) && 
                (!MAC_ADDR_EQUAL(pHeader->Addr1, pAd->PortList[PORT_0]->CurrentAddress))&& 
                (!MAC_ADDR_EQUAL(pHeader->Addr1, pAd->PortList[pPort->P2PCfg.PortNumber]->CurrentAddress)))
            {
                DBGPRINT(RT_DEBUG_INFO,("1 drop non-unicast packet..%x %x %x %x %x %x \n", pHeader->Addr1[0], pHeader->Addr1[1],pHeader->Addr1[2],pHeader->Addr1[3],pHeader->Addr1[4],pHeader->Addr1[5]));
                return;
            }
            else if ((pAd->OpMode != OPMODE_STAP2P) && (!MAC_ADDR_EQUAL(pHeader->Addr1, pPort->CurrentAddress)))
                return;

                DBGPRINT(RT_DEBUG_TRACE, ("%s Line[%d] Port[%d] SubType = %d Retry = %d  Sequence = %d from %x %x %x %x %x %x\n"
                , __FUNCTION__, __LINE__, pPort->PortNumber, pHeader->FC.SubType, pHeader->FC.Retry, pHeader->Sequence
                , pHeader->Addr2[0], pHeader->Addr2[1], pHeader->Addr2[2], pHeader->Addr2[3], pHeader->Addr2[4], pHeader->Addr2[5]));

            

            REPORT_MGMT_FRAME_TO_MLME(pAd,
                                    pPort,
                                    pRxD->RxDscrDW2.WLANIndex, 
                                    pHeader, 
                                    PacketSize, 
                                    rx_signal->raw_rssi[0], 
                                    rx_signal->raw_rssi[1], 
                                    rx_signal->raw_rssi[2], 
                                    0);
        }
    }
    /***********************************************************************************/
    //In the NdisTest 6.0 All MGMT frame have to indicate up NDIS layer
    /***********************************************************************************/
    Status = RecvApplyPacketFilterforMGMTFrame(pAd, pPort, pRxD, pHeader);
    if (Status == NDIS_STATUS_SUCCESS) 
        NdisCommonReportEthFrameToLLC(pAd,pPort, (PUCHAR)pHeader, DataSize + LENGTH_802_11, 0, FALSE, FALSE);
    return;  // end of processing this frame
}

VOID
RecvProcessCtrlPkt(
    IN  PMP_ADAPTER  pAd, 
    IN  PMP_PORT      pPort,
    IN  PHEADER_802_11  pHeader,
    IN  PRXDSCR_BASE_STRUC pRxD,
    IN  SHORT               PacketSize
    )
{
    //pPort = pAd->PortList[pAd->ucActivePortNum];
    if(pPort == NULL)
        return;      

    //drop not unicast to me packet
    if(((pHeader->Addr1[0] &  0x1) == 0) &&  !MAC_ADDR_EQUAL(pHeader->Addr1, pPort->CurrentAddress))
         return;   

    if ((pPort->CommonCfg.PhyMode >= PHY_11ABGN_MIXED) && (pHeader->FC.SubType == SUBTYPE_BLOCK_ACK_REQ))
    {
        DBGPRINT(RT_DEBUG_TRACE, ("*************** SUBTYPE_BLOCK_ACK_REQ pRxD->RxDscrDW2.WLANIndex = %d*************************\n", pRxD->RxDscrDW2.WLANIndex));
        DBGPRINT(RT_DEBUG_TRACE, ("*************** SUBTYPE_BLOCK_ACK_REQ Add1 = %02x %02x %02x %02x %02x %02x*************************\n", pHeader->Addr1[0], pHeader->Addr1[1], pHeader->Addr1[2], pHeader->Addr1[3], pHeader->Addr1[4], pHeader->Addr1[5]));
        DBGPRINT(RT_DEBUG_TRACE, ("*************** SUBTYPE_BLOCK_ACK_REQ Add2 = %02x %02x %02x %02x %02x %02x*************************\n", pHeader->Addr2[0], pHeader->Addr2[1], pHeader->Addr2[2], pHeader->Addr2[3], pHeader->Addr2[4], pHeader->Addr2[5]));
        DBGPRINT(RT_DEBUG_TRACE, ("*************** SUBTYPE_BLOCK_ACK_REQ Add3 = %02x %02x %02x %02x %02x %02x*************************\n", pHeader->Addr3[0], pHeader->Addr3[1], pHeader->Addr3[2], pHeader->Addr3[3], pHeader->Addr3[4], pHeader->Addr3[5]));
#if 0
        CntlEnqueueForRecv(pAd, pPort,READ_RXWI_WCID(pAd, pRxWI), (READ_RXWI_MPDU_TOTAL_BYTE_COUNT(pAd, pRxWI)), (PFRAME_BA_REQ)pHeader);
#else
        CntlEnqueueForRecv(pAd, pPort, pRxD->RxDscrDW2.WLANIndex, PacketSize, (PFRAME_BA_REQ)pHeader);
#endif
    }
}

VOID
RecvUpdateLastRxBeaconTimeSamp(
    IN  PMP_ADAPTER  pAd,
    IN  PMP_PORT      pPort,
    IN  PHEADER_802_11  pHeader,   
    IN  PRXDSCR_BASE_STRUC pRxD    
    )
{
    // Update LastBeacon Time - if the frame is from AP for Infra or any-packet for Adhoc
    if (((pRxD->RxDscrDW1.BSSID == 0xf) ? 0 : 1)/*(READ_RXINFO_MY_BSS(pAd, pRxInfo))*/ && 
        ((INFRA_ON(pPort) && (pHeader->FC.FrDs) && (pPort->PortNumber == PORT_0)) ||
        ADHOC_ON(pPort)))
    {
        // STA shall think of AP is active if any packet sent from the present AP exists                        
        NdisGetCurrentSystemTime((PLARGE_INTEGER)&pAd->StaCfg.LastBeaconRxTime);
    }

    // Update LastBeacon Time - if the frame is from GO for Infra
    if (((pPort->PortType == WFD_CLIENT_PORT) ||
        (IS_P2P_CON_CLI(pAd, pAd->PortList[pPort->P2PCfg.PortNumber]))) &&
        ((pRxD->RxDscrDW1.BSSID == 0xf) ? 0 : 1)/*(READ_RXINFO_MY_BSS(pAd, pRxInfo))*/ && INFRA_ON(pPort) && (pHeader->FC.FrDs))
    {
        PMAC_TABLE_ENTRY pEntry = MlmeSyncMacTabMatchedRoleType(pAd->PortList[pPort->P2PCfg.PortNumber], ROLE_WLANIDX_P2P_CLIENT);
        if(pEntry == NULL)
        {
            DBGPRINT(RT_DEBUG_TRACE,("%s(), Line:%d, Mac Entry is NULL\n", __FUNCTION__, __LINE__));
            return;
        }
        
        // STA shall think of AP is active if any packet sent from the present AP exists                        
        NdisGetCurrentSystemTime((PLARGE_INTEGER)&pEntry->LastBeaconRxTime);
    }
}

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
    )
{
    BOOLEAN       EAPOLFrame = FALSE;    
    BOOLEAN       QueueEAPOLFrame = FALSE;        
    USHORT           DataSize;
    NDIS_STATUS     Status =0;
    BOOLEAN         bHtVhtFrame = FALSE;

    UCHAR           This80211HdrLen = LENGTH_802_11;    
    UCHAR           UserPriority = 0;
    BOOLEAN         bQos = FALSE;
    BOOLEAN     WAIFrame = FALSE;
    
#if 1
    if (Status == NDIS_STATUS_SUCCESS)
    {
        // Apply packet filtering rule based on microsoft requirements.
        Status = RecvApplyPacketFilter(pAd,pPort, pRxD, pHeader);
    }   
#endif              
    // Add receive counters
    if (Status == NDIS_STATUS_SUCCESS)
    {
        // Increase 802.11 counters & general receive counters
        INC_COUNTER64(pAd->Counter.WlanCounters.ReceivedFragmentCount);
    }

    if (Status == NDIS_STATUS_SUCCESS)
    {
        pAd->Counter.MTKCounters.OneSecRxOkCnt ++;               
        do //do...while (FALSE)
        {

            // We should collect RSSI not only U2M data but also my beacon
            // When scaning, we can not collect RSSI, it maybe the adjacent traffic
            if ((!MT_TEST_FLAG(pAd, fRTMP_ADAPTER_BSS_SCAN_IN_PROGRESS)) && (pAd->HwCfg.RxAnt.EvaluatePeriod == 0) &&
                (/*(READ_RXINFO_U2M(pAd, pRxInfo)) || */(pRxD->RxDscrDW1.UnicastToMe == 1) || ((pHeader->FC.SubType == SUBTYPE_BEACON) && (MAC_ADDR_EQUAL(&pPort->PortCfg.Bssid, &pHeader->Addr2)))))
            {

            }

            This80211HdrLen = LENGTH_802_11;

            DataSize = PacketSize - LENGTH_802_11;
            
            //drop zero length packet
            //don't drop Null data here
            if((pHeader->FC.Type != BTYPE_DATA) && ((pHeader->FC.SubType & 0x04) != 0x04) && (DataSize <= 0))
            {                           
                if (!((pHeader->FC.Type == BTYPE_CNTL) && (pHeader->FC.SubType == SUBTYPE_BLOCK_ACK_REQ) && (MtkCompareMemory(pHeader->Addr1, pPort->CurrentAddress, 6) == 0)))
                    break;
            }

            // WDS packets should be droped for client, 
            // however, Cisco AP 1200 with FW 12.3(8)JA, it does send multicast packets as WDS packets when CCKM+WEP, CCKM+CMIC
            if ((pHeader->FC.ToDs) && (!IS_P2P_GROUP_FORMING(pPort)))
            {
                pData += MAC_ADDR_LEN;
                This80211HdrLen += MAC_ADDR_LEN;

                if (pHeader->FC.FrDs)
                    DataSize -= MAC_ADDR_LEN;
            }
            
            RecvUpdateLastRxBeaconTimeSamp(pAd, pPort, pHeader, pRxD);

            //
            // CASE I. receive a DATA frame
            //
            if (pHeader->FC.Type == BTYPE_DATA)
            {
                RecvProcessDataPkt(pAd, pPort, pHeader, pRxD, pData, PacketSize, DataSize, This80211HdrLen, pQoSField, rx_signal);
                break;
            }//if (pHeader->FC.Type == BTYPE_DATA)
            
            //
            // CASE II. receive a MGMT frame
            //
            else if (pHeader->FC.Type == BTYPE_MGMT)
            {
                RecvProcessMgntPkt(pAd, pPort, pHeader, pRxD, pData, PacketSize, DataSize, rx_signal);
                break;
            }
            //
            // CASE III. receive a CNTL frame
            //
            else if (pHeader->FC.Type == BTYPE_CNTL)
            {
                RecvProcessCtrlPkt(pAd, pPort, pHeader, pRxD, PacketSize);
                break;
            }
        }while (FALSE);             
    }

}

VOID
RecvParseBuffer(
    IN  PMP_ADAPTER  pAd,
    IN  PRX_CONTEXT     pRxContext,
    IN  ULONG               TransferBufferLength
    )
{
    BOOLEAN         bQos = FALSE;
    PMP_PORT      pPort =NULL;
    PHEADER_802_11  pHeader;
    ULONG           ThisFrameLen = 0;
    ULONG           ReadPosition = 0;     
    PRXDSCR_BASE_STRUC pRxD = NULL;
    PUCHAR          pRxStart;
    PUCHAR          pData;
    UCHAR           QosPadding = 0; // between RxD+RxV and Packet
    UCHAR           RxVectorLength = 0;
    USHORT          PacketSize;
    NDIS_STATUS     ndisStatus = NDIS_STATUS_SUCCESS;
    PUCHAR         pQoSField;

    struct rx_signal_info rx_signal;
    RXD_GRP4_STRUCT *RxdGrp4 = NULL;
    RXD_GRP1_STRUCT *RxdGrp1 = NULL;
    RXD_GRP2_STRUCT *RxdGrp2 = NULL;
    RXD_GRP3_STRUCT *RxdGrp3 = NULL;
    
    while (TransferBufferLength > 0)
    {
            // TransferBufferLength >= RxD (16 bytes)
            if (TransferBufferLength >= RMAC_RX_PKT_TYPE_RX_NORMAL_LENGTH)
            {
                pRxStart = &pRxContext->TransferBuffer[ReadPosition];
                pRxD = (PRXDSCR_BASE_STRUC)pRxStart;
                
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
                    //DBGPRINT(RT_DEBUG_TRACE,("This Frame Len (nIndex = %d) (0x%x) outranges. \n", nIndex, ThisFrameLen));
                    //DBGPRINT(RT_DEBUG_TRACE,("Total BulkIn (nIndex = %d)  length=%x, Now left  TransferBufferLength = %x, ReadPosition=%x\n", nIndex, pRxContext->TransferBufferLength,TransferBufferLength, ReadPosition));
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
                    DBGPRINT(RT_DEBUG_TRACE,("########### Got a RMAC_RX_PKT_TYPE_RX_TXS packet ###########\n"));

                    if (pAd->BeaconPort.pApGoPort != NULL)
                        MTEnqueueInternalCmd(pAd, pAd->BeaconPort.pApGoPort, MT_CMD_SEND_BEACON, NULL, 0);

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

                    pAd->StaCfg.RssiSample.LastRssi[0] = rx_signal.raw_rssi[0];
                    pAd->StaCfg.RssiSample.LastRssi[1] = rx_signal.raw_rssi[1];
                    pAd->StaCfg.RssiSample.LastRssi[2] = rx_signal.raw_rssi[2];
                    
                    pAd->StaCfg.RssiSample.AvgRssiX8[0] = (pAd->StaCfg.RssiSample.AvgRssiX8[0] - pAd->StaCfg.RssiSample.AvgRssi[0]) + pAd->StaCfg.RssiSample.LastRssi[0];
                    pAd->StaCfg.RssiSample.AvgRssiX8[1] = (pAd->StaCfg.RssiSample.AvgRssiX8[1] - pAd->StaCfg.RssiSample.AvgRssi[1]) + pAd->StaCfg.RssiSample.LastRssi[1];
                    pAd->StaCfg.RssiSample.AvgRssiX8[2] = (pAd->StaCfg.RssiSample.AvgRssiX8[2] - pAd->StaCfg.RssiSample.AvgRssi[1]) + pAd->StaCfg.RssiSample.LastRssi[2];

                    pAd->StaCfg.RssiSample.AvgRssi[0] = pAd->StaCfg.RssiSample.AvgRssiX8[0] >> 3;
                    pAd->StaCfg.RssiSample.AvgRssi[1] = pAd->StaCfg.RssiSample.AvgRssiX8[1] >> 3;
                    pAd->StaCfg.RssiSample.AvgRssi[2] = pAd->StaCfg.RssiSample.AvgRssiX8[2] >> 3;
                }
                
                //get mac context
                RecvGetMacContext(pAd,pHeader,&pPort);

#if DBG
                RecvPacketDebug(pPort, (PUCHAR)pHeader);
#endif          

                // [Exception]
                // After RecvGetMacContext(), ProbeResp gets Port2 but Sync machine runs at Port0.
                // So we need to chenge from Port2 to Port0
                if ((pAd->OpMode == OPMODE_STAP2P) && 
                    IS_P2P_CLIENT_OP(pPort) &&
                    (pHeader->FC.Type == BTYPE_MGMT) && 
                    (pHeader->FC.SubType == SUBTYPE_PROBE_RSP))
                {
                    pPort = pAd->PortList[PORT_0];
                }

                // for SoftAp
                if ((pPort->SoftAP.ApCfg.bSoftAPReady == TRUE) && 
                    (pHeader->FC.Type == BTYPE_MGMT) && 
                    (pHeader->FC.SubType == SUBTYPE_ACTION))
                {
                        if (pRxD->RxDscrDW2.WLANIndex == WLANINDEX_MAX)
                        {
                            pRxD->RxDscrDW2.WLANIndex = MlmeSyncGetWlanIdxByPort(pPort, ROLE_WLANIDX_VWF_BSSID);
                        }
                }

                // for SoftAp
                if ((pPort->SoftAP.ApCfg.bSoftAPReady == TRUE) && 
                    (pHeader->FC.Type == BTYPE_MGMT) && 
                    (pHeader->FC.SubType == SUBTYPE_PROBE_REQ))
                {
                    pPort = pAd->PortList[PORT_1];
                }

                if ((pPort->PortSubtype == PORTSUBTYPE_VwifiAP) || (pPort->PortSubtype == PORTSUBTYPE_P2PGO))
                {
                    if (pPort->PortSubtype == PORTSUBTYPE_VwifiAP)
                        VALIDATE_AP_OP_STATE(pPort);
#if 0
                    {
                        int i = 0;
                        PTX_STATUS_STRUC pTxS;
                        PUCHAR p = (PUCHAR)&pRxContext->TransferBuffer[0];

                        pTxS = (PTX_STATUS_STRUC)(p+4);

                        DBGPRINT(RT_DEBUG_TRACE,("Lenssss : LifetimeError = %d, TXPOLimitError = %d\n", pTxS->Format0.LifetimeError, pTxS->Format0.TXPOLimitError ));
                        DBGPRINT(RT_DEBUG_TRACE,("Lenssss : TxStatusToHIF = %d, FixedRate = %d\n", pTxS->Format0.TxStatusToHIF, pTxS->Format0.FixedRate ));
                        
                        for (i = 0; i < (int)pRxContext->TransferBufferLength; i = i + 8)
                        {
                            DBGPRINT(RT_DEBUG_TRACE, ("Lenssss [%d] %x %x %x %x %x %x %x %x\n", i, p[i], p[i+1], p[i+2], p[i+3], p[i+4], p[i+5], p[i+6], p[i+7]));
                        }
                    }
#endif
                    
                    ApDataCommonRxPacket(pAd, pPort, pRxD, pHeader, pData, PacketSize, QosPadding, pQoSField, &rx_signal);   
                    // for next packet from RXBULKAggregation, position is at ThisFrameLen+8, so add 8 here.
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

                if (pHeader->FC.Type == BTYPE_DATA &&
                    pHeader->FC.ToDs == 1 &&
                    pHeader->FC.FrDs == 1)
                {
         
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

                    continue;
                }

#if 0
                if (READ_RXWI_MPDU_TOTAL_BYTE_COUNT(pAd, pRxWI) < 14)
                    Status = NDIS_STATUS_FAILURE;
#else
                if (PacketSize < 14)
                    ndisStatus = NDIS_STATUS_FAILURE;
#endif

                RecvProcessPacket(pAd, pPort, pRxD, pHeader, pData, PacketSize, QosPadding, pQoSField, &rx_signal);

            } //if (TransferBufferLength >= pAd->HwCfg.RXWI_Length + 8))
        
            // for next packet from RXBULKAggregation, position is at ThisFrameLen+8, so add 8 here.

            // MT7603 need to check 4 bytes alignment, because the RxByteCount is not include padding bytes.
            // If not check the padding bytes, the next packet offset will be wrong.
            if ((ThisFrameLen % 4) == 1) // padding from LMAC
                ThisFrameLen += 3;
            else if ((ThisFrameLen % 4) == 2) // padding from LMAC
                ThisFrameLen += 2;
            else if ((ThisFrameLen % 4) == 3) // padding from LMAC
                ThisFrameLen += 1;
            
            
            ThisFrameLen += RXDSCR_LAST_STRUC_LENGTH;  // padding from PSE client
            // update next packet read position. 
            ReadPosition += ThisFrameLen;
            if (TransferBufferLength >= (ThisFrameLen))
                TransferBufferLength -= (ThisFrameLen);
            else
                TransferBufferLength = 0;
            
            if (TransferBufferLength <=  4)
                break;
            
        }    
}

VOID ParseRxVGroup(
    IN  PMP_ADAPTER pAd, 
    IN  UINT32 Type, 
    IN  struct rx_signal_info *rx_signal, 
    IN  UCHAR *Data)
{
    RXV_DWORD0 *DW0 = NULL;
    RXV_DWORD1 *DW1 = NULL;
    RX_VECTOR1_1ST_CYCLE *RXV1_1ST_CYCLE = NULL;
    RX_VECTOR1_2ND_CYCLE *RXV1_2ND_CYCLE = NULL;
    RX_VECTOR1_3TH_CYCLE *RXV1_3TH_CYCLE = NULL;
    RX_VECTOR1_4TH_CYCLE *RXV1_4TH_CYCLE = NULL;
    RX_VECTOR1_5TH_CYCLE *RXV1_5TH_CYCLE = NULL;
    RX_VECTOR1_6TH_CYCLE *RXV1_6TH_CYCLE = NULL;
    RX_VECTOR2_1ST_CYCLE *RXV2_1ST_CYCLE = NULL;
    RX_VECTOR2_2ND_CYCLE *RXV2_2ND_CYCLE = NULL;
    RX_VECTOR2_3TH_CYCLE *RXV2_3TH_CYCLE = NULL;

    if (Type == RMAC_RX_PKT_TYPE_RX_NORMAL)
    {
        RXV1_1ST_CYCLE = (RX_VECTOR1_1ST_CYCLE *)Data;
        RXV1_2ND_CYCLE = (RX_VECTOR1_2ND_CYCLE *)(Data + 4);
        RXV1_3TH_CYCLE = (RX_VECTOR1_3TH_CYCLE *)(Data + 8);
        RXV1_4TH_CYCLE = (RX_VECTOR1_4TH_CYCLE *)(Data + 12);
        RXV1_5TH_CYCLE = (RX_VECTOR1_5TH_CYCLE *)(Data + 16);
        RXV1_6TH_CYCLE = (RX_VECTOR1_6TH_CYCLE *)(Data + 20);
    }
    else if (Type == RMAC_RX_PKT_TYPE_RX_TXRXV)
    {
        DW0 = (RXV_DWORD0 *)Data;
        DW1 = (RXV_DWORD1 *)(Data + 4);
        RXV1_1ST_CYCLE = (RX_VECTOR1_1ST_CYCLE *)(Data + 8);
        RXV1_2ND_CYCLE = (RX_VECTOR1_2ND_CYCLE *)(Data + 12);
        RXV1_3TH_CYCLE = (RX_VECTOR1_3TH_CYCLE *)(Data + 16);
        RXV1_4TH_CYCLE = (RX_VECTOR1_4TH_CYCLE *)(Data + 20);
        RXV1_5TH_CYCLE = (RX_VECTOR1_5TH_CYCLE *)(Data + 24);
        RXV1_6TH_CYCLE = (RX_VECTOR1_6TH_CYCLE *)(Data + 28);
        RXV2_1ST_CYCLE = (RX_VECTOR2_1ST_CYCLE *)(Data + 32);
        RXV2_2ND_CYCLE = (RX_VECTOR2_2ND_CYCLE *)(Data + 36);
        RXV2_3TH_CYCLE = (RX_VECTOR2_3TH_CYCLE *)(Data + 40);
    }

    rx_signal->raw_rssi[0] = (RXV1_3TH_CYCLE->Rcpi0 - 220) / 2;
    rx_signal->raw_rssi[1] = (RXV1_3TH_CYCLE->Rcpi1 - 220) / 2;
    //RxBlk->rx_signal.raw_snr[0] = rxwi_n->bbp_rxinfo[0];
    //RxBlk->rx_signal.raw_snr[1] = rxwi_n->bbp_rxinfo[1];
    //RxBlk->rx_signal.freq_offset = rxwi_n->bbp_rxinfo[4];

#if 0
    RxBlk->rx_rate.field.MODE = RXV1_1ST_CYCLE->TxMode;
    RxBlk->rx_rate.field.MCS = RXV1_1ST_CYCLE->TxRate;
    RxBlk->rx_rate.field.ldpc = RXV1_1ST_CYCLE->HtAdCode;
    RxBlk->rx_rate.field.BW = RXV1_1ST_CYCLE->FrMode;
    RxBlk->rx_rate.field.STBC = RXV1_1ST_CYCLE->HtStbc;
    RxBlk->rx_rate.field.ShortGI = RXV1_1ST_CYCLE->HtShortGi;
#endif    
}

VOID
RecvPreParseBuffer(
     IN  PMP_ADAPTER  pAd,
     IN  ULONG           nIndex
    )
{
    PRX_CONTEXT     pRxContext;
    ULONG               TransferBufferLength; 
    BOOLEAN         bStop = FALSE;
    
    do // do...while (TRUE);
    {
        pRxContext = &pAd->pHifCfg->RxContext[nIndex];
        if ((pRxContext->Readable == FALSE) || (pRxContext->InUse == TRUE))
        {
            break;
        }

        TransferBufferLength = pRxContext->TransferBufferLength + pRxContext->ReadPosOffset;
        if ((TransferBufferLength == 0) ||  (TransferBufferLength == MAX_RXBULK_SIZE))  // check TransferBufferLength == MAX_RXBULK_SIZE can avoid BSOD when unplug device and rx cancel at the same time.
        {
            //DBGPRINT(RT_DEBUG_TRACE,("N6UsbRxStaPacket TransferBufferLength = %d) \n",  TransferBufferLength));
            bStop = TRUE;                 
        }

        if (bStop == TRUE)
        {
            pRxContext->Readable = FALSE;
            pRxContext->ReadPosOffset = 0;  
            pRxContext->TransferBufferLength = 0;       
            pAd->pHifCfg->NextRxBulkInReadIndex = (pAd->pHifCfg->NextRxBulkInReadIndex + 1) % (RX_RING_SIZE_END_INDEX);
            break;
        }
        pAd->pHifCfg->NextRxBulkInReadIndex = (pAd->pHifCfg->NextRxBulkInReadIndex + 1) % (RX_RING_SIZE_END_INDEX);
        
        //
        // for receving ANDES packet
        //

        DBGPRINT(RT_DEBUG_INFO,("########### Rx a packet  ###########\n"));
        
        //if(ANDES_FW_CAPABLE(pAd))
        if(RecvFwCmd7603(pAd, pRxContext) == TRUE)
            break;
        
        pAd->TrackInfo.RxPacketCountFor7603Debug ++;

        //
        //   Rx Aggreggation : RxByteCount = (RxD (8 bytes) + packet length), next packet loaction  =  (RxD (16 bytes) + packet length) + RXDSCR_LAST_STRUC (4 bytes, if no check sum offload, the 4 bytes are still there and all 0)
        //
        RecvParseBuffer(pAd, pRxContext, TransferBufferLength);
        
        pRxContext->Readable = FALSE;
        pRxContext->ReadPosOffset = 0;
    //      pRxContext = &pAd->pHifCfg->RxContext[pAd->pHifCfg->NextRxBulkInReadIndex];       
    } while (FALSE);
}

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
    )
{   
    UCHAR             UserPriority = 0;
    BOOLEAN         bQos = FALSE;
    BOOLEAN         bAMsdu,bWdsPacket = FALSE;
    USHORT           Msdu2Size;
    PUCHAR          ReFormatBuffer;
    NDIS_802_11_PRIVACY_FILTER   Privacy = Ndis802_11PrivFilterAcceptAll;
    AP_WPA_STATE    WpaState;
    PUCHAR          pDA, pSA;
    ULONG           wdsidx = 0;
    PCIPHER_KEY     pWpaKey;
    MAC_TABLE_ENTRY *pEntry = NULL;

    if (pHeader->FC.ToDs == 0)
        return; // give up this frame

    // check if Class2 or 3 error
    if ((pHeader->FC.FrDs == 0) && (ApDataCheckClass2Class3Error(pAd, pPort, READ_RXWI_WCID(pAd, pRxWI) ,pHeader)))
        return; // give up this frame                            

    // Drop NULL, CF-ACK(no data), CF-POLL(no data), and CF-ACK+CF-POLL(no data) data frame
    if (pHeader->FC.SubType & 0x04) // bit 2 : no DATA
        return; // give up this frame

    if(pPort->SoftAP.ApCfg.BANClass3Data==TRUE)
        return; // give up this frame

    pAd->Counter.MTKCounters.OneSecRxOkDataCnt++;
    
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
    
    bAMsdu = FALSE;
    UserPriority = 0;

    if (pHeader->FC.SubType & 0x08)
    {
        //user priority
        UserPriority = READ_RXWI_TID(pAd, pRxWI) & 0x07;
        DBGPRINT(RT_DEBUG_INFO,("Qos data, UserPriority = %d\n", UserPriority));

        // bit 7 in Qos Control field signals the HT A-MSDU format
        if ((*pData) & 0x80)
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
            return;
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

        // shift the 2 extra QOS CNTL bytes
        if (READ_RXINFO_L2PAD(pAd, pRxInfo)== 1)
        {
            pData += 2;
        }                               
    }
    else 
    {
        // shift the 2 extra QOS CNTL bytes
        if (READ_RXINFO_L2PAD(pAd, pRxInfo)== 1)
        {
            pData += 2;
        }

        if((pHeader->FC.Order) && (bHtVhtFrame == FALSE)&& CLIENT_STATUS_TEST_FLAG(&pPort->MacTab.Content[READ_RXWI_WCID(pAd, pRxWI)], fCLIENT_STATUS_AGGREGATION_CAPABLE))
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

        return;
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
            return;                              
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
                                    READ_RXWI_WCID(pAd, pRxWI), 
                                    pHeader, 
                                    DataSize + LENGTH_802_11, 
                                    READ_RXWI_RSSI0(pAd, pRxWI), 
                                    READ_RXWI_RSSI1(pAd, pRxWI), 
                                    READ_RXWI_RSSI2(pAd, pRxWI),
                                    READ_RXWI_AntSel(pAd, pRxWI));
        
        DBGPRINT(RT_DEBUG_TRACE, ("get WPA frame ===> \n"));
        return; // end of processing this frame
    }
    else
    {
        // Drop Unencrypted Packet (Except EAPOL & NULL Frame) when our AP sets Security Mode (WPA/WPA2 WEP)
        if((pPort->PortCfg.WepStatus != Ralink802_11WEPDisabled) &&
            (pHeader->FC.Wep == 0) &&
            !(pHeader->FC.SubType == SUBTYPE_NULL_FUNC || pHeader->FC.SubType == SUBTYPE_QOS_NULL))
        {
            DBGPRINT(RT_DEBUG_TRACE, ("~! Drop Plain text packet!!seq = %d \n", pHeader->Sequence));
            return;
        }
    }

    pEntry = ApWpaPACInquiry(pAd, pHeader->Addr2, &Privacy, &WpaState);
    // port access control
    if (READ_RXWI_WCID(pAd, pRxWI) < MAX_LEN_OF_MAC_TABLE)
    {
        pEntry = &pPort->MacTab.Content[READ_RXWI_WCID(pAd, pRxWI)];
        
        Privacy = pEntry->PrivacyFilter;
        WpaState = pEntry->WpaState;
        if ((pEntry->bIAmBadAtheros == FALSE)
            && (READ_RXINFO_AMPDU(pAd, pRxInfo)== 1)
            && (pHeader->FC.Retry)
            && (READ_RXWI_PHY_MODE(pAd, pRxWI)< MODE_VHT))
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

    //
    // Check NDS Case
    //
    if (pEntry && (pEntry->NDSClientSetting == NDS_CLIENT_EVL))
    {
        ApDataCheckNDSClient(pAd, pEntry, pRxWI);
    }

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
        return; // give up this frame
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

                if ((READ_RXINFO_BA(pAd, pRxInfo)== TRUE) && (pHeader->FC.SubType != SUBTYPE_QOS_NULL))
                {
                    BOOLEAN bStatus;

                    NdisAcquireSpinLock(&pPort->BADeleteRECEntry);
                    bStatus = QosBADataParse(pAd, 
                                            pPort,
                                            TRUE, 
                                            FALSE, 
                                            (PUCHAR)pHeader, 
                                            (UCHAR)READ_RXWI_WCID(pAd, pRxWI),                                                                  
                                            (UCHAR)READ_RXWI_TID(pAd, pRxWI),                                                                   
                                            (USHORT)READ_RXWI_SN(pAd, pRxWI),
                                            DataSize + LENGTH_802_11);
                    
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
                    return;

                if ((READ_RXINFO_BA(pAd, pRxInfo) == TRUE) && (pHeader->FC.SubType != SUBTYPE_QOS_NULL))
                {
                    NdisAcquireSpinLock(&pPort->BADeleteRECEntry);
                    Returnbrc = QosBADataParse(pAd, 
                                                pPort,
                                                FALSE, 
                                                FALSE, 
                                                (PUCHAR)pHeader, 
                                                (UCHAR)READ_RXWI_WCID(pAd, pRxWI),
                                                (UCHAR)READ_RXWI_TID(pAd, pRxWI),
                                                (USHORT)READ_RXWI_SN(pAd, pRxWI),
                                                DataSize + LENGTH_802_11);
                    
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
            return; // give up this frame
        }   
        else if ((pAd->pRxCfg->FragFrame.RxSize + DataSize) > MAX_FRAME_SIZE)
        {
            // Fragment frame is too large, it exeeds the maximum frame size.
            // Clear Fragment frame contents
            PlatformZeroMemory(&pAd->pRxCfg->FragFrame, sizeof(FRAGMENT_FRAME));
            DBGPRINT(RT_DEBUG_INFO,("[%s][LINE_%d]Drop Packet\n",__FUNCTION__,__LINE__));
            return; // give up this frame
        }

        // concatenate this fragment into the re-assembly buffer
        PlatformMoveMemory(&pAd->pRxCfg->FragFrame.Buffer[pAd->pRxCfg->FragFrame.RxSize], pData, DataSize);
        pAd->pRxCfg->FragFrame.RxSize  += DataSize;
        pAd->pRxCfg->FragFrame.LastFrag = pHeader->Frag;       // Update fragment number
        // Last fragment
        if (pHeader->FC.MoreFrag == FALSE)
        {
            // For TKIP frame, calculate the MIC value
            if ((READ_RXWI_WCID(pAd, pRxWI)< MAX_LEN_OF_MAC_TABLE) 
                && (pPort->MacTab.Content[READ_RXWI_WCID(pAd, pRxWI)].WepStatus == Ralink802_11Encryption2Enabled))
            {
                if (!pEntry)
                    return;
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
}

VOID
AP_RecvProcessMgntPkt(
    IN  PMP_ADAPTER  pAd, 
    IN  PMP_PORT      pPort,
    IN  PHEADER_802_11  pHeader,
    IN  PRXINFO_STRUC   pRxInfo,
    IN  PRXWI_STRUC     pRxWI    
    )
{

    //      
    // Decrypt management frame before mlme enqueue
    //

    if (pPort->SoftAP.ApCfg.BANClass3Data==TRUE)
    {
        // disallow new association
        if ((pHeader->FC.SubType == SUBTYPE_ASSOC_REQ) || (pHeader->FC.SubType == SUBTYPE_AUTH))
        {
            DBGPRINT(RT_DEBUG_TRACE, ("   Disallow new Association \n "));
            return; // give up this frame
        }
    }

    //
    // Software decrypts the challenge text in the authentication packet (sequency number #3)
    //
    if ((pHeader->FC.Wep == 1) && (READ_RXINFO_DECRYPTED(pAd, pRxInfo) == 0))
    {
    }

    REPORT_MGMT_FRAME_TO_MLME(pAd, 
                                pPort,
                                READ_RXWI_WCID(pAd, pRxWI),
                                pHeader, 
                                READ_RXWI_MPDU_TOTAL_BYTE_COUNT(pAd, pRxWI),
                                READ_RXWI_RSSI0(pAd, pRxWI),
                                READ_RXWI_RSSI1(pAd, pRxWI),
                                READ_RXWI_RSSI2(pAd, pRxWI),
                                READ_RXWI_AntSel(pAd, pRxWI));

}

VOID
AP_RecvProcessCtrlPkt(
    IN  PMP_ADAPTER  pAd, 
    IN  PMP_PORT      pPort,
    IN  PHEADER_802_11  pHeader,
    IN  PRXINFO_STRUC   pRxInfo,
    IN  PRXWI_STRUC     pRxWI    
    )
{
    PULONG          ptemp;
    
    if ((pPort->SoftAP.ApCfg.PhyMode >= PHY_11ABGN_MIXED) && (pHeader->FC.SubType == SUBTYPE_BLOCK_ACK_REQ))
    {
        CntlEnqueueForRecv(pAd,
                            pPort, 
                            READ_RXWI_WCID(pAd, pRxWI),
                            READ_RXWI_MPDU_TOTAL_BYTE_COUNT(pAd, pRxWI),
                            (PFRAME_BA_REQ)pHeader);

    }
    // handle PS-POLL here
    if ((pHeader->FC.SubType == SUBTYPE_PS_POLL))
    {
         USHORT Aid = pHeader->Duration & 0x3fff;
         PUCHAR pAddr = pHeader->Addr2;

         DBGPRINT(RT_DEBUG_TRACE,("rcv PS-POLL (AID=%d) (WCID=%d) from %02x:%02x:%02x:%02x:%02x:%02x\n", 
             Aid, READ_RXWI_WCID(pAd, pRxWI), pAddr[0], pAddr[1], pAddr[2], pAddr[3], pAddr[4], pAddr[5]));
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

}

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
    )
{
    BOOLEAN       EAPOLFrame = FALSE;    
    BOOLEAN       QueueEAPOLFrame = FALSE;    

    UCHAR           QosPadding = 0; // between RxD and Packet
    
    NDIS_STATUS     Status =0;
    BOOLEAN         bHtVhtFrame = FALSE;

    UCHAR           This80211HdrLen = LENGTH_802_11;    

    BOOLEAN     WAIFrame = FALSE;
    MAC_TABLE_ENTRY *pEntry = NULL;
    
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
       // DBGPRINT(RT_DEBUG_EMU,("RxInfo=NULL Error!! ThisFrameLen = %d\n", ThisFrameLen));
        // Finish 
        TransferBufferLength = 0;
        Status = NDIS_STATUS_FAILURE;
    }
    else if (pRxWI == NULL)
    {
        //DBGPRINT(RT_DEBUG_EMU,("pRxWI=NULL Error!! ThisFrameLen = %d\n", ThisFrameLen));
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

    if (Status == NDIS_STATUS_SUCCESS)
    {
        do
        {
            if (pPort->SoftAP.ApCfg.bSoftAPReady == FALSE)
            {
                DBGPRINT(RT_DEBUG_INFO, ("%s: Drop this frame due to the SoftAP is not ready.\n", __FUNCTION__));
                break; // give up this packet.
            }

            if (READ_RXINFO_U2M(pAd, pRxInfo) && (pHeader != NULL) )
            {
                if (pHeader->FC.Type == BTYPE_DATA)
                {
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
                    
                    pPort->SoftAP.ApCfg.NumOfAvgRssiSample ++;
            
                    // Gather PowerSave information from all valid DATA frames. IEEE 802.11/1999 p.461
                    // Atheros Station n has wrong
                    if ((pHeader->FC.PwrMgmt))
                    {
                        APPsIndicate(pAd, pPort, pHeader->Addr2, READ_RXWI_WCID(pAd, pRxWI), pPort->SoftAP.ApCfg.LastRssi, PWR_SAVE);
                    }
                    else
                    {
                        APPsIndicate(pAd, pPort, pHeader->Addr2, READ_RXWI_WCID(pAd, pRxWI), pPort->SoftAP.ApCfg.LastRssi, PWR_ACTIVE);
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
                AP_RecvProcessDataPkt(pAd, pPort, pHeader, pData, DataSize, pRxInfo, pRxWI, bHtVhtFrame);
            }
            //
            // CASE II. receive a MGMT frame
            //
            else if (pHeader->FC.Type == BTYPE_MGMT)
            {
                AP_RecvProcessMgntPkt(pAd, pPort, pHeader, pRxInfo, pRxWI);
            }
            //
            // CASE III. receive a CNTL frame
            //
            else if (pHeader->FC.Type == BTYPE_CNTL)
            {
                AP_RecvProcessCtrlPkt(pAd, pPort, pHeader, pRxInfo, pRxWI);
            }

            //
            // CASE IV. receive a frame of invalid type
            //
            else
                break; // give up this frame

        }while(FALSE);
    }

}


VOID
AP_RecvParseBuffer(
    IN  PMP_ADAPTER  pAd,
    IN  PRX_CONTEXT     pRxContext,
    IN  ULONG               TransferBufferLength
    )
{

    BOOLEAN         bQos = FALSE;
    PMP_PORT   pPort = pAd->PortList[NDIS_DEFAULT_PORT_NUMBER];
    PHEADER_802_11  pHeader;
    ULONG           ThisFrameLen = 0;
    ULONG           ReadPosition = 0;     
    PRXDSCR_BASE_STRUC pRxD = NULL;
    PUCHAR          pStart;
    PUCHAR          pData;
    UCHAR           QosPadding = 0; // between RxD and Packet
    PRXWI_STRUC     pRxWI;
    PRXINFO_STRUC   pRxInfo;
    BOOLEAN         bHtVhtFrame = FALSE;
    USHORT          DataSize;
    
    while (TransferBufferLength > 0)
    {
        if (TransferBufferLength >= (ULONG)(pAd->HwCfg.RXWI_Length + 8))
        {
            pData = &pRxContext->TransferBuffer[ReadPosition];
            pStart = &pRxContext->TransferBuffer[ReadPosition];

            pRxD = (PRXDSCR_BASE_STRUC)pData;

            // after shift First Word is  RXDMA Length
            ThisFrameLen = *pData + (*(pData+1)<<8);
            if ((ThisFrameLen&0x3) != 0)
            {
                DBGPRINT(RT_DEBUG_TRACE,("RXDMALen not multiple of 4. [%d] (Total TransferBufferLength = %d) \n", ThisFrameLen, pRxContext->TransferBufferLength));
                ThisFrameLen = (ThisFrameLen&0x3);
            }
            if (ThisFrameLen >= TransferBufferLength)
            {
                DBGPRINT(RT_DEBUG_TRACE,("This Frame Len(0x%x) outranges. \n", ThisFrameLen));
                DBGPRINT(RT_DEBUG_TRACE,("Total BulkIn length=%x, Now left  TransferBufferLength = %x, ReadPosition=%x\n", pRxContext->TransferBufferLength,TransferBufferLength, ReadPosition));
                ThisFrameLen = (ThisFrameLen&0x3);
                //Error frame. finish this loop
                ThisFrameLen = 0;
                TransferBufferLength = 0;
                break;
            }
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
            
 
            AP_RecvProcessPacket(pAd, pPort, pHeader, pRxInfo, pData, DataSize, pRxWI, TransferBufferLength);
            
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
    }       
}


VOID
AP_RecvPreParseBuffer(
     IN  PMP_ADAPTER  pAd,
     IN  ULONG           nIndex
    )
{
    PRX_CONTEXT     pRxContext;
    PRX_CONTEXT     pRxContextBulkINFail;
    ULONG               TransferBufferLength; 
    BOOLEAN         bStop = FALSE;
    
    do // do...while (TRUE);
    {
        pRxContext = &pAd->pHifCfg->RxContext[nIndex];
        if ((pRxContext->Readable == FALSE) || (pRxContext->InUse == TRUE))
        {
            break;
        }
    
        TransferBufferLength = pRxContext->TransferBufferLength;

        if (TransferBufferLength == 0)
        {
            bStop = TRUE;
        }
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

        if (bStop == TRUE)
        {
            pRxContext->Readable = FALSE;
            pRxContext->ReadPosOffset = 0;  
            pRxContext->TransferBufferLength = 0;       
            //pAd->pHifCfg->NextRxBulkInReadIndex = (pAd->pHifCfg->NextRxBulkInReadIndex + 1) % (RX_RING_SIZE_END_INDEX);
            break;
        }

        //pAd->pHifCfg->NextRxBulkInReadIndex = (pAd->pHifCfg->NextRxBulkInReadIndex + 1) % (RX_RING_SIZE_END_INDEX);

        AP_RecvParseBuffer(pAd, pRxContext, TransferBufferLength);

        pRxContext->Readable = FALSE;
        pRxContext->ReadPosOffset = 0;
        pRxContext = &pAd->pHifCfg->RxContext[nIndex];
    } while (FALSE);
}
#endif

VOID
RecordWaitToIndicateRxPktsNumbyPort(
    IN  PMP_ADAPTER  pAd,
    IN UCHAR      index
    )
{
        PMP_PORT      pPort =NULL;
        
        pPort = pAd->PortList[index];
        if (pPort != NULL)
        {
            NdisAcquireSpinLock(&pAd->pRxCfg->IndicateRxPktsLock[pPort->PortNumber]);

            if (pPort->PortNumber == 1)
                DBGPRINT(RT_DEBUG_INFO, ("Lens : pPort->PortNumber = %d, pAd->pRxCfg->nWaitToIndicateRxPktsNum[pPort->PortNumber] = %d\n", pPort->PortNumber, pAd->pRxCfg->nWaitToIndicateRxPktsNum[pPort->PortNumber]));
            
             if (pAd->pRxCfg->nWaitToIndicateRxPktsNum[pPort->PortNumber] > 0)
            {
                PlatformIndicatePktToNdis(pAd, (PVOID)pAd->pRxCfg->pWaitToIndicateRxPkts[pPort->PortNumber], pPort->PortNumber, pAd->pRxCfg->nWaitToIndicateRxPktsNum[pPort->PortNumber]);
                pAd->pRxCfg->pLastWaitToIndicateRxPkts[pPort->PortNumber] = NULL;
                pAd->pRxCfg->pWaitToIndicateRxPkts[pPort->PortNumber] = NULL;
                pAd->pRxCfg->nWaitToIndicateRxPktsNum[pPort->PortNumber] = 0;
            }
            NdisReleaseSpinLock(&pAd->pRxCfg->IndicateRxPktsLock[pPort->PortNumber]);
        }    
}

/*
    ========================================================================

    Routine Description:
        Apply packet filter policy, return NDIS_STATUS_FAILURE if this frame
        should be dropped.
        
    Arguments:
        pAd     Pointer to our adapter
        pRxD            Pointer to the Rx descriptor
        pHeader         Pointer to the 802.11 frame header
        
    Return Value:
        NDIS_STATUS_SUCCESS     Accept frame
        NDIS_STATUS_FAILURE     Drop Frame
        
    Note:
        Maganement frame should bypass this filtering rule.
    
    ========================================================================
*/
NDIS_STATUS RecvApplyPacketFilter(
    IN  PMP_ADAPTER   pAd, 
    IN  PMP_PORT  pPort, 
    IN  PRXDSCR_BASE_STRUC  pRxD, 
    IN  PHEADER_802_11  pHeader)
{
    BOOLEAN     bAddrMatch = (BOOLEAN)pRxD->RxDscrDW1.UnicastToMe;
    BOOLEAN     bIsBroadcast = (BOOLEAN)pRxD->RxDscrDW1.Broadcast;
    BOOLEAN     bIsMulticast = (BOOLEAN)pRxD->RxDscrDW1.Multicast;
    UINT i;
    switch(pHeader->FC.Type)
    {
        case DOT11_FRAME_TYPE_MANAGEMENT:
        case DOT11_FRAME_TYPE_CONTROL:  
            return NDIS_STATUS_SUCCESS;
                break;
        case DOT11_FRAME_TYPE_DATA:             
            // 0.1  Drop all Rx frames if MIC countermeasures kicks in
            if (pAd->StaCfg.MicErrCnt >= 3)
            {
                DBGPRINT_RAW(RT_DEBUG_TRACE,("Rx dropped by MIC countermeasure\n"));
                return NDIS_STATUS_FAILURE;
            }       

            //unicast packet and address match ,pass this packet
            if (bAddrMatch && (pPort->PortCfg.PacketFilter & NDIS_PACKET_TYPE_DIRECTED)){
                
                return NDIS_STATUS_SUCCESS;
            }

            //Ignore the filter if doing WPS, since the packet filter is 0x00000000 sometimes.
            if((bAddrMatch)&&(pPort->StaCfg.WscControl.WscState >= WSC_STATE_START))
            {
                return NDIS_STATUS_SUCCESS;
            }

            //broadcast packet and support broadcast filter flag, pass this packet
            if (bIsBroadcast && (pPort->PortCfg.PacketFilter & NDIS_PACKET_TYPE_BROADCAST))
                return NDIS_STATUS_SUCCESS;
        
            //add filter out un-wanted multicast address packet,for ndistest use
            if (bIsMulticast&& (pPort->PortCfg.PacketFilter & (NDIS_PACKET_TYPE_MULTICAST | 
                                                        NDIS_PACKET_TYPE_ALL_MULTICAST)))
            {
                //pass this packet ,if all multicast adress allowed
                if(pPort->PortCfg.PacketFilter & NDIS_PACKET_TYPE_ALL_MULTICAST)
                    return NDIS_STATUS_SUCCESS;
                    
                for (i = 0; i < pPort->PortCfg.MCAddressCount ; i++)
                {
                    //compare all multicast address,if address match ,pass this packet else drop
                    if (MAC_ADDR_EQUAL(pHeader->Addr1, pPort->PortCfg.MCAddressList[i]))
                    break;      // Matched
                }
                

                // Not matched
                if (i == pPort->PortCfg.MCAddressCount )
                {
                    UCHAR ICMPv6RA[]={0x33, 0x33, 0x00, 0x00, 0x00, 0x01};

                    // Patch Vista/Win7
                    if (MAC_ADDR_EQUAL(pHeader->Addr1, ICMPv6RA) && (pPort->CommonCfg.AcceptICMPv6RA == TRUE))            
                    {
                        return(NDIS_STATUS_SUCCESS);
                    }
                
                    DBGPRINT(RT_DEBUG_INFO,("Rx MCAST %02x:%02x:%02x:%02x:%02x:%02x dropped by RX_FILTER\n",
                                            pHeader->Addr1[0], pHeader->Addr1[1], pHeader->Addr1[2], 
                                            pHeader->Addr1[3], pHeader->Addr1[4], pHeader->Addr1[5]));
                    return(NDIS_STATUS_FAILURE);
                }
                else
                {
                    DBGPRINT(RT_DEBUG_INFO,("Accept multicast %02x:%02x:%02x:%02x:%02x:%02x\n",
                                            pHeader->Addr1[0], pHeader->Addr1[1], pHeader->Addr1[2], 
                                            pHeader->Addr1[3], pHeader->Addr1[4], pHeader->Addr1[5]));
                    return NDIS_STATUS_SUCCESS;
                }               
            }
            //drop multicast packet , if support multicast flag not set
            //notice !! if NDIS_PACKET_TYPE_PROMISCUOUS set but NDIS_PACKET_TYPE_MULTICAST not set ,we still need to drop
            //multicast packet
            else if(bIsMulticast && !(pPort->PortCfg.PacketFilter & (NDIS_PACKET_TYPE_MULTICAST | 
                                                            NDIS_PACKET_TYPE_ALL_MULTICAST)))
            {
                return(NDIS_STATUS_FAILURE);
            }
            
            //pass this packet if support promiscuos mode or raw data mode
            if (pPort->PortCfg.PacketFilter & (NDIS_PACKET_TYPE_PROMISCUOUS | 
                                            NDIS_PACKET_TYPE_802_11_RAW_DATA))
            {
                return NDIS_STATUS_SUCCESS;
            }

            break;

        default:
            //
            // Reserved packet should always be filtered
            //      
            return NDIS_STATUS_FAILURE;
    }

        return NDIS_STATUS_FAILURE; 
}

NDIS_STATUS RecvCheckExemptionList
    (IN PMP_ADAPTER   pAd,
    IN  PMP_PORT      pPort,
     IN PHEADER_802_11  pHeader,
     IN PRXWI_STRUC     pRxWI,
     IN PRXINFO_STRUC   pRxINFO,
     IN UCHAR           QOS_PAD
    )
{
    NDIS_STATUS                     ndisStatus = NDIS_STATUS_SUCCESS;
    USHORT                          EtherType =0;
    USHORT                          PacketType;
    PDOT11_PRIVACY_EXEMPTION        PrivacyExemption;
    BOOLEAN                         isUnicast;
    ULONG                           index;
    //PCIPHER_KEY pCipheyKey;
    PDOT11_PRIVACY_EXEMPTION_LIST   PrivacyExemptionList = NULL;
    int                             i;
    PUCHAR                          pData =(PUCHAR) pHeader;
    UCHAR                           PortSecured = pAd->StaCfg.PortSecured;
    PMAC_TABLE_ENTRY pWcidMacTabEntry = NULL;
    pWcidMacTabEntry = MlmeSyncMacTabMatchedWlanIdx(pPort, P2pGetClientWcid(pAd, pPort)); 
    UNREFERENCED_PARAMETER(pRxWI);
    UNREFERENCED_PARAMETER(pRxINFO);

    PrivacyExemptionList = pPort->PortCfg.PrivacyExemptionList;


    // Use P2P's PortSecured in concurrent Client
    if ((pWcidMacTabEntry != NULL) && (IS_P2P_CON_CLI(pAd, pPort) || IS_P2P_MS_CLI(pAd, pPort) || IS_P2P_MS_CLI2(pAd, pPort)))
    {
        PortSecured = (UCHAR)pWcidMacTabEntry->PortSecured;
    }

    __try 
    {
        //
        // Go through the privacy exemption list to see if we can accept the data frame.
        //
        if (PrivacyExemptionList && PrivacyExemptionList->uNumOfEntries > 0)
        {       
            //
            // Find the EtherType and PacketType of the frame
            //

            EtherType = ((PIEEE_8022_LLC_SNAP) Add2Ptr(pHeader, sizeof(DOT11_DATA_SHORT_HEADER) + QOS_PAD))->sh_etype;
            isUnicast = (BOOLEAN)ETH_IS_UNICAST(pHeader->Addr1);
            PacketType = isUnicast ? DOT11_EXEMPT_UNICAST : DOT11_EXEMPT_MULTICAST;

            //
            // Check the disposition of the frame.
            //

            PrivacyExemption = PrivacyExemptionList->PrivacyExemptionEntries;
            for (index = 0; index < PrivacyExemptionList->uNumOfEntries; index++, PrivacyExemption++)
            {
                //
                // Skip if EtherType does not match
                //

                if (PrivacyExemption->usEtherType != EtherType)
                {
                    continue;
                }

                //
                // Skip if PacketType does not match
                //

                if (PrivacyExemption->usExemptionPacketType != PacketType &&
                    PrivacyExemption->usExemptionPacketType != DOT11_EXEMPT_BOTH)
                {
                    continue;
                }

                if (PrivacyExemption->usExemptionActionType == DOT11_EXEMPT_ALWAYS)
                {
                    //
                    // In this case, we drop the frame if it was originally
                    // encrypted.
                    //

                    if (pHeader->FC.Wep)
                    {
                        ndisStatus = NDIS_STATUS_NOT_ACCEPTED;
                    }

                    __leave;
                }
                else if (PrivacyExemption->usExemptionActionType == DOT11_EXEMPT_ON_KEY_MAPPING_KEY_UNAVAILABLE)
                {
                    //
                    // In this case, we reject the frame if it was originally NOT encrypted but 
                    // we have the key mapping key for this frame.
                    //              
                    if((pPort->PortType == EXTAP_PORT) ||(IS_P2P_GO_OP(pPort)))
                    {
                        if(MlmeInfoAPFindKeyMappingKey(pAd,pPort,pHeader->Addr2))
                        {
                            if ((!pHeader->FC.Wep) && (isUnicast) )
                            {
                                DBGPRINT_RAW(RT_DEBUG_ERROR, ("client addr2 = %2x:%2x:%2x:%2x:%2x:%2x",pHeader->Addr2[0],
                                pHeader->Addr2[1],pHeader->Addr2[2],pHeader->Addr2[3],pHeader->Addr2[4],
                                pHeader->Addr2[5]));    
                                //DBGPRINT_RAW(RT_DEBUG_ERROR, ("AP examption fail, KeyIndex = %d\n",READ_RXWI_KEY_IDX(pAd, pRxWI)));
                                DBGPRINT_RAW(RT_DEBUG_ERROR, ("AP examption fail\n"));
                                for(i = 0 ;i < 16;i++)
                                {
                                    DBGPRINT_RAW(RT_DEBUG_ERROR, ("%2x",*(pData + i + 24)));    
                                }
                                ndisStatus = NDIS_STATUS_NOT_ACCEPTED;
                            }
                        }
                    }
                    else
                    {
                        if (TRUE == MlmeInfoFindKeyMappingKey(pAd,pPort,pHeader->Addr2))
                        {
                            if ((!pHeader->FC.Wep) &&
                            (isUnicast) &&
                            (PortSecured == WPA_802_1X_PORT_SECURED))
                            {
                                DBGPRINT_RAW(RT_DEBUG_ERROR, ("addr2 = %2x:%2x:%2x:%2x:%2x:%2x",pHeader->Addr2[0],
                                pHeader->Addr2[1],pHeader->Addr2[2],pHeader->Addr2[3],pHeader->Addr2[4],
                                pHeader->Addr2[5]));    
                                //DBGPRINT_RAW(RT_DEBUG_ERROR, ("<examption fail, KeyIndex = %d\n",READ_RXWI_KEY_IDX(pAd, pRxWI)));
                                DBGPRINT_RAW(RT_DEBUG_ERROR, ("<examption fail\n"));
                                for(i = 0 ;i < 16;i++)
                                {
                                    DBGPRINT_RAW(RT_DEBUG_ERROR, ("%2x",*(pData + i + 24)));    
                                }
                                // EAPOL frame examption fail...should never happen
                                if (PlatformEqualMemory(EAPOL_LLC_SNAP, pData+LENGTH_802_11, 8))
                                {
                                    DBGPRINT_RAW(RT_DEBUG_ERROR, ("$$$$$$$$$$$$$$$$ examption  for EAPOL, should not happen .... $$$$$$$$n"));
                                    return NDIS_STATUS_SUCCESS;
                                }
                                ndisStatus = NDIS_STATUS_NOT_ACCEPTED;
                            }
                        }
                    }
                    __leave;
                }
                else 
                {
                    //
                    // The privacy exemption does not apply to this frame.
                    //

                    break;
                }
            }
        }

        //
        // If the privacy exemption list does not apply to the frame, check ExcludeUnencrypted.
        // if ExcludeUnencrypted is set and this frame was not oringially an encrypted frame, 
        // dropped it.
        //
        if (pPort->PortCfg.ExcludeUnencrypted && !pHeader->FC.Wep &&  ((pHeader->FC.MoreFrag == 0 )&& (pHeader->Frag == 0)))
        {       
            ndisStatus = NDIS_STATUS_NOT_ACCEPTED;
            DBGPRINT_RAW(RT_DEBUG_TRACE, ("<examption fail, ExcludeUnencrypted = %x\n",pPort->PortCfg.ExcludeUnencrypted));
            DBGPRINT_RAW(RT_DEBUG_TRACE, ("<eth type = %x,qos pad= %d\n",EtherType,QOS_PAD));
            if (ETH_IS_MULTICAST(pHeader->Addr1))
            {
                pAd->Counter.StatisticsInfo.ullMcastWEPExcludedCount++;
            }
            else
            {
                pAd->Counter.StatisticsInfo.ullUcastWEPExcludedCount++;
            }
        }
    }
    __finally 
    {
    }


    return ndisStatus;
}

NDIS_STATUS RecvApplyPacketFilterforMGMTFrame(
    IN  PMP_ADAPTER   pAd, 
    IN  PMP_PORT      pPort,
    IN  PRXDSCR_BASE_STRUC  pRxD, 
    IN  PHEADER_802_11  pHeader)
{
    BOOLEAN     bAddrMatch = (BOOLEAN)pRxD->RxDscrDW1.UnicastToMe;
    BOOLEAN     bIsBroadcast = (BOOLEAN)pRxD->RxDscrDW1.Broadcast;
    BOOLEAN     bIsMulticast = (BOOLEAN)pRxD->RxDscrDW1.Multicast;
    UNREFERENCED_PARAMETER(pAd);
    UNREFERENCED_PARAMETER(pHeader);
        
    if (bAddrMatch && (pPort->PortCfg.PacketFilter & NDIS_PACKET_TYPE_802_11_DIRECTED_MGMT))
        return NDIS_STATUS_SUCCESS;

    if (bIsBroadcast && (pPort->PortCfg.PacketFilter & NDIS_PACKET_TYPE_802_11_BROADCAST_MGMT))
        return NDIS_STATUS_SUCCESS;

    if (bIsMulticast && (pPort->PortCfg.PacketFilter & (NDIS_PACKET_TYPE_802_11_MULTICAST_MGMT | 
                        NDIS_PACKET_TYPE_802_11_ALL_MULTICAST_MGMT)))
        return NDIS_STATUS_SUCCESS;

    if (pPort->PortCfg.PacketFilter & NDIS_PACKET_TYPE_802_11_PROMISCUOUS_MGMT)
    {
        return NDIS_STATUS_SUCCESS;
    }
    
    return NDIS_STATUS_FAILURE;
}

/*
    ========================================================================

    Routine Description:
        Check Rx descriptor, return NDIS_STATUS_FAILURE if any error dound
        
    Arguments:
        pRxD        Pointer to the Rx descriptor
        
    Return Value:
        NDIS_STATUS_SUCCESS     No err
        NDIS_STATUS_FAILURE     Error
        
    Note:
    
    ========================================================================
*/
NDIS_STATUS 
RecvCheckRxWi(
    IN  PMP_ADAPTER   pAd,
    IN  PMP_PORT      pPort,
    IN  PHEADER_802_11  pHeader,    
    IN  PRXWI_STRUC     pRxWI,  
    IN  PRXINFO_STRUC   pRxINFO
    )
{   
    UCHAR   CipherAlg;
    PDOT11_PHY_FRAME_STATISTICS pPhyStats = &pAd->Counter.StatisticsInfo.PhyCounters[pPort->CommonCfg.DefaultPhyId];

    if(pRxINFO == NULL)
    {
        DBGPRINT(RT_DEBUG_TRACE,("pRxINFO is NULL\n"));
        pPhyStats->ullReceivedFrameCount++;
        return(NDIS_STATUS_FAILURE);
    }
    
    if (pAd->UiCfg.bPromiscuous == TRUE)
    {
        pPhyStats->ullReceivedFrameCount++;
        return(NDIS_STATUS_SUCCESS);
    }

    // We should only increase the ullReceivedFrameCount counter with correct decryption and non-MIC Error
    if(READ_RXINFO_CIPHER_ERROR(pAd, pRxINFO) == 0) 
    {
        pPhyStats->ullReceivedFrameCount++;
        if (READ_RXINFO_U2M(pAd, pRxINFO))
        {
            pAd->Counter.StatisticsInfo.UcastCounters.ullReceivedFrameCount++;
        }
        else
        {
            pAd->Counter.StatisticsInfo.McastCounters.ullReceivedFrameCount++;
        }
    }

    // Phy errors & CRC errors
    if (READ_RXINFO_CRC_ERROR(pAd, pRxINFO))
    {
        pPhyStats->ullFCSErrorCount++;
        DBGPRINT(RT_DEBUG_TRACE,("pRxINFO crc error\n"));
        return(NDIS_STATUS_FAILURE);
    }

    // Drop ToDs promiscous frame, it is opened due to CCX 2 channel load statistics
    if (pHeader != NULL)
    {
        if (pHeader->FC.ToDs)
        {
            DBGPRINT_RAW(RT_DEBUG_INFO, ("Err;FC.ToDs\n"));
            return NDIS_STATUS_FAILURE; // drop ToDs frames
        }
    }

    // Paul 04-03 for OFDM Rx length issue
    if (READ_RXWI_MPDU_TOTAL_BYTE_COUNT(pAd, pRxWI) > MAX_AGGREGATION_SIZE)
    {
        DBGPRINT_RAW(RT_DEBUG_ERROR, ("received packet too long\n"));
        return NDIS_STATUS_FAILURE;
    }

    // Drop not U2M frames, cant's drop here because we will drop beacon in this case
    // I am kind of doubting the U2M bit operation
    // if (pRxD->U2M == 0)
    //  return(NDIS_STATUS_FAILURE);

    if ( (READ_RXINFO_DECRYPTED(pAd, pRxINFO) == 0) && (pHeader->FC.Wep == 1) &&
        (pHeader->FC.Type  == DOT11_FRAME_TYPE_DATA))
    {
        if (READ_RXINFO_U2M(pAd, pRxINFO))
        {
            pAd->Counter.StatisticsInfo.UcastCounters.ullWEPUndecryptableCount++;
        }
        else
        {
            pAd->Counter.StatisticsInfo.McastCounters.ullWEPUndecryptableCount++;
        }
    }

    if (((READ_RXINFO_CIPHER_ERROR(pAd, pRxINFO)) && (READ_RXINFO_DECRYPTED(pAd, pRxINFO)) && (pPort->PortCfg.WepStatus == Ralink802_11WEPDisabled))||
        ((READ_RXINFO_CIPHER_ERROR(pAd, pRxINFO) == 2) && (pPort->PortCfg.WepStatus == Ralink802_11Encryption3Enabled) && (pAd->pHifCfg->BulkLastOneSecCount >= 10)))
    {
        // I may get wrong data when calling AbortPipe in multiple bulkin reset.
        DBGPRINT_RAW(RT_DEBUG_ERROR, ("WepStatus[%s] but get CipherErr[%d], BulkLastOneSecCount[%d]\n",
                                    decodeCipherAlgorithm(pPort->PortCfg.WepStatus), 
                                    READ_RXINFO_CIPHER_ERROR(pAd, pRxINFO),
                                    pAd->pHifCfg->BulkLastOneSecCount));
        
        DBGPRINT_RAW(RT_DEBUG_ERROR, ("Prevent ReportMicError[%d]/[%d]/[%d]/[%d]/[%d]/[%d]\n",
                                    READ_RXINFO_DECRYPTED(pAd, pRxINFO),
                                    READ_RXINFO_MCAST(pAd, pRxINFO),
                                    READ_RXINFO_BCAST(pAd, pRxINFO),
                                    READ_RXINFO_U2M(pAd, pRxINFO),
                                    READ_RXWI_KEY_IDX(pAd, pRxWI),
                                    READ_RXINFO_MY_BSS(pAd, pRxINFO)));

        return(NDIS_STATUS_FAILURE);
    }

    // drop decyption fail frame
    if (READ_RXINFO_DECRYPTED(pAd, pRxINFO) && READ_RXINFO_CIPHER_ERROR(pAd, pRxINFO))
    {
        PUCHAR ptr = (PUCHAR)pHeader;

        DBGPRINT_RAW(RT_DEBUG_INFO,("CIPHER error = %d\n",READ_RXINFO_CIPHER_ERROR(pAd, pRxINFO)));
        /*DBGPRINT_RAW(RT_DEBUG_INFO,("ERROR: CRC ok but CipherErr %d (len = %d, Mcast=%d, Cipher=%s, KeyId=%d)\n", 
            pRxINFO->CipherErr,
            pRxWI->MPDUtotalByteCount, 
            pRxINFO->Mcast | pRxINFO->Bcast,
            DecodeCipherName(pRxINFO->CipherAlg),
            pRxINFO->KeyIndex));*/
#if 0
        for (i=0;i<64; i+=16)
        {
            DBGPRINT_RAW(RT_DEBUG_INFO,("%02x:%02x:%02x:%02x:%02x:%02x:%02x:%02x - %02x:%02x:%02x:%02x:%02x:%02x:%02x:%02x\n",
                *ptr,*(ptr+1),*(ptr+2),*(ptr+3),*(ptr+4),*(ptr+5),*(ptr+6),*(ptr+7),
                *(ptr+8),*(ptr+9),*(ptr+10),*(ptr+11),*(ptr+12),*(ptr+13),*(ptr+14),*(ptr+15)));
            ptr += 16;
        }
#endif  

        //
        // MIC Error. Forget it when add/update key in process.
        //
        if ((READ_RXINFO_CIPHER_ERROR(pAd, pRxINFO) == 2) && READ_RXINFO_MY_BSS(pAd, pRxINFO) && !(MT_TEST_FLAG(pAd, fRTMP_ADAPTER_ADDKEY_IN_PROGRESS)))
        {           
            N6PlatformIndicateMICFailure(pAd, pHeader->Addr2, READ_RXWI_KEY_IDX(pAd, pRxWI), (READ_RXWI_KEY_IDX(pAd, pRxWI) == 0) ? FALSE:TRUE);

            //WLK statistic
            if (READ_RXINFO_U2M(pAd, pRxINFO)) 
            {
                pAd->Counter.StatisticsInfo.ullTKIPCounterMeasuresInvoked++;
                pAd->Counter.StatisticsInfo.UcastCounters.ullTKIPLocalMICFailures++;
            }
            else
            {
                pAd->Counter.StatisticsInfo.ullTKIPCounterMeasuresInvoked++;
                pAd->Counter.StatisticsInfo.McastCounters.ullTKIPLocalMICFailures++;
            }
            DBGPRINT_RAW(RT_DEBUG_ERROR,("Rx MIC Value error\n"));
        }
        else
        {
            // We should not increase ullDecryptFailureCount when ullTKIPLocalMICFailures already increase
            if (READ_RXINFO_U2M(pAd, pRxINFO))
            {
                pAd->Counter.StatisticsInfo.UcastCounters.ullDecryptFailureCount++;
            }
            else
            {
                pAd->Counter.StatisticsInfo.McastCounters.ullDecryptFailureCount++;
            }
        }
        
        if (READ_RXINFO_DECRYPTED(pAd, pRxINFO) &&
            (pPort->SharedKey[BSS0][READ_RXWI_KEY_IDX(pAd, pRxWI)].CipherAlg == CIPHER_AES) &&
            (pHeader->Sequence == pAd->pRxCfg->FragFrame.Sequence))
        {
            //
            // Acceptable since the First FragFrame no CipherErr problem.
            //
            return(NDIS_STATUS_SUCCESS);
        }

        CipherAlg = pPort->SharedKey[BSS0][READ_RXWI_KEY_IDX(pAd, pRxWI)].CipherAlg;

        switch(CipherAlg)
        {
            case CIPHER_TKIP:
                // Key not valid also make ICV error
                if (READ_RXINFO_CIPHER_ERROR(pAd, pRxINFO) & 0x1)
                {
                    if (READ_RXINFO_U2M(pAd, pRxINFO))
                    {
                        pAd->Counter.StatisticsInfo.UcastCounters.ullTKIPICVErrorCount++;
                    }
                    else
                    {
                        pAd->Counter.StatisticsInfo.McastCounters.ullTKIPICVErrorCount++;
                    }
                }
            break;
            
            case CIPHER_AES:
                if (READ_RXINFO_U2M(pAd, pRxINFO))
                {
                    pAd->Counter.StatisticsInfo.UcastCounters.ullCCMPDecryptErrors++;
                }
                else
                {
                    pAd->Counter.StatisticsInfo.McastCounters.ullCCMPDecryptErrors++;
                }
            break;
            case CIPHER_WEP64:
            case CIPHER_WEP128: 
                if (READ_RXINFO_CIPHER_ERROR(pAd, pRxINFO) == 1)
                {
                    if (READ_RXINFO_U2M(pAd, pRxINFO))
                    {
                        pAd->Counter.StatisticsInfo.UcastCounters.ullWEPICVErrorCount++;
                    }
                    else
                    {
                        pAd->Counter.StatisticsInfo.McastCounters.ullWEPICVErrorCount++;
                    }
                }
                break;
            default:
                break;
        }
        return(NDIS_STATUS_FAILURE);
    }
    else
    {
        //
        // Decrypted packet successfully.
        //

        //
        // Update DecryptSuccessCount required on new WLK 1.5
        //
        if (READ_RXINFO_DECRYPTED(pAd, pRxINFO))
        {
            if (READ_RXINFO_U2M(pAd, pRxINFO))
            {
                pAd->Counter.StatisticsInfo.UcastCounters.ullDecryptSuccessCount++;
            }
            else
            {
                pAd->Counter.StatisticsInfo.McastCounters.ullDecryptSuccessCount++;
            }
        }
    }

    return(NDIS_STATUS_SUCCESS);
}

/*
    ========================================================================

    Routine Description:
        Process MIC error indication and record MIC error timer.
        
    Arguments:
        pAd     Pointer to our adapter
        pWpaKey         Pointer to the WPA key structure
        
    Return Value:
        None
        
    Note:
    
    ========================================================================
*/
VOID    
RecvReportMicError(
    IN  PMP_ADAPTER   pAd, 
    IN  PCIPHER_KEY     pWpaKey
    )
{
    ULONGLONG   Now;

    DBGPRINT(RT_DEBUG_WARN, ("RecvReportMicError .......... StaCfg.MicErrCnt =%d    \n",pAd->StaCfg.MicErrCnt ));

    // Record Last MIC error time and count
    NdisGetCurrentSystemTime((PLARGE_INTEGER)&Now);
    if (pAd->StaCfg.MicErrCnt == 0)
    {
        pAd->StaCfg.MicErrCnt++;        
    }
    else if (pAd->StaCfg.MicErrCnt == 1)
    {
        // Vista sends OID_DOT11_EXCLUDED_MAC_ADDRESS_LIST if there're two MicErr
        // Driver has to blck assoc before the OID requests to disconnect
        // pAd->StaCfg.bBlockAssoc = TRUE; //V2.3.3.0: Remove to pass WHQL(item: indications_ext)

        // MIC error is detected in 60 seconds
        if ((Now - pAd->StaCfg.LastMicErrorTime) < (60 * ONE_SECOND_TIME))
        {
            DBGPRINT(RT_DEBUG_WARN, ("MIC error was detected twice in 60 seconds. Disconnection would be perform.\n"));
            // Violate MIC error counts, MIC countermeasures kicks in
            pAd->StaCfg.MicErrCnt++;
            
            // We shall block all reception
            // We shall clean all Tx ring and disassoicate from AP after next EAPOL frame
            NdisCommonRejectPendingPackets(pAd);
            //N6USBCleanUpDataBulkOutQueue(pAd);
        }
        else
        {           
            // this did not violate two MIC errors within 60 seconds
            // pAd->StaCfg.MicErrCnt still ==1;     
            DBGPRINT(RT_DEBUG_WARN, ("The second MIC error was detected!! But the first one was detected 60 second ago. Connection retained.\n"));
        }
    }
    else
    {
        DBGPRINT(RT_DEBUG_WARN, ("******** StaCfg.MicErrCnt =%d, SHOULD NOT HAPPEN ******    \n",pAd->StaCfg.MicErrCnt ));
        // MIC error count >= 2
        // This should not happen       
    }
    // update pAd->StaCfg.LastMicErrorTime to current time
    pAd->StaCfg.LastMicErrorTime = Now;
}

/*
    ========================================================================

    Routine Description:
        RecvGetMacContext

    Arguments:
        pAd Pointer to our adapter

    Return Value:
        None

    IRQL = DISPATCH_LEVEL
    
    Note:

    Packet Header Format:
                    ToDS    FrDs    Addr1   Addr2   Addr3
        IBSS        0       0       DA      SA      BSSID
        To AP       1       0       BSSID   SA      DA
        From AP     0       1       DA      BSSID   SA
    ========================================================================
*/
VOID RecvGetMacContext(
    IN  PMP_ADAPTER  pAd,
    IN  PHEADER_802_11  pHeader,
    OUT PMP_PORT *ppPort)
{
    PMP_PORT        pApGoPort = pAd->BeaconPort.pApGoPort;
    UCHAR PortIndex = 0;
    *ppPort = NULL;

#if(COMPILE_WIN8_ABOVE(CURRENT_OS_NDIS_VER))  
    if(NDIS_WIN8_ABOVE(pAd))
    {
        // port 0 as default.
        *ppPort = pAd->PortList[PORT_0];

        if (pHeader->Addr1[0] & 0x01)
        {
            // Broadcast and multicast, compare BSSID.
            for (PortIndex = 0; PortIndex < RTMP_MAX_NUMBER_OF_PORT; PortIndex++)
            {
                if ((pAd->PortList[PortIndex] == NULL) ||   (pAd->PortList[PortIndex]->bActive == FALSE))
                {
                    continue;
                }

                // Infrastructure, compare Addr2.
                if (MAC_ADDR_EQUAL(pAd->PortList[PortIndex]->PortCfg.Bssid, pHeader->Addr2))
                {
                    DBGPRINT(RT_DEBUG_INFO, ("INFRABMCast_MacData_1_[%d](%d, %d), ToPort(%d), Addr1(%x %x %x %x %x %x)Addr2[%x %x %x %x %x %x]Addr3(%x %x %x %x %x %x), LatchRfRegs.Channel(%d)\n", 
                                                __LINE__,
                                                pHeader->FC.Type,
                                                pHeader->FC.SubType,                                                
                                                PortIndex,
                                                pHeader->Addr1[0], pHeader->Addr1[1], pHeader->Addr1[2], pHeader->Addr1[3], pHeader->Addr1[4], pHeader->Addr1[5],
                                                pHeader->Addr2[0], pHeader->Addr2[1], pHeader->Addr2[2], pHeader->Addr2[3], pHeader->Addr2[4], pHeader->Addr2[5],
                                                pHeader->Addr3[0], pHeader->Addr3[1], pHeader->Addr3[2], pHeader->Addr3[3], pHeader->Addr3[4], pHeader->Addr3[5],
                                                pAd->HwCfg.LatchRfRegs.Channel));
                    *ppPort = pAd->PortList[PortIndex];   
                    break;
                }

                // IBSS, compare Addr3,
                if (MAC_ADDR_EQUAL(pAd->PortList[PortIndex]->PortCfg.Bssid, pHeader->Addr3))
                {
                    DBGPRINT(RT_DEBUG_INFO, ("IBSSBMCast_MacData_1_[%d](%d, %d), ToPort(%d), Addr1(%x %x %x %x %x %x)Addr2(%x %x %x %x %x %x)Addr3[%x %x %x %x %x %x], LatchRfRegs.Channel(%d)\n", 
                                                __LINE__,
                                                pHeader->FC.Type,
                                                pHeader->FC.SubType,                                                
                                                PortIndex,
                                                pHeader->Addr1[0], pHeader->Addr1[1], pHeader->Addr1[2], pHeader->Addr1[3], pHeader->Addr1[4], pHeader->Addr1[5],
                                                pHeader->Addr2[0], pHeader->Addr2[1], pHeader->Addr2[2], pHeader->Addr2[3], pHeader->Addr2[4], pHeader->Addr2[5],
                                                pHeader->Addr3[0], pHeader->Addr3[1], pHeader->Addr3[2], pHeader->Addr3[3], pHeader->Addr3[4], pHeader->Addr3[5],
                                                pAd->HwCfg.LatchRfRegs.Channel));
                    *ppPort = pAd->PortList[PortIndex];         
                    break;
                }
            }
        }
        else
        {
            // Unicast, compare destination address.                        
            for (PortIndex  = 0; PortIndex < RTMP_MAX_NUMBER_OF_PORT; PortIndex++)
            {
                if ((pAd->PortList[PortIndex] == NULL) ||   (pAd->PortList[PortIndex]->bActive == FALSE))
                {
                    continue;
                }
                if ((pAd->PortList[PortIndex] != NULL) && 
                    (pAd->PortList[PortIndex]->bActive) &&
                    MAC_ADDR_EQUAL(pAd->PortList[PortIndex]->CurrentAddress, pHeader->Addr1))
                {
                    DBGPRINT(RT_DEBUG_INFO, ("Unicast_MacData_1_[%d](%d, %d), ToPort(%d), Addr1[%x %x %x %x %x %x]Addr2(%x %x %x %x %x %x)Addr3(%x %x %x %x %x %x), LatchRfRegs.Channel(%d)\n", 
                                                __LINE__,
                                                pHeader->FC.Type,
                                                pHeader->FC.SubType,                                                
                                                PortIndex,
                                                pHeader->Addr1[0], pHeader->Addr1[1], pHeader->Addr1[2], pHeader->Addr1[3], pHeader->Addr1[4], pHeader->Addr1[5],
                                                pHeader->Addr2[0], pHeader->Addr2[1], pHeader->Addr2[2], pHeader->Addr2[3], pHeader->Addr2[4], pHeader->Addr2[5],
                                                pHeader->Addr3[0], pHeader->Addr3[1], pHeader->Addr3[2], pHeader->Addr3[3], pHeader->Addr3[4], pHeader->Addr3[5],
                                                pAd->HwCfg.LatchRfRegs.Channel));
                    *ppPort = pAd->PortList[PortIndex];
                    break;
                }
            }
        }

        //Return here for win8
        return;
    }
    else
#endif
    {   
        if (PlatformEqualMemory(pAd->PortList[PORT_0]->CurrentAddress, pHeader->Addr1, MAC_ADDR_LEN)
            && (pHeader->FC.ToDs == 1)
            && (pAd->PortList[PORT_0]->PortSubtype == PORTSUBTYPE_P2PGO || pAd->PortList[PORT_0]->PortSubtype == PORTSUBTYPE_SoftapAP))
        {
            *ppPort = pAd->PortList[PORT_0];
        }
        else if ((pAd->PortList[PORT_2] != NULL) &&
                    (pAd->PortList[PORT_2]->bActive) &&
                    ((pAd->OpMode == OPMODE_STAP2P) || (pApGoPort && pApGoPort->SoftAP.bAPStart && (pApGoPort->SoftAP.ApCfg.ApPortNum == PORT_2))))
        {
            //The packet is belonged to this port if addr1 matches device address (for all packet types)
            if (PlatformEqualMemory(pAd->PortList[PORT_2]->CurrentAddress, pHeader->Addr1, MAC_ADDR_LEN))
            {
                *ppPort = pAd->PortList[PORT_2];
            }
            //The packet is belonged to this port if addr2 or addr3 matches device's BSSID (for From AP and IBSS)
            else if (PlatformEqualMemory(pAd->PortList[PORT_2]->PortCfg.Bssid,pHeader->Addr2, MAC_ADDR_LEN) ||
                PlatformEqualMemory(pAd->PortList[PORT_2]->PortCfg.Bssid,pHeader->Addr3,MAC_ADDR_LEN))
            {
                *ppPort = pAd->PortList[PORT_2];
            }
            else
            {
                *ppPort = pAd->PortList[PORT_0];
            }
        }
        else if ((pAd->PortList[PORT_1] != NULL) &&
                    (pAd->PortList[PORT_1]->bActive) &&
                    (pApGoPort && pApGoPort->SoftAP.bAPStart && pApGoPort->SoftAP.ApCfg.ApPortNum == PORT_1))
        {
            if (PlatformEqualMemory(pAd->PortList[PORT_1]->CurrentAddress, pHeader->Addr1, MAC_ADDR_LEN)
                 && (pAd->PortList[PORT_1]->PortSubtype == PORTSUBTYPE_VwifiAP))
            {
                *ppPort = pAd->PortList[PORT_1];
            }
            else if (PlatformEqualMemory(pAd->PortList[PORT_1]->PortCfg.Bssid,pHeader->Addr2, MAC_ADDR_LEN) ||
                PlatformEqualMemory(pAd->PortList[PORT_1]->PortCfg.Bssid,pHeader->Addr3,MAC_ADDR_LEN))
            {
                *ppPort = pAd->PortList[PORT_1];
            }
            else
                *ppPort = pAd->PortList[PORT_0];
        }
        else
        {
            *ppPort = pAd->PortList[PORT_0];
        }
    }   
}

/*
    ========================================================================

    Routine Description:
        Packet debug for EAP, EAPOL, ICMP

    Arguments:
        pHeader Pointer to PHEADER_802_11
        pData Pointer to PUCHAR

    Return Value:
        None

    IRQL = DISPATCH_LEVEL
    
    Note:
    ========================================================================
*/
#if DBG
VOID RecvPacketDebug(
    IN  PMP_PORT      pPort,
    IN  PUCHAR  pData)
{
    PHEADER_802_11  pHeader = (PHEADER_802_11) (pData);
    
    // Debug for WPS/WPA packet lost issue
    if (pHeader->FC.Type == BTYPE_DATA)
    {
        UCHAR   Ver = 0;
        UCHAR   HdrLen = 0;
        USHORT  TotalLen = 0;
        USHORT  Id = 0;

        UCHAR   Type = 0;
        UCHAR   Code = 0;
        USHORT  CheckSum = 0;
        USHORT  ICMPId = 0;
        USHORT  Seq = 0;
        
        PUCHAR  pNewData = NULL;
        UCHAR   i = 0;
        UCHAR   NextProtocol = 0;

        for (i = 0; i < 40; i++)
        {
            // 802.1x Authentication
            if ((*(pData + i) == 0x88) && (*(pData + i + 1) == 0x8e))
            {
                // 802.1x
                pNewData = (PUCHAR)(pData + i + 2);

                // EAPOL-Start
                if (*(pNewData + 1) == 1)
                {
                    DBGPRINT(RT_DEBUG_TRACE,("[EAPOL-Start packet]\n"));
                }
                // EAP Packet
                else if (*(pNewData + 1) == 0)
                {
                    pNewData = (PUCHAR)(pData + i + 2 + 4);

                    // EAP failure
                    if (*(pNewData + 0) == 4)
                    {
                        DBGPRINT(RT_DEBUG_TRACE,("[EAP WPS packet] Code(%d), Id(%d), Len(%d)\n", 
                                                    *(pNewData + 0),
                                                    *(pNewData + 1),
                                                    *(pNewData + 2) * 256 + *(pNewData + 3)));
                    }
                    //EAP request/response
                    else
                    {
                        DBGPRINT(RT_DEBUG_TRACE,("[EAP WPS packet] Code(%d), Id(%d), Len(%d), Type(%d)\n", 
                                                    *(pNewData + 0),
                                                    *(pNewData + 1),
                                                    *(pNewData + 2) * 256 + *(pNewData + 3),
                                                    *(pNewData + 4)));
                    }
                }
                // EAPOL-key
                else if (*(pNewData + 1) == 3)
                {
                    DBGPRINT(RT_DEBUG_TRACE,("[EAPOL-key packet] Size = %d\n", *(pNewData + 2) * 256 + *(pNewData + 3)));
                }
                
                break;
            }
            // IPv4
            else if ((*(pData + i) == 0x08) && (*(pData + i + 1) == 0x00))
            {
                // IPv4 start address
                pNewData = (PUCHAR)(pData + i + 2);
                Ver = ((*pNewData) & 0xf0) >> 4;
                HdrLen = ((*pNewData) & 0x0f) * 4;
                TotalLen = *(pNewData + 2) * 256 + *(pNewData + 3);
                Id = *(pNewData + 4) * 256 + *(pNewData + 5);
                
                // NextProtocol == ICMP
                NextProtocol = *(pNewData + 9);
                
                if (NextProtocol == 0x01)
                {
                    // ICMP start address
                    pNewData = (PUCHAR)(pNewData + /*20*/HdrLen);
                    Type = *(pNewData + 0);
                    Code = *(pNewData + 1);
                    CheckSum = *(pNewData + 2) * 256 + *(pNewData + 3);
                    ICMPId =  *(pNewData + 4) * 256 + *(pNewData + 5);
                    Seq = *(pNewData + 6) * 256 + *(pNewData + 7);

                    DBGPRINT(RT_DEBUG_INFO,("[Port %d] Seq(%d) [IPv4 Rx] Ver(%d), HdrLen(%d), TotalLen(%d), Id(%5d) [ICMP Rx] Type(%d), Code(%d), CheckSum(%d), ICMPId(%d), Seq(%d)\n\n", 
                                                pPort->PortNumber,
                                                pHeader->Sequence,
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
                    DBGPRINT(RT_DEBUG_INFO,("[Port %d] Seq(%d) [IPv4 Rx NOT ICMP] Ver(%d), HdrLen(%d), TotalLen(%d), Id(%5d), NextProtocol(%d)\n\n", 
                                                pPort->PortNumber,
                                                pHeader->Sequence,
                                                Ver,
                                                HdrLen,
                                                TotalLen,
                                                Id,
                                                NextProtocol));
                }
                
                break;
            }
        }
    }
}
#endif
