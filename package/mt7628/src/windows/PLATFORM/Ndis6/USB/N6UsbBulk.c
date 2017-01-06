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
    rtusb_bulk.c

    Abstract:

    Revision History:
    Who         When          What
    --------    ----------    ----------------------------------------------
    Paul Lin    06-25-2004    created

*/
#include    "MtConfig.h"

// After insert sub-AMSDU header, the padding bytes is different with in HardTransmit.  So needs some shift.
INT AMSDUPadShift[4]={2, 2, -2, -2};
/*
    ========================================================================
    
    Routine Description:

    Arguments:

    Return Value:

    IRQL = 
    
    Note:
    
    ========================================================================
*/
VOID    N6USBInitTxDesc(
    IN  PMP_ADAPTER           pAd,
    IN  PTX_CONTEXT             pTxContext,
    IN  UCHAR                   BulkOutPipeId,
    IN  PFN_WDF_REQUEST_COMPLETION_ROUTINE  Func)
{
    NTSTATUS            ntStatus;
    PUCHAR              pSrc = NULL;
    WDFREQUEST      writeRequest;
    WDFUSBPIPE      wdfUsbWritePipe;
    PUSB_WRITE_REQ_CONTEXT writeContext;    

    // Store BulkOut PipeId
    writeRequest = pTxContext->WriteRequestArray[0];

    writeContext = GetWriteRequestContext(writeRequest);

    DBGPRINT(RT_DEBUG_INFO, ("N6USBBulkOutDataPacket writeContext->UsbPipe = %x\n", writeContext->UsbPipe));

    // Store BulkOut PipeId
    pTxContext->BulkOutPipeId = BulkOutPipeId;
    
    if (pTxContext->bAggregatible)
    {
        pSrc = &pTxContext->TransferBuffer->Aggregation[2];
    }
    else
    {
        pSrc = (PUCHAR) &pTxContext->TransferBuffer->WirelessPacket[0];
    }

    DBGPRINT(RT_DEBUG_INFO, ("N6USBInitTxDesc %02x %02x %02x %02x %02x %02x %02x %02x\n", pSrc[0], pSrc[1], pSrc[2], pSrc[3], pSrc[4], pSrc[5], pSrc[6], pSrc[7]));
    DBGPRINT(RT_DEBUG_INFO, ("N6USBInitTxDesc %02x %02x %02x %02x %02x %02x %02x %02x\n", pSrc[8], pSrc[9], pSrc[10], pSrc[11], pSrc[12], pSrc[13], pSrc[14], pSrc[15]));

    
    // Initialize the URB
    UsbBuildInterruptOrBulkTransferRequest(
            writeContext->Urb,
            sizeof(struct _URB_BULK_OR_INTERRUPT_TRANSFER),
            writeContext->UsbdPipeHandle,
            pSrc,   //pTxContext->TransferBuffer
            NULL,
            pTxContext->BulkOutSize,
            USBD_TRANSFER_DIRECTION_OUT,
            NULL);
    wdfUsbWritePipe = writeContext->UsbPipe;
    
    ntStatus = WdfUsbTargetPipeFormatRequestForUrb(wdfUsbWritePipe,
                                                   writeRequest,
                                                   writeContext->UrbMemory,
                                                   NULL  );

    if (!NT_SUCCESS(ntStatus)) 
    {
        DBGPRINT(RT_DEBUG_ERROR, ("N6USBBulkOutDataPacket Failed to format requset for urb\n"));
        ntStatus = STATUS_INSUFFICIENT_RESOURCES;
        goto error;
    }
    WdfRequestSetCompletionRoutine(writeRequest, Func, writeContext);

error:
    return;
}

NTSTATUS    N6USBInitHTTxDesc(
    IN  PMP_ADAPTER           pAd,
    IN  PHT_TX_CONTEXT          pTxContext,
    IN  WDFREQUEST                  writeRequest,
    IN  PVOID                   pWriteContext,
    IN  UCHAR                   BulkOutPipeId,
    IN  ULONG                   NextBulkOutPosition,        
    IN  ULONG                   BulkOutSize,
    IN  UCHAR                   IrpIndex,
    IN   PFN_WDF_REQUEST_COMPLETION_ROUTINE     Func)
{
    NTSTATUS            ntStatus;       
    PUCHAR              pSrc = NULL;
    WDFUSBPIPE      wdfUsbWritePipe;
    PUSB_WRITE_REQ_CONTEXT writeContext = (PUSB_WRITE_REQ_CONTEXT)pWriteContext;    

    // Store BulkOut PipeId
    pTxContext->BulkOutPipeId = BulkOutPipeId;

    pSrc = &pTxContext->TransferBuffer->WirelessPacket[NextBulkOutPosition];
    
    // Initialize the URB
    UsbBuildInterruptOrBulkTransferRequest(
            writeContext->Urb,
            sizeof(struct _URB_BULK_OR_INTERRUPT_TRANSFER),
            writeContext->UsbdPipeHandle,
            pSrc,   //pTxContext->TransferBuffer
            NULL,
            BulkOutSize,
            USBD_TRANSFER_DIRECTION_OUT | USBD_SHORT_TRANSFER_OK,
            NULL);
    writeContext->WriteSize = BulkOutSize;
    writeContext->IrpIdx = IrpIndex;
    wdfUsbWritePipe = writeContext->UsbPipe;
    
    ntStatus = WdfUsbTargetPipeFormatRequestForUrb(wdfUsbWritePipe,
                                                   writeRequest,
                                                   writeContext->UrbMemory,
            NULL);

    if (!NT_SUCCESS(ntStatus)) 
    {
        DBGPRINT(RT_DEBUG_ERROR, ("N6USBBulkOutDataPacket Failed to format requset for urb\n"));
        ntStatus = STATUS_INSUFFICIENT_RESOURCES;
        goto error;
    }
    WdfRequestSetCompletionRoutine(writeRequest, Func, writeContext);

error:
    return ntStatus;   
}

/*
    ========================================================================

    Routine Description:
        
    Arguments:
        
    Return Value:

    Note:
    
    ========================================================================
*/
NTSTATUS    N6USBInitRxDesc(
    IN  PMP_ADAPTER   pAd,
    IN  PRX_CONTEXT     pRxContext)
{   
    ULONG               RX_bulk_size;
    NTSTATUS                    ntStatus;
    PUSB_DEVICE_CONTEXT pUsbDeviceContext;
    
    pUsbDeviceContext = GetUsbDeviceContext(pAd->pHifCfg->UsbDevice);
    if ( pUsbDeviceContext->BulkInMaxPacketSize == 64)
        RX_bulk_size = 4096;
    else
        RX_bulk_size = MAX_RXBULK_SIZE;

    ntStatus = WdfMemoryAssignBuffer(
                    pRxContext->RxWdfMemory,
                    &pRxContext->TransferBuffer[0/*pAd->pHifCfg->NextRxBulkInPosition*/],
                    (RX_bulk_size -pAd->pHifCfg->NextRxBulkInPosition)
                    );
    if (!NT_SUCCESS(ntStatus)) 
    {
        DBGPRINT_RAW(RT_DEBUG_TRACE, ("N6USBInitRxDesc Failed to assign memory object for transfer buffer \n"));
        ntStatus = STATUS_INSUFFICIENT_RESOURCES;
        goto error;
    }

    //
    // The format call validates to make sure that you are reading or
    // writing to the right pipe type, sets the appropriate transfer flags,
    // creates an URB and initializes the request.
    //
    ntStatus = WdfUsbTargetPipeFormatRequestForRead(
                                    pRxContext->pipe,
                                    pRxContext->Request,
                                    pRxContext->RxWdfMemory,
                                    NULL // Offsets
                                    ); 
    if (!NT_SUCCESS(ntStatus)) 
    {
        DBGPRINT_RAW(RT_DEBUG_TRACE, ("N6USBInitRxDesc Failed to format requset for urb\n"));
        ntStatus = STATUS_INSUFFICIENT_RESOURCES;
        goto error;
    }

    if (pRxContext->Index == 31)
    {
        WdfRequestSetCompletionRoutine(pRxContext->Request, N6USBBulkRxCMDComplete, pRxContext);    
    }
    else
    {
        WdfRequestSetCompletionRoutine(pRxContext->Request, N6USBBulkRxComplete, pRxContext);    
    }

error:
    return ntStatus;
}

/*
    ========================================================================

    Routine Description: Compose one MPDU or AMSDU.
        
    Arguments:
        
    Return Value:

    Note:
    
    ========================================================================
*/
ULONG   N6USBComposeUnit(
    IN  PMP_ADAPTER   pAd,
    IN  UCHAR           BulkOutPipeId,
    IN OUT    ULONG     *pAMSDUExist,
    IN OUT    ULONG     *pTmpBulkPos,
    IN OUT    ULONG     *pTotalBulkSize,
    IN OUT    ULONG     *pTmpBulkEndPos)
{
    PHT_TX_CONTEXT                  pTxContext;
    //PTXWI_STRUC                   pTXWI[2];
    //PTXINFO_STRUC                 pTXINFO[2];
    PHEADER_802_11                  pHeader80211;   
    ULONG                           USBDMATxPktLen;     
    ULONG                           SwUseSegmentEnd, SegmentIdx;
    ULONG                           CurBulkSize;
    UCHAR                           TID;
    PHEADER_802_11                  p80211;
    UCHAR                           allzero[4]= {0x0,0x0,0x0,0x0};
    
    PTXDSCR_SHORT_STRUC             pShortFormatTxD = NULL;
    PTXDSCR_LONG_STRUC          pLongFormatTxD = NULL;
    PTXDSCR_DW0                 pTxDDW0 = NULL;
    PTXDSCR_DW1                 pTxDDW1 = NULL;
    PTXDSCR_DW7                 pTxDDW7 = NULL;
    ULONG                           TxDSize;

    
        
    pTxContext = &(pAd->pHifCfg->TxContext[BulkOutPipeId]);

    pTxDDW0 = (PTXDSCR_DW0)&pTxContext->TransferBuffer->WirelessPacket[*pTmpBulkEndPos];
    pTxDDW1 = (PTXDSCR_DW1)&pTxContext->TransferBuffer->WirelessPacket[(*pTmpBulkEndPos) + 4];
        
    
    if (pTxDDW1->TxDescriptionFormat == TXD_LONG_FORMAT)
    {
        TxDSize = TXD_LONG_SIZE;
        pLongFormatTxD = (PTXDSCR_LONG_STRUC)&pTxContext->TransferBuffer->WirelessPacket[*pTmpBulkEndPos];
        pHeader80211 = (PHEADER_802_11) &pTxContext->TransferBuffer->WirelessPacket[(*pTmpBulkEndPos) + TxDSize];   
        pTxDDW7 = &pLongFormatTxD->TxDscrDW7;

        DBGPRINT(RT_DEBUG_INFO, ("using TXD_LONG_SIZE\n"));
    }
    else
    {
        TxDSize = TXD_SHORT_SIZE;
        pShortFormatTxD = (PTXDSCR_SHORT_STRUC)&pTxContext->TransferBuffer->WirelessPacket[*pTmpBulkEndPos];
        pHeader80211 = (PHEADER_802_11) &pTxContext->TransferBuffer->WirelessPacket[(*pTmpBulkEndPos) + TxDSize];   
        pTxDDW7 = &pShortFormatTxD->TxDscrDW7;

        DBGPRINT(RT_DEBUG_INFO, ("using TXD_SHORT_SIZE\n"));
    }

    

    if (0)
    {
        int i = 0;
        PUCHAR p = (PUCHAR)&pTxContext->TransferBuffer->WirelessPacket[*pTmpBulkEndPos];
        //for (i = 0; i < (int)pRxD->RxDscrDW0.RxByteCount; i = i + 8)
        for (i = 0; i < (int)pTxDDW0->TxByteCount; i = i + 8)
        {
            DBGPRINT_RAW(RT_DEBUG_ERROR, ("[%d] %x %x %x %x %x %x %x %x\n", i, p[i], p[i+1], p[i+2], p[i+3], p[i+4], p[i+5], p[i+6], p[i+7]));
        }
    }       
    TID = (UCHAR)pTxDDW1->TID;
    
    USBDMATxPktLen = pTxDDW0->TxByteCount - TxDSize;
    
    SwUseSegmentEnd = pTxDDW7->SwUseSegmentEnd;
    SegmentIdx = pTxDDW7->SwUseSegIdx;

    {
        p80211 = (PHEADER_802_11) &pTxContext->TransferBuffer->WirelessPacket[(*pTmpBulkEndPos) + TxDSize]; 

        CurBulkSize = pTxDDW0->TxByteCount + pTxDDW7->SwUseUSB4ByteAlign;
        
        *pTmpBulkEndPos += CurBulkSize;
        *pTmpBulkPos += CurBulkSize;
        *pTotalBulkSize += CurBulkSize;

        //pTxDDW7->SwUseAMSDU = 0;
    }

    pTxDDW7->SwUseSegmentEnd = 0;
    pTxDDW7->SwUseSegIdx = 0;


    
    // Set HW-not-use bit zero.
    pTxDDW7->SwUseNonQoS = 0;
    return (SwUseSegmentEnd + (SegmentIdx << 8));
}

