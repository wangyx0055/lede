/*++

Copyright (c) 2000  Microsoft Corporation

Module Name:

    bulkdev.h

Abstract:

Environment:

    Kernel mode

Notes:

    Copyright (c) 2000 Microsoft Corporation.  
    All Rights Reserved.

--*/

#ifndef _BULKUSB_DEV_H
#define _BULKUSB_DEV_H



IO_COMPLETION_ROUTINE IdleNotificationRequestComplete;


NTSTATUS
BulkUsb_ResetPipe(
    IN RTMP_ADAPTER *pAd,
    IN PUSBD_PIPE_INFORMATION PipeInfo
    );

NTSTATUS
BulkUsb_ResetDevice(
    IN RTMP_ADAPTER*pAd
    );

NTSTATUS
BulkUsb_GetPortStatus(
    IN RTMP_ADAPTER*pAd,
    IN PULONG PortStatus
    );
NTSTATUS
BulkUsb_ResetParentPort(
   IN RTMP_ADAPTER*pAd
    );

/*DRIVER_DISPATCH BulkUsb_DispatchDevCtrl;
DRIVER_DISPATCH BulkUsb_DispatchCreate;
DRIVER_DISPATCH BulkUsb_DispatchClose;
/











*/
VOID
IdleNotificationCallback(
    IN RTMP_ADAPTER *DeviceExtension
    );

NTSTATUS
SubmitIdleRequestIrp(
    IN RTMP_ADAPTER *DeviceExtension
    );

VOID
CancelSelectSuspend(
    IN RTMP_ADAPTER *DeviceExtension
    );

VOID
PoIrpCompletionFunc(
    IN PDEVICE_OBJECT   DeviceObject,
    IN UCHAR            MinorFunction,
    IN POWER_STATE      PowerState,
    IN PVOID            Context,
    IN PIO_STATUS_BLOCK IoStatus
    );

VOID
PoIrpAsyncCompletionFunc(
    IN PDEVICE_OBJECT DeviceObject,
    IN UCHAR MinorFunction,
    IN POWER_STATE PowerState,
    IN PVOID Context,
    IN PIO_STATUS_BLOCK IoStatus
    );

/*



VOID
WWIrpCompletionFunc(
    IN PDEVICE_OBJECT   DeviceObject,
    IN UCHAR            MinorFunction,
    IN POWER_STATE      PowerState,
    IN PVOID            Context,
    IN PIO_STATUS_BLOCK IoStatus
    );
*/

VOID LfsrInit(IN RTMP_ADAPTER *dx,IN ULONG Seed);
NTSTATUS UsbDoBulkOutTransfer(RTMP_ADAPTER *dx,int iBulkNum);
//VOID UsbDoBulkOutTransfer0(RTMP_ADAPTER *dx);
NTSTATUS UsbDoBulkOutTransfer0Complete(IN PDEVICE_OBJECT DeviceObject, IN PIRP pIrp, IN PVOID Context);
NTSTATUS UsbDoBulkOutTransfer1Complete(IN PDEVICE_OBJECT DeviceObject, IN PIRP pIrp, IN PVOID Context);
NTSTATUS UsbDoBulkOutTransfer2Complete(IN PDEVICE_OBJECT DeviceObject, IN PIRP pIrp, IN PVOID Context);
NTSTATUS UsbDoBulkOutTransfer3Complete(IN PDEVICE_OBJECT DeviceObject, IN PIRP pIrp, IN PVOID Context);
NTSTATUS UsbDoBulkOutTransfer4Complete(IN PDEVICE_OBJECT DeviceObject, IN PIRP pIrp, IN PVOID Context);
NTSTATUS UsbDoBulkOutTransfer5Complete(IN PDEVICE_OBJECT DeviceObject, IN PIRP pIrp, IN PVOID Context);



NTSTATUS UsbDoBulkIn0TransferComplete(IN PDEVICE_OBJECT DeviceObject, IN PIRP pIrp, IN PVOID Context);
NTSTATUS UsbDoBulkIn1TransferComplete(IN PDEVICE_OBJECT DeviceObject, IN PIRP pIrp, IN PVOID Context);
NTSTATUS UsbCancelBulkInIrp( IN RTMP_ADAPTER *DeviceExtension,int iBulkNum);


VOID UsbDoBulkInTransfer( IN RTMP_ADAPTER *dx,int iBulkNum);
NTSTATUS UsbBulkMemoryAlloc( IN RTMP_ADAPTER *dx);
VOID UsbBulkMemoryFree( IN RTMP_ADAPTER *pAd);


NTSTATUS RXStop_USB(IN RTMP_ADAPTER *pAdapter, IN  UINT16 Pipe);
NTSTATUS RXStart_USB(IN RTMP_ADAPTER *pAdapter, IN UINT16 Pipe);

#define START TRUE
#define STOP FALSE
#endif

