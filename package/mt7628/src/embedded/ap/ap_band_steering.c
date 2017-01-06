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
	ap_band_steering.c
*/

#ifdef BAND_STEERING
#include "rt_config.h"

#ifdef BNDSTRG_DAEMON
extern BNDSTRG_OPS D_BndStrgOps;
#else
extern BNDSTRG_OPS BndStrgOps;
#endif

/* ioctl */
INT Show_BndStrg_Info(
	PRTMP_ADAPTER	pAd,
	RTMP_STRING		*arg)
{
	PBND_STRG_CLI_TABLE table = P_BND_STRG_TABLE;

	if (table->Ops)
		table->Ops->ShowTableInfo(table);

	return TRUE;	
}


INT Show_BndStrg_List(
	PRTMP_ADAPTER	pAd,
	RTMP_STRING		*arg)
{
	PBND_STRG_CLI_TABLE table = P_BND_STRG_TABLE;
	
	if (table->Ops)
		table->Ops->ShowTableEntries(P_BND_STRG_TABLE);

	return TRUE;	
}


INT Set_BndStrg_Enable(
	PRTMP_ADAPTER	pAd,
	RTMP_STRING		*arg)
{
	PBND_STRG_CLI_TABLE table = P_BND_STRG_TABLE;
	BOOLEAN enable = (BOOLEAN) simple_strtol(arg, 0, 10);

	if (table->Ops)
		table->Ops->SetEnable(table, enable);

	return TRUE;
}


INT Set_BndStrg_RssiDiff(
	PRTMP_ADAPTER	pAd,
	RTMP_STRING		*arg)
{
	CHAR RssiDiff = (CHAR) simple_strtol(arg, 0, 10);
	PBND_STRG_CLI_TABLE table = P_BND_STRG_TABLE;

	if (table->Ops)
		table->Ops->SetRssiDiff(table, RssiDiff);

	table->RssiDiff = RssiDiff;
	DBGPRINT(RT_DEBUG_OFF, 
			("%s(): RssiCheck = %u\n", __FUNCTION__, table->RssiDiff));

	return TRUE;
}


INT Set_BndStrg_RssiLow(
	PRTMP_ADAPTER	pAd,
	RTMP_STRING		*arg)
{
	CHAR RssiLow = (CHAR) simple_strtol(arg, 0, 10);
	PBND_STRG_CLI_TABLE table = P_BND_STRG_TABLE;

	if (table->Ops)
		table->Ops->SetRssiLow(table, RssiLow);

	table->RssiLow = RssiLow;
	DBGPRINT(RT_DEBUG_OFF, 
			("%s(): RssiLow = %u\n", __FUNCTION__, table->RssiLow));

	return TRUE;
}


INT Set_BndStrg_Age(
	PRTMP_ADAPTER	pAd,
	RTMP_STRING		*arg)
{
	UINT32 AgeTime = (UINT32) simple_strtol(arg, 0, 10);
	PBND_STRG_CLI_TABLE table = P_BND_STRG_TABLE;

	if (table->Ops)
		table->Ops->SetAgeTime(table, AgeTime);

	return TRUE;
}


INT Set_BndStrg_HoldTime(
	PRTMP_ADAPTER	pAd,
	RTMP_STRING		*arg)
{
	UINT32 HoldTime = (UINT32) simple_strtol(arg, 0, 10);
	PBND_STRG_CLI_TABLE table = P_BND_STRG_TABLE;

	if (table->Ops)
		table->Ops->SetHoldTime(table, HoldTime);

	return TRUE;
}


INT Set_BndStrg_CheckTime5G(
	PRTMP_ADAPTER	pAd,
	RTMP_STRING		*arg)
{
	UINT32 CheckTime = (UINT32) simple_strtol(arg, 0, 10);
	PBND_STRG_CLI_TABLE table = P_BND_STRG_TABLE;

	if (table->Ops)
		table->Ops->SetCheckTime(table, CheckTime);

	return TRUE;
}


INT Set_BndStrg_FrmChkFlag(
	PRTMP_ADAPTER	pAd,
	RTMP_STRING		*arg)
{
	UINT32 FrmChkFlag = (UINT32) simple_strtol(arg, 0, 16);
	PBND_STRG_CLI_TABLE table = P_BND_STRG_TABLE;

	if (table->Ops)
		table->Ops->SetFrmChkFlag(table, FrmChkFlag);

	return TRUE;
}


INT Set_BndStrg_CndChkFlag(
	PRTMP_ADAPTER	pAd,
	RTMP_STRING		*arg)
{
	UINT32 CndChkFlag = (UINT32) simple_strtol(arg, 0, 16);
	PBND_STRG_CLI_TABLE table = P_BND_STRG_TABLE;

	if (table->Ops)
		table->Ops->SetCndChkFlag(table, CndChkFlag);

	return TRUE;
}

#ifdef BND_STRG_DBG
INT Set_BndStrg_MonitorAddr(
	PRTMP_ADAPTER	pAd,
	RTMP_STRING		*arg)
{
	UCHAR					MonitorAddr[MAC_ADDR_LEN];
	RTMP_STRING				*value;
	INT						i;
	PBND_STRG_CLI_TABLE table = P_BND_STRG_TABLE;


	if(strlen(arg) != 17)  /*Mac address acceptable format 01:02:03:04:05:06 length 17 */
		return FALSE;

	for (i=0, value = rstrtok(arg,":"); value; value = rstrtok(NULL,":")) 
	{
		if((strlen(value) != 2) || (!isxdigit(*value)) || (!isxdigit(*(value+1))) ) 
			return FALSE;  /*Invalid */

		AtoH(value, (UCHAR *)&MonitorAddr[i++], 1);
	}

	if (table->Ops)
		table->Ops->SetMntAddr(table, MonitorAddr);

	return TRUE;
}
#endif /* BND_STRG_DBG */
/**** end of ioctl ****/

INT BndStrg_Init(PRTMP_ADAPTER pAd)
{
	INT ret_val = BND_STRG_SUCCESS;
	PBND_STRG_CLI_TABLE table = P_BND_STRG_TABLE;

	DBGPRINT(RT_DEBUG_OFF, (YLW("%s()\n"), __FUNCTION__));

	ret_val = BndStrg_TableInit(pAd, table);

#ifndef BNDSTRG_DAEMON
	if (ret_val == BND_STRG_SUCCESS &&
		(table->b2GInfReady == TRUE && table->b5GInfReady == TRUE))
	{
		ret_val = BndStrg_Enable(table, 1);
	}
#endif

	if (ret_val != BND_STRG_SUCCESS)
	{
		BND_STRG_DBGPRINT(RT_DEBUG_ERROR,
					("Error in %s(), error code = %d!\n", __FUNCTION__, ret_val));
	}

	return ret_val;
}

INT BndStrg_TableInit(PRTMP_ADAPTER pAd, PBND_STRG_CLI_TABLE table)
{
	INT ret_val = BND_STRG_SUCCESS;

	if (table->bInitialized == TRUE)
		return BND_STRG_SUCCESS;

	NdisZeroMemory(table, sizeof(BND_STRG_CLI_TABLE));
	OS_NdisAllocateSpinLock(&table->Lock);

#ifdef BNDSTRG_DAEMON
	if (WMODE_CAP_5G(pAd->CommonCfg.PhyMode))
		table->Band = BAND_5G;
	else
		table->Band = BAND_24G;
#endif

	BndStrg_SetInfFlags(pAd, table, TRUE);

#ifdef BNDSTRG_DAEMON
	table->Ops = &D_BndStrgOps;
#else
	table->Ops = &BndStrgOps;
	BndStrg_InitTimer(&table->Timer, GET_TIMER_FUNCTION(BndStrg_PeriodicExec), table, TRUE);
#endif /* !BNDSTRG_DAEMON */
	table->RssiDiff = BND_STRG_RSSI_DIFF;
	table->RssiLow = BND_STRG_RSSI_LOW;
	table->AgeTime = BND_STRG_AGE_TIME;
	table->HoldTime = BND_STRG_HOLD_TIME;
	table->CheckTime_5G = BND_STRG_CHECK_TIME_5G;
	table->AutoOnOffThrd = BND_STRG_AUTO_ONOFF_THRD;
#if 1
	table->AlgCtrl.ConditionCheck = fBND_STRG_CND_5G_RSSI;
#endif
	table->AlgCtrl.FrameCheck =  fBND_STRG_FRM_CHK_PRB_REQ | \
								fBND_STRG_FRM_CHK_ATH_REQ;
	table->priv = (VOID *) pAd;
	table->bInitialized = TRUE;

	if (ret_val != BND_STRG_SUCCESS)
	{
		BND_STRG_DBGPRINT(RT_DEBUG_ERROR,
					("Error in %s(), error code = %d!\n", __FUNCTION__, ret_val));
	}

	return BND_STRG_SUCCESS;
}