/*
    ========================================================================
    
    Routine Description:

    Arguments:

    Return Value:

    IRQL = 
    
    Note:
    
    ========================================================================
*/
VOID    N6USBBulkOutDataPacket(
    IN  PMP_ADAPTER   pAd,
    IN  UCHAR           BulkOutPipeId,
    IN  UCHAR           Index)
{
    NTSTATUS                        Status;
    PHT_TX_CONTEXT                  pTxContext;
    ULONG                           TmpBulkEndPos;
    ULONG                           ThisBulkSize;
    UCHAR                           allzero[4]= {0x0,0x0,0x0,0x0};
    ULONG                           TmpBulkPos;
    ULONG                           SwUseSegmentEnd = 0, SwUseSegIdx;
    ULONG                           bAMSDUExist = 0;
    ULONG                           TmpBulkZeroEndPos = 0;
    UCHAR                           IrpIdx = 0;
    PUCHAR                          pDest;  
    WDFREQUEST                          writeRequest;
    PUSB_WRITE_REQ_CONTEXT      writeContext;  
    PTXDSCR_SHORT_STRUC             pShortFormatTxD = NULL;
    PTXDSCR_LONG_STRUC          pLongFormatTxD = NULL;
    PTXDSCR_DW0                 pTxDDW0 = NULL;
    PTXDSCR_DW1                 pTxDDW1 = NULL;
    PTXDSCR_DW7                 pTxDDW7 = NULL;
    ULONG                           TxDSize;
    ULONG                           USBDMATxPktLen;
    //ULONG                         RoundCount = 0;

    DBGPRINT(RT_DEBUG_TRACE, ("N6USBBulkOutDataPacket\n"));
    
    // no bulk out for radio off    
    if (MT_TEST_FLAG(pAd, fRTMP_ADAPTER_RADIO_OFF))
    {
        DBGPRINT(RT_DEBUG_TRACE, ("N6USBBulkOutDataPacket - fRTMP_ADAPTER_RADIO_OFF, RETURN!\n"));  
        return;
    }
    
    pTxContext = &(pAd->pHifCfg->TxContext[BulkOutPipeId]);

    if(pAd->pHifCfg->bIRPBulkOutData)
    {
        DBGPRINT(RT_DEBUG_TRACE, ("N6USBBulkOutDataPacket - 4 RETURN!\n"));
        NdisSetEvent(&(GET_THREAD_EVENT(&pAd->pHifCfg->hDequeueDataThread)));
        return;
    }
    pAd->pHifCfg->bIRPBulkOutData = TRUE;

#if 1
#if 0
    NdisAcquireSpinLock(&pAd->pTxCfg->TxContextQueueLock[BulkOutPipeId]);
    if (pAd->pHifCfg->BulkOutPending[BulkOutPipeId] == TRUE)
    {
        NdisReleaseSpinLock(&pAd->pTxCfg->TxContextQueueLock[BulkOutPipeId]);
        return;
    }
    pAd->pHifCfg->BulkOutPending[BulkOutPipeId] = TRUE;
    NdisReleaseSpinLock(&pAd->pTxCfg->TxContextQueueLock[BulkOutPipeId]);
#else
    NdisAcquireSpinLock(&pAd->pTxCfg->TxContextQueueLock[BulkOutPipeId]);
    if ((pTxContext->IRPPending[0] == TRUE) && (pTxContext->IRPPending[1] == TRUE))
    {
        DBGPRINT(RT_DEBUG_TRACE, ("N6USBBulkOutDataPacket - 3 RETURN!\n"));
        pAd->pHifCfg->bIRPBulkOutData = FALSE;
        NdisReleaseSpinLock(&pAd->pTxCfg->TxContextQueueLock[BulkOutPipeId]);
        //DBGPRINT_RAW(RT_DEBUG_TRACE,("  IRPPending all TRUE.  \n"));
        pAd->pHifCfg->BulkOutReqCount[2]++;
        NdisSetEvent(&(GET_THREAD_EVENT(&pAd->pHifCfg->hDequeueDataThread)));
        return;
    }
    else if ((!((pTxContext->IRPPending[0] == FALSE) && (pTxContext->IRPPending[1] == FALSE)))
    && ((pAd->pHifCfg->TxContext[BulkOutPipeId].bCopySavePad == TRUE) || (pTxContext->bCurWriting == TRUE)))
    {   
        DBGPRINT(RT_DEBUG_TRACE, ("N6USBBulkOutDataPacket - 2 RETURN!\n"));
        pAd->pHifCfg->bIRPBulkOutData = FALSE;
        // Already one IRP bulked out. 
        // if bCurWriting=TRUE, just return here.  
        // if bCopySavePad = TRUE,   just return here.
        NdisReleaseSpinLock(&pAd->pTxCfg->TxContextQueueLock[BulkOutPipeId]);
            DBGPRINT_RAW(RT_DEBUG_INFO,("Only One IRPPending.  SavedPad : %x  %x  %x  %x  %x  %x  %x  %x \n",
                pAd->pHifCfg->TxContext[BulkOutPipeId].SavedPad[0], pAd->pHifCfg->TxContext[BulkOutPipeId].SavedPad[1], pAd->pHifCfg->TxContext[BulkOutPipeId].SavedPad[2],pAd->pHifCfg->TxContext[BulkOutPipeId].SavedPad[3]
                ,pAd->pHifCfg->TxContext[BulkOutPipeId].SavedPad[4], pAd->pHifCfg->TxContext[BulkOutPipeId].SavedPad[5], pAd->pHifCfg->TxContext[BulkOutPipeId].SavedPad[6],pAd->pHifCfg->TxContext[BulkOutPipeId].SavedPad[7]));
        return;
    }

    if (pTxContext->IRPPending[0] == FALSE)
    {
        IrpIdx =0;
    }
    else if (pTxContext->IRPPending[1] == FALSE)
    {
        IrpIdx =1;
        pAd->pHifCfg->BulkOutReqCount[1]++;
    }

    
    pTxContext->IRPPending[IrpIdx] = TRUE;      
#endif
    
    if ((pAd->pHifCfg->TxContext[BulkOutPipeId].ENextBulkOutPosition == pAd->pHifCfg->TxContext[BulkOutPipeId].CurWritePosition)
        || ((pAd->pHifCfg->TxContext[BulkOutPipeId].ENextBulkOutPosition-8) == pAd->pHifCfg->TxContext[BulkOutPipeId].CurWritePosition))
    {
        DBGPRINT(RT_DEBUG_TRACE, ("N6USBBulkOutDataPacket - 1 RETURN!\n"));
        pAd->pHifCfg->bIRPBulkOutData = FALSE;
        pTxContext->IRPPending[IrpIdx] = FALSE;
        NdisReleaseSpinLock(&pAd->pTxCfg->TxContextQueueLock[BulkOutPipeId]);       
        return;
    }
#endif

    //DBGPRINT(RT_DEBUG_TRACE, ("N6USBBulkOutDataPacket ===> \n"));
    if (OPSTATUS_TEST_FLAG(pAd, fOP_STATUS_DOZE))
    {
        pAd->pHifCfg->bIRPBulkOutData = FALSE;
        pTxContext->IRPPending[IrpIdx] = FALSE;
        NdisReleaseSpinLock(&pAd->pTxCfg->TxContextQueueLock[BulkOutPipeId]);       

        DBGPRINT(RT_DEBUG_TRACE, ("N6USBBulkOutDataPacket - fOP_STATUS_DOZE, RETURN!\n"));  
        AsicForceWakeup(pAd);
        MlmeSetPsm (pAd, PWR_ACTIVE);
        NdisSetEvent(&(GET_THREAD_EVENT(&pAd->pHifCfg->hBulkOutDataThread[BulkOutPipeId])));
        return;
    }

    // Increase Total transmit byte counter
    pAd->Counter.MTKCounters.OneSecTransmittedByteCount +=  pTxContext->BulkOutSize; 
    pAd->Counter.MTKCounters.TransmittedByteCount +=  pTxContext->BulkOutSize;
    
    // Clear Data flag
    MTUSB_CLEAR_BULK_FLAG(pAd, (fRTUSB_BULK_OUT_DATA_FRAG << BulkOutPipeId));
    MTUSB_CLEAR_BULK_FLAG(pAd, (fRTUSB_BULK_OUT_DATA_NORMAL << BulkOutPipeId));

    pAd->pHifCfg->BulkOutDataOneSecCount++;
    pAd->pHifCfg->TxContext[BulkOutPipeId].NextBulkOutPosition = pAd->pHifCfg->TxContext[BulkOutPipeId].ENextBulkOutPosition; 

    ThisBulkSize = 0;
    TmpBulkPos = pAd->pHifCfg->TxContext[BulkOutPipeId].NextBulkOutPosition;
    TmpBulkEndPos = pAd->pHifCfg->TxContext[BulkOutPipeId].NextBulkOutPosition;
    // In previous BulkOut, We saved 8 bytes.  So Now, restore these 8 bytes.
    if ((pAd->pHifCfg->TxContext[BulkOutPipeId].bCopySavePad == TRUE))
    {
        if (PlatformEqualMemory(pAd->pHifCfg->TxContext[BulkOutPipeId].SavedPad, allzero,4))
        {
            DBGPRINT_RAW(RT_DEBUG_TRACE,("allzero : %x  %x  %x  %x  %x  %x  %x  %x \n",
                pAd->pHifCfg->TxContext[BulkOutPipeId].SavedPad[0], pAd->pHifCfg->TxContext[BulkOutPipeId].SavedPad[1], pAd->pHifCfg->TxContext[BulkOutPipeId].SavedPad[2],pAd->pHifCfg->TxContext[BulkOutPipeId].SavedPad[3]
                ,pAd->pHifCfg->TxContext[BulkOutPipeId].SavedPad[4], pAd->pHifCfg->TxContext[BulkOutPipeId].SavedPad[5], pAd->pHifCfg->TxContext[BulkOutPipeId].SavedPad[6],pAd->pHifCfg->TxContext[BulkOutPipeId].SavedPad[7]));
        }
        PlatformMoveMemory(&pAd->pHifCfg->TxContext[BulkOutPipeId].TransferBuffer->WirelessPacket[TmpBulkEndPos], pAd->pHifCfg->TxContext[BulkOutPipeId].SavedPad, 8);
        pAd->pHifCfg->TxContext[BulkOutPipeId].bCopySavePad = FALSE;
        if (pAd->pHifCfg->bForcePrintTX == TRUE)
            DBGPRINT(RT_DEBUG_TRACE,("N6USBBulkOutDataPacket --> COPY PAD. CurWrite = %d, NextBulk = %d.   ENextBulk = %d.\n",   pTxContext->CurWritePosition, pTxContext->NextBulkOutPosition, pAd->pHifCfg->TxContext[BulkOutPipeId].ENextBulkOutPosition));
    }

    pTxDDW0 = (PTXDSCR_DW0)&pTxContext->TransferBuffer->WirelessPacket[TmpBulkEndPos];
    pTxDDW1 = (PTXDSCR_DW1)&pTxContext->TransferBuffer->WirelessPacket[TmpBulkEndPos + 4];
    
    if (pTxDDW1->TxDescriptionFormat == TXD_LONG_FORMAT)
    {
        TxDSize = TXD_LONG_SIZE;
        pLongFormatTxD = (PTXDSCR_LONG_STRUC)&pTxContext->TransferBuffer->WirelessPacket[TmpBulkEndPos];
        pTxDDW7 = &pLongFormatTxD->TxDscrDW7;
    }
    else
    {
        TxDSize = TXD_SHORT_SIZE;
        pShortFormatTxD = (PTXDSCR_SHORT_STRUC)&pTxContext->TransferBuffer->WirelessPacket[TmpBulkEndPos];
        pTxDDW7 = &pShortFormatTxD->TxDscrDW7;
    }

    DBGPRINT(RT_DEBUG_INFO, ("N6USBBulkOutDataPacket ===> pTxDDW0->TxByteCount = %d\n", pTxDDW0->TxByteCount));

    // Make Ralink USB DMA aggregation URB
    do
    {
        // Limit BulkOut size to about 16896 bytes + one wireless packet size, but it will decrease 20Mbps throughput.
        // The SuperFrame size must "ThisBulkSize <= (ONE_SEGMENT_UNIT-LOCAL_TXBUF_SIZE)"
        // If not follow it, "TxQ0 or TxQ1 not empty" issue will happen.
        if ( ThisBulkSize >= 0x7000 )           
        {
            if ((TmpBulkEndPos != pAd->pHifCfg->TxContext[BulkOutPipeId].CurWritePosition))
            {
                PlatformMoveMemory(pAd->pHifCfg->TxContext[BulkOutPipeId].SavedPad, &pAd->pHifCfg->TxContext[BulkOutPipeId].TransferBuffer->WirelessPacket[TmpBulkEndPos], 8);
                pAd->pHifCfg->TxContext[BulkOutPipeId].bCopySavePad = TRUE;
            }               
            pAd->pHifCfg->TxContext[BulkOutPipeId].ENextBulkOutPosition = TmpBulkEndPos;
            pAd->pHifCfg->BulkOutReqCount[0]++;
            break;
        }
        else if (((pAd->pHifCfg->BulkOutMaxPacketSize < 512) && ((ThisBulkSize&0xfffff800) != 0) ) )// || ( (ThisBulkSize != 0)  && (pTxWI->AMPDU == 0)))
        {
            // In USB 1.0, Don't adopt Ralink USB DMA Aggregation.
            if ((TmpBulkEndPos != pAd->pHifCfg->TxContext[BulkOutPipeId].CurWritePosition))
            {
                PlatformMoveMemory(pAd->pHifCfg->TxContext[BulkOutPipeId].SavedPad, &pAd->pHifCfg->TxContext[BulkOutPipeId].TransferBuffer->WirelessPacket[TmpBulkEndPos], 8);
                pAd->pHifCfg->TxContext[BulkOutPipeId].bCopySavePad = TRUE;
            }               
            pAd->pHifCfg->TxContext[BulkOutPipeId].ENextBulkOutPosition = TmpBulkEndPos;
            break;
        }
        
        // Stop at no more packet in buffer.
        DBGPRINT(RT_DEBUG_INFO, ("TmpBulkEndPos = %d, pAd->pHifCfg->TxContext[BulkOutPipeId].CurWritePosition = %d\n", TmpBulkEndPos, pAd->pHifCfg->TxContext[BulkOutPipeId].CurWritePosition));
        if ((TmpBulkEndPos == pAd->pHifCfg->TxContext[BulkOutPipeId].CurWritePosition))
        {
            //RoundCount ++;
            //if (RoundCount < 500)
            //{
            //  continue;
            //}

            pAd->pHifCfg->TxContext[BulkOutPipeId].ENextBulkOutPosition = TmpBulkEndPos;
            break;
        }
        
        pTxDDW0 = (PTXDSCR_DW0)&pTxContext->TransferBuffer->WirelessPacket[TmpBulkEndPos];
        pTxDDW1 = (PTXDSCR_DW1)&pTxContext->TransferBuffer->WirelessPacket[TmpBulkEndPos + 4];
        
        if (pTxDDW1->TxDescriptionFormat == TXD_LONG_FORMAT)
        {
            TxDSize = TXD_LONG_SIZE;
            pLongFormatTxD = (PTXDSCR_LONG_STRUC)&pTxContext->TransferBuffer->WirelessPacket[TmpBulkEndPos];
            pTxDDW7 = &pLongFormatTxD->TxDscrDW7;
        }
        else
        {
            TxDSize = TXD_SHORT_SIZE;
            pShortFormatTxD = (PTXDSCR_SHORT_STRUC)&pTxContext->TransferBuffer->WirelessPacket[TmpBulkEndPos];
            pTxDDW7 = &pShortFormatTxD->TxDscrDW7;
        }

        USBDMATxPktLen = pTxDDW0->TxByteCount - TxDSize;
        // Error case!! Shouldn't happen.
        
        if(USBDMATxPktLen == 0)
        {       
            pAd->pHifCfg->bIRPBulkOutData = FALSE;
            DBGPRINT(RT_DEBUG_TRACE,("h-e2  USBDMATxPktLen == 0 --> Size = %d, bCopySavePad= %d, .CurWrite = %d, NextBulk = %d. \n", pAd->pHifCfg->TxContext[BulkOutPipeId].BulkOutSize, pAd->pHifCfg->TxContext[BulkOutPipeId].bCopySavePad, pTxContext->CurWritePosition, pTxContext->NextBulkOutPosition));
            {
                DBGPRINT_RAW(RT_DEBUG_TRACE,("%x  %x  %x  %x  %x  %x  %x  %x \n",
                    pAd->pHifCfg->TxContext[BulkOutPipeId].SavedPad[0], pAd->pHifCfg->TxContext[BulkOutPipeId].SavedPad[1], pAd->pHifCfg->TxContext[BulkOutPipeId].SavedPad[2],pAd->pHifCfg->TxContext[BulkOutPipeId].SavedPad[3]
                    ,pAd->pHifCfg->TxContext[BulkOutPipeId].SavedPad[4], pAd->pHifCfg->TxContext[BulkOutPipeId].SavedPad[5], pAd->pHifCfg->TxContext[BulkOutPipeId].SavedPad[6],pAd->pHifCfg->TxContext[BulkOutPipeId].SavedPad[7]));
            }
            pAd->pHifCfg->bForcePrintTX = TRUE;      
            pTxContext->IRPPending[IrpIdx] = FALSE;
            NdisReleaseSpinLock(&pAd->pTxCfg->TxContextQueueLock[BulkOutPipeId]);
            return;
        }
        SwUseSegmentEnd = N6USBComposeUnit(pAd, BulkOutPipeId, &bAMSDUExist, &TmpBulkPos, &ThisBulkSize, &TmpBulkEndPos);

        DBGPRINT(RT_DEBUG_INFO, ("2 SwUseSegmentEnd = %x, TmpBulkPos = %d, ThisBulkSize = %d, TmpBulkEndPos = %d\n", SwUseSegmentEnd, TmpBulkPos, ThisBulkSize, TmpBulkEndPos));

        SwUseSegIdx = ((SwUseSegmentEnd>>8));
        SwUseSegmentEnd &= 0x1;

        DBGPRINT(RT_DEBUG_INFO, ("2 SwUseSegmentEnd = %x, SwUseSegIdx = %x\n", SwUseSegmentEnd, SwUseSegIdx));
        
        pAd->pHifCfg->TxContext[BulkOutPipeId].ENextBulkOutPosition = TmpBulkEndPos;
        if ((SwUseSegmentEnd != 0) && (SwUseSegIdx < SEGMENT_TOTAL))
        {
            // Round to head of buffer.
            pAd->pHifCfg->TxContext[BulkOutPipeId].ENextBulkOutPosition = ONE_SEGMENT_UNIT * SwUseSegIdx;
            break;
        }
        else if ((SwUseSegmentEnd != 0) && (SwUseSegIdx == SEGMENT_TOTAL))
        {
            // Round to head of buffer.
            pAd->pHifCfg->TxContext[BulkOutPipeId].ENextBulkOutPosition = 8;
            break;
        }
    }while (TRUE);

    //Force last packet's NextValid to FALSE
    if (pTxDDW0)
    {
        pTxDDW0->USBNextValid = 0;
    }

    DBGPRINT(RT_DEBUG_INFO, ("pTxDDW0->USBNextValid = %d, pTxDDW0->USBTxBurst = %d\n", pTxDDW0->USBNextValid, pTxDDW0->USBTxBurst));

    // The data is ok.Start to make a complete URB.
    // USB DMA engine requires to pad extra 4 bytes. 

    TmpBulkZeroEndPos = pAd->pHifCfg->TxContext[BulkOutPipeId].NextBulkOutPosition + ThisBulkSize;
    pDest = &pTxContext->TransferBuffer->WirelessPacket[TmpBulkZeroEndPos];
    PlatformZeroMemory(pDest, 8);
    ThisBulkSize += 4;
    pTxContext->LastOne = TRUE;

    pAd->pHifCfg->TxContext[BulkOutPipeId].BulkOutSize = ThisBulkSize;
    pAd->pHifCfg->BulkOutSize = ThisBulkSize;
    pAd->pHifCfg->SegmentEnd = SwUseSegmentEnd;  

    // Init Tx context descriptor
    writeRequest = pTxContext->WriteRequestArray[IrpIdx];
    writeContext = GetWriteRequestContext(writeRequest);
    Status = N6USBInitHTTxDesc(pAd, pTxContext, writeRequest, (PVOID)writeContext, BulkOutPipeId, pTxContext->NextBulkOutPosition,ThisBulkSize, IrpIdx, N6USBBulkOutDataPacketComplete);
    pAd->pHifCfg->BulkFailParm.NextBulkOutPositon[BulkOutPipeId][IrpIdx] = pTxContext->NextBulkOutPosition;
    pAd->pHifCfg->BulkFailParm.BulkOutSize[BulkOutPipeId][IrpIdx] = ThisBulkSize;
    pTxContext->BulkoutEPCountDown = 2;
        
    DBGPRINT(RT_DEBUG_INFO, ("1 BulkOut Data Size = %d, IrpIdx = %d\n", pAd->pHifCfg->TxContext[BulkOutPipeId].BulkOutSize, IrpIdx));    

    if( 0)
    {
        int i = 0;
        PUCHAR p = (PUCHAR)&pTxContext->TransferBuffer->WirelessPacket[pTxContext->NextBulkOutPosition];
        //for (i = 0; i < (int)pRxD->RxDscrDW0.RxByteCount; i = i + 8)
        for (i = 0; i < (int)ThisBulkSize; i = i + 8)
        {
            DBGPRINT_RAW(RT_DEBUG_ERROR, ("[%d] %x %x %x %x %x %x %x %x\n", i, p[i], p[i+1], p[i+2], p[i+3], p[i+4], p[i+5], p[i+6], p[i+7]));
        }
    }
    
    NdisReleaseSpinLock(&pAd->pTxCfg->TxContextQueueLock[BulkOutPipeId]);   

#if 1
    if(NT_SUCCESS(Status))
    {   
        pAd->TrackInfo.debugpacketcount ++;
        if (WdfRequestSend(writeRequest, writeContext->IoTarget, WDF_NO_SEND_OPTIONS ) == FALSE) 
        {   // Failure - Return request
            DBGPRINT(RT_DEBUG_ERROR, ("N6USBBulkOutDataPacket send failed\n"));     
        }
    }
    MT_INC_REF(&pAd->pHifCfg->PendingTx[BulkOutPipeId]);
#endif

    pAd->pHifCfg->bIRPBulkOutData = FALSE;

    DBGPRINT_RAW(RT_DEBUG_INFO, ("<---N6USBBulkOutDataPacket \n"));
    
}

