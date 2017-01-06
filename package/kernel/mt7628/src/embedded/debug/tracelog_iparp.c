/*
 ***************************************************************************
 * Ralink Tech Inc.
 * 4F, No. 2 Technology 5th Rd.
 * Science-based Industrial Park
 * Hsin-chu, Taiwan, R.O.C.
 *
 * (c) Copyright 2002-2004, Ralink Technology, Inc.
 *
 * All rights reserved. Ralink's source code is an unpublished work and the
 * use of a copyright notice does not imply otherwise. This source code
 * contains confidential trade secret material of Ralink Tech. Any attemp
 * or participation in deciphering, decoding, reverse engineering or in any
 * way altering the source code is stricitly prohibited, unless the prior
 * written consent of Ralink Technology, Inc. is obtained.
 ***************************************************************************

    Module Name:
    tracelog_iparp.c

    Abstract:
    Miniport generic portion header file

    Revision History:
    Who         When          What
    --------    ----------    ----------------------------------------------
*/
#ifdef TRACELOG_IPARP

#include "rt_config.h"

#if 0
RTMP_OS_FD	 trace_log_file = NULL;
#define LOG_FILE 	"/etc_ro/trace_log.txt"
#endif

static const CHECK_POINT_LIST tx_chk_pnt_list[] =
{
	{P0,  "RTMPCheckEtherType"}, /* critical point */
	{P1,  ""},
	{P2,  ""},
	{P3,  ""},
	{P4,  ""},
	{P5,  ""},
	{P6,  ""},
	{P7,  "APHardTransmit()->Normal Path"}, /* critical point */
	{P8,  ""},
	{P9,  ""},
	{P10, "rtmp_enq_req"}, /* critical point */
	{P11, ""},
	{P12, "RTMPDeQueuePacket()->Normal Path"}, /* critical point */
	{P13, ""},
	{P14, ""},
	{P15, ""},
	{P16, ""},
	{P17, ""},
	{P18, ""},
	{P19, ""},
	{P20, ""},
	{P21, ""},
	{P22, ""},
	{P23, ""},
	{P24, ""},
	{P25, ""},
	{P26, ""},
	{P27, ""},
	{P28, ""},
	{P29, ""},
	{P30, ""},
	{P31, ""},	
};
UCHAR NUM_tx_chk_pnt_list = (sizeof(tx_chk_pnt_list) / sizeof(CHECK_POINT_LIST));

static const CHECK_POINT_LIST rx_chk_pnt_list[] =
{
	{P0,  "rx_data_frm_announce"}, /* critical point */
	{P1,  ""},
	{P2,  ""},
	{P3,  ""},
	{P4,  ""},
	{P5,  ""},
	{P6,  ""},
	{P7,  ""},
	{P8,  "Duplicate"}, /* assigned */
	{P9,  ""},
	{P10, ""},
	{P11, "Announce_or_Forward_802_3_Packet()->Normal Path (entry point)"}, /* critical point */
	{P12, ""},
	{P13, ""},
	{P14, ""},
	{P15, ""},
	{P16, ""},
	{P17, ""},
	{P18, ""},
	{P19, ""},
	{P20, ""},
	{P21, ""},
	{P22, ""},
	{P23, ""},
	{P24, ""},
	{P25, ""},
	{P26, ""},
	{P27, ""},
	{P28, ""},
	{P29, ""},
	{P20, ""},
	{P31, ""},
};
UCHAR NUM_rx_chk_pnt_list = (sizeof(rx_chk_pnt_list) / sizeof(CHECK_POINT_LIST));

