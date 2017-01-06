/*
    ***************************************************************************
    * Ralink Technology, Corp.
    * 5F., No. 5, Tai-Yuan 1st St., Jhubei City, Hsinchu County 30265, Taiwan
    *
    * (c) Copyright 2002-2011, Ralink Technology, Corp.
    *
    * All rights reserved. Ralink's source code is an unpublished work and the use of a copyright notice 
    * does not imply otherwise. This source code contains confidential trade secret material of 
    * Ralink Technology, Corp. Any attempt or participation in deciphering, decoding, reverse engineering 
    * or in any way altering the source code is stricitly prohibited, unless the prior written consent of 
    * Ralink Technology, Corp. is obtained.
    ***************************************************************************

    Module Name: 
    vht.h

    Abstract: 
    1. VHT IEs
    2. VHT protocol control

    Revision History:
    Who                     When            What
    -------------------     ----------      ----------------------------------------------
    Ian Tang                    2011/9/5        Start implementation
    Ian Tang                    2012/8/9        Add Operating Mode Notification
*/

#ifndef __VHT_H__
#define __VHT_H__

#pragma pack(push, struct_pack1)
#pragma pack(1)

//
// VHT IE IDs
//
#define IE_VHT_CAP                      191
#define IE_VHT_OP                       192
#define IE_VHT_EXT_BSS_LOAD             193
#define IE_VHT_WIDE_BW_CHANNEL_SWITCH   194
#define IE_VHT_TX_POWER                 195
#define IE_VHT_EXT_POWER_CONSTRAINT     196
#define IE_VHT_AID                      197
#define IE_VHT_QUIET_CHANNEL            198
#define IE_VHT_OPERATING_MODE_NOTIFICATION 199

//
#define MCS_0_7        0
#define MCS_0_8        1
#define MCS_0_9        2
#define NOT_SUPPORTED  3

//
// VHT MIMO control field, excluding element ID and length fields
//
//2 TODO: Add fields from WFA Test Plan
//
typedef struct _VHT_MIMO_CTRL_FIELD
{
    ULONG   Temp; //2 TODO: Remove
} VHT_MIMO_CTRL_FIELD, *PVHT_MIMO_CTRL_FIELD;

//
// Size of VHT MIMO control field, excluding element ID and length fields
//
#define SIZE_OF_VHT_MIMO_CTRL_FIELD (sizeof(VHT_MIMO_CTRL_FIELD))


//
//
//
typedef struct _VHT_CAP_INFO
{
    ULONG   MaxMpduLength:2;
    ULONG   SupportedChannelWidthSet:2;
    ULONG   RxLdpc:1;
    ULONG   ShortGIfor80Mhz:1;
    ULONG   ShortGIfor160Mhz:1;
    ULONG   TxStbc:1;
    ULONG   RxStbc:3;
    ULONG   SuBeamformerCapable:1;
    ULONG   SuBeamformeeCapable:1;
    ULONG   CompressedSteeringNumber:3;
    ULONG   SoundingDimensionsNum:3;
    ULONG   MuBeamformerCapable:1;
    ULONG   MuBeamformeeCapable:1;
    ULONG   VhtTxopPs:1;
    ULONG   HtcVhtCapable:1;
    ULONG   MaxAmpduLenExp:3;
    ULONG   VhtLinkAdaptationCapble:2;
    ULONG   RxAntPatternConsistency:1;
    ULONG   TxAntPatternConsistency:1;
    ULONG   Reserved:2;
} VHT_CAP_INFO, *PVHT_CAP_INFO;

typedef struct _MCS_MAP
{
    USHORT MaxMcsFor1ss:2;
    USHORT MaxMcsFor2ss:2;
    USHORT MaxMcsFor3ss:2;
    USHORT MaxMcsFor4ss:2;
    USHORT MaxMcsFor5ss:2;
    USHORT MaxMcsFor6ss:2;
    USHORT MaxMcsFor7ss:2;
    USHORT MaxMcsFor8ss:2;      
} MCS_MAP, *PMCS_MAP;

//
//
//
typedef struct _VHT_SUPPORT_MCS_SET
{
    MCS_MAP RxMcsMap;
    USHORT  RxHighSupportedDataRate:13;
    USHORT  Reserved1:3;
    MCS_MAP TxMcsMap;
    USHORT  TxHighSupportedDataRate:13;
    USHORT  Reserved2:3;
} VHT_SUPPORT_MCS_SET, *PVHT_SUPPORT_MCS_SET;

