/*
 ***************************************************************************
 * Ralink Tech Inc.
 * 4F, No. 2 Technology 5th Rd.
 * Science-based Industrial Park
 * Hsin-chu, Taiwan, R.O.C.
 *
 * (c) Copyright 2002, Ralink Technology, Inc.
 *
 * All rights reserved. Ralink's source code is an unpublished work and the
 * use of a copyright notice does not imply otherwise. This source code
 * contains confidential trade secret material of Ralink Tech. Any attemp
 * or participation in deciphering, decoding, reverse engineering or in any
 * way altering the source code is stricitly prohibited, unless the prior
 * written consent of Ralink Technology, Inc. is obtained.
 ***************************************************************************

    Module Name:
    ap_wpa.h

    Abstract:

    Revision History:
    Who         When            What
    --------    ----------      ----------------------------------------------
    Name        Date            Modification logs
*/

#ifndef __APWPA_H__
#define __APWPA_H__

#define PEER_MSG1_RETRY_EXEC_INTV           1024        // 1024 ms = 1 sec
#define PEER_MSG3_RETRY_EXEC_INTV           5120        // 5120 ms = 5 sec
#define GROUP_KEY_UPDATE_EXEC_INTV          10240

// Retry timer counter initial value
#define PEER_MSG1_RETRY_TIMER_CTR           0
#define PEER_MSG3_RETRY_TIMER_CTR           10
#define GROUP_MSG1_RETRY_TIMER_CTR          20

#define PAIRWISEKEY                         1
#define GROUPKEY                            0

//EPA VERSION
#define EAPOL_VER                           1
#define EAPOL_VER_2                         2   
#define DESC_TYPE_TKIP                      1
#define DESC_TYPE_AES                       2

#define LEN_MSG1_2WAY                       0x7f

#define MAX_LEN_OF_EAP_HS                   256

// group rekey interval
#define TIME_REKEY                          0
#define PKT_REKEY                           1
#define DISABLE_REKEY                       2
#define MAX_REKEY                           2

#define MAX_REKEY_INTER                     0x3ffffff

typedef struct PACKED _KEY_ENCAP {
    UCHAR       Type;
    UCHAR       Length;
    UCHAR       OUI[4];
} KEY_ENCAP, *PKEY_ENCAP;


//typedef struct PACKED _KEY_INFO
//{
//    UCHAR   KeyMic:1;
//    UCHAR   Secure:1;
//    UCHAR   Error:1;
//    UCHAR   Request:1;
//    UCHAR   EKD_DL:1;     // EKD for AP; DL for STA
//    UCHAR   Rsvd:3;
//    UCHAR   KeyDescVer:3;
//    UCHAR   KeyType:1;
//    UCHAR   KeyIndex:2;
//    UCHAR   Install:1;
//    UCHAR   KeyAck:1;
//} KEY_INFO, *PKEY_INFO;

typedef struct PACKED _AP_KEY_DESCRIPTER {
    UCHAR       Type;
    KEY_INFO    Keyinfo;
    UCHAR       KeyLength[2];
    UCHAR       RCounter[LEN_KEY_DESC_REPLAY];
    UCHAR       Nonce[LEN_KEY_DESC_NONCE];
    UCHAR       IV[LEN_KEY_DESC_IV];
    UCHAR       RSC[LEN_KEY_DESC_RSC];
    UCHAR       ID[LEN_KEY_DESC_ID];
    UCHAR       MIC[LEN_KEY_DESC_MIC];
    UCHAR       DataLen[2];    
    UCHAR       Data[AP_MAX_LEN_OF_RSNIE];
} AP_KEY_DESCRIPTER, *PAP_KEY_DESCRIPTER;

typedef struct PACKED _AP_EAPOL_PACKET {
    UCHAR    ProVer;
    UCHAR    ProType;
    UCHAR    Body_Len[2];
    AP_KEY_DESCRIPTER      KeyDesc;
} AP_EAPOL_PACKET, *PAP_EAPOL_PACKET;

typedef struct PACKED _RSNIE {
    UCHAR   oui[4];
    USHORT  version;
    UCHAR   mcast[4];
    USHORT  ucount;
    struct{
        UCHAR oui[4];
    }ucast[1];
} RSNIE, *PRSNIE;

typedef struct PACKED _RSNIE2 {
    USHORT  version;
    UCHAR   mcast[4];
    USHORT  ucount;
    struct{
        UCHAR oui[4];
    }ucast[1];
} RSNIE2, *PRSNIE2;

typedef struct PACKED _RSNIE_AUTH {
    USHORT acount;
    struct{
        UCHAR oui[4];
    }auth[1];
} RSNIE_AUTH,*PRSNIE_AUTH;

typedef union   _RSN_CAPABILITIES   {
    struct  {
        USHORT      Pre_Auth:1;
        USHORT      No_Pairwise:1;
        USHORT      PTKSA_R_Counter:2;
        USHORT      GTKSA_R_Counter:2;
        USHORT      Rsvd:10;
    }   field;
    USHORT          word;
}   RSN_CAPABILITIES, *PRSN_CAPABILITIES;


#endif


