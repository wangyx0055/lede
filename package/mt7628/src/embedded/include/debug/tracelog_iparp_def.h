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

#ifndef __TRACELOG_IPARP_DEF_H___
#define __TRACELOG_IPARP_DEF_H___

#ifdef TRACELOG_IPARP

#include "rt_config.h"

#define MAX_ENTRY_SIZE	255
#define MAX_DESCRP_LEN	255
/* valid index starts from 1~255 */
#define INVALID_INDEX	0
#define TRACE_IPARP_CB	25
#define TRACE_IPARP_TIMEOUT 4000 /* ms */
#define NUM_MAX_CHK_POINT		32

enum TRACE_IPARP_RETURN_CODE {
	TRACE_IPARP_SUCCESS = 0,
	TRACE_IPARP_INVALID_ARG,
	TRACE_IPARP_NO_TRACE,
	TRACE_IPARP_TABLE_FULL,
	TRACE_IPARP_CLIENT_NOT_EXIST,
	TRACE_IPARP_UNEXP,
	TRACE_IPARP_DBG
};

enum TRACE_IPARP_CHECK_POINT {
	P0 = 1,
	P1 = (1 << 1),
	P2 = (1 << 2),
	P3 = (1 << 3),
	P4 = (1 << 4),
	P5 = (1 << 5),
	P6 = (1 << 6),
	P7 = (1 << 7),
	P8 = (1 << 8),
	P9 = (1 << 9),
	P10 = (1 << 10),
	P11 = (1 << 11),
	P12 = (1 << 12),
	P13 = (1 << 13), 
	P14 = (1 << 14),
	P15 = (1 << 15),
	P16 = (1 << 16),
	P17 = (1 << 17),
	P18 = (1 << 18),
	P19 = (1 << 19),
	P20 = (1 << 20),
	P21 = (1 << 21),
	P22 = (1 << 22),
	P23 = (1 << 23),
	P24 = (1 << 24),
	P25 = (1 << 25),
	P26 = (1 << 26),
	P27 = (1 << 27),
	P28 = (1 << 28),
	P29 = (1 << 29),
	P30 = (1 << 30),
	P31 = (1 << 31),
};

#define TRACE_IPARP_TX_MASK (P0 | P7 | P10 | P12)
#define TRACE_IPARP_RX_MASK (P0 | P11)

enum TRACE_IPARP_DBG_CTRL {
	TRACE_IPARP_DBG_DONT_CHECK_TABLE	= (1 << 0),
	TRACE_IPARP_DBG_DUMP_TABLE			= (1 << 1),
	TRACE_IPARP_DBG_DONT_CLEAR_ENTRY	= (1 << 2),
	TRACE_IPARP_DBG_TABLE_INFO			= (1 << 3)
};

enum TRACE_IPARP_TABLE_TYPE {
	TX_TRACE,
	RX_TRACE
};

typedef struct _CHECK_POINT_LIST {
	UINT32	value;
	CHAR	descrption[MAX_DESCRP_LEN];
} CHECK_POINT_LIST, *PCHECK_POINT_LIST;

typedef struct _CODE_DESCRP {
	BOOLEAN interested; /* the one that we want to monitor */
	UINT8	value;
	CHAR	descrption[MAX_DESCRP_LEN];
} CODE_DESCRP, *PCODE_DESCRP;

typedef struct _DUMP_ICMP {
	PUCHAR	p_src_mac;
	PUCHAR	p_dst_mac;
	PUCHAR	p_src_ip;
	PUCHAR	p_dst_ip;
	PUCHAR	p_icmp_type;
	UINT16	seq_num;
} DUMP_ICMP;

typedef struct _TRACE_TABLE_IPARP_ENTRY {
	BOOLEAN valid;
	UINT8	wcid;
	ULONG	jiffies;
	UINT32	elapsed_time;
	UINT32	check_points;
	UINT16	protocol_type;
	UINT16	seq_num;
	UINT32	icmparp_code;
	UINT32	priv_data;
	PNDIS_PACKET packet;
	HTTRANSMIT_SETTING mcs_setting;
	PVOID	table;
} TRACE_TABLE_IPARP_ENTRY, *PTRACE_TABLE_IPARP_ENTRY;

// TODO: Add a bitmask to replace using "valid" in entries
typedef struct _TRACE_TABLE_IPARP {
	UINT32 size;
	UCHAR type;
	UINT32 dbg_ctrl;
	NDIS_SPIN_LOCK		lock;
	UINT32 critical_chk_points;	/* Path that frames should go */
	UINT32 branch_chk_points;	/* Path that frames may go */
	UINT32 bad_chk_points;		/* Path that frames shouldn't go */
	UINT32 list_size;
	const CHECK_POINT_LIST	*chk_pnt_list;
	TRACE_TABLE_IPARP_ENTRY entry[MAX_ENTRY_SIZE];
} TRACE_TABLE_IPARP, *PTRACE_TABLE_IPARP;

#endif /* TRACELOG_IPARP */

#endif  /* __TRACELOG_IPARP_DEF_H___ */

