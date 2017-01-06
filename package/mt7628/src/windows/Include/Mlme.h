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
    mlme.h

    Abstract:

    Revision History:
    Who         When            What
    --------    ----------      ----------------------------------------------
    John Chang  2003-08-28      Created
    John Chang  2004-09-06      modified for RT2600
    
*/
#ifndef __MLME_H__
#define __MLME_H__

//extern UCHAR BROADCAST_ADDR[];

//#define RTMPAllocateMemory(_ppVA, _Length)  NdisAllocateMemoryWithTag((PVOID*)(_ppVA), _Length, NIC_TAG)

//#define RTMP_ALLOCATE_MEMORY(_NdisHandle, _ppVA, _Length, _Tag) \
//   *_ppVA = NdisAllocateMemoryWithTagPriority(_NdisHandle, _Length, _Tag, NormalPoolPriority)


// maximum supported capability information - 
// ESS, IBSS, Privacy, Short Preamble, Spectrum mgmt, Short Slot
#define SUPPORTED_CAPABILITY_INFO   0x0533

#define END_OF_ARGS                 -1
#define LFSR_MASK                   0x80000057
#define MLME_TASK_EXEC_INTV         100       // 1 sec
#define STAY_IN_AWAKE         5       // 5 sec
#define STAY_10_SECONDS_AWAKE         10000       // 10 sec
#define WAKE_DELAY_INTV         900 
#define RECBA_EXEC_MULTIPLE         1
#define MLME_TASK_EXEC_MULTIPLE         10       // MLME_TASK_EXEC_MULTIPLE * MLME_TASK_EXEC_INTV = 1 sec

//
// The period, in seconds, of performing the VCO calibration
//
#define VCO_CALIBRATION_PERIOD          5       // unit: second
#define VCO_CALIBRATION_PERIOD_SHIFT    5       // When different action will effect each other and these have same period,
                                                // we need to make a shift to one of these.

#define SHUTDOWN_TIMEOUT 1200

#ifdef  NDIS51_MINIPORT
#define BEACON_LOST_TIME            8000       // 2048 msec = 2 sec
#define ADHOC_BEACON_LOST_TIME      10000       // 10 second
#define ONE_SECOND_TIME              1000       // 1 second
#elif(COMPILE_VISTA_ABOVE(CURRENT_OS_NDIS_VER))  
#define BEACON_ROAM_TIME             10000000
#define BEACON_LOST_TIME             80000000 // 8 second
#define SHORT_BEACON_LOST_TIME      3*ONE_SECOND_TIME       // 3 second (For ULONGLONG)
#define NOA_BEACON_LOST_TIME        6*ONE_SECOND_TIME       // 3 second (For ULONGLONG)
#define ADHOC_BEACON_LOST_TIME      100000000 // 10 second, In 100s of nanoseconds
#define ONE_SECOND_TIME             10000000 // 1 second
#define ONE_MSECOND_TIME                10000 // 1 milli-second 
#define FIVE_SECOND_TIME             50000000 // 5 second
#define THREE_SECOND_TIME             30000000 // 3 second
#else
#define BEACON_LOST_TIME            8000       // 2048 msec = 2 sec
#define ADHOC_BEACON_LOST_TIME      10000       // 10 second
#define ONE_SECOND_TIME              1000       // 1 second
#endif

#define STA_BEACON_LOST_TIME        6*ONE_SECOND_TIME       // 6  sec   
#define P2P_BEACON_LOST_TIME        30*ONE_SECOND_TIME      // 30 sec

#define DLS_TIMEOUT                 1200      // unit: msec
#define TDLS_TIMEOUT                1200      // unit: msec
#define TDLS_KEY_TIMEOUT            3600      // unit: sec
#define AUTH_TIMEOUT                1000//500       // unit: msec
#define ASSOC_TIMEOUT               1000//500       // unit: msec

#define JOIN_TIMEOUT                12000      // unit: msec, test with Atheros win8, SR=1 might be over 4.5 sec

#define P2P_LISTEN_PORT0_SCAN_TIME          70
#define SHORT_CHANNEL_TIME(_pAd) (90) // unit: msec
#define RESTORE_CHANNEL_TIME        300        // unit: msec
#define MIN_CHANNEL_TIME(_pAd) (110) // unit: msec, for dual band scan
#define MAX_CHANNEL_TIME            140       // unit: msec, for single band scan
#define HOLD_AT_OP_CHANNEL_TIME 1500  // unit: msec,for at op channel of GO waiting GO beacon
#define FAST_ACTIVE_SCAN_TIME       30        // Active scan waiting for probe response time
#define CW_MIN_IN_BITS              4         // actual CwMin = 2^CW_MIN_IN_BITS - 1
#define CW_MAX_IN_BITS              10        // actual CwMax = 2^CW_MAX_IN_BITS - 1

// Restore channel timer
#define RSETORE_CH_TIMEOUT  16000   // uint: msec

// Note: RSSI_TO_DBM_OFFSET has been changed to variable for new RF (2004-0720).
// SHould not refer to this constant anymore
#define RSSI_TRIGGERED_UPON_BELOW_THRESHOLD     0
#define RSSI_DELTA                              5

// Channel Quality Indication
#define CQI_IS_GOOD(cqi)            ((cqi) >= 50)
//#define CQI_IS_FAIR(cqi)          (((cqi) >= 20) && ((cqi) < 50))
#define CQI_IS_POOR(cqi)            (cqi < 50)  //(((cqi) >= 5) && ((cqi) < 20))
#define CQI_IS_BAD(cqi)             (cqi < 5)
#define CQI_BELOW_ROAM(cqi)            (cqi == 5)
#define CQI_IS_DEAD(cqi)            (cqi == 0)

// weighting factor to calculate Channel quality, total should be 100%
#define RSSI_WEIGHTING                   50
#define TX_WEIGHTING                     30
#define RX_WEIGHTING                     20

//#define PEER_KEY_NOT_USED                0
//#define PEER_KEY_64_BIT                  64
//#define PEER_KEY_128_BIT                 128

//#define PEER_KEY_64BIT_LEN               8
//#define PEER_KEY_128BIT_LEN              16

//#define MAX_LEN_OF_BSS_TABLE             64
#define BSS_NOT_FOUND                    0xFFFFFFFF

#if NDIS40_MINIPORT
#define MAX_LEN_OF_MLME_QUEUE            60 //prevend W98 queue overflow!
#else
//#define MAX_LEN_OF_MLME_QUEUE            60
// TODO
#define MAX_LEN_OF_MLME_QUEUE            200
#endif

#define SCAN_PASSIVE                     18     // scan with no probe request, only wait beacon and probe response
#define SCAN_ACTIVE                      19     // scan with probe request, and wait beacon and probe response
#define FAST_SCAN_ACTIVE                 25     // scan with probe request, and wait beacon and probe response
#define SCAN_WSC_ACTIVE                  26
#define SCAN_2040_BSS_COEXIST            27
#define SCAN_P2P                         28     
#define SCAN_P2P_PASSIVE                 29

//#define BSS_TABLE_EMPTY(x)             ((x).BssNr == 0)
#define MAC_ADDR_HASH(Addr)            (Addr[0] ^ Addr[1] ^ Addr[2] ^ Addr[3] ^ Addr[4] ^ Addr[5])
#define MAC_ADDR_HASH_INDEX(Addr)      (MAC_ADDR_HASH(Addr) % HASH_TABLE_SIZE)
#define TID_MAC_HASH(Addr,TID)            (TID^Addr[0] ^ Addr[1] ^ Addr[2] ^ Addr[3] ^ Addr[4] ^ Addr[5])
#define TID_MAC_HASH_INDEX(Addr,TID)      (TID_MAC_HASH(Addr,TID) % HASH_TABLE_SIZE)

