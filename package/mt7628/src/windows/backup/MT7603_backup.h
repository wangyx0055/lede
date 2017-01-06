/*
 ***************************************************************************
 * 5F, No.5, Tai-Yuen 1st St., Jhubei City, HsinChu Hsien 30265, Taiwan, R.O.C
 *
 * (c) Copyright 2005, Ralink Technology, Inc.
 *
 * All rights reserved. Ralink's source code is an unpublished work and the
 * use of a copyright notice does not imply otherwise. This source code
 * contains confidential trade secret material of Ralink Tech. Any attemp
 * or participation in deciphering, decoding, reverse engineering or in any
 * way altering the source code is stricitly prohibited, unless the prior
 * written consent of Ralink Technology, Inc. is obtained.
 ***************************************************************************

    Module Name:
    MT7603_backup.h

    Abstract:
    MT7603 usb ASIC related definition & structures

    Revision History:
    Who         When          What
    --------    ----------    ----------------------------------------------
    Lens Wang   05-15-2013    created

*/

// TODO: Shiang-usw-win, integrate this header file as unify one!!!

#ifndef __MT7603_BACKUP_H__
#define __MT7603_BACKUP_H__

typedef struct _EXT_CMD_CHAN_SWITCH_T;

#define MT_TX_RETRY_LIMITED		0x0f
#define MT_TX_RETRY_CNT			MT_TX_RETRY_LIMITED

#define PRINT_MAC(addr) \
    addr[0], addr[1], addr[2], addr[3], addr[4], addr[5]

#define inline __inline    

#define MAC_IO_READ32(_A, _R, _pV)			HW_IO_READ32(_A, _R, _pV)
#define MAC_IO_WRITE32(_A, _R, _V)			HW_IO_WRITE32(_A, _R, _V)

#define HIF_IO_READ32(_A, _R, _pV)			HW_IO_READ32(_A, _R, _pV)
#define HIF_IO_WRITE32(_A, _R, _V)			HW_IO_WRITE32(_A, _R, _V)

#define PHY_IO_READ32(_A, _R, _pV)			HW_IO_READ32(_A, _R, _pV)
#define PHY_IO_WRITE32(_A, _R, _V)			HW_IO_WRITE32(_A, _R, _V)

#define RTMP_SET_FLAG(_M, _F)       MT_SET_FLAG(_M, _F)
#define RTMP_CLEAR_FLAG(_M, _F)    MT_CLEAR_FLAG(_M, _F)
#define RTMP_CLEAR_FLAGS(_M)        MT_CLEAR_FLAGS(_M)
#define RTMP_TEST_FLAG(_M, _F)      MT_TEST_FLAG(_M, _F)
#define RTMP_TEST_FLAGS(_M, _F)     MT_TEST_FLAGS(_M, _F)


#define WMM_PARAM_TXOP	0
#define WMM_PARAM_AIFSN	1
#define WMM_PARAM_CWMIN	2
#define WMM_PARAM_CWMAX	3
#define WMM_PARAM_ALL		4

#define WMM_PARAM_AC_0		0
#define WMM_PARAM_AC_1		1
#define WMM_PARAM_AC_2		2
#define WMM_PARAM_AC_3		3


typedef enum _BA_SESSION_TYPE
{
    BA_SESSION_INV = 0,
    BA_SESSION_ORI = 1,
    BA_SESSION_RECP= 2,
} BA_SESSION_TYPE;


typedef struct rtmp_mac_ctrl
{
#ifdef MT_MAC
    UINT8 wtbl_entry_cnt[4];
    UINT8 wtbl_entry_size[4];
    UINT32 wtbl_base_addr[4]; // base address for WTBL2/3/4
    UINT32 wtbl_base_fid[4];
    UINT32 page_size;
#endif /* MT_MAC */
} RTMP_MAC_CTRL;


struct MT_TX_PWR_CAP {
    UINT8 pa_type;
    UINT8 tssi_stage;
    UINT8 tssi_0_slope_g_band;
    UINT8 tssi_1_slope_g_band;
    UINT8 tssi_0_offset_g_band;
    UINT8 tssi_1_offset_g_band;
    CHAR tx_0_target_pwr_g_band;
    CHAR tx_1_target_pwr_g_band;
    CHAR tx_0_chl_pwr_delta_g_band[3];
    CHAR tx_1_chl_pwr_delta_g_band[3];
    CHAR delta_tx_pwr_bw40_g_band;

    CHAR tx_pwr_cck_1_2;
    CHAR tx_pwr_cck_5_11;
    CHAR tx_pwr_g_band_ofdm_6_9;
    CHAR tx_pwr_g_band_ofdm_12_18;
    CHAR tx_pwr_g_band_ofdm_24_36;
    CHAR tx_pwr_g_band_ofdm_48_54;
    CHAR tx_pwr_ht_bpsk_mcs_0_8;
    CHAR tx_pwr_ht_qpsk_mcs_1_2_9_10;
    CHAR tx_pwr_ht_16qam_mcs_3_4_11_12;
    CHAR tx_pwr_ht_64qam_mcs_5_13;
    CHAR tx_pwr_ht_64qam_mcs_6_14;
    CHAR tx_pwr_ht_64qam_mcs_7_15;
};

#define INTERNAL_PA 0
#define EXTERNAL_PA 1
#define TSSI_TRIGGER_STAGE 0
#define TSSI_COMP_STAGE 1
#define TSSI_CAL_STAGE 2
#define TSSI_0_SLOPE_G_BAND_DEFAULT_VALUE 0x84
#define TSSI_1_SLOPE_G_BAND_DEFAULT_VALUE 0x83
#define TSSI_0_OFFSET_G_BAND_DEFAULT_VALUE 0x0A
#define TSSI_1_OFFSET_G_BAND_DEFAULT_VALUE 0x0B
#define TX_TARGET_PWR_DEFAULT_VALUE 0x26
#define G_BAND_LOW 0
#define G_BAND_MID 1
#define G_BAND_HI 2
#define A_BAND_LOW 0
#define A_BAND_HI 1

//#define TMAC_FP0R0 (0x21600 + 0x20)
#define LG_OFDM0_FRAME_POWER0_DBM_MASK (0x7f)
#define LG_OFDM0_FRAME_POWER0_DBM(p) (((p) & 0x7f))
#define LG_OFDM1_FRAME_POWER0_DBM_MASK (0x7f << 8)
#define LG_OFDM1_FRAME_POWER0_DBM(p) (((p) & 0x7f) << 8)
#define LG_OFDM2_FRAME_POWER0_DBM_MASK (0x7f << 16)
#define LG_OFDM2_FRAME_POWER0_DBM(p) (((p) & 0x7f) << 16)
#define LG_OFDM3_FRAME_POWER0_DBM_MASK (0x7f << 24)
#define LG_OFDM3_FRAME_POWER0_DBM(p) (((p) & 0x7f) << 24)

//#define TMAC_FP0R1 (0x21600 + 0x24)
#define HT20_0_FRAME_POWER0_DBM_MASK (0x7f)
#define HT20_0_FRAME_POWER0_DBM(p) (((p) & 0x7f))
#define HT20_1_FRAME_POWER0_DBM_MASK (0x7f << 8)
#define HT20_1_FRAME_POWER0_DBM(p) (((p) & 0x7f) << 8)
#define HT20_2_FRAME_POWER0_DBM_MASK (0x7f << 16)
#define HT20_2_FRAME_POWER0_DBM(p) (((p) & 0x7f) << 16)
#define HT20_3_FRAME_POWER0_DBM_MASK (0x7f << 24)
#define HT20_3_FRAME_POWER0_DBM(p) (((p) & 0x7f) << 24)

//#define TMAC_FP0R2 (0x21600 + 0x28)
#define HT40_0_FRAME_POWER0_DBM_MASK (0x7f)
#define HT40_0_FRAME_POWER0_DBM(p) (((p) & 0x7f))
#define HT40_1_FRAME_POWER0_DBM_MASK (0x7f << 8)
#define HT40_1_FRAME_POWER0_DBM(p) (((p) & 0x7f) << 8)
#define HT40_2_FRAME_POWER0_DBM_MASK (0x7f << 16)
#define HT40_2_FRAME_POWER0_DBM(p) (((p) & 0x7f) << 16)
#define HT40_3_FRAME_POWER0_DBM_MASK (0x7f << 24)
#define HT40_3_FRAME_POWER0_DBM(p) (((p) & 0x7f) << 24)

//#define TMAC_FP0R3 (0x21600 + 0x2C)
#define CCK0_FRAME_POWER0_DBM_MASK (0x7f)
#define CCK0_FRAME_POWER0_DBM(p) (((p) & 0x7f))
#define LG_OFDM4_FRAME_POWER0_DBM_MASK (0x7f << 8)
#define LG_OFDM4_FRAME_POWER0_DBM(p) (((p) & 0x7f) << 8)
#define CCK1_FRAME_POWER0_DBM_MASK (0x7f << 16)
#define CCK1_FRAME_POWER0_DBM(p) (((p) & 0x7f) << 16)
#define HT40_6_FRAME_POWER0_DBM_MASK (0x7f << 24)
#define HT40_6_FRAME_POWER0_DBM(p) (((p) & 0x7f) << 24)

//#define TMAC_FP0R4 (0x21600 + 0x30)
#define HT20_4_FRAME_POWER0_DBM_MASK (0x7f)
#define HT20_4_FRAME_POWER0_DBM(p) (((p) & 0x7f))
#define HT20_5_FRAME_POWER0_DBM_MASK (0x7f << 8)
#define HT20_5_FRAME_POWER0_DBM(p) (((p) & 0x7f) << 8)
#define HT40_4_FRAME_POWER0_DBM_MASK (0x7f << 16)
#define HT40_4_FRAME_POWER0_DBM(p) (((p) & 0x7f) << 16)
#define HT40_5_FRAME_POWER0_DBM_MASK (0x7f << 24)
#define HT40_5_FRAME_POWER0_DBM(p) (((p) & 0x7f) << 24)