INT BndStrg_Release(PRTMP_ADAPTER pAd)
{
	INT ret_val = BND_STRG_SUCCESS;
	PBND_STRG_CLI_TABLE table = P_BND_STRG_TABLE;

	DBGPRINT(RT_DEBUG_OFF, (YLW("%s()\n"), __FUNCTION__));
#if 1
	BndStrg_SetInfFlags(pAd, table, FALSE);

	if (table->bEnabled == TRUE)
		ret_val = BndStrg_Enable(table, 0);

	if ((table->b2GInfReady == FALSE && table->b5GInfReady == FALSE))
		ret_val = BndStrg_TableRelease(table);
#endif
	if (ret_val != BND_STRG_SUCCESS)
	{
		BND_STRG_DBGPRINT(RT_DEBUG_ERROR,
					("Error in %s(), error code = %d!\n", __FUNCTION__, ret_val));
	}

	return ret_val;
}


INT BndStrg_TableRelease(PBND_STRG_CLI_TABLE table)
{
	INT ret_val = BND_STRG_SUCCESS;

	if (table->bInitialized == FALSE)
		return BND_STRG_NOT_INITIALIZED;
	
	OS_NdisFreeSpinLock(&table->Lock);
	table->bInitialized = FALSE;

	if (ret_val != BND_STRG_SUCCESS)
	{
		BND_STRG_DBGPRINT(RT_DEBUG_ERROR,
					("Error in %s(), error code = %d!\n", __FUNCTION__, ret_val));
	}

	return ret_val;
}


INT BndStrg_InsertEntry(
	PBND_STRG_CLI_TABLE table,
	PUCHAR pAddr,
	PBND_STRG_CLI_ENTRY *entry_out)
{
	INT i;
	UCHAR HashIdx;
	PBND_STRG_CLI_ENTRY entry = NULL, this_entry = NULL;
	INT ret_val = BND_STRG_SUCCESS;

	if (table->Size >= BND_STRG_MAX_TABLE_SIZE) {
		DBGPRINT(RT_DEBUG_WARN, ("%s(): Table is full!\n", __FUNCTION__));
		return BND_STRG_TABLE_FULL;
	}

	NdisAcquireSpinLock(&table->Lock);
	for (i = 0; i< BND_STRG_MAX_TABLE_SIZE; i++)
	{
		entry = &table->Entry[i];

		/* pick up the first available vacancy*/
		if (entry->bValid == FALSE)	{
			NdisZeroMemory(entry, sizeof(BND_STRG_CLI_ENTRY));
			/* Fill Entry */
			RTMP_GetCurrentSystemTick(&entry->jiffies);
			COPY_MAC_ADDR(entry->Addr, pAddr);
			entry->bValid = TRUE;
			break;
		}
	}

	if (entry) {
		/* add this MAC entry into HASH table */
		HashIdx = MAC_ADDR_HASH_INDEX(pAddr);
		if (table->Hash[HashIdx] == NULL) {
			table->Hash[HashIdx] = entry;
		} else {
			this_entry = table->Hash[HashIdx];
			while (this_entry->pNext != NULL) {
				this_entry = this_entry->pNext;
			}
			this_entry->pNext = entry;
		}
		
		*entry_out = entry;
		table->Size++;
	}
	NdisReleaseSpinLock(&table->Lock);
#if 0
	BND_STRG_DBGPRINT(RT_DEBUG_OFF,
			("%s(): Index=%u, %02x:%02x:%02x:%02x:%02x:%02x, "
			"Table Size = %u\n",
			__FUNCTION__, i, PRINT_MAC(pAddr), table->Size));
#endif
	return ret_val;
}


INT BndStrg_DeleteEntry(PBND_STRG_CLI_TABLE table, PUCHAR pAddr, UINT32 Index)
{
	USHORT HashIdx;
	PBND_STRG_CLI_ENTRY entry, pre_entry, this_entry;
	INT ret_val = BND_STRG_SUCCESS;

#if 0
	BND_STRG_DBGPRINT(RT_DEBUG_OFF,
			("%s(): Index=%u, %02x:%02x:%02x:%02x:%02x:%02x, "
			"Table Size = %u\n",
			__FUNCTION__, Index, PRINT_MAC(pAddr), table->Size));
#endif

	NdisAcquireSpinLock(&table->Lock);
	HashIdx = MAC_ADDR_HASH_INDEX(pAddr);
	if (Index >= BND_STRG_MAX_TABLE_SIZE)
	{
		entry = table->Hash[HashIdx];
		while (entry) {
			if (MAC_ADDR_EQUAL(pAddr, entry->Addr)) {
				/* this is the entry we're looking for */
				break;
			} else {
				entry = entry->pNext;
			}
		}

		if (entry == NULL)
		{
			BND_STRG_DBGPRINT(RT_DEBUG_WARN,
				("%s(): Index=%u, %02x:%02x:%02x:%02x:%02x:%02x, "
				"Entry not found.\n",
				__FUNCTION__, Index, PRINT_MAC(pAddr)));
			NdisReleaseSpinLock(&table->Lock);
			return BND_STRG_INVALID_ARG;
		}
	}
	else	
		entry = &table->Entry[Index];
	
	if (entry && entry->bValid) 
	{
#if 0
		/*if (MAC_ADDR_EQUAL(entry->Addr, pAddr))*/
#endif
		{
			pre_entry = NULL;
			this_entry = table->Hash[HashIdx];
			ASSERT(this_entry);
			if (this_entry != NULL)
			{
				/* update Hash list*/
				do
				{
					if (this_entry == entry)
					{
						if (pre_entry == NULL)
							table->Hash[HashIdx] = entry->pNext;
						else
							pre_entry->pNext = entry->pNext;
						break;
					}

					pre_entry = this_entry;
					this_entry = this_entry->pNext;
				} while (this_entry);
			}

			/* not found !!!*/
			ASSERT(this_entry != NULL);

			NdisZeroMemory(entry->Addr, MAC_ADDR_LEN);
#ifndef BNDSTRG_DAEMON
			entry->jiffies = 0;
			entry->elapsed_time = 0;
#endif /* !BNDSTRG_DAEMON */
			entry->pNext = NULL;
			entry->bValid = FALSE;
			table->Size--;
		}
#if 0
		else {
			DBGPRINT(RT_DEBUG_ERROR,
					("%s(): worng entry!\n", __FUNCTION__));
			ret_val = BND_STRG_UNEXP;
		}
#endif
	}
	NdisReleaseSpinLock(&table->Lock);

	return ret_val;
}


PBND_STRG_CLI_ENTRY BndStrg_TableLookup(PBND_STRG_CLI_TABLE table, PUCHAR pAddr)
{
	ULONG HashIdx;
	BND_STRG_CLI_ENTRY *entry = NULL;
	
	HashIdx = MAC_ADDR_HASH_INDEX(pAddr);
	entry = table->Hash[HashIdx];

	while (entry && entry->bValid)
	{
		if (MAC_ADDR_EQUAL(entry->Addr, pAddr))
			break;
		else
			entry = entry->pNext;
	}

	return entry;
}

BOOLEAN BndStrg_CheckConnectionReq(
		PRTMP_ADAPTER	pAd,
		PUCHAR pSrcAddr,
		UINT8 FrameType,
		PCHAR Rssi)
{
	PBND_STRG_CLI_TABLE table = P_BND_STRG_TABLE;

	if (table->Ops && (table->bEnabled == TRUE))
	{
		return table->Ops->CheckConnectionReq(
										pAd,
										pSrcAddr,
										FrameType,
										Rssi);
	}
	
	return TRUE;
}


