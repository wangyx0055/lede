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
    rtmpFw.h

    Abstract:
    Miniport generic portion header file

    Revision History:
    Who         When          What
    --------    ----------    ----------------------------------------------
    */

#ifndef __MTFW_H__
#define __MTFW_H__

//#pragma pack(pop, struct_pack1)

NDIS_STATUS FirmwareCommnadRestartDownloadFW(
    IN  PMP_ADAPTER   pAd
    );

NDIS_STATUS FirmwareCommnadStartToLoadFW(
    IN  PMP_ADAPTER   pAd,
    IN  ULONG           ImageLength
    );

NDIS_STATUS FirmwareScatters(
    IN  PMP_ADAPTER   pAd,
    IN  PUCHAR          pFirmwareImage,
    IN  ULONG           StartAddress,
    IN  ULONG           ImageLength
    );

NDIS_STATUS FirmwareCommnadStartToRunFW(
    IN  PMP_ADAPTER   pAd
    );

//
// Call by LoadAndesFW
//
NDIS_STATUS USB_Load_ILM_DLM(
    IN  PMP_ADAPTER   pAd,
    IN  PUCHAR          pFirmwareImage,
    IN  ULONG           StartAddress,
    IN  ULONG           ImageLength);

//
// Load Andes firmware
//
NDIS_STATUS LoadAndesFW(
    IN PMP_ADAPTER pAd);
#endif