#ifndef _SDIO_WRITE_H_
#define _SDIO_WRITE_H_

#define MAX_TX_PACKET_COUNT  7
#define MAX_TX_FIFO_SIZE  2048  //bytes
//SDIO request irp
#define SDIO_ASYNC	1
#define SDIO_SYNC	0

BOOLEAN IsQueueEmpty(IN WDFQUEUE Queue);

size_t GetPaddingSizeByMode(RTMP_ADAPTER *pAdapter, size_t originalLen );

BOOLEAN IsChipWriteable(RTMP_ADAPTER *pAdapter, size_t originalLen);

VOID WriteTx(RTMP_ADAPTER *pAdapter, WDFREQUEST Request, size_t Length);

VOID IOWriteTx(RTMP_ADAPTER *pAdapter, PUCHAR pBuffer, size_t Length, BOOLEAN Async);
 
VOID ContinuousWriteTx(RTMP_ADAPTER *pAdapter);

NTSTATUS SDIO_TxData(RTMP_ADAPTER *pAdapter);

void Tx_DataTherad(IN OUT PVOID Context);

typedef struct _REQUEST_CONTEXT {
	NTSTATUS    status;
	RTMP_ADAPTER *pAdapter;
	//PVOID RefCount;
	size_t Length;
	PVOID SystemBuffer;
	ULONG_PTR Information;
	//WDFREQUEST RequestPre;
	//WDFREQUEST	RequestSub;
	//BOOLEAN bDowntoOne;
	//BOOLEAN bUseIntervalTimer;
	//PFN_WDF_REQUEST_CANCEL CancelRoutine;
	//ULONG IoctlCode;
} REQUEST_CONTEXT, *PREQUEST_CONTEXT;

#define STATUS_DEVICE_IS_SLEEPING        0xE0000001L

WDF_DECLARE_CONTEXT_TYPE_WITH_NAME(REQUEST_CONTEXT, GetRequestContext)

#endif  // _SDIO_WRITE_H_