// bit definition of the 2-byte pBEACON->Capability field
#define CAP_IS_ESS_ON(x)                 (((x) & 0x0001) != 0)
#define CAP_IS_IBSS_ON(x)                (((x) & 0x0002) != 0)
#define CAP_IS_CF_POLLABLE_ON(x)         (((x) & 0x0004) != 0)
#define CAP_IS_CF_POLL_REQ_ON(x)         (((x) & 0x0008) != 0)
#define CAP_IS_PRIVACY_ON(x)             (((x) & 0x0010) != 0)
#define CAP_IS_SHORT_PREAMBLE_ON(x)      (((x) & 0x0020) != 0)
#define CAP_IS_PBCC_ON(x)                (((x) & 0x0040) != 0)
#define CAP_IS_AGILITY_ON(x)             (((x) & 0x0080) != 0)
#define CAP_IS_SPECTRUM_MGMT(x)          (((x) & 0x0100) != 0)  // 802.11e d9
#define CAP_IS_QOS(x)                    (((x) & 0x0200) != 0)  // 802.11e d9
#define CAP_IS_SHORT_SLOT(x)             (((x) & 0x0400) != 0)
#define CAP_IS_APSD(x)                   (((x) & 0x0800) != 0)  // 802.11e d9
#define CAP_IS_IMMED_BA(x)               (((x) & 0x1000) != 0)  // 802.11e d9
#define CAP_IS_DSSS_OFDM(x)              (((x) & 0x2000) != 0)
#define CAP_IS_DELAY_BA(x)               (((x) & 0x4000) != 0)  // 802.11e d9

#define CAP_GENERATE(ess,ibss,priv,s_pre,s_slot)  (((ess) ? 0x0001 : 0x0000) | ((ibss) ? 0x0002 : 0x0000) | ((priv) ? 0x0010 : 0x0000) | ((s_pre) ? 0x0020 : 0x0000) | ((s_slot) ? 0x0400 : 0x0000))

#define STA_QOS_CAPABILITY               0 // 1-byte. see 802.11e d9.0 for bit definition

#define ERP_IS_NON_ERP_PRESENT(x)        (((x) & 0x01) != 0)    // 802.11g
#define ERP_IS_USE_PROTECTION(x)         (((x) & 0x02) != 0)    // 802.11g
#define ERP_IS_USE_BARKER_PREAMBLE(x)    (((x) & 0x04) != 0)    // 802.11g

#define AGS_TX_QUALITY_WORST_BOUND       8
#define AGS_VHT_QUICK_CHECK_DELAY        100 // 100ms, Extend Quick-check to 100 ms to patch PER delay. 

#define DRS_TX_QUALITY_WORST_BOUND       4

#define BA_NOTUSE   2
//BA Policy subfiled value in ADDBA frame
#define IMMED_BA    1
#define DELAY_BA    0

// BA Initiator subfield in DELBA frame
#define  ORIGINATOR 1
#define RECIPIENT   0
#pragma pack(1)
//
// 802.11 frame formats
//
//  HT Capability INFO field in HT Cap IE .   
typedef struct  _HT_CAP_INFO{
    USHORT  LpdcCoding:1;
    USHORT  ChannelWidth:1;
    USHORT  MimoPs:2;//Spatial Multiplexing power save mode.  0:Static, 1:dynamic
    USHORT  GF:1;   //green field
    USHORT  ShortGIfor20:1;
    USHORT  ShortGIfor40:1; //for40MHz
    USHORT  TxSTBC:1;
    USHORT  RxSTBC:2;
    USHORT  DelayedBA:1;    //rt2860c not support
    USHORT  AMsduSize:1;    // only support as zero
    USHORT  CCKmodein40:1;
    USHORT  PSMP:1;
    USHORT  Intolerant40:1;
    USHORT  LSIGTxopProSup:1;
} HT_CAP_INFO, *PHT_CAP_INFO;

//  HT Capability INFO field in HT Cap IE .   
typedef struct  _HT_CAP_PARM{
    UCHAR   MaxRAmpduFactor:2;
    UCHAR   MpduDensity:3;  // Minimum MPDU start Spacing
    UCHAR   rsv:3;//momi power safe
} HT_CAP_PARM, *PHT_CAP_PARM;

//  HT Capability INFO field in HT Cap IE .   
/*typedef struct  _HT_MCS_SET{
    UCHAR   MCSSet[10];
    UCHAR   RxSupRate[2];  // unit : 1Mbps
    UCHAR   TxMCSSetDefined:1; 
    UCHAR   TxRxNotEqual:1;
    UCHAR   TxMaxStream:2;
    UCHAR   TxUnequalModSup:1;
    UCHAR   rsv:3;
    UCHAR   rsv3[3];  
} HT_MCS_SET, *PHT_MCS_SET;
*/

//  HT Capability INFO field in HT Cap IE .   
typedef struct  _EXT_HT_CAP_INFO{
    USHORT  Pco:1;
    USHORT  TranTime:2;
    USHORT  rsv:5;//momi power safe
    USHORT  MCSFeedback:2;  //0:no MCS feedback, 2:unsolicited MCS feedback, 3:Full MCS feedback,  1:rsv.
    USHORT  PlusHTC:1;  //+HTC control field support
    USHORT  RDGSupport:1;   //reverse Direction Grant  support
    USHORT  rsv2:4;
} EXT_HT_CAP_INFO, *PEXT_HT_CAP_INFO;

//  HT Beamforming field in HT Cap IE .   
typedef struct  _HT_BF_CAP{
    ULONG   ITxBfRxCapable:1; // Implicit Transmit Beamforming Receiving Capable
    ULONG   RxStaggeredSoundnigCapable:1; // Receive Staggered Sounding Capable
    ULONG   TxStaggeredSoundingCapable:1; // Transmit Staggered Sounding Capable
    ULONG   RxNDPCapable:1; // Receive NDP Capable
    ULONG   TxNDPCapable:1; // Transmit NDP Capable
    ULONG   ITxBfCapable:1; // Implicit Transmit Beamforming Capable
    ULONG   Calibration:2; // Calibration
    ULONG   ETxBfCSICapable:1; // Explicit CSI Transmit Beamforming Capable
    ULONG   ETxBfNonCompressedSteeringCapable:1; // Explicit Non-compressed Steering Capable
    ULONG   ETxBfCompressedSteeringCapable:1; // Explicit Compressed Steering Capable
    ULONG   ETxBfCSIFeedback:2; // Explicit Transmit Beamforming CSI Feedback
    ULONG   ETxBfNonCompressedFeedbackCapable:2; // Explicit Non-compressed Beamforming Feedback Capable
    ULONG   ETxBfCompressedFeedbackCapable:2; // Explicit Compressed Beamforming Feedback Capable
    ULONG   MinimalGrouping :2; // Minimal Grouping
    ULONG   CSIBfAntSupported:2; // CSI Number of Beamformer Antennas Supported
    ULONG   NonCompressedSteeringBfAntSupported:2; // Non-compressed Steering Number of Beamformer Antennas Supported
    ULONG   CompressedSteeringBfAntSupported:2; // Compressed Steering Number of Beamformer Antennas Supported
    ULONG   MaxCSIRowsBfSupported:2; // CSI Max Number of Rows Beamformer Supported
    ULONG   ChannelEstimationCapability:2; // Channel Estimation Capability
    ULONG   Reserved:3; // Reserved
} HT_BF_CAP, *PHT_BF_CAP;

//  HT antenna selection field in HT Cap IE .   
typedef struct  _HT_AS_CAP{
    UCHAR   AntSelect:1; 
    UCHAR   ExpCSIFbkTxASEL:1;
    UCHAR   AntIndFbkTxASEL:1;
    UCHAR   ExpCSIFbk:1;
    UCHAR   AntIndFbk:1;
    UCHAR   RxASel:1;
    UCHAR   TxSoundPPDU:1;
    UCHAR   rsv:1;
} HT_AS_CAP, *PHT_AS_CAP;

