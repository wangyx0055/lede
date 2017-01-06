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
    NdisCommon_Usb.h

    Abstract:
    Miniport generic portion header file

    Revision History:
    Who         When          What
    --------    ----------    ----------------------------------------------
    */

#ifndef __NDISCOMMONUSB_H__
#define __NDISCOMMONUSB_H__

typedef struct _RTMP_ADAPTER MP_ADAPTER, *PMP_ADAPTER;

VOID    
NdisCommonUsbCleanUpMLMEWaitQueue(
    IN  PMP_ADAPTER   pAd
    );

VOID    
NdisCommonUsbCleanUpMLMEBulkOutQueue(
    IN  PMP_ADAPTER   pAd
    );    

NDIS_STATUS
NdisCommonUsbStartRx(
    IN PMP_ADAPTER pAd
    );

NDIS_STATUS
NdisCommonUsbStartTx(
    IN PMP_ADAPTER pAd
    );

VOID
NdisCommonUsbStopTx(
    IN PMP_ADAPTER pAd
    );

VOID
NdisCommonUsbStopRx(
    IN PMP_ADAPTER pAd
    );

VOID
NdisCommonUsbBulkRx(
    IN PMP_ADAPTER pAd
    );

VOID
NdisCommonUsbBulkRxCmd(
    IN PMP_ADAPTER pAd
    );

NDIS_STATUS
NdisCommonUsbInitTransmit(
    IN PMP_ADAPTER pAd
    );

NDIS_STATUS
NdisCommonUsbInitRecv(
    IN PMP_ADAPTER pAd
    );

VOID
NdisCommonUsbFreeNicRecv(
    IN PMP_ADAPTER pAd
    );

VOID
NdisCommonUsbFreeNICTransmit(
    IN PMP_ADAPTER pAd
    );

VOID    
NdisCommonUsbDeQueuePacket(
    IN  PMP_ADAPTER   pAd,    
    IN  UCHAR           BulkOutPipeId
    );    

NDIS_STATUS 
NdisCommonUsbFreeDescriptorRequest(
    IN  PMP_ADAPTER   pAd,
    IN  UCHAR           RingType,
    IN  UCHAR           BulkOutPipeId,
    IN  ULONG           NumberRequired
    );

VOID    
NdisCommonUsbMlmeHardTransmit(
    IN  PMP_ADAPTER   pAd,
    IN  PMGMT_STRUC     pMgmt
    );    

VOID    
NdisCommonUsbDequeueMLMEPacket(
    IN  PMP_ADAPTER   pAd
    );

VOID    
NdisCommonUsbMiniportMMRequest(
    IN  PMP_ADAPTER   pAd,    
    IN  PVOID           pBuffer,
    IN  ULONG           Length
    );     

size_t
NdisCommonUsbGetPipeNumber(
    IN PMP_ADAPTER pAd
    );    
#endif