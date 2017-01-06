#include    "MtConfig.h"

/*
    ========================================================================

    Routine Description:
        Check hardware status if it's ready to send out the packets.

    Arguments:
        pAd         -   pointer to the Adatper context
        pPort       -   pointer to the Port structure 

    Return Value:
        NDIS_STATUS_SUCCESS - The hardware is ready to send packets.
        Others              - The hardware is not ready to send packets.

    IRQL <= DISPATCH_LEVEL

    Note:

    ========================================================================
*/
NDIS_STATUS 
N6XmitCheckHWReadyToSend(
    IN PMP_ADAPTER        pAd,
    IN PMP_PORT           pPort)
{
    NDIS_STATUS         ndisStatus = NDIS_STATUS_SUCCESS;
    UCHAR               OpMode = OPMODE_STA;

    DBGPRINT(RT_DEBUG_TRACE, ("==> N6XmitCheckHWReadyToSend\n")); 
    do
    {
        if (pPort == NULL)
        {
            ndisStatus = NDIS_STATUS_INVALID_PORT;
            DBGPRINT(RT_DEBUG_ERROR, ("N6XmitCheckHWReadyToSend:: Unable to find Port corresponding to Port\n"));
            break;
        }
    
#if (COMPILE_WIN7_ABOVE(CURRENT_OS_NDIS_VER))
        if(NDIS_WIN8_ABOVE(pAd))
        {
            // For supporting "No Pause On Suspend", if power state is D3, stop sending NetBuffer by return "NDIS_STATUS_LOW_POWER_STATE"
            if (pAd->NextDevicePowerState == NdisDeviceStateD3)
            {
                ndisStatus = NDIS_STATUS_LOW_POWER_STATE;
                DBGPRINT(RT_DEBUG_TRACE, ("Stop send data to TxQueue, it is low power now!!\n"));
                break;
            }
        }
#endif
    
        if (MT_TEST_FLAG(pAd, fRTMP_ADAPTER_SURPRISE_REMOVED) ||
            MT_TEST_FLAG(pAd, fRTMP_ADAPTER_NIC_NOT_EXIST))
        {
            ndisStatus = NDIS_STATUS_ADAPTER_REMOVED;
            break;
        }
        
        if (pAd->AdapterState != NicRunning)
        {
            ndisStatus = NDIS_STATUS_PAUSED;
            DBGPRINT(RT_DEBUG_TRACE, ("N6XmitCheckHWReadyToSend:: Send failed, Adapter is not in a running state\n"));
            break;
        }

        if (MT_TEST_FLAG(pAd, fRTMP_ADAPTER_HALT_IN_PROGRESS))
        {
            ndisStatus = NDIS_STATUS_CLOSING;
            DBGPRINT(RT_DEBUG_ERROR, ("N6XmitCheckHWReadyToSend:: Send failed, Adapter is in Halt state\n"));
            break;
        }
        
        if (MT_TEST_FLAG(pAd, fRTMP_ADAPTER_RESET_IN_PROGRESS) ||
            MT_TEST_FLAG(pAd, fRTMP_ADAPTER_RADIO_OFF)
            )
        {           
            // Drop send request since hardware is in reset state
            ndisStatus = NDIS_STATUS_FAILURE;
            DBGPRINT(RT_DEBUG_TRACE, ("N6XmitCheckHWReadyToSend:: Send failed, Adapter is not Ready, pAd->Flags=0x%08x\n", pAd->Flags));
            break;
        }

        if (pPort->PortSubtype == PORTSUBTYPE_SoftapAP || pPort->PortSubtype == PORTSUBTYPE_VwifiAP || (pPort->PortSubtype == PORTSUBTYPE_P2PGO))
            OpMode = OPMODE_AP;

        if (IDLE_ON(pPort) && (OpMode != OPMODE_AP) && !IS_P2P_OP(pPort))
        {
            // Drop send request since there are no physical connection yet
            // Check the association status for infrastructure mode
            // And Mibss for Ad-hoc mode setup
            ndisStatus = NDIS_STATUS_FAILURE;
            DBGPRINT(RT_DEBUG_TRACE, ("N6XmitCheckHWReadyToSend:: Send failed, IDLE_ON(pPort)=%d, OpMode=%d, !IS_P2P_OP(pPort)=%d\n", IDLE_ON(pPort), OpMode, !IS_P2P_OP(pPort)));
            break;
        }
    } while (FALSE);

    DBGPRINT(RT_DEBUG_TRACE, ("<== N6XmitCheckHWReadyToSend, (ndisStatus=0x%08x)\n", ndisStatus)); 
    return ndisStatus;
}