static const CODE_DESCRP arp_code[] = {
 		{FALSE,  0, "(Reserved)"},
 		{TRUE ,  1, "(REQUEST)"},
 		{TRUE ,  2, "(REPLY)"},
 		{FALSE,  3, "(request Reverse)"},
 		{FALSE,  4, "(reply Reverse)"},
 		{FALSE,  5, "(DRARP-Request)"},
 		{FALSE,  6, "(DRARP-Reply)"},
 		{FALSE,  7, "(DRARP-Error)"},
 		{FALSE,  8, "(InARP-Request)"},
 		{FALSE,  9, "(InARP-Reply)"},
 		{FALSE, 10, "(ARP-NAK)"},
 		{FALSE, 11, "(MARS-Request)"},
 		{FALSE, 12, "(MARS-Multi)"},
 		{FALSE, 13, "(MARS-MServ)"},
 		{FALSE, 14, "(MARS-Join)"},
 		{FALSE, 15, "(MARS-Leave)"},
 		{FALSE, 16, "(MARS-NAK)"},
 		{FALSE, 17, "(MARS-Unserv)"},
 		{FALSE, 18, "(MARS-SJoin)"},
 		{FALSE, 19, "(MARS-SLeave)"},
 		{FALSE, 20, "(MARS-Grouplist-Request)"},
 		{FALSE, 21, "(MARS-Grouplist-Reply)"},
 		{FALSE, 22, "(MARS-Redirect-Map)"},
 		{FALSE, 23, "(MAPOS-UNARP)"},
 		{FALSE, 24, "(OP_EXP1)"},
 		{FALSE, 25, "(OP_EXP2)"},
};

static const CODE_DESCRP icmp_code[] = {
		{TRUE ,  0, "(Echo Reply)"},
		{FALSE,  1, "(Unassigned)"},
		{FALSE,  2, "(Unassigned)"},
		{FALSE,  3, "(Destination Unreachable)"},
		{FALSE,  4, "(Source Quench)"},
		{FALSE,  5, "(Redirect)"},
		{FALSE,  6, "(Alternate Host Address)"},
		{FALSE,  7, "(Unassigned)"},
		{TRUE ,  8, "(Echo Req)"},
		{FALSE,  9, "(Router Advertisement)"},
		{FALSE, 10, "(Router Selection)"},
		{FALSE, 11, "(Time Exceeded)"},
		{FALSE, 12, "(Parameter Problem)"},
		{FALSE, 13, "(Timestamp)"},
		{FALSE, 14, "(Timestamp Reply)"},
		{FALSE, 15, "(Information Request)"},
		{FALSE, 16, "(Information Reply)"},
		{FALSE, 17, "(Address Mask Request)"},
		{FALSE, 18, "(Address Mask Reply)"},
		{FALSE, 19, "(Reserved)"},
		{FALSE, 20, "(Reserved)"},
		{FALSE, 21, "(Reserved)"},
		{FALSE, 22, "(Reserved)"},
		{FALSE, 23, "(Reserved)"},
		{FALSE, 24, "(Reserved)"},
		{FALSE, 25, "(Reserved)"},
		{FALSE, 26, "(Reserved)"},
		{FALSE, 27, "(Reserved)"},
		{FALSE, 28, "(Reserved)"},
		{FALSE, 29, "(Reserved)"},
		{FALSE, 30, "(Traceroute)"},
		{FALSE, 31, "(Datagram Conversion Error)"},
		{FALSE, 32, "(Mobile Host Redirect)"},
		{FALSE, 33, "(IPv6 Where-Are-You)"},
		{FALSE, 34, "(IPv6 I-Am-Here)"},
		{FALSE, 35, "(Mobile Registration Request)"},
		{FALSE, 36, "(Mobile Registration Reply)"},
		{FALSE, 37, "(Domain Name Request)"},
		{FALSE, 38, "(Domain Name Reply)"},
		{FALSE, 39, "(SKIP)"},
		{FALSE, 40, "(Photuris)"},
};
UCHAR NUM_icmp_code = (sizeof(icmp_code) / sizeof(CODE_DESCRP));

// TODO: Check branch_point and bad_point
// TODO: Add statistic counter to loss packet (packets that checked fail )
// TODO: Check point auto registration
// TODO: Dump Table, valid (instead of all)

/*******     UTILS    *******/
#if 0
static UINT32 cmd_util_hex_get(CHAR **ppArgv)
{
	CHAR buf[3], *pNum;
	UINT32 ID;
	UCHAR Value;


	pNum = (*ppArgv);

	buf[0] = 0x30;
	buf[1] = 0x30;
	buf[2] = 0;

	for(ID=0; ID<sizeof(buf)-1; ID++)
	{
		if ((*pNum == '_') || (*pNum == 0x00))
			break;
		/* End of if */

		pNum ++;
	} /* End of for */

	if (ID == 0)
		return 0; /* argument length is too small */
	/* End of if */

	if (ID >= 2)
		memcpy(buf, (*ppArgv), 2);
	else
	    buf[1] = (**ppArgv);
	/* End of if */

	(*ppArgv) += ID;
	if ((**ppArgv) == '_')
		(*ppArgv) ++; /* skip _ */
	/* End of if */

	AtoH(buf, &Value, 1);
	return (UINT32)Value;
}


