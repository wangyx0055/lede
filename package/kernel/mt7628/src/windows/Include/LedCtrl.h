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
    Led_Ctrl.h

    Abstract:
    Miniport generic portion header file

    Revision History:
    Who         When          What
    --------    ----------    ----------------------------------------------
    */

#ifndef __LED_CTRL_H__
#define __LED_CTRL_H__

typedef struct _RTMP_ADAPTER MP_ADAPTER, *PMP_ADAPTER;

/*
    ========================================================================
    
    Routine Description:
        Set LED Status

    Arguments:
        pAd                     Pointer to our adapter
        Status                  LED Status

    Return Value:
        None

    IRQL = PASSIVE_LEVEL
    IRQL = DISPATCH_LEVEL
    
    Note:
        
    ========================================================================
*/

VOID LedCtrlSetLed(
    IN PMP_ADAPTER    pAd, 
    IN UCHAR            Status);

VOID LedCtrlSetSignalLed(
    IN PMP_ADAPTER    pAd, 
    IN NDIS_802_11_RSSI Dbm);

VOID 
LedCtrlBlinkThread(
    IN  PMP_ADAPTER   pAd
    );  

VOID 
LedCtrlSetBlinkMode(
    IN PMP_ADAPTER       pAd,
    IN UCHAR                WPSLedStatus
    );

VOID
LedCtrlConnectionStart(
    IN PMP_ADAPTER pAd
    );

VOID
LedCtrlConnectionCompletion(
    IN PMP_ADAPTER pAd, 
    IN  PMP_PORT pPort,
    IN BOOLEAN bSuccess
    );    
#endif