#define SIZE_HT_CAP_IE      26
typedef struct    _HT_CAPABILITY_IE{
    HT_CAP_INFO     HtCapInfo;
    HT_CAP_PARM HtCapParm;
    UCHAR           MCSSet[16];
    EXT_HT_CAP_INFO     ExtHtCapInfo;
    HT_BF_CAP           TxBFCap;    // beamforming cap. rt2860c not support beamforming.
    HT_AS_CAP           ASCap;  //antenna selection.
} HT_CAPABILITY_IE, *PHT_CAPABILITY_IE;

// ========== 802.11r =====================>
typedef union _FT_MIC_CTR_FIELD
{
        struct {
        USHORT rsv:8;
        USHORT IECnt:8;
        } field;
        USHORT    word;
} FT_MIC_CTR_FIELD, *PFT_MIC_CTR_FIELD;

typedef struct    _FT_IE{
    FT_MIC_CTR_FIELD        MICControl;
    UCHAR       MIC[16];
    UCHAR       ANonce[32];
    UCHAR       SNonce[32];
    UCHAR       SubElement[256]; // R0KH-ID, R1KD-ID, GTK,
} FT_IE, *PFT_IE;
// <========== 802.11r =====================

// 802.11n D3.03 7.3.2.60
typedef struct    _OVERLAP_BSS_SCAN_IE{
    UCHAR       ScanPassiveDwell[2];
    UCHAR       ScanActiveDwell[2];
    UCHAR       TriggerScanInt[2];  // Trigger scan interval
    UCHAR       PassiveTalPerChannel[2];    // passive total per channel
    UCHAR       ActiveTalPerChannel[2]; // active total per channel
    UCHAR       DelayFactor[2]; // BSS width channel transition delay factor
    UCHAR       ScanActThre[2]; // Scan Activity threshold
} OVERLAP_BSS_SCAN_IE, *POVERLAP_BSS_SCAN_IE;

// The structure for channel switch annoucement IE. This is in 802.11n D3.03
typedef struct {
    UCHAR           SwitchMode; //channel switch mode
    UCHAR           NewChannel; // 
    UCHAR           SwitchCount;    // 
} CHA_SWITCH_ANNOUNCE_IE, *PCHA_SWITCH_ANNOUNCE_IE;

// The structure for channel switch annoucement IE. This is in 802.11y
typedef struct {
    UCHAR           SwitchMode; //channel switch mode
    UCHAR           NewRegClass;    // 
    UCHAR           NewChannel; // 
    UCHAR           SwitchCount;    // 
} EXT_CHA_SWITCH_IE, *PEXT_CHA_SWITCH_IE;

// The structure for channel switch annoucement IE. This is in 802.11n D3.03
typedef struct {
    UCHAR           SecondartChannelOffset;  // 1: Secondary above, 3: Secondary below, 0: no Secondary 
} SEC_CHA_OFFSET_IE, *PSEC_CHA_OFFSET_IE;

//  
typedef struct  _SCAN_PARM{
    UCHAR               Dot11BssWidthChanTranDelayFactor;
    ULONG               Dot11BssWidthTriggerScanInt;    // Unit : Second
    ULONG               Dot11BssWidthChanTranDelay; // multiple of the above.
    ULONG               Dot11OBssScanPassiveDwell;  // Unit : TU. 5~1000
    ULONG               Dot11OBssScanActiveDwell;   // Unit : TU. 10~1000
    ULONG               Dot11OBssScanPassiveTotalPerChannel;    // Unit : TU. 200~10000
    ULONG               Dot11OBssScanActiveTotalPerChannel; // Unit : TU. 20~10000
    ULONG               Dot11OBssScanActivityThre;  // Unit : percentage
} SCAN_PARM, *PSCAN_PARM;

//  7.3.2.56. 20/40 Coexistence element used in  Element ID = 72 = IE_2040_BSS_COEXIST
typedef union  _COEXIST_ELEMT_2040{
 struct {
    UCHAR   InfoReq:1; 
    UCHAR   Intolerant40:1;
    UCHAR   BSS20WidthReq:1;
    UCHAR   ObssScanExempReq:1;
    UCHAR   ObssScanExempGrant:1;
    UCHAR   rsv:3;
    } field;
 UCHAR   word;
} COEXIST_ELEMT_2040, *PCOEXIST_ELEMT_2040;

//
// Extended capabilities bit mask
//
#define EXTENDED_CAPABILITIES_BIT_MASK (0x4000000000000000)

//
// Extended Capabilities element
//
typedef union  _EXT_CAP_ELEMT
{
    struct
    {
        ULONGLONG BssCoexstSup:1; // Support 20/40 BssCoexistence Support 
        ULONGLONG Reserved1:61;
        ULONGLONG OperatingModeNotification:1; // Operating Mode Notificaiton
        ULONGLONG Reserved2:1;
    } field;

    ULONGLONG DoubleWord;
} EXT_CAP_ELEMT, *PEXT_CAP_ELEMT;

//  7.3.2.56. 20/40 Coexistence element used in  Element ID = 72 = IE_2040_BSS_COEXIST
typedef struct  _TRIGGER_EVENTA{
    BOOLEAN     bValid;
    UCHAR   BSSID[6];   
    UCHAR   RegClass;   // Regulatory Class
    USHORT  Channel;
} TRIGGER_EVENTA, *PTRIGGER_EVENTA;

// 20/40 trigger event table
// If one Event A delete or created, or if Event B is detected or not detected, STA should send 2040BSSCoexistence to AP.
#define MAX_TRIGGER_EVENT       32
typedef struct  _TRIGGER_EVENT_TAB{
    UCHAR   EventANo;   
    TRIGGER_EVENTA  EventA[MAX_TRIGGER_EVENT];  
    ULONG           EventBCountDown;    // Count down counter for Event B. 
    ULONG           EventCCountDown;    // Count down counter for Event C. 
} TRIGGER_EVENT_TAB, *PTRIGGER_EVENT_TAB;

typedef struct _RT_HT_CAPABILITY{

    // ============================================================
    // Store HT capabilities
    // ============================================================ 
    BOOLEAN         bHtEnable;   // If we should use ht rate.
    BOOLEAN         bPreNHt;     // If we should use ht rate.
    //Substract from HT Capability IE
    UCHAR           MCSSet[16]; //only supoort MCS=0-15,32 , 
    USHORT  HtChannelWidth:2;
    USHORT  MimoPs:2;//mimo power safe MMPS_
    USHORT  GF:1;   //green field
    USHORT  ShortGIfor20:1;
    USHORT  ShortGIfor40:1; //for40MHz
    USHORT  TxSTBC:1;
    USHORT  RxSTBC:2;   // 2 bits
    USHORT  AmsduEnable:1;  // Enable to transmit A-MSDU. Suggest disable. We should use A-MPDU to gain best benifit of 802.11n
    USHORT  AmsduSize:1;    // Max receiving A-MSDU size
    USHORT  rsv:4;

    //Substract from Addiont HT INFO IE
    UCHAR   MaxRAmpduFactor:2;
    UCHAR   MpduDensity:3;
    UCHAR   ExtChanOffset:2;    // Please not the difference with following     UCHAR   NewExtChannelOffset; from 802.11n 
    UCHAR   RecomWidth:1;
    USHORT  OperaionMode:2;
    USHORT  NonGfPresent:1;
    USHORT  rsv2:13;
    
    // New Extension Channel Offset IE
    UCHAR   NewExtChannelOffset;    
    // Extension Capability IE = 127
    UCHAR   BSSCoexist2040; 
} RT_HT_CAPABILITY, *PRT_HT_CAPABILITY;

typedef struct {
    // ============================================================
    // Store VHT capabilities
    // ============================================================ 
    BOOLEAN     bVhtEnable;
    VHT_OP_IE   VhtOperation;  
    VHT_CAP_IE   VhtCapability; 
} RT_VHT_CAPABILITY, *PRT_VHT_CAPABILITY;