typedef struct _RTMP_CHIP_OP {
	/* MCU related callback functions */
	int (*load_rom_patch)(struct _RTMP_ADAPTER *ad);
	int (*erase_rom_patch)(struct _RTMP_ADAPTER *ad);
	int (*loadFirmware)(struct _RTMP_ADAPTER *pAd);
	int (*eraseFirmware)(struct _RTMP_ADAPTER *pAd);
	int (*sendCommandToMcu)(struct _RTMP_ADAPTER *pAd, UCHAR cmd, UCHAR token, UCHAR arg0, UCHAR arg1, BOOLEAN FlgIsNeedLocked);
	int (*sendCommandToAndesMcu)(struct _RTMP_ADAPTER *pAd, UCHAR QueIdx, UCHAR cmd, UCHAR *pData, USHORT DataLen, BOOLEAN FlgIsNeedLocked);

        VOID (*AsicResetBbpAgent)(IN struct _RTMP_ADAPTER *pAd);

	/* Channel */
	VOID (*ChipSwitchChannel)(struct _RTMP_ADAPTER *pAd, UCHAR ch, BOOLEAN bScan);

        VOID (*AsicMacInit)(struct _RTMP_ADAPTER *pAd);

}RTMP_CHIP_OP;


enum ASIC_CAP{
	fASIC_CAP_RX_SC = 0x1,
	fASIC_CAP_CSO = 0x2,
	fASIC_CAP_TSO = 0x4,
	fASIC_CAP_MCS_LUT = 0x8,

	fASIC_CAP_PMF_ENC = 0x10,
};

#define fRTMP_ADAPTER_DISABLE_DOT_11N	0x00000001
#define fRTMP_ADAPTER_WSC_PBC_PIN0		0x00000002
#define fRTMP_ADAPTER_DISABLE_DEQUEUE	0x00000004

enum PHY_CAP{
	fPHY_CAP_24G = 0x1,
	fPHY_CAP_5G = 0x2,

	fPHY_CAP_HT = 0x10,
	fPHY_CAP_VHT = 0x20,

	fPHY_CAP_TXBF = 0x100,
	fPHY_CAP_LDPC = 0x200,
};

enum HIF_TYPE{
	HIF_RTMP = 0x0,
	HIF_RLT = 0x1,
	HIF_MT = 0x2,
	HIF_MAX = HIF_MT,
};

enum MAC_TYPE{
	MAC_RTMP = 0x0,
	MAC_MT = 0x1,
};

enum RF_TYPE {
	RF_RT,
	RF_RLT,
	RF_MT76x2,
	RF_MT,
};

enum BBP_TYPE{
	BBP_RTMP = 0x0,
	BBP_RLT = 0x1,
	BBP_MT = 0x2,
};


/*
	2860: 28xx
	2870: 28xx

	30xx:
		3090
		3070
		2070 3070

	33xx:	30xx
		3390 3090
		3370 3070

	35xx:	30xx
		3572, 2870, 28xx
		3062, 2860, 28xx
		3562, 2860, 28xx

	3593, 28xx, 30xx, 35xx

	< Note: 3050, 3052, 3350 can not be compiled simultaneously. >
	305x:
		3052
		3050
		3350, 3050

	3352: 305x

	2880: 28xx
	2883:
	3883:
*/

typedef struct _RTMP_CHIP_CAP {
	/* ------------------------ packet --------------------- */
	UINT8 TXWISize;	// TxWI or LMAC TxD max size
	UINT8 RXWISize; // RxWI or LMAC RxD max size
	UINT8 tx_hw_hdr_len;	// Tx Hw meta info size which including all hw info fields
	UINT8 rx_hw_hdr_len;	// Rx Hw meta info size

	enum ASIC_CAP asic_caps;
	enum PHY_CAP phy_caps;
	enum HIF_TYPE hif_type;
	enum MAC_TYPE mac_type;
	enum BBP_TYPE bbp_type;
	enum MCU_TYPE MCUType;
	enum RF_TYPE rf_type;

	/* beacon */
	BOOLEAN FlgIsSupSpecBcnBuf;	/* SPECIFIC_BCN_BUF_SUPPORT */
	UINT8 BcnMaxNum;	/* software use */
	UINT8 BcnMaxHwNum;	/* hardware limitation */
	UINT8 WcidHwRsvNum;	/* hardware available WCID number */
	UINT32 WtblHwNum;		/* hardware WTBL number */
	UINT32 WtblPseAddr;		/* */
	UINT16 BcnMaxHwSize;	/* hardware maximum beacon size */
	UINT16 BcnBase[HW_BEACON_MAX_NUM];	/* hardware beacon base address */

	/* function */
	/* use UINT8, not bit-or to speed up driver */
	BOOLEAN FlgIsHwWapiSup;

	/* VCO calibration mode */
	UINT8 VcoPeriod; /* default 10s */
#define VCO_CAL_DISABLE		0	/* not support */
#define VCO_CAL_MODE_1		1	/* toggle RF7[0] */
#define VCO_CAL_MODE_2		2	/* toggle RF3[7] */
#define VCO_CAL_MODE_3		3	/* toggle RF4[7] or RF5[7] */
	UINT8	FlgIsVcoReCalMode;

	BOOLEAN FlgIsHwAntennaDiversitySup;
	BOOLEAN Flg7662ChipCap;
#ifdef TXBF_SUPPORT
	BOOLEAN FlgHwTxBfCap;
	BOOLEAN FlgITxBfBinWrite;
#endif /* TXBF_SUPPORT */
#ifdef FIFO_EXT_SUPPORT
	BOOLEAN FlgHwFifoExtCap;
#endif /* FIFO_EXT_SUPPORT */

	UCHAR ba_max_cnt;

#ifdef RTMP_MAC_PCI
#ifdef CONFIG_STA_SUPPORT
	BOOLEAN HW_PCIE_PS_SUPPORT;
	BOOLEAN HW_PCIE_PS_L3_ENABLE;
#endif /* CONFIG_STA_SUPPORT */
#endif /* RTMP_MAC_PCI */

	BOOLEAN temp_tx_alc_enable;
	INT32 temp_25_ref; /* a quantification value of temperature, but not ¢J */
	INT32 current_temp; /* unit ¢J */
#ifdef RTMP_TEMPERATURE_TX_ALC
	UINT32 high_temp_slope_a_band; /* slope with unit (¢J /dB) */
	UINT32 low_temp_slope_a_band; /* slope with unit (¢J /dB) */
	UINT32 high_temp_slope_g_band; /* slope with unit (¢J /dB) */
	UINT32 low_temp_slope_g_band; /* slope with unit (¢J /dB) */
	INT32 tc_upper_bound_a_band; /* unit dB */
	INT32 tc_lower_bound_a_band; /* unit dB */
	INT32 tc_upper_bound_g_band; /* unit dB */
	INT32 tc_lower_bound_g_band; /* unit dB */	
#endif /* RTMP_TEMPERATURE_TX_ALC */

#ifdef TXRX_SW_ANTDIV_SUPPORT
	BOOLEAN bTxRxSwAntDiv;
#endif /* TXRX_SW_ANTDIV_SUPPORT */

#ifdef DYNAMIC_VGA_SUPPORT
	BOOLEAN dynamic_vga_support;
	INT32 compensate_level;
	INT32 avg_rssi_0;
	INT32 avg_rssi_1;
	INT32 avg_rssi_all;
	UCHAR dynamic_chE_mode;
	BOOLEAN dynamic_chE_trigger;
#endif /* DYNAMIC_VGA_SUPPORT */

	/* ---------------------------- signal ---------------------------------- */
#define SNR_FORMULA1		0	/* ((0xeb     - pAd->StaCfg.wdev.LastSNR0) * 3) / 16; */
#define SNR_FORMULA2		1	/* (pAd->StaCfg.wdev.LastSNR0 * 3 + 8) >> 4; */
#define SNR_FORMULA3		2	/* (pAd->StaCfg.wdev.LastSNR0) * 3) / 16; */
#define SNR_FORMULA4		3	/* for MT7603 */
	UINT8 SnrFormula;

	UINT8 max_nss;			/* maximum Nss, 3 for 3883 or 3593 */
	UINT8 max_vht_mcs;		/* Maximum Vht MCS */

#ifdef DOT11_VHT_AC
	UINT8 ac_off_mode;		/* 11AC off mode */
#endif /* DOT11_VHT_AC */

	BOOLEAN bTempCompTxALC;
	BOOLEAN rx_temp_comp;

	BOOLEAN bLimitPowerRange; /* TSSI compensation range limit */

#if defined(RTMP_INTERNAL_TX_ALC) || defined(SINGLE_SKU_V2)
	INT16	PAModeCCK[4];
	INT16	PAModeOFDM[8];
	INT16	PAModeHT[16];
#ifdef DOT11_VHT_AC
	INT16	PAModeVHT[10];
#endif /* DOT11_VHT_AC */
#endif /* defined(RTMP_INTERNAL_TX_ALC) || defined(SINGLE_SKU_V2) */

#ifdef MT7601
	CHAR	TemperatureRef25C;
	UCHAR	TemperatureMode;
	BOOLEAN	bPllLockProtect;
	CHAR	CurrentTemperBbpR49;
	INT32	TemperatureDPD;					// temperature when do DPD calibration
	INT32	CurrentTemperature;					// (BBP_R49 - Ref25C) * offset
#endif /* MT7601 */

	/* ---------------------------- others ---------------------------------- */
#ifdef RTMP_EFUSE_SUPPORT
	UINT16 EFUSE_USAGE_MAP_START;
	UINT16 EFUSE_USAGE_MAP_END;
	UINT8 EFUSE_USAGE_MAP_SIZE;
	UINT8 EFUSE_RESERVED_SIZE;
#endif /* RTMP_EFUSE_SUPPORT */

	UCHAR *EEPROM_DEFAULT_BIN;
	UINT16 EEPROM_DEFAULT_BIN_SIZE;

#ifdef RTMP_FLASH_SUPPORT
	BOOLEAN ee_inited;
#endif /* RTMP_FLASH_SUPPORT */
#ifdef CARRIER_DETECTION_SUPPORT
	UCHAR carrier_func;
#endif /* CARRIER_DETECTION_SUPPORT */
#ifdef DFS_SUPPORT
	UINT8 DfsEngineNum;
#endif /* DFS_SUPPORT */

	/*
		Define the burst size of WPDMA of PCI
		0 : 4 DWORD (16bytes)
		1 : 8 DWORD (32 bytes)
		2 : 16 DWORD (64 bytes)
		3 : 32 DWORD (128 bytes)
	*/
	UINT8 WPDMABurstSIZE;

	/*
 	 * 0: MBSSID_MODE0
 	 * (The multiple MAC_ADDR/BSSID are distinguished by [bit2:bit0] of byte5)
 	 * 1: MBSSID_MODE1
 	 * (The multiple MAC_ADDR/BSSID are distinguished by [bit4:bit2] of byte0)
 	 */
	UINT8 MBSSIDMode;

#ifdef DOT11W_PMF_SUPPORT
#define PMF_ENCRYPT_MODE_0      0	/* All packets must software encryption. */
#define PMF_ENCRYPT_MODE_1      1	/* Data packets do hardware encryption, management packet do software encryption. */
#define PMF_ENCRYPT_MODE_2      2	/* Data and management packets do hardware encryption. */
	UINT8	FlgPMFEncrtptMode;
#endif /* DOT11W_PMF_SUPPORT */

#ifdef CONFIG_ANDES_SUPPORT
	UINT32 WlanMemmapOffset;
	UINT32 InbandPacketMaxLen; /* must be 48 multible */
	UINT8 CmdRspRxRing;
	BOOLEAN IsComboChip;
	BOOLEAN need_load_fw;
	BOOLEAN need_load_rom_patch;
	UINT8 ram_code_protect;
	UINT8 rom_code_protect;
	UINT8 load_iv;
	UINT32 ilm_offset;
	UINT32 dlm_offset;
	UINT32 rom_patch_offset;
	UINT8 DownLoadType;
#endif

	UINT8 cmd_header_len;
	UINT8 cmd_padding_len;

#ifdef SINGLE_SKU_V2
	CHAR	Apwrdelta;
	CHAR	Gpwrdelta;
#endif /* SINGLE_SKU_V2 */

#ifdef RTMP_USB_SUPPORT
	UINT8 DataBulkInAddr;
	UINT8 CommandRspBulkInAddr;
	UINT8 WMM0ACBulkOutAddr[4];
	UINT8 WMM1ACBulkOutAddr;
	UINT8 CommandBulkOutAddr;
#endif

#ifdef CONFIG_SWITCH_CHANNEL_OFFLOAD
	UINT16 ChannelParamsSize;
	UCHAR *ChannelParam;
	INT XtalType;
#endif

	UCHAR load_code_method;
	UCHAR *FWImageName;
	UCHAR *fw_header_image;
	UCHAR *fw_bin_file_name;
	UCHAR *rom_patch;
	UCHAR *rom_patch_header_image;
	UCHAR *rom_patch_bin_file_name;
	UINT32 rom_patch_len;
	UINT32 fw_len;
	UCHAR *MACRegisterVer;
	UCHAR *BBPRegisterVer;
	UCHAR *RFRegisterVer;

	BOOLEAN tssi_enable;
	BOOLEAN ed_cca_enable;
	
#ifdef MT_MAC
	struct MT_TX_PWR_CAP MTTxPwrCap;
	UCHAR TmrEnable;
#endif


	UINT8 TxBAWinSize;
	UINT8 RxBAWinSize;
	UINT8 AMPDUFactor;
#ifdef RACTRL_FW_OFFLOAD_SUPPORT
    BOOLEAN fgRateAdaptFWOffload;
#endif /* RACTRL_FW_OFFLOAD_SUPPORT */

    UINT32  CurrentTxOP;
}RTMP_CHIP_CAP;


