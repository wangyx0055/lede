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
    aironet.h

    Abstract:

    Revision History:
    Who         When            What
    --------    ----------      ----------------------------------------------
    Name        Date            Modification logs
    Paul Lin    04-06-15        Initial
*/

#ifndef __AIRONET_H__
#define __AIRONET_H__

/**** IMPORTANT: AIRONET has several none-4-bytes alignment elements ******/
#pragma pack( push, struct_pack1 )
#pragma pack( 1 )

// CCX administration control
typedef union   _CCX_CONTROL    {
    struct  {
        ULONG       Enable:1;           // Enable CCX2 / CCX3 / CCX4 / CCX5
        ULONG       CCKMEnable:1;       // Enable CCKM
        ULONG       RMEnable:1;         // Radio Measurement Enable
        ULONG       DCRMEnable:1;       // Non serving channel Radio Measurement enable
        ULONG       CACEnable:1;        // CAC Tolerence, Enable Retry of CAC/ACM for CCXv4, and AssocReq with CAC
        ULONG       FastRoamEnable:1;   // Enable fast roaming
        ULONG       NetworkEAP:1;       // Force using NetwrokEAP(128) Auth-Algorithm
        ULONG       DisableAutoReconnect:1;     // UI will rotate on profiles, driver should not autoreconnect
        ULONG       dBmToRoam:8;        // the condition to roam when receiving Rssi less than this value. It's negative value.
        ULONG       TuLimit:10;         // Limit for different channel scan
        ULONG       MFP:1;              // Management Frame Protection. not used in Vista
        ULONG       RFRoamEnable:1; // CCX5 roaming - RF RF Parameters, S72
        ULONG       VoiceRate:1;            // Enable Voice Rate, to use 6Mbps or 1 Mbps for CCX test, RtmpSendPacket do HardTx directly
        ULONG       Rsvd:3;         // Not Used
    }   field;
    ULONG           word;
}   CCX_CONTROL, *PCCX_CONTROL;

// MHDRIE for TKIP MFP frames.
typedef struct _MHDRIE 
{
    UCHAR           Eid;            // 0xDD
    UCHAR           Length;         // 0x0C
    UCHAR           AironetOui[3];  // AIronet OUI (00 40 96)
    UCHAR           ID;             // 0x10
    FRAME_CONTROL   FC;
    UCHAR           Bssid[MAC_ADDR_LEN];
} MHDRIE, *PMHDRIE;
#pragma pack( pop, struct_pack1 )

#endif  // __AIRONET_H__