/*
    ========================================================================
    
    Routine Description:

    Arguments:

    Return Value:

    IRQL <=  DISPATCH_LEVEL
    
    Note:
    
    ========================================================================
*/
VOID    N6USBBulkOutDataPacketComplete(
    IN WDFREQUEST  Request,
    IN WDFIOTARGET  Target,
    IN PWDF_REQUEST_COMPLETION_PARAMS  CompletionParams,
    IN WDFCONTEXT  Context)
{
    PMP_ADAPTER   pAd;
    PHT_TX_CONTEXT  pTxContext;
    UCHAR           BulkOutPipeId;
    UCHAR           IrpIdx = 0;
    NTSTATUS            status; 
    PUSB_WRITE_REQ_CONTEXT  writeContext;
#if UAPSD_AP_SUPPORT    
    PMP_PORT pPort;
    UCHAR       Idx = 0;
#endif
    //
    // For usb devices, we should look at the Usb.Completion param.
    //
    //usbCompletionParams = CompletionParams->Parameters.Usb.Completion;
    status = CompletionParams->IoStatus.Status;
    writeContext = (PUSB_WRITE_REQ_CONTEXT)Context;

    pTxContext =(PHT_TX_CONTEXT ) writeContext->pTxContext;
    pAd = pTxContext->pAd;

    IrpIdx = writeContext->IrpIdx;

    DBGPRINT(RT_DEBUG_INFO, ("2 IrpIdx = %d\n", IrpIdx));   

    // Store BulkOut PipeId
    BulkOutPipeId = pTxContext->BulkOutPipeId;
    ReturnWriteRequest(pAd,Request);
#if UAPSD_AP_SUPPORT
    pPort =  pAd->PortList[FXXK_PORT_0];
    if(IS_AP_SUPPORT_UAPSD(pAd, pPort))
    {
        ApUapsdUnTagFrame( pAd, BulkOutPipeId, pTxContext->NextBulkOutPosition, pTxContext->ENextBulkOutPosition);
    }
#endif  
    if (status == STATUS_CANCELLED)
    {
        DBGPRINT_RAW(RT_DEBUG_ERROR, ("BulkOutComplete(STATUS_CANCELLED) IrpIdx = %d, BulkOutPipeId = %d\n",IrpIdx, BulkOutPipeId));            
        FREE_HTTX_RING(pAd, BulkOutPipeId, pTxContext);     
        pAd->pHifCfg->BulkOutCompleteCancel ++;
        NdisAcquireSpinLock(&pAd->pTxCfg->TxContextQueueLock[BulkOutPipeId]);
#if 0
        if (pTxContext->AnotherPendingTx & 0xF0)
            pTxContext->AnotherPendingTx = (pTxContext->AnotherPendingTx | (0x01 << IrpIdx));
        else
            pTxContext->AnotherPendingTx = (0x10 << IrpIdx);
#else       
        if (pTxContext->AnotherPendingTx == 0x00) //First Irq cancel return
        {
            pTxContext->AnotherPendingTx = (0x01&IrpIdx);
            pTxContext->AnotherPendingTx |= 0x10;
        }
        else //Second Irq cancel return
        {
            pTxContext->AnotherPendingTx |= (0x02&(IrpIdx<<1));
            pTxContext->AnotherPendingTx |= 0x20;           
        }
#endif      
        //pTxContext->AnotherPendingTx |= (1 << IrpIdx);
        pTxContext->IRPPending[IrpIdx] = FALSE;
        NdisReleaseSpinLock(&pAd->pTxCfg->TxContextQueueLock[BulkOutPipeId]);
        MT_DEC_REF(&pAd->pHifCfg->PendingTx[BulkOutPipeId]);           
        return;
    }
    else if (status == STATUS_SUCCESS)
    {
        // to remove
        //
        if (pAd->pHifCfg->BulkLastOneSecCount < 50)
            pAd->pHifCfg->BulkLastOneSecCount ++;
        
        pAd->pHifCfg->BulkOutComplete++;

        pAd->Counter.Counters8023.GoodTransmits++;      
        FREE_HTTX_RING(pAd, BulkOutPipeId, pTxContext);         
        NdisSetEvent(&(GET_THREAD_EVENT(&pAd->pHifCfg->hDequeueDataThread)));
#if UAPSD_AP_SUPPORT
        // Dequeue outgoing frames from TxSwUapsdQueue[] and process it
        if (IS_AP_SUPPORT_UAPSD(pAd, pPort))
        {
            PMAC_TABLE_ENTRY pMacEntry = NULL;
            PMAC_TABLE_ENTRY pNextMacEntry = NULL;              
            PQUEUE_HEADER pHeader;

            pHeader = &pPort->MacTab.MacTabList;
            pNextMacEntry = (PMAC_TABLE_ENTRY)(pHeader->Head);
            while (pNextMacEntry != NULL)
            {
                pMacEntry = (PMAC_TABLE_ENTRY)pNextMacEntry;
                
                if(pMacEntry == NULL)
                {
                    break; 
                }
                
                if((pMacEntry->WlanIdxRole == ROLE_WLANIDX_BSSID) ||(pMacEntry->WlanIdxRole == ROLE_WLANIDX_MBCAST))
                {
                    pNextMacEntry = pNextMacEntry->Next;   
                    pMacEntry = NULL;
                    continue; 
                }

                if ((pMacEntry->bAPSDFlagSPStart == TRUE) && UAPSD_MR_IS_ENTRY_ACCESSIBLE(pMacEntry))
                {
                    ApUapsdDeQueueUAPSDPacket(pAd, pPort, BulkOutPipeId, pMacEntry);
                }   
                
                pNextMacEntry = pNextMacEntry->Next;
                pMacEntry = NULL;
            }
        }
#endif  
    }
    else
    {
        pAd->pHifCfg->BulkOutCompleteOther++;
    
        DBGPRINT_RAW(RT_DEBUG_ERROR, (">>BulkOutComplete=0x%x\n", pAd->pHifCfg->BulkOutComplete));
        DBGPRINT_RAW(RT_DEBUG_ERROR, (">>BulkOutCompleteCancel=0x%x, BulkOutCompleteOther=0x%x\n", pAd->pHifCfg->BulkOutCompleteCancel, pAd->pHifCfg->BulkOutCompleteOther));
        NdisAcquireSpinLock(&pAd->pTxCfg->TxContextQueueLock[BulkOutPipeId]);
        if ((!MT_TEST_FLAG(pAd, fRTMP_ADAPTER_RESET_IN_PROGRESS)) &&
            (!MT_TEST_FLAG(pAd, fRTMP_ADAPTER_HALT_IN_PROGRESS)) &&
            (!MT_TEST_FLAG(pAd, fRTMP_ADAPTER_NIC_NOT_EXIST)) &&
            (!MT_TEST_FLAG(pAd, fRTMP_ADAPTER_SUSPENDED)) &&
            ((pAd->pHifCfg->BulkFailParm.BulkResetPipeid & (1<<BulkOutPipeId)) != (1<<BulkOutPipeId)))               
//          (!MT_TEST_FLAG(pAd, fRTMP_ADAPTER_BULKOUT_RESET)))
        {
            RTMP_BULKOUT_RESET_FRAME_TYPE typeBulkOutRest = BULKOUT_RESET_TYPE_DATA;
            typeBulkOutRest|=(BulkOutPipeId<<4);
            if (!MT_TEST_FLAG(pAd, fRTMP_ADAPTER_BULKOUT_RESET))
            {
    
                DBGPRINT_RAW(RT_DEBUG_ERROR, (">>fRTMP_ADAPTER_BULKOUT_RESET BulkOutPipeId = %d\n",BulkOutPipeId)); 
            MT_SET_FLAG(pAd, fRTMP_ADAPTER_BULKOUT_RESET);            
                DBGPRINT(RT_DEBUG_TRACE,("%s: LINE %d, enqueue MTK_OID_N5_QUERY_USB_RESET_BULK_OUT\n",__FUNCTION__,__LINE__));
                MTEnqueueInternalCmd(pAd, pPort,  MT_CMD_USB_RESET_BULK_OUT, &typeBulkOutRest, sizeof(RTMP_BULKOUT_RESET_FRAME_TYPE));
            }           
            pAd->pHifCfg->BulkFailParm.BulkResetPipeid |= (1<<BulkOutPipeId);
//          pAd->pHifCfg->BulkFailParm.IrpIdx[BulkOutPipeId] = IrpIdx;                   
            pTxContext->IRPPending[IrpIdx] = FALSE;
            //pTxContext->AnotherPendingTx |= (1 << IrpIdx);
#if 0
            pTxContext->AnotherPendingTx = (0x10 << IrpIdx);
#else
            pTxContext->AnotherPendingTx = (0x01&IrpIdx);
            pTxContext->AnotherPendingTx |= 0x10;
#endif
            NdisReleaseSpinLock(&pAd->pTxCfg->TxContextQueueLock[BulkOutPipeId]);
            MT_DEC_REF(&pAd->pHifCfg->PendingTx[BulkOutPipeId]);           
            return;
        }
        NdisReleaseSpinLock(&pAd->pTxCfg->TxContextQueueLock[BulkOutPipeId]);

    }

    //
    // bInUse = TRUE, means some process are filling TX data, after that must turn on ReadyToBulkOut
    // ReadyToBulkOut = TRUE, means the TX data are waiting for bulk out. 
    //
    NdisAcquireSpinLock(&pAd->pTxCfg->TxContextQueueLock[BulkOutPipeId]);
    if ((pTxContext->ENextBulkOutPosition != pTxContext->CurWritePosition) && 
        (pTxContext->ENextBulkOutPosition != (pTxContext->CurWritePosition+8)) && !MTUSB_TEST_BULK_FLAG(pAd, (fRTUSB_BULK_OUT_DATA_FRAG << BulkOutPipeId)))
    {
        // Indicate There is data avaliable
        MTUSB_SET_BULK_FLAG(pAd, (fRTUSB_BULK_OUT_DATA_NORMAL << BulkOutPipeId));
    }
    pTxContext->IRPPending[IrpIdx] = FALSE;
    pTxContext->BulkoutEPCountDown = 1;     
    NdisReleaseSpinLock(&pAd->pTxCfg->TxContextQueueLock[BulkOutPipeId]);
    
    // Always call Bulk routine, even reset bulk.
    // The protectioon of rest bulk should be in BulkOut routine
    N6USBKickBulkOut(pAd);

    MT_DEC_REF(&pAd->pHifCfg->PendingTx[BulkOutPipeId]);
    DBGPRINT_RAW(RT_DEBUG_INFO, ("<---2RTUSBBulkOutDataPacketComplete\n"));
}

