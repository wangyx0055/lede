/*++

Copyright (c) Microsoft Corporation.  All rights reserved.

    THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
    KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
    IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A PARTICULAR
    PURPOSE.

Module Name:

    sdio.h

Abstract:

    sdio.h defines the data types used in the different stages of the function
    driver.

Environment:

    Kernel mode

--*/


#if !defined(_SDIO_H_)
#define _SDIO_H_

#pragma warning(disable:4100 4057)
#if 0
#include <ntddk.h>
#include <wdf.h>
#include <initguid.h> // required for GUID definitions
//
// Disables few warnings so that we can build our driver with MSC W4 level.
// Disable warning C4057; X differs in indirection to slightly different base types from Y
// Disable warning C4100: unreferenced formal parameter
//


#include <ntddsd.h>

//#include "sdio_read.h"
//#include "sdio_write.h"
#endif
//#include "bufpool.h"
//#include "sleepstatus.h"

#define SDIO_POOL_TAG (ULONG) 'sraM'

#define WRITETODEVICE  TRUE
#define READFROMDEVICE FALSE

#define FIFOMODE      TRUE
#if 0
//for CCCR
#define CCCR_CCCRx            (0x00)   // CCCR/SDIO revison
#define CCCR_SDx              (0x01)   // SD specification revison
#define CCCR_IOEX             (0x02)   // I/O enable
#define CCCR_IORX             (0x03)   // I/O Ready
#define CCCR_IENX             (0x04)   // int enable
#define CCCR_INTX             (0x05)   // int pending
#define CCCR_ASX              (0x06)   // Abort Select
#define CCCR_BIC              (0x07)   // Bus interface control
#define CCCR_CARDCAPABILITY   (0x08)   // Card Capability
#endif

// for WiFi HIF
#define WCIR		(0x0000) //WLAN Chip ID Register
#define CHIP_ID_MASK (0xffff)
#define GET_CHIP_ID(p) (((p) & CHIP_ID_MASK))
#define REVISION_ID_MASK (0xf << 16)
#define GET_REVISION_ID(p) (((p) & REVISION_ID_MASK) >> 16)
#define POR_INDICATOR (1 << 20)
#define GET_POR_INDICATOR(p) (((p) & POR_INDICATOR) >> 20)
#define W_FUNC_RDY (1 << 21)
#define GET_W_FUNC_RDY(p) (((p) & W_FUNC_RDY) >> 21)
#define DEVICE_STATUS_MASK (0xff << 24)
#define GET_DEVICE_STATUS(p) (((p) & DEVICE_STATUS_MASK) >> 24)
//WHLPCR
#define WHLPCR		(0x0004) //WLAN HIF Low Power Control Register
#define W_INT_EN_SET (1 << 0)
#define W_INT_EN_CLR (1 << 1)
#define W_FW_OWN_REQ_SET (1 << 8)
#define GET_W_FW_OWN_REQ_SET(p) (((p) & W_FW_OWN_REQ_SET) >> 8)
#define W_FW_OWN_REQ_CLR (1 << 9)
//WSDIOCSR
#define WSDIOCSR	(0x0008) //WLAN SDIO Control Status Register (SDIO Only)
//WHCR
#define WHCR		(0x000c) //WLAN HIF Control Register
#define W_INT_CLR_CTRL (1 << 1)
#define RECV_MAILBOX_RD_CLR_EN (1 << 2)
#define RPT_OWN_RX_PACKET_LEN (1 << 3)
#define MAX_HIF_RX_LEN_NUM_MASK (0x3f << 8)
#define MAX_HIF_RX_LEN_NUM(p) (((p) & 0x3f) << 8)
#define GET_MAX_HIF_RX_LEN_NUM(p) (((p) & MAX_HIF_RX_LEN_NUM_MASK) >> 8)
#define RX_ENHANCE_MODE (1 << 16)
//WHISR
#define WHISR		(0x0010) //WLAN HIF Interrupt Status Register
#define TX_DONE_INT (1 << 0)
#define RX0_DONE_INT (1 << 1)
#define RX1_DONE_INT (1 << 2)
#define ABNORMAL_INT (1 << 6)
#define FW_OWN_BACK_INT (1 << 7)
#define D2H_SW_INT (0xffffff << 8)
#define D2H_SW_INT_MASK (0xffffff << 8)
#define GET_D2H_SW_INT(p) (((p) & D2H_SW_INT_MASK) >> 8)
//WHIER
#define WHIER		(0x0014) //WLAN HIF Interrupt Enable Register
#define TX_DONE_INT_EN (1 << 0)
#define RX0_DONE_INT_EN (1 << 1)
#define RX1_DONE_INT_EN (1 << 2)
#define ABNORMAL_INT_EN (1 << 6)
#define FW_OWN_BACK_INT_EN (1 << 7)
#define D2H_SW_INT_EN_MASK (0xffffff << 8)
#define D2H_SW_INT_EN(p) (((p) & 0xffffff) << 8)
#define GET_D2H_SW_INT_EN(p) (((p) & D2H_SW_INT_EN_MASK) >> 8)

