#ifndef	__DEFINE_H__
#define	__DEFINE_H__


#include <usb.h>

#include <usbdrivr.h>
#include <Wmilib.h>

//#define MEMORY_TAG (ULONG) 'aqTM'
//#define DRV_INSTANCE_SIGNATURE (ULONG) 'aqTM'
#ifdef RTMP_USB_SUPPORT

#define BULKTAG (ULONG) 'KluB'

#undef ExAllocatePool
#define ExAllocatePool(type, size) \
        ExAllocatePoolWithTag(type, size, BULKTAG)
#undef  ExFreePool
#define ExFreePool(type) \
        ExFreePoolWithTag(type,BULKTAG)


typedef enum _DEVSTATE {

    USBNotStarted,         // not started
    USBStopped,            // device stopped
    USBWorking,            // started and working
    USBPendingStop,        // stop pending
    USBPendingRemove,      // remove pending
    USBSurpriseRemoved,    // removed by surprise
    USBRemoved             // removed

} DEVSTATE;

typedef enum _QUEUE_STATE {

    HoldRequests,       // device is not started yet
    AllowRequests,      // device is ready to process
    FailRequests        // fail both existing and queued up requests

} QUEUE_STATE;

typedef struct _USB_PIPE_CONTEXT {

    BOOLEAN PipeOpen;

} USB_PIPE_CONTEXT, *PUSB_PIPE_CONTEXTT;

typedef enum _WDM_VERSION {

    Win2kOrBetter = 1,
    WinXpOrBetter,
    WinVistaOrBetter

} WDM_VERSION;

typedef struct _BULKUSB_PIPE_CONTEXT {

    BOOLEAN PipeOpen;

} BULKUSB_PIPE_CONTEXT, *PBULKUSB_PIPE_CONTEXT;

#define MAX_TX_BULK_PIPE_NUM 6
#define MAX_RX_BULK_PIPE_NUM 2

#define NOR_FLASH_SIZE_TYPE_8_BIT		0
#define NOR_FLASH_SIZE_TYPE_8_16_BIT		1
//
// A structure representing the instance information associated with
// this particular device.
//

#define INITIALIZE_PNP_STATE(_Data_)    \
        (_Data_)->DeviceState =  USBNotStarted;\
        (_Data_)->PrevDevState = USBNotStarted;

#define SET_NEW_PNP_STATE(_Data_, _state_) \
        (_Data_)->PrevDevState =  (_Data_)->DeviceState;\
        (_Data_)->DeviceState = (_state_);

#define RESTORE_PREVIOUS_PNP_STATE(_Data_)   \
        (_Data_)->DeviceState =   (_Data_)->PrevDevState;

#endif

#endif

