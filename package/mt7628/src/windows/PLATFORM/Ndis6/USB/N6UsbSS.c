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
        usb_ss.c

    Abstract:
        Win8 USB Selective Suspend
*/

/* INCLUDE FILES */
#include "MtConfig.h"

#if _WIN8_USB_SS_SUPPORTED

VOID UsbSsIdleCallback(
    PVOID pContext)
{
    PMP_ADAPTER pAd = (PMP_ADAPTER)pContext;

    DBGPRINT(RT_DEBUG_TRACE, ("==> [Ss]UsbSsIdleCallback\n"));

    NdisMIdleNotificationConfirm(pAd->AdapterHandle, NdisDeviceStateD2);

    NdisAcquireSpinLock(&pAd->Ss.SsLock);
    MT_SET_FLAG(pAd, fRTMP_ADAPTER_SUSPENDED);
    NdisReleaseSpinLock(&pAd->Ss.SsLock);
}

NTSTATUS UsbSsIdleRequestComplete(
    IN PDEVICE_OBJECT pDeviceObject,
    IN PIRP pIrp,
    IN PVOID pContext)
{
    PMP_ADAPTER pAd = (PMP_ADAPTER)pContext;

    DBGPRINT(RT_DEBUG_TRACE, ("==> [Ss]UsbSsIdleRequestComplete\n"));

    NdisMIdleNotificationComplete(pAd->AdapterHandle);

    NdisAcquireSpinLock(&pAd->Ss.SsLock);
    MT_CLEAR_FLAG(pAd, fRTMP_ADAPTER_SUSPENDED);
    NdisReleaseSpinLock(&pAd->Ss.SsLock);

    return STATUS_MORE_PROCESSING_REQUIRED;
}

BOOLEAN UsbSsInit(
    PMP_ADAPTER pAd)
{
    BOOLEAN bSuccess = FALSE;

    DBGPRINT(RT_DEBUG_TRACE, ("==> [Ss]UsbSsInit\n"));

    do
    {
        //Prepare IRP
        pAd->Ss.pIrp = IoAllocateIrp(pAd->pHifCfg->pNextDeviceObject->StackSize + 1, FALSE);
        if (pAd->Ss.pIrp == NULL)
        {
            DBGPRINT(RT_DEBUG_TRACE, ("==> [Ss]Fail to alocate IRP.\n"));
            break;
        }

        //Prepare spinlock
        NdisAllocateSpinLock(&pAd->Ss.SsLock);

        bSuccess = TRUE;

    } while (0);

    return bSuccess;
}

VOID UsbSsHalt(
    PMP_ADAPTER pAd)
{
    DBGPRINT(RT_DEBUG_TRACE, ("==> [Ss]UsbSsHalt\n"));

    //Free IRP
    if (pAd->Ss.pIrp)
    {
        IoFreeIrp(pAd->Ss.pIrp);
        pAd->Ss.pIrp = NULL;
    }
}

VOID UsbSsIdleRequest(
    PMP_ADAPTER pAd)
{
    PIRP pIrp = pAd->Ss.pIrp;
    PIO_STACK_LOCATION pNextStack = NULL;

    NTSTATUS NtStatus = STATUS_SUCCESS;

    ASSERT(pIrp);

    DBGPRINT(RT_DEBUG_TRACE, ("==> [Ss]UsbSsIdleRequest\n"));

    IoReuseIrp(pIrp, STATUS_SUCCESS);

    pNextStack = IoGetNextIrpStackLocation(pIrp);
    pNextStack->MajorFunction = IRP_MJ_INTERNAL_DEVICE_CONTROL;
    pNextStack->Parameters.DeviceIoControl.IoControlCode = IOCTL_INTERNAL_USB_SUBMIT_IDLE_NOTIFICATION;
    pNextStack->Parameters.DeviceIoControl.InputBufferLength = sizeof(USB_IDLE_CALLBACK_INFO);

    pAd->Ss.CallbackInfo.IdleCallback = UsbSsIdleCallback;
    pAd->Ss.CallbackInfo.IdleContext = (PVOID)pAd;  

    pNextStack->Parameters.DeviceIoControl.Type3InputBuffer = &pAd->Ss.CallbackInfo;

    IoSetCompletionRoutine(pIrp,
            UsbSsIdleRequestComplete,
            pAd,
            TRUE,
            TRUE,
            TRUE);

    NtStatus = IoCallDriver(pAd->pHifCfg->pNextDeviceObject, pIrp);
    DBGPRINT(RT_DEBUG_TRACE, ("NtStatus = %x(STATUS_PENDING = %d)\n", NtStatus, (NtStatus == STATUS_PENDING)));

}