#define WHIER_DEFAULT (TX_DONE_INT_EN | RX0_DONE_INT_EN | RX1_DONE_INT_EN\
						| ABNORMAL_INT_EN\
						| D2H_SW_INT_EN_MASK)
//WASR
#define WASR		(0x0020) //WLAN Abnormal Status Register
#define TX1_OVERFLOW (1 << 1)
#define RX0_UNDERFLOW (1 << 8)
#define RX1_UNDERFLOW (1 << 9)
#define FW_OWN_INVALID_ACCESS (1 << 16)

#define WSICR		(0x0024) //WLAN Software Interrupt Control Register
#define WTDR1		(0x0034) //WLAN TX Data Register 1
#define WRDR0		(0x0050) //WLAN RX Data Register 0
#define WRDR1		(0x0054) //WLAN RX Data Register 1
#define H2DSM0R		(0x0070) //Host to Device Send Mailbox 0 Register
#define H2DSM1R		(0x0074) //Host to Device Send Mailbox 1 Register
#define D2HRM0R		(0x0078) //Device to Host Receive Mailbox 0 Register
#define D2HRM1R		(0x007c) //Device to Host Receive Mailbox 1 Register
//WRPLR
#define WRPLR		(0x0090) //WLAN RX Packet Length Register
#define RX0_PACKET_LENGTH_MASK (0xffff)
#define GET_RX0_PACKET_LENGTH(p) (((p) & RX0_PACKET_LENGTH_MASK))
#define RX1_PACKET_LENGTH_MASK (0xffff << 16)
#define GET_RX1_PACKET_LENGTH(p) (((p) & RX1_PACKET_LENGTH_MASK) >> 16)

#define WOLTCR		(0x00AC) //On Line-Tuning Control Register
#define WTMDR		(0x00B0) //Test Mode Data Port
#define WTMCR		(0x00B4) //Test Mode Control Register

#define WTMDPCR0	(0x00B8) //Test Mode Data Pattern Control Register 0
#define WTMDPCR1	(0x00BC) //Test Mode Data Pattern Control Register 1
#define WPLRCR		(0x00D4) //WLAN Packet Length Report Control Register
#define WSR		(0x00D8) //WLAN Snapshot Register
#define CLKIOCR_T55LP	(0x0100) //Clock Pad Macro IO Control Register
#define CMDIOCR_T55LP	(0x0104) //Command Pad Macro IO Control Register
#define DAT0IOCR_T55LP	(0x0108) //Data 0 Pad Macro IO Control Register
#define DAT1IOCR_T55LP	(0x010c) //Data 1 Pad Macro IO Control Register
#define DAT2IOCR_T55LP	(0x0110) //Data 2 Pad Macro IO Control Register
#define DAT3IOCR_T55LP	(0x0114) //Data 3 Pad Macro IO Control Register
#define CLKDLYCR		(0x0118) //Clock Pad Macro Delay Chain Control Register
#define CMDDLYCR	(0x011C) //Command Pad Macro Delay Chain Control Register
#define ODATDLYCR	(0x0120) //SDIO Output Data Delay Chain Control Register
#define IDATDLYCR1	(0x0124) //SDIO Input Data Delay Chain Control Register 1
#define IDATDLYCR2	(0x0128) //SDIO Input Data Delay Chain Control Register 2
#define ILCHCR		(0x012C) //SDIO Input Data Latch Time Control Register
#define WTQCR0		(0x0130) //WLAN TXQ Count Register 0
#define WTQCR1		(0x0134) //WLAN TXQ Count Register 1
#define WTQCR2		(0x0138) //WLAN TXQ Count Register 2
#define WTQCR3		(0x013C) //WLAN TXQ Count Register 3
#define WTQCR4		(0x0140) //WLAN TXQ Count Register 4
#define WTQCR5		(0x0144) //WLAN TXQ Count Register 5
#define WTQCR6		(0x0148) //WLAN TXQ Count Register 6
#define WTQCR7		(0x014C) //WLAN TXQ Count Register 7