/*
    ========================================================================
    
    Routine Description:

    Arguments:

    Return Value:

    IRQL = 
    
    Note: NULL frame use BulkOutPipeId = 0
    
    ========================================================================
*/
VOID    N6USBBulkOutNullFrame(
    IN  PMP_ADAPTER   pAd)
{   
    PTX_CONTEXT pNullContext = &(pAd->pHifCfg->NullContext);
    WDFREQUEST      writeRequest;
    PUSB_WRITE_REQ_CONTEXT writeContext;  

    if (MT_TEST_FLAG(pAd, fRTMP_ADAPTER_BSS_SCAN_IN_PROGRESS))
    {
        return;
    }
    
    // no bulk out for radio off    
    if (MT_TEST_FLAG(pAd, fRTMP_ADAPTER_RADIO_OFF))
    {
        pNullContext->InUse = FALSE;
        return;
    }

    // Increase Total transmit byte counter
    pAd->Counter.MTKCounters.TransmittedByteCount +=  pNullContext->BulkOutSize;

    DBGPRINT_RAW(RT_DEBUG_TRACE, ("--->N6USBBulkOutNullFrame \n"));
    
    // Init Tx context descriptor
    N6USBInitTxDesc(pAd, pNullContext, ENDPOINT_OUT_8, N6USBBulkOutNullFrameComplete);   
    writeRequest = pNullContext->WriteRequestArray[0];
    writeContext = GetWriteRequestContext(writeRequest);
    //NdisReleaseSpinLock(&pAd->pHifCfg->BulkOutMlmeLock);
    if (WdfRequestSend(writeRequest, writeContext->IoTarget, &G_RequestOptions ) == FALSE) 
    {   // Failure - Return request
        DBGPRINT(RT_DEBUG_ERROR, ("N6USBBulkOutDataPacket send failed\n"));     
    }
    //NdisAcquireSpinLock(&pAd->pHifCfg->BulkOutMlmeLock);
    DBGPRINT_RAW(RT_DEBUG_TRACE, ("<---N6USBBulkOutNullFrame \n"));
}

/*
    ========================================================================
    
    Routine Description:

    Arguments:

    Return Value:

    IRQL <=  DISPATCH_LEVEL
    
    Note: NULL frame use BulkOutPipeId = 0
    
    ========================================================================
*/
VOID    N6USBBulkOutNullFrameComplete(
    IN WDFREQUEST  Request,
    IN WDFIOTARGET  Target,
    IN PWDF_REQUEST_COMPLETION_PARAMS  CompletionParams,
    IN WDFCONTEXT  Context)
{
    NTSTATUS            status;
    PMP_ADAPTER   pAd;
    PTX_CONTEXT     pNullContext;
    
    PUSB_WRITE_REQ_CONTEXT  writeContext;
    
    writeContext = (PUSB_WRITE_REQ_CONTEXT)Context;
    pNullContext =(PTX_CONTEXT ) writeContext->pTxContext;
    status = CompletionParams->IoStatus.Status;
#pragma prefast(suppress: __WARNING_DEREF_NULL_PTR, " should not be NULL")              
    pAd = pNullContext->pAd;

    DBGPRINT_RAW(RT_DEBUG_INFO, ("--->N6USBBulkOutNullFrameComplete\n"));

    // Reset Null frame context flags
    pNullContext->InUse = FALSE;
    pNullContext->IRPPending = FALSE;
    ReturnWriteRequest(pAd,Request);

//**
/*  
    if (status == STATUS_SUCCESS)
    {
        // Don't worry about the queue is empty or not, this function will check itself
        KeSetEvent(&pAd->DequeueDataEvent, 0, FALSE);
    }
    else if (status != STATUS_CANCELLED)
    {
        if ((!MT_TEST_FLAG(pAd, fRTMP_ADAPTER_RESET_IN_PROGRESS)) &&
            (!MT_TEST_FLAG(pAd, fRTMP_ADAPTER_HALT_IN_PROGRESS)) &&
            ((pAd->pHifCfg->BulkFailParm.BulkResetPipeid & 0x01) != 0x01))       
//          (!MT_TEST_FLAG(pAd, fRTMP_ADAPTER_BULKOUT_RESET)))
        {
            RTMP_BULKOUT_RESET_FRAME_TYPE typeBulkOutRest = BULKOUT_RESET_TYPE_NULLFRAME;       
            DBGPRINT_RAW(RT_DEBUG_ERROR, ("Bulk Out Null Frame Failed\n"));
            MT_SET_FLAG(pAd, fRTMP_ADAPTER_BULKOUT_RESET);
            DBGPRINT(RT_DEBUG_TRACE,("%s: LINE %d, enqueue MTK_OID_N5_QUERY_USB_RESET_BULK_OUT\n",__FUNCTION__,__LINE__));
            MTEnqueueInternalCmd(pAd, pPort, MTK_OID_N5_QUERY_USB_RESET_BULK_OUT, &typeBulkOutRest, sizeof(RTMP_BULKOUT_RESET_FRAME_TYPE));
            pAd->pHifCfg->BulkFailParm.BulkResetPipeid |= 1;         
        }
    }
*/

    NdisAcquireSpinLock(&pAd->pHifCfg->BulkOutMlmeLock);
    pAd->pHifCfg->bBulkOutMlme = FALSE;
    pAd->pHifCfg->MlmePendingCnt --;
    DBGPRINT(RT_DEBUG_TRACE, ("===>%s() Bulk out, pAd->pHifCfg->MlmePendingCnt = %d\n", __FUNCTION__, pAd->pHifCfg->MlmePendingCnt));
    NdisReleaseSpinLock(&pAd->pHifCfg->BulkOutMlmeLock);
    
    // Always call Bulk routine, even reset bulk.
    // The protectioon of rest bulk should be in BulkOut routine
//**    N6USBKickBulkOut(pAd);
    DBGPRINT_RAW(RT_DEBUG_TRACE, ("<---%s   MlmePendingCnt  %d\n",  __FUNCTION__ ,pAd->pHifCfg->MlmePendingCnt ));    
}

/*
    ========================================================================
    
    Routine Description:

    Arguments:

    Return Value:

    IRQL <=  DISPATCH_LEVEL
    
    Note: NULL frame use BulkOutPipeId = 0
    
    ========================================================================
*/
VOID    MT7603BulkOutNullFrameComplete(
    IN WDFREQUEST  Request,
    IN WDFIOTARGET  Target,
    IN PWDF_REQUEST_COMPLETION_PARAMS  CompletionParams,
    IN WDFCONTEXT  Context)
{
    NTSTATUS            status;
    PMP_ADAPTER   pAd;
    PTX_CONTEXT     pEPNullContext;
    
    PUSB_WRITE_REQ_CONTEXT  writeContext;
    
    writeContext = (PUSB_WRITE_REQ_CONTEXT)Context;
    pEPNullContext =(PTX_CONTEXT ) writeContext->pTxContext;
    status = CompletionParams->IoStatus.Status;
#pragma prefast(suppress: __WARNING_DEREF_NULL_PTR, " should not be NULL")              
    pAd = pEPNullContext->pAd;

    DBGPRINT_RAW(RT_DEBUG_TRACE, ("--->MT7603BulkOutNullFrameComplete %d\n", (status == STATUS_SUCCESS)));

    // Reset Null frame context flags
    pEPNullContext->InUse = FALSE;
    pEPNullContext->IRPPending = FALSE;
    ReturnWriteRequest(pAd,Request);

//**
/*  
    if (status == STATUS_SUCCESS)
    {
        // Don't worry about the queue is empty or not, this function will check itself
        KeSetEvent(&pAd->DequeueDataEvent, 0, FALSE);
    }
    else if (status != STATUS_CANCELLED)
    {
        if ((!MT_TEST_FLAG(pAd, fRTMP_ADAPTER_RESET_IN_PROGRESS)) &&
            (!MT_TEST_FLAG(pAd, fRTMP_ADAPTER_HALT_IN_PROGRESS)) &&
            ((pAd->pHifCfg->BulkFailParm.BulkResetPipeid & 0x01) != 0x01))       
//          (!MT_TEST_FLAG(pAd, fRTMP_ADAPTER_BULKOUT_RESET)))
        {
            RTMP_BULKOUT_RESET_FRAME_TYPE typeBulkOutRest = BULKOUT_RESET_TYPE_NULLFRAME;       
            DBGPRINT_RAW(RT_DEBUG_ERROR, ("Bulk Out Null Frame Failed\n"));
            MT_SET_FLAG(pAd, fRTMP_ADAPTER_BULKOUT_RESET);
            DBGPRINT(RT_DEBUG_TRACE,("%s: LINE %d, enqueue MTK_OID_N5_QUERY_USB_RESET_BULK_OUT\n",__FUNCTION__,__LINE__));
            MTEnqueueInternalCmd(pAd, pPort, MTK_OID_N5_QUERY_USB_RESET_BULK_OUT, &typeBulkOutRest, sizeof(RTMP_BULKOUT_RESET_FRAME_TYPE));
            pAd->pHifCfg->BulkFailParm.BulkResetPipeid |= 1;         
        }
    }
*/

    NdisAcquireSpinLock(&pAd->pHifCfg->MT7603EPLock[pEPNullContext->EndPointIndex]);
    pAd->pHifCfg->bEndPointUsed[pEPNullContext->EndPointIndex] = FALSE;
    NdisReleaseSpinLock(&pAd->pHifCfg->MT7603EPLock[pEPNullContext->EndPointIndex]);
    
    // Always call Bulk routine, even reset bulk.
    // The protectioon of rest bulk should be in BulkOut routine
//**    N6USBKickBulkOut(pAd);
    DBGPRINT_RAW(RT_DEBUG_TRACE, ("<---%s   MlmePendingCnt  %d\n",  __FUNCTION__ ,pAd->pHifCfg->MlmePendingCnt ));   
}

/*
    ========================================================================
    
    Routine Description:

    Arguments:

    Return Value:

    IRQL = 
    
    Note: MLME use BulkOutPipeId = 0
    
    ========================================================================
*/
#if 1
VOID    N6USBBulkOutMLMEPacket(
    IN  PMP_ADAPTER   pAd)
{
    PTX_CONTEXT     pMLMEContext;
    WDFREQUEST      writeRequest;
    PUSB_WRITE_REQ_CONTEXT writeContext;
    UCHAR           Index = pAd->pHifCfg->PrioRingFirstIndex;
    
    // no bulk out for radio off    
    if (MT_TEST_FLAG(pAd, fRTMP_ADAPTER_RADIO_OFF))
        return;
    
    pMLMEContext = &pAd->pHifCfg->MLMEContext[Index];
    
#if _WIN8_USB_SS_SUPPORTED
    NdisAcquireSpinLock(&pAd->Ss.SsLock);
    if (MT_TEST_FLAG(pAd, fRTMP_ADAPTER_SUSPENDED))
    {
        NdisReleaseSpinLock(&pAd->Ss.SsLock);
        return;
    }
#endif
    
    if (pMLMEContext->ReadyToBulkOut == FALSE)
    {
#if _WIN8_USB_SS_SUPPORTED
        NdisReleaseSpinLock(&pAd->Ss.SsLock);
#endif
        return;
    }

    pMLMEContext->ReadyToBulkOut = FALSE;
    
#if _WIN8_USB_SS_SUPPORTED
    NdisReleaseSpinLock(&pAd->Ss.SsLock);
#endif

    NdisAcquireSpinLock(&pAd->pHifCfg->MLMEQLock);
    pAd->pHifCfg->PrioRingFirstIndex++;
    if (pAd->pHifCfg->PrioRingFirstIndex >= PRIO_RING_SIZE)
    {
        pAd->pHifCfg->PrioRingFirstIndex = 0;
    }   
    NdisReleaseSpinLock(&pAd->pHifCfg->MLMEQLock);

    if (0)
    {
        int i = 0;
        PUCHAR p = (PUCHAR)&pMLMEContext->TransferBuffer->WirelessPacket[0];
        for (i = 0; i < (int)pMLMEContext->BulkOutSize; i = i + 8)
        {
            DBGPRINT_RAW(RT_DEBUG_ERROR, ("[%d] %x %x %x %x %x %x %x %x\n", i, p[i], p[i+1], p[i+2], p[i+3], p[i+4], p[i+5], p[i+6], p[i+7]));
        }
    }

    // Increase Total transmit byte counter
    pAd->Counter.MTKCounters.TransmittedByteCount +=  pMLMEContext->BulkOutSize;

    DBGPRINT(RT_DEBUG_INFO, ("RTUSBBulkOutMLME::PrioRingFirstIndex = %d, PrioRingTxCnt = %d, PopMgmtIndex = %d, PushMgmtIndex = %d, NextMLMEIndex = %d\n", 
            pAd->pHifCfg->PrioRingFirstIndex, 
            pAd->pHifCfg->PrioRingTxCnt, pAd->pHifCfg->PopMgmtIndex, pAd->pHifCfg->PushMgmtIndex, pAd->pHifCfg->NextMLMEIndex));

    // Init Tx context descriptor
    N6USBInitTxDesc(pAd, pMLMEContext, ENDPOINT_OUT_8, N6USBBulkOutMLMEPacketComplete);    
    
    writeRequest = pMLMEContext->WriteRequestArray[0];
    writeContext = GetWriteRequestContext(writeRequest);

#if _WIN8_USB_SS_SUPPORTED
    DBGPRINT(RT_DEBUG_INFO, ("110427 bulkout %u\n", pMLMEContext->Index));
#endif
    //NdisReleaseSpinLock(&pAd->pHifCfg->BulkOutMlmeLock);
    if (WdfRequestSend(writeRequest, writeContext->IoTarget, &G_RequestOptions ) == FALSE) 
    {   // Failure - Return request
        DBGPRINT(RT_DEBUG_ERROR, ("N6USBBulkOutDataPacket send failed\n"));     
    }   
    //NdisAcquireSpinLock(&pAd->pHifCfg->BulkOutMlmeLock);
    DBGPRINT_RAW(RT_DEBUG_INFO, ("<---N6USBBulkOutMLMEPacket \n"));
}
#else
VOID    N6USBBulkOutMLMEPacket(
    IN  PMP_ADAPTER   pAd,
    IN  UCHAR           Index)
{
    PTX_CONTEXT     pMLMEContext;
    WDFREQUEST      writeRequest;
    PUSB_WRITE_REQ_CONTEXT writeContext;  
    
    // no bulk out for radio off    
    if (MT_TEST_FLAG(pAd, fRTMP_ADAPTER_RADIO_OFF))
        return;
    
    pMLMEContext = &pAd->pHifCfg->MLMEContext[Index];
    
#if _WIN8_USB_SS_SUPPORTED
    NdisAcquireSpinLock(&pAd->Ss.SsLock);
    if (MT_TEST_FLAG(pAd, fRTMP_ADAPTER_SUSPENDED))
    {
        NdisReleaseSpinLock(&pAd->Ss.SsLock);
        return;
    }
#endif
    
    if (pMLMEContext->ReadyToBulkOut == FALSE)
    {
#if _WIN8_USB_SS_SUPPORTED
        NdisReleaseSpinLock(&pAd->Ss.SsLock);
#endif
        return;
    }

    pMLMEContext->ReadyToBulkOut = FALSE;
    
#if _WIN8_USB_SS_SUPPORTED
    NdisReleaseSpinLock(&pAd->Ss.SsLock);
#endif

   if (0)
   {
        int i = 0;
        PUCHAR p = (PUCHAR)&pMLMEContext->TransferBuffer->WirelessPacket[0];
        for (i = 0; i < (int)pMLMEContext->BulkOutSize; i = i + 8)
        {
            DBGPRINT_RAW(RT_DEBUG_ERROR, ("[%d] %x %x %x %x %x %x %x %x\n", i, p[i], p[i+1], p[i+2], p[i+3], p[i+4], p[i+5], p[i+6], p[i+7]));
        }
    }

    // Increase Total transmit byte counter
    pAd->Counter.MTKCounters.TransmittedByteCount +=  pMLMEContext->BulkOutSize;

    DBGPRINT(RT_DEBUG_INFO, ("RTUSBBulkOutMLME::PrioRingFirstIndex = %d, PrioRingTxCnt = %d, PopMgmtIndex = %d, PushMgmtIndex = %d, NextMLMEIndex = %d\n", 
            pAd->pHifCfg->PrioRingFirstIndex, 
            pAd->pHifCfg->PrioRingTxCnt, pAd->pHifCfg->PopMgmtIndex, pAd->pHifCfg->PushMgmtIndex, pAd->pHifCfg->NextMLMEIndex));

    // Init Tx context descriptor
    N6USBInitTxDesc(pAd, pMLMEContext, ENDPOINT_OUT_8, N6USBBulkOutMLMEPacketComplete);    
    
    writeRequest = pMLMEContext->WriteRequestArray[0];
    writeContext = GetWriteRequestContext(writeRequest);

#if _WIN8_USB_SS_SUPPORTED
    DBGPRINT(RT_DEBUG_INFO, ("110427 bulkout %u\n", pMLMEContext->Index));
#endif
    NdisReleaseSpinLock(&pAd->pHifCfg->BulkOutMlmeLock);
    if (WdfRequestSend(writeRequest, writeContext->IoTarget, &G_RequestOptions ) == FALSE) 
    {   // Failure - Return request
        DBGPRINT(RT_DEBUG_ERROR, ("N6USBBulkOutDataPacket send failed\n"));     
    }   
    NdisAcquireSpinLock(&pAd->pHifCfg->BulkOutMlmeLock);
    DBGPRINT_RAW(RT_DEBUG_INFO, ("<---N6USBBulkOutMLMEPacket \n"));
}
#endif