VOID UsbSsCancelIdleRequest(
    PMP_ADAPTER pAd)
{
    PIRP pIrp = pAd->Ss.pIrp;

    ASSERT(pIrp);

    DBGPRINT(RT_DEBUG_TRACE, ("==> [Ss]UsbSsCancelIdleRequest\n"));

    IoCancelIrp(pIrp);
}

NDIS_STATUS UsbSsSuspend(
    PMP_ADAPTER pAd)
{
    DBGPRINT(RT_DEBUG_TRACE, ("==> [Ss]UsbSsSuspend\n"));

    //
    // Close the door
    //
    NdisAcquireSpinLock(&pAd->Ss.SsLock);
    MT_SET_FLAG(pAd, fRTMP_ADAPTER_SUSPENDED);
    NdisReleaseSpinLock(&pAd->Ss.SsLock);

    //
    // Cancel timer
    //
    MlmeCancelAllTimers(pAd);

    //
    // Cancel pending I/O
    //
    N6UsbRecvStop(pAd);
    while (pAd->pHifCfg->bBulkOutMlme == TRUE)
    {
        DBGPRINT(RT_DEBUG_TRACE, ("bBulkOutMlme = %u...\n", pAd->pHifCfg->bBulkOutMlme));
        NdisCommonGenericDelay(1000);
    }

    while (pAd->pHifCfg->ControlPending == TRUE)
    {
        DBGPRINT(RT_DEBUG_TRACE, ("ControlPending = %u...\n", pAd->pHifCfg->ControlPending));
        NdisCommonGenericDelay(1000);
    }

    while (pAd->pHifCfg->PendingRx > 0)
    {
        DBGPRINT(RT_DEBUG_TRACE, ("PendingRx = %u...\n", pAd->pHifCfg->PendingRx));
        NdisCommonGenericDelay(1000);
    }

    //
    // Wait pending indication
    //
    while (pAd->PendingStatusIndication > 0 || pAd->PendingNBLIndication > 0)
    {
        DBGPRINT(RT_DEBUG_TRACE, ("PendingStatusIndication = %u, PendingNBLIndication = %u...\n",
            pAd->PendingStatusIndication,
            pAd->PendingNBLIndication));
        NdisCommonGenericDelay(1000);
    }

    DBGPRINT(RT_DEBUG_TRACE, ("Final: %u, %u, %u\n", pAd->pHifCfg->bBulkOutMlme, pAd->pHifCfg->ControlPending, pAd->pHifCfg->PendingRx));

    return NDIS_STATUS_SUCCESS;
}

NDIS_STATUS UsbSsResume(
    PMP_ADAPTER pAd)
{
    UCHAR BulkInIndex;
    LARGE_INTEGER   dueTime;
    PMP_PORT    pPort = pAd->PortList[FXXK_PORT_0];

    DBGPRINT(RT_DEBUG_TRACE, ("==> [Ss]UsbSsResume\n"));

    //
    // Open the door
    //
    NdisAcquireSpinLock(&pAd->Ss.SsLock);
    MT_CLEAR_FLAG(pAd, fRTMP_ADAPTER_SUSPENDED);
    NdisReleaseSpinLock(&pAd->Ss.SsLock);

    //
    // Restart I/O
    //
    N6UsbRecvStart(pAd);
    for (BulkInIndex = 0; BulkInIndex < pAd->pNicCfg->NumOfBulkInIRP; BulkInIndex++)
    {
        if (pAd->pHifCfg->PendingRx < pAd->pNicCfg->NumOfBulkInIRP)
        {
            N6USBBulkReceive(pAd);
        }
    }

    //
    // Restart timer
    //
    //dueTime = RtlConvertLongToLargeInteger((-1)*MLME_TASK_EXEC_INTV); 
   // KeSetCoalescableTimer(&pPort->Mlme.PeriodicTimer, dueTime, MLME_TASK_EXEC_INTV, 10, &pAd->Mlme.MlmeDpc);

    return NDIS_STATUS_SUCCESS;
}
#endif
