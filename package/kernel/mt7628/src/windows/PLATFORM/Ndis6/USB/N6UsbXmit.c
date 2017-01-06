#include "MtConfig.h"

VOID
ReturnWriteRequest(
    IN  PMP_ADAPTER   pAd,
    WDFREQUEST  Request
    )
/*++

Routine Description

    Returns a request back to the available array

Arguments:
    Nic -- Pointer to the NIC data structure. 
    Request -- Handle of the WDFREQUEST which is being freed back to the available pool. 
    QueueType -- Used to decide to which pool the Request belongs
    
Return Value:

    
--*/
{
    PUSB_DEVICE_CONTEXT         pUsbDeviceContext;  
    WDF_REQUEST_REUSE_PARAMS    params;
    NTSTATUS ntStatus;


    pUsbDeviceContext = GetUsbDeviceContext(pAd->pHifCfg->UsbDevice);
    WDF_REQUEST_REUSE_PARAMS_INIT(&params, WDF_REQUEST_REUSE_NO_FLAGS, STATUS_SUCCESS);
    ntStatus = WdfRequestReuse(Request, &params);
    if (!NT_SUCCESS(ntStatus)) 
    {
        DBGPRINT(RT_DEBUG_TRACE,("WdfRequestReuse fail: status(0x%08X)", ntStatus));                    
    }
}

BOOLEAN
AllocateMLMEWriteRequestsWorker(
    IN  PMP_ADAPTER   pAd,
    IN  WDFUSBPIPE              UsbPipe,
    IN  PTX_CONTEXT pTxContext
    )
{
    WDFIOTARGET             ioTarget;
    WDF_OBJECT_ATTRIBUTES   requestAttributes;
    UCHAR                   requestIndex ;
    NTSTATUS                    ntStatus;
    WDFMEMORY               urbMemory;
    WDF_OBJECT_ATTRIBUTES   objectAttribs;
    struct _URB_BULK_OR_INTERRUPT_TRANSFER *urbBuffer;
    CHAR                    dummyBuffer[1];  //Need a buffer to pass to UsbBuildInterruptOrBulkTransferRequest  

    ioTarget = WdfUsbTargetPipeGetIoTarget(UsbPipe);

    WDF_OBJECT_ATTRIBUTES_INIT(&objectAttribs);
    objectAttribs.ParentObject = pAd->pHifCfg->WdfDevice;    

    WDF_OBJECT_ATTRIBUTES_INIT_CONTEXT_TYPE(&requestAttributes, USB_WRITE_REQ_CONTEXT);

    for (requestIndex = 0; requestIndex < 2; requestIndex++)
    {
        WDFREQUEST*             pNextRequest; 
        USBD_PIPE_HANDLE        usbdPipeHandle;
        PUSB_WRITE_REQ_CONTEXT  writeContext;

   
        pNextRequest = &pTxContext->WriteRequestArray[requestIndex];
        ntStatus = WdfRequestCreate(&requestAttributes, ioTarget, pNextRequest);

        if (!NT_SUCCESS(ntStatus)) 
        {
            DBGPRINT(RT_DEBUG_TRACE,("Could not create request: status(0x%08X)", ntStatus));            
            return FALSE;
        }
        

            //
            // Preallocate the request timer to prevent the request from failing while trying to send it.
            //
            ntStatus = WdfRequestAllocateTimer(*pNextRequest);

            if (!NT_SUCCESS(ntStatus))
        {
            DBGPRINT(RT_DEBUG_TRACE,("Could not allocate timer for  request status(0x%08X)", ntStatus));            
            return FALSE;
            }

        WDF_OBJECT_ATTRIBUTES_INIT(&objectAttribs);
        objectAttribs.ParentObject = *pNextRequest;

        ntStatus = WdfMemoryCreate(
                                        &objectAttribs,
                                        NonPagedPool,
                                        0,
                                        sizeof(struct _URB_BULK_OR_INTERRUPT_TRANSFER),
                                        &urbMemory,
                                        &urbBuffer);

        if (!NT_SUCCESS(ntStatus)) 
        {
            DBGPRINT(RT_DEBUG_TRACE,("Could not create request lock: status(0x%08X)", ntStatus));
            return FALSE;
        }
               
        usbdPipeHandle = WdfUsbTargetPipeWdmGetPipeHandle(UsbPipe);
            //
            // NOTE : call    UsbBuildInterruptOrBulkTransferRequest otherwise 
            //    WdfUsbTargetPipeFormatRequestForUrb    will assert 
            //  with *** Assertion failed: Urb->UrbHeader.Length >= sizeof(_URB_HEADER)
            //
            UsbBuildInterruptOrBulkTransferRequest(
                                (PURB )urbBuffer,
                                sizeof(struct _URB_BULK_OR_INTERRUPT_TRANSFER),
                                usbdPipeHandle,
                                dummyBuffer,
                                NULL,
                                sizeof(dummyBuffer),
                                USBD_TRANSFER_DIRECTION_OUT | USBD_SHORT_TRANSFER_OK,
                                NULL);


            //
            // By calling  WdfUsbTargetPipeFormatRequestForUrb the frameworks allocate a lot of resources
            // like the underlying IRP for the request and hence it is better to do it at initilization 
            // to prevent an avoidable  failure later. 
            //
            ntStatus = WdfUsbTargetPipeFormatRequestForUrb(
                                                UsbPipe,
                                                *pNextRequest,
                                                urbMemory,
                                                NULL
                                                );


        if(!NT_SUCCESS(ntStatus)) 
        {
            DBGPRINT(RT_DEBUG_TRACE, (
                "WdfUsbTargetPipeFormatRequestForUrb: Failed - 0x%x \n",
                ntStatus));
            return FALSE;
        }

            //
            // set REQUEST_CONTEXT  parameters.         
            //
            
        writeContext = GetWriteRequestContext(*pNextRequest);
        writeContext->UrbMemory       = urbMemory;
        writeContext->Urb = (PURB )urbBuffer;
        writeContext->UsbdPipeHandle       = usbdPipeHandle;
        writeContext->UsbPipe               = UsbPipe;
        writeContext->IoTarget              = ioTarget;        
        writeContext->IrpIdx            = requestIndex;
        writeContext->pTxContext        = pTxContext;
    }
    
    return TRUE;
}

