/*
 ***************************************************************************
 * MediaTek Inc. 
 *
 * All rights reserved. source code is an unpublished work and the
 * use of a copyright notice does not imply otherwise. This source code
 * contains confidential trade secret material of MediaTek. Any attemp
 * or participation in deciphering, decoding, reverse engineering or in any
 * way altering the source code is stricitly prohibited, unless the prior
 * written consent of MediaTek, Inc. is obtained.
 ***************************************************************************

	Module Name:
	mt7615.c
*/

#include "rt_config.h"
#include "mcu/mt7615_firmware.h"


#ifdef MT7615_FPGA
REG_CHK_PAIR hif_dft_cr[]=
{
	{HIF_BASE + 0x00, 0xffffffff, 0x76030001},
	{HIF_BASE + 0x04, 0xffffffff, 0x1b},
	{HIF_BASE + 0x10, 0xffffffff, 0x3f01},
	{HIF_BASE + 0x20, 0xffffffff, 0xe01001e0},
	{HIF_BASE + 0x24, 0xffffffff, 0x1e00000f},

	{HIF_BASE + 0x200, 0xffffffff, 0x0},
	{HIF_BASE + 0x204, 0xffffffff, 0x0},
	{HIF_BASE + 0x208, 0xffffffff, 0x52000c50},
	{HIF_BASE + 0x20c, 0xffffffff, 0x0},
	{HIF_BASE + 0x210, 0xffffffff, 0x0},
	{HIF_BASE + 0x214, 0xffffffff, 0x0},
	{HIF_BASE + 0x218, 0xffffffff, 0x0},
	{HIF_BASE + 0x21c, 0xffffffff, 0x0},
	{HIF_BASE + 0x220, 0xffffffff, 0x0},
	{HIF_BASE + 0x224, 0xffffffff, 0x0},
	{HIF_BASE + 0x234, 0xffffffff, 0x0},
	{HIF_BASE + 0x244, 0xffffffff, 0x0},
	{HIF_BASE + 0x300, 0xffffffff, 0x0},
	{HIF_BASE + 0x304, 0xffffffff, 0x0},
	{HIF_BASE + 0x308, 0xffffffff, 0x0},
	{HIF_BASE + 0x30c, 0xffffffff, 0x0},
	{HIF_BASE + 0x310, 0xffffffff, 0x0},
	{HIF_BASE + 0x314, 0xffffffff, 0x0},
	{HIF_BASE + 0x318, 0xffffffff, 0x0},
	{HIF_BASE + 0x31c, 0xffffffff, 0x0},
	{HIF_BASE + 0x320, 0xffffffff, 0x0},
	{HIF_BASE + 0x324, 0xffffffff, 0x0},
	{HIF_BASE + 0x328, 0xffffffff, 0x0},
	{HIF_BASE + 0x32c, 0xffffffff, 0x0},
	{HIF_BASE + 0x330, 0xffffffff, 0x0},
	{HIF_BASE + 0x334, 0xffffffff, 0x0},
	{HIF_BASE + 0x338, 0xffffffff, 0x0},
	{HIF_BASE + 0x33c, 0xffffffff, 0x0},

	{HIF_BASE + 0x400, 0xffffffff, 0x0},
	{HIF_BASE + 0x404, 0xffffffff, 0x0},
	{HIF_BASE + 0x408, 0xffffffff, 0x0},
	{HIF_BASE + 0x40c, 0xffffffff, 0x0},
	{HIF_BASE + 0x410, 0xffffffff, 0x0},
	{HIF_BASE + 0x414, 0xffffffff, 0x0},
	{HIF_BASE + 0x418, 0xffffffff, 0x0},
	{HIF_BASE + 0x41c, 0xffffffff, 0x0},
};

INT mt7615_chk_hif_default_cr_setting(RTMP_ADAPTER *pAd)
{
	UINT32 val;
	INT i;
	BOOLEAN match = TRUE;

	DBGPRINT(RT_DEBUG_OFF, ("%s(): Default CR Setting Checking for HIF!\n", __FUNCTION__));
	for (i = 0; i < sizeof(hif_dft_cr) / sizeof(REG_CHK_PAIR); i++)
	{
		RTMP_IO_READ32(pAd, hif_dft_cr[i].Register, &val);
		DBGPRINT(RT_DEBUG_OFF, ("\t Reg(%x): Current=0x%x(0x%x), Default=0x%x, Mask=0x%x, Match=%s\n", 
						hif_dft_cr[i].Register, val, (val & hif_dft_cr[i].Mask), 
						hif_dft_cr[i].Value, hif_dft_cr[i].Mask,
						((val & hif_dft_cr[i].Mask)!= hif_dft_cr[i].Value) ? "No" : "Yes"));

		if ((val & hif_dft_cr[i].Mask)!= hif_dft_cr[i].Value)
		{
			match = FALSE;
		}
	}
	DBGPRINT(RT_DEBUG_OFF, ("%s(): Checking Done, Result=> %s match!\n",
				__FUNCTION__, match == TRUE ? "All" : "No"));

	return match;
}