//
// VHT capabilities IE, excluding element ID and length fields
//
// Element ID: 191
//
typedef struct _VHT_CAP_IE
{
    VHT_CAP_INFO            VhtCapInfo;
    VHT_SUPPORT_MCS_SET     VhtSupportMcsSet;
} VHT_CAP_IE, *PVHT_CAP_IE;

//
// Size of VHT capabilities IE, excluding element ID and length fields
//
#define SIZE_OF_VHT_CAP_IE          (sizeof(VHT_CAP_IE))

//
//
//
typedef struct _VHT_OP_INFO
{
    UCHAR ChannelWidth;
    UCHAR ChannelCenterFreqSeg1; // only valid BW=80, 160 and 80+80
    UCHAR ChannelCenterFreqSeg2; // only valid BW=80+80
} VHT_OP_INFO, *PVHT_OP_INFO;

//
// VHT operation IE, excluding element ID and length fields
//
// Element ID: 192
//
typedef struct _VHT_OP_IE
{
    VHT_OP_INFO         VhtOpInfo;
    MCS_MAP       VhtBasicMcsSet;
} VHT_OP_IE, *PVHT_OP_IE;

//
// Size of VHT operation IE, excluding element ID and length fields
//
#define SIZE_OF_VHT_OP_IE           (sizeof(VHT_OP_IE))

//
// VHT BSS load IE, excluding element ID and length fields
//
// Element ID: 193
//
typedef struct _VHT_EXT_BSS_LOAD_IE
{
    UCHAR MuMimoCapableStaCount;
    UCHAR SpatialStreamUnderutilization;
    UCHAR Vht40MhzUtilization;
    UCHAR Vht80MhzUtilization;
    UCHAR Vht160MhzUtilization;
} VHT_EXT_BSS_LOAD_IE, *PVHT_EXT_BSS_LOAD_IE;

//
// Size of VHT BSS load IE, excluding element ID and length fields
//
#define SIZE_OF_VHT_EXT_BSS_LOAD_IE     (sizeof(VHT_EXT_BSS_LOAD_IE))


//
// Element ID: 194
//
typedef struct _VHT_WIDE_BW_CHANNEL_SWITCH_IE
{
    UCHAR NewStaChannelWidth;
    UCHAR NewChannelCenterFreqSeg1;
    UCHAR NewChannelCenterFreqSeg2;
} VHT_WIDE_BW_CHANNEL_SWITCH_IE, *PVHT_WIDE_BW_CHANNEL_SWITCH_IE;

#define SIZE_OF_VHT_WIDE_BW_CHANNEL_SWITCH_IE       (sizeof(VHT_WIDE_BW_CHANNEL_SWITCH_IE))

//
//
//
typedef struct _VHT_CHANNEL_SEGMENT_SETTING
{
    UCHAR ChannelCenterFreqSeg;
    UCHAR SegChannelWidth;
} VHT_CHANNEL_SEGMENT_SETTING, *PVHT_CHANNEL_SEGMENT_SETTING;
//
// Element ID: 195
//
typedef struct _VHT_TX_POWER_ENVELOPE_IE
{
    UCHAR MaxTxPower;
    UCHAR Octet[1];   // point to a struct VHT_CHANNEL_SEGMENT_SETTING  
} VHT_TX_POWER_ENVELOPE_IE, *PVHT_TX_POWER_ENVELOPE_IE;

#define SIZE_OF_VHT_TX_POWER_ENVELOPE_IE        (sizeof(VHT_TX_POWER_ENVELOPE_IE))

//
//
//
typedef struct _VHT_CHANNEL_POWER_PAIR
{
    UCHAR ChannelCenterFreqSeg;
    UCHAR SegChannelWidth;
} VHT_CHANNEL_POWER_PAIR, *PVHT_CHANNEL_POWER_PAIR;

//
// Element ID: 196
//
typedef struct _VHT_EXT_POWER_CONSTRAINT_IE
{
    UCHAR Octet[1];   // point to a struct VHT_CHANNEL_POWER_PAIR;  
} VHT_EXT_POWER_CONSTRAINT_IE, *PVHT_EXT_POWER_CONSTRAINT_IE;

