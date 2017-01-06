/*
 ***************************************************************************
 * Ralink Tech Inc.
 * 4F, No. 2 Technology 5th Rd.
 * Science-based Industrial Park
 * Hsin-chu, Taiwan, R.O.C.
 *
 * (c) Copyright 2002-2004, Ralink Technology, Inc.
 *
 * All rights reserved. Ralink's source code is an unpublished work and the
 * use of a copyright notice does not imply otherwise. This source code
 * contains confidential trade secret material of Ralink Tech. Any attemp
 * or participation in deciphering, decoding, reverse engineering or in any
 * way altering the source code is stricitly prohibited, unless the prior
 * written consent of Ralink Technology, Inc. is obtained.
 ***************************************************************************

    Module Name:
    dls.c

    Abstract:
    Handle WMM-DLS state machine

    Revision History:
    Who         When            What
    --------    ----------      ----------------------------------------------
    Rory        2006-2-14       Direct Link Setup
*/
#include "MtConfig.h"

#define DRIVER_FILE         0x00800000

/*
    ==========================================================================
    Description:
        dls state machine init, including state transition and timer init
    Parameters:
        Sm - pointer to the dls state machine
    Note:
        The state machine looks like this
    
                            DLS_IDLE
    MT2_MLME_DLS_REQUEST   MlmeDlsReqAction
    MT2_PEER_DLS_REQUEST   PeerDlsReqAction
    MT2_PEER_DLS_RESPONSE  PeerDlsRspAction
    MT2_MLME_DLS_TEARDOWN  MlmeTearDownAction
    MT2_PEER_DLS_TEARDOWN  PeerTearDownAction
    
    IRQL = PASSIVE_LEVEL

    ==========================================================================
 */
void DlsStateMachineInit(
    IN PMP_ADAPTER pAd, 
    IN STATE_MACHINE *Sm, 
    OUT STATE_MACHINE_FUNC Trans[]) 
{   
    StateMachineInit(Sm, Trans, MAX_DLS_STATE, MAX_DLS_MSG, Drop, DLS_IDLE, DLS_MACHINE_BASE);
 
    // the first column
    StateMachineSetAction(Sm, DLS_IDLE, MT2_MLME_DLS_REQ, MlmeDlsReqAction);
    StateMachineSetAction(Sm, DLS_IDLE, MT2_PEER_DLS_REQ, PeerDlsReqAction);
    StateMachineSetAction(Sm, DLS_IDLE, MT2_PEER_DLS_RSP, PeerDlsRspAction);
    StateMachineSetAction(Sm, DLS_IDLE, MT2_MLME_DLS_TEAR_DOWN, MlmeDlsTearDownAction);
    StateMachineSetAction(Sm, DLS_IDLE, MT2_PEER_DLS_TEAR_DOWN, PeerDlsTearDownAction);
}

/*
    ==========================================================================
    Description:
        
    IRQL = DISPATCH_LEVEL

    ==========================================================================
 */
VOID MlmeDlsReqAction(
    IN PMP_ADAPTER pAd, 
    IN MLME_QUEUE_ELEM *Elem) 
{
    PUCHAR          pOutBuffer = NULL;
    NDIS_STATUS     NStatus;
    ULONG           FrameLen = 0;
    HEADER_802_11   DlsReqHdr;
    PRT_802_11_DLS  pDLS = NULL;
    UCHAR           Category = CATEGORY_DLS;
    UCHAR           Action = ACTION_DLS_REQUEST;
    ULONG           tmpLen;
    USHORT          reason;
    ULONG           Timeout;
    BOOLEAN         TimerCancelled;
    ULONG           tmp;
    PMP_PORT      pPort = pAd->PortList[Elem->PortNum];

    if(!MlmeDlsReqSanity(pAd, Elem->Msg, Elem->MsgLen, &pDLS, &reason))
        return;

    DBGPRINT(RT_DEBUG_TRACE,("DLS - MlmeDlsReqAction() \n"));
    
    NStatus = MlmeAllocateMemory(pAd, &pOutBuffer);  //Get an unused nonpaged memory
    if (NStatus != NDIS_STATUS_SUCCESS) 
    {
        DBGPRINT(RT_DEBUG_ERROR,("DLS - MlmeDlsReqAction() allocate memory failed \n"));
        return;
    }

    ActHeaderInit(pAd,pPort, &DlsReqHdr, pPort->PortCfg.Bssid, pPort->PortCfg.Bssid);
        
    // Build basic frame first
    MakeOutgoingFrame(pOutBuffer,               &FrameLen,
                    sizeof(HEADER_802_11),      &DlsReqHdr,
                    1,                          &Category,
                    1,                          &Action,
                    6,                          &pDLS->MacAddr,
                    6,                          pPort->CurrentAddress,
                    2,                          &pAd->StaActive.CapabilityInfo,
                    2,                          &pDLS->TimeOut,
                    1,                          &SupRateIe,
                    1,                          &pAd->MlmeAux.SupRateLen,
                    pAd->MlmeAux.SupRateLen,    pAd->MlmeAux.SupRate,
                    END_OF_ARGS);

    if (pAd->MlmeAux.ExtRateLen != 0)
    {
        MakeOutgoingFrame(pOutBuffer + FrameLen,    &tmpLen,
                          1,                        &ExtRateIe,
                          1,                        &pAd->MlmeAux.ExtRateLen,
                          pAd->MlmeAux.ExtRateLen,  pAd->MlmeAux.ExtRate,
                          END_OF_ARGS);
        FrameLen += tmpLen;
    }

    //HT capability IE
    if ((pAd->MlmeAux.HtCapabilityLen > 0) && (pPort->CommonCfg.PhyMode >= PHY_11ABGN_MIXED))
    {
        HT_CAPABILITY_IE HtCap;
        PMAC_TABLE_ENTRY pMacTabEntry = MlmeSyncMacTabMatchedRoleType(pPort, ROLE_WLANIDX_BSSID);
        if(pMacTabEntry == NULL)
        {
            DBGPRINT(RT_DEBUG_TRACE,("%s(), Line:%d, Mac Entry is NULL\n", __FUNCTION__, __LINE__));
            return;
        }
        
        PlatformMoveMemory(&HtCap, &pAd->MlmeAux.HtCapability, pAd->MlmeAux.HtCapabilityLen);

        if ((READ_PHY_CFG_BW(pAd, &pMacTabEntry->TxPhyCfg) == BW_40) || 
             (READ_PHY_CFG_BW(pAd, &pMacTabEntry->TxPhyCfg) == BW_80))
        {
            //HtCap.HtCapInfo.ChannelWidth = pPort->MacTab.Content[BSSID_WCID].HTPhyMode.field.BW;
            HtCap.HtCapInfo.ChannelWidth = BW_40;
        }
        
        MakeOutgoingFrame(pOutBuffer + FrameLen,            &tmp,
                            1,                              &HtCapIe,
                            1,                              &pAd->MlmeAux.HtCapabilityLen,
                            pAd->MlmeAux.HtCapabilityLen,   &HtCap,//pAd->MlmeAux.HtCapability, 
                            END_OF_ARGS);
        FrameLen += tmp;
    }

    PlatformCancelTimer(&pDLS->Timer, &TimerCancelled);
    Timeout = DLS_TIMEOUT;
    PlatformSetTimer(pPort,&pDLS->Timer, Timeout);

    NdisCommonMiniportMMRequest(pAd, pOutBuffer, FrameLen);
    //MlmeFreeMemory(pAd, pOutBuffer);
}

/*
    ==========================================================================
    Description:
    
    IRQL = DISPATCH_LEVEL

    ==========================================================================
 */
