/*
 Module Name:
 	nic.h
 
Abstract:
	NIC related define and macro.

Revision History:
	Who		When		What
	-------	----------	----------------------------------------------
	Kun Wu	02-11-2014	Created

*/



#ifndef	__NIC_H__
#define	__NIC_H__

#define IS_MT7603(_pAdapter)				((_pAdapter->ChipID & 0x0000FFFF) == 0x7603)
#define IS_MT7636(_pAdapter)				((_pAdapter->ChipID & 0x0000FFFF) == 0x7636 ||(_pAdapter->MACVersion & 0x0000FFFF) == 0x7606)

/*	MT76x6 series: 7636/7606/7611
 	1. For definition/compiler option, use MT7636 for all 7636/7603/7611
 	2. For run time case (specific H/W), please use IS_MT7636(), IS_MT7606(), MT7611()
 	3. For run time case (all H/Ws), please use IS_MT76x6()
 	4. For different interface, please use IS_MT7636U(), IS_MT7636S() */
#define IS_MT76x6(_pAd)     ((((_pAd)->ChipID & 0x0000ffff) == 0x00007606)||\
							(((_pAd)->ChipID & 0x0000ffff) == 0x00007636) || \
							(((_pAd)->ChipID & 0x0000ffff) == 0x00007611))

//#define IS_MT7636U(_pAd)	((((_pAd)->ChipID & 0x0000ffff) == 0x00007606) && IS_USB_INF(_pAd))
//#define IS_MT7636S(_pAd)	((((_pAd)->ChipID & 0x0000ffff) == 0x00007606) && IS_SDIO_INF(_pAd))

//#define IS_MT7636(_pAd)		(((_pAd)->ChipID & 0x0000ffff) == 0x00007636)
#define IS_MT7606(_pAd)		(((_pAd)->ChipID & 0x0000ffff) == 0x00007606)
#define IS_MT7611(_pAd)		(((_pAd)->ChipID & 0x0000ffff) == 0x00007611)

//
// Scheduler Register 4 (offset: 0x0594, default: 0x0000_0000) 
// Note: 
//  1. DW : double word
//
typedef union _SCHEDULER_REGISTER4
{
	struct	{
	    // DW0
	    ULONG   ForceQid:4;                                 // [3 : 0]      When force_mode = 1, queue id will adopt this index.
	    ULONG   ForceMode:1;                            // [4]      Force enable dma_scheduler work in force queue without calculate TXOP time.
	    ULONG   BypassMode:1;                           // [5]      Bypass_mode:1, for PSE loopback only.
	    ULONG   HybridMode:1;                           // [6]      Hybrid_mode:1, when assert this reg, DMA scheduler would ignore the tx op time information from LMAC, and also use FFA and RSV for enqueue cal.
	    ULONG   RgPredictNoMask:1;                      // [7]      When disable high and queue mask, force in predict mode.
	    ULONG   RgResetScheduler:1;                         // [8]      DMA Scheduler soft reset. When assert this reset, dma scheduler state machine will be in INIT state and reset all queue counter.
	    ULONG   RgDoneClearHeader:1;                    // [9]      Switch header counter clear flag. 0 : Clear by tx_eof   1 : Clear by sch_txdone
	    ULONG   SwMode:1;                               // [10]     When SwMode is 1 : DMA scheduler will ignore rate mapping & txop time threshold from LMAC, and use RgRateMap & txtime_thd_* value.
	    ULONG   Reserves0:5;                            // [15 : 11]    Reserved bits.
	    ULONG   RgRateMap:14;                           // [29 : 16]    When SwMode 1 : DMA scheduler will load rate mapping information from this register, else rate mapping information from LMAC. Rate Mapping table : {mode X [2 : 0], rate [5 : 0], frmode [1 : 0], nss [1:  0], sg_flag}
	    ULONG   Reserves1:2;                            // [31 : 30]    Reserved bits.
	}	Default;
	ULONG			word;
} SCHEDULER_REGISTER4, *PSCHEDULER_REGISTER4;

//
// TOP MISC CONTROLS2 (offset: 0x0134, default: 0x0000_0000) 
//
// 
//
// Note: 
//  1. DW : double word
//
typedef union _TOP_MISC_CONTROLS2
{
	struct	{
		// DW0
	    ULONG   ReadyToLoadFW:1;                        // [0]      Reserved bit.
	    ULONG   FwIsRunning:1;                          // [1]      Check if FW need to reload.
	    ULONG   Reserves:22;                            // [23 : 0] Reserved bits.
	    ULONG   TOP_MISC_DBG_SEL:4;                     // [27 : 24]    
	    ULONG   DBG_SEL:4;                              // [31 : 28]    	}	Default;
	}	Default;
	ULONG			word;
} TOP_MISC_CONTROLS2, *PTOP_MISC_CONTROLS2;

