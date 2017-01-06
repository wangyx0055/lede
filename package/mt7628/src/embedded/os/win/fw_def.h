/*
 Module Name:
 	fw_def.h
 
Abstract:
	FW related define and macro and structure.

Revision History:
	Who		When		What
	-------	----------	----------------------------------------------
	Kun Wu	03-25-2014	Created

*/



#ifndef	__FW_DEF_H__
#define	__FW_DEF_H__

#define SEQ_FW_EVENT					0x0//not fw response
#define SEQ_CMD_FW_SCATTERS			0x90
#define SEQ_CMD_FW_STARTREQ			0x91
#define SEQ_CMD_FW_STARTTORUN		0x92
#define SEQ_CMD_FW_RESTART			0x93
#define SEQ_CMD_LOOPBACK_TEST			0x9F
#define SEQ_CMD_ROM_PATCH_SEMAPHORE	0xA0
#define SEQ_CMD_ROM_PATCH_STARTREQ	0xA1
#define SEQ_CMD_ROM_PATCH_FINISH		0xA2
#define SEQ_CMD_ROM_ACCESS_REG		0xA3
#define SEQ_CMD_RAM_ACCESS_REG_WRITE	0xA4
#define SEQ_CMD_RAM_ACCESS_REG_READ		0xA5
#define SEQ_CMD_RAM_ACCESS_EFUSE_WRITE	0xA6
#define SEQ_CMD_RAM_ACCESS_EFUSE_READ	0xA7
#define SEQ_CMD_RAM_TXBF_SOUNDING		0xA8

/*
 * Extension Command
 */

typedef enum {
 EXT_CMD_ID_EFUSE_ACCESS  =0x01,
 EXT_CMD_RF_REG_ACCESS = 0x02,
 EXT_CMD_RF_TEST = 0x04,
 EXT_CMD_RADIO_ON_OFF_CTRL = 0x05,
 EXT_CMD_WIFI_RX_DISABLE = 0x06,
 EXT_CMD_PM_STATE_CTRL = 0x07,
 EXT_CMD_CHANNEL_SWITCH = 0x08,
 EXT_CMD_NIC_CAPABILITY = 0x09,
 EXT_CMD_PWR_SAVING = 0x0A,
 EXT_CMD_MULTIPLE_REG_ACCESS = 0x0E,
 EXT_CMD_AP_PWR_SAVING_CAPABILITY = 0xF,
 EXT_CMD_SEC_ADDREMOVE_KEY = 0x10,
 EXT_CMD_SET_TX_POWER_CTRL=0x11,
 EXT_CMD_FW_LOG_2_HOST = 0x13,
 EXT_CMD_PS_RETRIEVE_START = 0x14,
 EXT_CMD_BT_COEX = 0x19,
 EXT_CMD_BF_SOUNDING_START=0x1E,
 EXT_CMD_BF_SOUNDING_STOP=0x1F,
 EXT_CMD_EFUSE_BUFFER_MODE = 0x21,
 EXT_CMD_THERMAL_PROTECT = 0x23,
}EXT_CMD_TYPE;

//only in MT7636
typedef enum{
	ILM = 0,
	DLM
}RAM_Type;

//for UI loading FW
typedef enum{
	MT7603_RAM = 0,
	MT7636_RAM,
	MT7636_ROM_PATCH
}FW_Type;

//
// FW TxD
//
// 
//
// Note: 
//  1. DW : double word
//
//ROM
typedef struct _FIRMWARE_TXDSCR
{
    // DW0
    ULONG   Length:16;                              // [15 : 0] Equal to TxByteCount of TxD.
    ULONG   PQ_ID:16;                               // [31 : 16]    For firmware download CMD, this field should be stuff to 0x8000; for image download pkt, this field should be stuff to 0xd000.

    // DW1
    ULONG   CID:8;                                  // [7 : 0]      The CMD ID defined in firmware download protocol.
    ULONG   PktTypeID:8;                            // [15 : 8] Set to 0xA0 (FT = 0x1, HF=0x1, other fields are ignored).
    ULONG   SetQuery:8;                             // [23 : 16]    not use so far.
    ULONG   SeqNum:8;                               // [31 : 24]    The sequence number of the pkt.

    // DW2
    ULONG   Reserve;                                // [31 : 0] TBD (sync to the normal TxD format).
} FIRMWARE_TXDSCR, *PFIRMWARE_TXDSCR;