static UINT32 cmd_util_num_get(CHAR **ppArgv)
{
	CHAR buf[20], *pNum;
	UINT32 ID;

	pNum = (*ppArgv);

	for(ID=0; ID<sizeof(buf)-1; ID++)
	{
		if ((*pNum == '_') || (*pNum == 0x00))
			break;
		/* End of if */

		pNum ++;
	} /* End of for */

	if (ID == sizeof(buf)-1)
		return 0; /* argument length is too large */
	/* End of if */

	memcpy(buf, (*ppArgv), ID);
	buf[ID] = 0x00;

	*ppArgv += ID+1; /* skip _ */

	return simple_strtol(buf, 0, 10);
}
#endif

/*******     IOCTL    *******/
INT trace_iparp_set_dbg_tx(
	IN	PRTMP_ADAPTER		ad,
	IN	RTMP_STRING			*arg)
{
	PTRACE_TABLE_IPARP table = P_TRACE_TABLE_TX(ad);

	table->dbg_ctrl= simple_strtol((RTMP_STRING *) arg, 0, 16);
	DBGPRINT(RT_DEBUG_OFF, ("%s(): dbg_ctrl = 0x%08x\n",
				__FUNCTION__, table->dbg_ctrl));

	trace_iparp_handle_dbg_ctrl(table);

	return TRUE;
}


INT trace_iparp_set_dbg_rx(
	IN	PRTMP_ADAPTER		ad,
	IN	RTMP_STRING			*arg)
{
	PTRACE_TABLE_IPARP table = P_TRACE_TABLE_RX(ad);

	table->dbg_ctrl= simple_strtol((RTMP_STRING *) arg, 0, 16);
	DBGPRINT(RT_DEBUG_OFF, ("%s(): dbg_ctrl = 0x%08x\n",
				__FUNCTION__, table->dbg_ctrl));

	trace_iparp_handle_dbg_ctrl(table);

	return TRUE;
}


INT trace_iparp_set_tx_critical_chk_pnt(
	IN	PRTMP_ADAPTER		ad,
	IN	RTMP_STRING			*arg)
{
	PTRACE_TABLE_IPARP table = P_TRACE_TABLE_TX(ad);

	table->critical_chk_points = simple_strtol((RTMP_STRING *) arg, 0, 16);
	DBGPRINT(RT_DEBUG_OFF, ("%s(): critical_chk_points = 0x%08x\n",
				__FUNCTION__, table->critical_chk_points));

	return TRUE;
}


INT trace_iparp_set_rx_critical_chk_pnt(
	IN	PRTMP_ADAPTER		ad,
	IN	RTMP_STRING			*arg)
{
	PTRACE_TABLE_IPARP table = P_TRACE_TABLE_RX(ad);

	table->critical_chk_points = simple_strtol((RTMP_STRING *) arg, 0, 16);
	DBGPRINT(RT_DEBUG_OFF, ("%s(): critical_chk_points = 0x%08x\n",
				__FUNCTION__, table->critical_chk_points));

	return TRUE;
}
/*******  End of  IOCTL    *******/


INT trace_iparp_handle_dbg_ctrl(PTRACE_TABLE_IPARP table)
{

	if (table->dbg_ctrl & TRACE_IPARP_DBG_DONT_CHECK_TABLE)
	{};

	if (table->dbg_ctrl & TRACE_IPARP_DBG_DUMP_TABLE)
		trace_iparp_dump_table(table);
	
	if (table->dbg_ctrl & TRACE_IPARP_DBG_DONT_CLEAR_ENTRY)
	{};

	if (table->dbg_ctrl & TRACE_IPARP_DBG_TABLE_INFO)
		trace_iparp_show_table_info(table);

	return TRACE_IPARP_SUCCESS;
}