VOID PeerDlsReqAction(
    IN PMP_ADAPTER pAd, 
    IN MLME_QUEUE_ELEM *Elem) 
{
    PUCHAR          pOutBuffer = NULL;
    NDIS_STATUS     NStatus;
    ULONG           FrameLen = 0;
    USHORT          StatusCode = MLME_SUCCESS;
    HEADER_802_11   DlsRspHdr;
    UCHAR           Category = CATEGORY_DLS;
    UCHAR           Action = ACTION_DLS_RESPONSE;
    ULONG           tmpLen;
    USHORT          CapabilityInfo;
    UCHAR           DA[MAC_ADDR_LEN], SA[MAC_ADDR_LEN];
    USHORT          DLSTimeOut;
    SHORT           i = 0;
    ULONG           Timeout;
    BOOLEAN         TimerCancelled;
    PRT_802_11_DLS  pDLS = NULL;
    HT_CAPABILITY_IE HtCap;
    ULONG HtCapLen = 0;
    ULONG tmp;
    PFRAME_802_11   pFrame = (PFRAME_802_11)Elem->Msg;
    PMP_PORT      pPort = pAd->PortList[Elem->PortNum];

    if (!PeerDlsReqSanity(pAd, Elem->Msg, Elem->MsgLen, DA, SA, &CapabilityInfo, &DLSTimeOut, &HtCap, &HtCapLen)) 
        return;

    //When softap is started, we will receive DLS request sent by ourselves, after AP
    //forwards this packet to another station, this packet should be ignored.
    if (MAC_ADDR_EQUAL(SA, pPort->CurrentAddress))
    {
        return;
    }
    if (!MAC_ADDR_EQUAL(pFrame->Hdr.Addr2, pPort->PortCfg.Bssid))
    {
        return;
    }

    DBGPRINT(RT_DEBUG_TRACE,("DLS - PeerDlsReqAction() from %02x:%02x:%02x:%02x:%02x:%02x\n", SA[0], SA[1], SA[2], SA[3], SA[4], SA[5]));

    NStatus = MlmeAllocateMemory(pAd, &pOutBuffer);  //Get an unused nonpaged memory
    if (NStatus != NDIS_STATUS_SUCCESS) 
    {
        DBGPRINT(RT_DEBUG_ERROR,("DLS - PeerDlsReqAction() allocate memory failed \n"));
        return;
    }

    if (!INFRA_ON(pPort))
    {
        StatusCode = MLME_REQUEST_DECLINED;
    }
    else if (!pPort->CommonCfg.bWmmCapable)
    {
        StatusCode = MLME_DEST_STA_IS_NOT_A_QSTA;
        DBGPRINT(RT_DEBUG_TRACE,("DLS - PeerDlsReqAction() MLME_DEST_STA_IS_NOT_A_QSTA\n"));
    }
    else if (!DLS_ON(pAd))
    {
        StatusCode = MLME_REQUEST_DECLINED;
        DBGPRINT(RT_DEBUG_TRACE,("DLS - PeerDlsReqAction() MLME_REQUEST_DECLINED\n"));
    }
    else if ((pPort->SoftAP.bAPStart) || (pPort->P2PCfg.bGOStart))
    {
        StatusCode = MLME_REQUEST_DECLINED;
    }
    else
    {
        // find table to update parameters
        for (i = MAX_NUM_OF_DLS_ENTRY-1; i >= 0; i--)
        {
            if (pAd->StaCfg.DLSEntry[i].Valid && MAC_ADDR_EQUAL(SA, pAd->StaCfg.DLSEntry[i].MacAddr))
            {
                if ((pPort->PortCfg.AuthMode >= Ralink802_11AuthModeWPA) && (pPort->PortCfg.AuthMode != Ralink802_11AuthModeAutoSwitch))
                    pAd->StaCfg.DLSEntry[i].Status = DLS_WAIT_KEY;
                else
                {
                    PlatformCancelTimer(&pPort->StaCfg.DLSEntry[i].Timer, &TimerCancelled);
                    pAd->StaCfg.DLSEntry[i].Status = DLS_FINISH;
                }
                
                pAd->StaCfg.DLSEntry[i].TimeOut = DLSTimeOut;
                pAd->StaCfg.DLSEntry[i].CountDownTimer = DLSTimeOut;
                pDLS = &pAd->StaCfg.DLSEntry[i];
                DBGPRINT(RT_DEBUG_TRACE,("DLS - PeerDlsReqAction() find entry \n"));
                break;
            }
        }

        // can not find in table, create a new one
        if (i < 0)
        {
            DBGPRINT(RT_DEBUG_TRACE,("DLS - PeerDlsReqAction() can not find same entry \n"));
            for (i=(MAX_NUM_OF_DLS_ENTRY-1); i>=MAX_NUM_OF_INIT_DLS_ENTRY; i--)
            {
                if (!pAd->StaCfg.DLSEntry[i].Valid)
                {
                    if ((pPort->PortCfg.AuthMode >= Ralink802_11AuthModeWPA) && (pPort->PortCfg.AuthMode != Ralink802_11AuthModeAutoSwitch))
                        pAd->StaCfg.DLSEntry[i].Status = DLS_WAIT_KEY;
                    else
                    {
                        PlatformCancelTimer(&pPort->StaCfg.DLSEntry[i].Timer, &TimerCancelled);
                        pAd->StaCfg.DLSEntry[i].Status = DLS_FINISH;
                    }

                    pAd->StaCfg.DLSEntry[i].Valid = TRUE;                   
                    pAd->StaCfg.DLSEntry[i].TimeOut = DLSTimeOut;
                    pAd->StaCfg.DLSEntry[i].CountDownTimer = DLSTimeOut;
                    PlatformMoveMemory(pAd->StaCfg.DLSEntry[i].MacAddr, SA, MAC_ADDR_LEN);
                    pDLS = &pAd->StaCfg.DLSEntry[i];
                    break;
                }
            }
        }
        StatusCode = MLME_SUCCESS;

        // can not find in table, create a new one
        if (i < 0)
        {
            StatusCode = MLME_QOS_UNSPECIFY;
            DBGPRINT(RT_DEBUG_ERROR,("DLS - PeerDlsReqAction() DLSEntry table full(only can support %d DLS session) \n", MAX_NUM_OF_DLS_ENTRY - MAX_NUM_OF_INIT_DLS_ENTRY));
        }
        else
        {
            DBGPRINT(RT_DEBUG_ERROR,("DLS - PeerDlsReqAction() use entry(%d) %02x:%02x:%02x:%02x:%02x:%02x\n",
                i, SA[0], SA[1], SA[2], SA[3], SA[4], SA[5]));
        }
    }

    //Insert into mac table
    if (StatusCode == MLME_SUCCESS)
    {
        PMAC_TABLE_ENTRY pMacEntry = MacTableLookup(pAd,  pPort, pAd->StaCfg.DLSEntry[i].MacAddr);
        if (pMacEntry == NULL)
        {
            pMacEntry = MacTableInsertEntry(pAd, pPort, pAd->StaCfg.DLSEntry[i].MacAddr, FALSE);
            if (pMacEntry)
            {
                //This has to be done, different with windows driver.

                //Keep wcid
                pAd->StaCfg.DLSEntry[i].Wcid = (UCHAR)pMacEntry->Aid;

                //Set state or DRS will not work
                pMacEntry->Sst = SST_ASSOC;

                //The DLS entry is always ralink chipse, AMPDU will be used for TKIP encryption.
                CLIENT_STATUS_SET_FLAG(pMacEntry, fCLIENT_STATUS_RALINK_CHIPSET);

                //Legacy as default
                WRITE_PHY_CFG_MODE(pAd, &pMacEntry->TxPhyCfg, MODE_OFDM);


                //HT will be used only when both parties are HT capable
                if ((HtCapLen > 0) && (pAd->MlmeAux.HtCapabilityLen > 0))
                {
                    PMAC_TABLE_ENTRY pMacTabEntry = MlmeSyncMacTabMatchedRoleType(pPort, ROLE_WLANIDX_BSSID);
                    if(pMacTabEntry == NULL)
                    {
                        DBGPRINT(RT_DEBUG_TRACE,("%s(), Line:%d, Mac Entry is NULL\n", __FUNCTION__, __LINE__));
                        return;
                    }
                    
                    PlatformMoveMemory(&pMacEntry->HTCapability, &HtCap, sizeof(HT_CAPABILITY_IE));

                    WRITE_PHY_CFG_MODE(pAd, &pMacEntry->TxPhyCfg, MODE_HTMIX);
                    pMacEntry->MmpsMode = (UCHAR)HtCap.HtCapInfo.MimoPs;
                    WRITE_PHY_CFG_BW(pAd, &pMacEntry->TxPhyCfg, BW_20);

                    //BW40 will be used only both parties uses BW40
                    if ((HtCap.HtCapInfo.ChannelWidth) && 
                         ((READ_PHY_CFG_BW(pAd, &pMacTabEntry->TxPhyCfg) == BW_40) || 
                           (READ_PHY_CFG_BW(pAd, &pMacTabEntry->TxPhyCfg) == BW_80)))
                    {
                        WRITE_PHY_CFG_BW(pAd, &pMacEntry->TxPhyCfg, BW_40);
                    }

                    pMacEntry->BaSizeInUse = (UCHAR)pPort->CommonCfg.BACapability.field.TxBAWinLimit;
                    if (pPort->CommonCfg.BACapability.field.AutoBA)
                    {
                        int j;
                        for (j = 0; j < NUM_OF_TID; j++)
                        {
                            BATableInsertEntry(pAd,pPort, pMacEntry->Aid, 0, 0x1, (UCHAR)j, BaSizeArray[HtCap.HtCapParm.MaxRAmpduFactor], Originator_SetAutoAdd, FALSE);
                            pMacEntry->TXAutoBAbitmap |= (1 << j);
                        }
                        DBGPRINT(RT_DEBUG_TRACE, ("BA BaSizeInUse = %d, TXBAbitmap=%x, RXBAbitmap=%x \n",
                            pMacEntry->BaSizeInUse,
                            pMacEntry->TXBAbitmap,
                            pMacEntry->RXBAbitmap));
                    }
                    DBGPRINT(RT_DEBUG_TRACE,("DLS - HT cap found, ch width(%d), MCS[0] = %x, MCS[1] = %x\n",
                            HtCap.HtCapInfo.ChannelWidth,
                            HtCap.MCSSet[0],
                            HtCap.MCSSet[1]));
                }

                //
                //Initialize data rate, BA will be setup faster.
                //
                {
                    PUCHAR pTable;
                    UCHAR TableSize;
                    UCHAR InitTxRateIdx;
                    PRTMP_TX_RATE_SWITCH pTxRate;

                    pMacEntry->RateLen = 12;
                    MlmeSelectTxRateTable(pAd, pMacEntry, &pTable, &TableSize, &InitTxRateIdx);
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
                    DBGPRINT(RT_DEBUG_TRACE,("DLS - init MCS %d\n", READ_PHY_CFG_MCS(pAd, &pMacEntry->TxPhyCfg)));
                }
            }
        }
    }

    ActHeaderInit(pAd,pPort,&DlsRspHdr, pPort->PortCfg.Bssid, pPort->PortCfg.Bssid);
        
    // Build basic frame first
    if (StatusCode == MLME_SUCCESS)
    {
        MakeOutgoingFrame(pOutBuffer,               &FrameLen,
                        sizeof(HEADER_802_11),      &DlsRspHdr,
                        1,                          &Category,
                        1,                          &Action,
                        2,                          &StatusCode,
                        6,                          SA,
                        6,                          pPort->CurrentAddress,
                        2,                          &pAd->StaActive.CapabilityInfo,
                        1,                          &SupRateIe,
                        1,                          &pAd->MlmeAux.SupRateLen,
                        pAd->MlmeAux.SupRateLen,    pAd->MlmeAux.SupRate,
                        END_OF_ARGS);

        if (pAd->MlmeAux.ExtRateLen != 0)
        {
            MakeOutgoingFrame(pOutBuffer + FrameLen,    &tmpLen,
                              1,                        &ExtRateIe,
                              1,                        &pAd->MlmeAux.ExtRateLen,
                              pAd->MlmeAux.ExtRateLen,  pAd->MlmeAux.ExtRate,
                              END_OF_ARGS);
            FrameLen += tmpLen;
        }

        //HT capability IE
        if ((pAd->MlmeAux.HtCapabilityLen > 0) && (pPort->CommonCfg.PhyMode >= PHY_11ABGN_MIXED))
        {
            PMAC_TABLE_ENTRY pMacTabEntry = MlmeSyncMacTabMatchedRoleType(pPort, ROLE_WLANIDX_BSSID);
            if(pMacTabEntry == NULL)
            {
                DBGPRINT(RT_DEBUG_TRACE,("%s(), Line:%d, Mac Entry is NULL\n", __FUNCTION__, __LINE__));
                return;
            }
            
            PlatformMoveMemory(&HtCap, &pAd->MlmeAux.HtCapability, pAd->MlmeAux.HtCapabilityLen);

            if ((READ_PHY_CFG_BW(pAd, &pMacTabEntry->TxPhyCfg) == BW_40) || 
                 (READ_PHY_CFG_BW(pAd, &pMacTabEntry->TxPhyCfg) == BW_80))
            {
                HtCap.HtCapInfo.ChannelWidth = BW_40;
            }
            
            MakeOutgoingFrame(pOutBuffer + FrameLen,            &tmp,
                                1,                              &HtCapIe,
                                1,                              &pAd->MlmeAux.HtCapabilityLen,
                                pAd->MlmeAux.HtCapabilityLen,   &HtCap,
                                END_OF_ARGS);
            FrameLen += tmp;
        }

        if (pDLS && (pDLS->Status != DLS_FINISH))
        {
            PlatformCancelTimer(&pDLS->Timer, &TimerCancelled);
            Timeout = DLS_TIMEOUT;
            PlatformSetTimer(pPort,&pDLS->Timer, Timeout);
        }
    }
    else
    {
        MakeOutgoingFrame(pOutBuffer,               &FrameLen,
                        sizeof(HEADER_802_11),      &DlsRspHdr,
                        1,                          &Category,
                        1,                          &Action,
                        2,                          &StatusCode,
                        6,                          SA,
                        6,                          pPort->CurrentAddress,
                        END_OF_ARGS);
    }

    DBGPRINT(RT_DEBUG_TRACE,("DLS - PeerDlsReqAction() SEND OUT \n"));
    NdisCommonMiniportMMRequest(pAd, pOutBuffer, FrameLen);
    //MlmeFreeMemory(pAd, pOutBuffer);
}