REG_CHK_PAIR top_dft_cr[]=
{
	{TOP_CFG_BASE+ 0x1000, 0xffffffff, 0x0},
	{TOP_CFG_BASE+ 0x1004, 0xffffffff, 0x0},
	{TOP_CFG_BASE+ 0x1008, 0xffffffff, 0x0},
	{TOP_CFG_BASE+ 0x1010, 0xffffffff, 0x0},

	{TOP_CFG_BASE+ 0x1100, 0xffffffff, 0x26110310},
	{TOP_CFG_BASE+ 0x1108, 0x0000ff00, 0x1400},
	{TOP_CFG_BASE+ 0x110c, 0x00000000, 0x0},
	{TOP_CFG_BASE+ 0x1110, 0x0f0f00ff, 0x02090040},
	{TOP_CFG_BASE+ 0x1124, 0xf000f00f, 0x00000008},
	{TOP_CFG_BASE+ 0x1130, 0x000f0000, 0x0},
	{TOP_CFG_BASE+ 0x1134, 0x00000000, 0x0},
	{TOP_CFG_BASE+ 0x1140, 0x00ff00ff, 0x0},
	
	{TOP_CFG_BASE+ 0x1200, 0x00000000, 0x0},
	{TOP_CFG_BASE+ 0x1204, 0x000fffff, 0x0},
	{TOP_CFG_BASE+ 0x1208, 0x000fffff, 0x0},
	{TOP_CFG_BASE+ 0x120c, 0x000fffff, 0x0},
	{TOP_CFG_BASE+ 0x1210, 0x000fffff, 0x0},
	{TOP_CFG_BASE+ 0x1214, 0x000fffff, 0x0},
	{TOP_CFG_BASE+ 0x1218, 0x000fffff, 0x0},
	{TOP_CFG_BASE+ 0x121c, 0x000fffff, 0x0},
	{TOP_CFG_BASE+ 0x1220, 0x000fffff, 0x0},
	{TOP_CFG_BASE+ 0x1224, 0x000fffff, 0x0},
	{TOP_CFG_BASE+ 0x1228, 0x000fffff, 0x0},
	{TOP_CFG_BASE+ 0x122c, 0x000fffff, 0x0},
	{TOP_CFG_BASE+ 0x1234, 0x00ffffff, 0x0},
	{TOP_CFG_BASE+ 0x1238, 0x00ffffff, 0x0},
	{TOP_CFG_BASE+ 0x123c, 0xffffffff, 0x5c1fee80},
	{TOP_CFG_BASE+ 0x1240, 0xffffffff, 0x6874ae05},
	{TOP_CFG_BASE+ 0x1244, 0xffffffff, 0x00fb89f1},

	{TOP_CFG_BASE+ 0x1300, 0xffffffff, 0x0},
	{TOP_CFG_BASE+ 0x1304, 0xffffffff, 0x8f020006},
	{TOP_CFG_BASE+ 0x1308, 0xffffffff, 0x18010000},
	{TOP_CFG_BASE+ 0x130c, 0xffffffff, 0x0130484f},
	{TOP_CFG_BASE+ 0x1310, 0xffffffff, 0xff000004},
	{TOP_CFG_BASE+ 0x1314, 0xffffffff, 0xf0000084},
	{TOP_CFG_BASE+ 0x1318, 0x00000000, 0x0},
	{TOP_CFG_BASE+ 0x131c, 0xffffffff, 0x0},
	{TOP_CFG_BASE+ 0x1320, 0xffffffff, 0x0},
	{TOP_CFG_BASE+ 0x1324, 0xffffffff, 0x0},
	{TOP_CFG_BASE+ 0x1328, 0xffffffff, 0x0},
	{TOP_CFG_BASE+ 0x132c, 0xffffffff, 0x0},
	{TOP_CFG_BASE+ 0x1330, 0xffffffff, 0x00007800},
	{TOP_CFG_BASE+ 0x1334, 0x00000000, 0x0},
	{TOP_CFG_BASE+ 0x1338, 0xffffffff, 0x0000000a},
	{TOP_CFG_BASE+ 0x1400, 0xffffffff, 0x0},
	{TOP_CFG_BASE+ 0x1404, 0xffffffff, 0x00005180},
	{TOP_CFG_BASE+ 0x1408, 0xffffffff, 0x00001f00},
	{TOP_CFG_BASE+ 0x140c, 0xffffffff, 0x00000020},
	{TOP_CFG_BASE+ 0x1410, 0xffffffff, 0x0000003a},
	{TOP_CFG_BASE+ 0x141c, 0xffffffff, 0x0},
	
	{TOP_CFG_BASE+ 0x1500, 0xffffffff, 0x0},
	{TOP_CFG_BASE+ 0x1504, 0xffffffff, 0x0},
};