NDIS_STATUS
N6XmitGetTxbufToXcu(
    IN PMP_ADAPTER              pAd,
    IN  NDIS_PORT_NUMBER    PortNumber,
    IN  PNET_BUFFER_LIST    NetBufferList,
    IN  PNET_BUFFER         NetBuffer,
    IN  PMT_XMIT_CTRL_UNIT               pXcu
    )
{
    UINT    MdlNum =0, TotalPktLength = 0, MdlByteCount = 0, MdlOffset = 0, FirstMdlOffset = 0, i = 0, bufferCnt =0, MdlLength = 0;
    UINT    FirstMdlLength =0, RemindLength = 0;
    PMDL    pCurrentMdl = NULL,  pFirstMdl = NULL, pNowMdl = NULL;
    PDOT11_EXTSTA_SEND_CONTEXT  pDot11SendContext = NULL;
    NDIS_STATUS         ndisStatus = NDIS_STATUS_SUCCESS;
    NDIS_NET_BUFFER_LIST_8021Q_INFO     Ndis8021QInfo;
    PUCHAR Ptr = NULL;
    PMT_XMIT_CTRL_UNIT XcuNextPtr = NULL;
    PMP_PORT pPort = pAd->PortList[PortNumber];
    ULONG xcuIdx = 0;
    
    if(pXcu== NULL)
    {
        ndisStatus = NDIS_STATUS_FAILURE;
        DBGPRINT(RT_DEBUG_ERROR, ("%s(), Line:%d, pXcu== NULL !!!\n", __FUNCTION__, __LINE__));
        return ndisStatus;
    }
    XcuNextPtr = pXcu->Next;
    xcuIdx = pXcu->CurrentXcuidx;
    PlatformZeroMemory(pXcu, sizeof(MT_XMIT_CTRL_UNIT));
    pXcu->Next = XcuNextPtr;
    pXcu->CurrentXcuidx = xcuIdx; 
    pXcu->Reserve1 = NULL;
    pXcu->Reserve2 = NULL;
    pXcu->NumOfBuf = 0;
    
    pXcu->BufferList[0].VirtualAddr = (PUCHAR)&pXcu->HeaderBuffer[0];

    pFirstMdl = NET_BUFFER_FIRST_MDL(NetBuffer);
    FirstMdlOffset = NET_BUFFER_CURRENT_MDL_OFFSET(NetBuffer);
    TotalPktLength  = NET_BUFFER_DATA_LENGTH(NetBuffer);

    if (pFirstMdl == NULL)
    {  
        DBGPRINT(RT_DEBUG_ERROR, ("%s(), Line:%d, first MDL can't be NULL\n", __FUNCTION__, __LINE__));
        
        ndisStatus = NDIS_STATUS_FAILURE;
        return ndisStatus;
    }

    MdlByteCount = MmGetMdlByteCount(pFirstMdl);
    MdlOffset = FirstMdlOffset;
    FirstMdlLength = MdlByteCount - FirstMdlOffset;
    if ((TotalPktLength <= LENGTH_802_11) || (TotalPktLength > MAX_FRAME_SIZE))
    {
        DBGPRINT(RT_DEBUG_ERROR, ("%s(), Line:%d, size(=%d) less than 802.11 header or large than 2346\n", __FUNCTION__, __LINE__,TotalPktLength));
        ndisStatus = NDIS_STATUS_FAILURE;
        return ndisStatus;
    }

    if ((FirstMdlLength) < LENGTH_802_11)
    {
        DBGPRINT(RT_DEBUG_ERROR, ("%s(), Line:%d, first MDL size less than 802.11 header, MdlByteCount=%d, MdlOffset=%d\n", __FUNCTION__, __LINE__, MdlByteCount, MdlOffset));
        ndisStatus = NDIS_STATUS_FAILURE;
        return ndisStatus;
    }    

    pCurrentMdl = pFirstMdl;
    while (pCurrentMdl) 
    {
        pCurrentMdl = pCurrentMdl->Next;
        MdlNum++;
    }

    pXcu->NumOfMdl = (UCHAR)MdlNum; 
    if(MdlNum > NUM_OF_MP_XMIT_CTRL_UNIT_BUF)
    {
        DBGPRINT(RT_DEBUG_ERROR, ("%s(), Line:%d, MdlNum  %d> NUM_TCB_BUFFER\n", __FUNCTION__, __LINE__, MdlNum));
        ndisStatus = NDIS_STATUS_FAILURE;
        return ndisStatus;
    }   
    RemindLength = TotalPktLength;
    pXcu->TotalPacketLength = TotalPktLength;
    pXcu->pAd = pAd;
    pXcu->pPort = pPort;
    pXcu->Reserve1 = NetBufferList;
    pXcu->Reserve2 = NetBuffer;

    //For debug purpose
    {
        ULONG val = (ULONG)MT_GET_NETBUFFER_XCUIDX(NetBuffer);
        if(val !=0)
        {
            MT_SET_NETBUFFER_XCUIDX(NetBuffer, ((pXcu->CurrentXcuidx)|(val<<16)));
        }
        else
        {
            MT_SET_NETBUFFER_XCUIDX(NetBuffer, pXcu->CurrentXcuidx);
        }
    }
    
    Ptr = (PUCHAR)MmGetSystemAddressForMdlSafe( pFirstMdl, NormalPagePriority )+ FirstMdlOffset;
    MdlLength = MmGetMdlByteCount(pFirstMdl) - FirstMdlOffset;

    if(FirstMdlLength == LENGTH_802_11)
    {
        PlatformMoveMemory(pXcu->BufferList[0].VirtualAddr, Ptr, LENGTH_802_11);
        pXcu->BufferList[0].Length = LENGTH_802_11;
        bufferCnt = 1;
        RemindLength -= LENGTH_802_11;
        pNowMdl = pFirstMdl->Next;
    }
    else if (FirstMdlLength < LENGTH_802_11)
    {
        UINT CopyLength =0;
        PUCHAR  CopyPtr = NULL;
        PlatformMoveMemory(pXcu->BufferList[0].VirtualAddr, Ptr, FirstMdlLength);
        CopyLength = FirstMdlLength;

        pCurrentMdl = pFirstMdl;
        while (pCurrentMdl) 
        {
            Ptr = (PUCHAR)MmGetSystemAddressForMdlSafe(pCurrentMdl, NormalPagePriority);
            MdlLength = MmGetMdlByteCount(pCurrentMdl);  

            if ( (CopyLength + CopyLength) >= LENGTH_802_11 )
            {
                PlatformMoveMemory(pXcu->BufferList[0].VirtualAddr+CopyLength, Ptr, LENGTH_802_11-CopyLength);
                CopyPtr = Ptr + (LENGTH_802_11-CopyLength);
                CopyLength += (LENGTH_802_11-CopyLength);
                break;
            }

            PlatformMoveMemory(pXcu->BufferList[0].VirtualAddr+CopyLength, Ptr, MdlLength);
            CopyLength += MdlLength;
        
            pCurrentMdl = pCurrentMdl->Next;
        }

        pXcu->BufferList[0].Length = CopyLength;
        if (CopyLength < LENGTH_802_11 || CopyLength == MdlLength)
        {
            bufferCnt=1;
        }
        else
        {
            pXcu->BufferList[1].VirtualAddr = CopyPtr;
            pXcu->BufferList[1].Length= (ULONG)(MdlLength - (CopyPtr - Ptr));
            bufferCnt=2;
        }

        RemindLength -= CopyLength;
        pNowMdl = pCurrentMdl->Next;
    }
    else //FirstMdlLength >LENGTH_802_11
    {
        PlatformMoveMemory(pXcu->BufferList[0].VirtualAddr, Ptr, LENGTH_802_11);
        pXcu->BufferList[0].Length = LENGTH_802_11;
        pXcu->BufferList[1].VirtualAddr = Ptr + LENGTH_802_11;
        pXcu->BufferList[1].Length= FirstMdlLength - LENGTH_802_11;
        bufferCnt=2;
        pNowMdl = pFirstMdl->Next;      
        RemindLength -= FirstMdlLength;
    }

    pCurrentMdl = pNowMdl;

    while (pCurrentMdl)
    {  
        Ptr = (pCurrentMdl == NET_BUFFER_CURRENT_MDL(NetBuffer)) ?
            (PUCHAR)MmGetSystemAddressForMdlSafe(pCurrentMdl, NormalPagePriority) + NET_BUFFER_CURRENT_MDL_OFFSET(NetBuffer) :
            MmGetSystemAddressForMdlSafe(pCurrentMdl, NormalPagePriority);
            
        MdlLength = (pCurrentMdl == NET_BUFFER_CURRENT_MDL(NetBuffer)) ?
            MmGetMdlByteCount(pCurrentMdl)-NET_BUFFER_CURRENT_MDL_OFFSET(NetBuffer) :
            MmGetMdlByteCount(pCurrentMdl);
            
        pXcu->BufferList[bufferCnt].CurrentMdlPtr = (PUCHAR)pCurrentMdl;
        pXcu->BufferList[bufferCnt].VirtualAddr = Ptr;

        if(RemindLength <MdlLength)
        {
            pXcu->BufferList[bufferCnt].Length = RemindLength;
            RemindLength = 0;
        }
        else
        {
           pXcu->BufferList[bufferCnt].Length = MdlLength;
           RemindLength -= MdlLength;
        }
        
        bufferCnt++;

        if(RemindLength <= 0)
            break;

        if(bufferCnt>= NUM_OF_MP_XMIT_CTRL_UNIT_BUF)
            break;
        pCurrentMdl = pCurrentMdl->Next;  
    }   

    pXcu->NumOfBuf = (UCHAR)bufferCnt;
    
    pXcu->pHeader80211 = (PHEADER_802_11) pXcu->BufferList[0].VirtualAddr;

    if (NetBufferList)
    {
        pDot11SendContext = RTMP_GET_SEND_CONTEXT(NetBufferList);

        if (NET_BUFFER_LIST_INFO(NetBufferList, Ieee8021QNetBufferListInfo) != 0) 
        {
            Ndis8021QInfo.Value = NET_BUFFER_LIST_INFO(NetBufferList, Ieee8021QNetBufferListInfo); 
            pXcu->UserPriority= (UCHAR)Ndis8021QInfo.WLanTagHeader.WMMInfo; 
            pXcu->QueIdx= MapUserPriorityToAccessCategory[pXcu->UserPriority];
        }
        
    }
    
    if (pDot11SendContext)
    {
        pXcu->pDot11SendContext = pDot11SendContext;
    }

    if (NET_BUFFER_NEXT_NB(NetBuffer) == NULL)
    {
        MT_SET_NETBUFFER_LIST(NetBuffer, NetBufferList);
    }
    else
    {
        MT_SET_NETBUFFER_LIST(NetBuffer, NULL);
    }
    
   return ndisStatus;
}