INT BndStrg_Enable(PBND_STRG_CLI_TABLE table, BOOLEAN enable)
{
#ifdef BNDSTRG_DAEMON
	BNDSTRG_MSG msg;
	PRTMP_ADAPTER pAd = NULL;
#endif

	if (table == NULL)
		return BND_STRG_TABLE_IS_NULL;

	if (table->bInitialized == FALSE)
		return BND_STRG_NOT_INITIALIZED;
#ifndef BNDSTRG_DAEMON
	if (table->b2GInfReady == FALSE)
		return BND_STRG_2G_INF_NOT_READY;

	if (table->b5GInfReady == FALSE)
		return BND_STRG_5G_INF_NOT_READY;
#endif /* BNDSTRG_DAEMON */
	if (!(table->bEnabled ^ enable))
	{
		/* Already enabled/disabled */
		BND_STRG_DBGPRINT(RT_DEBUG_OFF, /* TRACE */
				(GRN("%s(): Band steering is already %s.\n"),
				__FUNCTION__, (enable ? "enabled" : "disabled")));
		return BND_STRG_SUCCESS;
	}

	if (enable)
	{
#ifndef BNDSTRG_DAEMON
		BndStrg_SetTimer(&table->Timer, BND_STRG_TIMER_PERIOD);
#endif /* BNDSTRG_DAEMON */
		table->bEnabled = TRUE;
	}
	else
	{
#ifndef BNDSTRG_DAEMON
		BOOLEAN Cancelled;
		BndStrg_CancelTimer(&table->Timer, &Cancelled);
#endif /* BNDSTRG_DAEMON */

		table->bEnabled = FALSE;
	}

#ifdef BNDSTRG_DAEMON
		pAd = (PRTMP_ADAPTER) table->priv;
		msg.Action = BNDSTRG_ONOFF;
		msg.OnOff = table->bEnabled;
#if 1
		RtmpOSWrielessEventSend(
			pAd->net_dev,
			RT_WLAN_EVENT_CUSTOM,
			OID_BNDSTRG_MSG,
			NULL,
			(UCHAR *)&msg,
			sizeof(msg));
#else
		D_BndStrgSendMsg(pAd, &msg)
#endif
#endif /* BNDSTRG_DAEMON */

	BND_STRG_DBGPRINT(RT_DEBUG_OFF,
				(GRN("%s(): Band steering %s running.\n"),
				__FUNCTION__, (enable ? "start" : "stop")));

	return BND_STRG_SUCCESS;
}


INT BndStrg_SetInfFlags(PRTMP_ADAPTER pAd, PBND_STRG_CLI_TABLE table, BOOLEAN bInfReady)
{
	INT ret_val = BND_STRG_SUCCESS;
#if 1
	BNDSTRG_MSG msg;
#endif

	if (WMODE_CAP_5G(pAd->CommonCfg.PhyMode) &&
		(table->b5GInfReady ^ bInfReady))
	{
		table->b5GInfReady = bInfReady;

#if 1
		msg.Action = INF_STATUS_RSP_5G;
		msg.b5GInfReady = table->b5GInfReady;
		RtmpOSWrielessEventSend(
			pAd->net_dev,
			RT_WLAN_EVENT_CUSTOM,
			OID_BNDSTRG_MSG,
			NULL,
			(UCHAR *)&msg,
			sizeof(msg));
#endif
		BND_STRG_DBGPRINT(RT_DEBUG_OFF,
					(BLUE("%s(): BSS (%02x:%02x:%02x:%02x:%02x:%02x)")
					 BLUE(" set 5G Inf %s.\n")
					 , __FUNCTION__, PRINT_MAC(pAd->ApCfg.MBSSID[0].wdev.bssid),
					 bInfReady ? "ready" : "not ready"));
	}
	else if (table->b2GInfReady ^ bInfReady)
	{
		table->b2GInfReady = bInfReady;
#if 1
		msg.Action = INF_STATUS_RSP_2G;
		msg.b2GInfReady = table->b2GInfReady;
#if 1
		RtmpOSWrielessEventSend(
			pAd->net_dev,
			RT_WLAN_EVENT_CUSTOM,
			OID_BNDSTRG_MSG,
			NULL,
			(UCHAR *)&msg,
			sizeof(msg));
#else
		D_BndStrgSendMsg(pAd, &msg);
#endif
#endif
		BND_STRG_DBGPRINT(RT_DEBUG_OFF,
					(BLUE("%s(): BSS (%02x:%02x:%02x:%02x:%02x:%02x)")
					 BLUE(" set 2G Inf %s.\n")
					 , __FUNCTION__, PRINT_MAC(pAd->ApCfg.MBSSID[0].wdev.bssid),
					 bInfReady ? "ready" : "not ready"));
	}
	
	return ret_val;
}


BOOLEAN BndStrg_IsClientStay(
			PRTMP_ADAPTER pAd,
			PMAC_TABLE_ENTRY pEntry)
{
	PBND_STRG_CLI_TABLE table = P_BND_STRG_TABLE;
	CHAR Rssi = RTMPAvgRssi(pAd, &pEntry->RssiSample);
	
	if (table->AlgCtrl.ConditionCheck & fBND_STRG_CND_5G_RSSI &&
		table->Band == BAND_5G &&
		(Rssi < (table->RssiLow - 10/*Test*/)))
	{
		BNDSTRG_MSG msg;

		msg.Action = CLI_DEL;
		COPY_MAC_ADDR(msg.Addr, pEntry->Addr);
		 /* we don't know the index, daemon should look it up */
		msg.TalbeIndex = BND_STRG_MAX_TABLE_SIZE;

		BND_STRG_DBGPRINT(RT_DEBUG_TRACE,
				(YLW("%s(): Kick client (%02x:%02x:%02x:%02x:%02x:%02x)")
				 YLW(" due to low Rssi(%d).\n")
				 , __FUNCTION__, PRINT_MAC(pEntry->Addr), Rssi));

		RtmpOSWrielessEventSend(
			pAd->net_dev,
			RT_WLAN_EVENT_CUSTOM,
			OID_BNDSTRG_MSG,
			NULL,
			(UCHAR *) &msg,
			sizeof(BNDSTRG_MSG));

		table->Ops->TableEntryDel(table, pEntry->Addr, BND_STRG_MAX_TABLE_SIZE);

		return FALSE;
	}

	return TRUE;
}


#ifdef BNDSTRG_DAEMON
INT BndStrg_MsgHandle(PRTMP_ADAPTER pAd, RTMP_IOCTL_INPUT_STRUCT *wrq)
{
	BNDSTRG_MSG msg;
	PBND_STRG_CLI_TABLE table = P_BND_STRG_TABLE;

	if (table->bInitialized == FALSE)
		return BND_STRG_NOT_INITIALIZED;
	
	if (wrq->u.data.length != sizeof(BNDSTRG_MSG))
		return BND_STRG_INVALID_ARG;
	else
	{
		copy_from_user(&msg, wrq->u.data.pointer, wrq->u.data.length);
		if (table->Ops->MsgHandle)
			table->Ops->MsgHandle(pAd, &msg);
	}

	return BND_STRG_SUCCESS;
}

static INT D_BndStrgSendMsg(
			PRTMP_ADAPTER pAd,
			BNDSTRG_MSG *msg)
{
	return	RtmpOSWrielessEventSend(
			pAd->net_dev,
			RT_WLAN_EVENT_CUSTOM,
			OID_BNDSTRG_MSG,
			NULL,
			(UCHAR *) msg,
			sizeof(BNDSTRG_MSG));
}


static VOID D_ShowTableInfo(PBND_STRG_CLI_TABLE table)
{
	BNDSTRG_MSG msg;
	PRTMP_ADAPTER pAd = (PRTMP_ADAPTER) table->priv;
	
	msg.Action = TABLE_INFO;

	D_BndStrgSendMsg(pAd, &msg);
}

static VOID D_ShowTableEntries(PBND_STRG_CLI_TABLE table)
{
	BNDSTRG_MSG msg;
	PRTMP_ADAPTER pAd = (PRTMP_ADAPTER) table->priv;

#if 1
	INT i;
	BND_STRG_DBGPRINT(RT_DEBUG_OFF,
						("\t%s Accessible Clients:\n",
						(table->Band == BAND_24G) ?
						"2.4G" : "5G"));

	for (i = 0; i < BND_STRG_MAX_TABLE_SIZE; i++)
	{
		if (table->Entry[i].bValid)
		{
			BND_STRG_DBGPRINT(RT_DEBUG_OFF,
							("\t%d: %02x:%02x:%02x:%02x:%02x:%02x\n",
							 i, PRINT_MAC(table->Entry[i].Addr)));
		}
	}
#endif

	msg.Action = ENTRY_LIST;
	D_BndStrgSendMsg(pAd, &msg);
}