INT mt7615_chk_top_default_cr_setting(RTMP_ADAPTER *pAd)
{
	UINT32 val;
	INT i;
	BOOLEAN match = TRUE;

	DBGPRINT(RT_DEBUG_OFF, ("%s(): Default CR Setting Checking for TOP!\n", __FUNCTION__));
	for (i = 0; i < sizeof(top_dft_cr) / sizeof(REG_CHK_PAIR); i++)
	{
		RTMP_IO_READ32(pAd, top_dft_cr[i].Register, &val);
		DBGPRINT(RT_DEBUG_OFF, ("\t Reg(%x): Current=0x%x(0x%x), Default=0x%x, Mask=0x%x, Match=%s\n", 
					top_dft_cr[i].Register, val, (val & top_dft_cr[i].Mask), 
					top_dft_cr[i].Value, top_dft_cr[i].Mask,
					((val & top_dft_cr[i].Mask)!= top_dft_cr[i].Value) ? "No" : "Yes"));
		if ((val & top_dft_cr[i].Mask) != top_dft_cr[i].Value)
		{
			match = FALSE;
		}
	}
	DBGPRINT(RT_DEBUG_OFF, ("%s(): Checking Done, Result=> %s match!\n",
				__FUNCTION__, match == TRUE ? "All" : "No"));
	
	return match;

}
#endif /* MT7615_FPGA */


static VOID mt7615_bbp_adjust(RTMP_ADAPTER *pAd)
{
}

static void mt7615_tx_pwr_gain(RTMP_ADAPTER *pAd, UINT8 channel)
{
}


static void mt7615_switch_channel(RTMP_ADAPTER *pAd, UCHAR channel, BOOLEAN scan)
{
	DBGPRINT(RT_DEBUG_ERROR, ("%s() todo \n", __FUNCTION__));
	return;
}


/*
    Init TxD short format template which will copy by PSE-Client to LMAC
*/
static INT asic_set_tmac_info_template(RTMP_ADAPTER *pAd)
{
		UINT32 dw[5];
		TMAC_TXD_2 *dw2 = (TMAC_TXD_2 *)(&dw[0]);
		TMAC_TXD_3 *dw3 = (TMAC_TXD_3 *)(&dw[1]);
		TMAC_TXD_4 *dw4 = (TMAC_TXD_4 *)(&dw[2]);
		TMAC_TXD_5 *dw5 = (TMAC_TXD_5 *)(&dw[3]);
		TMAC_TXD_6 *dw6 = (TMAC_TXD_6 *)(&dw[4]);

		NdisZeroMemory((UCHAR *)(&dw[0]), sizeof(dw));

		dw2->htc_vld = 0;
		dw2->frag = 0;
		dw2->max_tx_time = 0;
		dw2->fix_rate = 0;

		dw3->remain_tx_cnt = 0xf;
		dw3->sn_vld = 0;
		dw3->pn_vld = 0;

		dw5->pid = PID_DATA_AMPDU;
		dw5->tx_status_fmt = 0;
		dw5->tx_status_2_host = 1;
		dw5->bar_sn_ctrl = 0; //HW
		dw5->pwr_mgmt = TMI_PM_BIT_CFG_BY_HW; // HW

#ifdef RTMP_PCI_SUPPORT
// TODO: shaing, for MT7628, may need to change this as RTMP_MAC_PCI

        RTMP_IO_WRITE32(pAd, MCU_PCIE_REMAP_2, 0x80080000);


        /* For short format */
		RTMP_IO_WRITE32(pAd, 0xc0040, dw[0]);
		RTMP_IO_WRITE32(pAd, 0xc0044, dw[1]);
		RTMP_IO_WRITE32(pAd, 0xc0048, dw[2]);
		RTMP_IO_WRITE32(pAd, 0xc004c, dw[3]);
		RTMP_IO_WRITE32(pAd, 0xc0050, dw[4]);


// TODO: shaing, for MT7628, may need to change this as RTMP_MAC_PCI
	// After change the Tx Padding CR of PCI-E Client, we need to re-map for PSE region
	RTMP_IO_WRITE32(pAd, MCU_PCIE_REMAP_2, MT_PSE_BASE_ADDR);

#endif /* RTMP_PCI_SUPPORT */

	return TRUE;
}