/*
    ========================================================================
    
    Routine Description:

    Arguments:

    Return Value:

    IRQL <=  DISPATCH_LEVEL
    
    Note: MLME use BulkOutPipeId = 0
    
    ========================================================================
*/
VOID    N6USBBulkOutMLMEPacketComplete(
        IN WDFREQUEST  Request,
    IN WDFIOTARGET  Target,
    IN PWDF_REQUEST_COMPLETION_PARAMS  CompletionParams,
    IN WDFCONTEXT  Context)
{
    NTSTATUS            status;
    PTX_CONTEXT         pMLMEContext;
    PMP_ADAPTER       pAd;
    PUSB_WRITE_REQ_CONTEXT  writeContext;
    PMP_PORT    pPort = NULL;

    writeContext = (PUSB_WRITE_REQ_CONTEXT)Context;
    pMLMEContext =(PTX_CONTEXT ) writeContext->pTxContext;
#if _WIN8_USB_SS_SUPPORTED
    DBGPRINT(RT_DEBUG_TRACE, ("110427 complete %u\n", pMLMEContext->Index));
#endif

    //
    // Check status at first, before access pAd.
    // Since WDF might return STATUS_WDF_DEVICE_REMOVED_NOT_SENT due to surprise remove when calling WdfDriverMiniportUnload
    // Meanwhile, all pAd resource has been freed at DriverUnload and the pAd can't be touch in this case.
    //
    status = CompletionParams->IoStatus.Status;
    if (status == STATUS_WDF_DEVICE_REMOVED_NOT_SENT)
    {
        DBGPRINT(RT_DEBUG_ERROR, ("%s: device has been removed. Status=0x%08x, TransferLength=0x%08x\n", status, CompletionParams->IoStatus.Information));     
        return;
    }
    
#pragma prefast(suppress: __WARNING_DEREF_NULL_PTR, " should not be NULL")
    pAd = pMLMEContext->pAd;
    
    NdisAcquireSpinLock(&pAd->pHifCfg->MLMEQLock);
    pAd->pHifCfg->PrioRingTxCnt--;
    if (pAd->pHifCfg->PrioRingTxCnt < 0)
        pAd->pHifCfg->PrioRingTxCnt = 0;
    NdisReleaseSpinLock(&pAd->pHifCfg->MLMEQLock);

    DBGPRINT(RT_DEBUG_INFO, ("N6USBBulkOutMLMEPacketComplete::PrioRingFirstIndex = %d, PrioRingTxCnt = %d, PopMgmtIndex = %d, PushMgmtIndex = %d, NextMLMEIndex = %d\n", 
            pAd->pHifCfg->PrioRingFirstIndex, 
            pAd->pHifCfg->PrioRingTxCnt, pAd->pHifCfg->PopMgmtIndex, pAd->pHifCfg->PushMgmtIndex, pAd->pHifCfg->NextMLMEIndex));
    
    // Reset MLME context flags 
    pMLMEContext->InUse         = FALSE;
    pMLMEContext->IRPPending    = FALSE;
    ReturnWriteRequest(pAd,Request);
    
    if (status == STATUS_SUCCESS)
    {
        // Don't worry about the queue is empty or not, this function will check itself
        NdisCommonUsbDequeueMLMEPacket(pAd);
    }
    else if (status != STATUS_CANCELLED)
    {
        if ((!MT_TEST_FLAG(pAd, fRTMP_ADAPTER_RESET_IN_PROGRESS)) &&
            (!MT_TEST_FLAG(pAd, fRTMP_ADAPTER_HALT_IN_PROGRESS)) &&
            (!MT_TEST_FLAG(pAd, fRTMP_ADAPTER_NIC_NOT_EXIST)) &&
            (!MT_TEST_FLAG(pAd, fRTMP_ADAPTER_SUSPENDED)) &&
            ((pAd->pHifCfg->BulkFailParm.BulkResetPipeid & 0x01) != 0x01))           
//          (!MT_TEST_FLAG(pAd, fRTMP_ADAPTER_BULKOUT_RESET)))
        {
            RTMP_BULKOUT_RESET_FRAME_TYPE typeBulkOutRest = BULKOUT_RESET_TYPE_MLME;            
            DBGPRINT_RAW(RT_DEBUG_ERROR, ("Bulk Out MLME Failed. Status = %x\n", status));
            MT_SET_FLAG(pAd, fRTMP_ADAPTER_BULKOUT_RESET);
            DBGPRINT(RT_DEBUG_TRACE,("%s: LINE %d, enqueue MTK_OID_N5_QUERY_USB_RESET_BULK_OUT\n",__FUNCTION__,__LINE__));
            MTEnqueueInternalCmd(pAd, pPort,  MT_CMD_USB_RESET_BULK_OUT, &typeBulkOutRest, sizeof(RTMP_BULKOUT_RESET_FRAME_TYPE));
            pAd->pHifCfg->BulkFailParm.BulkResetPipeid |= 1;         
        }
    }
    

    //pMLMEContext = &pAd->pHifCfg->MLMEContext[pAd->pHifCfg->PrioRingFirstIndex];
    if ( (pAd->pHifCfg->PrioRingTxCnt >= 1)/* && (pMLMEContext->ReadyToBulkOut == TRUE)*/)
        XmitSendMlmeCmdPkt(pAd, TXPKT_MLME_FRAME);

    NdisAcquireSpinLock(&pAd->pHifCfg->BulkOutMlmeLock);
    pAd->pHifCfg->bBulkOutMlme = FALSE;
    pAd->pHifCfg->MlmePendingCnt --;
    DBGPRINT(RT_DEBUG_TRACE, ("===>%s() Bulk out, pAd->pHifCfg->MlmePendingCnt = %d\n", __FUNCTION__, pAd->pHifCfg->MlmePendingCnt));
    NdisReleaseSpinLock(&pAd->pHifCfg->BulkOutMlmeLock);

    DBGPRINT_RAW(RT_DEBUG_TRACE, ("<---%s   MlmePendingCnt  %d\n",  __FUNCTION__ ,pAd->pHifCfg->MlmePendingCnt ));  
}

/*
    ========================================================================
    
    Routine Description:

    Arguments:

    Return Value:

    IRQL = 
    
    Note: PsPoll use BulkOutPipeId = 0
    
    ========================================================================
*/
VOID    N6USBBulkOutPsPoll(
    IN  PMP_ADAPTER   pAd)
{
    PTX_CONTEXT pPsPollContext = &(pAd->pHifCfg->PsPollContext);
    WDFREQUEST      writeRequest;
    PUSB_WRITE_REQ_CONTEXT writeContext;  

    DBGPRINT_RAW(RT_DEBUG_TRACE, ("N6USBBulkOutPsPoll ---> \n"));
    
    // Init Tx context descriptor
    N6USBInitTxDesc(pAd, pPsPollContext, ENDPOINT_OUT_8, N6USBBulkOutPsPollComplete);
    
    writeRequest = pPsPollContext->WriteRequestArray[0];
    writeContext = GetWriteRequestContext(writeRequest);
    //NdisReleaseSpinLock(&pAd->pHifCfg->BulkOutMlmeLock);
    if (WdfRequestSend(writeRequest, writeContext->IoTarget, &G_RequestOptions ) == FALSE) 
    {   // Failure - Return request
        DBGPRINT(RT_DEBUG_ERROR, ("N6USBBulkOutDataPacket send failed\n"));     
    }   
    //NdisAcquireSpinLock(&pAd->pHifCfg->BulkOutMlmeLock);
    DBGPRINT_RAW(RT_DEBUG_INFO, ("<---N6USBBulkOutPsPoll \n"));
}

/*
    ========================================================================
    
    Routine Description:

    Arguments:

    Return Value:

    IRQL <=  DISPATCH_LEVEL
    
    Note: PsPoll use BulkOutPipeId = 0
    
    ========================================================================
*/
VOID N6USBBulkOutPsPollComplete(
        IN WDFREQUEST  Request,
    IN WDFIOTARGET  Target,
    IN PWDF_REQUEST_COMPLETION_PARAMS  CompletionParams,
    IN WDFCONTEXT  Context)
{
    PMP_ADAPTER   pAd;
    NTSTATUS            status;
    PTX_CONTEXT     pPsPollContext;
    PUSB_WRITE_REQ_CONTEXT  writeContext;
    PMP_PORT    pPort = NULL;

    writeContext = (PUSB_WRITE_REQ_CONTEXT)Context;
    pPsPollContext =(PTX_CONTEXT ) writeContext->pTxContext;
    //
    // Check status at first. 
    // Since WDF might return STATUS_WDF_DEVICE_REMOVED_NOT_SENT due to surprise remove when calling WdfDriverMiniportUnload
    // Meanwhile, all pAd resource has been freed at DriverUnload and the pAd can't be touch in this case.
    //
    status = CompletionParams->IoStatus.Status;
    if (status == STATUS_WDF_DEVICE_REMOVED_NOT_SENT)
    {
        DBGPRINT(RT_DEBUG_ERROR, ("%s: device has been removed. Status=0x%08x, TransferLength=0x%08x\n", status, CompletionParams->IoStatus.Information));     
        return;
    }

#pragma prefast(suppress: __WARNING_DEREF_NULL_PTR, " should not be NULL")
    pAd = pPsPollContext->pAd;

    
    // Reset PsPoll context flags   
    pPsPollContext->InUse       = FALSE;
    pPsPollContext->IRPPending  = FALSE;
    pPort = pAd->PortList[FXXK_PORT_0];
    
    if (status == STATUS_SUCCESS)
    {
        // Don't worry about the queue is empty or not, this function will check itself
        NdisSetEvent(&(GET_THREAD_EVENT(&pAd->pHifCfg->hDequeueDataThread)));
    }
    else if (status != STATUS_CANCELLED)
    {
        if ((!MT_TEST_FLAG(pAd, fRTMP_ADAPTER_RESET_IN_PROGRESS)) &&
            (!MT_TEST_FLAG(pAd, fRTMP_ADAPTER_HALT_IN_PROGRESS)) &&
            (!MT_TEST_FLAG(pAd, fRTMP_ADAPTER_NIC_NOT_EXIST)) &&
            ((pAd->pHifCfg->BulkFailParm.BulkResetPipeid & 0x01) != 0x01))               
//          (!MT_TEST_FLAG(pAd, fRTMP_ADAPTER_BULKOUT_RESET)))
        {
            RTMP_BULKOUT_RESET_FRAME_TYPE typeBulkOutRest = BULKOUT_RESET_TYPE_MLME;
            DBGPRINT_RAW(RT_DEBUG_ERROR, ("Bulk Out PSPoll Failed\n"));
            MT_SET_FLAG(pAd, fRTMP_ADAPTER_BULKOUT_RESET);
            DBGPRINT(RT_DEBUG_TRACE,("%s: LINE %d, enqueue MTK_OID_N5_QUERY_USB_RESET_BULK_OUT\n",__FUNCTION__,__LINE__));
            MTEnqueueInternalCmd(pAd, pPort, MT_CMD_USB_RESET_BULK_OUT, &typeBulkOutRest, sizeof(RTMP_BULKOUT_RESET_FRAME_TYPE));
            pAd->pHifCfg->BulkFailParm.BulkResetPipeid |= 1;         
        }
        DBGPRINT_RAW(RT_DEBUG_TRACE, ("--->N6USBBulkOutPsPollComplete STATUS_CANCELLED\n"));
    }

    NdisAcquireSpinLock(&pAd->pHifCfg->BulkOutMlmeLock);
    pAd->pHifCfg->bBulkOutMlme = FALSE;
    pAd->pHifCfg->MlmePendingCnt --;
    NdisReleaseSpinLock(&pAd->pHifCfg->BulkOutMlmeLock);

    DBGPRINT_RAW(RT_DEBUG_TRACE, ("<---%s   MlmePendingCnt  %d\n",  __FUNCTION__ ,pAd->pHifCfg->MlmePendingCnt ));  

}

/*
    ========================================================================

    Routine Description:

    Arguments:

    Return Value:

    IRQL = 
    
    Note: Cmd frame use BulkOutPipeId = 0
    
    ========================================================================
*/
VOID    N6USBBulkOutFwPkt(
    IN  PMP_ADAPTER   pAd)
{   
    PTX_CONTEXT pFwPktContext = &(pAd->pHifCfg->FwPktContext);
    WDFREQUEST      writeRequest;
    PUSB_WRITE_REQ_CONTEXT writeContext;  

    pFwPktContext->BulkOutSize += 4; //add 4 byte padding to end Aggregation

    // Increase Total transmit byte counter
    pAd->Counter.MTKCounters.TransmittedByteCount +=  pFwPktContext->BulkOutSize;

    DBGPRINT(RT_DEBUG_TRACE, ("--->N6USBBulkOutFwPkt \n"));
    
    // Init Tx context descriptor
    N6USBInitTxDesc(pAd, pFwPktContext, ENDPOINT_OUT_4, N6USBBulkOutFwPktComplete);    
    
    writeRequest = pFwPktContext->WriteRequestArray[0];
    writeContext = GetWriteRequestContext(writeRequest);
   // NdisReleaseSpinLock(&pAd->pHifCfg->BulkOutMlmeLock);
    if (WdfRequestSend(writeRequest, writeContext->IoTarget, &G_RequestOptions ) == FALSE) 
    {   // Failure - Return request
        DBGPRINT(RT_DEBUG_ERROR, ("N6USBBulkOutFwPkt send failed\n"));     
    }
    //NdisAcquireSpinLock(&pAd->pHifCfg->BulkOutMlmeLock);
    DBGPRINT_RAW(RT_DEBUG_INFO, ("<---N6USBBulkOutFwPkt \n"));
}