#define TX_PAGE_SIZE	128
#define TX_PAGE_NUM	684
#if 0
// for Common HIF (common: BT/FM/GP)
#define CCIR		(0x0000)   //Common Chip ID Register
#define CHLPCR		(0x0004)   //Common HIF Low Power Control Register
#define CSDIOCSR	(0x0008)   //Common SDIO Control Status Register
#define CHCR		(0x000c)   //Common HIF Control Register
#define CHISR		(0x0010)   //Common HIF Interrupt Status Register
#define CHIER		(0x0014)   //Common HIF Interrupt Enable Register
#define CTDR		(0x0018)   //Common TX Data Register
#define CRDR		(0x001c)   //Common RX data register
#define CTFSR		(0x0020)   //Common TX FIFO Status Register
#define CRPLR		(0x0024)   //Common RX Packet Length Register

#ifndef BIT
#define BIT(n)                          ((UINT_32) 1 << (n))
#define BITS2(m,n)                      (BIT(m) | BIT(n) )
#define BITS3(m,n,o)                    (BIT(m) | BIT (n) | BIT (o))
#define BITS4(m,n,o,p)                  (BIT(m) | BIT (n) | BIT (o) | BIT(p))

/* bits range: for example BITS(16,23) = 0xFF0000
 *   ==>  (BIT(m)-1)   = 0x0000FFFF     ~(BIT(m)-1)   => 0xFFFF0000
 *   ==>  (BIT(n+1)-1) = 0x00FFFFFF
 */
#define BITS(m,n)                       (~(BIT(m)-1) & ((BIT(n) - 1) | BIT(n)))
#endif /* BIT */
#endif
#if 0
//
// The FDO_DATA structure describes the SDIO sample function driver's device
// extension. The device extension is where all per-device-instance information
// is kept.
//

typedef struct _FDO_DATA {

    WDFDEVICE               WdfDevice;

//    WDFQUEUE                IoctlQueue;

	WDFQUEUE   wdfDefaultQueue;//Queue for default

	WDFQUEUE   wdfWriteQueue;  //Queue for Write

	KMUTEX hMutexWriteQueue;// for sync WriteQueue.
    //
    // Context for SD BUS api
    //
    SDBUS_INTERFACE_STANDARD BusInterface;
    //
    // Driver version
    //
    USHORT DriverVersion;
    //
    // Function number on SD card
    //
    UCHAR FunctionNumber;

    //
    // Target function for I/O transactions (not necessarily our function#)
    //
    UCHAR FunctionFocus;

    //
    // Send data transactions in byte mode (0) or block mode (1)
    //
    UCHAR BlockMode;
    
	ULONG Addr;// Access Address for read
	BOOLEAN NoisyMode;  //currently no use
	ULONG intCount;     //currently no use
	//for interrupt
	INTR_STATUS_T currentIntrStatus;
	// mutual reference
    	RTMP_ADAPTER *pAdapter;
	//P_ADAPTER_T prAdapter;
	//WRITE_CONTEXT writeContext;

	//BUFFERPOOL  bufferpool; //buffer pool for Read
	
	WDFREQUEST    PendingWriteRequest;
	KMUTEX hMutexWriteRequest; 

 	FirmwareInfo firmwareinfo;
 
	//SLEEPSTATUS sleepstatus;

	KMUTEX hMutexRead;// for sync read.
	KMUTEX hMutexWrite;// for sync write.
    
	//
	IO_REMOVE_LOCK io_remove_lock;

	BOOLEAN useSingleTx; //use singleTx or OverLap
	BOOLEAN RX0;
	BOOLEAN RX1;
}  FDO_DATA, *PFDO_DATA;