//RAM
typedef struct _FW_RAM_TXDSCR
{
    // DW0
    ULONG   Length:16;                              // [15 : 0] Equal to TxByteCount of TxD.
    ULONG   PQ_ID:16;                               // [31 : 16]    For firmware download CMD, this field should be stuff to 0x8000; for image download pkt, this field should be stuff to 0xd000.

    // DW1
    ULONG   CID:8;                                  // [7 : 0]      The CMD ID defined in firmware download protocol.
    ULONG   PktTypeID:8;                            // [15 : 8] Set to 0xA0 (FT = 0x1, HF=0x1, other fields are ignored).
    ULONG   SetQuery:8;                             // [23 : 16]    not use so far.
    ULONG   SeqNum:8;                               // [31 : 24]    The sequence number of the pkt.

    // DW2
    ULONG   D2B0Rev:8;                              
    ULONG   ExtenCID:8;                             
    ULONG   D2B2Rev:8;                    
    ULONG   ExtCmdOption:8;           
    // DW3 ~ DW7
    ULONG   D3toD7Rev[5];               
} FW_RAM_TXDSCR, *PFW_RAM_TXDSCR;
//
// Command to start loading FW
//
// 
typedef struct _CMD_START_TO_LOAD_FW_REQUEST
{
    FIRMWARE_TXDSCR FwTxD;
    ULONG               Address;        // the start address of firmware code to be copied in SRAM in Address memory.
    ULONG               Length;         // the total length of the firmware image size in bytes.
    ULONG               DataMode;       // the option field for this CMD packet.
} CMD_START_TO_LOAD_FW_REQUEST, *PCMD_START_TO_LOAD_FW_REQUEST;

//
// Command to start running FW
//
// 
typedef struct _CMD_START_FW_REQUEST
{
    FIRMWARE_TXDSCR FwTxD;
    ULONG               Override;       // B0 : 1 --> indicate to adopt value of field u4Address to be the entry point for RAM code starting up.
                                        // B0 : 0 -->use default address as entry point for RAM code starting up.
                                        // Other bits are useless
    ULONG               Address;        // the address to the entry point for RAM to start up, it is used when B0 of u4Override field is equal to 1.
} CMD_START_FW_REQUEST, *PCMD_START_FW_REQUEST;

//
// Command to Get Rom Patch Semaphore
//
// 
typedef struct _CMD_GET_PATCH_SEMAPHORE
{
	FIRMWARE_TXDSCR FwTxD;
	ULONG	ucGetSemaphore:8;
	ULONG	Reserve:24;
	
} CMD_GET_PATCH_SEMAPHORE, *PCMD_GET_PATCH_SEMAPHORE;

//
// Command to Get Rom Patch Semaphore
//
// 
typedef struct _ROM_CMD_ACCESS_REG
{
	FIRMWARE_TXDSCR FwTxD;
	ULONG	SetQuery:8;
	ULONG	Reserve:24;
	ULONG	Address;
	ULONG	Data;
	
} ROM_CMD_ACCESS_REG, *PROM_CMD_ACCESS_REG;

//
// 
//
// 
typedef struct _RAM_CMD_ACCESS_REG
{
	FW_RAM_TXDSCR FwTxD;
	UINT32	Type;
	UINT32	Address;
	UINT32	Data;
	
} RAM_CMD_ACCESS_REG, *PRAM_CMD_ACCESS_REG;

//
// Command to start running FW
//
// 
typedef struct _CMD_ID_HIF_LOOPBACK_TEST
{
	FIRMWARE_TXDSCR 	FwTxD;
	UINT32				Loopback_Enable:16;
	UINT32				DestinationQid:16;
} CMD_ID_HIF_LOOPBACK_TEST, *PCMD_ID_HIF_LOOPBACK_TEST;