/*
    ========================================================================
    
    Routine Description:

    Arguments:

    Return Value:

    IRQL <=  DISPATCH_LEVEL
    
    Note: Cmd frame use BulkOutPipeId = 0
    
    ========================================================================
*/
VOID    N6USBBulkOutFwPktComplete(
    IN WDFREQUEST  Request,
    IN WDFIOTARGET  Target,
    IN PWDF_REQUEST_COMPLETION_PARAMS  CompletionParams,
    IN WDFCONTEXT  Context)
{
    NTSTATUS            status;
    PMP_ADAPTER   pAd;
    PTX_CONTEXT     pFwPktContext;
    PMP_PORT        pPort;
    
    PUSB_WRITE_REQ_CONTEXT  writeContext;
        
    writeContext = (PUSB_WRITE_REQ_CONTEXT)Context;
    pFwPktContext =(PTX_CONTEXT ) writeContext->pTxContext;
    //
    // Check status at first. 
    // Since WDF might return STATUS_WDF_DEVICE_REMOVED_NOT_SENT due to surprise remove when calling WdfDriverMiniportUnload
    // Meanwhile, all pAd resource has been freed at DriverUnload and the pAd can't be touch in this case.
    //
    status = CompletionParams->IoStatus.Status;
    if (status == STATUS_WDF_DEVICE_REMOVED_NOT_SENT)
    {
        DBGPRINT(RT_DEBUG_ERROR, ("%s: device has been removed. Status=0x%08x, TransferLength=0x%08x\n", status, CompletionParams->IoStatus.Information));     
        return;
    }

#pragma prefast(suppress: __WARNING_DEREF_NULL_PTR, " should not be NULL")              

    if ((pFwPktContext == NULL) || (pFwPktContext->pAd == NULL))  // Avoid 0x50 BSOD on Win7. packet command complete return after unload driver.
        return;

    pAd = pFwPktContext->pAd;
    pPort = pAd->PortList[FXXK_PORT_0];

    DBGPRINT_RAW(RT_DEBUG_TRACE, ("--->N6USBBulkOutFwPktComplete\n"));

    // Reset PktCmd context flags
    pFwPktContext->InUse = FALSE;
    pFwPktContext->IRPPending = FALSE;
    ReturnWriteRequest(pAd,Request);
    
    if (pAd->HwCfg.bLoadingFW == TRUE)
    {
        if ((status != STATUS_SUCCESS) && (status != STATUS_CANCELLED))
        {
            pAd->HwCfg.bLoadingFW = FALSE;
        }
        DBGPRINT_RAW(RT_DEBUG_TRACE, ("Kick Load firmware event\n"));
        NdisSetEvent(&(pAd->HwCfg.LoadFWEvent));
    }
    
    if (status == STATUS_SUCCESS)
    {
        // Don't worry about the queue is empty or not, this function will check itself
        //KeSetEvent(&pAd->DequeueDataEvent, 0, FALSE);
    }
    else if (status != STATUS_CANCELLED)
    {
        if ((!MT_TEST_FLAG(pAd, fRTMP_ADAPTER_RESET_IN_PROGRESS)) &&
            (!MT_TEST_FLAG(pAd, fRTMP_ADAPTER_HALT_IN_PROGRESS)) &&
            ((pAd->pHifCfg->BulkFailParm.BulkResetPipeid & 0x01) != 0x01))       
//          (!MT_TEST_FLAG(pAd, fRTMP_ADAPTER_BULKOUT_RESET)))
        {
            RTMP_BULKOUT_RESET_FRAME_TYPE typeBulkOutRest = BULKOUT_RESET_TYPE_NULLFRAME;       
            DBGPRINT_RAW(RT_DEBUG_ERROR, ("Bulk Out Pkt Cmd Failed, status = 0x%x\n", status));
            MT_SET_FLAG(pAd, fRTMP_ADAPTER_BULKOUT_RESET);
            DBGPRINT(RT_DEBUG_TRACE,("%s: LINE %d, enqueue MTK_OID_N5_QUERY_USB_RESET_BULK_OUT\n",__FUNCTION__,__LINE__));
            MTEnqueueInternalCmd(pAd, pPort, MT_CMD_USB_RESET_BULK_OUT, &typeBulkOutRest, sizeof(RTMP_BULKOUT_RESET_FRAME_TYPE));
            pAd->pHifCfg->BulkFailParm.BulkResetPipeid |= 1;         
        }
    }

    NdisAcquireSpinLock(&pAd->pHifCfg->BulkOutMlmeLock);
    pAd->pHifCfg->bBulkOutMlme = FALSE;
    pAd->pHifCfg->MlmePendingCnt --;
    DBGPRINT(RT_DEBUG_TRACE, ("===>%s() Bulk out, pAd->pHifCfg->MlmePendingCnt = %d\n", __FUNCTION__, pAd->pHifCfg->MlmePendingCnt));
    NdisReleaseSpinLock(&pAd->pHifCfg->BulkOutMlmeLock);

    DBGPRINT_RAW(RT_DEBUG_INFO, ("<---N6USBBulkOutFwPktComplete\n"));  
}

/*
    ========================================================================

    Routine Description:

    Arguments:

    Return Value:

    IRQL = 
    
    Note: Cmd frame use BulkOutPipeId = 0
    
    ========================================================================
*/
VOID    N6USBBulkOutPktCmd(
    IN  PMP_ADAPTER   pAd)
{   
    PTX_CONTEXT pPktCmdContext = &(pAd->pHifCfg->PktCmdContext);
    WDFREQUEST      writeRequest;
    PUSB_WRITE_REQ_CONTEXT writeContext;  

    pPktCmdContext->BulkOutSize += 4; //add 4 byte padding to end Aggregation

    // Increase Total transmit byte counter
    pAd->Counter.MTKCounters.TransmittedByteCount +=  pPktCmdContext->BulkOutSize;

    DBGPRINT(RT_DEBUG_TRACE, ("--->N6USBBulkOutPktCmd \n"));
    
    // Init Tx context descriptor
    N6USBInitTxDesc(pAd, pPktCmdContext, ENDPOINT_OUT_8, N6USBBulkOutPktCmdComplete);    
    
    writeRequest = pPktCmdContext->WriteRequestArray[0];
    writeContext = GetWriteRequestContext(writeRequest);
    //NdisReleaseSpinLock(&pAd->pHifCfg->BulkOutMlmeLock);
    if (WdfRequestSend(writeRequest, writeContext->IoTarget, &G_RequestOptions ) == FALSE) 
    {   // Failure - Return request
        DBGPRINT(RT_DEBUG_ERROR, ("N6USBBulkOutPktCmd send failed\n"));     
    }
    //NdisAcquireSpinLock(&pAd->pHifCfg->BulkOutMlmeLock);
    DBGPRINT_RAW(RT_DEBUG_INFO, ("<---N6USBBulkOutPktCmd \n"));
}

/*
    ========================================================================
    
    Routine Description:

    Arguments:

    Return Value:

    IRQL <=  DISPATCH_LEVEL
    
    Note: Cmd frame use BulkOutPipeId = 0
    
    ========================================================================
*/
VOID    N6USBBulkOutPktCmdComplete(
    IN WDFREQUEST  Request,
    IN WDFIOTARGET  Target,
    IN PWDF_REQUEST_COMPLETION_PARAMS  CompletionParams,
    IN WDFCONTEXT  Context)
{
    NTSTATUS            status;
    PMP_ADAPTER   pAd;
    PTX_CONTEXT     pPktCmdContext;
    PMP_PORT    pPort =NULL;
    
    PUSB_WRITE_REQ_CONTEXT  writeContext;
        
    writeContext = (PUSB_WRITE_REQ_CONTEXT)Context;
    pPktCmdContext =(PTX_CONTEXT ) writeContext->pTxContext;
    //
    // Check status at first, before access pAd.
    // Since WDF might return STATUS_WDF_DEVICE_REMOVED_NOT_SENT due to surprise remove when calling WdfDriverMiniportUnload
    // Meanwhile, all pAd resource has been freed at DriverUnload and the pAd can't be touch in this case.
    //
    status = CompletionParams->IoStatus.Status;
    if (status == STATUS_WDF_DEVICE_REMOVED_NOT_SENT)
    {
        DBGPRINT(RT_DEBUG_ERROR, ("%s: device has been removed. Status=0x%08x, TransferLength=0x%08x\n", status, CompletionParams->IoStatus.Information));     
        return;
    }

#pragma prefast(suppress: __WARNING_DEREF_NULL_PTR, " should not be NULL")              

    if ((pPktCmdContext == NULL) || (pPktCmdContext->pAd == NULL))  // Avoid 0x50 BSOD on Win7. packet command complete return after unload driver.
        return;

    pAd = pPktCmdContext->pAd;
    pPort = pAd->PortList[FXXK_PORT_0];

    DBGPRINT_RAW(RT_DEBUG_TRACE, ("--->N6USBBulkOutPktCmdComplete\n"));

    // Reset PktCmd context flags
    NdisAcquireSpinLock(&pAd->pHifCfg->PktCmdLock);
    pPktCmdContext->InUse = FALSE;
    NdisReleaseSpinLock(&pAd->pHifCfg->PktCmdLock);
    
    pPktCmdContext->IRPPending = FALSE;
    ReturnWriteRequest(pAd,Request);
    
    if (status == STATUS_SUCCESS)
    {
        // Don't worry about the queue is empty or not, this function will check itself
        //KeSetEvent(&pAd->DequeueDataEvent, 0, FALSE);
    }
    else if (status != STATUS_CANCELLED)
    {
        if ((!MT_TEST_FLAG(pAd, fRTMP_ADAPTER_RESET_IN_PROGRESS)) &&
            (!MT_TEST_FLAG(pAd, fRTMP_ADAPTER_HALT_IN_PROGRESS)) &&
            ((pAd->pHifCfg->BulkFailParm.BulkResetPipeid & 0x01) != 0x01))       
//          (!MT_TEST_FLAG(pAd, fRTMP_ADAPTER_BULKOUT_RESET)))
        {
            RTMP_BULKOUT_RESET_FRAME_TYPE typeBulkOutRest = BULKOUT_RESET_TYPE_NULLFRAME;       
            DBGPRINT_RAW(RT_DEBUG_ERROR, ("Bulk Out Pkt Cmd Failed, status = 0x%x\n", status));
            MT_SET_FLAG(pAd, fRTMP_ADAPTER_BULKOUT_RESET);
            DBGPRINT(RT_DEBUG_TRACE,("%s: LINE %d, enqueue MTK_OID_N5_QUERY_USB_RESET_BULK_OUT\n",__FUNCTION__,__LINE__));
            MTEnqueueInternalCmd(pAd, pPort,  MT_CMD_USB_RESET_BULK_OUT, &typeBulkOutRest, sizeof(RTMP_BULKOUT_RESET_FRAME_TYPE));
            pAd->pHifCfg->BulkFailParm.BulkResetPipeid |= 1;         
        }
    }

    NdisAcquireSpinLock(&pAd->pHifCfg->BulkOutMlmeLock);
    pAd->pHifCfg->bBulkOutMlme = FALSE;
    pAd->pHifCfg->MlmePendingCnt --;
    NdisReleaseSpinLock(&pAd->pHifCfg->BulkOutMlmeLock); 
    
    DBGPRINT_RAW(RT_DEBUG_TRACE, ("<---%s   MlmePendingCnt  %d\n",  __FUNCTION__ ,pAd->pHifCfg->MlmePendingCnt ));  
}


/*
    ========================================================================

    Routine Description:
        USB_RxPacket initializes a URB and uses the Rx IRP to submit it
        to USB. It checks if an Rx Descriptor is available and passes the
        the coresponding buffer to be filled. If no descriptor is available
        fails the request. When setting the completion routine we pass our
        Adapter Object as Context.
            
    Arguments:
        
    Return Value:
        TRUE            found matched tuple cache
        FALSE           no matched found

    Note:
    
    ========================================================================
*/
VOID    N6USBBulkReceive(
    IN  PMP_ADAPTER   pAd)
{
    
    PRX_CONTEXT         pRxContext;
    NTSTATUS            Status;

    if(MT_TEST_FLAG(pAd, fRTMP_ADAPTER_IDLE_RADIO_OFF))
    {
        DBGPRINT(RT_DEBUG_TRACE, ("fRTMP_ADAPTER_IDLE_RADIO_OFF is on, N6USBBulkReceive returned.\n"));
        return;
    }

    NdisAcquireSpinLock(&pAd->pHifCfg->BulkInLock);

    // founded that the 2 URB right before and right after BULKIn FAIL  are in fact "ONE complete Data from USB PHY".
    // So at BulkIn Fail,  we set (NextRxBulkInPosition = URB lenghen right before BulkInFail). 
    // Since we concatenate the 2 URB, then we can make one complete URB Data.        by Jan
    if (pAd->pHifCfg->NextRxBulkInPosition == 0)
    {
        // Normal case. Always step one index ahead.
        pAd->pHifCfg->NextRxBulkInIndex = (pAd->pHifCfg->NextRxBulkInIndex + 1) % (RX_RING_SIZE_END_INDEX);
    }
    // The exceptness case that the 2 URB is not ONE complete data from USB PHY is that
    // the right-before URB happens to be a last USB transaction, i.e. not multiple of MaxPacketSize.
    // In this case, we still step one index ahead.
    else
    {
        
        pAd->pHifCfg->NextRxBulkInIndex = (pAd->pHifCfg->NextRxBulkInIndex + 1) % (RX_RING_SIZE_END_INDEX);
        pAd->pHifCfg->NextRxBulkInPosition = 0;
    }

    // Manually set (NextRxBulkInPosition = MAX_RXBULK_SIZE) in BulkInFail. 
    // This is to distinguish the normal case, or the case that is right after BulkInFail case but TransferBufferLength = 0.
    if (pAd->pHifCfg->NextRxBulkInPosition == MAX_RXBULK_SIZE)
        pAd->pHifCfg->NextRxBulkInPosition = 0;
    
    pRxContext = &(pAd->pHifCfg->RxContext[pAd->pHifCfg->NextRxBulkInIndex]); 
    NdisReleaseSpinLock(&pAd->pHifCfg->BulkInLock);

        DBGPRINT_RAW(RT_DEBUG_TRACE, ("%s: pRxContext[%d] InUse = %d..  Return.\n", __FUNCTION__, pAd->pHifCfg->NextRxBulkInIndex,pRxContext->InUse));
    
    if ((pRxContext->InUse == TRUE))
    {
        DBGPRINT_RAW(RT_DEBUG_TRACE, ("%s: pRxContext[%d] InUse = %d..  Return.\n", __FUNCTION__, pAd->pHifCfg->NextRxBulkInIndex,pRxContext->InUse));
        return;
    }
    
    if ((pRxContext->Readable == TRUE))
    {       
        if (pAd->OpMode  == OPMODE_AP)
            ApDataN6RxPacket(pAd, TRUE, pRxContext->Index);
        else        
        {
            N6UsbRxStaPacket(pAd, TRUE, pRxContext->Index);
        }
        return;
    }   

    pRxContext->InUse = TRUE;
    pRxContext->IRPPending= TRUE;
    
    Status = N6USBInitRxDesc(pAd, pRxContext);
    if(NT_SUCCESS(Status))
    {
        MT_INC_REF(&pAd->pHifCfg->PendingRx);
        DBGPRINT(RT_DEBUG_TRACE, ("N6USBBulkReceive send a buffer to get 0x84 packet\n"));
        if (WdfRequestSend(pRxContext->Request, pRxContext->IoTarget, WDF_NO_SEND_OPTIONS ) == FALSE) 
        {   // Failure - Return request
            MT_DEC_REF(&pAd->pHifCfg->PendingRx);
            pRxContext->InUse = FALSE;
            pRxContext->IRPPending= FALSE;
            DBGPRINT_RAW(RT_DEBUG_TRACE, ("N6USBBulkReceive send failed\n"));   
            goto error;
        }   
        pAd->pHifCfg->BulkInReq++;
    }
error:
    return;
}