VOID dump_frame_icmp(UCHAR *p_hdr)
{
	DUMP_ICMP icmp_info;
#if 0
	UINT16	type_len = (*(p_hdr +12) << 8 ) | *(p_hdr + 13);
	UINT8	protocol = *(p_hdr + 23); 
	if (type_len == 0x0
		800 && protocol == 0x01) /* IP && ICMP */
#endif
	{
		icmp_info.p_dst_mac = p_hdr;
		icmp_info.p_src_mac = p_hdr + 6;
		icmp_info.p_src_ip = p_hdr + 26;
		icmp_info.p_dst_ip = p_hdr + 30;
		icmp_info.p_icmp_type = p_hdr + 34;
		icmp_info.seq_num = (*(p_hdr + 40) << 8) | *(p_hdr + 41);

		DBGPRINT(RT_DEBUG_OFF, 
					("\033[1;32m %u.%u.%u.%u (%02x:%02x:%02x:%02x:%02x:%02x) --> %u.%u.%u.%u (%02x:%02x:%02x:%02x:%02x:%02x),  ping %s,  seq = %u  \033[0m\t",
					icmp_info.p_src_ip[0], icmp_info.p_src_ip[1], icmp_info.p_src_ip[2], icmp_info.p_src_ip[3],
					icmp_info.p_src_mac[0], icmp_info.p_src_mac[1], icmp_info.p_src_mac[2], icmp_info.p_src_mac[3], icmp_info.p_src_mac[4], icmp_info.p_src_mac[5],
					icmp_info.p_dst_ip[0], icmp_info.p_dst_ip[1], icmp_info.p_dst_ip[2], icmp_info.p_dst_ip[3],
					icmp_info.p_dst_mac[0], icmp_info.p_dst_mac[1], icmp_info.p_dst_mac[2], icmp_info.p_dst_mac[3], icmp_info.p_dst_mac[4], icmp_info.p_dst_mac[5],
					*(icmp_info.p_icmp_type) == 0x8 ? "req": (*(icmp_info.p_icmp_type) == 0x0 ? "reply":"null"),
					icmp_info.seq_num));
	}
}


INT trace_iparp_check_point(PTRACE_TABLE_IPARP table, UINT32 index, UINT32 check_point, UINT32 priv_data)
{	
	if (index != INVALID_INDEX)
	{
		/*
			if this ASSERT happened, it could be a Rx check point is set to a tx table,
			or a Tx check point is set to a Rx table. use debug macro of
			"TRACE_IPARP_TX_CHK_PNT" and "TRACE_IPARP_RX_CHK_PNT" to check.
		*/
		ASSERT(table->entry[index-1].valid == TRUE);

		NdisAcquireSpinLock(&table->lock);
		table->entry[index-1].check_points |= check_point;
		table->entry[index-1].priv_data |= priv_data;
		NdisReleaseSpinLock(&table->lock);
#if 0
		trace_iparp_show_entry(&table->entry[index-1]);
#endif
	}
	return TRACE_IPARP_SUCCESS;
}


INT	trace_iparp_record_mcs_setting(PTRACE_TABLE_IPARP table, UINT32 index,  HTTRANSMIT_SETTING *transmit)
{
	if (index != INVALID_INDEX)
	{
		ASSERT(table->entry[index-1].valid == TRUE);

		NdisAcquireSpinLock(&table->lock);
		NdisCopyMemory(&table->entry[index-1].mcs_setting, transmit, sizeof(HTTRANSMIT_SETTING));
		NdisReleaseSpinLock(&table->lock);
	}
	return TRACE_IPARP_SUCCESS;
}


INT trace_iparp_init_table(PRTMP_ADAPTER ad, PTRACE_TABLE_IPARP table, UCHAR type)
{
	/*NdisZeroMemory(table, sizeof(TRACE_TABLE_IPARP));*/
	NdisAllocateSpinLock(ad, &table->lock);
	trace_iparp_reset_table(table);
	switch (type) 
	{
		case TX_TRACE:
			table->type = type;
			table->chk_pnt_list = tx_chk_pnt_list;
			table->list_size = NUM_tx_chk_pnt_list;
			table->critical_chk_points = TRACE_IPARP_TX_MASK;
			break;
		case RX_TRACE:
			table->type = type;
			table->chk_pnt_list = rx_chk_pnt_list;
			table->list_size = NUM_rx_chk_pnt_list;
			table->critical_chk_points = TRACE_IPARP_RX_MASK;
			break;
		default:
			DBGPRINT(RT_DEBUG_ERROR, 
						("%s(): Error! Table type (%u) is invalid!\n",
						__FUNCTION__, type));
			break;
	}

#if 0
if (trace_log_file == NULL)
{

	trace_log_file = RtmpOSFileOpen(LOG_FILE, O_WRONLY, 0);

	if (IS_FILE_OPEN_ERR(trace_log_file))
	{
		DBGPRINT(RT_DEBUG_ERROR, 
					(RED("%s(): Error in open file: %s\n"),
					__FUNCTION__, LOG_FILE));		
		trace_log_file = NULL;
	}	
}
#endif

	return TRACE_IPARP_SUCCESS;
}