BOOLEAN
AllocateWriteRequestsWorker(
    IN  PMP_ADAPTER   pAd,
    IN  WDFUSBPIPE              UsbPipe,
    IN  PHT_TX_CONTEXT  pTxContext
    )
{
    WDFIOTARGET             ioTarget;
    WDF_OBJECT_ATTRIBUTES   requestAttributes;
    UCHAR                   requestIndex ;
    NTSTATUS                    ntStatus;
    WDFMEMORY               urbMemory;
    WDF_OBJECT_ATTRIBUTES   objectAttribs;
    struct _URB_BULK_OR_INTERRUPT_TRANSFER *urbBuffer;
    CHAR                    dummyBuffer[1];  //Need a buffer to pass to UsbBuildInterruptOrBulkTransferRequest  

    ioTarget = WdfUsbTargetPipeGetIoTarget(UsbPipe);

    WDF_OBJECT_ATTRIBUTES_INIT(&objectAttribs);
    objectAttribs.ParentObject = pAd->pHifCfg->WdfDevice;
    /*ntStatus = WdfSpinLockCreate(&objectAttribs, &WriteResources->WriteRequestArrayLock);

    if (!NT_SUCCESS(ntStatus)) 
    {
         DBGPRINT(RT_DEBUG_TRACE,("Could not create request lock: status(0x%08X)", ntStatus));
        return FALSE;
    }*/

    WDF_OBJECT_ATTRIBUTES_INIT_CONTEXT_TYPE(&requestAttributes, USB_WRITE_REQ_CONTEXT);

    for (requestIndex = 0; requestIndex < MAX_WRITE_USB_REQUESTS; requestIndex++)
    {
        WDFREQUEST*             pNextRequest; 
        USBD_PIPE_HANDLE        usbdPipeHandle;
        PUSB_WRITE_REQ_CONTEXT  writeContext;

   
        pNextRequest = &pTxContext->WriteRequestArray[requestIndex];
        ntStatus = WdfRequestCreate(&requestAttributes, ioTarget, pNextRequest);

        if (!NT_SUCCESS(ntStatus)) 
        {
            DBGPRINT(RT_DEBUG_TRACE,("Could not create request: status(0x%08X)", ntStatus));            
            return FALSE;
        }
        

            //
            // Preallocate the request timer to prevent the request from failing while trying to send it.
            //
            ntStatus = WdfRequestAllocateTimer(*pNextRequest);

            if (!NT_SUCCESS(ntStatus))
        {
            DBGPRINT(RT_DEBUG_TRACE,("Could not allocate timer for  request status(0x%08X)", ntStatus));            
            return FALSE;
            }

        WDF_OBJECT_ATTRIBUTES_INIT(&objectAttribs);
        objectAttribs.ParentObject = *pNextRequest;

        ntStatus = WdfMemoryCreate(
                                        &objectAttribs,
                                        NonPagedPool,
                                        0,
                                        sizeof(struct _URB_BULK_OR_INTERRUPT_TRANSFER),
                                        &urbMemory,
                                        &urbBuffer);

        if (!NT_SUCCESS(ntStatus)) 
        {
            DBGPRINT(RT_DEBUG_TRACE,("Could not create request lock: status(0x%08X)", ntStatus));
            return FALSE;
        }
               
        usbdPipeHandle = WdfUsbTargetPipeWdmGetPipeHandle(UsbPipe);
            //
            // NOTE : call    UsbBuildInterruptOrBulkTransferRequest otherwise 
            //    WdfUsbTargetPipeFormatRequestForUrb    will assert 
            //  with *** Assertion failed: Urb->UrbHeader.Length >= sizeof(_URB_HEADER)
            //
            UsbBuildInterruptOrBulkTransferRequest(
                                (PURB )urbBuffer,
                                sizeof(struct _URB_BULK_OR_INTERRUPT_TRANSFER),
                                usbdPipeHandle,
                                dummyBuffer,
                                NULL,
                                sizeof(dummyBuffer),
                                USBD_TRANSFER_DIRECTION_OUT | USBD_SHORT_TRANSFER_OK,
                                NULL);


            //
            // By calling  WdfUsbTargetPipeFormatRequestForUrb the frameworks allocate a lot of resources
            // like the underlying IRP for the request and hence it is better to do it at initilization 
            // to prevent an avoidable  failure later. 
            //
            ntStatus = WdfUsbTargetPipeFormatRequestForUrb(
                                                UsbPipe,
                                                *pNextRequest,
                                                urbMemory,
                                                NULL
                                                );


        if(!NT_SUCCESS(ntStatus)) 
        {
            DBGPRINT(RT_DEBUG_TRACE, (
                "WdfUsbTargetPipeFormatRequestForUrb: Failed - 0x%x \n",
                ntStatus));
            return FALSE;
        }

            //
            // set REQUEST_CONTEXT  parameters.         
            //
            
        writeContext = GetWriteRequestContext(*pNextRequest);
        writeContext->UrbMemory       = urbMemory;
        writeContext->Urb = (PURB )urbBuffer;
        writeContext->UsbdPipeHandle       = usbdPipeHandle;
        writeContext->UsbPipe               = UsbPipe;
        writeContext->IoTarget              = ioTarget;        
        writeContext->IrpIdx            = requestIndex;
        writeContext->pTxContext        = pTxContext;
    }
    
    return TRUE;
}

