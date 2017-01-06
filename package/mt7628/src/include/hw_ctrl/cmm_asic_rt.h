/*
 ***************************************************************************
 * Ralink Tech Inc.
 * 4F, No. 2 Technology	5th Rd.
 * Science-based Industrial Park
 * Hsin-chu, Taiwan, R.O.C.
 *
 * (c) Copyright 2002-2004, Ralink Technology, Inc.
 *
 * All rights reserved.	Ralink's source	code is	an unpublished work and the
 * use of a copyright notice does not imply otherwise. This source code
 * contains confidential trade secret material of Ralink Tech. Any attemp
 * or participation in deciphering, decoding, reverse engineering or in any
 * way altering	the source code	is stricitly prohibited, unless	the prior
 * written consent of Ralink Technology, Inc. is obtained.
 ***************************************************************************

	Module Name:
	cmm_asic_rt.h

	Abstract:
	Ralink Wireless Chip HW related definition & structures

	Revision History:
	Who			When		  What
	--------	----------	  ----------------------------------------------
*/


#ifndef __CMM_ASIC_RT_H__
#define __CMM_ASIC_RT_H__

struct _RTMP_ADAPTER;
struct _MAC_TABLE_ENTRY;
struct _EDCA_PARM;
struct _CIPHER_KEY;


UINT32 RtAsicGetCrcErrCnt(struct _RTMP_ADAPTER *pAd);
UINT32 RtAsicGetCCACnt(struct _RTMP_ADAPTER *pAd);
UINT32 RtAsicGetChBusyCnt(struct _RTMP_ADAPTER *pAd, UCHAR ch_idx);
#ifdef FIFO_EXT_SUPPORT
BOOLEAN RtAsicGetFifoTxCnt(struct _RTMP_ADAPTER *pAd, struct _MAC_TABLE_ENTRY *pEntry);
VOID RtAsicFifoExtSet(struct _RTMP_ADAPTER *pAd);
VOID RtAsicFifoExtEntryClean(struct _RTMP_ADAPTER *pAd, struct _MAC_TABLE_ENTRY *pEntry);
#endif /* FIFO_EXT_SUPPORT */
#ifdef CONFIG_STA_SUPPORT
VOID RtAsicUpdateAutoFallBackTable(struct _RTMP_ADAPTER *pAd, UCHAR *pRateTable);
#endif /* CONFIG_STA_SUPPORT */


INT RtAsicSetAutoFallBack(struct _RTMP_ADAPTER *pAd, BOOLEAN enable);
INT RtAsicAutoFallbackInit(struct _RTMP_ADAPTER *pAd);
VOID RtAsicUpdateProtect(
	IN struct _RTMP_ADAPTER *pAd,
	IN USHORT OperationMode,
	IN UCHAR SetMask,
	IN BOOLEAN bDisableBGProtect,
	IN BOOLEAN bNonGFExist);
VOID RtAsicSwitchChannel(struct _RTMP_ADAPTER *pAd, UCHAR Channel, BOOLEAN bScan);

#ifdef ANT_DIVERSITY_SUPPORT
VOID RtAsicAntennaSelect(struct _RTMP_ADAPTER *pAd, UCHAR Channel);
#endif /* ANT_DIVERSITY_SUPPORT */

VOID RtAsicResetBBPAgent(struct _RTMP_ADAPTER *pAd);

VOID RtAsicSetBssid(struct _RTMP_ADAPTER *pAd, UCHAR *pBssid, UCHAR omac_idx);

INT RtAsicSetDevMac(struct _RTMP_ADAPTER *pAd, UCHAR *addr, UCHAR omac_idx);
#ifdef CONFIG_AP_SUPPORT
VOID RtAsicSetMbssMode(struct _RTMP_ADAPTER *pAd, UCHAR NumOfBcns);
#endif /* CONFIG_AP_SUPPORT */


#ifdef APCLI_SUPPORT
#ifdef MAC_REPEATER_SUPPORT
INT RtAsicSetMacAddrExt(struct _RTMP_ADAPTER *pAd, BOOLEAN enable);

VOID RtAsicInsertRepeaterEntry(
	IN struct _RTMP_ADAPTER *pAd,
	IN UCHAR CliIdx,
	IN PUCHAR pAddr);
VOID RtAsicRemoveRepeaterEntry(
	IN struct _RTMP_ADAPTER *pAd,
	IN UCHAR CliIdx);
#endif /* MAC_REPEATER_SUPPORT */
#endif /* APCLI_SUPPORT */


INT RtAsicSetRxFilter(struct _RTMP_ADAPTER *pAd);
#ifdef DOT11_N_SUPPORT
INT RtAsicSetRDG(struct _RTMP_ADAPTER *pAd, BOOLEAN bEnable);
#endif /* DOT11_N_SUPPORT */