/*
    ==========================================================================
    Description:
    
    IRQL = DISPATCH_LEVEL

    ==========================================================================
 */
VOID PeerDlsRspAction(
    IN PMP_ADAPTER pAd, 
    IN MLME_QUEUE_ELEM *Elem) 
{
    USHORT          CapabilityInfo;
    UCHAR           DA[MAC_ADDR_LEN], SA[MAC_ADDR_LEN];
    USHORT          StatusCode;
    SHORT           i,j;
    BOOLEAN         TimerCancelled;
    HT_CAPABILITY_IE HtCap;
    ULONG HtCapLen = 0;
    PMP_PORT  pPort = pAd->PortList[Elem->PortNum];
    PFRAME_802_11   pFrame = (PFRAME_802_11)Elem->Msg;

    if (!DLS_ON(pAd))
        return;

    if (!INFRA_ON(pPort))
        return;

    if (!PeerDlsRspSanity(pAd, Elem->Msg, Elem->MsgLen, DA, SA, &CapabilityInfo,  &HtCap, &HtCapLen,&StatusCode)) 
        return;

    //When softap is started, we will receive DLS request sent by ourselves, after AP
    //forwards this packet to another station, this packet should be ignored.
    if (MAC_ADDR_EQUAL(SA, pPort->CurrentAddress))
    {
        return;
    }
    if (!MAC_ADDR_EQUAL(pFrame->Hdr.Addr2, pPort->PortCfg.Bssid))
    {
        return;
    }

    DBGPRINT(RT_DEBUG_TRACE,("DLS - PeerDlsRspAction() from %02x:%02x:%02x:%02x:%02x:%02x with StatusCode=%d, CapabilityInfo=0x%x\n",
        SA[0], SA[1], SA[2], SA[3], SA[4], SA[5], StatusCode, CapabilityInfo));

    for (i = 0; i < MAX_NUM_OF_INIT_DLS_ENTRY; i++)
    {
        if (pAd->StaCfg.DLSEntry[i].Valid && MAC_ADDR_EQUAL(SA, pAd->StaCfg.DLSEntry[i].MacAddr))
        {
            if (StatusCode == MLME_SUCCESS)
            {
                PMAC_TABLE_ENTRY pMacEntry;
                if ((pPort->PortCfg.AuthMode >= Ralink802_11AuthModeWPA) && (pPort->PortCfg.AuthMode != Ralink802_11AuthModeAutoSwitch))
                {
                    // If support WPA or WPA2, start STAKey hand shake,
                    // If failed hand shake, just tear down peer DLS
                    if (RTMPSendSTAKeyRequest(pAd, pPort, pAd->StaCfg.DLSEntry[i].MacAddr) != NDIS_STATUS_SUCCESS)
                    {
                        MLME_DLS_REQ_STRUCT MlmeDlsReq;
                        USHORT              reason = REASON_QOS_CIPHER_NOT_SUPPORT;
                        
                        MlmeCntDlsParmFill(pAd, &MlmeDlsReq, &pAd->StaCfg.DLSEntry[i], reason);
                        MlmeEnqueue(pAd,pPort, DLS_STATE_MACHINE, MT2_MLME_DLS_TEAR_DOWN, sizeof(MLME_DLS_REQ_STRUCT), &MlmeDlsReq);
                        pAd->StaCfg.DLSEntry[i].Status = DLS_NONE;
                        pAd->StaCfg.DLSEntry[i].Valid   = FALSE;
                        DBGPRINT(RT_DEBUG_ERROR,("DLS - PeerDlsRspAction failed when call RTMPSendSTAKeyRequest \n"));
                    }
                    else
                    {
                        pAd->StaCfg.DLSEntry[i].Status = DLS_WAIT_KEY;
                        DBGPRINT(RT_DEBUG_TRACE,("DLS - waiting for STAKey handshake procedure\n"));
                    }
                }
                else
                {
                    PlatformCancelTimer(&pPort->StaCfg.DLSEntry[i].Timer, &TimerCancelled);
                    pAd->StaCfg.DLSEntry[i].Status = DLS_FINISH;
                    DBGPRINT(RT_DEBUG_TRACE,("DLS - PeerDlsRspAction() from %02x:%02x:%02x:%02x:%02x:%02x Succeed with WEP or no security\n", SA[0], SA[1], SA[2], SA[3], SA[4], SA[5]));
                }

                //Insert into mac table
                pMacEntry = MacTableLookup(pAd,  pPort, pAd->StaCfg.DLSEntry[i].MacAddr);
                if (pMacEntry == NULL)
                {
                    pMacEntry = MacTableInsertEntry(pAd, pPort, pAd->StaCfg.DLSEntry[i].MacAddr, FALSE);
                    if (pMacEntry)
                    {
                        //This has to be done, different with windows driver.

                        //Keep wcid
                        pAd->StaCfg.DLSEntry[i].Wcid = (UCHAR)pMacEntry->Aid;

                        //Set state or DRS will not work
                        pMacEntry->Sst = SST_ASSOC;

                        //The DLS entry is always ralink chipse, AMPDU will be used for TKIP encryption.
                        CLIENT_STATUS_SET_FLAG(pMacEntry, fCLIENT_STATUS_RALINK_CHIPSET);

                        //Legacy as default
                        WRITE_PHY_CFG_MODE(pAd, &pMacEntry->TxPhyCfg, MODE_OFDM);

                        //HT will be used only when both parties are HT capable
                        if ((HtCapLen > 0) && (pAd->MlmeAux.HtCapabilityLen > 0))
                        {
                            PMAC_TABLE_ENTRY pMacTabEntry = MlmeSyncMacTabMatchedRoleType(pPort, ROLE_WLANIDX_BSSID);
                            if(pMacTabEntry == NULL)
                            {
                                DBGPRINT(RT_DEBUG_TRACE,("%s(), Line:%d, Mac Entry is NULL\n", __FUNCTION__, __LINE__));
                                return;
                            }
                            
                            PlatformMoveMemory(&pMacEntry->HTCapability, &HtCap, sizeof(HT_CAPABILITY_IE));

                            WRITE_PHY_CFG_MODE(pAd, &pMacEntry->TxPhyCfg, MODE_HTMIX);
                            pMacEntry->MmpsMode = (UCHAR)HtCap.HtCapInfo.MimoPs;
                            WRITE_PHY_CFG_BW(pAd, &pMacEntry->TxPhyCfg, BW_20);

                            //BW40 will be used only both parties uses BW40
                            if ((HtCap.HtCapInfo.ChannelWidth) && 
                                 ((READ_PHY_CFG_BW(pAd, &pMacTabEntry->TxPhyCfg) == BW_40) || 
                                   (READ_PHY_CFG_BW(pAd, &pMacTabEntry->TxPhyCfg) == BW_80)))
                            {
                                WRITE_PHY_CFG_BW(pAd, &pMacEntry->TxPhyCfg, BW_40);
                            }

                            pMacEntry->BaSizeInUse = (UCHAR)pPort->CommonCfg.BACapability.field.TxBAWinLimit;
                            if (pPort->CommonCfg.BACapability.field.AutoBA)
                            {
                                for (j = 0; j < NUM_OF_TID; j++)
                                {
                                    BATableInsertEntry(pAd, pPort,pMacEntry->Aid, 0, 0x1, (UCHAR)j, BaSizeArray[HtCap.HtCapParm.MaxRAmpduFactor], Originator_SetAutoAdd, FALSE);
                                    pMacEntry->TXAutoBAbitmap |= (1 << j);
                                }
                                DBGPRINT(RT_DEBUG_TRACE, ("BA BaSizeInUse = %d, TXBAbitmap=%x, RXBAbitmap=%x \n",
                                    pMacEntry->BaSizeInUse,
                                    pMacEntry->TXBAbitmap,
                                    pMacEntry->RXBAbitmap));
                            }
                            DBGPRINT(RT_DEBUG_TRACE,("DLS - HT cap found, ch width(%d), MCS[0] = %x, MCS[1] = %x\n",
                                HtCap.HtCapInfo.ChannelWidth,
                                HtCap.MCSSet[0],
                                HtCap.MCSSet[1]));
                        }

                        //
                        //Initialize data rate, BA will be setup faster.
                        //
                        {
                            PUCHAR pTable;
                            UCHAR TableSize;
                            UCHAR InitTxRateIdx;
                            PRTMP_TX_RATE_SWITCH pTxRate;

                            pMacEntry->RateLen = 12;
                            MlmeSelectTxRateTable(pAd, pMacEntry, &pTable, &TableSize, &InitTxRateIdx);
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
                            DBGPRINT(RT_DEBUG_TRACE,("DLS - init MCS %d\n", READ_PHY_CFG_MCS(pAd, &pMacEntry->TxPhyCfg)));
                        }
                    }
                }
            }
            else
            {
                // DLS setup procedure failed.
                pAd->StaCfg.DLSEntry[i].Status = DLS_NONE;
                pAd->StaCfg.DLSEntry[i].Valid   = FALSE;
                PlatformCancelTimer(&pPort->StaCfg.DLSEntry[i].Timer, &TimerCancelled);
                DBGPRINT(RT_DEBUG_ERROR,("DLS - PeerDlsRspAction failed with StatusCode=%d \n", StatusCode));
            }
        }
    }

    if (i >= MAX_NUM_OF_INIT_DLS_ENTRY)
    {
        DBGPRINT(RT_DEBUG_TRACE,("DLS - PeerDlsRspAction() update timeout value \n"));
        for (i = (MAX_NUM_OF_DLS_ENTRY-1); i >= MAX_NUM_OF_INIT_DLS_ENTRY; i--)
        {
            if (pAd->StaCfg.DLSEntry[i].Valid && MAC_ADDR_EQUAL(SA, pAd->StaCfg.DLSEntry[i].MacAddr))
            {
                if (StatusCode == MLME_SUCCESS)
                {
                    if ((pPort->PortCfg.AuthMode >= Ralink802_11AuthModeWPA) && (pPort->PortCfg.AuthMode != Ralink802_11AuthModeAutoSwitch))
                    {
                        // If support WPA or WPA2, start STAKey hand shake,
                        // If failed hand shake, just tear down peer DLS
                        if (RTMPSendSTAKeyRequest(pAd, pPort, pAd->StaCfg.DLSEntry[i].MacAddr) != NDIS_STATUS_SUCCESS)
                        {
                            MLME_DLS_REQ_STRUCT MlmeDlsReq;
                            USHORT              reason = REASON_QOS_CIPHER_NOT_SUPPORT;
                            
                            MlmeCntDlsParmFill(pAd, &MlmeDlsReq, &pAd->StaCfg.DLSEntry[i], reason);
                            MlmeEnqueue(pAd,pPort, DLS_STATE_MACHINE, MT2_MLME_DLS_TEAR_DOWN, sizeof(MLME_DLS_REQ_STRUCT), &MlmeDlsReq);
                            pAd->StaCfg.DLSEntry[i].Status = DLS_NONE;
                            pAd->StaCfg.DLSEntry[i].Valid   = FALSE;
                            DBGPRINT(RT_DEBUG_ERROR,("DLS - PeerDlsRspAction failed when call RTMPSendSTAKeyRequest \n"));
                        }
                        else
                        {
                            pAd->StaCfg.DLSEntry[i].Status = DLS_WAIT_KEY;
                            DBGPRINT(RT_DEBUG_TRACE,("DLS - waiting for STAKey handshake procedure\n"));
                        }
                    }
                    else
                    {
                        PlatformCancelTimer(&pPort->StaCfg.DLSEntry[i].Timer, &TimerCancelled);
                        pAd->StaCfg.DLSEntry[i].Status = DLS_FINISH;
                        DBGPRINT(RT_DEBUG_TRACE,("DLS - PeerDlsRspAction() from %02x:%02x:%02x:%02x:%02x:%02x Succeed with WEP or no security\n", SA[0], SA[1], SA[2], SA[3], SA[4], SA[5]));
                    }
                }
                else
                {
                    // DLS setup procedure failed.
                    pAd->StaCfg.DLSEntry[i].Status = DLS_NONE;
                    pAd->StaCfg.DLSEntry[i].Valid   = FALSE;
                    PlatformCancelTimer(&pPort->StaCfg.DLSEntry[i].Timer, &TimerCancelled);
                    DBGPRINT(RT_DEBUG_ERROR,("DLS - PeerDlsRspAction failed with StatusCode=%d \n", StatusCode));
                }
            }
        }
    }
}

