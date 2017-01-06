/*++

    THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
    KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
    IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A PARTICULAR
    PURPOSE.

Module Name:

    adapter.h

Abstract:

    adapter.h defines the data types used in the different stages of the function
    driver.

Environment:

    Kernel mode

--*/

#if !defined(_ADAPTER_H_)
#define _ADAPTER_H_
#ifdef RTMP_SDIO_SUPPORT
typedef	union	_INTR_STATUS_T	{
	struct{
		UINT32 TxDoneInt:1;
		UINT32 Rx0DoneInt:1;
		UINT32 Rx1DoneInt:1;
		UINT32 Reserved:3;
		UINT32 AbnormalInt:1;
		UINT32 FwOwnBackInt:1;
		UINT32 D2h_Sw_Int:24;
	}field;
	UINT32 WHISRVal;
}	INTR_STATUS_T, *P_INTR_STATUS_T;


typedef struct _FirmwareInfo
{
	UINT16	rx_pkt_len;
	UINT16	fifo_free_bytes; // = fifo_free_cnt x 16
	UINT8	TxPacketCount; //how many sdio packets in Tx, should not great than 7
	UINT8	BlockMode;  //Send data transactions in byte mode (0) or block mode (1)
	UINT16	BlockLength;  //should not great than 2080    
	KMUTEX	hMutex; 
}FirmwareInfo;

typedef struct _SDIOHeader
{
	USHORT packetlen;
	USHORT reserved;
}SDIOHeader;
#endif