WDF_DECLARE_CONTEXT_TYPE_WITH_NAME(FDO_DATA, SDIOFdoGetData)
#endif	
WDF_DECLARE_CONTEXT_TYPE_WITH_NAME(RTMP_ADAPTER, SDIOpAdGetData)

//
// Declare the function prototypes for all of the function driver's routines in all
// the stages of the function driver.
//

DRIVER_INITIALIZE DriverEntry;

EVT_WDF_DRIVER_UNLOAD  DriverUnload;

EVT_WDF_DRIVER_DEVICE_ADD SDIOEvtDeviceAdd;
/*
NTSTATUS
SDIOEvtDeviceAdd(
    IN WDFDRIVER Driver,
    IN PWDFDEVICE_INIT DeviceInit
    );
*/

EVT_WDF_DEVICE_PREPARE_HARDWARE SDIOEvtDevicePrepareHardware;
/*
NTSTATUS
SDIOEvtDevicePrepareHardware(
    WDFDEVICE Device,
    WDFCMRESLIST Resources,
    WDFCMRESLIST ResourcesTranslated
    );
*/

EVT_WDF_DEVICE_RELEASE_HARDWARE SDIOEvtDeviceReleaseHardware;
/*
NTSTATUS
SDIOEvtDeviceReleaseHardware(
    IN  WDFDEVICE Device,
    IN  WDFCMRESLIST ResourcesTranslated
    );
*/


//
// Io events callbacks.
//

EVT_WDF_IO_QUEUE_IO_DEVICE_CONTROL SDIOEvtIoDeviceControl;
/*
VOID
SDIOEvtIoDeviceControl(
    IN WDFQUEUE Queue,
    IN WDFREQUEST Request,
    IN size_t OutputBufferLength,
    IN size_t InputBufferLength,
    IN ULONG IoControlCode
    );
*/
EVT_WDF_IO_QUEUE_IO_STOP DftEvtIoStop;
EVT_WDF_IO_QUEUE_IO_STOP EvtIoStopWrite;
EVT_WDF_REQUEST_CANCEL  EvtRequestCancelWrite;
EVT_WDF_IO_QUEUE_IO_READ SDIOEvtIoRead;
/*
VOID
SDIOEvtIoRead (
    WDFQUEUE      Queue,
    WDFREQUEST    Request,
    size_t         Length
    );
*/

EVT_WDF_IO_QUEUE_IO_WRITE SDIOEvtIoWrite;
/*
VOID
SDIOEvtIoWrite (
    WDFQUEUE      Queue,
    WDFREQUEST    Request,
    size_t         Length
    );
*/

SDBUS_CALLBACK_ROUTINE SDIOEventCallback;
/*
VOID
SDIOEventCallback(
    IN PVOID Context,
    IN ULONG InterruptType
    );
*/
/*
NTSTATUS
DriverEntry_SDIO(
           IN PDRIVER_OBJECT  DriverObject,
           IN PUNICODE_STRING RegistryPath
           );
           */
/*
NTSTATUS
SdioReadWriteBuffer(
    IN WDFDEVICE Device,
    IN ULONG Function,
    IN PMDL Mdl,
    IN ULONG Address,
    IN ULONG Length,
    IN BOOLEAN WriteToDevice,
    OUT PULONG BytesRead
    );
*/
NTSTATUS
SdioReadWriteByte(
    IN WDFDEVICE Device,
    IN ULONG Function,
    IN PUCHAR Data,
    IN ULONG Address,
    IN BOOLEAN WriteToDevice
    );

NTSTATUS
SdioGetProperty(
    IN WDFDEVICE Device,
    IN SDBUS_PROPERTY Property,
    IN PVOID Buffer,
    IN ULONG Length
    );

NTSTATUS
SdioSetProperty(
    IN WDFDEVICE Device,
    IN SDBUS_PROPERTY Property,
    IN PVOID Buffer,
    IN ULONG Length
    );

NTSTATUS
SdioReadWriteBuffer_BlockMode(
                   IN WDFDEVICE Device,
                   IN ULONG Function,
                   IN PMDL Mdl,
                   IN ULONG Address,
                   IN ULONG Length,
                   IN BOOLEAN FIFOMode,
                   IN BOOLEAN WriteToDevice,
                   OUT PULONG BytesRead,
	     IN BOOLEAN BlockMode
                   );