//   field in Addtional HT Information IE .   
typedef struct {
    UCHAR   ExtChanOffset:2;
    UCHAR   RecomWidth:1;
    UCHAR   RifsMode:1; 
    UCHAR   S_PSMPSup:1;
    UCHAR   SerInterGranu:3;     //service interval granularity
} ADD_HTINFO, *PADD_HTINFO;

typedef struct {
    USHORT  OperaionMode:2;
    USHORT  NonGfPresent:1;
    USHORT  rsv:1; 
    USHORT  OBSS_NonHTExist:1;
    USHORT  rsv2:11; 
} ADD_HTINFO2, *PADD_HTINFO2;

typedef struct {
    USHORT  StbcMcs:6;
    USHORT  DualBeacon:1;
    USHORT  DualCTSProtect:1; 
    USHORT  STBCBeacon:1;   // L-SIG TXOP protection full support 
    USHORT  LSIGTXOPProt:1; 
    USHORT  PcoActive:1; 
    USHORT  PcoPhase:1; 
    USHORT  rsv:4; 
} ADD_HTINFO3, *PADD_HTINFO3;

#define SIZE_ADD_HT_INFO_IE     22
typedef struct {
    UCHAR               ControlChan;
    ADD_HTINFO          AddHtInfo;
    ADD_HTINFO2         AddHtInfo2;  
    ADD_HTINFO3         AddHtInfo3;  
    UCHAR               MCSSet[16];
} ADD_HT_INFO_IE, *PADD_HT_INFO_IE;

typedef struct {
    UCHAR               NewExtChanOffset;
} NEW_EXT_CHAN_IE, *PNEW_EXT_CHAN_IE;


// 4-byte HTC field.  maybe included in any frame except non-QOS data frame.  The Order bit must set 1.
typedef struct {
    ULONG      MA:1;    //management action payload exist in (QoS Null+HTC)
    ULONG      TRQ:1;   //sounding request
    ULONG      MRQ:1;   //MCS feedback. Request for a MCS feedback
    ULONG      MRSorASI:3;  // MRQ Sequence identifier. unchanged during entire procedure. 0x000-0x110.
    ULONG      MFS:3;   //SET to the received value of MRS. 0x111 for unsolicited MFB.
    ULONG      MFBorASC:7;  //Link adaptation feedback containing recommended MCS. 0x7f for no feedback or not available
    ULONG      CalPos:2;    // calibration position
    ULONG      CalSeq:2;  //calibration sequence
    ULONG      FBKReq:2;    //feedback request
    ULONG      CsiSteering:2;   //CSI/Steering
    ULONG      NDPAnnouncement:1;   // NDP Announcement
    ULONG      rsv:5;  //calibration sequence
    ULONG      ACConstraint:1;  //feedback request
    ULONG      RDG:1;   //RDG / More PPDU
} HT_CONTROL, *PHT_CONTROL;

// 2-byte QOS CONTROL field
typedef struct {
    USHORT      TID:4;
    USHORT      EOSP:1;
    USHORT      AckPolicy:2;  //0: normal ACK 1:No ACK 2:scheduled under MTBA/PSMP  3: BA
    USHORT      AMsduPresent:1;
    USHORT      Txop_QueueSize:8;
} QOS_CONTROL, *PQOS_CONTROL;

// 2-byte Frame control field
typedef struct  {
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
}   FRAME_CONTROL, *PFRAME_CONTROL;

typedef struct _MLME_DLS_REQ_STRUCT {
    PRT_802_11_DLS  pDLS;
    USHORT          Reason;
} MLME_DLS_REQ_STRUCT, *PMLME_DLS_REQ_STRUCT;

typedef struct  _HEADER_802_11  {
    FRAME_CONTROL   FC;
    USHORT          Duration;
    UCHAR           Addr1[MAC_ADDR_LEN];
    UCHAR           Addr2[MAC_ADDR_LEN];
    UCHAR           Addr3[MAC_ADDR_LEN];
    USHORT          Frag:4;
    USHORT          Sequence:12;
}   HEADER_802_11, *PHEADER_802_11;

typedef struct _FRAME_802_11 {
    HEADER_802_11   Hdr;
    UCHAR            Octet[1];
}   FRAME_802_11, *PFRAME_802_11;

// QoSNull embedding of management action. When HT Control MA field set to 1.
typedef struct _MA_BODY {
    UCHAR            Category;
    UCHAR            Action;
    UCHAR            Octet[1];
}   MA_BODY, *PMA_BODY;

typedef struct  _HEADER_802_3   {
    UCHAR           DAAddr1[MAC_ADDR_LEN];
    UCHAR           SAAddr2[MAC_ADDR_LEN];
    CHAR            Octet[2];
}   HEADER_802_3, *PHEADER_802_3;
////Block ACK related format
// 2-byte BA Parameter  field  in   DELBA frames to terminate an already set up bA
typedef struct {
    USHORT      Rsv:11; // always set to 0
    USHORT      Initiator:1;    // 1: originator    0:recipient
    USHORT      TID:4;  // value of TC os TS
} DELBA_PARM, *PDELBA_PARM;

// 2-byte BA Parameter Set field  in ADDBA frames to signal parm for setting up a BA
typedef struct {
    USHORT      AMSDU:1;    
    USHORT      BAPolicy:1; // 1: immediately BA    0:delayed BA
    USHORT      TID:4;  // value of TC os TS
    USHORT      BufSize:10; // number of buffe of size 2304 octetsr
} BA_PARM, *PBA_PARM;

// 2-byte BA Starting Seq CONTROL field
typedef union {
    struct {
    USHORT      FragNum:4;  // always set to 0
    USHORT      StartSeq:12;   // sequence number of the 1st MSDU for which this BAR is sent
    }   field;
    USHORT           value;
} BASEQ_CONTROL, *PBASEQ_CONTROL;


//BAControl and BARControl are the same
// 2-byte BA CONTROL field in BA frame
typedef struct {
    USHORT      ACKPolicy:1; // only related to N-Delayed BA. But not support in RT2860b. 0:NormalACK  1:No ACK
    USHORT      MTID:1;     //EWC V1.24
    USHORT      Compressed:1;
    USHORT      Rsv:9;
    USHORT      TID:4;
} BA_CONTROL, *PBA_CONTROL;
// 2-byte BAR CONTROL field in BAR frame
typedef struct {
    USHORT      ACKPolicy:1;
    USHORT      MTID:1;     //if this bit1, use  FRAME_MTBA_REQ,  if 0, use FRAME_BA_REQ
    USHORT      Compressed:1;
    USHORT      Rsv1:9;
    USHORT      TID:4;
} BAR_CONTROL, *PBAR_CONTROL;
// BARControl in MTBAR frame
typedef struct {
    USHORT      ACKPolicy:1;
    USHORT      MTID:1;     
    USHORT      Compressed:1;
    USHORT      Rsv1:9;
    USHORT      NumTID:4;
} MTBAR_CONTROL, *PMTBAR_CONTROL;

typedef struct {
    USHORT      Rsv1:12;
    USHORT      TID:4;
} PER_TID_INFO, *PPER_TID_INFO;

typedef struct _PSPOLL_FRAME {
    FRAME_CONTROL   FC;
    USHORT          Aid;
    UCHAR           Bssid[MAC_ADDR_LEN];
    UCHAR           Ta[MAC_ADDR_LEN];
}   PSPOLL_FRAME, *PPSPOLL_FRAME;

typedef struct  _RTS_FRAME  {
    FRAME_CONTROL   FC;
    USHORT          Duration;
    UCHAR           Addr1[MAC_ADDR_LEN];
    UCHAR           Addr2[MAC_ADDR_LEN];
}   RTS_FRAME, *PRTS_FRAME;