/*
    ==========================================================================
    Description:
    
    IRQL = DISPATCH_LEVEL

    ==========================================================================
 */
VOID MlmeDlsTearDownAction(
    IN PMP_ADAPTER pAd, 
    IN MLME_QUEUE_ELEM *Elem) 
{
    PUCHAR          pOutBuffer = NULL;
    NDIS_STATUS     NStatus;
    ULONG           FrameLen = 0;
    UCHAR           Category = CATEGORY_DLS;
    UCHAR           Action = ACTION_DLS_TEARDOWN;
    USHORT          ReasonCode = REASON_QOS_UNSPECIFY;
    HEADER_802_11   DlsTearDownHdr;
    PRT_802_11_DLS  pDLS;
    BOOLEAN         TimerCancelled;
    UCHAR           i;
    PMP_PORT      pPort = pAd->PortList[Elem->PortNum];
    
    if(!MlmeDlsReqSanity(pAd, Elem->Msg, Elem->MsgLen, &pDLS, &ReasonCode))
        return;

    DBGPRINT(RT_DEBUG_TRACE,("DLS - MlmeDlsTearDownAction() with ReasonCode=%d \n", ReasonCode));
    
    NStatus = MlmeAllocateMemory(pAd, &pOutBuffer);  //Get an unused nonpaged memory
    if (NStatus != NDIS_STATUS_SUCCESS) 
    {
        DBGPRINT(RT_DEBUG_ERROR,("DLS - MlmeDlsTearDownAction() allocate memory failed \n"));
        return;
    }

    ActHeaderInit(pAd, pPort,&DlsTearDownHdr, pPort->PortCfg.Bssid, pPort->PortCfg.Bssid);
        
    // Build basic frame first
    MakeOutgoingFrame(pOutBuffer,               &FrameLen,
                    sizeof(HEADER_802_11),      &DlsTearDownHdr,
                    1,                          &Category,
                    1,                          &Action,
                    6,                          &pDLS->MacAddr,
                    6,                          pPort->CurrentAddress,
                    2,                          &ReasonCode,
                    END_OF_ARGS);

    // Remove key in local dls table entry
    for (i = 0; i < MAX_NUM_OF_INIT_DLS_ENTRY; i++)
    {
        if (MAC_ADDR_EQUAL(pDLS->MacAddr, pAd->StaCfg.DLSEntry[i].MacAddr))
        {
            MtAsicDelWcidTab(pAd, pAd->StaCfg.DLSEntry[i].Wcid);
            AsicRemoveKeyEntry(pAd, pAd->StaCfg.DLSEntry[i].Wcid, 0, TRUE);
            DlsDeleteMacTableEntry(pAd, pPort, pAd->StaCfg.DLSEntry[i].MacAddr);
        }
    }

    // clear peer dls table entry
    for (i = MAX_NUM_OF_INIT_DLS_ENTRY; i < MAX_NUM_OF_DLS_ENTRY; i++)
    {
        if (MAC_ADDR_EQUAL(pDLS->MacAddr, pAd->StaCfg.DLSEntry[i].MacAddr))
        {
            pAd->StaCfg.DLSEntry[i].Status  = DLS_NONE;
            pAd->StaCfg.DLSEntry[i].Valid   = FALSE;
            PlatformCancelTimer(&pPort->StaCfg.DLSEntry[i].Timer, &TimerCancelled);
            MtAsicDelWcidTab(pAd, pAd->StaCfg.DLSEntry[i].Wcid);
            AsicRemoveKeyEntry(pAd, pAd->StaCfg.DLSEntry[i].Wcid, 0, TRUE);
            DlsDeleteMacTableEntry(pAd, pPort, pAd->StaCfg.DLSEntry[i].MacAddr);
        }
    }

    NdisCommonMiniportMMRequest(pAd, pOutBuffer, FrameLen);
    //MlmeFreeMemory(pAd, pOutBuffer);
    PlatformCancelTimer(&pDLS->Timer, &TimerCancelled);
}

/*
    ==========================================================================
    Description:
    
    IRQL = DISPATCH_LEVEL

    ==========================================================================
 */
VOID PeerDlsTearDownAction(
    IN PMP_ADAPTER pAd, 
    IN MLME_QUEUE_ELEM *Elem) 
{
    UCHAR           DA[MAC_ADDR_LEN], SA[MAC_ADDR_LEN];
    USHORT          ReasonCode;
    UINT            i;
    BOOLEAN         TimerCancelled;
    PFRAME_802_11   pFrame = (PFRAME_802_11)Elem->Msg;
    PMP_PORT      pPort = pAd->PortList[Elem->PortNum];

    if (!DLS_ON(pAd))
        return;

    if (!INFRA_ON(pPort))
        return;
    
    if (!PeerDlsTearDownSanity(pAd, Elem->Msg, Elem->MsgLen, DA, SA, &ReasonCode)) 
        return;

    //When softap is started, we will receive DLS request sent by ourselves, after AP
    //forwards this packet to another station, this packet should be ignored.
    if (MAC_ADDR_EQUAL(SA, pPort->CurrentAddress))
    {
        return;
    }
    if (!MAC_ADDR_EQUAL(pFrame->Hdr.Addr2, pPort->PortCfg.Bssid))
    {
        return;
    }

    DBGPRINT(RT_DEBUG_TRACE,("DLS - PeerDlsTearDownAction() from %02x:%02x:%02x:%02x:%02x:%02x with ReasonCode=%d\n", SA[0], SA[1], SA[2], SA[3], SA[4], SA[5], ReasonCode));

    // clear local dls table entry
    for (i = 0; i < MAX_NUM_OF_INIT_DLS_ENTRY; i++)
    {
        if (pAd->StaCfg.DLSEntry[i].Valid && MAC_ADDR_EQUAL(SA, pAd->StaCfg.DLSEntry[i].MacAddr))
        {
            pAd->StaCfg.DLSEntry[i].Status  = DLS_NONE;
            pAd->StaCfg.DLSEntry[i].Valid   = FALSE;
            PlatformCancelTimer(&pPort->StaCfg.DLSEntry[i].Timer, &TimerCancelled);
            MtAsicDelWcidTab(pAd, pAd->StaCfg.DLSEntry[i].Wcid);
            AsicRemoveKeyEntry(pAd, pAd->StaCfg.DLSEntry[i].Wcid, 0, TRUE);
            DlsDeleteMacTableEntry(pAd, pPort, pAd->StaCfg.DLSEntry[i].MacAddr);
        }
    }

    // clear peer dls table entry
    for (i = MAX_NUM_OF_INIT_DLS_ENTRY; i < MAX_NUM_OF_DLS_ENTRY; i++)
    {
        if (pAd->StaCfg.DLSEntry[i].Valid && MAC_ADDR_EQUAL(SA, pAd->StaCfg.DLSEntry[i].MacAddr))
        {
            pAd->StaCfg.DLSEntry[i].Status  = DLS_NONE;
            pAd->StaCfg.DLSEntry[i].Valid   = FALSE;
            PlatformCancelTimer(&pPort->StaCfg.DLSEntry[i].Timer, &TimerCancelled);
            MtAsicDelWcidTab(pAd, pAd->StaCfg.DLSEntry[i].Wcid);
            AsicRemoveKeyEntry(pAd, pAd->StaCfg.DLSEntry[i].Wcid, 0, TRUE);
            DlsDeleteMacTableEntry(pAd, pPort, pAd->StaCfg.DLSEntry[i].MacAddr);
        }
    }
}

/*
    ==========================================================================
    Description:

    IRQL = DISPATCH_LEVEL

    ==========================================================================
 */
