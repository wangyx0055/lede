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
    wpa.h

    Abstract:

    Revision History:
    Who         When            What
    --------    ----------      ----------------------------------------------
    Name        Date            Modification logs
*/

#ifndef __WPA_H__
#define __WPA_H__

// WpaPsk EAPOL Key descripter frame format related length
#define LEN_KEY_DESC_NONCE          32
#define LEN_KEY_DESC_IV             16
#define LEN_KEY_DESC_RSC            8
#define LEN_KEY_DESC_ID             8
#define LEN_KEY_DESC_REPLAY         8
#define LEN_KEY_DESC_MIC            16

//EPA VERSION
#define EAPOL_VER                   1
#define DESC_TYPE_TKIP              1
#define DESC_TYPE_AES               2
#define RSN_KEY_DESC                2
#define WPA_KEY_DESC                0xfe

#define LEN_MASTER_KEY              32  

// EAPOL EK, MK
#define LEN_EAP_EK                  16
#define LEN_EAP_MICK                16
#define LEN_EAP_KEY                 ((LEN_EAP_EK)+(LEN_EAP_MICK))
// TKIP key related
#define LEN_TKIP_EK                 16
#define LEN_TKIP_RXMICK             8
#define LEN_TKIP_TXMICK             8
#define LEN_AES_EK                  16
#define LEN_AES_KEY                 LEN_AES_EK
#define LEN_TKIP_KEY                ((LEN_TKIP_EK)+(LEN_TKIP_RXMICK)+(LEN_TKIP_TXMICK))
#define TKIP_AP_TXMICK_OFFSET       ((LEN_EAP_KEY)+(LEN_TKIP_EK))
#define TKIP_AP_RXMICK_OFFSET       (TKIP_AP_TXMICK_OFFSET+LEN_TKIP_TXMICK)
#define TKIP_GTK_LENGTH             ((LEN_TKIP_EK)+(LEN_TKIP_RXMICK)+(LEN_TKIP_TXMICK))
#define LEN_PTK                     ((LEN_EAP_KEY)+(LEN_TKIP_KEY))
#define MAX_LEN_OF_RSNIE            48

//EAP Packet Type
#define EAPPacket       0
#define EAPOLStart      1
#define EAPOLLogoff     2
#define EAPOLKey        3
#define EAPOLASFAlert   4
#define EAPTtypeMax     5

#define EAPOL_MSG_INVALID   0
#define EAPOL_PAIR_MSG_1    1
#define EAPOL_PAIR_MSG_3    2
#define EAPOL_GROUP_MSG_1   3

// EAPOL Key Information definition within Key descriptor format
typedef struct _KEY_INFO
{
    UCHAR   KeyMic:1;
    UCHAR   Secure:1;
    UCHAR   Error:1;
    UCHAR   Request:1;
    UCHAR   EKD_DL:1;       // EKD for AP; DL for STA
    UCHAR   Rsvd:3;
    UCHAR   KeyDescVer:3;
    UCHAR   KeyType:1;
    UCHAR   KeyIndex:2;
    UCHAR   Install:1;
    UCHAR   KeyAck:1;
}   KEY_INFO, *PKEY_INFO;

// EAPOL Key descriptor format
typedef struct  _KEY_DESCRIPTER
{
    UCHAR       Type;
    KEY_INFO    KeyInfo;
    UCHAR       KeyLength[2];
    UCHAR       ReplayCounter[LEN_KEY_DESC_REPLAY];
    UCHAR       KeyNonce[LEN_KEY_DESC_NONCE];
    UCHAR       KeyIv[LEN_KEY_DESC_IV];
    UCHAR       KeyRsc[LEN_KEY_DESC_RSC];
    UCHAR       KeyId[LEN_KEY_DESC_ID];
    UCHAR       KeyMic[LEN_KEY_DESC_MIC];
    UCHAR       KeyDataLen[2];     
    UCHAR       KeyData[MAX_LEN_OF_RSNIE];
}   KEY_DESCRIPTER, *PKEY_DESCRIPTER;

typedef struct  _EAPOL_PACKET
{
    UCHAR               Version;
    UCHAR               Type;
    UCHAR               Len[2];
    KEY_DESCRIPTER      KeyDesc;
}   EAPOL_PACKET, *PEAPOL_PACKET;

// For supplicant state machine states. 802.11i Draft 4.1, p. 97
// We simplified it
typedef enum    _WpaState
{
    SS_NOTUSE,              // 0
    SS_START,               // 1
    SS_WAIT_MSG_3,          // 2
    SS_WAIT_GROUP,          // 3
    SS_FINISH,              // 4
    SS_KEYUPDATE,           // 5
    SS_STATE_MAX    
}   WPA_STATE;

#define WPA1_KEY_DESC                       0xfe
#define WPA2_KEY_DESC                       0x02

#endif