NTSTATUS
SdioReadWriteBuffer(
    IN WDFDEVICE Device,
    IN ULONG Function,
    IN PMDL Mdl,
    IN ULONG Address,
    IN ULONG Length,
    IN BOOLEAN FIFOMode,
    IN BOOLEAN WriteToDevice,
    OUT PULONG BytesRead
    );

VOID WaitMicroSecond1(ULONG ulMicroSecond);

ULONG NBytesAlign(ULONG len, ULONG nBytesAlign);

NTSTATUS CreateQueues(IN RTMP_ADAPTER *pAdapter);

VOID DispatchAndHandleInterrupt(IN RTMP_ADAPTER *pAdapter);

INT SDIO_init(RTMP_ADAPTER *pAdapter);

NTSTATUS 
SDIO_GET_INTR(
    RTMP_ADAPTER *pAdapter, P_INTR_STATUS_T ret);

NTSTATUS 
RTMP_SDIO_READ32(
    RTMP_ADAPTER *pAdapter,
    UINT32 u4Offset,
    PUINT32 pu4Value);

NTSTATUS 
RTMP_SDIO_WRITE32(
     RTMP_ADAPTER *pAdapter,
    UINT32 u4Offset,
    UINT32 u4Value);

NTSTATUS MTSDIOProcessRxInterrupt(RTMP_ADAPTER *pAdapter);

NTSTATUS MTSDIOSingleProcessRx0(RTMP_ADAPTER *pAdapter, INT32 Length);

NTSTATUS MTSDIOSingleProcessRx1(RTMP_ADAPTER *pAdapter, INT32 Length);

NTSTATUS MTSDIOProcessAbnormalInterrupt(RTMP_ADAPTER *pAdapter);

NTSTATUS MTSDIOProcessTxInterrupt(RTMP_ADAPTER *pAdapter);

NTSTATUS SdioReadWriteBuffer_BlockMode_Async(
				IN WDFDEVICE Device,
			                   IN ULONG Function,
			                   IN PMDL Mdl,
			                   IN ULONG Address,
			                   IN ULONG Length,
			                   IN BOOLEAN FIFOMode,
			                   IN BOOLEAN WriteToDevice,
			                   OUT PULONG BytesRead,
				     IN BOOLEAN BlockMode
			                   );
NTSTATUS SdioReadWriteBuffer_BlockMode_Async_Complete(IN PDEVICE_OBJECT DeviceObject, IN PIRP pIrp, IN PVOID Context);
NTSTATUS SDIOGetChipID(RTMP_ADAPTER *pAd);
NTSTATUS SDIOInitAdapter(RTMP_ADAPTER *pAd);
NTSTATUS SDIOInitInterface(RTMP_ADAPTER *pAd);
NTSTATUS SDIOSetupTxPacket(RTMP_ADAPTER *pAd, UCHAR *pData, UINT32 Length, UINT32 BufferNum);
NTSTATUS SDIOStartTxPacket(RTMP_ADAPTER *pAd, UINT32 txBufferNum, UINT32 TxRemained);
NTSTATUS SDIOStopTxPacket(RTMP_ADAPTER *pAd);
NTSTATUS SDIOCheckFWROMWiFiSysOn(RTMP_ADAPTER *pAd);
void SDIOLoopBack_Run(RTMP_ADAPTER *pAd, LOOPBACK_SETTING *pSetting, UINT32 length);

// {1960C286-26E7-453f-9567-A74E61D59B73}
DEFINE_GUID(GUID_QA_MT7603S, 
0x1960c286, 0x26e7, 0x453f, 0x95, 0x67, 0xa7, 0x4e, 0x61, 0xd5, 0x9b, 0x73);

#define IOCTL_SDIO_GET_DRIVER_VERSION \
            CTL_CODE( FILE_DEVICE_UNKNOWN, 0x780, METHOD_BUFFERED, FILE_ANY_ACCESS)

#define IOCTL_SDIO_GET_FUNCTION_NUMBER \
            CTL_CODE( FILE_DEVICE_UNKNOWN, 0x781, METHOD_BUFFERED, FILE_ANY_ACCESS)