// BAREQ AND MTBAREQ have the same subtype BAR
typedef struct _FRAME_BA_REQ {
    FRAME_CONTROL   FC;
    USHORT          Duration;
    UCHAR           Addr1[MAC_ADDR_LEN];
    UCHAR           Addr2[MAC_ADDR_LEN];
    BAR_CONTROL  BARControl;
    BASEQ_CONTROL    BAStartingSeq;
}   FRAME_BA_REQ, *PFRAME_BA_REQ;

typedef struct _FRAME_MTBA_REQ {
    FRAME_CONTROL   FC;
    USHORT          Duration;
    UCHAR           Addr1[MAC_ADDR_LEN];
    UCHAR           Addr2[MAC_ADDR_LEN];
    MTBAR_CONTROL  MTBARControl;
    PER_TID_INFO    PerTIDInfo;
    BASEQ_CONTROL    BAStartingSeq;
}   FRAME_MTBA_REQ, *PFRAME_MTBA_REQ;


// Compressed format is mandantory in HT STA
typedef struct _FRAME_MTBA {
    FRAME_CONTROL   FC;
    USHORT          Duration;
    UCHAR           Addr1[MAC_ADDR_LEN];
    UCHAR           Addr2[MAC_ADDR_LEN];
    BA_CONTROL  BAControl;
    BASEQ_CONTROL    BAStartingSeq;
    UCHAR       BitMap[8];
}   FRAME_MTBA, *PFRAME_MTBA;

typedef struct _FRAME_PSMP_ACTION {
    HEADER_802_11   Hdr;
    UCHAR   Category;
    UCHAR   Action;
    UCHAR   Psmp;   // 7.3.1.25
}   FRAME_PSMP_ACTION, *PFRAME_PSMP_ACTION;

typedef struct _FRAME_ACTION_HDR {
    HEADER_802_11   Hdr;
    UCHAR   Category;
    UCHAR   Action;
}   FRAME_ACTION_HDR, *PFRAME_ACTION_HDR;

//Action Frame
//Action Frame  Category:Spectrum,  Action:Channel Switch. 7.3.2.20
typedef struct _CHAN_SWITCH_ANNOUNCE {
    UCHAR   ElementIE;
    UCHAR   Len;
    UCHAR   Mode;
    UCHAR   NewChannel;
    UCHAR   CSCount;
}   CHAN_SWITCH_ANNOUNCE, *PCHAN_SWITCH_ANNOUNCE;

typedef struct _FRAME_ADDBA_REQ {
    HEADER_802_11   Hdr;
    UCHAR   Category;
    UCHAR   Action;
    UCHAR   Token;
    BA_PARM     BaParm;
    USHORT      TimeOutValue;
    BASEQ_CONTROL   BaStartSeq;
}   FRAME_ADDBA_REQ, *PFRAME_ADDBA_REQ;

typedef struct _FRAME_ADDBA_RSP {
    HEADER_802_11   Hdr;
    UCHAR   Category;
    UCHAR   Action;
    UCHAR   Token;
    USHORT  StatusCode;
    BA_PARM     BaParm;
    USHORT      TimeOutValue;
}   FRAME_ADDBA_RSP, *PFRAME_ADDBA_RSP;

typedef struct _FRAME_DELBA_REQ {
    HEADER_802_11   Hdr;
    UCHAR   Category;
    UCHAR   Action;
    DELBA_PARM      DelbaParm;
    USHORT  ReasonCode;
}   FRAME_DELBA_REQ, *PFRAME_DELBA_REQ;

//7.2.1.7
typedef struct _FRAME_BAR {
    FRAME_CONTROL   FC;
    USHORT          Duration;
    UCHAR           Addr1[MAC_ADDR_LEN];
    UCHAR           Addr2[MAC_ADDR_LEN];
    BAR_CONTROL     BarControl;
    BASEQ_CONTROL   StartingSeq;
}   FRAME_BAR, *PFRAME_BAR;

//7.2.1.7
typedef struct _FRAME_BA {
    FRAME_CONTROL   FC;
    USHORT          Duration;
    UCHAR           Addr1[MAC_ADDR_LEN];
    UCHAR           Addr2[MAC_ADDR_LEN];
    BA_CONTROL      BarControl;
    BASEQ_CONTROL   StartingSeq;
    UCHAR       BitMap[8];
}   FRAME_BA, *PFRAME_BA;

typedef struct _FRAME_SMPS_ACTION {
    HEADER_802_11   Hdr;
    UCHAR   Category;
    UCHAR   Action;
    UCHAR   Smps;   // 7.3.1.25
}   FRAME_SMPS_ACTION, *PFRAME_SMPS_ACTION;
#pragma pack()

// Compare Sequence number. 
#define SEQ_STEPONE(_SEQ1, _SEQ2)  (((_SEQ1 == 0) && (_SEQ2 == 0xfff))|| (_SEQ1 == ((_SEQ2+1))))
#define SEQ_SMALLER(_SEQ1, _SEQ2)  ((_SEQ1 - _SEQ2) & (0x800))
//#define SEQ_LARGER(_SEQ1, _SEQ2)  (((_SEQ1 - _SEQ2) & 0x800) == 0 && (_SEQ1 != _SEQ2 ))
#define SEQ_WITHIN_WIN(_SEQ1, _SEQ2, _WIN)  (SEQ_SMALLER(_SEQ1, ((_SEQ2 + _WIN + 1) & 0xfff)))

//
// Contention-free parameter (without ID and Length)
//
typedef struct {
    BOOLEAN     bValid;         // 1: variable contains valid value
    UCHAR       CfpCount;
    UCHAR       CfpPeriod;
    USHORT      CfpMaxDuration;
    USHORT      CfpDurRemaining;
} CF_PARM, *PCF_PARM;

typedef struct  _CIPHER_SUITE   {
    ULONG /*DOT11_CIPHER_ALGORITHM*/    PairCipher;     // Unicast cipher 1, this one has more secured cipher suite
    ULONG /*DOT11_CIPHER_ALGORITHM*/    PairCipherAux;  // Unicast cipher 2 if AP announce two unicast cipher suite
    ULONG /*DOT11_CIPHER_ALGORITHM*/    GroupCipher;    // Group cipher
    USHORT                          RsnCapability;  // RSN capability from beacon
    BOOLEAN                         bAKMwSHA256;    // Add for 11w 4-way use SHA256 case
    BOOLEAN                         bMixMode;       // Indicate Pair & Group cipher might be different
}   CIPHER_SUITE, *PCIPHER_SUITE;
    
// EDCA configuration from AP's BEACON/ProbeRsp
typedef struct _EDCA_PARM{
    BOOLEAN     bValid;         // 1: variable contains valid value
    BOOLEAN     bQAck;
    BOOLEAN     bQueueRequest;
    BOOLEAN     bTxopRequest;
        BOOLEAN     bAPSDCapable;
//  BOOLEAN     bMoreDataAck;
    UCHAR       EdcaUpdateCount;
    UCHAR       Aifsn[4];       // 0:AC_BK, 1:AC_BE, 2:AC_VI, 3:AC_VO
    UCHAR       Cwmin[4];
    UCHAR       Cwmax[4];
    USHORT      Txop[4];      // in unit of 32-us
    BOOLEAN     bACM[4];      // 1: Admission Control of AC_BK is mandattory
} EDCA_PARM, *PEDCA_PARM;

// QBSS LOAD information from QAP's BEACON/ProbeRsp
typedef struct {
    BOOLEAN     bValid;                     // 1: variable contains valid value
    USHORT      StaNum;
    UCHAR       ChannelUtilization;
    USHORT      RemainingAdmissionControl;  // in unit of 32-us
} QBSS_LOAD_PARM, *PQBSS_LOAD_PARM;

// QBSS Info field in QSTA's assoc req
typedef struct {
    UCHAR       UAPSD_AC_VO:1;
    UCHAR       UAPSD_AC_VI:1;
    UCHAR       UAPSD_AC_BK:1;
    UCHAR       UAPSD_AC_BE:1;
    UCHAR       Rsv1:1;
    UCHAR       MaxSPLength:2;
    UCHAR       Rsv2:1;
} QBSS_STA_INFO_PARM, *PQBSS_STA_INFO_PARM;

