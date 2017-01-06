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
    tracelog_iparp.h

    Abstract:
    Miniport generic portion header file

    Revision History:
    Who         When          What
    --------    ----------    ----------------------------------------------
*/

#ifndef __TRACELOG_IPARP_FUNC_H___
#define __TRACELOG_IPARP_FUNC_H___

#ifdef TRACELOG_IPARP

#include "rt_config.h"

VOID dump_frame_icmp(UCHAR *p_hdr);

INT trace_iparp_set_dbg_tx(PRTMP_ADAPTER ad, RTMP_STRING *arg);
INT trace_iparp_set_dbg_rx(PRTMP_ADAPTER ad, RTMP_STRING *arg);
INT trace_iparp_set_tx_critical_chk_pnt(PRTMP_ADAPTER ad, RTMP_STRING *arg);
INT trace_iparp_set_rx_critical_chk_pnt(PRTMP_ADAPTER ad, RTMP_STRING *arg);
INT trace_iparp_handle_dbg_ctrl(PTRACE_TABLE_IPARP table);


INT trace_iparp_check_point(PTRACE_TABLE_IPARP table, UINT32 index, UINT32 value, UINT32 priv_data);
INT	trace_iparp_record_mcs_setting(PTRACE_TABLE_IPARP table, UINT32 index,  HTTRANSMIT_SETTING *transmit);
INT trace_iparp_init_table(PRTMP_ADAPTER ad, PTRACE_TABLE_IPARP table, UCHAR type);
INT trace_iparp_reset_table(PTRACE_TABLE_IPARP table);
INT trace_iparp_release_table(PTRACE_TABLE_IPARP table);
INT trace_iparp_find_empty_entry_index(PTRACE_TABLE_IPARP table, PUINT32 p_index);
INT trace_iparp_check_table(PTRACE_TABLE_IPARP table, UINT32 mask);
INT trace_iparp_dump_table(	PTRACE_TABLE_IPARP table);
INT trace_iparp_show_table_info(	PTRACE_TABLE_IPARP table);
INT trace_iparp_dump_fail_entry(PTRACE_TABLE_IPARP_ENTRY entry);
INT trace_iparp_check_entry(UINT32 timeout, BOOLEAN *pass, PTRACE_TABLE_IPARP table, PTRACE_TABLE_IPARP_ENTRY entry);
INT trace_iparp_clear_entry(PTRACE_TABLE_IPARP table, PTRACE_TABLE_IPARP_ENTRY entry);
INT trace_iparp_init_entry(PTRACE_TABLE_IPARP table, UINT32 index);
INT trace_iparp_fill_entry(PTRACE_TABLE_IPARP table, PTRACE_TABLE_IPARP_ENTRY entry, UINT8 wcid, PNDIS_PACKET packet, PUCHAR hdr_protype);
INT trace_iparp_set_entry(PTRACE_TABLE_IPARP	 table, PTRACE_TABLE_IPARP_ENTRY	entry);
INT trace_iparp_show_entry(PTRACE_TABLE_IPARP_ENTRY	entry);

#define P_TRACE_TABLE_TX(_ad)		&_ad->iparp_trace_tx
#define P_TRACE_TABLE_RX(_ad)		&_ad->iparp_trace_rx

#define RED(_text)	"\033[1;31m"_text"\033[0m"
#define GRN(_text)	"\033[1;32m"_text"\033[0m"
#define YLW(_text)	"\033[1;33m"_text"\033[0m"
#define BLUE(_text)	"\033[1;36m"_text"\033[0m"

#define TRACE_IPARP_SET_TX_STARTING_PNT(_ad, _wcid, _packet, _hdr_protype)		\
{		\
	TRACE_TABLE_IPARP_ENTRY entry;		\
	if (trace_iparp_fill_entry(P_TRACE_TABLE_TX(_ad), &entry, _wcid,  _packet, _hdr_protype) \
																	== TRACE_IPARP_SUCCESS)\
		trace_iparp_set_entry(P_TRACE_TABLE_TX(_ad), &entry);	\
}

#define TRACE_IPARP_SET_RX_STARTING_PNT(_ad, _wcid, _packet, _hdr_protype)		\
{		\
	TRACE_TABLE_IPARP_ENTRY entry;		\
	if (trace_iparp_fill_entry(P_TRACE_TABLE_RX(_ad), &entry, _wcid, _packet, _hdr_protype) \
																	== TRACE_IPARP_SUCCESS)\
		trace_iparp_set_entry(P_TRACE_TABLE_RX(_ad), &entry);	\
}

#if 1
/* index8: in case _index is not an "unsigned" value */
#define TRACE_IPARP_TX_CHK_PNT(_ad, _index, _CHK_PNT, _ps_mode)		\
{		\
	UINT8 index8 = (UINT8) _index;		\
	trace_iparp_check_point(P_TRACE_TABLE_TX(_ad), (UINT32) index8, _CHK_PNT, _ps_mode);\
}

#define TRACE_IPARP_RX_CHK_PNT(_ad, _index, _CHK_PNT, _ps_mode)		\
{		\
	UINT8 index8 = (UINT8) _index;		\
	trace_iparp_check_point(P_TRACE_TABLE_RX(_ad), (UINT32) index8, _CHK_PNT, _ps_mode);\
}

#else /* Debug */
/* index8: incase _index is not an "unsigned" value */
#define TRACE_IPARP_TX_CHK_PNT(_ad, _index, _CHK_PNT, _ps_mode)		\
{		\
	UINT8 index8 = (UINT8) _index;		\
	printk("\033[1;32m %s, %u, Tx check point: %x \033[0m\n", __FUNCTION__, __LINE__, _CHK_PNT);  /* Haipin Debug Print */	\
	trace_iparp_check_point(P_TRACE_TABLE_TX(_ad), (UINT32) index8, _CHK_PNT, _ps_mode);		\
}

#define TRACE_IPARP_RX_CHK_PNT(_ad, _index, _CHK_PNT, _ps_mode)		\
{		\
	UINT8 index8 = (UINT8) _index;		\
	printk("\033[1;32m %s, %u, Rx check point: %x \033[0m\n", __FUNCTION__, __LINE__, _CHK_PNT);  /* Haipin Debug Print */	\
	trace_iparp_check_point(P_TRACE_TABLE_RX(_ad), (UINT32) index8, _CHK_PNT, _ps_mode);		\
}
#endif

#define TRACE_IPARP_RECORD_TX_MCS_SETTING(_ad, _index, _p_mcs_setting)	\
{	\
	UINT8 index8 = (UINT8) _index;		\
	trace_iparp_record_mcs_setting(P_TRACE_TABLE_TX(_ad), (UINT32) index8, _p_mcs_setting);	\
}

#define TRACE_IPARP_RECORD_RX_MCS_SETTING(_ad, _index, _p_mcs_setting)	\
{	\
	UINT8 index8 = (UINT8) _index;		\
	trace_iparp_record_mcs_setting(P_TRACE_TABLE_RX(_ad), (UINT32) index8, _p_mcs_setting);	\
}

#endif /* TRACELOG_IPARP */

#endif  /* __TRACELOG_IPARP_FUNC_H___ */

