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
	mt7636.c
*/

#include "rt_config.h"
#include "mcu/mt7636_rom_patch.h"
#include "mcu/mt7636_firmware.h"
#include "eeprom/mt7636_e2p.h"
#include "phy/wf_phy_back.h"

static VOID mt7636_bbp_adjust(RTMP_ADAPTER *pAd)
{
	static char *ext_str[]={"extNone", "extAbove", "", "extBelow"};
	UCHAR rf_bw, ext_ch;

#if defined(MT7636_FPGA)
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("%s() todo \n", __FUNCTION__));
	return;
#endif /* MT7636_FPGA */

#ifdef DOT11_N_SUPPORT
	if (get_ht_cent_ch(pAd, &rf_bw, &ext_ch) == FALSE)
#endif /* DOT11_N_SUPPORT */
	{
		rf_bw = BW_20;
		ext_ch = EXTCHA_NONE;
		pAd->CommonCfg.CentralChannel = pAd->CommonCfg.Channel;
	}

	bbp_set_bw(pAd, rf_bw);

#ifdef DOT11_N_SUPPORT
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("%s() : %s, ChannelWidth=%d, Channel=%d, ExtChanOffset=%d(%d) \n",
					__FUNCTION__, ext_str[ext_ch],
					pAd->CommonCfg.HtCapability.HtCapInfo.ChannelWidth,
					pAd->CommonCfg.Channel,
					pAd->CommonCfg.RegTransmitSetting.field.EXTCHA,
					pAd->CommonCfg.AddHTInfo.AddHtInfo.ExtChanOffset));
#endif /* DOT11_N_SUPPORT */
}


static void mt7636_switch_channel(RTMP_ADAPTER *pAd, UCHAR channel, BOOLEAN scan)
{
#if defined(MT7636_FPGA)
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("%s(%d) todo \n", __FUNCTION__, channel));
	return;
#endif /* MT7636_FPGA */

#ifdef RTMP_MAC_USB
	UINT32 ret = 0;

	if (IS_USB_INF(pAd)) {
		RTMP_SEM_EVENT_WAIT(&pAd->hw_atomic, ret);
		if (ret != 0) {
			MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("reg_atomic get failed(ret=%d)\n", ret));
			return STATUS_UNSUCCESSFUL;
		}
	}
#endif /* RTMP_MAC_USB */

	if (pAd->CommonCfg.BBPCurrentBW == BW_20)
		CmdChannelSwitch(pAd, channel, channel, BW_20,
								pAd->CommonCfg.TxStream, pAd->CommonCfg.RxStream, scan);
	else
		CmdChannelSwitch(pAd, pAd->CommonCfg.Channel, channel, pAd->CommonCfg.BBPCurrentBW,
							pAd->CommonCfg.TxStream, pAd->CommonCfg.RxStream, scan);

	/* Channel latch */
	pAd->LatchRfRegs.Channel = channel;

#ifdef RTMP_MAC_USB
	if (IS_USB_INF(pAd)) {
		RTMP_SEM_EVENT_UP(&pAd->hw_atomic);
	}
#endif

	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE,
			("%s(): Switch to Ch#%d(%dT%dR), BBP_BW=%d\n",
			__FUNCTION__,
			channel,
			pAd->CommonCfg.TxStream,
			pAd->CommonCfg.RxStream,
			pAd->CommonCfg.BBPCurrentBW));
}


static VOID mt7636_init_mac_cr(RTMP_ADAPTER *pAd)
{
	UINT32 mac_val;

	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("%s()-->\n", __FUNCTION__));

#ifdef MT7636_FPGA
	// enable MAC2MAC mode
	RTMP_IO_READ32(pAd, RMAC_MISC, &mac_val);
	mac_val |= BIT18;
	RTMP_IO_WRITE32(pAd, RMAC_MISC, mac_val);

	/*
	sync. MT7628.c
	WHQA_00016749, there are a lots of Tx retry in FPGA.
	per Jacky's comment, enlarge the timeout value of waiting ack.
	real chip shall not have the symptom.
	*/
	mac_val = 0x00d700d7;
	RTMP_IO_WRITE32(pAd, TMAC_CDTR, mac_val);
	mac_val = 0x00d700d7;
	RTMP_IO_WRITE32(pAd, TMAC_ODTR, mac_val);
#endif /* MT7603_FPGA */

	/* Disable 802.11 to 802.3 Hdr tranlation */
	RTMP_IO_READ32(pAd, DMA_DCR0, &mac_val);
	mac_val &= ~BIT19;
	RTMP_IO_WRITE32(pAd, DMA_DCR0, mac_val);

	/* TxS Setting */
	InitTxSTypeTable(pAd);
	MtAsicSetTxSClassifyFilter(pAd, TXS2HOST, TXS2H_QID1, TXS2HOST_AGGNUMS, 0x00);
	MtAsicSetTxSClassifyFilter(pAd, TXS2MCU, TXS2M_QID0, TXS2MCU_AGGNUMS, 0x00);
}