#if 0
static INT D_TableEntryAdd(
			BND_STRG_CLI_TABLE *table,
			PUCHAR pAddr,
			PBND_STRG_CLI_ENTRY *entry_out)
{
	INT i;
	UCHAR HashIdx;
	PBND_STRG_CLI_ENTRY entry = NULL, this_entry = NULL;
	INT ret_val = BND_STRG_SUCCESS;

	if (table->Size >= BND_STRG_MAX_TABLE_SIZE) {
		DBGPRINT(RT_DEBUG_WARN, ("%s(): Table is full!\n", __FUNCTION__));
		return BND_STRG_TABLE_FULL;
	}

	NdisAcquireSpinLock(&table->Lock);
	for (i = 0; i< BND_STRG_MAX_TABLE_SIZE; i++)
	{
		entry = &table->Entry[i];

		/* pick up the first available vacancy*/
		if (entry->bValid == FALSE)	{
			NdisZeroMemory(entry, sizeof(BND_STRG_CLI_ENTRY));
			/* Fill Entry */
			RTMP_GetCurrentSystemTick(&entry->jiffies);
			COPY_MAC_ADDR(entry->Addr, pAddr);
			entry->bValid = TRUE;
			break;
		}
	}

	if (entry) {
		/* add this MAC entry into HASH table */
		HashIdx = MAC_ADDR_HASH_INDEX(pAddr);
		if (table->Hash[HashIdx] == NULL) {
			table->Hash[HashIdx] = entry;
		} else {
			this_entry = table->Hash[HashIdx];
			while (this_entry->pNext != NULL) {
				this_entry = this_entry->pNext;
			}
			this_entry->pNext = entry;
		}
		
		*entry_out = entry;
		table->Size++;
	}
	NdisReleaseSpinLock(&table->Lock);
#if 0
	BND_STRG_DBGPRINT(RT_DEBUG_OFF,
			("%s(): Index=%u, %02x:%02x:%02x:%02x:%02x:%02x, "
			"Table Size = %u\n",
			__FUNCTION__, i, PRINT_MAC(pAddr), table->Size));
#endif
	return ret_val;
}

static INT D_TableEntryDel(
			BND_STRG_CLI_TABLE *table,
			PUCHAR pAddr,
			UINT32 Index)
{
	USHORT HashIdx;
	PBND_STRG_CLI_ENTRY entry, pre_entry, this_entry;
	INT ret_val = BND_STRG_SUCCESS;

	if (Index >= BND_STRG_MAX_TABLE_SIZE)
		return BND_STRG_INVALID_ARG;
#if 0
	BND_STRG_DBGPRINT(RT_DEBUG_OFF,
			("%s(): Index=%u, %02x:%02x:%02x:%02x:%02x:%02x, "
			"Table Size = %u\n",
			__FUNCTION__, Index, PRINT_MAC(pAddr), table->Size));
#endif
	NdisAcquireSpinLock(&table->Lock);
	HashIdx = MAC_ADDR_HASH_INDEX(pAddr);
	entry = &table->Entry[Index];
	if (entry && entry->bValid) 
	{
#if 0
		/*if (MAC_ADDR_EQUAL(entry->Addr, pAddr))*/
#endif
		{
			pre_entry = NULL;
			this_entry = table->Hash[HashIdx];
			ASSERT(this_entry);
			if (this_entry != NULL)
			{
				/* update Hash list*/
				do
				{
					if (this_entry == entry)
					{
						if (pre_entry == NULL)
							table->Hash[HashIdx] = entry->pNext;
						else
							pre_entry->pNext = entry->pNext;
						break;
					}

					pre_entry = this_entry;
					this_entry = this_entry->pNext;
				} while (this_entry);
			}

			/* not found !!!*/
			ASSERT(this_entry != NULL);

			NdisZeroMemory(entry->Addr, MAC_ADDR_LEN);
			entry->jiffies = 0;
			entry->elapsed_time = 0;
			entry->bValid = FALSE;
			table->Size--;
		}
#if 0
		else {
			DBGPRINT(RT_DEBUG_ERROR,
					("%s(): worng entry!\n", __FUNCTION__));
			ret_val = BND_STRG_UNEXP;
		}
#endif
	}
	NdisReleaseSpinLock(&table->Lock);

	return ret_val;
}

static INT D_TableLookup(
			BND_STRG_CLI_TABLE *table,
			PUCHAR pAddr)
{
	ULONG HashIdx;
	BND_STRG_CLI_ENTRY *entry = NULL;
	
	HashIdx = MAC_ADDR_HASH_INDEX(pAddr);
	entry = table->Hash[HashIdx];

	while (entry && entry->bValid)
	{
		if (MAC_ADDR_EQUAL(entry->Addr, pAddr))
			break;
		else
			entry = entry->pNext;
	}

	return entry;
}
#endif

static BOOLEAN D_CheckConnectionReq(
			PRTMP_ADAPTER pAd,
			PUCHAR pSrcAddr,
			UINT8 FrameType,
			PCHAR Rssi)
{
	PBND_STRG_CLI_TABLE table = P_BND_STRG_TABLE;
	BNDSTRG_MSG msg;
	UINT32 frame_type_to_frame_check_flags[] = { \
								fBND_STRG_FRM_CHK_PRB_REQ,
								0,
								fBND_STRG_FRM_CHK_ASS_REQ,
								fBND_STRG_FRM_CHK_ATH_REQ};
	UINT32 frame_check_flags = 0;

	/* Send to daemon */
	NdisCopyMemory(msg.Rssi, Rssi, 3);
	msg.Action= CONNECTION_REQ;
	msg.Band = table->Band;
	msg.FrameType = FrameType;
	COPY_MAC_ADDR(msg.Addr, pSrcAddr);
	D_BndStrgSendMsg(pAd, &msg);
#if 0
	DBGPRINT(RT_DEBUG_OFF, ("%02x:%02x:%02x:%02x:%02x:%02x, Band = %u, FrameType = %u\n",
			PRINT_MAC(pSrcAddr), table->Band, FrameType));
#endif	

	if (FrameType < (sizeof(frame_type_to_frame_check_flags)/sizeof(UINT32)))
		frame_check_flags = frame_type_to_frame_check_flags[FrameType];
	else
		{/* invalid frame type */}

	if (table->bEnabled == TRUE &&
		frame_check_flags & table->AlgCtrl.FrameCheck)
	{
		PBND_STRG_CLI_ENTRY entry = NULL;

		if (table->Ops)
			entry = table->Ops->TableLookup(table, pSrcAddr);

		if (entry/* || table->Band == BAND_5G*/)
			return TRUE;
		else
		{
#ifdef BND_STRG_QA
			BND_STRG_PRINTQAMSG(table, pSrcAddr,
			(RED("%s: check %s request failed. client's (%02x:%02x:%02x:%02x:%02x:%02x)"
			" request is ignored. \n"), (table->Band == BAND_24G ? "2.4G" : "5G"),
			FrameType == 0 ? ("probe") : (FrameType == 3 ? "auth" : "unknow"),
			PRINT_MAC(pSrcAddr)));
#endif
			return FALSE;
		}			

	}
	
	return TRUE;
}


static VOID D_InfStatusRsp(PBND_STRG_CLI_TABLE table, BNDSTRG_MSG *msg)
{
	PRTMP_ADAPTER pAd = (PRTMP_ADAPTER) table->priv;
	BNDSTRG_MSG Rsp;

	/* If user config is OFF, don't respond to this query */
	if (pAd->ApCfg.BandSteering == 0)
		return;
	else if (pAd->ApCfg.BandSteering == 2 /* Auto OnOff */
		&& pAd->CommonCfg.Channel <= 14) 
	{
		// TODO: Use Avg False CCA
		if (pAd->RalinkCounters.OneSecFalseCCACnt > table->AutoOnOffThrd &&
			table->bEnabled == FALSE) {
			table->Ops->SetEnable(table, 1);
			return;
		} else if (pAd->RalinkCounters.OneSecFalseCCACnt < table->AutoOnOffThrd &&
			table->bEnabled == TRUE){
			table->Ops->SetEnable(table, 0);
			return;
		}
	}
	
	/*INF_STATUS*/
	if (table->bInitialized == TRUE)
	{
		/* Send to daemon */
		if (table->Band == BAND_24G) {
			Rsp.Action = INF_STATUS_RSP_2G;
			Rsp.b2GInfReady = table->b2GInfReady;
		} else if (table->Band == BAND_5G) {
			Rsp.Action = INF_STATUS_RSP_5G;
			Rsp.b5GInfReady = table->b5GInfReady;
		}
		else
		{
			return;
		}
		D_BndStrgSendMsg(pAd, &Rsp);

	}
}