/*
    ========================================================================

    Routine Description:
        This routine process Rx Irp and call rx complete function.
        
    Arguments:
        DeviceObject    Pointer to the device object for next lower
                        device. DeviceObject passed in here belongs to
                        the next lower driver in the stack because we
                        were invoked via IoCallDriver in USB_RxPacket
                        AND it is not OUR device object
        Irp             Ptr to completed IRP
        Context         Ptr to our Adapter object (context specified
                        in IoSetCompletionRoutine
        
    Return Value:
        Always returns STATUS_MORE_PROCESSING_REQUIRED

    IRQL <=  DISPATCH_LEVEL

    Note:
        Always returns STATUS_MORE_PROCESSING_REQUIRED
    ========================================================================
*/
VOID    N6USBBulkRxComplete(
    IN WDFREQUEST  Request,
    IN WDFIOTARGET  Target,
    IN PWDF_REQUEST_COMPLETION_PARAMS  CompletionParams,
    IN WDFCONTEXT  Context)
{
    PRX_CONTEXT     pRxContext;
    PMP_ADAPTER   pAd;
    NTSTATUS        Status;
    PUSB_DEVICE_CONTEXT pUsbDeviceContext;
    PWDF_USB_REQUEST_COMPLETION_PARAMS  usbCompletionParams;
    WDF_REQUEST_REUSE_PARAMS            params;
    PMP_PORT    pPort = NULL;

//  PRXDSCR_BASE_STRUC pRxD;
    
    ASSERT(Context);
    pRxContext = (PRX_CONTEXT) Context;

    //
    // The request might not sent to the hardware(Target) and return failed
    // while device has been removed.
    // Meanwhile, We can't directly check pAd->Flags with fRTMP_ADAPTER_NIC_NOT_EXIST since pAd might be freed. 
    //
    Status = CompletionParams->IoStatus.Status;
    if (Status == STATUS_WDF_DEVICE_REMOVED_NOT_SENT)
    {
        DBGPRINT(RT_DEBUG_ERROR, ("%s: device has been removed. Status=0x%08x, TransferLength=0x%08x\n", Status, CompletionParams->IoStatus.Information));     
        return;
    }

#pragma prefast(suppress: __WARNING_DEREF_NULL_PTR, " should not be NULL")
    pAd = pRxContext->pAd;
    pPort = pAd->PortList[FXXK_PORT_0];
    pUsbDeviceContext = GetUsbDeviceContext(pAd->pHifCfg->UsbDevice);

    MT_DEC_REF(&pAd->pHifCfg->PendingRx);

    pRxContext->Readable = TRUE;
    pRxContext->InUse = FALSE;
    pRxContext->IRPPending = FALSE;

    DBGPRINT(RT_DEBUG_TRACE, ("N6USBBulkReceive MT7603 got packets from Bulkin pipe 0x84, pAd->pHifCfg->PendingRx = %d\n", pAd->pHifCfg->PendingRx)); 

    //
    // For usb devices, we should look at the Usb.Completion param.
    //
//  usbCompletionParams = CompletionParams->Parameters.Usb.Completion;
//  pRxContext->TransferBufferLength = (ULONG) usbCompletionParams->Parameters.PipeRead.Length;
    
    if (Status == STATUS_SUCCESS)
    {
        //
        // For usb devices, we should look at the Usb.Completion param.
        //
        usbCompletionParams = CompletionParams->Parameters.Usb.Completion;
        pRxContext->TransferBufferLength = (ULONG) usbCompletionParams->Parameters.PipeRead.Length;

        pAd->pHifCfg->BulkInComplete++;  
        pAd->pHifCfg->NextRxBulkInPosition = 0;

        // Fix S3/S4 BSOD requested by FUJI or from WHQL test
        WDF_REQUEST_REUSE_PARAMS_INIT(&params, WDF_REQUEST_REUSE_NO_FLAGS, STATUS_SUCCESS);
        Status = WdfRequestReuse(Request, &params);
        ASSERT(NT_SUCCESS(Status));
    }
    else
    {
        DBGPRINT_RAW(RT_DEBUG_INFO, ("Bulk In Status = 0x%x, failed\n", Status));
        pAd->pHifCfg->BulkInCompleteFail++;
        // Don't set Readable=TRUE in BulkInCompleteFail case.      

        // Fix S3/S4 BSOD requested by FUJI or from WHQL test, don't change the code to locate behind MTK_OID_N5_QUERY_USB_RESET_BULK_IN
        WDF_REQUEST_REUSE_PARAMS_INIT(&params, WDF_REQUEST_REUSE_NO_FLAGS, STATUS_SUCCESS);
        Status = WdfRequestReuse(Request, &params);
        ASSERT(NT_SUCCESS(Status));
//**
/*
        if ((!MT_TEST_FLAG(pAd, fRTMP_ADAPTER_RESET_IN_PROGRESS)) &&
            (!MT_TEST_FLAG(pAd, fRTMP_ADAPTER_BULKIN_RESET)) &&
            (!MT_TEST_FLAG(pAd, fRTMP_ADAPTER_HALT_IN_PROGRESS)) &&
            (!MT_TEST_FLAG(pAd, fRTMP_ADAPTER_NIC_NOT_EXIST)) &&
            (!MT_TEST_FLAG(pAd, fRTMP_ADAPTER_SLEEP_IN_PROGRESS)) &&
            (!MT_TEST_FLAG(pAd, fRTMP_ADAPTER_SUSPENDED)))
        {
            
            DBGPRINT_RAW(RT_DEBUG_ERROR, ("Bulk In Failed. Status = 0x%x\n", Status));
            DBGPRINT_RAW(RT_DEBUG_ERROR, ("==>NextRxBulkInIndex=0x%x, NextRxBulkInReadIndex=0x%x, TransferBufferLength= 0x%x\n", pAd->pHifCfg->NextRxBulkInIndex, pAd->pHifCfg->NextRxBulkInReadIndex, pRxContext->TransferBufferLength));

            MT_SET_FLAG(pAd, fRTMP_ADAPTER_BULKIN_RESET);
            MTEnqueueInternalCmd(pAd, pPort, MTK_OID_N5_QUERY_USB_RESET_BULK_IN, NULL, 0);
        }
*/      
        
        pRxContext->Readable = FALSE;
        return;
    }
    
//**

    //For Safe, N6UsbRxStaPacket can be called more than N6USBBulkReceive
    //pAd->pHifCfg->PendingRx value will have race condition between here and BULK_IN_RESET
//  if ( pAd->OpMode == OPMODE_AP)
//         ApDataN6RxPacket(pAd, FALSE, pRxContext->Index);
//  else
    {   
        DBGPRINT_RAW(RT_DEBUG_INFO, ("Bulk In Status = 0x%x\n", Status));
        // Call RxPacket to process packet and return the status
        N6UsbRxStaPacket(pAd, FALSE, pRxContext->Index);
    }       


#if 0
    pRxD = (PRXDSCR_BASE_STRUC)&pRxContext->TransferBuffer[0];

    DBGPRINT_RAW(RT_DEBUG_ERROR, ("84 N6USBBulkReceive AGG length %d\n", pRxContext->TransferBufferLength));
    DBGPRINT(RT_DEBUG_TRACE, ("84 N6USBBulkReceive MT7603 pRxD->PacketType = %d\n", pRxD->RxDscrDW0.PacketType));
    DBGPRINT(RT_DEBUG_TRACE, ("84 N6USBBulkReceive MT7603 pRxD->RxByteCount = %d\n", pRxD->RxDscrDW0.RxByteCount));
    DBGPRINT(RT_DEBUG_TRACE, ("84 N6USBBulkReceive MT7603 pRxD->RFBGroupValidIndicators = 0x%x\n", pRxD->RxDscrDW0.RFBGroupValidIndicators));

    {
        int i = 0;
        PUCHAR p = (PUCHAR)&pRxContext->TransferBuffer[0];
        //for (i = 0; i < (int)pRxD->RxDscrDW0.RxByteCount; i = i + 8)
        for (i = 0; i < (int)pRxContext->TransferBufferLength; i = i + 8)
        {
            DBGPRINT_RAW(RT_DEBUG_ERROR, ("[%d] %x %x %x %x %x %x %x %x\n", i, p[i], p[i+1], p[i+2], p[i+3], p[i+4], p[i+5], p[i+6], p[i+7]));
        }
    }
#endif

    pRxContext->Readable = FALSE;
    
//  if (/*(!MT_TEST_FLAG(pAd, fRTMP_ADAPTER_RESET_IN_PROGRESS)) &&*/
//      (!MT_TEST_FLAG(pAd, fRTMP_ADAPTER_BULKIN_RESET)) &&
//      (!MT_TEST_FLAG(pAd, fRTMP_ADAPTER_RADIO_OFF)) &&
//      (!MT_TEST_FLAG(pAd, fRTMP_ADAPTER_IDLE_RADIO_OFF))    &&
//      (!MT_TEST_FLAG(pAd, fRTMP_ADAPTER_HALT_IN_PROGRESS)) &&
//      (!MT_TEST_FLAG(pAd, fRTMP_ADAPTER_NIC_NOT_EXIST)) && 
//      (!MT_TEST_FLAG(pAd, fRTMP_ADAPTER_SLEEP_IN_PROGRESS)) &&
//      (!MT_TEST_FLAG(pAd, fRTMP_ADAPTER_SUSPENDED)))
    {
        if( pAd->pHifCfg->PendingRx < pAd->pNicCfg->NumOfBulkInIRP )
        {
            N6USBBulkReceive(pAd);
        }
    }
}

/*
    ========================================================================

    Routine Description:
        USB_RxPacket initializes a URB and uses the Rx IRP to submit it
        to USB. It checks if an Rx Descriptor is available and passes the
        the coresponding buffer to be filled. If no descriptor is available
        fails the request. When setting the completion routine we pass our
        Adapter Object as Context.
            
    Arguments:
        
    Return Value:
        TRUE            found matched tuple cache
        FALSE           no matched found

    Note:
    
    ========================================================================
*/
VOID    N6USBBulkReceiveCMD(
    IN  PMP_ADAPTER   pAd)
{
    
    PRX_CONTEXT         pRxContext;
    NTSTATUS            Status;

    if (pAd->TrackInfo.bStopCMDEventReceive == TRUE)
    {
        NdisSetEvent(&(pAd->HwCfg.WaitFWEvent));
        return;
    }

//  if(MT_TEST_FLAG(pAd, fRTMP_ADAPTER_IDLE_RADIO_OFF))
//  {
//      DBGPRINT(RT_DEBUG_TRACE, ("fRTMP_ADAPTER_IDLE_RADIO_OFF is on, N6USBBulkReceiveCMD returned.\n"));
//      return;
//  }

//  NdisAcquireSpinLock(&pAd->pHifCfg->BulkInLock);

    // Manually set (NextRxBulkInPosition = MAX_RXBULK_SIZE) in BulkInFail. 
    // This is to distinguish the normal case, or the case that is right after BulkInFail case but TransferBufferLength = 0.
//  if (pAd->pHifCfg->NextRxBulkInPosition == MAX_RXBULK_SIZE)
//      pAd->pHifCfg->NextRxBulkInPosition = 0;
    
    pRxContext = &(pAd->pHifCfg->RxContext[31]); 
//  NdisReleaseSpinLock(&pAd->pHifCfg->BulkInLock);

        DBGPRINT_RAW(RT_DEBUG_TRACE, ("%s: pRxContext[30] InUse = %d..  Return.\n", __FUNCTION__, pRxContext->InUse));
    
    if ((pRxContext->InUse == TRUE))
    {
        DBGPRINT_RAW(RT_DEBUG_TRACE, ("%s: pRxContext[31] InUse = %d..  Return.\n", __FUNCTION__, pRxContext->InUse));
        return;
    }
    
    if ((pRxContext->Readable == TRUE))
    {       
        DBGPRINT_RAW(RT_DEBUG_TRACE, ("%s: pRxContext[31] Readable = %d..  Return.\n", __FUNCTION__, pRxContext->Readable));
        return;
    }   

    pRxContext->InUse = TRUE;
    pRxContext->IRPPending= TRUE;
    
    Status = N6USBInitRxDesc(pAd, pRxContext);
    if(NT_SUCCESS(Status))
    {
        //MT_INC_REF(&pAd->pHifCfg->PendingRx);
        DBGPRINT(RT_DEBUG_TRACE, ("N6USBBulkReceive send a buffer to get CMD 0x85-1 packet\n"));
        if (WdfRequestSend(pRxContext->Request, pRxContext->IoTarget, WDF_NO_SEND_OPTIONS ) == FALSE) 
        {   // Failure - Return request
            //MT_DEC_REF(&pAd->pHifCfg->PendingRx);
            pRxContext->InUse = FALSE;
            pRxContext->IRPPending= FALSE;
            DBGPRINT_RAW(RT_DEBUG_TRACE, ("N6USBBulkReceive CMD send failed\n"));   
            goto error;
        }   
        pAd->pHifCfg->BulkInReq++;
    }
error:
    return;
}