/* Read power per rate */
void mt7636_get_tx_pwr_per_rate(RTMP_ADAPTER *pAd)
{
    BOOLEAN is_empty = 0;
    UINT16 value = 0;
	struct MT_TX_PWR_CAP *cap = &pAd->chipCap.MTTxPwrCap;

    /* G Band tx power for CCK 1M/2M, 5.5M/11M */
    is_empty = RT28xx_EEPROM_READ16(pAd, TX_PWR_CCK_1_2M, value);
    if (is_empty) {
        cap->tx_pwr_cck_1_2 = 0;
        cap->tx_pwr_cck_5_11 = 0;
    } else {
        /* CCK 1M/2M */
        if (value & TX_PWR_CCK_1_2M_EN) {
            if (value & TX_PWR_CCK_1_2M_SIGN) {
                cap->tx_pwr_cck_1_2 = (value & TX_PWR_CCK_1_2M_MASK);
            } else {
                cap->tx_pwr_cck_1_2 = -(value & TX_PWR_CCK_1_2M_MASK);
            }
        } else {
            cap->tx_pwr_cck_1_2 = 0;
        }

        /* CCK 5.5M/11M */
        if (value & TX_PWR_CCK_5_11M_EN) {
            if (value & TX_PWR_CCK_5_11M_SIGN) {
                cap->tx_pwr_cck_5_11 = ((value & TX_PWR_CCK_5_11M_MASK) >> 8);
            } else {
                cap->tx_pwr_cck_5_11 = -((value & TX_PWR_CCK_5_11M_MASK) >> 8);
            }
        } else {
            cap->tx_pwr_cck_5_11 = 0;
        }
    }

    MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("tx_pwr_cck_1_2 = %d\n", cap->tx_pwr_cck_1_2));
    MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("tx_pwr_cck_5_11 = %d\n", cap->tx_pwr_cck_5_11));

    /* G Band tx power for OFDM 6M/9M, 12M/18M, 24M/36M, 48M/54M */
    is_empty = RT28xx_EEPROM_READ16(pAd, TX_PWR_G_BAND_OFDM_6_9M, value);
    if (is_empty) {
        cap->tx_pwr_g_band_ofdm_6_9 = 0;
        cap->tx_pwr_g_band_ofdm_12_18 = 0;
    } else {
        /* OFDM 6M/9M */
        if (value & TX_PWR_G_BAND_OFDM_6_9M_EN) {
            if (value & TX_PWR_G_BAND_OFDM_6_9M_SIGN) {
                cap->tx_pwr_g_band_ofdm_6_9 = (value & TX_PWR_G_BAND_OFDM_6_9M_MASK);
            } else {
                cap->tx_pwr_g_band_ofdm_6_9 = -(value & TX_PWR_G_BAND_OFDM_6_9M_MASK);
            }
        } else {
            cap->tx_pwr_g_band_ofdm_6_9 = 0;
        }

        /* OFDM 12M/18M */
        if (value & TX_PWR_G_BAND_OFDM_12_18M_EN) {
            if (value & TX_PWR_G_BAND_OFDM_12_18M_SIGN) {
                cap->tx_pwr_g_band_ofdm_12_18 = ((value & TX_PWR_G_BAND_OFDM_12_18M_MASK) >> 8);
            } else {
                cap->tx_pwr_g_band_ofdm_12_18 = -((value & TX_PWR_G_BAND_OFDM_12_18M_MASK) >> 8);
            }
        } else {
            cap->tx_pwr_g_band_ofdm_12_18 = 0;
        }
    }
    MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("tx_pwr_g_band_ofdm_6_9 = %d\n", cap->tx_pwr_g_band_ofdm_6_9));
    MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("tx_pwr_g_band_ofdm_12_18 = %d\n", cap->tx_pwr_g_band_ofdm_12_18));

    is_empty = RT28xx_EEPROM_READ16(pAd, TX_PWR_G_BAND_OFDM_24_36M, value);
    if (is_empty) {
        cap->tx_pwr_g_band_ofdm_24_36 = 0;
        cap->tx_pwr_g_band_ofdm_48= 0;
    } else {
        /* OFDM 24M/36M */
        if (value & TX_PWR_G_BAND_OFDM_24_36M_EN) {
            if (value & TX_PWR_G_BAND_OFDM_24_36M_SIGN) {
                cap->tx_pwr_g_band_ofdm_24_36 = (value & TX_PWR_G_BAND_OFDM_24_36M_MASK);
            } else {
                cap->tx_pwr_g_band_ofdm_24_36 = -(value & TX_PWR_G_BAND_OFDM_24_36M_MASK);
            }
        } else {
            cap->tx_pwr_g_band_ofdm_24_36 = 0;
        }

        /* OFDM 48M */
        if (value & TX_PWR_G_BAND_OFDM_48M_EN) {
            if (value & TX_PWR_G_BAND_OFDM_48M_SIGN) {
                cap->tx_pwr_g_band_ofdm_48 = ((value & TX_PWR_G_BAND_OFDM_48M_MASK) >> 8);
            } else {
                cap->tx_pwr_g_band_ofdm_48 = -((value & TX_PWR_G_BAND_OFDM_48M_MASK) >> 8);
            }
        } else {
            cap->tx_pwr_g_band_ofdm_48 = 0;
        }
    }
    MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("tx_pwr_g_band_ofdm_24_36 = %d\n", cap->tx_pwr_g_band_ofdm_24_36));
    MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("tx_pwr_g_band_ofdm_48 = %d\n", cap->tx_pwr_g_band_ofdm_48));

    is_empty = RT28xx_EEPROM_READ16(pAd, TX_PWR_G_BAND_OFDM_54M, value);
    if (is_empty) {
        cap->tx_pwr_g_band_ofdm_54 = 0;
        cap->tx_pwr_ht_bpsk_mcs_0_8 = 0;
    } else {
        /* OFDM 54M */
        if (value & TX_PWR_G_BAND_OFDM_54M_EN) {
            if (value & TX_PWR_G_BAND_OFDM_54M_SIGN) {
                cap->tx_pwr_g_band_ofdm_54 = (value & TX_PWR_G_BAND_OFDM_54M_MASK);
            } else {
                cap->tx_pwr_g_band_ofdm_54 = -(value & TX_PWR_G_BAND_OFDM_54M_MASK);
            }
        } else {
            cap->tx_pwr_g_band_ofdm_54 = 0;
        }

        /* HT MCS_0, MCS_8 */
        if (value & TX_PWR_HT_BPSK_MCS_0_8_EN) {
            if (value & TX_PWR_HT_BPSK_MCS_0_8_SIGN) {
                cap->tx_pwr_ht_bpsk_mcs_0_8 = ((value & TX_PWR_HT_BPSK_MCS_0_8_MASK) >> 8);
            } else {
                cap->tx_pwr_ht_bpsk_mcs_0_8 = -((value & TX_PWR_HT_BPSK_MCS_0_8_MASK) >> 8);
            }
        } else {
            cap->tx_pwr_ht_bpsk_mcs_0_8 = 0;
        }    }
    MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("tx_pwr_g_band_ofdm_54 = %d\n", cap->tx_pwr_g_band_ofdm_54));
    MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("tx_pwr_ht_bpsk_mcs_0_8 = %d\n", cap->tx_pwr_ht_bpsk_mcs_0_8));

    is_empty = RT28xx_EEPROM_READ16(pAd, TX_PWR_HT_BPSK_MCS_32, value);
    if (is_empty) {
        cap->tx_pwr_ht_bpsk_mcs_32 = 0;
        cap->tx_pwr_ht_qpsk_mcs_1_2_9_10 = 0;
    } else {
        /* HT MCS_0, MCS_8 */
        if (value & TX_PWR_HT_BPSK_MCS_32_EN) {
            if (value & TX_PWR_HT_BPSK_MCS_32_SIGN) {
                cap->tx_pwr_ht_bpsk_mcs_32 = (value & TX_PWR_HT_BPSK_MCS_32_MASK);
            } else {
                cap->tx_pwr_ht_bpsk_mcs_32 = -(value & TX_PWR_HT_BPSK_MCS_32_MASK);
            }
        } else {
            cap->tx_pwr_ht_bpsk_mcs_32 = 0;
        }

        /* HT MCS_1, MCS_2, MCS_9, MCS_10 */
        if (value & TX_PWR_HT_QPSK_MCS_1_2_9_10_EN) {
            if (value & TX_PWR_HT_QPSK_MCS_1_2_9_10_SIGN) {
                cap->tx_pwr_ht_qpsk_mcs_1_2_9_10 = ((value & TX_PWR_HT_QPSK_MCS_1_2_9_10_MASK) >> 8);
            } else {
                cap->tx_pwr_ht_qpsk_mcs_1_2_9_10 = -((value & TX_PWR_HT_QPSK_MCS_1_2_9_10_MASK) >> 8);
            }
        } else {
            cap->tx_pwr_ht_qpsk_mcs_1_2_9_10 = 0;
        }
    }

    MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("tx_pwr_ht_bpsk_mcs_32 = %d\n", cap->tx_pwr_ht_bpsk_mcs_32));
    MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("tx_pwr_ht_qpsk_mcs_1_2_9_10 = %d\n", cap->tx_pwr_ht_qpsk_mcs_1_2_9_10));

    is_empty = RT28xx_EEPROM_READ16(pAd, TX_PWR_HT_16QAM_MCS_3_4_11_12, value);
    if (is_empty) {
        cap->tx_pwr_ht_16qam_mcs_3_4_11_12 = 0;
        cap->tx_pwr_ht_64qam_mcs_5_13 = 0;
    } else {
        /* HT MCS_3, MCS_4, MCS_11, MCS_12 */
        if (value & TX_PWR_HT_16QAM_MCS_3_4_11_12_EN) {
            if (value & TX_PWR_HT_16QAM_MCS_3_4_11_12_SIGN) {
                cap->tx_pwr_ht_16qam_mcs_3_4_11_12 = (value & TX_PWR_HT_16QAM_MCS_3_4_11_12_MASK);
            } else {
                cap->tx_pwr_ht_16qam_mcs_3_4_11_12 = -(value & TX_PWR_HT_16QAM_MCS_3_4_11_12_MASK);
            }
        } else {
            cap->tx_pwr_ht_16qam_mcs_3_4_11_12 = 0;
        }

        /* HT MCS_5, MCS_13 */
        if (value & TX_PWR_HT_64QAM_MCS_5_13_EN) {
            if (value & TX_PWR_HT_64QAM_MCS_5_13_SIGN) {
                cap->tx_pwr_ht_64qam_mcs_5_13 = ((value & TX_PWR_HT_64QAM_MCS_5_13_MASK) >> 8);
            } else {
                cap->tx_pwr_ht_64qam_mcs_5_13 = -((value & TX_PWR_HT_64QAM_MCS_5_13_MASK) >> 8);
            }
        } else {
            cap->tx_pwr_ht_64qam_mcs_5_13 = 0;
        }
    }
    MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("tx_pwr_ht_16qam_mcs_3_4_11_12 = %d\n", cap->tx_pwr_ht_16qam_mcs_3_4_11_12));
    MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("tx_pwr_ht_64qam_mcs_5_13 = %d\n", cap->tx_pwr_ht_64qam_mcs_5_13));

    is_empty = RT28xx_EEPROM_READ16(pAd, TX_PWR_HT_64QAM_MCS_6_14, value);
    if (is_empty) {
        cap->tx_pwr_ht_64qam_mcs_6_14 = 0;
        cap->tx_pwr_ht_64qam_mcs_7_15 = 0;
    } else {
        /* HT MCS_6, MCS_14 */
        if (value & TX_PWR_HT_64QAM_MCS_6_14_EN) {
            if (value & TX_PWR_HT_64QAM_MCS_6_14_SIGN) {
                cap->tx_pwr_ht_64qam_mcs_6_14 = (value & TX_PWR_HT_64QAM_MCS_6_14_MASK);
            } else {
                cap->tx_pwr_ht_64qam_mcs_6_14 = -(value & TX_PWR_HT_64QAM_MCS_6_14_MASK);
            }
        } else {
            cap->tx_pwr_ht_64qam_mcs_6_14 = 0;
        }

        /* HT MCS_7, MCS_15 */
        if (value & TX_PWR_HT_64QAM_MCS_7_15_EN) {
            if (value & TX_PWR_HT_64QAM_MCS_7_15_SIGN) {
                cap->tx_pwr_ht_64qam_mcs_7_15 = ((value & TX_PWR_HT_64QAM_MCS_7_15_MASK) >> 8);
            } else {
                cap->tx_pwr_ht_64qam_mcs_7_15 = -((value & TX_PWR_HT_64QAM_MCS_7_15_MASK) >> 8);
            }
        } else {
            cap->tx_pwr_ht_64qam_mcs_7_15 = 0;
        }
    }
    MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("tx_pwr_ht_64qam_mcs_6_14 = %d\n", cap->tx_pwr_ht_64qam_mcs_6_14));
    MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("tx_pwr_ht_64qam_mcs_7_15 = %d\n", cap->tx_pwr_ht_64qam_mcs_7_15));

	return;
}


