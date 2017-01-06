#ifndef __N6USBMAIN_H__
#define __N6USBMAIN_H__

//WDF releated define
/******************************************************************************************/
//usb pipe number
#define MAX_PIPES 12
#define MAX_READS 2

//#define BULK_TYPE_IN 0
//#define BULK_TYPE_OUT 1

typedef enum _USB_PIPE_TYPE {
    BULK_TYPE_IN    = 0,
    BULK_TYPE_OUT   = 1,
    BULK_TYPE_UNKNOWN   = 2
} USB_PIPE_TYPE;

//
// Recipient identity.
//
typedef enum _RTUSB_BMREQUEST_RECIPIENT {
    RTUSB_RequestToDevice       = BMREQUEST_TO_DEVICE,
    RTUSB_RequestToInterface    = BMREQUEST_TO_INTERFACE,
    RTUSB_RequestToEndpoint     = BMREQUEST_TO_ENDPOINT,
    RTUSB_RequestToOther        = BMREQUEST_TO_OTHER,
} RTUSB_BMREQUEST_RECIPIENT;

//
// USB request direction.
//
typedef enum _RTUSB_BMREQUEST_DIRECTION {
    RTUSB_TRANSFER_DIRECTION_OUT    = BMREQUEST_HOST_TO_DEVICE,
    RTUSB_TRANSFER_DIRECTION_IN     = BMREQUEST_DEVICE_TO_HOST,
} RTUSB_BMREQUEST_DIRECTION;

typedef struct _USB_WRITE_RESOURCES {
    WDFREQUEST      WriteRequestArray[MAX_WRITE_USB_REQUESTS];
    WDFSPINLOCK     WriteRequestArrayLock;
    USHORT          MaxOutstandingWrites;
    USHORT          NextAvailableRequestIndex;   
} USB_WRITE_RESOURCES, *PUSB_WRITE_RESOURCES;

typedef struct _USB_CONTROL_REQ_CONTEXT {
    ULONG_PTR         VirtualAddress; // va for next segment of xfer.
    PMP_ADAPTER NdisContext;
    UCHAR             QueueType;    
    UCHAR             Data[sizeof(ULONG)];  
} USB_CONTROL_REQ_CONTEXT, *PUSB_CONTROL_REQ_CONTEXT  ;

typedef struct _USB_CONTROL_RESOURCES {
//    WDFREQUEST      RequestArray[MAX_CONTROL_REQUESTS];
    WDFSPINLOCK     RequestArrayLock;
    USHORT          MaxOutstandingRequests;
    USHORT          NextAvailableRequestIndex;     
} USB_CONTROL_RESOURCES, *PUSB_CONTROL_RESOURCES;

#define INPUT_PIPE_NUM 2
#define OUTPUT_PIPE_NUM 10

typedef enum _ENDPOINT_IN
{
    ENDPOINT_IN_84,     // For packet/Load FW Event
    ENDPOINT_IN_85,     // For LMAC Event
    ENDPOINT_IN_NUM   // Endpoint In number
} ENDPOINT_IN, *PENDPOINT_IN;

typedef enum _ENDPOINT_OUT
{
    ENDPOINT_OUT_4,     // For WMM1 AC0
    ENDPOINT_OUT_5,     // For WMM1 AC1
    ENDPOINT_OUT_6,     // For WMM1 AC2
    ENDPOINT_OUT_7,     // For WMM1 AC3
    ENDPOINT_OUT_8,     // For CMD 
    ENDPOINT_OUT_9,     // For WMM2 AC0/AC1/AC2/AC3
    ENDPOINT_OUT_NUM   // Endpoint Out number
} ENDPOINT_OUT, *PENDPOINT_OUT;

