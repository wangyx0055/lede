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
    Port_oids_Set.c

    Abstract:
    Miniport generic portion header file

    Revision History:
    Who         When          What
    --------    ----------    ----------------------------------------------
    */
    
#include    "MtConfig.h"

NDIS_STATUS
N6SetOidDot11CreateMac(
    IN  PMP_ADAPTER         pAd,
    IN PNDIS_OID_REQUEST NdisRequest)
{
    NDIS_PORT_NUMBER PortNumber                 = NdisRequest->PortNumber;
    PMP_PORT         pPort                      = pAd->PortList[PortNumber];

    NDIS_STATUS ndisStatus = NDIS_STATUS_SUCCESS;
    FUNC_ENTER;

    // EXPAND_FUNC NONE
    //SDV: irql_protocol_driver_function
    ndisStatus = Ndis6CommonPortCreateMac(pAd, pPort, NdisRequest);

    FUNC_LEAVE_STATUS(ndisStatus);
    return ndisStatus;
}


NDIS_STATUS
N6SetOidDot11DeleteMac(
    IN  PMP_ADAPTER         pAd,
    IN PNDIS_OID_REQUEST NdisRequest)
{
    NDIS_PORT_NUMBER PortNumber = NdisRequest->PortNumber;
    PMP_PORT         pPort      = pAd->PortList[PortNumber];

    NDIS_STATUS ndisStatus = NDIS_STATUS_SUCCESS;
    FUNC_ENTER;

    // EXPAND_FUNC NONE
    //SDV: irql_protocol_driver_function
    ndisStatus = Ndis6CommonPortDeleteMac(pAd, pPort, NdisRequest);
    
    FUNC_LEAVE_STATUS(ndisStatus);
    return ndisStatus;
}

