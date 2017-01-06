#ifndef __MT7636_H__
#define __MT7636_H__

#include "mcu/andes_core.h"
#include "phy/mt_rf.h"

struct _RTMP_ADAPTER;

#define MAX_RF_ID	127
#define MAC_RF_BANK 7

#define ROM_PATCH_BIN_FILE_NAME "mt7636_patch_e1_hdr.bin"
#define FW_BIN_FILE_NAME "WIFI_RAM_CODE_MT7636.bin"

void mt7636_init(struct _RTMP_ADAPTER *pAd);
void mt7636_get_tx_pwr_per_rate(struct _RTMP_ADAPTER *pAd);
void mt7636_get_tx_pwr_info(struct _RTMP_ADAPTER *pAd);
void mt7636_antenna_sel_ctl(struct _RTMP_ADAPTER *pAd);
UINT32 mt7636_read_chl_pwr(struct _RTMP_ADAPTER *pAd);
void mt7636_pwrOn(struct _RTMP_ADAPTER *pAd);
void mt7636_calibration(struct _RTMP_ADAPTER *pAd, UCHAR channel);
void mt7636_tssi_compensation(struct _RTMP_ADAPTER *pAd, UCHAR channel);
#ifdef MT7636_BTCOEX
INT Set_MT7636Coex_Proc(struct _RTMP_ADAPTER *pAd, RTMP_STRING *arg);
INT Set_MT7636Coex_Protection_Mode_Proc(struct _RTMP_ADAPTER *pAd, RTMP_STRING *arg);
INT Set_MT7636Coex_Protection_Rate_Proc(struct _RTMP_ADAPTER *pAd, RTMP_STRING *arg);
#endif
#endif // __MT7636_H__