VOID RTMPCheckDLSTimeOut(
    IN PMP_ADAPTER    pAd,
    IN PMP_PORT  pPort)
{
    ULONG               i;
    MLME_DLS_REQ_STRUCT MlmeDlsReq;
    USHORT              reason = REASON_QOS_UNSPECIFY;
    
    if (! DLS_ON(pAd))
        return;

    if (pPort->PortSubtype != PORTSUBTYPE_STA)
        return;

    if (! INFRA_ON(pPort))
        return;

    // If timeout value is equaled to zero, it means always not be timeout.

    // update local dls table entry
    for (i = 0; i < MAX_NUM_OF_INIT_DLS_ENTRY; i++)
    {
        if ((pAd->StaCfg.DLSEntry[i].Valid) && (pAd->StaCfg.DLSEntry[i].Status == DLS_FINISH)
            && (pAd->StaCfg.DLSEntry[i].TimeOut != 0))
        {
            if (pAd->StaCfg.DLSEntry[i].CountDownTimer == 0)
            {
                DBGPRINT(RT_DEBUG_TRACE,("DLS timeout %02x %02x %02x %02x %02x %02x \n",
                    pAd->StaCfg.DLSEntry[i].MacAddr[0],
                    pAd->StaCfg.DLSEntry[i].MacAddr[1],
                    pAd->StaCfg.DLSEntry[i].MacAddr[2],
                    pAd->StaCfg.DLSEntry[i].MacAddr[3],
                    pAd->StaCfg.DLSEntry[i].MacAddr[4],
                    pAd->StaCfg.DLSEntry[i].MacAddr[5]));
                reason = REASON_QOS_REQUEST_TIMEOUT;
                pAd->StaCfg.DLSEntry[i].Valid   = FALSE;
                pAd->StaCfg.DLSEntry[i].Status  = DLS_NONE;
                MlmeCntDlsParmFill(pAd, &MlmeDlsReq, &pAd->StaCfg.DLSEntry[i], reason);
                MlmeEnqueue(pAd,pPort, DLS_STATE_MACHINE, MT2_MLME_DLS_TEAR_DOWN, sizeof(MLME_DLS_REQ_STRUCT), &MlmeDlsReq);
            }
            else
                pAd->StaCfg.DLSEntry[i].CountDownTimer --;
        }
    }

    // update peer dls table entry
    for (i=MAX_NUM_OF_INIT_DLS_ENTRY; i<MAX_NUM_OF_DLS_ENTRY; i++)
    {
        if ((pAd->StaCfg.DLSEntry[i].Valid) && (pAd->StaCfg.DLSEntry[i].Status == DLS_FINISH)
            && (pAd->StaCfg.DLSEntry[i].TimeOut != 0))
        {
            if (pAd->StaCfg.DLSEntry[i].CountDownTimer == 0)
            {
                DBGPRINT(RT_DEBUG_TRACE,("DLS timeout %02x %02x %02x %02x %02x %02x \n",
                    pAd->StaCfg.DLSEntry[i].MacAddr[0],
                    pAd->StaCfg.DLSEntry[i].MacAddr[1],
                    pAd->StaCfg.DLSEntry[i].MacAddr[2],
                    pAd->StaCfg.DLSEntry[i].MacAddr[3],
                    pAd->StaCfg.DLSEntry[i].MacAddr[4],
                    pAd->StaCfg.DLSEntry[i].MacAddr[5]));
                reason = REASON_QOS_REQUEST_TIMEOUT;
                pAd->StaCfg.DLSEntry[i].Valid   = FALSE;
                pAd->StaCfg.DLSEntry[i].Status  = DLS_NONE;
                MlmeCntDlsParmFill(pAd, &MlmeDlsReq, &pAd->StaCfg.DLSEntry[i], reason);
                MlmeEnqueue(pAd,pPort, DLS_STATE_MACHINE, MT2_MLME_DLS_TEAR_DOWN, sizeof(MLME_DLS_REQ_STRUCT), &MlmeDlsReq);
            }
            else
                pAd->StaCfg.DLSEntry[i].CountDownTimer --;
        }
    }
}

/*
    ==========================================================================
    Description:
    
    IRQL = DISPATCH_LEVEL

    ==========================================================================
 */