// QOS Capability reported in QAP's BEACON/ProbeRsp
// QOS Capability sent out in QSTA's AssociateReq/ReAssociateReq
typedef struct {
    BOOLEAN     bValid;                     // 1: variable contains valid value
    BOOLEAN     bQAck;
    BOOLEAN     bQueueRequest;
    BOOLEAN     bTxopRequest;
//  BOOLEAN     bMoreDataAck;
    UCHAR       EdcaUpdateCount;
} QOS_CAPABILITY_PARM, *PQOS_CAPABILITY_PARM;



typedef struct _BSS_ENTRY{
    ULONG   PhyId;

    UCHAR   Bssid[MAC_ADDR_LEN];
    UCHAR   Channel;
    UCHAR   CentralChannel; //Store the wide-band central channel for 40MHz.  .used in 40MHz AP. Or this is the same as Channel.
    UCHAR   BssType;
    USHORT  AtimWin;
    USHORT  BeaconPeriod;

    ULONGLONG BeaconTimestamp;
    ULONGLONG HostTimestamp;
    BOOLEAN     bInRegDomain; // Set to TRUE if not support 802.11d
    
    UCHAR   SupRate[MAX_LEN_OF_SUPPORTED_RATES];
    UCHAR   SupRateLen;
    UCHAR   ExtRate[MAX_LEN_OF_SUPPORTED_RATES];
    UCHAR   ExtRateLen;
    HT_CAPABILITY_IE HtCapability;
    UCHAR           HtCapabilityLen;
    VHT_CAP_IE      VhtCapability;
    VHT_OP_IE       VhtOperation;
    ADD_HT_INFO_IE AddHtInfo;   // AP might use this additional ht info IE 
    UCHAR           AddHtInfoLen;
    UCHAR           NewExtChanOffset;
    CHAR   Rssi;
    UCHAR   Privacy;            // Indicate security function ON/OFF. Don't mess up with auth mode.
    UCHAR   Hidden;
    BOOLEAN bIsCiscoAP; // use this flag to patch Cisco guest mode AP   

    USHORT  DtimPeriod;
    USHORT  CapabilityInfo;

    USHORT  CfpCount;
    USHORT  CfpPeriod;
    USHORT  CfpMaxDuration;
    USHORT  CfpDurRemaining;
    UCHAR   SsidLen;
    CHAR    Ssid[MAX_LEN_OF_SSID];

    ULONGLONG   LastBeaconRxTime; // OS's timestamp

    BOOLEAN bSES;
    RT_802_11_CIPHER_SUITE_TYPE     MixedModeGroupCipher; //for WPA+WEP mixed mode, and CCKM WEP

    // New for WPA2
    CIPHER_SUITE                    WPA;            // AP announced WPA cipher suite
    CIPHER_SUITE                    WPA2;           // AP announced WPA2 cipher suite

    // New for microsoft WPA support
#if 0   
    NDIS_802_11_FIXED_IEs   FixIEs;
#endif
    ULONG /*DOT11_AUTH_ALGORITHM*/  AuthModeAux;    // Addition mode for WPA2 / WPA capable AP
    ULONG /*DOT11_AUTH_ALGORITHM*/  AuthMode;   
    ULONG /*DOT11_CIPHER_ALGORITHM*/    WepStatus;              // Unicast Encryption Algorithm extract from VAR_IE
    USHORT                  VarIELen;               // Length of next VIE include EID & Length
    UCHAR                   VarIEs[MAX_VIE_LEN];

    UCHAR                   WSCInfoAtBeacons[MAX_VIE_LEN];
    USHORT                  WSCInfoAtBeaconsLen;
    UCHAR                   WSCInfoAtProbeRsp[MAX_VIE_LEN];
    USHORT                  WSCInfoAtProbeRspLen;
    
    // 802.11e d9, and WMM
    EDCA_PARM           EdcaParm;
    QOS_CAPABILITY_PARM QosCapability;
    QBSS_LOAD_PARM      QbssLoad;

    BOOLEAN bWPSAP;
    UCHAR   WSCAPVersion;   //The WPS version of AP
    BOOLEAN WSCAPSetupLocked;   //The SetUpLocked value of AP
    BOOLEAN P2PDevice;
    BOOLEAN bTHome303V_ConfiguredPINMode;   // T Home Speedport W 303V WPS state using PIN mode in the configured state.

    VHT_PEER_STA_CTRL_BSS_ENTRY VhtPeerStaCtrlBssEntry; // The VHT control of peer STA (BSS entry)

} BSS_ENTRY, *PBSS_ENTRY;

//
// if EnableBladEagle == TRUE, the STA can only connect to a specific AP set as Ssid[MAX_LEN_OF_SSID]
//
typedef struct {
    UCHAR     Ssid[MAX_LEN_OF_SSID];
    ULONG     SsidLen;
    BOOLEAN   EnableBladEagle;
} BALD_EAGLE, *PBALD_EAGLE;

typedef struct {
    UCHAR           BssNr;
    UCHAR           BssOverlapNr;
    BSS_ENTRY       BssEntry[MAX_LEN_OF_BSS_TABLE];
} BSS_TABLE, *PBSS_TABLE;


typedef struct _MLME_QUEUE_ELEM {
    ULONG                   Machine;
    ULONG                   MsgType;
    ULONG                   MsgLen;
    NDIS_PORT_NUMBER    PortNum;
    LARGE_INTEGER       TimeStamp;
    CHAR                    Rssi;
    CHAR                    Rssi2;
    CHAR                    Rssi3;  
    UCHAR               AntSel;
    UCHAR                   Channel;
    UCHAR                   Wcid;
    BOOLEAN                 Occupied;
    UCHAR                   Msg[MGMT_DMA_BUFFER_SIZE];
} MLME_QUEUE_ELEM, *PMLME_QUEUE_ELEM;

typedef struct _MLME_QUEUE {
    NDIS_SPIN_LOCK      Lock;
    ULONG               Num;
    ULONG               Head;
    ULONG               Tail;
    MLME_QUEUE_ELEM  Entry[MAX_LEN_OF_MLME_QUEUE];
} MLME_QUEUE, *PMLME_QUEUE;

typedef VOID (*STATE_MACHINE_FUNC)(VOID *Adaptor, MLME_QUEUE_ELEM *Elem);

typedef struct _STATE_MACHINE {
    ULONG                           Base;
    ULONG                           NrState;
    ULONG                           NrMsg;
    ULONG                           CurrState;
    STATE_MACHINE_FUNC             *TransFunc;
} STATE_MACHINE, *PSTATE_MACHINE;

#define SCAN_TERMINATION 0