typedef union _U3DMA_WLCFG
{
    struct
    {
    // DW0
        UINT32  WL_RX_AGG_TO:8;                         // [7 : 0]      WLAN UDMA RX Aggregation Time-Out Value. Unit:1us.
        UINT32  WL_RX_AGG_LMT:8;                        // [15 : 8] WLAN UDMA RX Aggregation Limit.
        UINT32  Reserve1:2;                                 // [17 : 16]    Reserved bits.
        UINT32  WL_RX_MPSZ_PAD0:1;                  // [18]     Padding 1DW if the length of IN data is with maxima packet size.
        UINT32  WL_TX_CLEAR:1;                          // [19]     WLAN UDMA UDMA TX Clear
        UINT32  WL_LPK_EN:1;                            // [20]     WLAN U3DMA loopback mode enable
        UINT32  WL_RX_AGG_EN:1;                     // [21]     WLAN UDMA RX Aggregation Function Enable
        UINT32  WL_RX_EN:1;                         // [22]     WLAN UDMA RX Enable
        UINT32  WL_TX_EN:1;                         // [23]     WLAN UDMA TX Enable
        UINT32  Reserve2:6;                             // [29 : 24]    Reserved bits
        UINT32  WL_RX_BUSY:1;                           // [30]     Rx Busy Status. 0 : idls   1:busy
        UINT32  WL_TX_BUSY:1;                           // [31]     Tx Busy Status. 0 : idls   1:busy
    } field;
    UINT32 word;
} U3DMA_WLCFG, *PU3DMA_WLCFG;



////////////////////////////////////
////////////////////////////////////
/// CMD packet header ///////////////
////////////////////////////////////
////////////////////////////////////

//
//CMD TxD
//
// 
//
// Note: 
//  1. DW : double word
//
typedef struct _CMD_TXDSCR
{
    // DW0
    ULONG   Length:16;                              // [15 : 0] Equal to TxByteCount of TxD.
    ULONG   PQ_ID:16;                               // [31 : 16]    For firmware download CMD, this field should be stuff to 0x8000; for image download pkt, this field should be stuff to 0xd000.

    // DW1
    ULONG   CID:8;                                  // [7 : 0]      The CMD ID defined in firmware download protocol.
    ULONG   PktTypeID:8;                            // [15 : 8] Set to 0xA0 (FT = 0x1, HF=0x1, other fields are ignored).
    ULONG   SetQuery:8;                             // [23 : 16]    not use so far.
    ULONG   SeqNum:8;                               // [31 : 24]    The sequence number of the pkt.

    UCHAR   D2B0Rev;                                // Padding fields, hw may auto modify this field.
    UCHAR   ExtendCID;                              // Extend CID for MT7603
    UCHAR   D2B2Rev;                                // Padding fields, hw may auto modify this field.
    UCHAR   ucExtCmdOption;
    ULONG   D3ToD7Rev[5];                           // Padding fields, hw will auto modify somebytes of this fields.
    UCHAR   Buffer[0];
} CMD_TXDSCR, *PCMD_TXDSCR;


//
// CMD RxD
//
// 
//
// Note: 
//  1. DW : double word
//
typedef struct _CMD_RXDSCR
{
    // DW0
    ULONG   RxByteCount:16;                             // [15 : 0] Equal to TxByteCount of TxD.
    ULONG   PacketType:16;                          // [31 : 16]    For firmware download CMD, this field should be stuff to 0x8000; for image download pkt, this field should be stuff to 0xd000.

    // DW1
    ULONG   EID:8;                                  // [7 : 0]      The CMD ID defined in firmware download protocol.
    ULONG   SeqNum:8;                               // [31 : 24]    The sequence number of the pkt.
    ULONG   Reserved0:8;                            // [15 : 8] Set to 0xA0 (FT = 0x1, HF=0x1, other fields are ignored).
    ULONG   Reserved1:8;                            // [23 : 16]    not use so far.

    UCHAR   ExtendEID;
    UCHAR   Reserved[3];
    UCHAR   Buffer[0];
} CMD_RXDSCR, *PCMD_RXDSCR;

// CMD_CSUM_OFFLOAD
typedef struct _CMD_CSUM_OFFLOAD
{
    USHORT  RxCheckSum;
    USHORT  TxCheckSum;
} CMD_CSUM_OFFLOAD, *PCMD_CSUM_OFFLOAD;

typedef struct _EVENT_EXT_CMD_RESULT_T_1
{
    UINT8   ExtendID;
    UINT8   Reserve[3];
    UINT32  Status;
} EVENT_EXT_CMD_RESULT_T_1, *PEVENT_EXT_CMD_RESULT_T_1;

// CMD_BASIC_CONFIG
typedef struct _CMD_BASIC_CONFIG
{
    UCHAR   Native80211;
    UCHAR   Reserved[3];

    CMD_CSUM_OFFLOAD    CheckSumOffload;
} CMD_BASIC_CONFIG, *PCMD_BASIC_CONFIG;

// CMD_BSS_ACTIVATE_CTRL
typedef struct _CMD_BSS_ACTIVATE_CTRL
{
    UCHAR   BssIndex;
    UCHAR   Active;
    UCHAR   NetworkType;
    UCHAR   OwnMacAddrIndex;
    UCHAR   BssMacAddr[6];
    UCHAR   Reserved[2];
} CMD_BSS_ACTIVATE_CTRL, *PCMD_BSS_ACTIVATE_CTRL;

// EXT_CMD_CHAN_SWITCH_T
#define SKU_SIZE 21
typedef struct _EXT_CMD_CHAN_SWITCH_T_1
{
    UINT8       CtrlCH;
    UINT8       CentralCH;
    UINT8       BW;
    UINT8       TxStreamNum;
    UINT8       RxStreamNum;
    UINT8       Reserve0[7];
    UINT8       TxPowerSKU[SKU_SIZE];
    UINT8       Reserve1[3];
} EXT_CMD_CHAN_SWITCH_T_1, *PEXT_CMD_CHAN_SWITCH_T_1;

//
// FW RxD
//
// 
//
// Note: 
//  1. DW : double word
//
typedef struct _FIRMWARE_RXDSCR
{
    // DW0
    ULONG   RxByteCount:16;                             // [15 : 0] Equal to TxByteCount of TxD.
    ULONG   PacketType:16;                          // [31 : 16]    For firmware download CMD, this field should be stuff to 0x8000; for image download pkt, this field should be stuff to 0xd000.

    // DW1
    ULONG   EID:8;                                  // [7 : 0]      The CMD ID defined in firmware download protocol.
    ULONG   SeqNum:8;                               // [15 : 8] The sequence number of the pkt.
    ULONG   Reserved0:8;                            // [23 : 16]    Set to 0xA0 (FT = 0x1, HF=0x1, other fields are ignored).
    ULONG   Reserved1:8;                            // [31 : 24]    not use so far.

    // DW2
    //ULONG Status:8;                               // [31 : 0] TBD (sync to the normal TxD format).
    //ULONG Reserve2:24;                            // [31 : 0] TBD (sync to the normal TxD format).
} FIRMWARE_RXDSCR, *PFIRMWARE_RXDSCR;