/*
    ========================================================================

    Routine Description:
        This routine process Rx Irp and call rx complete function.
        
    Arguments:
        DeviceObject    Pointer to the device object for next lower
                        device. DeviceObject passed in here belongs to
                        the next lower driver in the stack because we
                        were invoked via IoCallDriver in USB_RxPacket
                        AND it is not OUR device object
        Irp             Ptr to completed IRP
        Context         Ptr to our Adapter object (context specified
                        in IoSetCompletionRoutine
        
    Return Value:
        Always returns STATUS_MORE_PROCESSING_REQUIRED

    IRQL <=  DISPATCH_LEVEL

    Note:
        Always returns STATUS_MORE_PROCESSING_REQUIRED
    ========================================================================
*/
VOID    N6USBBulkRxCMDComplete(
    IN WDFREQUEST  Request,
    IN WDFIOTARGET  Target,
    IN PWDF_REQUEST_COMPLETION_PARAMS  CompletionParams,
    IN WDFCONTEXT  Context)
{
    PRX_CONTEXT     pRxContext;
    PMP_ADAPTER   pAd;
    NTSTATUS        Status;
    PUSB_DEVICE_CONTEXT pUsbDeviceContext;
    PWDF_USB_REQUEST_COMPLETION_PARAMS  usbCompletionParams;
    WDF_REQUEST_REUSE_PARAMS            params;

    PRXDSCR_BASE_STRUC pRxD;
    
    ASSERT(Context);
    pRxContext = (PRX_CONTEXT) Context;
    Status = CompletionParams->IoStatus.Status;
    //
    // Check status at first, before access pAd.
    // Since WDF might return STATUS_WDF_DEVICE_REMOVED_NOT_SENT due to surprise remove when calling WdfDriverMiniportUnload
    // Meanwhile, all pAd resource has been freed at DriverUnload and the pAd can't be touch in this case.
    //
    if (Status == STATUS_WDF_DEVICE_REMOVED_NOT_SENT)
    {
        DBGPRINT(RT_DEBUG_ERROR, ("%s: device has been removed. Status=0x%08x, TransferLength=0x%08x\n", Status, CompletionParams->IoStatus.Information));     
        return;
    }

#pragma prefast(suppress: __WARNING_DEREF_NULL_PTR, " should not be NULL")
    pAd = pRxContext->pAd;
    pUsbDeviceContext = GetUsbDeviceContext(pAd->pHifCfg->UsbDevice);

    //MT_DEC_REF(&pAd->pHifCfg->PendingRx);

    pRxContext->Readable = TRUE;
    pRxContext->InUse = FALSE;
    pRxContext->IRPPending = FALSE;

    DBGPRINT(RT_DEBUG_TRACE, ("N6USBBulkReceive MT7603 got packets from Bulkin pipe 0x85-1, pAd->pHifCfg->PendingRx = %d\n", pAd->pHifCfg->PendingRx));   

    //
    // For usb devices, we should look at the Usb.Completion param.
    //
    usbCompletionParams = CompletionParams->Parameters.Usb.Completion;
    pRxContext->TransferBufferLength = (ULONG) usbCompletionParams->Parameters.PipeRead.Length;
    
    if (Status == STATUS_SUCCESS)
    {
        pAd->pHifCfg->BulkInComplete++;  
        pAd->pHifCfg->NextRxBulkInPosition = 0;

        // Fix S3/S4 BSOD requested by FUJI or from WHQL test
        WDF_REQUEST_REUSE_PARAMS_INIT(&params, WDF_REQUEST_REUSE_NO_FLAGS, STATUS_SUCCESS);
        Status = WdfRequestReuse(Request, &params);
        ASSERT(NT_SUCCESS(Status));
    }
    else
    {
        pAd->pHifCfg->BulkInCompleteFail++;
        // Don't set Readable=TRUE in BulkInCompleteFail case.      

        // Fix S3/S4 BSOD requested by FUJI or from WHQL test, don't change the code to locate behind MTK_OID_N5_QUERY_USB_RESET_BULK_IN
        WDF_REQUEST_REUSE_PARAMS_INIT(&params, WDF_REQUEST_REUSE_NO_FLAGS, STATUS_SUCCESS);
        Status = WdfRequestReuse(Request, &params);
        ASSERT(NT_SUCCESS(Status));
//**
/*
        if ((!MT_TEST_FLAG(pAd, fRTMP_ADAPTER_RESET_IN_PROGRESS)) &&
            (!MT_TEST_FLAG(pAd, fRTMP_ADAPTER_BULKIN_RESET)) &&
            (!MT_TEST_FLAG(pAd, fRTMP_ADAPTER_HALT_IN_PROGRESS)) &&
            (!MT_TEST_FLAG(pAd, fRTMP_ADAPTER_NIC_NOT_EXIST)) &&
            (!MT_TEST_FLAG(pAd, fRTMP_ADAPTER_SLEEP_IN_PROGRESS)) &&
            (!MT_TEST_FLAG(pAd, fRTMP_ADAPTER_SUSPENDED)))
        {
            
            DBGPRINT_RAW(RT_DEBUG_ERROR, ("Bulk In Failed. Status = 0x%x\n", Status));
            DBGPRINT_RAW(RT_DEBUG_ERROR, ("==>NextRxBulkInIndex=0x%x, NextRxBulkInReadIndex=0x%x, TransferBufferLength= 0x%x\n", pAd->pHifCfg->NextRxBulkInIndex, pAd->pHifCfg->NextRxBulkInReadIndex, pRxContext->TransferBufferLength));

            MT_SET_FLAG(pAd, fRTMP_ADAPTER_BULKIN_RESET);
            MTEnqueueInternalCmd(pAd, pPort, MTK_OID_N5_QUERY_USB_RESET_BULK_IN, NULL, 0);
        }
*/      
        DBGPRINT_RAW(RT_DEBUG_INFO, ("Bulk In Status = 0x%x, failed\n", Status));
        pRxContext->Readable = FALSE;
        return;
    }
    
//**

    //For Safe, N6UsbRxStaPacket can be called more than N6USBBulkReceive
    //pAd->pHifCfg->PendingRx value will have race condition between here and BULK_IN_RESET
//  if ( pAd->OpMode == OPMODE_AP)
//         ApDataN6RxPacket(pAd, FALSE, pRxContext->Index);
//  else
    {   
        DBGPRINT_RAW(RT_DEBUG_INFO, ("Bulk In Status = 0x%x\n", Status));
        // Call RxPacket to process packet and return the status
        N6UsbRxStaPacket(pAd, FALSE, pRxContext->Index);
    }       


    pRxD = (PRXDSCR_BASE_STRUC)&pRxContext->TransferBuffer[0];

#if 0
    DBGPRINT_RAW(RT_DEBUG_ERROR, ("85 N6USBBulkReceive AGG length %d\n", pRxContext->TransferBufferLength));
    DBGPRINT(RT_DEBUG_TRACE, ("85 N6USBBulkReceive MT7603 pRxD->PacketType = %d\n", pRxD->RxDscrDW0.PacketType));
    DBGPRINT(RT_DEBUG_TRACE, ("85 N6USBBulkReceive MT7603 pRxD->RxByteCount = %d\n", pRxD->RxDscrDW0.RxByteCount));
    DBGPRINT(RT_DEBUG_TRACE, ("85 N6USBBulkReceive MT7603 pRxD->RFBGroupValidIndicators = 0x%x\n", pRxD->RxDscrDW0.RFBGroupValidIndicators));

    {
        int i = 0;
        PUCHAR p = (PUCHAR)&pRxContext->TransferBuffer[0];
        for (i = 0; i < (int)pRxD->RxDscrDW0.RxByteCount; i = i + 8)
        {
            DBGPRINT_RAW(RT_DEBUG_ERROR, ("%x %x %x %x %x %x %x %x\n", p[i], p[i+1], p[i+2], p[i+3], p[i+4], p[i+5], p[i+6], p[i+7]));
        }
    }
#endif

    pRxContext->Readable = FALSE;
    
//  if (/*(!MT_TEST_FLAG(pAd, fRTMP_ADAPTER_RESET_IN_PROGRESS)) &&*/
//      (!MT_TEST_FLAG(pAd, fRTMP_ADAPTER_BULKIN_RESET)) &&
//      (!MT_TEST_FLAG(pAd, fRTMP_ADAPTER_RADIO_OFF)) &&
//      (!MT_TEST_FLAG(pAd, fRTMP_ADAPTER_IDLE_RADIO_OFF))    &&
//      (!MT_TEST_FLAG(pAd, fRTMP_ADAPTER_HALT_IN_PROGRESS)) &&
//      (!MT_TEST_FLAG(pAd, fRTMP_ADAPTER_NIC_NOT_EXIST)) && 
//      (!MT_TEST_FLAG(pAd, fRTMP_ADAPTER_SLEEP_IN_PROGRESS)) &&
//      (!MT_TEST_FLAG(pAd, fRTMP_ADAPTER_SUSPENDED)))

    N6USBBulkReceiveCMD(pAd);


}


/*
    ========================================================================
    
    Routine Description:

    Arguments:

    Return Value:

    IRQL = 
    
    Note:
    
    ========================================================================
*/
VOID    N6USBKickBulkOut(
    IN  PMP_ADAPTER pAd)
{
    PMP_PORT pPort  = pAd->PortList[pAd->ucActivePortNum];
    DBGPRINT_RAW(RT_DEBUG_INFO, ("--->N6USBKickBulkOut\n"));
    // BulkIn Reset will reset whole USB PHY. So we need to make sure fRTMP_ADAPTER_BULKIN_RESET not flaged.
    if (!(MT_TEST_FLAG(pAd, fRTMP_ADAPTER_NIC_NOT_EXIST)) &&
        !(MT_TEST_FLAG(pAd, fRTMP_ADAPTER_HALT_IN_PROGRESS)) &&
        !(MT_TEST_FLAG(pAd, fRTMP_ADAPTER_RESET_IN_PROGRESS)) &&
        !(MT_TEST_FLAG(pAd, fRTMP_ADAPTER_BULKOUT_RESET)) &&
//      !(MT_TEST_FLAG(pAd, fRTMP_ADAPTER_BULKIN_RESET)) &&
        !(MT_TEST_FLAG(pAd, fRTMP_ADAPTER_RADIO_OFF)) &&
        !MT_TEST_FLAG(pAd, fRTMP_ADAPTER_SLEEP_IN_PROGRESS))
    {

        //DBGPRINT_RAW(RT_DEBUG_TRACE, ("m "));
        // 1. Data Fragment has highest priority
        if (MTUSB_TEST_BULK_FLAG(pAd, fRTUSB_BULK_OUT_DATA_FRAG))
        {
            if (((!MT_TEST_FLAG(pAd, fRTMP_ADAPTER_BSS_SCAN_IN_PROGRESS)) || 
                (!OPSTATUS_TEST_FLAG(pPort, fOP_STATUS_MEDIA_STATE_CONNECTED))))
            {               
                NdisSetEvent(&(GET_THREAD_EVENT(&pAd->pHifCfg->hBulkOutDataThread[0])));
            }
        }

        if (MTUSB_TEST_BULK_FLAG(pAd, fRTUSB_BULK_OUT_DATA_FRAG_2))
        {
            if (((!MT_TEST_FLAG(pAd, fRTMP_ADAPTER_BSS_SCAN_IN_PROGRESS)) || 
                (!OPSTATUS_TEST_FLAG(pPort, fOP_STATUS_MEDIA_STATE_CONNECTED))))
            {
                NdisSetEvent(&(GET_THREAD_EVENT(&pAd->pHifCfg->hBulkOutDataThread[1])));
            }
        }

        if (MTUSB_TEST_BULK_FLAG(pAd, fRTUSB_BULK_OUT_DATA_FRAG_3))
        {
            if (((!MT_TEST_FLAG(pAd, fRTMP_ADAPTER_BSS_SCAN_IN_PROGRESS)) || 
                (!OPSTATUS_TEST_FLAG(pPort, fOP_STATUS_MEDIA_STATE_CONNECTED))))
            {
                NdisSetEvent(&(GET_THREAD_EVENT(&pAd->pHifCfg->hBulkOutDataThread[2])));
            }
        }

        if (MTUSB_TEST_BULK_FLAG(pAd, fRTUSB_BULK_OUT_DATA_FRAG_4))
        {       
            if (((!MT_TEST_FLAG(pAd, fRTMP_ADAPTER_BSS_SCAN_IN_PROGRESS)) || 
                (!OPSTATUS_TEST_FLAG(pPort, fOP_STATUS_MEDIA_STATE_CONNECTED))))
            {
                NdisSetEvent(&(GET_THREAD_EVENT(&pAd->pHifCfg->hBulkOutDataThread[3])));
            }
        }
        
        // 6. Data frame normal is next
        if (MTUSB_TEST_BULK_FLAG(pAd, fRTUSB_BULK_OUT_DATA_NORMAL))
        {
            if (((!MT_TEST_FLAG(pAd, fRTMP_ADAPTER_BSS_SCAN_IN_PROGRESS)) || 
                (!OPSTATUS_TEST_FLAG(pPort, fOP_STATUS_MEDIA_STATE_CONNECTED))))
            {
                NdisSetEvent(&(GET_THREAD_EVENT(&pAd->pHifCfg->hBulkOutDataThread[0])));
            }
        }
        if (MTUSB_TEST_BULK_FLAG(pAd, fRTUSB_BULK_OUT_DATA_NORMAL_2))
        {       
            if (((!MT_TEST_FLAG(pAd, fRTMP_ADAPTER_BSS_SCAN_IN_PROGRESS)) || 
                (!OPSTATUS_TEST_FLAG(pPort, fOP_STATUS_MEDIA_STATE_CONNECTED))))
            {
                NdisSetEvent(&(GET_THREAD_EVENT(&pAd->pHifCfg->hBulkOutDataThread[1])));
            }
        }

                
        if (MTUSB_TEST_BULK_FLAG(pAd, fRTUSB_BULK_OUT_DATA_NORMAL_3))
        {
            if (((!MT_TEST_FLAG(pAd, fRTMP_ADAPTER_BSS_SCAN_IN_PROGRESS)) || 
                (!OPSTATUS_TEST_FLAG(pPort, fOP_STATUS_MEDIA_STATE_CONNECTED))))
            {
                NdisSetEvent(&(GET_THREAD_EVENT(&pAd->pHifCfg->hBulkOutDataThread[2])));
            }
        }

        if (MTUSB_TEST_BULK_FLAG(pAd, fRTUSB_BULK_OUT_DATA_NORMAL_4))
        {
            if (((!MT_TEST_FLAG(pAd, fRTMP_ADAPTER_BSS_SCAN_IN_PROGRESS)) || 
                (!OPSTATUS_TEST_FLAG(pPort, fOP_STATUS_MEDIA_STATE_CONNECTED))))
            {
                NdisSetEvent(&(GET_THREAD_EVENT(&pAd->pHifCfg->hBulkOutDataThread[3])));
            }
        }
            }
        }
        
VOID
N6USBXmitSendMlmeCmdPkt(
    IN  PMP_ADAPTER       pAd,
    IN  MLME_PKT_TYPE    PktType
    )
{
            
    if (pAd->pHifCfg->MlmePendingCnt > 1 && PktType == TXPKT_MLME_FRAME)
        return;

    if (pAd->pHifCfg->bBulking == TRUE)
        return;
    
    pAd->pHifCfg->bBulking = TRUE;
    
    NdisAcquireSpinLock(&pAd->pHifCfg->BulkOutMlmeLock);
    pAd->pHifCfg->bBulkOutMlme = TRUE;
    pAd->pHifCfg->MlmePendingCnt ++;
    NdisReleaseSpinLock(&pAd->pHifCfg->BulkOutMlmeLock);

    DBGPRINT(RT_DEBUG_TRACE, ("===>%s() Bulk out pAd->pHifCfg->MlmePendingCnt = %d\n", __FUNCTION__, pAd->pHifCfg->MlmePendingCnt));
    switch (PktType)
    {
        case TXPKT_FW_FRAME:
        {
            N6USBBulkOutFwPkt(pAd);
            break;
        }
        case TXPKT_CMD_FRAME:
        {
            N6USBBulkOutPktCmd(pAd);
            break;
        }
        case TXPKT_MLME_FRAME:
        {   
#if 0        
            N6USBBulkOutMLMEPacket(pAd, pAd->pHifCfg->PrioRingFirstIndex);
#else
            N6USBBulkOutMLMEPacket(pAd);
#endif
            break;
        }
        case TXPKT_NULL_FRAME:
        {
            N6USBBulkOutNullFrame(pAd);
            break;
        }
        case TXPKT_PS_POLL_FRAME:
        {
            N6USBBulkOutPsPoll(pAd);
            break;
        }
        default:
            break;
    }

    pAd->pHifCfg->bBulking = FALSE;

    DBGPRINT(RT_DEBUG_TRACE, ("<===%s() Bulk out Pkt type %d\n", __FUNCTION__, PktType));
}
    

/*
    ========================================================================
    
    Routine Description:
    Call from Reset action after BulkOut failed. 
    Arguments:

    Return Value:

    IRQL = 
    
    Note:
    
    ========================================================================
*/
VOID    N6USBCleanUpDataBulkOutQueue(
    IN  PMP_ADAPTER   pAd)
{
    UCHAR       Idx;

    DBGPRINT(RT_DEBUG_TRACE, ("--->CleanUpDataBulkOutQueue\n"));

    for (Idx = 0; Idx < 4; Idx++)
    {
        pAd->pHifCfg->TxContext[Idx].CurWritePosition = pAd->pHifCfg->TxContext[Idx].NextBulkOutPosition;
        pAd->pHifCfg->TxContext[Idx].LastOne                 = FALSE;
        NdisAcquireSpinLock(&pAd->pTxCfg->TxContextQueueLock[Idx]);
        pAd->pHifCfg->BulkOutPending[Idx] = FALSE;
        NdisReleaseSpinLock(&pAd->pTxCfg->TxContextQueueLock[Idx]);
    }
    
    DBGPRINT(RT_DEBUG_TRACE, ("<---CleanUpDataBulkOutQueue\n"));
}

VOID
N6USBWaitPendingRequest(
    IN PMP_ADAPTER pAd
    )
{
    UINT iWaitCount = 0, i=0;
    BOOLEAN bDone =FALSE;
    PRX_CONTEXT     pRxContext;
    FUNC_ENTER;
    while (TRUE) 
    {
        bDone = TRUE;
        if (pAd->pHifCfg->PendingRx > 0)
        {
            DBGPRINT(RT_DEBUG_TRACE, ("PendingRx = %d\n", pAd->pHifCfg->PendingRx));
            bDone = FALSE;
            
            //Clear Pending Request
            for(i = 0 ; i < RX_RING_SIZE ; i++)
            {
                pRxContext = &pAd->pHifCfg->RxContext[i];
                if (pRxContext->IRPPending == TRUE)
                {
                    WdfRequestCancelSentRequest(pRxContext->Request);
                }
            }
        }

        if (bDone)
        {
            break;
        }
        
        // not wait for 10 sec
        if (++iWaitCount > 10)
        {
            break;
        }

        NdisCommonGenericDelay(500000);
    }    

    iWaitCount =0;
    while (pAd->pHifCfg->MlmePendingCnt>0)
    {
        NdisCommonGenericDelay(200000);
        // not wait for 10 sec
        if (++iWaitCount > 2)
            break;        
    }
    FUNC_LEAVE;
}

UCHAR
N6UsbBulkGetPipeNumber(
    IN PMP_ADAPTER pAd
    )
{
    PUSB_DEVICE_CONTEXT                 usbDeviceContext;
    usbDeviceContext = GetUsbDeviceContext(pAd->pHifCfg->UsbDevice);
    return usbDeviceContext->NumberOfPipes;
}