/* For IOCTL */
static INT D_SetEnable(
			PBND_STRG_CLI_TABLE table,
			BOOLEAN enable)
{
	INT ret_val = BND_STRG_SUCCESS;
	PRTMP_ADAPTER pAd = (PRTMP_ADAPTER) table->priv;

	DBGPRINT(RT_DEBUG_OFF, 
			("%s(): enable = %u\n", __FUNCTION__,  enable));

	if (!(table->bEnabled ^ enable))
	{
		/* Already enabled/disabled */
		BND_STRG_DBGPRINT(RT_DEBUG_OFF, /* TRACE */
				(GRN("%s(): Band steering is already %s.\n"),
				__FUNCTION__, (enable ? "enabled" : "disabled")));
		return BND_STRG_SUCCESS;
	}

	pAd->ApCfg.BandSteering = enable;

	if (enable)
		ret_val = BndStrg_Init(pAd);
	else
		ret_val = BndStrg_Release(pAd);

	if (ret_val != BND_STRG_SUCCESS)
	{
		BND_STRG_DBGPRINT(RT_DEBUG_ERROR,
					("Error in %s(), error code = %d!\n", __FUNCTION__, ret_val));
	}

	return TRUE;
}


static INT D_SetRssiDiff(
			PBND_STRG_CLI_TABLE table,
			CHAR RssiDiff)
{
	PRTMP_ADAPTER pAd = (PRTMP_ADAPTER) table->priv;
	BNDSTRG_MSG msg;

	table->RssiDiff = RssiDiff;
	DBGPRINT(RT_DEBUG_OFF, 
			("%s(): RssiCheck = %u\n", __FUNCTION__, table->RssiDiff));

	msg.Action = SET_RSSI_DIFF;
	msg.RssiDiff = RssiDiff;
	D_BndStrgSendMsg(pAd, &msg);

	return TRUE;
}


static INT D_SetRssiLow(
			PBND_STRG_CLI_TABLE table,
			CHAR RssiLow)
{
	PRTMP_ADAPTER pAd = (PRTMP_ADAPTER) table->priv;
	BNDSTRG_MSG msg;

	table->RssiLow = RssiLow;
	DBGPRINT(RT_DEBUG_OFF, 
			("%s(): RssiLow = %u\n", __FUNCTION__, table->RssiLow));

	msg.Action = SET_RSSI_LOW;
	msg.RssiLow = RssiLow;
	D_BndStrgSendMsg(pAd, &msg);

	return TRUE;
}


static INT D_SetAgeTime(
			PBND_STRG_CLI_TABLE table,
			UINT32	Time)
{
	PRTMP_ADAPTER pAd = (PRTMP_ADAPTER) table->priv;
	BNDSTRG_MSG msg;

	table->AgeTime = Time;
	DBGPRINT(RT_DEBUG_OFF, 
			("%s(): AgeTime = %u\n", __FUNCTION__, table->AgeTime));

	msg.Action = SET_AGE_TIME;
	msg.Time = table->AgeTime;
	D_BndStrgSendMsg(pAd, &msg);

	return TRUE;
}


static INT D_SetHoldTime(
			PBND_STRG_CLI_TABLE table,
			UINT32	Time)
{
	PRTMP_ADAPTER pAd = (PRTMP_ADAPTER) table->priv;
	BNDSTRG_MSG msg;
	
	table->HoldTime= Time;
	DBGPRINT(RT_DEBUG_OFF, 
			("%s(): HoldTime = %u\n", __FUNCTION__, table->HoldTime));

	msg.Action = SET_HOLD_TIME;
	msg.Time = table->HoldTime;
	D_BndStrgSendMsg(pAd, &msg);

	return TRUE;
}


static INT D_SetCheckTime(
			PBND_STRG_CLI_TABLE table,
			UINT32	Time)
{
	PRTMP_ADAPTER pAd = (PRTMP_ADAPTER) table->priv;
	BNDSTRG_MSG msg;

	table->CheckTime_5G = Time;
	DBGPRINT(RT_DEBUG_OFF, 
			("%s(): CheckTime_5G = %u\n", __FUNCTION__, table->CheckTime_5G));

	msg.Action = SET_CHECK_TIME;
	msg.Time = table->CheckTime_5G;
	D_BndStrgSendMsg(pAd, &msg);

	return TRUE;
}


static INT D_SetFrmChkFlag(
			PBND_STRG_CLI_TABLE table,
			UINT32	FrmChkFlag)
{
	table->AlgCtrl.FrameCheck = FrmChkFlag;

	DBGPRINT(RT_DEBUG_OFF,
			("%s(): FrameCheck = 0x%x\n", __FUNCTION__, table->AlgCtrl.FrameCheck));

	return TRUE;
}


static INT D_SetCndChkFlag(
			PBND_STRG_CLI_TABLE table,
			UINT32	CndChkFlag)
{
	PRTMP_ADAPTER pAd = (PRTMP_ADAPTER) table->priv;
	BNDSTRG_MSG msg;

	table->AlgCtrl.ConditionCheck = CndChkFlag;

	DBGPRINT(RT_DEBUG_OFF,
			("%s(): CndChkFlag = 0x%x\n", __FUNCTION__, table->AlgCtrl.ConditionCheck));

	msg.Action = SET_CHEK_CONDITIONS;
	msg.ConditionCheck = table->AlgCtrl.ConditionCheck;
	D_BndStrgSendMsg(pAd, &msg);

	return TRUE;
}

#ifdef BND_STRG_DBG
static INT D_SetMntAddr(
			PBND_STRG_CLI_TABLE table,
			PUCHAR Addr)
{
	PRTMP_ADAPTER pAd = (PRTMP_ADAPTER) table->priv;
	BNDSTRG_MSG msg;
	
	COPY_MAC_ADDR(table->MonitorAddr, Addr);
	DBGPRINT(RT_DEBUG_OFF, 
			("%s(): %02x:%02x:%02x:%02x:%02x:%02x\n",
			__FUNCTION__, PRINT_MAC(table->MonitorAddr)));

	msg.Action = SET_MNT_ADDR;
	COPY_MAC_ADDR(msg.Addr, table->MonitorAddr);
	D_BndStrgSendMsg(pAd, &msg);

	return TRUE;
}
#endif /* BND_STRG_DBG */

static VOID D_MsgHandle(
			PRTMP_ADAPTER	pAd,
			BNDSTRG_MSG *msg)
{
	PBND_STRG_CLI_TABLE table = P_BND_STRG_TABLE;
	PBND_STRG_CLI_ENTRY entry = NULL;

	BND_STRG_DBGPRINT(RT_DEBUG_INFO, ("%s: action code (%d)\n",__FUNCTION__, msg->Action));

	if (!table)
		return;

	switch (msg->Action)
	{
		case CLI_ADD:
			if (!table->Ops->TableLookup(table, msg->Addr))
				table->Ops->TableEntryAdd(table, msg->Addr, &entry);
			break;
						
		case CLI_DEL:
			table->Ops->TableEntryDel(table, msg->Addr, 0xFF);
			break;

		case CLI_AGING_REQ:
			msg->Action = CLI_AGING_RSP;
			msg->Band = table->Band;

			if (MacTableLookup(pAd, msg->Addr) == NULL)
			{
				/* we can aging the entry if it is not in the mac table */
				msg->ReturnCode = BND_STRG_SUCCESS;
				table->Ops->TableEntryDel(table, msg->Addr, 0xFF);
			}
			else
			{
				msg->ReturnCode = BND_STRG_STA_IS_CONNECTED;
			}

			D_BndStrgSendMsg(pAd, msg);

			break;

		case INF_STATUS_QUERY:
			D_InfStatusRsp(table, msg);
			break;

		case BNDSTRG_ONOFF:
			BndStrg_Enable(table, msg->OnOff);
			break;

		default:
			DBGPRINT(RT_DEBUG_WARN, ("%s: unknown action code. (%d)\n",__FUNCTION__, msg->Action));
			break;
	}
}