void mt7636_get_tx_pwr_info(RTMP_ADAPTER *pAd)
{
    bool is_empty = 0;
    UINT16 value = 0;
	struct MT_TX_PWR_CAP *cap = &pAd->chipCap.MTTxPwrCap;

    /* Read 20/40 BW delta */
    is_empty = RT28xx_EEPROM_READ16(pAd, G_BAND_20_40_BW_PWR_DELTA, value);

	if (is_empty) {
        cap->delta_tx_pwr_bw40_g_band = 0x0;
    } else {
        /* G Band */
        if (value & G_BAND_20_40_BW_PWR_DELTA_EN) {
            if (value & G_BAND_20_40_BW_PWR_DELTA_SIGN) {
                /* bit[0..5] tx power delta value */
                cap->delta_tx_pwr_bw40_g_band = (value & G_BAND_20_40_BW_PWR_DELTA_MASK);
            } else {
                cap->delta_tx_pwr_bw40_g_band = -(value & G_BAND_20_40_BW_PWR_DELTA_MASK);
            }
        } else {
            cap->delta_tx_pwr_bw40_g_band = 0x0;
        }
    }
    MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("delta_tx_pwr_bw40_g_band = %d\n", cap->delta_tx_pwr_bw40_g_band));

    /////////////////// Tx0 //////////////////////////
    /* Read TSSI slope/offset for TSSI compensation */
    is_empty = RT28xx_EEPROM_READ16(pAd, TX0_G_BAND_TSSI_SLOPE, value);

    cap->tssi_0_slope_g_band =
        (is_empty) ? TSSI_0_SLOPE_G_BAND_DEFAULT_VALUE : (value & TX0_G_BAND_TSSI_SLOPE_MASK);

    cap->tssi_0_offset_g_band =
        (is_empty) ? TSSI_0_OFFSET_G_BAND_DEFAULT_VALUE : ((value & TX0_G_BAND_TSSI_OFFSET_MASK) >> 8);

    MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("tssi_0_slope_g_band = %d\n", cap->tssi_0_slope_g_band));
    MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("tssi_0_offset_g_band = %d\n", cap->tssi_0_offset_g_band));
    /* Read 54M target power */
    is_empty = RT28xx_EEPROM_READ16(pAd, TX0_G_BAND_TARGET_PWR, value);

    cap->tx_0_target_pwr_g_band =
        (is_empty) ? TX_TARGET_PWR_DEFAULT_VALUE : (value & TX0_G_BAND_TARGET_PWR_MASK);

    MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("tssi_0_target_pwr_g_band = %d\n", cap->tx_0_target_pwr_g_band));

    /* Read power offset (channel delta) */
    if (is_empty) {
        cap->tx_0_chl_pwr_delta_g_band[G_BAND_LOW] = 0x0;
    } else {
        /* tx power offset LOW */
        if (value & TX0_G_BAND_CHL_PWR_DELTA_LOW_EN) {
            if (value & TX0_G_BAND_CHL_PWR_DELTA_LOW_SIGN) {
                cap->tx_0_chl_pwr_delta_g_band[G_BAND_LOW] = ((value & TX0_G_BAND_CHL_PWR_DELTA_LOW_MASK) >> 8);
            } else {
                cap->tx_0_chl_pwr_delta_g_band[G_BAND_LOW] = -((value & TX0_G_BAND_CHL_PWR_DELTA_LOW_MASK) >> 8);
            }
        } else {
            cap->tx_0_chl_pwr_delta_g_band[G_BAND_LOW] = 0x0;
        }
    }
    MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("tx_0_chl_pwr_delta_g_band[G_BAND_LOW] = %d\n", cap->tx_0_chl_pwr_delta_g_band[G_BAND_LOW]));

    is_empty = RT28xx_EEPROM_READ16(pAd, TX0_G_BAND_CHL_PWR_DELTA_MID, value);

    if (is_empty) {
        cap->tx_0_chl_pwr_delta_g_band[G_BAND_MID] = 0x0;
        cap->tx_0_chl_pwr_delta_g_band[G_BAND_HI] = 0x0;
    } else {
        /* tx power offset MID */
        if (value & TX0_G_BAND_CHL_PWR_DELTA_MID_EN) {
            if (value & TX0_G_BAND_CHL_PWR_DELTA_MID_SIGN)
                cap->tx_0_chl_pwr_delta_g_band[G_BAND_MID] = (value & TX0_G_BAND_CHL_PWR_DELTA_MID_MASK);
            else
                cap->tx_0_chl_pwr_delta_g_band[G_BAND_MID] = -(value & TX0_G_BAND_CHL_PWR_DELTA_MID_MASK);
        } else {
            cap->tx_0_chl_pwr_delta_g_band[G_BAND_MID] = 0x0;
        }
        /* tx power offset HIGH */
        if (value & TX0_G_BAND_CHL_PWR_DELTA_HI_EN) {
            if (value & TX0_G_BAND_CHL_PWR_DELTA_HI_SIGN)
                cap->tx_0_chl_pwr_delta_g_band[G_BAND_HI] = ((value & TX0_G_BAND_CHL_PWR_DELTA_HI_MASK) >> 8);
            else
                cap->tx_0_chl_pwr_delta_g_band[G_BAND_HI] = -((value & TX0_G_BAND_CHL_PWR_DELTA_HI_MASK) >> 8);
        } else {
            cap->tx_0_chl_pwr_delta_g_band[G_BAND_HI] = 0x0;
        }
    }
    MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("tx_0_chl_pwr_delta_g_band[G_BAND_MID] = %d\n", cap->tx_0_chl_pwr_delta_g_band[G_BAND_MID]));
    MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("tx_0_chl_pwr_delta_g_band[G_BAND_HI] = %d\n", cap->tx_0_chl_pwr_delta_g_band[G_BAND_HI]));

    /////////////////// Tx1 //////////////////////////
    /* Read TSSI slope/offset for TSSI compensation */
    is_empty = RT28xx_EEPROM_READ16(pAd, TX1_G_BAND_TSSI_SLOPE, value);

    cap->tssi_1_slope_g_band = (is_empty) ? TSSI_1_SLOPE_G_BAND_DEFAULT_VALUE : (value & TX1_G_BAND_TSSI_SLOPE_MASK);

    cap->tssi_1_offset_g_band = (is_empty) ? TSSI_1_OFFSET_G_BAND_DEFAULT_VALUE : ((value & TX1_G_BAND_TSSI_OFFSET_MASK) >> 8);

    MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("tssi_1_slope_g_band = %d\n", cap->tssi_1_slope_g_band));
    MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("tssi_1_offset_g_band = %d\n", cap->tssi_1_offset_g_band));

    /* Read 54M target power */
    is_empty = RT28xx_EEPROM_READ16(pAd, TX1_G_BAND_TARGET_PWR, value);

    cap->tx_1_target_pwr_g_band = (is_empty) ? TX_TARGET_PWR_DEFAULT_VALUE : (value & TX1_G_BAND_TARGET_PWR_MASK);

    printk ("tssi_1_target_pwr_g_band = %d\n", cap->tx_1_target_pwr_g_band);

    /* Read power offset (channel delta) */
    if (is_empty) {
        cap->tx_1_chl_pwr_delta_g_band[G_BAND_LOW] =  0;
    } else {
        /* tx power offset LOW */
        if (value & TX1_G_BAND_CHL_PWR_DELTA_LOW_EN) {
            if (value & TX1_G_BAND_CHL_PWR_DELTA_LOW_SIGN) {
                cap->tx_1_chl_pwr_delta_g_band[G_BAND_LOW] = ((value & TX1_G_BAND_CHL_PWR_DELTA_LOW_MASK) >> 8);
            } else {
                cap->tx_1_chl_pwr_delta_g_band[G_BAND_LOW] = -((value & TX1_G_BAND_CHL_PWR_DELTA_LOW_MASK) >> 8);
            }
        } else {
            cap->tx_1_chl_pwr_delta_g_band[G_BAND_LOW] = 0;
        }
    }
    MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("tx_1_chl_pwr_delta_g_band[G_BAND_LOW] = %d\n", cap->tx_1_chl_pwr_delta_g_band[G_BAND_LOW]));

    is_empty = RT28xx_EEPROM_READ16(pAd, TX1_G_BAND_CHL_PWR_DELTA_MID, value);
    if (is_empty) {
        cap->tx_1_chl_pwr_delta_g_band[G_BAND_MID] = 0;
        cap->tx_1_chl_pwr_delta_g_band[G_BAND_HI] = 0;
    } else {
        /* tx power offset MID */
        if (value & TX1_G_BAND_CHL_PWR_DELTA_MID_EN) {
            if (value & TX1_G_BAND_CHL_PWR_DELTA_MID_SIGN) {
                cap->tx_1_chl_pwr_delta_g_band[G_BAND_MID] = (value & TX1_G_BAND_CHL_PWR_DELTA_MID_MASK);
            } else {
                cap->tx_1_chl_pwr_delta_g_band[G_BAND_MID] = -(value & TX1_G_BAND_CHL_PWR_DELTA_MID_MASK);
            }
        } else {
            cap->tx_1_chl_pwr_delta_g_band[G_BAND_MID] = 0;
        }
        /* tx power offset HIGH */
        if (value & TX1_G_BAND_CHL_PWR_DELTA_HI_EN) {
            if (value & TX1_G_BAND_CHL_PWR_DELTA_HI_SIGN) {
                cap->tx_1_chl_pwr_delta_g_band[G_BAND_HI] = ((value & TX1_G_BAND_CHL_PWR_DELTA_HI_MASK) >> 8);
            } else {
                cap->tx_1_chl_pwr_delta_g_band[G_BAND_HI] = -((value & TX1_G_BAND_CHL_PWR_DELTA_HI_MASK) >> 8);
            }
        } else {
            cap->tx_1_chl_pwr_delta_g_band[G_BAND_HI] = 0;
        }
    }
    MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("tx_1_chl_pwr_delta_g_band[G_BAND_MID] = %d\n", cap->tx_1_chl_pwr_delta_g_band[G_BAND_MID]));
    MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("tx_1_chl_pwr_delta_g_band[G_BAND_HI] = %d\n", cap->tx_1_chl_pwr_delta_g_band[G_BAND_HI]));

    return ;
}