typedef struct _OTHER_STATISTICS {
	UINT32					TxRepeatCount;
	UINT32					TransmittedCount;
	UINT32					Rx0ReceivedCount;
	UINT32					ReceivedCount;//++werner //loopback receive count
	UINT32					Ac0TxedCount;
	UINT32					Ac1TxedCount;
	UINT32					Ac2TxedCount;
	UINT32					Ac3TxedCount;
	UINT32					Ac4TxedCount;
	UINT32					Ac5TxedCount;
	UINT32					Ac6TxedCount;
	UINT32					Ac7TxedCount;
	UINT32					Ac8TxedCount;
	UINT32					Ac9TxedCount;
	UINT32					Ac10TxedCount;
	UINT32					Ac11TxedCount;	
	UINT32					MgmtTxedCount;
	UINT32					HccaTxedCount;
	INT32					RSSI0;
	INT32					RSSI1;
	INT32					RSSI2;
	INT32					SNR0;
	INT32					SNR1;
	INT32					SNR2;
	UINT32					UdpErr;
	UINT32					TcpErr;
	UINT32					IpErr;
	INT32					RCPI0;
	INT32					RCPI1;
	INT32					FreqOffstFromRX;
	INT32					TXPower;
	INT32					TSSI;
} OTHER_STATISTICS, *POTHER_STATISTICS;

typedef struct _COUNTER_802_11 {
	LARGE_INTEGER   CRCErrorCount;
	LARGE_INTEGER   RXOverFlowCount;
	LARGE_INTEGER   PHYErrorCount;
	LARGE_INTEGER   FalseCCACount;
	LARGE_INTEGER   U2MDataCount;
	LARGE_INTEGER   OtherDataCount;
	LARGE_INTEGER   BeaconCount;
	LARGE_INTEGER   othersCount;	
	LARGE_INTEGER   ErrorIdx;
} COUNTER_802_11, *PCOUNTER_802_11;


//bf
typedef struct _PFMU_PROFILE{
         UCHAR  ng, cw, nrow, ncol, LM;
         UCHAR  FIX_PSI, LD, EO, IO, IE;
         UCHAR  DMAC[6];
         UCHAR  Pn_STS1, Pn_STS0;
         UCHAR  SNR_STS0, SNR_STS1, SNR_STS2, SNR_STS3;
         UCHAR  MCS_ub, MCS_lb;
         UCHAR  Arrival_order;
         USHORT timeout;
         UCHAR  validFlg;
         UCHAR  CMDInIdx;
} PFMU_PROFILE, *pPFMU_PROFILE;

typedef struct _PFMU_DATA{
//DW0
	UINT32  phi11:9;
	UINT32  psi21:7;
	UINT32  phi21:9;
	UINT32  psi31:7;
//DW1
	UINT32  phi31:9;
	UINT32  psi41:7;
	UINT32  phi22:9;
	UINT32  psi32:7;
//DW2	
	UINT32  phi32:9;
	UINT32  psi42:7;
	UINT32  phi33:9;
	UINT32  psi43:7;
//DW3
	UINT32  Snr0:4;
	UINT32  Snr1:4;
	UINT32  Snr2:4;
	UINT32  Snr3:4;	
// DW4	
	UINT32  SubcarrierIdx;
} PFMU_DATA;

//Loopback
typedef enum{
	RUNNING = 0,
	PASS,
	FAIL
}LOOPBACK_STATUS;

typedef enum{
	NO_ERROR = 0,
	TX_TIMEOUT,
	RX_TIMEOUT,
	BIT_TRUE_FAIL
}LOOPBACK_FAIL;

typedef struct _LOOPBACK_SETTING{
	UINT32 StartLen;
	UINT32 StopLen;
	UINT32 RepeatTimes; // 0 = infinite
	UINT32 IsDefaultPattern;
//#ifdef RTMP_USB_SUPPORT	
	UINT32 BulkOutNumber;
	UINT32 BulkInNumber;
	UINT32 TxAggNumber;
	UINT32 RxAggPktLmt;// pkt numbers
	UINT32 RxAggLmt;// pkt size
	UINT32 RxAggTO;// timeout (us)
	UINT32 RxAggEnable;
//#endif			
} LOOPBACK_SETTING, *pLOOPBACK_SETTING;

typedef struct _LOOPBACK_RESULT{
	UINT32 Status; //0 running, 1 pass , 2 fail
	UINT32 FailReason; //0 no error, 1 TX timeout, 2 RX timeout, 3 bit true fail
	UINT32 TxPktCount;
	UINT32 RxPktCount;
	UINT32 TxByteCount; // 0 = infinite
	UINT32 RxByteCount;
#ifdef RTMP_USB_SUPPORT		
	UINT32 LastBulkOut;
#endif
} LOOPBACK_RESULT, *pLOOPBACK_RESULT;

typedef struct _RSSI_DATA{
	UINT32 Numbers; //Number of streams
	INT32 RSSI0;
	UINT32 RSSI0var;//variation
	INT32 RSSI1;
	UINT32 RSSI1var;//variation
	INT32 RSSI2;// 3x3
	UINT32 RSSI2var;//variation
	INT32 RSSI3;// 4x4
	UINT32 RSSI3var;//variation
} RSSI_DATA, *pRSSI_DATA;

//~Loopback

#ifdef RTMP_USB_SUPPORT
#define SCATTER_SIZE 14336
#endif
#ifdef RTMP_SDIO_SUPPORT
#define SCATTER_SIZE 4096
#endif
#define BUFFER_SIZE 1024*64
#define LOOPBACK_SIZE 1024*16

#define MAX_TX_BUFEER_NUM		6
#define TX_DATA_BUFEER_NUM	0
#define TX_CMD_BUFEER_NUM		1

#define BSSID_WCID		1	/* in infra mode, always put bssid with this WCID */

#define BACKUPCRSIZE		36

#define RXV_RSSI_LIMIT		1024
#endif //__NIC_H__