static INT mt7615_init_mac_cr(RTMP_ADAPTER *pAd)
{
	DBGPRINT(RT_DEBUG_OFF, ("%s()-->\n", __FUNCTION__));
	return TRUE;
}


static VOID MT7615BBPInit(RTMP_ADAPTER *pAd)
{
	DBGPRINT(RT_DEBUG_ERROR, ("%s() todo \n", __FUNCTION__));
	return;
}


static void mt7615_init_rf_cr(RTMP_ADAPTER *ad)
{
	return;
}


int mt7615_read_chl_pwr(RTMP_ADAPTER *pAd)
{
	return TRUE;
}


/* Read power per rate */
void mt7615_get_tx_pwr_per_rate(RTMP_ADAPTER *pAd)
{
	DBGPRINT(RT_DEBUG_ERROR, ("%s() todo \n", __FUNCTION__));
	return;
}


void mt7615_get_tx_pwr_info(RTMP_ADAPTER *pAd)
{
	DBGPRINT(RT_DEBUG_ERROR, ("%s() todo \n", __FUNCTION__));
	return;
}

	
static void mt7615_antenna_default_reset(
	struct _RTMP_ADAPTER *pAd,
	EEPROM_ANTENNA_STRUC *pAntenna)
{
	// TODO: shiang-MT7615
	DBGPRINT(RT_DEBUG_ERROR, ("%s() todo \n", __FUNCTION__));
	
	pAntenna->word = 0;
	pAd->RfIcType = RFIC_7615;
	pAntenna->field.TxPath = 1;
	pAntenna->field.RxPath = 1;
}


#ifdef CONFIG_STA_SUPPORT
static VOID mt7615_init_dev_nick_name(RTMP_ADAPTER *ad)
{
	if (IS_MT7615(ad))
		snprintf((RTMP_STRING) ad->nickname, sizeof(ad->nickname), "mt7615_sta");
}
#endif /* CONFIG_STA_SUPPORT */


static const RTMP_CHIP_CAP MT7615_ChipCap = {
	.max_nss = 4,
	.TXWISize = sizeof(TMAC_TXD_L), /* 32 */
	.RXWISize = 28,
#ifdef RTMP_MAC_PCI
	.WPDMABurstSIZE = 3,
#endif
	.SnrFormula = SNR_FORMULA4,
	.FlgIsHwWapiSup = TRUE,
	.FlgIsHwAntennaDiversitySup = FALSE,
#ifdef STREAM_MODE_SUPPORT
	.FlgHwStreamMode = FALSE,
#endif
#ifdef FIFO_EXT_SUPPORT
	.FlgHwFifoExtCap = FALSE,
#endif
	.asic_caps = (fASIC_CAP_PMF_ENC | fASIC_CAP_MCS_LUT),
	.phy_caps = (fPHY_CAP_24G | fPHY_CAP_5G | fPHY_CAP_HT | fPHY_CAP_VHT),
	.MaxNumOfRfId = MAX_RF_ID,
	.pRFRegTable = NULL,
	.MaxNumOfBbpId = 200,
	.pBBPRegTable = NULL,
	.bbpRegTbSize = 0,
#ifdef NEW_MBSSID_MODE
#ifdef ENHANCE_NEW_MBSSID_MODE
	.MBSSIDMode = MBSSID_MODE4,
#else
	.MBSSIDMode = MBSSID_MODE1,
#endif /* ENHANCE_NEW_MBSSID_MODE */
#else
	.MBSSIDMode = MBSSID_MODE0,
#endif /* NEW_MBSSID_MODE */
#ifdef RTMP_EFUSE_SUPPORT
	.EFUSE_USAGE_MAP_START = 0x1e0,
	.EFUSE_USAGE_MAP_END = 0x1fc,
	.EFUSE_USAGE_MAP_SIZE = 29,
	.EFUSE_RESERVED_SIZE = 22,	// Cal-Free is 22 free block
#endif
	.EEPROM_DEFAULT_BIN = NULL,
	.EEPROM_DEFAULT_BIN_SIZE = 0,
#ifdef CONFIG_ANDES_SUPPORT
	.CmdRspRxRing = RX_RING1,
	// TODO: shiang-MT7615, need load fw!
	.need_load_fw = TRUE,
	.DownLoadType = DownLoadTypeC,
	.rom_patch_header_image = NULL, /*mt7615_rom_patch, */
	.rom_patch_len = 0, /* sizeof(mt7615_rom_patch), */
	.load_code_method = HEADER_METHOD,
	// TODO: shiang-MT7615, need load patch!
	.need_load_rom_patch = FALSE,
	.ram_code_protect = FALSE,
	.rom_code_protect = TRUE,
	.ilm_offset = 0x00080000,
	.dlm_offset = 0x02090000,
	.rom_patch_offset = 0x8C000,
#endif
	.MCUType = ANDES,
	.cmd_header_len = sizeof(FW_TXD),
#ifdef RTMP_PCI_SUPPORT
	.cmd_padding_len = 0,
#endif
	.fw_header_image = MT7615_FirmwareImage,
	.fw_len = sizeof(MT7615_FirmwareImage),
#ifdef CARRIER_DETECTION_SUPPORT
	.carrier_func = TONE_RADAR_V2,
#endif
	.hif_type = HIF_MT,
	.rf_type = RF_MT,
};