INT trace_iparp_reset_table(PTRACE_TABLE_IPARP table)
{
	INT i;

	if (!table)
	{
		DBGPRINT(RT_DEBUG_ERROR,
					("%s(): Table is null !\n",
					__FUNCTION__));
		return TRACE_IPARP_INVALID_ARG;
	}

	table->size = 0;
	table->type = 0;
	table->dbg_ctrl = 0;
	table->chk_pnt_list = NULL;
	table->list_size = 0;

	for ( i = 0; i < MAX_ENTRY_SIZE; i++)
		trace_iparp_clear_entry(table, &table->entry[i]);

	return TRACE_IPARP_SUCCESS;
}

INT trace_iparp_release_table(PTRACE_TABLE_IPARP table)
{
	// TODO: Call this function
	NdisFreeSpinLock(&table->lock);
#if 0
	if (trace_log_file)
		RtmpOSFileClose(trace_log_file);
#endif 
	return TRACE_IPARP_SUCCESS;
}

INT trace_iparp_find_empty_entry_index(
		PTRACE_TABLE_IPARP table,
		PUINT32 p_index)
{
	INT i;

	*p_index = INVALID_INDEX;
	
	NdisAcquireSpinLock(&table->lock);
	for ( i = 0; i < MAX_ENTRY_SIZE; i++)
	{
		if (table->entry[i].valid == FALSE)
		{
			*p_index = i+1; /* valid index starts from 1~255 */
			break;
		}
	}
	NdisReleaseSpinLock(&table->lock);

	if (*p_index == INVALID_INDEX)
	{
		DBGPRINT(RT_DEBUG_WARN,
					("%s(): Table is full !\n",
					__FUNCTION__));
		return TRACE_IPARP_TABLE_FULL;
	}

	return TRACE_IPARP_SUCCESS;
}


/* 
	mask: determine check how many checkpoint is considered PASS
	timeout: if entry is timed out and not get checked pass,
			then this is an fail entry. (unit in second)
*/
INT trace_iparp_check_table(
	PTRACE_TABLE_IPARP table,
	UINT32 mask)
{
	INT i;

	if (!table)
	{
		DBGPRINT(RT_DEBUG_ERROR,
					("%s(): Table is null !\n",
					__FUNCTION__));
		return TRACE_IPARP_INVALID_ARG;
	}

	if (table->dbg_ctrl & TRACE_IPARP_DBG_DONT_CHECK_TABLE)
		return TRACE_IPARP_DBG;
	
	for ( i = 0; i < MAX_ENTRY_SIZE; i++)
	{
		BOOLEAN pass = FALSE;

		trace_iparp_check_entry(
				TRACE_IPARP_TIMEOUT,
				&pass,
				table,
				&table->entry[i]);

		if (pass == FALSE) {
			trace_iparp_dump_fail_entry(&table->entry[i]);
		}
	}
	
	return TRACE_IPARP_SUCCESS;
}


INT trace_iparp_dump_table(	PTRACE_TABLE_IPARP table)
{
	INT i;

	for ( i = 0; i < MAX_ENTRY_SIZE; i++)
	{
		if (table->entry[i].valid == TRUE)
			trace_iparp_show_entry(&table->entry[i]);
	}

	return TRACE_IPARP_SUCCESS;
}