VOID RtAsicSetPiggyBack(struct _RTMP_ADAPTER *pAd, BOOLEAN bPiggyBack);
INT RtAsicSetPreTbtt(struct _RTMP_ADAPTER *pAd, BOOLEAN enable);
INT RtAsicSetGPTimer(struct _RTMP_ADAPTER *pAd, BOOLEAN enable, UINT32 timeout);
INT RtAsicSetChBusyStat(struct _RTMP_ADAPTER *pAd, BOOLEAN enable);
INT RtAsicGetTsfTime(struct _RTMP_ADAPTER *pAd, UINT32 *high_part, UINT32 *low_part);
VOID RtAsicDisableSync(struct _RTMP_ADAPTER *pAd);
VOID RtAsicEnableBssSync(struct _RTMP_ADAPTER *pAd, USHORT BeaconPeriod);
VOID RtAsicEnableApBssSync(struct _RTMP_ADAPTER *pAd, USHORT BeaconPeriod);
#ifdef CONFIG_STA_SUPPORT
VOID RtAsicEnableIbssSync(struct _RTMP_ADAPTER *pAd);
#endif /* CONFIG_STA_SUPPORT */
INT RtAsicSetWmmParam(struct _RTMP_ADAPTER *pAd, UINT ac, UINT type, UINT val);
VOID RtAsicSetEdcaParm(struct _RTMP_ADAPTER *pAd, struct _EDCA_PARM *pEdcaParm);
#ifdef CONFIG_STA_SUPPORT
#ifdef RTMP_MAC_USB
#ifdef DOT11_N_SUPPORT
INT sta_wmm_adjust(struct _RTMP_ADAPTER *pAd);
#endif /* RTMP_MAC_USB */
#endif /* DOT11_N_SUPPORT */
#endif /* CONFIG_STA_SUPPORT */

#ifdef CONFIG_AP_SUPPORT
VOID dynamic_tune_be_tx_op(struct _RTMP_ADAPTER *pAd, ULONG nonBEpackets);
#endif /* CONFIG_AP_SUPPORT */
INT RtAsicSetRetryLimit(struct _RTMP_ADAPTER *pAd, UINT32 type, UINT32 limit);
UINT32 RtAsicGetRetryLimit(struct _RTMP_ADAPTER *pAd, UINT32 type);
VOID RtAsicSetSlotTime(struct _RTMP_ADAPTER *pAd, UINT32 SlotTime, UINT32 SifsTime);
INT RtAsicSetMacMaxLen(struct _RTMP_ADAPTER *pAd);
#ifdef CONFIG_AP_SUPPORT
VOID RtAsicGetTxTsc(struct _RTMP_ADAPTER *pAd, UCHAR apidx, UCHAR *pTxTsc);
#endif /* CONFIG_AP_SUPPORT */
VOID RtAsicAddSharedKeyEntry(
	IN struct _RTMP_ADAPTER *pAd,
	IN UCHAR		 	BssIndex,
	IN UCHAR		 	KeyIdx,
	IN struct _CIPHER_KEY *pCipherKey);
VOID RtAsicRemoveSharedKeyEntry(
	IN struct _RTMP_ADAPTER *pAd,
	IN UCHAR		 BssIndex,
	IN UCHAR		 KeyIdx);

VOID RtAsicUpdateWCIDIVEIV(
	IN struct _RTMP_ADAPTER *pAd,
	IN USHORT		WCID,
	IN ULONG        uIV,
	IN ULONG        uEIV);
#ifdef MCS_LUT_SUPPORT
VOID RtAsicMcsLutUpdate(struct _RTMP_ADAPTER *pAd, struct _MAC_TABLE_ENTRY *pEntry);
#endif /* MCS_LUT_SUPPORT */


VOID RtAsicUpdateBASession(struct _RTMP_ADAPTER *pAd, UCHAR wcid, UCHAR tid, UCHAR basize, BOOLEAN isAdd, INT ses_type);

VOID RtAsicUpdateRxWCIDTable(struct _RTMP_ADAPTER *pAd, USHORT WCID, UCHAR *pAddr);
VOID RtAsicUpdateWcidAttributeEntry(
	IN	struct _RTMP_ADAPTER *pAd,
	IN	UCHAR			BssIdx,
	IN 	UCHAR		 	KeyIdx,
	IN 	UCHAR		 	CipherAlg,
	IN	UINT8			Wcid,
	IN	UINT8			KeyTabFlag);
VOID RtAsicDelWcidTab(struct _RTMP_ADAPTER *pAd, UCHAR wcid_idx);
VOID RtAsicAddPairwiseKeyEntry(
	IN struct _RTMP_ADAPTER *pAd,
	IN UCHAR			WCID,
	IN PCIPHER_KEY		pCipherKey);
VOID RtAsicRemovePairwiseKeyEntry(
	IN struct _RTMP_ADAPTER *pAd,
	IN UCHAR		 Wcid);
INT RtAsicSendCommandToMcu(
	IN struct _RTMP_ADAPTER *pAd,
	IN UCHAR Command,
	IN UCHAR Token,
	IN UCHAR Arg0,
	IN UCHAR Arg1,
	IN BOOLEAN in_atomic);

#if defined(RLT_MAC) || defined(RTMP_MAC)
INT RtAsicTOPInit(struct _RTMP_ADAPTER *pAd);

VOID RtmpPrepareHwNullFrame(
	IN struct _RTMP_ADAPTER *pAd,
	IN struct _MAC_TABLE_ENTRY *pEntry,
	IN BOOLEAN bQosNull,
	IN BOOLEAN bEOSP,
	IN UCHAR OldUP,
	IN UCHAR OpMode,
	IN UCHAR PwrMgmt,
	IN BOOLEAN bWaitACK,
	IN CHAR Index);
#endif /* defined(RLT_MAC) || defined(RTMP_MAC) */

#endif /* __CMM_ASIC_RT_H__ */

