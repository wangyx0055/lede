
#ifndef _FW_CMD_H 
#define _FW_CMD_H

//for CMD_ACCESS_REG
#define WRITE			0x01
#define READ			0x00
//Rom cmd
#define CMD_START_LOAD		0x01
#define CMD_RAM_START_RUN	0x02
#define CMD_ROM_ACCESS_REG			0x03
#define CMD_PATCH_SEMAPHORE_CONTROL		0x10
#define CMD_PATCH_START			0x05
#define CMD_PATCH_FINISH			0x07
#define CMD_LOOPBACK_TEST			0x20

//Rom event 
#define INIT_EVENT_CMD_RESULT_SUCCESS		0x00
#define INIT_EVENT_CMD_RESULT			0x01
#define INIT_EVENT_ACCESS_REG			0x02
#define INIT_EVENT_PATCH_SEMA_CTRL			0x04

//Ram cmd
#define CMD_RESTART_DOWNLOAD			0xEF
#define CMD_EXT_CMD				0xED
#define CMD_RAM_ACCESS_REG			0xC2

//Ram Ext cmd
#define EXT_CMD_MULTIPLE_REG_ACCESS		0x0E
//Ram event 
#define EVENT_RESTART_DONWLOAD		0xEF
#define EVENT_EXT_CMD				0xED

//Ram Ext event
#define EXT_EVENT_MULTI_CR_ACCESS		0x0E
#define EXT_EVENT_TXBF_SOUNDING		0x0

typedef enum _ENUM_FW_MODE
{
	FWROM = 0,
	FWRAM
} ENUM_FW_MODE, *P_ENUM_FW_MODE;

typedef enum _ENUM_CR_TYPE
{
	MAC_CR= 0,
	RF_CR
} ENUM_CR_TYPE, *P_ENUM_CR_TYPE;

//FW struct
//
// 
typedef struct _CMD_EFUSE_ACCESS
{
	FW_RAM_TXDSCR FwTxD;
	UINT32	Address;
	UINT32	Valid;
	UINT8	Data[16];	
} RAM_CMD_EFUSE_ACCESS, *PRAM_CMD_EFUSE_ACCESS;
#ifdef RTMP_SDIO_SUPPORT


NTSTATUS	Cmd_Access_Reg(
	IN	RTMP_ADAPTER	*pAdapter,
	IN	UINT32			Address,
	IN	UINT32			Value,
	IN	BOOLEAN			IsWrite);

NTSTATUS	Cmd_Ram_Access_Reg(
	IN	RTMP_ADAPTER	*pAdapter,
	IN	UINT32			Address,
	IN	UINT32			Value,
	IN	BOOLEAN			IsWrite);

NTSTATUS	Cmd_SDIO_Write_Reg(
	IN	RTMP_ADAPTER	*pAdapter,
	IN	UINT32			Address,
	IN	UINT32			Value,
	IN	BOOLEAN			mapping);

NTSTATUS	Cmd_SDIO_Read_Reg(
	IN	RTMP_ADAPTER	*pAdapter,
	IN	UINT32			Address,
	IN	UINT32			*Value,
	IN	BOOLEAN			mapping);
#if 0
NTSTATUS	Cmd_SDIO_Nic_Capability(
	IN	RTMP_ADAPTER	*pAdapter);
//	IN	UINT32			Address,
//	IN	UINT32			*Value);
#endif
NTSTATUS	Cmd_SDIO_Read_Efuse(
	IN	RTMP_ADAPTER	*pAdapter,
	IN	UINT32			Address,
	IN	UINT32			*Value,
	IN	BOOLEAN			mapping);

NTSTATUS	Cmd_SDIO_Write_Efuse(
	IN	RTMP_ADAPTER	*pAdapter,
	IN	UINT32			Address,
	IN	UINT32			Value);
#endif

NTSTATUS	FWCmdEventHandler(
	IN	RTMP_ADAPTER	*pAdapter,
	IN	PVOID			pBuffer,
	IN	UINT32			Length);

NTSTATUS	FWInitCmdResultEvent(
	IN	RTMP_ADAPTER	*pAdapter,
	IN	PVOID			pBuffer,
	IN	UINT32			Length);

NTSTATUS	FWExtCmdEvent(
	IN	RTMP_ADAPTER	*pAdapter,
	IN	PVOID			pBuffer,
	IN	UINT32			Length);


//
// Load Andes ROM PATCH
// MT7636 FPGA
//
NTSTATUS NICLoadRomPatch(
	IN RTMP_ADAPTER *pAdapter);

NTSTATUS LoadROMPatchProcess(
	IN RTMP_ADAPTER *pAdapter,
	IN PUCHAR	pFileName,
	UINT32		FileLength
	);

NTSTATUS NICLoadFirmware(
	IN RTMP_ADAPTER *pAdapter);

NTSTATUS LoadFwImageProcess(
	IN RTMP_ADAPTER *pAdapter,
	IN PUCHAR	pFileName,
	IN UINT32	FileLength,
	IN UCHAR	Type
	);

NTSTATUS ROMPatchPowerOnWiFiSys(IN RTMP_ADAPTER *pAdapter);
 
NTSTATUS	USBROMPatchCalChecksum(
	IN	RTMP_ADAPTER	*pAdapter,
	IN	PVOID			pData,
	IN	USHORT			Length);
NTSTATUS	USBROMPatchGetChecksum(
	IN	RTMP_ADAPTER	*pAdapter,
	IN	PVOID			pData,
	IN	USHORT			Length);

NTSTATUS	FWTxCmd(
	IN	RTMP_ADAPTER	*pAdapter,
	IN	PVOID			pCmd,
	IN	UINT32			Length,
	IN	KEVENT			*pEvent);

NTSTATUS FirmwareScatters(
	IN  RTMP_ADAPTER *pAdapter,
	IN  PUCHAR          pFirmwareImage,
	IN  ULONG           ImageLength
	);

//
// MT7636 firmware command start to load patch
//
NTSTATUS FirmwareCommnadStartToLoadROMPatch(
	IN	RTMP_ADAPTER	*pAdapter,
	IN	UINT32			Address,
	IN	UINT32			Length
	);

NTSTATUS FirmwareCommnadLoadROMPatchFinish(
	IN	RTMP_ADAPTER	*pAdapter
	);
//
// Rom patch Get Semaphore
//
NTSTATUS FirmwareCommnadGetPatchSemaphore(
	IN	RTMP_ADAPTER	*pAdapter);

NTSTATUS FirmwareCommnadStartToLoadFW(
	IN  RTMP_ADAPTER   *pAdapter,
	IN  ULONG           ImageLength,
	IN  ULONG		Address,
	IN  ULONG		DataMode
	);

NTSTATUS FirmwareCommnadRestartDownloadFW(
    	IN  RTMP_ADAPTER   *pAdapter
    	);

NTSTATUS FirmwareCommnadStartToRunFW(
	IN  RTMP_ADAPTER *pAdapter
	);

NTSTATUS FirmwareCommnadHIFLoopBackTest(
	IN  RTMP_ADAPTER *pAdapter, BOOLEAN IsEnable, UINT8 RxQ
	);

NTSTATUS FWSwitchToROM(
	IN  RTMP_ADAPTER *pAd
	);

NTSTATUS CheckFWROMWiFiSysOn(IN  RTMP_ADAPTER   *pAd);

INT32 CmdETxBfSoundingPeriodicTriggerCtrl(RTMP_ADAPTER *pAd, UINT32 wlanidx, UINT8 On, UINT32 BW);
#endif //_FW_CMD_H