BNDSTRG_OPS D_BndStrgOps = {
	.ShowTableInfo = D_ShowTableInfo,
	.ShowTableEntries = D_ShowTableEntries,
	.TableEntryAdd = BndStrg_InsertEntry,
	.TableEntryDel = BndStrg_DeleteEntry,
	.TableLookup = BndStrg_TableLookup,
	.CheckConnectionReq = D_CheckConnectionReq,
	.SetEnable = D_SetEnable,
	.SetRssiDiff = D_SetRssiDiff,
	.SetRssiLow = D_SetRssiLow,
	.SetAgeTime = D_SetAgeTime,
	.SetHoldTime = D_SetHoldTime,
	.SetCheckTime = D_SetCheckTime,
	.SetFrmChkFlag = D_SetFrmChkFlag,
	.SetCndChkFlag = D_SetCndChkFlag,
#ifdef BND_STRG_DBG
	.SetMntAddr = D_SetMntAddr,
#endif /* BND_STRG_DBG */
	.MsgHandle= D_MsgHandle,
};
#else
extern NDIS_SPIN_LOCK TimerSemLock;

static UINT32 BndStrg_GetEntryElapsedTime(PBND_STRG_CLI_ENTRY entry);
static INT BndStrg_PrintEntryStatistics(PBND_STRG_ENTRY_STAT statistics);
static BOOLEAN _BndStrg_AllowStaConnect2G(PBND_STRG_CLI_TABLE table, PBND_STRG_CLI_ENTRY entry);

static BND_STRG_CLI_TABLE BndStrgTable;

static VOID ShowTableInfo(PBND_STRG_CLI_TABLE table)
{
	if (!table)
	{
		DBGPRINT(RT_DEBUG_ERROR,
					("%s(): Error! table is NULL!\n", __FUNCTION__));
		return;
	}
	
	BND_STRG_DBGPRINT(RT_DEBUG_OFF,
						("%s():\n"
							"\tbInitialized = %u\n"
							"\tbEnabled = %u\n"
							"\tb2GInfReady = %u\n"
							"\tb5GInfReady = %u\n"
							"\tRssiDiff = %d (dB)\n"
							"\tAgeTime = %u (ms)\n"
							"\tHoldTime = %u (ms)\n"
							"\tCheckTime_5G = %u (ms)\n"
#ifdef BND_STRG_DBG
							"\tMntAddr = %02x:%02x:%02x:%02x:%02x:%02x\n"
#endif /* BND_STRG_DBG */
							"\tSize = %u\n",
							__FUNCTION__,
							table->bInitialized,
							table->bEnabled,
							table->b2GInfReady,
							table->b5GInfReady,
							table->RssiDiff,
							table->AgeTime,
							table->HoldTime,
							table->CheckTime_5G,
#ifdef BND_STRG_DBG
							PRINT_MAC(table->MonitorAddr),
#endif /* BND_STRG_DBG */
							table->Size));
}

static VOID ShowTableEntries(PBND_STRG_CLI_TABLE table)
{
	INT i;

	for (i = 0; i < BND_STRG_MAX_TABLE_SIZE; i++)
	{
		BndStrg_PrintEntry(&table->Entry[i]);
	}
}

static VOID BndStrg_InitTimer(
	RALINK_TIMER_STRUCT *pTimer,
	VOID *pTimerFunc,
	VOID *pData,
	BOOLEAN	 Repeat)
{
	RTMP_SEM_LOCK(&TimerSemLock);

	pTimer->Valid		= TRUE;
	pTimer->PeriodicType	= Repeat;
	pTimer->State		= FALSE;
	pTimer->cookie		= (ULONG) pData;
	pTimer->pAd			= NULL;

	RTMP_OS_Init_Timer(NULL, &pTimer->TimerObj, pTimerFunc, (PVOID) pTimer, NULL);	
	DBGPRINT(RT_DEBUG_TRACE,("%s: %lx\n",__FUNCTION__, (ULONG)pTimer));

	RTMP_SEM_UNLOCK(&TimerSemLock);
}


static VOID BndStrg_SetTimer(RALINK_TIMER_STRUCT *pTimer, ULONG Value)
{
	RTMP_SEM_LOCK(&TimerSemLock);

	if (pTimer->Valid)
	{
#if 0		
		RTMP_ADAPTER *pAd;

		pAd = (RTMP_ADAPTER *)pTimer->pAd;
		if (RTMP_TEST_FLAG(pAd, fRTMP_ADAPTER_HALT_IN_PROGRESS | fRTMP_ADAPTER_NIC_NOT_EXIST))
		{
			DBGPRINT_ERR(("%s() failed, Halt in Progress!\n", __FUNCTION__));
			RTMP_SEM_UNLOCK(&TimerSemLock);
			return;
		}
#endif	
		pTimer->TimerValue = Value;
		pTimer->State = FALSE;
		if (pTimer->PeriodicType == TRUE)
		{
			pTimer->Repeat = TRUE;
			RTMP_SetPeriodicTimer(&pTimer->TimerObj, Value);
		}
		else
		{
			pTimer->Repeat = FALSE;
			RTMP_OS_Add_Timer(&pTimer->TimerObj, Value);
		}

		DBGPRINT(RT_DEBUG_INFO,("%s: %lx\n",__FUNCTION__, (ULONG)pTimer));
	}
	else
	{
		DBGPRINT_ERR(("%s() failed, Timer hasn't been initialize!\n", __FUNCTION__));
	}
	RTMP_SEM_UNLOCK(&TimerSemLock);
}


static VOID BndStrg_CancelTimer(RALINK_TIMER_STRUCT *pTimer, BOOLEAN *pCancelled)
{
	RTMP_SEM_LOCK(&TimerSemLock);

	if (pTimer->Valid)
	{
		if (pTimer->State == FALSE)
			pTimer->Repeat = FALSE;
		
		RTMP_OS_Del_Timer(&pTimer->TimerObj, pCancelled);
		
		if (*pCancelled == TRUE)
			pTimer->State = TRUE;
#if 0
#ifdef RTMP_TIMER_TASK_SUPPORT
		/* We need to go-through the TimerQ to findout this timer handler and remove it if */
		/*		it's still waiting for execution.*/
		RtmpTimerQRemove(pTimer->pAd, pTimer);
#endif /* RTMP_TIMER_TASK_SUPPORT */
#endif
		DBGPRINT(RT_DEBUG_INFO,("%s: %lx\n",__FUNCTION__, (ULONG)pTimer));
	}
	else
	{
		DBGPRINT(RT_DEBUG_INFO,("%s() failed, Timer hasn't been initialize!\n", __FUNCTION__));
	}

	RTMP_SEM_UNLOCK(&TimerSemLock);
}


VOID BndStrg_PeriodicExec(
	PVOID SystemSpecific1, 
	PVOID FunctionContext, 
	PVOID SystemSpecific2, 
	PVOID SystemSpecific3)
{
	INT i;
	UINT32 elapsed_time = 0;
	PBND_STRG_CLI_ENTRY entry = NULL;
	PBND_STRG_CLI_TABLE table = (PBND_STRG_CLI_TABLE)FunctionContext;

	if (!table)
	{
		DBGPRINT(RT_DEBUG_ERROR,
					("%s(): Error! table is NULL!\n", __FUNCTION__));
		return;
	}

	for (i = 0; i < BND_STRG_MAX_TABLE_SIZE; i++)
	{
		entry = &table->Entry[i];
		if (entry->bValid == TRUE)
		{
			elapsed_time = BndStrg_GetEntryElapsedTime(entry);
			if (elapsed_time >= table->AgeTime)
			{
				BndStrg_DeleteEntry(table, entry->Addr, i);
			}
			else
			{
				/* Update elapsed time */
				entry->elapsed_time = elapsed_time;

				if (elapsed_time >= table->CheckTime_5G &&
					!(entry->Control_Flags & fBND_STRG_CLIENT_SUPPORT_5G) &&
					!(entry->Control_Flags & fBND_STRG_CLIENT_IS_2G_ONLY))
				{
					/*	If we don't get any connection req from 5G for a long time,
						we condider this client is 2.4G only */
#ifdef BND_STRG_QA
					BND_STRG_PRINTQAMSG(table, entry,
					(YLW("Receive no frame by 5G interface within %u seconds,"
					" set client (%02x:%02x:%02x:%02x:%02x:%02x) to 2.4G only.\n"),
					table->CheckTime_5G/1000, PRINT_MAC(entry->Addr)));
#endif /* BND_STRG_QA */
					entry->Control_Flags |= \
						fBND_STRG_CLIENT_IS_2G_ONLY;
				}

				if (!(entry->Control_Flags & fBND_STRG_CLIENT_ALLOW_TO_CONNET_2G) &&
					_BndStrg_AllowStaConnect2G(table, entry) == TRUE)
				{
					entry->Control_Flags |= \
						fBND_STRG_CLIENT_ALLOW_TO_CONNET_2G;
				}
			}
		}
	}
}
#if 0
INT BndStrg_Init(struct wifi_dev *wdev)
{
	INT ret_val = BND_STRG_SUCCESS;
	PBND_STRG_CLI_TABLE table = P_BND_STRG_TABLE;

	DBGPRINT(RT_DEBUG_OFF, (YLW("%s()\n"), __FUNCTION__));

	ret_val = BndStrg_TableInit(table);

	BndStrg_SetInfFlags(wdev, table, TRUE);
	
	if (ret_val == BND_STRG_SUCCESS &&
		(table->b2GInfReady == TRUE && table->b5GInfReady == TRUE))
	{
		ret_val = BndStrg_Enable(table, 1);
	}

	if (ret_val != BND_STRG_SUCCESS)
	{
		BND_STRG_DBGPRINT(RT_DEBUG_ERROR,
					("Error in %s(), error code = %d!\n", __FUNCTION__, ret_val));
	}

	return ret_val;
}
#endif

