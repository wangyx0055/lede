#ifndef __MT7615_H__
#define __MT7615_H__

#include "mcu/andes_core.h"
#include "phy/mt_rf.h"

struct _RTMP_ADAPTER;

#define MAX_RF_ID	127
#define MAC_RF_BANK 7

void mt7615_init(struct _RTMP_ADAPTER *pAd);
void mt7615_get_tx_pwr_per_rate(struct _RTMP_ADAPTER *pAd);
void mt7615_get_tx_pwr_info(struct _RTMP_ADAPTER *pAd);
void mt7615_antenna_sel_ctl(struct _RTMP_ADAPTER *pAd);
int mt7615_read_chl_pwr(struct _RTMP_ADAPTER *pAd);
void mt7615_pwrOn(struct _RTMP_ADAPTER *pAd);
void mt7615_calibration(struct _RTMP_ADAPTER *pAd, UCHAR channel);
void mt7615_tssi_compensation(struct _RTMP_ADAPTER *pAd, UCHAR channel);

#ifdef MT7615_FPGA
INT mt7615_chk_top_default_cr_setting(struct _RTMP_ADAPTER *pAd);
INT mt7615_chk_hif_default_cr_setting(struct _RTMP_ADAPTER *pAd);
#endif /* MT7615_FPGA */

#endif // __MT7615_H__

