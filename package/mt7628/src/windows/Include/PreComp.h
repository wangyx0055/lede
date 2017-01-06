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
    PreComp.h

    Abstract:
    

    Revision History:
    Who         When          What
    --------    ----------    ----------------------------------------------


*/
#ifndef __PRECOMP_H__
#define __PRECOMP_H__

#define OS_WIN  1

// 1- If Device has P2P WMM-PS support. 0-If not supported.
// If you want to run WMM-PS, please turn off BA first.
#define UAPSD_AP_SUPPORT        1

//Set this to 1 to enable usb selective suspend
#if(DEV_BUS_TYPE == BUS_TYPE_USB)
#if(CURRENT_OS_NDIS_VER>WIN7_NDIS_VER)  
#ifdef WIN8_RALINK_WFD_SUPPORT
#define _WIN8_USB_SS_SUPPORTED      0
#else
#define _WIN8_USB_SS_SUPPORTED 0
#endif /*WIN8_RALINK_WFD_SUPPORT*/
#endif //(CURRENT_OS_NDIS_VER>WIN7_NDIS_VER)  
#endif//(DEV_BUS_TYPE == BUS_TYPE_USB)

// This is sw based multi-concurrent channel mechanism
#define MULTI_CHANNEL_SUPPORT

#ifndef NDIS_ATTRIBUTE_SURPRISE_REMOVE_OK
#define NDIS_ATTRIBUTE_SURPRISE_REMOVE_OK           0x00000080
#endif  

//

//
// ACT state machine: states, events, total function #
//
#define WFD_NEW_PUBLIC_ACTION

//#define BIG_ENDIAN

#define MT_MAC 1

#define DISABLE_SEND_BAR_AFTER_DISASSOC 1

#endif