//
// put this in the NIC data structure
//
typedef struct _USB_DEVICE_CONTEXT
{
    WDFUSBPIPE                      BulkInPipeHandle[ENDPOINT_IN_NUM]; //InputPipe;
    ULONG                           BulkInMaxPacketSize;
    WDFUSBPIPE                      BulkOutPipeHandle[ENDPOINT_OUT_NUM];//OutputPipeNormal;
    ULONG                           BulkOutMaxPacketSize;
    WDFWAITLOCK                     PipeStateLock;
    UCHAR                               NumberOfPipes;
    WDFUSBINTERFACE                 UsbInterface;
} USB_DEVICE_CONTEXT, *PUSB_DEVICE_CONTEXT;



typedef struct _USB_WRITE_REQ_CONTEXT
{
    WDFMEMORY         UrbMemory;
    PURB              Urb;
    PMDL              Mdl;
    USBD_PIPE_HANDLE  UsbdPipeHandle ;
    WDFUSBPIPE        UsbPipe;
    WDFIOTARGET       IoTarget;
    ULONG             WriteSize;
    UCHAR           IrpIdx; 
    PVOID   pTxContext;
} USB_WRITE_REQ_CONTEXT, *PUSB_WRITE_REQ_CONTEXT;

WDF_DECLARE_CONTEXT_TYPE_WITH_NAME(USB_WRITE_REQ_CONTEXT, GetWriteRequestContext)
    
WDF_DECLARE_CONTEXT_TYPE_WITH_NAME(USB_CONTROL_REQ_CONTEXT, GetControlRequestContext)

WDF_DECLARE_CONTEXT_TYPE_WITH_NAME(USB_DEVICE_CONTEXT, GetUsbDeviceContext)
/******************************************************************************************/

NTSTATUS
N6UsbCreateInitializeUsbDevice(
    IN  PMP_ADAPTER   pAd
    );

VOID
ReturnWriteRequest(
    IN  PMP_ADAPTER   pAd,
    WDFREQUEST  Request
    );

BOOLEAN
AllocateWriteRequestsWorker(
    IN  PMP_ADAPTER   pAd,
    IN  WDFUSBPIPE              UsbPipe,
    IN  PHT_TX_CONTEXT  pTxContext
    );

BOOLEAN
AllocateMLMEWriteRequestsWorker(
    IN  PMP_ADAPTER   pAd,
    IN  WDFUSBPIPE              UsbPipe,
    IN  PTX_CONTEXT pTxContext
    );

NDIS_STATUS
N6UsbXmitStart(
    IN  PMP_ADAPTER   pAd
    );

VOID
N6UsbXmitStop(
    IN  PMP_ADAPTER   pAd
    );

NDIS_STATUS
N6UsbRecvStart(
      IN    PMP_ADAPTER   pAd);

VOID
N6UsbRecvStop(
      IN    PMP_ADAPTER   pAd
    );

BOOLEAN 
N6UsbReadSafelyRemoveFlag(
    IN PMP_ADAPTER    pAd
    );

NDIS_STATUS 
N6USBInterfaceStart(
    IN  PMP_ADAPTER   pAd
    );    

NDIS_STATUS 
N6UsbInitTransmit(
    IN  PMP_ADAPTER   pAd
    );

NDIS_STATUS 
N6UsbInitRecv(
    IN  PMP_ADAPTER   pAd
    );    

VOID    
N6UsbFreeNICRecv(
    IN  PMP_ADAPTER   pAd
    );

VOID    
N6UsbFreeNICTransmit(
    IN PMP_ADAPTER pAd
    );     

NTSTATUS 
N6UsbRxStaPacket(
    IN  PMP_ADAPTER  pAd,
    IN    BOOLEAN          bBulkReceive,
    IN  ULONG           nIndex
    );

VOID 
XmitSendNullFrameFor7603Lookback(
    IN  PMP_ADAPTER   pAd,
    IN  UCHAR           EndpointIndex,
    IN  UCHAR           QueueId
    );

VOID 
XmitSendNullFrameFor7603Test(
    IN  PMP_ADAPTER   pAd,
    IN  UCHAR           EndpointIndex,
    IN  UCHAR           QueueId
    );

UCHAR
N6UsbBulkGetPipeNumber(
    IN PMP_ADAPTER pAd
    );    
#endif
