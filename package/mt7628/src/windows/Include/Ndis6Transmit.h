/*
 ***************************************************************************
 * MediaTek Inc.
 * 5F, No.5, Tai-Yuan 1st St., 
 * Chupei City, Hsinchu County 30265, Taiwan, R.O.C.
 *
 * (c) Copyright 2014-2016, MediaTek, Inc.
 *
 * All rights reserved. MediaTek's source code is an unpublished work and the
 * use of a copyright notice does not imply otherwise. This source code
 * contains confidential trade secret material of MediaTek. Any attemp
 * or participation in deciphering, decoding, reverse engineering or in any
 * way altering the source code is stricitly prohibited, unless the prior
 * written consent of MediaTek, Inc. is obtained.
 ***************************************************************************

    Module Name:
    Ndis6Transmit.h

    Abstract:
    Miniport generic portion header file

    Revision History:
    Who         When          What
    --------    ----------    ----------------------------------------------
    */

#ifndef __NDIS6TRANSFMIT_H__
#define __NDIS6TRANSFMIT_H__

MINIPORT_SEND_NET_BUFFER_LISTS N6XmitSendNetBufferLists;

NDIS_STATUS
N6XmitGetTxbufToXcu(
    IN PMP_ADAPTER              pAd,
    IN  NDIS_PORT_NUMBER    PortNumber,
    IN  PNET_BUFFER_LIST    NetBufferList,
    IN  PNET_BUFFER         NetBuffer,
    IN  PMT_XMIT_CTRL_UNIT               pXcu
    );

VOID
N6XmitSendNetBufferLists(
    IN  NDIS_HANDLE         MiniportAdapterContext,
    IN  PNET_BUFFER_LIST    NetBufferList,
    IN  NDIS_PORT_NUMBER    PortNumber,
    IN  ULONG               SendFlags
    );

BOOLEAN 
N6XmitGetByteFromMdl(
    IN  PMP_ADAPTER   pAd, 
    IN  PMT_XMIT_CTRL_UNIT pXcu,
    IN  ULONG           Location,
    IN  PUCHAR          pData
    );

#endif