static UINT8 mt7636_txpwr_chlist[] = {
	1, 2,3,4,5,6,7,8,9,10,11,12,13,14,
	36,38,40,44,46,48,52,54,56,60,62,64,
	100,102,104,108,110,112,116,118,120,124,126,128,132,134,136,140,
	149,151,153,157,159,161,165,167,169,171,173,
};

UINT32 mt7636_read_chl_pwr(RTMP_ADAPTER *ad)
{
	UINT32 i, choffset;

	/* Read Tx power value for all channels*/
	/* Value from 1 - 0x7f. Default value is 24.*/
	/* Power value : 2.4G 0x00 (0) ~ 0x1F (31)*/
	/*             : 5.5G 0xF9 (-7) ~ 0x0F (15)*/

	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("%s()--->\n", __FUNCTION__));

	for (i = 0; i < sizeof(mt7636_txpwr_chlist); i++) {
		ad->TxPower[i].Channel = mt7636_txpwr_chlist[i];
		ad->TxPower[i].Power = TX_TARGET_PWR_DEFAULT_VALUE;
		ad->TxPower[i].Power2 = TX_TARGET_PWR_DEFAULT_VALUE;
	}

	/* 1. U-NII lower/middle band: 36, 38, 40; 44, 46, 48; 52, 54, 56; 60, 62, 64 (including central frequency in BW 40MHz)*/
	choffset = 14;
	ASSERT((ad->TxPower[choffset].Channel == 36));

	choffset = 14 + 12 + 16 + 11;

	/* 4. Print and Debug*/
	for (i = 0; i < choffset; i++)
	{
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("Channel=%d, Power[Tx0:%d, Tx1:%d]\n",
				ad->TxPower[i].Channel, ad->TxPower[i].Power, ad->TxPower[i].Power2 ));
	}

	return TRUE;
}