INT trace_iparp_show_table_info(	PTRACE_TABLE_IPARP table)
{
	DBGPRINT(RT_DEBUG_OFF,
			("\n"
			"\tsize = %u\n"
			 "\ttype = %u\n"
			 "\tdbg_ctrl = %x\n"
			 "\tcritical_chk_points = %x\n"
			 "\tbranch_chk_points = %x\n"
			 "\tbad_chk_points = %x\n"
			 "\tlist_size = %x\n",
			  table->size,  table->type, table->dbg_ctrl,
			  table->critical_chk_points, table->branch_chk_points,
			  table->bad_chk_points, table->list_size));

	return TRACE_IPARP_SUCCESS;
}


INT trace_iparp_dump_fail_entry(
	PTRACE_TABLE_IPARP_ENTRY entry)
{
	INT i;
	UINT32 mask;
	const CHAR* word[] = {"ARP", "ICMP", "REQ", "RSP", "n/a"};
	const CHAR *s_protocol, *s_type;
	PTRACE_TABLE_IPARP table = NULL;

	if (!entry)
	{
		DBGPRINT(RT_DEBUG_ERROR,
					("%s(): entry is null !\n",
					__FUNCTION__));
		return TRACE_IPARP_INVALID_ARG;
	}

	// TODO: LOCK?
	if (entry->protocol_type == ETH_P_ARP) {
		s_protocol = word[0];
#if 0
		if (entry->icmparp_code == 0x1)
			s_type = word[2];
		else if (entry->icmparp_code == 0x2)
			s_type = word[3];
		else
			s_type = word[4];
#else
		s_type = arp_code[entry->icmparp_code].descrption;
#endif
	} else if (entry->protocol_type == ETH_P_IP){
		s_protocol = word[1];
#if 0
		if (entry->icmparp_code == 0x8)
			s_type = word[2];
		else if (entry->icmparp_code == 0x0)
			s_type = word[3];
		else
			s_type = icmp_code[entry->icmparp_code].descrption;
#else
		s_type = icmp_code[entry->icmparp_code].descrption;
#endif
	} else {
		DBGPRINT(RT_DEBUG_ERROR,
					("%s(): unexpected condition!\n",
					__FUNCTION__));
		trace_iparp_show_entry(entry);
		return TRACE_IPARP_UNEXP;
	}

	table = (PTRACE_TABLE_IPARP) entry->table;
	mask = table->critical_chk_points;

	if (entry->wcid == 0)
	{
#if 0 /* find out the mac address to make this more meaningful. */
		DBGPRINT(RT_DEBUG_ERROR,
				("\tA "YLW("%s %s")" is trying to reach a none exist client (wcid = %u).\n",
				s_protocol, s_type, entry->wcid));
#endif
		return TRACE_IPARP_CLIENT_NOT_EXIST;
	}

	DBGPRINT(RT_DEBUG_ERROR,
			("================%4s=================\n",
			(table->type == TX_TRACE) ? "TX" :
			(table->type == RX_TRACE) ? "RX" : "n/a"));

	DBGPRINT(RT_DEBUG_ERROR,
			("\tCheck critical points (0x%08x):\n"
			"\tA "BLUE("%s %s")" failed to reach the following check points:\n",
			mask, s_protocol, s_type));

	for ( i = 0; i < NUM_MAX_CHK_POINT; i++)
	{
		UINT32 check_point = (1 << i);
		
		if ((check_point & mask) &&
			!(check_point & entry->check_points))
		{
			DBGPRINT(RT_DEBUG_ERROR,
				(RED("\t\tCheck Point %d: %s\n"), i,
				table->chk_pnt_list[i].descrption));
		}
	}

#if 1

	DBGPRINT(RT_DEBUG_ERROR,
			("\tThe "BLUE("%s %s")" reached the following check points:\n",
			s_protocol, s_type));
	
	for ( i = 0; i < NUM_MAX_CHK_POINT; i++)
	{
		UINT32 check_point = (1 << i);
		if ( check_point & entry->check_points)
		{
#if 0
			DBGPRINT(RT_DEBUG_ERROR,
				(GRN("\t\tCheck Point %d  "), i));
#else
			DBGPRINT(RT_DEBUG_ERROR,
				(GRN("\t\tCheck Point %d: %s\n"), i,
				table->chk_pnt_list[i].descrption));
#endif
		}
	}
	DBGPRINT(RT_DEBUG_ERROR, ("\n"));

#endif

	trace_iparp_show_entry(entry);

	DBGPRINT(RT_DEBUG_ERROR,
			("=====================================\n"));

	return TRACE_IPARP_SUCCESS;
}


