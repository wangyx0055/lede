#include "MtConfig.h"

NDIS_STATUS
N6UsbRecvStart(
      IN    PMP_ADAPTER   pAd)
{
    PUSB_DEVICE_CONTEXT     pUsbDeviceContext;
    NTSTATUS                    ntStatus;
    NDIS_STATUS             ndisStatus = NDIS_STATUS_SUCCESS;
    UCHAR   i=0;

    ntStatus = STATUS_SUCCESS;
    pUsbDeviceContext = GetUsbDeviceContext(pAd->pHifCfg->UsbDevice);
    
    WdfWaitLockAcquire(pUsbDeviceContext->PipeStateLock, NULL);

    for(i=0;i<pAd->pHifCfg->BulkInPipeNum;i++)
    {
        ntStatus = WdfIoTargetStart(
                                WdfUsbTargetPipeGetIoTarget(pUsbDeviceContext->BulkInPipeHandle[i])
                                );
        
        if(!NT_SUCCESS(ntStatus))
            break;

        DBGPRINT(RT_DEBUG_TRACE,("Start Bulk-in pipe index = %d\n",i));
    }

    if (!NT_SUCCESS(ntStatus)) 
    {
        WdfWaitLockRelease(pUsbDeviceContext->PipeStateLock);
         DBGPRINT(RT_DEBUG_TRACE,("%s: Failed to start InputPipe\n",__FUNCTION__)); 
         ndisStatus = NDIS_STATUS_FAILURE;
        goto error;
    }
    
    WdfWaitLockRelease(pUsbDeviceContext->PipeStateLock);
    
error:    
    return ndisStatus;
}

VOID
N6UsbRecvStop(
      IN    PMP_ADAPTER   pAd
    )
{
    PUSB_DEVICE_CONTEXT     pUsbDeviceContext;
    UCHAR   i=0;

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

    for(i=0;i<pAd->pHifCfg->BulkInPipeNum;i++)
    {
        WdfIoTargetStop(
                        WdfUsbTargetPipeGetIoTarget(pUsbDeviceContext->BulkInPipeHandle[i]),
                        WdfIoTargetCancelSentIo
                        );
        
        DBGPRINT(RT_DEBUG_TRACE,("Stop Bulk-in pipe index = %d\n",i));
    }
    
    WdfWaitLockRelease(pUsbDeviceContext->PipeStateLock);
}