INT BndStrg_Release(struct wifi_dev *wdev)
{
	INT ret_val = BND_STRG_SUCCESS;
	PBND_STRG_CLI_TABLE table = P_BND_STRG_TABLE;

	DBGPRINT(RT_DEBUG_OFF, (YLW("%s()\n"), __FUNCTION__));

	BndStrg_SetInfFlags(wdev, table, FALSE);

	if (table->bEnabled == TRUE)
		ret_val = BndStrg_Enable(table, 0);

	if ((table->b2GInfReady == FALSE && table->b5GInfReady == FALSE))
		ret_val = BndStrg_TableRelease(table);

	if (ret_val != BND_STRG_SUCCESS)
	{
		BND_STRG_DBGPRINT(RT_DEBUG_ERROR,
					("Error in %s(), error code = %d!\n", __FUNCTION__, ret_val));
	}

	return ret_val;
}


INT BndStrg_TableRelease(PBND_STRG_CLI_TABLE table)
{
	INT ret_val = BND_STRG_SUCCESS;

	if (table->bInitialized == FALSE)
		return BND_STRG_NOT_INITIALIZED;
	
	OS_NdisFreeSpinLock(&table->Lock);
	table->bInitialized = FALSE;

	if (ret_val != BND_STRG_SUCCESS)
	{
		BND_STRG_DBGPRINT(RT_DEBUG_ERROR,
					("Error in %s(), error code = %d!\n", __FUNCTION__, ret_val));
	}

	return ret_val;
}







static UINT32 BndStrg_GetEntryElapsedTime(PBND_STRG_CLI_ENTRY entry)
{
	ULONG Now = 0;
	UINT32 elapsed_time = 0;

	if (entry->jiffies) {
		RTMP_GetCurrentSystemTick(&Now);
		elapsed_time = jiffies_to_msecs(Now - entry->jiffies);
	}

	return elapsed_time;
}


static INT BndStrg_PrintEntryStatistics(PBND_STRG_ENTRY_STAT statistics)
{
#ifdef BND_STRG_DBG
	BND_STRG_DBGPRINT(RT_DEBUG_OFF,
						("Rssi = %4d, AuthReqCount = %3u\n",
						 statistics->Rssi, statistics->AuthReqCount));
#endif /* BND_STRG_DBG */
	return BND_STRG_SUCCESS;
}


INT BndStrg_PrintEntry(PBND_STRG_CLI_ENTRY entry)
{
#ifdef BND_STRG_DBG
	/* Update elapsed time */
	entry->elapsed_time = BndStrg_GetEntryElapsedTime(entry);
#if 1
	if (entry->bValid)
#endif
	{
#if 1
		PBND_STRG_CLI_TABLE table = P_BND_STRG_TABLE;
		if (MAC_ADDR_EQUAL(table->MonitorAddr, entry->Addr))
			BND_STRG_DBGPRINT(RT_DEBUG_OFF, (YLW("*")));
#endif
		BND_STRG_DBGPRINT(RT_DEBUG_OFF,
			("\tbValid = %u, Control_Flags = 0x%08x,"
			  " Addr=%02x:%02x:%02x:%02x:%02x:%02x, elapsed time = %u (ms)\n",
				entry->bValid, entry->Control_Flags,
				PRINT_MAC(entry->Addr), entry->elapsed_time));
		BND_STRG_DBGPRINT(RT_DEBUG_OFF,
			("\t\t2G:"));
		BndStrg_PrintEntryStatistics(&entry->statistics[0]);
		BND_STRG_DBGPRINT(RT_DEBUG_OFF,
			("\t\t5G:"));
		BndStrg_PrintEntryStatistics(&entry->statistics[1]);
	}
#endif /* BND_STRG_DBG */
	return BND_STRG_SUCCESS;
}

static INT _SetEnable(
			PBND_STRG_CLI_TABLE table,
			BOOLEAN enable)
{
	INT ret_val = BND_STRG_SUCCESS;
	//PBND_STRG_CLI_TABLE table = P_BND_STRG_TABLE;
	PRTMP_ADAPTER pAd = (PRTMP_ADAPTER) table->priv;
	//POS_COOKIE	pObj= (POS_COOKIE)pAd->OS_Cookie;
	//struct wifi_dev *wdev = &pAd->ApCfg.MBSSID[pObj->ioctl_if].wdev;

	DBGPRINT(RT_DEBUG_OFF, 
			("%s(): enable = %u\n", __FUNCTION__,  enable));

#if 0
	if (table->bInitialized == FALSE)
		ret_val = BndStrg_Init(wdev);

	ret_val = BndStrg_Enable(P_BND_STRG_TABLE, enable);

	BndStrg_SetInfFlags(wdev, P_BND_STRG_TABLE, enable);

#if 0 /* Maybe we don't need to release it before driver close */
	if (!enable)
		ret_val = BndStrg_Release(wdev);
#endif 
#else
	if (enable)
		ret_val = BndStrg_Init(pAd);
	else
		ret_val = BndStrg_Release(pAd);
#endif

	if (ret_val != BND_STRG_SUCCESS)
	{
		BND_STRG_DBGPRINT(RT_DEBUG_ERROR,
					("Error in %s(), error code = %d!\n", __FUNCTION__, ret_val));
	}

	return TRUE;
}


static INT _SetRssiDiff(
			PBND_STRG_CLI_TABLE table,
			CHAR RssiDiff)
{

	table->RssiDiff = RssiDiff;
	DBGPRINT(RT_DEBUG_OFF, 
			("%s(): RssiDiff = %d\n", __FUNCTION__, table->RssiDiff));

	return TRUE;
}


static INT _SetAgeTime(
			PBND_STRG_CLI_TABLE table,
			UINT32	Time)
{

	table->AgeTime = Time;
	DBGPRINT(RT_DEBUG_OFF, 
			("%s(): AgeTime = %u\n", __FUNCTION__, table->AgeTime));

	return TRUE;
}


static INT _SetHoldTime(
			PBND_STRG_CLI_TABLE table,
			UINT32	Time)
{

	table->HoldTime= Time;
	DBGPRINT(RT_DEBUG_OFF, 
			("%s(): HoldTime = %u\n", __FUNCTION__, table->HoldTime));

	return TRUE;
}


static INT _SetCheckTime(
			PBND_STRG_CLI_TABLE table,
			UINT32	Time)
{

	table->CheckTime_5G = Time;
	DBGPRINT(RT_DEBUG_OFF, 
			("%s(): CheckTime_5G = %u\n", __FUNCTION__, table->CheckTime_5G));

	return TRUE;
}


#ifdef BND_STRG_DBG
static INT _SetMntAddr(
			PBND_STRG_CLI_TABLE table,
			PUCHAR Addr)
{

	COPY_MAC_ADDR(table->MonitorAddr, Addr);
	DBGPRINT(RT_DEBUG_OFF, 
			("%s(): %02x:%02x:%02x:%02x:%02x:%02x\n",
			__FUNCTION__, PRINT_MAC(table->MonitorAddr)));

	return TRUE;
}
#endif /* BND_STRG_DBG */