INT trace_iparp_check_entry(
	UINT32 timeout,
	BOOLEAN *pass,
	PTRACE_TABLE_IPARP table,
	PTRACE_TABLE_IPARP_ENTRY entry)
{
	ULONG Now = 0;
	UINT32 elapsed_time;
	BOOLEAN expired = FALSE;
	BOOLEAN valid = FALSE;
	UINT32 mask;

	if (!table || !entry || !pass)
	{
		DBGPRINT(RT_DEBUG_ERROR,
					("%s(): Invalid arguments!\n",
					__FUNCTION__));
		return TRACE_IPARP_INVALID_ARG;
	}

	*pass = TRUE; /* defaut */
	mask = table->critical_chk_points;

	RTMP_GetCurrentSystemTick(&Now);
	
	elapsed_time = jiffies_to_msecs(Now - entry->jiffies);

	NdisAcquireSpinLock(&table->lock);
	if ((valid = entry->valid) == TRUE)
	{
		expired = (elapsed_time > timeout) ?
				TRUE : FALSE;

		if (expired == TRUE) {
			*pass = ((entry->check_points & mask) == mask) ?
				TRUE : FALSE;	
			entry->elapsed_time = elapsed_time;
		} else {
		/* we don't check entry that hasn't expired; just return pass */
			//*pass = TRUE;
		}
	}
	NdisReleaseSpinLock(&table->lock);

	if (!(valid == FALSE || expired == FALSE))
		trace_iparp_clear_entry(table, entry);

	return TRACE_IPARP_SUCCESS;
}


INT trace_iparp_clear_entry(
	PTRACE_TABLE_IPARP table,
	PTRACE_TABLE_IPARP_ENTRY	entry)
{
	if (!table || !entry)
	{
		DBGPRINT(RT_DEBUG_ERROR,
					("%s(): Invalid table or entry!\n",
					__FUNCTION__));
		return TRACE_IPARP_INVALID_ARG;
	}

	if (table->dbg_ctrl & TRACE_IPARP_DBG_DONT_CLEAR_ENTRY)
		return TRACE_IPARP_DBG;

	NdisAcquireSpinLock(&table->lock);
	entry->valid = FALSE;
	NdisReleaseSpinLock(&table->lock);

	return TRACE_IPARP_SUCCESS;
}


INT trace_iparp_init_entry(	
	PTRACE_TABLE_IPARP	table,
	UINT32 index	)
{
	if (!table)
	{
		DBGPRINT(RT_DEBUG_ERROR,
					("%s(): Table is null !\n",
					__FUNCTION__));
		return TRACE_IPARP_INVALID_ARG;
	}
	
	NdisAcquireSpinLock(&table->lock);
	NdisZeroMemory(&table->entry[index-1],
					sizeof(TRACE_TABLE_IPARP_ENTRY));
	NdisReleaseSpinLock(&table->lock);

	return TRACE_IPARP_SUCCESS;
}