#define IOCTL_SDIO_GET_FUNCTION_FOCUS \
            CTL_CODE( FILE_DEVICE_UNKNOWN, 0x782, METHOD_BUFFERED, FILE_ANY_ACCESS)

#define IOCTL_SDIO_SET_FUNCTION_FOCUS \
            CTL_CODE( FILE_DEVICE_UNKNOWN, 0x783, METHOD_BUFFERED, FILE_ANY_ACCESS)



#define IOCTL_SDIO_GET_BUS_WIDTH \
            CTL_CODE( FILE_DEVICE_UNKNOWN, 0x784, METHOD_BUFFERED, FILE_ANY_ACCESS)
#define IOCTL_SDIO_SET_BUS_WIDTH \
            CTL_CODE( FILE_DEVICE_UNKNOWN, 0x785, METHOD_BUFFERED, FILE_ANY_ACCESS)


#define IOCTL_SDIO_GET_BUS_CLOCK \
            CTL_CODE( FILE_DEVICE_UNKNOWN, 0x786, METHOD_BUFFERED, FILE_ANY_ACCESS)
#define IOCTL_SDIO_SET_BUS_CLOCK \
            CTL_CODE( FILE_DEVICE_UNKNOWN, 0x787, METHOD_BUFFERED, FILE_ANY_ACCESS)


#define IOCTL_SDIO_GET_BLOCKLEN \
            CTL_CODE( FILE_DEVICE_UNKNOWN, 0x788, METHOD_BUFFERED, FILE_ANY_ACCESS)
#define IOCTL_SDIO_SET_BLOCKLEN \
            CTL_CODE( FILE_DEVICE_UNKNOWN, 0x789, METHOD_BUFFERED, FILE_ANY_ACCESS)


#define IOCTL_SDIO_GET_FN0_BLOCKLEN \
            CTL_CODE( FILE_DEVICE_UNKNOWN, 0x78a, METHOD_BUFFERED, FILE_ANY_ACCESS)
#define IOCTL_SDIO_SET_FN0_BLOCKLEN \
            CTL_CODE( FILE_DEVICE_UNKNOWN, 0x78b, METHOD_BUFFERED, FILE_ANY_ACCESS)


#define IOCTL_SDIO_GET_BUS_INTERFACE_CONTROL \
            CTL_CODE( FILE_DEVICE_UNKNOWN, 0x78c, METHOD_BUFFERED, FILE_ANY_ACCESS)
#define IOCTL_SDIO_SET_BUS_INTERFACE_CONTROL \
            CTL_CODE( FILE_DEVICE_UNKNOWN, 0x78d, METHOD_BUFFERED, FILE_ANY_ACCESS)

#define IOCTL_SDIO_GET_INT_ENABLE \
            CTL_CODE( FILE_DEVICE_UNKNOWN, 0x78e, METHOD_BUFFERED, FILE_ANY_ACCESS)
#define IOCTL_SDIO_SET_INT_ENABLE \
            CTL_CODE( FILE_DEVICE_UNKNOWN, 0x78f, METHOD_BUFFERED, FILE_ANY_ACCESS)



#define IOCTL_SDIO_READ_BYTE \
            CTL_CODE( FILE_DEVICE_UNKNOWN, 0x7b0, METHOD_BUFFERED, FILE_ANY_ACCESS)

#define IOCTL_SDIO_WRITE_BYTE \
            CTL_CODE( FILE_DEVICE_UNKNOWN, 0x7b1, METHOD_BUFFERED, FILE_ANY_ACCESS)

#define IOCTL_SDIO_SET_TRANSFER_MODE \
            CTL_CODE( FILE_DEVICE_UNKNOWN, 0x7b2, METHOD_BUFFERED, FILE_ANY_ACCESS)

#define IOCTL_SDIO_TOGGLE_MODE \
            CTL_CODE( FILE_DEVICE_UNKNOWN, 0x7b3, METHOD_BUFFERED, FILE_ANY_ACCESS)

#define IOCTL_SDIO_TOGGLE_NOISY \
            CTL_CODE( FILE_DEVICE_UNKNOWN, 0x7b4, METHOD_BUFFERED, FILE_ANY_ACCESS)
#endif  // _SDIO_H_