#define SIZE_OF_VHT_EXT_POWER_CONSTRAINT_IE     (sizeof(VHT_EXT_POWER_CONSTRAINT_IE))

//
// Element ID: 197
//
typedef struct _VHT_AID_IE
{
    USHORT Aid;
} VHT_AID_IE, *PVHT_AID_IE;

#define SIZE_OF_VHT_AID_IE      (sizeof(VHT_AID_IE))
//
//
//
typedef struct _VHT_QUIET_CHANNEL_SETTING
{
    UCHAR   QuietCount;
    UCHAR   QuietPeriod;
    USHORT  QuietDuration;
    USHORT  QuietOffset;    
} VHT_QUIET_CHANNEL_SETTING, *PVHT_QUIET_CHANNEL_SETTING;

//
// Element ID: 198
//
typedef struct _VHT_QUIET_CHANNEL_IE
{
    UCHAR ApQuietmode;
    UCHAR Octet[1];   // point to a struct VHT_QUIET_CHANNEL_SETTING
} VHT_QUIET_CHANNEL_IE, *PVHT_QUIET_CHANNEL_IE;


//
//
//
typedef union _HT_VHT_CONTROL
{
    struct {
        ULONG       VHT:1;
        ULONG       TRQ:1;
        ULONG       MAI:4;
        ULONG       MFSI:3;
        ULONG       MFBorASC:7;
        ULONG       CalPos:2;   // calibration position
        ULONG       CalSeq:2;  //calibration sequence
        ULONG       rsv1:2;
        ULONG       CsiSteering:2; // CSI/Steering
        ULONG       NDPAnnouncement:1; // NDP Announcement
        ULONG       rsv2:4;  //calibration sequence
        ULONG       DEI:1;
        ULONG       ACConstraint:1; //feedback request
        ULONG       RDG:1;  //RDG / More PPDU
    }HtVariant;

    struct {
        ULONG       VHT:1;
        ULONG       rsv1:1;
        ULONG       MRQ:1;
        ULONG       CompressedMsi:2;
        ULONG       StbcIndication:1;
        ULONG       MfsiGidL:3;
        
        ULONG       N_STS:3;
        ULONG       MCS:4;
        ULONG       BW:2;
        ULONG       SNR:6;
        
        ULONG       GidH:3;
        ULONG       CodingType:1;       
        ULONG       FbTxType:1;
        ULONG       UnsolicitedMfb:1;
        ULONG       ACConstraint:1; //feedback request
        ULONG       RDG:1;  //RDG / More PPDU
    }VHtVariant;
    
} HT_VHT_CONTROL, *PHT_VHT_CONTROL;


//
//
//
typedef struct _BEAMFORMING_REPORT_POLL {
    struct
    {
        USHORT      Ver:2;              // Protocol version
        USHORT      Type:2;             // MSDU type
        USHORT      SubType:4;          // MSDU subtype
        USHORT      ToDs:1;             // To DS indication
        USHORT      FrDs:1;             // From DS indication
        USHORT      MoreFrag:1;         // More fragment bit
        USHORT      Retry:1;            // Retry status bit
        USHORT      PwrMgmt:1;          // Power management bit
        USHORT      MoreData:1;         // More data bit
        USHORT      Wep:1;              // Wep data
        USHORT      Order:1;            // Strict order expected
    } FrameControl;

    USHORT                  Duration;
    DOT11_MAC_ADDRESS       RA;
    DOT11_MAC_ADDRESS       TA;
    UCHAR                   SegReTxBitmap;
    ULONG                   FCS;
} BEAMFORMING_REPORT_POLL, *PBEAMFORMING_REPORT_POLL;

//
// The supported channel width (CH_WIDTH_BWXXX)
//
#define CH_WIDTH_BW20 0 // 20MHz
#define CH_WIDTH_BW40 1 // 40MHz
#define CH_WIDTH_BW80 2 // 80MHz
#define CH_WIDTH_BW160_OR_BW80_80 3 // 160MHz or 80+80MHz

//
// The maximum number of spatial streams that the STA can receive (RX_NSS_XXX)
//
#define RX_NSS_1 0 // Nss = 1
#define RX_NSS_2 1 // Nss = 2
#define RX_NSS_3 2 // Nss = 3
#define RX_NSS_4 3 // Nss = 4
#define RX_NSS_5 4 // Nss = 5
#define RX_NSS_6 5 // Nss = 6
#define RX_NSS_7 6 // Nss = 7
#define RX_NSS_8 7 // Nss = 8