static VOID mt7636_show_pwr_info(RTMP_ADAPTER *pAd)
{
	struct MT_TX_PWR_CAP *cap = &pAd->chipCap.MTTxPwrCap;
	UINT32 value;

	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\n===================================\n"));
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("channel info related to power\n"));
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("===================================\n"));

	if (pAd->LatchRfRegs.Channel < 14) {
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("central channel = %d, low_mid_hi = %d\n", pAd->LatchRfRegs.Channel,
							get_low_mid_hi_index(pAd->LatchRfRegs.Channel)));
	}

	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\n===================================\n"));
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("channel power(unit: 0.5dbm)\n"));
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("===================================\n"));
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("tx_0_target_pwr_g_band = 0x%x\n", cap->tx_0_target_pwr_g_band));
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("tx_1_target_pwr_g_band = 0x%x\n", cap->tx_1_target_pwr_g_band));

	/* channel power delta */
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\n===================================\n"));
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("channel power delta(unit: 0.5db)\n"));
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("===================================\n"));
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("tx_0_chl_pwr_delta_g_band[G_BAND_LOW] = 0x%x\n", cap->tx_0_chl_pwr_delta_g_band[G_BAND_LOW]));
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("tx_0_chl_pwr_delta_g_band[G_BAND_MID] = 0x%x\n", cap->tx_0_chl_pwr_delta_g_band[G_BAND_MID]));
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("tx_0_chl_pwr_delta_g_band[G_BAND_HI] = 0x%x\n", cap->tx_0_chl_pwr_delta_g_band[G_BAND_HI]));
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("tx_1_chl_pwr_delta_g_band[G_BAND_LOW] = 0x%x\n", cap->tx_1_chl_pwr_delta_g_band[G_BAND_LOW]));
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("tx_1_chl_pwr_delta_g_band[G_BAND_MID] = 0x%x\n", cap->tx_1_chl_pwr_delta_g_band[G_BAND_MID]));
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("tx_1_chl_pwr_delta_g_band[G_BAND_HI] = 0x%x\n", cap->tx_1_chl_pwr_delta_g_band[G_BAND_HI]));

	/* bw power delta */
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\n===================================\n"));
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("bw power delta(unit: 0.5db)\n"));
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("===================================\n"));
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("delta_tx_pwr_bw40_g_band = %d\n", cap->delta_tx_pwr_bw40_g_band));

	/* per-rate power delta */
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\n===================================\n"));
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("per-rate power delta\n"));
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("===================================\n"));
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("tx_pwr_cck_1_2 = %d\n", cap->tx_pwr_cck_1_2));
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("tx_pwr_cck_5_11 = %d\n", cap->tx_pwr_cck_5_11));
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("tx_pwr_g_band_ofdm_6_9 = %d\n", cap->tx_pwr_g_band_ofdm_6_9));
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("tx_pwr_g_band_ofdm_12_18 = %d\n", cap->tx_pwr_g_band_ofdm_12_18));
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("tx_pwr_g_band_ofdm_24_36 = %d\n", cap->tx_pwr_g_band_ofdm_24_36));
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("tx_pwr_g_band_ofdm_48 = %d\n", cap->tx_pwr_g_band_ofdm_48));
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("tx_pwr_g_band_ofdm_54 = %d\n", cap->tx_pwr_g_band_ofdm_54));
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("tx_pwr_ht_bpsk_mcs_32 = %d\n", cap->tx_pwr_ht_bpsk_mcs_32));
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("tx_pwr_ht_bpsk_mcs_0_8 = %d\n", cap->tx_pwr_ht_bpsk_mcs_0_8));
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("tx_pwr_ht_qpsk_mcs_1_2_9_10 = %d\n", cap->tx_pwr_ht_qpsk_mcs_1_2_9_10));
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("tx_pwr_ht_16qam_mcs_3_4_11_12 = %d\n", cap->tx_pwr_ht_16qam_mcs_3_4_11_12));
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("tx_pwr_ht_64qam_mcs_5_13 = %d\n", cap->tx_pwr_ht_64qam_mcs_5_13));
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("tx_pwr_ht_64qam_mcs_6_14 = %d\n", cap->tx_pwr_ht_64qam_mcs_6_14));
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("tx_pwr_ht_64qam_mcs_7_15 = %d\n", cap->tx_pwr_ht_64qam_mcs_7_15));

	/* TMAC POWER INFO */
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\n===================================\n"));
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("per-rate power delta in MAC 0x60130020 ~ 0x60130030\n"));
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("===================================\n"));
	RTMP_IO_READ32(pAd, TMAC_FP0R0, &value);
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("TMAC_FP0R0 = 0x%x\n", value));
	RTMP_IO_READ32(pAd, TMAC_FP0R1, &value);
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("TMAC_FP0R1 = 0x%x\n", value));
	RTMP_IO_READ32(pAd, TMAC_FP0R2, &value);
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("TMAC_FP0R2 = 0x%x\n", value));
	RTMP_IO_READ32(pAd, TMAC_FP0R3, &value);
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("TMAC_FP0R3 = 0x%x\n", value));
	RTMP_IO_READ32(pAd, TMAC_FP0R4, &value);
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("TMAC_FP0R4 = 0x%x\n", value));

	/* TSSI info */
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\n===================================\n"));
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("TSSI info\n"));
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("===================================\n"));
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("TSSI enable = %d\n", pAd->chipCap.tssi_enable));
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("tssi_0_slope_g_band = 0x%x\n", cap->tssi_0_slope_g_band));
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("tssi_1_slope_g_band = 0x%x\n", cap->tssi_1_slope_g_band));
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("tssi_0_offset_g_band = 0x%x\n", cap->tssi_0_offset_g_band));
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("tssi_1_offset_g_band = 0x%x\n", cap->tssi_1_offset_g_band));
}