//
// Command Event from FW
//
// 
typedef struct _CMD_EVENT_FROM_FW
{
    FIRMWARE_RXDSCR FwRxD;
    ULONG               Status:8;       // 0 for Success; others for failure.
    ULONG               Reserve:24; // .
} CMD_EVENT_FROM_FW, *PCMD_EVENT_FROM_FW;

//
// Command Event from FW
//
// 
typedef struct _CMD_EVENT_START_FW_RESPONSE
{
    FIRMWARE_RXDSCR FwRxD;
    ULONG               Status;     // 0 for Success; others for failure.
    ULONG               Reserve;    // .
} CMD_EVENT_START_FW_RESPONSE, *PCMD_EVENT_START_FW_RESPONSE;

typedef struct _EXT_CMD_ID_LED
{
        UINT32  u4LedNo;
        UINT32  u4LedCtrl;
} EXT_CMD_ID_LED_T, *PEXT_CMD_ID_LED_T;

typedef struct _CMD_RF_TEST_CTRL_T
{
    UINT8   Action;
    UINT8   Reserved[3];
    union
    {
        UINT32  OpMode;
        UINT32  ChannelFreq;
        PARAM_MTK_WIFI_TEST_STRUC_T RfATInfo;
    } u;
} CMD_RF_TEST_CTRL_T, *PCMD_RF_TEST_CTRL_T;

//
// STOP_DROP_EPOUT  (offset: 0x080, default: 0x0000_0000) 
//
// 
//
// Note: 
//  1. DW : double word
//
typedef struct _UDMA_STOP_DROP_EPOUT
{
    // DW0
    ULONG   Reserve1:4;                                 // [3 : 0]      Reserved bits.
    ULONG   STOP_EP4OUT:1;                      // [4]      Stop EP4OUT packets from USB FIFO.
    ULONG   STOP_EP5OUT:1;                      // [5]      Stop EP5OUT packets from USB FIFO.
    ULONG   STOP_EP6OUT:1;                      // [6]      Stop EP6OUT packets from USB FIFO.
    ULONG   STOP_EP7OUT:1;                      // [7]      Stop EP7OUT packets from USB FIFO.
    ULONG   STOP_EP8OUT:1;                      // [8]      Stop EP8OUT packets from USB FIFO.
    ULONG   STOP_EP9OUT:1;                      // [9]      Stop EP9OUT packets from USB FIFO.
    ULONG   Reserve2:10;                            // [19 : 10]    Reserved bits.
    ULONG   DROP_EP4OUT:1;                      // [20]     Drop EP4OUT packets from USB FIFO.
    ULONG   DROP_EP5OUT:1;                      // [21]     Drop EP5OUT packets from USB FIFO.
    ULONG   DROP_EP6OUT:1;                      // [22]     Drop EP6OUT packets from USB FIFO.
    ULONG   DROP_EP7OUT:1;                      // [23]     Drop EP7OUT packets from USB FIFO.
    ULONG   DROP_EP8OUT:1;                      // [24]     Drop EP8OUT packets from USB FIFO.
    ULONG   DROP_EP9OUT:1;                      // [25]     Drop EP9OUT packets from USB FIFO.
    ULONG   Reserve3:6;                                 // [31 : 26]    Reserved bits.
} UDMA_STOP_DROP_EPOUT, *PUDMA_STOP_DROP_EPOUT;
//
// Scheduler Register 4 (offset: 0x0594, default: 0x0000_0000) 
//
// 
//
// Note: 
//  1. DW : double word
//
typedef union _SCHEDULER_REGISTER4
{
    struct
    {
        // DW0
        UINT32  ForceQid:4;                                 // [3 : 0]      When force_mode = 1, queue id will adopt this index.
        UINT32  ForceMode:1;                            // [4]      Force enable dma_scheduler work in force queue without calculate TXOP time.
        UINT32  BypassMode:1;                           // [5]      Bypass_mode:1, for PSE loopback only.
        UINT32  HybridMode:1;                           // [6]      Hybrid_mode:1, when assert this reg, DMA scheduler would ignore the tx op time information from LMAC, and also use FFA and RSV for enqueue cal.
        UINT32  RgPredictNoMask:1;                      // [7]      When disable high and queue mask, force in predict mode.
        UINT32  RgResetScheduler:1;                         // [8]      DMA Scheduler soft reset. When assert this reset, dma scheduler state machine will be in INIT state and reset all queue counter.
        UINT32  RgDoneClearHeader:1;                    // [9]      Switch header counter clear flag. 0 : Clear by tx_eof   1 : Clear by sch_txdone
        UINT32  SwMode:1;                               // [10]     When SwMode is 1 : DMA scheduler will ignore rate mapping & txop time threshold from LMAC, and use RgRateMap & txtime_thd_* value.
        UINT32  Reserves0:5;                            // [15 : 11]    Reserved bits.
        UINT32  RgRateMap:14;                           // [29 : 16]    When SwMode 1 : DMA scheduler will load rate mapping information from this register, else rate mapping information from LMAC. Rate Mapping table : {mode X [2 : 0], rate [5 : 0], frmode [1 : 0], nss [1:  0], sg_flag}
        UINT32  Reserves1:2;                            // [31 : 30]    Reserved bits.
    } field;
    UINT32 word;
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
    struct
    {
        // DW0
        UINT32  ReadyToLoadFW:1;                        // [0]      Reserved bit.
        UINT32  FwIsRunning:1;                          // [1]      Check if FW need to reload.
        UINT32  Reserves:22;                            // [23 : 0] Reserved bits.
        UINT32  TOP_MISC_DBG_SEL:4;                     // [27 : 24]    
        UINT32  DBG_SEL:4;                              // [31 : 28]    
    } field;
    UINT32 word;
} TOP_MISC_CONTROLS2, *PTOP_MISC_CONTROLS2;

//
// FW TxD
//
// 
//
// Note: 
//  1. DW : double word
//
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


#define FW_INFO_LENGTH 32

//
// The information of Firmware bin file
//
typedef struct  _FW_BIN_INFO_STRUC
{
    ULONG       MagicNumber;
    UCHAR       Version[12];
    UCHAR       Date[16];
    ULONG       ImageLength;
}   FW_BIN_INFO_STRUC, *PFW_BIN_INFO_STRUC;

//
// RX Classify Filter Register 0
//
// Note: 
//  1.
//
typedef struct _RX_CLASSIFY_FILTER_REGISTER0
{
    ULONG   DAT_N_NULL2M:1;                         // [0]      Set (Q)Null Data frame to MCU.
    ULONG   DAT_NULL2M:1;                               // [1]      Set Non-(Q)Null Data frame to MCU.
    ULONG   MGT2M:1;                                    // [2]      Set Management frame to MCU.
    ULONG   CTRL_N_BAR2M:1;                         // [3]      Set Non-BAR control frame to MCU.
    ULONG   CTRL_BAR2M:1;                               // [4]      Set BAR control frame to MCU.
    ULONG   FRAG2M:1;                                   // [5]      Set fragmented packet to MCU.
    ULONG   CLSF2M:10;                                  // [15 : 6] Set the interested CLS BITMAP and forward to MCU.
    ULONG   WOL2M:1;                                    // [16]     Set WOL(Wake-on-WLAN) packet to MCU.
    ULONG   OFLD2M:3;                                   // [19 : 17]    Set TDLS/ARP/NS/EAPOL frame to MCU.
    ULONG   MGC2M:1;                                    // [20]     Set Magic packet to MCU.
    ULONG   BP2M:1;                                     // [21]     Set Bypass packet (sniffer mode) to MCU.
    ULONG   EL2M:1;                                     // [22]     Set EL "Exceed Maximum Rx Length" packet to MCU.
    ULONG   Reserve0:1;                                 // [23]     Reserve bits.
    ULONG   UCDP2MH:2;                                  // [25 : 24]    Send the PF dropped unicast packet to MCU/HIF.
    ULONG   BMCDP2MH:2;                             // [27 : 26]    Send the PF dropped broadcast/multicast packet to MCU/HIF.
    ULONG   Reserve1:1;                                 // [28]     Reserve bits.
    ULONG   RX2H_QID:1;                                 // [29]     The RX HIF destination queue index.
    ULONG   RX2M_QID:2;                                 // [31 : 30]    The RX MCU destination queue index.

} RX_CLASSIFY_FILTER_REGISTER0, *PRX_CLASSIFY_FILTER_REGISTER0;

//
// RX Classify Filter Register 1
//
// Note: 
//  1.
//
typedef struct _RX_CLASSIFY_FILTER_REGISTER1
{
    ULONG   Reserve:24;                                 // [23 : 0] Reserve bits.
    ULONG   BSSID0DuplicatedRFB:2;                      // [25 : 24]    BSSID0_DUP_RFB : RFB duplication mode for BSSID0 & BSSID0_x (InfraAP).
    ULONG   BSSID1DuplicatedRFB:2;                      // [27 : 26]    BSSID1_DUP_RFB : RFB duplication mode for BSSID1.
    ULONG   BSSID2DuplicatedRFB:2;                      // [29 : 28]    BSSID2_DUP_RFB : RFB duplication mode for BSSID2.
    ULONG   BSSID3DuplicatedRFB:2;                      // [31 : 30]    BSSID3_DUP_RFB : RFB duplication mode for BSSID3.

} RX_CLASSIFY_FILTER_REGISTER1, *PRX_CLASSIFY_FILTER_REGISTER1;

//
// RX Classify Filter Register 2
//
// Note: 
//  1.
//
typedef struct _RX_CLASSIFY_FILTER_REGISTER2
{
    ULONG   HTCDuplicatedRFB;                           // [31 : 0] HTC_DUP_RFB : If teh received frame will pass to Host and the result of this field "AND" received frame's HTC field is NOT zero, duplicate RFB and pass to FW. If the received frame will pass to FW, ignore this field.

} RX_CLASSIFY_FILTER_REGISTER2, *PRX_CLASSIFY_FILTER_REGISTER2;