// MLME AUX data structure that hold temporarliy settings during a connection attempt.
// Once this attemp succeeds, all settings will be copy to pAd->StaActive.
// A connection attempt (user set OID, roaming, CCX fast roaming,..) consists of
// several steps (JOIN, AUTH, ASSOC or REASSOC) and may fail at any step. We purposely
// separate this under-trial settings away from pAd->StaActive so that once
// this new attempt failed, driver can auto-recover back to the active settings.
typedef struct _MLME_AUX {
    UCHAR               BssType;

    //
    // for Scan
    //
    ULONG               NumOfdot11SSIDs;
    DOT11_SSID          Dot11Ssid[STA_MAX_SCAN_SSID_LIST_COUNT];
    BOOLEAN             bNeedPlatformIndicateScanStatus;   //True:need to indicate to OS
    BOOLEAN             bQueueScan;   //True:need to indicate to OS
    BOOLEAN             bNeedIndicateConnectStatus;   //True:need to indicate to OS
    BOOLEAN             bNeedIndicateRoamStatus;   //True:need to indicate to OS
    INT                 ScanForConnectCnt;
    INT                 MaxScanChannelCnt;
    PVOID               RequestID;
    
    UCHAR               Ssid[MAX_LEN_OF_SSID];
    UCHAR               SsidLen;
    UCHAR               Bssid[MAC_ADDR_LEN];
    UCHAR               AutoReconnectSsid[MAX_LEN_OF_SSID];
    UCHAR               AutoReconnectSsidLen;
    BOOLEAN             AutoReconnectStatus;
    BOOLEAN             bStaCanRoam;
    BOOLEAN             bWCN; //connect with wcn AP

    USHORT              Alg;       // Must be DOT11_AUTH_OPEN_SYSTEM(0) or DOT11_AUTH_SHARED_KEY(1) or CISCO_AuthModeLEAP(0x80), don't mix up with DOT11_AUTH_ALGORITHM
    UCHAR               ScanType;
    UCHAR               LastScanChannel;
    USHORT              Aid;
    USHORT              CapabilityInfo;
    USHORT              BeaconPeriod;
    USHORT              CfpMaxDuration;
    USHORT              CfpPeriod;
    USHORT              AtimWin;

    // Copy supported rate from desired AP's beacon. We are trying to match
    // AP's supported and extended rate settings.
    UCHAR               SupRate[MAX_LEN_OF_SUPPORTED_RATES];
    UCHAR               ExtRate[MAX_LEN_OF_SUPPORTED_RATES];
    UCHAR               SupRateLen;
    UCHAR               ExtRateLen;
    HT_CAPABILITY_IE        HtCapability;
    UCHAR                   HtCapabilityLen;
    ADD_HT_INFO_IE      AddHtInfo;  // AP might use this additional ht info IE 
    EXT_CAP_ELEMT               ExtCapIE;   // this is the extened capibility IE appreed in MGMT frames. Doesn't need to update once set in Init.
    UCHAR           NewExtChannelOffset;

    // variables for VHT
    BOOLEAN             bVhtCapable;
    VHT_CAP_IE          VhtCapability;
    VHT_OP_IE           VhtOperation;

    // new for QOS
    QOS_CAPABILITY_PARM APQosCapability;    // QOS capability of the current associated AP
    EDCA_PARM           APEdcaParm;         // EDCA parameters of the current associated AP
    QBSS_LOAD_PARM      APQbssLoad;         // QBSS load of the current associated AP

    // new to keep Ralink specific feature
    ULONG               APRalinkIe;

    BSS_TABLE           SsidBssTab;     // AP list for the same SSID
    BSS_TABLE           RoamTab;        // AP list eligible for roaming
    ULONG               BssIdx;
    ULONG               RoamIdx;

    BOOLEAN             CurrReqIsFromNdis;

    DOT11_ASSOCIATION_STATE     AssocState; //Association state. Just keeps state
    LARGE_INTEGER   AssociationUpTime;  //Timestamp when we obtained the association

    BOOLEAN bActiveScanForHiddenAP; // Active scan to search any hidden APs 
    BOOLEAN bRetainBSSEntriesInScanTable;

    UCHAR   AssocRetryCount;
} MLME_AUX, *PMLME_AUX;

typedef struct _MLME_ADDBA_REQ_STRUCT{
    UCHAR   Wcid;   // 
    UCHAR   pAddr[MAC_ADDR_LEN];
    UCHAR   BaBufSize;
    USHORT  TimeOutValue;
    UCHAR   TID;
    UCHAR   Token;
} MLME_ADDBA_REQ_STRUCT, *PMLME_ADDBA_REQ_STRUCT;


typedef struct _MLME_DELBA_REQ_STRUCT{
    UCHAR   Wcid;   // 
    UCHAR     Addr[MAC_ADDR_LEN];
    UCHAR   TID;
    UCHAR   Initiator;
    USHORT  BAOriSequence; // For send DelBA.(USB device)
} MLME_DELBA_REQ_STRUCT, *PMLME_DELBA_REQ_STRUCT;

// assoc struct is equal to reassoc
typedef struct _MLME_ASSOC_REQ_STRUCT{
    UCHAR     Addr[MAC_ADDR_LEN];
    USHORT    CapabilityInfo;
    USHORT    ListenIntv;
    ULONG     Timeout;
} MLME_ASSOC_REQ_STRUCT, *PMLME_ASSOC_REQ_STRUCT, MLME_REASSOC_REQ_STRUCT, *PMLME_REASSOC_REQ_STRUCT;

typedef struct _MLME_DISASSOC_REQ_STRUCT{
    UCHAR     Addr[MAC_ADDR_LEN];
    USHORT    Reason;
} MLME_DISASSOC_REQ_STRUCT, *PMLME_DISASSOC_REQ_STRUCT;

typedef struct _MLME_AUTH_REQ_STRUCT {
    UCHAR        Addr[MAC_ADDR_LEN];
    USHORT       Alg;
    ULONG        Timeout;
} MLME_AUTH_REQ_STRUCT, *PMLME_AUTH_REQ_STRUCT;

typedef struct _MLME_DEAUTH_REQ_STRUCT {
    UCHAR        Addr[MAC_ADDR_LEN];
    USHORT       Reason;
} MLME_DEAUTH_REQ_STRUCT, *PMLME_DEAUTH_REQ_STRUCT;

typedef struct {
    ULONG      BssIdx;
} MLME_JOIN_REQ_STRUCT;

typedef struct _MLME_SCAN_REQ_STRUCT {
    UCHAR      Bssid[MAC_ADDR_LEN];
    UCHAR      BssType;
    UCHAR      ScanType;
    ULONG      NumOfdot11SSIDs;
    DOT11_SSID Dot11Ssid[STA_MAX_SCAN_SSID_LIST_COUNT];
#if 0   
    UCHAR      SsidLen;
    CHAR       Ssid[MAX_LEN_OF_SSID];
#endif  
} MLME_SCAN_REQ_STRUCT, *PMLME_SCAN_REQ_STRUCT;

typedef struct _MLME_START_REQ_STRUCT {
    CHAR        Ssid[MAX_LEN_OF_SSID];
    UCHAR       SsidLen;
} MLME_START_REQ_STRUCT, *PMLME_START_REQ_STRUCT;

typedef struct {
    UCHAR   Eid;
    UCHAR   Len;
    CHAR   Octet[1];
} EID_STRUCT,*PEID_STRUCT, BEACON_EID_STRUCT, *PBEACON_EID_STRUCT;

typedef struct {
    UCHAR   Eid;
    UCHAR   Len[2];
    CHAR   Octet[1];
} P2PEID_STRUCT,*PP2PEID_STRUCT;

//====================================================
// CCXv4, S53, SSIDL
//====================================================
typedef struct _SSIDL_ELEM
{
    UCHAR   ExCapability;   // extended capability. legacy 802.1x, WPS
    UCHAR   Capability[4];  // Cipher of Mcast, Ucast and AKM
    UCHAR   SsidLen;
    CHAR    Ssid[1];
} SSIDL_ELEM, *PSSIDL_ELEM;

typedef struct _SSIDL_IE
{
    UCHAR   ElementID;      // 0xDD
    UCHAR   Length;         // limited by 256
    UCHAR   OUI[4];         // should be SSIDL_OUI {00-50-f2-05}
    UCHAR   ExCapability;   // Primary SSID extended capability

    UCHAR   Count;          // numbers of SSIDL elements
    UCHAR   Octet[1];
} SSIDL_IE, *PSSIDL_IE;


//=================== 11 z ==========================>>>

typedef struct  _TDLS_ACTION_HEADER {
    UCHAR                   Protocol;
    UCHAR                   Category;
    UCHAR                   Action;
}   TDLS_ACTION_HEADER, *PTDLS_ACTION_HEADER;