static inline VOID bufferModeFieldSet(RTMP_ADAPTER *pAd,EXT_CMD_EFUSE_BUFFER_MODE_T *pCmd,UINT16 addr)
{
	UINT32 i = pCmd->ucCount;
	pCmd->aBinContent[i].u2Addr = cpu2le16(addr);
	pCmd->aBinContent[i].ucValue = pAd->EEPROMImage[addr] ;
	pCmd->ucCount++;
}


static VOID mt7636_bufferModeEfuseFill(RTMP_ADAPTER *pAd,EXT_CMD_EFUSE_BUFFER_MODE_T *pCmd)
{
	UINT16 i=0;
	pCmd->ucCount = 0;

	for(i=0x20; i<=0xff;i++)
	{
		bufferModeFieldSet(pAd,pCmd,i);
	}
	/*must minus last add*/
	pCmd->ucCount --;
}



static void mt7636_antenna_default_reset(
	struct _RTMP_ADAPTER *pAd,
	EEPROM_ANTENNA_STRUC *pAntenna)
{
#if defined(MT7636_FPGA)
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("%s() todo \n", __FUNCTION__));
	return;
#endif /* MT7636_FPGA */

	pAntenna->word = 0;
	pAd->RfIcType = RFIC_7636;
	pAntenna->field.TxPath = 2;
	pAntenna->field.RxPath = 2;
}