//
// RX Vector Frame
//
// Note: 
//  1.
//
typedef struct _RX_VECTORE_FRAME
{
    // DW 0
    ULONG   TA0;                                        // [31 : 0] TA [31 : 0] 

    // DW 1
    ULONG   TA1:16;                                     // [15 : 0] TA [47 : 32]
    ULONG   RXVectorSequenceNumber:8;                   // [23 : 16]    RXV_SN : RX Vector Sequence Number
    ULONG   RxVectorOrTxStatusVector:1;                 // [24]     TR : indicated the RX vector is for TX status or RX frame.
    ULONG   ReserveDW1:7;                               // [31 : 25]    Reserve bits

    // DW2
    ULONG   RxVector1_1;                                // [31 : 0] RX Vector 1 1st cycle

    // DW3
    ULONG   RxVector1_2;                                // [31 : 0] RX Vector 2 2nd cycle

    // DW4
    ULONG   RxVector1_3;                                // [31 : 0] RX Vector 3 3rd cycle

    // DW5
    ULONG   RxVector1_4;                                // [31 : 0] RX Vector 4 4th cycle

    // DW6
    ULONG   RxVector1_5;                                // [31 : 0] RX Vector 5 5th cycle

    // DW7
    ULONG   RxVector1_6;                                // [31 : 0] RX Vector 6 6th cycle

    // DW8
    ULONG   RxVector2DW0;                               // [31 : 0] RX Vector 2 DW0

    // DW9
    ULONG   RxVector2DW1;                               // [31 : 0] RX Vector 2 DW1

    // DW10
    ULONG   RxVector2DW2;                               // [31 : 0] RX Vector 3 DW2

} RX_VECTORE_FRAME, *PRX_VECTORE_FRAME;

//
// RX Vector in RXV-Q
//
// Note: 
//  1.
//
typedef struct _RX_VECTORE_IN_RX_VECTOR_Q
{
    ULONG   RxByteCount:16;                             // [15 : 0] Rx Byte Count. The total byte length of the VEC frame.
    ULONG   RXVectorCounter:5;                          // [20 : 16]    RXV_CNT : the valid RX Vector count in the frame
    ULONG   Reserve:8;                                  // [28 : 21]    Reserve bits
    ULONG   PacketType:3;                               // [31 : 29]    PKT_TYPE : indicated the current packet's type.

} RX_VECTORE_IN_RX_VECTOR_Q, *PRX_VECTORE_IN_RX_VECTOR_Q;

//
// RX Vector Classified Filter
//
// Note: 
//  1.
//
typedef struct _RX_VECTORE_CLASSIFY_FILTER_REGISTER0
{
    ULONG   RXV2M:1;                                    // [0]      Set RX vector to MCU
    ULONG   Reserve0:7;                                 // [7 : 1]      Reserve bits
    ULONG   RXV_CNT:5;                                  // [12 : 8] Set maximum aggregation count
    ULONG   RXV2H_QIDX:1;                               // [13]     The RXV HIF destination queue index
    ULONG   RXV2M_QIDX:2;                               // [15 : 14]    The RXV MCU destination queue index
    ULONG   RXV_TOUT:12;                                // [27 : 16]    RXV Timeout value (in unit of 32us)
    ULONG   Reserve1:4;                                 // [31 : 28]    Reserve bits

} RX_VECTORE_CLASSIFY_FILTER_REGISTER0, *PRX_VECTORE_CLASSIFY_FILTER_REGISTER0;

typedef struct _MAC_TX_INFO{
	UCHAR WCID;
	BOOLEAN FRAG;
	BOOLEAN InsTimestamp;
	BOOLEAN NSeq;
	BOOLEAN Ack;
	BOOLEAN BM;
	BOOLEAN CFACK;
	BOOLEAN AMPDU;
	UCHAR BASize;
	UCHAR PID;
	UCHAR TID;
	UCHAR TxRate;
	UCHAR Txopmode;
	ULONG Length;
	UCHAR hdr_len;
	UCHAR hdr_pad;
	UCHAR eth_type_offset;
	UCHAR bss_idx;
	UCHAR q_idx;
	UCHAR prot;
	UCHAR AntPri;
	UCHAR SpeEn;
	UCHAR Preamble;
#ifdef MT_MAC
	UCHAR Type;
	UCHAR SubType;
	UCHAR PsmBySw; /* PSM bit controlled by SW */
#endif /* MT_MAC */
}MAC_TX_INFO;


// ======================================================================================================
// Tx Structures
// ======================================================================================================

//
// TX Description (TXDscr) Main Part
//
// LMAC -> PSE
//
// Note: 
//  1. DW : double word
//
#define TXD_SHORT_SIZE  (3*4)
#define TXD_LONG_SIZE       (8*4)
#define TXD_SHORT_FORMAT    0
#define TXD_LONG_FORMAT 1
#define TXD_LMAC_PORT       0
#define TXD_MCU_PORT        1

typedef struct _TXDSCR_DW0
{
    // DW0
    ULONG   TxByteCount:16;                             // [15 : 0] Tx Byte Count (in unit of byte)
    ULONG   EtherTypeOffset:7;                      // [22 : 16]    Ether-Type Offset (in unit of word)
    ULONG   IPChecksumOffload:1;                    // [23]     I : Request to do IP checksum offload(LOW active)
    ULONG   UDPTCPChecksumOffload:1;                // [24]     UT : UDP/TCP checksum offload (LOW active)
    ULONG   USBNextValid:1;                         // [25]     UNxV : USB next valid. (only valid when USB interface is selected)
    ULONG   USBTxBurst:1;                           // [26]     UtxB : USB tx burst. (only valid when USB interface is selected)
    ULONG   DestinationQueueID:4;                   // [30 : 27]    Q_IDX : Indicate the destination queue ID.
    ULONG   DestinationPortID:1;                        // [31]     P_IDX : Indicate the destination port ID. --> 0 : LMAC port ; 1 : MCU port
} TXDSCR_DW0, *PTXDSCR_DW0;

#define TMI_HDR_PAD_BIT_MODE        2
#define TMI_HDR_PAD_MODE_TAIL       0

typedef struct _TXDSCR_DW1
{
    // DW1
    ULONG   WLANIndex:8;                            // [7 : 0]      WLAN index; WCID
    ULONG   HeaderLength:5;                         // [12 : 8] Header Length
    ULONG   HeaderFormat:2;                         // [14 : 13]    HF : Header Format. --> 00 : non-802.11 frame (ethernet/802.3) ; 01 : CMD frame ; 10 : 802.11 frame (normal mode) ; 11 : 802.11 frame (enhancement mode)
    ULONG   TxDescriptionFormat:1;                  // [15]     FT : Tx Description Format. 0 : short format. Host only provides TX.Dscr DW0 ~ DW1. 1 : Long format. Host provides TX.Dscr DW0 ~ DW6.
    ULONG   HeaderPadding:3;                        // [18 : 16]    Indicate the header padding byte count. Bit[2]:0  pad the dummy bytes in the tail of header. Bit[2] 1  pad the dummy bytes in the head of header. Bit[1 : 0] padding byte length
    ULONG   NoAck:1;                                // [19]     NA : No Ack. 0 : the packet needs ack. 1 : the packet does not need ack.
    ULONG   TID:3;                                  // [22 : 20]    TID : TID of QoS data frame, otherwise it should be set to 0. HW can aggregate packets with the same TID when there is a valid RA/TID entry in the BA table.
    ULONG   ProtectedFrameField:1;                  // [23]     PF : protected frame field. Indicating this is a packet need to be encrypted. 0: no need. 1 : need
    ULONG   Reserve0:2;                                 // [25 : 24]    Reserve bits.
    ULONG   OwnMACAddress:6;                        // [31 : 26]    OM : Own MAC Address. This field indicates the Own MAC address of current packet.
} TXDSCR_DW1, *PTXDSCR_DW1;

typedef struct _TXDSCR_DW2
{
    // DW2
    ULONG   Type:2;                                     // [1 : 0]      Type : Frame Type Field. (HF = 1'b1, HW reserved bit) Indicating Management/Control/Data frame of a packet. This field is copied from Frame Control Field of WLAN header at Tx Header Address.
    ULONG   Subtype:4;                              // [5 : 2]      Subtype : Frame Subtype Field. (HF = 1'b1, HW reserved bit) Copied from Frame Control Field of WLAN header at Tx Header Address
    ULONG   NDP:1;                                  // [6]      Indicate the frame is NDP.
    ULONG   NDPA:1;                                 // [7]      Indicate the frame is NDP announcement frame (11n) or NDPA (11ac).
    ULONG   SoundingPPDU:1;                         // [8]      SD : Sounding PPDU. when asserted, this frame is a sounding PPDU, it will not be aggregated with other frame.
    ULONG   RTS:1;                                  // [9]      RTS : used for BT over WiFi application.FW can set this bit to 1 and must ignore all other protection conditions while transmitting BT over WiFi packet.
    ULONG   BMPacket:1;                                 // [10]     BM : Broadcast/Multicast packet. FW would parse the A1 of WLAN header and set this bit if A1 is a BMCAST address. When this bit is set, HW MAC would default treat this packet as No-Ack packet. This bit is request by HW. HW MAC needs this information instead of get the A1 from SRAM.
    ULONG   BIPProtected:1;                             // [11]     B : This field indicates this packet should be BIP protected.
    ULONG   Duration:1;                                 // [12]     DU : this bit is used to control the Duration field content maintained by HW MAC or FW.
    ULONG   HTCValid:1;                                 // [13]     HE : HTC Exist. Indicating this is a packet already contains HTC field. (HW (in RDG mode) will not append HTC field if this bit is set)
    ULONG   Fragment:2;                             // [15 : 14]    FRAG : indicate the fragment packet position. 00 : no fragment packet, 01 : first fragment packet, 10 : middle fragment packet, 11 : last fragment packet
    ULONG   MaxTxTime:8;                            // [23 : 16]    Remaining Life Time / Max TX Time : This field indicates the remaining life time in unit of 8TU used for this packet.
    ULONG   PowerOffset:5;                          // [28 : 24]    Power Offset : Per-packet Tx Power control.
    ULONG   BADisable:1;                            // [29]     BA_DIS : indicate that the packet can NOT be aggregated to AMPDU. 0 : HW reference WTBL.BA setting. 1 : HW ignores the WTBL.BA setting and will NOT aggregate this packet to AMPDU
    ULONG   TimingMeasurement:1;                    // [30]     TM : Timing Measurement. 0 : This packet is not a timing measurement trigger packet. 1 : This packet is a timing measurement trigger packet.   The ToD & ToA will be reported by report packet. 
    ULONG   FixedRate:1;                            // [31]     FR : Fixed Rate. 0 : use auto rate for current TX. 1 : use fixed rate for current TX.
} TXDSCR_DW2, *PTXDSCR_DW2;

