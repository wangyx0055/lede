#ifndef _USB_IF_H 
#define _USB_IF_H

typedef enum{
	EP4QID = 0,
	EP5QID,
	EP6QID,
	EP7QID,
	EP8QID = 8,
	EP9QID,	
	EPALLQID
}EPQID;


#define VendorReqIn		1
#define VendorReqOut	0

NTSTATUS	USBVendorRequest(
	IN	RTMP_ADAPTER	*pAdapter,
	IN	PVOID			pData,
	IN	UINT16			Length,
	IN	UINT8			Request,
	IN	UINT8			Value,
	IN	UINT8			index,
	IN	BOOLEAN			InOut);

NTSTATUS	FwCmdUSBBulkout(
	IN	RTMP_ADAPTER	*pAdapter,
	IN	PUCHAR			pCmd,
	IN	UINT32			Length);

NTSTATUS USBGetChipID(RTMP_ADAPTER *pAd);
NTSTATUS USBInitAdapter(RTMP_ADAPTER *pAd);
NTSTATUS USBInitInterface(RTMP_ADAPTER *pAd);
NTSTATUS USBSetupTxPacket(RTMP_ADAPTER *pAd, UCHAR *pData, UINT32 Length, UINT32 BulkOutWhichPipeNeedRun);
NTSTATUS USBStartTxPacket(RTMP_ADAPTER *pAd, UINT32 BulkOutWhichPipeNeedRun, UINT32 TxRemained);
NTSTATUS USBStopTxPacket(RTMP_ADAPTER *pAd);
NTSTATUS RXStartStop(IN  RTMP_ADAPTER   *pAdapter, IN  BOOLEAN 	bIsStart, IN  UINT16	Pipe);
UINT8 GetBulkoutNumber(IN  RTMP_ADAPTER   *pAdapter);
NTSTATUS USBCheckFWROMWiFiSysOn(IN  RTMP_ADAPTER   *pAd);
void USBLoopBack_Run(RTMP_ADAPTER *pAd, LOOPBACK_SETTING *pSetting, UINT32 length);
NTSTATUS ConfigDMAScheduler(RTMP_ADAPTER *pAd, UINT8 Qid );
#endif //_USB_IF_H