BOOLEAN RTMPRcvFrameDLSCheck(
    IN PMP_ADAPTER    pAd,
    IN PMP_PORT       pPort,
    IN PHEADER_802_11   pHeader,
    IN ULONG            Len,
    IN PRXINFO_STRUC    pRxInfo)
{
    ULONG           i;
    BOOLEAN         bFindEntry = FALSE;
    BOOLEAN         bSTAKeyFrame = FALSE;
    PEAPOL_PACKET   pEap;
    PUCHAR          pProto, pAddr = NULL;
    PUCHAR          pSTAKey = NULL;
    UCHAR           ZeroReplay[LEN_KEY_DESC_REPLAY];
    UCHAR           Mic[16], OldMic[16];
    UCHAR           digest[80];
    UCHAR           DlsPTK[80];
    UCHAR           temp[64];
    BOOLEAN         TimerCancelled;
    CIPHER_KEY      PairwiseKey;

    if (! DLS_ON(pAd))
        return bSTAKeyFrame;

    if (! INFRA_ON(pPort))
        return bSTAKeyFrame;

    if (pHeader->FC.SubType & 0x08)
    {
        //QoS data
        if (pHeader->FC.Order)
        {
            //QoS control(2), HT control(4), LLC(6)
            pProto = (PUCHAR)pHeader + LENGTH_802_11 + 2 + 4 + 6;
    }
    else
    {
            //QoS control(2), LLC(6)
            pProto  = (PUCHAR)pHeader + LENGTH_802_11 + 2 + 6;
        }
    }
    else
    {
        //Non-QoS, LLC(6)
        pProto  = (PUCHAR)pHeader + LENGTH_802_11 + 6;
    }

    // L2PAD bit on will pad 2 bytes at LLC 
    if (READ_RXINFO_L2PAD(pAd, pRxInfo))
    {
        pProto += 2;    
    }

    //We care only QoS data and EAPOL-Key frame
    if ((!(pHeader->FC.SubType & 0x08)) && (!PlatformEqualMemory(EAPOL, pProto, 2)))
    {
        return bSTAKeyFrame;
    }

    pAddr   = pHeader->Addr2;

    if (PlatformEqualMemory(EAPOL, pProto, 2) 
        && ((pPort->PortCfg.AuthMode >=  Ralink802_11AuthModeWPA) && (pPort->PortCfg.AuthMode !=  Ralink802_11AuthModeAutoSwitch)))
    {
        pEap = (PEAPOL_PACKET) (pProto + 2);
        if (/*(Len >= (LENGTH_802_11 + 6 + 2 + 2 + sizeof(EAPOL_PACKET) - MAX_LEN_OF_RSNIE + 16)) &&*/
            pEap->KeyDesc.KeyInfo.KeyMic && 
            pEap->KeyDesc.KeyInfo.Install && 
            pEap->KeyDesc.KeyInfo.KeyAck && 
            pEap->KeyDesc.KeyInfo.Secure &&
            pEap->KeyDesc.KeyInfo.EKD_DL && 
            pEap->KeyDesc.KeyInfo.KeyType == 0 &&
            !pEap->KeyDesc.KeyInfo.Error && 
            !pEap->KeyDesc.KeyInfo.Request)
        {
            // First validate replay counter, only accept message with larger replay counter
            // Let equal pass, some AP start with all zero replay counter
            PlatformZeroMemory(ZeroReplay, LEN_KEY_DESC_REPLAY);
            if ((MtkCompareMemory(pEap->KeyDesc.ReplayCounter, pAd->StaCfg.ReplayCounter, LEN_KEY_DESC_REPLAY) != 1) &&
                (MtkCompareMemory(pEap->KeyDesc.ReplayCounter, ZeroReplay, LEN_KEY_DESC_REPLAY) != 0))
                return bSTAKeyFrame;

            PlatformMoveMemory(pAd->StaCfg.ReplayCounter, pEap->KeyDesc.ReplayCounter, LEN_KEY_DESC_REPLAY);
            DBGPRINT(RT_DEBUG_TRACE,("DLS - 1 Sniff replay counter (%02x-%02x-%02x-%02x-%02x-%02x-%02x-%02x) Len=%d, KeyDataLen=%d\n",
                pAd->StaCfg.ReplayCounter[0], pAd->StaCfg.ReplayCounter[1], pAd->StaCfg.ReplayCounter[2],
                pAd->StaCfg.ReplayCounter[3], pAd->StaCfg.ReplayCounter[4], pAd->StaCfg.ReplayCounter[5],
                pAd->StaCfg.ReplayCounter[6], pAd->StaCfg.ReplayCounter[7], Len, pEap->KeyDesc.KeyData[1]));

            // put these code segment to get the replay counter
            if (pAd->StaCfg.PortSecured == WPA_802_1X_PORT_NOT_SECURED)
                return bSTAKeyFrame;

            // Check MIC value
            // Save the MIC and replace with zero
            // use proprietary PTK
            PlatformZeroMemory(temp, 64);
            PlatformMoveMemory(temp, "IEEE802.11 WIRELESS ACCESS POINT", 32);
            ApWpaCountPTK(pAd, temp, temp, pPort->PortCfg.Bssid, temp, pAd->HwCfg.CurrentAddress, DlsPTK, LEN_PTK);

            PlatformMoveMemory(OldMic, pEap->KeyDesc.KeyMic, LEN_KEY_DESC_MIC);
            PlatformZeroMemory(pEap->KeyDesc.KeyMic, LEN_KEY_DESC_MIC);
            if (pPort->PortCfg.WepStatus == Ralink802_11Encryption3Enabled)
            {
                // AES
                HMAC_SHA1((PUCHAR) pEap, pEap->Len[1] + 4, DlsPTK, LEN_EAP_MICK, digest);
                PlatformMoveMemory(Mic, digest, LEN_KEY_DESC_MIC);
            }
            else
            {
                hmac_md5(DlsPTK, LEN_EAP_MICK, (PUCHAR) pEap, pEap->Len[1] + 4, Mic);
            }
            
            if (!PlatformEqualMemory(OldMic, Mic, LEN_KEY_DESC_MIC))
            {
                DBGPRINT(RT_DEBUG_ERROR, ("MIC Different in Msg1 of STAKey handshake! \n"));
                return bSTAKeyFrame;
            }
            else
                DBGPRINT(RT_DEBUG_TRACE, ("MIC VALID in Msg1 of STAKey handshake! \n"));
#if 1
            if ((pEap->KeyDesc.KeyData[0] == 0xDD) && (pEap->KeyDesc.KeyData[2] == 0x00) && (pEap->KeyDesc.KeyData[3] == 0x0C)
                && (pEap->KeyDesc.KeyData[4] == 0x43) && (pEap->KeyDesc.KeyData[5] == 0x02))
            {
                pAddr           = pEap->KeyDesc.KeyData + 8;        // Tpe(1), Len(1), OUI(3), DataType(1), Reserved(2)
                pSTAKey         = pEap->KeyDesc.KeyData + 14;   // Tpe(1), Len(1), OUI(3), DataType(1), Reserved(2), STAKey_Mac_Addr(6)

                DBGPRINT(RT_DEBUG_TRACE,("DLS - Receive STAKey Message-1 from %02x:%02x:%02x:%02x:%02x:%02x Len=%d, KeyDataLen=%d\n",
                    pAddr[0], pAddr[1], pAddr[2], pAddr[3], pAddr[4], pAddr[5], Len, pEap->KeyDesc.KeyData[1]));

                bSTAKeyFrame = TRUE;
            }
#else
            if ((pEap->KeyDesc.KeyData[0] == 0xDD) && (pEap->KeyDesc.KeyData[2] == 0x00) && (pEap->KeyDesc.KeyData[3] == 0x0F)
                && (pEap->KeyDesc.KeyData[4] == 0xAC) && (pEap->KeyDesc.KeyData[5] == 0x02))
            {
                pAddr           = pEap->KeyDesc.KeyData + 8;        // Tpe(1), Len(1), OUI(3), DataType(1), Reserved(2)
                pSTAKey         = pEap->KeyDesc.KeyData + 14;   // Tpe(1), Len(1), OUI(3), DataType(1), Reserved(2), STAKey_Mac_Addr(6)

                DBGPRINT(RT_DEBUG_TRACE,("DLS - Receive STAKey Message-1 from %02x:%02x:%02x:%02x:%02x:%02x Len=%d, KeyDataLen=%d\n",
                    pAddr[0], pAddr[1], pAddr[2], pAddr[3], pAddr[4], pAddr[5], Len, pEap->KeyDesc.KeyData[1]));

                bSTAKeyFrame = TRUE;
            }
#endif
            
        }
        else// if (Len >= (LENGTH_802_11 + 6 + 2 + 2 + sizeof(EAPOL_PACKET) - MAX_LEN_OF_RSNIE))
        {
            PlatformMoveMemory(pAd->StaCfg.ReplayCounter, pEap->KeyDesc.ReplayCounter, LEN_KEY_DESC_REPLAY);
            DBGPRINT(RT_DEBUG_TRACE,("DLS - 2 Sniff replay counter 2(%02x-%02x-%02x-%02x-%02x-%02x-%02x-%02x) Len=%d, KeyDataLen=%d\n",
                pAd->StaCfg.ReplayCounter[0], pAd->StaCfg.ReplayCounter[1], pAd->StaCfg.ReplayCounter[2],
                pAd->StaCfg.ReplayCounter[3], pAd->StaCfg.ReplayCounter[4], pAd->StaCfg.ReplayCounter[5],
                pAd->StaCfg.ReplayCounter[6], pAd->StaCfg.ReplayCounter[7], Len, pEap->KeyDesc.KeyData[1]));
        }
    }

    // If timeout value is equaled to zero, it means always not be timeout.
    // update local dls table entry
    for (i = 0; i < MAX_NUM_OF_INIT_DLS_ENTRY; i++)
    {
        if (pAd->StaCfg.DLSEntry[i].Valid && MAC_ADDR_EQUAL(pAddr, pAd->StaCfg.DLSEntry[i].MacAddr))
        {
            if (bSTAKeyFrame)
            {
                // STAKey frame, add pairwise key table
                pAd->StaCfg.DLSEntry[i].Status = DLS_FINISH;
                PlatformCancelTimer(&pPort->StaCfg.DLSEntry[i].Timer, &TimerCancelled);
                
                PairwiseKey.KeyLen = LEN_TKIP_EK;
                PlatformMoveMemory(PairwiseKey.Key, &pSTAKey[0], LEN_TKIP_EK);
                PlatformMoveMemory(PairwiseKey.TxMic, &pSTAKey[16], LEN_TKIP_RXMICK);
                PlatformMoveMemory(PairwiseKey.RxMic, &pSTAKey[24], LEN_TKIP_TXMICK);

                PairwiseKey.CipherAlg = pPort->SharedKey[BSS0][pPort->PortCfg.DefaultKeyId].CipherAlg;
                {
                    RT_SET_DLS_WCID_KEY  DLS_Cipher;
                    DLS_Cipher.WCID = pAd->StaCfg.DLSEntry[i].Wcid;
                    PlatformMoveMemory(&DLS_Cipher.CipherKey,  &PairwiseKey,sizeof(CIPHER_KEY));
                    //AsicAddKeyEntry(pAd, (USHORT)(i + 2), BSS0, 0, &PairwiseKey, TRUE, TRUE); // reserve 0 for multicast, 1 for unicast
                    MTEnqueueInternalCmd(pAd, pPort,  MT_CMD_SET_DLS_KEY_TABLE, &DLS_Cipher, sizeof(RT_SET_DLS_WCID_KEY));
                }
            
                //{
                //  RT_SET_ASIC_WCID    SetAsicWcid;
                //  // Another adhoc joining, add to our MAC table. 
                //  SetAsicWcid.WCID = i;                   
                //  PlatformMoveMemory(SetAsicWcid.Addr,pAddr,6);
                //  MTEnqueueInternalCmd(pAd, pPort, MT_CMD_SET_DLS_RX_WCID_TABLE, &SetAsicWcid, sizeof(RT_SET_ASIC_WCID));
                //}
                
                //MtAsicUpdateRxWCIDTable(pAd, (USHORT)(i + 2), pAddr);

                DBGPRINT(RT_DEBUG_TRACE,("DLS - Receive STAKey Message-1 (Peer STA MAC Address STAKey) \n"));

                RTMPSendSTAKeyHandShake(pAd, pPort, pAd->StaCfg.DLSEntry[i].MacAddr);
                
                DBGPRINT(RT_DEBUG_TRACE,("DLS - Finish STAKey handshake procedure (Initiator side)\n"));
            }
            else
            {
                // Data frame, update timeout value
                if (pAd->StaCfg.DLSEntry[i].Status == DLS_FINISH)
                {
                    pAd->StaCfg.DLSEntry[i].CountDownTimer = pAd->StaCfg.DLSEntry[i].TimeOut;
                }
            }

            bFindEntry = TRUE;
        }
    }

    // update peer dls table entry
    for (i = MAX_NUM_OF_INIT_DLS_ENTRY; i < MAX_NUM_OF_DLS_ENTRY; i++)
    {
        if (pAd->StaCfg.DLSEntry[i].Valid && MAC_ADDR_EQUAL(pAddr, pAd->StaCfg.DLSEntry[i].MacAddr))
        {
            if (bSTAKeyFrame)
            {
                // STAKey frame, add pairwise key table, and send STAkey Msg-2
                pAd->StaCfg.DLSEntry[i].Status = DLS_FINISH;
                PlatformCancelTimer(&pPort->StaCfg.DLSEntry[i].Timer, &TimerCancelled);

                PairwiseKey.KeyLen = LEN_TKIP_EK;
                PlatformMoveMemory(PairwiseKey.Key, &pSTAKey[0], LEN_TKIP_EK);
                PlatformMoveMemory(PairwiseKey.TxMic, &pSTAKey[16], LEN_TKIP_RXMICK);
                PlatformMoveMemory(PairwiseKey.RxMic, &pSTAKey[24], LEN_TKIP_TXMICK);

                PairwiseKey.CipherAlg = pPort->SharedKey[BSS0][pPort->PortCfg.DefaultKeyId].CipherAlg;

                {
                    RT_SET_DLS_WCID_KEY  DLS_Cipher;
                    DLS_Cipher.WCID = pAd->StaCfg.DLSEntry[i].Wcid;         
                    PlatformMoveMemory(&DLS_Cipher.CipherKey,  &PairwiseKey,sizeof(CIPHER_KEY));                    
                    //AsicAddKeyEntry(pAd, (USHORT)(i + 2), BSS0, 0, &PairwiseKey, TRUE, TRUE); // reserve 0 for multicast, 1 for unicast
                    MTEnqueueInternalCmd(pAd, pPort,  MT_CMD_SET_DLS_KEY_TABLE, &DLS_Cipher, sizeof(RT_SET_DLS_WCID_KEY));
                }

                //{
                //  RT_SET_ASIC_WCID    SetAsicWcid;
                //  // Another adhoc joining, add to our MAC table. 
                //  SetAsicWcid.WCID = i;                   
                //  PlatformMoveMemory(SetAsicWcid.Addr,pAddr,6);
                //  MTEnqueueInternalCmd(pAd, pPort, MT_CMD_SET_DLS_RX_WCID_TABLE, &SetAsicWcid, sizeof(RT_SET_ASIC_WCID));
                //}
                //MtAsicUpdateRxWCIDTable(pAd, (USHORT)(i + 2), pAddr);
                
                DBGPRINT(RT_DEBUG_TRACE,("DLS - Receive STAKey Message-1 (Initiator STA MAC Address STAKey)\n"));

                // If support WPA or WPA2, start STAKey hand shake,
                // If failed hand shake, just tear down peer DLS
                if (RTMPSendSTAKeyHandShake(pAd, pPort, pAddr) != NDIS_STATUS_SUCCESS)
                {
                    MLME_DLS_REQ_STRUCT MlmeDlsReq;
                    USHORT              reason = REASON_QOS_CIPHER_NOT_SUPPORT;

                    pAd->StaCfg.DLSEntry[i].Valid   = FALSE;
                    pAd->StaCfg.DLSEntry[i].Status  = DLS_NONE;
                    MlmeCntDlsParmFill(pAd, &MlmeDlsReq, &pAd->StaCfg.DLSEntry[i], reason);
                    MlmeEnqueue(pAd,pPort, DLS_STATE_MACHINE, MT2_MLME_DLS_TEAR_DOWN, sizeof(MLME_DLS_REQ_STRUCT), &MlmeDlsReq);
                }
                else
                {
                    DBGPRINT(RT_DEBUG_TRACE,("DLS - Finish STAKey handshake procedure (Peer side)\n"));
                }
            }
            else
            {
                // Data frame, update timeout value
                if (pAd->StaCfg.DLSEntry[i].Status == DLS_FINISH)
                {
                    pAd->StaCfg.DLSEntry[i].CountDownTimer = pAd->StaCfg.DLSEntry[i].TimeOut;
                }
            }
            
            bFindEntry = TRUE;
        }
    }

    return bSTAKeyFrame;
}