/* We don't want to do too many things while handling Auth Req,
     so just let _BndStrg_AllowStaConnect2G() to check periodically.
     Hence, we only need to get the result in this function */
BOOLEAN BndStrg_AllowStaConnect2G(PBND_STRG_CLI_ENTRY entry)
{
	return ((entry) && (entry->Control_Flags & fBND_STRG_CLIENT_ALLOW_TO_CONNET_2G));
}


static BOOLEAN _BndStrg_AllowStaConnect2G(
		PBND_STRG_CLI_TABLE table,
		PBND_STRG_CLI_ENTRY entry)
{
	PBND_STRG_ENTRY_STAT statistics_2G = NULL, statistics_5G = NULL;

	if (!table)
	{
		DBGPRINT(RT_DEBUG_ERROR,
					("%s(): Error! table is NULL!\n", __FUNCTION__));
		return FALSE;
	}

	if (!entry)
	{
		DBGPRINT(RT_DEBUG_ERROR,
			(RED("%s(): Error! entry is NULL!\n"), __FUNCTION__));
		return FALSE;
	}

	if (IS_BND_STRG_DUAL_BAND_CLIENT(entry->Control_Flags))
	{
		statistics_2G = &entry->statistics[0];
		statistics_5G = &entry->statistics[1];
	
		/* Condition 1: 2G Rssi >> 5G Rssi */
		if (statistics_2G->Rssi & statistics_5G->Rssi)
		{
			CHAR RssiDiff = statistics_2G->Rssi - statistics_5G->Rssi;

			if (RssiDiff >= table->RssiDiff)
			{
#ifdef BND_STRG_QA
				BND_STRG_PRINTQAMSG(table, entry, 
				(YLW("check RssiDiff >= %d, client (%02x:%02x:%02x:%02x:%02x:%02x)"
				" is allowed to connect 2.4G.\n"),
				table->RssiDiff, PRINT_MAC(entry->Addr)));
#endif /* BND_STRG_QA */
				return TRUE;
			}
		}

		/* Condition 2: Client really don't want to connect 5G */
		if (entry->elapsed_time >= table->HoldTime &&
			statistics_5G->AuthReqCount == 0 &&
			statistics_2G->AuthReqCount != 0)
		{
#ifdef BND_STRG_QA
			BND_STRG_PRINTQAMSG(table, entry,
			(YLW("check elapsed_time >= %u (sec) and no auth req found in 5G,"
			" client (%02x:%02x:%02x:%02x:%02x:%02x) is allowed to connect 2.4G.\n"),
			table->HoldTime/1000, PRINT_MAC(entry->Addr)));
#endif /* BND_STRG_QA */
			return TRUE;
		}
	}
	else if (entry->Control_Flags & fBND_STRG_CLIENT_NOT_SUPPORT_HT)
	{
#ifdef BND_STRG_QA
		BND_STRG_PRINTQAMSG(table, entry,
		(YLW("check HT support: [legacy]. client (%02x:%02x:%02x:%02x:%02x:%02x)"
		" is allowed to connect 2.4G.\n"), PRINT_MAC(entry->Addr)));
#endif /* BND_STRG_QA */
		return TRUE;
	}
	else if (entry->Control_Flags & fBND_STRG_CLIENT_IS_2G_ONLY)
	{
#ifdef BND_STRG_QA
		BND_STRG_PRINTQAMSG(table, entry,
		(YLW("check 2.4G only. client (%02x:%02x:%02x:%02x:%02x:%02x)"
		" is allowed to connect 2.4G.\n"), PRINT_MAC(entry->Addr)));
#endif /* BND_STRG_QA */
		return TRUE;
	}

	return FALSE;
}

INT BndStrg_UpdateEntryStatistics(
		PBND_STRG_ENTRY_STAT statistics,
		CHAR Rssi,
		UINT8 FrameSubType)
{
	INT ret_val = BND_STRG_SUCCESS;

	statistics->Rssi = Rssi;
	switch (FrameSubType)
	{
		case APMT2_PEER_PROBE_REQ:
			break;
			
		case APMT2_PEER_AUTH_REQ:
			statistics->AuthReqCount++;
			break;
			
		default:
			BND_STRG_DBGPRINT(RT_DEBUG_OFF,
			("%s(): Unexpect FrameSubType (%u)\n",
			__FUNCTION__, FrameSubType));
			ret_val = BND_STRG_UNEXP;
			break;
	}

	return ret_val;
}


/* Check Probe/Auth Req to determine client's capability of 2.4g/5g */
BOOLEAN CheckConnectionReq(
		struct wifi_dev *wdev,
		PUCHAR pSrcAddr,
		UINT8 FrameType,
		PCHAR Rssi)
{
	INT ret_val = BND_STRG_SUCCESS;
	BOOLEAN IsInf2G = FALSE;
	PBND_STRG_CLI_ENTRY entry = NULL;
	PBND_STRG_ENTRY_STAT statistics = NULL;
	PBND_STRG_CLI_TABLE table = P_BND_STRG_TABLE;

	if (table->bInitialized == FALSE || table->bEnabled == FALSE)
		return BND_STRG_NOT_INITIALIZED;

	entry = BndStrg_TableLookup(table, pSrcAddr);

	if (entry == NULL)
	{
		ret_val = BndStrg_InsertEntry(table, pSrcAddr, &entry);
	}

	if (entry)
	{
		CHAR MaxRssi = Rssi[0], i;

		for ( i = 1; i < 3; i++)
		{
			if (Rssi[i])
				MaxRssi = max(MaxRssi, Rssi[i]);		
		}

		ASSERT(wdev->channel != 0);
		if (WMODE_CAP_2G(wdev->PhyMode) &&
			wdev->channel <= 14)
		{
			IsInf2G = TRUE;
			entry->Control_Flags |= fBND_STRG_CLIENT_SUPPORT_2G;
			statistics = &entry->statistics[0];
		}
		else if (WMODE_CAP_5G(wdev->PhyMode) &&
			(wdev->channel > 14))
		{
			entry->Control_Flags |= fBND_STRG_CLIENT_SUPPORT_5G;
			statistics = &entry->statistics[1];
		}
		else
		{
			ret_val = BND_STRG_UNEXP;
		}

		if (statistics)
		{
			BndStrg_UpdateEntryStatistics(statistics, MaxRssi, FrameType);
		}
#if 0 /* We can't use this to determine peer HT capability */
		if (wdev->DesiredHtPhyInfo.bHtEnable == 0)
		{
			entry->Control_Flags |= fBND_STRG_CLIENT_NOT_SUPPORT_HT;
		}
#endif

		if (bAuthCheck)
		{
			if (IsInf2G ==TRUE && FrameType == APMT2_PEER_AUTH_REQ)
			{
				*bAuthCheck = BndStrg_AllowStaConnect2G(entry);
#ifdef BND_STRG_QA
				if (*bAuthCheck == FALSE)
				{
					BND_STRG_PRINTQAMSG(table, entry,
					(RED("check 2.4G connection failed. client (%02x:%02x:%02x:%02x:%02x:%02x)"
					" is not allowed to connect 2.4G.\n"), PRINT_MAC(entry->Addr)));
				}
#endif /* BND_STRG_QA */
			}
			else
				*bAuthCheck = TRUE;
		}
	}

	if (ret_val != BND_STRG_SUCCESS)
	{
		DBGPRINT(RT_DEBUG_ERROR,
					("Error in %s(), error code = %d!\n", __FUNCTION__, ret_val));
	}

	return ret_val;
}

static BNDSTRG_OPS BndStrgOps = {
	.ShowTableInfo = ShowTableInfo,
	.ShowTableEntries = ShowTableEntries,
	.TableEntryAdd = BndStrg_InsertEntry,
	.TableEntryDel = BndStrg_DeleteEntry,
	.TableLookup = BndStrg_TableLookup,
	.CheckConnectionReq = CheckConnectionReq,
	.SetEnable = _SetEnable,
	.SetRssiDiff = D_SetRssiDiff,
	.SetRssiLow = D_SetRssiLow,
	.SetAgeTime = D_SetAgeTime,
	.SetHoldTime = D_SetHoldTime,
	.SetCheckTime = D_SetCheckTime,
#ifdef BND_STRG_DBG
	.SetMntAddr = D_SetMntAddr,
#endif /* BND_STRG_DBG */
};
#endif /* !BNDSTRG_DAEMON */
#endif /* BAND_STEERING */