typedef struct _TXDSCR_DW3
{
    // DW3
    ULONG   Reserve1:6;                                 // [5 : 0]      Reserve bits.
    ULONG   TxCount:5;                              // [10 : 6] Tx Count. 5'h0 : the current packet has NOT been transmitted. 5'h1~1E : the exactly transmitted count. 5'h1F the transmitted count is equal to or greater than 31.  Software must write 0 in the field.
    ULONG   RemainingTxCount:5;                     // [15 : 11]    Remaining Tx Count. 5'h0 : mean cannot Tx. 5'h1~5'h1E : Tx count. 5'h1F : Tx count unlimited.
    ULONG   SequnceNumber:12;                       // [27 : 16]    SN : Sequence Number.
    ULONG   ReservedForHWUse:2;                     // [29 : 28]    Reserved for HW use.
    ULONG   PNValid:1;                              // [30]     PN_VLD : PN in TX descriptor is valid.
    ULONG   SNValid:1;                              // [31]     SN_VLD : SN in TX descriptor is valid.
} TXDSCR_DW3, *PTXDSCR_DW3;

typedef struct _TXDSCR_DW4
{
    // DW4
    ULONG   PacketNumberLow;                        // [31 : 0] Packet Number [31 : 0].  PacketNumber[47 : 0] = PacketNumber0  +PacketNumber1
} TXDSCR_DW4, *PTXDSCR_DW4;


/* TXDSCR_DW5.DataSourceSelection */
#define TMI_DAS_FROM_MPDU       0
#define TMI_DAS_FROM_WTBL       1

/* TXDSCR_DW5.BARSSNContorl */
#define TMI_BSN_CFG_BY_HW   0
#define TMI_BSN_CFG_BY_SW   1

/* TXDSCR_DW5.PowerManagement */
#define TMI_PM_BIT_CFG_BY_HW    0
#define TMI_PM_BIT_CFG_BY_SW    1
typedef struct _TXDSCR_DW5
{
    // DW5
    ULONG   PacketID:8;                                 // [7 : 0]      PID : Packet ID. the field is valid only when TXS2H or TXS2M is set.
    ULONG   TxStatusFormat:1;                       // [8]      TXSFM : Tx status format. 0 : time stamp / front time mode. 1 : noisy / RCPI mode
    ULONG   TxStatusMCU:1;                          // [9]      TXS2M : Tx status must return to MCU. 0 : no effect. 1 : this packet's status must return to MCU
    ULONG   TxStatusHost:1;                             // [10]     TXS2H : Tx status must return to Host. 0 : no effect. 1 : this packet's status must return to Host.
    ULONG   DataSourceSelection:1;                  // [11]     DAS : data source selection. (valid for Tx translation only)
    ULONG   BARSSNContorl:1;                        // [12]     BSN : BAR SSN control. 0 : HW mode, the SSN is assigned by HW. 1 : SW mode, the SSN is assigned by SW.
    ULONG   PowerManagement:1;                  // [13]     PM : power management. 0 : controlled by HW. 1 : controlled by SW. This bit is used to enable/disable the power management control of PM bit in frame control field.
    ULONG   Reserve2:2;                                 // [15 : 14]    Reserve bits.
    ULONG   PacketNumberHigh:16;                        // [31 : 16]    Packet Number [47 : 32].
} TXDSCR_DW5, *PTXDSCR_DW5;

/* TXDSCR_DW6.FixedRateMode */
#define TMI_FIX_RATE_BY_TXD 0
#define TMI_FIX_RATE_BY_CR      1

#define TMI_TX_RATE_BIT_STBC        11
#define TMI_TX_RATE_BIT_NSS     9
#define TMI_TX_RATE_MASK_NSS        0x3

#define TMI_TX_RATE_BIT_MODE        6
#define TMI_TX_RATE_MASK_MODE   0x7
#define TMI_TX_RATE_MODE_CCK        0
#define TMI_TX_RATE_MODE_OFDM   1
#define TMI_TX_RATE_MODE_HTMIX  2
#define TMI_TX_RATE_MODE_HTGF       3
#define TMI_TX_RATE_MODE_VHT        4

#define TMI_TX_RATE_BIT_MCS     0
#define TMI_TX_RATE_MASK_MCS        0x3f
#define TMI_TX_RATE_CCK_1M_LP       0
#define TMI_TX_RATE_CCK_2M_LP       1
#define TMI_TX_RATE_CCK_5M_LP       2
#define TMI_TX_RATE_CCK_11M_LP  3

#define TMI_TX_RATE_CCK_2M_SP       5
#define TMI_TX_RATE_CCK_5M_SP       6
#define TMI_TX_RATE_CCK_11M_SP  7

#define TMI_TX_RATE_OFDM_6M     11
#define TMI_TX_RATE_OFDM_9M     15
#define TMI_TX_RATE_OFDM_12M        10
#define TMI_TX_RATE_OFDM_18M        14
#define TMI_TX_RATE_OFDM_24M        9
#define TMI_TX_RATE_OFDM_36M        13
#define TMI_TX_RATE_OFDM_48M        8
#define TMI_TX_RATE_OFDM_54M        12

#define TMI_TX_RATE_HT_MCS0     0
#define TMI_TX_RATE_HT_MCS1     1
#define TMI_TX_RATE_HT_MCS2     2
#define TMI_TX_RATE_HT_MCS3     3
#define TMI_TX_RATE_HT_MCS4     4
#define TMI_TX_RATE_HT_MCS5     5
#define TMI_TX_RATE_HT_MCS6     6
#define TMI_TX_RATE_HT_MCS7     7
#define TMI_TX_RATE_HT_MCS8     8
#define TMI_TX_RATE_HT_MCS9     9
#define TMI_TX_RATE_HT_MCS10        10
#define TMI_TX_RATE_HT_MCS11        11
#define TMI_TX_RATE_HT_MCS12        12
#define TMI_TX_RATE_HT_MCS13        13
#define TMI_TX_RATE_HT_MCS14        14
#define TMI_TX_RATE_HT_MCS15        15
#define TMI_TX_RATE_HT_MCS16        16
#define TMI_TX_RATE_HT_MCS17        17
#define TMI_TX_RATE_HT_MCS18        18
#define TMI_TX_RATE_HT_MCS19        19
#define TMI_TX_RATE_HT_MCS20        20
#define TMI_TX_RATE_HT_MCS21        21
#define TMI_TX_RATE_HT_MCS22        22
#define TMI_TX_RATE_HT_MCS23        23

#define TMI_TX_RATE_HT_MCS32        32

#define TMI_TX_RATE_VHT_MCS0        0
#define TMI_TX_RATE_VHT_MCS1        1
#define TMI_TX_RATE_VHT_MCS2        2
#define TMI_TX_RATE_VHT_MCS3        3
#define TMI_TX_RATE_VHT_MCS4        4
#define TMI_TX_RATE_VHT_MCS5        5
#define TMI_TX_RATE_VHT_MCS6        6
#define TMI_TX_RATE_VHT_MCS7        7
#define TMI_TX_RATE_VHT_MCS8        8
#define TMI_TX_RATE_VHT_MCS9        9
typedef struct _TXDSCR_DW6
{
    // DW6
    ULONG   FixedRateMode:1;                        // [0]      FRM : fixed rate mode. 0 : use the TX.Dscr's fixed rate to transmit current frame. 1 : use the CR's fixed rate to transmit current frame.
    ULONG   Reserve3:1;                                 // [1]      Reserve bit.
    ULONG   AntennaIndex:6;                         // [7 : 2]      ANT_ID : the smart antenna index..
    ULONG   FixedBandwidthMode:3;                   // [10 : 8] BW  :Fixed Bandwidth Mdoe.
    ULONG   SpetialExtenstionEnable:1;              // [11]     SPE_EN : Spetial Extenstion enable.
    ULONG   AntennaPriority:3;                      // [14 : 12]    ANT_PRI : Antenna priority.
    ULONG   DynamicBandwidth:1;                     // [15]     DYN_BW : use dynamic bandwidth.
    ULONG   ExplicitBeamforming:1;                  // [16]     ETXBF : Use explicit beamforming matrix to transmit packet.
    ULONG   ImplicitBeamforming:1;                  // [17]     ITXBF : Use implicit beamforming matrix to transmit packet.
    ULONG   RateToBeFixed:12;                       // [29 : 18]    Rate to be fixed. To see spec.
    ULONG   LDPC:1;                                     // [30]     LDPC. 0 : BCC. 1 : LDPC
    ULONG   GI:1;                                   // [31]     GI. Guard Interval mode of the Fixed Rate. 0 : normal. 1 : short GI
} TXDSCR_DW6, *PTXDSCR_DW6;

typedef struct _TXDSCR_DW7
{
    // DW7
    ULONG   SchedulerTxTime:16;                     // [15 : 0]
    ULONG   SwFieldReserve:5;                       // [22 : 16]
    ULONG   SwUseSegIdx:6;                          // [26 : 23]
    ULONG   SwUseUSB4ByteAlign:2;                   // [28 : 27]
    ULONG   SwUseAMSDU:1;                           // [29]
    ULONG   SwUseSegmentEnd:1;                  // [30]
    ULONG   SwUseNonQoS:1;                          // [31]
} TXDSCR_DW7, *PTXDSCR_DW7;