/*
    ========================================================================

    Routine Description:
        Check if the frame can be sent through DLS direct link interface
        
    Arguments:
        pAd     Pointer to adapter
        
    Return Value:
        DLS entry index
        
    Note:
    
    ========================================================================
*/
int RTMPCheckDLSFrame(
    IN  PMP_ADAPTER   pAd,
    IN  PMP_PORT  pPort,
    IN  PUCHAR          pDA)
{
    int rval = -1;
    int i;

    if (!DLS_ON(pAd))
        return rval;

    if (pPort->PortSubtype != PORTSUBTYPE_STA)
        return rval;

    if (!INFRA_ON(pPort))
        return rval;

    do{
        // check local dls table entry
        for (i = 0; i < MAX_NUM_OF_INIT_DLS_ENTRY; i++)
        {
            if (pAd->StaCfg.DLSEntry[i].Valid && (pAd->StaCfg.DLSEntry[i].Status == DLS_FINISH) &&
                MAC_ADDR_EQUAL(pDA, pAd->StaCfg.DLSEntry[i].MacAddr))
            {
                rval = i;
                break;
            }
        }

        // check peer dls table entry
        for (i = MAX_NUM_OF_INIT_DLS_ENTRY; i < MAX_NUM_OF_DLS_ENTRY; i++)
        {
            if (pAd->StaCfg.DLSEntry[i].Valid && (pAd->StaCfg.DLSEntry[i].Status == DLS_FINISH) &&
                MAC_ADDR_EQUAL(pDA, pAd->StaCfg.DLSEntry[i].MacAddr))
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
VOID RTMPSendDLSTearDownFrame(
    IN  PMP_ADAPTER   pAd,
    IN  PUCHAR          pDA)
{
    PUCHAR          pOutBuffer = NULL;
    NDIS_STATUS     NStatus;
    HEADER_802_11   DlsTearDownHdr;
    ULONG           FrameLen = 0;
    USHORT          Reason = REASON_QOS_QSTA_LEAVING_QBSS;
    UCHAR           Category = CATEGORY_DLS;
    UCHAR           Action = ACTION_DLS_TEARDOWN;
    UCHAR           i = 0;
    PMP_PORT      pPort = pAd->PortList[PORT_0];
    if (MT_TEST_FLAG(pAd, fRTMP_ADAPTER_RESET_IN_PROGRESS) ||
        MT_TEST_FLAG(pAd, fRTMP_ADAPTER_HALT_IN_PROGRESS))
        return;

    DBGPRINT(RT_DEBUG_TRACE, ("Send DLS TearDown Frame \n"));

    NStatus = MlmeAllocateMemory(pAd, &pOutBuffer);  //Get an unused nonpaged memory
    if (NStatus != NDIS_STATUS_SUCCESS) 
    {
        DBGPRINT(RT_DEBUG_ERROR,("ASSOC - RTMPSendDLSTearDownFrame() allocate memory failed \n"));
        return;
    }

    ActHeaderInit(pAd, pPort, &DlsTearDownHdr, pPort->PortCfg.Bssid, pPort->PortCfg.Bssid);
    MakeOutgoingFrame(pOutBuffer,               &FrameLen, 
                    sizeof(HEADER_802_11),      &DlsTearDownHdr, 
                    1,                          &Category,
                    1,                          &Action,
                    6,                          pDA,
                    6,                          pPort->CurrentAddress,
                    2,                          &Reason,
                    END_OF_ARGS);

    NdisCommonMiniportMMRequest(pAd,pOutBuffer, FrameLen);    
}

/*
    ==========================================================================
    Description:
    
    IRQL = DISPATCH_LEVEL

    ==========================================================================
 */
NDIS_STATUS RTMPSendSTAKeyRequest(
    IN  PMP_ADAPTER   pAd,
    IN  PMP_PORT      pPort,
    IN  PUCHAR          pDA)
{
    HEADER_802_11       Header802_11;
    NDIS_STATUS         NStatus;
    ULONG               FrameLen = 0;
    EAPOL_PACKET        Packet;
    UCHAR               Mic[16];
    UCHAR               digest[80];
    PUCHAR              pOutBuffer = NULL;
    UCHAR               temp[64];
    UCHAR               DlsPTK[80];
    BOOLEAN             IsLastNetBuffer = FALSE;
    PMT_XMIT_CTRL_UNIT    pXcu = NULL;


    DBGPRINT(RT_DEBUG_TRACE,("DLS - RTMPSendSTAKeyRequest() to %02x:%02x:%02x:%02x:%02x:%02x\n", pDA[0], pDA[1], pDA[2], pDA[3], pDA[4], pDA[5]));
    
    pAd->pTxCfg->Sequence ++;

    // Make 802.11 header
    PlatformZeroMemory(&Header802_11, sizeof(Header802_11));
    Header802_11.FC.FrDs = 0;
    Header802_11.FC.ToDs = 1;
    Header802_11.FC.Type = DOT11_FRAME_TYPE_DATA;
    Header802_11.FC.SubType = DOT11_DATA_SUBTYPE_DATA;
    Header802_11.FC.MoreData = 0; //TODO
    Header802_11.FC.MoreFrag = 0; // TODO
    Header802_11.FC.PwrMgmt = 0; // TODO
    Header802_11.FC.Retry = 0; // TODO
    COPY_MAC_ADDR(Header802_11.Addr1, pPort->PortCfg.Bssid); // BSSID
    COPY_MAC_ADDR(Header802_11.Addr2, pAd->HwCfg.CurrentAddress); // SA
    COPY_MAC_ADDR(Header802_11.Addr3, pPort->PortCfg.Bssid); // DA

    // Zero message body
    PlatformZeroMemory(&Packet, sizeof(Packet));
    Packet.Version = EAPOL_VER;
    Packet.Type    = EAPOLKey;
    Packet.Len[1]  = sizeof(KEY_DESCRIPTER) - MAX_LEN_OF_RSNIE + 6 + MAC_ADDR_LEN;      // data field contain KDE andPeer MAC address
    
    // STAKey Message is as EAPOL-Key(1,1,0,0,G/0,0,0, MIC, 0,Peer MAC KDE)
    if ((pPort->PortCfg.AuthMode == Ralink802_11AuthModeWPA) || (pPort->PortCfg.AuthMode == Ralink802_11AuthModeWPAPSK))
    {
        Packet.KeyDesc.Type = WPA1_KEY_DESC;
    }
    else if ((pPort->PortCfg.AuthMode == Ralink802_11AuthModeWPA2) || (pPort->PortCfg.AuthMode == Ralink802_11AuthModeWPA2PSK))
    {
        Packet.KeyDesc.Type = WPA2_KEY_DESC;
    }
    
    // Key descriptor version
    Packet.KeyDesc.KeyInfo.KeyDescVer = 
        (((pPort->PortCfg.PairCipher == Ralink802_11Encryption3Enabled) || (pPort->PortCfg.GroupCipher == Ralink802_11Encryption3Enabled)) ? (DESC_TYPE_AES) : (DESC_TYPE_TKIP));

    Packet.KeyDesc.KeyInfo.KeyMic   = 1;
    Packet.KeyDesc.KeyInfo.Secure   = 1;
    Packet.KeyDesc.KeyInfo.Request  = 1;

    Packet.KeyDesc.KeyDataLen[1]    = 12;

    // use our own OUI to distinguish proprietary with standard.
    Packet.KeyDesc.KeyData[0]       = 0xDD;
    Packet.KeyDesc.KeyData[1]       = 0x0A;
    Packet.KeyDesc.KeyData[2]       = 0x00;
    Packet.KeyDesc.KeyData[3]       = 0x0C;
    Packet.KeyDesc.KeyData[4]       = 0x43;
    Packet.KeyDesc.KeyData[5]       = 0x03;
    PlatformMoveMemory(&Packet.KeyDesc.KeyData[6], pDA, MAC_ADDR_LEN);
    
    DBGPRINT(RT_DEBUG_TRACE,("DLS - replay counter (%02x-%02x-%02x-%02x-%02x-%02x-%02x-%02x)\n",
        pAd->StaCfg.ReplayCounter[0], pAd->StaCfg.ReplayCounter[1], pAd->StaCfg.ReplayCounter[2],
        pAd->StaCfg.ReplayCounter[3], pAd->StaCfg.ReplayCounter[4], pAd->StaCfg.ReplayCounter[5],
        pAd->StaCfg.ReplayCounter[6], pAd->StaCfg.ReplayCounter[7]));
    PlatformMoveMemory(Packet.KeyDesc.ReplayCounter, pAd->StaCfg.ReplayCounter, LEN_KEY_DESC_REPLAY);

    // Allocate buffer for transmitting message
    NStatus = MlmeAllocateMemory(pAd, &pOutBuffer);
    if (NStatus != NDIS_STATUS_SUCCESS) 
        return NStatus;

    // Prepare EAPOL frame for MIC calculation
    // Be careful, only EAPOL frame is counted for MIC calculation
    MakeOutgoingFrame(pOutBuffer,           &FrameLen,
                        Packet.Len[1] + 4,    &Packet,
                        END_OF_ARGS);

    // use proprietary PTK
    PlatformZeroMemory(temp, 64);
    PlatformMoveMemory(temp, "IEEE802.11 WIRELESS ACCESS POINT", 32);
    ApWpaCountPTK(pAd, temp, temp, pPort->PortCfg.Bssid, temp, pAd->HwCfg.CurrentAddress, DlsPTK, LEN_PTK);

    // calculate MIC
    if (pPort->PortCfg.WepStatus == Ralink802_11Encryption3Enabled)
    {
        // AES
        PlatformZeroMemory(digest,  sizeof(digest));
        HMAC_SHA1(pOutBuffer, FrameLen, DlsPTK, LEN_EAP_MICK, digest);
        PlatformMoveMemory(Packet.KeyDesc.KeyMic, digest, LEN_KEY_DESC_MIC);
    }
    else
    {
        PlatformZeroMemory(Mic, sizeof(Mic));
        hmac_md5(DlsPTK, LEN_EAP_MICK, pOutBuffer, FrameLen, Mic);
        PlatformMoveMemory(Packet.KeyDesc.KeyMic, Mic, LEN_KEY_DESC_MIC);
    }

    MakeOutgoingFrame(pOutBuffer,               &FrameLen,  
                        sizeof(Header802_11),       &Header802_11,
                        8,                      EAPOL_LLC_SNAP,
                        Packet.Len[1] + 4,      &Packet,
                        END_OF_ARGS);

    //NStatus = RTMPAllocateNdisPacket(pAd, &pNdisPacket, NULL, 0, pOutBuffer, FrameLen);

    // build net buffer
    pXcu = Ndis6CommonAllocateNdisNetBufferList(pAd, pPort->PortNumber, (PUCHAR)(pOutBuffer), FrameLen);
    
    if (pXcu != NULL)
    {
        pXcu->PktSource = PKTSRC_INTERNAL;
        XmitStaSendPacket(pAd, pPort->PortNumber, pXcu,  FALSE);
        NdisSetEvent(&(GET_THREAD_EVENT(&pAd->pHifCfg->hDequeueDataThread)));
    }

    // Kick bulk out
    N6USBKickBulkOut(pAd);

    //RELEASE_NDIS_PACKET(pAd, pNdisPacket, NDIS_STATUS_SUCCESS);
    MlmeFreeMemory(pAd, pOutBuffer);
    
    DBGPRINT(RT_DEBUG_TRACE, ("RTMPSendSTAKeyRequest- Send STAKey request (NStatus=%x, FrameLen=%d)\n", NStatus, FrameLen));

    return NStatus;
}

/*
    ==========================================================================
    Description:
    
    IRQL = DISPATCH_LEVEL

    ==========================================================================
 */
NDIS_STATUS RTMPSendSTAKeyHandShake(
    IN  PMP_ADAPTER   pAd,
    IN  PMP_PORT      pPort,
    IN  PUCHAR          pDA)
{
    HEADER_802_11       Header802_11;
    NDIS_STATUS         NStatus;
    ULONG               FrameLen = 0;
    EAPOL_PACKET        Packet;
    UCHAR               Mic[16];
    UCHAR               digest[80];
    PUCHAR              pOutBuffer = NULL;
    UCHAR               temp[64];
    UCHAR               DlsPTK[80];         // Due to dirver can not get PTK, use proprietary PTK
    BOOLEAN             IsLastNetBuffer = FALSE;
    PMT_XMIT_CTRL_UNIT    pXcu = NULL;

    DBGPRINT(RT_DEBUG_TRACE,("DLS - RTMPSendSTAKeyHandShake() to %02x:%02x:%02x:%02x:%02x:%02x\n", pDA[0], pDA[1], pDA[2], pDA[3], pDA[4], pDA[5]));
    
    pAd->pTxCfg->Sequence ++;

    // Make 802.11 header
    PlatformZeroMemory(&Header802_11, sizeof(Header802_11));
    Header802_11.FC.FrDs = 0;
    Header802_11.FC.ToDs = 1;
    Header802_11.FC.Type = DOT11_FRAME_TYPE_DATA;
    Header802_11.FC.SubType = DOT11_DATA_SUBTYPE_DATA;
    Header802_11.FC.MoreData = 0; //TODO
    Header802_11.FC.MoreFrag = 0; // TODO
    Header802_11.FC.PwrMgmt = 0; // TODO
    Header802_11.FC.Retry = 0; // TODO
    COPY_MAC_ADDR(Header802_11.Addr1, pPort->PortCfg.Bssid); // BSSID
    COPY_MAC_ADDR(Header802_11.Addr2, pAd->HwCfg.CurrentAddress); // SA
    COPY_MAC_ADDR(Header802_11.Addr3, pPort->PortCfg.Bssid); // DA

    // Zero message body
    PlatformZeroMemory(&Packet, sizeof(Packet));
    Packet.Version = EAPOL_VER;
    Packet.Type    = EAPOLKey;
    Packet.Len[1]  = sizeof(KEY_DESCRIPTER) - MAX_LEN_OF_RSNIE + 6 + MAC_ADDR_LEN;      // data field contain KDE and Peer MAC address
    
    // STAKey Message is as EAPOL-Key(1,1,0,0,G/0,0,0, MIC, 0,Peer MAC KDE)
    if ((pPort->PortCfg.AuthMode == Ralink802_11AuthModeWPA) || (pPort->PortCfg.AuthMode == Ralink802_11AuthModeWPAPSK))
    {
        Packet.KeyDesc.Type = WPA1_KEY_DESC;
    }
    else if ((pPort->PortCfg.AuthMode == Ralink802_11AuthModeWPA2) || (pPort->PortCfg.AuthMode == Ralink802_11AuthModeWPA2PSK))
    {
        Packet.KeyDesc.Type = WPA2_KEY_DESC;
    }
    
    // Key descriptor version
    Packet.KeyDesc.KeyInfo.KeyDescVer = 
        (((pPort->PortCfg.PairCipher == Ralink802_11Encryption3Enabled) || (pPort->PortCfg.GroupCipher == Ralink802_11Encryption3Enabled)) ? (DESC_TYPE_AES) : (DESC_TYPE_TKIP));

    Packet.KeyDesc.KeyInfo.KeyMic   = 1;
    Packet.KeyDesc.KeyInfo.Secure   = 1;

    Packet.KeyDesc.KeyDataLen[1]    = 12;

    // use our own OUI to distinguish proprietary with standard.
    Packet.KeyDesc.KeyData[0]       = 0xDD;
    Packet.KeyDesc.KeyData[1]       = 0x0A;
    Packet.KeyDesc.KeyData[2]       = 0x00;
    Packet.KeyDesc.KeyData[3]       = 0x0C;
    Packet.KeyDesc.KeyData[4]       = 0x43;
    Packet.KeyDesc.KeyData[5]       = 0x03;
    PlatformMoveMemory(&Packet.KeyDesc.KeyData[6], pDA, MAC_ADDR_LEN);
    
    DBGPRINT(RT_DEBUG_TRACE,("DLS - replay counter (%02x-%02x-%02x-%02x-%02x-%02x-%02x-%02x)\n",
        pAd->StaCfg.ReplayCounter[0], pAd->StaCfg.ReplayCounter[1], pAd->StaCfg.ReplayCounter[2],
        pAd->StaCfg.ReplayCounter[3], pAd->StaCfg.ReplayCounter[4], pAd->StaCfg.ReplayCounter[5],
        pAd->StaCfg.ReplayCounter[6], pAd->StaCfg.ReplayCounter[7]));
    PlatformMoveMemory(Packet.KeyDesc.ReplayCounter, pAd->StaCfg.ReplayCounter, LEN_KEY_DESC_REPLAY);

    // Allocate buffer for transmitting message
    NStatus = MlmeAllocateMemory(pAd, &pOutBuffer);
    if (NStatus != NDIS_STATUS_SUCCESS) 
        return NStatus;

    // Prepare EAPOL frame for MIC calculation
    // Be careful, only EAPOL frame is counted for MIC calculation
    MakeOutgoingFrame(pOutBuffer,           &FrameLen,
                        Packet.Len[1] + 4,    &Packet,
                        END_OF_ARGS);

    // use proprietary PTK
    PlatformZeroMemory(temp, 64);
    PlatformMoveMemory(temp, "IEEE802.11 WIRELESS ACCESS POINT", 32);
    ApWpaCountPTK(pAd, temp, temp, pPort->PortCfg.Bssid, temp, pAd->HwCfg.CurrentAddress, DlsPTK, LEN_PTK);

    // calculate MIC
    if (pPort->PortCfg.WepStatus == Ralink802_11Encryption3Enabled)
    {
        // AES
        PlatformZeroMemory(digest,  sizeof(digest));
        HMAC_SHA1(pOutBuffer, FrameLen, DlsPTK, LEN_EAP_MICK, digest);
        PlatformMoveMemory(Packet.KeyDesc.KeyMic, digest, LEN_KEY_DESC_MIC);
    }
    else
    {
        PlatformZeroMemory(Mic, sizeof(Mic));
        hmac_md5(DlsPTK, LEN_EAP_MICK, pOutBuffer, FrameLen, Mic);
        PlatformMoveMemory(Packet.KeyDesc.KeyMic, Mic, LEN_KEY_DESC_MIC);
    }

    MakeOutgoingFrame(pOutBuffer,               &FrameLen,  
                        sizeof(Header802_11),       &Header802_11,
                        8,                      EAPOL_LLC_SNAP,
                        Packet.Len[1] + 4,      &Packet,
                        END_OF_ARGS);

    pXcu  = Ndis6CommonAllocateNdisNetBufferList(pAd, pPort->PortNumber, (PUCHAR)(pOutBuffer), FrameLen);
    if (pXcu != NULL)
    {
        pXcu->PktSource = PKTSRC_INTERNAL;
        XmitStaSendPacket(pAd, pPort->PortNumber, pXcu,  FALSE);
        NdisSetEvent(&(GET_THREAD_EVENT(&pAd->pHifCfg->hDequeueDataThread)));
    }

    // Kick bulk out
    N6USBKickBulkOut(pAd);
    
    MlmeFreeMemory(pAd, pOutBuffer);
    
    DBGPRINT(RT_DEBUG_TRACE, ("RTMPSendSTAKeyHandShake- Send STAKey Message-2 (NStatus=%x, FrameLen=%d)\n", NStatus, FrameLen));

    return NStatus;
}

VOID DlsTimeoutTimerCallback(
    IN PVOID SystemSpecific1, 
    IN PVOID FunctionContext, 
    IN PVOID SystemSpecific2, 
    IN PVOID SystemSpecific3) 
{
    PMP_PORT pPort = (PMP_PORT )FunctionContext;
    PMP_ADAPTER  pAd = pPort->pAd;
/*    
    USHORT                  reason;
    MLME_DLS_REQ_STRUCT     MlmeDlsReq;

    if ((pDLS) && (pDLS->Valid))
    {
        reason          = REASON_QOS_REQUEST_TIMEOUT;
        pDLS->Valid     = FALSE;
        pDLS->Status    = DLS_NONE;
        MlmeCntDlsParmFill(pAd, &MlmeDlsReq, pDLS, reason);
        MlmeEnqueue(pAd,pPort, DLS_STATE_MACHINE, MT2_MLME_DLS_TEAR_DOWN, sizeof(MLME_DLS_REQ_STRUCT), &MlmeDlsReq);
        // call Mlme handler to execute it
    }
*/    
}

VOID DlsTearDown(
    IN PMP_ADAPTER pAd,
    IN  PMP_PORT   pPort)
{
    int i;

    // tear down local dls table entry
    for (i = 0; i < MAX_NUM_OF_INIT_DLS_ENTRY; i++)
    {
        if (pAd->StaCfg.DLSEntry[i].Valid && (pAd->StaCfg.DLSEntry[i].Status == DLS_FINISH))
        {
            pAd->StaCfg.DLSEntry[i].Status = DLS_NONE;
            pAd->StaCfg.DLSEntry[i].Valid = FALSE;
            MtAsicDelWcidTab(pAd, pAd->StaCfg.DLSEntry[i].Wcid);
#ifdef COMPOS_WIN
		// TODO: Shiang-usw-win, add here for compile pass!
		AsicRemoveKeyEntry(pAd, pAd->StaCfg.DLSEntry[i].Wcid, 0, TRUE);
#else
            AsicRemoveKeyEntry(pAd, pAd->StaCfg.DLSEntry[i].Wcid);
#endif
            DlsDeleteMacTableEntry(pAd, pPort, pAd->StaCfg.DLSEntry[i].MacAddr);
            RTMPSendDLSTearDownFrame(pAd, pAd->StaCfg.DLSEntry[i].MacAddr);
        }
    }

    // tear down peer dls table entry
    for (i = MAX_NUM_OF_INIT_DLS_ENTRY; i < MAX_NUM_OF_DLS_ENTRY; i++)
    {
        if (pAd->StaCfg.DLSEntry[i].Valid && (pAd->StaCfg.DLSEntry[i].Status ==  DLS_FINISH))
        {
            pAd->StaCfg.DLSEntry[i].Status = DLS_NONE;
            pAd->StaCfg.DLSEntry[i].Valid = FALSE;
            MtAsicDelWcidTab(pAd, pAd->StaCfg.DLSEntry[i].Wcid);
		// TODO: Shiang-usw-win, add here for compile pass!					
            AsicRemoveKeyEntry(pAd, pAd->StaCfg.DLSEntry[i].Wcid, 0, TRUE);
            DlsDeleteMacTableEntry(pAd, pPort, pAd->StaCfg.DLSEntry[i].MacAddr);
            RTMPSendDLSTearDownFrame(pAd, pAd->StaCfg.DLSEntry[i].MacAddr);
        }
    }
}

VOID DlsDeleteMacTableEntry(
    IN PMP_ADAPTER pAd,
    IN PMP_PORT    pPort,
    IN PUCHAR pMacAddr)
{
    PMAC_TABLE_ENTRY pMacEntry = MacTableLookup(pAd,  pPort, pMacAddr);
    if (pMacEntry)
    {
        MacTableDeleteAndResetEntry(pAd, pPort, pMacEntry->Aid, pMacAddr, TRUE);
    }
}