VOID
N6XmitSendNetBufferLists(
    IN  NDIS_HANDLE         MiniportAdapterContext,
    IN  PNET_BUFFER_LIST    NetBufferList,
    IN  NDIS_PORT_NUMBER    PortNumber,
    IN  ULONG               SendFlags
    )
{
    PNET_BUFFER         NetBuffer;
    PNET_BUFFER_LIST    CurrNetBufferList;
    PNET_BUFFER_LIST    NextNetBufferList;
    NDIS_STATUS         ndisStatus = NDIS_STATUS_FAILURE;
    ULONG               ulNumNBLReadyToSend = 0;
    PNET_BUFFER_LIST    failedNBLs = NULL;
    NDIS_STATUS         failedCompletionStatus = NDIS_STATUS_FAILURE;
    PMP_ADAPTER       pAd = (PMP_ADAPTER) MiniportAdapterContext;
    PMP_PORT          pPort = pAd->PortList[PortNumber];
    
    DBGPRINT(RT_DEBUG_INFO, ("==> N6XmitSendNetBufferLists type =%d, Port =%d,(Flags=0x%08x)\n",pPort->PortType,PortNumber,SendFlags));

    do
    {
        ndisStatus = N6XmitCheckHWReadyToSend(pAd, pPort);
        
        if (ndisStatus != NDIS_STATUS_SUCCESS)
        {
            // We fail all the packets right here
            failedNBLs = NetBufferList;
            failedCompletionStatus = ndisStatus;
            DBGPRINT(RT_DEBUG_TRACE, ("N6XmitSendNetBufferLists::N6XmitCheckHWReadyToSend failed (PortNumber=%d) (Status=0x%08x)\n", PortNumber, ndisStatus));
            break;
        }

        for (CurrNetBufferList = NetBufferList; CurrNetBufferList != NULL; CurrNetBufferList = NextNetBufferList)
        {
            UINT    Count = 0;
            NextNetBufferList = NET_BUFFER_LIST_NEXT_NBL(CurrNetBufferList);
            //
            // Cache a reference to the next NBL right away. We always process one NBL
            // at a time so the Reserve1 next pointer will be set to NULL.
            //
            //NET_BUFFER_LIST_NEXT_NBL(CurrNetBufferList) = NULL;

            for (NetBuffer = NET_BUFFER_LIST_FIRST_NB(CurrNetBufferList); NetBuffer != NULL; NetBuffer = NET_BUFFER_NEXT_NB(NetBuffer))
            {
                Count++;
            }
         
            MT_SET_NBL_REF_CNT(CurrNetBufferList, Count);

            NET_BUFFER_LIST_NEXT_NBL(CurrNetBufferList) = NULL;
            
            MT_INC_REF(&pAd->Counter.MTKCounters.PendingNdisPacketCount);

            for (NetBuffer = NET_BUFFER_LIST_FIRST_NB(CurrNetBufferList); NetBuffer != NULL; NetBuffer = NET_BUFFER_NEXT_NB(NetBuffer))
            {
                PMT_XMIT_CTRL_UNIT pXcu = NULL;
                
                PlatformZeroMemory(&NetBuffer->MiniportReserved[0], sizeof(PVOID) * 4);
                
                MT_INC_REF(&pAd->Counter.MTKCounters.PendingNdisPacketCount);
                
                NET_BUFFER_LIST_STATUS(CurrNetBufferList) = NDIS_STATUS_SUCCESS;

                NdisAcquireSpinLock(&pAd->pTxCfg->XcuIdelQueueLock);
                
                pXcu = (PMT_XMIT_CTRL_UNIT)RemoveHeadQueue(&pAd->pTxCfg->XcuIdelQueue);
                ndisStatus = N6XmitGetTxbufToXcu(pAd, PortNumber, CurrNetBufferList, NetBuffer, pXcu);

                if (ndisStatus != NDIS_STATUS_SUCCESS)
                {
                    if(pXcu != NULL)
                    {
                        InsertTailQueue(&pAd->pTxCfg->XcuIdelQueue, pXcu);
                    }
                    failedCompletionStatus = ndisStatus;
                    NdisReleaseSpinLock(&pAd->pTxCfg->XcuIdelQueueLock);
                    break;
                }
                
                pXcu->PktSource = PKTSRC_NDIS;
                NdisReleaseSpinLock(&pAd->pTxCfg->XcuIdelQueueLock);

                ndisStatus = XmitSendPacket(pAd, PortNumber, pXcu);
                if (ndisStatus != NDIS_STATUS_SUCCESS)
                {
                    failedCompletionStatus = ndisStatus;

                    NdisAcquireSpinLock(&pAd->pTxCfg->XcuIdelQueueLock);
                    if(pXcu != NULL)
                    {
                        InsertTailQueue(&pAd->pTxCfg->XcuIdelQueue, pXcu);
                    }
                    NdisReleaseSpinLock(&pAd->pTxCfg->XcuIdelQueueLock);
                    break;
                }

                ulNumNBLReadyToSend++;
            }


            if (ndisStatus != NDIS_STATUS_SUCCESS)
            {
                // Add this to the failed list
                NET_BUFFER_LIST_NEXT_NBL(CurrNetBufferList) = failedNBLs;
                failedNBLs = CurrNetBufferList;
            }
        }
    } while (FALSE);

#if IS_BUS_TYPE_PCI(DEV_BUS_TYPE)
    //
    // PCI don't need to trigger event to dequeue NBL.
    // It will dequeue NBL at ProcessSGListHandler.
    //
#elif IS_BUS_TYPE_USB(DEV_BUS_TYPE)
    if (ulNumNBLReadyToSend > 0)
    {
        // Dequeue one frame from SendTxWait queue and process it
        // There are two place calling dequeue for TX ring.
        // 1. Here, right after queueing the frame.
        // 2. After bulk out complete service routine.
        if ((!MT_TEST_FLAG(pAd, fRTMP_ADAPTER_BSS_SCAN_IN_PROGRESS)) && 
            (!MT_TEST_FLAG(pAd, fRTMP_ADAPTER_RADIO_OFF)) &&
            (!MT_TEST_FLAG(pAd, fRTMP_ADAPTER_RESET_IN_PROGRESS)) &&
            (!MT_TEST_FLAG(pAd, fRTMP_ADAPTER_HALT_IN_PROGRESS)))
        {
            NdisSetEvent(&(GET_THREAD_EVENT(&pAd->pHifCfg->hDequeueDataThread)));
        }
    }
#elif IS_BUS_TYPE_SDIO(DEV_BUS_TYPE)
    //
    // TODO:: Add SDIO 
    //
#endif

    //
    // Complete the NBLs back to NDIS
    //
    if (failedNBLs != NULL)
    {
        ULONG SendCompleteFlags = 0;

        if (NDIS_TEST_SEND_AT_DISPATCH_LEVEL(SendFlags))
            NDIS_SET_SEND_COMPLETE_FLAG(SendCompleteFlags, NDIS_SEND_COMPLETE_FLAGS_DISPATCH_LEVEL);

            DBGPRINT(RT_DEBUG_INFO, ("Completing NBL 0x%p\n", NetBufferList));
#pragma prefast(suppress: __WARNING_INVALID_PARAM_VALUE_3, " NetBufferList should not be NULL")                
            NdisMSendNetBufferListsComplete(pAd->AdapterHandle, failedNBLs, SendCompleteFlags);            
    }

    return;
}