typedef struct _TXDSCR_LONG_STRUC
{
    // DW0
    TXDSCR_DW0      TxDscrDW0;
    // DW1
    TXDSCR_DW1      TxDscrDW1;
    // DW2
    TXDSCR_DW2      TxDscrDW2;  
    // DW3
    TXDSCR_DW3      TxDscrDW3;
    // DW4
    TXDSCR_DW4      TxDscrDW4;
    // DW5
    TXDSCR_DW5      TxDscrDW5;
    // DW6
    TXDSCR_DW6      TxDscrDW6;
    // DW7
    TXDSCR_DW7      TxDscrDW7;
} TXDSCR_LONG_STRUC, *PTXDSCR_LONG_STRUC;

typedef struct _TXDSCR_SHORT_STRUC
{
    // DW0
    TXDSCR_DW0      TxDscrDW0;
    // DW1
    TXDSCR_DW1      TxDscrDW1;
    // DW7
    TXDSCR_DW7      TxDscrDW7;
} TXDSCR_SHORT_STRUC, *PTXDSCR_SHORT_STRUC;



// ======================================================================================================
// Rx Structures
// ======================================================================================================

#define RMAC_RXD_0_PKT_TYPE_MASK        0xe0000000
#define RMAC_RX_PKT_TYPE_RX_TXS     0x00
#define RMAC_RX_PKT_TYPE_RX_TXRXV       0x01
#define RMAC_RX_PKT_TYPE_RX_NORMAL  0x02
#define RMAC_RX_PKT_TYPE_RX_DUP_RFB 0x03
#define RMAC_RX_PKT_TYPE_RX_TMR     0x04
#define RMAC_RX_PKT_TYPE(_x)        (((_x) & RMAC_RXD_0_PKT_TYPE_MASK) >> 29)

#define RMAC_RX_PKT_TYPE_RX_NORMAL_LENGTH   16
#define RMAC_RX_PKT_TYPE_RX_TMR_LENGTH      28
#define RXDSCR_LAST_STRUC_LENGTH                4


typedef struct _RXDSCR_DW0
{
    // DW0
    ULONG   RxByteCount:16;                             // [15 : 0] Rx Byte Count (byte)
    ULONG   EtherTypeOffset:7;                      // [22 : 16]    Ether-Type Offset (word)
    ULONG   IPChecksumOffload:1;                    // [23]     IP : IP checksum offload
    ULONG   UDPTCPChecksumOffload:1;                // [24]     UT : UDP/TCP checksum offload
    ULONG   RFBGroupValidIndicators:4;              // [28 : 25]    Group VLD : RFB(Receive Frame Buffer) Group valid indicators
    ULONG   PacketType:3;                           // [31 : 29]    Packet Type
} RXDSCR_DW0, *PRXDSCR_DW0;

typedef struct _RXDSCR_DW1
{
    // DW1
    ULONG   HTC:1;                                  // [0]      HTC : Four bytes HTC are padded after MAC header
    ULONG   UnicastToMe:1;                          // [1]      UC2ME : Unicast to Me
    ULONG   Multicast:1;                                // [2]      MC : Multicast
    ULONG   Broadcast:1;                                // [3]      BC : Broadcast
    ULONG   BeaconWithBroadcastMulticast:1;         // [4]      BBM : Beacon with Broadcast/Multicast
    ULONG   BeaconWithUnicast:1;                    // [5]      BU : Beacon with Unicast
    ULONG   KeyID:2;                                // [7 : 6]      KID : Key ID
    ULONG   ChannelFrequency:8;                     // [15 : 8] CH Frequency : Channel Frequency
    ULONG   MACHeaderLength:6;                      // [21 : 16]    MAC Header Length
    ULONG   HeaderOffset:1;                         // [22]     HO : Header Offset
    ULONG   HeaderTranslation:1;                        // [23]     H : Header Translation --> 0 : original 802.11 header ; 1 : received frame is performed by HW RX header translation(for Linux)
    ULONG   PayloadFormat:2;                        // [25 : 24]    PF : Payload Format
    ULONG   BSSID:6;                                // [31 : 26]    BSSID : indicate the matched BSSID
} RXDSCR_DW1, *PRXDSCR_DW1;

typedef struct _RXDSCR_DW2
{
    // DW2
    ULONG   WLANIndex:8;                            // [7 : 0]      WLAN index
    ULONG   TID:4;                                  // [11 : 8] TID : copy from the TID field in QoS control field, if the Rx packet is an QoS Data frame. It will be 0, if the Rx packet is not an QoS Data frame.
    ULONG   SecurityMode:4;                             // [15 : 12]    SEC_mode : Security engine operation mode.
    ULONG   SWBIT:1;                                // [16]     SWBIT : Software assigned bit in WTBL(WLan Table)
    ULONG   FCSError:1;                             // [17]     FC : FCS Error. If this bit is set, the received frame contains FCS error.
    ULONG   CipherMismatch:1;                       // [18]     CM : Cipher Mismatch.
    ULONG   CipherLengthMismatch:1;                 // [19]     CLM : Cipher Length Mismatch.
    ULONG   ICVError:1;                             // [20]     I : ICV Error
    ULONG   TKIPMICError:1;                         // [21]     T : TKIPMIC Error
    ULONG   LengthMismatch:1;                       // [22]     LM : Length Mismatch
    ULONG   DAMSDU:1;                               // [23]     DAF : DAMSDU Fail. --> 0 : DAMSDU is not failed ; 1 : DAMSDU is failed
    ULONG   ExceedMaxRxLength:1;                    // [24]     EL : Exceed maxium Rx Length
    ULONG   LLCMismatched:1;                        // [25]     LLC-MIS : LLC Mismatched. --> 0 : the pre-defined LLC-SNAP pattern is matched. ; 1 : the pre-defined LLC-SNAP pattern is un-matched.
    ULONG   UndefinedVLANTaggedType:1;          // [26]     UDF_VLT : Undefined VLAN Tagged Type. --> 0 : VLAN is followed by pre-defined LLC-SNAP. ; 1 : VLAN is followed by un-defined LLC-SNAP.
    ULONG   FragmentedFrame:1;                      // [27]     FRAG : Fragmented Frame
    ULONG   NullFrame:1;                            // [28]     NULL : NULL Frame
    ULONG   DataFrame:1;                            // [29]     DATA : Data Frame
    ULONG   NonAMPDUSubFrame:1;                 // [30]     NASF : A-MPDU Subframe. --> 0 : this is an A-MPDU subframe ; 1 : this is not an A-MPDU subframe. If A-MPDU de-aggregation function was disabled, this field always be 0.
    ULONG   NonAMPDUFrame:1;                        // [31]     NAMP : A-MPDU Frame. --> 0 : this is an A-MPDU frame ; 1 : this is a MPDU frame.
} RXDSCR_DW2, *PRXDSCR_DW2;

typedef struct _RXDSCR_DW3
{
    // DW3
    ULONG   RxVectorSequenceNumber:8;               // [7 : 0]      RXV_SN : Rx Vector Sequence Number. --> 0 : Rx vector is invalid. ; 1~255 : Rx Vector sequence number.
    ULONG   TSFCompareLoss:1;                       // [8]      TCL : TSF Compare Loss.
    ULONG   PatternFilterDropBit:1;                 // [9]      DP : Pattern Filter Drop Bit. --> 0 : a wanted packet.  1 : a drop packet. 
    ULONG   CoalescingPacket:1;                     // [10]     CLS : indicate the packet is a coalescing packet.
    ULONG   FWOffloadPacketFilter:2;                    // [12 : 11]    OFLD : FW offload packet filter. --> 0 : not match ; 1 : EAPOL packet ; 2 : ARP/NS packet ; 3 : TDLS packet
    ULONG   WakeupMagicPacket:1;                    // [13]     MGC : Wake up magic packet.
    ULONG   WOWFilterMatchingNumber:5;              // [18 : 14]    WOL : Wake-on-WLAN filter matching number..
    ULONG   CoalescingFilterMatchingFlag:10;            // [28 : 19]    CLS_BITMAP : coalescing filter matching flag
    ULONG   PatternFilterMode:1;                        // [29]     PF_MODE : the pattern filter mode. --> 0 : White list ; 1 : Black list
    ULONG   PacketFilterStatus:2;                       // [31 : 30]    PF_STS : this is status report of filter mode..
} RXDSCR_DW3, *PRXDSCR_DW3;
//
// RX Description (RXDscr) Main Part
//
// LMAC -> PSE
//
// Note: 
//  1. DW : double word
//
typedef struct _RXDSCR_BASE_STRUC
{
    // DW0
    RXDSCR_DW0      RxDscrDW0;
    // DW1
    RXDSCR_DW1      RxDscrDW1;
    // DW2
    RXDSCR_DW2      RxDscrDW2;  
    // DW3
    RXDSCR_DW3      RxDscrDW3;
} RXDSCR_BASE_STRUC, *PRXDSCR_BASE_STRUC;

//
// RX Description (RXDscr) Group 1
//
// LMAC -> PSE
//
// Note: 
//  1. DW : double word
//
typedef struct _RXDSCR_GROUP1_STRUC
{
    // DW8
    ULONG   ReplayDetection1;                           // [31 : 0] Replay Detection part 1 : TKIP_TSC[31 : 0] / CCMP_PN[31 : 0] / GCMP_PN[31 : 0] / WPI_PN[31 : 0]

    // DW9
    ULONG   ReplayDetection2:16;                            // [15 : 0] Replay Detection part 2 : TKIP_TSC[47 : 32] / CCMP_PN[47 : 32] / GCMP_PN[47 : 32] / WPI_PN[47 : 32]
    ULONG   ReplayDetection3:16;                            // [31 : 16]    Replay Detection part 3 : WPI_PN[63 : 48]

    // DW10
    ULONG   ReplayDetection4;                           // [31 : 0] Replay Detection part 4 : WPI_PN[95 : 64]

    // DW11
    ULONG   ReplayDetection5;                           // [31 : 0] Replay Detection part 5 : WPI_PN[127 : 96]

} RXDSCR_GROUP1_STRUC, *PRXDSCR_GROUP1_STRUC;