#ifdef CONFIG_STA_SUPPORT
static VOID mt7636_init_dev_nick_name(RTMP_ADAPTER *ad)
{

#ifdef RTMP_MAC_USB
	if (IS_MT76x6(ad))
		snprintf((RTMP_STRING *) ad->nickname, sizeof(ad->nickname), "mt7636u_sta");
#endif

#ifdef RTMP_MAC_SDIO
if (IS_MT76x6(ad))
		snprintf((RTMP_STRING *) ad->nickname, sizeof(ad->nickname), "mt7636s_sta");
#endif
}
#endif /* CONFIG_STA_SUPPORT */
static const RTMP_CHIP_CAP MT7636_ChipCap = {
	.max_nss = 2,
	.TXWISize = sizeof(TMAC_TXD_L), /* 32 */
	.RXWISize = 28,
	.WtblHwNum = MT7636_MT_WTBL_SIZE,
	.WtblPseAddr = MT7636_PSE_WTBL_2_ADDR,

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
	.phy_caps = (fPHY_CAP_24G | fPHY_CAP_5G | fPHY_CAP_HT),
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
	.EEPROM_DEFAULT_BIN = MT7636_E2PImage,
	.EEPROM_DEFAULT_BIN_SIZE = sizeof(MT7636_E2PImage),
#ifdef CONFIG_ANDES_SUPPORT
	.CmdRspRxRing = RX_RING1,
	.need_load_fw = TRUE,
	.DownLoadType = DownLoadTypeB,
#ifdef WCX_SUPPORT
	.load_code_method = BIN_FILE_METHOD,
#else
	/*.load_code_method = BIN_FILE_METHOD,*/
	.load_code_method = HEADER_METHOD,
#endif /* WCX_SUPPORT */
	.rom_patch_header_image = mt7636_rom_patch,
	.rom_patch_bin_file_name = ROM_PATCH_BIN_FILE_NAME,
	.rom_patch_len = sizeof(mt7636_rom_patch),
	.need_load_rom_patch = TRUE,
	.ram_code_protect = FALSE,
	.rom_code_protect = TRUE,
	.ilm_offset = 0x80000,
	.dlm_offset = 0x110000,
	.rom_patch_offset = 0x8C000,
#endif
	.MCUType = ANDES,
	.cmd_header_len = sizeof(FW_TXD),
#ifdef RTMP_PCI_SUPPORT
	.cmd_padding_len = 0,
#endif

#ifdef RTMP_SDIO_SUPPORT
	.cmd_padding_len = 4,
#endif

#ifdef RTMP_USB_SUPPORT
	.cmd_padding_len = 4,
	.CommandBulkOutAddr = 0x8,
	.WMM0ACBulkOutAddr[0] = 0x4,
	.WMM0ACBulkOutAddr[1] = 0x5,
	.WMM0ACBulkOutAddr[2] = 0x6,
	.WMM0ACBulkOutAddr[3] = 0x7,
	.WMM1ACBulkOutAddr	= 0x9,
	.DataBulkInAddr = 0x84,
	.CommandRspBulkInAddr = 0x85,
#endif
	.fw_header_image = MT7636_FirmwareImage,
	.fw_bin_file_name = FW_BIN_FILE_NAME,
	.fw_len = sizeof(MT7636_FirmwareImage),
#ifdef CARRIER_DETECTION_SUPPORT
	.carrier_func = TONE_RADAR_V2,
#endif
#ifdef CONFIG_WIFI_TEST
	.MemMapStart = 0x0000,
	.MemMapEnd = 0xffff,
	.MacMemMapOffset = 0x1000,
	.MacStart = 0x0000,
	.MacEnd = 0x0600,
	.BBPMemMapOffset = 0x2000,
	.BBPStart = 0x0000,
	.BBPEnd = 0x0f00,
	.RFIndexNum = 0,
	.RFIndexOffset = 0,
	.E2PStart = 0x0000,
	.E2PEnd = 0x00fe,
#endif /* CONFIG_WIFI_TEST */
	.hif_type = HIF_MT,
	.rf_type = RF_MT,
#ifdef RTMP_SDIO_SUPPORT
	.RxBAWinSize = 21,
#endif

#ifdef RTMP_USB_SUPPORT
	.RxBAWinSize = 64,
#endif
	.AMPDUFactor = 3,
#ifdef RACTRL_FW_OFFLOAD_SUPPORT
	.fgRateAdaptFWOffload = TRUE,
#endif /* RACTRL_FW_OFFLOAD_SUPPORT */
    .TxOPScenario = 0
};


static const RTMP_CHIP_OP MT7636_ChipOp = {
	.ChipBBPAdjust = mt7636_bbp_adjust,
	.ChipSwitchChannel = mt7636_switch_channel,
	.AsicMacInit = mt7636_init_mac_cr,
	.AsicAntennaDefaultReset = mt7636_antenna_default_reset,
	.ChipAGCInit = NULL,
#ifdef CONFIG_STA_SUPPORT
	.ChipAGCAdjust = NULL,
#endif
	.AsicRfTurnOn = NULL,
	.AsicHaltAction = NULL,
	.AsicRfTurnOff = NULL,
	.AsicReverseRfFromSleepMode = NULL,
#ifdef CONFIG_STA_SUPPORT
	.NetDevNickNameInit = mt7636_init_dev_nick_name,
#endif
#ifdef CARRIER_DETECTION_SUPPORT
	.ToneRadarProgram = ToneRadarProgram_v2,
#endif
	.RxSensitivityTuning = NULL,
	.DisableTxRx = NULL,
#ifdef RTMP_USB_SUPPORT
	.AsicRadioOn = NULL,
	.AsicRadioOff = NULL,
	.usb_cfg_read = mtusb_cfg_read,
	.usb_cfg_write = mtusb_cfg_write,
#endif
#ifdef RTMP_PCI_SUPPORT
	//.AsicRadioOn = RT28xxPciAsicRadioOn,
	//.AsicRadioOff = RT28xxPciAsicRadioOff,
#endif
	.show_pwr_info = mt7636_show_pwr_info,
	.bufferModeEfuseFill =mt7636_bufferModeEfuseFill,

};