BOOLEAN 
N6XmitGetByteFromMdl(
    IN  PMP_ADAPTER   pAd, 
    IN  PMT_XMIT_CTRL_UNIT pXcu,
    IN  ULONG           Location,
    IN  PUCHAR          pData
    )
{
    PMDL    pCurrentMdl = NULL;
    ULONG   MdlOffset = 0;
    PUCHAR  pMdlData = NULL;
    ULONG   MdlSize, index, NumberOfBytesRead = 0;

    UNREFERENCED_PARAMETER(pAd);

    pCurrentMdl         = NET_BUFFER_FIRST_MDL((PNET_BUFFER)pXcu->Reserve2);
    MdlOffset       = NET_BUFFER_CURRENT_MDL_OFFSET((PNET_BUFFER)pXcu->Reserve2);

    index = Location;

    while(pCurrentMdl)
    {
        NdisQueryBufferSafe(pCurrentMdl,
                            &pMdlData,
                            &MdlSize,
                            NormalPagePriority);

        if(pMdlData != NULL)
        {
            pMdlData += MdlOffset;
            NumberOfBytesRead += (MdlSize - MdlOffset);
        }
        else
        {
            return FALSE;
        }

        if (NumberOfBytesRead > Location)
        {
            PlatformMoveMemory(pData, pMdlData + index, sizeof(UCHAR));
            return TRUE;
        }

        index = index - (MdlSize - MdlOffset);
        //
        // MdlOffset is used only for the first Mdl processed. For the remaining Mdls, it is 0.
        //
        MdlOffset = 0;
        pCurrentMdl = pCurrentMdl->Next;
    }

    return FALSE;
}