#define FW_INFO_LENGTH_7603 32

//
// The information of 7603 Firmware bin file
//
typedef struct _FW_BIN_INFO_STRUC_7603
{
	// DW0
	ULONG       MagicNumber;
	UCHAR       Version[12];
	UCHAR       Date[16];
	ULONG       ImageLength;
}  FW_BIN_INFO_STRUC_7603, *PFW_BIN_INFO_STRUC_7603;


#define FW_INFO_LENGTH_7636 36
//
// FW image feature set
//
typedef struct _FIRMWARE_FEATURE_SET
{
	UCHAR	Encryption:1; 							// [0]		0: plain image is used 1: encrypted image is used	
	UCHAR	KEY_INDEX:2; 							// [2 : 1]		key index of the cncryption
	UCHAR	Reserve:5;
} FIRMWARE_FEATURE_SET, *PFIRMWARE_FEATURE_SET;

#if 0
typedef union _FW_BIN_INFO_STRUC
{
	//
	// The information of 7603 Firmware bin file
	//
	struct	{
	    // DW0
	    ULONG       MagicNumber;
	    UCHAR       Version[12];
	    UCHAR       Date[16];
	    ULONG       ImageLength;
		ULONG	   Reserved:9;
	}	7603;
	//
	// The information of 7636 Firmware bin file
	//
	struct {
		ULONG		Address;
		FIRMWARE_FEATURE_SET		FeatureSet;
		UCHAR		Version[11];
		UCHAR		Date[16];
		ULONG		ImageLength;
	}	7636;
}  FW_BIN_INFO_STRUC, *PFW_BIN_INFO_STRUC;
#endif

//
// The information of 7636 Firmware bin file
//
typedef struct _FW_BIN_INFO_STRUC_7636
{
	ULONG		Address;
	UCHAR		ChipInfo;
	FIRMWARE_FEATURE_SET		FeatureSet;//FIRMWARE_FEATURE_SET		FeatureSet;	
	UCHAR		Version[10];
	UCHAR		Date[16];
	ULONG		ImageLength;
}  FW_BIN_INFO_STRUC_7636, *PFW_BIN_INFO_STRUC_7636;

#define	ROM_PATCH_ADDRESS	0x0008C000

//
// The header format of Patch bin file
//
typedef	struct	_ROM_PATCH_BIN_HEADER_STRUC	{
		ULONGLONG	   	TimeStampLowPart;   // ASCII code
		ULONGLONG       	TimeStampHighPart;  // ASCII code	
		ULONG       		Platform;
		ULONG       		HwSwVersion;
		ULONG       		PatchVersion;
		USHORT	  		Checksum;		
}	ROM_PATCH_BIN_HEADER_STRUC, *PROM_PATCH_BIN_HEADER_STRUC;

#define ROM_PATCH_INFO_LENGTH		30

//
// checksum caculation request data
//
typedef struct _ROMPATCH_CAL_CHECKSUM_DATA
{
	ULONG	Address;
	ULONG	Length;
} ROMPATCH_CAL_CHECKSUM_DATA, *PROMPATCH_CAL_CHECKSUM_DATA;


typedef struct _EXT_CMD_ETXBf_SD_PERIODIC_TRIGGER_CTRL_T {
	FW_RAM_TXDSCR FwTxD;
	UINT8 ucWlanIdx;
	UINT8 ucOMacIdx;
	UINT8 ucWMMIdx;
	UINT8 ucFixRateMode;           /* TRUE: Fix Rate by ucBW and u2RateCode */
	UINT8 ucBW;
	UINT16 u2NDPARateCode;
	UINT16 u2NDPRateCode;
	UINT32 u4SoundingInterval;     /* By ms */
} EXT_CMD_ETXBf_SD_PERIODIC_TRIGGER_CTRL_T;
#endif //__FW_DEF_H__