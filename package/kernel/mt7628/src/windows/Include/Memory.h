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
    Receive.h

    Abstract:
    Miniport generic portion header file

    Revision History:
    Who         When          What
    --------    ----------    ----------------------------------------------
    */

#ifndef __MEMORY_H__
#define __MEMORY_H__
#include    "MtConfig.h"

ULONG   MtkCompareMemory(
    IN  PVOID   pSrc1,
    IN  PVOID   pSrc2,
    IN  ULONG   Len
    );

NDIS_STATUS
PlatformAllocateMemory(
    IN  PMP_ADAPTER   pAd,
    OUT PVOID   *pPtr,
    IN  ULONG   length
    );

VOID
PlatformFreeMemory(
    IN  PVOID   ptr,
    IN  ULONG   length
    );

VOID
PlatformZeroMemory(
    IN  PVOID   ptr,
    IN  ULONG   length
    );

VOID
PlatformMoveMemory(
    IN  PVOID   pDest,
    IN  PVOID   pSrc,
    IN  ULONG   length
    );
    
LONG
PlatformEqualMemory(
    IN  PVOID   pBuf1,
    IN  PVOID   pBuf2,
    IN  ULONG   length
    );

VOID
PlatformFillMemory(
    IN  PVOID   pBuf,
    IN  ULONG   Length,
    IN  UCHAR   Fill
    );
    
#endif