VOID mt7636_init(RTMP_ADAPTER *pAd)
{
	RTMP_CHIP_CAP *pChipCap = &pAd->chipCap;

	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("%s()-->\n", __FUNCTION__));

	memcpy(&pAd->chipCap, &MT7636_ChipCap, sizeof(RTMP_CHIP_CAP));
	memcpy(&pAd->chipOps, &MT7636_ChipOp, sizeof(RTMP_CHIP_OP));

	pAd->chipCap.hif_type = HIF_MT;
	pAd->chipCap.mac_type = MAC_MT;

	mt_phy_probe(pAd);

	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("%s(FW(%x), HW(%x), CHIPID(%x))\n",
							__FUNCTION__,
							pAd->FWVersion,
							pAd->HWVersion,
							pAd->ChipID));

	if (((pAd->FWVersion & 0x0000ffff) == 0x8A00) && IS_MT76x6(pAd))
	{
		/* ROM Patch*/
		pChipCap->rom_patch_header_image = mt7636_rom_patch;
		pChipCap->rom_patch_bin_file_name = ROM_PATCH_BIN_FILE_NAME;
		pChipCap->rom_patch_len = sizeof(mt7636_rom_patch);

		/* RAM Code */
		pChipCap->fw_header_image = MT7636_FirmwareImage;
		pChipCap->fw_bin_file_name = FW_BIN_FILE_NAME;
		pChipCap->fw_len = sizeof(MT7636_FirmwareImage);
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("%s(%d)::(1), pChipCap->fw_len(%d), pChipCap->rom_patch_len(%d)\n", __FUNCTION__, __LINE__, pChipCap->fw_len, pChipCap->rom_patch_len));
	}
	else
	{
		/* ROM Patch*/
		pChipCap->rom_patch_header_image = mt7636_rom_patch;
		pChipCap->rom_patch_bin_file_name = ROM_PATCH_BIN_FILE_NAME;
		pChipCap->rom_patch_len = sizeof(mt7636_rom_patch);

		/* RAM Code */
		pChipCap->fw_header_image = MT7636_FirmwareImage;
		pChipCap->fw_bin_file_name = FW_BIN_FILE_NAME;
		pChipCap->fw_len = sizeof(MT7636_FirmwareImage);
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("%s(%d)::(2), pChipCap->fw_len(%d), pChipCap->rom_patch_len(%d)\n", __FUNCTION__, __LINE__, pChipCap->fw_len, pChipCap->rom_patch_len));
	}

#ifdef RTMP_MAC_PCI
	if (IS_PCI_INF(pAd)) {
		pChipCap->tx_hw_hdr_len = pChipCap->TXWISize;
		pChipCap->rx_hw_hdr_len = pChipCap->RXWISize;
	}
#endif /* RTMP_MAC_PCI */

#ifdef RTMP_MAC_USB
	if (IS_USB_INF(pAd)) {
		pChipCap->tx_hw_hdr_len = pChipCap->TXWISize;
		pChipCap->rx_hw_hdr_len = pChipCap->RXWISize;
	}
#endif /* RTMP_MAC_USB */

#ifdef RTMP_MAC_SDIO
	if (IS_SDIO_INF(pAd)) {
		pChipCap->tx_hw_hdr_len = pChipCap->TXWISize;
		pChipCap->rx_hw_hdr_len = pChipCap->RXWISize;
	}
#endif /* RTMP_MAC_PCI */

#ifdef CONFIG_CSO_SUPPORT
	pChipCap->asic_caps |= fASIC_CAP_CSO,
#endif

	/* For different capability */
	if (IS_MT7636(pAd))
	{
	}
	else if (IS_MT7606(pAd))
	{
	}
	else if (IS_MT7611(pAd))
	{
	}

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

	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("<--%s()\n", __FUNCTION__));
}

#ifdef MT7636_BTCOEX
INT Set_MT7636Coex_Proc(
	IN RTMP_ADAPTER		*pAd,
	IN RTMP_STRING		*arg)
{
	printk("--->Set_MT7636Coex_Proc\n");
	UINT8 status = (UINT8)simple_strtol(arg, 0, 10);
	/*
		0x2300[5] Default Antenna:
		0 for WIFI main antenna
		1  for WIFI aux  antenna

	*/

	//if (status >= 0 && status < 5 )
	{
			AndesCoexOP(pAd, status);
			MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("%s:status:0x%x\n", __FUNCTION__, status));
	}
	return TRUE;
}


INT Set_MT7636Coex_Protection_Mode_Proc(
	IN RTMP_ADAPTER		*pAd,
	IN RTMP_STRING		*arg)
{
	printk("--->Set_MT7636Coex_Protection_Mode_Proc\n");
	UINT8 mode = (UINT8)simple_strtol(arg, 0, 10);
	/*
		0x2300[5] Default Antenna:
		0 for WIFI main antenna
		1  for WIFI aux  antenna

	*/

	//if (status >= 0 && status < 5 )
	{
			pAd->BtProtectionMode = mode;
			MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("%s:mode:0x%x\n", __FUNCTION__, mode));
	}
	return TRUE;
}


INT Set_MT7636Coex_Protection_Rate_Proc(
	IN RTMP_ADAPTER		*pAd,
	IN RTMP_STRING		*arg)
{
	printk("--->Set_MT7636Coex_Protection_Rate_Proc\n");
	UINT8 rate = (UINT8)simple_strtol(arg, 0, 10);
	/*
		0x2300[5] Default Antenna:
		0 for WIFI main antenna
		1  for WIFI aux  antenna

	*/

	//if (status >= 0 && status < 5 )
	{
			pAd->BtProtectionRate= rate;
			AndesCoexProtectionFrameOP(pAd, pAd->BtProtectionMode, pAd->BtProtectionRate);
			MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("%s:mode:0x%x rate:0x%x\n", __FUNCTION__, pAd->BtProtectionMode, pAd->BtProtectionRate));
	}
	return TRUE;
}
#endif