static const RTMP_CHIP_OP MT7615_ChipOp = {
	.ChipBBPAdjust = mt7615_bbp_adjust,
	.ChipSwitchChannel = mt7615_switch_channel,
	.AsicMacInit = mt7615_init_mac_cr,
	.AsicBbpInit = MT7615BBPInit,
	.AsicRfInit = mt7615_init_rf_cr,
	.AsicAntennaDefaultReset = mt7615_antenna_default_reset,
	.ChipAGCInit = NULL,
#ifdef CONFIG_STA_SUPPORT
	.ChipAGCAdjust = NULL,
#endif
	.AsicRfTurnOn = NULL,
	.AsicHaltAction = NULL,
	.AsicRfTurnOff = NULL,
	.AsicReverseRfFromSleepMode = NULL,
#ifdef CONFIG_STA_SUPPORT
	.NetDevNickNameInit = mt7615_init_dev_nick_name,
#endif
#ifdef CARRIER_DETECTION_SUPPORT
	.ToneRadarProgram = ToneRadarProgram_v2,
#endif 
	.RxSensitivityTuning = NULL,
	.DisableTxRx = NULL,
#ifdef RTMP_PCI_SUPPORT
	//.AsicRadioOn = RT28xxPciAsicRadioOn,
	//.AsicRadioOff = RT28xxPciAsicRadioOff,
#endif
	.show_pwr_info = NULL,
};


VOID mt7615_init(RTMP_ADAPTER *pAd)
{
	RTMP_CHIP_CAP *pChipCap = &pAd->chipCap;

	DBGPRINT(RT_DEBUG_OFF, ("%s()-->\n", __FUNCTION__));
	
	memcpy(&pAd->chipCap, &MT7615_ChipCap, sizeof(RTMP_CHIP_CAP));
	memcpy(&pAd->chipOps, &MT7615_ChipOp, sizeof(RTMP_CHIP_OP));

	pAd->chipCap.hif_type = HIF_MT;
	pAd->chipCap.mac_type = MAC_MT;
	pAd->TxSwRingNum = 2;

	mt_phy_probe(pAd);

	pChipCap->tx_hw_hdr_len = pChipCap->TXWISize;
	pChipCap->rx_hw_hdr_len = pChipCap->RXWISize;

#ifdef CONFIG_CSO_SUPPORT
	pChipCap->asic_caps |= fASIC_CAP_CSO,
#endif

	RTMP_DRS_ALG_INIT(pAd, RATE_ALG_GRP);
		
	/*
		Following function configure beacon related parameters
		in pChipCap
			FlgIsSupSpecBcnBuf / BcnMaxHwNum / 
			WcidHwRsvNum / BcnMaxHwSize / BcnBase[]
	*/
	mt_bcn_buf_init(pAd);

#ifdef DOT11W_PMF_SUPPORT
	pChipCap->FlgPMFEncrtptMode = PMF_ENCRYPT_MODE_2;
#endif /* DOT11W_PMF_SUPPORT */

	DBGPRINT(RT_DEBUG_OFF, ("<--%s()\n", __FUNCTION__));
}

