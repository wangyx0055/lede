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
    MtConfig.h

    Abstract:
    Central header file to maintain all include files for all NDIS
    miniport driver routines.

    Revision History:
    Who         When          What
    --------    ----------    ----------------------------------------------
    Paul Lin    08-01-2002    created

*/
#ifndef __MTCONFIG_H__
#define __MTCONFIG_H__

//
// Globally disabled warnings
// These are superfluous errors at warning level 4.
//

#pragma warning(disable:4214)   // bit field types other than int
#pragma warning(disable:4200)   // non-standard extension used
#pragma warning(disable:4201)   // nameless struct/union
#pragma warning(disable:4115)   // named type definition in parentheses
#pragma warning(disable:4127)   // conditional expression is constant
#pragma warning(disable:4054)   // cast of function pointer to PVOID
#pragma warning(disable:4206)   // translation unit is empty
#pragma warning(disable:4100)
#pragma warning(disable:4189)
#pragma warning(disable:4057)
#pragma warning(disable:4213)
#pragma warning(disable:4244)
#pragma warning(disable:4152)
#pragma warning(disable:28247)
//
//  NDIS standard header files
//
#include <ndis.h>
#include <ntintsafe.h>

#if(DEV_BUS_TYPE==BUS_TYPE_USB)
    #include <wdf.h>
    #include <WdfMiniport.h>
    #include <wdftimer.h>
    #include <usb.h>
    #include <usbioctl.h>
    #include <usbdlib.h>
    #include <wdfusb.h>
    #include <wdfworkitem.h>
    #include <wdfdevice.h>
    #include <Wdfcore.h>
    #include <wdfrequest.h>
#endif

#include    <stdarg.h>
#pragma warning(disable:4214)   // bit field types other than int
#include "80211hdr.h"

#include    "OS_VER.h"
#include    "BusTypeDef.h"
#include    "PreComp.h"
#include    "QAToolDef.h"
//  Globa type definition
//

#include    "MTType.h"
#include    "Thread.h"
#include    "NdisCommon.h"
#include    "NdisCommonUsb.h"
#include    "PlatformIndicate.h"
//  Global constant definition
//
#include    "MtDef.h"

#include "..\hal\common\hal_common.h"
#include "..\hal\mt7603\mt7603.h"
#include "LedCtrl.h"

// TPC
//
#include "Tpc.h"

//
//  Hradware related header files
//
#include "mac\mac_mt\smac\mt_mac.h"
#include "hw_ctrl\cmm_asic_mt.h"
#include "mcu\mt_cmd.h"
#include    "Rt2870.h"
#include    "Eeprom.h"

//
// New feature start from new IC version and VHT(802.11ac)
//
#include    "Vht.h"
#include    "Rt6x9x.h"
#include    "RecvPktCmdRspEvt.h"
#include    "Oid6x9x.h"
#include    "..\backup\MT7603_backup.h"

//
//  Miniport defined header files
//
#include    "Memory.h"
#include    "Oid.h"
#include    "MlmeMgntPkt.h"
#include    "Mlme.h"
#include    "Fw.h"
#include    "NdisInit.h"
#include    "MlmeInfo.h"
#include    "Common.h"
#include    "Ndis6Common.h"
#include    "Ndis6Transmit.h"
#include    "N6Indicate.h"
#include    "Ndis6Init.h"

#include    "OidP2pWfdQuery.h"
#include    "OidP2pWfdSet.h"
#include    "OidPortQuery.h"
#include    "OidPortSet.h"
#include    "OidStaQuery.h"
#include    "OidStaSet.h"
#include    "OidApQuery.h"
#include    "OidApSet.h"
#include    "OidMtkQuery.h"
#include    "OidMtkSet.h"
#include    "Ndisoids.h"

#if (SUPPORT_QA_TOOL ==1)
#include    "OidQaSet.h"
#include    "OidQaQuery.h"
#endif

#include    "CryptoMd5.h"
#include    "Wpa.h"
#include    "Aironet.h"
#include    "Wsc.h"
#include    "MtMp.h"
#include    "Xmit.h"
#include    "Recv.h"
#include    "Ndis6Main.h"
#include    "Adapter.h"
#include    "ApMain.h"
#include    "MTCkipmic.h"
#include    "CryptoMd4.h"
#include    "ApWpa.h"
#include    "P2p.h"
#if (CURRENT_OS_NDIS_VER>=WIN8_NDIS_VER)
#include    "P2pMs.h"
#endif
#include    "Utility.h"

// WSC security code
#include    "WscCrypto.h"

// MFP
#include    "Pmf.h"

#include "N6UsbMain.h"

#if UAPSD_AP_SUPPORT
#include "ApUapsd.h"
#endif

//#include "hw_ctrl\cmm_asic_mt_unify.h"

#include "iface\mt_hif.h"
#include "drs_extr.h"

#endif  // __RT_CONFIG_H__