NDIS_STATUS
N6UsbXmitStart(
    IN  PMP_ADAPTER   pAd
    )
{
    PUSB_DEVICE_CONTEXT     pUsbDeviceContext;
    NTSTATUS            ntStatus;
    NDIS_STATUS     ndisStatus = NDIS_STATUS_SUCCESS;
    //UCHAR         BulkOutIndex = 0;
    WDFIOTARGET     WdfIoTarget = NULL;
    UCHAR           BulkOutIndex = 0;

    ntStatus = STATUS_SUCCESS;
    pUsbDeviceContext = GetUsbDeviceContext(pAd->pHifCfg->UsbDevice);

    if(pUsbDeviceContext == NULL)
    {
        return NDIS_STATUS_FAILURE;
    }

    
    WdfWaitLockAcquire(pUsbDeviceContext->PipeStateLock, NULL);

    for(BulkOutIndex=0;BulkOutIndex<pAd->pHifCfg->BulkOutPipeNum;BulkOutIndex++)
    {
        WdfIoTarget = WdfUsbTargetPipeGetIoTarget(pUsbDeviceContext->BulkOutPipeHandle[BulkOutIndex]);
        ntStatus =  WdfIoTargetStart(WdfIoTarget);
        
        DBGPRINT(RT_DEBUG_TRACE,("Start Bulk-out pipe index = %d\n",BulkOutIndex));

        if (!NT_SUCCESS(ntStatus)) 
        {        
            WdfWaitLockRelease(pUsbDeviceContext->PipeStateLock);
            ndisStatus = NDIS_STATUS_FAILURE;
            DBGPRINT(RT_DEBUG_ERROR,("Failed to start OutputPipeHigh\n"));
            break;
        }
    }
    
    if (!NT_SUCCESS(ntStatus)) 
    {
        ndisStatus = NDIS_STATUS_FAILURE;
        DBGPRINT(RT_DEBUG_ERROR, ("N6UsbXmitStart: Failed to start BulkOut\n"));
        return ndisStatus;
    }

    WdfWaitLockRelease(pUsbDeviceContext->PipeStateLock);    
    
    return ndisStatus;
}