// TDLS State
typedef enum _TDLS_STATE {
    TDLS_MODE_NONE,             // Init state
    TDLS_MODE_WAIT_RESULT,      // Wait a response from the Responder
    TDLS_MODE_WAIT_ADD,         // Wait an confirm from the Initiator
    TDLS_MODE_CONNECTED,        // Tunneled Direct Link estabilished
    TDLS_MODE_WAIT_DELETE,      // Wait to delete tdls entries
//  TDLS_MODE_SWITCH_CHANNEL,
//  TDLS_MODE_PSM,
//  TDLS_MODE_UAPSD
} TDLS_STATE;

// TDLS Settings for each link entry
typedef struct _RT_802_11_TDLS {
    USHORT                      TimeOut;        // unit: second , set by UI
    USHORT                      CountDownTimer; // unit: second , used by driver only
    UCHAR                       MacAddr[MAC_ADDR_LEN];      // set by UI
    UCHAR                       Status;         // 0: none , 1: wait result, 2: wait add , 3: connected
    BOOLEAN                     Valid;          // 1: valid , 0: invalid , set by UI, use to setup or tear down DLS link
    // The above parameters are the same as RT_802_11_DLS_UI
    
    UCHAR                       Token;          // Dialog token
    MTK_TIMER_STRUCT         Timer;          // Use to time out while handshake
    BOOLEAN                     bInitiator;     // TRUE: I am TDLS Initiator STA, FALSE: I am TDLS Responder STA
    UCHAR                       Wcid;
    PVOID                       pAd;
    USHORT                      CapabilityInfo;

    // Copy supported rate from desired Initiator. We are trying to match
    // Initiator's supported and extended rate settings.
    UCHAR               SupRateLen;
    UCHAR               ExtRateLen;
    UCHAR               SupRate[MAX_LEN_OF_SUPPORTED_RATES];
    UCHAR               ExtRate[MAX_LEN_OF_SUPPORTED_RATES];

    // For TPK handshake
    UCHAR   ANonce[32]; // Generated in Message 1, random variable
    UCHAR   SNonce[32]; // Generated in Message 2, random variable
    ULONG   KeyLifetime;    //  Use type= 'Key Lifetime Interval' unit: Seconds, min lifetime = 300 seconds
    UCHAR   TPK[LEN_PMK];   // TPK-KCK(16 bytes) for MIC + TPK-TP (16 bytes) for data
    UCHAR   TPKName[LEN_PMK_NAME];
    
    // For QOS
    BOOLEAN             bWmmCapable;    // WMM capable of the peer TDLS
    UCHAR               QosCapability;  // QOS capability of the peer TDLS
    EDCA_PARM           IrEdcaParm;     // EDCA parameters of the Initiator

    // Features
    UCHAR               HtCapabilityLen;
    HT_CAPABILITY_IE    HtCapability;
    EXT_CAP_ELEMT       ExtCap;
    
} RT_802_11_TDLS, *PRT_802_11_TDLS;

typedef struct _MLME_TDLS_REQ_STRUCT {
    PRT_802_11_TDLS pTDLS;
    USHORT          Reason;
    UCHAR           Action;
} MLME_TDLS_REQ_STRUCT, *PMLME_TDLS_REQ_STRUCT;

typedef struct _TDLS_SEARCH_ENTRY {
    struct _TDLS_SEARCH_ENTRY * pNext;
    ULONGLONG   LastRefTime;
    UCHAR   RetryCount;
    UCHAR   Addr[MAC_ADDR_LEN];
} TDLS_SEARCH_ENTRY, *PTDLS_SEARCH_ENTRY;

// <<<=================== 11 z ==========================

// ========================== AP mlme.h ===============================
#define TBTT_PRELOAD_TIME       384        // usec. LomgPreamble + 24-byte at 1Mbps
#define DEFAULT_DTIM_PERIOD     1

// weighting factor to calculate Channel quality, total should be 100%
//#define RSSI_WEIGHTING                   0
//#define TX_WEIGHTING                     40
//#define RX_WEIGHTING                     60

#define MAC_TABLE_AGEOUT_TIME            300        // unit: sec
#define MAC_TABLE_ASSOC_TIMEOUT          5          // unit: sec
#define MAC_TABLE_FULL(Tab)              ((Tab).size == MAX_LEN_OF_MAC_TABLE)

// Value domain of pMacEntry->Sst
typedef enum _Sst {
    SST_NOT_AUTH,   // 0: equivalent to IEEE 802.11/1999 state 1
    SST_AUTH,       // 1: equivalent to IEEE 802.11/1999 state 2
    SST_ASSOC       // 2: equivalent to IEEE 802.11/1999 state 3
} SST;

// value domain of pMacEntry->AuthState
typedef enum _AuthState {
    AS_NOT_AUTH,
    AS_AUTH_OPEN,       // STA has been authenticated using OPEN SYSTEM
    AS_AUTH_KEY,        // STA has been authenticated using SHARED KEY
    AS_AUTHENTICATING   // STA is waiting for AUTH seq#3 using SHARED KEY
} AUTH_STATE;

//for-wpa value domain of pMacEntry->WpaState  802.1i D3   p.114
typedef enum _ApWpaState {
    AS_NOTUSE,              // 0
    AS_DISCONNECT,          // 1
    AS_DISCONNECTED,        // 2
    AS_INITIALIZE,          // 3
    AS_AUTHENTICATION,      // 4
    AS_AUTHENTICATION2,     // 5
    AS_INITPMK,             // 6
    AS_INITPSK,             // 7
    AS_PTKSTART,            // 8
    AS_PTKINIT_NEGOTIATING, // 9
    AS_PTKINITDONE,         // 10
    AS_UPDATEKEYS,          // 11
    AS_INTEGRITY_FAILURE,   // 12
    AS_KEYUPDATE,           // 13
    AS_STATE_MAX        
} AP_WPA_STATE;

// for-wpa value domain of pMacEntry->WpaState  802.1i D3   p.114
typedef enum _GTKState {
    REKEY_NEGOTIATING,
    REKEY_ESTABLISHED,
    KEYERROR,
} GTK_STATE;

//  for-wpa  value domain of pMacEntry->WpaState  802.1i D3   p.114
typedef enum _WpaGTKState {
    SETKEYS,
    SETKEYS_DONE,
} WPA_GTK_STATE;

//
// VHT 1x1 AGS rate table
//
#define IS_VHT_1X1_AGS_RATE_TABLE(_pRateTable) ((_pRateTable == Ags1x1VhtRateTable) || (_pRateTable == Ags1x1Vht256QAMRateTable))

//
// VHT 2x2 AGS rate table
//
#define IS_VHT_2X2_AGS_RATE_TABLE(_pRateTable) (_pRateTable == Ags2x2VhtRateTable)

//
// VHT 1x1 AGS rate table with 256QAM support
//
#define IS_VHT_1X1_256QAM_RATE_TABLE(_pRateTable) (_pRateTable == Ags1x1Vht256QAMRateTable)

//
// VHT 1x1 256QAM capable on peer-STA
//
#define VHT_1X1_256QAM_CAPABLE_ON_PEER_STA(_pEntry) (_pEntry->VhtPeerStaCtrl.VhtCapability.VhtSupportMcsSet.RxMcsMap.MaxMcsFor1ss == MCS_0_9)

//
// Check current Rate table
//
#define CURRENT_RATETABLE_IS_VHT_TABLE(Table) ((Table == Ags1x1VhtRateTable) || (Table == Ags2x2VhtRateTable) || (Table == Ags1x1Vht256QAMRateTable))

//
// HT/VHT AGS rate table
//
#define IS_AGS_RATE_TABLE(_pRateTable) ((_pRateTable == AGS1x1HTRateTable) || (_pRateTable == AGS1x1HTRateTable5G) || (_pRateTable == AGS2x2HTRateTable) || (_pRateTable == AGS3x3HTRateTable) || (_pRateTable == Ags1x1VhtRateTable) || (_pRateTable == Ags2x2VhtRateTable) || (_pRateTable == Ags1x1Vht256QAMRateTable))

#endif  // MLME_H__