INT trace_iparp_fill_entry(
	PTRACE_TABLE_IPARP	table,
	PTRACE_TABLE_IPARP_ENTRY entry,
	UINT8	wcid,
	PNDIS_PACKET packet,
	PUCHAR	hdr_protype)
{
	UINT16  type_len;
	PUCHAR L3_hhr = NULL;

	if( (!entry) || (!packet) || (!hdr_protype)) {
		DBGPRINT(RT_DEBUG_ERROR,
					("%s(): invalid input!entry(%p), packet(%p), hdr_protype(%p)\n",
					__FUNCTION__, entry, packet, hdr_protype));
		return TRACE_IPARP_INVALID_ARG;
	}else if (!packet) {
		DBGPRINT(RT_DEBUG_ERROR,
					("%s(): packet is null !\n",
					__FUNCTION__));
		return TRACE_IPARP_INVALID_ARG;
	}

#if 1 /* since INVALID_INDEX is 0, we don't need to write this every time */
	RTMP_SET_PACKET_TRACEIPARP(packet, INVALID_INDEX);
#endif

	if (!hdr_protype) {
		DBGPRINT(RT_DEBUG_ERROR,
					("%s(): hdr_protype is null !\n",
					__FUNCTION__));
		return TRACE_IPARP_INVALID_ARG;
	}

	L3_hhr = hdr_protype + 2;

	type_len = OS_NTOHS(get_unaligned((PUINT16)(hdr_protype)));

	NdisZeroMemory(entry, sizeof(TRACE_TABLE_IPARP_ENTRY));

	if (type_len == ETH_P_ARP) {
		entry->protocol_type =  type_len;
		entry->icmparp_code = \
				(UINT32) OS_NTOHS(*((UINT16 *)(L3_hhr + 6))); /* ARP op */
		if (arp_code[entry->icmparp_code].interested != TRUE)
			{return TRACE_IPARP_NO_TRACE;}
		/*entry->seq_num = */
	} else if (type_len == ETH_P_IP) {
		UINT8 protocol = *(L3_hhr + 9);
		UINT8 icmp_type = *(L3_hhr + 20);

		if (protocol == 0x1 &&  /* ICMP */
			(icmp_code[icmp_type].interested == TRUE))
		{
			entry->protocol_type =  type_len;
			entry->icmparp_code = icmp_type;
			entry->seq_num = \
				OS_NTOHS(get_unaligned((PUINT16)(L3_hhr + 26)));
		} else {
			{return TRACE_IPARP_NO_TRACE;}
		}
	} else {
		{return TRACE_IPARP_NO_TRACE;}
	}

	entry->valid =  TRUE;
	RTMP_GetCurrentSystemTick(&entry->jiffies);
	entry->table = (PVOID) table;
	entry->packet = packet;
	entry->wcid = wcid;

	return TRACE_IPARP_SUCCESS;
}


INT trace_iparp_set_entry(
	PTRACE_TABLE_IPARP			table,
	PTRACE_TABLE_IPARP_ENTRY	entry)
{
	UINT32 index;
	
	if (!table || !entry)
	{
		DBGPRINT(RT_DEBUG_ERROR,
					("%s(): Invalid table or entry!\n",
					__FUNCTION__));
		return TRACE_IPARP_INVALID_ARG;
	}
	
	if (trace_iparp_find_empty_entry_index(table, &index)
		!= TRACE_IPARP_SUCCESS)
	{
		return TRACE_IPARP_TABLE_FULL;
	}

	trace_iparp_init_entry(table, index);

	NdisAcquireSpinLock(&table->lock);
	NdisCopyMemory(&table->entry[index-1], entry,
					sizeof(TRACE_TABLE_IPARP_ENTRY));
	NdisReleaseSpinLock(&table->lock);

	if (entry->packet) {
		RTMP_SET_PACKET_TRACEIPARP(entry->packet, index);
	}
	else {
		DBGPRINT(RT_DEBUG_ERROR,
					("%s(): packet is null!\n",
					__FUNCTION__));
		return TRACE_IPARP_INVALID_ARG;
	}
		
	return TRACE_IPARP_SUCCESS;
}

#if 0
INT trace_iparp_get_entry()
{
	return TRACE_IPARP_SUCCESS;
}
#endif


INT trace_iparp_show_entry(PTRACE_TABLE_IPARP_ENTRY	entry)
{
	if (!entry) {
		DBGPRINT(RT_DEBUG_ERROR,
					("%s(): entry is null !\n",
					__FUNCTION__));
		return TRACE_IPARP_INVALID_ARG;
	}

	DBGPRINT(RT_DEBUG_ERROR,
		("\tvalid = %u, wcid = %u, elapsed_time = %u ms, checkpoints = 0x%08x\n"
		"\tprotocol_type = 0x%04x, seq_num = %u, icmparp_code = 0x%x, packet = %p, mcs=%u, bw=%u\n",
		 entry->valid, entry->wcid, entry->elapsed_time, entry->check_points,
		 entry->protocol_type, entry->seq_num, entry->icmparp_code, entry->packet,
		 entry->mcs_setting.field.MCS, entry->mcs_setting.field.BW));
#if 0

{

	if (trace_log_file)
	{
		RtmpOSFileWrite(trace_log_file, (char*) entry, sizeof(TRACE_TABLE_IPARP_ENTRY));
	}
}
#endif

	return TRACE_IPARP_SUCCESS;
}

#endif /* TRACELOG_IPARP */