VOID
N6UsbXmitStop(
    IN  PMP_ADAPTER   pAd
    )
{
    PUSB_DEVICE_CONTEXT     pUsbDeviceContext;
    UCHAR                   BulkOutPipeIndex = 0;

    DBGPRINT(RT_DEBUG_TRACE, ("==> N6UsbStopAllPipes\n"));

    if (pAd == NULL)
    {
        DBGPRINT(RT_DEBUG_ERROR, ("%s: pAd is NULL\n", __FUNCTION__));     
        return;
    }

    if (pAd->pHifCfg == NULL)
    {
        DBGPRINT(RT_DEBUG_ERROR, ("%s: pHifCfg is NULL\n", __FUNCTION__));     
        return;
    }

    if (pAd->pHifCfg->UsbDevice == NULL)
    {
        DBGPRINT(RT_DEBUG_ERROR, ("%s: UsbDevice is NULL\n", __FUNCTION__));     
        return;
    }
    
    pUsbDeviceContext = GetUsbDeviceContext(pAd->pHifCfg->UsbDevice);

    WdfWaitLockAcquire(pUsbDeviceContext->PipeStateLock, NULL);

    //
    // Stop the OUT endpoints
    //
    for (BulkOutPipeIndex = 0; BulkOutPipeIndex< pAd->pHifCfg->BulkOutPipeNum; BulkOutPipeIndex++)
    {
        WdfIoTargetStop(WdfUsbTargetPipeGetIoTarget(pUsbDeviceContext->BulkOutPipeHandle[BulkOutPipeIndex]),
                                                    WdfIoTargetCancelSentIo);
    }

    WdfWaitLockRelease(pUsbDeviceContext->PipeStateLock);
}