typedef struct _RTMP_ADAPTER {
/*========================================================*/
/*=================SDIO pAdapter==============================*/
/*========================================================*/

#ifdef RTMP_SDIO_SUPPORT	
	WDFDEVICE               WdfDevice;
	
	WDFQUEUE   wdfDefaultQueue;//Queue for default

	WDFQUEUE   wdfWriteQueue;  //Queue for Write

	//KMUTEX hMutexWriteQueue;// for sync WriteQueue.

	PDEVICE_OBJECT              NextDeviceObject;       // NextDevice Object
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
    	UINT32 BlockLength;

	//for interrupt
	INTR_STATUS_T currentIntrStatus;

	WDFREQUEST    PendingWriteRequest;

 	FirmwareInfo firmwareinfo;
    
	KMUTEX hMutexRead;// for sync read.
	KMUTEX hMutexWrite;// for sync write.

	IO_REMOVE_LOCK io_remove_lock;

	BOOLEAN useSingleTx; //use singleTx or OverLap
	PIRP			pSDIrp;

	BOOLEAN TxSDRunning;
	KEVENT  				LoopBackSDIOTxEvent;   	// for cancelling bulk Out IRPs.

	UINT32	TxPageCount;

	KSPIN_LOCK TXLock;

#endif  // RTMP_SDIO_SUPPORT

/*========================================================*/
/*=================USB pAdapter==============================*/
/*========================================================*/
#ifdef RTMP_USB_SUPPORT
	 // Functional Device Object
	PDEVICE_OBJECT FunctionalDeviceObject;

	// Device object we call when submitting Urbs
	PDEVICE_OBJECT TopOfStackDeviceObject;

	// The bus driver object
	PDEVICE_OBJECT PhysicalDeviceObject;

	// Name buffer for our named Functional device object link
	// The name is generated based on the driver's class GUID
	UNICODE_STRING InterfaceName;

	// Bus drivers set the appropriate values in this structure in response
	// to an IRP_MN_QUERY_CAPABILITIES IRP. Function and filter drivers might
	// alter the capabilities set by the bus driver.
	DEVICE_CAPABILITIES DeviceCapabilities;

	// Configuration Descriptor
	PUSB_CONFIGURATION_DESCRIPTOR UsbConfigurationDescriptor;

	// Interface Information structure
	PUSBD_INTERFACE_INFORMATION UsbInterface;

	// Pipe context for the bulkusb driver
	PBULKUSB_PIPE_CONTEXT PipeContext;

	// current state of device
	DEVSTATE DeviceState;

	// state prior to removal query
	DEVSTATE PrevDevState;

	// obtain and hold this lock while changing the device state,
	// the queue state and while processing the queue.
	KSPIN_LOCK DevStateLock;

	// current system power state
	SYSTEM_POWER_STATE SysPower;

	// current device power state
	DEVICE_POWER_STATE DevPower;

	// Pending I/O queue state
	QUEUE_STATE QueueState;

	// Pending I/O queue
	LIST_ENTRY NewRequestsQueue;

	// I/O Queue Lock
	KSPIN_LOCK QueueLock;

	KEVENT RemoveEvent;

	KEVENT StopEvent;
    
	ULONG OutStandingIO;

	KSPIN_LOCK IOCountLock;

	PUSB_IDLE_CALLBACK_INFO IdleCallbackInfo;
	PIRP PendingIdleIrp;
	LONG IdleReqPend;

	LONG FreeIdleIrpCount;

	KSPIN_LOCK IdleReqStateLock;

	KEVENT NoIdleReqPendEvent;

	//default power state to power down to on self-susped
	ULONG PowerDownLevel;
    
	// remote wakeup variables
	PIRP WaitWakeIrp;

	LONG FlagWWCancel;

	LONG FlagWWOutstanding;    

	LONG FlagWWDispatched;

	// open handle count
	LONG OpenHandleCount;

	// This event is cleared when a DPC/Work Item is queued.
	// and signaled when the work-item completes.
	// This is essential to prevent the driver from unloading
	// while we have DPC or work-item queued up.
	//KEVENT NoDpcWorkItemPendingEvent;

	// WMI information
	WMILIB_CONTEXT WmiLibInfo;

	// WDM version
	WDM_VERSION WdmVersion;

	//--------------------------------------------------------------------------
	PUSBD_PIPE_INFORMATION UsbBulkInPipeInfo;
	PUSBD_PIPE_INFORMATION UsbBulkOutPipeInfo;
	
	USBD_PIPE_HANDLE UsbBulkInPipeHandle[MAX_RX_BULK_PIPE_NUM];	// Handle to input Bulk pipe
	USBD_PIPE_HANDLE UsbBulkOutPipeHandle[MAX_TX_BULK_PIPE_NUM];	// Handle to Output Bulk pipe
	
	USHORT                   UsbBulkOutMaxPacketSize;
	USHORT                   UsbBulkInMaxPacketSize;
	
	PURB				pRxUrb[MAX_RX_BULK_PIPE_NUM];
	PIRP					pRxIrp[MAX_RX_BULK_PIPE_NUM];
	PUCHAR				RxBuffer[MAX_RX_BULK_PIPE_NUM];
	ULONG				ulBulkInRunning[MAX_RX_BULK_PIPE_NUM];

	PURB				pTxUrb[MAX_TX_BULK_PIPE_NUM];
	PIRP					pTxIrp[MAX_TX_BULK_PIPE_NUM];
	PUCHAR				TxBuffer[MAX_TX_BULK_PIPE_NUM];
	ULONG				TxBufferLength[MAX_TX_BULK_PIPE_NUM];
	BOOLEAN				bBulkOutRunning[MAX_TX_BULK_PIPE_NUM];	

	KEVENT  				CancelRxIrpEvent[MAX_RX_BULK_PIPE_NUM];      // for cancelling bulk In IRPs.
	KEVENT  				CancelTxIrpEvent[MAX_TX_BULK_PIPE_NUM];   	// for cancelling bulk Out IRPs.

	ULONG				TxBulkCount[MAX_TX_BULK_PIPE_NUM];
	LARGE_INTEGER		ReceivedCount[MAX_RX_BULK_PIPE_NUM];	
	COUNTER_802_11		WlanCounters;
		
	//BOOLEAN				ContinBulkOut;
	//ULONG				BulkOutRemained;
	ULONG				BulkOutWhichPipeNeedRun;
	ULONG				BulkOutTxType;

	KSPIN_LOCK 			BulkInIoCallDriverSpinLock[MAX_RX_BULK_PIPE_NUM];
	BOOLEAN				bBulkInIoCallDriverFlag[MAX_RX_BULK_PIPE_NUM];	
	BOOLEAN				ContinBulkIn[MAX_RX_BULK_PIPE_NUM];
	USHORT				BulkInRemained[MAX_RX_BULK_PIPE_NUM];

	KSPIN_LOCK 			RxSpinLock[MAX_RX_BULK_PIPE_NUM];		// Spin lock for sniffer momory move
	BOOLEAN				bRxEnable[MAX_RX_BULK_PIPE_NUM];
	BOOLEAN				bNextVLD; 	

	//RX MPDU
	PUCHAR				RXMPDUBuffer;	

	// Resource allocations
	ULONG 				Vector;
	KAFFINITY 			Affinity;

	ULONG				ShiftReg;
	KEVENT  				LoopBackTxEvent[MAX_TX_BULK_PIPE_NUM];   	// for cancelling bulk Out IRPs.
	UINT32				IrpLock[MAX_RX_BULK_PIPE_NUM];
#endif	//RTMP_USB_SUPPORT

	/*========================================================*/
	/*=================pAdapter common part========================*/
	/*========================================================*/
	//for Efuse using
	UINT8	EfuseMode;// Buffer:0, Efuse:1, EEPROM:2
	UCHAR	EfuseContent[EFUSESIZE];

	//FW	
	UCHAR		FWImage[MAXFIRMWARESIZE];//From UI
	UINT32		FWSize;//From UI
	BOOLEAN		IsFWImageInUse;
	BOOLEAN		IsUISetFW;
	UINT32		LoadingFWCount;

	BOOLEAN		bLoadingFW;
	KEVENT		LoadFWEvent;
	KEVENT		WaitFWEvent;
	BOOLEAN		bIsWaitFW;

	KEVENT		FWCREvent;

	KEVENT		RomPatchEvent;
	BOOLEAN		bIsWaitRomPatch;
	UINT8		RomPatchSemStatus;	
	UINT32		FwCmdSeqNum;

	UCHAR 		ScatterBuf[SCATTER_SIZE];

	UCHAR		TxCmdBuffer[BUFFER_SIZE];
	UINT32		TxCmdBufferLength;

	OTHER_STATISTICS	OtherCounters;
// Avg Rssi	
	//INT32				AvgRssi0;
	//INT32				AvgRssi1;
	INT32				RecordRssi0[RXV_RSSI_LIMIT];
	INT32				RecordRssi1[RXV_RSSI_LIMIT];
	//INT32				RecordRssi2[];
	//INT32				RecordRssi3[];
	UINT32				RssiCount;
	UINT32				RssiLimit;
// ~Avg Rssi
	UINT32				SecurityFail ;
	BOOLEAN				IsTxSetFrequency;// check if is if first time to run TxSetFrequencyOffset
	UINT32		MACVersion;	
	UINT32		ChipID;
	UINT8		FWMode;
	BOOLEAN		FWon;	

	UINT32				TransmittedCount;

	//FW
	
	UINT32		RegValue;//FW cmd use
	UINT32		RegAddress;//FW cmd use
	UINT32		RegValueRead;//FW cmd use
	UINT8		FWRspStatus;
	UINT8		PacketCMDSeqMCU;
	UCHAR		FWRspContent[PKTCMD_EVENT_BUFFER];
	UINT32		FWRspContentLength;

/*=================Tx Data==================================*/
	UCHAR		TxDataBuffer[BUFFER_SIZE];
	UINT32		TxDataBufferLength;
	//UINT16		bMinLen;
	//UINT16		bCurrlength;
	UINT32		TxRemained[MAX_TX_BUFEER_NUM];
	KEVENT		TxDataEvent;
	BOOLEAN		TxProcessing;
	BOOLEAN				Seq_dbg;
	HANDLE		TxDataThread;//current sdio
	BOOLEAN		TxTheradRunning;
	KSPIN_LOCK 	TxLock;
/*=================Tx Data==================================*/
/*=================Tx BF==================================*/

	PFMU_PROFILE prof;
	BOOLEAN		iTxBf;
	BOOLEAN		eTxBf;
	PFMU_DATA	prePfmuDataBW20[255];
	PFMU_DATA	prePfmuDataBW40[255];
	PFMU_DATA	postPfmuDataBW20[255];
	PFMU_DATA	postPfmuDataBW40[255];
	BOOLEAN		IsBfStored;
/*=================Tx BF==================================*/
/*=================LoopBack==================================*/
	BOOLEAN				LoopBackRunning;
	BOOLEAN				LoopBackWaitRx;
	LOOPBACK_RESULT	LoopBackResult;
	LOOPBACK_SETTING	LoopBackSetting;
	UINT8				LoopBackBulkoutNumber;
	UCHAR				LoopBackTxRaw[LOOPBACK_SIZE];
	UCHAR				LoopBackRxRaw[LOOPBACK_SIZE];
	UINT32				LoopBackTxRawLen;
	UINT32				LoopBackRxRawLen;
	UINT32				LoopBackExpectTxLen;
	UINT32				LoopBackExpectRxLen;
	UCHAR				LoopBackExpectTx[LOOPBACK_SIZE];
	UCHAR				LoopBackExpectRx[LOOPBACK_SIZE];		
	KSPIN_LOCK 			LoopBackLock;
	BOOLEAN				LoopBackDefaultPattern;
	HANDLE				LoopBackTxThread;
	KEVENT				LoopBackEvent;
/*=================LoopBack==================================*/
	UINT32				BackupCR[BACKUPCRSIZE];

}  RTMP_ADAPTER, *PRTMP_ADAPTER;

NTSTATUS InitAdapterCommon(RTMP_ADAPTER *pAd);
NTSTATUS InitAdapterInterface(RTMP_ADAPTER *pAd);
NTSTATUS InitAdapterEvent(RTMP_ADAPTER *pAd);
NTSTATUS GetChipID(RTMP_ADAPTER *pAd);
NTSTATUS ReadReg(RTMP_ADAPTER *pAd,UINT32 Offset, UINT32 *pValue, UINT32 Length);
NTSTATUS WriteReg(RTMP_ADAPTER *pAd,UINT32 Offset, UINT32 Value);

#endif  // _ADAPTER_H_