//
// RX Description (RXDscr) Group 2
//
// LMAC -> PSE
//
// Note: 
//  1. DW : double word
//
typedef struct _RXDSCR_GROUP2_STRUC
{
    // DW12
    ULONG   TimeStamp;                                  // [31 : 0] Time Stamp

    // DW13
    ULONG   CRC;                                        // [31 : 0] CRC32 value of this MPDU

} RXDSCR_GROUP2_STRUC, *PRXDSCR_GROUP2_STRUC;

//
// RX Description (RXDscr) Group 3
//
// LMAC -> PSE
//
// Note: 
//  1. DW : double word
//
typedef struct _RXDSCR_GROUP3_STRUC
{
    // DW14
    ULONG   RxVectorGroup1;                             // [31 : 0] RXV(1st cycle) : RX Vector Group 1 1st cycle. The value should refer to "MT6620 WiFi BB MAC Interface" document

    // DW15
    ULONG   RxVectorGroup2;                             // [31 : 0] RXV(2nd cycle) : RX Vector Group 2 2nd cycle. The value should refer to "MT6620 WiFi BB MAC Interface" document

    // DW16
    ULONG   RxVectorGroup3;                             // [31 : 0] RXV(3rd cycle) : RX Vector Group 3 3rd cycle. The value should refer to "MT6620 WiFi BB MAC Interface" document

    // DW17
    ULONG   RxVectorGroup4;                             // [31 : 0] RXV(4th cycle) : RX Vector Group 4 4th cycle. The value should refer to "MT6620 WiFi BB MAC Interface" document

    // DW18
    ULONG   RxVectorGroup5;                             // [31 : 0] RXV(5th cycle) : RX Vector Group 5 5th cycle. The value should refer to "MT6620 WiFi BB MAC Interface" document

    // DW19
    ULONG   RxVectorGroup6;                             // [31 : 0] RXV(6th cycle) : RX Vector Group 6 6th cycle. The value should refer to "MT6620 WiFi BB MAC Interface" document

} RXDSCR_GROUP3_STRUC, *PRXDSCR_GROUP3_STRUC;

//
// No need to implement this part, it's for Linux OS.
// RX Description (RXDscr) Group 4
//
// Note: 
//  1. DW : double word
//
/*
typedef struct _RXDSCR_GROUP4_STRUC
{
    // DW4

    // DW5

    // DW6

    // DW7

} RXDSCR_GROUP4_STRUC, *PRXDSCR_GROUP4_STRUC;
*/

//
// RX Description (RXDscr) Last (behind payload)
//
// HIF -> HOST
//
// Note: 
//  1. DW : double word
//
typedef struct _RXDSCR_LAST_STRUC
{
    // DW Last
    ULONG   ChecksumStatus:4;                           // [3 : 0]      CS Status : Checksum status. Indicates the checking result of packet checksum.
    ULONG   ChecksumType:4;                         // [7 : 4]      CS Type : Checksum type. Indicates the checksum type.
    ULONG   IPDatagramLengthMismatch:1;             // [8]      C : IP datagram length mismatch.
    ULONG   IPFragmentationPacket:1;                    // [9]      F : IP fragmentation packet.
    ULONG   UnknownNextHeader:1;                        // [10]     UN : Unknown next header.
    ULONG   ReserveDWLast:21;                           // [31 : 11]    Reserve bits..

} RXDSCR_LAST_STRUC, *PRXDSCR_LAST_STRUC;

#define MONITOR_ON(_p)              (((_p)->StaCfg.BssType) == BSS_MONITOR)

#if defined(CONFIG_AP_SUPPORT) && defined(CONFIG_STA_SUPPORT)
#define IF_DEV_CONFIG_OPMODE_ON_AP(_pAd)		if(_pAd->OpMode == OPMODE_AP)
#define IF_DEV_CONFIG_OPMODE_ON_STA(_pAd)		if(_pAd->OpMode == OPMODE_STA)
#define RT_CONFIG_IF_OPMODE_ON_AP(__OpMode)		if (__OpMode == OPMODE_AP)
#define RT_CONFIG_IF_OPMODE_ON_STA(__OpMode)	if (__OpMode == OPMODE_STA)
#else
#define IF_DEV_CONFIG_OPMODE_ON_AP(_pAd)
#define IF_DEV_CONFIG_OPMODE_ON_STA(_pAd)
#define RT_CONFIG_IF_OPMODE_ON_AP(__OpMode)
#define RT_CONFIG_IF_OPMODE_ON_STA(__OpMode)
#endif

//#define IS_MT7603(_pAd)		(((_pAd)->ChipID & 0x0000ffff) == 0x00007603)
#define IS_MT7603(_pAd)		(TRUE)
#define IS_MT7603E(_pAd)	((((_pAd)->ChipID & 0x0000ffff) == 0x00007603) && IS_PCIE_INF(_pAd))
#define IS_MT7603U(_pAd)	((((_pAd)->ChipID & 0x0000ffff) == 0x00007603) && IS_USB_INF(_pAd) && (_pAd->AntMode == 0))
#define IS_MT76031U(_pAd)	((((_pAd)->ChipID & 0x0000ffff) == 0x00007603) && IS_USB_INF(_pAd) && (_pAd->AntMode == 1))

#define IS_MT7628(_pAd)		(((_pAd)->ChipID & 0x0000ffff) == 0x00007628)

#define MT7603E1 0x0000
#define MT7603E2 0x0010

#define MT76x6E1 0x8A00		/* MT7636 E1 */
#define MT76x6E2 0x8A01		/* MT7636 E2 TBD */

#define ASIC_MAC_TX                 1
#define ASIC_MAC_RX                 2
#define ASIC_MAC_TXRX             3
#define ASIC_MAC_TXRX_RXV     4
#define ASIC_MAC_RXV               5
#define ASIC_MAC_RX_RXV         6

#define SLOT_TIME_24G_LONG		20  // 802.11b (DS), 802.11g (ERP), 802.11n
#define SLOT_TIME_24G_SHORT	9	// 802.11g, 802.11n
#define SLOT_TIME_5G			9	// 802.11a, 802.11n, 802.11ac

#define SIFS_TIME_24G		10
#define SIFS_TIME_5G		16

#define RIFS_TIME		2	//802.11n
#define EIFS_TIME		360


#ifdef MT7603_FPGA
#define MT_PSE_PAGE_SIZE        256
#define MT_WTBL_SIZE            20
#else
#define MT_PSE_PAGE_SIZE        128
#define MT_WTBL_SIZE            128
#endif /* MT7603_FPGA */


#define RtmpusecDelay(_S) NdisCommonGenericDelay(_S)

/*
#define IS_MT7636(_pAd)     ((((_pAd)->ChipID & 0x0000ffff) == 0x00007606)||\
							(((_pAd)->ChipID & 0x0000ffff) == 0x00007636) || \
							(((_pAd)->ChipID & 0x0000ffff) == 0x00007611))
*/

/*	MT76x6 series: 7636/7606/7611
 	1. For definition/compiler option, use MT7636 for all 7636/7603/7611
 	2. For run time case (specific H/W), please use IS_MT7636(), IS_MT7606(), MT7611()
 	3. For run time case (all H/Ws), please use IS_MT76x6()
 	4. For different interface, please use IS_MT7636U(), IS_MT7636S() */
#define IS_MT76x6(_pAd)     ((((_pAd)->ChipID & 0x0000ffff) == 0x00007606)||\
							(((_pAd)->ChipID & 0x0000ffff) == 0x00007636) || \
							(((_pAd)->ChipID & 0x0000ffff) == 0x00007611))

#define IS_MT7636U(_pAd)	((((_pAd)->ChipID & 0x0000ffff) == 0x00007606) && IS_USB_INF(_pAd))
#define IS_MT7636S(_pAd)	((((_pAd)->ChipID & 0x0000ffff) == 0x00007606) && IS_SDIO_INF(_pAd))

#define IS_MT7636(_pAd)		(((_pAd)->ChipID & 0x0000ffff) == 0x00007636)
#define IS_MT7606(_pAd)		(((_pAd)->ChipID & 0x0000ffff) == 0x00007606)
#define IS_MT7611(_pAd)		(((_pAd)->ChipID & 0x0000ffff) == 0x00007611)


#define RT_REV_LT(_pAd, _chip, _rev)\
	IS_##_chip(_pAd) && (((_pAd)->MACVersion & 0x0000FFFF) < ((ULONG)_rev))

#define RT_REV_GTE(_pAd, _chip, _rev)\
	IS_##_chip(_pAd) && (((_pAd)->MACVersion & 0x0000FFFF) >= ((ULONG)_rev))

#define MT_REV_LT(_pAd, _chip, _rev)\
	IS_##_chip(_pAd) && (((_pAd)->ChipID & 0x0000FFFF) < ((ULONG)_rev))

#define MT_REV_GTE(_pAd, _chip, _rev)\
	IS_##_chip(_pAd) && (((_pAd)->ChipID & 0x0000FFFF) >= ((ULONG)_rev))

#define MTK_REV_LT(_pAd, _chip, _rev)\
	IS_##_chip(_pAd) && (((_pAd)->HWVersion & 0x000000ff) < ((ULONG)_rev))

#define MTK_REV_GTE(_pAd, _chip, _rev)\
	IS_##_chip(_pAd) && (((_pAd)->HWVersion & 0x000000ff) >= ((ULONG)_rev))


#define DMA_SCH_LMAC            0
#define DMA_SCH_BYPASS         1
#define DMA_SCH_HYBRID         2

typedef struct _MT_TX_COUNTER
{
    UINT32 TxCount;
    UINT32 TxFailCount;
    UINT16 Rate1TxCnt;
    UINT16 Rate1FailCnt;
    UCHAR Rate2TxCnt;
    UCHAR Rate3TxCnt;
    UCHAR Rate4TxCnt;
    UCHAR Rate5TxCnt;
    UCHAR RateIndex;
} MT_TX_COUNTER;


INT32 MtAsicSetMacTxRx(struct _RTMP_ADAPTER *pAd, INT32 TxRx, BOOLEAN Enable);
UINT32 AsicSetMacTxRx(MP_ADAPTER *pAd, UINT32 TxRx, UINT8 Enable);
#endif  // __RT7603_BACKUP_H__