//
// Rx Nss type (RX_NSS_TYPE_MAX_RX_NSS_XXX)
//
#define RX_NSS_TYPE_MAX_RX_NSS_NORMAL 0 // Maximum number of spatial streams that the STA can receive
#define RX_NSS_TYPE_MAX_RX_NSS_FOR_TXBF 1 // Maximum number of spatial streams that the STA can receive as an SU PPDU using a beamforming steering matrix derived from a VHT Compressed Beaforming frame

//
// Operating Mode field
//
typedef struct _OPERATING_MODE_FIELD
{
    UCHAR ChannelWidth:2; // Channel width (CH_WIDTH_BWXXX)
    UCHAR Reserved:2; // Reserved
    UCHAR RxNss:3; // Rx Nss (RX_NSS_XXX)
    UCHAR RxNssType:1; // Rx Nss Type (RX_NSS_TYPE_MAX_RX_NSS_XXX)
} OPERATING_MODE_FIELD, *POPERATING_MODE_FIELD;

//
// Size of VHT Operating Mode Notification IE, excluding element ID and length fields
//
#define SIZE_OF_VHT_OPERATING_MODE_NOTIFICATION_IE          (sizeof(OPERATING_MODE_FIELD))

//
// VHT Action fields (VHT_ACTION_XXX)
//
#define VHT_ACTION_VHT_COMPRESSED_BEAMFORMING (0)
#define VHT_ACTION_GROUP_ID_MANAGEMENT (1)
#define VHT_ACTION_OPERATING_MODE_NOTIFICATION (2)

//
// Operating Mode Notification frame Action field format
//
typedef struct _OPERATING_MODE_NOTIFICATION_ACTION
{
    UCHAR Category; // Category (CATEGORY_VHT)
    UCHAR Action; // VHT Action (VHT_ACTION_OPERATING_MODE_NOTIFICATION)
    OPERATING_MODE_FIELD OperatingMode; // Operating Mode
} OPERATING_MODE_NOTIFICATION_ACTION, *POPERATING_MODE_NOTIFICATION_ACTION;

#pragma pack(pop, struct_pack1)

//
// Initialize VHT MIMO control field
//
VOID VhtInittMimoCtrlField(
    IN PMP_ADAPTER pAd);

//
// Initialize VHT capabilities IE, ID=191
//
VOID VhtInitCapIE(
    IN PMP_ADAPTER pAd,
    IN PMP_PORT     pPort);

//
// Initialize VHT operation IE, ID=192
//
VOID VhtInitOpIE(
    IN PMP_ADAPTER pAd,
    IN PMP_PORT     pPort);

//
// Initialize VHT Ext BSS load IE, ID=193
//
VOID VhtInitExtBssLoadIE(
    IN PMP_ADAPTER pAd,
    IN PMP_PORT     pPort);

//
// Initialize VHT WIDE BW Channel Switch IE, ID=194
//
VOID VhtInitBideBWChannelSwitchIE(
    IN PMP_ADAPTER pAd);

//
// Initialize VHT TxPower Envelope IE, ID=195
//
VOID VhtInitTxPowerEnvelopeIE(
    IN PMP_ADAPTER pAd);

//
// Initialize VHT Ext Power Contraint IE, ID=196
//
VOID VhtInitExtPowerConstraintIE(
    IN PMP_ADAPTER pAd);

//
// Initialize VHT AID IE, ID=197
//
VOID VhtInitAidIE(
    IN PMP_ADAPTER pAd);


//
// Initialize VHT Quiet Channel IE, ID=198
//
VOID VhtInitQuietChannelIE(
    IN PMP_ADAPTER pAd);

//
// In VHT, use PrimaryChannel and BW to decide VhtCentralChannel
//
VOID Vht5GCentralChannelDecision(
    IN  PMP_ADAPTER   pAd,
    IN  UCHAR           PrimaryChannel,
    IN  UCHAR           BW,
    OUT PUCHAR          pCentralChannel);

VOID VhtDecideTheMaxMCSForSpatialStream(
    OUT   PMCS_MAP Result,
    IN    PMCS_MAP STA1,
    IN    PMCS_MAP STA2);
    
#endif // __VHT_H__

//////////////////////////////////// End of File ////////////////////////////////////


